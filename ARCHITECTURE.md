# Kindling: architecture

How Kindling is built: its parts, how they talk, the rules they keep, and why each choice was made.
It serves `PROJECT.md`, which says what the game must be, and is served by `IMPLEMENTATION.md`, which says in what order to build it.
The look aims at the pictures you chose (`art/targets/`).
What is not in `PROJECT.md`, this file or `IMPLEMENTATION.md` is not kept.
The code is the index: code names the `PROJECT.md` items it implements, so this file says how and why, never where (`CLAUDE.md`, rule 3).

## Status (8 October 2026)

- **Technology approved by you** (`PRC-03`): pinned Godot 4.7.2, stock templates, with the C++ simulation as a Godot plug-in (GDExtension).
  The 2D fixtures start on Compatibility and compare Mobile on two phones (A4.7).
  The earlier conditional 3D drawing patches leave M2's route.
- **Pre-production is closed** (5 October 2026): its prototypes' answers are written here as decisions, marked with the prototype that gave them (Pn); their code is deleted, and production writes its own.
- The foundations' sections (A2, A3, A17, A18) are written in full for M1; M1 is built and you accepted it on 6 October 2026.
- The graphics sections (A4, A5, A6 and A8) now carry the fully 2D design you approved on 8 October 2026, including the torus-preserving globe picture.
  M2 must still build and prove it; accepted M1 results are not 2D renderer results.
- Parts for later milestones are outlines, each designed in full when its milestone is next, from what the earlier ones taught.

## A1. Overview

### A1.1 What it must deliver

- **The look you chose, on your phone** (`PRE-01`, `PRE-02`, `VIS-14`): a fixed-camera 2D pixel-art world with rich light, crisp pixels and native controls, in portrait and landscape on your Pixel 11 Pro XL, also measured on a weaker phone.
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
  |  view/ (C++ plug-in): the bridge, projection, sprites, surfaces   |
  |        and bounded drawing caches on Godot's canvas               |
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
| Godot 4.7.2, stock templates | Pinned engine and existing extension; 2D fixtures replace the conditional 3D patch route |
| Compatibility first, with an identical Mobile comparison | Choose from measured look, drivers and sustained costs on both phones, not the old 3D assumptions |
| The simulation as a separate C++ library on its own threads | Thousands of minds need the processor's cores; C++ is Godot's official plug-in language on Android |
| EnTT entities with never-reused ids, content as data with no floats | Data laid out for the cache; new plants, animals, things and blueprints without code (`PRN-14`); history that names the dead forever |
| Events on one queue run in islands, keyed chance, correctly rounded maths, Box2D's rules for floating point | The same bits everywhere, at any speed and on any number of threads; long processes cost nothing until they end |
| A 540 × 1200 world image at 2×, native UI, neutral colour and custom receiver-aware light | Crisp pixels at the fixed camera, readable controls and changing light without runtime meshes |
| The feeling as guidance, a target card as an alarm, one approved picture per place relit for its hours; a saving kept only if it passes your blind test | Rules can't hold a feeling, and you judge the look; you turned down every visible saving |
| Three source art families, reviewed reductions, four facings and a small first animation set | Countable production; six fixtures prove the pipeline before expensive content grows |
| A generator in the order of real causes, many candidates scored | Believable worlds from a seed, tuned for the look |
| One orthographic projection, nearby origin, ordered surface pieces and shared map/globe overview | Huge zoom range and physical heights without changing exact torus coordinates or saves |
| Species as data, numbers by Damuth's law, individuals near people and counts far away | Nature that holds over centuries at any speed |
| Needs, energy, wounds and illness by real numbers; births by biology | Lives that land near foragers' real numbers |
| Choosing by utility with kept reasons, a small planner on top, knowledge per person | Explainable (`PRN-13`), reactive, cheap enough for thousands |
| Blueprints match characteristics, never names; every reality rule backed by an experiment | Discovery by the world's own rules (`PRN-07`) |
| Culture from causes; a naming language spelled with the font's letters | Nothing social scripted (`CUL-07`) |
| A story sifter that only sets speed and moments; pattern sentences; the phone's model only rewords, checked | The director never touches events (`TIM-03`); language models describe, never decide (`PRN-06`) |
| One column of panels, our own gesture reader, an integer-scaled pixel font | The world first, one thumb, crisp text in both orientations |
| Layered ambience, sounds made by code, Godot audio with world-space direction and distance, a voice manager | A lively camp from what is really there (`PRN-10`) |
| doctest and property tests, gdUnit4, pictures by Movie Maker mode, Perfetto on the phone | Every check runs in the cloud; the look is judged on the phone |

## A2. Code layout, builds and delivery

### A2.1 Repository layout

```
game/        the Godot project: scenes, the interface in GDScript, the theme and fonts, and its gdUnit4 tests;
             game/bin/ (the built extension) and game/data/ (a copy of data/) are made by the build, never committed
view/        C++ for the picture, as one Godot extension (libkindling): the bridge to the simulation (A3.8), the
             phone's telemetry (A3.9), one snapshot consumer, projection and ordered sprites/surfaces (A4, A6, A8)
sim/         the simulation in C++20, with no Godot: numbers, chance, time, entities, events, catalogues and saves,
             later the world, living things, people, minds, culture and history; its doctest tests; the `kindling`
             command-line tool for scenes, runs, benchmark worlds and catalogue checks; sim/thirdparty/ for
             vendored code
data/        catalogues, tuning files, scenes and benchmark worlds, as TOML (A3.6), in sources: base/ for the game,
             demo/ for what only the foundations show
android/     the release certificate; an Android plug-in only if the phone ever needs one (A3.9)
tools/       setup, checks, builds, delivery, the file check, signing
art/         the pictures you chose (targets); the textures: their levels, records, sheets, sources and
             requests (A5.4); sprite sources, frames, masks and optional offline rigs (A6.1); and the fixed views' golden pictures (A4.8)
dist/        the signed APK of the latest alpha and its note
```

### A2.2 Builds

