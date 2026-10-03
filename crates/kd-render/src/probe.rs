//! The probe scene (A11.13 rule 2): fixed inputs drawn one art pixel each through the shaders' per-pixel formulas into
//! a small target, read back and compared with the Rust twins (`pixel`) exactly; headless Chromium runs it every
//! alpha, and the phone in its self-check. Its lowest third holds the light's steps (α01a), from α01c lit by what a
//! point's fields and normal give (the sky and sun factors); its middle third the surface with the largest share of
//! a fixture's coverage, read at a mip level at a place the edges' noise moved, and the look the split noise picks
//! (α01b, from α01d by coverage); its third band, from α01c, whether the plane test outlines a pixel and the
//! haze's level (A11.2, A11.4); its fourth band, from α01d, the light's step of a point whose normal a surface's
//! micro-relief tilts (A11.5); its top band, from α02a, coarse ground's answers: whether a crown stands over a point,
//! the cover group the flat cover's draw picks, and the water's step (A11.5, A11.6).
//!
//! Implements PRE-20 and PRE-01, see A11.13: the GPU picks exactly the steps, surfaces and looks the twins pick.

use kd_core::num::hash2;

use crate::RenderError;
use crate::camera::View;
use crate::gl::{self, Format, Program, State, Target, Texture, unit};
use crate::ground::Coverage;
use crate::ground::coarse::CoarseLook;
use crate::pixel::{
    CROWN_GRID_M, OUTLINE_GAP_M, SEED_CROWN, SEED_SPLIT, SUN_TAN, cover_draw, cover_group, cover_level, cover_margin,
    cover_pick, cover_sample, crown_at, edge_wobble, faded_noise, ground_normal, hash3, haze, haze_level, haze_margin,
    ladder_pos, light_step, lightness, margin, outline_toward, relief_octaves, relief_tilt, sky_factor, split_look,
    sun_factor, water_light,
};
use crate::shaders::{self, Stage};

/// Each band of the probe target, one input an art pixel; the target is `W` × `4H`.
pub const W: u32 = 16;
pub const H: u32 = 16;
pub const N: usize = (W * H) as usize;
/// Rows of the input texture: the light's 7, then the surfaces' `SURFACE_ROWS`, then the edges' and haze's
/// `EDGE_ROWS`, then the relief's `RELIEF_ROWS`, then coarse ground's `COARSE_ROWS`.
pub const LIGHT_ROWS: usize = 7;
pub const SURFACE_ROWS: usize = 11;
pub const EDGE_ROWS: usize = 7;
pub const RELIEF_ROWS: usize = 17;
pub const COARSE_ROWS: usize = 12;
/// The bands, bottom to top.
pub const BANDS: u32 = 5;
/// How far from deciding otherwise every plane test stays, in metres, so the GPU's rounding of depths hundreds of
/// metres off cannot change it.
pub const EDGE_CLEARANCE: f32 = 0.01;
/// The probe's air (A11.4): the aerosol's and the air's extinction a metre at the sea's level, their scale
/// heights, the eye's distance and climb, and where the haze's levels begin.
pub const PROBE_BETA: [f32; 2] = [3e-4, 1.2e-5];
pub const PROBE_SCALE: [f32; 2] = [1200.0, 8000.0];
pub const PROBE_EYE: [f32; 2] = [2000.0, 0.3];
pub const PROBE_LEVELS: [f32; 3] = [0.1, 0.25, 0.45];
/// The probe's own numbers for coarse ground's formulas, as it has its own air: the catalogue's at α02a.
pub const PROBE_LOOK: CoarseLook = CoarseLook {
    crown_r: [2.0, 2.8],
    cover_oct: [24.0, 1.0 / 24.0, 6.0, 1.0 / 6.0],
    cover_spread: 1.2,
    water: [0.25, 4.75],
};
/// How far from deciding otherwise every surface input stays, in shares of the coverage or in noise.
pub const SURFACE_CLEARANCE: f32 = 1e-3;
/// The surface band's fixture: a coverage 16 m to a side, so its levels run from 0 to 4.
pub const COVER_SIDE: usize = 16;
pub const COVER_TOP: usize = 4;
/// The probe's light: the luminance of the sky and of the sun facing it.
pub const Y_SKY: f32 = 0.12;
pub const Y_SUN: f32 = 0.85;
/// How far, in lightness, every input stays from where its step flips, so a GPU's rounding cannot flip it (the
/// risk α01a names).
pub const CLEARANCE: f32 = 1e-4;

/// One fixed input: what a point's fields and normal give (the share of the sky its horizon leaves open, the
/// normal's upward part, its horizon toward the light, the light's slope, and `n·l`), the band's half-width in
/// steps, and the ladder's number of steps.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Input {
    pub open: f32,
    pub n_up: f32,
    pub horizon: f32,
    pub tan_e: f32,
    pub n_dot_l: f32,
    pub band: f32,
    pub steps: i32,
}

impl Input {
    /// Its sky and sun factors, σ and τ, as the twins give them.
    pub fn factors(&self) -> (f32, f32) {
        (
            sky_factor(self.open, self.n_up),
            sun_factor(self.horizon, self.tan_e, self.n_dot_l),
        )
    }
}

/// The path's range under the probe's light: deep shade to full sun.
pub fn range() -> [f32; 2] {
    [lightness(0.3, 0.0, Y_SKY, Y_SUN), lightness(1.0, 1.0, Y_SKY, Y_SUN)]
}

