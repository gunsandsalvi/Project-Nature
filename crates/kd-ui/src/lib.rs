//! kd-ui: the pixel UI: views, cards and gestures (A12). α01a builds the pixel font, the UI draw list and the bottom
//! strip, with the tap that steps the hour; α01b the camera's gestures (`gestures`); views, cards and the other
//! gestures join with their alphas.

#![deny(unsafe_code)]

pub mod draw;
pub mod font;
pub mod gestures;
pub mod strip;

use gestures::{Gesture, Gestures};
use kd_view::{InputEvent, InputKind};
use strip::Strip;

/// A tap is lifted within this time and this many UI pixels of where it went down (A12.2).
pub const TAP_NS: u64 = 300_000_000;
pub const TAP_SLOP: f32 = 6.0;

/// What a touch asks of the app.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum UiAction {
    /// The strip was tapped: the next hour (until the clock runs time, α03a).
    StepHour,
    /// The fingers on the land ask the camera to move (A12.2).
    Camera(Gesture),
}

/// The UI's state between frames.
#[derive(Clone, Debug, Default)]
pub struct Ui {
    pub strip: Strip,
    /// The touch that began on the strip and may become a tap: its pointer, where it went down in UI pixels, and
    /// when.
    down: Option<(i32, [f32; 2], u64)>,
    pub gestures: Gestures,
}

impl Ui {
    /// A raw touch in screen pixels, `scale` of them a UI pixel, on a screen of `screen` UI pixels with
    /// `inset_bottom` of them under the gesture strip: any touch shows the strip; a touch beginning on the strip
    /// belongs to it, and a tap there steps the hour; any other goes to the camera's gestures (A12.2).
    pub fn input(&mut self, e: &InputEvent, scale: u32, screen: [i32; 2], inset_bottom: i32) -> Option<UiAction> {
        let p = [e.x / scale as f32, e.y / scale as f32];
        let mine = self.down.is_some_and(|d| d.0 == e.pointer);
        match e.kind {
            InputKind::Down => {
                self.strip.touch(e.t_ns);
                let (corner, size) = Strip::rect(screen, inset_bottom);
                let on_strip = p[1] >= corner[1] as f32 && p[1] < (corner[1] + size[1]) as f32;
                if on_strip && self.down.is_none() && !self.gestures.busy() {
                    self.down = Some((e.pointer, p, e.t_ns));
                    return None;
                }
            }
            InputKind::Move if mine => {
                if let Some((_, at, _)) = self.down
                    && (p[0] - at[0]).abs().max((p[1] - at[1]).abs()) > TAP_SLOP
                {
                    // Dragged off its tap, it stays the strip's until it lifts.
                    self.down = Some((e.pointer, [f32::INFINITY; 2], 0));
                }
                return None;
            }
            InputKind::Up if mine => {
                let (_, at, t) = self.down.take()?;
                let still = (p[0] - at[0]).abs().max((p[1] - at[1]).abs()) <= TAP_SLOP;
                let quick = e.t_ns.saturating_sub(t) <= TAP_NS;
                return (still && quick).then_some(UiAction::StepHour);
            }
            InputKind::Cancel if mine => {
                self.down = None;
                return None;
            }
            _ => {}
        }
        let screen_h = (screen[1] * scale as i32) as f32;
        self.gestures.input(e, scale as f32, screen_h).map(UiAction::Camera)
    }
}
