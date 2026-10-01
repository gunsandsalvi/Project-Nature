//! kbench: B01 (fast numbers on the phone) and B02 (random draws) pre-test library.
//! Throwaway: deleted once the architecture is written.
//!
//! `run(config_json) -> result_json` is the whole interface (also exported over JNI
//! with the "android" feature, for dev.kindling.pretests.Bench in the phone test app).
pub mod cpp;
pub mod kernels;
pub mod rng;
#[cfg(feature = "android")]
pub mod android;

use kernels::*;
use rng::{Gen, Stream, ALL_GENS};
use serde_json::{json, Value};
use std::sync::atomic::{AtomicBool, AtomicI32, AtomicUsize, Ordering};
use std::sync::Mutex;
use std::time::Instant;

pub const KERNELS: [&str; 5] = ["heat", "walk", "sum", "learn", "rng"];

/// JSON in, JSON out. Never panics across the boundary.
pub fn run(config_json: &str) -> String {
    match std::panic::catch_unwind(|| run_inner(config_json)) {
        Ok(Ok(v)) => v.to_string(),
        Ok(Err(e)) => json!({ "error": e }).to_string(),
        Err(p) => {
            let msg = p
                .downcast_ref::<String>()
                .cloned()
                .or_else(|| p.downcast_ref::<&str>().map(|s| s.to_string()))
                .unwrap_or_default();
            json!({ "error": format!("panic: {msg}") }).to_string()
        }
    }
}

/// Pins the calling thread to one CPU (sched_setaffinity). Linux and Android only.
pub fn pin_current_thread(cpu: usize) -> bool {
    #[cfg(any(target_os = "linux", target_os = "android"))]
    unsafe {
        let mut set: libc::cpu_set_t = std::mem::zeroed();
        if cpu >= 8 * std::mem::size_of::<libc::cpu_set_t>() {
            return false;
        }
        libc::CPU_ZERO(&mut set);
        libc::CPU_SET(cpu, &mut set);
        libc::sched_setaffinity(0, std::mem::size_of::<libc::cpu_set_t>(), &set) == 0
    }
    #[cfg(not(any(target_os = "linux", target_os = "android")))]
    {
        let _ = cpu;
        false
    }
}

fn current_cpu() -> i32 {
    #[cfg(any(target_os = "linux", target_os = "android"))]
    unsafe {
        libc::sched_getcpu()
    }
    #[cfg(not(any(target_os = "linux", target_os = "android")))]
    {
        -1
    }
}

// ---------------------------------------------------------------- harness

/// A barrier that spins briefly, then yields. Steps are short, so a sleeping barrier
/// would cost more than the work on 4 threads.
struct SpinBarrier {
    n: usize,
    count: AtomicUsize,
    gen: AtomicUsize,
}
impl SpinBarrier {
    fn new(n: usize) -> Self {
        SpinBarrier { n, count: AtomicUsize::new(0), gen: AtomicUsize::new(0) }
    }
    fn wait(&self) {
        let g = self.gen.load(Ordering::Acquire);
        if self.count.fetch_add(1, Ordering::AcqRel) + 1 == self.n {
            self.count.store(0, Ordering::Relaxed);
            self.gen.fetch_add(1, Ordering::Release);
        } else {
            let mut spins = 0u32;
            while self.gen.load(Ordering::Acquire) == g {
                spins += 1;
                if spins < 2000 {
                    std::hint::spin_loop();
                } else {
                    std::thread::yield_now();
                }
            }
        }
    }
}

pub struct Outcome {
    pub reps: u64,
    pub elapsed: f64,
    pub ck_first: u64,
    pub ck_last: u64,
    pub cpus_seen: Vec<i32>,
}

