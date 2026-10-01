//! B80 / T6 pre-test: cloud runner and moving worlds (PLT-05, SCP-15, RES-05, X11). Throwaway.
//!
//! b80 burn  --secs S --threads T --tick K --csv FILE            part A: fixed all-core workload
//! b80 world --seed S --days D [--threads T] [--every N --dir DIR [--resume] [--keep K]
//!           [--slow-write-ms MS]] [--mind-iters M] [--trace N]  parts C, D: one world
//! b80 multi --worlds N --seed S --days D [--mind-iters M]       part D: N worlds, one thread each

mod burn;
mod ckpt;
mod par;
mod rng;
mod world;

use std::collections::HashMap;
use std::path::PathBuf;
use std::time::Instant;
use world::{Params, World};

struct Args(HashMap<String, String>);

impl Args {
    fn parse(v: &[String]) -> Args {
        let mut kv = HashMap::new();
        let mut i = 0;
        while i < v.len() {
            let k = v[i].trim_start_matches("--").to_string();
            if i + 1 < v.len() && !v[i + 1].starts_with("--") {
                kv.insert(k, v[i + 1].clone());
                i += 2;
            } else {
                kv.insert(k, "1".into());
                i += 1;
            }
        }
        Args(kv)
    }
    fn u64(&self, k: &str, d: u64) -> u64 {
        self.0.get(k).map(|s| s.parse().unwrap_or_else(|_| panic!("bad --{}", k))).unwrap_or(d)
    }
    fn has(&self, k: &str) -> bool {
        self.0.contains_key(k)
    }
    fn str(&self, k: &str) -> Option<&str> {
        self.0.get(k).map(|s| s.as_str())
    }
}

fn params(a: &Args) -> Params {
    Params {
        seed: a.u64("seed", 1),
        w: a.u64("w", 256) as u32,
        h: a.u64("h", 256) as u32,
        n0: a.u64("n0", 3000) as u32,
        cap: a.u64("cap", 8000) as u32,
        mind_iters: a.u64("mind-iters", 0) as u32,
    }
}

fn med_min_max(mut v: Vec<f64>) -> (f64, f64, f64) {
    v.sort_by(|a, b| a.partial_cmp(b).unwrap());
    (v[v.len() / 2], v[0], v[v.len() - 1])
}

fn cmd_world(a: &Args) {
    let p = params(a);
    let days = a.u64("days", 3650);
    let threads = a.u64("threads", 1) as usize;
    let every = a.u64("every", 0);
    let keep = a.u64("keep", 3) as usize;
    let slow = a.u64("slow-write-ms", 0);
    let trace = a.u64("trace", 0);
    let dir = a.str("dir").map(PathBuf::from);
    if let Some(d) = &dir {
        std::fs::create_dir_all(d).expect("dir");
    }
    let mut w = match (&dir, a.has("resume")) {
        (Some(d), true) => ckpt::load_newest(d, &p).unwrap_or_else(|| {
            eprintln!("FRESH no valid checkpoint");
            World::new(p)
        }),
        _ => World::new(p),
    };
    let start_day = w.day;
    let mut times: Vec<ckpt::SaveTimes> = Vec::new();
    let t0 = Instant::now();
    {
        let mut hook = |wd: &World| {
            if trace > 0 && wd.day % trace == 0 {
                eprintln!("TRACE day={} agents={} next_id={}", wd.day, wd.agents.len(), wd.next_id);
            }
            if every > 0 && wd.day % every == 0 {
                if let Some(d) = &dir {
                    times.push(ckpt::save(d, wd, slow, keep).expect("checkpoint save failed"));
                }
            }
        };
        par::run(&mut w, days, threads, &mut hook);
    }
    let wall = t0.elapsed().as_secs_f64();
    let ran = w.day - start_day;
    println!(
        "FINAL day={} agents={} next_id={} checksum={:016x} start_day={} days_run={} threads={} wall_s={:.4} world_days_per_s={:.2}",
        w.day,
        w.agents.len(),
        w.next_id,
        ckpt::checksum(&w),
        start_day,
        ran,
        threads,
        wall,
        ran as f64 / wall
    );
    if !times.is_empty() {
        let pick = |f: fn(&ckpt::SaveTimes) -> f64| med_min_max(times.iter().map(f).collect());
        let (tm, tlo, thi) = pick(|t| t.total_us);
        let sum_ms: f64 = times.iter().map(|t| t.total_us).sum::<f64>() / 1000.0;
        println!(
            "CKPT n={} bytes_last={} total_us_median={:.0} total_us_min={:.0} total_us_max={:.0} encode_us_median={:.0} write_us_median={:.0} fsync_us_median={:.0} rename_dirsync_us_median={:.0} ckpt_ms_sum={:.1} share_of_wall={:.4}",
            times.len(),
            times.last().unwrap().bytes,
            tm,
            tlo,
            thi,
            pick(|t| t.encode_us).0,
            pick(|t| t.write_us).0,
            pick(|t| t.fsync_us).0,
            pick(|t| t.rename_us).0,
            sum_ms,
            sum_ms / 1000.0 / wall
        );
    }
}

fn cmd_multi(a: &Args) {
    let p = params(a);
    let n = a.u64("worlds", 4) as usize;
    let days = a.u64("days", 3650);
    let distinct = a.has("distinct-seeds");
    let t0 = Instant::now();
    let mut ws: Vec<World> = Vec::new();
    std::thread::scope(|s| {
        let hs: Vec<_> = (0..n)
            .map(|i| {
                let pi = Params { seed: if distinct { p.seed + i as u64 } else { p.seed }, ..p };
                s.spawn(move || {
                    let mut w = World::new(pi);
                    par::run(&mut w, days, 1, &mut |_: &World| {});
                    w
                })
            })
            .collect();
        ws = hs.into_iter().map(|h| h.join().unwrap()).collect();
    });
    let wall = t0.elapsed().as_secs_f64();
    for (i, w) in ws.iter().enumerate() {
        println!("WORLD {} day={} agents={} checksum={:016x}", i, w.day, w.agents.len(), ckpt::checksum(w));
    }
    println!(
        "MULTI worlds={} days={} wall_s={:.4} world_days_per_s={:.2}",
        n,
        days,
        wall,
        (n as u64 * days) as f64 / wall
    );
}

fn main() {
    let argv: Vec<String> = std::env::args().collect();
    let cmd = argv.get(1).map(|s| s.as_str()).unwrap_or("");
    let a = Args::parse(argv.get(2..).unwrap_or(&[]));
    match cmd {
        "burn" => burn::run(a.u64("secs", 600), a.u64("threads", 4) as usize, a.u64("tick", 10), a.str("csv")),
        "world" => cmd_world(&a),
        "multi" => cmd_multi(&a),
        _ => {
            eprintln!("usage: b80 burn|world|multi [--options], see src/main.rs");
            std::process::exit(2);
        }
    }
}
