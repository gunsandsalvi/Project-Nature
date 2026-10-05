# Kindling: implementation plan

The order in which Kindling is built.
It follows `PROJECT.md` (what the game must be) and `ARCHITECTURE.md` (how it is built), and cites both: items by ID (`TIM-16`), parts of the architecture by section (A3.4).
It follows the guide of research 00, the way real teams work: pre-production first, a prototype for each risk and then a vertical slice, then production bottom up, as you asked: the foundations first, then the graphics engine, the world, living nature, people, minds, crafts, culture, and the game itself last.
Every step ends with a build on your phone, and every milestone with a report you review (`RES-06`, `RES-22`).

Only the next milestone is planned in detail: pre-production.
The later ones are outlines (their goal, the items they deliver, what you will see), each detailed when it comes next, from what the earlier ones taught.
The plan holds only work still to do: a step leaves it when it is done, and the code, which names the items it implements, is the record (`CLAUDE.md`, rule 3).

## Status (5 October 2026)

- Rewritten from the research redone on 4 October and the art book you accepted, after the first version of 3 October (git keeps it at `f881525`).
- **Approved by you on 4 October 2026,** with its proposals, now decided in `PROJECT.md`: pre-production as a milestone of its own (`MIL-18`), then the ten bottom-up milestones (`SCP-16`, `MIL-08` to `MIL-17`), the old seven retired; the early steps are tried rather than played (`PRN-09`, `SCP-03`, `PRC-11`); the documents and what comes next (`PRC-04`, `PRC-08`).
- The workshop (α0.1a) is delivered: the setup, checks and build scripts, and the prototype app with its self-check.
- P1 The look (α0.2a) is done: outline C, its flicker fixed, and the "ease" crawl fix, which you left to me (A4.1).
- P2 A full scene (α0.2b) is done: a busy camp at night with three fires holds 60 frames a second at close and camp zoom (A18.1).
- P3 The kit (α0.2c) is delivered, and your comments and the independent review's findings are fixed and delivered in α0.3a for your look: warmer firelight and flickering flames, grey daytime smoke, gentler turns, figures in their own clothes with a pose at every step, brush windbreaks and coursed lean-tos, and P2's people and camp zoom. The art book's dusk sun stays, as you asked for no change but faster discoveries.
- P4 Discovery pace (α0.3a) passes, and is reviewed and delivered: with every discovery window halved twice, as you asked for faster discoveries and the first village about an hour into play, tuning alone brings flakes within 2 years and fire in Years 2 to 8, and no value holds the pace on a knife's edge (A12, `TIM-19`). It was re-tuned on 5 October for the second halving; the next build carries its report.
- P5 The same bits (α0.4a) passes on your phone: its chip gives the cloud's digest on one thread and four, as x86-64 and arm64 under qemu do, and P6's thousand minds ended their 3,546 game days on your phone exactly as in the cloud (A3.4).
- P6 A thousand minds (α0.4b) passes on your phone: a thousand people hold 6.0 game years a real minute on four cores over 10 minutes, with no slowing from heat; the target is at least 1, hoping for 2 to 3 (`TIM-07`, A11).
- P7 World generation (α0.5a) passes on your phone: three worlds in 9.3 seconds and settling in 6.5, against `WLD-11`'s 3 minutes and 1, the very same worlds as the cloud's (A7.6).
- P8 The zoom (α0.5b) was built and delivered for your phone's Measure: one pinch from the globe to a person over P7's world, through rings of ground made on worker threads and a map that bends onto the globe (A8.1, A8.4, A8.5).
  Its second round (α0.5c), after your verdict on 5 October, is for your look, from space down to the valley: a planet at every scale, never unrolled; one tree of ground morphing smoothly; clouds from the climate that the descent passes among; the sea by its depth; the land textured with what can be seen from space; pixels finer up close; each in variants for you to choose.
  The camp and closer are done for pre-production, as you said on 5 October: trees, small plants and the camp from the kit on the new ground. What they lack is in the notes for production (`NOTES.md`), with your observations. Measure on your phone comes with α0.6a's build.
- P9 Ecology (α0.6a) passes in the cloud: in 20 worlds left alone for 100 years, every species stayed within 0.66 and 1.10 of its settled total and in every biome it lived in, with 82 to 99 big plant eaters for each hunter; round each world's start region numbers boomed and crashed from 0.21 to 2.31 with the weather (A9). Its report is on the app's Reports page.
- P10 Culture from causes (α0.6b) passes in the cloud: in 20 worlds of three bands run 100 years, a custom came inside its window in all 20, a shared spirit in 20, a rite a band keeps in 15 and a band split in 17, each from the events behind it (A13); `CUL-33` now gives customs a window of a year, with your OK.
- P11 The director (α0.6c) passes in the cloud: watching 20 test worlds of 100 years from the globe, it slowed time 9.5 times an hour within its budget, caught all 208 named discoveries and every death of those followed, and every world ended identical with it on and off (A14). Signs, as these worlds have them, rarely come true, and with 20 years between ages fire could never begin one, so ages now have no set length (`PRE-39`), with your OK.
- P12 The interface (α0.7a) is built for your phone and passes its tests in the cloud: a stand-in world under the interface the art book's plates show, drawn crisp in art pixels; one gesture reader, none of whose gestures is read as another; every control 48 dp and in a thumb's reach; and the card and the book on five grounds for you to choose from (A15).
- P14 Sound (α0.7c) is built for your phone and passes its tests in the cloud: the art book's close camp as a map you walk on, every sound from its place through Godot's 3D players, with the shelter's echo and the cliff's muffling; base sounds made by code, the fire and wind made live, and talk strung from a bank of syllables rendered in the cloud, shifted for age and feeling; 32 sounds at most in `SND-01`'s shares, the rest in hums; and a reel for your ears (A16). P13 The writer waits, as you said on 5 October.

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
   Each ends with its code and tests passing, and a commit naming the task and the items it touches (see Conventions).
   - If a task can't be built as the architecture says, stop that task and add a **Conflict:** note at the end of the step's section, with the reason and the smallest change that works.
     Carry on with that change, and update the architecture in the same branch.
   - If a step will clearly take more than about 6 hours, split it at a task boundary into two lettered steps, each still ending with a build.
