# Kindling: architecture

How Kindling is built: its parts, how they talk, the rules they keep, and why each choice was made.
It serves `PROJECT.md`, which says what the game must be, and is served by `IMPLEMENTATION.md`, which says in what order to build it.
Every choice cites the research note behind it (`research/NN-*.md`), where the sources are.
The code is the index: code names the `PROJECT.md` items it implements, so this file says how and why, never where (`CLAUDE.md`, rule 3).

## Status (3 October 2026)

- Written from scratch after the owner stopped the old plan.
  The old architecture, for a Rust engine of our own, stays in git history at commit `ebaeae3`.
  When a milestone is detailed, its domain designs (the 60-day year, activities, needs, clusters) may be consulted, but nothing is carried over unexamined.
- **Technology approved by the owner** (`PRC-03`):
  - Godot 4.7, after a bake-off against Bevy (research 01);
  - the simulation in C++ as a Godot plug-in (GDExtension);
  - Godot's own source changed only as a last resort.
- The parts for later milestones are outlines.
  Each is designed in full when its milestone is next, from what the earlier ones taught (research 00).

## A1. Overview

### A1.1 What it must deliver

- **Your reference look on your phone** (`PRE-01` to `PRE-31`, `VIS-14`): crisp 3D pixel art, smooth, in portrait and landscape, on a Pixel 11 Pro XL with a PowerVR GPU (research 01, 03, 04).
- **A big believable simulation** (`PRN-11`, `MND-14`): thousands of people with full minds, in a world 1,000 by 2,000 km (`WLD-03`), time running faster or slower but never cutting detail.
- **The same history every run, on the phone and in the cloud** (`RES-05`, `TIM-16`).
- **Offline, private, saved always** (`PLT-03`, `PLT-07`).
- **Built and tested by AI agents** in cloud sessions, with you judging the look and the feel (`PRC-01`, `RES-22`).

### A1.2 The big picture

```
            your touches                      what you see and hear
                 |                                     ^
                 v                                     |
  +-------------------------------------------------------------------+
  |  Godot app (game/): scenes, cameras, rendering, interface, sound  |
  |      sends COMMANDS  ------------------->                         |
  |      reads SNAPSHOTS <-------------------                         |
  +---------------------------------|---------------------------------+
                                    |  GDExtension boundary (A3.7)
  +---------------------------------v---------------------------------+
  |  Simulation library (sim/, C++): world, living things, people,    |
  |  minds, culture, history, saves. Never reads Godot. Same bits     |
  |  everywhere (A3.4).                                               |
  +---------------------------------|---------------------------------+
                                    |
           data/ catalogues (text)  +  art made by code (art/, A5)
```

- The simulation owns the truth.
- Godot only draws a snapshot of it each frame and passes your gestures and powers in as commands.
- Nothing on screen can change history (`WLD-13`, `TIM-03`).

### A1.3 Decisions

| Decision | Why | Research |
|---|---|---|
| Godot 4.7 | Reached the look fastest in the bake-off; mature Android export; stable; much help online | 01 |
| Simulation in C++ (GDExtension) | Fast enough for thousands of minds; Godot's official plug-in language; Rust plug-ins are experimental on Android | 01, 02 |
| EnTT for entities and components | Proven, fast, header-only, used in Minecraft; data-oriented | 02 |
| Content as data files | New plants, animals, things and blueprints without code (`PRN-14`) | 02, 06 |
| Fixed ticks, an event queue, keyed randomness | Determinism, and long processes cost nothing until they end | 02 |
| 3D drawn small and scaled up, outlines, stepped light | How every reference picture is made | 03 |
| Soft painted nature, pixel-textured made things, art made by code | Your choice; proven in the bake-off | 04, 13 |
| Generator in the real order of causes, tuned for the look | Believable worlds in minutes, from the seed | 05 |
| Utility choice with kept reasons, a small planner on top | Explainable (`PRN-13`), reactive, data-driven | 07 |
| A story sifter, not a storyteller | Nothing happens for a story's sake (`PRN-12`) | 09 |
| The phone's own Gemini Nano only rewords, checked | Language models describe, never decide (`PRN-06`) | 09 |

