# Kindling

The project file: what Kindling is, and every feature and target it must reach.
It is written so that someone with no prior context can read it and understand the whole project.

It contains no implementation details.
Those belong in the two other documents, the architecture and the implementation plan, which link back to this file by ID, as will the code.
Every item has a permanent ID so that no feature gets lost on the way from idea to code.

## Contents

<!-- generated: contents -->
- [How this file works](#how-this-file-works)
- [1.
  Vision](#1-vision)
- [2.
  Principles](#2-principles)
- [3.
  Scope and non-goals](#3-scope-and-non-goals)
- [4.
  The player as god](#4-the-player-as-god)
- [5.
  Time and history](#5-time-and-history)
- [6.
  World](#6-world)
- [7.
  Matter and physics](#7-matter-and-physics)
- [8.
  People: bodies and lives](#8-people-bodies-and-lives)
- [9.
  Minds](#9-minds)
- [10.
  Culture and society](#10-culture-and-society)
- [11.
  Presentation](#11-presentation)
- [12.
  Sound](#12-sound)
- [13.
  Platform and performance](#13-platform-and-performance)
- [14.
  Research and validation](#14-research-and-validation)
- [15.
  Project and process](#15-project-and-process)
- [16.
  Risks](#16-risks)
- [17.
  Not yet decided](#17-not-yet-decided)
- [18.
  Glossary](#18-glossary)
<!-- end generated -->

---

## How this file works

This section is *Decided* as a whole, and changes only with your OK (`PRC-07`).

### Status

| Status | Meaning |
|---|---|
| *Decided* | Agreed with you. |
| *Proposed* | Suggested but not yet confirmed. You confirm, change or drop it (`PRC-07`); the Not yet decided section lists them all. |
| *To test* | Settled by experiment or measurement, not by opinion. When the result is in, it is written into the item, which becomes *Decided* at a milestone review with your OK. |
| *Open* | Not decided yet. |
| *Dropped* | No longer planned. Kept for the record, with a "Dropped because:" line. |

Every item, including the signature moments, milestones and risks, starts with exactly one of these words in its marker, and nothing else.

### Kinds of item

- **Feature:** something to build.
  It needs a task in the implementation plan.
- **Rule:** something that must always hold.
  It needs a check.
- **Context:** explains or records, and needs nothing built.

Each area has a default kind, and the exceptions are listed here, in one place:

- **Context:** `VIS`, `MIL` and `RSK`, plus `SCP-01`, `SCP-13`, `SCP-16`, `GOD-01`, `MND-17`, `RES-07`, `PRC-01` and `PRC-08`.
- **Rules:** `PRN`, `MOM` (checked by experiment), `SCP`, `RCK`, `RES` and `PRC`, plus `GOD-05`, `GOD-06`, `GOD-07`, `TIM-03`, `WLD-13`, `WLD-30`, `MAT-09`, `MAT-13`, `MAT-14`, `MAT-15`, `MAT-17`, `BIO-14`, `BIO-17`, `MND-01`, `MND-02`, `CUL-07`, `PRE-17` and `PRE-31`.
- **Features:** every other area, plus `SCP-14`, `RES-02`, `RES-03`, `RES-06`, `RES-12`, `RES-15`, `RES-20`, `PRC-06` and `PRC-12`.

### Item format

Every item starts with its ID, a short name and its status.
Detailed items then add some of the following:

- **What:** what it is, in plain words.
- **Why:** the reason it exists.
- **Example:** a concrete illustration.
- **Done when:** checks that prove it has been delivered.
- **Check:** for rules that always apply, how we verify they are being followed.
- **Follows from:** the items it follows from.

*What*, *Done when* and *Check* are binding; *Why* and *Example* only explain; any other label counts as part of *What*.
If a summary and its source disagree, the source wins.
"About" means within 10% unless stated.
The detailed acceptance criteria for each item are written in the implementation plan, and you approve them when each milestone starts.
Tests link to items.

### Changing this file

- AI agents suggest additions or changes as *Proposed* items, listed in the Not yet decided section (`PRC-07`).
- To change a decided item, a **Proposed change:** line goes beneath it, with the new text and the reason.
  Your OK replaces the text.
- Items are edited in place, and keep their IDs.
- Every commit that changes this file ends with a line naming the changed IDs and why, for example `Changed: GOD-04 (blessing cap raised; owner OK)`.
  A check enforces it, and work linked to a changed item is flagged for re-checking.
- If two parts of an item can be delivered in different milestones, they get separate IDs, split when the plan needs it.
- In the source, each sentence starts on its own line, so changes show clearly; the page reads the same.

### IDs and links

1. Every item has a permanent ID: an area code plus a number, such as `WLD-01`.
   Numbers go to three digits after 99, such as `RCK-100`.
2. IDs are never renumbered or reused.
   A new item takes the next free number in its area, wherever it sits in the text.
   A new ID becomes permanent only when it reaches the main version; until then, a branch whose number is taken renumbers its own new items.
3. Items are never deleted.
   If something is cut, its status becomes *Dropped*, with a "Dropped because:" line, and no live item points to it.
4. The implementation plan maps every feature and rule to a milestone, and every task names the IDs it delivers.
   The coverage check enforces this (`PRC-12`).
5. Code and tests name the IDs they implement, so any feature can be followed from this file to the plan to the code, and back.
6. A new area gets a new three-letter code in the table below, and its own section or subsection.

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

This section says what Kindling is, what it feels like, and what success means.
Every other section serves it.

### 1.1 The game in brief

- `VIS-01` **In one sentence** *(Decided)*: A bottom-up simulation of humanity on a generated Earth-like world, where a few bands of early humans living in caves learn, entirely by themselves, to survive, build, believe and organise.

- `VIS-06` **In one paragraph** *(Decided)*: Kindling simulates a whole world from the ground up: rock, water, weather, plants, animals and people.
  It begins with a few bands of early humans sheltering in caves.
  They have modern brains but almost no culture: a few dozen words, no way to make fire, nothing but rough stones and sticks.
  Nothing tells them what to do.
  There are no recipes, no tech tree and no list of eras to unlock.
  They learn the way real people learned: by noticing, trying, failing, copying, teaching and dreaming.
  Everything they ever achieve, from a sharp flake of stone to rituals, languages, farms and perhaps cities, has to come from what they discover in the world and pass on to each other.
  You watch it all on your phone as an invisible force of nature.
  You can nudge the weather, luck and dreams, but you can never command anyone.

- `VIS-02` **The fantasy** *(Decided)*: You are nature.
  - **What:** You are the weather, the luck and the dreams.
    You can send a storm, bless a hunt, or let someone dream two of their own memories side by side.
    You can't speak, appear or work miracles, and the people of the world never learn you exist.
  - **Why:** A god who can't command anyone leaves every achievement theirs.
    Whatever gods they come to believe in are their own explanations of the world, and sometimes of you.
  - **Example:** A lightning strike you send to start a wildfire becomes, generations later, the myth of the storm spirit who first gave them fire.

### 1.2 What it feels like

- `VIS-07` **Wonder** *(Decided)*
  - **What:** Awe at a world that runs itself and keeps surprising you, its maker included.
  - **Why:** Nothing is authored.
    The rules are known; what they produce is not.
  - **Example:** You zoom out from one campfire to the whole world and watch a thousand years of migrations, languages and beliefs move across the land like weather.

- `VIS-08` **Curiosity** *(Decided)*
  - **What:** The urge to understand why something happened, and to try "what if".
  - **Why:** Every event has real causes, and the game lets you find them: the scientist's view of a mind (`PRE-14`), the buried layers of a site (`PRE-09`), and rewinding and branching history (`TIM-06`).
  - **Example:** A band abandons its cave.
    You look into their minds and find a run of failed hunts and a belief that the cave turned against them after a death.
    You rewind, send a good hunting season, and see whether they stay.

- `VIS-09` **Other feelings** *(Decided)*: Attachment to particular people, and the harshness of nature, will arise from the simulation and are welcome, but the design isn't built around them.
  When design choices conflict, wonder and curiosity decide.

### 1.3 How you play

- `VIS-10` **Two rhythms of play** *(Decided)*
  - **Short check-ins (5–15 minutes):** open the app, catch up on the latest live moments, follow someone for a while, nudge, close.
  - **Long sessions (an hour or more):** watch an era unfold at speed, read the chronicle, dig through the past, branch a "what if" and compare the outcomes.
  - **Why it matters:** both must feel natural.
    A check-in can't require any setup, and a long session needs tools for depth.
  - The world pauses when the app is closed (`TIM-05`), so every session starts exactly where the last one ended.

- `VIS-11` **A session, as a story** *(Decided)*: An illustration, not a script.

  > You open the app.
  > The world is exactly where you left it: late autumn in the valley of two rivers, year 2,314.
  > A live moment is waiting: *the eastern band has lost its fire*.
  > You zoom in, and time slows to walking pace.
  > The camp is cold; children huddle under hides; wolves circle at the edge of the scree.
  >
  > You could send a dry spell to the forest on the ridge and hope lightning finds it.
  > Instead you look through the memories of Ama, the band's most curious woman.
  > Last summer, boring a hole in a piece of wood, she saw the stick begin to smoke.
  > You give her a dream that sets that smoking stick beside the warmth of a fire.
  > The next morning she is twirling sticks.
  > It takes her eleven days.
  >
  > You zoom out, and decades pass in seconds.
  > On the knowledge overlay, fire-making spreads from band to band along the river.
  > In the chronicle, the story is already being retold as myth: *Ama stole the fire that sleeps inside the wood*.
  > You close the app, and the world waits for you.

### 1.4 Signature moments

- `VIS-12` **Signature moments** *(Decided)*: Stories the simulation must be able to produce.
  None of them is scripted.
  Each is an example of what the rules should make possible, and each becomes a long-term test (the `MOM` items below).
  The IDs in brackets are the parts of the project each moment depends on.

  - `MOM-01` **Fire from wood** *(Decided)*: In a hard winter, a band whose fire has died learns to make fire by friction.
    (`MND-11`, `RCK-02`, `GOD-03`)
  - `MOM-02` **The lost craft** *(Decided)*: A fever kills a band's best stoneworkers.
    For generations its blades are cruder, until the skill is rediscovered or learned again from neighbours.
    (`CUL-01`, `CUL-02`)
  - `MOM-03` **Your lightning becomes a god** *(Decided)*: A lightning strike you sent kills a hunter on a hilltop.
    The band avoids the hill, then leaves offerings there, then tells stories about the one who lives in the storm.
    (`GOD-02`, `GOD-06`, `CUL-05`)
  - `MOM-04` **The song that does nothing** *(Decided)*: A band sings before a hunt that goes well.
    The song becomes a hunting rite and is kept for centuries, though it changes nothing.
    (`MND-05`, `CUL-06`)
  - `MOM-05` **Two tongues** *(Decided)*: Two bands are separated by a rising sea and drift apart in speech.
    When their descendants meet again, they can hardly understand each other.
    (`CUL-04`, `WLD-16`)
  - `MOM-06` **The camp wolf** *(Decided)*: The boldest wolves scavenge at the edge of camp.
    Their pups grow tamer each generation, until a child raises one.
    (`MND-16`, `WLD-20`)
  - `MOM-07` **A painting that remembers** *(Decided)*: A painting of a great hunt outlasts everyone who saw it.
    You tap it and see the hunt.
    (`CUL-09`, `PRE-15`)
  - `MOM-08` **Seeds on the rubbish heap** *(Decided)*: Seeds thrown on a rubbish heap sprout near camp.
    Years later, someone starts planting on purpose.
    (`MND-11`, `WLD-18`)
  - `MOM-09` **The dig** *(Decided)*: Under a village, you find the hearths of the first band and the bones of the animals they ate.
    (`MAT-08`, `PRE-09`)
  - `MOM-10` **Two endings** *(Decided)*: You rewind to before a plague, send a mild winter instead, and compare two histories of the same people.
    (`TIM-06`)
  - `MOM-11` **Rivals, then in-laws** *(Decided)*: Two bands fight over a valley, then marry into each other.
    Each side's descendants tell the story differently.
    (`CUL-07`, `CUL-11`)
  - `MOM-12` **Metal from green stone** *(Decided)*: A kiln built very hot for pottery leaves a bead of shiny metal where green stones lined the fire, and someone notices.
    (`MAT-07`, `RCK-08`)

### 1.5 The arc of a world

- `VIS-03` **No ceiling** *(Decided)*
  - **What:** There are no eras, levels or end state.
    A world's history goes as far as its people take it.
  - **Why:** Any fixed sequence of eras would be a tech tree in disguise.
  - **In practice:** some worlds may stall for tens of thousands of years, and some bands will die out.
    Some peoples may reach farming, writing, metals and beyond; some may take paths our own history never took.
    Nothing about the order of our history is guaranteed, except where physics forces it: no one smelts copper without a fire hot enough.
    Collapse, stagnation and extinction are all valid histories.

### 1.6 What makes it different

- `VIS-13` **Seven differences** *(Decided)*: A summary of decisions made in other sections.
  - **No recipes.** Discoveries come from physics, not from lists (`PRN-01`; see Matter and physics).
  - **Minds that learn.** People form their own concepts, beliefs and skills.
    Science and superstition come from the same mechanism (see Minds).
  - **Real matter.** Real chemistry and real-world numbers decide what is possible (see Matter and physics).
  - **You are nature.** An invisible god, limited to what nature could do (see The player as god).
  - **Every story can be traced.** Two views of every mind, archaeology, and rewinding and branching history (see Time and history and Presentation).
  - **Rigour behind the wonder.** Experiments that can fail decide what the simulation really does (see Research and validation).
  - **In your pocket.** Designed for one phone, with detailed pixel art and one continuous zoom from the whole world to a single person (see Presentation and Platform and performance).

### 1.7 Inspirations

- `VIS-04` **Inspirations** *(Decided)*: What we take from each, and where we differ.
  - **[world-sim](https://world.world-sim.uk):** a living world whose villagers discover fire, pottery and bronze for themselves, with named souls, graves and a book of ages.
    *We take* its care for individual lives and a history worth reading.
    *We differ:* we start much earlier, simulate a far deeper physical world, have no tech tree or list of eras, and run on a phone.
  - **Dwarf Fortress:** deep simulation, and generated legends you can read.
    *We take* history as the main product.
    *We avoid* an interface that hides its stories.
  - **RimWorld:** stories that emerge from the simulation, paced by an AI storyteller.
    *We take* its care for pacing.
    *We differ:* our story director only controls the speed of time; it never creates events (`TIM-03`).
  - **WorldBox:** a pixel-art god sandbox made for phones.
    *We take* the joy of a living world in your hand.
    *We differ:* a far deeper simulation, and powers limited to what nature could do.
  - **Black & White:** a god whose acts shape what villagers believe.
    *We differ:* there is no worship and no visible god.
  - **Ancestors: The Humankind Odyssey:** early humans learning by experimenting.
    *We take* the thrill of discovery by trial.
    *We differ:* nobody is controlled, and discoveries come from physics, not from an unlockable skill tree.
  - **Noita and falling-sand games:** matter that follows rules, so interactions nobody designed still work.
    *We take* rules over recipes.
    *We differ:* matter is described by its chemistry and structure, not simulated grain by grain.
  - **Science:** research on cultural evolution, cognition, the origins of religion and the emergence of language.
    Each source is cited in the section that uses it.

### 1.8 Success

Who it's for: you alone (`SCP-02`).
Success is judged by the experience; the research rigour of `VIS-05` is how we get there.

- `VIS-14` **A joy on the phone** *(Decided)*
  - **What:** Beautiful, smooth and absorbing in your hand.
  - **Done when** (exact limits set from the measurements in `PLT-04`):
    - zooming and panning stay smooth at the screen's full refresh rate, at every zoom level;
    - the app opens to your world, ready to play, within about three seconds;
    - an hour's session uses about 25–30% of the battery, with longer sessions on the charger, and the phone never gets uncomfortably hot;
    - every screen works in both orientations (`PRE-34`).

- `VIS-15` **Histories worth reading** *(Decided)*
  - **What:** Every world produces a history you would want to read, and no two are alike.
  - **Done when** (judged by you at milestone reviews):
    - in milestone reviews, you'd choose to read a world's chronicle for pleasure;
    - worlds from different seeds tell clearly different stories;
    - every chronicle entry can be traced back to the simulated events behind it.

- `VIS-05` **Quality bar** *(Decided)*: The rigour of a research project and the craft of a well-funded studio.
  - **Research rigour:** what the simulation is claimed to do is tested by experiments that can fail, across many runs, with real-world values and repeatable results.
  - **Studio craft:** art, sound, interface and performance polished to the standard of a well-funded studio.
  - Rigour is the method, not the goal.
    It exists so that the wonder is earned and the histories are real.

### 1.9 Name

- `VIS-16` **Name** *(Decided)*: **Kindling**, what a fire grows from: small things that catch and spread, like knowledge.
  "Project Nature" was the working title.

## 2. Principles

The rules every part of the project follows.

- `PRN-16` **Principles come first** *(Decided)*: These rules apply to every part of the project, and outrank everything else in this file: if any decision conflicts with a principle, the principle wins.
  A principle changes only if you change it here.
  Every milestone review goes through the principles, using the **Check** line under each one.

### 2.1 The world

- `PRN-01` **The world is the only teacher** *(Decided)*
  - **What:** Everything the people of the world know, they learned inside it: from their senses, their own trials, other people or their dreams.
    Nothing is handed to them.
    There are no recipes, no tech tree, no scripted discoveries, and no knowledge given at the start beyond the starting kit (`BIO-02`).
  - **Why:** This is the heart of the project.
    A discovery only means something if it was really made.
  - **Example:** Nobody tells a band that flint makes good blades.
    Someone strikes one stone against another, notices a sharp edge, and over time the band learns which stones break that way.
  - **Check:** every discovery in an experiment can be traced back to the experiences that produced it, and the general-rules check (`PRN-07`) passes.

- `PRN-02` **Depth over breadth** *(Decided)*
  - **What:** A small world simulated deeply beats a large shallow one.
    When choosing between more things and deeper things, choose deeper.
  - **Why:** Discovery, belief and history all come from detail.
    A shallow world can't surprise anyone.
  - **Example:** The world is about 1,000 km from pole to pole (`WLD-03`), not the size of Earth, so the effort goes into what's actually there.
  - **Check:** any feature that adds breadth at the cost of depth needs an explicit reason in the implementation plan.

- `PRN-07` **General rules only** *(Decided)*
  - **What:** Everything in the world (matter, living things, minds, societies) follows general rules.
    No rule is ever written for one particular discovery, material, species or event.
    The name of a discovery (flake, knapping, fire-making, pottery and so on) never appears in the logic that decides what people or animals do.
    Those words appear only in descriptions of matter and in text written for you.
  - **Why:** A rule written for one outcome is a recipe in disguise.
    General rules are also what let the world produce things nobody planned.
  - **Example:** There is no "make pottery" rule.
    Clay changes when heated past a certain temperature, just as the general law of heat says any material can.
    Pottery is what people make of that.
  - **Check:**
    - an automated search of the decision-making logic finds no discovery vocabulary;
    - decision logic reads only what people and animals perceive, never what a thing is;
    - swapping two materials' identities changes no behaviour;
    - every law applies to at least two materials;
    - discovery still happens with decoy materials, and with a made-up material nobody designed for;
    - reviews flag any rule that applies to only one material, species or event.

- `PRN-05` **Real numbers, testable claims** *(Decided)*
  - **What:** The values that decide what is possible come from real measurements, each with its source: such as melting and ignition points, how stones break, the energy in food, and the thresholds of the reality checks (`RCK`).
    Everything else is estimated, by stated rules from those values or as plausible ranges, and labelled as an estimate: for example, wood strength from its density, or an animal's needs from its body size.
    Values that are chosen or tuned instead are labelled so, and listed in every milestone report.
    Every claim about what the simulation produces is tested by experiments that can fail, across many runs (`RES-13`).
  - **Why:** Real numbers make discoveries meaningful: copper really does need a furnace.
    Sourcing only the values that decide outcomes keeps this affordable.
    Experiments that can fail stop us fooling ourselves.
  - **Example:** "Bands discover how to chip stone" is accepted as true only after Experiment 1 passes its criteria in its sandbox runs and is confirmed in full worlds (`RES-03`, `RES-21`).
  - **Check:** every key value names its source; every estimate names its rule or range; chosen or tuned values are listed in the milestone report; every claim in a milestone report is backed by an experiment.

- `PRN-12` **Speed up time, never bend the rules** *(Decided)*
  - **What:** Pacing only ever comes from controlling time: zoom, the story director, and manual speed (see Time and history).
    The world's rules never change during play to make things faster or more dramatic.
    Dials that bend the rules, such as faster evolution (`BIO-07`), exist only for experiments.
  - **Why:** If the rules bent for drama, nothing the world produced could be trusted, and its histories would stop being real.
  - **Example:** Real genetic change is slow, so you won't see minds evolve in a single evening.
    To watch that, you run an experiment with the evolution dial turned up, clearly labelled as such.
  - **Check:** play has no rule-bending settings, and every experiment report lists any dial that was changed.

### 2.2 The player

- `PRN-03` **You are nature** *(Decided)*
  - **What:** The player acts only through natural means (`GOD-05`) and is never known to exist (`GOD-06`).
  - **Why:** A god who could command people or appear to them would make every achievement partly yours.
    Every belief about gods would be true, instead of theirs.
  - **Example:** You can't hand a band fire.
    You can make lightning strike a dry tree near their camp.
  - **Check:** every power produces only events the world could produce on its own.

### 2.3 What you see

- `PRN-04` **If the simulation knows it, you can see it** *(Decided)*
  - **What:** Anything the simulation keeps track of can be shown to you: maps of beliefs, family trees, buried layers, a person's memories.
  - **Why:** Curiosity (`VIS-08`) needs ways to find out why.
    A rich world you can't look into is wasted.
  - **Example:** The simulation tracks who taught whom to chip stone, so you can see that chain as a family tree of knowledge.
  - **Check:** everything the simulation keeps track of has at least one view that shows it, if only in the scientist's view (`PRE-14`).

- `PRN-10` **Nothing is faked** *(Decided)*
  - **What:** Everything you see, hear or read reflects what actually happened in the simulation.
    When you zoom in, detail can be filled in, but it never contradicts what was simulated and never invents events for show.
  - **Why:** Histories are only worth reading (`VIS-15`) if they are true to the world.
    Curiosity only works if every clue is real.
  - **Example:** Zooming into a camp that was being simulated in less detail, the game can show people walking between shelters.
    It cannot show a fight that never happened.
  - **Check:** every live moment, chronicle entry and on-screen event can be traced back to a simulated event.

- `PRN-13` **Every choice can be explained** *(Decided)*
  - **What:** Why anyone, person or animal, does something can be traced to their beliefs, drives and memories, and shown in the scientist's view (`PRE-14`): for anything happening now, and for the choices behind every event the history keeps (`PRN-15`).
    Fine physical detail, such as the exact force of a strike, can simply be numbers.
  - **Why:** Curiosity and research both depend on asking "why?" and getting a real answer.
  - **Example:** Why did Ama walk to the river at dawn?
    She was thirsty, and she believes the river is safe at dawn because she has never seen wolves there at that hour.
  - **Check:** for any action under way, and for the choices behind every saved event, the scientist's view shows the beliefs, drives and memories behind them.

- `PRN-06` **AI language models describe, never decide** *(Decided)*
  - **What:** AI language models are used only to turn simulation data into readable text: the chronicle, life stories, myths, dreams.
    They never choose, invent or know anything for the people or animals of the world, and never add facts the simulation doesn't contain (`PRE-17`).
  - **Why:** A language model knows our history.
    If it did their thinking, our knowledge would leak into their world and their discoveries would no longer be theirs.
  - **Example:** The model can tell you, in the voice of their tradition, how Ama "stole the fire that sleeps inside the wood".
    It cannot decide that she tries twirling sticks.
  - **Check:** nothing a language model writes ever feeds back into the simulation, and its descriptions are checked against the data they came from.

### 2.4 How it runs

- `PRN-15` **History is saved, not re-run** *(Decided)*
  - **What:** The past is kept as saved data, chosen for what the game uses: the chronicle and the events behind it, key moments in full, the full state of the world at saved moments, and the records each view of the past needs.
    The past is never recomputed, and the phone and cloud builds don't have to produce identical histories (`PLT-05`).
    A seed decides how a world is generated, not how its history unfolds.
  - **Why:** Re-running history exactly would need identical maths on every device, and every old version of the rules kept forever.
    Saving what matters avoids those costs, so the effort goes into depth on the phone.
  - **Example:** You tap a cave painting of a great hunt.
    The hunt was a key moment, so it was saved in full, and you watch it again as it happened.
  - **Check:** every view of the past reads saved data, and nothing re-simulates the past.

- `PRN-08` **Same seed, same history** *(Dropped)*
  - **Dropped because:** history is now saved rather than re-run (`PRN-15`), and the phone and cloud builds no longer need to match exactly (`PLT-05`).

- `PRN-11` **Time slows, depth stays** *(Decided)*
  - **What:** The screen never stutters.
    When the phone can't keep up, the simulation doesn't cut corners; history simply moves more slowly.
    The only simplification allowed is the planned one: less detail for what is routine, decided by the world's own rule and never by where you look (`WLD-12`, `MND-14`).
    A simpler form is used only once an experiment shows it gives the same history, statistically, as full detail, and moving between forms never contradicts what happened (`PRN-10`).
  - **Why:** Depth is the point of the project (`PRN-02`), and a smooth screen is part of the joy on the phone (`VIS-14`).
    Slowing time protects both.
  - **Example:** A fight breaks out between two bands while you watch.
    The phone works harder, so a day takes longer to pass, but everyone in the fight is still fully simulated and the screen stays smooth.
  - **Check:** measurements show no stutter under heavy load (`PLT-04`), and whatever runs in full stays in full whatever the load.

### 2.5 How it's built

- `PRN-09` **Only as deep as the next experiment needs** *(Decided)*
  - **What:** Each system is built to the depth the next experiment requires, on foundations that can go deeper later without starting over.
    This sets the order of work, not the ambition: in the end, every system is as deep as `PRN-02` asks.
  - **Why:** "No ceiling" plus "everything deep" could never be finished all at once.
    Building in the order the experiments need keeps the project moving and every step testable.
  - **Example:** Experiment 1 (sharp stone) needs to know how stone breaks, not how metal is smelted.
    Smelting waits until an experiment needs it, but matter is designed from the start so it can be added without rework.
  - **Check:** every task in the implementation plan names the experiment or feature that needs it.

- `PRN-14` **Modular by design** *(Decided)*
  - **What:** Every system grows by adding self-contained pieces (materials, laws, species, behaviours, views, checks), and never rewrites what already works without a stated reason.
    Adding something should be easy and effortless.
  - **Why:** A project with no ceiling grows forever, and only a modular one stays buildable.
  - **Example:** Adding tin ore to the world needs one new ingredient entry and its checks.
    Smelting tin already works, because the smelting law never named copper.
  - **Check:** every milestone report lists what was added and confirms that nothing earlier had to be rewritten, or explains why it had to be.

## 3. Scope and non-goals

This section sets the boundaries of the project: what it includes, where history starts, who it's for, how it gets built, and what it deliberately leaves out.

### 3.1 What the project includes

- `SCP-13` **The whole project at a glance** *(Decided)*: A summary of the sections that follow.
  - **A generated world** (see World): a small planet that wraps around, with real geology, climate, weather, water, soils, plants and animals.
  - **Real matter** (see Matter and physics): everything is made of real ingredients and changed by general laws, using real-world values where they decide what is possible.
  - **People** (see People: bodies and lives): one human species with modern minds, and bodies that eat, heal, age, have children and pass on traits.
  - **Minds** (see Minds): people and animals who perceive, form their own concepts, learn cause and effect, build skills, dream, and choose for reasons that can be explained.
  - **Culture and society** (see Culture and society): learning from others, language, belief, institutions, art, music, myths and style, all emerging on their own.
  - **Your powers** (see The player as god): weather and disasters, dreams, and fortune.
  - **Time and history** (see Time and history): time that follows zoom, a story director, and rewinding and branching history.
  - **Presentation** (see Presentation): detailed pixel art, one continuous zoom from the globe to a single person, and many ways to follow the story: the chronicle, following one person's life, map overlays, archaeology and more.
  - **Sound** (see Sound): a living soundscape first, then their music, their voices and a score.
  - **The phone app** (see Platform and performance): built for one phone, in portrait and landscape, smooth at all times.
  - **Research tools** (see Research and validation): experiments in small sandboxes, confirmed in full worlds, run in the cloud, with reports and saved moments you review on the phone.

### 3.2 Where history starts

- `SCP-01` **Starting point** *(Decided)*: Modern minds with very little culture.
  - **What:** Every world begins with 3–4 family bands of modern humans who have almost no culture: a few dozen words, no way to make fire, nothing but rough stones and sticks.
    The full starting kit is in `BIO-02`.
  - **Why:** Because their minds are already modern, progress depends on learning and culture, not on waiting millions of years for brains to evolve.
    Because they start with almost nothing, the great early discoveries happen in play: making fire, shaping stone, clothing, language.
  - This is a deliberate starting point, not a real moment in history.
    Real early humans already had more culture than this.

- `SCP-14` **Other starting points later** *(Decided)*
  - **What:** Worlds can later begin from other starting points:
    - **Ice-age hunters:** like humans of roughly 50,000–40,000 years ago, with full language, fire-making and fine stone blades.
    - **Ancestral minds:** smaller brains whose abilities must evolve over many generations.
      For experiments only, since minds barely change within a history you could watch (`BIO-06`).
    - **A blank slate:** modern brains with no language, fire or tools at all.

    Each still has a single human species (`SCP-05`).
  - **Why:** A starting point is just the knowledge and abilities put into people's heads at the beginning, so alternatives cost little and make good experiments.
  - They come after the main starting point works (`PRN-09`).

### 3.3 Who it's for

- `SCP-02` **Just you** *(Decided)*
  - **What:** Kindling is built for one person, on one phone.
  - **In practice:**
    - no public release, store listing or tutorial, only short help cards (`PRE-40`);
    - no support for other phones, tablets or computers (experiments in the cloud are a research tool, not a way to play);
    - no accounts, purchases, ads or analytics;
    - free to use your phone's specific hardware (`PLT-01`).
  - **Why:** Building for one person and one device removes whole categories of work, so the effort goes into depth and polish.

### 3.4 How it gets built

- `SCP-03` **Experiments first** *(Decided)*: Core ideas are proven in experiments, mostly in small sandboxes, before the game builds on them (`RES-01`), and a phone app grows alongside, so you can watch the results from the start.

- `SCP-15` **Experiments run in the AI's cloud sessions** *(Decided)*
  - **What:** Experiments run in the same cloud sessions where the AI builds the game, within those sessions' computing limits.
  - **Why:** There's nothing extra to set up, maintain or pay for.
  - If an experiment ever needs more computing power than a session offers, that is raised with you before anything else is set up.

- `SCP-16` **Milestones** *(Decided)*: The project moves through these milestones in order.
  Each ends with a report you review (`RES-06`).
  This file keeps each milestone's goal and order; the implementation plan maps every item to a milestone, with tasks and dates.

  1. `MIL-01` **Foundations** *(Decided)*: a small generated valley that runs on the phone and in the cloud with the same statistics (`RES-05`), the experiment runner and its first report, and a basic phone viewer for saved history.
     *Now possible:* watching a generated valley pass through its days and seasons on your phone.
  2. `MIL-02` **Sharp stone (Experiment 1)** *(Decided)*: stone that breaks by real rules; people who perceive, form concepts, learn cause and effect, build skills and learn from each other; just enough food and terrain to live on.
     *Now possible:* watching a band discover how to chip stone, and seeing the skill spread or be lost.
  3. `MIL-03` **Fire and the first power** *(Decided)*: heat, burning and friction; keeping and making fire; dreams, your first power; rewinding and branching history.
     *Now possible:* a band that can only keep fire learns to make it, and you can send a dream and compare what happens with and without it.
  4. `MIL-04` **A living world** *(Decided)*: plants and animals in food webs, with weather and seasons; animals with simpler minds; hunting; your powers over nature and fortune; the living soundscape.
     *Now possible:* hunting becomes an arms race, and your storms and blessings change lives.
  5. `MIL-05` **Words and beliefs** *(Decided)*: language emerging, explanations, ritual and myth; the chronicle and life stories written by the writer AI.
     *Now possible:* rites form, dialects drift apart, and the chronicle reads like a history.
  6. `MIL-06` **The whole world** *(Decided)*: the full wrap-around world, migrations, many bands and diverging cultures, one continuous zoom from the globe to a single person, and archaeology.
     *Now possible:* watching peoples spread, split and meet again across a whole world.
  7. `MIL-07` **Open-ended growth** *(Decided)*: taming animals, farming, settlements and whatever comes after, each built when an experiment calls for it.
     *Now possible:* history keeps going, with no ceiling.

### 3.5 Non-goals

Things the project deliberately does not do, and why.

- `SCP-04` **No recipes or tech tree** *(Decided)*: Discoveries come from physics and learning (`PRN-01`, `PRN-07`).
- `SCP-05` **No other human species** *(Decided)*: There is one human species, so the story stays about how one people learns.
- `SCP-06` **No AI language model making decisions** *(Decided)*: Our own knowledge would leak into their world (`PRN-06`).
- `SCP-07` **No goals, scores, wins or losses** *(Decided)*: It is a sandbox; the story is whatever happens.
- `SCP-08` **No worship of the player** *(Decided)*: Your power doesn't depend on their faith, and they never learn you exist (`GOD-06`).
- `SCP-09` **No terraforming** *(Decided)*: You can't reshape land or add or remove species.
  You act only as nature could (`GOD-05`).
- `SCP-10` **No shared online world or multiplayer** *(Decided)*: It's yours alone (`SCP-02`).
- `SCP-11` **No real-Earth map** *(Decided)*: Every world is generated (see World).
- `SCP-12` **No simulated planet formation** *(Decided)*: Worlds are generated directly in a realistic present-day state, which keeps generation cheap (`WLD-08`).
- `SCP-17` **No direct control** *(Decided)*: You never control any person or animal, not even briefly (`GOD-01`).
- `SCP-18` **No scripted story** *(Decided)*: There is no campaign, no quests and no authored events.
  Every story comes from the simulation (`PRN-01`).
- `SCP-19` **No magic in the world** *(Decided)*: Nothing supernatural exists in the world's physics.
  Spirits and gods exist only in people's beliefs.
  The only unseen force is you, and you act through nature.
- `SCP-20` **No borrowed real cultures** *(Decided)*: Their peoples, names, languages and customs are their own.
  Nothing is copied from real cultures, and descriptions never compare them to real peoples.

## 4. The player as god

You are an invisible force of nature.
This section defines exactly what you can do, how strong each power is, and the limits that keep every act natural.
Two principles govern all of it: you are nature (`PRN-03`), and the rules never bend (`PRN-12`).

### 4.1 Your role

- `GOD-01` **Role** *(Decided)*
  - **What:** A distant, invisible god in a pure sandbox.
    You can watch everything, everywhere, and you can nudge, but you never command or control anyone (`SCP-17`).
  - **Why:** Every achievement in the world stays theirs.
  - **Example:** You can't tell Ama to twirl sticks.
    You can only give her a dream and see what she does with it.

- `GOD-06` **Never known** *(Decided)*
  - **What:** People experience your interventions as nature: weather, luck, dreams.
    They may explain them as spirits or gods, and whatever they believe is their own interpretation, right or wrong.
    Nothing in the world can ever detect you directly.
  - **Why:** Their beliefs stay their own, and religion grows from the same machinery as discovery (`CUL-05`).
  - **Example:** After a run of lucky hunts that you sent, a band gives the credit to the bones they buried at the cave mouth.
    A ritual of burying bones begins.

- `GOD-05` **Only natural means** *(Decided)*
  - **What:** Every act must be something nature could do.
    Your powers feed into the world's own systems (weather, chance, sleep).
    They never create anything from nothing and never break a rule (`PRN-12`).
    There is no limited supply of power to spend, but nature's own limits always apply.
    Those limits:
    - lightning comes from storm clouds, so to strike a tree you first need a storm overhead.
      You can bring one, and it builds over hours, as weather does;
    - disasters happen only where conditions allow: eruptions at volcanoes with magma beneath them, earthquakes on faults, floods where rain can swell the rivers, wildfires where fuel is dry enough to burn;
    - weather nudges shift the weather's own chances within what the climate can produce at that place and season, so there is no snow in a tropical summer, and a run of nudges can't push a place beyond its climate's worst natural stretch;
    - a season can be pushed over a region at most about one climate zone across, roughly 100 km;
    - earthquakes and eruptions use up the stored strain and magma that make them possible: you choose where and when, and nature's stores decide how big;
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
  - **Not included:** changing the climate directly.
    If an eruption you trigger is big enough to cool the world for a few years, that is physics at work, not a power.
  - **Why:** Weather is the most natural lever there is, and the one people have always tried to explain.
  - **Example:** You bring a storm over the ridge and send lightning into a dead pine.
    Fire runs down the slope, and the band upwind gathers burning branches.

- `GOD-03` **Sending dreams** *(Decided)*
  - **What:** While someone sleeps, you can shape their dream from their own memories and feelings.
    A dream can:
    - bring two of their memories together, such as the smoking stick and the warmth of fire;
    - relive one memory vividly, so it stays strong and comes to mind more easily;
    - carry a feeling (fear, longing, hope or awe) that shapes what they make of it.

    A dream can only use what the dreamer has actually experienced.
    They still have to work out the "how" themselves, and they may never act on it at all.
  - **How strong:** a dream you send replaces that night's own dream, and is never stronger than the strongest natural dream: you only choose what it contains.
    Sending the same dream again follows the mind's normal rules for recurring dreams, and nothing marks your dreams out from natural ones (`GOD-06`).
  - **What follows:** The dream becomes a memory of its own.
    It makes certain ideas more likely to come to mind, and the dreamer may tell others about it, which can feed myth and belief (`CUL-05`).
  - **Why:** Dreams are where minds recombine experience (`MND-12`), so they are the most natural way for a god to touch an idea without supplying it.
  - **Example:** The session story in `VIS-11`.

- `GOD-12` **Animal dreams** *(Decided)*
  - **What:** Animals can be sent simpler dreams: one memory relived, coloured by a feeling.
  - **Why:** Animals learn too (`MND-16`).
    Dreams let you lean on that slowly, for example toward taming.
  - **Example:** A wolf dreams again of the warmth and the scraps by the fire, and comes a little closer to the camp the next night (`MOM-06`).

- `GOD-04` **Fortune and fate** *(Decided)*
  - **What:** You can bless or curse a person, a family, a band, an animal herd or a place.
    Fortune can touch luck in the hunt, finding food or materials, fertility, health and recovery, and sickness and plague.
  - **How it works:** Fortune acts only on the chance events around its target, such as whether a deer looks up, which way a spear wobbles, or whether a wound turns bad.
    A blessed failure gets one more try; a cursed success is retried at most half the time.
    Each chance event gets at most one retry, however many blessings and curses overlap.
  - **How strong:** Gentle.
    A blessing can never more than double a chance: a hunt with a 10% chance of success gets at most 20%.
    A curse can never more than halve one.
    Their skill still matters most, nothing is ever certain, and no one's choices are touched.
    There is one strength; you choose the target and how long it lasts, from a single hunt to a few years.
  - **Fortune works on chance, never on the rules:** it changes which of the possible outcomes happens, never what is possible.
    A plague needs a disease that already exists in the world.
  - **Why:** Luck is how the world feels to the people in it.
    Fortune lets you lean on it without taking over.
  - **Example:** You bless a band's hunters for one winter.
    They come home with meat a little more often, but whether they survive still depends on how well they hunt and share.

### 4.3 Using your powers

- `GOD-10` **Using your powers on the phone** *(Decided)*
  - **Touch first:** long-press a person, animal, group or place to see what you can do there (`PRE-33`).
  - **Nature:** choose "draw an area" and draw around it to push its weather or season; tap a spot for a small event.
  - **Dreams:** open a sleeper's memories, shown as small pixel-art scenes of what they remember (which can differ from what happened), choose what the dream is made of, and pick a feeling.
  - **Fortune:** choose what to bless or curse, and for how long.
  - Everything then plays out through the simulation.
    Nothing happens faster than nature could make it happen.

- `GOD-11` **What's possible here** *(Decided)*
  - **What:** The game only offers what nature could do at that place or to that being right now, and says briefly why other powers aren't available, such as "no volcano here" or "she is awake".
  - **Why:** You never have to guess what's natural, and you never try a miracle by accident.

### 4.4 Records of your interventions

- `GOD-08` **Recorded behind the scenes** *(Decided)*: Every intervention is recorded with its time, place, target and every detail (the memories chosen, the feeling, the region drawn, the duration), as part of the saved history (`PRN-15`).
  The scientist's view of your interventions depends on this record (`GOD-09`).

- `GOD-07` **No trace in the story view** *(Decided)*: The story view never shows where you intervened or how much you helped.

- `GOD-09` **Interventions in the scientist's view** *(Decided)*
  - **What:** The scientist's view shows where and when you intervened, and traces what changed because of it.
    By default it follows the chain of causes from your act through the saved history; on request, it runs a comparison branch without the act (`TIM-06`, `TIM-13`).
  - **Why:** Curiosity (`VIS-08`): you can find out what your nudges actually did.
    Branching (`TIM-06`) lets you compare history with and without them.
  - **Example:** You select the dream you sent Ama and follow what came of it: eleven days of twirling sticks, the first fire, and fire-making spreading along the river.

## 5. Time and history

After the camera, time is your main control.
This section defines how fast time runs, what decides its speed, what happens while you're away, and how you go back in history.
Two principles shape all of it: pacing comes only from controlling time (`PRN-12`), and when the phone can't keep up, time slows rather than the simulation cutting corners (`PRN-11`).

### 5.1 How fast time runs

- `TIM-01` **Time follows zoom** *(Decided)*
  - **What:** By default, the speed of time follows the zoom: the closer you look, the slower time runs; the further out, the faster, up to whatever the phone can manage at the detail the world needs (`PRN-11`).
    One gesture controls both where you look and how fast history moves.
  - **What zoom asks for** (how fast history can actually run depends on how much of the world needs full detail at that moment, and is measured, `TIM-07`):
    - **one person:** natural speed (`TIM-10`);
    - **a camp:** a day passes in a few minutes;
    - **a valley:** a season passes in about a minute;
    - **a region:** years pass every minute;
    - **the whole world:** centuries pass every minute.
  - **The past at any speed:** history that has already happened, for example overnight, can be played back at any speed from the saved history (`PRN-15`), so a thousand years can still sweep past like weather (`VIS-07`).
  - **Why:** Close-up moments are lived; distant eras are watched.
  - **Example:** You watch the knapper strike, flake by flake.
    Then you pull back over the valley, and a whole summer passes while the herds move north.

- `TIM-10` **Natural speed up close** *(Decided)*: At the closest zoom, people and animals move at real-life speed.
  You can watch a flake come off the stone.

- `TIM-04` **Manual control** *(Decided)*: You can unlink speed from zoom whenever you want.
  The controls: pause, play, a speed dial, and a lock that keeps the current speed while you move the camera.

- `TIM-15` **Who sets the speed** *(Decided)*: Your pause and speed lock beat the story director (`TIM-02`), and the director beats zoom.
  Choosing a power pauses time.
  Overnight mode (`TIM-12`) ignores the director, but keeps its moments for the morning.

### 5.2 The story director

- `TIM-02` **Story director** *(Decided)*
  - **What:** The director watches the whole world for important moments and adjusts the speed of time around them.
    When nothing important is happening, it lets quiet years race past, up to the top speed your zoom allows.
  - **When something important happens elsewhere:** time slows, a live moment appears (`PRE-08`), and one tap takes you there.
    You stay in control of the camera.
  - **What counts as important:**
    - firsts: the first time anyone does something new;
    - births and deaths among the people you follow;
    - discoveries spreading or being lost;
    - conflicts, disasters and migrations;
    - a band forming, splitting or ending;
    - the consequences of your own interventions.
  - **Why:** In a world that runs itself, the best moments are easy to miss (`RSK-03`).

- `TIM-03` **The director never touches events** *(Decided)*: The director controls speed only.
  It decides where to slow down but never causes, changes or hides anything.
  Follows from `PRN-10` and `PRN-12`.

- `TIM-11` **Skip to the next moment** *(Decided)*: A control that runs time at top speed until the next important moment, then slows down.
  Useful for short check-ins (`VIS-10`).

### 5.3 While you're away

- `TIM-05` **Pauses when closed** *(Decided)*: When the app is closed or in the background, the world stops.
  Nothing happens while you're away, and every session starts exactly where the last one ended.
  Opening the app resumes time.

- `TIM-12` **Overnight mode** *(Decided)*
  - **What:** Leave the app open on the charger and switch on overnight mode.
    The world runs at top speed with the screen dimmed.
    When you come back, a summary tells you what happened, drawn from the chronicle (`PRE-05`).
  - **Why:** Deep simulation runs slowly on a phone (`PRN-11`).
    Overnight mode gives history the hours it needs without you having to watch.
  - **Safeguards:** it runs only while the phone is charging, and it stops if the phone gets too hot.
  - **Example:** You start it before bed.
    In the morning: "312 years passed.
    Two bands merged by the river; a long drought pushed the eastern band over the hills; on the coast, someone began drying fish."

### 5.4 Going back

- `TIM-06` **Rewind and branch** *(Decided)*
  - **What:** Go back to any saved moment in a world's history (`PRN-15`) and carry on from there, changing something or nothing.
    The original timeline is kept, and the new one becomes a branch.
  - **Chance is local:** each chance event belongs to one being and one moment.
    On the same phone and version, a branch therefore differs from the original only where its changes reach, and a branch with no change repeats the original.
    So a comparison shows what a change did, not luck.
  - **Why:** Curiosity (`VIS-08`): the only way to really answer "what if?".
  - **Example:** You rewind to before the plague, send a mild winter instead, and compare the two histories (`MOM-10`).

- `TIM-13` **Comparing timelines** *(Decided)*: Two branches side by side: their chronicles, their maps, and key numbers (population, discoveries, languages, beliefs), with the moment they split clearly marked.

- `TIM-08` **Saved worlds and timelines** *(Decided)*: Several worlds, each with its own tree of timelines, kept on the phone.
  Branches can be named, and you can switch between them.

- `TIM-14` **Dates** *(Decided)*: The game counts years from the moment a world's history begins ("year 2,314"), with days and seasons set by that world's own sun and moons (`WLD-06`).
  The people's own calendars are separate (`CUL-13`).
  Inside the simulation, and in every target and criterion, time is counted in Earth days and years; on screen, dates use the world's own years and ages use Earth years.
  Bodies are adapted to their world's day length.

### 5.5 Pacing and endings

- `TIM-07` **Pacing** *(To test)*: How fast history runs is measured and tuned during development (`PLT-04`).
  The first target for the tests: a thousand years in one night for a world of a few hundred people.

- `TIM-09` **If everyone dies** *(Decided)*: The world goes on without them.
  Nature carries on, and you can keep watching, rewind to before the end, or start a new world.

## 6. World

The world is a small planet with everything a planet has: rock, water, air, plants, animals and microbes, all following real rules.
This section defines the world's shape and size, how a world is made, how detail is managed, and each natural system.
What matter is made of is in Matter and physics; how animals think is in Minds.

### 6.1 Shape and size

- `WLD-01` **Torus with latitude** *(Decided)*
  - **What:** The map wraps around in both directions.
    Walk east long enough and you come back from the west.
    An equator runs across the middle of the map, and the poles lie along the line where it wraps north–south.
    Climate zones and seasons behave as on a planet, with seasons reversed between the northern and southern halves.
  - **The polar seam:** Along the line where the map wraps north–south lies a wide, permanent ice cap.
    Weather systems stop at it, and it is too wide and barren for any animal or person to cross, so nothing ever passes from one pole to the other.
    This is a stated exception to real physics (`PRN-05`).
  - **Why:** There are no edges and no stretched or squashed regions, so every place can be simulated in the same way.

- `WLD-02` **Globe view** *(Decided)*: Fully zoomed out, the world is drawn as a globe.
  The wrap only shows at the poles.
  The globe squeezes the polar regions, which on the map are as wide as the equator; this is a known exception in the display only, and the map keeps every place at its true size.

- `WLD-03` **Size** *(Decided)*
  - **What:** About 1,000 km from pole to pole and about 2,000 km around: roughly 2 million km² in all, land and sea together.
  - **What follows:** Each climate zone is roughly 100 km wide, about four to five days' walk.
  - **Why:** It is big enough for many separate peoples and small enough to simulate deeply (`PRN-02`).

- `WLD-30` **What scales with the world** *(Decided)*: Quantities set by distance (weather systems, ocean currents, migrations and climate belts) scale with the world's size.
  Local quantities (bodies, chemistry, materials and rates of change) stay real.
  Every scaled value is labelled as scaled, with the real value it came from (`PRN-05`).

- `WLD-04` **How many people it can feed** *(To test)*: Estimated at roughly 50,000 hunter-gatherers (about one person per 10 km² of good land), or about half a million to five million once farming exists, since farming supports 10 to 100 times more people on the same land.
  These are orders of magnitude only: on the wrap-around map a third of the area lies beyond 60° latitude, so there is less good land than Earth intuition suggests.
  Measured in experiments.

### 6.2 The planet

- `WLD-06` **Varied within reason** *(Decided)*: Each world has its own day length, year length, axial tilt (and so the strength of its seasons), moons, and share of land, all within ranges that allow human-like life.
  The ranges: day 18–36 hours, year 250–500 days, tilt 5°–35°, 0–3 moons, 25–50% land.
  Gravity, air and chemistry stay Earth-like.

- `WLD-07` **A rich sky** *(Decided)*
  - **What:** The sun, moons, stars and planets move realistically for each world's orbit and tilt.
    Eclipses, comets, meteor showers and auroras happen.
  - **Why:** The sky is the first calendar, the first compass and a great source of myth (`CUL-13`).
  - **Example:** A comet that hangs over the valley for a month, the same month the old chief dies, becomes part of how the band remembers that winter.

### 6.3 Making a world

- `WLD-08` **Realistic, not from scratch** *(Decided)*: Worlds are generated directly in a realistic present-day state, using fast methods that imitate what deep time would have produced.
  Generating one is cheap.

- `WLD-09` **What generation produces** *(Decided)*: using rules derived from real physics and calibrated to Earth, not full physical models, generation produces, in this order:
  1. tectonic plates, mountain ranges, volcanoes and faults;
  2. rock types and layers, with minerals and ores in geologically plausible places;
  3. erosion: valleys, rivers, lakes, deltas and coastlines;
  4. climate, worked out from the geography (`WLD-16`);
  5. soils, from rock, climate and time;
  6. vegetation and landscapes;
  7. animals and microbes adapted to them (`WLD-19`).

- `WLD-19` **Species from Earth families** *(Decided)*
  - **What:** Earth's families of plants and animals (deer, wolves, wild cattle, salmon, grasses, birches, oaks, berries and so on) are the starting point.
    Generation adapts them into each world's own species to fit its landscapes.
    Every species gets its traits: size, diet, behaviour, seasons, and the chemistry of its body, which decides what is edible, poisonous, medicinal or useful (see Matter and physics).
  - **Why:** Familiar enough to understand, new enough that each world has its own tree of life to discover.

- `WLD-23` **Richness of life** *(Decided)*: About 50 animal and 200 plant species per world, across all groups: mammals, birds, fish, shellfish and insects; trees, shrubs, grasses, herbs and fungi.

- `WLD-10` **Generate many, keep the best** *(Decided)*
  - **What:** The generator makes many candidate worlds, scores each one, and never edits them.
    "New world" shows the best three as small globes, each with a one-line summary.
    You pick one or let the game pick, and you can also enter a seed instead.
  - **What scores well:**
    - varied landscapes and climates;
    - natural barriers (mountains, seas, deserts) that let separate cultures form;
    - resources spread unevenly (flint here, copper there);
    - a good place to begin (`WLD-24`).

- `WLD-24` **Where history begins** *(Decided)*: The bands start in a temperate region with caves, fresh water and varied food within reach.
  The region is found by the scoring, never placed by hand.

- `WLD-11` **Generation time** *(Decided)*: Generating the candidate worlds and finding the best three takes a few minutes in total on the phone.

### 6.4 Detail

- `WLD-12` **Detail where it matters** *(Decided)*: Each system runs at the coarsest scale that keeps it true.
  Climate is worked out region by region; rivers and soils kilometre by kilometre; plants and animals in patches of a few hundred metres.
  Everything goes down to the metre where people are, or where something new or critical is happening.
  Where you look changes only the picture, never the simulation (`WLD-13`).
  Follows from `PRN-11`.

- `WLD-13` **Looking changes nothing** *(Decided)*: Where you look never changes what happens.
  Fine detail drawn for the picture is generated the same way every time, and never contradicts what was simulated.
  Follows from `PRN-10`.

### 6.5 Natural systems

- `WLD-29` **Systems feed each other** *(Decided)*: All the natural systems below are simulated in depth, and each feeds the others: weather shapes soils and plants, plants feed animals, fire and floods change the land, and people come to change them all (`WLD-25`).

- `WLD-14` **Geology and materials** *(Decided)*
  - **What:** Rocks, minerals, soils and ores lie in realistic places, so what can be discovered depends on what's underfoot.
  - **Example:** Flint comes out of chalk and limestone, obsidian near volcanoes, copper ores in certain mountains, clay along rivers, salt in dry basins.

- `WLD-15` **Living geology** *(Decided)*: Change continues during play.
  Erosion wears the land, rivers shift their course, landslides fall, earthquakes strike along faults, volcanoes erupt, and coastlines move as the sea rises and falls.

- `WLD-27` **Soils** *(Decided)*: Soils form from rock, climate, plants and time.
  They hold water and nutrients, decide what grows where, and can later be enriched or exhausted by people.

- `WLD-16` **Climate and weather** *(Decided)*
  - **Climate from geography:** Each place's climate (rain, temperature and winds through the seasons) is worked out by rules derived from real physics and calibrated to Earth, not by a full physical climate model: latitude, height, distance from the sea, prevailing winds, and mountains that block rain.
  - **Daily weather** is drawn from that climate, with storm systems that move across the land.
  - **Long cycles:** ice ages and warm periods follow real cycle lengths, tens of thousands of years long, moving coastlines and pushing migrations.
    Worlds begin as an ice age ends, so seas rise over the first ten thousand years or so and can cut bands apart (`MOM-05`).
    A great eruption can cool the world for a few years.
  - **Example:** Rain clouds coming off the western sea drop their rain on the mountains, so the valleys beyond are dry grassland with forest only along the rivers.

- `WLD-05` **Climate on a small world** *(Decided)*: Climate zones sit closer together than on Earth, a few days' walk apart, and weather systems are scaled to fit the world (`WLD-30`).

- `WLD-25` **People change the climate** *(Decided)*: What covers the land and, much later, fuel burned at scale feed back into the climate through the same physics.
  Clearing a forest can dry a region; centuries of burning could warm the world.

- `WLD-17` **Fresh water** *(Decided)*: Rivers, lakes, wetlands, springs, underground water, ice and floods.
  Life and settlement gather around them.

- `WLD-26` **Seas** *(Decided)*: Oceans with currents that carry heat and moisture, tides set by the moons and the sun (so worlds without moons still have weaker tides), and a sea level that rises and falls with the ice ages.
  At low tide, shellfish beds are exposed on the shore.

- `WLD-18` **Ecology** *(Decided)*
  - **What:** Plants grow, flower, fruit and die back with the seasons.
    Animals eat, breed, migrate and die.
    Everything is tied together in food webs, with populations that boom and crash.
  - **Why:** It is what people live from, and what they will one day change.
  - **Example:** A run of mild winters lets the deer multiply; the wolves follow; then a hard winter cuts both down, and the hunters go hungry.

- `WLD-28` **Fire in the landscape** *(Decided)*: Lightning and dry fuel start wildfires, which spread with wind and slope; landscapes regrow after them, and some plants depend on fire.
  People can learn to use fire on the land.

- `WLD-20` **Heredity in plants and animals** *(Decided)*: Inheritance continues during play, so adaptation and domestication (wolves into dogs, wild grasses into grain) can happen on their own.

- `WLD-21` **Microbes** *(Decided)*: Rot, fermentation and disease are living microbes that spread and evolve.
  Crowding, and living close to animals, bring epidemics.

- `WLD-22` **Natural disasters** *(Decided)*: Eruptions, earthquakes, floods, droughts, storms, wildfires and lightning come from the world's own systems, not only from you.
  Follows from `GOD-05` and the natural systems in this section.

How animals think is covered in `MND-16`.

## 7. Matter and physics

This is where "no recipes" lives.
Nothing in the world is a recipe item: everything is matter with real chemistry and structure, changed by a few dozen general laws using real-world numbers.
Discovery means people finding out what those laws allow (`PRN-01`, `PRN-07`).

### 7.1 What things are made of

- `MAT-01` **Made of real ingredients** *(Decided)*
  - **What:** All matter is built from real ingredients: real minerals, compounds and the substances of living things.
    Results come from how these interact, never from rules written for each material.
  - **Examples by group:**
    - **rock and minerals:** silica (as quartz, flint, chert, obsidian or sand), calcite (limestone, chalk), clays, iron oxides (yellow and red ochre), copper minerals, tin ore, salt;
    - **water and air:** water as ice, liquid and vapour; the gases of the air;
    - **living matter:** cellulose and lignin (wood, plant fibres), starches, sugars, proteins, fats, collagen (hide, sinew, bone), bone mineral, resins, tannins, and the plant chemicals that make things poisonous or medicinal.
  - **How it works:** the ingredients catalogue (`MAT-13`) gives each ingredient its elements (`MAT-09`) and, for each state (solid, liquid and gas), its measured values:
    - density, and melting and boiling points;
    - heat capacity, and how well it conducts heat;
    - stiffness, hardness and resistance to cracking;
    - how it burns: ignition temperature, heat released and air needed;
    - how it dissolves;
    - colour and gloss;
    - for foods and poisons, nutrition and effects per dose.

    A thing's properties come only from its ingredients and structure (`MAT-03`); no rule ever reads a thing's name.

- `MAT-09` **Elements and energy are kept** *(Decided)*
  - **What:** Every ingredient has its real elemental makeup (carbon, hydrogen, oxygen, nitrogen, silicon, calcium, iron, copper, tin and so on), and every change keeps elements and energy balanced.
    Nothing ever comes from nothing.
  - **How it works:** every law is written as a balanced change: the ingredients going in and coming out, with the elements counted on both sides, and the heat taken in or given off, from measured values.
    Gases go into the air of the place, such as smoke and steam; ash stays behind as a thing.
    An automatic check runs every law on test cases and fails if any element or any energy appears or disappears.
  - **Why:** It makes the world honest, and it keeps the door open to any chemistry people might reach later (`VIS-03`).
  - **Example:** Smelting copper ore yields exactly the copper that was in it, plus gases.
    Burning wood releases the energy stored in it as heat and light, and leaves ash holding its minerals.

- `MAT-02` **Structure matters** *(Decided)*
  - **What:** Matter also records how it's put together: crystal or glass, fibrous, porous or dense, coarse or fine grain, wet or dry.
    Grinding, melting, cooling and drying change structure without changing makeup.
  - **How it works:**
    - **Stored for each thing:** its form (crystal, glass, fibre, grains, powder, paste or liquid), grain size, pores (the share of empty space), the direction of any fibres or layers, moisture, and how many tiny flaws it has.
    - **Changed only by laws (`MAT-04`):** grinding makes grains finer; melting turns any form to liquid, and cooling gives glass if fast or crystals if slow; drying removes moisture; heating clay past a measured temperature turns grains and water into a fired solid; gentle heating removes flaws (`RCK-10`).
  - **Example:** Sand, flint and obsidian are all mostly silica, but only flint and obsidian chip into blades.
    Sand melted with plant ash and cooled becomes glass.

- `MAT-03` **Properties from data and rules** *(Decided)*: Every property comes from measured data where it decides what is possible, and otherwise from estimates, combined by stated rules for mixtures and structures (`MAT-05`):
  - **mechanical:** weight, hardness, strength, toughness, springiness, and how it breaks (in shell-like flakes, in splinters, or by crumbling);
  - **heat:** how it burns, melts, holds heat and passes it on;
  - **water:** how it soaks up water, dissolves, softens or swells;
  - **the body:** nutrition, poison, medicine, taste and smell;
  - **the senses:** colour, sheen, texture, and the sound it makes when struck (`MND-03`);
  - **time:** how fast it rots, rusts, wears or weathers.
  - **How it works:** each property is a rule over makeup, structure and temperature, worked out when one of them changes and kept until the next change.
    For example:
    - **how it breaks:** in shell-like flakes when the solid is glassy or very fine-grained and even throughout, by crumbling when coarse-grained, and by splitting along fibres when fibrous (`RCK-01`);
    - **density:** the ingredients' densities, weighted by their shares, less the pores;
    - **heat:** heat capacity and conduction mixed by share, and burning from the burnable ingredients;
    - **food energy:** from protein, starch, sugar and fat, at measured values per gram;
    - **for the senses:** colour mixed from the ingredients, gloss from glassy structure, and the sound when struck from stiffness, density and shape (`SND-06`).

    Values for pure ingredients carry their source; mixing rules are labelled as estimates (`PRN-05`).

- `MAT-10` **Things** *(Decided)*: Everything in the world is a thing with a makeup, a structure, a shape, a size and a temperature.
  Things can be split, joined, worn down, heated, mixed and carried.
  - **How it works:**
    - **Everything exists, fixed by the seed:** the world generator defines all matter everywhere, such as rock layers, soil, loose stones, fallen wood and sand, with each patch's kinds and amounts set by its geology and plants (`WLD-09`).
      Any single stone is fixed by the seed: generating it twice gives the same stone.
    - **Stored once touched:** a piece of matter becomes a stored thing the moment anything acts on it (picks it up, strikes, moves, burns or eats it), and it stays stored from then on.
      Each patch records what was taken from it and what was left in it; places where nothing has changed store nothing, and are regenerated from the seed when needed.
    - **A thing's record**, about 100 bytes: its makeup (up to about 8 ingredients with their shares by mass, `MAT-01`), its structure (`MAT-02`), its shape (a simple form with sizes, such as a slab, rod, block, lump, sheet or tube), its mass, its temperature at the surface and at the core, where it is (on the ground, held, inside or tied to something), and who last changed it and when.
    - **Fine shape on demand:** when an action depends on exact shape, such as striking a stone to break it, carving, or fitting two pieces together, the simple form is refined into a detailed 3D surface, the same way every time, and kept.
    - **Small units in bulk:** berries, seeds and sand are kept as one lot (so many units, with one total mass) until a unit is taken out.
    - **Joined things** keep their parts as things, plus each joint: tied, glued or fitted, and its strength.
    - **Bulk water and air are not things:** rivers, lakes and the air belong to the world's water and weather systems (`WLD-16`, `WLD-17`); water in a container is a thing.
    - **Leftovers merge after a season:** ordinary leftovers, such as knapping debris, that nothing has touched for a season merge into their patch's record: so many pieces, of what, made by whom and when.
      Anything later taken from that record is generated from it, the same way every time.
      Tools, art, graves, hearths and anything a key moment depends on always stay individual (`MAT-08`, `PRN-15`).

### 7.2 How things change

- `MAT-04` **A few dozen general laws** *(Decided)*: Change comes from general laws, each decided by real data on heat and rates of change.
  No law ever names a product.
  The starting list:
  - **force:** breaking, cutting, scraping and grinding, bending and springing back, pressing and pounding, friction, twisting and binding, joining by tying, gluing or fitting;
  - **heat:** heating and cooling, burning with more or less air, charring, melting and setting, drying, roasting;
  - **water:** wetting and soaking, dissolving and leaching, swelling, freezing;
  - **flow:** floating and sinking, and flowing water and air, such as a draught that feeds a fire;
  - **vibration:** how struck, plucked or blown things ring (`SND-06`);
  - **chemistry:** metals giving up or taking up oxygen (smelting and rusting), minerals breaking down when heated (as limestone does), taking up gases from the air (as lime does when it sets), tannins binding to proteins, and fluxes lowering the melting point of silica;
  - **life:** growing, digesting, healing, rotting and fermenting, with microbes at work (`WLD-21`).
  - **How it works:**
    - **A law is a rule over properties:** it states which things it applies to, by their properties and situation and never by name; what it computes; and when it runs.
      Its numbers come from the catalogues (`MAT-13`), with sources wherever they decide what is possible.
    - **Laws at a contact** (breaking, cutting, scraping, pressing, bending, joining) run when force is applied, by an action (`MAT-06`) or by something falling, rolling or flowing.
      They compute the result from the force, speed, angle and point of contact and the materials' properties.
      For example, the breaking law decides whether a strike knocks a piece off, and gives the piece's shape: a flake from glassy stone, fragments from coarse stone, a split along the grain of wood.
    - **Laws over time** (heating and cooling, burning, drying, wetting, dissolving, freezing and melting, chemical change, rotting) run as rates for as long as their conditions hold.
      The rates come from measured data: how fast heat flows, how fast fuel burns with the air it gets, how fast a reaction goes at a given temperature.
    - **Only what is changing is computed:** a thing under a law over time is checked again sooner or later depending on how fast it is changing: every few seconds in a fire, hourly for a drying hide, never for a cold, dry stone.
      Untouched matter costs nothing.
    - **Fire is a law, not a thing:** burning things form a fire, whose heat balance is worked out at each step.
      Heat comes in from the fuel burned, limited by the air that reaches it: still air, a draught through gaps, or someone blowing.
      Heat goes out to the air, to the surroundings and into the ground.
      Stones, earth or walls around a fire hold heat and cut its losses, so open, enclosed and blown fires reach different temperatures without any of them being named; fire temperatures are results, never set (`MAT-05`, `RCK-02`, `RCK-08`, `RCK-22`).
    - **Vibration** is the law the sound tests already use: a struck, plucked or blown thing rings at frequencies set by its stiffness, density and shape (`SND-06`).
    - **Life's laws** (growing, digesting, healing, rotting and fermenting) run over time on living ingredients, with microbes as living things (`WLD-21`); their details are written with bodies and the world.
    - **Checks:** every law has its reality checks and its balance check (`MAT-09`), and the general-rules check (`PRN-07`) searches the law code for product names.
    - **Open:** knapping's breaking rule comes from experiments that relate flake size to how deep into the edge and at what angle a stone is struck, mostly on glass cores; whether it holds for the varied stones people pick up is tested in Experiment 1 (`RES-02`).
      Some chemistry, such as tanning and fermenting, has no measured rate and rests on estimates, labelled as such.

- `MAT-07` **One law, many inventions** *(Decided)*: Laws are general enough that one law covers many inventions.
  For example, "metal ores give up their metal when heated hot enough in contact with burning charcoal" covers copper, tin, lead and iron.
  Each needs its own real conditions, so they become possible in a natural order that nobody wrote down.
  Follows from `PRN-07`.
  - **How it works,** taking smelting as the example: any metal-bearing mineral touching burning charcoal gives up its metal once the fire passes the temperature at which charcoal pulls oxygen from that metal more strongly than the metal holds it; standard measured tables give that point for each metal.
    The metal melts only if the fire is also hotter than its own melting point.
    So lead and tin come out at lower temperatures, copper runs out as liquid at about 1,085 °C, and iron comes out as a spongy lump that such a fire never melts (`RCK-18`): one law, measured data, and an order nobody wrote down.

- `MAT-06` **Actions are physical** *(Decided)*: Every action has force, angle, speed, duration and temperature, and the physics decides the outcome.
  Technique matters: a clumsy strike shatters the stone.
  - **How it works:**
    - **An action is an instruction from a mind to its body:** which basic action (`MAT-12`), with which hand or body part, the thing held if any, the target (a thing or a spot), and its settings: force or speed, direction and angle against the target's surface, point of contact, duration and number of repeats.
    - **The body carries it out within its limits:** strength caps force and speed, and reach and posture limit where it can act.
    - **Error is real:** the settings that reach the target are the intended ones plus a random error.
      Practice shrinks the error (`MND-06`); fatigue, cold hands, poor light and haste grow it.
      Each error comes from that being's own chance at that moment (`TIM-06`).
    - **The physics decides the result:** the contact laws (`MAT-04`) take the actual settings, not the intended ones.
      Struck at the right point and angle, a stone gives off a flake; struck too hard or off the point, it crushes or shatters.
    - **Named simplification:** the body is not simulated muscle by muscle.
      Each action is a stroke with physical settings, limited by the body, and the picture animates it.

- `MAT-11` **Mechanics** *(Decided)*: Weight, momentum, leverage, springiness and friction follow real physics.
  Throwing sticks, spear-throwers and bows can work only because the physics makes them work.
  Follows from `MAT-06`.
  - **How it works:**
    - **Moving things:** anything set moving (thrown, dropped, falling or rolling) follows its path in fine steps, with gravity, air drag from its shape, and spin.
      It stops at what it hits, and the contact laws decide what happens there.
    - **Throwing:** release speed comes from how fast the body swings, times how far from the pivot the thing is held.
      A spear held at the end of a rigid stick is further from the pivot, so the same arm throws it faster and further; that is the only reason a spear-thrower works.
    - **Bending:** a bent thing stores energy set by its stiffness (from its material and thickness) and by how far it is bent, and gives it back, less losses, when released.
      So a bent stick and a cord can drive a dart, if the numbers make it worth it.
    - **Levers:** force applied far along a rigid thing is multiplied at the near end.
    - **Friction:** sliding under pressure turns work into heat where the surfaces touch, set by the pressure, the speed and the materials, and the heat law spreads it.
      So fast, hard twirling of dry wood can push its dust past ignition (`RCK-02`).
    - **Weight:** carrying costs the body energy, by weight and distance (`BIO-09`).

- `MAT-12` **What a body can do** *(Decided)*: The basic actions everything else is built from: grasp, carry, put down, drop, throw, strike, press, rub, twist, bend, tear, dig, cut or scrape with an edge, pierce with a point, pour, blow, chew, and put into fire or water.
  Anything more, such as knapping, sewing or smelting, is a sequence of these that someone has to learn.
  These are actions on matter; moving, eating, sleeping and acts between people are in `BIO-21`.
  - **How it works:** each basic action is defined by the contact or motion it creates and the laws it calls:
    - **grasp:** holds a thing if it fits the hand and is not too heavy.
      Touching it gives the senses its weight, warmth, texture and sharpness, and a sharp edge gripped hard can cut the hand (the injury law);
    - **carry, put down and drop:** move a thing with the body, or let it go; a dropped thing falls (`MAT-11`);
    - **throw:** releases a thing at speed (`MAT-11`);
    - **strike:** drives the hand or a held thing into a target, calling the contact laws: breaking, crushing and cutting;
    - **press:** force spread over an area presses; through an edge it cuts, and through a point it pierces;
    - **rub:** sliding under pressure, giving friction heat and wear: grinding, polishing, fire by friction;
    - **twist:** turning force, which twists fibres together or bores a hole;
    - **bend:** turning force on a long thing, which springs back or breaks;
    - **tear:** pulls a thing apart;
    - **dig:** moves earth into a heap, and the patch records it;
    - **cut or scrape with an edge, and pierce with a point:** edges drawn and points driven, calling the cutting and piercing laws;
    - **pour:** tips a container so liquid flows out;
    - **blow:** pushes air from the lungs, at a measured rate, at a spot, and a fire there gets that air (`MAT-04`);
    - **chew:** crushes and softens what is in the mouth;
    - **put into fire or water:** moves a thing there, where the laws over time take over.

    Sequences are learned, never built in: knapping or sewing exist only as skills a mind has learned (`MND-06`), sequences of these actions with learned settings.
    Nothing in the body or the laws knows them.

- `MAT-08` **Traces last** *(Decided)*: Hearths, tools, bones, graves and rubbish heaps stay in the world and get buried over time, feeding the archaeology view (`PRE-09`).
  What survives depends on the material and the ground: stone lasts; bone survives in dry caves and limestone; wood, hide and plant fibre usually rot, except in waterlogged, frozen or very dry ground.
  - **How it works:**
    - **Burial:** each patch's surface builds up at a rate set by the land: a river flat silts up in floods, a cave floor gathers dust and fallen rock, and a slope loses soil (`WLD-15`).
      Things lying there sink into the layer of their time.
    - **What survives** is decided by the slow laws over time, still running in the ground: rot needs water, air and warmth; acid ground dissolves bone and lime-rich ground keeps it; waterlogged, frozen or very dry ground stops rot.
      Each layer's wetness, air and acidity come from the soils (`WLD-27`).
    - **Checked rarely:** buried things are checked every few years, since they change slowly, so a buried world costs little.
    - **For archaeology:** notable things stay individual and ordinary leftovers stay merged in their layer's record (`MAT-10`); the archaeology view reads both (`PRE-09`).

### 7.3 Real numbers

- `MAT-05` **Real-world values** *(Decided)*
  - **What:** The numbers that decide what is possible, such as temperatures, hardness, energy content and toxic doses, come from real measurements, each with its source; the rest are estimated by stated rules or plausible ranges (`PRN-05`).
  - **Example:** Copper melts at about 1,085 °C.
    An open wood fire reaches roughly 600–900 °C; a charcoal furnace with forced air passes 1,100 °C.
    So copper waits until someone builds a hotter fire.
  - **How it works:** values that decide what is possible live in the catalogues with their source and a quote, are checked once and locked (`RSK-16`): melting points, ignition temperatures, heats of burning and of reaction, hardness, resistance to cracking, toxic doses and nutrition.
    Fire temperatures are not stored values: they come out of the fire law (`MAT-04`) and are checked against real ranges, such as the two above.
    Estimated values name the rule or range they come from, and tuned values are listed in every milestone report (`PRN-05`).

### 7.4 How matter grows

Matter must be easy to extend, forever (`PRN-14`).

- `MAT-13` **Four catalogues** *(Decided)*: Matter is described in four catalogues: ingredients, structures, laws and reality checks.
  Each entry stands alone, is written in plain language a person can read and check, gives its key values with their sources and the rules or ranges for the rest, and names the reality checks that prove it.
  Once checked, a key value is locked: changing it means sourcing and checking it again (`RSK-16`).

- `MAT-14` **Adding without rewriting** *(Decided)*
  - **What:** Adding an ingredient, structure, law or check never requires changing the others.
  - **Why it works:** Laws never name products (`PRN-07`), so a new ingredient automatically takes part in every existing law.
  - **Example:** Adding tin ore needs no new smelting rule; the smelting law already covers it.

- `MAT-15` **Every addition proves itself** *(Decided)*: Each new entry comes with the reality checks it must pass, and the whole checklist runs again, so nothing that worked before breaks.

- `MAT-16` **Matter grows in layers** *(Decided)*: Matter is built in layers, in this order, each added without rewriting the earlier ones.
  The implementation plan sets when each arrives:
  1. stone, wood, bone and water;
  2. heat and fire;
  3. food and the body's chemistry;
  4. fibres, hides and joining;
  5. clay, lime and pigments;
  6. metals and glass;
  7. further layers as experiments call for them.

- `MAT-17` **How reality checks work** *(Decided)*: Each check has a real-world range and, where it helps, a "must not" partner, such as "green wood doesn't light by friction".
  Results are judged by properties, not names: leather is hide that stops rotting and stays supple.
  A check becomes active once its layer is built (`MAT-16`).
  This file keeps the checks that define what the world must do; the catalogues hold the rest, each naming the item it supports (`MAT-13`).

### 7.5 Reality checklist

The physics must reproduce these without any rule written specially for them (`PRN-07`).
How the checks work is set out in `MAT-17`, and they all run again whenever anything changes (`MAT-15`).

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
- `RCK-21` **Floating** *(Decided)*: A dry log floats; a stone sinks.
- `RCK-22` **Air feeds fire** *(Decided)*: Blowing on embers makes them hotter.

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

Every person has a body that must be fed, watered, kept warm and rested; that can be hurt, fall sick and heal; and that grows, ages, has children and dies.
All of it follows real biology with real-world numbers (`PRN-05`).
How people think is in Minds.

### 8.1 Who they are

- `BIO-01` **One species, modern minds** *(Decided)*: Their bodies and brains are as capable as ours.
  Their culture starts almost empty.

- `BIO-02` **Starting kit** *(Decided)*: The first people are generated like the world: realistic, not grown from nothing.
  Families, ages and relationships follow real hunter-gatherer patterns.
  Each adult knows their home range (its food, water, dangers and seasons) and nothing beyond it.
  The details are in `BIO-20`.
  - **Language:** a few dozen shared words and calls; grammar must grow.
  - **Fire:** they can feed a fire found after lightning or a wildfire, but cannot make one.
    A band may start with a fire it is keeping, depending on recent weather.
  - **Tools:** unshaped stones for bashing, and sticks.
  - **Clothing:** none.
  - **Shelter:** natural caves and overhangs.
  - **Food:** gathering, scavenging, some ambush hunting.
  - **Beliefs:** only the practical knowledge of their home range; none about spirits, hidden causes or how to make things.

- `BIO-03` **Starting population** *(Decided)*: 3–4 family bands of 15–30 people each, about 45–120 people in all, living in one region (`WLD-24`).

- `BIO-20` **Starting knowledge in detail** *(Decided)*:
  - **Bands:** each band is a few related families.
    The bands are neighbours who sometimes meet, and share one language.
  - **What adults know:** where water, shelter and the main foods are in each season; which local plants and animals are food, which are poison and which are dangerous; the routes of their home range; who is kin to whom.
    Children know less, according to their age.
  - **Words:** water, fire, food, danger, kin, the main animals and plants of home, and simple actions such as come, go, eat and look.
  - **Fire:** they know how to carry embers to keep a fire alive on the move.
  - **Memories:** adults begin with their knowledge but no remembered events; their stories start at year 0.

- `BIO-08` **Everyone is different** *(Decided)*: Height, strength, stamina, senses, health, temperament, curiosity, memory and learning speed vary from person to person, with real-world spreads.
  These traits are partly inherited and partly shaped by how a person grew up, through childhood food, illness and activity.
  Follows from `BIO-06` and `PRN-05`.

### 8.2 Staying alive

- `BIO-09` **Basic needs** *(Decided)*: Food, water, warmth and sleep, in real-world amounts that depend on body size, activity and climate.
  Follows from `PRN-05`.

- `BIO-10` **Nutrition** *(Decided)*
  - **What:** Food provides energy, protein, fat and key vitamins and minerals, all from the real chemistry of what is eaten (`MAT-03`).
    A missing vitamin causes its real deficiency disease.
  - **Why:** Diet becomes something people can discover and get wrong.
  - **Example:** A band that winters on dried meat suffers bleeding gums every spring (scurvy, from a lack of vitamin C).
    Eventually someone notices that the people who ate the first green shoots recovered.

- `BIO-11` **Heat and cold** *(Decided)*
  - **Follows from:** `MAT-03` and `PRN-05`.
  - **What:** Bodies lose and gain heat by real physics.
    Clothing, shelter, fire and huddling together keep them warm.
    Cold can kill; heat exhausts.
  - **Example:** In an ice-age winter, sewn clothing can matter more than food.

- `BIO-12` **Poison and medicine** *(Decided)*: The chemistry of plants, animals and minerals acts on the body.
  Some things poison, some heal, and some do either depending on the dose.
  For example, willow bark eases pain.
  Follows from `MAT-03`.

### 8.3 Harm and healing

- `BIO-13` **Injuries to body parts** *(Decided)*
  - **What:** Wounds, fractures, burns and infections affect specific parts of the body.
    They heal, scar, or leave a lasting disability.
    Care from others (food, water, protection, cleaning a wound) changes the outcome.
  - **Example:** A hunter with a broken leg survives the winter because the band carries and feeds them.
    They never hunt again, but they become the best stoneworker in the valley.

- `BIO-05` **Disease** *(Decided)*: Illness comes from microbes that enter through wounds, food, water, air, touch or animals.
  People who recover can become immune.
  Crowding, and living close to animals, bring epidemics.
  Follows from `WLD-21`.

- `BIO-14` **Every death has a cause** *(Decided)*: Nobody dies of random chance.
  Every death comes from something in the simulation: hunger, cold, disease, injury, childbirth, violence, accident or old age.
  Follows from `PRN-10`.

### 8.4 A life

- `BIO-04` **Life cycle** *(Decided)*
  - **Follows from:** `PRN-05`.
  - **What:** Birth, childhood, adolescence, adulthood, old age and death, following the life patterns of real hunter-gatherers.
  - **Target figures** (from studies of hunter-gatherers; they must come out of the causes, never be programmed, and are checked by experiment with tolerances, `RES-14`):
    - children are weaned at about 2–4 years, and a mother has a child about every 3–4 years;
    - around four in ten children die before the age of 15;
    - adults who reach 15 often live into their 60s and 70s;
    - women stop having children in their 40s.

- `BIO-15` **Pregnancy and birth** *(Decided)*: Children come from pairs, through pregnancy, birth and nursing, with their real risks.
  Who pairs with whom, and how families are formed, is cultural (`CUL-07`).
  Pairing and conception are simulated abstractly, never as explicit acts, so sexual violence is not modelled.
  Follows from `PRN-05`.

- `BIO-16` **Ageing** *(Decided)*: Strength, senses and fertility decline with age.
  Ageing also brings wear and frailty: wounds heal more slowly and defences against disease weaken, so old age kills through real causes (`BIO-14`).
  Knowledge and experience don't decline, so elders can matter as keepers of what the band knows (`CUL-02`).
  Follows from `PRN-05`.

### 8.5 The sexes

- `BIO-17` **Real biology, culture decides** *(Decided)*
  - **What:** Bodies differ only in real biological ways: reproduction, and average differences in size and strength, with wide overlap between individuals.
  - **What doesn't:** Who hunts, gathers, leads or makes things is decided entirely by each culture, and can differ between cultures.
    The simulation never assigns a role by sex.
  - **Minds:** Minds don't differ by sex from birth.
    Every inborn mental trait has the same average in both sexes (`BIO-08`, `MND-20`), and any difference in behaviour comes from culture or from bodies.

### 8.6 Senses and actions

- `BIO-18` **Senses** *(Decided)*: Sight (limited by light, fog and distance), hearing, smell, taste and touch, each with real ranges and differences between people, and declining with age.
  They are how people learn about the world (`MND-03`).

- `BIO-21` **Moving, eating and acting together** *(Decided)*: Alongside the actions on matter (`MAT-12`), bodies move (walk, run, climb, crouch, swim), eat and drink, sleep, touch, hold and give, and communicate (call, sing, point, gesture).
  Walking, running, climbing, calling and pointing are inborn; swimming is learned, as is everything people come to do with these acts.

### 8.7 Inheritance

- `BIO-06` **Heredity** *(Decided)*: Body traits and mental traits (curiosity, memory, learning speed, temperament) pass from parents to children.
  They shift over generations at real-world speeds as some people survive and have children and others don't.
  Minds barely change over thousands of years; culture does the heavy lifting, as in our own history.

- `BIO-07` **Evolution dial** *(Decided)*: A setting speeds up genetic change for experiments (`PRN-12`).

- `BIO-22` **Looks** *(Decided)*: Skin, hair and faces are inherited (`BIO-06`) and vary by region with sunlight, as in real biology and at real speeds, so they change slowly.
  They are designed so that no people reads as a copy of a real one (`SCP-20`).

### 8.8 Animals

- `BIO-19` **Animal bodies** *(Decided)*: Animals have bodies that work in the same way, with their own species' traits (`WLD-19`): needs, injuries, disease, life cycles and senses.

## 9. Minds

How people think, and in simpler form how animals think.
Everything a mind knows is learned inside the world (`PRN-01`), every choice can be explained (`PRN-13`), and no AI language model ever thinks for anyone (`PRN-06`).

### 9.1 Ground rules

- `MND-01` **No AI language model thinks for them** *(Decided)*: Every belief and invention comes from the mechanisms in this section (`PRN-06`).

- `MND-02` **Knowledge only from inside the world** *(Decided)*: Any learning a mind does draws only on experience in its own world.
  Nothing carries real-world knowledge in.
  Follows from `PRN-01` and `PRN-06`.

- `MND-17` **Why ordinary minds are enough** *(Decided)*: No single mind needs to be a genius.
  A people's intelligence comes from four sources, and only one of them is inside a head:
  1. **a world made of properties, not recipes** (see Matter and physics), so simple learning finds real things;
  2. **small, well-understood learning mechanisms**, the ones described in this section;
  3. **many minds over generations**, copying imperfectly, varying and passing things on (`CUL-01`).
     Researchers call this cumulative cultural evolution;
  4. **time:** an accident with a one-in-ten-thousand chance happens routinely over centuries.

### 9.2 Perceiving and knowing

- `MND-03` **Senses, not labels** *(Decided)*: People perceive properties (weight, hardness, colour, smell, taste, warmth, sound) through their senses (`BIO-18`), never the game's names for things.

- `MND-04` **Their own concepts** *(Decided)*
  - **What:** People sort what they perceive into their own categories.
    Categories differ between groups and can be wrong.
  - **Example:** One band lumps flint and chert together as "cutting stone"; another confuses a poisonous berry with a safe one.

- `MND-05` **Cause-and-effect beliefs** *(Decided)*
  - **What:** "Doing this to that, in this situation, leads to this." Each belief is held with more or less certainty, which rises and falls as evidence comes in.
  - **Why:** Discovery and superstition come from the same mechanism, with different luck.
  - **Example:** Striking glassy stone makes sharp edges: a discovery.
    The band sang before a hunt that went well: a superstition, which can become a rite (`MOM-04`).

- `MND-27` **Kinds of belief** *(Decided)*: Beliefs come in several kinds, each held with a certainty and the evidence behind it: cause and effect (`MND-05`); that something exists, such as an unseen being; what others know and want (`MND-23`); rules, such as what not to eat (`CUL-20`); and plain facts, such as where the water is.

- `MND-18` **Memory** *(Decided)*: People remember:
  - events they lived through;
  - places, as a mental map with the seasons attached ("hazelnuts on the south slope in autumn");
  - people: who's who, family, and who owes whom;
  - know-how (`MND-06`) and beliefs (`MND-05`).

  Vivid and repeated memories last; others fade.
  Retelling can change a memory.

- `MND-08` **Feelings shape memory** *(Decided)*: Strong feelings decide what is remembered and how strongly.
  A terrifying storm stays for life; an ordinary day fades.

### 9.3 Wanting and feeling

- `MND-07` **Drives** *(Decided)*: Hunger, thirst, cold, tiredness, fear, belonging, status, curiosity, sexual desire and attachment.
  Nobody knows at first that sex leads to children; that has to be learned (`PRN-01`).

- `MND-19` **Feelings** *(Decided)*: Fear, anger, joy, grief, disgust, surprise, affection, shame, pride, awe, longing and hope.
  They colour choices and memories, and some last: grief for months, or a fear of the forest for life after a wolf attack.

- `MND-20` **Personality** *(Decided)*: A few inborn tendencies (curiosity, boldness, sociability, patience, dominance, readiness to conform), partly inherited (`BIO-06`) and shaped by what happens to each person.

- `MND-21` **Inborn tendencies** *(Decided)*
  - **What:** The biases evolution gave humans: a taste for sweet and fat and a wariness of bitter; quicker fear of long, legless things that move suddenly (such as snakes), of heights and of the dark; attachment between parent and child; the urge to imitate; and suspecting a hidden someone behind unexplained events.
  - **Why:** They make some lessons easier to learn but teach nothing by themselves, so the world stays the only teacher (`PRN-01`).
  - **More:** further tendencies are in `MND-26`.

- `MND-26` **More inborn tendencies** *(Decided)*: Added to `MND-21` from research, each with its sources and a comparison run (`RES-10`) showing what it changes:
  - pain, and avoiding what causes it;
  - favouring kin, and caring for the hurt and the sick;
  - returning favours, and anger at cheats;
  - favouring one's own group;
  - not desiring those one was raised with;
  - shared attention and pointing;
  - readiness to learn words;
  - moving together to a beat.

### 9.4 Deciding and doing

- `MND-09` **Choosing what to do** *(Decided)*: Habits handle routine.
  Deliberate planning takes over when habits fail or the stakes rise: working backwards from a need through what they believe causes what.
  People explore most when they are comfortable (play) and when they are desperate (need).

- `MND-06` **Skills** *(Decided)*
  - **What:** Learned sequences of actions with fine control (angle, force, timing), built from the body's basic actions (`MAT-12`), that improve with practice.
  - **Why:** Knowing something can be done is not the same as doing it well.
  - **Example:** A child who has watched knapping knows that striking makes flakes, but shatters a dozen stones before getting one good edge.

- `MND-13` **Learning over a lifetime** *(Decided)*: People get better at things through their own experience.

- `MND-22` **Planning ahead** *(Decided)*: People can plan days and seasons ahead once they have learned the patterns, such as storing nuts before winter.
  Tools from culture, such as calendars, counting and records, make longer plans reliable (`CUL-03`).

### 9.5 New ideas

- `MND-10` **Curiosity in minds** *(Decided)*: Attention goes where expectations fail.
  Surprises are remembered and tried again.

- `MND-11` **Where new ideas come from** *(Decided)*:
  - accidents someone notices;
  - watching nature, such as fire after lightning, or seeds sprouting from a rubbish heap;
  - tinkering with skills they already have;
  - analogy: what works on wood might work on bone;
  - dreams (`MND-12`).

  Every idea is a guess until the physics says yes or no.

  **How fire-making could be discovered with no recipe:**
  1. A band keeps fires found after lightning.
     They have learned that dry wood feeds fire, rain kills it, and losing it means cold nights and wolves.
  2. The physics knows that friction makes heat, and that dry tinder catches fire above a certain temperature.
     There is no "make fire" rule.
  3. Someone twirls a stick against wood to bore a hole.
     The tip gets hot and smokes.
     Smoke means fire to them, so this is surprising and is remembered as a weak hunch.
  4. Winter comes and their fire goes out, so the need is desperate.
     A curious person tries twirling again, faster, longer, with drier wood.
     An ember appears, then the tinder catches, then flame.
  5. Others watch and copy imperfectly.
     Some succeed, teach others and improve the method.
     The band now knows how to make fire, a skill nobody programmed.
  6. You might help: during that crisis, a dream puts "smoking stick" next to "fire" in the most curious person's head (`GOD-03`).

- `MND-12` **Dreaming** *(Decided)*: During sleep, people replay and recombine their own memories.
  This strengthens what they learned, sometimes connects things in a new way, and is also your lever (`GOD-03`).

### 9.6 Other minds

- `MND-23` **Understanding others** *(Decided)*
  - **What:** People track what others know, want and believe, and can reason one step deeper ("she thinks I don't know").
  - **Why:** This is what makes teaching, cooperation, gossip and deception possible.
  - **Example:** Tamo keeps a good flint source secret, believing nobody knows about it.
    Ama has noticed the fresh flakes Tamo brings back, and follows one morning.

- `MND-24` **Relationships** *(Decided)*: People know who's who: family, friends, rivals, and who owes whom.
  Trust and affection grow and fade with shared experience.

### 9.7 Scale and inspection

- `MND-14` **Detail follows what matters** *(Decided)*: Everyone is always an individual, with their own body, family, memories, skills and beliefs.
  People in routine situations may run more cheaply, even as part of their band, but only once an experiment shows this gives the same history, statistically, as full detail.
  Anyone facing something new, risky or important runs in full.
  The rule depends only on the world, never on where you look.
  Follows from `PRN-11`.

- `MND-15` **No population cap** *(Decided)*: How many minds the phone can run at each level of detail is found by measurement (`PLT-04`).

- `MND-25` **Thoughts are structured; words come later** *(Decided)*: What a person thinks is kept as beliefs, intentions, feelings and memories, never as sentences.
  The story view (`PRE-14`) turns them into words through the writer AI (`PRE-37`); the scientist's view shows them raw.
  Follows from `PRN-06`.

### 9.8 Animals

- `MND-16` **Animals** *(Decided)*
  - **What:** Animals have the same kind of mind with fewer abilities.
    They learn fear, routes and habits, so hunting becomes an arms race and taming becomes possible.
  - **What animals lack:** language, deliberate teaching, long plans and abstract concepts.
    Species differ: wolves hunt together, deer are wary grazers.
  - **Detail:** near people, animals are individuals with minds.
    Elsewhere they are populations that carry inherited and learned traits, such as wariness of people (`WLD-12`).

## 10. Culture and society

Culture is everything people pass to each other rather than inherit through their bodies: skills, words, beliefs, customs and art.
None of it is scripted (`PRN-01`, `PRN-07`).
It grows out of minds (see Minds) living together, and it changes, spreads, splits and dies.

### 10.1 Passing things on

- `CUL-01` **Learning from others** *(Decided)*
  - **What:** People imitate (and imperfect copying creates variation), teach (possible because they understand what others know, `MND-23`), and copy whoever succeeds or whatever most people do.
  - **Why:** This is how a people becomes cleverer than any of its members (`MND-17`).
  - **Example:** The best knapper's technique spreads because others copy whoever succeeds.
    Small copying errors make each band's blades slightly different (`CUL-12`).

- `CUL-02` **Knowledge can be lost** *(Decided)*: Knowledge lives in heads and dies with them unless it is passed on.
  Small, isolated groups can lose skills, as may have happened in Tasmania (`MOM-02`).

- `CUL-03` **Memory outside heads** *(Decided)*: Marks, symbols, writing and records can emerge, letting knowledge outlive the people who had it.
  Signs gain meaning the same way words do, by agreement (`CUL-04`): tally marks for counting, pictures that tell, and eventually signs that stand for words.

- `CUL-16` **How things spread** *(Decided)*: Knowledge, words, styles and beliefs spread through contact: shared camps, marriages between bands, trade and conflict.
  Isolation makes groups drift apart.

### 10.2 Language

- `CUL-04` **Language emerges** *(Decided)*: Words are labels a group agrees on, and they spread through use.
  Groups that separate drift into dialects, then separate languages.
  Language makes teaching faster and lets people talk about things that aren't there: plans, the dead, spirits.

- `CUL-17` **Sounds, words and grammar** *(Decided)*
  - **What:** Each language has its own sounds, words and grammar, starting from the few dozen shared words and calls of the starting kit (`BIO-02`).
    Words drift through regular sound changes, so related languages share telltale patterns and form families you can trace.
  - **Example:** After the eastern band crosses the hills, its words drift away from those of the band left behind.
    Centuries later, their words for water, fire and stone still differ in the same regular way, which shows they were once one language.

- `CUL-18` **Names** *(Decided)*: People, places, peoples and things are named in their own languages, often after events, features or traits.
  You see the original name with a translation (`PRE-12`), and later hear it spoken (`SND-03`).

- `CUL-24` **Conversations** *(Decided)*: People tell each other things: warnings, questions, news, teaching and retold stories.
  What they say is held as meaning first (`MND-25`), and their language puts it into words (`CUL-04`).
  Being told something is weighed against one's own experience, by how far the speaker is trusted (`MND-24`).

### 10.3 Belief

- `CUL-05` **Belief from explanation** *(Decided)*
  - **What:** Big unexplained events (death, sickness, storms, your interventions) demand a cause.
    When no physical cause is known, the inborn tendency to suspect a hidden someone (`MND-21`) suggests an unseen being.
    Beliefs that seem to work spread and last.
    They become ritual, gain specialists such as shamans and priests, and in time grow into religions with their own myths, rules and sacred places.
  - **Why:** Religion grows from the same machinery as discovery (`MND-05`), and your own acts become part of what people try to explain (`GOD-06`).
  - **Example:** Your lightning becomes a god (`MOM-03`).

- `CUL-19` **Dreams and the dead** *(Decided)*: Dreams of dead relatives can lead people to believe the dead live on in some form.
  That can shape burials, rites for ancestors and ideas of a soul.

- `CUL-20` **Taboos** *(Decided)*: Beliefs can harden into rules about what not to eat, where not to go and what not to do.
  Some protect people by accident; others cost them dearly.

### 10.4 Society

- `CUL-06` **Institutions form from habit** *(Decided)*: Repeated behaviour hardens into shared, named things that people know, teach and enforce: a norm, a role, a rank, a rite.
  They can change, split and dissolve, and they become the named things the chronicle and overlays talk about, such as "the rite of first fire" or "the elders' council".

- `CUL-07` **Nothing social is scripted** *(Decided)*: Family and marriage rules, sharing, exchange, trade, leadership, alliances, conflict and war all come from people's interactions.

- `CUL-21` **Sharing and exchange** *(Decided)*: Food sharing, gifts, trade between bands, specialists, rules about who owns what and, perhaps one day, money.
  Each emerges from need and repeated habit.

- `CUL-22` **Leadership and status** *(Decided)*: Depending on the culture, status comes from skill, generosity, age, success, fear or birth.
  Leaders, councils and chiefs emerge where a group needs to act together.

- `CUL-08` **Dark history can happen** *(Decided)*: Violence and war, captivity and slavery, sacrifice, cruelty, infanticide and cannibalism can emerge like anything else.
  Sexual acts stay abstract (`BIO-15`).
  What is shown is controlled by the content setting (`PRE-18`).

- `CUL-23` **Peoples** *(Decided)*: The game recognises peoples by what their members share (language, beliefs, customs and style) and names them by what they call themselves.
  Boundaries can be blurry and shift over time.
  Peoples split, merge and disappear.

### 10.5 Expression

- `CUL-25` **Expression is real** *(Decided)*: Each form of expression exists as a real thing in the world: paint on rock, marks on wood and bone, sound in the air, movement in a dance.
  What it holds is kept as content: a song as its notes and rhythm; a picture or map as what it shows and how (composition, style, skill and pigments), from which the game draws it.

- `CUL-09` **Visual art** *(Decided)*: Paintings, carvings and body decoration composed from their own memories and myths, made with real pigments and tools (`RCK-15`, `RCK-16`) on cave walls, objects and bodies.
  What they depict reflects what matters to them (`MOM-07`).

- `CUL-10` **Music and dance** *(Decided)*: Rhythms, scales, songs and instruments that grow out of each culture.
  Instruments follow real acoustics (`MAT-03`), from bone flutes to drums of stretched hide.

- `CUL-11` **Myths and stories** *(Decided)*: Built from the band's own memories, beliefs and dreams, told and retold, and changing a little with each telling.

- `CUL-12` **Style and ornament** *(Decided)*: Each culture's look in tools, clothing and buildings, drifting over time, so objects could be dated by their style.

- `CUL-13` **Their sky and calendar** *(Decided)*: Constellations they name, seasons they track, festivals they keep (`WLD-07`).

- `CUL-14` **Their maps and names** *(Decided)*: Places named in their own languages, and maps drawn the way they see the land.

- `CUL-15` **Remembered lives** *(Decided)*: Genealogies, and legends of remarkable people as their culture remembers them.

## 11. Presentation

### 11.1 Visual style

This is how the world looks.
It is written to stand on its own, without needing any image to understand it.

- `PRE-01` **Detailed pixel art** *(Decided)*: Everything on screen is crisp pixel art: limited colours, hard pixel edges, no blur and no smooth gradients.

- `PRE-02` **Pixel-rendered 3D** *(Decided)*
  - **What:** The world is a real 3D world, drawn at low resolution and enlarged with hard pixel edges.
    It looks like hand-made pixel art but has real depth, scale and structure.
    The camera turns freely and zooms continuously.
  - **Why:** Real 3D shows height, depth, sizes and structures (cliffs, caves, shelters, later buildings) at every zoom.
    The land comes straight from the simulation instead of being hand-drawn, which suits generated worlds.
  - **Example:** At dusk, from an oblique angle, you see a band's camp below a limestone cliff: the cave mouth in shadow, long shadows across the grass, the river beyond.
    You turn the camera and fly down until one person fills the screen.

- `PRE-20` **Colour in steps** *(Decided)*
  - **What:** Every material has a short ladder of shades, about 4–7 colours, drawn from one master palette.
    Ladders are made automatically from each material's simulated colour (`MAT-03`) and matched to the palette; common materials, such as grass, limestone and water, get hand-picked ladders instead.
    Light chooses a step on the ladder.
    Where two steps meet, a fine pixel pattern blends them in a narrow band only; surfaces are never speckled all over.
    The pattern is fixed to the surface, so it doesn't swim when the camera moves.
  - **Why:** Clean colour is what separates pixel art from a shrunken photograph.

- `PRE-21` **Outlines and lit edges** *(Decided)*
  - **What:** A one-pixel dark outline wherever one thing stands in front of another: people, animals, trees, rocks, the top edge of a cliff.
    A one-pixel bright edge where the sun or a fire catches a shape, such as the sunlit rim of a cliff or the fire-facing side of a person.
  - **Why:** Crisp silhouettes keep small things readable on a phone screen.

- `PRE-22` **Stable pixels** *(Decided)*
  - **What:** Pixels never crawl or shimmer as the camera moves: the picture stays locked to its pixel grid, and turns ease to rest.
    One art pixel is always the same size on screen, in portrait and in landscape, so turning the phone only changes the framing.
    About 4 screen pixels make one art pixel.
  - **Why:** Shimmering pixels are the most common flaw of 3D pixel art, and the first thing that makes it look cheap.

- `PRE-23` **Rock faces** *(Decided)*
  - **What:** Cliffs show their geology: horizontal rock layers of different thicknesses, irregular vertical cracks, a few long fissures, lichen, water stains, soot above inhabited caves, grass hanging over the top edge, and scree at the foot.
    The same layers continue underground (`PRE-25`).
  - **Why:** Geology is part of the story (`WLD-14`).
    What people can find depends on what the land is made of, and the rock should show it.

- `PRE-24` **Real shapes** *(Decided)*
  - **What:** Overhangs, caves, rock shelters and, later, buildings have real depth.
  - **Example:** Looking into a cave mouth from an angle, you see its dark interior, the firelit floor and the hide windbreak across the entrance.

- `PRE-25` **Cut-away view** *(Decided)*
  - **What:** The ground can be sliced open to show what lies beneath: rock layers, soils, underground water, and the buried layers of past life (hearths, tools, bones, graves).
  - **Why:** It is how you see geology and dig through history.
    The archaeology view (`PRE-09`) uses it.

- `PRE-26` **Water** *(Decided)*
  - **What:** Rivers meander and change width, with gravel bars, reeds, lines that follow the current, ripples at fords, glints of sun and drifting mist.
    From far away a river never becomes thinner than one or two art pixels, so it stays readable.

- `PRE-27` **People and animals** *(Decided)*
  - **What:** People and animals are small 3D figures drawn through the same pixel look and animated at a deliberate, sprite-like rhythm of about 8–12 poses a second.
    They look like crisp pixel art from any angle and turn properly with the camera.
    At the closest zoom, a person is about 40–60 art pixels tall: enough for a face, hair, clothing and gestures.
  - **Why:** The simulation will produce actions nobody planned (`PRN-01`).
    Figures built from parts can perform any of them from any angle, without a new drawing for each.

- `PRE-28` **Readable from far away** *(Decided)*: As you zoom out, people become tiny outlined figures in strong clothing colours, then groups become small markers, then a camp becomes a glowing point.

- `PRE-29` **From above** *(Decided)*
  - **What:** As the camera rises, it tilts toward looking straight down, and the land shifts into a clean map look: crisp colours for forest, grassland, rock and water, rivers as lines, shaded hills.
    Map overlays (`PRE-07`) sit on this view.
    At the very top, the whole world appears as a globe (`WLD-02`).
    Close up to globe is one continuous zoom (`PRE-03`).
  - **Why:** A landscape seen from high up at an angle turns to mush.
    A map stays clear at every height.

- `PRE-30` **Light, time and season** *(Decided)*
  - **What:** One master palette, with versions for each time of day (dawn, day, dusk, night) and each season.
    The sun casts real shadows, the sky tints everything, and distance adds haze.
    A fire lights its surroundings with a warm, flickering glow that fades with distance, warms the faces of people nearby, and sends up smoke and embers.

- `PRE-03` **Seamless zoom** *(Decided)*: One continuous zoom from the whole world, drawn as a globe, down to one person chipping flint.

- `PRE-04` **Sharp at every zoom** *(Decided)*: The pixel art stays sharp and readable at every zoom level (`PRE-22`, `PRE-28`, `PRE-29`).

- `PRE-31` **Visual review** *(Decided)*
  - **Done when:** at every milestone, screenshots at each zoom level, in both orientations and at every time of day, pass a review for:
    - clean colour, with no speckled surfaces;
    - crisp silhouettes;
    - pixels that stay still while the camera moves;
    - people and animals readable at phone size.

### 11.2 On the screen

- `PRE-32` **World first** *(Decided)*
  - **What:** The world fills the screen.
    Controls and panels appear only when you ask for them: tap a person, animal, group or place to open its card, or swipe up for the chronicle and other views.
    Nothing stays on screen unless you called it up, apart from a live moment appearing briefly (`PRE-08`).
    A brief touch shows the date, the real speed of time and the time control (`PRE-33`).
  - **Why:** The world is the point.
    It should feel like looking at a living place, not at a dashboard.
  - **Example:** You open the app to nothing but the valley at dusk, exactly as you left it.

- `PRE-34` **Both orientations** *(Decided)*: Every screen works one-handed in portrait and two-handed in landscape (`VIS-14`).
  Follows from `PLT-02`.

- `PRE-33` **Gestures** *(Decided)*:
  - drag to move, and twist with two fingers to turn;
  - pinch to zoom, or double-tap and drag with one thumb, which also sets the speed of time (`TIM-01`);
  - tap to select;
  - long-press for your powers at that spot (`GOD-10`), including drawing an area, so a drag always moves the camera;
  - swipe up from the bottom edge for views;
  - a brief touch anywhere shows the date, the real speed of time and the time control: pause, speed and speed lock (`TIM-04`).

- `PRE-35` **Cards** *(Decided)*: Selecting anything opens a card with what matters about it, such as a person's name, age, mood, and what they are doing and why, or a place's land and history.
  Links lead into deeper views: the story view, the scientist's view, family trees, archaeology.

- `PRE-40` **Screens** *(Decided)*: Besides the world itself: a first-launch screen, a list of your worlds, and settings.
  Short help cards appear the first time you use something; there is no tutorial (`SCP-02`).

### 11.3 Following the story

- `PRE-05` **Chronicle** *(Decided)*
  - **What:** An automatically written history of the world, organised as a book of ages, with a timeline for each people.
    Eras are named by their own people, or after the events that defined them.
    Every entry links to the moments and people behind it (`VIS-15`).
  - **Why:** It is the main way to read a world's history, and the measure of "histories worth reading".

- `PRE-06` **Follow a soul** *(Decided)*: Pick anyone and follow their life: their card shows what they feel, want and think (`PRE-14`), and the camera can stay with them.
  The people you follow are kept in a list, separate from the camera, so you can follow several and still look elsewhere.
  When they die, the game offers to follow someone close to them.

- `PRE-07` **Map overlays** *(Decided)*: Information shown spread across the land.
  The overlays: beliefs; knowledge, meaning who knows which skill; moods; languages and dialects; family ties; territories and paths; food and water; disease; climate and seasons; rock and resources.

- `PRE-08` **Live moments** *(Decided)*: Only what matters interrupts you: firsts, deaths of people you follow, disasters, and big turns in history ("someone has made fire for the first time").
  What can interrupt is one shared list, also used by the story director (`TIM-02`).
  At most about one interruption comes a minute, and the closer you are watching, the more important something must be to interrupt.
  Live moments you don't take wait in a list; everything else waits in the chronicle.
  The level can be adjusted in settings, and one tap takes you to the moment.

- `PRE-39` **Recognising what emerges** *(Decided)*
  - **What:** The game spots and names what emerges, for you only: firsts and discoveries, skills, languages, institutions, peoples and eras.
    It uses both general detection of anything new and a catalogue of notable outcomes, such as fire made by friction.
  - **Rules:** recognisers sit on the describing side.
    They never feed back into the world, and are kept provably apart from the logic that decides what people and animals do (`PRN-07`).
    Their thresholds, such as when a dialect becomes a language, are set in the implementation plan and listed in milestone reports.
    A first counts both worldwide and for each people, and a rediscovery after a loss is marked as one.
  - **Why:** The director (`TIM-02`), live moments (`PRE-08`), the chronicle (`PRE-05`), overlays (`PRE-07`), timeline comparisons (`TIM-13`) and experiment measures (`RES-03`) all need to know what happened, without the simulation ever naming it.

- `PRE-09` **Archaeology** *(Decided)*
  - **What:** Dig down through the buried layers of past life with the cut-away view (`PRE-25`): hearths, graves, lost tools, rubbish heaps.
    Tap a find to see who made or left it, and when.
  - **Example:** The dig in `MOM-09`.

- `PRE-10` **Family trees and legends** *(Decided)*: Genealogies across generations, and the legends their culture keeps.
  Each legend can be set side by side with what really happened.

- `PRE-11` **Their sky and calendar (view)** *(Decided)*: The sky as they understand it: their constellations, the seasons they track, their festivals.

- `PRE-12` **Their maps and names (view)** *(Decided)*: Their place names with translation, and the maps they draw, compared with the real land.

- `PRE-14` **Two views of every mind** *(Decided)*: A story view in their own words, and a scientist's view of their raw beliefs, how certain they are, and the evidence behind each belief.
  Other views, such as the chronicle, the map overlays and archaeology, also have a story version and a scientist's version, switched separately in each view.
  Story versions never show your interventions (`GOD-07`).

- `PRE-15` **Art that remembers** *(Decided)*: Tap a painting or carving to see it, what its maker meant, and the event or myth it depicts.
  If that event was saved as a key moment, you can watch it as it really happened (`PRN-15`).

- `PRE-16` **Bestiary** *(Decided)*: Each world's tree of life and its species.

- `PRE-36` **Language family tree** *(Decided)*: How their languages split and drifted over time (`CUL-17`).

- `PRE-13` **Every view the simulation allows** *(Decided)*: Any further view the simulation's data supports, within physical limits (`PRN-04`).

### 11.4 Text written for you

- `PRE-17` **Descriptions stick to the data** *(Decided)*: The AI language model only turns simulation data into text: life stories, myths, dreams, the chronicle.
  It never adds facts the simulation doesn't contain (`PRN-06`).

- `PRE-37` **The writer AI runs on the phone** *(Decided)*: All text is written on the phone, fully offline, with no running cost.
  If the writing turns out too plain for histories worth reading (`VIS-15`), that is raised at a milestone review.

- `PRE-41` **How text is written** *(Decided)*: Text is written when it is first opened or during pauses, checked against the data it came from, stored, and never silently rewritten.
  You can ask for a rewrite.
  Text that fails its check is replaced by plain factual text.
  What the chronicle covers, and where its ages begin, come from fixed rules (`PRE-39`), not from the writer's taste.
  The writer chooses words and rhythm, never content: every claim, cause, motive, image and name must be in the data (`PRE-17`).

- `PRE-38` **English** *(Decided)*: The interface, the chronicle and translations are in English.
  Their own words appear in their own languages, with English translations (`PRE-12`).
  Until their own names emerge, people, places and peoples get labels made from that world's own sounds, marked as the game's (`CUL-18`).
  English text describes things through their concepts, such as "cutting stone" (`MND-04`); our own words for them, such as "flint", appear only in the scientist's view.

- `PRE-19` **Storytelling voices** *(To test)*: Documentary, archaeologist, their own tradition, and intimate.
  Each is tried live on real simulation output and chosen by ear.
  Different views may use different voices.

### 11.5 Content

- `PRE-18` **Content setting** *(Decided)*: You choose how much of history's darker side is shown, at one of three levels:
  - **Show:** everything, with pictures and sounds;
  - **Plain:** no graphic pictures or sounds, and factual text;
  - **Gentle:** dark events mentioned briefly, in the chronicle only.

  The simulation underneath never changes (`CUL-08`), and bodies are drawn without sexual detail at every level.

## 12. Sound

Sound comes in layers, added over time, starting with the living soundscape.
Like everything you see, everything you hear reflects what is actually happening (`PRN-10`).

### 12.1 The layers

- `SND-01` **Living soundscape** *(Decided)*
  - **What:** Wind, rain, rivers, animals, fire and people at work, driven by what is actually happening where you're looking.
    Zoom changes the mix: close up you hear single sounds; further out they blend; from the whole world, near silence.
  - **Example:** At the camp at dusk: the crackle of the fire, the tap of the knapper's hammerstone, a child laughing, the river beyond, a wolf far off.

- `SND-03` **Their voices** *(Decided)*: Zoomed in, you hear real speech: actual sentences in their language, spoken with its own sounds and grammar (`CUL-17`), with English subtitles if you want them.
  Further out, talk blends into a murmur.

- `SND-02` **Their music** *(Decided)*: Songs, rhythms and instruments from each culture (`CUL-10`), heard when you are near.
  Their scales and rhythms develop and drift, as their languages do.

- `SND-04` **Score** *(Decided)*: Background music generated live from the world.
  It is assembled from short themes that respond to time of day, season, events and the people nearby, and it draws on their own scales and rhythms as their music develops.
  It is never the same twice.

- `SND-05` **Order of the layers** *(Decided)*: The soundscape comes first, then their voices, then their music and the score.
  The implementation plan sets when each arrives.

### 12.2 How sound is made

- `SND-06` **Sounds from the physics** *(Decided)*
  - **What:** Impacts, fire, water and instruments are created from what things are made of (`MAT-03`).
    A strike on flint sounds unlike one on granite, and an instrument they invent sounds the way its materials would.
    Background wind and rain can use recordings, and so can birdsong, but only where matching birds are simulated.
  - **Why:** General rules (`PRN-07`) apply to sound as well.
    Nobody has to record the sound of an instrument nobody planned.

- `SND-07` **Sound follows time** *(Decided)*: At natural speed (`TIM-10`), every sound plays in real time.
  When time runs fast, single sounds give way to the feel of the period: seasons of wind and rain, the hum of a busy camp.

- `SND-08` **Space and distance** *(Decided)*: Sounds come from where they happen and fade and muffle with distance; caves echo.
  A sound can draw your attention to something off-screen, such as a scream or thunder.

- `SND-09` **Silence** *(Decided)*: Quiet is part of the design.
  Nights are hushed, deep snow muffles everything, and the whole world seen from above is close to silent.

### 12.3 Touch

- `SND-10` **Vibration for big moments** *(Decided)*: Subtle and optional: thunder, an earthquake, the heartbeat of someone you follow when they are in danger.

## 13. Platform and performance

Kindling is built for one phone, and nothing else is used to play it (`SCP-02`).
The phone must stay smooth, cool and responsive (`VIS-14`, `PRN-11`).
How much simulation fits on it is found by measuring, not guessing.

### 13.1 The phone

- `PLT-01` **One phone** *(Decided)*: Built and optimised for your Pixel 11 Pro XL (16 GB of memory and 512 GB of storage, so the app can use about 10 GiB), and free to use that phone's specific hardware wherever it helps: its graphics chip for the pixel-rendered 3D (`PRE-02`), its AI hardware for the writer AI (`PRE-37`), and either of them for the simulation itself (`PLT-05`).

- `PLT-02` **Portrait and landscape** *(Decided)*: Both are supported, and the layout adapts (`PRE-34`).

- `PLT-03` **Works offline** *(Decided)*: Everything works without a connection, including the text descriptions, which are written on the phone (`PRE-37`).

- `PLT-06` **Installing new versions** *(Decided)*: Each new version is a file you download on the phone and install, after allowing installs from your browser once.
  Builds are signed for your free hobbyist developer account with Google, so they keep installing this way under Android's developer rules from 2027 (`RSK-18`).
  No store and no fees.
  Each milestone report links to its version.

### 13.2 Performance

- `PLT-04` **Measured limits** *(To test)*: Measured from the first build and reported at every milestone:
  - smoothness of zooming and panning;
  - simulated time per real minute at each zoom level;
  - how many people the phone can run at each level of detail;
  - battery use and heat per hour of play;
  - time to generate a world.

  The targets, from `VIS-14`: the camera stays smooth at the screen's full refresh rate; the app opens to your world in about three seconds; an hour's session uses about 25–30% of the battery, without the phone getting uncomfortably hot.

### 13.3 Worlds on the phone

- `PLT-07` **Always saved** *(Decided)*: Worlds save continuously, so closing the app or a flat battery never loses anything (`TIM-05`).

- `PLT-08` **Manual export** *(Decided)*: Export a world, with all its timelines, as a file whenever you want, and import it again on the same phone or a new one.
  The export keeps the full record, so an imported world opens exactly as it was.
  There are no automatic backups.

- `PLT-09` **Worlds across updates** *(Decided)*
  - **What:** The game's rules will keep growing (`PRN-14`).
    After a small update, a world carries on: everything that already happened stays as it was, the world continues under the new rules, and the change is marked on its timeline.
    A big update, one that adds a new layer of the world such as new matter, species or systems, may need a new world.
    Worlds are only promised to last between big updates.
  - **Why:** Fitting a new layer into a running world would be costly, and could make its past dishonest.
    Starting a new world keeps every world true to one set of rules.
  - Going back to a saved moment from before a small update and branching runs the new branch under the current rules.

- `PLT-10` **Storage** *(Decided)*: Saved moments are kept densely near the present and thinned with age by a fixed rule; key moments are always kept (`PRN-15`).
  The event history thins with age in the same way: recent years keep every event, and older history keeps what the chronicle, the views and the key moments use, such as births, deaths and firsts.
  You can delete worlds and branches.
  When the phone nears full, the game warns you and asks what to delete; it never deletes anything else by itself.

### 13.4 The cloud

- `PLT-05` **Experiments in the cloud** *(Decided)*: The simulation also runs without graphics in the AI's cloud sessions, many runs at a time.
  The phone build comes first and is optimised for the phone; the cloud build doesn't have to match it exactly, only behave the same statistically (`RES-05`).
  An experiment's world can be opened on the phone at any of its saved moments.
  Follows from `SCP-15`.

## 14. Research and validation

This section turns "research standard" into practice: how the project proves that its world really does what it claims (`PRN-05`).

### 14.1 How experiments work

- `RES-01` **Experiments lead** *(Decided)*
  - **What:** Core ideas are proven in experiments, run without graphics, before the game builds on them (`SCP-03`): first in sandboxes, then confirmed in full worlds (`RES-21`).
  - **Why:** The biggest risk is that nothing emerges (`RSK-01`).
    Experiments find out early and cheaply.

- `RES-21` **Sandboxes, then full worlds** *(Decided)*
  - **What:** Most experiments run in sandboxes: small settings built for one question, such as a band on a riverbank with flint, granite and decoy stones, or a winter camp whose fire is dying.
    A sandbox uses the game's own rules and minds, with no special rules and nothing scripted inside it (`PRN-07`); only its setting is chosen, and it includes decoys and materials nobody designed for.
    Each experiment states its computing budget up front, and its sandbox is sized to fit it (`RES-16`).
    Sandbox runs are cheap and repeat exactly from their seed (`TIM-06`), so each question gets many runs.
    At every milestone, a few full worlds from the play generator confirm that what the sandboxes showed also happens in a real world, within the sandboxes' ranges.
  - **Why:** Whole worlds are far too costly to run by the hundred (`RSK-14`), while a sandbox answers one question cheaply and repeatably.
    The full worlds guard against a sandbox so well arranged that it makes the result likely by design.
  - **Check:** every claim names its sandbox and its full-world confirmation, and a result seen only in sandboxes is reported as such.

- `RES-08` **What every experiment has** *(Decided)*: A question; a setup (the sandbox or world settings, starting kit, population, length); the number of runs; what is measured; pass and fail criteria; and comparison runs.

- `RES-09` **Criteria fixed first** *(Decided)*: Each experiment's pass and fail criteria, with exact numbers and definitions, are written down before it runs, checked by the independent reviewer (`RES-11`) for ways the experiment couldn't fail, and approved by you.
  They are never adjusted afterwards.

- `RES-10` **Comparison runs** *(Decided)*: Each experiment also runs with one mechanism switched off, such as imitation, to show that what emerged depends on it.

- `RES-11` **Independent review** *(Decided)*: A separate AI agent, not the one that built the experiment, checks it and tries to find flaws in the results.

- `RES-12` **Surprises log** *(Decided)*: Unexpected results are recorded even when they weren't the question.
  They often become new signature moments (`MOM`).

- `RES-13` **Many runs, reported as ranges** *(Decided)*: Every claim rests on many runs (100 sandbox runs per setup unless stated), confirmed in a few full worlds (`RES-21`), and is reported as a range, for example "discovered in 62 of 100 runs; typically around year 140".

- `RES-14` **Compared with reality where possible** *(Decided)*: Where real-world data exist, such as hunter-gatherer populations or rates of learning and cultural change, results are compared with them.

- `RES-16` **Tuning and failure** *(Decided)*
  - **What:** Values are tuned on development seeds, then confirmed once on fresh seeds kept back for that.
    Every attempt and every tuned value is logged, with what it was tuned against.
    A test may stop early once its result is clear, within a stated computing budget.
  - **When it fails:** a failed confirmation holds the milestone until you choose: redesign, a weaker claim, or dropping the claim.
  - **Why:** Re-running on the same worlds until something passes would make "experiments that can fail" meaningless (`PRN-05`).

- `RES-17` **Signature moments keep passing** *(Decided)*: Each signature moment has its own sandbox, and passes if it happens in at least 1 run in 10 within its time window, unless its own criteria say otherwise (`RES-21`).
  A small sample of these sandboxes re-runs before every merge and all of them at every milestone, and a failure blocks the milestone (`PRC-10`).
  Each milestone report also says which moments appeared in its full worlds.

- `RES-18` **Same rules as play** *(Decided)*: Sandboxes use the same rules as play, and the full worlds that confirm them come from the play generator (`WLD-10`, `RES-21`).
  Scripted events and dials appear only in clearly labelled experiments, and a moment that passes only with a dial doesn't count as passing in play (`PRN-12`).

- `RES-19` **Promises are tested** *(Decided)*: Every claim in Minds and in Culture and society that something emerges either gets an experiment before its milestone closes, or is marked "possible, not promised".

- `RES-20` **Your own experiments** *(Decided)*: You can ask for an experiment in any cloud session (`SCP-15`).
  An experiment's world can be opened on the phone (`PLT-05`) and played on; it keeps its dial settings for good and always shows them, so it is never mistaken for a play world (`PRN-12`).

- `RES-04` **Reality checklist first** *(Decided)*: The physics must pass every reality check (`RCK`) before any discovery that depends on it is trusted.

- `RES-05` **Reproducibility** *(Decided)*: Results are reproducible statistically: re-running an experiment on fresh seeds gives results within its stated ranges, and the cloud build gives the same statistics as the phone build.
  Exact repeats of a history are not required (`PRN-15`).
  Checked at every milestone.

### 14.2 The experiments

- `RES-02` **Experiment 1: sharp stone** *(Decided)*: Do bands that only bash rocks discover how to chip sharp flakes, and does the skill spread?
  Its sandbox includes uses for a sharp edge: carcasses to butcher, and hides and wood to work.

- `RES-03` **Experiment 1 pass criteria** *(Decided)*: starting values, fixed before it runs (`RES-09`), met in its sandbox runs and confirmed in full worlds (`RES-21`):
  - **Discovery:** happens in at least half of the runs, within 500 simulated years.
  - **Variety:** discovery times differ widely between worlds, and at least two different routes to the discovery appear, for example an accident someone notices versus deliberate tinkering.
  - **Spread:** once discovered, at least three quarters of the adults in the discovering band can do it within 50 simulated years.
  - **Loss:** the skill is lost noticeably more often in small, isolated groups than in large, connected ones.
  - **General rules only:** the check in `PRN-07` passes.
  - **Comparison runs:** without imitation, the skill does not spread; without curiosity, discovery is much rarer (`RES-10`).
  - **Exact numbers:** words such as "widely", "noticeably" and "much rarer", and what counts as a discovery and as being able to do it, are given exact values before the run (`RES-09`).

- `RES-07` **The series** *(Decided)*: Experiments follow the signature moments in milestone order, adjusted after each report.
  The implementation plan sets which experiment closes which milestone.
  The order:
  1. the phone and cloud statistical match, and performance baselines;
  2. Experiment 1, sharp stone;
  3. fire from wood (`MOM-01`), with and without a dream, to show a dream raises the odds without guaranteeing anything;
  4. the lost craft (`MOM-02`);
  5. the song that does nothing (`MOM-04`), and your lightning becomes a god (`MOM-03`);
  6. two tongues (`MOM-05`), once seas and the whole world exist, and rivals, then in-laws (`MOM-11`);
  7. the camp wolf (`MOM-06`), seeds on the rubbish heap (`MOM-08`), and metal from green stone (`MOM-12`).

  The remaining signature moments (a painting that remembers, the dig, and two endings) are features, checked at milestone reviews rather than run as experiments.

### 14.3 Reports

- `RES-06` **Milestone reports** *(Decided)*: Every milestone ends with a report for you, covering:
  - what was tested and the results, with charts;
  - the comparison runs;
  - what emerged, and the surprises;
  - the measurements (`PLT-04`);
  - what was added (`PRN-14`) and how the principles were checked;
  - the risks (see Risks);
  - links to saved moments in the game.

- `RES-15` **A page on the phone** *(Decided)*: Each report is a readable page with charts and plain conclusions, whose links open saved moments in the game.
  A copy is kept in the repository.

## 15. Project and process

How the project is run: you direct, and AI agents build.
This section defines the roles, the documents, how this file changes, and how work flows from an idea to your phone.

### 15.1 Roles

- `PRC-01` **Passion project, built by AI** *(Decided)*: You direct; AI agents write, test and review the code.
  There are no running costs beyond the AI sessions themselves, since the writer AI runs on the phone and there is no store.

- `PRC-02` **Your role** *(Decided)*: You read the milestone reports, try the builds, set direction, and approve changes to this file.
  The AI handles code review and testing.

- `PRC-03` **Technology** *(Decided)*: Chosen by the AI and proposed in the architecture for your approval.

### 15.2 Documents

- `PRC-04` **Three documents** *(Decided)*: The finished project has three documents.
  This file is the source of truth for what to build; the architecture says how it is built; the implementation plan says in what order, mapping every item to a milestone and its tasks.
  Code and tests link back here by ID.

- `PRC-06` **A guide for AI agents** *(Decided)*: A short file in the repository (`CLAUDE.md`) that every AI agent reads first.
  It tells them to read this file, follow the principles, link all work to IDs, and never mark anything Decided without you.

- `PRC-07` **Changes to this file** *(Decided)*: AI agents can suggest additions or changes, marked *Proposed*.
  Nothing becomes *Decided*, and no decided item changes, without your OK.
  How changes are proposed and recorded is set out in How this file works.

- `PRC-05` **Reviewed with you** *(Decided)*: Changes to this file are worked through with you, section by section or in rounds of questions, and *Proposed* items are confirmed, changed or dropped in those reviews (`PRC-07`).

- `PRC-08` **Next: tests, then the architecture and the plan** *(Decided)*: Before the architecture and the implementation plan are written, small throwaway tests settle the basic technical choices, such as the language, storing data, the map, drawing, sound, speech and the writer AI.
  Each block is tested on its own, with no working world; anything that needs a world is designed in the architecture and tested in sandboxes (`RES-21`).
  The architecture follows, starting with the technology proposal (`PRC-03`), then the implementation plan, starting with the first milestone (`MIL-01`).

### 15.3 How work flows

- `PRC-09` **Branches, checks and review** *(Decided)*: AI agents work on separate branches.
  Work joins the main version only after every automatic check passes and an independent AI review approves it.
  You review at milestones.

- `PRC-10` **The checks** *(Decided)*:
  - **before any work joins the main version:** the tests, the reality checklist (`RES-04`), the general-rules check (`PRN-07`), a small sample of the signature-moment tests (`RES-17`), and the file check: every ID defined once, every reference resolving, every status valid, and no live item pointing to a dropped one;
  - **before a milestone closes:** the visual review (`PRE-31`), the measurements (`PLT-04`), the phone and cloud statistical match (`RES-05`), the full signature-moment tests (`RES-17`), the coverage check (`PRC-12`), the independent review of experiments (`RES-11`) and the report (`RES-06`).

- `PRC-11` **Builds between milestones** *(Decided)*: A new version whenever something you can see or try has changed, with a one-line note, installed by download (`PLT-06`).
  The full report still comes at each milestone.

- `PRC-12` **Nothing gets lost** *(Decided)*: An automatic coverage check, run at every milestone gate (`PRC-10`), confirms that every feature and rule that isn't *Dropped* or *Proposed* is mapped to a milestone in the implementation plan, that the current milestone's items have tasks, that every task names the IDs it delivers, and that every ID named in code and tests exists and isn't dropped.

## 16. Risks

What could stop Kindling from succeeding, how we would notice early, and what we do about it.
Each risk has a rating (likelihood and impact), the early signs to watch for, and a response.
Every milestone report reviews them all (`RES-06`), and AI agents may update the ratings there.

### 16.1 The core idea

- `RSK-01` **Nothing emerges** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** The minds and physics might not produce discoveries often enough.
  - **Signs:** Experiment 1 discovery rates far below its criteria; discoveries by only one route; skills that never spread.
  - **Response:** Experiment 1 tests this early and cheaply (`RES-02`), with comparison runs showing which mechanism is missing (`RES-10`).

- `RSK-07` **Our own knowledge leaks in** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** Real-world knowledge slips in through the writer AI or through design shortcuts, so discoveries stop being theirs.
  - **Signs:** the general-rules check finds discovery words in decision logic; discoveries happening suspiciously fast; descriptions containing facts the simulation doesn't.
  - **Response:** `PRN-06`, `PRN-07` and `MND-02`, enforced by the checks (`PRC-10`) and independent review (`RES-11`).

- `RSK-06` **The chemistry gives absurd results** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** General laws combine in ways that produce nonsense.
  - **Signs:** reality checks failing; odd outcomes in the surprises log, such as things burning that shouldn't.
  - **Response:** the reality checklist (`RCK`), run in full after every addition (`MAT-15`).

- `RSK-13` **Simplified minds behave differently** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** People run more cheaply in routine situations (`MND-14`) may discover, spread or lose things differently from full minds.
  - **Signs:** the comparison experiment shows different statistics for simplified and full minds.
  - **Response:** no simplified form is used until it matches (`PRN-11`), and the comparison is repeated at every milestone.

- `RSK-19` **Language or belief fails to emerge** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** Grammar, rich beliefs or rituals may not emerge from general mechanisms (`CUL-17`, `CUL-05`).
  - **Signs:** small language tests and later experiments failing.
  - **Response:** test early with small models; if needed, restate the promise with you, for example as "words and simple word order".

### 16.2 The experience

- `RSK-03` **Real but dull to watch** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** Simulated worlds often hide their best stories.
  - **Signs:** in milestone reviews, you skim the chronicle; few live moments; long stretches of years where nothing seems to happen.
  - **Response:** the story director, live moments and the two views of each mind bring the stories out (`TIM-02`, `PRE-08`, `PRE-14`), and every report judges whether they do.

- `RSK-08` **Writing too plain** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** The writer AI on the phone (`PRE-37`) produces flat or repetitive text, undermining histories worth reading (`VIS-15`).
  - **Signs:** chronicle entries that read alike; storytelling voices you can't tell apart.
  - **Response:** voices tested live (`PRE-19`); rich, structured simulation data for the writer to draw on; if it still falls short, it is raised at a milestone review with options.

- `RSK-10` **History too slow to watch** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Even with fast time and overnight mode, a deep simulation may take too long to reach interesting points.
  - **Signs:** overnight runs covering only a few years; quiet centuries dominating the chronicle.
  - **Response:** measure from the start (`TIM-07`); less detail for what is routine (`WLD-12`, `MND-14`); overnight mode (`TIM-12`); the story director skipping quiet years (`TIM-02`).

- `RSK-11` **Pixel look hard to keep clean** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Keeping pixel-rendered 3D free of speckle and shimmer at every zoom may be harder than it looks.
  - **Signs:** visual reviews failing on speckled surfaces, crawling pixels or unreadable figures.
  - **Response:** the style is defined in words (see Visual style) and checked at every milestone (`PRE-31`).

- `RSK-17` **The writer AI softens dark history** *(Decided)*
  - **Rating:** likelihood medium, impact low.
  - **Risk:** A safety-tuned model may refuse or soften violence, slavery or sacrifice (`CUL-08`), breaking `PRE-17`.
  - **Signs:** vague or missing chronicle entries for dark events.
  - **Response:** test it early; when it refuses, plain factual text is shown instead.

### 16.3 The phone

- `RSK-02` **The phone can't keep up** *(Decided)*
  - **Rating:** likelihood high, impact medium.
  - **Risk:** Deep minds, chemistry and detailed pixel art add up.
  - **Signs:** dropped frames; simulated time per minute falling as the population grows; the phone getting hot.
  - **Response:** measure from the first week (`PLT-04`); less detail for what is routine (`WLD-12`, `MND-14`); time slows rather than the simulation cutting corners (`PRN-11`).

- `RSK-04` **Cloud experiments drift from the phone** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** The phone build is optimised on its own (`PLT-05`), so cloud experiments may stop showing what actually happens on the phone.
  - **Signs:** the milestone comparison finds different statistics on the phone and in the cloud.
  - **Response:** one set of rules for both builds, and the statistical comparison at every milestone (`RES-05`).

- `RSK-12` **Updates change worlds in odd ways** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** A world that continues under new rules (`PLT-09`) may change suddenly at the point of the update.
  - **Signs:** sudden jumps in a world's state just after an update.
  - **Response:** history before the update is kept and the change is marked (`PLT-09`); the reality checklist runs before every release; branching lets you compare.

- `RSK-15` **The memory limit** *(Decided)*
  - **Rating:** likelihood high, impact medium.
  - **Risk:** About 10 GiB must hold the simulation, the picture and the writer AI (`PLT-01`).
  - **Signs:** the system slowing or closing the app; fewer detailed people than planned.
  - **Response:** measure memory per person early, and keep history in storage rather than in memory (`PRN-15`).

- `RSK-20` **Saved worlds grow too large** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Long histories with saved moments and key moments may fill the phone (`PRN-15`).
  - **Signs:** worlds growing by gigabytes every thousand years.
  - **Response:** measure early; thin saved moments and old events with age by fixed rules, and ask before deleting anything (`PLT-10`).

- `RSK-21` **Losing a world to a bad update** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** A bug in an update, or a damaged save, could make a world unreadable, and there are no automatic backups (`PLT-08`).
  - **Signs:** worlds failing to open after an update.
  - **Response:** a safety copy before any update touches a world, and tests that open old saves.

- `RSK-22` **The writer AI too slow or too large** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** The model may be too slow, too big or too hot to run beside the simulation (`PRE-37`).
  - **Signs:** entries taking a minute to appear; memory pressure.
  - **Response:** test it early on the phone; write text when it is first needed, and keep it.

- `RSK-18` **New install rules** *(Decided)*
  - **Rating:** likelihood medium, impact low.
  - **Risk:** From 2027, certified Android phones require apps from registered developers, which affects installing by download (`PLT-06`).
  - **Signs:** installs blocked or warned against.
  - **Response:** builds are signed for your free hobbyist developer account (`PLT-06`); the one-off advanced unlock and a USB cable remain as fallbacks.

- `RSK-24` **The phone ages or is replaced** *(Decided)*
  - **Rating:** likelihood low, impact medium.
  - **Risk:** The project is built for one phone (`PLT-01`), which will age, break or be replaced.
  - **Signs:** battery wear; a new phone.
  - **Response:** worlds move by export (`PLT-08`), and moving to a new model is planned with you.

### 16.4 The project

- `RSK-05` **The scope never ends** *(Decided)*
  - **Rating:** likelihood high, impact medium.
  - **Risk:** "No ceiling" plus "everything deep" never finishes.
  - **Signs:** milestones slipping again and again; a growing pile of proposed items.
  - **Response:** build only as deep as the next experiment needs (`PRN-09`), in milestones (`MIL`).

- `RSK-09` **AI-built code drifts** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** A large codebase built by many AI sessions slowly drifts from what this file says.
  - **Signs:** gaps in the coverage check; reviews finding behaviour that contradicts this file.
  - **Response:** the guide for AI agents (`PRC-06`), IDs and the coverage check (`PRC-12`), independent review (`PRC-09`) and modular design (`PRN-14`).

- `RSK-14` **Experiments too big for the cloud** *(Decided)*
  - **Rating:** likelihood high, impact high.
  - **Risk:** Experiment 1 alone may need months of a cloud session's computing (`SCP-15`).
  - **Signs:** runs that can't finish within a session; experiments cut short.
  - **Response:** sandboxes instead of whole worlds, each sized to a computing budget stated up front (`RES-21`); stop each test once its result is clear (`RES-16`); leaner minds; and raise more computing with you first (`SCP-15`).

- `RSK-16` **Invented sources** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** AI agents may cite sources that don't exist, or don't say what is claimed (`PRN-05`).
  - **Signs:** values whose quoted passage can't be found in the source.
  - **Response:** only key values are sourced (`PRN-05`); each is checked once, when it is added, against the fetched source, with the supporting passage copied by a tool, never typed; the value is then locked, keeping only the source's name, link and quote, and the fetched copy is deleted.

- `RSK-23` **Your time** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Reviews, phone tests and judgements of look and sound all need you, so the project moves only as fast as your time allows.
  - **Signs:** milestones waiting on reviews.
  - **Response:** phone tests bundled into one app per round; short, clear reports; you are asked only what needs you.

## 17. Not yet decided

### 17.1 Open, or settled by measurement

<!-- generated: open items -->
- **Pacing** (`TIM-07`): how long history takes to watch; measured during development.
- **How many people the world can feed** (`WLD-04`): measured in experiments.
- **Storytelling voices** (`PRE-19`): tried live and chosen by ear.
- **The phone's limits** (`PLT-04`): measured from the first build.
<!-- end generated -->

### 17.2 Proposals awaiting confirmation

New suggestions from AI agents are marked *Proposed* and listed here until you confirm, change or drop them (`PRC-07`).

<!-- generated: proposals -->
- None at the moment.
<!-- end generated -->

## 18. Glossary

- **Art pixel:** one pixel of the low-resolution picture, enlarged on screen (`PRE-22`).
- **Band:** a small group of people, usually family, who live and move together.
- **Belief:** something a person holds true, with more or less certainty: a cause and effect, that something exists, what others think, a rule, or a plain fact (`MND-27`).
- **Catalogue:** one of the four lists that describe matter: ingredients, structures, laws and reality checks (`MAT-13`).
- **Comparison run:** an experiment run again with one mechanism switched off, to show what depends on it (`RES-10`).
- **Concept:** a category a person forms from what they perceive, such as "cutting stone" (`MND-04`).
- **Cut-away view:** the ground sliced open to show rock layers and buried traces of past life (`PRE-25`).
- **General-rules check:** confirms that no rule is written for one particular discovery, material, species or event, and that no discovery's name appears in decision-making logic (`PRN-07`).
- **Ingredient:** a real mineral, compound or substance of living things that matter is made of (`MAT-01`).
- **Institution:** a shared, named pattern of behaviour (a norm, role, rank or rite) that people know, teach and enforce (`CUL-06`).
- **Intervention:** anything you do with your powers (see The player as god).
- **Law:** a general rule of change, such as burning or smelting, that never names a product (`MAT-04`).
- **Level of detail:** how finely something is being simulated at a given moment, set by the world's own rule and never by where you look (`WLD-12`, `MND-14`).
- **Live moment:** a notable event the game shows you as it happens (`PRE-08`).
- **Milestone:** a stage of the project that ends with a report you review (`MIL`).
- **Overnight mode:** the world running at top speed, screen dimmed, while the phone charges (`TIM-12`).
- **People (a people):** a group recognised by its shared language, beliefs, customs and style (`CUL-23`).
- **Reality checklist:** real-world changes the physics must reproduce without special rules (`RCK`).
- **Recogniser:** the part of the game that spots and names what emerges, for you only.
  It never feeds back into the world (`PRE-39`).
- **Run:** one simulation of a sandbox or a world for an experiment.
- **Sandbox:** a small setting built for one research question, using the game's own rules and minds, with nothing scripted (`RES-21`).
- **Saved moment:** a point in a world's past whose full state was saved, so you can look at it or branch from it (`PRN-15`, `TIM-06`).
- **Scientist's view / story view:** the two ways to look into a mind: raw beliefs and evidence, or their own words (`PRE-14`).
- **Seed:** the number a world is generated from.
  It decides the world, not its history (`PRN-15`).
- **Signature moment:** a story the simulation must be able to produce without it being scripted (`MOM`).
- **Skill:** a learned way of doing something, which improves with practice (`MND-06`).
- **Story director:** sets the speed of time according to what is happening.
  It never causes events (`TIM-02`, `TIM-03`).
- **Structure:** how matter is put together: crystal or glass, fibrous, porous or dense, grain, wetness (`MAT-02`).
- **Timeline / branch:** one history of a world.
  Going back to a saved moment and carrying on starts a new branch (`TIM-06`).
- **World:** one generated planet (see World).
- **Writer AI:** the AI language model, running on the phone, that turns simulation data into readable text.
  It never decides anything (`PRE-17`, `PRE-37`).
