//! B74 instruments (CUL-10, SND-06): a bone flute whose notes come from its bore and
//! finger holes, and a hide drum whose modes come from the membrane's size and tension.
//! Stand-in values (PRN-05).

use crate::dsp::{contact_spectrum, Biquad, Noise, Rng, PI, TAU};
use crate::impact::{Mode, MAXM};

const C_AIR_0: f64 = 331.3;
const RHO_AIR: f64 = 1.2;

/// A bone tube, open at both ends, blown across one end; holes measured from that end.
#[derive(Clone, Debug)]
pub struct Flute {
    pub length: f64,
    pub bore_r: f64,
    pub wall: f64,
    /// (distance from the blowing end, hole radius), metres, nearest the mouth first.
    pub holes: Vec<(f64, f64)>,
    pub temp_c: f64,
}

impl Flute {
    /// Speed of sound in the air inside (the world's temperature would set it).
    pub fn c(&self) -> f64 {
        C_AIR_0 * (1.0 + self.temp_c / 273.15).sqrt()
    }

    /// Acoustic length with the lowest `open` holes open (0 = all closed): the bore is cut
    /// short at the first open hole, plus a correction for the hole's size and wall
    /// thickness and for the open tube below it (open hole and tube below act as two
    /// air masses side by side).
    pub fn effective_length(&self, open: usize) -> f64 {
        let a = self.bore_r;
        let n = self.holes.len();
        let mut pos = self.length;
        let mut tail = 0.61 * a; // open far end
        for &(x, b) in self.holes[n - open.min(n)..].iter().rev() {
            let te = self.wall + 1.5 * b; // hole chimney plus its end corrections
            let hole = te * (a / b).powi(2); // as a length of bore
            let down = (pos - x) + tail;
            tail = hole * down / (hole + down);
            pos = x;
        }
        pos + tail + 0.8 * a // blowing end, partly covered by the lips
    }

    /// Sounding pitch in Hz: an open-open tube's first resonance (register 2 = overblown).
    pub fn note(&self, open: usize, register: u32) -> f64 {
        register as f64 * self.c() / (2.0 * self.effective_length(open))
    }

    /// Resonance sharpness from wall losses (narrow, rough bores are breathier).
    pub fn q(&self, f: f64) -> f64 {
        let a = self.bore_r;
        let k = TAU * f / self.c();
        let wall = 2.95e-5 * f.sqrt() / a;
        let radiation = (k * a).powi(2) / (4.0 * self.effective_length(0));
        k / (2.0 * (wall + radiation))
    }
}

/// A griffon-vulture-wing-sized bone, five holes spaced evenly by eye (stand-in).
pub fn flute_a() -> Flute {
    Flute { length: 0.22, bore_r: 0.0042, wall: 0.0015, holes: (0..5).map(|i| (0.095 + 0.022 * i as f64, 0.0028)).collect(), temp_c: 15.0 }
}

/// A longer, wider bone with four larger holes.
pub fn flute_b() -> Flute {
    Flute { length: 0.31, bore_r: 0.0058, wall: 0.0022, holes: (0..4).map(|i| (0.15 + 0.03 * i as f64, 0.0035)).collect(), temp_c: 15.0 }
}

const NH: usize = 10;

/// One blown note: harmonics of the bore's resonance plus breath noise shaped by the bore.
pub struct FluteVoice {
    rot: (f32, f32), // per-sample rotation of the fundamental
    z: (f32, f32),
    amps: [f32; NH],
    nh: usize,
    noise: Noise,
    bands: [Biquad; 3],
    hiss: Biquad,
    t: u32,
    attack: u32,
    hold: u32,
    total: u32,
    glide: f32,
    glide_mul: f32,
    level: f32,
    wobble: f32,
    wob_rng: Noise,
    sr: f32,
    f: f32,
}

impl FluteVoice {
    /// `f` and `q` come from the flute's shape (`Flute::note`, `Flute::q`).
    pub fn new(f: f64, q: f64, seconds: f64, breath: f64, seed: u64, sr: f64) -> Self {
        let mut rng = Rng::new(seed);
        let nh = ((10_000.0 / f) as usize).clamp(1, NH);
        let mut amps = [0f32; NH];
        for (h, a) in amps.iter_mut().enumerate().take(nh) {
            let n = (h + 1) as f64;
            // soft blowing: weak upper harmonics; harder blowing brightens
            *a = (n.powf(-(2.2 - breath)) * if h % 2 == 1 { 0.7 } else { 1.0 }) as f32;
        }
        let q = q.min(60.0);
        let bands = [Biquad::bandpass(f, q, sr), Biquad::bandpass(2.0 * f, q, sr), Biquad::bandpass(3.0 * f, q * 0.8, sr)];
        FluteVoice {
            rot: (0.0, 0.0),
            z: (1.0, 0.0),
            amps,
            nh,
            noise: Noise((rng.next_u64() as u32) | 1),
            bands,
            hiss: Biquad::highpass(3000.0, 0.7, sr),
            t: 0,
            attack: (0.05 * sr) as u32,
            hold: (seconds * sr) as u32,
            total: ((seconds + 0.25) * sr) as u32,
            glide: 0.965, // the jet settles: the note starts a little flat
            glide_mul: (0.01f64.ln() / (0.04 * sr)).exp() as f32,
            level: 0.0,
            wobble: 0.0,
            wob_rng: Noise((rng.next_u64() as u32) | 1),
            sr: sr as f32,
            f: f as f32,
        }
    }

