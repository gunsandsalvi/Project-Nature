//! The relief noise (A5.3), for the ground's detail and the escarpment's line: gradient noise on a square lattice
//! that wraps at the world's size, and its fractal sum. Where a point lies in its lattice square comes from integer
//! ticks, and the rest is plain `f32` adding and multiplying, so every target makes the same ground (A3.2).
//!
//! Implements WLD-12, see A5.3: ground with detail to the metre, running on unbroken round the world.

use kd_core::geo::{H, TICKS_PER_M, W};
use kd_core::num;

const S: f32 = std::f32::consts::FRAC_1_SQRT_2;

/// The 8 gradient directions, 45° apart, counter-clockwise from east (x east, y south).
const GRADIENTS: [(f32, f32); 8] = [
    (1.0, 0.0),
    (S, -S),
    (0.0, -1.0),
    (-S, -S),
    (-1.0, 0.0),
    (-S, S),
    (0.0, 1.0),
    (S, S),
];

/// The largest lattice spacing, in metres: lattices up to it divide the world both ways.
pub const MAX_PERIOD_M: u32 = 8_192;

/// Which of the 8 directions lattice point (`i`, `j`) has: the low bits of `num::hash2(seed, (i << 32) | j)`
/// (A5.3).
pub fn key(seed: u64, i: u64, j: u64) -> u8 {
    (num::hash2(seed, (i << 32) | j) & 7) as u8
}

/// The quintic fade 6t⁵ − 15t⁴ + 10t³, so heights and slopes stay smooth across the lattice lines.
fn fade(t: f32) -> f32 {
    t * t * t * (t * (t * 6.0 - 15.0) + 10.0)
}

/// One octave of the noise at (`x`, `y`) in ticks, on a lattice `period_m` metres apart (a power of two up to
/// `MAX_PERIOD_M`), within ±1. Lattice indices wrap at the world's size, so a point and the same point a world
/// away east or south give the same value.
pub fn octave(seed: u64, x: i64, y: i64, period_m: u32) -> f32 {
    debug_assert!(
        period_m.is_power_of_two() && period_m <= MAX_PERIOD_M,
        "lattice spacing {period_m} m"
    );
    let p = i64::from(period_m) * i64::from(TICKS_PER_M);
    let (nx, ny) = (i64::from(W) / p, i64::from(H) / p);
    let (i, j) = (x.div_euclid(p), y.div_euclid(p));
    // Exact: the remainder is below 2^21 and the spacing a power of two.
    let (fx, fy) = (x.rem_euclid(p) as f32 / p as f32, y.rem_euclid(p) as f32 / p as f32);
    let corner = |di: i64, dj: i64, dx: f32, dy: f32| {
        let k = key(seed, (i + di).rem_euclid(nx) as u64, (j + dj).rem_euclid(ny) as u64);
        let (gx, gy) = GRADIENTS[usize::from(k)];
        gx * dx + gy * dy
    };
    let n00 = corner(0, 0, fx, fy);
    let n10 = corner(1, 0, fx - 1.0, fy);
    let n01 = corner(0, 1, fx, fy - 1.0);
    let n11 = corner(1, 1, fx - 1.0, fy - 1.0);
    let (u, v) = (fade(fx), fade(fy));
    let a = n00 + (n10 - n00) * u;
    let b = n01 + (n11 - n01) * u;
    // Unit gradients reach at most √2/2, at a square's centre with all four pointing to it.
    let n = (a + (b - a) * v) * std::f32::consts::SQRT_2;
    num::max(-1.0, num::min(1.0, n))
}

/// The seed of octave `o`, so octaves are independent.
fn octave_seed(seed: u64, o: u32) -> u64 {
    num::hash2(seed, u64::from(o))
}