- **C++:** CMake and Ninja, C++20, through ccache.
  - `sim/` is a static library with its tests and the `kindling` tool, and never includes Godot.
  - `view/` is one shared library, `libkindling`, linking `sim/` and godot-cpp 4.5 built from a trimmed profile (`view/build_profile.json`), which Godot 4.7 loads (P5).
    A class that a used method takes must be named in the profile, or the method silently vanishes.
    The 4.5 pin stays until a needed 2D API is absent; any upgrade is deliberate and tested (A4.7).
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
  - Stock export templates serve the 2D baseline; no leaf pre-pass, shading-rate patch or fractional-resolution fallback remains in M2.
    Configure Compatibility first and a measured Mobile variant (A4.7), with the world SubViewport separate from native UI (A4.1).
  - `quit_on_go_back` is off and `retain_data_on_uninstall` on (A3.7).
- **Art:** validated sprite/atlas metadata, aligned masks and normals, and reviewed complete reductions (A5.3, A5.4).
  Blender rigs are optional offline tools, never a required runtime kit export.
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
- Optional sprite editors and offline rig tools when the art pipeline needs them; no required Blender kit export.

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

- **Three classes for GDScript,** from `view/`: the world (make, open, save, close, export, import; commands; the goal and the frontier; counters and events), the crowd view (replaced by ordered 2D drawing in M2), and the device (cores, heat, telemetry).
  A few calls a frame, never one per walker: a call into the extension costs about 0.1–0.2 µs.
- **Commands in:** plain records, stamped with the game second they act at and written to the journal before they act; while you choose a power the game is paused, so a power acts on exactly the world shown.
  - *Known gap, from M1's review:* the demonstration's call home acts at the world's frontier, up to a quarter of a real second of the speed ahead of what you see, hours of game time at top speed. It is a test command, not a power; before the first power (M9), a tap pauses the world and acts at the moment shown (`WLD-13`).
- **Snapshots out:** after each batch, the simulation fills one slot of a triple buffer, so neither side ever waits and the screen always takes the newest: for each walker its id, kind and camp, and its ways from the screen's game time to the frontier.
  The accepted sampler places each walker along its own way at displayed game time.
  M2 replaces MultiMesh submission with A4.6's one-consumer controller and owned 2D drawing records; snapshot timing, commands and simulation publication stay unchanged.
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

The 2D design approved on 8 October 2026 replaces M2's runtime 3D route (`PRE-01`, `PRE-02`).
M1 remains accepted.
Its exact numbers, chance keys, calendar, catalogues, IDs, events, activities, workers, saves and proofs stay unchanged.
M1 built foundations and saved demonstrations, not generated terrain, ecology or a physical sky.
M2 uses labelled look fixtures until those later systems exist.
No new renderer or phone pass is claimed here.

### A4.1 The picture (`PRE-01`, `PLT-02`)

- **One world image:** a dedicated SubViewport, 540 × 1200 visible pixels at the reference portrait size, presented at exactly 2× with nearest filtering to 1080 × 2400.
  Landscape uses 1200 × 540 world pixels.
  The native root holds the world presentation rectangle and a native UI CanvasLayer above it.
  Text, touch targets, menus and accessibility scaling use native resolution.
  Root canvas_items stretch alone does not create the low-resolution world image.
- Ground, ordered objects, water, effects and world overlays share the world grid.
  Shadow targets feed their materials.
  Map and globe replace active world content; they do not run another simulation reader.
- At other window sizes, keep integer presentation scale and reveal or crop world pixels.
  Include safe insets and UI bars; never stretch fractionally to fill a new aspect ratio.
  The reference size is not a measured usable phone window.
- Keep camera position continuous, then rasterise at a resting art step.
  Small overscan permits a final offset in whole physical pixels for finer pans.
  Picking records and undoes that offset.
  Slow motion can still move in visible pixel steps; test it.
- Only a live pinch may resample the world rectangle around the fingers.
  On release, draw the nearest resting step at its intended density, with aligned anchors.
  No mip interpolation at rest and no routine resampling on pans.
  A8.4 owns the common projection.

### A4.2 Pixels and animation (`PRE-22`, `PRE-44`)

- Art families and reviewed intermediate levels are A5.3's.
  Sample at the intended grid with nearest filtering at rest.
  Sprites already painted in the projection are not squashed again.
- Project shared ground edges from world coordinates before rounding.
  Never accumulate a rounded tile pitch; slopes and objects use one raster grid.
- Stable pivots hold feet and attachments through animation and family changes.
  Walk frames follow sampled travel distance; work follows activity time and phase.
  A view-only idle cycle may use a stable ID seed.
- Flow, wind, smoke and glints may use visual approximations with stable phases and look keys.
  Their values never feed back into simulation rules.

### A4.3 Light (`PRE-20`, `PRE-21`, `PRE-30`)

- **One shared light function,** with neutral albedo, material masks and mild normals.
  A calibration shape proves the normals' basis before bulk art.
  Ground normals come from the same height field as the ground.
  Object normals convert their documented basis to the same world sun direction.
- The baseline is a CanvasItem shader and a custom sun-visibility mask:

    colour = albedo × (sky × sky_visibility
                      + sun × normal_response × sun_visibility)
             + local_fire_light + emission

  Material tuning supplies skin, hide, wood, stone, foliage and water responses.
  Keep contact darkening modest and do not count it twice at overlaps.
  A black shadow overlay on the finished picture would also darken fire and emission, so it is not the sun path.
- Sun, sky, moon, bounced light, lit edges, haze and small fire pools retain the targets' feeling.
  True midday is white-warm; gold belongs to low sun, shade to the sky.
  Tone and colour finishing are restrained, with a soft shoulder so bright snow and fire retain detail.
  Check night banding; debanding or HDR 2D is measured on both renderers before use.
- M2's hour and weather records are fixtures, not a physical sky.
  Later, the sun follows simulation latitude, season and hour.
  Once a day passes in under about 10 real seconds, hold relief light high and steady, with slow tint changes (`PRE-30`).
  Maps use the same speed rule, never permanent northwest light at ordinary speeds.
- Stock 2D lights are comparison tools and may serve small local effects.
  DirectionalLight2D's height affects normals, not terrain height in metres; its directional light ignores item cull masks.
  Stock extrusion and normals alone do not solve receiver heights.
  Check the pinned engine's actual canvas APIs and renderer behaviour in the fixture.

The target card starts from the previously chosen moments, then is refitted by owner verdicts (A5.5):

| Moment | Mean lightness | Share dark | Lights | Shade |
|---|---|---|---|---|
| Late afternoon | 0.55 | 32% | golden | neutral, slightly warm |
| True midday | 0.61 | 22% | warm | slightly blue |
| Dusk | 0.42 | 66% | warm and red | neutral |
| Night | 0.32–0.36 | 85–87% | warm pools | blue |
| Winter | 0.66 | 21% | pale gold | blue |
| Rain | 0.47 | 46% | grey | neutral |

