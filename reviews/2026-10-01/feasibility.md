# Kindling: feasibility review of PROJECT.md

Adversarial review. The question: can what the file demands be built, and run as specified, by this team, on this phone? Reviewed version: commit `e87455a`, 1 October 2026. Nothing in the repository was changed.

How to read references: "§3.1" or "4.6" points to this review; "section 9" and IDs like `MND-14` point to PROJECT.md.

## Summary

**Verdicts, 17 areas:** none is feasible exactly as written; 11 are feasible with stated limits; 4 are doubtful; 2 are not feasible as written.

| Verdict | Areas |
|---|---|
| Not feasible as written | The principles (section 2); speed of history |
| Doubtful | Minds; culture and language; the pixel-3D look; experiments in cloud sessions |
| Feasible with stated limits | The other 11 |

**The three biggest problems**

1. **Looking changes history.** Things get more detail where you look (`PRN-11`, `MND-14`, `WLD-12`). A detailed mind behaves differently from a simple one, so where you point the camera changes what happens. But `PRN-08` says only the seed and your interventions decide history, identically on the phone and in the cloud, where nobody looks. Both can't be true.
2. **The arithmetic of depth.** Suppose a full mind costs a modest 50 milliseconds of computing per simulated day. Then the phone runs about 4 years of history per hour for 100 people. "Centuries a minute" needs about 1,500 times less per person: a sum over a band, not a mind. Experiment 1 needs about 2 years of one cloud session at that cost, and about 80 days even at a lean 5 ms.
3. **The minds are a research problem, not an engineering task.** No published system shows small, general learners discovering crafts from raw properties within a few centuries of a few hundred lives, without the designers' choice of senses and actions doing the work. The planned check, a word search for discovery names, can't detect recipes hidden in those choices.

**Do first:** decide the detail rule (no code needed, §3.1). Then run three throwaway prototypes: the cost of a mind, a toy Experiment 0, and a phone–cloud determinism test (§7).

---

## 1. Facts this review rests on

