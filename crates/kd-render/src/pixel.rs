//! The Rust twins of the shaders' per-pixel formulas (A11.13 rule 2), under the same names as in `lib.glsl` and
//! with the same constants: the light's lightness from the sky and sun factors, its step on a ladder, the band's
//! 4 × 4 Bayer dither fixed to the world grid, colour 0's packing, and from α01b the world-fixed noise faded below
//! four art pixels and the split between a surface's looks; from α01c the shadows, outlines and haze; from α01d the
//! surfaces' coverage read at the art pixel's footprint, the looks' micro-relief, and which stones and tufts show.
//! Tests use the twins, and the probe scene (`probe`) checks that the GPU gives the same answers exactly.
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

/// The sun's disc, 0.53° across (A11.4), as the tangent of its width.
pub const SUN_TAN: f32 = 0.009_25;

/// The share of the light's disc showing over a point's horizon (A11.4, A11.5): `horizon` is the slope of the
/// highest ground toward the light (its sun field) and `tan_e` the light's own slope. The disc rises clear over its
/// 0.53° as the light climbs past the horizon, `(tan e − m) / (1 + m tan e)` being the tangent of the angle between
/// them, so a shadow's edge is sharp near its caster and soft far from it; a light more than a right angle above
/// the horizon, as from a peak, is clear of it.
pub fn sunlit(horizon: f32, tan_e: f32) -> f32 {
    ((tan_e - horizon) / (SUN_TAN * (1.0 + tan_e * horizon).max(1e-6)) + 0.5).clamp(0.0, 1.0)
}

/// The sun factor τ (A11.3): the sunlit share times how squarely the surface faces the light, `n·l`.
pub fn sun_factor(horizon: f32, tan_e: f32, n_dot_l: f32) -> f32 {
    sunlit(horizon, tan_e) * n_dot_l.max(0.0)
}

