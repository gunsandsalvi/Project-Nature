# Kindling: implementation plan

The order in which Kindling is built.
It follows `PROJECT.md` (what the game must be) and `ARCHITECTURE.md` (how it is built), and cites both: items by ID (`TIM-16`), parts of the architecture by section (A3.4).
It follows the guide of research 00, the way real teams work, and builds bottom up, as you asked: the foundations first, then the graphics engine, the world, living nature, people, minds, crafts, culture, and the game itself last.
Every step ends with a build on your phone, and every milestone with a report you review (`RES-06`, `RES-22`).

Only the next milestone is planned in detail: the foundations (M1).
The later ones are outlines (their goal, the items they deliver, what you will see), each detailed when it comes next, from what the earlier ones taught.
The plan holds only work still to do: a step leaves it when it is done, and the code, which names the items it implements, is the record (`CLAUDE.md`, rule 3).

## Status (5 October 2026)

- The ten milestones were approved by you on 4 October 2026, with their proposals, now decided in `PROJECT.md` (`SCP-16`, `MIL-08` to `MIL-17`); the early steps are tried rather than played (`PRN-09`, `SCP-03`, `PRC-11`).
- The risks were tried first, as research 00 advises, and that work closed on 5 October 2026, as you asked: its answers are decisions in the architecture, and its evidence, numbers and lessons are in `LESSONS.md`.
  None of its code is carried into production, which writes its own.
- Production begins with the foundations (M1), scoped for the whole game and for what may come after it, and planned in full below from research 18: eleven steps in five alphas, each ending with a build on your phone.
- The first alpha, α1.1 (the workshop, then numbers and chance), is delivered as 20101 and 20102 and closed by the builder's review; your phone's lines from its self-check are still to come.
- α1.2a, the clock and the calendar, is delivered as 20201; your phone's top speed on its Time page is still to come.
- α1.2b, catalogues and tuning, is delivered as 20202, and α1.2 is closed by the builder's review; its note stays in the repository rather than being republished, as you asked on 5 October 2026.
- α1.3a, entities and events, is delivered as 20301: the world's clockwork, with a crowd of markers walking, resting and sleeping, and the phone's self-check running a small world.
- α1.3b, activities and islands, is delivered as 20302: markers meet and greet, and islands give exactly the one-thread world on any number of threads; for the crowd's light events one worker is faster, so it runs on one (A3.3).
- α1.3c, the crowd on your phone, is delivered as 20303, and α1.3 is closed by the builder's review: 10,000 markers drawn from the world's newest snapshot at any speed, each exactly where the world has it, with the heat governor and the counters (A3.8, A3.9).

## How to use this plan

For the AI agent building a step:
1. **Pick** the first step in the status table that is not done, unless the owner names another.
2. **Read**, in this order:
   - the step's section and its milestone's lines;
   - every architecture section and item it cites, and the research notes those sections cite;
   - the traps in `LESSONS.md` for the tools the step uses;
   - the last note (`dist/NOTE.md`), for anything the owner reported.
3. **Set up** the session:
   - run `tools/setup.sh` if tools are missing;
   - fetch main by name (`git fetch origin +refs/heads/main:refs/remotes/origin/main`), since a shallow cloud clone otherwise never sees it;
   - work on the session's own branch, brought up to date with main first.
4. **Build the tasks in order.**
   Each ends with its code and tests passing, and a commit naming the task and the items it touches (see Conventions).
   - If a task can't be built as the architecture says, stop that task and add a **Conflict:** note at the end of the step's section, with the reason and the smallest change that works.
     Carry on with that change, and update the architecture in the same branch.
   - If a step will clearly take more than about 6 hours, split it at a task boundary into two lettered steps, each still ending with a build.
5. **Deliver** (`PRC-11`, A2.3):
   - the signed APK in `dist/`, when the step has one;
   - the note: what is new, what to try, what is rough, the items touched and the links, published at the note's link.
6. **Check:** `tools/check.sh --deliver` passes (`PRC-10`).
7. **Review** (`PRC-09`), as you set on 5 October 2026:
   - The builder reviews each lettered step itself: each new test made to fail once, the numbers checked against how they were measured, and the pictures looked at.
   - At the last step of each numbered alpha, the builder reviews the whole alpha the same way, against its sections and the items it claims.
   - The independent review comes only at a milestone's end (9).
   - The pull request says which review approved it.