### The phone: Pixel 11 Pro XL (announced 12 August 2026)
- **Chip:** Tensor G6 with 7 CPU cores: one C1-Ultra at 4.11 GHz, four C1-Pro at 3.38 GHz, two C1-Pro at 2.65 GHz, plus a PowerVR GPU ([Android Authority](https://www.androidauthority.com/tensor-g6-benchmarks-3699714/)).
- **Heat:** under sustained load the CPU held 51% of its peak in a throttling test (it slows down when hot) ([Gizbot](https://www.gizbot.com/features/google-tensor-g6-vs-tensor-g5-benchmarks-tested-and-compared-128059.html)). The GPU kept about 79% at the end of a 20-run 3DMark stress test ([Android Authority](https://www.androidauthority.com/tensor-g6-benchmarks-3699714/)). One hands-on test saw the GPU score fall below a tenth of its cold peak once the phone was warm ([Notebookcheck](https://www.notebookcheck.net/Google-Pixel-11-Pro-and-Tensor-G6-showcase-severe-GPU-thermal-throttling-in-hands-on-benchmark-tests.1373783.0.html)). Reports conflict, so this must be measured.
- **RAM depends on the model:** 12 GB with 256 GB of storage; 16 GB with 512 GB or 1 TB ([Engadget](https://engadget.com/2234960/pixel-11-series-cost-more-but-less-ram)). **The file doesn't say which one you have.**
- **Memory cap per app:** Android 17 limits each app. The platform's own table gives a visible app 8 GiB on a 12 GB phone and 10 GiB on a 16 GB phone. Beyond that, the system swaps the app's memory out (stutter), and may kill it ([AOSP](https://source.android.com/docs/core/perf/memory-limiter)). Pixels enforce it first ([ecorpit](https://ecorpit.com/android-app-memory-limits-oem-rollout-vitals-2026/)).
- **Screen and battery:** 1344 × 2992 at 120 Hz; 5,115 mAh, about 20 Wh ([droid-life](https://www.droid-life.com/2026/07/13/pixel-11-series-specs-storage-ram-displays-battery/)).

### The cloud session (measured in this review's own session)
- 4 virtual CPUs (Intel Xeon, 2.1 GHz, x86-64), 15 GB RAM, no swap, about 30 GB of free disk. No GPU, no ARM emulator, no Android tools installed.
- A single command can run in the background for at most 2 hours here.
- **Speed per core**, measured with a small C program on a 1 GiB table: about 32 million scattered memory updates a second; about 3 million when each read depends on the one before (310 ns each); about 4 billion simple arithmetic operations a second.
- **One compiler flag changes history.** The same toy chaotic model, on the same machine, ended at 0.754 with fused multiply–add off and at 0.277 with it on, after a million steps. (Fused multiply–add is a CPU shortcut that rounds once instead of twice. ARM phones always have it; compilers use it by default there.) That is what an unguarded phone build versus an unguarded cloud build looks like.
- Code: `scratchpad/bench/membench.c` and `scratchpad/bench/fmademo.c`.

### Writer AI options on this phone
- **Gemma 4** runs on the phone through Google's LiteRT-LM. On a Galaxy S26 Ultra, the small E2B writes 52 tokens a second on the GPU (676 MB peak memory). The larger E4B writes 22 tokens a second (710 MB on the GPU, 3.3 GB on the CPU), and its file is 3.65 GB. Multi-token prediction adds up to 2.2× on GPUs. No Pixel figures are published, and the Pixel's GPU is weaker ([LiteRT-LM](https://developers.google.com/edge/litert-lm/models/gemma-4)).
- **The Pixel's AI chip:** Google's Tensor SDK (beta, May 2026) lets apps run their own models on the TPU from Pixel 10 on, with a Gemma 4 demo ([Google](https://developers.googleblog.com/google-tensor-sdk-beta-with-litert/)).
- **Gemini Nano through ML Kit (beta):** works only while the app is in front, has per-app request limits and a separate long-run battery quota, and different Nano versions give different text for the same prompt ([ML Kit](https://developers.google.com/ml-kit/genai)).

### What the budget numbers below assume (prototype 1 replaces these)
- **Phone computing for the simulation:** about 2 core-seconds per real second while you play. That leaves room for the picture, sound and writer, and stays cool enough for an hour. About 4 overnight on the charger. A phone core and a cloud vCPU are treated as roughly equal; this could be off by 2× either way.
- **Cost of one full mind per simulated day,** in milliseconds of one core:
  - *Lean, 5 ms:* about 150,000 memory touches a day. Roughly one decision a minute.
  - *Middle, 50 ms:* about 1.5 million. A decision every 10 seconds awake, looking at ~25 things and ~250 memories, plus nightly replay and some planning.
  - *Heavy, 500 ms:* perception every second and larger searches.
  - Middle is my central guess for minds as section 9 describes them.
- **Memory per detailed person:** lean 0.1 MB, middle 1 MB, heavy 5 MB. That is 10⁴–10⁵ remembered items: events, places with seasons, people and debts, beliefs with evidence, skills, and what others know.

## 2. The budget in plain numbers

### History per real hour on the phone: 100 people, full minds

| Mind cost | Playing | Overnight (8 h) |
|---|---|---|
| Lean | ~40 years an hour | ~630 years |
| Middle | ~4 years an hour | ~63 years |
| Heavy | ~5 months an hour | ~6 years |

Ten times the people gives a tenth of the history. At the file's own estimate of 50,000 hunter-gatherers (`WLD-04`), a middle-cost overnight run covers about 46 days.

### What the file's speeds need: computing per person per simulated day (100 people, playing)
- **One person or a camp:** met at any cost.
- **A valley, "a season a minute"** (~1.5 days a second): 13 ms or less. Only lean minds manage it.
- **A region, "years a minute"** (say two years a minute, ~12 days a second): 1.6 ms or less. Not even lean.
- **The world, "centuries a minute"** (~600 days a second): 0.03 ms or less. That is about 1,000 memory touches per person per day: a sum over a band, not a mind.
- **The overnight example in `TIM-12`** ("312 years passed"; say 500 people): about 2 ms or less.

### Memory
The 8–10 GiB cap must also hold the writer model (0.7–3.3 GB), the world and the picture (very roughly 2–3 GB). That leaves about 2–8 GB for people:
- lean: 20,000–80,000 detailed people;
- middle: 2,000–8,000;
- heavy: 400–1,600.

So 50,000 detailed hunter-gatherers fits only at the lean end. Ten million farmers (`WLD-04`) would need under 1 KB each: a population table, not people.

### Experiment 1 in one cloud session
Three setups (the main run and the two comparison runs of `RES-03`) × 100 worlds × 500 years × ~100 people ≈ 5.5 billion person-days.
- lean: ~80 days of the session's 4 cores;
- middle: ~2.2 years;
- heavy: ~22 years.

Stopping each world 50 years after its discovery might halve this. One session can only hold Experiment 1 if a mind costs well under 1 ms a day.

---

## 3. Where the principles make things infeasible

**Verdict for section 2 as a whole: not feasible as written,** because of 3.1 and 3.3. Each subsection gives the smallest change.

### 3.1 Looking changes history (`PRN-08` against `PRN-11`, `MND-14`, `WLD-12`, `WLD-13`)
- **The conflict.** `PRN-08`: the seed and your interventions fully decide history, the same on the phone and in the cloud. `PRN-11`, `MND-14` and `WLD-12`: things get more detail where you look. Minds far from your attention run as "habits, and knowledge held by the group", and sharpen when you zoom in. A sharpened mind can notice an accident and discover something; a group habit can't, or does it by another route. So the camera changes history. The cloud has no camera, so a cloud history and a phone history of the same seed diverge. `PRN-08`'s own example (open a cloud world and watch year 41 happen as reported) fails the moment you zoom.
- **Experiments too.** If cloud runs treat everyone as unwatched, Experiment 1 tests group habits, not the minds of section 9. If they treat everyone as watched, the phone, where almost everyone is unwatched, isn't running the same world.
- **Sharpening is causal.** A coarse mind that sharpens needs memories and beliefs filled in, and those then drive what it does. "Generated the same way every time" (`WLD-13`) holds only if generation never depends on when you happened to look.
- **No coarse model can be exactly equivalent to a full one.** If it were, it would be the full model.
- **It won't show up early.** `MIL-01`'s test (a small valley, identical on phone and cloud) can pass by running the whole valley in full detail. The conflict appears once the world is too big for that (`MIL-06`).
- **Smallest change (recommended):** in `PRN-11`, `MND-14`, `WLD-12` and `WLD-13`, replace "where you look" with "a rule that depends only on the world's own state and your recorded interventions". The camera then only changes the picture. The story director's sense of what matters (`TIM-02`) is a natural basis for the rule. Cost: following a quiet person (`PRE-06`) shows a coarse mind unless the rule sharpens them.
- **Alternative:** record the camera as an intervention. `PRN-08` then holds, but watching changes outcomes, and cloud experiments need a declared default "camera". It also means what experiments test is not what you watch.

### 3.2 Depth stays, with no cap (`PRN-11`, `MND-15`)
- At full depth, time slows in proportion to population (§2). With no cap, a thriving world slows towards a standstill: at 50,000 people, middle cost, one overnight is about 46 days.
- `PRN-11` already allows less detail for what no one is watching. Once 3.1 is fixed, that coarse model must carry most of history at far zoom. It must also be able to discover things using only general rules, or innovation stops whenever detail drops.
- **Smallest change:** keep `PRN-11`, and add that the coarse model is chosen by the rule of 3.1 and must match the full model's statistics in a comparison experiment before it is trusted.

### 3.3 Real numbers for everything (`PRN-05`) can't hold literally
- Several decided items aren't real measurements: a 2,000 km planet with Earth gravity and air (`WLD-03`, `WLD-06`); weather "scaled to fit" (`WLD-05`); species "adapted" from Earth families (`WLD-19`); properties of mixtures, which are modelled, not measured (`MAT-03`).
- **Smallest change:** "Every value comes from a real measurement, or from a stated rule applied to real measurements; both name their sources."

### 3.4 Same rules, updates keep history, rewind anywhere (`PRN-08`, `PLT-09`, `TIM-06`)
- Re-running a past moment exactly needs the rules that produced it. After an update, re-running earlier history needs the old rules still in the app, bit for bit, built so that a newer compiler doesn't change results. `PRC-11` ships a build "whenever something you can see or try has changed", so a long-lived world could cross dozens of rule versions.
- **Smallest change:** version the rules separately from the app. Keep old rules runnable for a stated number of versions. Before that point, the past is viewable from saved snapshots and the chronicle, and branching starts only from saved points.

### 3.5 Every choice explained, for any action in any run (`PRN-13`)
- Storing a reason for every decision is too big. At middle cost that is ~5,800 decisions per person per day; for 100 people, about 13 GB a year at 64 bytes each.
- It is feasible only by re-running from the nearest saved point when you ask, which takes seconds to minutes. **Limit to state:** explanations of the past are rebuilt, not stored.

### 3.6 General rules only (`PRN-07`): buildable, but its check doesn't test the principle
- A word search is easy to pass. Recipes hide in design choices: which properties a person can sense ("edge sharpness"), which action settings exist ("strike angle"), and which outcomes count as surprising. Those choices can make one discovery nearly certain without ever naming it.
- **Add a check that can fail:** add materials and laws the minds weren't designed around, including a made-up material with a useful property. Test whether discovery still happens, and whether decoys slow it down.

---

## 4. Area by area

Each area gives a verdict, the binding constraint, the evidence, and what would make it feasible. Verdicts are for whole areas; many single items are fine as written, for example the sky, the story director, pausing, timeline comparison, cards, gestures, overlays, and the experiment method itself.

### 4.1 Player powers (section 4): feasible with stated limits
- **Binding constraint:** consistency with mechanistic physics. No hard limit.
- **Evidence:**
  - "A blessing at most doubles a chance" (`GOD-04`) assumes a hunt has one probability. With physical actions (`MAT-06`), success emerges from many chance events: finding, approaching, throwing, wounding. Bounding each one doesn't guarantee the overall chance at most doubles, and checking it would take many trial runs.
  - "Bring a storm" (`GOD-05`) needs weather that can be steered. That is easy if storms are drawn from the climate's own statistics, and hard if weather is a fluid model.
  - Memories "shown as small pixel-art scenes" (`GOD-10`) mean every memory must store enough to draw it (place, people, objects, light). That raises memory per person.
- **To make it feasible:** state that fortune biases single chance events by a bounded factor, and that its overall effect is measured, not guaranteed. State that weather is drawn from climate statistics.

### 4.2 Speed of history (`TIM-01`, `TIM-02`, `TIM-04`, `TIM-10`–`12`, `VIS-07`, `VIS-11`): not feasible as written
- **Binding constraint:** compute, then the conflict in 3.1.
- **Evidence:**
  - §2: with full minds, only the closest zooms reach their stated speed.
  - "A thousand years of migrations, languages and beliefs" (`VIS-07`) at middle cost takes ~16 overnights for 100 people, and ~160 for 1,000.
  - Some signature moments need real-world timescales. Two tongues becoming mutually unintelligible (`MOM-05`) takes real languages on the order of a thousand years. Wolves becoming dogs (`MOM-06`) and cereals being domesticated (`MOM-08`) take centuries or more. At full depth that is hundreds of phone-hours.
  - Under the fix in 3.1, zoom only requests a speed. The fastest possible speed is set by how much of the world is detailed at that moment, not by the zoom.
  - Comparable games get speed by aggregating. Dwarf Fortress players report frame-rate collapse around 200–250 dwarves on desktop PCs, and the default population cap is 200 ([DF wiki](https://dwarffortresswiki.org/index.php/v0.34:Maximizing_framerate), [Steam](https://steamcommunity.com/app/975370/discussions/0/6063574513309438286)). Crusader Kings III slows down with 24,000–35,000 light characters, and players purge them ([Steam](https://steamcommunity.com/app/1158310/discussions/0/3727323721760166023)).
- **To make it feasible:** fix 3.1; adopt a validated coarse model (3.2); measure the cost of a mind first (prototype 1) and rewrite `TIM-01`'s scale from measured numbers; accept that close-up history is slow.

### 4.3 Rewind, branches and saves (`TIM-06`, `TIM-08`, `TIM-13`, `PLT-07`–`09`): feasible with stated limits
- **Binding constraint:** storage and re-run time, then old rule versions (3.4).
- **Evidence:**
  - A snapshot of a 100-person world at middle cost holds ~100 MB of minds plus the world: roughly 100–200 MB compressed. One per simulated year for 10,000 years is 1–2 TB, more than the phone holds.
  - Rewinding to "any moment" means loading the nearest saved point and re-running. At middle cost for 100 people, that is about 8–15 minutes per simulated year.
  - Saving continuously by rewriting gigabytes would wear out the phone's storage. With determinism, saving only needs the intervention log plus occasional snapshots.
- **To make it feasible:** state the rewind granularity: instant to saved points, a wait in between. Save densely near the present and sparsely further back. Prototype 9 measures snapshot sizes.

### 4.4 Making a world (`WLD-01`–`11`, `WLD-19`, `WLD-23`, `WLD-24`): feasible with stated limits
- **Binding constraint:** compute (under a minute on the phone, `WLD-11`).
- **Evidence:**
  - At 1 km per cell the world is ~2 million cells. Fast erosion methods and rain-shadow rules handle that in seconds. Existing generators already do plates, erosion, rain shadows and biomes ([WorldEngine](https://github.com/esampson/worldengine)).
  - Climate "from real physics" is another matter. A simplified physical climate model, ExoPlaSim, takes 30–90 seconds of a computing node per simulated year at its standard low resolution, and needs many years to settle ([ExoPlaSim](https://arxiv.org/pdf/2107.07685)). Under a minute on the phone means rules of thumb, not physics.
  - The torus gives every latitude band equal area, so a third of the map lies beyond 60° (on Earth, 13%). The habitable share is smaller than Earth-like intuition suggests, so `WLD-04`'s 50,000 may be high.
  - 50 animal and 200 plant species per world, each with body chemistry (`WLD-19`, `WLD-23`), is a large sourcing job (4.6).
- **To make it feasible:** state that generation uses physically motivated rules, not physical models. Give the grid resolution. Measure generation time on the phone at `MIL-01`.

### 4.5 Natural systems (`WLD-12`–`18`, `WLD-20`–`22`, `WLD-25`–`28`): feasible with stated limits
- **Binding constraint:** compute, then 3.1.
- **Evidence:**
  - Plants and animals "in patches of a few hundred metres" over 0.5–1 million km² of land makes 2–11 million patches. With ~20 species per patch, a daily update is ~10⁸ species-updates per simulated day. At one simulated day a second, that alone is the phone's whole budget, before any minds.
  - Ice-age cycles (tens of thousands of years) can only be watched at coarse, far-zoom speeds.
  - There are perhaps 10⁶–10⁷ large animals. They can't each have a body and a mind; populations with trait spreads can.
  - Microbes "that spread and evolve" (`WLD-21`) can be strains with traits in each population, not individual microbes.
- **To make it feasible:** run ecology, heredity and disease coarse away from people, chosen by the rule of 3.1. Give animals individual bodies and minds only near people. Restate "real physics" as in 3.3.

### 4.6 Matter and physics (section 7): feasible with stated limits
- **Binding constraint:** content effort and checking it, then storage.
- **Evidence:**
  - Properties can't be "derived" from makeup and structure in general (`MAT-03`). The toughness of flint or the springiness of yew must be looked up for each ingredient and structure.
  - Mixtures need modelled rules, and simple averaging fails the file's own checks. Bronze is harder than both copper and tin (`RCK-17`); glass needs a flux that lowers the melting point of silica (`RCK-20`). Each `RCK` item pulls in a real law with real data.
  - **Size of the job:** 150–300 ingredients × ~30 properties, plus ~100 species templates, laws and structures. That is on the order of 10,000 sourced numbers.
  - **AI-written citations are unreliable.** In 2023, 55% of GPT-3.5's and 18% of GPT-4's citations were fabricated ([Walters & Wilder, Sci. Rep.](https://doaj.org/article/fc88ea07ec994a7c8a144f8516c48cbf)). Newer models do better, but not perfectly. A "names its source" check only proves a source string exists.
  - **Knapping has a usable real basis.** Controlled experiments show platform angle and depth dominate flake size (the "EPA-PD" model) ([Li et al. 2022](https://link.springer.com/article/10.1007/s10816-022-09586-2)). Simulating the fracture of arbitrary shapes on every strike is unaffordable; an empirical law is not.
  - Keeping elements and energy exactly balanced (`MAT-09`) drifts with ordinary decimal arithmetic. It needs whole-number bookkeeping, which also helps determinism.
  - If every flake is a "thing" (`MAT-10`), a band leaves thousands to tens of thousands a year: 10⁷–10⁸ things over millennia. Traces have to be stored as deposits with counts.
- **To make it feasible:** list properties per ingredient and structure, with each source fetched and the supporting passage quoted; keep whole-number accounts; store traces as deposits; restate `MAT-03` as "properties come from measured data and stated rules".

### 4.7 Bodies and lives (section 8): feasible with stated limits
- **Binding constraint:** content and calibration. Compute is small next to minds.
- **Evidence:** energy balance, nutrition, heat loss, dose and response, injuries by body part, disease, pregnancy and ageing all have textbook models with real parameters. Each costs perhaps thousands of operations per person-day. The hard part: life patterns (`BIO-04`, e.g. ~40% of children dying before 15) must emerge from food, disease and injury. That means calibrating many parameters against data.
- **To make it feasible:** state that the `BIO-04` figures are calibration targets for the mechanisms, checked by experiment (`RES-14`).

### 4.8 Minds (section 9): doubtful
- **Binding constraint:** an unsolved research problem, then compute and memory (§2).
- **Evidence:**
  - **General learners need enormous experience.** OpenAI's hide-and-seek agents built shelters after ~25 million games, and found later tricks after ~380 million ([OpenAI](https://openai.com/index/emergent-tool-use)). DreamerV3, the first algorithm to mine diamonds in Minecraft from scratch, needed about nine days of continuous play ([Nature 2025 summary](https://gigazine.net/gsc_news/en/20250403-google-deepmind-dreamerv3/)).
  - **Learners as fast as people get there through hand-built structure.** EMPA matches human learning speed on 90 simple games, but learns inside a hand-written language of game rules ([Tsividis et al. 2021](https://arxiv.org/abs/2107.12544)). The classic design closest to section 9, Drescher's schema mechanism (*Made-Up Minds*, 1991), was shown only in a small simulated microworld.
  - **What Kindling asks:** discovery within ~100 lives × 500 years per world (`RES-03`), with no recipes, by mechanisms general enough to later cover fire, cooking, pottery, farming and metal. No published system shows this. The more structure is built in to make it fast, the closer it comes to the hidden recipes of 3.6.
  - Theory of mind, relationships, seasonal memory, planning, skills, feelings and dreams are each buildable. Together, at 10⁴–10⁵ items per person, they set the memory figures of §2. Ten million detailed farmers is out of reach.
- **To make it feasible:** run prototype 2 (a toy Experiment 0) before building the engine. State population limits per detail level from measurement. Accept that most of a large world runs coarse (3.2).

### 4.9 Culture, language and belief (section 10): doubtful
- **Binding constraint:** unsolved research (grammar, writing), then memory.
- **Evidence:**
  - **Feasible and cheap:** shared word lists emerge reliably in simulated "naming games" ([Baronchelli et al. 2006](https://arxiv.org/abs/physics/0509075v1)). Imperfect copying loses skills in small groups even in models where a skill is one number ([Henrich 2004](https://www2.psych.ubc.ca/~henrich/Website/Papers/HenrichTasmania.pdf); [Powell, Shennan & Thomas 2009](https://discovery.ucl.ac.uk/id/eprint/168654/)). So `CUL-01`, `CUL-02`, `CUL-16`, and words as labels, are feasible.
  - **Grammar is not.** Agents that invent languages usually make codes that work but are neither human-like nor built from reusable parts ([Kottur et al. 2017](https://arxiv.org/abs/1706.08502); survey: [Lazaridou & Baroni 2020](https://arxiv.org/abs/2006.02419)). Structured languages do emerge through repeated learning, shown with human learners in the lab ([Kirby, Cornish & Smith 2008](https://pmc.ncbi.nlm.nih.gov/articles/PMC2504810)), but only over small, fixed sets of meanings.
  - `CUL-04` asks language to do real work: speed up teaching, and talk about absent things and the dead. A listener must then recover a belief from words. That is the hard part.
  - Regular sound change can be simulated: agent models of it exist as research tools ([LMU overview](https://www.phonetik.uni-muenchen.de/Forschung/interaccent/publications/kapia_riverin-coutlee_2024_chapter.pdf)). A rule-based drift is easier but less emergent.
  - Myths, rites, institutions, religion and art can be built in stylised forms: beliefs about hidden causes, superstitious learning, conformity. Whether the results read as rich is unknown. Writing (`CUL-03`) depends on grammar and is far off.
- **To make it feasible:** run prototype 10 before `MIL-05`. Be ready to restate `CUL-17` and `SND-03` as "words and simple word order" if grammar doesn't emerge.

### 4.10 The look: pixel-rendered 3D (section 11.1): doubtful
- **Binding constraint:** one unsolved rendering problem, and what AI agents can reliably polish.
- **Evidence:**
  - **The GPU load is small.** If "about 4 screen pixels" (`PRE-22`) means 4 per side, the picture is ~336 × 748 art pixels (0.25 million); if it means 2 × 2, ~672 × 1496 (1 million). Either is small. 120 Hz is reachable if scenes stay simple, but Tensor GPUs throttle hard (§1).
  - **"Pixels never crawl or shimmer as the camera moves" is not solved** for a freely turning, perspective, continuously zooming camera (`PRE-02`, `PRE-22`). A March 2026 paper states that snapping to a grid works only for orthographic cameras: under perspective, "pixels at different depths drift at different rates, and no single snap corrects all depths". Its own method fixes turning, but leaves gaps behind objects as an open trade-off ([Ebert 2026](https://arxiv.org/abs/2603.14587)). Zooming at a fixed art-pixel size redraws everything at a new scale, so fine detail changes during the zoom.
  - Caves and overhangs (`PRE-24`) and the cut-away view (`PRE-25`) need real 3D ground near people, not just a height map. Doable, but a real system.
  - Figures that perform any action from any angle (`PRE-27`): composing a few dozen recorded basic motions with inverse kinematics is proven. Spore animated user-made creatures this way ([Hecker et al. 2008](https://chrishecker.com/Real-time_Motion_Retargeting_to_Highly_Varied_User-Created_Morphologies)), but it took a specialist team.
  - "Studio craft" (`VIS-05`) judged on a phone by one human, while AI agents can't see the phone's screen, is unproven.
- **To make it feasible:** restate `PRE-22` as "pixels stay locked while panning and at rest; turning and zooming may shift them briefly", or limit the close camera to orthographic with eased turns. Run prototype 8. Have AI agents measure pixel stability automatically on recorded frames.

### 4.11 Story tools and the writer AI (sections 11.2–11.5): feasible with stated limits
- **Binding constraint:** memory, heat and speed, then checking text against data.
- **Evidence:**
  - At roughly 10–50 tokens a second (flagship figures; the Pixel is unmeasured), a 120-word chronicle entry takes ~3–16 seconds and a life story ~15–80 seconds. When "decades pass in seconds", events arrive faster than any phone model can describe them. Text must be written when opened, and then kept.
  - The model takes 0.7–3.3 GB of the 8–10 GiB cap.
  - Gemini Nano won't run in the background, has request and battery quotas, and its text changes between versions ([ML Kit](https://developers.google.com/ml-kit/genai)). Your own Gemma model through LiteRT avoids the quotas, but costs memory.
  - Checking that text adds no facts (`PRN-06`, `PRE-17`) can be automated for names, numbers, places and events. Claims about causes and feelings can't be fully checked automatically.
  - Unknown: whether a safety-tuned small model softens or refuses the darker history the content setting allows (`CUL-08`, `PRE-18`). Prototype 7 tests it.
- **To make it feasible:** write on demand and store the result; write while the simulation is paused or slow; check names and numbers in every output; run prototype 7.

### 4.12 Sound (section 12): feasible with stated limits (voices doubtful)
- **Binding constraint:** content and quality. Voices depend on grammar (4.9).
- **Evidence:**
  - Impact sounds synthesised from material properties ran in real time in 2001 ([FoleyAutomatic](https://www.cs.mcgill.ca/~kry/pubs/foleyautomatic/foleyautomatic.pdf)).
  - Arbitrary invented shapes need a costly vibration analysis per shape. Families of shapes (tube, skin, bar, string) are cheap.
  - Speech in an invented language can be voiced by a formant synthesiser from phoneme strings ([eSpeak NG](https://en.wikipedia.org/wiki/ESpeak)), but it sounds robotic. Natural-sounding voices are trained per real language. "Actual sentences" (`SND-03`) need the grammar of 4.9.
- **To make it feasible:** shape families for instruments; accept a stylised voice; tie `SND-03` to the result of prototype 10.

### 4.13 The phone: memory, heat, battery, start-up, installs (sections 13.1–13.2, `VIS-14`): feasible with stated limits
- **Binding constraint:** the memory cap, then battery and heat.
- **Evidence:**
  - **Memory:** 8 GiB (12 GB phone) or 10 GiB (16 GB phone), shared by simulation, picture and writer ([AOSP](https://source.android.com/docs/core/perf/memory-limiter)).
  - **Heat:** the CPU settles near half its peak under sustained load. "Comfortable for an hour" (`VIS-14`) has no number. At ~5 W in total the battery drains ~25% an hour; at ~8 W, ~40%, and the phone gets hot. Whatever number you choose sets how fast history can run.
  - **Overnight:** Pixels cap charging at 80% under continuous charging with heavy use ([XDA](https://www.xda-developers.com/google-pixel-battery-charging-limit-feature/)). Long, hot, fully charged nights age the battery.
  - **Start-up:** opening in about 3 seconds with gigabytes of world plus a 3.6 GB model needs lazy loading.
  - **Installs (`PLT-06`):** from 2027, certified Android phones worldwide will require apps to come from registered developers. The ways around it are an "advanced flow" (Developer Mode and a 24-hour wait) or a USB cable. A free hobbyist account covers up to 20 devices with no ID or fee, but it is an account ([Android Authority](https://www.androidauthority.com/android-sideloading-changes-timeline-3679204/)).
  - The simulation can't use the GPU or the TPU (4.14). So `PLT-01`'s "use the phone's hardware" helps only the picture and the writer.
- **To make it feasible:** name your RAM tier; put a number on "comfortable" (battery % per hour, no heat warning); run prototype 4.

### 4.14 Phone–cloud identity (`PRN-08`, `RES-05`, `PLT-05`): feasible with stated limits
- **Binding constraint:** cross-device determinism (identical results on both), then what AI agents can reliably maintain.
- **Evidence:**
  - **Achievable on CPUs with discipline.** Box2D v3 gives identical results on x64 and ARM after turning off fused multiply–add and replacing `atan2` ([Box2D](https://box2d.org/posts/2024/08/determinism/)). Rapier's cross-platform mode needs strict IEEE 754 arithmetic and its own maths functions ([Rapier](https://rapier.rs/docs/user_guides/rust/determinism/)). Jolt's cross-platform mode disables fused multiply–add ([Jolt](https://raw.githubusercontent.com/jrouwe/JoltPhysics/master/Build/README.md)).
  - **The traps:**
    - compilers fuse operations differently by default on ARM and x86 ([GCC list, 2023](https://gcc.gnu.org/pipermail/gcc/2023-September/242467.html));
    - maths libraries differ in the last digit ([Zimmermann](https://homepages.loria.fr/PZimmermann/papers/glibc240.pdf));
    - GPUs may differ by vendor, since Vulkan allows sin and cos to be off by up to 2⁻¹¹ ([Vulkan spec](https://registry.khronos.org/vulkan/specs/1.3/html/chap37.html)), and the cloud has no GPU at all.
  - **Experts miss rare paths.** Factorio desynced between ARM Macs and x86 PCs in late 2024, because one rare calculation turned a negative number into an unsigned one, which ARM and x86 handle differently. It was fixed in 2.0.29 ([Factorio forum](https://forums.factorio.com/viewtopic.php?p=629157)).
  - **`PRC-10` can't run as written.** It requires this check before every merge, "on the phone and in the cloud", but AI agents can't reach the phone. GitHub's ARM64 Linux machines are free within the plan's minutes for private repositories since January 2026 (2,000 a month on the Free plan) ([GitHub](https://github.blog/changelog/2026-01-29-arm64-standard-runners-are-now-available-in-private-repositories)). They can stand in for the phone.
  - **Moving experiment worlds to the phone:** GitHub blocks files over 100 MiB in a repository and recommends repositories under 5 GB ([GitHub](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github)). Snapshots must travel as release files, or the phone re-runs from the seed, which takes hours for long histories.
- **To make it feasible:**
  - the simulation runs on the CPU only, with no fused operations and its own maths functions;
  - parallel sums are added in a fixed order;
  - nothing depends on hash order or memory addresses;
  - each tick's state is hashed, to find where two runs part;
  - an ARM64 check runs on every merge, and the phone self-checks at each install;
  - run prototype 3.

### 4.15 Experiments in cloud sessions (`SCP-15`, section 14): doubtful
- **Binding constraint:** compute.
- **Evidence:**
  - §2: Experiment 1 needs from ~80 days (lean) to ~2 years (middle) of a 4-core session.
  - A background job here stops after 2 hours, so long runs must save and resume.
  - How long sessions live, and how many can run at once, isn't documented.
  - Later experiments (languages drifting apart over centuries, domestication) are longer still.
  - `SCP-15`'s escape clause (raise it with you) will very likely trigger at `MIL-02`.
- **To make it feasible:** run prototypes 1 and 6 before `MIL-02`. Allow sequential testing, which stops early once the result is clear. Stop each world a set time after its discovery. State a computing budget per experiment.

### 4.16 Building it with AI agents (section 15, `VIS-05`): feasible with stated limits
- **Binding constraint:** what AI agents can reliably build and maintain.
- **Evidence:**
  - In May 2026 the best AI agent completed software tasks that take a human expert 16 hours or more half the time, and ~3-hour tasks four times in five ([METR, via Wikipedia](https://en.wikipedia.org/wiki/METR)). A project this size is thousands of such tasks, so failures are routine and must be caught.
  - Determinism, the reality checklist and the rules check are mechanical, and suit AI review. Three things don't:
    - whether an emergent behaviour is right, since there is no expected answer to test against;
    - visual and sound polish, since no AI sees the phone;
    - sourcing ~10,000 numbers.
  - A second AI reviewer from the same family shares the first one's blind spots. Polish iterates only as fast as your time on the phone allows.
- **To make it feasible:** small tasks with tests; per-tick hashing and tools to find divergence; every sourced value checked against the fetched page, with a quote; milestone visual reviews on the phone by you.

---

## 5. Risks missing from section 16

All *Proposed*:
- **Looking changes history** (3.1). Likelihood high if unchanged; impact high.
- **Experiments too big for sessions** (4.15). High; high.
- **The memory cap** (4.13). High; medium.
- **Old rule versions needed to replay the past** (3.4). High; medium.
- **Fabricated or mismatched sources in AI-written catalogues** (4.6). Medium; medium.
- **The writer model softening dark history** (4.11). Medium; low.
- **Install rules from 2027** (4.13). Medium; low.

## 6. Proposed changes for you to confirm, change or drop

All *Proposed* (`PRC-07`). None has been made to PROJECT.md.

1. **`PRN-11`, `MND-14`, `WLD-12`, `WLD-13`:** detail is set by a rule that depends only on the world and your recorded interventions, never on the camera.
2. **`PRN-11`:** a coarse model must match the full model's statistics in an experiment before it is used.
3. **`PRN-05`:** "a real measurement, or a stated rule applied to real measurements; both with sources".
4. **`PLT-09`, `TIM-06`:** rules versioned separately from the app; old rules kept runnable for a stated number of versions; earlier history viewable from saved points.
5. **`PRC-10`, `RES-05`:** an ARM64 machine stands in for the phone on every merge; the phone self-checks at each install and at each milestone.
6. **`PRE-22`:** pixels stay locked while panning and at rest; brief shifts are allowed while turning and zooming.
7. **`RES-13`, `RES-03`:** sequential testing, a stop rule after discovery, and a computing budget per experiment.
8. **`PLT-01`:** name the RAM tier (12 GB or 16 GB).
9. **`VIS-14`, `PLT-04`:** put a number on "comfortable for an hour".
10. **`PRN-13`:** explanations of the past are rebuilt by re-running, which can take time.
11. **`PRN-07`:** add the decoy-and-novel-material test of 3.6 to the general-rules check.
12. **Section 16:** add the risks in §5.

---

## 7. Prototypes to run first, ranked

These are throwaway programs, each built to measure one thing and then deleted.

**0. Decide the detail rule (no code).** §3.1 is a contradiction in the principles, not an unknown. Prototypes 1, 2 and 5 assume detail depends only on the world.

**1. The cost of a mind**
- **Measures:** milliseconds of one core per person per simulated day, and MB per person. It uses a stripped-down perceive–remember–decide–learn loop with realistic memory sizes (10⁴–10⁵ items), on the phone (30 minutes sustained, logging temperature) and in a cloud session.
- **Size:** ~1,500 lines; a few agent-days; you install one build.
- **Changes the plan if:**
  - over ~1 ms a day: Experiment 1 can't fit in one session, so shrink or parallelise it;
  - over ~5 ms a day: the overnight example and "a season a minute" fail for a few hundred people, so the coarse model becomes essential;
  - over ~1 MB per person: the detailed population caps in the low thousands.

**2. Experiment 0: a toy sharp-stone world**
- **Measures:** whether flake-making is discovered and spreads in 100 toy 2D worlds. The worlds have ~10 materials described only by properties, generic body actions with continuous force and angle, a property-based fracture law, and the planned learning mechanisms. Then it repeats the test with 10 decoy materials and actions, and with a made-up material whose useful property nobody designed for.
- **Size:** 3,000–5,000 lines; one to two agent-weeks; cloud only.
- **Changes the plan if:**
  - discovery happens only when senses and actions are shaped around knapping, or collapses with decoys: rethink section 9 before building the engine;
  - it never happens: `RSK-01` has arrived, cheaply.

**3. Phone–cloud determinism**
- **Measures:** whether 10⁶ steps of a small simulation give bit-identical state on the phone, in the cloud and on a GitHub ARM64 machine. The simulation uses decimals, sin and exp, threads, maps and random numbers. It also checks that default builds diverge as expected and that the guard rails catch it, and what the rules cost in speed.
- **Size:** 1,000–2,000 lines; two to three agent-days.
- **Changes the plan if:**
  - a mismatch can't be traced within a day: use whole-number arithmetic for all simulation state;
  - ARM64 Linux always matches the phone: it becomes the per-merge stand-in;
  - the rules cost over ~20%: budget for it.

**4. The phone's real budget**
- **Measures:** 60 minutes on the phone running three things at once: simulated load on N cores, a 120 Hz pixel-3D test scene, and periodic text writing. It logs temperature status, CPU speeds, frame times, battery % per hour, the memory cap (`am memory-limiter status`), and whether memory-mapped files count against it. Once on battery, once overnight on the charger.
- **Size:** ~1,000 lines plus a test scene; you run it twice.
- **Changes the plan if:**
  - fewer than ~2 cores' worth survive "comfortable": halve every speed in §2;
  - the cap is 8 GiB rather than 10: that decides between Gemma E2B and E4B;
  - GPU throttling pulls below 120 Hz: lower the target.

**5. Coarse versus full minds**
- **Measures:** whether a band-level model (habits plus shared knowledge) reproduces the full model's discovery, spread, loss and population numbers in prototype 2's world, and how big the jumps are when switching between them.
- **Size:** 1,000–2,000 lines, on top of prototype 2.
- **Changes the plan if:** no coarse model matches. Then far-zoom speeds and large populations can't be had with depth: restate `TIM-01`, `VIS-07` and `WLD-04`.

**6. How much computing the sessions really give**
- **Measures:** how long a background job survives in an AI cloud session, whether it resumes after interruption, how many sessions can run at once, and how many CPU-hours a week that gives at no extra cost.
- **Size:** one script; a few days elapsed.
- **Changes the plan if:** under ~100 CPU-hours a week. Then Experiment 1 at full size needs other computers: raise `SCP-15` now.

**7. The writer on the phone**
- **Measures:** words per second, memory and heat for Gemma 4 E2B and E4B (GPU and TPU) and Gemini Nano, on 50 hand-built event bundles. Also: how often text adds facts (an automatic check of names, numbers, places and events), refusals or softening on dark events, and your rating of 30 entries.
- **Size:** ~800 lines plus the bundles; one hour of your time.
- **Changes the plan if:**
  - more than a few percent of entries add facts: use fixed sentence frames, with the model only smoothing them;
  - you rate the writing flat: raise `PRE-37` now rather than at `MIL-05`.

**8. Stable pixels with a turning camera**
- **Measures:** the share of art pixels that change between frames while only the camera moves (pan, turn, zoom), orthographic versus perspective, on the phone at 120 Hz; plus your judgement.
- **Size:** ~1,500 lines; one scene (a cliff, a cave, a person).
- **Changes the plan if:** crawl while turning or zooming bothers you. Then limit the close camera, or restate `PRE-22`.

**9. Snapshot size and rewind time**
- **Measures:** compressed snapshot size, its growth per simulated day and year, and the time to re-run to any moment, using prototype 1's minds.
- **Size:** ~500 lines.
- **Changes the plan if:** snapshots exceed ~100 MB. Then "any moment" becomes "any saved moment, or a wait".

**10. A language toy**
- **Measures:** whether naming games, repeated learning across generations and sound drift together give shared words; a word order that carries "who did what to what" well enough to pass on a belief; and regular sound correspondences after a split. Also memory per speaker.
- **Size:** ~2,000 lines; cloud only.
- **Changes the plan if:** no usable grammar emerges. Then restate `CUL-17` and `SND-03` before `MIL-05`.

---

## Sources

**Phone and Android**
- [Android Authority: Tensor G6 tested](https://www.androidauthority.com/tensor-g6-benchmarks-3699714/)
- [Gizbot: Tensor G6 vs G5, CPU throttling](https://www.gizbot.com/features/google-tensor-g6-vs-tensor-g5-benchmarks-tested-and-compared-128059.html)
- [Notebookcheck: Pixel 11 Pro GPU throttling](https://www.notebookcheck.net/Google-Pixel-11-Pro-and-Tensor-G6-showcase-severe-GPU-thermal-throttling-in-hands-on-benchmark-tests.1373783.0.html)
- [Engadget: Pixel 11 RAM tiers](https://engadget.com/2234960/pixel-11-series-cost-more-but-less-ram)
- [droid-life: Pixel 11 specs](https://www.droid-life.com/2026/07/13/pixel-11-series-specs-storage-ram-displays-battery/)
- [AOSP: Memory Limiter](https://source.android.com/docs/core/perf/memory-limiter)
- [ecorpit: memory limits rollout](https://ecorpit.com/android-app-memory-limits-oem-rollout-vitals-2026/)
- [XDA: Pixel charging limit](https://www.xda-developers.com/google-pixel-battery-charging-limit-feature/)
- [Android Authority: sideloading timeline](https://www.androidauthority.com/android-sideloading-changes-timeline-3679204/)

**Writer AI**
- [LiteRT-LM: Gemma 4](https://developers.google.com/edge/litert-lm/models/gemma-4)
- [Google: Tensor SDK beta](https://developers.googleblog.com/google-tensor-sdk-beta-with-litert/)
- [ML Kit GenAI APIs](https://developers.google.com/ml-kit/genai)

**Determinism**
- [Box2D: Determinism](https://box2d.org/posts/2024/08/determinism/)
- [Rapier: Determinism](https://rapier.rs/docs/user_guides/rust/determinism/)
- [Jolt: build options](https://raw.githubusercontent.com/jrouwe/JoltPhysics/master/Build/README.md)
- [GCC mailing list: -ffp-contract default](https://gcc.gnu.org/pipermail/gcc/2023-September/242467.html)
- [Zimmermann: accuracy of maths libraries](https://homepages.loria.fr/PZimmermann/papers/glibc240.pdf)
- [Vulkan: SPIR-V precision](https://registry.khronos.org/vulkan/specs/1.3/html/chap37.html)
- [Factorio: ARM/x86 desync](https://forums.factorio.com/viewtopic.php?p=629157)
- [GitHub: ARM64 runners in private repositories](https://github.blog/changelog/2026-01-29-arm64-standard-runners-are-now-available-in-private-repositories)
- [GitHub: large files](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github)

**Minds, culture and language**
- [OpenAI: emergent tool use](https://openai.com/index/emergent-tool-use)
- [DreamerV3 (Nature 2025) summary](https://gigazine.net/gsc_news/en/20250403-google-deepmind-dreamerv3/)
- [Tsividis et al. 2021](https://arxiv.org/abs/2107.12544)
- [Henrich 2004](https://www2.psych.ubc.ca/~henrich/Website/Papers/HenrichTasmania.pdf)
- [Powell, Shennan & Thomas 2009](https://discovery.ucl.ac.uk/id/eprint/168654/)
- [Baronchelli et al. 2006](https://arxiv.org/abs/physics/0509075v1)
- [Kottur et al. 2017](https://arxiv.org/abs/1706.08502)
- [Lazaridou & Baroni 2020](https://arxiv.org/abs/2006.02419)
- [Kirby, Cornish & Smith 2008](https://pmc.ncbi.nlm.nih.gov/articles/PMC2504810)
- [LMU: agent models of sound change](https://www.phonetik.uni-muenchen.de/Forschung/interaccent/publications/kapia_riverin-coutlee_2024_chapter.pdf)

**World, matter, picture and sound**
- [ExoPlaSim](https://arxiv.org/pdf/2107.07685)
- [WorldEngine](https://github.com/esampson/worldengine)
- [Li et al. 2022: flake formation experiments](https://link.springer.com/article/10.1007/s10816-022-09586-2)
- [Ebert 2026: perspective-stable 3D pixel art](https://arxiv.org/abs/2603.14587)
- [Hecker et al. 2008: Spore animation](https://chrishecker.com/Real-time_Motion_Retargeting_to_Highly_Varied_User-Created_Morphologies)
- [FoleyAutomatic 2001](https://www.cs.mcgill.ca/~kry/pubs/foleyautomatic/foleyautomatic.pdf)
- [eSpeak NG](https://en.wikipedia.org/wiki/ESpeak)

**Games and AI builders**
- [Dwarf Fortress wiki: frame rate](https://dwarffortresswiki.org/index.php/v0.34:Maximizing_framerate)
- [Steam: Dwarf Fortress population](https://steamcommunity.com/app/975370/discussions/0/6063574513309438286)
- [Steam: Crusader Kings III characters](https://steamcommunity.com/app/1158310/discussions/0/3727323721760166023)
- [METR time horizons (Wikipedia)](https://en.wikipedia.org/wiki/METR)
- [Walters & Wilder 2023: fabricated citations](https://doaj.org/article/fc88ea07ec994a7c8a144f8516c48cbf)
