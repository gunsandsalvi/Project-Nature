//! Numbers with the same bits on every target (A3.2): minimum and maximum by plain comparison, floats cleaned
//! before they are stored or hashed, the hashes, sums in a fixed order, and the saved types' marker.
//!
//! Implements RES-05, see A3.2: the phone, the browser and the cloud get the same bits from the same numbers.

#[cfg(not(target_endian = "little"))]
compile_error!("saved and hashed types are written as little-endian bytes (A3.2)");

/// The smaller of two floats by plain comparison: `b` when they are equal, so −0.0 against +0.0 gives the same
/// bits on every target, which `f32::min` does not promise (A3.2).
#[inline]
pub fn min(a: f32, b: f32) -> f32 {
    if a < b { a } else { b }
}

/// The larger of two floats by plain comparison: `b` when they are equal (A3.2).
#[inline]
pub fn max(a: f32, b: f32) -> f32 {
    if a > b { a } else { b }
}

/// A float as it may be stored or hashed: −0.0 becomes +0.0 (adding 0.0 changes nothing else), and a NaN or an
/// infinity, which asserts in debug and test builds, becomes 0 (A3.2).
#[inline]
pub fn clean(x: f32) -> f32 {
    debug_assert!(
        x.is_finite(),
        "a NaN or an infinity was about to be stored or hashed (A3.2)"
    );
    if x.is_finite() { x + 0.0 } else { 0.0 }
}

/// One field's record of whether it has logged a NaN or an infinity, kept by whatever owns the field, so a release
/// build logs once per field rather than once per write (A3.2).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct CleanLog {
    logged: bool,
}

impl CleanLog {
    /// `clean`, logging the field's first NaN or infinity.
    pub fn clean(&mut self, field: &str, x: f32) -> f32 {
        if !x.is_finite() && !self.logged {
            self.logged = true;
            log::error!(target: "kd::core", "{field}: {x} stored as 0 (A3.2)");
        }
        clean(x)
    }
}

/// The odd constant A3.3's mixing starts from (the golden ratio's fraction).
pub const GOLDEN: u64 = 0x9e37_79b9_7f4a_7c15;

/// SplitMix64's finaliser: every input bit reaches every output bit (A3.3).
#[inline]
pub const fn mix64(mut z: u64) -> u64 {
    z = (z ^ (z >> 30)).wrapping_mul(0xbf58_476d_1ce4_e5b9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94d0_49bb_1331_11eb);
    z ^ (z >> 31)
}

/// XXH3-64 of some bytes: state hashes, chunk integrity and the catalogue's hash (A3.2).
#[inline]
pub fn hash64(bytes: &[u8]) -> u64 {
    xxhash_rust::xxh3::xxh3_64(bytes)
}

/// A hash of an ordered pair, for draws about two beings and stable looks: `mix64(a ^ mix64(b ^ GOLDEN))` (A3.2,
/// A3.3).
#[inline]
pub const fn hash2(a: u64, b: u64) -> u64 {
    mix64(a ^ mix64(b ^ GOLDEN))
}

/// How many values the sum's first level adds as one block (A3.2, B01).
pub const SUM_BLOCK: usize = 4096;

/// The sum of many floats in one fixed order, whatever their number or the target (A3.2): blocks of 4,096 values,
/// each padded with zeros to a power of two and halved repeatedly (element `i` plus element `i + h`), then the
/// blocks' results the same way.
pub fn sum_f32(xs: &[f32]) -> f32 {
    let mut buf = [0.0f32; SUM_BLOCK];
    if xs.len() <= SUM_BLOCK {
        return block_sum(&mut buf, xs);
    }
    let mut blocks: Vec<f32> = xs.chunks(SUM_BLOCK).map(|b| block_sum(&mut buf, b)).collect();
    blocks.resize(blocks.len().next_power_of_two(), 0.0);
    halve(&mut blocks)
}

