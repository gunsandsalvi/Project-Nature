//! The Rust twins of the shaders' per-pixel formulas (A11.13 rule 2), under the same names as in `lib.glsl` and
//! with the same constants: the light's lightness from the sky and sun factors, its step on a ladder, the band's
//! 4 × 4 Bayer dither fixed to the world grid, and colour 0's packing. Tests use the twins, and the probe scene
//! (`probe`) checks that the GPU gives the same steps exactly.
//!
//! Implements PRE-20 and PRE-01, see A11.3 and A11.13: the light picks the step, a narrow band round each threshold
//! is dithered and nowhere else, and every pixel is a palette index.

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
