# Kindling independent re-review — 10 October 2026

Verdict: M3 is a useful discovery slice, closed partially by the owner. That closure stands. This completes the independent static re-review; it does not certify passing acceptance. Spread/fire chains, growing storage and phone evidence must lead M4.

Reviewed HEAD a78a07f8 against the pre-amendment PROJECT contract, required documents, history from the initial plan, and relevant sim/view/game/tools paths. No code, tests, game or build ran. Reported test passes below are retained evidence, not independently rerun results. New concerns are static findings requiring reproduction. Proposed PROJECT edits must not retroactively turn M3 failures into passes.

Short code paths below start at sim/src/kd/ unless they begin view/ or game/.

Findings, most severe first:

1. **Autonomous acceptance still fails.** Original spread is jointly 5/20, not 6/20 (dist/M3-CHECKS.md:10). Fresh 6101–6120 stopped at 1/6: fourteen remaining successes give only 15/20, below 16. Fire 7101–7120 stopped at 0/11: at most 9/20, below 10. Every completed fire run had zero tending/cooking. These are failed first batches; a fresh fire batch under RES-13 was not completed. Old spread cannot reach 32/40 even with twenty further successes. Wet control completed; fresh spread control did not. Records: dist/M3-REPAIR-GATES.json:83,350,736. No seed shopping or changed T values established. The later plough-observer repair cannot rescue zero tending/cooking.

2. **Storage and hot work scale with the past.** Spent items remain entities (demo/craft_work.cpp:26); every kept choice appends permanently (demo/choice.cpp:31). Day 45 retained 22,541 spent items (dist/M3-REVIEW.md:34); recorded seed 7112 has 438,943 choices and a 9,982,452-byte snapshot after three years (dist/M3-REPAIR-GATES.json:323). Fire handling scans all things (demo/fire.cpp:416,422), food refresh scans stock (demo/fire_food.cpp:182), learning repeatedly builds maker sets (demo/learning.cpp:33,108), and moving thermal exposure iterates seconds (demo/fire_thermal.cpp:75). Display publications copy full CraftHistory (view/src/crowd_core.cpp:155); History searches choices for each event (view/src/world.cpp:1255). Existing event skipping is real (world/world.cpp:511); crafting nevertheless forces serial execution (demo/living.hpp:12). Recorded serial speed around 0.6–2.6 years/minute is far below tens. Indexing repairs help; they do not bound storage or prove parallel speed.

3. **PRN-13/MND-09 are repaired only in part.** Fire/craft/warming now retain immutable reasons, saved and displayed (demo/choice.cpp:3; world/craft_store.cpp:281; game/camp/words.gd:165). But living.cpp:571–578 clears them then tries warming→fire→teaching→craft in priority order. It never compares all categories together. Choices::keep ranks supplied alternatives plus body options, not necessarily the best two rejected choices overall; it does not keep MND-09's three decisive score differences. Body choices lack this same immutable chain. Fix the chooser before assuming motivation tuning solves spread.

4. **Two narrow validation/observer gaps remain.** Pending ideas now require a real person, and hearth Item/Fire ownership agrees even when zero (world/world.cpp:1515; world/fire_store.cpp:90): original finding fixed in source. However the live-target condition applies only to kind=1 ideas; pending kind=0 place dreams can name a person-family ID made from another allocated serial. The later check validates only targets that exist (world/world.cpp:1653). Add a corruption regression and typed ended-person handling; do not reject legitimate historical targets indiscriminately. Separately, the fire observer checks chronological ember/tend/flame/cook times without linking the same fire lineage (proof/fire_cases.cpp:5,69). This could count unrelated events as one future success. Current zero-stage failures remain valid.

5. **Founder experience contradicts BIO-20.** Required sector bonus is +1 per 15 years over 20; code adds 0.1 per decade over 18, also to starting blueprint skills (demo/crafting.cpp:40). Full starting family bands/home-range knowledge are absent, appropriately still partial; this numeric mismatch needs correction, not a retrospective rule change.

6. **Repairs are source-only; phone acceptance is still owed.** Focused passes are claimed in dist/M3-CHECKS.md:65–89. Its line 97 records the older routine/audit, not a fresh repaired audit; the repair routine was interrupted. APK 41305 predates fixes (dist/NOTE-3.13e.md:11). No current physical-phone digest, sustained frames, hour-long battery/heat, sound judgement or unaided play is proved. ARM emulation and M1 markers cannot supply them.

