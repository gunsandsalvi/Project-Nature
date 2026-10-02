//! Pass 6, the pixel UI (A11.2, A12.1): the draw list's rectangles and glyphs, at `scale` screen pixels a UI pixel on
//! the upscale's grid, so a UI pixel covers exactly one art pixel; colours from the palette's current row.
//! Implements `PRE-01` and `PRE-32` in part, see A12.1.

use crate::RenderError;
use crate::pass::Prog;
use crate::target::texture;
use glow::HasContext;
use kd_view::{FontAtlas, UiDrawList};

/// Floats per vertex: screen position, UI position, atlas position, palette index.
const STRIDE: usize = 7;

/// Where UI pixels fall on the window: the left edge of UI column 0 and the top edge of UI row 0, in screen pixels
/// from the bottom left, on the art grid that the upscale shifts by `off`.
pub fn grid_origin(off: [f32; 2], h: u32, s: u32) -> (f32, f32) {
    let s = s.max(1) as f32;
    let x0 = (off[0].floor() - off[0]) * s;
    // The art row whose top edge is at or above the window's top.
    let b_top = (h as f32 / s + off[1]).ceil() - 1.0;
    (x0, (b_top + 1.0 - off[1]) * s)
}

/// The vertices of one frame's draw list.
pub fn vertices(list: &UiDrawList, font: &FontAtlas, off: [f32; 2], h: u32, s: u32) -> Vec<f32> {
    let (x0, y0) = grid_origin(off, h, s);
    let sf = s.max(1) as f32;
    let mut v = Vec::new();
    let mut quad = |x: i32, y: i32, w: i32, hgt: i32, uv: Option<(u32, u32)>, col: u8| {
        let (l, t) = (x0 + x as f32 * sf, y0 - y as f32 * sf);
        let (r, b) = (l + w as f32 * sf, t - hgt as f32 * sf);
        let (u0, v0) = uv.map_or((-1.0, -1.0), |(u, v)| (u as f32, v as f32));
        let (u1, v1) = if uv.is_some() {
            (u0 + w as f32, v0 + hgt as f32)
        } else {
            (-1.0, -1.0)
        };
        let (ux0, uy0, ux1, uy1) = (x as f32, y as f32, (x + w) as f32, (y + hgt) as f32);
        let c = f32::from(col);
        let corners = [
            [l, b, ux0, uy1, u0, v1],
            [r, b, ux1, uy1, u1, v1],
            [r, t, ux1, uy0, u1, v0],
            [l, b, ux0, uy1, u0, v1],
            [r, t, ux1, uy0, u1, v0],
            [l, t, ux0, uy0, u0, v0],
        ];
        for k in corners {
            v.extend_from_slice(&k);
            v.push(c);
        }
    };
    for r in &list.rects {
        quad(r.x, r.y, r.w, r.h, None, r.colour.0);
    }
    for run in &list.runs {
        let mut pen = run.x;
        for ch in run.text.chars() {
            let Some((cell, adv)) = font.find(ch) else { continue };
            let (cx, cy) = (
                (cell as u32 % font.cols) * font.cell_w,
                (cell as u32 / font.cols) * font.cell_h,
            );
            let w = i32::from(adv) - 1;
            if ch != ' ' && w > 0 {
                quad(pen, run.y, w, font.cell_h as i32, Some((cx, cy)), run.colour.0);
            }
            pen += i32::from(adv);
        }
    }
    v
}

/// The window and the upscale's grid on it: window size, the shift in art pixels, screen pixels an art pixel.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Grid {
    pub win: (u32, u32),
    pub off: [f32; 2],
    pub s: u32,
}

/// The UI pass's program, glyph atlas and vertex buffer.
pub struct UiPass {
    prog: Prog,
    atlas: glow::Texture,
    font: FontAtlas,
    vao: glow::VertexArray,
    vbo: glow::Buffer,
}

