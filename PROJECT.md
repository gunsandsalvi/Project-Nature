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
  Everything they ever achieve, from a sharp flake of stone to rituals, languages and farms, has to come from what they discover in the world and pass on to each other.
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
  - **Example:** Night after night, you zoom out from one campfire to the whole world and watch migrations, languages and beliefs move across the land like weather.

- `VIS-08` **Curiosity** *(Decided)*
  - **What:** The urge to understand why something happened, and to try "what if".
  - **Why:** Every event has real causes, and the game lets you find them: the scientist's view of a mind (`PRE-14`), the buried layers of a site (`PRE-09`), and the chronicle with the events behind it (`PRE-05`).
  - **Example:** A band is about to abandon its cave.
    You look into their minds and find a run of failed hunts and a belief that the cave turned against them after a death.
    You send a good hunting season, and see whether they stay.

- `VIS-09` **Other feelings** *(Decided)*: Attachment to particular people, and the harshness of nature, will arise from the simulation and are welcome, but the design isn't built around them.
  When design choices conflict, wonder and curiosity decide.

### 1.3 How you play

- `VIS-10` **Two rhythms of play** *(Decided)*
  - **Short check-ins (5–15 minutes):** open the app, catch up on the latest live moments, follow someone for a while, nudge, close.
  - **Long sessions (an hour or more):** watch an era unfold at speed, read the chronicle, dig through the past, and try a "what if" with your powers to see what follows.
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
  > You leave the world running overnight on the charger.
  > By morning, on the knowledge overlay, fire-making has spread from band to band along the river.
  > In the chronicle, the story is already being retold as myth: *Ama stole the fire that sleeps inside the wood*.
  > You close the app, and the world waits for you.

### 1.4 Signature moments

- `VIS-12` **Signature moments** *(Decided)*: Stories the simulation must be able to produce.
  None of them is scripted.
  Each is an example of what the rules should make possible, and each becomes a long-term test (the `MOM` items below).
  The IDs in brackets are the parts of the project each moment depends on.

  - `MOM-01` **Fire from wood** *(Decided)*: In a hard winter, a band whose fire has died learns to make fire by friction.
    (`MND-11`, `RCK-02`, `GOD-03`)
    - **How it works:**
      1. The band keeps a fire found after lightning (`BIO-02`), with beliefs that dry wood feeds it and rain kills it (`MND-05`).
      2. Twirling a stick to bore a hole heats its tip by friction (`MAT-11`), and the smoke is a surprise that leaves a weak belief (`MND-10`).
      3. In a hard winter the fire dies; cold and fear push the most curious to explore (`MND-09`), and planning reaches for smoke as a sign of fire (`MND-11`).
      4. Faster, longer twirling with drier wood heats the dust past its ignition point (`RCK-02`): an ember, then flame.
      5. Success strengthens the belief and the skill (`MND-05`, `MND-06`), and others watch, copy and are taught (`CUL-01`).
      6. A dream you send can pair the smoking stick with fire, raising the odds without guaranteeing anything (`GOD-03`).
  - `MOM-02` **The lost craft** *(Decided)*: A fever kills a band's best stoneworkers.
    For generations its blades are cruder, until the skill is rediscovered or learned again from neighbours.
    (`CUL-01`, `CUL-02`)
    - **How it works:**
      1. The best stoneworkers hold the finest knapping settings (`MND-06`), and others copy from them (`CUL-01`).
      2. A fever spreads through the band by real contact (`WLD-21`) and kills them (`BIO-14`).
      3. The survivors copy from the best who remain, whose settings are worse, so blades come out cruder (`CUL-02`).
      4. Blades improve again only through practice and lucky variation (`MND-06`), or by copying neighbours who kept the skill, met through contact (`CUL-16`).
      5. The recognisers mark the loss and any rediscovery (`PRE-39`).
  - `MOM-03` **Your lightning becomes a god** *(Decided)*: A lightning strike you sent kills a hunter on a hilltop.
    The band avoids the hill, then leaves offerings there, then tells stories about the one who lives in the storm.
    (`GOD-02`, `GOD-06`, `CUL-05`)
    - **How it works:**
      1. You bring a storm and send lightning to the hilltop (`GOD-02`), and it kills the hunter there (`WLD-28`, `BIO-14`).
      2. The band sees a death with no believed cause: the hidden-someone tendency makes a weak belief in an unseen someone in the storm (`MND-21`, `CUL-05`), and fear ties itself to the hill (`MND-08`).
      3. They avoid the hill; later visits that pass safely after things were left there are credited to the leaving (`MND-05`), and the offerings become a rite (`CUL-06`).
      4. Retold stories of the one in the storm become a myth (`CUL-11`), and nothing marks the strike as yours (`GOD-06`).
  - `MOM-04` **The song that does nothing** *(Decided)*: A band sings before a hunt that goes well.
    The song becomes a hunting rite and is kept for centuries, though it changes nothing.
    (`MND-05`, `CUL-06`)
    - **How it works:**
      1. The band happens to sing before a hunt, and the hunt goes well (`WLD-18`).
      2. Credit spreads over what came before, the song included (`MND-05`), and the success is remembered vividly (`MND-08`).
      3. Singing again before hunts is a cheap try, and hunts succeed often enough through skill and luck that the belief survives; copying spreads it (`CUL-01`).
      4. Shared expectation turns it into a rite (`CUL-06`), taught and kept long after anyone remembers why (`CUL-20`).
  - `MOM-05` **Two tongues** *(Decided)*: Two bands are separated by a rising sea and drift apart in speech.
    When their descendants meet again, they can hardly understand each other.
    (`CUL-04`, `WLD-16`)
    - **How it works:**
      1. As the ice age ends, melting ice raises the sea (`WLD-16`, `WLD-26`), and it floods the low land between two bands' ranges (`WLD-15`).
      2. Without contact, words and sound changes are copied only within each band (`CUL-16`), so each takes up its own regular sound changes and new words (`CUL-17`).
      3. When their descendants meet again, too few of their words match for them to understand each other (`CUL-04`), and the language tree shows the split (`PRE-36`).
  - `MOM-06` **The camp wolf** *(Decided)*: The boldest wolves scavenge at the edge of camp.
    Their pups grow tamer each generation, until a child raises one.
    (`MND-16`, `WLD-20`)
    - **How it works:**
      1. Wolves near the camp are individuals with minds (`WLD-12`, `MND-16`); food smells and scraps draw the boldest to the camp's edge (`MND-07`, `MND-20`).
      2. Wolves that are fed and not harmed lose their fear of people and grow attached (`MND-16`), and their dreams replay the warm scraps (`MND-12`, `GOD-12`).
      3. The bolder wolves raise more pups near people, and boldness is inherited (`WLD-20`).
      4. A child who feeds and plays with a pup grows attached to it, and it to the child (`MND-07`, `MND-24`), and the pup is raised in camp.
  - `MOM-07` **A painting that remembers** *(Decided)*: A painting of a great hunt outlasts everyone who saw it.
    You tap it and read what really happened in that hunt.
    (`CUL-09`, `PRE-15`)
    - **How it works:**
      1. A great hunt is a vivid shared memory (`MND-08`).
      2. Someone paints it on a sheltered wall with prepared ochre (`CUL-09`, `RCK-15`), and the painting's record keeps what it shows and the memories it came from (`CUL-25`).
      3. Paint in shelter weathers slowly (`RCK-16`), so the painting outlasts everyone who saw the hunt.
      4. The hunt's events are in the saved history (`PRN-15`), so tapping the painting shows what really happened (`PRE-15`).
  - `MOM-08` **Seeds on the rubbish heap** *(Decided)*: Seeds thrown on a rubbish heap sprout near camp.
    Years later, someone starts planting on purpose.
    (`MND-11`, `WLD-18`)
    - **How it works:**
      1. People eat seeds and fruit and throw the waste on a heap by the camp, and some seeds survive in it (`MAT-10`).
      2. The heap is rich from waste and ash (`WLD-27`), so the seeds sprout and grow well there (`WLD-18`).
      3. People notice food plants growing where seeds were thrown (`MND-10`) and form a belief linking thrown seed to plants (`MND-05`).
      4. When food runs short, someone puts seeds in the ground on purpose (`MND-11`); the plants that come up confirm it, and planting spreads by copying (`CUL-01`).
  - `MOM-09` **The dig** *(Decided)*: Under a village, you find the hearths of the first band and the bones of the animals they ate.
    (`MAT-08`, `PRE-09`)
    - **How it works:**
      1. The first band's hearths, bones and tools stay where they were left, as things or as merged leftovers (`MAT-10`).
      2. Layer by layer the place is buried as the land builds up (`MAT-08`), and what survives depends on the soil's wetness, air and acidity (`WLD-27`).
      3. Centuries later a village stands above; the cut-away shows the layers (`PRE-25`), and each find's record tells who left it and when (`PRE-09`).
  - `MOM-10` **Two endings** *(Dropped)*
    - **Dropped because:** rewinding and branching were cut in the realism pass: a world keeps only its present state and its chronicle (`PRN-15`).
  - `MOM-11` **Rivals, then in-laws** *(Decided)*: Two bands fight over a valley, then marry into each other.
    Each side's descendants tell the story differently.
    (`CUL-07`, `CUL-11`)
    - **How it works:**
      1. Two bands depend on one valley's food (`WLD-18`); meeting there, fear, anger and hunger make fighting a choice each side weighs (`MND-09`, `CUL-08`).
      2. Losses on both sides, small bands, and desire held down toward those one grew up with (`MND-26`) make pairing across the bands a better choice for some (`BIO-15`, `CUL-07`).
      3. Pairings make kin across the bands (`MND-24`), and favouring kin makes fighting costlier to choose (`MND-26`).
      4. Each side keeps its own memories of the fight, retold through its own beliefs, so their stories differ (`MND-18`, `CUL-11`).
  - `MOM-12` **Metal from green stone** *(Decided)*: A kiln built very hot for pottery leaves a bead of shiny metal where green stones lined the fire, and someone notices.
    (`MAT-07`, `RCK-08`)
    - **How it works:**
      1. People build a kiln for pottery, enclosing a charcoal fire with clay or stone and blowing it, so it runs hotter (`MAT-04`, `RCK-04`).
      2. Green copper-bearing stones lining the fire touch burning charcoal past the temperature at which charcoal takes their oxygen (`MAT-07`), and copper runs out as a bead once the fire passes its melting point (`RCK-08`).
      3. The bead's shine and weight are a surprise (`MND-10`); whoever notices links it to the green stones and the hot fire (`MND-05`), and may try again on purpose.

### 1.5 The arc of a world

- `VIS-03` **No ceiling** *(Decided)*
  - **What:** There are no eras, levels or end state.
    A world's history goes as far as its people take it.
  - **Why:** Any fixed sequence of eras would be a tech tree in disguise.
  - **In practice:** some worlds may stall for tens of thousands of years, and some bands will die out.
    Some peoples may reach farming, writing, metals and beyond; some may take paths our own history never took.
    Nothing about the order of our history is guaranteed, except where physics forces it: no one smelts copper without a fire hot enough.
    Collapse, stagnation and extinction are all valid histories.
    The one limit is the phone: a world grows only as far as the phone can run every person at full depth (`MND-15`), so cities and farming-scale worlds are out of reach.

### 1.6 What makes it different

- `VIS-13` **Seven differences** *(Decided)*: A summary of decisions made in other sections.
  - **No recipes.** Discoveries come from physics, not from lists (`PRN-01`; see Matter and physics).
  - **Minds that learn.** People form their own concepts, beliefs and skills.
    Science and superstition come from the same mechanism (see Minds).
  - **Real matter.** Real chemistry and real-world numbers decide what is possible (see Matter and physics).
  - **You are nature.** An invisible god, limited to what nature could do (see The player as god).
  - **Every story can be traced.** Two views of every mind, archaeology, and a chronicle whose every entry leads back to what happened (see Presentation).
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

- `VIS-05` **Quality bar** *(Decided)*: The rigour of a research project, and craft polished as far as the tools allow.
  - **Research rigour:** what the simulation is claimed to do is tested by experiments that can fail, across many runs, with real-world values and repeatable results.
  - **Craft:** art, sound, interface and performance polished as far as procedural art, animation and sound made by AI agents allow, judged by you at every visual review (`PRE-31`).
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
  - **Check:** every milestone report lists each principle with the result of its Check line (`RES-06`).

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
  - **What:** The past is kept only as the chronicle and the events behind it, with the records each view of the past needs.
    The world's full state is kept only for the present (`PLT-07`), so the past can't be replayed or returned to.
    The past is never recomputed, and the phone and cloud builds don't have to produce identical histories (`PLT-05`).
    A seed decides how a world is generated, not how its history unfolds.
  - **Why:** Re-running history exactly would need identical maths on every device, and every old version of the rules kept forever.
    Saving what matters avoids those costs, so the effort goes into depth on the phone.
  - **Example:** You tap a cave painting of a great hunt.
    The hunt's events were saved, so you can read who was there, what happened and how it ended.
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
  - **Time and history** (see Time and history): time that follows zoom, a story director, and a chronicle of everything that happened.
  - **Presentation** (see Presentation): detailed pixel art, one continuous zoom from the globe to a single person, and many ways to follow the story: the chronicle, following one person's life, map overlays, archaeology and more.
  - **Sound** (see Sound): a living soundscape first, then their music, their voices and a score.
  - **The phone app** (see Platform and performance): built for one phone, in portrait and landscape, smooth at all times.
  - **Research tools** (see Research and validation): experiments in small sandboxes, confirmed in full worlds, run in the cloud, with reports and experiment worlds you review on the phone.

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
  - **How it works:** a starting point is a set of starting records and settings for the short run before year 0 and the starting kit (`BIO-02`, `BIO-20`): ice-age hunters start with the words, skills and beliefs of foragers of that time; ancestral minds start with lower settings for learning, memory and planning, inherited and able to evolve (`BIO-06`); a blank slate starts with no words, skills or fire; the world itself is made the same way.

### 3.3 Who it's for

- `SCP-02` **Just you** *(Decided)*
  - **What:** Kindling is built for one person, on one phone.
  - **In practice:**
    - no public release, store listing or tutorial, only short help cards (`PRE-40`);
    - no support for other phones, tablets or computers (experiments in the cloud are a research tool, not a way to play);
    - no accounts, purchases, ads or analytics;
    - free to use your phone's specific hardware (`PLT-01`).
  - **Why:** Building for one person and one device removes whole categories of work, so the effort goes into depth and polish.
  - **Check:** the builds hold no account, purchase, advertising or analytics code, and target only your phone (`PLT-01`).

### 3.4 How it gets built

- `SCP-03` **Experiments first** *(Decided)*: Core ideas are proven in experiments, mostly in small sandboxes, before the game builds on them (`RES-01`), and a phone app grows alongside, so you can watch the results from the start.
  - **Check:** every milestone report traces its features to experiments that passed (`RES-06`).

- `SCP-15` **Experiments run in the AI's cloud sessions** *(Decided)*
  - **What:** Experiments run in the same cloud sessions where the AI builds the game, within those sessions' computing limits.
  - **Why:** There's nothing extra to set up, maintain or pay for.
  - If an experiment ever needs more computing power than a session offers, that is raised with you before anything else is set up.
  - **Check:** every experiment report states where it ran and within what computing budget (`RES-16`).

- `SCP-16` **Milestones** *(Decided)*: The project moves through these milestones in order.
  Each ends with a report you review (`RES-06`).
  This file keeps each milestone's goal and order; the implementation plan maps every item to a milestone, with tasks and dates.

  1. `MIL-01` **Foundations** *(Decided)*: a small generated valley that runs on the phone and in the cloud with the same statistics (`RES-05`), the experiment runner and its first report, and a basic phone viewer for saved history.
     *Now possible:* watching a generated valley pass through its days and seasons on your phone.
  2. `MIL-02` **Sharp stone (Experiment 1)** *(Decided)*: stone that breaks by real rules; people who perceive, form concepts, learn cause and effect, build skills and learn from each other; just enough food and terrain to live on.
     *Now possible:* watching a band discover how to chip stone, and seeing the skill spread or be lost.
  3. `MIL-03` **Fire and the first power** *(Decided)*: heat, burning and friction; keeping and making fire; dreams, your first power.
     *Now possible:* a band that can only keep fire learns to make it, and you can send a dream and see what comes of it.
  4. `MIL-04` **A living world** *(Decided)*: plants and animals in food webs, with weather and seasons; animals with simpler minds; hunting; your powers over nature and fortune; the living soundscape.
     *Now possible:* hunting becomes an arms race, and your storms and blessings change lives.
  5. `MIL-05` **Words and beliefs** *(Decided)*: language emerging, explanations, ritual and myth; the chronicle and life stories written by the writer AI.
     *Now possible:* rites form, dialects drift apart, and the chronicle reads like a history.
  6. `MIL-06` **The whole world** *(Decided)*: the full wrap-around world, migrations, many bands and diverging cultures, one continuous zoom from the globe to a single person, and archaeology.
     *Now possible:* watching peoples spread, split and meet again across a whole world.
  7. `MIL-07` **Open-ended growth** *(Decided)*: taming animals, farming, settlements and whatever comes after, each built when an experiment calls for it.
     *Now possible:* history keeps going for as long as the world's people fit what the phone can run at full depth (`MND-15`).

### 3.5 Non-goals

Things the project deliberately does not do, and why.

- `SCP-04` **No recipes or tech tree** *(Decided)*: Discoveries come from physics and learning (`PRN-01`, `PRN-07`).
  - **Check:** the general-rules check passes (`PRN-07`), and the code holds no list of recipes or unlocks.
- `SCP-05` **No other human species** *(Decided)*: There is one human species, so the story stays about how one people learns.
  - **Check:** the species catalogue holds one human species, and no starting point adds another (`SCP-14`).
- `SCP-06` **No AI language model making decisions** *(Decided)*: Our own knowledge would leak into their world (`PRN-06`).
  - **Check:** the check of `MND-01` passes.
- `SCP-07` **No goals, scores, wins or losses** *(Decided)*: It is a sandbox; the story is whatever happens.
  - **Check:** the app has no goal, score, win or loss, on screen or in the code.
- `SCP-08` **No worship of the player** *(Decided)*: Your power doesn't depend on their faith, and they never learn you exist (`GOD-06`).
  - **Check:** no power reads any mind's beliefs, and the check of `GOD-06` passes.
- `SCP-09` **No terraforming** *(Decided)*: You can't reshape land or add or remove species.
  You act only as nature could (`GOD-05`).
  - **Check:** every power is a request to a natural system (`GOD-05`); none reshapes land or adds or removes a species.
- `SCP-10` **No shared online world or multiplayer** *(Decided)*: It's yours alone (`SCP-02`).
  - **Check:** nothing in play uses a network connection (`PLT-03`), and the app has no online features.
- `SCP-11` **No real-Earth map** *(Decided)*: Every world is generated (see World).
  - **Check:** worlds come only from the generator (`WLD-10`), and the app holds no real-Earth map data.
- `SCP-12` **No simulated planet formation** *(Decided)*: Worlds are generated directly in a realistic present-day state, which keeps generation cheap (`WLD-08`).
  - **Check:** generation runs only the stages of `WLD-09`.
- `SCP-17` **No direct control** *(Decided)*: You never control any person or animal, not even briefly (`GOD-01`).
  - **Check:** no control in the app sets any being's actions; your only way into the simulation is your powers (`GOD-05`).
- `SCP-18` **No scripted story** *(Decided)*: There is no campaign, no quests and no authored events.
  Every story comes from the simulation (`PRN-01`).
  - **Check:** a code search finds no authored events, quests or campaign data; every event comes from the rules.
- `SCP-19` **No magic in the world** *(Decided)*: Nothing supernatural exists in the world's physics.
  Spirits and gods exist only in people's beliefs.
  The only unseen force is you, and you act through nature.
  - **Check:** every law in the catalogue is physical or biological (`MAT-13`), and no law reads people's beliefs about spirits.
- `SCP-20` **No borrowed real cultures** *(Decided)*: Their peoples, names, languages and customs are their own.
  Nothing is copied from real cultures, and descriptions never compare them to real peoples.
  - **Check:** each milestone review checks names, words, customs and descriptions for anything copied from or compared with a real people; languages draw on their own sounds (`CUL-17`), starting looks are mixed (`BIO-22`), and the writer's instructions forbid comparisons (`PRE-17`).

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
  - **How it works:** every intervention enters the world only as a change to a natural system's own inputs: a storm system added to the weather (`WLD-16`), a season's chances shifted, stored strain or magma released (`WLD-15`), a dream's content chosen (`MND-12`), or a chance draw retried (`GOD-04`).
    People perceive only the weather, the luck and the dreams, through their senses (`MND-03`), and nothing anyone can perceive marks an event as yours; what they make of it forms by the usual mechanisms (`CUL-05`).
  - **Check:** a code check finds no path from the record of your interventions (`GOD-08`) into anything a mind can perceive.
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
  - **How it works:** each power is a request to a natural system, which carries it out by its own rules: a storm is added only where the weather could form one, drawn as its storm belt would draw it, and builds over hours (`WLD-16`); lightning needs a storm overhead; a pushed season shifts the weather's chances within the place's climate (`WLD-16`); quakes and eruptions release strain and magma already stored, so their size is nature's (`WLD-15`); floods and fires need the water and fuel to be there.
    Each request is checked against the same conditions the world uses for natural events, and refused if it fails them (`GOD-11`).
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
  - **How it works:**
    - **Small events:** each sets its weather cell's own physics to what gives it, within what that cell's weather could do that hour (`WLD-16`): a lightning strike lands at the spot you chose under a storm overhead (`WLD-28`); a shower falls from cloud that is there; a gust, a cold night or a fog comes from shifting the cell's wind, sky or air within its range.
    - **Seasons:** over a region you draw, up to about one climate zone across, storms are drawn more or less often and the air is nudged warmer, colder, wetter or drier, within the place's natural range for that season (`GOD-05`).
    - **Disasters:** a flood comes from rain sent over a river's catchment, with the water balance doing the rest (`WLD-17`); a drought from storms held away; a storm from one drawn in its belt; a wildfire from lightning on dry fuel (`WLD-28`); an eruption or earthquake from releasing what is stored, at its stored size (`WLD-15`).
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
  - **How it works:** you choose, from the sleeper's own memory records (`MND-18`), one memory to relive or two to bring together, and a feeling; that night's dream uses your choice in place of its own random recombination (`MND-12`), at the strength of the night's strongest natural dream.
    It then works as any dream does: the relived memory is renewed, a pairing leaves a weak new belief, the feeling colours both, and the dream is remembered and can be told (`CUL-24`); sending it again follows the same rules as a natural recurring dream.
  - **Why:** Dreams are where minds recombine experience (`MND-12`), so they are the most natural way for a god to touch an idea without supplying it.
  - **Example:** The session story in `VIS-11`.

- `GOD-12` **Animal dreams** *(Decided)*
  - **What:** Animals can be sent simpler dreams: one memory relived, coloured by a feeling.
  - **How it works:** in a sleeping animal, you choose one of its memories and a feeling; its simpler replay (`MND-16`, `MND-12`) renews that memory and shifts the feelings tied to it, so a camp relived with ease leaves a little less fear of it.
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
  - **How it works:** a long-press opens the powers that pass the checks for that target (`GOD-11`); time is paused while you choose (`TIM-15`); a drawn area is traced on the map with a finger; a sleeper's memories are shown as scenes drawn from their memory records, as they remember them (`MND-18`); and confirming sends the request to its natural system (`GOD-05`), with your choices recorded (`GOD-08`).

- `GOD-11` **What's possible here** *(Decided)*
  - **What:** The game only offers what nature could do at that place or to that being right now, and says briefly why other powers aren't available, such as "no volcano here" or "she is awake".
  - **How it works:** for the chosen target, each power's conditions are checked against the world's present state: a storm overhead for lightning, magma for an eruption, strain on a fault, fuel dry enough, the target asleep for a dream (`GOD-05`); powers that pass are offered, and each that fails shows the condition it failed.
  - **Why:** You never have to guess what's natural, and you never try a miracle by accident.

### 4.4 Records of your interventions

- `GOD-08` **Recorded behind the scenes** *(Decided)*: Every intervention is recorded with its time, place, target and every detail (the memories chosen, the feeling, the region drawn, the duration), as part of the saved history (`PRN-15`).
  The scientist's view of your interventions depends on this record (`GOD-09`).
  - **How it works:** each intervention is an event record holding every choice you made, saved in the history like any event and marked as yours (`PRN-15`); it is kept out of anything minds can perceive (`GOD-06`) and out of the story view (`GOD-07`).

- `GOD-07` **No trace in the story view** *(Decided)*: The story view never shows where you intervened or how much you helped.
  - **How it works:** the story view and the writer receive the records with your intervention records left out, and events your acts caused look like natural ones; only the scientist's view reads the intervention records (`GOD-09`).
  - **Check:** a test runs the story view on a history with interventions and finds no trace of them in what it shows or writes.

- `GOD-09` **Interventions in the scientist's view** *(Decided)*
  - **What:** The scientist's view shows where and when you intervened, and traces what changed because of it.
    It follows the chain of causes from your act through the saved events.
  - **How it works:** the view lists your interventions from their records (`GOD-08`) and follows their consequences through the cause links the history keeps: what lit each fire, the memories behind each belief (`MND-05`), the chain behind each death (`BIO-14`).
  - **Why:** Curiosity (`VIS-08`): you can find out what your nudges actually did.
  - **Example:** You select the dream you sent Ama and follow what came of it: eleven days of twirling sticks, the first fire, and fire-making spreading along the river.

## 5. Time and history

After the camera, time is your main control.
This section defines how fast time runs, what decides its speed, what happens while you're away, and how worlds, chance and dates are kept.
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
    - **the whole world:** as fast as the phone can.
  - **How it works:**
    - **One world clock:** every system advances to the same clock, in steps set by the world's own rates (`WLD-12`); the speed asked for is how much simulated time should pass per real second.
    - **Zoom asks for a speed:** each zoom level has its target from the list above, blended smoothly between levels.
    - **As fast as the phone can, up to that speed:** each frame, the simulation runs as many steps as the phone's budget allows while the screen stays smooth (`PLT-04`); if it can't reach the speed asked, time runs slower and the speed shown is the real one (`PRN-11`).
      The steps themselves never depend on the speed (`WLD-13`).
    - **What is drawn at speed:** each frame shows the world's state at that moment; nothing is drawn that the simulation didn't have (`PRN-10`).
  - **Why:** Close-up moments are lived; distant eras are watched.
  - **Example:** You watch the knapper strike, flake by flake.
    Then you pull back over the valley, and a whole summer passes while the herds move north.

- `TIM-10` **Natural speed up close** *(Decided)*: At the closest zoom, people and animals move at real-life speed.
  You can watch a flake come off the stone.
  - **How it works:** at the closest zoom the speed asked is one simulated second per real second; each action is animated over its real duration from its settings (`MAT-06`), and sounds play in real time (`SND-07`).

- `TIM-04` **Manual control** *(Decided)*: You can unlink speed from zoom whenever you want.
  The controls: pause, play, a speed dial, and a lock that keeps the current speed while you move the camera.
  - **How it works:** pause asks for no time at all; play hands the speed back to zoom; the dial asks for the speed you set; and the lock keeps the speed asked when you started moving the camera.
    The real speed still can't pass what the phone manages (`PRN-11`).

