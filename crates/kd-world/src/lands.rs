//! Land presets (A5.6, `WLD-34`): an island and its sea made from a preset's few numbers and `data/tuning/lands.md`,
//! into the world's cells, by the same columns and side tables generation fills (A5.2), so nothing is thrown away
//! when the generator arrives. The preset fills a square of cells round its centre; beyond its sea every cell
//! stays void. Every draw is keyed on a place's uid at moment 0 (system 1, `purposes`), so the same seed makes the
//! same island on every target.
//!
//! The land, its coast and its hills are smooth functions of the place, worked out in metres from the island's
//! middle; the river's line and the escarpment's are drawn first, so each cell's height takes the valley and the
//! step as it is set.
//!
//! Implements WLD-34, see A5.6: the first region set from a few numbers, an island in a sea nobody crosses.

use kd_core::chance::{Stream, moment};
use kd_core::geo::{CELLS_X, CELLS_Y, CellIx, Pos, TICKS_PER_M};
use kd_core::ids::{PlaceKind, Uid};
use kd_core::num;
use kd_core::time::GameTime;
use kd_data::world::{Land, Side};
use kd_data::{Catalogue, Measure, Tuning};

use crate::area::relief;
use crate::cells::{
    CELL_M, CELL_TICKS, CaveKind, CaveRecord, EscarpmentLine, FixedCells, Geo, NONE, WorldCells, caves_byte,
    cliff_byte, side_dir, water,
};
use crate::purposes;

/// The numbers `data/tuning/lands.md` sets (A5.6), in metres where they are lengths.
#[derive(Clone, Debug, PartialEq)]
pub struct LandTuning {
    pub shelf_period: f32,
    pub shelf_amplitude: f32,
    pub shelf_octaves: u32,
    pub coast_rise_m: f32,
    pub coast_depth_m: f32,
    pub sea_depth_m: f32,
    pub relief_period_m: f32,
    pub relief_octaves: u32,
    pub uplands_m: f32,
    pub uplands_share: f32,
    pub uplands_fade_m: f32,
    pub source_inside_m: f32,
    pub control_every_m: f32,
    pub shift_m: f32,
    pub above_river_m: f32,
    pub valley_side_m: f32,
    pub escarpment_offset_m: f32,
    pub top_m: [f32; 2],
    pub second_m: [f32; 2],
    pub caves_within_m: f32,
    pub caves_apart_m: f32,
    pub cave_depth_m: [f32; 2],
    pub cave_dry_share: f32,
    pub camp_within_m: f32,
    /// From `data/tuning/world.md`: how often an escarpment has a gap people can climb.
    pub gap_every_m: f32,
}

impl LandTuning {
    pub fn from(cat: &Catalogue) -> Result<LandTuning, String> {
        let t = cat.tuning("lands")?;
        let len = |k: &str| t.get(k, Measure::Length);
        let num = |k: &str| t.number(k);
        let count = |k: &str| -> Result<u32, String> {
            let v = t.number(k)?;
            if v >= 1.0 && v.fract() == 0.0 {
                Ok(v as u32)
            } else {
                Err(format!("tuning lands: {k} is {v}, not a count"))
            }
        };
        let world: &Tuning = cat.tuning("world")?;
        Ok(LandTuning {
            shelf_period: num("coast.shelf_period")?,
            shelf_amplitude: num("coast.shelf_amplitude")?,
            shelf_octaves: count("coast.shelf_octaves")?,
            coast_rise_m: len("coast.rise")?,
            coast_depth_m: len("coast.depth")?,
            sea_depth_m: len("coast.sea_depth")?,
            relief_period_m: len("relief.period")?,
            relief_octaves: count("relief.octaves")?,
            uplands_m: len("uplands.height")?,
            uplands_share: num("uplands.share")?,
            uplands_fade_m: len("uplands.fade")?,
            source_inside_m: len("valley.source_inside")?,
            control_every_m: len("valley.control_every")?,
            shift_m: len("valley.shift")?,
            above_river_m: len("valley.above_river")?,
            valley_side_m: len("valley.side")?,
            escarpment_offset_m: len("escarpment.offset")?,
            top_m: [len("rocks.top_min")?, len("rocks.top_max")?],
            second_m: [len("rocks.second_min")?, len("rocks.second_max")?],
            caves_within_m: len("caves.within")?,
            caves_apart_m: len("caves.apart")?,
            cave_depth_m: [len("caves.depth_min")?, len("caves.depth_max")?],
            cave_dry_share: num("caves.dry_share")?,
            camp_within_m: len("caves.camp_within")?,
            gap_every_m: world.get("escarpment_gap_every", Measure::Length)?,
        })
    }
}

/// The square of cells a preset fills: columns `x0` to `x0 + size − 1` and rows `y0` to `y0 + size − 1`.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Window {
    pub x0: u32,
    pub y0: u32,
    pub size: u32,
}

impl Window {
    /// Its cells, row by row from the north-west.
    pub fn cells(self) -> impl Iterator<Item = CellIx> {
        (0..self.size * self.size).map(move |k| CellIx::at(self.x0 + k % self.size, self.y0 + k / self.size))
    }

