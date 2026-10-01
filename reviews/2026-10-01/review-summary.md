# Kindling: review of the project file

On 1 October 2026, four independent AI reviewers examined `PROJECT.md` as of commit `e87455a`. One looked at the file's structure. Two hunted for gaps, one section by section and one through scenarios. One judged feasibility. Their full reports are in this folder. This summary merges them into one list without duplicates, with a recommendation for each item.

Nothing in `PROJECT.md` has changed yet. Every change needs your OK (`PRC-07`).

## In short

- **About 160 findings came in:**
  - structure: 15;
  - gaps by section: 51;
  - gaps by scenario: 45;
  - feasibility: verdicts on 17 areas, 12 proposals, 7 risks and 10 prototypes.

  They merge into 21 decisions to make now, 14 that can wait for their milestone, 12 that tests will settle, and three packages: the file's own rules, plain corrections, and missing risks.
- **One contradiction above all.** All four reviewers found it independently: detail follows your camera, so looking changes history (`D1`).
- **Feasibility verdicts:**
  - 11 of 17 areas are feasible with stated limits;
  - 4 are doubtful: minds; culture and language; the pixel-3D look; experiments in the cloud sessions;
  - 2 are not feasible as written: the principles (the camera conflict, and "real numbers for everything"), and the speed of history.
- **The arithmetic of depth:**
  - If a full mind costs 50 ms of computing per simulated day (a middle estimate), the phone runs about 4 years of history an hour for 100 people, and about 63 years overnight.
  - "Centuries a minute" at world zoom would need about 1,500 times less per person.
  - Experiment 1 (3 setups × 100 worlds × 500 years) would take about 2 years of one 4-core cloud session at that cost, or about 80 days at a lean 5 ms.

  These are estimates; the first tests measure the real numbers (`T1`, `T6`).
- **The deepest risk is research, not engineering.** No published system shows small, general learners discovering crafts from raw properties within a few centuries of a few hundred lives. A cheap toy "Experiment 0" (`T2`) tests this before the engine is built.
- **The phone:**
  - **Chip:** Tensor G6, with 7 CPU cores and a PowerVR graphics chip.
  - **Memory:** each app gets about 8 GiB on the 12 GB model and 10 GiB on the 16 GB model.
  - **Heat:** under sustained load, the CPU settles near half its peak.

## How to use this

- **Decide now (`D1`–`D21`):** these shape the tests and the architecture. We go through them in rounds, then the three packages.
- **Decide before their milestone (`L1`–`L14`):** whenever you like. Nothing waits on them yet.
- **Settled by tests (`T1`–`T12`):** the tests measure, and you decide once the numbers are in.
- **Packages (`F`, `C`, `R`):** confirm all at once, or pick.

Sources are given as St (structure), Se (gaps by section), Sc (gaps by scenario) and Fe (feasibility), with their finding numbers. "Blocks" refers to `BUILDING-BLOCKS.md`.

---

## Decide now

### The simulation's foundations

**D1. Detail and the camera** · Se H1 · Sc H1 · Fe 3.1 · blocks X2
- **Problem:** `WLD-12`, `MND-14` and `PRN-11` give more detail wherever you look. A detailed mind doesn't behave exactly like a simple one, so looking changes history. That breaks "same seed, same history" (`PRN-08`) in three ways:
  - the cloud, where nobody looks, and the phone disagree;
  - branches mix your change with where you happened to look;
  - the director, by drawing your attention, changes events (`TIM-03`).
- **Options:**
  - (a) The world decides detail by its own rule, using only its state and your recorded interventions. The camera only changes the picture.
  - (b) Where you look is recorded like an intervention, and cloud runs use a fixed rule for attention.
- **Recommendation: (a).** All four reviewers recommend it. `WLD-12`, `WLD-13`, `MND-14` and `PRN-11` are reworded to match.

**D2. What every person is, everywhere** · Se H1, M17 · Sc H2 · Fe 3.2, 4.5, 4.8
- **Problem:** the file never says what a far-away person is.
  - Do they keep their own memories?
  - Can they discover anything? If not, a world watched from the globe, or run overnight, never invents.
  - Which animals are individuals?

  Feasibility: at any real population, most people must run more cheaply than a full mind.
- **Options:**
  - (a) Everyone always runs as a full mind. This is purest, but history slows in step with population.
  - (b) Everyone is always an individual, with their own body, family, memories, skills and beliefs.
    - People in routine situations may run more cheaply, even as part of their band, but only once a test shows it gives the same history, statistically, as full detail.
    - Anyone facing something new, risky or important runs in full.
  - (c) Far-away groups become summaries, and individuals are rebuilt when needed.
- **Recommendation: (b).** Animals are individuals with minds near people. Elsewhere they are populations, carrying inherited and learned traits such as wariness.

