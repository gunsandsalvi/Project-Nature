//! kd-view: the types the world hands the front end: snapshots, commands, view requests, input, UI draw lists,
//! sound events and text records (A4, A11 to A13).
//! α00 holds raw input, the system insets and an empty snapshot; α01a adds the sky, the font atlas and the UI draw
//! list; the world fills the snapshot from α03a.

#![deny(unsafe_code)]

/// One raw touch or pointer event, in screen pixels from the screen's top-left corner (A12.2).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct InputEvent {
    pub kind: InputKind,
    pub pointer: i32,
    pub x: f32,
    pub y: f32,
    pub t_ns: u64,
}

/// What a raw touch did (A12.2).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum InputKind {
    Down,
    Move,
    Up,
    Cancel,
}

/// The screen's edges that controls must stay clear of: system bars and the gesture strip, in screen pixels (A2.5).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Insets {
    pub left: u32,
    pub top: u32,
    pub right: u32,
    pub bottom: u32,
}

/// What the world hands the renderer each frame (A4.13, A11.9); empty until the world exists (α03a).
#[derive(Clone, Debug, Default)]
pub struct Snapshot {}

/// The sky a frame is lit by (A11.4): the sun's and the moon's directions (east, north, up) and the moon's phase from
/// `kd_core::sky`, and the air's turbidity from the weather, 0 meaning the catalogue's air until weather comes.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct SkyView {
    pub sun_dir: [f32; 3],
    pub moon_dir: [f32; 3],
    pub moon_phase: f32,
    pub turbidity: f32,
}

/// The pixel font as one bitmap (A12.1): `kd-ui` makes it from `assets/font/`, and the renderer uploads it.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct FontAtlas {
    /// The bitmap's size in pixels, and its pixels, row by row from the top: 255 on, 0 off.
    pub w: u32,
    pub h: u32,
    pub pixels: Vec<u8>,
    /// Every glyph's cell is this tall, its baseline this far from its top.
    pub cell_h: u8,
    pub baseline: u8,
    /// The glyphs, sorted by character.
    pub glyphs: Vec<(char, Glyph)>,
}

/// Where a glyph sits in the atlas, and how wide it is; it advances its width plus one pixel.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Glyph {
    pub x: u16,
    pub y: u16,
    pub w: u8,
}

impl FontAtlas {
    /// A character's glyph, or `?`'s for one the font lacks.
    pub fn glyph(&self, c: char) -> Glyph {
        let find = |c: char| {
            self.glyphs
                .binary_search_by_key(&c, |g| g.0)
                .ok()
                .map(|i| self.glyphs[i].1)
        };
        find(c).or_else(|| find('?')).unwrap_or_default()
    }
}

/// What the renderer needs besides the catalogue (A11.1).
#[derive(Clone, Debug, Default)]
pub struct Assets {
    pub font: FontAtlas,
}

/// One frame's UI, in UI pixels from the screen's top-left corner, drawn after the upscale (A12.1). A UI pixel is an
/// art pixel; colours are palette indices of the fixed colours.
#[derive(Clone, Debug, Default, PartialEq)]
pub struct UiDrawList {
    pub items: Vec<UiItem>,
}

/// An item of the UI, each shown through the 4 × 4 Bayer pattern by `fade`, 1 whole and 0 gone (`PRE-32`).
#[derive(Clone, Debug, PartialEq)]
pub enum UiItem {
    Rect {
        at: [i32; 2],
        size: [i32; 2],
        colour: u8,
        fade: f32,
    },
    /// A run of glyphs from `at`, its top-left corner: each glyph's place in the atlas and its x from `at`.
    Text {
        at: [i32; 2],
        colour: u8,
        fade: f32,
        glyphs: Vec<(i32, Glyph)>,
    },
}