/// The sky factor σ (A11.3): the share of the sky the point's horizon leaves open (its sky field), times how much
/// of the sky the surface faces, `(1 + n_up) / 2`.
pub fn sky_factor(open: f32, n_up: f32) -> f32 {
    open * (1.0 + n_up) / 2.0
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

/// How far, in metres, a neighbour must lie beyond the plane through a pixel and its opposite neighbour for the pixel
/// to be on a silhouette, by category (A11.2): figures 0.25 m, things 0.3 m, plants 1 m, rock 1.5 m, ground and
/// water 6 m; void and effects never.
pub const OUTLINE_GAP_M: [f32; 8] = [1e9, 6.0, 1.5, 6.0, 1.0, 0.25, 0.3, 1e9];

/// Whether a pixel at depth `depth` stands in front of its neighbour at `far` (A11.2): the neighbour lies beyond
/// where the plane through the pixel and its opposite neighbour, at `opposite`, puts it by more than `gap`, all in
/// metres. A slope seen edge-on keeps its neighbours on its plane however steep it is, so it draws no outline.
pub fn outline_toward(depth: f32, far: f32, opposite: f32, gap: f32) -> bool {
    far - (2.0 * depth - opposite) > gap
}

/// The haze at a point `depth` metres beyond the view's target plane and `height` metres above the sea (A11.4): the
/// air's optical depth along its ray to the eye's plane, `eye[0]` metres before the target, climbing as the view
/// tilts (`eye[1]`, the sine of its pitch), through the aerosol's and the air's layers, of extinction `beta` a metre
/// at the sea's level falling over `scale` metres of height, in closed form; as `1 − exp(−depth)`.
pub fn haze(depth: f32, height: f32, beta: [f32; 2], scale: [f32; 2], eye: [f32; 2]) -> f32 {
    let rise = eye[1].max(1e-3);
    let l = (eye[0] + depth).max(0.0);
    let mut tau = 0.0;
    for k in 0..2 {
        tau += beta[k] * scale[k] / rise * m::exp(-height / scale[k]) * (1.0 - m::exp(-l * rise / scale[k]));
    }
    1.0 - m::exp(-tau)
}

/// The haze's level, 0 to 3: how many of `levels` the haze is above, each within `band` of its threshold mixed by
/// the Bayer pattern at world pixel `(x, y)`, as the light's steps are (A11.4).
pub fn haze_level(haze: f32, levels: [f32; 3], band: f32, x: i32, y: i32) -> i32 {
    let b = bayer(x, y);
    levels
        .iter()
        .map(|&at| {
            let d = haze - at;
            i32::from(if band > 0.0 && d.abs() < band {
                (d + band) / (2.0 * band) > b
            } else {
                d >= 0.0
            })
        })
        .sum()
}

/// How far haze `haze` lies from the nearest point where `haze_level`'s answer changes; the probe keeps only
/// inputs well clear of these.
pub fn haze_margin(haze: f32, levels: [f32; 3], band: f32, x: i32, y: i32) -> f32 {
    let flip = if band > 0.0 {
        band * (2.0 * bayer(x, y) - 1.0)
    } else {
        0.0
    };
    levels
        .iter()
        .map(|&at| (haze - (at + flip)).abs())
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
/// (λ, 1/λ) pairs (A11.5).
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

/// Surfaces an area's coverage holds: four to a texture, two textures (A11.5).
pub const COVER_CHANNELS: usize = 8;

/// The level of coverage an art pixel of `texel` metres reads: its footprint, `log2(texel / 1 m)`, from level 0,
/// the square metres, to `top` (A11.5).
pub fn cover_level(texel: f32, top: usize) -> f32 {
    m::log2(texel).clamp(0.0, top as f32)
}

/// Each channel's share at `q` (in level-0 texels from the texture's corner, `side` to a side: metres for the
/// coverage, half metres for the contact shade) read at mip `level`: bilinear between the four texels round `q` at
/// the levels either side, clamped at the edge, and mixed between them by the level's fraction, as the shader reads
/// them by `texelFetch` (A11.5).
pub fn cover_sample<const N: usize>(levels: &[Vec<[u8; N]>], side: usize, q: [f32; 2], level: f32) -> [f32; N] {
    let top = levels.len() - 1;
    let l0 = (level.floor().max(0.0) as usize).min(top);
    let l1 = (l0 + 1).min(top);
    let t = level - l0 as f32;
    let bilinear = |l: usize| -> [f32; N] {
        let n = (side >> l).max(1) as i32;
        let scale = 1.0 / (1u32 << l) as f32;
        let p = [q[0] * scale - 0.5, q[1] * scale - 0.5];
        let i = p.map(f32::floor);
        let f = [p[0] - i[0], p[1] - i[1]];
        let texel = |dx: i32, dy: i32| {
            let x = (i[0] as i32 + dx).clamp(0, n - 1);
            let y = (i[1] as i32 + dy).clamp(0, n - 1);
            levels[l][(y * n + x) as usize]
        };
        let (a, b, c, d) = (texel(0, 0), texel(1, 0), texel(0, 1), texel(1, 1));
        std::array::from_fn(|k| {
            let v = |t: [u8; N]| f32::from(t[k]) / 255.0;
            let upper = v(a) + (v(b) - v(a)) * f[0];
            let lower = v(c) + (v(d) - v(c)) * f[0];
            upper + (lower - upper) * f[1]
        })
    };
    let lo = bilinear(l0);
    if t <= 0.0 || l1 == l0 {
        return lo;
    }
    let hi = bilinear(l1);
    std::array::from_fn(|k| lo[k] + (hi[k] - lo[k]) * t)
}

/// The surface with the largest share among the channels holding one (`ids` −1 for none); a tie goes to the
/// earlier channel, the lower number (A11.5).
pub fn cover_pick(shares: [f32; COVER_CHANNELS], ids: [i32; COVER_CHANNELS]) -> i32 {
    let (mut best, mut best_v) = (-1, -1.0f32);
    for k in 0..COVER_CHANNELS {
        if ids[k] >= 0 && shares[k] > best_v {
            best = ids[k];
            best_v = shares[k];
        }
    }
    best
}

/// How far `cover_pick` is from picking otherwise: the largest share less the next (for the probe's clearance).
pub fn cover_margin(shares: [f32; COVER_CHANNELS], ids: [i32; COVER_CHANNELS]) -> f32 {
    let mut held: Vec<f32> = (0..COVER_CHANNELS)
        .filter(|&k| ids[k] >= 0)
        .map(|k| shares[k])
        .collect();
    held.sort_by(|a, b| b.total_cmp(a));
    match held.as_slice() {
        [a, b, ..] => a - b,
        _ => 1.0,
    }
}

/// Octaves a surface's micro-relief may have, and the seeds of its two tilts (A11.5).
pub const RELIEF_OCTAVES: usize = 4;
pub const SEED_RELIEF_X: u32 = 14;
pub const SEED_RELIEF_Y: u32 = 15;

/// How many octaves a micro-relief of wavelengths from `smallest` to `largest` metres has: from the largest,
/// halving, down to the smallest, at most `RELIEF_OCTAVES`.
pub fn relief_octaves(smallest: f32, largest: f32) -> usize {
    let (mut n, mut lambda) = (0, largest);
    while n < RELIEF_OCTAVES && lambda >= smallest * (1.0 - 1e-4) {
        n += 1;
        lambda *= 0.5;
    }
    n
}

/// The micro-relief's tilt of the ground at world-fixed `w` metres, a slope east and south added to the ground's
/// (A11.5): `relief` is (λ₀, 1/λ₀, octaves, each octave's greatest tilt), each octave gradient noise half the last's
/// wavelength and as strong in slope, as a self-similar surface's bumps are, faded below four art pixels (A11.1
/// rule 2).
pub fn relief_tilt(w: [f32; 2], relief: [f32; 4], inv_texel: f32) -> [f32; 2] {
    let (mut lambda, mut inv) = (relief[0], relief[1]);
    let octaves = ((relief[2] + 0.5) as usize).min(RELIEF_OCTAVES);
    let mut sum = [0.0f32; 2];
    for k in 0..octaves {
        let fade = octave_fade(lambda, inv_texel);
        if fade > 0.0 {
            let p = [w[0] * inv, w[1] * inv];
            let s = (k as u32) << 16;
            sum[0] += fade * noise2(p, SEED_RELIEF_X ^ s);
            sum[1] += fade * noise2(p, SEED_RELIEF_Y ^ s);
        }
        lambda *= 0.5;
        inv *= 2.0;
    }
    [sum[0] * relief[3], sum[1] * relief[3]]
}

/// The ground's normal (east, north, up) where its slope east and south is `slope`, tilted by `tilt`.
pub fn ground_normal(slope: [f32; 2], tilt: [f32; 2]) -> [f32; 3] {
    let g = [slope[0] + tilt[0], slope[1] + tilt[1]];
    let v = [-g[0], g[1], 1.0];
    let inv = 1.0 / (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]).sqrt();
    [v[0] * inv, v[1] * inv, v[2] * inv]
}

/// Which of a surface's `looks` the split noise `v` picks: the next look wherever `v` is above its take-over value.
pub fn split_look(v: f32, at: [f32; 2], looks: i32) -> i32 {
    i32::from(looks > 1 && v > at[0]) + i32::from(looks > 2 && v > at[1])
}

/// Whether a stone or tuft `size` metres across (a tuft: tall) shows at art pixels of 1 / `inv_texel` metres:
/// while it spans `1.5 + 2u` art pixels, `u` its seeded importance, so as the art pixel grows the items drop out one
/// by one, the least important first (A11.1 rule 3, A11.5).
pub fn cover_shows(size: f32, u: f32, inv_texel: f32) -> bool {
    size * inv_texel >= 1.5 + 2.0 * u
}

/// The art pixels between which coarse ground's map look dissolves in (A11.5, `PRE-29`): none of it at the first,
/// all of it from the second.
pub const MAP_LOOK_M: [f32; 2] = [1.2, 4.5];

/// How much of the ground the map look takes at art pixels of `texel` metres, from 0 to 1: a pixel shows it where
/// its Bayer threshold lies below this, so it dissolves in by the world-fixed pattern (A11.5).
pub fn map_weight(texel: f32) -> f32 {
    ((texel - MAP_LOOK_M[0]) / (MAP_LOOK_M[1] - MAP_LOOK_M[0])).clamp(0.0, 1.0)
}

/// The seed of the noise coarse ground picks its flat cover by (A11.5).
pub const SEED_GROUPS: u32 = 17;
/// The highest draw, just under 1, so the last group always takes it.
pub const COVER_DRAW_TOP: f32 = 0.999_999;

/// The flat cover's draw at world-fixed `w` metres, from 0 to under 1: the noise of octaves `oct`, as (λ₀, 1/λ₀, λ₁,
/// 1/λ₁), spread round a half by `spread`, so patches of each group stay where they are and keep their edges; as
/// its octaves fade below four art pixels, every draw goes to a half, and the group in the middle of a cell's shares
/// takes the ground, as in the map look (A11.5).
pub fn cover_draw(w: [f32; 2], inv_texel: f32, oct: [f32; 4], spread: f32) -> f32 {
    (0.5 + 0.5 * spread * faded_noise(w, oct, inv_texel, SEED_GROUPS)).clamp(0.0, COVER_DRAW_TOP)
}

/// The cover group a draw `u` picks from shares of trees, bushes, reeds and bare ground (grass and herbs the rest),
/// in the groups' order, trees, bushes, grass and herbs, reeds, bare ground: the first whose running total passes
/// `u` (A11.5).
pub fn cover_group(shares: [f32; 4], u: f32) -> i32 {
    let herbs = (1.0 - shares[0] - shares[1] - shares[2] - shares[3]).max(0.0);
    let mut total = 0.0;
    for (k, v) in [shares[0], shares[1], herbs, shares[2], shares[3]]
        .into_iter()
        .enumerate()
    {
        total += v;
        if u < total {
            return k as i32;
        }
    }
    4
}

/// The world-fixed grid coarse ground's crowns stand on, metres, and their seed (A11.5).
pub const CROWN_GRID_M: f32 = 5.0;
pub const SEED_CROWN: u32 = 18;

/// The crown over world-fixed `w` metres where trees take `trees` of the ground, at art pixels of 1 / `inv_texel`
/// metres (A11.5): of the 3 × 3 squares of the grid round `w`, each whose hash falls under `trees` holds one, a dome
/// of a radius within `radius` (metres, least and most), round a point in the middle half of its square, showing
/// while it spans 1.5 + 2u art pixels, as stones and tufts do; over `w` the highest wins. Its normal there (east,
/// north, up), or none.
pub fn crown_at(w: [f32; 2], trees: f32, inv_texel: f32, radius: [f32; 2]) -> Option<[f32; 3]> {
    let inv = 1.0 / CROWN_GRID_M;
    let c = [(w[0] * inv).floor(), (w[1] * inv).floor()];
    let mut best: Option<(f32, [f32; 3])> = None;
    for dy in -1..=1 {
        for dx in -1..=1 {
            let g = [c[0] + dx as f32, c[1] + dy as f32];
            let h = hash3(g[0] as i32 as u32, g[1] as i32 as u32, SEED_CROWN);
            let byte = |at: u32| ((h >> at) & 255) as f32 / 255.0;
            if byte(0) >= trees {
                continue;
            }
            let r = radius[0] + (radius[1] - radius[0]) * byte(8);
            if !cover_shows(2.0 * r, byte(16), inv_texel) {
                continue;
            }
            let nib = |at: u32| ((h >> at) & 15) as f32 / 15.0;
            let centre = [
                (g[0] + 0.25 + 0.5 * nib(24)) * CROWN_GRID_M,
                (g[1] + 0.25 + 0.5 * nib(28)) * CROWN_GRID_M,
            ];
            let d = [w[0] - centre[0], w[1] - centre[1]];
            let d2 = d[0] * d[0] + d[1] * d[1];
            if d2 >= r * r {
                continue;
            }
            let z = (r * r - d2).sqrt();
            if best.is_none_or(|(top, _)| z > top) {
                best = Some((z, [d[0] / r, -d[1] / r, z / r]));
            }
        }
    }
    best.map(|b| b.1)
}

/// How much of the light falling on water comes back to the eye, from 0 to 1, for water `depth` metres deep seen at
/// `cos_view`, the cosine of the view's angle from straight down (A11.6): the bed's light darkened by the depth, by
/// `e` over `water[1]` metres down and back up, over a floor `water[0]` of light scattered in the water, mixed by
/// Fresnel's term (Schlick's, 0.02 looking straight down) with the sky's light the surface reflects.
pub fn water_light(depth: f32, cos_view: f32, water: [f32; 2]) -> f32 {
    let c = 1.0 - cos_view.clamp(0.0, 1.0);
    let fresnel = 0.02 + 0.98 * (c * c * c * c * c);
    let below = water[0] + (1.0 - water[0]) * m::exp(-depth.max(0.0) / water[1]);
    (1.0 - fresnel) * below + fresnel
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-29 PRE-03
    #[test]
    fn map_look_dissolves_in_between_its_art_pixels() {
        assert_eq!(map_weight(0.03), 0.0);
        assert_eq!(map_weight(MAP_LOOK_M[0]), 0.0);
        assert_eq!(map_weight(MAP_LOOK_M[1]), 1.0);
        assert_eq!(map_weight(37.0), 1.0);
        let mut last = 0.0;
        for k in 0..=100 {
            let w = map_weight(1.0 + 0.04 * k as f32);
            assert!(w >= last);
            last = w;
        }
        // Each pixel of a 4 x 4 block takes the map look in its Bayer order: none at the first, all at the second.
        let showing = |texel: f32| {
            (0..4)
                .flat_map(|y| (0..4).map(move |x| (x, y)))
                .filter(|&(x, y)| bayer(x, y) < map_weight(texel))
                .count()
        };
        assert_eq!((showing(1.2), showing(2.85), showing(4.5)), (0, 8, 16));
    }

    // checks: PRE-03 PRE-29
    #[test]
    fn cover_groups_take_their_shares() {
        // Draws spread evenly over 0 to 1 pick each group in its share; the herbs take what the others leave.
        let shares = [0.55, 0.15, 0.02, 0.03];
        let mut counts = [0usize; 5];
        let n = 100_000;
        for k in 0..n {
            counts[cover_group(shares, (k as f32 + 0.5) / n as f32) as usize] += 1;
        }
        for (count, want) in counts.iter().zip([0.55, 0.15, 0.25, 0.02, 0.03]) {
            assert!((*count as f32 / n as f32 - want).abs() < 1e-3, "{counts:?}");
        }
        // The top draw goes to the last group with any share; a cell of bare ground is all bare.
        assert_eq!(cover_group(shares, COVER_DRAW_TOP), 4);
        assert_eq!(cover_group([0.0, 0.0, 0.0, 1.0], 0.0), 4);
        // The draw is world-fixed and within its range; as its octaves fade, every draw goes to a half, so the
        // group in the middle of the shares takes the ground.
        let w = [1234.5, 678.25];
        let oct = [24.0, 1.0 / 24.0, 6.0, 1.0 / 6.0];
        assert_eq!(cover_draw(w, 1.0, oct, 1.2), cover_draw(w, 1.0, oct, 1.2));
        for k in 0..1000 {
            let u = cover_draw([k as f32 * 3.7, k as f32 * 1.3], 1.0, oct, 1.2);
            assert!((0.0..=COVER_DRAW_TOP).contains(&u));
        }
        assert_eq!(cover_draw(w, 1e-3, oct, 1.2), 0.5);
        assert_eq!(cover_group(shares, cover_draw(w, 1e-3, oct, 1.2)), 0);
    }

    // checks: PRE-03 PRE-46
    #[test]
    fn crowns_stand_where_trees_do() {
        // Over 200 m square of points every half metre: no crown without trees, more ground under crowns as trees
        // grow denser, each crown's normal a unit facing up, and none once a crown spans under 1.5 art pixels.
        let cover = |trees: f32, inv_texel: f32| {
            let mut n = 0;
            for j in 0..400 {
                for i in 0..400 {
                    let w = [100.0 + i as f32 * 0.5, 300.0 + j as f32 * 0.5];
                    if let Some(c) = crown_at(w, trees, inv_texel, [2.0, 2.8]) {
                        let len = (c[0] * c[0] + c[1] * c[1] + c[2] * c[2]).sqrt();
                        assert!((len - 1.0).abs() < 1e-4 && c[2] >= 0.0);
                        n += 1;
                    }
                }
            }
            n as f32 / 160_000.0
        };
        let close = 10.0;
        assert_eq!(cover(0.0, close), 0.0);
        let (sparse, dense) = (cover(0.2, close), cover(0.8, close));
        assert!(
            sparse > 0.05 && dense > 2.5 * sparse && dense < 0.95,
            "{sparse} {dense}"
        );
        // All still stand at the camp stop; beyond it they fade one by one, the least important first.
        let (camp, mid, far) = (cover(0.8, 1.0 / 1.1), cover(0.8, 1.0 / 1.8), cover(0.8, 1.0 / 2.6));
        assert!(
            camp == dense && mid < camp && far < mid && far > 0.0,
            "{dense} {camp} {mid} {far}"
        );
        assert_eq!(cover(0.8, 1.0 / 4.0), 0.0);
        // World-fixed: the same place gives the same crown.
        let w = [2000.25, 1500.75];
        assert_eq!(crown_at(w, 0.6, close, [2.0, 2.8]), crown_at(w, 0.6, close, [2.0, 2.8]));
    }

    // checks: PRE-26 PRE-30
    #[test]
    fn water_darkens_with_depth_and_mirrors_at_a_slant() {
        // Straight down, still water a hand deep sends back nearly all the light, deep water its floor, and between
        // it darkens with depth; Fresnel's term lifts it toward a mirror as the view grazes it.
        let water = [0.25, 4.75];
        assert!((water_light(0.0, 1.0, water) - 1.0).abs() < 1e-6);
        let deep = water_light(200.0, 1.0, water);
        assert!((deep - (0.98 * 0.25 + 0.02)).abs() < 1e-4, "{deep}");
        let mut last = 2.0;
        for d in [0.0, 0.5, 1.2, 4.0, 10.0, 40.0] {
            let v = water_light(d, 1.0, water);
            assert!(v < last, "{d} m");
            last = v;
        }
        // At its depth scale the bed's share has fallen by e.
        let at_scale = water_light(4.75, 1.0, water);
        assert!((at_scale - (0.98 * (0.25 + 0.75 / std::f32::consts::E) + 0.02)).abs() < 1e-4);
        assert!(water_light(20.0, 0.2, water) > water_light(20.0, 0.9, water));
        assert!((water_light(20.0, 0.0, water) - 1.0).abs() < 1e-6);
    }

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

    // checks: PRE-20 PRE-22
    #[test]
    fn coverage_takes_the_largest_share() {
        // A strip of rock 2 m wide across grass, and one square of dirt on its own, 16 m to a side.
        let side = 16;
        let (grass, dirt, rock) = (0, 1, 2);
        let surfaces: Vec<u8> = (0..side * side)
            .map(|i| match (i % side, i / side) {
                (6 | 7, _) => rock,
                (12, 3) => dirt,
                _ => grass,
            })
            .collect();
        let c = crate::ground::Coverage::new(&surfaces, side).unwrap();
        assert_eq!(c.ids[..4], [0, 1, 2, -1]);
        let shares = |q: [f32; 2], level: f32| cover_sample(&c.levels, side, q, level);
        let at = |q: [f32; 2], level: f32| cover_pick(shares(q, level), c.ids);
        // Read square by square, each square is its own surface and an edge lies half way between two squares'
        // centres; at the exact middle the tie goes to the lower number.
        assert_eq!(at([6.5, 8.0], 0.0), i32::from(rock));
        assert_eq!(at([5.9, 8.0], 0.0), i32::from(grass));
        assert_eq!(at([6.1, 8.0], 0.0), i32::from(rock));
        assert_eq!(at([6.0, 8.0], 0.0), i32::from(grass));
        assert_eq!(at([12.5, 3.5], 0.0), i32::from(dirt));
        // Read 4 m a texel, the lone square is a sixteenth of its texel and the grass round it wins; the strip, half
        // its texel's width, still shows 2 m a texel (no detail finer than two art pixels, A11.1 rule 2).
        assert_eq!(at([12.5, 3.5], 2.0), i32::from(grass));
        assert_eq!(at([7.0, 8.0], 1.0), i32::from(rock));
        // Between levels the shares mix, so the dirt fades as the footprint grows, with no jump at a whole level.
        let mut last = 1.0f32;
        for k in 0..=300 {
            let v = shares([12.5, 3.5], k as f32 / 100.0)[1];
            assert!(
                v <= last + 1e-6 && last - v < 0.05,
                "level {}: {v} after {last}",
                k as f32 / 100.0
            );
            last = v;
        }
        // An edge read between levels runs smooth: crossing it, the pick changes once.
        let picks: Vec<i32> = (0..=75).map(|k| at([4.0 + k as f32 * 0.04, 8.0], 1.5)).collect();
        assert_eq!(picks.windows(2).filter(|w| w[0] != w[1]).count(), 1, "{picks:?}");
        // The margin is the winner's share less the next one's.
        assert!((cover_margin(shares([5.9, 8.0], 0.0), c.ids) - 0.2).abs() < 1e-5);
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

    // checks: PRE-20 PRE-22
    #[test]
    fn relief_fades_below_two_pixels() {
        // Each surface's relief from the catalogue: fixed to the world, and at every point within what its octaves
        // still showing allow, a bound that only shrinks as the art pixel grows, reaching 0 once the largest octave
        // spans two art pixels or fewer; on average strong close up and fading as the camera rises.
        for s in &crate::tests::catalogue().surfaces {
            let [smallest, largest] = s.relief_m;
            let n = relief_octaves(smallest, largest);
            assert!((1..=RELIEF_OCTAVES).contains(&n), "{}", s.id);
            let relief = [largest, 1.0 / largest, n as f32, s.relief_tilt];
            let bound = |texel: f32| {
                let showing: f32 = (0..n)
                    .map(|k| octave_fade(largest / (1 << k) as f32, 1.0 / texel))
                    .sum();
                s.relief_tilt * showing
            };
            let rms = |texel: f32| {
                let sum: f32 = (0..1_000)
                    .map(|k| {
                        let w = [100.0 + k as f32 * 3.37, 200.0 + k as f32 * 1.91];
                        let t = relief_tilt(w, relief, 1.0 / texel);
                        assert!(
                            t.iter().all(|v| v.abs() <= bound(texel) + 1e-6),
                            "{} at {texel} m",
                            s.id
                        );
                        assert_eq!(t, relief_tilt(w, relief, 1.0 / texel));
                        t[0] * t[0] + t[1] * t[1]
                    })
                    .sum();
                (sum / 1_000.0).sqrt()
            };
            let close = rms(0.02);
            assert!(close > 0.1 * s.relief_tilt, "{}: {close}", s.id);
            let (mut last_bound, mut last_rms) = (bound(0.02), close);
            for k in 1..=40 {
                let texel = 0.02 * 1.12f32.powi(k);
                let (b, r) = (bound(texel), rms(texel));
                assert!(b <= last_bound, "{} at {texel} m: bound {b} after {last_bound}", s.id);
                assert!(
                    r <= last_rms * 1.05 + 1e-6,
                    "{} at {texel} m: {r} after {last_rms}",
                    s.id
                );
                (last_bound, last_rms) = (b, r);
            }
            assert_eq!(bound(largest / 2.0), 0.0, "{}", s.id);
            assert_eq!(rms(largest / 2.0), 0.0, "{}", s.id);
        }
        // The octaves run from the largest wavelength halving down to the smallest.
        assert_eq!(
            [
                relief_octaves(0.4, 1.6),
                relief_octaves(0.3, 1.0),
                relief_octaves(0.2, 2.0),
                relief_octaves(0.2, 0.8)
            ],
            [3, 2, 4, 3]
        );
        // A tilt leans the normal, which stays a unit vector.
        let n = ground_normal([0.2, -0.1], [0.3, 0.0]);
        assert!(n[0] < -0.4 && (n.iter().map(|v| v * v).sum::<f32>() - 1.0).abs() < 1e-6);
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

    // checks: PRE-21
    #[test]
    fn outline_only_where_something_stands_in_front() {
        // A row of pixels across a ridge in front of lower ground 30 m farther: the ridge's last pixel is outlined
        // toward the ground behind it, and no other pixel is.
        let depths = [100.0, 101.0, 102.0, 103.0, 133.0, 134.0, 135.0];
        let gap = OUTLINE_GAP_M[Cat::Ground as usize];
        let outlined: Vec<usize> = (1..depths.len() - 1)
            .filter(|&i| {
                outline_toward(depths[i], depths[i + 1], depths[i - 1], gap)
                    || outline_toward(depths[i], depths[i - 1], depths[i + 1], gap)
            })
            .collect();
        assert_eq!(outlined, vec![3]);
        // Behind is not in front: the farther pixel across the step is not outlined toward the nearer.
        assert!(!outline_toward(133.0, 103.0, 134.0, gap));
        // A gap a little over the category's is outlined, one under is not; rock's gap is finer than the ground's.
        assert!(outline_toward(10.0, 16.5, 10.0, gap) && !outline_toward(10.0, 15.5, 10.0, gap));
        let rock = OUTLINE_GAP_M[Cat::Rock as usize];
        assert!(outline_toward(10.0, 12.0, 10.0, rock) && !outline_toward(10.0, 12.0, 10.0, gap));
        // Void is never outlined, as no gap is that large.
        assert!(!outline_toward(10.0, 1e6, 10.0, OUTLINE_GAP_M[Cat::Void as usize]));
    }

    // checks: PRE-21
    #[test]
    fn steep_slope_seen_edge_on_has_no_outline() {
        // A cliff's face seen nearly edge-on: each pixel lies 20 m beyond the last, far more than the ground's gap,
        // but on one plane, so nothing is outlined; where the face meets the plain the plane bends, and a bend
        // within the gap is not outlined either.
        let face: Vec<f32> = (0..8).map(|i| 50.0 + 20.0 * i as f32).collect();
        let gap = OUTLINE_GAP_M[Cat::Rock as usize];
        for i in 1..face.len() - 1 {
            assert!(!outline_toward(face[i], face[i + 1], face[i - 1], gap), "pixel {i}");
            assert!(!outline_toward(face[i], face[i - 1], face[i + 1], gap), "pixel {i}");
        }
        assert!(!outline_toward(70.0, 91.0, 50.0, gap));
        // A plain depth jump would have flagged every pixel of it.
        assert!(face.windows(2).all(|w| w[1] - w[0] > gap));
    }

    // checks: PRE-30
    #[test]
    fn haze_grows_with_the_air_path() {
        // The air of a fine day, at the camp stop's eye: haze grows with the ray's length and falls with height.
        let air = crate::tests::catalogue().air;
        let (beta, scale) = crate::light::haze_air(&air, air.turbidity);
        let eye = [1.37 * 300.0, (52.0f32).to_radians().sin()];
        let at = |depth: f32, height: f32| haze(depth, height, beta, scale, eye);
        let mut last = 0.0;
        for d in [-300.0, 0.0, 300.0, 1_000.0, 5_000.0, 20_000.0] {
            let h = at(d, 300.0);
            assert!(h > last && h < 1.0, "{d} m: {h}");
            last = h;
        }
        assert!(at(500.0, 2_000.0) < at(500.0, 300.0));
        // At the camp stop the air is all but clear; across a valley's 10 km it gives depth.
        assert_eq!(haze_level(at(0.0, 300.0), air.haze_levels, 0.0, 0, 0), 0);
        let valley = haze(0.0, 300.0, beta, scale, [1.37 * 10_000.0, 1.0]);
        assert!(haze_level(valley, air.haze_levels, 0.0, 0, 0) >= 1, "{valley}");
        // Levels climb through their thresholds, dithered only within the band.
        let levels = air.haze_levels;
        assert_eq!(haze_level(levels[1] + 0.01, levels, 0.0, 0, 0), 2);
        let mixed: i32 = (0..16).map(|p| haze_level(levels[0], levels, 0.02, p % 4, p / 4)).sum();
        assert_eq!(mixed, 8);
        assert!(haze_margin(levels[2] + 0.0005, levels, 0.0, 0, 0) < 0.001);
    }

    // checks: PRE-30
    #[test]
    fn haze_warmer_toward_the_sun() {
        // Late in the afternoon, the haze tables seen looking toward the low sun in the west map the ground's
        // colours to warmer ones than looking away from it.
        let cat = crate::tests::catalogue();
        let layout = crate::looks::Layout::new(&cat).unwrap();
        let light = crate::light::light(&cat.air, &crate::light::tests::sky_at(17.5));
        let pitch = (38.0f32).to_radians();
        let view = |east: f32| [east * pitch.cos(), 0.0, -pitch.sin()];
        let warmth = |p: &crate::looks::Palette| {
            let (mut red, mut blue) = (0.0, 0.0);
            for i in layout.fixed..layout.len {
                let c = p.row[usize::from(p.tables[crate::looks::table::HAZE[2]][i])];
                red += f32::from(c[0]);
                blue += f32::from(c[2]);
            }
            red / blue
        };
        let toward = crate::looks::Palette::new(&cat, &layout, &light, view(-1.0));
        let away = crate::looks::Palette::new(&cat, &layout, &light, view(1.0));
        assert!(
            warmth(&toward) > warmth(&away),
            "{} and {}",
            warmth(&toward),
            warmth(&away)
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