**D3. How fast history can run** · Se H3 · Fe 4.2, §2
- **Problem:** `TIM-01`'s scale (centuries a minute from the globe) can't be met at full depth. The file has no minimum speed, and no rule for what gives.
- **Options:**
  - (a) Zoom asks for a speed, and history runs as fast as the phone allows at the detail the world needs. `TIM-01`'s table is rewritten from measurements. History that has already run, for example overnight, can be replayed at any speed, so a thousand years can still sweep past like weather. Target for the tests: a thousand years in one night for a few hundred people.
  - (b) As (a), with a lower target: a thousand years in about a week of nights.
  - (c) Cap the population or the world's size to keep the original speeds.
- **Recommendation: (a).** The target tells the tests what to aim for; it becomes a promise only once measured.

**D4. The past after an update** · Se H6 · Sc H7 · Fe 3.4 · blocks X4
- **Problem:** re-running any past moment needs the rules that made it. A new build ships with every visible change (`PRC-11`), so each install could break:
  - replays and report links;
  - "watch year 41";
  - "tap the painting, see the hunt".
- **Options:**
  - (a) Rules change only in named rules releases, separate from app builds, roughly one per milestone. The app keeps every rules release, so the past and old experiments always replay exactly. A branch can use the old rules or the new ones.
  - (b) Keep only the last few rules releases. Older history can be browsed (chronicle, snapshots, archaeology) but not replayed.
  - (c) Keep the current rules only.
- **Recommendation: (a).** If keeping every release ever becomes too costly, it's raised with you before anything is dropped.

### Worlds, the first people, and what emerges

**D5. New layers in old worlds** · Sc H8, M13 · Se M1
- **Problem:** nothing says how an update's new tin ore, disease or law enters a running world without appearing from nothing. Separately, a world can run into a law that isn't built yet, and nobody can tell that from a real dead end.
- **Options:**
  - (a) New things are back-filled as if they had always been there, generated from the world's own seed, and the join is marked on the timeline. Worlds made before the whole-world milestone may be replaced.
  - (b) Old worlds keep the layers they were made with. This needs `D4` (a).
  - (c) Early worlds are disposable.
- **Recommendation: (a).** Also, the scientist's view flags actions that run into a law not built yet, and they are logged to help decide what to build next.

**D6. The first people** · Se H4 · Sc H3 · St 8
- **Problem:**
  - `BIO-02` lists headings, not contents. Nothing says what people know about local food and dangers, or whether they have a fire, families, ages, memories or words.
  - "Beliefs: none" contradicts the kit's own abilities.
  - `VIS-06` says "a handful of words"; three other items say "a few dozen".
- **Options:**
  - (a) Generated like the world.
    - Families, ages and relationships follow real hunter-gatherer patterns.
    - Each adult knows their home range (food, water, dangers, seasons), plus the kit's abilities and a few dozen words, all written precisely in `BIO-02` for your OK. They know nothing beyond the home range.
    - A band may start with a fire it is keeping, depending on recent weather.
  - (b) A prehistory: run the founders for a generation or two from the kit alone, and keep a run that survives. Everything they know was then learned in the world. Purer, but making a world takes much longer.
  - (c) A bare start with no local knowledge, accepting heavy early losses.
- **Recommendation: (a).** It mirrors how worlds are made: realistic, not grown from nothing.

**D7. Recognising what emerges** · Se H5 · Sc H4 · blocks B64
- **Problem:** many features need the game to spot and name what emerges:
  - firsts and live moments;
  - the skills overlay;
  - counts of languages and beliefs;
  - named institutions, peoples and eras;
  - Experiment 1's measurements.

  No item says how this works, or where such naming may live under `PRN-07`.
- **Options:**
  - (a) General detection of anything new, only.
  - (b) A written catalogue of notable outcomes, only.
  - (c) Both, as a new item.
    - Recognisers sit on the describing side, never feed back into the world, and are kept provably apart from the deciding logic.
    - Thresholds, such as when a dialect becomes a language, are set in the plan and listed in reports.
    - A "first" counts both worldwide and for each people, and a rediscovery is marked as one.
- **Recommendation: (c).**

**D8. Experiment rules** · Se H7, H8, M3, M16 · Sc H5, H6, M1, M3, M20 · Fe 3.3, 4.15
- **Problem:**
  - Experiment 1's criteria use words, not numbers ("noticeably", "much rarer").
  - Nothing stops anyone tuning and re-running on the same seeds until it passes.
  - Signature moments have no pass bar and are never re-checked.
  - Experiment worlds may differ from the worlds you play.
  - Many values have no real-world measurement.
  - Many "can emerge" claims have no experiment at all.
