//! Coarse ground (A5.5): a cell's ground for the picture beyond the view areas, 33 × 33 points every 32 m from its
//! north-west corner, its edges shared with its neighbours'. Each point's height is the cells' bicubic blend
//! (Catmull–Rom over the 4 × 4 cells round it) with every escarpment's step put back sharp; each has a material, the
//! rock at an escarpment's rim and scree at its foot, soil elsewhere; the sea's surface lies over every point below
//! the sea's level; and the river's and the streams' lines are draped over the ground, which carves no channel at
//! this size, with each line's bankfull width and depth.
//!
//! Implements PRE-03 and WLD-12, see A5.5 and A11.5: the land beyond the view areas, shaped every few tens of metres,
//! made from the cells in a moment.

use kd_core::geo::{CELLS_X, CELLS_Y, CellIx, H, Pos, TICKS_PER_M, W, wrap};
use kd_core::{m, num};
use kd_data::Catalogue;

use super::{CELL_TICKS, NONE, WorldCells, water};
use crate::area::Material;

/// Points along a side of a cell's coarse ground.
pub const COARSE_SIDE: usize = 33;
/// Metres between the points.
pub const COARSE_M: i32 = 32;
/// The softness from which a rock shows as soft rock (A5.7's scale, 0.5 to 2).
const SOFT_FROM: f32 = 1.0;

/// What a cell's picture functions read (A5.5): the seed, the cells and the catalogue.
#[derive(Clone, Copy)]
pub struct CellCtx<'a> {
    pub seed: u64,
    pub cells: &'a WorldCells,
    pub cat: &'a Catalogue,
}

/// A line of fresh water over the coarse ground (A11.6): a river's or a stream's, its points in ticks, at most
/// `COARSE_M` apart, its surface's height at each, metres above the sea, and its bankfull width and depth and the
/// area it drains.
#[derive(Clone, Debug, PartialEq)]
pub struct WaterLine {
    pub river: bool,
    pub points: Vec<[i32; 2]>,
    pub z_m: Vec<f32>,
    pub width_m: f32,
    pub depth_m: f32,
    pub drainage_km2: f32,
}

/// A cell's coarse ground (A5.5).
#[derive(Clone, Debug, PartialEq)]
pub struct CoarseGround {
    pub cell: CellIx,
    /// `COARSE_SIDE` × `COARSE_SIDE` heights, row by row from the north-west corner, metres above the sea.
    pub heights: Vec<f32>,
    /// A material at each point.
    pub material: Vec<Material>,
    /// The water's surface over each point, metres above the sea, if any: the sea's, over the points below it.
    pub water: Vec<Option<f32>>,
    /// The river's line through the cell, and a stream's from the cell's middle to the cell it drains into.
    pub lines: Vec<WaterLine>,
}

/// Catmull–Rom's weights for the cells at −1, 0, 1 and 2 of a point a share `t` of the way from cell 0 to cell 1.
fn catmull_rom(t: f32) -> [f32; 4] {
    let (t2, t3) = (t * t, t * t * t);
    [
        0.5 * (-t3 + 2.0 * t2 - t),
        0.5 * (3.0 * t3 - 5.0 * t2 + 2.0),
        0.5 * (-3.0 * t3 + 4.0 * t2 + t),
        0.5 * (t3 - t2),
    ]
}

impl WorldCells {
    /// The coarse ground's height at a place (A5.5): the 4 × 4 cells round it blended by Catmull–Rom's weights between
    /// their middles, which the blend passes through, with every escarpment's step put back sharp; across the
    /// east–west wrap, the rows held at the poles.
    pub fn coarse_z(&self, p: Pos) -> f32 {
        let f = &self.fixed;
        let half = i64::from(CELL_TICKS / 2);
        let cell = i64::from(CELL_TICKS);
        let (ux, uy) = (i64::from(p.x) - half, i64::from(p.y) - half);
        let (cx, cy) = (ux.div_euclid(cell), uy.div_euclid(cell));
        // Below 2^18, so exact in a float.
        let wx = catmull_rom(ux.rem_euclid(cell) as f32 / CELL_TICKS as f32);
        let wy = catmull_rom(uy.rem_euclid(cell) as f32 / CELL_TICKS as f32);
        let mut blend = [(CellIx(0), 0.0f32); 16];
        let mut z = 0.0;
        for (j, &v) in wy.iter().enumerate() {
            let row = (cy + j as i64 - 1).clamp(0, i64::from(CELLS_Y) - 1) as u32;
            for (i, &u) in wx.iter().enumerate() {
                let col = (cx + i as i64 - 1).rem_euclid(i64::from(CELLS_X)) as u32;
                let c = CellIx::at(col, row);
                let w = u * v;
                z += w * f32::from(f.height[c.0 as usize]);
                blend[j * 4 + i] = (c, w);
            }
        }
        z + self.step_correction(p, &blend)
    }
}

