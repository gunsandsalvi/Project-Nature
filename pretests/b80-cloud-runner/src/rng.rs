//! B80 stand-in for B02 (random draws). Chance is local (X11, TIM-06): every draw is computed
//! from a key (world seed, being, day, purpose). There is no generator state to save, and thread
//! timing cannot change a draw.

#[inline(always)]
pub fn splitmix64(mut z: u64) -> u64 {
    z = z.wrapping_add(0x9E37_79B9_7F4A_7C15);
    z = (z ^ (z >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
    z ^ (z >> 31)
}

/// Hash of the key (seed, being, day, purpose).
#[inline(always)]
pub fn key_hash(seed: u64, being: u64, day: u64, purpose: u64) -> u64 {
    let mut h = splitmix64(seed ^ 0x6B38_0B80_0000_0001);
    h = splitmix64(h ^ being);
    h = splitmix64(h ^ day.rotate_left(17));
    h ^ purpose.wrapping_mul(0xD1B5_4A32_D192_ED03)
}

/// One keyed draw: splitmix64 over the key hash.
#[inline(always)]
pub fn draw_u64(seed: u64, being: u64, day: u64, purpose: u64) -> u64 {
    splitmix64(key_hash(seed, being, day, purpose))
}

/// Uniform in [0, 1).
#[inline(always)]
pub fn draw_f64(seed: u64, being: u64, day: u64, purpose: u64) -> f64 {
    (draw_u64(seed, being, day, purpose) >> 11) as f64 * (1.0 / 9_007_199_254_740_992.0)
}