- **Recommendation, in six parts:**
  1. **Criteria first.** Each experiment's exact numbers and definitions are written before it runs. The independent reviewer checks them for ways the experiment couldn't fail, and you approve them. Experiment 1's world includes uses for a sharp edge: carcasses, hides and wood.
  2. **Tuning and failure.**
     - Values are tuned on development seeds, then confirmed once on fresh seeds held back for that.
     - Every attempt and every tuned value is logged, with what it was tuned against.
     - A failed confirmation holds the milestone until you choose: redesign, a weaker claim, or dropping the claim.
     - A test may stop early once its result is clear, within a stated computing budget.
  3. **Signature moments.**
     - A moment passes if it happens in at least 1 world in 10 within its time window, unless its own criteria say otherwise.
     - A small sample re-runs before every merge, and the full set at every milestone.
     - A failure blocks the milestone.
  4. **The same worlds.**
     - Experiments use the same world generator as play.
     - Scripted events and dials appear only in clearly labelled experiments.
     - A moment that passes only with a dial doesn't count as passing in play.
  5. **Promises.** Every "emerges" claim in sections 9 and 10 either gets an experiment before its milestone closes, or is marked "possible, not promised".
  6. **Real numbers.**
     - `PRN-05` becomes: "a real measurement, or a stated rule or published model applied to real measurements, with sources".
     - Values with no measurement are labelled chosen or tuned, and listed in every milestone report.
- **Options:** accept all; accept with changes; discuss each.

### Your powers

**D9. Fortune** · Se H2 · Sc H11 · Fe 4.1 · blocks B61
- **Problem:** a hunt's outcome comes from many small chance events, so there is no single "chance" to double. Stacking, high chances, renewals, strength and "finding materials" are also undefined.
- **Options:**
  - (a) Fortune acts only on chance events around its target.
    - A blessed failure gets one more try.
    - A cursed success is retried at most half the time.
    - Each event gets at most one retry, however many blessings and curses overlap.

    This can never more than double or halve a chance, never make the impossible possible, and never touch a choice. One fixed strength: you choose the target and how long it lasts.
  - (b) A separate luck roll on top of the physics. Simpler, but a rule above nature.
  - (c) Fortune shifts a success rate that the game estimates for each action.
- **Recommendation: (a).** Its overall effect is measured in tests (`B61`).

**D10. Limits on weather and disasters** · Se M6, M8 · Sc H10 · Fe 4.1
- **Problem:** several things are undefined:
  - where a storm you "bring" comes from;
  - how hard a hard winter is;
  - how big a region can be;
  - whether repetition has any limit. A once-a-century drought sent every year passes today's checks.
- **Options:**
  - (a) Budgets from nature.
    - Weather is drawn from the climate's own statistics, and your nudges shift those draws within the place's real range.
    - A run of nudges can't push a place beyond its climate's worst natural stretch.
    - A region is at most about one climate zone across (~100 km).
    - Storms build over hours, as weather does.
    - Earthquakes and eruptions use up stored strain and magma: you choose where and when, and nature's stores decide how big.
  - (b) Nothing more: anything possible can be repeated.
  - (c) Simple cool-downs for each place.
- **Recommendation: (a).**

**D11. Dreams** · Se M7 · Sc M6
- **Problem:** fortune is capped; dreams aren't. Nothing says whether your dream replaces the night's natural one. Two of the feelings in `GOD-03`, longing and hope, don't exist in `MND-19`.
- **Options:**
  - (a) Your dream replaces that night's own dream and is never stronger than the strongest natural dream. You only choose what it contains. Repeats follow the mind's normal rules for recurring dreams. Longing and hope are added to `MND-19`.
  - (b) A stated cap, like fortune's.
  - (c) Strength set by experiment, then fixed.
- **Recommendation: (a).** It also keeps your dreams indistinguishable from natural ones (`GOD-06`).

### Minds

**D12. Inborn tendencies** · Sc H9 · Se L8
- **Problem:**
  - `MND-21` has nothing social beyond parent and child, and `MND-07` has no pain. Yet favouring kin, returning favours, fairness, loyalty to one's group, avoiding incest, shared attention, learning words and rhythm are what society and language grow from.
  - Fear of "snakes" names a group of species, against `PRN-07`.
- **Options:**
  - (a) Closed list: everything social must be learned.
  - (b) Extend the list now from research, with sources. Each addition comes as a Proposed item with a comparison run:
    - pain;
    - favouring kin, and caring for the hurt;
    - returning favours, and anger at cheats;
    - favouring one's own group;
    - not desiring those one was raised with;
    - shared attention and pointing;
    - readiness to learn words;
    - moving to a beat.

    Fear of snakes becomes fear of long, legless things that move suddenly.
  - (c) Add a tendency only when an experiment needs it.
- **Recommendation: (b).**

**D13. Desire, pairing and sex** · Se M21 · Sc M15, M17
- **Problem:** "the urge to have children" assumes people know where children come from (`PRN-01`). Whether sex is a simulated act decides which dark acts are possible.
- **Options:**
  - (a) Sexual desire and attachment replace the urge, and the link to pregnancy is learned. Pairing and conception are simulated abstractly, never as explicit acts, so sexual violence is not modelled.
  - (b) Keep the urge, as a stated simplification that carries no knowledge.
  - (c) Sex is a simulated act between people.
