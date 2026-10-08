# Kindling: build a living camp first

**Proposed replacement, 8 October 2026.** This plan awaits the owner's decision on the proposals in PROJECT.md and CRITIQUE.md.
It is not permission to change decided requirements, weaken a passed test or resume stopped work.
The earlier plan is preserved at commit `8928aca`; its approved acceptance criteria remain obligations until explicitly changed.
M1 stays accepted. Existing M2 work is reused, including unfinished work after repair; production does not restart.

The next result is a small camp you can watch, understand, influence and reopen on your phone.
People need food and water, choose their own actions, remember useful places, and react to an indirect dream.
Then someone discovers something, another person learns it, and the camp changes.
Everything else earns its place by improving that experience.

## The route

| Stage | What you can do at its end | Decision it answers |
|---|---|---|
| M2: living camp | Follow a person through a day; send a place dream; save and return | Can I understand and influence autonomous people? |
| M3: discovery slice | Watch stone and fire discoveries, learning and consequences | Is this worth watching and playing again? |
| M4: a changing camp | Live through scarcity, recovery and a new generation | Does time make the camp more interesting? |
| M5: a neighbour | Follow knowledge and relationships between two camps | Does society add readable choices? |
| M6: a small complete game | Start unaided, play a short arc, revisit its history | Is there a satisfying game worth expanding? |
| M7–M10: conditional expansion | Add the wider world, society, presentation and copper in small releases | Which addition improves the game already in hand? |

Only M2 is ready for detailed scheduling after approval.
M3 below describes the next experiment, not a parallel work lane; confirm it from M2's results.
M4 onward is a map of remaining scope, not a promise of dates or permission to start.
Each expansion must be split into visible deliveries before it starts.
Do not replace six oversized system milestones with six oversized mixed-system milestones.

**First camp scope:** one bounded, seeded test valley, roughly 25 adults, known drinking water, a few edible plants, loose stones, wood and natural shelter.
Use normal coordinates, catalogues, saves and simulation rules; initial facts may be supplied by a labelled test scene (`RES-21`).
No generated planet, complete ecosystem, reproduction, illnesses or final art is needed for this first camp.
Do not advertise it as the full New World promised by `WLD-08`–`WLD-11` or claim the starting-population checks have passed.
Never script a discovery, a successful dream, a death or a rescue to make the demonstration work.
Simple sprites may stand for real records; decorative people may not stand for missing behaviour.

**First finished game scope:** the valley and two camps, the stone-to-fire-and-shelter arc, a small history, a few meaningful nature powers and enough seasons/lives to show consequences.
The global world, complete cultures, copper, bulk catalogues and optional prose writer are excluded from this first release proposal.
They remain unfulfilled full-project requirements unless the owner retires them; M6 is not completion of the existing full specification.

## How the work proceeds

Use one builder responsible for the running game, its tests and its delivery.
Delegate a bounded art or research task only when the builder has a concrete need and a stable interface.
Keep at most one gameplay change and one supporting task in flight.
The coordinator owns integration and the next visible outcome; forwarding instructions is not production management.
Changing the current role restriction in CLAUDE.md needs the approval recorded with `PRC-06`.

Keep the engine, renderer, camera and save approach stable through M3.
A measured blocker may justify a local change; dissatisfaction with unfinished art does not justify another renderer restart.
Write only the architecture needed for the next running behaviour.
Before M2 begins, reconcile A9–A12's outlines with this small camp, and record the new milestone crosswalk; do not rewrite all of A7 now.
A3's deterministic state and save boundaries remain in force.

Aim for one observable change per delivery, sized for a working session.
The first session produces a running increment; if the remaining work is larger, split it and retain the working increment.
Record actual time and blockers, then estimate the next increment from evidence.
There is no credible six-hour guarantee for a new mind, a renderer or a planetary generator (`SCP-03`, proposed).
Reuse begun alpha/task IDs only for their original work; new work begins at α2.13a and α3.13a, beyond the previous reservations.
Version codes still follow A2.3; no previously distributed code is reused.

## Rules every alpha keeps