## A2. Code layout, builds and delivery

### A2.1 Repository layout

```
game/          the Godot project: project.godot, scenes, GDScript for glue and interface, shaders,
               imported assets (art/ writes its output into game/assets/generated/, never committed)
sim/core/      the simulation in C++20, with no Godot dependency: maths, chance, time, storage, catalogues,
               saves, world, life, people, minds, culture, history
sim/bind/      the GDExtension classes that expose the simulation to Godot (A3.7)
sim/tests/     doctest unit tests and scene runners, built natively
data/          catalogues and test scenes as TOML files (A3.5, A11)
art/           the art generators (research 13): textures, the model kit, animation poses
tools/         check.sh, build and delivery scripts, filecheck.py, signing
research/      the research notes this file cites
prototypes/    throwaway prototypes, deleted when their question is answered
dist/          the signed APK of the latest alpha and its note
```

### A2.2 Builds

- **C++:** CMake and Ninja, C++20, with godot-cpp pinned to Godot 4.7.
  - The simulation is built natively for the cloud (x86-64 Linux, for tests) and for the phone (arm64 Android, NDK).
  - The extension carries both.
- **Godot:** 4.7.2, with its export templates pinned.
  The app is exported from the command line (`--headless --export-release`), as in the bake-off.
- **Targets:** Android arm64 for the phone, and Linux x86-64 for tests and pictures in the cloud.
  There is no web build: Godot's is about 40 MB, beyond the private page's 15 MB (research 01).
- **Compiler rules for the simulation (A3.4):** no fast-math, no fused multiply-add contraction (`-ffp-contract=off`), exceptions and RTTI as godot-cpp needs.

### A2.3 Delivery of each alpha (`PRC-11`, `PLT-06`)

- **The APK:**
  - signed with the release key derived from the passphrase secret (`tools/signing-key.py`; only that script reads it);
  - package `dev.kindling.app`, so each alpha installs over the last;
  - committed to `dist/` on the work branch and linked from the note.
- **The note** says what is new, what to try, what is rough, the items delivered and the links.
  It is published at the owner's note link, with pictures from the cloud.
- **Version code:** milestone × 10000 + alpha × 100 + step (a = 1), so α1.2b is 10202.
  The first is above the old app's 1014, so it installs over it.

### A2.4 A fresh cloud session

The setup script installs whatever is missing:
- Godot and its export templates;
- the Android SDK, NDK and JDK;
- CMake and Ninja;
- Mesa's software Vulkan driver (lavapipe) and Xvfb, so Godot can draw pictures without a GPU (research 12).

## A3. The simulation core (research 02)

### A3.1 Its boundary

The simulation is a library with a small surface:
- create or open a world;
- advance it by a budget of time;
- take commands (your powers, test switches in tests only);
- hand out snapshots;
- save.

It never calls Godot, never reads the camera, and keeps no state about what is on screen (`WLD-13`).
The same library runs the scenes and whole-world tests natively in the cloud (`RES-18`).

### A3.2 Entities and components

- People, animals, plants, things, places, groups and records are EnTT entities with generational IDs.
- Their data are components in tight arrays.
- Systems (rules) run in a fixed order each tick.
- A thing's kind comes from its catalogue entry, and its parts from components the entry lists, as RimWorld does with Defs and Comps.
  There is no class hierarchy of kinds, which Dwarf Fortress called its mistake.

### A3.3 Time

- **Game time is counted in whole ticks** of a fixed length.
  The 60-day year and its seasons are computed from the tick count (`TIM-18`, `TIM-14`).
- **Activities have a start and an end** (`TIM-17`).
  Each is an event on a queue ordered by (time, entity, sequence), so long processes cost nothing until they finish or are interrupted.
- **Speed is how many ticks run per real second, within a frame budget** (A3.8).
  When the phone can't keep up, time slows; detail is never cut (`PRN-11`).