    pub fn contains(self, c: CellIx) -> bool {
        let (x, y) = c.xy();
        (self.x0..self.x0 + self.size).contains(&x) && (self.y0..self.y0 + self.size).contains(&y)
    }
}

/// A land preset made into cells: the world's cells, the square it fills, its middle, and its river's line from
/// source to sea in ticks with the water's level at each point, for the river's cells (A5.7 step 5).
#[derive(Clone, Debug, PartialEq)]
pub struct PresetWorld {
    pub cells: WorldCells,
    pub window: Window,
    pub centre: CellIx,
    pub river: Vec<[i32; 2]>,
    pub river_level: Vec<f32>,
}

/// The `Side` from the island's middle toward each way, as a unit step in metres (x east, y south).
fn toward(s: Side) -> [f32; 2] {
    match s {
        Side::North => [0.0, -1.0],
        Side::East => [1.0, 0.0],
        Side::South => [0.0, 1.0],
        Side::West => [-1.0, 0.0],
    }
}

fn opposite(s: Side) -> Side {
    match s {
        Side::North => Side::South,
        Side::East => Side::West,
        Side::South => Side::North,
        Side::West => Side::East,
    }
}

/// A position's x and y.
fn ticks(p: Pos) -> [i32; 2] {
    [p.x, p.y]
}

/// 0 below `a`, 1 above `b`, and the smooth step between.
fn smooth(a: f32, b: f32, x: f32) -> f32 {
    let t = num::min(1.0, num::max(0.0, (x - a) / (b - a)));
    t * t * (3.0 - 2.0 * t)
}

fn len2(v: [f32; 2]) -> f32 {
    (v[0] * v[0] + v[1] * v[1]).sqrt()
}

/// The island's shape: its coast, sea floor and hills as functions of the place in metres from its middle.
struct Shape {
    radius_m: f32,
    sea_m: f32,
    base_m: f32,
    relief_m: f32,
    shelf_seed: u64,
    relief_seed: u64,
    t: LandTuning,
}

impl Shape {
    /// A noise of `period_m` with `octaves` octaves at `q`, within ±1: the relief noise's lattice scaled so its
    /// largest spacing is the period, worked out from the island's middle so any period serves.
    fn noise(seed: u64, q: [f32; 2], period_m: f32, octaves: u32) -> f32 {
        let scale = relief::MAX_PERIOD_M as f32 / period_m * TICKS_PER_M as f32;
        relief::fbm(
            seed,
            (q[0] * scale) as i64,
            (q[1] * scale) as i64,
            relief::MAX_PERIOD_M,
            octaves,
        )
    }

    /// The coast's distance from the middle toward `q`: the island's radius moved by the shelf noise.
    fn coast_r(&self, q: [f32; 2]) -> f32 {
        let width = 2.0 * self.radius_m;
        self.radius_m
            + width
                * self.t.shelf_amplitude
                * Shape::noise(self.shelf_seed, q, width * self.t.shelf_period, self.t.shelf_octaves)
    }

    fn is_land(&self, q: [f32; 2]) -> bool {
        len2(q) < self.coast_r(q)
    }

    /// Whether `q` lies beyond the sea, where cells are void: a circle `sea_m` beyond the island's radius.
    fn is_void(&self, q: [f32; 2]) -> bool {
        len2(q) > self.radius_m + self.sea_m
    }

    /// The sea floor's height at `q`, off the coast: from the shore's depth down to the outer sea's.
    fn sea_floor(&self, q: [f32; 2]) -> f32 {
        let out = num::max(0.0, len2(q) - self.coast_r(q)) / self.sea_m;
        -(self.t.coast_depth_m + (self.t.sea_depth_m - self.t.coast_depth_m) * num::min(1.0, out))
    }

    /// The hills alone, about the base height.
    fn hills(&self, q: [f32; 2]) -> f32 {
        self.relief_m * Shape::noise(self.relief_seed, q, self.t.relief_period_m, self.t.relief_octaves)
    }

    /// The land's height before its valley and escarpment: the base, the hills and the uplands of the north
    /// third, rising from the shore over `coast_rise_m`.
    fn ground(&self, q: [f32; 2]) -> f32 {
        let inland = self.coast_r(q) - len2(q);
        let rise = smooth(0.0, self.t.coast_rise_m, inland);
        // The north third: north of a third of the radius above the middle.
        let edge = -self.radius_m * (1.0 - 2.0 * self.t.uplands_share);
        let half = 0.5 * self.t.uplands_fade_m;
        let uplands = self.t.uplands_m * smooth(edge + half, edge - half, q[1]);
        rise * (self.base_m + self.hills(q) + uplands)
    }
}

/// The river's line from its source to the sea, in metres from the island's middle, with the length along it and
/// the water's level at each point, and an index of its segments by 2 km square for finding the nearest.
struct ValleyLine {
    points: Vec<[f32; 2]>,
    level: Vec<f32>,
    squares: std::collections::BTreeMap<(i32, i32), Vec<u32>>,
}

const SQUARE_M: f32 = 2_048.0;