Keep all existing principles: `PRN-01`, `PRN-02`, `PRN-03`, `PRN-04`, `PRN-05`, `PRN-06`, `PRN-07`, `PRN-09`, `PRN-10`, `PRN-11`, `PRN-12`, `PRN-13`, `PRN-14`, `PRN-15`, `PRN-16`, `PRN-17`.
The proposed order changes when they are exercised, not causal truth, independent minds, general rules or view-independent outcomes.
Keep scope rules `SCP-02`, `SCP-03`, `SCP-04`, `SCP-05`, `SCP-06`, `SCP-07`, `SCP-08`, `SCP-09`, `SCP-10`, `SCP-11`, `SCP-12`, `SCP-15`, `SCP-17`, `SCP-18`, `SCP-19`, `SCP-20`, `SCP-21`, subject only to explicitly approved first-release changes.
Keep `PRC-02`, `PRC-03`, `PRC-04`, `PRC-06`, `PRC-07`, `PRC-09`, `PRC-10`, `PRC-11`, `PRC-12`, `RES-01`, `RES-09`, `RES-13`, `RES-18`, `RES-19`, `RES-22`.

A **Serves** line means work on an item, not completion of everything that item promises.
For each delivery, name the actual behaviour proved and what remains open in the existing delivery note.
The proposal under `PRC-12` also changes the current rule requiring separate IDs for every staged part: permit incremental work under one stable ID, with full acceptance still open.
Until that proposal is accepted, split the relevant items with the owner before implementing them in different stages.
Never call an ID finished because the comment checker found its name.
The later milestone mappings below preserve outstanding obligations; they are not retroactive passes.

Full acceptance has an explicit later home for every unfinished clause of an early item:

| Unfinished part of an item served in M2–M6 | Stage responsible for its remaining acceptance |
|---|---|
| Bodies, minds, materials, craft rules and society (`BIO`, `MND`, `MAT`, `RCK`, `CUL`) | M8, except the full launch catalogue, farming/copper and whole-arc targets already assigned to M10 |
| Geography, weather, plants and ecology (`WLD`) | M7; individual/herd animals and domestication remain M8 |
| Presentation, sounds and powers (`PRE`, `SND`, `GOD`) | M9 |
| Full-scale time, platform and pace targets (`TIM`, `PLT`, `RES`) | M10; regression checks still run whenever an earlier change requires them |

These carry-forward assignments include IDs already listed under earlier **Serves** lines, even when the later line does not repeat them.
A delivery note must name the specific unfinished clauses and their destination, not merely repeat this table.
No small milestone may close a full item by pointing at an unimplemented future stage.

For every delivery:

1. Run touched tests while building. Preserve the existing deterministic, corruption/recovery and old-save regressions. Prove new behaviour with outcomes and a relevant planted fault, not assertions copied from the implementation.
2. Run `tools/check.sh --deliver` before delivering. Keep the approved routine/audit split (`PRC-10`); foundation changes trigger the relevant audit and milestone closes require it. Do not bypass known failures to obtain a green result.
3. Open the build, use normal navigation, inspect changed views in portrait and landscape, and check the action named under **On the phone**. Headless tests alone missed a real graphics entry-path fault in the reviewed baseline.
4. Keep save compatibility or explain the explicitly approved big update (`PLT-09`). Compare viewed/unviewed, uninterrupted/reopened and one/four-worker outcomes for new authoritative state (`RES-05`, `WLD-13`).
5. Provide the installable build and short note: what changed, three things to try, rough edges, measured results and partial IDs. No routine owner approval is needed between stages (`PRC-11`).
6. At stage close, obtain one independent review and the owner's play review. Report available phone measurements honestly; a cloud render proves neither phone speed nor touch usability (`PLT-04`).

The routine check currently fails on stopped work at `8928aca`; repair and rerun before the next delivery.
See CRITIQUE.md for the exact baseline failures and limits of this review.
No phone performance or APK acceptance is claimed by this document change.

**Review size:** ask the owner to try a five-to-ten-minute route and answer at most three questions about what they saw.
Long automatic measurements run separately and return a short report; do not hide a two-hour benchmark inside “try this build”.
Art review covers the next scene in motion, with its essential sprites, not hundreds of unrelated reference sheets.
Each stage closes on both reliable behaviour and an understandable experience.
Passing tests does not prove the latter.

## M2 A living camp

**Goal:** reuse the foundations to put autonomous people and one indirect power on the phone.
Repair only the unfinished graphics work needed by this camp, then connect actual state to it.
This replaces the graphics-only goal of `MIL-09`, subject to approval.

