//! Laying out the UI in UI pixels (A12.1): a UI pixel is an art pixel, 4 screen pixels; nothing goes under the
//! insets. In α01a, the one strip at the top.
//! Implements `PRE-32` in part, see A12.1.

use kd_core::kinds::ColourId;
pub use kd_view::{GlyphRun, UiDrawList, UiRect};

/// Lists and labels sit on an 11-pixel line (A12.1).
pub const LINE: i32 = 11;
/// The strip's text starts this far from its left edge.
pub const PAD_X: i32 = 3;
/// A glyph box (cap plus descent) sits this far below the top of its line.
pub const PAD_Y: i32 = 1;

/// The UI area's width and the first UI row below the top inset, for a window `w_px` wide at `scale` screen pixels
/// a UI pixel.
pub fn area(w_px: u32, top_inset_px: f32, scale: u32) -> (i32, i32) {
    let s = scale.max(1);
    let w = w_px.div_ceil(s) as i32;
    let top = (top_inset_px.max(0.0) / s as f32).ceil() as i32;
    (w, top)
}

/// The strip under the top inset holding `lines` of text: its panel and the runs, one a line.
pub fn strip(
    w_px: u32,
    top_inset_px: f32,
    scale: u32,
    lines: &[String],
    panel: ColourId,
    text: ColourId,
) -> UiDrawList {
    let (w, top) = area(w_px, top_inset_px, scale);
    let h = LINE * lines.len().max(1) as i32;
    UiDrawList {
        rects: vec![UiRect {
            x: 0,
            y: top,
            w,
            h,
            colour: panel,
        }],
        runs: lines
            .iter()
            .enumerate()
            .map(|(i, l)| GlyphRun {
                x: PAD_X,
                y: top + PAD_Y + LINE * i as i32,
                text: l.clone(),
                colour: text,
            })
            .collect(),
        show: 1.0,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-32 PLT-02
    #[test]
    fn strip_under_insets() {
        let lines = vec!["Kindling".to_string(), "a01a".to_string()];
        for (w, inset) in [(1080, 0.0), (1080, 96.0), (1080, 97.0), (2404, 0.0), (412, 47.5)] {
            let d = strip(w, inset, 4, &lines, ColourId(2), ColourId(11));
            let r = d.rects[0];
            assert!(
                (r.y * 4) as f32 >= inset,
                "the strip's top {} under the inset {inset}",
                r.y * 4
            );
            assert!((((r.y - 1) * 4) as f32) < inset || r.y == 0, "and right under it");
            assert_eq!((r.x, r.w, r.h), (0, w.div_ceil(4) as i32, 22));
            assert!(
                d.runs.iter().all(|g| g.y > r.y && g.y + 9 <= r.y + r.h),
                "text inside the strip"
            );
        }
    }
}
