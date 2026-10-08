# Handover, 8 October 2026

## Update, 8 October 2026, about 18:40 UTC: all work stopped

The owner stopped all work at 18:29 UTC. The lead stopped its three helpers at 18:35 and wrote a handover, now in `drafts/handovers/lead.md`; read it before starting again. Nothing is running.

- **Done, reviewed and merged since 15:00:**
  - M3 is adopted into `ARCHITECTURE.md` and `IMPLEMENTATION.md` (`f80caa6`), with 11 `PROJECT.md` changes the owner approved (`aa1a3ae`). `drafts/m3` is gone.
  - `PRC-10` is now a short routine check of about 2 minutes warm (it took 23); the slow suites run only with `tools/check.sh --audit` (`eca18a3`).
  - Delivery 30801 is merged (`66b0664`): 48.87 MiB, SHA-256 `13fd6424…422d`.
  - The old 3D runtime and its pages are removed (`0b2ac49`).
  - Game-ready art: the meadow, boulder, birch and tent fixtures (`0b2ac49`), and ground 1.3 bare earth, 1.4 bank gravel and 1.5 river bed, six state packs (`c31d58f`).
  - The hazel 8.1 sheet is signed off; 129 of 373 catalogue pieces are now signed.
- **Merged as unfinished work, not reviewed** (merge `750abc1`):
  - α2.9a from `lead-code-b`: the camera with 19 zoom stops and a smooth settle, bounded streaming, ground demand, the new sky and contact shading, and the reworked Examples and UI. 57 of 57 native tests passed, but the whole step has never had its code review, and no screen has passed the art team's review.
  - Ground 1.6 hearth and 1.7 mud from `lead-design`: sources, partial exports and exporter changes. The last 1,260 export files were written after the lead's list and may be incomplete.
- **Known open problems:**
  - A texture-lifetime bug in `drawing.gd` (old textures stay attached when the ground hides).
  - The streaming timing run must be redone; its first timings are not valid.
  - The Explore drawer can overflow in landscape.
- **The owner's feedback still to act on:**
  - The zoom snapping is too visible.
  - The UI is too small and busy; the example tabs must be clear and fully composed.
  - The art designer and critic must check every screen, test and UI pages included.
  - No APK or try-list goes to the owner until the art in the game looks decent.
  - The camp picture: the meadow repeats in diagonal stripes, the birch, boulder and tent shadows look wrong, and "Camp" appears twice.
- **Open question for the owner:** may unsigned catalogue pieces go straight to game-ready art without a design sheet first? Asked at 15:57, no answer yet.
- **Next, once the owner says to resume:** follow the "Resumption order" in `drafts/handovers/lead.md`: fix the texture bug, rerun the timing, take fresh pictures of every screen for the art team, finish and review ground 1.6 and 1.7, then review the whole α2.9a step and run the short check before any delivery.

## Update, 8 October 2026, about 15:00 UTC

Between 13:50 and 14:50 UTC three leads (design, coding, research) carried the work on. Then the owner replaced them with **one lead** who runs every team, and asked for this update.

- **Their work is merged here as work in progress** (merge `441a2cd`), and each lead's own handover is in `drafts/handovers/`:
  - `code.md`: α2.8a's terrain work; read it first.
  - `design.md` and `design-artist.md`: the six art fixtures.
- **α2.8a:**
  - The code review of the whole inherited step found 7 issues, all fixed with new tests.
  - A second review of the fixes was clean, and 19 game and 72 view tests passed.
  - The latest packaging changes are not reviewed yet.
- **Delivery 30801 is not done.** The signed APK came out at 53.3 MB, over the 50 MB limit for a committed file. A lossless fix is saved but not yet built or reviewed:
  - compress the native library;
  - drop a duplicate ground sheet.

  It should bring the APK to about 49 MB. If it does not fit, the owner chooses between invisible compression and release files outside the repository (A2.3). `dist/kindling.apk` is still 30301.
- **Art:**
  - The meadow and boulder still pass, unchanged.
  - The birch's fourth round was cut off before review. It still has a squared upper-stem join and grey bark in the upper-left leaves.
  - The tent has new source drawings but still exports the old tent.
  - The person and red deer sheets still need their round-1 fixes.
  - The code review of the art tools found 2 bugs, and the fixes are unreviewed.
- **Art must go much faster:** 373 catalogue entries, of which 245 have no sheet yet, and in a whole day only two fixtures passed. The lead must find a process many times faster without lowering the bar.
- **M3, "The world", is researched and planned in full, as drafts**, in `drafts/m3/`: the research, the architecture, the plan (12 alphas, 24 deliveries, 72 tasks), the questions and the critic's review.
  - The owner answered questions 1 to 10 on 8 October 2026 (`drafts/m3/OWNER-ANSWERS.md`). All are as recommended except the eclipses: 3 to 8 visible at every place in 70 game years.
  - Worlds have one woolly mammoth species, so the bone hut is unblocked.
  - The drafts move into `ARCHITECTURE.md` and `IMPLEMENTATION.md` only when the owner approves the plan; then `drafts/` is deleted.

Read the rest of this note for the state before 13:50.

## Where things stand

- **M1 (the foundations)** is built and accepted. Its same-bits proofs must never change.
- **M2 is now fully 2D.** The camera is fixed at 37° and the rich light stays (`PRE-01`, `PRE-02`). `PROJECT.md`, `ARCHITECTURE.md` and `IMPLEMENTATION.md` were rewritten for 2D (commit `3adead5`).
- **α2.7a's engine part** is merged, reviewed and checked (`8607855`): the owned snapshot, the fixed projection with picking, the Fixtures page and the 2D captures. Its art (`T2.7a.3`) and its delivery 30701 are not done.
- **The art catalogue:** 127 of 373 sheets are signed off. The sheets are the art standard.