### A3.4 The same bits everywhere (`RES-05`, `TIM-16`)

- **Arithmetic:** IEEE single and double precision only, with no fast-math or contraction (A2.2).
  Transcendental functions (sine, exponent, logarithm, power) come from our own library, never the platform's.
- **Randomness is keyed:** each draw is a counter-based hash of (world seed, entity, purpose, tick, index), so any thread can draw any number in any order and get the same one (research 02).
- **Order:** every loop over entities runs in a defined order.
  No iteration over unordered hash maps decides anything.
  Parallel work is split into fixed chunks whose results are merged in a fixed order.
- **Proof:** the same seeded world runs on x86-64 natively and on arm64 under qemu, and their state hashes must match at checkpoints.
  The phone runs the same check in its self-test.

### A3.5 Catalogues (`MAT-13`, `MAT-14`, `MAT-17`)

- Content lives in TOML files in `data/`, one entry per thing, plant, animal, material, blueprint, illness, custom pattern or model-kit form.
- The simulation loads and checks them at start:
  - fields and ranges;
  - links between entries;
  - the blueprint fit checks (`MAT-17`);
  - plausible values (`MAT-05`).
- Entry IDs are stable names, so saves survive reordering.
- Adding an entry never needs code (`PRN-14`).

### A3.6 Saves (`TIM-08`, `PLT-07`, `PLT-09`, `PLT-10`, `PLT-08`)

- A world is saved in chunks (layers, regions, people, history), each with a format version.
- Saves are written to a new file and swapped in only when complete.
- Old saves are upgraded by a chain of migrations, never deleted; a folder of old saves must open in every build.
- Export and import is one file you can keep.

### A3.7 Talking to Godot

- **Commands in:** a queue of plain records (a power at a place, a speed change), applied at the next tick boundary.
- **Snapshots out:** after each batch of ticks, the simulation fills a double-buffered set of arrays: positions, headings, poses, kinds, colours, and events worth showing.
  Godot copies them straight into MultiMesh buffers and nodes.
  No Godot object is touched from a simulation thread.
- The `sim/bind` classes are thin: they convert, never decide.

### A3.8 Threads and budgets

- The simulation runs on its own worker threads, never on Godot's main thread.
- Each frame Godot asks for "as much time as fits in N ms", and the simulation advances that many ticks, then publishes a snapshot.
- Within a tick, systems that are independent run in parallel by fixed chunks (A3.4).

## A4. Drawing (research 03, 04)

### A4.1 The pipeline

As proven in the bake-off (`prototypes/bakeoff/godot/`):
1. **Low resolution.**
   The 3D scene renders into a SubViewport at a quarter of the screen size, one art pixel to 4 × 4 screen pixels (`PRE-22`), and is shown scaled up with nearest sampling.
2. **The camera locked to the pixel grid.**
   - The camera is orthographic for the close views.
   - Its position snaps to the art-pixel grid in its own axes.
   - The leftover fraction shifts the scaled image by part of a pixel, so pans are smooth and pixels never crawl (`PRE-22`).
3. **Outlines and lit edges** (`PRE-21`): a quad over the whole image reads depth and normals and compares each pixel with its 4 neighbours.
   - A nearer pixel over a farther one darkens.
   - An outward edge lightens.
   - Leaves, grass and water are drawn after it and carry none.
4. **Light in three bands** (`PRE-20`, `PRE-30`):
   - one shared light function, written once, used by every lit material;
   - real shadows from the sun's shadow map, hard-edged;
   - the sky's cool light fills the shade.
5. **Grass, leaves and flames** as cards drawn from one sprite sheet; **water** with depth tint, foam, sparkles and sky colour (`PRE-26`).

The renderer, Forward+ or Mobile, is chosen by the phone's measured frame times (pre-production, see the plan).
Forward+ gives the normal buffer the outline pass reads; on the Mobile renderer, a cheap pass of our own draws normals instead.

### A4.2 Materials

One shader per kind of surface, each including the shared light function and the Bayer pattern (`PRE-20`):

