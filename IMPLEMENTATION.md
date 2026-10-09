# Kindling: the build plan

Next: M3, a discovery that changes the camp. Only this milestone has scheduled tasks.
One builder; reviews run while the builder is stopped. Keep the current renderer, stand-in art, individual minds and indirect powers.
This plan contains unfinished work only. M4–M10 remain later choices, not parallel work.
Budget exception: M3 needs explicit save formats, rule values and five deliveries; the extra detail stays only while it is the next milestone.

## Where M3 stands (9 October 2026, 14:50 UTC; the session that built it was closed here)

- **Done:** α3.13a is delivered as signed build **41301** (27.66 MiB, `dist/`), merged at `ec3797d`. All of T3.13a.1–8 is done: real-phone touch (owner confirmed), current-format save chunks with older saves refused, items/catalogue, finite stock, crafting work and meals, personal discovery, Discovery camp, item cards, the recorded First flake example, and the clearer camp performance test.
- **Also done early:** the chainsaw `T3.13e.5` (owner moved it forward when the package went 73,403 bytes over 50 MiB). It removed the Developer tools pages, old converters, fixtures and unused packed art: 50.07 → 27.66 MiB. Approved art stays in `data/`, out of the APK.
- **α3.13b delivered as check build 41302:** personal skills/evidence (format 3), observation, telling/shared practice and factual History are built. The fireless 40-run spread gate failed; the input-selection repair awaits fresh closing evidence in α3.13e. Release signing lacks the owner’s passphrase; see `dist/NOTE-3.13b.md`. The earlier storage draft in `wip/` is superseded; do not reapply it.
- **Next:** α3.13c, α3.13d and α3.13e, each with a check APK until the owner supplies the release passphrase. Then the end-of-M3 work: closing proofs and the one full audit (`T3.13e.3`), then the independent review (`T3.13e.4`).
- **Owner rules in force (9 October 2026):** no conversion of older saves; the builder works straight through M3 with no per-join independent reviews; the full audit runs only at a milestone's end; APKs stay under 50 MiB or the builder stops and reports; portrait first; status updates at least every 10 minutes, short and plain.
- **Waiting on the owner:** the ten-minute camp performance test on 41301 (the earlier missing-counter report never reproduced in the cloud), sustained battery/heat evidence, and the M2/M3 play answers.
- **Known limits:** before observation learning, a 20-seed three-day search noticed flakes only in seeds 12 and 19; the real discovery gate is α3.13b's two-year 20-seed check. Craft work settles serially even with four workers configured.

## Rules every alpha keeps

Principles: `PRN-01`, `PRN-02`, `PRN-03`, `PRN-04`, `PRN-05`, `PRN-06`, `PRN-07`, `PRN-09`, `PRN-10`, `PRN-11`, `PRN-12`, `PRN-13`, `PRN-14`, `PRN-15`, `PRN-16`, `PRN-17`.
Scope: `SCP-02`, `SCP-03`, `SCP-04`, `SCP-05`, `SCP-06`, `SCP-07`, `SCP-08`, `SCP-09`, `SCP-10`, `SCP-11`, `SCP-12`, `SCP-15`, `SCP-17`, `SCP-18`, `SCP-19`, `SCP-20`, `SCP-21`.
Process: `PRC-02`, `PRC-03`, `PRC-04`, `PRC-06`, `PRC-07`, `PRC-09`, `PRC-10`, `PRC-11`, `PRC-12`, `RES-01`, `RES-09`, `RES-13`, `RES-18`, `RES-19`, `RES-22`.

Run touched tests during work and the routine/delivery gate before joining or shipping; keep save recovery and deterministic proofs. Run the full audit (`tools/check.sh --audit`) only at the end of the milestone, never during its deliveries (owner, 9 October 2026).
**Older saves are not converted** (owner, 9 October 2026). Each build opens only saves in its own format and refuses older ones with a plain message; the player starts a new camp. Write no migration code, old-save fixtures or conversion tests, and when a format change breaks an old-save test, delete that test. The note says when the build cannot open the previous build's camps. Keep save/reopen, journal replay and crash recovery for the build's own saves.
Self-review each delivery. In M3 the builder works straight through without stopping for independent reviews before joining; one independent review covers all of M3 at its end (owner, 9 October 2026).
Deliver an APK with What is new / What to try / What is rough, measured build/check/review/package time and honest remaining acceptance.
Only the owner changes decided meaning. The sixteen scope-cut proposals are not approvals.
**Serves** identifies a built subset, never full acceptance; preserve old IDs and distributed version codes.

**Outstanding acceptance:**

| Work still owed | Destination |
|---|---|
| Physical-phone evidence (`PLT-04`, `RES-06`): run Menu → Camp performance test, cool/unplugged/flight mode, ten minutes; retain its report. | Before M3 closes; repeat on the new build, plus the required sustained battery/heat run. A ten-minute report does not prove an hour's battery target. |
| Owner play (`RES-22`): tap/hold fixed in 31305 and confirmed on the phone; decision/influence answers remain open. | Obtain decision/influence/comfort answers before M3 closes. |
| Full starting kit/map/renewal, bodies, minds, materials and social clauses beyond the slice. | M8; physical generation/weather M7, complete living settling and biological starts M8. The bounded camp proves neither. |
| Unbuilt presentation, sound and powers, including animal/person/fear dreams. | M9; retain current place dreams, UI and save regressions throughout. |
| Full population, long arc, old-world storage and performance gates. | M10, with relevant checks when their real consumers arrive. |

