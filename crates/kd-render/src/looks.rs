//! The palette under the light (A11.3): `void`, the fixed colours, then each look's ladder of steps on one path
//! through the light, from deep shade to open shade to full sun, at equal steps of the light's lightness; the path's
//! range, from which the shaders pick a step; and the tables post maps colours through. Pure CPU, no GL (A11.13
//! rule 1), every function through `kd_core::m`, so the row has the same bits on every target.
//!
//! Implements PRE-20 and PRE-01, see A11.3: each material's ladder is its colour under the light of the hour, and
//! every art pixel is one palette colour.

use kd_core::m;
use kd_data::{Air, Catalogue};

use crate::light::{LUM, Light, Rgb, dot, haze_colour, shade, to_oklab};

/// Entries in the palette texture, `void` included (A11.3).
pub const PALETTE_SIZE: usize = 256;
/// Rows in the tables texture (A11.3); rows past the last named one are unused.
pub const TABLE_ROWS: usize = 16;
/// The share of the sky deep shade sees, where every ladder starts (A11.3).
pub const DEEP_SHADE_SKY: f32 = 0.3;

/// The tables' rows: each maps every palette index to another (A11.3).
pub mod table {
    /// The look two steps down.
    pub const OUTLINE: usize = 0;
    /// The look's top step.
    pub const EDGE: usize = 1;
    /// The nearest look colour to the colour mixed toward the haze, levels 1–3.
    pub const HAZE: [usize; 3] = [2, 3, 4];
    /// The look relit by a fire, levels 1–3; fire's light arrives with α14a, and until then these rows leave every
    /// colour as it is.
    pub const WARM: [usize; 3] = [5, 6, 7];
    /// Toward a glow's colour, rings 1–2; as `WARM`, until α14a.
    pub const GLOW: [usize; 2] = [8, 9];
}

/// Where a look's ladder sits in the palette: steps `base` to `base + steps − 1`.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Ladder {
    pub base: u8,
    pub steps: u8,
}

/// The palette's layout, fixed by the catalogue (A11.3): the fixed colours from index 0 (`void`) in number order,
/// then each look's ladder in number order.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Layout {
    /// Fixed colours: index `i` is the catalogue's `i`th colour.
    pub fixed: usize,
    /// One ladder a look, in the catalogue's order.
    pub ladders: Vec<Ladder>,
    /// Entries used, `void` included.
    pub len: usize,
}

impl Layout {
    /// The layout of a catalogue's palette; it fails when the entries outgrow the texture, as `kd catalog check` will
    /// once looks can be merged (A11.3).
    pub fn new(cat: &Catalogue) -> Result<Layout, String> {
        let fixed = cat.colours.len();
        let mut at = fixed;
        let mut ladders = Vec::with_capacity(cat.looks.len());
        for look in &cat.looks {
            let steps = usize::from(look.steps);
            if at + steps > PALETTE_SIZE {
                return Err(format!(
                    "the palette needs more than {PALETTE_SIZE} entries at look {}",
                    look.id
                ));
            }
            ladders.push(Ladder {
                base: at as u8,
                steps: look.steps,
            });
            at += steps;
        }
        Ok(Layout {
            fixed,
            ladders,
            len: at,
        })
    }

    /// The look and step an index holds, if it holds one.
    pub fn step_of(&self, index: usize) -> Option<(usize, usize)> {
        self.ladders.iter().enumerate().find_map(|(j, l)| {
            let base = usize::from(l.base);
            (base..base + usize::from(l.steps))
                .contains(&index)
                .then_some((j, index - base))
        })
    }
}

/// The ends of every ladder's path as the light's lightness (the cube root of its luminance): deep shade and full
/// sun. The shaders take them as uniforms and pick a step from where a pixel's lightness falls between them.
pub fn range(light: &Light) -> [f32; 2] {
    let y_sky = dot(LUM, light.sky);
    let y_sun = dot(LUM, light.sun);
    [m::cbrt(DEEP_SHADE_SKY * y_sky), m::cbrt(y_sky + y_sun)]
}

/// The light along the path at lightness `l`: the sky's alone from deep shade to open shade, then the sky's and the
/// sun's facing it, so the luminance is exactly `l³`.
pub fn path_light(light: &Light, l: f32) -> Rgb {
    let y = l * l * l;
    let y_sky = dot(LUM, light.sky);
    let y_sun = dot(LUM, light.sun);
    if y <= y_sky || y_sun <= 0.0 {
        light.sky.map(|c| c * (y / y_sky))
    } else {
        let k = (y - y_sky) / y_sun;
        [0, 1, 2].map(|i| light.sky[i] + light.sun[i] * k)
    }
}

/// Step `k`'s lightness on a ladder of `steps` over `range`.
pub fn step_lightness(range: [f32; 2], k: usize, steps: usize) -> f32 {
    range[0] + (range[1] - range[0]) * k as f32 / (steps - 1) as f32
}

