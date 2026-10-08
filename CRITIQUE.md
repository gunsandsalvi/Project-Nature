# Kindling: independent review, 8 October 2026

You have built substantial foundations and an art workshop. You have not yet built Kindling's central game.
The biggest problem is the order you chose: the plan rewards completed components and approved pictures while postponing the experience that could justify them.
More detailed instructions and more agents will not repair that order.

Keep this codebase. Repair the stopped work, then make one small camp worth watching and influencing.
Do not commission another foundation, renderer or exhaustive world plan before that camp works.

This review examined commit `8928aca` in a separate working copy.
I read PROJECT.md completely, audited and built the code, then read ARCHITECTURE.md and the old IMPLEMENTATION.md completely, then examined history.
I recorded my verdict before reading HANDOVER.md and `drafts/handovers/lead.md`.
Code and history audits were independently delegated; the build, app inspection and final conclusions were checked here.
No APK was built or signed, no physical phone was available, and no commits, branches or git history were changed.

## What is actually here

The distinction is between useful infrastructure and unproved game behaviour, not between work and no work.

| Evidence at the reviewed commit | What it establishes |
|---|---|
| 56,820 lines of own source and tests across sim, view, game and tools | A substantial engineering investment; not a completion percentage |
| 163 simulation and 57 native view test cases pass | Meaningful current foundation and view contracts |
| 10 registered app pages, one main scene | A development application with several demonstrations |
| 33 source data TOML files | Demo, tuning and look data; no living game catalogue |
| 6,259 tracked art files, about 251 MiB | Includes sources, masks, levels and generated variants; not that many distinct designs |
| 373 catalogue pieces, 129 signed design sheets | A large art commitment; signature is not animated runtime integration |
| 99,811 words across the original three main documents | 5,808 lines of specification, architecture and plan for one owner to navigate |

Counts exclude vendored libraries and generated builds from source totals; game/tools totals include their tests.
The three document word counts use whitespace-separated words.
The repository has 6,909 tracked files.
These are inventories, not estimates of value or time spent.

**The app does not simulate people yet.** `game/main.gd:6–17` registers Check, Examples, Time, Crowd, Worlds, Catalogues, Reports, Bench, Fixtures and Terrain; it starts on Check.
The registered world components are Activity, demo Home/MarkerKind, Ident, Place and Schedule (`sim/src/kd/world/world.hpp:36–37`).
The crowd chooses walking, resting, sleeping and greetings through a small state machine (`sim/src/kd/demo/crowd_world.cpp:301–369`).
It has no needs, craft knowledge, meaningful memories or discovery.
Calling this “a thousand minds proved” from an old prototype does not make those minds exist in production.

**A new saved world is a saved demo.** `view/src/worlds.cpp:195–212` creates demo metadata; `game/pages/worlds.gd:82–97` opens Crowd.
There is no production planet generator, ecosystem, body simulation, generic craft catalogue, culture or nature-power loop.
These are absent, not merely hidden behind unfinished art.

**The attractive scene is a fixture.** Normal navigation into Examples renders a tree, boulder and shelter on repeated grass.
I inspected a fresh 1080×2400 software-rendered capture; Crowd also renders and is labelled “Simulation markers”.
Examples has its own demo save (`game/pages/examples.gd:30–31`), and candidate mode deliberately removes the actors (`game/terrain/drawing.gd:138`).
Terrain's scene records are fixed test inputs (`view/src/terrain.cpp:165–229`).
That is legitimate graphics testing. It is not evidence of a living camp.

**Some broad claims exceed their proof.** The coverage checker regards an ID comment as implementation and a matching test annotation as testing (`tools/filecheck.py:566–616`).
It cannot establish every clause of an item or every blueprint's trial merely by seeing an ID.
`TIM-17` on the generic Activity machinery does not prove interrupted meals or real bodily effects.
Likewise, the smooth-export claim in `game/pages/worlds.gd:11–14` exceeds the current implementation: `ArchiveWriter::next` reads and hashes a whole constituent file before yielding (`sim/src/kd/save/archive.cpp:68–94`), synchronously called by the view.
A small demo export is no proof of smooth multi-gigabyte history export.