- `TIM-15` **Who sets the speed** *(Decided)*: Your pause and speed lock beat the story director (`TIM-02`), and the director beats zoom.
  Choosing a power pauses time.
  Overnight mode (`TIM-12`) ignores the director, but keeps its moments for the morning.
  - **How it works:** the speed asked comes from the highest active source in this order: pause and the lock, then the director, then zoom; choosing a power sets pause until you confirm or cancel it (`GOD-10`).

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
  - **How it works:**
    - **It reads the event stream:** every event the simulation records (`PRN-15`) passes the recognisers (see Presentation), which tag what kind of importance it has, by patterns in the records and never by the simulation naming anything: a first (an outcome of a kind this world's history has never recorded), births and deaths of the people you follow, a skill or belief reaching a new band or losing its last holder, fighting between groups, disasters past a size, a band leaving its range, a band forming, splitting or ending, and events traced back to one of your acts.
    - **Signs before outcomes:** it also watches present states that often come before such events, such as someone trying something new, a predator closing on a band, or a storm building, so it can slow down before the outcome; it never looks ahead in time.
    - **Scores and speed:** each tag carries a score by kind and size (tuned with you); while a score passes the threshold, the director asks for a slower speed around it, and when nothing does, it lets time race up to the top speed your zoom allows.
    - **Catching up:** a moment you miss waits in the list of live moments (`PRE-08`), with its chronicle entry (`PRE-05`).
  - **Why:** In a world that runs itself, the best moments are easy to miss (`RSK-03`).

- `TIM-03` **The director never touches events** *(Decided)*: The director controls speed only.
  It decides where to slow down but never causes, changes or hides anything.
  Follows from `PRN-10` and `PRN-12`.
  - **How it works:** the director only reads the event stream and the world's state, and only sets the speed asked and the live-moment prompts; it cannot write to the simulation, and since speed changes nothing (`WLD-13`), history is the same with or without it.
  - **Check:** the same saved state run with the director on and off gives the same results bit for bit on the same phone (`TIM-16`), and a code check finds no path from the director into the simulation.

- `TIM-11` **Skip to the next moment** *(Decided)*: A control that runs time at top speed until the next important moment, then slows down.
  Useful for short check-ins (`VIS-10`).
  - **How it works:** it asks for the top speed until the director's next score passes its threshold (`TIM-02`), then hands the speed back to the director and zoom.

### 5.3 While you're away

- `TIM-05` **Pauses when closed** *(Decided)*: When the app is closed or in the background, the world stops.
  Nothing happens while you're away, and every session starts exactly where the last one ended.
  Opening the app resumes time.
  - **How it works:** when the app leaves the screen, the simulation finishes its current step, stops, and saves its state (`PLT-07`); nothing runs in the background, and reopening loads that state and carries on from the same step.

- `TIM-12` **Overnight mode** *(Decided)*
  - **What:** Leave the app open on the charger and switch on overnight mode.
    The world runs at top speed with the screen dimmed.
    When you come back, a summary tells you what happened, drawn from the chronicle (`PRE-05`).
  - **Why:** Deep simulation runs slowly on a phone (`PRN-11`).
    Overnight mode gives history the hours it needs without you having to watch.
  - **Safeguards:** it runs only while the phone is charging, and it stops if the phone gets too hot.
  - **How it works:** it asks for the top speed, draws only a dim, slowly updated picture, and queues the director's moments instead of slowing for them (`TIM-15`).
    It reads the phone's own temperature warnings and slows, then pauses, before the phone gets hot (`PLT-04`), and it pauses when the charger is unplugged.
    The morning summary takes the night's most important events by the director's scores, from the chronicle (`PRE-05`), worded by the writer, with dark events given as plain facts (`PRE-37`).
  - **Example:** You start it before bed.
    In the morning: "312 years passed.
    Two bands merged by the river; a long drought pushed the eastern band over the hills; on the coast, someone began drying fish."

### 5.4 Worlds, chance and dates

- `TIM-06` **Rewind and branch** *(Dropped)*
  - **Dropped because:** saved history was cut in the realism pass: one full save of the world is estimated at a few GB, so a world keeps only its present state and its chronicle (`PRN-15`).

- `TIM-16` **Chance is local** *(Decided)*: Each chance event belongs to one being and one moment, so on the same phone and version, the same saved state always gives the same result.
  - **How it works:** every chance draw is made from a key of world, system, being, moment and purpose, so the same being at the same moment for the same purpose always gets the same draw.
  - **Why:** It makes tests repeatable (`WLD-13`, `TIM-03`, `RES-21`) and lets a world recover exactly after a crash (`PLT-07`).

- `TIM-13` **Comparing timelines** *(Dropped)*
  - **Dropped because:** branching was cut with saved history in the realism pass (`PRN-15`).

- `TIM-08` **Saved worlds** *(Decided)*: Several worlds kept on the phone, each with its present state and its chronicle.
  You can switch between them.
  - **How it works:** each world keeps its seed and generator version (`WLD-08`), its present state (`PLT-07`), and its event history and chronicle (`PLT-10`); switching saves the current world and loads the other.

- `TIM-14` **Dates** *(Decided)*: The game counts years from the moment a world's history begins ("year 2,314"), with days and seasons set by that world's own sun and moons (`WLD-06`).
  The people's own calendars are separate (`CUL-13`).
  Inside the simulation, and in every target and criterion, time is counted in Earth days and years; on screen, dates use the world's own years and ages use Earth years.
  Bodies are adapted to their world's day length.
  - **How it works:** the clock counts Earth seconds; the world's own days and years come from its spin and orbit (`WLD-06`) and are worked out from the clock only for display, counting years from year 0.
    Each body's daily rhythm follows the world's day, so people sleep through its nights, and the share of the day spent asleep is kept (estimated).

### 5.5 Pacing and endings

- `TIM-07` **Pacing** *(To test)*: How fast history runs is measured and tuned during development (`PLT-04`).
  The first target for the tests: a thousand years in one night for a world of a few hundred people.
  - **How it works:** every build runs a benchmark world at overnight speed on the phone and reports the years passed per hour, at each zoom and for each number of people (`PLT-04`); the speed comes only from the mechanisms above and from engineering, never from cutting depth (`PRN-11`).

- `TIM-09` **If everyone dies** *(Decided)*: The world goes on without them.
  Nature carries on, and you can keep watching or start a new world.
  - **How it works:** the last death is an important moment for the director; the world's systems carry on as before, faster with no minds to run, and the choices offered are to keep watching or start a new world (`WLD-10`).

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
  - **How it works:**
    - **The map** is a rectangle about 2,000 km east to west and 1,000 km north to south, whose opposite edges join, so positions wrap both ways and every distance and neighbour is measured across the wrap.
    - **Cells:** the map is cut into square cells that nest, from the whole map down to 1 m, in about 21 halvings.
    - **Latitude:** the middle line is the equator, and latitude rises toward the north–south wrap line, which is both poles at once.
      The sun's height and the length of the day at any place and date follow from latitude, tilt and orbit (`WLD-06`) by standard astronomy formulas, so seasons reverse between the two halves on their own.
    - **The polar seam:** the ice along the wrap line is about 200 km wide, about two weeks' walk, and stays permanent through every change of climate (`WLD-16`).
      Weather stops at it, and, as the named exception, nothing crosses its centre line, so the promise holds even for a people who could carry food and fuel across that much ice.
  - **Why:** There are no edges and no stretched or squashed regions, so every place can be simulated in the same way.

- `WLD-02` **Globe view** *(Decided)*: Fully zoomed out, the world is drawn as a globe.
  The wrap only shows at the poles.
  The globe squeezes the polar regions, which on the map are as wide as the equator; this is a known exception in the display only, and the map keeps every place at its true size.
  - **How it works:** a picture only: east–west position becomes longitude, latitude stays as on the map, and the result is drawn on a sphere, with the polar ice hiding the seam.
    The simulation never uses the globe.

- `WLD-03` **Size** *(Decided)*
  - **What:** About 1,000 km from pole to pole and about 2,000 km around: roughly 2 million km² in all, land and sea together.
  - **What follows:** Each climate zone is roughly 100 km wide, about four to five days' walk.
  - **How it works:** areas are measured on the flat map, so a square kilometre is the same everywhere; only the globe view squeezes the poles.
    Climate zones are whatever the climate rules give at each latitude (`WLD-16`).
  - **Why:** It is big enough for many separate peoples and small enough to simulate deeply (`PRN-02`).

- `WLD-30` **What scales with the world** *(Decided)*: Quantities set by distance (weather systems, ocean currents, migrations and climate belts) scale with the world's size.
  Local quantities (bodies, chemistry, materials and rates of change) stay real.
  Every scaled value is labelled as scaled, with the real value it came from (`PRN-05`).
  - **How it works:** the scale factor is the world's pole-to-pole distance over Earth's, about 1 to 20.
    A value set by distance is its real value times that factor, kept in the catalogue with the real value it came from: a storm system here is about 50 km across instead of 1,000.
    Winds keep their real speeds, so storms cross the smaller world faster.
    Local values, such as a body's needs, a stone's hardness or a fire's heat, are never scaled.

- `WLD-04` **How many people it can feed** *(To test)*: Estimated at roughly 50,000 hunter-gatherers (about one person per 10 km² of good land), or about half a million to five million once farming exists, since farming supports 10 to 100 times more people on the same land.
  These are orders of magnitude only: on the wrap-around map a third of the area lies beyond 60° latitude, so there is less good land than Earth intuition suggests.
  Measured in experiments.
  These are what the land could feed, not what the phone can run (`MND-15`).
  - **How it works:** it is set nowhere: it is however many people the food the land really produces (`WLD-18`) can keep alive (`BIO-09`), measured by running worlds.

### 6.2 The planet

- `WLD-06` **Varied within reason** *(Decided)*: Each world has its own day length, year length, axial tilt (and so the strength of its seasons), moons, and share of land, all within ranges that allow human-like life.
  The ranges: day 18–36 hours, year 250–500 days, tilt 5°–35°, 0–3 moons, 25–50% land.
  Gravity, air and chemistry stay Earth-like.
  - **How it works:** the seed draws day length, year length, tilt, the number of moons with their sizes and orbits, and the land share, within the ranges.
    They feed the sun's path and the seasons (`WLD-01`), the tides (`WLD-26`) and the climate (`WLD-16`).

- `WLD-07` **A rich sky** *(Decided)*
  - **What:** The sun, moons, stars and planets move realistically for each world's orbit and tilt.
    Eclipses, comets, meteor showers and auroras happen.
  - **How it works:**
    - **Drawn from the seed:** the world's orbit, spin and tilt, its moons' orbits, a few planets on their own orbits, and the star field; where each one is at any moment comes from standard orbit formulas.
    - **Events:** eclipses happen when orbits line up, comets come on orbits drawn from the seed, meteor showers return on the same dates each year, and auroras follow a seeded activity cycle, seen at high latitudes.
    - **Seen like anything else:** people see the sky by sight (`BIO-18`), and minds can learn its cycles (`CUL-13`).
  - **Why:** The sky is the first calendar, the first compass and a great source of myth (`CUL-13`).
  - **Example:** A comet that hangs over the valley for a month, the same month the old chief dies, becomes part of how the band remembers that winter.

### 6.3 Making a world

- `WLD-08` **Realistic, not from scratch** *(Decided)*: Worlds are generated directly in a realistic present-day state, using fast methods that imitate what deep time would have produced.
  Generating one is cheap.
  - **How it works:**
    - **Stages:** generation runs the stages of `WLD-09` in order, each worked out from the seed and the stages before it.
      Each stage uses the real rule for its process, such as uplift where plates meet or water cutting valleys, run in a few long steps instead of through geological time.
    - **The same rules as play:** where play has a law for the same process, such as erosion, soil change or plant growth, the generator uses that law, so play carries on from the generated state without a jump.
    - **What is kept:** the stages' results are stored with the world, from whole regions down to cells of about 1 km, and patches of a few hundred metres for plants and animals (`WLD-12`).
      Finer detail, down to the metre, is never made in advance: it comes from the same rules when needed, the same way every time (`MAT-10`).
    - **Exactly repeatable:** the generator uses exact, repeatable maths, so a seed gives the same world bit for bit on the phone and in the cloud, and an untouched place always comes back as it was.
      So any change to the generator counts as a big update (`PLT-09`).
    - **Settling:** once you pick a world, it runs the years before year 0 by the play rules, with no people, until water, plant cover and animal numbers stop trending and only rise and fall with the seasons and the weather, up to a limit in years (tuned).
      History then begins from a state the world's own rules keep.
      At year 0, the animals around the bands are given the wariness of people that living beside hunters gives (`MND-16`, estimated), just as the bands start with their knowledge (`BIO-02`).
      Named simplification: the bands' own small effect on the land before year 0, such as their hunting, is left out.
    - **Calibrated to Earth:** run on many seeds during development, each stage's results are compared with Earth's measured figures, such as the spread of heights and slopes, how rivers branch and lengthen, how rough coastlines are, the sizes of lakes, and the share of each kind of climate.
      The rules are tuned until worlds fall inside Earth's ranges, and the figures used are sourced (`PRN-05`).

- `WLD-09` **What generation produces** *(Decided)*: using rules derived from real physics and calibrated to Earth, not full physical models, generation produces, in this order:
  1. tectonic plates, mountain ranges, volcanoes and faults;
  2. rock types and layers, with minerals and ores in geologically plausible places;
  3. erosion: valleys, rivers, lakes, deltas and coastlines;
  4. climate, worked out from the geography (`WLD-16`);
  5. soils, from rock, climate and time;
  6. vegetation and landscapes;
  7. animals and microbes adapted to them (`WLD-19`).
  - **How it works:**
    - **Plates:** the seed draws about 6 to 12 plates (tuned), each moving its own way, with ocean or continental crust; the continents make up the land share (`WLD-06`).
      Continents are pieced together from blocks of different ages, so they have old worn-down ranges, basins and rifts inside them, and ragged edges.
      Where plates meet, their motion sets what happens: plates pushing together raise ranges; an ocean plate sinking under another raises a line of volcanoes; plates pulling apart open rifts; plates sliding past each other leave faults.
      The height and width of each range follow rules calibrated on Earth's ranges of the same kind.
      Volcanoes, faults and hot spots are kept as features that stay active in play (`WLD-15`).
    - **Rock:** each cell of about 1 km gets a column of rock layers down to about 1 km deep (estimated), each with its rock, thickness, tilt and cracks, set by the cell's history: old crystalline rock under the continents; sandstone, shale, limestone and chalk where seas and basins lay; lava and ash near volcanoes; folded and baked rock in ranges; granite where molten rock cooled underground.
      A catalogue of rock settings lists, for each setting, the rocks it makes and the minerals and ores that come with them and how often, from geology: flint in chalk, obsidian in young silica-rich lava, copper ores near granites in volcanic ranges with green weathered tops (`MOM-12`), tin in granites and in the river gravels below them, salt in dry basins, ochre where iron-rich rock weathers, and clay from weathered rock and along rivers.
      Ore bodies are single features with a place, a size and a share of metal, drawn from the seed.
      Caves form where water dissolves limestone along its cracks, where lava drained out of tubes, and where soft rock wore away under hard, leaving overhangs; each is kept as a 3D piece.
    - **Erosion:** water cuts the land faster where more water flows (from the area upstream and the rain), where it is steeper, and where the rock is softer; slopes creep, and slide where steeper than they can hold; carried sediment settles where the water slows, in fans, floodplains, lakes and deltas.
      Run in long steps until the land nears a balance (tuned), it makes valleys, river networks, lakes (hollows fill up to their outlets) and coastlines.
      It runs with the sea at its ice-age low, so valleys run out across the shelves the rising sea will flood (`WLD-16`).
      Where ice lay, the land is carved as glaciers carve: U-shaped valleys, lake basins, ridges of rubble, and, beyond the ice, spreads of wind-blown silt.
    - **Climate:** worked out on the finished land by the climate rules (`WLD-16`), for the start date and for the long cycle ahead.
    - **Soils:** worked out by the soil rules (`WLD-27`) from the material underneath (the rock, or river silt, rubble left by ice, wind-blown silt or ash), the climate, the slope, the plant cover, and how long the surface has stood: young where the ice has just left.
    - **Plants:** plant species are made to fit the world's climates and soils (`WLD-19`), and each species' tolerances (of cold, drought, flooding, shade, acid soil and fire) decide where it can grow.
      Each patch of a few hundred metres gets the plants its conditions support, at a stage of regrowth since its last fire, flood or storm, drawn from the seed at the rates such events have in that climate on Earth (estimated): so the land is a mosaic of old forest, burned and regrowing patches, meadow, marsh and scrub.
      Each patch keeps each species' amount and, for trees, their ages; single plants come from that when needed (`MAT-10`).
    - **Animals and microbes:** animal species are made to fit the habitats and foods (`WLD-19`).
      Each patch's numbers of each species come from the food the patch grows and that species' needs, which follow from its body size (`BIO-09`), so plant eaters follow the plants, and hunters their prey.
      Herds that migrate get summer and winter ranges from where their food is in each season.
      Numbers are kept by age and sex, and individuals come from them when needed (`MND-16`, `WLD-12`).
      Microbes come from Earth families in the same way: decomposers in soil and water, yeasts on fruit, and diseases in their hosts (`WLD-21`).
    - **Stages that need each other:** erosion needs rain before the climate stage, and soils need plants before the plant stage.
      So each uses a first, rough version of the later stage (rain from latitude, the sea and the heights; plant cover from the climate alone), and the later stage then works on the finished result.

- `WLD-19` **Species from Earth families** *(Decided)*
  - **What:** Earth's families of plants and animals (deer, wolves, wild cattle, salmon, grasses, birches, oaks, berries and so on) are the starting point.
    Generation adapts them into each world's own species to fit its landscapes.
    Every species gets its traits: size, diet, behaviour, seasons, and the chemistry of its body, which decides what is edible, poisonous, medicinal or useful (see Matter and physics).
  - **How it works:**
    - **A catalogue of Earth families:** each entry is a group of related Earth species, such as deer, wolves, salmon, birches or grasses, with its real ranges: body size; diet; lifespan, age at first breeding and number of young; group size and behaviour (`MND-16`); the climates, soils and habitats it tolerates; and the makeup of each body part as ingredients (`MAT-01`), including its defensive chemicals and their doses.
      Values that decide what is possible, such as nutrition, poisons and sizes, are sourced (`PRN-05`).
    - **Choosing families:** the generator lists the habitats the world has and draws families that fit them, weighted by how common each family is in such places on Earth (estimated), so every world has its own mix.
    - **From family to species:** where a family's habitat is split by a barrier it can't cross, such as a sea for deer or a watershed for river fish, each side gets its own species, so species follow the world's geography as they do on Earth.
      Each species draws its traits within its family's real ranges, then shifts them by real patterns of how living things fit their climate: bigger bodies where it is colder, shorter limbs and ears in the cold, changes in size on islands, and darker colours where it is humid.
      Traits tied to size, such as food needs, lifespan, age at first breeding and range, follow measured scaling laws.
    - **Use comes from chemistry:** whether something is edible, poisonous, medicinal or useful is never a label: it is the ingredients in each part and their doses, acting on bodies by their measured effects (`BIO-12`).
      Each species' defensive chemicals are drawn within its family's range, so a berry that is food in one world can have a bitter, poisonous cousin in another.
    - **No two alike:** every trait has a real spread between individuals and a real share that is inherited (`WLD-20`, `BIO-06`).
    - **The tree is kept:** species of one family share an ancestor, and families sit in Earth's own tree, so each world has a tree of life you can look at (`PRN-04`).
  - **Why:** Familiar enough to understand, new enough that each world has its own tree of life to discover.

- `WLD-23` **Richness of life** *(Decided)*: About 50 animal and 200 plant species per world, across all groups: mammals, birds, fish, shellfish and insects; trees, shrubs, grasses, herbs and fungi.
  - **How it works:** the generator draws families (`WLD-19`) until the world has about 50 animal and 200 plant species, with every group listed, and every habitat having its plant eaters, hunters, scavengers, pollinators and decomposers.
    Species that can't hold on through settling (`WLD-08`) die out, as they would in play, so the generator aims a little higher (tuned).

- `WLD-10` **Generate many, keep the best** *(Decided)*
  - **What:** The generator makes many candidate worlds, scores each one, and never edits them.
    "New world" shows the best three as small globes, each with a one-line summary.
    You pick one or let the game pick, and you can also enter a seed instead.
  - **What scores well:**
    - varied landscapes and climates;
    - natural barriers (mountains, seas, deserts) that let separate cultures form;
    - resources spread unevenly (flint here, copper there);
    - a good place to begin (`WLD-24`).
  - **How it works:**
    - **Two passes:** about 100 candidates (tuned to fit `WLD-11`), each with its own seed, go through the plates, rock, erosion and a first climate at low detail, and are scored on what those decide.
      The best 10 then go through every stage in full and are scored again.
    - **Each score is a measurement:**
      - **variety:** how many kinds of climate and land the world has with a fair share of the land each, and how evenly the land is shared among them;
      - **barriers:** how many regions big enough to feed a people of several bands (`WLD-04`) are cut off from each other by sea, or by land that takes more than a few days (tuned) to cross on foot by the body's walking rules (slope, rivers, marsh, snow and desert);
      - **uneven resources:** each key material, such as stone that flakes, copper ore, tin ore, clay, salt and ochre, is found in some regions and missing from others, judged by makeup and properties, never by name;
      - **a good start:** the best start region's score (`WLD-24`).
    - **Choosing:** a world must have a start region that qualifies; worlds that do are ranked by the sum of their scores, with weights that are tuned and listed (`PRN-05`).
      If fewer than three qualify, more candidates are made.
    - **Never edited:** a world is offered exactly as generated, or not at all.
    - **What you see:** the best three as small globes drawn from their land and climate, each with a one-line summary built from its scores and facts by fixed sentence patterns, so it says only what the world holds (`PRN-10`).
      Letting the game pick takes the top score.
    - **Your own seed:** entering a seed makes that one world, the same as before for the same version (`WLD-08`); it skips the search, still finds its start region by scoring, and tells you if none qualifies.

- `WLD-24` **Where history begins** *(Decided)*: The bands start in a temperate region with caves, fresh water and varied food within reach.
  The region is found by the scoring, never placed by hand.
  - **How it works:**
    - **Where it looks:** every stretch of land big enough to feed the starting bands all year (`BIO-03`), counting only food they can get with the starting kit (`BIO-02`).
    - **What a region must have,** judged by the world's own rules:
      - **temperate:** a real cool season, with the coldest month below about 10 °C (tuned), that people with the starting kit, with no clothes and no fire, can live through in the region's caves, huddled together, by the body's own heat rules (`BIO-11`) in an ordinary year; and a warm season that doesn't overheat them in shade with water;
      - **caves:** a dry cave or overhang for each band, big enough to shelter it (floor area per person estimated);
      - **fresh water:** water within about 2 km of each shelter (estimated) that lasts through an ordinary year's dry season, from a river, lake or spring;
      - **varied food within reach:** within about 10 km of the shelters, a day's walk there and back (estimated), food the starting kit can get (gathered by hand, scavenged or ambushed) that meets the bands' needs (`BIO-09`) in every season with a margin (tuned), from several kinds, such as plants, land animals and water life, so one failing doesn't starve them;
      - **stone that flakes:** within the same reach, stone that breaks into sharp flakes, judged by its makeup and structure by the breaking rule (`RCK-01`), never by name, so Experiment 1 can happen in every world (`RES-02`).
    - **Ranking:** among regions that qualify, a bigger margin of food, more kinds of food, and more shelters and water score higher (weights tuned); ties go by the seed.
    - **Found again after settling:** the search runs once more on the settled world (`WLD-08`), so the bands start from what is really there.
    - **The bands' places:** each band gets one shelter as its home base, and its home range is the land around it that feeds it, next to its neighbours' (`BIO-20`).

- `WLD-11` **Generation time** *(Decided)*: Generating the candidate worlds and finding the best three takes a few minutes in total on the phone.
  - **How it works:**
    - **Only the best get full detail:** the two passes of `WLD-10` keep most of the work on the few worlds that might be chosen, and nothing is made down to the metre in advance (`WLD-08`).
    - **All cores:** candidates are made side by side on all the phone's cores.
    - **Fewer worlds, never less detail:** if a milestone's measurement (`PLT-04`) shows generation running past a few minutes, fewer candidates are made; each world keeps its full detail.
    - **Settling** the world you pick (`WLD-08`) comes after this and takes as long as the world's own rules need; its time is measured too (`PLT-04`).

### 6.4 Detail

- `WLD-12` **Detail where it matters** *(Decided)*: Each system runs at the coarsest scale that keeps it true.
  Climate is worked out region by region; rivers and soils kilometre by kilometre; plants and animals in patches of a few hundred metres.
  Everything goes down to the metre where people are, or where something new or critical is happening.
  Where you look changes only the picture, never the simulation (`WLD-13`).
  Follows from `PRN-11`.
  - **How it works:**
    - **Levels:** each system keeps its state at its own level of the nested cells (`WLD-01`):
      - **weather:** cells of about 8 km (tuned), small enough for storms about 50 km across (`WLD-30`); the weather at any smaller place comes from its cell by physical rules: air cools with height, cold air pools in hollows, slopes facing the sun warm faster, and wind drops in shelter;
      - **water and soils:** rivers, lakes and ground water on cells of about 1 km, with each soil's makeup and slow change there too; the water and nutrients that plants draw on are kept per plant patch;
      - **plants:** patches of about 250 m, each holding every species present with the amounts of its leaves, wood, roots, flowers, fruit and seed, and the ages of its trees;
      - **animals:** each species' animals counted per patch, whole animals only, by age and sex, and moved between patches each day by where food, cover and danger are; river life per stretch of river, and sea life per cell of a few kilometres, with shores and shellfish beds in patches like the land;
      - **the metre:** single things: stones, plants, animals as individuals, and the shape of the ground.
    - **Steps:** each level moves by its own rates on one world clock: weather by the hour, rivers by the day and by the hour in floods, plants in steps as short as their changes need (days in a spring flush, weeks in winter), animal counts by the day, and single beings and things in fine steps while they act.
      Grid levels are worked out many cells at once, on the graphics chip where that helps (`PLT-01`).
    - **Where people are:** what a person's senses can reach is real at metre detail.
      Things are made from the seed as senses reach them, large ones far off and small ones only close, by what each sense could pick out at that distance (`BIO-18`).
      The ground takes its metre shape wherever bodies use it: where they walk, sit, climb, dig or build, and in the shelters they live in.
      Anything touched is stored from then on (`MAT-10`).
    - **Animals near people:** within the distance its species covers in a day of any person (estimated from body size and diet), an animal of a kind people meet one at a time (larger mammals and birds, and anything that can hurt a person; set per species, estimated) is an individual with a mind (`MND-16`), drawn from its patch's count with its own traits from the species' spread.
      It rejoins the count when no person is near, its learned wariness of people passing into the population's traits.
      Any animal that has dealt with people (hunted, wounded, fed, tamed, or known to someone) stays an individual for good.
      Other animals stay counted, and one is made on its own only when something acts on it, as with matter: a fish caught, a grub dug up.
    - **Something new or critical:** detail also goes down to the metre, near people or not, wherever a process depends on finer detail than its level: a fire spreading, water breaking out of its channel, ground giving way, lava and ash, a lightning strike.
      It starts with the event and ends when the event does, and the results are written back to the coarser levels.
    - **Between levels:** the coarser level always holds the totals.
      When detail starts, single things are made from the current amounts and the seed; whatever is taken, eaten, killed, cut or burned at metre detail is taken off its patch at once.
      Plants touched by people merge back into their patch once untouched for a season, keeping their place and identity from the seed, so the same tree is found again, grown or gone as its patch's amounts say.
    - **The rule reads only the world:** what sets the level is people, animals and events, never the camera (`WLD-13`).
    - **Checked:** each level is run in sandboxes against a finer one, and the same area run both ways must give the same statistics (`PRN-11`), such as plant amounts, animal numbers and how far fires spread.

- `WLD-13` **Looking changes nothing** *(Decided)*: Where you look never changes what happens.
  Fine detail drawn for the picture is generated the same way every time, and never contradicts what was simulated.
  Follows from `PRN-10`.
  - **How it works:**
    - **One way only:** the picture and sound read the simulation; nothing in the simulation reads the camera, the zoom, or anything made only for the picture.
    - **The same generator:** detail made for the picture where the simulation has none yet, such as ground, stones and plants close up, comes from the seed and the current coarser state by the same rules the simulation uses (`WLD-12`), so what you saw is what people will find.
      It is never stored and never read back.
    - **Animals that are counted:** where animals are counted rather than individuals (`MND-16`), the picture shows those real animals in the patches they are in, by age and sex.
      Where each stands within its patch, and how it moves there, are filled in from the seed, and no event is shown that the counts didn't have (`PRN-10`).
    - **Speed changes nothing:** step lengths come from the world's own rates, never from how fast time runs or how busy the phone is (`PRN-11`).
      Fast or slow, only the batching changes: up close the steps run in small slices between frames, and at speed in large batches, with the same results.
  - **Check:** the same saved state, run with the camera in different places and at different speeds, gives the same results bit for bit on the same phone (`TIM-16`); and a code check finds no path from the picture's data into the simulation.

### 6.5 Natural systems

- `WLD-29` **Systems feed each other** *(Decided)*: All the natural systems below are simulated in depth, and each feeds the others: weather shapes soils and plants, plants feed animals, fire and floods change the land, and people come to change them all (`WLD-25`).
  - **How it works:**
    - **Shared places:** every system reads and writes the same cells and patches (`WLD-12`), so what one changes, the others read at their next step.
    - **What feeds what:**
      - weather feeds water (rain, snow, melt, evaporation), soils (wetting, freezing), plants (warmth, light, water, frost), animals and people (heat, cold, snow cover) and fire (dry fuel, lightning, wind);
      - water feeds soils and plants (wetness, flooding), the land (cutting and settling) and the weather (water returned to the air);
      - soils feed plants (water and nutrients) and decide what buried things keep (wetness, air, acidity: `MAT-08`);
      - plants feed animals (food, cover), soils (fallen leaves and roots, with their nutrients), water (what roots draw up and leaves give off), the weather (how much sunlight the land reflects and how much water it returns to the air) and fire (fuel);
      - animals feed plants (grazing, trampling, spreading seed, pollinating, dung), other animals (hunting, competing) and microbes (hosts, carcasses);
      - microbes feed soils (rot releasing nutrients), every living thing (disease) and stored food (rot, fermenting) (`WLD-21`);
      - fire feeds plants (burned, then regrowth), soils (ash, and bare ground that erodes), the air (smoke) and animals (killed or driven off);
      - changes to the land (erosion, slides, floods, quakes, eruptions) feed everything where they happen.
    - **Nothing lost:** water, carbon, nutrients and heat pass between systems with their elements and energy counted (`MAT-09`): the nitrogen in grass eaten by a deer goes into the deer, its dung and in time its carcass, and back to the soil.
    - **Order:** within a step the systems run in a fixed order, and a slow system takes the faster ones' totals over its step, such as the week's warmth and water for plants, so nothing reads a value from the future.
    - **People are one more feeder:** what people do at metre detail goes into the same cells and patches (plants cleared, land burned, earth dug, animals killed), so its effects travel the same links (`WLD-25`).

- `WLD-14` **Geology and materials** *(Decided)*
  - **What:** Rocks, minerals, soils and ores lie in realistic places, so what can be discovered depends on what's underfoot.
  - **Example:** Flint comes out of chalk and limestone, obsidian near volcanoes, copper ores in certain mountains, clay along rivers, salt in dry basins.
  - **How it works:**
    - **What lies at the surface:** each place's surface comes from its rock layers (`WLD-09`) and what has happened above them: bare rock where slopes are steep or soil is thin; soil elsewhere (`WLD-27`); loose blocks where frost and roots break rock along its cracks, sized by how far apart the cracks are; scree below cliffs; and gravel, sand and mud where water or ice left them.
    - **Stones travel:** a river's gravel holds stones from every rock upstream, in proportion to how much of each is exposed and how well it resists wear, rounded and sorted by size with distance; beaches take theirs from nearby cliffs and rivers, and rubble left by ice comes from wherever the ice came from.
      So flint from chalk hills turns up in valley gravels far away, and tin in the gravels below granite.
    - **Every piece differs:** each rock in the settings catalogue is a set of ingredients and a structure (`MAT-01`, `MAT-02`) with real ranges of grain, flaws and impurities, and each piece draws its own within them, so some flint breaks better than other flint.
    - **Seen through layers:** layers show wherever something cuts through them: river banks, cliffs, cave walls, landslide scars, the roots of fallen trees, and burrows; beneath the soil, digging reaches them (`MAT-12`).
    - **Ores show themselves as they really do:** by colour (green and blue copper minerals, red and yellow iron), by weight for their size, by sheen, and by collecting in gravels because they are heavy; they are perceived through the senses like anything else, never labelled (`PRN-07`).
    - **Salt and clay:** salt forms crusts where closed lakes dry (`WLD-17`), and seeps out at salty springs where ground water passes through salt layers; clay settles where water stands still, in floodplain hollows, old river bends and lake beds, and forms in place where feldspar-rich rock weathers.

- `WLD-15` **Living geology** *(Decided)*: Change continues during play.
  Erosion wears the land, rivers shift their course, landslides fall, earthquakes strike along faults, volcanoes erupt, and coastlines move as the sea rises and falls.
  - **How it works:**
    - **Erosion:** the same law as in generation (`WLD-09`): water cuts faster where more of it flows, where it is steeper and where the rock is softer, and slopes creep down.
      Bare ground wears away many times faster than ground under plants (measured ranges), so a burned or cleared slope loses its soil in a few heavy rains.
      It is worked out on the 1 km cells each year and after heavy rain, and on patches where the plant cover has been stripped.
    - **Rivers shift:** each river's channel is kept as a line with a width, finer than its cell.
      The outside of each bend wears back and the inside builds up at measured rates, so bends wander, and a flood can cut through a narrow neck and leave an old bend as a lake.
      Where a channel has built itself up above its floodplain, a flood can break out and take a new course.
    - **Landslides:** a slope fails when the pull down it passes its strength, which falls as the ground fills with water; heavy rain, melting snow, shaking, or a river or people cutting away its foot can set one off.
      Only slopes steep enough ever to fail are checked, and only after such a trigger.
      The moving ground runs out until the slope eases, burying what lies below, and can dam a river into a lake that may later burst (`WLD-17`).
    - **Earthquakes:** each fault from generation builds strain at the speed its plates move, and slips when the strain passes the fault's strength, which varies from the seed.
      The length that slips sets the size, and shaking fades with distance, both by measured rules.
      Shaking pulls on everything joined or stacked, so built things fall by their joints' strength (`MAT-10`), slopes give way, and a fault under the sea raises a wave (`WLD-26`).
    - **Volcanoes:** each volcano from generation fills with molten rock at its own rate, and erupts when the pressure passes its limit, sized by how much has built up (measured eruption sizes).
      Runny lava pours out and flows downhill at speeds set by its stiffness and the slope, cooling as it goes (`MAT-04`); sticky, silica-rich lava blows out ash and glowing flows, and leaves domes and obsidian.
      Ash rises with the eruption's size, drifts with the winds of the day, and falls thinner with distance; the largest eruptions put enough gas into the air to cool the world for a few years (`WLD-16`).
      Swarms of small quakes, swelling ground, gas and warmer springs come before, with measured lead times of days to months.
    - **Coasts move:** the sea's level follows the ice on land (`WLD-26`), and each year the coastline is wherever the land lies below it.
      The rising sea drowns shores, kills plants with salt and covers what lay there (`MAT-08`); land freed of ice rises slowly at measured rates; cliffs wear back and spits grow by the waves.

- `WLD-27` **Soils** *(Decided)*: Soils form from rock, climate, plants and time.
  They hold water and nutrients, decide what grows where, and can later be enriched or exhausted by people.
  - **How it works:**
    - **What a soil holds:** its depth and layers; the shares of sand, silt and clay; stones; dead plant matter; nutrients (nitrogen, phosphorus, potassium, calcium and others) as amounts of each element, some free for roots and some bound; acidity; water; and temperature (`WLD-12`).
    - **Water:** each day, rain and melt soak in up to what the soil can take, the rest runs off to the rivers (`WLD-17`), plants draw water up, the surface dries, and any extra drains down to the ground water.
      How much a soil holds and how fast it drains come from its sand, silt and clay by measured rules.
    - **Nutrients:** plants take them up as they grow; fallen leaves, dead roots, dung, carcasses and ash return them; microbes free them from dead matter, faster when warm and moist (`WLD-21`); rain washes some away, most in wet climates and sandy soils; weathering rock adds a little each year; and some plants, and lightning, add nitrogen from the air.
      Every element is counted (`MAT-09`).
    - **Acidity:** it rises as rain washes out calcium and under some plants' litter, and falls with ash or lime; it decides which nutrients roots can reach and which plants thrive.
    - **Slow change:** dead matter builds up where plants grow and is lost where they are cleared; soil deepens as rock weathers below and thins as erosion takes its top (`WLD-15`); this is worked out each year.
    - **People enrich or exhaust it through the same flows:** crops carried away take their nutrients with them, so a plot used year after year yields less; dung, ash and rotted waste put nutrients back; trampled ground lets less water in; cleared ground erodes.
      There is no fertility score.
    - **What grows where:** each plant grows by its access to water, nutrients, warmth and light, against its own needs and limits (`WLD-18`).

- `WLD-16` **Climate and weather** *(Decided)*
  - **Climate from geography:** Each place's climate (rain, temperature and winds through the seasons) is worked out by rules derived from real physics and calibrated to Earth, not by a full physical climate model: latitude, height, distance from the sea, prevailing winds, and mountains that block rain.
  - **Daily weather** is drawn from that climate, with storm systems that move across the land.
  - **Long cycles:** ice ages and warm periods follow real cycle lengths, tens of thousands of years long, moving coastlines and pushing migrations.
    Worlds begin as an ice age ends, so seas rise over the first ten thousand years or so and can cut bands apart (`MOM-05`).
    A great eruption can cool the world for a few years.
  - **How it works:**
    - **What drives it:** for each weather cell (`WLD-12`) and day of the year, rules from physics, calibrated to Earth, set the sunlight (from latitude and the date, `WLD-01`); the wind belts at Earth's latitudes, shifting with the seasons and bending around land and sea (warm land in summer draws in moist sea air, cold land in winter pushes dry air out); and the storm belts, where storms form and the paths they take, strongest where warm and cold air meet, with tropical storms only over seas warmer than a measured temperature.
    - **Storms are drawn from the climate:** storm systems are drawn from their belts' real statistics (how often, how big and how fast, with sizes scaled, `WLD-30`), and move along their belts.
    - **Weather is worked out hour by hour:** each cell keeps its air's temperature, moisture, cloud, wind and pressure.
      Air moves with the belts and the storms; it takes up water from seas, lakes, wet ground and plants by measured rules; it cools as it rises over hills or in storms, and rain or snow falls once it cools past what it can hold.
      The sun warms the ground by day by how much light the ground takes in (snow and sand reflect most, forest least), and the ground cools at night, most under clear skies; fog and frost form where moist air cools at night, most in hollows and over water.
      So what storms bring, and all other weather, comes from the land and water as they are now, which is how forests, lakes, snow and people's clearing change it (`WLD-25`).
    - **Thunderstorms** build where the air is warm, moist and rising, and give lightning (`WLD-28`).
    - **Climate is the weather's long-run average:** checked against Earth in calibration (`WLD-08`), such as each kind of climate's share and where it lies.
    - **Long cycles:** the slow wobble of the world's tilt and orbit, drawn from the seed within Earth-like ranges, changes how much summer sun high latitudes get.
      Where winter snow outlasts the summer, ice sheets grow and reflect more sunlight, and the air's carbon dioxide falls and rises with the ice by the relation measured in Earth's ice cores, deepening each swing.
      Why Earth's ice ages keep the rhythm they do is not fully understood, so the timing follows the orbit and the feedbacks are tuned until the ice and the sea match the size of Earth's record (`WLD-26`).
    - **Eruptions:** gas from a great eruption dims the sun for a few years through the same sunlight balance (`WLD-15`).
  - **Example:** Rain clouds coming off the western sea drop their rain on the mountains, so the valleys beyond are dry grassland with forest only along the rivers.

- `WLD-05` **Climate on a small world** *(Decided)*: Climate zones sit closer together than on Earth, a few days' walk apart, and weather systems are scaled to fit the world (`WLD-30`).
  - **How it works:** latitude changes about 20 times faster per kilometre than on Earth, about one degree every 5.6 km, so the belts the climate rules give (`WLD-16`) are a few days' walk wide.
    Storm systems, fronts and the great loops of wind and current are scaled (`WLD-30`); local weather, such as thunderstorms, sea and valley breezes and frost hollows, keeps its real size.

- `WLD-25` **People change the climate** *(Decided)*: What covers the land and, much later, fuel burned at scale feed back into the climate through the same physics.
  Clearing a forest can dry a region; centuries of burning could warm the world.
  - **How it works:**
    - **Land cover:** each weather cell reads, at every step, how much sunlight its land reflects, how much water its plants and soil give back to the air, and how rough its surface is, from the patches beneath it (`WLD-29`).
      Clearing a forest makes the land brighter, drier and smoother, and downwind gets less of the water the forest gave back, so the region can dry by itself, with no rule for it.
    - **The air's carbon:** everything that burns, rots or breathes gives off carbon dioxide, and growing plants take it in (`MAT-09`); it mixes through the world's air, and the sea takes some up slowly at measured rates.
      More of it warms the world by the measured amount for each doubling, through the same sunlight balance (`WLD-16`), so centuries of burning at scale would warm the world.

- `WLD-17` **Fresh water** *(Decided)*: Rivers, lakes, wetlands, springs, underground water, ice and floods.
  Life and settlement gather around them.
  - **How it works:**
    - **Each cell's water, day by day:** rain and snow come from the weather; snow lies until warmth melts it; water soaks into the soil or runs off (`WLD-27`); soil water drains to the ground water, which seeps slowly downhill through the rock, fast through cracked limestone and caves and slowly through clay (measured ranges).
    - **Springs:** ground water comes out where it meets the surface, at the foot of slopes and where water-bearing rock lies on rock that holds it back; springs and seeping ground keep rivers flowing in dry seasons.
    - **Rivers:** each cell passes its water downhill along the network from generation, at the speed its slope and channel allow (measured rules), so a storm's water reaches the lower valley later, as a flood wave.
      Each stretch's channel is as wide and deep as its usual flow makes it (measured rules); when the flow passes what the channel holds, the water spreads over the floodplain, worked out at finer detail while it lasts (`WLD-12`), leaving silt and drowning or carrying things.
    - **Lakes:** a lake rises with what flows in and falls with what flows out over its outlet and what evaporates; a lake with no outlet in a dry land turns salty and leaves salt where it dries (`WLD-14`).
    - **Wetlands:** where ground water stays at the surface, on flat or badly drained ground and below springs, soils stay waterlogged, reeds and sedges grow, and peat builds up and keeps what falls into it (`MAT-08`).
    - **Ice:** snow that outlasts the summer builds glaciers, which flow downhill at rates set by their thickness and slope (a measured law) and melt at their ends, feeding rivers in summer.
      Lakes and rivers freeze when cold enough, and the ice bears a person once it is thick enough (measured thickness); in the coldest places the ground stays frozen all year.
    - **Floods** come from heavy rain, fast melt, ice jams, or a burst dam of rubble or ice, all from the same water balance.
    - **What the water carries:** salt, mud, warmth, and microbes from dung and waste upstream (`WLD-21`), which decide whether it is safe to drink (`BIO-05`).
    - **Gathering around water:** nothing is placed there; plants grow better where water is, and animals and people go to it because their bodies need it each day (`BIO-09`).

- `WLD-26` **Seas** *(Decided)*: Oceans with currents that carry heat and moisture, tides set by the moons and the sun (so worlds without moons still have weaker tides), and a sea level that rises and falls with the ice ages.
  At low tide, shellfish beds are exposed on the shore.
  - **How it works:**
    - **Currents:** worked out from the winds and the shapes of the seas, and again when either changes, such as when a rising sea opens a strait (`WLD-16`).
      Great loops of current turn with the winds; warm water flows poleward along one side of each sea and cold water returns along the other; and where wind pushes surface water away from a coast, cold water rich in nutrients rises.
      Each sea cell keeps its temperature, saltiness, nutrients and current, and passes heat and water to the weather above it, so coasts by warm water are mild and wet.
    - **Tides:** worked out each hour from where the sun and moons are (`WLD-07`); each body's pull sets its share, so tides swell when they line up and ease when they don't, and several moons make a richer pattern.
      Heights are Earth's, scaled by each body's pull relative to Earth's moon and by the shape of each coast: a named exception (`PRN-05`), since a planet this small with Earth's gravity is not real physics, so its tides can't be worked out from it.
      With no moon, the sun alone gives about a third of the range of Earth's highest tides.
    - **Sea level:** set by how much water is locked up as ice on land (`WLD-16`), within Earth's measured range between ice ages and warm times; coastlines follow (`WLD-15`).
    - **The shore:** each shore patch has its height against the sea, so each hour it is either under water or bare; shellfish beds can be reached only while bare, so gathering follows the tides.
    - **Sea life:** fish and sea mammals per sea cell (`WLD-12`), most where cold, rich water rises and in shallow seas; shellfish and seaweed on the shore patches.
    - **Storms and waves:** storm winds raise waves and surges that flood low coasts, and quakes under the sea raise great waves (`WLD-15`).
    - **Sea ice:** cold seas freeze in winter, thick enough in places to walk on; the polar ice never melts (`WLD-01`).
    - **Salt water** can't be drunk safely (`BIO-09`), and leaves salt where it dries.

- `WLD-18` **Ecology** *(Decided)*
  - **What:** Plants grow, flower, fruit and die back with the seasons.
    Animals eat, breed, migrate and die.
    Everything is tied together in food webs, with populations that boom and crash.
  - **Why:** It is what people live from, and what they will one day change.
  - **How it works:**
    - **Plants grow by what they catch and draw:** each step, each species in a patch (`WLD-12`) grows by the light its leaves catch, with taller plants shading shorter ones, limited by water, nutrients and warmth against its needs (`WLD-27`), at measured rates for its family (`WLD-19`).
      The growth goes to leaves, wood, roots, stores, flowers and seed by the species' own rules for its age and the season.
    - **Seasons come from warmth and day length:** each species leafs out, flowers, fruits and dies back when its sums of warmth and the day's length reach its own thresholds, so a cold spring delays everything and a warm one brings fruit early.
    - **Seed:** flowers set seed when pollinated, by the wind or by the pollinating insects in the patch (`WLD-23`); seed is carried by wind, water and the animals that eat it, and waits in the soil until warmth, wetness, light or fire lets it sprout.
    - **Plants die** of drought, frost beyond their limits, fire, shade, being eaten past recovery, disease and old age; dead matter falls as litter, rots (`WLD-21`), returns its nutrients (`WLD-27`) and is fuel (`WLD-28`).
    - **Animals live by the same body rules as people:** each day, each animal needs food energy and nutrients by its body size and activity (`BIO-09`), and eats what its diet allows from its patch, sharing it with every other eater there; what is eaten comes off the patch.
      Fed animals build fat and hungry ones burn it; cold and deep snow cost energy and bury food.
    - **Moving:** counted animals move between patches each day toward food, water and cover and away from hunters, by their species' rules and what the population has learned (`MND-16`); herds move together, and migrations follow the seasons' food along routes the population has learned.
    - **Breeding:** in its season, set by day length and warmth, each species breeds at its measured rates, with more young from females in good condition; young grow and mature at the species' measured ages.
    - **Hunting:** hunters meet prey at rates set by both their numbers in a patch and how well the prey escapes, by measured rules; each kill is a whole animal, decided by chance from the rate, and the carcass feeds scavengers and then rots.
    - **Every death has a cause:** hunger, a hunter, disease, cold, drowning, fire, old age or people, and each is counted with its cause, as for people (`BIO-14`).
    - **Booms and crashes:** nothing sets them; they come from these rules and the weather, as in the example.
    - **Checked:** in sandboxes, each kind's numbers fall within real densities for its habitat, and hunters and prey keep real ratios (`RES-14`).
  - **Example:** A run of mild winters lets the deer multiply; the wolves follow; then a hard winter cuts both down, and the hunters go hungry.

- `WLD-28` **Fire in the landscape** *(Decided)*: Lightning and dry fuel start wildfires, which spread with wind and slope; landscapes regrow after them, and some plants depend on fire.
  People can learn to use fire on the land.
  - **How it works:**
    - **Fuel:** each patch's fuel comes from its plants (`WLD-12`): dead grass, fallen leaves and twigs, dead wood and living foliage, each with its amount and wetness.
      Dead fuel dries and wets with the weather, fine fuel within hours and logs over weeks (measured rates); living foliage's wetness follows the season and the soil water.
    - **Lightning:** strikes come from thunderstorms (`WLD-16`), each landing by chance, more often on high ground and tall trees; it injures or kills what it hits (`BIO-13`) and lights fuel dry enough to catch (`MAT-04`).
    - **Starting:** any heat source lights fuel the same way: lightning, lava, embers blown from a fire, or a fire people left.
    - **Spreading:** while a fire burns, its area is worked out at finer detail (`WLD-12`).
      It spreads from cell to cell at a speed set by the fuel's amount, size and dryness, the wind and the slope, by the heat that reaches unburned fuel ahead of the flames, calibrated by measured fire spread.
      Wind throws embers ahead to start new fires, and fire climbs into tree crowns where low branches and wind let it (measured thresholds).
      It stops where fuel runs out, rain falls, or it meets water, bare rock or burned ground.
    - **After fire:** each patch loses plants according to how hot the fire was; ash returns nutrients (`WLD-27`), and bare ground erodes until plants return (`WLD-15`).
      Regrowth follows each species' fire traits (`WLD-19`): some sprout again from their roots, some seeds sprout only after heat or smoke, some cones open only in fire, and grasses return within weeks; grazers come back to the new growth.
    - **How often places burn** is a result of their climate, lightning and fuel, checked in calibration against Earth's measured fire intervals for each kind of landscape (`WLD-08`).
    - **People:** a fire people start spreads by the same law; whether they ever burn land on purpose is up to their minds (`PRN-01`).

- `WLD-20` **Heredity in plants and animals** *(Decided)*: Inheritance continues during play, so adaptation and domestication (wolves into dogs, wild grasses into grain) can happen on their own.
  - **How it works:**
    - **Traits are numbers:** body size, growth, tolerances, seed size, whether seeds stay on the stalk, poison doses, boldness and fear, breeding season, and the rest of each species' traits (`WLD-19`).
    - **Individuals inherit from their parents:** an animal or plant that is an individual (near people, or touched by them, `WLD-12`) carries its own traits.
      Its young get their parents' average plus variation, by each trait's measured share that is inherited; traits set by one or two genes in Earth's species, such as seeds that stay on the stalk or coat colour, pass by Mendel's rules.
    - **Counted populations inherit as a whole:** each keeps, per weather cell, the average and spread of each trait and the share of each gene.
      Who dies and who breeds is decided by the same body rules across the spread, such as cold killing more of the small, and the next generation's average shifts by the breeder's rule: the measured share that is inherited, times the difference between the parents and the whole.
      Animals moving between cells mix their traits, and an individual rejoining its count adds its traits to the average.
    - **New variation:** mutation adds a little new spread each generation at measured rates, so traits can move beyond their starting range over long times.
    - **Selection is only what happens:** no one writes a fitness rule; survival and breeding come from cold, hunger, hunters, people and the rest of the world.
      So bolder wolves that eat at the edge of camp raise more pups near people (`MOM-06`), and grass seeds that stay on the stalk are the ones gathered by cutting and carried home, so if people sow saved seed, that gene spreads.
    - **Real speed:** change takes as many generations as it takes; only the experiment dial speeds it up (`BIO-07`, `PRN-12`).

- `WLD-21` **Microbes** *(Decided)*: Rot, fermentation and disease are living microbes that spread and evolve.
  Crowding, and living close to animals, bring epidemics.
  - **How it works:**
    - **Kinds:** microbes come from Earth families (`WLD-09`): decomposers in soil, litter and carcasses; fermenters on fruit, grain and milk; and about 20 kinds of disease (tuned), several for each way in (a wound, food or water, breath, touch, a bite) and for the main animal hosts.
    - **Where they live:** as amounts on things (each piece of food or dead matter carries its load, `MAT-10`), in each patch's soil and litter, in each stretch of water, and in each infected body.
    - **Rot and fermenting are laws over time** (`MAT-04`): microbes on food or dead matter grow at measured rates set by warmth, wetness, air, salt and acidity, and turn its ingredients into others, balanced (`MAT-09`): sugars into acids, alcohol and gas, flesh into rot.
      So meat rots fast when warm and wet and slowly when cold, dry, salted or smoked, and crushed fruit ferments (`RCK-07`, `RCK-09`, `RCK-14`).
    - **Disease in a body:** a germ gets in by its way in; if the dose beats the body's defences (immunity, condition, age), it multiplies at its rate and does its harm until the body clears it, it kills, or it stays (`BIO-05`); those who recover stay immune for the germ's measured time.
    - **Spread:** a sick body sheds the germ by its way out: into the air nearby, into water and soil with its dung, onto what it touches, or into the insects that bite it.
      Others catch it only from where they really are and what they really do, such as sharing a shelter, drinking downstream or butchering a carcass, never from an assumed contact rate.
    - **From animals:** germs that live in animals reach people through bites, meat, dung and shared water, more often the closer and more often people deal with animals.
    - **Crowding:** a germ that kills fast or leaves lasting immunity runs out of new hosts in small, scattered bands and dies out; only a large, close-packed population keeps it going, so epidemics arrive with crowding by themselves.
      This is checked against real figures for the population a disease needs to persist (`RES-14`).
    - **Evolving:** each germ's traits, such as how easily it spreads, how harmful it is, how long immunity lasts and which hosts it can live in, vary and are inherited with mutation as in `WLD-20`, over many generations a day, so the strains that spread best take over.
      A strain that gains a new host can jump from animals to people; there is no measured rate for such jumps, so it is estimated and tuned.
    - **Counted animals:** in counted populations, the sick, the recovered and the dead are counted per patch (`WLD-12`).

- `WLD-22` **Natural disasters** *(Decided)*: Eruptions, earthquakes, floods, droughts, storms, wildfires and lightning come from the world's own systems, not only from you.
  Follows from `GOD-05` and the natural systems in this section.
  - **How it works:** no disaster is ever scheduled or drawn as a disaster: each is the far end of its own system.
    Eruptions and earthquakes come from pressure and strain (`WLD-15`); floods from the water balance (`WLD-17`); droughts from runs of dry weather when storms keep to other paths (`WLD-16`); storms from the storm systems, whose winds fell trees and wreck built things by force (`MAT-11`); wildfires and lightning from fuel and thunderstorms (`WLD-28`).
    Your powers act through the same systems (`GOD-05`).
    How often each comes is a result, checked in calibration against Earth's records, such as lightning strikes per square kilometre in each climate and how often rivers flood (`WLD-08`).

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
      Tools, art, graves and hearths always stay individual (`MAT-08`).

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
      Each error comes from that being's own chance at that moment (`TIM-16`).
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
  - **How it works:** each entry is readable text with its values and their sources, and the simulation loads the catalogues when a world starts; nothing in them is code.
    - **Ingredients:** elements and measured values (`MAT-01`).
    - **Structures:** each form, and how it changes properties (`MAT-02`, `MAT-03`).
    - **Laws:** each law's plain statement, the properties it reads, its data, and the code that computes it, which never names a product (`MAT-04`).
    - **Reality checks:** each check's setup, its real range and its "must not" partner (`MAT-17`).

- `MAT-14` **Adding without rewriting** *(Decided)*
  - **What:** Adding an ingredient, structure, law or check never requires changing the others.
  - **Why it works:** Laws never name products (`PRN-07`), so a new ingredient automatically takes part in every existing law.
  - **How it works:** laws find what they act on by reading properties, so a new ingredient takes part in every law whose conditions it meets, and a new law acts on every existing ingredient.
    Adding an entry changes no other entry.
  - **Example:** Adding tin ore needs no new smelting rule; the smelting law already covers it.

- `MAT-15` **Every addition proves itself** *(Decided)*: Each new entry comes with the reality checks it must pass, and the whole checklist runs again, so nothing that worked before breaks.
  - **How it works:** each entry names the checks it supports; on every change, the whole checklist runs automatically before the work can join the main version (`PRC-10`), and a failing check blocks it.

- `MAT-16` **Matter grows in layers** *(Decided)*: Matter is built in layers, in this order, each added without rewriting the earlier ones.
  The implementation plan sets when each arrives:
  1. stone, wood, bone and water;
  2. heat and fire;
  3. food and the body's chemistry;
  4. fibres, hides and joining;
  5. clay, lime and pigments;
  6. metals and glass;
  7. further layers as experiments call for them.
  - **How it works:** each layer is a set of ingredients, structures, laws and checks; its checks switch on when it is added (`MAT-17`), and later layers only add entries (`MAT-14`).

- `MAT-17` **How reality checks work** *(Decided)*: Each check has a real-world range and, where it helps, a "must not" partner, such as "green wood doesn't light by friction".
  Results are judged by properties, not names: leather is hide that stops rotting and stays supple.
  A check becomes active once its layer is built (`MAT-16`).
  This file keeps the checks that define what the world must do; the catalogues hold the rest, each naming the item it supports (`MAT-13`).
  - **How it works:** each check is a small sandbox built from catalogue entries, whose things are defined by their makeup and structure, never by name.
    It applies fixed actions and conditions, such as a strike at a set energy or a fire with set fuel and air, and runs many times with different chance draws.
    The measured result must fall within the real range, and the "must not" partner must never happen.

### 7.5 Reality checklist

The physics must reproduce these without any rule written specially for them (`PRN-07`).
How the checks work is set out in `MAT-17`, and they all run again whenever anything changes (`MAT-15`).
Each check's exact numbers are sourced when its layer is built (`PRN-05`).

**Core**

- `RCK-01` **Flint chips, granite doesn't** *(Decided)*: Flint and obsidian chip into sharp flakes; granite doesn't.
  - **Check:** 1,000 hammerstone strikes each, at a spread of strengths and angles, on glassy, very fine-grained and coarse-grained stone: the fine and glassy stones give thin flakes that cut hide, in the share of well-placed strikes found in knapping experiments; the coarse stone gives grit and chunks, and never a flake that cuts.
- `RCK-02` **Fire by friction** *(Decided)*: Rubbing wood fast enough can light dry tinder.
  - **Check:** a dry stick twirled on dry soft wood, at the speeds and pressures hands can manage, over dry tinder, gives an ember within the time range found in experiments; green or wet wood never does.
- `RCK-03` **Cooking helps** *(Decided)*: Cooking makes food more nourishing.
  - **Check:** the same roots and meat, raw and cooked, eaten by a body: cooked food yields more usable energy, by the measured margin (`BIO-10`).
- `RCK-04` **Pottery needs fire** *(Decided)*: Fired clay becomes pottery; sun-dried clay softens again in water.
  - **Check:** sun-dried clay softens and falls apart in water; the same clay heated past its measured firing temperature, for long enough, stays hard in water.
- `RCK-05` **Lime** *(Decided)*: Burned limestone becomes lime.
  - **Check:** limestone heated past its measured breakdown temperature turns to quicklime and gives off gas; below that temperature, nothing changes.
- `RCK-06` **Leather** *(Decided)*: Hides soaked with oak bark become leather instead of rotting.
  - **Check:** hides soaked for weeks with crushed oak bark stop rotting and stay supple when dried; hides soaked in plain water rot.
- `RCK-07` **Fermentation** *(Decided)*: Fruit sugars ferment.
  - **Check:** crushed sweet fruit left warm with wild yeasts turns sugar into alcohol over days, at real rates; boiled and sealed, it does not.
- `RCK-08` **Copper needs a furnace** *(Decided)*: Copper smelts in a charcoal furnace with forced air, but not over a campfire.
  - **Check:** copper ore in an open fire never yields liquid copper; in charcoal enclosed by clay or stone and blown through tubes it does, but only once the fire law's temperature passes copper's melting point (`MAT-04`, `MAT-07`).
- `RCK-09` **Rot** *(Decided)*: Untreated meat and hides rot, faster when warm and wet.
  - **Check:** meat and hide rot at measured rates: within days when warm and wet, far slower when cold or dry.
- `RCK-21` **Floating** *(Decided)*: A dry log floats; a stone sinks.
  - **Check:** a dry log floats, sitting as deep as its density says; a stone sinks.
- `RCK-22` **Air feeds fire** *(Decided)*: Blowing on embers makes them hotter.
  - **Check:** blowing on embers at a person's measured breath rate raises their temperature within the real range; smothering them lowers it.

**Early crafts and food**

- `RCK-10` **Heat-treated flint** *(Decided)*: Flint gently heated in a fire chips more easily and more predictably.
  - **Check:** flint heated slowly into its measured range and cooled slowly needs less force per flake and gives more regular flakes; heated fast or too hot, it cracks.
- `RCK-11` **Cord** *(Decided)*: Plant fibres twisted together make cord far stronger than the single fibres.
  - **Check:** twisted plant fibres hold several times the load of the same fibres laid side by side, as measured.
- `RCK-12` **Glue from bark** *(Decided)*: Birch bark heated without air gives a tar that glues a stone point to a shaft.
  - **Check:** birch bark heated without air, within its measured range, gives tar that holds a stone point to a shaft under a real pull; heated in open air, it only burns.
- `RCK-13` **Leaching** *(Decided)*: Soaking in running water draws the bitterness out of acorns.
  - **Check:** crushed acorns in running water lose their bitterness below the level that stops a body eating them, within real times; in still water it takes longer.
- `RCK-14` **Keeping meat** *(Decided)*: Salting, smoking and drying make meat keep far longer.
  - **Check:** salted, smoked or dried meat stays edible many times longer than fresh meat at the same temperature.

**Colour and art**

- `RCK-15` **Ochre turns red** *(Decided)*: Yellow ochre turns red when heated.
  - **Check:** yellow ochre heated past its measured change temperature turns red; below that temperature, it stays yellow.
- `RCK-16` **Paint that lasts** *(Decided)*: Charcoal and ochre mixed with fat or water make paint that lasts on rock.
  - **Check:** charcoal or ochre mixed with fat or water and spread on rock stays visible for years where sheltered, weathering at real rates; on exposed rock it fades faster.

**Later crafts**

- `RCK-17` **Bronze** *(Decided)*: Copper with a little tin is harder than copper.
  - **Check:** copper with about a tenth tin is measurably harder than copper.
- `RCK-18` **Iron** *(Decided)*: Iron needs a hotter, longer charcoal fire than copper and comes out spongy; it must be hammered to make it useful.
  - **Check:** iron ore in a blown, enclosed charcoal fire, run hotter and longer than copper needs, gives a spongy lump mixed with waste; hammering it hot squeezes the waste out; it never runs liquid in such a fire.
- `RCK-19` **Mortar** *(Decided)*: Lime mortar hardens in the air.
  - **Check:** quicklime mixed with water and sand hardens in air over weeks to months; kept away from air, it stays soft.
- `RCK-20` **Glass** *(Decided)*: Sand with plant ash melts into glass in a very hot fire.
  - **Check:** sand with plant ash melts into glass past the mixture's measured melting range; at those temperatures, sand alone does not melt.

## 8. People: bodies and lives

Every person has a body that must be fed, watered, kept warm and rested; that can be hurt, fall sick and heal; and that grows, ages, has children and dies.
All of it follows real biology with real-world numbers (`PRN-05`).
How people think is in Minds.

### 8.1 Who they are

- `BIO-01` **One species, modern minds** *(Decided)*: Their bodies and brains are as capable as ours.
  Their culture starts almost empty.
  - **How it works:** each body's numbers (size, strength, stamina, senses, healing and defences) are drawn within the measured ranges of living people, foragers above all (`BIO-08`), and every mind has the full set of modern human abilities (see Minds); their culture is only what `BIO-02` and `BIO-20` give them.

- `BIO-02` **Starting kit** *(Decided)*: The first people are generated like the world: realistic, not grown from nothing.
  Families, ages and relationships follow real hunter-gatherer patterns.
  Each adult knows their home range (its food, water, dangers and seasons) and nothing beyond it.
  The details are in `BIO-20`.
  - **Language:** a few dozen shared words and calls; word order and new words must grow.
  - **Fire:** they can feed a fire found after lightning or a wildfire, but cannot make one.
    A band may start with a fire it is keeping, depending on recent weather.
  - **Tools:** unshaped stones for bashing, and sticks.
  - **Clothing:** none.
  - **Shelter:** natural caves and overhangs.
  - **Food:** gathering, scavenging, some ambush hunting.
  - **Beliefs:** only the practical knowledge of their home range; none about spirits, hidden causes or how to make things.
  - **How it works:**
    - **Families from a short run:** the generator starts a few generations back and runs births, pairings and deaths at forager rates (`BIO-04`), with bodies only, up to year 0, so ages, families, kinship and genes (`BIO-06`) fit together.
      Only the result is kept, with no memories of it (`BIO-20`).
    - **Bodies:** each person's body is grown to their age from their genes and the food their region gives (`BIO-08`), with healed old injuries and scars at foragers' measured rates.
    - **Things they carry:** a few unshaped stones and sticks taken from their own range's ground: real things (`MAT-10`), nothing shaped.
    - **A kept fire:** a band starts with a fire only if lightning or a wildfire burned in its home range during the last months of settling (`WLD-08`, tuned); otherwise it has none.
    - **Skills:** adults start with skills for gathering, scavenging and ambush hunting with their kit (`MND-06`), at levels set by age (estimated); children have less.

- `BIO-03` **Starting population** *(Decided)*: 3–4 family bands of 15–30 people each, about 45–120 people in all, living in one region (`WLD-24`).
  - **How it works:** the short run of `BIO-02` is sized from the seed so that it ends with 3 or 4 bands of 15–30 people, within what the start region feeds (`WLD-24`).

- `BIO-20` **Starting knowledge in detail** *(Decided)*:
  - **Bands:** each band is a few related families.
    The bands are neighbours who sometimes meet, and share one language.
  - **What adults know:** where water, shelter and the main foods are in each season; which local plants and animals are food, which are poison and which are dangerous; the routes of their home range; who is kin to whom.
    Children know less, according to their age.
  - **Words:** water, fire, food, danger, kin, the main animals and plants of home, and simple actions such as come, go, eat and look.
  - **Fire:** they know how to carry embers to keep a fire alive on the move.
  - **Memories:** adults begin with their knowledge but no remembered events; their stories start at year 0.
  - **How it works:**
    - **Written from the world's truth:** starting beliefs are made from the settled world (`WLD-08`): where water, shelter and each main food are in each season; which local species are food, poison or dangerous, from their real chemistry and behaviour (`WLD-19`); the routes of the home range; and who is kin to whom.
      They are stored as records in each mind (see Minds).
    - **True but incomplete:** every starting belief is true; each adult knows the common things of their home range and fewer of the rare ones, more with age, and children know a share by their age (estimated).
      So every mistake comes from something that happens in play, and each has a cause.
    - **Words:** the few dozen shared words are drawn from the language's own sounds, each linked to its meaning in every adult's mind (see Culture and society).

- `BIO-08` **Everyone is different** *(Decided)*: Height, strength, stamina, senses, health, temperament, curiosity, memory and learning speed vary from person to person, with real-world spreads.
  These traits are partly inherited and partly shaped by how a person grew up, through childhood food, illness and activity.
  Follows from `BIO-06` and `PRN-05`.
  - **How it works:**
    - **Each trait is three parts:** an inherited value (`BIO-06`), how the person grew up, and chance.
      Childhood hunger and serious illness hold back growth in height and strength at measured rates, and activity builds strength and stamina; the spread of each trait, and the share each part explains, are sourced where measured and estimated otherwise.
    - **Traits that go together:** linked traits keep their measured links, so taller people are heavier and stronger on average, with wide overlap.

### 8.2 Staying alive

- `BIO-09` **Basic needs** *(Decided)*: Food, water, warmth and sleep, in real-world amounts that depend on body size, activity and climate.
  Follows from `PRN-05`.
  - **How it works:**
    - **What the body keeps:** its fat and lean mass, the food in its gut, its water, its core and skin temperature (`BIO-11`), and its need for sleep.
    - **Energy:** at each step the body burns energy: its resting rate from its size, age and sex (measured equations); each action's cost (`MAT-06`), and walking by distance, slope and load (measured rules); shivering or sweating (`BIO-11`); and growth, pregnancy and nursing (`BIO-15`).
      Digested food adds energy (`BIO-10`), and the difference goes into or out of fat (measured energy per kilogram).
      When fat runs low, lean mass is burned and strength falls, and below measured limits the body dies of hunger (`BIO-14`).
    - **Water:** lost through sweat (by heat and effort), breath, urine and illness, and gained by drinking and from food; losing water weakens and confuses at measured shares of body weight, and kills beyond them.
    - **Sleep:** the need for sleep builds with time awake and is cleared by sleep, at measured rates by age; lack of it slows body and mind and widens the error of every action (`MAT-06`).
    - **Felt, not known:** hunger, thirst, cold, heat and tiredness are what the mind feels of these stores (see Minds); the stores themselves decide survival.

- `BIO-10` **Nutrition** *(Decided)*
  - **What:** Food provides energy, protein, fat and key vitamins and minerals, all from the real chemistry of what is eaten (`MAT-03`).
    A missing vitamin causes its real deficiency disease.
  - **How it works:**
    - **What food gives:** each food's ingredients (`MAT-01`) give energy (from protein, starch, sugar and fat, at measured values per gram), protein, essential fats, and the vitamins and minerals in them.
      How much the gut takes from them depends on the food's structure and on cooking, which breaks down starch and toughness (`RCK-03`), at measured rates; food is digested over hours at measured rates, and the stomach holds only so much.
    - **A store for each key nutrient:** vitamins C, A, D, thiamine, niacin, B12 and folate; iron, iodine, calcium and salt.
      Each is filled by food, and vitamin D also by sunlight on skin, by latitude, season and skin colour (`BIO-22`); each is used up at its measured daily rate.
    - **Deficiency:** when a store falls below its measured threshold, its real disease develops at its real pace: scurvy (vitamin C), night blindness (vitamin A), rickets and soft bones (vitamin D), beriberi (thiamine), pellagra (niacin), anaemia (iron, B12, folate) and goitre (iodine, where soil and water lack it, `WLD-27`); eating the nutrient again cures it.
    - **Too much harms too:** some nutrients poison in excess, at measured doses, such as the vitamin A in some animals' livers.
    - **Growing bodies:** shortfalls of energy and protein in childhood hold back growth (`BIO-08`).
    - **More later:** other nutrients can be added as a layer (`MAT-16`).
  - **Why:** Diet becomes something people can discover and get wrong.
  - **Example:** A band that winters on dried meat suffers bleeding gums every spring (scurvy, from a lack of vitamin C).
    Eventually someone notices that the people who ate the first green shoots recovered.

- `BIO-11` **Heat and cold** *(Decided)*
  - **Follows from:** `MAT-03` and `PRN-05`.
  - **What:** Bodies lose and gain heat by real physics.
    Clothing, shelter, fire and huddling together keep them warm.
    Cold can kill; heat exhausts.
  - **How it works:**
    - **A heat balance at each step:** heat made (resting, activity, and shivering up to measured limits) against heat lost or gained: to moving air, by the wind and the difference between skin and air; by radiation, to a clear night sky and from the sun or a fire, by distance; by contact, with cold ground and with water, which takes heat about 25 times faster than air; and by evaporating sweat and breath.
    - **What shields the body:** its fat; clothing, by its insulation, measured from what it is made of (`MAT-03`) and much less when wet; shelter, which cuts wind and rain and gives back warmth from its walls; huddling, which hides part of each body; and a fire's radiant heat.
    - **Core and skin:** core temperature follows the balance by the body's heat capacity; past measured thresholds people shiver, then slow, grow confused (so their actions' errors grow, `MAT-06`) and die of cold, or tire, faint and die of heatstroke.
      Sweating and shivering respond by themselves.
    - **Hands, feet and face** cool first; flesh that freezes is frostbite, an injury to that part (`BIO-13`).
    - **Size matters:** children and old people lose heat faster for their size (measured).
  - **Example:** In an ice-age winter, sewn clothing can matter more than food.

