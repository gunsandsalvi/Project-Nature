# Kindling: evidence and process decisions

The owner adopted the camp-first plan and all 22 first-review proposals on 8 October 2026.
The owner also approved the testing-policy changes below on 8 October 2026. Further game-scope cuts remain undecided.
This is a decision note, not another specification. Earlier review detail is preserved in commit `a9da2bf`.

## What the review established

At `8928aca`, Kindling had reusable foundations and graphics fixtures, but no production needs, learning, crafting or indirect-power loop.
The registry contains demo markers and activities (`sim/src/kd/world/world.hpp`); Crowd uses a walking/resting/greeting state machine (`sim/src/kd/demo/crowd_world.cpp`).
Examples renders fixed candidate art and suppresses crowd actors (`game/terrain/drawing.gd`).
A saved demo world is not a generated living world.

The review passed 163 simulation cases, 57 native view cases, the numerical oracle and one/four-worker proof suites; 173 Python tests also passed.
The routine check still failed on formatting/lints, three app assertions in two cases, and 15 missing ID annotations.
Normal Examples navigation rendered; direct startup failed with a parent-busy setup error.
This was explicitly stopped WIP, not a secretly failed accepted release. A17.0 holds the repair list.
No physical-phone performance or APK acceptance was proved in this review.

There were real owner-directed resets (`1f65726`, `dd68890`), and the vertical slice was explicitly dropped (`4320212`).
However, all 199 sampled M1 source files survive; 141 are byte-identical.
Keep those foundations. The production mistake was putting the game behind graphics, planetary simulation and a large art catalogue.
The adopted plan reverses that order without another restart.

## Are we too pedantic?

**Yes: about labels, repeated proof rituals and reporting. No: about deterministic state, saves and recovery.**
An ID beside a test is useful navigation. It is not evidence that the test establishes the requirement.
The former checker counted labels and their proximity; it could not establish that every blueprint, behaviour or acceptance clause was covered.
A handwritten “owner OK” in a commit is similarly not an approval mechanism.

The changes below are **approved and applied: owner OK 8 October 2026**. PROJECT.md records the approval with each changed decision.

| Rule | Approved policy | Reason |
|---|---|---|
| IDs in every commit | **Drop the mandate.** Describe the change; cite IDs when useful. | Commit subjects should explain behaviour, not satisfy a second indexing system. |
| IDs in code and every test | **Loosen.** Keep existing IDs and useful module/suite links; stop requiring one beside every test. | Fifteen annotation failures do not reveal fifteen broken behaviours. Meaningful test names and assertions matter more. |
| `Changed:` trailer and commit gate | **Drop format enforcement.** Keep the owner's decision, date and reason with the changed requirement. | The checker trusts literal words, adds history dependence and blocks on an already-pushed message typo. It does not verify consent. |
| Coverage checker and `checks:` | **Keep traceability; drop completion inference.** Check live/retired IDs and plan mapping, retain `where`, make source annotations optional. | Broken references are objectively checkable. Feature completion needs behaviour evidence and review. |
| Detailed run budgets for every test | **Loosen.** Ordinary unit/regression tests state expected outcomes in assertions; scenario/statistical/performance tests retain declared inputs, thresholds and budgets. | A one-line boundary test does not need a research protocol. Existing acceptance thresholds still stand. |
| Every test must first fail on a planted fault | **Loosen.** Reproduce regressions before fixing them; use fault injection for critical invariants and error paths. | Manually sabotaging every UI or ordinary unit test is a costly ritual. Save corruption, lost events and duplicate effects deserve adversarial tests. |
| Routine check and audit | **Keep the approved split.** Remove only the duplicate checker selftest already run by its unit tests. | Host tests and quick deterministic proofs protect integration. Wider compiler/sanitizer/recovery/performance audits belong at their existing triggers and stage close. |
| Review each step, then the whole alpha again | **Loosen.** One builder self-review per integrated delivery; independent review at milestone close and for high-risk foundation changes. | Re-reading the same small diff under two names is not two independent protections. |
| Delivery notes | **Loosen.** Three short sections and the APK link; add material limitations and test results where relevant. | Mandatory “IDs delivered” and separate links sections add formatting, not player understanding. Keep partial acceptance honest. |

No adopted change removes deterministic proof suites, corruption/recovery tests, old-save checks, behavioural assertions, formatting/lint checks, phone evidence or audit triggers.
The existing implementation failures remain the first camp task; this round does not repair them or weaken their assertions.
The 15 annotation failures disappear because their metadata requirement was deliberately removed; gameplay coverage has not increased.

### Applied policy changes

PROJECT, ARCHITECTURE, IMPLEMENTATION and the agent guide now match the changed checker and its tests.
The checker removes commit-message inspection and its dependency on git history; checks document structure directly; validates IDs that are present; retains the plan's requirement map and search command.
It stops inferring “tested” from matching labels and stops requiring annotations within four lines of each test.
Old notes remain valid; new notes need only What is new, What to try, What is rough and an APK link.
The routine script stops running the checker fixture selftest twice; it remains covered by the tool unit tests.