impl ValleyLine {
    fn square(q: [f32; 2]) -> (i32, i32) {
        ((q[0] / SQUARE_M).floor() as i32, (q[1] / SQUARE_M).floor() as i32)
    }

    fn new(points: Vec<[f32; 2]>, level: Vec<f32>, reach_m: f32) -> ValleyLine {
        let mut squares: std::collections::BTreeMap<(i32, i32), Vec<u32>> = std::collections::BTreeMap::new();
        for k in 0..points.len().saturating_sub(1) {
            let (a, b) = (points[k], points[k + 1]);
            let lo = ValleyLine::square([num::min(a[0], b[0]) - reach_m, num::min(a[1], b[1]) - reach_m]);
            let hi = ValleyLine::square([num::max(a[0], b[0]) + reach_m, num::max(a[1], b[1]) + reach_m]);
            for sy in lo.1..=hi.1 {
                for sx in lo.0..=hi.0 {
                    squares.entry((sx, sy)).or_default().push(k as u32);
                }
            }
        }
        ValleyLine { points, level, squares }
    }

    /// The nearest point of the line to `q` within the index's reach: its distance and the water's level there.
    fn nearest(&self, q: [f32; 2]) -> Option<(f32, f32)> {
        let mut best: Option<(f32, f32)> = None;
        for &k in self.squares.get(&ValleyLine::square(q)).into_iter().flatten() {
            let k = k as usize;
            let (a, b) = (self.points[k], self.points[k + 1]);
            let ab = [b[0] - a[0], b[1] - a[1]];
            let l2 = ab[0] * ab[0] + ab[1] * ab[1];
            let t = if l2 > 0.0 {
                num::min(1.0, num::max(0.0, ((q[0] - a[0]) * ab[0] + (q[1] - a[1]) * ab[1]) / l2))
            } else {
                0.0
            };
            let near = [a[0] + ab[0] * t, a[1] + ab[1] * t];
            let d = len2([q[0] - near[0], q[1] - near[1]]);
            let level = self.level[k] + (self.level[k + 1] - self.level[k]) * t;
            if best.is_none_or(|(bd, _)| d < bd) {
                best = Some((d, level));
            }
        }
        best
    }
}

/// A point on a uniform Catmull–Rom spline through `p1` and `p2`, `t` from 0 to 1 between them.
fn catmull_rom(p0: [f32; 2], p1: [f32; 2], p2: [f32; 2], p3: [f32; 2], t: f32) -> [f32; 2] {
    let (t2, t3) = (t * t, t * t * t);
    std::array::from_fn(|i| {
        0.5 * (2.0 * p1[i]
            + (p2[i] - p0[i]) * t
            + (2.0 * p0[i] - 5.0 * p1[i] + 4.0 * p2[i] - p3[i]) * t2
            + (3.0 * p1[i] - p0[i] - 3.0 * p2[i] + p3[i]) * t3)
    })
}

/// Samples on each span of the river's spline.
const SPLINE_STEPS: usize = 32;

/// How a place in metres from the island's middle sits in the world, and back.
#[derive(Clone, Copy)]
struct Frame {
    middle: Pos,
}

impl Frame {
    fn world(self, q: [f32; 2]) -> [i32; 2] {
        let t = TICKS_PER_M as f32;
        [
            self.middle.x + (q[0] * t).round() as i32,
            self.middle.y + (q[1] * t).round() as i32,
        ]
    }

    fn local(self, c: CellIx) -> [f32; 2] {
        let mid = c.centre();
        let t = TICKS_PER_M as f32;
        [(mid.x - self.middle.x) as f32 / t, (mid.y - self.middle.y) as f32 / t]
    }

    fn cell(self, q: [f32; 2]) -> CellIx {
        let [x, y] = self.world(q);
        CellIx::of(Pos { x, y, z: 0 })
    }
}