A small hearth's light halves within about 2 m and falls to a tenth at about 3.25–4.25 m.
Sunlit snow is cream, shaded snow blue.

### A4.4 Shadows, overlap and reveal (`PRE-21`, `PRE-24`, `PRE-28`, `PRE-30`)

- **Approve flat shadows first:** person, tree, boulder and shelter, contact, direction, softness, long dusk shadow and fire.
  Build the terrain-aware prototype at the same time; its approval follows the flat style review.
- Logical caster proxies carry shape and height: crowns, trunks, bodies, boulders, roofs and walls.
  Trace toward the sun from each receiver's actual height.
  Slopes, cliff faces and shelter floors are receivers.
  Roof and floor need separate masks or explicit layers; a single top-height field cannot describe both.
- Start with CPU preparation on bounded tiles and a low-resolution mask aligned to the world grid.
  Cache static work by terrain, caster and sun revisions; add small moving-body shadows on visible receivers.
  Measure update frequency.
  Compatibility never relies on compute shaders.
- Keep useful contact, openness and horizon algorithms from the pilot.
  Replace their triangle/top-height input and flat-receiver contract.
  They are not a finished terrain shadow system.
  Include off-screen casters and a sun-angle-dependent halo: a 6 m tree casts about 22.4 m at 15° sun elevation.
  The pilot's 14 m reach is not a universal bound.
  Any capped low-sun approximation is a declared look limit, never the physical sunlight rule.
- Merge overlaps without multiplying them into black.
  Test softening over distance, long shadows across tile boundaries, wet ground, roof openings and a body partly in shade.
- A8.1's piece order handles height and overlap.
  Fade the crown or roof hiding the selected person, and reveal occupied shelter roofs.
  Keep physical obstruction, weather protection and shadow unless an explanatory interior layer deliberately replaces the shadow presentation.
  Other obscured band members get quiet silhouettes in the appropriate bands.
  Keep ordinary near figures free of heavy outlines (`PRE-21`).
  Drawing overlap determines reveal; personal knowledge does not hide simulated animals (`PRN-04`).

### A4.5 Water, fire and weather (`PRE-26`, `PRE-30`)

- Bed and water use one projection and height source.
  Depth is water height minus bed height: bed stones in shallows, darker depth, thin shore edges and directional flow marks.
  Wading feet and submerged parts meet that same surface.
  Refraction reads a prepared bed/background layer, never the already-composed foreground people.
- Start with depth, shore, contact and restrained glints, then measure a simple projected reflection layer.
  Reflections of sky and things above water remain required.
  A scrolling noise shader alone does not provide bed shape, waterfalls or reflections.
  The pilot's procedural river/bed functions remain fixture geometry; its second shader shape is replaced, so there is one source of truth.
- Fire light is separate from sun visibility.
  Flames, smoke and embers have depth bands and follow shelter openings.
  A near fire lights its doorway without leaking through walls.
- Rain and snow decorate the picture; later physical weather changes water, wetness and snow amounts.
  Wind, smoke and water movement may approximate looks, with stable seeds and phases across views.
  Seasonal shapes and cover follow actual state when supplied.
  GPU particle time never changes a rule or wets a cell.

### A4.6 The bridge, jobs and caches (`WLD-13`, `TIM-17`, `PLT-09`)

- Reuse the accepted C++ Godot connection, its saved-run lifecycle, activity sampler and pace controller.
  Do not build a second simulation in GDScript.
  One view controller consumes the single-consumer triple buffer once per display frame.
  Local view, map, globe and overlays read its owned copy.
  No asynchronous job retains a recycled buffer slot.
- The current crowd snapshot has walkers and walk histories only.
  Terrain, animals, weather, water and roofs need explicit additions when their source systems exist.
  Current Place and Activity positions are two-dimensional.
  The stage's stand-in east, north and up fields are centimetres; later physical heights are millimetres (A3.4).
  Convert units explicitly and add a surface/layer reference; never reinterpret an old field.
- Keep the stage's persistent-ID drawing copies and coalesced change ownership.
  New drawing records carry world epoch, displayed second, ID, position, surface reference, appearance key and activity phase.
  Terrain and water also carry revisions.
  Skipped snapshots require revision manifests with resynchronisation or a lossless invalidation feed.
  Disposable picture updates stay separate from lossless commands and greetings.
- Sample the correct activity at displayed time, including interrupted walks and torus seams.
  Generic interpolation between two recent positions would be wrong.
  Taps record the displayed time and ID; M2 does not solve the known command-frontier gap (A3.8).
- The simulation thread and workers own exact physical state, commands and safe snapshot publication.
  The existing save I/O thread owns journals, snapshots, recovery and archives.
  C++ view code samples, projects, culls, sorts and selects animation.
  Preparation jobs build CPU pixels, geometry and shadow inputs from owned immutable copies.
  Godot's main/render path applies canvas and resource changes, handles native controls and records timings.
  No live-world or arbitrary scene-tree access from view jobs.
- Begin with a small visible canvas set or a drawing list, not one independently processing Node per being.
  Batch atlas/material groups only where order allows.
  MultiMesh may help non-interleaving decorations or far marks; it does not cull individual instances or interleave separate trees with people.
- Art pages, ground, maps and sun masks have separate bounded caches.
  Jobs carry world epoch and revision; cancel stale work on world, zoom and terrain changes.
  Bound pending work and staging bytes.
  Keep a coarse parent visible until children are CPU-ready, GPU-uploaded and ready to show.
  Background loads are polled for readiness, never fetched early in a gesture callback.
  A ViewportTexture is live; a frozen transition needs a controlled copy, never an assumed frozen reference or GPU readback.
- A18.1 gives initial byte budgets.
  Count colour, normals, masks, gutters and complete mip chains, separately from targets and upload staging.
  Derive tile demand from the projected portrait footprint plus height, shadow halo and overscan.
  Four near tiles are not assumed to cover that footprint.
  Preload the six fixture pages; uploads use measured frame headroom, never an unmeasured fixed quota.
- Render caches live outside essential world archives, keyed by world ID, data fingerprints, renderer version and terrain revision.
  Deleting them must leave a world able to open unchanged.
  Sprite metadata is look data in A3.6's validation system; editor JSON is only a build input.
  A new physical surface, water or movement rule needs world/rules versions and migrations.
  Approximate view floats and GPU arithmetic never write rules state, and deterministic build flags stay in force.

