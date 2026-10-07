# Kindling: architecture

How Kindling is built: its parts, how they talk, the rules they keep, and why each choice was made.
It serves `PROJECT.md`, which says what the game must be, and is served by `IMPLEMENTATION.md`, which says in what order to build it.
The look aims at the pictures you chose (`art/targets/`).
What is not in `PROJECT.md`, this file or `IMPLEMENTATION.md` is not kept.
The code is the index: code names the `PROJECT.md` items it implements, so this file says how and why, never where (`CLAUDE.md`, rule 3).

## Status (6 October 2026)

- **Technology approved by you** (`PRC-03`): Godot 4.7, its source unchanged, with the simulation in C++ as a Godot plug-in (GDExtension).
  On 6 October 2026 you allowed our own build of Godot 4.7.2 for a few patches to its drawing (a depth pre-pass for leaves, shading once per 2 × 2 pixels, buffers kept off memory), only if M2's calibration on your phone shows they are needed (A2.2).
- **Pre-production is closed** (5 October 2026): its prototypes' answers are written here as decisions, marked with the prototype that gave them (Pn); their code is deleted, and production writes its own.
- The foundations' sections (A2, A3, A17, A18) are written in full for M1; M1 is built and you accepted it on 6 October 2026.
- The graphics engine's sections (A4, A5, A6, and A8's near stops and camera) are written in full for the look you chose on 6 October 2026, with your OK; M2 is under way.
- Parts for later milestones are outlines, each designed in full when its milestone is next, from what the earlier ones taught.

## A1. Overview

### A1.1 What it must deliver

- **The look you chose, on your phone** (`PRE-01`, `PRE-02`, `VIS-14`): a sharp 3D world at the screen's full resolution wearing pixel-art textures, steady and smooth, in portrait and landscape, on a Pixel 11 Pro XL with a PowerVR graphics chip.
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

| Decision | Why |
|---|---|
| Godot 4.7, its source unchanged; our own build of 4.7.2 only if M2's calibration needs its drawing patches | Reached the look fastest in the bake-off; stable; much help online; everything asked is within reach; the look you chose may need a leaf pre-pass and shading per 2 × 2 pixels, which stock Godot lacks |
| The Mobile renderer on Vulkan | Your phone's PowerVR driver: OpenGL ES runs through a translation layer, compute shaders that read images fail |
| The simulation as a separate C++ library on its own threads | Thousands of minds need the processor's cores; C++ is Godot's official plug-in language on Android |
| EnTT entities with never-reused ids, content as data with no floats | Data laid out for the cache; new plants, animals, things and blueprints without code (`PRN-14`); history that names the dead forever |
| Events on one queue run in islands, keyed chance, correctly rounded maths, Box2D's rules for floating point | The same bits everywhere, at any speed and on any number of threads; long processes cost nothing until they end |
| The full-resolution picture with 2× MSAA; texture pixels steadied by a smooth-pixel filter and a level drawn for each zoom band; one shared light function with our own soft shadows and darkening; no outlines up close | The look you chose; steadiness from how textures are read, not from the camera; Mobile has neither shadows that widen nor darkening in corners |
| The feeling as guidance, a target card as an alarm, one approved picture per place relit for its hours; a saving kept only if it passes your blind test | Rules can't hold a feeling, and you judge the look; you turned down every visible saving |
| The model kit's shapes made in Blender as parts, put together by code like Lego and drawn as MultiMesh copies; surfaces from code, the world, or approved pictures prepared by code; a skeleton per body pattern, posed in C++ at 10 a second and bent on the graphics chip | Countable content with the detail you chose; many options from few parts; colour and style per copy; crowds without Godot's cost per figure |
| A generator in the order of real causes, many candidates scored | Believable worlds from a seed, tuned for the look |
| Our own levels of detail on Godot's RenderingServer, a moving origin, a map look that bends into the globe | No plug-in does our mix; Godot draws in single precision |
| Species as data, numbers by Damuth's law, individuals near people and counts far away | Nature that holds over centuries at any speed |
| Needs, energy, wounds and illness by real numbers; births by biology | Lives that land near foragers' real numbers |
| Choosing by utility with kept reasons, a small planner on top, knowledge per person | Explainable (`PRN-13`), reactive, cheap enough for thousands |
| Blueprints match characteristics, never names; every reality rule backed by an experiment | Discovery by the world's own rules (`PRN-07`) |
| Culture from causes; a naming language spelled with the font's letters | Nothing social scripted (`CUL-07`) |
| A story sifter that only sets speed and moments; pattern sentences; the phone's model only rewords, checked | The director never touches events (`TIM-03`); language models describe, never decide (`PRN-06`) |
| One column of panels, our own gesture reader, an integer-scaled pixel font | The world first, one thumb, crisp text in both orientations |
| Layered ambience, sounds made by code, Godot's 3D audio, a voice manager | A lively camp from what is really there (`PRN-10`) |
| doctest and property tests, gdUnit4, pictures by Movie Maker mode, Perfetto on the phone | Every check runs in the cloud; the look is judged on the phone |

## A2. Code layout, builds and delivery

### A2.1 Repository layout

```
game/        the Godot project: scenes, the interface in GDScript, the theme and fonts, and its gdUnit4 tests;
             game/bin/ (the built extension) and game/data/ (a copy of data/) are made by the build, never committed
view/        C++ for the picture, as one Godot extension (libkindling): the bridge to the simulation (A3.8), the
             phone's telemetry (A3.9) and crowds as MultiMesh copies; later the model kit's shapes (A6) and the
             ground's levels of detail (A8)
sim/         the simulation in C++20, with no Godot: numbers, chance, time, entities, events, catalogues and saves,
             later the world, living things, people, minds, culture and history; its doctest tests; the `kindling`
             command-line tool for scenes, runs, benchmark worlds and catalogue checks; sim/thirdparty/ for
             vendored code
data/        catalogues, tuning files, scenes and benchmark worlds, as TOML (A3.6), in sources: base/ for the game,
             demo/ for what only the foundations show
android/     the release certificate; an Android plug-in only if the phone ever needs one (A3.9)
tools/       setup, checks, builds, delivery, the file check, signing
art/         the pictures you chose (targets); the textures: their levels, records, sheets, sources and
             requests (A5.4); the kit's parts as Blender files (A6.1); and the fixed views' golden pictures (A4.8)
dist/        the signed APK of the latest alpha and its note
```

### A2.2 Builds

- **C++:** CMake and Ninja, C++20, through ccache.
  - `sim/` is a static library with its tests and the `kindling` tool, and never includes Godot.
  - `view/` is one shared library, `libkindling`, linking `sim/` and godot-cpp 4.5 built from a trimmed profile (`view/build_profile.json`), which Godot 4.7 loads (P5).
    A class that a used method takes must be named in the profile, or the method silently vanishes.
    godot-cpp 10, which targets Godot 4.7's own interface, is offered after M1.
- **Flags for all our C++** (A3.4): `-std=c++20 -O2 -Wall -Wextra -Werror -fno-exceptions -funsigned-char -fno-fast-math -fno-math-errno -ffp-contract=off`, with `-ffp-contract=off` the last floating-point flag on the line; symbols hidden; `-ffunction-sections -fdata-sections` linked with `--gc-sections`.
  Nothing throws: errors are values.
- **Five builds**:
  - the cloud's main build: x86-64 with clang 18, for the tests, the tool and the extension the Godot tests load;
  - a second compiler: x86-64 with GCC 13 and its undefined-behaviour and float-cast checks;
  - arm64 with GCC 13, and arm64 with the phone's own compiler (NDK r30, clang 21), both static executables run under qemu, for the same-bits check (A3.4); every test runs emulated once, under the phone's own compiler, and both run the same-bits proofs;
  - the phone's: the extension for arm64 Android, API 24, the C++ runtime linked statically, newer Android functions linked weakly and guarded, 16 KB-aligned, stripped in the APK and kept whole for crash symbols.
  - Beside them, since α1.5a, the simulation's tests built once more with GCC's thread checker, since a race between threads can damage memory without failing a test: one did, once, before the checker named it (A17).
- **Godot:** 4.7.2, pinned, exported from the command line (`--headless --export-release`).
  - The export is unsigned; `zipalign` and `apksigner` finish it, so only `tools/signing-key.py` reads the secret.
  - The build first copies `data/` into `game/data/` with `build.toml`, the list of its files and their digests (A3.6), and the scenes' last reports with a world each (A17); the export's filter includes `data/*.toml` and the reports, since Godot skips text files otherwise.
  - Android export needs ETC2 and ASTC texture imports on, for the icon and the interface's pictures; the world's textures are lossless data files with their own levels, read by `view/` (A5.4), so Godot's "Detect 3D" stays off.
    The preset leaves out `addons/` and `test/`, so the test framework never reaches the phone; it asks for no permissions.
  - **Our own build,** only if M2's calibration scenes call for it (your OK, 6 October 2026; A4.1): Godot 4.7.2's Android export templates built from its tagged source in the cloud with at most three small patches kept as files beside the build script (a depth pre-pass for leaves with an equal depth test; buffers that never reach memory; a shading rate for each material), pinned and tested like the stock ones.
    If keeping it proves too costly, the stock templates return and the 3D is drawn at 0.75 scale where a scene needs it.
  - `quit_on_go_back` is off and `retain_data_on_uninstall` on (A3.7).
- **Blender:** each part's Blender file is its source; the build exports every part with its texture layout and joints for Godot (A6.1).
- **Targets:** Android arm64 for the phone, and Linux x86-64 for tests and pictures in the cloud.
  There is no web build: Godot's is about 40 MB, beyond the private page's 15 MB.

### A2.3 Delivery of each alpha (`PRC-11`, `PLT-06`)

- **The APK:**
  - signed with the release key, derived from the passphrase secret by `tools/signing-key.py`, the only script that reads it;
  - package `dev.kindling.app`, so each alpha installs over the last;
  - committed to `dist/` on the work branch and linked from the note;
  - within the 50 MB a committed file may have: 27.6 MiB at M1's end, so pixel-art textures shipped losslessly fit while they stay small; if they stop fitting, you choose between compression you cannot tell from the original in a blind test and delivering builds as release files outside the repository, once that is proved from a cloud session.
- **The note** says what is new, what to try, what is rough, the items delivered and the links, and is published at your note link with pictures from the cloud.
- **Version code:** (milestone + 1) × 10000 + alpha × 100 + step (a = 1), so α1.1a is 20101 and α1.2b is 20202.
  Each is above every earlier build's, so it installs over it.
- **Self-check:** the first start of each version runs a few seconds of checks and shows them, with a short code to send if anything fails:
  - the same bits: seeded runs of the numbers, chance and a small world, each digest against the one the cloud wrote into the build;
  - the floating-point environment a simulation thread finds;
  - the catalogues' digests against the build's;
  - a save written and reopened;
    *built in α1.4a as the last save's time and the moment it holds, not a save made for the check: the kill tests in the cloud prove the saving itself;*
  - the graphics driver's version, read from the id of Godot's pipeline cache, the only place Godot gives it; the screen's refresh rate; the cores and their top clocks; the heat thresholds; and how the storage is mounted.

### A2.4 A fresh cloud session

