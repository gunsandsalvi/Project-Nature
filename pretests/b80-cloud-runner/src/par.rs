//! B80 / B05 stand-in: one world on N threads, in fixed read-then-write phases.
//!
//! X11: each phase splits rows or agents into fixed ranges, every element is computed only from data
//! of earlier phases, and the serial steps (count, deaths and births, checkpoint) run on thread 0 in
//! a fixed order. So the result is bit-identical for any thread count and any thread timing.

use crate::world::{self, Agent, Params, World};
use std::cell::UnsafeCell;
use std::sync::atomic::{AtomicUsize, Ordering};

/// Spin-then-yield barrier (cheap when every thread has its own core).
pub struct SpinBarrier {
    n: usize,
    count: AtomicUsize,
    gen: AtomicUsize,
}

impl SpinBarrier {
    pub fn new(n: usize) -> Self {
        SpinBarrier { n, count: AtomicUsize::new(0), gen: AtomicUsize::new(0) }
    }
    pub fn wait(&self) {
        let gen = self.gen.load(Ordering::Acquire);
        if self.count.fetch_add(1, Ordering::AcqRel) + 1 == self.n {
            self.count.store(0, Ordering::Relaxed);
            self.gen.fetch_add(1, Ordering::Release);
        } else {
            let mut spins = 0u32;
            while self.gen.load(Ordering::Acquire) == gen {
                if spins < 20_000 {
                    spins += 1;
                    std::hint::spin_loop();
                } else {
                    std::thread::yield_now();
                }
            }
        }
    }
}

/// Raw view of the world's arrays, republished by thread 0 after every serial step.
#[derive(Clone, Copy)]
struct Snap {
    day: u64,
    field: *mut f32,
    tmp: *mut f32,
    ncell: usize,
    agents: *mut Agent,
    nag: usize,
    count: *mut u16,
    flags: *mut u8,
}

struct Shared {
    snap: UnsafeCell<Snap>,
    bar: SpinBarrier,
    p: Params,
    n: usize,
    until: u64,
}

// Safety: threads touch disjoint ranges between barriers; only thread 0 writes `snap`, and only
// while the others wait at a barrier.
unsafe impl Sync for Shared {}

fn snap_of(w: &mut World) -> Snap {
    Snap {
        day: w.day,
        field: w.field.as_mut_ptr(),
        tmp: w.tmp.as_mut_ptr(),
        ncell: w.field.len(),
        agents: w.agents.as_mut_ptr(),
        nag: w.agents.len(),
        count: w.count.as_mut_ptr(),
        flags: w.flags.as_mut_ptr(),
    }
}

#[inline]
fn range(len: usize, n: usize, t: usize) -> (usize, usize) {
    (len * t / n, len * (t + 1) / n)
}

/// Run `world` until `until` (a day number). `hook` runs on thread 0 after every day.
pub fn run(world: &mut World, until: u64, nthreads: usize, hook: &mut dyn FnMut(&World)) {
    if nthreads <= 1 {
        while world.day < until {
            world.step();
            hook(world);
        }
        return;
    }
    let sh = Shared {
        snap: UnsafeCell::new(snap_of(world)),
        bar: SpinBarrier::new(nthreads),
        p: world.p,
        n: nthreads,
        until,
    };
    std::thread::scope(|s| {
        for t in 1..nthreads {
            let shr = &sh;
            s.spawn(move || worker(shr, t, None, &mut |_: &World| {}));
        }
        worker(&sh, 0, Some(world), hook);
    });
}

fn worker(sh: &Shared, t: usize, mut world: Option<&mut World>, hook: &mut dyn FnMut(&World)) {
    let p = sh.p;
    let n = sh.n;
    let w = p.w as usize;
    let (y0, y1) = range(p.h as usize, n, t);
    loop {
        sh.bar.wait(); // B0: snapshot published
        let s = unsafe { *sh.snap.get() };
        if s.day >= sh.until {
            break;
        }
        let day = s.day;
        // P1: diffuse own rows into tmp (reads all of field).
        unsafe {
            let src = std::slice::from_raw_parts(s.field as *const f32, s.ncell);
            let dst = std::slice::from_raw_parts_mut(s.tmp.add(y0 * w), (y1 - y0) * w);
            world::diffuse_rows(&p, src, dst, y0, y1);
        }
        sh.bar.wait(); // B1
        if let Some(wd) = world.as_deref_mut() {
            std::mem::swap(&mut wd.field, &mut wd.tmp);
            unsafe { *sh.snap.get() = snap_of(wd) };
        }
        sh.bar.wait(); // B2
        let s = unsafe { *sh.snap.get() };
        let (a0, a1) = range(s.nag, n, t);
        // P2: minds and moves of own agents (reads field).
        unsafe {
            let field = std::slice::from_raw_parts(s.field as *const f32, s.ncell);
            let ags = std::slice::from_raw_parts_mut(s.agents.add(a0), a1 - a0);
            for a in ags.iter_mut() {
                world::mind_and_move(&p, field, day, a);
            }
        }
        sh.bar.wait(); // B3
        if let Some(wd) = world.as_deref_mut() {
            wd.recount();
            unsafe { *sh.snap.get() = snap_of(wd) };
        }
        sh.bar.wait(); // B4
        let s = unsafe { *sh.snap.get() };
        // P3: eating, deaths and births decided for own agents (reads field and count).
        unsafe {
            let field = std::slice::from_raw_parts(s.field as *const f32, s.ncell);
            let count = std::slice::from_raw_parts(s.count as *const u16, s.ncell);
            let ags = std::slice::from_raw_parts_mut(s.agents.add(a0), a1 - a0);
            let fl = std::slice::from_raw_parts_mut(s.flags.add(a0), a1 - a0);
            for (a, f) in ags.iter_mut().zip(fl.iter_mut()) {
                *f = world::eat_and_fate(&p, field, count, day, a);
            }
        }
        sh.bar.wait(); // B5
        // P4: eaten cells lose food (own rows).
        unsafe {
            let fr = std::slice::from_raw_parts_mut(s.field.add(y0 * w), (y1 - y0) * w);
            let cr = std::slice::from_raw_parts(s.count.add(y0 * w) as *const u16, (y1 - y0) * w);
            world::eaten_rows(fr, cr);
        }
        sh.bar.wait(); // B6
        if let Some(wd) = world.as_deref_mut() {
            wd.apply_fates(day);
            wd.day += 1;
            hook(wd);
            unsafe { *sh.snap.get() = snap_of(wd) };
        }
    }
}