/// Input `i`'s pixel, row by row from the target's bottom, as `gl_FragCoord` counts.
fn pixel_of(i: usize) -> (i32, i32) {
    ((i as u32 % W) as i32, (i as u32 / W) as i32)
}

/// The probe's inputs: drawn from a fixed sequence of hashes, kept only when `CLEARANCE` from any flip. Half the
/// horizons lie within the sun's disc of the light, where its share is partly shown.
pub fn inputs() -> Vec<Input> {
    let range = range();
    let mut out = Vec::with_capacity(N);
    let mut n = 0u64;
    while out.len() < N {
        let (h, g) = (hash2(0x0070_726f_6265, n), hash2(0x0070_726f_6266, n));
        n += 1;
        let unit = |v: u64, shift: u32| ((v >> shift) & 0xffff) as f32 / 65536.0;
        let tan_e = 3.0 * unit(g, 0) - 0.2;
        let input = Input {
            open: unit(h, 0),
            n_up: unit(h, 16),
            horizon: if g >> 63 == 1 {
                tan_e + SUN_TAN * (1.0 + tan_e * tan_e) * (unit(g, 16) - 0.5)
            } else {
                3.5 * unit(g, 16) - 0.5
            },
            tan_e,
            n_dot_l: 1.2 * unit(g, 32) - 0.2,
            band: [0.0, 0.15, 0.35][((h >> 32) % 3) as usize],
            steps: 4 + ((h >> 40) % 4) as i32,
        };
        let (x, y) = pixel_of(out.len());
        let (sigma, tau) = input.factors();
        let s = ladder_pos(lightness(sigma, tau, Y_SKY, Y_SUN), range, input.steps);
        let clear = margin(s, input.band, input.steps, x, y) * (range[1] - range[0]) / (input.steps - 1) as f32;
        if clear >= CLEARANCE {
            out.push(input);
        }
    }
    out
}

/// The twins' step for each input, in the target's order.
pub fn twins(inputs: &[Input]) -> Vec<u8> {
    let range = range();
    inputs
        .iter()
        .enumerate()
        .map(|(i, p)| {
            let (x, y) = pixel_of(i);
            let (sigma, tau) = p.factors();
            let s = ladder_pos(lightness(sigma, tau, Y_SKY, Y_SUN), range, p.steps);
            light_step(s, p.band, p.steps, x, y) as u8
        })
        .collect()
}

/// One fixed surface input: a place on the fixture's coverage (metres east and south of its corner), the
/// world-fixed place the noises read, art pixels a metre and the coverage's level they read, and a split: its
/// take-over value and its octaves as (λ₀, 1/λ₀, λ₁, 1/λ₁).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct SurfaceInput {
    pub q: [f32; 2],
    pub w: [f32; 2],
    pub inv_texel: f32,
    pub level: f32,
    pub split_at: f32,
    pub oct: [f32; 4],
}

/// The surface band's coverage: eight surfaces in blocks of 4 m, each twice, sprinkled with single squares of
/// others, so every surface wins at every level and some texels are near ties.
pub fn cover_fixture() -> Coverage {
    let surfaces: Vec<u8> = (0..COVER_SIDE * COVER_SIDE)
        .map(|i| {
            let (x, y) = (i % COVER_SIDE, i / COVER_SIDE);
            let h = hash2(0x636f_7672, i as u64);
            if h.is_multiple_of(9) {
                ((h >> 8) % 8) as u8
            } else {
                ((x / 4 + 2 * (y / 4)) % 8) as u8
            }
        })
        .collect();
    Coverage::new(&surfaces, COVER_SIDE).expect("eight surfaces fit")
}

/// What a surface input gives, as the GPU and the twins work it out: the surface, its look, and how far each was
/// from going the other way.
fn surface_answer(p: &SurfaceInput, fixture: &Coverage) -> (i32, i32, f32) {
    let wobble = edge_wobble(p.w, p.inv_texel);
    let q = [p.q[0] + wobble[0], p.q[1] + wobble[1]];
    let shares = cover_sample(&fixture.levels, fixture.side, q, p.level);
    let surface = cover_pick(shares, fixture.ids);
    let v = faded_noise(p.w, p.oct, p.inv_texel, SEED_SPLIT);
    let look = split_look(v, [p.split_at, 2.0], 2);
    (
        surface,
        look,
        cover_margin(shares, fixture.ids).min((v - p.split_at).abs()),
    )
}

/// Whether a surface input lies `SURFACE_CLEARANCE` or more from deciding otherwise, so a GPU's rounding cannot
/// change its answer.
pub fn clear(p: &SurfaceInput, fixture: &Coverage) -> bool {
    surface_answer(p, fixture).2 >= SURFACE_CLEARANCE
}

/// The surface inputs: drawn from a fixed sequence of hashes, kept only when clear of deciding otherwise.
pub fn surface_inputs() -> Vec<SurfaceInput> {
    let fixture = cover_fixture();
    let mut out = Vec::with_capacity(N);
    let mut n = 0u64;
    while out.len() < N {
        let h = hash2(0x7375_7266, n);
        let g = hash2(0x7375_7267, n);
        n += 1;
        let unit = |v: u64, shift: u32| ((v >> shift) & 0xffff) as f32 / 65536.0;
        // From the person stop past the camp stop, some at whole levels, some between.
        let texel = [0.03, 0.13, 0.4, 1.1, 2.0, 2.2, 4.0, 4.4, 9.0][(g % 9) as usize];
        let lambda = 2.0 + 38.0 * unit(g, 8);
        let side = COVER_SIDE as f32;
        let p = SurfaceInput {
            q: [side * unit(h, 0), side * unit(h, 16)],
            w: [8000.0 * unit(h, 32), 8000.0 * unit(h, 48)],
            inv_texel: 1.0 / texel,
            level: cover_level(texel, COVER_TOP),
            split_at: unit(g, 40) - 0.5,
            oct: if g >> 63 == 1 {
                [lambda, 1.0 / lambda, lambda / 4.0, 4.0 / lambda]
            } else {
                [lambda, 1.0 / lambda, 0.0, 0.0]
            },
        };
        if clear(&p, &fixture) {
            out.push(p);
        }
    }
    out
}

