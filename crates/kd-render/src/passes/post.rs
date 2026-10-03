//! Pass 3, post (A11.2): each art pixel's palette colour from the index colour 0 holds and the row in use, into an
//! art-size target the upscale enlarges, after outlines, lit edges and haze, each a table lookup (A11.3); glow
//! joins with fire (α14a).
//!
//! Implements PRE-21 and PRE-30, see A11.2 and A11.4: outlines only where one thing stands in front of another, lit
//! edges where the light catches a silhouette, and haze by the air between the eye and the ground.

use crate::RenderError;
use crate::gl::{self, Format, Program, State, Target, Texture, unit};
use crate::passes::scene::ArtView;
use crate::shaders::{self, Stage};

pub struct PostPass {
    program: Program,
    u_depth_m: Option<glow::UniformLocation>,
    u_sun_screen: Option<glow::UniformLocation>,
    pub target: Option<Target>,
}

/// What post needs of a frame besides its textures: the metres colour 0's depth spans, and the light's way across
/// the screen, right and up.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct PostFrame {
    pub depth_m: f32,
    pub sun_screen: [f32; 2],
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
        program.set_sampler(gl, "u_tables", unit::TABLES);
        Ok(PostPass {
            u_depth_m: program.uniform(gl, "u_depth_m"),
            u_sun_screen: program.uniform(gl, "u_sun_screen"),
            program,
            target: None,
        })
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

    pub fn draw(
        &self,
        gl: &glow::Context,
        scene: &Texture,
        palette: &Texture,
        tables: &Texture,
        frame: PostFrame,
        vao: glow::VertexArray,
    ) {
        let Some(t) = &self.target else { return };
        t.bind(gl);
        gl::apply(gl, &State::flat(t.w, t.h));
        self.program.bind(gl);
        scene.bind(gl, unit::SCENE);
        palette.bind(gl, unit::PALETTE);
        tables.bind(gl, unit::TABLES);
        gl::set_f32(gl, self.u_depth_m.as_ref(), frame.depth_m);
        gl::set_vec2(gl, self.u_sun_screen.as_ref(), frame.sun_screen);
        gl::draw_full_target(gl, vao);
    }
}