The architecture is often more honest than the project's shorthand: it marks later systems as outlines and recognises memory and timing risks.
The failure is treating isolated prototype results and foundation contracts as enough confidence to postpone integration.

## What passed, and what did not

The latest commit is explicitly a stopped, unfinished snapshot (`750abc1`, `8928aca`).
Its failures are resume blockers, not evidence that the previously delivered APK secretly failed its acceptance checks.

I sourced `tools/env.sh` and ran the routine `tools/check.sh`, with its network git-fetch operation suppressed to obey this review's read-only-git instruction.
It stopped at formatting.
Separate diagnostic continuations temporarily ignored only formatting/lint exit statuses to expose later results; those runs are not a passing routine check.

- Host simulation and extension builds succeeded. All 163 simulation and 57 native view cases passed; the numerical oracle made 3,810,073 comparisons with no mismatch. Eight existing proof suites matched on one and four workers.
- Catalogue validation passed: 578 files, 574 entries, 15 kinds, nine checks. Those are today's registered kinds, not the promised species and blueprint catalogues.
- 173 Python tool tests passed. The app ran headless, and 47 scripts compiled without errors after import recovery.
- The routine gdUnit run executed 62 cases and reported three failed assertions in two cases: fixture orientation (`game/test/fixtures_test.gd:107,109`) and terrain selection (`game/test/terrain_test.gd:154`). The long benchmark was excluded, as in the routine check.
- Formatting fails in `game/fixtures/drawing.gd`. Three files fail line-length lint: `game/test/atlas_lifecycle_test.gd`, `game/test/terrain_stream_integration_test.gd`, `tools/stream-bench.gd`.
- ID coverage fails on 15 missing test annotations across seven test files. The original document check passes. These are different checks and must not be reported as one green result.
- First import logged a missing bitmap-font resource; a subsequent import succeeded. Godot also created three missing script UID files and changed one import descriptor. Those review-generated changes were restored/removed.
- Direct startup into Examples through the repository's screenshot route produces a blank world and a parent-busy `add_child` error at `game/fixtures/sprite_stream.gd:27`. Entering Examples after the main scene is ready renders correctly. The fault is entry-path-specific; it does not establish that ordinary navigation is broken.

The expanded compiler/sanitizer/Android matrix, long pace tests and physical phone performance were not rerun.
Screenshots from the cloud establish appearance on that renderer, not mobile speed, heat or touch comfort.
Logs and captures are retained with OWNER-SUMMARY.txt in the review scratch folder.

## The plan makes the wrong work urgent

**You explicitly put the game last.** `SCP-16`, `PRN-09` and the old plan defer the playable surface to M9.
People arrive in M5, minds in M6, crafts in M7, powers/history in M9.
The very things that distinguish Kindling are downstream of complete graphics and a sophisticated empty world.
That is not a dependency imposed by code: a small real person can eat a real item on simple local ground without a tectonic generator.

**The vertical slice was removed precisely where it was needed.** Commit `4320212` accepts M1 and drops it; `MIL-18` records the decision.
Earlier disposable tests can answer whether algorithms run. They cannot answer whether people, knowledge, indirect control and readable consequences form a game together.
The missing proof is integration and interest.

**The scope behaves like a finished encyclopaedia.** The specification asks for roughly 60 plants, 30 wild species, five domestic kinds, 15 illnesses, 190 items, 140 blueprints, 45 animations and a centuries-long arc.
The planet has two million world cells, detailed geological causes, climate validation and an eclipse guarantee at every place.
Each demand may be defensible in isolation. Together they overwhelm the next useful decision: what makes one person worth following?
`PRN-02` favours believability over exactness and `SCP-21` rejects deep science; the universal eclipse certification in `WLD-07` pulls the other way.

