# Kindling: architecture

How Kindling is built: its parts, how they talk, the rules they keep, and why each choice was made.
It serves `PROJECT.md`, which says what the game must be, and is served by `IMPLEMENTATION.md`, which says in what order to build it.
Every choice cites the research note behind it (`research/NN-*.md`), where the sources are, and the look follows the art book (`art/book/`).
The code is the index: code names the `PROJECT.md` items it implements, so this file says how and why, never where (`CLAUDE.md`, rule 3).

## Status (4 October 2026)

- Rewritten for your OK from the research redone on 4 October (research 00 to 17) and the art book you accepted as the starting point for the look.
  The Rust architecture stays in git history at commit `ebaeae3`, and the first Godot version of this file at `f881525`.
- **Technology approved by you** (`PRC-03`): Godot 4.7, its source unchanged, with the simulation in C++ as a Godot plug-in (GDExtension) (research 01).
- **Pre-production comes first** (research 00): a prototype for each risk, then a vertical slice.
  A part marked *to prove (Pn)* is the design we expect; prototype Pn in the plan measures it, and its answer is written here before production builds on it.
- Parts for later milestones are outlines, each designed in full when its milestone is next, from what the earlier ones taught.

## A1. Overview

### A1.1 What it must deliver

- **The art book's look on your phone** (`PRE-01`, `PRE-02`, `VIS-14`): crisp 3D pixel art, smooth, in portrait and landscape, on a Pixel 11 Pro XL with a PowerVR graphics chip (research 01, 02, 04, 05).
- **A big believable simulation** (`PRN-11`, `MND-14`): thousands of people with full minds in a world 2,000 by 1,000 km (`WLD-03`), time running faster or slower but never cutting detail.
- **The same history every run, on the phone and in the cloud** (`RES-05`, `TIM-16`).
- **Offline, private, saved always** (`PLT-03`, `PLT-07`).
- **Built and tested by AI agents** in cloud sessions, with you judging the look and the feel (`PRC-01`, `RES-22`).

### A1.2 The big picture

```
            your touches                      what you see and hear
                 |                                     ^
                 v                                     |
  +-------------------------------------------------------------------+
  |  game/ (Godot): scenes, shaders, interface, sound                 |
  |  view/ (C++ plug-in): the bridge, the model kit, crowds, the      |
  |        ground's levels of detail on Godot's RenderingServer       |
  |      sends COMMANDS  ------------------->                         |
  |      reads SNAPSHOTS <-------------------                         |
  +---------------------------------|---------------------------------+
                                    |  the boundary (A3.8)
  +---------------------------------v---------------------------------+
  |  sim/ (C++ library): world, living things, people, minds,         |
  |  culture, history, saves. Never reads Godot. Same bits            |
  |  everywhere (A3.4).                                               |
  +---------------------------------|---------------------------------+
                                    |
                     data/ catalogues and tuning files (text)
```

- The simulation owns the truth.
- Godot only draws a snapshot of it each frame, and passes your gestures and powers in as commands.
- Nothing on screen can change history (`WLD-13`, `TIM-03`).

### A1.3 Decisions

| Decision | Why | Research |
|---|---|---|
| Godot 4.7, its source unchanged | Reached the look fastest in the bake-off; stable; much help online; everything asked is within reach | 01 |
| The Mobile renderer on Vulkan | Your phone's PowerVR driver: OpenGL ES runs through a translation layer, compute shaders that read images fail | 01, 02, 04 |
| The simulation as a separate C++ library on its own threads | Thousands of minds need the processor's cores; C++ is Godot's official plug-in language on Android | 01, 03 |
| EnTT entities, content as data | Data laid out for the cache; new plants, animals, things and blueprints without code (`PRN-14`) | 03 |
| Events on one queue, keyed chance, Box2D's rules for floating point | The same bits everywhere; long processes cost nothing until they end | 03 |
| A quarter-size picture, a pixel-locked camera, outlines from rebuilt normals, light in clean steps | How the reference pictures are made, built for the Mobile renderer | 04 |
| The art book is the look, kept by the art bible's rules | You accepted it; the rules keep generated content on it | 05 |
| The model kit built by code at load, drawn as MultiMesh copies; block figures with no skeletons | Countable content; colour and style per copy; cheap crowds | 17 |
| A generator in the order of real causes, many candidates scored | Believable worlds from a seed, tuned for the look | 06 |
| Our own levels of detail on Godot's RenderingServer, a moving origin, a map look that bends into the globe | No plug-in does our mix; Godot draws in single precision | 07 |
| Species as data, numbers by Damuth's law, individuals near people and counts far away | Nature that holds over centuries at any speed | 08 |
| Needs, energy, wounds and illness by real numbers; births by biology | Lives that land near foragers' real numbers | 09 |
| Choosing by utility with kept reasons, a small planner on top, knowledge per person | Explainable (`PRN-13`), reactive, cheap enough for thousands | 10 |
| Blueprints match characteristics, never names; every reality rule backed by an experiment | Discovery by the world's own rules (`PRN-07`) | 11 |
| Culture from causes; a naming language spelled with the font's letters | Nothing social scripted (`CUL-07`) | 12 |
| A story sifter that only sets speed and moments; pattern sentences; the phone's model only rewords, checked | The director never touches events (`TIM-03`); language models describe, never decide (`PRN-06`) | 13 |
| One column of panels, our own gesture reader, an integer-scaled pixel font | The world first, one thumb, crisp text in both orientations | 14 |
| Layered ambience, sounds made by code, Godot's 3D audio, a voice manager | A lively camp from what is really there (`PRN-10`) | 15 |
| doctest and property tests, gdUnit4, pictures by Movie Maker mode, Perfetto on the phone | Every check runs in the cloud; the look is judged on the phone | 16 |

