# R2 adversary G9: How this file works, platform, testing, process, risks, glossary

Read: ROUND.md, BRIEF.md, R1-coord.md, R1-adv-G9.md, R1-impl-G9.md, the whole assembled.md (346.6 KB), CLAUDE.md, pretests/BUILDING-BLOCKS.md.
Scripted: every live item against the header's kind lists and its Done when or Check line; every glossary term against its use by section; secheck on my files (PASS).

**Round 1 landing.** Decisions 22–27, 33 and my 25 accepted changes landed as written: RES-07's three sizes, RES-13's reruns, RES-24, RES-25, PLT-04's budgets and benchmarks, PLT-05's resume, RES-05's exact match, PRC-09 to PRC-12, RSK-04, RSK-29 and the glossary corrections.
All features have a Done when line and all rules have a Check line, except the four items in finding 9.
The test plan covers the signature moments (RES-17 with each MOM Check). It covers determinism (RES-05, PRC-10's repeat check, TIM-17, WLD-13, MND-14, PLT-05, PLT-07, PLT-08) and speed (TIM-07, PLT-04).
What did not land is where decisions meet: RES-07 against CUL-33 and the stages (1, 6), the benchmarks against WLD-34 and BIO-04 (3, 5), and RES-24 against RES-13 (8).
Rules stated twice: the computing arrangement (SCP-15 and RES-07, each citing the other), battery and opening time (VIS-14 and PLT-04), and the exact match (RES-05, PLT-01 and PLT-05). Findings 4 and the cuts handle them.
Cost figures use the file's own numbers. MND-15: a person costs about 1 ms of a held middle core per game day, about 0.5 ms of a cloud core by the pre-tests. TIM-07: the world alone runs at 10 game years a minute, about 5 on one cloud core. BIO-04: 0.8% growth a year from about 80 people. A session has 3.4 effective cores.

### 1. [blocker] RES-07, CUL-33, CUL-26, RES-16, RES-25, RES-21, MAT-21 The pace tests judge culture before it exists, and two tests lack setups other items ask for
Problem:
- RES-07 gives stages only to the nine steps of TIM-19. The CUL-33 targets fall under "Sizes, each judging the targets whose windows close within its years", with TIM-19's pass rule (CUL-33's Check). So from MIL-02, the nightly run to Year 60 judges a shared spirit by Year 5, a rite by 20, a myth by 40, a band split by 50 and a festival by 60. But beliefs, rites and myths come at MIL-05, and band splits and festivals at MIL-06 (SCP-16). Read as written, every nightly run fails, and MIL-02 to MIL-05 can never close (PRC-10: the pace tests must pass before a stage closes).
- CUL-26's Done when, the only check of religion, reads "the 20 pace-test worlds run to Year 150". That size first runs at the close of MIL-06, so MIL-05, which builds religion, can't confirm it.
- RES-16 relies on "each stage's closing pace test" on new seeds, but RES-07 defines one only for MIL-06 and MIL-07.
- RES-25 has no stage. Before whole worlds exist (MIL-04), a world has too little to record.
- MAT-21's Done when needs "a second people's first" from the sharp-stone test. That scene is one band of one people (RES-02), and peoples only split at MIL-06.
- RES-21 puts every scene on "a piece of land about 10 km across". MOM-02 needs two bands a day's walk apart (20–30 km), and CUL-16 and CUL-23 need a mountain range between bands.
Fix:
- RES-07 **Stages** becomes: "Each target counts from the stage that builds it: flakes `MIL-02`; fire `MIL-03`; clothing and huts `MIL-04`; shared spirits, rites and myths `MIL-05`; pottery, dogs, band splits, festivals, feuds, new peoples and raids `MIL-06`; herding, villages, farming, copper and chiefs `MIL-07` (`SCP-16`); `RES-25` from `MIL-04`."
- RES-07 **Sizes**: after "20 worlds to Year 60", add "and at each other stage close, the same on new seeds (`RES-16`)".
- G7, CUL-26 Done when: "of the 20 pace-test worlds run to Year 60 (`RES-07`), at least 10 have a people with a sacred place, a shaman and a myth, and no step ever comes before its conditions hold." If G7 keeps Year 150, RES-07 also runs that size at the close of MIL-05, which costs one more night (about 6 session-hours).
- G4, MAT-21 Done when: "in the sharp-stone test (`RES-02`), every first flake gives an entry with who, when, where, route, inputs and word; from `MIL-06`, a second people's first in the pace tests gives a short one."
- RES-21: "on land cut from a generated world, about 10 km across for one band and as wide as its setting needs for more, or from the first region before `MIL-04` (`WLD-34`)".
Cross-section: G7 (CUL-26), G4 (MAT-21).