/// The point (`i`, `j`) of a cell's coarse ground, `i` east and `j` south of its north-west corner, on the world.
pub fn coarse_point(c: CellIx, i: usize, j: usize) -> Pos {
    let o = c.origin();
    let step = COARSE_M * TICKS_PER_M;
    Pos {
        x: (o.x + i as i32 * step).rem_euclid(W),
        y: (o.y + j as i32 * step).min(H - 1),
        z: 0,
    }
}

/// A point's material: an escarpment's rim, the first `COARSE_M` of its high side along its face, shows the rock
/// of the cell it lies in, soft or hard by the rock's softness; its foot, the first `COARSE_M` below, scree; a gap
/// and anywhere else, soil.
fn material_at(cx: &CellCtx, p: Pos) -> Material {
    let f = &cx.cells.fixed;
    let band = COARSE_M * TICKS_PER_M;
    for e in &f.escarpments {
        let Some(d) = e.face_across([p.x, p.y]) else {
            continue;
        };
        if d > 0 && d <= band {
            let top = f.rock[CellIx::of(p).0 as usize][0];
            let soft = top != NONE
                && cx
                    .cat
                    .rocks
                    .iter()
                    .find(|r| r.number == u16::from(top))
                    .is_some_and(|r| r.softness >= SOFT_FROM);
            return if soft { Material::SoftRock } else { Material::HardRock };
        }
        if d <= 0 && d > -band {
            return Material::Scree;
        }
    }
    Material::Soil
}

/// The river's rating, from the world's tuned numbers and the river's mouth: streams are scaled from it by the area
/// they drain (A5.7 step 5).
struct Rating {
    width_m: f32,
    depth_m: f32,
    drainage_km2: f32,
    width_exp: f32,
    depth_exp: f32,
}

impl Rating {
    fn new(cx: &CellCtx) -> Option<Rating> {
        let f = &cx.cells.fixed;
        let (last, mouth) = (f.rivers.last()?, f.stretches.last()?);
        let world = cx.cat.tuning("world").ok()?;
        Some(Rating {
            width_m: last.width_m,
            depth_m: last.depth_m,
            drainage_km2: num::max(mouth.drainage_km2, 1.0),
            width_exp: world.number("river_rating.width_exp").ok()?,
            depth_exp: world.number("river_rating.depth_exp").ok()?,
        })
    }

    /// The bankfull width and depth of water draining `km2`.
    fn shape(&self, km2: f32) -> (f32, f32) {
        let share = km2 / self.drainage_km2;
        (
            self.width_m * m::powf(share, self.width_exp),
            self.depth_m * m::powf(share, self.depth_exp),
        )
    }
}

/// A line from `a` to `b` in ticks, cut into pieces at most `COARSE_M` long, draped at the higher of the water's
/// level and the coarse ground (`level` none for a stream, which runs on its ground).
fn draped(w: &WorldCells, a: [i32; 2], b: [i32; 2], level: Option<f32>) -> (Vec<[i32; 2]>, Vec<f32>) {
    let (dx, dy) = (i64::from(b[0]) - i64::from(a[0]), i64::from(b[1]) - i64::from(a[1]));
    let len = ((dx * dx + dy * dy) as f32).sqrt();
    let pieces = num::max(1.0, (len / (COARSE_M * TICKS_PER_M) as f32).ceil()) as i64;
    let mut points = Vec::with_capacity(pieces as usize + 1);
    let mut z_m = Vec::with_capacity(pieces as usize + 1);
    for k in 0..=pieces {
        let q = [
            (i64::from(a[0]) + dx * k / pieces) as i32,
            (i64::from(a[1]) + dy * k / pieces) as i32,
        ];
        let ground = w.coarse_z(Pos {
            x: q[0].rem_euclid(W),
            y: q[1].clamp(0, H - 1),
            z: 0,
        });
        points.push(q);
        z_m.push(level.map_or(ground, |l| num::max(l, ground)));
    }
    (points, z_m)
}

