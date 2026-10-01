//! B02: keyed random generators (TIM-06 chance is local, X6, X7, X11, GOD-04).
//!
//! Every draw is a pure function of its key (world, system, being, moment, purpose).
//! No generator keeps a position in a sequence, so threads, work order and skipped
//! draws never change any other draw (X11), and switching a mechanism off leaves the
//! others' draws untouched (X7).
//!
//! The key parts that stay fixed for a whole loop (world, system, purpose) are folded
//! once into a `Stream`; each draw then mixes in being and moment.
//! A fortune retry (GOD-04) is just another purpose (`retry_purpose`), so it gets its
//! own independent draw and disturbs nothing else.

pub const GOLDEN: u64 = 0x9e37_79b9_7f4a_7c15;

/// SplitMix64 finaliser (Steele, Lea, Flood 2014; Stafford's mix13 constants).
#[inline(always)]
pub fn mix64(mut z: u64) -> u64 {
    z = (z ^ (z >> 30)).wrapping_mul(0xbf58_476d_1ce4_e5b9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94d0_49bb_1331_11eb);
    z ^ (z >> 31)
}

/// The purpose used for the single fortune retry of a chance event (GOD-04).
pub fn retry_purpose(purpose: u32) -> u32 {
    purpose ^ 0x8000_0000
}

/// Stream key: everything about a draw except being and moment. Shared with C++ (same layout).
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
        let chacha_key = [
            world as u32,
            (world >> 32) as u32,
            system,
            purpose,
            0,
            0,
            0,
            0,
        ];
        Stream { k0, k1, sq_key, wy_seed, chacha_key }
    }
}

/// The six candidate generators.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(i32)]
pub enum Gen {
    SplitMix = 0,
    Philox = 1,
    Squares = 2,
    Pcg = 3,
    Wy = 4,
    ChaCha8 = 5,
}

pub const ALL_GENS: [Gen; 6] = [Gen::SplitMix, Gen::Philox, Gen::Squares, Gen::Pcg, Gen::Wy, Gen::ChaCha8];

impl Gen {
    pub fn name(self) -> &'static str {
        match self {
            Gen::SplitMix => "splitmix",
            Gen::Philox => "philox",
            Gen::Squares => "squares",
            Gen::Pcg => "pcg",
            Gen::Wy => "wy",
            Gen::ChaCha8 => "chacha8",
        }
    }
    pub fn from_name(s: &str) -> Option<Gen> {
        ALL_GENS.iter().copied().find(|g| g.name() == s)
    }
    pub fn about(self) -> &'static str {
        match self {
            Gen::SplitMix => "splitmix64: seed = mix(k0^being), output = mix(seed + (moment+1)*golden)",
            Gen::Philox => "Philox4x32-10: counter = (moment, being) 128 bits, key = k0 64 bits",
            Gen::Squares => "Squares64 (Widynski): key from stream, counter = mix(k1^being) + moment",
            Gen::Pcg => "PCG-style hash: LCG step + RXS-M-XS, applied as pcg(pcg(k0^being) + moment)",
            Gen::Wy => "wyhash-style: wyhash of the 16 bytes (being, moment) seeded by k0",
            Gen::ChaCha8 => "ChaCha8 block: key = (world, system, purpose), counter = moment, nonce = being",
        }
    }
}

/// One keyed generator bound to a stream: `draw(being, moment)` is a pure function.
pub trait KeyedGen: Copy + Send + Sync {
    fn draw(&self, being: u64, moment: u64) -> u64;
}

// ---------- splitmix64 over a hash of the key ----------
#[derive(Clone, Copy)]
pub struct SplitMix {
    k0: u64,
}
impl SplitMix {
    pub fn new(s: &Stream) -> Self {
        SplitMix { k0: s.k0 }
    }
}
impl KeyedGen for SplitMix {
    #[inline(always)]
    fn draw(&self, being: u64, moment: u64) -> u64 {
        let seed = mix64(self.k0 ^ being);
        mix64(seed.wrapping_add(moment.wrapping_add(1).wrapping_mul(GOLDEN)))
    }
}

// ---------- Philox4x32-10 (Salmon et al. 2011, Random123) ----------
const PH_M0: u32 = 0xD251_1F53;
const PH_M1: u32 = 0xCD9E_8D57;
const PH_W0: u32 = 0x9E37_79B9;
const PH_W1: u32 = 0xBB67_AE85;

