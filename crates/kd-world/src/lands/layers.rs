//! The second half of a land preset's making (A5.6): from its heights, the water (drainage, the river's cells and
//! stretches, streams and springs), the soils, the deposits, the cover and biomes, and the climate. Soils and
//! biomes come from the catalogue by the kind of ground they form on (`Landform`), deposits by the rock they lie in
//! or the rivers, so the rules name no entry (A2.3 rule 5).
//!
//! Implements WLD-17, WLD-27, WLD-14 and WLD-16, see A5.7 steps 5, 8, 9 and 10, A5.10, A5.11 and A5.8: water, soils,
//! deposits and climate set by the land's own rules.

use std::collections::BTreeMap;

use kd_core::chance::{Stream, moment};
use kd_core::geo::{CellIx, TICKS_PER_M};
use kd_core::ids::{PlaceKind, Uid};
use kd_core::time::GameTime;
use kd_core::{m, num};
use kd_data::Catalogue;
use kd_data::world::{COVER_GROUPS, Land, Landform, shares_255};

use super::{Frame, LandTuning, ValleyLine, Window};
use crate::cells::{CELL_M, CELL_TICKS, CellState, FixedCells, NONE, Spring, SpringKind, water};
use crate::climate::{ClimateMap, ClimateRecord};
use crate::r#gen::drain::{self, Ground};
use crate::purposes;
use crate::water::{RiverSpec, river_cells};

/// What the first half made, for the second.
pub(super) struct Made<'a> {
    pub p: &'a Land,
    pub seed: u64,
    pub t: &'a LandTuning,
    pub cat: &'a Catalogue,
    pub window: Window,
    pub frame: Frame,
    pub valley: Option<&'a ValleyLine>,
    pub river: &'a [[i32; 2]],
    pub river_level: &'a [f32],
}

/// The cover groups by their place in a cell's shares (A5.2).
const TREES: usize = 0;
const GRASS: usize = 2;
const REEDS: usize = 3;
const BARE: usize = 4;

fn uid(c: CellIx) -> u64 {
    Uid::place(PlaceKind::Cell, u64::from(c.0)).0
}