    pub fn render_add(&mut self, out: &mut [f32]) -> bool {
        // pitch and breath change slowly: update the rotation once per block
        self.wobble = 0.995 * self.wobble + 0.002 * self.wob_rng.next();
        let gl = self.glide;
        self.glide = 1.0 - (1.0 - self.glide) * self.glide_mul.powi(out.len() as i32);
        let w = TAU as f32 * self.f * gl * (1.0 + self.wobble * 0.1) / self.sr;
        self.rot = (w.cos(), w.sin());
        let mag = (self.z.0 * self.z.0 + self.z.1 * self.z.1).sqrt();
        self.z = (self.z.0 / mag, self.z.1 / mag);
        for o in out.iter_mut() {
            let t = self.t;
            let target = if t < self.attack {
                t as f32 / self.attack as f32
            } else if t < self.hold {
                1.0
            } else {
                0.0
            };
            let rate = if t < self.hold { 0.002 } else { 0.0008 };
            self.level += (target - self.level) * rate;
            let breath = 1.0 + 0.08 * self.wobble * 50.0;
            self.z = (self.z.0 * self.rot.0 - self.z.1 * self.rot.1, self.z.0 * self.rot.1 + self.z.1 * self.rot.0);
            let (mut wr, mut wi) = self.z;
            let mut s = 0.0f32;
            for h in 0..self.nh {
                s += self.amps[h] * wi;
                let nr = wr * self.z.0 - wi * self.z.1;
                wi = wr * self.z.1 + wi * self.z.0;
                wr = nr;
            }
            let chiff = if t < self.attack { 3.0 } else { 1.0 };
            let n = self.noise.next();
            let air = self.bands[0].tick(n) * 0.5 + self.bands[1].tick(n) * 0.3 + self.bands[2].tick(n) * 0.2;
            let hiss = self.hiss.tick(n) * 0.02;
            *o += self.level * (0.25 * s * breath + chiff * (0.35 * air + hiss));
            self.t += 1;
        }
        self.t < self.total
    }
}

/// A frame drum: rawhide stretched over a hoop, open at the back.
#[derive(Clone, Copy, Debug)]
pub struct Drum {
    pub radius: f64,
    /// Membrane tension, N per metre of edge.
    pub tension: f64,
    /// Hide mass per area, kg/m2.
    pub density: f64,
    pub eta: f64,
}

pub const DRUM_SMALL_TIGHT: Drum = Drum { radius: 0.15, tension: 3500.0, density: 0.6, eta: 0.03 };
pub const DRUM_LARGE_SLACK: Drum = Drum { radius: 0.30, tension: 1200.0, density: 0.9, eta: 0.03 };

/// Zeros of the Bessel functions J_m: J_ZEROS[m][n-1].
const J_ZEROS: [[f64; 3]; 7] = [
    [2.4048, 5.5201, 8.6537],
    [3.8317, 7.0156, 10.1735],
    [5.1356, 8.4172, 11.6198],
    [6.3802, 9.7610, 13.0152],
    [7.5883, 11.0647, 14.3725],
    [8.7715, 12.3386, 15.7002],
    [9.9361, 13.5893, 17.0038],
];

/// Bessel function of the first kind, by its power series (fine for x below about 20).
pub fn bessel_j(m: u32, x: f64) -> f64 {
    let mut term = (x / 2.0).powi(m as i32) / (1..=m).map(|k| k as f64).product::<f64>();
    let mut sum = term;
    for k in 1..60 {
        term *= -(x * x / 4.0) / (k as f64 * (k + m) as f64);
        sum += term;
        if term.abs() < 1e-17 {
            break;
        }
    }
    sum
}

/// The drum's modes for a strike at `r_frac` of the radius by a beater whose contact lasts
/// `tau` seconds (a hand is soft and long, a stick short). Air on both faces adds mass,
/// lowering the low modes most; a frame drum's faces partly cancel at low notes.
pub fn drum_modes(d: &Drum, r_frac: f64, tau: f64, fmax: f64) -> Vec<Mode> {
    let c_air = 343.0;
    let mut v = Vec::new();
    for (m, zeros) in J_ZEROS.iter().enumerate() {
        for &j in zeros {
            let sigma = d.density + 2.0 * RHO_AIR * d.radius / j;
            let f = j / (TAU * d.radius) * (d.tension / sigma).sqrt();
            if f > fmax {
                continue;
            }
            let ka = TAU * f / c_air * d.radius;
            let order = 2 * (m as i32 + 1);
            let radiation = ka.powi(order) / (1.0 + ka.powi(order));
            let shape = bessel_j(m as u32, j * r_frac);
            let amp = TAU * f * shape.abs() * contact_spectrum(f, tau) * radiation;
            let decay = PI * f * d.eta + RHO_AIR * c_air * radiation / (2.0 * d.density);
            v.push(Mode { freq: f, amp, decay });
            if m > 0 {
                // the twin mode at right angles, split by uneven hide
                v.push(Mode { freq: f * 1.006, amp: amp * 0.3, decay });
            }
        }
    }
    v.sort_by(|a, b| b.amp.partial_cmp(&a.amp).unwrap());
    v.truncate(MAXM);
    v
}

pub const HAND_TAU: f64 = 0.003;
pub const STICK_TAU: f64 = 0.0006;