8. **Join and tell:**
   - merge into main;
   - take the step's section and status row out of this plan, moving anything the code doesn't hold, such as a decision or a measured number, into the architecture;
   - tell the owner in two or three lines, with the note's link.
9. **At a milestone's end:**
   - one independent subagent verifies the whole milestone.
     It is given only the milestone's diff, its sections as they stood when it began, and the items it claims.
     It also judges, as a pixel artist and a game art director, how the game actually looks: from pictures it draws itself of every screen the milestone touched, at each hour, on their own merits and independently of the art book, as you asked on 4 October 2026;
   - the milestone report (`RES-06`), with the phone's numbers (`PLT-04`), the contact sheet (`PRE-31`) once there is a game to show, and what went right and wrong;
   - the next milestone detailed here, and its architecture sections written in full;
   - the owner approves both with the milestone review (`RES-22`).

For you, the owner:
- Each step's **On the phone** says what to open and what you should see; a step that ran in the cloud shows its charts in the app's Reports page.
  Each build installs over the last (`PLT-06`).
- Reply with anything that looks wrong: it goes into the next step.
- At each milestone's end you get a short report to accept or send back, with the next milestone's plan for your OK.

## Conventions

- **Milestones:** M1 to M10, `MIL-08` to `MIL-17` in `PROJECT.md`, in order.
- **Steps:** each milestone is built as numbered alphas, and each alpha as lettered steps of a few hours.
  α1.2b is milestone 1, alpha 2, step b; it is written `a1.2b` in file and branch names.
- **Tasks:** `T<step>.<n>`, such as `T1.2b.3`; they never change once a step starts.
- **Commits:** `T1.1c.2: keyed chance (TIM-16, A3.5)`, then the attribution lines the session requires.
  A commit that changes `PROJECT.md` also carries the `Changed:` lines of `PRC-07`.
- **Items in code** (`CLAUDE.md`, rule 3):
  - C++ and shaders name what they implement in a doc comment (`/// Implements TIM-16, see A3.5`), and each test what it checks on a line above it (`// checks: TIM-16`);
  - GDScript does the same with `## Implements` and `# checks:`;
  - catalogue entries and scenes list theirs in `checks = [...]`.

  `python3 tools/filecheck.py where TIM-16` then finds them all.
- **Scenes:** TOML files in `data/scenes/`.
  Each states, before its first run, the items it checks, its seed, its runs (about 20 where chance matters), its time limit, its budget in session-hours and its pass rule (`RES-09`, `RES-13`).
- **Numbers:** every tunable number lives in a catalogue or a tuning file, never in code (`PRN-14`).
- **Version codes:** as A2.3 sets out, so α1.1a is 20101; the version name is the step's name.
- **Hours:** the builder's estimate for building, testing and delivering a step; your time is the few minutes of trying it.

## Definition of done (every step)

1. Every task is built with its tests, and committed.
2. `tools/check.sh --deliver` passes (`PRC-10`): the checks that exist at that step, from formats and lints to the file, commit and coverage checks (`PRC-12`) and the APK check.
3. The step's scenes pass at their stated rules, and every earlier quick scene still passes, once scenes exist.
4. The phone's numbers are recorded in the note, once a step measures them; a slowdown of more than 10% against the previous step carries a reason or a fix.
5. A changed save format brings its migration and an old save that must still open (`PLT-09`), once saves exist.
6. The review approves (`PRC-09`).
7. The APK, when the step has one, is built, signed and committed, and the note is written and published (`PRC-11`).
8. The step's section and status row have left this plan.

## Rules every alpha keeps

These items hold for the whole build rather than being delivered by one step.
Every step keeps them, the reviews check them, and the coverage check counts them as served:
- **Principles:** `PRN-16`, `PRN-01`, `PRN-02`, `PRN-07`, `PRN-05`, `PRN-12`, `PRN-17`, `PRN-03`, `PRN-04`, `PRN-10`, `PRN-13`, `PRN-06`, `PRN-15`, `PRN-11`, `PRN-09`, `PRN-14`.
- **Scope and non-goals:** `SCP-02`, `SCP-03`, `SCP-15`, `SCP-04`, `SCP-05`, `SCP-06`, `SCP-07`, `SCP-08`, `SCP-09`, `SCP-10`, `SCP-11`, `SCP-12`, `SCP-17`, `SCP-18`, `SCP-19`, `SCP-20`, `SCP-21`.
- **Process:** `PRC-02`, `PRC-03`, `PRC-04`, `PRC-06`, `PRC-07`, `PRC-09`, `PRC-10`, `PRC-11`, `PRC-12`.
- **Testing:** `RES-01`, `RES-09`, `RES-13`, `RES-18`, `RES-19`.