### A4.7 Godot and phones (`PLT-04`)

- Pin Godot 4.7.2.
  Start on Compatibility, then export a Mobile comparison with identical fixtures and art.
  Record the actual renderer and driver; a fallback is not a Mobile result.
- Keep godot-cpp 4.5 until a needed API is missing; an older compatible minor is not by itself a reason to upgrade.
  Add needed canvas and texture classes and argument types to the reduced binding profile.
  Sources, registrations, bindings and pages change together.
- Probe canvas materials, aligned colour/normal/mask pages, targets and effects in both renderers.
  Keep feature alternatives small and explicit.
  For GPU particles, lowering amount_ratio does not lower configured processing cost; measure actual pools and amount.
- Reuse frame, heat, device and calibration reporting.
  A missing GPU timer or thermal reading is unavailable evidence, never zero time or a cool phone.
  Retain the existing heat governor, with pause and pace rules unchanged.
  Prefer cheaper visual passes and 30 fps before altering approved detail.
  The 30 fps mode keeps the same world grid.
  A further grid or visible seasonal/reveal change needs your decision.

### A4.8 Tests (`PRE-31`, `RES-05`)

- Golden pictures use frozen fixture time and a pinned cloud renderer; record which renderer produced them.
  Code-only changes are exact there; look changes show before/after pairs and FLIP differences.
  Old 3D goldens remain labelled history, not the new acceptance set.
  Phone GPU rounding may differ; the owner judges the phone picture.
- Script pans, slow walks, interrupted walks, pinches and orientation changes.
  At rest, pixels keep integer scale and frozen frames match.
  Measure explained motion before counting shimmer (`PRE-22`); test slow quantised motion by eye too.
- Fixtures include seams, slopes, front/rear high shelves, cliff joins, an occupied shelter, a tree over a selected person, water depth and wading, fire in sun shade, low sun from several directions and seasonal changes.
  Piece ordering crosses chunk boundaries, and picking undoes every presentation transform.
  The animal has a four/eight-facing comparison.
- Colour, material and object-ID pictures support the look checks and target card (A5.5).
  An ID buffer maps small encoded values through a CPU table, never a 64-bit ID in a GPU float.
  Cloud and phone statistics are written once in C++; FLIP remains outside exact simulation digests.
- Preserve ground accent and repeat checks where they describe the new art: initial accent line about 20, each resting band at least 90% of the near one; repeat correlation at most 0.2.
  Calibrate visual statistics against owner verdicts; the card warns and never approves.
  Readability starts with median person salience at about the 80th percentile, none below the 70th, then a timed find in busy scenes.
- At identical simulation seconds, identical commands give identical digests across views, camera positions, renderer variants and worker counts.
  Save/reopen, recovery and cache deletion preserve world state.
  Compare game seconds, not equal wall-clock runs with different zoom speeds.

## A5. The look

The feeling of the pictures you liked is the goal; the guidance holds starting numbers to calibrate against your verdicts, never rules for their own sake.

### A5.1 The pictures

- **The targets** (`art/targets/`): only the pictures you chose: the liked camp from above and at sunset, the look itself (`d2-pixel-paint`), the close camp and its hours (noon, night, the painted-over night), winter, the far views, backlight, rain, mist, a relit dusk, a camp of thirty, the first cave, the truthful village, the storm, finding people by light, and the guides for people, animals, poses, trees, made things, materials, small plants and the close camp's meadow.
  They show the feeling and are never shipped or traced: the engine is judged against their light, colour, density and composition, never their flagged things (sawn wood, metal tools, tipi-like cones, borrowed dress).

### A5.2 Guidance about the feeling

It replaces the art bible's rules.
1. **The feeling first.** When a guideline and your picture disagree, the picture wins, unless truth (`PRE-42`) or a principle says otherwise.
2. **Light true to the hour.** True midday at noon, gold only when the sun is low; shade takes the sky's colour, lights are warm, and hollows, corners and contacts darken.
   The fixture starts with sun behind the camera's left shoulder; changing sun direction is tested.
3. **Real darks.** In daylight about a quarter of the picture is dark (the liked camp 27%); nights about 85% dark, with firelight in a few small pools and warm colour on less than about a fifth of the picture; winter soft, with very dark under about 5%.
4. **Strong colour as accents,** except where the season itself is the colour: in summer daylight on no more than about an eighth of the picture (the liked camp 11%), carried by flowers, fire, dyes, ochre and beads; in autumn the wood's colour is the field.
   Strong colour means chroma above 0.12; GPT's daylight pictures drift to 13–18%, so your pictures stay the reference.
5. **Greens muted and warm,** never one green over most of the frame.
6. **A colour plan for each biome, hour and season,** from the true colours of its species by season (`WLD-31`).
7. **Quiet pixel texture:** marks of 2 to 3 texture pixels, low contrast within a material, never single-pixel speckle, each source family drawn as pixel art and its reductions reviewed.
8. **Density with a stage:** nature dense, airy and wild, and people on the quiet ground the world has made: trodden paths, working floors, snow, cave floors.
9. **People read first,** found by real light and movement, with the approved overlap reveal (A4.4): nothing in the world changes to make them stand out, and what they wear follows from their materials and their people's style (`CUL-12`).
10. **Edges by light:** no ordinary near outlines; quiet obscured-band silhouettes are the reveal exception; a bright edge where sun, sky or fire grazes a shape; contact darkening; small far figures alone are outlined.
11. **Life in motion:** wind, water, smoke, rain in its four layers, people and animals; nothing moves that the world does not move.
12. **Truth before beauty in content:** shapes, materials and dress from archaeology (`PRE-42`), each people's style generated (`CUL-12`), no real culture's motifs (`SCP-20`), every picture checked (A5.6).
- The smallest thing worth making is about a fist-sized stone; anything smaller is texture.

### A5.3 Three art families (`PRE-22`)

- Source density is in the internal world image: 64, 16 and 4 pixels a metre.
  At 2× presentation these appear at 128, 32 and 8 physical pixels a metre.
  They are art families, not navigation names (A8.3).
- Near art keeps faces, garments, equipment, wear and lit edges.
  Middle art keeps important shape with fewer marks; far art keeps crowns, shelters, rocks and groups readable.
  Review and repair intermediate reductions at 32, 8 and 2 pixels a metre and farther out: lost faces, thin tools, noisy patterns and broken edges.
  A reduction is a starting point, not final art.
