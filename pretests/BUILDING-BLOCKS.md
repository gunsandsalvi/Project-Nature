# Kindling: pre-tests

The basic technical blocks Kindling is built from, each tested on its own before the architecture is written.

A pre-test answers one "how" question about one basic block, such as which language, how to store data or how to make sound, with the smallest setup that answers it, in hours rather than weeks. No pre-test needs a working world. Everything that does (matter laws, minds, culture, ecology, your powers, the story director) is designed in the architecture and tested later in sandboxes (`RES-21`).

This is a working document, kept with the tests in the temporary `pretests` folder. The finished project has only three documents (`PRC-04`). Once the architecture is written, what this file records moves there, and the whole folder is deleted.

## How pre-tests work

- **What qualifies:** the block can be built and measured without any other part of the game; it settles a technical choice the architecture needs; and it takes hours, not weeks.
- **How each one runs:**
  1. The question, and the approaches to compare.
  2. A decision rule, written before measuring: which result picks which approach.
  3. The smallest setup that answers the question, with a time limit.
  4. It runs in the cloud where possible. Phone tests are bundled into one test app per round: you install it, tap run, and paste back the result code it shows.
  5. Some answers need your eyes and ears: drawing, sound and speech.
  6. The result is recorded here; the code stays in its folder until the architecture is written.
- **Ground rules from the project file:** the phone comes first, and the cloud only has to match it statistically (`PLT-05`); chance is local, so on the same phone and version the same saved state always gives the same result (`TIM-06`).
- **IDs:** blocks keep the IDs they had in the first version of this file, so folders and commits still match.

## Status

- **Done:** `B09` data catalogues, `B10` map cells, `B80` cloud runner, `B78` building the phone app.
- **Measured on your phone:** `B01`, `B02` and `B79` (first test app).
- **Running:** `B04` storing data, `B11` terrain, `B66` drawing and gestures, `B73` writer AI, `B74` sound, `B76` speech.

## The blocks

- `B01` **Numbers and languages** · running (`pretests/b01-b02-numbers-random/`)
  - **Question:** which language (Rust, C++, or managed code as Android runs it) and which number formats (decimals of 32 or 64 bits, or whole-number fixed point) give the most simulation per watt on each kind of phone core, and which results repeat exactly (`TIM-06`).
  - **Approaches:** the same small kernels (heat flow on a wrapping grid, random walks, large sums, learning updates) in each language and format, timed in the cloud and on the phone, on one core of each kind and on all of them.
  - **Result** (1 October 2026; cloud, then the phone):
    - **Language: Rust.** Rust and C++ ran equally fast. Rust gave bit-identical results on this cloud machine and on the phone's kind of chip (run under emulation), decimals included; C++ differed there unless one compiler setting is switched off. Java ran at a quarter to a third of native speed: fine for the app's screens, not for the simulation.
    - **Numbers: 32-bit decimals** were fastest for every kind of work. Fixed point may catch up on the phone's chip, so the phone decides.
    - **Repeatable:** every kernel gave identical results across runs and across thread counts (`TIM-06`). With Rust, the cloud even reproduces the phone's results exactly, which isn't required (`PLT-05`) but helps comparisons.
    - **On the phone:** Rust was 16% faster than C++ on the middle cores and 47% on the fastest core, and even on the others; Java ran at a fifth of Rust's speed. 32-bit decimals stay: fixed point was close but never clearly faster. Every result repeated exactly, and all 45 that had a cloud prediction matched it bit for bit, so the cloud can reproduce the phone's results.
    - **Kinds of core:** 2 small, 4 middle and 1 fastest. The fastest is 1.7 times a middle core but uses about twice the energy per step; the middle cores do the most work per joule.

- `B02` **Random draws** · running (`pretests/b01-b02-numbers-random/`)
  - **Question:** which generator makes each draw from a key (world, system, being, moment, purpose), as local chance requires (`TIM-06`).
  - **Approaches:** six keyed generators, compared on statistical quality, including between neighbouring keys, and on speed in the cloud and on the phone.
  - **Result** (1 October 2026; cloud, then the phone):
    - **Generator: a guarded hash of the wyhash family.** All six passed the quality tests on single streams and on neighbouring keys. The fastest, plain wyhash-style (1.1 billion draws a second on one core), gives every being the same draw at one rare moment per stream, so the guarded version is used: about a fifth slower, and still faster than the others.
    - **Fortune retries** get their own independent draw by flipping one bit of the key, with no detectable link to the first draw (`GOD-04`).
    - **On the phone:** 1.0 billion draws a second on the fastest core and 3.3 billion on all cores, second only to the unguarded version.

- `B04` **Storing data** · running (`pretests/b04-b11-storage-terrain/`)
  - **Question:** how to hold the world in memory and on the phone's storage (`PRN-15`, `PLT-07`, `PLT-10`).
  - **Approaches:** on synthetic data at realistic sizes (people, animals, things, map cells, events), compare layouts in memory for millions of small records, and formats for saved moments and the history log: a custom binary format, SQLite, or a schema format such as FlatBuffers. Measure memory per record, save and load speed, size per saved moment and per thousand years of events, query speed, and whether a save survives the app being killed mid-write.

