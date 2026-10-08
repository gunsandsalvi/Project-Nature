# Kindling: implementation plan

The order in which Kindling is built.
It follows `PROJECT.md` (what the game must be) and `ARCHITECTURE.md` (how it is built), and cites both: items by ID (`TIM-16`), parts of the architecture by section (A3.4).
It builds bottom up, as you asked: the foundations first, then the graphics engine, the world, living nature, people, minds, crafts, culture, and the game itself last.
Every step ends with a build on your phone, and every milestone with a report you review (`RES-06`, `RES-22`).

Only the next milestone is planned in detail: the graphics engine (M2).
The later ones are outlines (their goal, the items they deliver, what you will see), each detailed when it comes next, from what the earlier ones taught.
The plan holds only work still to do: a step leaves it when it is done, and the code, which names the items it implements, is the record (`CLAUDE.md`, rule 3).

## Status (8 October 2026)

- The ten milestones were approved by you on 4 October 2026, with their proposals, now decided in `PROJECT.md` (`SCP-16`, `MIL-08` to `MIL-17`); the early steps are tried rather than played (`PRN-09`, `SCP-03`, `PRC-11`).
- The risks were tried first, and that work closed on 5 October 2026, as you asked: its answers are decisions in the architecture.
  None of its code is carried into production, which writes its own.
- The foundations (M1) are built, eleven steps in five alphas delivered as 20101 to 20502, and you accepted them on 6 October 2026: your phone's benchmark met all 18 of its pass lines, and its self-check matched the cloud in all seven suites.
- The vertical slice was dropped on 6 October 2026, as you asked.
- **The graphics engine (M2)** now follows the fully 2D design you approved on 8 October 2026: fixed camera, crisp pixels and rich moving light.
  You also confirmed keeping the wrap-around world and saves, with map and globe pictures drawn from one overview.
  The 3D pilot was built: 30101 to 30202 measured its drawing, 30301 delivered the four-piece art pilot, and α2.3b work added its kit, ground, river and light.
  That route was replaced by your decision on 8 October 2026; its results remain labelled 3D history, not proof of the new renderer.
- **The world (M3)** has an approved architecture and 24-delivery plan (owner OK, 8 October 2026), to build after M2 acceptance. Its implementation, phone evidence and final acceptance remain open.
- **The art lane** keeps signed-off sheets as the design standard.
  Production now starts with six engine fixtures: person, animal, tree, boulder, ground and shelter.
  Pending runtime approvals and missing seasonal shapes are resolved before bulk art.
- **The way of working,** by your word of 7 October 2026: the main session coordinates agents and writes no code or designs; it briefs them, checks their work, merges it and brings it to you, and any agent may consult GPT through Codex, for art or for deep research.
- **Still open from pre-production:** the ground for the card and the book, and whether reading text should be larger (P12); your ears on the camp's sound (P14); the heat of a busy scene over ten minutes (P2); the writer, proved when M9 builds the book (P13); discovery's pace with the whole catalogue, at M7's scenes (`RSK-01`).

## The art lane

A separate instance makes art while the builder builds the engine (A5.4, A6).
The signed-off catalogue sheets stay the design standard.
The production brief below carries the 2D decision of 8 October 2026; the old Blender-parts brief is replaced.
A reference sheet's approval is not approval of a runtime texture, its normal map or its engine result.

> You are the art lane for Kindling, a Stone Age world simulation for phones.
> Make fully 2D pixel art for a north-facing camera exactly 37° above the horizon, with the feeling of the owner's art book.
> Signed-off sheets set design, proportions, colour and detail.
>
> **First six fixtures.** Make a person, animal, tree, boulder, ground and shelter.
> Show each in the running engine at noon and dusk, on slopes, by water and under shelter, in portrait and landscape.
> Resolve pending approvals and missing seasonal shapes, including bare winter branches where needed.
> Prove height, shadows, streaming and two-phone performance before bulk production.
>
> **The bar.** Each piece stands beside its sheet as the same thing at the same quality, true size and enlarged.
> Nothing below that level reaches the owner.
> Every frame and family keeps the important silhouette and accents at its actual screen size.
>
> **GPT is the artist; you two are a design team.** Work turn by turn on each piece, critiquing and measuring it against the sheet.
> Start from what it is made of, how it lives or is used, then shape, light/dark masses, colour and important detail.
> Improve the work; discard a weak design or method when the gap is not closing.
>
> **Any free cloud tool.** Drawing, image generation/editing, pixel editors and image tools are available.
> Offline 3D rigs may produce sprite frames and normal references, followed by pixel repair.
> The game never requires their meshes, bones or Blender parts at runtime.
>
> **What the engine reads (A5.3, A5.4, A6).**
> - Three authored families at 64, 16 and 4 internal pixels a metre, with reviewed and repaired intermediate reductions and complete halving chains.
> - Neutral colour, material masks and mild normals, aligned in alpha, pivot and trim, with atlas gutters.
> - Four facings, a six-frame walk, a few work actions and a small clothing set first.
>   Test eight facings on one animal before expanding that scope.
> - Frame holds, attachment points, facings and atlas metadata through the existing validated catalogue system.
> - Quiet ground variants, drawn material edges, broad stamps and ecology-selected details.
> - Painted sunny art may be a labelled stopgap; final art carries no fixed bright sun side.
>
> Keep originals, prompts, provenance, signed-off sheets and approvals.
> Approval records distinguish design, runtime pages and engine result.
> Check Stone Age truth before every review (A5.6); no later tools, sawn wood or borrowed cultural motifs.
> If a format needs to change, resolve it with the builder before shipping.
>
> **Working.** Work in the art and art-tool areas on your own branch; the builder reviews and merges.
> Reach GPT through the art tools outside the build.
> Never print or move secrets, buy credits or put model names in committed files; at a usage limit, wait.
>
> **Each batch's report:** pieces beside their sheets, true size and enlarged, with engine stills and action clips; your verdict and GPT's; discarded attempts and why; questions for the owner.

## The catalogue

The catalogue counts all launch art, while the first six fixtures prove production.
Its signed-off sheets remain the design standard after the 2D switch.
New sheets and runtime work follow A5.3–A5.4 and A6, with batches expanding only after the fixture gates:
- **First the list:** every element the game needs, from `PROJECT.md`, in words, group by group.
- **Then each group's sheets,** made by the builder with GPT, as you set on 7 October 2026, each piece with what it needs of:
  - views from the front, the side, the back where it is not the same all round, and above, and the fixed 37° game view; shelters and caves also inside or cut through;
  - a striped scale stick of one design beside it, sized to the piece (10 cm, 1 m or 10 m), and a standing adult beside anything big;
  - the piece at each distance the game shows it;
  - each state it goes through (seasons, growth, ages and sexes, wear, fresh to dried, materials, each people's style) as a row, and each action as a strip of its key poses and frames, starting with the six-frame walk;
  - its separate parts, then the whole;
  - each surface enlarged so its texture pixels show;
  - flat even light on the front, side, back and top views, and the game's late-afternoon light on the camera views.
  - Ground, water, sky, effects and the interface take only what applies to them; a piece's views come once, its states and actions as rows; a piece the art book already shows starts from that picture.
  - What the world lays over a ground is a piece of its own, drawn once: the wear of paths and camp floors, wetness, frost and snow, and the water over a river bed; a ground's sheet shows only its own states, as you set on 7 October 2026.
  - An object's sheet is laid out like the hide tent's, as you OK'd on 7 October 2026: its flat views at one scale beside the adult and the stick, the view from above, the camera's views, its true size at each zoom, its parts, its states and its surface.
  - Every view shows the same object, with the same parts, counts and colours, as you asked the same day: it is designed once, in words and numbers, and every view is drawn to that design.
- **You sign off each piece** as its sheet is finished.
  Production takes the six fixtures first, then expands in batches after engine and phone review.
  Existing sheet approvals are kept; runtime pages, normals and engine previews receive their own review.
  Missing seasonal silhouettes and pending approvals come before bulk work.
- It is kept in `art/catalogue/`: a file of each group's pieces, and each piece's sheet.

## How to use this plan

For the AI agent building a step:
1. **Pick** the first step in the status table that is not done, unless the owner names another.
2. **Read**, in this order:
   - the step's section and its milestone's lines;
   - every architecture section and item it cites;
   - the last note (`dist/NOTE.md`), for anything the owner reported.
3. **Set up** the session:
   - run `tools/setup.sh` if tools are missing;
   - fetch main by name (`git fetch origin +refs/heads/main:refs/remotes/origin/main`), since a shallow cloud clone otherwise never sees it;
   - work on the session's own branch, brought up to date with main first.
4. **Build the tasks in order.**
   Each ends with its code and tests passing, and a commit naming the task and the items it touches (see Conventions).
   - While building, run only what the change touches: its part's build, tests and lints. The full check, which builds and tests everything seven ways, runs once, before delivery (6), as you asked on 6 October 2026, and never while code is still changing.
   - If a task can't be built as the architecture says, stop that task and add a **Conflict:** note at the end of the step's section, with the reason and the smallest change that works.
     Carry on with that change, and update the architecture in the same branch.
   - If a step will clearly take more than about 6 hours, split it at a task boundary into two lettered steps, each still ending with a build.
5. **Deliver** (`PRC-11`, A2.3):
   - the signed APK in `dist/`, when the step has one;
   - the note: what is new, what to try, what is rough, the items touched and the links, published at the note's link.
6. **Check, once:** `tools/check.sh --deliver` passes (`PRC-10`). Only a delivery's check runs the benchmark through every scenario; a check without `--deliver` leaves that one test out.
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
     It also judges, as a pixel artist and a game art director, how the game actually looks: from pictures it draws itself of every screen the milestone touched, at each hour, on their own merits, as you asked on 4 October 2026;
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
- **Process:** `PRC-02`, `PRC-03`, `PRC-04`, `PRC-06`, `PRC-07`, `PRC-09`, `PRC-10`, `PRC-11`, `PRC-12`, and your review closing each milestone (`RES-22`).
- **Testing:** `RES-01`, `RES-09`, `RES-13`, `RES-18`, `RES-19`.

## What the plan asks of you

- **Each step:** install it when you like, and reply with anything that looks wrong.
- **At each milestone's end:** its report, and the next milestone's plan, for your OK (`RES-22`).
- **Once, if not yet done:**
  - register the package `dev.kindling.app` and the release certificate's fingerprint (`android/keys/release-cert.sha256`) in your hobbyist developer account (`PLT-06`);
  - make `main` the default branch on GitHub (Settings, General, Default branch).
- **Choices by eye and ear:** the ground of the cards and the book, and whether reading text should be larger (`PRE-35`), when the first cards are built (M4); the murmur's voice (`SND-03`) and the drums (`SND-02`) at M9.
- **In M2:**
  - six runtime fixtures beside their signed-off sheets, neutral light response, flat shadow style and then terrain shadows/reveal;
  - the animal's four/eight-facing comparison, intermediate art steps, seasonal shapes and animation holds;
  - a concrete weaker phone before comparative measurement, then both phones' look and sustained 60/30 fps results;
  - how builds and art reach your phone if they exceed the committed-file limit;
  - mammoth-bone shelters may now be designed: you approved one woolly mammoth species in cold open grassland on 8 October 2026; its catalogue and art still need their usual review.

## Status

The first five 2D alphas follow the adopted build order.
Numbering continues after the old 3D route, so begun task IDs and delivered version codes are never reused.
If a build exceeds six hours, split it at a task boundary into lettered deliveries, as the conventions require.

| Step | Title | Milestone | Hours | Status |
|---|---|---|---|---|
| α2.7a | Projection and six fixtures | M2 | 6 | Engine built; art review and delivery pending |
| α2.8a | Flat shadow style and terrain prototype | M2 | 6 | Engine built; flat/terrain review and delivery pending |
| α2.9a | Zoom and streaming | M2 | 6 | Planned |
| α2.10a | World map and globe | M2 | 6 | Planned |
| α2.11a | Two-phone renderer comparison | M2 | 6 | Planned |
| α2.12a | Art scale-up, cleanup and M2's end | M2 | 6 | After the five fixture and phone gates |
| α3.1a to α3.12b | The world: 24 deliveries below | M3 | 96–144 | Plan approved; starts after M2 acceptance |
| M4 to M10 | Outlines below | M4 to M10 | | Detailed when each comes next |

## M2 The graphics engine

**Goal:** build the fully 2D graphics engine approved on 8 October 2026 (A4, A5, A6, A8):
- one fixed 37° projection, crisp world pixels and native phone controls;
- neutral art, mild normals, moving light and shadows that know receiver heights;
- slopes, cliffs, simple shelters, water and overlap reveal;
- three authored art families with reviewed reductions and bounded streaming;
- an overhead map and 2D globe disc from one overview, keeping torus rules and saves;
- measured Compatibility/Mobile rendering at 60 and 30 fps on two phones.

M1 stays as built and accepted.
Its saved demonstrations provide clock, walkers and commands; they are not a generated living world.
M2's terrain, sky, weather and overview are labelled fixtures until M3 and later systems supply real state.
The geological slice (`PRE-25`), deep-cave world records and the full living-world activity catalogue retain their later milestones.
The first five alphas prove the risky engine path; only then does art expand and unused 3D work leave the build.

**Serves:** `PRE-01`, `PRE-02`, `PRE-03`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-27`, `PRE-28`, `PRE-29`, `PRE-30`, `PRE-31`, `PRE-33`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`, `PLT-02`, `PLT-04`, `PLT-09`, `VIS-14`, `WLD-01`, `WLD-02`, `WLD-03`, `WLD-13`, `TIM-01`, `TIM-04`, `TIM-15`, `TIM-16`, `TIM-17`, `RES-05`, `RES-06`, `RES-22`.

**You will see:**
- Six pieces in the engine, beside their signed-off sheets, at noon and dusk in both orientations.
- Flat shadows, then slopes, cliffs, shelter floors, occupied-roof cutaways and quiet silhouettes behind cover.
- Visible water bed, wading, fire in sun shade, weather and seasonal shapes.
- A smooth pinch from person to camp, valley, region, world map and globe, with crisp resting pixels and the same selection and time.
- Busy scenes measured on both phones, with real frame, heat, memory and streaming evidence.

**Risks:**
- Piece order, picking and receiver-aware shadows fail on height: test them before bulk art.
- Pixels step during slow motion or jump when families change: review pans, walking and pinch release.
- Streaming or stale jobs break a view/world switch: bound work, retain parents and reject old epochs/revisions.
- The look heats the phone or misses frame lines (`RSK-30`): use the existing governor, measured visual savings and 30 fps at the same grid.
- Sheet approvals are mistaken for runtime approvals: keep the fixture gate and records separate.

### α2.7a Projection and six fixtures

**Goal:** the first 2D picture and controls on the accepted saved-run connection, with the six art fixtures ready for engine review.

**Serves:** `PRE-01`, `PRE-02`, `PRE-03`, `PRE-20`, `PRE-22`, `PRE-27`, `PRE-31`, `PRE-33`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`, `PLT-02`, `WLD-13`, `TIM-17`, `RES-05`.

**Architecture:** A3.8, A4.1, A4.2, A4.6, A4.7, A4.8, A5.3, A5.4, A6, A8.2, A8.4, A15.

**Tasks:**

1. `T2.7a.1` **The connection and projection (`PRE-02`, `WLD-13`, `TIM-17`).**
   One view controller takes one snapshot and owns its drawing copy.
   Keep activity history sampling, torus offsets, saved worlds and pace; add epoch/revision-ready drawing records.
   Replace perspective maths with the shared east/south/height projection and explicit centimetre/millimetre conversions.
2. `T2.7a.2` **The picture and controls (`PRE-01`, `PRE-03`, `PRE-22`, `PRE-33`, `PLT-02`).**
   Dedicated world SubViewport, integer nearest presentation, native UI and safe insets.
   Pan, live pinch, discrete resting steps and transform-aware picking; no twist route.
   Preserve focus, time and selection through orientation changes.
   Configure Compatibility and the canvas bindings without rebuilding the accepted extension from scratch.
3. `T2.7a.3` **Six fixtures (`PRE-20`, `PRE-27`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`).**
   Person, animal, tree, boulder, ground and shelter from signed-off sheets.
   Neutral colour, masks and mild normals; fixed-projection frames with stable pivots and few attachments.
   Four facings, a six-frame walk and initial work actions; compare eight facings on the animal.
   Check pending approvals and missing seasonal shapes; mark painted-sun stopgaps plainly.