| Material | What it does |
|---|---|
| ground | patches of three greens with a fine pattern only where two meet; surface weights for path, bank, bed and rock |
| rock | strata on sides, horizontal in the world so layers run across a cliff (`PRE-23`); a mossy top texture on upward faces |
| textured | made things by their UVs, at 16 texture pixels a metre |
| leaf | leaf-cluster cards tinted by the species' ramp, lit as one round crown, swaying |
| grass | tufts and flowers as instanced cards, lit and shadowed as the ground at their foot |
| figure | people and animals: block bodies coloured by looks, clothing and state (`PRE-27`) |
| water | depth tint, foam line, sparkles, sky colour, and later reflections |
| flame | four frames of fire, unlit |

### A4.3 The ground at every distance (`PRE-03`, `PRE-29`)

- **Near:** areas of detailed ground (A6.3), drawn as chunks in rings of detail round the camera, as clipmaps do.
  Plants, rocks and grass are MultiMesh instances per chunk, fading out by distance.
- **Middle:** world-cell ground under its cover colours, with simple stand-ins for forests.
- **Far:** the map look, flat-coloured cells with river lines and shaded hills.
  Then the globe, a sphere painted from the world map (`WLD-02`).
- Godot draws relative to an origin that moves with the camera, while the simulation keeps exact coordinates (research 05).

### A4.4 Camera and zoom

- One continuous zoom from one person to the globe (`PRE-03`).
- The camera is orthographic and pitched for the close views.
  It tilts toward straight down as it rises (`PRE-29`).
- Turning eases to rest, and panning moves in whole art pixels (`PRE-22`).
- Time's speed follows the zoom (`TIM-01`).

### A4.5 Tests

Golden pictures from Godot on the software Vulkan driver at the phone's art size, compared within a small tolerance.
A contact sheet and a model sheet are drawn for your review at every milestone (`PRE-31`).

## A5. The art pipeline (research 04, 13)

### A5.1 Generators

The bake-off's `make_art.py` grows into `art/`, which makes everything from catalogue parameters and is the same on every run:
- textures in hue-shifted ramps at 16 pixels a metre;
- the model kit: about 8 plant forms, about 6 animal body patterns, the block figure, things as layouts of parts, rocks and cliffs (`PRE-46`).

It writes glTF and PNG into `game/assets/generated/` at build time.

### A5.2 Variety and materials

- Each thing varies its proportions, lean, wear and colour by its own keyed chance (`PRE-43`).
- Its parts take the colours of the materials used (`PRE-42`).

### A5.3 Animation (`PRE-44`)

- Each movement is 2 to 6 key poses of the block figure, made by code and held about 10 times a second.
- Variants bend the poses by rule.
- Feet meet the ground through Godot's inverse kinematics (SkeletonModifier3D).

### A5.4 Review

The model sheet and the texture contact sheet are rendered in the cloud whenever the kit changes, for your eye.

## A6. The world (research 05)

### A6.1 Layers (`WLD-12`)

- **World cells** about 1 km across hold elevation, rock, soil, climate, cover, water and animal totals.
- **Weather cells** hold the hour's weather (`WLD-16`).
- **Areas** are detailed ground made near people and near the camera.
- **Kept areas** are areas where people changed something.

### A6.2 Generation (`WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`)

1. Plates.
2. Elevation from plate edges.
3. Erosion by rivers, glaciers and slopes.
4. Climate on the torus (`WLD-01`), with winds, rain shadows, and temperature by latitude and height.
5. Rivers by flow accumulation, and lakes.
6. Soils and biomes.
7. Deposits by geology (`WLD-14`).

Several candidate worlds are generated and scored, and the best three are offered (`WLD-10`).
The start region is found by scoring (`WLD-24`).
Every stage runs on several threads in fixed chunks (A3.4), and its numbers are tuned until the map looks like the art guide.

### A6.3 Detail on demand (`WLD-13`)

An area is made from its world cell and the seed whenever it is needed, the same every time.
Making it for the picture changes nothing in the world.