- **Recommendation: (a).**

**D14. Beliefs, actions and talk** · Se M22, M23 · Sc M16 · blocks B46
- **Recommendation, in three parts:**
  1. **Kinds of belief.** Beliefs come in several kinds: cause and effect, that something exists, about other minds, rules, and plain facts. Each has a certainty and its evidence. The glossary follows.
  2. **Actions.** `MAT-12` stays "actions on matter". A new item lists moving, eating, sleeping, social and communicative acts, saying which are inborn (walking, running, climbing, calling, pointing) and which are learned (swimming, among others).
  3. **Conversations.** A new item covers what people can say to each other (warn, ask, tell, teach, retell), and how being told something is weighed against their own experience, by trust in the speaker.
- **Options:** accept all; accept with changes; discuss.

### Matter and bodies

**D15. Matter** · Se M4, M18, M19, L7 · Sc L7 · St 12 · Fe 3.6, 4.6
- **Recommendation, in five parts:**
  1. **Properties.** `MAT-03` becomes: properties come from measured data for each ingredient and structure, combined by stated rules for mixtures and structures. Feasibility notes they can't be derived from makeup in general.
  2. **Laws.** Laws are general reactions, decided by real data on heat and rates. `MAT-04`'s product names become general laws, such as "tannins bind to proteins" and "a flux lowers the melting point of silica".
  3. **Missing laws.** Add buoyancy, flowing water and air, and vibration, with checks such as "a dry log floats; a stone sinks" and "blowing on embers makes them hotter".
  4. **Reality checks.**
     - Each check gets a real-world range and a "must not" partner.
     - Results are judged by properties, not names.
     - A check becomes active once its layer is built.
     - The file keeps the defining checks; the catalogues hold the rest, each naming the item it supports.
  5. **The general-rules check** also proves four things:
     - decision logic reads only what people perceive, never what a thing is;
     - swapping two materials' identities changes no behaviour;
     - every law applies to at least two materials;
     - discovery survives decoys, and a made-up material nobody designed for.
- **Options:** accept all; accept with changes; discuss.

**D16. Life figures and old age** · Se M20 · Sc M14 · Fe 4.7
- **Problem:** `BIO-04`'s figures could be built-in rates, which `BIO-14` forbids, or results to check. "Old age" has no mechanism.
- **Options:**
  - (a) The figures are targets, checked by experiment with tolerances. Every death has a simulated cause, and ageing brings wear and frailty: slower healing, weaker defences against disease.
  - (b) Built-in rates for old age only.
  - (c) Built-in rates.
- **Recommendation: (a).**

### The phone and the process

**D17. Your phone's memory** · Fe 1, 4.13
- The Pixel 11 Pro XL comes with 12 GB of memory (256 GB storage) or 16 GB (512 GB or 1 TB). Each app gets about 8 GiB on the first and 10 GiB on the second, shared by the simulation, the picture and the writer AI. Which do you have?

**D18. "Comfortable for an hour"** · Se M2 · St 5 · Fe 4.13
- **Problem:** `VIS-14` has no number, and the number sets how fast history can run. At about 5 W in total, the battery drains about 25% an hour.
- **Options:**
  - (a) About 20% of the battery an hour while playing, and no heat warning.
  - (b) About 10–15% an hour: gentler, with less history per hour.
  - (c) About 25–30% an hour: more history, with long sessions on the charger.
- **Recommendation: (a).**

**D19. Checks that need the phone** · Se M30 · Fe 4.14 · blocks B82
- **Problem:** `PRC-10` requires the phone-and-cloud check before every merge, but AI agents can't reach your phone.
- **Options:**
  - (a) A cloud machine with the same kind of processor as the phone (ARM64) stands in for it on every merge. GitHub offers these free for private repositories, within monthly minutes. The phone checks itself at each install, against fingerprints of known runs, and only tells you if something is wrong.
  - (b) You run a check build after each install.
  - (c) A second phone of the same model, used for automatic tests.
- **Recommendation: (a).**

**D20. Milestones: in the file or in the plan?** · St 9 · Se M5 · Sc M2
- **Problem:** the build order is written in five places that already disagree.
  - Voices, disease, clothing, pigments and seas have no milestone.
  - `MOM-05` is tested before seas exist.
- **Options:**
  - (a) Each milestone lists the items it delivers, in this file.
  - (b) The file keeps each milestone's goal and order, and the plan maps every item to a milestone. `MAT-16`, `SND-05` and `RES-07` state order only, and `MOM-05` moves to the whole-world milestone.
- **Recommendation: (b).**

**D21. Units of time** · Se M11 · Sc M4
- **Problem:** world years run from about half an Earth year to two, but the life figures and Experiment 1's limits don't say which year they mean. An 18- or 36-hour day also has no rule for sleep.
- **Options:**
  - (a) Earth time inside the simulation and in every criterion. Dates show the world's own years, and ages show Earth years. Bodies are adapted to the world's day.
  - (b) World years everywhere, with biology converted.