4. `T2.7a.4` **Engine previews and delivery (`PRE-31`, `RES-05`).**
   Noon/dusk stills in portrait/landscape, walking/action clips, sheets beside runtime art and inspectable masks/normals.
   Establish new 2D golden views and colour/object/material capture hooks; keep old proof history labelled 3D.
   Deliver as 30701, with review outcomes tracked in art records.

Engine built (8 October 2026): `T2.7a.1`, `T2.7a.2` and the engine previews of `T2.7a.4` are implemented. One copied snapshot serves both the retained 3D crowd and the new 2D records; accepted activity sampling, saves, pace and M1 outcomes are preserved. The shared 37-degree projection converts exact centimetre torus offsets and explicit millimetre heights, and supplies drawing and inverse picking. The native fixture page has safe-area controls, continuous focus, integer nearest presentation, fixed source density during live pinch and a discrete release step. Each pinch is bounded to the adjacent density steps, keeping its temporary viewport area bounded; further steps remain available after release.

The six previews are labelled developer fixtures: approved-sheet crops for tree, shelter, boulder and ground, with diagram person and animal walk/work frames. Source sheets, pivots, four/eight-facing comparison, alpha inspection and colour/object/material capture passes are wired. Actual normal and material art can be inspected when supplied; those channels are currently absent. Dusk is a diagnostic tint. This is engine evidence: `T2.7a.3` art approval and seasonal shapes remain pending the art handoff. Delivery 30801 carries this engine work together with α2.8a, superseding the separate planned 30701. Rich lighting is available on Terrain in α2.8a.

Four inspected noon/dusk portrait/landscape captures establish separate Compatibility 2D goldens, and a 72-frame clip covers walk/work, facing counts, slow pan and pinch release. All sixteen frozen image passes reproduce exactly. The release extension, 67 C++ view tests, ten fixture/retained-crowd game tests and the Python golden-checker test pass. All twelve new tests were observed failing against a fault, then passing after restoration or correction. Eight accepted simulation proof suites match with one and four threads; same-bits flags, object scanning, changed-file lint and file/ID coverage pass. Phone review and the full delivery check remain with the coordinator; no APK was built.

**Tests:**
- Projection, seam offsets and inverse picking agree at known coordinates; neighbouring edges do not accumulate rounded pitch error.
- Walk sampling follows the correct interrupted activity through the seam; frame pivots hold feet and equipment.
- Resting pixels have integer presentation scale; frozen frames match; slow pans and pinch release show no permanent blur or anchor jump.
- Native controls keep readable size; orientation keeps focus, selection, time and pixel size without reopening the world.
- Same commands at the same simulation seconds give accepted M1 digests; opening old worlds changes no outcomes.
- The six engine previews match their sheets; the owner reviews noon/dusk, clips and seasonal shapes before bulk production.

**On the phone:** open the fixture page, pan and pinch, tap the person and turn the phone.
Compare each piece with its sheet at noon and dusk; watch the walk and work clips and the animal's four/eight-facing turns.
Say which pieces pass and which need repair.

### α2.8a Flat shadow style and terrain prototype

**Goal:** approve the flat shadow style while proving height-aware shadows, overlap, water and reveal on the restricted terrain fixtures.