/// The river's line (A5.6): from `source_inside_m` inside the coast on the valley's side, toward the opposite
/// coast through points every `control_every_m`, each but the first moved to one side by a keyed draw of up to
/// `shift_m`, smoothed by a Catmull–Rom spline, and run on into the sea until it reaches a sea cell; the water
/// falls evenly from the source's height to the sea's at the coast.
fn river_line(p: &Land, seed: u64, shape: &Shape, frame: Frame, step: &dyn Fn([f32; 2]) -> f32) -> Option<ValleyLine> {
    if p.valley.flow_m3s <= 0.0 {
        return None;
    }
    let t = &shape.t;
    let up = toward(p.valley.from);
    let down = toward(opposite(p.valley.from));
    let across = [-down[1], down[0]];
    // The coast on the valley's side, then the source inside it.
    let mut d = 0.0;
    while shape.is_land([up[0] * d, up[1] * d]) && d < shape.radius_m * 2.0 {
        d += 64.0;
    }
    let source = [up[0] * (d - t.source_inside_m), up[1] * (d - t.source_inside_m)];
    // Control points, every one past the far coast's crossing and two more.
    let shifts = Stream::new(seed, &purposes::VALLEY_SHIFT);
    let mut controls = vec![source];
    let mut past = 0;
    let mut k = 1;
    while past < 2 && k < 200 {
        let along = [
            source[0] + down[0] * t.control_every_m * k as f32,
            source[1] + down[1] * t.control_every_m * k as f32,
        ];
        let subject = Uid::place(PlaceKind::Cell, u64::from(frame.cell(along).0)).0;
        let shift = (shifts.unit(subject, moment(GameTime(0), 0)) * 2.0 - 1.0) * t.shift_m;
        controls.push([along[0] + across[0] * shift, along[1] + across[1] * shift]);
        if !shape.is_land(along) {
            past += 1;
        }
        k += 1;
    }
    let n = controls.len();
    let at = |i: isize| -> [f32; 2] {
        if i < 0 {
            let (a, b) = (controls[0], controls[1]);
            [2.0 * a[0] - b[0], 2.0 * a[1] - b[1]]
        } else if i as usize >= n {
            let (a, b) = (controls[n - 1], controls[n - 2]);
            [2.0 * a[0] - b[0], 2.0 * a[1] - b[1]]
        } else {
            controls[i as usize]
        }
    };
    let mut points = Vec::new();
    for i in 0..n as isize - 1 {
        for s in 0..SPLINE_STEPS {
            points.push(catmull_rom(
                at(i - 1),
                at(i),
                at(i + 1),
                at(i + 2),
                s as f32 / SPLINE_STEPS as f32,
            ));
        }
    }
    points.push(controls[n - 1]);
    // Keep the line up to the first point in a sea cell.
    let sea_cell = |q: [f32; 2]| !shape.is_land(frame.local(frame.cell(q)));
    let end = points.iter().position(|&q| sea_cell(q)).map_or(points.len(), |i| i + 1);
    points.truncate(end);
    // The water falls evenly from the source's height to the sea at the coast.
    let mut s = vec![0.0f32; points.len()];
    for i in 1..points.len() {
        s[i] = s[i - 1] + len2([points[i][0] - points[i - 1][0], points[i][1] - points[i - 1][1]]);
    }
    let mouth = points
        .iter()
        .position(|&q| !shape.is_land(q))
        .map_or(s[s.len() - 1], |i| s[i]);
    let top = shape.ground(source) + step(source);
    let level = s.iter().map(|&si| top * num::max(0.0, 1.0 - si / mouth)).collect();
    Some(ValleyLine::new(
        points,
        level,
        t.valley_side_m + 0.5 * p.valley.floodplain_m + SQUARE_M,
    ))
}

/// Builds a land preset into the world's cells (A5.6): its coast and sea, its hills and uplands, its river's
/// valley and its escarpment, the rock beds, and the caves and shelters along the escarpment.
pub fn build(p: &Land, seed: u64, cat: &Catalogue) -> Result<PresetWorld, String> {
    let t = LandTuning::from(cat)?;
    let [cx, cy] = p.centre_cell;
    if cx >= CELLS_X || cy >= CELLS_Y {
        return Err(format!("land {}: its centre cell lies off the world", p.id));
    }
    let centre = CellIx::at(cx, cy);
    let radius_m = 0.5 * p.island_m;
    let size = 2 * ((radius_m + p.sea_m) / CELL_M).ceil() as u32 + 2;
    let half = size / 2;
    if cx < half || cx + half > CELLS_X || cy <= half || cy + half > CELLS_Y {
        return Err(format!(
            "land {}: its {size} × {size} cells cross the world's edge or its seam",
            p.id
        ));
    }
    let window = Window {
        x0: cx - half,
        y0: cy - half,
        size,
    };
    let frame = Frame {
        middle: centre.centre(),
    };
    let land_uid = Uid::place(PlaceKind::Cell, u64::from(centre.0)).0;
    let shape = Shape {
        radius_m,
        sea_m: p.sea_m,
        base_m: p.base_height_m,
        relief_m: p.relief_m,
        shelf_seed: Stream::new(seed, &purposes::LAND_SHELF).draw(land_uid, moment(GameTime(0), 0)),
        relief_seed: Stream::new(seed, &purposes::LAND_RELIEF).draw(land_uid, moment(GameTime(0), 0)),
        t: t.clone(),
    };

    // The escarpment's line, on the row (or column) of cells nearest its offset, through their middles.
    let x = &p.escarpment;
    let line = (x.height_m > 0.0).then(|| {
        let n = (t.escarpment_offset_m / CELL_M).round() as i32;
        let dir = toward(x.side);
        let row = CellIx::at(
            (cx as i32 + n * dir[0] as i32) as u32,
            (cy as i32 + n * dir[1] as i32) as u32,
        );
        let mid = row.centre();
        EscarpmentLine {
            facing: opposite(x.side),
            at: if dir[1] != 0.0 { mid.y } else { mid.x },
            span: [0, 0],
            breaks: Vec::new(),
            gaps: Vec::new(),
            height_m: x.height_m,
        }
    });
    let step = |q: [f32; 2]| line.as_ref().map_or(0.0, |l| l.height_m * l.share_at(frame.world(q)));
    let valley = river_line(p, seed, &shape, frame, &step);
    let valley_weight = |q: [f32; 2]| -> Option<(f32, f32)> {
        let (d, level) = valley.as_ref()?.nearest(q)?;
        let w = 1.0
            - smooth(
                0.5 * p.valley.floodplain_m,
                0.5 * p.valley.floodplain_m + t.valley_side_m,
                d,
            );
        (w > 0.0).then_some((w, level + t.above_river_m))
    };

    // Each cell's height and water, and its spread.
    let mut f = FixedCells::void();
    for c in window.cells() {
        let i = c.0 as usize;
        let q = frame.local(c);
        if shape.is_void(q) {
            continue;
        }
        if !shape.is_land(q) {
            f.height[i] = shape.sea_floor(q).round() as i16;
            f.water[i] = water::SEA;
            continue;
        }
        let mut h = shape.ground(q)
            + line
                .as_ref()
                .map_or(0.0, |l| l.height_m * l.share_at(ticks(c.centre())));
        if let Some((w, floor)) = valley_weight(q) {
            h += (floor - h) * w;
        }
        f.height[i] = num::max(-32_000.0, num::min(32_000.0, h)).round() as i16;
        f.water[i] = 0;
        f.geo[i] = Geo::Basin as u8;
        let third = CELL_M / 3.0;
        let (mut lo, mut hi) = (f32::INFINITY, f32::NEG_INFINITY);
        for k in 0..9 {
            let o = [
                q[0] + third * (k % 3) as f32 - third,
                q[1] + third * (k / 3) as f32 - third,
            ];
            let v = shape.hills(o);
            lo = num::min(lo, v);
            hi = num::max(hi, v);
        }
        f.rough[i] = num::min(255.0, ((hi - lo) / 4.0).round()) as u8;
    }

    // The escarpment's face along the land it crosses, its breaks where the valley runs, and its gaps.
    let mut lines = Vec::new();
    if let Some(mut l) = line {
        escarpment_face(&mut l, &mut f, window, frame, &shape, &valley_weight)?;
        lines.push(l);
    }
    f.escarpments = lines;

    rock_beds(p, seed, &t, &mut f, window);
    if let Some(l) = f.escarpments.first().cloned() {
        caves(p, seed, &t, &mut f, &l, frame, valley.as_ref())?;
    }

    let (river, river_level) = valley.map_or((Vec::new(), Vec::new()), |v| {
        (v.points.iter().map(|&q| frame.world(q)).collect(), v.level)
    });
    Ok(PresetWorld {
        cells: WorldCells {
            fixed: std::sync::Arc::new(f),
            state: crate::cells::CellState::new(),
        },
        window,
        centre,
        river,
        river_level,
    })
}