/// Copies up to `SUM_BLOCK` values into `buf`, pads them with zeros to a power of two and halves them to one.
fn block_sum(buf: &mut [f32; SUM_BLOCK], xs: &[f32]) -> f32 {
    if xs.is_empty() {
        return 0.0;
    }
    let n = xs.len().next_power_of_two();
    buf[..xs.len()].copy_from_slice(xs);
    buf[xs.len()..n].fill(0.0);
    halve(&mut buf[..n])
}

/// Halves a power-of-two run of values to one: element `i` plus element `i + h`, for h = n/2, n/4, … 1.
fn halve(v: &mut [f32]) -> f32 {
    debug_assert!(v.len().is_power_of_two());
    let mut h = v.len() / 2;
    while h >= 1 {
        let (low, high) = v.split_at_mut(h);
        for (a, b) in low.iter_mut().zip(&high[..h]) {
            *a += *b;
        }
        h /= 2;
    }
    v[0]
}

/// The dot product of two equally long slices in 8 fixed lanes: element `i` goes to lane `i % 8`, and the lanes
/// are then halved as `sum_f32` halves (A3.2).
pub fn dot_f32(a: &[f32], b: &[f32]) -> f32 {
    assert_eq!(a.len(), b.len(), "dot_f32 of slices of different lengths");
    let mut lanes = [0.0f32; 8];
    for (i, (x, y)) in a.iter().zip(b).enumerate() {
        lanes[i % 8] += x * y;
    }
    for h in [4, 2, 1] {
        for i in 0..h {
            lanes[i] += lanes[i + h];
        }
    }
    lanes[0]
}

/// A type saved or hashed as its bytes: plain data of fixed-width integers and `f32`, never `usize` or `isize`
/// (A3.2). Declared with `fixed!`, which checks its size on every target.
pub trait Fixed: bytemuck::Pod {}