**Serves:** `PRE-20`, `PRE-21`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-28`, `PRE-30`, `PRE-31`, `PRE-33`, `PLT-04`, `WLD-13`.

**Architecture:** A4.3, A4.4, A4.5, A4.6, A4.8, A5.5, A6.4, A8.1, A18.1.

**Tasks:**

1. `T2.8a.1` **Flat light and shadows (`PRE-20`, `PRE-21`, `PRE-30`).**
   Shared sun/sky/local-light function, normals calibration and custom sun visibility.
   Bounded CPU mask preparation, static revision caches and moving-body shadows.
   Show contact, several sun directions, soft long dusk shadows and fire; obtain the flat style review before terrain approval.
2. `T2.8a.2` **Terrain, ordering and reveal (`PRE-23`, `PRE-24`, `PRE-28`, `PRE-33`).**
   Floor height field, slope, cliff joins, front/rear shelves and one simple shelter.
   Split occluders and order across chunk edges, with surface-aware picking and stable ties.
   Separate roof/floor receivers; fade selected-person cover and occupied roofs, with quiet band silhouettes.
   Keep roofs physically present; stop bulk art if ordering or shadows fail.
3. `T2.8a.3` **Water and shelter light (`PRE-26`, `PRE-30`, `WLD-13`).**
   One bed/water height source, depth tint, shore, flow, contact and wading; prepared background refraction and a measured projected reflection layer.
   Fire lights openings without sun shadow darkening its emission or light leaking through shelter walls.
   Add declared hour/weather records; never call them a generated physical sky.
4. `T2.8a.4` **Review and delivery (`PRE-31`, `PLT-04`).**
   Flat and terrain before/after views, long shadows at tile edges and a debug receiver/layer view.
   Show noon/dusk, deep-cave entrance/view handling and shallow cutaway scope; preserve the later geological slice.
   Record CPU/GPU costs with missing counters labelled unavailable; deliver as 30801.

Engine built (8 October 2026): `T2.8a.1` to `T2.8a.3` and the engine review route of `T2.8a.4` are implemented on labelled developer fixtures. One shader light function combines separate sun visibility, sky openness, local fire and emission. World-basis normal calibration, material-byte masks and the material response table are explicit. Five angular rays soften long dusk shadows; logical caster overlap is a union and contact takes the strongest contribution once. CPU masks start at actual receiver heights, cache terrain/light revisions, update moving bodies at 10 Hz and have fixed sample, body and byte caps. The declared noon/dusk/night and weather profiles are fixture records, not a generated physical sky.

The restricted surfaces share their heights with feet, picking, masks, water depth and wading. Slope, joined cliff faces, front/rear shelves and a split shelter exercise one local overlap graph across chunk borders, with stable ID ties and a visible split remedy for a cycle. Roof and floor receivers stay separate. Selected cover fades, occupied roofs admit a shallow cutaway, and other obscured people have quiet band silhouettes; physical proxies and saved-world digests stay unchanged. A doorway or native control opens a separate cave interior and restores exterior focus and selection on exit. The later geological slice is untouched.

Water refracts a prepared receiver-only bed, clips emerged bodies at the common water height and uses a separate projected reflection target. Fire visibility checks shelter walls and the doorway independently of direct sun. Existing atlas pivots and authored density levels accept optional aligned world normals, indexed material masks and named tree pieces. Approved-sheet crops, diagram people/animals and plain terrain/shelter pieces remain developer art; actual neutral runtime art and seasonal shapes still need their own review. Flat style approval must precede terrain approval. Delivery 30801 carries the projection work too; flat style and then terrain approval remain separate owner reviews.

Recovery checks (8 October 2026): the rebuilt extension passes all 72 C++ view tests, and the Terrain, Fixtures and retained Crowd pages passed all 19 game tests before packaging recovery; the final shared-sheet change passes all 20 targeted game tests. Six new regressions were observed failing before correction: trunk order, floating flame geometry, stale prepared-bed order, off-screen actors losing visible shadows, accelerated time defeating the mask throttle, and submerged objects appearing above water. Flame inspection now names its sampled receiver. All seven findings from the inherited implementation's independent code review have corrections. The independent follow-up review is clean; it also passed nine Terrain tests, twelve packaging tests and resource validation on a fresh Android export. Renderer checks were performed separately in the capture environment.

The reported pink sliver beside the birch is visible through its source alpha; it does not cover opaque bark. The design sheet opens on request instead of obscuring the scene. A shared shader helper keeps colour and semantic water coverage aligned, with bounded-footprint clipping and a thin waterline contact cue. An actual-renderer probe failed without that cue, then passed with it; it also checks normal axes, material indexing, fire visibility and emission in sun shade. The restricted shelter receivers are separate from the old dome-shaped sheet crop, which remains explicitly temporary until the reviewed cone tent arrives.

All eight accepted M1 proof suites still match the main-build manifest on one and four threads, and deterministic flag/object scans pass. Android export now includes the fixture design sheets. Twelve packaged-resource tests, each observed failing under a targeted fault, check raw records, sheets and imported atlas targets. A thirteenth resource test then passed after failing for the shared catalogue sheet path. The signed 30801 rebuild passes the release signature, version, offline, orientation, 16 KiB alignment and packaged-resource checks at 51,248,270 bytes (48.87 MiB), within the 50 MiB limit. Lossless extension compression and reuse of the identical ground sheet preserve the resource contents while removing a duplicate. All fourteen focused packaging tests pass; the compression and shared-sheet tests were observed failing before correction. Final independent review and the full delivery check remain merge gates. These checks do not replace final-art, flat-style or phone performance approval.

Inspected captures cover 23 views and 92 colour/page/object/material passes, repeated with exact pixels and mapped semantic codes; the four earlier Fixtures views retain all 16 existing golden passes. Six separate water bed/reflection targets also repeat exactly. Golden validation first rejected the cave background as an unknown object ID, then passed after inspection backgrounds became ID zero; visible colour is unchanged. A 96-frame, 12 fps sequence shows sun rotation, roof reveal, wading and cave entry/exit. Flat/cliff lettered comparisons use diagnostic unlit albedo, not approved art baselines; FLIP means are 0.123/0.231 at 80 pixels per degree. Visual critique prefers the lit/contact versions, but still finds faint composed reflections, noisy ground and ambiguous repeated colour selectors. Owner style approval remains open.

Cloud capture costs use Compatibility/Mesa llvmpipe, 80 measured frames after 10 settling frames, a frozen 60 fps elapsed clock and 10 Hz moving masks. Flat noon averages 4.47 ms script, 20.57 ms viewport CPU and 36.91 ms summed viewport GPU; water noon averages 4.05/33.72/50.10 ms, including 8.81 ms bed and 2.07 ms reflection GPU counters. These software-renderer counters are not phone frame times. The flat/water CPU mask payloads are 84,450/101,350 bytes against a 1,352,000-byte hard cap. A separate 32-body probe measured outdoor CPU mask medians of 2.68–5.50 ms; actual phone speed, power and temperature remain unmeasured.

**Tests:**
- A foreground tree can cover a rear high shelf, and cliff faces and shelter fronts order correctly across chunk borders.
- Rays start at actual receiver height; roof and floor are distinct; off-screen casters and low-sun reach are included.
- Overlapping shadows stay coloured, contact is not counted twice, and a partly shaded person reads; fire remains bright inside sun shade.
- Selected cover fades while physical shelter state and digests stay unchanged; other obscured band members remain visible as quiet silhouettes.
- Water depth uses the common heights; wading feet meet the surface; foreground people are not refracted with the bed.
- Flat style and then terrain result pass owner review; failures name the split/mask remedy before adding content.

**On the phone:** move the sun over the flat fixture, compare contact and long dusk shadows, then open the slope, cliff and shelter views.
Select someone beneath a tree or roof, look into the occupied shelter and watch wading and fire in shade.
Approve the flat style first, then judge the terrain result.

### α2.9a Zoom and streaming

**Goal:** authored art steps, coarse parents and bounded caches across the zoom range, with safe world switching and accepted time controls.

**Serves:** `PRE-01`, `PRE-03`, `PRE-22`, `PRE-28`, `PRE-42`, `PRE-43`, `PRE-46`, `PLT-02`, `PLT-04`, `PLT-09`, `WLD-13`, `TIM-01`, `TIM-04`, `TIM-15`, `RES-05`.

**Architecture:** A3.6, A3.7, A3.9, A4.1, A4.6, A4.8, A5.3, A6.2, A8.1, A8.3, A8.6, A18.1.

**Tasks:**

1. `T2.9a.1` **Art steps and wide forms (`PRE-22`, `PRE-28`, `PRE-42`, `PRE-43`, `PRE-46`).**
   Separate 64/16/4 source families with repaired intermediate chains, aligned masks and atlas gutters.
   Quiet tile variants, drawn transitions, broad stamps and ecology-selected detail; stable world-coordinate keys and wrapped neighbours.
   Designed tiny figures, group marks and camps; retain real-object inspection data.
2. `T2.9a.2` **Bounded preparation and caches (`PRE-03`, `PLT-04`, `WLD-13`).**
   Owned immutable job inputs with epochs/revisions, bounded queues, staged CPU-ready/GPU-uploaded/visible states and cancellation.
   Revision resynchronisation survives skipped snapshots.
   Retain coarse parents through cold loads and rapid pinches; count textures, targets and staging separately.
   Use measured upload headroom; preload fixture pages.
3. `T2.9a.3` **Zoom, time and worlds (`PRE-01`, `PLT-02`, `PLT-09`, `TIM-01`, `TIM-04`, `TIM-15`).**
   Keep selected IDs, focus and display time across art changes, orientation and reopen.
   Existing zoom speeds and manual priority remain; leave pause/skip/director priority connections possible.
   Cache keys include world/data/renderer/revision; caches stay outside essential archives.
   Taps retain displayed time and ID, without claiming to fix the later power-timing gap.
4. `T2.9a.4` **Cold-load route and delivery (`PLT-04`, `RES-05`).**
   Exercise eviction, cancelled and stale jobs, rapid world swaps, pinch releases and cache deletion.
   Show resident/peak bytes and decode/upload frame costs, with new named 2D benchmark variants.
   Deliver as 30901.

**Tests:**
- Named stops retain person about 8 m, close camp 20–50 m, camp a few hundred metres, valley about 10 km and region about 100 km; culling covers the projected portrait footprint and shadow halo.
- Intermediate steps keep repaired faces/tools/edges and crisp resting pixels; no permanent mip blend or synchronous load in gestures.
- Wrapped joins and detail placement are unchanged by cache size or load order; ground repeat/accent checks and busy-camp readability hold.
- Parents remain visible until detail arrives; stale world/revision jobs cannot publish, even with skipped snapshots or eviction.
- Queues and byte ledgers stay within A18.1's measured starting caps; preparation/opening meet 10 s after install and 3 s warm open.
- At equal simulation seconds, different zooms, views and worker counts retain identical digests; deleting caches and save/recovery change no world state.

**On the phone:** pinch through person, close camp, camp and map scales, then switch saved worlds and turn the phone during a load.
Try a cold view and the cache-clear check; look for holes, stalls, lost selection, blur or a jump on release.
Check that zoom requests time speed and a manual speed choice still wins.

### α2.10a World map and globe

**Goal:** the owner-confirmed map and 2D globe display from one overview, preserving torus rules and saves.

**Serves:** `PRE-03`, `PRE-26`, `PRE-28`, `PRE-29`, `PRE-30`, `PRE-33`, `PLT-04`, `WLD-01`, `WLD-02`, `WLD-03`, `WLD-13`, `RES-05`.

**Architecture:** A4.1, A4.6, A8.3, A8.4, A8.5, A8.6, A18.1.

**Tasks:**

1. `T2.10a.1` **Shared overview (`PRE-26`, `PRE-29`, `PRE-30`, `WLD-03`).**
   An explicitly labelled 2000 × 1000-cell overview fixture, shared by overhead map and disc, with relief, cover, drainage and declared weather channels.
   Count colour's 8–11 MiB inside the map cache, plus extra channels.
   Preserve minimum river visibility and ordinary changing light; use stable relief only at very fast rates.
2. `T2.10a.2` **Globe display (`WLD-01`, `WLD-02`, `WLD-13`).**
   Forward torus-coordinate mapping in a 2D disc shader, longitude repeat, latitude clamp and ice pole treatment.
   Polar compression changes only the picture; distances, cell areas, paths and saves remain exact.
   No sphere scene or physical rewrite, and no claim that M2 built the permanent generated ice barrier.
3. `T2.10a.3` **Markers, taps and transitions (`PRE-03`, `PRE-28`, `PRE-33`).**
   Shared marker mapping and inverse visible-disc picking, hidden hemisphere rejected.
   Keep focus, selected IDs and time through local/map/globe transitions; centre another longitude without rotating local art.
   If shown, a scale uses model distances and explains globe distortion.
4. `T2.10a.4` **Display checks and delivery (`PLT-04`, `RES-05`).**
   Seam/pole screenshots, inverse-tap checks and the full pinch journey on the saved run.
   Measure incremental globe cost against the proposed under-1-ms goal and total cache/frame lines.
   Deliver as 31001; M3 later replaces fixture geography with generated cells.

**Tests:**
- East-west seam samples and markers agree; latitude clamps without blending top and bottom rows; warm land never filters through the ice edge.
- Known points round-trip through markers and visible-disc taps; back-hemisphere taps are ignored.
- At 60° latitude display width halves while true model distances and cell sizes remain unchanged.
- Focus, selection and time survive every transition; local camera remains at 37° without rotation.
- Shared overview is counted once; memory and incremental GPU cost are reported; unchanged saves reopen with identical simulation digests.
- Fixture results prove display mapping only; the generator and ice-crossing tests remain in M3.

**On the phone:** pinch from a person to the world map and globe, tap a marked place and return to it.
Centre another longitude, check poles and the seam, and compare ordinary-time sun with the fast-time relief view.
The page labels this geography as a fixture.

### α2.11a Two-phone renderer comparison

**Goal:** measure identical fixtures on Compatibility and Mobile at 60 and 30 fps, then keep the simplest path that passes both look and performance review.

**Serves:** `PRE-01`, `PRE-22`, `PRE-26`, `PRE-28`, `PRE-30`, `PRE-31`, `PLT-02`, `PLT-04`, `VIS-14`, `RES-05`, `RES-06`.

**Architecture:** A3.9, A4.7, A4.8, A5.5, A8.7, A18.1.

**Tasks:**

1. `T2.11a.1` **Named devices and fixtures (`PLT-04`, `PRE-28`, `PRE-30`).**
   Record the owner's Pixel 11 Pro XL and the chosen weaker phone: RAM, OS, usable display, renderer and driver.
   Identical seven-person day/three-fire night scenes, thirty-person woodland camp with water/fire/smoke/weather, separate crowd stress, wide-map streaming and repeated pinches.
   State counts, visible areas and initial approved actions; six objects alone are not a camp benchmark.
2. `T2.11a.2` **Frame and streaming measures (`PLT-04`, `PRE-22`, `PRE-26`).**
   Compatibility and Mobile at 60 and 30 fps, with cold loads, saves, reopen and sustained activity.
   Existing calibration/result reporting with explicitly 2D variant names, frame distribution, GPU/main-thread times, batches/passes, resident/peak bytes and upload costs.
   Include every active viewport; label unavailable APIs and silent renderer fallback.
3. `T2.11a.3` **The twenty-minute route (`PRE-01`, `PLT-04`, `VIS-14`).**
   On each phone and renderer/mode, run the busiest passing scene after cooling, unplugged and in flight mode.
   Read power, battery temperature, thermal status and 10-second forecast; use the existing governor.
   Keep the approved grid in 30 fps; any logged heat step must pass the owner's blind test.
4. `T2.11a.4` **Decision, review and delivery (`PRE-31`, `PLT-02`, `RES-05`, `RES-06`).**
   Compare the same noon/dusk, night, season, weather, orientation and action views.
   Record which renderer passes and why; fix failures before scaling art.
   Repeat accepted simulation proofs at equal game seconds with both views and worker counts; deliver as 31101 with both phones' evidence.

**Tests:**
- At 60 fps: GPU mean at most 8 ms, p95 at most 9.5 ms; main thread mean at most 8 ms, p99 at most 12 ms; busy woodland camp GPU mean at most 6 ms.
- At least 97% on-time frames and no frame over 66.7 ms; at 30 fps use its 33.33 ms interval and the same grid.
  Report distributions and each minute's frame delivery, not only averages.
- Whole-phone mean at most 4 W, battery at most 40 °C at end, thermal status none and forecast below the light threshold minus 0.05 where available.
- Application memory at most 1 GiB; bounded caches/targets/staging, preparation within 10 s after install, warm opening within 3 s and no new shader compilation stalls during active drawing.
- Carry 300 draws as an initial diagnostic ceiling, distinguishing canvas/UI/off-screen batches; it never substitutes for frame-time evidence.
- Both look reviews and measurement routes pass; unavailable evidence is reported explicitly, never filled with zero.
- Digests, old-save opening, recovery and cache deletion still pass; graphics timings do not stand in for simulation correctness.

**On the phone:** install each named renderer build, compare the same fixtures, then run the cool, unplugged twenty-minute route at 60 and 30 fps.
Send each result code with its phone and renderer; the report shows any unavailable readings and the resulting choice.

### α2.12a Art scale-up, cleanup and M2's end

**Goal:** after the five engine gates, expand the M2 art set, port remaining pages and checks, remove unused runtime 3D work and close M2 with the owner's review.

**Serves:** `PRE-20`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-27`, `PRE-28`, `PRE-30`, `PRE-31`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`, `PLT-02`, `PLT-04`, `PLT-09`, `VIS-14`, `RES-06`, `RES-22`.

**Architecture:** A2.2, A4, A5, A6, A8, A17, A18.

**Tasks:**

1. `T2.12a.1` **Expand reviewed art (`PRE-20`, `PRE-23`, `PRE-24`, `PRE-27`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`).**
   Only after fixture, height, shadow, streaming and phone review, take signed-off catalogue batches into the pipeline.
   Fill M2's camp materials, plants, shelters, figures, initial actions and seasonal shapes, including two-material sheets and dawn/noon/dusk/night views in summer and winter.
   Rain, storm and mist remain declared look fixtures; the full later activity list stays with its simulation milestone.