## What the plan asks of you

- **Each step:** install it when you like, and reply with anything that looks wrong.
- **At each milestone's end:** its report, and the next milestone's plan, for your OK (`RES-22`).
- **Once, if not yet done:**
  - register the package `dev.kindling.app` and the release certificate's fingerprint (`android/keys/release-cert.sha256`) in your hobbyist developer account (`PLT-06`);
  - make `main` the default branch on GitHub (Settings, General, Default branch).
- **Choices by eye and ear:** the ground of the cards and the book, and whether reading text should be larger (`PRE-35`), when the first cards are built (M4); the murmur's voice (`SND-03`) and the drums (`SND-02`) at M9.

## Status

| Step | Title | Milestone | Hours | Status |
|---|---|---|---|---|
| α1.4a | Saves and the journal | M1 | 6 | Next |
| α1.4b | Worlds, export and updates | M1 | 5 | Planned |
| α1.5a | Scenes and runs | M1 | 6 | Planned |
| α1.5b | The benchmark and M1's end | M1 | 6 | Planned |
| The slice | The vertical slice | After M1 | | Proposed, for your OK |
| M2 to M10 | Outlines below | M2 to M10 | | Detailed when each comes next |

## M1 Foundations

**Goal:** the data structures and plumbing everything else stands on, scoped for the whole game and for what may come after it (A2, A3; research 18):
- the app and its delivery, and the simulation library;
- numbers, time and chance, with the same bits on the phone and in the cloud;
- entities, events, activities, catalogues, commands and snapshots;
- saves, scenes and the phone's benchmark.

It is built in five alphas, each ending with a build on your phone and closed by the builder's own review; the milestone ends with the independent review and its report for your review.
The demonstration content (markers that walk, meet and greet in camps) lives in its own source, `data/demo/`, and never enters the game's own catalogue (`MAT-16`).

**Serves:** `TIM-01`, `TIM-05`, `TIM-08`, `TIM-10`, `TIM-14`, `TIM-16`, `TIM-17`, `TIM-18`, `PLT-01`, `PLT-03`, `PLT-04`, `PLT-05`, `PLT-06`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `MAT-05`, `MAT-13`, `MAT-14`, `MAT-17`, `RES-05`, `RES-06`, `RES-09`, `RES-10`, `RES-12`, `RES-13`, `RES-21`, `RES-22`, `PRC-10`, `WLD-13`.

**You will see:**
- A self-check on every start: the phone computing the same bits as the cloud.
- The calendar running through the 60-day year at the speed you set, the real speed shown.
- A crowd of 10,000 markers walking, meeting and greeting smoothly at any speed, while the simulation counts its events on its own threads.
- Worlds saved, closed, killed and reopened exactly, several at once, one exported to a file and imported again.
- A benchmark of one tap and about 20 minutes, ending in a short code.

**Risks:**
- The C++ build for Android: proven on your phone (`LESSONS.md`).
- Islands, the way to run the world on four cores with exactly the one-core result, are designed from the literature but not yet built (research 18): if they fail, one core runs the world, with the same results, until they work.
- The cost of a crowd per frame: about 0.26 ms in the cloud, measured on your phone by the benchmark.

### α1.4a Saves and the journal

**Goal:** a world that is always saved: a snapshot every 30 seconds and whenever the app leaves the screen, your commands written at once, and after any crash or kill the world reopens where it was and catches up exactly.

**Serves:** `TIM-05`, `PLT-07`, `PLT-10`, `RES-05`.

**Architecture:** A3.7, A3.2, A3.8.

**Tasks:**

1. `T1.4a.1` **Files that survive anything (`PLT-07`).**
   The I/O thread and its interface (the real one, and a fake for tests that can drop unsynced writes as a power cut would); writing by new file, sync, rename and folder sync; logs framed by length, type, sequence and checksum, cut at the first bad record.