**The apparent precision does not make the schedule credible.** At `8928aca`, M2 lists six six-hour deliveries and M3 lists 24 four-to-six-hour deliveries: 132–180 estimated builder hours for those whole stages, not a measured remaining forecast, before real people.
M3 already has 72 tasks while M2 is unfinished.
The “only next milestone detailed” rule has become aspirational.
Its G7 route separately budgets about 100–110 minutes for 25 generation/settling measurements, plus the 20-minute play route.
The old M2 renderer comparison spans two devices, two renderers and two frame modes: eight sustained runs before repeats.
These may be useful final proofs. They are poor gates in front of the first discovery.

**Art has become a competing product.** Reference approval, runtime pages, material channels, normals and engine appearance are distinct stages, which is technically reasonable.
Requiring that whole process across a 373-piece catalogue before the gameplay has settled is not.
The owner is asked to manage an asset factory before knowing which actions and views matter.
Keep the signed designs. Stop filling the catalogue. Build the next scene with readable stand-ins and upgrade the handful of assets it actually uses.

## History: real resets, but do not restart again

This history spans 1–8 October: eight calendar days, not a long failed commercial production.
Its 762 commits are not 762 units of progress or a measure of labour.
They do show repeated changes in direction.

| Commit | What happened | Fair interpretation |
|---|---|---|
| `640a7d9`, 2 Oct | 55,862 lines of pretests deleted | Intentional disposable experiments, not a finished game thrown away |
| `1f65726`, 3 Oct | 310 files, 20,564 lines deleted | Explicit owner-directed codebase reset |
| `dd68890`, 4 Oct | 226 files, 34,902 lines deleted | Another clear-out about 30 hours later; replacement experiments followed |
| `29beda4`, 5 Oct | 38,083 lines deleted as pre-production closed | Prototype retirement; useful results retained, integration still unproved |
| `4320212`, 6 Oct | M1 accepted; vertical slice dropped | A consequential production choice, not an accidental omission |
| `ac7ac07`, 6 Oct | Research/lessons folded into three documents | Fewer documents, but weaker convenient access to decision evidence |
| `f7b5389`, 7 Oct | Earlier art lane removed: 975 files | Another owner-directed change before the central loop was playable |
| `3adead5`, `1662f0b`, 8 Oct | 2D pivot; obsolete 3D runtime removed | A rendering change; core simulation was preserved |

Crucially, all 199 selected source files present at accepted M1 still survive at HEAD; 141 are byte-identical and 58 changed.
The sample covers own C++, GDScript, shaders, Python and shell files in the main code/tool directories, not all assets.
The chance, runner, world and save-keeper implementations are among the unchanged files.
The foundations are not being rebuilt at every recent turn.
Another purge would manufacture that problem again.

Document churn is measurable: 174 of 707 non-merge commits change only Markdown.
Across PROJECT, ARCHITECTURE, IMPLEMENTATION and CLAUDE, non-merge diffs add 35,389 lines and remove 29,556.
Repeated edits and formatting count repeatedly; these figures do not prove equivalent wasted hours.
They do show that rewriting the description of progress has become a major output.

## Your production habits need to change

**You are the source of some of the instability.** The resets, discarded slice and camera/art changes were explicitly your decisions.
Agents should not be blamed for obeying them.
But agents also failed as producers when they accepted the growing prerequisite list without insisting on a small game-shaped test.
An agent can satisfy every local instruction and still steer the project away from a finish.
You need fewer simultaneous priorities and a stable short-term target, not a more elaborate hierarchy.

**Delegation has multiplied handoffs.** CLAUDE.md reserves the main session for coordination and forbids it from doing code or design.
The handover records three leads being replaced by one lead on the same day.
At this size, one builder should own the complete running increment and integrate it continuously.
A supporting art or research task is useful when its consumer is already clear.
A chain of leads, critics and messengers cannot substitute for that ownership.

**Approvals must protect decisions, not consume the owner.** `PRC-11` already says only milestone reviews wait for you; keep that.
But the later no-APK-until-art-looks-decent instruction and repeated sheet/runtime/scene gates pull play feedback back behind aesthetics.
The proposal is not to send unreadable junk: require legible people, actions and controls in early builds, then judge final beauty in the relevant scene.
Approve a direction or a finished playable increment, not every intermediate asset representation.