/// Along the escarpment's row: its face's span over the land, broken where the valley's sides reach the line, the
/// gaps at the lowest point of each `gap_every_m` of face, and the `cliff` and `feature` bytes of its cells.
fn escarpment_face(
    l: &mut EscarpmentLine,
    f: &mut FixedCells,
    window: Window,
    frame: Frame,
    shape: &Shape,
    valley: &dyn Fn([f32; 2]) -> Option<(f32, f32)>,
) -> Result<(), String> {
    let east_west = matches!(l.facing, Side::North | Side::South);
    // The line's cells across the window, and the run of land through the middle's.
    let cells: Vec<CellIx> = (0..window.size)
        .map(|k| {
            let (lx, ly) = if east_west {
                (
                    window.x0 + k,
                    CellIx::of(Pos {
                        x: frame.middle.x,
                        y: l.at,
                        z: 0,
                    })
                    .xy()
                    .1,
                )
            } else {
                (
                    CellIx::of(Pos {
                        x: l.at,
                        y: frame.middle.y,
                        z: 0,
                    })
                    .xy()
                    .0,
                    window.y0 + k,
                )
            };
            CellIx::at(lx, ly)
        })
        .collect();
    let land = |c: CellIx| f.water[c.0 as usize] & (water::SEA | water::VOID) == 0;
    let mid = (window.size / 2) as usize;
    if !land(cells[mid]) {
        return Err("the escarpment's line misses the island's middle".into());
    }
    let (mut a, mut b) = (mid, mid);
    while a > 0 && land(cells[a - 1]) {
        a -= 1;
    }
    while b + 1 < cells.len() && land(cells[b + 1]) {
        b += 1;
    }
    let along = |c: CellIx| if east_west { c.centre().x } else { c.centre().y };
    let half = CELL_TICKS / 2;
    l.span = [along(cells[a]) - half, along(cells[b]) + half];
    // Where the valley's sides reach the line, sampled every 64 m: no face there.
    let tick = TICKS_PER_M as f32;
    let to_local = |s: i32| -> [f32; 2] {
        let w = if east_west { [s, l.at] } else { [l.at, s] };
        [
            (w[0] - frame.middle.x) as f32 / tick,
            (w[1] - frame.middle.y) as f32 / tick,
        ]
    };
    let stride = 64 * TICKS_PER_M;
    let mut open: Option<i32> = None;
    let mut s = l.span[0];
    while s <= l.span[1] {
        let inside = valley(to_local(s)).is_some();
        match (inside, open) {
            (true, None) => open = Some(s),
            (false, Some(o)) => {
                l.breaks.push([o, s]);
                open = None;
            }
            _ => {}
        }
        s += stride;
    }
    if let Some(o) = open {
        l.breaks.push([o, l.span[1]]);
    }
    // Each break out to its cells' edges, so a cell is all face or none.
    for b in &mut l.breaks {
        b[0] = b[0].div_euclid(CELL_TICKS) * CELL_TICKS;
        b[1] = (b[1] + CELL_TICKS - 1).div_euclid(CELL_TICKS) * CELL_TICKS;
    }
    // The face's pieces between the breaks, and a gap at the lowest point of every stretch of each.
    let mut pieces = Vec::new();
    let mut from = l.span[0];
    for b in &l.breaks {
        if b[0] > from {
            pieces.push([from, b[0]]);
        }
        from = b[1];
    }
    if l.span[1] > from {
        pieces.push([from, l.span[1]]);
    }
    let every = (shape.t.gap_every_m * tick) as i32;
    for piece in pieces {
        let mut start = piece[0];
        while start < piece[1] {
            let end = (start + every).min(piece[1]);
            let mut best = (f32::INFINITY, start);
            let mut s = start;
            while s <= end {
                let h = shape.ground(to_local(s));
                if h < best.0 {
                    best = (h, s);
                }
                s += stride;
            }
            l.gaps.push(best.1);
            start = end;
        }
    }
    // The cells of the face.
    let in_break = |s: i32| l.breaks.iter().any(|b| (b[0]..=b[1]).contains(&s));
    for &c in &cells[a..=b] {
        if !in_break(along(c)) {
            let i = c.0 as usize;
            f.cliff[i] = cliff_byte(side_dir(l.facing), l.height_m);
            f.feature[i] = 1;
        }
    }
    Ok(())
}

