//! Pass 6, the UI (A11.2, A12.1): the frame's draw list on the window, after the upscale, one UI pixel 4 × 4 screen
//! pixels on the screen's own grid, in the palette's fixed colours; each item shows through the 4 × 4 Bayer pattern
//! by its fade.

use kd_view::{FontAtlas, UiDrawList, UiItem};

use crate::RenderError;
use crate::gl::{self, Attr, Component, Format, Layout, Mesh, Program, State, Texture, unit};
use crate::passes::scene::ArtView;
use crate::shaders::{self, Stage};

/// A UI vertex: its corner in UI pixels, the font texel there (negative for a filled rectangle), and the palette
/// index with the fade in 255ths.
pub const LAYOUT: Layout = Layout {
    attrs: &[
        Attr {
            location: 0,
            component: Component::F32,
            count: 2,
        },
        Attr {
            location: 1,
            component: Component::F32,
            count: 2,
        },
        Attr {
            location: 2,
            component: Component::U16,
            count: 2,
        },
    ],
};

/// One quad as two triangles: corners `at` and `at + size`, font texels from `tex` (or none), colour and fade.
fn quad(out: &mut Vec<u8>, at: [i32; 2], size: [i32; 2], tex: Option<[f32; 2]>, colour: u8, fade: f32) {
    let info = [u16::from(colour), (fade.clamp(0.0, 1.0) * 255.0 + 0.5) as u16];
    for (dx, dy) in [(0, 0), (1, 0), (0, 1), (0, 1), (1, 0), (1, 1)] {
        let p = [(at[0] + dx * size[0]) as f32, (at[1] + dy * size[1]) as f32];
        let t = tex.map_or([-1.0, -1.0], |t| {
            [t[0] + (dx * size[0]) as f32, t[1] + (dy * size[1]) as f32]
        });
        for v in [p[0], p[1], t[0], t[1]] {
            out.extend_from_slice(&v.to_ne_bytes());
        }
        out.extend_from_slice(&info[0].to_ne_bytes());
        out.extend_from_slice(&info[1].to_ne_bytes());
    }
}

/// The list's vertices in `LAYOUT`: a quad for each rectangle and for each glyph.
pub fn vertices(list: &UiDrawList, font: &FontAtlas) -> Vec<u8> {
    let mut out = Vec::new();
    for item in &list.items {
        match item {
            UiItem::Rect { at, size, colour, fade } => quad(&mut out, *at, *size, None, *colour, *fade),
            UiItem::Text {
                at,
                colour,
                fade,
                glyphs,
            } => {
                for (dx, g) in glyphs {
                    let size = [i32::from(g.w), i32::from(font.cell_h)];
                    let tex = [f32::from(g.x), f32::from(g.y)];
                    quad(&mut out, [at[0] + dx, at[1]], size, Some(tex), *colour, *fade);
                }
            }
        }
    }
    out
}

pub struct UiPass {
    program: Program,
    u_screen: Option<glow::UniformLocation>,
    u_scale: Option<glow::UniformLocation>,
    mesh: Mesh,
    font: Texture,
    atlas: FontAtlas,
}

impl UiPass {
    pub fn new(gl: &glow::Context, atlas: &FontAtlas) -> Result<UiPass, RenderError> {
        let program = Program::new(
            gl,
            "ui",
            &shaders::source(Stage::Vertex, shaders::UI_VERT),
            &shaders::source(Stage::Fragment, shaders::UI_FRAG),
        )?;
        program.set_sampler(gl, "u_font", unit::FONT);
        program.set_sampler(gl, "u_palette", unit::PALETTE);
        let font = Texture::new(gl, Format::R8, atlas.w, atlas.h, Some(&atlas.pixels))?;
        Ok(UiPass {
            u_screen: program.uniform(gl, "u_screen"),
            u_scale: program.uniform(gl, "u_scale"),
            program,
            mesh: Mesh::new(gl, &LAYOUT, &[], None)?,
            font,
            atlas: atlas.clone(),
        })
    }

    /// Draws the list over the window.
    pub fn draw(&mut self, gl: &glow::Context, view: &ArtView, list: &UiDrawList, palette: &Texture) {
        if list.items.is_empty() {
            return;
        }
        if let Err(e) = self.mesh.refill(gl, &LAYOUT, &vertices(list, &self.atlas)) {
            log::error!(target: "kd::render", "UI: {e}");
            return;
        }
        gl::bind_window(gl);
        gl::apply(gl, &State::flat(view.window[0], view.window[1]));
        self.program.bind(gl);
        self.font.bind(gl, unit::FONT);
        palette.bind(gl, unit::PALETTE);
        gl::set_vec2(
            gl,
            self.u_screen.as_ref(),
            [view.window[0] as f32, view.window[1] as f32],
        );
        gl::set_f32(gl, self.u_scale.as_ref(), view.scale as f32);
        self.mesh.draw(gl);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_view::Glyph;

    // checks: PRE-32 PRE-01
    #[test]
    fn quads_on_whole_pixels() {
        let font = FontAtlas {
            cell_h: 11,
            ..FontAtlas::default()
        };
        let g = Glyph { x: 16, y: 22, w: 5 };
        let list = UiDrawList {
            items: vec![
                UiItem::Rect {
                    at: [0, 577],
                    size: [270, 24],
                    colour: 2,
                    fade: 0.5,
                },
                UiItem::Text {
                    at: [8, 578],
                    colour: 4,
                    fade: 1.0,
                    glyphs: vec![(0, g), (6, g)],
                },
            ],
        };
        let bytes = vertices(&list, &font);
        let (_, stride) = LAYOUT.offsets();
        assert_eq!(stride, 20);
        assert_eq!(bytes.len(), 3 * 6 * stride);
        let vertex = |i: usize| {
            let v = &bytes[i * stride..(i + 1) * stride];
            let f = |k: usize| f32::from_ne_bytes(v[k * 4..k * 4 + 4].try_into().unwrap());
            let u = |k: usize| u16::from_ne_bytes(v[16 + k * 2..18 + k * 2].try_into().unwrap());
            ([f(0), f(1)], [f(2), f(3)], [u(0), u(1)])
        };
        // The rectangle: whole UI pixels, no font, its colour and half shown.
        assert_eq!(vertex(0), ([0.0, 577.0], [-1.0, -1.0], [2, 128]));
        assert_eq!(vertex(5).0, [270.0, 601.0]);
        // The second glyph starts 6 pixels along and reads its cell, one texel a UI pixel.
        assert_eq!(vertex(12), ([14.0, 578.0], [16.0, 22.0], [4, 255]));
        assert_eq!(vertex(17).0, [19.0, 589.0]);
        assert_eq!(vertex(17).1, [21.0, 33.0]);
    }
}