5. **For a prototype step** (pre-production, research 00):
   - it answers one question, with numbers measured where the step says, and lets nothing in that the question doesn't need;
   - it is time-boxed at its hours: well past them, stop and report what was learned instead of pushing on;
   - its code lives in `prototypes/` and is thrown away; what carries over is the answer, written into the architecture where the section says *to prove*.
6. **Deliver** (`PRC-11`, A2.3):
   - the signed APK in `dist/`, when the step has one;
   - the note: what is new, what to try, what is rough, the items touched and the links, published at the note's link.
7. **Check:** `tools/check.sh --deliver` passes (`PRC-10`).
8. **Review** (`PRC-09`):
   - The builder reviews each lettered step itself: each new test made to fail once, the numbers checked against how they were measured, and the pictures looked at.
   - At the last step of each numbered alpha, one independent subagent verifies the whole alpha.
     It is given only the alpha's diff, its sections as they stood when it began, and the items it claims.
     It also judges, as a pixel artist and a game art director, how the game actually looks: from pictures it draws itself of every screen the alpha touched, at each hour, on their own merits and independently of the art book, as you asked on 4 October 2026.
   - The pull request says which review approved it.
9. **Join and tell:**
   - merge into main;
   - take the step's section and status row out of this plan, moving anything the code doesn't hold, such as a decision or a measured number, into the architecture;
   - tell the owner in two or three lines, with the note's link.
10. **At a milestone's end:**
    - the milestone report (`RES-06`), with the phone's numbers (`PLT-04`), the contact sheet (`PRE-31`) once there is a game to show, and what went right and wrong;
    - the next milestone detailed here, and its architecture sections written in full;
    - the owner approves both with the milestone review (`RES-22`).

For you, the owner:
- Each step's **On the phone** says what to open and what you should see; a step that ran in the cloud shows its charts in the app's Reports page.
  Each build installs over the last (`PLT-06`).
- Reply with anything that looks wrong: it goes into the next step.
- At each milestone's end you get a short report to accept or send back, with the next milestone's plan for your OK.

## Conventions

- **Milestones:** M0 is pre-production (`MIL-18`); M1 to M10 are production, `MIL-08` to `MIL-17` in `PROJECT.md`, in order.
- **Steps:** each milestone is built as numbered alphas, and each alpha as lettered steps of a few hours.
  α1.2b is milestone 1, alpha 2, step b; it is written `a1.2b` in file and branch names.
- **Prototypes** are named P1 to P14 in the order they are built, riskiest first; each is one lettered step of M0.
- **Tasks:** `T<step>.<n>`, such as `T1.2b.3`; they never change once a step starts.
- **Commits:** `T1.1c.2: keyed chance (TIM-16, A3.5)`, then the attribution lines the session requires.
  A commit that changes `PROJECT.md` also carries the `Changed:` lines of `PRC-07`.
- **Items in code** (`CLAUDE.md`, rule 3):
  - C++ and shaders name what they implement in a doc comment (`/// Implements TIM-16, see A3.5`), and each test what it checks on a line above it (`// checks: TIM-16`);
  - GDScript does the same with `## Implements` and `# checks:`;
  - catalogue entries and scenes list theirs in `checks = [...]`.

  `python3 tools/filecheck.py where TIM-16` then finds them all.
  Prototype code implements nothing for good, so it carries no `Implements` lines: its README names the items its question is about.
- **Scenes:** TOML files in `data/scenes/`.
  Each states, before its first run, the items it checks, its seed, its runs (about 20 where chance matters), its time limit, its budget in session-hours and its pass rule (`RES-09`, `RES-13`).
- **Numbers:** every tunable number lives in a catalogue or a tuning file, never in code (`PRN-14`).
- **Version codes:** as A2.3 sets out, so α0.2b is 10202; the version name is the step's name.
- **Hours:** the builder's estimate for building, testing and delivering a step; your time is the few minutes of trying it.

## Definition of done (every step)

