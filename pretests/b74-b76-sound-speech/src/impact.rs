//! B74 impacts (SND-06, MAT-03): a strike on an object, made from the object's material
//! properties and shape. One general rule for every material and shape (PRN-07); nothing
//! here names flint or wood except the stand-in values at the top (PRN-05: estimates).
//!
//! - A1 modal: the object's ringing modes from beam and plate theory; a Hertz contact sets
//!   how long the strike lasts, so how bright it is; damping sets how long each mode rings.
//! - A2 shaped noise: a noise burst with the same brightness, pitch region and decay.

use crate::dsp::{contact_spectrum, Biquad, Noise, Rng, PI, TAU};

pub const MAXM: usize = 32; // modes per voice, a multiple of LANES
pub const LANES: usize = 8;
pub const CLICK_MAX: usize = 512;
const C_AIR: f64 = 343.0;
const RHO_AIR: f64 = 1.2;

#[derive(Clone, Copy, Debug)]
pub struct Material {
    pub name: &'static str,
    /// Young's modulus for bending (along the grain for wood and bone), Pa.
    pub e: f64,
    /// Modulus across the surface, used for the contact, Pa.
    pub e_contact: f64,
    pub rho: f64,
    pub nu: f64,
    /// Loss factor: the fraction of energy lost per cycle, over 2 pi.
    pub eta: f64,
    /// 0 to 1: grains crushing and fibres rubbing at the contact.
    pub crunch: f64,
}

// Stand-in values (PRN-05): plausible, not sourced.
pub const FLINT: Material = Material { name: "flint", e: 70e9, e_contact: 70e9, rho: 2600.0, nu: 0.15, eta: 0.0006, crunch: 0.05 };
pub const GRANITE: Material = Material { name: "granite", e: 50e9, e_contact: 50e9, rho: 2650.0, nu: 0.25, eta: 0.006, crunch: 0.6 };
pub const DRY_WOOD: Material = Material { name: "dry wood", e: 12e9, e_contact: 1.0e9, rho: 600.0, nu: 0.35, eta: 0.009, crunch: 0.2 };
pub const BONE: Material = Material { name: "bone", e: 18e9, e_contact: 10e9, rho: 1900.0, nu: 0.3, eta: 0.012, crunch: 0.1 };
pub const QUARTZITE: Material = Material { name: "quartzite", e: 70e9, e_contact: 70e9, rho: 2650.0, nu: 0.15, eta: 0.002, crunch: 0.1 };

#[derive(Clone, Copy, Debug)]
pub enum Shape {
    /// Bars, slabs and plates: length >= width >= thickness, metres.
    Slab { l: f64, w: f64, h: f64 },
    /// Sticks and long bones; r_in > 0 is hollow.
    Rod { l: f64, r_out: f64, r_in: f64 },
}

impl Shape {
    pub fn volume(&self) -> f64 {
        match *self {
            Shape::Slab { l, w, h } => l * w * h,
            Shape::Rod { l, r_out, r_in } => PI * (r_out * r_out - r_in * r_in) * l,
        }
    }
    /// Area that pushes on the air.
    pub fn area(&self) -> f64 {
        match *self {
            Shape::Slab { l, w, .. } => 2.0 * l * w,
            Shape::Rod { l, r_out, .. } => TAU * r_out * l,
        }
    }
    /// Width across which the two faces' sound partly cancels.
    pub fn width(&self) -> f64 {
        match *self {
            Shape::Slab { w, .. } => w,
            Shape::Rod { r_out, .. } => 2.0 * r_out,
        }
    }
}

#[derive(Clone, Copy, Debug)]
pub struct Object {
    pub mat: Material,
    pub shape: Shape,
    /// Extra decay from what holds it (a hand, the ground), 1/s.
    pub support: f64,
    /// Local radius of the surface where it is struck, m (large = flat).
    pub contact_r: f64,
}

impl Object {
    pub fn mass(&self) -> f64 {
        self.mat.rho * self.shape.volume()
    }
}

#[derive(Clone, Copy, Debug)]
pub struct Striker {
    pub mat: Material,
    pub mass: f64,
    pub radius: f64,
    pub speed: f64,
    pub restitution: f64,
}

