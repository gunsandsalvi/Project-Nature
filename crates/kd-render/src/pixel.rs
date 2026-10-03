//! The Rust twins of the shaders' per-pixel formulas (A11.13 rule 2), under the same names as in `lib.glsl` and
//! with the same constants: the light's lightness from the sky and sun factors, its step on a ladder, the band's
//! 4 × 4 Bayer dither fixed to the world grid, colour 0's packing, and from α01b the world-fixed noise faded below
//! four art pixels, the surface the four nearest squares vote for, and the split between a surface's looks. Tests
//! use the twins, and the probe scene (`probe`) checks that the GPU gives the same answers exactly.
//!
//! Implements PRE-20, PRE-22 and PRE-01, see A11.1, A11.3, A11.5 and A11.13: the light picks the step, a narrow band
//! round each threshold is dithered and nowhere else, no pattern is finer than two art pixels, and every pixel is a
//! palette index.

use kd_core::m;

/// The categories of colour 0 (A11.2), bits 0–2 of its G channel; the shaders take them as generated defines.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum Cat {
    Void = 0,
    Ground = 1,
    Rock = 2,
    Water = 3,
    Plant = 4,
    Figure = 5,
    Thing = 6,
    Effect = 7,
}

impl Cat {
    pub const ALL: [Cat; 8] = [
        Cat::Void,
        Cat::Ground,
        Cat::Rock,
        Cat::Water,
        Cat::Plant,
        Cat::Figure,
        Cat::Thing,
        Cat::Effect,
    ];

    /// The define's name in the shaders: `CAT_ROCK` for `Cat::Rock`.
    pub fn define(self) -> String {
        format!("CAT_{}", format!("{self:?}").to_uppercase())
    }
}

/// Colour 0's flags above the category in its G channel (A11.2).
pub mod flag {
    pub const SUNLIT: u8 = 1 << 3;
    /// The haze level, 0 to 3, in bits 4 and 5.
    pub const HAZE_SHIFT: u8 = 4;
    pub const FIRELIT: u8 = 1 << 6;
    pub const GLOWING: u8 = 1 << 7;
}

/// The 4 × 4 Bayer matrix: the order in which a band's pixels take the upper step.
pub const BAYER: [[u8; 4]; 4] = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]];

/// The Bayer threshold at a world pixel, between 0 and 1: `(k + 0.5) / 16`.
pub fn bayer(x: i32, y: i32) -> f32 {
    (f32::from(BAYER[(y & 3) as usize][(x & 3) as usize]) + 0.5) / 16.0
}

/// The light's lightness at sky factor σ and sun factor τ: the cube root of `σ Y_sky + τ Y_sun`, taken as the
/// shaders take it, `pow(y, 1/3)` (A11.3).
pub fn lightness(sigma: f32, tau: f32, y_sky: f32, y_sun: f32) -> f32 {
    m::powf(sigma * y_sky + tau * y_sun, 1.0 / 3.0)
}

/// Where lightness `l` falls on a ladder of `steps` over the path's `range`: 0 at deep shade, `steps − 1` at full sun.
pub fn ladder_pos(l: f32, range: [f32; 2], steps: i32) -> f32 {
    (l - range[0]) / (range[1] - range[0]) * (steps - 1) as f32
}

/// The step at ladder position `s` (A11.3): the nearest one, except within `band` of the threshold midway between
/// two steps, where the Bayer pattern at world pixel `(x, y)` mixes them by how far `s` has crossed the band. The
/// shaders' `band` is `fwidth(s)`, so the band is about two art pixels wide.
pub fn light_step(s: f32, band: f32, steps: i32, x: i32, y: i32) -> i32 {
    let top = (steps - 1) as f32;
    let s = s.clamp(0.0, top);
    let k = s.floor();
    let d = s - k - 0.5;
    let up = if band > 0.0 && d.abs() < band {
        (d + band) / (2.0 * band) > bayer(x, y)
    } else {
        d >= 0.0
    };
    (k as i32 + i32::from(up)).min(steps - 1)
}