/// The twins' answer for each surface input, as the probe writes it: surface × 3 + look.
pub fn surface_twins(inputs: &[SurfaceInput]) -> Vec<u8> {
    let fixture = cover_fixture();
    inputs
        .iter()
        .map(|p| {
            let (surface, look, _) = surface_answer(p, &fixture);
            (surface * 3 + look) as u8
        })
        .collect()
}

/// One fixed input of the top third: a pixel's depth, its neighbour's and its opposite neighbour's, and its
/// category's gap, in metres; and a point's depth beyond the target's plane, its height above the sea, and the
/// haze band's half-width.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct EdgeInput {
    pub depth: f32,
    pub far: f32,
    pub opposite: f32,
    pub gap: f32,
    pub air_depth: f32,
    pub height: f32,
    pub band: f32,
}

/// Input `i` of the top third's pixel in the target, as `gl_FragCoord` counts.
fn edge_pixel(i: usize) -> (i32, i32) {
    ((i as u32 % W) as i32, (2 * H + i as u32 / W) as i32)
}

/// What a top-third input gives, as the GPU and the twins work it out: whether it is outlined, its haze level, and
/// how far each lies from deciding otherwise.
fn edge_answer(i: usize, p: &EdgeInput) -> (bool, i32, f32, f32) {
    let (x, y) = edge_pixel(i);
    let hz = haze(p.air_depth, p.height, PROBE_BETA, PROBE_SCALE, PROBE_EYE);
    (
        outline_toward(p.depth, p.far, p.opposite, p.gap),
        haze_level(hz, PROBE_LEVELS, p.band, x, y),
        (p.far - (2.0 * p.depth - p.opposite) - p.gap).abs(),
        haze_margin(hz, PROBE_LEVELS, p.band, x, y),
    )
}

/// The top third's inputs: drawn from a fixed sequence of hashes, half the plane tests either side of their gap,
/// kept only when clear of deciding otherwise.
pub fn edge_inputs() -> Vec<EdgeInput> {
    let mut out = Vec::with_capacity(N);
    let mut n = 0u64;
    while out.len() < N {
        let (h, g) = (hash2(0x6564_6765, n), hash2(0x6564_6766, n));
        n += 1;
        let unit = |v: u64, shift: u32| ((v >> shift) & 0xffff) as f32 / 65536.0;
        let gap = OUTLINE_GAP_M[1 + (h % 6) as usize];
        let depth = 1.0 + 400.0 * unit(h, 8);
        let opposite = depth - 60.0 * (unit(h, 24) - 0.5);
        let p = EdgeInput {
            depth,
            far: 2.0 * depth - opposite + gap * 2.0 * unit(h, 40),
            opposite,
            gap,
            air_depth: 20_000.0 * unit(g, 0) - 500.0,
            height: 3_000.0 * unit(g, 16),
            band: [0.0, 0.01, 0.03][(g >> 32) as usize % 3],
        };
        let (_, _, edge, air) = edge_answer(out.len(), &p);
        if edge >= EDGE_CLEARANCE && air >= CLEARANCE {
            out.push(p);
        }
    }
    out
}

/// The twins' answer for each top-third input, as the probe writes it: level × 2 + outlined.
pub fn edge_twins(inputs: &[EdgeInput]) -> Vec<u8> {
    inputs
        .iter()
        .enumerate()
        .map(|(i, p)| {
            let (outline, level, _, _) = edge_answer(i, p);
            (level * 2 + i32::from(outline)) as u8
        })
        .collect()
}

/// One fixed relief input (the top band): a point lit as the ground lights it, its normal from the ground's slope
/// east and south tilted by a surface's micro-relief at a world-fixed place: the share of the sky its horizon leaves
/// open, its horizon toward the light and the light's slope and direction (east, north, up), the slope, the place
/// (metres), art pixels a metre, the relief (λ₀, 1/λ₀, octaves, greatest tilt), the band and the steps.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ReliefInput {
    pub open: f32,
    pub horizon: f32,
    pub tan_e: f32,
    pub light: [f32; 3],
    pub slope: [f32; 2],
    pub w: [f32; 2],
    pub inv_texel: f32,
    pub relief: [f32; 4],
    pub band: f32,
    pub steps: i32,
}

/// Relief input `i`'s pixel in the target, as `gl_FragCoord` counts: the top band.
fn relief_pixel(i: usize) -> (i32, i32) {
    let (x, y) = pixel_of(i);
    (x, y + 3 * H as i32)
}