2. `T1.4a.2` **Snapshots (`TIM-05`, `PLT-07`, `RES-05`).**
   zstd 1.5.7, pinned; the snapshot's header, chunks and trailer; each component written through its descriptor in id order, the queue's live events, the clock, the sources and name lists; reading verifies every hash; the state's digest after reading equals the one before writing, and the world carries on exactly.
3. `T1.4a.3` **The journal, history and catching up (`PLT-07`, `PLT-10`).**
   Commands synced at once, with pause marks; history appended in yearly segments; the 30-second save (a copy at an event, compressed and written on other threads); the save when the app leaves the screen; recovery from the newest whole snapshot, a damaged one moved aside, commands re-applied at their moments while the world catches up under a short note, its re-made history compared with what was written.
4. `T1.4a.4` **The kill test (`PLT-07`, `TIM-05`).**
   A demonstration command (call a camp home), so the journal has something to keep; in the cloud the tool killed at 100 random moments and reopened each time; deliver.

**Tests:**
- Every world opens after each of the 100 kills, and ends identical to an unbroken run.
- Damaged files (cut short, a flipped bit, zeros) refused and set aside; the previous snapshot used.
- A save at any second, inside a window or not, reopened, gives the same history as never closing (`TIM-05`).
- Passes if all pass.

**On the phone:** on Crowd, tap a camp to call it home; swipe the app away, reopen it: the world is where it was, the camp still walking home; open the self-check for the last save's time.

### α1.4b Worlds, export and updates

**Goal:** several worlds kept and switched exactly, one exported to a file and imported again, old saves opened by new versions, and a warning before the phone is full.

**Serves:** `TIM-08`, `PLT-08`, `PLT-09`, `PLT-10`.

**Architecture:** A3.7, A3.6.

**Tasks:**

1. `T1.4b.1` **Several worlds (`TIM-08`).**
   Each world's folder and `world.toml`; a Worlds page to make, open, switch, rename and delete them, deleting only after you confirm; switching saves one and opens the other exactly.
2. `T1.4b.2` **Export and import (`PLT-08`).**
   The `.kindling` file, with a checksum for each part; export and import through Android's file picker, streamed; an import checked as it arrives and refused with a message naming the damage.
3. `T1.4b.3` **Updates and old saves (`PLT-09`).**
   A version in every chunk with its upgrades, and named migrations recorded in the save; small or big decided by the world digest, the note saying which; a corpus of exported worlds from each alpha opened by every build; the previous version's last save kept until a world has run an hour.
4. `T1.4b.4` **Space and thinning (`PLT-10`).**
   Free space checked at each save, with the warning and the question of which worlds to delete; the history thinned at year boundaries by the fixed rule (every event for 25 years, then what each kind keeps); each world's size by part; deliver.

**Tests:**
- Three worlds switched in turn each open exactly where they were left.
- An exported world, imported, runs on identically; a damaged file is refused.
- The corpus opens and carries on; a big update keeps the history readable.
- A 30-year world's history keeps every event of its last 25 years and only kept kinds before.
- Passes if all pass.

**On the phone:** open Worlds: make three, switch among them, export one and import it as a copy, and delete the copy.

### α1.5a Scenes and runs

**Goal:** the test machinery every later milestone uses: scenes stated before they run, runs over 20 seeds, switches only in tests, oddities flagged, long runs that resume exactly, the repeat check before anything joins, and reports you can read.

**Serves:** `RES-09`, `RES-10`, `RES-12`, `RES-13`, `RES-21`, `RES-05`, `RES-06`, `PLT-05`, `PRC-10`.

**Architecture:** A17, A3.1, A3.7.

**Tasks:**

1. `T1.5a.1` **Scenes and their pass rules (`RES-21`, `RES-09`, `RES-13`).**
   Scenes as TOML in `data/scenes/`, each stating its items, seed, runs, time limit, budget and pass rule before its first run; `kindling scene` runs many worlds at once on the cloud's cores and counts the rule; a failed rule re-runs on 20 fresh seeds and is judged on all 40.
2. `T1.5a.2` **Switches and oddities (`RES-10`, `RES-12`).**
   Test switches compiled only into test builds and recorded in the world and the report; oddities flagged by expected ranges and "never" rules, and a crash, creeping memory or a save that won't reopen.
3. `T1.5a.3` **Long runs and the repeat check (`PLT-05`, `RES-05`, `PRC-10`).**
   Runs that keep checkpoints and resume after a restart as if never stopped; `tools/check.sh` runs one scene and one benchmark world twice, on one core and on four with a stop and resume between, and they must end identical; a test world marked as one, which opens on the phone.
