# Handover, 8 October 2026

The owner stopped all agents on 8 October 2026 at about 13:00 UTC and asked for everything to be merged here with this note. Read `CLAUDE.md` and `PROJECT.md` first, as always. This note says where the work stopped and what to do next.

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
