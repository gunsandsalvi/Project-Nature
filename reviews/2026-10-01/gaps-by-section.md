# Kindling project file: completeness review

**Lens:** completeness. This review looks for gaps, hidden open questions, forgotten design choices, contradictions, ambiguities and checks that can't be checked.
**File reviewed:** `PROJECT.md` at commit `e87455a` (1,445 lines). Line numbers refer to that version.
**Method:** I read the whole file in order and asked of every item: would two independent teams build the same thing from this text? I also checked every ID reference by script. All 322 IDs are defined and every reference resolves, so no finding below is about broken links.
**Nothing in the project folder was changed.**

## Summary

51 findings: **8 High, 31 Medium, 12 Low.**

The three that matter most:

1. **H1:** Where you look changes what happens, because detail follows your camera. That breaks "same seed, same history", moving worlds between cloud and phone, and the validity of the experiments.
2. **H5:** Nothing defines how the game recognises a discovery, a skill, a people, a language or an institution. The chronicle, live moments, overlays and every experiment's pass/fail depend on it.
3. **H6:** After an update, the past may no longer be watchable (replays, report links, "watch year 41"). Nothing says whether old rules are kept, and builds will be frequent.

**How to read:** H = High, M = Medium, L = Low. Within each severity, findings follow the file's section order. Each finding ends with the question you need to answer.

---

## High

### Section 2: Principles

#### H1. Where you look changes what happens, which breaks "same seed, same history"

