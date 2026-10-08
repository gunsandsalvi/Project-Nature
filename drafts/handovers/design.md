# Design handover, 8 October 2026

The owner asked at 14:38 UTC to stop the three leads and transfer their work to one lead. New work stopped. This is a wind-down handover, not an art acceptance or completed assignment.

Worktree: `/home/user/Project-Nature/.claude/worktrees/lead-design`, branch `lead-design`.
This folder is `/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/lead/design`.
Read `turn-01.message`, `../PROTOCOL.txt`, `INBOX.md`, and the workspace's `CLAUDE.md` before continuing.
Git is read-only for the team; the coordinator commits, merges and pushes.
Never mark owner approval yourself or edit `PROJECT.md` or `CLAUDE.md`.

## Each item's state

| Item | Exact review state | What remains |
| --- | --- | --- |
| Meadow | Historical critic PASS, round 3; prior director agreed. Its exports stayed unchanged in this turn. | Owner must approve muted greens. Engine and phone review remain separate. |
| Boulder | Historical critic PASS, round 2; prior director agreed. Its exports stayed unchanged. | Engine light/contact/shadow/phone review. |
| Birch, summer and winter | Round 2 FIX, then round 3 FIX. Round 4 has not been reviewed. Final repair was in progress when wind-down arrived; see the final checkpoint below. | Last critic found a squared upper-stem join at near row 205 and grey bark intruding into the upper-left summer leaf edge. Both normal-field defects passed round 3. |
| Covered hide tent | Historical round 1 FIX. New image-tool sources exist, but the runtime recipe/exports still use the old tent. | Fit internal heights and doorway; trace wood/hide/sinew/stone masks; give each surface suitable mild normals; review all sizes. Interior/back frame is still absent. |
| Person proposal | Historical round 1 FIX; sheet unchanged this turn and not signed off. | Remove heavy near outline, keep one stance through four facings, quiet garment texture; walk pictures remain studies. First people and full age kit are missing. The earlier unclothed image request was refused; do not repeat it. |
| Red deer proposal | Historical round 1 FIX; sheet unchanged this turn and not signed off. | Reconcile nose-to-rump/shoulder/antler dimensions, keep one antler pair across views, remove near outline/noise, repair far mark and walk-study consistency. |
| Hazel, 8.1 | Owner approval was reported in the incoming brief but is not recorded; no rebuild or approval edit in this turn. | Correct vertical rod measurement in the sheet spec, rebuild, critic/lead inspect, then READY FOR OWNER. |
| Remaining catalogue | No new group started; all 127 recorded signed-off sheets remain untouched. | Continue unsigned sheets group by group after the earlier tasks. Runtime bulk production still waits on the six engine/phone approvals. |

The owner answered at 14:33: worlds **do have one woolly mammoth species in cold open grassland**. The bone hut is unblocked for later catalogue work. This supersedes the initial brief's open question. No decided document was edited by this team.

## Birch review and code findings

Read `reviews/review-09-birch-r2.md`, `reviews/review-10-birch-r3.md`, `DECISIONS.md`, and `reviews/RESUME.md`.
The lead inspected the pictures and agreed with both FIX verdicts.
The critic stopped after saving its handover; no final-round review was consumed.

Round 3 closed two issues: continuous canopy normals replaced arbitrary roughly 38-degree seams, and local trunk normals stopped changing when a distant branch entered the same image row.
Keep their restrained response.
The remaining visible faults were:

- Near row 204 ended at x508 in summer/x509 in winter; row 205 abruptly extended to x519. Follow the actual upper centreline and taper into the lower shaft; moving the rectangular cutoff higher just moved the fault.
- Grey bark pixels contaminated the summer leaf edge around x463–477, y230–285, including (464,263) and (469,271). Winter-wood proximity and matching dark green are not proof that a summer pixel is exposed bark. Preserve leaf occlusion and trace visible wood before geometric cleanup.

