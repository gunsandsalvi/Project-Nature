//! World cells (A5.2): the world's coarse layer, 2,000 × 1,000 cells of 1,024 m, as columns indexed by `CellIx`.
//! The fixed layers, which generation or a land preset (A5.6) makes and nothing changes, sit in one `Arc` shared by
//! the simulation and the picture, with their side tables: river cells, stretches, caves, springs and escarpments.
//! The changing state is the world's own. Cells beyond a preset's sea are void, lifeless and drawn as haze, but
//! kept in the grid, so every code path is the real one (A5.6).
//!
//! The ground between cells' middles is the blend of their heights, and an escarpment's step is put back sharp
//! where its line runs (`surface_z`): a cell's height is its mean, the step included, so the blend alone would
//! smear a 30 m cliff over two kilometres.
//!
//! Implements WLD-12 and WLD-01, see A5.2 and A3.7: every cell's layers, read the same way at every place, the
//! world wrapping east and west and never across the seam.

mod coarse;

use std::sync::Arc;

use kd_core::geo::{CELLS_X, CELLS_Y, CellIx, NEIGHBOURS8, Pos, TICKS_PER_M, W, wrap};
use kd_core::num;
use kd_data::world::Side;

pub use coarse::{COARSE_M, COARSE_SIDE, CellCtx, CoarseGround, WaterLine, coarse_ground, coarse_point};

/// Cells in the grid.
pub const CELLS: usize = (CELLS_X * CELLS_Y) as usize;
/// A cell's side, in metres and in ticks.
pub const CELL_M: f32 = 1_024.0;
pub const CELL_TICKS: i32 = 1_024 * TICKS_PER_M;

/// A byte column's value for nothing: no rock, no biome, no steepest neighbour.
pub const NONE: u8 = 255;
/// No stretch below: the river reaches the sea.
pub const TO_SEA: u32 = u32::MAX;

/// The `water` column's flags (A5.2).
pub mod water {
    pub const SEA: u8 = 1;
    pub const LAKE: u8 = 2;
    pub const RIVER: u8 = 4;
    pub const STREAM: u8 = 8;
    pub const MARSH: u8 = 16;
    pub const SPRING: u8 = 32;
    pub const GLACIER: u8 = 64;
    /// Beyond a land preset's sea: no rule runs there, and the picture draws haze (A5.6).
    pub const VOID: u8 = 128;
}

/// A cell's geology class (A5.7), which picks its rock columns.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum Geo {
    OldLand = 0,
    Basin = 1,
    Folded = 2,
    Volcanic = 3,
    Rift = 4,
}

/// The `cliff` byte of an escarpment's cell: the way its face looks, as an index into `NEIGHBOURS8`, in the top
/// three bits, and its height in 2 m steps below, up to 62 m; 0 where there is none.
pub fn cliff_byte(facing: u8, height_m: f32) -> u8 {
    let steps = num::min(31.0, num::max(1.0, (height_m / 2.0).round())) as u8;
    (facing & 7) << 5 | steps
}

/// A `cliff` byte's facing and height in metres, if it holds a cliff.
pub fn cliff_of(b: u8) -> Option<(u8, f32)> {
    (b & 31 != 0).then(|| (b >> 5, f32::from(b & 31) * 2.0))
}

/// The index into `NEIGHBOURS8` of a compass side.
pub fn side_dir(s: Side) -> u8 {
    let step = match s {
        Side::East => (1, 0),
        Side::North => (0, -1),
        Side::West => (-1, 0),
        Side::South => (0, 1),
    };
    NEIGHBOURS8.iter().position(|&d| d == step).unwrap_or(0) as u8
}

/// What kind of hollow a cave record is (A5.2).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum CaveKind {
    Cave = 0,
    LavaTube = 1,
    Shelter = 2,
}

impl CaveKind {
    pub fn from_u8(b: u8) -> Option<CaveKind> {
        [CaveKind::Cave, CaveKind::LavaTube, CaveKind::Shelter]
            .get(usize::from(b))
            .copied()
    }
}

/// The `caves` byte: how many records the cell has (0–3) in bits 0–1, and of its deepest, the kind in bits 2–3,
/// whether it is dry in bit 4 and its depth in 4 m steps in bits 5–7.
pub fn caves_byte(count: usize, deepest: Option<&CaveRecord>) -> u8 {
    let count = count.min(3) as u8;
    deepest.map_or(count, |c| {
        count | (c.kind as u8) << 2 | u8::from(c.dry) << 4 | (c.depth_m / 4).min(7) << 5
    })
}

/// One cell of a river (A5.2, A5.7 step 5): its stretch, where the river enters and leaves the cell (on its
/// edges, but for the source and the mouth), its water's level and its bankfull width and depth.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct RiverCell {
    pub cell: CellIx,
    pub stretch: u32,
    /// Entry and exit, x and y in ticks: a cell's exit is the next cell's entry.
    pub entry: [i32; 2],
    pub exit: [i32; 2],
    /// Metres above the sea; fixed until routing runs (A5.10).
    pub level_m: f32,
    pub width_m: f32,
    pub depth_m: f32,
}

