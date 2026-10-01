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

_To be filled in after the runs._

## Results

_To be filled in after the runs._

## Verdict

_To be filled in after the runs._

## Caveats

_To be filled in after the runs._

## How to re-run

_To be filled in after the runs._
