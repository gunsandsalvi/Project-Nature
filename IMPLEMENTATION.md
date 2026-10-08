# Kindling: the build plan

Adopted by the owner on 8 October 2026. M1 remains accepted; reuse its simulation, saves and current 2D work.
The next delivery is α2.13a: repair the stopped baseline and put real person records in one saved camp.
Only M2 is scheduled. Detail M3 after playing M2; M4–M10 map later obligations, not parallel work.

## Route and scope

| Stage | Playable result |
|---|---|
| M2 | Follow daily needs, send a place dream, save and return |
| M3 | Watch stone/fire discovery, learning and an idea dream change the camp |
| M4 | Follow scarcity, care and a family through time |
| M5 | Follow a skill and relationships between two camps |
| M6 | A small finished valley game: start, influence, history and reliable continuation |
| M7–M10 | Optional wider world, remaining life/society, presentation and copper; split each into small stages before starting |

The first camp is a bounded, seeded test valley with roughly 25 adults, drinking water, a few food plants, stones, wood and natural shelter.
Use existing coordinates, catalogues and saves. Supplied initial facts are labelled test-scene inputs (`RES-21`), not completed planet generation.
Outcomes, decisions and discoveries are simulated. Readable stand-ins depict real records.
The small M6 finish excludes the globe, complete cultures/catalogues, copper and the optional prose writer.
Their requirements remain open unless explicitly retired; finishing M6 does not complete the full specification.

One builder owns the running increment. Add at most one bounded supporting art or research task.
Keep the engine, renderer, camera and save approach stable through M3 unless a measured blocker needs a local change.
Build one observable change per delivery; split overruns and estimate from actual completed work.
Begin new task IDs at α2.13a; begun IDs and distributed version codes are never reused (A2.3).

## Rules every alpha keeps

Principles: `PRN-01`, `PRN-02`, `PRN-03`, `PRN-04`, `PRN-05`, `PRN-06`, `PRN-07`, `PRN-09`, `PRN-10`, `PRN-11`, `PRN-12`, `PRN-13`, `PRN-14`, `PRN-15`, `PRN-16`, `PRN-17`.
Scope: `SCP-02`, `SCP-03`, `SCP-04`, `SCP-05`, `SCP-06`, `SCP-07`, `SCP-08`, `SCP-09`, `SCP-10`, `SCP-11`, `SCP-12`, `SCP-15`, `SCP-17`, `SCP-18`, `SCP-19`, `SCP-20`, `SCP-21`.
Process/checks: `PRC-02`, `PRC-03`, `PRC-04`, `PRC-06`, `PRC-07`, `PRC-09`, `PRC-10`, `PRC-11`, `PRC-12`, `RES-01`, `RES-09`, `RES-13`, `RES-18`, `RES-19`, `RES-22`.

**Serves** means work on an item, not full completion. Keep a stable ID through partial deliveries; the note names the behaviour proved and its remaining checks.
Unfinished clauses of early items have these acceptance destinations, even where the later Serves list does not repeat them:

| Remaining scope | Destination |
|---|---|
| Bodies, minds, materials, craft rules, society | M8; full catalogue, farming/copper and arc targets M10 |
| Physical geography/weather and ecological inputs | M7; full living settling/ecology, biological starts, animals and domestication M8 |
| Presentation, sound, powers | M9 |
| Full-scale time, platform and pace targets | M10; relevant regressions still run earlier |

For each delivery:

1. Run touched tests while building. Preserve determinism, recovery and old-save regressions; prove new behaviour with outcomes and a relevant planted fault (`PRC-09`).
2. Run the routine/delivery check once before delivery. Keep its existing audit triggers; do not waive current failures (`PRC-10`).
3. Exercise normal navigation, changed views in both orientations and the stated phone route. Keep save compatibility or explain an approved big update. Compare authoritative outcomes across camera use, reopen and worker counts.
4. Ship the APK and short note: what changed, three things to try, rough edges, results and partial IDs. Only stage reviews wait for the owner.
5. At stage close, obtain independent review, due audit/phone measurements and the owner's play review. Ask at most three questions on a five-to-ten-minute route; long measurements run separately.

The baseline repair list is in A17.0. Existing check failures remain open; this document cleanup does not repair them.
CRITIQUE.md recommends further process changes, but its separate patch is not adopted.

## M2 A living camp

**Goal:** reuse the foundations to put autonomous people and one indirect power on the phone.
Repair only the unfinished graphics work needed by this camp, then connect actual state to it.
Milestone `MIL-09`.

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

1. `T2.13a.1` Restore the baseline (`PRC-10`, `PRC-12`, `PRE-31`): fix formatting/lints, missing test links, the two failing app cases, reproducible import/UID issues and direct Examples startup. Inspect and repair the texture detach/lifetime paths recorded in A17.0 before relying on their memory limits. Re-run the routine check; preserve assertions unless evidence shows they assert obsolete behaviour and the changed requirement is approved.
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

**Goal:** Prove the central loop: ordinary action, unexpected result, individual knowledge, a second learner and a useful consequence (`MIL-10`).