/// A stretch of river, at most 10 km of its cells in order (A5.7 step 5).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Stretch {
    /// Its first cell's row in the river table, and how many it has.
    pub first: u32,
    pub cells: u32,
    pub length_m: f32,
    /// Its flow, fixed until routing runs (A5.10).
    pub flow_m3s: f32,
    /// The area draining through its last cell, km² (A5.7 step 4).
    pub drainage_km2: f32,
    /// The stretch it runs into, or `TO_SEA`.
    pub down: u32,
}

/// A cave or rock shelter (A5.2): its cell, its mouth, its kind, how deep it runs, whether it is dry, and whether
/// it is the first camp's (A8.8).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct CaveRecord {
    pub cell: CellIx,
    pub mouth: [i32; 2],
    pub kind: CaveKind,
    pub depth_m: u8,
    pub dry: bool,
    pub first_camp: bool,
}

/// Where a spring rises (A5.10).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum SpringKind {
    /// At an escarpment's foot.
    CliffFoot = 0,
    /// Where a stream starts.
    StreamHead = 1,
}

/// A spring: its cell, where it rises and what it rises from.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Spring {
    pub cell: CellIx,
    pub at: [i32; 2],
    pub kind: SpringKind,
}

/// An escarpment (A5.6, A5.3): a straight line where the land on its high side stands `height_m` higher, its face
/// looking to the low side; until faults and volcanoes join (`MIL-04`), every feature is one, and the `feature`
/// column of each cell it crosses holds its row plus one.
#[derive(Clone, Debug, PartialEq)]
pub struct EscarpmentLine {
    /// The side its face looks to: north or south for a line running east and west, east or west for one
    /// running north and south.
    pub facing: Side,
    /// The line, in ticks: its y for a face looking north or south, its x for one looking east or west.
    pub at: i32,
    /// Where its face runs along the line, in ticks: from its west end to its east end (north to south), on cells'
    /// edges.
    pub span: [i32; 2],
    /// Stretches of the line with no face, where a valley cuts through it, in ticks along it, on cells' edges.
    pub breaks: Vec<[i32; 2]>,
    /// Where people can climb it, a slope under 35° some 20 m wide, in ticks along it (A5.12).
    pub gaps: Vec<i32>,
    pub height_m: f32,
}

/// How far either side of a gap's middle its slope runs along the line, metres.
pub const GAP_HALF_M: f32 = 10.0;
/// How far, metres, the sharp step fades into the blend before the ends of a face and a break.
const STEP_FADE_M: f32 = 200.0;
/// The steepest a gap's slope is, as its run for each metre of rise: a slope under 35°.
const GAP_RUN: f32 = 1.43;

impl EscarpmentLine {
    /// Whether its line runs east and west.
    fn east_west(&self) -> bool {
        matches!(self.facing, Side::North | Side::South)
    }

    /// A position's distance across the line toward its high side, and along it, in ticks.
    fn across_along(&self, p: [i32; 2]) -> (i32, i32) {
        let (across, along) = if self.east_west() { (p[1], p[0]) } else { (p[0], p[1]) };
        // The face looks to the low side; the high side lies the other way.
        let d = match self.facing {
            Side::South | Side::East => self.at - across,
            Side::North | Side::West => across - self.at,
        };
        (d, along)
    }

    /// The share of the step a cell's height holds: all of it on the high side, half on the line, none below.
    pub fn share_at(&self, p: [i32; 2]) -> f32 {
        let (d, _) = self.across_along(p);
        match d.signum() {
            1 => 1.0,
            0 => 0.5,
            _ => 0.0,
        }
    }

    /// How sharp the step is at a place along the line, 0 to 1: sharp along its face, fading to the blend over its
    /// last `STEP_FADE_M` before its ends and a break, so the face's cells hold all of it and no other cell any.
    fn sharpness(&self, along: i32) -> f32 {
        let fade = STEP_FADE_M * TICKS_PER_M as f32;
        let inside = |lo: i32, hi: i32| (along - lo).min(hi - along) as f32;
        let mut s = num::min(1.0, num::max(0.0, inside(self.span[0], self.span[1]) / fade));
        for b in &self.breaks {
            let out = -inside(b[0], b[1]);
            s = num::min(s, num::max(0.0, num::min(1.0, out / fade)));
        }
        s
    }

    /// Whether a place along the line lies in one of its gaps.
    fn in_gap(&self, along: i32) -> bool {
        self.gaps
            .iter()
            .any(|&g| ((along - g) as f32).abs() <= GAP_HALF_M * TICKS_PER_M as f32)
    }

    /// Where a place lies across the face, in ticks toward its high side, where it has one: none in a gap, nor where
    /// the step has faded below half its sharpness toward the face's ends and breaks.
    pub(crate) fn face_across(&self, p: [i32; 2]) -> Option<i32> {
        let (d, along) = self.across_along(p);
        (!self.in_gap(along) && self.sharpness(along) >= 0.5).then_some(d)
    }

