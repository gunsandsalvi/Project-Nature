//! The pixel font (A12.1): proportional glyphs with a 7-pixel cap height and 2-pixel descenders, read from the
//! text-art file `assets/font/glyphs-7.txt`; measuring, wrapping at spaces, and the atlas the renderer draws from.
//! Implements `PRE-32` in part, see A12.1.

use kd_view::FontAtlas;

/// Glyph rows: cap height plus descent.
pub const ROWS: usize = 9;

/// One glyph: its character, advance (width plus one pixel of spacing) and rows, `true` where inked.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Glyph {
    pub ch: char,
    pub advance: u8,
    pub rows: Vec<Vec<bool>>,
}

/// A parsed font.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct Font {
    pub cap: u8,
    pub descent: u8,
    pub glyphs: Vec<Glyph>,
}

/// One wrapped line and its width in pixels.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Line {
    pub text: String,
    pub w: u32,
}

impl Font {
    /// Reads the text-art format: comment lines (`# ...`) and blank lines between glyphs, the header
    /// `cap 7 descent 2`, then per glyph `glyph <c> advance <n>` (`space` for the space) and 9 rows of `.` and `#`,
    /// each `advance - 1` wide.
    pub fn parse(text: &str) -> Result<Font, String> {
        let mut font = Font::default();
        let mut lines = text.lines().enumerate();
        while let Some((n, line)) = lines.next() {
            let t = line.trim_end();
            if t.is_empty() || t.starts_with("# ") {
                continue;
            }
            let w: Vec<&str> = t.split(' ').collect();
            match w.as_slice() {
                ["cap", c, "descent", d] => {
                    font.cap = c.parse().map_err(|_| format!("line {}: cap", n + 1))?;
                    font.descent = d.parse().map_err(|_| format!("line {}: descent", n + 1))?;
                }
                ["glyph", c, "advance", a] => {
                    let ch = match *c {
                        "space" => ' ',
                        s if s.chars().count() == 1 => s.chars().next().unwrap_or(' '),
                        s => return Err(format!("line {}: `{s}` is not one character", n + 1)),
                    };
                    let advance: u8 = a.parse().map_err(|_| format!("line {}: advance", n + 1))?;
                    let mut rows = Vec::with_capacity(ROWS);
                    for _ in 0..ROWS {
                        let (m, r) = lines.next().ok_or(format!("glyph `{ch}`: fewer than {ROWS} rows"))?;
                        let r = r.trim_end();
                        if r.chars().count() + 1 != usize::from(advance) || !r.chars().all(|c| c == '.' || c == '#') {
                            return Err(format!(
                                "line {}: glyph `{ch}`'s row is not {} of . and #",
                                m + 1,
                                advance - 1
                            ));
                        }
                        rows.push(r.chars().map(|c| c == '#').collect());
                    }
                    if font.glyphs.iter().any(|g| g.ch == ch) {
                        return Err(format!("glyph `{ch}` twice"));
                    }
                    font.glyphs.push(Glyph { ch, advance, rows });
                }
                _ => return Err(format!("line {}: `{t}` is neither a header nor a glyph", n + 1)),
            }
        }
        if usize::from(font.cap) + usize::from(font.descent) != ROWS {
            return Err(format!(
                "cap {} and descent {} do not make {ROWS} rows",
                font.cap, font.descent
            ));
        }
        Ok(font)
    }

    /// A character's glyph; a missing character draws as `?`.
    pub fn glyph(&self, c: char) -> Option<&Glyph> {
        self.glyphs
            .iter()
            .find(|g| g.ch == c)
            .or_else(|| self.glyphs.iter().find(|g| g.ch == '?'))
    }

    /// The width of a line in pixels: the sum of its advances.
    pub fn measure(&self, s: &str) -> u32 {
        s.chars()
            .map(|c| self.glyph(c).map_or(0, |g| u32::from(g.advance)))
            .sum()
    }