pub const HAMMERSTONE: Striker = Striker { mat: QUARTZITE, mass: 0.35, radius: 0.035, speed: 2.5, restitution: 0.5 };

// The four test objects (stand-ins): a flint slab being worked, a granite anvil stone on
// the ground, a dry stick held in one hand, a deer long bone held in one hand.
pub const FLINT_SLAB: Object = Object { mat: FLINT, shape: Shape::Slab { l: 0.16, w: 0.09, h: 0.02 }, support: 5.0, contact_r: 0.004 };
pub const GRANITE_ANVIL: Object = Object { mat: GRANITE, shape: Shape::Slab { l: 0.30, w: 0.20, h: 0.06 }, support: 40.0, contact_r: 0.5 };
pub const DRY_STICK: Object = Object { mat: DRY_WOOD, shape: Shape::Rod { l: 0.5, r_out: 0.015, r_in: 0.0 }, support: 6.0, contact_r: 0.015 };
pub const LONG_BONE: Object = Object { mat: BONE, shape: Shape::Rod { l: 0.24, r_out: 0.012, r_in: 0.007 }, support: 6.0, contact_r: 0.012 };

#[derive(Clone, Copy, Debug, Default)]
pub struct Mode {
    pub freq: f64,
    /// Pressure amplitude at 1 m, Pa (relative scale shared by all objects).
    pub amp: f64,
    /// Amplitude decay rate, 1/s.
    pub decay: f64,
}

/// Hertz contact: duration (s) and impulse (N s) for a strike at `speed` m/s.
pub fn contact(obj: &Object, s: &Striker, speed: f64) -> (f64, f64) {
    let e_star = 1.0 / ((1.0 - obj.mat.nu.powi(2)) / obj.mat.e_contact + (1.0 - s.mat.nu.powi(2)) / s.mat.e_contact);
    let r_star = 1.0 / (1.0 / obj.contact_r + 1.0 / s.radius);
    let m = obj.mass();
    let m_star = m * s.mass / (m + s.mass);
    let tau = 2.87 * (m_star * m_star / (r_star * e_star * e_star * speed)).powf(0.2);
    (tau, (1.0 + s.restitution) * m_star * speed)
}

/// Free-free beam: beta*L for the first bending modes.
const BETA: [f64; 6] = [4.7300, 7.8532, 10.9956, 14.1372, 17.2788, 20.4204];

/// Frequencies and excitation weights of an object's modes, from its shape and material.
pub fn mode_freqs(obj: &Object) -> Vec<(f64, f64)> {
    let m = &obj.mat;
    let mut out = Vec::new();
    let beam = |b: f64, len: f64| b * b / (TAU * len * len);
    match obj.shape {
        Shape::Slab { l, w, h } => {
            let plate = w >= 3.0 * h;
            let e_eff = if plate { m.e / (1.0 - m.nu * m.nu) } else { m.e };
            let k = h * (e_eff / (12.0 * m.rho)).sqrt();
            for b in BETA {
                out.push((beam(b, l) * k, 1.0)); // bending along the length
            }
            if w >= 2.0 * h {
                for b in &BETA[..4] {
                    out.push((beam(*b, w) * k, 0.6)); // bending across the width
                }
            }
            let cs = (m.e / (2.0 * (1.0 + m.nu)) / m.rho).sqrt();
            for n in 1..=3 {
                out.push((n as f64 / (2.0 * l) * cs * 2.0 * h / (w * w + h * h).sqrt(), 0.5)); // twisting
            }
            let k2 = w * (m.e / (12.0 * m.rho)).sqrt();
            for b in &BETA[..3] {
                out.push((beam(*b, l) * k2, 0.15)); // edgewise bending
            }
            for n in 1..=2 {
                out.push((n as f64 * (m.e / m.rho).sqrt() / (2.0 * l), 0.1)); // along the length
            }
        }
        Shape::Rod { l, r_out, r_in } => {
            let k = (m.e * (r_out * r_out + r_in * r_in) / (4.0 * m.rho)).sqrt();
            for b in BETA {
                let f = beam(b, l) * k;
                out.push((f * 0.99, 1.0)); // two bending planes: real sticks and bones aren't round
                out.push((f * 1.012, 0.7));
            }
            for n in 1..=2 {
                out.push((n as f64 * (m.e / m.rho).sqrt() / (2.0 * l), 0.1));
            }
        }
    }
    out
}

