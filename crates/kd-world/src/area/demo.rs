//! The demo area (α01b): a 256 m square of hilly ground with a 30 m escarpment of rock beds crossing it and scree at
//! its foot, standing in for made areas until α02b makes them from cells (A5.3). It is a fixture: its numbers are
//! the plan's (T01b.2), the relief's ±11 m by the owner's OK of 3 October 2026, and α02b's generator takes its own
//! from `data/tuning/world.md`.
//!
//! Implements WLD-12 in part, see A5.3: an area's ground to the metre, with a cliff.

use super::{Ground, Material, SIDE, SQUARES, relief};
use kd_core::chance::unit_of;
use kd_core::geo::{AREA_M, AREA_TICKS, AREAS_X, AreaId, TICKS_PER_M};
use kd_core::num;

/// Where it lies: 21° N, where the strip's hours are seen from (A11.4), 3 km west of the seam at longitude 0, so
/// the world's large coordinates and its wrap are met from the first area (A3.7, A11.2).
pub const AREA: AreaId = AreaId(1_533 * AREAS_X + 7_988);
/// The seed the app makes it with.
pub const SEED: u64 = 1;
/// Its ground's hash with `SEED` (`Ground::hash`), as x86-64 makes it: the tests on x86-64 and arm64, the phone's
/// self-check and the browser compare theirs with it (A15.9 item 5).
pub const HASH: u64 = 0x6543_a78b_34ec_a2ad;

/// Its corners' heights in metres above sea level: north-west, north-east, south-west, south-east.
const CORNERS_M: [f32; 4] = [310.0, 342.0, 365.0, 330.0];
/// The relief: 7 octaves from a 256 m period down to 4 m, reaching ±11 m (hilly, `rough` about 5).
const RELIEF_PERIOD_M: u32 = 256;
const RELIEF_OCTAVES: u32 = 7;
const RELIEF_M: f32 = 11.0;
/// The escarpment: 30 m high, the high side west, rising over 4 m across a line that wanders up to 150 m either
/// side of the area's middle by 4 octaves from a 512 m period.
const CLIFF_M: f32 = 30.0;
const CLIFF_WIDTH_M: f32 = 4.0;
const LINE_WANDER_M: f32 = 150.0;
const LINE_PERIOD_M: u32 = 512;
const LINE_OCTAVES: u32 = 4;
/// Rock beds: floors 3.75 m apart, each moved by up to 1.125 m either way, so beds are 1.5–6 m thick; a quarter
/// soft.
const BED_SPACING_M: f32 = 3.75;
const BED_JITTER_M: f32 = 2.25;
const SOFT_SHARE: f32 = 0.25;
/// Slopes as rises a metre: soil under tan 30°, the bed over tan 45°, bare dirt between.
pub const SOIL_UNDER: f32 = 0.577_350_3;
pub const BED_OVER: f32 = 1.0;
/// Scree lies within 6 m of the cliff's foot, on its low side.
const SCREE_M: f32 = 6.0;

/// Salts keeping the demo's keyed numbers apart.
const RELIEF_SALT: u64 = 1;
const LINE_SALT: u64 = 2;
const BED_SALT: u64 = 3;

/// Row `j`'s distance from the north pole, in ticks (half rows give squares' centres).
fn row_y(j2: i64) -> i64 {
    i64::from(AREA.origin().y) + j2 * i64::from(TICKS_PER_M) / 2
}

/// How far east of the area's west edge the escarpment's line runs at `y` ticks, in metres.
pub fn line_m(seed: u64, y: i64) -> f32 {
    let middle = i64::from(AREA.origin().x) + i64::from(AREA_TICKS / 2);
    let wander = relief::fbm(num::hash2(seed, LINE_SALT), middle, y, LINE_PERIOD_M, LINE_OCTAVES);
    AREA_M as f32 / 2.0 + LINE_WANDER_M * wander
}

/// The share of the cliff's height at `d` metres east of its line: all of it 2 m west, none 2 m east.
fn cliff_share(d: f32) -> f32 {
    num::max(0.0, num::min(1.0, (CLIFF_WIDTH_M / 2.0 - d) / CLIFF_WIDTH_M))
}