- **Type:** contradiction
- **Severity:** High
- **Items:** PRN-08 (l. 281–285), PRN-11 (l. 287–291), PRN-10 (l. 261–265), WLD-12 (l. 606), MND-14 (l. 944), TIM-03 (l. 512), PLT-05 (l. 1229), RES-01 (l. 1238), RSK-02 (l. 1379), glossary "Intervention" (l. 1428)
- **What's wrong:** Several items make the simulation's level of detail depend on your camera:
  - WLD-12: "Everything goes down to the metre where people are or where you look."
  - MND-14: "Minds far from your attention run in simpler form (habits, and knowledge held by the group as a whole)."
  - PRN-11's check: "what you're watching is simulated in the same detail whatever the load."

  A simpler mind, or a patch of animals, won't behave exactly like the detailed version, so what happens depends on where you looked. But PRN-08 says history is "fully determined by its seed and your interventions", and the glossary defines an intervention as "anything you do with your powers". The camera isn't one of them. (MND-10 also uses "attention" for a person's own attention, a second meaning of the same word.)
- **Why it matters:**
  - **Cloud and phone drift apart.** The cloud has no camera, so an experiment world and the same world watched on the phone will diverge. PRN-08's own example ("watch year 41 happen exactly as reported") and PLT-05 fail.
  - **Branches pick up noise.** Rewind, change nothing, look somewhere else, and you get a different history. Branch comparisons mix your intervention with your camera path.
  - **Experiments test the wrong world.** They run "without graphics". Either every mind runs in full detail, which tests a world you never play, or every mind runs in simple form. A mind made of "habits and group knowledge" may be unable to tinker its way to a new idea. Discoveries would then happen only where you look, and overnight mode, with nobody looking, would produce little.
  - **Sharpened minds lack a past.** When a simplified mind "sharpens", it needs personal memories of years nobody simulated person by person. Inventing them breaks PRN-10 ("never invents events"). Leaving them out breaks "no break in their story" and PRN-13 (every act explained).
  - **The director changes history.** Live moments pull your camera, which raises detail, which changes events, against TIM-03 ("never causes, changes or hides anything").
  - **Filled-in detail vs invented events.** PRN-10's own example ("people walking between shelters") shows something that wasn't simulated. The line between filled-in detail and an invented event is never drawn.
- **Question for you:** Should where you look ever change what happens?
  - (a) **No.** Detail is set by rules inside the world, for example every person always at full detail, and animals and plants detailed wherever people are. Looking only changes what's drawn. Phone, cloud and replays always agree. The phone carries more load, so history may run slower.
  - (b) **Detail follows world-computed "attention".** For example, full detail for anyone facing something new or dangerous, never set by the camera. Phone and cloud still agree.
  - (c) **Keep camera-driven detail** and record the camera path as part of history. Then PRN-08, PLT-05 and the meaning of experiment results must be rewritten.
  - Whichever you choose: can simplified minds make discoveries, and what do they remember when they sharpen?

### Section 4: The player as god

#### H2. Fortune has nothing defined to act on

- **Type:** hidden question
- **Severity:** High
- **Items:** GOD-04 (l. 446–451), GOD-10 (l. 459), MAT-06 (l. 694), PRN-13 (l. 267–268), SCP-17 (l. 388), WLD-13 (l. 608)
- **What's wrong:** "A blessing at most doubles a chance: a hunt with a 10% chance of success gets 20%." But nothing in this world stores a "chance of success". A hunt's outcome comes from bodies, senses and physics: "the physics decides the outcome" (MAT-06). The file never says what a blessing actually changes. Open points:
  - **Stacking.** You can bless a person, their family, their band and the place they hunt. Is that 2×, or 2×2×2×2 = 16×?
  - **High chances.** What does "doubles" do to a 70% chance?
  - **Curses.** A curse works "in reverse, at most halving a chance". Halving the chance of staying healthy (95% to 47%) is brutal. Doubling the chance of falling ill (5% to 10%) is mild. Which one is meant?
  - **Conflicts.** If you bless the hunters and also the herd they hunt, who wins?
  - **Strength.** GOD-10 lets you choose what to bless "and for how long", but not how strongly. Is every blessing the maximum?
  - **"Finding food or materials."** Materials sit where geology put them, and fine detail is "generated the same way every time" (WLD-13). So luck could only work by steering where people search. Steering a choice breaks PRN-13 (every choice comes from "their beliefs, drives and memories") and SCP-17 (no control, "not even briefly").
- **Why it matters:** Two teams would build different powers. One team re-rolls the world's chance events around the target, another nudges the target's own choices, a third overrides a computed success rate. The second breaks two principles. The others need a rule for measuring "at most doubles".
- **Question for you:** What does a blessing change?
  - (a) Only chance events in the world around the target, never their choices: whether the deer looks up, where a gust blows, whether a wound gets infected.
  - (b) A success rate the game estimates for each action and then shifts.
  - (c) Something else.

  Then settle four rules: stacking (one option is that the strongest blessing applies and the total never goes above 2×), what doubling and halving mean, whether you set the strength, and whether a blessing can be renewed forever (see M6).

### Section 5: Time and history

#### H3. No minimum speed for history, and no decision on what gives when the phone is too slow

- **Type:** hidden question
- **Severity:** High
- **Items:** TIM-01 (l. 483–492), TIM-07 (l. 541), TIM-12 (l. 520–524), PLT-04 (l. 1205–1212), PRN-11 (l. 287–291), MND-15 (l. 946), WLD-04 (l. 562), VIS-03 (l. 156), VIS-07 (l. 109), VIS-10 (l. 122), RSK-10 (l. 1364–1367)
- **What's wrong:** The file imagines speeds two orders of magnitude apart:
  - TIM-01: at world zoom, "centuries pass every minute", which is over 10,000 years an hour.
  - TIM-12's example: "312 years passed" in a whole night, about 40 years an hour.
  - RSK-10 lists "overnight runs covering only a few years" as a warning sign.

  PLT-04 sets targets for smoothness, start-up, battery and heat, but none for simulated years per real hour. TIM-07 says there is "no fixed target". Meanwhile, three items push the load up with no limit:
  - PRN-11 forbids cutting corners.
  - MND-15 forbids a population cap.
  - WLD-04 expects "around ten million" people once farming exists.

  Nothing says what happens when these collide.
- **Why it matters:** The vision needs thousands of years: VIS-07 promises watching "a thousand years of migrations … like weather", VIS-03 talks of stalls of "tens of thousands of years", and the camp wolf and two tongues need thousands of years too. At 40 years an hour, a thousand years takes 25 hours of running, and every "what if" branch (VIS-10) costs the same again. As population grows, history slows further, with no floor. RSK-10's responses (the director, overnight mode) can't help when the phone itself is the limit.
- **Question for you:** What is the slowest acceptable speed at world zoom, in years per real hour, both awake and overnight? If measurements fall short, what gives first?
  - (a) Accept slower history.
  - (b) A defined, stronger simplification for people nobody is watching (group-level minds), written into PRN-11 as "planned". This depends on H1.
  - (c) A cap on world size or population.
  - (d) Let worlds be pre-run in the cloud and downloaded. This changes SCP-02 and SCP-15.

### Section 8: People: bodies and lives

#### H4. What the first people know, remember and have is not defined

- **Type:** gap
- **Severity:** High
- **Items:** BIO-02 (l. 774–781), PRN-01 (l. 215–216), BIO-03 (l. 783), SCP-01 (l. 329), VIS-06 (l. 97), CUL-17 (l. 978), MND-18 (l. 875–881), MND-24 (l. 940), WLD-24 (l. 600), BIO-11 (l. 796–798)
- **What's wrong:** PRN-01 allows "no knowledge given at the start beyond the starting kit (`BIO-02`)", so the kit is the principle's only exception. But the kit lists headings, not contents:
  - **"Food: gathering, scavenging, some ambush hunting."** Gathering what? Do they know which local plants are food or poison, where the water is, which animals are dangerous, what each season brings?
  - **"A few dozen shared words and calls."** Which meanings, and are they the same in every world? VIS-06 says "a handful of words"; SCP-01, BIO-02 and CUL-17 say "a few dozen".
  - **"They can feed a fire found after lightning."** Do they also know to carry embers and keep a fire out of the rain? Does any band have a fire in year 0?
  - **Each person's starting state.** Memories, ages, skill levels and relationships are all unset. MND-18 and MND-24 say people remember "who's who, family, and who owes whom".
  - **The bands.** Are the 3–4 bands kin, neighbours or strangers, and do they share one language?
  - **The season history starts in.** With no clothing and no way to make fire, the first temperate winter may kill most bands.
- **Why it matters:** The start shapes Experiment 1 and every early history. Give people full local food knowledge and they thrive; give them none and they starve in year one. An engineer will pick something, and it will be the biggest single piece of knowledge handed to them in the whole game.
- **Question for you:** What exactly is in each head at year 0?
  - (a) A written list of starting knowledge for their home region, which becomes useless when they move.
  - (b) Nothing beyond the kit's words and abilities, accepting heavy early losses.
  - (c) A short hidden "pre-history" run before year 0, whose experiences become their starting memories, so that even the start is learned inside the world.

  Also fix one number for the starting vocabulary.

### Section 11: Presentation

#### H5. No rule for how the game recognises a discovery, skill, people, language or institution

- **Type:** gap
- **Severity:** High
- **Items:** TIM-02 (l. 503–509), PRE-08 (l. 1119), PRE-05 (l. 1111–1113), PRE-07 (l. 1117), TIM-13 (l. 533), CUL-04 (l. 975), CUL-06 (l. 996), CUL-23 (l. 1006), PRE-36 (l. 1137), RES-03 (l. 1264–1266), VIS-03 (l. 154), PRN-07 (l. 227–231), MND-06 (l. 901–904)
- **What's wrong:** Many features need the game to spot and name things that emerge:
  - TIM-02: "firsts: the first time anyone does something new" and "discoveries spreading or being lost".
  - PRE-08: "someone has made fire for the first time".
  - PRE-07: the overlay of "who knows which skill".
  - TIM-13: counts of "discoveries, languages, beliefs".
  - CUL-04: dialects that become "separate languages".
  - CUL-06: institutions as "named things … such as 'the rite of first fire'".
  - CUL-23: "The game recognises peoples".
  - PRE-05: a chronicle "organised as a book of ages", though VIS-03 says "There are no eras".
  - RES-03: Experiment 1's "Discovery: happens".

  Nothing defines how any of this recognition works. Skills are each person's own action sequences (MND-06), so even deciding that two people know "the same skill" needs a rule. Is a rediscovery after loss a "first"? PRN-07 keeps discovery names out of the logic people use to decide, but the part of the game that watches and names things is never specified.
- **Why it matters:** The chronicle, live moments, overlays, timeline comparisons and every experiment's pass or fail rest on this. A fixed list of known discoveries would miss exactly the inventions "our own history never took" (VIS-03). A fully general detector needs thresholds that nobody has chosen, such as how different two dialects must be to count as two languages.
- **Question for you:** How are things that emerge recognised and named?
  - (a) A written list of known patterns (fire-making, pottery and so on) plus a general "new technique" catch-all.
  - (b) General rules only, such as "a new combination of action, material and result" or "shared words or customs above a threshold", with English names composed by the writer AI from the data.
  - (c) Both.

  Also: who sets the thresholds, and what starts and ends an "age" in the chronicle?

### Section 13: Platform and performance

#### H6. After an update, the past may no longer be watchable

- **Type:** hidden question
- **Severity:** High
- **Items:** PLT-09 (l. 1222–1225), PRN-08 (l. 281–285), PRC-11 (l. 1327), PLT-05 (l. 1229), RES-15 (l. 1293), PRE-15 (l. 1133), MOM-07 (l. 144), TIM-06 (l. 528–531), RES-05 (l. 1257), RSK-12 (l. 1386–1389)
- **What's wrong:** PLT-09 keeps "everything that already happened" after an update, and runs any rewound branch "under the current rules". It doesn't say whether you can still watch a moment that was simulated under older rules. Replays rely on re-running a seed "under the same version of the rules" (PRN-08). Yet replays are everywhere:
  - report links "open the replays in the game" (RES-15);
  - experiment worlds open "at any moment of its history" (PLT-05);
  - PRN-08's example has you watch year 41 "exactly as reported";
  - MOM-07 promises "You tap it and see the hunt".

  PRC-11 ships a new version "whenever something you can see or try has changed", so most worlds will cross many update marks. Experiments also run in the cloud on one version while your phone may already have the next.
- **Why it matters:** If the app carries only the newest rules, a report's replay links break as soon as you install the next build. Every moment before an update can then be read about but not watched. Keeping every old rule version and storing full recordings are both big builds, and very different ones. RSK-12 rates update effects "impact low"; with frequent builds, the impact is not low.
- **Question for you:** What must stay watchable after an update?
  - (a) Every rules version stays inside the app, so any moment can be re-run exactly.
  - (b) The past is stored as a recording detailed enough to watch, at a storage cost (see M12).
  - (c) The past before an update can be browsed (chronicle, minds, archaeology) but not replayed.

  And do experiment worlds carry their rules version, so the phone can open them exactly?

### Section 14: Research and validation

#### H7. Signature moments have no pass bar, and nothing re-checks them later

- **Type:** gap, unverifiable check
- **Severity:** High
- **Items:** VIS-12 (l. 136), MOM-01 to MOM-12 (l. 138–149), RES-07 (l. 1271–1280), RES-09 (l. 1243), PRC-10 (l. 1323–1325), MAT-15 (l. 719)
- **What's wrong:** Each signature moment "becomes a long-term test" (VIS-12), but the file never says what passing means. Is it the moment in at least 1 world of 100, or in half of them, and within how many years? RES-07 schedules each experiment once, at its milestone. PRC-10's checks before work joins the main version are the tests, the reality checklist, reproducibility and the general-rules check. Experiments aren't among them, and nothing says earlier experiments are run again at later milestones. The reality checklist is protected ("the whole checklist runs again", MAT-15); the signature moments are not.
- **Why it matters:** A change to minds at MIL-05 could quietly stop bands from discovering fire (MOM-01), and nothing would notice. "Long-term test" would really mean "tested once".
- **Question for you:** What is the pass bar for a signature moment, and must it keep passing?
  - (a) Re-run every passed experiment at each milestone; a failure blocks the milestone.
  - (b) Re-run a small sample of worlds on every change.
  - (c) Test once and accept the risk.

  Also set the default bar, for example "in at least N of 100 worlds within Y years".

#### H8. Experiment 1's criteria can't fail as written, and nothing stops re-running until it passes

- **Type:** unverifiable check
- **Severity:** High
- **Items:** RES-03 (l. 1263–1269), RES-09 (l. 1243), RES-02 (l. 1261), RSK-01 (l. 1337–1340), MIL-02 (l. 368), PRN-05 (l. 233–237), VIS-05 (l. 201)
- **What's wrong:**
  - **Words instead of numbers.** Discovery times "differ widely"; the skill is lost "noticeably more often" in small groups; without curiosity, discovery is "much rarer". Almost any result can be read as a pass, which empties RES-09 ("never adjusted afterwards").
  - **Undefined terms.** "Discovery" and "can do it" have no definition. Is one lucky flake a discovery, or must someone make sharp flakes on purpose, repeatedly, and use them?
  - **No rule for failure.** If the team tunes the minds and re-runs on the same 100 seeds until the criteria pass, the result fits those worlds and nothing else. "Experiments that can fail" then mean nothing.
  - **No use for a sharp edge.** MIL-02 gives "just enough food and terrain to live on". With no carcasses or hides to cut, a sharp edge is useless, and Experiment 1 tests curiosity in a vacuum.
  - **Nothing to compare against.** A valley of 3–4 small bands may contain no "large, connected" groups for the Loss criterion.
- **Why it matters:** Experiment 1 is the project's first proof (RSK-01). If its bar can't fail, the research rigour of VIS-05 is decoration from day one.
- **Question for you:** Put a number on every criterion, and define "discovery" and "can do it". Then choose a re-run rule:
  - (a) Tune on a development set of seeds, then confirm once on fresh seeds never used before, with the same criteria.
  - (b) Allow any number of attempts, but report every attempt and every change.
  - (c) After a set number of failed attempts, stop and ask you.

  And what must Experiment 1's world contain so that a sharp edge is actually useful?

---

## Medium

### Section 1: Vision

#### M1. The build order is a hidden tech tree

- **Type:** contradiction
- **Severity:** Medium
- **Items:** VIS-03 (l. 153–156), PRN-09 (l. 295–299), MAT-16 (l. 721–728), MAT-07 (l. 692), PLT-09 (l. 1222–1225)
- **What's wrong:** VIS-03 says "Any fixed sequence of eras would be a tech tree in disguise" and "Nothing about the order of our history is guaranteed, except where physics forces it." But the physics only allows what has been built, and MAT-16 builds it in our history's order: stone, fire, food, fibres and hides, clay and pigments, then metals and glass, each added "as experiments call for them". MAT-07 says inventions arrive "in a natural order that nobody wrote down"; MAT-16 writes that order down.
- **Why it matters:** Worlds are long-lived (PLT-09). A band in today's build that puts clay in its fire gets nothing, because of our schedule rather than their culture. The chronicle will record a stall that is ours, not theirs, and new possibilities will appear suddenly at update marks.
- **Question for you:** How should a world treat the edge of what's built?
  - (a) Mark it, so you can tell a stall is ours.
  - (b) Build the near-term layers (1–6) before the whole world (MIL-06), so long-running worlds aren't capped by the schedule.
  - (c) Accept it as a temporary limit.

#### M2. Several success checks can't fail as written

- **Type:** unverifiable check
- **Severity:** Medium
- **Items:** VIS-14 (l. 185–191), VIS-05 (l. 200–203), VIS-15 (l. 193–198), PRE-31 (l. 1083–1088), PLT-04 (l. 1212)
- **What's wrong:**
  - VIS-14's limits are "set from the measurements in `PLT-04`", so the target becomes whatever the phone manages, and "comfortable for battery and heat" has no number.
  - VIS-05's "studio craft" has no Done-when at all.
  - VIS-15's "clearly different stories" has no measure.
  - PRE-31's screenshots must "pass a review", but it doesn't say whose review, or against what reference.
- **Why it matters:** Both success items (VIS-14, VIS-15) and the craft half of the quality bar can't fail, so they can't steer anything.
- **Question for you:** Who judges, and against what?
  - (a) You judge on the phone, with a short fixed checklist.
  - (b) Numbers set now, for example battery use per hour, a maximum skin temperature, dropped frames per minute.
  - (c) Reference screenshots or games that you pick as the bar.

### Section 2: Principles

#### M3. "Real numbers" where no measurement exists

- **Type:** hidden question
- **Severity:** Medium
- **Items:** PRN-05 (l. 233–237), MAT-03 (l. 673–679), MAT-05 (l. 704–706), BIO-08 (l. 785), WLD-05 (l. 628), GOD-04 (l. 448)
- **What's wrong:** PRN-05 says "Every quantity in the world comes from real-world measurements", and its check says "every value names its real-world source". Many values have no measurement:
  - how fast a belief's certainty moves;
  - how fast memories fade in this model;
  - how strong curiosity is, and its "real-world spreads" (BIO-08);
  - the properties of mixtures nobody has measured (MAT-03 derives them, but from what model?);
  - scaled weather (WLD-05);
  - design numbers such as fortune's "at most doubles".
- **Why it matters:** One team will stall waiting for data that doesn't exist; another will invent values and call them sourced. Either way the check means less than it says.
- **Question for you:** What counts as a source?
  - (a) Measured values only; anything else blocks work.
  - (b) Published models and fitted values also count, labelled as such.
  - (c) Chosen design values are allowed if labelled "chosen" and listed in every milestone report, so you can see them.

#### M4. The general-rules check can pass while recipes hide in numbers

- **Type:** unverifiable check
- **Severity:** Medium
- **Items:** PRN-07 (l. 227–231), glossary "General-rules check" (l. 1425), RSK-07 (l. 1342–1345)
- **What's wrong:** The automated part of the check searches the decision logic for "discovery vocabulary". A rule like "if hardness is above X and it breaks like glass, strike it at angle Y" contains no forbidden word, yet it is a recipe for knapping. A rule could also test a material's identity number directly. The vocabulary list itself isn't defined. The second half of the check ("reviews flag any rule that applies to only one material") is manual and has no method.
- **Why it matters:** PRN-07 is the heart of "no recipes". A check that only scans for words gives false comfort, and that is exactly how knowledge leaks in (RSK-07).
- **Question for you:** What else must the check prove?
  - (a) Decision logic may read only perceived properties, never a material's or species' identity.
  - (b) A "swap test": swapping or renaming a material's identity changes no behaviour.
  - (c) Every law must be shown to apply to at least two real materials in the catalogue.

### Section 3: Scope and non-goals

#### M5. Milestones don't contain what their experiments need

- **Type:** contradiction
- **Severity:** Medium
- **Items:** SCP-16 and MIL-01 to MIL-07 (l. 365–373), RES-07 (l. 1271–1280), MAT-16 (l. 721–728), SND-05 (l. 1171), MOM-02 (l. 139), MOM-05 (l. 142), MOM-07 (l. 144), MOM-09 (l. 146), GOD-04 (l. 449), SCP-01 (l. 330)
- **What's wrong:**
  - **Disease.** MOM-02 ("A fever kills a band's best stoneworkers") is tested at MIL-04, and fortune's plague curse arrives at MIL-04, but no milestone includes disease or microbes.
  - **Seas.** MOM-05 ("separated by a rising sea") is tested at MIL-05, but seas, the whole world and migrations arrive at MIL-06.
  - **Matter layers.** MAT-16 says "each milestone adds a layer", but layers 4 (fibres, hides, joining) and 5 (clay, lime, pigments) match no milestone's theme.
  - **Clothing.** Clothing is one of SCP-01's "great early discoveries" but has no milestone, even though hard winters are part of MIL-03's fire test.
  - **Pigments.** MOM-07's painting needs pigments (layer 5), which no milestone owns.
  - **Sound.** SND-05 puts voices at MIL-05, but MIL-05 doesn't list them. "Their music and the score after that" has no milestone.
  - **The dig.** MOM-09 is a dig "under a village", but villages arrive at MIL-07 and archaeology at MIL-06.
- **Why it matters:** Experiments will either be scheduled before the systems they test, or those systems will be added with no milestone owning them.
- **Question for you:** Which milestone owns disease, seas and sea level, clothing (layer 4), pigments (layer 5), voices and music? Should MOM-05 move to MIL-06?

### Section 4: The player as god

#### M6. Repeating a natural act isn't natural, but nothing limits it

- **Type:** ambiguity
- **Severity:** Medium
- **Items:** GOD-05 (l. 409–417), GOD-02 (l. 426), GOD-04 (l. 448), PRN-03 (l. 247–251)
- **What's wrong:** GOD-05 judges each act on its own ("There is no limited supply of power to spend"). PRN-03's check is "every power produces only events the world could produce on its own". But a series can be unnatural even when each event in it is natural:
  - Pushing a "dry year over a region" every year for a century is "changing the climate directly", which GOD-02 says is "Not included".
  - A blessing lasts "from a single hunt to a few years", but can simply be renewed forever.
  - One lightning strike on a tree is natural; one every day for a year is not.
- **Why it matters:** Without a rule, you can steer history by repetition, and the promise that every achievement stays theirs quietly erodes.
- **Question for you:** Is each act judged alone, or the series?
  - (a) Each act alone. This is the current text: anything can be repeated.
  - (b) The series must stay within what the climate and chance produce in their worst stretches, for example no more hard winters per decade than the climate's worst decade.
  - (c) Simple cool-downs per place and per person.

#### M7. Dreams have no strength limit, and two of their feelings don't exist in minds

- **Type:** gap, contradiction
- **Severity:** Medium
- **Items:** GOD-03 (l. 430–439), MND-12 (l. 931), MND-19 (l. 889), GOD-05 (l. 414), RES-07 (l. 1274)
- **What's wrong:**
  - **No cap.** Fortune is capped ("at most doubles"). Dreams only make "certain ideas more likely to come to mind", with no cap. RES-07's MIL-03 experiment must show "a dream raises the odds without guaranteeing anything", so the size of that effect is a real design number.
  - **Replacement.** It isn't said whether your dream replaces that night's natural dream (MND-12).
  - **Feelings.** GOD-03 lets a dream carry "fear, longing, hope or awe". MND-19's feelings are "Fear, anger, joy, grief, disgust, surprise, affection, shame, pride and awe": no longing and no hope.
- **Why it matters:** A dream's strength decides whether dreams feel useless or like a cheat, and whether the MIL-03 test means anything.
- **Question for you:** How strong can a sent dream be?
  - (a) No stronger than the strongest natural dream.
  - (b) A fixed cap, like fortune's.
  - (c) Set by experiment.

  Does your dream replace the natural one? And should longing and hope be added to MND-19, or should GOD-03's list change?

#### M8. Who sets the size of a disaster, and how fast can you bring a storm?

- **Type:** hidden question
- **Severity:** Medium
- **Items:** GOD-02 (l. 421–428), GOD-05 (l. 410–413), GOD-10 (l. 457–460), WLD-22 (l. 647), WLD-15 (l. 618)
- **What's wrong:**
  - **Size.** GOD-02's own note ("If an eruption you trigger is big enough to cool the world") implies you might choose the size. The file never says who decides how big an eruption, quake, flood or hard winter is, or how strong a seasonal push is.
  - **Arrival.** "To strike a tree you first need a storm overhead, which you can bring." Does the storm form over days through the weather physics, or appear at once?
  - **Stored stress.** Does a quake you trigger release stress that would have caused a natural quake later (moving that event), or does it add an extra one?
- **Why it matters:** These decide how powerful you are. "Nothing happens faster than nature could make it happen" (GOD-10) gives no measure of nature's speed.
- **Question for you:** For each power: who sets its size (you within limits, or nature with you choosing only the timing)? How long does it take to arrive? Do disasters you trigger use up nature's own?

#### M9. Branch comparisons and "what changed because of it" may show mostly chance

- **Type:** hidden question
- **Severity:** Medium
- **Items:** GOD-09 (l. 472–475), GOD-08 (l. 468), TIM-13 (l. 533), MOM-10 (l. 147), RES-07 (l. 1274), PRN-08 (l. 281)
- **What's wrong:** In a detailed world run on chance, one small change, such as a dream, can reshuffle chance everywhere. Two branches then soon differ in valleys your dream never reached. "What changed because of it" (GOD-09) becomes everything, and a side-by-side comparison (TIM-13, MOM-10) shows chance, not your effect. Separately, GOD-08 records only "time, place and target". That isn't enough to replay a dream (its memories and feeling), a blessing (its strength and length) or a drawn region.
- **Why it matters:** "Compare what happens with and without it" is the payoff of MIL-03 and MOM-10. If any change scrambles the whole world, one comparison proves little.
- **Question for you:** Which of these do you want?
  - (a) Chance stays local, so a branch differs only where your intervention's effects actually spread.
  - (b) World-wide divergence is accepted, and GOD-09 shows only the chain of causes (memory, then choice, then act).

  Separately: should the record hold every detail of an intervention?

### Section 5: Time and history

#### M10. The director's rules are incomplete

- **Type:** ambiguity
- **Severity:** Medium
- **Items:** TIM-01 (l. 484), TIM-02 (l. 500–510), TIM-04 (l. 496), TIM-11 (l. 514), TIM-12 (l. 521), PRE-06 (l. 1115), PRE-08 (l. 1119), SND-10 (l. 1187)
- **What's wrong:**
  - **Priority.** Does the director still slow time when you've locked the speed (TIM-04), or in overnight mode ("runs at top speed")?
  - **Rate.** How far does it slow, and how often may it interrupt? With many bands, "births and deaths among the people you follow" and "firsts" could fire every few seconds at world zoom. Does importance scale with zoom?
  - **"Follow."** "The people you follow" (TIM-02, PRE-08, SND-10) suggests a list, but PRE-06 only defines following one person with the camera.
  - **Two lists.** TIM-02's list of what interrupts includes births, migrations, band changes and the consequences of your interventions. PRE-08's list ("firsts, deaths of people you follow, disasters, and big turns in history") doesn't.
- **Why it matters:** The director is the main defence against a world that is "real but dull" (RSK-03) and against constant interruptions. These are core decisions about how the game feels.
- **Question for you:** Settle four things:
  - how the director ranks against manual speed and overnight mode;
  - a maximum interruption rate, and whether importance scales with zoom;
  - what "follow" means: one person or a list;
  - one shared list of what interrupts.

#### M11. Units of time: world years or Earth years?

- **Type:** ambiguity
- **Severity:** Medium
- **Items:** TIM-14 (l. 537), WLD-06 (l. 566), BIO-04 (l. 814–820), BIO-09 (l. 789), RES-03 (l. 1264, 1266), RES-13 (l. 1251), PRE-35 (l. 1107)
- **What's wrong:**
  - **Calendar.** Dates count the world's own years (TIM-14), and WLD-06 lets a year run from 250 days of 18 hours to 500 days of 36 hours: from about half an Earth year to about two.
  - **Biology.** BIO-04's figures (weaning at 2–4 years, living into the 60s) are in Earth years.
  - **Criteria.** RES-03's "within 500 simulated years" and "within 50 simulated years" don't say which year is meant.
  - **Sleep.** Human body clocks run close to 24 hours; nothing says how people sleep on an 18- or 36-hour day.
- **Why it matters:** The same Experiment 1 bar is four times easier on a long-year world. "Age" on a person's card could mean two different things.
- **Question for you:** Are rules, criteria and ages in Earth time or world years?
  - (a) Earth time everywhere inside the simulation and in every criterion; world years only on screen.
  - (b) World years everywhere, with biology converted.

  And how do sleep and daily rhythm work on days that aren't 24 hours long?

#### M12. No limits or clean-up for saved worlds

- **Type:** gap
- **Severity:** Medium
- **Items:** TIM-06 (l. 529), TIM-08 (l. 535), PLT-07 (l. 1218), PLT-08 (l. 1220), PRE-09 (l. 1122), MAT-08 (l. 700)
- **What's wrong:** You can rewind to "any moment", keep "several worlds, each with its own tree of timelines", see who made every buried object, and export "the full record". Nothing says:
  - how large a world may grow;
  - whether worlds or branches can be deleted;
  - how exact "any moment" is (to the second, or to the nearest saved point);
  - what happens when the phone is nearly full.
- **Why it matters:** Worlds run for thousands of years with no ceiling. Without a rule, the first full phone decides for you, possibly by failing to save (PLT-07).
- **Question for you:** Can you delete worlds and branches? What happens near a full phone: warn, thin out old detail, or refuse to branch? How exact must "any moment" be?

### Section 6: World

#### M13. Which numbers shrink with the small world?

- **Type:** contradiction
- **Severity:** Medium
- **Items:** WLD-05 (l. 628), PRN-05 (l. 233–234), WLD-16 (l. 623), WLD-03 (l. 557–559), WLD-06 (l. 566), WLD-07 (l. 569)
- **What's wrong:** PRN-05 says every quantity is real. WLD-05 scales weather systems "to fit the world"; a real storm system is about half this world's circumference. Nothing says what else scales:
  - animal and bird migrations of 1,000 km or more on Earth;
  - ocean currents;
  - the speed of ice-age cycles;
  - how far the horizon is;
  - how fast the sun's angle changes as you walk (about 18° per 100 km here).
- **Why it matters:** Each team will scale a different list, and some choices change history. For example, a herd may be unable to migrate its real distance.
- **Question for you:**
  - (a) One rule: the planet's size is small, so everything set by distance (weather, currents, migrations) scales by the same factor, and everything local (bodies, chemistry, materials) stays real.
  - (b) Case by case, with each scaled quantity listed with its reason.

#### M14. The seam at the poles

- **Type:** gap
- **Severity:** Medium
- **Items:** WLD-01 (l. 551–553), WLD-02 (l. 555), WLD-16 (l. 625), WLD-25 (l. 630), WLD-07 (l. 569)
- **What's wrong:** The map wraps north to south, so the north and south poles are one 2,000 km line. Because the seasons are "reversed between the northern and southern halves", northern summer touches southern winter along that line. WLD-02 calls crossing it "rare and harmless", which assumes permanent ice. Warm periods (WLD-16) and people warming the world (WLD-25) could open it. The night sky also flips across the seam.
- **Why it matters:** Climate, sea ice, currents and animals will all meet this line, and an engineer will choose how it behaves.
- **Question for you:**
  - (a) The seam is always impassable ice, guaranteed by generation and climate.
  - (b) A blend zone where the seasons and the sky are averaged.
  - (c) The jump is allowed and simulated as it is.

#### M15. A "present-day" start makes the rising-sea moment nearly impossible

- **Type:** contradiction
- **Severity:** Medium
- **Items:** SCP-12 (l. 387), WLD-08 (l. 575), WLD-16 (l. 625), WLD-26 (l. 634), MOM-05 (l. 142), RES-07 (l. 1276), MIL-06 (l. 372)
- **What's wrong:**
  - **Start point.** Worlds start "in a realistic present-day state": a warm period, with seas high.
  - **Real cycles.** At real cycle lengths (PRN-05), the next big change is a slow fall in sea level over tens of thousands of years. A big rise would only come after the next ice age, roughly 100,000 years in. WLD-16's "over thousands of years" is faster than real cycles.
  - **The moment.** MOM-05 needs bands "separated by a rising sea", and RES-07 tests it at MIL-05, before seas arrive at MIL-06.
- **Why it matters:** A signature-moment test that can't realistically happen in the time available will either fail, or tempt someone to speed up the climate without saying so.
- **Question for you:**
  - (a) Worlds start at different points of the ice-age cycle, some late in an ice age when the seas are about to rise.
  - (b) Shorter cycles, labelled as a departure from real numbers.
  - (c) Keep everything real and treat MOM-05 as rare rather than a pass/fail test.

  Also move MOM-05's experiment to MIL-06 or later.

#### M16. What every world must contain, and which worlds experiments use

- **Type:** gap
- **Severity:** Medium
- **Items:** WLD-10 (l. 592–598), WLD-24 (l. 600), WLD-19 (l. 586–588), RES-01 (l. 1238), RES-03 (l. 1264), MOM-06 (l. 143), MOM-08 (l. 145), MOM-12 (l. 149)
- **What's wrong:** The start region needs "caves, fresh water and varied food within reach", but nothing guarantees:
  - stone that chips, near the start (Experiment 1 needs it);
  - a wolf-like animal (MOM-06);
  - wild grasses with big seeds (MOM-08);
  - green copper minerals (MOM-12).

  Also, the game "keeps the best" of many candidate worlds (WLD-10), while experiments run on "random worlds" (RES-01, RES-03). So experiments may test a different kind of world from the ones you play.
- **Why it matters:** Pass rates will partly measure geological luck, and results may not hold for your worlds.
- **Question for you:** Must every world contain what the signature moments need?
  - (a) Yes, as part of the world's score.
  - (b) No, and each test counts only worlds where the moment is physically possible.

  And should experiments use the same "keep the best" worlds you play?

#### M17. Which animals have minds, and when are they individuals?

- **Type:** ambiguity
- **Severity:** Medium
- **Items:** MND-16 (l. 952–954), WLD-12 (l. 606), WLD-23 (l. 590), BIO-19 (l. 844), PRN-13 (l. 267–268), GOD-04 (l. 447), GOD-12 (l. 441–444), MOM-06 (l. 143)
- **What's wrong:** MND-16 gives animals "the same kind of mind with fewer abilities", one that learns fear and routes, and PRN-13 says every animal's choice can be explained. But WLD-12 simulates animals "in patches of a few hundred metres" away from people, and WLD-23's animals include insects and shellfish. Nothing says:
  - which animals get minds;
  - whether a herd's learned fear survives when the herd drops to patch level;
  - how a wolf pup's inherited tameness (MOM-06) is tracked inside a patch.
- **Why it matters:** The hunting arms race and taming both need animals that remember, as individuals or as herds.
- **Question for you:**
  - (a) Mammals and birds are always individuals with minds; other groups are simulated as populations.
  - (b) All animals are populations away from people and individuals near them, with learned habits carried over as herd traits.
  - (c) Something else.

### Section 7: Matter and physics

#### M18. The starting law list names products, and "real chemistry" is undefined

- **Type:** contradiction
- **Severity:** Medium
- **Items:** MAT-04 (l. 685–690), PRN-07 (l. 227–231), VIS-13 (l. 163), MAT-07 (l. 692), section 7 intro (l. 653), RCK-06 (l. 741), RCK-19 (l. 763), RCK-20 (l. 764)
- **What's wrong:**
  - **Product laws.** MAT-04 says "No law ever names a product", then lists "burning lime, setting of mortar, tanning, glass-making". As written, these are product laws that each fit one material, which is exactly what PRN-07's review is meant to flag.
  - **Two depths.** VIS-13 promises "Real chemistry"; section 7 promises "a few dozen general laws". These are different depths.
  - **Invisible products.** The reality checks name products the simulation can't see. RCK-06's "become leather" needs a test by properties, such as "stops rotting, stays supple".
- **Why it matters:** A "tanning law" written to pass RCK-06 is a recipe in disguise. The depth of chemistry decides what can ever be discovered.
- **Question for you:** Are these placeholder names for general laws? For example, "plant tannins bind to proteins" would work on any protein, and "a flux lowers the melting point of silica" would make both glass and glazes. Then choose a depth for chemistry:
  - (a) Real reactions between real compounds, with their real energies.
  - (b) Property-level laws (heat plus a class of mineral gives a change).
  - (c) A mix, stated for each law.

  And is every reality-check result judged by properties?

#### M19. No law for floating, flowing water and air, or musical sound

- **Type:** gap
- **Severity:** Medium
- **Items:** MAT-04 (l. 685–690), WLD-06 (l. 566), RCK-08 (l. 743), CUL-10 (l. 1014), SND-06 (l. 1175–1177), MOM-05 (l. 142)
- **What's wrong:**
  - **Floating.** Half to three quarters of each world is sea (WLD-06: "25–50% land"), yet nothing in the file mentions floating, rafts or boats, and no law covers buoyancy.
  - **Moving water and air.** No law covers them, yet RCK-08 needs "forced air", and drying in wind and carrying things by river need them too.
  - **Musical sound.** CUL-10 says instruments "follow real acoustics", but acoustics exists only as a property of struck things (MAT-03), not as a law for air in a flute or a stretched hide.
- **Why it matters:** Crossing water shapes migration and how peoples meet; MOM-05's descendants "meet again". If floating isn't in the physics, nobody can ever discover it, and islands become prisons.
- **Question for you:** Should buoyancy, the flow of water and air, and vibration be added to the starting law list, with checks such as "a dry log floats, a stone sinks" and "blowing on embers makes them hotter"? At which milestone?

### Section 8: People: bodies and lives

#### M20. Life figures: programmed rates, or results to check?

- **Type:** ambiguity
- **Severity:** Medium
- **Items:** BIO-04 (l. 814–820), BIO-14 (l. 810), BIO-15 (l. 822), RES-14 (l. 1253)
- **What's wrong:** BIO-04 gives "Typical figures": four in ten children die before 15, and a birth every 3–4 years. If these are inputs, the game sets death rates directly, which BIO-14 forbids ("Nobody dies of random chance"). If they are results, they are targets that the causes must reproduce, and no tolerance is given.
- **Why it matters:** One team will program the rates: fast, realistic-looking, and against BIO-14. Another will build only the causes and hope. The results will differ widely.
- **Question for you:**
  - (a) Results only: the figures become reality checks with a stated tolerance.
  - (b) Some are inputs (such as menopause age and pregnancy length) and the rest are results. Which ones?

#### M21. Mating and reproduction: what is inborn?

- **Type:** hidden question
- **Severity:** Medium
- **Items:** MND-07 (l. 887), BIO-15 (l. 822), BIO-17 (l. 828–830), MND-21 (l. 893–895), PRN-01 (l. 215–216), BIO-03 (l. 783)
- **What's wrong:**
  - **The drive.** It is "the urge to have children" (MND-07). Real people have sexual desire. A drive aimed at children assumes they know where babies come from, which PRN-01 says they must learn.
  - **Minds.** BIO-17 rules out inborn role differences for bodies but says nothing about minds. Are drives and temperament the same on average for both sexes?
  - **Incest avoidance.** It is an inborn tendency in real people but is missing from MND-21, and there are only 45–120 founders.
- **Why it matters:** These choices shape family life, population growth and inherited disease. They are sensitive, and they should be yours, not an engineer's.
- **Question for you:**
  - The drive: (a) sexual desire, with the link to children learned; (b) keep "the urge to have children" as a simplification.
  - Minds: (a) no inborn average differences between the sexes; (b) average differences from research, with wide overlap.
  - Incest avoidance: add it or not?

### Section 9: Minds

#### M22. "Belief" is defined too narrowly for what beliefs must do

- **Type:** contradiction
- **Severity:** Medium
- **Items:** glossary "Belief" (l. 1420), MND-05 (l. 870–873), CUL-05 (l. 985–987), CUL-06 (l. 996), CUL-20 (l. 992), MND-23 (l. 935–938), PRN-13 (l. 270), PRE-14 (l. 1131)
- **What's wrong:** The glossary defines a belief as "a person's idea of what causes what". But the file also needs four other kinds of belief:
  - that something exists: "an unseen being" (CUL-05);
  - about other minds: "she thinks I don't know" (MND-23);
  - rules: "what not to eat" (CUL-20), and norms people "enforce" (CUL-06);
  - plain facts: "the river is safe at dawn" (PRN-13).
- **Why it matters:** If a belief can only be cause and effect, gods, taboos and norms have nowhere to live, or each team invents a different home for them.
- **Question for you:**
  - (a) Cause and effect only, with gods and rules written as causal claims.
  - (b) Several kinds of belief (cause and effect, existence, other minds, rules), each with a certainty and its evidence.

  Then update the glossary to match.

#### M23. Moving, voice, gesture and talk are undefined

- **Type:** gap
- **Severity:** Medium
- **Items:** MAT-12 (l. 698), CUL-04 (l. 975), CUL-01 (l. 963), MND-23 (l. 937), GOD-03 (l. 437), SND-03 (l. 1165), MOM-04 (l. 141), CUL-10 (l. 1014)
- **What's wrong:** MAT-12 lists "the basic actions everything else is built from", but leaves out walking, running, climbing, swimming, making sounds, singing, pointing and gesturing. No item says:
  - what people can tell each other;
  - how a listener weighs being told against their own experience;
  - how children pick up words.

  Yet much depends on it: teaching, gossip and deception (MND-23), telling a dream (GOD-03), the hunting song (MOM-04), dance (CUL-10) and "actual sentences in their language" (SND-03).
- **Why it matters:** Whether swimming is inborn or learned decides whether rivers are barriers. How much people trust what they're told decides how fast knowledge spreads, and spread is half of Experiment 1.
- **Question for you:** Which of these actions are inborn, and which must be learned? What can be said in words, and how is being told weighed against seeing for yourself?

### Section 10: Culture and society

#### M24. Names before there is a language to name with

- **Type:** gap
- **Severity:** Medium
- **Items:** CUL-18 (l. 981), BIO-02 (l. 775), VIS-11 (l. 130), PRE-35 (l. 1107), PRE-16 (l. 1135), CUL-23 (l. 1006), CUL-17 (l. 978), SCP-20 (l. 391)
- **What's wrong:** People are "named in their own languages", and cards show "a person's name". But people start with a few dozen words, and the file doesn't say whether personal names are among them. It also doesn't say:
  - what the game calls a person, a place, a species (in the bestiary) or a people that has no name yet (CUL-23 "names them by what they call themselves");
  - where the starting words' sounds come from: created fresh for each world, or the same everywhere?
- **Why it matters:** "Ama" appears on page one, and the first build has to call people something.
- **Question for you:**
  - (a) Personal names are part of the starting kit, made from each world's own sounds.
  - (b) Until people name someone, the game uses plain descriptions, such as "the tall woman of the river band".
  - (c) The game invents names in the people's own sounds, marked as the game's.

#### M25. Is the artwork itself simulated?

- **Type:** hidden question
- **Severity:** Medium
- **Items:** CUL-09 (l. 1012), CUL-10 (l. 1014), CUL-14 (l. 1022), PRE-15 (l. 1133), PRE-12 (l. 1129), SND-02 (l. 1167), MOM-07 (l. 144), PRN-10 (l. 262)
- **What's wrong:** A painting is "composed from their own memories and myths" and must look like something. Does the simulation produce its actual marks (shapes made from the painter's memory, skill and pigments)? Or does the game draw a picture of what the painting "depicts"? The same question applies to:
  - songs: does the simulation produce actual melodies (SND-02)?
  - "the maps they draw": objects they make, or the game's drawing of their mental map?

  "Tap a painting or carving to see the event or myth it depicts" could show the painter's memory, a replay of the real event, or the myth version.