**Serves:** `MAT-03`, `MAT-04`, `MAT-07`, `MAT-18`, `MAT-21`, `MAT-22`, `MND-04`, `MND-06`, `MND-10`, `MND-11`, `MND-13`, `MND-23`, `CUL-01`, `CUL-02`, `CUL-03`, `CUL-16`, `RCK-01`, `RCK-02`, `RCK-03`, `RCK-06`, `RCK-07`, `RCK-10`, `RCK-11`, `RCK-12`, `RCK-13`, `RCK-25`, `BIO-20`, `BIO-23`, `GOD-03`, `GOD-05`, `GOD-06`, `PRE-05`, `PRE-08`, `PRE-13`, `PRE-14`, `PRE-17`, `PRE-31`, `PRE-35`, `PRE-37`, `RES-02`, `RES-03`, `RES-05`, `RES-06`, `RES-10`, `RES-16`, `RES-17`, `RES-22`, `RES-23`, `RES-24`, `RES-25`, `MOM-01`, `MOM-02`, `MOM-09`, `SND-01`, `SND-12`, `PLT-04`.

**You will see:** Flakes used, fire helping people, idea dreams prompting attempts and a short truthful history.

**Risks:** Discovery may stall or look scripted. Repair the small loop before expanding.

Next deliveries, to detail after M2:
- Sharp edge: one generic material/action/blueprint chain, useful repetition, observation and teaching; no named-material shortcut in the mind. History records the actual observer and source of learning.
- Fire: fuel, wetness, heat, warmth and cooking; an idea dream uses a real handled-input memory and creates a hunch, never guaranteed success. Preserve all dream limits and urgent needs.
- Slice review: improve only the play route's readability, sound and pacing; an independent tester starts, intervenes, follows learning, saves and returns without narration.

Acceptance retains valid/invalid blueprint trials, consumption without duplication, observer/prerequisite limits, save mid-attempt/fire/dream, individual knowledge loss, and viewed/unviewed/worker equivalence.
Run the sharp-stone and disabled-learning controls at their promised scope; a chosen demonstration seed proves no natural frequency.
The idea-dream scene requires attempts within a few days in at least 15 of 20 runs; fire success and natural discovery are separate results.
Preserve failed runs and full later pace obligations. Record due audit and phone results.
Close only when the owner understands a discovery and an intervention and wants another outcome; otherwise use one bounded repair cycle, then reconsider scope.

## M4 The camp survives change

**Goal:** Add seasonal pressure, care and a family through time, separately (`MIL-11`).

**Serves:** `BIO-04`, `BIO-05`, `BIO-06`, `BIO-08`, `BIO-10`, `BIO-11`, `BIO-12`, `BIO-13`, `BIO-14`, `BIO-15`, `BIO-16`, `BIO-17`, `BIO-22`, `MAT-08`, `MAT-10`, `MAT-11`, `MAT-19`, `MAT-20`, `MOM-07`, `PRE-30`, `RCK-14`, `RCK-15`, `RCK-21`, `RCK-22`, `RES-14`, `TIM-09`, `WLD-18`, `WLD-27`, `WLD-28`, `WLD-31`.

**You will see:** A lean season changes choices, help matters, and a younger person learns from an older one.

**Risks:** Many new rates can hide why a camp fails.

Candidate increments: seasonal food/regrowth; one wound/care chain; births, learning, ageing and death.
Use paired controls and saved traces. Close on a readable season and family story.
Unbuilt illness, inheritance, ecology and population checks remain open; the next detailed plan names each tested subset.

## M5 Meet one neighbouring camp

**Goal:** Add relationships, communication and travel between two camps (`MIL-12`).

**Serves:** `CUL-07`, `CUL-17`, `CUL-18`, `CUL-21`, `CUL-24`, `CUL-27`, `CUL-30`, `CUL-31`, `MND-05`, `MND-08`, `MND-19`, `MND-20`, `MND-21`, `MND-22`, `MND-24`, `MND-26`, `MND-27`, `MND-29`, `MND-30`, `MND-32`, `MND-33`, `MOM-04`, `PRE-45`, `VIS-17`.

**You will see:** A skill crosses camps; friendship or disagreement changes meeting, helping or sharing.

**Risks:** Names and labels can suggest society without causal behaviour.

Candidate increments: recognise and remember someone; exchange something useful; carry knowledge to the second camp.
Test observer access, individual knowledge, competing needs and save/reopen during travel.
Close on a social consequence the owner can follow. Full personality, belief and culture catalogues remain later work.

## M6 Finish the small game

**Goal:** Finish the agreed valley game from first launch to a satisfying short arc (`MIL-13`).

**Serves:** `GOD-04`, `MOM-03`, `PLT-03`, `PRE-06`, `PRE-07`, `PRE-09`, `PRE-10`, `PRE-15`, `PRE-16`, `PRE-18`, `PRE-19`, `PRE-34`, `PRE-39`, `RES-12`, `RES-16`, `SND-06`, `TIM-01`, `TIM-02`, `TIM-03`, `TIM-10`, `TIM-11`, `TIM-15`, `VIS-15`.

**You will see:** A clear start, useful nature choices, readable moments/history and reliable continuation.