4. `T1.5a.4` **Reports (`RES-06`).**
   A run's or a scene's report as a page with its charts and its ranges ("in 18 of 20 worlds"); the app's Reports page shows the last ones; deliver.

**Tests:**
- A scene with one of each oddity planted flags every one (`RES-12`).
- A run stopped with its session and resumed in a new one ends identical to an unbroken run (`PLT-05`).
- A scene whose rule fails once passes or fails by the 40-run rule (`RES-13`).
- Passes if all pass.

**On the phone:** open Reports to read the cloud's last scene report, and open the test world it ran, marked as a test world.

### α1.5b The benchmark and M1's end

**Goal:** the phone benchmark, one tap and about 20 minutes ending in a short code, whose end states match the cloud's; and M1's report for your review.

**Serves:** `PLT-04`, `PLT-01`, `RES-05`, `RES-06`, `RES-22`.

**Architecture:** A18.1, A3.9, A17.

**Tasks:**

1. `T1.5b.1` **Telemetry (`PLT-04`).**
   The device class reads battery and power rails, the cores' clocks, our threads' CPU time, memory and every frame's interval by our own measure (on time within a period plus half a refresh; a stall counted for every period it skipped), and marks frames and batches for the phone's System Tracing.
2. `T1.5b.2` **The scenarios (`PLT-04`, `RES-05`).**
   A18.1's scenarios: the calendar alone, 10,000 markers at real speed and at top speed with the camera touring, the same pinned to the middle cores, a sweep through the zoom stops' speeds, saves with an export and a reopening, and a still camera; each scenario's digest at its set date; the cloud runs the same ones headless with the same digests.
3. `T1.5b.3` **The code (`PLT-04`).**
   A version, a fixed layout and a checksum in Crockford base32, in groups the chat apps leave alone; the layout written once and read by both the app and the cloud's decoder; the pass lines stated before the first run (`RES-09`).
4. `T1.5b.4` **M1's report (`RES-06`, `RES-22`).**
   What was added and what you can try, the tests and where they ran, the phone's numbers, what went right and wrong; the next step's plan for your OK; deliver.

**Tests:**
- A code made in the cloud decodes after its letters' case is changed, its lines broken and its dashes swapped, and one wrong letter is caught.
- The cloud's headless scenarios give the digests the phone must match.
- Passes if all pass.

**On the phone:** unplug the phone, turn on flight mode, open Bench and tap Run; leave it for about 20 minutes, then copy the code into the chat.

## The vertical slice (proposed: after the foundations)

*Proposed, for your OK* (`MIL-18`, `PRC-08`): with pre-production closed, the slice is built on the foundations, before the graphics engine, and planned in detail at M1's end.

One band at a cliff camp through a day, at the art book's look, on your phone, built in the real architecture: the first piece of the game, and the quality bar for the rest of production (research 00).

**Its must-haves:**
- **The foundations** of M1 underneath: the real stack, the core and the checks (A2, A3, A17).
- **The land:** a stretch of a small generated island round a limestone cliff, a stream and a meadow, made by the area rules from the seed (A7.5, A7.6).
- **The look:** the art book's person, close camp and camp zoom stops, from dawn through noon and dusk to night with the fire lit, by the methods the architecture chose (A4, A5, A6).
- **Living things:** two kinds of tree, grass and one herd of deer placed by keyed chance (A9).
- **The band:** about 25 people with bodies and needs, choosing a dozen everyday activities by utility, the reasons on each card (A10, A11).
- **The screen:** time controls, tapping a person for their card, both orientations (A15).
- **Sound:** the camp's ambience, the fire and the murmur (A16).
- **Every kind of check** of A17, golden pictures included.

**Not in the slice:** discovery, culture, the world beyond the island, the zoom out past the camp, your powers, the book of ages.

**Done when:**
- you judge the slice against the art book, and accept it as the bar for production;
- its frame and graphics times and its heat are measured on your phone;
- a second of each thing (a tree, an animal, an activity, a hut) is made after the first, and the time it took re-estimates the later milestones (research 00).

## M2 The graphics engine