/// Rep 0 is an untimed warm-up whose checksum is kept; then reps run until `seconds`
/// have passed (and at least `min_reps`). The last rep's checksum must equal rep 0's.
pub fn measure(k: &dyn Kernel, threads: usize, seconds: f64, min_reps: u64, cpus: &[usize]) -> Outcome {
    let barrier = SpinBarrier::new(threads);
    let stop = AtomicBool::new(false);
    let result: Mutex<Option<(u64, f64, u64, u64)>> = Mutex::new(None);
    let seen: Vec<AtomicI32> = (0..threads).map(|_| AtomicI32::new(-1)).collect();
    std::thread::scope(|s| {
        for t in 0..threads {
            let (barrier, stop, result, seen) = (&barrier, &stop, &result, &seen);
            s.spawn(move || {
                if !cpus.is_empty() {
                    pin_current_thread(cpus[t % cpus.len()]);
                }
                let part = Part { t, n: threads };
                let steps = k.steps();
                let mut rep: u64 = 0;
                let mut ck_first = 0;
                let mut t0 = Instant::now();
                loop {
                    k.reset(part);
                    barrier.wait();
                    for st in 0..steps {
                        k.step(st, part);
                        barrier.wait();
                    }
                    if t == 0 {
                        if rep == 0 {
                            ck_first = k.checksum();
                            t0 = Instant::now();
                        } else {
                            let el = t0.elapsed().as_secs_f64();
                            if el >= seconds && rep >= min_reps {
                                *result.lock().unwrap() = Some((rep, el, ck_first, k.checksum()));
                                stop.store(true, Ordering::Release);
                            }
                        }
                    }
                    barrier.wait();
                    if stop.load(Ordering::Acquire) {
                        break;
                    }
                    rep += 1;
                }
                seen[t].store(current_cpu(), Ordering::Relaxed);
            });
        }
    });
    let (reps, elapsed, ck_first, ck_last) = result.into_inner().unwrap().unwrap();
    Outcome { reps, elapsed, ck_first, ck_last, cpus_seen: seen.iter().map(|a| a.load(Ordering::Relaxed)).collect() }
}

// ---------------------------------------------------------------- config

fn default_steps(name: &str) -> usize {
    match name {
        "heat" => 64,
        "walk" => 16,
        "sum" => 2,
        "learn" => 32,
        _ => 16, // rng
    }
}

fn op_name(name: &str) -> &'static str {
    match name {
        "heat" => "one cell update",
        "walk" => "one agent-step (draw, move, add cell value)",
        "sum" => "one value added",
        "learn" => "one delta-rule update (64 weights)",
        _ => "one keyed 64-bit draw",
    }
}

pub fn build_kernel(lang: Lang, name: &str, fmt: Format, gen: Gen, steps: usize) -> Result<Box<dyn Kernel>, String> {
    use Format::*;
    Ok(match (name, fmt) {
        ("heat", F32) => Box::new(Heat::<f32>::new(lang, steps)),
        ("heat", F64) => Box::new(Heat::<f64>::new(lang, steps)),
        ("heat", Fx32) => Box::new(Heat::<i32>::new(lang, steps)),
        ("heat", Fx64) => Box::new(Heat::<i64>::new(lang, steps)),
        ("walk", F32) => Box::new(Walk::<f32>::new(lang, gen, steps)),
        ("walk", F64) => Box::new(Walk::<f64>::new(lang, gen, steps)),
        ("walk", Fx32) => Box::new(Walk::<i32>::new(lang, gen, steps)),
        ("walk", Fx64) => Box::new(Walk::<i64>::new(lang, gen, steps)),
        ("sum", F32) => Box::new(Sum::<f32>::new(lang)),
        ("sum", F64) => Box::new(Sum::<f64>::new(lang)),
        ("sum", Fx32) => Box::new(Sum::<i32>::new(lang)),
        ("sum", Fx64) => Box::new(Sum::<i64>::new(lang)),
        ("learn", F32) => Box::new(Learn::<f32>::new(lang, steps)),
        ("learn", F64) => Box::new(Learn::<f64>::new(lang, steps)),
        ("learn", Fx32) => Box::new(Learn::<i32>::new(lang, steps)),
        ("rng", _) => Box::new(Rng::new(lang, gen, steps)),
        _ => return Err(format!("no kernel {name} for format {}", fmt.name())),
    })
}

