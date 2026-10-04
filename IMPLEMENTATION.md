# Kindling: implementation plan

The order in which Kindling is built.
It follows `PROJECT.md` (what the game must be) and `ARCHITECTURE.md` (how it is built), and cites both: items by ID (`TIM-16`), parts of the architecture by section (A3.4).
It goes bottom up, as you asked: the foundations and data structures first, then the graphics engine, the world, living nature, people, minds, crafts, culture, and the game itself last (research 00).
Every step ends with a build on your phone, and every milestone with a report you review (`RES-06`, `RES-22`).

Only the next milestone is planned in detail.
The later ones are outlines (their goal, the items they deliver, what you will see), each detailed when it comes next, from what the earlier ones taught.
The plan holds only work still to do: a step leaves it when it is done, and the code, which names the items it implements, is the record (`CLAUDE.md`, rule 3).

## Status (3 October 2026)

- Written from scratch after you stopped the old plan, which stays in git history at commit `ebaeae3`.
- **Waiting for your OK**, with the proposals it needs in `PROJECT.md` (listed in its section 17.2):
  - the ten bottom-up milestones (`SCP-16`, `MIL-08` to `MIL-17`), the old seven retired, and the items that named them pointed at the new ones;
  - the early steps are tried rather than played (`PRN-09`, `SCP-03`, `PRC-11`);
  - the documents and what comes next (`PRC-04`, `PRC-08`).
- Nothing is built until then.
  On your OK, the first step removes the old code (git keeps it) and building starts at α1.1a.

## How to use this plan

For the AI agent building a step:
1. **Pick** the first step in the status table that is not done, unless the owner names another.
2. **Read**, in this order:
   - the step's section and its milestone's lines;
   - every architecture section and item it cites, and the research notes those sections cite;
   - the last note (`dist/NOTE.md`), for anything the owner reported.
3. **Set up** the session:
   - run `tools/setup.sh` if tools are missing;
   - fetch main by name (`git fetch origin +refs/heads/main:refs/remotes/origin/main`), since a shallow cloud clone otherwise never sees it;
   - work on the session's own branch, brought up to date with main first.
4. **Build the tasks in order.**
   Each ends with its code and tests passing, and a commit naming the task and the items it delivers (see Conventions).
   - If a task can't be built as the architecture says, stop that task and add a **Conflict:** note at the end of the step's section, with the reason and the smallest change that works.
     Carry on with that change, and update the architecture in the same branch.
   - If a step will clearly take more than about 6 hours, split it at a task boundary into two lettered steps, each still ending with a build.
5. **Deliver** (`PRC-11`, A2.3):
   - the signed APK in `dist/`;
   - the note: what is new, what to try, what is rough, the items delivered and the links, published at the note's link.
6. **Check:** `tools/check.sh --deliver` passes (`PRC-10`).
7. **Review** (`PRC-09`):
   - The builder reviews each lettered step itself: each new test made to fail once, and the pictures looked at.
   - At the last step of each numbered alpha, one independent subagent verifies the whole alpha.
     It is given only the alpha's diff, its sections as they stood when it began, and the items it claims.
   - The pull request says which review approved it.
8. **Join and tell:**
   - merge into main;
   - take the step's section and status row out of this plan, moving anything the code doesn't hold, such as a decision or a measured number, into the architecture;
   - tell the owner in two or three lines, with the note's link.
9. **At a milestone's end:**
   - the milestone report (`RES-06`), with the phone's numbers (`PLT-04`) and, from M2, the contact sheet (`PRE-31`);
   - the next milestone detailed here, and its architecture sections written in full;
   - the owner approves both with the milestone review (`RES-22`).

For you, the owner:
- Each step's **On the phone** says what to open and what you should see.
  Each build installs over the last (`PLT-06`).
- Reply with anything that looks wrong: it goes into the next step.
- At each milestone's end you get a short report to accept or send back, with the next milestone's plan for your OK.

## Conventions

- **Milestones** are M1 to M10, in order: `MIL-08` to `MIL-17` in `PROJECT.md`.
- **Steps:** each milestone is built as numbered alphas, and each alpha as lettered steps of a few hours.
  α1.2b is milestone 1, alpha 2, step b; it is written `a1.2b` in file and branch names.
- **Tasks:** `T<step>.<n>`, such as `T1.2b.3`; they never change once a step starts.
- **Commits:** `T1.1c.2: keyed chance (TIM-16, A3.4)`, then the attribution lines the session requires.
  A commit that changes `PROJECT.md` also carries the `Changed:` lines of `PRC-07`.