Retained regression obligations: pause/speed (`TIM-04`), person identity (`BIO-03`), needs/actions/senses (`BIO-09`, `BIO-18`, `BIO-21`, `MND-03`, `MND-07`), remembered places (`MND-28`), equal minds/population policy (`MND-15`), start/navigation and readable text (`PRE-32`, `PRE-40`). Full clauses still follow the destinations above.
Further acceptance: care (`BIO-23`) starts M4, finishes M8; winter fire (`MOM-01`) waits for those cold/health consumers. Fever loss, buried tools and cultural transmission (`MOM-02`, `MOM-09`, `CUL-01`, `CUL-03`, `CUL-16`) close M5/M8; moment checks (`RES-17`) follow actual dependencies.
Unbuilt craft checks stay M8/M10: tanning, fermentation, heat treatment, other material transformations and joined-part properties (`RCK-06`, `RCK-07`, `RCK-10`, `RCK-11`, `RCK-12`, `RCK-13`, `RCK-25`); no flake/fire subset claims them.

The small M6 finish still excludes the globe, full catalogues/cultures, copper and optional prose writer; ending there requires explicit retirement of remaining promises.

## M3 A discovery that changes the camp

**Goal:** ordinary action produces a useful flake; a second person learns; fire changes food and comfort; an idea dream prompts attempts without supplying knowledge or success (`MIL-10`).

**Serves:** `PRE-32`, `GOD-05`, `BIO-09`, `BIO-18`, `MND-03`, `MND-07`, `MAT-01`, `MAT-02`, `MAT-03`, `MAT-04`, `MAT-06`, `MAT-09`, `MAT-10`, `MAT-12`, `MAT-13`, `MAT-14`, `MAT-17`, `MAT-20`, `MAT-21`, `MND-04`, `MND-09`, `MND-10`, `MND-11`, `BIO-20`, `RCK-01`, `TIM-17`, `PLT-07`, `PRE-35`, `MND-06`, `MND-13`, `MND-14`, `MND-18`, `MND-23`, `CUL-02`, `PRE-05`, `PRE-14`, `RES-02`, `RES-03`, `RES-10`, `MAT-07`, `MAT-18`, `MAT-19`, `MAT-22`, `BIO-02`, `BIO-11`, `RCK-02`, `RCK-03`, `RES-23`, `RES-24`, `GOD-03`, `GOD-06`, `GOD-07`, `GOD-08`, `GOD-09`, `GOD-10`, `GOD-11`, `MND-12`, `RES-05`, `PRE-08`, `PRE-13`, `PRE-17`, `PRE-31`, `PRE-37`, `PRE-44`, `SND-01`, `SND-12`, `PLT-04`, `RES-06`, `RES-16`, `RES-22`

**You will see:** named makers, useful tools, learning sources, tended fire, cooked food and a short factual history, with portrait controls.

**Risks:** an older save could be half-read instead of refused; hidden catalogue facts can leak into minds; fast-forward can multiply discovery rolls; long tests can outlive finite supplies.
Touch selection/holding was the first blocker; 31305 fixed it and the owner confirmed it on the phone. Never cut individual knowledge, generic fitting or unscripted outcomes.

**Basis and notation:** `sim/src/kd/demo/living.cpp` currently scores food/water/rest/exploration; `Life` remembers three sites and one bodily action, not crafts.
`Camp` owns aggregate stone/wood; there are no production craft items or fires. Extend these systems, not a second simulation.
Below, **T** marks a proposed tunable value, not a new owner decision; unmarked requirement numbers come from PROJECT.md.
The illustrative flake recipe is adopted as T; its characteristic constraints still obey `RCK-01`.
All new headers/modules named below are additions; C++ paths are below `sim/src/kd/` unless stated.

**Common storage contract:** positions are torus centimetres; catalogue lengths millimetres, masses milligrams, water millilitres, temperature milli-°C, times integer game seconds; a year is 60 days.
Use stable entity IDs, catalogue names through NAME remapping, fixed-point quantities and keyed random draws, never container order or frames.
Add rules-only entries under existing `data/base/`; assert unchanged world-making fingerprint and old NAME remapping, changed rules fingerprint. Do not add a new source or bypass big-update rejection.
Use extension chunks rather than changing foundation BEIN/THNG registry layouts. Include every new field in canonical digests, snapshots and rollback/touch copies.
`CAMP4` appends one u32 feature mask after the existing records: craft=1, learning=2, fire=4, ideas=8; enabled bits require the exact chunks below. Missing/duplicate/future chunks, orphan IDs, invalid catalogue references, quantities, progress or queued events fail visibly.
Saves from earlier builds are refused, not converted: check the format version before reading anything else and show “This camp was made by an older build. Start a new camp.” Add no upgrades or migration seals; the early `T3.13e.5` cleanup removes the obsolete APIs and old-format fixtures.

### α3.13a A sharp edge earns its place

**Done:** delivered as 41301 (`ec3797d`), 9 October 2026. Kept below as the record of its rules and values until M3 closes.

**Goal:** introduce conserved things, generic work and a first noticed flake that makes ordinary work better.

**Serves:** `PRE-32`, `GOD-05`, `BIO-09`, `BIO-18`, `MND-03`, `MND-07`, `MAT-01`, `MAT-02`, `MAT-03`, `MAT-04`, `MAT-06`, `MAT-09`, `MAT-10`, `MAT-12`, `MAT-13`, `MAT-14`, `MAT-17`, `MAT-20`, `MAT-21`, `MND-04`, `MND-09`, `MND-10`, `MND-11`, `BIO-20`, `RCK-01`, `TIM-17`, `PLT-07`, `PRE-35`.

**Architecture:** A3.2, A3.6–A3.8, A11, A12, A14, A15.

Data: add `world/craft.hpp`: Item(kind, owner/home, position, mass, length, state, quality 0–5, wear in millionths of a 0–5 step, maker, made_at, parent-input IDs); instances live on the existing things registry.
Work on each person holds action, intended recipe or none, input IDs/reserved masses, target, start/end, next try, completed tries, applied result marker and retained progress.
`world/knowledge.hpp` holds per-person familiar kinds with known-characteristic mask/value/certainty, performed-action mask, skill/sector levels, recent handling memories and hunches; learning fields are defined in α3.13b.
Save items/work in critical `CRFT1`, knowledge in `KNOW1`, result events in `HIST1` (fields below); `CAMP4` craft bit requires all three, `LIFE2` and DRMS1, or DRMS2 when ideas are enabled.
LIFE2 retains existing fields and adds meal_item (zero means legacy berries), food_factor_ppm, water_ml_per_kg and nutrient remainder; accepts named new action codes 8=craft, 9=watch-craft, 10=teach, 11=warm, 12=tend, with Work required where relevant. Preserve existing action codes, urgent slot 1 and habitat hourly slot 0.
A work Activity lasts to its goal or at most one hour; person slot 2 settles repeated known tries at Work.next_try, slot 0 ends the activity. Unknown fits roll only at activity end. Cancelled strikes have no result; interrupted gradual work keeps earned progress. Same-second ends settle once in event order, and multiple fits cannot consume an already-spent input.