fn run_inner(cfg: &str) -> Result<Value, String> {
    let v: Value = serde_json::from_str(cfg).map_err(|e| format!("bad json: {e}"))?;
    if v.get("list").and_then(Value::as_bool) == Some(true) {
        return Ok(list());
    }
    if v.get("selftest").and_then(Value::as_bool) == Some(true) {
        return Ok(selftest());
    }
    let kernel = v.get("kernel").and_then(Value::as_str).ok_or("missing \"kernel\"")?;
    let (lang_s, name) = kernel.split_once(':').ok_or("kernel must look like \"rust:heat\"")?;
    let lang = Lang::from_name(lang_s).ok_or_else(|| format!("unknown language {lang_s}"))?;
    if !KERNELS.contains(&name) {
        return Err(format!("unknown kernel {name}"));
    }
    let fmt_s = v.get("format").and_then(Value::as_str).unwrap_or("f32");
    let fmt = Format::from_name(fmt_s).ok_or_else(|| format!("unknown format {fmt_s}"))?;
    let rng_s = v.get("rng").and_then(Value::as_str).unwrap_or("splitmix");
    let gen = Gen::from_name(rng_s).ok_or_else(|| format!("unknown rng {rng_s}"))?;
    let threads = v.get("threads").and_then(Value::as_u64).unwrap_or(1) as usize;
    if !(1..=64).contains(&threads) {
        return Err("threads must be 1..64".into());
    }
    let seconds = v.get("seconds").and_then(Value::as_f64).unwrap_or(2.0).clamp(0.0, 600.0);
    let min_reps = v.get("min_reps").and_then(Value::as_u64).unwrap_or(3).max(1);
    let steps = v.get("steps").and_then(Value::as_u64).map(|s| s as usize).unwrap_or(default_steps(name));
    let steps = if name == "sum" { 2 } else { steps.max(1) };
    let cpus: Vec<usize> = v
        .get("cpus")
        .and_then(Value::as_array)
        .map(|a| a.iter().filter_map(Value::as_u64).map(|c| c as usize).collect())
        .unwrap_or_default();

    let t_setup = Instant::now();
    let k = build_kernel(lang, name, fmt, gen, steps)?;
    let setup_s = t_setup.elapsed().as_secs_f64();
    let o = measure(&*k, threads, seconds, min_reps, &cpus);
    let ops = o.reps * k.ops_per_rep();
    let uses_rng = name == "walk" || name == "rng";
    Ok(json!({
        "kernel": kernel,
        "lang": lang.name(),
        "name": name,
        "format": if name == "rng" { Value::Null } else { json!(fmt.name()) },
        "rng": if uses_rng { json!(gen.name()) } else { Value::Null },
        "threads": threads,
        "ops_per_sec": ops as f64 / o.elapsed,
        "ops": ops,
        "elapsed_s": o.elapsed,
        "reps": o.reps,
        "ops_per_rep": k.ops_per_rep(),
        "op": op_name(name),
        "checksum": format!("{:016x}", o.ck_last),
        "checksum_first_rep": format!("{:016x}", o.ck_first),
        "reps_consistent": o.ck_first == o.ck_last,
        "setup_s": setup_s,
        "cpus_requested": cpus,
        "cpus_seen": o.cpus_seen,
        "arch": std::env::consts::ARCH,
        "os": std::env::consts::OS,
        "extra": k.extra(),
    }))
}

