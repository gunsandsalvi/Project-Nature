//! B74 host tool (SND-06, CUL-10). Subcommands:
//!   render clips DIR   write the listening clips as 16-bit WAV files
//!   render describe    pitch, brightness and ring time per material; flute and drum checks (JSON)
//!   render bench       synthesis cost per second of sound and voices per core (JSON)
use ksound::dsp::{Rng, PI, TAU};
use ksound::impact::*;
use ksound::instrument::*;
use ksound::mix::{Approach, Mixer, DRUM_PEAK_PA};
use ksound::{offline_cost, thread_cpu_seconds};
use serde_json::{json, Value};
use std::io::Write;

const SR: f64 = 48_000.0;

fn write_wav(path: &str, channels: u16, data: &[f32]) {
    let mut f = std::io::BufWriter::new(std::fs::File::create(path).expect("create wav"));
    let bytes = (data.len() * 2) as u32;
    let sr = SR as u32;
    f.write_all(b"RIFF").unwrap();
    f.write_all(&(36 + bytes).to_le_bytes()).unwrap();
    f.write_all(b"WAVEfmt ").unwrap();
    f.write_all(&16u32.to_le_bytes()).unwrap();
    f.write_all(&1u16.to_le_bytes()).unwrap();
    f.write_all(&channels.to_le_bytes()).unwrap();
    f.write_all(&sr.to_le_bytes()).unwrap();
    f.write_all(&(sr * 2 * channels as u32).to_le_bytes()).unwrap();
    f.write_all(&(2 * channels).to_le_bytes()).unwrap();
    f.write_all(&16u16.to_le_bytes()).unwrap();
    f.write_all(b"data").unwrap();
    f.write_all(&bytes.to_le_bytes()).unwrap();
    for x in data {
        f.write_all(&((x.clamp(-1.0, 1.0) * 32767.0) as i16).to_le_bytes()).unwrap();
    }
}

fn normalize(x: &mut [f32], peak_db: f32) {
    let p = x.iter().fold(0f32, |m, v| m.max(v.abs()));
    if p > 0.0 {
        let g = 10f32.powf(peak_db / 20.0) / p;
        x.iter_mut().for_each(|v| *v *= g);
    }
}

/// Add one strike on `obj` into `out` at sample `at`. The strike point comes from `key`, so
/// A1 and A2 get the same strikes.
fn strike_into(out: &mut [f32], at: usize, obj: &Object, speed: f64, key: u64, approach: Approach) -> f64 {
    let mut rng = Rng::new(key);
    let s = prepare_strike(obj, &HAMMERSTONE, speed, &mut rng, SR);
    let mut buf = vec![0f32; 256];
    let mut pos = at;
    match approach {
        Approach::Modal => {
            let mut v = ModalVoice::silent();
            v.set_click(&s.click[..s.click_len], 1.0);
            v.start(&s.modes, 1.0, SR);
            loop {
                buf.fill(0.0);
                let alive = v.render_add(&mut buf);
                let start = pos.min(out.len());
                for (o, b) in out[start..].iter_mut().zip(&buf) {
                    *o += *b;
                }
                pos += buf.len();
                if !alive || pos >= out.len() {
                    break;
                }
            }
        }
        Approach::Noise => {
            let mut v = NoiseVoice::silent();
            v.set_click(&s.click[..s.click_len], 1.0);
            v.start(&s.modes, s.tau, 1.0, key as u32, SR);
            loop {
                buf.fill(0.0);
                let alive = v.render_add(&mut buf);
                let start = pos.min(out.len());
                for (o, b) in out[start..].iter_mut().zip(&buf) {
                    *o += *b;
                }
                pos += buf.len();
                if !alive || pos >= out.len() {
                    break;
                }
            }
        }
    }
    s.tau
}

fn modal_into(out: &mut [f32], at: usize, modes: &[Mode], scale: f64) {
    let mut v = ModalVoice::silent();
    v.start(modes, scale, SR);
    let mut buf = vec![0f32; 256];
    let mut pos = at;
    loop {
        buf.fill(0.0);
        let alive = v.render_add(&mut buf);
        let start = pos.min(out.len());
                for (o, b) in out[start..].iter_mut().zip(&buf) {
            *o += *b;
        }
        pos += buf.len();
        if !alive || pos >= out.len() {
            break;
        }
    }
}