- **Items in code** (`CLAUDE.md`, rule 3):
  - C++ and shaders name what they implement in a doc comment (`/// Implements TIM-16, see A3.4`), and each test what it checks on a line above it (`// checks: TIM-16`);
  - GDScript does the same with `## Implements` and `# checks:`;
  - catalogue entries and scenes list theirs in `checks = [...]`.

  `python3 tools/filecheck.py where TIM-16` then finds them all.
- **Scenes:** TOML files in `data/scenes/`.
  Each states, before its first run, the items it checks, its seed, its runs (about 20 where chance matters), its time limit, its budget in session-hours and its pass rule (`RES-09`, `RES-13`).
- **Numbers:** every tunable number lives in a catalogue or a tuning file, never in code (`PRN-14`).
- **Version codes:** milestone × 10000 + alpha × 100 + step (a = 1), so α1.2b is 10202.
  The first is above the old app's 1014, so it installs over it; the version name is the step's name (A2.3).
- **Hours:** the builder's estimate for building, testing and delivering a step; your time is the few minutes of trying it.

## Definition of done (every step)

1. Every task is built with its tests, and committed.
2. `tools/check.sh --deliver` passes (`PRC-10`):
   - formats, and lints with warnings as errors;
   - the C++ and Godot tests;
   - the catalogue checks and the same-bits check (`RES-05`), once they exist;
   - the file, commit and coverage checks (`PRC-12`);
   - the APK check.
3. The step's scenes pass at their stated rules, and every earlier quick scene still passes (from α1.2c).
4. The phone's numbers are recorded in the note (from α1.2c).
   A slowdown of more than 10% against the previous step carries a reason or a fix.
5. A changed save format brings its migration and an old save that must still open (`PLT-09`, from α1.2b).
6. The review approves (`PRC-09`).
7. The APK is built, signed and committed, and the note is written and published (`PRC-11`).
8. The step's section and status row have left this plan.

## Rules every alpha keeps

These items hold for the whole build rather than being delivered by one step.
Every step keeps them, the independent review checks them, and the coverage check counts them as served:
- **Principles:** `PRN-16`, `PRN-01`, `PRN-02`, `PRN-07`, `PRN-05`, `PRN-12`, `PRN-17`, `PRN-03`, `PRN-04`, `PRN-10`, `PRN-13`, `PRN-06`, `PRN-15`, `PRN-11`, `PRN-09`, `PRN-14`.
- **Scope and non-goals:** `SCP-02`, `SCP-03`, `SCP-15`, `SCP-04`, `SCP-05`, `SCP-06`, `SCP-07`, `SCP-08`, `SCP-09`, `SCP-10`, `SCP-11`, `SCP-12`, `SCP-17`, `SCP-18`, `SCP-19`, `SCP-20`, `SCP-21`.
- **Process:** `PRC-02`, `PRC-03`, `PRC-04`, `PRC-06`, `PRC-07`, `PRC-09`, `PRC-10`, `PRC-11`, `PRC-12`.
- **Testing:** `RES-01`, `RES-09`, `RES-13`, `RES-18`, `RES-19`.

## What the plan asks of you

- **Now:**
  - your OK for this plan and its proposals;
  - the numbers from the bake-off app, for P1: the "frame" and "gpu" lines while you pan, and while it is still.
- **Once, if not yet done, before M1 ends:**
  - register the package `dev.kindling.app` and the release certificate's fingerprint (`android/keys/release-cert.sha256`) in your hobbyist developer account (`PLT-06`);
  - make `main` the default branch on GitHub (Settings, General, Default branch).
- **Each step:** install it when you like, and reply with anything that looks wrong.
- **Each milestone's end:** the report, a few minutes with the build, and your OK for the next milestone's plan.
  At M2's end, your verdict on the look against your reference pictures, before anything is built on it.
- **Choices by eye and ear, when their milestone comes:** the fix for crawling pixels at M2 (`PRE-22`); the murmur's voice (`SND-03`) and the drums (`SND-02`) at M9.

## Status

| Step | Title | Milestone | Hours | Status |
|---|---|---|---|---|
| P1 | The renderer | Pre-production | 1 to 3 | Waiting for your numbers |
| α1.1a | The app shell | M1 | 5 | Waiting for your OK |
| α1.1b | The simulation library | M1 | 4.5 | Not started |
| α1.1c | Numbers, time and chance | M1 | 5.5 | Not started |
| α1.2a | Entities, events and catalogues | M1 | 5.5 | Not started |
| α1.2b | Saves | M1 | 5 | Not started |
| α1.2c | Scenes and the benchmark | M1 | 5.5 | Not started |
| M2 to M10 | Outlines below | M2 to M10 | | Detailed when each comes next |

## Pre-production

Done:
- the research, topic by topic, with its sources (`research/`);
- the art guide (research 04), and the look you chose: soft painted nature with pixel-textured made things, their textures made by code;
- the engine bake-off: you chose Godot, with the simulation in C++ (research 01);
- the architecture and this plan.