- `BIO-12` **Poison and medicine** *(Decided)*: The chemistry of plants, animals and minerals acts on the body.
  Some things poison, some heal, and some do either depending on the dose.
  For example, willow bark eases pain.
  Follows from `MAT-03`.
  - **How it works:**
    - **Doses:** each chemical in what is eaten, drunk or breathed, or put on skin or a wound, enters the body at its measured rate, and its dose is counted per kilogram of body.
    - **Effects by measured dose curves:** each acts on its body systems (pain, gut, heart, breathing, nerves, liver, skin or mind) by its measured dose–effect curve, for as long as it stays, and the body clears it at its measured rate.
      So a small dose of willow bark eases pain and a large one harms the gut, and foxglove steadies the heart or stops it, by dose.
    - **Bodies differ:** size, age, health and each person's own sensitivity (`BIO-08`) shift the curve.
    - **Processing changes doses:** soaking, leaching, cooking, drying and fermenting change the chemicals by the laws of matter (`MAT-04`): acorns lose their bitterness in running water (`RCK-13`), and some poisons break down with heat.
    - **Warnings are partial:** many poisons taste bitter (`BIO-18`), but not all; what people learn about them is up to their minds.
    - **Sources:** doses that decide life and death are sourced; the rest are estimated (`PRN-05`).

