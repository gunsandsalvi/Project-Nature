//! B74 pre-test (SND-01, SND-06, SND-08, CUL-10): sounds made from material properties and
//! shapes, and a mixer for many of them at once.
//!
//! Phone test entry: `run(config_json) -> result_json` (JSON in, JSON out, like the B01
//! kbench crate). It measures the offline mixing cost for 8, 32 and 128 voices, and on
//! Android (feature "android") plays the test pattern through AAudio, reporting the output
//! delay and the audio thread's load. See INTEGRATION.md.

pub mod dsp;
pub mod impact;
pub mod instrument;
pub mod mix;

#[cfg(all(feature = "android", target_os = "android"))]
pub mod android;

use serde_json::{json, Value};
use std::time::Instant;

pub fn run(config_json: &str) -> String {
    match std::panic::catch_unwind(|| run_inner(config_json)) {
        Ok(Ok(v)) => v.to_string(),
        Ok(Err(e)) => json!({ "error": e }).to_string(),
        Err(_) => json!({ "error": "panic" }).to_string(),
    }
}

/// CPU time used by this thread so far, seconds (wall time where unavailable).
pub fn thread_cpu_seconds() -> f64 {
    #[cfg(any(target_os = "linux", target_os = "android"))]
    unsafe {
        let mut ts: libc::timespec = std::mem::zeroed();
        libc::clock_gettime(libc::CLOCK_THREAD_CPUTIME_ID, &mut ts);
        ts.tv_sec as f64 + ts.tv_nsec as f64 * 1e-9
    }
    #[cfg(not(any(target_os = "linux", target_os = "android")))]
    {
        use std::sync::OnceLock;
        static T0: OnceLock<Instant> = OnceLock::new();
        T0.get_or_init(Instant::now).elapsed().as_secs_f64()
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

pub fn current_cpu() -> i32 {
    #[cfg(any(target_os = "linux", target_os = "android"))]
    unsafe {
        libc::sched_getcpu()
    }
    #[cfg(not(any(target_os = "linux", target_os = "android")))]
    {
        -1
    }
}

/// Mix `voices` always-sounding voices for `seconds` of audio on this thread; returns
/// (share of one core used, mean active voices per block). 1.0 means exactly real time.
pub fn offline_cost(voices: usize, seconds: f64, sr: f64, approach: mix::Approach, seed: u64) -> (f64, f64) {
    let mut m = mix::Mixer::new(sr, voices, approach, seed);
    m.set_voices(voices);
    let frames = 192; // a typical phone burst at 48 kHz
    let mut buf = vec![0f32; frames * 2];
    // warm-up, untimed
    for _ in 0..((0.2 * sr) as usize / frames) {
        m.render(&mut buf);
    }
    let (vb0, b0) = (m.voice_blocks, m.blocks);
    let total = (seconds * sr) as usize / frames;
    let c0 = thread_cpu_seconds();
    for _ in 0..total {
        m.render(&mut buf);
    }
    let cpu = thread_cpu_seconds() - c0;
    let audio = (total * frames) as f64 / sr;
    (cpu / audio, (m.voice_blocks - vb0) as f64 / (m.blocks - b0).max(1) as f64)
}

fn online_cpus() -> usize {
    std::thread::available_parallelism().map(|n| n.get()).unwrap_or(1)
}

fn run_inner(cfg: &str) -> Result<Value, String> {
    let v: Value = serde_json::from_str(cfg).map_err(|e| format!("bad json: {e}"))?;
    let t_start = Instant::now();
    let voices: Vec<usize> = v
        .get("voices")
        .and_then(Value::as_array)
        .map(|a| a.iter().filter_map(Value::as_u64).map(|x| x as usize).collect())
        .unwrap_or_else(|| vec![8, 32, 128]);
    let voices: Vec<usize> = voices.into_iter().map(|n| n.clamp(1, 512)).collect();
    let offline_seconds = v.get("offline_seconds").and_then(Value::as_f64).unwrap_or(1.0).clamp(0.05, 10.0);
    let approach = match v.get("approach").and_then(Value::as_str) {
        Some("noise") => mix::Approach::Noise,
        _ => mix::Approach::Modal,
    };
    let sr = 48_000.0;

    // 1. offline mixing cost on whatever core this thread runs on
    let offline: Vec<Value> = voices
        .iter()
        .map(|&n| {
            let (load, active) = offline_cost(n, offline_seconds, sr, approach, 7);
            json!({"voices": n, "core_share": load, "voices_per_core": n as f64 / load, "active": active})
        })
        .collect();

    // 2. the same, pinned to each core in turn (big and little cores differ)
    let cpus: Vec<usize> = v
        .get("cpus")
        .and_then(Value::as_array)
        .map(|a| a.iter().filter_map(Value::as_u64).map(|c| c as usize).collect())
        .unwrap_or_else(|| (0..online_cpus()).collect());
    let per_cpu_voices = v.get("per_cpu_voices").and_then(Value::as_u64).unwrap_or(32) as usize;
    let per_cpu_seconds = v.get("per_cpu_seconds").and_then(Value::as_f64).unwrap_or(0.5).clamp(0.05, 5.0);
    let per_cpu: Vec<Value> = if v.get("skip_per_cpu").and_then(Value::as_bool) == Some(true) {
        vec![]
    } else {
        std::thread::scope(|s| {
            s.spawn(|| {
                cpus.iter()
                    .map(|&c| {
                        let pinned = pin_current_thread(c);
                        let (load, _) = offline_cost(per_cpu_voices, per_cpu_seconds, sr, approach, 11);
                        json!({"cpu": c, "pinned": pinned, "seen_on": current_cpu(), "voices": per_cpu_voices, "voices_per_core": per_cpu_voices as f64 / load})
                    })
                    .collect()
            })
            .join()
            .unwrap_or_default()
        })
    };

    // 3. live playback through the phone's low-latency audio
    #[cfg(all(feature = "android", target_os = "android"))]
    let audio = android::play_test(&v, &voices, approach);
    #[cfg(not(all(feature = "android", target_os = "android")))]
    let audio = json!({"skipped": "not on Android"});

    Ok(json!({
        "block": "B74",
        "approach": format!("{approach:?}"),
        "sample_rate_offline": sr,
        "offline": offline,
        "per_cpu": per_cpu,
        "audio": audio,
        "elapsed_s": t_start.elapsed().as_secs_f64(),
    }))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn run_returns_json_and_errors_as_json() {
        let r: Value = serde_json::from_str(&run(r#"{"voices":[4],"offline_seconds":0.05,"skip_per_cpu":true}"#)).unwrap();
        assert!(r["offline"][0]["core_share"].as_f64().unwrap() > 0.0);
        let e: Value = serde_json::from_str(&run("nope")).unwrap();
        assert!(e["error"].is_string());
    }

    #[test]
    fn flute_notes_rise_as_holes_open() {
        let fl = instrument::flute_a();
        let notes: Vec<f64> = (0..=fl.holes.len()).map(|o| fl.note(o, 1)).collect();
        assert!(notes.windows(2).all(|w| w[1] > w[0]), "{notes:?}");
    }

    #[test]
    fn bessel_zeros_are_zeros() {
        assert!(instrument::bessel_j(0, 2.4048).abs() < 1e-4);
        assert!(instrument::bessel_j(2, 5.1356).abs() < 1e-4);
    }

    #[test]
    fn modal_voice_ends_and_stays_finite() {
        let mut rng = dsp::Rng::new(1);
        let s = impact::prepare_strike(&impact::FLINT_SLAB, &impact::HAMMERSTONE, 2.5, &mut rng, 48_000.0);
        let mut v = impact::ModalVoice::silent();
        v.set_click(&s.click[..s.click_len], 1.0);
        v.start(&s.modes, 1.0, 48_000.0);
        let mut buf = [0f32; 256];
        let mut blocks = 0;
        while v.render_add(&mut buf) {
            blocks += 1;
            assert!(buf.iter().all(|x| x.is_finite()));
            assert!(blocks < 48_000 * 10 / 256);
        }
    }
}
