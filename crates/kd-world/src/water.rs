//! Fresh water's fixed layers (A5.10, A5.7 step 5): a river's cells from its line, each with the points where the
//! line enters and leaves it on its edges, worked out exactly in ticks so a cell's exit is the next cell's entry,
//! and the river's stretches. Until routing runs (`MIL-03`) a river holds its water at a fixed level.
//!
//! Implements WLD-17, see A5.10: rivers with fixed entry and exit points per cell, running on unbroken.

use kd_core::geo::{CELLS_X, CELLS_Y, CellIx};

use crate::cells::{CELL_TICKS, FixedCells, RiverCell, Stretch, TO_SEA, water};

/// What a river's cells are made from: its line in ticks from source to sea with the water's level at each point,
/// and its bankfull width and depth and its flow.
pub struct RiverSpec<'a> {
    pub line: &'a [[i32; 2]],
    pub level: &'a [f32],
    pub width_m: f32,
    pub depth_m: f32,
    pub flow_m3s: f32,
    /// The longest a stretch runs, metres (A5.7 step 5).
    pub stretch_max_m: f32,
}

/// One cell of the walk: where it is, where the line entered and left it, and the water's level there.
struct Visit {
    cell: [i64; 2],
    entry: [i32; 2],
    exit: [i32; 2],
    level_in: f32,
    level_out: f32,
}

fn cell_of(p: [i32; 2]) -> [i64; 2] {
    let c = i64::from(CELL_TICKS);
    [i64::from(p[0]).div_euclid(c), i64::from(p[1]).div_euclid(c)]
}

fn index(cell: [i64; 2]) -> Option<CellIx> {
    let (x, y) = (cell[0], cell[1]);
    ((0..i64::from(CELLS_X)).contains(&x) && (0..i64::from(CELLS_Y)).contains(&y))
        .then(|| CellIx::at(x as u32, y as u32))
}