    /// The step's share at a point between the blend's cells: 0 or 1 across the line, a slope under 35° through a
    /// gap.
    fn step_at(&self, p: [i32; 2]) -> f32 {
        let (d, along) = self.across_along(p);
        if self.in_gap(along) {
            let run = GAP_RUN * self.height_m * TICKS_PER_M as f32;
            num::min(1.0, num::max(0.0, 0.5 + d as f32 / run))
        } else if d > 0 {
            1.0
        } else {
            0.0
        }
    }
}

/// The fixed layers (A5.2), 32 bytes a cell, and their side tables.
#[derive(Clone, Debug, PartialEq)]
pub struct FixedCells {
    /// Mean height, metres (the sea floor below 0), and the height's spread inside the cell, in 4 m steps.
    pub height: Vec<i16>,
    pub rough: Vec<u8>,
    /// The surface rock and the two below, by catalogue number (`NONE` for none), and the top two's thickness in
    /// 2 m steps.
    pub rock: Vec<[u8; 3]>,
    pub layer_m: Vec<[u8; 2]>,
    /// The geology class (`Geo`), the escarpment's face (`cliff_byte`) and the caves (`caves_byte`).
    pub geo: Vec<u8>,
    pub cliff: Vec<u8>,
    pub caves: Vec<u8>,
    /// The soil kind by catalogue number, its base fertility (0–5 in 40ths) and the biome by catalogue number.
    pub soil: Vec<u8>,
    pub soil_base: Vec<u8>,
    pub biome: Vec<u8>,
    /// `water` flags, and the steepest neighbour downhill as an index into `NEIGHBOURS8`, or `NONE`.
    pub water: Vec<u8>,
    pub flow: Vec<u8>,
    /// A river cell's row in `rivers`, or a stream cell's drainage in km² (A5.10); a lake; a feature's row plus one.
    pub river: Vec<u32>,
    pub lake: Vec<u16>,
    pub feature: Vec<u16>,
    /// Two deposits as (catalogue number plus one, richness 1–5), 0 for none.
    pub deposits: Vec<[u8; 4]>,
    /// Temperature off its weather cell's, in 0.25 °C, and a rain factor; cold-air pooling and aspect; the sea's
    /// current's warmth (`WLD-26`).
    pub clim: Vec<[i8; 2]>,
    pub hollow: Vec<u8>,
    pub sea_warm: Vec<i8>,
    pub rivers: Vec<RiverCell>,
    pub stretches: Vec<Stretch>,
    pub cave_records: Vec<CaveRecord>,
    pub springs: Vec<Spring>,
    pub escarpments: Vec<EscarpmentLine>,
}

impl Default for FixedCells {
    fn default() -> FixedCells {
        FixedCells::void()
    }
}

impl FixedCells {
    /// The whole grid void: sea floor 200 m down, no rock, soil or biome, nothing flowing.
    pub fn void() -> FixedCells {
        FixedCells {
            height: vec![-200; CELLS],
            rough: vec![0; CELLS],
            rock: vec![[NONE; 3]; CELLS],
            layer_m: vec![[0; 2]; CELLS],
            geo: vec![0; CELLS],
            cliff: vec![0; CELLS],
            caves: vec![0; CELLS],
            soil: vec![NONE; CELLS],
            soil_base: vec![0; CELLS],
            biome: vec![NONE; CELLS],
            water: vec![water::VOID; CELLS],
            flow: vec![NONE; CELLS],
            river: vec![0; CELLS],
            lake: vec![0; CELLS],
            feature: vec![0; CELLS],
            deposits: vec![[0; 4]; CELLS],
            clim: vec![[0; 2]; CELLS],
            hollow: vec![0; CELLS],
            sea_warm: vec![0; CELLS],
            rivers: Vec::new(),
            stretches: Vec::new(),
            cave_records: Vec::new(),
            springs: Vec::new(),
            escarpments: Vec::new(),
        }
    }

    /// Whether a cell is void.
    pub fn is_void(&self, c: CellIx) -> bool {
        self.water[c.0 as usize] & water::VOID != 0
    }

    /// The cave records of a cell, which are kept in cell order.
    pub fn caves_of(&self, c: CellIx) -> &[CaveRecord] {
        let lo = self.cave_records.partition_point(|r| r.cell < c);
        let hi = self.cave_records.partition_point(|r| r.cell <= c);
        &self.cave_records[lo..hi]
    }

    /// The cells' fixed layers in their saved encoding (A14.3): each cell's 32 bytes in A5.2's column order,
    /// little-endian and fixed-width, cell by cell in the order given.
    pub fn encode_cells(&self, cells: &[CellIx]) -> Vec<u8> {
        let mut out = Vec::with_capacity(cells.len() * 32);
        for c in cells {
            let i = c.0 as usize;
            out.extend(self.height[i].to_le_bytes());
            out.push(self.rough[i]);
            out.extend(self.rock[i]);
            out.extend(self.layer_m[i]);
            out.extend([
                self.geo[i],
                self.cliff[i],
                self.caves[i],
                self.soil[i],
                self.soil_base[i],
                self.biome[i],
                self.water[i],
                self.flow[i],
            ]);
            out.extend(self.river[i].to_le_bytes());
            out.extend(self.lake[i].to_le_bytes());
            out.extend(self.feature[i].to_le_bytes());
            out.extend(self.deposits[i]);
            out.extend(self.clim[i].map(|v| v as u8));
            out.push(self.hollow[i]);
            out.push(self.sea_warm[i] as u8);
        }
        out
    }

