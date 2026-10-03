//! The palette, its ladders and its index tables (A11.3), ported from the mockup's `PAL` in `f64` as its JavaScript
//! computes: `toLab`, `fromLab`, `nearest`, `table` and the versions of `variant`, built from the compiled catalogue
//! into the three textures the shaders read.
//! Implements `PRE-01`, `PRE-20`, `PRE-21` and `PRE-30` in part, see A11.3.

use kd_data::Catalogue;
use kd_data::blob::LightRec;
use kd_data::schema::{LightMethod, LightRole};

/// Ladder texture rows (the mockup's `RAMP_ROWS`, 64 there): A11.3 allows up to 512 ladders.
pub const RAMP_ROWS: usize = 512;
/// Table texture rows.
pub const TABLE_ROWS: usize = 16;
/// Palette texture rows: dusk, dawn, day, night.
pub const VERSION_ROWS: usize = 4;

fn clamp(v: f64, a: f64, b: f64) -> f64 {
    if v < a {
        a
    } else if v > b {
        b
    } else {
        v
    }
}

fn s2l(c: f64) -> f64 {
    let c = c / 255.0;
    if c <= 0.04045 {
        c / 12.92
    } else {
        ((c + 0.055) / 1.055).powf(2.4)
    }
}

fn l2s(c: f64) -> u8 {
    let v = if c <= 0.0031308 {
        c * 12.92
    } else {
        1.055 * c.powf(1.0 / 2.4) - 0.055
    };
    // Math.round, then clamp: negative ties round differently in Rust, but clamp to 0 either way.
    clamp((v * 255.0).round(), 0.0, 255.0) as u8
}

/// sRGB (0–255, not necessarily whole) to OKLab (the mockup's `toLab`).
pub fn to_lab(c: [f64; 3]) -> [f64; 3] {
    let (lr, lg, lb) = (s2l(c[0]), s2l(c[1]), s2l(c[2]));
    let l = (0.4122214708 * lr + 0.5363325363 * lg + 0.0514459929 * lb).cbrt();
    let m = (0.2119034982 * lr + 0.6806995451 * lg + 0.1073969566 * lb).cbrt();
    let s = (0.0883024619 * lr + 0.2817188376 * lg + 0.6299787005 * lb).cbrt();
    [
        0.2104542553 * l + 0.793617785 * m - 0.0040720468 * s,
        1.9779984951 * l - 2.428592205 * m + 0.4505937099 * s,
        0.0259040371 * l + 0.7827717662 * m - 0.808675766 * s,
    ]
}

/// OKLab to whole sRGB channels (the mockup's `fromLab`).
pub fn from_lab(lab: [f64; 3]) -> [u8; 3] {
    let [ll, a, b] = lab;
    let l = (ll + 0.3963377774 * a + 0.2158037573 * b).powf(3.0);
    let m = (ll - 0.1055613458 * a - 0.0638541728 * b).powf(3.0);
    let s = (ll - 0.0894841775 * a - 1.291485548 * b).powf(3.0);
    [
        l2s(4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s),
        l2s(-1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s),
        l2s(-0.0041960863 * l - 0.7034186147 * m + 1.707614701 * s),
    ]
}

fn num(s: &str) -> f64 {
    s.parse().unwrap_or(0.0)
}

fn rgbf(c: [u8; 3]) -> [f64; 3] {
    c.map(f64::from)
}

/// The palette's colours and their OKLab values, from the catalogue.
pub struct Palette {
    pub rgb: Vec<[u8; 3]>,
    lab: Vec<[f64; 3]>,
}

impl Palette {
    pub fn new(cat: &Catalogue) -> Palette {
        let rgb: Vec<[u8; 3]> = cat.body.colours.iter().map(|c| c.rgb).collect();
        let lab = rgb.iter().map(|c| to_lab(rgbf(*c))).collect();
        Palette { rgb, lab }
    }

    /// The allowed colour nearest `c` in OKLab, chroma weighted 1.3, ties to the lower index, never `void` (the
    /// mockup's `nearest`).
    pub fn nearest(&self, c: [f64; 3], allow: impl Fn(usize) -> bool) -> u8 {
        let l = to_lab([
            clamp(c[0], 0.0, 255.0),
            clamp(c[1], 0.0, 255.0),
            clamp(c[2], 0.0, 255.0),
        ]);
        let (mut best, mut bd) = (1usize, 1e9);
        for (k, p) in self.lab.iter().enumerate().skip(1) {
            if !allow(k) {
                continue;
            }
            let d = (p[0] - l[0]).powi(2) + 1.3 * ((p[1] - l[1]).powi(2) + (p[2] - l[2]).powi(2));
            if d < bd {
                bd = d;
                best = k;
            }
        }
        best as u8
    }

