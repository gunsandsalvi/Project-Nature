//! kd-ui: the pixel UI (A12.1, A12.2); in α01a the pixel font and the version strip that any touch shows for three
//! seconds (`PRE-32`), whose taps step the palette row (`PRE-30`). Implements PRE-32 in part.

pub mod draw;
pub mod font;

use kd_core::kinds::ColourId;
use kd_data::Catalogue;
use kd_data::schema::LightRole;
use kd_view::{InputEvent, InputKind, Insets, UiDrawList};

pub use font::Font;

/// Any touch shows the strip this long, seconds (A12.2, `PRE-32`).
pub const SHOW_S: f64 = 3.0;
/// Its last part dissolves, seconds.
pub const FADE_S: f64 = 0.5;
/// A tap lifts within this time, ns, and this many UI pixels (A12.2).
pub const TAP_NS: u64 = 300_000_000;
pub const TAP_UI_PX: f32 = 6.0;

#[derive(Clone, Copy, Debug)]
struct Down {
    pointer: i32,
    x: f32,
    y: f32,
    t_ns: u64,
}

/// The UI, immediate-mode (A12.1): input in, a draw list out each frame.
pub struct Ui {
    font: Font,
    panel: ColourId,
    text: ColourId,
    rows: Vec<String>,
    row: usize,
    w_px: u32,
    scale: u32,
    insets: Insets,
    now_s: f64,
    shown_at: Option<f64>,
    touched: bool,
    down: Option<Down>,
    strip_h: i32,
}

impl Ui {
    /// The UI over a font and the catalogue's colours (panels `night`, text `s6`, A12.1) and palette versions.
    pub fn new(font: Font, cat: &Catalogue) -> Ui {
        let rows = cat
            .light_rows(LightRole::Version)
            .map(|v| v.id.strip_prefix("version_").unwrap_or(&v.id).to_string())
            .collect();
        Ui {
            font,
            panel: cat.colour("night").unwrap_or_default(),
            text: cat.colour("s6").unwrap_or_default(),
            rows,
            row: 0,
            w_px: 1,
            scale: 4,
            insets: Insets::default(),
            now_s: 0.0,
            shown_at: None,
            touched: false,
            down: None,
            strip_h: 0,
        }
    }

    pub fn font(&self) -> &Font {
        &self.font
    }

    /// The window's width in screen pixels and the screen pixels a UI pixel spans.
    pub fn resize(&mut self, w_px: u32, scale: u32) {
        self.w_px = w_px.max(1);
        self.scale = scale.max(1);
    }

    pub fn set_insets(&mut self, i: Insets) {
        self.insets = i;
    }

    /// The palette row the strip has stepped to: 0 dusk, 1 dawn, 2 day, 3 night (`PRE-30`).
    pub fn palette_row(&self) -> usize {
        self.row
    }

    fn visible(&self) -> bool {
        self.shown_at.is_some_and(|t| self.now_s - t < SHOW_S)
    }

    /// Whether a point in screen pixels is on the strip.
    fn on_strip(&self, y: f32) -> bool {
        let (_, top) = draw::area(self.w_px, self.insets.top, self.scale);
        let y_ui = y / self.scale as f32;
        self.visible() && y_ui >= top as f32 && y_ui < (top + self.strip_h) as f32
    }

    /// A raw touch (A12.2): any touch shows the strip; a tap on the visible strip steps the palette row. Returns
    /// whether the UI took the touch, so the world does not see it.
    pub fn input(&mut self, e: &InputEvent) -> bool {
        match e.kind {
            InputKind::Down => {
                self.touched = true;
                if self.down.is_none() && self.on_strip(e.y) {
                    self.down = Some(Down {
                        pointer: e.pointer,
                        x: e.x,
                        y: e.y,
                        t_ns: e.t_ns,
                    });
                    return true;
                }
                false
            }
            InputKind::Move => self.down.is_some_and(|d| d.pointer == e.pointer),
            InputKind::Up | InputKind::Cancel => {
                let Some(d) = self.down.filter(|d| d.pointer == e.pointer) else {
                    return false;
                };
                self.down = None;
                let moved = (e.x - d.x).abs().max((e.y - d.y).abs()) / self.scale as f32;
                if e.kind == InputKind::Up && e.t_ns.saturating_sub(d.t_ns) <= TAP_NS && moved <= TAP_UI_PX {
                    self.row = (self.row + 1) % self.rows.len().max(1);
                }
                true
            }
        }
    }