- **Recommendation: (a).**

---

## Packages to confirm

### F. The file's own rules (St 1–15, Se L12, Sc L8)

Recommendation: accept all.

- **F1. Coverage that works.**
  - Items are marked as a *feature* (needs a task), a *rule* (needs a check) or *context* (needs nothing: vision text, summaries, risks).
  - The plan maps every feature and rule to a milestone, and tasks are required only for the current one.
  - `PRC-12` states the rule once, excluding *Dropped* and *Proposed* items, and ID rule 4 points to it.
  - The check runs at every milestone gate, and also confirms that IDs named in code and tests exist.
- **F2. Changes on record.**
  - Every commit that changes `PROJECT.md` ends with a line naming the changed IDs and why, for example `Changed: GOD-04 (blessing cap raised; owner OK)`. A check enforces it.
  - Work linked to a changed item is flagged for re-checking.
  - Items are edited in place.
  - Sentences in the source go one per line, so changes show clearly.
- **F3. Safe IDs.**
  - A file check on every merge confirms: each ID is defined once; references resolve; statuses are valid; and no live item points to a dropped one.
  - A new ID becomes permanent only when it reaches the main version.
- **F4. Item size.** If two parts of an item can be delivered in different milestones, they get separate IDs, split when the plan needs it. Tests link to items.
- **F5. Done-when.**
  - Acceptance criteria are written in the plan, and you approve them when each milestone starts.
  - "About" means within 10% unless stated.
  - `VIS-15`'s checks are your judgement at milestone reviews.
- **F6. One status marker.**
  - Each item has exactly one of the five status words, with nothing else inside the marker.
  - "Follows from" becomes its own field.
  - Risk ratings become fields that agents may update in each milestone report.
- **F7. Changing a decided item.**
  - A "Proposed change" line goes beneath the item and is listed in 17.2. Your OK replaces the text.
  - A *To test* item gets its measured result written in, and becomes *Decided* at a milestone review with your OK.
  - `RES-03` counts as *Decided*.
- **F8. One home per fact.**
  - Duplicates become short pointers: `SCP-03` points to `RES-01`; `RES-05` to `PRN-08`'s check; `SCP-06`, `MND-01` and `PRE-17` to `PRN-06`; the orientation rule in `VIS-14` and `PLT-02` to `PRE-34`.
  - If a summary and its source disagree, the source wins.
- **F9. Generated lists.**
  - Section 17 and the contents are generated by a tool, between markers.
  - The glossary points to items instead of restating rules.
  - `CLAUDE.md` cites IDs, names sections rather than numbering them, and says "never without your OK".
  - Changes to `CLAUDE.md` need your OK.
- **F10. Rules without IDs.**
  - "How this file works" is *Decided* as a whole, under `PRC-07`.
  - Binding sentences in section introductions become items or pointers.
  - The loose line after `PLT-04` goes, since it repeats `PRN-11`.
- **F11. Room to grow.**
  - Numbers go to three digits after 99.
  - One sentence explains how a new area code is added.
  - Later experiments' criteria become new `RES` items, proposed before they run.
- **F12. Binding fields.**
  - *What*, *Done when* and *Check* are binding.
  - *Why* and *Example* only explain.
  - Any other label counts as *What*.
  - `PRN-14` becomes "never rewrite what works without a stated reason", matching its own check.
- **F13. Navigation.**
  - Sections are referred to by name.
  - Items that share a name are renamed, for example "Their sky (simulated)" and "Their sky (view)".
  - A generated phone copy with tappable IDs comes later.
- **F14. Dated lines.**
  - `PRC-05` and `PRC-08` become lasting rules, or are dropped once true.
  - Dropped items get the marker *(Dropped)* plus a "Dropped because:" line.
  - The empty state of 17.2 is just "None."

### C. Plain corrections

Recommendation: accept all.

- **C1.** `VIS-06`: "a handful of words" becomes "a few dozen words" (St 8, Se H4, Sc H3).
- **C2.** `WLD-26`: the sun also raises tides, so worlds without moons still have weaker tides (Sc L5).
- **C3.** `WLD-04` (Sc L6, Fe 4.4):
  - fix the arithmetic: farming would feed about half a million to five million people, not ten million;
  - note that on the wrap-around map a third of the area lies beyond 60° latitude, so there is less good land than Earth intuition suggests;
  - mark the figures as orders of magnitude only.