**Serves:** `BIO-02`, `BIO-03`, `BIO-09`, `BIO-18`, `BIO-21`, `MAT-01`, `MAT-02`, `MAT-06`, `MAT-09`, `MAT-12`, `MND-03`, `MND-07`, `MND-09`, `MND-12`, `MND-14`, `MND-15`, `MND-18`, `MND-28`, `GOD-03`, `GOD-05`, `GOD-06`, `GOD-07`, `GOD-08`, `GOD-09`, `GOD-10`, `GOD-11`, `PRE-01`, `PRE-02`, `PRE-03`, `PRE-14`, `PRE-22`, `PRE-27`, `PRE-28`, `PRE-31`, `PRE-32`, `PRE-33`, `PRE-35`, `PRE-37`, `PRE-40`, `PRE-42`, `PRE-43`, `PRE-44`, `PLT-02`, `PLT-04`, `PLT-07`, `PLT-08`, `PLT-09`, `TIM-04`, `TIM-08`, `TIM-16`, `TIM-17`, `TIM-18`, `WLD-13`, `RES-05`, `RES-06`, `RES-21`, `RES-22`, `RES-23`, `PRC-10`, `PRC-12`.

**You will see:** a camp instead of the Check page; people you can follow and inspect, food and water use, sleep, pause/speed, a place dream and the same camp after reopening.
Diagnostics remain available behind a developer menu.

**Risks:** the generic Activity and save systems have not carried real needs or knowledge yet; the command frontier can run ahead of the displayed moment; a static small valley must not masquerade as finished world generation.

### α2.13a Put real people in one saved camp

**Goal:** establish the running game scene on repaired foundations, with stable person identities rather than demo markers.

**Serves:** `BIO-03`, `MAT-01`, `MAT-02`, `PRE-01`, `PRE-02`, `PRE-22`, `PRE-27`, `PRE-31`, `PRE-32`, `PRE-35`, `PRE-40`, `PRE-42`, `PLT-07`, `PLT-08`, `PLT-09`, `TIM-04`, `TIM-08`, `TIM-16`, `WLD-13`, `RES-05`, `RES-21`, `PRC-10`, `PRC-12`.

**Architecture:** A3.2, A3.6–A3.8, A4, A9–A11, A15, A17.
Extend the existing registry, catalogue and snapshot path; do not create a second simulator or save format.

**Tasks:**

1. `T2.13a.1` Restore the baseline (`PRC-10`, `PRC-12`, `PRE-31`): fix formatting/lints, missing test links, the two failing app cases, reproducible import/UID issues and direct Examples startup. Inspect and repair the handover's still-open texture detach/lifetime paths before relying on their memory limits. Re-run the unmodified routine check; preserve assertions unless evidence shows they assert obsolete behaviour and the changed requirement is approved.
2. `T2.13a.2` Add a small Person record and camp scene (`BIO-03`, `MAT-01`, `MAT-02`, `RES-21`): stable identities, positions, initial resource quantities and a local traversable patch. Label the bounded scene as Camp alpha. Draw those records with existing projection and simple readable sprites; do not implement needs prematurely in rendering code (`WLD-13`).
3. `T2.13a.3` Make it the front door (`PRE-32`, `PRE-35`, `PRE-40`, `TIM-04`, `PLT-07`, `PLT-08`, `PLT-09`): select a person, show truthful identity/position, pause/speed, save, reopen, switch and export the actual camp. Keep the old marker world as a regression fixture. Idle or walking people are explicitly not yet a survival simulation.

**Tests:** routine checks pass from a clean import; normal navigation and direct entry both draw; selecting a sprite resolves its actual record; pause holds state; camera motion changes no outcome; reopen/export/import preserves identity, resource counts and digest; old demo saves still open.

**On the phone:** open Camp alpha, select two people, change speed, close and reopen, then export and import that camp.
This delivery proves the integrated scene; it does not yet prove the central game.

### α2.13b Give the camp a real daily life

**Goal:** hunger, thirst and fatigue cause visible choices and consequences.

**Serves:** `BIO-02`, `BIO-09`, `BIO-18`, `BIO-21`, `MAT-01`, `MAT-06`, `MAT-09`, `MAT-12`, `MND-03`, `MND-07`, `MND-09`, `MND-14`, `MND-15`, `MND-18`, `MND-28`, `PRE-14`, `PRE-27`, `PRE-28`, `PRE-35`, `PRE-37`, `PRE-43`, `PRE-44`, `TIM-17`, `TIM-18`, `RES-05`, `RES-23`.

**Architecture:** A3.3, A3.5–A3.8, A9–A12, A15.
Write the small needs/action-selection contract before its code; preserve general choice rules and data-defined affordances.

**Tasks:**

