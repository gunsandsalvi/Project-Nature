//! Drainage (A5.7 step 4's methods, A2.9): a priority-flood fill from the sea, so every land cell drains; each land
//! cell's receiver, its steepest neighbour downhill of eight on the filled surface; and the land draining through
//! each cell, summed from the highest cell down. Single-threaded, plain comparisons only (A3.2), so every target
//! drains the land the same way.
//!
//! Implements WLD-17, see A5.7 and A5.10: water runs downhill to the sea from every cell.

use std::cmp::{Ordering, Reverse};
use std::collections::BinaryHeap;

use kd_core::geo::NEIGHBOURS8;
use kd_core::num;

use crate::cells::{CELL_M, NONE};

/// What a cell is to the water: land drains, the sea takes it, void is left out.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Ground {
    Land,
    Sea,
    Void,
}

/// The fill's rise for each cell step from the cell it was reached from, metres (A5.7 step 4).
pub const FILL_STEP_M: f32 = 0.01;

/// A square of cells drained: heights filled, receivers and drainage areas, by cell row by row.
#[derive(Clone, Debug, PartialEq)]
pub struct Drainage {
    pub size: usize,
    /// The filled surface, metres: each land cell at least `FILL_STEP_M` above the cell the fill reached it from.
    pub filled: Vec<f32>,
    /// Each land cell's steepest neighbour downhill on the filled surface, as an index into `NEIGHBOURS8`; `NONE`
    /// for the sea and void.
    pub receiver: Vec<u8>,
    /// The land draining through each cell, its own included, km²; 0 for the sea and void.
    pub area_km2: Vec<f32>,
}

/// A queue entry: its filled height, then its index, lowest first.
#[derive(Clone, Copy, PartialEq)]
struct Entry(f32, usize);

impl Eq for Entry {}

impl Ord for Entry {
    fn cmp(&self, o: &Entry) -> Ordering {
        self.0.total_cmp(&o.0).then(self.1.cmp(&o.1))
    }
}

impl PartialOrd for Entry {
    fn partial_cmp(&self, o: &Entry) -> Option<Ordering> {
        Some(self.cmp(o))
    }
}

/// The neighbour of cell `i` a step `d` away in a square of `size`, if inside it.
fn step(i: usize, d: usize, size: usize) -> Option<usize> {
    let (x, y) = ((i % size) as i32, (i / size) as i32);
    let (dx, dy) = NEIGHBOURS8[d];
    let (nx, ny) = (x + dx, y + dy);
    (0..size as i32)
        .contains(&nx)
        .then_some(())
        .filter(|_| (0..size as i32).contains(&ny))
        .map(|_| ny as usize * size + nx as usize)
}

/// Drains a square of `size` × `size` cells, row by row, with their heights in metres.
pub fn drain(heights: &[f32], ground: &[Ground], size: usize) -> Drainage {
    let n = size * size;
    debug_assert!(heights.len() == n && ground.len() == n);
    let mut filled = heights.to_vec();
    let mut seen = vec![false; n];
    let mut queue = BinaryHeap::new();
    for i in 0..n {
        if ground[i] == Ground::Sea {
            seen[i] = true;
            queue.push(Reverse(Entry(filled[i], i)));
        }
    }
    while let Some(Reverse(Entry(h, i))) = queue.pop() {
        for d in 0..8 {
            let Some(j) = step(i, d, size) else { continue };
            if seen[j] || ground[j] != Ground::Land {
                continue;
            }
            seen[j] = true;
            filled[j] = num::max(filled[j], h + FILL_STEP_M);
            queue.push(Reverse(Entry(filled[j], j)));
        }
    }
    // Steepest descent by drop over distance, ties to the lower index.
    let diagonal = CELL_M * std::f32::consts::SQRT_2;
    let mut receiver = vec![NONE; n];
    for i in 0..n {
        if ground[i] != Ground::Land {
            continue;
        }
        let mut best: Option<(f32, usize, u8)> = None;
        for d in 0..8u8 {
            let Some(j) = step(i, usize::from(d), size) else {
                continue;
            };
            if ground[j] == Ground::Void {
                continue;
            }
            let run = if d % 2 == 0 { CELL_M } else { diagonal };
            let slope = (filled[i] - filled[j]) / run;
            if slope <= 0.0 {
                continue;
            }
            let better = best.is_none_or(|(s, k, _)| slope > s || (slope == s && j < k));
            if better {
                best = Some((slope, j, d));
            }
        }
        if let Some((_, _, d)) = best {
            receiver[i] = d;
        }
    }
    // Drainage from the highest cell down, ties by index.
    let cell_km2 = (CELL_M / 1_000.0) * (CELL_M / 1_000.0);
    let mut order: Vec<usize> = (0..n).filter(|&i| ground[i] == Ground::Land).collect();
    order.sort_by(|&a, &b| filled[b].total_cmp(&filled[a]).then(a.cmp(&b)));
    let mut area_km2 = vec![0.0f32; n];
    for &i in &order {
        area_km2[i] += cell_km2;
        if receiver[i] != NONE
            && let Some(j) = step(i, usize::from(receiver[i]), size)
            && ground[j] == Ground::Land
        {
            area_km2[j] += area_km2[i];
        }
    }
    Drainage {
        size,
        filled,
        receiver,
        area_km2,
    }
}

