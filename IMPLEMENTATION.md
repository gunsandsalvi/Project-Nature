# Kindling: implementation plan

The order in which Kindling is built.
It follows `PROJECT.md` (what the game must be) and `ARCHITECTURE.md` (how it is built), and cites both: items by ID (`TIM-16`), parts of the architecture by section (A3.4).
It builds bottom up, as you asked: the foundations first, then the graphics engine, the world, living nature, people, minds, crafts, culture, and the game itself last.
Every step ends with a build on your phone, and every milestone with a report you review (`RES-06`, `RES-22`).

Only the next milestone is planned in detail: the graphics engine (M2).
The later ones are outlines (their goal, the items they deliver, what you will see), each detailed when it comes next, from what the earlier ones taught.
The plan holds only work still to do: a step leaves it when it is done, and the code, which names the items it implements, is the record (`CLAUDE.md`, rule 3).

## Status (6 October 2026)

- The ten milestones were approved by you on 4 October 2026, with their proposals, now decided in `PROJECT.md` (`SCP-16`, `MIL-08` to `MIL-17`); the early steps are tried rather than played (`PRN-09`, `SCP-03`, `PRC-11`).
- The risks were tried first, and that work closed on 5 October 2026, as you asked: its answers are decisions in the architecture.
  None of its code is carried into production, which writes its own.
- The foundations (M1) are built, eleven steps in five alphas delivered as 20101 to 20502, and you accepted them on 6 October 2026: your phone's benchmark met all 18 of its pass lines, and its self-check matched the cloud in all seven suites.
- The vertical slice was dropped on 6 October 2026, as you asked.
- The graphics engine (M2) is being built: you chose its look and OK'd the 13 changes to `PROJECT.md` that follow from it.
  You OK'd its plan below, fifteen steps in six alphas, one of them only if needed, and its sections of the architecture (A4, A5, A6 and A8's near stops) on 6 October 2026: "Yes, that works. Let's start".
- **The art lane** works beside the builder (below).
- **Still open from pre-production:** the ground for the card and the book, and whether reading text should be larger (P12); your ears on the camp's sound (P14); the heat of a busy scene over ten minutes (P2); the writer, proved when M9 builds the book (P13); discovery's pace with the whole catalogue, at M7's scenes (`RSK-01`).

## The art lane