fn note_into(out: &mut [f32], at: usize, fl: &Flute, f: f64, secs: f64, key: u64) {
    let mut v = FluteVoice::new(f, fl.q(f), secs, 0.5, key, SR);
    let mut buf = vec![0f32; 128];
    let mut pos = at;
    loop {
        buf.fill(0.0);
        let alive = v.render_add(&mut buf);
        let start = pos.min(out.len());
                for (o, b) in out[start..].iter_mut().zip(&buf) {
            *o += *b;
        }
        pos += buf.len();
        if !alive || pos >= out.len() {
            break;
        }
    }
}

fn secs(t: f64) -> usize {
    (t * SR) as usize
}

const MATERIALS: [(&str, Object); 4] = [("flint", FLINT_SLAB), ("granite", GRANITE_ANVIL), ("wood", DRY_STICK), ("bone", LONG_BONE)];

fn drum_set(d: &Drum, r: f64, tau: f64) -> Vec<Mode> {
    let mut m = drum_modes(d, r, tau, 20_000.0);
    let sum: f64 = m.iter().map(|x| x.amp).sum();
    m.iter_mut().for_each(|x| x.amp *= DRUM_PEAK_PA / sum);
    m
}

/// Round 2: the same strike with the corrected radiation law (`drum_modes_v2`).
fn drum_set_v2(d: &Drum, r: f64, tau: f64) -> Vec<Mode> {
    let mut m = drum_modes_v2(d, r, tau, 20_000.0);
    let sum: f64 = m.iter().map(|x| x.amp).sum();
    m.iter_mut().for_each(|x| x.amp *= DRUM_PEAK_PA / sum);
    m
}

fn clips(dir: &str) {
    std::fs::create_dir_all(dir).unwrap();
    // 1. four materials, three strikes each, by A1 and A2 (same strikes)
    for (name, obj) in MATERIALS {
        for (tag, ap) in [("modal", Approach::Modal), ("noise", Approach::Noise)] {
            let mut out = vec![0f32; secs(4.2)];
            for (i, sp) in [1.0, 0.7, 1.3].iter().enumerate() {
                strike_into(&mut out, secs(0.1 + 1.3 * i as f64), &obj, HAMMERSTONE.speed * sp, 100 + i as u64, ap);
            }
            normalize(&mut out, -3.0);
            write_wav(&format!("{dir}/impact-{name}-{tag}.wav"), 1, &out);
        }
    }
    // 2. flint at three sizes, one shared level
    let sizes = [(0.06, 0.03, 0.005), (0.10, 0.05, 0.012), (0.22, 0.12, 0.03)];
    let mut out = vec![0f32; secs(5.0)];
    for (i, (l, w, h)) in sizes.iter().enumerate() {
        let obj = Object { shape: Shape::Slab { l: *l, w: *w, h: *h }, ..FLINT_SLAB };
        strike_into(&mut out, secs(0.1 + 1.6 * i as f64), &obj, HAMMERSTONE.speed, 200 + i as u64, Approach::Modal);
    }
    normalize(&mut out, -3.0);
    write_wav(&format!("{dir}/impact-flint-sizes.wav"), 1, &out);
    // 3. flutes: each hole in turn, the overblown note, then a little tune
    for (tag, fl) in [("a", flute_a()), ("b", flute_b())] {
        let n = fl.holes.len();
        let mut out = vec![0f32; secs(11.0)];
        let mut t = 0.1;
        for o in 0..=n {
            note_into(&mut out, secs(t), &fl, fl.note(o, 1), 0.5, 300 + o as u64);
            t += 0.68;
        }
        note_into(&mut out, secs(t), &fl, fl.note(0, 2), 0.7, 310);
        t += 1.1;
        let tune: [(usize, f64); 9] = [(1, 0.3), (2, 0.3), (3, 0.6), (2, 0.3), (1, 0.3), (0, 0.6), (2, 0.3), (n, 0.3), (1, 0.9)];
        for (k, (o, d)) in tune.iter().enumerate() {
            note_into(&mut out, secs(t), &fl, fl.note((*o).min(n), 1), *d, 320 + k as u64);
            t += d + 0.06;
        }
        out.truncate(secs(t + 0.4));
        normalize(&mut out, -3.0);
        write_wav(&format!("{dir}/flute-{tag}.wav"), 1, &out);
    }
    // 4. drums: two hand strikes at the centre, two near the edge, then a short rhythm;
    //    "-ring" is round 2's corrected radiation law
    for (tag, d, v2) in [("small", DRUM_SMALL_TIGHT, false), ("large", DRUM_LARGE_SLACK, false), ("small-ring", DRUM_SMALL_TIGHT, true), ("large-ring", DRUM_LARGE_SLACK, true)] {
        let set = if v2 { drum_set_v2 } else { drum_set };
        let mut out = vec![0f32; secs(6.0)];
        let centre = set(&d, 0.1, HAND_TAU);
        let edge = set(&d, 0.85, HAND_TAU);
        let stick = set(&d, 0.85, STICK_TAU);
        let mut t = 0.1;
        for set in [&centre, &centre, &edge, &edge] {
            modal_into(&mut out, secs(t), set, 1.0);
            t += 0.7;
        }
        for (k, set) in [&centre, &stick, &stick, &centre, &stick, &edge, &centre, &stick].iter().enumerate() {
            modal_into(&mut out, secs(t), set, if k % 2 == 0 { 1.0 } else { 0.6 });
            t += 0.25;
        }
        out.truncate(secs(t + 1.0));
        normalize(&mut out, -3.0);
        write_wav(&format!("{dir}/drum-{tag}.wav"), 1, &out);
    }
    // 5. a busy camp: 32 voices with pauses, placed around the listener (stereo)
    for (tag, ap) in [("modal", Approach::Modal), ("noise", Approach::Noise)] {
        let mut m = Mixer::new(SR, 32, ap, 2026);
        m.set_voices(32);
        m.gap_max = 2.5;
        let mut out = vec![0f32; secs(12.0) * 2];
        m.render(&mut out);
        normalize(&mut out, -3.0);
        write_wav(&format!("{dir}/camp-{tag}.wav"), 2, &out);
    }
}