- Neutral colour, material IDs/masks and mild normals are separate aligned pages.
  Match alpha, pivots and frame trim, with gutters around atlas entries.
  Keep numeric masks distinct from colour; never blend categorical IDs into invented materials.
  Renormalise and review reduced normals.
  CanvasTexture supplies colour, normal and specular inputs, not physical height or depth.
- Keep lossless source packing.
  GPU bytes include every resident page, normal, mask, gutter and reduction; a small PNG is not a small resident texture.
  Full mip chains add about a third to base storage.
  The checked texture loader expects complete halving chains, so 64 → 16 → 4 cannot be consecutive mip levels.
  Pack authored families separately with their reviewed complete intermediate chains; no automatically blurred family ladder.
- Ground uses quiet base variants, drawn material edges, broad wear/dampness stamps and ecology-selected details (A8.1).
  Shader blends are fixture experiments, retained only after owner review and measured cost on both phones.

### A5.4 Art production (`PRE-20`, `PRE-42`, `PRE-46`)

1. **Design:** start from a signed-off catalogue sheet and the chosen targets, at the fixed projection and each required size.
   Preserve original sheets, requests, prompts, source images, provenance and approval records.
2. **Make:** neutral pixel-art colour, matching material masks and mild normals, then frames, pivots, holds, facings and attachment points.
   Editor exports are build inputs, converted through the validated catalogue system.
   Optional offline rigs may help; runtime art remains 2D.
3. **Check:** Stone Age truth (A5.6), alpha/pivot alignment, scale, edge joins, frame stability, seasonal shapes and each family's reviewed reductions.
   Existing art with a painted sunny side is a labelled stopgap, not final neutral art.
   World-made geology, soot, wetness, snow and traces later come from their actual records.
4. **Review:** each piece beside its signed-off sheet, true size and enlarged, at noon and dusk, on slopes, by water and under shelter.
   Sheet approval does not approve a cleaned runtime texture, its normals or the engine result.
   Keep pending approvals separate; resolve them and missing seasonal silhouettes before bulk art.
5. **Ship:** checked lossless pages and metadata, digests in the look fingerprint, through existing asset packing and loading.
   Physical catalogue rules never move to a competing editor JSON catalogue.
   Texture changes remain look updates; real surface rules use normal versioning (A4.6).
6. **Gate production:** first approve a person, animal, tree, boulder, ground and shelter in the running engine.
   Prove height, overlap and shadows, then streaming, map/globe and two-phone performance before scaling the art catalogue.

The art lane works from the catalogue in batches, on its own branch, under the brief in `IMPLEMENTATION.md`.
GPT runs through the art tools outside the build; failed runs or usage limits only delay new sources.
The builder reviews each batch beside its sheets, and the owner says yes or no.
Shared colour measures stay in the one C++ look library, not a second Python copy.
Targets are never shipped or traced; sources carry provenance and their approval.
Original art and approvals survive the migration; only obsolete derived runtime exports are later removed.

### A5.5 Targets and the loop

- **Targets:** one anchor picture you approve for each place, either a repaint of an engine frame that holds the right content or GPT's own scene made from approved pictures; its other hours and seasons come from relights.
  - Six of the nine relit pictures you judged kept the feeling, and you chose a relit dusk over a repainted one; a target is never repainted again to chase, since repainting drifts darker and busier round after round.
  - From each target the builder takes each material's colour, spread and density through the engine's material masks, and the moment's numbers for the card; the light's direction comes from declared M2 fixture records, then the physical sky when it exists.
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
  1. **draw:** one Godot run draws the fixed views at the reference 1080 × 2400 presentation, with the colour, object and material pictures, lossless frames of the scripted paths, and many-sample pictures of small patches;
  2. **check:** A4.8's checks and the card's alarms;
  3. **look:** changed views beside their last approved versions, whole and in enlarged crops, on a lettered grid, so a fault is "C7";
  4. **judge:** a fresh subagent sees the pictures, the targets and a yes-or-no checklist for each Done when and truth question, and says which of each pair is closer to the target, asked twice with the order swapped; it reports faults, never approval;
  5. **show** you what changed, in pairs, with the alpha;
  6. **ask** only when the look has an open question.
- **Savings must be invisible** (`PRE-01`): first the machine line (A4.8), then your blind test on the phone, ten random pairs asking "which is sharper?", where eight or more right means it shows (guessing gets there about 5% of the time).
  On the Compare page each pair is one view drawn two ways, in random order; the result code holds the seed and answers.
  The new tests keep the approved world grid and vary measured light, shadow or drawing methods.
  Earlier MSAA and mesh-fire comparisons remain 3D proof history, not 2D approvals.
- **The heat step:** if the 20-minute heat run shows the picture alone heats the phone, one planned, logged step under heat, such as distant fires casting no shadows, chosen among the savings that pass your blind test, as you chose on 6 October 2026; if none is enough, it comes back to you.
- **The AI judge advises, never decides:** it reports faults, never approval.
- **Your choices:** two to four labelled pictures with a one-line cost each, in batches of four, with builds you open anyway; free of known faults; never offering a visible trade-off.
  A gallery for a setting with many values spreads wide first, then narrows, made by the engine or by code changing one setting only.
  Every choice you make is written into the section it decides.
- **Reveal approved on 8 October 2026:** fade covering crowns/roofs and show quiet obscured-band silhouettes (A4.4).
  Further visual aids need their own decision; the fixture tests the approved behaviour now.

### A5.6 Truth in pictures (`PRE-42`, `SCP-20`)

- GPT draws later or borrowed things and ignores "avoid" lines, such as metal tools after "no metal".
- So every target, guide and source picture is checked before anyone aims at it or prepares it:
  1. ask truthfully, with the prompt's truth lines;
  2. look at every made thing, animal and garment at twice size, against the known slips: metal before copper, sawn wood, later things (chickens, hooped buckets, winches, lattice windows, glass-bead colours, rucksacks, slatted sleds, maize, a pot hung over a fire, boats with seats), spotted or long-maned horses, striped piglets outside spring, tipi-like cones, Lascaux-like paintings, real cultures' motifs, fur bikinis, grass rain capes, and anything countable in a ground texture;
  3. date anything doubtful against a first-hand source, and write the verdict beside the picture;
  4. keep the picture's feeling, not its mistakes: a target is approved for its light, colour, density and composition, never for its flagged things;
  5. never take motifs from real cultures' art or dress (`SCP-20`).