## A2. Code layout, builds and delivery

### A2.1 Repository layout

```
game/        the Godot project: scenes, shaders, the interface in GDScript, the theme and fonts
view/        C++ for the picture, as a Godot plug-in: the bridge to the simulation (A3.8), the model kit's
             shapes (A6), crowds as MultiMesh copies, the ground's levels of detail (A8)
sim/         the simulation in C++20, with no Godot, and its tests: maths, chance, time, catalogues, saves,
             world, living things, people, minds, culture, history
data/        catalogues, tuning files and test scenes, as TOML (A3.6)
android/     the small Android plug-in (screen rate, heat forecast, the writer) and the release certificate
tools/       setup, checks, builds, delivery, the file check, signing
art/         the art book and its painter (A5.1), and the pixel fonts
research/    the research notes this file cites
prototypes/  pre-production's throwaway prototypes, each deleted once its answer is written here
dist/        the signed APK of the latest alpha and its note
```

### A2.2 Builds

- **C++:** CMake and Ninja, C++20.
  - `sim/` builds natively for the cloud (x86-64 Linux, for tests) and for the phone (arm64 Android, with the NDK), and never includes Godot.
  - `view/` links godot-cpp, pinned to Godot 4.7, and carries `sim/` into the app.
- **Compiler rules for `sim/`** (A3.4): warnings as errors, no fast-math, no fused multiply-add (`-ffp-contract=off`).
- **Godot:** 4.7, its version and export templates pinned, exported from the command line (`--headless --export-release`), as in the bake-off.
  - The export is unsigned; `zipalign` and `apksigner` finish it, so only `tools/signing-key.py` reads the secret.
  - Android export needs ETC2 and ASTC texture imports on, and the preset leaves out `addons/` and `test/`, so the test framework never reaches the phone.
  - Godot's template sets Android API 24 to 36 and asks for no permissions.
- **Targets:** Android arm64 for the phone, and Linux x86-64 for tests and pictures in the cloud.
  There is no web build: Godot's is about 40 MB, beyond the private page's 15 MB (research 01).

### A2.3 Delivery of each alpha (`PRC-11`, `PLT-06`)

- **The APK:**
  - signed with the release key, derived from the passphrase secret by `tools/signing-key.py`, the only script that reads it;
  - package `dev.kindling.app`, so each alpha installs over the last;
  - committed to `dist/` on the work branch and linked from the note.
- **The note** says what is new, what to try, what is rough, the items delivered and the links, and is published at your note link with pictures from the cloud.
- **Version code:** (milestone + 1) × 10000 + alpha × 100 + step (a = 1), so pre-production's α0.2b is 10202 and α1.2b is 20202.
  The first is above the old app's 1014, so it installs over it.
- **Self-check:** the first start of each version runs a few seconds of checks (the same bits, a save and reopen) and shows the graphics driver's version, with a short code to send if anything fails (research 02).
  Godot gives the driver's version only inside the id of its pipeline cache, so the self-check reads it there, shown as Vulkan packs versions beside the raw number, since some makers pack theirs differently.

### A2.4 A fresh cloud session

`tools/setup.sh` installs whatever is missing, pinned and checked by checksum, and says nothing when all is present:
- Godot and its export templates;
- the Android SDK, NDK and JDK;
- CMake and Ninja, clang-format and clang-tidy, and the formatters and linters of GDScript (gdtoolkit) and Python (ruff);
- gdUnit4;
- Mesa's software Vulkan driver (lavapipe) and Xvfb, so Godot can draw pictures without a graphics chip (research 16).

## A3. The simulation core (research 03)

### A3.1 Its boundary

The simulation is a library with a small surface:
- create or open a world;
- advance it by a budget of time;
- take commands (your powers; test switches in test builds only, `RES-10`);
- hand out snapshots;
- save.

It never calls Godot, never reads the camera, and keeps no state about what is on screen (`WLD-13`).
The same library runs scenes and whole worlds headless in the cloud, under the same rules as play (`RES-18`, `PLT-05`).

