//! kd-view: the types the world hands the front end: snapshots, commands, view requests, input, UI draw lists,
//! sound events and text records (A4, A11 to A13).
//! α00 holds raw input, the system insets and an empty snapshot; the world fills the snapshot from α03a.

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