/// One light's palette: the colours, the path's range and the tables.
#[derive(Clone, Debug, PartialEq)]
pub struct Palette {
    /// Each entry's sRGB colour, `len` of them.
    pub row: Vec<[u8; 3]>,
    pub range: [f32; 2],
    /// `TABLE_ROWS` rows of `PALETTE_SIZE` indices.
    pub tables: Vec<[u8; PALETTE_SIZE]>,
}

impl Palette {
    /// The palette of `cat` under `light`, with the haze seen along `view` (east, north, up, from the eye into the
    /// scene; A11.4).
    pub fn new(cat: &Catalogue, layout: &Layout, light: &Light, view: [f32; 3]) -> Palette {
        let (row, radiance) = row(cat, layout, light);
        let tables = tables(&cat.air, layout, light, &row, &radiance, view);
        Palette {
            row,
            range: range(light),
            tables,
        }
    }

    /// The row as the palette texture's bytes: `PALETTE_SIZE` RGBA entries, unused ones as `void`.
    pub fn texture_bytes(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(PALETTE_SIZE * 4);
        for i in 0..PALETTE_SIZE {
            let [r, g, b] = self.row.get(i).copied().unwrap_or(self.row[0]);
            out.extend_from_slice(&[r, g, b, 255]);
        }
        out
    }

    /// The tables as their texture's bytes: `TABLE_ROWS` rows of `PALETTE_SIZE`.
    pub fn table_bytes(&self) -> Vec<u8> {
        self.tables.iter().flatten().copied().collect()
    }
}

/// An 8-bit sRGB colour in OKLab.
fn lab_of(c: [u8; 3]) -> [f32; 3] {
    to_oklab(c.map(|v| kd_data::schema::srgb_to_linear(f32::from(v) / 255.0)))
}

/// The palette row under `light`: the fixed colours, then each ladder's steps; and each step's albedo times its
/// light, which the haze tables mix toward the haze.
pub fn row(cat: &Catalogue, layout: &Layout, light: &Light) -> (Vec<[u8; 3]>, Vec<Rgb>) {
    let range = range(light);
    let mut row: Vec<[u8; 3]> = cat.colours.iter().map(|c| c.rgb).collect();
    let mut radiance = Vec::with_capacity(layout.len - layout.fixed);
    for (look, ladder) in cat.looks.iter().zip(&layout.ladders) {
        let albedo = look.albedo();
        let n = usize::from(ladder.steps);
        for k in 0..n {
            let e = path_light(light, step_lightness(range, k, n));
            row.push(shade(&cat.air, light, albedo, e));
            radiance.push([0, 1, 2].map(|i| albedo[i] * e[i]));
        }
    }
    (row, radiance)
}