The correct dimensions remain 20 m height, 7 m spread, shaft about 0.25 m (16/4/1 pixels), collar about 0.38 m (24/6/2 pixels), at 64/16/4 internal pixels per metre. Base pivots remain established in `art/sources/fixtures27/exports.json`. The far winter arches/hooks were accepted at actual 2x as clearly bare; no mandatory redraw there.

`code-reviews/review-01.md` found two P2 bugs in the round-3 tool:

- Singleton cleanup erased the final 1x1 birch mip. Keep coverage at that level.
- Shared winter-source preparation during a summer-only build saved over winter's repaired `-cleaned.png` files. Preparing a cache input must not overwrite another fixture's output.

Both were assigned to the artist with the final repair. A fresh built-in code review is still required after those fixes. Do not treat review 01 as clean. The coordinator now watches `code-reviews/request-NN.txt`; a short plain request starts a review, with the result in `review-NN.md`. No request 02 was started during wind-down.

## Files and saved work

- Current tool and tests: `tools/art/fixtures.py`, `tools/tests/test_art_fixtures.py`.
- Recipes/export contract: `art/sources/fixtures27/recipe.json`, `exports.json`, `provenance.json`.
- Birch derived work: `art/textures/fixtures27/birch_*`, six colour/map previews and six light studies in `art/previews/fixtures27/`, cleaned images under `art/sources/fixtures27/{neutral,revision}/`.
- Artist reports, numbers and exact-2x strips: `artist/birch-r2-handoff.md`, `artist/birch-r3-handoff.md`, their numbers/seasonal-strip files, and the artist's final handover when present.
- Tent kept sources: `art/sources/fixtures27/revision/tent-near-r2-v2-original.png`, `tent-middle-r2-original.png`, `tent-far-r2-original.png`. Other near originals are retained rejected attempts, not selected art. Exact prompts are under `art/requests/fixtures27/revision/`; provenance lists the five generated sources. Tent `-cleaned.png` drafts are not approved exports.
- Tent detailed continuation: `artist/tent-next.md`; `artist/tent_semantics.py` is a scratch prototype, not the production pipeline. It explores half-coverage alpha to retain the far doorway and semantic masks. Verify it before use.
- Proposal sheets: `art/catalogue/sheets/10.1-body.webp` and `13.1-red_deer.webp`. Specs and source boards are under `art/sources/fixtures27/design/`; historical faults are in `history/reviews/review-05.md`.
- Hazel original sources/spec: `scratchpad/catalogue/g08-hazel/` under the same session root. `part-rod.png` is vertical, but its spec says `across: 1.5`; the rod should be measured `tall: 1.5`. The mistaken width produced the enormous height. Preserve the drawing and rebuild through `tools/art/sheet.py`; no general sheet-tool change is needed for that error.

The coordinator removed old scratch source folders for signed-off catalogue items to free disk space. Fixture, hazel and unsigned-item sources were retained. Use signed-off repository sheets as the design standard.

`make_owner_peek.py` in this folder composes existing near colour pages at one true scale, exactly 2x nearest neighbour, with ground projected once and a projected upright 1 m stick. `owner-layout-draft.png` only tested the layout while art was changing; it is **not** a reviewed owner picture. Final `owner-peek.png` has not been handed off. Run the script after the fixtures/proposals have completed their review sequence.

The current game fixture manifest still references the **16.5 dome** crop. The new tent is the **16.4 cone**, with a 4.2 m stone ring and 3.1 m pole tips. A TO CODING LEAD note in STATUS asks the coordinator to carry the proper dimensions/pivots and world-east/south/up normal basis into integration. Covered roof/front slices do not supply an occupied interior.

## Checks and commands