**Goal:** the engine that draws everything, grown from the slice's drawing to all the art book shows up close (A4, A5, A6; research 04, 05, 17):
- the low-resolution picture, the camera locked to its pixels, outlines and lit edges;
- light in clean steps with real shadows, through the hours and the seasons;
- every material, grass and leaf cards, and water;
- the whole model kit, made by code;
- the camera's gestures.

**Serves:** `PRE-01`, `PRE-02`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-30`, `PRE-31`, `PRE-33`, `PRE-42`, `PRE-43`, `PRE-46`, `PLT-02`, `PLT-04`, `VIS-14`.

**You will see:**
- The art book's scenes as the game draws them: a camp under a cliff, a river valley, a winter steppe, a lakeshore, through dawn, day, dusk and night and the four seasons, crisp and steady as you drag, pinch and turn, in portrait and landscape.
- The model sheet: every shape in the kit.
- This is the quality gate: you judge the scenes against the art book, and nothing is built on the engine until you are happy with it.

**Risks:**
- The PowerVR chip's driver: everything the look needed so far ran on your phone (`LESSONS.md`); measured again at every step.
- The cost of the outline pass and of mirrored water at the phone's resolution.

## M3 The world

**Goal:** whole worlds, made from a seed in the order of real causes and tuned until the map looks like the art book's zoom stops (A7, A8; research 06, 07):
- from plates to biomes, with rivers, lakes, seas, soils and deposits;
- detail made on demand, the same every time;
- the ground drawn at every distance, up to the globe;
- the climate and the weather running over it.

**Serves:** `WLD-01`, `WLD-02`, `WLD-03`, `WLD-06`, `WLD-07`, `WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`, `WLD-12`, `WLD-13`, `WLD-14`, `WLD-15`, `WLD-16`, `WLD-17`, `WLD-22`, `WLD-24`, `WLD-26`, `WLD-27`, `WLD-30`, `PRE-03`, `PRE-25`, `PRE-29`.

**You will see:**
- Make a world, and choose among the best three.
- Zoom in one gesture from the globe to a cliff face.
- Rivers running to the sea, the weather moving over the land, and the seasons turning it.
- The ground sliced open, showing its rock, soil and water.

**Risks:**
- Making a world within about 3 minutes on the phone (`WLD-11`): a first version made three in 9.3 s (`LESSONS.md`), so the stages can grow richer.
- Believable land needs tuning by eye.
- Memory for detailed areas.

## M4 Things and living nature

**Goal:** the world's matter and life, before any people (A9, A12; research 08, 11, 17):
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
- Nature's numbers staying believable over a century (`WLD-18`): a first version held them (`LESSONS.md`).
- The world alone reaching its speed (`TIM-07`).

## M5 People: bodies and lives

**Goal:** people with bodies that live, act and die for real reasons, grown from the slice's band (A10, A11; research 09, 10):
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
- Thousands of people at speed on the phone (`MND-15`, `TIM-07`): a thousand simple minds kept 6 game years a real minute (`LESSONS.md`).
- Paths for many walkers at once.

## M6 Minds

**Goal:** the inner life (A11; research 10):
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

**Goal:** the heart of the arc, tuned to its windows (A12; research 10, 11):
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

**Goal:** peoples with their own words, customs, beliefs, rites, art and music, made by what happens to them (A13; research 12):
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

**Risks:** culture forming too fast, too slowly, or the same in every world (`CUL-33`, `RSK-19`): a first version formed it from causes alone (`LESSONS.md`).

## M9 The game

**Goal:** the surface you play (A14, A15, A16; research 13, 14, 15):
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
- The phone's writer, never yet tried: its interface is in beta and may change; the pattern sentences stand on their own (`PRE-37`).
- The writer straying from the facts (`PRE-17`).

## M10 The whole arc

**Goal:** the full launch catalogue and the pace (research 00, 11):
- pottery, herding, farming, villages and copper;
- the chances tuned until worlds go from caves to copper in a few hundred years, at a watchable speed.

**Serves:** `TIM-19`, `CUL-28`, `CUL-33`, `MAT-23`, `RCK-04`, `RCK-08`, `WLD-33`, `RES-07`, `RES-12`, `RES-16`, `RES-25`, `MOM-08`, `MOM-12`, `VIS-14`.

**You will see:**
- Pots, herds, fields, villages and the first copper, each in its own time.
- The pace tests' charts in the report.

**Risks:**
- The pace needing long tuning (`RSK-26`).
- The phone's speed with thousands of people (`TIM-07`).
