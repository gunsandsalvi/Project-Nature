//! The UI draw list (A12.1): what `kd-ui` builds each frame and `kd-render`'s pass 6 draws, in UI pixels, where a UI
//! pixel is an art pixel (4 screen pixels); colours are palette indices (`PRE-01`).
//! It lives here, not in `kd-ui`, because `kd-render` may not depend on `kd-ui` (A2.3; A2.2 gives `kd-view` the UI
//! draw lists).
//! Implements `PRE-32` in part, see A12.1.

use kd_core::kinds::ColourId;

/// A solid rectangle, in UI pixels from the top left of the UI area.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct UiRect {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
    pub colour: ColourId,
}

/// A line of text whose glyph box (cap height plus descent) starts at `x`, `y`.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct GlyphRun {
    pub x: i32,
    pub y: i32,
    pub text: String,
    pub colour: ColourId,
}

/// One frame's UI.
#[derive(Clone, Debug, Default, PartialEq)]
pub struct UiDrawList {
    pub rects: Vec<UiRect>,
    pub runs: Vec<GlyphRun>,
    /// How much of the UI shows, 1 to 0: below 1 it dissolves pixel by pixel (`PRE-32`: the line fades).
    pub show: f32,
}

/// The pixel font as the renderer needs it (A12.1): a bitmap of glyph cells and each character's cell and advance,
/// made by `kd-ui` from `assets/font/`.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct FontAtlas {
    /// Cell size in pixels: the widest glyph, and cap height plus descent.
    pub cell_w: u32,
    pub cell_h: u32,
    /// Cells per row of the bitmap.
    pub cols: u32,
    /// Each character with its advance, in cell order.
    pub chars: Vec<(char, u8)>,
    /// `cols * cell_w` by rows of cells, one byte a pixel, 255 inked, top row first.
    pub width: u32,
    pub height: u32,
    pub pixels: Vec<u8>,
}

impl FontAtlas {
    /// A character's cell and advance; a missing character draws as `?`.
    pub fn find(&self, c: char) -> Option<(usize, u8)> {
        let at = |c: char| self.chars.iter().position(|(x, _)| *x == c);
        at(c).or_else(|| at('?')).map(|k| (k, self.chars[k].1))
    }
}
