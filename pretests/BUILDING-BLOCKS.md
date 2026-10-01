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

- **Done:** `B09` data catalogues, `B10` map cells, `B80` cloud runner.
- **Running:** `B01` numbers and languages, `B02` random draws, `B78` building the phone app, `B79` what the phone sustains.
- **Next:** `B04` storing data, `B11` terrain, `B66` drawing and gestures, `B74` sound, `B76` speech, `B73` writer AI.

## The blocks

- `B01` **Numbers and languages** · running (`pretests/b01-b02-numbers-random/`)
  - **Question:** which language (Rust, C++, or managed code as Android runs it) and which number formats (decimals of 32 or 64 bits, or whole-number fixed point) give the most simulation per watt on each kind of phone core, and which results repeat exactly (`TIM-06`).
  - **Approaches:** the same small kernels (heat flow on a wrapping grid, random walks, large sums, learning updates) in each language and format, timed in the cloud and on the phone, on one core of each kind and on all of them.

- `B02` **Random draws** · running (`pretests/b01-b02-numbers-random/`)
  - **Question:** which generator makes each draw from a key (world, system, being, moment, purpose), as local chance requires (`TIM-06`).
  - **Approaches:** six keyed generators, compared on statistical quality, including between neighbouring keys, and on speed in the cloud and on the phone.

- `B04` **Storing data** · next
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

- `B11` **Terrain** · next
  - **Question:** how to store land from whole regions down to the metre, with cliffs, caves and overhangs (`PRE-23`, `PRE-24`), and how to generate it fast enough on the phone (`WLD-11`).
  - **Approaches:** on synthetic land, a height map with local 3D pieces against sparse grids of small cubes: memory for the whole world, and how fast metre-level detail appears near a camp. For generation, two or three fast methods, such as shaped noise or noise with a quick erosion pass, timed on the phone.

- `B66` **Drawing and gestures** · next
  - **Question:** which graphics engine draws the game's look (`PRE-01` to `PRE-04`, `PRE-20` to `PRE-22`) at the screen's full refresh rate on the phone, and whether the gestures (`PRE-33`) feel right in the hand.
  - **Approaches:** the mockup's scene (`mockups/visual-style.html`) drawn as it is with WebGL, and by a native engine (Vulkan or OpenGL ES): frame time, battery and heat; a fix for pixels crawling while the camera turns or zooms (`PRE-22`); and the decided gestures, which you try.

- `B73` **Writer AI** · next
  - **Question:** which model and runtime write the text on the phone (`PRE-37`), and at what cost beside the simulation and drawing.
  - **Approaches:** the phone's built-in model (Gemini Nano) and an open model run by the app (Gemma), on hand-made sample data: words per second, memory and heat; how often each adds facts that aren't in the data (`PRE-17`); whether it softens dark events (`RSK-17`); and your rating of a few entries. Gemma's download needs a Hugging Face account and accepting its licence.

- `B74` **Sound** · next
  - **Question:** how sound is made and played on the phone (`SND-01`, `SND-06`, `SND-08`).
  - **Approaches:** struck flint, granite and wood made from their properties by simple physical models, against recordings shaped by those properties; a bone flute and a hide drum made from their shapes (`CUL-10`); the phone's audio engine, its delay, and the cost of many sounds at once. You listen.

- `B76` **Speech** · next
  - **Question:** how to speak an invented language on the phone (`SND-03`).
  - **Approaches:** a synthetic voice built from the language's own sounds, against a natural-sounding neural voice fed phonetic spelling; quality, cost, and how far each bends invented sounds toward real languages. You listen.

- `B78` **Building the phone app** · running (`pretests/b78-b79-phone/`)
  - **Question:** the best way to build and deliver the app from the cloud sessions (`PLT-06`): Kotlin with a Rust or C++ core, pure native, or a web view; build time, size, and how easily one core builds for both the phone and the cloud. Signing for your free hobbyist developer account comes later.
  - **Also:** produces the first phone test app, which runs `B01`, `B02` and `B79` on the phone.

- `B79` **What the phone sustains** · running (`pretests/b78-b79-phone/`)
  - **Question:** the sustained speed of each kind of core before the phone heats up; frame pacing at full refresh; how much memory the app can really use, about 10 GiB (`PLT-01`); and battery per hour, against 25–30% (`VIS-14`).

- `B80` **Cloud runner** · done (`pretests/b80-cloud-runner/`)
  - **Question:** how much computing the cloud sessions give, and a runner that survives interruptions without changing results (`PLT-05`, `SCP-15`).
  - **Result** (1 October 2026; `pretests/b80-cloud-runner/`):
    - **Design:** one process per world, four at a time per session, with a checkpoint every simulated month: written to a temporary file, flushed, renamed, and checked on loading. Plan with 3.4 effective cores per session.
    - **Same result after interruptions (`TIM-06`):** in 10 trials, each killed twice at random moments and resumed, every run ended bit-identical to the uninterrupted one, even when the number of threads changed at a resume. Half-written and damaged checkpoints were never loaded. What made it exact: keyed draws, fixed read-then-write phases, whole-number counts, a fixed order for births, deaths and saving, and the full state saved in a fixed layout with a hash.
    - **Speed held:** over 20 minutes, the 4 cores kept 94–101% of their first-minute speed, with 1.5% lost to other machines on the host.
    - **Parallel worlds:** four worlds as four processes ran 3.7–3.9 times as fast as one; one world on four threads gained only 1.1 to 3.4 times, so it is used only when there are fewer worlds than cores, as on the phone.
    - **Experiment 1 at its old full size** (300 worlds of 500 years, about 100 people each): 1,521, 7,604 or 76,042 CPU-hours at a guessed 1, 5 or 50 ms per person per simulated day. Far too much, so experiments now run mainly in small sandboxes sized to a computing budget stated up front, confirmed in a few full worlds (`RES-21`).
    - **Still open:** how long a detached process survives beyond 2 hours and through idle time (a heartbeat is running), and where checkpoints live between sessions.

## Moved out of the pre-tests

Everything that needs a working world: matter laws, bodies, minds, culture and language, ecology, your powers, the story director, recognisers and views, and the research tests on what a mind costs (`T1`), Experiment 0 (`T2`) and simplified minds (`T5`). These go into the architecture and the implementation plan, and are tested in sandboxes (`RES-21`).