### 2. [major] PRC-10, PRC-11, PLT-05, RES-01, RES-17 Nothing says what happens when the night's checks fail
Problem:
- Work joins after the 20-minute checks. The pace tests, moment scenes over 10 game years and every check "longer" than the gate run that night, after the join (RES-01, RES-17, PRC-10).
- PLT-05 runs them "on the latest alpha", which can lag the main version by several joins, and "their results wait for the next session".
  Nothing says the next session reads the results first, that a new failure stops other joins until it is fixed or undone, or that a failing build must not reach your phone.
  So agents can stack alphas on a broken main version until a stage close finds the problem.
- "Anything longer running overnight" sets no order. Trials and scenes grow with every blueprint and chain, and at TIM-07's minimum speeds the gate can outgrow 20 minutes. Then an agent may move any check out of it, the repeat check included.
- One more gap: a pace target still being tuned must not freeze all work, so the rule has to tell a new failure from a target not yet met.
Fix:
- PRC-10, after the first list: "**If they outgrow 20 minutes,** scenes of items the change doesn't touch move to the night first; the trials, the scenes of the items it touches, and the catalogue, repeat, file, commit and coverage checks always run before joining."
- PRC-10 adds: "**After the night:** the next session reads the night's results before anything else. A check that passed before and now fails is fixed, or the change behind it undone, before other work joins. A pace target not yet met goes to tuning (`RES-16`) and blocks only the stage close."
- PLT-05: "on the latest alpha" becomes "on the main version as it stands".
- PRC-11 Check adds: "... and that no check that passed before was failing on its build."
Cross-section: none (CLAUDE.md rule 8 already sends agents to PRC-10).

### 3. [major] PLT-04, WLD-34, MND-15, RES-05 The benchmarks can't be built as written
Problem:
- The stage budgets need "the 1,000-person world" from the close of MIL-01, "placed on generated land until whole worlds hold that many". There is no generator until MIL-04, only the first region: about 40 by 40 km, set by hand (WLD-34).
  That land feeds about 130–400 foragers (WLD-04: one per 10 km²; WLD-30: 100–300 km² for a band of 25).
  Worlds of 500 to 3,000 people would starve there within weeks. At 8 game years a minute, the 3-minute warm-up alone covers about 24 game years, so the reading would measure a famine, not a world.
- MND-15's Check reads the phone benchmark for "a camp of 30 and a village of 300", but the benchmark list has no camp of 30.
- RES-05 needs the benchmark worlds to "end exactly the same on the phone and in the cloud". A benchmark read over a fixed time ends at different game dates on different machines, and nothing says the result code carries an end state.
Fix: PLT-04 **Benchmark worlds** becomes: "a camp of about 30 and a village of about 300 at close camp zoom, and worlds of about 100, 500, 1,000, 2,000 and 3,000 people with about 10 km² of land each: until `MIL-04` on land set by hand like the first region (`WLD-34`), then on generated worlds; from `MIL-04` also the world with nobody in it. Each runs to a set game date, and the result code carries its end state for the match of `RES-05`."
Cross-section: G3 (WLD-34 may say that benchmark land is set the same way, as large as needed).

### 4. [major] RES-07, SCP-15, RES-09, RSK-14, WLD-15, WLD-18, BIO-06, BIO-22, MAT-08 No test states its cost, and a dozen whole-world checks have no slot in the nights
Problem:
- SCP-15 asks you to agree once to five side-by-side sessions for the full test, and RSK-14 watches for "a full pace test taking over a week". But no size of RES-07 states its session-hours. RES-09 asks each test for a budget only before it first runs, which is after your agreement.
- By the file's own numbers:
  - nightly run: about 2 session-hours;
  - Year-150 run: about 4–6;
  - full test: about 20–45. Each of its worlds runs 6–15 hours on one core, so it takes one or two nights on three to five sessions, mostly for the late centuries above 2,000 people.
  That is affordable, but it is written nowhere, so neither you nor RSK-14 can tell when a test overruns.
- Other items ask for whole worlds that RES-07 doesn't promise:
  - WLD-15: 20 worlds of 500 game years, about 10 session-hours even with nobody in them, more than a nightly run;
  - WLD-18: 20 of 100; WLD-16: 20 of 20;
  - WLD-08, WLD-14 and WLD-24: 20–100 generated worlds;
  - BIO-06: Year 500; BIO-22 and MAT-08: 200 years;
  - PRE-39 and the culture Done when lines: "the pace-test worlds".
  Nothing says which run serves them, or when the empty-world checks run, so builders will either add nights or skip the checks.
