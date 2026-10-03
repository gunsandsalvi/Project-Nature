# Review checklist (A15.13 step 3, PRC-09)

The reviewer works through every line for the alpha in review: the builder for a lettered alpha that a later letter of its number follows, a fresh subagent (or a separate session) for the whole number at the alpha that ends it.
A subagent gets only the number's whole diff, its alphas' sections as the plan held them when the number began (on `main` before its first alpha merged), and the items they cite; never the builder's reasoning.
Each line is answered yes, or with a finding.

## Run

1. Format, clippy, the tests and the quick scenes pass when the reviewer runs them again (`tools/check.sh`).
2. Each new test fails once when the code it checks is broken on purpose, and passes again when restored.
3. The head of the pull request is the commit the checks passed on, or its parent when the head adds only `results/`.

## Scope

4. Every task of the alpha's section is built, and each commit names its task and IDs.
5. Every test the section's **Tests** line names exists, checks what it says, and passes.
6. Nothing outside the section's scope changed without a **Conflict:** note or an architecture note saying why.
7. The section and its status row have left the plan, their decisions and measured results moved into the architecture, and the coverage map is current.

## Items

8. For each claimed ID: the part delivered meets its **What**, **Done when** and **Check** lines in `PROJECT.md`, as far as the alpha claims.
9. No test was weakened and no pass rule loosened to pass (`RES-09`); a changed pass rule carries its reason.
10. Every catalogue entry the alpha adds names its checks (`MAT-17`).
11. `PROJECT.md` holds no implementation (`PRC-04`), and any change to it carries its `Changed:` line and the owner's OK (`PRC-07`).

## Principles and rules

12. The principles hold, above all: nothing knows what it has not learned (`PRN-01`); language models describe, never decide (`PRN-06`); rules never name content (`PRN-07`); no rule bends for play (`PRN-12`); everything is built from modules (`PRN-14`).
13. Determinism (A3.1): no clock, thread count, hash order or platform maths reaches the world; floats are cleaned before they are stored or hashed; chance comes only from keyed draws.
14. Layering and bans: `kd check layers`, `kd check names` and `tools/check-banned.sh` pass, and nothing is allowed by a new exception without a reason.
15. Budgets: the alpha's benchmarks are recorded, and anything over budget or 10% slower than the last alpha is flagged in the note.

## Pictures

16. The reviewer looks at every picture the alpha makes (goldens, the smoke test's screenshots, the note's pictures) as a pixel artist and a designer would, and says whether the graphics are as they should be:
    - every art pixel crisp and square, no blur, no stray half pixels, no shimmer where nothing moves;
    - colours from the palette, light and shade reading clearly, nothing muddy;
    - the layout right at the phone's size in portrait and landscape, nothing clipped by the screen's rounded corners, its cut-out or its edges;
    - text legible at its size, and nothing on screen the alpha did not mean to show.

## Delivery

17. The APK is signed with the release key, its checksum matches, it is at most 50 MB, and its link downloads it.
18. The web page and the note are published at their links, and the note says in plain words what is new, what to try, what is rough, the IDs delivered and both links.

## Verdict

The reviewer adds one line to the pull request's description: `Review: APPROVE <commit> <reviewer>`, the reviewer `builder`, `subagent:<label>` or a session, or `Review: CHANGES` followed by its findings.
A later push voids an approval.
