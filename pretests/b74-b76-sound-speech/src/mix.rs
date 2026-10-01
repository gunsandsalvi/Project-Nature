//! B74 mixer (SND-01, SND-08): many sounds at once, each placed by distance and direction,
//! quieter and duller further away. The same code runs in the cloud benchmark and on the
//! phone. Nothing here allocates once the mixer is built, so it is safe on the audio thread.

use crate::dsp::{Rng, PI, TAU};
use crate::impact::*;
use crate::instrument::*;

pub const BLOCK: usize = 128;
/// Full scale corresponds to this many pascals at the listener.
const FULL_SCALE_PA: f64 = 20.0;
/// Stand-in loudness: a hand on a frame drum, and a bone flute, at 1 m (peak, Pa).
pub const DRUM_PEAK_PA: f64 = 3.0;
pub const FLUTE_PEAK_PA: f64 = 0.6;

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum Approach {
    Modal,
    Noise,
}

#[derive(Clone, Copy)]
enum Kind {
    Impact(usize),
    Drum(usize),
    Flute,
}

struct Slot {
    playing: u8, // 0 waiting, 1 modal, 2 noise, 3 flute
    modal: ModalVoice,
    noise: NoiseVoice,
    flute: Option<FluteVoice>,
    gain: f32,
    pl: f32,
    pr: f32,
    lp_g: f32,
    lp_y: f32,
    wait: u32,
}

pub struct Mixer {
    sr: f64,
    pub approach: Approach,
    rng: Rng,
    slots: Vec<Slot>,
    target: usize,
    /// Longest pause between one sound and the next in a slot, seconds (0 = always sounding).
    pub gap_max: f64,
    impacts: Vec<(Object, Vec<Mode>)>,
    drums: Vec<Vec<Mode>>,
    flute_notes: Vec<(f64, f64)>,
    kinds: Vec<(Kind, f64)>,
    scratch: [f32; BLOCK],
    pub gain: f32,
    pub triggers: u64,
    pub voice_blocks: u64,
    pub blocks: u64,
}

/// Scale a set of modes so their amplitudes add up to `peak`.
fn scale_to(modes: &mut [Mode], peak: f64) {
    let sum: f64 = modes.iter().map(|m| m.amp).sum();
    for m in modes.iter_mut() {
        m.amp *= peak / sum;
    }
}

impl Mixer {
    pub fn new(sr: f64, max_voices: usize, approach: Approach, seed: u64) -> Self {
        let fmax = (0.45 * sr).min(20_000.0);
        let impacts: Vec<(Object, Vec<Mode>)> = [FLINT_SLAB, GRANITE_ANVIL, DRY_STICK, LONG_BONE].iter().map(|o| (*o, object_modes(o, fmax))).collect();
        let drums: Vec<Vec<Mode>> = [DRUM_SMALL_TIGHT, DRUM_LARGE_SLACK]
            .iter()
            .map(|d| {
                let mut m = drum_modes(d, 0.6, HAND_TAU, fmax);
                scale_to(&mut m, DRUM_PEAK_PA);
                m
            })
            .collect();
        let fl = flute_a();
        let flute_notes = (0..=fl.holes.len()).map(|o| (fl.note(o, 1), fl.q(fl.note(o, 1)))).collect();
        // what a camp sounds like (stand-in shares): knapping, wood, bone, the anvil, drums, a flute
        let kinds = vec![
            (Kind::Impact(0), 0.35),
            (Kind::Impact(1), 0.15),
            (Kind::Impact(2), 0.2),
            (Kind::Impact(3), 0.1),
            (Kind::Drum(0), 0.08),
            (Kind::Drum(1), 0.06),
            (Kind::Flute, 0.06),
        ];
        let slots = (0..max_voices)
            .map(|_| Slot { playing: 0, modal: ModalVoice::silent(), noise: NoiseVoice::silent(), flute: None, gain: 0.0, pl: 0.0, pr: 0.0, lp_g: 1.0, lp_y: 0.0, wait: 0 })
            .collect();
        Mixer {
            sr,
            approach,
            rng: Rng::new(seed),
            slots,
            target: 0,
            gap_max: 0.0,
            impacts,
            drums,
            flute_notes,
            kinds,
            scratch: [0.0; BLOCK],
            gain: 1.0,
            triggers: 0,
            voice_blocks: 0,
            blocks: 0,
        }
    }

    /// How many slots are in use (up to the number built).
    pub fn set_voices(&mut self, n: usize) {
        let n = n.min(self.slots.len());
        for s in &mut self.slots[n..] {
            s.playing = 0;
            s.wait = 0;
        }
        self.target = n;
    }