Fix:
- RES-07 **Computing** becomes: "One cloud session a night: the nightly size about 2 session-hours, the Year-150 run about 6; the full test about 20–45, over one or two nights on up to five sessions side by side, which you agree to once (`SCP-15`); each stage report sets the real cost against these (`RES-06`)."
- RES-07 adds: "**Other whole-world checks** read these worlds, at the first size that reaches their years (`BIO-04`, `BIO-06`, `BIO-22`, `MAT-08`, `PRE-39` and the culture items); checks of the world with nobody in it, such as `WLD-15` and `WLD-18`, run in the night's session at the stage that builds them, and again only when their rules change."
- SCP-15 keeps one line that cites RES-07 for the arrangement (G1).
- RSK-14's sign becomes "a pace test going over its stated budget".
- G3: WLD-15's Done when becomes "in 20 test worlds of 100 game years". That still gives hundreds of events and saves about 8 session-hours each time it runs.
Cross-section: G1 (SCP-15), G3 (WLD-15), G5 (BIO-06, BIO-22), G4 (MAT-08).

### 5. [major] PLT-04, PLT-10, TIM-07, RSK-02 Worlds at the end of the arc outgrow everything measured
Problem:
- BIO-04 gives 1,000–3,000 people at Year 400, and nothing caps births. At 0.8% a year, worlds then hold about 2,200–6,700 at Year 500, the end of the copper window and of the full pace test.
- Nothing measures worlds that size:
  - the largest benchmark is 3,000 (PLT-04), and TIM-07 sets no target beyond about 2,000;
  - PLT-04's memory line for kept areas assumes 2,000 people at Year 500;
  - TIM-07's target for "a world 500 years old" has no benchmark. No world is that old before the full test, so kept areas piling up (RSK-15, RSK-20) would show only at the last stage.
- PLT-10 sizes storage on "a world of 1,000 people after 1,000 game years". BIO-04 can't make that world: at 0.8% it would hold about 100,000 people or more. Its Done when also needs a 1,000-year run with births held down.
  The typical saved world is a Year-500 world of several thousand people, and the last 50 years, kept whole, make up most of its size.
- RSK-02's last response, "a lower population limit (MND-15)", changes nothing, since nothing caps births.
Fix:
- PLT-04 **How it works** adds: "**Old worlds:** from `MIL-06`, the pace-test worlds at Year 150, and from `MIL-07` the full test's at Year 500, each against a new world with as many people (`TIM-07`), with their kept areas read against the memory line."
- PLT-04 **Memory**: "... with kept areas at most about 1 GiB in a full pace-test world at Year 500 (`WLD-12`)."
- PLT-10: "Target: a full pace-test world at Year 500 fits in about 4 GB, history and kept areas together (measured in `PLT-04`)."
  Done when: "every full pace-test world, saved at Year 500, fits its target, and the warning comes before the phone is full."
- RSK-02 Response: "if needed, a lower population limit (`MND-15`)" becomes "past about 2,000, time slows and the game says so (`MND-15`)".
- G2 or G5 chooses one:
  - TIM-07 adds a target for the worlds the full test reaches, such as "about 6,000 people: at least a sixth";
  - or BIO-04 lets growth slow past about 2,000, so worlds stay near 3,000 at Year 500.
Cross-section: G2 (TIM-07), G5 (BIO-04).

### 6. [major] RES-13, RES-07, BIO-04 The full test is always "provisional", and its rerun is undefined
Problem:
- RES-07's full test uses 10 worlds by design. RES-13 marks any check with fewer than 20 runs "provisional", and reruns a failed check "on 20 fresh seeds".
- So the test that closes MIL-07 ("every pace target met", SCP-16) is always provisional. The file never says whether a provisional pass can close a stage.
  A failed full test could rerun 10 or 20 worlds to Year 500, which means one to three more nights on five sessions, and builders will choose differently.
- BIO-04's Done when wants its ranges, including 1,000–3,000 people at Year 400, "in at least 16 of 20 runs". Only the full test reaches Year 400, and it has 10 worlds.
Fix:
- RES-13 adds: "The full pace test uses 10 worlds by design and is not provisional; if it fails, 10 more run and its rule is applied to all 20 (at least 10 inside each window, at most 5 before it)."
- RES-13 adds: "A provisional result counts, and the stage report names it (`RES-06`)."
- G5, BIO-04 Done when: "... in at least 16 of 20 runs, and the Year-400 numbers in at least 8 of the full test's 10 worlds (`RES-07`)."
Cross-section: G5 (BIO-04).

