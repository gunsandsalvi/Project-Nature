//! Pass 2, the scene, and its art target (A11.2). In α01a the scene is the light card, drawn as palette indices into
//! colour 0; the ground (α01b) replaces it.

use kd_core::m;

use crate::RenderError;
use crate::frame::Lighting;
use crate::gl::{self, Format, Program, State, Target};
use crate::light::{LUM, dot};
use crate::looks::Layout;
use crate::shaders::{self, Stage};

/// The light card's layout, in art pixels from the screen's top-left corner, worked out on the CPU (A11.13 rule 1);
/// the shaders take the fixed sizes as generated defines (rule 6).
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
    /// The block: 2 m on a side, seen from 30° above the horizon, turning once in 24 seconds.
    pub const BLOCK_M: f32 = 2.0;
    pub const PITCH_DEG: f32 = 30.0;
    pub const TURN_S: f32 = 24.0;
    /// Metres the block's outline may need on screen: 2√2 m across when turned, 3.1 m high from 30° above.
    const BLOCK_SPAN_M: f32 = 3.4;
    /// Room kept free for the strip at the foot of a portrait screen (A12.1).
    const STRIP_ROOM: i32 = 32;
    /// The widest ladder, in swatches (`PRE-20`).
    const MAX_STEPS: i32 = 7;

    /// What the card shows this frame.
    #[derive(Clone, Copy, Debug, Default, PartialEq)]
    pub struct CardFrame {
        /// The look the block is made of, by its place in the catalogue.
        pub block_look: usize,
        /// The block's turn about the vertical, in radians.
        pub turn: f32,
        /// The block alone, filling the screen: the `block` golden (A11.12).
        pub block_only: bool,
    }

    /// Where the swatches and the block sit, and the block's scale.
    #[derive(Clone, Copy, Debug, PartialEq)]
    pub struct CardLayout {
        /// The first swatch row's top-left corner, or none in `block_only`.
        pub swatches: Option<[i32; 2]>,
        pub block_centre: [i32; 2],
        pub px_per_m: f32,
    }

    /// The card's layout on a screen of `visible` art pixels with `looks` ladders: in portrait the block under the
    /// swatches, in landscape beside them.
    pub fn layout(visible: [u32; 2], looks: usize, block_only: bool) -> CardLayout {
        let [w, h] = [visible[0] as i32, visible[1] as i32];
        if block_only {
            return CardLayout {
                swatches: None,
                block_centre: [w / 2, h / 2],
                px_per_m: (w.min(h) - 2 * MARGIN).max(8) as f32 / BLOCK_SPAN_M,
            };
        }
        let rows = looks.min(MAX_LOOKS) as i32;
        let (centre, room) = if w > h {
            let left = MARGIN + MAX_STEPS * SWATCH + MARGIN;
            ([(left + w - MARGIN) / 2, h / 2], [w - MARGIN - left, h - 2 * MARGIN])
        } else {
            let top = MARGIN + rows * ROW;
            let bottom = h - STRIP_ROOM;
            ([w / 2, (top + bottom) / 2], [w - 2 * MARGIN, bottom - top])
        };
        CardLayout {
            swatches: Some([MARGIN, MARGIN]),
            block_centre: centre,
            px_per_m: room[0].min(room[1]).max(8) as f32 / BLOCK_SPAN_M,
        }
    }

    /// The block's six faces as normals in the world (east, north, up), turned by `turn` about the vertical.
    pub fn faces(turn: f32) -> [[f32; 3]; 6] {
        let (s, c) = (kd_core::m::sin(turn), kd_core::m::cos(turn));
        [[1.0, 0.0], [-1.0, 0.0], [0.0, 1.0], [0.0, -1.0]]
            .map(|[x, y]| [c * x - s * y, s * x + c * y, 0.0])
            .into_iter()
            .chain([[0.0, 0.0, 1.0], [0.0, 0.0, -1.0]])
            .collect::<Vec<_>>()
            .try_into()
            .unwrap_or([[0.0; 3]; 6])
    }
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
    /// Art pixels from the target's corner to the window's bottom-left corner: one across, and up whatever puts an
    /// art pixel's top edge on the window's top edge, so the art grid and the UI's grid, counted from the screen's
    /// top-left, are one (A12.1); the camera's snapping sets its own shift from α01b.
    pub off: [f32; 2],
    /// The art pixel at the window's top-left corner: its column, and its row counted from the target's bottom.
    pub top_left: [i32; 2],
}