- Truth costs no feeling: the truthful village and the first cave kept it.

## A6. The art kit and animation

### A6.1 The kit (`PRE-46`)

- Replace runtime Blender parts with a countable catalogue of sprite bodies, garments, overlays, attachments, actions, plants, rocks, shelters and things.
  Signed-off sheets remain the design standard.
  The six fixtures prove the engine pipeline; they are not all launch art.
- Keep about eight plant forms, six animal body patterns and eight garment kinds, in child and adult sizes.
  Species supply proportions, seasonal shapes, stages and material colours; approved family sheets supply age proportions.
  Bare winter branches are separate shapes where the species needs them.
- Shelters keep their archaeological basis: hides closing rock shelters, skin tents on stone rings, post huts, mammoth-bone circles only where mammoths live, brush huts and later longhouses.
  Each records its excavation and what is reconstruction.
  No present-day smoke flaps or borrowed cultural detail (`SCP-20`).
- Bodies show build, age, injuries, dress and feelings as `PRE-27` requires.
  Faces retain their small designs for feelings, calm, sleep, hurt, death and cold.
  Small plants, tools, faces, flowers and berries retain readable marks at each art step.

### A6.2 Appearance (`PRE-42`, `PRE-43`)

- Materials, equipment and wear follow actual records, never invented kit.
  A few aligned body, clothing and tool overlays replace hundreds of precombined sheets.
  Attachments share pivots, facings and trim metadata.
- Stable per-being look keys choose approved build, wear and pattern variations.
  Cosmetic chance uses a separate purpose in A3.5's key; it never consumes a physical system's sequence.
  Made things show their inputs, amounts and people's styles within their design limits.
  Motifs and icons derive from the same approved designs.

### A6.3 Movement (`PRE-27`, `PRE-44`)

- Start with four world facings, a six-frame walk, idle, carrying, cutting and tending fire, and a small clothing set.
  Test eight facings on one animal first: readability, turns, file size, production time and phone cost.
  Do not expand every creature before that comparison is reviewed.
- C++ view code selects frames from A4.6's activity sampler.
  Walking follows distance, work follows phase, and idle may use a stable seed offset.
  At fast time keep the activity readable at a steady visual pace.
  Frame holds and pivots are data; feet do not slide through a frame change.
- Later milestones add the full movement list, body-condition variants, feelings, social gestures and dances.
  A six-frame walk does not fix the frame count of every action.
  Shared dance beats remain shared, while ordinary crowd timing varies.
- Runtime bones, skinning and palettes are replaced by sprite frames and aligned attachments.
  Optional offline 3D rigs may generate frames or normal references, followed by pixel repair.
  They are not runtime requirements.

### A6.4 Surfaces and picking (`PRE-24`, `PRE-33`)

- M2's fixture provides a floor height field, cliff segments and a simple shelter.
  Slope patches use A8.4's projection; cliff tops and faces are separate pieces.
  Feet and contact sit on the sampled walk surface.
  Physical paths remain authoritative; adding real movement heights and layers belongs to later simulation milestones.
- Deep caves have separate surfaces and entrance transitions.
  Arbitrary stacked overhangs wait; keep the data extensible without building a general 3D visibility engine now.
  A shallow cutaway does not fulfil the geological slice (`PRE-25`), which stays in M3 and later archaeology work.
- Picking undoes UI layout, integer presentation scale, live pinch and residual camera offset.
  Intersect the drawn surfaces in visible order, then object bounds and alpha masks, returning persistent IDs.
  Flat inverse projection alone cannot pick slopes or floors beneath roofs.

### A6.5 Sheets and review (`PRE-31`)

Sheets show the same design at true size and enlarged in each family and intermediate step, in two materials where applicable.
Filmstrips show movement and stable feet; normals and material masks can be inspected beside colour.
Phone previews show noon, dusk, slopes, water, shelter and both orientations.
Reference approval, runtime texture approval and engine approval remain distinct records (A5.4).

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

### A8.1 Ground, height and order (`PRE-03`, `PRE-23`, `PRE-24`)

- M2 uses labelled river/bed and look-field fixtures.
  M3 supplies height, geology, soil, drainage, vegetation and resource abundance.
  Tile colour never invents a biome, river or harvestable plant.
- Four ground layers: quiet material variants, drawn transition edges, broad wear/dampness/leaf-litter stamps, and small ecology-selected details.
  Decorative grass need not be an entity; a harvestable plant must correspond to one.
  Zoom may aggregate decoration, but inspection preserves real resources and objects.
- Neighbour-aware transitions need a halo and wrapped neighbours in both axes.
  Material, edge and detail choices use stable world-coordinate look keys, never cache size or load order.
- Render pieces are disposable, separate from the planned 256 m simulation areas.
  Start with shared base tiles and 8–16 m near pieces, measured against the portrait footprint and shadow halo.
  A unique 16 m square at 64 px/m already costs 4 MiB for colour alone; shared materials and bounded caches are essential.
  TileMapLayer is useful only where regular ground helps; its signed 16-bit saved cells are not a world coordinate store.
  Keep world coordinates in C++ and rebase a finite visible layer.
- Floor height fields, cliff faces and shelter floor/back/roof/opening records describe the approved terrain.
  These measured surfaces do not require Godot 3D cameras, meshes or physics.
  Normals, feet, water and contact share those records and A8.4's projection.
- Flat actors can sort by projected ground depth.
  Large occluders split into trunk/crown, cliff cap/face and shelter back/floor/roof/front pieces.
  Local projected overlap and surfaces create before/after relationships, with persistent-ID ties for unrelated equal-depth pieces.
  Order crosses render-chunk boundaries.
  Neither ordinary Y-sort nor drawing all elevated objects last handles a foreground tree over a rear shelf.
- Point depth, cos(37°) × south + sin(37°) × height, helps points, not whole irregular sprites.
  Start with split shapes that admit an acyclic order.
  If a fixture cannot be ordered, change its split.
  A per-pixel depth compositor is larger fallback work, raised before broadening terrain complexity.

### A8.2 A nearby origin

Subtract an exact nearby camera origin before converting world coordinates to drawing floats.
Horizontal differences use A3.4's torus shortest offsets; simulation north is negated into drawing south.
The simulation and saves never depend on the drawing origin or renderer arithmetic.