Catalogue: add `data/craft.hpp`, register in `data/kinds.cpp`; `data/base/item/` records class/form, all 18 characteristics, size, state changes, break result and icon; unspecified characteristics explicitly zero (T).
Initial T values: stone weight 4; flint hardness/toughness/flaking 5/2/5, chert 4/2/4, granite 4/4/0, quartzite 5/5/0; raw edge 0, raw quality 2.
Also add bone (hardness/toughness 3/3, weight 3, food 1), dry stick/board (hardness 2, toughness 3, flexibility 2, weight 1, burn/fuel 3/3, water 0), green variants (water 4), grass/tinder (burn 5, fuel 1, fibre 2), nuts/kernels (food 3), roots (food 2, water 3), carcass/meat (food 3, water 3), raw hide, leaf bedding and rotten carrying wood (dry wood values, burn 4/fuel 1 T).
Add flake/crumb/chunk and wood-shaving result forms; dry shavings have burn 4 (T); flakes inherit their source, not a special named-material rule.
`data/base/blueprint/` entries contain action; class/form/range/size/mass input roles and retention/wear; place conditions; result/state/amount/leftovers; sector/difficulty/time; failure and hint signs; discovery factor.
Add crack_nuts_bones, butcher, leaf_bed, split_firewood, scrape_hide and sharp_flake; declare bank_fire/carry_ember now so starting knowledge resolves, but require a fire before these can run.
T known-use defaults: cracking difficulty 1, 60 seconds/100 g; bedding difficulty 1, 600 seconds/kg; butchery/firewood/scraping difficulty 1 with times below. Splitting firewood uses cut with edge≥3, yielding 950 g usable sticks and 50 g shavings per kilogram (T); gathering/sorting already-small wood is the slower plain-use alternative. The cut recipe is initially unknown; scraping without an edge uses a rough hard stone. All retain tools and conserve named leftovers. Names occur in catalogues/tests/text, never in mind selection or input matches.
T food water values become 200 ml/kg per characteristic point, preserving berries at 800 ml/kg; raw/cooked poison effects remain a later health obligation, not a poison-safety pass.

Rules: T flake consumes a core 80–300 mm, hardness 4–5/flaking 3–5, against retained striker ≥60 mm, hardness/toughness 3–5; strike, difficulty 2, 30 seconds; result 30–80 mm, edge=flaking, toughness 1. Allocate 20 g to each 50 mm flake, no more than available core mass (T), remainder to a reusable chunk when ≥80 mm, otherwise crumbs; no mass appears twice.
T failures split crumbs/shattered core 90/10. The example's cut-hand chance waits for M4's wound system (owner, 9 October 2026); M3 keeps lost input, time and tool wear. Do not claim the full illustrative recipe until M4 adds the injury.
Effective sharp edge means edge ≥3 after quality/wear (T): butcher 1 kg in 10 minutes with 90% edible yield versus bare hands 30 minutes/50%; prepare 1 kg firewood in 5 versus 10 minutes; scrape 1 kg hide in 15 versus 30 minutes (T). These are separate generic affordances/recipes, not faster berry gathering. Tool steps above minimum reduce try time 10% each, capped at one third.
Wear/quality follow `MAT-20`; T base flake wear is 25,000 millionths of a wear step per kilogram butchered, with toughness/quality multipliers applied once; use a 20 kg usable-meat deer fixture to calibrate roughly one-deer dulling. Broken pieces remain things.
Known success: clamp 50% +10 percentage points × (mean skill/sector − difficulty), plus the specified quality/rest/cold/darkness/arm modifiers, to 5–95%.
Unknown: one roll per fitting blueprint per whole activity, maker chance × accident 1/20, experiment 1/5 or hunch 1/2 × discovery factor 1. Repeated known tries may settle individually; repeated unknown tries never add rolls.
Own work is perceived; surprise is noticed with probability 1/4–3/4 by curiosity, default 1/2, halved once if busy/tired/frightened. Only noticed success grants skill 1; failure signs can grant a hunch.
Choices inspect only known properties and reachable things within 30 m; hidden flaking/fuel become known through striking/burning. Curiosity, not predicted unknown results, motivates an experiment.

**Tasks:**

1. `T3.13a.1` Reproduce/fix the owner's Android tap/hold failure before craft work (`PRE-32`, `PRE-35`, `GOD-05`). In `game/pages/camp.gd`, `game/camp/drawing.gd`, root input forwarding and `game/test/camp_test.gd`, trace actual InputEventScreenTouch/ScreenDrag through viewport, safe-area coordinates, GUI hit tests and picking; do not assume the mouse path is equivalent. Tap selects the person; hold ≥550 ms opens their ring; dragging/pinching cancels selection/hold. Keep camera gestures and controls working. Tests inject press/release, drag, two fingers and elapsed hold through the viewport input path, never direct select/open_dream calls or mouse substitutes. On a physical phone verify portrait tap/hold and repeat after rotation; until then mark physical verification pending, not passed. Delivered in 31305; the owner confirmed touch works on the phone (9 October 2026).