### 8.3 Harm and healing

- `BIO-13` **Injuries to body parts** *(Decided)*
  - **What:** Wounds, fractures, burns and infections affect specific parts of the body.
    They heal, scar, or leave a lasting disability.
    Care from others (food, water, protection, cleaning a wound) changes the outcome.
  - **How it works:**
    - **Body parts:** each body has a head, eyes, neck, chest, belly, back, and each arm, hand, leg and foot, made of skin, fat, muscle, bone or organs, sized from the body's measurements; each part has its work: a leg carries weight, a hand grips, an eye sees.
    - **Injuries come from the laws of matter:** the contact laws (`MAT-04`) act on tissue like any material: an edge cuts as deep as its force and sharpness allow against the tissue's measured toughness; a point pierces; a blow bruises, or breaks bone past its measured strength; heat burns by temperature and time (measured thresholds); cold freezes (`BIO-11`); a fall strikes by its height and what is hit.
      Bites, horns and claws are edges and points driven by the animal's force.
    - **An injury's record:** the part, the kind (cut, puncture, bruise, fracture, burn, frostbite or bite), its size and depth, its bleeding, the dirt and germs in it (`WLD-21`), its pain, and how much the part still works.
    - **Bleeding:** blood is lost at a rate set by the wound's depth and place; losing measured shares of the body's blood weakens, then kills; pressing a wound slows it.
    - **Working parts:** a broken leg can't take weight, a cut hand grips weaker and a damaged eye sees less; pain makes every action less exact (`MAT-06`) and is felt by the mind.
    - **Healing:** each injury heals at measured rates for its kind (skin in weeks, bone in months), slower with poor food, age, cold and infection, and faster with rest.
    - **Outcomes:** a clean heal; a scar, stiffer and visible; or a lasting disability: a bone that moves while it heals knits crooked and leaves a limp, a cut tendon leaves a weak hand, and an infection can spread (`BIO-05`) and kill.
    - **Care is actions under the same laws:** pressing a wound slows bleeding; washing removes dirt and germs; a stick tied along a broken limb keeps the bone still so it knits straight; food, water, warmth and carrying keep the injured alive while they can't fend for themselves.
      Nothing heals a body except its own rates under better conditions.
  - **Example:** A hunter with a broken leg survives the winter because the band carries and feeds them.
    They never hunt again, but they become the best stoneworker in the valley.

- `BIO-05` **Disease** *(Decided)*: Illness comes from microbes that enter through wounds, food, water, air, touch or animals.
  People who recover can become immune.
  Crowding, and living close to animals, bring epidemics.
  Follows from `WLD-21`.
  - **How it works:**
    - **What a disease does:** each disease's effects come from its traits (`WLD-21`), with measured courses (how long before it shows, how long it lasts): fever, which burns more energy (`BIO-09`); diarrhoea and vomiting, which lose water and food; cough, weakness and pain; and damage to particular parts or organs.
    - **Defences:** the body fights each germ with its defences, stronger in well-fed adults and weaker in babies, the old (`BIO-16`), the starving and the injured.
      After recovery, immunity to that germ lasts its measured time and also guards against close strains; mother's milk gives babies some protection (measured).
    - **Being sick:** weakness and fever slow the body and widen its errors (`MAT-06`), and the mind feels the sickness (see Minds).
    - **Care works through the body's needs:** water, food and warmth given by others keep a sick body's stores up so its defences can win; replacing the water lost to diarrhoea saves lives, by measured amounts.
      Medicines act by `BIO-12`.

- `BIO-14` **Every death has a cause** *(Decided)*: Nobody dies of random chance.
  Every death comes from something in the simulation: hunger, cold, disease, injury, childbirth, violence, accident or old age.
  Follows from `PRN-10`.
  - **How it works:**
    - **Death only by a body's own limits:** a body dies only when one of its stores or parts passes a fatal limit: too little energy (hunger), water (thirst) or blood (bleeding); a core too cold or too hot (`BIO-11`); no air (drowning or smothering); a vital part destroyed; an organ failed by poison or germ (`BIO-12`, `BIO-05`); or birth's own dangers (`BIO-15`).
    - **No death roll:** chance enters only through events, such as a slip, a strike's error or a germ caught, and through each body's own variation (`TIM-16`), never as a chance of dying.
    - **Old age kills through frailty:** an old body has less in reserve and heals and defends itself less (`BIO-16`), so a cold, a fall or a fever that a younger body would survive passes the limit; the record names both, such as "pneumonia, in old age".
    - **The record:** each death keeps its cause and the chain behind it, as far as the simulation knows it, such as "bleeding, from a boar's tusk, while hunting", for the history and its views (`PRN-15`).
  - **Check:** every death record names the fatal limit that was passed and its cause; a code search finds no chance-of-death draw anywhere.

### 8.4 A life

- `BIO-04` **Life cycle** *(Decided)*
  - **Follows from:** `PRN-05`.
  - **What:** Birth, childhood, adolescence, adulthood, old age and death, following the life patterns of real hunter-gatherers.
  - **Target figures** (from studies of hunter-gatherers; they must come out of the causes, never be programmed, and are checked by experiment with tolerances, `RES-14`):
    - children are weaned at about 2–4 years, and a mother has a child about every 3–4 years;
    - around four in ten children die before the age of 15;
    - adults who reach 15 often live into their 60s and 70s;
    - women stop having children in their 40s.
  - **How it works:**
    - **Growth:** each child grows along measured growth curves for their sex toward their inherited height (`BIO-06`), held back by hunger and illness (`BIO-08`); strength, stamina and skills grow with age and use.
    - **Stages are body states, not labels:** a baby lives on milk; how long babies nurse is up to their mothers' minds and culture, while the body sets how much milk is made and what a child can eat.
      Puberty starts when the body reaches its measured size and fat for its age, earlier when well fed, and brings adult fertility.
    - **Birth spacing is not a rule:** a woman can conceive only when her energy balance and fat allow it, and frequent nursing holds her fertility back for months to years (measured), so the spacing of births comes out of nursing and food (`BIO-15`).
    - **Children die of the same causes as anyone** (`BIO-14`), with weaker defences and smaller bodies.
    - **The target figures above are checked, never set** (`RES-14`).

- `BIO-15` **Pregnancy and birth** *(Decided)*: Children come from pairs, through pregnancy, birth and nursing, with their real risks.
  Who pairs with whom, and how families are formed, is cultural (`CUL-07`).
  Pairing and conception are simulated abstractly, never as explicit acts, so sexual violence is not modelled.
  Follows from `PRN-05`.
  - **How it works:**
    - **Pairing is a relationship:** who pairs with whom is decided by minds and culture (`CUL-07`) and kept as a relationship between two people (`MND-24`); pairing and conception are never actions or animations.
    - **Conception:** each cycle, a paired woman who is fertile (`BIO-04`) and living with her partner conceives at the measured chance for both their ages.
    - **Pregnancy:** about 38 weeks, with a measured spread; it costs the mother measured extra energy and nutrients (`BIO-09`, `BIO-10`), miscarriage comes at measured rates by age and health, the baby grows on what she eats, and late in pregnancy she tires sooner and moves more slowly.
    - **Birth:** its dangers come at measured rates, raised by the mother's age, small size or poor food and by the baby's size and position: long labour, bleeding, and infection afterwards (`BIO-05`); the newborn's weight and health come from the pregnancy, and twins come at the measured rate.
      Help from others works through the same body laws: warmth, cleaning and feeding.
    - **Nursing:** milk costs the mother measured energy, is made in amounts set by how often the baby nurses and how well she eats, feeds the baby fully for months and partly for years, and holds back her fertility (`BIO-04`).
    - **Inheritance:** the child's genes come from both parents (`BIO-06`).