### A8.3 Zoom steps and forms (`PRE-03`, `PRE-28`)

At the reference portrait width, metres across are 1080 divided by physical pixels per metre.
These are calculated starting snap steps, tuned in data after phone review:

| Physical pixels/metre | Width | Navigation |
|---|---|---|
| 128 | 8.44 m | person, near source at 2× |
| 64 | 16.88 m | intermediate near |
| 32 | 33.75 m | close camp, middle source at 2× |
| 16 | 67.5 m | intermediate local |
| 8 | 135 m | wider local, far source at 2× |
| 4 | 270 m | camp, reviewed 2 px/m art at 2× |
| 2 / 1 / 0.5 | 540 m / 1.08 km / 2.16 km | landscape and map transitions |
| 0.125 | 8.64 km | valley |
| 0.015625 / 0.0078125 | 69.12 / 138.24 km | region |
| 0.00048828125 | 2211.84 km | whole 2000 km world with margin |

Keep intermediate powers of two without giving each a new mode name.
Source families are not navigation stops; 32 physical px/m is close camp, not camp.
Cull the projected north-south footprint, not a square equal to the screen width.
Near views show individuals, wider ones designed tiny figures, groups and camps; valley and region show real terrain, drainage, vegetation and settlement aggregates.
These aggregates await the generator and living-world systems, not M1's crowd packet.
Selection, focus and time survive each change of form.
Coarse parents remain until detail is ready, with no black holes or synchronous terrain work in a pinch (A4.6).

### A8.4 One projection (`PRE-02`, `PRE-33`)

The camera faces north at exactly 37° above the horizon and never turns.
East goes right; south comes toward it.
Let X be local east, Y local south and Z height in metres, after A8.2's origin conversion.
With a = sin(37°), b = cos(37°) and s internal pixels a metre:

    screen_x = centre_x + s × X
    screen_y = centre_y + s × (a × Y − b × Z)

Subtract the projected focus too.
One tested C++ helper owns projection, bounds, shadow endpoints and inverse picking; asset tools follow the same specification.
At 64 px/m, one north-south ground metre projects to about 38.516 pixels, one vertical metre to 51.113.
A 64 × 64 ground square is not a 64 × 64 projected diamond; never repeat a rounded 39-pixel pitch.
Replace the perspective rig's lens, turn and 1.25× zoom maths; keep useful input state and the nearby-origin idea.
UI hit testing comes before world gestures; pan, pinch and one-thumb controls share one controller.
A4.1 owns presentation and A6.4 surface-aware picking.

### A8.5 World map and globe (`PRE-29`, `WLD-01`, `WLD-02`, `WLD-03`)

- **Confirmed by you on 8 October 2026:** retain the 2000 × 1000 km torus, its coordinates, distances and saves.
  Local camera turning is removed; the overhead map and 2D globe disc are pictures drawn from one shared overview.
  Polar land compresses only in the globe picture.
  There is no sphere physics, great-circle routing or save migration.
- The generated 200 km permanent polar ice barrier and its crossing rules remain world-system work.
  Wrapping is built; that barrier is not yet generated.
  Overview geography in M2 is a labelled fixture until M3 supplies real cells.
- Normalise east/north coordinates as u = east / width and v = north / height.
  Longitude is 2π × (u − 0.5); latitude is π × (v − 0.5).
  A disc shader maps these angles around a chosen display meridian onto a unit sphere and shows only the front hemisphere.
  No runtime 3D globe scene is needed.
- Use the 2:1 overview as an equirectangular texture: repeat longitude, clamp latitude.
  Top and bottom rows are different sides of the torus seam; never blend them around a pole.
  Ice hides the poles; filtering must not draw warm land across the ice edge.
  At latitude 60° the displayed east-west width halves; true metres do not.
  Any scale uses model distances and explains the display distortion.
- Markers use the same forward mapping; taps invert the visible disc to longitude/latitude, then wrapped coordinates.
  Ignore the hidden hemisphere and keep the selected place through transitions.
  The globe may centre another longitude without turning local art.
- Map relief, vegetation, rivers, sea depth, clouds and shadows come from the shared overview and later world state.
  At ordinary speeds light follows hour and season; at very fast rates A4.3's stable relief rule applies.
  Rivers keep `PRE-26`'s minimum visible widths.
- A 2000 × 1000 RGBA8 overview is about 7.63 MiB, a padded 2048 × 1024 one 8 MiB, or 10.67 MiB with a full mip chain.
  Hold about 8–11 MiB for colour inside the map cache (A18.1); count relief and weather channels too.
  Measure the disc's incremental GPU time, with an initial goal under 1 ms, not a claimed result.
  Test seam/pole appearance, markers, inverse taps and zoom transitions.

### A8.6 Time by zoom (`TIM-01`, `TIM-04`, `TIM-15`)

Keep the existing data-driven person, close-camp, camp, valley and region speeds and pace controller.
Zoom may change requested time speed, never simulation detail or unseen minds.
World and globe speed use sustainable simulation capacity.
Leave the full future priority order possible: pause, skip, manual dial/lock, story director, then zoom.
The director and intervention controls remain later work.
Determinism is checked at equal simulation seconds, not equal real durations (A4.8).

### A8.7 Budget

Measure each zoom and cold/warm transition on both phones (`PLT-04`).
A18.1 keeps the frame, heat, memory and opening lines.
The camp in dense woodland with water, fire/smoke and weather retains the conservative 6 ms mean GPU line; a six-object fixture alone does not prove it.

## A9. Living things, outline

- Each species is a catalogue entry: climate and soil ranges, seasons, size, diet, group size, yields, life cycle, art-kit form and colours (`WLD-31`, `WLD-32`).
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
- Godot audio uses world-space direction and distance relative to the fixed view, with low-pass for distance and cave reverb; muffling by land comes from the simulation's line test (`SND-08`).
  The 2D picture does not flatten sound distances or require a runtime 3D camera.
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
    These are prototype results, not proof of the new 2D art, surfaces or shadows; M2 measures those afresh.
- **The 2D graphics engine:** planning limits, not phone results.
  State every scene's pass lines before its first run (`RES-09`).