### P1 The renderer

**Question:** can Godot's Forward+ renderer, which gives the outline pass its normals, draw the look within the frame's budget on your phone's PowerVR chip, or must it be the Mobile renderer (A4.1, `PLT-04`)?

**How it is decided:**
- If the bake-off scene's graphics time stays under about 8 ms a frame while panning, Forward+ stays.
  That is half a frame at 60 frames a second, leaving the other half for everything the game adds.
- If not, the bake-off scene is built again on the Mobile renderer, with the outlines reading depth only, and you send its numbers too.
- The choice goes into A4.1, and the first frame budgets into A12.

**On the phone:** open the bake-off app, pan for ten seconds, and send the "frame" and "gpu" lines, then the same with the picture still.

## M1 Foundations

**Goal:** the data structures and plumbing everything else stands on (A2, A3):
- the app and its delivery, and the simulation library;
- numbers, time and chance, with the same bits on the phone and in the cloud;
- entities, activities, catalogues, commands and snapshots;
- saves, scenes and the phone's benchmark.

**Serves:** `TIM-05`, `TIM-08`, `TIM-14`, `TIM-16`, `TIM-17`, `TIM-18`, `PLT-01`, `PLT-03`, `PLT-04`, `PLT-05`, `PLT-06`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `MAT-05`, `MAT-13`, `MAT-14`, `MAT-17`, `RES-05`, `RES-06`, `RES-10`, `RES-12`, `RES-21`, `RES-22`.

**You will see:** no game yet, but its foundations working on your phone, on a test screen:
- the calendar running through the 60-day year at the speed you set;
- a crowd of 10,000 markers moving smoothly while the simulation counts its ticks;
- worlds saved, closed and reopened exactly;
- a line confirming that your phone computes the same bits as the cloud.

**Risks:**
- The C++ build for Android: proven in the first two steps.
- The same bits on the phone's chip: proven in α1.1c; if it fails, the architecture changes before anything is built on it.
- The crowd's cost per frame: measured in α1.2a.

### α1.1a The app shell

**Goal:** the new Kindling opens on your phone: a Godot app, built, signed and delivered from the cloud by one command, installing over the old app.

**Serves:** `PLT-01`, `PLT-03`, `PLT-06`.

**Architecture:** A2.1, A2.2, A2.3, A2.4, A11.

**Tasks:**

1. `T1.1a.1` **Clear the old code (`PLT-06`, A2.1).**
   Remove the Rust workspace, the web build, the old Android shell, and the old catalogues, benchmarks and tools; git keeps them at `ebaeae3`.
   Keep the release certificate (`android/keys/`), `tools/signing-key.py`, the note's page maker and the research, and lay out the folders of A2.1.
2. `T1.1a.2` **The setup script (`PRC-10`, A2.4).**
   `tools/setup.sh` installs, pinned and checked by checksum: Godot 4.7.2 and its export templates, the Android SDK, NDK and JDK, CMake and Ninja, the GDScript formatter and linter, gdUnit4, Xvfb and the software Vulkan driver.
   It says nothing when all are present.
3. `T1.1a.3` **The Godot project (`PLT-01`, A2.1, A4.1).**
   `game/` with Vulkan on Android, Forward+ unless P1 has chosen otherwise, nearest filtering, and the orientation following the phone.
   Its main scene is a test screen: the version, the build, the phone's model and the screen's size, laid out for portrait and landscape.
4. `T1.1a.4` **Build, sign and deliver (`PLT-06`, `PRC-11`, A2.3).**
   `tools/build.sh` exports the APK headless, sets its version code and name from the step, and signs it with the release key through `tools/signing-key.py`, as the bake-off's `sign-apk.sh` does.
   It then checks the APK: package `dev.kindling.app`, arm64 only, the release certificate, and no network permission.
5. `T1.1a.5` **The checks for the new layout (`PRC-10`, `PRC-12`, A11).**
   `tools/check.sh` runs the formats and lints for GDScript and Python, the Godot tests headless (gdUnit4), and the file, coverage and note checks; with `--deliver`, also the build and the APK check.
   The coverage check reads C++ and shaders (`///`, and `// checks:` above each doctest `TEST_CASE`), GDScript (`##`, and `# checks:` above each gdUnit4 `func test_`) and data files, in `game/`, `sim/`, `data/`, `art/` and `tools/`.
6. `T1.1a.6` **One phone, offline (`PLT-01`, `PLT-03`).**
   The app asks for no network permission, and a check fails any code that opens a connection.
   It targets only arm64 and the phone's Android version.
7. `T1.1a.7` **Deliver α1.1a (`PRC-11`).**
   The APK, the note and the checks, as the definition of done says.

