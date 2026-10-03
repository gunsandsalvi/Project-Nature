//! Pass 3, post (A11.2): each art pixel's palette colour from the index colour 0 holds and the row in use, into an
//! art-size target the upscale enlarges; outlines, lit edges, haze and glow join in α01c.

use crate::RenderError;
use crate::gl::{self, Format, Program, State, Target, Texture, unit};
use crate::passes::scene::ArtView;
use crate::shaders::{self, Stage};

pub struct PostPass {
    program: Program,
    pub target: Option<Target>,
}

impl PostPass {
    pub fn new(gl: &glow::Context) -> Result<PostPass, RenderError> {
        let program = Program::new(
            gl,
            "post",
            &shaders::source(Stage::Vertex, shaders::FULL_TARGET_VERT),
            &shaders::source(Stage::Fragment, shaders::POST_FRAG),
        )?;
        program.set_sampler(gl, "u_scene", unit::SCENE);
        program.set_sampler(gl, "u_palette", unit::PALETTE);
        Ok(PostPass { program, target: None })
    }

    /// Makes the target again when the art target's size changes.
    pub fn resize(&mut self, gl: &glow::Context, view: &ArtView) -> Result<(), RenderError> {
        if self.target.as_ref().is_some_and(|t| [t.w, t.h] == view.art) {
            return Ok(());
        }
        if let Some(t) = self.target.take() {
            t.delete(gl);
        }
        self.target = Some(Target::new(gl, view.art[0], view.art[1], &[Format::Rgba8], false)?);
        Ok(())
    }

    pub fn draw(&self, gl: &glow::Context, scene: &Texture, palette: &Texture, vao: glow::VertexArray) {
        let Some(t) = &self.target else { return };
        t.bind(gl);
        gl::apply(gl, &State::flat(t.w, t.h));
        self.program.bind(gl);
        scene.bind(gl, unit::SCENE);
        palette.bind(gl, unit::PALETTE);
        gl::draw_full_target(gl, vao);
    }
}
