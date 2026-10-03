# The review's checklist (PRC-09, A15.13 step 3)

Since the owner's instruction of 3 October 2026, the builder reviews each lettered alpha that a later letter of its
number follows, in a pass of its own after the delivery; the alpha that ends its number gets one fresh subagent (or a
reviewer in its own session), which works from the whole number's diff, its alphas' sections of `IMPLEMENTATION.md`
and the items they cite, never the builder's reasoning. Either reviewer re-runs format, clippy, the tests and the
quick scenes, and reverts each new test's code to see the test fail.

1. **Items.** For each ID the alpha claims, read its What, Done when and Check lines in `PROJECT.md`: the diff does
   what the alpha's part of it asks, and no more than the alpha says.
2. **Tasks.** Every task of the alpha is built and committed, each commit naming its task and IDs.
3. **Tests.** Every test and scene the alpha names exists, passes, carries its `// checks:` line, and fails when its
   new code is reverted.
4. **Nothing weakened.** No test weakened and no pass rule loosened (`RES-09`); a changed rule carries its reason.
5. **Catalogue.** Every catalogue entry the alpha adds names its checks (`MAT-17`), and appears in its Data line.
6. **Rules every alpha keeps** (the plan's section of that name), above all:
   - `PRN-01` the world is the only teacher: no choice on an unknown blueprint or unseen fact;
   - `PRN-06` language models describe, never decide;
   - `PRN-07` generic blueprints: no content names in rules;
   - `PRN-12` no rule-bending settings in play;
   - `PRN-14` modular: nothing that works rewritten without a stated reason.
7. **Determinism** (A3.1): keyed chance only, `kd_core::m` and `num` for maths, fixed order, no clocks or hash order
   in the world.
8. **Layering** (A2.3): crates depend only as `tools/layers.toml` allows; simulation crates keep the bans.
9. **Budgets:** benchmark slowdowns over 10%, APK growth over 5 MB, checks over 20 minutes are flagged in the note.
10. **Scope:** nothing outside the alpha's scope changed without a note; Conflict notes name the smallest change.
11. **`PROJECT.md`:** no implementation detail in it (`PRC-04`); any change carries its `Changed:` line and the owner's
    OK (`PRC-07`).
12. **Pictures** (the owner's request of 3 October 2026): open every picture the alpha makes or changes (the goldens
    in `tests/golden/`, the smoke's shot in `target/screens/smoke/`, the note's pictures) and look at them as a pixel
    artist and a designer: the alpha's On the phone steps show in them; every pixel is a whole 4 × 4 block in the
    palette's colours; light falls in clear steps, dithered only in narrow bands where two steps meet; outlines, rims
    and shadows fall where the shapes and the light put them; and nothing is smeared, noisy, streaked, banded or
    broken that should not be. Name each fault with its picture and where in it, and say how it should look.

Verdict, added to the pull request's description: `Review: APPROVE <commit> <reviewer>`, the reviewer `builder`,
`subagent:<label>` or a session, or `Review: CHANGES` with the findings.