2. `T3.13a.2` Add the new save chunks and their strict readers in `world/world.cpp`, `demo/kept.cpp` and keeper/archive (`PLT-07`). Do not convert older saves (owner, 9 October 2026): a save from 31305 or earlier is refused with a plain message, never half-read. The build's own saves reopen exactly, including identities, times, LIFE progress, queues, dream limits and ledger.
3. `T3.13a.3` Add schemas, catalogue entries and characteristic/fit validation (`MAT-03`, `MAT-04`, `MAT-17`). Each new entry supplies expected-fit tests; fabricated compatible materials work and granite cannot flake.
4. `T3.13a.4` Add items and reservations (`MAT-01`, `MAT-09`) in `world/craft.hpp`, `demo/crafting.cpp`. Materialise old stone 25% flint/25% chert/50% granite and wood as 60% dry sticks, 35% boards, 5% rotten carrying wood (T), debiting aggregate stocks once; display totals from the new stock, never both representations.
5. `T3.13a.5` Extend `living.cpp`/new `crafting.cpp` with generic known uses, finite meals, atomic try settlement and interruption (`MAT-04`, `TIM-17`). New food uses berry-equivalent nutrition: food characteristic/2 × mass (T); legacy berries stay exactly unchanged. Reserve portions, return unused stock, save remainders and prevent concurrent double use.
6. `T3.13a.6` Add perception, experiments, surprise/hunch recording and result knowledge in `demo/discovery.cpp` (`MND-04`, `MND-10`, `MND-11`). Details must expose actual evidence, not catalogue truth.
7. `T3.13a.7` Add Discovery camp under Saved camps → New, a recorded First flake example under Examples, and item cards via `view/src/world.cpp`, `game/camp/{drawing,words}.gd`, `game/pages/camp.gd` (`PRE-35`, `BIO-20`). Initial facts are labelled scene inputs: 25 adults, existing supplies plus finite nuts, carcasses, hides, roots, tinder and quartzite; quantities are recorded in the scene, not replenished to rescue discovery. T additions: nuts 20 kg, roots 50 kg, carcass 200 kg, hide 20 kg, grass 10 kg and quartzite 10 kg at the matching existing food/stone/wood sites; stone cores 120 mm/2 kg, strikers 100 mm/1 kg, sticks 300 mm/250 g, boards 300 mm/1 kg, food portions 1 kg. Use deterministic IDs and trim a final portion to conserve totals. New adults know the five starting crafts at skill 3, gathering 3/hunting 2/fire 1 plus age increments. Full starting-map/kin acceptance stays M8.
8. `T3.13a.8` Make the camp performance test work and be unmistakable (owner, 9 October 2026: on 31305 the running counter never appears) in `game/pages/camp_measure.gd` (`PLT-04`). The current test calls `start_measurement()` directly; first reproduce by pressing the real Run camp test button with touch in a portrait window, and find the cause. Hold the screen awake for the whole run (the Rendering test releases it on exit), show time left and the current speed in large text, and mark the report interrupted if the app left the foreground. Test it through the page, not by calling its functions.

**Tests:** first, `screen_touch_select`, `screen_touch_hold_ring`, `screen_drag_no_hold`, `two_finger_no_hold`, `touch_after_rotation` in `game/test/camp_test.gd`; retain raw-touch reproduction evidence and verify no ring before 550 ms. Then new `sim/tests/craft_test.cpp`: `generic_fit`, `granite_control`, `unknown_once_per_activity`, `interrupted_strike`, `mass_and_reservation`, `edge_improves_work`, `quality_wear`, `older_save_refused`. Plant duplicate settlement, hidden-flaking choice and missing CRFT; existing living/save tests remain except old-save tests, which are deleted when they break.
Use 200 attempts at low/high levels for each new blueprint (`RES-24`); fixtures declare bounds before tuning. Save at each phase, fuzz registry order, compare 1/4 workers and viewed/unviewed continuation.
Screens: portrait keeps the current map above a bottom card; tapping a thing shows “Sharp flake · edge 4 · held by Ari”, and the person's card “Cutting wood · quicker with a sharp edge”. Details show inputs, elapsed/remaining time and actual reason; unknown crafts are never listed as unlocks.

**On the phone:** your earlier camp will not open (older saves are not converted); confirm the plain message, then Menu → Saved camps → New → Discovery camp → Play at one minute/sec; tap a worker, open Details, then inspect a noticed result and the task it improves. Pause midway through work, save/reopen and confirm one result and unchanged stock. A run without discovery remains an honest outcome; Saved camps → Examples → First flake loads a labelled snapshot captured from an ordinary run, never edited into success, and proves no frequency.
Cut line: drop hide scraping from this delivery first and carry its consumer to α3.13b; keep butchery/firewood. Do not broaden to hunting, clothing or new art. Split any task exceeding one build session at its tested boundary.

### α3.13b A second person learns

**Goal:** knowledge has a holder, a source and a useful route to another person.

**Serves:** `MND-07`, `TIM-17`, `MND-06`, `MND-11`, `MND-13`, `MND-14`, `MND-18`, `MND-23`, `CUL-02`, `MAT-21`, `PRE-05`, `PRE-14`, `PRE-35`, `RES-02`, `RES-03`, `RES-10`.

**Architecture:** A11–A15, A17.

Data: KNOW1 records skills by recipe (level/best in thousandths, practice seconds, last use, source person/event and route), all 15 sector levels, observation credits in quarters, and beliefs about another person's known recipes with evidence time/source.
A memory stores ID/time/place, action, input kinds, result/sign, participants, strength, certainty; maximum 200, weakest first then oldest/ID. Hunches store action/input-kind/result guess, source memory/person, failures, last use; maximum five, expire after ten failures or one unused year. Preserve performing an action separately from knowing a recipe.
Save curiosity/kindness in 0–100, learning multiplier in ppm, scoped curiosity need/mood 0–100, hourly draw index and candidate reasons; these fields start in KNOW1 in α3.13a, with T test defaults 50/50, 1,000,000 and 60/60. Actual founders draw curiosity/kindness independently in 20–80 from seed/person-ID keys, so some can teach; never select a trait because it ensures a discovery. Test varied founders; full personality/inheritance remains M8.
Add `LEARN1` shared sessions: ID, teacher/learner, recipe, meeting position/time, state, credited seconds and last demonstrated try ID; set CAMP4 learning bit. No global unlocked-recipe set. LEARN1 references are sorted/unique and must agree with participants' Work/activity events; interruption credits use an applied counter. Peers learn absence only through a failed attempt or a truthful exchange, not by inspecting another mind.