1. `T2.13b.1` Implement need-driven eating, drinking and resting (`BIO-09`, `BIO-21`, `MND-07`, `MND-09`, `MAT-06`, `TIM-17`). Walking, gathering and carrying take time; input quantities and effects apply exactly once, including interruptions. No hidden food refill or predetermined day script.
2. `T2.13b.2` Let senses find nearby supplies and memory retain them (`BIO-18`, `MND-03`, `MND-18`, `MND-28`). Use actual local obstacles and reachable destinations. Begin with the agreed starting skills (`BIO-02`); do not grant unknown crafts to solve hunger.
3. `T2.13b.3` Show the choice and its cause (`PRE-14`, `PRE-35`, `PRE-37`, `MND-09`). The card reads the recorded decision; simple movement/work/rest poses show the current Activity (`PRE-27`, `PRE-44`). Add enough real resource renewal for the scoped camp, with documented inputs and bounds (`MAT-09`).

**Tests:** absent food remains absent; consumed food cannot be eaten twice; a thirstier otherwise identical person favours drinking; interrupted actions cannot duplicate results; unreachable water leads to a new choice or an honest failure, not teleportation. Save mid-action and compare continuation. Run fixed initial camp seeds through several days; report failures and tuning without calling this a full ecology or health pass.

**On the phone:** follow one person from thirst through walking, drinking and recovery; read why another rests; pause mid-action and return later.
If the owner cannot explain these actions after looking at their cards, fix readability before adding more needs.

### α2.13c Make the first indirect choice

**Goal:** the camp becomes playable through a place dream that influences a person's own choices.

**Serves:** `GOD-03`, `GOD-05`, `GOD-06`, `GOD-07`, `GOD-08`, `GOD-09`, `GOD-10`, `GOD-11`, `MND-09`, `MND-12`, `MND-18`, `MND-28`, `PRE-03`, `PRE-31`, `PRE-33`, `PRE-35`, `PLT-02`, `PLT-04`, `PLT-07`, `TIM-04`, `RES-05`, `RES-06`, `RES-22`.

**Architecture:** A3.8, A8.6, A11, A15, A18.
Resolve displayed time versus the simulation command frontier before accepting a power.

**Tasks:**

1. `T2.13c.1` Implement one complete place-dream route (`GOD-03`, `GOD-10`, `GOD-11`): known subjects only, queued sleep, vanished-subject cancellation, one per sleeper/night and three total/night, expiring influence and no stacking. Reuse the natural dream/choice path (`MND-12`, `MND-09`); no move command.
2. `T2.13c.2` Record the request and execution time (`GOD-07`, `GOD-08`, `GOD-09`, `RES-05`). Keep player attribution out of personal memory (`GOD-06`). Cap/drain queued work safely so a power chosen at the displayed moment cannot silently act hours later; test at the fastest supported rate.
3. `T2.13c.3` Deliver and review the camp (`PRE-31`, `PLT-02`, `PLT-04`, `RES-06`, `RES-22`): usable touch controls in both orientations, save/reopen pending dreams, a short owner route and a separate sustained phone measurement. Use one art pass only if identity, action or target is unreadable.

**Tests:** in paired seeded runs, a valid dream can tip a close choice; urgent thirst still wins. Invalid/unknown subjects are refused; daily caps and cancellation survive reopening; no memory can identify the player. Record command latency at supported speeds. Independent review checks the whole M2 route, existing save/determinism regressions and the measured phone results.
The idea, animal, person and fear dream acceptance checks remain open.

**On the phone:** inspect someone's known places, send a dream, watch them wake and decide, and find the consequence in their card and your record.
Stage questions: Did their decision make sense? Did the dream matter without commanding them? Was it comfortable to use?

## M3 A discovery that changes the camp

**Goal:** prove Kindling's central loop in a small vertical slice: ordinary action, unexpected result, remembered knowledge, another learner and a useful consequence.
Proposed replacement for `MIL-10`; confirm these steps after M2.

**Serves:** `MAT-03`, `MAT-04`, `MAT-07`, `MAT-18`, `MAT-21`, `MAT-22`, `MND-04`, `MND-06`, `MND-10`, `MND-11`, `MND-13`, `MND-23`, `CUL-01`, `CUL-02`, `CUL-03`, `CUL-16`, `RCK-01`, `RCK-02`, `RCK-03`, `RCK-06`, `RCK-07`, `RCK-10`, `RCK-11`, `RCK-12`, `RCK-13`, `RCK-25`, `BIO-20`, `BIO-23`, `GOD-03`, `GOD-05`, `GOD-06`, `PRE-05`, `PRE-08`, `PRE-13`, `PRE-14`, `PRE-17`, `PRE-31`, `PRE-35`, `PRE-37`, `RES-02`, `RES-03`, `RES-05`, `RES-06`, `RES-10`, `RES-16`, `RES-17`, `RES-22`, `RES-23`, `RES-24`, `RES-25`, `MOM-01`, `MOM-02`, `MOM-09`, `SND-01`, `SND-12`, `PLT-04`.