- **C4.** `WLD-11`: generating the candidates and choosing the best takes "a few minutes in total" (Se L6, Sc M21).
- **C5.** `WLD-16` and `WLD-09`: climate and generation use rules derived from real physics and calibrated to Earth, not full physical models (Fe 4.4). A physical climate model takes 30–90 seconds of a computer per simulated year.
- **C6.** `PRN-13`: explanations of the past are rebuilt on request by re-running from the nearest saved point, which can take a little time (Fe 3.5).
- **C7.** `SCP-14`: "ancestral minds" is a starting point for experiments only (Se L4, Sc L9).
- **C8.** `SND-06` and `WLD-02` (Se L11):
  - recorded birdsong plays only where matching birds are simulated;
  - the globe's squeezed poles are a labelled exception in the display.

### R. Risks to add

All ratings are proposed. Recommendation: accept all.

- **R1.** Simplified minds behave differently from full ones · likelihood medium, impact high
- **R2.** Experiments too big for the cloud sessions · high, high
- **R3.** The app's memory cap · high, medium
- **R4.** Keeping old rules runnable to replay the past · high, medium
- **R5.** Made-up or mismatched sources in AI-written catalogues · medium, medium
- **R6.** The writer AI softening or refusing dark history · medium, low
- **R7.** New install rules for Android apps from 2027 · medium, low
- **R8.** Language, grammar or belief failing to emerge · medium, high
- **R9.** Saved worlds growing too large · medium, medium
- **R10.** Losing a world to a bad update or a damaged save · medium, high
- **R11.** The writer AI too slow or too large to run · medium, medium
- **R12.** Your own time for reviews and phone checks · medium, medium
- **R13.** The phone ageing or being replaced · low, medium
- **Also:** `RSK-12`'s impact rises from low to medium.

---

## Decide before their milestone

Each comes with my recommendation; the alternatives are in the source findings.

- **L1. Minds and sex** (Se M21): no inborn average differences in minds between the sexes. Differences, if any, come from culture. The alternative is average differences taken from research, with wide overlap.
- **L2. Scaling for a small world** (Se M13, Sc M5, Fe 3.3): one rule. Quantities set by distance (weather systems, currents, migrations, climate belts) scale with the world. Local quantities (bodies, chemistry, materials, rates) stay real. Every scaled value is labelled.
- **L3. The polar seam** (Se M14, Sc M22): a permanent ice cap along the seam that no weather or animal crosses, as a labelled exception.
- **L4. Ice ages and the rising sea** (Se M15, Sc M23): real cycle lengths, with worlds starting late in an ice age, so seas rise during the first ten thousand years. `WLD-16`'s "over thousands of years" becomes tens of thousands.
- **L5. Names before words** (Se M24, Sc M8):
  - the game uses labels built from each world's own sounds, marked as the game's, until people's own names emerge;
  - English text uses their concepts ("cutting stone"), and our words appear only in the scientist's view.
- **L6. Their art, songs and maps** (Se M25, Sc L3):
  - songs are real sequences of notes;
  - pictures and maps are stored as what they show and how (composition, style, skill, pigments), and drawn from that;
  - tapping a painting shows the painting and what the painter meant, and in the scientist's view, the real event;
  - memory scenes show what the person remembers.
- **L7. Story mode and scientist mode** (Se M26, Sc M9): these are two modes of the whole game.
  - Story mode is the default and never reveals your interventions anywhere; events you caused appear as natural events.
  - Scientist mode shows everything.
- **L8. Dark history and the content setting** (Se M27, Sc M17, Fe 4.11):
  - violence, war, captivity and slavery, sacrifice, cruelty, infanticide and cannibalism can emerge;
  - sexual acts are abstract (`D13`), and bodies are drawn without sexual detail;
  - three levels: show; plain (no graphic pictures or sounds, factual text); and gentle (dark events mentioned briefly, in the chronicle only);
  - if the writer AI refuses, plain factual text is shown instead.
- **L9. Interface and time** (Se M10, M28, L1, L5, L10; Sc M10, M11, L1, L2):
  - **Gestures:** one-thumb zoom by double-tapping and dragging. "Draw an area" is opened from the long-press menu. Views swipe up from the bottom edge only.
  - **Who sets the speed:** manual pause and lock beat the director, and the director beats zoom. Choosing a power pauses time. Overnight mode ignores the director but keeps its moments for the morning.
  - **Interruptions:** one shared list of what interrupts, at most about one interruption a minute, with importance scaled by zoom.
  - **Following:** a list of the people you follow, separate from the camera.
  - **On screen:** touching the screen briefly shows the date, the real speed and the time control. Live moments wait in a list. Opening the app resumes time toward the next moment.
  - **Screens:** a first-launch screen, a list of worlds and settings, with help cards on first use instead of a tutorial.
- **L10. Saves and phones** (Se M12, M29; Sc M12, M24; Fe 4.3):
  - **Rewinding:** instant to saved points, with a short wait in between. Saved points are dense near the present and sparse further back.
  - **Storage:** you can delete worlds and branches. Near a full phone, the game warns you and asks what to delete, and never silently thins history.
  - **Safety:** a safety copy is kept before an update changes a world. A world can also be rebuilt from its seed and your interventions (with `D4` (a)).
  - **Changing phones:** the project moves to the new model. Worlds replay identically, because the simulation uses only the processor, under strict rules.
