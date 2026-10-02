//! kd-view: snapshot, command and view-request types, input events and UI draw lists (A4.13, A11–A13, A12.1, A12.2);
//! implements PRC-11, PRE-32 and PLT-02 in part.
#![deny(unsafe_code)]

pub mod ui;

pub use ui::{FontAtlas, GlyphRun, UiDrawList, UiRect};

/// What a touch or pointer did (A12.2).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum InputKind {
    Down,
    Move,
    Up,
    Cancel,
}

/// One raw touch, in screen pixels, with its time in nanoseconds (A12.2).
/// Implements `PRC-11` in part, see A2.4.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct InputEvent {
    pub kind: InputKind,
    pub pointer: i32,
    pub x: f32,
    pub y: f32,
    pub t_ns: u64,
}

/// The screen's edges that the system covers (a camera cutout, the status bar), in screen pixels (A12.1: nothing
/// goes under the insets).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Insets {
    pub top: f32,
    pub bottom: f32,
    pub left: f32,
    pub right: f32,
}

/// The golden cube's pose (A11.12), until the world fills the snapshot in α03a.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct CubeView {
    pub yaw: f32,
    pub pitch: f32,
}

/// What the renderer reads each frame (A11.9); in α01a only the cube.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Snapshot {
    pub cube: Option<CubeView>,
}