### 7. [major] PLT-04, WLD-12, RSK-02 The target for making areas is 8 to 30 times faster than the pre-tests measured
Problem:
- PLT-04 caps making areas at a tenth of the simulation's time, "at about 5 ms each".
- The pre-tests built the metre-scale ground of 1 km² in 0.59 s on the phone's fastest core. That is about 40 ms an area at full speed and 90–150 ms at held speed, before adding single trees, ground-cover patches and stones. The R1 implementer flagged this, and nobody acted on it.
- WLD-12 makes an area wherever people stop, and makes it again once their marks fade.
  At 1,000 people and 2–4 game years a minute, even one new area per band per game day is 80–160 areas every real second. At the measured cost, that is about 3–20 seconds of one core's work every real second.
- So the share fails from MIL-01, because the first region already makes areas. The file neither names this risk nor offers a way out.
Fix:
- PLT-04 **Shares**: drop "at about 5 ms each". The share itself is the target.
- RSK-02 **Signs** adds "making areas over its share (`PLT-04`)".
- G3 decides in WLD-12, the one design lever: areas made where people stop hold what the rules read (single plants, ground cover, stones, things, caves and shelter). The metre-scale ground shape is made only for the picture, within about 1 km of the camera, and no rule reads it (`WLD-13`).
Cross-section: G3 (WLD-12).

### 8. [major] RES-24, RES-13, header ("About"), MAT-04 Blueprint chances have two different pass rules
Problem:
- The header sends every chance to RES-13. RES-13 checks shares "over at least 1,000 cases ... to within a third either way", which allows 40–80% for a 60% chance.
- MAT-04's Done when checks the same chances "in trials (RES-24)": 200 tries per level, "in the range its chance allows, such as 100–140 of 200", which is 50–70%.
- So for one blueprint, one builder runs 1,000 loosely judged cases, and another runs 400 tightly judged tries.
- RES-24 also gives no rule beyond its one example, though MAT-04's chances run from 5% to 95%.
Fix:
- RES-13's last line: "Shares an item promises, other than a blueprint's chance (`RES-24`), are checked over at least 1,000 cases: ..."
- RES-24: "... and its successes must fall where 200 tries of that chance land 99 times in 100, such as 100–140 for a 60% chance and 3–19 for a 5% one."
Cross-section: none (MAT-04, MAT-17 and section 7.6 already point to RES-24).

### 9. [major] Header kind lists and approvals, VIS-17, TIM-16, VIS-14, VIS-15, PLT-01, section 17, glossary Lines that break the file's own rules
Problem:
- VIS-17 now has a binding Done when (whole worlds pass RES-25, and at close camp zoom most awake people visibly do different things). But VIS is context, so the coverage check (PRC-12) never maps or tests it. The file's only check of a busy camp goes untracked.
- TIM-16, which every repeat rests on, is listed as a rule but has no Check line.
- VIS-14 and VIS-15 are listed as rules, but they carry Done when lines, the label for features.
- PLT-01's Done when needs the phone benchmark at every alpha. That clashes with PLT-04 and RES-05 (every stage), and with PRC-11 and RSK-23 (only stage reviews wait for you).
- The header's "you approve them when each milestone starts" adds a second wait that PRC-11 and RSK-23 rule out.
- Section 17 says WLD-04 is "measured in experiments". WLD-04 itself says "by running worlds", and WLD-30's scene measures it.
- Glossary:
  - "Herd count" is a term the file never uses (it says "count");
  - "Catalogue" leaves out thoughts (MAT-13);
  - "held speed" has no entry, though MND-15, PLT-04 and TIM-07 set their targets at it.
- CLAUDE.md says the pre-test folder is deleted "once the architecture is written". PRC-08 adds "and each open question has moved into it or into its item".
Fix:
- Header: Rules drops VIS-14 and VIS-15; Features adds "`VIS-14`, `VIS-15`, `VIS-17`".
- G1, VIS-17 Done when, so that it can be tested: "whole worlds pass `RES-25`, and in a camp scene of about 30 by day, awake people are doing at least 6 different activities at most moments."
- G2, TIM-16 adds: "**Check:** the repeat check (`PRC-10`) and the phone and cloud match (`RES-05`)."
- PLT-01 Done when: "every alpha installs and runs on your phone, and at every stage its benchmark worlds end exactly as they do in the cloud (`RES-05`)."
- Header: "... and you approve them in the review that closes the stage before (`RES-22`), the first stage's with the implementation plan; loosening one you approved needs your OK again (`RES-09`)."
- Section 17: "- **How many people the world can feed** (`WLD-04`): measured in a scene (`WLD-30`)."
- Glossary:
  - "Herd count" becomes "**Count:** animals kept as a number in their world cell: a herd far from people, and hares, small birds and most fish everywhere (`WLD-32`)";
  - Catalogue adds thoughts;
  - new entry: "**Held speed:** the speed the phone keeps after a few minutes at full load, at which every speed target is read (`PLT-01`, `PLT-04`)."