/// The rock bed at `z_m` metres above sea level, and whether it is soft: bed `k`'s floor lies at 3.75 k m, moved by
/// a draw keyed by `k`, its band, so beds are 1.5–6 m thick, and a quarter of them are soft.
pub fn bed_at(seed: u64, z_m: f32) -> (i64, bool) {
    let draw = |k: i64, which: u64| unit_of(num::hash2(num::hash2(seed, BED_SALT), ((k as u64) << 1) | which));
    let floor = |k: i64| BED_SPACING_M * k as f32 + BED_JITTER_M * (draw(k, 0) - 0.5);
    let mut k = (z_m / BED_SPACING_M).floor() as i64;
    // A floor moves less than half the spacing, so the bed is this band's, the one below or the one above.
    if z_m < floor(k) {
        k -= 1;
    } else if z_m >= floor(k + 1) {
        k += 1;
    }
    (k, draw(k, 1) < SOFT_SHARE)
}

/// Square (`i`, `j`)'s rise a metre, from its four corners' heights in metres.
pub fn slope(h00: f32, h10: f32, h01: f32, h11: f32) -> f32 {
    let gx = (h10 + h11 - h00 - h01) / 2.0;
    let gy = (h01 + h11 - h00 - h10) / 2.0;
    (gx * gx + gy * gy).sqrt()
}

