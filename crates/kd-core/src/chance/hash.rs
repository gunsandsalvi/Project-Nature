//! B02's guarded wyhash, ported bit for bit from `pretests/b01-b02-numbers-random/kbench/src/rng.rs` (A3.3, A2.9).
//! Implements `TIM-16` and `RES-05` in part.

/// The golden ratio constant of splitmix64.
pub const GOLDEN: u64 = 0x9e37_79b9_7f4a_7c15;
/// wyhash's first prime (B02's `WY_P0`).
pub const P0: u64 = 0xa076_1d64_78bd_642f;
/// wyhash's second prime (B02's `WY_P1`).
pub const P1: u64 = 0xe703_7ed1_a0b4_28db;

/// SplitMix64 finaliser (Steele, Lea, Flood 2014; Stafford's mix13 constants).
#[inline(always)]
pub fn mix64(mut z: u64) -> u64 {
    z = (z ^ (z >> 30)).wrapping_mul(0xbf58_476d_1ce4_e5b9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94d0_49bb_1331_11eb);
    z ^ (z >> 31)
}

/// The full 128-bit product of two words, as (low, high).
#[inline(always)]
pub fn wymum(a: u64, b: u64) -> (u64, u64) {
    let r = (a as u128) * (b as u128);
    (r as u64, (r >> 64) as u64)
}

/// wyhash's guarded multiply: the inputs are xored back into the product halves, so no input makes every
/// subject draw the same (B02).
#[inline(always)]
pub fn wymum_safe(a: u64, b: u64) -> (u64, u64) {
    let (lo, hi) = wymum(a, b);
    (a ^ lo, b ^ hi)
}

/// A stream's seed: everything about a draw except subject and moment, folded once (A3.3; B02's `Stream::new`
/// reduced to its `wy_seed`).
pub fn stream_seed(world: u64, system: u32, purpose: u32) -> u64 {
    let mut h = mix64(world.wrapping_add(GOLDEN));
    h = mix64(h ^ (system as u64).wrapping_add(GOLDEN.wrapping_mul(2)));
    h = mix64(h ^ (purpose as u64).wrapping_add(GOLDEN.wrapping_mul(3)));
    let (lo, hi) = wymum(h ^ P0, P1);
    h ^ (lo ^ hi)
}

/// One keyed draw (A3.3; B02's `WySafe::draw`).
#[inline(always)]
pub fn draw(seed: u64, subject: u64, moment: u64) -> u64 {
    let (a, b) = wymum_safe(subject ^ P1, moment ^ seed);
    let (c, d) = wymum_safe(a ^ P0 ^ 16, b ^ P1);
    c ^ d
}