- `B09` **Data catalogues** · done (`pretests/b09-catalogues/`)
  - **Question:** the format of the catalogue entries (`MAT-13`), and what it costs to source a value honestly (`PRN-05`, `RSK-16`).
  - **Result** (1 October 2026; `pretests/b09-catalogues/`):
    - **Format:** Markdown, with the data block inside each entry as the single source and the table generated from it. All three formats tried (YAML, TOML, Markdown) had no errors, so readability on a phone decided.
    - **Cost:** about 3 minutes per checked entry, or 27 seconds a value; 10,000 values come to about 76 agent-hours, plus review.
    - **Quotes:** always copied by a tool from the fetched text, never typed. All 34 tool-copied quotes matched; a summarising fetch tool got 5 of 10 wrong or missing.
    - **Checks:** the checker caught all 168 planted errors, but not a value taken from the wrong column of the right table, so an independent reviewer still checks meaning.
    - **Sources:** a third of the pages tried couldn't be read by a script later, so each value is checked once, when it is added, while its source is at hand.
    - **The real limit is gaps, not time:** 15% of properties had no checkable source, and 6 values are stand-ins, such as wood in general for birch.
    - **Licences:** use USGS, the USDA Wood Handbook, Wikipedia and CC BY papers; cite only NIST's data, The Engineering ToolBox and the Handbook of Mineralogy; avoid the CRC Handbook and MatWeb.
    - **Since then:** only key values are sourced, a few hundred growing layer by layer, each checked once when added, then locked, keeping only the source's name, link and quote; fetched copies are deleted (`PRN-05`, `RSK-16`). The trial's 5 entries alone had left 47 MB of fetched pages. Everything else, stand-ins included, is a labelled estimate.

- `B10` **Map cells** · done (`pretests/b10-map/`)
  - **Question:** square or six-sided cells on the wrap-around world, and how they nest across scales (`WLD-01`, `WLD-03`).
  - **Result** (1 October 2026; `pretests/b10-map/`):
    - **Square cells in a quadtree.** Hexagons lost on every count: a coarse hexagon is never exactly its children (7–44% of the area lands in the wrong parent), rolling fine data up was 4.5 times slower, and their paths were less accurate.
    - **Paths:** on square cells, raw paths run about 6% longer than the true distance, and 1% after a simple smoothing; hexagons ran about 10%, and 1.3% smoothed. For pathfinding later: 1 path in 20 is still about 4% long after smoothing.
    - **Wrapping:** correct across both seams; with the ice cap blocked, no path crosses it.
    - **Fit:** squares divide the 2:1 world exactly at every scale, from the whole map down to the metre.
    - **Globe:** map columns become longitude and rows latitude, on a globe of radius 318 km. East–west distances shrink with latitude, to a half at 60°, so the third of the map beyond 60° fills 13% of the globe.
    - **For you:** `WLD-01` doesn't say how wide the polar ice cap is.

- `B11` **Terrain** · running (`pretests/b04-b11-storage-terrain/`)
  - **Question:** how to store land from whole regions down to the metre, with cliffs, caves and overhangs (`PRE-23`, `PRE-24`), and how to generate it fast enough on the phone (`WLD-11`).
  - **Approaches:** on synthetic land, a height map with local 3D pieces against sparse grids of small cubes: memory for the whole world, and how fast metre-level detail appears near a camp. For generation, two or three fast methods, such as shaped noise or noise with a quick erosion pass, timed on the phone.

- `B66` **Drawing and gestures** · running (`pretests/b66-drawing/`)
  - **Question:** which graphics engine draws the game's look (`PRE-01` to `PRE-04`, `PRE-20` to `PRE-22`) at the screen's full refresh rate on the phone, and whether the gestures (`PRE-33`) feel right in the hand.
  - **Approaches:** the mockup's scene (`mockups/visual-style.html`) drawn as it is with WebGL, and by a native engine (Vulkan or OpenGL ES): frame time, battery and heat; a fix for pixels crawling while the camera turns or zooms (`PRE-22`); and the decided gestures, which you try.

- `B73` **Writer AI** · running (`pretests/b73-writer/`)
  - **Question:** which model and runtime write the text on the phone (`PRE-37`), and at what cost beside the simulation and drawing.
  - **Approaches:** the phone's built-in model (Gemini Nano) and an open model run by the app (Gemma), on hand-made sample data: words per second, memory and heat; how often each adds facts that aren't in the data (`PRE-17`); whether it softens dark events (`RSK-17`); and your rating of a few entries. Gemma's download needs a Hugging Face account and accepting its licence.

- `B74` **Sound** · running (`pretests/b74-b76-sound-speech/`)
  - **Question:** how sound is made and played on the phone (`SND-01`, `SND-06`, `SND-08`).
  - **Approaches:** struck flint, granite and wood made from their properties by simple physical models, against recordings shaped by those properties; a bone flute and a hide drum made from their shapes (`CUL-10`); the phone's audio engine, its delay, and the cost of many sounds at once. You listen.