impl ArtView {
    pub fn new(w: u32, h: u32, scale: u32) -> ArtView {
        let s = scale.max(1);
        let visible = [w.div_ceil(s), h.div_ceil(s)];
        // The window's top edge, at h / s art pixels, lands on a whole art pixel.
        let off = [1.0, 1.0 + (s - h % s) as f32 % s as f32 / s as f32];
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
    u: [Option<glow::UniformLocation>; 13],
    pub target: Option<Target>,
}

/// The light card's uniforms, in `ScenePass::u`'s order.
const UNIFORMS: [&str; 13] = [
    "u_top_left",
    "u_swatches",
    "u_looks",
    "u_ladders[0]",
    "u_block_at",
    "u_px_per_m",
    "u_turn",
    "u_pitch",
    "u_block",
    "u_light_dir",
    "u_y",
    "u_range",
    "u_block_only",
];

impl ScenePass {
    pub fn new(gl: &glow::Context) -> Result<ScenePass, RenderError> {
        let program = Program::new(
            gl,
            "light card",
            &shaders::source(Stage::Vertex, shaders::FULL_TARGET_VERT),
            &shaders::source(Stage::Fragment, shaders::LIGHT_CARD_FRAG),
        )?;
        Ok(ScenePass {
            u: UNIFORMS.map(|name| program.uniform(gl, name)),
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

    /// Draws the light card: each look's ladder of the palette's `layout` as a row of swatches with the block
    /// turning beside them, lit by the frame's `lighting` (A11.3, A11.12).
    pub fn draw(
        &self,
        gl: &glow::Context,
        view: &ArtView,
        layout: &Layout,
        lighting: &Lighting,
        frame: &card::CardFrame,
        vao: glow::VertexArray,
    ) {
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
        let place = card::layout(view.visible, ladders.len(), frame.block_only);
        let block = ladders.get(frame.block_look).copied().unwrap_or([0, 1]);
        let pitch = card::PITCH_DEG.to_radians();
        let light = &lighting.light;
        let [
            u_top_left,
            u_swatches,
            u_looks,
            u_ladders,
            u_block_at,
            u_px_per_m,
            u_turn,
            u_pitch,
            u_block,
            u_light_dir,
            u_y,
            u_range,
            u_block_only,
        ] = &self.u;
        gl::set_ivec2(gl, u_top_left.as_ref(), view.top_left);
        gl::set_ivec2(gl, u_swatches.as_ref(), place.swatches.unwrap_or([0, 0]));
        gl::set_i32(
            gl,
            u_looks.as_ref(),
            if place.swatches.is_some() {
                ladders.len() as i32
            } else {
                0
            },
        );
        gl::set_ivec2_array(gl, u_ladders.as_ref(), &ladders);
        gl::set_ivec2(gl, u_block_at.as_ref(), place.block_centre);
        gl::set_f32(gl, u_px_per_m.as_ref(), place.px_per_m * card::BLOCK_M / 2.0);
        gl::set_vec2(gl, u_turn.as_ref(), [m::cos(frame.turn), m::sin(frame.turn)]);
        gl::set_vec2(gl, u_pitch.as_ref(), [m::sin(pitch), m::cos(pitch)]);
        gl::set_ivec2(gl, u_block.as_ref(), block);
        gl::set_vec3(gl, u_light_dir.as_ref(), light.dir);
        gl::set_vec2(gl, u_y.as_ref(), [dot(LUM, light.sky), dot(LUM, light.sun)]);
        gl::set_vec2(gl, u_range.as_ref(), lighting.palette.range);
        gl::set_i32(gl, u_block_only.as_ref(), i32::from(frame.block_only));
        gl::draw_full_target(gl, vao);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::pixel::{ladder_pos, light_step, lightness};

    // checks: PRE-20 PRE-30
    #[test]
    fn block_faces_follow_the_light() {
        // At 16:30 the sun stands low in the west: the block's south-west face, turned 30°, takes a higher step of
        // its ladder than its south-south-east face in shade, and the top lies between, as the twins pick them.
        let cat = crate::tests::catalogue();
        let layout = Layout::new(&cat).unwrap();
        let block = cat.looks.iter().position(|l| l.id == "limestone").unwrap();
        let steps = i32::from(layout.ladders[block].steps);
        let step_at = |hour: f32, n: [f32; 3]| {
            let lighting = Lighting::new(&cat, &layout, &crate::light::tests::sky_at(hour));
            let l = &lighting.light;
            let sigma = (1.0 + n[2]) / 2.0;
            let tau = (n[0] * l.dir[0] + n[1] * l.dir[1] + n[2] * l.dir[2]).max(0.0);
            let lit = lightness(sigma, tau, dot(LUM, l.sky), dot(LUM, l.sun));
            light_step(ladder_pos(lit, lighting.palette.range, steps), 0.0, steps, 0, 0)
        };
        let [_, south_west, _, south_south_east, up, _] = card::faces(30f32.to_radians());
        let (sunny, shaded, top) = (
            step_at(16.5, south_west),
            step_at(16.5, south_south_east),
            step_at(16.5, up),
        );
        assert!(sunny > top && top > shaded, "sunny {sunny}, top {top}, shaded {shaded}");
        // At noon the top is the brightest face. At night the moon lights the top, and its step is the night's own
        // ladder's, so it is darker than noon's step only in colour.
        assert!(step_at(12.0, up) > step_at(12.0, south_west));
        let shown = |hour: f32, n: [f32; 3]| {
            let lighting = Lighting::new(&cat, &layout, &crate::light::tests::sky_at(hour));
            let c = lighting.palette.row[usize::from(layout.ladders[block].base) + step_at(hour, n) as usize];
            u32::from(c[0]) + u32::from(c[1]) + u32::from(c[2])
        };
        assert!(
            shown(23.0, up) * 2 < shown(12.0, up),
            "night {}, noon {}",
            shown(23.0, up),
            shown(12.0, up)
        );
    }

    // checks: PRE-20 PRE-34
    #[test]
    fn card_fits_both_ways() {
        let span = |c: &card::CardLayout| 3.4 * c.px_per_m / 2.0;
        // Portrait: the block under the five ladders, clear of them and of the strip.
        let p = card::layout([270, 601], 5, false);
        assert_eq!(p.swatches, Some([card::MARGIN, card::MARGIN]));
        assert!(p.block_centre[1] as f32 - span(&p) >= (card::MARGIN + 5 * card::ROW) as f32);
        assert!(p.block_centre[1] as f32 + span(&p) <= (601 - 24) as f32);
        // Landscape: the block beside them.
        let l = card::layout([601, 270], 5, false);
        assert!(l.block_centre[0] as f32 - span(&l) >= (card::MARGIN + 7 * card::SWATCH) as f32);
        assert!(l.block_centre[1] as f32 + span(&l) <= 270.0);
        // The block alone, in the middle.
        let b = card::layout([270, 601], 5, true);
        assert_eq!((b.swatches, b.block_centre), (None, [135, 300]));
        // Each face's normal is a unit vector, the top facing up.
        for n in card::faces(1.0) {
            assert!(((n[0] * n[0] + n[1] * n[1] + n[2] * n[2]) - 1.0).abs() < 1e-5);
        }
        assert_eq!(card::faces(0.3)[4], [0.0, 0.0, 1.0]);
    }

    // checks: PRE-22 PLT-02
    #[test]
    fn art_size() {
        // The phone's 1080 × 2404 screen at 4 screen pixels an art pixel, in portrait and turned.
        let p = ArtView::new(1080, 2404, 4);
        assert_eq!((p.visible, p.art), ([270, 601], [273, 604]));
        let l = ArtView::new(2404, 1080, 4);
        assert_eq!((l.visible, l.art), ([601, 270], [604, 273]));
        // A size that is not a whole number of art pixels shows the last one in part, at the bottom and the right:
        // the art grid meets the screen's top-left corner, as the UI's does.
        let odd = ArtView::new(1081, 2401, 4);
        assert_eq!(odd.art, [274, 604]);
        assert_eq!(odd.off, [1.0, 1.75]);
        assert_eq!(((odd.off[1] + 2401.0 / 4.0) % 1.0, p.off), (0.0, [1.0, 1.0]));
        // The window's top-left art pixel lies inside the target, with the border round it.
        for v in [p, l, ArtView::new(1081, 2401, 4)] {
            assert_eq!(v.top_left[0], 1);
            assert_eq!(v.top_left[1], v.visible[1] as i32);
            assert!(v.top_left[1] + 2 < v.art[1] as i32);
        }
    }
}