/// Declares a saved or hashed type: `fixed!(Type, bytes)` implements `Fixed` and asserts at compile time that the
/// type is `bytes` long, so a `usize` field fails either the 32-bit web build or the 64-bit ones (A3.2).
#[macro_export]
macro_rules! fixed {
    ($t:ty, $bytes:expr) => {
        impl $crate::num::Fixed for $t {}
        const _: () = assert!(
            ::core::mem::size_of::<$t>() == $bytes,
            concat!(stringify!($t), " is not the size fixed! declares (A3.2)")
        );
    };
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: RES-05
    #[test]
    fn min_max_signed_zero() {
        let (pz, nz) = (0.0f32, -0.0f32);
        // Equal values give the second, so the bits never depend on the target.
        assert_eq!(min(nz, pz).to_bits(), pz.to_bits());
        assert_eq!(min(pz, nz).to_bits(), nz.to_bits());
        assert_eq!(max(nz, pz).to_bits(), pz.to_bits());
        assert_eq!(max(pz, nz).to_bits(), nz.to_bits());
        assert_eq!(
            (min(1.0, 2.0), max(1.0, 2.0), min(-3.0, -4.0), max(-3.0, -4.0)),
            (1.0, 2.0, -4.0, -3.0)
        );
        // A NaN never compares, so it is returned only in second place: defined, if never wanted.
        assert_eq!(min(f32::NAN, 1.0), 1.0);
        assert!(min(1.0, f32::NAN).is_nan());
    }

    // checks: RES-05
    #[test]
    fn clean_turns_negative_zero() {
        assert_eq!(clean(-0.0).to_bits(), 0);
        for x in [
            0.0f32,
            1.5,
            -1.5,
            f32::MIN_POSITIVE,
            -f32::MIN_POSITIVE / 8.0,
            f32::MAX,
            f32::MIN,
            1e-45,
        ] {
            assert_eq!(clean(x).to_bits(), x.to_bits(), "{x}");
        }
        let mut log = CleanLog::default();
        assert_eq!(log.clean("t", -0.0).to_bits(), 0);
        assert_eq!(log, CleanLog::default(), "a finite value logs nothing");
    }

    // checks: RES-05
    #[test]
    #[should_panic(expected = "NaN or an infinity")]
    fn clean_asserts_on_nan_in_tests() {
        clean(f32::NAN);
    }

    // checks: RES-05
    #[test]
    fn sum_tree_fixed_order() {
        // The tree pairs 1e8 with −1e8 first; adding in index order would lose the first 1.0 in 1e8.
        assert_eq!(sum_f32(&[1e8, 1.0, -1e8, 1.0]), 2.0);
        assert_eq!(sum_f32(&[]), 0.0);
        assert_eq!(sum_f32(&[-0.0]).to_bits(), (-0.0f32).to_bits());
        // Against the tree written out another way: recursive halves of the zero-padded values, then the blocks.
        fn reference(xs: &[f32]) -> f32 {
            fn tree(v: &[f32]) -> f32 {
                let n = v.len().next_power_of_two();
                let mut p = v.to_vec();
                p.resize(n, 0.0);
                while p.len() > 1 {
                    let h = p.len() / 2;
                    p = (0..h).map(|i| p[i] + p[i + h]).collect();
                }
                p[0]
            }
            if xs.is_empty() {
                return 0.0;
            }
            let blocks: Vec<f32> = xs.chunks(SUM_BLOCK).map(tree).collect();
            tree(&blocks)
        }
        let mut z = 1u64;
        let xs: Vec<f32> = (0..3 * SUM_BLOCK + 17)
            .map(|_| {
                z = mix64(z);
                ((z >> 40) as f32 - 8_388_608.0) * 1e-3
            })
            .collect();
        for n in [1, 3, 64, SUM_BLOCK - 1, SUM_BLOCK, SUM_BLOCK + 1, xs.len()] {
            assert_eq!(sum_f32(&xs[..n]).to_bits(), reference(&xs[..n]).to_bits(), "{n} values");
        }
        // Lanes of the dot product: element i in lane i % 8, then halved.
        let a: Vec<f32> = xs[..19].to_vec();
        let ones = vec![1.0f32; 19];
        let mut lanes = [0.0f32; 8];
        for (i, x) in a.iter().enumerate() {
            lanes[i % 8] += x;
        }
        let want = ((lanes[0] + lanes[4]) + (lanes[2] + lanes[6])) + ((lanes[1] + lanes[5]) + (lanes[3] + lanes[7]));
        assert_eq!(dot_f32(&a, &ones).to_bits(), want.to_bits());
    }

    // checks: RES-05
    #[test]
    fn hash2_mixes() {
        // Known answers, worked out apart from this code (64-bit SplitMix64 finaliser, A3.3).
        assert_eq!(mix64(0), 0);
        assert_eq!(mix64(1), 0x5692_161d_100b_05e5);
        assert_eq!(hash2(0, 0), 0x4821_8226_ff3c_d4bf);
        assert_eq!(hash2(1, 2), 0xf282_6f98_653e_9e57);
        assert_ne!(hash2(1, 2), hash2(2, 1), "the pair is ordered");
        // Flipping one input bit flips about half the output bits.
        let mut flips = 0u32;
        let mut z = 7u64;
        for i in 0..2000u32 {
            z = mix64(z);
            let bit = 1u64 << (i % 64);
            flips += (hash2(z, i as u64) ^ hash2(z ^ bit, i as u64)).count_ones();
            flips += (hash2(z, i as u64) ^ hash2(z, i as u64 ^ bit)).count_ones();
        }
        let mean = flips as f32 / 4000.0;
        assert!((31.0..33.0).contains(&mean), "{mean} bits flipped on average");
        // XXH3-64's published answer for no bytes.
        assert_eq!(hash64(b""), 0x2d06_8005_38d3_94c2);
    }

    #[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
    #[repr(C)]
    struct Pair {
        a: u32,
        b: f32,
    }
    crate::fixed!(Pair, 8);

    // checks: RES-05
    #[test]
    fn fixed_types_are_their_bytes() {
        fn bytes<T: Fixed>(t: &T) -> &[u8] {
            bytemuck::bytes_of(t)
        }
        let p = Pair { a: 1, b: 1.0 };
        assert_eq!(bytes(&p), &[1, 0, 0, 0, 0, 0, 0x80, 0x3f]);
    }
}