/// How far ladder position `s` lies from the nearest point where `light_step`'s answer changes, in units of `s`. The
/// answer is the same on both sides of whole numbers and of the band's edges (the Bayer thresholds lie strictly
/// inside 0 to 1), so it changes only where a threshold's comparison flips: midway between two steps with no band,
/// and inside a band where the crossed share equals the pixel's Bayer threshold. The probe keeps only inputs well
/// clear of these.
pub fn margin(s: f32, band: f32, steps: i32, x: i32, y: i32) -> f32 {
    let s = s.clamp(0.0, (steps - 1) as f32);
    let flip = if band > 0.0 {
        band * (2.0 * bayer(x, y) - 1.0)
    } else {
        0.0
    };
    (0..steps - 1)
        .map(|k| (s - (k as f32 + 0.5 + flip)).abs())
        .fold(f32::INFINITY, f32::min)
}

/// Colour 0 (A11.2): R the palette index, G the category and flags, B and A the view depth in 16 bits, high byte
/// first; the shaders' `pack_out` writes the same bytes.
pub fn pack(index: u8, cat: Cat, flags: u8, depth: f32) -> [u8; 4] {
    let d = (depth.clamp(0.0, 1.0) * 65535.0 + 0.5) as u32;
    [index, cat as u8 | (flags & !7), (d >> 8) as u8, (d & 255) as u8]
}

/// A lattice point's 32-bit hash, in unsigned arithmetic that wraps alike in Rust and GLSL ES 3.00.
pub fn hash3(x: u32, y: u32, seed: u32) -> u32 {
    let mut h = x.wrapping_mul(0x8da6_b343) ^ y.wrapping_mul(0xd816_3841) ^ seed.wrapping_mul(0xcb1a_b31f);
    h ^= h >> 15;
    h = h.wrapping_mul(0x2c1b_3c6d);
    h ^= h >> 12;
    h = h.wrapping_mul(0x297a_2d39);
    h ^ (h >> 15)
}

const S: f32 = std::f32::consts::FRAC_1_SQRT_2;

/// The noise's 8 gradient directions, 45° apart.
pub const GRADIENTS: [[f32; 2]; 8] = [
    [1.0, 0.0],
    [S, -S],
    [0.0, -1.0],
    [-S, -S],
    [-1.0, 0.0],
    [-S, S],
    [0.0, 1.0],
    [S, S],
];

/// The quintic fade 6t⁵ − 15t⁴ + 10t³.
pub fn fade5(t: f32) -> f32 {
    t * t * t * (t * (t * 6.0 - 15.0) + 10.0)
}

/// Gradient noise at `p` in lattice units (`p` at least 0), within ±1: the renderer's patterns, fixed to the world.
pub fn noise2(p: [f32; 2], seed: u32) -> f32 {
    let c = [p[0].floor(), p[1].floor()];
    let f = [p[0] - c[0], p[1] - c[1]];
    let (x, y) = (c[0] as i32 as u32, c[1] as i32 as u32);
    let corner = |dx: u32, dy: u32| {
        let g = GRADIENTS[(hash3(x.wrapping_add(dx), y.wrapping_add(dy), seed) & 7) as usize];
        g[0] * (f[0] - dx as f32) + g[1] * (f[1] - dy as f32)
    };
    let (n00, n10, n01, n11) = (corner(0, 0), corner(1, 0), corner(0, 1), corner(1, 1));
    let (u, v) = (fade5(f[0]), fade5(f[1]));
    let a = n00 + (n10 - n00) * u;
    let b = n01 + (n11 - n01) * u;
    ((a + (b - a) * v) * std::f32::consts::SQRT_2).clamp(-1.0, 1.0)
}

/// GLSL's `smoothstep`.
pub fn smoothstep(e0: f32, e1: f32, x: f32) -> f32 {
    let t = ((x - e0) / (e1 - e0)).clamp(0.0, 1.0);
    t * t * (3.0 - 2.0 * t)
}

/// How much of an octave of `wavelength` metres shows at art pixels of 1 / `inv_texel` metres: all of it from four
/// pixels a wavelength, none below two (A11.1 rule 2).
pub fn octave_fade(wavelength: f32, inv_texel: f32) -> f32 {
    smoothstep(2.0, 4.0, wavelength * inv_texel)
}