/// What a relief input gives, as the GPU and the twins work it out: its step, and how far, in lightness, it lies
/// from another.
fn relief_answer(i: usize, p: &ReliefInput) -> (u8, f32) {
    let (x, y) = relief_pixel(i);
    let n = ground_normal(p.slope, relief_tilt(p.w, p.relief, p.inv_texel));
    let n_dot_l = n[0] * p.light[0] + n[1] * p.light[1] + n[2] * p.light[2];
    let range = range();
    let lit = lightness(
        sky_factor(p.open, n[2]),
        sun_factor(p.horizon, p.tan_e, n_dot_l),
        Y_SKY,
        Y_SUN,
    );
    let s = ladder_pos(lit, range, p.steps);
    let clear = margin(s, p.band, p.steps, x, y) * (range[1] - range[0]) / (p.steps - 1) as f32;
    (light_step(s, p.band, p.steps, x, y) as u8, clear)
}

/// The relief inputs: the wavelengths of the catalogue's four reliefs at tilts up to 0.5, stronger than the
/// catalogue's so more steps turn on them, at art pixels from 0.02 to 0.3 m, under lights from below the horizon to
/// high, drawn from a fixed sequence of hashes and kept only when `CLEARANCE` from any flip.
pub fn relief_inputs() -> Vec<ReliefInput> {
    let reliefs = [(0.4, 1.6, 0.35), (0.3, 1.0, 0.3), (0.2, 2.0, 0.5), (0.2, 0.8, 0.45)];
    let mut out = Vec::with_capacity(N);
    let mut n = 0u64;
    while out.len() < N {
        let (h, g, k) = (hash2(0x7265_6c69, n), hash2(0x7265_6c6a, n), hash2(0x7265_6c6b, n));
        n += 1;
        let unit = |v: u64, shift: u32| ((v >> shift) & 0xffff) as f32 / 65536.0;
        let (smallest, largest, tilt) = reliefs[(g % 4) as usize];
        let e = (85.0 * unit(g, 8) - 5.0).to_radians();
        let az = std::f32::consts::TAU * unit(g, 24);
        let tan_e = e.tan();
        let p = ReliefInput {
            open: 0.2 + 0.8 * unit(h, 0),
            horizon: if g >> 63 == 1 {
                tan_e + SUN_TAN * (1.0 + tan_e * tan_e) * (unit(g, 40) - 0.5)
            } else {
                3.5 * unit(g, 40) - 0.5
            },
            tan_e,
            light: [e.cos() * az.sin(), e.cos() * az.cos(), e.sin()],
            slope: [1.6 * unit(h, 16) - 0.8, 1.6 * unit(h, 32) - 0.8],
            w: [8000.0 * unit(h, 48), 8000.0 * unit(k, 0)],
            inv_texel: 1.0 / [0.02, 0.04, 0.08, 0.15, 0.3][((k >> 16) % 5) as usize],
            relief: [largest, 1.0 / largest, relief_octaves(smallest, largest) as f32, tilt],
            band: [0.0, 0.15, 0.35][((k >> 24) % 3) as usize],
            steps: 4 + ((k >> 32) % 4) as i32,
        };
        if relief_answer(out.len(), &p).1 >= CLEARANCE {
            out.push(p);
        }
    }
    out
}

/// The twins' step for each relief input, in the target's order.
pub fn relief_twins(inputs: &[ReliefInput]) -> Vec<u8> {
    inputs.iter().enumerate().map(|(i, p)| relief_answer(i, p).0).collect()
}

/// One fixed input of coarse ground's band (the top one): a world-fixed place (metres), the shares of trees, bushes,
/// reeds and bare ground there, art pixels a metre, and water's depth, the view's cosine from straight down, the
/// light's slope and its upward part, and a ladder's steps.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct CoarseInput {
    pub w: [f32; 2],
    pub shares: [f32; 4],
    pub inv_texel: f32,
    pub depth: f32,
    pub cos_view: f32,
    pub tan_e: f32,
    pub up: f32,
    pub steps: i32,
}

/// Coarse input `i`'s pixel in the target, as `gl_FragCoord` counts: the top band.
fn coarse_pixel(i: usize) -> (i32, i32) {
    let (x, y) = pixel_of(i);
    (x, y + 4 * H as i32)
}

/// What a coarse input gives, as the GPU and the twins work it out: whether a crown stands over it, the cover group
/// the draw picks, the water's step, and how far, in their own measures, each lies from going the other way.
fn coarse_answer(i: usize, p: &CoarseInput) -> (bool, i32, u8, f32) {
    let (x, y) = coarse_pixel(i);
    let l = PROBE_LOOK;
    let crown = crown_at(p.w, p.shares[0], p.inv_texel, l.crown_r).is_some();
    // The crown's decisions: each nearby square's hash against the share, its size against its importance, and the
    // place against its edge.
    let inv = 1.0 / CROWN_GRID_M;
    let c = [(p.w[0] * inv).floor(), (p.w[1] * inv).floor()];
    let mut crown_clear = f32::INFINITY;
    for dy in -1..=1 {
        for dx in -1..=1 {
            let g = [c[0] + dx as f32, c[1] + dy as f32];
            let h = hash3(g[0] as i32 as u32, g[1] as i32 as u32, SEED_CROWN);
            let byte = |at: u32| ((h >> at) & 255) as f32 / 255.0;
            crown_clear = crown_clear.min((byte(0) - p.shares[0]).abs());
            let r = l.crown_r[0] + (l.crown_r[1] - l.crown_r[0]) * byte(8);
            crown_clear = crown_clear.min((2.0 * r * p.inv_texel - 1.5 - 2.0 * byte(16)).abs());
            let nib = |at: u32| ((h >> at) & 15) as f32 / 15.0;
            let centre = [
                (g[0] + 0.25 + 0.5 * nib(24)) * CROWN_GRID_M,
                (g[1] + 0.25 + 0.5 * nib(28)) * CROWN_GRID_M,
            ];
            let d = ((p.w[0] - centre[0]).powi(2) + (p.w[1] - centre[1]).powi(2)).sqrt();
            crown_clear = crown_clear.min((d - r).abs());
        }
    }
    let u = cover_draw(p.w, p.inv_texel, l.cover_oct, l.cover_spread);
    let group = cover_group(p.shares, u);
    let herbs = (1.0 - p.shares[0] - p.shares[1] - p.shares[2] - p.shares[3]).max(0.0);
    let mut total = 0.0;
    let mut group_clear = f32::INFINITY;
    for v in [p.shares[0], p.shares[1], herbs, p.shares[2]] {
        total += v;
        group_clear = group_clear.min((u - total).abs());
    }
    let k = water_light(p.depth, p.cos_view, l.water);
    let range = range();
    let lit = lightness(k, k * sun_factor(0.0, p.tan_e, p.up), Y_SKY, Y_SUN);
    let s = ladder_pos(lit, range, p.steps);
    let water_clear = margin(s, 0.0, p.steps, x, y) * (range[1] - range[0]) / (p.steps - 1) as f32;
    (
        crown,
        group,
        light_step(s, 0.0, p.steps, x, y) as u8,
        crown_clear.min(group_clear).min(water_clear),
    )
}

