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
  - before M3, whether worlds have mammoths, since mammoth-bone shelters belong only where they live.

## Status

The first five 2D alphas follow the adopted build order.
Numbering continues after the old 3D route, so begun task IDs and delivered version codes are never reused.
If a build exceeds six hours, split it at a task boundary into lettered deliveries, as the conventions require.

| Step | Title | Milestone | Hours | Status |
|---|---|---|---|---|
| α2.7a | Projection and six fixtures | M2 | 6 | Engine built; art review and delivery pending |
| α2.8a | Flat shadow style and terrain prototype | M2 | 6 | Planned |
| α2.9a | Zoom and streaming | M2 | 6 | Planned |
| α2.10a | World map and globe | M2 | 6 | Planned |
| α2.11a | Two-phone renderer comparison | M2 | 6 | Planned |
| α2.12a | Art scale-up, cleanup and M2's end | M2 | 6 | After the five fixture and phone gates |
| M3 to M10 | Outlines below | M3 to M10 | | Detailed when each comes next |

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

The six previews are labelled developer fixtures: approved-sheet crops for tree, shelter, boulder and ground, with diagram person and animal walk/work frames. Source sheets, pivots, four/eight-facing comparison, alpha inspection and colour/object/material capture passes are wired. Actual normal and material art can be inspected when supplied; those channels are currently absent. Dusk is a diagnostic tint. This is engine evidence, with `T2.7a.3` art approval, seasonal shapes and delivery 30701 still pending the art worker and coordinator. Rich lighting remains in α2.8a.

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
   Plan M3 in detail and write its architecture in full for the owner's stage review; never close M2 from a design or painted sheet alone.

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

**Goal:** whole worlds, made from a seed in the order of real causes and tuned by eye until the map looks right at every zoom stop (A7, A8):
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
- Making a world within about 3 minutes on the phone (`WLD-11`): a first version made three in 9.3 s, so the stages can grow richer.
- Believable land needs tuning by eye.
- Memory for detailed areas.

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