**You will see:** flakes used, discoveries with a cause and a knower, learning through observation, an idea dream that prompts attempts, and fire helping people.
A short history links back to real people, objects and events.

**Risks:** discovery may be too rare to notice or so automatic that it feels staged; generic rules may be quietly replaced by special cases; fire needs real fuel and consequences to matter.
Stop and tune the small loop if it fails, rather than build the next subsystem.

### α3.13a Someone finds a sharp edge

**Goal:** the first real discovery and its transfer to another person.

**Serves:** `MAT-03`, `MAT-04`, `MAT-07`, `MAT-21`, `MAT-22`, `MND-04`, `MND-06`, `MND-10`, `MND-11`, `MND-13`, `MND-23`, `CUL-01`, `CUL-02`, `CUL-03`, `CUL-16`, `RCK-01`, `RCK-02`, `RCK-11`, `PRE-05`, `PRE-08`, `PRE-13`, `PRE-14`, `PRE-17`, `PRE-35`, `PRE-37`, `RES-02`, `RES-03`, `RES-05`, `RES-10`, `RES-23`, `RES-24`, `MOM-01`.

**Architecture:** A3.6, A11–A12, A14–A15.

**Tasks:**

1. `T3.13a.1` Build the smallest generic action/characteristic/blueprint evaluator (`MAT-03`, `MAT-04`, `MAT-07`, `RCK-01`, `RCK-02`, `RES-24`). Trial one flake chain with valid and invalid materials; record consumption, failure, result and the actual observer. No “if flint, discover” branch in the mind.
2. `T3.13a.2` Connect surprise, knowledge and useful repetition (`MND-04`, `MND-10`, `MND-11`, `MAT-21`). A known sharp edge must improve a real supported task; knowledge belongs to individuals. Add observation/teaching sufficient for a second knower (`MND-13`, `CUL-01`, `CUL-02`).
3. `T3.13a.3` Surface the event in a plain history and card (`PRE-05`, `PRE-08`, `PRE-17`, `MND-23`). Record who saw what and learned from whom; no invented motive or universal knowledge. Run the sharp-stone and disabled-learning controls (`RES-02`, `RES-03`, `RES-10`).

**Tests:** blueprint inputs/outputs obey their rules; unobserved discovery grants nobody knowledge; copying requires access and prerequisites; save mid-attempt preserves its outcome. Run the existing promised sharp-stone criteria with their stated populations/seeds or explicitly report which remain unmet by this camp. A chosen demonstration seed is never evidence of natural frequency.

**On the phone:** follow a discoverer, inspect the useful object, find its discovery entry, then find a second person who learned from them.

### α3.13b A dream leads to attempts at fire

**Goal:** the player's idea hint feeds the same discovery system and produces a meaningful survival benefit.

**Serves:** `MAT-04`, `MAT-07`, `MAT-18`, `MND-06`, `MND-11`, `MND-13`, `RCK-03`, `RCK-06`, `RCK-07`, `RCK-10`, `RCK-12`, `RCK-13`, `RCK-25`, `BIO-20`, `BIO-23`, `GOD-03`, `GOD-05`, `GOD-06`, `PRE-05`, `PRE-14`, `RES-03`, `RES-17`, `RES-23`, `RES-24`, `MOM-02`, `MOM-09`.

**Architecture:** A9–A12, A14–A15.

**Tasks:**

1. `T3.13b.1` Give combustion actual fuel, wetness and heat (`MAT-18`, `RCK-03`, `RCK-07`, `RCK-13`). Add the minimal warmth and cooking consumers needed to make it useful (`BIO-23`, `RCK-06`, `RCK-10`). A flame animation alone cannot close this step.
2. `T3.13b.2` Add the idea dream using handled inputs, known actions and a real prior memory (`GOD-03`, `MND-11`). Reuse M2's limits and queues. A hint produces a hunch and attempts, never a completed blueprint or guaranteed success.
3. `T3.13b.3` Exercise the fire chain and explain failure (`RES-23`, `RES-24`, `PRE-14`, `PRE-05`). Use the dream-attempt scene's approved 15-of-20 rule for attempts within a few days; success, natural discovery and useful fire are separate observations. Test the approved multiple routes as they become available; unbuilt routes remain open.