// ---------- measurements ----------

fn fft(re: &mut [f64], im: &mut [f64]) {
    let n = re.len();
    let mut j = 0;
    for i in 1..n {
        let mut bit = n >> 1;
        while j & bit != 0 {
            j ^= bit;
            bit >>= 1;
        }
        j |= bit;
        if i < j {
            re.swap(i, j);
            im.swap(i, j);
        }
    }
    let mut len = 2;
    while len <= n {
        let w = -TAU / len as f64;
        for s in (0..n).step_by(len) {
            for k in 0..len / 2 {
                let (c, si) = ((w * k as f64).cos(), (w * k as f64).sin());
                let (a, b) = (s + k, s + k + len / 2);
                let (tr, ti) = (re[b] * c - im[b] * si, re[b] * si + im[b] * c);
                re[b] = re[a] - tr;
                im[b] = im[a] - ti;
                re[a] += tr;
                im[a] += ti;
            }
        }
        len <<= 1;
    }
}

/// Spectral centroid and strongest frequency of the first 8192 samples from `at`.
fn spectrum_stats(x: &[f32], at: usize) -> (f64, f64) {
    let n = 8192;
    let mut re: Vec<f64> = (0..n).map(|i| x.get(at + i).copied().unwrap_or(0.0) as f64 * (0.5 - 0.5 * (TAU * i as f64 / n as f64).cos())).collect();
    let mut im = vec![0.0; n];
    fft(&mut re, &mut im);
    let (mut num, mut den, mut best, mut bi) = (0.0, 0.0, 0.0, 0);
    for k in 1..n / 2 {
        let mag = (re[k] * re[k] + im[k] * im[k]).sqrt();
        let f = k as f64 * SR / n as f64;
        num += f * mag;
        den += mag;
        if mag > best {
            best = mag;
            bi = k;
        }
    }
    (num / den, bi as f64 * SR / n as f64)
}

/// Ring time: from the loudest 5 ms frame to 30 dB down, doubled (seconds).
fn t60(x: &[f32]) -> f64 {
    let fr = secs(0.005);
    let lv: Vec<f64> = x.chunks(fr).map(|c| 10.0 * (c.iter().map(|v| (*v as f64).powi(2)).sum::<f64>() / c.len() as f64 + 1e-30).log10()).collect();
    let (imax, vmax) = lv.iter().enumerate().fold((0, f64::MIN), |a, (i, v)| if *v > a.1 { (i, *v) } else { a });
    let i30 = lv[imax..].iter().position(|v| *v < vmax - 30.0).unwrap_or(lv.len() - imax);
    2.0 * i30 as f64 * 0.005
}

