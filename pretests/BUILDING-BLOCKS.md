# Kindling: building blocks

The elementary pieces Kindling is built from, and the questions to settle by testing before choosing how to build each one.

`PROJECT.md` says what Kindling must do. This file breaks that down into the building blocks the game needs, to prepare the architecture and the implementation plan. For each block it records the open "how" questions, so that small throwaway tests can settle them cheaply before anything permanent is built. The tests are thrown away afterwards; their results stay here.

This is a working document, kept with the tests in the temporary `pretests` folder. The finished project has only three documents: the project file, the architecture and the implementation plan (`PRC-04`). Once the architecture is written, what this file records moves there, and the whole folder is deleted.

## Contents

- [How this file works](#how-this-file-works)
- [Crossroads: choices that shape everything](#crossroads-choices-that-shape-everything)
- [Map of the blocks](#map-of-the-blocks)
- [Test order](#test-order)
- [1. Foundations](#1-foundations)
- [2. Space](#2-space)
- [3. Making a world](#3-making-a-world)
- [4. The living planet](#4-the-living-planet)
- [5. Matter](#5-matter)
- [6. Bodies](#6-bodies)
- [7. Minds](#7-minds)
- [8. Culture](#8-culture)
- [9. Your powers](#9-your-powers)
- [10. Time and history](#10-time-and-history)
- [11. Presentation](#11-presentation)
- [12. Sound](#12-sound)
- [13. Platform](#13-platform)
- [14. Research tools](#14-research-tools)

---

## How this file works

- **IDs.** Each block has a permanent ID, starting at `B01`. A new block takes the next free number, and IDs are never reused. Blocks name the `PROJECT.md` items they serve, so nothing is lost on the way to the architecture.
- **Fields.** *Does:* what the block is for. *Serves:* the project items it delivers. *Needs:* the blocks it depends on. *To settle:* the questions a test must answer, with the approaches to compare. *Result:* added once the test has run.
- **Test levels.**
  - *Critical:* the answer could change the plan or a decided item.
  - *Choose:* a real choice between approaches; a test picks one.
  - *Measure:* the approach is clear; a test only measures cost and limits.
- **How each test runs.**
  1. The question and the approaches to compare come from the block.
  2. A decision rule is written before the test runs: which result picks which road (as in `RES-09`).
  3. The test is the smallest setup that answers the question, with a time limit.
  4. It runs in the AI's cloud sessions where possible. Phone tests are bundled into one test app per wave: you install it, tap run, and paste back the short result code it shows.
  5. Some answers need your eyes and ears: the look (`B66`, `B67`, `B69`, `B70`), pictures (`B54`), writing (`B73`), and sound, speech and music (`B75`, `B76`, `B77`).
  6. The result is recorded under the block. The test's code stays in its own folder here until the architecture is written.
- This file changes nothing in `PROJECT.md`. If a test shows that a decided item can't work as written, it is raised with you (`PRC-07`).

## Crossroads: choices that shape everything

These choices cut across many blocks. Each is settled by the tests of the blocks named.

- `X1` **The phone comes first.** *Decided:* the phone and cloud builds don't have to match exactly; the cloud build only has to behave the same statistically (`PLT-05`, `RES-05`). So the simulation is free to use whatever is fastest on the phone, including its graphics chip. *Settled by* `B01`, `B05`.

- `X2` **Detail and the camera.** *Decided:* the world decides where detail is needed, by its own rule; where you look changes only the picture (`WLD-12`, `WLD-13`, `MND-14`, `PRN-11`). People in routine situations may run more cheaply, but only once an experiment shows it gives the same history statistically. What this costs is measured by `B08`, `B43` and `B79`.

- `X3` **Speed never changes outcomes.** Every system takes steps of a fixed simulated length whatever the speed of time; speed only changes how many steps run per real second. Otherwise fast time would quietly cut corners (`PRN-11`). *Confirmed by* `B03`.

- `X4` **What history keeps.** *Decided:* history is saved, not re-run (`PRN-15`). What remains is what to save, how often, and what it costs: the events and the reasons behind them, key moments in full, and full saved moments to look at and branch from. *Settled by* `B06`, `B07`.

- `X5` **Running ahead of the screen.** Whether the simulation runs a little ahead of what you see, so the story director can slow time before an important moment instead of after it, and heavy moments never cause stutter. An intervention then discards and recomputes the part that ran ahead. *Settled by* `B03`, `B65`.

- `X6` **Every chance in one place.** Fortune (`GOD-04`) works on chance, so every random outcome must go through one mechanism that fortune can lean on, within provable limits. *Settled by* `B02`, `B61`.

- `X7` **Off switches and dials.** Comparison runs switch one mechanism off (`RES-10`), and experiments turn dials (`BIO-07`). Every mechanism needs a switch from the start, and switching one off must not reshuffle the randomness of all the others. *Settled by* `B02`, `B81`.

- `X8` **Deciding apart from describing.** The general-rules check (`PRN-07`) needs the code that decides what people and animals do kept apart from the code that names and describes things for you, so a search can prove that no discovery word leaks in. *Settled by* `B64`, `B82`.

- `X9` **Sharing the phone.** The simulation, the drawing, the writer AI and the sound run at once. How processor, memory, battery and heat are split between them, and when the writer AI does its work. *Settled by* `B73`, `B79`.

- `X10` **Technology and toolchain.** The programming language, engine and graphics interface (`PRC-03`), judged first on speed on the phone, then on how reliably AI agents write and test it, and on whether the cloud sessions can build the phone app and run the same rules without graphics. *Settled by* `B01`, `B66`, `B78`, `B80`.

- `X11` **Same phone, same result.** *Decided:* chance is local (`TIM-06`), so on the same phone and version, a branch with no change repeats the original, and a changed branch differs only where the change reaches. The simulation's results must therefore never depend on thread timing, the order in which work finishes, or anything outside the saved state. *Settled by* `B02`, `B05`, `B07`.

## Map of the blocks

82 blocks in 14 layers: 24 critical, 34 to choose, 24 to measure.

- **Foundations:** `B01` fast numbers on the phone · `B02` random draws · `B03` clock and scheduler · `B04` world state and memory · `B05` using every core · `B06` event record and causes · `B07` saved moments and branches · `B08` detail levels · `B09` catalogues and real-world data
- **Space:** `B10` the wrap-around map · `B11` land at every scale · `B12` movement, paths and sight
- **Making a world:** `B13` planet and sky · `B14` landforms, rock and rivers · `B15` climate · `B16` species · `B17` choosing the best world
- **The living planet:** `B18` weather · `B19` water · `B20` living geology and burial · `B21` soils, plants and wildfire · `B22` animal populations · `B23` heredity · `B24` microbes and disease
- **Matter:** `B25` ingredients and balance · `B26` structure and properties · `B27` things and shapes · `B28` breaking and knapping · `B29` heat and fire · `B30` chemistry · `B31` mechanics
- **Bodies:** `B32` the body · `B33` life course and population
- **Minds:** `B34` perception · `B35` concepts · `B36` memory · `B37` cause and effect · `B38` drives and feelings · `B39` choosing · `B40` skills · `B41` new ideas and dreams · `B42` other minds · `B43` simple minds at a distance · `B44` animal minds
- **Culture:** `B45` learning from others · `B46` conversations · `B47` words and names · `B48` sound change · `B49` grammar · `B50` belief and ritual · `B51` institutions · `B52` social life · `B53` stories and myths · `B54` pictures · `B55` music and dance · `B56` style · `B57` sky, calendars, maps and records
- **Your powers:** `B58` what nature allows · `B59` weather nudges · `B60` dreams · `B61` fortune · `B62` tracing your influence
- **Time and history:** `B63` time control · `B64` recognisers · `B65` story director
- **Presentation:** `B66` pixel renderer · `B67` land on screen · `B68` cut-away and archaeology · `B69` figures and animation · `B70` appearance from matter · `B71` screen and gestures · `B72` views and overlays · `B73` writer AI
- **Sound:** `B74` sound engine · `B75` sounds from physics · `B76` speech · `B77` music and score
- **Platform:** `B78` toolchain and app shell · `B79` phone budgets and heat · `B80` cloud runner
- **Research tools:** `B81` experiment runner and reports · `B82` automatic checks

## Test order

Tests run in waves, and each wave informs the next. Waves 0 to 2 settle what the architecture must get right from day one. Waves 3 and 4 can use lighter tests that mainly confirm the architecture can host those layers later (`PRN-09`). Where a test needs a block from a later wave, it uses a simple stand-in.

- **Wave 0, can we build and measure at all?** `B01`, `B02`, `B78`, `B79`, `B80`. Everything else depends on these.
- **Wave 1, the engine:** `B03` to `B12`, `B81`, `B82`. In parallel, the two biggest research risks run as combined tests: what a mind costs (`T1`) and Experiment 0 (`T2`).
- **Wave 2, matter, bodies and minds for the first experiments, and the first look on the phone:** `B24` to `B43`, `B45`, `B64`, `B66`, `B69`, `B73`.
- **Wave 3, fire and a living world:** `B13` to `B23`, `B44`, `B58` to `B63`, `B65`, `B67`, `B70`, `B71`, `B74`, `B75`.
- **Wave 4, words, beliefs and the whole world:** `B46` to `B57`, `B68`, `B72`, `B76`, `B77`.

### Combined tests

Some questions span several blocks, so they get tests of their own. They keep the numbers the feasibility review gave them.

- `T1` **What a mind costs** (wave 1; `B04`, `B39`): a stripped-down loop of perceiving, remembering, deciding and learning, with realistic memory sizes, timed on the phone for 30 minutes and in the cloud. Over about 1 ms per person per simulated day, Experiment 1 doesn't fit one cloud session; over about 5 ms, simplified minds become essential; over about 1 MB per person, detailed populations stop in the low thousands.
- `T2` **Experiment 0** (wave 1, cloud only; `B26`, `B28`, `B34` to `B41`, `B45`): 100 runs of a toy sandbox (`RES-21`) with about ten materials described only by their properties, generic actions with continuous force and angle, a fracture law based on properties, and the planned learners. Then the same with ten decoy materials and actions, and with a made-up material whose useful property nobody designed for. If discovery happens only when senses and actions are shaped around knapping, or collapses with the decoys, the minds are rethought before the engine is built.
- `T5` **Simplified versus full minds** (wave 2; `B43`): whether a band-level model reproduces the full model's discovery, spread, loss and population in Experiment 0's worlds. If none does, the far-zoom speeds and large populations are restated with you.

## 1. Foundations

The engine everything else runs on.

- `B01` **Fast numbers on the phone** · *Measure*
  - **Does:** chooses how the simulation does its maths, for the most simulation per watt on the phone (`X1`).
  - **Serves:** `PLT-01`, `PLT-05`, `RES-05`
  - **Needs:** `B78` for the phone side
  - **To settle:** which number formats and which hardware (fast cores, slow cores, the graphics chip) give the most simulation per watt for typical work: heat flow, random walks, large sums, learning updates; whether the cloud build still gives the same statistics (`RES-05`); whether results repeat exactly from run to run on the same phone, on each kind of hardware (`X11`).

- `B02` **Random draws** · *Measure*
  - **Does:** gives every chance event its own draw, computed from a key (world, system, being, moment, purpose), so that mechanisms don't disturb each other and a branch differs only where its changes reach.
  - **Serves:** `TIM-06`, `PRN-14`, `RES-10`, `GOD-04`
  - **Needs:** `B01`
  - **To settle:** decided: chance is local (`TIM-06`), so draws come from keys, not from one shared sequence. The test picks the keyed generator: statistical quality, including between neighbouring keys, and speed on the phone and in the cloud; how a retry for fortune gets its own draw (`GOD-04`).

- `B03` **Clock and scheduler** · *Choose*
  - **Does:** runs dozens of systems at their natural rates, from a strike lasting a moment to ice ages, at any speed from natural to centuries per minute.
  - **Serves:** `TIM-01`, `TIM-04`, `TIM-10`, `TIM-14`, `PRN-11`, `WLD-12`, `WLD-29`
  - **Needs:** `B01`, `B02`
  - **To settle:** one global tick, a fixed rate per system, a queue of timed events, or a hybrid; how systems pass their effects to each other (`WLD-29`); overhead per simulated day, in Earth time (`TIM-14`); proof that the speed of time never changes the size of the simulation's steps (`X3`).

- `B04` **World state and memory** · *Measure*
  - **Does:** stores everything that exists (land, things, plants, animals, people, minds) compactly and quick to update.
  - **Serves:** `MAT-10`, `MND-15`, `PLT-04`, `PRN-14`
  - **Needs:** `B01`
  - **To settle:** memory per person, full mind, simple mind, animal and thing; a layout that is fast on the phone; how new kinds of data are added without touching old ones.

- `B05` **Using every core** · *Choose*
  - **Does:** spreads the simulation over all the phone's cores, and the graphics chip where it helps.
  - **Serves:** `PRN-11`, `PLT-01`, `PLT-04`
  - **Needs:** `B01`, `B03`, `B04`
  - **To settle:** how to split the work (by region, by system, or in read-then-write phases) on the phone's mix of fast and slow cores; whether the graphics chip pays off for large fields such as weather, water and plants; the real speed-up on the phone; keeping results independent of thread timing (`X11`).

- `B06` **Event record and causes** · *Choose*
  - **Does:** records what happened, to whom, where, and what caused it. It is the base of the chronicle, live moments, the scientist's view and research.
  - **Serves:** `PRN-10`, `PRN-13`, `PRN-15`, `VIS-15`, `GOD-08`, `TIM-02`, `PRE-05`, `PRE-14`, `MND-25`
  - **Needs:** `B03`, `B04`
  - **To settle:** what to keep so every view of the past works (`X4`): events, the reasons behind recorded choices, and key moments in full; events per simulated year at 100, 1,000 and 10,000 people; storage per thousand years; how quickly the past can be searched and shown.

- `B07` **Saved moments and branches** · *Choose*
  - **Does:** saves the world continuously, keeps full saved moments to look at and branch from, exports and imports worlds, and keeps history across updates.
  - **Serves:** `PRN-15`, `TIM-06`, `TIM-08`, `PLT-05`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `RSK-12`
  - **Needs:** `B04`, `B06`
  - **To settle:** the size of a saved moment, and the fixed rule for thinning them with age (`PLT-10`), against storage, for worlds that need only last between big updates (`PLT-09`); proof that a branch with no change repeats the original exactly (`X11`); storage per thousand years and per branch; saving without stutter, and without loss when the app is killed mid-save (`X4`).

- `B08` **Detail levels** · *Critical*
  - **Does:** simulates coarsely where little is happening and finely where it matters, and moves between the two without contradiction.
  - **Serves:** `WLD-12`, `WLD-13`, `MND-14`, `PRN-10`, `PRN-11`
  - **Needs:** `B03`, `B04`, `B06`
  - **To settle:** the world's own rule for where detail is needed (`X2`); how far coarse and fine runs drift apart for a band foraging through a season and a herd through a year; handover rules so that nothing you saw is contradicted (fine detail drawn from coarse results, or fine results fed back); how much each level saves.

- `B09` **Catalogues and real-world data** · *Measure*
  - **Does:** keeps every ingredient, structure, law, reality check and species as a stand-alone entry in plain language, with its key values and their sources and the rules or ranges for the rest (`PRN-05`), readable by people and loaded by the simulation.
  - **Serves:** `MAT-05`, `MAT-13`, `MAT-14`, `MAT-15`, `PRN-05`, `PRN-14`, `WLD-19`
  - **Needs:** nothing
  - **To settle:** an entry format you can read on a phone and a program can check; how long it takes to source and verify one entry, since there will be thousands, with every value checked against the fetched source and the supporting passage quoted (`RSK-16`); the licences of the data sources.
  - **Result** (1 October 2026; `pretests/b09-catalogues/`):
    - **Format:** Markdown, with the data block inside each entry as the single source and the table generated from it. All three formats tried (YAML, TOML, Markdown) had no errors, so readability on a phone decided.
    - **Cost:** about 3 minutes per checked entry, or 27 seconds a value; 10,000 values come to about 76 agent-hours, plus review.
    - **Quotes:** always copied by a tool from the fetched text, never typed. All 34 tool-copied quotes matched; a summarising fetch tool got 5 of 10 wrong or missing.
    - **Checks:** the checker caught all 168 planted errors, but not a value taken from the wrong column of the right table, so an independent reviewer still checks meaning.
    - **Sources:** a third of the pages tried couldn't be read by a script later, so each value is checked once, when it is added, while its source is at hand.
    - **The real limit is gaps, not time:** 15% of properties had no checkable source, and 6 values are stand-ins, such as wood in general for birch.
    - **Licences:** use USGS, the USDA Wood Handbook, Wikipedia and CC BY papers; cite only NIST's data, The Engineering ToolBox and the Handbook of Mineralogy; avoid the CRC Handbook and MatWeb.
    - **Since then:** only key values are sourced, a few hundred growing layer by layer, each checked once when added, then locked, keeping only the source's name, link and quote; fetched copies are deleted (`PRN-05`, `RSK-16`). The trial's 5 entries alone had left 47 MB of fetched pages. Everything else, stand-ins included, is a labelled estimate.

## 2. Space

- `B10` **The wrap-around map** · *Measure*
  - **Does:** handles places, distances and neighbours on a world that wraps both ways, with latitude and seasons, and maps it onto a globe for display.
  - **Serves:** `WLD-01`, `WLD-02`, `WLD-03`
  - **Needs:** `B01`
  - **To settle:** square or six-sided cells, and how they nest across scales; distances across the wrap; the polar seam as a permanent ice cap that weather systems stop at and nothing crosses (`WLD-01`), and how it looks on the globe.
  - **Result** (1 October 2026; `pretests/b10-map/`):
    - **Square cells in a quadtree.** Hexagons lost on every count: a coarse hexagon is never exactly its children (7–44% of the area lands in the wrong parent), rolling fine data up was 4.5 times slower, and their paths were less accurate.
    - **Paths:** on square cells, raw paths run about 6% longer than the true distance, and 1% after a simple smoothing; hexagons ran about 10%, and 1.3% smoothed. For `B12`: 1 path in 20 is still about 4% long after smoothing.
    - **Wrapping:** correct across both seams; with the ice cap blocked, no path crosses it.
    - **Fit:** squares divide the 2:1 world exactly at every scale, from the whole map down to the metre.
    - **Globe:** map columns become longitude and rows latitude, on a globe of radius 318 km. East–west distances shrink with latitude, to a half at 60°, so the third of the map beyond 60° fills 13% of the globe.
    - **For you:** `WLD-01` doesn't say how wide the polar ice cap is.

- `B11` **Land at every scale, above and below ground** · *Critical*
  - **Does:** represents the land from whole regions down to the metre: cliffs, caves, overhangs, rock layers, soils, underground water and buried things.
  - **Serves:** `WLD-12`, `WLD-13`, `WLD-14`, `WLD-24`, `MAT-08`, `PRE-23`, `PRE-24`, `PRE-25`
  - **Needs:** `B08`, `B10`
  - **To settle:** a height map with local 3D pieces for caves and cliffs, columns of stacked layers, or sparse grids of tiny cubes (voxels); memory for 2 million km²; how fast metre-level detail appears near people, identical every time; how changes such as digging, erosion and burial are stored on top.

- `B12` **Movement, paths and sight** · *Choose*
  - **Does:** lets people and animals move (walk, run, climb, swim), find their way, and see what is around them.
  - **Serves:** `BIO-18`, `BIO-21`, `MND-03`, `MND-16`, `MND-18`, `CUL-16`
  - **Needs:** `B11`
  - **To settle:** paths planned on each mind's own mental map, or on the true land; route-finding over long distances in layers; the cost of lines of sight with light, fog and terrain; travel times on slopes checked against real walking data.

## 3. Making a world

- `B13` **Planet and sky** · *Measure*
  - **Does:** sets each world's day, year, tilt, moons and share of land, and moves the sun, moons, stars, planets, comets, meteors and auroras.
  - **Serves:** `WLD-06`, `WLD-07`, `WLD-26`, `TIM-14`
  - **Needs:** `B01`, `B10`
  - **To settle:** exact sky motion over thousands of years at low cost; eclipses and tides from the moons; a consistent sky over a world that is a torus underneath.

- `B14` **Landforms, rock and rivers** · *Choose*
  - **Does:** makes plates, mountains, volcanoes, faults, rock layers, minerals and ores, then valleys, rivers, lakes, deltas and coasts.
  - **Serves:** `WLD-08`, `WLD-09`, `WLD-14`, `WLD-17`
  - **Needs:** `B10`, `B11`
  - **To settle:** which fast methods imitate deep time convincingly (growing plates with uplift and erosion, or noise shaped by rules); flint, ores and clay where a geologist would expect them; all within the time budget of `B17`.

- `B15` **Climate from geography** · *Choose*
  - **Does:** works out each place's seasons of rain, temperature and wind from latitude, height, the sea and mountains.
  - **Serves:** `WLD-05`, `WLD-09`, `WLD-16`, `WLD-30`
  - **Needs:** `B13`, `B14`
  - **To settle:** a simple physical model (sunlight, heat balance, moisture carried by winds) against rules distilled from Earth's climates; whether rain shadows, coastal and inland climates and climate belts look right; speed. Decided: quantities set by distance (climate belts, weather systems, currents and migrations) scale with the world, and everything local stays real (`WLD-30`); the test confirms the scaled patterns still look and behave right.

- `B16` **Species from Earth families** · *Choose*
  - **Does:** adapts Earth's plant and animal families into each world's 50 or so animals and 200 or so plants, with traits and body chemistry.
  - **Serves:** `WLD-09`, `WLD-19`, `WLD-23`, `BIO-19`
  - **Needs:** `B09`, `B15`
  - **To settle:** a quick evolution run over traits, or rule-based adaptation; the data needed per family (size, diet, breeding, seasons, chemistry, with sources) and the time to gather it; whether the results feel new but familiar.

- `B17` **Choosing the best world** · *Measure*
  - **Does:** generates many candidate worlds, scores them, offers you the best three and finds where history begins.
  - **Serves:** `WLD-10`, `WLD-11`, `WLD-24`
  - **Needs:** `B13`, `B14`, `B15`, `B16`, `B21`, `B22`
  - **To settle:** generating the candidates and finding the best three within a few minutes in total on the phone (`WLD-11`); whether high scores match the worlds a person would pick.

## 4. The living planet

- `B18` **Weather** · *Choose*
  - **Does:** draws daily weather from the climate, moves storms across the land, and runs the long cycles: ice ages at real cycle lengths, with worlds starting as one ends (`WLD-16`), eruption winters, and the effect of people on the land.
  - **Serves:** `WLD-16`, `WLD-22`, `WLD-25`, `GOD-02`
  - **Needs:** `B15`
  - **To settle:** a statistical weather generator with moving storms, or a simplified dynamic model; extremes at natural rates; nudging without leaving the climate's range (`B59`).

- `B19` **Water** · *Choose*
  - **Does:** rivers, lakes, wetlands, springs, groundwater, ice and floods; seas with currents, tides and changing sea level.
  - **Serves:** `WLD-15`, `WLD-17`, `WLD-26`
  - **Needs:** `B11`, `B14`, `B18`
  - **To settle:** the cost of daily river flow and floods at kilometre scale; coastlines that move as the sea rises and falls; how currents feed the climate.

- `B20` **Living geology, burial and what lasts** · *Measure*
  - **Does:** erosion, shifting rivers, landslides, earthquakes and eruptions during play; sediment that buries things; what survives in which ground.
  - **Serves:** `WLD-15`, `WLD-22`, `MAT-08`
  - **Needs:** `B11`, `B19`
  - **To settle:** event sizes and frequencies from real records; burial rates in caves, floodplains and lakes checked against archaeological data; storage for buried things over tens of thousands of years.

- `B21` **Soils, plants and wildfire** · *Choose*
  - **Does:** soils that form and wear out; plants that grow, flower, fruit, seed and die back; wildfires that spread with wind and slope, and land that regrows.
  - **Serves:** `WLD-04`, `WLD-09`, `WLD-18`, `WLD-27`, `WLD-28`
  - **Needs:** `B11`, `B18`, `B19`
  - **To settle:** plants as patches far away and as single plants near people, with a clean handover between the two (`B08`); food yields in real calories per hectare; fire spread at real rates; cost.

- `B22` **Animal populations** · *Critical*
  - **Does:** populations that eat, breed, migrate and die in food webs that boom and crash, with single animals near people.
  - **Serves:** `WLD-18`, `WLD-19`, `BIO-19`
  - **Needs:** `B08`, `B16`, `B21`
  - **To settle:** whether 50 species under general rules (body size setting appetite, breeding and range, from real scaling laws) stay alive and plausible for a thousand years without tuning any single species; herds as groups far away and individuals nearby; cost.

- `B23` **Heredity** · *Choose*
  - **Does:** passes body and mind traits from parents to young in people, animals and plants at real speeds, with the evolution dial for experiments.
  - **Serves:** `BIO-06`, `BIO-07`, `BIO-08`, `BIO-22`, `WLD-20`, `MND-20`
  - **Needs:** `B02`
  - **To settle:** explicit genes, the simpler model where a child's trait is the parents' average plus some scatter, or a mix; whether selection works at real speeds (for example, foxes bred for tameness); heredity for herds and plant patches simulated as groups; looks that vary by region with sunlight at real speeds (`BIO-22`).

- `B24` **Microbes and disease** · *Choose*
  - **Does:** rot, fermentation and disease as living microbes that grow, spread and evolve.
  - **Serves:** `WLD-21`, `BIO-05`, `RCK-07`, `RCK-09`, `RCK-14`
  - **Needs:** `B09`, `B21`, `B22`
  - **To settle:** microbes as populations in hosts, foods and places rather than as individuals; growth from real temperature and moisture data; whether epidemics behave as in reality, where some diseases die out in small bands and persist only in large populations.

## 5. Matter

- `B25` **Ingredients and exact balance** · *Measure*
  - **Does:** gives every ingredient its real elemental makeup and keeps elements and energy exactly balanced through every change.
  - **Serves:** `MAT-01`, `MAT-09`
  - **Needs:** `B01`, `B09`
  - **To settle:** whole-number bookkeeping that stays exact through millions of changes; cost per change.

- `B26` **Structure and derived properties** · *Critical*
  - **Does:** gives every property (hardness, how it breaks, how it burns, taste, colour, sound) from measured data where it decides what is possible and from labelled estimates otherwise, combined by stated rules (`MAT-03`).
  - **Serves:** `MAT-02`, `MAT-03`, `MAT-05`, `PRN-07`
  - **Needs:** `B25`
  - **To settle:** the smallest set of structural descriptions (grain size, glassiness, porosity, fibres, moisture, flaws) and rules for combining them that reproduce measured properties of the first layer, with no rule for any single material: flint, chert, obsidian, quartzite, granite, basalt, limestone and sandstone; oak, birch and pine, wet and dry; bone and antler; ice. Which properties are stored per ingredient, and which are derived.

- `B27` **Things and their shapes** · *Critical*
  - **Does:** gives each thing a shape fine enough to knap a stone, haft a point or bend a bow, and cheap enough for a whole camp.
  - **Serves:** `MAT-06`, `MAT-10`, `MAT-12`
  - **Needs:** `B26`
  - **To settle:** grids of tiny cubes, shapes built from simple solids, surface meshes, or a few measurements (mass, edge angle, sharpness); memory per thing; whether tools nobody planned can still be represented.

- `B28` **Breaking and knapping** · *Critical*
  - **Does:** decides how anything breaks when struck or pressed: flakes, edges, splinters, crumbling, shattering.
  - **Serves:** `MAT-04`, `MAT-06`, `RCK-01`, `RCK-10`, `RES-02`
  - **Needs:** `B26`, `B27`, `B31`
  - **To settle:** a true fracture simulation, or a model built from controlled knapping experiments (where a flake comes off and how big it is, given the edge angle, the force and where the blow lands) that carves the stone's shape; whether flint and obsidian chip while granite doesn't, and heat-treated flint chips better, with no special rules; cost per strike.

- `B29` **Heat and fire** · *Choose*
  - **Does:** heat moving between things, ignition, burning with more or less air, embers, charring, and the temperatures fires reach.
  - **Serves:** `MAT-04`, `RCK-02`, `RCK-08`, `RCK-22`, `BIO-11`, `WLD-28`
  - **Needs:** `B26`, `B27`, `B31`
  - **To settle:** one temperature per thing, or finer detail inside things; whether friction lights dry tinder but not damp, an open fire reaches 600–900 °C and a charcoal furnace with forced air passes 1,100 °C, all from general physics; cost per fire at natural speed and in fast time.

- `B30` **Chemistry by general reactions** · *Critical*
  - **Does:** changes matter through a few kinds of reaction (burning, reduction as in smelting, breakdown by heat, taking up or losing water, dissolving, charring without air, binding), each decided by real heat and rate data, never by named products.
  - **Serves:** `MAT-04`, `MAT-07`, `MAT-14`, `MAT-16`, `RCK-03`, `RCK-04`, `RCK-05`, `RCK-06`, `RCK-07`, `RCK-08`, `RCK-09`, `RCK-12`, `RCK-13`, `RCK-14`, `RCK-15`, `RCK-16`, `RCK-17`, `RCK-18`, `RCK-19`, `RCK-20`
  - **Needs:** `B24`, `B25`, `B26`, `B29`
  - **To settle:** whether a small engine with a dozen kinds of reaction and about 30 ingredients passes the reality checks with zero special cases; whether adding tin ore gives bronze with no new law; which real data tables are needed, and whether they exist with usable licences. Metals come late, but the shape of this engine must be right from the start (`PRN-09`).

- `B31` **Mechanics: bodies, tools and structures** · *Choose*
  - **Does:** weight, momentum, leverage, springiness and friction for the body's basic actions; throwing, spear-throwers, bows, cord, cutting and scraping; floating and sinking, and flowing water and air; shelters that stand or fall.
  - **Serves:** `MAT-04`, `MAT-06`, `MAT-11`, `MAT-12`, `RCK-11`, `RCK-21`, `BIO-21`
  - **Needs:** `B27`
  - **To settle:** a physics engine, or simple physical formulas for each kind of interaction; whether throwing sticks, spear-throwers and bows work purely from physics, at speeds that match real ones; cost.

## 6. Bodies

- `B32` **The body: needs, food, harm and healing** · *Measure*
  - **Does:** energy, water, warmth and sleep; nutrition down to vitamins and minerals; poison and medicine by dose; wounds, fractures, burns and infections by body part; healing, scars, disability and the effect of care.
  - **Serves:** `BIO-01`, `BIO-09`, `BIO-10`, `BIO-11`, `BIO-12`, `BIO-13`, `BIO-14`, `BIO-17`, `BIO-19`
  - **Needs:** `B24`, `B26`, `B30`
  - **To settle:** an hourly model per person, and a cheaper one for simple minds; whether real timelines come out (days without water, weeks without food, scurvy after one to three months without vitamin C, bones mending in weeks); cost per person per simulated day.

- `B33` **Life course and population** · *Critical*
  - **Does:** growth, ageing, fertility, pregnancy, birth and nursing, and the population patterns of real hunter-gatherers.
  - **Serves:** `BIO-02`, `BIO-03`, `BIO-04`, `BIO-14`, `BIO-15`, `BIO-16`, `BIO-20`, `TIM-09`, `WLD-04`
  - **Needs:** `B23`, `B24`, `B32`
  - **To settle:** whether real life tables (four in ten children dying before 15, adults living into their 60s and 70s, a child every three to four years) emerge from causes alone, since no death may be random (`BIO-14`); how often a starting population of 45–120 dies out within 500 years, which decides how many worlds Experiment 1 needs; generating the first people like the world, with families, ages and relationships from real patterns (`BIO-02`).

## 7. Minds

- `B34` **Perception** · *Measure*
  - **Does:** turns the world into what a person senses: properties such as weight, hardness, colour, smell, taste, warmth and sound, never names; limited by light, fog, distance, age and attention.
  - **Serves:** `BIO-18`, `MND-02`, `MND-03`
  - **Needs:** `B12`, `B26`
  - **To settle:** how what is sensed is encoded; cost per person per second; how attention picks what gets noticed.

- `B35` **Concepts** · *Choose*
  - **Does:** lets each person sort what they sense into their own categories, which can be wrong and differ between groups.
  - **Serves:** `MND-04`
  - **Needs:** `B34`
  - **To settle:** running averages (prototypes), remembered examples, or categories that split when something doesn't fit; stable, sensible categories with human-like mistakes (flint and chert lumped together, a poisonous berry taken for a safe one); memory and cost per person.

- `B36` **Memory** · *Measure*
  - **Does:** keeps events, a mental map with seasons attached, people, know-how and beliefs. Feeling and repetition keep memories, the rest fade, and retelling changes them.
  - **Serves:** `MND-08`, `MND-18`
  - **Needs:** `B35`
  - **To settle:** how many memories a person can hold within the phone's budget; forgetting that matches human data; how fading memories are condensed into beliefs.

- `B37` **Cause-and-effect learning** · *Critical*
  - **Does:** forms beliefs such as "doing this to that, in this situation, leads to this", with a certainty that follows the evidence, superstitions included.
  - **Serves:** `MND-05`, `MND-13`, `MND-27`, `BIO-02`, `BIO-20`, `PRN-01`
  - **Needs:** `B35`, `B36`
  - **To settle:** certainty from counting outcomes, learning by association, or small causal models; how a learner finds which features mattered (the stone, the angle, the song) and links effects that come hours later, such as food poisoning; whether discovery and superstition rates match human studies of learning from coincidences; holding the several kinds of belief, each with a certainty and its evidence (`MND-27`); cost per update.

- `B38` **Drives, feelings and personality** · *Measure*
  - **Does:** needs and urges, emotions that colour choices and memories, inborn temperament, and humanity's inborn biases.
  - **Serves:** `MND-07`, `MND-19`, `MND-20`, `MND-21`, `MND-26`
  - **Needs:** `B32`
  - **To settle:** a light model in which what an event means for a person's goals decides the feeling; inborn biases that tilt learning without teaching anything; which further tendencies (`MND-26`) change outcomes in comparison runs.

- `B39` **Choosing what to do** · *Critical*
  - **Does:** habits for routine; planning backwards from a need through believed causes when habits fail or the stakes rise; exploring in play and in desperation; every choice recorded with its reasons.
  - **Serves:** `MND-01`, `MND-09`, `MND-22`, `MND-25`, `PRN-13`
  - **Needs:** `B36`, `B37`, `B38`
  - **To settle:** scoring the options against needs, goal-directed planning over learned beliefs, or layered habits, compared on how well people survive, how readable their reasons are, and cost per decision; how many decisions a person makes per day. This, more than anything else, sets how fast time can run (`TIM-01`).

- `B40` **Skills and practice** · *Choose*
  - **Does:** learned sequences of the body's basic actions with fine control (angle, force, timing) that improve with practice.
  - **Serves:** `MND-06`, `MND-13`, `MAT-12`, `BIO-02`
  - **Needs:** `B28`, `B31`
  - **To settle:** improvement by trial and error around the current technique, or by learning from rewards; learning curves that match real ones (a novice shatters stones; mastery takes many hours); cost.

- `B41` **New ideas, curiosity and dreams** · *Critical*
  - **Does:** notices surprises, and turns accidents, observations of nature, tinkering, analogies and dreams into hunches worth trying.
  - **Serves:** `MND-10`, `MND-11`, `MND-12`, `MND-17`
  - **Needs:** `B36`, `B37`, `B40`
  - **To settle:** how often each source gives useful and useless ideas; how dreams recombine memories into new links; whether the fire-making path in `MND-11` happens at plausible rates without being made likely by design.

- `B42` **Other minds and relationships** · *Measure*
  - **Does:** tracks what others know, want and believe, one step deeper, and who is family, friend, rival or owed.
  - **Serves:** `MND-23`, `MND-24`
  - **Needs:** `B36`
  - **To settle:** tracking only what matters (who knows which skill or place), so cost doesn't grow with the square of the group; cost for a band of 30 and a camp of 300.

- `B43` **Simple minds at a distance** · *Critical*
  - **Does:** runs people in routine situations more cheaply, even as part of their band, and in full again when something new, risky or important happens to them, with no break in their story (`X2`).
  - **Serves:** `MND-14`, `MND-15`, `PRN-11`
  - **Needs:** `B08`, `B39`, `B45`
  - **To settle:** whether simple and full minds give the same discoveries, spread and losses at the level of a band; how each person's own beliefs and memories are kept or rebuilt when they return to full detail; the saving.

- `B44` **Animal minds** · *Choose*
  - **Does:** the same machinery with fewer abilities: fear, routes, habits, and each species' ways, such as pack hunting.
  - **Serves:** `MND-16`, `GOD-12`
  - **Needs:** `B37`, `B39`
  - **To settle:** cost per animal; whether hunted animals grow warier as real ones do, so hunting becomes an arms race.

## 8. Culture

- `B45` **Learning from others** · *Critical*
  - **Does:** imitation with copying errors, teaching aimed at someone who doesn't know, and copying whoever succeeds or whatever most people do.
  - **Serves:** `CUL-01`, `CUL-02`, `CUL-16`, `MND-17`, `RES-03`
  - **Needs:** `B40`, `B42`
  - **To settle:** which copying rules reproduce known results (skills spread within a generation; small, isolated groups lose skills, as in studies of Tasmania); how well an onlooker can see a technique to copy it.

- `B46` **Conversations** · *Choose*
  - **Does:** what people actually say to each other (warnings, questions, news, teaching, stories), held as structured meanings before any words.
  - **Serves:** `CUL-01`, `CUL-04`, `CUL-24`, `MND-23`, `MND-25`, `SND-03`
  - **Needs:** `B39`, `B42`
  - **To settle:** what a message can say; how talk changes beliefs, weighted by trust in the speaker; cost. It is now an item of its own (`CUL-24`).

- `B47` **Words and names** · *Choose*
  - **Does:** words agreed through use for each person's own concepts, drifting into dialects and languages; names for people, places and things, with English glosses.
  - **Serves:** `BIO-02`, `CUL-04`, `CUL-14`, `CUL-18`, `PRE-12`, `PRE-38`
  - **Needs:** `B35`, `B46`
  - **To settle:** "naming games" (repeated attempts to be understood) when speakers' concepts differ; how fast a band's words settle; drift against the real rate at which languages replace words; labels made from each world's own sounds until people's own names emerge (`PRE-38`).

- `B48` **Sounds and sound change** · *Choose*
  - **Does:** gives each language its own sounds and changes words by regular sound laws, so related languages form families you can trace.
  - **Serves:** `CUL-17`, `PRE-36`
  - **Needs:** `B47`
  - **To settle:** sound changes drawn from those common in real languages, or arising from ease of speaking and hearing; check: the standard methods linguists use to compare languages recover the true family tree from the simulated words.

- `B49` **Grammar and sentences** · *Critical*
  - **Does:** grows grammar from a few dozen words, and turns meanings into sentences in each language.
  - **Serves:** `CUL-04`, `CUL-17`, `SND-03`
  - **Needs:** `B46`, `B47`, `B48`
  - **To settle:** the hardest research question in the language layer: which mechanism (pressure from each generation relearning the language, words worn down into grammar, or patterns agreed in use) gives real, drifting grammar at low cost; how much grammar speech and translation need.

- `B50` **Explanation, belief and ritual** · *Critical*
  - **Does:** seeks causes for big unexplained events, suspects hidden beings, and turns believed causes into rituals, taboos, specialists, sacred places and ideas about the dead.
  - **Serves:** `CUL-05`, `CUL-19`, `CUL-20`, `GOD-06`, `MND-21`
  - **Needs:** `B37`, `B41`, `B45`
  - **To settle:** whether the chain in `MOM-03` (a death on a hill, avoidance, offerings, a being in the storm) arises from general mechanisms at plausible rates; how gifts to the unseen arise by analogy with gifts between people; how beliefs survive evidence against them.

- `B51` **Norms, roles and institutions** · *Choose*
  - **Does:** turns repeated behaviour into shared expectations that people know, teach and enforce: norms, roles, ranks and rites.
  - **Serves:** `CUL-06`, `CUL-22`
  - **Needs:** `B42`, `B45`
  - **To settle:** how people hold "what one does here" and react to breaches; how institutions split and dissolve.

- `B52` **Social life** · *Choose*
  - **Does:** pairing and family rules, sharing, gifts, trade, ownership, leadership and status, conflict, war and cruelty.
  - **Serves:** `BIO-15`, `BIO-17`, `CUL-07`, `CUL-08`, `CUL-21`, `CUL-22`
  - **Needs:** `B42`, `B51`
  - **To settle:** whether hunter-gatherer patterns, such as sharing out large game, arise from general mechanisms (kinship, give and take, norms); whether feuds and alliances follow from interests and memory (`MOM-11`); pairing and conception kept abstract (`BIO-15`); cost.

- `B53` **Stories and myths** · *Choose*
  - **Does:** keeps stories as structured accounts (who, what, why) built from memories, beliefs and dreams, retold with drift; genealogies and legends.
  - **Serves:** `CUL-11`, `CUL-15`, `PRE-10`
  - **Needs:** `B36`, `B46`
  - **To settle:** a story structure that can drift with retelling, be set beside what really happened, and feed the writer AI; drift that matches how real retellings change (simpler, more familiar).

- `B54` **Pictures: art and remembered scenes** · *Choose*
  - **Does:** paintings and carvings composed from memories and myths in a culture's style and real pigments, and the small scenes of memories you use to shape dreams.
  - **Serves:** `CUL-09`, `CUL-25`, `PRE-15`, `GOD-10`
  - **Needs:** `B53`, `B56`, `B66`, `B69`
  - **To settle:** decided: a picture is kept as what it shows and how (composition, style, skill and pigments, `CUL-25`). The test settles how it is drawn as pixel art from that, and whether you can recognise the event it shows.

- `B55` **Music and dance** · *Measure*
  - **Does:** rhythms, scales, songs, instruments and dances that grow out of each culture and drift.
  - **Serves:** `CUL-10`, `CUL-25`
  - **Needs:** `B31`, `B45`
  - **To settle:** how scales follow from the instruments people make (where the holes are bored on a flute sets its notes); how musical traditions drift.

- `B56` **Style** · *Measure*
  - **Does:** each culture's look in tools, clothing and buildings, drifting so objects can be dated by style.
  - **Serves:** `CUL-12`
  - **Needs:** `B27`, `B45`
  - **To settle:** style as a set of drifting features; check: archaeologists' methods of dating by style put simulated objects in the right order.

- `B57` **Their sky, calendars, maps and records** · *Measure*
  - **Does:** constellations, tracked seasons, festivals, drawn maps, tally marks and signs.
  - **Serves:** `CUL-03`, `CUL-13`, `CUL-14`, `PRE-11`
  - **Needs:** `B13`, `B37`, `B47`, `B54`
  - **To settle:** little until its milestone nears; it builds on learning, language and pictures.

## 9. Your powers

- `B58` **What nature allows here** · *Measure*
  - **Does:** checks each power against the conditions at that place and moment, offers only what nature could do, and says why the rest isn't possible.
  - **Serves:** `GOD-01`, `GOD-05`, `GOD-11`, `PRN-03`
  - **Needs:** `B18`, `B20`, `B41`
  - **To settle:** checks that stay fast and complete as systems are added, with each system reporting what it allows.

- `B59` **Weather and disaster nudges** · *Choose*
  - **Does:** places small events exactly, pushes seasons over a region, and sets off disasters where conditions allow, always within the climate's range.
  - **Serves:** `GOD-02`, `GOD-05`, `WLD-22`
  - **Needs:** `B18`, `B19`, `B20`, `B21`
  - **To settle:** decided: nudges shift the weather's own draws within the climate's real range, a run of them can't exceed the climate's worst natural stretch, and quakes and eruptions use up stored strain and magma (`GOD-05`). The test checks that these budgets hold and stay cheap.

- `B60` **Dreams as a power** · *Choose*
  - **Does:** lets you browse a sleeper's memories, compose a dream from memories and a feeling, and have it act on their mind.
  - **Serves:** `GOD-03`, `GOD-10`, `GOD-12`
  - **Needs:** `B36`, `B41`, `B54`
  - **To settle:** decided: a sent dream is never stronger than the strongest natural dream (`GOD-03`). A fire-making test bench measures how much that raises the odds, without guaranteeing anything (`RES-07`).

- `B61` **Fortune** · *Choose*
  - **Does:** blesses or curses people, groups, herds and places, at most doubling or halving a chance.
  - **Serves:** `GOD-04`
  - **Needs:** `B02`
  - **To settle:** decided: a blessed failure gets one more try, and a cursed success is retried at most half the time, with at most one retry per chance event (`GOD-04`). Check: the limits hold for hunts, foraging, fertility, recovery and sickness, and nothing impossible becomes possible.

- `B62` **Tracing your influence** · *Choose*
  - **Does:** shows where you intervened and what changed because of it, in the scientist's view only.
  - **Serves:** `GOD-07`, `GOD-08`, `GOD-09`
  - **Needs:** `B06`, `B07`
  - **To settle:** decided: the chain of causes by default, and a comparison branch on request, which differs only where the act reached (`GOD-09`, `X11`). The test settles how the chain is followed through the record, and how long a comparison branch takes from the nearest saved moment.

## 10. Time and history

- `B63` **Time control and the speed table** · *Measure*
  - **Does:** ties the speed of time to zoom, with pause, a speed dial and a lock, and natural speed close up; pauses when the app is closed.
  - **Serves:** `TIM-01`, `TIM-04`, `TIM-05`, `TIM-07`, `TIM-10`, `TIM-14`, `TIM-15`
  - **Needs:** `B03`, `B39`
  - **To settle:** the speeds actually reachable at each zoom, once the cost of minds and nature is known, against the first target: a thousand years in one night for a few hundred people (`TIM-07`).

- `B64` **Recognisers: naming what happened** · *Critical*
  - **Does:** watches events and labels them for you: firsts and discoveries ("fire made by friction for the first time"), institutions, peoples, eras, dark content. It never feeds back into the world.
  - **Serves:** `CUL-06`, `CUL-23`, `PRE-05`, `PRE-08`, `PRE-18`, `PRE-39`, `TIM-02`, `RES-03`, `PRN-07`
  - **Needs:** `B06`
  - **To settle:** whether discoveries can be recognised reliably from raw events (Experiment 1 must count who "can chip sharp flakes"); peoples that don't flicker in and out over time; keeping this code provably apart from the code that decides (`X8`). It is now an item of its own (`PRE-39`).

- `B65` **Story director and running ahead** · *Choose*
  - **Does:** spots important moments, slows time around them, raises live moments, and skips to the next one.
  - **Serves:** `TIM-02`, `TIM-03`, `TIM-11`, `PRE-08`, `RSK-03`
  - **Needs:** `B03`, `B07`, `B64`
  - **To settle:** scoring importance from the record; whether running ahead of the screen (`X5`) lets it slow down before a moment; how often it interrupts you, within about one interruption a minute (`PRE-08`).

## 11. Presentation

- `B66` **Pixel renderer** · *Critical*
  - **Does:** draws the world in 3D at low resolution with hard pixels: colour in steps, fine patterns only in narrow bands and fixed to surfaces, outlines and lit edges, stable pixels, shadows, firelight, haze, and palettes for time and season.
  - **Serves:** `PRE-01`, `PRE-02`, `PRE-04`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-30`, `PRE-31`, `VIS-14`
  - **Needs:** `B78`, `B79`
  - **To settle:** the graphics interface and engine; frame time at the screen's full refresh rate with about 4 screen pixels per art pixel; pixels that stay still while the camera moves and turns; battery and heat. The cleaned-up mockup (`mockups/visual-style.html`) is the visual reference; its pixels still crawl while the camera turns or zooms.

- `B67` **Land on screen at every zoom** · *Critical*
  - **Does:** draws terrain, rock faces with their layers, caves and overhangs, plants and water, from close up through the map look to the globe, in one continuous zoom.
  - **Serves:** `PRE-03`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-29`, `WLD-02`
  - **Needs:** `B11`, `B66`
  - **To settle:** switching levels of detail without popping or shimmer; drawing caves and overhangs; the move from slanted view to map to globe on a world that wraps; rivers never thinner than one or two art pixels.

- `B68` **Cut-away and archaeology** · *Measure*
  - **Does:** slices the ground open to show layers, soils, water and buried finds, and lets you dig through history.
  - **Serves:** `PRE-09`, `PRE-25`
  - **Needs:** `B20`, `B67`
  - **To settle:** drawing cross-sections from the underground model; picking finds by touch.

- `B69` **Figures and animation** · *Critical*
  - **Does:** small 3D figures built from parts, animated at 8–12 poses a second for any action the simulation produces, with faces, hair and clothing. From far away, figures become markers, then glowing points.
  - **Serves:** `PRE-27`, `PRE-28`, `MAT-12`, `BIO-22`, `PRE-18`
  - **Needs:** `B66`
  - **To settle:** tiny-cube models or low-poly models; whether animation built from the body's basic actions can show any action, including ones nobody planned; readability at 40–60 art pixels tall; cost with hundreds on screen.

- `B70` **Appearance from matter and style** · *Choose*
  - **Does:** chooses colour ladders, sheen and texture from what a thing is made of, and its look from its simulated shape and its culture's style, so unplanned things still look right.
  - **Serves:** `PRE-20`, `MAT-03`, `CUL-12`, `PRN-07`
  - **Needs:** `B26`, `B27`, `B56`, `B66`
  - **To settle:** decided: ladders are made automatically from the simulated colour, with hand-picked ladders for common materials (`PRE-20`). The test settles the mapping, and whether invented objects look right without being drawn by hand.

- `B71` **Screen, gestures, cards and settings** · *Measure*
  - **Does:** the world-first interface: gestures, cards, both orientations, and settings such as live moments, the content setting, subtitles and vibration.
  - **Serves:** `PRE-18`, `PRE-32`, `PRE-33`, `PRE-34`, `PRE-35`, `PRE-40`, `TIM-15`, `GOD-10`, `PLT-02`, `VIS-10`
  - **Needs:** `B66`
  - **To settle:** decided: one-thumb zoom, drawing an area from the long-press menu, views from the bottom edge, and a brief touch for the time control (`PRE-33`). The test settles how they feel in the hand, one-handed in portrait and two-handed in landscape.

- `B72` **Views and overlays** · *Measure*
  - **Does:** the chronicle, following a soul, map overlays, family trees and legends, their sky and maps, the bestiary, the language tree, the two views of a mind, comparing timelines, and any further view the data supports.
  - **Serves:** `PRE-05`, `PRE-06`, `PRE-07`, `PRE-10`, `PRE-11`, `PRE-12`, `PRE-13`, `PRE-14`, `PRE-16`, `PRE-36`, `TIM-08`, `TIM-13`, `GOD-09`, `PRN-04`
  - **Needs:** `B06`, `B66`
  - **To settle:** one common way to build views from data, so each new view is cheap (`PRN-14`), each with a story version and a scientist's version (`PRE-14`); speed of queries over long histories.

- `B73` **Writer AI** · *Critical*
  - **Does:** turns simulation data into the chronicle, life stories, myths, dreams and summaries, in chosen voices, on the phone, and checks every text against its data.
  - **Serves:** `PRE-17`, `PRE-19`, `PRE-37`, `PRE-38`, `PRE-41`, `PRN-06`, `VIS-15`, `TIM-12`, `RSK-08`
  - **Needs:** `B06`, `B53`, `B79`
  - **To settle:** which model and runtime (the phone's built-in AI model, or an open model run by the app); words per second, memory and battery alongside the simulation and drawing; whether the writing is good enough (`RSK-08`); how to catch any fact, name or real-world knowledge that isn't in the data; how quickly text appears when you open something, since it is written when first opened or during pauses (`PRE-41`). This can be tested now, on hand-made sample data.

## 12. Sound

- `B74` **Sound engine** · *Measure*
  - **Does:** places sounds in space with distance, muffling and cave echo; mixes by zoom; turns single sounds into the feel of a period when time runs fast; keeps silence; drives vibration.
  - **Serves:** `SND-01`, `SND-05`, `SND-07`, `SND-08`, `SND-09`, `SND-10`
  - **Needs:** `B78`
  - **To settle:** the phone's audio interface; the cost of many sounds at once; how fast-time sound is built from the event record.

- `B75` **Sounds from physics** · *Choose*
  - **Does:** makes impacts, fire, water and instruments sound the way their materials and shapes would.
  - **Serves:** `SND-06`, `MAT-03`
  - **Needs:** `B26`, `B27`, `B74`
  - **To settle:** physical sound models (a struck object rings at frequencies set by its stiffness, weight and size), or recordings shaped by properties; whether flint and granite sound clearly different and real; cost per sound.

- `B76` **Speech in their languages** · *Critical*
  - **Does:** speaks real sentences in each language with its own sounds, in voices that differ from person to person.
  - **Serves:** `SND-03`, `CUL-17`, `CUL-18`
  - **Needs:** `B48`, `B49`, `B74`
  - **To settle:** fully synthetic voices, which can make any sound but sound robotic, or neural voices fed phonetic spelling, which sound natural but may bend invented sounds towards real languages; quality on the phone. You listen and choose, as you asked to hear it live.

- `B77` **Their music and the score** · *Choose*
  - **Does:** performs their songs and instruments, and generates the score from short themes that follow the world.
  - **Serves:** `SND-02`, `SND-04`, `CUL-10`
  - **Needs:** `B55`, `B75`
  - **To settle:** where the score's themes come from (written in advance, or generated) and how they adapt; whether it reaches studio quality. You listen and judge.

## 13. Platform

- `B78` **Toolchain and app shell** · *Critical*
  - **Does:** builds the phone app in the AI's cloud sessions, delivers it as a download, and handles the app's life: pausing when closed, both orientations, working offline, saving, export and import.
  - **Serves:** `PLT-01`, `PLT-02`, `PLT-03`, `PLT-06`, `PLT-07`, `PLT-08`, `TIM-05`, `PRC-11`
  - **Needs:** nothing
  - **To settle:** whether a cloud session can fetch the Android tools and build, sign and publish an installable app; how you download it, and signing builds for your free hobbyist developer account under the 2027 install rules (`PLT-06`, `RSK-18`); whether saving survives the app being killed mid-write. Every phone test depends on this.

- `B79` **Phone budgets and heat** · *Critical*
  - **Does:** measures and manages what the phone can sustain (processor, graphics, memory, battery, heat), keeps frames smooth by slowing time under load, and runs overnight mode on the charger.
  - **Serves:** `PLT-04`, `PRN-11`, `VIS-14`, `MND-15`, `TIM-12`, `RSK-02`
  - **Needs:** `B78`
  - **To settle:** the sustained speed of each kind of core before the phone throttles, over minutes and over hours; frame pacing at full refresh; using the phone's own heat warnings to slow time before it gets hot; the split between simulation, drawing, writer AI and sound (`X9`). Known: the phone has 16 GB of memory, so the app can use about 10 GiB (`PLT-01`), and the battery budget is 25–30% an hour (`VIS-14`).

- `B80` **Cloud runner and moving worlds** · *Critical*
  - **Does:** runs the same rules without graphics in the AI's cloud sessions, many worlds at once; checks that the cloud gives the same statistics as the phone; and moves an experiment's saved moments to the phone.
  - **Serves:** `PLT-05`, `SCP-15`, `RES-01`, `RES-05`, `PRN-15`
  - **Needs:** `B01`, `B07`
  - **To settle:** how much computing a session really offers (cores, memory, how long it may run), and so how many world-years an hour; how long Experiment 1 (100 worlds of 500 years) would take; how the statistical comparison with the phone build is run; the size of a saved moment sent to the phone.
  - **Result** (1 October 2026; `pretests/b80-cloud-runner/`):
    - **Design:** one process per world, four at a time per session, with a checkpoint every simulated month: written to a temporary file, flushed, renamed, and checked on loading. Plan with 3.4 effective cores per session.
    - **Same result after interruptions (`X11`):** in 10 trials, each killed twice at random moments and resumed, every run ended bit-identical to the uninterrupted one, even when the number of threads changed at a resume. Half-written and damaged checkpoints were never loaded. What made it exact: keyed draws, fixed read-then-write phases, whole-number counts, a fixed order for births, deaths and saving, and the full state saved in a fixed layout with a hash.
    - **Speed held:** over 20 minutes, the 4 cores kept 94–101% of their first-minute speed, with 1.5% lost to other machines on the host.
    - **Parallel worlds:** four worlds as four processes ran 3.7–3.9 times as fast as one; one world on four threads gained only 1.1 to 3.4 times, so it is used only when there are fewer worlds than cores, as on the phone.
    - **Experiment 1 at its old full size** (300 worlds of 500 years, about 100 people each): 1,521, 7,604 or 76,042 CPU-hours at a guessed 1, 5 or 50 ms per person per simulated day. Far too much, so experiments now run mainly in small sandboxes sized to a computing budget stated up front, confirmed in a few full worlds (`RES-21`).
    - **Still open:** how long a detached process survives beyond 2 hours and through idle time (a heartbeat is running), and where checkpoints live between sessions.

## 14. Research tools

- `B81` **Experiment runner and reports** · *Measure*
  - **Does:** defines experiments with criteria fixed first, runs comparison runs with mechanisms switched off, gathers results as ranges, logs surprises, and writes report pages with charts and links to saved moments.
  - **Serves:** `RES-01`, `RES-02`, `RES-03`, `RES-06`, `RES-07`, `RES-08`, `RES-09`, `RES-10`, `RES-12`, `RES-13`, `RES-14`, `RES-15`, `RES-16`, `RES-17`, `RES-18`, `RES-19`, `RES-20`, `RES-21`, `PRN-05`, `PRN-12`, `BIO-07`
  - **Needs:** `B64`, `B80`
  - **To settle:** how quickly a sandbox can be set up from the real rules, and how many runs a question needs within its budget (`RES-21`). This is also where the rule that every mechanism has an off switch is set (`X7`), with the rules for tuning, confirming on fresh seeds and re-running the signature moments (`RES-16`, `RES-17`).

- `B82` **Automatic checks** · *Choose*
  - **Does:** runs the reality checklist, the general-rules check, the coverage check, the phone and cloud statistical comparison at milestones, and the screenshot tour for the visual review.
  - **Serves:** `MAT-15`, `MAT-17`, `PRE-31`, `PRN-07`, `PRC-10`, `PRC-12`, `RES-04`, `RES-05`, `RES-11`
  - **Needs:** `B78`, `B80`
  - **To settle:** how the phone runs its share of the statistical comparison with little of your time; how the code is laid out so the general-rules search means something (`X8`); the stronger general-rules check, with identity swaps, decoys and a made-up material (`PRN-07`); the file check and the coverage check (`PRC-10`, `PRC-12`); taking screenshots on the phone or in the cloud.