/// Up to two octaves of noise at `p` metres, given as (λ₀, 1/λ₀, λ₁, 1/λ₁) with λ₁ 0 for one octave, the second half
/// the first's height, each faded by its size in art pixels and scaled by the unfaded total, so a faded octave goes
/// to its average, 0.
pub fn faded_noise(p: [f32; 2], oct: [f32; 4], inv_texel: f32, seed: u32) -> f32 {
    let (mut sum, mut total, mut amp) = (0.0, 0.0, 1.0);
    for k in 0..2 {
        let (lambda, inv) = (oct[2 * k], oct[2 * k + 1]);
        if lambda <= 0.0 {
            break;
        }
        let n = noise2([p[0] * inv, p[1] * inv], seed ^ ((k as u32) << 16));
        sum += amp * octave_fade(lambda, inv_texel) * n;
        total += amp;
        amp *= 0.5;
    }
    if total > 0.0 { sum / total } else { 0.0 }
}

/// How far, in metres, the edges between surfaces wander, and the octaves of the noise they wander by, as
/// (λ, 1/λ) pairs (A11.5, until α01d's coverage).
pub const EDGE_WOBBLE_M: f32 = 0.45;
pub const EDGE_OCTAVES: [f32; 4] = [4.0, 0.25, 1.0, 1.0];
/// Seeds of the wobble's two directions and of the looks' split.
pub const SEED_EDGE_X: u32 = 11;
pub const SEED_EDGE_Y: u32 = 12;
pub const SEED_SPLIT: u32 = 13;

/// The wobble at world-fixed position `w` metres: the shift, east and south, at which the surfaces are looked up.
pub fn edge_wobble(w: [f32; 2], inv_texel: f32) -> [f32; 2] {
    [
        EDGE_WOBBLE_M * faded_noise(w, EDGE_OCTAVES, inv_texel, SEED_EDGE_X),
        EDGE_WOBBLE_M * faded_noise(w, EDGE_OCTAVES, inv_texel, SEED_EDGE_Y),
    ]
}

/// The four squares nearest position `q` (metres east and south of the area's corner, squares 1 m with centres at
/// halves): the lowest one's column and row, at most `max_base`, and where `q` lies between their centres, 0 to 1.
pub fn vote_base(q: [f32; 2], max_base: i32) -> ([i32; 2], [f32; 2]) {
    let base = q.map(|v| ((v - 0.5).floor() as i32).clamp(0, max_base));
    let f = [
        (q[0] - 0.5 - base[0] as f32).clamp(0.0, 1.0),
        (q[1] - 0.5 - base[1] as f32).clamp(0.0, 1.0),
    ];
    (base, f)
}

/// The surface four squares vote for at `f` between their centres, each by its bilinear weight, a surface's
/// squares adding up; `ids` in the order (0, 0), (1, 0), (0, 1), (1, 1); a tie goes to the lower number (A11.5).
pub fn vote4(f: [f32; 2], ids: [i32; 4]) -> i32 {
    let w = [
        (1.0 - f[0]) * (1.0 - f[1]),
        f[0] * (1.0 - f[1]),
        (1.0 - f[0]) * f[1],
        f[0] * f[1],
    ];
    let (mut best, mut best_w) = (ids[0], -1.0f32);
    for k in 0..4 {
        let mut t = 0.0;
        for m in 0..4 {
            t += if ids[m] == ids[k] { w[m] } else { 0.0 };
        }
        if t > best_w || (t == best_w && ids[k] < best) {
            best_w = t;
            best = ids[k];
        }
    }
    best
}