- **Why it matters:** Simulated marks are a large system; a drawn stand-in risks breaking PRN-10 ("nothing is faked"). These lead to very different builds.
- **Question for you:**
  - (a) The simulation makes the real marks and notes, and the game shows exactly those.
  - (b) The simulation stores what is depicted, and the game draws it in that culture's style.
  - (c) A mix.

  And what does tapping a painting show?

### Section 11: Presentation

#### M26. "Story view" and "scientist's view" mean two different things

- **Type:** ambiguity, contradiction
- **Severity:** Medium
- **Items:** PRE-14 (l. 1131), glossary (l. 1437), GOD-07 (l. 470), GOD-09 (l. 473), PRN-04 (l. 259), PRE-35 (l. 1107), TIM-02 (l. 509)
- **What's wrong:**
  - **Two meanings.** PRE-14 and the glossary define both views as "the two ways to look into a mind". But GOD-07 says "The story view never shows where you intervened", and GOD-09 says the scientist's view "shows where and when you intervened": places and times, not minds. PRN-04 uses the scientist's view for everything the simulation tracks, including soils.
  - **A leak.** TIM-02 makes "the consequences of your own interventions" a reason for a live moment, which appears in the normal view and gives your hand away.
- **Why it matters:** One team will build a panel for each person; another will build a raw-data mode for the whole game. Where interventions are hidden stays unclear.
- **Question for you:** Is the scientist's view a whole-game mode, or a panel for each mind? Exactly which screens must hide your interventions: the chronicle, overlays, live moments, the overnight summary, the score? Can a live moment be triggered because you intervened without revealing that you did?

