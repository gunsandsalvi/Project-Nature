//! B74 phone side (SND-01, SND-08): the JNI entry for dev.kindling.pretests.Sound, and the
//! live test. The mixer plays a camp test pattern through AAudio (low-latency, float,
//! stereo); each callback's work time is recorded; the main thread reads the output delay.

use crate::mix::{Approach, Mixer};
use jni::objects::{JClass, JString};
use jni::sys::jstring;
use jni::JNIEnv;
use serde_json::{json, Value};
use std::cell::UnsafeCell;
use std::ffi::c_void;
use std::sync::atomic::{AtomicI32, AtomicUsize, Ordering};
use std::time::{Duration, Instant};

#[no_mangle]
pub extern "system" fn Java_dev_kindling_pretests_Sound_run<'l>(mut env: JNIEnv<'l>, _c: JClass<'l>, config: JString<'l>) -> jstring {
    let out = match env.get_string(&config) {
        Ok(s) => crate::run(&String::from(s)),
        Err(_) => r#"{"error":"could not read config string"}"#.to_string(),
    };
    env.new_string(out).map(|s| s.into_raw()).unwrap_or(std::ptr::null_mut())
}

// ---------- AAudio, from the NDK (API 26+; usage and content type API 28+) ----------
#[repr(C)]
pub struct AAudioStreamBuilder {
    _p: [u8; 0],
}
#[repr(C)]
pub struct AAudioStream {
    _p: [u8; 0],
}
type DataCallback = unsafe extern "C" fn(*mut AAudioStream, *mut c_void, *mut c_void, i32) -> i32;
type ErrorCallback = unsafe extern "C" fn(*mut AAudioStream, *mut c_void, i32);

