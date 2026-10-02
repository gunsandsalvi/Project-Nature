# The independent review's checklist (PRC-09, A15.13 step 3)

The reviewer works in its own session, from the pull request's diff, the alpha's section of `IMPLEMENTATION.md` and
the items it cites, never the builder's reasoning. It re-runs format, clippy, the tests and the quick scenes, and
reverts each new test's code to see the test fail.

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

Verdict, added to the pull request's description: `Review: APPROVE <commit> <reviewer session>` or
`Review: CHANGES` with the findings.