#### M27. The content setting is undefined, and nudity and sex aren't covered

- **Type:** gap
- **Severity:** Medium
- **Items:** PRE-18 (l. 1153), CUL-08 (l. 1004), BIO-02 (l. 778), PRE-27 (l. 1067), BIO-15 (l. 822), PRN-04 (l. 255–256), PRE-17 (l. 1143)
- **What's wrong:**
  - **Levels.** PRE-18 lets you choose "how much of history's darker side is shown", but defines no levels and doesn't say what each would hide: pictures, chronicle text, live moments or sounds.
  - **Nudity and sex.** People start with "Clothing: none", and figures are big enough for "a face, hair, clothing and gestures", but nudity and sex are never mentioned.
  - **Refusals.** The writer AI on the phone may refuse or soften descriptions of violence, sacrifice or slavery, which would quietly break PRE-17 ("Descriptions stick to the data").
- **Why it matters:** Otherwise an engineer picks the levels, and the writer AI's own limits decide the rest. Hiding content also sits uneasily with PRN-04 ("If the simulation knows it, you can see it").
- **Question for you:** What are the levels, and what does each one hide? How are nudity and sex shown? When the writer AI won't describe something, what do you see: plain factual text, or a marker?

#### M28. The main gesture needs two hands, and gestures collide

