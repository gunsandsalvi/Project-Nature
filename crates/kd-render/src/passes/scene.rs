//! Pass 2, the scene, and its art target (A11.2). In α01a the scene is the light card, drawn as palette indices into
//! colour 0; the ground (α01b) replaces it.

use crate::RenderError;
use crate::gl::{self, Format, Program, State, Target};
use crate::looks::Layout;
use crate::shaders::{self, Stage};

/// The light card's layout, in art pixels from the screen's top-left corner; the shaders take these as generated
/// defines (A11.13 rule 6).
pub mod card {
    /// 128 screen pixels in from each edge, clear of a phone's rounded corners and camera cut-out.
    pub const MARGIN: i32 = 32;
    /// A swatch's side.
    pub const SWATCH: i32 = 16;
    /// The line above each row of swatches that holds the look's name.
    pub const NAME_H: i32 = 9;
    /// From one look's row to the next: its name, its swatches and a gap.
    pub const ROW: i32 = NAME_H + SWATCH + 7;
    /// Looks the card shows; more would not fit a portrait screen with the block (A11.12).
    pub const MAX_LOOKS: usize = 8;
}

/// Where the art target sits behind the window, worked out on the CPU (A11.13 rule 1).
///
/// Implements PRE-22, see A11.2: an art pixel is `scale` × `scale` screen pixels in portrait and landscape alike,
/// and the art target is `ceil(W/s) + 3` by `ceil(H/s) + 3`, its border carrying the upscale's shift.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ArtView {
    pub window: [u32; 2],
    pub scale: u32,
    pub art: [u32; 2],
    /// Art pixels the window shows, counting a partly shown one.
    pub visible: [u32; 2],
    /// Art pixels from the target's corner to the window's bottom-left corner: one in each direction until the
    /// camera's snapping sets its own shift (α01b).
    pub off: [f32; 2],
    /// The art pixel at the window's top-left corner: its column, and its row counted from the target's bottom.
    pub top_left: [i32; 2],
}

impl ArtView {
    pub fn new(w: u32, h: u32, scale: u32) -> ArtView {
        let s = scale.max(1);
        let visible = [w.div_ceil(s), h.div_ceil(s)];
        let off = [1.0, 1.0];
        // The window's top row of pixels has its centres at h − 0.5; the upscale shows floor(off + y / s) there.
        let top_row = (off[1] + (h as f32 - 0.5) / s as f32).floor() as i32;
        ArtView {
            window: [w, h],
            scale: s,
            art: [visible[0] + 3, visible[1] + 3],
            visible,
            off,
            top_left: [off[0] as i32, top_row],
        }
    }
}

pub struct ScenePass {
    program: Program,
    u_top_left: Option<glow::UniformLocation>,
    u_swatches: Option<glow::UniformLocation>,
    u_looks: Option<glow::UniformLocation>,
    u_ladders: Option<glow::UniformLocation>,
    pub target: Option<Target>,
}

impl ScenePass {
    pub fn new(gl: &glow::Context) -> Result<ScenePass, RenderError> {
        let program = Program::new(
            gl,
            "light card",
            &shaders::source(Stage::Vertex, shaders::FULL_TARGET_VERT),
            &shaders::source(Stage::Fragment, shaders::LIGHT_CARD_FRAG),
        )?;
        Ok(ScenePass {
            u_top_left: program.uniform(gl, "u_top_left"),
            u_swatches: program.uniform(gl, "u_swatches"),
            u_looks: program.uniform(gl, "u_looks"),
            u_ladders: program.uniform(gl, "u_ladders[0]"),
            program,
            target: None,
        })
    }

    /// Makes the art target again when its size changes: colour 0 and depth with stencil (A11.2; colour 1, for
    /// picking, joins in α03c).
    pub fn resize(&mut self, gl: &glow::Context, view: &ArtView) -> Result<(), RenderError> {
        if self.target.as_ref().is_some_and(|t| [t.w, t.h] == view.art) {
            return Ok(());
        }
        if let Some(t) = self.target.take() {
            t.delete(gl);
        }
        self.target = Some(Target::new(gl, view.art[0], view.art[1], &[Format::Rgba8], true)?);
        Ok(())
    }

    /// Draws the light card: each look's ladder of the palette's `layout` as a row of swatches.
    pub fn draw(&self, gl: &glow::Context, view: &ArtView, layout: &Layout, vao: glow::VertexArray) {
        let Some(t) = &self.target else { return };
        t.bind(gl);
        gl::apply(gl, &State::flat(t.w, t.h));
        self.program.bind(gl);
        let ladders: Vec<[i32; 2]> = layout
            .ladders
            .iter()
            .take(card::MAX_LOOKS)
            .map(|l| [i32::from(l.base), i32::from(l.steps)])
            .collect();
        gl::set_ivec2(gl, self.u_top_left.as_ref(), view.top_left);
        gl::set_ivec2(gl, self.u_swatches.as_ref(), [card::MARGIN, card::MARGIN]);
        gl::set_i32(gl, self.u_looks.as_ref(), ladders.len() as i32);
        gl::set_ivec2_array(gl, self.u_ladders.as_ref(), &ladders);
        gl::draw_full_target(gl, vao);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-22 PLT-02
    #[test]
    fn art_size() {
        // The phone's 1080 × 2404 screen at 4 screen pixels an art pixel, in portrait and turned.
        let p = ArtView::new(1080, 2404, 4);
        assert_eq!((p.visible, p.art), ([270, 601], [273, 604]));
        let l = ArtView::new(2404, 1080, 4);
        assert_eq!((l.visible, l.art), ([601, 270], [604, 273]));
        // A size that is not a whole number of art pixels shows the last one in part.
        assert_eq!(ArtView::new(1081, 2401, 4).art, [274, 604]);
        // The window's top-left art pixel lies inside the target, with the border round it.
        for v in [p, l, ArtView::new(1081, 2401, 4)] {
            assert_eq!(v.top_left[0], 1);
            assert_eq!(v.top_left[1], v.visible[1] as i32);
            assert!(v.top_left[1] + 2 < v.art[1] as i32);
        }
    }
}
