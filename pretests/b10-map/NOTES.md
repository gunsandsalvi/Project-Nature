# B10 pre-test: the wrap-around map

Throwaway test for block `B10` (serves `WLD-01`, `WLD-02`, `WLD-03`, `WLD-12`). Delete with the rest of `pretests/` once the architecture exists.

## The question

1. Which cell layout should the map use: square cells that nest as a quadtree, or six-sided cells (hexes) in a standard hierarchy?
2. How is the torus drawn as a globe (`WLD-02`), and how much does that drawing distort?

The world (`WLD-01`, `WLD-03`): a torus about 2,000 km east to west and 1,000 km pole to pole. The equator runs across the middle, and both poles lie on the north-south seam under a permanent ice cap that nothing crosses. Detail by scale (`WLD-12`): regions for climate, about 1 km for rivers and soils, a few hundred metres for plants and animals, 1 m near people.

## The approaches

- **Squares.** 4 children per cell (quadtree). 8 neighbours: 4 sides, 4 corners.
- **Hexes.** 6 neighbours, all the same distance away. Three standard hierarchies:
  - aperture 7 (7 children; the grid turns about 19° each level, alternating back, as in the H3 system),
  - aperture 4 (4 children; same orientation every level),
  - aperture 3 (3 children; turns 30° each level, alternating).

## Decision rule (written before any measurement)

1. **Must pass, or the layout is out.**
   - Wrapping: every wrap check passes, across the east-west seam and, for distance maths, the north-south seam. A path search with the ice cap blocked never crosses the seam.
   - Logical nesting: every fine cell has exactly one parent at every level, so totals rolled up equal the fine totals.
2. **Default: squares.**
3. **Hexes win only if all three hold:**
   - **(a) Accuracy.** Path-length error is measured two ways on ground with obstacles: raw grid paths, and the same paths after a simple any-angle smoothing. Hexes must cut the mean error by more than **X = 30%** (relative, say 1.0% against 1.5%) in at least one of the two, and must not be more than 30% worse in the other. Or: squares with smoothing stay above **Z = 3%** mean error while hexes with smoothing get below it.
   - **(b) Cost.** Hexes need less than **Y = 25%** extra time to visit all of a cell's neighbours and to roll fine data up, and no more memory per cell index.
   - **(c) Nesting.** A coarse hex differs from the union of its own children by less than **5%** of its area per level, so rollups need no fractional weights.
4. If neither layout clearly wins, squares stay: they are simpler.

Why these numbers:

- **X = 30%.** Smaller gaps vanish in the noise of the walking-speed model (`B12` checks travel times against real walking data, which vary by 10% or more between people and ground) and of how obstacles are drawn at a given cell size. A third less error is the smallest gain worth giving up exact square nesting.
- **Z = 3%.** A direction-dependent error under a third of that walking noise can't be seen. 3% of a 100 km climate-zone crossing (`WLD-03`) is 3 km, about 40 minutes of a four-day walk.
- **Y = 25%.** Neighbour visits and rollups sit in the inner loops of rivers, soils, plants and animals, run on the phone (`X1`, `B79`). Ordinary tuning can win back about 25%, so a smaller gap decides nothing; a larger one is a lasting cost on every step.
- **5% nesting.** Rolling up hands quantities between detail levels (`WLD-12`, `B08`). If 1 fine cell in 20 lands in a neighbouring coarse cell, that is about the blur a coarse cell has at its edges anyway. More than that, and coarse rain, plants and herds visibly sit in the wrong place, or every rollup needs area weights.

The globe (part B) has no rule: there is one natural mapping, described below with its distortion.

## Method

