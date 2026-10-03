//! The bottom strip (`PRE-32`, A12.1, A12.2): 24 UI pixels above the gesture strip and the insets, holding the hour,
//! which a tap steps until the clock runs time (α03a), and under it the version line. Any touch shows it for 3
//! seconds; then it dissolves through the 4 × 4 Bayer pattern, leaving nothing else on the screen.
//!
//! Implements PRE-32, see A12.1: the strip and the version line, nothing else on screen.

use kd_view::{FontAtlas, UiDrawList, UiItem};

use crate::font::run;

/// The strip's height in UI pixels.
pub const HEIGHT: i32 = 24;
/// How long a touch shows it, and how long it then takes to dissolve.
pub const SHOWN_NS: u64 = 3_000_000_000;
pub const DISSOLVE_NS: u64 = 600_000_000;
/// Text starts this far in from the screen's left edge.
const TEXT_X: i32 = 8;

/// The fixed colours the strip is drawn in, as palette indices (A11.3).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Colours {
    pub panel: u8,
    pub line: u8,
    pub text: u8,
    pub dim: u8,
}

impl Colours {
    /// The UI's fixed colours from the catalogue, where palette index `i` is its `i`th colour (A11.3).
    pub fn from_catalogue(cat: &kd_data::Catalogue) -> Colours {
        let at = |id: &str| cat.colours.iter().position(|c| c.id == id).unwrap_or(0) as u8;
        Colours {
            panel: at("ui_panel"),
            line: at("ui_line"),
            text: at("ui_text"),
            dim: at("ui_text_dim"),
        }
    }
}

#[derive(Clone, Debug, Default)]
pub struct Strip {
    last_touch_ns: Option<u64>,
}

impl Strip {
    /// Shows the strip from `now_ns` for `SHOWN_NS`.
    pub fn touch(&mut self, now_ns: u64) {
        self.last_touch_ns = Some(now_ns);
    }

    /// Whether anything has shown the strip yet.
    pub fn touched(&self) -> bool {
        self.last_touch_ns.is_some()
    }

    /// How much of the strip shows: 1 for 3 seconds after a touch, then down to 0 as it dissolves.
    pub fn visibility(&self, now_ns: u64) -> f32 {
        let Some(t) = self.last_touch_ns else {
            return 0.0;
        };
        let age = now_ns.saturating_sub(t);
        if age <= SHOWN_NS {
            1.0
        } else if age >= SHOWN_NS + DISSOLVE_NS {
            0.0
        } else {
            1.0 - (age - SHOWN_NS) as f32 / DISSOLVE_NS as f32
        }
    }

    /// The strip's corner and size: the screen's width, `HEIGHT` tall, its foot on the bottom inset (UI pixels).
    pub fn rect(screen: [i32; 2], inset_bottom: i32) -> ([i32; 2], [i32; 2]) {
        ([0, screen[1] - inset_bottom - HEIGHT], [screen[0], HEIGHT])
    }

    /// Adds the strip to a frame's list while it shows: its panel, a line along its top, the hour and the version.
    #[allow(clippy::too_many_arguments)]
    pub fn draw(
        &self,
        list: &mut UiDrawList,
        font: &FontAtlas,
        now_ns: u64,
        screen: [i32; 2],
        inset_bottom: i32,
        hour: &str,
        version: &str,
        c: Colours,
    ) {
        let fade = self.visibility(now_ns);
        if fade <= 0.0 {
            return;
        }
        let (at, size) = Strip::rect(screen, inset_bottom);
        list.items.push(UiItem::Rect {
            at,
            size,
            colour: c.panel,
            fade,
        });
        list.items.push(UiItem::Rect {
            at,
            size: [size[0], 1],
            colour: c.line,
            fade,
        });
        for (text, dy, colour) in [(hour, 1, c.text), (version, 12, c.dim)] {
            list.items.push(UiItem::Text {
                at: [TEXT_X, at[1] + dy],
                colour,
                fade,
                glyphs: run(font, text),
            });
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{Ui, UiAction};
    use kd_view::{InputEvent, InputKind};

    const MS: u64 = 1_000_000;

    fn touch(kind: InputKind, x: f32, y: f32, t_ms: u64) -> InputEvent {
        InputEvent {
            kind,
            pointer: 1,
            x,
            y,
            t_ns: t_ms * MS,
        }
    }

    // checks: PRE-32 PRE-30
    #[test]
    fn tap_steps_the_hour() {
        // A 1080 × 2404 screen, 4 screen pixels a UI pixel, no inset: the strip spans UI rows 577 to 600.
        let (screen, inset) = ([270, 601], 0);
        let on_strip = 590.0 * 4.0;
        let mut ui = Ui::default();
        let tap = |ui: &mut Ui, x: f32, y: f32, t0: u64, t1: u64, moved: f32| {
            let mut out = ui.input(&touch(InputKind::Down, x, y, t0), 4, screen, inset);
            if moved > 0.0 {
                out = out.or(ui.input(&touch(InputKind::Move, x + moved, y, t0 + 10), 4, screen, inset));
            }
            out.or(ui.input(&touch(InputKind::Up, x + moved, y, t1), 4, screen, inset))
        };
        assert_eq!(tap(&mut ui, 400.0, on_strip, 0, 120, 0.0), Some(UiAction::StepHour));
        // Held too long, dragged, or above the strip: no step.
        assert_eq!(tap(&mut ui, 400.0, on_strip, 1000, 1400, 0.0), None);
        assert_eq!(tap(&mut ui, 400.0, on_strip, 2000, 2100, 40.0), None);
        assert_eq!(tap(&mut ui, 400.0, 1000.0, 3000, 3050, 0.0), None);
        // A cancelled touch is no tap.
        ui.input(&touch(InputKind::Down, 400.0, on_strip, 4000), 4, screen, inset);
        ui.input(&touch(InputKind::Cancel, 400.0, on_strip, 4010), 4, screen, inset);
        assert_eq!(
            ui.input(&touch(InputKind::Up, 400.0, on_strip, 4020), 4, screen, inset),
            None
        );
        // Any touch shows the strip for 3 seconds, then it dissolves within 0.6 s more.
        let mut s = Strip::default();
        assert_eq!(s.visibility(0), 0.0);
        s.touch(10 * MS);
        assert_eq!(s.visibility(10 * MS + SHOWN_NS), 1.0);
        let mid = s.visibility(10 * MS + SHOWN_NS + DISSOLVE_NS / 2);
        assert!(mid > 0.4 && mid < 0.6, "{mid}");
        assert_eq!(s.visibility(10 * MS + SHOWN_NS + DISSOLVE_NS), 0.0);
        // Above the inset: the strip moves up with the gesture strip.
        assert_eq!(Strip::rect([270, 601], 12).0, [0, 601 - 12 - HEIGHT]);
    }
}
