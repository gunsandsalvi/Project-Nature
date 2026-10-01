# B04 and B11 pre-test: storing data, and terrain

Throwaway test for blocks `B04` (storing data: `PRN-15`, `PLT-07`, `PLT-10`) and `B11` (terrain: `PRE-23`, `PRE-24`, `WLD-11`, `WLD-12`, on the `WLD-03` torus). Builds on `B10` (square cells in a quadtree) and `B80` (temp file, flush, rename, hash). Delete with the rest of `pretests/`.

## The question

- **B04:** how to hold the world in memory, and how to keep saved moments and the history log on the phone's storage.
- **B11:** how to store land from whole regions down to the metre, with cliffs, caves and overhangs, and how to generate a whole world fast enough on the phone.

## Size assumptions (synthetic data)

- **People:** 10,000. A 256-byte core (body, needs, feelings, skills) plus 50 to 150 memories of 24 bytes and 10 to 60 relations of 16 bytes: about 3.2 KB each.
- **Animals:** 100,000 records of 32 bytes.
- **Things:** 1,000,000 items of 32 bytes.
- **Map:** `B10` squares. 1 km cells, 2048 x 1024, 32 bytes each (water, soil, plants, snow, temperature). 256 m patches, 8192 x 4096, 8 bytes each (plant and sea life by group).
- **Regions** (for loading one region): 128 x 128 km tiles, 16 x 8 = 128 regions.
- **Events:** 32 bytes each, **10 events per person per day** (meals, work, talk, travel, plus what happens to them). Size scales in step with this rate.
- Fields are smooth noise with small random wobble, so they compress somewhat. Real data may compress better or worse.

## The approaches

- **B04-1 Layout in memory:** array of structs; struct of arrays; the `hecs` entity-component crate.
- **B04-2 Saved moment:** custom fixed-layout binary file, cut into region chunks with a hash each (as `B80`); SQLite in WAL mode; FlatBuffers. Each without compression, with lz4, and with zstd.
- **B04-3 History log:** an append-only binary log in yearly segments sorted by region and person, with a small index; against one SQLite table with two indexes.
- **B04-4 Crash safety:** kill the writer with `SIGKILL` at random moments mid-save, many times; also truncate saves and flip bytes in them.
- **B11-1 Land storage:** height map plus local 3D pieces (16 m blocks of 1 m cubes, only where cliffs, overhangs or caves need them); against a sparse grid of 1 m cubes everywhere near the surface.
- **B11-2 Generation:** for the whole world at 1 km: (a) shaped noise with domain warping; (b) noise plus a quick erosion pass; (c) simple plates with uplift, then the same erosion.

## Decision rules (written before measuring, not changed afterwards)

**B04-1 Layout.** Array of structs stays, unless another layout is at least 1.3 times as fast on the update passes over animals and things (median of the two) while using at most 1.25 times the memory per record. If more than one qualifies, take the fastest, but take `hecs` instead if it is within 10% of the fastest (adding parts without touching core types serves `PRN-14`).

**B04-2 Saved moment.**
1. Out: any format that fails a crash test (B04-4), reads a whole saved moment in more than 1.5 s here (half of the 3-second opening of `VIS-14`), or needs more than 0.25 s to load one region.
2. Of the rest, take the smallest file, unless its full write (to disk, flushed) is more than 1.5 times the fastest remaining write and more than 1 s. Then take the next smallest.
3. If two are within 15% of each other on size, take the simpler. Simplest first: custom binary, FlatBuffers, SQLite; no compression, lz4, zstd.

**B04-3 History log.**
1. SQLite, which is less code to keep right, if its size per 1,000 years is at most 1.5 times the custom log's and both queries take under 0.5 s on the 1,000-person log; otherwise the custom log.
2. If the chosen log needs more than 50 GB per 1,000 years for 1,000 people (a tenth of the phone's 512 GB, `PLT-01`), report the event rate that would fit and raise it with the owner (`PRN-15`, `PLT-10`).

**B04-4 Crash safety.** At least 100 kills per saved-moment format and 50 per log format, plus 100 damaged files each. Zero failures allowed: a damaged or half-written save never loads, and the previous one always does. A format that fails is out until fixed and re-run.

**B11-1 Land storage.** Height map with 3D pieces, unless the cube grid uses at most 1.5 times its memory per km² of metre detail and builds 1 km² at most 1.5 times slower; then cubes, as one shape for everything is simpler. Either must rebuild bit-identically every time, also with a different number of threads; if not, that is fixed first. The pick must also hold metre detail for 400 camps of 1 km² (10,000 people in bands of 25) within 2 GiB, a fifth of the phone's 10 GiB (`PLT-01`), or build 1 km² in under 0.5 s so detail can be rebuilt instead of kept.

**B11-2 Generation.**
1. A method passes if, at 1 km: at least 95% of river cells (drainage of 50 km² or more) reach the sea by steepest descent; the median land slope is between 0.5° and 5°; and under 1% of land is steeper than 30°.
2. Time limit: the whole world in under 10 s on one core here. Reason: 20 candidates on 4 big phone cores within 3 minutes (`WLD-11`) gives 36 s a world; terrain gets at most a third, about 12 s, and a phone core is assumed no faster than this one.
3. Of the methods that pass within the limit, take the one that carries most of `WLD-09`'s order: plates, then erosion, then noise. If none passes, take the one closest to passing and say what it lacks.

## Method

(Filled in with the results.)