    /// Wraps text at spaces into lines no wider than `max_w`; a word wider than that stands alone on its line.
    pub fn layout(&self, s: &str, max_w: u32) -> Vec<Line> {
        let mut out: Vec<Line> = Vec::new();
        let mut cur = String::new();
        for word in s.split(' ') {
            let candidate = if cur.is_empty() {
                word.to_string()
            } else {
                format!("{cur} {word}")
            };
            if cur.is_empty() || self.measure(&candidate) <= max_w {
                cur = candidate;
            } else {
                out.push(Line {
                    w: self.measure(&cur),
                    text: std::mem::take(&mut cur),
                });
                cur = word.to_string();
            }
        }
        out.push(Line {
            w: self.measure(&cur),
            text: cur,
        });
        out
    }

    /// The renderer's atlas: one cell per glyph, 16 to a row.
    pub fn atlas(&self) -> FontAtlas {
        let cell_w = self.glyphs.iter().map(|g| u32::from(g.advance)).max().unwrap_or(1);
        let cell_h = ROWS as u32;
        let cols = 16;
        let n = self.glyphs.len() as u32;
        let (width, height) = (cols * cell_w, n.div_ceil(cols) * cell_h);
        let mut pixels = vec![0u8; (width * height) as usize];
        for (k, g) in self.glyphs.iter().enumerate() {
            let (cx, cy) = ((k as u32 % cols) * cell_w, (k as u32 / cols) * cell_h);
            for (y, row) in g.rows.iter().enumerate() {
                for (x, ink) in row.iter().enumerate() {
                    if *ink {
                        pixels[((cy + y as u32) * width + cx + x as u32) as usize] = 255;
                    }
                }
            }
        }
        FontAtlas {
            cell_w,
            cell_h,
            cols,
            chars: self.glyphs.iter().map(|g| (g.ch, g.advance)).collect(),
            width,
            height,
            pixels,
        }
    }
}

/// The font file, compiled in.
pub const GLYPHS_7: &str = include_str!("../../../assets/font/glyphs-7.txt");

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-32
    #[test]
    fn every_glyph_present() {
        let f = Font::parse(GLYPHS_7).unwrap();
        assert_eq!((f.cap, f.descent), (7, 2));
        let want: Vec<char> = (32u8..=126).map(char::from).chain("·–—’…°×".chars()).collect();
        assert_eq!(want.len(), 102);
        for c in &want {
            let g = f
                .glyphs
                .iter()
                .find(|g| g.ch == *c)
                .unwrap_or_else(|| panic!("no glyph `{c}`"));
            assert_eq!(g.rows.len(), 9, "`{c}`");
            assert!((2..=6).contains(&g.advance), "`{c}`: widths 1 to 5 plus spacing");
            if *c != ' ' {
                assert!(g.rows.iter().flatten().any(|p| *p), "`{c}` is blank");
            }
        }
        assert_eq!(f.glyphs.len(), 102);
        // Capitals stand 7 pixels tall on the baseline, with nothing below it.
        let a = f.glyph('A').unwrap();
        assert!(a.rows[0].iter().any(|p| *p) && a.rows[6].iter().any(|p| *p));
        assert!(!a.rows[7].iter().any(|p| *p) && !a.rows[8].iter().any(|p| *p));
        assert!(
            f.glyph('g').unwrap().rows[8].iter().any(|p| *p),
            "descenders reach row 9"
        );
    }

    // checks: PRE-32
    #[test]
    fn measure_known() {
        let f = Font::parse(GLYPHS_7).unwrap();
        let sum: u32 = "Kindling".chars().map(|c| u32::from(f.glyph(c).unwrap().advance)).sum();
        assert_eq!(f.measure("Kindling"), sum);
        assert_eq!(f.measure(""), 0);
        assert_eq!(f.measure("\u{1}"), f.measure("?"), "a missing character draws as ?");
    }

    // checks: PRE-32
    #[test]
    fn layout_wraps_at_spaces() {
        let f = Font::parse(GLYPHS_7).unwrap();
        let w = f.measure("Kindling a01a");
        let lines = f.layout("Kindling a01a · 120 Hz", w);
        assert_eq!(lines[0].text, "Kindling a01a");
        assert!(lines.iter().all(|l| l.w <= w));
        assert_eq!(f.layout("one", 1).len(), 1, "a long word stands alone");
        let at = f.atlas();
        assert_eq!(at.chars.len(), 102);
        assert_eq!(at.pixels.len() as u32, at.width * at.height);
    }
}
