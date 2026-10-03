//! The pixel font (A12.1): `assets/font/glyphs-7.txt`, drawn in-house as text art, read into one bitmap the renderer
//! uploads, and runs of text laid out with it. Its cells are 11 pixels tall: two rows for the accents of capitals,
//! the 7-pixel cap height, and 2-pixel descenders.
//!
//! Implements PRE-32 and PRE-01, see A12.1: the game's own pixel font, one UI pixel an art pixel.

use kd_view::{FontAtlas, Glyph};

/// The font's text art.
pub const GLYPHS_7: &str = include_str!("../../../assets/font/glyphs-7.txt");
/// Rows in a glyph's cell, and the row under the baseline.
pub const CELL_H: usize = 11;
pub const BASELINE: usize = 9;
/// Glyphs are at most this wide; the atlas packs them in cells one pixel wider, this many a row.
pub const MAX_W: usize = 7;
const PER_ROW: usize = 16;

/// A glyph as text art: its rows, each pixel on or off.
pub type Rows = Vec<Vec<bool>>;

/// Each glyph of a text-art font, in file order: its character and its rows.
pub fn parse(text: &str) -> Result<Vec<(char, Rows)>, String> {
    let mut out = Vec::new();
    let mut lines = text.lines().enumerate().peekable();
    while let Some((n, line)) = lines.next() {
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        let Some(name) = line.strip_prefix("== ") else {
            return Err(format!("line {}: not a glyph's `== <char>`", n + 1));
        };
        let c = match name {
            "space" => ' ',
            _ if name.chars().count() == 1 => name.chars().next().unwrap_or(' '),
            _ => return Err(format!("line {}: `{name}` is not one character", n + 1)),
        };
        let mut rows = Vec::with_capacity(CELL_H);
        for _ in 0..CELL_H {
            let Some((m, row)) = lines.next() else {
                return Err(format!("glyph {c:?}: fewer than {CELL_H} rows"));
            };
            if row.is_empty() || !row.chars().all(|p| p == '#' || p == '.') {
                return Err(format!("line {}: a row of `#` and `.` expected", m + 1));
            }
            rows.push(row.chars().map(|p| p == '#').collect::<Vec<bool>>());
        }
        let w = rows[0].len();
        if w > MAX_W || rows.iter().any(|r| r.len() != w) {
            return Err(format!("glyph {c:?}: rows of one width, at most {MAX_W}, expected"));
        }
        if out.iter().any(|(d, _)| *d == c) {
            return Err(format!("glyph {c:?} drawn twice"));
        }
        out.push((c, rows));
    }
    Ok(out)
}

/// The font as one bitmap, glyphs packed in cells `MAX_W + 1` wide, `PER_ROW` a row.
pub fn atlas(text: &str) -> Result<FontAtlas, String> {
    let glyphs = parse(text)?;
    let cell_w = MAX_W + 1;
    let w = PER_ROW * cell_w;
    let h = glyphs.len().div_ceil(PER_ROW) * CELL_H;
    let mut pixels = vec![0u8; w * h];
    let mut index = Vec::with_capacity(glyphs.len());
    for (i, (c, rows)) in glyphs.iter().enumerate() {
        let (x0, y0) = ((i % PER_ROW) * cell_w, (i / PER_ROW) * CELL_H);
        for (y, row) in rows.iter().enumerate() {
            for (x, &on) in row.iter().enumerate() {
                pixels[(y0 + y) * w + x0 + x] = if on { 255 } else { 0 };
            }
        }
        let glyph = Glyph {
            x: x0 as u16,
            y: y0 as u16,
            w: rows[0].len() as u8,
        };
        index.push((*c, glyph));
    }
    index.sort_by_key(|g| g.0);
    Ok(FontAtlas {
        w: w as u32,
        h: h as u32,
        pixels,
        cell_h: CELL_H as u8,
        baseline: BASELINE as u8,
        glyphs: index,
    })
}

/// The game's font.
pub fn font() -> FontAtlas {
    atlas(GLYPHS_7).expect("assets/font/glyphs-7.txt is checked by font::tests")
}

/// A run of text: each glyph with its x from the run's start.
pub fn run(font: &FontAtlas, text: &str) -> Vec<(i32, Glyph)> {
    let mut x = 0;
    text.chars()
        .map(|c| {
            let g = font.glyph(c);
            let at = x;
            x += i32::from(g.w) + 1;
            (at, g)
        })
        .collect()
}

/// A run's width in pixels, without the space after its last glyph.
pub fn width(font: &FontAtlas, text: &str) -> i32 {
    run(font, text).last().map_or(0, |(x, g)| x + i32::from(g.w))
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-32
    #[test]
    fn every_glyph_fits_its_cell() {
        let glyphs = parse(GLYPHS_7).expect("the font parses");
        // Every printable ASCII character, the middle dot of the version line, and the accented letters.
        for c in (32u8..127)
            .map(char::from)
            .chain("·áàâäāéèêëēíìîïīóòôöōúùûüūñçÁÉÍÓÚÑÇ".chars())
        {
            assert!(glyphs.iter().any(|g| g.0 == c), "no glyph for {c:?}");
        }
        for (c, rows) in &glyphs {
            assert_eq!(rows.len(), CELL_H, "{c:?}");
            assert!((1..=MAX_W).contains(&rows[0].len()), "{c:?}");
            // Lowercase letters and digits leave the two accent rows above the cap height free.
            if c.is_ascii_alphanumeric() {
                assert!(
                    rows[..2].iter().all(|r| r.iter().all(|&p| !p)),
                    "{c:?} reaches above its cap"
                );
            }
        }
        // The atlas holds each glyph's pixels where its index says.
        let f = font();
        let a = f.glyph('A');
        let at = |x: u16, y: u16| f.pixels[y as usize * f.w as usize + x as usize];
        assert_eq!((at(a.x + 2, a.y + 2), at(a.x, a.y + 2), a.w), (255, 0, 5));
        assert_eq!(f.glyph('\u{2603}'), f.glyph('?'));
        // Text lays out by each glyph's width plus a pixel.
        assert_eq!(width(&f, "Il"), 3 + 1 + 2);
        assert_eq!(run(&f, "ab").iter().map(|g| g.0).collect::<Vec<_>>(), [0, 5]);
        // A malformed font fails with its line.
        assert!(parse("== A\n#.\n").unwrap_err().contains("fewer than"));
        assert!(parse("== AB\n").unwrap_err().contains("not one character"));
    }

    // checks: PRE-32
    #[test]
    fn descenders_two_pixels() {
        let glyphs = parse(GLYPHS_7).unwrap();
        let ink = |c: char, row: usize| glyphs.iter().find(|g| g.0 == c).unwrap().1[row].iter().any(|&p| p);
        // g, j, p, q and y reach the bottom row, two below the baseline; capitals and digits stand on the baseline.
        for c in "gjpqy".chars() {
            assert!(ink(c, BASELINE + 1), "{c} has no 2-pixel descender");
        }
        for c in ('A'..='Z').chain('0'..='9').chain("abcdefhiklmnorstuvwxz".chars()) {
            assert!(
                !ink(c, BASELINE) && !ink(c, BASELINE + 1),
                "{c} goes under the baseline"
            );
            assert!(
                ink(c, BASELINE - 1) || "\"'^`".contains(c),
                "{c} does not stand on the baseline"
            );
        }
        // Capitals are 7 pixels tall and lowercase letters 5, from row 4.
        assert!(ink('E', 2) && ink('E', 8) && !ink('x', 3) && ink('x', 4));
    }
}