/// Pitch by a fine scan of the spectrum around `guess` (Hann window), in Hz.
fn pitch(x: &[f32], guess: f64) -> f64 {
    let n = x.len();
    let w: Vec<f64> = (0..n).map(|i| x[i] as f64 * (0.5 - 0.5 * (TAU * i as f64 / n as f64).cos())).collect();
    let mag = |f: f64| {
        let (mut a, mut b) = (0.0, 0.0);
        let dw = TAU * f / SR;
        for (i, v) in w.iter().enumerate() {
            a += v * (dw * i as f64).cos();
            b += v * (dw * i as f64).sin();
        }
        a * a + b * b
    };
    let (mut best_f, mut best) = (guess, 0.0);
    let mut c = -80.0;
    while c <= 80.0 {
        let f = guess * 2f64.powf(c / 1200.0);
        let m = mag(f);
        if m > best {
            best = m;
            best_f = f;
        }
        c += 0.25;
    }
    best_f
}

fn describe() -> Value {
    let mut mats = vec![];
    for (name, obj) in MATERIALS {
        let (tau, imp) = contact(&obj, &HAMMERSTONE, HAMMERSTONE.speed);
        let ones = [1.0; MAXM];
        let modes = strike_modes(&obj, &HAMMERSTONE, HAMMERSTONE.speed, &ones, 20_000.0);
        let mut lowest: Vec<f64> = modes.iter().map(|m| m.freq).collect();
        lowest.sort_by(|a, b| a.partial_cmp(b).unwrap());
        let mut per = serde_json::Map::new();
        for (tag, ap) in [("modal", Approach::Modal), ("noise", Approach::Noise)] {
            let mut out = vec![0f32; secs(3.0)];
            strike_into(&mut out, 0, &obj, HAMMERSTONE.speed, 999, ap);
            let (centroid, peak) = spectrum_stats(&out, 0);
            per.insert(tag.into(), json!({"centroid_hz": centroid.round(), "peak_hz": peak.round(), "ring_s": t60(&out)}));
        }
        mats.push(json!({
            "material": name, "mass_kg": obj.mass(), "contact_us": (tau * 1e6).round(), "impulse_ns": imp,
            "lowest_modes_hz": lowest.iter().take(4).map(|f| f.round()).collect::<Vec<_>>(),
            "strongest_mode_hz": modes[0].freq.round(), "strongest_ring_s": 6.91 / modes[0].decay,
            "measured": per,
        }));
    }
    let mut flutes = vec![];
    for (tag, fl) in [("a", flute_a()), ("b", flute_b())] {
        let mut notes = vec![];
        for (o, reg) in (0..=fl.holes.len()).map(|o| (o, 1)).chain([(0, 2)]) {
            let f = fl.note(o, reg);
            let mut out = vec![0f32; secs(1.6)];
            note_into(&mut out, 0, &fl, f, 1.4, 50 + o as u64);
            let seg = &out[secs(0.5)..secs(1.1)];
            let got = pitch(seg, f);
            notes.push(json!({"open_holes": o, "register": reg, "effective_length_mm": (fl.effective_length(o) * 1000.0 * 10.0).round() / 10.0,
                "computed_hz": (f * 10.0).round() / 10.0, "measured_hz": (got * 10.0).round() / 10.0, "error_cents": ((1200.0 * (got / f).log2()) * 10.0).round() / 10.0, "q": fl.q(f).round()}));
        }
        flutes.push(json!({"flute": tag, "length_mm": fl.length * 1000.0, "bore_mm": fl.bore_r * 2000.0, "notes": notes}));
    }
    let mut drums = vec![];
    for (tag, d) in [("small", DRUM_SMALL_TIGHT), ("large", DRUM_LARGE_SLACK)] {
        let mut m = drum_modes(&d, 0.6, HAND_TAU, 20_000.0);
        m.sort_by(|a, b| a.freq.partial_cmp(&b.freq).unwrap());
        let f0 = m[0].freq;
        // round 2: ring time (60 dB) of the six lowest notes under each law, and of a whole strike
        let mut m2 = drum_modes_v2(&d, 0.6, HAND_TAU, 20_000.0);
        m2.sort_by(|a, b| a.freq.partial_cmp(&b.freq).unwrap());
        let ring = |ms: &[Mode]| ms.iter().take(6).map(|x| (6.91 / x.decay * 1000.0).round() / 1000.0).collect::<Vec<_>>();
        let strike = |ms: &[Mode]| {
            let mut out = vec![0f32; secs(3.0)];
            modal_into(&mut out, 0, ms, 1.0);
            t60(&out)
        };
        drums.push(json!({"drum": tag, "radius_m": d.radius, "tension_n_per_m": d.tension,
            "lowest_modes_hz": m.iter().take(6).map(|x| x.freq.round()).collect::<Vec<_>>(),
            "ratios": m.iter().take(6).map(|x| (x.freq / f0 * 100.0).round() / 100.0).collect::<Vec<_>>(),
            "ideal_ratios_no_air": [1.0, 1.59, 2.14, 2.30, 2.65, 2.92],
            "ring_s_v1": ring(&m), "ring_s_v2": ring(&m2),
            "strike_ring_s_v1": strike(&drum_set(&d, 0.6, HAND_TAU)), "strike_ring_s_v2": strike(&drum_set_v2(&d, 0.6, HAND_TAU))}));
    }
    json!({"materials": mats, "flutes": flutes, "drums": drums})
}

