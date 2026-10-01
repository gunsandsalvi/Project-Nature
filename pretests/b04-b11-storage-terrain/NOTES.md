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

- Rust. The shared code is the phone crate `phone/` (`kstorage`: synthetic data, save formats, log, terrain); the cloud bench is `src/` (FlatBuffers code from `schema/moment.fbs` by `flatc` 2.0.8, layouts, crash tests). Packages: `rusqlite` (bundled SQLite), `zstd` (level 1), `lz4_flex`, `flatbuffers` 2.1, `hecs` 0.10, `png`. Raw output in `results/`.
- Every synthetic value comes from keyed hashes, so the same seed gives the same bytes; every load is checked against a hash of the whole content.
- **Layouts:** 7 runs of many passes each; memory from a counting allocator. Passes: animals update energy, age and health (reading species); things wear down by material; people update 4 needs and 1 feeling.
- **Saved moments:** full size, 403 MB raw. Write = encode, write, flush, rename, flush the folder; for SQLite, inserts in one transaction (WAL, full sync) plus the checkpoint into the main file. Read = cold (pages dropped from memory after flushing), whole file decoded into the in-memory state. One region = a 128 km tile with people in it. 3 runs each.
- **Log:** events appended day by day, flushed daily. Custom: one journal for the open year, then a segment per year (sorted by region, person, time; blocks of 4,096 events; raw, or delta-coded columns with lz4 or zstd). SQLite: one table, indexes on (person, day) and (region, day), one transaction per day. 100 people x 20 years and 1,000 people x 2 years (7.3 million events each), scaled to 1,000 years. Queries: all events of one person; one region over the middle third of the span. 3 runs, 5 queries each.
- **Crash:** a writer process saves moment after moment (a 1/8-size world, 14 MB, each save different) and is killed at a random moment within about 3 save cycles (logs: within 0.5 s of daily appends, which spans year ends). Then the newest moment is loaded and checked against what was saved; it must be at least as new as the last save the writer reported done. Damage: 100 copies of a good save, each cut short, with one flipped bit, or with a 4 KB block zeroed (SQLite: one flipped bit after the first page).
- **Generation:** 2048 x 1024 cells, 3 runs on one thread, plus one on 4 threads to compare bits. Rivers and slopes from steepest descent on the final heights.
- **Metre detail:** one 1,024 x 1,024 m cell with a hard case on purpose: a 30 m cliff of layered rock across it, soft layers cut back up to 6 m (overhangs, rock shelters), and caves in a band 120 m deep behind the edge. Both approaches use the same shape function. 3 runs on 1 and 4 threads.
- All timings under the shared lock, median of 3 or more runs, range in brackets.

## Results

**B04-1 Layout in memory** (ns per record per pass; bytes per record)

| | Array of structs | Struct of arrays | `hecs` |
|---|---|---|---|
| Animals (100,000) | 2.12 (1.59-2.44) | 0.45 (0.43-0.48) | 1.60 (1.55-1.69) |
| Things (1,000,000) | 1.77 (1.71-1.90) | 0.66 (0.64-0.81) | 0.55 (0.51-0.74) |
| People cores (10,000) | 7.9 (7.1-44) | 1.4 (1.3-41) | 1.5 (1.3-42) |
| Bytes per animal or thing | 32 | 32 | 48 |
| Bytes per person core | 256 | 256 | 272 |

- Random updates by id (1 million on things): array 11.6 ns each, `hecs` 96 ns.
- Memories and relations add 3,006 bytes per person in every layout. Whole state in memory: 403 MB, two-thirds of it the 256 m patches.

**B04-2 Saved moment** (full size, 403 MB raw)