- **Type:** contradiction
- **Severity:** Medium
- **Items:** VIS-14 (l. 191), PRE-34 (l. 1097), PRE-33 (l. 1099–1105), GOD-10 (l. 457), TIM-01 (l. 484)
- **What's wrong:**
  - **Two hands.** "Every screen works one-handed in portrait", yet zoom, which also sets the speed of time, is "pinch to zoom", and turning is "twist with two fingers".
  - **Collisions.** GOD-10's "draw around an area" collides with "drag to move", and "swipe up for views" collides with dragging the map upward.
- **Why it matters:** As specified, the core control fails a success check, and an engineer will invent the fixes.
- **Question for you:** What is the one-handed zoom: double-tap and drag, or a thumb slider? How does a drag become "draw an area", for example as a mode entered by long-press? How does "swipe up" differ from panning?

### Section 13: Platform and performance

#### M29. When this phone is replaced

- **Type:** hidden question
- **Severity:** Medium
- **Items:** PLT-01 (l. 1195), SCP-02 (l. 349), PLT-08 (l. 1220), VIS-03 (l. 153)
- **What's wrong:** The game is built for one phone and is "free to use that phone's specific hardware wherever it helps", with "no support for other phones". Yet PLT-08 lets you import a world "on the same phone or a new one". The project has no ceiling and will outlive one handset. Does "a new one" mean a different model, which would need support, or another phone of the same model?
- **Why it matters:** Leaning on one model's hardware now makes moving later expensive or impossible, and your worlds go with it.
- **Question for you:**
  - (a) Moving to a future phone is a goal, so features tied to this phone's hardware need a fallback.
  - (b) "A new one" means another Pixel 11 Pro XL, and moving to a new model would be a separate future project.