- CLAUDE.md: raise it with you as a proposed change, in PRC-08's words (it needs your OK).
Cross-section: G1 (VIS-17), G2 (TIM-16).

### Cuts
Sizes now (KB): s00 6.5, s13 8.2, s14 13.4, s15 6.9, s16 12.8, s17 0.8, s18 9.0, total 57.6.
1. **s18, drop 8 entries for terms used in one section only (−0.9 KB):** Biome, Chronicle, Cut-away view, Plain use, Recogniser, Shaman, Sound blueprint and Switch-off run.
   R1's coordinator set the rule that glossary terms appear in at least two sections. Nothing is lost, because each term is defined in the one item that uses it.
   Heat level stays, since "heat 5" is used in sections 1, 6 and 7.
2. **s18, shorten three entries to one clause plus an ID (−0.2 KB):**
   - "**Blueprint:** a hidden rule: one base action, on things that fit set ranges, in set conditions, gives a named result; nobody knows one until they discover or learn it (`MAT-04`)."
   - "**Area:** a patch about 256 m across, detailed to about a metre, made only where needed; unchanged, it is what the seed gives (`WLD-12`, `WLD-13`)."
   - "**Writer AI:** the phone's built-in language model, which only rewords pattern sentences and never writes dark events (`PRE-37`, `PRE-17`)."
3. **s16, merge risks that share a cause (−0.7 KB):**
   - RSK-17 and RSK-22 merge into RSK-08, "The writer falls short": flat text, softened dark events, or a phone update that changes or removes the writer, with one Signs line and one Response line;
   - RSK-20 merges into RSK-15, "Worlds outgrow the phone", covering memory and storage;
   - RSK-12 merges into RSK-21, "Updates harm worlds".
   Each merged risk becomes *Dropped* with "merged into `RSK-xx`". No other item cites them.
4. **s16, cap every live risk at about 350–400 bytes (−1.5 KB):** keep the rating line, and let Signs and Response cite the items that hold the mechanism instead of restating it. For example:
   - RSK-24 Response: "overnight mode's heat limit (`TIM-12`, `PLT-04`); worlds move by export (`PLT-08`)";
   - RSK-06 Response: "reality rules and the expected-fits check (`RCK`, `MAT-17`)".
   RSK-25 stays longer, because it owns the fallback launch set.
5. **s14 (−0.35 KB):**
   - merge RES-15 into RES-06: "published as a short page that opens on the phone, its links opening the worlds and entries it names, with a copy in the repository". RES-15 becomes *Dropped*; PLT-05, RSK-23 and the header's Features list cite RES-06 instead;
   - RES-03 drops "Fixed before the test first runs (`RES-09`):", because RES-09 already says so;
   - RES-02 drops its question and states the scene directly;
   - RES-12 drops "and a good surprise can become a new signature moment";
   - RES-21 drops "Only the setting is chosen", because RES-18 says it.
6. **s13 (−0.35 KB):**
   - PLT-01's "Measured in the pre-tests" becomes one line: "**Plans on:** 2 small, 4 middle and 1 fastest core at the speed they hold under full load, about 3 W, and about 8 GiB of memory.";
   - PLT-04's Battery and Opening lines cite VIS-14 and WLD-11 instead of restating them, keeping only the time to make a new area;
   - PLT-07 drops "as a new copy that replaces the old only once complete,", an implementation detail;
   - "at about 5 ms each" goes (finding 7).
7. **s15, drop PRC-05, merged into PRC-07 (−0.24 KB):** its Check repeats those of PRC-07 and PRC-02, and no item cites it.
8. **s00 (−0.13 KB):**
   - drop "It is written so that someone with no prior context can read it and understand the whole project.";
   - drop the unused *Open* row of the status table; section 17's heading becomes "Settled by measurement".
Cuts total about 4.4 KB. Findings 1–9 add about 1.8 KB here: RES-07 0.6, PRC-10 0.4, PLT-04 0.4, RES-13 0.25 and the glossary 0.15. Net, my sections fall by about 2.6 KB, to about 55 KB.
Outside my sections, findings 4 and 9 also allow G1 to cut SCP-15 to one line citing RES-07 (−0.15 KB) and G3 to shorten WLD-15's test.