**Tests:**
- gdUnit4: the test screen lays out in portrait and landscape and shows the version (`PLT-01`).
- The APK check: package, version code, signature, arm64 only, no network permission (`PLT-06`, `PLT-03`).
- The tool tests in `tools/tests/`: the file, coverage and note checks.

**On the phone:**
- Install from the note's link: it installs over the old Kindling, whose test views are gone.
- It opens on a plain test screen with its version (α1.1a), your phone's model and the screen's size.
- Turn the phone and it follows; it works in flight mode.

### α1.1b The simulation library

**Goal:** the C++ library that will hold the whole simulation builds for the cloud and for the phone, and the app talks to it.

**Serves:** `PLT-01`, `PLT-05`.

**Architecture:** A1.2, A2.2, A3.1, A3.7.

**Tasks:**

1. `T1.1b.1` **The library and its tests (`PLT-05`, A2.2).**
   `sim/core` as a C++20 library built with CMake and Ninja, under the compiler rules of A2.2: warnings as errors, no fast-math, no contraction.
   Its doctest tests build and run natively in the cloud, and again in a second build with the sanitizers.
2. `T1.1b.2` **The extension (`PLT-01`, A3.7).**
   `sim/bind` with godot-cpp pinned to Godot 4.7, built for arm64 Android with the NDK and for x86-64 Linux.
   One class, `Simulation`, is registered with Godot, and `game/` loads it through its `.gdextension` file.
3. `T1.1b.3` **The boundary (`PRN-14`, A3.1).**
   A check fails if `sim/core` includes anything from Godot or godot-cpp: the core builds and runs its tests with no Godot at all.
4. `T1.1b.4` **The checks build the library (`PRC-10`, A11).**
   `tools/check.sh` formats the C++ (clang-format), lints it (clang-tidy, warnings as errors), builds both targets, runs the doctest tests, and loads the extension in Godot headless.
5. `T1.1b.5` **The app calls the library (`PLT-01`).**
   The test screen asks the library for its version and for the sum of a known series computed in C++, and shows both.
6. `T1.1b.6` **Deliver α1.1b (`PRC-11`).**
   The APK, the note and the checks.

**Tests:**
- doctest: the version and the series (`PLT-05`).
- gdUnit4: the extension loads headless and answers (`PLT-01`).
- The boundary check, and the APK check finding the arm64 library.

**On the phone:** the test screen adds a line from the C++ library running on your phone: its version and the series' sum.

### α1.1c Numbers, time and chance

**Goal:** the simulation's arithmetic, clock and chance, giving the same bits on your phone and in the cloud.

**Serves:** `TIM-14`, `TIM-16`, `TIM-18`, `RES-05`.

**Architecture:** A3.3, A3.4.

**Tasks:**

1. `T1.1c.1` **Our own maths (`RES-05`, A3.4).**
   Sine, cosine, arctangent, exponent, logarithm and power, in single and double precision, from IEEE operations only.
   Each stays within a stated error of a reference, and matches a table of known answers bit for bit.
2. `T1.1c.2` **Keyed chance (`TIM-16`, A3.4).**
   A counter-based generator (Philox, 4 × 32 bits, 10 rounds), keyed by world seed, system, being, tick, purpose and index.
   It gives uniform numbers, ranges, normal draws and weighted choices, checked by known-answer vectors and statistical tests.
3. `T1.1c.3` **Ticks and the game year (`TIM-18`, A3.3).**
   Game time in whole ticks: the 24-hour day, the 15-day season and the 60-day year.
   Spans up to about two weeks take their real time, and longer ones about a sixth of it, as `TIM-18` says.
4. `T1.1c.4` **Dates (`TIM-14`).**
   Dates such as "Year 112, autumn, day 6", from the tick count, starting at Year 1, spring, day 1.
5. `T1.1c.5` **The same bits on the phone and in the cloud (`RES-05`, `TIM-16`, A3.4).**
   A fixed workload of maths and chance, hashed, is built for x86-64 and for arm64 Linux, and run in the cloud natively and under qemu; `tools/check.sh` compares the two hashes.
   The expected hash goes into the app, so the phone checks its own.
6. `T1.1c.6` **The clock on the test screen (`TIM-14`, `TIM-18`).**
   The date and hour running, with pause and a speed dial from one game minute to one game year a second, and the same-bits line.
7. `T1.1c.7` **Deliver α1.1c, and α1.1's independent review (`PRC-11`, `PRC-09`).**
   The APK, the note, the checks, and the review of the whole of α1.1.

**Tests:**
- doctest: each maths function's error and known answers (`RES-05`).
- doctest: keyed chance's vectors, its independence from the order of draws, and its statistics (`TIM-16`).
- doctest: ticks, seasons and dates, with round trips across year ends (`TIM-18`, `TIM-14`).
- The same-bits check across x86-64 and arm64, in `tools/check.sh` (`RES-05`).

