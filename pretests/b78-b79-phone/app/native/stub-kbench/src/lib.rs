//! B78: stand-in for the B01/B02 `kbench` crate, with the same interface, so the
//! round-1 app works even if the real crate doesn't build for the phone.
//! `run(config_json) -> result_json`; `{"list": true}` lists kernels and a run plan.
//! Kernels: "stub:heat" (f32 diffusion on a ring) and "stub:mix" (keyed 64-bit draws).
//! Every repetition does the same work, so its checksum must repeat exactly (X11).
use serde_json::{json, Value};
use std::time::Instant;

#[cfg(feature = "android")]
pub mod android;

const CELLS: usize = 1 << 14; // per thread
const STEPS: usize = 32;
const DRAWS: u64 = 1 << 16; // per thread per repetition

pub fn run(config_json: &str) -> String {
    match std::panic::catch_unwind(|| run_inner(config_json)) {
        Ok(Ok(v)) => v.to_string(),
        Ok(Err(e)) => json!({ "error": e }).to_string(),
        Err(_) => json!({ "error": "panic" }).to_string(),
    }
}

pub fn pin_current_thread(cpu: usize) -> bool {
    #[cfg(any(target_os = "linux", target_os = "android"))]
    unsafe {
        if cpu >= 8 * std::mem::size_of::<libc::cpu_set_t>() {
            return false;
        }
        let mut set: libc::cpu_set_t = std::mem::zeroed();
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

fn mix64(mut x: u64) -> u64 {
    x = (x ^ (x >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
    x = (x ^ (x >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
    x ^ (x >> 31)
}

/// One repetition of a kernel for thread `t`; returns (ops, checksum).
fn rep(kernel: &str, t: usize) -> (u64, u64) {
    match kernel {
        "stub:heat" => {
            let mut a: Vec<f32> = (0..CELLS).map(|i| ((i * 7 + t) % 101) as f32).collect();
            let mut b = vec![0f32; CELLS];
            for _ in 0..STEPS {
                for i in 0..CELLS {
                    let l = a[(i + CELLS - 1) % CELLS];
                    let r = a[(i + 1) % CELLS];
                    b[i] = a[i] + 0.25 * (l + r - 2.0 * a[i]);
                }
                std::mem::swap(&mut a, &mut b);
            }
            let ck = a.iter().fold(0u64, |h, v| mix64(h ^ v.to_bits() as u64));
            ((CELLS * STEPS) as u64, ck)
        }
        _ => {
            // keyed draws: being = t, moment = i
            let key = mix64(0x5EED ^ (t as u64) << 32);
            let ck = (0..DRAWS).fold(0u64, |h, i| h ^ mix64(key ^ mix64(i)));
            (DRAWS, ck)
        }
    }
}

fn run_inner(cfg: &str) -> Result<Value, String> {
    let v: Value = serde_json::from_str(cfg).map_err(|e| format!("bad json: {e}"))?;
    if v.get("list").and_then(Value::as_bool) == Some(true) {
        return Ok(json!({
            "stub": true,
            "kernels": [{"kernel": "stub:heat", "formats": ["f32"]}, {"kernel": "stub:mix", "formats": []}],
            "plan": [
                {"kernel": "stub:heat", "format": "f32", "threads": 1, "seconds": 1.5},
                {"kernel": "stub:mix", "rng": "splitmix", "threads": 1, "seconds": 1.5}
            ]
        }));
    }
    if v.get("selftest").and_then(Value::as_bool) == Some(true) {
        return Ok(json!({"selftest": mix64(1) == mix64(1), "stub": true}));
    }
    let kernel = v.get("kernel").and_then(Value::as_str).ok_or("missing \"kernel\"")?.to_string();
    if kernel != "stub:heat" && kernel != "stub:mix" {
        return Err(format!("unknown kernel {kernel}"));
    }
    let threads = v.get("threads").and_then(Value::as_u64).unwrap_or(1).clamp(1, 64) as usize;
    let seconds = v.get("seconds").and_then(Value::as_f64).unwrap_or(1.5).clamp(0.0, 600.0);
    let cpus: Vec<usize> = v
        .get("cpus")
        .and_then(Value::as_array)
        .map(|a| a.iter().filter_map(Value::as_u64).map(|c| c as usize).collect())
        .unwrap_or_default();
    let t0 = Instant::now();
    let per_thread: Vec<(u64, u64, u64, bool, i32)> = std::thread::scope(|s| {
        let hs: Vec<_> = (0..threads)
            .map(|t| {
                let (kernel, cpus) = (&kernel, &cpus);
                s.spawn(move || {
                    if !cpus.is_empty() {
                        pin_current_thread(cpus[t % cpus.len()]);
                    }
                    let (_, first) = rep(kernel, t); // untimed warm-up; its checksum is kept
                    let start = Instant::now();
                    let (mut ops, mut reps, mut last) = (0u64, 0u64, first);
                    while reps < 3 || start.elapsed().as_secs_f64() < seconds {
                        let (o, c) = rep(kernel, t);
                        ops += o;
                        reps += 1;
                        last = c;
                    }
                    (ops, reps, last, last == first, current_cpu())
                })
            })
            .collect();
        hs.into_iter().map(|h| h.join().unwrap()).collect()
    });
    let elapsed = t0.elapsed().as_secs_f64();
    let ops: u64 = per_thread.iter().map(|p| p.0).sum();
    let ck = per_thread.iter().fold(0u64, |h, p| mix64(h ^ p.2));
    Ok(json!({
        "kernel": kernel, "stub": true, "threads": threads,
        "ops_per_sec": ops as f64 / elapsed, "ops": ops, "elapsed_s": elapsed,
        "reps": per_thread.iter().map(|p| p.1).min().unwrap_or(0),
        "checksum": format!("{ck:016x}"),
        "reps_consistent": per_thread.iter().all(|p| p.3),
        "cpus_seen": per_thread.iter().map(|p| p.4).collect::<Vec<_>>(),
    }))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn get(cfg: &str) -> Value {
        serde_json::from_str(&run(cfg)).unwrap()
    }

    #[test]
    fn list_has_plan() {
        let l = get(r#"{"list":true}"#);
        assert!(l["plan"].as_array().unwrap().len() >= 2);
    }

    #[test]
    fn checksums_repeat_across_runs_and_threads_keep_contract() {
        for k in ["stub:heat", "stub:mix"] {
            let a = get(&format!(r#"{{"kernel":"{k}","threads":1,"seconds":0.05}}"#));
            let b = get(&format!(r#"{{"kernel":"{k}","threads":1,"seconds":0.05}}"#));
            assert_eq!(a["checksum"], b["checksum"], "{k}: same work, same checksum (X11)");
            assert_eq!(a["reps_consistent"], true);
            for key in ["ops_per_sec", "ops", "elapsed_s", "checksum"] {
                assert!(a.get(key).is_some(), "contract key {key}");
            }
            let c = get(&format!(r#"{{"kernel":"{k}","threads":3,"seconds":0.05,"cpus":[0]}}"#));
            assert_eq!(c["threads"], 3);
        }
    }

    #[test]
    fn errors_come_back_as_json() {
        assert!(get(r#"{"kernel":"nope"}"#)["error"].is_string());
        assert!(get("not json")["error"].is_string());
    }
}