### A3.2 Entities and components

- People, animals, plants, things, places, groups and records are EnTT entities with generational IDs, their data in tight arrays.
- Systems (rules) run in a fixed order.
- A thing's kind is its catalogue entry, and its parts are the components the entry lists, as RimWorld's Defs and Comps.
  There is no class hierarchy of kinds, which Dwarf Fortress regretted.

### A3.3 Time

- **The clock counts whole game seconds,** and the 60-day year, its seasons and dates come from it (`TIM-18`, `TIM-14`).
- **Work happens at events on one queue,** ordered by (time, entity, sequence):
  - each activity ends at an event, and the person decides again only then, or when something interrupts it (`TIM-17`);
  - timers, such as a hide drying, are events (`MAT-19`);
  - each layer of the world runs at its own pace as events: weather hourly, water daily, plant cover every five days (`WLD-12`).
- So a long process costs nothing until it ends, and the cost of a game day follows what happens in it, not the speed.
- **Speed is how much game time runs per real second, within the frame's budget** (A3.9).
  When the phone can't keep up, time slows; detail is never cut (`PRN-11`).
- **The screen moves smoothly at any speed:** the view places each walker along its path between the start and end of its activity, so nothing in the simulation runs per frame.
- *To prove (P6):* that the event queue holds a thousand people at a year a minute.

### A3.4 The same bits everywhere (`RES-05`, `TIM-16`)

Following Box2D and Factorio (research 03):
- **Arithmetic:** IEEE single and double precision, with no fast-math and no contraction (A2.2).
  No platform maths function is used except the square root: sine, exponent, logarithm and power come from our own library.
- **Order:** every loop over entities runs in a defined order, and no iteration over an unordered map decides anything.
  Parallel work is split into fixed chunks, gathered, then applied in entity order.
- **Proof:** a seeded world runs on x86-64 and on arm64 (under qemu in the cloud, and on your phone), on one core and on four, and checksums of the whole state must match at checkpoints.
  The phone runs the same check in its self-check (A2.3).
- *To prove (P5):* identical results on your phone's chip and in the cloud.

### A3.5 Chance

- Every draw is keyed by (world seed, system, being, tick, purpose, index) through a counter-based generator, Philox or a Squirrel-style hash (research 03).
- So any thread can draw any number in any order and get the same one, and adding a new kind of draw never shifts the others (`TIM-16`).

### A3.6 Catalogues and tuning (`MAT-13`, `MAT-14`, `MAT-17`)

- Content lives in TOML files in `data/`: one entry per thing, material, plant, animal, blueprint, illness, custom pattern or model-kit form.
- The simulation loads and checks them at start: fields and ranges, links between entries, the blueprint fit checks (`MAT-17`) and plausible values (`MAT-05`).
- Entry names are stable, so saves survive reordering, and adding an entry never needs code (`PRN-14`).
- Every tunable number lives in a catalogue or a tuning file, never in code.

### A3.7 Saves (`TIM-08`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`)

- A world is saved as versioned chunks: layers, regions, people, history.
- Each save records the migrations it has been through; old saves are upgraded by a chain of migrations, never deleted, and a folder of old saves must open in every build.
- A save is written to a temporary file, flushed, checked, then renamed over the old one, keeping one backup.
- A small, fast save runs whenever the app leaves the screen, since Android may close it there (`TIM-05`).
- History is appended as it happens, never rewritten (`PRN-15`).
- Export and import is one file you can keep.

### A3.8 Talking to Godot

- **Commands in:** a queue of plain records (a power at a place, a speed change), applied at the next event boundary, in order.
- **Snapshots out:** after each batch of game time, the simulation fills a double-buffered set of arrays: positions and paths, headings, poses, kinds, colours, and events worth showing.
  `view/` copies them straight into MultiMesh buffers through Godot's RenderingServer.
- No Godot object is touched from a simulation thread, and `view/` converts but never decides.

### A3.9 Threads and budgets

- The simulation runs on its own worker threads, up to your phone's four middle cores; the fastest core and the small ones stay for Godot, sound and the system (`PLT-01`).
- Each frame Godot asks for as much game time as fits the frame's budget, and never waits for the simulation.
- **Heat:** the Android plug-in reads the phone's thermal headroom forecast, and time slows before the phone throttles (research 02).
- **Watch the known killers from the first benchmark:** pathfinding at scale, temperature fields and lines of sight (research 03).

## A4. Drawing (research 04, 02)

### A4.1 The picture

1. **Low resolution.**
   The 3D world renders into a SubViewport at a quarter of the screen's width and height, one art pixel to 4 × 4 screen pixels (`PRE-22`), on the Mobile renderer over Vulkan.
   It is shown scaled up with nearest sampling; the interface draws at full resolution over it.