    /// Reads back what `encode_cells` wrote for the same cells.
    pub fn decode_cells(&mut self, cells: &[CellIx], bytes: &[u8]) -> Result<(), String> {
        if bytes.len() != cells.len() * 32 {
            return Err(format!("{} bytes for {} cells", bytes.len(), cells.len()));
        }
        for (c, b) in cells.iter().zip(bytes.chunks_exact(32)) {
            let i = c.0 as usize;
            self.height[i] = i16::from_le_bytes([b[0], b[1]]);
            self.rough[i] = b[2];
            self.rock[i] = [b[3], b[4], b[5]];
            self.layer_m[i] = [b[6], b[7]];
            self.geo[i] = b[8];
            self.cliff[i] = b[9];
            self.caves[i] = b[10];
            self.soil[i] = b[11];
            self.soil_base[i] = b[12];
            self.biome[i] = b[13];
            self.water[i] = b[14];
            self.flow[i] = b[15];
            self.river[i] = u32::from_le_bytes([b[16], b[17], b[18], b[19]]);
            self.lake[i] = u16::from_le_bytes([b[20], b[21]]);
            self.feature[i] = u16::from_le_bytes([b[22], b[23]]);
            self.deposits[i] = [b[24], b[25], b[26], b[27]];
            self.clim[i] = [b[28] as i8, b[29] as i8];
            self.hollow[i] = b[30];
            self.sea_warm[i] = b[31] as i8;
        }
        Ok(())
    }

    /// The side tables in their saved encoding: each table's length, then its rows, every field little-endian and
    /// fixed-width, floats as their bits with −0 as +0 (A14.3).
    pub fn encode_tables(&self) -> Vec<u8> {
        let mut e = Enc::default();
        e.u32(self.rivers.len() as u32);
        for r in &self.rivers {
            e.u32(r.cell.0);
            e.u32(r.stretch);
            for v in [r.entry, r.exit].iter().flatten() {
                e.i32(*v);
            }
            e.f32(r.level_m);
            e.f32(r.width_m);
            e.f32(r.depth_m);
        }
        e.u32(self.stretches.len() as u32);
        for s in &self.stretches {
            e.u32(s.first);
            e.u32(s.cells);
            e.f32(s.length_m);
            e.f32(s.flow_m3s);
            e.f32(s.drainage_km2);
            e.u32(s.down);
        }
        e.u32(self.cave_records.len() as u32);
        for c in &self.cave_records {
            e.u32(c.cell.0);
            e.i32(c.mouth[0]);
            e.i32(c.mouth[1]);
            e.0.extend([c.kind as u8, c.depth_m, u8::from(c.dry), u8::from(c.first_camp)]);
        }
        e.u32(self.springs.len() as u32);
        for s in &self.springs {
            e.u32(s.cell.0);
            e.i32(s.at[0]);
            e.i32(s.at[1]);
            e.0.push(s.kind as u8);
        }
        e.u32(self.escarpments.len() as u32);
        for x in &self.escarpments {
            e.0.push(side_dir(x.facing));
            e.i32(x.at);
            e.i32(x.span[0]);
            e.i32(x.span[1]);
            e.u32(x.breaks.len() as u32);
            for b in &x.breaks {
                e.i32(b[0]);
                e.i32(b[1]);
            }
            e.u32(x.gaps.len() as u32);
            for &g in &x.gaps {
                e.i32(g);
            }
            e.f32(x.height_m);
        }
        e.0
    }

    /// Bytes held on the heap.
    pub fn heap_bytes(&self) -> usize {
        use std::mem::size_of_val;
        let cols = [
            size_of_val(self.height.as_slice()),
            size_of_val(self.rough.as_slice()),
            size_of_val(self.rock.as_slice()),
            size_of_val(self.layer_m.as_slice()),
            size_of_val(self.geo.as_slice()),
            size_of_val(self.cliff.as_slice()),
            size_of_val(self.caves.as_slice()),
            size_of_val(self.soil.as_slice()),
            size_of_val(self.soil_base.as_slice()),
            size_of_val(self.biome.as_slice()),
            size_of_val(self.water.as_slice()),
            size_of_val(self.flow.as_slice()),
            size_of_val(self.river.as_slice()),
            size_of_val(self.lake.as_slice()),
            size_of_val(self.feature.as_slice()),
            size_of_val(self.deposits.as_slice()),
            size_of_val(self.clim.as_slice()),
            size_of_val(self.hollow.as_slice()),
            size_of_val(self.sea_warm.as_slice()),
        ];
        let lines: usize = self
            .escarpments
            .iter()
            .map(|x| size_of_val(x) + size_of_val(x.breaks.as_slice()) + size_of_val(x.gaps.as_slice()))
            .sum();
        cols.iter().sum::<usize>()
            + size_of_val(self.rivers.as_slice())
            + size_of_val(self.stretches.as_slice())
            + size_of_val(self.cave_records.as_slice())
            + size_of_val(self.springs.as_slice())
            + lines
    }
}

