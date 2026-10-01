//! B80 toy world: a few thousand agents on a wrapping grid with a diffusing food field, one step per
//! simulated day. Stand-in for the real simulation (B04, B05, B07).
//!
//! X11: everything that decides the future is in the saved fields (params, day, next_id, field,
//! agents). The scratch buffers (tmp, count, flags) are fully rebuilt every day before they are read,
//! so a resumed run cannot depend on them.

use crate::rng::{draw_f64, draw_u64, key_hash, splitmix64};

// Purposes of keyed draws (the last part of the key).
pub const P_INIT_FIELD: u64 = 1;
pub const P_INIT_AGENT: u64 = 2;
pub const P_MIND: u64 = 3;
pub const P_MOVE: u64 = 4;
pub const P_MOVE_PICK: u64 = 5;
pub const P_DEATH: u64 = 6;
pub const P_BIRTH: u64 = 7;
pub const P_CHILD: u64 = 8;

pub const DIE: u8 = 1;
pub const BIRTH: u8 = 2;

const DIFF: f32 = 0.10;
const REGROW: f32 = 0.004;
const EAT: f32 = 0.5;
const METAB: f32 = 0.05;
const BIRTH_E: f32 = 2.0;
const P_BIRTH_DAY: f64 = 0.02;
const P_DEATH_DAY: f64 = 1.0 / (365.0 * 30.0);
const OLD_AGE: u32 = 365 * 40;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Params {
    pub seed: u64,
    pub w: u32,
    pub h: u32,
    pub n0: u32,
    pub cap: u32,
    /// Stand-in for a mind's computing per agent per day (iterations of a hash chain).
    pub mind_iters: u32,
}

#[derive(Clone, Copy, Debug)]
pub struct Agent {
    pub id: u64,
    pub energy: f32,
    pub mind: f32,
    pub age: u32,
    pub x: u16,
    pub y: u16,
}

pub struct World {
    pub p: Params,
    pub day: u64,
    pub next_id: u64,
    pub field: Vec<f32>,
    pub agents: Vec<Agent>,
    // Scratch, rebuilt every day, never saved.
    pub tmp: Vec<f32>,
    pub count: Vec<u16>,
    pub flags: Vec<u8>,
}

impl World {
    pub fn alloc(p: Params) -> World {
        let n = (p.w * p.h) as usize;
        World {
            p,
            day: 0,
            next_id: 0,
            field: vec![0.0; n],
            agents: Vec::with_capacity(p.cap as usize),
            tmp: vec![0.0; n],
            count: vec![0; n],
            flags: vec![0; p.cap as usize],
        }
    }

    /// Day-0 state, made only from the seed.
    pub fn new(p: Params) -> World {
        let mut w = World::alloc(p);
        for c in 0..w.field.len() {
            w.field[c] = 0.5 + 0.5 * draw_f64(p.seed, c as u64, 0, P_INIT_FIELD) as f32;
        }
        for i in 0..p.n0.min(p.cap) as u64 {
            let r = draw_u64(p.seed, i, 0, P_INIT_AGENT);
            let x = (r % p.w as u64) as u16;
            let y = ((r >> 20) % p.h as u64) as u16;
            let age = ((r >> 40) % (365 * 30)) as u32;
            w.agents.push(Agent { id: i, energy: 1.0, mind: 0.0, age, x, y });
        }
        w.next_id = p.n0 as u64;
        w
    }

    /// One simulated day on one thread. Same kernels as the multi-thread engine in par.rs.
    pub fn step(&mut self) {
        let day = self.day;
        let p = self.p;
        diffuse_rows(&p, &self.field, &mut self.tmp, 0, p.h as usize);
        std::mem::swap(&mut self.field, &mut self.tmp);
        for a in self.agents.iter_mut() {
            mind_and_move(&p, &self.field, day, a);
        }
        self.recount();
        let n = self.agents.len();
        for (a, f) in self.agents.iter_mut().zip(self.flags[..n].iter_mut()) {
            *f = eat_and_fate(&p, &self.field, &self.count, day, a);
        }
        eaten_rows(&mut self.field, &self.count);
        self.apply_fates(day);
        self.day += 1;
    }

    /// Serial: how many agents stand on each cell (integer, so order cannot matter).
    pub fn recount(&mut self) {
        self.count.iter_mut().for_each(|c| *c = 0);
        let w = self.p.w as usize;
        for a in &self.agents {
            let c = a.y as usize * w + a.x as usize;
            self.count[c] = self.count[c].saturating_add(1);
        }
    }