**On the phone:**
- The test screen shows "Year 1, spring, day 1, 06:00", running; raise the speed and watch the seasons turn.
- A line says "Same bits as the cloud: yes".
  If it says no, send the code under it.

### α1.2a Entities, events and catalogues

**Goal:** the simulation's backbone: thousands of beings as entities, activities with an end on one event queue, content from catalogues, and snapshots that Godot draws smoothly at any speed.

**Serves:** `TIM-17`, `MAT-05`, `MAT-13`, `MAT-14`, `MAT-17`.

**Architecture:** A3.2, A3.3, A3.5, A3.7, A3.8.

**Tasks:**

1. `T1.2a.1` **Entities and systems (`PRN-14`, A3.2).**
   EnTT, pinned: components as plain data, systems registered and run in a fixed order each tick, and entities with generational IDs.
2. `T1.2a.2` **Activities with an end (`TIM-17`, A3.3).**
   One event queue, ordered by tick, entity and sequence.
   Activities have a start and an end, their results land at the end, and anything can interrupt them, by the same rules for everyone.
3. `T1.2a.3` **Catalogues (`MAT-13`, `MAT-14`, A3.5).**
   TOML files in `data/`, read with toml++, one entry per thing under a stable name.
   Each kind of entry is declared by the code that uses it, and a test kind proves that a new entry needs no code.
4. `T1.2a.4` **The catalogue checks (`MAT-17`, `MAT-05`, A3.5).**
   Types, ranges, links between entries and plausible values for each field, each failure naming its file, line and rule.
   `tools/check.sh` runs them, and a fixture holds one of each fault.
5. `T1.2a.5` **Commands and snapshots (`PRN-11`, A3.7).**
   Commands are queued and applied at the next tick.
   After each batch of ticks, double-buffered arrays of positions, headings, kinds and colours are published, and `sim/bind` copies them straight into a MultiMesh buffer.
6. `T1.2a.6` **Threads and the frame budget (`PRN-11`, `RES-05`, A3.8).**
   The simulation runs on its own threads and, each frame, as many ticks as fit the budget.
   Parallel work is split into fixed chunks merged in a fixed order, so one thread and four give the same hash.
7. `T1.2a.7` **The crowd (`PRN-11`).**
   On the test screen, 10,000 markers wander by keyed chance, each walk an activity with an end, drawn from the snapshots.
   The screen shows the frame time, the ticks a second, and the speed asked for and held.
8. `T1.2a.8` **Deliver α1.2a (`PRC-11`).**
   The APK, the note and the checks.

**Tests:**
- doctest: the queue's order and interruptions under thousands of overlapping activities (`TIM-17`).
- doctest: loading entries, and each planted catalogue fault caught (`MAT-13`, `MAT-17`, `MAT-05`).
- doctest: one thread and four end with the same hash (`RES-05`).
- gdUnit4: the crowd draws a snapshot (`PRN-11`).

**On the phone:**
- Tap Crowd: 10,000 markers wander smoothly.
- Raise the speed: they move faster until the phone's limit; then the held speed stops rising while the picture stays smooth.
  Time slows, and the screen never stutters.

### α1.2b Saves

**Goal:** worlds are always saved, exactly, and open across versions; several are kept, and any can be exported.