#[inline(always)]
pub fn philox4x32_10(mut c: [u32; 4], k: [u32; 2]) -> [u32; 4] {
    let (mut k0, mut k1) = (k[0], k[1]);
    for r in 0..10 {
        if r > 0 {
            k0 = k0.wrapping_add(PH_W0);
            k1 = k1.wrapping_add(PH_W1);
        }
        let p0 = (PH_M0 as u64) * (c[0] as u64);
        let p1 = (PH_M1 as u64) * (c[2] as u64);
        let (hi0, lo0) = ((p0 >> 32) as u32, p0 as u32);
        let (hi1, lo1) = ((p1 >> 32) as u32, p1 as u32);
        c = [hi1 ^ c[1] ^ k0, lo1, hi0 ^ c[3] ^ k1, lo0];
    }
    c
}

#[derive(Clone, Copy)]
pub struct Philox {
    key: [u32; 2],
}
impl Philox {
    pub fn new(s: &Stream) -> Self {
        Philox { key: [s.k0 as u32, (s.k0 >> 32) as u32] }
    }
}
impl KeyedGen for Philox {
    #[inline(always)]
    fn draw(&self, being: u64, moment: u64) -> u64 {
        let c = [moment as u32, (moment >> 32) as u32, being as u32, (being >> 32) as u32];
        let o = philox4x32_10(c, self.key);
        (o[0] as u64) | ((o[1] as u64) << 32)
    }
}

// ---------- Squares64 (Widynski 2020) ----------
#[inline(always)]
pub fn squares64(ctr: u64, key: u64) -> u64 {
    let y = ctr.wrapping_mul(key);
    let mut x = y;
    let z = y.wrapping_add(key);
    x = x.wrapping_mul(x).wrapping_add(y);
    x = x.rotate_right(32); // round 1
    x = x.wrapping_mul(x).wrapping_add(z);
    x = x.rotate_right(32); // round 2
    x = x.wrapping_mul(x).wrapping_add(y);
    x = x.rotate_right(32); // round 3
    let t = x.wrapping_mul(x).wrapping_add(z);
    x = t.rotate_right(32); // round 4
    t ^ (x.wrapping_mul(x).wrapping_add(y) >> 32) // round 5
}

/// A Squares key in Widynski's style: in each 32-bit half, 8 distinct non-zero hex
/// digits; lowest digit odd.
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

#[derive(Clone, Copy)]
pub struct Squares {
    key: u64,
    k1: u64,
}
impl Squares {
    pub fn new(s: &Stream) -> Self {
        Squares { key: s.sq_key, k1: s.k1 }
    }
}
impl KeyedGen for Squares {
    #[inline(always)]
    fn draw(&self, being: u64, moment: u64) -> u64 {
        squares64(mix64(self.k1 ^ being).wrapping_add(moment), self.key)
    }
}

// ---------- PCG-style hash (O'Neill 2014 output function on one LCG step) ----------
#[inline(always)]
pub fn pcg_mix(x: u64) -> u64 {
    let s = x.wrapping_mul(6_364_136_223_846_793_005).wrapping_add(1_442_695_040_888_963_407);
    let w = ((s >> ((s >> 59) + 5)) ^ s).wrapping_mul(12_605_985_483_714_917_081);
    (w >> 43) ^ w
}

#[derive(Clone, Copy)]
pub struct Pcg {
    k0: u64,
}
impl Pcg {
    pub fn new(s: &Stream) -> Self {
        Pcg { k0: s.k0 }
    }
}
impl KeyedGen for Pcg {
    #[inline(always)]
    fn draw(&self, being: u64, moment: u64) -> u64 {
        pcg_mix(pcg_mix(self.k0 ^ being).wrapping_add(moment))
    }
}

// ---------- wyhash-style (Wang Yi) ----------
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

#[derive(Clone, Copy)]
pub struct Wy {
    seed: u64,
}
impl Wy {
    pub fn new(s: &Stream) -> Self {
        Wy { seed: s.wy_seed }
    }
}
impl KeyedGen for Wy {
    #[inline(always)]
    fn draw(&self, being: u64, moment: u64) -> u64 {
        // wyhash's final step for a 16-byte input (being, moment) with a pre-mixed seed.
        let (a, b) = wymum(being ^ WY_P1, moment ^ self.seed);
        wymix(a ^ WY_P0 ^ 16, b ^ WY_P1)
    }
}

// ---------- ChaCha8 (Bernstein 2008), one block per draw ----------
#[inline(always)]
fn qr(x: &mut [u32; 16], a: usize, b: usize, c: usize, d: usize) {
    x[a] = x[a].wrapping_add(x[b]);
    x[d] = (x[d] ^ x[a]).rotate_left(16);
    x[c] = x[c].wrapping_add(x[d]);
    x[b] = (x[b] ^ x[c]).rotate_left(12);
    x[a] = x[a].wrapping_add(x[b]);
    x[d] = (x[d] ^ x[a]).rotate_left(8);
    x[c] = x[c].wrapping_add(x[d]);
    x[b] = (x[b] ^ x[c]).rotate_left(7);
}