Rules: a deliberate watcher within 5 m, clear sight and usable light gets a full credit per complete demonstrated use; a busy observer gets one quarter (distance T). Five credits teach skill 1; the first watched use gives a hunch. Partial watching earns proportional credit, saved in millionths of a quarter, never again on reopening.
A kind teacher (kindness ≥60 T) who knows a recipe and has evidence that a nearby learner lacks it offers a 30-minute shared practice (T); the learner accepts only if needs and current plans permit. Meet within 2 m; ordinary routes, no teleport; stop/credit elapsed work if separated, called or urgent.
The learner uses full maker chance rather than discovery discount; first success teaches skill 1. Taught practice earns 4× ordinary credit, with a further T multiplier 1 + teacher level/10. Telling gives only a hunch; known-use observation updates beliefs, not omniscient access to the learner's skill table.
T skill curve: successful practice counts twice failure; level 1→5 needs 180 effective practice hours, then 5→10 another 780, scaled by learning speed; sector practice accrues with the same work. Calibrate, without relaxing `MND-06`, against T one hour on six days of seven and its 60-day years. Unused levels decay toward half their recorded best, never below it; T half-life five years.
Experiments remain ordinary candidates: at most eight known recipes/30 options. T hourly eligible draws: average adult 1/168, curious ≥75 adult 1/24; do not retry draws at every short action. Mood >50/no pressing needs favours trying; unsolved urgent needs allow only experiments aimed at known relevant properties. Curiosity falls 10/day and rises 10 for novelty. Score expected known benefit × urgency as existing needs, subtract minutes/risks, add bounded curiosity; record the winning reasons and two rejected options. Calibrate actual attempts, not just eligibility.

**Tasks:**

1. `T3.13b.1` Implement durable personal evidence/skills and shared-session save validation in `world/knowledge.hpp`, `world/world.cpp`, `demo/learning.cpp` (`MND-06`, `MND-23`). Saves from earlier builds are refused, not converted.
2. `T3.13b.2` Implement distance/light/occlusion checks and elapsed observation credits (`MND-13`); hook demonstrated work ends, not the camera or the old hourly site survey.
3. `T3.13b.3` Add autonomous offers, shared practice and telling (`MND-13`, `TIM-17`); preserve unfinished practice across save and interrupts. Add the deferred scrape consumer if necessary.
4. `T3.13b.4` Fill the existing `HIST1` discovery/learning records and readable cards (`MAT-21`, `PRE-05`): event ID/time/place, actor, source, recipe/result, input IDs, route and coined word. Coin/store a stable two-syllable word at the first noticed success (T syllable list in the catalogue, collision suffix), reuse it on rediscovery; this history index is never a mind unlock. Created with the craft chunks, HIST1 has already retained first-flake events; now record ordinary learning and loss/return separately, never fictitious earlier history.
5. `T3.13b.5` Run sharp-stone scenes through `sim/src/kd/proof/` and new `sim/tests/learning_test.cpp` (`RES-02`, `RES-03`, `RES-10`). Supply and record sufficient finite resource/renewal budgets for the whole run; using scripted discoveries or replenishment invalidates it.

**Tests:** `five_watches`, `busy_quarter`, `occluded_observer`, `taught_full_chance`, `partial_lesson_reopen`, `no_global_unlock`, `last_holder_removed`, `skill_curve`. Removing a holder is a labelled test setup, not a disease/death implementation. Plant distance bypass and duplicate-credit faults.

`T3.13b.2` evidence: the named observation tests exercise actual craft ends, inclusive 5-m range, blocked sight, night/dawn/dusk, sleep, passing observers and saved fractional credit. Continuation matches after reopen, shuffled pools and four configured workers (craft execution still serial). Removing the distance check fails `occluded_observer` and `no_global_unlock`; ignoring the saved exposure cursor fails `partial_lesson_reopen` with excess credit. Both faults were removed. The previous three-day First flake seed 12 stopped reproducing as observation changed ordinary choices; seed 31 is recaptured without switches. This example is not a discovery-frequency gate. `T3.13b.4` must distinguish unfinished observation from known skills: the current bridge/card labels every skill record as knowledge, including pending credits. This task does not deliver teaching, telling, learning-history cards, population spread or the long discovery gate; `T3.13b.3–5` retain those checks.
`T3.13b.3` mechanics evidence: 100 independently keyed supervised trials distinguish full maker chance from a discovery discount. Telling remains a hunch; evidence-backed offers collect real inputs and walk before practice. Interrupted gradual practice reopens and continues exactly across shuffled storage and four configured workers, with saved elapsed credit applied once. Urgent learners decline, successful learners update the teacher's witnessed belief, and distant peers remain ignorant. Personal fading, hunch expiry and curiosity decline now settle at ordinary choices. These mechanics tests do not certify population spread or the long discovery gate.
`T3.13b.4` records actual watched/taught acquisitions separately from the source result. Pending observation cards expose credit, not a hidden recipe name. The plain History sheet links actual people, sources and physical inputs/results and restores selection with Back; its index never unlocks a mind. Labelled last-holder removal records loss without a disease implementation; a subsequent real noticed unknown use records return and reuses the stored word. No earlier events are invented.
`T3.13b.5` fireless proof result (seeds 21–40, then RES-13 fresh 42–61, identical frozen inputs): discovery 20/20 then 40/40 within two years; spread only 3/20 then 4/40 reached three quarters of 25 adults, failing the required 32/40. Three routes appeared across all forty; the four-year non-flaking control made zero flakes. Preserve these failures; no third set or passing retune. Five independent processes took 131 s then 138 s. Initial reserves were 40 t fruit, 40,000 L root water, 120,000 L upstream and 200 extra 50-kg stone cores; no runtime replenishment. Labelled twenty-seed practice calendars reached skill 5 by day 179 under the declared six-days-in-seven schedule and retained at least half-best after ten unused years; this is mechanical calibration, not autonomous practice evidence. Round-to-nearest fading removes whole-thousandth loss at brief practice anchors without changing slopes or half-life. Population acceptance remains failed.
Same-seed diagnosis reproduced all forty end digests in 202 s: 4,737 exchanges, 30 accepted offers (26 runs had none), 532 watched and five taught sharp-flake acquisitions. A reproduced input-selection bug preferred a teacher-owned tool and then rejected it without trying accessible alternatives; selection now filters first and also considers the learner’s visible tools, preserving ownership, predicates and all T values. Exact full/busy watch splits and completed-lesson/lifetime hunch counts were not retained. These forty remain failed evidence from before that fix; α3.13e must judge fresh closing seeds.
The complete starting-kit RES-02/03 gate closes in α3.13e, after α3.13c supplies fire; α3.13b reports its fireless mechanics scenes as partial. Gate: 20 seeds, until one year after first flake or three without; discovery within two years in ≥16/20, three quarters of adults learn within the following year, ≥2 discovery routes; non-flaking control four years with zero flakes. Preserve failures and apply RES-13's fresh-seed failure procedure. Run the separate MND-06 practice/fading checks; no short demo substitutes for them.
Screens: person → Details → Knowledge reads “Sharp flake · skill 1 · watched Bela”; “Hunch: try striking this stone” stays separate. History reads “Bela noticed a sharp flake while cracking nuts”, with actual time, inputs and word; tap source/person to inspect.