**Tests:** wet/unsuitable fuel can fail; fire exhausts its fuel; warmth/cooking results follow actual exposure; knowledge survives in learners and disappears with its last holder. Dream attempts respect urgent needs and saved limits. Compare natural and intervened runs without inserting successes or changing hidden chances to hit a story beat.

**On the phone:** select an eligible memory, send the idea dream, see an attempt and its result, then follow how a working fire changes someone's next actions.

### α3.13c Make the slice worth returning to

**Goal:** a readable, stable ten-to-fifteen-minute experience that survives an independent playtest.

**Serves:** `PRE-05`, `PRE-08`, `PRE-31`, `PRE-35`, `PRE-37`, `RES-05`, `RES-06`, `RES-16`, `RES-17`, `RES-22`, `RES-25`, `SND-01`, `SND-12`, `PLT-04`.

**Architecture:** A4–A6, A14–A18.

**Tasks:**

1. `T3.13c.1` Repair only the slice's readability and pacing (`PRE-31`, `PRE-35`, `RES-16`). Add a few useful world sounds (`SND-01`, `SND-12`), clear failure feedback, usable speed controls and links from events to their subjects. Upgrade stand-ins only where they obstruct understanding.
2. `T3.13c.2` Reproduce the whole loop on fixed and fresh seeds (`RES-05`, `RES-17`, `RES-25`), preserving failed runs. Compare survival, attempts, learning and waiting time; use natural and intervention runs separately. Keep the full later pace promises open.
3. `T3.13c.3` Deliver the independent review and owner route (`RES-06`, `RES-22`, `PLT-04`). Run the due audit and sustained phone test separately. Write M4's next small steps only after the play result; record what the owner wanted to follow again.

**Tests:** reopen during fire, learning and a queued dream; viewed/unviewed and worker-count outcomes agree. No unexplained stuck person, false history or lost world in the review route. Phone frame/memory/heat results and all due checks are present as passes or failures, never inferred from screenshots.
Technical acceptance and interest are separate: the owner can explain a discovery and a power's consequence without the builder narrating, and wants to watch another outcome.
If not, use one bounded repair cycle on this slice, then cut/reconsider the loop before expanding it.

**On the phone:** start a fresh camp unaided, follow someone, make one intervention, observe discovery and learning, save, reopen and read the short history.
Stage questions: What held your attention? What was confusing? What would you choose to do next?

## M4 The camp survives change

**Goal:** Add one pressure at a time to the existing camp: seasonal food first, then a wound and care, then births, ageing and death. Proposed replacement for MIL-11. These are bounded first versions; full disease, inheritance, ecology and population checks remain open until their complete scope is built.

**Serves:** `BIO-04`, `BIO-05`, `BIO-06`, `BIO-08`, `BIO-10`, `BIO-11`, `BIO-12`, `BIO-13`, `BIO-14`, `BIO-15`, `BIO-16`, `BIO-17`, `BIO-22`, `MAT-08`, `MAT-10`, `MAT-11`, `MAT-19`, `MAT-20`, `MOM-07`, `PRE-30`, `RCK-14`, `RCK-15`, `RCK-21`, `RCK-22`, `RES-14`, `TIM-09`, `WLD-18`, `WLD-27`, `WLD-28`, `WLD-31`.

**You will see:** Food becomes scarce, someone changes a plan, help matters, and a child eventually inherits knowledge from an older person. The same saved camp continues.

**Risks:** Too many interacting rates can hide the cause of failure. Do not add all illnesses, species or body detail together.

Three candidate deliveries: a lean season with real food regrowth; one visible wound/care chain; one family through birth, learning and death.
Plan each from the last playable build, with paired controls and saved traces (`RES-14`, `RES-23`).
Close on a readable season and family story, not a claim that the full 15-illness catalogue is complete.
Carry unbuilt biological, physical and ecological acceptance checks forward explicitly; broaden only when an observed failure needs them.
The next plan must state exactly which part of each served ID this stage tests.

## M5 Meet one neighbouring camp

**Goal:** Add relationships, communication and movement between two camps. Proposed replacement for MIL-12. Full personality, belief and culture catalogues are not prerequisites.

**Serves:** `CUL-07`, `CUL-17`, `CUL-18`, `CUL-21`, `CUL-24`, `CUL-27`, `CUL-30`, `CUL-31`, `MND-05`, `MND-08`, `MND-19`, `MND-20`, `MND-21`, `MND-22`, `MND-24`, `MND-26`, `MND-27`, `MND-29`, `MND-30`, `MND-32`, `MND-33`, `MOM-04`, `PRE-45`, `VIS-17`.