- `BIO-16` **Ageing** *(Decided)*: Strength, senses and fertility decline with age.
  Ageing also brings wear and frailty: wounds heal more slowly and defences against disease weaken, so old age kills through real causes (`BIO-14`).
  Knowledge and experience don't decline, so elders can matter as keepers of what the band knows (`CUL-02`).
  Follows from `PRN-05`.
  - **How it works:**
    - **Declines at measured rates:** muscle and strength fall by about a percent a year after middle age; stamina falls; eyes lose near focus in the 40s and sharpness after; hearing loses high sounds; healing slows; defences weaken; bones thin and break more easily; and women's fertility ends in their 40s, while men's falls slowly.
    - **Wear:** old injuries and years of heavy work add to it (estimated).
    - **Frailty** is all of these together (`BIO-14`).
    - **Minds:** what a person knows and can do stays; how fast they learn and recall slows somewhat with age, at measured rates (see Minds).

### 8.5 The sexes

- `BIO-17` **Real biology, culture decides** *(Decided)*
  - **What:** Bodies differ only in real biological ways: reproduction, and average differences in size, strength and body fat, with wide overlap between individuals.
  - **What doesn't:** Who hunts, gathers, leads or makes things is decided entirely by each culture, and can differ between cultures.
    The simulation never assigns a role by sex.
  - **Minds:** Minds don't differ by sex from birth.
    Every inborn mental trait has the same average in both sexes (`BIO-08`, `MND-20`), and any difference in behaviour comes from culture or from bodies.
  - **How it works:** each sex's body numbers are drawn from its measured ranges, with their wide overlap: upper-body strength differs most, lower-body strength and height less, and women carry a measured higher share of body fat, tied to pregnancy and nursing.
    No rule, action or law reads a person's sex except the body's own rules for reproduction, size, strength and body fat, and every inborn mental trait is drawn from the same spread for both sexes.
  - **Check:** a code search finds a person's sex read only by those body rules.

### 8.6 Senses and actions

- `BIO-18` **Senses** *(Decided)*: Sight (limited by light, fog and distance), hearing, smell, taste and touch, each with real ranges and differences between people, and declining with age.
  They are how people learn about the world (`MND-03`).
  - **How it works:**
    - **Sight:** a thing is seen when it is in view and not hidden by land or plants; lit enough, by sun, moon or fire; big enough for its distance against the eye's measured sharpness; and different enough from its background, so camouflage works.
      Fog, rain, dust and smoke cut how far anyone can see (`WLD-16`), and movement catches the eye.
    - **Hearing:** a sound fades with distance, high pitches fastest, and is blocked by land; it is heard if it passes the listener's measured threshold for its pitch, which rises with age, and stands out from the background noise of wind, water and rain; its direction is heard roughly.
    - **Smell:** a smell spreads from its source downwind with the weather's wind, and is noticed where it passes the measured threshold for that substance (estimated where none is measured); its direction is found only by moving.
    - **Taste:** sweet, salty, sour, bitter and savoury, from the ingredients of what is in the mouth (`MAT-01`), by measured thresholds.
    - **Touch:** on contact: texture, hardness, weight when lifted, warmth, wetness, sharpness and pain.
    - **People differ:** each sense's sharpness comes from the person (`BIO-08`) and age (`BIO-16`); an injured eye or ear (`BIO-13`) or a cold dulls it.
    - **No scanning:** senses never sweep the world.
      Whatever makes light, sound or smell (a strike, a call, a fire, an animal moving) is passed once to the bodies within its physical reach, and each checks it against its own senses; looking, listening and sniffing are things a mind chooses to do (see Minds).

- `BIO-21` **Moving, eating and acting together** *(Decided)*: Alongside the actions on matter (`MAT-12`), bodies move (walk, run, climb, crouch, swim), eat and drink, sleep, touch, hold and give, and communicate (call, sing, point, gesture).
  Walking, running, climbing, calling and pointing are inborn; swimming is learned, as is everything people come to do with these acts.
  - **How it works:**
    - **Moving:** walking speed is set by slope, ground and load (measured rules), at its measured energy cost (`BIO-09`); running is faster and costlier, limited by stamina; climbing needs holds and strength, and a slip is a real fall (`MAT-11`, `BIO-13`); crouching makes a body slower, quieter and harder to see; swimming costs measured energy, and water over the face drowns (`BIO-14`).
    - **Inborn and learned:** walking, running, climbing, calling and pointing start as working skills; swimming starts unskilled, with large errors (`MAT-06`), and improves only with practice (`MND-06`).
    - **Eating, drinking and sleeping:** food is put in the mouth, chewed and swallowed into the gut (`BIO-10`), and water drunk by mouth or hand; asleep, the senses are dulled, but loud sounds, pain or cold wake the body.
    - **Holding and giving:** passing a thing from one person to another needs both to hold it; touching another body passes warmth and is felt.
    - **Communicating:** calls and songs are sounds with pitch, loudness and length, heard by hearing (`BIO-18`); pointing is an arm aimed along a line, and what lies along it is up to the onlooker's mind; gestures are poses and movements that others see.
    - **Speech sounds:** each sound of a language is a set of real articulation features, where and how the mouth makes it, so ease of saying and of hearing apart can shape how languages change (see Culture and society).
      Named simplification: the throat itself is not simulated.

### 8.7 Inheritance

- `BIO-06` **Heredity** *(Decided)*: Body traits and mental traits (curiosity, memory, learning speed, temperament) pass from parents to children.
  They shift over generations at real-world speeds as some people survive and have children and others don't.
  Minds barely change over thousands of years; culture does the heavy lifting, as in our own history.
  - **How it works:**
    - **An inherited value for each trait:** each person carries one for every body and mind trait (`BIO-08`), such as height, build, strength, stamina, senses, defences, curiosity, memory, learning speed and temperament (`MND-20`), plus the single genes behind looks (`BIO-22`).
    - **From both parents:** a child's inherited value is the parents' average plus variation, by the trait's measured share that is inherited; traits set by single genes pass by Mendel's rules; and mutation adds new variation at measured rates, as for plants and animals (`WLD-20`).
    - **Selection is only what happens:** who survives and has children (`BIO-14`, `BIO-15`) shifts the averages over generations, at real speed, with no fitness rule; at real rates, mind traits move too slowly to notice within thousands of years.

- `BIO-07` **Evolution dial** *(Decided)*: A setting speeds up genetic change for experiments (`PRN-12`).
  - **How it works:** one experiment setting multiplies the new variation mutation adds each generation by a stated factor, so selection has more to work with and traits move faster; it cannot be set in play, and every experiment report lists it.

- `BIO-22` **Looks** *(Decided)*: Skin, hair and faces are inherited (`BIO-06`) and vary by region with sunlight, as in real biology and at real speeds, so they change slowly.
  They are designed so that no people reads as a copy of a real one (`SCP-20`).
  - **How it works:**
    - **Genes for looks:** skin colour, hair colour and form, eye colour and face shape are each inherited through many genes (`BIO-06`).
    - **Sunlight shapes skin by the body's own rules:** strong sunlight destroys folate in pale skin, and weak sunlight makes too little vitamin D in dark skin (`BIO-10`), so who survives and has children shifts skin colour toward what each region's sunlight favours, over thousands of years.
    - **The rest drifts:** hair, eyes and faces change by chance and by who has children with whom, so peoples kept apart slowly come to look different.
    - **No copy of a real people:** the starting look genes are drawn from wide ranges and mixed so that no group matches the typical look of any real people.

### 8.8 Animals

- `BIO-19` **Animal bodies** *(Decided)*: Animals have bodies that work in the same way, with their own species' traits (`WLD-19`): needs, injuries, disease, life cycles and senses.
  - **How it works:** animals' bodies run the same rules as people's (`BIO-09` to `BIO-18`) with their species' numbers: needs scaled to their size (`WLD-18`); warmth kept by fur, feathers or fat (`BIO-11`); injuries to their own body parts; disease (`WLD-21`); life cycles; and senses with each species' measured ranges, so dogs smell far better than people and birds of prey see farther.
    Individual animals have full bodies (`WLD-12`); for counted animals, the same rules act on each age and sex group in a patch, deciding births and deaths with their causes (`WLD-18`).

## 9. Minds

How people think, and in simpler form how animals think.
Everything a mind knows is learned inside the world (`PRN-01`), every choice can be explained (`PRN-13`), and no AI language model ever thinks for anyone (`PRN-06`).

### 9.1 Ground rules

- `MND-01` **No AI language model thinks for them** *(Decided)*: Every belief and invention comes from the mechanisms in this section (`PRN-06`).
  - **How it works:** a mind's mechanisms are small numerical rules over its own records; no language model is ever called by a mind or reads a mind to choose anything.
    The only language model in the game, the writer (`PRE-37`), reads finished records to write text, and nothing it writes goes back into the simulation.
  - **Check:** a code check finds no call from the simulation to any language model, and no path from the writer's output back into the simulation.

- `MND-02` **Knowledge only from inside the world** *(Decided)*: Any learning a mind does draws only on experience in its own world.
  Nothing carries real-world knowledge in.
  Follows from `PRN-01` and `PRN-06`.
  - **How it works:** a mind starts with only its starting records (`BIO-20`), drawn from its own world; every later record comes from its own senses, its own actions' results, and what others in the world show or tell it.
    The mechanisms' tuned values, such as learning rates, are settings, never content; the catalogues are the world's physics, and no mind can read them.
  - **Check:** mind code reads only the mind's own records and percepts, and a code search finds no catalogue name or real-world word list in it.

- `MND-17` **Why ordinary minds are enough** *(Decided)*: No single mind needs to be a genius.
  A people's intelligence comes from four sources, and only one of them is inside a head:
  1. **a world made of properties, not recipes** (see Matter and physics), so simple learning finds real things;
  2. **small, well-understood learning mechanisms**, the ones described in this section;
  3. **many minds over generations**, copying imperfectly, varying and passing things on (`CUL-01`).
     Researchers call this cumulative cultural evolution;
  4. **time:** an accident with a one-in-ten-thousand chance happens routinely over centuries.

### 9.2 Perceiving and knowing

- `MND-03` **Senses, not labels** *(Decided)*: People perceive properties (weight, hardness, colour, smell, taste, warmth, sound) through their senses (`BIO-18`), never the game's names for things.
  - **How it works:**
    - **Percepts:** what the senses pass on reaches the mind as a percept: the thing's properties as that sense gives them, blurred by distance, light and the person's sharpness.
      For sight: colour, gloss, size, shape, texture and movement; for sound: pitch, loudness and timbre; for smell: the mix of smell substances; for taste: the five tastes; for touch: hardness, weight, warmth, wetness and sharpness.
      A percept also holds where the thing is, and a pointer to the thing itself so the person can act on it, never its kind or name.
    - **Appearance kept with the thing:** each thing's visible properties are worked out once from its makeup and structure (`MAT-03`) and kept until it changes, so many viewers cost little.
    - **Attention:** each moment, a mind takes in only its few most noticeable percepts, about four, as measured for real attention: those it didn't expect, those that move or are loud, and those that match what it is looking for or what its drives want.
      The rest pass unnoticed, which is also how people miss things.
    - **Recognised once:** a percept is matched to the person's own concepts (`MND-04`), and a recognised thing stays linked to its concept until it changes.

- `MND-04` **Their own concepts** *(Decided)*
  - **What:** People sort what they perceive into their own categories.
    Categories differ between groups and can be wrong.
  - **How it works:**
    - **A concept** is a mind's own group: a typical example (the average of the properties of what it has grouped), how widely its members vary, and how much each property counts in deciding what belongs.
    - **Forming:** a percept that fits no concept well starts a new one; percepts that fit are added and nudge the typical example toward them.
    - **Learning what matters:** properties that predicted outcomes come to count more: if glossy stones chipped and dull ones crumbled, gloss gains weight; and things that behaved alike are pulled into one concept even if they look different.
    - **Kinds of concept:** things, places, people, animals, actions (such as striking or carrying), properties (such as sharp or red) and events (such as fire after lightning).
    - **Concepts can be wrong:** a poisonous berry whose looks fit a safe berry's concept is taken as safe until something tells them apart, such as taste or a sickness.
    - **Groups differ:** each person's concepts come from their own experience and from the words others use for things (`CUL-04`), so bands come to divide the world differently.
    - **Open:** concepts of things, places and events are well understood; abstract ones, such as number, debt or spirit, form from patterns across events and relationships, which is less well understood, and experiments test it.
  - **Example:** One band lumps flint and chert together as "cutting stone"; another confuses a poisonous berry with a safe one.

- `MND-05` **Cause-and-effect beliefs** *(Decided)*
  - **What:** "Doing this to that, in this situation, leads to this." Each belief is held with more or less certainty, which rises and falls as evidence comes in.
  - **How it works:**
    - **A belief's record:** an action or event, what it acts on, the situation (place, season and what else is there), the outcome, how strongly the outcome is expected, and its evidence: how many times it held and failed, and links to the memories behind it.
    - **Learning by surprise:** after anything happens, beliefs that predicted it grow and beliefs that predicted something else shrink, by the size of the surprise (the gap between what was expected and what happened), at a rate set by the person's learning speed (`BIO-08`), as in the Rescorla–Wagner rule, a well-tested model from psychology.
      So a fully expected outcome teaches little, and a cause that adds nothing to an already known cause gets no credit, as in real learning.
    - **Credit over time:** recent actions and events leave fading traces, so an outcome credits what came before it in proportion to how recent and how noticeable each was; some links span hours, as when sickness is tied to a meal.
    - **Certainty is the evidence:** many confirmations and few failures make a confident belief; one striking event makes a strong but uncertain one.
    - **Discovery and superstition** both come from this: repeated tries sort real causes, which keep working, from coincidences, which fail, unless a belief is never tested again.
    - **Tuned in Experiment 1:** the learning rate and the length of the traces (`RES-02`).
  - **Why:** Discovery and superstition come from the same mechanism, with different luck.
  - **Example:** Striking glassy stone makes sharp edges: a discovery.
    The band sang before a hunt that went well: a superstition, which can become a rite (`MOM-04`).

- `MND-27` **Kinds of belief** *(Decided)*: Beliefs come in several kinds, each held with a certainty and the evidence behind it: cause and effect (`MND-05`); that something exists, such as an unseen being; what others know and want (`MND-23`); rules, such as what not to eat (`CUL-20`); and plain facts, such as where the water is.
  - **How it works:**
    - **One record per belief,** of its kind, with its certainty and its evidence: cause and effect (`MND-05`); existence, that something is, or is at a place, including something never seen (`MND-21`); others' minds (`MND-23`); rules, that an act is required or forbidden and what follows breaking it (`CUL-20`); and facts of where and when, such as the water at the spring or hazelnuts on the south slope in autumn.
    - **Sources:** each belief records where it came from: the person's own experience, seeing someone else's, or being told, and by whom; trust in the source sets its starting certainty (`MND-24`).
    - **Clashes:** when beliefs disagree, the more certain one guides choices, and evidence decides between them over time.

- `MND-18` **Memory** *(Decided)*: People remember:
  - events they lived through;
  - places, as a mental map with the seasons attached ("hazelnuts on the south slope in autumn");
  - people: who's who, family, and who owes whom;
  - know-how (`MND-06`) and beliefs (`MND-05`).

  Vivid and repeated memories last; others fade.
  Retelling can change a memory.
  - **How it works:**
    - **Events:** each noticed event becomes a memory record: what happened (its concepts and things), who was there, where and when, what it led to, and how the person felt (`MND-19`).
    - **Places:** a mental map of places, each with where it is, linked by the routes walked and how long they took, with facts attached by season (`MND-27`).
    - **People** each have a record (`MND-24`); **know-how and beliefs** are their own records (`MND-06`, `MND-05`).
    - **Fading:** each memory has a strength that fades with time by the measured forgetting curve, and is renewed whenever it is recalled, retold or replayed in sleep (`MND-12`); strong feelings make it start stronger (`MND-08`).
      Below a threshold its details are lost: what it taught stays in the beliefs it fed, and the event itself is forgotten.
    - **Recall:** a memory comes back when something cues it: the same place, people, things or feelings; the strongest matches come first.
    - **Retelling changes memory:** each recall rebuilds the event from what remains and what the person now believes, so details drift toward expectations, and a story heard from others can replace a person's own details.
    - **Size:** memory is large but finite, and the weakest records go first; the size per person is measured in Experiment 1 (`RES-02`).

- `MND-08` **Feelings shape memory** *(Decided)*: Strong feelings decide what is remembered and how strongly.
  A terrifying storm stays for life; an ordinary day fades.
  - **How it works:** a new memory's starting strength is multiplied by how strongly the person felt at the time (`MND-19`), by the measured link between arousal and memory; very strong feelings also fade more slowly (estimated).
    Fear and pain tie their feeling to the place, people and things that were there, so meeting them again brings the feeling back, as in real fear learning.

### 9.3 Wanting and feeling

- `MND-07` **Drives** *(Decided)*: Hunger, thirst, cold, tiredness, fear, belonging, status, curiosity, sexual desire and attachment.
  Nobody knows at first that sex leads to children; that has to be learned (`PRN-01`).
  - **How it works:**
    - **Each drive is a number** read from the body or the social world: hunger, thirst, cold or heat, and tiredness from the body's stores (`BIO-09`, `BIO-11`); fear from harm the person believes is coming (`MND-05`); belonging from time apart from the band and kin, and from being shunned or included; status from how others treat the person (`MND-24`); curiosity from unexplained surprises and new things noticed (`MND-10`); sexual desire from adult age and the body's state; and attachment toward particular people, rising with time apart from them.
    - **Weights:** personality scales each drive's pull (`MND-20`), so the curious feel curiosity more and the dominant feel status more.
    - **What drives do:** the most pressing drives set what a mind wants now (`MND-09`); meeting them lowers them, and how they rise and fall feeds feelings (`MND-19`).
    - **Sex and children:** no belief links them at the start; with a delay of nine months and no sure sign, that link is hard to learn from evidence (`MND-05`), so it may take a long time.

- `MND-19` **Feelings** *(Decided)*: Fear, anger, joy, grief, disgust, surprise, affection, shame, pride, awe, longing and hope.
  They colour choices and memories, and some last: grief for months, or a fear of the forest for life after a wolf attack.
  - **How it works:**
    - **Feelings come from judging events:** each arises when an event is judged against the person's drives, goals and beliefs, by the rules of appraisal theory, a well-tested family of models in psychology: surprise, an outcome far from what was expected; fear, likely harm ahead; anger, a goal blocked or harm done by someone; joy, a goal met; grief, losing someone or something the person is attached to; disgust, rot, filth or acts learned to be foul; affection, warmth from shared good experience; shame and pride, one's own act judged by the group's rules (`CUL-20`) and by others' regard; awe, something vast or unexplained; longing, missing someone or somewhere; and hope, a good outcome believed possible.
    - **Strength and fading:** each feeling has a strength that fades at its own rate (measured where possible, estimated otherwise): surprise in seconds, anger in hours, grief over months; learned links can make one last for life (`MND-08`).
    - **What feelings do:** they shift choices (fear makes risks loom larger, anger makes striking back likelier, affection draws people together, grief slows them), strengthen memories (`MND-08`), and show in face, posture and voice, which others can see (`MND-23`).

- `MND-20` **Personality** *(Decided)*: A few inborn tendencies (curiosity, boldness, sociability, patience, dominance, readiness to conform), partly inherited (`BIO-06`) and shaped by what happens to each person.
  - **How it works:**
    - **Six numbers per person,** each an inherited value plus upbringing plus chance (`BIO-08`), with measured spreads and the same spread for both sexes (`BIO-17`).
    - **What each does:** curiosity raises the pull of new things and unexplained surprises (`MND-10`); boldness lowers how much risk and fear weigh; sociability raises belonging and the pull of company; patience makes future rewards count more against present ones; dominance raises the status drive and the readiness to challenge; and readiness to conform raises the weight of doing what most others do (`CUL-01`).
    - **Shaped by life:** strong experiences shift them slowly (estimated): repeated harm makes a person less bold, and repeated success bolder.

- `MND-21` **Inborn tendencies** *(Decided)*
  - **What:** The biases evolution gave humans: a taste for sweet and fat and a wariness of bitter; quicker fear of long, legless things that move suddenly (such as snakes), of heights and of the dark; attachment between parent and child; the urge to imitate; and suspecting a hidden someone behind unexplained events.
  - **Why:** They make some lessons easier to learn but teach nothing by themselves, so the world stays the only teacher (`PRN-01`).
  - **More:** further tendencies are in `MND-26`.
  - **How it works:** each is a bias in the mechanisms, never a belief or a name:
    - sweet and fat tastes add to how good food feels, and bitter takes away (`BIO-18`);
    - fear links to percepts of long, legless, suddenly moving things, to heights and to darkness form from less evidence, as measured in studies of prepared fear;
    - parents and children feel attachment toward each other from birth (`MND-07`);
    - watching others act raises the pull to copy them (`CUL-01`);
    - when an important event has no cause the mind believes in, it forms a weak belief that an unseen someone caused it (`MND-27`), which later events can strengthen or weaken.

    The evidence for this last tendency is debated; like every tendency, its comparison run shows what it changes (`RES-10`).

- `MND-26` **More inborn tendencies** *(Decided)*: Added to `MND-21` from research, each with its sources and a comparison run (`RES-10`) showing what it changes:
  - pain, and avoiding what causes it;
  - favouring kin, and caring for the hurt and the sick;
  - returning favours, and anger at cheats;
  - favouring one's own group;
  - not desiring those one was raised with;
  - shared attention and pointing;
  - readiness to learn words;
  - moving together to a beat.
  - **How it works:** each is a bias in the mechanisms, like those in `MND-21`:
    - **pain** is a strong bad feeling from injuries (`BIO-13`) that teaches avoiding its cause quickly;
    - **favouring kin:** the wellbeing of kin, as the person believes kinship to be, counts in their choices, more for closer kin;
    - **caring:** seeing someone hurt or sick raises the pull to help, more for kin and friends;
    - **returning favours:** a favour received creates a debt in the giver's record (`MND-24`) that pulls toward repaying, and seeing someone take without returning raises anger and lowers trust;
    - **one's own group:** people taken to be of one's own group, by learned signs such as shared words or ways, get more trust;
    - **not desiring those raised with:** desire is held down toward anyone a person lived closely with as a young child, as measured in real studies, whoever they are;
    - **shared attention:** people follow others' gaze and pointing to the same thing (`BIO-21`) and know they both attend to it;
    - **learning words:** children pair sounds they hear with things attended to together quickly (`CUL-04`);
    - **a beat:** a steady beat pulls movements into time with it and raises closeness among those moving together.

### 9.4 Deciding and doing

- `MND-09` **Choosing what to do** *(Decided)*: Habits handle routine.
  Deliberate planning takes over when habits fail or the stakes rise: working backwards from a need through what they believe causes what.
  People explore most when they are comfortable (play) and when they are desperate (need).
  - **How it works:**
    - **When a mind decides:** only when its activity ends or fails, when something it notices interrupts (a surprise, a threat or a call, `MND-03`), when a drive passes its threshold, or when a plan's time comes (`MND-22`); in between, the body carries on with the chosen activity and the mind costs nothing.
    - **An activity** is a goal with its steps: a skill (`MND-06`), or a short run of actions toward a target, such as going to the spring to drink.
    - **Habits first:** a habit links a situation (place, time of day, season, drives, who is near) to an activity, with a value learned from how well it went; if one fits and no drive presses beyond it, it runs with no further thought.
    - **Deliberate planning:** when no habit fits, a habit fails or the stakes rise, the mind works backward from what its most pressing drive wants, through its cause-and-effect beliefs (`MND-05`), to actions it can take now: warmth needs fire, fire needs feeding, feeding needs wood, and wood lies by the river.
      It chains a few steps (tuned), and weighs each option by how much it should meet the drives, given the beliefs' strength and certainty, against effort, time, risk (weighted by boldness), the future (weighted by patience, `MND-20`), and others' expectations and the group's rules (`CUL-20`); chance from the person's own draws (`TIM-16`) settles near-ties.
    - **Exploring:** instead of the best known option, the mind sometimes tries something new: a varied skill setting, a known action on an unfamiliar thing, or an analogy (`MND-11`); it explores most when its drives are low (play, more for the curious and the young) and when they are high with no believed way out, and least in between.
    - **Reasons kept:** each decision records the drives, beliefs, memories and feelings that won, for the scientist's view (`PRN-13`).
    - **Habits form:** a choice that keeps working in the same situation becomes a habit.

- `MND-06` **Skills** *(Decided)*
  - **What:** Learned sequences of actions with fine control (angle, force, timing), built from the body's basic actions (`MAT-12`), that improve with practice.
  - **How it works:**
    - **A skill's record:** a sequence of steps, each a basic action (`MAT-12`) on a kind of target (a concept), with its settings (force, angle, point of contact and timing) as a typical value and a spread, plus how often it has worked and how much it has been practised.
    - **Doing it:** each try draws its settings around the typical value with the skill's spread, adds the body's own error (`MAT-06`), and the physics decides the result.
    - **Practice:** after each try, the typical settings move toward those that worked better, at the person's learning speed (`BIO-08`); the spread shrinks with practice by the measured power law of practice, and failures widen the search a little.
    - **Where skills come from:** a person's own tries (`MND-11`), copying what others are seen doing, with copying errors (`CUL-01`), and being taught.
    - **Knowing is not doing:** a belief that striking makes flakes (`MND-05`) gives no settings; only practice does.
    - **Fading:** skills unused for long slowly lose their precision (estimated).
  - **Why:** Knowing something can be done is not the same as doing it well.
  - **Example:** A child who has watched knapping knows that striking makes flakes, but shatters a dozen stones before getting one good edge.

- `MND-13` **Learning over a lifetime** *(Decided)*: People get better at things through their own experience.
  - **How it works:** every kind of learning in this section goes on for life: beliefs by surprise (`MND-05`), concepts (`MND-04`), skills by practice (`MND-06`), habits (`MND-09`), the mental map (`MND-18`) and knowledge of people (`MND-24`).
    Each person learns at their own speed (`BIO-08`), fastest in childhood and somewhat slower with age (`BIO-16`), at measured rates.

- `MND-22` **Planning ahead** *(Decided)*: People can plan days and seasons ahead once they have learned the patterns, such as storing nuts before winter.
  Tools from culture, such as calendars, counting and records, make longer plans reliable (`CUL-03`).
  - **How it works:**
    - **A plan** is a goal, its steps, and when each should start, kept in the mind and checked when its time comes (`MND-09`).
    - **From learned patterns:** facts and cause-and-effect beliefs about the seasons (hazelnuts come in autumn; late winter brings hunger) let the backward search reach future needs: late winter needs food, stored nuts give it, so gather nuts in autumn and keep them.
    - **The future counts less:** future needs weigh less than present ones, by the person's patience (`MND-20`), and less again when the pattern is uncertain.
    - **How far:** a person holds only a few plans and chains only a few steps (tuned); counts and dates kept outside the head, such as tally marks or a calendar (`CUL-03`), let plans reach further and keep them reliable.
    - **Revised:** when a plan's time comes and things differ from what was expected, it is decided again.

### 9.5 New ideas