**On the phone:** follow a knapper, then a watcher; compare their Knowledge lines before/after learning. Reopen during a lesson, inspect its source, then watch the learner make a useful tool without help. Landscape keeps the same controls in the side card.
Cut line: drop animated teaching gestures and decorative history layout first. Kin/friend preferences, children, cross-camp spread, tallies, fever loss and buried-artifact stories remain M5/M8; never replace teaching with a band-wide unlock.

### α3.13c Fire has a cost and a use

**Goal:** discover friction fire through generic attempts; tending, warmth and cooking produce visible consequences.

**Serves:** `BIO-09`, `MAT-04`, `MAT-07`, `MAT-18`, `MAT-19`, `MAT-22`, `BIO-02`, `BIO-11`, `RCK-02`, `RCK-03`, `MND-04`, `MND-10`, `MND-11`, `RES-23`, `RES-24`, `PRE-35`.

**Architecture:** A3.3, A10–A12, A15.

Data: `world/fire.hpp` Fire stores hearth/owner/position, heat 0–5, remaining fuel and ash mg, burn remainder, settled_at, embers_until, banked_until, air_until and next transition. HeatTimer stores item, target state, elapsed exposure seconds, settled_at, required heat band, completed flag. Add both to critical `FIRE1`; CAMP4 fire bit requires it.
`THER1` per person saves felt milli-°C, warmth 0–100, settled_at, warming progress and rate remainders; per camp saves scene ambient temperature and next change. This is comfort, not full health/weather. Fire transitions use camp slots 2/3 for next thermal/timer deadline; keep hourly renewal and all due events consistent.
New rules-only entries: ember_drill, ember_plough and roast_food; activate the predeclared bank_fire/carry_ember and add blow_tinder as a plain-use affordance; T bank/carry difficulty 1, 300/60 seconds, ash-cover/rotten-wood-or-fungus containers, no new ember on failure; blow_tinder is a tended heat action using the lighting rule, not another discovery requirement; result ember/ash/charcoal and cooked/burnt states. Work and KNOW1 reuse existing fields; earlier saves are refused, so no old camp gains fire. Fresh Discovery camp starts with one lightning-derived campfire holding 5 kg deducted from its wood stock (T); bank/carry knowledge is starting knowledge, friction/cooking are not.

Rules: dry wood/tinder water 0–1 only for friction, never green/wet; T drill/plough difficulty 4/3, 300-second tries, two wood inputs ≥100 mm with hardness 1–3, burn ≥2, one rod and one board/grooved piece. T failed dry friction yields a smoke hint in one half of failures; no heat/ember is created by a mere hint.
An ember blown in burn 4–5 tinder lights within 60 seconds; unblown has T 1/2 chance to die after 180 seconds. Water ≥3 cannot ignite and damps an existing fire; T one kilogram lowers its heat one level while evaporating.
Open fire tops at 3: tinder/twigs heat 2; suitable sticks/logs reach 3 in 600 seconds. Level 3 consumes 5 kg/hour, level 2 T 1 kg/hour, settled by mass with carried remainder. Exhausted fuel leaves embers for T three hours; banking keeps them T twelve hours, carrying in suitable rotten wood/tinder fungus one day. No perpetual heat from zero fuel.
Feeding, banking, blowing and carrying are ordinary choices using known fire affordances and finite inputs; working/sleeping people are not commanded to tend it. Flame spreads within 1 m to burn≥2, water<3 things; a ring contains ground spread. Keep stock beyond 1 m by scene layout, not immunity flags. Global wildfire/weather/kilns stay later.
Campfire adds 15 °C within 2 m. T mild scene ambient 24 °C day/18 °C night; warmth=max(0,100−5×degrees below the 24 °C resting comfort limit), capped 100. Work lowers the comfort limit 10 °C. Add a warm/rest-near-fire candidate, urgent below 20; no claim of lethal cold, clothing or winter survival. Above 32 °C, T water use rises 2% per extra degree with saved rate remainder.
At heat 2–3, the cooking attempt settles after one hour at maker level, or level 0 if unmeant, through the normal recipe chance; success gives cooked state, food +1 and no higher poison. T roast difficulty 1; failed cooking leaves raw food, with elapsed heat retained. Double total exposure or heat 4 burns to food 0. Changes occur without an observer; only noticing supplies knowledge. Recompute the next deadline on every moved input/heat transition; no frame polling or free restart of elapsed exposure. T a full hour at heat 4 burns; moving food away pauses exposure, rather than granting another first-hour chance.