#### M30. Checks that must run on the phone depend on you

- **Type:** gap
- **Severity:** Medium
- **Items:** RES-05 (l. 1257), PRC-10 (l. 1323–1325), PRN-08 (l. 285), PLT-04 (l. 1205–1212), PRE-31 (l. 1084), PRC-02 (l. 1303), RSK-04 (l. 1381–1384)
- **What's wrong:** Reproducibility "on the phone and in the cloud" must pass "before any work joins the main version" and is "Checked from the first build". Performance is "Measured from the first build", and visual reviews need screenshots. The AI agents can't reach your phone. Nothing says who runs these checks on the real device, how often, or how the results come back.
- **Why it matters:** An emulator won't show the chip differences RSK-04 is about. Without one, every merge waits on you, which contradicts PRC-02's light role for you.
- **Question for you:**
  - (a) Merges rely on cloud and emulator checks; at each milestone you run a phone check build and send back its results.
  - (b) You run a check build after each version you install.
  - (c) A cheap second phone of the same model, used for automatic testing.

### Section 16: Risks

#### M31. Big risks missing from the register

- **Type:** gap
- **Severity:** Medium
- **Items:** section 16 (l. 1331–1401), RSK-01 (l. 1338), RSK-08 (l. 1359–1362), SCP-15 (l. 360–363)
- **What's wrong:** RSK-01 covers only discoveries ("The minds and physics might not produce discoveries often enough"). The register has no entry for:
  - language, grammar or belief failing to emerge (MIL-05 and SND-03's spoken voices depend on them);
  - detail that follows where you look breaking reproducibility (H1);
  - the size of saved worlds (M12);
  - experiments too big for the AI's cloud sessions (SCP-15): 100 worlds, each run for hundreds of years;
  - the phone ageing or being replaced (M29);
  - the writer AI on the phone being too slow or too large to run at all, rather than too plain (RSK-08);
  - your own time for reviews and phone checks (M30).
- **Why it matters:** The register is reviewed at every milestone. Risks missing from it won't be watched.
- **Question for you:** Should these be added, each with likelihood, impact, signs and response?

---

## Low

### Section 1: Vision

#### L1. Check-ins with nothing new to catch up on

- **Type:** contradiction
- **Severity:** Low
- **Items:** VIS-10 (l. 121–124), TIM-05 (l. 518), VIS-11 (l. 128)
- **What's wrong:** A check-in means you "catch up on the latest live moments", but "Nothing happens while you're away", so nothing new is waiting unless overnight mode ran.
- **Question for you:** When you open the app, should time start running toward the next moment (TIM-11), or should it show the moments you missed last time?

### Section 2: Principles

#### L2. What happens when the writer AI's text fails its check

- **Type:** gap
- **Severity:** Low
- **Items:** PRN-06 (l. 277), PRE-17 (l. 1143), PRE-37 (l. 1145)
- **What's wrong:** PRN-06's check says descriptions "are checked against the data they came from". It doesn't say what does the checking, or what you see when a description fails. It also doesn't say whether a chronicle entry is written once and kept, so you always read the same words, or rewritten each time you open it.
- **Question for you:** When text fails, should the game show plain factual text, try again, or flag it? Is text written once and stored?

#### L3. Can you run experiments, and can an experiment world become a played one?

- **Type:** ambiguity
- **Severity:** Low
- **Items:** PRN-12 (l. 242), PLT-05 (l. 1229), SCP-02 (l. 349), PRC-02 (l. 1303)
- **What's wrong:** PRN-12's example says "you run an experiment with the evolution dial turned up". But PRC-02 leaves experiments to the AI, and SCP-02 says the cloud is "not a way to play". If you open an experiment world on the phone and keep intervening, is it still labelled as bending the rules?
- **Question for you:** How do you ask for an experiment? Do a world's dial settings stay with it forever and show on screen?

### Section 3: Scope and non-goals

#### L4. "Ancestral minds" can't change within play

- **Type:** contradiction
- **Severity:** Low
- **Items:** SCP-14 (l. 333–341), BIO-06 (l. 838), BIO-07 (l. 840), PRN-12 (l. 240)
- **What's wrong:** This starting point needs brains "whose abilities must evolve over many generations". But at real rates, "Minds barely change over thousands of years", and the speed-up dial is for experiments only. SCP-14 also says other starting points "cost little", which isn't true for minds that must evolve.
- **Question for you:** Is "ancestral minds" a starting point for experiments only?

### Section 4: The player as god

#### L5. Does time stop while you choose a power?

- **Type:** gap
- **Severity:** Low
- **Items:** GOD-10 (l. 455–460), GOD-03 (l. 431), TIM-01 (l. 487)
- **What's wrong:** At camp zoom, a day passes in a few minutes, and choosing memories for a dream may take longer than the sleeper's night. Nothing says whether time pauses while a power's menu is open, or whether you can schedule a power ("tonight", "next winter").
- **Question for you:** Should time pause while you choose? Should scheduling be allowed?

### Section 6: World

#### L6. How a new world is created

- **Type:** gap
- **Severity:** Low
- **Items:** TIM-09 (l. 543), WLD-10 (l. 592–593), WLD-11 (l. 602), SCP-14 (l. 333), WLD-06 (l. 566)
- **What's wrong:** You can "start a new world", but nothing says what you choose (seed, starting point, planet ranges), or whether you see the candidates. WLD-11's arithmetic also doesn't hold: a dozen candidates at "under a minute" each could take up to 12 minutes, not "a few minutes".
- **Question for you:** Is creation fully automatic, or do you pick from the best few? Can you enter a seed? What is the real time target?

### Section 7: Matter and physics

#### L7. Reality checks have no tolerances and few "must not" cases

- **Type:** unverifiable check
- **Severity:** Low
- **Items:** RCK-01 to RCK-20 (l. 736–764), MAT-15 (l. 719), RSK-06 (l. 1347–1350)
- **What's wrong:** "Cooking makes food more nourishing" would pass with a 0.1% gain. Only a few checks say what must not happen (granite doesn't chip; no copper over a campfire). RSK-06's warning sign, "things burning that shouldn't", has no checks behind it.
- **Question for you:** Should each check get a real-world range and a "must not" partner, such as "green wood doesn't light by friction" or "wet tinder doesn't catch"?

