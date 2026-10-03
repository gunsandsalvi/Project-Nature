//! Building a frame's UI draw list (A12.1): rectangles and runs of text in UI pixels from the screen's top-left
//! corner, in the palette's fixed colours, each shown through the 4 × 4 Bayer pattern by its fade.

use kd_view::{FontAtlas, UiDrawList, UiItem};

use crate::font::run;

/// Adds a filled rectangle.
pub fn rect(list: &mut UiDrawList, at: [i32; 2], size: [i32; 2], colour: u8, fade: f32) {
    list.items.push(UiItem::Rect { at, size, colour, fade });
}

/// Adds a run of text, its cell's top-left corner at `at` (the cap height starts two rows down).
pub fn text(list: &mut UiDrawList, font: &FontAtlas, at: [i32; 2], colour: u8, fade: f32, words: &str) {
    list.items.push(UiItem::Text {
        at,
        colour,
        fade,
        glyphs: run(font, words),
    });
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-32
    #[test]
    fn items_in_order() {
        let font = crate::font::font();
        let mut list = UiDrawList::default();
        rect(&mut list, [0, 10], [5, 2], 3, 1.0);
        text(&mut list, &font, [1, 2], 4, 0.5, "Hi");
        assert_eq!(list.items.len(), 2);
        let UiItem::Text { glyphs, .. } = &list.items[1] else {
            panic!("text expected")
        };
        assert_eq!(glyphs.iter().map(|g| g.0).collect::<Vec<_>>(), [0, 6]);
    }
}