/// Little-endian fixed-width writing (A14.3).
#[derive(Default)]
struct Enc(Vec<u8>);

impl Enc {
    fn u32(&mut self, v: u32) {
        self.0.extend(v.to_le_bytes());
    }
    fn i32(&mut self, v: i32) {
        self.0.extend(v.to_le_bytes());
    }
    fn f32(&mut self, v: f32) {
        self.0.extend(num::clean(v).to_bits().to_le_bytes());
    }
}

/// The changing state (A5.2), 34 bytes a cell.
#[derive(Clone, Debug, PartialEq)]
pub struct CellState {
    /// Trees, bushes, grass and herbs, reeds, bare, in 255ths summing to 255; the trees' mean age, years.
    pub cover: Vec<[u8; 5]>,
    pub tree_age: Vec<u8>,
    /// Game years since fire or flood; degree-days this year; shares of the season's food people took, by group.
    pub since: Vec<u8>,
    pub warmth: Vec<u16>,
    pub taken: Vec<[u8; 5]>,
    /// Dryness of litter and logs (4 bits each); snow in 2 cm steps; ice in cm.
    pub dry: Vec<u8>,
    pub snow: Vec<u8>,
    pub ice: Vec<u8>,
    /// Soil water's share, the ground water store, and a stream's flow (A5.10).
    pub soil_w: Vec<u8>,
    pub ground_w: Vec<u8>,
    pub stream: Vec<u8>,
    /// Fertility now (0–5 in 40ths), ash and fresh silt.
    pub fertility: Vec<u8>,
    pub ash: Vec<u8>,
    /// The burning list's row plus one; the first herd here (A7); small animals on the ground, in the air and in the
    /// water (A7).
    pub fire: Vec<u16>,
    pub herds: Vec<u32>,
    pub small: Vec<[u16; 3]>,
}

impl Default for CellState {
    fn default() -> CellState {
        CellState::new()
    }
}

impl CellState {
    /// Every cell bare and quiet.
    pub fn new() -> CellState {
        CellState {
            cover: vec![[0, 0, 0, 0, 255]; CELLS],
            tree_age: vec![0; CELLS],
            since: vec![0; CELLS],
            warmth: vec![0; CELLS],
            taken: vec![[0; 5]; CELLS],
            dry: vec![0; CELLS],
            snow: vec![0; CELLS],
            ice: vec![0; CELLS],
            soil_w: vec![0; CELLS],
            ground_w: vec![0; CELLS],
            stream: vec![0; CELLS],
            fertility: vec![0; CELLS],
            ash: vec![0; CELLS],
            fire: vec![0; CELLS],
            herds: vec![0; CELLS],
            small: vec![[0; 3]; CELLS],
        }
    }

    /// Bytes held on the heap.
    pub fn heap_bytes(&self) -> usize {
        use std::mem::size_of_val;
        [
            size_of_val(self.cover.as_slice()),
            size_of_val(self.tree_age.as_slice()),
            size_of_val(self.since.as_slice()),
            size_of_val(self.warmth.as_slice()),
            size_of_val(self.taken.as_slice()),
            size_of_val(self.dry.as_slice()),
            size_of_val(self.snow.as_slice()),
            size_of_val(self.ice.as_slice()),
            size_of_val(self.soil_w.as_slice()),
            size_of_val(self.ground_w.as_slice()),
            size_of_val(self.stream.as_slice()),
            size_of_val(self.fertility.as_slice()),
            size_of_val(self.ash.as_slice()),
            size_of_val(self.fire.as_slice()),
            size_of_val(self.herds.as_slice()),
            size_of_val(self.small.as_slice()),
        ]
        .iter()
        .sum()
    }
}

/// The world's cells: the shared fixed layers and the changing state.
#[derive(Clone, Debug, Default, PartialEq)]
pub struct WorldCells {
    pub fixed: Arc<FixedCells>,
    pub state: CellState,
}

/// The cells within a distance of a place, row by row from the north-west (`WorldCells::within`).
#[derive(Clone, Debug)]
pub struct CellsWithin {
    centre: Pos,
    r_m: f32,
    cols: [i32; 2],
    rows: [i32; 2],
    next: [i32; 2],
}

impl Iterator for CellsWithin {
    type Item = CellIx;

    fn next(&mut self) -> Option<CellIx> {
        while self.next[1] <= self.rows[1] {
            let [x, y] = self.next;
            if x < self.cols[1] {
                self.next[0] += 1;
            } else {
                self.next = [self.cols[0], y + 1];
            }
            let c = CellIx::at(x.rem_euclid(CELLS_X as i32) as u32, y as u32);
            if gap_to_cell_m(self.centre, c) <= self.r_m {
                return Some(c);
            }
        }
        None
    }
}