- `MND-10` **Curiosity in minds** *(Decided)*: Attention goes where expectations fail.
  Surprises are remembered and tried again.
  - **How it works:**
    - **Surprise draws attention:** a percept or outcome far from what beliefs predicted is attended to first (`MND-03`) and raises the curiosity drive (`MND-07`), by the person's curiosity (`MND-20`).
    - **Kept and tried again:** a surprise is remembered strongly (`MND-18`) and leaves a weak belief linking what came before with what happened (`MND-05`); when drives allow, the mind repeats what came before, varied a little, to see whether it happens again (`MND-09`), and each try's result strengthens or kills the belief.
    - **Curiosity follows learning, not noise:** it is drawn most to what the mind is learning fastest about, and fades for what has stopped teaching anything, as in well-studied models of curiosity, so people don't fixate on pure chance.

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

  - **How it works:** every new idea is a new try, built from what is already in the mind:
    - **accidents:** an action meant for one thing gives an unexpected result, and the surprise makes it a weak belief (`MND-10`);
    - **watching nature:** an event seen with no one acting, such as fire after lightning, is learned like any outcome (`MND-05`), with the event before it as its cause;
    - **tinkering:** a skill's settings pushed beyond their usual range, its steps reordered, or the skill used on a new target (`MND-06`);
    - **analogy:** an action believed to work on one thing is tried on another that shares the properties the belief rests on, so what scrapes wood might scrape bone (`MND-04`);
    - **signs of the goal:** in planning, what usually comes with a goal can be sought as a step toward it (`MND-09`): smoke comes with fire, so a smoking stick becomes a hunch about fire, as in step 3;
    - **dreams:** new links between memories, held as weak beliefs (`MND-12`).

    The physics decides each try (`MAT-04`), and the belief grows or dies (`MND-05`).
    A discovery happens only through a chain whose every step pays off enough to be repeated, like the one above; a step that never pays off is reached only by rare luck over long times (`MND-17`).

- `MND-12` **Dreaming** *(Decided)*: During sleep, people replay and recombine their own memories.
  This strengthens what they learned, sometimes connects things in a new way, and is also your lever (`GOD-03`).
  - **How it works:**
    - **Replay:** during sleep, the day's strongest memories, by feeling, surprise and how much they matter to pressing drives, are replayed, a limited number each night (estimated); each replay renews the memory (`MND-18`) and its beliefs and skill settings, like a little more practice, as real sleep does.
    - **Recombination:** pieces of different replayed memories (their things, places and people) are put together at random, weighted by how active each is; a pairing that matches a pressing want or an open question (`MND-10`) leaves a weak new belief, such as "smoking stick" next to "fire", which waking tries can test (`MND-11`).
      Experiments have shown sleep helping people find a hidden rule.
    - **Dream records:** each night's dream is kept as a record of what was replayed and combined, with its feelings, for the views and the writer.
    - **Your lever:** you can choose which pieces one sleeper's dream combines, from what is already in their mind; nothing new is added (`GOD-03`).
    - **Animals** replay their memories too, more simply (`MND-16`).

### 9.6 Other minds

- `MND-23` **Understanding others** *(Decided)*
  - **What:** People track what others know, want and believe, and can reason one step deeper ("she thinks I don't know").
  - **How it works:**
    - **A model of each known person:** kept in their record (`MND-24`): what they know, want and believe, each as a belief about their mind with its certainty (`MND-27`).
    - **How it is built:** from what the other was present for and could perceive (if she was there when the fire went out, she knows it went out), from what they said, and from what they did, explained by the simplest want that fits (walking to the spring means thirst), as in well-studied models of how people read others' actions.
    - **One step deeper:** beliefs can be about what another believes about oneself or a third person ("she thinks I don't know"), to one extra level only.
    - **What it makes possible:** teaching (I know, she doesn't, and I want her to, so I show slowly and correct); cooperation (we both want the deer, and I expect him to go round); gossip (telling what someone did); and deception (acting or speaking to make someone believe what isn't so, when the person believes it pays, such as hiding a flint source).
    - **Children** grow into it with age, at the measured pace, understanding others' false beliefs from about four.
  - **Why:** This is what makes teaching, cooperation, gossip and deception possible.
  - **Example:** Tamo keeps a good flint source secret, believing nobody knows about it.
    Ama has noticed the fresh flakes Tamo brings back, and follows one morning.

- `MND-24` **Relationships** *(Decided)*: People know who's who: family, friends, rivals, and who owes whom.
  Trust and affection grow and fade with shared experience.
  - **How it works:**
    - **A record for each known person:** how they are recognised (face, voice and smell, as concepts of that one person, `MND-04`); kinship as believed; affection, trust, respect and fear; the favours owed each way (`MND-26`); shared memories (`MND-18`); and the model of their mind (`MND-23`).
    - **Changing:** each shared experience adjusts these: help and shared success raise affection and trust; harm, cheating and broken promises lower them, more in close bonds; and time apart lets them fade slowly (estimated).

### 9.7 Scale and inspection

- `MND-14` **Detail follows what matters** *(Decided)*: Everyone is always an individual, with their own body, family, memories, skills and beliefs.
  People in routine situations may run more cheaply, even as part of their band, but only once an experiment shows this gives the same history, statistically, as full detail.
  Anyone facing something new, risky or important runs in full.
  The rule depends only on the world, never on where you look.
  Follows from `PRN-11`.
  - **How it works:**
    - **Full detail is already sparing:** a mind works only when something happens (`MND-09`) and takes in only its few most noticeable percepts (`MND-03`), so a quiet day already costs little.
    - **Always in full when:** a surprise passes its threshold (`MND-10`); there is a threat, an injury or an illness; there is a conflict or an unusual social event, such as a stranger, a quarrel, a birth or a death; no habit fits; or the person chooses to explore (`MND-09`).
      This is the world's own rule, never the camera's (`WLD-13`).
    - **The cheaper routine path, only after its experiment:** while a person runs a habit and every outcome matches what they expected, every action still happens with its physics, but learning updates are applied at the end of each activity, and runs of the same action, such as picking berry after berry, are worked out together.
      Nothing is skipped or summarised.
    - **Checked:** the same sandboxes run with and without the routine path, and their discoveries, the spread of skills and words, and deaths are compared (`PRN-11`).

- `MND-15` **No population cap** *(Decided)*: How many minds the phone can run at each level of detail is found by measurement (`PLT-04`).
  That number is the limit of a world: every person stays a full individual (`MND-14`), so history slows as a world nears it, and the game tells you when it is reached.
  By rough estimates, each mind needs about 0.3–1 MB and about a millisecond of computing per simulated day, so the limit is a few thousand people at a watchable speed; cities and farming-scale worlds of hundreds of thousands are beyond any phone.
  - **How it works:** nothing in the code limits how many people or animals there are: food and the body rules set their numbers (`WLD-04`); when there are more than the phone can run at the speed asked for, time slows (`PRN-11`); and how many run at each speed and level of detail is measured (`PLT-04`).

- `MND-25` **Thoughts are structured; words come later** *(Decided)*: What a person thinks is kept as beliefs, intentions, feelings and memories, never as sentences.
  The story view (`PRE-14`) turns them into words through the writer AI (`PRE-37`); the scientist's view shows them raw.
  Follows from `PRN-06`.
  - **How it works:** everything in a mind is a record: concepts, beliefs, plans, feelings, memories, skills and relationships, each made of references to concepts, things, places and people, with numbers; nothing is stored as a sentence.
    The scientist's view shows the records raw; the story view hands them to the writer, which only phrases them (`PRE-17`), and nothing it writes is read back (`MND-01`).

### 9.8 Animals

- `MND-16` **Animals** *(Decided)*
  - **What:** Animals have the same kind of mind with fewer abilities.
    They learn fear, routes and habits, so hunting becomes an arms race and taming becomes possible.
  - **What animals lack:** language, deliberate teaching, long plans and abstract concepts.
    Species differ: wolves hunt together, deer are wary grazers.
  - **Detail:** near people, animals are individuals with minds.
    Elsewhere they are populations that carry inherited and learned traits, such as wariness of people (`WLD-12`).
  - **How it works:**
    - **The same mechanisms, with fewer parts:** percepts, concepts, cause-and-effect beliefs, memory with a mental map, drives and feelings, habits, short plans and dreams work as in people; animals have no words, no teaching, plans of only a step or two, simple models of others at most, and no abstract concepts.
    - **Species settings:** each species' drives, instincts (herding, pack hunting, territory, caching food), senses (`BIO-19`), learning speed and how much it can hold come from its family (`WLD-19`), measured where possible and estimated otherwise.
    - **Fear of people:** animals that are chased or wounded, or that see others killed by people, learn wariness of people and of their signs (`MND-08`), so hunting grows harder where people hunt.
    - **Taming:** an animal that is fed and not harmed by people loses its fear of them and grows attached to particular people (`MND-07`); over generations, boldness near people is inherited (`WLD-20`).
    - **Hunting together:** each wolf moves to keep the prey between itself and its packmates, as in a published model of pack hunting, so a pack surrounds its prey with no plan.
    - **Counted populations** carry learned wariness and routes as population traits (`WLD-12`, `WLD-20`).

## 10. Culture and society

Culture is everything people pass to each other rather than inherit through their bodies: skills, words, beliefs, customs and art.
None of it is scripted (`PRN-01`, `PRN-07`).
It grows out of minds (see Minds) living together, and it changes, spreads, splits and dies.

### 10.1 Passing things on

- `CUL-01` **Learning from others** *(Decided)*
  - **What:** People imitate (and imperfect copying creates variation), teach (possible because they understand what others know, `MND-23`), and copy whoever succeeds or whatever most people do.
  - **How it works:**
    - **Watching:** a person who notices someone act (`MND-03`) sees the action, roughly its settings, and its outcome.
      The outcome updates their own beliefs (`MND-05`), more weakly than their own experience would, weighted by their trust in the one acting (`MND-24`); and they copy the steps and settings they saw into a skill of their own (`MND-06`), blurred by distance and attention and changed by their own body, so copies vary.
    - **Whom to copy:** each person weights others by their seen success and prestige, by kinship and likeness, and by how many people do a thing the same way, the pull to conform that is well studied in cultural evolution; their readiness to conform (`MND-20`) sets the balance.
    - **Teaching:** someone who believes another lacks a skill or belief they have (`MND-23`), and wants them to have it, may teach: doing the steps slowly in view, pointing out the key parts (`MND-26`), correcting the learner's errors, and using words once there are words for it (`CUL-04`).
      Teaching makes copying faster and more exact, as measured in experiments on learning to knap.
    - **Building up:** imperfect copying makes variation, and copying the successful keeps the better kinds, so culture builds up over generations (`MND-17`).
  - **Why:** This is how a people becomes cleverer than any of its members (`MND-17`).
  - **Example:** The best knapper's technique spreads because others copy whoever succeeds.
    Small copying errors make each band's blades slightly different (`CUL-12`).

- `CUL-02` **Knowledge can be lost** *(Decided)*: Knowledge lives in heads and dies with them unless it is passed on.
  Small, isolated groups can lose skills, as may have happened in Tasmania (`MOM-02`).
  - **How it works:** knowledge exists only as records in minds, and later in marks (`CUL-03`); a person's records end at death, and a skill survives only if others have learned it, while unused skills and beliefs fade (`MND-18`).
    A group copies mostly from its best (`CUL-01`); in a small group the best may not be very good, and copies of copies decay, so whether a complex skill survives depends on the group's size and contact with others, as in the published model of Tasmania's losses.
    No rule makes it happen.

- `CUL-03` **Memory outside heads** *(Decided)*: Marks, symbols, writing and records can emerge, letting knowledge outlive the people who had it.
  Signs gain meaning the same way words do, by agreement (`CUL-04`): tally marks for counting, pictures that tell, and eventually signs that stand for words.
  - **How it works:**
    - **Marks are real things:** a scratch, paint, a knot or a notch is a change to a thing made by an action (`MAT-10`, `CUL-25`), seen by sight.
    - **Meaning by linking:** a mark comes to mean something when people link its look to a concept, as words are linked (`CUL-04`): by making it while attending to something together, and by others seeing and copying the link; one notch for each day becomes a tally that others can read.
    - **Pictures:** people who see a picture recall the things it shows (`MND-04`).
    - **Meaning lives with the link:** a sign keeps its meaning only while someone who knows the link can read it; after that the thing remains, but its meaning is lost.
    - **Open:** how far counting goes depends on number concepts forming, which is less well understood (`MND-04`).

- `CUL-16` **How things spread** *(Decided)*: Knowledge, words, styles and beliefs spread through contact: shared camps, marriages between bands, trade and conflict.
  Isolation makes groups drift apart.
  - **How it works:** everything cultural passes only when people are actually together and perceive each other (`CUL-01`, `CUL-24`): sharing a camp, a person moving to another band by marriage, meeting to trade, raids and captives.
    So distance, barriers (`WLD-10`) and people's own choices set how often two groups meet, and the less they meet, the more their copying errors pull them apart.

### 10.2 Language

- `CUL-04` **Language emerges** *(Decided)*: Words are labels a group agrees on, and they spread through use.
  Groups that separate drift into dialects, then separate languages.
  Language makes teaching faster and lets people talk about things that aren't there: plans, the dead, spirits.
  - **How it works:**
    - **A word** is a sequence of the language's sounds (`BIO-21`) linked in a person's mind to a concept (`MND-04`), with a strength for that link; a person can have several words for one concept, and one word for several.
    - **Speaking and hearing:** a speaker picks, for each concept, the word they link to it most strongly; a hearer matches the sounds to their own words, allowing small differences, and recovers the concepts; with shared attention (`MND-26`), the hearer links a new word to what both attend to.
    - **Agreement through use:** when a word works (the hearer does what was meant, or shared attention confirms it), both speaker and hearer strengthen that link and weaken its rivals; when it fails, it weakens, as in the naming-game models that show how groups come to agree on words.
    - **New words:** a speaker with no word for what they want to say makes one: new sounds shaped like the language's other words, or a compound of words it has (`CUL-17`), helped by pointing and gesture (`BIO-21`).
    - **Children** learn words fastest, from what is said about what they attend to (`MND-26`).
    - **Drift:** groups that rarely talk (`CUL-16`) make different choices and different sound changes (`CUL-17`); dialects become separate languages once their speakers no longer understand each other.
    - **What it makes possible:** teaching with words (`CUL-01`), and passing on beliefs about things that aren't there, such as plans, the dead and unseen beings, as told beliefs weighed by trust (`CUL-24`).

- `CUL-17` **Sounds, words and word order** *(Decided)*
  - **What:** Each language has its own sounds, words, word order and compound words, starting from the few dozen shared words and calls of the starting kit (`BIO-02`).
    Words drift through regular sound changes, so related languages share telltale patterns and form families you can trace.
    Richer grammar, such as word endings, may grow but is not promised (`RES-19`).
  - **How it works:**
    - **Sounds:** each language has its own set of sounds, each a set of articulation features (`BIO-21`), starting from the starting kit's (`BIO-20`).
    - **Regular sound change:** each speaker's way of saying a sound in a given position drifts a little, toward what is easier to say and with copying errors; such a change is a rule over a sound in its surroundings, not over one word, so once a group takes it up by the usual copying (`CUL-01`), every word with that sound in that position changes together.
    - **Families you can trace:** separated groups take up different changes, so their languages keep regular matches between them, as historical linguists find in real ones, and the family tree can be shown (see Presentation).
    - **Word order:** when a speaker says several concepts together (who did what to whom), they put them in an order; hearers use the order to tell the roles apart, and orders that work get copied, so each language settles on its own preferred orders.
    - **Compound words:** two words often said together for one thing become one word, and wear down in sound over time.
    - **Richer grammar:** endings and sentence structure could grow as common words wear down and fuse with others, but no tested model shows they will, so they are possible, not promised.
  - **Example:** After the eastern band crosses the hills, its words drift away from those of the band left behind.
    Centuries later, their words for water, fire and stone still differ in the same regular way, which shows they were once one language.

- `CUL-18` **Names** *(Decided)*: People, places, peoples and things are named in their own languages, often after events, features or traits.
  You see the original name with a translation (`PRE-12`), and later hear it spoken (`SND-03`).
  - **How it works:** a name is a word for one person, place, people or thing, linked to the concept of that one (`MND-04`), and made like any word (`CUL-04`): from new sounds, or as a compound of words for something about it, such as an event there ("where the boar died"), a feature ("red cliff") or a trait ("tall one").
    Names are made when people need to talk about something, by whoever speaks of it first, and spread by use.
    The game shows the original sounds with the meaning of their parts; a name whose parts no longer match any words is shown without one, and its old meaning stays in the scientist's view.

- `CUL-24` **Conversations** *(Decided)*: People tell each other things: warnings, questions, news, teaching and retold stories.
  What they say is held as meaning first (`MND-25`), and their language puts it into words (`CUL-04`).
  Being told something is weighed against one's own experience, by how far the speaker is trusted (`MND-24`).
  - **How it works:**
    - **Saying:** a speaker chooses what to tell, as meaning first: a belief, a memory, a plan, a question or a warning (`MND-25`), when they believe the hearer lacks it and want them to have it, or want something from them (`MND-23`); their language turns it into words in order (`CUL-17`), and it becomes sound in the air (`BIO-21`), heard by anyone in range (`BIO-18`).
    - **Understanding:** the hearer matches the words to their own (`CUL-04`) and recovers what meaning they can; words they don't share lose part of it, so misunderstandings are real.
    - **Weighing:** what is understood becomes a told belief or memory, its certainty set by trust in the speaker and how well it fits the hearer's own experience (`MND-27`); a retold story can replace a person's own details (`MND-18`).
    - **Questions and lies:** a question asks for a belief, and the hearer may answer; a speaker can also say what they don't believe, when they believe it pays (`MND-23`).

### 10.3 Belief

- `CUL-05` **Belief from explanation** *(Decided)*
  - **What:** Big unexplained events (death, sickness, storms, your interventions) demand a cause.
    When no physical cause is known, the inborn tendency to suspect a hidden someone (`MND-21`) suggests an unseen being.
    Beliefs that seem to work spread and last.
    They become ritual, gain specialists such as shamans and priests, and in time grow into religions with their own myths, rules and sacred places.
  - **How it works:**
    - **An unseen someone:** when an event matters (a death, a sickness, a storm, fire from the sky, or one of your acts, `GOD-06`) and no believed cause explains it, the hidden-someone tendency (`MND-21`) makes a weak belief that an unseen someone caused it (`MND-27`), tied to the event's own concepts, such as the sky and its fire.
    - **It grows like any belief:** later events of the same kind are tied to the same unseen someone; acts done before good outcomes, such as singing before a hunt, are credited by the usual learning (`MND-05`) and become things done to win its favour or turn away its harm; outcomes vary by chance, and vivid ones are remembered best (`MND-08`), so such beliefs can last.
    - **Shared:** told and retold (`CUL-24`), these beliefs spread by trust and by the pull to conform (`CUL-01`); a band's common beliefs about unseen beings are its religion.
    - **Ritual and specialists:** acts repeated to sway the unseen harden into rites (`CUL-06`); someone others believe knows the unseen better, through dreams, success or age, is asked, followed and rewarded, and the role becomes a shaman's or, later, a priest's.
    - **Myths, rules and sacred places** come from the same records: stories about the unseen (`CUL-11`), rules said to please or avoid them (`CUL-20`), and places tied to them, such as where lightning struck.
  - **Why:** Religion grows from the same machinery as discovery (`MND-05`), and your own acts become part of what people try to explain (`GOD-06`).
  - **Example:** Your lightning becomes a god (`MOM-03`).

- `CUL-19` **Dreams and the dead** *(Decided)*: Dreams of dead relatives can lead people to believe the dead live on in some form.
  That can shape burials, rites for ancestors and ideas of a soul.
  - **How it works:** sleep replays memories of the dead (`MND-12`), so a dead relative is seen acting and speaking in a dream, and a dream is remembered as an event (`MND-18`) that the mind does not always tell apart from waking life (estimated share).
    Each such dream, and each one told by others (`CUL-24`), feeds a belief that the dead person still exists somewhere (`MND-27`); that belief can lead to care for the body, gifts to the dead and rites for ancestors (`CUL-06`), and to the idea of a part of a person that lives on.

- `CUL-20` **Taboos** *(Decided)*: Beliefs can harden into rules about what not to eat, where not to go and what not to do.
  Some protect people by accident; others cost them dearly.
  - **How it works:** a rule is a belief that an act is forbidden or required, with a feared result (`MND-27`); it forms when an act is followed by harm (sickness after eating something, a death after entering a cave, `MND-05`) or when others tell it, and becomes the group's rule once it is shared and others punish or shun those who break it (`CUL-06`).
    Whether a rule helps depends on whether its cause was real: a ban on a poisonous plant protects, while a ban on a good food after a chance illness costs, and the mechanism can't tell which is which.
    Breaking a rule one holds brings fear and shame (`MND-19`), so rules can outlast the memory of why they began.

### 10.4 Society

- `CUL-06` **Institutions form from habit** *(Decided)*: Repeated behaviour hardens into shared, named things that people know, teach and enforce: a norm, a role, a rank, a rite.
  They can change, split and dissolve, and they become the named things the chronicle and overlays talk about, such as "the rite of first fire" or "the elders' council".
  - **How it works:**
    - **Shared expectations:** when many in a group do the same thing in the same situation (a shared habit, `MND-09`), each comes to believe the others will do it and expect it (`MND-23`); once people react to those who don't, with disapproval or punishment (`MND-26`), the shared expectation is a norm.
    - **Roles and ranks:** when particular people keep doing particular things for others, such as leading the hunt or tending the sick, others come to expect it of them, which makes a role; shared expectations of who defers to whom make ranks.
    - **Rites** are fixed sequences of acts done together at set times or events (`CUL-05`).
    - **Named, taught and enforced:** people come to have words for them (`CUL-04`), teach them to children (`CUL-01`) and enforce them.
    - **Change:** when behaviour changes, expectations follow; institutions split when groups split, and dissolve when nobody keeps them.
    - **Named for you:** the game finds these patterns in the records and names them for the chronicle and overlays, while the simulation itself never names them (see Presentation).

- `CUL-07` **Nothing social is scripted** *(Decided)*: Family and marriage rules, sharing, exchange, trade, leadership, alliances, conflict and war all come from people's interactions.
  - **How it works:** no rule in the code sets marriage, sharing, trade, leadership, alliances, conflict or war; each comes from minds choosing actions for their drives through their beliefs about the world and each other (`MND-09`, `MND-23`), and the inborn tendencies (`MND-21`, `MND-26`) are the only built-in social leanings.
  - **Check:** a code search finds no social outcome named in decision logic (`PRN-07`), and comparison runs show social patterns differing between cultures.

- `CUL-21` **Sharing and exchange** *(Decided)*: Food sharing, gifts, trade between bands, specialists, rules about who owns what and, perhaps one day, money.
  Each emerges from need and repeated habit.
  - **How it works:**
    - **Giving is an action:** passing a thing to someone (`BIO-21`) moves it to them.
    - **Why people give:** to kin and those they're attached to (`MND-26`), to repay a debt (`MND-24`), to earn regard (generosity that others see raises their respect), and because others expect it (`CUL-06`).
    - **Sharing food:** a big kill rots before one family can eat it (`WLD-21`), and sharing it leaves debts in others' records, so sharing pays off in lean times, as forager studies find.
    - **Exchange:** one thing is given for another when each side values what it gets more, by its own needs and beliefs; between bands it needs contact and trust (`CUL-16`), and repeated exchanges become habits and then norms, with set partners, places and times.
    - **Specialists:** someone much more skilled (`MND-06`), whose products others give things for, can spend more of their time on that skill.
    - **Ownership:** rules about who may take what form as norms (`CUL-06`), from repeated expectations such as "I made it", "I found it" or "this is our place".
    - **Money,** a thing many accept because others accept it, is possible, not promised (`RES-19`).

- `CUL-22` **Leadership and status** *(Decided)*: Depending on the culture, status comes from skill, generosity, age, success, fear or birth.
  Leaders, councils and chiefs emerge where a group needs to act together.
  - **How it works:**
    - **Status** is how much others regard a person (respect in their records, `MND-24`); it rises with whatever others value in that culture, whether skill, generosity, success, age, fear or a parent's standing, and what is valued is itself copied (`CUL-01`).
    - **Leaders:** when a group must act together (a hunt, a move, a fight), people follow someone they trust and respect who proposes a plan, expecting others to follow too (`MND-23`); done again and again, it becomes a role (`CUL-06`).
    - **Councils and chiefs:** groups that often decide together settle on fixed ways of deciding, as norms (`CUL-06`); leadership passes to a child when others come to expect it.
    - **Force:** boldness, dominance (`MND-20`) and strength can win status through fear, and others' anger at bullies (`MND-26`) limits it.

- `CUL-08` **Dark history can happen** *(Decided)*: Violence and war, captivity and slavery, sacrifice, cruelty, infanticide and cannibalism can emerge like anything else.
  Sexual acts stay abstract (`BIO-15`).
  What is shown is controlled by the content setting (`PRE-18`).
  - **How it works:** these come from the same mechanisms as everything else: violence is striking a person (`MAT-12`, `BIO-13`), chosen when a mind believes it serves its drives, such as fear, anger, status or hunger, at a cost it weighs (`MND-09`); captivity is holding someone by force; sacrifice is a killing believed to please an unseen someone (`CUL-05`); infanticide comes when parents believe a child can't be kept alive; and cannibalism comes from starvation or rite.
    None is a rule, and no action exists for sexual violence (`BIO-15`).

- `CUL-23` **Peoples** *(Decided)*: The game recognises peoples by what their members share (language, beliefs, customs and style) and names them by what they call themselves.
  Boundaries can be blurry and shift over time.
  Peoples split, merge and disappear.
  - **How it works:** the game measures how much groups share: words (`CUL-04`), beliefs (`MND-27`), customs (`CUL-06`) and style (`CUL-12`), together with how often they meet; people who share much and meet often are grouped into a people, by tuned thresholds.
    This is done by the views, never by the simulation (see Presentation), and is worked out again as things change, so one person can belong partly to two peoples, and peoples split, merge and disappear.
    A people is named by what its members call themselves (`CUL-18`), if they have such a name, and otherwise described.

### 10.5 Expression

- `CUL-25` **Expression is real** *(Decided)*: Each form of expression exists as a real thing in the world: paint on rock, marks on wood and bone, sound in the air, movement in a dance.
  What it holds is kept as content: a song as its notes and rhythm; a picture or map as what it shows and how (composition, style, skill and pigments), from which the game draws it.
  - **How it works:**
    - **A real thing:** paint is pigment on a surface (`MAT-10`), weathering by the laws (`RCK-16`); marks are cuts (`MAT-04`); a song is sound in the air, from voices or instruments; a dance is bodies moving (`BIO-21`).
    - **Made by skills:** drawing, carving, singing and dancing are skills (`MND-06`): the maker intends content drawn from their memories and beliefs, and their strokes land with their own error (`MAT-06`), so a clumsy painter's deer is harder to recognise.
    - **Content kept:** a picture's record holds what it shows and where, its style, the maker's skill and the pigments used; a song's, its notes, rhythm and words; a dance's, its sequence of movements.
    - **Perceived by others** through sight or hearing (`MND-03`), who recognise in it the things they know (`CUL-03`).