/// The nearest point to `p` on the segment from `a` to `b`, all in ticks.
fn nearest_on(p: [i32; 2], a: [i32; 2], b: [i32; 2]) -> [i32; 2] {
    let d = [i64::from(b[0]) - i64::from(a[0]), i64::from(b[1]) - i64::from(a[1])];
    let v = [i64::from(p[0]) - i64::from(a[0]), i64::from(p[1]) - i64::from(a[1])];
    let len2 = d[0] * d[0] + d[1] * d[1];
    if len2 == 0 {
        return a;
    }
    let t = (v[0] * d[0] + v[1] * d[1]).clamp(0, len2);
    [
        (i64::from(a[0]) + d[0] * t / len2) as i32,
        (i64::from(a[1]) + d[1] * t / len2) as i32,
    ]
}

/// A cell's water lines: its stretch of the river, entry to exit at the water's level; or, for a stream cell, a line
/// from its middle to the cell it drains into, ending on the river's line where that cell holds the river.
fn lines_of(cx: &CellCtx, c: CellIx, rating: Option<&Rating>) -> Vec<WaterLine> {
    let f = &cx.cells.fixed;
    let i = c.0 as usize;
    let mut out = Vec::new();
    if f.water[i] & water::RIVER != 0
        && let Some(r) = f.rivers.get(f.river[i] as usize)
    {
        let (points, z_m) = draped(cx.cells, r.entry, r.exit, Some(r.level_m));
        let drainage_km2 = f.stretches.get(r.stretch as usize).map_or(0.0, |s| s.drainage_km2);
        out.push(WaterLine {
            river: true,
            points,
            z_m,
            width_m: r.width_m,
            depth_m: r.depth_m,
            drainage_km2,
        });
    }
    if f.water[i] & water::STREAM != 0 && f.flow[i] != NONE {
        let to = c.neighbours8()[usize::from(f.flow[i] & 7)];
        let mid = c.centre();
        let from = [mid.x, mid.y];
        // The receiver's middle, the short way round the world.
        let there = to.centre();
        let mut end = [mid.x + wrap(there.x - mid.x, W), there.y];
        let j = to.0 as usize;
        if f.water[j] & water::RIVER != 0
            && let Some(r) = f.rivers.get(f.river[j] as usize)
        {
            let shift = end[0] - there.x;
            let near = nearest_on([there.x, there.y], r.entry, r.exit);
            end = [near[0] + shift, near[1]];
        }
        let km2 = f.river[i] as f32;
        let (width_m, depth_m) = rating.map_or((0.0, 0.0), |r| r.shape(km2));
        let (points, z_m) = draped(cx.cells, from, end, None);
        out.push(WaterLine {
            river: false,
            points,
            z_m,
            width_m,
            depth_m,
            drainage_km2: km2,
        });
    }
    out
}

/// A cell's coarse ground (A5.5).
pub fn coarse_ground(cx: &CellCtx, c: CellIx) -> CoarseGround {
    let f = &cx.cells.fixed;
    let n = COARSE_SIDE * COARSE_SIDE;
    let (mut heights, mut material, mut sea) = (Vec::with_capacity(n), Vec::with_capacity(n), Vec::with_capacity(n));
    for j in 0..COARSE_SIDE {
        for i in 0..COARSE_SIDE {
            let p = coarse_point(c, i, j);
            let h = cx.cells.coarse_z(p);
            heights.push(h);
            material.push(material_at(cx, p));
            sea.push((h < 0.0 && !f.is_void(CellIx::of(p))).then_some(0.0));
        }
    }
    let rating = Rating::new(cx);
    CoarseGround {
        cell: c,
        heights,
        material,
        water: sea,
        lines: lines_of(cx, c, rating.as_ref()),
    }
}

#[cfg(test)]
mod tests {
    use std::sync::Arc;

    use super::*;
    use crate::cells::{CellState, EscarpmentLine, FixedCells, RiverCell, Stretch, TO_SEA};
    use kd_data::world::Side;

    fn catalogue() -> Catalogue {
        kd_data::compile::compile(&kd_data::repo_sources!(), false)
            .expect("the catalogue compiles")
            .catalogue
    }