| Measure | Initial line |
|---|---|
| 60 fps | 16.67 ms frame interval; GPU mean at most 8 ms, p95 at most 9.5 ms |
| Main thread | mean at most 8 ms, p99 at most 12 ms |
| Frame delivery | at least 97% on time, slowest frame at most 66.7 ms; report the full distribution |
| Busy woodland camp | conservative GPU mean at most 6 ms, with an explicitly 2D fixture |
| Draws | initial diagnostic ceiling 300; distinguish canvas batches, native UI and off-screen passes |
| 30 fps mode | 33.33 ms interval, same world grid and readable native controls |
| Sustained heat route | 20 minutes; whole-phone mean at most 4 W; battery at most 40 °C at the end; thermal status none; each minute meets frame delivery |
| Heat headroom | 10-second forecast below the light threshold minus 0.05 where readings exist |
| Application memory | at most 1 GiB |
| Opening | textures prepared within 10 s after install; warm world open within 3 s |
| Streaming | bounded jobs and bytes; decode and uploads inside the frame budget; no gesture-time generation |

  - Initial sampled-texture proposal: 128 MiB, split as 48 MiB maps, 48 MiB sprite/material pages, 16 MiB generated ground and 16 MiB masks.
    The globe colour overview's 8–11 MiB is inside maps; extra relief/weather channels are counted there too.
    These partitions are starting budgets to measure, not new owner limits.
  - Count render targets separately: one 540 × 1200 RGBA8 target is about 2.47 MiB, two about 4.94 MiB before extra formats.
    Initially reserve up to 32 MiB for targets and 4 MiB for bounded CPU upload staging; measure actual formats and peak overlap.
    Shared base art and A8.1's small pieces keep near ground bounded.
  - Prepare used shader/material variants before drawing; later compilation stalls are faults to fix.
    Record CPU, GPU, resident bytes, upload/decode costs, frame distribution and measurement method.
    No average alone proves frame pacing or thermal stability.
  - Compare Compatibility and Mobile at 60 and 30 fps on the owner's Pixel 11 Pro XL and one named weaker phone.
    Record model, RAM, display mode, OS, renderer, driver and thermal readings.
    Missing counters are unavailable evidence.
    Prefer cheaper visual passes and 30 fps before changing approved detail; simulation density is never reduced.
  - Preserve the seven-person day scene and three-fire night comparison, the thirty-person camp in dense woodland with water/fire/smoke/weather, a separate crowd stress case, map streaming and repeated pinches.
    Use approved initial actions, then expand the activity review with the catalogue.
    State counts and visible area; six objects alone are not a busy camp.
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

- **Calibration and proof history:** reuse the existing data-driven runner, declared lines, switches, timing windows and checked result codes.
  Add explicitly named 2D variants and counters; retain decoding of historical 3D results against their original definitions.
  Never reinterpret old triangle, skeleton or MSAA variants as sprite results.
  Cloud runs prove fixtures draw and counts match; the phone supplies performance evidence.
  Periodic targets count their last draw cost only for the frames they update.
  Each disabled-pass comparison includes every active viewport, including native UI.

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
| Height ordering or picking fails | Ordinary Y-sort cannot solve roofs, cliffs and foreground trees over rear shelves | change piece splits and test across chunks; raise the scope of a depth compositor before expanding terrain |
| Receiver-aware shadows cost too much or miss layers | Pilot light maps use highest surfaces and flat receivers | prove slope, cliff and shelter-floor receivers early; change masks before bulk art |
| Pixels step or a family change jumps | One raster grid and discrete resting steps are planned, not yet phone-proved | tune pivots, steps and residual whole-pixel pans; review slow movement and pinch release |
| The 2D look costs too much (`RSK-30`) | No sustained result for the new renderer | measure passes on both phones, cheaper visual work then 30 fps; keep the approved grid and density |
| A busy scene heats the phone | M1's heat results do not prove M2 graphics | the 20-minute route sets the line; existing heat guard and one approved logged visual step (A3.9, A5.5) |
| Art, masks and seasonal shapes exceed production capacity | Signed-off sheets exist, runtime approval is separate | six fixtures first, repair reductions and missing seasonal forms before bulk work |
| Cache jobs outlive a world or revision | The bridge has one consumer and may skip snapshots | owned immutable inputs, epochs/revisions, resynchronisation and bounded queues |
| The driver mishandles a 2D feature | Old 3D probes are not proof of both renderer paths | explicit fixtures on Compatibility and Mobile; record fallback and missing evidence |
| People stay hard to find | Moving light and overlap reveal are approved | test busy woods, camp and night; further aids come to the owner |
| Globe seam or distortion misleads | The torus-to-disc mapping is display only | repeat longitude, clamp latitude, hide poles in ice; inverse-tap and model-distance tests |
| Discovery can't be tuned to its pace (`RSK-01`) | P4: tuning alone sets the pace | the discovery rules redesigned with you at M7 |
| Phone and cloud results differ (`RSK-04`) | P5: the same bits on your phone | our own function for whatever differs |
| A thousand minds miss a year a minute (`RSK-02`) | P6: 6.0 game years a real minute | profile and simplify scoring; the population target revisited with you (`MND-15`) |
| Making a world takes too long (`WLD-11`) | P7: about twenty times the room | fewer coarse candidates; settling while you choose |
| Nature's totals drift (`WLD-18`) | P9: within 0.66 and 1.10 for a century | damping in the predator and prey rules |
| Culture fails to emerge (`RSK-19`) | P10: from causes in most worlds | templates (`CUL-05`), with you |
| The director misses moments or overspends | P11: within budget, every named discovery caught | its recognisers and budget retuned |
| The writer falls short (`RSK-08`) | Not built (P13) | pattern text stands alone (`PRE-37`) |
| Sound breaks up (`RSK-28`) | P14: 9% of the audio thread at worst in the cloud; your phone to come | fewer voices |
| Godot upgrades break things | each milestone | pinned versions, upgraded between milestones |
| Islands give a different answer, or too little parallel work | α1.3b: exact on every build and thread count; too costly for the crowd's 1.3 µs events, so it runs on one worker | one worker in key order, the same results; cheaper bounds and joins before minds need them (M6) |
| EnTT misbehaves on the phone | Built only in the cloud | flecs behind the same thin layer |
| The screen stays at 120 Hz | Read from Godot's source: set the frame cap again at run time | Android's frame-rate call through JavaClassWrapper; a small plug-in |
| History outgrows 4 GB (`PLT-10`) | Estimated at 3.5–4.1 GB for 7,000 people at Year 250 | a tighter encoding; what counts as an event, with you |
| Opening many small catalogue files is slow on the phone | Not measured | one file a kind, read by the same loader |
