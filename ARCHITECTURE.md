# Kindling: architecture

How Kindling is built: its parts, how they talk, the rules they keep, and why each choice was made.
It serves `PROJECT.md`, which says what the game must be, and is served by `IMPLEMENTATION.md`, which says in what order to build it.
Every choice cites the research note behind it (`research/NN-*.md`), where the sources are, and the look follows the art book (`art/book/`).
The code is the index: code names the `PROJECT.md` items it implements, so this file says how and why, never where (`CLAUDE.md`, rule 3).

## Status (5 October 2026)

- Rewritten for your OK from the research redone on 4 October (research 00 to 17) and the art book you accepted as the starting point for the look.
  The Rust architecture stays in git history at commit `ebaeae3`, and the first Godot version of this file at `f881525`.
- **Technology approved by you** (`PRC-03`): Godot 4.7, its source unchanged, with the simulation in C++ as a Godot plug-in (GDExtension) (research 01).
- **Pre-production is closed** (5 October 2026): its prototypes' answers are written here as decisions, marked with the prototype that gave them (Pn), and the evidence, numbers and lessons are in `LESSONS.md`.
  Their code is deleted; production writes its own.
- The foundations' sections (A2, A3, A17, A18) are written in full for M1, from research 18 (5 October 2026).
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
| EnTT entities with never-reused ids, content as data with no floats | Data laid out for the cache; new plants, animals, things and blueprints without code (`PRN-14`); history that names the dead forever | 03, 18 |
| Events on one queue run in islands, keyed chance, correctly rounded maths, Box2D's rules for floating point | The same bits everywhere, at any speed and on any number of threads; long processes cost nothing until they end | 03, 18 |
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
art/         the art book and its painter (A5.1), and the pixel fonts
research/    the research notes this file cites
dist/        the signed APK of the latest alpha and its note
```

### A2.2 Builds

- **C++:** CMake and Ninja, C++20, through ccache.
  - `sim/` is a static library with its tests and the `kindling` tool, and never includes Godot.
  - `view/` is one shared library, `libkindling`, linking `sim/` and godot-cpp 4.5 built from a trimmed profile (`view/build_profile.json`), which Godot 4.7 loads (P5).
    A class that a used method takes must be named in the profile, or the method silently vanishes (`LESSONS.md`).
    godot-cpp 10, which targets Godot 4.7's own interface, is offered after M1 (research 18).
- **Flags for all our C++** (A3.4): `-std=c++20 -O2 -Wall -Wextra -Werror -fno-exceptions -fno-fast-math -fno-math-errno -ffp-contract=off`, with `-ffp-contract=off` the last floating-point flag on the line; symbols hidden; `-ffunction-sections -fdata-sections` linked with `--gc-sections`.
  Nothing throws: errors are values.
- **Five builds** (research 18):
  - the cloud's main build: x86-64 with clang 18, for the tests, the tool and the extension the Godot tests load;
  - a second compiler: x86-64 with GCC 13 and its undefined-behaviour and float-cast checks;
  - arm64 with GCC 13, and arm64 with the phone's own compiler (NDK r30, clang 21), both static executables run under qemu, for the same-bits check (A3.4);
  - the phone's: the extension for arm64 Android, API 24, the C++ runtime linked statically, newer Android functions linked weakly and guarded, 16 KB-aligned, stripped in the APK and kept whole for crash symbols.
- **Godot:** 4.7.2, pinned, exported from the command line (`--headless --export-release`).
  - The export is unsigned; `zipalign` and `apksigner` finish it, so only `tools/signing-key.py` reads the secret.
  - The build first copies `data/` into `game/data/` with `build.toml`, the list of its files and their digests (A3.6); the export's filter includes `data/*.toml`, since Godot skips text files otherwise.
  - Android export needs ETC2 and ASTC texture imports on, and the preset leaves out `addons/` and `test/`, so the test framework never reaches the phone; it asks for no permissions.
  - `quit_on_go_back` is off and `retain_data_on_uninstall` on (A3.7).
- **Targets:** Android arm64 for the phone, and Linux x86-64 for tests and pictures in the cloud.
  There is no web build: Godot's is about 40 MB, beyond the private page's 15 MB (research 01).

### A2.3 Delivery of each alpha (`PRC-11`, `PLT-06`)

- **The APK:**
  - signed with the release key, derived from the passphrase secret by `tools/signing-key.py`, the only script that reads it;
  - package `dev.kindling.app`, so each alpha installs over the last;
  - committed to `dist/` on the work branch and linked from the note.
- **The note** says what is new, what to try, what is rough, the items delivered and the links, and is published at your note link with pictures from the cloud.
- **Version code:** (milestone + 1) × 10000 + alpha × 100 + step (a = 1), so α1.1a is 20101 and α1.2b is 20202.
  Each is above every earlier build's, so it installs over it.
- **Self-check:** the first start of each version runs a few seconds of checks and shows them, with a short code to send if anything fails (research 02, 18):
  - the same bits: seeded runs of the numbers, chance and a small world, each digest against the one the cloud wrote into the build;
  - the floating-point environment a simulation thread finds;
  - the catalogues' digests against the build's;
  - a save written and reopened;
  - the graphics driver's version, read from the id of Godot's pipeline cache, the only place Godot gives it; the screen's refresh rate; the cores and their top clocks; the heat thresholds; and how the storage is mounted.

### A2.4 A fresh cloud session

`tools/setup.sh` installs whatever is missing, pinned and checked by checksum, and says nothing when all is present:
- Godot and its export templates;
- the Android SDK, NDK and JDK;
- CMake and Ninja, clang-format and clang-tidy, GCC 13 and its arm64 cross compiler, qemu, MPFR (the maths library's test oracle), and the formatters and linters of GDScript (gdtoolkit) and Python (ruff);
- gdUnit4, doctest, godot-cpp 4.5, EnTT 4.0.0, toml++, xxHash and zstd;
- Mesa's software Vulkan driver (lavapipe) and Xvfb, so Godot can draw pictures without a graphics chip (research 16).

CORE-MATH's few C files are kept in `sim/thirdparty/` at a pinned commit, with a sample of its hard cases, since its host is the one source a session might not reach; `tools/core-math.py` copies them from a checkout of that commit.

## A3. The simulation core (research 03, 18)

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

- **EnTT 4.0.0** holds people, animals, plants near people, things, places, groups and records, behind a thin layer (`sim/ecs`), so no rule creates or destroys entities itself and a later change of library stays local (research 18).
  - Two registries keep memory tight: beings (people, animals, places, groups) with 32-bit handles, and things with 64-bit handles, room for more than a million at once.
  - Plants and far animals stay as counts on world cells, outside EnTT (`WLD-32`).
- **Every entity has an id that is never reused:** 64 bits from one world counter, its top four bits naming its family.
  Components, events, history, memories and saves hold only these ids; EnTT's own handles live within one step and are never stored.
  The dead leave their entity but keep their record in the history (`PRN-15`).
- **A thing's kind is its catalogue entry,** and its parts are the components the entry lists, as RimWorld's Defs and Comps; each entry becomes a ready recipe at load.
  There is no class hierarchy of kinds, which Dwarf Fortress regretted.
- **One descriptor per component:** its stable name, its version and its fields (name, type, unit, range, whether it names another entity or entry, what it affects, a plain description).
  The same descriptor loads its values from the catalogues, saves and loads it, hashes it for the checksum and shows it in the details view (`PRN-14`).
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
- **The queue** is a binary heap of the keys (research 18: 120–230 ns an event, 1–4% of a core at the speed targets); a two-tier one of minute buckets and a heap replaces it only if a profile shows the queue above about 5%, and the checksum test proves the switch changes nothing.
- **Cancelling is lazy:** the owner keeps the sequence it expects for each slot (its activity's end, each timer); interrupting clears it; a popped event whose sequence no longer matches is skipped.
  The heap is rebuilt without its dead entries when they pass a quarter of the live ones, which no outcome can see.
  A save holds only the live events, sorted.
- **Activities** have a start, an end and a way (where the doer is at any moment, from its start, end and pace), so they can be seen, met or attacked on the way (`TIM-17`).
  An interruption ends one early by the kind's own rule of what it keeps: a walker stands where they got to, what builds up gives its share, work stays in the thing, a single act does nothing.
- **Events happen one at a time in key order, and that one-thread run is the reference.** Every faster way must give the same bits (research 18):
  1. Game time is cut into windows on a fixed grid, also cut at the world's layer events and wherever the simulation stops; since cuts cannot change results, cutting is free.
  2. At each window's start, owners that could touch each other or the same thing within the window join one island: those within twice the longest reach plus twice the fastest pace times the window, or sharing a store, a household, a thing or a shared activity.
  3. Each island takes its events for the window from the queue and runs them in key order on one worker; events it makes inside the window stay in it, later ones go out to the queue.
  4. Islands run side by side on up to four workers, then merge their events and history by key.
  - Since no island can read what another writes within the window, the result equals the one-thread run for any window, thread count, speed or pause; a test proves it, and a debug build logs any touch across islands.
  - New ids for things made inside an island are handed out in key order at the merge, so they never depend on other islands.
  - Up to about camp speed, one worker runs events in order, with the same results.
- **So a long process costs nothing until it ends,** and the cost of a game day follows what happens in it, not the speed.
- *Measured in pre-production* (P6): a thousand people at 6.0 game years a real minute on your phone's four cores, with decisions in 5-minute windows read from a snapshot; production replaces those windows with islands, which give the same history at any speed (research 18).

### A3.4 The same bits everywhere (`RES-05`, `TIM-16`)

Following Box2D, Factorio and research 18:
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
- **Banned in `sim/`, each with its replacement in `sim/num`:** `long double`, `float` in state, plain `char` arithmetic, platform maths, `fmin` and `fmax`, casts from floating to integer outside one checked function, parsing or printing floats, `<random>`'s distributions and shuffles, `std::reduce`, `std::hash` or unordered order deciding anything, sorts without a strict total order, thread counts, clocks, addresses or the locale in decisions, two calls with side effects in one expression, and raw memory hashed or saved.
- **The floating-point environment:** each simulation thread sets the default one first, and checkpoints assert it (x86-64's MXCSR, arm64's FPCR), since a new thread inherits whatever its creator had.
- **Order:** every loop that decides anything runs in a defined order (A3.2); parallel work is cut into chunks whose size and borders depend only on the data, gathered, then applied in key or id order.
- **Checksums:** XXH3 over a canonical stream, each system in a fixed order, entities by id, fields little-endian; a digest per system and for the whole state at every checkpoint, so a difference narrows to a system and a day.
- **Proof** (A17): seeded worlds run on the five builds of A2.2, on one to four threads, with islands of several window lengths, stopped, saved, reopened and resumed, and every digest must match; libc++'s randomized tie order and the order fuzzer must not move them.
  The phone runs the same check in its self-check (A2.3).
- *Proved in pre-production on your phone* (P5): a seeded world ended each day with the same checksum on x86-64, on arm64 under qemu and on your phone, on one thread and four.

### A3.5 Chance

- Every draw is keyed by (world seed, system, being, moment, purpose, index) through a chain of the SplitMix64 finaliser, a counter-based generator in Squirrel Eiserloh's way (research 03), chosen in pre-production (P5).
  Each part of the key is spread over 64 bits as SplitMix64 spreads its counter, (part + 1) × its golden constant, joined to the chain and mixed; the first five parts are mixed once for a being's draws at a moment (`chance::Draws`), and each draw adds its index.
- Systems and purposes are named, and keyed by a stable 64-bit hash of their names, XXH3 through the canonical digest, so adding a new kind of draw never shifts the others (`TIM-16`).
- Draws are whole numbers: below a threshold for a chance, a 128-bit multiply for a whole number in a range (as even as 64 bits allow, off by at most the range over 2^64), the top 53 bits for a fraction in [0, 1).
- Any thread can draw any number in any order and get the same one; the generator's statistics are tested over structured keys (indexes, beings, moments and seeds counting up: frequencies, every bit, and neighbours' correlation and differing bits), and a few draws are pinned for ever, since changing them would change every world.

### A3.6 Catalogues and tuning (`MAT-13`, `MAT-14`, `MAT-17`)

- **Content lives in TOML 1.0 files in sources:** `data/base/` for the game, one entry a file, its kind from its folder and its name from its file name; tuning files hold every tunable number; `checks/` holds what only the checks read, such as each item's expected fits (`MAT-17`) and the orders of plausible values (`MAT-05`).
- **No floats:** whole numbers are TOML integers, and quantities and ratios are strings with units ("3.5 kg", "1 h 30 min", "15%", "1 in 100") read exactly into whole base units, so the phone's parse cannot differ from the cloud's (research 18).
  "m" is only a metre, never a minute.
- **Durations record both lengths,** `{ life = "3 month", game = "15 d" }`: the simulation reads the game length, and the `TIM-18` check holds it to the rule, "about" read as within 10%.
- **Schema once:** each kind has one `visit()` naming its fields with their types, units, ranges, links and what they affect (`rules`, `world` or `look`).
  The loader, the schema writer and the fingerprinter all walk it, so nothing describes a kind twice.
  The loader refuses unknown keys and floats, and names every error by file, line and column.
- **toml++,** pinned, behind one file and with no exceptions, reads the text (research 18).
- **No templates:** every entry is complete and reads alone, since a parent's values would be the child's inputs (`MAT-13`); a tool copies an entry as a starting point instead.
- **Names:** lower case, namespaced by source (`base:flint_nodule`, `base:` implied); numbered at load by sorted name, those numbers used only for arrays; chance and tie-breaks keyed by a stable hash of the name; saves holding each kind's names; renames listed in a file (`PRN-14`).
- **Fingerprints:** a digest per entry from its canonical values, and per source three: rules (all that affects outcomes), world (only what makes the land, with the sets of plant, animal and material kinds) and look; plus a world-making version raised by hand when code that makes worlds changes, guarded by a test of golden worlds.
  Each world keeps its sources, their versions and digests (`TIM-08`); a world digest that changed means a big update (`PLT-09`).
- **Checks:** the loader checks syntax, types, units, ranges, required fields, links and `TIM-18` at every start, on the phone too; the heavy `MAT-17` checks run in the cloud on every change, written in `sim/` so the phone could run them on combinations the cloud never saw.
- **Onto the phone:** the build copies the sources into `game/data/` with `build.toml`; `view/` reads each listed file through Godot's `FileAccess` and hands the bytes to `sim/`, and the self-check compares the phone's digests with the build's.
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

### A3.8 Talking to Godot

- **Three classes for GDScript,** from `view/`: the world (make, open, save, close, export, import; commands; the goal and the frontier; counters and events), the crowd (a node that draws walkers), and the device (cores, heat, telemetry).
  A few calls a frame, never one per walker: a call into the extension costs about 0.1–0.2 µs (research 18).
- **Commands in:** plain records, stamped with the game second they act at and written to the journal before they act; while you choose a power the game is paused, so a power acts on exactly the world shown.
- **Snapshots out:** after each batch, the simulation fills one slot of a triple buffer, so neither side ever waits and the screen always takes the newest: for each walker its id, kind, colour and flags, and its activity's way (start and end positions and seconds).
  `view/` places each walker along its way at the screen's game time and copies the result into MultiMesh buffers, one per area of the world, each with its own bounding box.
- **Events worth showing** travel in a lossless queue, drained once a frame.
- No Godot object is touched from a simulation thread, and `view/` converts but never decides.

### A3.9 Threads, speed and budgets

- **The simulation runs on its own worker threads,** up to four, made with an explicit 8 MiB stack (bionic's default is 1), named, and at a slightly lower priority than Godot's main thread (`PLT-01`).
  The fastest core and the small ones stay for Godot, sound and the system; Godot's own worker pool is kept small.
  Whether the workers are pinned to the middle cores is decided by the benchmark, which runs both ways, since Android advises against pinning (research 18).
- **The speed loop:** the simulation works toward a goal at most about a quarter of a real second ahead of the screen, and sleeps once it gets there.
  - Each frame the screen's game time moves by the speed asked times the frame's real time, but never past the simulation's frontier; when it reaches the frontier, time slows (`PRN-11`).
  - The speed shown is measured from what was drawn, so it is always the real speed (`TIM-01`).
  - Pausing lets the screen glide to the frontier within that quarter second, then stop.
  - At one game second a real second, a game minute takes a real minute (`TIM-10`).
- **Heat:** `view/` reads the phone's heat headroom every 2 s with a 10-s forecast (Android forecasts only while asked at least every 10 s), and listens for its thermal status; as the forecast nears the first throttling level, the simulation's working share is cut quickly and given back slowly, so time slows before the phone throttles (research 02, 18).
- **Telemetry:** the device class also reads battery and power rails, the cores' clocks, our threads' CPU time and memory, and the interval of every frame; trace sections mark each frame and batch for the phone's own System Tracing.
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
   - *Measured in pre-production* (P1): on your phone every outline way kept 99–100% of frames on time at 60, C the dearest by half a millisecond.
4. **Light in clean steps** (`PRE-20`, `PRE-30`).
   - One shared light function, written once, used by every lit material.
   - Each material's ramp of 4 to 7 shades; the light picks the step, and steps meet in clean edges with no pattern mixing them: the sharp look you chose.
   - Hard sun shadows from a shadow map at mobile size, and cloud shadows.
   - Shade filled by the sky's cool purple-blue light, never black; hollows darker; haze by distance, warmer toward the sun; mist on water and in hollows.
   - The Mobile renderer draws forward, so each material hands its ramp, pattern, openness to the sky and firelight to the shared light function, which picks the step after the sun's shadow (P1).
5. **Fire** (`MAT-18`): a warm, flickering light as bright as its heat.
   - Flames are the art book's own flame pictures, a few frames 10 times a second, each change also setting the fire's flicker (P3).
   - Fires reach figures and tents through a firelight term in our shaders, summing up to four fires (P2).
   - **Camp zoom** puts the camera 1 km back, past the nearest ground in view, and stretches the sun's shadows, the haze and the outline depths to the view's 1.2 km of ground; beyond the sky's height map, the sky is open.
     *After the review (α0.2c's fixes):* the forest stands in patches on a meadow, as the art book's camp zoom has it, the meadow at the height and in the cover of the camp's ground where it ends, so no square shows round the camp, except at dusk, when the cliff's long shadow stops where the painter's scene ends; the crowns are wider and drawn without outlines, which made each tree a dark twig; the ground's small plants, specks of a pixel or less there, are left out; and people are drawn four times their size, as tiny figures (`PRE-28`). The fires' height maps and smoke are off there.
   Godot stops lighting MultiMesh copies once its per-object light limit is used up, so fires reach figures and huts through a firelight term in our shaders, fed by a short list of nearby fires, if needed (research 17).
   - *Proved in pre-production* (P2): a camp lit by three fires at night holds 60 frames a second on your phone.
   - **Shadows from every fire,** as you asked after P2 (*built in α0.2c*): people and things cast a shadow from each fire as well as the sun, and stand in each other's.
     Two maps of the heights round the fires, 48 m across at 512 pixels, are drawn every frame as people move, inside the picture's viewport so they are ready before it: the tops of what stands there, seen from above, and its undersides, the faces turned down, seen from below.
     For each fire, a point walks the line to 1.5 m above the fire's light, 2 m above the fire, in even steps of at least 0.2 m, at most 32: where the line runs through something, between its underside and its top, or passes from above a thing to below it between two steps, the point is in that fire's shadow.
     So a fire under a roof or an overhang lights the ground round it, while a tent, a person or a windbreak stops its light; foliage, and anything more than 2.4 m above the fires, stays out.
     The line ends above people's heads, so a whole figure casts its shadow from a fire, as you asked.
6. **Water** (`PRE-26`), the clear water you chose:
   - shallow water shows its bed, deeper water darkens away from the shore in steps, and a thin bright line marks where water meets land or anything standing in it;
   - the sky's colour on the surface, with glints;
   - the reflection of what stands above it by a second, low-resolution pass of the scene through a mirrored camera, every one of our shaders discarding what lies below the water in that pass, since Godot 4.7 has no clipped camera projection (research 01).
   - *Answered in pre-production* (P1): the mirrored pass costs at most 0.3 ms on your phone, so reflections stay.
   - The shore's line is drawn where the water is thinnest, from the depth texture (P1).
7. **At speed** (`PRE-30`, `PRE-29`): once a day passes in under about 10 seconds, the light holds steady from high up and only its tint follows the hour; the map look is always lit so.
8. **Smoke and mist.**
   - Smoke is a lit volume along its path from the fire, under a roof or overhang and away on the wind, as you asked after P2 (P3).
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
  It stays a tool for trying a change before the game has it, never a second copy of the game's code: once the game draws the look, changes are made in the game and pictured by the game (`CLAUDE.md`, rule 4).

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
- *Proved in pre-production* (P3): eleven shapes, two plants and a deer built at load from parameters in about 23 ms.

### A6.2 Copies

- Models are layouts of shared shapes, drawn as MultiMesh copies grouped by area, since a MultiMesh is culled as one.
- Each copy carries its material's colour, its wear and its maker's style in per-instance data, so huts of birch and of reed look different with no new art (`PRE-42`, `PRE-43`).
  Each copy's data holds its material's row, its pattern, its wear and a seed (P3).

### A6.3 Animation (`PRE-44`)

- Each movement is 2 to 6 key poses of the block figure, stepped about 10 times a second; our C++ poses the rigid parts, with no skeletons.
- Rules bend the poses (stoop, limp, slump, hunch), each figure's seed offsets its timing, and feet meet the ground by a two-bone sum.
- *Proved in pre-production* (P3): key poses with their in-betweens give a new pose 10 times a second.

### A6.4 Sheets

The model sheet of every kit shape in two materials, and the animation sheet, are rendered in the cloud whenever the kit changes, for your eye.

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
- The ground's height and cover anywhere come from the cells, joined smoothly, with relief only as fine as the spacing shows (P8).

### A7.6 Starting small

The generator is first built and tuned on a small island, quick to make and judge by eye, then grown to full size.
*Measured in pre-production* (P7): your phone made three worlds in 9.3 s and settled the first in 6.5, about twenty times the room `WLD-11` gives, so the stages can grow richer.

## A8. From a person to the globe (research 07)

### A8.1 Levels of detail (`PRE-03`)

Our own system in `view/`, driving Godot's RenderingServer directly: one tree of square chunks over the whole world, each splitting into four as the camera nears, so the ground is detailed within about 300 m from camp zoom inward, shaped every few tens of metres out to about 10 km, and the world cells' own from the region out.
Each chunk morphs into the grid half as fine as the camera draws away (CDLOD, Strugar 2010), so no level pops and the pixel look never blurs, and a full area shows its coarse ground until its detail is in, within about a second.
*Changed in P8's second round,* after your verdict on 5 October 2026: the rings, the map mesh and their dithered hand-overs, built first, gave way to the tree, since the hand-overs and the map's bend were what you saw jump.


The tree's chunks are made on worker threads, nearest first, and freed by when the tree last reached them; only what the camera can see is drawn or split (P8's second round).

### A8.2 A moving origin

Godot draws in single precision around an origin that moves with the camera, as Kerbal Space Program does; the simulation's exact coordinates never depend on it.

### A8.3 Things by distance (`PRE-28`)

- Grass and small stones only near.
- Trees as models, then impostor cards, then the cover's colour.
- People, herds and camps as models, then tiny figures outlined in a darker shade of their own colour, then markers: a banner for a camp, one mark for a herd.

One rule for each thing, on the graphics chip, read by everything that draws it, so trees, plants, ground and camp always agree (P8's second round).

### A8.4 The camera (`PRE-03`, `PRE-29`)

- Orthographic and pitched for the close stops, tilting toward straight down as it rises.
- Perspective for the globe: the flat map bends onto a sphere for the last step, as Google Maps morphs to its globe, squeezing the polar lands and hiding the seam under the ice (`WLD-02`).
- *Changed in P8's second round,* as you asked on 5 October 2026: the world is a sphere at every scale and never unrolls; near the focus the ground is stretched east to west back to its true size as the zoom closes in, so the close stops are as before.
- Perspective at every stop, each chunk set on the sphere by its vertex shader, exact near the focus (P8's second round).

### A8.5 The map look (`PRE-29`)

World cells in flat cover colours, rivers as lines (from the region out, those draining about 1,000 km² or more), hills shaded the cartographers' way, lit from high up at every hour with only the tint following it, sea in depth bands with the shore's bright line (`PRE-26`).
*Changed with your OK on 5 October 2026:* the land vivid and textured with what can be seen from above, lit by the sun of the hour, with the weather's clouds and their shadows (`PRE-29`), as P8's second round draws it, below.
The variants for the land, the sea and the rivers that P8's second round drew for you are in `LESSONS.md`.

### A8.6 Time and light by zoom (`TIM-01`, `PRE-30`)

One gesture sets where you look and how fast time runs, from real speed at the person to top speed at the globe.
From the valley out a day passes in under a second, so the light holds steady.
Weather, clouds and air are drawn from the climate on the graphics card, in the variants of P8's second round (`LESSONS.md`).

### A8.7 Budget

Each level's cost is measured on your phone at every zoom stop (`PLT-04`), and so is the time to make a full area.
*Answered in pre-production* (P8): built twice and judged by you; its Measure on your phone is still to come.

## A9. Living things (research 08), outline

- Each species is a catalogue entry: climate and soil ranges, seasons, size, diet, group size, yields, life cycle, model-kit form and colours (`WLD-31`, `WLD-32`).
- Plants by ecology: which types can grow from BIOME1's numbers, which win by dominance, how dense by competition and self-thinning; single plants placed from that density by keyed chance, so a place always grows the same plants (`WLD-13`).
- Animal numbers by Damuth's law: each species' natural density from its body mass, a sixth of it per cell (`WLD-30`); predator and prey rules on the cells' totals, driven by the weather (`WLD-18`).
- Herds' days by need zones and hours, their years by following the green-up, their movement by Reynolds' steering rules plus those goals.
- Near people, individuals; far away, counts, with condition and wariness carried both ways (`WLD-32`).
- *Proved in pre-production* (P9): these rules on the cells' totals held every species within 0.66 and 1.10 of its total for a century in 20 worlds; their lessons are in `LESSONS.md`.

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
  *Measured in pre-production* (P6): about 32,000 decisions a game day for a thousand people, 6.0 game years a real minute on your phone's four cores; fuller minds may cost about 2½ times as much before the speed falls below 2½.

## A12. Crafts and discovery (research 11), outline

- An item's 18 characteristics come from its material and form, and made things inherit from their inputs (`MAT-03`).
- Blueprints match characteristics and classes, never names (`MAT-04`, `PRN-07`), fenced by the expected-fits check (`MAT-17`).
- Every reality rule has a real experiment behind it, cited in its catalogue check (`RCK-01` and the rest of section 7.6).
- Quality from skill and inputs (`MAT-20`); skill grows by the power law, a few years to competence and five to ten to mastery (`MND-06`); teaching beats watching (`MND-13`).
- Discovery belongs to people: accidents, personal hunches and copying found things (`MND-11`); crafts die with their last holder and return only by rediscovery, neighbours or copying (`CUL-02`, `CUL-16`).
- *Proved in pre-production* (P4): tuning alone, with the world's own rules, brings flakes and fire into their windows; each blueprint's discovery factor lives in its catalogue entry and is re-tuned with the whole catalogue (`RSK-01` stays open until M7).

## A13. Culture (research 12), outline

- A naming language per people from the seed, in O'Leary's way, spelled only with letters the pixel font has (`CUL-17`, `CUL-18`).
- Customs as fixed questions with a few answers, each answer from a band's own cases (`CUL-06`).
- Beliefs and rites from coincidences: the act before a good outcome becomes a rite, harm after an act a taboo (`CUL-05`, `CUL-20`, `CUL-34`); small bands keep vivid, rare rites, large villages regular ones (`CUL-26`).
- Societies with real numbers: bands of about 28 adults, a few families linked by kin and marriage; leaders kept in check; gifts as insurance; villages only where stores allow (`CUL-30`, `RES-07`).
- Violence in its real order: personal killings and revenge first, raids growing with stores (`CUL-31`).
- Stories and gossip drift as transmission chains do; styles drift by copying with small changes (`CUL-11`, `CUL-12`).
- *Proved in pre-production* (P10): customs, spirits, rites and splits came from events alone, each inside its window in most of 20 worlds; the lessons are in `LESSONS.md`.

## A14. Story, the book of ages and the writer (research 13), outline

- **The director** keeps Left 4 Dead's rhythm without its power: peaks, then a guaranteed rest; it reads the world and sets only speed and live moments (`TIM-02`, `TIM-03`).
  A test runs a world with it on and off and compares the results.
- **Recognisers** are story-sifting patterns over the event log; half-matched ones are the director's signs, so time slows before an outcome without looking ahead (`PRE-39`).
- **Pattern sentences:** a small grammar, at least 5 phrasings for each kind of event, picked by the event's seed and filled from its records (`PRE-37`).
- **The writer:** Gemini Nano through ML Kit's Prompt API, with a fixed seed, sentence by sentence, behind the Android plug-in; it writes only while the app is in front, queues and backs off, and stops for the day at its battery quota.
  Every rewording passes a strict check without any model, and dark events never reach it (`PRE-41`, `PRE-17`); pattern text always works alone.
- *Proved in pre-production* (P11): the director slowed time 9.5 times an hour within its budget and caught every named discovery, and every world ended identical with it on and off; signs are scored by how often they come true.
- *Not proved:* P13 was left unbuilt when pre-production closed; the writer is proved when M9 builds the book, and pattern text stands alone until then (`PRE-37`).

## A15. The interface (research 14)

- **The world fills the screen;** panels show only what the moment needs, then fade (`PRE-32`).
- **One column of panels:** full width with controls in the bottom third in portrait, beside the world in landscape (`PRE-34`).
- **Every control at least 48 dp,** with 8 dp between.
- **Our own gesture reader on raw touches,** so all gestures share one rule set and a scripted test can tell them apart; edge swipes stay out of the system's gesture insets (`PRE-33`).
- **One Godot theme in the art bible's palette.**
  The plain pixel font for everything read, at whole multiples of its design size, nearest filtering and no subpixel positioning; the pixel handwriting only for big titles, at twice the size, as the art book's interface plates show.
  The layout is built on a square base, so both orientations scale alike; safe areas and cutouts come from `DisplayServer`.
- **Cards open to what matters now,** with deeper sections folding out (`PRE-35`); screen-reader labels come with Godot 4.5's support.
- *Built in pre-production* (P12): art pixels at a whole multiple of the screen's pixels, the pixel fonts from the art book's source, one gesture reader and every control within reach; your verdict on reach, legibility and the panels' ground is still to come.

## A16. Sound (research 15), outline

- Ambience as layers from the place's land, water, weather, hour and season, plus one-shots only from real things near the camera (`SND-11`, `PRN-10`).
- The 44 base sounds made by our C++ at load and varied each play; live synthesis only for what follows the world continuously, such as fire by its heat (`SND-06`).
- Godot's 3D players for direction and distance, their low-pass for distance, an area with reverb for each cave; muffling by land from the simulation's line test (`SND-08`).
- A voice manager keeps `SND-01`'s 32 voices, blending the quietest into its kind's hum when a share is full (`SND-07`).
- The murmur in the people's language, shifted for age, build and feeling (`SND-03`); the speaker's missing bass restored by harmonics, off with headphones.
- *Built in pre-production* (P14): in the cloud, 32 sounds at most with the mix at 2.7% of the audio thread's time; a 3D sound world needs a camera to sound; your ears and Measure are still to come.

## A17. Testing and checks (research 16, 18)

- **C++ tests** with doctest, and property tests with RapidCheck for rules that must always hold, such as no result heavier than its inputs (`MAT-09`).
- **Scenes and whole worlds** run by the C++ library alone, through the `kindling` tool, many at once in the cloud: each scene is a TOML file in `data/scenes/` stating, before its first run, the items it checks, its seed, its runs, its time limit, its budget and its pass rule, counted over about 20 runs where chance matters (`RES-21`, `RES-09`, `RES-13`).
  A run keeps checkpoints and resumes from the last as if it had never stopped (`PLT-05`); a run with a test switch says so in its report and its world (`RES-10`).
- **The same results everywhere** (A3.4): seeded worlds on the five builds, on one to four threads, with islands of several window lengths, stopped, saved, reopened and resumed; libc++ with its tie order randomized, and the order fuzzer scrambling EnTT's pools; every digest must match (`RES-05`).
  The check also reads the compile commands for the last floating-point flag, scans our built code for fused instructions and for the platform's maths functions, and holds the maths library to MPFR's correctly rounded answers.
- **Saves:** the headless tool killed at random a hundred times mid-run, each reopening carrying on exactly (`PLT-07`); damaged files (cut, a flipped bit, zeros) refused; the corpus of old worlds opened by every build (`PLT-09`).
- **Catalogues:** a test catalogue with one planted fault for each check, each refused at its file, line and column (`MAT-17`).
- **The Godot side** with gdUnit4: layouts, cards and views opened from records, headless; gestures by simulated touch under Xvfb, since Godot's headless mode drops input events.
  Every script is compiled before the tests, so an error in one no test loads still stops the check.
- **Pictures and reels** by Movie Maker mode at a fixed frame rate: golden pictures in the cloud; the contact sheet and the sound reel on the phone for your reviews (`PRE-31`, `SND-12`).
  - Movie Maker records at the project's base size, so a single picture at the phone's 1344 × 2992 pixels is read from the screen by our script (`tools/picture.sh`).
  - A rendering driver named on the command line brings Forward+ unless the Mobile renderer is named beside it.
- **Phone measurements:** the in-app benchmark (A18.1), its frames by our own measure, its result in a short code; trace sections that the phone's own System Tracing records beside the chip's speed and heat, with no computer; Android GPU Inspector for a slow frame on the PowerVR chip, if ever needed (`PLT-04`).
- **One command before anything joins:** `tools/check.sh`, rebuilt for C++ and Godot, runs the formats, lints, builds, tests, the same-results check, and the file, commit and coverage checks (`PRC-10`, `PRC-12`).
  - It costs about what changed, since every delivery waits on it: C++ compiles through ccache, so godot-cpp and unchanged files compile once across runs and build folders; each C++ file is linted, on every core, only when its code, the headers it reads, its compile command or the rules changed since it passed, and each project's tests, the Godot project's import and tests and the picture test run only when something they read changed (`tools/cppcache.py`); the C++ tests run beside the Godot and tool tests; and each step prints its time.
    Measured on 5 October 2026: about 20 seconds with nothing changed, about 45 with one line of one library changed, and about 5 minutes the first time, which fills the caches.
  Production's foundations bring every kind of check to the game's own code (M1).

## A18. Budgets and risks

### A18.1 Budgets

As measured on your phone in pre-production (`LESSONS.md`), each re-measured at every milestone (`PLT-04`):
- **Frame:** 16.7 ms at 60 frames a second, the graphics chip under about 8 ms in the busiest scene, so heat leaves room; at least 97% of frames on time while moving the camera (`PLT-04`).
  - The close camp (P1): 99–100% of frames on time at 60, the chip about 10 ms a frame, partly idle, since it lowers its clock with time to spare.
  - A busy camp at night with thirty figures and three fires (P2): 5.5 ms of the picture's pass at 120 frames a second; every frame on time at 60 at close and camp zoom, with trees of 12 triangles at camp zoom.
  - The model sheet with fire shadows and smoke (P3): 4.3 ms at 120, every frame on time at 60.
  - Measure sums every pass of the frame; the heat over ten minutes of a busy scene is still to measure.
- **Simulation:** up to the four middle cores at held speed (`PLT-01`): a thousand simple minds at 6.0 game years a real minute (P6), so production's fuller minds have about 2½ times P6's cost before `TIM-07`'s hoped-for speed falls.
- **World generation:** three worlds in 9.3 s and settling in 6.5 on your phone (P7), against `WLD-11`'s 3 minutes and 1.
- **Sound:** 32 sounds at about 3% of the audio thread's time in the cloud, 9% at worst (P14); the phone's figure to come.
- **Power:** about 3 W while playing; **memory:** within about 8 GiB.
- **The APK:** 25 MB from Godot itself, 38 MB with every prototype; within the 50 MB limit for files committed to the repository.
- **The foundations,** from research 18 in the cloud, each measured again on your phone by M1's benchmark:
  - the event queue: 1–4% of one core at `TIM-07`'s speeds;
  - drawing 10,000 walkers: about 0.26 ms of the main thread a frame (filling and uploading their buffer);
  - loading a launch-size catalogue: 25–33 ms;
  - a save: a pause of tens of milliseconds to copy the state at an event, the rest on other threads;
  - opening a world: within `PLT-04`'s 3 seconds.
- **Storage** (`PLT-10`): a world of 7,000 people at Year 250 estimated at 3.5–4.1 GB, against 4 GB: its history fits at about 17–30 events a person a game day.

**M1's benchmark,** one tap and about 20 minutes, with the phone unplugged, in flight mode, after it has cooled:
- the calendar alone at top speed; 10,000 markers at real speed and at top speed, with the camera touring, held speed read after 3 minutes; the same pinned to the middle cores; a sweep through the zoom stops' speeds; saves every 30 seconds with an export and a reopening; a still camera for the screen's own power;
- for each, its end state's digest against the cloud's, the share of frames on time, the slowest frame, the speed held, heat, battery and power, and memory, in one code.

### A18.2 Risks

| Risk | What we know | If it fails |
|---|---|---|
| The PowerVR driver mishandles a feature we use | P1 to P3 drew outlines, mirrors, fire shadows and smoke without fault | avoid that feature; the self-check reports the driver |
| The look costs too much on the Mobile renderer | P1, P2: 60 frames a second with room | a cheaper outline method; fewer cards |
| A busy scene heats the phone | Open: P2's run was cut to 90 seconds | smaller shadow maps; time slows earlier |
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
| Islands give a different answer, or too little parallel work | Designed from the literature, not yet built (research 18) | one worker in key order, the same results; speculation inside big islands |
| EnTT misbehaves on the phone | Built only in the cloud | flecs behind the same thin layer |
| The screen stays at 120 Hz | Read from Godot's source: set the frame cap again at run time | Android's frame-rate call through JavaClassWrapper; a small plug-in |
| History outgrows 4 GB (`PLT-10`) | Estimated at 3.5–4.1 GB for 7,000 people at Year 250 | a tighter encoding; what counts as an event, with you |
| Opening many small catalogue files is slow on the phone | Not measured | one file a kind, read by the same loader |