    /// A light entry's change to one colour, as the mockup writes it; Lab results are whole channels already.
    fn change(&self, r: &LightRec, k: usize) -> [f64; 3] {
        let c = rgbf(self.rgb[k]);
        match r.method {
            LightMethod::Same => c,
            LightMethod::Warm => {
                let k = num(&r.k);
                [
                    c[0] * (1.0 + 0.1 * k) + 13.0 * k,
                    c[1] * (1.0 + 0.035 * k) + 4.5 * k,
                    c[2] * (1.0 - 0.12 * k),
                ]
            }
            LightMethod::Haze => {
                let a = num(&r.amount);
                [0, 1, 2].map(|i| c[i] + (f64::from(r.toward[i]) - c[i]) * a)
            }
            LightMethod::Lab => {
                let lab = to_lab(c);
                let m = [num(&r.mul[0]), num(&r.mul[1]), num(&r.mul[2])];
                let a = [num(&r.add[0]), num(&r.add[1]), num(&r.add[2])];
                let mut l = lab[0] * m[0] + a[0];
                if let Some(lo) = &r.l_min
                    && l < num(lo)
                {
                    l = num(lo);
                }
                if let Some(hi) = &r.l_max
                    && l > num(hi)
                {
                    l = num(hi);
                }
                rgbf(from_lab([l, lab[1] * m[1] + a[1], lab[2] * m[2] + a[2]]))
            }
        }
    }

    /// An index table (the mockup's `table`): every index to itself, then each colour to the nearest colour the
    /// entry allows after its change.
    pub fn table(&self, r: &LightRec) -> [u8; 256] {
        let mut t = [0u8; 256];
        for (k, v) in t.iter_mut().enumerate() {
            *v = k as u8;
        }
        for (k, v) in t.iter_mut().enumerate().take(self.rgb.len()).skip(1) {
            *v = self.nearest(self.change(r, k), |j| r.mask.get(j).copied().unwrap_or(false));
        }
        t
    }

    /// A palette version (the mockup's `variant`): every colour changed, those kept left as they are.
    pub fn version(&self, r: &LightRec) -> Vec<[u8; 3]> {
        (0..self.rgb.len())
            .map(|k| {
                if r.mask.get(k).copied().unwrap_or(false) || r.method == LightMethod::Same {
                    self.rgb[k]
                } else if r.method == LightMethod::Lab {
                    // Versions start from the stored OKLab value (the mockup's `lab[k]`), which equals to_lab(rgb).
                    let c = self.change(r, k);
                    [c[0] as u8, c[1] as u8, c[2] as u8]
                } else {
                    let c = self.change(r, k);
                    c.map(|x| clamp(x.round(), 0.0, 255.0) as u8)
                }
            })
            .collect()
    }
}

/// The three textures of A11.3, as RGBA8 bytes, row by row from the bottom row 0.
pub struct PaletteTextures {
    /// 256 × 4: rows dusk, dawn, day, night; RGB the colour, A 255 for each palette colour.
    pub palette: Vec<u8>,
    /// 8 × 512: R the colour index of step i (the last step repeated), G the ladder's length, A 255.
    pub ladders: Vec<u8>,
    /// 256 × 16: R the table's index for each index, A 255; rows in the catalogue's table order.
    pub tables: Vec<u8>,
}

impl PaletteTextures {
    /// Builds the textures from the catalogue (the mockup's `textures()`).
    pub fn build(cat: &Catalogue) -> PaletteTextures {
        let pal = Palette::new(cat);
        let mut palette = vec![0u8; 256 * VERSION_ROWS * 4];
        for (row, r) in cat.light_rows(LightRole::Version).take(VERSION_ROWS).enumerate() {
            for (k, c) in pal.version(r).iter().enumerate() {
                palette[(row * 256 + k) * 4..][..4].copy_from_slice(&[c[0], c[1], c[2], 255]);
            }
        }
        let mut ladders = vec![0u8; 8 * RAMP_ROWS * 4];
        for (row, l) in cat.body.ladders.iter().take(RAMP_ROWS).enumerate() {
            let n = l.steps.len();
            for i in 0..8 {
                let o = (row * 8 + i) * 4;
                ladders[o] = l.steps.get(i.min(n.saturating_sub(1))).copied().unwrap_or(0);
                ladders[o + 1] = n as u8;
                ladders[o + 3] = 255;
            }
        }
        let mut tables = vec![0u8; 256 * TABLE_ROWS * 4];
        for (row, r) in cat.light_rows(LightRole::Table).take(TABLE_ROWS).enumerate() {
            for (k, v) in pal.table(r).iter().enumerate() {
                let o = (row * 256 + k) * 4;
                tables[o] = *v;
                tables[o + 3] = 255;
            }
        }
        PaletteTextures {
            palette,
            ladders,
            tables,
        }
    }
}