    fn trigger(&mut self, i: usize) {
        let sr = self.sr;
        let total: f64 = self.kinds.iter().map(|k| k.1).sum();
        let mut pick = self.rng.unit() * total;
        let mut kind = self.kinds[0].0;
        for &(k, w) in &self.kinds {
            kind = k;
            if pick < w {
                break;
            }
            pick -= w;
        }
        // where it happens (SND-08): distance sets loudness and how muffled it is
        let dist = self.rng.range(1.5, 25.0);
        let ang = (self.rng.range(-1.0, 1.0) + 1.0) * PI / 4.0;
        let fc = 16_000.0 / (1.0 + dist / 6.0);
        let seed = self.rng.next_u64();
        let slot = &mut self.slots[i];
        slot.gain = (1.0 / (dist * FULL_SCALE_PA)) as f32;
        slot.pl = ang.cos() as f32;
        slot.pr = ang.sin() as f32;
        slot.lp_g = (1.0 - (-TAU * fc / sr).exp()) as f32;
        slot.lp_y = 0.0;
        match kind {
            Kind::Impact(k) => {
                let (obj, base) = &self.impacts[k];
                let speed = HAMMERSTONE.speed * self.rng.range(0.6, 1.4);
                let mut point = [1.0f64; MAXM];
                for p in point.iter_mut() {
                    *p = self.rng.range(0.25, 1.0);
                }
                let mut ms = [Mode::default(); MAXM];
                let n = base.len().min(MAXM);
                let (tau, imp) = strike_amps(&base[..n], obj, &HAMMERSTONE, speed, &point, &mut ms);
                let mut click = [0f32; CLICK_MAX];
                let cl = strike_click(obj, &HAMMERSTONE, tau, imp, sr, &mut self.rng, &mut click);
                match self.approach {
                    Approach::Modal => {
                        slot.modal.set_click(&click[..cl], 1.0);
                        slot.modal.start(&ms[..n], 1.0, sr);
                        slot.playing = 1;
                    }
                    Approach::Noise => {
                        slot.noise.set_click(&click[..cl], 1.0);
                        slot.noise.start(&ms[..n], tau, 1.0, seed as u32, sr);
                        slot.playing = 2;
                    }
                }
            }
            Kind::Drum(d) => {
                let strength = self.rng.range(0.5, 1.2);
                slot.modal.set_click(&[], 0.0);
                slot.modal.start(&self.drums[d], strength, sr);
                slot.playing = 1;
            }
            Kind::Flute => {
                let (f, q) = self.flute_notes[(self.rng.unit() * self.flute_notes.len() as f64) as usize];
                let secs = self.rng.range(0.3, 1.0);
                slot.flute = Some(FluteVoice::new(f, q, secs, 0.5, seed, sr));
                slot.gain *= FLUTE_PEAK_PA as f32 / 0.35;
                slot.playing = 3;
            }
        }
        self.triggers += 1;
    }

    /// Fill interleaved stereo `out` (overwrites it).
    pub fn render(&mut self, out: &mut [f32]) {
        let frames = out.len() / 2;
        out.fill(0.0);
        let mut done = 0;
        while done < frames {
            let n = (frames - done).min(BLOCK);
            for i in 0..self.target {
                if self.slots[i].playing == 0 {
                    if self.slots[i].wait as usize > n {
                        self.slots[i].wait -= n as u32;
                        continue;
                    }
                    self.trigger(i);
                }
                let buf = &mut self.scratch[..n];
                buf.fill(0.0);
                let slot = &mut self.slots[i];
                let alive = match slot.playing {
                    1 => slot.modal.render_add(buf),
                    2 => slot.noise.render_add(buf),
                    _ => slot.flute.as_mut().map(|f| f.render_add(buf)).unwrap_or(false),
                };
                let (g, lg, pl, pr) = (slot.gain * self.gain, slot.lp_g, slot.pl, slot.pr);
                let mut y = slot.lp_y;
                let o = &mut out[2 * done..2 * (done + n)];
                for (j, x) in buf.iter().enumerate() {
                    y += lg * (*x - y);
                    let v = y * g;
                    o[2 * j] += v * pl;
                    o[2 * j + 1] += v * pr;
                }
                slot.lp_y = y;
                self.voice_blocks += 1;
                if !alive {
                    slot.playing = 0;
                    slot.wait = (self.rng.unit() * self.gap_max * self.sr) as u32;
                }
            }
            self.blocks += 1;
            done += n;
        }
        // gentle limiter so a pile-up never clips harshly
        for v in out.iter_mut() {
            *v /= (1.0 + *v * *v).sqrt();
        }
    }
}