- Rust, no packages (`src/`); Python for the globe (`globe.py`). Raw output in `results.txt`.
- Test worlds at the 1 km level: squares 2048 × 1024 (cell 0.98 km); hexes 1862 × 1078 rows (spacing 1.07 km). Same cell area within 5%. The hex sizes are multiples of 49, so four aperture-7 levels wrap exactly.
- Hexes are coded as Eisenstein integers: each coarser level is the grid multiplied by one number (2+w for aperture 7, 2 for aperture 4, 1+w for aperture 3). The torus is stored as rows with a shift where the pole seam is crossed, which works at every level and every turn of the grid.
- **Wrap checks.** All 2 million cells have distinct, mutual neighbours. Path search over the wrapped grid equals the distance formula, from sources on both seams and corners. With a 3-row ice cap around the pole seam, no path crosses it. Distances in km are symmetric and obey the triangle rule (1 million triples). Every coarse cell has exactly its children, across the seams too, and rolled-up totals match.
- **Nesting.** 400,000 random points per case: how often the point's chain of parents ends in a coarse cell that doesn't contain the point.
- **Path error.** Open ground: 200,000 pairs 20 to 200 cells apart, and 200,000 random pairs on the whole torus. With obstacles: a 256 × 256-cell patch, a quarter blocked in blobs 5 to 20 cells across, 300 pairs 20 to 80 cells apart, three different patches. "True" length: the same search plus smoothing on squares 8 times finer. Smoothing: from each kept point, skip ahead along the path while the next point is in sight.
- **Timings.** One core, whole 1 km world. Median of 7 runs after a warm-up, spread is min to max, all under the shared lock.

## Results

**Must-pass checks.** Both layouts pass every wrap check and nest logically (each fine cell has one parent, totals kept). No failures.

**Nesting:** share of the area that rolls up into a coarse cell it isn't in.

| Layout | 1 level | 4 levels | 6 levels |
|---|---|---|---|
| Squares | 0% | 0% | 0% |
| Hex aperture 7 (H3-like) | 7.2% | 6.5% | n/a |
| Hex aperture 7 (fixed turn) | 7.1% | 8.3% | n/a |
| Hex aperture 4 | 37% | 30% | 30% |
| Hex aperture 3 | 44% | 75% | 79% |

Hexes can't nest exactly: 7 small hexes never make one big hex. Aperture 3 drifts with my simple tie-break; its one-level 44% is built in.

**Path error** with obstacles: mean over 900 pairs, three patches (95th percentile in brackets).

| Layout | Raw grid path | Smoothed |
|---|---|---|
| Squares, 8 neighbours | 6.3% (9.6%) | 1.03% (3.8%) |
| Hexes, 6 neighbours | 10.5% (15.5%) | 1.31% (4.9%) |
| Squares, 4 neighbours (reference) | 27.3% (41%) | 2.6% (9%) |

Open ground, raw: squares-8 mean 5.5%, worst 8.2%; hexes-6 mean 10.3%, worst 15.5%. These match the textbook values. Smoothing makes open-ground paths exactly straight (0%). On random pairs across the whole torus: squares 5.7%, hexes 10.7%.

**Time per cell** (spread in brackets).

| Operation | Squares | Hexes | Hex vs square |
|---|---|---|---|
| All neighbours, storage order, tuned loop | 0.74 ns (0.70-1.11) | 0.61 ns (0.59-0.84) | -18% |
| Same, plain loop | 4.27 ns (4.04-4.51) | 7.46 ns (5.15-7.83) | +75% |
| All neighbours of random cells | 17.1 ns (16.9-19.5) | 20.8 ns (20.5-21.0) | +21% |
| Neighbour indices only | 4.48 ns (4.44-4.76) | 5.48 ns (5.42-5.58) | +22% |
| Rollup, one level | 0.22 ns (0.20-0.42) | 1.00 ns (0.98-1.29) with a stored parent table | 4.5x |
| Rollup, to about 60 km | 0.28 ns (0.26-0.31) | 1.22 ns (1.20-1.26) with tables | 4.4x |
| Rollup, parent worked out each time | n/a | 19.7 ns (19.6-19.7) | 89x |

Per neighbour read, squares are faster in every case (tuned: 10.7 against 9.9 billion reads a second). Hex aperture 4 rollup, worked out each time: 4.7 ns.

**Memory per cell index:** equal. 4 bytes within one level of up to 4 billion cells (every level coarser than about 23 m here); 8 bytes for a full address down to 1 m (squares need 46 bits: 5 for the level, 21 east-west, 20 north-south). Hexes need the same bits for the same cell count. Their stored parent tables add 4 bytes per fine cell: 8 MB for the world at 1 km, 128 MB at 250 m.

**Fitting the world.** Squares fit the 2:1 torus exactly at every level (2048 × 1024 here; 2^21 × 2^20 at 1 m). Hexes can only come close (2,000 × 1,003 km here), and a hex hierarchy wraps only if the sides are multiples of 7 for every two levels. On this world 4 aperture-7 levels wrap; going from 1 m to 1,000 km would need sides that are multiples of 823,543 cells.