/// The shortest distance in metres from a place to any point of a cell, east and west the short way; north and
/// south never across the seam.
fn gap_to_cell_m(p: Pos, c: CellIx) -> f32 {
    let mid = c.centre();
    let half = CELL_TICKS / 2;
    let dx = (wrap(mid.x - p.x, W).abs() - half).max(0);
    let dy = ((mid.y - p.y).abs() - half).max(0);
    let (dx, dy) = (dx as f32 / TICKS_PER_M as f32, dy as f32 / TICKS_PER_M as f32);
    (dx * dx + dy * dy).sqrt()
}

impl WorldCells {
    /// Every cell void, and its state bare.
    pub fn void() -> WorldCells {
        WorldCells::default()
    }

    /// The cells any part of which lies within `r_m` metres of `p`: across the east–west wrap, never across the
    /// seam (A3.7), row by row from the north-west.
    pub fn within(&self, p: Pos, r_m: f32) -> CellsWithin {
        let reach = (num::max(r_m, 0.0) * TICKS_PER_M as f32) as i64;
        let cell = i64::from(CELL_TICKS);
        let (x, y) = (i64::from(p.x), i64::from(p.y));
        let cols = [
            ((x - reach).div_euclid(cell)) as i32,
            ((x + reach).div_euclid(cell)) as i32,
        ];
        // A whole lap at most, so no cell comes twice.
        let cols = if cols[1] - cols[0] >= CELLS_X as i32 {
            [0, CELLS_X as i32 - 1]
        } else {
            cols
        };
        let rows = [
            ((y - reach).div_euclid(cell)).max(0) as i32,
            ((y + reach).div_euclid(cell)).min(i64::from(CELLS_Y) - 1) as i32,
        ];
        CellsWithin {
            centre: p,
            r_m,
            cols,
            rows,
            next: [cols[0], rows[0]],
        }
    }

    /// The ground's height at a place for walks (A5.12): the four nearest cells' heights blended by their middles,
    /// with each escarpment's step put back sharp along its face and as a slope through its gaps; at the poles'
    /// rows the blend holds the nearest row, never reaching across the seam.
    pub fn surface_z(&self, p: Pos) -> f32 {
        let f = &self.fixed;
        let half = CELL_TICKS / 2;
        let (ux, uy) = (i64::from(p.x) - i64::from(half), i64::from(p.y) - i64::from(half));
        let cell = i64::from(CELL_TICKS);
        let (cx, cy) = (ux.div_euclid(cell), uy.div_euclid(cell));
        // Below 2^18, so exact in a float.
        let tx = ux.rem_euclid(cell) as f32 / CELL_TICKS as f32;
        let ty = uy.rem_euclid(cell) as f32 / CELL_TICKS as f32;
        let at = |dx: i64, dy: i64| {
            let col = (cx + dx).rem_euclid(i64::from(CELLS_X)) as u32;
            let row = (cy + dy).clamp(0, i64::from(CELLS_Y) - 1) as u32;
            CellIx::at(col, row)
        };
        let corners = [
            (at(0, 0), (1.0 - tx) * (1.0 - ty)),
            (at(1, 0), tx * (1.0 - ty)),
            (at(0, 1), (1.0 - tx) * ty),
            (at(1, 1), tx * ty),
        ];
        let mut z = 0.0;
        for &(c, w) in &corners {
            z += w * f32::from(f.height[c.0 as usize]);
        }
        z + self.step_correction(p, &corners)
    }

    /// What a blend of cell heights with these weights needs added at `p` so each escarpment's step stands sharp
    /// there: the step's share at `p`, less the blend of the shares the cells' heights hold, times its height and
    /// its sharpness along the line; nothing away from the lines.
    pub fn step_correction(&self, p: Pos, cells: &[(CellIx, f32)]) -> f32 {
        let mut out = 0.0;
        for e in &self.fixed.escarpments {
            let (d, along) = e.across_along([p.x, p.y]);
            if d.abs() > 3 * CELL_TICKS {
                continue;
            }
            let sharp = e.sharpness(along);
            if sharp <= 0.0 {
                continue;
            }
            // Measured from p, the short way, so a line near the date line still meets its cells.
            let mut held = 0.0;
            for &(c, w) in cells {
                let mid = c.centre();
                let near = [p.x + wrap(mid.x - p.x, W), mid.y];
                held += w * e.share_at(near);
            }
            out += e.height_m * sharp * (e.step_at([p.x, p.y]) - held);
        }
        out
    }