| Format | Size | Write | Cold read | One region |
|---|---|---|---|---|
| Custom | 403 MB | 1.33 s (1.25-3.02) | 0.94 s (0.80-1.98) | 5.5 ms |
| Custom + lz4 | 345 MB | 3.22 s (2.24-3.32) | 0.78 s (0.76-1.42) | 5.3 ms |
| **Custom + zstd** | **263 MB** | **1.77 s (1.54-1.91)** | **1.09 s (1.08-1.32)** | **7.8 ms** |
| FlatBuffers | 403 MB | 3.32 s (2.24-3.93) | 0.71 s (0.63-1.16) | 95 ms |
| FlatBuffers + lz4 | 346 MB | 4.26 s (3.40-4.47) | 1.02 s (0.99-2.34) | 531 ms |
| FlatBuffers + zstd | 264 MB | 3.24 s (2.91-3.89) | 1.12 s (1.10-2.15) | 719 ms |
| SQLite | 438 MB | 4.10 s (3.81-4.36) | 2.88 s (2.57-3.03) | 64 ms |
| SQLite + lz4 | 385 MB | 4.07 s (3.98-6.19) | 2.20 s (2.17-2.27) | 42 ms |
| SQLite + zstd | 313 MB | 4.54 s (4.25-5.68) | 2.66 s (2.58-2.91) | 59 ms |