/// Each land cell's rock beds, top first, as the preset names them (A5.6): the top two beds' thickness keyed on the
/// cell, and on the escarpment's low side the top thinned by its height, so the step cuts down through the beds.
fn rock_beds(p: &Land, seed: u64, t: &LandTuning, f: &mut FixedCells, window: Window) {
    let draws = Stream::new(seed, &purposes::ROCK_BEDS);
    let rocks: Vec<u8> = p.escarpment.rocks.iter().map(|&r| r.min(254) as u8).collect();
    let line = f.escarpments.first().cloned();
    for c in window.cells() {
        let i = c.0 as usize;
        if f.water[i] & (water::SEA | water::VOID) != 0 {
            continue;
        }
        let subject = Uid::place(PlaceKind::Cell, u64::from(c.0)).0;
        let top = draws.range(subject, moment(GameTime(0), 0), t.top_m[0], t.top_m[1]);
        let second = draws.range(subject, moment(GameTime(0), 1), t.second_m[0], t.second_m[1]);
        let mut beds: Vec<(u8, f32)> = rocks
            .iter()
            .zip([top, second, f32::INFINITY])
            .map(|(&r, h)| (r, h))
            .collect();
        let mut cut = line.as_ref().map_or(0.0, |l| {
            if l.share_at(ticks(c.centre())) == 0.0 {
                l.height_m
            } else {
                0.0
            }
        });
        while cut > 0.0 && beds.len() > 1 {
            if cut < beds[0].1 {
                beds[0].1 -= cut;
                cut = 0.0;
            } else {
                cut -= beds[0].1;
                beds.remove(0);
            }
        }
        let mut col = [NONE; 3];
        let mut thick = [0u8; 2];
        for (k, &(r, h)) in beds.iter().take(3).enumerate() {
            col[k] = r;
            if k < 2 && h.is_finite() {
                thick[k] = num::min(255.0, (h / 2.0).round()) as u8;
            }
        }
        f.rock[i] = col;
        f.layer_m[i] = thick;
    }
}