**You will see:** A useful skill crosses between two camps; a friendship or disagreement changes who meets, helps or shares; the cards and history show why.

**Risks:** Labels called culture can appear without any useful causal behaviour. Two camps with different names are not evidence of different societies.

Candidate deliveries: recognise/remember another person; exchange something useful; carry a learned skill to the second camp.
Test observer access, individual knowledge and mutually incompatible needs; save/reopen a traveller halfway there.
Close when the owner can follow a real social consequence without a debugger.
A feud, rite, chief or religion is added later only with its own observable chain; the whole original culture milestone is not silently included here.

## M6 Finish the small game

**Goal:** Make the agreed valley game usable from first launch to a satisfying short arc, with an honest boundary around its content. Proposed replacement for MIL-13 and a first-release scope decision, not completion of Kindling's full promise.

**Serves:** `GOD-04`, `MOM-03`, `PLT-03`, `PRE-06`, `PRE-07`, `PRE-09`, `PRE-10`, `PRE-15`, `PRE-16`, `PRE-18`, `PRE-19`, `PRE-34`, `PRE-39`, `RES-12`, `RES-16`, `SND-06`, `TIM-01`, `TIM-02`, `TIM-03`, `TIM-10`, `TIM-11`, `TIM-15`, `VIS-15`.

**You will see:** A clear start, people worth following, a few nature choices, helpful moments and history, a reliable saved world and a reason to return.

**Risks:** Polish can grow without limit. Freeze the small release content before this stage and reject additions that do not repair its route.

Candidate deliveries: unaided first launch and continuation; readable important moments and speed-by-zoom with manual override; final repair of the fixed play route.
Use existing art as the base and upgrade only the sprites, sounds and controls in this game.
Measure the owner's actual device, sustained load, old-world memory and largest representative save/export; repair synchronous large-file copying before claiming bounded export latency.
An independent tester should start, learn, intervene, leave and return without instructions beyond the game.
The owner chooses whether to enjoy this version as it stands or fund one expansion. No automatic march into M7.

## M7 Expansion option: a wider living landscape

**Goal:** Preserve the outstanding geography and world requirements as an expansion option. Proposed replacement for MIL-14. This block is not ready to schedule as one milestone.

**Serves:** `PLT-04`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `PRC-09`, `PRC-11`, `PRE-02`, `PRE-03`, `PRE-23`, `PRE-24`, `PRE-25`, `PRE-26`, `PRE-29`, `PRE-30`, `PRE-31`, `PRN-04`, `PRN-10`, `PRN-14`, `PRN-15`, `PRN-16`, `RES-05`, `RES-06`, `RES-09`, `RES-12`, `RES-13`, `RES-18`, `RES-21`, `RES-22`, `SCP-11`, `TIM-14`, `TIM-16`, `TIM-18`, `WLD-01`, `WLD-02`, `WLD-03`, `WLD-06`, `WLD-07`, `WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`, `WLD-12`, `WLD-13`, `WLD-14`, `WLD-15`, `WLD-16`, `WLD-17`, `WLD-22`, `WLD-24`, `WLD-26`, `WLD-27`, `WLD-30`.

**You will see:** First, the existing people cross into a new local area using its actual food and water; later, they inhabit generated regions with weather that affects their lives.

**Risks:** Rebuilding the old 24-delivery empty-world programme would restore the original failure. Full globe, geology, eclipse and candidate guarantees remain expensive unfulfilled obligations.

Choose ONE playable goal for the next short stage: a generated neighbouring valley, a river affecting travel, or weather affecting food and shelter.
Add it to the same running people and saves before expanding the generator again.
Then evaluate region travel, climates, ecology and ultimately global generation, one visible release at a time.
A7 contains useful causal constraints and deferred approved gates; retain their meaning until specifically amended.
When global generation is actually commissioned, re-cost the original held-out world, candidate, water, eclipse and phone gates from `8928aca`; do not call them passed by a local test scene.
The complete scope needs several future stages. Re-number/replan those with owner approval rather than treating this holding block as a short implementation commitment.

## M8 Expansion option: richer lives and society

**Goal:** Hold the remaining bodies, animals, minds, crafts and cultures for separate additions to the playable camps. Proposed replacement for MIL-15. This is an obligation map, not a simultaneous build list.