#[link(name = "aaudio")]
extern "C" {
    fn AAudio_createStreamBuilder(builder: *mut *mut AAudioStreamBuilder) -> i32;
    fn AAudioStreamBuilder_setDirection(b: *mut AAudioStreamBuilder, d: i32);
    fn AAudioStreamBuilder_setSharingMode(b: *mut AAudioStreamBuilder, m: i32);
    fn AAudioStreamBuilder_setPerformanceMode(b: *mut AAudioStreamBuilder, m: i32);
    fn AAudioStreamBuilder_setFormat(b: *mut AAudioStreamBuilder, f: i32);
    fn AAudioStreamBuilder_setChannelCount(b: *mut AAudioStreamBuilder, c: i32);
    fn AAudioStreamBuilder_setUsage(b: *mut AAudioStreamBuilder, u: i32);
    fn AAudioStreamBuilder_setContentType(b: *mut AAudioStreamBuilder, c: i32);
    fn AAudioStreamBuilder_setDataCallback(b: *mut AAudioStreamBuilder, cb: DataCallback, user: *mut c_void);
    fn AAudioStreamBuilder_setErrorCallback(b: *mut AAudioStreamBuilder, cb: ErrorCallback, user: *mut c_void);
    fn AAudioStreamBuilder_openStream(b: *mut AAudioStreamBuilder, s: *mut *mut AAudioStream) -> i32;
    fn AAudioStreamBuilder_delete(b: *mut AAudioStreamBuilder) -> i32;
    fn AAudioStream_requestStart(s: *mut AAudioStream) -> i32;
    fn AAudioStream_requestStop(s: *mut AAudioStream) -> i32;
    fn AAudioStream_close(s: *mut AAudioStream) -> i32;
    fn AAudioStream_waitForStateChange(s: *mut AAudioStream, input: i32, next: *mut i32, timeout_ns: i64) -> i32;
    fn AAudioStream_getSampleRate(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getChannelCount(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getFormat(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getFramesPerBurst(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getBufferSizeInFrames(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getBufferCapacityInFrames(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getXRunCount(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getSharingMode(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getPerformanceMode(s: *mut AAudioStream) -> i32;
    fn AAudioStream_getFramesWritten(s: *mut AAudioStream) -> i64;
    fn AAudioStream_getTimestamp(s: *mut AAudioStream, clock: libc::clockid_t, frame: *mut i64, time_ns: *mut i64) -> i32;
    fn AAudio_convertResultToText(r: i32) -> *const libc::c_char;
}

const DIRECTION_OUTPUT: i32 = 0;
const FORMAT_FLOAT: i32 = 2;
const SHARING_EXCLUSIVE: i32 = 0;
const PERF_LOW_LATENCY: i32 = 12;
const USAGE_GAME: i32 = 14;
const CONTENT_SONIFICATION: i32 = 4;
const STATE_STOPPING: i32 = 9;
const CALLBACK_CONTINUE: i32 = 0;
const MAX_CALLBACKS: usize = 16_384; // per phase: 8 s of 1 ms callbacks with room to spare
const NO_PHASE: usize = usize::MAX;

fn text(r: i32) -> String {
    unsafe {
        let p = AAudio_convertResultToText(r);
        if p.is_null() {
            format!("{r}")
        } else {
            std::ffi::CStr::from_ptr(p).to_string_lossy().into_owned()
        }
    }
}

fn now_ns() -> i64 {
    unsafe {
        let mut ts: libc::timespec = std::mem::zeroed();
        libc::clock_gettime(libc::CLOCK_MONOTONIC, &mut ts);
        ts.tv_sec * 1_000_000_000 + ts.tv_nsec
    }
}

fn thread_cpu_ns() -> i64 {
    unsafe {
        let mut ts: libc::timespec = std::mem::zeroed();
        libc::clock_gettime(libc::CLOCK_THREAD_CPUTIME_ID, &mut ts);
        ts.tv_sec * 1_000_000_000 + ts.tv_nsec
    }
}

/// What one phase (one voice count) records on the audio thread.
struct Phase {
    work_ns: Vec<u32>,
    frames: Vec<u16>,
    cpu_ns: i64,
    total_frames: u64,
    cpus: [u32; 16],
}

/// Owned by the audio callback while the stream runs; read by the main thread after stop.
struct CallbackState {
    mixer: Mixer,
    voices: usize,
    base_gain: f32,
    channels: usize,
    stereo: Vec<f32>,
    phases: Vec<Phase>,
}

struct Shared {
    phase: AtomicUsize,
    want_voices: AtomicUsize,
    error: AtomicI32,
    st: UnsafeCell<CallbackState>,
}

unsafe extern "C" fn data_cb(_s: *mut AAudioStream, user: *mut c_void, data: *mut c_void, frames: i32) -> i32 {
    let sh = &*(user as *const Shared);
    let st = &mut *sh.st.get();
    let (t0, c0) = (now_ns(), thread_cpu_ns());
    let n = frames.max(0) as usize;
    let out = std::slice::from_raw_parts_mut(data as *mut f32, n * st.channels);
    let want = sh.want_voices.load(Ordering::Acquire);
    if want != st.voices {
        st.voices = want;
        st.mixer.set_voices(want);
        st.mixer.gain = st.base_gain * (8.0 / want.max(1) as f32).sqrt();
    }
    let p = sh.phase.load(Ordering::Acquire);
    if st.channels == 2 {
        st.mixer.render(out);
    } else if 2 * n <= st.stereo.len() {
        let buf = &mut st.stereo[..2 * n];
        st.mixer.render(buf);
        for (i, o) in out.chunks_mut(st.channels).enumerate() {
            let m = 0.5 * (buf[2 * i] + buf[2 * i + 1]);
            o.fill(m);
        }
    } else {
        out.fill(0.0);
    }
    if p < st.phases.len() {
        let ph = &mut st.phases[p];
        if ph.work_ns.len() < MAX_CALLBACKS {
            ph.work_ns.push((now_ns() - t0).clamp(0, u32::MAX as i64) as u32);
            ph.frames.push(n.min(u16::MAX as usize) as u16);
        }
        ph.cpu_ns += thread_cpu_ns() - c0;
        ph.total_frames += n as u64;
        let cpu = libc::sched_getcpu();
        if (0..16).contains(&cpu) {
            ph.cpus[cpu as usize] += 1;
        }
    }
    CALLBACK_CONTINUE
}

unsafe extern "C" fn error_cb(_s: *mut AAudioStream, user: *mut c_void, err: i32) {
    let sh = &*(user as *const Shared);
    sh.error.store(err, Ordering::Release);
}

fn pct(v: &mut [f64], q: f64) -> f64 {
    if v.is_empty() {
        return f64::NAN;
    }
    v.sort_by(|a, b| a.partial_cmp(b).unwrap());
    v[((v.len() - 1) as f64 * q).round() as usize]
}

/// Play the test pattern at each voice count; report delay, load and dropouts.
pub fn play_test(cfg: &Value, voices: &[usize], approach: Approach) -> Value {
    let seconds = cfg.get("seconds_each").and_then(Value::as_f64).unwrap_or(8.0).clamp(1.0, 30.0);
    let gain = cfg.get("gain").and_then(Value::as_f64).unwrap_or(0.5).clamp(0.0, 1.0) as f32;
    let exclusive = cfg.get("exclusive").and_then(Value::as_bool).unwrap_or(true);
    let max_v = voices.iter().copied().max().unwrap_or(8);
    let shared = Box::new(Shared {
        phase: AtomicUsize::new(NO_PHASE),
        want_voices: AtomicUsize::new(voices.first().copied().unwrap_or(8)),
        error: AtomicI32::new(0),
        st: UnsafeCell::new(CallbackState {
            mixer: Mixer::new(48_000.0, max_v, approach, 74),
            voices: 0,
            base_gain: gain,
            channels: 2,
            stereo: Vec::new(),
            phases: voices
                .iter()
                .map(|_| Phase { work_ns: Vec::with_capacity(MAX_CALLBACKS), frames: Vec::with_capacity(MAX_CALLBACKS), cpu_ns: 0, total_frames: 0, cpus: [0; 16] })
                .collect(),
        }),
    });
    let user = &*shared as *const Shared as *mut c_void;
    unsafe {
        let mut b: *mut AAudioStreamBuilder = std::ptr::null_mut();
        let r = AAudio_createStreamBuilder(&mut b);
        if r != 0 {
            return json!({"error": format!("createStreamBuilder: {}", text(r))});
        }
        AAudioStreamBuilder_setDirection(b, DIRECTION_OUTPUT);
        if exclusive {
            AAudioStreamBuilder_setSharingMode(b, SHARING_EXCLUSIVE);
        }
        AAudioStreamBuilder_setPerformanceMode(b, PERF_LOW_LATENCY);
        AAudioStreamBuilder_setFormat(b, FORMAT_FLOAT);
        AAudioStreamBuilder_setChannelCount(b, 2);
        AAudioStreamBuilder_setUsage(b, USAGE_GAME);
        AAudioStreamBuilder_setContentType(b, CONTENT_SONIFICATION);
        AAudioStreamBuilder_setDataCallback(b, data_cb, user);
        AAudioStreamBuilder_setErrorCallback(b, error_cb, user);
        let mut s: *mut AAudioStream = std::ptr::null_mut();
        let r = AAudioStreamBuilder_openStream(b, &mut s);
        AAudioStreamBuilder_delete(b);
        if r != 0 || s.is_null() {
            return json!({"error": format!("openStream: {}", text(r))});
        }
        let sr = AAudioStream_getSampleRate(s);
        let ch = AAudioStream_getChannelCount(s).max(1) as usize;
        let cap = AAudioStream_getBufferCapacityInFrames(s).max(0) as usize;
        {
            // before start the callback can't run, so this is the only user of the state
            let st = &mut *shared.st.get();
            if sr > 0 && sr != 48_000 {
                st.mixer = Mixer::new(sr as f64, max_v, approach, 74);
            }
            st.channels = ch;
            st.stereo = vec![0.0; 2 * cap.max(4096)];
        }
        let info = json!({
            "sample_rate": sr, "channels": ch, "format_float": AAudioStream_getFormat(s) == FORMAT_FLOAT,
            "frames_per_burst": AAudioStream_getFramesPerBurst(s), "buffer_frames": AAudioStream_getBufferSizeInFrames(s),
            "buffer_capacity": cap, "exclusive": AAudioStream_getSharingMode(s) == SHARING_EXCLUSIVE,
            "low_latency": AAudioStream_getPerformanceMode(s) == PERF_LOW_LATENCY,
        });
        let r = AAudioStream_requestStart(s);
        if r != 0 {
            AAudioStream_close(s);
            return json!({"error": format!("requestStart: {}", text(r)), "stream": info});
        }
        let sr_f = sr.max(1) as f64;
        std::thread::sleep(Duration::from_millis(1000)); // settle, unrecorded
        let mut phases_out = vec![];
        for (pi, &nv) in voices.iter().enumerate() {
            shared.want_voices.store(nv, Ordering::Release);
            std::thread::sleep(Duration::from_millis(300)); // let the new count take over
            let x0 = AAudioStream_getXRunCount(s);
            shared.phase.store(pi, Ordering::Release);
            let t0 = Instant::now();
            let mut lat_ms = vec![];
            while t0.elapsed().as_secs_f64() < seconds && shared.error.load(Ordering::Acquire) == 0 {
                let (mut frame, mut tns) = (0i64, 0i64);
                if AAudioStream_getTimestamp(s, libc::CLOCK_MONOTONIC, &mut frame, &mut tns) == 0 {
                    let written = AAudioStream_getFramesWritten(s);
                    let now = now_ns();
                    let at_speaker = tns + ((written - frame) as f64 * 1e9 / sr_f) as i64;
                    lat_ms.push((at_speaker - now) as f64 / 1e6);
                }
                std::thread::sleep(Duration::from_millis(50));
            }
            shared.phase.store(NO_PHASE, Ordering::Release);
            let x1 = AAudioStream_getXRunCount(s);
            phases_out.push((nv, x1 - x0, lat_ms));
        }
        AAudioStream_requestStop(s);
        let mut next = 0i32;
        AAudioStream_waitForStateChange(s, STATE_STOPPING, &mut next, 2_000_000_000);
        AAudioStream_close(s);
        // the stream is closed: the callback no longer runs, so its state can be read
        let st = &mut *shared.st.get();
        let mut out = vec![];
        for ((nv, xruns, mut lat), ph) in phases_out.into_iter().zip(st.phases.iter()) {
            let mut work: Vec<f64> = ph.work_ns.iter().map(|w| *w as f64 / 1000.0).collect();
            let budget: Vec<f64> = ph.frames.iter().map(|f| *f as f64 / sr_f * 1e6).collect();
            let mean_budget = budget.iter().sum::<f64>() / budget.len().max(1) as f64;
            let mean_work = work.iter().sum::<f64>() / work.len().max(1) as f64;
            let audio_s = ph.total_frames as f64 / sr_f;
            out.push(json!({
                "voices": nv, "callbacks": work.len(), "xruns": xruns,
                "work_us_mean": mean_work, "work_us_p50": pct(&mut work, 0.5), "work_us_p99": pct(&mut work, 0.99), "work_us_max": pct(&mut work, 1.0),
                "budget_us": mean_budget, "load_mean": mean_work / mean_budget, "load_p99": pct(&mut work, 0.99) / mean_budget,
                "thread_cpu_share": ph.cpu_ns as f64 / 1e9 / audio_s.max(1e-9),
                "latency_ms_p50": pct(&mut lat, 0.5), "latency_ms_min": pct(&mut lat, 0.0), "latency_ms_max": pct(&mut lat, 1.0), "latency_samples": lat.len(),
                "cpus": ph.cpus.iter().enumerate().filter(|(_, c)| **c > 0).map(|(i, c)| json!([i, c])).collect::<Vec<_>>(),
            }));
        }
        let err = shared.error.load(Ordering::Acquire);
        json!({"stream": info, "phases": out, "stream_error": if err != 0 { Value::from(text(err)) } else { Value::Null }})
    }
}