/// Water, soils, deposits, cover and biomes into the cells, and the climate of each weather cell over them.
pub(super) fn water_soils_cover(m: &Made, f: &mut FixedCells, st: &mut CellState) -> Result<ClimateMap, String> {
    let (p, t) = (m.p, m.t);
    let size = m.window.size as usize;
    let cells: Vec<CellIx> = m.window.cells().collect();
    let ix = |c: CellIx| c.0 as usize;

    // Drainage over the square (A5.7 step 4's fill, receivers and areas).
    let heights: Vec<f32> = cells.iter().map(|&c| f32::from(f.height[ix(c)])).collect();
    let ground: Vec<Ground> = cells
        .iter()
        .map(|&c| {
            let w = f.water[ix(c)];
            if w & water::VOID != 0 {
                Ground::Void
            } else if w & water::SEA != 0 {
                Ground::Sea
            } else {
                Ground::Land
            }
        })
        .collect();
    let d = drain::drain(&heights, &ground, size);
    // Closed hollows filled to where they spill, so the land holds no lake until lakes come (A5.10, `MIL-04`).
    let mut heights = heights;
    for (k, &c) in cells.iter().enumerate() {
        f.flow[ix(c)] = d.receiver[k];
        if ground[k] == Ground::Land && d.filled[k] > heights[k] + 0.5 {
            heights[k] = d.filled[k].round();
            f.height[ix(c)] = heights[k] as i16;
        }
    }

    // The river: the valley's line, its cells, its rating's width and depth at its flow, its stretches.
    if p.valley.flow_m3s > 0.0 && m.river.len() > 1 {
        let spec = RiverSpec {
            line: m.river,
            level: m.river_level,
            width_m: p.valley.width_m,
            depth_m: t.bankfull_depth_m,
            flow_m3s: p.valley.flow_m3s,
            stretch_max_m: t.stretch_max_m,
        };
        let (rivers, stretches) = river_cells(&spec, f)?;
        for (k, r) in rivers.iter().enumerate() {
            f.water[ix(r.cell)] |= water::RIVER;
            f.river[ix(r.cell)] = k as u32;
        }
        f.rivers = rivers;
        f.stretches = stretches;
    }

    // Streams: land off the river draining `stream_min_km2` or more, its own included, where a year's rain reaches
    // `stream_min_rain_m`.
    let year_rain: f32 = p.climate.rain_m.iter().fold(0.0, |a, &r| a + r);
    if year_rain >= t.stream_min_rain_m {
        for (k, &c) in cells.iter().enumerate() {
            let i = ix(c);
            if ground[k] == Ground::Land && f.water[i] & water::RIVER == 0 && d.area_km2[k] >= t.stream_min_km2 {
                f.water[i] |= water::STREAM;
                f.river[i] = d.area_km2[k].round() as u32;
            }
        }
    }

    // Springs: out from the escarpment's foot below each cell of its face with a cave or shelter, and where each
    // stream starts.
    let mut springs = Vec::new();
    if let Some(l) = f.escarpments.first().cloned() {
        let place = Stream::new(m.seed, &purposes::SPRING_PLACE);
        let east_west = matches!(l.facing, kd_data::world::Side::North | kd_data::world::Side::South);
        let out = (t.spring_foot_out_m * TICKS_PER_M as f32) as i32;
        let low = match l.facing {
            kd_data::world::Side::South | kd_data::world::Side::East => out,
            kd_data::world::Side::North | kd_data::world::Side::West => -out,
        };
        for &c in &cells {
            let i = ix(c);
            if f.caves[i] & 3 == 0 || f.cliff[i] == 0 {
                continue;
            }
            let o = c.origin();
            for n in 0..t.springs_per_cave_cell {
                let u = place.unit(uid(c), moment(GameTime(0), n as u16));
                let along = (if east_west { o.x } else { o.y }) + (u * CELL_TICKS as f32) as i32;
                let at = if east_west {
                    [along, l.at + low]
                } else {
                    [l.at + low, along]
                };
                springs.push(Spring {
                    cell: c,
                    at,
                    kind: SpringKind::CliffFoot,
                });
            }
            f.water[i] |= water::SPRING;
        }
    }
    for (k, &c) in cells.iter().enumerate() {
        let i = ix(c);
        if f.water[i] & water::STREAM == 0 {
            continue;
        }
        let fed = c.neighbours8().iter().any(|&nb| {
            m.window.contains(nb) && f.water[ix(nb)] & water::STREAM != 0 && {
                let (x, y) = nb.xy();
                let j = (y - m.window.y0) as usize * size + (x - m.window.x0) as usize;
                d.next(j) == Some(k)
            }
        });
        if !fed {
            let mid = c.centre();
            springs.push(Spring {
                cell: c,
                at: [mid.x, mid.y],
                kind: SpringKind::StreamHead,
            });
            f.water[i] |= water::SPRING;
        }
    }
    springs.sort_by_key(|s| (s.cell, s.at));
    f.springs = springs;

    // The floodplain's cells: the river's, and those its floodplain reaches.
    let reach = 0.5 * (p.valley.floodplain_m + CELL_M);
    let floodplain: Vec<bool> = cells
        .iter()
        .map(|&c| {
            f.water[ix(c)] & water::RIVER != 0
                || m.valley
                    .and_then(|v| v.nearest(m.frame.local(c)))
                    .is_some_and(|(dist, _)| dist <= reach)
        })
        .collect();
    let line = f.escarpments.first().cloned();
    let on_face: Vec<bool> = f.cliff.iter().map(|&b| b != 0).collect();
    let face = |i: usize| on_face[i];

    // Soils (A5.11): the land's own, but where one forms on the floodplain, the escarpment's face and foot, or its
    // high side; fertility from the land's base by the soil's shift.
    let soil_on = |lf: Landform| m.cat.soils.iter().find(|s| s.landform == Some(lf));
    let own_soil = m
        .cat
        .soils
        .iter()
        .find(|s| s.number == p.soil)
        .ok_or_else(|| format!("land {}: its soil is not in the catalogue", p.id))?;
    for (k, &c) in cells.iter().enumerate() {
        let i = ix(c);
        if ground[k] != Ground::Land {
            continue;
        }
        let mid = c.centre();
        let lf = if floodplain[k] {
            Some(Landform::Floodplain)
        } else if face(i) {
            Some(Landform::ScarpFoot)
        } else if line.as_ref().is_some_and(|l| l.share_at([mid.x, mid.y]) == 1.0) {
            Some(Landform::ScarpTop)
        } else {
            None
        };
        let soil = lf.and_then(soil_on).unwrap_or(own_soil);
        f.soil[i] = soil.number.min(254) as u8;
        let fertility = (i32::from(p.fertility) + i32::from(soil.fertility_shift)).clamp(0, 5) as u8;
        f.soil_base[i] = fertility * 40;
        st.fertility[i] = fertility * 40;
    }

    // Deposits (A5.7 step 9): each in every cell whose rock column holds its rock, and in the river's cells; a
    // carried one's richness falls downstream of the escarpment, where the river leaves the high side.
    let rich = Stream::new(m.seed, &purposes::DEPOSIT_RICHNESS);
    let mut below = Vec::with_capacity(f.rivers.len());
    let mut run = 0.0f32;
    let mut past = false;
    for r in &f.rivers {
        let mid = r.cell.centre();
        past |= line.as_ref().is_none_or(|l| l.share_at([mid.x, mid.y]) < 1.0);
        below.push(if past { run } else { 0.0 });
        if past {
            let dx = (r.exit[0] - r.entry[0]) as f32 / TICKS_PER_M as f32;
            let dy = (r.exit[1] - r.entry[1]) as f32 / TICKS_PER_M as f32;
            run += (dx * dx + dy * dy).sqrt();
        }
    }
    let put = |f: &mut FixedCells, i: usize, kind: u8, v: u8| {
        let slot = &mut f.deposits[i];
        if slot[0] == 0 {
            slot[0] = kind;
            slot[1] = v;
        } else if slot[2] == 0 {
            slot[2] = kind;
            slot[3] = v;
        }
    };
    for dep in &m.cat.deposits {
        let kind = (dep.number + 1).min(255) as u8;
        let [lo, hi] = dep.richness;
        let keyed = |c: CellIx| lo + rich.below(uid(c), moment(GameTime(0), dep.number), u32::from(hi - lo) + 1) as u8;
        if let Some(r) = dep.rock {
            let r = r.min(254) as u8;
            for (k, &c) in cells.iter().enumerate() {
                if ground[k] == Ground::Land && f.rock[ix(c)].contains(&r) {
                    put(f, ix(c), kind, keyed(c));
                }
            }
        }
        if dep.rivers {
            for (k, &down) in below.iter().enumerate() {
                let c = f.rivers[k].cell;
                let v = match dep.carried {
                    Some(carried) => {
                        let share = m::powf(carried.keep, down / carried.every_m);
                        num::max(f32::from(lo), num::min(f32::from(hi), (f32::from(hi) * share).round())) as u8
                    }
                    None => keyed(c),
                };
                put(f, ix(c), kind, v);
            }
        }
    }

    // Cover (A5.2): the land's shares, with reeds on the river and floodplain, grass on the floodplain and bare
    // rock on the escarpment's face set, and the rest scaled to fill the cell; the trees' age keyed.
    let base: [f32; COVER_GROUPS] = p.cover.map(|v| f32::from(v) / 255.0);
    let ages = Stream::new(m.seed, &purposes::TREE_AGE);
    for (k, &c) in cells.iter().enumerate() {
        let i = ix(c);
        if ground[k] != Ground::Land {
            continue;
        }
        let mut set: [Option<f32>; COVER_GROUPS] = [None; COVER_GROUPS];
        if floodplain[k] {
            set[REEDS] = Some(t.river_reeds);
            set[GRASS] = Some(t.floodplain_grass);
        }
        if face(i) {
            set[BARE] = Some(t.face_bare);
        }
        let fixed: f32 = set.iter().flatten().fold(0.0, |a, &v| a + v);
        let rest: f32 = (0..COVER_GROUPS)
            .filter(|&g| set[g].is_none())
            .fold(0.0, |a, g| a + base[g]);
        let scale = if rest > 0.0 {
            num::max(0.0, 1.0 - fixed) / rest
        } else {
            0.0
        };
        let shares: [f32; COVER_GROUPS] = std::array::from_fn(|g| set[g].unwrap_or(base[g] * scale));
        let total: f32 = shares.iter().fold(0.0, |a, &v| a + v);
        st.cover[i] = shares_255(shares.map(|v| v / total));
        st.tree_age[i] = if st.cover[i][TREES] > 0 {
            ages.range(uid(c), moment(GameTime(0), 0), t.tree_age[0], t.tree_age[1])
                .round() as u8
        } else {
            0
        };
    }

    // Biomes (A5.7 step 10): the land's own, but marsh on a flat floodplain draining a wide land, shore by the sea,
    // and the sea's.
    let biome_on = |lf: Landform| {
        m.cat
            .biomes
            .iter()
            .find(|b| b.landform == Some(lf))
            .map(|b| b.number.min(254) as u8)
    };
    let flat = m::tan(t.wetland_slope_deg.to_radians());
    for (k, &c) in cells.iter().enumerate() {
        let i = ix(c);
        f.biome[i] = match ground[k] {
            Ground::Void => NONE,
            Ground::Sea => biome_on(Landform::Sea).unwrap_or(NONE),
            Ground::Land => {
                // The slope down to where its water runs.
                let slope = d.next(k).map_or(0.0, |j| {
                    let run = if d.receiver[k].is_multiple_of(2) {
                        CELL_M
                    } else {
                        CELL_M * std::f32::consts::SQRT_2
                    };
                    num::max(0.0, heights[k] - heights[j]) / run
                });
                let coast = c.neighbours8().iter().any(|nb| f.water[ix(*nb)] & water::SEA != 0);
                let wetland = floodplain[k] && d.area_km2[k] > t.wetland_km2 && slope < flat;
                let lf = if wetland {
                    Some(Landform::Wetland)
                } else if coast {
                    Some(Landform::Coast)
                } else {
                    None
                };
                lf.and_then(biome_on).unwrap_or(p.biome.min(254) as u8)
            }
        };
    }

    // The climate (A5.8): each weather cell over the island and its sea takes the land's, measured at its land's
    // mean height; each cell's warmth is set off from it by its own height, sea cells at the water's.
    let mut sums: BTreeMap<u16, (f32, u32)> = BTreeMap::new();
    for (k, &c) in cells.iter().enumerate() {
        if ground[k] == Ground::Void {
            continue;
        }
        let e = sums.entry(c.weather().0).or_insert((0.0, 0));
        if ground[k] == Ground::Land {
            e.0 += heights[k];
            e.1 += 1;
        }
    }
    let mut climate = ClimateMap::default();
    let cl = &p.climate;
    for (&w, &(sum, n)) in &sums {
        climate.records[usize::from(w)] = Some(ClimateRecord {
            mean_c: cl.mean_c,
            range_c: cl.range_c,
            rain_m: cl.rain_m,
            storm_days: cl.storm_days,
            thunder_days: cl.thunder_days,
            wind: cl.wind,
            ref_height_m: if n > 0 { sum / n as f32 } else { 0.0 },
        });
    }
    for (k, &c) in cells.iter().enumerate() {
        if ground[k] == Ground::Void {
            continue;
        }
        let r = climate.of(c.weather()).map_or(0.0, |r| r.ref_height_m);
        let off = -t.cooling_per_km_c * (num::max(0.0, heights[k]) - r) / 1_000.0;
        f.clim[ix(c)][0] = num::max(-128.0, num::min(127.0, (off / 0.25).round())) as i8;
    }
    Ok(climate)
}
