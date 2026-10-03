//! Pass 5 (A11.2): the art target enlarged to the window, nearest, by a whole-number scale, with the sub-pixel
//! shift.

use crate::RenderError;
use crate::gl::{self, Program, State, Texture, unit};
use crate::passes::scene::ArtView;
use crate::shaders;

pub struct UpscalePass {
    program: Program,
    u_off: Option<glow::UniformLocation>,
    u_scale: Option<glow::UniformLocation>,
}

impl UpscalePass {
    pub fn new(gl: &glow::Context) -> Result<UpscalePass, RenderError> {
        let program = Program::new(
            gl,
            "upscale",
            &shaders::source(shaders::FULL_TARGET_VERT),
            &shaders::source(shaders::UPSCALE_FRAG),
        )?;
        program.set_sampler(gl, "u_art", unit::ART);
        let u_off = program.uniform(gl, "u_off");
        let u_scale = program.uniform(gl, "u_scale");
        Ok(UpscalePass {
            program,
            u_off,
            u_scale,
        })
    }

    pub fn draw(&self, gl: &glow::Context, view: &ArtView, art: &Texture, vao: glow::VertexArray) {
        gl::bind_window(gl);
        gl::apply(gl, &State::flat(view.window[0], view.window[1]));
        self.program.bind(gl);
        art.bind(gl, unit::ART);
        gl::set_vec2(gl, self.u_off.as_ref(), view.off);
        gl::set_f32(gl, self.u_scale.as_ref(), view.scale as f32);
        gl::draw_full_target(gl, vao);
    }
}