/// A1: the object's modes before the contact: amplitude per unit impulse with a perfectly
/// short contact. Computed once per object; `strike_amp` applies a strike to it.
pub fn object_modes(obj: &Object, fmax: f64) -> Vec<Mode> {
    let mass = obj.mass();
    let (area, d) = (obj.shape.area(), obj.shape.width());
    let mut v: Vec<Mode> = mode_freqs(obj)
        .into_iter()
        .filter(|&(f, _)| f < fmax)
        .map(|(f, weight)| {
            let omega = TAU * f;
            let kd = omega / C_AIR * d;
            let radiation = kd * kd / (1.0 + kd * kd); // small or narrow things radiate low notes poorly
            let accel = omega * 4.0 / mass; // struck near an end, where every mode moves
            Mode {
                freq: f,
                amp: RHO_AIR / TAU * area * radiation * accel * weight,
                decay: PI * f * obj.mat.eta + obj.support,
            }
        })
        .collect();
    v.sort_by(|a, b| b.amp.partial_cmp(&a.amp).unwrap());
    v.truncate(MAXM);
    v
}

/// One mode's amplitude for a strike with this contact duration and impulse.
#[inline]
pub fn strike_amp(m: &Mode, tau: f64, impulse: f64) -> f64 {
    m.amp * impulse * contact_spectrum(m.freq, tau)
}

/// A1: the modes a strike excites, strongest first.
pub fn strike_modes(obj: &Object, tau: f64, impulse: f64, fmax: f64) -> Vec<Mode> {
    let mut v: Vec<Mode> = object_modes(obj, fmax).iter().map(|m| Mode { amp: strike_amp(m, tau, impulse), ..*m }).collect();
    v.sort_by(|a, b| b.amp.partial_cmp(&a.amp).unwrap());
    v
}

/// The contact itself: both bodies jolt during the strike (a click as long as the
/// contact), plus crunch noise for grainy materials. Pressure at 1 m, same scale as modes.
pub fn strike_click(obj: &Object, s: &Striker, tau: f64, impulse: f64, sr: f64, rng: &mut Rng, out: &mut [f32; CLICK_MAX]) -> usize {
    let f_max = PI * impulse / (2.0 * tau);
    let k = RHO_AIR / (2.0 * TAU * C_AIR) * (1.0 / obj.mat.rho + 1.0 / s.mat.rho);
    let n_click = ((tau * sr).ceil() as usize + 1).min(CLICK_MAX);
    let n_crunch = (((4.0 * tau + 0.003) * sr) as usize).min(CLICK_MAX);
    let len = n_click.max(n_crunch);
    out[..len].fill(0.0);
    let mut peak = 0.0f64;
    for (i, o) in out.iter_mut().enumerate().take(n_click) {
        let t = i as f64 / sr;
        if t <= tau {
            // d/dt of a half-sine force pulse
            let v = k * f_max * (PI / tau) * (PI * t / tau).cos();
            peak = peak.max(v.abs());
            *o = v as f32;
        }
    }
    // the click is band-limited by the sample rate: scale its peak to what a pulse of this
    // length can reach in one sample period
    let bl = (1.0 / (tau * sr)).min(1.0) as f32;
    for o in out.iter_mut().take(n_click) {
        *o *= bl;
    }
    let mut nz = Noise((rng.next_u64() as u32) | 1);
    let crunch = obj.mat.crunch * peak * bl as f64 * 0.5;
    for (i, o) in out.iter_mut().enumerate().take(n_crunch) {
        let env = (-(i as f64) / (n_crunch as f64 * 0.3)).exp();
        *o += (crunch * env) as f32 * nz.next();
    }
    len
}