1. Every task is built with its tests, and committed.
2. `tools/check.sh --deliver` passes (`PRC-10`): the checks that exist at that step, from formats and lints to the file, commit and coverage checks (`PRC-12`) and the APK check.
3. The step's scenes pass at their stated rules, and every earlier quick scene still passes, once scenes exist.
4. The phone's numbers are recorded in the note, once a step measures them; a slowdown of more than 10% against the previous step carries a reason or a fix.
5. A changed save format brings its migration and an old save that must still open (`PLT-09`), once saves exist.
6. A prototype's answer is written into the architecture, with the numbers and how they were measured.
7. The review approves (`PRC-09`).
8. The APK, when the step has one, is built, signed and committed, and the note is written and published (`PRC-11`).
9. The step's section and status row have left this plan.

## Rules every alpha keeps

These items hold for the whole build rather than being delivered by one step.
Every step keeps them, the independent review checks them, and the coverage check counts them as served:
- **Principles:** `PRN-16`, `PRN-01`, `PRN-02`, `PRN-07`, `PRN-05`, `PRN-12`, `PRN-17`, `PRN-03`, `PRN-04`, `PRN-10`, `PRN-13`, `PRN-06`, `PRN-15`, `PRN-11`, `PRN-09`, `PRN-14`.
- **Scope and non-goals:** `SCP-02`, `SCP-03`, `SCP-15`, `SCP-04`, `SCP-05`, `SCP-06`, `SCP-07`, `SCP-08`, `SCP-09`, `SCP-10`, `SCP-11`, `SCP-12`, `SCP-17`, `SCP-18`, `SCP-19`, `SCP-20`, `SCP-21`.
- **Process:** `PRC-02`, `PRC-03`, `PRC-04`, `PRC-06`, `PRC-07`, `PRC-09`, `PRC-10`, `PRC-11`, `PRC-12`.
- **Testing:** `RES-01`, `RES-09`, `RES-13`, `RES-18`, `RES-19`.

## What the plan asks of you

- **During pre-production,** a few minutes per phone prototype:
  - install it from the note's link, try what the note says, and copy the short code it shows into the chat;
  - at P1 and P3, say how close the look is to the art book, and choose the fix for crawling pixels (`PRE-22`);
  - at P2, leave the phone alone for about 10 minutes while it measures its heat;
  - at P13, leave it about an hour while the writer is timed.
- **At pre-production's end:** the prototypes' report and the vertical slice's plan, then your verdict on the slice itself, which sets the quality bar for production; its frame and graphics times are measured then on your phone, as you asked.
- **Once, before the first build:**
  - register the package `dev.kindling.app` and the release certificate's fingerprint (`android/keys/release-cert.sha256`) in your hobbyist developer account (`PLT-06`);
  - make `main` the default branch on GitHub (Settings, General, Default branch).
- **Each step:** install it when you like, and reply with anything that looks wrong.
- **Choices by eye and ear:** the fix for crawling pixels at P1 (`PRE-22`); the murmur's voice (`SND-03`) and the drums (`SND-02`) at M9.

## Status

| Step | Title | Milestone | Hours | Status |
|---|---|---|---|---|
| α0.2c | P3 The kit | M0 | 6 | Fixed after your comments and the review; your look in the α0.3a build |
| α0.3a | P4 Discovery pace | M0 | 6 | Passes; reviewed and delivered |
| α0.4a | P5 The same bits | M0 | 4 | Passes on your phone; the α0.4 review next |
| α0.4b | P6 A thousand minds | M0 | 6 | Passes on your phone; the α0.4 review next |
| α0.5a | P7 World generation | M0 | 6 | Passes on your phone |
| α0.5b | P8 The zoom | M0 | 6 | Done for your look; Measure on your phone in α0.6a's build |
| α0.6a | P9 Ecology | M0 | 4 | Passes in the cloud; its report in the app (α0.6a) |
| α0.6b | P10 Culture from causes | M0 | 5 | Passes in the cloud; its report in the app (α0.6b) |
| α0.6c | P11 The director | M0 | 3 | Passes in the cloud; its report in the app (α0.6c) |
| α0.7a | P12 The interface | M0 | 4 | Built and passing its tests in the cloud; for your look and feel on your phone (α0.7a) |
| α0.7b | P13 The writer | M0 | 4 | Not started |
| α0.7c | P14 Sound | M0 | 4 | Built and passing its tests in the cloud; for your ears and Measure on your phone (α0.7c) |
| α0.8a | What the prototypes found | M0 | 4 | Not started |
| The slice | The vertical slice | M0 | detailed in α0.8a | Not started |
| M1 to M10 | Outlines below | M1 to M10 | | Detailed when each comes next |

## M0 Pre-production

**Goal:** proof before production, as research 00 sets out:
- a throwaway prototype for each risk, riskiest first, each answering one question on your phone or in the cloud;
- then the vertical slice: one band at a cliff camp through a day, at the art book's look, built in the real architecture, on your phone.

The prototypes that share a build share one app on your phone, with a menu of what each measures.
Already done in pre-production: the design (`PROJECT.md`), the research (`research/`), the art bible and the art book (research 05, `art/book/`), the bake-off that chose Godot (research 01), the architecture (`ARCHITECTURE.md`), and the workshop (α0.1a): the tools and the prototype app.