    /// Cells round (1000, 244), rolling and falling south to a sea from row 250, an escarpment on row 236's middle
    /// facing south, 30 m high from column 990 to 1010 with a gap at column 995's middle, chalk on top; a river
    /// down column 1003 from row 240 to the sea, and a stream in (1001, 245) draining into it.
    fn world(cat: &Catalogue) -> WorldCells {
        let mut f = FixedCells::void();
        let chalk = cat
            .rocks
            .iter()
            .find(|r| r.softness >= SOFT_FROM)
            .expect("a soft rock")
            .number as u8;
        for cy in 226..256 {
            for cx in 980..1020 {
                let c = CellIx::at(cx, cy);
                let i = c.0 as usize;
                let rolling = 3.0 * ((cx * 7 + cy * 3) % 5) as f32;
                let fall = 120.0 - 12.0 * (cy as f32 - 236.0);
                let share = match cy.cmp(&236) {
                    std::cmp::Ordering::Less => 1.0,
                    std::cmp::Ordering::Equal => 0.5,
                    std::cmp::Ordering::Greater => 0.0,
                };
                if cy >= 250 {
                    f.height[i] = -20 - 10 * (cy as i16 - 250);
                    f.water[i] = water::SEA;
                } else {
                    f.height[i] = (fall + rolling + 30.0 * share).round() as i16;
                    f.water[i] = 0;
                    f.rock[i] = [chalk, NONE, NONE];
                }
            }
        }
        let x_of = |cx: u32| CellIx::at(cx, 236).centre().x;
        f.escarpments.push(EscarpmentLine {
            facing: Side::South,
            at: CellIx::at(1000, 236).centre().y,
            span: [CellIx::at(990, 236).origin().x, CellIx::at(1010, 236).origin().x],
            breaks: Vec::new(),
            gaps: vec![x_of(995)],
            height_m: 30.0,
        });
        // The river, entering each cell on its north edge and leaving on its south one, 2 m below its ground.
        for cy in 240..250u32 {
            let c = CellIx::at(1003, cy);
            let (o, mid) = (c.origin(), c.centre());
            f.water[c.0 as usize] |= water::RIVER;
            f.river[c.0 as usize] = f.rivers.len() as u32;
            f.rivers.push(RiverCell {
                cell: c,
                stretch: 0,
                entry: [mid.x, o.y],
                exit: [mid.x, o.y + CELL_TICKS],
                level_m: 60.0 - 12.0 * (cy as f32 - 240.0),
                width_m: 18.0,
                depth_m: 1.2,
            });
        }
        f.stretches.push(Stretch {
            first: 0,
            cells: 10,
            length_m: 10_240.0,
            flow_m3s: 8.0,
            drainage_km2: 900.0,
            down: TO_SEA,
        });
        // A stream draining 100 km² east into the river.
        let s = CellIx::at(1002, 245);
        f.water[s.0 as usize] |= water::STREAM;
        f.river[s.0 as usize] = 100;
        f.flow[s.0 as usize] = 0;
        WorldCells {
            fixed: Arc::new(f),
            state: CellState::new(),
        }
    }

    // checks: PRE-03 WLD-12
    #[test]
    fn neighbours_share_their_edges() {
        let cat = catalogue();
        let w = world(&cat);
        let cx = CellCtx {
            seed: 1,
            cells: &w,
            cat: &cat,
        };
        let n = COARSE_SIDE;
        for (cx_, cy) in [(1000, 240), (1004, 236), (995, 236), (1002, 249)] {
            let c = coarse_ground(&cx, CellIx::at(cx_, cy));
            let east = coarse_ground(&cx, CellIx::at(cx_ + 1, cy));
            let south = coarse_ground(&cx, CellIx::at(cx_, cy + 1));
            for k in 0..n {
                // Bit for bit, so a picture of neighbouring cells never shows a seam.
                assert_eq!(
                    c.heights[k * n + n - 1].to_bits(),
                    east.heights[k * n].to_bits(),
                    "{cx_} {cy} row {k}"
                );
                assert_eq!(
                    c.heights[(n - 1) * n + k].to_bits(),
                    south.heights[k].to_bits(),
                    "{cx_} {cy} col {k}"
                );
            }
            // The blend passes through the cell's middle, away from the escarpment.
            if cy != 236 {
                let mid = c.heights[16 * n + 16];
                assert_eq!(
                    mid,
                    f32::from(w.fixed.height[CellIx::at(cx_, cy).0 as usize]),
                    "{cx_} {cy}"
                );
            }
            // Every point is the world's own height there.
            for (k, &h) in c.heights.iter().enumerate() {
                assert_eq!(h, w.coarse_z(coarse_point(c.cell, k % n, k / n)));
            }
        }
    }