The named material/perception repairs are present: generic thermal cooking fits (demo/fire_food.cpp:132), perceived known uses (demo/craft_work.cpp:181), masked uncertain fire evidence and generic maintenance (demo/fire.cpp:38,59), learner-owned acceptance/reply (demo/teaching.cpp:126) and truthful knowledge exchange (teaching.cpp:73). This addresses roots/meat, bank/carry and the original hidden burn/private-needs reads. Source regressions cover renamed/hidden inputs; their recorded passes remain claimed evidence. MND-23's full social knowledge model is not finished.

What each milestone actually delivers:

| Milestone | Built / partial / missing |
|---|---|
| M1, MIL-08 | Built and previously accepted: keyed chance (chance/chance.cpp:28), canonical event scheduling (world/world.cpp:511), snapshots/strict chunks (world/world.cpp:868), journal/history/recovery (save/keeper.cpp:207,324), bridge/display and device tools. Large living-world performance is unproved. |
| M2, MIL-09 | Built: 25 bounded adults (demo/crowd_world.cpp:506), food/water/rest, remembered supplies, finite renewal and saved place dreams (demo/living.cpp:488,783; world/life.hpp:21; world/world.cpp:1421). Partial presentation/owner acceptance. No actual starvation/dehydration death, wounds, births, ageing lifecycle, society or full ecology yet. |
| M3, MIL-10 | Built: conserved craft work, personal evidence/learning, fire/thermal food, memory-based ideas and factual linked History. Mechanical/conditional scene evidence is substantial. Population spread and autonomous chain fail; full choice explanations, strict identities and phone evidence remain partial. Conditional idea setup uses real installed prior handling (proof/idea_cases.cpp:78), not evidence of autonomous discovery frequency. |

Every active principle (paths below are relative to sim/src/kd unless prefixed):

| Principle | Static verdict and evidence |
|---|---|
| PRN-01 | Scoped repair present; personal evidence/replies above. Full minds/starting knowledge partial. |
| PRN-02 | Bounded simple camp fits; unbounded diagnostic retention has no corresponding play benefit (demo/choice.cpp:31). |
| PRN-03 | Scoped place/idea dreams influence choices indirectly; full nature powers absent (demo/living.cpp:543; proof/idea_cases.cpp:78). |
| PRN-04 | Partial: cards/History expose many facts, but complete kept explanations remain open (view/src/world.cpp:1215,1231). |
| PRN-05 | T catalogues and trial machinery exist; realism/full catalogue acceptance unproved (proof/fire_cases.cpp:25; dist/M3-CHECKS.md:97). |
| PRN-06 | Holds for built path: factual templates, no language model chooses (game/camp/words.gd:376). Optional writer absent. |
| PRN-07 | Original name-based violations repaired in reviewed paths (demo/fire_food.cpp:132; demo/fire.cpp:59). |
| PRN-09 | Playable increments delivered; post-review source repairs lack APK by owner instruction (dist/NOTE-3.13e.md:11). |
| PRN-10 | Scoped actual state/records drive words and sound (game/camp/words.gd:376; sounds.gd:59); full presentation unproved. |
| PRN-11 | Partial: owned display boundary exists (view/src/crowd_core.cpp:130); retained growth and phone smoothness remain open. |
| PRN-12 | No runtime tuning or story forcing found in ordinary path; keyed choice/speed-independent rules (demo/living.cpp:588; world/world.cpp:511). |
| PRN-13 | Partial, finding 3. |
| PRN-14 | Modular data/world/view boundaries retained; no new simulator needed (world/world.cpp:991; view/src/crowd_core.cpp:130). |
| PRN-15 | Saved events/reasons, no past-view re-simulation (save/keeper.cpp:324; view/src/world.cpp:1231); compaction missing. |
| PRN-16 | Principles can be reviewed, not declared all passed; this table supplies the independent account. |
| PRN-17 | Not met: spread/fire acceptance fails; no adaptive runtime mechanism exists yet (dist/M3-REPAIR-GATES.json:83). |

PRN-08 is retired, not an omitted active principle.

Scope/docs: retained renderer studies and art exceed the current valley slice but belong to owner-directed earlier work/future requirements, not evidence of an unauthorised gameplay feature. First-flake examples/finite proof reserves are inspection/test fixtures, not ordinary-world acceptance. No additional unrequested production mechanic was established. ARCHITECTURE previously said M3 awaited review and overstated complete rejected-choice explanations; this draft corrects those. PROJECT's audit paragraph contradicted the approved trimmed A17/guide policy; corrected. CRITIQUE is historical and its older-save-test statement is superseded by the 9 October refusal decision. Old full-scale speed promises conflict with the owner's new tens-of-years target; proposed replacements are explicit, not achieved results.

M4 order: validate/explain → bound storage and hot work → repair/test learning and fire → seasons → care → family. Keep M3 closed, failed counts visible, and all later full-system acceptance open.
