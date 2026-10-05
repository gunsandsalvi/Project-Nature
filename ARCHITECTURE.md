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

Each folder arrives with its first file: `game/` with the vertical slice, `sim/` and `data/` with the first C++ prototype.

### A2.2 Builds

- **C++:** CMake and Ninja, C++20.
  - `sim/` builds natively for the cloud (x86-64 Linux, for tests) and for the phone (arm64 Android, with the NDK), and never includes Godot.
  - `view/` links godot-cpp at its newest release, 4.5, which Godot 4.7 loads (P5), and carries `sim/` into the app.
  - *Built in P5:* each C++ prototype is also a Godot extension, built natively for the cloud's tests by `tools/check.sh` and for arm64 Android with the NDK by `tools/build.sh`, 16 KB aligned, so the app runs the very code the cloud runs; the libraries are built, never committed.
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
- *Measured in P6:* a queue of five-minute windows over two days holds a thousand people at about 7 game years a real minute on one core of the cloud and 12 on four, and 6 on your phone's four (A11).

### A3.4 The same bits everywhere (`RES-05`, `TIM-16`)

Following Box2D and Factorio (research 03):
- **Arithmetic:** IEEE single and double precision, with no fast-math and no contraction (A2.2).
  No platform maths function is used except the square root: sine, exponent, logarithm and power come from our own library.
- **Order:** every loop over entities runs in a defined order, and no iteration over an unordered map decides anything.
  Parallel work is split into fixed chunks, gathered, then applied in entity order.
- **Proof:** a seeded world runs on x86-64 and on arm64 (under qemu in the cloud, and on your phone), on one core and on four, and checksums of the whole state must match at checkpoints.
  The phone runs the same check in its self-check (A2.3).
- *Proved in P5 in the cloud,* in C++ as pre-production code: a toy world of 4,096 walkers on events, with keyed chance, our own sine, cosine, exponent, logarithm and power, and each evening's sums gathered in fixed chunks of 256, ends each of its 30 days with the same checksum on x86-64 and on arm64 under qemu, on one thread and four (its digest 1bbbe1d787b4d4fe).
  - Our functions agree with the platform's to within a few last bits; they use only IEEE adds, multiplies and divides, with no contraction into fused multiply-adds.
  - *Proved on your phone* (5 October 2026): the Pixel 11 Pro XL's chip gives the cloud's digest on one thread and on four, through the app's extension built with the NDK.
    P6's thousand minds did too: their checksum after the 3,546 game days of your phone's run on four threads is the one the cloud's replay of the same run reached that day.

### A3.5 Chance

- Every draw is keyed by (world seed, system, being, tick, purpose, index) through a counter-based generator, Philox or a Squirrel-style hash (research 03).
- So any thread can draw any number in any order and get the same one, and adding a new kind of draw never shifts the others (`TIM-16`).
- *Built in P5:* SplitMix64's finaliser over the seed, the being, the tick, the purpose and an index.

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
   The 3D world renders into a SubViewport at a quarter of the screen's width and height, one art pixel to 4 × 4 screen pixels, on the Mobile renderer over Vulkan.
   *Changed with your OK on 5 October 2026:* an art pixel is about 2 × 2 screen pixels at the person, growing with the zoom to about 6 × 6 at the globe (`PRE-22`), as P8's second round draws it (A8.1).
   It is shown scaled up with nearest sampling; the interface draws at full resolution over it.