**Tasks:**

1. `T3.13c.1` Add fire/timer/thermal records, strict FIRE1/THER1 readers and deadlines; earlier saves are refused (`MAT-18`, `MAT-19`) in `world/world.cpp`, `world/fire.hpp`, `demo/fire.cpp`.
2. `T3.13c.2` Add the two friction routes and smoke hints to catalogues/matcher (`MAT-07`, `RCK-02`); ordinary drill/grind experiments must work before any idea dream exists.
3. `T3.13c.3` Implement lighting, fuel, banking/carrying and local ignition with conserved stocks (`MAT-18`); add generic tending options in `living.cpp`.
4. `T3.13c.4` Add felt warmth, warm/rest selection and water-rate integration (`BIO-11`); teach actual experienced fire warmth as a memory, not an initial invented event.
5. `T3.13c.5` Implement cooking/burning, nutrition and incidental discovery (`RCK-03`, `MND-10`) in `demo/fire.cpp`, `living.cpp`; at-risk food may fall by heat through ordinary placement, never a scripted first-cooking event.
6. `T3.13c.6` Add a stand-in flame and fire/food cards in the existing drawing/bridge (`PRE-35`), plus a fresh scene variant whose initial fire is already out for friction validation (`RES-23`). This variant has no forced success or added knowledge.

**Tests:** `dry_friction_both_routes`, `wet_never_ignites`, `five_kg_hour`, `banked_overnight`, `warmth_two_metres`, `cook_one_hour_burn_two`, `unseen_timer`, `fuel_competition`, `fire_reopen`. Plant negative-fuel, repeated-cook and missed-transition faults; preserve interrupt and 1/4-worker/view equivalence; 200 low/high trials per recipe.
Screens: fire card “Campfire · 2.5 kg fuel · about 30 minutes left”; person “Resting by fire · feels warmer”; food “Cooked roots · more filling”; quantities/times are sampled state, not animation guesses. History names the first noticer and actual route, not merely the person nearest the camera.

**On the phone:** inspect fuel, follow someone tending, compare warmth near/away, watch roots cook, then leave a separate portion until burnt. Reopen with a timer running. In the out-fire scene, observe attempts and their failures as well as successes.
Cut line: remove extra flame poses and extra foods first; retain roots and one meat entry for the cooking rule. No kiln, furnace, rain simulation or complete winter health. Carry `MOM-01`'s hard-winter acceptance to M4/M8 when its dependencies exist; a mild camp is not that pass.

### α3.13d An idea remains their own

**Goal:** extend the existing dream path from known places to a remembered action, without leaking hidden recipes or guaranteeing discovery.

**Serves:** `GOD-03`, `GOD-05`, `GOD-06`, `GOD-07`, `GOD-08`, `GOD-09`, `GOD-10`, `GOD-11`, `MND-11`, `MND-12`, `PRE-35`, `RES-05`.

**Architecture:** A3.8, A11, A15.

Data: `DRMS2` versions Dream/DreamAct with kind (place/idea), memory ID, action, input-kind references, desired experienced property, internal chosen recipe reference, hunch ID and first_attempt_at. Keep all existing request/receipt/sleep/expiry/choice fields and the single shared nightly ledger. Saves with DRMS1 are refused, not converted; enable CAMP4 ideas bit only with DRMS2. Personal thoughts never contain sender, request number or private attribution; internal fit IDs are never “known” recipes.
New command 3 carries person and stable memory ID; resolve the compatible route deterministically at execution, not from UI text. Journal it before acting. No new blueprint catalogue entries. The catalogue hint selector pairs the selected memory with an already experienced benefit (for example warmth); the player confirms that visible pairing, not a named hidden result.

Rules: eligible memory requires a performed action, similar handled inputs and a compatible unknown blueprint leading toward an experienced need/result property. Choose highest sector experience, then highest success, then stable recipe ID for ties (T). A dry-wood twirl plus remembered warmth can form a hunch, not a known “fire drill” instruction.
Sent dreams replace that night's natural one at sleep, share one/person/night and three total, and retain the existing 06:00 reset, cancellation, pending cap and three-day mild pull. +60 is the existing bounded pull: zero against urgent needs or an existing plan; refreshed dreams never stack. The hunch can outlive the pull under MND-11's ordinary expiry/failure rules.
Natural hints use 1/60 per night, 1/20 with need<20, one third compatible/otherwise a plausible non-fitting guess; natural and sent thoughts use the same constructor. Do not run the old natural-place draw as a second dream. Lost memory/person or invalid fit at sleep cancels with a private reason. The private ledger alone distinguishes sent from natural source; the ordinary hunch source is dream. First_attempt_at records only a real action matching that hunch, never mere eligibility.

**Tasks:**

1. `T3.13d.1` Version `world/dream.hpp` and the strict validators/ledger in `world/world.cpp`; refuse earlier saves, never convert them (`GOD-07`, `RES-05`).
2. `T3.13d.2` Add memory-fit selection, command 3, shared natural/sent construction and attempt attribution in `living.cpp`/`discovery.cpp` (`GOD-03`, `MND-12`).
3. `T3.13d.3` Extend `view/src/world.{hpp,cpp}` and `game/camp/dreams.gd` with memory choices, revalidation, confirmation and private attempt records (`GOD-05`, `GOD-09`); keep prepare_dream's paused frontier and durable journal failure handling.
4. `T3.13d.4` Add paired scenes and `sim/tests/idea_dream_test.cpp`, plus app tests (`GOD-03`, `RES-05`); preserve failed seeds and compare the same sleepers with no sent dream.

