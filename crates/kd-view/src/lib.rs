//! kd-view: snapshot, command and view-request types, input events (A4.13, A11–A13, A12.2); implements PRC-11 in part.
#![deny(unsafe_code)]

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