## Merged today without review: treat as work in progress

The owner asked for these to be merged as they were. Neither has passed `tools/check.sh` or the code review, so check both before building on them.

### α2.8a, the flat shadow style and terrain prototype (merge `7228f47`)

- **Built:**
  - terrain heights;
  - roof and floor shadow receivers;
  - shadows from a low sun;
  - walls that block fire light;
  - draw order;
  - a mask cache with a hard cap;
  - the terrain test page (`game/pages/terrain.gd`, `view/src/terrain*.cpp`);
  - capture tools (`tools/terrain-*`).
- **Last tests:** at about 12:00 UTC, all 72 view tests and 13 game tests passed in an optimised build. Each of the 8 new tests had failed once on purpose. M1's digests matched on one and four threads.
- **After that test run,** the builder fixed the fire's height on slopes (it now uses the sampled ground height). It was adding before/after captures when it was stopped.
- **Not done:**
  - the frozen captures;
  - `IMPLEMENTATION.md`'s account of the step;
  - the built-in code review;
  - the full check;
  - delivery 30801.
- **Two small fixes the coordinator noted:** the design-sheet thumbnail covers the test scene, and one figure seemed drawn on the birch's trunk (check its draw order).

### The six art fixtures, `T2.7a.3` (merge `2678980`)

- **Files:** `art/textures/fixtures27/`, `art/sources/fixtures27/` (the drawings unchanged, plus cleaned versions and prompts), `art/requests/fixtures27/` and `art/previews/fixtures27/`. Tool: `tools/art/fixtures.py` (`build`, `check`, `design`).
- **Meadow:** redrawn after the owner's example of a calm, muted meadow (a game screenshot; a guide to feel only, never copied). It now has three tiles at each size, mixed so no grid shows. It **passed** the art critic in 3 rounds. The muted greens still need the owner's OK.
- **Boulder:** **passed** in 2 rounds. It keeps the sheet's rounded dome and crack.
- **Birch:** failed round 1. Fixes were under way when stopped, so the files may be half repaired:
  - the trunk is 0.36 m wide, but the sheet says 0.25 m (root collar 0.38 m);
  - some bark pixels are marked as leaves;
  - summer and winter should share one trunk;
  - its normals are too flat.
- **Hide tent:** failed round 1:
  - its inner heights should be cover 2.6 m, pole crossing 2.7 m and tips 3.1 m;
  - the wood, sinew, hide and stone masks need cleaning;
  - each material needs its own normals.
- **Person and red deer design sheets** (`art/catalogue/sheets/10.1-body.webp`, `13.1-red_deer.webp`): proposals only, **not signed off**, and the catalogue does not mark them signed off.
  - The critic's round 1 asked for no heavy outline, one steady stance across the four facings, calmer garment pixels, deer sizes that agree across views, and one antler pair throughout.
  - The image tool refused an unclothed figure, so the person is in hide clothing. The first people and the full age kit are still missing.
- **Not covered yet:**
  - The tent interior is not drawn.
  - Noon, dusk, slope, water and shelter lighting have to be judged in the running engine.

## What to do next, in order

1. Build both merged parts and run what they touch. Fix whatever broke, then run the code review on α2.8a and fix its findings.
2. Finish α2.8a: the captures, its account in `IMPLEMENTATION.md`, `tools/check.sh --deliver`, and **delivery 30801**. The owner is waiting for an APK. It carries α2.7a's work too.
3. Finish the birch and tent fixes, and run one more critic round on the two proposal sheets. Then show the owner the six fixtures and the two sheets for sign-off.
4. Carry on with α2.9a (zoom and streaming) and the later M2 steps in `IMPLEMENTATION.md`.

## How the work was being done

`CLAUDE.md` describes the method: the main session coordinates and GPT does the work. These practical lessons are not written down anywhere else.

- **Running GPT:** use the Codex command line: `codex exec`, then `codex exec resume <id>` for later turns. Add `--search` before `exec` for web search. Use `-s workspace-write` and `-c model_reasoning_effort="high"`.
  - Agents that run Godot captures need `-c sandbox_workspace_write.network_access=true`, or the virtual display fails.
- **Models:** `gpt-6.1-sol` is the workhorse. `gpt-6-astra` is the top model.
  - One Astra can lead Sol sub-agents: `-m gpt-6-astra -c agents.default_subagent_model="gpt-6.1-sol"`. This was tested and works.
- **Git is read-only inside Codex's sandbox.** Agents write a COMMITS.md list, and the coordinator makes the commits.
- **The built-in code review** (`codex review --uncommitted`) works from outside an agent but **not** from inside one, because the sandbox inside a sandbox fails. Run it from outside, on request.
- **Background jobs in a cloud session stop after 2 hours.** Start long agent runs detached (`setsid nohup ... &`) and watch them separately.
- **Messages to a running agent:** `codex queue --thread <id> --message ...` queues one.
- **The owner's preferences:**
  - every step goes through the code review before it counts as done;
  - the art agent always works with a critic;
  - agents post short status updates;
  - plain words, for reading on a phone.

## Still open from before

- **Hazel (8.1):** the owner approved it, but it is not recorded. Its sheet is too tall for WebP (over 16,383 px), so its rod part must be rescaled first.
- **Bone hut (16):** waits on the mammoth question. In group 17, 6 tools are not started, and the first 12 need the camera-size fix.
- **Calibration decisions** from the owner's codes are not yet recorded.

## Cleaned up

- All agents are stopped.
- Their work branches are merged here and their working folders are removed.
- Notes and pictures kept outside the repository (research reports, scene mock-ups, critic reviews) were session-only. What mattered from them is in the three documents and in this note.