/// Coarse ground's inputs: places over 8 km at the camp stop's art pixels and closer, where crowns show and fade,
/// shares from bare to dense woods, and water from a ford to the deep sea seen from the person stop to straight down;
/// drawn from a fixed sequence of hashes and kept only when `CLEARANCE` from any flip.
pub fn coarse_inputs() -> Vec<CoarseInput> {
    let mut out = Vec::with_capacity(N);
    let mut n = 0u64;
    while out.len() < N {
        let (h, g, k) = (hash2(0x636f_6172, n), hash2(0x636f_6173, n), hash2(0x636f_6174, n));
        n += 1;
        let unit = |v: u64, shift: u32| ((v >> shift) & 0xffff) as f32 / 65536.0;
        // Shares summing to under 1, the rest grass and herbs.
        let raw = [unit(g, 0), unit(g, 16), 0.2 * unit(g, 32), 0.3 * unit(g, 48)];
        let sum: f32 = raw.iter().sum();
        let scale = if sum > 0.95 { 0.95 / sum } else { 1.0 };
        let e = (80.0 * unit(k, 32) - 5.0).to_radians();
        let p = CoarseInput {
            w: [8000.0 * unit(h, 0), 8000.0 * unit(h, 16)],
            shares: raw.map(|v| v * scale),
            inv_texel: 1.0 / [0.05, 0.13, 0.4, 0.8, 1.1, 1.6, 2.5, 4.0][(h >> 32) as usize % 8],
            depth: [0.3, 1.2, 4.0, 20.0, 150.0][((h >> 40) % 5) as usize] * (0.5 + unit(k, 0)),
            cos_view: 0.5 + 0.5 * unit(k, 16),
            tan_e: e.tan(),
            up: e.sin(),
            steps: 4 + ((h >> 48) % 4) as i32,
        };
        if coarse_answer(out.len(), &p).3 >= CLEARANCE {
            out.push(p);
        }
    }
    out
}

/// The twins' answer for each coarse input, as the probe writes it: crown × 50 + group × 10 + the water's step.
pub fn coarse_twins(inputs: &[CoarseInput]) -> Vec<u8> {
    inputs
        .iter()
        .enumerate()
        .map(|(i, p)| {
            let (crown, group, step, _) = coarse_answer(i, p);
            (i32::from(crown) * 50 + group * 10 + i32::from(step)) as u8
        })
        .collect()
}

