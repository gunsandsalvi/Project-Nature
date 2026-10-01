//! B80 / T6 part A: a fixed, deterministic all-core workload. Every unit of work is identical:
//! one diffusion sweep over a 128 x 128 field plus 2048 keyed draws. Units are counted per tick,
//! together with CPU time shares from /proc/stat (steal is what vmstat shows as "st").

use crate::rng::key_hash;
use crate::world::{diffuse_rows, Params};
use std::io::Write;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::time::{Duration, Instant};

#[repr(align(64))]
struct Counter(AtomicU64);

/// user nice system idle iowait irq softirq steal (jiffies, all CPUs).
fn proc_stat() -> [u64; 8] {
    let s = std::fs::read_to_string("/proc/stat").unwrap_or_default();
    let mut out = [0u64; 8];
    if let Some(line) = s.lines().next() {
        for (i, v) in line.split_whitespace().skip(1).take(8).enumerate() {
            out[i] = v.parse().unwrap_or(0);
        }
    }
    out
}

pub fn run(secs: u64, threads: usize, tick: u64, csv: Option<&str>) {
    let stop = AtomicBool::new(false);
    let counters: Vec<Counter> = (0..threads).map(|_| Counter(AtomicU64::new(0))).collect();
    let mut out: Box<dyn Write> = match csv {
        Some(path) => Box::new(std::fs::File::create(path).expect("csv")),
        None => Box::new(std::io::sink()),
    };
    let cols: Vec<String> = (0..threads).map(|t| format!("units_t{}", t)).collect();
    writeln!(out, "t_s,units_total,{},user_pct,sys_pct,idle_pct,iowait_pct,steal_pct", cols.join(",")).unwrap();
    std::thread::scope(|s| {
        for t in 0..threads {
            let stop = &stop;
            let c = &counters[t];
            s.spawn(move || {
                let p = Params { seed: 0xB80 + t as u64, w: 128, h: 128, n0: 0, cap: 0, mind_iters: 0 };
                let mut f = vec![0.0f32; 128 * 128];
                let mut g = vec![0.0f32; 128 * 128];
                for (i, v) in f.iter_mut().enumerate() {
                    *v = 0.5 + 0.5 * ((key_hash(p.seed, i as u64, 0, 1) >> 40) as f32 / 16_777_216.0);
                }
                let mut unit = 0u64;
                let mut sink = 0u64;
                while !stop.load(Ordering::Relaxed) {
                    diffuse_rows(&p, &f, &mut g, 0, 128);
                    std::mem::swap(&mut f, &mut g);
                    for i in 0..2048u64 {
                        sink ^= key_hash(p.seed, i, unit, 9);
                    }
                    unit += 1;
                    c.0.store(unit, Ordering::Relaxed);
                }
                std::hint::black_box(sink);
                std::hint::black_box(&f);
            });
        }
        let t0 = Instant::now();
        let mut prev_units = vec![0u64; threads];
        let mut prev_stat = proc_stat();
        let mut k = 1u64;
        while k * tick <= secs {
            let target = t0 + Duration::from_secs(k * tick);
            let now = Instant::now();
            if target > now {
                std::thread::sleep(target - now);
            }
            let st = proc_stat();
            let units: Vec<u64> = counters.iter().map(|c| c.0.load(Ordering::Relaxed)).collect();
            let d: Vec<u64> = units.iter().zip(&prev_units).map(|(a, b)| a - b).collect();
            let ds: Vec<u64> = st.iter().zip(&prev_stat).map(|(a, b)| a.saturating_sub(*b)).collect();
            let tot = ds.iter().sum::<u64>().max(1) as f64;
            let pct = |x: u64| 100.0 * x as f64 / tot;
            let per: Vec<String> = d.iter().map(|x| x.to_string()).collect();
            let line = format!(
                "{},{},{},{:.2},{:.2},{:.2},{:.2},{:.2}",
                k * tick,
                d.iter().sum::<u64>(),
                per.join(","),
                pct(ds[0] + ds[1]),
                pct(ds[2] + ds[5] + ds[6]),
                pct(ds[3]),
                pct(ds[4]),
                pct(ds[7])
            );
            writeln!(out, "{}", line).unwrap();
            out.flush().unwrap();
            println!("{}", line);
            prev_units = units;
            prev_stat = st;
            k += 1;
        }
        stop.store(true, Ordering::Relaxed);
    });
}