**Tests:** `handled_memory_only`, `no_recipe_leak`, `shared_nightly_cap`, `urgent_need_wins`, `refresh_not_stack`, `natural_sent_same_thought`, `pending_idea_reopen`, `command_cut_recovery`, `no_guaranteed_success`. Plant bypassed input memory, separate place/idea caps and duplicated hunch strength.
In 20 scenes with remembered dry-wood twirling, valid tinder and experienced fire warmth, ≥15 produce a new relevant try within T three days after dreaming; report actual success separately. Remove the needed input in a control and prove no attempted consumption of absent material.
Screens: hold person → Dream… → Idea → “Twirled dry wood yesterday” (small idea mark) → “Recall the warmth they felt” → Send dream. Show no unknown recipe/result names; disabled choices explain missing memory/experience. Card says “Woke with an idea about the wood”; Your dreams lists requested/received/dreamt/attempt times and actual later result, without claiming causation from timing alone.

**On the phone:** choose an eligible memory, send, save while queued, reopen, advance through sleep, then read Your dreams and follow the next relevant attempt. Try a hungry sleeper too: urgent eating/drinking can win. Repeat refreshes the same mild influence.
Cut line: remove fancy memory illustrations and extra idea examples first. Keep both place and idea compatibility; animal/person/fear dreams remain M9. Never substitute a direct “make fire” button.

### α3.13e Follow the whole discovery

**Goal:** a new player can explain a discovery, its spread and an intervention without narration.

**Serves:** `PRE-05`, `PRE-08`, `PRE-13`, `PRE-14`, `PRE-17`, `PRE-31`, `PRE-35`, `PRE-37`, `PRE-44`, `SND-01`, `SND-12`, `PLT-04`, `RES-02`, `RES-03`, `RES-05`, `RES-06`, `RES-16`, `RES-22`.

**Architecture:** A14–A18.

Data/rules: no new simulation records or save versions. Read HIST1/KNOW1/CRFT1/FIRE1/DRMS2 through immutable snapshots; public history omits player attribution, Your dreams alone includes it. Add only reusable tap/work/fire sounds, tied to actual actions and camera distance; mute changes no state. Unknown future recipes stay absent from all cards/search.
Portrait: normal card shows name/action, one-line reason and Pause/Speed/Details/Dream; Details occupies at most 60% of safe height and scrolls vertically. History opens a separate scroll sheet; tap entry → actor/source/input cards → Back restores selection. Minimum 48 logical-pixel targets, fixed bottom confirmation bar, no horizontal scrolling; test 360×800 and 1080×2400, large text and keyboard-free use. Landscape uses the existing side dock, never overlaps controls with the world hit area.

**Tasks:**

1. `T3.13e.1` Finish history links and plain factual templates in `game/camp/words.gd`, a new `game/camp/history.gd`, and the bridge (`PRE-05`, `PRE-17`). Add “no source recorded” rather than inventing missing history.
2. `T3.13e.2` Repair portrait flow and add only the needed action/sound cues in existing drawing/pages (`PRE-31`, `PRE-44`, `SND-01`); test landscape with the same selection and active popup.
3. `T3.13e.3` Run the integrated discovery/teaching/fire/dream proofs, routine check and the milestone's one full audit (owner, 9 October 2026); compare phone/cloud end digests (`RES-05`, `RES-16`). Freeze settings before fresh closing seeds. Cloud scenario budgets T: sharp-stone two hours, learning/fire/dream one hour each; report overruns rather than change pass rules.
4. `T3.13e.4` With builder stopped, obtain independent milestone review, phone evidence and owner answers (`PLT-04`, `RES-06`, `RES-22`). Carry unresolved full-scale acceptance explicitly; do not infer playability from a demonstration seed.
5. `T3.13e.5` **Done** (`16fdd01`, 9 October 2026). Chainsaw the app (owner, 9 October 2026). Moved forward by the owner the same day, when the first α3.13a package came out 73,403 bytes over the 50 MiB limit: do it now, after T3.13a.4–7 are committed and before the α3.13a delivery, so every later delivery starts from the cut app (`PRE-35`). Remove every page, menu entry, developer tool, example, fixture, data file, test and code path the game no longer needs, including Developer tools pages left from M1 and M2 proving work. Delete rather than hide. Keep only what the camp game, its saves, determinism, meaningful regressions, the routine checks and the phone measurements need; where PROJECT.md requires something, keep it or list it for the owner. Owner-approved art (signed-off catalogue sheets and textures) stays in the repository but leaves the APK until the game draws it. The note lists what went, what stayed and why, with the APK size before and after. Completed early in α3.13a: obsolete pages/converters and unused APK art removed; required reports and exercised renderer regressions retained. Measured current-source APK: 52,506,299 → 29,006,950 bytes; approved repository art unchanged.

**Tests:** `portrait_discovery_route`, `history_source_links`, `dream_cancel_back`, `large_text_no_clip`, `rotation_keeps_selection`, `all_m3_phases_reopen`, `public_private_history`. Keep power-cut and export tests for the build's own saves; there are no old-save fixtures. Phone measurements include ≥97% on-time frames, no frame >50 ms late, held-speed/thermal report after warm-up and sustained battery use; missing hardware is unmeasured.

**On the phone:** install over the previous build; earlier camps are refused with a plain message. Start Discovery camp, find a useful flake, follow its learner, inspect fuel/cooked food, send an idea from a real memory, save/reopen and follow the result in History/Your dreams. Use 1 hour/sec only to wait, 1 minute/sec to watch, pause to read; turn once to landscape. Record a no-discovery run honestly; use a labelled saved example only to inspect later interactions.
Ask: “What changed and who learned it?”, “What did your dream influence?”, “Could you follow this upright?” Then run sustained measurements separately.
Cut line: drop decorative effects and extra sound variants first; allow one bounded readability repair. If the loop still needs narration, report the blocker and revisit scope with the owner rather than call M3 complete.

## M4 The camp survives change

**Goal:** Add seasonal pressure, care and a family through time, separately (`MIL-11`).
Also add the flake recipe's cut-hand chance once wounds exist, deferred from M3 (owner, 9 October 2026).

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