    // checks: PRE-03 WLD-34 PRE-23
    #[test]
    fn the_escarpment_stands_sharp() {
        let cat = catalogue();
        let w = world(&cat);
        let cx = CellCtx {
            seed: 1,
            cells: &w,
            cat: &cat,
        };
        let n = COARSE_SIDE;
        // On the face: the point on the line is the foot, 30 m below the rim 32 m north of it; the rim shows the
        // chalk as soft rock, the foot scree, the rest soil.
        let g = coarse_ground(&cx, CellIx::at(1004, 236));
        for i in 0..n {
            let (rim, foot) = (g.heights[15 * n + i], g.heights[16 * n + i]);
            assert!((rim - foot - 30.0).abs() < 3.5, "column {i}: {rim} {foot}");
            assert_eq!(g.material[15 * n + i], Material::SoftRock);
            assert_eq!(g.material[16 * n + i], Material::Scree);
            assert_eq!(g.material[10 * n + i], Material::Soil);
            assert_eq!(g.material[20 * n + i], Material::Soil);
            // Away from the face the ground runs smooth: no step between rows.
            for j in (0..14).chain(17..n - 1) {
                let step = (g.heights[j * n + i] - g.heights[(j + 1) * n + i]).abs();
                assert!(step < 3.0, "row {j}, column {i}: {step} m");
            }
        }
        // Through the gap, a slope people climb, of soil.
        let gap = coarse_ground(&cx, CellIx::at(995, 236));
        assert_eq!(gap.material[15 * n + 16], Material::Soil);
        let (rim, foot) = (gap.heights[15 * n + 16], gap.heights[16 * n + 16]);
        assert!(rim - foot < 25.0, "{rim} {foot}");
        // Beyond the face's ends, no rock.
        let beyond = coarse_ground(&cx, CellIx::at(985, 236));
        assert!(beyond.material.iter().all(|&m| m == Material::Soil));
    }

    // checks: PRE-26 WLD-17 PRE-03
    #[test]
    fn water_lies_over_the_sea_and_along_the_lines() {
        let cat = catalogue();
        let w = world(&cat);
        let cx = CellCtx {
            seed: 1,
            cells: &w,
            cat: &cat,
        };
        // The sea's surface over every point below it, none over the land or the void.
        for c in [CellIx::at(1000, 249), CellIx::at(1000, 250), CellIx::at(1000, 240)] {
            let g = coarse_ground(&cx, c);
            for (h, s) in g.heights.iter().zip(&g.water) {
                assert_eq!(*s, (*h < 0.0).then_some(0.0), "{c:?}");
            }
        }
        let far = coarse_ground(&cx, CellIx::at(900, 100));
        assert!(far.water.iter().all(Option::is_none));
        // The river's line runs from its entry to its exit in pieces of at most 32 m, never below its level.
        let g = coarse_ground(&cx, CellIx::at(1003, 242));
        let r = &w.fixed.rivers[2];
        assert_eq!(g.lines.len(), 1);
        let line = &g.lines[0];
        assert_eq!((line.points[0], *line.points.last().unwrap()), (r.entry, r.exit));
        assert!(
            line.points
                .windows(2)
                .all(|p| (p[1][1] - p[0][1]).abs() <= COARSE_M * TICKS_PER_M)
        );
        assert!(line.z_m.iter().all(|&z| z >= r.level_m));
        assert_eq!((line.width_m, line.depth_m, line.drainage_km2), (18.0, 1.2, 900.0));
        assert!(line.river);
        // The stream runs from its cell's middle onto the river's line in the cell it drains into, as wide as the
        // river's rating gives for its 100 km².
        let s = coarse_ground(&cx, CellIx::at(1002, 245));
        assert_eq!(s.lines.len(), 1);
        let line = &s.lines[0];
        let (from, to) = (CellIx::at(1002, 245).centre(), w.fixed.rivers[5]);
        assert_eq!(line.points[0], [from.x, from.y]);
        assert_eq!(*line.points.last().unwrap(), [to.entry[0], from.y]);
        let share: f32 = 100.0 / 900.0;
        assert!((line.width_m - 18.0 * share.sqrt()).abs() < 0.01, "{}", line.width_m);
        assert!((line.depth_m - 1.2 * kd_core::m::powf(share, 0.4)).abs() < 0.01);
        assert_eq!(line.drainage_km2, 100.0);
        assert!(!line.river);
        // Cells with neither hold no line.
        assert!(coarse_ground(&cx, CellIx::at(1000, 240)).lines.is_empty());
    }
}