A separate instance, your idea, prepares the content while the builder builds the engine: the picture-made textures (α2.3a's tools and materials, T2.3a.2 and T2.3a.3), the targets and guide pictures, and the kit's parts in Blender (A6.1), each first drawn by GPT and then fixed by the lane.
The builder reviews and merges each batch and wires its content into the engine at the step that uses it; you say yes or no to each material and part on its sheet.
It restarts with your word, with its next round below as its one order.

**Rules:**
- It works only in `art/`, `tools/art/` and `tools/tests/test_art_*.py`, on its own branch, and never pushes or merges.
- GPT only through `tools/art/gpt-run.sh`, outside every build; never print or move a secret; never buy credits; at a usage limit, wait; while `/tmp/kindling-gpt-paused` exists, runs are held.
- As many pictures as improve the result, each counted in the batch's report.
- No AI model's name in any committed file, and nothing secret in a prompt.
- GPT's size, block size, scale and seams are never relied on: re-gridding finds them.
- Colour measures only from `kindling look` (`stats`, `adjust`, `texel`), the engine's own C++.
- Blender (4.0, in the cloud) for every part; GPT's first pass through Codex writing Blender scripts, run headless, then the lane's own fixes.
- Each tool has unit tests and passes `ruff`; commits are `T2.3a.2: …` for tools and `T2.3a.3: …` for materials.
- Truth before use: every picture enlarged before it becomes a source; never metal before copper, sawn wood, later things (chickens, hooped buckets, winches, lattice windows, glass-bead colours, rucksacks, slatted sleds, maize, a pot hung over a fire, boats with seats), spotted or long-maned horses, striped piglets outside spring, tipi-like cones, Lascaux-like paintings, real cultures' motifs, fur bikinis, grass rain capes; and nothing countable in a ground texture.

**Files:**
- `art/textures/<name>/`: levels `b0.png` to `b8.png` (lossless), `record.toml`, and `source.png` (the source on band 0's grid); a big surface's middle and far tiles in `middle/` and `far/`, and versions in `v2/` to `v4/`, each with its levels and record.
- `art/models/<family>.blend`: the kit's parts, one Blender file for each family (camp things, plants, rocks, people, each animal pattern), each part with its texture layout in metres, its named joints (empties called `joint_…`) and its material slots named by role (wood, bark, hide, stone, leaf, grass, skin, hair); beside each file a preview sheet and a stretch report.
- `art/sources/<name>/<name>-<nn>.webp`: each original used; `art/requests/<name>-<nn>.txt`: its request; `art/sheets/<name>.webp`: the sheet, 1080 pixels wide.
- A request: purpose, size, input picture, and the prompt (the area in metres, blocks of 8 to 12 picture pixels or larger for a coarse band, the material only, straight on and orthographic, even light, seamless, a truth line).
- A record, integers and strings only: `about`, `route`, `tile_texels`, `texels_a_metre`, `first_band`, `sources`, `original_sha256`, `c2pa`, `requests`, `made`, `regrid_loss`, `truth`, `approved`, and for each level `level`, `file`, `sha256`, `made_from`, `way`, `regrid_loss` (redrawn levels), `calibration`.
- The sheet: every band at true size and enlarged, flat and under three stand-in lights, beside its source; a big surface's bands also as full-width strips with its versions mixed as the ground mixes them.

**Its next round,** one order in two phases, Blender first, as you asked on 6 October 2026 ("First Blender, then the textures to match where necessary"):

*Phase 1: the kit's parts in Blender (T2.3a.4).*
1. **The tool:** `tools/art/gpt-blender.sh`, which hands Codex a written request to write a Blender script and run it headless in a scratch folder, held by the same pause file, with its tests.
2. **The camp's parts,** GPT's first pass, then the lane's fixes, in `art/models/camp.blend`, `plants.blend` and `rocks.blend`:
   - wood: poles (straight and slightly bent, 1.5–3 m), logs, branches and a birch trunk in segments;
   - covers: hide panels (cut shapes, flat and draped), bark sheets (curved);
   - stone: hearth stones and tent-ring stones (several of each), boulders, cliff pieces and scree;
   - things: a basket, a drying rack's bars, a hand axe;
   - plants: grass tufts and leaf clusters as cut-out cards;
   - each at real size in metres, its origin at its main joint, its texture coordinates in metres (one texture pixel 1/64 m), stretched at most 1.5:1, a pole's circumference rounded to whole texture pixels, its joints as empties named `joint_…`, its material slots named by role.
3. **A first person and a red deer,** in `art/models/people.blend` and `deer.blend`: each body of parts (head, torso, arms, hands, legs, feet; for the deer its hoofed pattern and antlers by age) skinned to its skeleton, with build, age and sex as shape keys; the person's three garments as parts on the same skeleton.
4. **For each file:** a preview sheet (every part at real size beside a 1 m bar, with a checker of 64 pixels a metre showing any stretch) and a stretch report; committed when phase 1 is done, before phase 2 begins.

*Phase 2: textures to match (T2.3a.3).*
5. **Textures for the parts** where they need their own: hide, bark, peeled and unpeeled wood, stone, basket weave, skin, hair and garments, drawn to fit each part's layout and joints, as materials with their levels.
6. **The ground's tiles:** middle and far tiles, each with two to four versions, for the meadow, bare earth, trodden floor, bank gravel, river bed and rock A; band 1 redrawn by GPT for bare earth, trodden floor and bank gravel; bare earth, trodden floor, stone and ash redrawn without GPT's stepped-diamond pattern; the meadow's band 0 stays.
7. **The code reduction** draws bolder marks and fewer of them, never single-pixel speckle; **the checks** as T2.3a.1 lists them; **the sheets** with full-width strips.

**The batch report:** each material and its sheet, each check's result, the pictures used, every truth flag, and questions for you.

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
  - the blind tests and the choices by eye its steps ask for: the camera's tilt and lens up close, poses held or gliding, moving patterns stepping in whole texture pixels, each lever that may show, each material's sheet, the rock surface, the close camp's ground and the small plants at each band;
  - how builds and textures reach your phone, if textures outgrow the 50 MB a committed file may have;
  - before M3, whether worlds have mammoths, since a mammoth-bone shelter belongs only where they live.

## Status

| Step | Title | Milestone | Hours | Status |
|---|---|---|---|---|
| α2.1a | The picture and the bench | M2 | 6 | Delivered as 30101 on 6 October 2026; your phone's readings in, every probe passed |
| α2.1b | The look's checks | M2 | 6 | **Building.** T2.1b.1 done: the look's measures and FLIP (`kindling look`); T2.1b.2 next |
| α2.2a | Calibration: the fixed cost, the material and triangles | M2 | 4 | Planned |
| α2.2b | Calibration: leaves, fires and figures | M2 | 5 | Planned |
| α2.2c | Our own build of Godot, if needed | M2 | 6 | Only if C1 or C2 call for it |
| α2.3a | The texture path and the camp's materials | M2 | 6 | The art lane's batch 1 joined on 6 October 2026: its tools and 18 materials; its next round below waits for the lane's restart; the rest after α2.2 |
| α2.3b | Ground, cliff, water and light | M2 | 6 | Planned |
| α2.3c | Plants, shelters and fire | M2 | 6 | Planned |
| α2.3d | People, a deer and first light | M2 | 6 | Planned |
| α2.4a | Stress scenes | M2 | 5 | Planned |
| α2.4b | The heat run and the line | M2 | 3 | Planned |
| α2.5a | The hours | M2 | 5 | Planned |
| α2.5b | Seasons and weather | M2 | 6 | Planned |
| α2.6a | The zoom bands | M2 | 6 | Planned |
| α2.6b | People in busy scenes, landscape and M2's end | M2 | 6 | Planned |
| M3 to M10 | Outlines below | M3 to M10 | | Detailed when each comes next |

## M2 The graphics engine

**Goal:** the engine that draws the world up close in the look you chose on 6 October 2026, a sharp 3D world at the phone's full resolution wearing pixel-art textures, toward the pictures you chose (A4, A5, A6, A8):
- the full-resolution picture, with steady texture pixels and a level of every texture drawn for each zoom band;
- smooth light true to the hour and season, with our own soft shadows and darkening;
- every material from code, the world or approved pictures; plants as dense and airy as you liked them; water, fire and weather;
- the model kit: shapes made in Blender as parts and put together like Lego, with detailed people and animals that bend at the joints;
- the camera's gestures, in portrait and landscape;
- your phone measured part by part, so what fits is known before content grows.

It is built in six alphas: measuring, calibration, first light, the phone's risks, the hours and seasons, then the zoom bands and your three problems.
Its content stands in for later milestones': a camp under a cliff by a river, made by code where M3 will make whole worlds, with people and animals that M4 and M5 bring to life.
Each step lists the pictures it asks GPT for; GPT stays outside the build (A5.4).

**Serves:** `PRE-01`, `PRE-02`, `PRE-03`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-27`, `PRE-28`, `PRE-30`, `PRE-31`, `PRE-33`, `PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`, `PLT-02`, `PLT-04`, `VIS-14`, `RES-06`, `RES-22`.

**You will see:**
- First, your phone measured part by part: what the look costs, a switch for each lever, and blind tests wherever a saving might show.
- Then first light: the camp under the cliff by the river at the closest zoom, as the engine draws it, beside the picture you liked.
- The camp at dawn, true midday, dusk and night, in winter, rain, storm and lake mist, each beside its relit target, steady as you drag, pinch and turn.
- The camp of thirty at its tasks, detailed people and a deer, and every person found at a glance in the busy scenes.
- The zoom from the person out to the camp, each band's textures drawn for it, the small plants keeping their charm and the ground no longer speckled.
- This is the quality gate: you judge the scenes on their own, as the bar for everything built on them (`MIL-09`).

**Risks:**
- The look costs too much at full resolution (`RSK-30`): about 13–45 ms of the graphics chip drawn plainly, against a line of 8 (estimates); retired first by calibration (α2.2), then by A4.1's levers, our own build of Godot only if needed.
- The phone heats in long play: retired by the 20-minute heat run (α2.4b).
- The PowerVR driver mishandles a feature the look needs: each probed in the first build (α2.1a).
- Texture pixels shimmer or a band's level pops; designed levels are much content work; people stay hard to find in busy shade; the rock surface under each world's layers is still to find.

### α2.1a The picture and the bench

**Goal:** the engine's spine on your phone: the world drawn at full resolution with 2× MSAA on a page of its own, the camera rig and its gestures, textures read through the one sampling function with levels of our own, and the bench reading each part's cost with power and heat, after every feature the look needs has been probed safely.

**Serves:** `PRE-01`, `PRE-02`, `PRE-22`, `PRE-33`, `PLT-02`, `PLT-04`, `VIS-14`.

**Architecture:** A4.1, A4.2, A4.6, A4.7, A8.4, A3.9, A18.1.

**Tasks:**

1. `T2.1a.1` **The picture and the Look page (`PRE-01`, `PRE-02`).**
   A Look page draws a 3D scene on the Mobile renderer straight into the window at full resolution with 2× MSAA, the interface over it, and nothing that reads the screen or its depth.
   Drawing switches for each part (MSAA off, 2× and 4×; the 3D at 1.0, 0.75 and 0.5 scale; each pass on or off) touch only the drawing, never the world (`WLD-13`).
   The stage, the change feed and the first family drawing through the RenderingServer (A4.6), and the warm-up scene.
2. `T2.1a.2` **The camera rig and its gestures (`PRE-33`, `PLT-02`).**
   One perspective rig (A8.4): narrow up close, 5° or 10° across by a switch, tilted 35–40°, with the ease to 5° and 1.25× steps; drag, pinch and turn read by our own gesture reader on raw touches (A15).
   The rig's state and the globals it publishes (the zoom band, the sun, the time, the moving origin); scripted camera paths (a pan, a full turn in eased steps, a pinch through a band); turning the phone keeps the focus, the turn and the metres per screen pixel.
3. `T2.1a.3` **Textures with our own levels (`PRE-22`).**
   `view/` reads lossless texture files holding every level, builds each image with `Image.create_from_data` and gathers texture arrays by size class; the one sampling function as a shader include (the smooth-pixel filter with `textureGrad`, the level from the texture pixel's area, the short blend).
   Stand-in textures made by code at 64 texture pixels a metre, with designed levels down to 1 a metre: a meadow and a test pattern.
4. `T2.1a.4` **The bench's new readings and the heat guard (`PLT-04`).**
   Each viewport's graphics time and Godot's timestamps for each pass; Godot's counters (triangles, draws, video memory); Android's GPU headroom where offered; power every 2 s from the battery, and the rails where offered; the phone's heat thresholds (API 35), its headroom listener (API 36) and the minutes to its light threshold; the bench code's next layout with these fields.
   The heat guard acts at the light threshold less 0.05, and a missing reading is no reading (A3.9).
5. `T2.1a.5` **Probes, the self-check and delivery (`VIS-14`).**
   Each probe is noted before it runs, so a crash names it on the next start: MSAA at full resolution, `textureGrad`, texture arrays with our own levels, alpha to coverage, a shading rate for each draw, a MultiMesh shader reading bone weights, and one GPU particles node, expected to fail.
   The self-check lists the shading rates, the GPU headroom and the power rails; deliver as 30101.

**Tests:**
- gdUnit4, headless: the Look page builds; every switch changes only the drawing (a world's digest is unchanged with each switch flipped); the rig keeps its metres per screen pixel through a simulated turn of the phone.
- doctest: the level picked for a texture pixel's area keeps it between 1.4 and 2.8 screen pixels at every zoom; the bench code's new fields read back as written; the heat guard's response to planted readings (a missing one, the light level, the moderate level).
- A golden picture of the meadow at the closest zoom, drawn in the cloud, its texture pixel measured at 1.5–3 screen pixels.
- Passes if all pass, the app installs over the last one, and every probe reports on the phone.

**On the phone:** open Look: a pixel-art meadow at the closest zoom; drag, pinch and turn it, flip the switches and watch the frame time; the self-check adds the probes, the shading rates, the GPU headroom and the heat thresholds; if a line is red, copy the code into the chat.

### α2.1b The look's checks

**Goal:** the checks that guard the feeling, written once in C++ for the cloud and the phone; the loop that runs them; and a blind test on your phone.

**Serves:** `PRE-01`, `PRE-22`, `PRE-28`, `PRE-31`, `PLT-04`.

**Architecture:** A4.8, A5.5, A17.

**Tasks:**

1. `T2.1b.1` **The measures in C++ (`PRE-22`, `PRE-28`).**
   Its colour measures (OKLab, ground accents, texture pixel contrast) come first, before α2.1a's own work, since the art lane's checks use them.
   A Godot-free library shared by the `kindling` tool and the phone: OKLab; the target card's statistics; ground accents; people's salience from an object picture; shimmer after following the motion, against a many-sample picture; the texture pixel's size; the distinct levels of a dark gradient; and NVIDIA's FLIP from its C++ source, vendored with its licence.
2. `T2.1b.2` **The target card (`PRE-01`).**
   Its fixed goals and its bands for each moment, measured from the pictures you chose (`art/targets/`), as a tuning file; `kindling look card` shows each view's alarms green, amber or red.
3. `T2.1b.3` **The loop's drawing run (`PRE-31`).**
   One Godot run in the cloud draws the fixed views at 1080 × 2404 with the colour, object and material pictures, lossless frames of the scripted paths, and many-sample pictures of small patches, on the software Vulkan driver with one thread and time frozen.
   Changed views go beside their last approved versions on a lettered grid; golden pictures are exact for changes of code alone, and within FLIP's tolerance otherwise.
4. `T2.1b.4` **The blind test, and delivery (`PRE-01`).**
   A Compare page: ten random pairs, stills or clips, asking "which is sharper?", the answers in a short code; eight or more right means it shows; deliver as 30102.

**Tests:**
- doctest: each measure against pictures worked by hand (a known accent, a known shift, a known FLIP pair from FLIP's own tests); the card's goals pass on the pictures you chose and fail on a flat, speckled picture made by code.
- The shimmer check flags a recorded pan of a texture read nearest-pixel (17–28% of pixels) and passes the smooth-pixel read (0.4–1.1%).
- The phone and the cloud give the same measures on the same pictures (a digest in the self-check).
- Passes if all pass.

**On the phone:** open Compare and take the sample blind test, MSAA 2× against 4× on the meadow, and send the code.

### α2.2a Calibration: the fixed cost, the material and triangles

**Goal:** the first three numbers only your phone can give, each with its decision stated before the run: the fixed cost of a frame, what the full material costs a pixel at full resolution, and what a triangle costs.

**Serves:** `PRE-01`, `PLT-04`.

**Architecture:** A4.1, A4.3, A4.4, A18.1.

**Tasks:**

1. `T2.2a.1` **The calibration runner (`PLT-04`).**
   Calibration scenes as data in `data/scenes/look/`, each with its parts, switches, camera path, pass line and the decision its number makes, stated before its first run (`RES-09`).
   Each runs once at a 120-frame cap for the graphics chip's time and once at 60 for frames, power and heat, from a Calibrate page with one tap and one code.
2. `T2.2a.2` **C4 and C1 (`PRE-01`).**
   C4, the fixed cost: the world hidden, the interface on and off, MSAA off and 2×; expected 0.9–1.7 ms.
   C1, the material: a field filling the screen with the full material (the sampling function, sun, sky and bounce, a soft shadow read, the openness and contact maps, haze, the fire grid, the grade), MSAA off, 2× and 4×, the 3D at 1.0, 0.75 and 0.5; expected 1.7–4.0 ms at 1.0 with 2×.
3. `T2.2a.3` **C3, and delivery (`PLT-04`).**
   C3, triangles: a field of solid rocks at 100, 200, 400 and 800 thousand triangles a pass, MSAA 2×, the shadow pass on and off; expected 2.5–10 ns a triangle in the main pass; and the main thread's time for 100, 300 and 1,000 MultiMesh draws in two and three passes; deliver as 30201.

**Tests:**
- Each scene's file names its items, its pass line and its decision; the runner refuses a scene without them.
- The cloud runs each scene headless on the software driver to prove it draws, and its counters match the triangles and draws it states.
- The decisions, stated now: C4 over 2.0 ms looks first at the interface pass and Godot's stores; C1 at most 2.5 ms keeps full resolution everywhere, 2.5–4.0 ms builds the shading-rate patch for you to judge (α2.2c), and over 4.0 ms simplifies the material first; C3 at most 4 ns sets the triangle line at 0.6 million triangle-passes, 4–6 ns keeps 0.4 million, and over 6 ns sets 0.3 million and keeps leaves as cards or brings the pre-pass.
- Passes if every scene runs to its code on the phone.

**On the phone:** with the phone cool, unplugged and in flight mode, open Calibrate and tap Run (about 15 minutes); send the code.

### α2.2b Calibration: leaves, fires and figures

**Goal:** the three costs that decide how the camp is drawn, each with its decision stated before the run: leaves at the density you liked, fires with their shadows, and many detailed figures.

**Serves:** `PRE-27`, `PRE-30`, `PRE-46`, `PLT-04`.

**Architecture:** A4.4, A4.5, A4.6, A6.3, A18.1.

**Tasks:**

1. `T2.2b.1` **C2, leaves (`PRE-46`).**
   The liked camp's layout of plants, as stand-ins at its density, drawn four ways with all else equal: plain cut-out cards, cards cut close to their leaves, solid cores with cut-out fringes, and close-cut cards with alpha to coverage; expected 0.6–6.0 ms for the leaves.
2. `T2.2b.2` **C5, fires (`PRE-30`).**
   1, 3 and 5 fires at the closest zoom, their shadows by the walk at full and at half resolution and by a small map for each fire, all through the light grid; expected 2.5–24, 0.8–6.5 and 0.2–1.7 ms.
3. `T2.2b.3` **C6, figures (`PRE-27`).**
   30, 100 and 300 stand-in figures on a skeleton, posed 10 times a second, by Godot's own skeletons and by our bone palettes read in a MultiMesh shader, which this proves end to end; the main thread's time for each figure; and reads in the vertex stage, 100,000 to 1 million vertices reading 1, 3 and 12 texture pixels, against the same without reads.
4. `T2.2b.4` **Delivery, and the decisions written down (`PLT-04`).**
   Deliver as 30202; when the codes come back, each decision goes into A18.1 and the sections it changes, before the next step.

**Tests:**
- As α2.2a's; and in the cloud, the palette path's pose matches Godot's skeleton's for the same pose within 1 cm at every vertex.
- The decisions, stated now: C2's cheapest way that you cannot tell from plain cards in a blind test becomes the default, and if it still costs over 1.5 ms for the leaves, the leaf pre-pass is built (α2.2c); C5's map for each fire becomes the way unless you see a difference from the walk; C6's time for each figure sets how many Godot skeletons may be in view within 1.0 ms of the main thread, the rest on palettes.
- Passes if every scene runs to its code on the phone.

**On the phone:** run Calibrate again (about 15 minutes) and send the code; then the blind tests of the leaf ways and of fire shadows by map against the walk.

### α2.2c Our own build of Godot, if needed

**Goal:** only if C1 or C2 call for it, as you allowed on 6 October 2026: Godot 4.7.2's export templates built in the cloud with the two patches that close most of the gap and the one that keeps buffers off memory, measured and judged.

**Serves:** `PRE-01`, `PLT-04`.

**Architecture:** A2.2, A4.1.

**Tasks:**

1. `T2.2c.1` **The build (`PLT-04`).**
   Godot 4.7.2's Android templates built from its tag and cached in the cloud by `tools/setup.sh`, the patches kept as files beside the build script: a depth pre-pass for leaves with an equal depth test; the multisampled and depth buffers made transient; and a shading rate for each material through the driver's rate for each draw.
   The stock templates stay beside them, one setting apart.
2. `T2.2c.2` **Measured and judged, and delivery (`PRE-01`).**
   C1 and C2 again with the patches, and your blind test of shading once per 2 × 2 pixels on ground and plants; deliver as 30203.

**Tests:**
- Godot's own tests pass on the patched build in the cloud; the golden pictures are unchanged by the pre-pass and the transient buffers.
- Passes if the patched build installs, every probe and calibration scene runs, and each patch's saving is measured; shading per 2 × 2 pixels stays only if it passes your blind test.

**On the phone:** run Calibrate and the blind test; if you can tell the 2 × 2 shading apart, it stays off.

### α2.3a The texture path and the camp's materials

**Goal:** textures from request to phone with their records, and the camp's first materials on lab sheets for your OK.

**Serves:** `PRE-20`, `PRE-22`, `PRE-23`, `PRE-42`, `PLT-04`.

**Architecture:** A5.3, A5.4, A5.6, A3.6.

**Tasks:**

1. `T2.3a.1` **The `texture` kind and its checks (`PRE-20`, `PRE-42`).**
   The record as a catalogue kind (A5.4), read from `art/textures/` by the loader, as the art lane's files set it out; the art lane's checks (T2.3a.2) run in `tools/check.sh`: every texture traced to its record, original, request, truth check and approval; no stale level; re-grid loss at most 10% for band 0 and each redrawn band; band 0's seams at most 1.2, and a redrawn band's no larger than its own ordinary steps; band 0's painted light at most a slope of 0.02; no strong repeat (at most 0.2, or the source's own where a grain repeats); texture pixel contrast within a quarter of its approved source's; accents at every band a surface is seen at, from the tile that serves it, at least 90% of the near tile's band 0 (a one-tile material's bands 4 to 6 reported only, since there its texture pixel is larger than its marks); lightness within 0.02 and hue within 5° between bands and across tiles.
2. `T2.3a.2` **The tools, in the art lane (`PRE-22`).**
   In the cloud, from the start of M2: re-gridding (block size and phase window by window, the median colour, seams, the light check), the code reduction for a band, and colour matching to band 0 in four numbers that keeps the accents; the lab sheet (flat and lit, every band, at true size and enlarged, beside its source); and T2.3a.1's checks, with the colour measures of `kindling look` (T2.1b.1), so each measure is written once.
3. `T2.3a.3` **The camp's materials, in the art lane (`PRE-20`, `PRE-23`).**
   Meadow grass and earth, a trodden floor, bank gravel and the river bed, hide, birch bark and poles, brush and bark sheets, hearth stones, ash, and rock A, your pick, under each world's layers laid by code; each by its route with its designed levels, and each big surface (the ground covers and rock A, your pick) with its near, middle and far tiles, each from its own picture of the material at that distance, in two to four versions mixed by place (A5.3); with as many GPT pictures as improve the result, each vetted for truth (A5.6) and recorded.
4. `T2.3a.4` **The kit's parts, in the art lane (`PRE-46`).**
   The camp's parts in Blender, then a first person and a red deer, as the art lane's next round sets them out, each with its preview and stretch report.
5. `T2.3a.5` **On the phone, and delivery (`PLT-04`).**
   A Lab page with every sheet; loading time and texture memory measured; deliver as 30301.

**Tests:**
- Each check catches its planted fault: a missing record, a stale level, painted light, a seam, a repeat, and an averaged level whose accents fall to 77% of band 0's.
- Re-gridding the material swatches you accepted (`textures-a`, `delight-liked`) loses at most 10% each and 2–8% at the median.
- The set loads within the 3 seconds a world may take to open, and its memory is within A18.1's 300 MB.
- Passes if all pass and you approve or send back each sheet.

**On the phone:** open Lab and look at each material at true size and enlarged; say yes or no to each.

### α2.3b Ground, cliff, water and light

**Goal:** the camp's land in the look you chose: the ground from its patch, the cliff with its own layers, clear water, and smooth light with soft shadows and darkening built by us.

**Serves:** `PRE-20`, `PRE-21`, `PRE-22`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-30`.

**Architecture:** A4.3, A4.4, A4.5, A4.6, A6.1, A6.4, A8.1.

**Tasks:**

1. `T2.3b.1` **The kit in the engine (`PRE-46`).**
   Blender in the cloud's setup and the build, exporting the art lane's parts for Godot with their joints; the assembler putting parts together from recipes in the catalogues, varied by seed; the model sheet of every part and thing.
2. `T2.3b.2` **The ground and the cliff (`PRE-23`, `PRE-24`).**
   A stand-in area made by code, a camp under a cliff by a river about 256 m across, its ground on the tree of ground near the focus with its patch picture and designed levels, each big surface's near, middle and far tiles blending at their switches as any two levels do, and each cell picking one of a tile's versions by its place (A5.3); a cliff whose layers come from a stand-in geology (limestone over shale, weathered back into a rock shelter), with cracks, stains, soot above the shelter, and the art lane's boulders, cliff pieces and scree at its foot.
3. `T2.3b.3` **Light, shadows and darkening (`PRE-21`, `PRE-30`).**
   The shared light function (the sun by its height, the sky's fill by openness, bounce, backlight, haze); Godot's sun map at 2,048 for small casters and our height-field sun map for big ones; the openness and contact maps; creases baked by the kit; the tone curve, a first colour table and debanding.
4. `T2.3b.4` **Water, and delivery (`PRE-26`).**
   The bed below the water's level, the surface's sky colour by angle, the mirror at half resolution with a smaller set, flow lines and foam stepping in whole texture pixels, glints of one texture pixel, the shore line from height; deliver as 30302.

**Tests:**
- Golden pictures of the area at the closest zoom at noon and dusk; shimmer on the three scripted paths at most 2 in 100 pixels; the texture pixel 1.5–3 screen pixels; no pass reads the screen or depth (a scan of the shaders).
- The ground's and the cliff's texture pixels stretch at most 1.5:1 on every triangle (A6.4).
- On a pinch from the closest zoom to the camp zoom, no tile repeats strongly in any frame (the repeat measure on the ground's pixels at most 0.2), and the switches between tiles pass the shimmer line.
- Shade is never black; the hollow under the overhang is darker than open ground; a long shadow's edge is softer 10 m from its caster than 1 m from it.
- Passes if all pass, and the frame on the phone stays within its line or names the part over it.

**On the phone:** open Look at the camp: the ground, the cliff and the river at the closest zoom in late afternoon light; drag along the river and turn round the cliff.

### α2.3c Plants, shelters and fire

**Goal:** the camp's still life: the meadow's cover set out by the graphics chip, plants as dense and airy as you liked them, two shelter types, and the hearth with its fire.

**Serves:** `PRE-20`, `PRE-24`, `PRE-30`, `PRE-42`, `PRE-43`, `PRE-46`.

**Architecture:** A4.5, A4.6, A6.1, A6.2.

**Tasks:**

1. `T2.3c.1` **Cover and plants (`PRE-46`, `PRE-43`).**
   Tufts, flower clumps and pebbles set out by the chip from the patch by the density rule; plants drawn the way C2 chose, cut close to their leaves; reeds, bushes and a birch put together from Blender's parts by growth rules and varied by seed; each plant's design for band 0 (A6.1).
2. `T2.3c.2` **Shelters and things (`PRE-42`, `PRE-24`).**
   Put together from the kit's parts; two shelter types in two materials each, a skin tent on a stone ring and hides closing off the rock shelter, each recording the excavation it rests on; a hearth ring, a drying rack, baskets and tools as kit layouts; light inside only from openings and fire.
3. `T2.3c.3` **Fire, and delivery (`PRE-30`).**
   Flames as pixel art made by code at about 10 frames a second, embers as copies, the light grid, fire shadows the way C5 chose, glow in the light function, and smoke as lit cards drawn last; deliver as 30303.

**Tests:**
- Copies only from about 12 screen pixels, counted at each zoom stop against A4.6's estimates; the same patch gives the same tufts on two visits (`WLD-13`).
- A fire inside the tent lights its doorway, and no light passes through its wall (a view of each fire's reach).
- The two shelters in two materials are told apart at the close camp on the model sheet.
- Every part's texture pixels stretch at most 1.5:1 on every triangle, and a pole's wrap never shows (A6.4); every recipe's parts meet at their joints.
- Passes if all pass, with the frame within its line or the part over it named.

**On the phone:** the camp has its meadow, plants, two shelters and a fire; turn round the tent and look into the shelter.

### α2.3d People, a deer and first light

**Goal:** detailed people and a deer from Blender's parts on their skeletons, moving 10 times a second; then first light: the camp at the closest zoom beside the picture you liked, for your eye.

**Serves:** `PRE-01`, `PRE-27`, `PRE-28`, `PRE-31`, `PRE-44`, `PRE-46`.

**Architecture:** A6.1, A6.3, A5.5, A8.4.

**Tasks:**

1. `T2.3d.1` **The figure (`PRE-27`, `PRE-46`).**
   One skeleton; the body from Blender's parts, with build, age and sex as shape keys and part choices; three ages in three garments; hair and beads; faces at band 0 in their 12 states as small designs; held tools on bones; proportions from the family sheet you accepted.
2. `T2.3d.2` **The poser (`PRE-44`).**
   Key poses as joint angles for standing, walking, sitting, crouching at work, knapping and carrying; the bending rules and seed offsets; palettes written at each pose step and bent on the chip, or Godot's skeletons for as many as C6 allows; poses held or gliding by a switch.
3. `T2.3d.3` **A deer (`PRE-44`, `PRE-46`).**
   The hoofed body pattern on its skeleton with a red deer's proportions, coat and antlers by age; walk, trot and gallop from numbers for each leg; still poses from the approved sheet.
4. `T2.3d.4` **First light (`PRE-01`, `PRE-31`).**
   The camp at the closest zoom in late afternoon, with seven people at their tasks and a deer at the river: the loop's first full run (the card, the checks, the judge's faults), the engine's frame beside the liked picture, the camera's tilt at 35°, 40° and 50° and its lens at 5° and 10° for your eye, and poses held or gliding; deliver as 30304.

**Tests:**
- Filmstrips of every movement, enlarged, show no gap or tear at a joint; the same seed gives the same person.
- Every figure's and deer's texture pixels stretch at most 1.5:1 on every triangle, standing and in each key pose (A6.4).
- The deer's feet stay planted while down (a slide under 1 cm).
- The card on first light's frame, and the shimmer, size and banding checks; people's salience in the camp.
- Passes if all pass; the look itself is your verdict.

**On the phone:** first light: open Look at the camp and compare it with the picture you liked (a button shows it); try the tilts, the lenses, poses held or gliding, and poses at 10, 15 and 30 a second side by side, and say what you prefer.

### α2.4a Stress scenes

**Goal:** the camp at its busiest on your phone, with every pass line stated before the run: the liked camp by day, the camp of thirty at ten tasks, and night by the fire; each lever that may show switchable, for your blind test.

**Serves:** `PRE-28`, `PRE-44`, `PLT-02`, `PLT-04`, `VIS-14`.

**Architecture:** A4.1, A6.3, A18.1.

**Tasks:**

1. `T2.4a.1` **The camp of thirty (`PRE-44`, `PRE-28`).**
   Thirty people at ten tasks (knapping, scraping a hide, carrying wood, tending the fire, cooking, sewing, drying meat, sitting and talking, children playing, walking), each task with its key poses, offset by seed so no two move in step.
2. `T2.4a.2` **S1, S3 and S2 (`PLT-04`).**
   S1, the liked camp by day at the closest zoom, seven people and one fire; S3, the camp of thirty at the close camp by day; S2, S1 at night with three fires; each on its scripted path, at a 120 cap and at 60.
3. `T2.4a.3` **The longest frames, the levers and delivery (`PLT-02`, `VIS-14`).**
   The longest frame while the phone turns, while a lever switches, and while an area's copies arrive at budgets of 128, 256 and 512 KB a frame; each lever that may show with its switch; deliver as 30401.

**Tests:**
- The pass lines, stated now: at least 97% of frames on time and none over 66.7 ms; the graphics chip at most 8.0 ms on average and 9.5 at the 95th percentile in each scene; the main thread at most 8 ms on average and 12 at the 99th percentile; triangles within C3's line and at most 300 draws; the app within 1 GiB; after an install, textures made within 10 s, a warm open within 3 s, and no pipeline compiled while drawing.
- A scene over a line names its parts over their allowances, which is not itself a failure; then A4.1's levers in order.
- The cloud runs each scene headless first, and its counts match.

**On the phone:** run the three scenes from Bench (about 15 minutes) and send the code; then the blind test of each lever that may show.

### α2.4b The heat run and the line

**Goal:** the true line, set by heat: twenty minutes of the costliest scene that passed, and the decisions that follow.

**Serves:** `PRE-01`, `PLT-04`.

**Architecture:** A3.9, A5.5, A18.1.

**Tasks:**

1. `T2.4b.1` **H1 (`PLT-04`).**
   Twenty minutes of the costliest S scene that passed, at 60 frames, unplugged, in flight mode and after the phone has cooled, with heat, power and frames read every 2 s.
2. `T2.4b.2` **The line and the step, and delivery (`PRE-01`, `PLT-04`).**
   If the phone stays cool, the line may rise; if the picture alone heats it, the line comes down for good or the one planned, logged step under heat is chosen among the savings that pass your blind test (A5.5); the decision goes into A18.1; deliver as 30402.

**Tests:**
- The pass lines, stated now: the 10-second forecast never reaches the phone's light threshold less 0.05; the thermal status stays at none; the battery ends at or below 40 °C; the whole phone averages at most 4.0 W; every minute has at least 97% of frames on time.
- The heat step, if chosen, is logged each time it acts and has passed your blind test.

**On the phone:** with the phone cool, unplugged and in flight mode, start Heat on Bench and leave it 20 minutes; send the code.

### α2.5a The hours

**Goal:** the camp at dawn, true midday, dusk and night, each beside a target relit from the picture you approve for the camp.

**Serves:** `PRE-20`, `PRE-30`, `PRE-31`.

**Architecture:** A4.3, A5.5, A5.6.

**Tasks:**

1. `T2.5a.1` **The camp's targets (`PRE-31`).**
   The camp's anchor picture, approved by you: first light's frame repainted, or a target you chose, its truth checked; relit by GPT to dawn, true midday, dusk, night and winter, each vetted; their numbers into the card's bands.
2. `T2.5a.2` **The hours (`PRE-30`).**
   The sun's colour by its height, the sky's fill, the moon by its phase, firelight pools at night halving within about 2 m, haze at dawn; a colour table for each moment, blended as the sun moves; the light held steady at speed.
3. `T2.5a.3` **The checks, and delivery (`PRE-20`).**
   An Hours switch on the Look page; the card and the banding check on each moment; deliver as 30501.

**Tests:**
- The card's bands for each moment (A4.3's table) on the engine's frames; across a moonlit slope with debanding, no step wider than the dither hides.
- A day passing in under 10 seconds keeps the light steady (its change from frame to frame below a stated amount).
- The grade's cost: colour correction on and off, one colour table against two blended.
- Passes if all pass; each moment's look is your verdict against its target.

**On the phone:** step the camp through dawn, noon, dusk and night beside each target, at your usual brightness and a fixed colour mode, and say which feel right, whether the darks band, and whether people read by firelight.

### α2.5b Seasons and weather

**Goal:** the camp in winter, spring and autumn, in rain's four layers, a storm and lake mist, with a stress scene for each.

**Serves:** `PRE-20`, `PRE-26`, `PRE-30`, `PLT-04`.

**Architecture:** A4.3, A4.5, A18.1.

**Tasks:**

1. `T2.5b.1` **Seasons (`PRE-20`, `PRE-30`).**
   Snow on faces turned up by slope and shelter, trodden on paths, cream in sun and blue in shade, melted round the hearth, over clear water; autumn's colour as the field; spring's; each plant's season state.
2. `T2.5b.2` **Weather (`PRE-26`, `PRE-30`).**
   Rain's four layers, a storm with lightning's flash and bolt, lake mist in banks, and falling snow, from a weather setting that stands in for M3's weather.
3. `T2.5b.3` **Stress scenes, and delivery (`PLT-04`).**
   S4, a village of huts at night with sixteen fires and about ninety people in view; S5, the storm with a burning oak; S6, winter; S7, the lake in mist with reeds and canoes; S8, the autumn wood with people and falling leaves; with α2.4a's pass lines; deliver as 30502.

**Tests:**
- α2.4a's pass lines for S4 to S8, and the card's bands for winter and rain.
- Passes if all pass; each season's and weather's look is your verdict.

**On the phone:** step through the seasons and weather on Look; run the new scenes from Bench and send the code.

### α2.6a The zoom bands

**Goal:** the zoom from the person out to the camp, each band's textures and small things drawn for it, so the ground never speckles and small plants keep their charm.

**Serves:** `PRE-03`, `PRE-22`, `PRE-28`, `PRE-46`, `PLT-04`.

**Architecture:** A4.2, A5.3, A5.4, A6.1, A6.3, A8.3.

**Tasks:**

1. `T2.6a.1` **Every material's levels (`PRE-22`).**
   Every material's designed levels for the bands it is seen at, the big surfaces redrawn by GPT from the level above and matched by code, the rest by code; each checked for accents and drift; the blend between bands.
2. `T2.6a.2` **Small things by band (`PRE-46`).**
   Each plant form's designs for bands 1 and 2, with flowers and berries kept at least one texture pixel; plants below 12 screen pixels into the ground's band texture; the meadow strip three ways (shrunk, filtered, designed), for your eye.
3. `T2.6a.3` **Figures by size (`PRE-28`).**
   The simple, small, tiny and marker forms (A6.3), the small form drawn to read with its outline shell growing from nothing at about 50 pixels; the baked pose library for crowds; trees' far crowns.
4. `T2.6a.4` **The camp zoom, and delivery (`PRE-03`, `PLT-04`).**
   The pinch from 8 m to about 300 m across, the bands' changes measured at fixed points; S9, the camp in thick forest at the camp zoom with the camera turning, at most 6 ms of the graphics chip; deliver as 30601.

**Tests:**
- Ground accents at every band at least about 20 and at least 90% of band 0's; the texture pixel 1.5–3 screen pixels at every zoom stop; shimmer through a slow pinch at most 2 in 100 pixels; a camp of 30 readable at every zoom stop, with no jump as its forms change (`PRE-28`).
- Passes if all pass; the meadow strip and the close camp's ground are your verdict.

**On the phone:** pinch from the person out to the camp and back over the meadow, the cliff and the camp, and say whether any band change pops or swims; compare the meadow strip's three ways, and the far ground with and without grain at true size; the close camp's ground at three accent levels, whose yes or no sets the accent line.

### α2.6b People in busy scenes, landscape and M2's end

**Goal:** every person found at a glance in the busy scenes by real light and movement, the engine in landscape, the contact sheet, and M2's report for your review.

**Serves:** `PRE-28`, `PRE-31`, `PLT-02`, `PLT-04`, `RES-06`, `RES-22`, `VIS-14`.

**Architecture:** A4.8, A5.5, A15, A18.

**Tasks:**

1. `T2.6b.1` **The busy scenes (`PRE-28`).**
   An autumn wood, a crowded camp and a night camp, with people moving at their tasks in sun and shade; the salience check on each; a timed find on the phone, tapping every person, aiming at about a second each; your yes or no on ten close-camp crops sets the readability line; help drawn only for your eye comes only if the find falls short, and then with your OK (A5.5).
2. `T2.6b.2` **Landscape (`PLT-02`).**
   The engine in landscape with the interface beside it; turning the phone keeps the world, the camera and the texture pixel's size.
3. `T2.6b.3` **The contact sheet (`PRE-31`).**
   Made on the phone from fixed saved scenes: each near zoom stop at noon and dusk in portrait, one landscape view, the model sheet, three short clips of people at work, and the busy scenes and camera clips the look's checks are judged on.
4. `T2.6b.4` **M2's end (`RES-06`, `RES-22`, `VIS-14`).**
   Deliver as 30602; the independent review of the whole milestone, judging as a pixel artist and a game art director against the targets you chose; M2's report with the phone's numbers, the contact sheet and what went right and wrong; M3 planned in detail and its architecture written in full, for your OK.

**Tests:**
- In the busy scenes the median person at about the 80th percentile of salience or above and none below about the 70th (starting lines, refitted on your verdicts), and your timed find of every person.
- Turning the phone keeps the world, the camera and the texture pixel's size, with no reload (`PLT-02`).
- Every earlier check and scene still passes; the contact sheet meets every Done when of 11.1 and 11.2 that M2 builds, judged by the review and then by you (`PRE-31`).

**On the phone:** find every person in each busy scene; turn the phone; look through the contact sheet; then M2's report, for your review.

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
- Little figures of blocks, each activity with its own movement.

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