/// Which of a surface's `looks` the split noise `v` picks: the next look wherever `v` is above its take-over value.
pub fn split_look(v: f32, at: [f32; 2], looks: i32) -> i32 {
    i32::from(looks > 1 && v > at[0]) + i32::from(looks > 2 && v > at[1])
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-20
    #[test]
    fn step_at_thresholds() {
        // With no band, a ladder position rounds to the nearest step, the threshold going up, clamped to the ends.
        for steps in 4..=7 {
            let top = steps - 1;
            for k in 0..top {
                let t = k as f32 + 0.5;
                assert_eq!(light_step(t - 0.001, 0.0, steps, 0, 0), k);
                assert_eq!(light_step(t, 0.0, steps, 0, 0), k + 1);
                assert_eq!(light_step(t + 0.001, 0.0, steps, 0, 0), k + 1);
            }
            assert_eq!(light_step(-3.0, 0.0, steps, 0, 0), 0);
            assert_eq!(light_step(top as f32 + 2.0, 0.0, steps, 0, 0), top);
        }
        // Lightness: the cube root of the light the factors give, so full sun on a ladder's range is its top.
        let (y_sky, y_sun) = (0.12, 0.85);
        let range = [lightness(0.3, 0.0, y_sky, y_sun), lightness(1.0, 1.0, y_sky, y_sun)];
        assert!((ladder_pos(range[1], range, 6) - 5.0).abs() < 1e-5);
        assert!(ladder_pos(range[0], range, 6).abs() < 1e-5);
        // Open shade sits a quarter of the way up the lightness range at this light: step 1 of 6.
        assert_eq!(
            light_step(ladder_pos(lightness(1.0, 0.0, y_sky, y_sun), range, 6), 0.0, 6, 0, 0),
            1
        );
    }

    // checks: PRE-20 PRE-01
    #[test]
    fn dither_only_in_the_band() {
        // Outside the band every pixel takes the nearest step; inside it, the share of a 4 × 4 tile taking the
        // upper step grows with how far the band is crossed, from none at its bottom to all at its top.
        let band = 0.2;
        for (s, want_up) in [(1.25, 0), (1.29, 0), (1.71, 16), (1.9, 16)] {
            let up: i32 = (0..16).map(|i| light_step(s, band, 6, i % 4, i / 4) - 1).sum();
            assert_eq!(up, want_up, "s {s}");
        }
        let mut last = 0;
        for i in 0..=20 {
            let s = 1.3 + 0.4 * i as f32 / 20.0;
            let up: i32 = (0..16).map(|p| light_step(s, band, 6, p % 4, p / 4) - 1).sum();
            assert!(up >= last, "the share falls at {s}");
            last = up;
        }
        // Halfway through the band, half the tile; the pattern is fixed to the world grid, repeating every 4.
        let half: i32 = (0..16).map(|p| light_step(1.5, band, 6, p % 4, p / 4) - 1).sum();
        assert_eq!(half, 8);
        assert_eq!(light_step(1.45, band, 6, 1, 2), light_step(1.45, band, 6, 5, 6));
        // The margin finds where the answer flips: midway with no band, at the pixel's Bayer share inside one.
        assert!(margin(1.5005, 0.0, 6, 0, 0) < 0.001);
        let flip = 1.5 + band * (2.0 * bayer(0, 0) - 1.0);
        assert!(margin(flip + 0.0005, band, 6, 0, 0) < 0.001);
        assert_ne!(
            light_step(flip - 0.001, band, 6, 0, 0),
            light_step(flip + 0.001, band, 6, 0, 0)
        );
        assert!(margin(1.0, band, 6, 0, 0) > 0.2);
        // Across a band's edge and a whole number nothing flips.
        assert_eq!(
            light_step(1.3 - 0.001, band, 6, 0, 0),
            light_step(1.3 + 0.001, band, 6, 0, 0)
        );
        assert_eq!(
            light_step(2.0 - 0.001, band, 6, 0, 0),
            light_step(2.0 + 0.001, band, 6, 0, 0)
        );
    }

    // checks: PRE-20
    #[test]
    fn surface_vote_by_weight() {
        // Each square counts by how near its centre the pixel lies, and a surface's squares add up: one square of
        // rock beside three of grass wins only close to its own centre.
        let (grass, rock) = (0, 2);
        assert_eq!(vote4([0.1, 0.1], [rock, grass, grass, grass]), rock);
        assert_eq!(vote4([0.4, 0.4], [rock, grass, grass, grass]), grass);
        // Two squares each: the nearer pair wins, and at the exact middle the tie goes to the lower number.
        assert_eq!(vote4([0.3, 0.5], [rock, grass, rock, grass]), rock);
        assert_eq!(vote4([0.7, 0.5], [rock, grass, rock, grass]), grass);
        assert_eq!(vote4([0.5, 0.5], [rock, grass, rock, grass]), grass);
        // Three surfaces: a split pair outweighs a single square of the third.
        assert_eq!(vote4([0.5, 0.45], [1, 1, 3, 2]), 1);
        // The lookup finds the four squares round a place and where it lies between their centres, clamped at the
        // area's edge, where the edge's squares alone vote.
        let (base, f) = vote_base([3.7, 10.2], 254);
        assert_eq!(base, [3, 9]);
        assert!((f[0] - 0.2).abs() < 1e-5 && (f[1] - 0.7).abs() < 1e-5, "{f:?}");
        assert_eq!(vote_base([0.1, 255.9], 254), ([0, 254], [0.0, 1.0]));
        // The wobble moves the place by at most its size, and is fixed to the world: the same place, the same move.
        for k in 0..1_000 {
            let w = [k as f32 * 7.31, k as f32 * 3.17];
            let m = edge_wobble(w, 1.0 / 0.05);
            assert!(m.iter().all(|v| v.abs() <= EDGE_WOBBLE_M));
            assert_eq!(m, edge_wobble(w, 1.0 / 0.05));
        }
        // A split picks the next look above each take-over value, never more looks than the surface has.
        assert_eq!(split_look(0.3, [0.25, 2.0], 2), 1);
        assert_eq!(split_look(0.2, [0.25, 2.0], 2), 0);
        assert_eq!(split_look(0.9, [-0.5, 0.5], 3), 2);
        assert_eq!(split_look(0.9, [-0.5, 0.5], 1), 0);
    }

    // checks: PRE-22 PRE-20
    #[test]
    fn noise_fades_below_four_pixels() {
        // An octave shows fully from four art pixels a wavelength and not at all below two, rising between.
        assert_eq!(octave_fade(4.0, 1.0), 1.0);
        assert_eq!(octave_fade(8.0, 1.0), 1.0);
        assert_eq!(octave_fade(2.0, 1.0), 0.0);
        assert_eq!(octave_fade(1.0, 1.0), 0.0);
        assert_eq!(octave_fade(3.0, 1.0), 0.5);
        let mut last = 0.0;
        for i in 0..=100 {
            let f = octave_fade(1.5 + i as f32 * 0.03, 1.0);
            assert!(f >= last);
            last = f;
        }
        // The grass's split at the person stop has both octaves; at 2 m art pixels the 6 m octave is fading and
        // the 24 m one whole; at 13 m pixels nothing is left, so the pattern settles to its average, 0.
        let oct = [24.0, 1.0 / 24.0, 6.0, 1.0 / 6.0];
        let spread = |texel: f32| {
            (0..2_000)
                .map(|k| faded_noise([k as f32 * 1.37, k as f32 * 0.71], oct, 1.0 / texel, SEED_SPLIT).abs())
                .fold(0.0, f32::max)
        };
        assert!(spread(0.03) > 0.4, "{}", spread(0.03));
        assert!(spread(2.0) < spread(0.03) && spread(2.0) > 0.1);
        assert_eq!(spread(13.0), 0.0);
        // A faded octave adds nothing but its share of the scale: the noise at 2 m pixels is the 24 m octave's
        // alone, two thirds of its full height.
        let w = [101.5, 77.25];
        let one = noise2([w[0] / 24.0, w[1] / 24.0], SEED_SPLIT);
        let faded = faded_noise(w, [24.0, 1.0 / 24.0, 6.0, 1.0 / 6.0], 1.0 / 3.0, SEED_SPLIT);
        assert!((faded - one * 2.0 / 3.0).abs() < 1e-6, "{faded} {one}");
        // The noise is within ±1, zero on its lattice, and smooth: a hundredth of a lattice step moves it little.
        assert_eq!(noise2([5.0, 9.0], 1), 0.0);
        for k in 0..10_000 {
            let p = [k as f32 * 0.137, k as f32 * 0.071];
            let v = noise2(p, 3);
            assert!((-1.0..=1.0).contains(&v));
            assert!((noise2([p[0] + 0.01, p[1]], 3) - v).abs() < 0.1);
        }
    }

    // checks: PRE-01
    #[test]
    fn colour_0_packing() {
        assert_eq!(pack(37, Cat::Rock, flag::SUNLIT, 0.0), [37, 2 | 8, 0, 0]);
        assert_eq!(
            pack(255, Cat::Effect, flag::GLOWING | 3 << flag::HAZE_SHIFT, 1.0),
            [255, 7 | 128 | 48, 255, 255]
        );
        let p = pack(1, Cat::Void, 0, 0.5);
        assert_eq!(u32::from(p[2]) << 8 | u32::from(p[3]), 32768);
        // A flag never spills into the category.
        assert_eq!(pack(0, Cat::Ground, 0xff, 0.0)[1] & 7, 1);
        assert_eq!(Cat::Rock.define(), "CAT_ROCK");
        assert_eq!(Cat::ALL.map(|c| c as u8), [0, 1, 2, 3, 4, 5, 6, 7]);
    }
}