    /// Serial, in agent order: remove the dead, add the newborn with fresh ids.
    pub fn apply_fates(&mut self, day: u64) {
        let n = self.agents.len();
        let survivors = self.flags[..n].iter().filter(|&&f| f & DIE == 0).count();
        let mut room = (self.p.cap as usize).saturating_sub(survivors);
        let (w, h) = (self.p.w as u64, self.p.h as u64);
        let mut births: Vec<Agent> = Vec::new();
        let mut j = 0;
        for i in 0..n {
            let f = self.flags[i];
            if f & DIE != 0 {
                continue;
            }
            let a = self.agents[i];
            if f & BIRTH != 0 && room > 0 {
                room -= 1;
                let id = self.next_id;
                self.next_id += 1;
                let r = draw_u64(self.p.seed, id, day, P_CHILD);
                let dx = r % 3;
                let dy = (r >> 8) % 3;
                let x = ((a.x as u64 + w + dx - 1) % w) as u16;
                let y = ((a.y as u64 + h + dy - 1) % h) as u16;
                births.push(Agent { id, energy: a.energy, mind: a.mind * 0.5, age: 0, x, y });
            }
            self.agents[j] = a;
            j += 1;
        }
        self.agents.truncate(j);
        self.agents.extend_from_slice(&births);
    }
}

#[inline(always)]
fn cell(l: f32, r: f32, u: f32, d: f32, c: f32) -> f32 {
    // Fixed order of operations, so every build of this code gives the same bits.
    let v = c + DIFF * (((l + r) + (u + d)) - 4.0 * c);
    v + REGROW * (1.0 - v)
}

/// Diffusion and regrowth for rows y0..y1, written into `dst` (which holds just those rows).
pub fn diffuse_rows(p: &Params, src: &[f32], dst: &mut [f32], y0: usize, y1: usize) {
    let w = p.w as usize;
    let h = p.h as usize;
    for y in y0..y1 {
        let yu = if y == 0 { h - 1 } else { y - 1 };
        let yd = if y + 1 == h { 0 } else { y + 1 };
        let row = &src[y * w..(y + 1) * w];
        let up = &src[yu * w..(yu + 1) * w];
        let dn = &src[yd * w..(yd + 1) * w];
        let out = &mut dst[(y - y0) * w..(y - y0 + 1) * w];
        out[0] = cell(row[w - 1], row[1], up[0], dn[0], row[0]);
        for x in 1..w - 1 {
            out[x] = cell(row[x - 1], row[x + 1], up[x], dn[x], row[x]);
        }
        out[w - 1] = cell(row[w - 2], row[0], up[w - 1], dn[w - 1], row[w - 1]);
    }
}

/// The agent "thinks" (a stand-in cost), then moves to its own or a neighbouring cell.
#[inline]
pub fn mind_and_move(p: &Params, field: &[f32], day: u64, a: &mut Agent) {
    let w = p.w as usize;
    let h = p.h as usize;
    let mut hs = key_hash(p.seed, a.id, day, P_MIND);
    let mut m = a.mind;
    for _ in 0..p.mind_iters {
        hs = splitmix64(hs);
        m = m * 0.999 + ((hs >> 40) as f32) * (0.001 / 16_777_216.0);
    }
    a.mind = m;
    let x = a.x as usize;
    let y = a.y as usize;
    let xl = if x == 0 { w - 1 } else { x - 1 };
    let xr = if x + 1 == w { 0 } else { x + 1 };
    let yu = if y == 0 { h - 1 } else { y - 1 };
    let yd = if y + 1 == h { 0 } else { y + 1 };
    let opts = [(x, y), (xr, y), (xl, y), (x, yd), (x, yu)];
    let greed = 0.6 + 0.3 * m.fract() as f64;
    let pick = if draw_f64(p.seed, a.id, day, P_MOVE) < greed {
        let mut best = 0;
        let mut bv = field[y * w + x];
        for (k, &(ox, oy)) in opts.iter().enumerate().skip(1) {
            let v = field[oy * w + ox];
            if v > bv {
                bv = v;
                best = k;
            }
        }
        best
    } else {
        (draw_u64(p.seed, a.id, day, P_MOVE_PICK) % 5) as usize
    };
    a.x = opts[pick].0 as u16;
    a.y = opts[pick].1 as u16;
}

/// The agent eats its share of its cell, ages, and may die or give birth. Returns flags.
#[inline]
pub fn eat_and_fate(p: &Params, field: &[f32], count: &[u16], day: u64, a: &mut Agent) -> u8 {
    let c = a.y as usize * p.w as usize + a.x as usize;
    let gain = field[c] * EAT / count[c] as f32;
    a.energy = a.energy + gain - METAB;
    a.age += 1;
    if a.energy <= 0.0 {
        return DIE;
    }
    let mut pd = P_DEATH_DAY;
    if a.age > OLD_AGE {
        pd += (a.age - OLD_AGE) as f64 * 2e-6;
    }
    if draw_f64(p.seed, a.id, day, P_DEATH) < pd {
        return DIE;
    }
    if a.energy > BIRTH_E && draw_f64(p.seed, a.id, day, P_BIRTH) < P_BIRTH_DAY {
        a.energy *= 0.5;
        return BIRTH;
    }
    0
}

/// Cells where anyone ate lose that share of their food.
pub fn eaten_rows(field: &mut [f32], count: &[u16]) {
    for (f, &c) in field.iter_mut().zip(count) {
        if c > 0 {
            *f *= 1.0 - EAT;
        }
    }
}