impl Drainage {
    /// The cell a land cell's water runs to next, if any.
    pub fn next(&self, i: usize) -> Option<usize> {
        (self.receiver[i] != NONE)
            .then(|| step(i, usize::from(self.receiver[i]), self.size))
            .flatten()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// A square island of noisy heights with pits and a closed basin, in its sea.
    fn island(size: usize) -> (Vec<f32>, Vec<Ground>) {
        let mut h = vec![0.0; size * size];
        let mut g = vec![Ground::Sea; size * size];
        for y in 0..size {
            for x in 0..size {
                let i = y * size + x;
                if x == 0 || y == 0 || x == size - 1 || y == size - 1 {
                    g[i] = Ground::Void;
                } else if (3..size - 3).contains(&x) && (3..size - 3).contains(&y) {
                    g[i] = Ground::Land;
                    let k = num::hash2(7, i as u64);
                    h[i] = 50.0 + (k % 97) as f32 - 0.1 * ((x as f32 - 20.0).abs() + (y as f32 - 20.0).abs());
                } else {
                    h[i] = -30.0;
                }
            }
        }
        // A closed basin in the middle: a ring of high cells round a deep pit.
        for y in 18..23 {
            for x in 18..23 {
                h[y * size + x] = if (19..22).contains(&x) && (19..22).contains(&y) {
                    5.0
                } else {
                    300.0
                };
            }
        }
        (h, g)
    }

    // checks: WLD-17
    #[test]
    fn all_land_drains_to_sea() {
        // No closed basin is left after the fill: every land cell's water reaches the sea, each step downhill on
        // the filled surface, with no cycle, and the areas add up to all the land.
        let size = 40;
        let (h, g) = island(size);
        let d = drain(&h, &g, size);
        let land: Vec<usize> = (0..size * size).filter(|&i| g[i] == Ground::Land).collect();
        for &i in &land {
            assert!(d.filled[i] >= h[i]);
            let mut at = i;
            let mut steps = 0;
            while g[at] == Ground::Land {
                let next = d.next(at).unwrap_or_else(|| panic!("cell {at} has no receiver"));
                assert!(d.filled[next] < d.filled[at], "{at} -> {next} is not downhill");
                at = next;
                steps += 1;
                assert!(steps < size * size, "a cycle from {i}");
            }
            assert_eq!(g[at], Ground::Sea);
        }
        // The pit was filled to spill over its ring.
        assert!(d.filled[20 * size + 20] > 300.0);
        // Everything that drains reaches the sea: the cells next to the sea carry all the land's area.
        let cell = (CELL_M / 1_000.0) * (CELL_M / 1_000.0);
        let into_sea: f32 = land
            .iter()
            .filter(|&&i| d.next(i).is_some_and(|j| g[j] == Ground::Sea))
            .map(|&i| d.area_km2[i])
            .sum();
        assert!(
            (into_sea - land.len() as f32 * cell).abs() < 0.01 * into_sea,
            "{into_sea}"
        );
        // The same heights drain the same way every time.
        assert_eq!(drain(&h, &g, size), d);
    }
}