Before the changes, 106 art tests passed, with 14 skips; the fixture check passed 1,008 aligned pages. File check passed: 364 items, 2,920 citations (`filecheck.log`).
Stable round 3 passed 20 focused fixture tests, all 1,008 page checks, and Ruff formatting/lint. These passes did not catch the two code-review bugs or the remaining visual faults. See final checkpoint below for the last results after the artist's current edit.
Protected meadow/boulder files matched both the parent's 342-file hash list and the artist's broader 393-file list on their last checks. No full check or phone test was run.

Run these from the worktree:

```sh
# Rebuild both tree seasons, then check all fixture pages.
python3 tools/art/fixtures.py build --only birch-summer birch-winter
python3 tools/art/fixtures.py check
# Test the changed fixture tool.
python3 -m unittest discover -s tools/tests -p 'test_art_fixtures.py'
# Broader art tests, when changes justify them.
python3 -m unittest discover -s tools/tests -p 'test_art_*.py'
# Check Python style and document references.
ruff format --check tools/art/fixtures.py tools/tests/test_art_fixtures.py
ruff check tools/art/fixtures.py tools/tests/test_art_fixtures.py
python3 tools/filecheck.py file
```

The assignment explicitly requested art checks rather than the full check. Git operations remain the coordinator's job.

## Next steps in order

1. Read the artist's final checkpoint and inspect the current birch pixels. Resolve any unfinished final repair, run the targeted checks, request a fresh built-in code review, and give the stable candidate to the critic for its one remaining round. If it still fails, report the failure plainly; do not reset the count.
2. Finish the tent using the saved drawings and notes, then run its remaining critic rounds. Preserve the signed-off cone design and keep interior limitations explicit.
3. Repair and review the two proposal sheets, retaining their honest scope; do not begin person/deer runtime production before owner approval.
4. Render `owner-peek.png` and report READY FOR OWNER with that picture and the two proposal paths. The owner signs off through the coordinator.
5. Correct/rebuild hazel and present it for owner review.
6. Continue unsigned catalogue sheets in owner batches. Ask before a group with an unresolved question; the mammoth question is now answered.

The wind-down `COMMITS.json` accounts for all uncommitted work, including unfinished tent sources. Its messages must remain explicit WIP. It replaces the earlier `COMMITS.birch-draft.json`, which was only an unaccepted merge candidate.

## Final checkpoint

Pending the artist's stop confirmation. The lead will append the exact final file/check state and manifest counts before announcing READY TO MERGE (WIND-DOWN).

Final checkpoint — 2026-10-08 14:49 UTC: both agents have stopped; no running jobs or image calls remain. The artist completed and rebuilt both birch seasons as a coherent final-round candidate. The curved upper shaft/profile and local semantic-mask repair are applied, and both code-review P2 findings are addressed with focused tests. These latest changes have NOT had critic round 4 or a fresh built-in code review. The last completed art verdict remains round-3 FIX; one review round remains. Use current repository textures/previews, not the older r2/r3 scratch strips.

Final checks: art suite successful (113 tests, 14 skipped); 24 focused fixture tests passed; 1,008 fixture pages passed with zero failures; Ruff formatting/lint clean. All six final birch colour mips retain alpha 255. All 393 artist-protected meadow/boulder files remained unchanged; the lead independently confirmed its 342 protected export hashes. `git diff --check` passed. The earlier file check passed and the decided documents stayed unchanged. The full check was not run.

`artist/HANDOVER.md` and `artist/final-art-tests.txt` hold the final details. Tent recipe/production exports remain round 1; its new sources and scratch preparation are WIP. Person/deer, hazel and later catalogue work remain as listed above. No owner-peek or approval was delivered.

`COMMITS.json` now lists every one of the 437 git-visible uncommitted files: 422 in the WIP birch/tool commit and 15 in the WIP tent-source commit. No listed file exceeds 50 MB. `WIND-DOWN-SHA256.json` records the frozen bytes (null would mean a deletion). The old draft manifest is superseded. Files are frozen for the coordinator's owner-authorized wind-down merge; this merge must not be described as approval.