- **L11. The writer AI's rules** (Sc M7, Se L2, Fe 4.11):
  - **Writing:** text is written when first opened or during pauses, checked, stored, and never silently rewritten. You can ask for a rewrite. Text that fails its check falls back to plain factual text.
  - **Selection:** what the chronicle covers, and where ages begin, come from fixed rules (`D7`), not the writer's taste.
  - **Content:** the writer chooses words and rhythm, never content. Every claim, cause, motive, image and name must be in the data.
- **L12. Branches and "what changed because of it"** (Se M9, Sc M19):
  - chance stays local, since each random draw belongs to one being and moment, so a branch differs only where your change actually reaches;
  - the scientist's view shows the chain of causes by default, and runs a comparison branch on request;
  - the intervention record holds every detail: the memories chosen, the feeling, the region drawn, the duration.
- **L13. Colours, new worlds and looks** (Se L6, L9; Sc M18, M21, L4):
  - **Colours:** colour ladders are made automatically from the simulated colour and matched to the master palette. Hand-picked ladders are overrides for common materials only.
  - **New worlds:** "New world" shows the best three candidates as small globes, each with a one-line summary. You pick one or let the game pick, and you can enter a seed.
  - **Looks:** skin, hair and faces are inherited and vary by region with sunlight, as in real biology. They are designed so no people reads as a copy of a real one.
- **L14. Your own experiments** (Se L3): you ask for an experiment in a session. An experiment world opened on the phone keeps its dial settings for good, and shows them.

---

## Settled by tests

These join the building blocks as the first tests to prepare (Fe §7).

- **T1. What a mind costs** (blocks `B39`, `B04`, `B63`): milliseconds and memory per person per simulated day, on the phone and in the cloud.
  - Over ~1 ms: Experiment 1 can't fit in one session.
  - Over ~5 ms: simplified minds become essential.
  - Over ~1 MB per person: detailed populations stop in the low thousands.
- **T2. Experiment 0** (`B26`, `B28`, `B34`–`B41`, `B45`): a toy sharp-stone world with materials described only by properties, generic actions and the planned learners, plus decoys and a made-up material. If discovery happens only when senses and actions are shaped around knapping, or collapses with decoys, section 9 is rethought before the engine is built.
- **T3. Same numbers on phone and cloud** (`B01`, `B05`, `B82`): 10⁶ steps, compared on the phone, the cloud and a GitHub ARM64 machine.
  - A mismatch that can't be traced in a day means whole-number maths for all simulation state.
  - If ARM64 always matches the phone, it becomes the stand-in on every merge.