- `B76` **Speech** · running (`pretests/b74-b76-sound-speech/`)
  - **Question:** how to speak an invented language on the phone (`SND-03`).
  - **Approaches:** a synthetic voice built from the language's own sounds, against a natural-sounding neural voice fed phonetic spelling; quality, cost, and how far each bends invented sounds toward real languages. You listen.

- `B78` **Building the phone app** · running (`pretests/b78-b79-phone/`)
  - **Question:** the best way to build and deliver the app from the cloud sessions (`PLT-06`): Kotlin with a Rust or C++ core, pure native, or a web view; build time, size, and how easily one core builds for both the phone and the cloud. Signing for your free hobbyist developer account comes later.
  - **Also:** produces the first phone test app, which runs `B01`, `B02` and `B79` on the phone.
  - **Result** (1 October 2026; `pretests/b78-b79-phone/`):
    - **All four ways build easily from the cloud:** clean builds took 18–37 seconds, a one-line rebuild about 2 seconds, and the apps came out at 44–742 KB. The tools take 1.1–1.4 GB to download, mostly the native toolkit.
    - **Choice so far: a Kotlin screen with a native core.** The written rule put Kotlin with C++ one point ahead of Kotlin with Rust, but the rule mixed two questions: the core's language is settled by `B01`, which chose Rust. Kotlin with Rust is the base of the test app.
    - **The first test app** (`dist/kindling-pretests-r1.apk`, 1.1 MB) is built and checked as far as possible here: its native code ran correctly on the phone's kind of chip under emulation, and its unit tests and Android's code checks passed. It has not yet run on a real phone.
    - **Not tried:** the Godot game engine; time ran out.

- `B79` **What the phone sustains** · running (`pretests/b78-b79-phone/`)
  - **Question:** the sustained speed of each kind of core before the phone heats up; frame pacing at full refresh; how much memory the app can really use, about 10 GiB (`PLT-01`); and battery per hour, against 25–30% (`VIS-14`).
  - **Result** (1 October 2026, from the first test app; `pretests/b78-b79-phone/`):
    - **Sustained speed:** under full load on all 7 cores, speed fell within 2 minutes to about 43% of a short burst, then held steady for the rest of the 10 minutes. The phone stayed cool (battery 37.6 °C, thermal status "light") at about 3 W, using 14.6% of the battery an hour, inside the 25–30% budget with room for drawing. Plan the simulation on the held speed, not the burst.
    - **Smoothness:** a simple scene kept 120 Hz with 0.2% missed frames. The mockup's WebGL drawing took about 2 ms of the 8.3 ms each frame allows, with 1.4% missed frames.
    - **Memory:** the app took and used 10 GiB, the test's limit, with no warning and without being closed. At most 8.4 GiB stayed in fast memory at once, and the phone's free memory fell to under 1 GB, so 10 GiB (`PLT-01`) is reachable but at the edge; the simulation should plan on about 8 GiB.

- `B80` **Cloud runner** · done (`pretests/b80-cloud-runner/`)
  - **Question:** how much computing the cloud sessions give, and a runner that survives interruptions without changing results (`PLT-05`, `SCP-15`).
  - **Result** (1 October 2026; `pretests/b80-cloud-runner/`):
    - **Design:** one process per world, four at a time per session, with a checkpoint every simulated month: written to a temporary file, flushed, renamed, and checked on loading. Plan with 3.4 effective cores per session.
    - **Same result after interruptions (`TIM-06`):** in 10 trials, each killed twice at random moments and resumed, every run ended bit-identical to the uninterrupted one, even when the number of threads changed at a resume. Half-written and damaged checkpoints were never loaded. What made it exact: keyed draws, fixed read-then-write phases, whole-number counts, a fixed order for births, deaths and saving, and the full state saved in a fixed layout with a hash.
    - **Speed held:** over 20 minutes, the 4 cores kept 94–101% of their first-minute speed, with 1.5% lost to other machines on the host.
    - **Parallel worlds:** four worlds as four processes ran 3.7–3.9 times as fast as one; one world on four threads gained only 1.1 to 3.4 times, so it is used only when there are fewer worlds than cores, as on the phone.
    - **Experiment 1 at its old full size** (300 worlds of 500 years, about 100 people each): 1,521, 7,604 or 76,042 CPU-hours at a guessed 1, 5 or 50 ms per person per simulated day. Far too much, so experiments now run mainly in small sandboxes sized to a computing budget stated up front, confirmed in a few full worlds (`RES-21`).
    - **Long runs:** a detached process has kept running for over 2 hours 25 minutes without a missed beat while the session was in use, so long runs needn't be cut into 2-hour pieces. Still open: what happens when the session goes idle, and where checkpoints live between sessions.

## Moved out of the pre-tests

Everything that needs a working world: matter laws, bodies, minds, culture and language, ecology, your powers, the story director, recognisers and views, and the research tests on what a mind costs (`T1`), Experiment 0 (`T2`) and simplified minds (`T5`). These go into the architecture and the implementation plan, and are tested in sandboxes (`RES-21`).