2. **A camera locked to the pixel grid** (`PRE-22`).
   - It is orthographic and pitched for the close stops, and snaps to whole art pixels in its own axes.
   - The leftover fraction shifts the scaled image by part of a pixel, so pans are smooth and pixels never crawl.
   - Turns ease to rest; the fix that best lessens crawling in a free turn or zoom is chosen in P1, with you.
3. **Outlines and lit edges** (`PRE-21`).
   A full-screen pass compares each pixel's depth and normal with its four neighbours: a nearer pixel over a farther one darkens to the thing's own darker shade, and an outward fold lightens.
   The Mobile renderer gives no normal buffer, so normals are rebuilt from depth.
   Grass, leaves and water are drawn after the pass and carry none.
   - *To prove (P1):* the four ways of research 04 (rebuilt normals; depth only; a second low-resolution camera drawing normals; enlarged back faces), each for its cost and its look.
4. **Light in clean steps** (`PRE-20`, `PRE-30`).
   - One shared light function, written once, used by every lit material.
   - Each material's ramp of 4 to 7 shades; the light picks the step, and steps meet in clean edges with no pattern mixing them: the sharp look you chose.
   - Hard sun shadows from a shadow map at mobile size, and cloud shadows.
   - Shade filled by the sky's cool purple-blue light, never black; hollows darker; haze by distance, warmer toward the sun; mist on water and in hollows.
5. **Fire** (`MAT-18`): a warm, flickering light as bright as its heat.
   Godot stops lighting MultiMesh copies once its per-object light limit is used up, so fires reach figures and huts through a firelight term in our shaders, fed by a short list of nearby fires, if needed (research 17).
   - *To prove (P2, P3):* a camp lit by three fires at night.
6. **Water** (`PRE-26`), the clear water you chose:
   - shallow water shows its bed, deeper water darkens away from the shore in steps, and a thin bright line marks where water meets land or anything standing in it;
   - the sky's colour on the surface, with glints;
   - the reflection of what stands above it by a second, low-resolution pass of the scene through a mirrored camera, every one of our shaders discarding what lies below the water in that pass, since Godot 4.7 has no clipped camera projection (research 01).
   - *To prove (P1):* the mirrored pass's cost; if it is too dear, reflections fall back to the sky's colour alone.
7. **At speed** (`PRE-30`, `PRE-29`): once a day passes in under about 10 seconds, the light holds steady from high up and only its tint follows the hour; the map look is always lit so.

### A4.2 Materials

One shader per kind of surface, each including the shared light function and the water clip:

| Material | What it does |
|---|---|
| ground | flat colours of its cover (meadow, dry grass, path, bank, bed, rock) by surface weights, meeting in clean edges |
| rock | strata laid in the world, so layers run across a cliff (`PRE-23`); upward faces take moss or soil |
| made things | the lines and broad shapes of what they are made of, such as seams, lashings and courses of thatch, at 16 texture pixels a metre and never fine grain (`PRE-20`, `PRE-42`) |
| leaf | leaf-cluster cards in the species' ramp, lit as one round crown, swaying |
| grass | tufts and flowers as cards in metres, at their true size, lit and shadowed as the ground at their foot (Godot's `LIGHT_VERTEX`) |
| figure | block bodies coloured by looks, clothes and state, faces with eyes and a mouth (`PRE-27`) |
| water | as A4.1 |
| flame | frames of fire, unlit, sized by its heat |
| map | world cells in flat cover colours, shaded hills, rivers as lines, the shore's bright line (A8.5) |

### A4.3 Rules for the phone (research 02)

- Vulkan only, with no compute shaders that sample images; effects are full-screen fragment passes.
- Shadow maps at mobile sizes; pipelines precompiled at load, which Godot backs with ubershaders, so there is no shader stutter.
- The screen runs at 60 Hz, set through the Android plug-in, since Godot's frame cap alone leaves it at 120.
- The first phone builds exercise every rendering feature: shadows, MultiMesh, transparency and every shader trick, since a driver bug found late is the most expensive kind.

### A4.4 Tests

- Golden pictures drawn by Movie Maker mode on the software Vulkan driver in the cloud, compared within a tolerance, approved by you when they change (research 16).
- They guard our code only: the phone's chip may round differently, so the look is judged on the phone (`PRE-31`).

## A5. The look (research 05, the art book)

### A5.1 The art book

- `art/book/` holds the look the game is built toward: a scene for each age, the zoom stops from one person to the globe at noon and dusk, the model kit's sheets and the interface plates, at true size, with the design canvas.
  You accepted it on 4 October 2026 as the starting point, to be tweaked as the real game takes shape on the phone.
- Its painter, a small three.js tool, made those pictures with the same art rules.
  It stays a tool for trying a change before the game has it, never a second copy of the game's code: once the vertical slice reaches the look, changes are made in the game and pictured by the game (`CLAUDE.md`, rule 4).

### A5.2 The art bible's rules

The art bible is research 05's rules, as the art book applied them:
1. **Tie-breaker:** the art book wins over realism, and readability wins over detail: a person, an animal or a fire reads at every zoom (`PRE-28`).
2. **Light:** a warm sun; shade filled by a cool purple-blue sky light; the default camera looks with the sun behind its left shoulder.
3. **Shadow colour as a value:** the deepest shade is a set dark purple, never black; black only deep inside caves.
4. **The smallest thing worth making** is about a fist-sized stone.
5. **Ramps:** each material has 4 to 7 shades with hue shift and the most saturation mid-ramp, from one master palette (`PRE-20`).
6. **Nature soft, made things crisp:** ground, grass, leaves and water in flat patches meeting in clean edges; rock, wood, hide, reed and bone show the lines and broad shapes of how they are made, never fine grain (`PRE-20`).
7. **Edges:** outlines a darker shade of the thing's own colour, never black; outward edges catch a lighter shade (`PRE-21`).
8. **The Stone Age from archaeology:** hide tents and windbreaks, dome huts, hearth rings and working floors, as excavated camps show (`PRE-42`).
9. **Variety that shows:** silhouette, size and colour vary within a kind; differences nobody can see don't count (`PRE-43`).
10. **Mistakes to avoid:** a carpet of grass noise; grain and speckle; pure black outlines or shadows; saturated cartoon green everywhere; smooth gradients over large areas; textures at different pixel sizes side by side; things that differ only in ways nobody can see.
11. **Review:** a contact sheet and a model sheet, made by code from fixed scenes at every change of the art (`PRE-31`, `PRE-46`).

### A5.3 One texel density

Every texture is drawn at 16 texture pixels a metre, about one to one art pixel at the close camp zoom, so nothing looks as if it came from another game.
Grass, reeds, flowers and flames are sized in metres, so they keep their true size at every zoom.

## A6. The model kit and animation (research 17)

### A6.1 The kit (`PRE-46`)

- Each form's shared shape is built at load by our C++ in `view/` as an `ArrayMesh`, from the kit's parameters in the catalogues.
- Plants come from 8 parametric forms, animals from 6 body patterns, and people from one block figure (`PRE-27`).
- A new thing is a catalogue entry, and its model follows from its parts.

### A6.2 Copies

- Models are layouts of shared shapes, drawn as MultiMesh copies grouped by area, since a MultiMesh is culled as one.
- Each copy carries its material's colour, its wear and its maker's style in per-instance data, so huts of birch and of reed look different with no new art (`PRE-42`, `PRE-43`).
- Icons are rendered once per kind from the model, through a SubViewport set to update once.

### A6.3 Animation (`PRE-44`)

- Each movement is 2 to 6 key poses of the block figure, stepped about 10 times a second; our C++ poses the rigid parts, with no skeletons.
- Rules bend the poses (stoop, limp, slump, hunch), each figure's seed offsets its timing, and feet meet the ground by a two-bone sum.

### A6.4 Sheets

The model sheet of every kit shape in two materials, and the animation sheet, are rendered by Movie Maker mode in the cloud whenever the kit changes, for your eye.
*To prove (P3):* about 10 shapes, 2 plants, 1 animal, the figure with 5 movements and a hut in two materials, at noon and at night.

## A7. The world (research 06)

### A7.1 Layers (`WLD-12`)

- **World cells,** about 1 km across, 2,000 by 1,000 on the torus (`WLD-01`, `WLD-03`): height, rock, soil, climate, cover, water, deposits, snow, fire and the herds passing.
- **Areas,** about 256 m across and detailed to about a metre, made only where needed.
- **Things and creatures** in areas, or on the cells' ground.
- **Weather cells,** about 10 km across (`WLD-16`).

### A7.2 Generation, in the order of real causes (`WLD-08`, `WLD-09`)

1. Plates, approximated as Procedural Tectonic Planets does, with ranges, volcano lines and rifts along their edges.
2. Rock, from each cell's place among the plates.
3. Uplift and erosion by the stream power law, FastScape-style, with slopes and glaciers, so rivers cut branching valleys.
4. Priority-Flood for basins, lakes and flow, so every river reaches the sea or a lake (`WLD-17`).
5. Climate on the torus: temperature by latitude, height and season, winds, and rain shadows by a linear orographic model (`WLD-16`).
6. Soils by Jenny's factors (`WLD-27`), and plant types by BIOME1's five numbers.
7. Deposits by geology: flint in chalk and limestone, clay in flood plains, obsidian at volcanoes, copper over subduction (`WLD-14`).
8. Settling, until the world is in a believable present state (`WLD-08`).

### A7.3 Candidates (`WLD-10`, `WLD-24`)

About 20 candidates are made at a coarse size, scored, and rejected with logged reasons, as Dwarf Fortress does; the best few are made at full size, and the best three are offered.
The start is found by scoring (`WLD-24`).

### A7.4 Determinism and tuning

- Processor only, in fixed chunks, under A3.4's rules, so a seed makes the same world on the phone and in the cloud, checked by hashes.
- Tuned for the look: maps and close-ups are judged against the art book's zoom stops, and the climate is checked on Earth's real relief, as Around The World did.

### A7.5 Detail on demand (`WLD-13`)

- An area is made from the seed, its world cell and neighbours, and the date, the same every time; making it for the picture changes nothing.
- Only what people change is kept (`WLD-12`).

### A7.6 Starting small

The generator is first built and tuned on a small island, quick to make and judge by eye, then grown to full size.
*To prove (P7):* one candidate at coarse and at full size, and the settling run, timed on your phone (`WLD-11`).

## A8. From a person to the globe (research 07)

### A8.1 Levels of detail (`PRE-03`)

Our own system in `view/`, driving Godot's RenderingServer directly:
- **Near,** within about 300 m from camp zoom inward: areas as detailed chunks in rings round where the camera looks, as clipmaps do;
- **Middle,** out to about 10 km: each area's coarse ground, shaped every few tens of metres, under its cover;
- **Far:** the world cells as one map mesh, then the globe.

Each level fades into the next by dithering, so the pixel look never blurs, and a full area shows its coarse ground until its detail fades in, within about a second.

### A8.2 A moving origin

Godot draws in single precision around an origin that moves with the camera, as Kerbal Space Program does; the simulation's exact coordinates never depend on it.

### A8.3 Things by distance (`PRE-28`)

- Grass and small stones only near.
- Trees as models, then impostor cards, then the cover's colour.
- People, herds and camps as models, then tiny figures outlined in a darker shade of their own colour, then markers: a banner for a camp, one mark for a herd.

### A8.4 The camera (`PRE-03`, `PRE-29`)

- Orthographic and pitched for the close stops, tilting toward straight down as it rises.
- Perspective for the globe: the flat map bends onto a sphere for the last step, as Google Maps morphs to its globe, squeezing the polar lands and hiding the seam under the ice (`WLD-02`).

### A8.5 The map look (`PRE-29`)

World cells in flat cover colours, rivers as lines (from the region out, those draining about 1,000 km² or more), hills shaded the cartographers' way, lit from high up at every hour with only the tint following it, sea in depth bands with the shore's bright line (`PRE-26`).

### A8.6 Time and light by zoom (`TIM-01`, `PRE-30`)

One gesture sets where you look and how fast time runs, from real speed at the person to top speed at the globe.
From the valley out a day passes in under a second, so the light holds steady.

### A8.7 Budget

Each level's cost is measured on your phone at every zoom stop (`PLT-04`), and so is the time to make a full area.
*To prove (P8):* one pinch from the globe to a person over unvisited land, smooth at every stop.

## A9. Living things (research 08), outline

- Each species is a catalogue entry: climate and soil ranges, seasons, size, diet, group size, yields, life cycle, model-kit form and colours (`WLD-31`, `WLD-32`).
- Plants by ecology: which types can grow from BIOME1's numbers, which win by dominance, how dense by competition and self-thinning; single plants placed from that density by keyed chance, so a place always grows the same plants (`WLD-13`).
- Animal numbers by Damuth's law: each species' natural density from its body mass, a sixth of it per cell (`WLD-30`); predator and prey rules on the cells' totals, driven by the weather (`WLD-18`).
- Herds' days by need zones and hours, their years by following the green-up, their movement by Reynolds' steering rules plus those goals.
- Near people, individuals; far away, counts, with condition and wariness carried both ways (`WLD-32`).
- *To prove (P9):* 20 worlds with nobody in them for 100 years, every species within half and twice its total.

## A10. People: bodies and lives (research 09), outline

- Needs as levels that run down at their own rates, and things and places advertise what they offer, as in The Sims (`BIO-09`).
- Energy by real numbers, by body size and activity; food values from the catalogue (`BIO-10`).
- Wounds by body part and layer, with bleeding, pain, disabled limbs and infection per wound (`BIO-13`); illness as a race between severity and immunity, care slowing severity (`BIO-05`, `BIO-23`).
- Births by biology: fertility from age and nourishment, gaps from breastfeeding (`BIO-15`); inheritance blended with variation, rare traits passed by chance (`BIO-06`).
- Whole worlds must land near foragers' real numbers: about half of children reaching 15, adult deaths peaking near 70, three-year birth gaps, illness the main cause of death (`RES-14`, `BIO-04`).

## A11. Minds (research 10), outline

- **Choosing by utility:** every known action is scored by data-driven response curves against needs, personality, mood, plans and beliefs, and drawn by keyed chance among the best; the top reasons are kept for the card (`MND-09`, `PRN-13`).
- **A small planner on top** for jobs of several steps, each step re-checked by utility, so people still react to a wolf.
- **Decisions only when an activity ends or is interrupted,** never per tick (A3.3), spread over threads in fixed chunks.
- **Feelings by appraisal; mood as summed thoughts** with thresholds moved by traits; memories at three depths that can change personality, with a date (`MND-19`, `MND-29`, `MND-30`, `MND-18`).
- **Knowledge per person with its source:** seen, told by whom, worked out; beliefs can be wrong, misremembered or spread by talk, and choices read only this, never the world's truth (`PRN-01`, `MND-02`, `MND-23`).
- **Opinions move in talk;** social acts and norms are data rules over relationships and personality (`MND-33`, `CUL-24`).
- **Paths in levels:** connected regions, then cached cluster paths, then A* or flow fields inside clusters, recomputed only where the land changes.
- **The estimate:** a thousand people at a game year a real minute is about 48,000 decisions a second, some 80 µs each on four cores, before bodies, talk and paths (`TIM-07`, `MND-15`).
  *To prove (P6):* a thousand simple minds with needs, choice, talk and paths, on your phone at held speed.

## A12. Crafts and discovery (research 11), outline

- An item's 18 characteristics come from its material and form, and made things inherit from their inputs (`MAT-03`).
- Blueprints match characteristics and classes, never names (`MAT-04`, `PRN-07`), fenced by the expected-fits check (`MAT-17`).
- Every reality rule has a real experiment behind it, cited in its catalogue check (`RCK-01` and the rest of section 7.6).
- Quality from skill and inputs (`MAT-20`); skill grows by the power law, a few years to competence and five to ten to mastery (`MND-06`); teaching beats watching (`MND-13`).
- Discovery belongs to people: accidents, personal hunches and copying found things (`MND-11`); crafts die with their last holder and return only by rediscovery, neighbours or copying (`CUL-02`, `CUL-16`).
- *To prove (P4):* whether tuning alone gives sharp flakes within 5 years and fire within its window (`RES-02`, `RES-03`, `TIM-19`, `RSK-01`).

## A13. Culture (research 12), outline

- A naming language per people from the seed, in O'Leary's way, spelled only with letters the pixel font has (`CUL-17`, `CUL-18`).
- Customs as fixed questions with a few answers, each answer from a band's own cases (`CUL-06`).
- Beliefs and rites from coincidences: the act before a good outcome becomes a rite, harm after an act a taboo (`CUL-05`, `CUL-20`, `CUL-34`); small bands keep vivid, rare rites, large villages regular ones (`CUL-26`).
- Societies with real numbers: bands of about 28 adults, a few families linked by kin and marriage; leaders kept in check; gifts as insurance; villages only where stores allow (`CUL-30`, `RES-07`).
- Violence in its real order: personal killings and revenge first, raids growing with stores (`CUL-31`).
- Stories and gossip drift as transmission chains do; styles drift by copying with small changes (`CUL-11`, `CUL-12`).
- *To prove (P10):* customs, a spirit, a rite and a band split arising inside their windows from their own causes (`CUL-33`).

## A14. Story, the book of ages and the writer (research 13), outline

- **The director** keeps Left 4 Dead's rhythm without its power: peaks, then a guaranteed rest; it reads the world and sets only speed and live moments (`TIM-02`, `TIM-03`).
  A test runs a world with it on and off and compares the results.
- **Recognisers** are story-sifting patterns over the event log; half-matched ones are the director's signs, so time slows before an outcome without looking ahead (`PRE-39`).
- **Pattern sentences:** a small grammar, at least 5 phrasings for each kind of event, picked by the event's seed and filled from its records (`PRE-37`).
- **The writer:** Gemini Nano through ML Kit's Prompt API, with a fixed seed, sentence by sentence, behind the Android plug-in; it writes only while the app is in front, queues and backs off, and stops for the day at its battery quota.
  Every rewording passes a strict check without any model, and dark events never reach it (`PRE-41`, `PRE-17`); pattern text always works alone.
- *To prove (P11, P13):* the director's budget on recorded worlds, and the writer's pass rate, speed and quotas on your phone.

## A15. The interface (research 14)

- **The world fills the screen;** panels show only what the moment needs, then fade (`PRE-32`).
- **One column of panels:** full width with controls in the bottom third in portrait, beside the world in landscape (`PRE-34`).
- **Every control at least 48 dp,** with 8 dp between.
- **Our own gesture reader on raw touches,** so all gestures share one rule set and a scripted test can tell them apart; edge swipes stay out of the system's gesture insets (`PRE-33`).
- **One Godot theme in the art bible's palette.**
  The plain pixel font for everything read, at whole multiples of its design size, nearest filtering and no subpixel positioning; the pixel handwriting only for big titles, at twice the size, as the art book's interface plates show.
  The layout is built on a square base, so both orientations scale alike; safe areas and cutouts come from `DisplayServer`.
- **Cards open to what matters now,** with deeper sections folding out (`PRE-35`); screen-reader labels come with Godot 4.5's support.
- *To prove (P12):* thumb reach, gesture misreads in a scripted test, and crisp text, in both orientations.

## A16. Sound (research 15), outline

- Ambience as layers from the place's land, water, weather, hour and season, plus one-shots only from real things near the camera (`SND-11`, `PRN-10`).
- The 44 base sounds made by our C++ at load and varied each play; live synthesis only for what follows the world continuously, such as fire by its heat (`SND-06`).
- Godot's 3D players for direction and distance, their low-pass for distance, an area with reverb for each cave; muffling by land from the simulation's line test (`SND-08`).
- A voice manager keeps `SND-01`'s 32 voices, blending the quietest into its kind's hum when a share is full (`SND-07`).
- The murmur in the people's language, shifted for age, build and feeling (`SND-03`); the speaker's missing bass restored by harmonics, off with headphones.
- *To prove (P14):* a camp with 32 voices, distance filters, a cave reverb and the murmur, without breaks.

## A17. Testing and checks (research 16)

- **C++ tests** with doctest, and property tests with RapidCheck for rules that must always hold, such as no result heavier than its inputs (`MAT-09`).
- **Scenes and whole worlds** run by the C++ library alone, many at once in the cloud, their pass rules stated before their first run and counted over about 20 runs where chance matters (`RES-21`, `RES-09`, `RES-13`).
- **The same results everywhere** (A3.4): state hashes compared between x86-64 and arm64, and between one thread and four (`RES-05`).
- **The Godot side** with gdUnit4: layouts, cards and views opened from records, headless; gestures by simulated touch under Xvfb, since Godot's headless mode drops input events.
  Every script is compiled before the tests, so an error in one no test loads still stops the check.
- **Pictures and reels** by Movie Maker mode at a fixed frame rate: golden pictures in the cloud; the contact sheet and the sound reel on the phone for your reviews (`PRE-31`, `SND-12`).
  - Movie Maker records at the project's base size, so a single picture at the phone's 1344 × 2992 pixels is read from the screen by our script (`tools/picture.sh`).
  - A rendering driver named on the command line brings Forward+ unless the Mobile renderer is named beside it.
- **Phone measurements:** the in-app benchmark writes our own trace events into Perfetto traces, beside the chip's speed and heat; Android GPU Inspector for a slow frame on the PowerVR chip (`PLT-04`).
- **One command before anything joins:** `tools/check.sh`, rebuilt for C++ and Godot, runs the formats, lints, builds, tests, the same-results check, and the file, commit and coverage checks (`PRC-10`, `PRC-12`).
  The vertical slice passes through every kind of check before production starts.

## A18. Budgets and risks

### A18.1 Budgets

Starting estimates, each replaced by what the prototypes measure on your phone at held speed (research 02):
- **Frame:** 16.7 ms at 60 frames a second, the graphics chip under about 8 ms in the busiest scene, so heat leaves room; at least 97% of frames on time while moving the camera (`PLT-04`).
- **Simulation:** up to the four middle cores at held speed (`PLT-01`), at the speeds of `TIM-07`.
- **Power:** about 3 W while playing; **memory:** within about 8 GiB.
- **The APK:** 25 MB from Godot itself (measured at α0.1a), within the 50 MB limit for files committed to the repository.

### A18.2 Risks

| Risk | Proven by | If it fails |
|---|---|---|
| The PowerVR driver mishandles a feature we use | P1 to P3 | avoid that feature; the self-check reports the driver |
| The look costs too much on the Mobile renderer | P1 | a cheaper outline method; fewer cards |
| Mirrored water costs too much | P1 | reflections of the sky's colour alone |
| Fires leave MultiMesh copies unlit | P2, P3 | the firelight term in our shaders |
| A busy camp misses 60 frames a second, or heats the phone | P2 | fewer cards and smaller shadow maps; time slows earlier |
| Discovery can't be tuned to its pace (`RSK-01`) | P4 | the discovery rules redesigned with you before production |
| Phone and cloud results differ (`RSK-04`) | P5 | our own function for whatever differs |
| A thousand minds miss a year a minute (`RSK-02`) | P6 | profile and simplify scoring; the population target revisited with you (`MND-15`) |
| Making a world takes too long (`WLD-11`) | P7 | fewer coarse candidates; settling while you choose |
| The zoom stalls | P8 | a coarser middle level; coarse chunks made ahead |
| Nature's totals drift (`WLD-18`) | P9 | damping in the predator and prey rules |
| Culture fails to emerge (`RSK-19`) | P10 | templates (`CUL-05`), with you |
| The director misses moments or overspends | P11 | its recognisers and budget retuned |
| Gestures misread or text blurs | P12 | gesture rules and scale retuned |
| The writer falls short (`RSK-08`) | P13 | pattern text stands alone (`PRE-37`) |
| Sound breaks up (`RSK-28`) | P14 | fewer voices |
| Godot upgrades break things | each milestone | pinned versions, upgraded between milestones |
