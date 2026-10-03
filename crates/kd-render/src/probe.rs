//! The probe scene (A11.13 rule 2): fixed inputs drawn one art pixel each through the shaders' per-pixel formulas into
//! a small target, read back and compared with the Rust twins (`pixel`) exactly; headless Chromium runs it every
//! alpha, and the phone in its self-check. Its lower half holds the light's steps (α01a), from α01c lit by what a
//! point's fields and normal give (the sky and sun factors); its upper half the surface the four nearest squares
//! vote for at a place the edges' noise moved, and the look the split noise picks (α01b).
//!
//! Implements PRE-20 and PRE-01, see A11.13: the GPU picks exactly the steps, surfaces and looks the twins pick.

use kd_core::num::hash2;

use crate::RenderError;
use crate::gl::{self, Format, Program, State, Target, Texture, unit};
use crate::pixel::{
    SEED_SPLIT, SUN_TAN, edge_wobble, faded_noise, ladder_pos, light_step, lightness, margin, sky_factor, split_look,
    sun_factor, vote_base, vote4,
};
use crate::shaders::{self, Stage};

/// Each half of the probe target, one input an art pixel; the target is `W` × `2H`.
pub const W: u32 = 16;
pub const H: u32 = 16;
pub const N: usize = (W * H) as usize;
/// Rows of the input texture: the light's 7, then the surfaces' `SURFACE_ROWS`.
pub const LIGHT_ROWS: usize = 7;
pub const SURFACE_ROWS: usize = 14;
/// How far from deciding otherwise every surface input stays, in shares of a vote or in noise.
pub const SURFACE_CLEARANCE: f32 = 1e-3;
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

/// One fixed surface input: a place among four squares' centres (metres east and south of the first square's
/// corner, the squares 1 m), the world-fixed place the noises read, art pixels a metre, the four squares' surfaces,
/// and a split: its take-over value and its octaves as (λ₀, 1/λ₀, λ₁, 1/λ₁).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct SurfaceInput {
    pub q: [f32; 2],
    pub w: [f32; 2],
    pub inv_texel: f32,
    pub ids: [i32; 4],
    pub split_at: f32,
    pub oct: [f32; 4],
}

/// What a surface input gives, as the GPU and the twins work it out: the surface, its look, and how far each was
/// from going the other way.
fn surface_answer(p: &SurfaceInput) -> (i32, i32, f32) {
    let wobble = edge_wobble(p.w, p.inv_texel);
    let (_, f) = vote_base([p.q[0] + wobble[0], p.q[1] + wobble[1]], 0);
    let surface = vote4(f, p.ids);
    let v = faded_noise(p.w, p.oct, p.inv_texel, SEED_SPLIT);
    let look = split_look(v, [p.split_at, 2.0], 2);
    // The vote's margin: the winner's share less the best other surface's.
    let w = [
        (1.0 - f[0]) * (1.0 - f[1]),
        f[0] * (1.0 - f[1]),
        (1.0 - f[0]) * f[1],
        f[0] * f[1],
    ];
    let share = |id: i32| (0..4).filter(|&m| p.ids[m] == id).map(|m| w[m]).sum::<f32>();
    let other = p
        .ids
        .iter()
        .filter(|&&id| id != surface)
        .map(|&id| share(id))
        .fold(0.0, f32::max);
    (surface, look, (share(surface) - other).min((v - p.split_at).abs()))
}

/// Whether a surface input lies `SURFACE_CLEARANCE` or more from deciding otherwise, so a GPU's rounding cannot
/// change its answer.
pub fn clear(p: &SurfaceInput) -> bool {
    surface_answer(p).2 >= SURFACE_CLEARANCE
}

/// The surface inputs: drawn from a fixed sequence of hashes, kept only when clear of deciding otherwise.
pub fn surface_inputs() -> Vec<SurfaceInput> {
    let mut out = Vec::with_capacity(N);
    let mut n = 0u64;
    while out.len() < N {
        let h = hash2(0x7375_7266, n);
        let g = hash2(0x7375_7267, n);
        n += 1;
        let unit = |v: u64, shift: u32| ((v >> shift) & 0xffff) as f32 / 65536.0;
        let texel = [0.03, 0.13, 0.4, 1.1, 2.2][(g % 5) as usize];
        let lambda = 2.0 + 38.0 * unit(g, 8);
        let p = SurfaceInput {
            q: [0.2 + 1.6 * unit(h, 0), 0.2 + 1.6 * unit(h, 16)],
            w: [8000.0 * unit(h, 32), 8000.0 * unit(h, 48)],
            inv_texel: 1.0 / texel,
            ids: [0, 1, 2, 3].map(|k| ((g >> (24 + 2 * k)) & 3) as i32),
            split_at: unit(g, 40) - 0.5,
            oct: if g >> 63 == 1 {
                [lambda, 1.0 / lambda, lambda / 4.0, 4.0 / lambda]
            } else {
                [lambda, 1.0 / lambda, 0.0, 0.0]
            },
        };
        if clear(&p) {
            out.push(p);
        }
    }
    out
}

