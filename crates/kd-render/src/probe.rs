//! The probe scene (A11.13 rule 2): fixed inputs drawn one art pixel each through the shaders' per-pixel formulas into
//! a small target, read back and compared with the Rust twins (`pixel`) exactly; headless Chromium runs it every
//! alpha, and the phone in its self-check.
//!
//! Implements PRE-20 and PRE-01, see A11.13: the GPU picks exactly the steps the twins pick.

use kd_core::num::hash2;

use crate::RenderError;
use crate::gl::{self, Format, Program, State, Target, Texture, unit};
use crate::pixel::{ladder_pos, light_step, lightness, margin};
use crate::shaders::{self, Stage};

/// The probe target's size, one input an art pixel.
pub const W: u32 = 16;
pub const H: u32 = 16;
pub const N: usize = (W * H) as usize;
/// The probe's light: the luminance of the sky and of the sun facing it.
pub const Y_SKY: f32 = 0.12;
pub const Y_SUN: f32 = 0.85;
/// How far, in lightness, every input stays from where its step flips, so a GPU's rounding cannot flip it (the
/// risk α01a names).
pub const CLEARANCE: f32 = 1e-4;

/// One fixed input: the sky and sun factors, the band's half-width in steps, and the ladder's number of steps.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Input {
    pub sigma: f32,
    pub tau: f32,
    pub band: f32,
    pub steps: i32,
}

/// The path's range under the probe's light: deep shade to full sun.
pub fn range() -> [f32; 2] {
    [lightness(0.3, 0.0, Y_SKY, Y_SUN), lightness(1.0, 1.0, Y_SKY, Y_SUN)]
}

/// Input `i`'s pixel, row by row from the target's bottom, as `gl_FragCoord` counts.
fn pixel_of(i: usize) -> (i32, i32) {
    ((i as u32 % W) as i32, (i as u32 / W) as i32)
}

/// The probe's inputs: drawn from a fixed sequence of hashes, kept only when `CLEARANCE` from any flip.
pub fn inputs() -> Vec<Input> {
    let range = range();
    let mut out = Vec::with_capacity(N);
    let mut n = 0u64;
    while out.len() < N {
        let h = hash2(0x0070_726f_6265, n);
        n += 1;
        let unit = |shift: u32| ((h >> shift) & 0xffff) as f32 / 65536.0;
        let input = Input {
            sigma: unit(0),
            tau: unit(16),
            band: [0.0, 0.15, 0.35][((h >> 32) % 3) as usize],
            steps: 4 + ((h >> 40) % 4) as i32,
        };
        let (x, y) = pixel_of(out.len());
        let s = ladder_pos(lightness(input.sigma, input.tau, Y_SKY, Y_SUN), range, input.steps);
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
            let s = ladder_pos(lightness(p.sigma, p.tau, Y_SKY, Y_SUN), range, p.steps);
            light_step(s, p.band, p.steps, x, y) as u8
        })
        .collect()
}

/// The inputs as an R32F texture `N` wide and 4 rows high: σ, τ, band, steps.
pub fn texture_bytes(inputs: &[Input]) -> Vec<u8> {
    let rows: [Vec<f32>; 4] = [
        inputs.iter().map(|p| p.sigma).collect(),
        inputs.iter().map(|p| p.tau).collect(),
        inputs.iter().map(|p| p.band).collect(),
        inputs.iter().map(|p| p.steps as f32).collect(),
    ];
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
        let list = inputs();
        let inputs = Texture::new(gl, Format::R32F, N as u32, 4, Some(&texture_bytes(&list)))?;
        let target = Target::new(gl, W, H, &[Format::Rgba8], false)?;
        Ok(ProbePass {
            program,
            u_y,
            u_range,
            inputs,
            target,
            twins: twins(&list),
        })
    }

    /// Draws the probe and reads it back: the GPU's steps and the twins', in the same order.
    pub fn run(&self, gl: &glow::Context, vao: glow::VertexArray) -> (Vec<u8>, Vec<u8>) {
        self.target.bind(gl);
        gl::apply(gl, &State::flat(W, H));
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
        // The same inputs every time, on every target: a fixed sequence.
        assert_eq!(inputs(), list);
        assert_eq!(texture_bytes(&list).len(), N * 4 * 4);
    }
}