/// Core-seconds per second of sound for one kind of voice, and voices per core.
fn bench() -> Value {
    let mut rows = vec![];
    for (name, obj) in MATERIALS {
        for (tag, ap) in [("modal", Approach::Modal), ("noise", Approach::Noise)] {
            let reps = 200;
            let mut audio = 0.0;
            let mut buf = vec![0f32; 192];
            let c0 = thread_cpu_seconds();
            for r in 0..reps {
                let mut rng = Rng::new(r as u64);
                let s = prepare_strike(&obj, &HAMMERSTONE, HAMMERSTONE.speed, &mut rng, SR);
                let mut n = 0usize;
                match ap {
                    Approach::Modal => {
                        let mut v = ModalVoice::silent();
                        v.set_click(&s.click[..s.click_len], 1.0);
                        v.start(&s.modes, 1.0, SR);
                        while v.render_add(&mut buf) {
                            n += buf.len();
                        }
                    }
                    Approach::Noise => {
                        let mut v = NoiseVoice::silent();
                        v.set_click(&s.click[..s.click_len], 1.0);
                        v.start(&s.modes, s.tau, 1.0, r, SR);
                        while v.render_add(&mut buf) {
                            n += buf.len();
                        }
                    }
                }
                audio += (n + buf.len()) as f64 / SR;
            }
            let cpu = thread_cpu_seconds() - c0;
            std::hint::black_box(&buf);
            rows.push(json!({"sound": name, "approach": tag, "sound_s_each": audio / reps as f64, "core_share_per_voice": cpu / audio, "voices_per_core": audio / cpu}));
        }
    }
    // instruments
    let fl = flute_a();
    let c0 = thread_cpu_seconds();
    let mut out = vec![0f32; secs(2.25)];
    for k in 0..20 {
        out.fill(0.0);
        note_into(&mut out, 0, &fl, fl.note(k % 6, 1), 2.0, k as u64);
    }
    let cpu = thread_cpu_seconds() - c0;
    std::hint::black_box(&out);
    rows.push(json!({"sound": "flute", "approach": "harmonics+noise", "core_share_per_voice": cpu / (20.0 * 2.25), "voices_per_core": 20.0 * 2.25 / cpu}));
    let c0 = thread_cpu_seconds();
    let set = drum_set(&DRUM_LARGE_SLACK, 0.6, HAND_TAU);
    let mut total = 0.0;
    for _ in 0..50 {
        let mut v = ModalVoice::silent();
        v.start(&set, 1.0, SR);
        let mut buf = vec![0f32; 192];
        let mut n = 192;
        while v.render_add(&mut buf) {
            n += 192;
        }
        total += n as f64 / SR;
        std::hint::black_box(&buf);
    }
    let cpu = thread_cpu_seconds() - c0;
    rows.push(json!({"sound": "drum", "approach": "modal", "core_share_per_voice": cpu / total, "voices_per_core": total / cpu}));
    // the mixer, with every voice always sounding (worst case)
    let mut mixes = vec![];
    for ap in [Approach::Modal, Approach::Noise] {
        for n in [8usize, 32, 128, 512] {
            let (load, active) = offline_cost(n, 3.0, SR, ap, 5);
            mixes.push(json!({"approach": format!("{ap:?}"), "voices": n, "core_share": load, "voices_per_core": n as f64 / load, "active": active}));
        }
    }
    json!({"sounds": rows, "mixer": mixes})
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    match args.get(1).map(String::as_str) {
        Some("clips") => clips(args.get(2).map(String::as_str).unwrap_or("clips")),
        Some("describe") => println!("{}", serde_json::to_string_pretty(&describe()).unwrap()),
        Some("bench") => println!("{}", bench()),
        _ => eprintln!("usage: render clips DIR | describe | bench"),
    }
    let _ = PI;
}