**Do not cut the useful checks.** Determinism, saves, corruption recovery and meaningful behaviour regressions are assets.
`eca18a3` already separated routine checks from the expensive audit; credit that improvement rather than recommending it as new.
Cut repeated review of unchanged work and speculative acceptance infrastructure.
Keep one independent milestone review, and distinguish technical correctness from whether the owner wants to continue watching.

**The documents need truth, not just valid links.** Examples of drift: CLAUDE.md says 127 signed pieces while the catalogue has 129; `PRE-02` fixes the north-facing camera while `PRE-33` still offers rotation; A8.1 still says 256-metre areas while `WLD-12` specifies 250; A10's body outline carries different mortality/lifespan/birth-spacing assumptions from PROJECT.
The ID checker can pass all these contradictions.
Keep PROJECT authoritative, reconcile the architecture immediately before building its consumer, and stop expanding distant designs.

After recording my verdict, I read the two handovers named above.
I agree with their candid WIP status, preserved M1 proofs, known texture-lifetime issue and refusal to claim cloud timing as phone proof.
I disagree with their production prescription: finishing the art review matrix and further ground packs before returning to the original world-first route repeats the central mistake.
Their call to make the art process many times faster treats throughput as the problem; the more important question is why so much art is on the critical path now.
The recorded no-delivery art gate remains binding until the owner accepts the specific proposal below.

## What the indie evidence actually supports

There is no single “real indie” method, and none of these sources guarantees a schedule for Kindling.
The useful common lesson is to make the whole experience testable while changes are still cheap.