**Serves:** `TIM-05`, `TIM-08`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`.

**Architecture:** A3.6.

**Tasks:**

1. `T1.2b.1` **Saving (`PLT-07`, A3.6).**
   A world is saved in chunks, each with a format version, written to a new file and swapped in when complete.
   The present state is saved every 30 seconds and whenever the app leaves the screen, and each event joins the history as it happens.
2. `T1.2b.2` **History kept by age (`PLT-10`).**
   The history thins with age by `PLT-10`'s rule, and the storage each world uses is reported.
3. `T1.2b.3` **Migrations and old saves (`PLT-09`, A3.6).**
   A chain of migrations, one for each change of format, never deleted, and a folder of saves from every format version that every build must open.
   A big update marks the world.
4. `T1.2b.4` **Several worlds, export and import (`TIM-08`, `PLT-08`).**
   The test screen lists the worlds: new, open, delete; switching saves the current one.
   A world exports to a file and imports back; a damaged file is refused with a message.
5. `T1.2b.5` **Pauses when closed (`TIM-05`).**
   Leaving the screen stops time and saves; reopening carries on, every activity where it was.
6. `T1.2b.6` **Deliver α1.2b (`PRC-11`).**
   The APK, the note and the checks.

**Tests:**
- doctest: a world saved and reopened at random ticks ends exactly as one never closed (`TIM-05`, `PLT-07`).
- doctest: every old save opens (`PLT-09`); export and import, and a damaged file refused (`PLT-08`); thinning keeps what it must (`PLT-10`).
- gdUnit4: the list of worlds (`TIM-08`).

**On the phone:**
- Make two worlds of markers and switch between them.
- Close the app in the middle of a run and reopen it: everything is where it was.
- Export a world to a file, delete it, and import it back.

### α1.2c Scenes and the benchmark

**Goal:** tests as seeded scenes in the cloud, the repeat check, and the phone measuring itself; then M1's report.

**Serves:** `PLT-04`, `PLT-05`, `RES-05`, `RES-06`, `RES-10`, `RES-12`, `RES-21`, `RES-22`.

**Architecture:** A11, A12.

**Tasks:**

1. `T1.2c.1` **Scenes (`RES-21`, `RES-09`, `RES-13`, A11).**
   Scene files in `data/scenes/` state their items, seed, runs, time limit, budget and pass rule.
   A headless runner, the same library with no Godot, runs several at a time and writes a report.
2. `T1.2c.2` **Switch-off runs (`RES-10`).**
   Switches exist only in test builds, and every run using one says so in its report and its world.
3. `T1.2c.3` **The repeat check (`RES-05`).**
   A scene runs twice, on one thread and on four with a save and reopen between, and must end identical; `tools/check.sh` runs it.
4. `T1.2c.4` **Test worlds on the phone (`PLT-05`).**
   A scene's world, saved as it ends, opens on the phone, marked as a test world with its switches.
5. `T1.2c.5` **Oddities (`RES-12`).**
   Long runs flag a crash, memory creeping up, a save that won't open, and a tick over its budget.
6. `T1.2c.6` **The benchmark and the self-check (`PLT-04`, `PRC-11`).**
   One tap runs the benchmark: frame times, ticks a second, memory, battery and heat over a timed run, ending with a short code to send.
   The first start of each version runs a self-check of a few seconds (the same bits, a save and reopen), and shows a code if anything fails.
7. `T1.2c.7` **M1's report (`RES-06`, `RES-22`).**
   A page: what was added, the test results, where the tests ran and the computing they used, the phone's numbers, and M2's plan for your OK.
8. `T1.2c.8` **Deliver α1.2c, and α1.2's independent review (`PRC-11`, `PRC-09`).**
   The APK, the note, the checks, and the review of the whole of α1.2.

**Tests:**
- The runner's own tests: a scene passes and fails by its rule (`RES-21`).
- The repeat check (`RES-05`); a switch shown in the report (`RES-10`).
- A planted oddity of each kind is flagged (`RES-12`); the benchmark's code decodes (`PLT-04`).

**On the phone:**
- Tap Benchmark and leave the phone alone for about 10 minutes, then send the code.
- Open the test world made by a scene in the cloud.
- Read M1's report, and reply OK or say what to change.

## M2 The graphics engine

**Goal:** the engine that draws everything, grown from the bake-off into the game (A4, A5; research 03, 04, 13):
- the low-resolution picture, the camera locked to its pixels, outlines and lit edges;
- three bands of light with real shadows, and the light of the hours and the seasons;
- the materials, grass and leaf cards, and water;
- the art made by code, with the model kit;
- the camera's gestures.

It is built and judged on a test land made by hand, since the world comes next; the bake-off prototype is deleted once the engine has replaced it.

**Serves:** `PRE-01`, `PRE-02`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-30`, `PRE-31`, `PRE-33`, `PRE-42`, `PRE-43`, `PRE-46`, `PLT-02`, `PLT-04`, `VIS-14`.

**You will see:**
- A small scene at your reference look: a camp under a cliff, with trees, bushes, rocks, grass, a stream, hides and a fire.
- It goes through dawn, day, dusk and night and the four seasons, crisp and steady as you drag, pinch and turn, in portrait and landscape.
- The model sheet: every shape in the kit.
- This is the quality gate: you judge the scene against your reference pictures, and nothing is built on the engine until you are happy with it.

**Risks:**
- The PowerVR chip's driver: measured in P1 and at every step.
- The outline pass's cost at the phone's resolution.
- Pixels crawling in a turn or a zoom: `PRE-22`'s fix is chosen at your review.

## M3 The world

**Goal:** whole worlds, made from a seed in the order of real causes and tuned until the map looks like the art guide (A6, A4.3, A4.4; research 05):
- from plates to biomes, with rivers, lakes, seas, soils and deposits;
- detail made on demand, the same every time;
- the ground drawn at every distance, up to the globe;
- the climate and the weather running over it.

**Serves:** `WLD-01`, `WLD-02`, `WLD-03`, `WLD-06`, `WLD-07`, `WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`, `WLD-12`, `WLD-13`, `WLD-14`, `WLD-15`, `WLD-16`, `WLD-17`, `WLD-22`, `WLD-24`, `WLD-26`, `WLD-27`, `WLD-30`, `WLD-34`, `PRE-03`, `PRE-25`, `PRE-29`.