- **T4. The phone's real budget** (`B79`, `B66`, `B73`): an hour of simulation, picture and writing at once. It measures sustained speed, frame times, battery per hour, heat and the memory cap, once on battery and once overnight on the charger.
- **T5. Simplified versus full minds** (`B43`, `B08`): does a band-level model reproduce the full model's discovery, spread, loss and population? If none does, `TIM-01`, `VIS-07` and `WLD-04` are restated.
- **T6. Computing available in the cloud** (`B80`): how long background jobs survive, whether they resume, and how many run at once. Under ~100 CPU-hours a week, Experiment 1 needs other computers, and that is raised with you (`SCP-15`).
- **T7. The writer AI on the phone** (`B73`): the candidate models are the open Gemma 4 models in two sizes, run by the app through LiteRT-LM (on the graphics chip or the phone's AI chip), and Google's built-in Gemini Nano. The test measures speed, memory and heat; how often each adds facts; whether it softens dark events; and your rating of 30 entries.
  - More than a few percent of entries adding facts means fixed sentence frames, with the model only smoothing them.
  - If you find the writing flat, `PRE-37` is raised now rather than later.
- **T8. Still pixels while turning** (`B66`, `B67`): a perspective camera can't keep every pixel still while turning or zooming; this is an open research problem. If the shimmer bothers you, `PRE-22` is restated (locked while panning and at rest), or the close camera is limited.
- **T9. Snapshots and rewinding** (`B07`): snapshot sizes, and the time to reach any moment. Over ~100 MB per snapshot means "any moment" becomes "any saved moment, or a wait".
- **T10. A language toy** (`B47`–`B49`, `B76`): naming, relearning across generations, and sound drift together. If no usable grammar emerges, `CUL-17` and `SND-03` are restated before the words-and-beliefs milestone, as "words and simple word order" with stylised voices.
- **T11. Installing from 2027** (`B78`): certified Android phones will require apps from registered developers. The ways open are a free hobbyist developer account, a one-off "advanced" unlock with a 24-hour wait, or a USB cable. Decide before 2027.
- **T12. Sourcing ten thousand numbers** (`B09`): AI-written citations can be invented. Every value is checked against the fetched source, with the supporting passage quoted. The test measures the time per entry.

---

## Where every finding went

- **Structure (St):**

  | St | Goes to |
  |---|---|
  | 1 | F1 |
  | 2 | F2 |
  | 3 | F3 |
  | 4 | F4 |
  | 5 | F5, D18 |
  | 6 | F6 |
  | 7 | F7 |
  | 8 | F8, C1 |
  | 9 | D20 |
  | 10 | F9 |
  | 11 | F10 |
  | 12 | F11, D15 |
  | 13 | F12 |
  | 14 | F13 |
  | 15 | F14 |

- **Gaps by section (Se):**

  | Se | Goes to |
  |---|---|
  | H1 | D1, D2 |
  | H2 | D9 |
  | H3 | D3 |
  | H4 | D6 |
  | H5 | D7 |
  | H6 | D4 |
  | H7 | D8 |
  | H8 | D8 |
  | M1 | D5 |
  | M2 | D18, F5 |
  | M3 | D8 |
  | M4 | D15 |
  | M5 | D20 |
  | M6 | D10 |
  | M7 | D11 |
  | M8 | D10 |
  | M9 | L12 |
  | M10 | L9 |
  | M11 | D21 |
  | M12 | L10 |
  | M13 | L2 |
  | M14 | L3 |
  | M15 | L4 |
  | M16 | D8 |
  | M17 | D2 |
  | M18 | D15 |
  | M19 | D15 |
  | M20 | D16 |
  | M21 | D13, L1 |
  | M22 | D14 |
  | M23 | D14 |
  | M24 | L5 |
  | M25 | L6 |
  | M26 | L7 |
  | M27 | L8 |
  | M28 | L9 |
  | M29 | L10 |
  | M30 | D19 |
  | M31 | R |
  | L1 | L9 |
  | L2 | L11 |
  | L3 | L14 |
  | L4 | C7 |
  | L5 | L9 |
  | L6 | L13, C4 |
  | L7 | D15 |
  | L8 | D12 |
  | L9 | L13 |
  | L10 | L9 |
  | L11 | C8 |
  | L12 | F1 |

- **Gaps by scenario (Sc):**

  | Sc | Goes to |
  |---|---|
  | H1 | D1 |
  | H2 | D2 |
  | H3 | D6 |
  | H4 | D7 |
  | H5 | D8 |
  | H6 | D8 |
  | H7 | D4 |
  | H8 | D5 |
  | H9 | D12 |
  | H10 | D10 |
  | H11 | D9 |
  | M1 | D8 |
  | M2 | D20 |
  | M3 | D8 |
  | M4 | D21 |
  | M5 | L2 |
  | M6 | D11 |
  | M7 | L11 |
  | M8 | L5 |
  | M9 | L7 |
  | M10 | L9 |
  | M11 | L9 |
  | M12 | L10 |
  | M13 | D5 |
  | M14 | D16 |
  | M15 | D13 |
  | M16 | D14 |
  | M17 | L8, D13 |
  | M18 | L13 |
  | M19 | L12 |
  | M20 | D8 |
  | M21 | L13, C4 |
  | M22 | L3 |
  | M23 | L4 |
  | M24 | L10 |
  | L1 | L9 |
  | L2 | L9 |
  | L3 | L6 |
  | L4 | L13 |
  | L5 | C2 |
  | L6 | C3 |
  | L7 | D15 |
  | L8 | F5 |
  | L9 | C7 |
  | L10 | R |

- **Feasibility (Fe):**

  | Fe | Goes to |
  |---|---|
  | 3.1 | D1 |
  | 3.2 | D2, D3 |
  | 3.3 | D8 |
  | 3.4 | D4 |
  | 3.5 | C6 |
  | 3.6 | D15 |
  | 4.1 | D9, D10 |
  | 4.2 | D3 |
  | 4.3 | L10, T9 |
  | 4.4 | C3, C5 |
  | 4.5 | D2 |
  | 4.6 | D15, T12 |
  | 4.7 | D16 |
  | 4.8 | D2, T2 |
  | 4.9 | T10 |
  | 4.10 | T8 |
  | 4.11 | L8, L11, T7 |
  | 4.12 | T10 |
  | 4.13 | D17, D18, T11 |
  | 4.14 | D19, T3 |
  | 4.15 | D8, T6 |
  | 4.16 | T12, D19 |
  | §5 | R |
  | §6 proposal 1 | D1 |
  | §6 proposal 2 | D2 |
  | §6 proposal 3 | D8 |
  | §6 proposal 4 | D4 |
  | §6 proposal 5 | D19 |
  | §6 proposal 6 | T8 |
  | §6 proposal 7 | D8 |
  | §6 proposal 8 | D17 |
  | §6 proposal 9 | D18 |
  | §6 proposal 10 | C6 |
  | §6 proposal 11 | D15 |
  | §6 proposal 12 | R |
  | §7 prototype 0 | D1 |
  | §7 prototypes 1–10 | T1–T10 |