/// First 64 bits of the ChaCha8 block for (key, 64-bit counter, 64-bit nonce).
#[inline(always)]
pub fn chacha8_first64(key: &[u32; 8], counter: u64, nonce: u64) -> u64 {
    let s: [u32; 16] = [
        0x6170_7865, 0x3320_646e, 0x7962_2d32, 0x6b20_6574,
        key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
        counter as u32, (counter >> 32) as u32, nonce as u32, (nonce >> 32) as u32,
    ];
    let mut x = s;
    for _ in 0..4 {
        qr(&mut x, 0, 4, 8, 12);
        qr(&mut x, 1, 5, 9, 13);
        qr(&mut x, 2, 6, 10, 14);
        qr(&mut x, 3, 7, 11, 15);
        qr(&mut x, 0, 5, 10, 15);
        qr(&mut x, 1, 6, 11, 12);
        qr(&mut x, 2, 7, 8, 13);
        qr(&mut x, 3, 4, 9, 14);
    }
    (x[0].wrapping_add(s[0]) as u64) | ((x[1].wrapping_add(s[1]) as u64) << 32)
}

#[derive(Clone, Copy)]
pub struct ChaCha8 {
    key: [u32; 8],
}
impl ChaCha8 {
    pub fn new(s: &Stream) -> Self {
        ChaCha8 { key: s.chacha_key }
    }
}
impl KeyedGen for ChaCha8 {
    #[inline(always)]
    fn draw(&self, being: u64, moment: u64) -> u64 {
        chacha8_first64(&self.key, moment, being)
    }
}

/// Runs `$body` with `$v` bound to the concrete generator, so the hot loop is
/// compiled once per generator with no dynamic dispatch.
#[macro_export]
macro_rules! with_gen {
    ($gen:expr, $stream:expr, |$v:ident| $body:expr) => {
        match $gen {
            $crate::rng::Gen::SplitMix => { let $v = $crate::rng::SplitMix::new($stream); $body }
            $crate::rng::Gen::Philox => { let $v = $crate::rng::Philox::new($stream); $body }
            $crate::rng::Gen::Squares => { let $v = $crate::rng::Squares::new($stream); $body }
            $crate::rng::Gen::Pcg => { let $v = $crate::rng::Pcg::new($stream); $body }
            $crate::rng::Gen::Wy => { let $v = $crate::rng::Wy::new($stream); $body }
            $crate::rng::Gen::ChaCha8 => { let $v = $crate::rng::ChaCha8::new($stream); $body }
        }
    };
}

/// One draw by generator id (slow path, for tests and streams).
pub fn draw(gen: Gen, s: &Stream, being: u64, moment: u64) -> u64 {
    with_gen!(gen, s, |g| g.draw(being, moment))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn splitmix_reference() {
        // splitmix64 seeded with 0: first output.
        assert_eq!(mix64(GOLDEN), 0xe220_a839_7b1d_cdaf);
    }

    #[test]
    fn philox_known_answers() {
        // Random123 known-answer vectors for philox4x32-10.
        assert_eq!(philox4x32_10([0; 4], [0; 2]), [0x6627e8d5, 0xe169c58d, 0xbc57ac4c, 0x9b00dbd8]);
        assert_eq!(
            philox4x32_10([u32::MAX; 4], [u32::MAX; 2]),
            [0x408f276d, 0x41c83b0e, 0xa20bc7c6, 0x6d5451fd]
        );
        assert_eq!(
            philox4x32_10([0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344], [0xa4093822, 0x299f31d0]),
            [0xd16cfe09, 0x94fdcceb, 0x5001e420, 0x24126ea1]
        );
    }

    #[test]
    fn chacha8_known_answer() {
        // ChaCha8, all-zero key and IV: keystream starts 3e 00 ef 2f 89 5f 40 d6.
        assert_eq!(chacha8_first64(&[0; 8], 0, 0), 0xd640_5f89_2fef_003e);
    }

    #[test]
    fn squares_key_rules() {
        for seed in 0..1000u64 {
            let k = squares_key(seed);
            assert_eq!(k & 1, 1);
            for half in [k >> 32, k & 0xffff_ffff] {
                let mut seen = 0u32;
                for i in 0..8 {
                    let d = (half >> (4 * i)) & 15;
                    assert!(d != 0 && seen & (1 << d) == 0);
                    seen |= 1 << d;
                }
            }
        }
    }

    #[test]
    fn retry_is_independent_key() {
        let a = Stream::new(1, 2, 3);
        let b = Stream::new(1, 2, retry_purpose(3));
        for g in ALL_GENS {
            assert_ne!(draw(g, &a, 5, 6), draw(g, &b, 5, 6));
        }
    }
}
