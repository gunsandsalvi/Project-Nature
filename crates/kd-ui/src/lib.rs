//! kd-ui: the pixel UI: views, cards and gestures (A12). α01a builds the pixel font, the UI draw list and the bottom
//! strip, with the tap that steps the hour; views, cards and the other gestures join with their alphas.

#![deny(unsafe_code)]

pub mod draw;
pub mod font;
pub mod strip;

use kd_view::{InputEvent, InputKind};
use strip::Strip;

/// A tap is lifted within this time and this many UI pixels of where it went down (A12.2).
pub const TAP_NS: u64 = 300_000_000;
pub const TAP_SLOP: f32 = 6.0;

/// What a gesture asks of the app.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum UiAction {
    /// The strip was tapped: the next hour (until the clock runs time, α03a).
    StepHour,
}

/// The UI's state between frames.
#[derive(Clone, Debug, Default)]
pub struct Ui {
    pub strip: Strip,
    /// The touch that may become a tap: its pointer, where it went down in UI pixels, and when.
    down: Option<(i32, [f32; 2], u64)>,
}

impl Ui {
    /// A raw touch in screen pixels, `scale` of them a UI pixel, on a screen of `screen` UI pixels with
    /// `inset_bottom` of them under the gesture strip: any touch shows the strip, and a tap on it steps the hour
    /// (A12.2).
    pub fn input(&mut self, e: &InputEvent, scale: u32, screen: [i32; 2], inset_bottom: i32) -> Option<UiAction> {
        let p = [e.x / scale as f32, e.y / scale as f32];
        match e.kind {
            InputKind::Down => {
                self.strip.touch(e.t_ns);
                self.down = Some((e.pointer, p, e.t_ns));
                None
            }
            InputKind::Move => {
                if let Some((id, at, _)) = self.down
                    && id == e.pointer
                    && (p[0] - at[0]).abs().max((p[1] - at[1]).abs()) > TAP_SLOP
                {
                    self.down = None;
                }
                None
            }
            InputKind::Up => {
                let (id, at, t) = self.down.take()?;
                let still = (p[0] - at[0]).abs().max((p[1] - at[1]).abs()) <= TAP_SLOP;
                let quick = e.t_ns.saturating_sub(t) <= TAP_NS;
                let (corner, size) = Strip::rect(screen, inset_bottom);
                let on_strip = p[1] >= corner[1] as f32 && p[1] < (corner[1] + size[1]) as f32;
                (id == e.pointer && still && quick && on_strip).then_some(UiAction::StepHour)
            }
            InputKind::Cancel => {
                self.down = None;
                None
            }
        }
    }
}