### Section 9: Minds

#### L8. The inborn fear of snakes names a group of species

- **Type:** contradiction
- **Severity:** Low
- **Items:** MND-21 (l. 894), PRN-07 (l. 228, 231)
- **What's wrong:** PRN-07 forbids any rule "written for one particular … species", but MND-21 gives a "quicker fear of snakes".
- **Question for you:** Should it be described by what is seen (long, legless, moves suddenly), so that it applies to anything that looks that way and passes the check?

### Section 11: Presentation

#### L9. Hand-picked colours for materials nobody planned

- **Type:** contradiction
- **Severity:** Low
- **Items:** PRE-20 (l. 1039–1041), MAT-03 (l. 678), PRN-14 (l. 302), MAT-14 (l. 714–717)
- **What's wrong:** Every material gets "a short, hand-picked ladder of shades". But MAT-03 says colour is derived from what a thing is made of, and materials nobody planned (an alloy, a glaze, a dye mix) will appear.
- **Question for you:** Should colour ladders be made automatically from the simulated colour, with hand-picked ladders used only as overrides?

#### L10. Date and speed on screen

- **Type:** contradiction
- **Severity:** Low
- **Items:** PRE-32 (l. 1093), PRE-33 (l. 1105), TIM-01 (l. 484), PRN-11 (l. 288)
- **What's wrong:** "Nothing stays on screen unless you called it up", yet PRE-33 has "a small corner control for time". Speed changes with zoom, with the director and with phone load (PRN-11 slows time without telling you), so you can't tell how fast history is moving, or what year it is.
- **Question for you:** Is the corner control always visible? Should the date and the true speed always show (small), or appear only when you touch the screen?