/// The tables for a row (A11.3); `radiance` holds each look step's albedo times its light, in layout order.
pub fn tables(
    air: &Air,
    layout: &Layout,
    light: &Light,
    row: &[[u8; 3]],
    radiance: &[Rgb],
    view: [f32; 3],
) -> Vec<[u8; PALETTE_SIZE]> {
    let identity: [u8; PALETTE_SIZE] = std::array::from_fn(|i| i as u8);
    let mut t = vec![identity; TABLE_ROWS];
    let haze = haze_colour(air, light, view);
    let looks: Vec<(usize, [f32; 3])> = (layout.fixed..layout.len).map(|i| (i, lab_of(row[i]))).collect();
    // The nearest look colour to one in OKLab; the lower index wins a tie, so every target picks the same.
    let nearest = |c: [u8; 3]| {
        let lab = lab_of(c);
        let mut best = (f32::INFINITY, 0usize);
        for &(i, l) in &looks {
            let d = (0..3).map(|k| (l[k] - lab[k]) * (l[k] - lab[k])).sum::<f32>();
            if d < best.0 {
                best = (d, i);
            }
        }
        best.1 as u8
    };
    for ladder in &layout.ladders {
        let base = usize::from(ladder.base);
        let n = usize::from(ladder.steps);
        for k in 0..n {
            let i = base + k;
            t[table::OUTLINE][i] = (base + k.saturating_sub(2)) as u8;
            t[table::EDGE][i] = (base + n - 1) as u8;
            let r = radiance[i - layout.fixed];
            for (level, &mix) in air.haze_mixes.iter().enumerate() {
                let mixed = [0, 1, 2].map(|c| r[c] * (1.0 - mix) + haze[c] * mix);
                t[table::HAZE[level]][i] = nearest(shade(air, light, [1.0; 3], mixed));
            }
        }
    }
    t
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::light::light;
    use crate::light::tests::{air, sky_at};

    fn palette_at(hour: f32) -> (Catalogue, Layout, Light, Palette) {
        let cat = crate::tests::catalogue();
        let layout = Layout::new(&cat).unwrap();
        let l = light(&air(), &sky_at(hour));
        let p = Palette::new(&cat, &layout, &l, [0.0, 0.616, -0.788]);
        (cat, layout, l, p)
    }

    // checks: PRE-20 PRE-30
    #[test]
    fn steps_evenly_spaced() {
        for hour in [6.5, 12.0, 17.75, 23.0] {
            let (cat, layout, l, p) = palette_at(hour);
            for (look, ladder) in cat.looks.iter().zip(&layout.ladders) {
                let n = usize::from(ladder.steps);
                // The light's lightness at each step: evenly spaced, from deep shade to full sun.
                let ls: Vec<f32> = (0..n)
                    .map(|k| m::cbrt(dot(LUM, path_light(&l, step_lightness(p.range, k, n)))))
                    .collect();
                let gap = (p.range[1] - p.range[0]) / (n - 1) as f32;
                for w in ls.windows(2) {
                    assert!((w[1] - w[0] - gap).abs() < 0.01 * gap, "{} at {hour}: {ls:?}", look.id);
                }
                // As shown, each step lighter than the one below it.
                let shown: Vec<f32> = (0..n).map(|k| lab_of(p.row[usize::from(ladder.base) + k])[0]).collect();
                for w in shown.windows(2) {
                    assert!(w[1] > w[0], "{} at {hour}: {shown:?}", look.id);
                }
            }
        }
    }

    // checks: PRE-30
    #[test]
    fn shade_steps_bluer_than_lit() {
        // Whenever the sun is up, a ladder's lowest step, lit by the sky alone, has more blue for its red than its
        // top step in full sun: the cool shade and warm light a pixel artist paints.
        let blue_for_red = |c: [u8; 3]| {
            let lin = c.map(|v| kd_data::schema::srgb_to_linear(f32::from(v) / 255.0));
            lin[2] / lin[0]
        };
        for hour in [6.5, 9.0, 12.0, 15.0, 16.5, 17.75] {
            let (cat, layout, _, p) = palette_at(hour);
            for (look, ladder) in cat.looks.iter().zip(&layout.ladders) {
                let base = usize::from(ladder.base);
                let top = base + usize::from(ladder.steps) - 1;
                let (shade, lit) = (blue_for_red(p.row[base]), blue_for_red(p.row[top]));
                assert!(shade > lit, "{} at {hour}: shade {shade}, lit {lit}", look.id);
            }
        }
    }

    // checks: PRE-01
    #[test]
    fn tables_stay_in_the_palette() {
        for hour in [6.5, 12.0, 18.5, 23.0] {
            let (_, layout, _, p) = palette_at(hour);
            assert_eq!(p.tables.len(), TABLE_ROWS);
            for (r, t) in p.tables.iter().enumerate() {
                for (i, &to) in t.iter().enumerate().take(layout.len) {
                    let to = usize::from(to);
                    assert!(to < layout.len, "row {r} sends {i} to {to}");
                    // A look's colour stays a look's colour; void and the fixed colours stay themselves.
                    if i < layout.fixed {
                        assert_eq!(to, i, "row {r}");
                    } else {
                        assert!(to >= layout.fixed, "row {r} sends {i} to fixed colour {to}");
                    }
                }
            }
            // Haze moves colours, more at each level, toward the haze; level 3 leaves no ladder untouched.
            let moved = |level: usize| {
                (layout.fixed..layout.len)
                    .filter(|&i| p.tables[table::HAZE[level]][i] as usize != i)
                    .count()
            };
            assert!(
                moved(2) > 0 && moved(2) >= moved(0),
                "haze levels move {} then {}",
                moved(0),
                moved(2)
            );
        }
    }

    // checks: PRE-20
    #[test]
    fn outline_and_edge_in_own_look() {
        let (_, layout, _, p) = palette_at(16.5);
        for ladder in &layout.ladders {
            let (base, n) = (usize::from(ladder.base), usize::from(ladder.steps));
            for k in 0..n {
                assert_eq!(
                    usize::from(p.tables[table::OUTLINE][base + k]),
                    base + k.saturating_sub(2)
                );
                assert_eq!(usize::from(p.tables[table::EDGE][base + k]), base + n - 1);
                assert_eq!(layout.step_of(base + k).map(|s| s.1), Some(k));
            }
        }
    }

    // checks: PRE-01 PRE-20
    #[test]
    fn palette_fits() {
        let (cat, layout, _, p) = palette_at(12.0);
        // void at 0, then the fixed colours, then every look's steps, all within the texture.
        assert_eq!(cat.colours[0].id, "void");
        assert_eq!(p.row[0], cat.colours[0].rgb);
        assert_eq!(layout.fixed, cat.colours.len());
        assert_eq!(
            layout.len,
            layout.fixed + cat.looks.iter().map(|l| usize::from(l.steps)).sum::<usize>()
        );
        assert_eq!(p.row.len(), layout.len);
        assert!(layout.len <= PALETTE_SIZE);
        assert_eq!(p.texture_bytes().len(), PALETTE_SIZE * 4);
        assert_eq!(p.table_bytes().len(), PALETTE_SIZE * TABLE_ROWS);
        // Looks that outgrow the texture fail.
        let mut big = cat.clone();
        for _ in 0..40 {
            let mut l = big.looks[0].clone();
            l.steps = 7;
            big.looks.push(l);
        }
        assert!(Layout::new(&big).unwrap_err().contains("more than 256"));
    }
}