/// The preset's caves and shelters along the escarpment's face within `caves_within_m` of the valley (A5.6): the
/// first a dry cave within `camp_within_m` of the river, so the first camp has one; the rest at keyed places at
/// least `caves_apart_m` apart; the deepest dry cave within reach of the river is the first camp's (A8.8).
fn caves(
    p: &Land,
    seed: u64,
    t: &LandTuning,
    f: &mut FixedCells,
    l: &EscarpmentLine,
    frame: Frame,
    valley: Option<&ValleyLine>,
) -> Result<(), String> {
    let total = usize::from(p.escarpment.caves) + usize::from(p.escarpment.shelters);
    if total == 0 {
        return Ok(());
    }
    let east_west = matches!(l.facing, Side::North | Side::South);
    let tick = TICKS_PER_M as f32;
    // Where the river crosses the line, or the island's middle with no river.
    let cross = valley
        .and_then(|v| {
            let along = |q: [f32; 2]| if east_west { q[0] } else { q[1] };
            let across_q = |q: [f32; 2]| if east_west { q[1] } else { q[0] };
            let at_local = (l.at - if east_west { frame.middle.y } else { frame.middle.x }) as f32 / tick;
            v.points.windows(2).find_map(|w| {
                let (a, b) = (across_q(w[0]) - at_local, across_q(w[1]) - at_local);
                (a.signum() != b.signum() || a == 0.0).then(|| {
                    let s = if a == b { 0.0 } else { a / (a - b) };
                    along(w[0]) + (along(w[1]) - along(w[0])) * s
                })
            })
        })
        .unwrap_or(0.0);
    let middle = if east_west { frame.middle.x } else { frame.middle.y };
    let cross_t = middle + (cross * tick).round() as i32;
    let on_face = |s: i32| (l.span[0]..=l.span[1]).contains(&s) && !l.breaks.iter().any(|b| (b[0]..=b[1]).contains(&s));
    let near_break = l
        .breaks
        .iter()
        .find(|b| (b[0]..=b[1]).contains(&cross_t))
        .map_or(0, |b| (cross_t - b[0]).max(b[1] - cross_t));
    let subject = Uid::place(PlaceKind::Feature, 0).0;
    let place = Stream::new(seed, &purposes::CAVE_PLACE);
    let size = Stream::new(seed, &purposes::CAVE_SIZE);
    let dry = Stream::new(seed, &purposes::CAVE_DRY);
    let within = (t.caves_within_m * tick) as i32;
    let camp = (t.camp_within_m * tick) as i32;
    let apart = (t.caves_apart_m * tick) as i64;
    let mut placed: Vec<(usize, i32)> = Vec::new();
    for k in 0..total {
        for a in 0..256u16 {
            let slot = |n: u16| moment(GameTime(0), (k as u16) << 9 | a << 1 | n);
            let u = place.unit(subject, slot(0));
            let s = if k == 0 {
                // The first camp's: on either side of the river, past the break, within reach.
                let side = if place.unit(subject, slot(1)) < 0.5 { -1 } else { 1 };
                cross_t + side * (near_break + ((camp - near_break).max(0) as f32 * u) as i32)
            } else {
                cross_t + ((u * 2.0 - 1.0) * within as f32) as i32
            };
            let clear = placed
                .iter()
                .all(|&(_, o)| (i64::from(o) - i64::from(s)).abs() >= apart);
            if on_face(s) && clear {
                placed.push((k, s));
                break;
            }
        }
    }
    if placed.first().is_none_or(|&(k, _)| k != 0) {
        return Err("no place for the first camp's cave near the river".into());
    }
    let mut records: Vec<CaveRecord> = placed
        .iter()
        .map(|&(k, s)| {
            let kind = if k < usize::from(p.escarpment.caves) {
                CaveKind::Cave
            } else {
                CaveKind::Shelter
            };
            let mouth = if east_west { [s, l.at] } else { [l.at, s] };
            let depth = size.range(
                subject,
                moment(GameTime(0), k as u16),
                t.cave_depth_m[0],
                t.cave_depth_m[1],
            );
            CaveRecord {
                cell: CellIx::of(Pos {
                    x: mouth[0],
                    y: mouth[1],
                    z: 0,
                }),
                mouth,
                kind,
                depth_m: depth.round() as u8,
                dry: kind == CaveKind::Shelter
                    || k == 0
                    || dry.chance(subject, moment(GameTime(0), k as u16), t.cave_dry_share),
                first_camp: false,
            }
        })
        .collect();
    // The first camp's: the deepest dry cave within reach of the river, the first placed among equals.
    let camp_cave = (0..records.len())
        .filter(|&i| {
            let r = &records[i];
            let s = if east_west { r.mouth[0] } else { r.mouth[1] };
            r.kind == CaveKind::Cave && r.dry && (s - cross_t).abs() <= camp
        })
        .max_by(|&i, &j| records[i].depth_m.cmp(&records[j].depth_m).then(j.cmp(&i)));
    if let Some(i) = camp_cave {
        records[i].first_camp = true;
    }
    records.sort_by_key(|r| (r.cell, r.mouth));
    for (k, r) in records.iter().enumerate() {
        let here: Vec<&CaveRecord> = records.iter().filter(|o| o.cell == r.cell).collect();
        if here.first().is_some_and(|first| std::ptr::eq(*first, &records[k])) {
            let deepest = here.iter().copied().max_by_key(|o| o.depth_m);
            f.caves[r.cell.0 as usize] = caves_byte(here.len(), deepest);
        }
    }
    f.cave_records = records;
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_core::geo::dist;

    fn catalogue() -> Catalogue {
        kd_data::compile::compile(&kd_data::repo_sources!(), false)
            .expect("the catalogue compiles")
            .catalogue
    }

    fn first_region(cat: &Catalogue) -> PresetWorld {
        build(cat.land("first_region").expect("the first region"), 1, cat).expect("the island builds")
    }

    // checks: WLD-34 WLD-12
    #[test]
    fn island_size() {
        // Land within 10% of 60 × 60 km, its sea at least 45 km wide on every side, void beyond, in 160 × 160
        // cells round the centre.
        let cat = catalogue();
        let w = first_region(&cat);
        let f = &w.cells.fixed;
        assert_eq!(w.window.size, 160);
        let land: Vec<CellIx> = w
            .window
            .cells()
            .filter(|c| f.water[c.0 as usize] & (water::SEA | water::VOID) == 0)
            .collect();
        let km2 = land.len() as f32 * (CELL_M / 1000.0) * (CELL_M / 1000.0);
        let circle = std::f32::consts::PI * 30.0 * 30.0;
        assert!((km2 / circle - 1.0).abs() < 0.1, "{km2} km² of land");
        // The sea: from every land cell, at least 45 km to the nearest void cell.
        let void: Vec<CellIx> = w.window.cells().filter(|c| f.is_void(*c)).collect();
        let coast: Vec<CellIx> = land
            .iter()
            .copied()
            .filter(|c| c.neighbours8().iter().any(|n| f.water[n.0 as usize] & water::SEA != 0))
            .collect();
        for c in coast.iter().step_by(7) {
            let nearest = void
                .iter()
                .map(|v| dist(c.centre(), v.centre()))
                .fold(f32::INFINITY, num::min);
            assert!(nearest > 45_000.0, "{c:?}: void {nearest} m away");
        }
        // Outside the window every cell stays void; inside, the sea floor lies from 20 m to 200 m down.
        assert!(f.is_void(CellIx::at(w.window.x0 - 1, w.window.y0)));
        for c in w.window.cells() {
            if f.water[c.0 as usize] & water::SEA != 0 {
                assert!((-200..=-20).contains(&f.height[c.0 as usize]), "{c:?}");
            }
        }
        // The land stands up from the shore to its hills and uplands: its highest under base + relief + uplands +
        // escarpment, its middle about the base height.
        let heights: Vec<i16> = land.iter().map(|c| f.height[c.0 as usize]).collect();
        let top = *heights.iter().max().unwrap();
        assert!((250..=430).contains(&top), "{top}");
        let mid = f.height[w.centre.0 as usize];
        assert!((100..=340).contains(&mid), "{mid}");
    }

    // checks: WLD-34 WLD-09 PRE-23
    #[test]
    fn escarpment_and_caves() {
        // 6 caves and 4 shelters, at least 1.5 km apart, all on the escarpment's face, one marked the first camp's
        // within 3 km of the river.
        let cat = catalogue();
        let w = first_region(&cat);
        let f = &w.cells.fixed;
        let l = &f.escarpments[0];
        assert_eq!((l.facing, l.height_m), (Side::South, 30.0));
        // 8 cells north of the middle, crossing the land from coast to coast, broken by the valley.
        assert_eq!(l.at, CellIx::at(1000, 236).centre().y);
        assert!(l.span[1] - l.span[0] > 40_000 * TICKS_PER_M, "{:?}", l.span);
        assert_eq!(l.breaks.len(), 1, "{:?}", l.breaks);
        assert!(!l.gaps.is_empty());
        let recs = &f.cave_records;
        assert_eq!(recs.len(), 10);
        assert_eq!(recs.iter().filter(|r| r.kind == CaveKind::Cave).count(), 6);
        assert_eq!(recs.iter().filter(|r| r.kind == CaveKind::Shelter).count(), 4);
        for (i, a) in recs.iter().enumerate() {
            assert!(f.cliff[a.cell.0 as usize] != 0, "{a:?} is not on the face");
            assert!((8..=30).contains(&a.depth_m));
            for b in &recs[i + 1..] {
                assert!((a.mouth[0] - b.mouth[0]).abs() >= 1_500 * TICKS_PER_M, "{a:?} {b:?}");
            }
        }
        let camps: Vec<&CaveRecord> = recs.iter().filter(|r| r.first_camp).collect();
        assert_eq!(camps.len(), 1);
        let camp = camps[0];
        assert!(camp.dry && camp.kind == CaveKind::Cave);
        let river_x = w
            .river
            .windows(2)
            .find(|s| (s[0][1] - l.at).signum() != (s[1][1] - l.at).signum())
            .map(|s| s[0][0])
            .expect("the river crosses the line");
        assert!((camp.mouth[0] - river_x).abs() <= 3_100 * TICKS_PER_M);
        // The face's cells look south, 30 m high; cells north of it stand about 30 m higher than south of it.
        let face: Vec<CellIx> = w.window.cells().filter(|c| f.cliff[c.0 as usize] != 0).collect();
        assert!(face.len() > 30);
        assert!(
            face.iter()
                .all(|c| crate::cells::cliff_of(f.cliff[c.0 as usize]) == Some((6, 30.0)))
        );
        // The beds: the face shows the preset's rocks, top first, and the low side has lost the step's depth.
        let chalk = cat.rock("chalk").unwrap().number as u8;
        let c = face[face.len() / 2];
        assert_eq!(f.rock[c.0 as usize][0], chalk);
        assert!((12..=20).contains(&f.layer_m[c.0 as usize][0]));
        // Every cell's caves byte counts its records.
        for r in recs {
            assert_eq!(usize::from(f.caves[r.cell.0 as usize] & 3), f.caves_of(r.cell).len());
        }
        // And all of it the same from the same seed, another from another.
        assert_eq!(first_region(&cat), w);
        let other = build(cat.land("first_region").unwrap(), 2, &cat).unwrap();
        assert_ne!(other.cells.fixed.height, f.height);
    }
}