    /// One frame's UI at `now_s` real seconds: the strip `Kindling <build> · <Hz> Hz · <ms> ms · <row>` while shown,
    /// dissolving over its last half second.
    pub fn build(&mut self, now_s: f64, build: &str, hz: f32, gl_ms: f32) -> UiDrawList {
        self.now_s = now_s;
        if self.touched {
            self.touched = false;
            self.shown_at = Some(now_s);
        }
        if !self.visible() {
            self.strip_h = 0;
            return UiDrawList::default();
        }
        let row = self.rows.get(self.row).map(String::as_str).unwrap_or("");
        let text = format!("Kindling {build} · {hz:.0} Hz · {gl_ms:.1} ms · {row}");
        let (w, _) = draw::area(self.w_px, self.insets.top, self.scale);
        let lines: Vec<String> = self
            .font
            .layout(&text, (w - 2 * draw::PAD_X).max(1) as u32)
            .into_iter()
            .map(|l| l.text)
            .collect();
        let mut list = draw::strip(self.w_px, self.insets.top, self.scale, &lines, self.panel, self.text);
        self.strip_h = list.rects[0].h;
        let left = SHOW_S - (now_s - self.shown_at.unwrap_or(now_s));
        list.show = (left / FADE_S).clamp(0.0, 1.0) as f32;
        list
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn ui() -> Ui {
        let mut cat = Catalogue::default();
        for (i, n) in ["void", "night", "s6"].iter().enumerate() {
            cat.body.colours.push(kd_data::blob::ColourRec {
                name: n.to_string(),
                rgb: [i as u8; 3],
                family: 0,
            });
        }
        let mut u = Ui::new(Font::parse(font::GLYPHS_7).unwrap(), &cat);
        u.rows = ["dusk", "dawn", "day", "night"].map(String::from).to_vec();
        u.resize(1080, 4);
        u
    }

    fn ev(kind: InputKind, y: f32, t_ms: u64) -> InputEvent {
        InputEvent {
            kind,
            pointer: 0,
            x: 500.0,
            y,
            t_ns: t_ms * 1_000_000,
        }
    }

    // checks: PRE-32 PRE-30
    #[test]
    fn touch_shows_tap_steps_then_it_goes() {
        let mut u = ui();
        assert!(
            u.build(0.0, "a01a", 120.0, 0.4).rects.is_empty(),
            "the world alone until touched"
        );
        assert!(
            !u.input(&ev(InputKind::Down, 1200.0, 0)),
            "a touch on the world is the world's"
        );
        u.input(&ev(InputKind::Up, 1200.0, 50));
        let d = u.build(1.0, "a01a", 120.0, 0.4);
        assert_eq!(d.show, 1.0);
        assert!(
            d.runs[0].text.starts_with("Kindling a01a · 120 Hz · 0.4 ms · dusk"),
            "{}",
            d.runs[0].text
        );
        assert!(
            u.input(&ev(InputKind::Down, 10.0, 1000)),
            "a touch on the strip is the UI's"
        );
        assert!(u.input(&ev(InputKind::Up, 10.0, 1100)));
        assert_eq!(u.palette_row(), 1, "a tap steps dusk to dawn");
        let d = u.build(1.2, "a01a", 120.0, 0.4);
        assert!(d.runs[0].text.ends_with("dawn"));
        assert!(u.build(3.9, "a01a", 120.0, 0.4).show < 1.0, "it dissolves at the end");
        assert!(
            u.build(4.3, "a01a", 120.0, 0.4).rects.is_empty(),
            "gone three seconds after the last touch"
        );
        u.input(&ev(InputKind::Down, 1200.0, 4900));
        u.input(&ev(InputKind::Up, 1200.0, 4950));
        u.build(5.0, "a01a", 120.0, 0.4);
        for t in 0..3 {
            assert!(u.input(&ev(InputKind::Down, 10.0, 5000 + t * 400)));
            u.input(&ev(InputKind::Up, 10.0, 5100 + t * 400));
            u.build(5.2 + t as f64 * 0.4, "a01a", 120.0, 0.4);
        }
        assert_eq!(u.palette_row(), 0, "four taps in all come back to dusk");
    }
}