- `CUL-09` **Visual art** *(Decided)*: Paintings, carvings and body decoration composed from their own memories and myths, made with real pigments and tools (`RCK-15`, `RCK-16`) on cave walls, objects and bodies.
  What they depict reflects what matters to them (`MOM-07`).
  - **How it works:** a person makes a picture when their drives and beliefs favour it, such as play, regard from others, a rite (`CUL-05`), or a memory that matters, and its content comes from their strongest memories and beliefs at the time (`MND-18`).
    Pigments must be found and prepared, such as ochre ground and mixed with fat or water (`RCK-15`, `RCK-16`), and surfaces are real; style is copied from others (`CUL-12`).

- `CUL-10` **Music and dance** *(Decided)*: Rhythms, scales, songs and instruments that grow out of each culture.
  Instruments follow real acoustics (`MAT-03`), from bone flutes to drums of stretched hide.
  - **How it works:**
    - **Sound patterns:** voices and struck, blown or plucked things make sounds with pitch, loudness and timing (`BIO-21`, `MAT-04`); people repeat patterns they enjoy, pulled by the beat tendency (`MND-26`), and copy others' (`CUL-01`), so a group's songs come to share their scales and rhythms, which drift (`CUL-12`).
    - **Instruments:** anything that rings when struck, blown or plucked sounds by the vibration law (`SND-06`); people who notice that a hole or a length changes the pitch can learn to make the notes they want (`MND-05`).
    - **Scales:** which pitches sound well together comes from the physics of overtones, but which scale a culture uses is copied, never set.
    - **Dance:** moving together to a beat (`MND-26`), in sequences learned and copied like any skill.

- `CUL-11` **Myths and stories** *(Decided)*: Built from the band's own memories, beliefs and dreams, told and retold, and changing a little with each telling.
  - **How it works:** a story is a retold run of memories, beliefs or dreams (`CUL-24`); hearers keep it as a memory told by that person, and each retelling rebuilds it (`MND-18`), drifting toward what the teller believes and what moves the listeners (`MND-08`).
    Myths are stories about unseen beings and beginnings, built from the band's beliefs (`CUL-05`), memories and dreams (`CUL-19`); stories told often at gatherings become shared.
    They are kept as records, which the writer puts into words for you (`PRE-37`).

- `CUL-12` **Style and ornament** *(Decided)*: Each culture's look in tools, clothing and buildings, drifting over time, so objects could be dated by their style.
  - **How it works:** style is the settings people copy: the shapes, sizes and angles in a toolmaking skill (`MND-06`), and the patterns and forms in what they make (`CUL-25`), copied with small errors (`CUL-01`), so each group's style drifts at a pace set by copying error and contact (`CUL-16`).
    Ornament, such as marks, beads or paint, is added when drives favour it, such as regard from others or showing one's group (`MND-26`).
    Because the drift is gradual and every thing keeps its shape, things carry their time's style and can be dated by it (see Presentation).

- `CUL-13` **Their sky and calendar** *(Decided)*: Constellations they name, seasons they track, festivals they keep (`WLD-07`).
  - **How it works:** the sky is seen like anything else (`BIO-18`); its cycles, such as moon phases, where the sun rises and which stars rise at dawn, become beliefs about time (`MND-27`), such as "when that star rises at dawn, the salmon come", which plans can use (`MND-22`).
    Groups of stars become concepts when people point them out and name them (`CUL-18`), tied to stories (`CUL-11`); gatherings at set points of the year become festivals (`CUL-06`); and tallies of days or moons (`CUL-03`) make the counting reliable.

- `CUL-14` **Their maps and names** *(Decided)*: Places named in their own languages, and maps drawn the way they see the land.
  - **How it works:** places are named as people talk about them (`CUL-18`); a map is a picture (`CUL-25`) of places from the maker's mental map (`MND-18`), laid out by routes and travel times rather than true distance and showing what matters to them, such as water and danger, drawn when someone wants to show another a place.

- `CUL-15` **Remembered lives** *(Decided)*: Genealogies, and legends of remarkable people as their culture remembers them.
  - **How it works:** genealogies are kinship as people believe and tell it (`MND-24`, `CUL-24`), passed down while it is remembered and retold, with gaps and errors (`MND-18`); legends are retold stories about people whose deeds were memorable (`CUL-11`), changing with each telling and sometimes merging people or adding the unseen (`CUL-05`).
    The simulation keeps the true record too (`PRE-10`), so legend can be set against what really happened.

## 11. Presentation

### 11.1 Visual style

This is how the world looks.
It is written to stand on its own, without needing any image to understand it.

- `PRE-01` **Detailed pixel art** *(Decided)*: Everything on screen is crisp pixel art: limited colours, hard pixel edges, no blur and no smooth gradients.
  - **How it works:** the picture is drawn at a low resolution, about a quarter of the screen's in each direction (`PRE-22`), and enlarged by whole pixels with no smoothing; every colour comes from the material ladders of one palette (`PRE-20`), so there is no blur and no smooth gradient.

- `PRE-02` **Pixel-rendered 3D** *(Decided)*
  - **What:** The world is a real 3D world, drawn at low resolution and enlarged with hard pixel edges.
    It looks like hand-made pixel art but has real depth, scale and structure.
    The camera turns freely and zooms continuously.
  - **How it works:** the 3D scene is built from the simulation's own state: the land from its heights and 3D pieces at the detail each distance needs (`WLD-12`), things from their records' shapes (`MAT-10`), and plants and animals from their records or their patches' counts (`WLD-13`).
    It is drawn on the graphics chip at the low resolution of `PRE-01`, which was measured fast enough on the phone.
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
  - **How it works:** each material's ladder is made from its simulated colour (`MAT-03`) by choosing 4–7 palette colours from its darkest to its lightest shade, or taken from its hand-picked ladder.
    The light reaching each point of a surface (sun, sky, fire and shadow, `PRE-30`) picks the step, and only within a narrow band at each step's edge does a fine pattern, fixed in the surface's own coordinates, mix the two steps.
  - **Why:** Clean colour is what separates pixel art from a shrunken photograph.

- `PRE-21` **Outlines and lit edges** *(Decided)*
  - **What:** A one-pixel dark outline wherever one thing stands in front of another: people, animals, trees, rocks, the top edge of a cliff.
    A one-pixel bright edge where the sun or a fire catches a shape, such as the sunlit rim of a cliff or the fire-facing side of a person.
  - **How it works:** a dark pixel is drawn wherever the depth jumps between neighbouring pixels, where one thing stands in front of another; a bright pixel is drawn at a shape's edge where its surface faces the sun or a fire strongly (`PRE-30`).
  - **Why:** Crisp silhouettes keep small things readable on a phone screen.

- `PRE-22` **Stable pixels** *(Decided)*
  - **What:** Pixels never crawl or shimmer as the camera moves: the picture stays locked to its pixel grid, and turns ease to rest.
    One art pixel is always the same size on screen, in portrait and in landscape, so turning the phone only changes the framing.
    About 4 screen pixels make one art pixel.
  - **How it works:** the camera's position is snapped to whole art pixels, and turns ease to rest; each art pixel is a fixed block of about 4 by 4 screen pixels in both orientations.
    Pixels still crawl while the camera turns or zooms; the fix for that is chosen on a real world at the first visual review (`PRE-31`).
  - **Why:** Shimmering pixels are the most common flaw of 3D pixel art, and the first thing that makes it look cheap.

- `PRE-23` **Rock faces** *(Decided)*
  - **What:** Cliffs show their geology: horizontal rock layers of different thicknesses, irregular vertical cracks, a few long fissures, lichen, water stains, soot above inhabited caves, grass hanging over the top edge, and scree at the foot.
    The same layers continue underground (`PRE-25`).
  - **How it works:** a cliff is drawn from its rock column (`WLD-09`): each layer's rock and thickness, cracks spaced as its joints are, and long fissures from faults; lichen and water stains from how wet and how old the face is and which way it faces (`WLD-16`); soot where hearth smoke has settled above lived-in caves (`MAT-04`); overhanging grass from the plants of the patch at the top (`WLD-12`); and scree from the loose rock actually lying at the foot (`WLD-14`).
  - **Why:** Geology is part of the story (`WLD-14`).
    What people can find depends on what the land is made of, and the rock should show it.

- `PRE-24` **Real shapes** *(Decided)*
  - **What:** Overhangs, caves, rock shelters and, later, buildings have real depth.
  - **How it works:** caves, overhangs and shelters are the land's 3D pieces (`WLD-12`), and built things are drawn from the things and joints they are made of (`MAT-10`); inside, light comes only from openings and fires (`PRE-30`), so the depths stay dark.
  - **Example:** Looking into a cave mouth from an angle, you see its dark interior, the firelit floor and the hide windbreak across the entrance.

- `PRE-25` **Cut-away view** *(Decided)*
  - **What:** The ground can be sliced open to show what lies beneath: rock layers, soils, underground water, and the buried layers of past life (hearths, tools, bones, graves).
  - **How it works:** a slice along the line you choose is drawn from the rock columns (`WLD-09`), the soil layers (`WLD-27`), the ground water (`WLD-17`), and the buried things at their depths, in the layers that buried them (`MAT-08`).
  - **Why:** It is how you see geology and dig through history.
    The archaeology view (`PRE-09`) uses it.

- `PRE-26` **Water** *(Decided)*
  - **What:** Rivers meander and change width, with gravel bars, reeds, lines that follow the current, ripples at fords, glints of sun and drifting mist.
    From far away a river never becomes thinner than one or two art pixels, so it stays readable.
  - **How it works:** a river is drawn from its channel line and width (`WLD-15`) and its flow (`WLD-17`): lines that follow the current's direction and speed, gravel bars and reeds from the patches along it, ripples where it runs shallow, glints by the sun's angle (`PRE-30`), and mist where the weather makes fog (`WLD-16`); seen from far away, it is drawn at least one or two art pixels wide.

- `PRE-27` **People and animals** *(Decided)*
  - **What:** People and animals are small 3D figures drawn through the same pixel look and animated at a deliberate, sprite-like rhythm of about 8–12 poses a second.
    They look like crisp pixel art from any angle and turn properly with the camera.
    At the closest zoom, a person is about 40–60 art pixels tall: enough for a face, hair, clothing and gestures.
  - **How it works:** each figure is a 3D body made of parts, shaped by that body's own measurements and looks (`BIO-08`, `BIO-22`) and wearing what it actually wears; its pose comes from the action under way (`MAT-12`), animated from the action's settings (`MAT-06`), and its face shows its strongest feeling (`MND-19`).
    It is drawn through the pixel look, with poses changing 8–12 times a second.
  - **Why:** The simulation will produce actions nobody planned (`PRN-01`).
    Figures built from parts can perform any of them from any angle, without a new drawing for each.

- `PRE-28` **Readable from far away** *(Decided)*: As you zoom out, people become tiny outlined figures in strong clothing colours, then groups become small markers, then a camp becomes a glowing point.
  - **How it works:** by its size on screen (tuned thresholds), a figure is drawn in full, then as a tiny outlined figure in its clothing's strongest colour; a group close together becomes one marker at its centre, and a camp a glowing point at its hearth.

- `PRE-29` **From above** *(Decided)*
  - **What:** As the camera rises, it tilts toward looking straight down, and the land shifts into a clean map look: crisp colours for forest, grassland, rock and water, rivers as lines, shaded hills.
    Map overlays (`PRE-07`) sit on this view.
    At the very top, the whole world appears as a globe (`WLD-02`).
    Close up to globe is one continuous zoom (`PRE-03`).
  - **How it works:** as the camera rises past set heights, its tilt eases toward straight down and the land's drawing blends into the map look: each patch in a flat colour for its cover (`WLD-12`), rivers as lines, and hills shaded from the heights.
  - **Why:** A landscape seen from high up at an angle turns to mush.
    A map stays clear at every height.

- `PRE-30` **Light, time and season** *(Decided)*
  - **What:** One master palette, with versions for each time of day (dawn, day, dusk, night) and each season.
    The sun casts real shadows, the sky tints everything, and distance adds haze.
    A fire lights its surroundings with a warm, flickering glow that fades with distance, warms the faces of people nearby, and sends up smoke and embers.
  - **How it works:** the sun's and moons' places in the sky (`WLD-07`) set the light's direction, and the time of day and season pick the palette's version, blended through the changes; shadows come from the 3D scene, and haze grows with distance.
    Each fire is a light whose brightness comes from the heat it gives off (`MAT-04`), flickering as its burning varies and fading with distance, with smoke and embers from what it burns.

- `PRE-03` **Seamless zoom** *(Decided)*: One continuous zoom from the whole world, drawn as a globe, down to one person chipping flint.
  - **How it works:** one camera rises continuously from a person's height to the globe; the drawing changes with on-screen size (`PRE-28`, `PRE-29`), and detail made for the picture streams in from the simulation's own generator (`WLD-13`), so there is never a loading break.

- `PRE-04` **Sharp at every zoom** *(Decided)*: The pixel art stays sharp and readable at every zoom level (`PRE-22`, `PRE-28`, `PRE-29`).
  - **How it works:** the art pixel never changes size (`PRE-22`), small things switch to forms that stay readable (`PRE-28`), and high views become the map (`PRE-29`).

- `PRE-31` **Visual review** *(Decided)*
  - **Done when:** at every milestone, screenshots at each zoom level, in both orientations and at every time of day, pass a review for:
    - clean colour, with no speckled surfaces;
    - crisp silhouettes;
    - pixels that stay still while the camera moves;
    - people and animals readable at phone size.
  - **How it works:** a tool captures the screenshots on the phone from a fixed set of saved worlds, at each zoom, in both orientations and at each time of day; the review checks them against the list, and you take part as the final judge (`PRC-10`).

### 11.2 On the screen

- `PRE-32` **World first** *(Decided)*
  - **What:** The world fills the screen.
    Controls and panels appear only when you ask for them: tap a person, animal, group or place to open its card, or swipe up for the chronicle and other views.
    Nothing stays on screen unless you called it up, apart from a live moment appearing briefly (`PRE-08`).
    A brief touch shows the date, the real speed of time and the time control (`PRE-33`).
  - **How it works:** nothing is drawn over the world until you call it: a tap opens the card of what is under your finger, a swipe up opens the views, and both close when dismissed; the brief touch shows the date, the real speed and the time controls, which fade after a few seconds (tuned).
  - **Why:** The world is the point.
    It should feel like looking at a living place, not at a dashboard.
  - **Example:** You open the app to nothing but the valley at dusk, exactly as you left it.

- `PRE-34` **Both orientations** *(Decided)*: Every screen works one-handed in portrait and two-handed in landscape (`VIS-14`).
  Follows from `PLT-02`.
  - **How it works:** each screen has a portrait layout, with its controls within one thumb's reach at the bottom, and a landscape layout for two hands; the art pixel keeps its size in both (`PRE-22`).

- `PRE-33` **Gestures** *(Decided)*:
  - drag to move, and twist with two fingers to turn;
  - pinch to zoom, or double-tap and drag with one thumb, which also sets the speed of time (`TIM-01`);
  - tap to select;
  - long-press for your powers at that spot (`GOD-10`), including drawing an area, so a drag always moves the camera;
  - swipe up from the bottom edge for views;
  - a brief touch anywhere shows the date, the real speed of time and the time control: pause, speed and speed lock (`TIM-04`).
  - **How it works:** each gesture is told apart by its number of fingers, its length and its path; a tap selects the nearest thing under the finger, and zooming also sets the speed asked for (`TIM-01`).

- `PRE-35` **Cards** *(Decided)*: Selecting anything opens a card with what matters about it, such as a person's name, age, mood, and what they are doing and why, or a place's land and history.
  Links lead into deeper views: the story view, the scientist's view, family trees, archaeology.
  - **How it works:** a card reads the chosen thing's records: for a person, their name (`PRE-38`), age, strongest feelings (`MND-19`), and current activity with the reasons recorded for choosing it (`MND-09`); for a place, its land (`WLD-12`) and the events recorded there; for a thing, its makeup and history (`MAT-10`); each link opens a view on the same thing.

- `PRE-40` **Screens** *(Decided)*: Besides the world itself: a first-launch screen, a list of your worlds, and settings.
  Short help cards appear the first time you use something; there is no tutorial (`SCP-02`).
  - **How it works:** the first launch goes straight to making a world (`WLD-10`); the list shows your worlds (`TIM-08`); settings hold the content level (`PRE-18`) and the live-moment level (`PRE-08`); and each help card shows once, the first time its control is used.

### 11.3 Following the story

- `PRE-05` **Chronicle** *(Decided)*
  - **What:** An automatically written history of the world, organised as a book of ages, with a timeline for each people.
    Eras are named by their own people, or after the events that defined them.
    Every entry links to the moments and people behind it (`VIS-15`).
  - **How it works:** the chronicle is built from the recognisers' tagged events (`PRE-39`): events above a tuned importance become entries, grouped into ages that begin at turning points set by fixed rules (`PRE-41`), on one timeline for each people (`CUL-23`); each entry is written by the writer from its records (`PRE-37`) and links back to them.
    An era takes the name its people have for that time, if they have one (`CUL-18`), and is otherwise named after its defining events.
  - **Why:** It is the main way to read a world's history, and the measure of "histories worth reading".

- `PRE-06` **Follow a soul** *(Decided)*: Pick anyone and follow their life: their card shows what they feel, want and think (`PRE-14`), and the camera can stay with them.
  The people you follow are kept in a list, separate from the camera, so you can follow several and still look elsewhere.
  When they die, the game offers to follow someone close to them.
  - **How it works:** the list holds the people you follow; their cards update as they live, the camera can lock onto one of them, and at a death the game offers those with the strongest bonds to the dead (`MND-24`).

- `PRE-07` **Map overlays** *(Decided)*: Information shown spread across the land.
  The overlays: beliefs; knowledge, meaning who knows which skill; moods; languages and dialects; family ties; territories and paths; food and water; disease; climate and seasons; rock and resources.
  - **How it works:** each overlay colours the land from the records at that date: the share of people in each place holding a belief, and how strongly; who holds which skill (`MND-06`); average feelings (`MND-19`); languages and dialects as measured (`CUL-23`); kinship lines (`MND-24`); home ranges and paths from where people actually go; food and water from the patches (`WLD-12`); the sick; the weather and seasons (`WLD-16`); and surface rock and ores (`WLD-14`); each has a story and a scientist's version (`PRE-14`).

- `PRE-08` **Live moments** *(Decided)*: Only what matters interrupts you: firsts, deaths of people you follow, disasters, and big turns in history ("someone has made fire for the first time").
  What can interrupt is one shared list, also used by the story director (`TIM-02`).
  At most about one interruption comes a minute, and the closer you are watching, the more important something must be to interrupt.
  Live moments you don't take wait in a list; everything else waits in the chronicle.
  The level can be adjusted in settings, and one tap takes you to the moment.
  - **How it works:** each tagged event carries the director's importance score (`TIM-02`); it interrupts only if its score passes a threshold that rises the closer you are watching and with your setting, and no more often than about once a minute; the rest wait in the list, and a tap takes the camera there.

- `PRE-39` **Recognising what emerges** *(Decided)*
  - **What:** The game spots and names what emerges, for you only: firsts and discoveries, skills, languages, institutions, peoples and eras.
    It uses both general detection of anything new and a catalogue of notable outcomes, such as fire made by friction.
  - **Rules:** recognisers sit on the describing side.
    They never feed back into the world, and are kept provably apart from the logic that decides what people and animals do (`PRN-07`).
    Their thresholds, such as when a dialect becomes a language, are set in the implementation plan and listed in milestone reports.
    A first counts both worldwide and for each people, and a rediscovery after a loss is marked as one.
  - **How it works:**
    - **Anything new:** each recorded outcome, such as a thing with a new mix of properties, an act never recorded before, or a belief newly shared by many, is compared by its pattern of properties with the world's history; one never recorded before is a first, for the world and for that people, and a first that a people once held and lost is marked as a rediscovery.
    - **Notable outcomes:** a catalogue defines each by a pattern in the records, such as a fire whose ignition came from a person rubbing wood (`RCK-02`), and gives the names you see.
    - **Skills, languages, institutions, peoples and eras** are found by measures over the records (`MND-06`, `CUL-17`, `CUL-06`, `CUL-23`).
    - **Kept apart:** recognisers read finished records after each step and write only to the describing side (`PRN-07`).
  - **Why:** The director (`TIM-02`), live moments (`PRE-08`), the chronicle (`PRE-05`), overlays (`PRE-07`) and experiment measures (`RES-03`) all need to know what happened, without the simulation ever naming it.

- `PRE-09` **Archaeology** *(Decided)*
  - **What:** Dig down through the buried layers of past life with the cut-away view (`PRE-25`): hearths, graves, lost tools, rubbish heaps.
    Tap a find to see who made or left it, and when.
  - **How it works:** the cut-away (`PRE-25`) shows what the trace laws kept, in its layer (`MAT-08`); tapping a find reads its record: who last made or changed it and when (`MAT-10`), or, for merged leftovers, who left them and when.
  - **Example:** The dig in `MOM-09`.

- `PRE-10` **Family trees and legends** *(Decided)*: Genealogies across generations, and the legends their culture keeps.
  Each legend can be set side by side with what really happened.
  - **How it works:** the true tree comes from the birth records, each child with its parents (`BIO-15`); the culture's own genealogies and legends (`CUL-15`) are shown beside it, each linked to the true events it tells of.

- `PRE-11` **Their sky and calendar (view)** *(Decided)*: The sky as they understand it: their constellations, the seasons they track, their festivals.
  - **How it works:** the real sky (`WLD-07`) is drawn with the people's own star groups over it, from their concepts and names, and with the cycles and festivals they keep, from their beliefs and rites (`CUL-13`).

- `PRE-12` **Their maps and names (view)** *(Decided)*: Their place names with translation, and the maps they draw, compared with the real land.
  - **How it works:** place names come from their words, with the meaning of each part translated (`CUL-18`); their drawn maps (`CUL-14`) are shown beside the real land, matched place by place.

- `PRE-14` **Two views of every mind** *(Decided)*: A story view in their own words, and a scientist's view of their raw beliefs, how certain they are, and the evidence behind each belief.
  Other views, such as the chronicle, the map overlays and archaeology, also have a story version and a scientist's version, switched separately in each view.
  Story versions never show your interventions (`GOD-07`).
  - **How it works:** the story view hands the person's records (feelings, wants, beliefs and memories) to the writer, phrased through their own concepts (`PRE-38`); the scientist's view shows the records raw: each belief with its certainty and evidence (`MND-05`), drives and feelings as numbers, and the reasons recorded for each choice (`MND-09`).

- `PRE-15` **Art that remembers** *(Decided)*: Tap a painting or carving to see it, what its maker meant, and the event or myth it depicts.
  You can then read what really happened, from the saved events (`PRN-15`).
  - **How it works:** a painting's content record (`CUL-25`) gives what it depicts and the memories or myths its maker drew on; these link to the event or story records (`PRN-15`).

- `PRE-16` **Bestiary** *(Decided)*: Each world's tree of life and its species.
  - **How it works:** each species is shown from its records (`WLD-19`): its traits, body chemistry and range in the scientist's view, and its place in the world's tree; the story view shows what each people believes about it and calls it (`MND-04`, `CUL-18`).

- `PRE-36` **Language family tree** *(Decided)*: How their languages split and drifted over time (`CUL-17`).
  - **How it works:** the tree is drawn from the measured history of languages: a branch splits, with its date, when two groups' words drift past the threshold for separate languages (`PRE-39`), and example words show their regular sound changes (`CUL-17`).

- `PRE-13` **Every view the simulation allows** *(Decided)*: Any further view the simulation's data supports, within physical limits (`PRN-04`).
  - **How it works:** every kind of record can be shown raw in the scientist's view, and new views are added from the same records without touching the simulation (`PRN-04`, `PRN-14`).

### 11.4 Text written for you

- `PRE-17` **Descriptions stick to the data** *(Decided)*: The AI language model only turns simulation data into text: life stories, myths, dreams, the chronicle.
  It never adds facts the simulation doesn't contain (`PRN-06`).
  - **How it works:** the writer receives only the records a text is about and the voice to use (`PRE-19`), and is told to phrase them and add nothing; every text is checked against those records before it is shown (`PRE-41`); and dark events are never left to it: they are stated as plain facts taken from the data.
  - **Check:** the fact checker runs on every text before it is shown, and a sample of texts is reviewed at each milestone for added or changed facts.

- `PRE-37` **The writer AI runs on the phone** *(Decided)*: All text is written on the phone, fully offline, with no running cost.
  If the writing turns out too plain for histories worth reading (`VIS-15`), that is raised at a milestone review.
  - **How it works:** the phone's own built-in language model writes all text, offline and at no running cost; dark events never go to it (`PRE-17`), and its prompts are fixed when the writer is built.

- `PRE-41` **How text is written** *(Decided)*: Text is written when it is first opened or during pauses, checked against the data it came from, stored, and never silently rewritten.
  You can ask for a rewrite.
  Text that fails its check is replaced by plain factual text.
  What the chronicle covers, and where its ages begin, come from fixed rules (`PRE-39`), not from the writer's taste.
  The writer chooses words and rhythm, never content: every claim, cause, motive, image and name must be in the data (`PRE-17`).
  - **How it works:** text is written when first opened, or while the phone is otherwise idle, and checked by a fact checker that compares every name, number, cause and event in it with its records, with no language model involved; text that passes is stored beside its records and never rewritten unless you ask, and text that fails is replaced by plain factual text built from the records by fixed patterns.

- `PRE-38` **English** *(Decided)*: The interface, the chronicle and translations are in English.
  Their own words appear in their own languages, with English translations (`PRE-12`).
  Until their own names emerge, people, places and peoples get labels made from that world's own sounds, marked as the game's (`CUL-18`).
  English text describes things through their concepts, such as "cutting stone" (`MND-04`); our own words for them, such as "flint", appear only in the scientist's view.
  - **How it works:** until people name something, it gets a label made from that world's own sounds (`CUL-17`), marked as the game's; English text names things by the people's concept, described by the properties that define it for them (`MND-04`), or by a translation of their word for it.

- `PRE-19` **Storytelling voices** *(To test)*: Documentary, archaeologist, their own tradition, and intimate.
  Each is tried live on real simulation output and chosen by ear.
  Different views may use different voices.
  - **How it works:** each voice is a fixed set of instructions to the writer (who speaks, tone and tense); the same real records are written in each voice, and you choose by reading and listening, view by view.

### 11.5 Content

- `PRE-18` **Content setting** *(Decided)*: You choose how much of history's darker side is shown, at one of three levels:
  - **Show:** everything, with pictures and sounds;
  - **Plain:** no graphic pictures or sounds, and factual text;
  - **Gentle:** dark events mentioned briefly, in the chronicle only.

  The simulation underneath never changes (`CUL-08`), and bodies are drawn without sexual detail at every level.
  - **How it works:** the recognisers tag how dark each event is (violence, injury, captivity, sacrifice or cruelty, `PRE-39`), and the setting filters only what the views show: everything; no graphic pictures or sounds, with injuries drawn without detail and text kept plain; or a brief mention in the chronicle only.

## 12. Sound

Sound comes in layers, added over time, starting with the living soundscape.
Like everything you see, everything you hear reflects what is actually happening (`PRN-10`).

### 12.1 The layers