/// Kernels and suggested settings; `plan` is a ready-made run list for the phone app.
pub fn list() -> Value {
    let all_f = ["f32", "f64", "fx32", "fx64"];
    let gens: Vec<&str> = ALL_GENS.iter().map(|g| g.name()).collect();
    let mut kernels = vec![];
    for lang in ["rust", "cpp", "cppnc"] {
        for name in KERNELS {
            let formats: Vec<&str> = match name {
                "learn" => vec!["f32", "f64", "fx32"],
                "rng" => vec![],
                _ => all_f.to_vec(),
            };
            kernels.push(json!({
                "kernel": format!("{lang}:{name}"),
                "formats": formats,
                "rngs": if name == "walk" || name == "rng" { json!(gens) } else { Value::Null },
                "op": op_name(name),
                "suggested": {"threads": 1, "seconds": 1.5},
                "note": if lang == "cppnc" { "C++ with fused multiply-add off: for checksum comparison only" } else { "" },
            }));
        }
    }
    let mut plan = vec![];
    for lang in ["rust", "cpp"] {
        for f in all_f {
            plan.push(json!({"kernel": format!("{lang}:heat"), "format": f, "threads": 1, "seconds": 1.5}));
            plan.push(json!({"kernel": format!("{lang}:walk"), "format": f, "rng": "splitmix", "threads": 1, "seconds": 1.5}));
            plan.push(json!({"kernel": format!("{lang}:sum"), "format": f, "threads": 1, "seconds": 1.5}));
            if f != "fx64" {
                plan.push(json!({"kernel": format!("{lang}:learn"), "format": f, "threads": 1, "seconds": 1.5}));
            }
        }
        for g in &gens {
            plan.push(json!({"kernel": format!("{lang}:rng"), "rng": g, "threads": 1, "seconds": 1.5}));
        }
    }
    json!({
        "kernels": kernels,
        "formats": all_f,
        "rngs": gens,
        "rng_about": ALL_GENS.iter().map(|g| json!({"rng": g.name(), "about": g.about()})).collect::<Vec<_>>(),
        "config_keys": {
            "kernel": "lang:name, e.g. rust:heat",
            "format": "f32|f64|fx32|fx64 (ignored by rng)",
            "rng": "generator for walk and rng",
            "threads": "1..64, fixed partition",
            "seconds": "timed length after one warm-up rep",
            "cpus": "optional list of CPU numbers; worker t is pinned to cpus[t % len]",
            "steps": "optional steps per rep",
            "min_reps": "optional, default 3",
        },
        "plan": plan,
    })
}

/// Quick correctness checks that also make sense on the phone: known answers for the
/// generators, and Rust, C++ and C++-without-FMA giving the same draws.
pub fn selftest() -> Value {
    let mut fails: Vec<String> = vec![];
    if rng::philox4x32_10([0; 4], [0; 2]) != [0x6627e8d5, 0xe169c58d, 0xbc57ac4c, 0x9b00dbd8] {
        fails.push("philox known answer".into());
    }
    if rng::chacha8_first64(&[0; 8], 0, 0) != 0xd640_5f89_2fef_003e {
        fails.push("chacha8 known answer".into());
    }
    if rng::mix64(rng::GOLDEN) != 0xe220_a839_7b1d_cdaf {
        fails.push("splitmix known answer".into());
    }
    let st = Stream::new(42, 5, 77);
    for g in ALL_GENS {
        for i in 0..2000u64 {
            let (b, m) = (rng::mix64(i) >> (i % 64), i.wrapping_mul(0x1234_5678_9abc_def1));
            let r = rng::draw(g, &st, b, m);
            let c = unsafe { (cpp::CPP.rng_one)(g as i32, &st, b, m) };
            let n = unsafe { (cpp::CPPNC.rng_one)(g as i32, &st, b, m) };
            if r != c || r != n {
                fails.push(format!("{} rust/cpp draws differ at being {b} moment {m}", g.name()));
                break;
            }
        }
    }
    json!({"selftest": fails.is_empty(), "failures": fails, "arch": std::env::consts::ARCH})
}