**Serves:** `BIO-19`, `CUL-05`, `CUL-06`, `CUL-08`, `CUL-09`, `CUL-10`, `CUL-11`, `CUL-12`, `CUL-19`, `CUL-20`, `CUL-22`, `CUL-23`, `CUL-26`, `CUL-29`, `CUL-32`, `CUL-34`, `MND-01`, `MND-02`, `MND-16`, `MND-31`, `MOM-06`, `MOM-11`, `RCK-16`, `RCK-23`, `RCK-24`, `RCK-26`, `TIM-07`, `WLD-32`, `WLD-33`.

**You will see:** Each chosen addition changes an existing life: an animal is hunted or tamed, a craft solves a shortage, or a repeated event leads to a shared practice.

**Risks:** Catalogues can grow much faster than interesting interactions. No entry is useful merely because its source research or art sheet is complete.

Choose one small causal chain, such as clothes improving a cold season, a tame animal changing a camp, or a practice spreading between camps.
Ship and play that chain before choosing the next.
Full species, illness, emotions, social acts, religion, music and craft-route acceptance remains open and is checked when each full item is implemented.
The narrower M2–M5 versions of bodies, needs, knowledge and social behaviour return here for their remaining cases; their early tests remain regressions.
No duplicated system and no fresh foundation are justified by calling this a later phase.

## M9 Expansion option: the full presentation and powers

**Goal:** Hold the rest of the visual, interface, sound and power scope for improvements to an already playable game. Proposed replacement for MIL-16; this is no longer where gameplay first appears.

**Serves:** `GOD-02`, `GOD-03`, `GOD-10`, `GOD-11`, `GOD-12`, `GOD-13`, `PRE-20`, `PRE-21`, `PRE-31`, `PRE-37`, `PRE-41`, `PRE-46`, `SND-02`, `SND-03`, `SND-07`, `SND-08`, `SND-11`, `VIS-14`.

**You will see:** One improvement at a time: a clearer scene in motion, a useful additional power, a better sound layer or a more readable history.

**Risks:** Completing every image and every effect can absorb unlimited work; a prose writer introduces platform and factual risks without proving gameplay.

Select the next addition from a play problem or an explicit owner preference.
Complete the missing power types, full dream routes, global interfaces and art families in separate playable releases with their own checks.
Bulk art follows stable actions and useful camera distances; the sheet count is not a gate for a playable build.
Patterns remain the default text. Commission the optional writer only after a device feasibility test and a blind comparison show a worthwhile improvement; that is a proposed change to `PRE-37`, not a silent waiver.
All representation must stay grounded in actual state, including later buried history and unseen places.

## M10 Expansion option: settlements to copper

**Goal:** Keep the original long arc as an explicit later choice. Proposed revision of MIL-17: it is not required to finish the first small game.

**Serves:** `CUL-28`, `CUL-33`, `MAT-23`, `MOM-08`, `MOM-12`, `PLT-04`, `RCK-04`, `RCK-08`, `RES-07`, `RES-12`, `RES-16`, `RES-25`, `TIM-07`, `TIM-19`, `VIS-14`, `WLD-33`.

**You will see:** One useful new chain per release: pottery, tending plants, herding, a settled village, then copper, each with consequences for existing people.

**Risks:** Centuries of tuned history, thousands of people and the full launch catalogue are a much larger product than the first valley. Nothing in the present test results proves them feasible together.

After each chain, run the relevant blueprint, consumer and discovery/pace scenes, then decide the next addition.
Before the full arc closes, run the existing multi-world long-term pace and old-world performance gates in `RES-07`, `RES-12`, `RES-25`, `TIM-07`, `TIM-19`, `CUL-33` and `PLT-04` at their actual required scope.
Partial checks in earlier stages do not waive those gates.
Freeze any numerical changes with the owner before tuning; preserve failed seeds and interventions separately.
If the owner chooses to end at the smaller game, explicitly retire the excluded IDs instead of claiming the current specification is complete.

## What stops now, if this proposal is accepted

Stop bulk reference sheets, globe/cutaway polish, a second renderer campaign, detailed eclipse fitting, new engine abstractions without a camp consumer, and full endgame catalogues.
Do not discard their source files; stop assigning them to the critical path.
Keep current art provenance, source sheets, useful tests, the proof suites and old saves.
Remove obsolete runtime/export paths only after checking that the active camp and tests no longer depend on them.

A stop is not a failed project. Expanding a game nobody yet wants to watch is the avoidable failure.
At every stage the owner may choose repair, expansion, a smaller finish or a pause from an actual playable build.