2. `T2.12a.2` **Port and remove (`PLT-09`, `PRE-26`, `PRE-30`).**
   Follow the migration rules below: callers, tests, data and registrations first, then unused runtime 3D sources and derived exports.
   Keep original art, approvals and benchmark history, and never relabel old 3D evidence as a 2D pass.
   Native world/save/import/export/time controls remain on the existing extension.
3. `T2.12a.3` **Final busy scenes and contact sheet (`PRE-28`, `PRE-31`, `PLT-02`, `PLT-04`).**
   Fixed saved scenes at each zoom stop at noon and dusk, landscape, art sheets, work clips, pans/pinches and busy woods/camp/night views.
   Timed find of every person, including reveal; rerun only measurements changed by expanded art before the final required checks.
4. `T2.12a.4` **M2's end (`VIS-14`, `RES-06`, `RES-22`).**
   Deliver as 31201, with the milestone report and independent milestone review.
   State which requirements are proved by fixtures and which await real world systems.
   Reconcile the approved M3 plan and architecture with M2’s final interfaces for the owner’s stage review; never close M2 from a design or painted sheet alone.

**Tests:**
- Expanded art reaches its signed-off sheets, including materials, wear, style, seasonal silhouettes, animation and each art step.
- Busy scenes retain readability, lit edges/contact, water bed/reflections and the approved reveal without changing physical state.
- Ported scripts, data, build entries and registrations contain no live runtime-3D dependency; old save corpus and accepted M1 proofs still pass.
- Both-phone frame/heat/memory lines still hold where art changes affect them; all required earlier scenes and checks pass at delivery.
- Contact sheet meets every applicable M2 Done when, judged independently and then by the owner; later world and archaeology promises stay mapped.

**On the phone:** explore the expanded camp through hours, seasons and weather, find every person, and view the art sheets and clips in both orientations.
Read the report with both phones' numbers and M3's plan, then accept M2 or send it back.

#### Migration rules

This is planned M2 work, not code deletion in the document update.
Port each caller and test before removing its old drawing path.

| Keep | Replace | Delete once replacements pass |
|---|---|---|
| M1 simulation, exact arithmetic/chance, IDs/events/activities, workers, catalogues, scenes, saves and proofs | Add sprite look schemas and later physical surface fields through existing versioned mechanisms | Only obsolete 3D look fields and unused serializers; no foundational subsystem |
| World lifecycle, snapshot transfer, activity sampler, pace and native management controls | One consumer with owned sprite/surface records and epoch/revision handling | No accepted M1 service merely because graphics changed |
| Stage ownership, pan/pinch input, nearby origins, timings, heat/device/calibration services | Fixed projection, canvas controller, visible 2D draw lists and named 2D counters | Twist routes, turn tours, perspective scenarios, 3D transform submission and unused drawing handles |
| River/bed fixture functions, stable look fields, useful contact/openness/horizon algorithms and tests | Projected surface patches, drawn ground edges, explicit layered receivers and logical casters | Mesh uploads, triangle-image wrappers, flat-only sun path and duplicate shader terrain shape |
| Checked texture reading, image validation and upload helpers | Separate authored families, complete repaired reductions, aligned masks/normals and sprite packing | Old 3D ground-ladder assumptions and unused packing paths |
| Catalogue concepts, original art, source rigs, records and approvals | Sprite body/clothing/action catalogue, frames and attachments | Runtime Blender assembly, bones, poser/skinning, mesh kit draw and required kit exports |
| Review layouts, fixture ideas, UI pages, data inspection, save/export/import, time controls and reports | 2D fixture pages/shaders, native UI outside the world viewport, Compatibility/Mobile variants and coherent bindings/build entries | Unused 3D camera/light pages, spatial shaders, prepasses, leaf coverage, bone palettes, mesh helpers and unused companion assets |
| Setup, exact builds, Android packaging/signing, scene/samebits/kill checks, capture and art/provenance tools | Asset build, fixtures, atlas/pivot/mask/season checks and sprite calibration data | Old mandatory Blender-part production lane and unused mesh/kit entries, only after migration |

Do not delete source sheets, raw art, useful optional offline rigs or historical performance records.
A page, its scripts, data, shader, extension registrations and build entries change coherently.
The build and data checks find remaining references before cleanup.

## M3 The world

**Goal:** make whole worlds from a seed in causal order, choose among the best candidates, and explore the same land from globe to cliff and geological section, through real seasons, weather, water and rare land events (`MIL-10`).