/// All the inputs as an R32F texture `N` wide: the light's rows (the open sky, the normal's upward part, the
/// horizon, the light's slope, n·l, the band and the steps), then the surfaces' q, w, 1/texel, level, take-over
/// value and octaves, then the third band's depths, gap, air depth, height and band, then the relief's open sky,
/// horizon, light, slope, place, 1/texel, relief, band and steps, then coarse ground's place, shares, 1/texel,
/// depth, view, light and steps.
pub fn texture_bytes(
    inputs: &[Input],
    surfaces: &[SurfaceInput],
    edges: &[EdgeInput],
    reliefs: &[ReliefInput],
    coarse: &[CoarseInput],
) -> Vec<u8> {
    let light = |f: &dyn Fn(&Input) -> f32| inputs.iter().map(f).collect::<Vec<f32>>();
    let mut rows: Vec<Vec<f32>> = vec![
        light(&|p| p.open),
        light(&|p| p.n_up),
        light(&|p| p.horizon),
        light(&|p| p.tan_e),
        light(&|p| p.n_dot_l),
        light(&|p| p.band),
        light(&|p| p.steps as f32),
    ];
    let field = |f: &dyn Fn(&SurfaceInput) -> f32| surfaces.iter().map(f).collect::<Vec<f32>>();
    rows.extend([
        field(&|p| p.q[0]),
        field(&|p| p.q[1]),
        field(&|p| p.w[0]),
        field(&|p| p.w[1]),
        field(&|p| p.inv_texel),
        field(&|p| p.level),
        field(&|p| p.split_at),
    ]);
    for k in 0..4 {
        rows.push(field(&|p| p.oct[k]));
    }
    let edge = |f: &dyn Fn(&EdgeInput) -> f32| edges.iter().map(f).collect::<Vec<f32>>();
    rows.extend([
        edge(&|p| p.depth),
        edge(&|p| p.far),
        edge(&|p| p.opposite),
        edge(&|p| p.gap),
        edge(&|p| p.air_depth),
        edge(&|p| p.height),
        edge(&|p| p.band),
    ]);
    let relief = |f: &dyn Fn(&ReliefInput) -> f32| reliefs.iter().map(f).collect::<Vec<f32>>();
    rows.extend([
        relief(&|p| p.open),
        relief(&|p| p.horizon),
        relief(&|p| p.tan_e),
        relief(&|p| p.light[0]),
        relief(&|p| p.light[1]),
        relief(&|p| p.light[2]),
        relief(&|p| p.slope[0]),
        relief(&|p| p.slope[1]),
        relief(&|p| p.w[0]),
        relief(&|p| p.w[1]),
        relief(&|p| p.inv_texel),
        relief(&|p| p.relief[0]),
        relief(&|p| p.relief[1]),
        relief(&|p| p.relief[2]),
        relief(&|p| p.relief[3]),
        relief(&|p| p.band),
        relief(&|p| p.steps as f32),
    ]);
    let land = |f: &dyn Fn(&CoarseInput) -> f32| coarse.iter().map(f).collect::<Vec<f32>>();
    rows.extend([
        land(&|p| p.w[0]),
        land(&|p| p.w[1]),
        land(&|p| p.shares[0]),
        land(&|p| p.shares[1]),
        land(&|p| p.shares[2]),
        land(&|p| p.shares[3]),
        land(&|p| p.inv_texel),
        land(&|p| p.depth),
        land(&|p| p.cos_view),
        land(&|p| p.tan_e),
        land(&|p| p.up),
        land(&|p| p.steps as f32),
    ]);
    rows.iter().flatten().flat_map(|v| v.to_ne_bytes()).collect()
}

/// One frame as the crawl counter sees it (A11.10): its view, and each art pixel of the art target, bottom row
/// first, as post coloured it, with its depth along the view in metres from the world's corner, none for void.
#[derive(Clone, Debug)]
pub struct Capture {
    pub view: View,
    pub colours: Vec<[u8; 3]>,
    pub depths: Vec<Option<f64>>,
}

/// What changed between two frames (A11.10).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct CrawlCount {
    /// Art pixels whose colour changed while the surface they show moved less than one art pixel on the screen.
    pub crawl: u32,
    /// Art pixels whose colour changed where they show on the screen.
    pub changed: u32,
    /// Art pixels counted: the art target's, less a border of two.
    pub pixels: u32,
}

/// Counts what changed between frames `prev` and `next` (A11.10). Each art pixel of `next` showing ground is put
/// back through `prev`'s view, its surface found from its depth: where that surface moved less than one art pixel
/// on the screen and `prev` showed it in another colour, it crawled. Every art pixel whose colour differs from what
/// `prev` showed at the same place on the screen changed; the rest of a turn's or zoom's changes are the picture
/// really moving.
pub fn count(prev: &Capture, next: &Capture) -> CrawlCount {
    let (v, p) = (&next.view, &prev.view);
    let (w, h) = (i64::from(v.art[0]), i64::from(v.art[1]));
    let (pw, ph) = (i64::from(p.art[0]), i64::from(p.art[1]));
    let shown =
        |x: i64, y: i64| ((0..pw).contains(&x) && (0..ph).contains(&y)).then(|| prev.colours[(y * pw + x) as usize]);
    let mut out = CrawlCount::default();
    for y in 2..h - 2 {
        for x in 2..w - 2 {
            out.pixels += 1;
            let i = (y * w + x) as usize;
            let colour = next.colours[i];
            // Where the pixel's middle shows on the screen, in art pixels from the window's corner.
            let at = [
                x as f64 + 0.5 - f64::from(v.off[0]),
                y as f64 + 0.5 - f64::from(v.off[1]),
            ];
            let there = |k: usize| (at[k] + f64::from(p.off[k])).floor() as i64;
            if shown(there(0), there(1)).is_some_and(|c| c != colour) {
                out.changed += 1;
            }
            let Some(depth) = next.depths[i] else {
                continue;
            };
            // The surface the pixel shows, from its middle on the screen and its depth, and where `prev` put it.
            let s = [
                (v.corner[0] as f64 + x as f64 + 0.5) * v.texel,
                (v.corner[1] as f64 + y as f64 + 0.5) * v.texel,
            ];
            let m = [0, 1, 2].map(|k| v.right[k] * s[0] + v.up[k] * s[1] + v.fwd[k] * depth);
            let q = p.screen(m);
            let q = [q[0] - p.corner[0] as f64, q[1] - p.corner[1] as f64];
            let moved = (0..2)
                .map(|k| (q[k] - f64::from(p.off[k]) - at[k]).abs())
                .fold(0.0, f64::max);
            if moved < 1.0 && shown(q[0].floor() as i64, q[1].floor() as i64).is_some_and(|c| c != colour) {
                out.crawl += 1;
            }
        }
    }
    out
}

pub struct ProbePass {
    program: Program,
    u_y: Option<glow::UniformLocation>,
    u_range: Option<glow::UniformLocation>,
    u_cover_ids: Option<glow::UniformLocation>,
    inputs: Texture,
    /// The surface band's coverage, two textures with their levels, and the surface each channel holds.
    cover: [Texture; 2],
    cover_ids: [i32; 8],
    target: Target,
    twins: Vec<u8>,
}

