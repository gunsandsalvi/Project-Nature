//! B74: small signal helpers shared by the sound models: keyed random draws, filters.

pub const PI: f64 = std::f64::consts::PI;
pub const TAU: f64 = 2.0 * PI;

/// Keyed draw (TIM-06 style: every draw comes from a key, so a sound repeats exactly).
pub fn mix64(mut x: u64) -> u64 {
    x = (x ^ (x >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
    x = (x ^ (x >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
    x ^ (x >> 31)
}

/// A small generator seeded from a key.
#[derive(Clone, Copy, Debug)]
pub struct Rng(pub u64);

impl Rng {
    pub fn new(key: u64) -> Self {
        Rng(mix64(key ^ 0x6A09_E667_F3BC_C909) | 1)
    }
    pub fn next_u64(&mut self) -> u64 {
        self.0 = self.0.wrapping_add(0x9E37_79B9_7F4A_7C15);
        mix64(self.0)
    }
    /// Uniform in [0, 1).
    pub fn unit(&mut self) -> f64 {
        (self.next_u64() >> 11) as f64 * (1.0 / (1u64 << 53) as f64)
    }
    pub fn range(&mut self, lo: f64, hi: f64) -> f64 {
        lo + (hi - lo) * self.unit()
    }
}

/// Cheap white noise for the audio thread, uniform in [-1, 1).
#[derive(Clone, Copy, Debug)]
pub struct Noise(pub u32);

impl Noise {
    #[inline(always)]
    pub fn next(&mut self) -> f32 {
        let mut x = self.0;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        self.0 = x;
        (x as i32) as f32 * (1.0 / 2_147_483_648.0)
    }
}

/// Second-order filter (RBJ cookbook forms), direct form I with f32 state.
#[derive(Clone, Copy, Debug, Default)]
pub struct Biquad {
    b0: f32,
    b1: f32,
    b2: f32,
    a1: f32,
    a2: f32,
    x1: f32,
    x2: f32,
    y1: f32,
    y2: f32,
}

impl Biquad {
    fn from(b0: f64, b1: f64, b2: f64, a0: f64, a1: f64, a2: f64) -> Self {
        Biquad {
            b0: (b0 / a0) as f32,
            b1: (b1 / a0) as f32,
            b2: (b2 / a0) as f32,
            a1: (a1 / a0) as f32,
            a2: (a2 / a0) as f32,
            ..Default::default()
        }
    }
    /// Band-pass, 0 dB peak gain.
    pub fn bandpass(f: f64, q: f64, sr: f64) -> Self {
        let w = TAU * (f / sr).min(0.49);
        let alpha = w.sin() / (2.0 * q);
        Self::from(alpha, 0.0, -alpha, 1.0 + alpha, -2.0 * w.cos(), 1.0 - alpha)
    }
    pub fn lowpass(f: f64, q: f64, sr: f64) -> Self {
        let w = TAU * (f / sr).min(0.49);
        let (c, alpha) = (w.cos(), w.sin() / (2.0 * q));
        Self::from((1.0 - c) / 2.0, 1.0 - c, (1.0 - c) / 2.0, 1.0 + alpha, -2.0 * c, 1.0 - alpha)
    }
    pub fn highpass(f: f64, q: f64, sr: f64) -> Self {
        let w = TAU * (f / sr).min(0.49);
        let (c, alpha) = (w.cos(), w.sin() / (2.0 * q));
        Self::from((1.0 + c) / 2.0, -(1.0 + c), (1.0 + c) / 2.0, 1.0 + alpha, -2.0 * c, 1.0 - alpha)
    }
    #[inline(always)]
    pub fn tick(&mut self, x: f32) -> f32 {
        let y = self.b0 * x + self.b1 * self.x1 + self.b2 * self.x2 - self.a1 * self.y1 - self.a2 * self.y2;
        self.x2 = self.x1;
        self.x1 = x;
        self.y2 = self.y1;
        self.y1 = y;
        y
    }
}

/// Hertz contact force spectrum, smoothed: flat below about 1/(2 tau), falling above.
/// A short contact (hard on hard) excites high modes; a long one (soft) only low ones.
pub fn contact_spectrum(f: f64, tau: f64) -> f64 {
    1.0 / (1.0 + (2.0 * f * tau).powi(2))
}