/// A1 voice: a bank of decaying resonators (two-pole), eight modes at a time.
#[derive(Clone)]
pub struct ModalVoice {
    n: usize,
    c1: [f32; MAXM],
    c2: [f32; MAXM],
    y1: [f32; MAXM],
    y2: [f32; MAXM],
    end: [u32; MAXM], // sample at which each mode falls below the floor, latest first
    t: u32,
    total: u32,
    click: [f32; CLICK_MAX],
    click_len: usize,
}

impl ModalVoice {
    pub fn silent() -> Self {
        ModalVoice { n: 0, c1: [0.0; MAXM], c2: [0.0; MAXM], y1: [0.0; MAXM], y2: [0.0; MAXM], end: [0; MAXM], t: 0, total: 0, click: [0.0; CLICK_MAX], click_len: 0 }
    }

    /// Start ringing `modes` (amplitudes scaled by `scale`), each a decaying sine from zero.
    /// Modes below -80 dB of the strongest are dropped; each stops once it falls below that.
    pub fn start(&mut self, modes: &[Mode], scale: f64, sr: f64) {
        let top = modes.iter().map(|m| m.amp).fold(0.0, f64::max);
        let floor = top * 1e-4;
        // no allocation here: this runs on the audio thread
        let mut ms = [(0u32, Mode::default()); MAXM];
        let mut cnt = 0;
        for m in modes.iter().filter(|m| m.amp > floor && m.freq < 0.45 * sr) {
            if cnt == MAXM {
                break;
            }
            ms[cnt] = (((m.amp / floor).ln() / m.decay * sr).min(sr * 8.0) as u32, *m);
            cnt += 1;
        }
        let ms = &mut ms[..cnt];
        ms.sort_unstable_by(|a, b| b.0.cmp(&a.0));
        let (click, click_len) = (self.click, self.click_len);
        self.n = cnt;
        self.c1 = [0.0; MAXM];
        self.c2 = [0.0; MAXM];
        self.y1 = [0.0; MAXM];
        self.y2 = [0.0; MAXM];
        self.end = [0; MAXM];
        self.t = 0;
        self.click = click;
        self.click_len = click_len;
        for (k, (end, m)) in ms.iter().enumerate() {
            let w = TAU * m.freq / sr;
            let r = (-m.decay / sr).exp();
            let a = m.amp * scale;
            self.c1[k] = (2.0 * r * w.cos()) as f32;
            self.c2[k] = (r * r) as f32;
            // state so that the first output sample is a*sin(0) and the next a*r*sin(w)
            self.y1[k] = (-a * w.sin() / r) as f32;
            self.y2[k] = (-a * (2.0 * w).sin() / (r * r)) as f32;
            self.end[k] = *end;
        }
        self.total = ms.first().map(|x| x.0).unwrap_or(0).max(self.click_len as u32);
    }

    pub fn set_click(&mut self, click: &[f32], scale: f32) {
        self.click_len = click.len().min(CLICK_MAX);
        for (o, c) in self.click.iter_mut().zip(click) {
            *o = c * scale;
        }
    }

    pub fn duration_samples(&self) -> u32 {
        self.total
    }

    /// Add the next `out.len()` samples into `out`; returns false once silent.
    pub fn render_add(&mut self, out: &mut [f32]) -> bool {
        let len = out.len();
        let t = self.t as usize;
        if t < self.click_len {
            for (o, c) in out.iter_mut().zip(&self.click[t..self.click_len]) {
                *o += *c;
            }
        }
        let mut active = 0;
        while active < self.n && self.end[active] > self.t {
            active += 1;
        }
        let groups = active.div_ceil(LANES);
        for g in 0..groups {
            let b = g * LANES;
            let mut c1 = [0f32; LANES];
            let mut c2 = [0f32; LANES];
            let mut y1 = [0f32; LANES];
            let mut y2 = [0f32; LANES];
            c1.copy_from_slice(&self.c1[b..b + LANES]);
            c2.copy_from_slice(&self.c2[b..b + LANES]);
            y1.copy_from_slice(&self.y1[b..b + LANES]);
            y2.copy_from_slice(&self.y2[b..b + LANES]);
            for o in out.iter_mut() {
                for l in 0..LANES {
                    let y = c1[l] * y1[l] - c2[l] * y2[l];
                    y2[l] = y1[l];
                    y1[l] = y;
                }
                *o += ((y1[0] + y1[1]) + (y1[2] + y1[3])) + ((y1[4] + y1[5]) + (y1[6] + y1[7]));
            }
            self.y1[b..b + LANES].copy_from_slice(&y1);
            self.y2[b..b + LANES].copy_from_slice(&y2);
        }
        self.t += len as u32;
        self.t < self.total
    }
}