- `SND-01` **Living soundscape** *(Decided)*
  - **What:** Wind, rain, rivers, animals, fire and people at work, driven by what is actually happening where you're looking.
    Zoom changes the mix: close up you hear single sounds; further out they blend; from the whole world, near silence.
  - **How it works:** every sound comes from something simulated near the camera: a strike or a break (`MAT-04`), a fire's burning, a river's flow (`WLD-17`), the wind and rain of the weather cell (`WLD-16`), an animal's call, people at work, and voices.
    Close up, single sounds play, the loudest at the camera first, up to the measured limit of 32 at once on the phone; further out, the sounds of one kind in an area are summed into one blended sound from their number and loudness, such as wind over a forest or the hum of a camp; from the whole world, near silence (`SND-09`).
    Calls of counted animals (`WLD-12`) come at their species' calling rates from the animals counted there, filled in as the picture is (`WLD-13`).
  - **Example:** At the camp at dusk: the crackle of the fire, the tap of the knapper's hammerstone, a child laughing, the river beyond, a wolf far off.

- `SND-03` **Their voices** *(Decided)*: Zoomed in, you hear real speech: actual sentences in their language, spoken with its own sounds and word order (`CUL-17`), with English subtitles if you want them.
  Further out, talk blends into a murmur.
  - **How it works:** when a person speaks (`CUL-24`), their words are said in order (`CUL-17`), each as its sequence of the language's sounds (`BIO-21`), by a natural-sounding speech voice fed those sounds, chosen over a synthetic one by your ear in the pre-tests, with pitch and quality from the speaker's body, age and feeling (`MND-19`); subtitles give the meaning in English (`PRE-38`).
    Voices beyond a set distance are mixed without words, as a murmur.
    Such voices bend unfamiliar sounds toward their training language, so the voice used is the one that keeps most of each language's sounds.

- `SND-02` **Their music** *(Decided)*: Songs, rhythms and instruments from each culture (`CUL-10`), heard when you are near.
  Their scales and rhythms develop and drift, as their languages do.
  - **How it works:** songs and tunes are played from their content records (`CUL-25`): notes, rhythm and words, with instruments sounding by the physics of their shapes and materials (`SND-06`) and voices by the speech voice held on the notes' pitches; they are heard by distance like any sound (`SND-08`).

- `SND-04` **Score** *(Decided)*: Background music generated live from the world.
  It is assembled from short themes that respond to time of day, season, events and the people nearby, and it draws on their own scales and rhythms as their music develops.
  It is never the same twice.
  - **How it works:** short composed themes are chosen and varied live from the world's state: time of day and season, the director's current scores (`TIM-02`), events such as a death or a first, and the moods of people nearby (`MND-19`); once a people's own music exists (`CUL-10`), the score takes up its scales and rhythms from their songs' records, and variation is drawn so it never repeats exactly.

- `SND-05` **Order of the layers** *(Decided)*: The soundscape comes first, then their voices, then their music and the score.
  The implementation plan sets when each arrives.
  - **How it works:** each layer is a separate source in one mixer, switched on when it is built, in this order.

### 12.2 How sound is made

- `SND-06` **Sounds from the physics** *(Decided)*
  - **What:** Impacts, fire, water and instruments are created from what things are made of (`MAT-03`).
    A strike on flint sounds unlike one on granite, and an instrument they invent sounds the way its materials would.
    Background wind and rain can use recordings, and so can birdsong, but only where matching birds are simulated.
  - **How it works:**
    - **Impacts:** each time a contact law runs (a strike, a break, a fall), its sound is made as noise shaped by the materials' stiffness, density and damping, the things' sizes and the force of the impact (`MAT-03`, `MAT-04`), the method your ear chose in the pre-tests, so flint and granite sound different.
    - **Fire and water:** a fire's crackle and roar come from how fast it burns and what it burns (`MAT-04`); water's sound from its speed and depth (`WLD-17`).
    - **Instruments:** notes come from shape and material by the vibration law (`MAT-04`); flute notes from a bore and holes matched their worked-out pitch within a few cents in the pre-tests, and drums are still to be tuned.
    - **The phone's speaker:** a last step tuned to the speaker lifts deep sounds it can't play well, and switches off with headphones.
  - **Why:** General rules (`PRN-07`) apply to sound as well.
    Nobody has to record the sound of an instrument nobody planned.

- `SND-07` **Sound follows time** *(Decided)*: At natural speed (`TIM-10`), every sound plays in real time.
  When time runs fast, single sounds give way to the feel of the period: seasons of wind and rain, the hum of a busy camp.
  - **How it works:** at natural speed, each sound plays as its event happens; when time runs faster, the mixer plays each kind of sound as a blend at the rate its events are happening (`SND-01`), so the weather's sounds follow the state of each frame and a camp becomes its hum.

- `SND-08` **Space and distance** *(Decided)*: Sounds come from where they happen and fade and muffle with distance; caves echo.
  A sound can draw your attention to something off-screen, such as a scream or thunder.
  - **How it works:** each sound plays from its place in 3D: placed by its direction from the camera, quieter with distance, its high pitches fading faster, muffled by land in between, and echoing in caves by their size and shape (`PRE-24`); a loud sound off-screen plays from its direction.

- `SND-09` **Silence** *(Decided)*: Quiet is part of the design.
  Nights are hushed, deep snow muffles everything, and the whole world seen from above is close to silent.
  - **How it works:** quiet comes from the same rules: at night fewer things make sound, since people and most animals sleep; snow cover soaks up sound by its measured absorption; and from high above only blends remain, fading with height; nothing adds sound where nothing happens.

### 12.3 Touch

- `SND-10` **Vibration for big moments** *(Decided)*: Subtle and optional: thunder, an earthquake, the heartbeat of someone you follow when they are in danger.
  - **How it works:** when the setting is on, the phone's vibration plays a pattern for thunder near the camera, its strength from the thunder's loudness; for an earthquake, from the shaking where the camera is (`WLD-15`); and for someone you follow, a heartbeat while their fear is high (`MND-19`).

## 13. Platform and performance

Kindling is built for one phone, and nothing else is used to play it (`SCP-02`).
The phone must stay smooth, cool and responsive (`VIS-14`, `PRN-11`).
How much simulation fits on it is found by measuring, not guessing.

### 13.1 The phone

- `PLT-01` **One phone** *(Decided)*: Built and optimised for your Pixel 11 Pro XL (16 GB of memory and 512 GB of storage, so the app can use about 10 GiB), and free to use that phone's specific hardware wherever it helps: its graphics chip for the pixel-rendered 3D (`PRE-02`), its AI hardware for the writer AI (`PRE-37`), and either of them for the simulation itself (`PLT-05`).
  - **How it works:** the app is built for that phone's processor, graphics chip and memory: the simulation runs on its cores and, for grid systems, its graphics chip (`WLD-12`); the picture on the graphics chip (`PRE-02`); and the writer on its AI hardware (`PRE-37`); everything fits in about 10 GiB, as measured (`PLT-04`).

- `PLT-02` **Portrait and landscape** *(Decided)*: Both are supported, and the layout adapts (`PRE-34`).
  - **How it works:** turning the phone switches between the two layouts of `PRE-34`, keeping the world, the camera and the art pixel's size (`PRE-22`).

- `PLT-03` **Works offline** *(Decided)*: Everything works without a connection, including the text descriptions, which are written on the phone (`PRE-37`).
  - **How it works:** everything the game needs is on the phone: the simulation, the catalogues, the writer and the voices; nothing in play makes a network call.

- `PLT-06` **Installing new versions** *(Decided)*: Each new version is a file you download on the phone and install, after allowing installs from your browser once.
  Builds are signed for your free hobbyist developer account with Google, so they keep installing this way under Android's developer rules from 2027 (`RSK-18`).
  No store and no fees.
  Each milestone report links to its version.
  - **How it works:** each build is signed for your developer account and published as a file linked from its note or report (`PRC-11`); you download and install it, and worlds carry over by `PLT-09`.

### 13.2 Performance

- `PLT-04` **Measured limits** *(To test)*: Measured from the first build and reported at every milestone:
  - smoothness of zooming and panning;
  - simulated time per real minute at each zoom level;
  - how many people the phone can run at each level of detail;
  - battery use and heat per hour of play;
  - time to generate a world.

  The targets, from `VIS-14`: the camera stays smooth at the screen's full refresh rate; the app opens to your world in about three seconds; an hour's session uses about 25–30% of the battery, without the phone getting uncomfortably hot.
  - **How it works:** at each milestone, a benchmark on the phone records frame times while zooming and panning, simulated time per real minute at each zoom, how many people run at each level of detail and speed, battery use and temperature per hour from the phone's own counters, and the time to generate and settle a world (`WLD-11`); the results go into the report (`RES-06`).

### 13.3 Worlds on the phone

- `PLT-07` **Always saved** *(Decided)*: Worlds save continuously, so closing the app or a flat battery never loses anything (`TIM-05`).
  - **How it works:** each event is added to the history log as it happens (a small addition took under a tenth of a millisecond on your phone in the pre-tests); the present state is saved whenever the app leaves the screen (`TIM-05`) and at set intervals, each time as a new file that replaces the old only once complete, so a damaged file is never loaded (in the pre-tests, 1,000 kills mid-write never left one).
    After a crash, the world reopens at its last save and runs forward to where it stopped, repeating exactly (`TIM-16`), so nothing is lost.

- `PLT-08` **Manual export** *(Decided)*: Export a world, with its present state and its chronicle, as a file whenever you want, and import it again on the same phone or a new one.
  The export keeps the full record, so an imported world opens exactly as it was.
  There are no automatic backups.
  - **How it works:** export packs a world's whole folder (seed and generator version, present state, history log, stored things and written text, `TIM-08`) into one file; import checks the file and unpacks it, and the world opens exactly as it was.

- `PLT-09` **Worlds across updates** *(Decided)*
  - **What:** The game's rules will keep growing (`PRN-14`).
    After a small update, a world carries on: everything that already happened stays as it was, the world continues under the new rules, and the change is marked in its chronicle.
    A big update, one that adds a new layer of the world such as new matter, species or systems, may need a new world.
    Worlds are only promised to last between big updates.
  - **Why:** Fitting a new layer into a running world would be costly, and could make its past dishonest.
    Starting a new world keeps every world true to one set of rules.
  - **How it works:** each world records the version of the rules it runs under, and each update declares itself small or big; after a small update, worlds carry on under the new rules with the change marked in their chronicle; after a big update, including any change to the generator (`WLD-08`), an older world's chronicle can still be read if its files can be, but carrying it on needs a new world.

- `PLT-10` **Storage** *(Decided)*: Each world keeps its present state and its event history (`PRN-15`).
  The event history thins with age by a fixed rule: recent years keep every event, and older history keeps what the chronicle and the views use, such as births, deaths and firsts.
  You can delete worlds.
  When the phone nears full, the game warns you and asks what to delete; it never deletes anything else by itself.
  - **How it works:** the history log keeps every event for recent years, and for older years only the kinds the chronicle and the views use (`PRE-39`), by a fixed rule (tuned); a storage check warns before the phone fills.

### 13.4 The cloud

- `PLT-05` **Experiments in the cloud** *(Decided)*: The simulation also runs without graphics in the AI's cloud sessions, many runs at a time.
  The phone build comes first and is optimised for the phone; the cloud build doesn't have to match it exactly, only behave the same statistically (`RES-05`).
  An experiment's world can be opened on the phone as it stands at the end of its run.
  Follows from `SCP-15`.
  - **How it works:** the same simulation core is built for the cloud without picture or sound, runs many sandboxes at once, and writes the same saved files, which the phone opens (`TIM-08`); its statistics are compared with the phone's at every milestone (`RES-05`).

## 14. Research and validation

This section turns "research standard" into practice: how the project proves that its world really does what it claims (`PRN-05`).

### 14.1 How experiments work

- `RES-01` **Experiments lead** *(Decided)*
  - **What:** Core ideas are proven in experiments, run without graphics, before the game builds on them (`SCP-03`): first in sandboxes, then confirmed in full worlds (`RES-21`).
  - **Why:** The biggest risk is that nothing emerges (`RSK-01`).
    Experiments find out early and cheaply.
  - **Check:** every milestone report traces its features to experiments that passed (`RES-06`).

- `RES-21` **Sandboxes, then full worlds** *(Decided)*
  - **What:** Most experiments run in sandboxes: small settings built for one question, such as a band on a riverbank with flint, granite and decoy stones, or a winter camp whose fire is dying.
    A sandbox uses the game's own rules and minds, with no special rules and nothing scripted inside it (`PRN-07`); only its setting is chosen, and it includes decoys and materials nobody designed for.
    Each experiment states its computing budget up front, and its sandbox is sized to fit it (`RES-16`).
    Sandbox runs are cheap and repeat exactly from their seed (`TIM-16`), so each question gets many runs.
    At every milestone, one or two full worlds from the play generator, run overnight on your phone, confirm that what the sandboxes showed also happens in a real world, within the sandboxes' ranges.
  - **Why:** Whole worlds are far too costly to run by the hundred (`RSK-14`), while a sandbox answers one question cheaply and repeatably.
    The full worlds guard against a sandbox so well arranged that it makes the result likely by design.
  - **Check:** every claim names its sandbox and its full-world confirmation, and a result seen only in sandboxes is reported as such.

- `RES-08` **What every experiment has** *(Decided)*: A question; a setup (the sandbox or world settings, starting kit, population, length); the number of runs; what is measured; pass and fail criteria; and comparison runs.
  - **How it works:** each experiment is a file with these fields, kept in the repository, and the runner refuses one with any field missing.

- `RES-09` **Criteria fixed first** *(Decided)*: Each experiment's pass and fail criteria, with exact numbers and definitions, are written down before it runs, checked by the independent reviewer (`RES-11`) for ways the experiment couldn't fail, and approved by you.
  They are never adjusted afterwards.
  - **Check:** the criteria file is committed and approved before the first run, and every run records which version of it was used.

- `RES-10` **Comparison runs** *(Decided)*: Each experiment also runs with one mechanism switched off, such as imitation, to show that what emerged depends on it.
  - **How it works:** each mechanism can be switched off by a setting that exists only in experiments (`PRN-12`), and every report includes the run without it.

- `RES-11` **Independent review** *(Decided)*: A separate AI agent, not the one that built the experiment, checks it and tries to find flaws in the results.
  - **Check:** each report carries the reviewer's findings and names a reviewer other than the builder.

- `RES-12` **Surprises log** *(Decided)*: Unexpected results are recorded even when they weren't the question.
  They often become new signature moments (`MOM`).
  - **How it works:** the runner flags every measure outside its expected range and every first the recognisers find that the question didn't ask about (`PRE-39`); each goes into the log and into the report's surprises.

- `RES-13` **Many runs, reported as ranges** *(Decided)*: Every claim rests on many runs (about 20 sandbox runs per setup unless stated, as many as the experiment's computing budget allows), confirmed in one or two full worlds (`RES-21`), and is reported as a range, for example "discovered in 13 of 20 runs; typically around year 140".
  - **Check:** every claim in a report states its number of runs and its range.

- `RES-14` **Compared with reality where possible** *(Decided)*: Where real-world data exist, such as hunter-gatherer populations or rates of learning and cultural change, results are compared with them.
  - **Check:** wherever a measure has named real-world data, the report shows the comparison.

- `RES-16` **Tuning and failure** *(Decided)*
  - **What:** Values are tuned on development seeds, then confirmed once on fresh seeds kept back for that.
    Every attempt and every tuned value is logged, with what it was tuned against.
    A test may stop early once its result is clear, within a stated computing budget.
  - **When it fails:** a failed confirmation holds the milestone until you choose: redesign, a weaker claim, or dropping the claim.
  - **Why:** Re-running on the same worlds until something passes would make "experiments that can fail" meaningless (`PRN-05`).
  - **How it works:** the runner draws development seeds and held-back seeds from separate pools, logs every attempt and every tuned value, and refuses to run the held-back seeds a second time for the same claim.

- `RES-17` **Signature moments keep passing** *(Decided)*: Each signature moment has its own sandbox, and passes if it happens in at least 1 run in 10 within its time window, unless its own criteria say otherwise (`RES-21`).
  Each moment's sandbox runs at the milestone it belongs to, and again only when something it depends on changes; before a merge, only the short ones run, and a failure blocks the milestone (`PRC-10`).
  Each milestone report also says which moments appeared in its full worlds.
  - **How it works:** each moment's sandbox and pass rule are files beside its experiment (`RES-08`); the merge check runs the short ones, and each milestone runs those that are due (`PRC-10`).

- `RES-18` **Same rules as play** *(Decided)*: Sandboxes use the same rules as play, and the full worlds that confirm them come from the play generator (`WLD-10`, `RES-21`).
  Scripted events and dials appear only in clearly labelled experiments, and a moment that passes only with a dial doesn't count as passing in play (`PRN-12`).
  - **Check:** sandboxes and play are built from one simulation core, and every run records any dial or scripted event it used.

- `RES-19` **Promises are tested** *(Decided)*: Every claim in Minds and in Culture and society that something emerges either gets an experiment before its milestone closes, or is marked "possible, not promised".
  - **Check:** the coverage check lists every emergence claim in Minds and in Culture and society with its experiment or its "possible, not promised" mark (`PRC-12`).

- `RES-20` **Your own experiments** *(Decided)*: You can ask for an experiment in any cloud session (`SCP-15`).
  An experiment's world can be opened on the phone (`PLT-05`) and played on; it keeps its dial settings for good and always shows them, so it is never mistaken for a play world (`PRN-12`).
  - **How it works:** you describe the question in a cloud session; the AI writes the experiment file (`RES-08`) and its criteria for your approval (`RES-09`), runs it, and reports it as a page (`RES-15`).

- `RES-04` **Reality checklist first** *(Decided)*: The physics must pass every reality check (`RCK`) before any discovery that depends on it is trusted.
  - **Check:** the runner refuses to report a discovery whose reality checks are not all passing on that build.

- `RES-05` **Reproducibility** *(Decided)*: Results are reproducible statistically: re-running an experiment on fresh seeds gives results within its stated ranges, and the cloud build gives the same statistics as the phone build.
  Exact repeats of a history are not required (`PRN-15`).
  Checked at every milestone.
  - **How it works:** the same experiment runs on fresh seeds in the cloud and on the phone, and each measure's ranges are compared; a difference beyond the stated tolerance fails the check (`PRC-10`).

### 14.2 The experiments

- `RES-02` **Experiment 1: sharp stone** *(Decided)*: Do bands that only bash rocks discover how to chip sharp flakes, and does the skill spread?
  Its sandbox includes uses for a sharp edge: carcasses to butcher, and hides and wood to work.
  - **How it works:** a sandbox valley holds a band with the starting kit (`BIO-02`), glassy, fine-grained and coarse stones with decoys among them (`RES-21`), and carcasses, hides and wood, run for up to 500 simulated years per run; the recognisers mark the first struck flake used to cut (`PRE-39`), and the measures of `RES-03` are counted from the records.

- `RES-03` **Experiment 1 pass criteria** *(Decided)*: starting values, fixed before it runs (`RES-09`), met in its sandbox runs and confirmed in full worlds (`RES-21`):
  - **Discovery:** happens in at least half of the runs, within 500 simulated years.
  - **Variety:** discovery times differ widely between worlds, and at least two different routes to the discovery appear, for example an accident someone notices versus deliberate tinkering.
  - **Spread:** once discovered, at least three quarters of the adults in the discovering band can do it within 50 simulated years.
  - **Loss:** the skill is lost noticeably more often in small, isolated groups than in large, connected ones.
  - **General rules only:** the check in `PRN-07` passes.
  - **Comparison runs:** without imitation, the skill does not spread; without curiosity, discovery is much rarer (`RES-10`).
  - **Exact numbers:** words such as "widely", "noticeably" and "much rarer", and what counts as a discovery and as being able to do it, are given exact values before the run (`RES-09`).
  - **How it works:** each measure is counted from the records: discovery as the first use of a struck flake to cut, marked by the recognisers; spread as the share of the band's adults whose skill records make such flakes; loss as a skill with no living holder; and each is set against the comparison runs.

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

  The remaining signature moments (a painting that remembers, and the dig) are features, checked at milestone reviews rather than run as experiments.

### 14.3 Reports

- `RES-06` **Milestone reports** *(Decided)*: Every milestone ends with a report for you, covering:
  - what was tested and the results, with charts;
  - the comparison runs;
  - what emerged, and the surprises;
  - the measurements (`PLT-04`);
  - what was added (`PRN-14`) and how the principles were checked;
  - the risks (see Risks);
  - links to the experiments' worlds and chronicles in the game.
  - **How it works:** the report is built from the experiment results, the measurements and the coverage check, checked by the independent reviewer (`RES-11`), and published as a page (`RES-15`).

- `RES-15` **A page on the phone** *(Decided)*: Each report is a readable page with charts and plain conclusions, whose links open the experiments' worlds and chronicles in the game.
  A copy is kept in the repository.
  - **How it works:** each report is a page with its charts, whose links open an experiment's world or chronicle entry on the phone; its source is committed beside the experiment.

## 15. Project and process

How the project is run: you direct, and AI agents build.
This section defines the roles, the documents, how this file changes, and how work flows from an idea to your phone.

### 15.1 Roles

- `PRC-01` **Passion project, built by AI** *(Decided)*: You direct; AI agents write, test and review the code.
  There are no running costs beyond the AI sessions themselves, since the writer AI runs on the phone and there is no store.

- `PRC-02` **Your role** *(Decided)*: You read the milestone reports, try the builds, set direction, and approve changes to this file.
  The AI handles code review and testing.
  - **Check:** every change to this file names your OK in its commit (`PRC-07`).

- `PRC-03` **Technology** *(Decided)*: Chosen by the AI and proposed in the architecture for your approval.
  - **Check:** the architecture's technology proposal records your approval before building starts.

### 15.2 Documents

- `PRC-04` **Three documents** *(Decided)*: The finished project has three documents.
  This file is the source of truth for what to build; the architecture says how it is built; the implementation plan says in what order, mapping every item to a milestone and its tasks.
  Code and tests link back here by ID.
  - **Check:** the repository holds these documents, and each milestone review checks that this file holds no implementation details.

- `PRC-06` **A guide for AI agents** *(Decided)*: A short file in the repository (`CLAUDE.md`) that every AI agent reads first.
  It tells them to read this file, follow the principles, link all work to IDs, and never mark anything Decided without you.
  - **How it works:** the guide sits at the top of the repository, where every agent's session reads it first, and changes to it need your OK.

- `PRC-07` **Changes to this file** *(Decided)*: AI agents can suggest additions or changes, marked *Proposed*.
  Nothing becomes *Decided*, and no decided item changes, without your OK.
  How changes are proposed and recorded is set out in How this file works.
  - **Check:** the commit check confirms that every commit changing this file names the changed IDs and why, and that no item became Decided without your OK.

- `PRC-05` **Reviewed with you** *(Decided)*: Changes to this file are worked through with you, section by section or in rounds of questions, and *Proposed* items are confirmed, changed or dropped in those reviews (`PRC-07`).
  - **Check:** every change to this file names, in its commit, the review or instruction from you that it came from.

- `PRC-08` **Next: tests, then the architecture and the plan** *(Decided)*: Before the architecture and the implementation plan are written, small throwaway tests settle the basic technical choices, such as the language, storing data, the map, drawing, sound, speech and the writer AI.
  Each block is tested on its own, with no working world; anything that needs a world is designed in the architecture and tested in sandboxes (`RES-21`).
  The architecture follows, starting with the technology proposal (`PRC-03`), then the implementation plan, starting with the first milestone (`MIL-01`).

### 15.3 How work flows

- `PRC-09` **Branches, checks and review** *(Decided)*: AI agents work on separate branches.
  Work joins the main version only after every automatic check passes and an independent AI review approves it.
  You review at milestones.
  - **Check:** the main version accepts work only from branches whose checks passed and whose independent review approved them (`PRC-10`).

- `PRC-10` **The checks** *(Decided)*:
  - **before any work joins the main version:** the tests, the reality checklist (`RES-04`), the general-rules check (`PRN-07`), the short signature-moment tests (`RES-17`), and the file check: every ID defined once, every reference resolving, every status valid, and no live item pointing to a dropped one;
  - **before a milestone closes:** the visual review (`PRE-31`), the measurements (`PLT-04`), the phone and cloud statistical match (`RES-05`), the signature-moment tests that are due (`RES-17`), the coverage check (`PRC-12`), the independent review of experiments (`RES-11`) and the report (`RES-06`).
  - **How it works:** the checks run automatically on every request to join the main version and at each milestone gate, and any failure blocks it.

- `PRC-11` **Builds between milestones** *(Decided)*: A new version whenever something you can see or try has changed, with a one-line note, installed by download (`PLT-06`).
  The full report still comes at each milestone.
  - **How it works:** each build that changes something you can see or try gets a one-line note and its download link (`PLT-06`).

- `PRC-12` **Nothing gets lost** *(Decided)*: An automatic coverage check, run at every milestone gate (`PRC-10`), confirms that every feature and rule that isn't *Dropped* or *Proposed* is mapped to a milestone in the implementation plan, that the current milestone's items have tasks, that every task names the IDs it delivers, and that every ID named in code and tests exists and isn't dropped.
  - **How it works:** a script reads this file's IDs and statuses, the plan's map of items to milestones and tasks, and the IDs named in code and tests, and fails on any feature or rule left unmapped, any current item without tasks, any task without IDs, and any ID in code or tests that doesn't exist or is dropped.

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
  - **Risk:** Agreed words and word order, rich beliefs or rituals may not emerge from general mechanisms (`CUL-17`, `CUL-05`).
  - **Signs:** small language tests and later experiments failing.
  - **Response:** test early with small models; if needed, restate the promise with you, for example as "agreed words alone".

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
  - **Response:** history before the update is kept and the change is marked (`PLT-09`); the reality checklist runs before every release.

- `RSK-15` **The memory limit** *(Decided)*
  - **Rating:** likelihood high, impact medium.
  - **Risk:** About 10 GiB must hold the simulation, the picture and the writer AI (`PLT-01`).
  - **Signs:** the system slowing or closing the app; fewer detailed people than planned.
  - **Response:** measure memory per person early, and keep history in storage rather than in memory (`PRN-15`).

- `RSK-20` **Saved worlds grow too large** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Long event histories may fill the phone (`PRN-15`).
  - **Signs:** worlds growing by gigabytes every thousand years.
  - **Response:** measure early; thin old events with age by a fixed rule, and ask before deleting anything (`PLT-10`).

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
  - **Response:** sandboxes instead of whole worlds, each sized to a computing budget stated up front (`RES-21`); about 20 runs per setup (`RES-13`); stop each test once its result is clear (`RES-16`); leaner minds; and raise more computing with you first (`SCP-15`).

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
- **Scientist's view / story view:** the two ways to look into a mind: raw beliefs and evidence, or their own words (`PRE-14`).
- **Seed:** the number a world is generated from.
  It decides the world, not its history (`PRN-15`).
- **Signature moment:** a story the simulation must be able to produce without it being scripted (`MOM`).
- **Skill:** a learned way of doing something, which improves with practice (`MND-06`).
- **Story director:** sets the speed of time according to what is happening.
  It never causes events (`TIM-02`, `TIM-03`).
- **Structure:** how matter is put together: crystal or glass, fibrous, porous or dense, grain, wetness (`MAT-02`).
- **World:** one generated planet (see World).
- **Writer AI:** the AI language model, running on the phone, that turns simulation data into readable text.
  It never decides anything (`PRE-17`, `PRE-37`).