2. **A camera locked to the pixel grid** (`PRE-22`).
   - It is orthographic and pitched for the close stops, and snaps to whole art pixels in its own axes.
   - The leftover fraction shifts the scaled image by part of a pixel, so pans are smooth and pixels never crawl.
   - *Chosen after P1:* "ease": the view locks to whole art pixels as it pans, and a turn or zoom eases to rest on the nearest whole step, 5° or 1.25 times, when the fingers lift.
     It keeps `PRE-22` as decided, and is the most thorough of the four fixes P1 tried; you left the choice to me on 4 October 2026.
     After P3 you found the turns snapped: the steps were 15°, so a turn could swing 7.5° on its own after you let go. They are now 5°, at most 2.5°, eased more gently (α0.2c's fixes).
3. **Outlines and lit edges** (`PRE-21`), the art book's own way (C), chosen after P1; you left the choice to me on 4 October 2026.
   - A second low-resolution camera draws each pixel's facing and depth into a half-float picture, from the materials compiled for that pass (A4.2), since the Mobile renderer gives no normal buffer.
   - The shared light function compares each pixel with its four neighbours there: a nearer pixel over a farther one darkens to the thing's own darker shade, and an outward fold lightens. So every shape, crowns and figures too, gets its outline and its lit edge.
   - **That picture is drawn first in each frame:** its viewport sits inside the picture's, since Godot draws a viewport's own viewports before it and others in the order they came. Drawn after, the picture read the last frame's outlines, which jumped whenever the camera moved: the flicker you saw in P1. A picture taken while the camera slides into place now matches one taken still, where before 2,998 of its pixels differed. The mirror's picture is drawn first the same way.
   - On your phone D held still where C flickered, but D draws no lit edges and outlines solid shapes only.
   - **Every shape drawn has its copy in that picture,** made by the one function that adds shapes: P2's second build added figures, tents and trees without one, and C read each of them as all edge, darkening it whole.
   - **Easy to switch, as you asked:** the outline way is one setting, and each way is its own pass behind it, so another can replace C later without touching the materials.
     P1's code for the other three stays in git: A, normals rebuilt from Godot's depth texture by a quad over the picture; B, depth alone; D, each shape drawn again, enlarged, behind itself.
   - *Measured in P1:* in the cloud's software drawing, against no outlines, A and B cost about the same, D about a tenth more and C about a third more.
     On your phone (4 October, at 1080 × 2404 and 60 frames a second) every way kept 99–100% of frames on time, at 9.5 to 10.1 ms of graphics time a frame on average, C the dearest by half a millisecond.
4. **Light in clean steps** (`PRE-20`, `PRE-30`).
   - One shared light function, written once, used by every lit material.
   - Each material's ramp of 4 to 7 shades; the light picks the step, and steps meet in clean edges with no pattern mixing them: the sharp look you chose.
   - Hard sun shadows from a shadow map at mobile size, and cloud shadows.
   - Shade filled by the sky's cool purple-blue light, never black; hollows darker; haze by distance, warmer toward the sun; mist on water and in hollows.
   - *Built in P1:* the Mobile renderer draws forward, so the painter's separate passes become one.
     Each material's fragment function hands its ramp, pattern, openness to the sky and firelight to the shared light function, which picks the step after the sun's shadow and writes the graded, hazed colour, with the surface colour white and Godot's ambient light off.
     Openness to the sky comes from a height map drawn once from above.
     In the cloud, 87% of the close camp's pixels at noon and 76% at dusk come within 3 levels of 255 of the art book's; most of the rest are shadows and edges a pixel apart.
5. **Fire** (`MAT-18`): a warm, flickering light as bright as its heat.
   - *Built in α0.2c's fixes:* flames are the art book's own flame pictures from the atlas, three or five standing over a fire, red at their edges and white-hot at their core, in four frames that change 10 times a second with a few sparks above: P3's fires and P2's tent fires have them, while the painted hearth of P1 and P2 keeps the art book's own flame.
     Each change also sets a fire's flicker, 82% to 100% of its light, felt within about 1 to 2.5 m of its flames only, so the pool's rim holds still (after α0.3's review).
     Firelit colour leans to a warm ramp of the pixel's own brightness, as the painter's does, so lit grass reads amber, not olive; and a fire's light is lost in full sun: by day it shows only in shade, so sunlit ground round a fire takes no pale disc.
   - *Built in P2:* our firelight term, summing up to four fires as the painter does, lights the instanced figures and the tents near each fire; small creatures take a fire's light round their sides too (0.65 of it), so a figure by a fire reads as lit from wherever the camera stands.
     The shaders work in world coordinates, so a moved or instanced shape gets its fire, sky and patterns where it stands, not where it was built.
   - **Camp zoom** puts the camera 1 km back, past the nearest ground in view, and stretches the sun's shadows, the haze and the outline depths to the view's 1.2 km of ground; beyond the sky's height map, the sky is open.
     *After the review (α0.2c's fixes):* the forest stands in patches on a meadow, as the art book's camp zoom has it, the meadow at the height and in the cover of the camp's ground where it ends, so no square shows round the camp, except at dusk, when the cliff's long shadow stops where the painter's scene ends; the crowns are wider and drawn without outlines, which made each tree a dark twig; the ground's small plants, specks of a pixel or less there, are left out; and people are drawn four times their size, as tiny figures (`PRE-28`). The fires' height maps and smoke are off there.
   Godot stops lighting MultiMesh copies once its per-object light limit is used up, so fires reach figures and huts through a firelight term in our shaders, fed by a short list of nearby fires, if needed (research 17).
   - *Proved in P2:* a camp lit by three fires at night holds 60 frames a second (A18.1).
   - **Shadows from every fire,** as you asked after P2 (*built in α0.2c*): people and things cast a shadow from each fire as well as the sun, and stand in each other's.
     Two maps of the heights round the fires, 48 m across at 512 pixels, are drawn every frame as people move, inside the picture's viewport so they are ready before it: the tops of what stands there, seen from above, and its undersides, the faces turned down, seen from below.
     For each fire, a point walks the line to 1.5 m above the fire's light, 2 m above the fire, in even steps of at least 0.2 m, at most 32: where the line runs through something, between its underside and its top, or passes from above a thing to below it between two steps, the point is in that fire's shadow.
     So a fire under a roof or an overhang lights the ground round it, while a tent, a person or a windbreak stops its light; foliage, and anything more than 2.4 m above the fires, stays out.
     The line ends above people's heads, so a whole figure casts its shadow from a fire, as you asked.
6. **Water** (`PRE-26`), the clear water you chose:
   - shallow water shows its bed, deeper water darkens away from the shore in steps, and a thin bright line marks where water meets land or anything standing in it;
   - the sky's colour on the surface, with glints;
   - the reflection of what stands above it by a second, low-resolution pass of the scene through a mirrored camera, every one of our shaders discarding what lies below the water in that pass, since Godot 4.7 has no clipped camera projection (research 01).
   - *To prove (P1):* the mirrored pass's cost; if it is too dear, reflections fall back to the sky's colour alone.
   - *Built in P1:* forward drawing leaves the water no record of the land round it, so its shore line is drawn where the water is thinnest, from the depth texture: within about an art pixel of the bank.
     In the cloud the mirrored pass adds about a quarter to the frame; on your phone at most 0.3 ms, so the reflections stay.
7. **At speed** (`PRE-30`, `PRE-29`): once a day passes in under about 10 seconds, the light holds steady from high up and only its tint follows the hour; the map look is always lit so.
8. **Smoke and mist.**
   - *Built in α0.2c, as you asked:* smoke is a lit volume, not the painter's flat puffs. Its path is worked out at load from the scene: up from the fire, along the underside of a roof or overhang to its edge, then up and away on the wind, widening as it goes.
     Round the path, a box is drawn whose every pixel walks its ray through it in 24 steps, stopping at whatever stands there, so what stands in the smoke shows through it; noise climbing with time stirs it.
     The fire below lights it; its colour is a step of the smoke's ramp, a mid grey by day and dark at night, and its thickness five steps of opacity, so its edges are clean like everything else.
     By day it is torn into drifting puffs with gaps between them, its thin parts taking the sun's tint, since a smooth sheet read as pale glass (α0.2c's fixes).
     Godot blends in linear light, which would thicken and brighten it, so its blend is solved against the screen's copy, to mix over the picture's stored colours as the painter mixes (P1).
   - Mist is not drawn yet: the painter's flat puffs in P1's close camp are hidden, and the smoke's volume took their place by its fires. Mist on water and in hollows comes with the world's weather (`WLD-07`).

### A4.2 Materials

One shader per kind of surface, each including the shared light function and the water clip.
Each extra pass a material takes part in, such as the outline data picture, the sky's height map, the mirror or the hull, is the same shader file compiled with a `#define`, so nothing is written twice (P1).

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

- **Your phone, by its self-check** (α0.1a, 4 October 2026):
  - Android 17 (API level 37, build 16238327);
  - the screen at 1080 × 2404 pixels, 390 dpi and 120 Hz, four fifths of the panel's 1344 × 2992 each way, as its Screen resolution setting allows;
  - the graphics chip a PowerVR C-Series CXTP-48-1536 MC1, driver 1.662.3024 (6908880 as Vulkan reports it), Vulkan 1.4.317.

  At 4 × 4 screen pixels an art pixel (`PRE-22`), that screen shows 270 × 601 art pixels, where the art book's plates, drawn for the full panel, show 336 × 748.
- Vulkan only, with no compute shaders that sample images; effects are full-screen fragment passes.
- Shadow maps at mobile sizes; pipelines precompiled at load, which Godot backs with ubershaders, so there is no shader stutter.
- The screen runs at 60 Hz, set through the Android plug-in, since Godot's frame cap alone leaves it at 120.
- The first phone builds exercise every rendering feature: shadows, MultiMesh, transparency and every shader trick, since a driver bug found late is the most expensive kind.
- **The cloud's software Vulkan driver** crashes drawing P2's smoke from some angles near 15°, inside the driver's own threads (α0.2c's fixes); whether your phone's driver does is to be seen in the next build, and the cloud's pictures of P2 are taken at 16°.
- **Godot's rules met in P1:**
  - front faces wind clockwise, the opposite of three.js, so the painter's triangles are reversed on import;
  - a pass never declares the picture it draws into, since Vulkan refuses a texture that is both its target and its input;
  - varyings are written only in their stage's own function, and the light function has no vertex position, so depth reaches it in a varying;
  - a shadow bias of 0.08 and a normal bias of 1.6 on the 4096 map keep dusk's low sun free of stripes.

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
- *Built in P3,* in GDScript as pre-production code: eleven shared shapes, two plants and a deer, each in two materials, from a catalogue of parameters, built at load in about 23 ms.
  After your P3 comments and the review, the windbreak is the art book's: poles, a bar and brush packed against them with a ragged top and twigs, where a flat hide read as tiled boards; and the lean-to's roof lies in three overlapping courses of sheets with ragged edges, its ends closed with brush, where a thin slab read as a rack; α0.3's review found the roof's facing turned 86°, which left the courses flat on it, now set square to its slope. The carrier's bundle rides on the back, clear of the head and face.
  Each vertex carries its material's row of the palette, its step up or down, its surface pattern and its flags, and its place in its own part, for patterns such as a face.

### A6.2 Copies

- Models are layouts of shared shapes, drawn as MultiMesh copies grouped by area, since a MultiMesh is culled as one.
- Each copy carries its material's colour, its wear and its maker's style in per-instance data, so huts of birch and of reed look different with no new art (`PRE-42`, `PRE-43`).
  *Built in P3:* the hut is one shape drawn as two MultiMesh copies; each copy's data holds its material's row, its pattern (bark or thatch), how worn it is and a seed, and the shared shader gives the parts of the shape flagged to take them that material, and darkens a share of patches a step in places set by the seed.
- Icons are rendered once per kind from the model, through a SubViewport set to update once.

### A6.3 Animation (`PRE-44`)

- Each movement is 2 to 6 key poses of the block figure, stepped about 10 times a second; our C++ poses the rigid parts, with no skeletons.
- Rules bend the poses (stoop, limp, slump, hunch), each figure's seed offsets its timing, and feet meet the ground by a two-bone sum.
- *Built in P3:* walk and carry in 4 key poses, knap, scrape and rest in 2, each pose the angles at the hips, knees, shoulders and elbows, the body's bend, the head's nod and the pelvis's drop to sit or kneel, with what the hands hold; each pose a shape of its own, built at load, swapped in place at its step. The head bears a face of two eyes and a mouth.
  Between each two key poses come the poses halfway, or a third and two thirds, so every step shows a new pose, 10 a second (`PRE-44`), where a held pose changed only 2.5 to 5 times a second; each movement wears its own clothes, a pale fur or a red-dyed tunic, so the figures no longer read as one brown (α0.2c's fixes). P2's camp uses the same figure and movements.

### A6.4 Sheets

The model sheet of every kit shape in two materials, and the animation sheet, are rendered in the cloud whenever the kit changes, for your eye.
*Built in P3:* the sheet is a screen of the prototype app, at noon, dusk and night with three fires; the cloud draws its pictures by the app's own picture option, which saves the low-resolution picture itself, since Movie Maker mode records only at the project's base size.

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
- *Built in P8,* in C++ as pre-production code: the ground's height anywhere, from the cells' heights joined smoothly between their middles, with relief in eight steps from 400 m down to 3 m, rougher where the cells are steep and none at the shore; its colour from the cover, whose edges are warped by noise at 800 m and 200 m so cover meets cover along natural lines, not the cells' squares; and trees by keyed chance on a 5 m grid, as many as the cover holds, each standing in exactly one chunk.
- *P8's second round* adds a chunk of ground at any spacing, from the cells' heights averaged into ever coarser copies and relief only as fine as the spacing shows, so ground seen from far off is never noisier than its pixels; with its normals, the grid half as fine it morphs into (A8.1) and its lowest and highest points, for the camera's culling. The cells' climate, cover and water go to the picture as textures, and the clouds' noise with its smaller copies.

### A7.6 Starting small

The generator is first built and tuned on a small island, quick to make and judge by eye, then grown to full size.
*Measured in P7 in the cloud,* in C++ as pre-production code, with every stage of A7.2 in a first, simple form: on four cores of the cloud's x86-64, 20 candidates at 512 × 256 cells take about 5 seconds and the best 4 made again at 2,048 × 1,024 about 12, so three worlds are offered in about 17 seconds; settling the first for 10 years takes about 10 more. On one core: about a minute, and 16 seconds.
- A candidate takes about 0.8 seconds on one core and a world at full size about 9: erosion about half, the plates and rock a quarter, the climate an eighth.
- One thread and four make the same worlds, as arm64 under qemu does at a sixteenth of the size.
- So `WLD-11`'s 3 minutes and 1 leave room for richer stages: more erosion at full size, glaciers, and settling's real rules.
- *Measured on your phone* (5 October 2026), the Pixel 11 Pro XL on four cores: three worlds in 9.3 seconds (20 candidates in 2.5, the best 4 in 6.8) and settling in 6.5, faster than the cloud; its digest is the cloud's, so it made the very same worlds; its heat forecast rose from 0.44 to 0.54, with no slowing.
  So generation has about twenty times the room `WLD-11` gives it.

## A8. From a person to the globe (research 07)

### A8.1 Levels of detail (`PRE-03`)

Our own system in `view/`, driving Godot's RenderingServer directly: one tree of square chunks over the whole world, each splitting into four as the camera nears, so the ground is detailed within about 300 m from camp zoom inward, shaped every few tens of metres out to about 10 km, and the world cells' own from the region out.
Each chunk morphs into the grid half as fine as the camera draws away (CDLOD, Strugar 2010), so no level pops and the pixel look never blurs, and a full area shows its coarse ground until its detail is in, within about a second.
*Changed in P8's second round,* after your verdict on 5 October 2026: the rings, the map mesh and their dithered hand-overs, built first, gave way to the tree, since the hand-overs and the map's bend were what you saw jump.

*Built in P8,* over P7's world, in GDScript and C++ as pre-production code, drawn through the RenderingServer:
- **Rings, as clipmaps do:** the near ground every metre to 128 m, a full area across, and every 2 m to 300 m, its trees as models; the middle ground every 40 m to 5 km and every 80 m to 16 km, which covers the valley stop's tall portrait picture; trees beyond 300 m as cards to 900 m, where the camp's tilted view ends (A8.3); then the map.
- **Chunks** of 64 m to 3.2 km, made on Godot's worker threads from the seed, nearest first, at most six at once; each ring is planned again only when the focus has moved half a chunk.
- **Handing over:** each ring draws where all its chunks round the focus are made, and gives way to the finer ring inside it over the last sixth of that ring's radius by a 4 × 4 ordered dither, the two sharing the pixels so none is empty or drawn twice; it shrinks to nothing over the last halving of its scale, so detail recedes as you zoom out rather than switching off.
- **What a stop keeps,** at the start region's open grassland: about 0.56 million triangles at the valley, 0.9 million at the camp and 1.2 million from the close camp in, most of them outside the picture.

*Built in P8's second round,* in GDScript and C++ as pre-production code:
- **The tree:** 4 × 2 roots of 500 km, fourteen levels down to chunks 30.5 m across, each 33 × 33 points with a skirt hanging from each edge so no crack shows where levels meet; a chunk splits while the camera is nearer than 2.4 of its sides and its four are made, and morphs into its parent's grid over the last third of its reach.
- **Made as needed:** on Godot's worker threads from the seed, nearest first, at most six at once; at most 900 kept, freed by how long ago the tree last reached them, never one it reached this frame. Freeing by when a chunk was last drawn freed the ones between, so a deep zoom asked for them again without end: the eight minutes the first pictures took.
- **Only what the camera can see:** a chunk behind the planet's horizon, or outside the camera's view once widened by as much as the sphere can move it, is neither drawn nor split. At the river where the descent ends: 25 chunks drawn at the person, 38 at the close camp, 83 at the camp, 66 at the valley, 70 at the region, 21 at the world map and 8 at the globe, where without it the person drew 989. Choosing them takes about 1 to 2 ms a frame in the cloud.
- **Pixels:** the picture is drawn at the size of the pixels shown, so the clouds' marching costs a quarter to a ninth of drawing it at half the screen; or twice that each way for the blended variant, shown as the average of what lies under each pixel (`PRE-22`).

### A8.2 A moving origin

Godot draws in single precision around an origin that moves with the camera, as Kerbal Space Program does; the simulation's exact coordinates never depend on it.

### A8.3 Things by distance (`PRE-28`)

- Grass and small stones only near.
- Trees as models, then impostor cards, then the cover's colour.
- People, herds and camps as models, then tiny figures outlined in a darker shade of their own colour, then markers: a banner for a camp, one mark for a herd.

*Built in P8's second round, the camp and closer,* in GDScript and shaders as pre-production code:
- **One rule for each thing, on the graphics chip:** where each tree stands, by keyed chance on a grid of slots (`WLD-13`), and what the ground shows close up: meadow, dry grass, the woods' floor, mud along the banks, the camp's trodden floor, sand, rock, snow or water.
  Each is written once and read by everything that needs it: the trees up close, the forests' crowns painted on the ground further out, the small plants, and a probe that reads the rules back once to find the camp's place by the river.
  So trees, plants, ground and camp always agree.
- **Trees** from the camp stop inward, as cards traced pixel by pixel into crowns and trunks, a chunk's trees in one draw; each fades into the crowns painted on the ground as its chunk morphs away, and casts the sun's shadow, which reaches 2.5 km.
- **Small plants** only on the finest chunks: the kit's tuft, which the shader sets where the ground grows grass, flowers or reeds.
- **The camp** from the kit (A6): tents, a hut, a lean-to, racks, logs, pots and the hearth, its flames drawn pixel by pixel and lighting what stands round them; the band at work and walking its path to the river; from the camp stop out, a glow at the hearth.
- **Heights** joined by smooth curves between the cells' middles, since straight blends showed a seam up close.
- What it still lacks for production is in `NOTES.md`.

### A8.4 The camera (`PRE-03`, `PRE-29`)

- Orthographic and pitched for the close stops, tilting toward straight down as it rises.
- Perspective for the globe: the flat map bends onto a sphere for the last step, as Google Maps morphs to its globe, squeezing the polar lands and hiding the seam under the ice (`WLD-02`).
- *Changed in P8's second round,* as you asked on 5 October 2026: the world is a sphere at every scale and never unrolls; near the focus the ground is stretched east to west back to its true size as the zoom closes in, so the close stops are as before.
- *Built in P8:* pitched 30° from the camp inward, rising to straight down at the valley; past the world map, perspective, its field of view widening from 10° to 30° as the map bends, so the flat map first looks as it did.
  The map's mesh spans the world round the focus, east and west, with the poles at its edges; bent, each place goes to a sphere as far around as the world, turned so the focus is on top.
  The torus's top and bottom edges both meet at the icy poles, so its seam lies under the ice.
- *Built in P8's second round:* perspective at every stop, pitched 30° at the camp and below, straight down from the valley out (variant A) or only from the world map out (B, a flight that shows the horizon). Each chunk's vertex shader sets it on the sphere by haversine forms that keep the ground near the focus exact; the camera's culling bounds how far that can move it.
  The polar ice is drawn round each pole whatever the cells hold there, its edge and floes read round the pole, so nothing squeezed into it shows.

### A8.5 The map look (`PRE-29`)

World cells in flat cover colours, rivers as lines (from the region out, those draining about 1,000 km² or more), hills shaded the cartographers' way, lit from high up at every hour with only the tint following it, sea in depth bands with the shore's bright line (`PRE-26`).
*Changed with your OK on 5 October 2026:* the land vivid and textured with what can be seen from above, lit by the sun of the hour, with the weather's clouds and their shadows (`PRE-29`), as P8's second round draws it, below.
*Built in P8:* P7's map, a texel a cell, without the rivers it draws itself; the rivers and the shore as lines a pixel wide over it; the sea in four bands 250 m deep; the hills lit from the north-west.
P7's map had its hills lit from the south-east by mistake, which can make ridges read as valleys; it is lit from the north-west now.
*Built in P8's second round,* in variants, one shader for every level, so only detail changes with the zoom (`PRE-29`):
- **The land:** A, the art book's map colours in clean steps of light; B, the cover vivid, forests as clumps of crowns lit on their sun side at every scale the pixels can show, sand and stone in the deserts, hills shaded from the heights and deepened from far off as maps are, warm sun and blue shade; C, B's colours in clean steps. Cover and climate are read a little off where the pixel is, by noise, so no cell's edge shows.
- **The sea:** A, the art book's bands; B, from deep navy to turquoise shallows by depth, with currents and eddies drifting across it and the sun's glint; C, B with waves near the shore.
- **Rivers** as curves through the cells, as wide as the land they drain and never thinner than about a pixel once they show, so they stay as the zoom closes in.

### A8.6 Time and light by zoom (`TIM-01`, `PRE-30`)

One gesture sets where you look and how fast time runs, from real speed at the person to top speed at the globe.
From the valley out a day passes in under a second, so the light holds steady.
*Built in P8's second round:*
- **Weather:** a picture of the whole world's clouds, 2 km a pixel, drawn on the graphics card: the climate's wetness and the belts of rising and sinking air, which follow the sun through the seasons; noise warped by noise into swirls and streams (Quílez), heaped into rounded masses, twisted round the storm tracks' lows and, in summer, a tropical storm, all carried by the trade winds and the westerlies; small fair-weather clouds over warm land (`WLD-16`).
- **Clouds:** A, marched through their 3D shapes and lit through themselves (Schneider 2015, with Wrenninge's octaves for light scattered many times), their heaps lit on the sun's side and their shadows on the ground, so the descent passes among them; B, the same in the art book's clean steps; C, the art book's flat clouds. Their noise is shifted from place to place by the weather's own fields, so its pattern never shows repeating, and a pixel shows only the shapes larger than it.
- **Air:** scattering as Earth's does (Nishita 1993, Hillaire 2020): A, only the rim's glow against space; B, its haze over the ground too.
- **The sun** stands where the date, the hour and the world's tilt put it: morning (35° up in the east, so the land's relief and the clouds' heaps show), noon, dusk, night, or the live hour.

### A8.7 Budget

Each level's cost is measured on your phone at every zoom stop (`PLT-04`), and so is the time to make a full area.
*To prove (P8):* one pinch from the globe to a person over unvisited land, smooth at every stop.

## A9. Living things (research 08), outline

- Each species is a catalogue entry: climate and soil ranges, seasons, size, diet, group size, yields, life cycle, model-kit form and colours (`WLD-31`, `WLD-32`).
- Plants by ecology: which types can grow from BIOME1's numbers, which win by dominance, how dense by competition and self-thinning; single plants placed from that density by keyed chance, so a place always grows the same plants (`WLD-13`).
- Animal numbers by Damuth's law: each species' natural density from its body mass, a sixth of it per cell (`WLD-30`); predator and prey rules on the cells' totals, driven by the weather (`WLD-18`).
- Herds' days by need zones and hours, their years by following the green-up, their movement by Reynolds' steering rules plus those goals.
- Near people, individuals; far away, counts, with condition and wariness carried both ways (`WLD-32`).
- *Proved in P9 in the cloud,* in C++ as pre-production code (`prototypes/worldgen`): P7's worlds from 20 seeds, at the candidates' 512 × 256 cells of about 4 km, each run 30 years into its present-day state, settled 10, then 100 more with nobody in them, in steps of five game days.
  - **The answer:** Yes. In all 20 worlds every species stayed within 0.66 and 1.10 of its settled total for the 100 years, and in every biome it lived in; grass, browse and trees within 0.83 and 1.09; and there were 82 to 99 big plant eaters for each hunter, where `WLD-18` asks 50 to 200. The world's totals hold because its regions' bad years fall at different times: round each world's start region, about 64 km across, numbers swung from 0.21 to 2.31 of their settled level, crashing in a drought or a hard winter and taking 10 to 20 years to mend. Wild sheep and horses in dry country swing most; forest deer and tropical antelope least; hunters, whose prey is never scarce enough to starve them, follow their prey.
  - **The rules,** all on the cells' totals: plant cover growing by the season's warmth and the soil's water, which fire thins and which grows back over years; the weather's years at three scales, a region's of about 64 km, a pattern its neighbours share for several hundred km, and the whole world's; 14 plant eaters at a sixth of Damuth's density for their weight (`WLD-30`), eating what they reach under the snow, green food best, their condition following how full they are, dying first of hunger in a hard winter; 4 hunters taking prey by Holling's second type, the weak and those floundering in snow most easily, their room following their prey's weight by Carbone and Gittleman's rule (2002); each breeding once a year as its condition lets it, fewer as its cell fills; the young leaving for less crowded cells next door.
  - **Lessons for production:**
    - A world made from a rough start drifts for decades, so production makes it directly near its balance, as `WLD-08` asks; 30 years stood in here.
    - A game winter must cost an animal what a real one does, or the weather moves nothing: at first every species held within 2% of its total.
    - Dry country's plants live on less water than a meadow's: grown as if they were a wet meadow short of rain, they starved the animals there even in a normal year, and wild sheep in 3 of 20 worlds crashed in a drought and stayed down. They now grow by the soil's water partly against the place's own, so a normal year feeds dry country's animals and a drought costs them.
    - Young animals leave every cell for less crowded ones next door, more from crowded cells, so land emptied by a hard year is found again.
  - **Its cost:** 8 to 19 seconds a world for its 141 years on four cores of the cloud's x86-64.

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
  *Measured in P6 in the cloud,* in C++ as pre-production code: a thousand people in 40 bands, each with nine needs, 50 actions scored by response curves with their reasons kept, talk passing places, opinions and news, and trips by paths in levels, make about 32,000 decisions a game day, about 32 each, near `TIM-17`'s 10–30 activities a day.
  - One core of the cloud's x86-64 runs them at about 7 game years a real minute, four cores at about 12: a person costs about a seventh of `MND-15`'s thousandth of a second a game day, before bodies and the rest of a full mind.
  - Choosing takes over half the time, talk a sixth, paths a seventh, results landing a twenty-fifth, and the rest (where everyone stands, the queue, the hourly mood) a tenth.
  - Four cores give less than twice one: each five minutes of game time hands about a hundred choices to the threads, and what must happen in order between them (results landing with what talk passes on, and the snapshot of where people stand) is about a third of the time.
  - Paths first took four fifths of the time, with A* inside the first and last clusters of every trip and a cache for each thread; fields of distances from each entrance, made at the start (the flow fields above), and one cache shared by every thread, merged between windows, brought them to a seventh.
    A place in another connected region is passed over before scoring.
  - One thread and four, and arm64 under qemu, end every day the same.
  - P6's people choose one activity at a time: the small planner for jobs of several steps comes with production's minds.
  - *Measured on your phone* (5 October 2026), the Pixel 11 Pro XL on four cores for 10 minutes: 6.0 game years a real minute once warm, over the last 8 minutes, and 5.5 over the first 2; 31,400 decisions a game day; choosing 62% of the time, talk 17%, paths 9%, results landing 3% and the rest 10%; its heat forecast rose from 0.39 to 0.49 of the way to slowing itself, and it never slowed.
    So a thousand people hold `TIM-07`'s hoped-for 2 to 3 game years a minute with room: production's fuller minds may cost about 2½ times P6's before the speed falls below 2½.

## A12. Crafts and discovery (research 11), outline

- An item's 18 characteristics come from its material and form, and made things inherit from their inputs (`MAT-03`).
- Blueprints match characteristics and classes, never names (`MAT-04`, `PRN-07`), fenced by the expected-fits check (`MAT-17`).
- Every reality rule has a real experiment behind it, cited in its catalogue check (`RCK-01` and the rest of section 7.6).
- Quality from skill and inputs (`MAT-20`); skill grows by the power law, a few years to competence and five to ten to mastery (`MND-06`); teaching beats watching (`MND-13`).
- Discovery belongs to people: accidents, personal hunches and copying found things (`MND-11`); crafts die with their last holder and return only by rediscovery, neighbours or copying (`CUL-02`, `CUL-16`).
- *Proved in P4,* in a model of one band in Python, as pre-production code: tuning alone, with the world's own rules, brings sharp flakes and fire within `TIM-19`'s windows, halved at your word on 4 October 2026 for faster discoveries and again on 5 October, so the first village comes about an hour into play (`RES-02`, `RES-03`, `TIM-19`, `RSK-01`).
  - **Flakes:** within 2 years, their window and the sharp-stone test's bar, in 19 of 20 runs, the median about eight months in, and in 20 of 20 on seeds never tuned against; never without stone that flakes.
    They came by accident in 17 runs, by a dream's hunch in 2 and by experiment in 1, and a year after the first every adult could make them, in every run.
  - **Fire,** counted at its first anywhere in a world of 3 or 4 bands, as `TIM-19` counts a step: in Years 2 to 8 in 15 of 20 worlds and none before them, the median about 4½ years in; on new seeds 11 and none.
    The windows are dates (`TIM-14`), so Year 2 begins a year in.
    17 of the 20 worlds' first fires began with a hunch from a dream (`MND-12`), and 3 with an experiment.
  - **What sets the pace:** one discovery factor a blueprint, 0.413 for flakes and 0.136 for each way of making fire, tuned on 20 fixed seeds and checked on 20 new ones (`RES-16`).
  - **What fire's pace rests on:** P4's model knows three blueprints, and a dream's real hunch goes to the sector the dreamer knows best, so as soon as an adult has once rubbed sticks, every such dream points at fire: that is why most first fires began with a dream.
    At the pace of 4 October, with dreams unable to point at fire, the tuned factor brought fire into its window in 2 of 20 worlds, 7 never finding it within 60 years, and about eight times the factor restored it (α0.3's review).
    So tuning alone still sets the pace, but production re-tunes with the whole catalogue and dreams' choice among all its blueprints (`MND-12`, `GOD-03`): these factors don't carry over, and `RSK-01` stays open until M7's scenes.
  - **No value holds it on a knife's edge:** each tuned value changed alone by a quarter either way, over 40 runs and 40 worlds, keeps both steps' rules; halving the flake factor or noticing breaks the flakes' rule, and halving experimenting fire's, so those three are the strong levers.
    Fire's thin side is now lateness: a quarter less hunch or experimenting leaves 24 of 40 worlds' first fire in its window, where the rule needs 20, while no quarter's change brings more than 4 too early, where it allows 10.
  - **What production takes:** each blueprint's discovery factor in its catalogue entry (A3.6); a step counted at its first anywhere in the world (`RES-07`); and tuning runs that keep what they have done, keyed by the model's and the catalogue's version, so one cut short resumes and none is reused by a changed model (`RSK-14`).

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
  - It costs about what changed, since every delivery waits on it: C++ compiles through ccache, so godot-cpp and unchanged files compile once across runs and build folders; each C++ file is linted, on every core, only when its code, the headers it reads, its compile command or the rules changed since it passed, and each project's tests, the Godot project's import and tests, the picture test and each Python prototype's tests run only when something they read changed (`tools/cppcache.py`); the C++ tests run beside the Godot and tool tests; and each step prints its time.
    Measured on 5 October 2026: about 20 seconds with nothing changed, about 45 with one line of one prototype changed, and about 5 minutes the first time, which fills the caches.
  The vertical slice passes through every kind of check before production starts.

## A18. Budgets and risks

### A18.1 Budgets

Starting estimates, each replaced by what the prototypes measure on your phone at held speed (research 02):
- **Frame:** 16.7 ms at 60 frames a second, the graphics chip under about 8 ms in the busiest scene, so heat leaves room; at least 97% of frames on time while moving the camera (`PLT-04`).
  - P1's close camp on your phone (4 October, 1080 × 2404): 99–100% of frames on time at 60, the graphics chip about 10 ms a frame on average whichever the outline way or the mirror.
    Ways that cost a third more in the cloud cost under a tenth more there, which suggests the chip lowers its clock when it has time to spare, so the 10 ms is partly idle.
  - **P2's and P3's phone numbers below count the picture's own pass only.** Their Measure left out the outline data, the mirror and the fires' height maps, which the cloud's timing puts at 17–31% more; Measure now sums every pass (α0.2c's fixes), as P1's did, so the next phone run gives the whole frame.
  - P2 on your phone (4 October): the close camp at night with thirty figures and three fires costs 6.1 ms a frame at 120 frames a second for the picture alone, all on time, so near the 8 ms aim but not shown within it; at 60 it reads 9.9 ms, the slowed clock again.
    The forest at camp zoom, 12,000 trees of 44 triangles drawn three times (picture, outlines, shadow), took 17.8 ms with 65% of frames on time: trees for camp zoom take 12 triangles.
  - P2's last build (4 October): the close camp at night 5.5 ms a frame at 120 frames a second, 99% on time; at 60, the close camp and the forest at camp zoom had every frame on time (8.3 and 10.8 ms with the slowed clock). **P2 passes on frame rate:** a busy camp at night with three fires holds 60 frames a second, at close and camp zoom.
    **Its heat question is still open:** the plan's run is about 10 minutes, but Measure was cut to about 90 seconds at your word, and over them the phone's forecast of its heat was still rising, from 0.56 to 0.59 and in the next build from 0.61 to 0.66 of the way to slowing itself, read through Godot's Android runtime with no plug-in.
  - P3 on your phone (4 October, α0.2c): the model sheet at night with three fires, their shadows and smoke, 4.3 ms a frame (5.3 at most) at 120 frames a second, all on time; at 60, 6.4 ms at night and 6.6 at noon, every frame on time; the heat forecast 0.63 to 0.64. The picture's pass alone, as above.
- **Simulation:** up to the four middle cores at held speed (`PLT-01`), at the speeds of `TIM-07`.
- **Power:** about 3 W while playing; **memory:** within about 8 GiB.
- **The APK:** 25 MB from Godot itself (measured at α0.1a), 33 MB with P1's scene of the close camp (α0.2a), within the 50 MB limit for files committed to the repository.

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