/// The twins' answer for each surface input, as the probe writes it: surface × 3 + look.
pub fn surface_twins(inputs: &[SurfaceInput]) -> Vec<u8> {
    inputs
        .iter()
        .map(|p| {
            let (surface, look, _) = surface_answer(p);
            (surface * 3 + look) as u8
        })
        .collect()
}

/// All the inputs as an R32F texture `N` wide: the light's rows (the open sky, the normal's upward part, the
/// horizon, the light's slope, n·l, the band and the steps), then the surfaces' q, w, 1/texel, their four surfaces,
/// the take-over value and the octaves.
pub fn texture_bytes(inputs: &[Input], surfaces: &[SurfaceInput]) -> Vec<u8> {
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
    ]);
    for k in 0..4 {
        rows.push(field(&|p| p.ids[k] as f32));
    }
    rows.push(field(&|p| p.split_at));
    for k in 0..4 {
        rows.push(field(&|p| p.oct[k]));
    }
    rows.iter().flatten().flat_map(|v| v.to_ne_bytes()).collect()
}

pub struct ProbePass {
    program: Program,
    u_y: Option<glow::UniformLocation>,
    u_range: Option<glow::UniformLocation>,
    inputs: Texture,
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
        let u_y = program.uniform(gl, "u_y");
        let u_range = program.uniform(gl, "u_range");
        let (list, surfaces) = (inputs(), surface_inputs());
        let rows = (LIGHT_ROWS + SURFACE_ROWS) as u32;
        let inputs = Texture::new(gl, Format::R32F, N as u32, rows, Some(&texture_bytes(&list, &surfaces)))?;
        let target = Target::new(gl, W, 2 * H, &[Format::Rgba8], false)?;
        let mut answers = twins(&list);
        answers.extend(surface_twins(&surfaces));
        Ok(ProbePass {
            program,
            u_y,
            u_range,
            inputs,
            target,
            twins: answers,
        })
    }

    /// Draws the probe and reads it back: the GPU's steps and the twins', in the same order.
    pub fn run(&self, gl: &glow::Context, vao: glow::VertexArray) -> (Vec<u8>, Vec<u8>) {
        self.target.bind(gl);
        gl::apply(gl, &State::flat(W, 2 * H));
        self.program.bind(gl);
        self.inputs.bind(gl, unit::PROBE);
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
        let surfaces = surface_inputs();
        assert_eq!(
            texture_bytes(&list, &surfaces).len(),
            N * (LIGHT_ROWS + SURFACE_ROWS) * 4
        );
    }

    // checks: PRE-20 PRE-22
    #[test]
    fn surface_inputs_cover_every_case() {
        // Every surface wins somewhere, both looks are picked, the vote is sometimes split between squares of the
        // same surface, and the art pixel's size ranges from the person stop to beyond the camp stop.
        let list = surface_inputs();
        assert_eq!(list.len(), N);
        let answers = surface_twins(&list);
        for surface in 0..4u8 {
            assert!(answers.iter().any(|a| a / 3 == surface), "surface {surface} never wins");
        }
        assert!(answers.iter().any(|a| a % 3 == 0) && answers.iter().any(|a| a % 3 == 1));
        assert!(
            list.iter()
                .any(|p| p.ids.iter().filter(|&&i| i == p.ids[0]).count() == 2)
        );
        assert!(list.iter().any(|p| p.inv_texel > 30.0) && list.iter().any(|p| p.inv_texel < 0.5));
        assert!(list.iter().all(clear));
        assert_eq!(surface_inputs(), list);
        // An input on a decision's edge is refused: two surfaces tied at the middle of four squares, or the split
        // noise at its take-over value (art pixels so large the noises have faded to 0).
        let edge = SurfaceInput {
            q: [1.0, 1.0],
            w: [100.0, 100.0],
            inv_texel: 1e-3,
            ids: [0, 1, 0, 1],
            split_at: 0.3,
            oct: [24.0, 1.0 / 24.0, 0.0, 0.0],
        };
        assert!(!clear(&edge));
        assert!(!clear(&SurfaceInput {
            ids: [2; 4],
            split_at: 0.0,
            ..edge
        }));
        assert!(clear(&SurfaceInput { ids: [2; 4], ..edge }));
    }
}