/// The river's cells (A5.7 step 5): the line walked cell by cell, each crossing of a cell's edge worked out in whole
/// ticks and shared by the cell it leaves and the one it enters, up to the first sea cell; where the line comes
/// back into a cell it left, the cells between are dropped, so the cells form a chain, each touching the next.
/// Then the stretches, at most `stretch_max_m` long, each running into the next and the last into the sea.
pub fn river_cells(spec: &RiverSpec, f: &FixedCells) -> Result<(Vec<RiverCell>, Vec<Stretch>), String> {
    let line = spec.line;
    if line.len() < 2 {
        return Err("a river's line needs two points".into());
    }
    let is_sea = |cell: [i64; 2]| index(cell).is_none_or(|c| f.water[c.0 as usize] & water::SEA != 0);
    let start = cell_of(line[0]);
    if is_sea(start) {
        return Err("the river rises in the sea".into());
    }
    let mut visits = vec![Visit {
        cell: start,
        entry: line[0],
        exit: line[0],
        level_in: spec.level[0],
        level_out: spec.level[0],
    }];
    let c = i64::from(CELL_TICKS);
    let mut reached = false;
    'walk: for k in 0..line.len() - 1 {
        let (p, q) = (line[k], line[k + 1]);
        let d = [i64::from(q[0]) - i64::from(p[0]), i64::from(q[1]) - i64::from(p[1])];
        let (la, lb) = (spec.level[k], spec.level[k + 1]);
        loop {
            let cur = visits[visits.len() - 1].cell;
            // The edge ahead in each direction, as a share of the segment: num / den, each den above 0.
            let edge = |axis: usize| -> Option<(i128, i128)> {
                if d[axis] == 0 {
                    return None;
                }
                let b = if d[axis] > 0 {
                    (cur[axis] + 1) * c
                } else {
                    cur[axis] * c
                };
                let num = (i128::from(b) - i128::from(p[axis])) * i128::from(d[axis].signum());
                let den = i128::from(d[axis].abs());
                (num <= den).then_some((num, den))
            };
            let (ex, ey) = (edge(0), edge(1));
            let first = match (ex, ey) {
                (None, None) => break,
                (Some(_), None) => 0,
                (None, Some(_)) => 1,
                (Some((nx, dx)), Some((ny, dy))) => match (nx * dy).cmp(&(ny * dx)) {
                    std::cmp::Ordering::Less => 0,
                    std::cmp::Ordering::Greater => 1,
                    std::cmp::Ordering::Equal => 2,
                },
            };
            let (num, den) = match first {
                0 | 2 => ex.unwrap_or((0, 1)),
                _ => ey.unwrap_or((0, 1)),
            };
            let at = |axis: usize| -> i32 { (i128::from(p[axis]) + i128::from(d[axis]) * num / den) as i32 };
            let mut cross = [at(0), at(1)];
            let mut next = cur;
            if first != 1 {
                let b = if d[0] > 0 { (cur[0] + 1) * c } else { cur[0] * c };
                cross[0] = b as i32;
                next[0] += d[0].signum();
            }
            if first != 0 {
                let b = if d[1] > 0 { (cur[1] + 1) * c } else { cur[1] * c };
                cross[1] = b as i32;
                next[1] += d[1].signum();
            }
            let level = la + (lb - la) * (num as f32 / den as f32);
            let last = visits.len() - 1;
            visits[last].exit = cross;
            visits[last].level_out = level;
            if is_sea(next) {
                reached = true;
                break 'walk;
            }
            if let Some(j) = visits.iter().position(|v| v.cell == next) {
                visits.truncate(j + 1);
            } else {
                visits.push(Visit {
                    cell: next,
                    entry: cross,
                    exit: cross,
                    level_in: level,
                    level_out: level,
                });
            }
        }
    }
    if !reached {
        return Err("the river's line never reaches a sea cell".into());
    }
    // The cells and their stretches.
    let mut rivers = Vec::with_capacity(visits.len());
    let mut stretches: Vec<Stretch> = Vec::new();
    let mut run = f32::INFINITY;
    for v in &visits {
        let dx = (v.exit[0] - v.entry[0]) as f32 / kd_core::geo::TICKS_PER_M as f32;
        let dy = (v.exit[1] - v.entry[1]) as f32 / kd_core::geo::TICKS_PER_M as f32;
        let len = (dx * dx + dy * dy).sqrt();
        if run + len > spec.stretch_max_m {
            let next = stretches.len() as u32;
            if let Some(s) = stretches.last_mut() {
                s.down = next;
            }
            stretches.push(Stretch {
                first: rivers.len() as u32,
                cells: 0,
                length_m: 0.0,
                flow_m3s: spec.flow_m3s,
                drainage_km2: 0.0,
                down: TO_SEA,
            });
            run = 0.0;
        }
        run += len;
        let s = stretches.len() - 1;
        stretches[s].cells += 1;
        stretches[s].length_m += len;
        rivers.push(RiverCell {
            cell: index(v.cell).ok_or("the river leaves the world")?,
            stretch: s as u32,
            entry: v.entry,
            exit: v.exit,
            level_m: 0.5 * (v.level_in + v.level_out),
            width_m: spec.width_m,
            depth_m: spec.depth_m,
        });
    }
    Ok((rivers, stretches))
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_core::geo::TICKS_PER_M;

    /// Cells round (1000, 244) all land, a row of sea at row 250.
    fn land_to_sea() -> FixedCells {
        let mut f = FixedCells::void();
        for cy in 236..252 {
            for cx in 990..1010 {
                let c = CellIx::at(cx, cy);
                f.water[c.0 as usize] = if cy >= 250 { water::SEA } else { 0 };
            }
        }
        f
    }

    // checks: WLD-17 PRE-26
    #[test]
    fn river_chains_to_the_sea() {
        let f = land_to_sea();
        let m = TICKS_PER_M;
        let o = CellIx::at(1000, 240).origin();
        // A line wandering south-east, through a corner, back into a cell it left, and on into the sea.
        let line: Vec<[i32; 2]> = [
            [300, 300],
            [1_700, 900],
            [2_048, 1_024 * 2],
            [1_900, 3_100],
            [2_100, 3_000],
            [2_600, 6_000],
            [3_000, 11_000],
        ]
        .iter()
        .map(|&[x, y]| [o.x + x * m, o.y + y * m])
        .collect();
        let level: Vec<f32> = (0..line.len()).map(|k| 100.0 - 15.0 * k as f32).collect();
        let spec = RiverSpec {
            line: &line,
            level: &level,
            width_m: 18.0,
            depth_m: 1.2,
            flow_m3s: 8.0,
            stretch_max_m: 3_000.0,
        };
        let (rivers, stretches) = river_cells(&spec, &f).unwrap();
        // From its source, each cell's exit is the next one's entry, on the edge they share (or the corner), each
        // cell once; the last leaves into a sea cell.
        assert_eq!(rivers[0].entry, line[0]);
        for w in rivers.windows(2) {
            assert_eq!(w[0].exit, w[1].entry);
            let (a, b) = (w[0].cell.xy(), w[1].cell.xy());
            assert!(
                a.0.abs_diff(b.0) <= 1 && a.1.abs_diff(b.1) <= 1 && a != b,
                "{a:?} {b:?}"
            );
            let o = w[1].cell.origin();
            let e = w[1].entry;
            assert!(e[0] == o.x || e[0] == o.x + CELL_TICKS || e[1] == o.y || e[1] == o.y + CELL_TICKS);
        }
        let mut cells: Vec<CellIx> = rivers.iter().map(|r| r.cell).collect();
        cells.sort();
        cells.dedup();
        assert_eq!(cells.len(), rivers.len());
        let last = rivers.last().unwrap();
        let below = CellIx::of(kd_core::geo::Pos {
            x: last.exit[0],
            y: last.exit[1] + 1,
            z: 0,
        });
        assert!(f.water[below.0 as usize] & water::SEA != 0);
        // The water falls toward the sea, and the stretches cover the cells in order, each at most 3 km and
        // running into the next.
        assert!(rivers.windows(2).all(|w| w[1].level_m <= w[0].level_m));
        assert_eq!(stretches.iter().map(|s| s.cells).sum::<u32>() as usize, rivers.len());
        for (k, s) in stretches.iter().enumerate() {
            assert!(s.length_m <= 3_000.0 + 1_500.0);
            assert_eq!(s.down, if k + 1 < stretches.len() { k as u32 + 1 } else { TO_SEA });
        }
        // A line that never reaches the sea, or rises in it, is refused.
        let dry = RiverSpec {
            line: &line[..3],
            ..spec
        };
        assert!(river_cells(&dry, &f).is_err());
    }
}