**Serves:** `PLT-01`, `PLT-02`, `PLT-03`, `PLT-04`, `PLT-06`, `PLT-07`, `PRC-10`, `PRC-11`, `PRC-12`, `VIS-14`, `PRE-01`, `PRE-02`, `PRE-03`, `PRE-17`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-26`, `PRE-27`, `PRE-28`, `PRE-29`, `PRE-30`, `PRE-31`, `PRE-32`, `PRE-33`, `PRE-34`, `PRE-35`, `PRE-37`, `PRE-39`, `PRE-41`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`, `MAT-18`, `RES-02`, `RES-03`, `RES-05`, `RES-06`, `RES-21`, `RES-22`, `TIM-01`, `TIM-02`, `TIM-03`, `TIM-07`, `TIM-16`, `TIM-17`, `TIM-19`, `MND-06`, `MND-09`, `MND-11`, `MND-13`, `MND-14`, `MND-15`, `RCK-01`, `RCK-02`, `BIO-09`, `BIO-21`, `WLD-02`, `WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`, `WLD-12`, `WLD-13`, `WLD-18`, `WLD-30`, `WLD-31`, `WLD-32`, `CUL-05`, `CUL-06`, `CUL-30`, `CUL-33`, `CUL-34`, `SND-01`, `SND-03`, `SND-08`, `SND-11`, `SND-12`.

**You will see:**
- On your phone, one prototype app growing step by step: the art book's camp drawn by Godot, a busy camp at night, the model kit, a thousand minds at speed, a world made and one pinch from the globe to a person, a card and a book page, the writer's sentences, and the camp's sounds.
- From the cloud, reports shown in the app: whether discovery can be tuned to its pace, whether nature and culture hold, and whether the director keeps its budget.
- Then the vertical slice: a band living a day at their cliff camp at the art book's look, the first piece of the real game.

**Risks:**
- A prototype answering no: that is the point of asking first; its fallback is in A18.2, and anything that changes `PROJECT.md` comes to you.
- The PowerVR driver: every rendering feature is exercised in the first phone builds (A4.3).
- Prototypes growing into products: each is time-boxed and thrown away; only the slice is built to last.

### α0.2c P3 The kit

**Goal:** answer: do the kit's shapes, built by code at load, read well as the art book's sheets do, with the figure's movements and a hut in two materials, at noon and at night?