- Encoding is most of a write: custom + zstd spends 1.29 s cutting, compressing and hashing, and 0.2 s flushing. The time to hand bytes to the disk varied up to 17-fold between runs (the virtual disk's write-back); the ranges show it.
- FlatBuffers can't load one region from a compressed file without unpacking all of it.

**B04-3 History log** (10 events per person per day)

| Log | Bytes per event | Per 1,000 years, 100 people | 1,000 people | Append, per day of 1,000 people | One person (2 years) | One region (8 months) |
|---|---|---|---|---|---|---|
| Custom, raw | 32.0 | 11.7 GB | 117 GB | 4.0 ms | 0.5 ms | 4.6 ms |
| Custom + lz4 | 17.9 | 6.5 GB | 65 GB | 4.2 ms | 1.0 ms | 8.4 ms |
| **Custom + zstd** | **12.8** | **4.7 GB** | **47 GB** | **4.0 ms** | **1.2 ms** | **9.4 ms** |
| SQLite | 63.5-65.7 | 23 GB | 240 GB | 78 ms | 6.2 ms | 20 ms |

- With 100 people over 20 years, all of one person's 73,000 events took 9.4 ms (custom + zstd) against 56 ms (SQLite); one region over 7 years, 31 ms against 167 ms. A person's whole 1,000 years would be about 0.5 s against 2.8 s.
- Appends include the daily flush; SQLite's time is mostly index upkeep.

**B04-4 Crash safety**

| Format | Kills | Kills inside a save | Failures | Damaged files rejected |
|---|---|---|---|---|
| Custom (3 compressions) | 300 | 275 | 0 | 300 of 300 |
| FlatBuffers (3) | 300 | 282 | 0 | 300 of 300 |
| SQLite (3) | 300 | 296 | 0 | 260 of 300; 40 changed only unused bytes |
| Custom log | 50 | 48 | 0 | not tested |
| SQLite log | 50 | 50 | 0 | not tested |

- In 204 kills the file itself was being written. Half-written temp files were never loaded, and the newest finished save always loaded with the right content. The custom log was killed through 46 simulated years, including year ends.
- **SQLite caught only 75 of its 300 damaged files itself.** The other 185 were caught by the content hash this test adds. Without it, SQLite would have loaded wrong data in silence: by default it keeps no checksums of its data.

**B11-1 Land storage** (metre detail for one km² with cliff, overhangs and caves)

| | Height map + 3D pieces | Cube grid |
|---|---|---|
| Memory | 10.8 MB (3 MB of heights and rock layers, plus 1,860 pieces) | 22.4 MB (5,450 mixed blocks) |
| Build, 1 thread | 1.15 s (1.12-1.21) | 2.03 s (2.00-2.16) |
| Build, 4 threads | 0.42 s (0.34-0.42) | 0.86 s (0.76-0.87) |
| Same bits every time, 1 or 4 threads | yes | yes |

- Without cliffs or caves the height map needs no pieces: 3 MB per km².
- Whole world at the coarser levels: heights at 1 km 8 MB; at 256 m 100 MB (height and rock type). As cubes: 34 MB at 1 km, 0.54 GB at 256 m.
- 400 camps of 1 km²: 1.2 GB on plain ground, 4.3 GB if every camp sat by a cliff like this one; cubes 9 GB.

**B11-2 Generation** (whole world, 2048 x 1024 cells, one thread)

| Method | Time | 4 threads | River cells reaching the sea | Pits per 10,000 km² | Median slope | 99th percentile | Land over 30° |
|---|---|---|---|---|---|---|---|
| (a) Warped noise | 2.64 s (2.55-2.78) | 0.75 s | 29% | 52 | 2.2° | 9.1° | 0% |
| (b) Noise + erosion | 2.54 s (2.38-2.65) | 1.56 s | 100% | 0 | 1.6° | 4.6° | 0% |
| **(c) Plates + erosion** | **3.28 s (3.00-3.41)** | **1.73 s** | **100%** | **0** | **1.8°** | **26°** | **0.9%** |
| Extra: (a) + pit filling | 2.76 s (2.69-2.89) | 0.95 s | 100% | 0 | 2.0° | 9.1° | 0% |

- Every method gives the same bits on 1 and 4 threads. Land is 35% in each.
- Previews: `previews/warp.png`, `erode.png`, `plates.png`, `warpfill.png` (one pixel per 3 x 3 km; blue lines are rivers draining 400 km² or more).

## Verdict

- **B04-1: struct of arrays.** It is 4.7 times as fast as the array of structs on animals and 2.7 times on things, at the same memory. `hecs` is 1.3 and 3.2 times as fast but needs 1.5 times the memory (16 bytes per entity), so the rule rules it out; its random lookups are also 8 times slower.
- **B04-2: the custom binary file, cut into region chunks, each compressed with zstd** (level 1). It passed every crash test and is the smallest (263 MB, 35% under raw). Its write, 1.77 s, is 1.33 times the fastest (raw custom), under the 1.5 limit. SQLite is out (it reads in 2.2-2.9 s), and so is compressed FlatBuffers (a region takes 0.5-0.7 s). Raw FlatBuffers passed but is 53% larger.
- **B04-3: the custom log, with zstd.** SQLite is 5 times larger (240 GB per 1,000 years for 1,000 people), and its appends are 20 times slower. The rule doesn't name a compression; zstd is the only one under rule 2's 50 GB, at 47 GB. So rule 2 isn't triggered, but only just (see below).
- **B04-4: all formats pass.** 1,000 kills, 0 failures; every damaged custom and FlatBuffers file rejected.
- **B11-1: height map with 3D pieces.** Cubes need 2.1 times the memory and 1.8 times the time. On the hard cliff site, 400 camps would need 4.3 GB, over the 2 GiB limit, but a km² rebuilds in 0.42 s on 4 threads, under 0.5 s. So detail is rebuilt, not kept. On one thread it takes 1.15 s.
- **B11-2: plates with uplift, then erosion.** It passes every check in 3.3 s, a third of the limit. At that speed, 20 candidate worlds on 4 cores need about 16 s of terrain (`WLD-11`). Warped noise alone fails: only 29% of rivers reach the sea. One pit-filling pass fixes that for 0.1 s.
- **The previews show what the numbers don't.** The plates world has plateau-like continents and straight plate edges; erosion alone looks the most natural. The plate model needs tuning before the architecture relies on it.

## For the owner

- **History log size** (`PRN-15`, `PLT-10`): at 10 events per person per day, 1,000 people need 47 GB per 1,000 years. That is under the 50 GB limit, but only just. With 10,000 people it is 470 GB, nearly the whole phone. To fit 50 GB with 10,000 people, either keep about 1 event per person per day, or thin old events by a fixed rule, as saved moments already are. `PLT-10` covers only saved moments today.
- **Saved moments are big:** 263 MB each, two-thirds of it the 256 m patches. 100 saved moments take 26 GB. Thinning by age (`PLT-10`) is needed from the start; storing only what changed is an option for the architecture.

## Caveats

- Synthetic data. Its compression (zstd saves 35%) says little about real data, which may be smoother. The data's own random wobble limits it.
- The cloud's virtual disk: write times varied up to 17-fold between runs. Read times were steadier.
- `SIGKILL` tests the app being killed, not a flat battery. Power loss depends on the flush order (file, rename, folder), which is followed but not tested.
- The cliff site is deliberately hard. Camps may need metre detail beyond 1 km² for foraging.
- The quality checks are rough: steepest descent at 1 km, and slopes against "Earth-like" ranges set in the rule.
- One machine, one afternoon; other tests shared the disk.

## What only the phone can answer

- Write, flush and cold-read speed on the phone's storage, and the flush cost of small appends (the daily log journal).
- Whether a 263 MB saved moment opens within the 3 seconds of `VIS-14`: the phone reads a 76 MB one (quarter world) in the phone test; multiply by 3.5.
- Whether generation and metre detail give the same bits on the phone. Cloud hashes: plates at 1024 x 512 `895e636495687a48`; metre detail `5e3b0c482d789a49`.
- zstd and generation speed on the phone's different cores, and whether Android killing the app mid-save behaves like `SIGKILL` here.

## Phone part

`phone/` builds for `aarch64-linux-android` (checked: `libkstorage.so`, 3.3 MB, exports `Java_dev_kindling_pretests_Storage_run`). See `INTEGRATION.md`. Its default run (quarter world, 5 runs each: custom raw and custom + zstd saves, small-append flushes, plates at 1024 x 512, metre detail on 1 and 4 threads) takes 20 s in the cloud.

## How to re-run

From this folder, with `CACHE` set to the shared cache folder:

```
LOCK=$CACHE/cpu.lock
export CARGO_TARGET_DIR=$CACHE/b04-target
flock $LOCK cargo build --release
B=$CARGO_TARGET_DIR/release/b04b11
flock $LOCK $B layouts
flock $LOCK $B moment 1 3 all
flock $LOCK $B log 100 20 all 3
flock $LOCK $B log 1000 2 all 3
flock $LOCK $B crash custom-zstd 100     # any format; log-custom, log-sqlite
flock $LOCK $B damage custom-zstd 100
flock $LOCK $B gen 3 warp,erode,plates,warpfill
flock $LOCK $B detail 3
flock $LOCK $B previews
flock $LOCK $B phone '{"dir":"DIR"}'
```

CPU time: about 35 minutes in all, most of it SQLite and the crash tests. `flatc --rust -o src/ schema/moment.fbs` remakes the FlatBuffers code.

## Phone results, round 2 (1 October 2026)

From the second test app on the Pixel 11 Pro XL (`pretests/phone-r2/results/phone-r2.json`), 13.6 s in all.
- **Same bits as the cloud:** plates `895e636495687a48` and metre detail `5e3b0c482d789a49` match exactly, so terrain generation is bit-identical on the phone.
- **Saved moment, quarter world** (117.5 MB of state, 75.6 MB on disk with zstd): written in 409 ms in all (305 ms to encode, 63 ms to flush), and read in 240 ms. The full 263 MB moment would open in about 0.8 s, well inside the 3 seconds of `VIS-14`.
- **Small appends** (the daily log): a 4 KB flush took 0.07 ms, and a day's events for 1,000 people 0.36 ms.
- **Speed:** plates at 1024 x 512 took 446 ms (cloud core: 901 ms), and the metre-detail test patch (1,860 pieces) 591 ms on one core and 314 ms on four (cloud: 1,006 and 297 ms). On one core, the phone is about twice as fast as a cloud core here.