- **FTL — Justin Ma and Matthew Davis, 2013.** Their own postmortem describes four months spent on mechanics before structure and pacing came together near an external prototype deadline. The relevant milestone was a playable game, not another subsystem. Their account also credits luck and publicity; it is no reliable time estimate for this project. [Developer postmortem](https://www.gamedeveloper.com/business/a-mini-postmortem-roundup).
- **Into the Breach — Matthew Davis and Justin Ma, 2018.** They describe developing small pieces that could immediately be played, constant internal testing, and the difficulty of a strategic layer whose parts needed to come together before it could be judged. This supports integrating the next system into an existing game. [Direct developer interview](https://www.gamedeveloper.com/business/how-subset-games-made-the-jump-from-i-ftl-i-to-i-into-the-breach-i-).
- **Factorio — Albert, 2016.** Their entity workflow starts with a functioning prototype using placeholder graphics, then iterates its behaviour before final graphics. This supports a small camp art kit after behaviour exists, not abandoning visual direction altogether. [Developer production account](https://www.factorio.com/blog/post/fff-146).
- **Factorio: Space Age — kovarex, 2024.** The team built a bare playable expansion, tested mechanics cheaply and changed scope after playthroughs exposed repetition. This is useful evidence for a vertical slice and cuts, but that team was nearly 30 people: it is not a staffing or schedule comparison. [Development account](https://www.factorio.com/blog/post/fff-417).
- **A Short Hike — Adam Robinson-Yu, GDC 2020.** The official talk abstract describes pausing a larger project and shaping a tiny open world for an initial four-month release. The lesson is a finishable scope with a deliberate aesthetic, not that simulation games take four months. Only the published abstract, not the full talk, was verified here. [Official postmortem abstract](https://www.gdcvault.com/play/1026613/Independent-Games-Summit-Crafting-A).

The replacement plan applies these lessons without changing Kindling into a direct-control colony game.
Real people retain their own choices; powers remain indirect; discoveries still need causes.
The proposed small initial world cuts breadth, not the central premise.

## Stop, keep and discard

**Stop now:** bulk art commissioning, full planetary construction before people, speculative new abstractions, another renderer comparison campaign, and detailed plans for systems with no near-term game consumer.
Stop interpreting a bigger specification as lower risk.

**Keep:** deterministic state and keyed chance; existing ECS/activity boundaries; catalogues; save/recovery and proof tests; the 2D projection and useful streaming work; signed designs and provenance; phone reports; generic materials and independent minds as the game's identity.
Keep old demos as cheap regression fixtures behind the developer menu.

**Discard as production policy:** game-last sequencing, total rewrites as a response to dissatisfaction, fixed six-hour promises, hundreds of assets as a prerequisite, and ID coverage as a completion claim.
Retire obsolete runtime paths only after checking active dependencies.
Do not mass-delete sources, art or tests to make the folder feel simpler.

The new plan's M2 is a real camp plus a place dream. M3 is stone/fire discovery and learning with an idea dream.
M4–M6 bring seasons, a neighbour and a small finish.
M7–M10 are explicitly unscheduled expansion scope, to be split again before work starts; they are not four giant new commitments.
If M3 is dull, repair or reduce it before buying more scope with the owner's attention.

## Proposals awaiting confirmation

PROJECT.md contains **22 added Proposed change lines** and a refreshed generated proposals list.
No existing decided item wording or status was changed; this was checked with the repository's own decided-word comparison.
The following is the complete list of changed items and reasons.
These are recommendations, not accepted changes.

| ID | Proposed change and reason |
|---|---|
| `PRN-09` | Play in the next milestone; technical demonstrations do not prove the game. |
| `SCP-03` | Estimate visible increments from evidence; remove the blanket few-hours expectation. |
| `SCP-16` | Camp and slice first, small valley finish before optional expansion; reduce the initial product. |
| `MIL-09` | M2 becomes the living camp and place dream; reuse graphics. |
| `MIL-10` | M3 becomes the discovery slice; defer full world generation. |
| `MIL-11` | M4 adds seasonal pressure, care and lives in small increments. |
| `MIL-12` | M5 connects two camps; prove social value before scale. |
| `MIL-13` | M6 finishes the small game; the full specification stays open. |
| `MIL-14` | M7 holds wider-world expansion, to split before scheduling. |
| `MIL-15` | M8 holds remaining lives/crafts/society, one useful chain at a time. |
| `MIL-16` | M9 improves an existing game's presentation and powers; gameplay no longer waits here. |
| `MIL-17` | Copper becomes a later choice; preserve full arc gates if commissioned. |
| `WLD-07` | Drop the universal eclipse-count guarantee; retain a coherent sky without a costly quota proof. |
| `PRE-31` | Accept readable stand-ins for early gameplay deliveries; replace the blanket art gate for those builds only. |
| `PRE-33` | Remove rotation to match the decided fixed camera. |
| `PRE-37` | Patterns for the first finish; writer optional after measured value. |
| `PRE-46` | Produce the next scene's art rather than the whole catalogue; retain approved sources. |
| `PLT-04` | Rebind scale tests to real consumers and available devices; preserve numerical targets unless separately approved. |
| `RES-07` | Rebind pace-test stage triggers; retain the full promised populations, durations and pass rules. |
| `PRC-04` | Allow short durable decision/evidence records; stop losing convenient access to lessons. |
| `PRC-06` | One accountable integrated builder and bounded support; remove the coordinator-only relay restriction. |
| `PRC-12` | Distinguish traceability from acceptance; permit partial work under stable IDs, with remaining checks explicit. This also proposes changing the staged-ID rule in How this file works. |

ARCHITECTURE.md and CLAUDE.md were left unchanged because the new order and role policy are not yet approved.
On acceptance, update their stage references and next-step instructions before implementation, including the test triggers in `PLT-04` and `RES-07`.
The old plan and its numerical gates remain available at `8928aca`; replacing the plan does not silently waive them.
Do not begin another design rewrite while this choice is still pending.

Validation of these document changes: the repository document check passes; every cited requirement ID is live; decided item wording is unchanged; coverage reports exactly the same 15 pre-existing annotation failures and no newly unmapped features.
An independent factual review found no blockers; its carry-forward clarification is included in the new plan.
Temporary native/Godot build outputs were removed after retaining logs and captures; shared caches were left intact.
