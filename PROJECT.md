# Project Nature

The project file: what Project Nature is, and every feature and target it must reach. It is written so that someone with no prior context can read it and understand the whole project.

It contains no implementation details. Those belong in the implementation plan, which will link back to this file by ID, as will the code. Every item has a permanent ID so that no feature gets lost on the way from idea to code.

## Contents

- [How this file works](#how-this-file-works)
- [1. Vision](#1-vision)
- [2. Principles](#2-principles)
- [3. Scope and non-goals](#3-scope-and-non-goals)
- [4. The player as god](#4-the-player-as-god)
- [5. Time and history](#5-time-and-history)
- [6. World](#6-world)
- [7. Matter and physics](#7-matter-and-physics)
- [8. People: bodies and lives](#8-people-bodies-and-lives)
- [9. Minds](#9-minds)
- [10. Culture and society](#10-culture-and-society)
- [11. Presentation](#11-presentation)
- [12. Sound](#12-sound)
- [13. Platform and performance](#13-platform-and-performance)
- [14. Research and validation](#14-research-and-validation)
- [15. Project and process](#15-project-and-process)
- [16. Risks](#16-risks)
- [17. Not yet decided](#17-not-yet-decided)
- [18. Glossary](#18-glossary)

---

## How this file works

### Status

| Status | Meaning |
|---|---|
| *Decided* | Agreed in our sessions. |
| *Proposed* | Suggested but not yet confirmed. To be confirmed or changed in that section's deep dive. |
| *To test* | Settled by experiment or measurement, not by opinion. |
| *Open* | Not decided yet. |
| *Dropped* | No longer planned. Kept for the record, with the reason. |

### Item format

Every item starts with its ID, a short name and its status. Detailed items then add some of the following:

- **What:** what it is, in plain words.
- **Why:** the reason it exists.
- **Example:** a concrete illustration.
- **Done when:** checks that prove it has been delivered. The implementation plan and the tests link to these. A check marked *(Proposed)* is a suggested target awaiting confirmation.
- **Check:** for rules that always apply, such as principles, how we verify they are being followed.

### IDs and links

1. Every feature and target has a permanent ID: an area code plus a number, such as `WLD-01`.
2. IDs are never renumbered or reused. A new item takes the next free number in its area, wherever it sits in the text.
3. Items are never deleted. If something is cut, its status becomes *Dropped* with a one-line reason.
4. The implementation plan (a separate file, still to come) names the IDs each task delivers. Every ID that isn't *Dropped* must appear in at least one task.
5. Code and tests name the IDs they implement, so any feature can be followed from this file to the plan to the code, and back.

### Area codes

| Code | Area |
|---|---|
| `VIS` | Vision |
| `MOM` | Signature moments (part of the vision) |
| `PRN` | Principles |
| `SCP` | Scope and non-goals |
| `MIL` | Milestones (part of scope) |
| `GOD` | The player as god |
| `TIM` | Time and history |
| `WLD` | World |
| `MAT` | Matter and physics |
| `RCK` | Reality checklist |
| `BIO` | People: bodies and lives |
| `MND` | Minds |
| `CUL` | Culture and society |
| `PRE` | Presentation |
| `SND` | Sound |
| `PLT` | Platform and performance |
| `RES` | Research and validation |
| `PRC` | Project and process |
| `RSK` | Risks |

---

## 1. Vision

This section says what Project Nature is, what it feels like, and what success means. Every other section serves it.

### 1.1 The game in brief

- `VIS-01` **In one sentence** *(Decided)*: A bottom-up simulation of humanity on a generated Earth-like world, where a few bands of early humans living in caves learn, entirely by themselves, to survive, build, believe and organise.

- `VIS-06` **In one paragraph** *(Decided)*: Project Nature simulates a whole world from the ground up: rock, water, weather, plants, animals and people. It begins with a few bands of early humans sheltering in caves. They have modern brains but almost no culture: a handful of words, no way to make fire, nothing but rough stones and sticks. Nothing tells them what to do. There are no recipes, no tech tree and no list of eras to unlock. They learn the way real people learned: by noticing, trying, failing, copying, teaching and dreaming. Everything they ever achieve, from a sharp flake of stone to rituals, languages, farms and perhaps cities, has to come from what they discover in the world and pass on to each other. You watch it all on your phone as an invisible force of nature. You can nudge the weather, luck and dreams, but you can never command anyone.

- `VIS-02` **The fantasy** *(Decided)*: You are nature.
  - **What:** You are the weather, the luck and the dreams. You can send a storm, bless a hunt, or let someone dream two of their own memories side by side. You can't speak, appear or work miracles, and the people of the world never learn you exist.
  - **Why:** A god who can't command anyone leaves every achievement theirs. Whatever gods they come to believe in are their own explanations of the world, and sometimes of you.
  - **Example:** A lightning strike you send to start a wildfire becomes, generations later, the myth of the storm spirit who first gave them fire.

### 1.2 What it feels like

- `VIS-07` **Wonder** *(Decided)*
  - **What:** Awe at a world that runs itself and keeps surprising you, its maker included.
  - **Why:** Nothing is authored. The rules are known; what they produce is not.
  - **Example:** You zoom out from one campfire to the whole world and watch a thousand years of migrations, languages and beliefs move across the land like weather.

- `VIS-08` **Curiosity** *(Decided)*
  - **What:** The urge to understand why something happened, and to try "what if".
  - **Why:** Every event has real causes, and the game lets you find them: the scientist's view of a mind (`PRE-14`), the buried layers of a site (`PRE-09`), and rewinding and branching history (`TIM-06`).
  - **Example:** A band abandons its cave. You look into their minds and find a run of failed hunts and a belief that the cave turned against them after a death. You rewind, send a good hunting season, and see whether they stay.

- `VIS-09` **Other feelings** *(Proposed)*: Attachment to particular people, and the harshness of nature, will arise from the simulation and are welcome, but the design isn't built around them. When design choices conflict, wonder and curiosity decide.

### 1.3 How you play

- `VIS-10` **Two rhythms of play** *(Decided)*
  - **Short check-ins (5–15 minutes):** open the app, catch up on the latest live moments, follow someone for a while, nudge, close.
  - **Long sessions (an hour or more):** watch an era unfold at speed, read the chronicle, dig through the past, branch a "what if" and compare the outcomes.
  - **Why it matters:** both must feel natural. A check-in can't require any setup, and a long session needs tools for depth.
  - The world pauses when the app is closed (`TIM-05`), so every session starts exactly where the last one ended.

- `VIS-11` **A session, as a story** *(Proposed; an illustration, not a script)*

  > You open the app. The world is exactly where you left it: late autumn in the valley of two rivers, year 2,314. A live moment is waiting: *the eastern band has lost its fire*. You zoom in, and time slows to walking pace. The camp is cold; children huddle under hides; wolves circle at the edge of the scree.
  >
  > You could send a dry spell to the forest on the ridge and hope lightning finds it. Instead you look through the memories of Ama, the band's most curious woman. Last summer, boring a hole in a piece of wood, she saw the stick begin to smoke. You give her a dream that sets that smoking stick beside the warmth of a fire. The next morning she is twirling sticks. It takes her eleven days.
  >
  > You zoom out, and decades pass in seconds. On the knowledge overlay, fire-making spreads from band to band along the river. In the chronicle, the story is already being retold as myth: *Ama stole the fire that sleeps inside the wood*. You close the app, and the world waits for you.

### 1.4 Signature moments

- `VIS-12` **Signature moments** *(Proposed)*: Stories the simulation must be able to produce. None of them is scripted. Each is an example of what the rules should make possible, and each becomes a long-term test (the `MOM` items below). The IDs in brackets are the parts of the project each moment depends on.

  - `MOM-01` **Fire from wood**: In a hard winter, a band whose fire has died learns to make fire by friction. (`MND-11`, `RCK-02`, `GOD-03`)
  - `MOM-02` **The lost craft**: A fever kills a band's best stoneworkers. For generations its blades are cruder, until the skill is rediscovered or learned again from neighbours. (`CUL-01`, `CUL-02`)
  - `MOM-03` **Your lightning becomes a god**: A lightning strike you sent kills a hunter on a hilltop. The band avoids the hill, then leaves offerings there, then tells stories about the one who lives in the storm. (`GOD-02`, `GOD-06`, `CUL-05`)
  - `MOM-04` **The song that does nothing**: A band sings before a hunt that goes well. The song becomes a hunting rite and is kept for centuries, though it changes nothing. (`MND-05`, `CUL-06`)
  - `MOM-05` **Two tongues**: Two bands are separated by a rising sea and drift apart in speech. When their descendants meet again, they can hardly understand each other. (`CUL-04`, `WLD-16`)
  - `MOM-06` **The camp wolf**: The boldest wolves scavenge at the edge of camp. Their pups grow tamer each generation, until a child raises one. (`MND-16`, `WLD-20`)
  - `MOM-07` **A painting that remembers**: A painting of a great hunt outlasts everyone who saw it. You tap it and see the hunt. (`CUL-09`, `PRE-15`)
  - `MOM-08` **Seeds on the rubbish heap**: Seeds thrown on a rubbish heap sprout near camp. Years later, someone starts planting on purpose. (`MND-11`, `WLD-18`)
  - `MOM-09` **The dig**: Under a village, you find the hearths of the first band and the bones of the animals they ate. (`MAT-08`, `PRE-09`)
  - `MOM-10` **Two endings**: You rewind to before a plague, send a mild winter instead, and compare two histories of the same people. (`TIM-06`)
  - `MOM-11` **Rivals, then in-laws**: Two bands fight over a valley, then marry into each other. Each side's descendants tell the story differently. (`CUL-07`, `CUL-11`)
  - `MOM-12` **Metal from green stone**: A kiln built very hot for pottery leaves a bead of shiny metal where green stones lined the fire, and someone notices. (`MAT-07`, `RCK-08`)

### 1.5 The arc of a world

- `VIS-03` **No ceiling** *(Decided)*
  - **What:** There are no eras, levels or end state. A world's history goes as far as its people take it.
  - **Why:** Any fixed sequence of eras would be a tech tree in disguise.
  - **In practice:** some worlds may stall for tens of thousands of years, and some bands will die out. Some peoples may reach farming, writing, metals and beyond; some may take paths our own history never took. Nothing about the order of our history is guaranteed, except where physics forces it: no one smelts copper without a fire hot enough. Collapse, stagnation and extinction are all valid histories.

### 1.6 What makes it different

- `VIS-13` **Seven differences** *(Decided; a summary of decisions made in other sections)*
  - **No recipes.** Discoveries come from physics, not from lists (`PRN-01`, section 7).
  - **Minds that learn.** People form their own concepts, beliefs and skills. Science and superstition come from the same mechanism (section 9).
  - **Real matter.** Real chemistry and real-world numbers decide what is possible (section 7).
  - **You are nature.** An invisible god, limited to what nature could do (section 4).
  - **Every story can be traced.** Two views of every mind, archaeology, and rewinding and branching history (sections 5 and 11).
  - **Rigour behind the wonder.** Experiments that can fail decide what the simulation really does (section 14).
  - **In your pocket.** Designed for one phone, with detailed pixel art and one continuous zoom from the whole world to a single person (sections 11 and 13).

### 1.7 Inspirations

- `VIS-04` **Inspirations** *(Decided)*: What we take from each, and where we differ.
  - **[world-sim](https://world.world-sim.uk):** a living world whose villagers discover fire, pottery and bronze for themselves, with named souls, graves and a book of ages. *We take* its care for individual lives and a history worth reading. *We differ:* we start much earlier, simulate a far deeper physical world, have no tech tree or list of eras, and run on a phone.
  - **Dwarf Fortress:** deep simulation, and generated legends you can read. *We take* history as the main product. *We avoid* an interface that hides its stories.
  - **RimWorld:** stories that emerge from the simulation, paced by an AI storyteller. *We take* its care for pacing. *We differ:* our story director only controls the speed of time; it never creates events (`TIM-03`).
  - **WorldBox:** a pixel-art god sandbox made for phones. *We take* the joy of a living world in your hand. *We differ:* a far deeper simulation, and powers limited to what nature could do.
  - **Black & White:** a god whose acts shape what villagers believe. *We differ:* there is no worship and no visible god.
  - **Ancestors: The Humankind Odyssey:** early humans learning by experimenting. *We take* the thrill of discovery by trial. *We differ:* nobody is controlled, and discoveries come from physics, not from an unlockable skill tree.
  - **Noita and falling-sand games:** matter that follows rules, so interactions nobody designed still work. *We take* rules over recipes. *We differ:* matter is described by its chemistry and structure, not simulated grain by grain.
  - **Science:** research on cultural evolution, cognition, the origins of religion and the emergence of language. Each source is cited in the section that uses it.

### 1.8 Success

Who it's for: you alone (`SCP-02`). Success is judged by the experience; the research rigour of `VIS-05` is how we get there.

- `VIS-14` **A joy on the phone** *(Decided)*
  - **What:** Beautiful, smooth and absorbing in your hand.
  - **Done when** *(Proposed; exact limits set from the measurements in `PLT-04`)*:
    - zooming and panning stay smooth at the screen's full refresh rate, at every zoom level;
    - the app opens to your world, ready to play, within about three seconds;
    - an hour's session stays comfortable for battery and heat;
    - every screen works one-handed in portrait and two-handed in landscape.

- `VIS-15` **Histories worth reading** *(Decided)*
  - **What:** Every world produces a history you would want to read, and no two are alike.
  - **Done when** *(Proposed)*:
    - in milestone reviews, you'd choose to read a world's chronicle for pleasure;
    - worlds from different seeds tell clearly different stories;
    - every chronicle entry can be traced back to the simulated events behind it.

- `VIS-05` **Quality bar** *(Decided)*: The rigour of a research project and the craft of a well-funded studio.
  - **Research rigour:** what the simulation is claimed to do is tested by experiments that can fail, across many worlds, with real-world values and repeatable results.
  - **Studio craft:** art, sound, interface and performance polished to the standard of a well-funded studio.
  - Rigour is the method, not the goal. It exists so that the wonder is earned and the histories are real.

### 1.9 Name

- `VIS-16` **Name** *(Open)*: "Project Nature" is the working title. The final name is chosen once the rest of this file is complete.

## 2. Principles

These rules apply to every part of the project, and they outrank everything else in this file. If any decision conflicts with a principle, the principle wins. A principle changes only if you change it here. Every milestone review goes through the principles using the **Check** line under each one.

### 2.1 The world

- `PRN-01` **The world is the only teacher** *(Decided)*
  - **What:** Everything the people of the world know, they learned inside it: from their senses, their own trials, other people or their dreams. Nothing is handed to them. There are no recipes, no tech tree, no scripted discoveries, and no knowledge given at the start beyond the starting kit (`BIO-02`).
  - **Why:** This is the heart of the project. A discovery only means something if it was really made.
  - **Example:** Nobody tells a band that flint makes good blades. Someone strikes one stone against another, notices a sharp edge, and over time the band learns which stones break that way.
  - **Check:** every discovery in an experiment can be traced back to the experiences that produced it, and the general-rules check (`PRN-07`) passes.

- `PRN-02` **Depth over breadth** *(Decided)*
  - **What:** A small world simulated deeply beats a large shallow one. When choosing between more things and deeper things, choose deeper.
  - **Why:** Discovery, belief and history all come from detail. A shallow world can't surprise anyone.
  - **Example:** The world is about 1,000 km from pole to pole (`WLD-03`), not the size of Earth, so the effort goes into what's actually there.
  - **Check:** any feature that adds breadth at the cost of depth needs an explicit reason in the implementation plan.

- `PRN-07` **General rules only** *(Decided)*
  - **What:** Everything in the world (matter, living things, minds, societies) follows general rules. No rule is ever written for one particular discovery, material, species or event. The name of a discovery (flake, knapping, fire-making, pottery and so on) never appears in the logic that decides what people or animals do. Those words appear only in descriptions of matter and in text written for you.
  - **Why:** A rule written for one outcome is a recipe in disguise. General rules are also what let the world produce things nobody planned.
  - **Example:** There is no "make pottery" rule. Clay changes when heated past a certain temperature, just as the general law of heat says any material can. Pottery is what people make of that.
  - **Check:** an automated search of the decision-making logic finds no discovery vocabulary, and reviews flag any rule that applies to only one material, species or event.

- `PRN-05` **Real numbers, testable claims** *(Decided)*
  - **What:** Every quantity in the world comes from real-world measurements: temperatures, hardness, energy, growth rates, how fast genes change. Every claim about what the simulation produces is tested by experiments that can fail, across many worlds.
  - **Why:** Real numbers make discoveries meaningful: copper really does need a furnace. Experiments that can fail stop us fooling ourselves.
  - **Example:** "Bands discover how to chip stone" is accepted as true only after Experiment 1 passes its criteria across 100 worlds (`RES-03`).
  - **Check:** every value names its real-world source, and every claim in a milestone report is backed by an experiment.

- `PRN-12` **Speed up time, never bend the rules** *(Decided)*
  - **What:** Pacing only ever comes from controlling time: zoom, the story director, and manual speed (section 5). The world's rules never change during play to make things faster or more dramatic. Dials that bend the rules, such as faster evolution (`BIO-07`), exist only for experiments.
  - **Why:** If the rules bent for drama, nothing the world produced could be trusted, and its histories would stop being real.
  - **Example:** Real genetic change is slow, so you won't see minds evolve in a single evening. To watch that, you run an experiment with the evolution dial turned up, clearly labelled as such.
  - **Check:** play has no rule-bending settings, and every experiment report lists any dial that was changed.

### 2.2 The player

- `PRN-03` **You are nature** *(Decided)*
  - **What:** The player acts only through natural means (`GOD-05`) and is never known to exist (`GOD-06`).
  - **Why:** A god who could command people or appear to them would make every achievement partly yours. Every belief about gods would be true, instead of theirs.
  - **Example:** You can't hand a band fire. You can make lightning strike a dry tree near their camp.
  - **Check:** every power produces only events the world could produce on its own.

### 2.3 What you see

- `PRN-04` **If the simulation knows it, you can see it** *(Decided)*
  - **What:** Anything the simulation keeps track of can be shown to you: maps of beliefs, family trees, buried layers, a person's memories.
  - **Why:** Curiosity (`VIS-08`) needs ways to find out why. A rich world you can't look into is wasted.
  - **Example:** The simulation tracks who taught whom to chip stone, so you can see that chain as a family tree of knowledge.
  - **Check:** everything the simulation keeps track of has at least one view that shows it, if only in the scientist's view (`PRE-14`).

- `PRN-10` **Nothing is faked** *(Decided)*
  - **What:** Everything you see, hear or read reflects what actually happened in the simulation. When you zoom in, detail can be filled in, but it never contradicts what was simulated and never invents events for show.
  - **Why:** Histories are only worth reading (`VIS-15`) if they are true to the world. Curiosity only works if every clue is real.
  - **Example:** Zooming into a camp that was being simulated in less detail, the game can show people walking between shelters. It cannot show a fight that never happened.
  - **Check:** every live moment, chronicle entry and on-screen event can be traced back to a simulated event.

- `PRN-13` **Every choice can be explained** *(Decided)*
  - **What:** Why anyone, person or animal, did something can always be traced to their beliefs, drives and memories, and shown in the scientist's view (`PRE-14`). Fine physical detail, such as the exact force of a strike, can simply be numbers.
  - **Why:** Curiosity and research both depend on asking "why?" and getting a real answer.
  - **Example:** Why did Ama walk to the river at dawn? She was thirsty, and she believes the river is safe at dawn because she has never seen wolves there at that hour.
  - **Check:** for any action in any run, the scientist's view shows the beliefs, drives and memories behind it.

- `PRN-06` **AI language models describe, never decide** *(Decided)*
  - **What:** AI language models are used only to turn simulation data into readable text: the chronicle, life stories, myths, dreams. They never choose, invent or know anything for the people or animals of the world, and never add facts the simulation doesn't contain (`PRE-17`).
  - **Why:** A language model knows our history. If it did their thinking, our knowledge would leak into their world and their discoveries would no longer be theirs.
  - **Example:** The model can tell you, in the voice of their tradition, how Ama "stole the fire that sleeps inside the wood". It cannot decide that she tries twirling sticks.
  - **Check:** nothing a language model writes ever feeds back into the simulation, and its descriptions are checked against the data they came from.

### 2.4 How it runs

- `PRN-08` **Same seed, same history** *(Decided)*
  - **What:** A world's history is fully determined by its seed and your interventions. Running it again gives exactly the same history, on the phone or in the cloud.
  - **Why:** Rewinding and branching (`TIM-06`), experiments in the cloud (`PLT-05`) and replays on the phone all depend on it. It also means any strange result can be reproduced and investigated.
  - **Example:** An experiment in the cloud finds a world where fire-making is discovered in year 41. You open that world on your phone and watch year 41 happen exactly as reported.
  - **Check:** automated runs from the same seed and interventions produce identical histories on the phone and in the cloud (`RES-05`).

- `PRN-11` **Time slows, depth stays** *(Decided)*
  - **What:** The screen never stutters. When the phone can't keep up, the simulation doesn't cut corners; history simply moves more slowly. The only simplification allowed is the planned one: less detail for what no one is watching (`WLD-12`, `MND-14`), restored without contradiction when you look (`PRN-10`).
  - **Why:** Depth is the point of the project (`PRN-02`), and a smooth screen is part of the joy on the phone (`VIS-14`). Slowing time protects both.
  - **Example:** A fight breaks out between two bands while you watch. The phone works harder, so a day takes longer to pass, but everyone in the fight is still fully simulated and the screen stays smooth.
  - **Check:** measurements show no stutter under heavy load (`PLT-04`), and what you're watching is simulated in the same detail whatever the load.

### 2.5 How it's built

- `PRN-09` **Only as deep as the next experiment needs** *(Decided)*
  - **What:** Each system is built to the depth the next experiment requires, on foundations that can go deeper later without starting over. This sets the order of work, not the ambition: in the end, every system is as deep as `PRN-02` asks.
  - **Why:** "No ceiling" plus "everything deep" could never be finished all at once. Building in the order the experiments need keeps the project moving and every step testable.
  - **Example:** Experiment 1 (sharp stone) needs to know how stone breaks, not how metal is smelted. Smelting waits until an experiment needs it, but matter is designed from the start so it can be added without rework.
  - **Check:** every task in the implementation plan names the experiment or feature that needs it.

- `PRN-14` **Modular by design** *(Decided)*
  - **What:** Every system grows by adding self-contained pieces (materials, laws, species, behaviours, views, checks), never by rewriting what already works. Adding something should be easy and effortless.
  - **Why:** A project with no ceiling grows forever, and only a modular one stays buildable.
  - **Example:** Adding tin ore to the world needs one new ingredient entry and its checks. Smelting tin already works, because the smelting law never named copper.
  - **Check:** every milestone report lists what was added and confirms that nothing earlier had to be rewritten, or explains why it had to be.

## 3. Scope and non-goals

This section sets the boundaries of the project: what it includes, where history starts, who it's for, how it gets built, and what it deliberately leaves out.

### 3.1 What the project includes

- `SCP-13` **The whole project at a glance** *(Decided; a summary of the sections that follow)*
  - **A generated world** (section 6): a small planet that wraps around, with real geology, climate, weather, water, soils, plants and animals.
  - **Real matter** (section 7): everything is made of real ingredients and changed by general laws, using real-world values.
  - **People** (section 8): one human species with modern minds, and bodies that eat, heal, age, have children and pass on traits.
  - **Minds** (section 9): people and animals who perceive, form their own concepts, learn cause and effect, build skills, dream, and choose for reasons that can be explained.
  - **Culture and society** (section 10): learning from others, language, belief, institutions, art, music, myths and style, all emerging on their own.
  - **Your powers** (section 4): weather and disasters, dreams, and fortune.
  - **Time and history** (section 5): time that follows zoom, a story director, and rewinding and branching history.
  - **Presentation** (section 11): detailed pixel art, one continuous zoom from the globe to a single person, and many ways to follow the story: the chronicle, following one person's life, map overlays, archaeology and more.
  - **Sound** (section 12): a living soundscape first, then their music, their voices and a score.
  - **The phone app** (section 13): built for one phone, in portrait and landscape, smooth at all times.
  - **Research tools** (section 14): experiments across many worlds, run in the cloud, with reports and replays you review on the phone.

### 3.2 Where history starts

- `SCP-01` **Starting point** *(Decided)*: Modern minds with very little culture.
  - **What:** Every world begins with 3–4 family bands of modern humans who have almost no culture: a few dozen words, no way to make fire, nothing but rough stones and sticks. The full starting kit is in `BIO-02`.
  - **Why:** Because their minds are already modern, progress depends on learning and culture, not on waiting millions of years for brains to evolve. Because they start with almost nothing, the great early discoveries happen in play: making fire, shaping stone, clothing, language.
  - This is a deliberate starting point, not a real moment in history. Real early humans already had more culture than this.

- `SCP-14` **Other starting points later** *(Decided)*
  - **What:** Worlds can later begin from other starting points:
    - **Ice-age hunters:** like humans of roughly 50,000–40,000 years ago, with full language, fire-making and fine stone blades.
    - **Ancestral minds:** smaller brains whose abilities must evolve over many generations.
    - **A blank slate:** modern brains with no language, fire or tools at all.

    Each still has a single human species (`SCP-05`).
  - **Why:** A starting point is just the knowledge and abilities put into people's heads at the beginning, so alternatives cost little and make good experiments.
  - They come after the main starting point works (`PRN-09`).

### 3.3 Who it's for

- `SCP-02` **Just you** *(Decided)*
  - **What:** Project Nature is built for one person, on one phone.
  - **In practice:**
    - no public release, store listing, onboarding or tutorial;
    - no support for other phones, tablets or computers (experiments in the cloud are a research tool, not a way to play);
    - no accounts, purchases, ads or analytics;
    - free to use your phone's specific hardware (`PLT-01`).
  - **Why:** Building for one person and one device removes whole categories of work, so the effort goes into depth and polish.

### 3.4 How it gets built

- `SCP-03` **Experiments lead** *(Decided)*
  - **What:** Core ideas are proven first, in experiments across many random worlds run without graphics (`RES-01`). A phone app grows alongside them, so you can watch the results from the start.
  - **Why:** The biggest risk is that nothing emerges (`RSK-01`). Experiments find out early and cheaply.

- `SCP-15` **Experiments run in the AI's cloud sessions** *(Decided)*
  - **What:** Experiments run in the same cloud sessions where the AI builds the game, within those sessions' computing limits.
  - **Why:** There's nothing extra to set up, maintain or pay for.
  - If an experiment ever needs more computing power than a session offers, that is raised with you before anything else is set up.

- `SCP-16` **Milestones** *(Decided; the list itself is Proposed)*: The project moves through these milestones in order. Each ends with a report you review (`RES-06`). Tasks and dates live in the implementation plan.

  1. `MIL-01` **Foundations:** a small generated valley that plays out identically on the phone and in the cloud (`PRN-08`), the experiment runner and its first report, and a basic phone viewer for replays. *Now possible:* watching a generated valley pass through its days and seasons on your phone.
  2. `MIL-02` **Sharp stone (Experiment 1):** stone that breaks by real rules; people who perceive, form concepts, learn cause and effect, build skills and learn from each other; just enough food and terrain to live on. *Now possible:* watching a band discover how to chip stone, and seeing the skill spread or be lost.
  3. `MIL-03` **Fire and the first power:** heat, burning and friction; keeping and making fire; dreams, your first power; rewinding and branching history. *Now possible:* a band that can only keep fire learns to make it, and you can send a dream and compare what happens with and without it.
  4. `MIL-04` **A living world:** plants and animals in food webs, with weather and seasons; animals with simpler minds; hunting; your powers over nature and fortune; the living soundscape. *Now possible:* hunting becomes an arms race, and your storms and blessings change lives.
  5. `MIL-05` **Words and beliefs:** language emerging, explanations, ritual and myth; the chronicle and life stories written by the describing model. *Now possible:* rites form, dialects drift apart, and the chronicle reads like a history.
  6. `MIL-06` **The whole world:** the full wrap-around world, migrations, many bands and diverging cultures, one continuous zoom from the globe to a single person, and archaeology. *Now possible:* watching peoples spread, split and meet again across a whole world.
  7. `MIL-07` **Open-ended growth:** taming animals, farming, settlements and whatever comes after, each built when an experiment calls for it. *Now possible:* history keeps going, with no ceiling.

### 3.5 Non-goals

Things the project deliberately does not do, and why.

- `SCP-04` **No recipes or tech tree** *(Decided)*: Discoveries come from physics and learning (`PRN-01`, `PRN-07`).
- `SCP-05` **No other human species** *(Decided)*: There is one human species, so the story stays about how one people learns.
- `SCP-06` **No AI language model making decisions** *(Decided)*: Our own knowledge would leak into their world (`PRN-06`).
- `SCP-07` **No goals, scores, wins or losses** *(Decided)*: It is a sandbox; the story is whatever happens.
- `SCP-08` **No worship of the player** *(Decided)*: Your power doesn't depend on their faith, and they never learn you exist (`GOD-06`).
- `SCP-09` **No terraforming** *(Decided)*: You can't reshape land or add or remove species. You act only as nature could (`GOD-05`).
- `SCP-10` **No shared online world or multiplayer** *(Decided)*: It's yours alone (`SCP-02`).
- `SCP-11` **No real-Earth map** *(Decided)*: Every world is generated (section 6).
- `SCP-12` **No simulated planet formation** *(Decided)*: Worlds are generated directly in a realistic present-day state, which keeps generation cheap (`WLD-08`).
- `SCP-17` **No direct control** *(Decided)*: You never control any person or animal, not even briefly (`GOD-01`).
- `SCP-18` **No scripted story** *(Decided)*: There is no campaign, no quests and no authored events. Every story comes from the simulation (`PRN-01`).
- `SCP-19` **No magic in the world** *(Decided)*: Nothing supernatural exists in the world's physics. Spirits and gods exist only in people's beliefs. The only unseen force is you, and you act through nature.
- `SCP-20` **No borrowed real cultures** *(Decided)*: Their peoples, names, languages and customs are their own. Nothing is copied from real cultures, and descriptions never compare them to real peoples.

## 4. The player as god

You are an invisible force of nature. This section defines exactly what you can do, how strong each power is, and the limits that keep every act natural. Two principles govern all of it: you are nature (`PRN-03`), and the rules never bend (`PRN-12`).

### 4.1 Your role

- `GOD-01` **Role** *(Decided)*
  - **What:** A distant, invisible god in a pure sandbox. You can watch everything, everywhere, and you can nudge, but you never command or control anyone (`SCP-17`).
  - **Why:** Every achievement in the world stays theirs.
  - **Example:** You can't tell Ama to twirl sticks. You can only give her a dream and see what she does with it.

- `GOD-06` **Never known** *(Decided)*
  - **What:** People experience your interventions as nature: weather, luck, dreams. They may explain them as spirits or gods, and whatever they believe is their own interpretation, right or wrong. Nothing in the world can ever detect you directly.
  - **Why:** Their beliefs stay their own, and religion grows from the same machinery as discovery (`CUL-05`).
  - **Example:** After a run of lucky hunts that you sent, a band gives the credit to the bones they buried at the cave mouth. A ritual of burying bones begins.

- `GOD-05` **Only natural means** *(Decided)*
  - **What:** Every act must be something nature could do. Your powers feed into the world's own systems (weather, chance, sleep). They never create anything from nothing and never break a rule (`PRN-12`). There is no limited supply of power to spend, but nature's own limits always apply. Those limits *(the list is Proposed)*:
    - lightning comes from storm clouds, so to strike a tree you first need a storm overhead, which you can bring;
    - disasters happen only where conditions allow: eruptions at volcanoes with magma beneath them, earthquakes on faults, floods where rain can swell the rivers, wildfires where fuel is dry enough to burn;
    - weather stays within what the climate can produce at that place and season, so there is no snow in a tropical summer;
    - a person or animal has at most one dream per sleep;
    - fortune works on chance, never on the rules (`GOD-04`).
  - **Why:** A single miracle would make the world's history untrustworthy.
  - **Check:** every intervention passes the same physical checks as a natural event would.

### 4.2 Your powers

- `GOD-02` **Nature and disasters** *(Decided)*
  - **What:** Three scales of influence:
    - **Small events, placed exactly:** a lightning strike, a shower, a gust of wind, a cold night, a fog.
    - **Seasons, pushed over a region:** a wet spring over a valley, a dry year over a region, a hard winter.
    - **Disasters, where conditions allow:** floods, droughts, storms, wildfires, eruptions, earthquakes, landslides.
  - **Not included:** changing the climate directly. If an eruption you trigger is big enough to cool the world for a few years, that is physics at work, not a power.
  - **Why:** Weather is the most natural lever there is, and the one people have always tried to explain.
  - **Example:** You bring a storm over the ridge and send lightning into a dead pine. Fire runs down the slope, and the band upwind gathers burning branches.

- `GOD-03` **Dreams** *(Decided)*
  - **What:** While someone sleeps, you can shape their dream from their own memories and feelings. A dream can:
    - bring two of their memories together, such as the smoking stick and the warmth of fire;
    - relive one memory vividly, so it stays strong and comes to mind more easily;
    - carry a feeling (fear, longing, hope or awe) that shapes what they make of it.

    A dream can only use what the dreamer has actually experienced. They still have to work out the "how" themselves, and they may never act on it at all.
  - **What follows:** The dream becomes a memory of its own. It makes certain ideas more likely to come to mind, and the dreamer may tell others about it, which can feed myth and belief (`CUL-05`).
  - **Why:** Dreams are where minds recombine experience (`MND-12`), so they are the most natural way for a god to touch an idea without supplying it.
  - **Example:** The session story in `VIS-11`.

- `GOD-12` **Animal dreams** *(Decided)*
  - **What:** Animals can be sent simpler dreams: one memory relived, coloured by a feeling.
  - **Why:** Animals learn too (`MND-16`). Dreams let you lean on that slowly, for example toward taming.
  - **Example:** A wolf dreams again of the warmth and the scraps by the fire, and comes a little closer to the camp the next night (`MOM-06`).

- `GOD-04` **Fortune and fate** *(Decided)*
  - **What:** You can bless or curse a person, a family, a band, an animal herd or a place. Fortune can touch luck in the hunt, finding food or materials, fertility, health and recovery, and sickness and plague.
  - **How strong:** Gentle. A blessing at most doubles a chance: a hunt with a 10% chance of success gets 20%. Their skill still matters most, and nothing is ever certain. A curse works the same way in reverse, at most halving a chance *(Proposed)*. A blessing or curse lasts as long as you set, from a single hunt to a few years *(Proposed)*.
  - **Fortune works on chance, never on the rules:** it changes which of the possible outcomes happens, never what is possible. A plague needs a disease that already exists in the world.
  - **Why:** Luck is how the world feels to the people in it. Fortune lets you lean on it without taking over.
  - **Example:** You bless a band's hunters for one winter. They come home with meat a little more often, but whether they survive still depends on how well they hunt and share.

### 4.3 Using your powers

- `GOD-10` **Using your powers on the phone** *(Proposed)*
  - **Touch first:** tap a person, animal, group or place to see what you can do there.
  - **Nature:** draw around an area to push its weather or season; tap a spot for a small event.
  - **Dreams:** open a sleeper's memories, shown as small pixel-art scenes, choose what the dream is made of, and pick a feeling.
  - **Fortune:** choose what to bless or curse, and for how long.
  - Everything then plays out through the simulation. Nothing happens faster than nature could make it happen.

- `GOD-11` **What's possible here** *(Proposed)*
  - **What:** The game only offers what nature could do at that place or to that being right now, and says briefly why other powers aren't available, such as "no volcano here" or "she is awake".
  - **Why:** You never have to guess what's natural, and you never try a miracle by accident.

### 4.4 Records of your interventions

- `GOD-08` **Recorded behind the scenes** *(Decided)*: Every intervention is recorded with its time, place and target. Rewinding, branching and replays depend on this record (`PRN-08`, `TIM-06`).

- `GOD-07` **No trace in the story view** *(Decided)*: The story view never shows where you intervened or how much you helped.

- `GOD-09` **Interventions in the scientist's view** *(Decided)*
  - **What:** The scientist's view shows where and when you intervened, and traces what changed because of it.
  - **Why:** Curiosity (`VIS-08`): you can find out what your nudges actually did. Branching (`TIM-06`) lets you compare history with and without them.
  - **Example:** You select the dream you sent Ama and follow what came of it: eleven days of twirling sticks, the first fire, and fire-making spreading along the river.

## 5. Time and history

After the camera, time is your main control. This section defines how fast time runs, what decides its speed, what happens while you're away, and how you go back in history. Two principles shape all of it: pacing comes only from controlling time (`PRN-12`), and when the phone can't keep up, time slows rather than the simulation cutting corners (`PRN-11`).

### 5.1 How fast time runs

- `TIM-01` **Time follows zoom** *(Decided)*
  - **What:** By default, the speed of time is tied to the zoom. The closer you look, the slower time runs; the further out, the faster. One gesture controls both where you look and how fast history moves.
  - **The scale** *(Proposed; exact values are tuned by measurement, `TIM-07`)*:
    - **one person:** natural speed (`TIM-10`);
    - **a camp:** a day passes in a few minutes;
    - **a valley:** a season passes in about a minute;
    - **a region:** years pass every minute;
    - **the whole world:** centuries pass every minute.
  - **Why:** Close-up moments are lived; distant eras are watched.
  - **Example:** You watch the knapper strike, flake by flake. Then you pull back over the valley, and a whole summer passes while the herds move north.

- `TIM-10` **Natural speed up close** *(Decided)*: At the closest zoom, people and animals move at real-life speed. You can watch a flake come off the stone.

- `TIM-04` **Manual control** *(Decided)*: You can unlink speed from zoom whenever you want. *(Proposed controls: pause, play, a speed dial, and a lock that keeps the current speed while you move the camera.)*

### 5.2 The story director

- `TIM-02` **Story director** *(Decided)*
  - **What:** The director watches the whole world for important moments and adjusts the speed of time around them. When nothing important is happening, it lets quiet years race past, up to the top speed your zoom allows.
  - **When something important happens elsewhere** *(Decided)*: time slows, a live moment appears (`PRE-08`), and one tap takes you there. You stay in control of the camera.
  - **What counts as important** *(Proposed)*:
    - firsts: the first time anyone does something new;
    - births and deaths among the people you follow;
    - discoveries spreading or being lost;
    - conflicts, disasters and migrations;
    - a band forming, splitting or ending;
    - the consequences of your own interventions.
  - **Why:** In a world that runs itself, the best moments are easy to miss (`RSK-03`).

- `TIM-03` **The director never touches events** *(Decided; follows from `PRN-10` and `PRN-12`)*: The director controls speed only. It decides where to slow down but never causes, changes or hides anything.

- `TIM-11` **Skip to the next moment** *(Proposed)*: A control that runs time at top speed until the next important moment, then slows down. Useful for short check-ins (`VIS-10`).

### 5.3 While you're away

- `TIM-05` **Pauses when closed** *(Decided)*: When the app is closed or in the background, the world stops. Nothing happens while you're away, and every session starts exactly where the last one ended.

- `TIM-12` **Overnight mode** *(Decided)*
  - **What:** Leave the app open on the charger and switch on overnight mode. The world runs at top speed with the screen dimmed. When you come back, a summary tells you what happened, drawn from the chronicle (`PRE-05`).
  - **Why:** Deep simulation runs slowly on a phone (`PRN-11`). Overnight mode gives history the hours it needs without you having to watch.
  - **Safeguards** *(Proposed)*: it runs only while the phone is charging, and it stops if the phone gets too hot.
  - **Example:** You start it before bed. In the morning: "312 years passed. Two bands merged by the river; a long drought pushed the eastern band over the hills; on the coast, someone began drying fish."

### 5.4 Going back

- `TIM-06` **Rewind and branch** *(Decided)*
  - **What:** Go back to any moment in a world's history and carry on from there, changing something or nothing. The original timeline is kept, and the new one becomes a branch.
  - **Why:** Curiosity (`VIS-08`): the only way to really answer "what if?".
  - **Example:** You rewind to before the plague, send a mild winter instead, and compare the two histories (`MOM-10`).

- `TIM-13` **Comparing timelines** *(Proposed)*: Two branches side by side: their chronicles, their maps, and key numbers (population, discoveries, languages, beliefs), with the moment they split clearly marked.

- `TIM-08` **Saved worlds and timelines** *(Proposed)*: Several worlds, each with its own tree of timelines, kept on the phone. Branches can be named, and you can switch between them.

- `TIM-14` **Dates** *(Proposed)*: The game counts years from the moment a world's history begins ("year 2,314"), with days and seasons set by that world's own sun and moons (`WLD-06`). The people's own calendars are separate (`CUL-13`).

### 5.5 Pacing and endings

- `TIM-07` **Pacing** *(To test)*: There is no fixed target for how long history takes to watch. It is measured and tuned during development (`PLT-04`).

- `TIM-09` **If everyone dies** *(Decided)*: The world goes on without them. Nature carries on, and you can keep watching, rewind to before the end, or start a new world.

## 6. World

The world is a small planet with everything a planet has: rock, water, air, plants, animals and microbes, all following real rules. This section defines the world's shape and size, how a world is made, how detail is managed, and each natural system. What matter is made of is in section 7; how animals think is in section 9.

### 6.1 Shape and size

- `WLD-01` **Torus with latitude** *(Decided)*
  - **What:** The map wraps around in both directions. Walk east long enough and you come back from the west; walk north across the polar ice and you come back from the south. An equator runs across the middle of the map, and polar ice lies along the line where it wraps north–south. Climate zones and seasons behave as on a planet, with seasons reversed between the northern and southern halves.
  - **Why:** There are no edges and no stretched or squashed regions, so every place can be simulated in the same way.

- `WLD-02` **Globe view** *(Decided)*: Fully zoomed out, the world is drawn as a globe. The wrap only shows at the poles. Someone crossing the polar ice would seem to jump from one pole to the other on the globe, which is rare and harmless.

- `WLD-03` **Size** *(Decided)*
  - **What:** About 1,000 km from pole to pole and about 2,000 km around: roughly 2 million km² in all, land and sea together.
  - **What follows:** Each climate zone is roughly 100 km wide, about four to five days' walk.
  - **Why:** It is big enough for many separate peoples and small enough to simulate deeply (`PRN-02`).

- `WLD-04` **How many people it can feed** *(To test)*: Estimated at roughly 50,000 hunter-gatherers (about one person per 10 km² of good land), or around ten million once farming exists, since farming supports 10 to 100 times more people on the same land. Measured in experiments.

### 6.2 The planet

- `WLD-06` **Varied within reason** *(Decided)*: Each world has its own day length, year length, axial tilt (and so the strength of its seasons), moons, and share of land, all within ranges that allow human-like life. *(Proposed ranges: day 18–36 hours, year 250–500 days, tilt 5°–35°, 0–3 moons, 25–50% land. Gravity, air and chemistry stay Earth-like.)*

- `WLD-07` **A rich sky** *(Decided)*
  - **What:** The sun, moons, stars and planets move realistically for each world's orbit and tilt. Eclipses, comets, meteor showers and auroras happen.
  - **Why:** The sky is the first calendar, the first compass and a great source of myth (`CUL-13`).
  - **Example:** A comet that hangs over the valley for a month, the same month the old chief dies, becomes part of how the band remembers that winter.

### 6.3 Making a world

- `WLD-08` **Realistic, not from scratch** *(Decided)*: Worlds are generated directly in a realistic present-day state, using fast methods that imitate what deep time would have produced. Generating one is cheap.

- `WLD-09` **What generation produces** *(Decided)*, in this order:
  1. tectonic plates, mountain ranges, volcanoes and faults;
  2. rock types and layers, with minerals and ores in geologically plausible places;
  3. erosion: valleys, rivers, lakes, deltas and coastlines;
  4. climate, worked out from the geography (`WLD-16`);
  5. soils, from rock, climate and time;
  6. vegetation and landscapes;
  7. animals and microbes adapted to them (`WLD-19`).

- `WLD-19` **Species from Earth families** *(Decided)*
  - **What:** Earth's families of plants and animals (deer, wolves, wild cattle, salmon, grasses, birches, oaks, berries and so on) are the starting point. Generation adapts them into each world's own species to fit its landscapes. Every species gets its traits: size, diet, behaviour, seasons, and the chemistry of its body, which decides what is edible, poisonous, medicinal or useful (section 7).
  - **Why:** Familiar enough to understand, new enough that each world has its own tree of life to discover.

- `WLD-23` **Richness of life** *(Decided)*: About 50 animal and 200 plant species per world, across all groups: mammals, birds, fish, shellfish and insects; trees, shrubs, grasses, herbs and fungi.

- `WLD-10` **Generate many, keep the best** *(Decided)*
  - **What:** The generator makes many candidate worlds and scores each one. It keeps the best, and never edits it.
  - **What scores well** *(Proposed)*:
    - varied landscapes and climates;
    - natural barriers (mountains, seas, deserts) that let separate cultures form;
    - resources spread unevenly (flint here, copper there);
    - a good place to begin (`WLD-24`).

- `WLD-24` **Where history begins** *(Proposed)*: The bands start in a temperate region with caves, fresh water and varied food within reach. The region is found by the scoring, never placed by hand.

- `WLD-11` **Generation time** *(Proposed)*: A candidate world takes under a minute to generate on the phone, so choosing the best of a dozen takes a few minutes.

### 6.4 Detail

- `WLD-12` **Detail where it matters** *(Decided; follows from `PRN-11`)*: Each system runs at the coarsest scale that keeps it true. Climate is worked out region by region; rivers and soils kilometre by kilometre; plants and animals in patches of a few hundred metres. Everything goes down to the metre where people are or where you look.

- `WLD-13` **Nothing changes when you look away** *(Decided; follows from `PRN-10`)*: Fine detail is generated the same way every time and never contradicts what was simulated more coarsely.

### 6.5 Natural systems

All of these are simulated in depth, and each feeds the others.

- `WLD-14` **Geology and materials** *(Decided)*
  - **What:** Rocks, minerals, soils and ores lie in realistic places, so what can be discovered depends on what's underfoot.
  - **Example:** Flint comes out of chalk and limestone, obsidian near volcanoes, copper ores in certain mountains, clay along rivers, salt in dry basins.

- `WLD-15` **Living geology** *(Decided)*: Change continues during play. Erosion wears the land, rivers shift their course, landslides fall, earthquakes strike along faults, volcanoes erupt, and coastlines move as the sea rises and falls.

- `WLD-27` **Soils** *(Decided)*: Soils form from rock, climate, plants and time. They hold water and nutrients, decide what grows where, and can later be enriched or exhausted by people.

- `WLD-16` **Climate and weather** *(Decided)*
  - **Climate from geography:** Each place's climate (rain, temperature and winds through the seasons) is worked out from real physics: latitude, height, distance from the sea, prevailing winds, and mountains that block rain.
  - **Daily weather** is drawn from that climate, with storm systems that move across the land.
  - **Long cycles:** over thousands of years, ice ages and warm periods move coastlines and push migrations. A great eruption can cool the world for a few years.
  - **Example:** Rain clouds coming off the western sea drop their rain on the mountains, so the valleys beyond are dry grassland with forest only along the rivers.

- `WLD-05` **Climate on a small world** *(Decided)*: Climate zones sit closer together than on Earth, a few days' walk apart, and weather systems are scaled to fit the world.

- `WLD-25` **People change the climate** *(Decided)*: What covers the land and, much later, fuel burned at scale feed back into the climate through the same physics. Clearing a forest can dry a region; centuries of burning could warm the world.

- `WLD-17` **Fresh water** *(Decided)*: Rivers, lakes, wetlands, springs, underground water, ice and floods. Life and settlement gather around them.

- `WLD-26` **Seas** *(Proposed)*: Oceans with currents that carry heat and moisture, tides set by the moons, and a sea level that rises and falls with the ice ages. At low tide, shellfish beds are exposed on the shore.

- `WLD-18` **Ecology** *(Decided)*
  - **What:** Plants grow, flower, fruit and die back with the seasons. Animals eat, breed, migrate and die. Everything is tied together in food webs, with populations that boom and crash.
  - **Why:** It is what people live from, and what they will one day change.
  - **Example:** A run of mild winters lets the deer multiply; the wolves follow; then a hard winter cuts both down, and the hunters go hungry.

- `WLD-28` **Fire in the landscape** *(Proposed)*: Lightning and dry fuel start wildfires, which spread with wind and slope; landscapes regrow after them, and some plants depend on fire. People can learn to use fire on the land.

- `WLD-20` **Heredity in plants and animals** *(Decided)*: Inheritance continues during play, so adaptation and domestication (wolves into dogs, wild grasses into grain) can happen on their own.

- `WLD-21` **Microbes** *(Decided)*: Rot, fermentation and disease are living microbes that spread and evolve. Crowding, and living close to animals, bring epidemics.

- `WLD-22` **Natural disasters** *(Decided; follows from `GOD-05` and the systems above)*: Eruptions, earthquakes, floods, droughts, storms, wildfires and lightning come from the world's own systems, not only from you.

How animals think is covered in `MND-16`.

## 7. Matter and physics

This is where "no recipes" lives. Nothing in the world is a recipe item: everything is matter with real chemistry and structure, changed by a few dozen general laws using real-world numbers. Discovery means people finding out what those laws allow (`PRN-01`, `PRN-07`).

### 7.1 What things are made of

- `MAT-01` **Made of real ingredients** *(Decided)*
  - **What:** All matter is built from real ingredients: real minerals, compounds and the substances of living things. Results come from how these interact, never from rules written for each material.
  - **Examples by group:**
    - **rock and minerals:** silica (as quartz, flint, chert, obsidian or sand), calcite (limestone, chalk), clays, iron oxides (yellow and red ochre), copper minerals, tin ore, salt;
    - **water and air:** water as ice, liquid and vapour; the gases of the air;
    - **living matter:** cellulose and lignin (wood, plant fibres), starches, sugars, proteins, fats, collagen (hide, sinew, bone), bone mineral, resins, tannins, and the plant chemicals that make things poisonous or medicinal.

- `MAT-09` **Elements and energy are kept** *(Decided)*
  - **What:** Every ingredient has its real elemental makeup (carbon, hydrogen, oxygen, nitrogen, silicon, calcium, iron, copper, tin and so on), and every change keeps elements and energy balanced. Nothing ever comes from nothing.
  - **Why:** It makes the world honest, and it keeps the door open to any chemistry people might reach later (`VIS-03`).
  - **Example:** Smelting copper ore yields exactly the copper that was in it, plus gases. Burning wood releases the energy stored in it as heat and light, and leaves ash holding its minerals.

- `MAT-02` **Structure matters** *(Decided)*
  - **What:** Matter also records how it's put together: crystal or glass, fibrous, porous or dense, coarse or fine grain, wet or dry. Grinding, melting, cooling and drying change structure without changing makeup.
  - **Example:** Sand, flint and obsidian are all mostly silica, but only flint and obsidian chip into blades. Sand melted with plant ash and cooled becomes glass.

- `MAT-03` **Properties are derived** *(Decided)*: Every property follows from what something is made of and how it's put together:
  - **mechanical:** weight, hardness, strength, toughness, springiness, and how it breaks (in shell-like flakes, in splinters, or by crumbling);
  - **heat:** how it burns, melts, holds heat and passes it on;
  - **water:** how it soaks up water, dissolves, softens or swells;
  - **the body:** nutrition, poison, medicine, taste and smell;
  - **the senses:** colour, sheen, texture, and the sound it makes when struck (`MND-03`);
  - **time:** how fast it rots, rusts, wears or weathers.

- `MAT-10` **Things** *(Decided)*: Everything in the world is a thing with a makeup, a structure, a shape, a size and a temperature. Things can be split, joined, worn down, heated, mixed and carried.

### 7.2 How things change

- `MAT-04` **A few dozen general laws** *(Decided)*: Change comes from general laws, each using real temperatures and conditions. No law ever names a product. A starting list *(Proposed)*:
  - **force:** breaking, cutting, scraping and grinding, bending and springing back, pressing and pounding, friction, twisting and binding, joining by tying, gluing or fitting;
  - **heat:** heating and cooling, burning with more or less air, charring, melting and setting, drying, roasting;
  - **water:** wetting and soaking, dissolving and leaching, swelling, freezing;
  - **chemistry:** rusting and smelting, burning lime, setting of mortar, tanning, glass-making;
  - **life:** growing, digesting, healing, rotting and fermenting, with microbes at work (`WLD-21`).

- `MAT-07` **One law, many inventions** *(Decided; follows from `PRN-07`)*: Laws are general enough that one law covers many inventions. For example, "metal ores give up their metal when heated hot enough in contact with burning charcoal" covers copper, tin, lead and iron. Each needs its own real conditions, so they become possible in a natural order that nobody wrote down.

- `MAT-06` **Actions are physical** *(Decided)*: Every action has force, angle, speed, duration and temperature, and the physics decides the outcome. Technique matters: a clumsy strike shatters the stone.

- `MAT-11` **Mechanics** *(Decided; follows from `MAT-06`)*: Weight, momentum, leverage, springiness and friction follow real physics. Throwing sticks, spear-throwers and bows can work only because the physics makes them work.

- `MAT-12` **What a body can do** *(Proposed)*: The basic actions everything else is built from: grasp, carry, put down, drop, throw, strike, press, rub, twist, bend, tear, dig, cut or scrape with an edge, pierce with a point, pour, blow, chew, and put into fire or water. Anything more, such as knapping, sewing or smelting, is a sequence of these that someone has to learn.

- `MAT-08` **Traces last** *(Decided)*: Hearths, tools, bones, graves and rubbish heaps stay in the world and get buried over time, feeding the archaeology view (`PRE-09`). *(Proposed detail: what survives depends on the material and the ground. Stone lasts; bone survives in dry caves and limestone; wood, hide and plant fibre usually rot, except in waterlogged, frozen or very dry ground.)*

### 7.3 Real numbers

- `MAT-05` **Real-world values** *(Decided)*
  - **What:** Temperatures, hardness, energy content, toxicity and every other number come from real measurements, each with its source (`PRN-05`).
  - **Example:** Copper melts at about 1,085 °C. An open wood fire reaches roughly 600–900 °C; a charcoal furnace with forced air passes 1,100 °C. So copper waits until someone builds a hotter fire.

### 7.4 How matter grows

Matter must be easy to extend, forever (`PRN-14`).

- `MAT-13` **Four catalogues** *(Proposed)*: Matter is described in four catalogues: ingredients, structures, laws and reality checks. Each entry stands alone, is written in plain language a person can read and check, gives its real-world values and sources, and names the reality checks that prove it.

- `MAT-14` **Adding without rewriting** *(Proposed)*
  - **What:** Adding an ingredient, structure, law or check never requires changing the others.
  - **Why it works:** Laws never name products (`PRN-07`), so a new ingredient automatically takes part in every existing law.
  - **Example:** Adding tin ore needs no new smelting rule; the smelting law already covers it.

- `MAT-15` **Every addition proves itself** *(Proposed)*: Each new entry comes with the reality checks it must pass, and the whole checklist runs again, so nothing that worked before breaks.

- `MAT-16` **Matter grows in layers** *(Proposed)*: Each milestone adds a layer without rewriting earlier ones:
  1. stone, wood, bone and water (Experiment 1);
  2. heat and fire;
  3. food and the body's chemistry;
  4. fibres, hides and joining;
  5. clay, lime and pigments;
  6. metals and glass;
  7. further layers as experiments call for them.

### 7.5 Reality checklist

The physics must reproduce every item below without any rule written specially for it. The checklist grows with each layer (`MAT-16`), and every item is run again whenever anything changes (`MAT-15`).

**Core**

- `RCK-01` **Flint chips, granite doesn't** *(Decided)*: Flint and obsidian chip into sharp flakes; granite doesn't.
- `RCK-02` **Fire by friction** *(Decided)*: Rubbing wood fast enough can light dry tinder.
- `RCK-03` **Cooking helps** *(Decided)*: Cooking makes food more nourishing.
- `RCK-04` **Pottery needs fire** *(Decided)*: Fired clay becomes pottery; sun-dried clay softens again in water.
- `RCK-05` **Lime** *(Decided)*: Burned limestone becomes lime.
- `RCK-06` **Leather** *(Decided)*: Hides soaked with oak bark become leather instead of rotting.
- `RCK-07` **Fermentation** *(Decided)*: Fruit sugars ferment.
- `RCK-08` **Copper needs a furnace** *(Decided)*: Copper smelts in a charcoal furnace with forced air, but not over a campfire.
- `RCK-09` **Rot** *(Decided)*: Untreated meat and hides rot, faster when warm and wet.

**Early crafts and food**

- `RCK-10` **Heat-treated flint** *(Decided)*: Flint gently heated in a fire chips more easily and more predictably.
- `RCK-11` **Cord** *(Decided)*: Plant fibres twisted together make cord far stronger than the single fibres.
- `RCK-12` **Glue from bark** *(Decided)*: Birch bark heated without air gives a tar that glues a stone point to a shaft.
- `RCK-13` **Leaching** *(Decided)*: Soaking in running water draws the bitterness out of acorns.
- `RCK-14` **Keeping meat** *(Decided)*: Salting, smoking and drying make meat keep far longer.

**Colour and art**

- `RCK-15` **Ochre turns red** *(Decided)*: Yellow ochre turns red when heated.
- `RCK-16` **Paint that lasts** *(Decided)*: Charcoal and ochre mixed with fat or water make paint that lasts on rock.

**Later crafts**

- `RCK-17` **Bronze** *(Decided)*: Copper with a little tin is harder than copper.
- `RCK-18` **Iron** *(Decided)*: Iron needs a hotter, longer charcoal fire than copper and comes out spongy; it must be hammered to make it useful.
- `RCK-19` **Mortar** *(Decided)*: Lime mortar hardens in the air.
- `RCK-20` **Glass** *(Decided)*: Sand with plant ash melts into glass in a very hot fire.

## 8. People: bodies and lives

- `BIO-01` **One species, modern minds** *(Decided)*: Their brains are as capable as ours. Their culture starts almost empty.
- `BIO-02` **Starting kit** *(Decided)*:
  - **Language:** a few dozen shared words and calls; grammar must grow.
  - **Fire:** they can feed a fire found after lightning or a wildfire, but cannot make one.
  - **Tools:** unshaped stones for bashing, and sticks.
  - **Clothing:** none.
  - **Shelter:** natural caves and overhangs.
  - **Food:** gathering, scavenging, some ambush hunting.
  - **Beliefs:** none are set in advance.
- `BIO-03` **Starting population** *(Decided)*: 3–4 family bands of 15–30 people each.
- `BIO-04` **Life cycle** *(Proposed)*: Birth, childhood, adulthood, ageing and death; pairing and having children.
- `BIO-05` **Health** *(Proposed)*: Nutrition comes from what they actually eat. Injuries heal or don't. Illness comes from microbes (`WLD-21`).
- `BIO-06` **Heredity** *(Decided)*: Body traits and mental traits (curiosity, memory, learning speed, temperament) pass from parents to children. They shift over generations at real-world speeds as some people survive and have children and others don't. Minds barely change over thousands of years; culture does the heavy lifting, as in our own history.
- `BIO-07` **Evolution dial** *(Decided)*: A setting speeds up genetic change for experiments.

## 9. Minds

How people think, and in simpler form, how animals think. Everything here is learned inside the world.

- `MND-01` **No AI language model thinks for them** *(Decided)*: Every belief and invention comes from the mechanisms below.
- `MND-02` **Knowledge only from inside the world** *(Proposed)*: Any learning a mind does draws only on experience in its own world. Nothing carries real-world knowledge in.
- `MND-03` **Senses, not labels** *(Decided)*: People perceive properties (weight, hardness, colour, smell, taste, warmth, sound), never the game's names for things.
- `MND-04` **Their own concepts** *(Decided)*: People sort what they perceive into their own categories. Categories differ between groups and can be wrong. One band lumps flint and chert together as "cutting stone"; another confuses a poisonous berry with a safe one.
- `MND-05` **Cause-and-effect beliefs** *(Proposed)*: "Doing this to that, in this situation, leads to this." Each belief is held with more or less certainty as evidence comes in. Discovery and superstition come from the same mechanism.
- `MND-06` **Skills** *(Proposed)*: Learned sequences of actions with fine control (angle, force, timing) that improve with practice. Knowing something can be done is not the same as doing it well.
- `MND-07` **Drives** *(Proposed)*: Hunger, thirst, cold, tiredness, fear, belonging, status, curiosity and the urge to have children.
- `MND-08` **Feelings shape memory** *(Proposed)*: Strong feelings decide what is remembered and how strongly. A terrifying storm stays for life; an ordinary day fades.
- `MND-09` **Choosing what to do** *(Proposed)*: Habits handle routine. Deliberate planning takes over when habits fail or the stakes rise: working backwards from a need through what they believe causes what. People explore most when comfortable (play) and when desperate (need).
- `MND-10` **Curiosity** *(Proposed)*: Attention goes where expectations fail. Surprises are remembered and tried again.
- `MND-11` **Where new ideas come from** *(Proposed)*:
  - accidents someone notices;
  - watching nature, such as fire after lightning, or seeds sprouting from a rubbish heap;
  - tinkering with skills they already have;
  - analogy: what works on wood might work on bone;
  - dreams.

  Every idea is a guess until the physics says yes or no.
- `MND-12` **Dreams** *(Proposed)*: During sleep, people replay and recombine their own memories. This is also the player's lever (`GOD-03`).
- `MND-13` **Learning over a lifetime** *(Decided)*: People get better at things through their own experience.
- `MND-14` **Detail follows attention** *(Proposed)*: Minds far from your attention run in simpler form (habits, and knowledge held by the group as a whole). They sharpen again when you zoom in, with no break in their story.
- `MND-15` **No population cap** *(Decided)*: How many minds the phone can run at each level of detail is found by measurement (`PLT-04`).
- `MND-16` **Animals** *(Decided)*: Animals have the same kind of mind with fewer abilities. They learn fear, routes and habits, so hunting becomes an arms race and taming becomes possible.

## 10. Culture and society

**Knowledge**

- `CUL-01` **Learning from others** *(Decided)*: Imitation (imperfect copying creates variation), teaching, and copying whoever succeeds or whatever most people do.
- `CUL-02` **Knowledge can be lost** *(Proposed)*: Knowledge lives in heads and dies with them unless passed on. Small, isolated groups can lose skills.
- `CUL-03` **Memory outside heads** *(Decided)*: Marks, symbols, writing and records can emerge, letting knowledge outlive the people who had it.
- `CUL-04` **Language** *(Proposed)*: Words are labels a group agrees on, and they spread through use. Groups that separate drift into dialects, then separate languages. Language makes teaching faster and lets people talk about things that aren't there: plans, the dead, spirits.

**Belief**

- `CUL-05` **Explaining the world** *(Proposed)*: Big unexplained events (death, sickness, storms, the player's interventions) demand a cause. When no physical cause is known, people suspect an unseen being. Such beliefs spread, become ritual, and eventually have specialists such as shamans and priests.

**Society**

- `CUL-06` **Institutions form from habit** *(Decided)*: Repeated behaviour hardens into shared, named things that people know, teach and enforce: a norm, a role, a rank, a rite. They can change, split and dissolve.
- `CUL-07` **Nothing social is scripted** *(Proposed)*: Kinship and marriage rules, sharing and exchange, trade, leadership, alliances, conflict and war all come from people's interactions.
- `CUL-08` **Dark history can happen** *(Decided)*: War, slavery, sacrifice and cruelty can emerge like anything else. What is shown is controlled by the content setting (`PRE-18`).

**Expression** (each exists as a real thing in the world)

- `CUL-09` **Visual art** *(Decided)*: Paintings, carvings and body decoration composed from their own memories and myths, on cave walls and objects.
- `CUL-10` **Music and dance** *(Decided)*: Rhythms, scales, songs and instruments that grow out of each culture.
- `CUL-11` **Myths and stories** *(Decided)*: Told and retold, changing as they spread.
- `CUL-12` **Style and ornament** *(Decided)*: Each culture's look in tools, clothing and buildings, drifting over time, so objects could be dated by their style.
- `CUL-13` **Their sky and calendar** *(Decided)*: Constellations they name, seasons they track, festivals they keep.
- `CUL-14` **Their maps and names** *(Decided)*: Places named in their own languages, and maps drawn the way they see the land.
- `CUL-15` **Remembered lives** *(Decided)*: Genealogies, and legends of remarkable people as their culture remembers them.

## 11. Presentation

### 11.1 Visual style

This is how the world looks. It is written to stand on its own, without needing any image to understand it.

- `PRE-01` **Detailed pixel art** *(Decided)*: Everything on screen is crisp pixel art: limited colours, hard pixel edges, no blur and no smooth gradients.

- `PRE-02` **Pixel-rendered 3D** *(Decided)*
  - **What:** The world is a real 3D world, drawn at low resolution and enlarged with hard pixel edges. It looks like hand-made pixel art but has real depth, scale and structure. The camera turns freely and zooms continuously.
  - **Why:** Real 3D shows height, depth, sizes and structures (cliffs, caves, shelters, later buildings) at every zoom. The land comes straight from the simulation instead of being hand-drawn, which suits generated worlds.
  - **Example:** At dusk, from an oblique angle, you see a band's camp below a limestone cliff: the cave mouth in shadow, long shadows across the grass, the river beyond. You turn the camera and fly down until one person fills the screen.

- `PRE-20` **Colour in steps** *(Decided)*
  - **What:** Every material has a short, hand-picked ladder of shades, about 4–7 colours, drawn from one master palette. Light chooses a step on the ladder. Where two steps meet, a fine pixel pattern blends them in a narrow band only; surfaces are never speckled all over. The pattern is fixed to the surface, so it doesn't swim when the camera moves.
  - **Why:** Clean colour is what separates pixel art from a shrunken photograph.

- `PRE-21` **Outlines and lit edges** *(Decided)*
  - **What:** A one-pixel dark outline wherever one thing stands in front of another: people, animals, trees, rocks, the top edge of a cliff. A one-pixel bright edge where the sun or a fire catches a shape, such as the sunlit rim of a cliff or the fire-facing side of a person.
  - **Why:** Crisp silhouettes keep small things readable on a phone screen.

- `PRE-22` **Stable pixels** *(Decided)*
  - **What:** Pixels never crawl or shimmer as the camera moves: the picture stays locked to its pixel grid, and turns ease to rest. One art pixel is always the same size on screen, in portrait and in landscape, so turning the phone only changes the framing. *(Proposed: about 4 screen pixels per art pixel.)*
  - **Why:** Shimmering pixels are the most common flaw of 3D pixel art, and the first thing that makes it look cheap.

- `PRE-23` **Rock faces** *(Decided)*
  - **What:** Cliffs show their geology: horizontal rock layers of different thicknesses, irregular vertical cracks, a few long fissures, lichen, water stains, soot above inhabited caves, grass hanging over the top edge, and scree at the foot. The same layers continue underground (`PRE-25`).
  - **Why:** Geology is part of the story (`WLD-14`). What people can find depends on what the land is made of, and the rock should show it.

- `PRE-24` **Real shapes** *(Decided)*
  - **What:** Overhangs, caves, rock shelters and, later, buildings have real depth.
  - **Example:** Looking into a cave mouth from an angle, you see its dark interior, the firelit floor and the hide windbreak across the entrance.

- `PRE-25` **Cut-away view** *(Decided)*
  - **What:** The ground can be sliced open to show what lies beneath: rock layers, soils, underground water, and the buried layers of past life (hearths, tools, bones, graves).
  - **Why:** It is how you see geology and dig through history. The archaeology view (`PRE-09`) uses it.

- `PRE-26` **Water** *(Decided)*
  - **What:** Rivers meander and change width, with gravel bars, reeds, lines that follow the current, ripples at fords, glints of sun and drifting mist. From far away a river never becomes thinner than one or two art pixels, so it stays readable.

- `PRE-27` **People and animals** *(Decided)*
  - **What:** People and animals are small 3D figures drawn through the same pixel look and animated at a deliberate, sprite-like rhythm of about 8–12 poses a second. They look like crisp pixel art from any angle and turn properly with the camera. At the closest zoom, a person is about 40–60 art pixels tall *(Proposed)*: enough for a face, hair, clothing and gestures.
  - **Why:** The simulation will produce actions nobody planned (`PRN-01`). Figures built from parts can perform any of them from any angle, without a new drawing for each.

- `PRE-28` **Readable from far away** *(Decided)*: As you zoom out, people become tiny outlined figures in strong clothing colours, then groups become small markers, then a camp becomes a glowing point.

- `PRE-29` **From above** *(Decided)*
  - **What:** As the camera rises, it tilts toward looking straight down, and the land shifts into a clean map look: crisp colours for forest, grassland, rock and water, rivers as lines, shaded hills. Map overlays (`PRE-07`) sit on this view. At the very top, the whole world appears as a globe (`WLD-02`). Close up to globe is one continuous zoom (`PRE-03`).
  - **Why:** A landscape seen from high up at an angle turns to mush. A map stays clear at every height.

- `PRE-30` **Light, time and season** *(Decided)*
  - **What:** One master palette, with versions for each time of day (dawn, day, dusk, night) and each season. The sun casts real shadows, the sky tints everything, and distance adds haze. A fire lights its surroundings with a warm, flickering glow that fades with distance, warms the faces of people nearby, and sends up smoke and embers.

- `PRE-03` **Seamless zoom** *(Decided)*: One continuous zoom from the whole world, drawn as a globe, down to one person chipping flint.

- `PRE-04` **Sharp at every zoom** *(Decided)*: The pixel art stays sharp and readable at every zoom level (`PRE-22`, `PRE-28`, `PRE-29`).

- `PRE-31` **Visual review** *(Proposed)*
  - **Done when:** at every milestone, screenshots at each zoom level, in both orientations and at every time of day, pass a review for:
    - clean colour, with no speckled surfaces;
    - crisp silhouettes;
    - pixels that stay still while the camera moves;
    - people and animals readable at phone size.

**Following the story**

- `PRE-05` **Chronicle** *(Decided)*: An automatically written history: timelines, a book of ages, eras named by their own people.
- `PRE-06` **Follow a soul** *(Decided)*: Pick anyone and follow their life: thoughts, dreams, relationships, death.
- `PRE-07` **Map overlays** *(Decided)*: Beliefs, knowledge, moods, languages and family ties shown spread across the land.
- `PRE-08` **Live moments** *(Decided)*: Key events surface as they happen ("someone has made fire for the first time").
- `PRE-09` **Archaeology** *(Decided)*: Dig down through buried layers of past life: hearths, graves, lost tools, rubbish heaps.
- `PRE-10` **Family trees and legends** *(Decided)*: Genealogies across generations, and the legends their culture keeps.
- `PRE-11` **Their sky and calendar** *(Decided)*: The sky as they understand it.
- `PRE-12` **Their maps and names** *(Decided)*: Their place names and maps, with translation.
- `PRE-13` **Every view the simulation allows** *(Decided)*: Any further view the simulation's data supports, within physical limits.
- `PRE-14` **Two views of every mind** *(Decided)*: A story view in their own words, and a scientist's view of their raw beliefs, how certain they are, and the evidence behind each belief.
- `PRE-15` **Art that remembers** *(Proposed)*: Tap a painting or carving to see the event or myth it depicts.
- `PRE-16` **Bestiary** *(Proposed)*: Each world's tree of life and its species.

**Text written for you**

- `PRE-17` **Descriptions stick to the data** *(Decided)*: The AI language model only turns simulation data into text: life stories, myths, dreams, the chronicle. It never adds facts the simulation doesn't contain.
- `PRE-18` **Content setting** *(Decided)*: You choose how much of history's darker side is shown. The simulation underneath never changes.
- `PRE-19` **Storytelling voices** *(To test)*: Documentary, archaeologist, their own tradition, and intimate. Each is tried live on real simulation output and chosen by ear. Different views may use different voices.

## 12. Sound

Added in layers, starting with the living soundscape.

- `SND-01` **Living soundscape** *(Decided; first layer)*: Wind, rain, rivers, animals and fire, driven by what's actually happening where you're looking.
- `SND-02` **Their music** *(Decided; later layer)*: Songs, rhythms and instruments from each culture.
- `SND-03` **Their voices** *(Decided; later layer)*: Their own languages spoken aloud, with sounds generated for each language and translated for you.
- `SND-04` **Score** *(Decided; later layer)*: Original background music that reacts to the state of the world.

## 13. Platform and performance

- `PLT-01` **One phone** *(Decided)*: Built for your Pixel 11 Pro XL, and free to use that phone's specific hardware wherever it helps.
- `PLT-02` **Portrait and landscape** *(Decided)*: Both are supported, and the layout adapts.
- `PLT-03` **Works offline** *(Proposed)*: Including the text descriptions, using the phone's own built-in AI where possible.
- `PLT-04` **Measured limits** *(To test)*: Measured from the first build and reported at every milestone:
  - smoothness of zooming and panning;
  - simulated time per real minute at each zoom level;
  - how many people the phone can run at each level of detail;
  - battery use and heat per hour of play;
  - time to generate a world.
- `PLT-05` **Experiments in the cloud** *(Proposed)*: The simulation also runs without graphics on cloud computers, many worlds at a time. Results can be replayed on the phone.

## 14. Research and validation

- `RES-01` **Experiments lead** *(Decided)*: Core ideas are proven in experiments across many random worlds, run without graphics, before the game builds on them.
- `RES-02` **Experiment 1: sharp stone** *(Decided)*: Do bands that only bash rocks discover how to chip sharp flakes, and does the skill spread?
- `RES-03` **Experiment 1 pass criteria** *(Proposed; numbers to be calibrated)*:
  - **Discovery:** happens in at least half of 100 random worlds, within 500 simulated years.
  - **Variety:** discovery times differ widely between worlds, and at least two different routes to the discovery appear (for example, an accident someone notices versus deliberate tinkering).
  - **Spread:** once discovered, at least three quarters of the adults in the discovering band can do it within 50 simulated years.
  - **Loss:** the skill is lost noticeably more often in small, isolated groups than in large, connected ones.
  - **General rules only:** the check in `PRN-07` passes.
- `RES-04` **Reality checklist** *(Proposed)*: The physics must pass every `RCK` item before any discovery that depends on it is trusted.
- `RES-05` **Reproducibility** *(Decided)*: Re-running a seed with the same interventions gives the same history on the phone and in the cloud (`PRN-08`). Checked from the first build.
- `RES-06` **Milestone reports** *(Decided)*: Every milestone ends with a report for you: what was tested, charts, what emerged, and replays to watch on the phone.
- `RES-07` **Candidate later experiments** *(Open)*:
  - making fire;
  - skills lost in small, isolated groups;
  - superstition and ritual;
  - shared words and dialects;
  - taming animals.

## 15. Project and process

- `PRC-01` **Passion project, built by AI** *(Decided)*: You direct; AI agents write, test and review the code.
- `PRC-02` **Your role** *(Decided)*: You review milestones and experiment reports and set direction. The AI handles code review and testing.
- `PRC-03` **Technology** *(Decided)*: Chosen by the AI and proposed in the implementation plan for your approval.
- `PRC-04` **Source of truth** *(Decided)*: This file says what to build, and the implementation plan says how. Code and tests link back here by ID.
- `PRC-05` **Section deep dives** *(Decided)*: Each section is expanded with more detail, one at a time. *Proposed* items are confirmed or changed along the way.

## 16. Risks

Reviewed at every milestone.

- `RSK-01` **Nothing emerges**: The minds and physics might not produce discoveries often enough. *Response:* Experiment 1 tests this early and cheaply.
- `RSK-02` **The phone can't keep up**: Deep minds, chemistry and detailed pixel art add up. *Response:* measure from the first week (`PLT-04`), and lower detail where no one is looking (`WLD-12`, `MND-14`).
- `RSK-03` **Real but dull to watch**: Simulated worlds often hide their best stories. *Response:* the story director, live moments and the two views of each mind bring them out, and every report judges whether they do.
- `RSK-04` **Phone and cloud disagree**: Different processors can calculate slightly differently, which would break rewind and experiments. *Response:* reproducibility checks from the first build (`RES-05`).
- `RSK-05` **The scope never ends**: "No ceiling" plus "everything deep" never finishes. *Response:* `PRN-09`.
- `RSK-06` **The chemistry gives absurd results**: *Response:* the reality checklist (`RCK-01` onwards).
- `RSK-07` **Our own knowledge leaks in**: through the describing model or design shortcuts. *Response:* `PRN-06`, `PRN-07`, `MND-02`.

## 17. Not yet decided

Items marked *Open* or *To test*, for the deep dives. Every *Proposed* item also needs your confirmation.

- **Vision:** `VIS-16` (name)
- **Time and history:** `TIM-07`
- **World:** `WLD-04`
- **Minds:** `MND-15` (limits)
- **Presentation:** `PRE-19`
- **Platform and performance:** `PLT-04`
- **Research and validation:** `RES-07`

## 18. Glossary

- **Band:** a small group of people, usually family, who live and move together.
- **World:** one generated planet.
- **Seed:** the number a world is generated from. Same seed, same world.
- **Timeline / branch:** one history of a world. Rewinding and changing something starts a new branch.
- **Intervention:** anything the player does with their powers.
- **Run:** one simulation of a world for an experiment.
- **Concept:** a category a person forms from what they perceive, such as "cutting stone".
- **Belief:** a person's idea of what causes what, held with more or less certainty.
- **Skill:** a learned way of doing something, which improves with practice.
- **Institution:** a shared, named pattern of behaviour (a norm, role, rank or rite) that people know, teach and enforce.
- **Level of detail:** how finely something is being simulated at a given moment.
- **Describing model:** the AI language model that turns simulation data into readable text. It never decides anything.
- **Story view / scientist's view:** the two ways to look into a mind (`PRE-14`).
- **Story director:** sets the speed of time according to what is happening. It never causes events.
- **Overnight mode:** the world running at top speed, screen dimmed, while the phone charges (`TIM-12`).
- **Reality checklist:** real-world changes the physics must reproduce without special rules (`RCK`).
- **Signature moment:** a story the simulation must be able to produce without it being scripted (`MOM`).
- **Milestone:** a stage of the project that ends with a report you review (`MIL`).
- **Live moment:** a notable event the game surfaces to you as it happens (`PRE-08`).
- **General-rules check:** confirms that no rule is written for one particular discovery, material, species or event, and that no discovery's name appears in decision-making logic (`PRN-07`).