This policy change is applied. Pushed history and unrelated source failures are unchanged.

### The pushed commit exception

Commit `a9da2bf` omitted its required trailer. The owner explicitly authorized an honest repair without rewriting it.
`tools/filecheck-known.txt` records that exact historical finding and reason; the output now calls it a documented exception rather than falsely saying it awaits approval.
That exception repaired the old policy honestly. The adopted checker no longer inspects commit messages; it checks current document structure directly.

### What does a delivery cost?

The earlier lead recorded a 133-second warm delivery check after the routine/audit split, compared with about 23 minutes before it (`drafts/handovers/lead.md` at `a9da2bf`).
That is a historical report, not a measurement of today's whole delivery.
Building, signing, visual inspection, fixes, review and owner waiting are additional costs; there is no trustworthy total yet.
For the next three deliveries, record actual time in building, checks, review and packaging in the short note. Do not promise a fixed six-hour delivery or remove valuable tests to meet it.

## What small-team evidence supports

There is no evidence that every indie uses one process, or that successful teams dispense with automated tests.
The useful distinction is between tests that expose a costly failure and ceremony that merely asserts the work was checked.

- **Suspicious Developments — Tom Francis:** his account of a studio averaging about three full-time salaries stresses small scope, early prototypes and fresh testers. This supports a short integrated delivery and owner play feedback. It does not establish an annotation or commit-message convention. [First-person small-team account](https://www.pentadact.com/2026-01-08-15-years-of-indie-dev-in-4-bits-of-advice/).
- **Factorio's early team:** its 2014 account describes gradually expanding automated tests and using replay/save comparisons to expose determinism errors. That is directly relevant to Kindling's valuable protections. [Early testing account](https://www.factorio.com/blog/post/fff-29).
- **Factorio's larger later team:** a heavy suite saved and loaded every tick; the team moved slow checks onto automatic servers so development could continue. Later guidance describes useful GUI tests and reproducing bugs before fixing them. Keep risk-driven checks, rather than requiring every test to undergo a separate sabotage ceremony. This mature project is not a small-team staffing template. [Slow-check workflow](https://www.factorio.com/blog/post/fff-315), [GUI and regression testing](https://www.factorio.com/blog/post/fff-366).
- **Subnautica:** its experimental builds were automated, with no manual changelog for every such build. This supports reducing delivery bookkeeping; it does not justify shipping known failures through Kindling's stable merge gate. [Developer delivery account](https://unknownworlds.com/en/news/subnautica-get-daily-updates).
- **FTL and A Short Hike:** their postmortems support a playable experience and a deliberate small finish before scope expansion. Neither gives Kindling a predictable schedule. [FTL developers](https://www.gamedeveloper.com/business/a-mini-postmortem-roundup), [A Short Hike official abstract](https://www.gdcvault.com/play/1026613/Independent-Games-Summit-Crafting-A) (abstract only verified).

These sources support the adopted camp-first order. The exact ID, trailer and review recommendations are this review's judgement about this repository, not claims that those developers used the same rules.

## Document discipline

The working guide states each document's purpose and size budget.
PROJECT remains the binding reference; it cannot become a tiny brief while retaining hundreds of detailed decisions.
The plan should be the short daily view: next playable result, current tasks, checks and what remains open.
Architecture keeps reusable contracts and consequential gotchas, with distant systems reduced to pointers and outstanding acceptance obligations.
Superseded handovers can be deleted once their useful facts are there.

Removing prose is not permission to remove a feature, numerical limit or protection.
Any further reduction that changes such a decision is a new scope proposal, not editorial cleanup.
Word counts before and after are in OWNER-SUMMARY-2.txt; they include deleted handovers so moving text is not mistaken for cutting it.

## Further cuts that would change decisions

These were not applied and are not part of the testing patch. Recommendation: decide them when their expansion is commissioned, after the camp slice is playable.

- Replace fixed future catalogue counts with the content needed for the chosen expansion (`WLD-31`, `WLD-32`, `MAT-10`, `MAT-23`). Current counts remain binding meanwhile.
- Reconsider detailed disease/wound arithmetic and customs/leadership rules against visible outcomes (`BIO-05`, `BIO-13`, `CUL-06`, `CUL-22`, `CUL-26`). Keep causes, care and individual histories; do not silently remove the old rules.
- Reassess global ecological/climate sampling and complete art/motion/sound inventories when their real consumers exist (`WLD-08`, `WLD-16`, `WLD-18`, `PRE-22`, `PRE-44`, `PRE-46`, `SND-06`). Any changed threshold or promised count needs a separate decision.

This is why PROJECT remains long: making production optional did not retire the underlying full-game promises.