**Part B: the globe** (`WLD-02`). Map column x becomes longitude 360° × x / 2,000 km. Map row y becomes latitude 90° − 180° × y / 1,000 km, the same latitude the simulation uses (`WLD-04` already assumes this spacing). The globe radius is 318 km, so the equator keeps its length, and since the map is twice as wide as it is tall, every meridian does too. If each map row must be one circle of latitude at its own latitude, this mapping is the only choice, so the squeeze below can't be avoided.

| Latitude | East-west scale (= area scale) | Worst change of angle |
|---|---|---|
| 0° | 1.00 | 0° |
| 30° | 0.87 | 8° |
| 45° | 0.71 | 20° |
| 60° | 0.50 | 39° |
| 75° | 0.26 | 72° |
| 85° | 0.09 | 114° |

- North-south scale is 1 everywhere; only east-west shrinks, by cos(latitude).
- The third of the map beyond 60° (`WLD-04`) fills 13% of the globe. An ice cap 100 km wide either side of the seam is 20% of the map but 5% of the globe; 50 km wide, 10% and 1.2%.
- Distances read off the globe never exceed true ones. Median ratio 0.86 for random pairs; 0.92 when both points lie within 60° of the equator.
- East-west seam: continuous. Pole seam: each map edge collapses to a pole point, so neighbours across the seam appear at opposite poles, 1,000 km apart. Nothing crosses the ice (`WLD-01`), so nothing is ever seen to jump.
- Rejected: an equal-area globe. It would draw the 60° climate belt at 42°.
- The mockup draws an Earth-sized sphere (radius 6,371 km) straight from 3D noise, with ice beyond about 69°. It never reads the torus, so its squeeze is just the sphere's. The real globe should use this mapping instead.

## Verdict

**Squares in a quadtree.** Both pass the must-pass checks, and hexes fail all three conditions for switching:

- **(a) Accuracy:** hexes are worse, not better: 66% more error raw, 27% more after smoothing. Squares with smoothing average 1.0%, well under Z = 3%.
- **(b) Cost:** visiting neighbours is within 25% either way in tuned and random-access code (a plain loop makes hexes 75% slower). Rollups are 4.5 times slower even with a stored parent table, which also costs 4 bytes per fine cell.
- **(c) Nesting:** about 7% of the area per level at best (aperture 7), against the 5% limit. Squares: 0%.

Squares also fit the 2:1 world exactly at every scale. The quadtree levels can also serve as the globe's coarser picture levels: near a pole one screen pixel covers many map cells east-west, and squares can be averaged east-west only.

On the rule itself: neighbour cost depends a lot on loop style (−18% to +75% for hexes). The rule didn't say which loop, but rollups and nesting decide without it. The 5% nesting limit was read one-sided; read two-sided, hexes miss it by more.

For `B12`: grid paths on squares overestimate by about 6% (worst about 15%). Simple smoothing brings the mean to 1%, but 1 pair in 20 is still about 4% too long. Whole-map distance fields, which can't be smoothed, keep the 5.5% direction bias, up to 8% at 22.5°. Any-angle search, or a better distance solver, fixes that if `B12` needs it.

## Caveats

- Timings are from the cloud's x86 cores, not the phone (`B01`, `B79`). The ratios matter more than the absolute times.
- Single-threaded, with no hand-written vector code.
- The "true" path is itself a smoothed path on a grid 8 times finer, so the smoothed errors are approximate. Both layouts share it, so the comparison is fair.
- One kind of obstacle field (blobs), three patches. Per-patch means agreed within 0.3 points (raw) and 0.1 point (smoothed).
- Not measured: hexes stored in hierarchy order, with children side by side as H3 numbers them. That should make rollups about as fast as squares, but neighbour lookups much slower.
- The aperture 3 and 4 multi-level figures depend on how ties between equal parents are broken. One level is built in: 44% and 37%.
- `WLD-01` doesn't give the ice cap's width; the globe figures assume 50 or 100 km.

## How to re-run

```
cd pretests/b10-map
LOCK=<scratchpad>/cache/cpu.lock
export CARGO_TARGET_DIR=<scratchpad>/cache/b10-target
flock $LOCK cargo build --release
flock $LOCK $CARGO_TARGET_DIR/release/b10-map checks    # also: nesting, iso, iso-more, bench
python3 globe.py
```

CPU time: about 30 s in all, build included.