/// The demo area made with `seed` (A5.3): the corners' blend, the relief and the escarpment at each point, rounded
/// to decimetres; then each square's material from its slope, the bed at its height and the cliff's foot.
pub fn make(seed: u64) -> Ground {
    let o = AREA.origin();
    let (ox, m) = (i64::from(o.x), i64::from(TICKS_PER_M));
    let relief_seed = num::hash2(seed, RELIEF_SALT);
    let line: Vec<f32> = (0..2 * SIDE as i64).map(|j2| line_m(seed, row_y(j2))).collect();
    let [nw, ne, sw, se] = CORNERS_M;
    let dm: Vec<i32> = (0..SIDE * SIDE)
        .map(|k| {
            let (i, j) = (k % SIDE, k / SIDE);
            let (u, v) = (i as f32 / SQUARES as f32, j as f32 / SQUARES as f32);
            let (north, south) = (nw + (ne - nw) * u, sw + (se - sw) * u);
            let base = north + (south - north) * v;
            let (x, y) = (ox + i as i64 * m, row_y(2 * j as i64));
            let detail = RELIEF_M * relief::fbm(relief_seed, x, y, RELIEF_PERIOD_M, RELIEF_OCTAVES);
            let rise = CLIFF_M * cliff_share(i as f32 - line[2 * j]);
            ((base + detail + rise) * 10.0).round() as i32
        })
        .collect();
    let base_dm = dm.iter().copied().min().expect("points");
    let h = |i: usize, j: usize| dm[j * SIDE + i] as f32 / 10.0;
    let material = (0..SQUARES * SQUARES)
        .map(|k| {
            let (i, j) = (k % SQUARES, k / SQUARES);
            let (h00, h10, h01, h11) = (h(i, j), h(i + 1, j), h(i, j + 1), h(i + 1, j + 1));
            let s = slope(h00, h10, h01, h11);
            let past_foot = i as f32 + 0.5 - line[2 * j + 1] - CLIFF_WIDTH_M / 2.0;
            let m = if s > BED_OVER {
                match bed_at(seed, (h00 + h10 + h01 + h11) / 4.0) {
                    (_, true) => Material::SoftRock,
                    (_, false) => Material::HardRock,
                }
            } else if past_foot > 0.0 && past_foot <= SCREE_M {
                Material::Scree
            } else if s < SOIL_UNDER {
                Material::Soil
            } else {
                Material::Dirt
            };
            m as u8
        })
        .collect();
    Ground {
        id: AREA,
        base_dm,
        heights: dm.iter().map(|&d| (d - base_dm) as u16).collect(),
        material,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// The line at a square's centre row, as `make` uses it.
    fn line_at_square(j: usize) -> f32 {
        line_m(SEED, row_y(2 * j as i64 + 1))
    }

    // checks: WLD-12
    #[test]
    fn escarpment_in_the_window() {
        let g = make(SEED);
        let rock = |i: usize, j: usize| matches!(g.material_at(i, j), Material::HardRock | Material::SoftRock);
        for j in 0..SIDE {
            // The line stays inside the window, 16 m clear of its sides, on every row.
            let x = line_m(SEED, row_y(2 * j as i64));
            assert!((16.0..240.0).contains(&x), "row {j}: the line at {x} m");
            // Across it, the ground 6 m west stands about 30 m above the ground 6 m east.
            let at = x.round() as usize;
            let step = g.height_m(at - 6, j) - g.height_m(at + 6, j);
            assert!((22.0..38.0).contains(&step), "row {j}: a step of {step} m");
        }
        // Rock crosses the window from its north edge to its south edge, every row of squares having some.
        for j in 0..SQUARES {
            assert!((0..SQUARES).any(|i| rock(i, j)), "no rock in row {j}");
        }
        // Heights start at 0 at the lowest point and stay well inside u16.
        assert_eq!(g.heights.iter().copied().min(), Some(0));
        assert!(g.heights.iter().all(|&h| h < 2_000));
        assert_eq!(g.heights.len(), SIDE * SIDE);
        assert_eq!(g.material.len(), SQUARES * SQUARES);
    }

    // checks: WLD-12
    #[test]
    fn materials_by_slope_bed_and_foot() {
        let g = make(SEED);
        let mut counts = [0u32; 5];
        for j in 0..SQUARES {
            for i in 0..SQUARES {
                let (h00, h10, h01, h11) = (
                    g.height_m(i, j),
                    g.height_m(i + 1, j),
                    g.height_m(i, j + 1),
                    g.height_m(i + 1, j + 1),
                );
                let s = slope(h00, h10, h01, h11);
                let past_foot = i as f32 + 0.5 - line_at_square(j) - CLIFF_WIDTH_M / 2.0;
                let under_cliff = past_foot > 0.0 && past_foot <= SCREE_M;
                let m = g.material_at(i, j);
                let at = format!("square ({i}, {j}), slope {s}, {past_foot} m past the foot: {m:?}");
                match m {
                    Material::HardRock | Material::SoftRock => {
                        assert!(s > BED_OVER, "{at}");
                        let soft = bed_at(SEED, (h00 + h10 + h01 + h11) / 4.0).1;
                        assert_eq!(soft, m == Material::SoftRock, "{at}");
                    }
                    Material::Scree => assert!(s <= BED_OVER && under_cliff, "{at}"),
                    Material::Soil => assert!(s < SOIL_UNDER && !under_cliff, "{at}"),
                    Material::Dirt => assert!((SOIL_UNDER..=BED_OVER).contains(&s) && !under_cliff, "{at}"),
                }
                counts[m as usize] += 1;
            }
        }
        // Every material shows, and soil covers most of the ground.
        assert!(counts.iter().all(|&c| c > 0), "{counts:?}");
        assert!(counts[0] > SQUARES as u32 * SQUARES as u32 / 2, "{counts:?}");
        // Beds from sea level to 1,000 m: each 1.5–6 m thick, about a quarter of them soft.
        let mut beds: Vec<(i64, bool, f32, f32)> = Vec::new();
        for c in 0..100_000 {
            let z = c as f32 / 100.0;
            let (k, soft) = bed_at(SEED, z);
            match beds.last_mut() {
                Some(b) if b.0 == k => b.3 = z,
                _ => beds.push((k, soft, z, z)),
            }
        }
        assert!(beds.windows(2).all(|w| w[1].0 == w[0].0 + 1), "beds in order");
        for b in &beds[1..beds.len() - 1] {
            let thick = b.3 - b.2 + 0.01;
            assert!((1.49..=6.01).contains(&thick), "bed {} is {thick} m", b.0);
        }
        let soft = beds.iter().filter(|b| b.1).count() as f32 / beds.len() as f32;
        assert!((0.18..0.32).contains(&soft), "{soft} of beds soft");
    }

    // checks: WLD-12 RES-05
    #[test]
    fn golden_demo_hash() {
        // The demo's heights and materials hash to the value stored on x86-64, here and on arm64 under qemu
        // (tools/check.sh step 6); another seed makes other ground.
        let g = make(SEED);
        assert_eq!(g.hash(), HASH, "the demo area's ground differs from x86-64's");
        assert_ne!(make(SEED + 1).hash(), g.hash());
    }
}