**You will see:**
- Make a world, and choose among the best three.
- Zoom in one gesture from the globe to a cliff face.
- Rivers running to the sea, the weather moving over the land, and the seasons turning it.
- The ground sliced open, showing its rock, soil and water.

**Risks:**
- Making a world within about 3 minutes on the phone (`WLD-11`).
- Believable land needs tuning by eye.
- Memory for detailed areas.

## M4 Things and living nature

**Goal:** the world's matter and life, before any people (A7; research 06, 13):
- materials and things, with their shapes, characteristics, wear, simple physics, timers and traces;
- fire;
- plants and animals as catalogue entries, placed by rules, growing, grazing, hunting and dying as totals on the world's cells, the herds drawn from their counts.

**Serves:** `MAT-01`, `MAT-02`, `MAT-03`, `MAT-08`, `MAT-09`, `MAT-10`, `MAT-11`, `MAT-18`, `MAT-19`, `MAT-20`, `RCK-14`, `RCK-15`, `RCK-21`, `RCK-22`, `RCK-23`, `WLD-18`, `WLD-28`, `WLD-31`, `WLD-32`, `PRE-27`, `PRE-28`, `PRE-35`, `PRE-44`.

**You will see:**
- Plants leafing, flowering, fruiting and dying back through the seasons.
- Herds grazing and migrating, wolves hunting, carcasses rotting where they fell.
- Lightning starting a fire that runs through dry grass until rain or a river stops it, leaving ash.
- A card for any plant, animal or thing you tap.

**Risks:**
- Nature's numbers staying believable over a century (`WLD-18`).
- The world alone reaching its speed (`TIM-07`).

## M5 People: bodies and lives

**Goal:** people with bodies that live, act and die for real reasons (A8; research 07, 08):
- needs, senses, everyday activities and the base actions on things;
- health, wounds, illness and plain care;
- pairing, birth, growing up, inheritance, ageing and death;
- choosing what to do by their needs, with the reasons kept, and walking by layered paths;
- names from their language;
- the animals near them as individuals, with bodies and simple minds.

**Serves:** `BIO-02`, `BIO-03`, `BIO-04`, `BIO-05`, `BIO-06`, `BIO-08`, `BIO-09`, `BIO-10`, `BIO-11`, `BIO-12`, `BIO-13`, `BIO-14`, `BIO-15`, `BIO-16`, `BIO-17`, `BIO-18`, `BIO-19`, `BIO-21`, `BIO-22`, `BIO-23`, `MAT-06`, `MAT-12`, `MND-07`, `MND-09`, `MND-14`, `MND-15`, `MND-16`, `MND-28`, `CUL-17`, `CUL-18`, `CUL-30`, `RCK-26`, `TIM-07`, `TIM-09`, `PRE-27`, `PRE-28`, `PRE-35`, `PRE-37`, `PRE-44`, `RES-14`, `RES-23`.

**You will see:**
- The first bands at their camps: gathering, drinking, carrying, resting and sleeping by a found fire.
- People falling ill and getting better, hurt by a fall or a wolf, pairing, giving birth, growing old and dying.
- Each with a name, and on their card what they are doing and why.
- Little figures of blocks, each activity with its own movement.

**Risks:**
- Thousands of people at speed on the phone (`MND-15`, `TIM-07`).
- Paths for many walkers at once.

## M6 Minds

**Goal:** the inner life (A8; research 07):
- personality, mood and feelings, memories and dreams;
- knowledge with its source, and beliefs about causes;
- plans and ambitions;
- relationships, social acts and talk.

**Serves:** `MND-01`, `MND-02`, `MND-03`, `MND-04`, `MND-05`, `MND-08`, `MND-09`, `MND-12`, `MND-18`, `MND-19`, `MND-20`, `MND-21`, `MND-22`, `MND-23`, `MND-24`, `MND-26`, `MND-27`, `MND-29`, `MND-30`, `MND-32`, `MND-33`, `CUL-24`, `PRE-14`, `PRE-45`, `VIS-17`.

**You will see:**
- Moods and reasons you can read in the details of anyone's mind.
- Friendships, quarrels and comfort.
- Talk about food, danger, places and each other, its topic in a bubble.
- Plans made, kept or dropped, and dreams remembered.

**Risks:**
- The cost of a full mind for everyone (`MND-14`).
- Keeping explanations short and true (`PRN-13`).

## M7 Crafts and discovery

**Goal:** the heart of the arc (A8, A9; research 07, 08):
- blueprints and their several routes, and the chains they make;
- skill from experience, and the four routes to discovery;
- teaching and imitation, and knowledge lost and found again;
- the reality rules that make crafts work or fail.

