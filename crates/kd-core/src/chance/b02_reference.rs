//! B02's original `Stream::new` and `WySafe::draw`, unchanged, for the port test (A3.3, A2.9).
//! From `pretests/b01-b02-numbers-random/kbench/src/rng.rs`.
#![allow(dead_code, clippy::all)]

pub const GOLDEN: u64 = 0x9e37_79b9_7f4a_7c15;

#[inline(always)]
pub fn mix64(mut z: u64) -> u64 {
    z = (z ^ (z >> 30)).wrapping_mul(0xbf58_476d_1ce4_e5b9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94d0_49bb_1331_11eb);
    z ^ (z >> 31)
}

pub fn retry_purpose(purpose: u32) -> u32 {
    purpose ^ 0x8000_0000
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Stream {
    pub k0: u64,
    pub k1: u64,
    pub sq_key: u64,
    pub wy_seed: u64,
    pub chacha_key: [u32; 8],
}

impl Stream {
    pub fn new(world: u64, system: u32, purpose: u32) -> Stream {
        let mut h = mix64(world.wrapping_add(GOLDEN));
        h = mix64(h ^ (system as u64).wrapping_add(GOLDEN.wrapping_mul(2)));
        h = mix64(h ^ (purpose as u64).wrapping_add(GOLDEN.wrapping_mul(3)));
        let k0 = h;
        let k1 = mix64(k0.wrapping_add(GOLDEN));
        let sq_key = squares_key(mix64(k1.wrapping_add(GOLDEN)));
        let wy_seed = k0 ^ wymix(k0 ^ WY_P0, WY_P1);
        // ChaCha8 takes the raw key fields as its 256-bit key: no hashing needed.
        let chacha_key = [world as u32, (world >> 32) as u32, system, purpose, 0, 0, 0, 0];
        Stream {
            k0,
            k1,
            sq_key,
            wy_seed,
            chacha_key,
        }
    }
}

pub fn squares_key(seed: u64) -> u64 {
    let mut s = seed;
    let mut key: u64 = 0;
    for half in 0..2 {
        let mut d: [u8; 15] = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15];
        for i in 0..8 {
            s = s.wrapping_add(GOLDEN);
            let j = i + (mix64(s) % (15 - i) as u64) as usize;
            d.swap(i, j);
        }
        if half == 1 && d[7] % 2 == 0 {
            let i = (0..7).find(|&i| d[i] % 2 == 1).unwrap();
            d.swap(i, 7);
        }
        let mut h: u64 = 0;
        for &x in d.iter().take(8) {
            h = (h << 4) | x as u64;
        }
        key = (key << 32) | h;
    }
    key
}

pub const WY_P0: u64 = 0xa076_1d64_78bd_642f;
pub const WY_P1: u64 = 0xe703_7ed1_a0b4_28db;

#[inline(always)]
fn wymum(a: u64, b: u64) -> (u64, u64) {
    let r = (a as u128) * (b as u128);
    (r as u64, (r >> 64) as u64)
}
#[inline(always)]
pub fn wymix(a: u64, b: u64) -> u64 {
    let (lo, hi) = wymum(a, b);
    lo ^ hi
}

/// Plain wy, kept to show the zero-product case the guard removes.
pub fn wy_draw(seed: u64, being: u64, moment: u64) -> u64 {
    let (a, b) = wymum(being ^ WY_P1, moment ^ seed);
    wymix(a ^ WY_P0 ^ 16, b ^ WY_P1)
}

#[derive(Clone, Copy)]
pub struct WySafe {
    seed: u64,
}
impl WySafe {
    pub fn new(s: &Stream) -> Self {
        WySafe { seed: s.wy_seed }
    }
}
#[inline(always)]
fn wymum_safe(a: u64, b: u64) -> (u64, u64) {
    let (lo, hi) = wymum(a, b);
    (a ^ lo, b ^ hi)
}
impl WySafe {
    #[inline(always)]
    pub fn draw(&self, being: u64, moment: u64) -> u64 {
        let (a, b) = wymum_safe(being ^ WY_P1, moment ^ self.seed);
        let (c, d) = wymum_safe(a ^ WY_P0 ^ 16, b ^ WY_P1);
        c ^ d
    }
}