impl UiPass {
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn new(gl: &glow::Context, font: &FontAtlas) -> Result<UiPass, RenderError> {
        unsafe {
            let prog = Prog::new(
                gl,
                include_str!("../shaders/ui.vert"),
                include_str!("../shaders/ui.frag"),
                "",
                false,
                &[(0, "aPos"), (1, "aUi"), (2, "aUv"), (3, "aCol")],
            )?;
            let atlas = texture(
                gl,
                font.width.max(1),
                font.height.max(1),
                glow::R8,
                glow::RED,
                Some(&font.pixels),
            )?;
            let vao = gl.create_vertex_array().map_err(RenderError::Resource)?;
            gl.bind_vertex_array(Some(vao));
            let vbo = gl.create_buffer().map_err(RenderError::Resource)?;
            gl.bind_buffer(glow::ARRAY_BUFFER, Some(vbo));
            let stride = (STRIDE * 4) as i32;
            for (loc, n, at) in [(0, 2, 0), (1, 2, 2), (2, 2, 4), (3, 1, 6)] {
                gl.vertex_attrib_pointer_f32(loc, n, glow::FLOAT, false, stride, at * 4);
                gl.enable_vertex_attrib_array(loc);
            }
            gl.bind_vertex_array(None);
            Ok(UiPass {
                prog,
                atlas,
                font: font.clone(),
                vao,
                vbo,
            })
        }
    }

    /// Draws the list over the window; returns the draw calls made.
    ///
    /// # Safety
    /// Needs a current GL context on this thread, with the window bound.
    #[allow(unsafe_code)]
    pub unsafe fn draw(&self, gl: &glow::Context, list: &UiDrawList, pal: glow::Texture, row: f32, grid: Grid) -> u32 {
        let Grid { win, off, s } = grid;
        if list.show <= 0.0 || (list.rects.is_empty() && list.runs.is_empty()) {
            return 0;
        }
        let v = vertices(list, &self.font, off, win.1, s);
        let bytes: Vec<u8> = v.iter().flat_map(|f| f.to_le_bytes()).collect();
        unsafe {
            let g = &self.prog;
            g.bind(gl);
            g.tex(gl, "uAtlas", 0, Some(self.atlas));
            g.tex(gl, "uPal", 1, Some(pal));
            g.f(gl, "uWin", &[win.0 as f32, win.1 as f32]);
            g.f(gl, "uPalRow", &[row]);
            g.f(gl, "uShow", &[list.show]);
            gl.bind_vertex_array(Some(self.vao));
            gl.bind_buffer(glow::ARRAY_BUFFER, Some(self.vbo));
            gl.buffer_data_u8_slice(glow::ARRAY_BUFFER, &bytes, glow::STREAM_DRAW);
            gl.draw_arrays(glow::TRIANGLES, 0, (v.len() / STRIDE) as i32);
            gl.bind_vertex_array(None);
        }
        1
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_core::kinds::ColourId;
    use kd_view::UiRect;

    // checks: PRE-22 PRE-01
    #[test]
    fn ui_pixels_land_on_the_art_grid() {
        // 412 x 860 at 4: art 105 x 217, so the upscale shifts by half an art pixel across and none down.
        let off = crate::pass::offset((105, 217), 412, 860, 4);
        assert_eq!(off, [0.5, 0.5]);
        let (x0, y0) = grid_origin(off, 860, 4);
        assert_eq!((x0, y0), (-2.0, 862.0));
        // Each UI pixel's edges fall where the upscale's art pixels change: off + x / s is whole there.
        for k in 0..5 {
            let x = x0 + 4.0 * k as f32;
            assert_eq!((off[0] + x / 4.0).fract(), 0.0);
            let y = y0 - 4.0 * k as f32;
            assert_eq!((off[1] + y / 4.0).fract(), 0.0);
        }
        let list = UiDrawList {
            rects: vec![UiRect {
                x: 0,
                y: 0,
                w: 2,
                h: 1,
                colour: ColourId(2),
            }],
            runs: vec![],
            show: 1.0,
        };
        let v = vertices(&list, &FontAtlas::default(), off, 860, 4);
        assert_eq!(v.len(), 6 * STRIDE);
        assert_eq!(
            &v[..2],
            &[-2.0, 858.0],
            "bottom left of a 2 x 1 rectangle at the top left"
        );
    }
}