**Serves:** `PRE-27`, `PRE-30`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`.

**Architecture:** A4.1, A6.1, A6.2, A6.3, A6.4.

**Tasks:**

1. `T0.2c.1` **Shapes from parameters (`PRE-46`).**
   About 10 shared shapes, 2 plants and 1 animal built at load as `ArrayMesh` from catalogue-like parameters.
2. `T0.2c.2` **The figure and five movements (`PRE-27`, `PRE-44`).**
   The block figure with walk, carry, knap, scrape and rest as key poses, stepped about 10 times a second, with a face of eyes and a mouth.
3. `T0.2c.3` **A hut in two materials (`PRE-42`, `PRE-43`).**
   The same layout in birch bark and in reed, by per-copy colour and wear.
4. `T0.2c.4` **The model sheet (`PRE-46`).**
   Every shape in two materials, at noon and at night with three fires, by Movie Maker mode, beside the art book's sheets.
5. `T0.2c.5` **Shadows from every light (`PRE-30`, `PRE-44`), as you asked.**
   People and things cast shadows from each fire as well as the sun, and stand in them.
6. `T0.2c.6` **Volumetric smoke (`PRE-30`), as you asked.**
   Smoke rising from a fire as a lit volume, not flat puffs: it gathers under an overhang and flows out past its edge, glows from the fire below, and what stands in it shows through.

**Tests:**
- The model sheet beside the art book's people and kit sheets.
- The frame time of the sheet's scene on the phone.
- Passes if you judge the shapes readable and close to the art book, within the frame's budget.

**On the phone:** open "The kit", turn the sheet, switch noon and night, and say what reads well and what doesn't.

**Conflict:** the sheet's pictures come from the app's own picture option, not Movie Maker mode, which records only at the project's base size (A6.4); and they are not set beside the art book's sheets, since you asked me to stop comparing pictures with it. The kit has eleven shared shapes: a lean-to joined the ten, so a fire burns under a roof and its smoke and light can be seen to meet one.

### α0.3a P4 Discovery pace

**Goal:** answer the riskiest design question (`RSK-01`): can tuning alone make sharp flakes come within their window and fire within its own, with the world's own rules?

**Serves:** `TIM-19`, `RES-02`, `RES-03`, `RES-06`, `MND-06`, `MND-11`, `MND-13`, `RCK-01`, `RCK-02`.

**Architecture:** A12, A11.

**Tasks:**

1. `T0.3a.1` **One band, simple minds (`MND-11`, `MND-06`).**
   A headless run of one band of about 25 with needs, simple choice, noticing and hunches, on land with flint, granite, dry wood and fires from lightning.
2. `T0.3a.2` **The stone and fire blueprints (`RCK-01`, `RCK-02`).**
   Flakes only from stone that flakes; friction fire with soft dry wood; each discovered only by accident, a hunch or copying.
3. `T0.3a.3` **Teaching and loss (`MND-13`).**
   Watching and teaching spread a skill; it dies with its last holder.
4. `T0.3a.4` **Tune and test (`RES-02`, `RES-03`, `TIM-19`).**
   The sharp-stone test over 20 runs; the discovery factors tuned until it passes and fire lands in its window; how sensitive the pace is to each factor.
5. `T0.3a.5` **The Reports page (`RES-06`).**
   The prototype app gains a Reports page that shows the charts a cloud prototype makes, starting with this one's; the later cloud prototypes add theirs.

**Tests:**
- The sharp-stone pass rule (`RES-03`) over 20 runs, and fire's window (`TIM-19`).
- Passes if a tuning meets both, and no single factor holds the pace on a knife's edge.

**On the phone:** the prototype app's Reports page shows the runs' discovery years and how the pace moves with each factor; the note links the full report.

**Conflict:** you asked for faster discoveries on 4 October 2026, and on 5 October for the first village about an hour into play, so `TIM-19`'s windows are halved twice and everything tied to the years moved with them (`RES-02`, `RES-03`, `RES-07`); P4 tunes to them: flakes within 2 years, which is also the sharp-stone test's bar, and fire in Years 2 to 8. The windows are dates (`TIM-14`), so fire's Years 2 to 8 begin a year in; P4 first read them a year late. Fire is counted at its first in a world of 3 or 4 bands, as `TIM-19` counts a step, not in one band's run. A knife's edge is read as a change of a quarter either way breaking a step's rule; a value that breaks one only when halved or doubled is reported as a strong lever, since so large a change is meant to move the pace.

### α0.4a P5 The same bits

**Goal:** answer: do your phone and the cloud, on one thread and on four, end a world identically?

**Serves:** `RES-05`, `TIM-16`.

**Architecture:** A3.4, A3.5, A2.2.

**Tasks:**

1. `T0.4a.1` **A toy world in C++ (`TIM-16`).**
   Walkers on events with keyed chance, our own sine, exponent and logarithm, floating-point sums in fixed chunks on one thread and on four.
2. `T0.4a.2` **Checksums at checkpoints (`RES-05`).**
   The whole state hashed every game day, in the cloud on x86-64 and on arm64 under qemu, and on your phone through the prototype app.

**Tests:**
- The hashes compared across x86-64, arm64 under qemu, the phone, one thread and four.
- Passes if every hash matches.

**On the phone:** open "Same bits", tap Run, and copy the code into the chat: it holds the phone's hashes.

**Conflict:** godot-cpp has tagged no release for Godot 4.7, so its 4.5 release, which Godot 4.7 loads, is pinned (A2.2). P5's keyed chance takes any prototype's list of draws, and its hashes and its arm64 check are files of their own, so the prototypes after it share them rather than copying them.

### α0.4b P6 A thousand minds

**Goal:** answer: do a thousand simple minds with needs, choice, talk and paths keep a game year a real minute on your phone at held speed?

**Serves:** `MND-09`, `MND-14`, `MND-15`, `TIM-07`, `TIM-17`.

**Architecture:** A3.3, A3.9, A11.

**Tasks:**

1. `T0.4b.1` **A thousand people on events (`TIM-17`).**
   Activities that end at events, decisions only then; needs running down at their own rates.
2. `T0.4b.2` **Choice by utility (`MND-09`).**
   About 50 actions scored by response curves, the top reasons kept.
3. `T0.4b.3` **Talk and paths (`MND-14`).**
   Talk passing a few topics; paths by regions, cached cluster paths, then A* inside.
4. `T0.4b.4` **Measure at held speed (`TIM-07`, `MND-15`).**
   A 10-minute run on four cores, game years a real minute after the phone has warmed, ending with a code.

**Tests:**
- The measurement: game years a real minute at held speed, and the cost of each part.
- Passes at 1 or more game years a minute, aiming for 2 to 3 (`TIM-07`).

**On the phone:** open "A thousand minds", tap Run, put the phone down for about 10 minutes, then copy the code into the chat.

**Conflict:** inside a cluster, trips follow fields of distances from each entrance, made at the start, rather than A*, and one cache of paths between clusters' parts is shared by every thread, merged between windows: with A* and a cache for each thread, paths took four fifths of the time, and four cores ran no faster than one (A11). People choose one activity at a time, with no planner on top, which P6's question doesn't need. P5 and P6 ship in one build, α0.4b, to save a delivery.

### α0.5a P7 World generation

**Goal:** answer: how long do a candidate world and the settling run take on your phone?

**Serves:** `WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`.

**Architecture:** A7.2, A7.3, A7.4, A7.6.

**Tasks:**

1. `T0.5a.1` **The stages in C++ (`WLD-09`).**
   Plates, uplift and stream-power erosion, Priority-Flood, climate, soils and plant types, at coarse and full size, deterministic in fixed chunks.
2. `T0.5a.2` **Candidates (`WLD-10`).**
   About 20 coarse candidates scored with logged reasons, the best few at full size.
3. `T0.5a.3` **Settling (`WLD-08`).**
   A first settling run, timed.
4. `T0.5a.4` **Timed on the phone (`WLD-11`).**
   Each stage's time at held speed, with a code; the maps as pictures beside the art book's world map.

**Tests:**
- The timings on the phone; the world map's picture.
- Passes if candidates and the best three fit about 3 minutes, and settling about 1 more (`WLD-11`).

**On the phone:** open "World generation", tap Make, wait for the three maps, then copy the code into the chat.

**Conflict:** the best few are made again at full size from their candidate's worn land, the full grid's finer relief laid on it and worn 10 steps more, rather than from nothing, so the world offered is the candidate judged. The linear model of rain over mountains runs once for each of four winds, the trades and the westerlies of each half, blended where the belts meet. Glaciers, currents beyond a broad variation of warmth, and the plants' and herds' real rules wait for production, so settling's rules are stand-ins of about the work production's will do, for its time. The maps are set beside the art book's in the note rather than on the phone.

### α0.5b P8 The zoom

**Goal:** answer: does one pinch from the globe to a person stay smooth at every stop on your phone?

**Serves:** `PRE-03`, `PRE-29`, `WLD-02`, `TIM-01`.

**Architecture:** A8.1, A8.2, A8.3, A8.4, A8.5, A8.6, A8.7.

**Tasks:**

1. `T0.5b.1` **Levels of detail (`PRE-03`).**
   On P7's world: rings of near chunks, coarse chunks to about 10 km, the map mesh and the globe, on Godot's RenderingServer, with dithered fades and a moving origin.
2. `T0.5b.2` **The map look and the globe (`PRE-29`, `WLD-02`).**
   Flat cover colours, shaded hills lit from high up, rivers as lines, the shore's bright line; the map bent onto the globe, the seam under the ice.
3. `T0.5b.3` **The pinch (`TIM-01`).**
   One gesture from the globe to a person over land never visited, measured at every stop, with the time to make a full area.
4. `T0.5b.4` **Second round, from space to the valley (`PRE-03`, `PRE-29`, `WLD-02`).**
   After your verdict on 5 October: a planet at every scale; one tree of ground morphing smoothly; clouds from the climate the descent passes among, and light that follows the sun; the sea by its depth; the land textured with what can be seen from space; pixels finer up close; each in variants for you to choose.
5. `T0.5b.5` **Second round, the camp and closer (`PRE-03`).**
   Trees, grass and the camp's things on the new ground, at the art book's look.

**Tests:**
- The measurements: the share of frames on time through the pinch, and the area's time.
- Passes if at least 97% of frames are on time at every stop and a full area is in within about a second.

**On the phone:** open "The zoom", tap Measure, then pinch yourself from the globe to a person and back, and copy the code into the chat.

**Conflict:** the stops' scales are set for the phone's 336 art pixels across, so the world map shows the whole 2,000 km around and never an empty edge. The near and middle levels are two rings each, the outer one half as fine, as clipmaps do, and the middle ground reaches 16 km, not 10, so the valley's tall portrait picture is covered. Trees beyond the near rings are drawn as cards out to 900 m, A8.3's middle step, brought into P8 because without them the camp showed a disc of trees. Rivers show only on the map: the near and middle ground show lakes but not yet the rivers' courses, which come with water in production. Measure's pinch runs at a steady pace; your own pinch is for the feel.
The second round replaces the rings, the map mesh and their dithered hand-overs with one tree of chunks that morph into each other (CDLOD), and the map's bend with a sphere at every scale, as you asked (A8.1, A8.4); rivers now show at every scale. The descent ends beside the nearest river draining at least 300 km² to the start region, so the close stops show water. `PRE-22`'s pixels that grow with the zoom and `PRE-29`'s vivid land lit by the sun are decided with your OK on 5 October; the land's and the sea's smooth light in variant B would need `PRE-01` changed too, if you choose it.
The zoom starts in stages, the weather, the ground, then the sky, each noted in a file first: α0.5c stopped on your phone as the world was made, and if a stage stops it again, the next start says which, in a light mode without the clouds and the land's finest detail.

### α0.6a P9 Ecology

**Goal:** answer: do the totals of plants and animals stay believable for 100 years with nobody in the world?

**Serves:** `WLD-18`, `WLD-30`, `WLD-31`, `WLD-32`.

**Architecture:** A9.

**Tasks:**

1. `T0.6a.1` **Nature on world cells (`WLD-31`, `WLD-32`).**
   Plant cover growing by season, animal totals by Damuth's law at a sixth of natural density (`WLD-30`), grazing and hunting by predator and prey rules, driven by the weather.
2. `T0.6a.2` **A hundred years, twenty worlds (`WLD-18`).**
   Headless runs, each species' total charted.

**Tests:**
- Passes if every species stays within half and twice its total, and hunters to prey within `WLD-18`'s range, in the 20 worlds.

**On the phone:** the Reports page shows each species' total over the hundred years; the note links the full report.

**Conflict:** the worlds run at the candidates' size, cells of about 4 km, since the rules hold per km² and 20 worlds at full size would take over an hour; a run at full size is left to production's scenes. A 30-year run stands in for making each world directly in its present-day state (`WLD-08`), since 10 settling years from a rough start left a drift. Fish, birds and bears wait for production: they need rivers, seas and seasons of their own.

### α0.6b P10 Culture from causes

**Goal:** answer: do customs, a spirit, a rite and a band split arise inside their windows, each from its own cause?

**Serves:** `CUL-05`, `CUL-06`, `CUL-30`, `CUL-33`, `CUL-34`.

**Architecture:** A13.

**Tasks:**

1. `T0.6b.1` **Two or three bands, simple minds (`CUL-30`).**
   Bands with needs, talk and memories of events, run headless for a hundred years.
2. `T0.6b.2` **Rules from coincidences (`CUL-05`, `CUL-06`, `CUL-34`).**
   Customs from a band's own cases, beliefs and rites from good and bad outcomes after acts.
3. `T0.6b.3` **Windows and causes (`CUL-33`).**
   About 20 runs: when each first appears, and the chain of events behind it.

**Tests:**
- Passes if each appears inside its window in at least half the runs, and every one traces back to its own cause.

**On the phone:** the Reports page shows when each first appeared, and one run's story told from its events.

**Conflict:** `CUL-33` gave customs no window, so P10 measured them against a year, which it now gives, with your OK. What the project file leaves open was set by sense and by the runs: bands start at 22 to 30, hunters do something unusual before a hunt one time in a hundred, and a hunter dies in a hunt about once in 250 years. A rite a band keeps is one most adults have credited, or done as the band's way, a year long; the project file gives no test of keeping.

### α0.6c P11 The director

**Goal:** answer: does the director keep its budget on recorded worlds while catching every named discovery, without changing them?

**Serves:** `TIM-02`, `TIM-03`, `PRE-39`.

**Architecture:** A14.

**Tasks:**

1. `T0.6c.1` **Recognisers over recorded worlds (`PRE-39`).**
   Story-sifting patterns over P4's and P10's event logs, half-matched patterns as signs.
2. `T0.6c.2` **Its budget and its hands off (`TIM-02`, `TIM-03`).**
   The slowdowns and live moments it would ask for, against its budget; the same worlds run with it on and off must end identical.

**Tests:**
- Passes if the budget holds, no named discovery is missed, and the worlds end identical with it on and off.

**On the phone:** the Reports page shows the director's budget and the moments it caught.

**Conflict:** neither recorded world alone has both discoveries and culture, so each test world joins P10's valley and P4's three bands side by side, untouched by each other. Its top speed, 5 game years a real minute, is `TIM-07`'s least for 100 to 300 people, though the test worlds hold 450 to 1,250 people by Year 100. Of `TIM-02`'s signs, a predator stalking and two hostile groups in sight wait for animals and peoples these worlds don't have. The scores, the bar and the rest after each slowdown are set by sense, for tuning with you.

### α0.7a P12 The interface

**Goal:** answer: do thumb reach, gestures and crisp pixel text work in both orientations on your phone?

**Serves:** `PRE-32`, `PRE-33`, `PRE-34`, `PRE-35`, `PLT-02`.

**Architecture:** A15.

**Tasks:**

1. `T0.7a.1` **A stand-in world with the time controls (`PRE-32`, `PRE-34`).**
   The art book's interface plates in Godot: the world full screen, the controls in the bottom third in portrait and beside the world in landscape.
2. `T0.7a.2` **One card and one book page (`PRE-35`).**
   In the pixel fonts at whole multiples of their size: the plain font for reading, the handwriting for big titles.
3. `T0.7a.3` **The gesture reader (`PRE-33`).**
   Our reader on raw touches, and a scripted test of every gesture.

**Tests:**
- gdUnit4: the scripted gestures, none misread (`PRE-33`); every control at least 48 dp.
- Passes if no gesture is misread, every control falls in thumb reach, and text stays crisp at the chosen scale.

**On the phone:** open "The interface", turn the phone, tap a person, open the book page, and say if anything is hard to reach or read.

**Conflict:** the stand-in world is the art book's zoom stops at noon, painted upright and, for P12, sideways; a tap at the close stops opens Aru's card wherever it lands. As you asked on 5 October, the card and the book stand on a choice of five grounds: the art book's paper, Night, Hide, Slate and Glass. The rules moved some of the plates' controls: the live moment and the book's tabs come down within a thumb's reach, the time controls spread to 48 dp each, and they wait while a panel is open in portrait. At your phone's 1080-pixel setting an art pixel takes 3 screen pixels, about 7% smaller than the art book's 4 on the full panel.

### α0.7b P13 The writer

**Goal:** answer: how many reworded sentences pass the check, how fast does each come, and when do the quotas bite?

**Serves:** `PRE-17`, `PRE-37`, `PRE-41`.

**Architecture:** A14.

**Tasks:**

1. `T0.7b.1` **The Android plug-in's writer (`PRE-41`).**
   ML Kit's Prompt API with a fixed seed, and the Rewriting API's Rephrase as a second option, behind the plug-in.
2. `T0.7b.2` **The check (`PRE-17`, `PRE-41`).**
   100 pattern sentences and the 50 trap records, each rewording passed through the strict check.
3. `T0.7b.3` **Timed for an hour (`PRE-37`).**
   Pass rate, time per sentence, and when the quotas stop it.

**Tests:**
- Passes as a measurement: the report gives the pass rate, the time and the quota; pattern text stands alone whatever the answer (`PRE-37`).

**On the phone:** open "The writer", tap Run, leave the phone for about an hour, then copy the code into the chat.

### α0.7c P14 Sound

**Goal:** answer: do 32 voices with distance filters, a cave's reverb and the murmur play without breaks on your phone?

**Serves:** `SND-01`, `SND-03`, `SND-08`, `SND-11`, `SND-12`.

**Architecture:** A16.

**Tasks:**

1. `T0.7c.1` **A camp's sounds (`SND-11`, `SND-01`).**
   Ambience layers and one-shots from a stand-in camp, base sounds made by code at load.
2. `T0.7c.2` **Space (`SND-08`).**
   3D players with distance filters, and a cave's reverb.
3. `T0.7c.3` **The murmur (`SND-03`).**
   Syllables from a stand-in language, shifted by age and feeling.
4. `T0.7c.4` **Measured and recorded (`SND-12`).**
   The audio thread's time with 32 voices, and a reel for your ears.

**Tests:**
- Passes if the audio thread stays within its share, and you hear no breaks in the reel or on the phone.

**On the phone:** open "Sound", listen with headphones and on the speaker, and say what sounds wrong.

### α0.8a What the prototypes found

**Goal:** the prototypes' answers written into the architecture, the prototypes thrown away, and the vertical slice planned in detail for your OK.

**Serves:** `RES-06`, `RES-22`, `PLT-04`.

**Architecture:** A18.1, A18.2.

**Tasks:**

1. `T0.8a.1` **Answers into the architecture (`PLT-04`).**
   Every *to prove* line becomes the decision with its numbers; A18.1's budgets become the measured ones; each failed answer's fallback is taken, or brought to you.
   Every note in `NOTES.md` goes into the architecture or the slice's plan, or comes to you if it would change `PROJECT.md`.
2. `T0.8a.2` **The prototypes thrown away (`RES-06`).**
   `prototypes/` is emptied; what the slice needs is rebuilt there properly.
3. `T0.8a.3` **The slice planned (`RES-22`).**
   The vertical slice's steps written in full here, from the outline below and what the prototypes taught.
4. `T0.8a.4` **The report (`RES-06`).**
   What each prototype answered, what went right and wrong, and the slice's plan, for your OK.

**Tests:**
- A search of the architecture finds no *to prove* line left without its answer or a reason.

**On the phone:** the Reports page shows what each prototype answered; read the full report, and reply OK or say what to change.

### The vertical slice (outline, detailed in α0.8a)

One band at a cliff camp through a day, at the art book's look, on your phone, built in the real architecture: the first piece of the game, and the quality bar for production (research 00).

**Its must-haves:**
- **The real stack:** the Godot app with `view/` and `sim/` (A1.2), delivered and checked as every later step will be (A2, A17).
- **The core:** entities, the clock and events, keyed chance, a few catalogue entries, and a save that reopens exactly, the same bits on your phone and in the cloud (A3).
- **The land:** a stretch of a small generated island round a limestone cliff, a stream and a meadow, made by the area rules from the seed (A7.5, A7.6).
- **The look:** the art book's person, close camp and camp zoom stops, from dawn through noon and dusk to night with the fire lit, by the methods P1 to P3 chose (A4, A5, A6).
- **Living things:** two kinds of tree, grass and one herd of deer placed by keyed chance (A9).
- **The band:** about 25 people with bodies and needs, choosing a dozen everyday activities by utility, the reasons on each card (A10, A11).
- **The screen:** time controls, tapping a person for their card, both orientations (A15).
- **Sound:** the camp's ambience, the fire and the murmur (A16).
- **Every kind of check** of A17, from C++ tests to golden pictures and the phone benchmark.

**Not in the slice:** discovery, culture, the world beyond the island, the zoom out past the camp, your powers, the book of ages.

**Done when:**
- you judge the slice against the art book, and accept it as the bar for production;
- its frame and graphics times and its heat are measured on your phone;
- a second of each thing (a tree, an animal, an activity, a hut) is made after the first, and the time it took re-estimates the production milestones (research 00).

## M1 Foundations

**Goal:** the data structures and plumbing everything else stands on, grown from the slice's core to everything production needs (A2, A3):
- the app and its delivery, and the simulation library;
- numbers, time and chance, with the same bits on the phone and in the cloud;
- entities, activities, catalogues, commands and snapshots;
- saves, scenes and the phone's benchmark.

**Serves:** `TIM-05`, `TIM-08`, `TIM-14`, `TIM-16`, `TIM-17`, `TIM-18`, `PLT-01`, `PLT-03`, `PLT-04`, `PLT-05`, `PLT-06`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `MAT-05`, `MAT-13`, `MAT-14`, `MAT-17`, `RES-05`, `RES-06`, `RES-10`, `RES-12`, `RES-21`, `RES-22`.

**You will see:**
- The calendar running through the 60-day year at the speed you set.
- A crowd of 10,000 markers moving smoothly while the simulation counts its events.
- Worlds saved, closed and reopened exactly, several at once, and one exported to a file.
- A line confirming that your phone computes the same bits as the cloud.

**Risks:**
- The C++ build for Android: proven in pre-production.
- The cost of a crowd per frame: measured with the benchmark worlds.

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
- The PowerVR chip's driver: measured in P1 to P3 and at every step.
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
- Making a world within about 3 minutes on the phone (`WLD-11`): timed in P7.
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
- Nature's numbers staying believable over a century (`WLD-18`): checked in P9.
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
- Thousands of people at speed on the phone (`MND-15`, `TIM-07`): measured in P6.
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

**Goal:** the heart of the arc, tuned from what P4 found (A12; research 10, 11):
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

**Risks:** culture forming too fast, too slowly, or the same in every world (`CUL-33`, `RSK-19`): first checked in P10.

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
- The phone's writer: its interface is in beta and may change; the pattern sentences stand on their own (P13).
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