`tools/setup.sh` installs whatever is missing, pinned and checked by checksum, and says nothing when all is present:
- Godot and its export templates;
- the Android SDK, NDK and JDK;
- CMake and Ninja, clang-format and clang-tidy, GCC 13 and its arm64 cross compiler, qemu, MPFR (the maths library's test oracle), and the formatters and linters of GDScript (gdtoolkit) and Python (ruff);
- gdUnit4, doctest, godot-cpp 4.5, EnTT 4.0.0, toml++, xxHash and zstd;
- Mesa's software Vulkan driver (lavapipe) and Xvfb, so Godot can draw pictures without a graphics chip;
- Blender, to make and export the kit's parts.

CORE-MATH's few C files are kept in `sim/thirdparty/` at a pinned commit, with a sample of its hard cases, since its host is the one source a session might not reach; `tools/core-math.py` copies them from a checkout of that commit.

## A3. The simulation core

### A3.1 Its boundary

The simulation is a library with a small surface:
- make a world from a seed and its sources of data, or open a saved one;
- set a goal in game time, and say how far it has got (its frontier);
- take commands (your powers; test switches in test builds only, `RES-10`);
- hand out snapshots and the events worth showing;
- save, export and import.

It never calls Godot, never reads the camera, the zoom or the speed, and keeps no state about what is on screen (`WLD-13`).
It reads files only as bytes handed to it, and writes saves only under the folder it is given.
The same library runs scenes and whole worlds headless in the cloud, under the same rules as play (`RES-18`, `PLT-05`).

### A3.2 Entities and components

- **EnTT 4.0.0** holds people, animals, plants near people, things, places, groups and records, behind a thin layer (`sim/ecs`), so no rule creates or destroys entities itself and a later change of library stays local.
  - Two registries keep memory tight: beings (people, animals, places, groups) with 32-bit handles, and things with 64-bit handles, room for more than a million at once.
  - Plants and far animals stay as counts on world cells, outside EnTT (`WLD-32`).
- **Every entity has an id that is never reused:** 64 bits from one world counter, its top four bits naming its family.
  Components, events, history, memories and saves hold only these ids; EnTT's own handles live within one step and are never stored.
  - Each registry maps id to handle in one list per family, each kept in id order by appending, so a lookup is a binary search and a walk in id order needs no sort; an ended entity leaves a gap, swept out once a family's gaps pass a quarter of its live entries.
  - The world's own owners of events, its layers and your commands, have ids below every entity's (family none).
  The dead leave their entity but keep their record in the history (`PRN-15`).
- **A thing's kind is its catalogue entry,** and its parts are the components the entry lists, as RimWorld's Defs and Comps; each entry becomes a ready recipe at load.
  There is no class hierarchy of kinds, which Dwarf Fortress regretted.
- **One descriptor per component:** its stable name, its version and its fields (name, type, unit, range, whether it names another entity or entry, what it affects, a plain description).
  The same descriptor loads its values from the catalogues, saves and loads it, hashes it for the checksum and shows it in the details view (`PRN-14`); since α1.4a the digest, the snapshot's writer and its reader all walk it.
  A new component is one header, one descriptor, its rules and one line in the component list.
- **Order:** EnTT's own order is not canonical (it shifts with unrelated changes and across a save), so nothing that decides may follow it.
  - Decisions follow the event queue (A3.3) or lists sorted by id, and every sort breaks ties by id.
  - Raw order is allowed only where the result cannot depend on it: per-entity updates, whole-number totals, minimum and maximum with ties broken by id.
  - Every pool is made at start, in name order, so a new world and a reopened one have the same pools in the same order.
  - EnTT's signals keep indexes up to date and never carry game rules.
  - An order fuzzer in the checks scrambles every pool before each batch, and the results must not move.

### A3.3 Time and events

- **The clock counts whole game seconds** in 64 bits, and the 60-day year, its seasons and dates come from it (`TIM-18`, `TIM-14`).
- **Work happens at events on one queue,** each with a unique key: (game second, owner's id, owner's sequence number).
  - The key decides the order, never the structure: equal times are settled by owner, then by sequence, the same everywhere (`TIM-17`).
  - The world's layers and your acts own reserved ids, so they come first within their second.
  - A handler may only schedule keys after its own, so an effect on someone else lands at least one game second later: a call travels, a reaction takes time.
  - Each activity ends at an event, and the doer decides again only then, or when interrupted (`TIM-17`); timers are events (`MAT-19`); each layer of the world is one event that reschedules itself, weather hourly, water daily, plant cover every five days, each running its cells as a batch (`WLD-12`).
- **The queue** is a binary heap of the keys (120–230 ns an event, 1–4% of a core at the speed targets); a two-tier one of minute buckets and a heap replaces it only if a profile shows the queue above about 5%, and the checksum test proves the switch changes nothing.
- **Cancelling is lazy:** the owner keeps the sequence it expects for each slot (its activity's end, each timer); interrupting clears it; a popped event whose sequence no longer matches is skipped.
  The heap is rebuilt without its dead entries when they pass a quarter of the live ones, which no outcome can see.
  A save holds only the live events, sorted.
- **Activities** have a start, an end and a way (where the doer is at any moment, from its start, end and pace), so they can be seen, met or attacked on the way (`TIM-17`).
  An interruption ends one early by the kind's own rule of what it keeps: a walker stands where they got to, what builds up gives its share, work stays in the thing, a single act does nothing.
- **The world runs every event before its goal,** and its frontier is then the goal: every event before the frontier is done, none at or after it, so a digest taken at a frontier, such as each midnight, is the same however the run was cut into batches.
  The world's rules live in systems, each handling the events of one family of entity or of one of the world's own owners.
- **Events happen one at a time in key order, and that one-thread run is the reference.** Every faster way must give the same bits:
  1. Game time is cut into windows on a fixed grid, also cut at the world's layer events and wherever the simulation stops; since cuts cannot change results, cutting is free.
  2. At each window's start, owners that could touch each other or the same thing within the window join one island: those within twice the longest reach plus twice the fastest pace times the window, or sharing a store, a household, a thing or a shared activity.
  3. Each island takes its events for the window from the queue and runs them in key order on one worker; events it makes inside the window stay in it, later ones go out to the queue.
  4. Islands run side by side on up to four workers, then merge their events and history by key.
  - Since no island can read what another writes within the window, the result equals the one-thread run for any window, thread count, speed or pause; a test proves it, and a debug build logs any touch across islands.
  - New ids for things made inside an island are handed out in key order at the merge, so they never depend on other islands; until a rule needs it, beings are made and ended only between windows, and the world stops the run if a rule tries otherwise.
  - Up to about camp speed, one worker runs events in order, with the same results.
  - *As built in α1.3b:* events run through a context, the whole world's or one island's, through which a rule reads the moment, schedules, cancels and records history; an island refuses any owner it does not hold, so a rule that touches another island stops the run.
    Each system gives a circle each of its owners with events cannot leave within the window, and names its owners without events whose way could come within reach of one; owners within reach join one island (a union-find, the active owners' circles met in cells of 250 m).
    The world's own owners run alone between windows, which are also cut at their events; an island's events and history merge by key.
    A test runs one world one event at a time and in islands of 60, 300 and 900 seconds on one to four threads, stopped at seconds chosen by keyed chance, with the order fuzzer on, and every digest matches; a planted rule that decides by a marker in another island fails it.
  - *Measured in α1.3b:* for the demonstration's crowd islands cost more than they save, since its events take about 1.3 µs each: 10,000 markers ran 3 game days (0.8 million events) in 1.05 s on one worker, and in 3.9 s in islands of one-minute windows on four threads (about 80 islands a window, the largest with 16% of the events), or 17.8 s with five-minute windows, where one island held 90% of the events.
    With about 8 µs more work an event, islands of one-minute windows on four threads were only about 10% faster than one worker.
    So the crowd runs on one worker, and islands wait for heavier events, such as minds (M6); before then, ways bounded as lines rather than circles, a cheaper join and workers that need no waking would cut their cost.
- **So a long process costs nothing until it ends,** and the cost of a game day follows what happens in it, not the speed.
- *Measured in pre-production* (P6): a thousand people at 6.0 game years a real minute on your phone's four cores, with decisions in 5-minute windows read from a snapshot; production replaces those windows with islands, which give the same history at any speed.

### A3.4 The same bits everywhere (`RES-05`, `TIM-16`)

Following Box2D and Factorio:
- **Arithmetic:** IEEE double precision for working values, with no fast-math and no contraction into fused multiply-adds (A2.2).
  The basic operations, the square root and the exact helpers (`floor`, `fmod`, `ldexp` and the like) are the same everywhere.
- **Maths functions:** CORE-MATH's correctly rounded double functions, vendored at a pinned commit and wrapped once in `sim/num`, which refuses any input outside a function's domain and any answer that is not a finite number.
  - Since angles are turns, sine, cosine, tangent and their inverses are those in half turns (`sinpi`, `asinpi` and the like), exact at the quarter turns; then exponents, logarithms, power, hyperbolic tangent, error function, cube root and hypotenuse.
  - A correctly rounded answer is unique, so every machine agrees: MPFR checks every function in the cloud on its edges, CORE-MATH's hard cases and 200,000 inputs across its domain, and no platform maths function is used.
  - Measured in the cloud (x86-64 without fused multiply-add): 10–30 ns a call, 82 for power and 127 for the direction of a vector; the phone's own times are in its self-check.
- **Numbers** (`sim/num`):
  - positions as 32-bit whole centimetres on the torus, 200,000,000 by 100,000,000, differences wrapped in 64-bit arithmetic, squared distances exact and square roots taken on whole numbers; exactly half way round, the way taken does not cross the edge where the map wraps, so a way back is always the exact reverse;
  - angles as turns in 2^32 steps, which wrap by themselves and add exactly, read by the trigonometric functions as half turns;
  - heights in millimetres;
  - time in 64-bit game seconds;
  - amounts as 64-bit whole base units (milligrams, millimetres, seconds, parts per million), rates applied in closed form at events;
  - probabilities as 64-bit thresholds read exactly from the catalogue's text or a ratio, a draw firing below its threshold, and certainty its own case;
  - one conversion from floating to whole numbers, rounding down, up, toward zero or to the nearest (halves away from zero), which refuses what is not finite or does not fit;
  - floats never in saved state, and never NaN.
- **Banned in `sim/`, each with its replacement,** checked on the code's syntax tree (clang-query) by `tools/rules.py`, which names the replacement in each message:
  - `long double` (double), also in `view/`;
  - `float` (double for working values, whole numbers in state);
  - platform maths and `fmin` and `fmax` (`kd::num`'s maths, `std::min` and `std::max`), also in `view/`;
  - casts from floating to whole numbers, but in `kd::num::to_int`;
  - parsing or printing floats (unit strings read into whole numbers, A3.6);
  - `<random>` (`kd::chance`);
  - `std::reduce`, the scans and the execution policies (pieces of a fixed size added in their order);
  - `std::hash` and the unordered containers (`std::map`, sorted vectors, `kd::chance::name` for a stable hash);
  - sorts and heaps that leave ties to the library (`kd::num::sort_strict`, which refuses ties, or `std::stable_sort`);
  - thread counts, clocks, the locale, character classes and addresses;
  - two calls with effects in one expression (a statement each);
  - raw memory hashed, compared or copied (fields one by one, `std::bit_cast` for a value's bits).

  Plain `char` is unsigned on every build (`-funsigned-char`, as on arm64, and as Linux has made it since 6.2), so its arithmetic cannot differ between chips; the same-bits check holds every compile command to it.
- **The floating-point environment:** each simulation thread sets the default one first, and checkpoints assert it (x86-64's MXCSR, arm64's FPCR), since a new thread inherits whatever its creator had.
- **Order:** every loop that decides anything runs in a defined order (A3.2); parallel work is cut into chunks whose size and borders depend only on the data, gathered, then applied in key or id order.
- **Checksums:** XXH3 over a canonical stream, each system in a fixed order, entities by id, fields little-endian; a digest per system and for the whole state at every checkpoint, so a difference narrows to a system and a day.
- **Proof** (A17): seeded worlds run on the five builds of A2.2, on one to four threads, with islands of several window lengths, stopped, saved, reopened and resumed, and every digest must match; libc++'s randomized tie order and the order fuzzer must not move them.
  The phone runs the same check in its self-check (A2.3).
- *Proved in pre-production on your phone* (P5): a seeded world ended each day with the same checksum on x86-64, on arm64 under qemu and on your phone, on one thread and four.

### A3.5 Chance

- Every draw is keyed by (world seed, system, being, moment, purpose, index) through a chain of the SplitMix64 finaliser, a counter-based generator in Squirrel Eiserloh's way, chosen in pre-production (P5).
  Each part of the key is spread over 64 bits as SplitMix64 spreads its counter, (part + 1) × its golden constant, joined to the chain and mixed; the first five parts are mixed once for a being's draws at a moment (`chance::Draws`), and each draw adds its index.
- Systems and purposes are named, and keyed by a stable 64-bit hash of their names, XXH3 through the canonical digest, so adding a new kind of draw never shifts the others (`TIM-16`).
- Draws are whole numbers: below a threshold for a chance, a 128-bit multiply for a whole number in a range (as even as 64 bits allow, off by at most the range over 2^64), the top 53 bits for a fraction in [0, 1).
- Any thread can draw any number in any order and get the same one; the generator's statistics are tested over structured keys (indexes, beings, moments and seeds counting up: frequencies, every bit, and neighbours' correlation and differing bits), and a few draws are pinned for ever, since changing them would change every world.

### A3.6 Catalogues and tuning (`MAT-13`, `MAT-14`, `MAT-17`)

- **Content lives in TOML 1.0 files in sources:** `data/base/` for the game, one entry a file, its kind from its folder and its name from its file name; tuning files hold every tunable number; `checks/` holds what only the checks read, such as each item's expected fits (`MAT-17`) and the orders of plausible values (`MAT-05`).
- **No floats:** whole numbers are TOML integers, and quantities and ratios are strings with units ("3.5 kg", "1 h 30 min", "15%", "1 in 100") read exactly into whole base units, so the phone's parse cannot differ from the cloud's.
  "m" is only a metre, never a minute.
  - toml++ lives behind one file (`kd/data/toml.cpp`), which turns a file into a small tree whose every value keeps its line and column; a float, a date or a time is refused where it is written, with what to write instead.
  - The measures and their base units (`kd/data/units.hpp`): mass in milligrams, length in millimetres, area in square millimetres, volume in millilitres, speed in millimetres a second, temperature in thousandths of a degree, ratios in parts per million, and time in seconds, read two ways: in life a month is 365.25 / 12 days and a year 365.25 days, in the game a season is 15 days and a year 60 (`TIM-18`).
  - Each unit is an exact fraction of its base unit, and a number has at most 18 digits and a point, never an exponent; a value finer than its base unit is refused, never rounded, and so is a ratio such as "1 in 3" that is no whole number of parts per million; a chance is read as an exact fraction into its threshold.
  - The proof suite `units` reads 200,000 written quantities and chances on every build and on the phone, and their digests must match.
- **Durations record both lengths,** `{ life = "3 month", game = "15 d" }`: the simulation reads the game length, and the `TIM-18` check holds it to the rule, "about" read as within 10% (`kd::time::check_rule`): up to 15.4 days in life the two are equal; from 27.4 days (a month, 365.25 / 12 days, less a tenth) the game length is within a tenth of 60 / 365.25 of the life length; between, anything from a tenth under that to the life length; a refusal names both lengths and the range allowed.
- **Schema once:** each kind has one `visit()` naming its fields with their types, units, ranges, links and what they affect (`rules`, `world` or `look`).
  The loader, the schema writer and the fingerprinter all walk it, so nothing describes a kind twice.
  The loader refuses unknown keys and floats, and names every error by file, line and column.
  - The walkers (`kd/data`): the loader, the link resolver (a link names an entry, and once all are loaded holds its canonical name and number), the fingerprinter, the schema writer (`kindling catalogue schema`) and the display (`kindling catalogue show`).
  - Field types: whole numbers, truth values, texts, a choice among named options, quantities in a measure, chances, durations with both lengths, and one link or a list of links to a kind.
  - A kind's files are `<source>/<folder>/<name>.toml`; a tuning file is a kind of one entry, `<source>/tuning/<name>.toml`, such as the speeds of the zoom stops.
  - Every kind is one line in `kd/data/kinds.cpp`, and the catalogue loads files handed to it as text, in path order whatever order they come in.
- **toml++,** pinned, behind one file and with no exceptions, reads the text.
- **No templates:** every entry is complete and reads alone, since a parent's values would be the child's inputs (`MAT-13`); a tool copies an entry as a starting point instead.
- **Names:** lower case, namespaced by source (`base:flint_nodule`, `base:` implied); numbered at load by sorted name, those numbers used only for arrays; chance and tie-breaks keyed by a stable hash of the name; saves holding each kind's names; renames listed in a file (`PRN-14`).
- **Fingerprints:** a digest per entry from its canonical values, and per source three: rules (all that affects outcomes), world (only what makes the land, with the sets of plant, animal and material kinds) and look; plus a world-making version raised by hand when code that makes worlds changes, guarded by a test of golden worlds.
  Each world keeps its sources, their versions and digests (`TIM-08`); a world digest that changed means a big update (`PLT-09`).
- **Checks:** the loader checks syntax, types, units, ranges, required fields, links, duplicates (a key written twice, a link or name listed twice, a file handed over twice) and `TIM-18` at every start, on the phone too; the heavy `MAT-17` checks run in the cloud on every change, written in `sim/` so the phone could run them on combinations the cloud never saw.
  - Each check on the whole catalogue (`kd/data/checks.hpp`) is registered beside its kind's line in `kd/data/kinds.cpp`, with its name, the items it serves and what it holds the catalogue to; `kindling catalogue check` runs them all once the catalogue loads, and each fault is named at its file, line and column, since every entry keeps where each of its fields was written.
  - `MAT-05`'s orders are `[[order]]` tables in a source's `checks/orders.toml`, each naming a kind, a field that can be ordered (a whole number, a quantity, a chance or a duration's game length), two entries or more from the least to the most, and why; an entry out of its place is refused at its place in the list.
- **Onto the phone:** the build (`tools/gamedata.py`) checks the catalogue and copies the sources into `game/data/` with `build.toml`, which lists each file with its SHA-256 and each source's version and digests; `view/` reads each listed file through Godot's `FileAccess` and hands the bytes to `sim/`, and the self-check compares the phone's files and digests with the build's.
  The screen reads an entry's values in base units through the world class (`KdWorld.entry`), as the Time page reads the zoom stops' speeds.
- **Sources for later:** the loader takes an ordered list of sources, each with its id, version and requirements, so later layers such as bronze, or content from elsewhere, come as more sources; for now a source may only add entries.

### A3.7 Saves (`TIM-05`, `TIM-08`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`)

- **Each world is a folder** under `user://worlds/<id>/`: `world.toml` (name, seed, sources and digests, dates, size, flags such as a test world's switches), the snapshots, the command journal, the history in yearly segments, later the book of ages and the kept areas.
- **The rule:** your commands are the only input that cannot be re-made, so they are written and synced at once; everything after the last snapshot is re-made exactly by re-simulating (`TIM-16`).
- **One I/O thread** owns every file and every sync, through a small interface tests can fake, and never stalls the simulation or the screen.
- **Snapshots:** a header, then chunks, each with a tag, a version, its lengths and a hash, compressed by zstd at level 1, hashed before compression; unknown optional chunks are skipped, unknown critical ones refused.
  Rows are written field by field, little-endian, through the component descriptors (A3.2).
- **Logs** (commands, history): records framed by length, type, sequence number and checksum; reading stops at the first bad record and cuts the file there.
- **Writing:** a new file, synced, renamed over the old, then the folder synced; old files are deleted only when the new state is safe.
- **When:** a snapshot every 30 real seconds while running, and when the app leaves the screen, when you switch worlds, export or quit; the simulation pauses only to copy its state at an event boundary, and compression and writing happen on other threads (`PLT-07`).
- **When the app leaves the screen** (Godot's `NOTIFICATION_APPLICATION_PAUSED`, on its main thread between frames): the simulation stops at the next event, a pause mark is synced to the journal, and the snapshot follows, well within the 10 seconds before Android freezes the app (`TIM-05`).
  Back never quits outright: it closes panels, and at the top the app saves and goes to the background.
- **After a crash:** the newest snapshot whose hashes hold is opened, a damaged one moved aside and never loaded; the journal's commands are re-applied at their moments while the world catches up under a short note; the re-made history must match what was written, or the mismatch is reported as a bug (`PLT-07`).
- **Old saves:** every chunk and record carries its version, with an upgrade for each step; whole-world migrations are named and recorded in the save, applied once; saves keep names, never catalogue numbers.
  A corpus of small exported worlds from each alpha is opened by every build, carried on after small updates and its history read after big ones (`PLT-09`).
  A new version keeps the previous version's last snapshot, and the files it needs, until the world has run an hour.
- **Export and import:** one `.kindling` file holding the world's files with a checksum for each, written and read through Android's file picker; an import is checked as it arrives and a damaged one refused with a message naming the damage (`PLT-08`).
- **Space:** free space is checked at every save; the game warns before the phone is full and asks which worlds to delete, and never deletes anything itself (`PLT-10`).
- **History** is appended as it happens, in yearly segments, never rewritten but by the fixed thinning rule at year boundaries (`PRN-15`, `PLT-10`).
- *Built in α1.4a:*
  - **Files** (`kd/save`): a folder's few primitives (write, append, cut, rename, sync a file, sync a folder) behind one interface, the disk's own and a fake that keeps what each sync made safe, so a test cuts the power between any two calls; writing a whole file, keeping a log and setting a file aside are written once on the primitives, so the fake tests the steps the disk takes.
  - **The keeper** (`save::Keeper`), with its own I/O thread: `world.toml`, the newest two snapshots, `journal.log` and `history/<year>.log`.
    A command is appended and synced before it acts; history is appended as each batch ends and synced before each snapshot, so none can be lost behind one; a snapshot's state is copied between two events on the simulation's thread, then compressed and written on the I/O thread.
  - **Snapshots:** the clock, the catalogue's names kind by kind, both registries entity by entity through the component descriptors (A3.2), the live events and each system's own state, each a chunk; the file ends with a hash of every byte before it, so damage anywhere is refused, even where zstd would not notice.
    An entry of the catalogue is written by its number and read back through the names, so a save keeps names, never numbers.
  - **Commands** are part of the world's state: the world's own owner of commands holds those pending, wakes at the earliest, and hands each to the system that takes it; the demonstration's is calling a camp home.
    The runner takes jobs between batches, so a command, a save or the save as the app leaves the screen meets the world at an event boundary, even while time is paused.
  - **Opening** (`demo::keep_crowd`): the newest snapshot whose every hash holds, newer damaged ones moved aside; the journal's later commands act again at their own seconds; the world catches up to its last pause mark, command or record of history, and each record it makes again is compared with the one written.
  - *Measured in the cloud:* the 10,000 markers' state copied in 3.4 ms (1.6 MB), compressed in 3.3 ms to 242 KB, and read back and opened in 5.3 ms.
    The kill test kills the tool at 100 random moments over a world of 2,500 markers with three commands, and the world it ends with is the unbroken run's, with no record made differently.
- *Built in α1.4b:*
  - **Worlds** (`KdWorlds` in `view/`): each world a folder under `user://worlds/`, its id the folder's name, and `world.toml` holding its name, seed and camps as TOML, written whole; a file `current` names the world the Crowd page opens, `crowd` until you choose one, where α1.4a kept its world.
    The Worlds page lists each world with its name, moment, when and by which version it was saved, and its size by part, and makes, opens, renames, exports, imports and deletes them, deleting only on a second tap.
  - **The .kindling file** (`save::ArchiveWriter` and `save::ArchiveReader`): a header, then `world.toml`, the newest snapshot, the journal and each year of the history, each part with its path, length and hash, and an end holding a hash of every byte before it.
    It is written and read a megabyte at a time, a few each frame, through Android's file picker, whose `content://` files Godot's `FileAccess` opens; an import is checked part by part as it arrives and refused at the first damage, with words naming it; a path that is no part of a world is refused before anything is written, and a refused import leaves nothing behind.
  - **Versions** (`save::Versions`, a chunk of each snapshot that the world skips): the app's version that saved it; the rules for making worlds as one digest, of the world-making version and each source's world digest; each source's version and rules digest; the migrations the world has had; each version it has run under and from which second; and the real seconds it has run under the last.
    A world opened by another version meets an update: a changed making digest is a big one, and the world is not run and nothing is written, its history still read; anything else is small, and the world carries on from its snapshot under the new rules, its history after the snapshot made again rather than compared, and the previous version's last snapshot and `world.toml` kept in `previous/` until the world has run an hour under the new one.
    α1.4a's snapshots, saved before versions were kept, are a small update.
  - **Upgrades:** each part of a world's snapshot carries its version, and an older one is brought up a step at a time (`world::upgrades()`); a component reads its older shapes through its own `upgrade()`; a migration (`world::migrations()`) is made once to each world saved before it, as it opens, its name kept in the save, and a new world counts them all as had.
    None is needed yet, and a test proves each path; a history record's shape is numbered by its frame's type.
  - **The corpus** (`sim/tests/corpus/`): a small world saved by each alpha's own tool and data, exported as a .kindling file and listed in `corpus.toml` with the update this build is for it; every build opens each, catches it up and carries it on a day with no record made differently, or after a big update reads its history.
    α1.4a's was made by building α1.4a's tool from its commit.
  - **Thinning:** each history record is framed as kept for ever or not, as its system's `keeps()` says when it is written (your commands are kept, greetings are not); each year's file is numbered from 1, and α1.4a's, numbered on from the year before, still read.
    Before the first record of a year is written, each year more than 25 years past is rewritten whole as a thinned year, a mark and its kept records, so a file for a year shows that the years 26 before it are thinned; a world made again from its seed passes thinned years by and leaves them as they are.
    Opening reads the history only from the snapshot's year on, so a long history costs nothing to open.
  - **A failed write** (M1's review): once any write to a world's folder fails, as when the phone is full or its storage breaks, nothing more is written, your command then is refused, and the world stops and says so; the folder keeps what was safe before the failure and opens there whole, so nothing is written past a gap and no snapshot hides history that was never made safe.
    An import makes each part safe as it ends, and the folder's new name with it.
    A snapshot's whole-file hash is checked before any part is unpacked, so damage can never make it ask for memory.
  - **Space:** the free space is checked at each save, and below the saves' tuning's `warn_below`, 1 GB, the Crowd page and the self-check warn and point to the Worlds page, where each world's size shows; nothing is deleted but by you.
  - *Measured in the cloud:* a world of 10,000 markers ten game days in, 5.4 MB, exported in 23 ms and imported, every part checked, in 9 ms; its history grows by about 31 MB a game year, 68 bytes a record as written, and zlib makes a year about a third of that.

### A3.8 Talking to Godot

- **Three classes for GDScript,** from `view/`: the world (make, open, save, close, export, import; commands; the goal and the frontier; counters and events), the crowd (which draws the walkers into MultiMesh buffers), and the device (cores, heat, telemetry).
  A few calls a frame, never one per walker: a call into the extension costs about 0.1–0.2 µs.
- **Commands in:** plain records, stamped with the game second they act at and written to the journal before they act; while you choose a power the game is paused, so a power acts on exactly the world shown.
  - *Known gap, from M1's review:* the demonstration's call home acts at the world's frontier, up to a quarter of a real second of the speed ahead of what you see, hours of game time at top speed. It is a test command, not a power; before the first power (M9), a tap pauses the world and acts at the moment shown (`WLD-13`).
- **Snapshots out:** after each batch, the simulation fills one slot of a triple buffer, so neither side ever waits and the screen always takes the newest: for each walker its id, kind and camp, and its ways from the screen's game time to the frontier.
  `view/` places each walker along the way it was on at the screen's game time and copies the result into MultiMesh buffers, one per area of the world, each with its own bounding box.
  - *Built in α1.3c:* the world is ahead of the screen by up to a quarter of a real second, so a walker may have begun a new way the screen has not reached; the world keeps each way a doer sets off on, as it runs, merged by key in islands (`world::Way`), and the snapshot holds each walker's ways back to the screen's time, so it is drawn exactly where the world has it at every whole second, and on a straight line between.
  - Between the whole seconds the screen interpolates the world's own place (`Activity::at`), so there is one rule for where a doer is.
- **Events worth showing** travel in a lossless queue, drained once a frame.
- No Godot object is touched from a simulation thread, and `view/` converts but never decides.

### A3.9 Threads, speed and budgets

- **The simulation runs on its own worker threads,** up to four, made with an explicit 8 MiB stack (bionic's default is 1), named, and at a slightly lower priority than Godot's main thread (`PLT-01`).
  The fastest core and the small ones stay for Godot, sound and the system; Godot's own worker pool is kept small.
  Whether the workers are pinned to the middle cores is decided by the benchmark, which runs both ways, since Android advises against pinning.
  - *Decided by your phone's benchmark (α1.5b):* pinned to the middle cores the world held 4.1 game days a second, unpinned 4.3, so the world runs unpinned; the comparison favours unpinned a little, since pinned runs after it on a warmer phone, but nothing in it argues for pinning.
- **The speed loop:** the simulation works toward a goal at most about a quarter of a real second ahead of the screen, and sleeps once it gets there.
  - The world's runner (`kd::run::Runner`) is one simulation thread that works toward the goal in batches the world chooses, publishes its frontier after each, and rereads the goal between them, so a lower goal stops it within one batch; a cut between batches never changes the result.
  - Each frame the screen's game time moves by the speed asked times the frame's real time, read from the steady clock, since Godot's delta is smoothed, but never past the simulation's frontier; when it reaches the frontier, time slows (`PRN-11`).
  - The goal is the screen's time plus the speed times a quarter of a second, and at least one game second, rounded up to a whole second (`kd::view::Pace`, which touches no Godot, so its tests run alone).
  - The speed shown is measured from what was drawn over the last real second, so it is always the real speed (`TIM-01`).
  - Pausing asks the world to go no further than its frontier, and the screen glides to it at whatever rate arrives a quarter of a second after the pause, however far ahead the world had got after a drop in speed; then it stops.
  - At one game second a real second, a game minute takes a real minute (`TIM-10`).
  - *Built in α1.3c:* the crowd's batches take about 12 ms of real time at most, in steps of at most a game hour, so at top speed the frontier moves on 60 times a second; the screen asks for at most 90% of what the phone can do, measured from the batches, times the heat's working share, so it glides behind the world instead of catching it.
    In the cloud, top speed holds about 2.3 game days a real second for 10,000 markers, some 600,000 events a second, every frame on time.
- **Heat:** `view/` reads the phone's heat headroom every 2 s with a 10-s forecast (Android forecasts only while asked at least every 10 s), and listens for its thermal status; as the forecast nears the first throttling level, the simulation's working share is cut quickly and given back slowly, so time slows before the phone throttles.
  - *To build in M2:* Android's headroom of 1.0 is *severe* throttling, not the first level, so the guard reads the phone's own *light* and *moderate* thresholds (API 35) and its headroom listener (API 36), and acts when the 10-s forecast reaches the light threshold less a margin (0.05 to start); a missing reading is no reading, never a cool phone.
  - Slowing time cools the phone at far zooms at top speed, where the simulation is the load; at close zooms the picture is the load, so the graphics budget must pass the 20-minute heat run itself (A18.1), with one planned, logged step under heat as its last resort (A5.5).
  - *Built in α2.1a:* the thresholds are read once and kept, since the array Android returns is the manager's own before Android 16 and the caller's after; the guard acts at the light threshold less `margin` (5%, in `base/tuning/heat.toml`), and on a phone without thresholds at `near`.
- **Telemetry:** the device class also reads battery and power rails, the cores' clocks, our threads' CPU time and memory, and the interval of every frame; trace sections mark each frame and batch for the phone's own System Tracing.
  - *Built in α1.5b:* the extension times every frame itself, once a frame after every node's process, from the steady clock (`kd::view::FrameMeter`): a frame is on time within its period plus half a refresh, a stall counts every period it skipped, and a gap over 66.7 ms is more than 50 ms late.
    The battery's charge, current and charging come from Android's BatteryManager through Godot's AndroidRuntime, and its current gives the phone's whole power; the power rails, which need Android 15's power monitor service, are left for a later benchmark.
    Our threads' processor time comes from `/proc` by their `kd-` names, memory from the process's status, and the cores' clocks from `cpufreq`.
    Trace sections mark each batch (`kd batch`), the world's own work each frame (`kd frame`), the crowd's drawing (`kd draw`) and each benchmark scenario.
  - *Built in α2.1a:* the graphics chip's time for every viewport drawn, Godot's draws, triangles and video memory, the chip's headroom (Android 16), and power as the battery's current times the kernel's voltage where the app may read it, else 3.85 V; the benchmark code's layout 3 carries them.
    Godot's release build times only a whole viewport, not each pass, so each part's cost comes from switching it off in turn, as the calibration scenes do (A18.1).
    The power rails stay unread: Android gives them only to Java callbacks, which the app does not have yet.
    The heat headroom is asked twice in a row, now and in 10 s; Android may refuse calls faster than once a second, but on your phone both answered in every reading.
- **Watch the known killers from the first benchmark:** pathfinding at scale, temperature fields and lines of sight.

## A4. Drawing

The look you chose on 6 October 2026: a sharp 3D world at the phone's full resolution wearing pixel-art textures (`PRE-01`, `PRE-02`).
Pre-production's low-resolution picture, pixel lock, outline pass and light in steps are gone.
Every cost marked *estimate* waits for M2's calibration scenes on your phone (A18.1), which replace it.

### A4.1 The picture

1. **Full resolution, straight to the screen.**
   - The 3D world draws on the Mobile renderer over Vulkan straight into the window, 1080 × 2404 in portrait, with 2× MSAA for smooth edges, which the chip's maker calls "virtually free"; the interface draws over it.
   - No low-resolution picture, no pixel lock and no outline pass.
   - Never FXAA, SMAA or TAA, which blur texture pixels, and never Godot's glow, auto exposure or depth of field: each also ends the merged pass.
2. **The merged pass is kept:** nothing in the main picture reads the screen or its depth, so Godot draws the whole picture without it leaving the chip; the shore line comes from height (A4.5) and smoke is drawn in the pass.
3. **Drawing order:** solid things first; cut-out plants after them, by a higher render priority, so the chip skips their hidden parts behind solid things; the few blended things last (smoke, mist, glints), as Imagination advises.
4. **Smooth things in maps at their own sizes,** never searched for at every screen pixel: the sun's shadow map, our height-field sun map, the fire maps, the water's mirror at half size and the view's maps (A4.6).
   A map costs the same whatever the screen, where a search at every pixel costs four times what it did at 2 × 2.
5. **The levers,** used only where a scene measures over its line, in the order they show least:
   1. every saving that does not show (A4.3, A4.4, A4.6), always on;
   2. our own build of Godot 4.7.2 (A2.2), only if the calibration scenes call for it, as you allowed on 6 October 2026: a depth pre-pass for leaves with an equal depth test, which changes nothing on screen; buffers that never reach memory, which Godot lists as a TODO; and shading once per 2 × 2 pixels on ground and plants, which your phone's driver offers for each draw and which may show faintly up close;
   3. the 3D drawn at 0.75 scale in the costliest scenes only, which looks softer, enlarged by FSR 1 or bilinear and switched only when the fingers lift; if the switch stutters, two prepared pictures are kept (about 15 MB more);
   4. half resolution, last.
   - Each lever that may show has an on-off switch on the phone and stays only if it passes your blind test (`PRE-01`, A5.5).
   - The world's density is never cut.
   - *Estimated:* the liked camp drawn plainly needs about 13–45 ms of the graphics chip a frame; with every saving that does not show about 5–23 ms, about 10 at the closest zoom; adding the leaf pre-pass and the 2 × 2 shading, about 6–8; against a line of 8 ms.
6. **Portrait and landscape** (`PLT-02`): the texture pixel's size is set from the screen's short side, so it is the same both ways; turning the phone rebuilds the 3D buffers once, and the rig keeps its focus, turn and metres per screen pixel.

### A4.2 Steady texture pixels (`PRE-22`)

- **One sampling function** reads every texture (`CLAUDE.md`, rule 4): the "smooth pixel" filter, crisp inside each texture pixel and blended over one screen pixel at its edge.
  - It reads with the chip's own blending, at a coordinate moved so that the blend happens only at a texture pixel's edge, from textures stored with blending and mipmaps on, at the level it picks from the coordinates' true slope (`textureLod`): once, or twice within the short blend between levels or between a big surface's tiles (A5.3).
    *Built in α2.1a:* the chip's own choice of level (`textureGrad`) blends two levels over a whole level's span and reads the second without moving its coordinate, so the function picks the level itself.
    The rig names the band at the focus by the same rule, to know which levels to keep, and publishes the least size (`kd_texel_least`), so the rule's number is written once.
  - *Computed:* it flickers on 0.4–1.1% of pixels as the camera moves, against 17–28% for nearest-pixel reading, and keeps 94–98% of its crispness.
- **The texel ladder** (A5.3): each band's level of a texture is drawn as pixel art, each texture pixel covering exactly 2 × 2 of the level below, and loaded as the texture's own mipmaps (`Image.create_from_data`), never Godot's averaged ones.
  - The level comes from the texture pixel's area on screen, kept between about 1.4 and 2.8 screen pixels, with a short blend to the next, each read the same crisp way.
  - *Computed:* without a level made for the zoom every filter flickers on 10–43% of pixels; with one, the filter flickers on 0.1%.
- **Moving patterns** (the river's flow lines, foam, ripples, flames, smoke cards) step in whole texture pixels about 10 times a second, as poses do (`PRE-44`), if your eye agrees at the first review.
- **Thin things** (blades, twigs, poles, shafts) are at least one texture pixel wide, and below about one screen pixel they switch to a coarser drawn form.
- **Glints** on water, wet stone and snow are whole texture pixels that live a few frames, placed by a hash of their place in the world, so they sparkle but never shimmer.
- Texture pixels ride on their surfaces, so swaying grass and walking people carry them without swimming; the sun's shadow holds still in pans and turns, since Godot fits it to a sphere and snaps it to whole shadow texels.
- At the camera's tilt of 35–40°, a ground texture pixel shows about 0.6 as tall as wide, as on any real 3D view, where the pictures painted it square; the level is chosen by its area, and your eye judges the flattened look against the targets at first light.

### A4.3 Light (`PRE-30`)

- **One shared light function,** written once (rule 4).
  Each material hands it its texture pixel's colour, its facing, its openness to the sky, its crease darkening, its wetness and its snow, and it adds:
  - **the sun,** its colour and strength keyed by its height through the air, as Vibrant Visuals keys them; at noon the true white-warm light you chose;
  - **the sky's fill,** coloured by the sky and scaled by openness: shade near neutral by day, a little blue at true midday, clearly blue at night, as measured on your chosen pictures;
  - **light bounced from the ground,** tinted by the sunlit cover below;
  - **backlight:** leaves and grass glowing with the sun behind them, rims on edges a low sun catches, glints on water, which keep the feeling when facing the sun;
  - **fire** from the light grid (A4.5), its colour and flicker by its heat, and its **glow** as light scattered in the air near each fire, with no glow pass;
  - **the moon** as the same directional light at night, silver-blue, its strength by its phase (`WLD-07`);
  - **haze and mist** by distance, warmer toward the sun, with banks over water and hollows drifting on the wind;
  - **snow** as a cover on faces turned up, by slope and shelter, trodden where the world's traces say.
- **Textures carry colour, grain and crease darkening, never sunlight from one side,** since our sun goes round and the camera turns.
- **The moments' targets,** measured on your chosen pictures, are the target card's starting bands (A5.5):

| Moment | Mean lightness | Share dark | Lights | Shade |
|---|---|---|---|---|
| Late afternoon | 0.55 | 32% | golden | neutral, slightly warm |
| True midday | 0.61 | 22% | warm | slightly blue |
| Dusk, relit | 0.42 | 66% | warm and red | neutral |
| Night, your three | 0.32–0.36 | 85–87% | a few warm pools | blue |
| Winter | 0.66 | 21% | pale gold | blue |
| Rain | 0.47 | 46% | grey, the gold gone | neutral |

  A small hearth's light over moonlit ground halves within about 2 m and falls to a tenth by about 3.25–4.25 m; sunlit snow is cream and shaded snow blue.
- **Colour:**
  - a tone curve with a soft shoulder (Godot's AgX or Filmic), so snow, fire and glints keep their texture;
  - a gentle colour table for each moment and biome, applied in the final step inside the merged pass, blended as the sun moves and as Vibrant Visuals blends biomes: a finishing touch, never the look;
  - grades are small text settings, a set for each season, with a true midday.
- **Debanding on,** in the materials and the final step: the phone's 10-bit picture steps 3–7 screen levels at a time in the darkest tones, which smooth night light would show as bands; 16-bit colour (Godot's HDR 2D) only if bands remain.
- **At speed** (`PRE-30`, `PRE-29`): once a day passes in under about 10 seconds, the light holds steady from high up and only its tint follows the hour; the map look is always lit so.
- *Built in α2.2a, its first version* (`game/look/light.gdshaderinc`), for calibration scene C1's field: a material's `fragment()` hands it its colour, point, facing and crease darkening, and gets back Godot's albedo, its openness as ambient occlusion (which scales the sky's fill, Godot's ambient light), the bounce and the fires as emission, and the haze as fog; its `light()` adds the sun through Godot's light with the small casters' soft shadow and the big casters' from the view's maps.
  - The view's maps are one picture of four channels, one read: openness, contact, the steepest angle toward the sun and how far away it is; the light grid is a 64 × 64 picture of up to four fires a square and a table of a row a fire.
  - Godot's stock material, which no family uses, stays dark on the cloud's software driver once the sun's shadow has been drawn, even after it is switched off, and a Godot forum report describes a floor darkened by the sun's shadow on an iPad; every family therefore lights through this function's own `light()`, which stays right, and C3's rocks and copies through its sun (`game/look/solid.gdshader`).
  - Its numbers are stand-ins, and the ground's shader takes it with its colours tuned against the targets (α2.3b).

### A4.4 Shadows and darkening (`PRE-21`, `PRE-24`, `PRE-30`)

- **Small casters** (people, animals, held and made things, stones, bushes and reeds) cast through Godot's sun map of 2,048 texels, its caster mask set to their layers only, softened by Mobile's soft filter so the edge is about one texture pixel at the closest zoom.
  - *Set in α2.2a:* the phone's soft filter is Godot's soft low, as the cloud's, since Mobile's own on a phone is hard; the calibration scenes draw one map over the whole view (orthogonal), as the game will.
- **Big casters** (the ground, cliffs, trees, tents and huts) cast through our own **height-field sun map:** for each texel of the ground in view, the steepest angle of anything between it and the sun, from the heights of tops and undersides.
  - The shadow fades over a band that widens with the distance to what casts it, so long shadows go soft and shadows at the foot stay sharp (`PRE-30`), which Mobile cannot do itself.
  - It is a small two-dimensional pass, never a compute program, remade only when the sun moves a step and read once a pixel; walls and crowns above the ground read a stored shadow height for their texel.
- **Grass and small plants cast no sun shadow;** they darken at their foot.
- **Cloud shadows** come from the weather's clouds moving over the ground, with no pass.
- *Estimated:* a 2,048 map with grass and small plants kept out of it, and softness that needs no wide search, save about 2–8 ms against pre-production's 4,096 map with every leaf in it.
- **Darkening in corners and under things,** which you kept, with no screen-space pass, which Mobile lacks and which would cost about 2–5 ms at full resolution and end the merged pass:
  - baked into each part's corners, creases, insides and undersides in Blender, and where parts meet when they are put together (A6.1);
  - painted into textures as crease and cavity darkening;
  - an **openness map** seen from above round the focus (about 1024 texels square, about 6 cm a texel at the closest zoom), made from the ground's heights and the tops of what stands there, the same heights the fires use: still things when an area loads or changes, moving ones at their pose steps;
  - a **contact map:** each thing's footprint darkens the ground under it, stones and baskets once, people and animals as soft ellipses at each pose step;
  - for each area, when it is made: its openness to the sky and the darkness of its hollows, so light enters under overhangs and stays out of caves (`PRE-24`);
  - plants darker toward their base, and the ground under dense cover darker by its density.
  - A screen-space pass comes only if the phone shows gaps, then at half resolution with a hierarchical depth buffer, as Imagination advises.

### A4.5 Fire, water and weather

- **Fire** (`MAT-18`):
  - flames are pixel art made by code at about 10 frames a second, the core, the flames and the embers each at its own rate, sized by the heat; embers are copies;
  - **the light grid:** every fire the stage keeps (place, heat, flicker seed) is listed in a grid of about 4 m squares over the view, up to four fires a square, as a 64 × 64 texture and a fire table read by every family and rebuilt when a fire or the view changes; Godot's own lights reach at most 8 a mesh on Mobile, and a MultiMesh is one mesh;
  - **fire shadows** from a small map for each near fire (about 128 texels square), made 10–20 times a second as people move, from the two height maps of tops and undersides that pre-production drew round its fires (P3), instead of a walk of up to 32 steps at every pixel, which costs about 2.5–5 ms a fire at full resolution (estimate);
    none where the sun outshines the fire, and none from the camp zoom out; a hut's or tent's walls stand in the height maps, so a fire inside lights only its doorway (`PRE-24`);
    calibration scene C5 times the map against the walk at half resolution, and your eye compares their looks;
    *built in α2.2b,* the walk is written once (`game/look/fire.gdshaderinc`) and used three ways: at every pixel; in a picture of its own at half resolution, of a flat ground on a layer only its camera sees, read by screen position for the first three fires a pixel; and in the maps, a two-dimensional pass of eight 128-texel maps side by side, each over 8 m, remade 20 times a second; the fire table holds where a fire's light comes from, its flame's middle 0.35 m up, since a fire at ground level lights the ground at no angle at all;
    in the cloud, three fires' shadows darken 1.6% of the picture by the walk, 1.3% at half resolution and 0.95% from the maps, whose shadows are softer, and the maps darken nothing the walk leaves lit;
  - **smoke** as soft, lit cards cut close to their shape and drawn last; a wildfire's smoke as a volume marched at half resolution in its own picture, read as a texture by cards in the main pass.
  - *Proved in pre-production* (P2, P3), at its low resolution: a camp lit by three fires at night held 60 frames a second, and every person and thing cast a shadow from each fire.
- **Water** (`PRE-26`):
  - the bed is drawn by the ground's own shader below the water's level, tinted toward teal and dark by depth, its coordinates gently wobbled: clear water with no screen read;
  - the surface draws the sky's colour by angle, and the reflection of what stands above it from a mirrored pass at half resolution with a smaller set of things (no grass), every shader discarding what lies below the water in that pass, since Godot 4.7 has no clipped camera projection;
  - ripples, flow lines along the current and foam at fords step in whole texture pixels; glints are single texture pixels; rain makes rings;
  - the thin bright line where water meets land or anything standing in it is worked out from height, never from depth;
  - mist lies on water at dawn, from the haze.
  - *Measured in pre-production* (P1): the mirrored pass cost at most 0.3 ms at 4 × 4; at full resolution it is calibrated again.
- **Rain's four layers**, from the world's weather (`WLD-16`):
  - streaks as thin copies one texture pixel wide in a volume round the camera, slanting with the wind, moved by the chip from a seed and the time;
  - splashes and wet shine in the materials' own shaders, surfaces darkening and gaining highlights with wetness, puddles in hollows reflecting the sky;
  - ripples as rings in the water;
  - drifting mist from the haze with moving noise, or a few soft cards.
- **Snow falling** as flakes of one or two texture pixels, as copies.
- **Lightning:** the flash is a change of the light for two or three frames, cool and bright with sharp shadows; the bolt is a bright mesh.
- **Sea foam:** a band along the shore from the distance to it, stepping in whole texture pixels.
- **Particles are copies moved by the chip** in the vertex shader. Godot's GPU particles, a compute program, drew on your phone in α2.1a's probe, so they stay an option where copies cost more, measured first.
- A debug view of each fire's reach checks that no light leaks through a hut's or tent's wall.

### A4.6 Families, materials and the view's maps

- **Round 1's layout holds.**
  - C++ families in `view/` draw through Godot's RenderingServer: ground, cover, plants, things, figures, creatures, markers, water, sky and effects.
  - A change feed fills a copy the view keeps (the stage), so still copies upload once and change only at events.
  - One tree of ground (A8.1) and one camera rig (A8.4).
  - Each family lists its forms by size on screen (full, simple, small, texture or marker), and for each form the passes it joins, so small cover draws in the main pass only.
  - Uploads go by changed byte ranges within a budget a frame (about 256 KB, an estimate), so an area's copies arrive over several frames.
- **Four shared parts,** each written once:
  - **materials:** texture arrays by size class (ground covers, made things, small things, garments and faces), one layer a material with its designed levels, since an array needs one size, format and level count; the material table; each copy's look data; the one sampling function;
  - **the poser** (A6.3);
  - **lights:** the fire list, the light grid, the fires' height maps, the sun and moon, the cloud shadows;
  - **the view's maps,** small pictures seen from above, built from the stage and read by every family: the patches' cover, the light grid, openness, contact, a push map of where people and animals stand, and the weather's wetness and snow.
- **Materials are data:** about a dozen family shaders, each compiled for its passes with a `#define` as pre-production did, about 40–60 pipelines; a new material is a layer and a row, never a shader (`PRN-14`); wear, wetness, snow, soot and burn are values for each copy or map over the layer.
- **Each copy's look data,** 20 numbers: its place (12), then its layer, tint row, wear and seed, then its season, growth stage, style pattern and a spare (`PRE-42`, `PRE-43`).
- **The density rule:** a form is drawn as copies only while each covers about 12 screen pixels or more (a tuning value); smaller, the ground's band texture carries it, and copies thin out by hash over the same marks, so nothing pops.
- **Forms switch by size on screen,** never by Godot's visibility ranges, which fade by transparency: each switch sits where both forms give about the same pixels, with a margin so forms never flicker.
  - *Estimated:* about 450–900 copies at the closest zoom at the liked density, and at most about 5,000 small ones at any zoom.
- **Who places what** (`WLD-13`, `PRN-10`):
  - the area maker places what rules read (single trees, bushes, stones and the plants people gather), as copies with data;
  - grass tufts, flower clumps, pebbles, fallen leaves and seedlings are set out by the chip from each patch's kinds, density, season and ripe yield (`WLD-31`) and a hash of the place, so every tuft is the patch's own and the same on every visit, and no rule ever reads a tuft;
  - each area brings a patch picture (4 m patches, 64 × 64 texels, about 32 KB) that the ground shader and the cover shader both read, so ground and tufts agree;
  - one MultiMesh for each cover form spans the view: each frame the processor writes a small table of the visible cells, and the shader places each copy from its number, sways it in the wind and drops empty slots, in about 10 draws with no uploads a copy;
    if the chip's vertex stage proves slow, copies with data for each cell, built on worker threads, follow the same rule.
- **Airy plants as you liked them**, saved only in ways that do not show:
  - each plant's shape is cut close to its leaves from its texture's see-through mask (Imagination: from 22% of the work wasted to 3% for a round sprite);
  - solid middles, with cut-outs only at the finest tips;
  - cut-outs drawn after solid things;
  - no sun shadow from small plants;
  - in stock Godot no depth pre-pass, which gains nothing for solid things on this chip.
  - Smoothing cut-out edges by alpha to coverage moves them to Godot's blended pass, as its source shows, so calibration scene C2 times it rather than assuming it free.
  - *Built in α2.2b,* for C2: a card cut close is the polygon where 16 lines round its design touch its leaves, a texture pixel out; a solid core keeps two texture pixels inside a design's dense middle, ringed by its cut-out fringe; a design's own levels keep a leaf's pixel where two of four are leaves, so the leaves keep their cover.
    Godot's render priority is the first thing its opaque pass sorts by, so cut-outs drawn after solid things, at a higher priority, works as A4.1 says.
    In the cloud, close-cut cards and solid cores draw plain cards' picture (4 of 103,680 pixels differ, where a card's foot meets the ground), while alpha to coverage changes 13% of them.
- **Buckets** are sized for each form: about 16–32 m for full forms near the camera, a whole area for far ones, still and moving copies apart.
  If draws prove the bottleneck, each frame's visible copies of a form are gathered into one buffer, as Saber's engine does.

### A4.7 Rules for the phone

- **Your phone, by its self-check** (α0.1a, 4 October 2026):
  - Android 17 (API level 37, build 16238327);
  - the screen at 1080 × 2404 pixels, 390 dpi and 120 Hz, four fifths of the panel's 1344 × 2992 each way, as its Screen resolution setting allows;
  - the graphics chip a PowerVR C-Series CXTP-48-1536 MC1, driver 1.662.3024 (6908880 as Vulkan reports it), Vulkan 1.4.317.
- Vulkan only, with no compute program that samples images: another engine stopped before its first frame on this phone and driver for one (Bevy issue 25788).
  Godot's skinning is a compute program that reads buffers only, and is tried on the phone before anything relies on it (A6.3).
- 2× MSAA; no FXAA, SMAA, TAA, glow, auto exposure or depth of field; nothing in the main pass reads the screen or depth; debanding on.
- Pipelines precompiled at load, which Godot backs with ubershaders, and a warm-up scene that draws every material kind in every pass with the real texture arrays bound and the colour table on, so nothing compiles while drawing.
- The screen runs at 60 Hz, set through the Android plug-in, since Godot's frame cap alone leaves it at 120.
- **Probes before use:** M2's first phone build tries each feature the look needs, each noted before it runs, so a crash names it: MSAA at full resolution, `textureGrad`, texture arrays with our own levels, alpha to coverage, a shading rate for each draw, and bone reads in a MultiMesh shader.
  The self-check lists the driver's shading rates (a public report of your phone offers a rate for each draw but none by texture), Android's GPU headroom and the power rails, where they are offered.
  - *Built in α2.1a:* each probe draws a little in a viewport of its own at the window's size for four frames, once a build, its state kept in the app's files; one still marked running at the next start closed the app, and is named and not tried again. The shading rates are asked through a Vulkan instance of the app's own. A Godot GPU particles node is probed last, since it is expected to fail.
- **Your phone, by α2.1a's self-check** (6 October 2026): heat thresholds light 0.80, moderate 0.93 and severe 1.00; the headroom listener works; shading rates for each draw only, not by triangle or picture, at 1 × 1 to 4 × 4; no GPU headroom offered; every probe passed, Godot's GPU particles included; the same bits as the cloud in all seven suites.
- **The cloud's software Vulkan driver** crashed drawing P2's smoke from some angles near 15° (α0.2c's fixes), so the cloud's pictures avoid that angle.
- **Godot's rules met in P1:**
  - front faces wind clockwise, the opposite of three.js, so imported triangles are reversed;
  - a pass never declares the picture it draws into, since Vulkan refuses a texture that is both its target and its input; a global sampler counts as declared wherever its file is included, so each is declared only where it is read (met again in α2.2b);
  - varyings are written only in their stage's own function, and the light function has no vertex position, so depth reaches it in a varying;
  - a shadow bias of 0.08 and a normal bias of 1.6 on a 4,096 map kept dusk's low sun free of stripes; the 2,048 map is tuned again.

### A4.8 Tests

- **Golden pictures,** drawn by one Godot run on the cloud's software Vulkan driver with one thread and time frozen: exact for a change of code alone on the pinned driver, and within a perceptual tolerance (NVIDIA's FLIP) for a change of look, whose before-and-after pairs you see; the set you approved changes only with your OK.
  The fixed views so far are the Look page's stand-in meadow up close, turned 30°, four times farther out, and the test board; they redraw identically run after run.
  A changed view shows FLIP's mean and its share of pixels above 0.2, and goes beside its golden picture on a lettered grid, so a fault is "E10".
- They guard our code only: the phone's chip may round differently, so the look is judged on the phone (`PRE-31`).
  At first start the phone draws a few golden views and reports FLIP's share of pixels above 0.2 against the cloud's; a jump is a driver problem to chase.
- **The look's checks** (A5.5), each with its line stated before its first run (`RES-09`):
  - A statistic becomes a check only after it agrees with your verdicts: four plausible speckle measures did not.
  - still frames are identical;
  - **shimmer:** on scripted pans, turns and pinches, each frame's error against a many-sample picture of the same view is compared frame to frame after following the camera's known motion; at most 2 in 100 pixels may change by more than 0.03 (`PRE-22`), and never a tenth more than the last build without a note;
    the error is each pixel's OKLab lightness less the many-sample picture's; the last frame's is moved through the motion, a projective map exact for flat ground, to its nearest pixel, and pixels newly in view are left out;
    on the stand-in ground, the meadow read smooth-pixel shimmers on none of its pixels a frame and the test board's crisp lines on 2.0%, at the line; read nearest-pixel, 2.7% and 25%; the scripted pan, turn and pinch over the meadow, 0.0%, 0.4% and 0.0%;
  - **the texture pixel's size** at every zoom stop: 1.5 to 3 screen pixels (`PRE-01`, `PRE-22`);
  - **banding at night:** the distinct screen levels across a moonlit slope, and its widest band of one colour along a row;
  - **ground accents at every band** (the colour difference of the most striking 1% of ground texture pixels from their surroundings): at least about 20, where accepted grounds score 23–30 and the speckled one 11–12, and each band at least 90% of band 0's;
    measured on the finished frame's ground, where materials sit together: a quiet material alone scores far less, as the trodden floor's 4–5 does, as asked.
  - **people against their surroundings** in busy scenes, from the engine's object picture: the median person at about the 80th percentile of the frame or above, none below about the 70th, starting lines refitted on your verdicts (`PRE-28`);
    each point's colour blurred over 1.5 pixels against over 12, as an OKLab distance; a person's mean over their pixels, ranked among all the frame's points;
  - **savings invisible:** under a tenth of the difference you saw at half resolution (5.3% of pixels above FLIP 0.2 at 30 cm), then your blind test (A5.5).
- **Test hooks in the engine,** used by the drawing run through cameras of its own, so the page's controls never show:
  - the colour picture as the game draws it;
  - a material picture and an object picture, each surface's number as a colour under plain light, 8 levels a channel, which come back exactly;
  - many-sample pictures of small patches, drawn 8 times larger with each texture level read as at the frame's size, then shrunk in linear light;
  - the camera's motion, from where four of a patch's pixels were in the frame before;
  - scripted camera paths in steps of 1/30 s, with time otherwise frozen;
  - a nearest-pixel read, for the shimmer check only.
- The statistics both the cloud and the phone compute are written once, in C++ (rule 4), and give the same bits on both: the self-check's look suite runs them on pictures made by chance, with the same digest on the cloud's builds and on arm64.
  FLIP is NVIDIA's own C++, vendored at a pinned commit, seen at about 80 pixels a degree (your phone at 30 cm); it works in floats with the platform's maths, so it stays out of the digests the two must share, and its mean on a known pair matches FLIP's own tool.

## A5. The look

The feeling of the pictures you liked is the goal; the guidance holds starting numbers to calibrate against your verdicts, never rules for their own sake.

### A5.1 The pictures

- **The targets** (`art/targets/`): only the pictures you chose: the liked camp from above and at sunset, the look itself (`d2-pixel-paint`), the close camp and its hours (noon, night, the painted-over night), winter, the far views, backlight, rain, mist, a relit dusk, a camp of thirty, the first cave, the truthful village, the storm, finding people by light, and the guides for people, animals, poses, trees, made things, materials, small plants and the close camp's meadow.
  They show the feeling and are never shipped or traced: the engine is judged against their light, colour, density and composition, never their flagged things (sawn wood, metal tools, tipi-like cones, borrowed dress).

### A5.2 Guidance about the feeling

It replaces the art bible's rules.
1. **The feeling first.** When a guideline and your picture disagree, the picture wins, unless truth (`PRE-42`) or a principle says otherwise.
2. **Light true to the hour.** True midday at noon, gold only when the sun is low; shade takes the sky's colour, lights are warm, and hollows, corners and contacts darken.
   The default view keeps the sun behind the camera's left shoulder.
3. **Real darks.** In daylight about a quarter of the picture is dark (the liked camp 27%); nights about 85% dark, with firelight in a few small pools and warm colour on less than about a fifth of the picture; winter soft, with very dark under about 5%.
4. **Strong colour as accents,** except where the season itself is the colour: in summer daylight on no more than about an eighth of the picture (the liked camp 11%), carried by flowers, fire, dyes, ochre and beads; in autumn the wood's colour is the field.
   Strong colour means chroma above 0.12; GPT's daylight pictures drift to 13–18%, so your pictures stay the reference.
5. **Greens muted and warm,** never one green over most of the frame.
6. **A colour plan for each biome, hour and season,** from the true colours of its species by season (`WLD-31`).
7. **Quiet pixel texture:** marks of 2 to 3 texture pixels, low contrast within a material, never single-pixel speckle, each zoom band drawn as its own pixel art.
8. **Density with a stage:** nature dense, airy and wild, and people on the quiet ground the world has made: trodden paths, working floors, snow, cave floors.
9. **People read first,** found by light from real sources and by their movement only: nothing in the world changes to make them stand out, and what they wear follows from their materials and their people's style (`CUL-12`).
10. **Edges by light:** no drawn outlines up close; a bright edge where sun, sky or fire grazes a shape; contact darkening; small far figures alone are outlined.
11. **Life in motion:** wind, water, smoke, rain in its four layers, people and animals; nothing moves that the world does not move.
12. **Truth before beauty in content:** shapes, materials and dress from archaeology (`PRE-42`), each people's style generated (`CUL-12`), no real culture's motifs (`SCP-20`), every picture checked (A5.6).
- The smallest thing worth making is about a fist-sized stone; anything smaller is texture.

### A5.3 The texel ladder

- A texture pixel shows as about 2 × 2 screen pixels at every zoom: 64 texture pixels a metre at band 0, the closest zoom (about 8 m across), then 32, 16, 8, 4, 2 and 1 a metre, one band a halving.
  - *Computed:* at 12 m across the view moves from 64 to 32 a metre, at 24 m to 16, at 50 m to 8, at 100 m to 4, and at the camp zoom (about 300 m) to 2.
- One density for the world and the figures: a standing adult is about 100 texture pixels tall up close, and a face about 14.
- Every level is drawn as pixel art for its band, with bolder marks and fewer of them, never the level above averaged: averaging lost about a fifth of the ground's accents and was the speckle you turned down.
- **A big surface has a tile for each distance,** each drawn from a picture of how the material looks from there, never the nearer tile shrunk (`PRE-22`), so no zoom repeats a small tile:
  - **near,** 4 m, for bands 0 and 1 (up to about 24 m across): blades, crumbs and pebbles;
  - **middle,** 16 m, for bands 2 and 3 (the close camp, about 24–100 m across): clumps, tufts and stones in drifts;
  - **far,** 64 m, for bands 4 to 6 (the camp zoom, about 100–800 m across): swathes of taller and shorter growth, bare and damp patches;
  - each tile is 256 texture pixels across at its first band, with designed levels below it, so all three fit one texture array; at a switch, the nearer tile's last level and the farther tile's first blend over the same short zoom as any two levels, and each place's patch picture (A4.6) varies all three, so none repeats as wallpaper;
  - beyond about 800 m, the world's own colours take over (A4.6).
- **Two to four versions of each tile:** each version joins every other without a seam (they share their edges), and each cell of the ground, 4 m for the near tile and 16 m and 64 m for the others, picks one by a hash of its place, so the same piece never sits on a regular grid.
  - *Estimated:* about 50 MiB more for the big surfaces, within A18.1's 300 MB.
- Grass, reeds, flowers and flames keep their true size in metres while their design follows the band; flowers, berries and eyes never fall below one texture pixel (`PRE-46`).
- Memory is the cost, not time: about 50–150 MiB of lossless textures at 64 a metre for about 150 materials with their levels, and about 30 MiB more for the middle and far tiles of about 30 big surfaces (estimates), within A18.1's 300 MB.

### A5.4 Where textures come from

- **Three routes, combined for each material** (`MIL-09`): code from rules, made on the phone; the world itself (rock layers laid by each world's geology, soot, stains, wetness, snow and traces: `PRE-23`, `PRN-10`); and pictures you approved, prepared by code.
  A ground texture holds only its material's background; paths, flowers, stones and tufts are things the world places.
  A cliff is rock A, the surface you picked, on the texel grid under each world's own layers laid by code.
- **The path of a picture-made texture:**
  1. **request,** committed text from a template: the area in metres, blocks of 8 to 12 picture pixels, the background material only, even overcast light, tileable, and an approved picture or the level above as input wherever one exists;
  2. **run** by the builder through Codex, outside the build and never in it; kept: the request, the prompt as passed, a run record and the original with its signed C2PA record;
  3. **vet** for Stone Age truth (A5.6), nothing countable in a ground, and the scale asked; recorded;
  4. **re-grid** by a deterministic cloud tool: the block size and its phase found window by window, each block's median colour becoming a texture pixel; seams blended where needed; a light check (a slope of at most 0.02); lossless.
     *Measured:* swatches asked for blocks re-grid with 2–8% loss;
  5. **designed levels,** one for each band where the material is seen: for big surfaces a near, middle and far tile, each from its own picture of the material at that distance (A5.3), and within each tile GPT redraws the next band from the level above, which then goes through steps 2 to 4 (your pick for the close camp's ground); for the rest a code reduction or code from rules; each level records the digest of the level it came from, so a change above marks it stale;
  6. **calibrate** in the look lab: the material lit at the target card's hours, with a few numbers fitted (lightness, hue, colourfulness, contrast) until each band matches band 0 and the card, keeping the accents, not only the spread;
  7. **approve:** a lab sheet in the note shows the texture flat and lit, at every band, at true size and enlarged, beside its source; your words and the date go into its record;
  8. **ship:** the levels are committed as lossless files and listed with their digests in `build.toml`, inside the look digest, so a texture change is a small update (`PLT-09`);
  9. **on the phone:** `view/` reads each file, builds one image holding all its levels (`Image.create_from_data`), gathers each material kind into a texture array and uploads them before the warm-up; the self-check compares the digests.
- **The record:** a `texture` catalogue kind (A3.6), one for each picture-made texture: what it is, band 0's size in metres and its texture pixels a metre, the original (its file, digest and C2PA record), how it was made, the re-grid's loss, the truth check, each band's file, digest and source, its calibration, and your approval.
- **Storage:** lossless with our own levels, read by `view/`, as Godot advises for pixel art even in 3D; Godot's import only for the icon and the interface, with its "Detect 3D" off; ASTC 4 × 4 only if the phone shows a need and your eye sees no loss.
- *Built in α2.3a:* the records are the catalogue's third source, `art`, beside `data/` in the repository and inside the phone's copy, each `record.toml` an entry named by its folder (`art:meadow`, `art:meadow/middle/v2`), its levels a list of tables; every field counts in the look digest alone.
  The build packs each record's levels, largest first, into one `.kdtex` (`textures/art/meadow/middle.kdtex`), which `view/` makes into one image whose mipmaps are its own levels; the self-check holds every texture to the build's digest and times making and uploading them all against a world's 3 seconds and A18.1's 300 MB.
  The Lab page lists every material; each one's sheet shows its tiles near, middle and far with their versions, each level at the bands it serves at true size (a texture pixel 2 × 2 screen pixels, worked out from the screen, since the interface is drawn 540 wide and stretched), its first level's corner at 8 screen pixels a texture pixel, and its record's words; in the cloud, 22 materials' 79 textures made and uploaded in 151 ms, 26.8 MB with their levels.
- **Code-made and world-made textures** are made on the phone at first start or with their area, and cached by look digest.
- **The originals** are kept beside their records as WebP in `art/sources/`, with the full original's digest and whether it carried its C2PA record; the full originals go to release files once those are proved from a cloud session (A2.3).
- **GPT's place:** before the build, never in it, so a failed or refused run, or a limit, only delays new sources.
- **Three uses, kept apart:** references (kept, never measured), targets (never shipped) and texture sources (re-gridded, approved, shipped); only sources need the full record.
- **Consistency:** every request uses one of your pictures or an accepted sheet as its style input, with the same wording; seasons come as matched sets sharing clump shapes; every GPT picture is checked by code (layout, drift) and for truth before you see it.
- GPT's size, block size, scale and seams are never relied on: it returned 1254 × 1254 for 1024 × 1024 and blocks of 12 for 8.
- Code-made textures' fingerprints are compared with the cloud's in the self-check.
  Each batch has a budget of pictures; at a limit the work waits and carries on with code-made textures, never buying credits (`PRC-01`).
- **The art lane,** your idea: a separate instance on the builder's machine prepares the content, the picture-made textures, the targets and guide pictures, and the kit's parts in Blender (A6.1), by the rules in `IMPLEMENTATION.md`.
  - It works only in `art/` and `tools/art/`, on its own branch, and runs GPT itself through the tools there, keeping a log of every run (`art/log/gpt-runs.md`).
  - It works with GPT as a partner, by your rules of 6 October 2026: GPT does more of the drawing and modelling and revises its own work; each piece is discussed and critiqued back and forth, never just ordered; both check it against the artwork it follows; and nothing reaches you below that level.
  - The builder reviews each batch (its records, its checks, its pictures enlarged, its log) before it joins, and you approve each material on its sheet.
  - The colour measures its checks use come from `kindling look`, the same C++ the engine's checks use (A4.8), so each is written once.

### A5.5 Targets and the loop

- **Targets:** one anchor picture you approve for each place, either a repaint of an engine frame that holds the right content or GPT's own scene made from approved pictures; its other hours and seasons come from relights.
  - Six of the nine relit pictures you judged kept the feeling, and you chose a relit dusk over a repainted one; a target is never repainted again to chase, since repainting drifts darker and busier round after round.
  - From each target the builder takes each material's colour, spread and density through the engine's material masks, and the moment's numbers for the card; the light's direction comes from the engine's physics.
- **The target card,** measured from the pictures you chose and refitted after every choice you make, warns and never decides.
  - Its fixed goals: no flat ground (at most about 5% flat patches); detail as things; golden lights (35°–79°); muted greens (at most about 12% in low sun); strong colour only in specks (at most about 3.5%), and no one colour over about 7% of a frame; texture as strong as the masses; warm lights (about +5 to +9 by day) with shade near neutral (about −2.4 to +2.1); each line measured on your chosen pictures.
  - **How it reads a frame:** in cells of 4 × 4 pixels, each its pixels' mean colour in OKLab, about 4 × 4 screen pixels on your phone or 2 × 2 texture pixels at the closest zoom:
    - lightness, the cells' mean; dark, the share below 0.45; golden lights, the brightest 5%'s hue; warm lights and shade, the lightest and darkest fifths' mean yellowness (OKLab's b);
    - strong colour, the share with chroma above 0.15; greens, the share with hue 110°–170° and chroma above 0.04, and their median chroma;
    - flat patches, the share of squares of 6 × 6 cells all within 0.02 of their mean colour; the commonest colour, the largest share in one box of OKLab 0.02 wide;
    - small things, spots 2 to 5 cells across (lightness standing out by more than 0.03 between blurs over 1 and 2.5 cells), each the strongest within 2 cells, counted a thousand cells;
    - texture, the spread of lightness less its blur over one cell; masses, the spread of lightness blurred over four cells.
  - **Its goals and bands are game tuning:** the goals, and how far past a band an alarm stays amber, in one file; each moment's bands in a file of its own naming its chosen pictures, from the lowest to the highest of them widened by that slack, and measured again whenever its pictures change.
    Nine moments so far: late afternoon (nine pictures), low sun, true midday, dusk, night (your three), winter, rain, storm and cave.
    Green inside, amber within the slack past a band or up to twice a goal, red beyond; every picture you chose passes, all green but the painted-over night's commonest colour, and a flat, speckled meadow made by code goes red on eight statistics.
  - On your two liked camps it reads as the first study did: lightness 57 and 46, dark 24% and 50%, the lights at 80° and 57°, flat under 0.1%, texture 6.3 and 5.6 against masses 9.3 and 8.3; its small things read 28 a thousand cells there and 18 on the flattest chosen picture.
  - Its bands for each moment start from A4.3's table, shown green, amber or red beside each view.
  - Tested on your answers, it caught 12 of the 18 pictures you turned down, and wrongly flagged 6 of the 17 you picked.
- **The loop,** at every step that changes the look:
  1. **draw:** one Godot run draws the fixed views at 1080 × 2404, with the colour, object and material pictures, lossless frames of the scripted paths, and many-sample pictures of small patches;
  2. **check:** A4.8's checks and the card's alarms;
  3. **look:** changed views beside their last approved versions, whole and in enlarged crops, on a lettered grid, so a fault is "C7";
  4. **judge:** a fresh subagent sees the pictures, the targets and a yes-or-no checklist for each Done when and truth question, and says which of each pair is closer to the target, asked twice with the order swapped; it reports faults, never approval;
  5. **show** you what changed, in pairs, with the alpha;
  6. **ask** only when the look has an open question.
- **Savings must be invisible** (`PRE-01`): first the machine line (A4.8), then your blind test on the phone, ten random pairs asking "which is sharper?", where eight or more right means it shows (guessing gets there about 5% of the time).
  - *Your first, in α2.1b* (6 October 2026): MSAA 4× against 2× on the stand-in meadow, 3 of 10 right, so the difference does not show and 2× stays.
  On the Compare page each pair is one view drawn two ways, one above the other, the better way placed by chance from the test's seed; you tap the sharper; the short code you send holds the seed and your answers, and the cloud reads it the same way. The first test compares MSAA 4× with 2× on the meadow.
  - *Built in α2.2b:* the page offers each test by name, and its code names it: Sharpness, the first; Leaf edges, C2's plants cut out plainly in one picture and smoothed by alpha to coverage in the other, each way on a layer only its picture's camera sees, asking which has smoother leaf edges; and Fire shadows, C5's three fires at night, the ground drawn by the walk in one picture and from the maps in the other, each ground on its own layer, asking which has sharper shadows.
  Half resolution applied to textures brings the shimmer back (9–16% of pixels), so a half-resolution saving may touch only smooth things: light, shadow and haze.
- **The heat step:** if the 20-minute heat run shows the picture alone heats the phone, one planned, logged step under heat, such as distant fires casting no shadows, chosen among the savings that pass your blind test, as you chose on 6 October 2026; if none is enough, it comes back to you.
- **The AI judge advises, never decides:** it reports faults, never approval.
- **Your choices:** two to four labelled pictures with a one-line cost each, in batches of four, with builds you open anyway; free of known faults; never offering a visible trade-off.
  A gallery for a setting with many values spreads wide first, then narrows, made by the engine or by code changing one setting only.
  Every choice you make is written into the section it decides.
- **Help only for your eye** (thinning leaves over a person, silhouettes behind leaves, a hold that marks every person) comes only if the phone shows people in shade are still hard to find, and then with your OK, as you decided.

### A5.6 Truth in pictures (`PRE-42`, `SCP-20`)

- GPT draws later or borrowed things and ignores "avoid" lines, such as metal tools after "no metal".
- So every target, guide and source picture is checked before anyone aims at it or prepares it:
  1. ask truthfully, with the prompt's truth lines;
  2. look at every made thing, animal and garment at twice size, against the known slips (`IMPLEMENTATION.md`, the art lane);
  3. date anything doubtful against a first-hand source, and write the verdict beside the picture;
  4. keep the picture's feeling, not its mistakes: a target is approved for its light, colour, density and composition, never for its flagged things;
  5. never take motifs from real cultures' art or dress (`SCP-20`).
- Truth costs no feeling: the truthful village and the first cave kept it.

## A6. The model kit and animation

### A6.1 The kit (`PRE-46`)

- **Parts like Lego.** Every shape is a part made in Blender: poles, hide panels, bark sheets, stones, branches, leaf clusters, tufts, body parts, garment pieces, hair, tools, rocks and cliff pieces.
  - Each part has its texture layout (A6.4) and named joints where it plugs into others; GPT makes and revises them through Codex, writing Blender scripts, in discussion with the art lane, both checking each against its guide pictures before you see it (A5.4), and you can open any part in Blender and change it.
  - Code puts parts together from recipes in the catalogues, varying size, count, angle, material and wear by seed, so a few hundred parts give thousands of things; a new thing is a catalogue entry.
  - Corners, creases and undersides are darkened in each part, and where parts meet when they are put together (A4.4).
  - By your direction of 7 October 2026, things are made of many parts and their organic forms sculpted in Blender with fine detail; the full sculpt is each part's master, and every form by height on screen is made from it (A6.3).
- **Plants:** about 8 forms.
  Trees and bushes are trunks, branches and leaf clusters from Blender, put together by growth rules for each species, stage and variant; leaf clusters are cut-out cards designed for each band and cut close to their leaves; one approved sheet for each species sets its crown, its colours by season and its stages, in the style of the birch sheet you accepted.
- **People:** one figure on one skeleton.
  - The body is made of Blender parts on the skeleton (head, torso, arms, hands, legs, feet), with build, age and sex as shape keys and part choices; near a joint the mesh follows both bones in part, so it bends smoothly.
  - Garments are shells over it (about 8 kinds, in child and adult sizes); hair, beads and paint follow each people's style (`CUL-12`); proportions by age are read from the family sheet you accepted.
  - Garments vary in material and colour within what the finds show (pale and dark hide, light and dark fur, striped coats), never one brown, which also helps people stand out from busy ground.
  - In full form, as many triangles as the sculpted detail needs within A18.1's budget of 4,000 (1,500 was the first estimate), until calibration scenes C3 and C6 set the line.
- **Animals:** six body patterns, one skeleton each, with bodies from Blender parts the same way.
  A species is its proportions, colours and markings, with small generators for antlers (tines by age), horns, tusks and manes; coats come from approved sheets, their markings varied by seed.
- **Shelters,** several types, each a layout of shared parts tied to what an excavation shows:
  - hides closing off a rock shelter or a cave mouth;
  - small round tents or huts of skins held down by stone rings, as a cone or a dome;
  - round post huts;
  - mammoth-bone circles, only where mammoths live;
  - brush huts over a shallow floor;
  - for farmers, longhouses.

  Windbreaks and lean-tos keep your comments on P3: poles, a bar and brush, and a roof in overlapping courses.
  Each type records the excavation it rests on and what is reconstruction; present-day peoples' details, such as smoke flaps, stay out (`SCP-20`).
- **Sizes:** anything small (ground cover, flowers, leaves, faces, held tools) has a design for each band it is seen at, drawn for that size and never only shrunk (`PRE-46`).
  - Code rules make repeated structure (blades, fronds, leaf clusters, cracks, pebbles), and tiny pixel designs written as data place the pixels charm needs (flower heads, berries, eyes, held tools), guided by GPT's design sheets: about 8 plant forms, 3 bands and 4 variants, about 100 designs, recoloured by species and season.
  - Band 0 holds the full design; the next band half the parts, one texture pixel wide, with light tips and dark bases kept; the close camp the silhouette, a light top, a dark base and the accents.
  - Cut-out levels have hardened edges, colour bled into their see-through pixels, and their coverage kept at every level, so leaves never fade away with distance.
- **Faces:** at the closest zoom a head is about 14 texture pixels tall, and its 12 states (the 7 feelings, calm, asleep, hurt, dead, and worn out with cold) are small designs drawn for that size; one band out they keep eyes, brows and mouth as single texture pixels; at the close camp the feeling moves to the pose.

### A6.2 Copies

- Models are layouts of shared shapes, drawn as MultiMesh copies in buckets sized for each form (A4.6).
- Each copy's look data holds its material's layer, its tint, its wear, its maker's style and a seed, so huts of birch and of reed look different with no new art (`PRE-42`, `PRE-43`).

### A6.3 Figures and movement (`PRE-27`, `PRE-44`)

- **One path:** a skinned mesh for each body pattern, with garments, hair and beads as further skinned meshes on the same skeleton.
  Held tools and carried things are copies that name their figure and bone, so the chip moves them with the hand; rigid weights give the old blocks as a special case.
- **Posing in C++ at 10 a second:**
  - key poses as joint angles, 2 to 6 for each movement, with the bending rules (stoop, limp, slump, hunch) and each figure's seed offsetting its timing;
  - gaits as numbers for each leg (how long each foot stays down and when it lifts), so walk, trot and gallop come from numbers and a limp is a layer; feet planted by two-bone solving;
  - still poses (feed, drink, rest, sleep, call, fight, play, fall) from approved pose sheets, as the deer's were.
  - birds and fish move by key poses and waves along the body.
- **Skinned on the chip:** each figure in full writes its bone palette (about 24 bones, three half-float texels each) into a data texture at its own pose step, and the chip bends every vertex from the palette its copy names.
  A MultiMesh shader can read the mesh's bone indices and weights, as Godot's source shows; this is probed in M2's first build and proved end to end by calibration scene C6 before anything relies on it.
- Whether a figure holds each pose for a tenth of a second or glides between the last two is one setting on the chip, for your eye.
- **Tiny figures and crowds** read a baked library: every movement's poses at 10 a second, made once at load by the same poser, about 900 poses, 0.5 MB for people and about 3 MB for the animals (estimates); a copy holds only its movement, start time and seed.
- **Forms by height on screen** (tuning values; each switch where both forms give about the same pixels, so nothing pops):

| Form | Height on screen | What it is |
|---|---|---|
| full | about 100 px and up | the sculpted detail, up to 4,000 triangles; textures about 100 texture pixels tall; faces as layers for each state |
| simple | about 40–100 px | about 500 triangles; the textures' next designed level |
| small, drawn to read | about 15–40 px | about 150 triangles; head and tool a little larger by bone scale; a designed level with the face and light clothes kept light; an outline in a darker shade of its own colour, drawn as a slightly larger shell behind it, growing from nothing at about 50 px |
| tiny | below about 15 px | enlarged up to about 4 times, baked poses (`PRE-28`) |
| marker | a group or herd close together | one mark (`PRE-28`) |

- **The fallback** is Godot's own skeletons for figures near the camera: Godot bends a mesh again only when its skeleton changes, so poses held for a tenth of a second are cheap, but each figure costs a draw for each surface and pass.
  Calibration scene C6 sets how many fit 1.0 ms of the main thread (an estimate of 15 to 40); beyond that, the batched way.
- Reads of a texture in the vertex stage count as dependent reads on this chip, so palettes are half floats and far forms use one bone a vertex.
- *Built in α2.2b,* for C6: the poser's stand-in, a figure of about 1,500 triangles on 24 bones walking by joint angles, posed 10 times a second, each figure's steps offset by its seed; its skinning matrices go to Godot's skeletons, or into a palette row of half floats, three texels a bone, the same numbers both ways.
  A MultiMesh's shader reads the mesh's bone indices and weights as Godot's own skinning does, and in the cloud it bends every vertex within 2 mm of where Godot's skeleton bends it.

### A6.4 How textures sit on shapes (`PRE-22`, `PRE-46`)

- **One rule everywhere:** texture coordinates are in metres, so a texture pixel is 1/64 m on every surface at band 0, on people, tents and ground alike, and none is stretched beyond about 1.5:1 (`PRE-22`).
- **The ground** is mapped from above in world metres, offset with the moving origin so its texture pixels never drift; above about 45° of slope, where a top-down map would stretch a pixel past 1.4 times its length, faces take a projection from the side.
- **Cliffs and other rock:** each triangle takes one projection, from the direction it faces, with height as its vertical, so the texture lines up with the rock layers laid by code; never a blend of three projections, which smears pixel art. Where two faces meet, their texture pixels need not line up: the edge is where the light changes anyway.
- **Each part is unwrapped in Blender at this density:** a pole, trunk or branch around and along, its circumference rounded to whole texture pixels so the wrap never shows; a hide panel or bark sheet by its flat cut shape; a stone by one projection a face.
  - *Made in the art lane's round 2:* a wrapped part takes its texture from its material's **wrap atlas** (`art/textures/<name>/wraps/`), a 256-pixel tile cut into strips of 64, 48, 40, 32, 24, 16, 12, 8, 4 and 6 texture pixels, each seamless round its own width (`tools/art/kitmath.py`, `WRAPS`); a part's circumference takes the nearest strip, so the engine reads each wrapped part from its own strip.
- **Figures:** each limb unwraps like a sleeve, around and along its bone; garments are shells that reuse the body's layout; the face is a small design on the head's front, about 14 texture pixels tall. The coordinates belong to the mesh, so texture pixels ride on the body as the skeleton bends.
  - *Made in the art lane's round 2:* a figure's texture is drawn to its layout, kept beside it as `layout.png` on a square canvas whose bottom row is coordinate v = 0, and is never tiled.
- **Plants:** leaves and grass are cut-out cards carrying their design for each band; trunks and branches are sleeves of bark.
- **A check in the cloud** measures each part's stretch, triangle by triangle, and fails any outside the line.

### A6.5 Sheets

The model sheet of every kit shape in two materials, filmstrips of every movement (since motion can't be judged from a still), and the band sheets, every entry at true size and enlarged at each band, are rendered in the cloud whenever the kit changes, for your eye.

## A7. The world

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
- Tuned for the look: maps and close-ups are judged by eye at every zoom stop, against the look M2 sets, and the climate is checked on Earth's real relief, as Around The World did.

### A7.5 Detail on demand (`WLD-13`)

- An area is made from the seed, its world cell and neighbours, and the date, the same every time; making it for the picture changes nothing.
- Only what people change is kept (`WLD-12`).
- The ground's height and cover anywhere come from the cells, joined smoothly, with relief only as fine as the spacing shows (P8).

### A7.6 Starting small

The generator is first built and tuned on a small island, quick to make and judge by eye, then grown to full size.
*Measured in pre-production* (P7): your phone made three worlds in 9.3 s and settled the first in 6.5, about twenty times the room `WLD-11` gives, so the stages can grow richer.

## A8. From a person to the globe

### A8.1 Levels of detail (`PRE-03`)

Our own system in `view/`, driving Godot's RenderingServer directly: one tree of square chunks over the whole world, each splitting into four as the camera nears, so the ground is detailed within about 300 m from camp zoom inward, shaped every few tens of metres out to about 10 km, and the world cells' own from the region out.
Each chunk morphs into the grid half as fine as the camera draws away (CDLOD, Strugar 2010), so no level pops, and a full area shows its coarse ground until its detail is in, within about a second.
*Changed in P8's second round,* after your verdict on 5 October 2026: the rings, the map mesh and their dithered hand-overs, built first, gave way to the tree, since the hand-overs and the map's bend were what you saw jump.

The tree's chunks are made on worker threads, nearest first, and freed by when the tree last reached them; only what the camera can see is drawn or split (P8's second round).
The tree also carries each area's patch picture and the view's maps (A4.6), and the ground's texture changes its designed level with the zoom band (A4.2), so the ground, its cover, its darkening and its colour far away follow one rule.

### A8.2 A moving origin

Godot draws in single precision around an origin that moves with the camera, as Kerbal Space Program does; the simulation's exact coordinates never depend on it.

### A8.3 Things by distance (`PRE-28`)

- **Copies only while they cover about 12 screen pixels;** smaller detail is the ground's band texture (A4.6), and rules read what the world keeps (single plants, stones, patches), never a tuft.
- **Trees** as models, then low-poly crowns wearing textures designed for their band, dense enough to read as forest, then the cover's colour; impostor cards only where a need is measured, drawn as pixel art for their size.
- **People and animals** by their height on screen (A6.3): in full, simple, then small figures drawn to read, outlined in a darker shade of their own colour with faces and light clothes kept light and tools drawn larger, then tiny; a group or herd close together as one marker, and a camp as a point at its hearth that glows if it has a fire.

One rule for each thing, on the graphics chip, read by everything that draws it, so trees, plants, ground and camp always agree (P8's second round).

### A8.4 The camera (`PRE-03`, `PRE-29`)

- **One perspective rig at every stop, with no pixel lock**.
- **Up close, a narrow view:** about 5° to 10° across in portrait (a tuning value), tilted about 35° to 40°, as in the pictures you liked; a wide lens at this tilt squashes texture pixels at the top of a portrait screen to a fraction of their height at the bottom.
  With a lens of 5–10°, texture pixels stay within about 5–11% of 2 × 2 from the bottom of the screen to the top; the near and far planes hug the scene, so depth stays precise and the sun's map is spent on what is seen.
  Your eye judges the view against the liked pictures' flat look at first light.
- **As it rises,** the view widens and tilts toward straight down (`PRE-29`).
- **The ease** to rest on 5° and 1.25× steps when the fingers lift stays as you chose it, now for calm framing rather than steady pixels.
- The near and far planes hug the scene, so depth stays precise and the sun's shadow map is spent on what is seen.
- **The rig's state:** the focus in whole centimetres, the turn, the pitch, the field of view and the metres per screen pixel; it publishes the zoom band, the sun, the time and the moving origin to every shader.
- **The globe:** the flat map bends onto a sphere for the last step, as Google Maps morphs to its globe, squeezing the polar lands and hiding the seam under the ice (`WLD-02`).
- *Changed in P8's second round,* as you asked on 5 October 2026: the world is a sphere at every scale and never unrolls; near the focus the ground is stretched east to west back to its true size as the zoom closes in, so the close stops are as before; each chunk is set on the sphere by its vertex shader, exact near the focus.

### A8.5 The map look (`PRE-29`)

World cells in flat cover colours, rivers as lines (from the region out, those draining about 1,000 km² or more), hills shaded the cartographers' way, lit from high up at every hour with only the tint following it, sea in depth bands with the shore's bright line (`PRE-26`).
*Changed with your OK on 5 October 2026:* the land vivid and textured with what can be seen from above, lit by the sun of the hour, with the weather's clouds and their shadows (`PRE-29`), as P8's second round draws it, below.

### A8.6 Time and light by zoom (`TIM-01`, `PRE-30`)

One gesture sets where you look and how fast time runs, from real speed at the person to top speed at the globe.
From the valley out a day passes in under a second, so the light holds steady.
Weather, clouds and air are drawn from the climate on the graphics card.

### A8.7 Budget

Each level's cost is measured on your phone at every zoom stop (`PLT-04`), and so is the time to make a full area.
*Answered in pre-production* (P8): built twice and judged by you; its Measure on your phone is still to come.
The close stops' budgets are A18.1's; the camp zoom's stress scene, a camp in thick forest with the camera turning (S9), has a line of 6 ms of the graphics chip.

## A9. Living things, outline

- Each species is a catalogue entry: climate and soil ranges, seasons, size, diet, group size, yields, life cycle, model-kit form and colours (`WLD-31`, `WLD-32`).
- Plants by ecology: which types can grow from BIOME1's numbers, which win by dominance, how dense by competition and self-thinning; single plants placed from that density by keyed chance, so a place always grows the same plants (`WLD-13`).
- Animal numbers by Damuth's law: each species' natural density from its body mass, a sixth of it per cell (`WLD-30`); predator and prey rules on the cells' totals, driven by the weather (`WLD-18`).
- Herds' days by need zones and hours, their years by following the green-up, their movement by Reynolds' steering rules plus those goals.
- Near people, individuals; far away, counts, with condition and wariness carried both ways (`WLD-32`).
- *Proved in pre-production* (P9): these rules on the cells' totals held every species within 0.66 and 1.10 of its total for a century in 20 worlds.

## A10. People: bodies and lives, outline

- Needs as levels that run down at their own rates, and things and places advertise what they offer, as in The Sims (`BIO-09`).
- Energy by real numbers, by body size and activity; food values from the catalogue (`BIO-10`).
- Wounds by body part and layer, with bleeding, pain, disabled limbs and infection per wound (`BIO-13`); illness as a race between severity and immunity, care slowing severity (`BIO-05`, `BIO-23`).
- Births by biology: fertility from age and nourishment, gaps from breastfeeding (`BIO-15`); inheritance blended with variation, rare traits passed by chance (`BIO-06`).
- Whole worlds must land near foragers' real numbers: about half of children reaching 15, adult deaths peaking near 70, three-year birth gaps, illness the main cause of death (`RES-14`, `BIO-04`).

## A11. Minds, outline

- **Choosing by utility:** every known action is scored by data-driven response curves against needs, personality, mood, plans and beliefs, and drawn by keyed chance among the best; the top reasons are kept for the card (`MND-09`, `PRN-13`).
- **A small planner on top** for jobs of several steps, each step re-checked by utility, so people still react to a wolf.
- **Decisions only when an activity ends or is interrupted,** never per tick (A3.3), spread over threads in fixed chunks.
- **Feelings by appraisal; mood as summed thoughts** with thresholds moved by traits; memories at three depths that can change personality, with a date (`MND-19`, `MND-29`, `MND-30`, `MND-18`).
- **Knowledge per person with its source:** seen, told by whom, worked out; beliefs can be wrong, misremembered or spread by talk, and choices read only this, never the world's truth (`PRN-01`, `MND-02`, `MND-23`).
- **Opinions move in talk;** social acts and norms are data rules over relationships and personality (`MND-33`, `CUL-24`).
- **Paths in levels:** connected regions, then cached cluster paths, then A* or flow fields inside clusters, recomputed only where the land changes.
- **The estimate:** a thousand people at a game year a real minute is about 48,000 decisions a second, some 80 µs each on four cores, before bodies, talk and paths (`TIM-07`, `MND-15`).
  *Measured in pre-production* (P6): about 32,000 decisions a game day for a thousand people, 6.0 game years a real minute on your phone's four cores; fuller minds may cost about 2½ times as much before the speed falls below 2½.

## A12. Crafts and discovery, outline

- An item's 18 characteristics come from its material and form, and made things inherit from their inputs (`MAT-03`).
- Blueprints match characteristics and classes, never names (`MAT-04`, `PRN-07`), fenced by the expected-fits check (`MAT-17`).
- Every reality rule has a real experiment behind it, cited in its catalogue check (`RCK-01` and the rest of section 7.6).
- Quality from skill and inputs (`MAT-20`); skill grows by the power law, a few years to competence and five to ten to mastery (`MND-06`); teaching beats watching (`MND-13`).
- Discovery belongs to people: accidents, personal hunches and copying found things (`MND-11`); crafts die with their last holder and return only by rediscovery, neighbours or copying (`CUL-02`, `CUL-16`).
- *Proved in pre-production* (P4): tuning alone, with the world's own rules, brings flakes and fire into their windows; each blueprint's discovery factor lives in its catalogue entry and is re-tuned with the whole catalogue (`RSK-01` stays open until M7).

## A13. Culture, outline

- A naming language per people from the seed, in O'Leary's way, spelled only with letters the pixel font has (`CUL-17`, `CUL-18`).
- Customs as fixed questions with a few answers, each answer from a band's own cases (`CUL-06`).
- Beliefs and rites from coincidences: the act before a good outcome becomes a rite, harm after an act a taboo (`CUL-05`, `CUL-20`, `CUL-34`); small bands keep vivid, rare rites, large villages regular ones (`CUL-26`).
- Societies with real numbers: bands of about 28 adults, a few families linked by kin and marriage; leaders kept in check; gifts as insurance; villages only where stores allow (`CUL-30`, `RES-07`).
- Violence in its real order: personal killings and revenge first, raids growing with stores (`CUL-31`).
- Stories and gossip drift as transmission chains do; styles drift by copying with small changes (`CUL-11`, `CUL-12`).
- *Proved in pre-production* (P10): customs, spirits, rites and splits came from events alone, each inside its window in most of 20 worlds.

## A14. Story, the book of ages and the writer, outline

- **The director** keeps Left 4 Dead's rhythm without its power: peaks, then a guaranteed rest; it reads the world and sets only speed and live moments (`TIM-02`, `TIM-03`).
  A test runs a world with it on and off and compares the results.
- **Recognisers** are story-sifting patterns over the event log; half-matched ones are the director's signs, so time slows before an outcome without looking ahead (`PRE-39`).
- **Pattern sentences:** a small grammar, at least 5 phrasings for each kind of event, picked by the event's seed and filled from its records (`PRE-37`).
- **The writer:** Gemini Nano through ML Kit's Prompt API, with a fixed seed, sentence by sentence, behind the Android plug-in; it writes only while the app is in front, queues and backs off, and stops for the day at its battery quota.
  Every rewording passes a strict check without any model, and dark events never reach it (`PRE-41`, `PRE-17`); pattern text always works alone.
  Its second option is ML Kit's Rewriting API ("Rephrase"), and its fallback a small Gemma through LiteRT-LM.
- *Proved in pre-production* (P11): the director slowed time 9.5 times an hour within its budget and caught every named discovery, and every world ended identical with it on and off; signs are scored by how often they come true.
- *Not proved:* P13 was left unbuilt when pre-production closed; the writer is proved when M9 builds the book, and pattern text stands alone until then (`PRE-37`).

## A15. The interface

- **The world fills the screen;** panels show only what the moment needs, then fade (`PRE-32`).
- **One column of panels:** full width with controls in the bottom third in portrait, beside the world in landscape (`PRE-34`).
- **Every control at least 48 dp,** with 8 dp between.
- **Our own gesture reader on raw touches,** so all gestures share one rule set and a scripted test can tell them apart; edge swipes stay out of the system's gesture insets (`PRE-33`).
- **One Godot theme,** its palette chosen with you when the interface is built.
  The plain pixel font for everything read, at whole multiples of its design size, nearest filtering and no subpixel positioning; the pixel handwriting only for big titles, at twice the size.
  The layout is built on a square base, so both orientations scale alike; safe areas and cutouts come from `DisplayServer`.
- **Cards open to what matters now,** with deeper sections folding out (`PRE-35`); screen-reader labels come with Godot 4.5's support.
- *Built in pre-production* (P12): art pixels at a whole multiple of the screen's pixels, the pixel fonts, one gesture reader and every control within reach; your verdict on reach, legibility and the panels' ground is still to come.

## A16. Sound, outline

- Ambience as layers from the place's land, water, weather, hour and season, plus one-shots only from real things near the camera (`SND-11`, `PRN-10`).
- The 44 base sounds made by our C++ at load and varied each play; live synthesis only for what follows the world continuously, such as fire by its heat (`SND-06`).
- Godot's 3D players for direction and distance, their low-pass for distance, an area with reverb for each cave; muffling by land from the simulation's line test (`SND-08`).
- A voice manager keeps `SND-01`'s 32 voices, blending the quietest into its kind's hum when a share is full (`SND-07`).
- The murmur in the people's language, shifted for age, build and feeling (`SND-03`); the speaker's missing bass restored by harmonics, off with headphones.
- *Built in pre-production* (P14): in the cloud, 32 sounds at most with the mix at 2.7% of the audio thread's time; a 3D sound world needs a camera to sound; your ears and Measure are still to come.

## A17. Testing and checks

### A17.0 Traps met so far

- **Godot:** Movie Maker records at the project's window size, set by an `override.cfg`; a rendering driver named on the command line brings Forward+ unless the Mobile renderer is named beside it; Godot 4.7 can abort as it exits after importing new files in the cloud, and a second import exits cleanly; the headless audio driver mixes, but a picture or a 3D sound needs a camera; `call_deferred` from a worker and `WorkerThreadPool` tasks are each waited for exactly once; a control's own `_draw` lies behind its children; a wrapping label measures itself at zero width until laid out, so panels are sized by containers.
- **godot-cpp:** a method that takes a native structure appears only if the build profile names the structure; `OS` is needed by its own printing; a local class cannot hold a member template.
- **The phone:** the screen runs at 120 Hz unless capped again at run time, since Godot sets the cap before the swapchain exists; the chip lowers its clock with time to spare, so its milliseconds are partly idle; Android forecasts heat only while asked within the last 10 s.
- **The cloud:** the software Vulkan driver can crash on some shaders at some angles; waits are loops with a time limit; heavy jobs run one at a time.
- **C++:** no fast-math, no contraction, our own transcendental functions; clang-tidy's analyzer misreads doctest's own strings as leaked in some tests.

- **C++ tests** with doctest, and property tests with RapidCheck for rules that must always hold, such as no result heavier than its inputs (`MAT-09`).
- **Scenes and whole worlds** run by the C++ library alone, through the `kindling` tool, many at once in the cloud: each scene is a TOML file in `data/scenes/` stating, before its first run, the items it checks, its seed, its runs, its time limit, its budget and its pass rule, counted over about 20 runs where chance matters (`RES-21`, `RES-09`, `RES-13`).
  A run keeps checkpoints and resumes from the last as if it had never stopped (`PLT-05`); a run with a test switch says so in its report and its world (`RES-10`).
  - *Built in α1.5a:*
    - **Scenes** (`kd/scene`, `data/scenes/`): a scene's file states the items it checks, its kind of world and size, its first seed, its runs, its game time, its time limit and budget, its test switches (every run all of them, or one each in turn), its pass rule, the ranges it expects and the rules it must never break.
      It is read through the catalogue's loader, so a mistake is named at its file, line and column, and its measures and rules must be ones its kind of world offers: the crowd's are the greetings, the greetings a camp a day, the farthest any marker strays from its camp, and the most awake at midnight.
    - **Runs** (`kindling scene`): each run is a world in a process of its own, as many at once as the cloud has cores, kept in a folder of its own under `build/scenes/<scene>/` with a checkpoint and that day's sample at each game day's end; a scene stopped and run again takes the runs that ended from their results and resumes the rest from their checkpoints.
      A run's measures come from the history its folder keeps and from its days' samples, so a resumed run counts exactly as an unbroken one.
      Its samples go through its keeper's I/O thread, like everything in a kept folder (A3.7): written straight to the folder, they raced the history's writes, which the C library caught once as a damaged heap and the thread checker then named.
    - **Judging** (`RES-13`): the rule counts the runs that meet it, a run that gave no measure failing it; a rule that fails on 20 runs or more runs as many again on fresh seeds and is judged on all, its count scaled and rounded against passing; fewer than 20 runs judged are provisional.
    - **Oddities** (`RES-12`): a measure outside its expected range; a never rule broken, named at its first day; a crash, by its signal; a run over its time limit, stopped; memory that creeps up by more than 32 MB over a run; and a last save that does not open again as the world was.
    - **Test switches** (`RES-10`): compiled only into the simulation's own build for its tests and tool (`KD_TEST_SWITCHES`), never into the game's, which the flags scan checks.
      A world's switches are part of its clock's chunk (version 2; older saves are upgraded to none) and of its digest whenever it has any, and its `world.toml` marks it as a test's world with them, which the Worlds and Crowd pages show; the game reads a test world's switches but never turns one on, so such a world carries on there without them.
      One switch turns greetings off; six each plant a fault for the checks to find.
    - **Reports** (`RES-06`): `report.json` beside the runs holds the scene as stated, its rule in words, the verdict, the real time against the budget, the version its worlds were saved under, each measure's range over the runs (lowest, middle, highest, and in how many runs its expected range held), and each run with its seed, switches, measures, oddities and digest.
      Each build runs the scenes in `data/scenes/` under the app's own version and puts each report in the game's data, with the world of its first odd run, or else its first, as a `.kindling` file; the Reports page shows each with a chart of every measure, drawn to scale, and opens that world, imported once, as a test's world exactly as it ended in the cloud.
    - **The checks** (`tools/scenecheck.py`, in `tools/check.sh`): every scene passes with no oddity; the planted scene (`sim/tests/scenes/planted.toml`) flags each of its six faults and nothing else; a rule that fails once is judged on 40 runs; and the repeat check runs the greetings scene on one core and on four, stopped once some runs are done and another is days in, then resumed, and keeps the benchmark world of 10,000 markers two game days on one core and in islands of one-minute windows on four, stopped after its first day and resumed; their reports, histories, journals, samples and end states must match byte for byte.
    - *Measured in the cloud:* the greetings scene, 20 worlds of 4 camps over 20 game days, in 0.6 s on all cores and 1.6 s on one; the benchmark world's two game days in 0.8 s on one core and 2.7 s in islands on four, as α1.3b found for the crowd's light events; the scene check in about 7 s.
      Islands of hour-long windows join nearly every marker into one island and cost far more, since each marker's circle then covers hundreds of 250 m cells and every pair in a cell is tried: forming islands needs the cheaper joins A3.3 names before minds use long windows (M6).
- **The same results everywhere** (A3.4): seeded worlds on the five builds, on one to four threads, with islands of several window lengths, stopped, saved, reopened and resumed; libc++ with its tie order randomized, and the order fuzzer scrambling EnTT's pools; every digest must match (`RES-05`).
  The check also reads the compile commands for the last floating-point flag and unsigned `char`, scans our built code for fused instructions and for the platform's maths functions, holds the maths library to MPFR's correctly rounded answers, and checks the banned list on the code's syntax tree, each rule proved by a planted use.
  The phone's compiler builds the proof suites twice more with libc++'s order of ties randomized under two seeds, and their digests must match too.
- **Saves:** the headless tool killed at random a hundred times mid-run, each reopening carrying on exactly (`PLT-07`); damaged files (cut, a flipped bit, zeros) refused; the corpus of old worlds opened by every build (`PLT-09`).
- **Catalogues:** a test catalogue with one planted fault for each check, each refused at its file, line and column (`MAT-17`).
- **The Godot side** with gdUnit4: layouts, cards and views opened from records, headless; gestures by simulated touch under Xvfb, since Godot's headless mode drops input events.
  Every script is compiled before the tests, so an error in one no test loads still stops the check.
- **Pictures and reels** by Movie Maker mode at a fixed frame rate: golden pictures in the cloud; the contact sheet and the sound reel on the phone for your reviews (`PRE-31`, `SND-12`).
  - From M2, the look's own checks (shimmer, the texture pixel's size, banding, ground accents, people against their surroundings, savings invisible) and the loop that runs them are A4.8's and A5.5's.
  - Movie Maker records at the project's base size, so a single picture at the phone's 1344 × 2992 pixels is read from the screen by our script (`tools/picture.sh`).
  - A rendering driver named on the command line brings Forward+ unless the Mobile renderer is named beside it.
- **Phone measurements:** the in-app benchmark (A18.1), its frames by our own measure, its result in a short code; trace sections that the phone's own System Tracing records beside the chip's speed and heat, with no computer; Android GPU Inspector for a slow frame on the PowerVR chip, if ever needed (`PLT-04`).
- **One command before anything joins:** `tools/check.sh`, rebuilt for C++ and Godot, runs the formats, lints, builds, tests, the same-results check, the scenes and the repeat check, and the file, commit and coverage checks (`PRC-10`, `PRC-12`).
  - It costs about what changed, since every delivery waits on it: C++ compiles through ccache, so godot-cpp and unchanged files compile once across runs and build folders; each C++ file is linted, on every core, only when its code, the headers it reads, its compile command or the rules changed since it passed, and each project's tests, the Godot project's import and tests and the picture test run only when something they read changed (`tools/cppcache.py`); the C++ tests run beside the Godot and tool tests; and each step prints its time.
    Measured on 5 October 2026: about 20 seconds with nothing changed, about 45 with one line of one library changed, and about 5 minutes the first time, which fills the caches.
  Production's foundations bring every kind of check to the game's own code (M1).

## A18. Budgets and risks

### A18.1 Budgets

As measured on your phone in pre-production, each re-measured at every milestone (`PLT-04`):
- **Frame:** 16.7 ms at 60 frames a second, the graphics chip within an 8 ms planning line in the busiest close scene, so heat leaves room; at least 97% of frames on time while moving the camera (`PLT-04`).
  - Pre-production measured its low-resolution picture: the close camp (P1) 99–100% of frames on time at 60, the chip about 10 ms a frame, partly idle; a camp at night with thirty figures and three fires (P2) 5.5 ms of the picture's pass at 120; the model sheet with fire shadows and smoke (P3) 4.3 ms at 120.
    The look you chose draws four times the pixels, so these numbers no longer hold; M2's calibration replaces them.
- **The graphics engine,** every number an estimate until calibration scenes C1 to C6 and stress scenes S1 to S9 replace it, each pass line stated before its first run (`RES-09`):
  - the graphics chip in the busiest close scene, in milliseconds at a 120-frame cap, every viewport summed, with a run with the world hidden for the fixed part:

| Part | Allowance | Estimate with every saving that doesn't show | Lever if over |
|---|---|---|---|
| Fixed: the interface pass, the copy, the stores | 1.1 | 0.9–1.7 | a lighter interface; buffers that never reach memory (patch) |
| Shading: the material, light, soft shadows, grade | 2.3 | 1.7–4.0 | fewer reads; 16-bit maths; shading per 2 × 2 pixels on ground and plants (patch) |
| Sun shadow map, 2,048, small casters | 0.4 | 0.3–1.5 | a smaller map at far zooms |
| Geometry, all passes | 1.6 | 1.2–7.7 | detail by size; fewer leaf triangles |
| Leaves' hidden layers | 0.8 | 0.6–8.0 | the leaf pre-pass (patch) |
| Fires: maps and shadows | 0.4 | 0.1–1.2 | the nearest four a pixel |
| Water and mirror | 0.6 | 0.1–3.3 | the mirror's coverage |
| Effects: smoke, mist, rain, snow, glow | 0.5 | 0.1–3.3 | rain streaks as copies; mist at half resolution |
| Darkening maps | 0.1 | 0.03–0.1 | |
| Reserve | 0.2 | | |
| **Line** | **8.0** | | |

  - **triangles:** at most 0.4 million triangle-passes a frame, every pass counted, until calibration scene C3 sets the line, with detail falling with size on screen, since this chip's efficiency drops below about 32 pixels a triangle;
  - **the main thread:** 8 ms on average and 12 ms at the 99th percentile: Godot's culling and draw recording 3.0 (about 300 draws over all passes), figures 1.0, plants' and patches' buffers 0.5, scripts and interface 1.0, Godot's other work 0.5, other view work 0.5, reserve 1.5;
  - **figures:** at the closest zoom at most 10 in view at up to 4,000 triangles; at the close camp at most 100 at 1,000–1,500; at the camp zoom up to 3,000 tiny ones at up to 60, with no sun shadow;
  - **power and heat** at close zooms: the whole phone at most 4.0 W on average over a 20-minute run (about 20% of the battery an hour): the screen about 1.0–1.4, the graphics chip at most 2.0, the processor for the picture at most 0.6, the simulation at most 0.3; the heat forecast at least 0.05 below the phone's own light threshold, and the battery at or below 40 °C at the end;
  - **each scene:** the graphics chip at most 8.0 ms on average and 9.5 ms at the 95th percentile, S9 at most 6.0; the heat run also gives the watts a millisecond costs and whether 120 Hz costs power against 60; the Pixel's Game Dashboard frame counter is a second check.
  - **memory:** textures at most 300 MB uncompressed; render targets at most about 150 MB (about 100 MB at full resolution with 2× MSAA, 42 MB of it the multisampled buffers Godot keeps in memory though it never writes them); the app at most 1 GiB.
  - *Estimated:* at an 8 ms picture the whole phone draws about 3.1–5.6 W, 16–28% of the battery an hour, within `PLT-04`'s 25–30%; comfortable long play needs about 4 W or less, so the heat run, not the battery, sets the true line: if the phone stays cool at 9 or 10 ms the line can rise, and if it heats at 8 it comes down for good or takes its one planned step (A5.5).
- **Simulation:** up to the four middle cores at held speed (`PLT-01`): a thousand simple minds at 6.0 game years a real minute (P6), so production's fuller minds have about 2½ times P6's cost before `TIM-07`'s hoped-for speed falls.
- **World generation:** three worlds in 9.3 s and settling in 6.5 on your phone (P7), against `WLD-11`'s 3 minutes and 1.
- **Sound:** 32 sounds at about 3% of the audio thread's time in the cloud, 9% at worst (P14); the phone's figure to come.
- **Power:** about 3 W while playing; **memory:** within about 8 GiB.
- **The APK:** 25 MB from Godot itself, 38 MB with every prototype, 27.6 MiB at M1's end; within the 50 MB limit for files committed to the repository (A2.3).
- **The foundations,** from the cloud, each measured again on your phone by M1's benchmark:
  - the event queue: 1–4% of one core at `TIM-07`'s speeds;
    measured in α1.3a, the crowd of 10,000 markers ran 60 game days, 14.4 million events, in 8.1 s on one cloud core: about 0.56 µs an event with its handler and a digest of the whole state each game day;
    since markers meet and greet (α1.3b), about 1.4 µs in the cloud (5.2 million events in 20 game days, 7.4 s), and about 0.9 µs on your phone's fastest core, which held 4.3 game days a real second at top speed (α1.5b);
  - drawing 10,000 walkers: about 0.26 ms of the main thread a frame (filling and uploading their buffer);
    measured in α1.3c, 0.8 ms in the cloud, 1.1 ms for nine frames in ten, placing each walker from its ways and filling sixteen areas' buffers;
    on your phone (α1.5b), 3.4 to 3.6 ms of the main thread a frame while the camera tours, with every frame on time: well over the hoped-for 0.26 ms, and the first cost to win back as figures replace squares (M2);
  - loading a launch-size catalogue: 25–33 ms;
  - a save: a pause of tens of milliseconds to copy the state at an event, the rest on other threads;
    measured in α1.4a, 3.4 ms for the 10,000 markers in the cloud, with 3.3 ms of compression on the I/O thread; 20 ms at most on your phone (α1.5b);
  - opening a world: within `PLT-04`'s 3 seconds; 143 ms on your phone, its export 23 ms (α1.5b).
- **Your phone in M1's benchmark** (α1.5b, 6 October 2026): 99.8 to 100% of frames on time and the slowest 49 ms while the camera moves, at every speed; the heat forecast at most 0.83 of the first throttling level, so time never had to slow; about 1 W at real speed and 5.6 W at top speed, the screen's own share about 1 W; 330 to 410 MB of memory; every scenario's world ending as the cloud's.
- **Storage** (`PLT-10`): a world of 7,000 people at Year 250 estimated at 3.5–4.1 GB, against 4 GB: its history fits at about 17–30 events a person a game day.
  - Measured in α1.4b, a record of the history takes 68 bytes as written, too many for that estimate: a year will be compressed as it closes (zlib takes the crowd's to a third) and its records made smaller, when people's lives begin to fill it (M5).

- *Built in α2.2a:* the calibration scenes are files in `data/scenes/look`, each stating before its first run its items, what it draws (nothing, the field, rocks or copies), its camera, its variants' switches (MSAA, the 3D's scale, the interface, the sun's shadow pass, triangles, copies and passes), the number that decides, this table's line, the estimate and the decision each band of the number makes; `kd::look` refuses a scene without them (`RES-09`).
  - The Calibrate page runs them in the order of their names, each variant drawn for 3 s, read for 10 s at a 120-frame cap (the graphics chip's mean time, every viewport summed, and the main thread's: the frame's setup and each viewport's culling and draw recording), then settled for 2 s and read for 20 s at 60 (frames on time, the battery's power, the heat forecast at the end); about 15 minutes for C1, C3, C3's draws and C4.
  - One code holds every reading (layout 1: the version in 8 bits, the build in 17, then a variant's graphics and main-thread time in microseconds in 16 bits each, frames on time a thousand in 11, power in milliwatts in 15 and heat in hundredths in 9, each its reading plus one and 0 for none, then a CRC-24); `kindling look calibrate <code>` reads it against the same files and gives each scene's number, its line and estimate, and its decision, in the same words as the phone.
  - A number is the deciding variant's time less its minus's, as C1 takes C4's bare frame off, or for triangles the slope of the graphics chip's time over the variants without the shadow pass.
  - In the cloud, `tools/calibrun.py` runs every variant on the software driver and holds what each drew to its file: rocks give exactly their triangles in each pass they have, copies exactly their draws; Godot draws a sun's shadow for every view that sees the sun, so the mirror's view is blind to the sun's layer and adds one pass, not two.
- *Built in α2.2b:* a scene names the step whose run it belongs to, and the Calibrate page offers the build's own step's scenes first, with a box for each; a scene picked picks the one its minus takes from.
  - A scene may decide by the least of several variants, as C2 takes the cheapest way of drawing its leaves among those that draw plain cards' picture.
  - The code, layout 2, holds which scenes ran in 16 bits after the build; `kindling look calibrate` still reads α2.2a's layout 1, against that build's own files.
  - C2's stand-in plants, set out by a hash of their place along a bank, a path and a meadow, put about 850 copies in view on your phone's screen; Godot counts a draw for each instance seen, however many surfaces it has.
  - C5's fires burn at night, each with two people standing, one sitting, two logs and nine hearth stones, which stand in the height maps; a viewport drawn only now and then, as the fires' maps are, counts its last drawing's time for its share of the frames, since Godot keeps a viewport's time until it draws again.
  - C6's figures stand in a grid over the field by day; on Godot's skeletons each is a draw in each pass, on palettes all of them are one, and the main thread's reading adds the figures' posing, which Godot's clocks leave out.
    Its reads are points set out over the screen by their indices, each reading its texels before it is placed, so no compiler leaves a read out.
  - In the cloud, `tools/calibrun.py` also draws one figure as points, a vertex each, bent by Godot's skeleton and by the palette at two moments half a second apart, each vertex at a pixel of its own with its place as its colour, and holds the two within 1 cm at every vertex (2 mm in α2.2b, the view keeping a place to about 4 mm).
  - The cloud's run fails on any error Godot reports but the missing sound device.

**M1's benchmark,** one tap and about 20 minutes, with the phone unplugged, in flight mode, after it has cooled:
- the calendar alone at top speed; 10,000 markers at real speed and at top speed, with the camera touring, held speed read after 3 minutes; the same pinned to the middle cores; a sweep through the zoom stops' speeds; saves every 30 seconds with an export and a reopening; a still camera for the screen's own power;
- for each, its end state's digest against the cloud's, the share of frames on time, the slowest frame, the speed held, heat, battery and power, and memory, in one code.
- *Built in α1.5b:* the Bench page runs the seven scenarios of `kd::bench::scenarios()`, 17 minutes of them and about 19 in all, each on a page of its own with a world of its own under `user://bench`, apart from yours, and deleted after.
  - Each world takes its digest as it passes a set game second, and the saves scenario calls a camp home at an exact second: `kd::run::Marked` stops the world's batches there, which changes nothing but where they end.
    `kindling bench` runs the same worlds headless, and `tools/gamedata.py` writes their digests into the build, so the phone compares its own with the cloud's as it goes (`RES-05`).
  - A scenario's frames count after its first 5 seconds, with the crowd's own drawing time on the main thread; its heat, battery, memory, the fastest core's clock and the speed are read every 2 seconds, the speed over its last minute, after 3 minutes at top speed; and a world that has not reached its mark by the end runs on to it with the screen still.
    The sweep's world takes its digest at day 30, after its fastest speeds.
  - The pass lines, stated in the scenarios before the first run (`RES-09`): at least 97% of frames on time and the slowest at most 66 ms where the camera moves, a world reopened within 3 seconds, and every digest the cloud's.
  - The code (`kd::bench`): layout version 2 (version 1, the first build's, read its speeds earlier and marked the sweep at its start), 103 fields in 1054 bits with a CRC-24 (OpenPGP's), 218 letters of Crockford's base32 in groups of five; `kindling bench decode <code>` reads it and holds each measure to its line.
    Run a hundred times faster in the cloud, every scenario's world ends as the headless one's.

### A18.2 Risks

| Risk | What we know | If it fails |
|---|---|---|
| The PowerVR driver mishandles a feature we use | P1 to P3 drew outlines, mirrors, fire shadows and smoke without fault; the look you chose adds MSAA at full resolution, `textureGrad`, texture arrays, alpha to coverage, shading rates and bone reads, each probed in M2's first build (A4.7) | avoid that feature; the self-check reports the driver |
| The look you chose costs too much on the Mobile renderer (`RSK-30`) | The liked camp drawn plainly about 13–45 ms of the graphics chip, about 5–23 ms with every saving that does not show, against 8 (estimates) | A4.1's levers in order: our own build's two patches, the 3D at 0.75 where needed, half resolution last; the density never cut |
| A busy scene heats the phone | Open: the 20-minute heat run (H1) in M2 | the line comes down, or one planned, logged step under heat that passes your blind test (A5.5); the heat guard acts at the light level (A3.9) |
| Texture pixels shimmer, or a band's level pops | Simulated: the smooth-pixel filter flickers on 0.4–1.1% of pixels, 0.1% with a level made for the zoom | levels drawn calmer near the end of their range; the blend between bands retuned |
| Designed levels and small designs are too much work | GPT redrew a band's level in 42 seconds; about 100 small designs estimated | code levels for code-made textures; ground, rock and plants first; one level checked at each zoom stop |
| People stay hard to find in busy shade | Light only; light aids lifted people in sun and not in shade | movement and real light first; help drawn only for your eye if the phone shows the need, with your OK (A5.5) |
| Our own build of Godot is hard to keep | Not tried | the stock templates, with the 3D at 0.75 where a scene needs it |
| Discovery can't be tuned to its pace (`RSK-01`) | P4: tuning alone sets the pace | the discovery rules redesigned with you at M7 |
| Phone and cloud results differ (`RSK-04`) | P5: the same bits on your phone | our own function for whatever differs |
| A thousand minds miss a year a minute (`RSK-02`) | P6: 6.0 game years a real minute | profile and simplify scoring; the population target revisited with you (`MND-15`) |
| Making a world takes too long (`WLD-11`) | P7: about twenty times the room | fewer coarse candidates; settling while you choose |
| The zoom stalls | P8: one tree of ground, culled; your Measure still to come | a coarser middle level; coarse chunks made ahead |
| Nature's totals drift (`WLD-18`) | P9: within 0.66 and 1.10 for a century | damping in the predator and prey rules |
| Culture fails to emerge (`RSK-19`) | P10: from causes in most worlds | templates (`CUL-05`), with you |
| The director misses moments or overspends | P11: within budget, every named discovery caught | its recognisers and budget retuned |
| Gestures misread or text blurs | P12: none misread in the cloud; your verdict to come | gesture rules and scale retuned |
| The writer falls short (`RSK-08`) | Not built (P13) | pattern text stands alone (`PRE-37`) |
| Sound breaks up (`RSK-28`) | P14: 9% of the audio thread at worst in the cloud; your phone to come | fewer voices |
| Godot upgrades break things | each milestone | pinned versions, upgraded between milestones |
| Islands give a different answer, or too little parallel work | α1.3b: exact on every build and thread count; too costly for the crowd's 1.3 µs events, so it runs on one worker | one worker in key order, the same results; cheaper bounds and joins before minds need them (M6) |
| EnTT misbehaves on the phone | Built only in the cloud | flecs behind the same thin layer |
| The screen stays at 120 Hz | Read from Godot's source: set the frame cap again at run time | Android's frame-rate call through JavaClassWrapper; a small plug-in |
| History outgrows 4 GB (`PLT-10`) | Estimated at 3.5–4.1 GB for 7,000 people at Year 250 | a tighter encoding; what counts as an event, with you |
| Opening many small catalogue files is slow on the phone | Not measured | one file a kind, read by the same loader |