**Serves:** `MAT-04`, `MAT-07`, `MAT-21`, `MAT-22`, `MND-06`, `MND-10`, `MND-11`, `MND-13`, `BIO-20`, `BIO-23`, `CUL-01`, `CUL-02`, `CUL-03`, `CUL-16`, `RCK-01`, `RCK-02`, `RCK-03`, `RCK-06`, `RCK-07`, `RCK-10`, `RCK-11`, `RCK-12`, `RCK-13`, `RCK-16`, `RCK-25`, `RES-02`, `RES-03`, `RES-17`, `RES-23`, `RES-24`, `MOM-01`, `MOM-02`, `MOM-09`.

**You will see:**
- Someone strikes flint and finds a sharp edge; the skill spreads by watching and teaching, or dies with its last holder.
- Fire made by drilling, food cooked, cord twisted, tools hafted, clothes and huts made.
- Each discovery traced back to its moment.

**Risks:**
- Discoveries stalling: the sharp-stone test watches for it (`RSK-01`).
- Blueprints giving absurd results: the trials watch for it (`RSK-06`).

## M8 Culture and society

**Goal:** peoples with their own words, customs, beliefs, rites, art and music, made by what happens to them (A9; research 08):
- kin and marriage, leaders and specialists;
- sharing, trade, feuds and raids;
- bands splitting into peoples with territories;
- tame animals.

**Serves:** `CUL-05`, `CUL-06`, `CUL-07`, `CUL-08`, `CUL-09`, `CUL-10`, `CUL-11`, `CUL-12`, `CUL-17`, `CUL-19`, `CUL-20`, `CUL-21`, `CUL-22`, `CUL-23`, `CUL-26`, `CUL-27`, `CUL-29`, `CUL-30`, `CUL-31`, `CUL-32`, `CUL-34`, `MND-31`, `WLD-33`, `RCK-24`, `MOM-04`, `MOM-06`, `MOM-11`.

**You will see:**
- Bands splitting into named peoples with their own words and ways.
- Taboos, rites and myths that grew from real events; paintings and songs.
- Trade, marriages, feuds and raids.
- A wolf pup raised in a camp.

**Risks:** culture forming too fast, too slowly, or the same in every world (`CUL-33`, `RSK-19`).

## M9 The game

**Goal:** the surface you play (A10; research 09, 10, 11):
- your powers as nature;
- time following your zoom, the story director and live moments;
- the book of ages, worded by the phone's own writer;
- every screen and card;
- the sound.

**Serves:** `GOD-02`, `GOD-03`, `GOD-04`, `GOD-05`, `GOD-06`, `GOD-07`, `GOD-08`, `GOD-09`, `GOD-10`, `GOD-11`, `GOD-12`, `GOD-13`, `TIM-01`, `TIM-02`, `TIM-03`, `TIM-04`, `TIM-10`, `TIM-11`, `TIM-15`, `PRE-05`, `PRE-06`, `PRE-07`, `PRE-08`, `PRE-09`, `PRE-10`, `PRE-13`, `PRE-15`, `PRE-16`, `PRE-17`, `PRE-18`, `PRE-19`, `PRE-32`, `PRE-33`, `PRE-34`, `PRE-35`, `PRE-37`, `PRE-39`, `PRE-40`, `PRE-41`, `SND-01`, `SND-02`, `SND-03`, `SND-06`, `SND-07`, `SND-08`, `SND-11`, `SND-12`, `VIS-15`, `PLT-03`, `MOM-03`, `MOM-07`.

**You will see:**
- Long-press for your powers: lightning, storms, dreams, fortune and revelation.
- Time slowing as you zoom in; live moments when something matters, and skip to the next.
- The book of ages in flowing prose.
- The camp's sounds, and the murmur of its talk.

**Risks:**
- The phone's writer, Gemini Nano, has an interface still in alpha that may change; the pattern sentences stand on their own.
- The writer straying from the facts (`PRE-17`).

## M10 The whole arc

**Goal:** the full launch catalogue and the pace (research 00):
- pottery, herding, farming, villages and copper;
- the chances tuned until worlds go from caves to copper in a few hundred years, at a watchable speed.

**Serves:** `TIM-19`, `CUL-28`, `CUL-33`, `MAT-23`, `RCK-04`, `RCK-08`, `WLD-33`, `RES-07`, `RES-12`, `RES-16`, `RES-25`, `MOM-08`, `MOM-12`, `VIS-14`.

**You will see:**
- Pots, herds, fields, villages and the first copper, each in its own time.
- The pace tests' charts in the report.

**Risks:**
- The pace needing long tuning (`RSK-26`).
- The phone's speed with thousands of people (`TIM-07`).