impl ProbePass {
    pub fn new(gl: &glow::Context) -> Result<ProbePass, RenderError> {
        let program = Program::new(
            gl,
            "probe",
            &shaders::source(Stage::Vertex, shaders::FULL_TARGET_VERT),
            &shaders::source(Stage::Fragment, shaders::PROBE_FRAG),
        )?;
        program.set_sampler(gl, "u_inputs", unit::PROBE);
        program.set_sampler(gl, "u_cover0", unit::COVER0);
        program.set_sampler(gl, "u_cover1", unit::COVER1);
        let u_y = program.uniform(gl, "u_y");
        let u_range = program.uniform(gl, "u_range");
        let u_cover_ids = program.uniform(gl, "u_cover_ids[0]");
        let fixture = cover_fixture();
        let side = COVER_SIDE as u32;
        let cover = [
            Texture::with_levels(gl, Format::Rgba8, side, side, &fixture.texture_levels(0))?,
            Texture::with_levels(gl, Format::Rgba8, side, side, &fixture.texture_levels(1))?,
        ];
        let (list, surfaces, edges, reliefs) = (inputs(), surface_inputs(), edge_inputs(), relief_inputs());
        let coarse = coarse_inputs();
        let rows = (LIGHT_ROWS + SURFACE_ROWS + EDGE_ROWS + RELIEF_ROWS + COARSE_ROWS) as u32;
        let bytes = texture_bytes(&list, &surfaces, &edges, &reliefs, &coarse);
        let inputs = Texture::new(gl, Format::R32F, N as u32, rows, Some(&bytes))?;
        let target = Target::new(gl, W, BANDS * H, &[Format::Rgba8], false)?;
        let mut answers = twins(&list);
        answers.extend(surface_twins(&surfaces));
        answers.extend(edge_twins(&edges));
        answers.extend(relief_twins(&reliefs));
        answers.extend(coarse_twins(&coarse));
        Ok(ProbePass {
            program,
            u_y,
            u_range,
            u_cover_ids,
            inputs,
            cover,
            cover_ids: fixture.ids,
            target,
            twins: answers,
        })
    }