### A6.4 Starting small

The generator is first built and tuned on a small island world, about 60 km across, quick to make and to judge by eye, then grown to the full size (`WLD-03`).
Whole worlds come before any people, so no land set by hand stands in for them: the first region of `WLD-34` is proposed for retirement with the new milestones (`SCP-16`).

## A7. Living things (research 06), outline

- Species are catalogue entries: climate and soil ranges, seasons, sizes, yields, life cycles, model-kit forms (`WLD-31`, `WLD-32`).
- Placement is by density rules from the cell, through keyed chance.
- On world cells, cover and animal totals grow, graze, hunt and die by predator-and-prey rules driven by the weather (`WLD-18`).
- Near people, they become individuals, then return to the totals.
- Herds move by steering rules plus goals.

## A8. People and minds (research 07), outline

- **Bodies:** levels for food, water, warmth, rest and health, wounds by body part, illnesses, a life cycle and inheritance (`BIO`).
- **Choosing by utility:** every known action is scored against needs, personality, mood, plans and beliefs, and drawn among the best by keyed chance.
  The top reasons are kept for the card (`MND-09`, `PRN-13`).
  A small planner breaks jobs into steps, re-checked at each step.
- **Feelings and memories:** experiences become feelings, then memories at three depths.
  Deep memories can change personality, with a date.
- **Knowledge per person, with its source:** choices read only what the person knows (`PRN-01`, `MND-02`).
- **Discovery and teaching:** skills spread by imitation and teaching, and can be lost.
- **Pathfinding:** connected regions, then cluster paths, then A* inside a cluster.

## A9. Culture and history (research 08), outline

- A naming language per people, built from sounds, syllable shapes and word-parts, and derived when peoples split (`CUL-17`).
- Customs, taboos, rites and beliefs are data rules created by experience and spread by teaching and talk (`CUL-06`, `CUL-20`, `CUL-05`).
- History is the simulation's event log, kept for good (`PRN-15`), with the facts each later view needs.

## A10. The game layer, outline

- **Powers** are commands into the simulation, acting only through natural systems (`GOD-05`).
- **The story director** is a story sifter over the event log.
  It chooses only what to show and when to skip (`TIM-02`, `TIM-03`, research 09).
- **The book of ages** is written from pattern sentences, replacement grammars over each entry's facts (`PRE-37`).
  The writer, Gemini Nano through ML Kit's Prompt API in a small Android plug-in, only rewords, and each rewording is checked (`PRE-41`).
- **Screens:** the world fills the screen, with bottom sheets in portrait and a side column in landscape, our own gesture reader, and a pixel theme (research 10).
- **Sound:** layered ambience from what is there, and the murmur from each people's language (research 11).

## A11. Testing and checks (research 12)

- C++ unit tests (doctest) for the simulation, run natively.
- Seeded scenes with pass rules stated first (`RES-09`, `RES-21`).
- gdUnit4 for the Godot side, run headless.
- Golden pictures from the software Vulkan driver.
- The same-history check across x86-64 and arm64 (A3.4).
- `tools/check.sh` runs the formats, lints, builds, tests, the file check and the coverage check, and must pass before work joins the main version (`PRC-10`).

## A12. Budgets and risks

- **Frame:** at least 97% of frames on time while moving the camera (`PLT-04`).
  The phone's numbers from the bake-off app set the first real budget for each pass.
- **Simulation:** the speeds of `TIM-07`, measured on the phone with benchmark worlds each milestone (`PLT-04`).
- **Risks and fallbacks:**
  - **The PowerVR driver:** prefer Vulkan, measure early, and keep the Mobile renderer as the fallback (research 01).
  - **Determinism drift:** the same-history check on every change to the simulation.
  - **Gemini Nano's API changing (alpha):** the pattern sentences stand alone.
  - **The APK's size:** about 30 MB from Godot itself, within the 50 MB limit for files committed to the repository.
  - **Godot upgrades:** pinned versions, upgraded deliberately between milestones.