/// A2 voice: noise through a resonance at the object's strongest mode and a low-pass at the
/// contact's brightness, under an exponential decay; the same click starts it.
#[derive(Clone)]
pub struct NoiseVoice {
    noise: Noise,
    bp: Biquad,
    lp: Biquad,
    env: f32,
    env_mul: f32,
    t: u32,
    total: u32,
    click: [f32; CLICK_MAX],
    click_len: usize,
}

impl NoiseVoice {
    pub fn silent() -> Self {
        NoiseVoice { noise: Noise(1), bp: Biquad::default(), lp: Biquad::default(), env: 0.0, env_mul: 0.0, t: 0, total: 0, click: [0.0; CLICK_MAX], click_len: 0 }
    }

    /// `modes` from the same physics as A1: only the strongest mode's frequency and decay
    /// are used, with the contact's brightness. Q is capped so it stays noise.
    pub fn start(&mut self, modes: &[Mode], tau: f64, scale: f64, seed: u32, sr: f64) {
        let top = modes.iter().cloned().fold(Mode::default(), |a, m| if m.amp > a.amp { m } else { a });
        let q = (PI * top.freq / top.decay).clamp(1.0, 8.0);
        let f_bright = (1.0 / (PI * tau)).clamp(200.0, 0.45 * sr);
        let floor = 1e-4f64;
        self.noise = Noise(seed | 1);
        self.bp = Biquad::bandpass(top.freq.max(40.0), q, sr);
        self.lp = Biquad::lowpass(f_bright, 0.707, sr);
        // white noise spread over the band: scale so the band carries the mode's loudness
        let band = top.freq.max(40.0) / q;
        self.env = (top.amp * scale * (0.5 * sr / band).sqrt() * 1.22) as f32;
        self.env_mul = (-top.decay / sr).exp() as f32;
        self.t = 0;
        self.total = ((1.0 / floor).ln() / top.decay * sr).min(sr * 8.0) as u32;
        self.total = self.total.max(self.click_len as u32);
    }

    pub fn set_click(&mut self, click: &[f32], scale: f32) {
        self.click_len = click.len().min(CLICK_MAX);
        for (o, c) in self.click.iter_mut().zip(click) {
            *o = c * scale;
        }
    }

    pub fn render_add(&mut self, out: &mut [f32]) -> bool {
        let t = self.t as usize;
        if t < self.click_len {
            for (o, c) in out.iter_mut().zip(&self.click[t..self.click_len]) {
                *o += *c;
            }
        }
        for o in out.iter_mut() {
            let x = self.noise.next() * self.env;
            self.env *= self.env_mul;
            *o += self.lp.tick(self.bp.tick(x));
        }
        self.t += out.len() as u32;
        self.t < self.total
    }
}

/// Everything needed to strike `obj` once: the A1 modes and the click, for a given speed.
pub struct Strike {
    pub tau: f64,
    pub impulse: f64,
    pub modes: Vec<Mode>,
    pub click: [f32; CLICK_MAX],
    pub click_len: usize,
}

/// Prepare one strike. `rng` varies where it lands (each mode's share), as real strikes do.
pub fn prepare_strike(obj: &Object, s: &Striker, speed: f64, rng: &mut Rng, sr: f64) -> Strike {
    let (tau, impulse) = contact(obj, s, speed);
    let mut modes = strike_modes(obj, tau, impulse, (0.45 * sr).min(20_000.0));
    for m in modes.iter_mut() {
        m.amp *= rng.range(0.25, 1.0); // the strike point: each mode's shape there
    }
    let mut click = [0f32; CLICK_MAX];
    let click_len = strike_click(obj, s, tau, impulse, sr, rng, &mut click);
    Strike { tau, impulse, modes, click, click_len }
}