**Serves:** `WLD-01`, `WLD-02`, `WLD-03`, `WLD-06`, `WLD-07`, `WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`, `WLD-12`, `WLD-13`, `WLD-14`, `WLD-15`, `WLD-16`, `WLD-17`, `WLD-22`, `WLD-24`, `WLD-26`, `WLD-27`, `WLD-30`, `PRE-03`, `PRE-25`, `PRE-29`. It also serves `PRC-09`, `PRC-11`, `PRN-04`, `PRN-10`, `PRN-14`, `PRN-15`, `PRN-16`, `RES-09`, `RES-12`, `RES-13`, `RES-18`, `SCP-11`.
It also integrates `PRE-02`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-30`, `PRE-31`, `PLT-04`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `TIM-14`, `TIM-16`, `TIM-18`, `RES-05`, `RES-06`, `RES-21`, `RES-22`.
The existing “Rules every alpha keeps” still apply in full.

**Entry gate:** M2's accepted projection, height/light/order, streaming and map/globe interfaces, with their actual merged code re-read.
At adoption α2.9a/α2.10a are still planned and α2.8a awaits delivery acceptance.
Do not treat their architecture as completed implementation.
You approved this design and plan on 8 October 2026, to build after M2. Freeze remaining detailed reference tables and pass rules before tuning.

**Approved scope (8 October 2026):** complete M3 claims cover physical geography and the world systems built here.
The approved phased acceptance carries full ecological settling and traces to M4, bodies/bands/survival to M5, and controls to M9.
No missing subsystem is implemented as a decorative substitute or declared passed from a mock.
Keep every later check open until its assigned milestone proves it; the visual and milestone acceptance reviews remain at M3’s end.

**You will see:** plates become land and rivers, climate shapes soils and vegetation potential, three qualified world choices, storms and delayed floods, a cold-to-warm world tour, cave entrances/interiors, and a line through the ground showing its true layers and water.

**Risks:** generation and daily world work must fit held phone speed; eclipse parameters must meet the full spatial range; plausible land needs your visual review; later ecology and survival checks stay open.

**Estimates:** each lettered delivery below is initially 4–6 builder hours, including focused tests and delivery; 24 deliveries imply roughly 96–144 hours, not a promise of calendar completion.
If a step exceeds six hours, split at a task boundary and give each new letter its own APK and tests.
Numbered alpha/task IDs become fixed only when work starts; version codes below follow the existing formula.

| Step | Title | Delivery | Status |
|---|---|---:|---|
| α3.1a | World contract and boundary tests | 40101 | Planned |
| α3.1b | Generated-world saves and proof harness | 40102 | Planned |
| α3.2a | Plates, land and ice | 40201 | Planned |
| α3.2b | Rock layers, faults and cave envelopes | 40202 | Planned |
| α3.3a | Drainage, flats and lakes | 40301 | Planned |
| α3.3b | Erosion and fixed river geometry | 40302 | Planned |
| α3.4a | Final climate and ocean influences | 40401 | Planned |
| α3.4b | Soils and geological resources | 40402 | Planned |
| α3.5a | Biomes and initial life records | 40501 | Planned |
| α3.5b | Repeatable area facts and coarse ground | 40502 | Planned |
| α3.6a | Sun, moon and stars | 40601 | Planned |
| α3.6b | Hourly weather | 40602 | Planned |
| α3.7a | Water, snow and floods | 40701 | Planned |
| α3.7b | Sea ice, soil changes and water quality | 40702 | Planned |
| α3.8a | Natural faults and volcanoes | 40801 | Planned |
| α3.8b | Disturbance logs and area catch-up | 40802 | Planned |
| α3.9a | Candidate ranking and start certificates | 40901 | Planned |
| α3.9b | Choosing, settling and world lifecycle | 40902 | Planned |
| α3.10a | Generated land from globe to cliff | 41001 | Planned |
| α3.10b | Caves and the geological slice | 41002 | Planned |
| α3.11a | Generation and steady-world performance | 41101 | Planned |
| α3.11b | Held-phone, streaming and recovery route | 41102 | Planned |
| α3.12a | Held-out worlds and final visual repair | 41201 | Planned |
| α3.12b | Independent review and M3 close | 41202 | Planned |

### Delivery rule for every lettered step

Run only touched tests while building, each new test failing once against a planted fault.
At delivery run `tools/check.sh --deliver` once and review the step; at b review its whole numbered alpha too (`PRC-09`, `PRC-10`).
Keep all accepted M1 proof answers, old-save behavior and earlier quick checks intact.
Ship the signed APK and plain note naming task/requirement IDs, what to try, rough edges, measured results and outstanding cross-milestone checks (`PRC-11`).
World-making changes identify a big update and keep old history readable; schema-only compatible changes supply migrations (`PLT-09`).
The builder makes the review and delivery concrete; the coordinator handles commits/merges under the protocol.
Adopting this plan does not close any implementation, APK or phone-performance check.

### α3.1a World contract and boundary tests

**Goal:** typed fields, units, IDs and immutable inputs before any terrain algorithm.

**Serves:** `PRN-04`, `RES-21`, `TIM-16`, `WLD-01`, `WLD-03`, `WLD-08`, `WLD-09`, `WLD-12`.

**Architecture:** A7.2–A7.4.

**Tasks:**

1. `T3.1a.1` **Grid and topology (`WLD-01`, `WLD-03`, `WLD-12`).** Implement canonical world/weather/area addresses, the approved 250 m pitch and common polar transport-edge checks without altering `Torus` maths.
2. `T3.1a.2` **Schemas and job interface (`WLD-08`, `WLD-09`, `TIM-16`).** Add validated generation/rock/climate parameter schemas through existing catalogue visitors; exact units, separate world/rules/look fields and stage digests.
3. `T3.1a.3` **Land laboratory (`PRN-04`, `RES-21`).** A clearly labelled small synthetic data page shows layers, units and seam samples through the existing extension, with generation progress/cancellation and measured allocation counters.

**Tests:** every one of 2,000 × 1,000 cell addresses maps to exactly sixteen 250 m areas; randomized wrapped queries preserve coordinates; all seam-crossing adjacent, diagonal and swept transport cases are refused; east-west cases pass.
Catalogue planted faults fail with source locations; cancellation changes no saved world.
No phone timing claim comes from this synthetic page.

**On the phone:** open Land laboratory, inspect a cell and its sixteen areas, pan across east-west and inspect the polar stop.
**Risk:** mixing grid/storage sizes with the accepted world size; keep exact dimensions in the proof.

### α3.1b Generated-world saves and proof harness

**Goal:** real saved layer state on M1's lifecycle.

**Serves:** `PLT-07`, `PLT-08`, `PLT-09`, `RES-05`, `RES-12`, `RES-21`, `TIM-16`, `WLD-12`.

**Architecture:** A7.14, A17.

**Tasks:**

1. `T3.1b.1` **Layer owner extension (`TIM-16`, `WLD-12`, `PLT-09`).** Append owner IDs, version owner schedules and preserve old owners; a new generated-world wrapper attaches systems while demos retain their original systems.
2. `T3.1b.2` **Saved base and mutable chunks (`PLT-07`, `PLT-08`, `PLT-09`).** Critical base chunk, locally versioned system payloads inside SYST, candidate metadata, checked sizes and migrations; immutable shared bytes where safe; no foreign cache files in archives.
3. `T3.1b.3` **Scenes and new proofs (`RES-05`, `RES-21`, `RES-12`).** Add generated-world scene measures and stage-resume harness; retain all M1 proof suites unchanged; add one new generated-layer suite and corruption faults.

**Tests:** one/four workers, save/reopen/export/import and stage resume yield identical canonical state; old corpus loads with original outcomes; unknown critical chunks/oversized counts are refused before allocation.
Owner arrays migrate with no new demo events; capture snapshot copy/peak bytes.

**On the phone:** save the test layer, switch worlds, reopen/export/import, and run its matching-digest self-check.
**Risk:** `owners::count` and small history payloads are concrete extension work, not pre-existing general world storage.

### α3.2a Plates, land and ice

**Goal:** recognizable generated continents and ranges, with causal labels.

**Serves:** `PRE-29`, `WLD-01`, `WLD-02`, `WLD-03`, `WLD-06`, `WLD-08`, `WLD-09`, `WLD-30`.

**Architecture:** A7.5.

**Tasks:**

1. `T3.2a.1` **Plates and boundaries (`WLD-06`, `WLD-09`).** Periodic plate partition, motion types and boundary-derived uplift/rift/subduction/transform features.
2. `T3.2a.2` **Ground and sea datum (`WLD-03`, `WLD-08`, `WLD-30`).** Shelves, worn continents, range profiles and one frozen sea level; no generation of detailed areas.
3. `T3.2a.3` **Permanent cap and previews (`WLD-01`, `WLD-02`, `PRE-29`).** Generate the 200 km ice strip; show real height and plate overlays through the shared overview provider.

**Tests:** 100 parameter seeds stay within tilt/land ranges and contain 6–12 plates; plate edges and feature references agree at both seams; no partial area generation occurs.
Report relief/land share at full resolution; ordinary plateau noise cannot create an unrelated high range.
The same 100 seeds are checked again after erosion: final land remains 25–50%, and ten equal bins of both tilt and land share each contain at least one world (proposed minimum spread check, frozen before tuning).

**On the phone:** choose a seed, watch the plate/height layers, compare two seeds and inspect the cap on the globe.
**Risk:** straight cellular plate borders visible in final relief; boundary perturbations must retain shared identities.

### α3.2b Rock layers, faults and cave envelopes

**Goal:** rocks and shelters have a consistent cause before local detail.

**Serves:** `PRE-23`, `PRE-25`, `RES-05`, `WLD-09`, `WLD-15`, `WLD-24`.

**Architecture:** A7.5–A7.6.

**Tasks:**

1. `T3.2b.1` **Geological provinces (`WLD-09`, `PRE-23`).** Named rock catalogue, coherent three-layer columns and ancient marine/basin provenance, with erosion resistance and permeability.
2. `T3.2b.2` **Feature descriptors (`WLD-09`, `WLD-15`, `WLD-24`).** Fault/volcano identities and type, plus provisional cave/overhang host and size envelopes; final existence/entrance geometry follows erosion in α3.3b, and dryness follows final water.
3. `T3.2b.3` **Rock inspection and checks (`PRE-25`, `RES-05`).** Provisional geological profiles and provenance inspection; pin stage hashes and test neighbouring contact samples.

**Tests:** 20 worlds have ranges/faults/volcanoes in their declared boundary influence zones and chalk/limestone only in permitted ancient basin settings; no stack exceeds three rock layers; sampled shared contacts agree exactly.
Provisional cave envelopes fit eligible hosts and dimensional bounds; reject contradictory descriptors. These are not yet final dry-shelter certificates.

**On the phone:** tap a range, volcano or provisional cave envelope to see its source and rock stack.
**Risk:** visual cave beauty must not invent dry floor area the certificate lacks.

### α3.3a Drainage, flats and lakes

**Goal:** all flow has a destination without flattening lakes away.

**Serves:** `RES-05`, `WLD-01`, `WLD-08`, `WLD-09`, `WLD-16`, `WLD-17`.

**Architecture:** A7.6.

**Tasks:**

1. `T3.3a.1` **Rough runoff (`WLD-09`, `WLD-16`).** First climate/runoff fields from latitude, relief and sea, explicitly separate from final climate.
2. `T3.3a.2` **Basin graph (`WLD-08`, `WLD-17`).** Sea-seeded Priority-Flood, original bed retained, spill hierarchy and deterministic flat rank; terminal basins explicit.
3. `T3.3a.3` **Flow proofs (`RES-05`, `WLD-01`).** Topological accumulation, lake storage curves, origin/destination debug overlay and adversarial flat/seam basin tests.

**Tests:** every receiver chain terminates, no directed cycle or illegal polar transfer; all lake curves are monotone; nested basins fill/spill with exact modeled volume accounting.
Reversed receiver and lost-basin faults fail; one/four workers and reversed task issue order match.

**On the phone:** follow water from a ridge to a lake and then its outlet; inspect original bed versus spill level.
**Risk:** the filled routing surface must never overwrite physical ground.

### α3.3b Erosion and fixed river geometry

**Goal:** branching valleys, floodplains, fans, deltas and continuous rivers.

**Serves:** `RES-09`, `WLD-08`, `WLD-09`, `WLD-14`, `WLD-17`, `WLD-30`.

**Architecture:** A7.6, A7.13.

**Tasks:**

1. `T3.3b.1` **Bounded incision (`WLD-08`, `WLD-09`, `WLD-30`).** Fixed erosion passes with lithology/runoff response, slope relaxation and generated frozen-glacier forms; final routing after height edits.
2. `T3.3b.2` **Sediment provenance and waterways (`WLD-14`, `WLD-17`).** Depositional masks, transported-rock provenance and distance-dependent rounding classes; fixed reaches, junction anchors, width/depth and monotone bed profiles; finalize cave existence and entrances against the eroded ground.
3. `T3.3b.3` **Map measures (`WLD-08`, `RES-09`).** Implement the pre-approved slope, coast, drainage and lake metrics, with landform diagnostics and a 20-world training report.

**Tests:** the G1 terrain measures below pass on the development set; rivers stay connected across 1 km/250 m borders and the longitude seam; no curve exits its valley/divide envelope or climbs its bed profile.
Cut through a provisional cave envelope in an erosion fixture: final entrances/host/floors must follow the changed ground; later flooded-start checks must reject a wet shelter.
The final held-out G1 confirmation remains α3.12a.

**On the phone:** follow a large river from mountain valley through floodplain to delta; compare hard- and soft-rock valleys.
**Risk:** more erosion iterations can cost time without making better land; retain them only with visible evidence.

### α3.4a Final climate and ocean influences

**Goal:** temperature, rain shadows and contrasting coasts explained by land and sky.

**Serves:** `RES-09`, `SCP-11`, `WLD-06`, `WLD-16`, `WLD-26`, `WLD-30`.

**Architecture:** A7.7.

**Tasks:**

1. `T3.4a.1` **Ocean descriptors (`WLD-26`, `WLD-16`).** Basin-aware warm/cold boundary influence, current direction, upwelling and coast classes.
2. `T3.4a.2` **Four-season climate (`WLD-06`, `WLD-16`, `WLD-30`).** Tilt/latitude/lapse, continentality and winds, bounded moisture transport with no polar crossing; store normal/extreme weather envelopes.
3. `T3.4a.3` **Reference harness (`WLD-16`, `SCP-11`, `RES-09`).** A7.7’s approved Earth comparison for research only and synthetic mountain/coast tests; weather in mm per real-length game day, Earth-equivalent annual biome indices, and 15-day saved seasonal totals; no Earth map in playable data.

**Tests:** lee dries versus matched windward terrain; height lapse and continental temperature range respond in the right direction; seasonal reversal holds; repeated longitude shifts create no privileged seam.
Reference bins and climate-share diagnostics use stated denominators; use A7.7’s approved method: 10-degree bands, ≤10 percentage-point class-share difference per sufficiently sampled band, about 2°C matched-bin warmth, and Earth-tilt plus 15°/30° cases.

**On the phone:** switch seasonal temperature/rain/wind overlays and inspect opposite ocean coasts.
**Risk:** exact Earth averages on a tiny variable-tilt world are not a self-defining test.

### α3.4b Soils and geological resources

**Goal:** useful ground follows causes and has readable depth/exposure.

**Serves:** `PRE-23`, `PRE-25`, `RES-05`, `WLD-09`, `WLD-14`, `WLD-27`.

**Architecture:** A7.7.

**Tasks:**

1. `T3.4b.1` **Soil data (`WLD-27`, `WLD-09`).** Parent sediment, slope/climate/potential vegetation, kind/thickness/fertility/retention/diggability.
2. `T3.4b.2` **Deposits (`WLD-14`).** Host-constrained flint/chert/obsidian, hammer/grinding stones, clay/ochre and copper/native copper with source-rock provenance in gravels. A7.7 keeps flint in chalk and chert in eligible limestone, both counted as the flaking-stone family.
3. `T3.4b.3` **Exposure and checks (`PRE-23`, `PRE-25`, `RES-05`).** Shared underground/exposed sample API; geological invalidity traps, independent hammer/grinding-stone host and matched upstream/downstream rounding checks; resource overlays and canonical encoding.

**Tests:** 100 worlds have no illegal deposit occurrence, including transported gravel and native-copper cases; no material is regenerated just by inspecting it.
Fertility stays 0–5 and soil/rock units validate; source and downstream gravel agree.

**On the phone:** inspect flint in chalk, gravel downstream, a glassy volcanic source and weathered copper; slice their depth in the diagnostic profile.
**Risk:** finding a rare required ore must not turn into adding it to a failed candidate.

### α3.5a Biomes and initial life records

**Goal:** the world's initial vegetation and animal records have habitat causes.

**Serves:** `PRE-29`, `PRN-10`, `WLD-09`, `WLD-10`, `WLD-16`, `WLD-27`, `WLD-30`.

**Architecture:** A7.1, A7.7.

**Tasks:**

1. `T3.5a.1` **Potential biomes (`WLD-09`, `WLD-16`, `WLD-27`).** Earth-equivalent temperature/moisture/growing-season indices, soil/wetness and shore/highland overrides.
2. `T3.5a.2` **Initialization contract (`WLD-09`, `WLD-10`, `WLD-30`).** Approved plant cover shares/regrowth-age fields, approved species occurrence/count/range descriptors and food-potential inputs; one schema extended by M4, no parallel species catalogue.
3. `T3.5a.3` **Visible scope and coverage (`PRN-10`, `PRE-29`).** Draw real supplied cover; clearly label unfinished ecology; list missing biome species and the later M4 acceptance checks.

**Tests:** species occur only within catalogue habitat bounds; cover shares total their defined whole; sea and shore kinds use appropriate capacity tags; all initialized counts obey the sixth-scale convention.
Full 6-plant/4-animal biome coverage and ecological stability are not passed by initialization alone.
The approved species scope includes one woolly mammoth species in cold open grassland within the roughly 30 wild-species budget; M4 proves its reviewed catalogue entry, food demand and density.

**On the phone:** visit forest, grassland, desert, marsh and tundra and inspect why each grows there.
**Risk:** initial cover is not a proved living food web; keep the M4 ledger explicit.

### α3.5b Repeatable area facts and coarse ground

**Goal:** on-demand queries agree across every edge and load order.

**Serves:** `PRE-03`, `RES-05`, `WLD-12`, `WLD-13`.

**Architecture:** A7.13, A8.8.

**Tasks:**

1. `T3.5b.1` **Canonical area queries (`WLD-12`, `WLD-13`).** Shared cave/river/deposit facts and world-coordinate placement keys; distinguish generated feature IDs from allocated entity IDs.
2. `T3.5b.2` **Coarse ground (`PRE-03`, `WLD-12`).** Tens-of-metres sampling from constrained parent relief, wrapped halos, fixed channel/cave anchors and pure material queries.
3. `T3.5b.3` **Order/cache proof (`RES-05`, `WLD-13`).** Request reversals, randomized partitions, cache erasure and neighbour placement competition; record bytes and cold preparation times.

**Tests:** 10,000 sampled shared borders/corners agree exactly; river junctions do not break; repeated queries on the same day are byte-identical and change no world/save/ID counter.
Different cache tile sizes and one/four jobs yield identical canonical results.

**On the phone:** jump between distant unvisited places, clear the picture cache and return; the same ground and resources return.
**Risk:** a per-area random stream would change border objects when neighbours load in another order.

### α3.6a Sun, moon and stars

**Goal:** physical hour, latitude and season drive the existing light.

**Serves:** `PRE-29`, `PRE-30`, `TIM-14`, `TIM-18`, `WLD-06`, `WLD-07`, `WLD-13`.

**Architecture:** A7.10, A8.8.

**Tasks:**

1. `T3.6a.1` **Solar sky (`WLD-07`, `TIM-14`, `TIM-18`).** Turn-angle sun geometry, seed tilt, local solar hour, start-hemisphere phase and polar day/night limits.
2. `T3.6a.2` **Moon and eclipse geometry (`WLD-07`, `WLD-06`).** Fifteen-day phase, inclination/node parameters and seeded stars; the approved target requires 3–8 visible solar/lunar eclipses at every place in 70 game years, regardless of cloud.
3. `T3.6a.3` **View connection (`PRE-30`, `PRE-29`, `WLD-13`).** Physical sky replaces light fixtures only in generated worlds; preserve fast-time presentation light and fixture regression pages; build the full-location eclipse coverage proof after the sampled diagnostic.

**Tests:** four full moons in each 60-day year; seasonal hemispheres reverse; equinox/solstice and high-latitude samples match the approved geometry oracle; the 70-year visibility proof requires a minimum of 3 and maximum of 8 at every place, including intra-cell visibility boundaries.
Exact same sky state on every proof build; fast display light changes no physical temperature/daylight.

**On the phone:** watch one place at dawn/noon/dusk/night in summer and winter, then visit the opposite hemisphere and polar ice.
**Risk:** the adjustable orbit family is structurally compatible with the approved target, but no fitted parameter set yet proves the 3–8 range everywhere. Fixed phase alone can cause repeating artifacts; fit inclination/precession/apparent sizes, then certify geometry. Report an unsuccessful fit without changing the requirement.

### α3.6b Hourly weather

**Goal:** storms move and deliver the climate's rain.

**Serves:** `PLT-07`, `PRE-29`, `PRE-30`, `TIM-16`, `WLD-16`, `WLD-22`, `WLD-30`.

**Architecture:** A7.10, A7.14.

**Tasks:**

1. `T3.6b.1` **Weather layer (`WLD-16`, `WLD-22`, `TIM-16`).** Hourly cell fields, storm objects, bounded persistent anomaly state and keyed births/decay.
2. `T3.6b.2` **Swept rain and lightning (`WLD-16`, `WLD-30`).** Real wind speeds, integrated crossed-cell exposure, snow/rain partition and natural lightning records with causes.
3. `T3.6b.3` **Display/history integration (`PRE-29`, `PRE-30`, `PLT-07`).** Saved trajectories and phase, bracketing snapshots at displayed time, climate-envelope inspection and 20-year diagnostics; implement G4b natural storm/lightning/drought/harsh-winter rate measures, storm-day and dry-spell checks.

**Tests:** multi-cell storm path wets every intersected cell in its integrated footprint, wraps longitude and never crosses the polar boundary; no rain without its source.
20-world/20-year G4 weather averages pass before alpha completion, or stay visibly failed pending tuning; no per-year quota repair.
Save during motion resumes identically; skipped snapshots cannot show a future storm at the old displayed time.

**On the phone:** follow an upwind storm through a valley, inspect hourly rain and snow, and run the climate-average report.
**Risk:** instantaneous cell hopping and incorrect seasonal units can both look plausible while failing the simulation.

### α3.7a Water, snow and floods

**Goal:** water stores respond and downstream water arrives later.

**Serves:** `RES-18`, `RES-21`, `TIM-18`, `WLD-12`, `WLD-17`.

**Architecture:** A7.11.

**Tasks:**

1. `T3.7a.1` **Normal daily water (`WLD-17`, `WLD-12`).** Infiltration/groundwater/spring buckets, snow/melt and lake storage, exact residual accounting.
2. `T3.7a.2` **Reach routing and flood promotion (`WLD-17`, `TIM-18`).** Saved delayed arrivals, fixed hourly active-flood routing, connected floodplain wetting and dated silt intents.
3. `T3.7a.3` **Generated water scenes (`RES-21`, `RES-18`).** Extract real valleys, test spring-fed versus runoff-only streams and snowmelt with their actual rules; add naturally occurring flood rates to G4b; injected-rain unit tests remain labelled diagnostics.

**Tests:** no negative store or unaccounted water; daily↔hourly transitions conserve in-flight volumes; natural upstream storm scene reaches its declared downstream lag without an inserted fixed delay.
A spring-fed stream outlasts the matched runoff-only one over the registered dry-spell interval; snow loss equals melt input after allowed sinks.
The 21.6–26.4 h diagnostic interval follows the general 10% rule, not a universal river transit time.

**On the phone:** watch rain upstream, skip forward to the later downstream flood, and compare a spring-fed stream through a dry season.
**Risk:** daily topological routing must not deliver rain across the whole continent at once.

### α3.7b Sea ice, soil changes and water quality

**Goal:** coasts, soil and cleanliness have the shared physical state later life needs.

**Serves:** `WLD-15`, `WLD-17`, `WLD-26`, `WLD-27`.

**Architecture:** A7.11.

**Tasks:**

1. `T3.7b.1` **Sea and ice state (`WLD-26`, `WLD-17`).** Five-day sea warmth and daily sea/freshwater ice, fixed sea level, depth and upwelling capacity; explicit safe-thickness query for later bodies.
2. `T3.7b.2` **Soil transitions (`WLD-27`).** Harvest removal/rest/ash/dung/waste/silt inputs through one bounded fertility rule and fixed cadence; fields store their own fertility in area deltas over the cell baseline.
3. `T3.7b.3` **Water-quality API (`WLD-17`, `WLD-15`).** Downstream source/carcass/crossing influence, warm small pools, removal expiry and ash contamination; test event feeds and inspections.

**Tests:** warm/cold opposite ocean-coast fixtures differ; upwelling/shallow water raises the specified capacity, not an invented fish population; sea level never oscillates as a tide.
Repeated harvest-input years lower fertility, rest restores it and additions raise it within 0–5; two fields in one cell can differ without mutating each other.
Sea and freshwater ice respond on intervening days between five-day sea-warmth updates, including save/reopen halfway through that cycle.
Contamination follows ≤about 1 km river distance and expiry, never jumps upstream.
Actual crops, shellfish recovery and gut sickness remain later consumer checks.

**On the phone:** inspect winter ice, a spring versus downstream water, and the soil response graphs.
**Risk:** API tests demonstrate state transitions, not living yield or disease results.

### α3.8a Natural faults and volcanoes

**Goal:** rare events arise only where generated geology permits.

**Serves:** `RES-05`, `RES-13`, `WLD-15`, `WLD-22`, `WLD-27`.

**Architecture:** A7.12.

**Tasks:**

1. `T3.8a.1` **Quake hazards (`WLD-15`, `WLD-22`).** Per-kind annual rates, quiet-until state, keyed opportunities and fault intensity/rockfall intents.
2. `T3.8a.2` **Eruption sequence (`WLD-15`, `WLD-27`).** Days of warning, lava material/burial route, windblown ash and season-long water fouling; fixed heights and no new obsidian.
3. `T3.8a.3` **Rate and cause report (`RES-13`, `RES-05`).** Twenty generated worlds for 100 years, rates by type/eligible area, quiet intervals and saved warning/active/cooldown phases.

**Tests:** every event references an eligible fault/volcano; every quiet interval holds; event-rate shares meet `RES-13` once sufficient opportunities are counted; no height or obsidian delta exists.
Natural runs prove rates; forced diagnostic eruption shows the consumer intents but is not counted as natural occurrence.

**On the phone:** inspect a volcano's warning in a saved natural run, follow its lava/ash and see the cooldown record.
**Risk:** missing M4/M5 damage consumers must not be reported as tested shelter collapse or death.

### α3.8b Disturbance logs and area catch-up

**Goal:** returning late gives the same changed ground, whether or not it was watched.

**Serves:** `PLT-07`, `PLT-10`, `PRN-15`, `RES-05`, `WLD-12`, `WLD-13`.

**Architecture:** A7.12–A7.14.

**Tasks:**

1. `T3.8b.1` **Delta/log format (`WLD-12`, `PLT-10`, `PRN-15`).** Area changes, dated disturbance payloads, cursor/index and retention independent of public-history thinning.
2. `T3.8b.2` **Daily catch-up (`WLD-12`, `WLD-13`).** One routine for inactive kept areas using seasonal usual weather and day-keyed chance; display catches up copies only.
3. `T3.8b.3` **Proof and old-area report (`RES-05`, `PLT-07`).** Daily versus delayed/camera-only replay, exact once-only cell ledger and saved cursors; byte-growth diagnostics before choosing compaction.

**Tests:** 20 changed-area scenes updated daily or once after a season match exactly; arbitrary camera visits leave authoritative saves unchanged; no resource/depletion input is applied twice.
A disturbance older than 25 years still reaches a dependent area; an area with a remaining buried effect is not discarded.
M4 adds actual object timers/marks to the same routine and reruns the proof.

**On the phone:** leave a changed test patch, watch a season elsewhere, return and compare daily/delayed versions.
**Risk:** a renderer that “helpfully” commits catch-up would make observation alter history.

### α3.9a Candidate ranking and start certificates

**Goal:** choose good worlds without repairing them.

**Serves:** `RES-05`, `RES-09`, `WLD-10`, `WLD-11`, `WLD-24`, `WLD-30`.

**Architecture:** A7.8.

**Tasks:**

1. `T3.9a.1` **Two-pass search (`WLD-10`, `WLD-11`).** Ordered 20-candidate coarse pass, six initial finalists and deterministic fallback to 40; versioned scores, total ties and logged rejection reasons.
2. `T3.9a.2` **Start certificate (`WLD-24`, `WLD-10`, `WLD-30`).** Full-resolution shelters/water/stone, season food margin with overlap correction, start-landmass arc resources and A7.1’s species evidence status.
3. `T3.9a.3` **Seed survey (`RES-09`, `RES-05`).** One hundred root seeds, candidate pass counts, proposed G3 definition, own-seed refusal, score distribution and same-three reproduction.

**Tests:** every offered world passes all available hard gates; ≥25% of the declared candidate denominator qualifies; every root seed replays the same ordered result under the same version; no post-score terrain/resource changes.
Force the 0/1/2/3-result and fallback paths with test inputs, separate from natural qualification statistics.

**On the phone:** inspect the ranked candidates, their short summaries and reasons a rejected candidate failed; enter a world seed.
**Risk:** qualification by coarse habitat alone would be a false full-resolution certificate.

### α3.9b Choosing, settling and world lifecycle

**Goal:** a usable New World path and honest staged settling.

**Serves:** `PLT-07`, `PLT-08`, `PRE-29`, `TIM-14`, `WLD-08`, `WLD-10`, `WLD-11`, `WLD-24`.

**Architecture:** A7.9, A7.14.

**Tasks:**

1. `T3.9b.1` **Three-globe choice (`WLD-10`, `PRE-29`).** Show offered previews, one-line fact summaries, choose/top-ranked pick/explicit seed, cancel/resume and no-result explanation in both orientations.
2. `T3.9b.2` **Ten-year settling lifecycle (`WLD-08`, `WLD-11`, `TIM-14`).** Implement the prehistory origin/offset without negative events in today's zero-frontier world; 600-day water/current-system settling, fixed start retained and available gates rechecked.
3. `T3.9b.3` **Saves and honest scope (`PLT-07`, `PLT-08`, `WLD-24`).** Selected candidate becomes one saved world; temporary finalist storage bounded; header records pending ecology/bands and the phased acceptance status.

**Tests:** cancel/reopen preserves existing worlds, choice never regenerates a different candidate, uninterrupted/resumed settling matches and history begins on the correct spring day.
Carry weather stores, quiet times and arrivals through the history boundary without redrawing chance.
No area is built during world generation; visible areas may be requested after opening.
Full living settling and post-settled food/bands remain open under the approved phased scope.

**On the phone:** make a world, choose it, watch labelled settling, save/switch/export it, and replay its seed.
**Risk:** restarting at Year 1 must not erase hazards or make world generation depend on time spent viewing the three choices.

### α3.10a Generated land from globe to cliff

**Goal:** replace α2.9a/α2.10a fixture inputs with generated surfaces and revisions.

**Serves:** `PLT-04`, `PRE-03`, `PRE-23`, `PRE-26`, `PRE-29`, `PRE-30`, `WLD-02`, `WLD-12`, `WLD-13`.

**Architecture:** A8.8, A7.13–A7.14.

**Tasks:**

1. `T3.10a.1` **Fine relief and surfaces (`PRE-03`, `PRE-23`, `WLD-12`).** Metre picture detail constrained to world river/coast/cave/layer anchors; common normals/water/receiver geometry.
2. `T3.10a.2` **Published revisions and bounded jobs (`WLD-13`, `PLT-04`).** Owned immutable base/state, current revision manifests and stale-epoch rejection; coarse parents and measured upload headroom.
3. `T3.10a.3` **Shared overview and whole zoom (`WLD-02`, `PRE-26`, `PRE-29`, `PRE-30`).** Coasts, relief, real cover, rivers, water/ice/weather at the proper bands, physical light and inverse selection at every stop.

**Tests:** 10,000 border samples match, generated rivers never break, no black hole during cold pinch, parent/fine forms retain selected focus and fixed camera; minimum river visibility holds.
Skipped snapshots, cache deletion and rapid world swaps never publish stale land or alter digests.
Full detail arrives within G7's limit on the phone, not merely in cloud screenshots.

**On the phone:** one continuous journey from globe to an unvisited cliff, pan over a river edge and switch worlds mid-load.
**Risk:** unique near-resolution terrain colour explodes memory; shared art and small disposable pieces are mandatory.

### α3.10b Caves and the geological slice

**Goal:** the chosen line exposes real underground records.

**Serves:** `PRE-24`, `PRE-25`, `PRE-31`, `WLD-09`, `WLD-17`, `WLD-27`.

**Architecture:** A8.9.

**Tasks:**

1. `T3.10b.1` **Generated shelters (`PRE-24`, `WLD-09`).** Materialize certificate geometry into M2's floor/back/roof/opening contracts; deep-cave entry/exit preserves exterior focus.
2. `T3.10b.2` **Section query (`PRE-25`, `WLD-17`, `WLD-27`).** Arbitrary chosen line, seam-safe distance, analytical feature intersections, rock contacts/soil/caves/water head, labelled scale and section width.
3. `T3.10b.3` **Buried-record adapter and review (`PRE-25`, `PRE-31`).** Show true-depth test records through the future trace interface, clearly marked synthetic; no claim of a simulated 200-year camp.

**Tests:** a cliff face and intersecting section show identical contacts; cave openings/floor/roof and water-table values match queries; seam lines use correct local distance; zero-length/very long cuts are bounded/refused clearly.
All included records obey the displayed section width and retain stored depth; repeated cutaways change no state.

**On the phone:** draw a line through a cliff/cave and river, inspect the layers and water, enter the cave and return.
**Risk:** roof reveal is not a geological slice; coloured rock bands are not saved archaeology.

### α3.11a Generation and steady-world performance

**Goal:** measure the costs before claiming the targets.

**Serves:** `PLT-04`, `PLT-10`, `RES-05`, `WLD-11`, `WLD-12`, `WLD-13`, `WLD-15`, `WLD-17`.

**Architecture:** A18 additions.

**Tasks:**

1. `T3.11a.1` **Cost ledger (`PLT-04`, `WLD-11`).** Stage wall/core time, full/fallback search counts, physical field/feature/scratch/cache/save bytes and worst-case overflow reporting.
2. `T3.11a.2` **Measured optimization (`WLD-12`, `WLD-13`, `RES-05`).** Remove duplicate arrays/serializations, improve ordered kernels and exact unchanged-cell work; no camera-dependent cadence or timed candidate cutoff.
3. `T3.11a.3` **Worst-case scenes (`WLD-17`, `WLD-15`, `PLT-10`).** Many basins/reaches, large floods/ash footprints, old changed-area logs and snapshot/export overlap; compare optimized/reference digests.

**Tests:** every optimization gives identical state to its reference; counts/allocations cannot grow without a declared bound or memory notice; report normal daily and flood-day world-layer cost against 0.2 core-s/day.
Cloud numbers only guide work; no phone pass is claimed here.

**On the phone:** open the generation/world cost report and run the short timing sampler.
**Risk:** frequency times two million cells dominates even when individual rules are cheap.

### α3.11b Held-phone, streaming and recovery route

**Goal:** measured end-to-end performance on the actual phone, with M2 visuals active.

**Serves:** `PLT-04`, `PLT-07`, `PLT-08`, `PLT-09`, `PRE-03`, `RES-05`, `WLD-11`, `WLD-13`.

**Architecture:** A18, A17.

**Tasks:**

1. `T3.11b.1` **Phone route (`WLD-11`, `PLT-04`, `RES-05`).** G7 fixed roots, fallback cases and final hashes after at least three minutes of load, unplugged/in flight mode; report median/p95/max and stage times.
2. `T3.11b.2` **Concurrent view/save stress (`PRE-03`, `WLD-13`, `PLT-07`).** Twenty-minute generated-world pan/pinch/weather run, cold area requests, cache churn, periodic saves, switch/reopen/export and frame/heat/memory ledgers.
3. `T3.11b.3` **Recovery and corpus (`PLT-08`, `PLT-09`, `RES-05`).** One hundred random kill points in cloud, corrupted chunks and valid prior-state recovery; phone subset and old-world reopen, retained M1 proofs.

**Tests:** G7 numbers are present as met/missed; phone/cloud state hashes match; every frame minute and memory peak reported; unavailable sensors explicitly unavailable.
Warm open ≤3.3 s approved outer limit under the approved timing convention, target 3 s; no catch-up silently changes history.
Repeat affected renderer route on the named weaker M2 phone where available; its absence is reported, not invented.

**On the phone:** run the one-tap review route and send its code; try the cold globe-to-cliff journey and reopen afterward.
**Risk:** cloud timing or an average frame rate cannot prove held-phone latency and heat.

### α3.12a Held-out worlds and final visual repair

**Goal:** numerical and visual evidence from worlds never tuned against.

**Serves:** `PRE-03`, `PRE-25`, `PRE-29`, `PRE-31`, `RES-05`, `RES-09`, `WLD-08`, `WLD-09`, `WLD-10`, `WLD-14`, `WLD-16`, `WLD-24`.

**Architecture:** A17, A8.8–A8.9.

**Tasks:**

1. `T3.12a.1` **Closing seeds (`WLD-08`, `WLD-09`, `WLD-10`, `WLD-14`, `WLD-16`, `WLD-24`).** Freeze build/config and draw new recorded seed sets; run G1–G6 with checkpointed reports and failed-world exports.
2. `T3.12a.2` **Phone contact sheet (`PRE-31`, `PRE-03`, `PRE-25`, `PRE-29`).** Every stop at noon/dusk and full-moon night, one landscape, cliff/cave/section, coast/river/lake, polar seam and panning/pinching clips; real weather/seasons, moving cloud shadows and visible currents; retain the approved M2 PRE-31 route.
3. `T3.12a.3` **Fix and preserve gates (`RES-09`, `RES-05`).** Repair faults, rerun affected tests and new held-out tests after tuning; retain failures and unchanged thresholds in the report.

**Tests:** applicable G1–G7 gates pass without weakened rules, owner judges land credible, all new features have a cause/data inspection and held-out results name their actual counts.
Missing ecology/bodies/archaeology claims remain in the coverage and remaining acceptance table below.

**On the phone:** compare the worlds and contact sheet, follow rivers to their destination and inspect the geology.
**Risk:** selecting only attractive screenshots hides generator failures; include fixed random seeds and every worst diagnostic case.

### α3.12b Independent review and M3 close

**Goal:** an accepted milestone and the next detailed plan.

**Serves:** `PLT-04`, `PRC-09`, `PRC-11`, `PRN-14`, `PRN-16`, `RES-05`, `RES-06`, `RES-09`, `RES-22`.

**Architecture:** existing A17/A18 and `PRC-09`.

**Tasks:**

1. `T3.12b.1` **Fresh milestone critic (`RES-05`, `RES-09`, `RES-22`).** Independent subagent gets original approved requirements/plan, diff and test evidence, draws its own affected views, checks requirement claims and threshold changes; fix findings.
2. `T3.12b.2` **Milestone report (`RES-06`, `PRN-16`, `PLT-04`).** Added work, numerical and phone results, every principle's check, costs, risks and explicit deferred obligations; plain things for the owner to try.
3. `T3.12b.3` **M4 handoff and delivery (`PRN-14`, `RES-22`, `PRC-11`).** Plan M4 in detail with A9, the single ecology/catch-up/trace interfaces and deferred closure checks; deliver 41202 and request owner's stage acceptance.

**Tests:** final delivery check and independent review approve; report links open; owner accepts M3 under the agreed scope and M4's plan.
A report or the earlier design review is not the independent implementation milestone review.

**On the phone:** make and choose a world, tour it through weather, slice its ground, read the short result report and decide whether to close M3.
**Risk:** closing on a design alone; the build, phone evidence and owner review are all required.

### Acceptance matrix: answered decisions and remaining implementation definitions

The numerical rules from PROJECT remain binding.
Where PROJECT gives “about,” the default tolerance is 10%; the table states a stricter nominal target and any proposed interpretation rather than silently replacing it.
The owner decisions below are approved as of 8 October 2026; remaining detailed thresholds, reference tables and scene definitions must be frozen under `RES-09`/`RES-22` before tuning.
Failures of chance tests follow `RES-13` fresh-seed reruns; never use repeated retries to discard the failed sample.

| Gate | Data / exact proposed check | Runs and budget | Task owner |
|---|---|---|---|
| G1 Terrain (`WLD-08`, `WLD-09`) | Final 100-seed land/tilt range-and-spread check from α3.2a, then each terrain world: ≥95% of independently enumerated river reaches with contributing area ≥50 km² terminate at sea/lake; no routing cycles; land-cell median slope 0.5–5°, <1% >30°; lakes 1–3% of land; no coast run >22 km whose contour stays within 500 m of its endpoint chord. Primary slope: central differences at 1 km, land-only, permanent cap excluded; report including cap too. Fine 25 m/1 m slopes and cliff footprints separately, never substitute them after a failure. Land denominator excludes sea; lake fraction = freshwater lake footprint / (dry land + freshwater lakes), excluding permanent ice. | 20 fresh full worlds, all meet geometric gates; initial 2 session-hour cap, checkpoint and report overrun | T3.3b.3, T3.12a.1 |
| G2 Geology (`WLD-09`, `WLD-14`) | Every range/fault/volcano refers to its causal boundary/province and falls within its recorded influence envelope; chalk/limestone have marine/basin provenance. Every flint/chert/gravel, hammer/grinding stone, obsidian and copper occurrence passes its host/exposure/provenance predicate; matched lithology/size stones never become less rounded with greater routed transport distance. Inspect independently built expected-host tests, not only call generator predicates again. | 20 worlds for rock, 100 for deposits; 4 session-hours; no invalid occurrence | T3.2b.3, T3.4b.3 |
| G3 Candidates/start (`WLD-10`, `WLD-24`) | 100 root seeds; ≥25% qualify in aggregate among the first 20 candidates of each root using full-resolution qualification in the offline audit (2,000 candidates), with per-root results also reported, so the denominator cannot be improved by counting only finalists. Every offered world passes final gates; same seed/version gives identical ordering. Approved food margin ≥20%, shelters ≥2 m²/person, cold mean 2–10°C, water ≤2 km, accessible food/stone ≤10 km; physical/food gates recheck selected start after real settling as available. Frost “few” proposed 2–5 nights per cold season. | 100 roots and paired determinism replay; 8 session-hours initial cap, may require owner-approved larger audit budget; full biological confirmation remains in M4/M5 | T3.9a.3, T3.9b.3 |
| G4 Climate/weather (`WLD-16`) | 20 worlds ×20 years, every place's accumulated rain within ±10% of its climate and mean temperature ±1°C; online per-cell accumulators, not selected attractive locations. For near-zero rainfall use an explicitly approved absolute rounding allowance of one stored precipitation unit over the full run, report all such cells. Earth matched-bin warmth target ±2°C (about: outer ±2.2°C), ≤10 percentage-point climate-class share difference in matched 10-degree bands under the approved comparison; rain-shadow/coastal tests independent of that comparison. | 2 session-hours initial cap; fixed natural runs, checkpointed, failed chance checks rerun per RES-13 | T3.4a.3, T3.6b.3 |
| G5 Sky/hazards (`WLD-07`, `WLD-15`, `WLD-22`) | Four full moons/year; approved sky oracle error ≤0.1° direction and ≤2 minutes daylight in non-singular test cases, explicit polar cases. Eclipse acceptance requires 3–8 visible solar/lunar events at every place over 70 game years, clouds or not: 100 stratified places for diagnostics, then every canonical world-cell location plus certified bounds over intra-cell horizon/footprint crossings; report spatial minimum/maximum and fail any uncertified region. The complete 100-year runs record zero polar-barrier crossings by every implemented transport consumer. Every quake/eruption has valid location, origin and cooldown; per-kind event rate judged by RES-13 over ≥1,000 opportunities, using the promised share tolerance (rare shares half-to-double). | 20 worlds ×100 years for hazards; 4 session-hours; use analytical alignment/visibility queries for the eclipse proof rather than run unrelated ecology; initial 2 additional session-hours for full spatial coverage, checkpoint/report any overrun | T3.6a.3, T3.8a.3 |
| G6 Area/water/slice (`WLD-12`, `WLD-13`, `WLD-17`, `PRE-25`) | 10,000 shared edges exact (stronger than ≤0.55 m outer tolerance); unchanged same-date regeneration identical; 20 kept-area scenes daily vs after season exact; 20 natural generated-valley scenes, at least 16 meet declared flood/spring behavior, rerun as RES-13 says. Section contacts/water/depth match source values exactly at sample points; zero camera-caused save changes. | 2 session-hours; small deterministic kernels in quick checks, longer natural scenes background | T3.5b.3, T3.7a.3, T3.8b.3, T3.10b.3 |
| G7 Phone (`WLD-11`, `PLT-04`, `PRE-03`) | Held speed after ≥3 minutes load, unplugged: 20 ordinary root searches plus 5 saved roots known to use fallback, fixed before measurement; all target ≤180 s and outer ≤198 s under the approved timing convention. Selected settling target ≤60/outer66 s, ecological/band completion later. Areas target ≤0.1/outer0.11 s; visible detail ≤1/outer1.1 s; warm open ≤3/outer3.3 s. ≥97% frames on time at the selected 60/30 fps mode, none more than 50 ms late (maximum frame duration 66.7/83.3 ms respectively), every minute of the 20-minute route; world layers ≤0.2 core-s/game day nominal, plus actual world-alone throughput ≥10 game years/minute on two middle cores for the current systems. M4 repeats with full ecology. Peak process ≤2 GiB under the approved memory line. GPU/heat lines retain M2's approved route. Every physical digest matches cloud. | Approximately 20-minute play route; 25-search generation suite separately about 100 minutes at nominal limits or 110 minutes at outer limits including one selected settling run per root, plus setup; resumable between roots. Do not conceal it in the promised short review route. | T3.11b.1–3 |

The 20-world weather gate quantifies an unusually strong **every place** promise.
If it fails from stochastic variance or the Earth protocol cannot fit the small world, report that evidence to the owner before changing quantifiers, tolerances or climate structure.
No literature result guarantees this acceptance rate.
Whole-world event runs use no player interventions or test switches.
Test-only injected events prove mechanisms, with reports carrying their switch, never natural frequency (`RES-18`).

**G1 audit denominator:** derive all channels draining at least 50 km² from the final drainage graph, independently of the renderer or generator's river-registration threshold. Split reaches only at headwaters, confluences and terminal water bodies, with segmentation frozen before tuning. Missing registered geometry is a failure, not a removed denominator entry.

**G4b natural rates (`WLD-16`, `WLD-22`, `WLD-30`):** T3.6b.3 and T3.7a.3 own a shared suite for storms/storm days, lightning, droughts, naturally triggered floods and harsh winters, each by climate. Reuse G5's 20-world/100-year runs where possible, with 2 additional session-hours initially for reference preparation and diagnostics; retain checkpoints and seek a larger budget if necessary. Freeze per-climate Earth targets, event thresholds, storm identity/de-duplication, exposure denominators and short-year conversion before tuning. Every event must trace to its natural system; no injected weather counts. Apply RES-13 to each promised rate with at least 1,000 eligible cases per tested kind/climate, report uncertainty and missing coverage rather than pooling away a failing climate. Wildfire rate and lightning-to-fire conversion remain M4 under the phased scope.

Use the approved wet-day threshold (1 mm/day) and report each place's mean seasonal storm days and longest annual dry-spell distribution. Approved climate definitions: mean storm days within the RES-13 share bounds of the saved climate; the saved longest-dry-spell envelope denotes the 95th percentile of annual maxima, with its observed exceedance share checked against 5% under RES-13. Compare drought/harsh-winter event definitions and flood exceedance thresholds to independent Earth reference definitions, not generator labels. You approved the dry-day/dry-spell interpretation and unit convention on 8 October 2026; freeze the remaining per-climate reference values and event thresholds before tuning. A correct rain total with wrong persistence must fail. For lightning, total-flash data and ground-strike/fire opportunities are different quantities; conversion needs an explicit sourced rule.

The session-hour allocations are initial caps per suite, not claims about measured runtime.
Record actual core count, CPU time, wall time, version and storage for each.
A 100-year ×20 empty-world run is 120,000 game days; even at 0.2 core-s/day it is 6.67 core-hours, so parallelism and checkpointing matter.
If a suite exceeds its stated allocation, keep its checkpoint and raise the needed budget; do not shrink its sample and call the original check passed.
Train on fixed named seeds, close on new seeds; candidate-quality tests deliberately audit candidates not only selected winners.

### Coverage and remaining acceptance

| Item group | Delivered mechanism in M3 | Full check that remains elsewhere under the phased scope |
|---|---|---|
| WLD-01/02/03/06 | Grid/barrier, real overview/disc input, dimensions/tilt/land | Future walker/herd/fire consumers must rerun no-cross checks in M4/M5 |
| WLD-07 | Sun/moon/stars/eclipses | Learning sky cycles in later culture |
| WLD-08/09 | Whole causal generator and 600-day lifecycle; physical systems settling | Full cover/herds play-rule settling in M4; no false claim of already complete ecology |
| WLD-10/24 | Rejection/ranking/start certificates and available physical/initial-species gates | Actual food/yield sufficiency and settled species occurrence M4; naked-band winter survival M5 |
| WLD-11 | Full generation-to-choice timing and current-system settling | Full ecological settling timing M4; bands-included time M5, rechecked each stage |
| WLD-12/13 | Layers, paces, pure queries, deltas/catch-up and immutable view publication | M4 plants/things/timers and M5 near-person activation add consumers to the same proof |
| WLD-14 | Geologic deposits and stable patch identity/exposure | Digging/gathering creates real things in M4/M5 |
| WLD-15/22 | Natural hazards, rates, warning/quiet times and effects records | Things/plants/fire M4; injuries/deaths M5; god controls M9 |
| WLD-16 | Fixed climate and live hourly weather | Powers use its bounds in M9, no second weather system |
| WLD-17 | Water/snow/ice/floods/springs and water-quality inputs | Floating/carrying things M4; fouled-water illness and bodily ice safety M5 |
| WLD-26 | Sea/currents/upwelling/ice and habitat capacity | Actual fish/mammal totals and stripped shellfish regrowth M4 |
| WLD-27 | Soil/fertility rule with actual weather and declared mutation inputs | Actual crop removal/yield/recovery plus dung/waste consumers M4/M10 |
| WLD-30 | Geographic, event-rate, climate-index and initial capacity scaling | Actual foraging 25-person/100–300 km² scene M5; every yield/animal entry M4 |
| PRE-03/29 | One generated landscape and weather from cliff to globe | Later real people/herd/building aggregate consumers at their own milestones |
| PRE-25 | True geological/water section and buried-record adapter | 200-year real hearth/bones/tools/graves from M4 traces, then M5 human-world confirmation |

Every M3-served item above has tasks and tests; a partial acceptance line is deliberately still open.
The principles review must show that caches never create facts, generation never patches a failed candidate, natural events keep their causes, history is stored, and no model output enters the running world's decisions.
The earlier research and design review does not substitute for the fresh implementation critic at α3.12b.

## M4 Things and living nature

**Goal:** the world's matter and life, before any people (A9, A12):
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
- Nature's numbers staying believable over a century (`WLD-18`): a first version held them.
- The world alone reaching its speed (`TIM-07`).

## M5 People: bodies and lives

**Goal:** people with bodies that live, act and die for real reasons (A10, A11):
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
- Pixel-art figures, each activity with its own movement, extending M2's approved frames.

**Risks:**
- Thousands of people at speed on the phone (`MND-15`, `TIM-07`): a thousand simple minds kept 6 game years a real minute.
- Paths for many walkers at once.

## M6 Minds

**Goal:** the inner life (A11):
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

**Goal:** the heart of the arc, tuned to its windows (A12):
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

**Goal:** peoples with their own words, customs, beliefs, rites, art and music, made by what happens to them (A13):
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

**Risks:** culture forming too fast, too slowly, or the same in every world (`CUL-33`, `RSK-19`): a first version formed it from causes alone.

## M9 The game

**Goal:** the surface you play (A14, A15, A16):
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

**Goal:** the full launch catalogue and the pace:
- pottery, herding, farming, villages and copper;
- the chances tuned until worlds go from caves to copper in a few hundred years, at a watchable speed.

**Serves:** `TIM-19`, `CUL-28`, `CUL-33`, `MAT-23`, `RCK-04`, `RCK-08`, `WLD-33`, `RES-07`, `RES-12`, `RES-16`, `RES-25`, `MOM-08`, `MOM-12`, `VIS-14`.

**You will see:**
- Pots, herds, fields, villages and the first copper, each in its own time.
- The pace tests' charts in the report.

**Risks:**
- The pace needing long tuning (`RSK-26`).
- The phone's speed with thousands of people (`TIM-07`).