    /// Bytes the cells hold on the heap: about 132 MB for the whole grid with its side tables (A5.2, A16.4).
    pub fn heap_bytes(&self) -> usize {
        self.fixed.heap_bytes() + self.state.heap_bytes()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_core::geo::{H, Vec2, dist, offset};

    fn cells_of(w: &WorldCells, p: Pos, r: f32) -> Vec<CellIx> {
        w.within(p, r).collect()
    }

    // checks: WLD-12 WLD-01
    #[test]
    fn within_wraps_but_never_across_the_seam() {
        let w = WorldCells::void();
        // In the open: a cell's own middle with radius 0 is that cell alone; 1.6 km reaches its 8 neighbours and
        // the 4 cells two away east, west, north and south, whose near edges lie 1,536 m off, and no other.
        let c = CellIx::at(1000, 244);
        assert_eq!(cells_of(&w, c.centre(), 0.0), vec![c]);
        let near = cells_of(&w, c.centre(), 1_600.0);
        assert_eq!(near.len(), 13, "{near:?}");
        assert!(c.neighbours8().iter().all(|n| near.contains(n)));
        assert!(near.contains(&CellIx::at(1002, 244)) && !near.contains(&CellIx::at(1002, 245)));
        // Every cell found lies within the distance, and every cell within it is found, by brute force.
        let p = offset(c.centre(), Vec2::new(300.0, -200.0));
        let found = cells_of(&w, p, 2_600.0);
        for cy in 240..249 {
            for cx in 995..1006 {
                let k = CellIx::at(cx, cy);
                assert_eq!(found.contains(&k), gap_to_cell_m(p, k) <= 2_600.0, "{cx}, {cy}");
            }
        }
        // Across the east–west wrap: near x = 0 the cells at the far east come in, in row order.
        let edge = Pos {
            x: 100 * TICKS_PER_M,
            y: c.centre().y,
            z: 0,
        };
        let wrapped = cells_of(&w, edge, 1_200.0);
        assert!(wrapped.contains(&CellIx::at(CELLS_X - 1, 244)) && wrapped.contains(&CellIx::at(1, 244)));
        // Never across the seam: by the north pole, rows from the south pole's side stay out.
        let pole = Pos {
            x: c.centre().x,
            y: 300 * TICKS_PER_M,
            z: 0,
        };
        let top = cells_of(&w, pole, 3_000.0);
        assert!(top.iter().all(|k| k.xy().1 <= 3), "{top:?}");
        assert!(top.contains(&CellIx::at(1000, 0)));
        let bottom = Pos { y: H - 100, ..pole };
        assert!(cells_of(&w, bottom, 3_000.0).iter().all(|k| k.xy().1 >= CELLS_Y - 4));
        // A circle wider than the world finds each cell of its rows once.
        let all = cells_of(&w, c.centre(), 2_100_000.0);
        let mut sorted = all.clone();
        sorted.sort();
        sorted.dedup();
        assert_eq!((all.len(), sorted.len()), (CELLS, CELLS));
        // The distance agrees with the world's own.
        assert!((gap_to_cell_m(p, c) - 0.0).abs() < 1e-6 && dist(p, c.centre()) > 0.0);
    }

    /// Cells round (1000, 244) sloping gently east, an escarpment on row 236 facing south, 30 m high from column
    /// 990 to 1010 with a break round column 1000 and a gap at column 995's middle.
    fn cliff_world() -> WorldCells {
        let mut f = FixedCells::void();
        let at = CellIx::at(1000, 236).centre().y;
        for cy in 228..246 {
            for cx in 985..1016 {
                let c = CellIx::at(cx, cy);
                let base = 200.0 + (cx as f32 - 1000.0) * 2.0;
                let share = match cy.cmp(&236) {
                    std::cmp::Ordering::Less => 1.0,
                    std::cmp::Ordering::Equal => 0.5,
                    std::cmp::Ordering::Greater => 0.0,
                };
                f.height[c.0 as usize] = (base + 30.0 * share).round() as i16;
                f.water[c.0 as usize] = 0;
            }
        }
        let x_of = |cx: u32| CellIx::at(cx, 236).centre().x;
        f.escarpments.push(EscarpmentLine {
            facing: Side::South,
            at,
            span: [x_of(990), x_of(1010)],
            breaks: vec![[x_of(1000) - 500 * TICKS_PER_M, x_of(1000) + 500 * TICKS_PER_M]],
            gaps: vec![x_of(995)],
            height_m: 30.0,
        });
        WorldCells {
            fixed: Arc::new(f),
            state: CellState::new(),
        }
    }

    // checks: WLD-12 WLD-34
    #[test]
    fn surface_steps_at_the_escarpment() {
        let w = cliff_world();
        let line = CellIx::at(1003, 236).centre();
        let at = |dx: f32, dy: f32| w.surface_z(offset(line, Vec2::new(dx, dy)));
        // At cells' middles away from the line the surface is their height.
        let mid = CellIx::at(1005, 240);
        assert_eq!(w.surface_z(mid.centre()), f32::from(w.fixed.height[mid.0 as usize]));
        // Across the face: the full 30 m within a metre, the slope of the land either side.
        let (north, south) = (at(0.0, -1.0), at(0.0, 1.0));
        assert!((north - south - 30.0).abs() < 0.2, "{north} {south}");
        // Away from the line the ground runs smooth: no step within 100 m steps north or south.
        for k in 1..20 {
            let (a, b) = (at(0.0, -50.0 * k as f32), at(0.0, -50.0 * (k + 1) as f32));
            assert!((a - b).abs() < 1.0, "{k}: {a} {b}");
            let (a, b) = (at(0.0, 50.0 * k as f32), at(0.0, 50.0 * (k + 1) as f32));
            assert!((a - b).abs() < 1.0, "{k}: {a} {b}");
        }
        // Through a gap the step becomes a slope people can walk, under 35°.
        let gap = CellIx::at(995, 236).centre();
        let g = |dy: f32| w.surface_z(offset(gap, Vec2::new(0.0, dy)));
        for k in -30..30 {
            let rise = g(k as f32 - 0.5) - g(k as f32 + 0.5);
            assert!(rise < 0.71, "{k}: {rise}");
        }
        assert!((g(-30.0) - g(30.0) - 30.0).abs() < 1.0);
        // In the break the blend alone holds, so the valley crosses the line with no face.
        let brk = CellIx::at(1000, 236).centre();
        let b = |dy: f32| w.surface_z(offset(brk, Vec2::new(0.0, dy)));
        assert!((b(-1.0) - b(1.0)).abs() < 0.2);
        // Past the face's ends the step fades into the blend, never a wall.
        let end = CellIx::at(1012, 236).centre();
        let e = |dy: f32| w.surface_z(offset(end, Vec2::new(0.0, dy)));
        assert!((e(-1.0) - e(1.0)).abs() < 0.2);
        // East and west along the face, no seam: neighbouring points differ by the land's gentle slope alone.
        for k in -400..400 {
            let x = 10.0 * k as f32;
            let (a, b) = (at(x, -3.0), at(x + 10.0, -3.0));
            assert!((a - b).abs() < 2.0, "{x}: {a} {b}");
        }
    }

    // checks: WLD-12 PLT-09
    #[test]
    fn columns_round_trip() {
        // Every column of every cell comes back as written, through its saved encoding.
        let mut f = FixedCells::void();
        let cells: Vec<CellIx> = (0..40u32).map(|k| CellIx::at(990 + k % 20, 240 + k / 20)).collect();
        for (k, c) in cells.iter().enumerate() {
            let i = c.0 as usize;
            let k8 = k as u8;
            f.height[i] = -300 + 37 * k as i16;
            f.rough[i] = k8;
            f.rock[i] = [k8, k8 + 1, k8 + 2];
            f.layer_m[i] = [k8 + 3, k8 + 4];
            f.geo[i] = k8 % 5;
            f.cliff[i] = cliff_byte(k8 % 8, 2.0 * k as f32);
            f.caves[i] = k8 * 3;
            f.soil[i] = k8 + 5;
            f.soil_base[i] = 200 - k8;
            f.biome[i] = k8 + 7;
            f.water[i] = k8.wrapping_mul(7);
            f.flow[i] = k8 % 9;
            f.river[i] = 70_000 + k as u32;
            f.lake[i] = 2_000 + k as u16;
            f.feature[i] = 900 + k as u16;
            f.deposits[i] = [k8, 5, k8 + 1, 3];
            f.clim[i] = [-(k as i8), k as i8];
            f.hollow[i] = k8 * 5;
            f.sea_warm[i] = -3 + k as i8;
        }
        let bytes = f.encode_cells(&cells);
        assert_eq!(bytes.len(), 32 * cells.len());
        let mut back = FixedCells::void();
        back.decode_cells(&cells, &bytes).unwrap();
        assert_eq!(back, f);
        assert!(back.decode_cells(&cells, &bytes[1..]).is_err());
        // The cliff and caves bytes hold what they were given.
        assert_eq!(cliff_of(cliff_byte(6, 30.0)), Some((6, 30.0)));
        assert_eq!(cliff_of(0), None);
        assert_eq!(side_dir(Side::South), 6);
        let cave = CaveRecord {
            cell: CellIx(5),
            mouth: [0, 0],
            kind: CaveKind::Shelter,
            depth_m: 18,
            dry: true,
            first_camp: false,
        };
        let b = caves_byte(2, Some(&cave));
        assert_eq!(
            (b & 3, b >> 2 & 3, b >> 4 & 1, b >> 5),
            (2, CaveKind::Shelter as u8, 1, 4)
        );
        // The side tables' encoding changes with what they hold, and −0 writes as +0.
        let mut g = f.clone();
        let empty = g.encode_tables();
        g.rivers.push(RiverCell {
            cell: CellIx(9),
            stretch: 0,
            entry: [1, 2],
            exit: [3, 4],
            level_m: -0.0,
            width_m: 18.0,
            depth_m: 1.2,
        });
        let one = g.encode_tables();
        assert_ne!(one, empty);
        g.rivers[0].level_m = 0.0;
        assert_eq!(g.encode_tables(), one);
    }

    // checks: WLD-12 PLT-04
    #[test]
    fn heap_bytes_of_the_grid() {
        // 32 bytes of fixed layers and 34 of state a cell, 132 MB for the 2 million (A5.2, A16.4).
        let w = WorldCells::void();
        assert_eq!(w.heap_bytes(), CELLS * 66);
        assert_eq!(w.fixed.heap_bytes(), CELLS * 32);
        let x = cliff_world();
        assert!(x.heap_bytes() > CELLS * 66);
    }
}