### Section 12: Sound

#### L11. Two places where "nothing is faked" bends

- **Type:** contradiction
- **Severity:** Low
- **Items:** SND-06 (l. 1176), PRN-10 (l. 261–262), section 12 intro (l. 1157), WLD-02 (l. 555), WLD-01 (l. 552)
- **What's wrong:** Birdsong "can use recordings". Those are recordings of real Earth species, not this world's birds, and might play where no bird is simulated. Separately, the globe view must squash the map's polar regions, because on the map every line of latitude is 2,000 km long.
- **Question for you:** Accept both as labelled exceptions? Or should recorded birdsong play only where matching birds are simulated?

### Section 15: Project and process

#### L12. The coverage rule catches items that aren't work

- **Type:** gap
- **Severity:** Low
- **Items:** "IDs and links" rule 4 (l. 60), PRC-12 (l. 1329), PRC-05 (l. 1315), VIS-16 (l. 207), VIS-04 (l. 171), VIS-11 (l. 126)
- **What's wrong:** "Every ID that isn't *Dropped* must appear in at least one task". But some IDs are records, not work: "This file was written with you one section at a time", the name, the inspirations. The coverage check will flag them forever, or invite fake tasks.
- **Question for you:** Should such items be marked "record only" and left out of the coverage check?