/// `octaves` octaves from `period_m` down, each half the last's period and height, summed and scaled to ±1 (A5.3).
pub fn fbm(seed: u64, x: i64, y: i64, period_m: u32, octaves: u32) -> f32 {
    debug_assert!(
        octaves >= 1 && period_m >> (octaves - 1) >= 1,
        "{octaves} octaves from {period_m} m"
    );
    let (mut sum, mut amp, mut total) = (0.0f32, 1.0f32, 0.0f32);
    for o in 0..octaves {
        sum += amp * octave(octave_seed(seed, o), x, y, period_m >> o);
        total += amp;
        amp *= 0.5;
    }
    sum / total
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_core::chance::{draw, stream_seed};

    /// The `i`th keyed test point, anywhere on the world.
    fn point(stream: u64, i: u64) -> (i64, i64) {
        let d = draw(stream_seed(0x0072_656c, 0, 0), i, stream);
        (((d & 0xffff_ffff) % W as u64) as i64, ((d >> 32) % H as u64) as i64)
    }

    // checks: WLD-12
    #[test]
    fn octaves_periodic_and_bounded() {
        let (w, h) = (i64::from(W), i64::from(H));
        for period in [1, 4, 16, 256, 1024, MAX_PERIOD_M] {
            let seed = 77 + u64::from(period);
            for k in 0..20_000 {
                let (x, y) = point(u64::from(period), k);
                let v = octave(seed, x, y, period);
                assert!((-1.0..=1.0).contains(&v), "{period} m at ({x}, {y}): {v}");
                // The same point a world away, either way, and across both seams at once.
                assert_eq!(octave(seed, x + w, y, period), v);
                assert_eq!(octave(seed, x, y - h, period), v);
                assert_eq!(octave(seed, x - 3 * w, y + 2 * h, period), v);
            }
            // Zero on the lattice, and continuous across the seams: a tick either side differs by no more than
            // the steepest slope allows, 8 a lattice spacing.
            let p = i64::from(period) * 256;
            assert_eq!(octave(seed, 5 * p, 9 * p, period), 0.0);
            let tick = 8.0 / p as f32;
            for y in [0, p / 3, h - 1] {
                let (west, east) = (octave(seed, w - 1, y, period), octave(seed, 0, y, period));
                assert!((west - east).abs() <= tick, "{period} m, row {y}: {west} {east}");
            }
            let (north, south) = (octave(seed, p / 2, 0, period), octave(seed, p / 2, h - 1, period));
            assert!((north - south).abs() <= tick, "{period} m: {north} {south}");
        }
        // The scale reaches its bound: at square centres the largest value is nearly 1.
        let top = (0..65_536i64)
            .map(|k| octave(5, (k % 256) * 256 * 4 + 512, (k / 256) * 256 * 4 + 512, 4).abs())
            .fold(0.0f32, num::max);
        assert!(top > 0.99, "{top}");
        // The fractal sum stays within ±1 and is the same round the world.
        for k in 0..20_000 {
            let (x, y) = point(99, k);
            let v = fbm(3, x, y, 256, 7);
            assert!((-1.0..=1.0).contains(&v));
            assert_eq!(fbm(3, x + w, y + h, 256, 7), v);
        }
        // Each direction is chosen about an eighth of the time.
        let mut counts = [0u32; 8];
        for i in 0..80_000u64 {
            counts[usize::from(key(11, i % 400, i / 400))] += 1;
        }
        assert!(counts.iter().all(|&c| (9_000..11_000).contains(&c)), "{counts:?}");
    }

    // checks: WLD-12 RES-05
    #[test]
    fn same_bits_on_every_target() {
        // Fixed points of single octaves and of the ground's sum, across the seam: their bits hash to the value
        // stored on x86-64, here and on arm64 under qemu (tools/check.sh step 6).
        let mut bytes = Vec::new();
        for k in 0..4_096i64 {
            let (x, y) = (i64::from(W) - 300_000 + k * 977, i64::from(H) / 3 + (k % 64) * 4_093);
            for v in [
                octave(1, x, y, 1),
                octave(2, x, y, 32),
                octave(3, x, y, MAX_PERIOD_M),
                fbm(4, x, y, 256, 7),
                fbm(5, x, y, 512, 4),
            ] {
                bytes.extend(v.to_bits().to_le_bytes());
            }
        }
        assert_eq!(num::hash64(&bytes), STORED, "the relief's bits differ from x86-64's");
    }

    /// `same_bits_on_every_target`'s hash, as x86-64 made it.
    const STORED: u64 = 0xcdf66f7d41d4bc6d;
}