/// The shader lines naming each ladder's row and each table's row (the mockup's `SH.rampDefs`): `#define R_BIRCH 14.0`,
/// `#define L_HAZE1 3.0`, `#define RAMP_ROWS 512.0`, and `#define I_INK` for the post pass's figure outline. Names are
/// the ids upper-cased without underscores (`grass_damp` gives `R_GRASSDAMP`), as the mockup's shaders spell them.
pub fn shader_defines(cat: &Catalogue) -> String {
    let mut s = String::new();
    for (k, l) in cat.body.ladders.iter().enumerate() {
        s.push_str(&format!("#define R_{} {k}.0\n", define_name(&l.id)));
    }
    for (k, t) in cat.light_rows(LightRole::Table).enumerate() {
        s.push_str(&format!("#define L_{} {k}.0\n", define_name(&t.id)));
    }
    s.push_str(&format!("#define RAMP_ROWS {RAMP_ROWS}.0\n"));
    s.push_str(&format!(
        "#define I_INK {}.0\n",
        cat.colour("ink").map(|c| c.0).unwrap_or(1)
    ));
    s
}

fn define_name(id: &str) -> String {
    id.replace('_', "").to_uppercase()
}

#[cfg(test)]
mod tests {
    use super::*;

    fn catalogue() -> Catalogue {
        Catalogue::load(&compiled()).unwrap()
    }

    fn compiled() -> Vec<u8> {
        let root = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../data");
        kd_data::compile::compile(&root, false).unwrap().blob
    }

    // checks: PRE-20
    #[test]
    fn ladders_are_palette_indices() {
        let cat = catalogue();
        let tx = PaletteTextures::build(&cat);
        assert_eq!(cat.body.ladders.len(), 51);
        for (row, l) in cat.body.ladders.iter().enumerate() {
            assert!((2..=8).contains(&l.steps.len()), "{}", l.id);
            for i in 0..8 {
                let o = (row * 8 + i) * 4;
                let idx = usize::from(tx.ladders[o]);
                // `void` only as a first step: the dark around the globe and land at night on the map start there.
                assert!(
                    idx < cat.body.colours.len() && (idx > 0 || i == 0),
                    "{} step {i}: {idx}",
                    l.id
                );
                assert_eq!(usize::from(tx.ladders[o + 1]), l.steps.len());
            }
        }
        assert!(cat.ladder("birch").is_some());
    }

    // checks: PRE-30 PRE-21
    #[test]
    fn warming_never_flame() {
        let cat = catalogue();
        let pal = Palette::new(&cat);
        let fire = cat.family("fire").unwrap().0;
        for t in cat
            .light_rows(LightRole::Table)
            .filter(|t| t.method == LightMethod::Warm)
        {
            let table = pal.table(t);
            for (k, v) in table.iter().enumerate().take(pal.rgb.len()).skip(1) {
                let to = &cat.body.colours[usize::from(*v)];
                assert!(
                    to.family != fire || cat.body.colours[k].family == fire && *v as usize == k,
                    "{} maps {} into the flame colour {}",
                    t.id,
                    cat.body.colours[k].name,
                    to.name
                );
            }
        }
    }

    // checks: PRE-01
    #[test]
    fn tables_stay_in_the_palette() {
        let cat = catalogue();
        let tx = PaletteTextures::build(&cat);
        let n = cat.body.colours.len();
        for row in 0..12 {
            for k in 1..n {
                assert!(usize::from(tx.tables[(row * 256 + k) * 4]) < n);
            }
        }
        let defs = shader_defines(&cat);
        assert!(
            defs.contains("#define R_BIRCH 14.0\n")
                && defs.contains("#define L_RIMSUN 7.0\n")
                && defs.contains("#define L_HAZE1 3.0\n"),
            "{defs}"
        );
        assert!(defs.contains("#define I_INK 1.0\n") && defs.contains("#define R_GRASSDAMP 1.0\n"));
    }
}