**Risks:** Polish can grow without limit; freeze release content first.

Candidate increments: unaided start/continuation; moments and speed-by-zoom with manual override; final route repairs.
Upgrade only assets and controls this game uses. Measure sustained load, old-world memory and representative large saves/exports on the owner phone.
Fix whole-file export blocking before claiming bounded latency. An independent tester must start, learn, intervene, leave and return.
The owner then chooses to keep this small game or commission an expansion; M7 is not automatic.

## M7 Expansion option: a wider living landscape

**Goal:** Hold outstanding geography and world scope until commissioned (`MIL-14`); split it before scheduling.

**Serves:** `PLT-04`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `PRC-09`, `PRC-11`, `PRE-02`, `PRE-03`, `PRE-23`, `PRE-24`, `PRE-25`, `PRE-26`, `PRE-29`, `PRE-30`, `PRE-31`, `PRN-04`, `PRN-10`, `PRN-14`, `PRN-15`, `PRN-16`, `RES-05`, `RES-06`, `RES-09`, `RES-12`, `RES-13`, `RES-18`, `RES-21`, `RES-22`, `SCP-11`, `TIM-14`, `TIM-16`, `TIM-18`, `WLD-01`, `WLD-02`, `WLD-03`, `WLD-06`, `WLD-07`, `WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`, `WLD-12`, `WLD-13`, `WLD-14`, `WLD-15`, `WLD-16`, `WLD-17`, `WLD-22`, `WLD-24`, `WLD-26`, `WLD-27`, `WLD-30`.

**You will see:** Existing people use a new valley, cross a river or cope with meaningful weather.

**Risks:** The former empty-world programme must not return as one prerequisite.

Choose one playable addition, integrate it with people and saves, then decide the next.
A7 and A17.1 retain deferred design/acceptance obligations. Global candidate, water, climate and phone guarantees require their actual scope, not a local scene. Full living settling and biological start/survival acceptance follow their complete M8 consumers.
The universal eclipse quota is removed by the approved WLD-07 change. Remaining targets cannot be loosened without approval.

## M8 Expansion option: richer lives and society

**Goal:** Hold remaining bodies, animals, minds, crafts and cultures for separate additions (`MIL-15`).

**Serves:** `BIO-19`, `CUL-05`, `CUL-06`, `CUL-08`, `CUL-09`, `CUL-10`, `CUL-11`, `CUL-12`, `CUL-19`, `CUL-20`, `CUL-22`, `CUL-23`, `CUL-26`, `CUL-29`, `CUL-32`, `CUL-34`, `MND-01`, `MND-02`, `MND-16`, `MND-31`, `MOM-06`, `MOM-11`, `RCK-16`, `RCK-23`, `RCK-24`, `RCK-26`, `TIM-07`, `WLD-32`, `WLD-33`.

**You will see:** One chain changes existing lives: clothing helps in cold, an animal becomes tame or a practice spreads.

**Risks:** Catalogue size is not evidence of interesting interactions.

Ship and play one useful chain before choosing the next. Early body/mind/material/social IDs return here for their unfinished clauses, preserving their regressions.
Full species, illness, emotion, social-action, religion, music and craft-route acceptance stays open until implemented.

## M9 Expansion option: presentation and powers

**Goal:** Improve an already playable game with remaining visuals, sound, interface and powers (`MIL-16`).

**Serves:** `GOD-02`, `GOD-03`, `GOD-10`, `GOD-11`, `GOD-12`, `GOD-13`, `PRE-20`, `PRE-21`, `PRE-31`, `PRE-37`, `PRE-41`, `PRE-46`, `SND-02`, `SND-03`, `SND-07`, `SND-08`, `SND-11`, `VIS-14`.

**You will see:** A clearer scene, useful new power, sound layer or history improvement.

**Risks:** Effects and assets can grow without limit.

Choose each addition from a play problem or owner preference. Complete missing power/dream routes, global interfaces and art families in separate releases.
Bulk art follows stable actions and camera distances. Patterns remain default text; the optional writer needs a device feasibility test and a comparison showing improvement.
All pictures and words remain grounded in actual state.

## M10 Expansion option: settlements to copper

**Goal:** Keep the long arc as a later choice (`MIL-17`), not a condition of the small finish.

**Serves:** `CUL-28`, `CUL-33`, `MAT-23`, `MOM-08`, `MOM-12`, `PLT-04`, `RCK-04`, `RCK-08`, `RES-07`, `RES-12`, `RES-16`, `RES-25`, `TIM-07`, `TIM-19`, `VIS-14`, `WLD-33`.

**You will see:** Pottery, tending plants, herding, settlement and copper, one useful chain at a time.

**Risks:** Centuries, thousands of people and the full catalogue are a larger product than the valley.

Each chain gets its blueprint, consumer and discovery/pace checks. Full closure requires the promised long-term multi-world and old-world performance gates in RES-07, RES-12, RES-25, TIM-07, TIM-19, CUL-33 and PLT-04.
Freeze numerical changes before tuning; retain failed seeds and distinguish interventions.
If the owner chooses to end at the small game, explicitly retire excluded requirements instead of declaring them complete.