    /// Draws the probe and reads it back: the GPU's steps and the twins', in the same order.
    pub fn run(&self, gl: &glow::Context, vao: glow::VertexArray) -> (Vec<u8>, Vec<u8>) {
        self.target.bind(gl);
        gl::apply(gl, &State::flat(W, BANDS * H));
        self.program.bind(gl);
        self.inputs.bind(gl, unit::PROBE);
        self.cover[0].bind(gl, unit::COVER0);
        self.cover[1].bind(gl, unit::COVER1);
        let ids = self.cover_ids;
        gl::set_ivec4_array(
            gl,
            self.u_cover_ids.as_ref(),
            &[[ids[0], ids[1], ids[2], ids[3]], [ids[4], ids[5], ids[6], ids[7]]],
        );
        gl::set_vec2(gl, self.u_y.as_ref(), [Y_SKY, Y_SUN]);
        gl::set_vec2(gl, self.u_range.as_ref(), range());
        gl::draw_full_target(gl, vao);
        let pixels = self.target.read_rgba8(gl);
        gl::bind_window(gl);
        (pixels.chunks(4).map(|p| p[0]).collect(), self.twins.clone())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::pixel::sunlit;

    // checks: PRE-20
    #[test]
    fn inputs_cover_every_case_clear_of_flips() {
        let list = inputs();
        assert_eq!(list.len(), N);
        // Every ladder length, every band, steps from the bottom to the top, and some pixels inside a band.
        for steps in 4..=7 {
            assert!(list.iter().any(|p| p.steps == steps), "no ladder of {steps}");
        }
        for band in [0.0, 0.15, 0.35] {
            assert!(list.iter().any(|p| p.band == band));
        }
        let steps = twins(&list);
        assert!(steps.contains(&0) && steps.iter().zip(&list).any(|(&s, p)| i32::from(s) == p.steps - 1));
        // Points in full sun, in shadow though facing the light, and in a shadow's soft edge; and some facing away.
        let share = |p: &Input| sunlit(p.horizon, p.tan_e);
        assert!(list.iter().any(|p| share(p) == 1.0 && p.n_dot_l > 0.0));
        assert!(list.iter().any(|p| share(p) == 0.0 && p.n_dot_l > 0.0));
        assert!(
            list.iter()
                .filter(|p| share(p) > 0.05 && share(p) < 0.95 && p.n_dot_l > 0.1)
                .count()
                > 20
        );
        assert!(list.iter().any(|p| p.n_dot_l < 0.0));
        // The same inputs every time, on every target: a fixed sequence.
        assert_eq!(inputs(), list);
        let (surfaces, edges, reliefs) = (surface_inputs(), edge_inputs(), relief_inputs());
        assert_eq!(
            texture_bytes(&list, &surfaces, &edges, &reliefs, &coarse_inputs()).len(),
            N * (LIGHT_ROWS + SURFACE_ROWS + EDGE_ROWS + RELIEF_ROWS + COARSE_ROWS) * 4
        );
    }

    // checks: PRE-20 PRE-22
    #[test]
    fn relief_inputs_cover_every_case() {
        // Every one of the catalogue's reliefs, octaves shown whole and fading, light from below the horizon to
        // high, and many points whose step the relief's tilt changes, so the band tests the relief and not only the
        // light; all clear of a flip, and the same every time.
        let list = relief_inputs();
        assert_eq!(list.len(), N);
        for largest in [1.6, 1.0, 2.0, 0.8] {
            assert!(list.iter().any(|p| p.relief[0] == largest), "no relief of {largest} m");
        }
        let fade = |p: &ReliefInput| crate::pixel::octave_fade(p.relief[0] / 4.0, p.inv_texel);
        assert!(list.iter().any(|p| fade(p) == 1.0) && list.iter().any(|p| fade(p) > 0.0 && fade(p) < 1.0));
        assert!(list.iter().any(|p| p.tan_e < 0.0) && list.iter().any(|p| p.tan_e > 2.0));
        let flat: Vec<ReliefInput> = list
            .iter()
            .map(|p| ReliefInput {
                relief: [1.0, 1.0, 0.0, 0.0],
                ..*p
            })
            .collect();
        let (with, without) = (relief_twins(&list), relief_twins(&flat));
        let changed = with.iter().zip(&without).filter(|(a, b)| a != b).count();
        assert!(changed > 30, "{changed}");
        assert!(list.iter().enumerate().all(|(i, p)| relief_answer(i, p).1 >= CLEARANCE));
        assert_eq!(relief_inputs(), list);
    }

    // checks: PRE-03 PRE-29 PRE-26
    #[test]
    fn coarse_inputs_cover_every_case() {
        // Crowns standing and not, every cover group, the water's every step, all clear of a flip and the same every
        // time.
        let list = coarse_inputs();
        assert_eq!(list.len(), N);
        let answers = coarse_twins(&list);
        assert!(answers.iter().any(|a| a / 50 == 1) && answers.iter().any(|a| a / 50 == 0));
        for group in 0..5u8 {
            assert!(
                answers.iter().any(|a| a % 50 / 10 == group),
                "group {group} never picked"
            );
        }
        let steps: std::collections::BTreeSet<u8> = answers.iter().map(|a| a % 10).collect();
        assert!(steps.len() >= 5, "{steps:?}");
        assert!(list.iter().enumerate().all(|(i, p)| coarse_answer(i, p).3 >= CLEARANCE));
        assert_eq!(coarse_inputs(), list);
    }

    // checks: PRE-21 PRE-30
    #[test]
    fn edge_inputs_cover_every_case() {
        // Outlined and not, every haze level, some inside a level's dithered band, every category's gap.
        let list = edge_inputs();
        assert_eq!(list.len(), N);
        let answers = edge_twins(&list);
        assert!(answers.iter().any(|a| a % 2 == 1) && answers.iter().any(|a| a % 2 == 0));
        for level in 0..4u8 {
            assert!(answers.iter().any(|a| a / 2 == level), "no level {level}");
        }
        assert!(list.iter().any(|p| p.band > 0.0));
        for gap in &OUTLINE_GAP_M[1..7] {
            assert!(list.iter().any(|p| p.gap == *gap), "no gap {gap}");
        }
        assert_eq!(edge_inputs(), list);
        // An input on its gap's edge is refused.
        let edge = EdgeInput {
            depth: 10.0,
            far: 16.0,
            opposite: 10.0,
            gap: 6.0,
            air_depth: 0.0,
            height: 0.0,
            band: 0.0,
        };
        assert!(edge_answer(0, &edge).2 < EDGE_CLEARANCE);
    }

    // checks: PRE-20 PRE-22
    #[test]
    fn surface_inputs_cover_every_case() {
        // Every surface wins somewhere, both looks are picked, and the coverage is read at level 0, at whole levels
        // above it and between levels, from the person stop's art pixel to beyond the camp stop's.
        let fixture = cover_fixture();
        assert_eq!(fixture.ids, [0, 1, 2, 3, 4, 5, 6, 7]);
        let list = surface_inputs();
        assert_eq!(list.len(), N);
        let answers = surface_twins(&list);
        for surface in 0..8u8 {
            assert!(answers.iter().any(|a| a / 3 == surface), "surface {surface} never wins");
        }
        assert!(answers.iter().any(|a| a % 3 == 0) && answers.iter().any(|a| a % 3 == 1));
        assert!(list.iter().any(|p| p.level == 0.0));
        assert!(list.iter().any(|p| p.level >= 1.0 && p.level.fract() == 0.0));
        assert!(list.iter().any(|p| p.level > 3.0 && p.level.fract() > 0.0));
        assert!(list.iter().any(|p| p.inv_texel > 30.0) && list.iter().any(|p| p.inv_texel < 0.2));
        assert!(list.iter().all(|p| clear(p, &fixture)));
        assert_eq!(surface_inputs(), list);
        // An input on a decision's edge is refused: half way between the centres of two squares of different
        // surfaces, read square by square with the noises faded away, or the split noise at its take-over value.
        let level0 = &fixture.levels[0];
        let x = (0..COVER_SIDE - 1).find(|&x| level0[x] != level0[x + 1]).unwrap();
        let edge = SurfaceInput {
            q: [x as f32 + 1.0, 0.5],
            w: [100.0, 100.0],
            inv_texel: 1e-3,
            level: 0.0,
            split_at: 0.3,
            oct: [24.0, 1.0 / 24.0, 0.0, 0.0],
        };
        assert!(!clear(&edge, &fixture));
        let inside = SurfaceInput {
            q: [x as f32 + 0.5, 0.5],
            ..edge
        };
        assert!(clear(&inside, &fixture));
        assert!(!clear(
            &SurfaceInput {
                split_at: 0.0,
                ..inside
            },
            &fixture
        ));
    }
}
