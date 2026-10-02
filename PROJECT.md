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
  Things and blueprints](#7-things-and-blueprints)
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
  Testing](#14-testing)
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
- **Rules:** `PRN`, `MOM` (checked by test scenes), `SCP`, `RCK`, `RES` and `PRC`, plus `GOD-05`, `GOD-06`, `GOD-07`, `TIM-03`, `TIM-16`, `TIM-17`, `WLD-13`, `WLD-30`, `MAT-09`, `MAT-13`, `MAT-14`, `MAT-15`, `MAT-17`, `BIO-14`, `BIO-17`, `MND-01`, `MND-02`, `MND-14`, `CUL-07`, `PRE-17` and `PRE-31`.
- **Features:** every other area, plus `RES-02`, `RES-03`, `RES-06`, `RES-12`, `RES-15`, `RES-22`, `PRC-06` and `PRC-12`.

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
| `MAT` | Things and blueprints |
| `RCK` | Reality checklist |
| `BIO` | People: bodies and lives |
| `MND` | Minds |
| `CUL` | Culture and society |
| `PRE` | Presentation |
| `SND` | Sound |
| `PLT` | Platform and performance |
| `RES` | Testing |
| `PRC` | Project and process |
| `RSK` | Risks |

---

## 1. Vision

This section says what Kindling is, what it feels like, and what success means.
Every other section serves it.

### 1.1 The game in brief

- `VIS-01` **In one sentence** *(Decided)*: A realistic life and world game for one phone, in which a few bands of early humans living in caves find their own way to sharp stone, fire, huts, pottery, herds, fields and first copper, while you watch over them as a hidden god.

- `VIS-06` **In one paragraph** *(Decided)*: Kindling is a living world in your pocket: land, weather, plants, animals and people, all running at once.
  It begins with three or four family bands, 45–120 people in all, sheltering in caves.
  They are modern humans with a language of their own but almost no culture: no shaped tools, no clothes, and no way to make fire, only to keep one they have found.
  Each is a full person, with a name, needs, moods, memories, friends and rivals.
  Nothing tells them what to do.
  Hidden in the world are about 150 blueprints: what happens when someone strikes, heats, soaks or shapes things with the right characteristics.
  People find them by accident, by experimenting, in dreams and by copying, and each first success becomes a named discovery in the book of ages.
  A game year lasts 60 days, so a typical world goes from sharp flakes to first copper in a few hundred game years.
  You watch it all as a hidden god: you can send weather, dreams and luck, but you can never command anyone, and nobody ever learns you exist.

- `VIS-02` **The fantasy** *(Decided)*: You are a hidden god who acts only through nature.
  - **What:** You are the weather, the luck and the dreams.
    You can bring a storm, send lightning, push a dry season, bless a hunt, or give someone a dream made of their own memories.
    You can't speak, appear or work miracles, and the people of the world never learn you exist.
  - **Why:** A god who can't command anyone leaves every achievement theirs.
    Whatever gods they come to believe in are their own explanations of the world, and sometimes of you.
  - **Example:** Lightning you send sets a pine burning on the ridge, and a band carries the fire home.
    Generations later, their myths tell of the storm spirit who first gave them fire.

### 1.2 What it feels like

- `VIS-07` **Wonder** *(Decided)*
  - **What:** Awe at a world that runs itself and keeps surprising you, its maker included.
  - **Why:** Nothing is scripted.
    The rules are known; what they produce is not.
  - **Example:** Night after night, you zoom out from one campfire to the whole globe and watch herds, peoples and beliefs move across the land like weather.

- `VIS-08` **Curiosity** *(Decided)*
  - **What:** The urge to understand why something happened, and to try "what if".
  - **Why:** Every event has real causes, and the game lets you find them: the details view of each mind (`PRE-14`), graves and old camps (`PRE-09`), and the book of ages, whose every entry leads back to what happened (`PRE-05`).
  - **Example:** A band is about to leave its cave.
    You look into their minds and find a run of failed hunts and a belief that the cave turned against them after a death.
    You send a good hunting season, and see whether they stay.

- `VIS-17` **Life** *(Decided)*
  - **What:** The pleasure of a world that is always busy, with everyone doing something for reasons of their own.
  - **Why:** A world that runs itself is only worth watching if it feels alive at every zoom.
  - **Example:** At dusk in camp, a man knaps flint by the fire, a woman scrapes a hide, children chase each other round the hearth, and the murmur of talk rises and falls (`SND-01`).
    Pull back to the valley, and a season passes in a minute: herds drift north, a band moves camp, hunters come home.

- `VIS-09` **Other feelings** *(Decided)*: Attachment to particular people, and the harshness of nature, will arise from the simulation and are welcome, but the design isn't built around them.
  When design choices conflict, wonder, curiosity and life decide.

### 1.3 How you play

- `VIS-10` **Two rhythms of play** *(Decided)*
  - **Short check-ins (5–15 minutes):** open the app, catch up on the live moments waiting, follow someone for a while, nudge, close.
  - **Long sessions (an hour or more):** watch a season or a century go by, read the book of ages, visit graves and old camps, and try a "what if" with your powers.
  - **Why it matters:** both rhythms must feel natural.
    A check-in can't need any setup, and a long session needs tools for depth.
  - The world pauses when the app is closed (`TIM-05`), so every session starts exactly where the last one ended.

- `VIS-11` **A session, as a story** *(Decided)*: An illustration, not a script.

  > You open the app.
  > The world is where you left it: Year 19, autumn, day 8, a week before winter, in the valley of two rivers.
  > A live moment is waiting: *the cliff band has lost its fire*.
  > You tap it, the camera swoops down, and time slows to real speed.
  > Under the cliff the camp should be loud with children and the crack of stone on stone.
  > Tonight it is cold, the murmur of voices is low and worried, and wolves pace at the edge of the scree.
  >
  > You could bring a storm over the ridge and hope lightning finds a dry pine.
  > Instead you open the memories of Ama, the band's most curious woman.
  > Last summer, drilling a hole through a piece of dry wood, she saw smoke curl from the tip of her stick.
  > While she sleeps, you give her a dream that sets that smoking stick beside the warmth of a fire.
  > Nobody will ever know the dream was yours.
  > In the morning she is drilling again, longer and harder, with the driest wood she can find.
  > You pull back to the camp, and the days pass in minutes.
  > On day 12 an ember glows in the dust, and she breathes it into flame.
  > The book of ages records a named discovery: *hesoru*, "fire from wood", first made by Ama in Year 19.
  > By spring, four others can do it.
  >
  > You leave the world running overnight on the charger.
  > In the morning, the summary is waiting: 282 years have passed.
  > The river peoples live all year in villages of reed huts, keep goats and dogs, and sow wild grain on their old rubbish heaps.
  > In the book of ages, Ama's story has become a myth: *Ama took the fire that sleeps inside the wood*.
  > You close the app, and the world waits for you.

### 1.4 Signature moments

- `VIS-12` **Signature moments** *(Decided)*: Stories the game must be able to produce.
  None of them is scripted.
  Each is an example of what the rules should make possible, and each has a sandbox scene that must keep producing it (`RES-17`).
  The IDs in brackets are the parts of the game each moment depends on.

  - `MOM-01` **Fire from wood** *(Decided)*: In a hard winter, a band whose fire has died learns to make fire by drilling, a trick one of them stumbled on by accident.
    (`MND-11`, `RCK-02`, `GOD-03`)
    - **How it works:**
      1. The band keeps a fire found after lightning, and carries its embers when it moves (`BIO-02`).
      2. Someone drilling a hole in dry wood happens to match the blueprint for fire by drilling (`RCK-02`), and fails in its usual way, with smoke but no ember; the surprise leaves a weak belief (`MND-05`).
      3. In a hard winter the fire dies, and cold and fear push the most curious to experiment: they drill again, longer, with drier wood (`MND-11`).
      4. Each try's chance rises with experience and drier wood; at last an ember glows, and the first success becomes a named discovery (`MAT-21`).
      5. Others watch, copy and are taught (`MND-13`), and the skill spreads (`CUL-01`).
      6. A dream you send can set the smoking stick beside the warmth of a fire, raising the odds without guaranteeing anything (`GOD-03`).
  - `MOM-02` **The lost craft** *(Decided)*: A fever kills a band's last good knapper.
    For generations its blades are cruder, until the craft is found again, learned from neighbours, or copied from an old blade.
    (`CUL-02`, `MND-06`)
    - **How it works:**
      1. Fine blades are a blueprint that only people with long experience in stone make well (`MND-06`); the young learn it by watching and being taught (`MND-13`).
      2. A fever spreads by breath and touch (`BIO-05`) and kills the last people who knew it (`BIO-14`).
      3. Knowledge dies with its last holder (`CUL-02`): the survivors still make plain flakes, so their tools are cruder and wear out sooner (`MAT-20`).
      4. The craft comes back only by experimenting, by learning from neighbours met through marriage or trade (`CUL-16`), or by copying an old blade (`MND-11`).
      5. The book of ages marks the loss and the return (`PRE-39`).
  - `MOM-03` **Your lightning becomes a god** *(Decided)*: A lightning strike you sent kills a hunter on a hilltop.
    The band avoids the hill, then leaves offerings there, then tells stories about the one who lives in the storm.
    (`GOD-02`, `GOD-06`, `CUL-05`)
    - **How it works:**
      1. You bring a storm and send lightning to the hilltop (`GOD-02`), and it kills the hunter there (`BIO-14`).
      2. The death has no cause they know, so they explain it with an unseen someone in the storm (`MND-21`, `CUL-05`), and fear ties itself to the hill (`MND-19`).
      3. They avoid the hill (`CUL-20`); when visits after leaving something there pass safely, the gift gets the credit (`MND-05`), and the offerings become a rite (`CUL-06`).
      4. Retold, the story becomes a myth of the one in the storm (`CUL-11`), and in time part of their religion (`CUL-26`).
      5. Nothing marks the strike as yours (`GOD-06`).
  - `MOM-04` **The song that does nothing** *(Decided)*: A band sings before a hunt that goes well.
    The song becomes a hunting rite and is kept for centuries, though it changes nothing.
    (`MND-05`, `CUL-06`)
    - **How it works:**
      1. A band happens to sing before a hunt, and the hunt goes well.
      2. People link a strong outcome to something unusual that came before it, so the song gets the credit (`MND-05`), and the hunt is remembered vividly (`MND-18`).
      3. Singing before hunts costs little, and hunts succeed often enough through skill and luck that the belief survives; others copy it (`CUL-01`).
      4. Shared habit turns it into a rite (`CUL-06`), taught and kept long after anyone remembers why.
  - `MOM-05` **Two tongues** *(Dropped)*
    - **Dropped because:** each world now has one language that doesn't change over time (`CUL-17`), so peoples never drift apart in speech.
  - `MOM-06` **The camp wolf** *(Decided)*: The boldest wolves scavenge at the edge of camp.
    A child raises a pup, and generations later the band keeps dogs.
    (`WLD-33`, `RCK-24`)
    - **How it works:**
      1. Near people, wolves are single animals with simple minds (`WLD-32`, `MND-16`), and the bones and scraps by the camp draw the boldest close.
      2. Wolves that are fed and not harmed lose their fear of people (`MND-16`), and you can send one a dream of the warm scraps by the fire (`GOD-12`).
      3. A child who feeds and plays with a pup raises it in camp, and young animals raised by people grow tame (`RCK-24`).
      4. Tame wolves breed near camp and their pups are born tame; after several generations kept by people, the line becomes dogs (`WLD-33`), and the book of ages records the first ones (`PRE-05`).
  - `MOM-07` **A painting that remembers** *(Decided)*: A painting of a great hunt outlasts everyone who saw it.
    You tap it and read what really happened that day.
    (`CUL-09`, `PRE-15`)
    - **How it works:**
      1. A great hunt becomes a vivid shared memory, retold around the fire (`MND-18`).
      2. Someone grinds red ochre, mixes it with fat and paints the hunt on a sheltered wall, in their people's style, with the animals and hunters that were really there (`CUL-09`).
      3. Paint in shelter lasts for centuries (`RCK-16`), so the painting outlasts everyone who saw the hunt.
      4. The hunt was kept in the saved history (`PRN-15`), so tapping the painting shows what it depicts and what really happened (`PRE-15`).
  - `MOM-08` **Seeds on the rubbish heap** *(Decided)*: Seeds thrown on the rubbish heap sprout near camp.
    Years later, someone sows them on purpose.
    (`RCK-23`, `MND-11`)
    - **How it works:**
      1. People eat wild grain and fruit and throw the waste on a heap by the camp (`MAT-08`).
      2. The heap is rich and damp, so in the growing season some seeds sprout there (`RCK-23`, `WLD-31`).
      3. People notice food plants where seeds were thrown, and the surprise leaves a belief linking thrown seeds to plants (`MND-05`).
      4. When food runs short, someone experiments by putting seeds in the ground on purpose (`MND-11`); planted and tended seeds grow better (`RCK-23`), the harvest confirms the belief, and sowing spreads by copying and teaching (`CUL-01`).
  - `MOM-09` **The dig** *(Decided)*: Digging a pit on an old campsite, someone turns up a tool nobody living knows how to make, and copies it.
    (`MND-11`, `MAT-08`)
    - **How it works:**
      1. A band that made eyed bone needles dies out or moves away; its camp, hearths and tools stay where they were left and are slowly buried (`MAT-08`).
      2. Generations later, another band camps there and digs a storage pit, and a woman finds a needle.
      3. Seeing a finished thing is enough to try making it: she copies it, and her experience decides how soon she succeeds (`MND-11`, `MND-06`).
      4. The book of ages marks it as a rediscovery (`PRE-39`), and the craft spreads again (`CUL-01`).
      5. You can see the same layers in the cut-away, and each find tells who left it and when (`PRE-09`).
  - `MOM-10` **Two endings** *(Dropped)*
    - **Dropped because:** rewinding and branching were cut in the realism pass: a world keeps only its present state and its chronicle (`PRN-15`).
  - `MOM-11` **Rivals, then in-laws** *(Decided)*: Two bands fight over a valley, then marry into each other.
    Each side's descendants tell the story differently.
    (`CUL-27`, `CUL-11`)
    - **How it works:**
      1. Two bands rely on one valley's game and nuts (`MND-28`); when they meet there, fear, anger and hunger make a raid a choice each side weighs (`MND-09`, `CUL-08`).
      2. Losses on both sides, and too few partners at home, make marrying across the bands the better choice for some (`CUL-27`).
      3. Marriages make kin across the bands (`MND-24`), and few choose to raid their own kin, so fights grow rarer (`MND-09`).
      4. Each side keeps its own memories of the fight, retold through its own beliefs, so their stories differ (`MND-18`, `CUL-11`).
  - `MOM-12` **Metal from green stone** *(Decided)*: A pottery kiln, fired hotter than ever, leaves a bead of shiny metal where green stones lined the fire, and someone notices.
    (`RCK-08`, `MND-11`)
    - **How it works:**
      1. Potters find ways to make their kilns hotter, until one burns as hot as a furnace (`MAT-18`, `RCK-04`).
      2. Green copper ore, picked up for its colour, lines the hottest part of the fire (`WLD-14`); at furnace heat it matches the blueprint for smelting (`RCK-08`), and copper runs out as a bead.
      3. The shine and the weight are a surprise (`MND-10`); whoever notices links them to the green stones and the hot fire (`MND-05`), and may try again on purpose (`MND-11`).
      4. The first success is a named discovery (`MAT-21`), and a new age begins in the book of ages (`PRE-05`).

### 1.5 The arc of a world

- `VIS-03` **The arc of a world** *(Decided)*
  - **What:** Every world starts in caves (`SCP-01`), and over a few hundred game years its people can find their way to fire, huts, pottery, dogs, herds, villages, fields and first copper.
    After that, history goes on within the launch catalogue, for as long as you watch.
  - **The arc, in game years from the start** (targets for typical worlds, tuned by testing, `TIM-19`): sharp flakes 1–5; fire-making 5–30; clothing and huts 10–40; pottery 60–150; tame dogs 80–150; herding 120–250; villages 100–300; farming 200–350; copper 300–500.
  - **No scripted eras:** each step happens only when someone discovers it (`PRN-17`), so the order differs between worlds, and stalls, lost crafts and peoples dying out are all valid histories.
    Only the blueprints and the world set what is possible: no one smelts copper without a furnace (`RCK-08`).
  - **Room to grow:** later layers, such as bronze or writing, can extend the arc by adding items and blueprints (`PRN-14`).
  - **The phone's limit:** up to about 2,000 people at a watchable speed; beyond that, time slows rather than detail being cut (`MND-15`).

### 1.6 What makes it different

- `VIS-13` **Seven differences** *(Decided)*: A summary of decisions made in other sections.
  - **No tech tree.** About 150 hidden, generic blueprints, found only in play (`SCP-04`).
  - **Minds that learn and believe.** People form their own beliefs, and science and superstition grow from the same habit of mind (see Minds).
  - **Things look like what they are made of.** Every tool, hut and pot shows what went into it (`PRE-42`).
  - **A hidden god.** You act only as nature could, and nobody ever learns you exist (see The player as god).
  - **History at a watchable pace.** A 60-day year, real speed up close and centuries overnight (`PRN-17`).
  - **Every story can be traced.** The book of ages, graves and old camps, and the details view of each mind all lead back to what really happened (see Presentation).
  - **In your pocket.** Built for one phone, with pixel-rendered 3D and one zoom from the globe to one person's hands (see Presentation and Platform and performance).

### 1.7 Inspirations

- `VIS-04` **Inspirations** *(Decided)*: What we take from each, and where we differ.
  Where this file leaves a design question open, start from what these games do, adapted to the principles.
  - **[world-sim](https://world.world-sim.uk):** a living world of named souls, graves and a book of ages, whose villagers discover pottery and bronze for themselves.
    *We take* the named souls, the graves and the book of ages.
    *We differ:* we start earlier, in caves, have no tech tree or list of eras, and run on a phone.
  - **Dwarf Fortress:** personalities, memories, legends, and artworks that depict real events.
    *We take* history as the main product, and art that remembers.
    *We avoid* an interface that hides its stories.
  - **RimWorld:** needs, moods built from thoughts that last a while, a storyteller, and 60-day years.
    *We take* its needs, its moods, its 60-day year and its care for pacing.
    *We differ:* our story director only controls the speed of time; it never creates events (`TIM-03`).
  - **WorldBox:** a pixel-art god game for phones, where you watch peoples rise.
    *We take* the joy of a living world in your hand.
    *We differ:* a far deeper life for each person, and powers limited to what nature could do.
  - **The Sims:** needs that pull each person toward what serves them best, and an animation for every action.
    *We take* both.
    *We differ:* nobody is controlled.
  - **Black & White:** a god whose acts shape what villagers believe.
    *We differ:* there is no worship and no visible god.
  - **Ancestors: The Humankind Odyssey:** early humans learning by trying things.
    *We take* the thrill of discovery by trial.
    *We differ:* nobody is controlled, and discoveries come from hidden blueprints found in play, not from a skill tree.

### 1.8 Success

Who it's for: you alone (`SCP-02`).
Success is judged by playing it; the tests and reviews of `VIS-05` are how we get there.

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
    - you'd choose to read a world's book of ages for pleasure;
    - worlds from different seeds tell clearly different stories;
    - every entry in the book of ages can be traced back to the events behind it.

- `VIS-05` **Quality bar** *(Decided)*: A believable game, tested at every step, and craft polished as far as the tools allow.
  - **Tested:** what the game is meant to do is checked by automated tests that can fail, at every alpha: its rules, blueprint chains, behaviours, pace and speed (`RES-01`).
  - **Craft:** art, animation, sound, interface and performance polished as far as AI agents can take them, judged by you at every visual review (`PRE-31`).
  - Tests are the method, not the goal: they exist so the world stays believable and its histories stay true.

### 1.9 Name

- `VIS-16` **Name** *(Decided)*: **Kindling**, what a fire grows from: small things that catch and spread, like knowledge.
  "Project Nature" was the working title.

## 2. Principles

The rules every part of the game follows.

- `PRN-16` **Principles come first** *(Decided)*: These rules apply to every part of the game, and outrank everything else in this file: if any decision conflicts with a principle, the principle wins.
  A principle changes only if you change it here.
  Every milestone review goes through the principles, using the **Check** line under each one.
  - **Check:** every milestone report lists each principle with the result of its Check line (`RES-06`).

### 2.1 The world

- `PRN-01` **The world is the only teacher** *(Decided)*
  - **What:** Everything the people of the world know, they learned inside it: from their senses, their own tries, other people or their dreams.
    Nothing is handed to them beyond the little they know at the start (`BIO-02`).
    Nobody knows a blueprint until they discover it or learn it from someone (`MND-11`).
    Their choices never use a blueprint they don't know, or a fact they haven't seen or been told (`MND-09`).
  - **Why:** This is the heart of the game.
    A discovery only means something if it was really made.
  - **Example:** Nobody tells a band that flint makes good blades.
    Someone cracking nuts with a flint sees a sharp flake come off, and over time the band learns which stones break that way.
  - **Check:** every discovery can be traced to how it was made (by accident, by experimenting, from a dream or by copying), and tests find no choice that used an unknown blueprint or an unseen fact.

- `PRN-02` **Believable over exact** *(Decided)*
  - **What:** The world looks, sounds and behaves like the real one, without simulating how the real one works underneath.
    Where an exact model would be costly, a simple rule that gives a believable result wins (`SCP-21`).
  - **Why:** A game is judged by what you see, hear and read, so the effort goes into what shows.
  - **Example:** Flint gives sharp flakes and granite doesn't because flint's characteristics say it flakes well (`RCK-01`), not because the game simulates cracks running through stone.
  - **Check:** each milestone review flags any system whose detail changes nothing you can see or read.

- `PRN-07` **Generic blueprints** *(Decided)*
  - **What:** Every way of making or changing things is a blueprint, and every blueprint is generic: it asks for actions, characteristics and conditions, never for particular items (`MAT-04`).
    So one blueprint works for everything with the right characteristics, and people can find routes nobody listed.
    The rules that decide what people and animals do are the same for every blueprint: they never single one out by name, and see only what people perceive and know.
    Names such as "flake" or "pottery" appear only in the catalogue and in text written for you.
  - **Why:** A rule written for one outcome is a recipe in disguise.
    Generic blueprints let the world surprise its makers.
  - **Example:** The blueprint for a scraper asks for something hard that flakes well and has a good edge, never for flint by name.
    Obsidian and chert work too, and so would a stone added years later.
  - **Check:**
    - tests confirm that every blueprint's inputs are ranges of characteristics, never named items;
    - a search of the rules that decide what people and animals do finds no blueprint, discovery or item names;
    - swapping the names of two materials changes no behaviour;
    - a made-up material with the right characteristics works in every blueprint it fits.

- `PRN-05` **Plausible numbers** *(Decided)*
  - **What:** Every value in the game, such as a material's characteristics, a plant's yield, an animal's speed or an illness's danger, is a plausible estimate, set by hand from what the real thing is like and tuned so the game feels right.
    No value needs a sourced measurement.
    Every claim about what the game produces is checked by a test that can fail (`RES-01`).
  - **Why:** Sourcing every value would cost far more than it adds to a game.
    Plausible values, tuned and tested, keep the world believable and the work affordable.
  - **Example:** A flint flake's edge sits near the top of the scale and a granite chip's near the bottom, because that is how they cut in the hand, not because of a measured number.
  - **Check:** the catalogue tests pass (`MAT-17`), and every claim in a milestone report is backed by a test.

- `PRN-12` **Speed up time, never bend the rules** *(Decided)*
  - **What:** In play, pace comes only from controlling time: zoom, the story director and manual speed (see Time and history).
    Tuning happens when the game is made, the same for every world (`PRN-17`).
    The rules never change during play to make things faster or more dramatic, and nothing happens because it would make a better story.
    Sandbox scenes for tests may set up a starting situation, but they run the same rules as play (`RES-18`).
  - **Why:** If the rules bent for drama, nothing the world produced could be trusted.
  - **Example:** When a band is close to making fire, the story director slows time so you can watch.
    It never makes the fire come sooner.
  - **Check:** play has no rule-bending settings, and the same saved world runs the same with the story director on or off (`TIM-03`).

- `PRN-17` **History at a watchable pace** *(Decided)*
  - **What:** Discoveries come at a pace you can watch, set by the pace targets (`TIM-19`).
    The pace comes from tuning chances and amounts, the same for every world: how often people experiment, how likely a blueprint is to succeed, how fast experience grows, how much food the land gives.
    No discovery is ever scripted or forced by a date.
  - **Why:** A world where nothing changes for ten thousand years may be true to history, but it is no fun to watch.
  - **Example:** If tests show fire-making arriving around year 80 instead of within 5–30, the chance of an ember from drilling is raised for every world.
    No world is ever told to find fire by year 20.
  - **Check:** overnight pace tests on whole worlds show each step landing in its target range in typical worlds, not always in the same order, with nothing scripted (`RES-07`).

### 2.2 The player

- `PRN-03` **You are nature** *(Decided)*
  - **What:** The player acts only through natural means (`GOD-05`) and is never known to exist (`GOD-06`).
  - **Why:** A god who could command people or appear to them would make every achievement partly yours.
    Every belief about gods would be true, instead of theirs.
  - **Example:** You can't hand a band fire.
    You can make lightning strike a dry tree near their camp.
  - **Check:** every power produces only events the world could produce on its own.

### 2.3 What you see

- `PRN-04` **If the game knows it, you can see it** *(Decided)*
  - **What:** Anything the game keeps track of can be shown to you: a person's needs, mood, memories, beliefs and the blueprints they know; family trees; who taught whom; graves and old camps.
  - **Why:** Curiosity (`VIS-08`) needs ways to find out why.
    A rich world you can't look into is wasted.
  - **Example:** The game tracks who taught whom to drill fire, so you can follow the chain back to the first person who did it.
  - **Check:** everything the game keeps track of has at least one view that shows it, if only a card (`PRE-35`) or the details view of a mind (`PRE-14`).

- `PRN-10` **Nothing is faked** *(Decided)*
  - **What:** Every picture, sound and word shows what is really there and what really happened in the world.
    Places nobody has visited are drawn from the world's seed, just as they are made when someone first goes there (`WLD-13`).
    Nothing is added for show.
  - **Why:** Histories are only worth reading (`VIS-15`) if they are true to the world, and curiosity only works if every clue is real.
  - **Example:** The murmur at a camp comes from people really talking there (`SND-03`), and a painting shows the hunt that really happened (`MOM-07`).
  - **Check:** every live moment, entry in the book of ages, sound and on-screen event can be traced back to something that happened in the world.

- `PRN-13` **Every choice can be explained** *(Decided)*
  - **What:** Why anyone, person or animal, does something can be traced to their needs, personality, plans, beliefs and memories.
    Each choice keeps its top reasons, and the details view shows them (`PRE-14`), for what is happening now and for the choices behind the events the history keeps (`PRN-15`).
  - **Why:** Curiosity depends on asking "why?" and getting a real answer.
  - **Example:** Why did Ama walk to the river at dawn?
    She was thirsty, and she believes the river is safe at dawn because she has never seen wolves there at that hour.
  - **Check:** for any activity under way, and for the choices behind every saved event, the details view shows the reasons kept with it.

- `PRN-06` **AI language models describe, never decide** *(Decided)*
  - **What:** AI language models are used only to turn what the game records into readable text: the book of ages, life stories, myths, dreams.
    They never choose, invent or know anything for the people or animals of the world, and never add facts the game doesn't contain (`PRE-17`).
  - **Why:** A language model knows our history.
    If it did their thinking, our knowledge would leak into their world and their discoveries would no longer be theirs.
  - **Example:** The model can tell you, in the voice of their tradition, how Ama "took the fire that sleeps inside the wood".
    It cannot decide that she tries drilling.
  - **Check:** nothing a language model writes ever feeds back into the game, and its texts are checked against what they came from.

### 2.4 How it runs

- `PRN-15` **History is saved, not re-run** *(Decided)*
  - **What:** The past is kept as the book of ages and the events behind it, with what each view of the past needs: graves, old camps, family trees, art.
    The world's full state is kept only for the present (`PLT-07`), so the past can't be replayed or returned to.
    A seed decides how a world is made, not how its history unfolds.
  - **Why:** Replaying history exactly would need identical maths on every device, and every old version of the rules kept forever.
    Saving what matters avoids that cost.
  - **Example:** You tap a cave painting of a great hunt.
    The hunt was saved, so you can read who was there, what happened and how it ended.
  - **Check:** every view of the past reads what was saved, and nothing re-runs the past.

- `PRN-08` **Same seed, same history** *(Dropped)*
  - **Dropped because:** history is now saved rather than re-run (`PRN-15`), and the phone and cloud builds no longer need to match exactly (`PLT-05`).

- `PRN-11` **Time slows, the screen stays smooth** *(Decided)*
  - **What:** The screen never stutters.
    When the phone can't keep up, the world doesn't cut detail: time simply runs more slowly.
    Every person is a full individual at all times, watched or not (`MND-14`).
    The only simplifications are the planned ones, the same whether you look or not: herds far from people are counts rather than single animals, and plant cover far away is kept per world cell (`WLD-12`).
  - **Why:** A smooth screen is part of the joy on the phone (`VIS-14`), and cutting detail under load would make history depend on how busy the phone is.
  - **Example:** A fight breaks out between two bands while you watch.
    The phone works harder, so the season takes longer to pass, but everyone in the fight is fully simulated and the screen stays smooth.
  - **Check:** measurements show no stutter under heavy load (`PLT-04`), and the same saved world gives the same results at every speed and zoom (`TIM-17`).

### 2.5 How it's built

- `PRN-09` **Build in playable steps** *(Decided)*
  - **What:** The game is built as a series of playable alphas, each ending with something you can open and play on your phone (`SCP-03`).
    Each alpha builds only what it needs, on foundations that later ones extend without starting over.
  - **Why:** A game you can play at every step shows early what works and what is fun, and keeps the project moving.
  - **Example:** The first alpha is one band in one generated area, eating, drinking and sleeping through day and night (`MIL-01`).
    Copper waits for the last milestone (`MIL-07`), but things and blueprints are designed from the start so it can be added without rework.
  - **Check:** every alpha ends with a build you can install and play (`PRC-11`), and every task in the implementation plan names the items it delivers.

- `PRN-14` **Modular by design** *(Decided)*
  - **What:** Every system grows by adding self-contained pieces (items, blueprints, plants, animals, illnesses, behaviours, views, tests), and never rewrites what already works without a stated reason.
    Adding something should be easy.
  - **Why:** A game that keeps growing stays buildable only if it is modular.
  - **Example:** Adding a new animal later needs one catalogue entry, with its model, sounds and tests.
    Hunting, taming and herding already work for it, because their rules never named a species.
  - **Check:** every milestone report lists what was added and confirms that nothing earlier had to be rewritten, or explains why it had to be.

## 3. Scope and non-goals

This section sets the boundaries of the game: what it includes, where history starts, who it's for, how it gets built, and what it deliberately leaves out.

### 3.1 What the game includes

- `SCP-13` **The whole game at a glance** *(Decided)*: A summary of the sections that follow.
  - **You, a hidden god** (see The player as god): weather and natural disasters where nature allows, dreams, and fortune.
  - **Time and history** (see Time and history): a 60-day year, real speed up close, centuries overnight, and a story director that slows time for what matters.
  - **A generated world** (see World): about 2,000 km around, with climate and weather, about 60 kinds of plant and 30 of animal, and flint, clay, ochre and copper ore where the rocks put them.
  - **Things and blueprints** (see Things and blueprints): about 200 kinds of item, 21 base actions, and about 150 hidden blueprints, from sharp flakes to copper.
  - **People** (see People: bodies and lives): bodies with needs, wounds and illnesses, from birth to old age.
  - **Minds** (see Minds): needs, personality, moods, memories, beliefs, plans, friends and rivals, and choices whose reasons you can read.
  - **Culture and society** (see Culture and society): one language from the start, teaching, religion, kin and marriage, leaders, peoples, trade and raids, villages, art, music and festivals.
  - **Presentation** (see Presentation): pixel-rendered 3D, one zoom from the globe to one person, things that look like what they are made of, and the book of ages.
  - **Sound** (see Sound): a lively camp, the murmur of their language, and their songs.
  - **The phone app** (see Platform and performance): built for one phone, in portrait and landscape, smooth at all times.
  - **Testing** (see Testing): automated tests at every alpha, sandbox scenes for every blueprint chain, overnight pace tests on whole worlds, and your reviews.

### 3.2 Where history starts

- `SCP-01` **Starting point** *(Decided)*: Modern minds with almost no culture.
  - **What:** Every world begins with 3–4 family bands of 15–30 people, 45–120 in all, in one start region, sharing one language (`BIO-03`, `CUL-17`).
    They are modern humans with almost no culture: no shaped tools, no clothing, natural shelter only, and no way to make fire, though they can keep a found fire alive and carry its embers (`BIO-02`).
    This is the only starting point.
  - **Why:** Because their minds are already modern, progress depends on discovery and culture, not on waiting for brains to evolve.
    Because they start with almost nothing, the great early discoveries happen in play: sharp stone, fire-making, clothing and huts.
  - This is a deliberate starting point, not a real moment in history.
    Real early humans already had more culture than this.

- `SCP-14` **Other starting points later** *(Dropped)*
  - **Dropped because:** every world starts the same way (`SCP-01`), which keeps the game smaller and its tests comparable.

### 3.3 Who it's for

- `SCP-02` **Just you** *(Decided)*
  - **What:** Kindling is built for one person, on one phone.
  - **In practice:**
    - no public release, store listing or tutorial, only short help cards (`PRE-40`);
    - no support for other phones, tablets or computers (tests in the cloud are a building tool, not a way to play);
    - no accounts, purchases, ads or analytics;
    - free to use your phone's specific hardware (`PLT-01`).
  - **Why:** Building for one person and one device removes whole categories of work, so the effort goes into the game itself.
  - **Check:** the builds hold no account, purchase, advertising or analytics code, and target only your phone (`PLT-01`).

### 3.4 How it gets built

- `SCP-03` **Playable alphas** *(Decided)*: The game is built as a series of playable alphas, each a few hours of AI work, each ending with something you can install, open and play on your phone (`PRN-09`, `PRC-11`).
  Every alpha comes with its automated tests (`RES-01`).
  - **Check:** every alpha has a build you can install, its tests pass (`PRC-10`), and its note says what you can now see or do.

- `SCP-15` **Tests run in the AI's cloud sessions** *(Decided)*
  - **What:** Automated tests, including the overnight pace tests on whole worlds, run in the same cloud sessions where the AI builds the game, within those sessions' computing limits.
  - **Why:** There's nothing extra to set up, maintain or pay for.
  - If a test ever needs more computing than a session offers, that is raised with you before anything else is set up.
  - **Check:** every milestone report states where its tests ran and how much computing they used (`RES-06`).

- `SCP-16` **Milestones** *(Decided)*: The game is built in seven milestones, in order, each a stage of several playable alphas (`SCP-03`).
  Each milestone ends with a report you review (`RES-06`).
  This file keeps each milestone's goal and order; the implementation plan maps every item to a milestone, with its alphas and tasks.

  1. `MIL-01` **First camp** *(Decided)*: a generated area with one band; needs (hunger, thirst, warmth, rest); gathering, eating, drinking and sleeping; day and night; zoom from the area to one person; time controls; saving.
     *Now possible:* watching a band live through its days and nights at the foot of a cliff.
  2. `MIL-02` **Sharp stone** *(Decided)*: items and their characteristics, base actions, blueprints, discovery, experience and teaching; sharp flakes discovered; the first entries in the book of ages.
     *Now possible:* watching someone find that struck flint gives a sharp edge, and the skill spread or be lost.
  3. `MIL-03` **Fire and the first power** *(Decided)*: fire and heat, fire-making by drilling, and cooking; local weather with lightning; your first powers, lightning and dreams; the first sounds and the murmur.
     *Now possible:* a band that can only keep fire learns to make it, and you can send a storm or a dream and see what comes of it.
  4. `MIL-04` **A living world** *(Decided)*: the whole world generated; the map layers and zoom out to the globe; plants, animals and hunting; seasons and weather; illness.
     *Now possible:* following herds and hunters across a whole world, season by season.
  5. `MIL-05` **Minds and beliefs** *(Decided)*: full minds (personality, mood, memories, relationships, beliefs); conversations; belief templates and religion; the writer AI's texts; the details view of each mind.
     *Now possible:* rites and taboos form, and the book of ages reads like a history.
  6. `MIL-06` **Many peoples** *(Decided)*: bands splitting into named peoples with territories; marriage, trade, feuds and raids; the story director; overnight mode.
     *Now possible:* watching peoples spread, split, fight and marry, centuries at a time overnight.
  7. `MIL-07` **Herds, fields and villages** *(Decided)*: taming and herding, farming, villages, pottery, copper, art, music and festivals; the full launch catalogue; the pace targets met (`TIM-19`).
     *Now possible:* the whole arc, from caves to first copper, in a few hundred game years.

### 3.5 Non-goals

Things the game deliberately does not do, and why.

- `SCP-04` **No tech tree** *(Decided)*: Blueprints exist, but they are hidden and generic, found only in play, by accident, by experimenting, in dreams or by copying (`MND-11`), and never chosen from a menu or unlocked with points (`PRN-07`).
  - **Check:** the game has no menu, list or tree of discoveries to choose from, and nothing on screen shows a blueprint nobody in the world has found.
- `SCP-05` **No other human species** *(Decided)*: There is one human species, so the story stays about how one people learns.
  - **Check:** the game holds one human species, and no catalogue adds another.
- `SCP-06` **No AI language model making decisions** *(Decided)*: Our own knowledge would leak into their world (`PRN-06`).
  - **Check:** the check of `MND-01` passes.
- `SCP-07` **No goals, scores, wins or losses** *(Decided)*: It is a sandbox; the story is whatever happens.
  - **Check:** the app has no goal, score, win or loss, on screen or in the code.
- `SCP-08` **No worship of the player** *(Decided)*: Your power doesn't depend on their faith, and they never learn you exist (`GOD-06`).
  - **Check:** no power reads any mind's beliefs, and the check of `GOD-06` passes.
- `SCP-09` **No terraforming** *(Decided)*: You can't reshape land or add or remove species.
  You act only as nature could (`GOD-05`).
  - **Check:** every power works through a natural system (`GOD-05`); none reshapes land or adds or removes a species.
- `SCP-10` **No shared online world or multiplayer** *(Decided)*: It's yours alone (`SCP-02`).
  - **Check:** nothing in play uses a network connection (`PLT-03`), and the app has no online features.
- `SCP-11` **No real-Earth map** *(Decided)*: Every world is generated (see World).
  - **Check:** worlds come only from the generator (`WLD-10`), and the app holds no real-Earth map.
- `SCP-12` **No simulated planet formation** *(Decided)*: Worlds are generated directly in a realistic present-day state, which keeps generation to a few minutes (`WLD-08`).
  - **Check:** generation runs only the stages of `WLD-09`.
- `SCP-17` **No direct control** *(Decided)*: You never control any person or animal, not even briefly (`GOD-01`).
  - **Check:** no control in the app sets any being's actions; your only way into the world is your powers (`GOD-05`).
- `SCP-18` **No scripted story** *(Decided)*: There is no campaign, no quests and no authored events.
  Every story comes from the world's rules (`PRN-01`).
  - **Check:** a search of the game finds no authored events, quests or campaign; every event comes from the rules.
- `SCP-19` **No magic in the world** *(Decided)*: Nothing supernatural exists in the world's rules.
  Spirits and gods exist only in people's beliefs.
  The only unseen force is you, and you act through nature.
  - **Check:** every blueprint and world rule is physical or biological (`MAT-13`), and no rule of the world reads people's beliefs about spirits.
- `SCP-20` **No borrowed real cultures** *(Decided)*: Their peoples, names, languages and customs are their own.
  Nothing is copied from real cultures, and descriptions never compare them to real peoples.
  - **Check:** each milestone review checks names, words, customs and descriptions for anything copied from or compared with a real people; languages draw on their own sounds (`CUL-17`), starting looks are mixed (`BIO-22`), and the writer's instructions forbid comparisons (`PRE-17`).
- `SCP-21` **No deep science simulation** *(Decided)*: Kindling is a believable game, not a science simulation (`PRN-02`).
  It does not simulate chemistry, the balance of elements and energy, how cracks run through stone, microbes, heredity and evolution in plants and animals, insects, ice ages, tides, people changing the climate, slow changes in the land beyond rare quakes and eruptions, or language changing over time.
  - **Why:** Each would cost a great deal and show little; what does show comes from simpler rules, such as blueprints (`MAT-04`), illnesses that spread by real routes (`BIO-05`) and taming (`WLD-33`).
  - **Check:** each milestone review confirms that no part of the game simulates any of these.

## 4. The player as god

You are an invisible force of nature.
You have four kinds of power: weather, dreams, animal dreams and fortune.
This section sets what each can do, its limits and costs, and how you use it on the phone.
Two principles govern all of it: you are nature (`PRN-03`), and the rules never bend (`PRN-12`).

### 4.1 Your role

- `GOD-01` **Role** *(Decided)*
  - **What:** A distant, invisible god in a sandbox.
    You can watch everything, everywhere, and you can nudge, but you never command or control anyone (`SCP-17`).
  - **Why:** Every achievement in the world stays theirs.
  - **Example:** You can't tell Ama to drill a stick into dry wood.
    You can only send her a dream and see what she does with it.

- `GOD-06` **Never known** *(Decided)*
  - **What:** People and animals meet your acts only as nature: weather, dreams and luck.
    Nothing in the world can tell that an event was yours.
    People explain your acts as they explain any event, through spirits, ancestors or not at all (`CUL-05`), and what they believe is their own, right or wrong.
  - **Check:** a test confirms that nothing a person or animal can sense, remember or believe records whether an event was your act.
  - **Why:** Their beliefs stay their own, and religion grows from the same events as everything else (`CUL-26`).
  - **Example:** You bless a band's best hunter for a season, and his hunts go well.
    The band credits the bones they buried at the cave mouth, and burying bones before a hunt becomes a rite.

- `GOD-05` **Only natural means** *(Decided)*
  - **What:** Every act is something the world itself could do, at a place and time where it could happen.
    Your powers work only through the world's own weather, sleep and luck.
    They never make anything from nothing (`MAT-09`) and never bend a rule (`PRN-12`).
    There is no power to collect or spend; instead, every power has the same kinds of limits and costs.
  - **Limits:**
    - **needs:** lightning needs a thunderstorm overhead, rain needs moist air, a flood a river, a quake a fault, and a dream a sleeper, made only from what they know;
    - **strength:** you choose where and when, and the world sets how strong, as it would for a natural event there and then;
    - **rest:** each power rests before it can be used again, as its item says.
  - **Costs:**
    - **time:** nothing is instant: a storm gathers over hours, a flood rises over a day or two, a drought bites over days, a dream works only after waking, and fortune only when chance comes up;
    - **no favourites:** weather falls on everyone in reach, friend or rival, people and animals alike, and lightning burns or kills whatever it hits;
    - **no undo:** what has happened stays.
  - **Why:** A single miracle would make the world's history untrustworthy, and limits make every act a real choice.
  - **Check:** a test tries each power across many places, seasons and targets, and confirms that it is offered only where its conditions hold, and that what it brings matches a natural event of the same kind and size.

### 4.2 Your powers

- `GOD-02` **Weather and disasters** *(Decided)*
  - **What:** Seven powers over weather and land:
    - **Lightning:** under a thunderstorm, you aim its next strike at a spot.
      It hits the tallest thing within a few metres, such as a tree, a rock or a person in the open, and can split, burn, wound or kill (`MAT-18`, `BIO-13`).
      Strikes come at the storm's own pace, about one every few minutes (tuned).
    - **Rain:** where there is cloud or moist air, rain falls over that weather cell, about 10 km across, for up to a day, or snow where it is cold enough.
    - **Storm:** where that place's climate has storms in that season, one gathers upwind and arrives within a few hours (`WLD-16`).
      It brings strong wind and heavy rain or snow, with thunder where the air is warm and moist, and moves on with the wind after hours to a day.
    - **Drought:** rain is held away from a region up to about 50 km across for up to a season: springs and rivers fall, plants wither, and fire catches easily.
    - **Cold snap:** a region up to about 50 km across gets up to three days as cold as that place ever gets in that season: frost in autumn, deep cold in winter, but no frost where that season never has any.
    - **Flood:** where a storm could come, it stalls over a river's upper valley for a day, and over the next day or two the river rises and spills over its banks downstream, as far as its valley allows (`WLD-17`).
    - **Quake or eruption:** only on a fault or at a volcano the world has (`WLD-15`).
      You choose when; the world sets how big, as for a natural one.
  - **Rest:**
    - one storm of yours at a time, a flood's included, and one rain;
    - one drought and one cold snap at a time, and afterwards that region can't be dried again for a year or chilled again for a season;
    - a fault or volcano you set off stays quiet as long as it would after a natural quake or eruption, usually many years.
  - **Not included:** changing the climate, reshaping the land, or adding or removing species (`SCP-09`).
  - **Why:** Weather is the most natural lever there is, and the one people have always tried to explain.
  - **Example:** You bring a storm over the ridge and aim its lightning at a dead pine.
    Fire runs down the slope, and the band upwind carries burning branches home.

- `GOD-03` **Dreams** *(Decided)*
  - **What:** While someone sleeps, you can choose what they dream, from what they have seen and done.
    There are five kinds of dream:
    - **a place** they know (`MND-28`): they feel drawn to go there;
    - **an animal** they have met: they feel drawn to hunt, watch or feed it;
    - **a person** they know: they feel drawn to seek them out, to talk, help, court or make peace;
    - **a fear** of a place, an animal or a person they know: they keep away from it;
    - **an idea:** you pick a memory of something they did or handled, and the dream sets it beside what a blueprint they don't know yet could give, such as the smoking stick beside the warmth of a fire.
      The blueprint must start from that memory and lie near their experience: they know its action and have handled things like its inputs (if several fit, the nearest to what they know).
      They wake with a hunch, and may try it when they have time and the things in reach (`MND-11`); whether it works is down to the blueprint's chance and their experience (`MAT-04`).
  - **How strong:** a dream's pull lasts a few days (tuned).
    It can tip a close choice (`MND-09`), but never beats hunger, danger or a firm plan, and they may never act on it at all.
  - **Limits and costs:**
    - one dream per sleeper per night, and up to three dreams a night in all, animal dreams included;
    - only what the dreamer knows: never a place, animal, person or thing they have not met, and only memories that lead to such a blueprint can become an idea;
    - your dream replaces that night's own dream (`MND-12`), and any hint it might have brought;
    - sending the same dream again keeps it fresh but doesn't make it stronger.
  - **What follows:** the dream is remembered like any other, and may be told, so it can spread and feed belief (`CUL-05`).
  - **Why:** Dreams are where minds recombine what they know (`MND-12`), so they are the most natural way to touch an idea without supplying it.
  - **Example:** Ama once saw smoke curl from her stick as she drilled a hole in dry wood.
    The band's fire dies, and while she sleeps you pick that memory for an idea.
    She dreams of the smoking stick beside the warmth of a fire, and in the morning she is drilling again, longer and harder, with the driest wood she can find.

- `GOD-12` **Animal dreams** *(Decided)*
  - **What:** While an animal or a herd rests, you can send it one of three simple dreams:
    - **toward a place** within its range, such as the scraps at a camp's edge: over the next few days it drifts there, but a herd never leaves its range for that season (`WLD-32`);
    - **calmer:** for a few days it startles less, flees later and fights less, so it is easier to approach, hunt or tame (`WLD-33`);
    - **bolder:** for a few days it comes closer to people and camps and stands its ground, but is also quicker to fight when cornered.
  - **Limits and costs:** one dream per animal or herd per night, within the three a night (`GOD-03`); repeats keep it fresh but don't add up; and a bolder wolf is bolder with everyone, children included.
  - **Why:** Animals learn and move by their own simple minds (`MND-16`), and dreams let you lean on them gently, toward a hunt or toward taming.
  - **Example:** A young wolf dreams of the scraps at the camp's edge and comes a little closer the next night (`MOM-06`).

- `GOD-04` **Fortune** *(Decided)*
  - **What:** You can bless or curse one person for a day, a season or a year.
    Fortune changes only their luck: which of the possible outcomes comes about in what they do and what befalls them, such as a hunt, a find, a try at a blueprint, a birth, a wound or an illness.
    It never changes what is possible, and never touches their choices.
  - **How strong:** when chance goes against a blessed person, it gets a second roll.
    When chance goes a cursed person's way, it is rolled again half the time.
    So a blessing never more than doubles a chance, and a curse never more than halves one: a hunt with a 10% chance becomes about 19% blessed and 5.5% cursed, and one with a 50% chance becomes 75% or about 37%.
    Their skill still matters most, and nothing becomes certain.
  - **Limits:** one fortune per person at a time, and up to three people carrying your fortune at once.
  - **Why:** Luck is how the world feels to the people in it, and fortune lets you lean on it without taking over.
  - **Example:** You bless a band's best hunter for one winter.
    He brings meat home a little more often, but whether the band gets through still depends on how well they hunt and share.

### 4.3 Using your powers

- `GOD-10` **Using your powers on the phone** *(Decided)*
  - **Touch first:** long-press a person, animal, herd or place, and a ring shows the powers possible there (`PRE-33`, `GOD-11`).
    While the ring is open, faint marks show your acts at work nearby, such as a storm you brought or a blessed person.
  - **Weather:** pick a power and tap the spot; for a drought or cold snap, draw the region with a finger, up to about 50 km across, and pick how long.
  - **Dreams:** on a sleeper, pick the kind of dream, then what it is about, from small pictures of the places, animals, people and memories they have.
  - **Animal dreams:** on a resting animal or herd, pick calmer, bolder, or a place, then tap the place.
  - **Fortune:** on a person, pick bless or curse, then a day, a season or a year.
  - **Confirm or cancel:** time pauses while you choose (`TIM-15`); confirming hands the act to the world, which carries it out by its own rules, and cancelling leaves everything as it was.
  - **Ending early:** a drought, cold snap or fortune can be ended early from its page (`GOD-09`); a storm runs its course.

- `GOD-11` **What's possible here** *(Decided)*: The ring offers only what nature could do at that place, or to that being, right now.
  Each power that isn't possible says why in a few words, such as "no storm overhead", "she is awake", "no fault here" or "resting: ready in 2 days".
  - **Why:** You never have to guess what is natural, and you never try a miracle by accident.

### 4.4 Records of your acts

- `GOD-08` **Recorded behind the scenes** *(Decided)*: Every act is saved with the world's history: when, where, on whom, and every choice you made (`PRN-15`).
  Minds can never sense it (`GOD-06`), and no text ever uses it (`GOD-07`); it feeds only your own marked lines and the pages of what came of your acts (`GOD-09`).

- `GOD-07` **No trace in the story** *(Decided)*: The book of ages, live moments and every text written for you tell what happened, never that it was your doing.
  There, your acts read as nature.
  If you choose, your acts show beside the story as separate lines marked as yours, never woven into the text (`PRE-05`).
  - **Check:** a test runs a world with many acts and finds no mention of them in any entry, live moment or text, apart from the marked lines.

- `GOD-09` **What came of your acts** *(Decided)*
  - **What:** For each act, a page shows its date, place and target, and what followed:
    - what your lightning hit, and any fire it lit;
    - each roll your fortune turned;
    - what a dreamer did after your dream, and any discovery it led to;
    - where a herd you drew went, and the hunts that followed;
    - for weather over a region, the notable events there while it lasted and soon after.

    You reach it from the act's marked line in the book of ages (`PRE-05`), from the details of a person it touched (`PRE-14`), or from its mark in the world while it lasts (`GOD-10`).
  - **How it works:** each act's direct results are saved with it, and the page follows them on through the history the world keeps, such as who learned a discovery from whom (`PRN-15`).
  - **Why:** Curiosity (`VIS-08`): you can find out what your nudges really did.
  - **Example:** You open the dream you sent Ama: the days of drilling that followed, the first fire made by drilling, and the people she taught.

## 5. Time and history

Time works one way for everything in the world: activities that start and end on one world clock, through a 60-day year, by the same rules at every zoom and every speed.
After the camera, time is also your main control.
This section sets how time works, how fast it runs and who sets its speed, what happens while you're away, how worlds and chance are kept, and the pace history should keep.
Two principles shape it: pace comes only from controlling time (`PRN-12`), and when the phone can't keep up, time slows rather than the world cutting corners (`PRN-11`).

### 5.1 How time works

- `TIM-17` **Activities with an end** *(Decided)*
  - **What:** Everything people and animals do is an activity with a start and an end: a walk to the spring, a strike at a flint, a meal, a night's sleep.
    Its results land when it ends.
    Anything can be interrupted, by the same rules for everyone.
    There are no separate close-up and far-away versions of anyone: everyone runs by the same rules wherever they are, at every zoom and every speed, so looking changes nothing (`WLD-13`).
  - **How it works:**
    - **One world clock:** everything in the world runs on one clock of game time, and the speed of time is only how fast that clock runs against real time (`TIM-01`).
    - **Length:** each activity lasts as long as it would in life (`TIM-18`): a strike a few seconds, a meal some minutes, a walk as long as the way takes at the walker's pace, and work by a blueprint the time the blueprint gives (`MAT-04`).
      A person's day holds about 10–30 activities.
    - **Results at the end:** the flake comes off as the strike ends, the meal feeds as it ends, and whether a blueprint succeeds is settled as it ends (`MAT-04`).
      Work can repeat within one activity, such as striking flake after flake, and each flake lands as its strike ends.
    - **The body keeps count:** hunger, thirst, warmth and rest change with the time that passes and the effort spent (`BIO-09`), and an activity ends early at the moment one of them turns urgent.
    - **Choosing:** when an activity ends, the doer chooses the next (`MND-09`, `MND-16`).
    - **Interruptions:** an activity also ends early when something the doer notices matters more than finishing it, such as a threat, a call or pain, or when the time comes for one of their plans (`MND-03`, `MND-09`, `MND-22`); a blow, a fall or death ends it at once.
    - **What an interruption keeps:** a walker stands where they had got to, and a sleeper keeps the rest they had; work on a thing stays in it, so a half-scraped hide or a half-built hut needs only the time left from whoever takes it up; anything else cut short simply doesn't happen.
    - **On the way:** where a walker is at any moment follows from the way and their pace, so they can be seen, met or attacked on the way.
    - **Things and the land:** fire and the timers on things, such as rotting and drying, end when their time is up, sooner or later as conditions change (`MAT-18`, `MAT-19`); weather, water, plants and herds far from people move in rounds of set length on the same clock (`WLD-12`).
    - **Same at any speed:** whatever ends at a given moment ends then, whether you watch it at real speed or race through the year, and things that end at the same moment are always settled in the same order.
  - **Why:** One simple way of running time keeps the world believable up close and fast from afar, and makes history the same whether or not you watch.
  - **Done when:** in test scenes, an interrupted walk leaves the walker where they had got to, a sleeper woken early keeps the rest they had, and a half-scraped hide is finished by someone else in the time left.
  - **Check:** the same saved world, run at different speeds and zooms, gives the same results on the same phone (`TIM-16`).
  - **Example:** Ama sets off for the flint cliff, two hours' walk away.
    Follow her step by step, or let the season race past over the valley: she reaches the cliff at the same moment.
    If a bear crosses the path halfway, her walk ends where she stands, and she chooses again.

- `TIM-18` **The game year** *(Decided)*
  - **What:** A game year is 60 days: four seasons of 15 days, spring, summer, autumn and winter.
    A day has 24 hours.
    What happens within days takes its real time, while what takes months or years in life is squeezed into the 60-day year, so it runs about six times faster.
  - **Real time:**
    - what people and animals do (`TIM-17`): walking 20–30 km a day, knapping a flake in seconds, a meal, a night's sleep;
    - hunger, thirst, warmth and rest (`BIO-09`), and the course of an illness, over days as in life (`BIO-05`);
    - weather, and day and night (`WLD-16`, `WLD-07`);
    - fire and the timers on things: meat rots in a few days, a hide dries over days, a pot fires in hours (`MAT-18`, `MAT-19`).
  - **Squeezed into the year:**
    - the seasons, with their warmth, daylight and weather (`WLD-05`);
    - plants: grasses and herbs grow within a season, trees over decades, and a crop goes from sowing to harvest within the growing seasons (`WLD-31`, `RCK-23`);
    - animals' breeding seasons, growing up and migrations (`WLD-32`);
    - people's lives: childhood to about 14 game years, old age from about 45, most dead by 70 (`BIO-04`, `BIO-16`), pregnancy about 45 days, three quarters of a year (`BIO-15`), and nursing;
    - healing, so even a broken bone mends over days (`BIO-13`);
    - slow decay: huts falling in, and bones and tools left behind being buried (`MAT-08`).
  - **Why:** Real years would make history far too slow to watch, and squeezed days would make every day look rushed.
    Squeezing only what takes months and years keeps each day believable while generations pass in an evening.
  - **Check:** catalogue tests confirm that every duration is real or squeezed as these lists say.
  - **Example:** Ama walks to the flint cliff and back between dawn and dusk, as she would in life.
    A child she conceives as spring begins is born as winter begins, and is grown 14 years later.

- `TIM-14` **Dates** *(Decided)*: Dates give the year, the season and the day, such as "Year 112, autumn, day 6".
  - **How it works:**
    - **Years** count from the start of history, which begins on Year 1, spring, day 1; each season's days run from 1 to 15 (`TIM-18`).
    - **One calendar:** seasons are named as in the half of the world where history begins; in the other half they are reversed (`WLD-01`), but dates keep the same names, so a date means the same day everywhere.
    - **Game time throughout:** ages, durations, targets and tests in this file are in game days and game years (`TIM-18`).
    - **Their own calendars** are separate: each people reckons time by its own signs (`CUL-13`).

### 5.2 How fast time runs

- `TIM-01` **Time follows zoom** *(Decided)*
  - **What:** By default, the closer you look, the slower time runs, and the further out, the faster.
    One gesture sets both where you look and how fast history moves (`PRE-33`).
  - **Speeds zoom asks for,** changing smoothly in between:
    - **one person:** real-life speed, one game second each real second (`TIM-10`);
    - **a camp,** a few hundred metres across: a day in a few minutes;
    - **a valley,** about 10 km across: a season in about a minute;
    - **a region,** about 100 km across: a few years a minute;
    - **the whole world,** the world map and the globe: top speed, as fast as the phone can (`TIM-07`).
  - **How it works:**
    - **Asked and real:** time runs at the speed asked, or as fast as the phone can while the screen stays smooth, whichever is slower (`PRN-11`); the speed shown is always the real one.
    - **Only the pace changes:** the rules are the same at every speed (`TIM-17`).
    - **What you see at speed:** the picture always shows the world as it is at that moment (`PRN-10`), and each figure keeps showing what it is doing even when it passes too fast to follow (`PRE-44`).
  - **Why:** Close-up moments are lived; distant ages are watched.
  - **Example:** You watch the knapper strike, flake by flake.
    Then you pull back over the valley, and a whole summer passes in a minute while the herds move north.

- `TIM-10` **Natural speed up close** *(Decided)*: At the closest zoom, one game second passes each real second, so people and animals move at real-life speed.
  You can watch a flake come off the stone.
  - **How it works:** each activity plays over its real length with its animation (`TIM-17`, `PRE-44`), and sounds play in real time (`SND-07`).

- `TIM-04` **Manual control** *(Decided)*: You can unlink speed from zoom whenever you want.
  The controls: pause, play, a speed dial, and a lock that keeps the current speed while you move the camera (`PRE-33`).
  - **How it works:** pause stops time; play hands the speed back to zoom; the dial sets a speed, from real speed to top speed, that stays wherever you look; and the lock keeps the speed you had when you set it.
    The real speed still can't pass what the phone manages (`PRN-11`).

- `TIM-15` **Who sets the speed** *(Decided)*: Your controls beat the story director, and the director beats zoom.
  Choosing a power pauses time.
  - **How it works:** the first of these that is active sets the speed, and none can make time run faster than the phone can (`PRN-11`):
    1. **pause,** yours or while you choose a power (`GOD-10`): no time passes;
    2. **overnight mode** (`TIM-12`): top speed, with the director's moments kept for the morning instead of slowing time;
    3. **skip** (`TIM-11`): top speed until the next important moment;
    4. **the dial or the lock** (`TIM-04`): the speed you set;
    5. **the story director** (`TIM-02`): a slower speed around an important moment;
    6. **zoom** (`TIM-01`): at all other times.

- `TIM-07` **Speed target** *(To test)*: How fast history can run at the world view on your phone, in game years per real minute, by the number of people.
  - **Targets:**
    - **1,000 people:** at least 1 game year per real minute, aiming for 2–10;
    - **about 2,000 people:** at least half a game year per real minute, still watchable; beyond that, time slows further (`MND-15`);
    - **fewer people** run faster;
    - **overnight,** about 8 hours: a few hundred to a thousand game years (`TIM-12`).
  - **What it takes:** each person does about 10–30 activities a game day (`TIM-17`), so 1,000 people at 1 game year a minute need about 10,000–30,000 activities settled every real second, each ending in a choice (`MND-09`).
  - **How it is measured:** at every stage, fixed saved worlds of about 100, 500, 1,000 and 2,000 people run at the world view on your phone, at the speed it can hold without heating up (`PLT-01`, `PLT-04`); the same worlds run in the cloud at every alpha to catch slowdowns early (`PLT-05`).
  - **Never by cutting depth:** speed comes only from these rules and from good engineering, never from simpler minds or bodies (`PRN-11`, `MND-14`).

### 5.3 The story director

- `TIM-02` **Story director** *(Decided)*
  - **What:** The director watches the whole world for important moments and sets the speed of time around them.
    When nothing important is happening, quiet years race past at the speed your zoom asks.
    When something important happens, or is about to, time slows, a live moment appears (`PRE-08`), and one tap takes you there.
    You stay in control of the camera.
  - **What counts as important:**
    - named discoveries and other firsts (`MAT-21`, `PRE-39`);
    - births and deaths among the people you follow (`PRE-06`);
    - crafts reaching a new people, or lost with their last holder (`CUL-02`);
    - fights, raids and feuds between groups (`CUL-31`);
    - disasters, such as wildfires, floods, quakes and hard winters (`WLD-22`);
    - peoples forming, splitting or dying out, and villages founded (`CUL-23`, `CUL-28`);
    - what follows your own acts (`GOD-09`).
  - **How it works:**
    - **Reading what happens:** it reads what the game recognises as notable (`PRE-39`).
    - **Signs:** it also watches present signs that often come before such moments, such as someone trying something new again and again, a predator stalking someone, a storm building over a camp, or two hostile groups meeting, so time can slow before the outcome; it never looks ahead in time.
    - **Importance:** each kind of moment or sign has a score, higher the more people it touches and when you follow them, tuned with you.
    - **Slowing down:** when a score passes the bar for live moments (`PRE-08`), time slows, more for higher scores: to valley speed for most moments, and to camp speed for the biggest, such as a named discovery or a death you follow (`TIM-01`).
      If you don't tap the moment within about half a minute (tuned), time speeds up again and the moment waits in the list.
    - **Only slower:** the director never asks for a faster speed than zoom does.
  - **Why:** In a world that runs itself, the best moments are easy to miss (`RSK-03`).
  - **Done when:** in test worlds watched from the globe, every named discovery and every death of someone you follow slows time and offers a live moment, and quiet stretches run at the speed zoom asks.
  - **Example:** You are watching the globe, and quiet years race by.
    Far to the east, a woman of the Tavu has been drilling dry wood all morning, and time slows.
    You tap the live moment, swoop down, and arrive in time to see the first ember glow.

- `TIM-03` **The director never touches events** *(Decided)*: The director controls speed only.
  It decides where to slow down, but never causes, changes or hides anything.
  - **Follows from:** `PRN-10` and `PRN-12`.
  - **How it works:** it only reads what has happened and the world as it is, and only sets the speed and the live moments; since speed changes nothing (`TIM-17`), history is the same with or without it.
  - **Check:** the same saved world, run with the director on and off, gives the same results on the same phone (`TIM-16`), and a code check finds no path from the director into the world.

- `TIM-11` **Skip to the next moment** *(Decided)*: A control that runs time at top speed until the next important moment, then slows down.
  Useful for short check-ins (`VIS-10`).
  - **How it works:** it asks for top speed until the director's next moment passes the bar (`TIM-02`), or until a game year has passed, then hands the speed back to whatever set it before (`TIM-15`).

### 5.4 While you're away

- `TIM-05` **Pauses when closed** *(Decided)*: When the app is closed or in the background, the world stops.
  Nothing happens while you're away, and every session starts exactly where the last one ended.
  Opening the app resumes time.
  - **How it works:** when the app leaves the screen, the world stops at that moment and is saved (`PLT-07`); nothing runs in the background, and reopening carries on from the same moment, with every activity under way where it was.
  - **Done when:** closing and reopening the app at any moment gives the same history as never closing it (`TIM-16`).

- `TIM-12` **Overnight mode** *(Decided)*
  - **What:** Leave the app open on the charger and switch on overnight mode.
    The world runs at top speed with the screen dimmed: a few hundred to a thousand game years in a night (`TIM-07`).
    When you come back, a summary tells you what happened, drawn from the book of ages (`PRE-05`).
  - **Why:** A living world needs hours to make its history (`PRN-11`).
    Overnight mode gives it those hours without you having to watch.
  - **Safeguards:** it runs only while the phone is charging, and it slows, then pauses, before the phone gets hot.
  - **How it works:** it asks for top speed, draws only a dim picture updated now and then, and keeps the director's moments for the morning instead of slowing for them (`TIM-15`).
    It follows the phone's own temperature warnings (`PLT-04`), and it pauses when the charger is unplugged.
    The morning summary takes the night's most important moments by the director's scores, worded by the writer from the book of ages (`PRE-37`), with dark events stated as plain facts (`PRE-17`).
  - **Example:** You start it before bed.
    In the morning: "540 years passed.
    Two bands merged by the river; a long drought pushed the eastern band over the hills; on the coast, people now keep goats."

### 5.5 Worlds and chance

- `TIM-06` **Rewind and branch** *(Dropped)*
  - **Dropped because:** saved history was cut in the realism pass: one full save of the world is estimated at a few GB, so a world keeps only its present state and its chronicle (`PRN-15`).

- `TIM-16` **Chance is local** *(Decided)*: Each chance event belongs to one being and one moment, so on the same phone and version, the same saved state always gives the same result.
  - **How it works:** every chance draw is made from a key of world, system, being, moment and purpose, so the same being at the same moment for the same purpose always gets the same draw.
  - **Why:** It makes tests repeatable (`WLD-13`, `TIM-03`, `RES-21`) and lets a world recover exactly after a crash (`PLT-07`).

- `TIM-13` **Comparing timelines** *(Dropped)*
  - **Dropped because:** branching was cut with saved history in the realism pass (`PRN-15`).

- `TIM-08` **Saved worlds** *(Decided)*: Several worlds are kept on the phone, each with its present state and its book of ages.
  You can switch between them.
  - **How it works:** each world keeps its seed and generator version (`WLD-08`), its present state and the areas people have changed (`PLT-07`, `WLD-12`), and its history and book of ages (`PLT-10`); switching saves the current world and opens the other.

### 5.6 Pace and endings

- `TIM-19` **Pace of discovery** *(To test)*: In typical worlds, each step of the arc first happens within its window of years.
  - **The windows,** in the world's dates (`TIM-14`):
    - sharp stone flakes: Years 1–5;
    - making fire: 5–30;
    - clothing and huts, each: 10–40;
    - pottery: 60–150;
    - tame dogs: 80–150;
    - herding: 120–250;
    - villages, with people living all year in one place: 100–300;
    - farming: 200–350;
    - copper: 300–500.
  - **When a step counts:** the first time the book of ages records it anywhere in the world:
    - for a craft (flakes, fire-making, clothing, huts, pottery, copper), its first named discovery (`MAT-21`);
    - for tame dogs, the first wolves kept long enough to become dogs, and for herding, the first herd kept and bred (`WLD-33`);
    - for villages, the first village (`CUL-28`);
    - for farming, the first crop sown and harvested on purpose (`RCK-23`).
  - **Typical worlds:** not every world reaches every step, and the order can differ; stalls and lost crafts are valid histories (`VIS-03`).
  - **How it is met:** only by tuning chances and amounts, the same for every world, never by scripting or dates (`PRN-17`, `RES-16`).
    A window changes only with your OK (`RES-09`).
  - **Check:** the pace tests (`RES-07`): for each step, at least half of about 20 whole worlds reach it inside its window, and at most a quarter before it opens.

- `TIM-09` **If everyone dies** *(Decided)*: The world goes on without them.
  Nature carries on, and you can keep watching or start a new world.
  - **How it works:** the last death is an important moment for the director (`TIM-02`) and an entry in the book of ages (`PRE-05`); the land, weather, plants and animals carry on by the same rules, faster with no people, and the game asks whether you want to keep watching or start a new world (`WLD-10`).

## 6. World

The world is a small planet that wraps around, made once in realistic detail: moving plates, mountains, rivers, climate, soils, plants, herds, and deposits of useful stone where geology puts them.
After that it lives at a pace the phone can carry: weather every game hour on cells of about 10 km; plant cover, fire, water and herds on cells of about 1 km; full detail only where people are.
Things and their materials are in section 7, bodies and illness in section 8, and how animals think in `MND-16`.

### 6.1 Shape and size

- `WLD-01` **Torus with latitude** *(Decided)*
  - **What:** The map wraps both ways: walk east long enough and you come back from the west.
    The equator runs across the middle and the poles lie along the line where the map wraps north to south, so climate and seasons behave as on a planet, with the seasons reversed between the northern and southern halves.
  - **The polar seam:** a permanent ice cap about 200 km wide, about two weeks' walk, lies along that line.
    Weather stops at it, and nothing ever crosses its middle, not even a people able to carry food and fuel over that much ice: a deliberate exception, so the wrap never shows.
  - **Why:** There are no edges and no stretched regions, so every place is simulated the same way.

- `WLD-02` **Globe view** *(Decided)*: Fully zoomed out, the world is drawn as a globe, with the polar ice hiding the seam.
  The globe squeezes the polar lands, which on the map are as wide as the equator; this is in the picture only, and the map keeps every place at its true size.

- `WLD-03` **Size** *(Decided)*: About 1,000 km from pole to pole and 2,000 km around: about 2 million km² of land and sea.
  A climate zone is roughly 100 km wide, four or five days' walk.
  - **Why:** Big enough for many separate peoples, small enough for one phone to keep all of it alive.

- `WLD-30` **What scales with the world** *(Decided)*: Things set by distance are scaled to this small world, about 1 to 20; things set by bodies and materials keep their real size.
  - **How it works:** a storm system is about 50 km across instead of 1,000, climate belts are about 100 km wide, and herds migrate tens of kilometres instead of hundreds.
    A person, a tree, a day's walk and a fire keep their real sizes, and winds their real speeds, so storms cross this world faster.
    Times measured in years are squeezed into the 60-day year instead (`TIM-18`, `WLD-05`).
  - **Check:** every scaled value in the catalogues is marked as scaled, with the Earth value it came from.

- `WLD-04` **How many people it can feed** *(To test)*: Estimated at tens of thousands of foragers, about one person per 10 km² of good land, and ten to a hundred times more with farming.
  Nothing sets this number: it is however many people the land's food keeps alive (`WLD-18`, `BIO-09`), measured by running worlds.
  It lies far above what the phone runs (`MND-15`), so the land limits people only locally: a crowded valley, a hard winter.

### 6.2 Layers and systems

- `WLD-12` **Map layers** *(Decided)*: The world is held in four layers, each kept at its own pace.
  Detail is made where people are, and looking never changes it (`WLD-13`).
  1. **World cells,** about 1 km across, about 2 million of them: height, rock, soil fertility, biome and plant cover, rivers and lakes, sea, deposits of useful stone, clay, ochre and ore, snow, fire, and the herds passing through.
     They are always simulated, at a coarse pace: plant cover, fire, water and herds.
  2. **Areas,** about 256 m across, 16 to a world cell, with detail down to about 1 m: the ground's shape and material, rocks and loose stones, each tree, bush and flower, caves and overhangs, and water.
  3. **Things and creatures** live inside areas: every person, every animal near people, and every item (`MAT-10`).
  4. **Weather cells,** about 10 km across, about 20,000 of them, updated every game hour (`WLD-16`).
  - **How it works:**
    - **Paces:** weather every game hour; a wildfire every hour while it burns (`WLD-28`); water and herds once a game day, and water every hour during a flood (`WLD-17`, `WLD-32`); plant cover every few days (`WLD-31`); people, animals and things by their own activities (`TIM-17`).
      All run on one world clock, and each pace is tuned by measurement (`PLT-04`).
    - **Areas are made when needed:** an area is made from the world's seed and its world cell when people first go there, or when you look at it, which makes the same area.
      It matches its cell: a forest cell gives a wood of its species and ages, a river cell its stream and banks, a chalk cell its flint.
    - **Kept or forgotten:** an area people have changed (something made, moved, left, cut, dug or burned) is kept; an unchanged one is forgotten once no one is near, and made again the same way when needed.
      Kept areas live on by the same rules whether anyone is there or not.
    - **The layers agree:** what happens in an area counts in its world cell, so a grove cut or a slope burned lowers the cell's plant cover, and an animal killed leaves its herd's count.
      What happens to the cell, such as a fire, a flood or the turn of the season, reaches its areas by the same rules.
    - **Animals:** far from people, herds are counts in world cells; near people, their animals become individuals in areas (`WLD-32`).
    - **Weather on the ground:** each place takes its weather cell's weather, adjusted for its own height and shelter (`WLD-16`).
    - **Zoom:** the globe and the world map show world cells, a region adds the weather, and closer in you see areas with their things and creatures (`PRE-03`).
  - **Done when:** in test scenes, a stretch of land run with its areas made, and with them forgotten, ends a year with the same plant cover and herd numbers, within a tuned margin.

- `WLD-13` **Looking changes nothing** *(Decided)*: Where you look, and how fast time runs, never change what happens.
  Places nobody has visited are drawn from the seed and their world cell, exactly as people would find them (`PRN-10`).
  - **How it works:**
    - **One way only:** the picture and sound read the world; nothing in the world reads the camera, the zoom or the speed.
    - **The same area:** an area made for the picture is the one people would get (`WLD-12`).
    - **Counted herds** are drawn as that many animals of the right kinds and ages, placed from the seed, and show nothing the counts don't hold.
    - **Speed:** the same rules run at every speed and zoom (`TIM-17`); only how much game time passes per real second changes.
  - **Check:** the same saved world, run with the camera in different places and at different speeds, gives the same results on the same phone (`TIM-16`).

- `WLD-29` **Systems feed each other** *(Decided)*: Weather, water, soils, plants, animals and fire read and change the same world cells, so each feeds the others.
  - **How it works:** weather brings rain, snow, warmth, frost, wind and lightning; water feeds plants, and floods leave silt; soils feed plants; plants feed animals and fuel fires; animals graze plants and feed hunters; fire clears plants, leaves ash and brings new grass.
    People change plants, soils and herds through the same links, never the climate (`SCP-21`).
    Nothing comes from nothing: what is eaten, cut or burned comes off the cover or herd it came from (`MAT-09`).

### 6.3 Making a world

- `WLD-08` **Realistic, not from scratch** *(Decided)*: Each world is made directly in a believable present-day state, by fast rules that imitate what deep time would have made, without simulating its history (`SCP-12`).
  - **How it works:**
    - **Stages:** generation runs the stages of `WLD-09` in order, each from the seed and the stages before it.
    - **What is made:** world cells and weather cells for the whole world; areas are never made in advance (`WLD-12`).
    - **Repeatable:** the same seed always makes the same world in the same version of the game, so any change to the generator is a big update (`PLT-09`).
    - **Settling:** once you pick a world, its plant cover, water and herds run for a few game years (tuned) by the play rules, with no people, until they only swing with the seasons.
      Then the start region is found again (`WLD-24`), and history begins in year 0.
      Animals around the start begin with the wariness that living beside hunters gives (`WLD-32`).
  - **Done when:** across many seeds, worlds fall inside Earth's usual ranges for the spread of heights and slopes, how rivers branch, how ragged the coasts are, the sizes of lakes, and the share of each kind of climate.

- `WLD-09` **What generation makes** *(Decided)*: Generation follows the real order of causes, each stage built on the ones before, by rules modelled on how Earth's land took shape:
  1. plates, mountain ranges, volcanoes and faults;
  2. rock layers;
  3. erosion: valleys, rivers, lakes, coasts and caves;
  4. climate, from the geography (`WLD-16`);
  5. soils (`WLD-27`);
  6. deposits of useful stone, clay, ochre, salt and ore (`WLD-14`);
  7. biomes and plant cover (`WLD-31`);
  8. animals (`WLD-32`).
  - **How it works:**
    - **Plates:** about 6 to 12 plates (tuned), each of ocean or continental crust and each moving its own way; the continents make up the world's share of land (`WLD-06`).
      Where plates push together, mountain ranges rise; where an ocean plate dives under another, a line of volcanoes rises beside a deep trench; where plates pull apart, rift valleys open; where they slide past each other, faults run.
      Continents are pieced together from older blocks, so they hold worn-down old ranges, basins and ragged coasts, not flat plains with straight edges.
    - **Rock:** each world cell gets its stack of rock layers from its history: old hard rock under the continents; sandstone, shale, limestone and chalk where seas and basins lay; lava and ash near volcanoes; folded rock in the ranges; granite where molten rock cooled underground.
    - **Erosion:** rain gathers into streams and rivers that cut valleys, deepest where the most water flows, the slope is steep and the rock soft; slopes slump to what they can hold; sand and mud settle where the water slows, building floodplains, fans and deltas.
      Hollows fill into lakes up to their outlets, and every river reaches the sea or a lake.
      High mountains and the polar lands keep glaciers, with wide U-shaped valleys and lakes below them.
    - **Caves** form where water dissolves limestone along its cracks, where lava drained out of tubes, and where soft rock wore away under hard, leaving overhangs.
      Each world cell records its caves and their sizes; their exact shape comes with their area (`WLD-12`).
    - **Biomes and plant cover:** each world cell gets the biome its climate, soil and wetness support, and a cover at some stage of regrowth since its last fire or flood, drawn at the rates such events have in that climate, so the land is a patchwork of old forest, regrowing burns, meadow, marsh and scrub.
    - **Animals:** each species' herds are placed where its food and cover are, in the numbers that food can feed; herds that migrate get summer and winter ranges.
    - **Stages that need each other:** erosion needs rain before the climate exists, so it uses a first, rough climate from latitude, the sea and the heights.

- `WLD-14` **Deposits placed by geology** *(Decided)*
  - **What:** Useful stone, clay, ochre, salt and ore lie where the rocks and rivers put them, so what a people can discover depends on where it lives.
  - **How it works:**
    - **Where each lies:** flint as nodules in chalk, and chert in some limestones; obsidian in young lava from sticky, silica-rich volcanoes; tough stones for hammering and grinding, such as quartzite, basalt and sandstone, where those rocks show; clay along rivers, in old river bends and lake beds, and where feldspar-rich rock weathers; red and yellow ochre where iron-rich rock weathers; salt where closed lakes dry, at salty springs and in shore lagoons; green and blue copper ore, with a little native copper, at the weathered tops of copper-bearing rock near granites in volcanic ranges.
    - **How much:** each world cell records which deposits it holds and how rich each is.
    - **Rivers carry stones:** a river's gravel holds stones from the rocks upstream, rounded with distance, so flint from chalk hills turns up in valley gravels far away.
    - **Seen where the land is cut:** in river banks, cliffs, screes, cave walls and the roots of fallen trees; deeper deposits are reached only by digging (`MAT-06`).
    - **In areas,** deposits become things: nodules in a chalk bank, cobbles on a gravel bar, a bed of clay in a river bend; pieces vary in quality (`MAT-20`), so some flint is better than other flint.
    - **Seen, not labelled:** a deposit looks like what it is (a green stain, red earth, a dark glassy rock), and what it is good for is learned by use (`MAT-03`, `PRN-01`).
  - **Example:** Three valleys from the first camps, a cliff of green-stained rock waits for a people whose kilns already burn hot (`MOM-12`).

- `WLD-19` **Species from Earth families** *(Decided)*: The plants and animals are Earth species or close kin, with their real habits, seasons and sizes, so the world is familiar and believable.
  - **How it works:**
    - **The catalogue** holds about 60 plants (`WLD-31`) and about 30 animals (`WLD-32`), each with where it can live (warmth, wetness, soil, cover), its seasons, its size, and what it yields, with the characteristics of each yield (`MAT-03`), set by hand from the real species (`PRN-05`).
    - **Use is learned:** whether a berry is food, poison or medicine is in its characteristics, which people learn by trying; some look-alikes differ, such as a sweet root and a deadly one (`MAT-03`).
  - **Why:** Familiar species make the world readable at a glance, and their real habits make gathering, hunting and taming believable.

- `WLD-23` **Every habitat lived in** *(Decided)*: Every landscape has its plants, its plant eaters and hunters, its birds, and fish in its waters, so no land is empty or still.
  - **How it works:** the catalogue covers every biome a world can have, from tundra to the warm belt and from desert to marsh, and generation places each species wherever its needs are met (`WLD-09`), so a world uses most of the catalogue.

- `WLD-10` **Generate several, offer the best three** *(Decided)*
  - **What:** The game makes several candidate worlds, scores each, and never edits them.
    "New world" shows the best three as small globes, each with a one-line summary of its facts.
    You pick one, let the game pick the top score, or enter a seed instead.
  - **How it works:**
    - **Two passes:** about 20 candidates (tuned to fit `WLD-11`) go through plates, rock, erosion and a rough climate, and are scored on those; the best few go through every stage and are scored again.
    - **Must have:** a start region that qualifies (`WLD-24`) and, on the same landmass, everything the arc needs (`TIM-19`): stone that flakes, clay, wild grains, wolves and a herd animal with a domestic kind (`WLD-33`), and copper ore.
    - **Scores:** varied landscapes and climates; barriers such as mountains, seas and deserts that let separate peoples form; resources spread unevenly, flint here and copper there; and the start region's own score.
      The weights are tuned.
    - **Never edited:** a world is offered exactly as made, or not at all; if fewer than three qualify, more are made.
    - **Your own seed** makes that one world and finds its start region the same way, or says it has none.

- `WLD-24` **Where history begins** *(Decided)*: The bands start in a temperate region with caves, fresh water, varied food and stone that flakes within reach, found by scoring, never placed by hand.
  - **How it works:** a region qualifies when, by the world's own rules:
    - **winters** matter, with the coldest season averaging below about 10 °C (tuned), yet people without clothes or fire can live through them, sheltering in caves and huddling together (`BIO-11`);
    - **caves:** there is a dry cave or overhang big enough for each band;
    - **water** lasts all year within about 2 km of each shelter;
    - **food** the bands can get with the starting kit (`BIO-02`) is enough within about 10 km of the shelters, a day's walk there and back, in every season with a margin (tuned), from several kinds, so that one failing doesn't starve them;
    - **stone that flakes** lies within the same reach, so the sharp-stone test can happen in every world (`RES-02`).
  - **Ranking:** among regions that qualify, bigger margins of food, more kinds of food, and more shelters and water score higher.
  - **The bands' places:** each band gets one shelter as its home and the land around it as its home range, next to its neighbours (`BIO-03`).

- `WLD-11` **Generation time** *(Decided)*: Making the candidates and offering the best three takes a few minutes on the phone.
  - **How it works:** only the best few candidates get every stage (`WLD-10`), and nothing is made down to the metre in advance (`WLD-12`).
    If measurement (`PLT-04`) shows it running long, fewer candidates are made, never less detail in each.
    Settling the chosen world (`WLD-08`) comes after, and is measured too.

### 6.4 Sky, climate and weather

- `WLD-06` **Varied within reason** *(Decided)*: Each world has its own axial tilt (15° to 30°, which sets how strong its seasons are), share of land (25% to 50%), continents, seas and star field, all drawn from the seed.
  Every world has one moon, a 24-hour day and the 60-day year (`TIM-18`); gravity, air and water are as on Earth.

- `WLD-07` **The sky** *(Decided)*
  - **What:** The sun, moon and stars move as they would for the world's tilt, with the year squeezed into 60 days (`TIM-18`).
    Eclipses, comets, meteor showers and auroras happen.
  - **How it works:**
    - **Sun:** its height and the length of daylight follow latitude and date, so summer days are long and winter days short, more so toward the poles, with midnight sun and polar night near the ice.
    - **Moon:** it waxes and wanes once a season, every 15 days, so each season has one full moon; full-moon nights are bright enough to walk and hunt by.
    - **Events:** eclipses when sun and moon line up, a few in a lifetime at any place; comets on paths drawn from the seed; meteor showers on the same days each year; auroras near the poles.
    - **Seen like anything else:** people see the sky with their senses (`BIO-18`) and can learn its cycles (`CUL-13`).
  - **Why:** The sky is the first calendar, and a great source of myth.
  - **Example:** A comet hangs over the valley through the autumn the old leader dies, and the band remembers that winter by it.

- `WLD-16` **Climate and weather** *(Decided)*
  - **Climate from geography:** at generation, each place's climate (its warmth, rain, snow and winds through the seasons) is worked out by rules modelled on Earth's.
    The sun's warmth comes from latitude; wind belts are like Earth's, steady easterlies near the equator and westerlies farther out, shifting with the seasons; rain falls where moist sea air is lifted over land and mountains, leaving dry rain shadows behind them; deserts lie in the dry belts either side of the tropics and far inland; warm currents make coasts mild and wet, cold ones cool and foggy (`WLD-26`); and the air is colder higher up.
    The climate stays the same through history: no ice ages, and no change made by people (`SCP-21`).
  - **Weather** in each weather cell, every game hour: temperature, humidity, wind, cloud, rain and snow.
  - **How it works:**
    - **Seasons** come from latitude and tilt (`WLD-06`), squeezed into the 60-day year (`WLD-05`), and reversed between the two halves of the world.
    - **Day and night:** the sun warms the land by day and clear nights are coldest; deserts swing most, coasts least.
    - **Storm systems** are drawn from each place's climate (how often, how big and how wet, by season) and move with the prevailing winds, dropping most of their rain on the slopes that face them.
    - **Thunderstorms** build in warm, moist air and bring lightning (`WLD-28`).
    - **Snow** falls when it is cold enough and lies on each world cell until warmth melts it, feeding the rivers (`WLD-17`).
    - **Height and shelter:** air is about 6 °C colder for each 1,000 m up; within a weather cell, each place's weather is adjusted for its height, slope and shelter, with cold air and frost pooling in hollows.
    - **Good and bad years:** chance brings runs of wet, dry, warm and cold years at about the rates such runs have on Earth in that climate (tuned), which is where droughts and harsh winters come from (`WLD-22`).
    - **A great eruption** can cool the world for a year or two (`WLD-15`).
  - **Done when:** across many test worlds, the weather's long-run average matches each place's climate, and each kind of climate's share and place are like Earth's.
  - **Example:** Rain off the western sea falls on the mountains, so the valleys beyond are dry grassland, with forest only along the rivers.

- `WLD-05` **Climate on a small world** *(Decided)*: Climate zones sit closer together than on Earth, a few days' walk apart, and the seasons are squeezed into the 60-day year while each day's weather runs at real speed.
  - **How it works:**
    - **In space:** latitude changes about one degree every 5.6 km, so climate belts are about 100 km wide; storm systems are scaled (`WLD-30`), while thunderstorms, breezes and frost hollows keep their real size.
    - **In time:** storms, showers, snowfall and melt, and each day's warming and cooling run at real rates.
      The seasons' warmth and daylight follow the squeezed year (`TIM-18`), with land and sea warming and cooling within it, so a season here is as warm or as cold as on Earth at the same latitude.

### 6.5 Water and soil

- `WLD-17` **Fresh water** *(Decided)*: Rivers, lakes, springs, marshes, snow and ice, with floods and dry spells; life and settlement gather around them.
  - **How it works:**
    - **Each world cell's water,** once a game day: rain and melt soak into the soil or run off, more off clay and frozen ground (`WLD-27`); ground water seeps downhill and comes out as springs at the foot of slopes and in limestone country, keeping streams flowing in dry spells.
    - **Rivers** carry water downhill along the network from generation at real speeds, so a storm's water reaches the lower valley later as a flood wave; small streams dry up in a long dry spell, and big rivers shrink.
      Rivers keep their courses; each has its line, width and depth, from which areas make their banks, bars and pools.
    - **Lakes** rise with what flows in and fall with what flows out and dries away; a lake with no outlet in a dry land turns salty and leaves salt where it dries (`WLD-14`).
    - **Floods** come from heavy rain, fast melt or a burst ice dam: water spreads over the floodplain, drowning and carrying things, and leaves silt that raises the soil's fertility (`WLD-27`) and buries what lay there (`MAT-08`).
    - **Marshes** form on flat or badly drained ground and below springs, with reeds, and with peat that keeps what falls into it (`MAT-08`).
    - **Ice:** rivers and lakes freeze in hard cold, and the ice bears a person once it is thick enough; thin ice breaks.
      Glaciers stay as generated, and their summer melt feeds rivers.
    - **Clean water:** water below camps, herds or carcasses, or standing still, can carry illness (`BIO-05`).
    - **Gathering around water:** nothing is placed there; plants grow better near water, and animals and people go to it because they need it every day (`BIO-09`).

- `WLD-26` **Seas** *(Decided)*: Seas and oceans with currents that carry warmth, a fixed sea level with no tides, and shores rich in food.
  - **How it works:**
    - **Currents** are worked out at generation from the winds and the shapes of the seas: warm water flows toward the poles along one side of each ocean and cold water returns along the other; where wind pushes surface water off a coast, cold water rich in food wells up, with the best fishing.
    - **Each sea cell** keeps its warmth through the seasons, its ice, and its fish and sea mammals (`WLD-32`), most where rich water wells up and in shallow seas.
    - **Shores** are beaches, rocky shores, estuaries and salt marsh, from the rocks and rivers; shellfish beds in the shallows are gathered like plants and grow back by the season.
    - **No tides,** and the sea stays at one level, so coasts never move (`SCP-21`).
    - **Storms** raise waves that pound coasts and flood low shores; cold seas freeze in winter, in places thick enough to walk on; the polar ice never melts (`WLD-01`).
    - **Salt water** can't be drunk (`BIO-09`), and leaves salt where it dries in shallow lagoons (`WLD-14`).

- `WLD-27` **Soils** *(Decided)*: Each world cell has a soil and a fertility from 0 to 5, which decide what grows there and how well.
  - **How it works:**
    - **At generation,** the soil comes from what lies beneath (rock, river silt, wind-blown dust, volcanic ash or peat), the climate, the slope and the plants: deep and rich on river silt, wind-blown dust, ash and old grassland; poor on sand, steep slopes and peat, and where heavy rain in hot lands washes it out.
    - **Its kind** decides how it holds water (sand dries fast, clay stays wet and puddles) and how easily it is dug.
    - **Fertility changes with use:** each harvest carried away lowers it, so a field yields less year after year; resting land recovers over a few years; ash, dung, rotted waste and flood silt raise it quickly.
      Fields and other worked ground keep their own fertility, in their area (`WLD-12`).
    - **Rich spots:** ground where ash, dung and food waste pile up, such as a camp's rubbish heap, grows richer than the cell around it, so seeds dropped there grow well (`MOM-08`, `RCK-23`).
    - **What buried things keep:** wet, airless peat keeps wood and hide, dry caves keep bone, and acid soils eat bone away (`MAT-08`).

### 6.6 Plants and animals

- `WLD-31` **Plants** *(Decided)*
  - **What:** About 60 species from Earth families: trees, bushes, grasses including wild grains, herbs, roots, reeds and flowers, with a few mushrooms and tinder fungi counted among them.
    Each has where it can grow, its seasons, its growth, and its yields (fruit, nuts, seeds, wood, bark, fibre, leaves or roots), each with its characteristics (`MAT-03`).
  - **How it works:**
    - **In areas,** each plant is a thing (`MAT-10`) with a growth stage (seedling, young, grown, old or dead), a season state (bud, leaf, flower, fruit or bare) and yields that ripen with the season and are taken by gatherers and animals.
    - **Seasons:** each species buds, flowers, fruits and goes bare at its own time of year, earlier after a warm spell and later after a cold one (`WLD-16`).
    - **Growth** is squeezed with the year (`TIM-18`): grasses and herbs grow within a season, bushes over a few years, trees over decades.
    - **Spreading:** each season, each plant has a chance to start a new one of its kind nearby, where the ground suits it and there is light and room; seeds people drop can sprout too (`RCK-23`).
    - **Dying:** from drought, frost beyond its limits, fire, deep shade, being eaten or trampled, or old age; dead wood lies as fuel and rots (`MAT-19`).
    - **Far from people,** each world cell keeps its plant cover, worked out every few days: how much is trees, bushes, grass and herbs, reeds and bare ground; which species lead; how old the trees are; and how much food is ripe now.
      Cover grows back toward its biome at each species' pace, follows the season, and is grazed, burned and flooded.
    - **Biome and cover:** the biome is what the cell's climate, soil and wetness would grow if left alone, such as pine forest, oak wood, grassland, scrub, tundra, desert or marsh; the cover is what grows there now, so a burned oak wood is grass and young trees on the way back to oak.
  - **Example:** A hazel thicket by the river fruits in early autumn; the band strips it, wild boar take the fallen nuts, and next autumn it fruits again.

- `WLD-32` **Animals** *(Decided)*
  - **What:** About 30 species from Earth families, no insects: mammals, birds, fish and a few reptiles.
    Each has its diet, group size, speed, danger, yields (meat, fat, hide, fur, bone, antler, sinew or feathers) and seasonal habits, such as breeding, moulting, migrating and sleeping through winter.
  - **How it works:**
    - **Far from people,** big animals live in herds, packs, flocks or alone, each kept as a count of adults and young, with its condition and its wariness of people, passing through the world cells.
      Once a game day each moves toward food, water and cover and away from hunters, along seasonal routes where it migrates.
      Small, plentiful animals, such as hares, small birds and most fish, are kept as how many live in each world cell or stretch of water.
    - **Numbers change at a coarse pace:** young are born once a year in each species' season, more when the mothers are well fed, and grow up and age in game years (`TIM-18`); animals die of hunger, cold, hunters, illness and old age (`WLD-18`).
    - **Near people,** within about 3 km of anyone (tuned), a herd's animals become individuals in the areas they are in, each with a body (`BIO-19`) and a simple mind (`MND-16`), its age, sex and condition drawn from the count.
      They rejoin the count once no one has been near for a day; animals people have hunted, wounded, fed or tamed stay individuals for good.
    - **Fear of people:** a herd that loses animals to hunters, or is chased, grows warier, fleeing sooner and keeping farther off; where no one hunts, wariness fades over the years, and the young take it from their herd.
    - **Fish runs:** fish that swim upriver to breed crowd the rivers in their season, a time of plenty.
    - **Danger:** big hunters such as wolves, bears and big cats may attack people when hungry, cornered or guarding young; numbers, noise and fire keep them off (`MND-16`).
    - **Illness:** some animals carry illnesses that pass to people who handle them, eat them or share their water (`BIO-05`).
    - **Your animal dreams** can draw a herd toward a place, or make it calmer or bolder (`GOD-12`).
  - **Example:** In a valley hunted every autumn, red deer bolt at a hundred metres; over the ridge, where nobody hunts, they let a person walk close.

- `WLD-33` **Taming and domestic kinds** *(Decided)*
  - **What:** Animals fed and kept near people grow tame; young born to tame animals kept by people are tame from birth; and a line kept by people for several generations becomes a domestic kind, such as wolf to dog or wild goat to goat.
  - **How it works:**
    - **Tameness** runs from 0 (wild) to 5 (tame) for each animal: food, and time near people without harm, raise it slowly, and harm lowers it; young raised by people tame fast, grown animals rarely (`RCK-24`).
    - **Tame animals** stay near people, follow them, can be penned or tethered, and breed there when kept together.
    - **Domestic kinds:** only species with a domestic kind in the catalogue can become one: wolf to dog, wild goat to goat, wild sheep to sheep, wild cattle to cattle, wild boar to pig.
      Others, such as deer, can be tamed one by one but never bred into a kind.
      After about five generations (tuned) born among people, a line becomes its domestic kind: calmer, quicker to breed, with its own look.
    - **Kept herds** are individuals that graze the plant cover around them (`WLD-31`), need water and guarding, and are the start of herding.
    - **Recorded:** a people's first domestic kind enters the book of ages, named in their language (`PRE-05`).
  - **Example:** Children feed scraps to pups from a wolf den near camp; the pups' own pups grow up at the hearth; five generations on, the band has dogs (`MOM-06`).

- `WLD-18` **Ecology** *(Decided)*: Plants grow, fruit and die back with the seasons; animals eat, breed, migrate and die; numbers boom and crash with the weather and with each other, never by script.
  - **How it works:**
    - **Plant eaters** eat the cover of the cells they are in (grazers grass, browsers bushes and young trees, others fruit, nuts and roots); what they eat comes off the cover, and heavy grazing thins it.
    - **Hunters** take prey by chance each day, more often when both are plentiful and the prey is young or weak; each kill is a whole animal, and its carcass feeds scavengers.
    - **Condition:** well-fed animals fatten and breed well; hungry ones grow thin, breed less, and die first in a hard winter or deep snow.
  - **Done when:** test worlds run for 100 game years without people keep every species within believable numbers for its habitat and believable ratios of hunters to prey, with none dying out or overrunning the land.
  - **Example:** A run of mild winters lets the deer multiply; the wolves follow; a hard winter cuts both down, and the hunters go hungry.

### 6.7 Fire, quakes and other events

- `WLD-28` **Fire in the landscape** *(Decided)*: Lightning and dry fuel start wildfires that spread with wind and slope; the land regrows after them, and some plants need fire.
  People can learn to use fire on the land.
  - **How it works:**
    - **Fuel** comes from each world cell's plant cover, and dries or wets with the weather: dead grass dries within a day of sun, forest litter over days, logs over weeks.
    - **Starting:** lightning (`WLD-16`), lava (`WLD-15`), or a fire people leave or set, wherever the fuel is dry enough.
    - **Spreading,** checked every hour while it burns: from cell to cell, fastest through dry grass, downwind and uphill, slowly through damp forest, with wind throwing embers ahead; it stops at water, bare rock, snow and burned ground, and in rain.
    - **In areas,** it burns thing by thing by the fire rules (`MAT-18`), so people can fight it, flee it or be caught.
    - **After fire:** cover burns by how hot the fire was; ash raises fertility for a few years (`WLD-27`); grass returns within the season, bushes over a few years, forest over decades (`WLD-31`); some trees sprout from their roots or drop seed only after fire; grazers come to the new grass.
    - **How often places burn** follows from climate, lightning and fuel, checked against Earth's usual intervals: grassland every few years, dry forest every few decades, wet forest rarely.
    - **People** spread fire by the same rules; whether they ever burn land on purpose, to drive game or bring new grass, is theirs to discover (`PRN-01`).

- `WLD-15` **Quakes and eruptions** *(Decided)*: Rare earthquakes strike along the faults made at generation, and volcanoes erupt where geology allows; otherwise the land keeps the shape it was made with.
  - **How it works:**
    - **How often:** each fault and volcano has its own chance each year, set by its kind, at about Earth's rates for the same area, so a people may see a large quake or eruption once in several lifetimes (tuned).
    - **Quakes:** shaking is strongest near the fault and fades with distance; shelters and stacked things fall (`MAT-11`), and rocks drop from cliffs and cave roofs; a strong quake under the sea sends a great wave onto low coasts.
    - **Eruptions** give warning days to weeks ahead (small quakes, rumbling, gas, warm springs), which people can notice (`BIO-18`).
      Runny lava flows slowly downhill, setting fires (`WLD-28`); sticky lava blasts out ash and leaves obsidian (`WLD-14`).
      Ash drifts with the wind and falls thinner with distance, smothering plants and fouling water, and in later years makes rich soil (`WLD-27`).
      A great eruption dims the sun and cools the world for a year or two (`WLD-16`).
    - **Nothing else moves:** during play the land does not wear down, rivers do not wander, slopes do not slide on their own, and coasts stay put.

- `WLD-22` **Natural events** *(Decided)*: Lightning, wildfires, storms, droughts, floods and harsh winters, and rarely quakes and eruptions, come from the world's own systems, not only from you.
  - **How it works:** none is ever scheduled; each is the far end of its own system: droughts and harsh winters from runs of bad years (`WLD-16`), floods from rain and melt (`WLD-17`), storms whose winds fell trees and flatten shelters (`MAT-11`), wildfires (`WLD-28`), and quakes and eruptions (`WLD-15`).
    Your powers work through the same systems (`GOD-05`).
    Big events that touch people enter the book of ages (`PRE-05`).
  - **Check:** how often each kind of event comes, in each climate, stays within Earth's usual rates, and every event traces back to the state of its system.

### 6.8 Cut from the world

Cut from the launch design (see also `SCP-21`): microbes as a simulated system, heredity and evolution of plants and animals, insects, ice ages, tides, people changing the climate, and slow geology during play.

- `WLD-20` **Heredity in plants and animals** *(Dropped)*
  - **Dropped because:** plants and animals don't evolve during play; tame lines and domestic kinds take its place (`WLD-33`).

- `WLD-21` **Microbes** *(Dropped)*
  - **Dropped because:** microbes are not simulated; rot and fermenting are timers on things (`MAT-19`), and illness has its own rules (`BIO-05`).

- `WLD-25` **People change the climate** *(Dropped)*
  - **Dropped because:** cut from the launch design (`SCP-21`); people change plants, soils and herds, never the climate.

## 7. Things and blueprints

The core of the game: what things are, and how people change them.
Every thing has the same 18 characteristics, and every way of making or changing things is a blueprint: hidden, generic and found only in play (`SCP-04`, `PRN-07`).
Nothing simulates chemistry or cracks underneath (`SCP-21`): values are plausible and set by hand (`MAT-05`), and short reality rules keep results believable (7.6).
How people discover, practise and teach blueprints is in Minds (`MND-06`, `MND-11`, `MND-13`).

### 7.1 Things

- `MAT-01` **Things are made of materials** *(Decided)*: Every thing is made of one or more materials, such as flint, birch wood, deer hide or clay; a spear is hazel, flint and sinew.
  - **How it works:** a material has a colour, base values for the 18 characteristics (`MAT-03`), and one of nine classes: stone, earth (clay, sand, ochre, salt), wood, plant, bone (with antler, horn and teeth), hide (with fur and sinew), flesh (with fat), metal or water.
    The class sets a thing's sounds (`SND-06`), whether it rots (`MAT-19`) and how long it lasts in the ground (`MAT-08`).
    Each part of a made thing keeps its material and is drawn in it (`PRE-42`); a broken spear leaves a shaft and a point.

- `MAT-10` **Items** *(Decided)*: An item is a kind of thing in the catalogue, such as flint, sharp flake or sewn cloak: about 200 at launch.
  - **How it works:**
    - **About 90 raw:** stones, earths, woods, barks, fibres, plant foods, herbs, animal parts, water, ash and charcoal; species that yield alike share items, so most deer give the same hide (`WLD-31`, `WLD-32`).
    - **About 110 made:** tools, weapons, containers, clothing, shelters, foods, medicines, art, instruments and copper things (`MAT-23`).
    - **Each item** lists its materials, form, characteristics, usual size, what each timer makes of it (`MAT-19`), what it breaks into, and its model, icon and sound (`MAT-21`).
    - **A thing** is one item in the world, lying in an area (`WLD-12`), with its own size, wear, quality (`MAT-20`), timers, maker, date and style (`PRE-43`); a heap of small things, such as nuts in a basket, is one thing with an amount.

- `MAT-02` **Shape and size matter** *(Decided)*: A thing's form and size count as much as its material.
  - **How it works:**
    - **Form:** lump, flake, blade, point, rod, pole, sheet, strand, powder, paste, liquid, container or structure; it sets some characteristics, so a flake has an edge and a nodule hasn't (`MAT-03`).
    - **Size** is in real units, length or amount, with weight in kilograms from the size and the weight characteristic.
    - **Blueprints ask for sizes** (`MAT-04`), such as a fist-sized core or a hut pole 2–4 m long, and size sets amounts: a bigger log burns longer, a bigger pot holds more, more poles make a bigger hut.

- `MAT-03` **Characteristics** *(Decided)*: Every item has the same 18 characteristics, each from 0 (none) to 5 (as much as any launch material has).
  They come from its material and its form: a flint nodule and a flint flake share flaking 5 but differ in edge.
  - **The 18,** with typical values:
    1. **hardness:** resists scratches and blows, and serves as a hammer: flint 5, granite 4, bone 3, oak 2, hide 1.
    2. **edge:** how well it cuts or pierces now: flint flake 5, bone splinter 3, sharpened stick 2, flint nodule 1.
    3. **toughness:** takes blows and loads without breaking: quartzite cobble 5, oak and hide 4, bone 3, flint 2, obsidian 1.
    4. **flaking:** breaks into sharp, predictable flakes when struck: obsidian and flint 5, chert 4, quartzite 2, granite 0.
    5. **flexibility:** bends and springs back: green hazel rod 5, sinew and soft hide 4, dry stick 1, stone 0.
    6. **weight:** how heavy for its size, against water: feathers 0, dry wood 1 (half), water and flesh 2 (the same), bone and clay 3 (twice), stone 4 (nearly three times), copper 5 (nine times).
    7. **burn:** how readily it catches fire: dry grass, tinder fungus and birch bark 5, dry twigs 4, dry logs 2, green wood 1.
    8. **fuel:** how much heat it gives, and for how long: charcoal 5, dry hardwood and fat 4, dry dung 3, bone 2, dry grass 1.
    9. **food:** how much it nourishes: fat 5, hazelnuts and cooked meat 4, raw meat 3, berries and raw roots 2, grass 0.
    10. **water:** how much water it holds or gives: water 5, berries 4, fresh meat and wet clay 3, green wood 2, dried meat 0.
    11. **poison:** how much harm it does eaten: deadly nightshade berries 5, rotten meat 3, raw acorns 2, most foods 0.
    12. **medicine:** how much it helps healing, eaten or put on a wound: willow bark and yarrow 3, moss 2.
    13. **warmth:** how well it keeps warmth in, worn, slept on or as a wall: fur 5, hide 3, woven grass 2, bark 1.
    14. **fibre:** how much long, strong fibre it gives for cord and sewing: sinew 5, nettle and lime bast 4, grass 2.
    15. **stickiness:** how well it glues: birch tar 5, pine resin and hide glue 4, wet clay 2, fat 1.
    16. **plasticity:** how well it takes and keeps a shape pressed or hammered into it: wet clay 5, copper 3, fat 2, dried clay 0.
    17. **waterproof:** how well it keeps water in or out: stone 5, birch bark 4, fired pot and rawhide 3, basket 1.
    18. **pigment:** how strongly it colours what it is rubbed on: red ochre 5, charcoal and yellow ochre 4, chalk and green copper ore 3.
  - **Seen or learned:** hardness, edge, flexibility, weight, water, fibre, stickiness, plasticity and pigment are known on sight or in the hand; toughness, flaking, burn, fuel, food, poison, medicine, warmth and waterproof only by use or by being told (`MND-04`).
  - **Example:** A flint nodule has hardness 5, edge 1, toughness 2, flaking 5, weight 4, waterproof 5 and 0 for the rest; a flake struck from it differs only in edge 5 and toughness 1.

- `MAT-20` **Wear and quality** *(Decided)*: Things wear with use and break; quality, how well a thing is made, comes from its maker's skill and its inputs.
  - **How it works:**
    - **Wear** runs from 0 (new) to 5 (broken).
      Each use adds the wear its blueprint sets, less for tougher things: a flake dulls after butchering about one deer, a scraper after about one hide, a cloak after about two game years, and a hammerstone lasts for years.
      Things left out wear too: hides and baskets in a season or two, wood in a few years, stone hardly at all.
      An edge falls a step for each step of wear; a broken thing becomes what its item breaks into, such as sherds; and blueprints can mend, such as retouching an edge or rebinding a haft.
    - **Quality,** 0 to 5, is set when a thing is made: half the maker's level for the try (`MAT-04`), rounded down; a step up for fine inputs (quality 4–5 on average) or down for poor ones (0–1); a step up or down by chance; and a step up if the maker is inspired (`MND-29`).
      Raw things take their quality from their source: some flint is better than other flint (`WLD-14`).
    - **What quality does:** at 0–1 a thing's main characteristic (edge for a blade, warmth for a cloak) is a step lower and it wears twice as fast; at 4–5 it is a step higher, wears half as fast, raises its maker's respect (`MND-24`) and is prized in gifts and trade (`CUL-21`).
      It shows on the thing (`PRE-42`), so a band that loses its best knapper sees its blades grow cruder (`MOM-02`).

- `MAT-09` **Nothing from nothing** *(Decided)*: Every result uses up its inputs, and nothing appears from nowhere.
  - **How it works:** a blueprint uses up what it works: a core shrinks with each flake, a hide becomes a cloak, fuel becomes ash.
    Tools are kept but wear (`MAT-20`).
    A result is never bigger or heavier than what went into it, counting water soaked up.
    Gathering takes from a plant's yields (`WLD-31`), butchering from the animal's body (`WLD-32`) and digging from the ground (`WLD-14`); timers change things in place (`MAT-19`).
  - **Check:** an automated check runs every blueprint and timer on test things and fails if a result outweighs its inputs or appears without them; whole-world runs flag any thing from nothing (`RES-12`).

### 7.2 Actions

- `MAT-06` **Base actions** *(Decided)*: People change things with 21 base actions, and every blueprint is built from them.
  Each has its animation (`PRE-44`) and its sounds by material (`SND-06`).
  - **The 21:**
    1. **gather:** pick up loose things or pluck a plant's yields, or fill a container with water.
    2. **dig:** move earth with hands, a stick or a tool, for roots, clay, flint, pits, graves or fields.
    3. **strike:** hit one thing with another: knap stone, crack nuts and bones, split wood, hammer copper.
    4. **press:** push or squeeze hard: flake an edge with a bone tip, squeeze out juice, work fat into a hide.
    5. **cut:** draw an edge through something: meat, hide, sinew, reeds, wood, notches in a tally.
    6. **scrape:** draw an edge across a surface to take a layer off: flesh from a hide, bark from a shaft.
    7. **grind:** rub one thing hard against another: grain into flour, ochre into powder, an axe to a polish, a stick along a groove until it smokes.
    8. **twist:** twist fibres or strips into cord, thread or rope.
    9. **bind:** tie or sew things together: a point to a shaft, poles into a frame, hides into a cloak.
    10. **weave:** interlace bendable strands into baskets, mats, nets, fences and fish traps.
    11. **shape:** form something soft or bendable by hand: clay into a pot, green rods into a frame.
    12. **drill:** turn a pointed stick back and forth against something: fire by friction, holes in beads and hides.
    13. **heat:** put things in, on or by a fire, or build a fire in a pit, kiln or furnace (`MAT-18`).
    14. **soak:** put things in still or running water (`MAT-19`).
    15. **dry:** lay or hang things in sun and wind or by a fire (`MAT-19`).
    16. **mix:** combine soft or loose things: pigment with fat into paint, clay with sand, resin with ash.
    17. **stack:** pile or set things into a heap or a structure: firewood, a hearth ring, a hut frame, a kiln.
    18. **plant:** put seeds, roots or cuttings into the ground on purpose (`RCK-23`).
    19. **throw:** send something through the air: stones, spears or arrows at prey, rubbish onto the heap.
    20. **feed:** give food to an animal or a person, or fuel to a fire.
    21. **apply:** put one thing onto another: paint on rock, a poultice on a wound, clay on a basket, earth on a fire.
  - **Plain uses:** gather, dig, throw, feed, stack, heat, soak, dry and apply also work without a blueprint, to take, move, wet, warm or dry things, which can set timers going (`MAT-19`); anything that makes a new kind of thing needs a blueprint (`MAT-04`).
  - **Everyday activities,** such as walking, eating, sleeping, talking, fighting and dancing, are not base actions, and no blueprint matches them (`BIO-21`).

- `MAT-12` **What a body can do** *(Decided)*: A body does base actions within its strength, hands and health, never by rules for each blueprint.
  - **How it works:**
    - **Hands:** gather, cut, throw, feed and apply need one hand, the rest two; a broken arm stops two-handed work, and a hurt hand lowers every chance of success (`BIO-13`).
    - **Strength:** heavy work, such as felling, digging pits and carrying loads, goes slower for the weak, children and the old, and some is beyond them (`BIO-16`).
    - **Children** help from about age 5 with light work, such as gathering, twisting cord and tending fires, and copy adults' work in play (`CUL-01`).
    - **State:** tiredness, cold, pain, sickness and darkness slow work and lower its chance of success (`MAT-04`).

- `MAT-11` **Simple physics** *(Decided)*: Things are carried, fall, float, burn and topple by a few simple rules from their characteristics and size, with no fracture physics and no exact paths (`SCP-21`).
  - **How it works:**
    - **Carrying:** an adult carries about a quarter of their own weight all day, more for short distances (`BIO-09`); small loose things need a container, and liquids one with waterproof 3 or more.
    - **Falling:** a dropped thing breaks if it is not tough enough for the fall: a pot (toughness 1) dropped from waist height usually breaks, a stone never; people who fall are hurt (`BIO-13`).
    - **Floating:** weight 0–1 floats, 2 floats low and 3–5 sinks (`RCK-21`), and floating things drift with the current (`WLD-17`).
    - **Throwing:** reach and harm come from the thrower's strength and the thing's weight and edge: a stone stuns at about 10 m, a thrown spear wounds at about 15 m, a dart from a spear-thrower at about 30 m and an arrow at about 40 m (tuned).
    - **Toppling:** stacks and buildings stand until strong wind, heavy snow or a quake knocks them down, poorly made ones first (`MAT-20`, `WLD-15`, `WLD-22`).
    - **Burning** follows the fire rules (`MAT-18`).

### 7.3 Fire, timers and traces

- `MAT-18` **Fire** *(Decided)*: A fire is fuel burning at a heat level from 0 to 5: 0 out, 1 embers, 2 small fire, 3 campfire (about 700 °C), 4 pit or kiln (about 900 °C), 5 furnace with forced air (over 1,100 °C, enough to melt copper).
  It burns fuel by the fuel's value, spreads to things by their burn value, and is fed or smothered.
  - **How it works:**
    - **Highest level:** an open fire reaches 3; a walled, covered pit or kiln reaches 4 with fuel 3 or more; only an enclosed fire of fuel 5 (charcoal), with air blown in through tubes or hide bellows all the time, reaches 5.
    - **Lighting:** an ember or a flame in tinder (burn 4–5) makes a small fire within a minute, and kindling, then wood, a campfire within about 10 minutes.
      Blowing raises embers or a small fire a level while someone blows (`RCK-22`); things with water 3 or more don't catch, and damp a fire they are put on.
    - **Fuel:** a campfire burns about 5 kg of dry wood an hour, a hotter fire more and better fuel less.
      Unfed, it falls to embers within about an hour, and embers die within a few hours unless banked under ash, which keeps them overnight.
    - **Spreading:** a fire of level 2 or more lights things within about a metre, sooner the higher their burn: dry grass within minutes, green wood almost never; wind carries sparks a few metres, and a hearth ring stops it creeping; across the land it spreads by `WLD-28`.
    - **Putting out:** earth, sand or water puts it out, leaving half-burnt wood as charcoal; rain lowers an open fire a level an hour, and heavy rain puts it out unless it is roofed.
    - **Effects:** it warms and lights (`BIO-11`), keeps animals off (`WLD-32`), cooks, dries, smokes and fires what is in or by it (`MAT-19`), and burns people (`BIO-13`); green wood makes thick smoke.
    - **Carrying fire:** embers in a bundle of tinder fungus or rotten wood last about a day on the move (known at the start, `BIO-20`), and a torch about an hour.
  - **Done when:** in scenes, a fed fire lasts for days, an unfed one dies within hours, a banked one lives through the night, and dry grass by a campfire catches while green wood does not.

- `MAT-19` **Timers** *(Decided)*: Slow changes run on things by themselves while their conditions hold, meant or not: rotting, drying, cooking, smoking, soaking, fermenting, setting and firing.
  - **How it works:**
    - **A timer** has a usual time, sped up, slowed or stopped by conditions (wet, heat, smoke, sealed, cold, frozen); when it ends, the thing's values change or it becomes the kind its item lists for that timer: raw meat becomes cooked, dried, smoked or rotten meat.
    - **Rotting** (flesh, fresh hide, fruit, leaves): food falls and poison rises; fresh meat rots in about 3 days in summer, about 2 weeks near freezing and never frozen; dried, smoked, salted or tanned things rot many times slower (`RCK-09`, `RCK-14`), and dead wood on damp ground over a few years.
    - **Drying** (wet things in sun and wind or by a fire): meat strips in 2–3 days, a stretched hide or a shaped pot in about 2, green poles in a season; rain and damp shade stop it.
    - **Cooking** (food at heat 2–3, or in boiling water): about an hour; food rises a step and some poisons fall (`RCK-03`); left twice as long, or at heat 4 or more, it burns to food 0.
    - **Smoking** (in thick smoke at heat 1–2): about 2 days; meat and fish then keep a season, and hides stay soft after wetting.
    - **Soaking:** crushed acorns lose their bitterness in about 2 days in running water and 6 in still (`RCK-13`), and a hide with crushed bitter bark becomes leather in about 10 days (`RCK-06`).
    - **Fermenting** (sweet fruit or grain mash with water, closed and warm): about 3 days, giving a mild drink that lifts the mood (`MND-29`); cold slows it and frost stops it (`RCK-07`).
    - **Setting:** tar and resin set as they cool, hide glue in about a day; a glued joint is weak until set.
    - **Firing:** clay at heat 3 or more for about 4 hours becomes fired clay, a step tougher and tighter if fired at 4 or more (`RCK-04`); yellow ochre at 2 for an hour turns red (`RCK-15`); flaking stone buried under a fire at 2 for half a day flakes better (`RCK-10`); limestone at 4 for half a day becomes quicklime (`RCK-05`); wood covered from the air at 3 for a day becomes charcoal; birch bark covered from the air at 2–3 for an hour gives tar (`RCK-12`); green copper ore among charcoal at 5 for an hour gives copper (`RCK-08`).
      Wet clay, or stone heated too fast, cracks.
    - **Meant or not:** a blueprint can set a timer going, such as hanging meat to dry or firing pots; its result lands when the timer ends (`TIM-17`), and the blueprint's chance decides whether it comes out well (`MAT-04`).
      Without one, a timer comes out as for someone with no skill, and meat cooked by falling into the fire, a clay hearth fired hard or a copper bead in a hot kiln can be noticed (`MND-10`, `MOM-12`).
  - **Done when:** in scenes, each timer runs in its stated time under each condition, and each firing change happens at its heat and never below it.

- `MAT-08` **Traces last** *(Decided)*: Paths, rubbish heaps, old camps, bones and graves stay in the world, are slowly buried and decay by their materials, for later people, and you, to find (`PRE-09`).
  - **How it works:**
    - **Paths:** ground walked often becomes a trodden path that people follow, and grows over after a few unused years.
    - **Rubbish heaps:** a camp's food waste, bones, ash and broken things make a heap of rich ground, where thrown seeds sprout (`RCK-23`, `MOM-08`) and wolves come to scavenge (`MOM-06`).
    - **Old camps:** hearths, hut remains, chips, tools, rubbish and graves (`CUL-19`) stay where they were left, with who left them and when; chips, bones and ash are kept as heaps, and tools, art, hearths and graves as single things.
    - **Burial:** a cave floor rises a few centimetres a game century, a river flat with each flood (`WLD-17`), a slope not at all; digging turns buried things up (`MOM-09`).
    - **What survives:** stone, fired clay and copper last; bone lasts in caves and lime-rich ground but rots in acid ground within a few centuries; wood, hide and fibre rot within a few years, except in waterlogged, frozen or very dry ground (`WLD-27`).
    - **Found again:** whoever finds an old thing can copy it (`MND-11`), and you can see the buried layers (`PRE-25`).

### 7.4 Blueprints

- `MAT-04` **Blueprints** *(Decided)*: A blueprint says: if someone does these actions, on things with these characteristics, in these conditions, with this much experience in this sector, then this happens.
  Blueprints are hidden: nobody knows one until they discover or learn it (`MND-11`), and none is picked from a menu (`SCP-04`).
  They are generic: inputs are ranges of characteristics and sizes, never named items, so one blueprint works for everything that fits (`PRN-07`).
  - **The fields:**
    1. **Named result:** the item made (`MAT-21`) and how much; its main characteristic, the one quality moves (`MAT-20`); which input each part is made of (`PRE-42`); and leftovers, such as chips or ash.
    2. **Actions:** one base action, or a sequence of up to four done as one activity (`MAT-06`).
    3. **Inputs:** each with its role (worked thing, tool, binding, fuel or container), ranges of characteristics, size and amount, and whether it is used up or kept; a kept tool takes the wear the blueprint sets (`MAT-20`).
    4. **Place:** conditions, such as near a fire of at least some heat (`MAT-18`), in water, dry or sheltered, in a pit, in smoke, or in the growing season.
    5. **Sector and difficulty:** one of the 15 sectors (`MND-06`), and a difficulty from 1 to 10.
    6. **Time:** how long the activity takes, shorter with better tools (tuned), and any timer it starts (`MAT-19`).
    7. **Chance:** from the difficulty and the maker's level (below).
    8. **Failures:** what failed tries give, and how often: lost time, spoiled inputs (shattered, burnt, cracked, torn), a poor result (quality 0) or a hurt (`BIO-13`); and which failures hint at the blueprint, such as smoke but no ember (`MND-11`).
  - **How it works:**
    - **Level and chance:** a try's level is the average of the maker's skill in the blueprint and experience in its sector (`MND-06`), taught or not (`MND-13`).
      At a level equal to the difficulty, one try in two succeeds; each level above adds a tenth and each below takes a tenth, within 5% and 95%.
      Fine inputs (quality 4–5) add a tenth and poor ones (0–1) take a tenth; tiredness, pain, cold and darkness take more (`MAT-12`, tuned).
    - **Unknown blueprints:** when an action ends, any blueprint it fits that the person doesn't know has a small chance of working anyway: smallest by accident, bigger when experimenting, bigger again with a hunch (`MND-11`), all tuned for pace (`PRN-17`).
    - **Results land** when the activity, or the timer it started, ends (`TIM-17`); an interrupted try keeps the work done.
  - **Example: sharp flake by striking,** the first blueprint in most worlds:
    - **Named result:** one flake, 3–8 cm, of the core's stone, with its hardness and flaking, an edge equal to its flaking (5 from flint) and toughness 1; main characteristic edge; leftovers, chips (`MAT-08`).
    - **Action:** strike.
    - **Inputs:** a core, worked and partly used up: hardness 4–5, flaking 3–5, 8–30 cm; and a striker, kept: hardness 3–5, toughness 3–5, 6–12 cm.
      Either may be the one that moves, so a flint cobble that hits a stone anvil can lose a flake too.
    - **Place:** anywhere.
    - **Sector and difficulty:** stone, 2.
    - **Time:** about half a minute a try; a core gives up to about 10 flakes.
    - **Chance:** 30% at level 0, 50% at level 2, 90% at level 6.
    - **Failures:** most knock off only crumbs, one in four shatters the core into chunks, and one in twenty cuts the holding hand.
    - **Discovery:** nobody knows it at the start (`BIO-20`), but cracking nuts or bones with a flint cobble fits it, so a flake can come off by accident (`MND-11`).
  - **Done when:** every launch blueprint has every field, and its scene shows it succeeding about as often, and taking about as long, as its fields say (`RES-23`).

- `MAT-07` **Several routes** *(Decided)*: The same named result can come from different materials, since blueprints are generic, or from different blueprints, so peoples reach the same things by different paths.
  - **Examples:** flakes of flint, chert or obsidian; huts of poles and hides or of poles and reeds; an ember by drilling or by ploughing a stick along a groove; boiled food from hot stones dropped into a hide-lined pit or from a pot on the fire; glue from birch tar or from resin mixed with ash; copper hammered from native copper or smelted from green ore (`RCK-08`).
  - **How it works:** routes differ in what they need and how they look (`PRE-42`): a reed hut is as dry as a hide one but colder, and obsidian is sharper than flint but breaks sooner.
    Which route a people finds first depends on what lies around it (`WLD-14`) and on chance, so histories differ.

- `MAT-22` **Chains** *(Decided)*: Results feed other blueprints, so most things take a chain of steps, each a blueprint discovered or learned on its own.
  A people can stall at any step, or get round it by another route (`MAT-07`).
  - **Example: from hide to clothing:**
    1. **Sharp flake:** strike; stone, difficulty 2 (`MAT-04`).
    2. **Scraper:** press small chips off a flake's edge with bone, antler or soft stone (hardness 2–4), leaving a steep, strong edge (edge 3, toughness 3); stone, difficulty 3, about 5 minutes.
    3. **Fresh hide:** cut the skin from a carcass with an edge of 2 or more; hunting, difficulty 2, about an hour for a deer.
    4. **Scraped hide:** scrape flesh and fat off a fresh hide (warmth 2–5, toughness 3–5, water 2–5) with an edge of 2–4 and toughness 3 or more; hides, difficulty 2, about 3 hours, within about 2 days of the kill, before it rots (`MAT-19`).
    5. **Dried hide:** stretch it on stakes or a frame in sun and wind (dry); hides, difficulty 1, about 2 days; stiff (flexibility 1), but it no longer rots.
    6. **Soft hide:** work fat into it, then pull and rub it supple (apply, press); hides, difficulty 3, about 3 hours; flexibility 4.
    7. **Awl and thread:** grind a bone splinter to a point on sandstone (grind; hides, difficulty 2, about an hour), and twist dried sinew into thread (twist; hides, difficulty 2).
    8. **Sewn cloak:** cut the soft hide to shape, pierce holes along its edges with the awl and sew them with the thread (cut, drill, bind); hides, difficulty 4, about a day; warmth 3, or 5 with the fur left on (`RCK-26`).
  - **Shorter routes:** a dried hide tied on with a thong is clothing after step 5, warm but stiff and quick to wear out; a cape of woven grass needs no hide, but keeps far less warmth.

- `MAT-21` **Named discoveries** *(Decided)*: Every named result has a name, a prepared model, an icon and a sound.
  A people's first success at making one is a named discovery, named in their language and written in the book of ages with who made it.
  - **How it works:**
    - **Names:** an English name for you, such as sharp flake, and each people's own word, coined when it first makes it and shown with its meaning (`CUL-18`).
    - **Model, icon and sound:** one prepared model per named result, whose parts take the materials used (`PRE-42`), an icon drawn from it, and a sound blueprint for its action and materials (`SND-06`).
    - **The entry:** who, when, where, by which route (accident, experiment, dream or copying, `MND-11`), from what, and the new word (`PRE-05`); a result a timer gives by accident counts too, such as copper from a kiln (`MOM-12`).
    - **Size of entry:** the steps of the arc (`TIM-19`) and firsts in the whole world are major entries and live moments (`PRE-08`); a people's first of something others already make is short; a lost craft and its return are marked (`CUL-02`).
  - **Example:** "Year 3, summer, day 9: Ama of the Hazel band strikes the first sharp flake, which her people name kel-tam, stone that bites."

- `MAT-23` **The launch blueprints** *(Decided)*: About 150 blueprints cover the arc from caves to first copper, across the 15 sectors (`MND-06`); a blueprint's sector follows its purpose.
  - **Known at the start** (`BIO-20`): crack nuts and bones with a stone, butcher a carcass with a broken stone or bare hands (slowly, wasting much), make a bed of grass or leaves, bank a fire under ash, and carry embers.
  - **By sector,** with some of their named results:
    - **stone (about 12):** sharp flake, blade, scraper, borer, hand axe, spear point, arrowhead, heat-treated stone, polished axe, grinding stone;
    - **wood (about 12):** sharpened stick, digging stick, fire-hardened spear, club, shaft, handle, throwing stick, spear-thrower, bow, wooden bowl;
    - **fire (about 8):** ember by drilling, ember by ploughing, tinder bundle, hearth ring, torch, fat lamp, charcoal;
    - **cooking (about 15):** roast meat and roots, stone-boiled food, pot-boiled food, dried meat, smoked fish, salted meat, rendered fat, leached acorns, flour, flatbread, porridge, fermented drink;
    - **hunting (about 10):** butchered carcass, hafted spear, darts, arrows, snare, net, fish trap, fish spear, bone hook;
    - **gathering (about 8):** basket, net bag, stripped fibre, cut reeds, harvested wild grain, birch sap;
    - **hides (about 15):** scraped hide, dried hide, soft hide, smoked hide, leather, thong, sinew thread, awl, eyed needle, hide wrap, sewn cloak, tunic and leggings, shoes, hide bag, fur blanket;
    - **building (about 14):** windbreak, lean-to, pole hut, reed hut, pit house, post house with daub, lime plaster, storage pit, drying rack, fence, kiln, furnace;
    - **healing (about 8):** moss dressing, yarrow poultice, willow-bark drink, splint, washed wound, fat salve for burns;
    - **pottery (about 8):** tempered clay, shaped pot, fired pot, kiln-fired pot, clay figure, clay-lined basket, clay lamp;
    - **herding (about 4):** tether, pen, winter hay;
    - **farming (about 8):** cleared plot, dug plot, sown plot, planted roots or cuttings, weeded plot, sickle, threshed grain, seed store;
    - **metal (about 5):** hammered copper, smelted copper, copper awl, copper bead, copper axe;
    - **art (about 10):** ochre powder, paint, rock painting, body paint, engraving, carved figure, bead, pendant, tally stick;
    - **music (about 5):** bone flute, hide drum, rattle, clappers, bullroarer.

### 7.5 Values and catalogues

- `MAT-05` **Plausible values** *(Decided)*: Every value here, such as a characteristic, a size, a time or a chance, is a plausible estimate, set by hand from the real thing and tuned so the game feels right (`PRN-05`).
  - **How it works:** values keep the real order of things, such as flint harder than bone, fur warmer than hide and a furnace hotter than a campfire; the orders that decide what is possible are fixed by the reality rules (7.6), and tuned values are logged with what they were tuned against (`RES-16`).
  - **Check:** the catalogue checks hold every value to the orders the reality rules fix (`MAT-17`).

- `MAT-13` **The catalogues** *(Decided)*: The game's content is data in catalogues, written by AI agents and checked by automated tests: items (about 200, `MAT-10`), blueprints (about 150, `MAT-23`), plants (about 60, `WLD-31`), animals (about 30, `WLD-32`) and illnesses (about 15, `BIO-05`).
  Other sections keep their prepared lists the same way, such as belief templates, art motifs and dance moves (`CUL-07`, `CUL-09`, `CUL-10`).
  - **How it works:** each entry stands alone, in plain words, with all its values and the checks it supports (`MAT-17`); entries name others only as results, such as a blueprint's named result or an animal's yields, never as inputs (`PRN-07`); and every rule in them is physical or biological, never depending on beliefs (`SCP-19`).
  - **Check:** the catalogue checks find no entry that names another as an input.

- `MAT-14` **Adding without rewriting** *(Decided)*: Adding an item, blueprint, plant, animal or illness never needs the others changed (`PRN-14`).
  - **How it works:** a new item takes part in every blueprint its characteristics fit, and a new blueprint works on every item in its ranges.
  - **Example:** Adding jasper, a stone with flaking 4, gives jasper flakes, scrapers and points at once.
  - **Check:** a made-up item added for testing works in every blueprint it fits, with no other entry changed.

- `MAT-15` **Every addition proves itself** *(Decided)*: Each new or changed entry comes with its checks, and all the catalogue checks run again before the change joins the main version (`PRC-10`); a failing check blocks it.
  - **Check:** the review confirms that every new entry names its checks (`PRC-09`).

- `MAT-16` **The catalogue grows by milestone** *(Decided)*: The catalogues grow with the milestones (`SCP-16`), each adding only what its steps need, without rewriting earlier entries:
  - **first camp** (`MIL-01`): only the wild foods and water the band lives on;
  - **sharp stone** (`MIL-02`): the start region's stones and wood, the blueprints known at the start (`BIO-20`), and flaking, cutting and scraping;
  - **fire** (`MIL-03`): fuels, heat levels, fire-making, cooking, drying and smoking;
  - **a living world** (`MIL-04`): every plant's and animal's yields, hunting weapons, hides, cord, clothing, huts and healing;
  - **herds, fields and villages** (`MIL-07`): pottery, herding, farming, houses and stores, copper, art and instruments, completing the launch catalogue;
  - **later** layers, such as bronze or writing, the same way (`VIS-03`).

- `MAT-17` **How the catalogue checks work** *(Decided)*: Automated checks keep the catalogues complete and believable, and run on every change (`MAT-15`).
  - **How it works:**
    - **Complete:** every item has its 18 values, size, class, model, icon and sound, and every blueprint every field of `MAT-04`, with inputs as ranges, never named items (`PRN-07`).
    - **Reachable from the start:** following blueprints and timers from what the start region holds and what the bands know (`BIO-20`), every named result can be made by at least one route, using only things the world has (`WLD-14`), and no chain needs its own result first, such as a copper tool to make the first copper.
    - **Possible:** each heat a blueprint needs can be reached with a fuel and a setting the catalogue has (`MAT-18`), and every input size exists.
    - **Reality rules:** each rule in 7.6 is checked over the catalogue, by matching every blueprint and timer against every item, and, where chance matters, in a sandbox scene run about 20 times (`RES-13`, `RES-23`).
    - **Matches nobody planned:** each change lists the new pairings of items and blueprints it makes possible, for the review to look at, so an axe of bark or a pot of sand is caught (`RSK-06`).
    - **Pace** is checked on whole worlds by the pace tests (`RES-07`).
  - **Check:** no change joins the main version without these checks passing (`PRC-10`).

### 7.6 Reality rules

Short rules on the catalogues that must always hold, each with its automated check (`MAT-17`).
They keep results believable (`RSK-06`) and fix the orders that decide what is possible (`MAT-05`).

**Stone, fire and food**

- `RCK-01` **Flint flakes, granite doesn't** *(Decided)*: Stones that flake, such as flint, chert and obsidian, give sharp flakes; coarse stones, such as granite, sandstone and limestone, never do.
  - **Check:** every coarse stone has flaking 0–1, and only items with flaking 3 or more fit the flake, blade and point blueprints; without stone that flakes, the sharp-stone test never makes a flake (`RES-03`).

- `RCK-02` **Fire by friction** *(Decided)*: Dry wood, drilled or ploughed hard enough, gives an ember; green or wet wood never does, and wet tinder never catches.
  - **Check:** the fire-making blueprints accept only wood and tinder with water 0–1; in a scene, someone who knows fire by drilling makes an ember within about 5 minutes in most tries with dry wood, and never with green wood.

- `RCK-22` **Air feeds fire** *(Decided)*: Blowing on embers makes them flare; smothering puts a fire out.
  - **Check:** in a scene, blowing on embers makes a small fire within a minute in most tries, and earth on a campfire puts it out (`MAT-18`).

- `RCK-08` **Copper needs a furnace** *(Decided)*: Green copper ore gives copper only at heat 5, in an enclosed charcoal fire with forced air, never in a campfire or a kiln; native copper can be hammered cold.
  - **Check:** every route to copper starts from green ore at heat 5 or from native copper, and heat 5 needs fuel 5 and forced air (`MAT-18`); in a scene, ore never gives copper in a campfire or a kiln, and does in a furnace in most tries.

- `RCK-03` **Cooking helps** *(Decided)*: Cooked meat, roots and grain nourish more than raw, and cooking lowers some poisons.
  - **Check:** every cooked meat, root and grain has food a step above its raw kind, and no cooked kind has more poison than its raw kind.

- `RCK-09` **Rot** *(Decided)*: Meat, fish and fresh hides rot within days when warm, far slower when cold or dry, and not at all when frozen.
  - **Check:** every flesh and fresh hide item rots at the times in `MAT-19`, and nothing rots frozen.

- `RCK-14` **Keeping food** *(Decided)*: Drying, smoking and salting make meat and fish keep for a season or more, and dry grain and nuts keep for a year in a dry store.
  - **Check:** dried, smoked and salted meat rots at least 10 times slower than fresh, and dry grain and nuts keep a year in a dry store (`MAT-19`).

- `RCK-13` **Leaching** *(Decided)*: Soaking crushed acorns in running water draws out their bitterness.
  - **Check:** crushed acorns go from poison 2 to 0 at the times in `MAT-19`, and whole ones hardly change.

- `RCK-07` **Fermenting** *(Decided)*: Crushed sweet fruit or grain mash, kept warm and closed, ferments in a few days; cold slows it, frost stops it, and dry things never ferment.
  - **Check:** only things with food 2 or more and water 3 or more ferment, and never frozen (`MAT-19`).

- `RCK-21` **Floating** *(Decided)*: Dry wood floats; stone sinks.
  - **Check:** every dry wood, bark, reed and charcoal item has weight 0–1, and every stone, earth and metal item 3 or more (`MAT-11`).

**Crafts**

- `RCK-10` **Heat-treated stone** *(Decided)*: Flaking stone buried under a fire and cooled slowly flakes better; put straight into the flames, it cracks.
  - **Check:** every flaking stone's treated kind has flaking a step higher, or quality a step higher if its flaking is already 5; in a scene, stone put into a campfire cracks in most tries.

- `RCK-11` **Cord** *(Decided)*: Fibres twisted into cord are far stronger than the loose fibres, and only fibrous things make cord.
  - **Check:** only items with fibre 3 or more fit the cord blueprints, and every cord is at least two steps tougher than its loose fibres.

- `RCK-12` **Glue from bark** *(Decided)*: Birch bark heated without air gives a tar that glues a point to a shaft; in an open fire it only burns.
  - **Check:** tar comes only from birch bark covered from the air at heat 2–3 (`MAT-19`); in a scene, birch bark in an open fire leaves only ash.

- `RCK-25` **Hafting** *(Decided)*: A head bound to a shaft or handle with cord or sinew, or glued with tar, gives the head's edge with the shaft's reach, so a flint-tipped spear wounds deeper than a sharpened stick.
  - **Check:** every hafted result takes its edge from its head and its toughness from the weakest of head, binding and shaft, and hafting accepts only bindings with fibre 3 or more or glues with stickiness 4 or more.

- `RCK-04` **Pottery needs fire** *(Decided)*: Shaped clay dried in the sun softens again in water; only clay fired at heat 3 or more stays hard and holds water.
  - **Check:** every unfired clay thing has waterproof 0 and turns back to wet clay when soaked, every fired one has waterproof 3 or more, and no route makes a waterproof clay thing without heat 3 (`MAT-19`).

- `RCK-05` **Lime** *(Decided)*: Limestone burned at heat 4 or more becomes quicklime, which mixed with water sets into white plaster; below heat 4 nothing changes.
  - **Check:** quicklime comes only from limestone fired at heat 4 or more (`MAT-19`).

- `RCK-06` **Leather** *(Decided)*: Scraped hides soaked for about 10 days with crushed bark that is bitter and stains, such as oak, become leather that stays supple and doesn't rot; hides in plain water rot.
  - **Check:** the tanning blueprint accepts only barks with poison 2–3 and pigment 2 or more, and its leather doesn't rot; in a scene, a hide in plain water rots on time (`MAT-19`).

- `RCK-26` **Warmth from the material** *(Decided)*: Clothes, bedding and shelters keep warmth by what they are made of: fur most, then hide and leather, then woven grass, reeds and bark; wet, they keep far less.
  - **Check:** fur has warmth 5, hide and leather 3, and woven plant things 1–2; every made thing takes the warmth of its main material, halved when wet (`BIO-11`).

**Colour and art**

- `RCK-15` **Ochre turns red** *(Decided)*: Yellow ochre heated at heat 2 or more turns red; below that, it stays yellow.
  - **Check:** yellow ochre's firing result is red ochre at heat 2 or more, and nothing else turns it red (`MAT-19`).

- `RCK-16` **Paint that lasts** *(Decided)*: Charcoal or ochre mixed with fat or water and put on rock lasts for centuries where sheltered; on open rock, rain wears it away within years.
  - **Check:** a painting in a cave or under an overhang wears at most a step a game century, and one on open rock in the rain at least a step every few game years (`MAT-20`).

**Growing and taming**

- `RCK-23` **Seeds grow** *(Decided)*: Seeds in fertile, moist ground in the growing season sprout; planted and tended ones grow better, which is farming.
  - **How it works:** a handful of seed thrown on rich, damp ground, such as a rubbish heap, gives a few plants the next season (`MOM-08`); dug in and covered, most seeds sprout; weeded and watered, a plot yields about twice as much as one left alone (tuned); roots and cuttings planted the same way grow too.
  - **Check:** every food plant's seeds, roots or cuttings list their growing season and ground (`WLD-31`); in a scene, seeds thrown on a heap sprout in some runs, and a tended plot yields at least twice an untended one.

- `RCK-24` **Young animals grow tame** *(Decided)*: Young animals raised and fed by people grow tame; adults rarely do.
  - **Check:** in a scene, wolf pups fed daily by people from their first days reach tameness 5 within a season (`WLD-33`), while grown wolves fed for a season stay at 2 or below in most runs.

**Beyond the launch arc**

- `RCK-17` **Bronze** *(Dropped)*
  - **Dropped because:** beyond the launch arc, which ends at first copper (`VIS-03`).

- `RCK-18` **Iron** *(Dropped)*
  - **Dropped because:** beyond the launch arc, which ends at first copper (`VIS-03`).

- `RCK-19` **Mortar** *(Dropped)*
  - **Dropped because:** beyond the launch arc, which ends at first copper (`VIS-03`).

- `RCK-20` **Glass** *(Dropped)*
  - **Dropped because:** beyond the launch arc, which ends at first copper (`VIS-03`).

## 8. People: bodies and lives

Every person has a body that must be fed, watered, kept warm and rested; that can be hurt, fall ill and heal; and that grows, ages, has children and dies.
The rules are simple and the numbers plausible (`PRN-05`), so bodies look and behave believably without simulating biology (`PRN-02`, `SCP-21`).
Ages are in game years of 60 days (`TIM-18`).
How people think is in Minds.

### 8.1 Who they are

- `BIO-01` **Modern humans** *(Decided)*: The people are one human species, with bodies and minds as able as ours (`SCP-05`).
  Only their culture starts almost empty (`SCP-01`): what they can do grows from what they discover and pass on, never from a limit built into them.

- `BIO-02` **Starting kit** *(Decided)*: The first people have almost no culture, but they share one language and know a few blueprints and their home range.
  - **Language:** the world's language, from the start (`CUL-17`).
  - **Fire:** they can feed a fire, bank it under ash and carry its embers, but cannot make one (`MAT-18`).
    About one band in three (tuned) starts with a fire taken from lightning, and the others must find one (`WLD-28`).
  - **Tools:** none shaped; unshaped stones for bashing and sticks for digging.
  - **Clothing:** none.
  - **Shelter:** caves and overhangs, with beds of grass or leaves.
  - **Food:** gathering, scavenging, and small game taken with thrown stones and sticks.
  - **Knowledge and beliefs:** a few blueprints, their home range and which local things are food or poison (`BIO-20`); no beliefs about spirits or hidden causes.
  - **Done when:** every new world's bands start with exactly this kit, and the sharp-stone test runs from it (`RES-02`).

- `BIO-03` **Starting population** *(Decided)*: 3–4 family bands of 15–30 people, 45–120 in all, in one start region (`WLD-24`).
  Together they are one people, with one language and a name for themselves (`CUL-17`, `CUL-23`).
  - **How it works:**
    - **Families:** each band is a few related families of believable ages, with some orphans and widowed people, and kin in the other bands (`CUL-27`, `CUL-30`).
      They come from a few generations of births, pairings and deaths, run by this section's rules with bodies only, before history begins (`TIM-14`).
    - **Ages:** about two in five are children under 14, a few are over 45, and men and women are about equal in number (tuned).
    - **Bodies** are grown to their ages from their inherited traits (`BIO-06`), with old scars and healed injuries.
    - **Places:** each band has its own shelter and home range (`WLD-24`).
  - **Done when:** every start has 3–4 bands of 15–30 people, every child has a living parent or kin to care for it, and every mother was about 15–45 at each child's birth.

- `BIO-20` **Starting knowledge** *(Decided)*: Each adult starts knowing a few blueprints, their home range, which local things are food or poison, and their kin.
  Every starting fact is true, so every mistake comes from play.
  - **How it works:**
    - **Blueprints:** the five foragers everywhere know (`MAT-23`): cracking nuts and bones with a stone, butchering a carcass with a broken stone or bare hands, making a bed of grass or leaves, banking a fire under ash, and carrying embers in a bundle of tinder fungus or rotten wood.
      Adults know them at a skill set by age, and children learn them as they grow (`MND-06`).
    - **Experience:** adults have some in gathering, hunting and fire, more with age, and none in the other sectors; children have less (`MND-06`).
    - **Mental map:** the places within about 10 km of their shelter (`WLD-24`), with water, shelter, food by season, stone, routes and dangers, and where the other bands live (`MND-28`); older adults know more, and children a share by age.
    - **Things:** which common local plants and animals are food, poison or dangerous (`MND-04`).
    - **People:** their band and kin, with a few friendships and grudges (`MND-24`).
    - **Memories:** none from before history begins (`MND-18`).
  - **Done when:** in a new world every adult knows exactly this, and nothing more (`MND-02`).

- `BIO-08` **Everyone is different** *(Decided)*: Each person has their own height, build, strength, stamina, resistance, sight, hearing and learning speed.
  - **How it works:**
    - **Spread:** each is drawn around the human average, most people within about a fifth of it (tuned), from inheritance (`BIO-06`), sex (`BIO-17`), age (`BIO-16`) and chance.
      Long hunger or serious illness in childhood leaves people shorter and weaker for life.
    - **What each does:** height and build set size, food needs and looks; strength sets loads, blows and heavy work; stamina, how long people walk, run and work; resistance, how well the body fights illness and infection and how fast it heals (`BIO-05`, `BIO-13`); sight and hearing, what reaches the mind (`BIO-18`); and learning speed, how fast skill grows (`MND-06`).
    - **Shown** in words on the card, such as "tall, strong, sharp-eyed" (`PRE-35`).
  - **Done when:** each number's spread in a starting population matches the stated range, and children who went hungry for long grow up shorter and weaker on average.

### 8.2 Staying alive

- `BIO-09` **Needs of the body** *(Decided)*: Hunger, thirst, warmth and rest, each felt by the mind as a need (`MND-07`), with real effects on the body when unmet.
  - **How it works:**
    - **Hunger:** the body uses about a day's food a day (`BIO-10`), more for hard work, heavy loads (`MAT-11`), cold, growth, pregnancy and nursing; the belly holds at most a day and a half's food.
    - **Condition:** the body's reserve, from 0 (starved) to 100 (well padded), seen as thin or stout (`PRE-27`); it rises when people eat more than they use and falls when they eat less.
      Below about 30, people are weaker, heal and fight illness worse, and women stop conceiving (`BIO-15`); at 0 they die (`BIO-14`), which takes a well-fed adult about 20 days without food (tuned).
    - **Thirst:** an adult needs about 3 litres of water a day, up to twice that in heat or hard work (tuned), from drink and food (`BIO-10`); without it, people weaken within a day, grow confused within two and die in about three, and salt water makes it worse (`WLD-26`).
    - **Warmth:** see `BIO-11`.
    - **Rest:** adults need about 8 hours' sleep a day, children about 10 and babies most of the day (tuned); hard work tires faster, and cold, hunger or pain spoil sleep.
      Tired people work slower and fail more often (`MAT-04`), and after about a day and a half awake they fall asleep where they are.
    - **Out of breath:** running, fighting and heavy work tire people within minutes, and a few minutes' rest restores them (`BIO-08`).
  - **Done when:** in test scenes, a person without food dies in about the stated time, one without water in about three days, and a band with enough food, water and shelter keeps its condition through a mild year.

- `BIO-10` **Food** *(Decided)*: A thing feeds by its food value; varied food keeps people strong, and one kind of food for long makes them weak.
  - **How it works:**
    - **Food value:** each step up the food characteristic (`MAT-03`) about doubles what a kilogram gives, so a day's food for an adult is about 4 kg at food 2 (berries, raw roots), 2 kg at 3 (raw meat), 1 kg at 4 (nuts, cooked meat) or half a kilogram at 5 (fat) (tuned).
      Cooking raises food a step (`RCK-03`), and a thing's water characteristic counts toward thirst.
    - **Four food groups:** meat and fish (with eggs and milk), fruit and greens (with berries, shoots and mushrooms), nuts and seeds (with grain), and roots; the catalogue puts each food in one (`MAT-13`).
    - **Variety:** someone who has eaten from only one group over the last 10 days (tuned) grows weak: strength, stamina, healing and resistance drop by about a fifth until they eat more widely.
    - **Scurvy:** after about 15 days without fresh fruit, berries, greens or shoots, gums bleed and wounds stop healing, until a few days of them cure it.
  - **Example:** A band that winters on dried meat and nuts gets bleeding gums every spring, until someone notices that those who ate the first green shoots got better (`MND-05`).
  - **Done when:** in test scenes, people living on one food group for 10 days grow weak, people on dried food for 15 days get scurvy and recover on fresh greens, and cooked meat feeds more than raw.

- `BIO-11` **Heat and cold** *(Decided)*: Bodies keep warm by work, clothing, shelter, fire and each other; cold can kill, and heat exhausts.
  - **How it works:**
    - **Comfort:** felt temperature is the air's (`WLD-16`), colder in wind or when wet and warmer in sun or shelter; naked and at rest, people are comfortable down to about 24 °C of it.
      Each point of warmth in what they wear (`MAT-03`), counted by how much of the body it covers, lowers that limit by about 6 °C, and work lowers it by about 10 °C more (tuned); wet clothing keeps only half its warmth (`RCK-26`).
    - **Shelter, bedding and fire:** a shelter cuts wind and rain and is warmer inside by the warmth of its walls, a cave by a few degrees (tuned); bedding counts like clothing for sleepers, and huddling as one point of warmth; and a campfire warms those within about 2 m by about 15 °C (`MAT-18`).
    - **Cold:** below the limit people shiver; about 10 °C below, they grow clumsy and slow; about 20 °C below, they freeze, grow confused and fall asleep, and die within a few hours (tuned).
      Hours of freezing on bare arms, legs or head bring frostbite, a burn from cold (`BIO-13`), and children and the old chill faster.
    - **Heat:** above about 32 °C felt, people sweat and need more water (`BIO-09`), and hard work in great heat brings fainting and heatstroke, which can kill.
  - **Done when:** in winter test scenes, a naked band huddled in a cave lives through the night, a lone person outside on a freezing night without fire or cover gets frostbite or dies, and people in warm clothing work outdoors all day.

- `BIO-12` **Poison and medicine** *(Decided)*: Poison and medicine come from the characteristics of what is eaten or put on the body (`MAT-03`), and some things do both, by how much is taken.
  - **How it works:**
    - **Poison** acts within hours, by its value and the amount eaten against the eater's size: 1–2 brings cramps and vomiting for a day; 3 makes people ill for days and can kill a child; 4 can kill an adult who eats a meal of it; 5 can kill anyone who eats a mouthful (tuned).
      It runs like a short illness (`BIO-05`), worst for children, the old and the weak.
    - **Medicine,** eaten or put on a wound, eases pain by its value for some hours and helps the body fight illness and infection (`BIO-05`, `BIO-13`); strong medicine (3 or more) taken for a few days clears worms.
      A thing with both helps in small amounts and harms in large ones, as willow bark eases pain but upsets the gut when chewed to excess.
    - **Other sources:** a venomous snake's bite poisons by its kind (`WLD-32`), and food grows poisonous as it rots (`MAT-19`).
    - **Making food safe:** leaching, cooking and drying can lower poison, as each blueprint's result says, so leached acorns lose their bitterness (`RCK-13`).
    - **Learned by use:** both are hidden until tried (`MND-04`), and many poisons taste bitter, though not all (`MND-21`).
  - **Done when:** in test scenes, a mouthful of something with poison 5 kills an adult in most runs, poison-2 berries make the eater ill for a day, and wounds treated with medicine hurt less and get infected less often.

### 8.3 Harm and healing

- `BIO-13` **Body parts and wounds** *(Decided)*: Every body has six parts, head, torso, two arms and two legs (hands and feet count with their limbs), each with its own health.
  Wounds bleed, can get infected and heal over days; a broken leg slows, a broken arm stops two-handed work, and a bad head or torso wound can kill.
  - **How it works:**
    - **Health:** each part runs from 100 (sound) to 0, and each wound takes its size from its part until it heals.
    - **Five kinds of wound:** cuts (edges and points), bruises (blows and falls), bites (teeth, horns and claws), burns (fire, hot things and freezing cold) and broken bones (a blow or fall of more than about 25 on an arm or leg).
    - **Size** comes from the cause: the striker's strength and the weapon's weight, hardness or edge (`MAT-03`), the animal's kind (`WLD-32`), the heat (`MAT-18`) or the height of a fall (`MAT-11`); a knapping slip gives about 5, a club or a dog bite about 20, and a spear thrust or a bear's swipe 40 or more (tuned).
      Blows land by chance, most on the torso and least on the head, unless the event decides (a fall lands on the legs).
    - **Bleeding:** cuts and bites bleed by size, small ones stopping within minutes and big ones only when pressed or dressed (`BIO-23`).
      Blood counts from 100: losing about 25 weakens, 35 makes people collapse, and 50 kills; it comes back at about 10 a day (tuned).
    - **Effects:** pain grows with every wound, slowing work and lowering mood (`MND-29`).
      A broken leg allows only a hobble at a quarter speed, and a broken arm stops two-handed work (`MAT-12`); lesser wounds slow walking, or slow work and lower its chance of success.
      Below half health a hurt head dazes and a hurt torso weakens, below a quarter a hurt head knocks its owner out, and either at 0 kills (`BIO-14`); an arm or leg at 0 is useless for life.
    - **Infection:** a cut, bite or burn can get infected in its first two days, more if big or dirty and less if washed or dressed (`BIO-23`) or in a strong body (`BIO-08`); it then stops healing until the body clears it, and may spread into wound fever (`BIO-05`).
    - **Healing,** squeezed like the year (`TIM-18`): bruises and small cuts in 1–3 days, deep cuts, bites and burns in 3–6, and broken bones in about 10; faster with rest, food and warmth, and slower with age, hunger and cold (tuned).
    - **Lasting harm:** big wounds scar, and a broken bone not kept still may heal crooked, leaving a limp or a weak arm for life; all of it shows on the figure, in how it moves and on the card (`PRE-27`, `PRE-44`, `PRE-35`).
  - **Example:** A hunter with a broken leg lives through the winter because the band carries and feeds him; he limps ever after and never hunts again, but becomes the best knapper in the valley.
  - **Done when:** in test scenes, a deep cut left alone bleeds to death while a pressed or dressed one doesn't, a broken leg hobbles its owner for about 10 days, dirty wounds get infected more often than washed ones, and a head or torso at 0 kills.

- `BIO-05` **Illness** *(Decided)*: About 15 illnesses, each with its routes, a time before it shows, a course, a danger and, for some, immunity afterwards.
  Some spread only where many people live close together, as in real history.
  - **How it works:**
    - **Catching:** breath illnesses pass to people sharing a shelter or hearth with the sick; touch illnesses to those who tend them or share their bed or food; others come through fouled water (`WLD-17`), raw meat and fish, wounds (`BIO-13`), or sick animals' bites, carcasses and milk (`WLD-32`).
      Each contact has the illness's own daily chance (tuned), from a day before the signs show until recovery, and the immune don't catch it.
    - **Where it starts:** everyday illnesses start now and then in any group, each at its own rate per season (tuned), breath illnesses most in winter; animal ones come from the few sick animals of the kinds that carry them.
    - **Crowd illnesses** start only in villages of at least about 200 people that keep herds, at a small chance each year (tuned), and travel with visits, trade, marriages and raids (`CUL-28`); in a band they burn out once all have had them, while villages, with new babies always coming, keep them.
    - **The fight:** once it shows, an illness grows each day by its strength while the body fights back by its resistance (`BIO-08`), weaker in babies, the old, the hungry, the cold, the tired and the wounded, and stronger with rest, food, water, warmth, care and medicine (`BIO-23`, `BIO-12`).
      If the illness peaks first, a deadly one kills (`BIO-14`), and strengths are tuned so each kills about its stated share.
    - **Courses** of days run as in life, and longer ones are squeezed like the year (`TIM-18`); the sick look and act ill, and others notice (`MND-03`) and may keep away (`MND-05`).
  - **The illnesses** (routes; days before the signs show), each a catalogue entry (`MAT-13`); deaths are among untreated healthy adults, and babies, the old and the hungry fare worse:
    - **Cold** (breath, touch; 1–2 days): sniffles and cough for about 5 days; harmless itself, but it can turn to chest fever in the weak; immunity for a year.
    - **Coughing fever** (breath, touch; worst in winter; 1–3 days): fever, aches and cough for about a week; kills 1 in 50, and 1 in 8 babies and old people; immunity for a few years.
    - **Chest fever** (not catching; follows a cold, a coughing fever, freezing or near-drowning): hard breathing and high fever for 7–10 days; kills 1 in 4.
    - **Gut sickness** (fouled water or food; within a day): cramps and the runs for 2–5 days; kills few adults but about 1 in 20 babies and old people, through lost water that drinking replaces.
    - **Worms** (raw or undercooked meat and fish): slow thinning and weakness until strong medicine clears them (`BIO-12`); cooking prevents them.
    - **Wound fever** (an infected wound, or a birth; 1–3 days): fever and spreading redness for 5–10 days; kills 1 in 3.
    - **Lockjaw** (deep wounds soiled by earth or dung; 3–15 days): a jaw and body that stiffen for 10–20 days; kills 1 in 2.
    - **Foaming madness** (the bite of a mad wolf, dog or other meat-eater; 5–20 days): rage, fear of water and death within days, always; mad animals lose their fear of people.
    - **Hunter's fever** (skinning or eating sick hares and other small game; 2–5 days): fever and sores for about 2 weeks; kills 1 in 20; immunity for life.
    - **Herder's fever** (kept goats, sheep and cattle, through raw milk and helping births; 5–15 days): fevers that come and go for 15–30 days; rarely kills; immunity for life.
    - **Sore eyes** (touch; worst in crowded, smoky camps; 2–5 days): red, crusted eyes for about 10 days; many bouts can blind (`BIO-18`).
    - **Spotted fever** (crowd; breath; about 10 days): fever and spots for about 10 days; kills 1 in 10, more of children; immunity for life.
    - **Pox** (crowd; breath, touch; about 12 days): fever and blisters that leave scars, for about 15 days; kills 3 in 10; immunity for life.
    - **Bloody flux** (crowd; water fouled by a village's waste; 1–3 days): cramps and bloody runs for about a week; kills 1 in 10.
    - **Wasting cough** (crowd; long close living, and sick cattle's milk; a season or more): cough and wasting over one to three years; kills about half.
  - **Why:** Illness shapes history: a fever can take a band's last knapper (`MOM-02`), and villages pay for their numbers with crowd illnesses.
  - **Done when:** over 20 runs of test scenes (`RES-13`), each illness spreads only by its routes, runs about its stated course and kills about its stated share, and a crowd illness that reaches a band burns out while a village keeps it.

- `BIO-23` **Care and healing** *(Decided)*: The hurt and the sick do better with care, and some treatments are blueprints to discover.
  Nothing heals a body but the body itself; care and medicine only help it.
  - **How it works:**
    - **Plain care,** an everyday activity anyone can do, drawn by the caring leaning (`MND-26`): pressing a bleeding wound slows it by about three quarters, and a few minutes of it stop all but the worst; food, water, warmth, carrying and company help the body (`BIO-05`).
    - **Healing blueprints** (`MAT-23`), found or learned like any other (`MND-11`): washing a wound with clean water halves its chance of infection; a moss dressing bound on stops the bleeding and halves the chance of infection again; a splint of straight sticks and cord lets a broken bone heal straight; a yarrow poultice or a willow-bark drink gives medicine (`BIO-12`); and a fat salve eases a burn and keeps it clean like a dressing.
    - **Skill:** care works better with the carer's healing experience, up to about twice as well for a master (`MND-06`, tuned).
    - **Overall,** the sick and wounded cared for every day die about half as often as those left alone (tuned).
    - **Rites and comfort** ease pain and lift mood but cure nothing, so belief in healing rites rests on recoveries that would have come anyway (`MND-05`, `CUL-26`).
    - **Healers** are sought out, and where food allows, some heal for a living (`CUL-32`).
  - **Done when:** in test scenes, cared-for people die about half as often as those left alone, washed wounds get infected about half as often, and splinted breaks heal straight far more often.

- `BIO-14` **Every death has a cause** *(Decided)*: Nobody dies for no reason: every death comes from hunger, thirst, cold, heat, bleeding, a wound, illness, poison, drowning, childbirth, violence, an accident or old age.
  - **How it works:**
    - **Only through the body:** a person dies only when the body passes a limit: condition, water or blood running out (`BIO-09`, `BIO-13`), freezing or heatstroke (`BIO-11`), a head or torso at 0 (`BIO-13`), a deadly illness or poison (`BIO-05`, `BIO-12`), too long under water (`BIO-21`), or an old body giving out (`BIO-16`).
    - **Chance comes through events,** such as a slip, a blow that lands, an illness caught or a hard birth, which your fortune can tilt (`GOD-04`).
    - **The record:** each death keeps its cause and how it came about, such as "bleeding, from a boar's tusk, while hunting", for the book of ages, graves and cards (`PRE-05`, `PRE-09`, `PRE-35`).
  - **Check:** every death in test runs and overnight worlds names its cause and how it came about, and none is unknown.

### 8.4 A life

- `BIO-04` **Life cycle** *(Decided)*: Childhood to about 14, adulthood, old age from about 45, and death, mostly before 70, in game years (`TIM-18`).
  - **How it works:**
    - **Stages:** a baby lives on milk and is carried (`BIO-15`); a child walks at about 1, talks at about 2 and is weaned at 2 to 3; from about 5, children help with gathering and carrying, play a lot, and learn by watching and being taught (`MND-13`).
      Adulthood begins at about 14, with full growth by about 16 and children possible from about 15, and old age at about 45 (`BIO-16`).
    - **Growth:** children grow toward their inherited height and build (`BIO-06`), held back for life by long hunger or serious illness (`BIO-08`).
  - **Target ranges** on whole worlds (`RES-14`), coming out of the rules, never set:
    - about 1 baby in 5 dies in its first year, and about 2 children in 5 before 14;
    - those who reach 14 live on average to about 50–60, most die before 70, and few pass 80;
    - a woman who lives through her childbearing years has about 5 to 7 children;
    - in good times numbers grow by about 1% a year, reaching the low thousands by the copper age (`TIM-19`).
  - **Done when:** overnight whole worlds land within these ranges in at least 16 of 20 runs (`RES-13`).

- `BIO-15` **Pregnancy and birth** *(Decided)*: Children come from couples, through a pregnancy of about 45 days, a birth with real risks, and years of nursing.
  Who pairs with whom is cultural (`CUL-27`), and pairing and conception are never shown, so sexual violence is not part of the game.
  - **How it works:**
    - **Conception:** a partnered woman of about 15 to 45 (`BIO-16`), in fair condition (`BIO-09`) and not nursing a baby under about 2, conceives within about 30 days on average (tuned); it is never an action, an animation or a description.
    - **Pregnancy** lasts about 45 days; the mother needs about a fifth more food, and in the last 15 days she tires sooner and moves more slowly; about 1 in 6 ends early, more in hungry, ill or older mothers (tuned).
    - **Birth** takes hours; about 1 in 10 is hard, more for a first child or a young, old, small or hungry mother (tuned), and can bleed (`BIO-13`) or kill the baby.
      Wound fever can follow (`BIO-05`), about 1 birth in 100 kills the mother, a helper makes it safer, a skilled one more so (`BIO-23`), and twins come about once in 80.
    - **Nursing:** milk alone feeds a baby for about half a year, then with soft food until weaning at 2 to 3; the mother needs about a quarter more food, and a hungry one makes less milk.
      Nursing holds back the next pregnancy, so births come about every 3 years in bands, sooner where porridge or animal milk lets babies wean early (`CUL-28`); a baby whose mother dies lives only if another nursing woman feeds it, or, once older, on soft food.
    - **The record:** each birth is kept with mother, father (her partner), date and place (`PRE-10`, `PRE-05`).
  - **Done when:** in test scenes and whole worlds, pregnancies last about 45 days, births in bands come about every 3 years, about 1 birth in 100 kills the mother, and a young baby left without milk dies unless another mother feeds it.

- `BIO-16` **Ageing** *(Decided)*: From about 45, bodies slowly weaken, heal more slowly and fight illness worse, so most people die before 70.
  Knowledge and skill don't fade, so elders matter as keepers of what their people knows (`CUL-02`).
  - **How it works:**
    - **Decline:** from about 45, strength, stamina, healing and resistance fall by about 2% a year (tuned); eyes lose near sight and then sharpness, hearing loses quiet sounds (`BIO-18`), hair greys and backs bend (`BIO-22`).
    - **Fertility:** women's falls from the late 30s and ends at about 45; men's falls slowly.
    - **Giving out:** from about 55 the body itself can give out, through a failing heart or a stroke, at a chance of about 1 in 50 a year that doubles every 7 years or so (tuned).
      With frailty in illness, cold and falls, this makes most adults die before 70 and few pass 80 (`BIO-04`).
    - **Minds:** what people know stays, though the old learn more slowly (`MND-06`).
  - **Done when:** ages at death on whole worlds match `BIO-04`, and in test scenes the old heal and recover from illness more slowly than the young.

### 8.5 The sexes

- `BIO-17` **Real biology, culture decides** *(Decided)*: Men and women differ only in real body ways: pregnancy and nursing, and on average size, strength and body fat, with wide overlap.
  Who hunts, gathers, leads or makes things is up to each culture (`CUL-27`), and minds don't differ by sex.
  - **How it works:** on average men are a little taller (about 7%) and stronger (about a third, most in the arms), and women carry more body fat (tuned); every mind trait has the same spread in both sexes (`MND-20`).
    No rule gives anyone a task or role by sex.
  - **Check:** a review of the rules finds a person's sex used only for pregnancy, nursing, size, strength, body fat, looks and voice, and test scenes show no task or role given by sex.

### 8.6 Senses and activities

- `BIO-18` **Senses** *(Decided)*: Sight, hearing, smell, taste and touch, with plausible ranges that change with light, weather, the person and age.
  They decide what reaches each mind (`MND-03`).
  - **How it works:**
    - **Sight:** by day in the open, people see a moving person or deer at about 1 km and spot small things, such as a flint nodule, within about 30 m (tuned).
      Land, trees and walls block it, and night, fog, rain, snow and smoke cut it (`WLD-16`); movement catches the eye, and a still, crouching body is harder to see.
    - **Hearing:** a shout carries about 1 km in still air, talk about 50 m and footsteps about 20 m (tuned); wind, rain and rushing water drown sounds.
    - **Smell:** smoke and rot carry a few hundred metres downwind, and many animals smell far better (`BIO-19`).
    - **Taste:** sweet, salty, sour, bitter and savoury; bitterness warns of many poisons, though not all (`MND-21`).
    - **Touch:** texture, hardness, warmth, wetness, sharpness, weight and pain.
    - **People differ** by body (`BIO-08`) and age (`BIO-16`), and scarred eyes see less (`BIO-05`).
  - **Done when:** in test scenes, people spot a deer in the open at about the stated range by day and far nearer at night or in fog, hear a shout at about 1 km, and wolves find people from farther downwind than upwind.

- `BIO-21` **Everyday activities** *(Decided)*: The body's side of the everyday activities: walk, carry, eat, drink, sleep, talk, play, fight, flee, care for someone, teach, watch, sing and dance.
  Everyone can do them from the start, and none is a blueprint (`MAT-06`).
  - **How it works:**
    - **Walking:** about 4–5 km an hour on open, flat ground, slower uphill, in forest, marsh or snow, or loaded, so a day's walk covers 20–30 km (tuned); the young, the old, the hurt and the heavily pregnant go slower.
    - **Water and heights:** people wade to waist depth; deeper water must be swum, which tires and can drown the weak in cold or fast water, and about 3 minutes under water drowns anyone (`BIO-14`); a slip while climbing is a fall (`MAT-11`).
    - **Carrying:** an adult carries about a quarter of their own weight all day, more for a short way (`MAT-11`), by their strength (`BIO-08`).
    - **Running and fleeing:** about three times walking speed, until out of breath (`BIO-09`).
    - **Eating, drinking and sleeping:** a meal takes a quarter to half an hour and a drink a minute (`BIO-10`); sleepers wake to loud sounds, pain, cold or a touch.
    - **Fighting:** blows land by chance, by strength and hunting experience (`MND-06`), and wound a part (`BIO-13`); most fights end when one is hurt, flees or gives up (`MND-33`).
    - **The rest,** caring (`BIO-23`), talking, teaching, watching, singing, playing and dancing, use voice, eyes and body, and play and dance tire like work.
  - **Done when:** in test scenes, people walk 20–30 km a day on open ground and less loaded or uphill, someone fleeing a bear is out of breath within minutes, and the hurt and the heavily pregnant fall behind.

### 8.7 Inheritance

- `BIO-06` **Inherited traits** *(Decided)*: Looks, build and personality leanings come from both parents, with variation; families resemble each other, but nothing evolves.
  - **How it works:**
    - **What passes on:** the body numbers of `BIO-08`, the twelve personality traits (`MND-20`), and looks (`BIO-22`).
    - **How:** a child's value is its parents' average, pulled about halfway back toward the human average, plus chance (tuned for each); so tall parents have tall children, though less tall, and brothers and sisters differ.
    - **No evolution:** because every value is pulled back toward the same human average, no family line or people drifts from it over generations, whoever survives, and each trait's spread stays as it was at the start.
    - **Life adds the rest:** childhood hunger and illness shape the body (`BIO-08`), and big events nudge personality (`MND-20`).
  - **Done when:** over 20 generations of whole-world runs, the average and spread of every inherited trait stay within about 5% of the start, while children resemble their parents.

- `BIO-07` **Evolution dial** *(Dropped)*
  - **Dropped because:** nothing evolves (`BIO-06`), and play has no rule-bending settings (`PRN-12`).

- `BIO-22` **Looks** *(Decided)*: Skin, hair, eyes and faces are inherited, so children look like a mix of their parents, and no people looks like a copy of a real one (`SCP-20`).
  - **How it works:**
    - **Two copies:** each person carries two values for each feature (skin tone, hair colour, hair form, eye colour and face shape), one from each parent, shows a blend of them, and passes one, by chance, to each child.
      So brothers and sisters differ, and a grandparent's red hair can come back.
    - **Peoples apart:** a people grown from a few families keeps its founders' looks, so peoples long apart come to look somewhat different; looks never help anyone survive.
    - **Mixed at the start:** the first people's looks are drawn from wide ranges and mixed, so that no group matches any real people's typical look.
    - **Age and life:** hair greys and skin wrinkles (`BIO-16`), and scars, limps and pox marks stay (`BIO-13`, `BIO-05`), all shown on the figure (`PRE-27`).
  - **Done when:** in test scenes children visibly mix their parents' looks, and in whole worlds peoples apart for centuries look somewhat different.

### 8.8 Animals

- `BIO-19` **Animal bodies** *(Decided)*: Animals near people have bodies on the same pattern: a head, a body and legs, with wings or fins where they have them.
  - **How it works:**
    - **Wounds** work as for people (`BIO-13`): hurt legs slow an animal, a broken wing grounds a bird, and a speared animal bleeds and slows, or runs off to die later, leaving a blood trail hunters can follow.
    - **Needs** follow the same rules with their kind's numbers (`WLD-32`): food and water by size, warmth from fur, feathers or fat, and rest; their condition (`BIO-09`) shows in the fat they yield (`WLD-18`).
    - **Illness:** a few animals of the kinds that carry an illness are sick, and pass it on by bites, carcasses, milk or water (`BIO-05`).
    - **Life:** they are born in their season, grow, age and die in game years (`WLD-32`), of the same causes as people (`BIO-14`).
    - **Senses** have their kind's ranges: wolves and dogs smell people about 1 km downwind, and birds of prey see farther than people (tuned).
    - **Far from people,** herds are counts without bodies (`WLD-32`).
  - **Done when:** in test scenes, a speared deer bleeds, slows and can be tracked and found, weak animals die first in a hard winter, and a mad wolf can pass foaming madness by a bite.

## 9. Minds

How people think, and in simpler form how animals think.
Minds are the heart of the game.
They are built the way the best life games build them: needs, and choices weighed by how well each option serves them (The Sims); thoughts that lift or lower mood for a while, and breakdowns (RimWorld); personalities, memories, relationships and legends (Dwarf Fortress).
Every mind learns only from its own world (`PRN-01`), every choice can be explained (`PRN-13`), and no AI language model thinks for anyone (`PRN-06`).

### 9.1 Ground rules

- `MND-01` **No AI language model thinks for them** *(Decided)*: Every choice, belief and discovery comes from the rules in this section (`PRN-06`).
  - **How it works:** minds run only on the game's own rules and numbers.
    The writer AI (`PRE-37`) only reads finished records to write text for you, and nothing it writes goes back into the world (`PRE-17`).
  - **Check:** an automated check finds no call from the simulation to any language model, and no path from the writer's text back into the simulation.

- `MND-02` **Knowledge only from inside the world** *(Decided)*: A mind knows only its starting knowledge (`BIO-20`) and what it has since seen, done, been told or dreamt.
  Nobody knows a blueprint, a hidden characteristic or a far place until they find it out or learn it (`PRN-01`).
  - **How it works:** every choice is built from the person's own blueprints (`MND-06`), knowledge of things (`MND-04`), mental map (`MND-28`) and beliefs (`MND-27`).
    The catalogues are the world's rules, and no mind reads them.
  - **Check:** test scenes confirm that nobody chooses a blueprint they don't know or heads for food they never learned of, and the reasons kept with each choice (`MND-09`) name only what the person knows.

- `MND-17` **Why ordinary minds are enough** *(Decided)*: Nobody needs to be a genius.
  A people's cleverness comes from four things, and only one of them is inside a head:
  1. **generic blueprints** (`MAT-04`): one blueprint works for anything with the right characteristics, so simple tries find real results;
  2. **simple rules in each mind**, the ones in this section;
  3. **many minds over generations**, copying, teaching and improving (`CUL-01`);
  4. **time:** a one-in-a-thousand chance comes up routinely over a few centuries.

### 9.2 Needs and personality

- `MND-07` **Needs** *(Decided)*: Nine needs drive everyone: hunger, thirst, warmth and rest for the body (`BIO-09`), and safety, belonging, status, curiosity and love for the mind.
  - **How it works:**
    - **A level for each,** from 0 (desperate) to 100 (met).
      The body's needs follow the body (`BIO-09`, `BIO-11`), and the mind's follow life:
      - **safety** falls with danger seen or believed near, and rises in shelter, by a fire and among many;
      - **belonging** falls with time alone or shunned, and rises with company, talk, shared work and rites;
      - **status** follows the respect others show (`MND-24`), raised by praise and being followed, lowered by insults and failing in front of others;
      - **curiosity** falls with sameness and idle time, and rises with new places, things, stories and experiments;
      - **love** falls with time apart from partner, children and close friends, and rises with time together; adults also want a partner.
    - **Weights:** personality sets each need's pull (`MND-20`).
      The most pressing needs weigh most in every choice (`MND-09`), and unmet ones bring bad thoughts (`MND-29`).
    - **Children:** babies feel only the body's needs, safety and love, and cry to have them met; the other needs grow in through childhood (`BIO-04`).

- `MND-20` **Personality** *(Decided)*: Twelve traits make each person different: curious, brave, cautious, patient, hard-working, playful, sociable, kind, greedy, hot-tempered, proud and spiritual.
  - **How it works:**
    - **A level for each,** from −3 to +3, where the low end is the opposite: timid, generous, calm and so on.
      Most people sit near the middle, and a few are extreme.
    - **Inherited and shaped:** traits are set at birth from both parents, with variation (`BIO-06`), the same way for both sexes (`BIO-17`).
      Big events nudge them for life: a child who survives a bear attack grows more cautious.
    - **What each does** in every choice (`MND-09`):
      - **curious:** new things, places and experiments pull harder, and surprises are noticed more (`MND-10`);
      - **brave:** danger and pain count less, and fear fades faster;
      - **cautious:** the unknown seems riskier, so new foods, places and ways are tried less, and custom, warnings and taboos weigh more;
      - **patient:** the future counts more (`MND-22`), and long tasks tire less;
      - **hard-working:** work beats idling, so they practise more and learn faster;
      - **playful:** play, music and dance pull harder, and play often turns into experimenting;
      - **sociable:** belonging matters more, and they talk and visit more;
      - **kind:** others' needs count, so they share, help, comfort and teach more;
      - **greedy:** they want more, share less, bargain hard and may steal;
      - **hot-tempered:** anger comes fast and lasts, so they quarrel and fight more;
      - **proud:** status matters more, insults hurt more, and they want to lead;
      - **spiritual:** unseen beings seem likelier and matter more (`MND-31`), and rites pull harder.
    - **Shown as words** on the card, such as "very curious, hot-tempered, generous" (`PRE-35`).

- `MND-21` **Inborn leanings** *(Decided)*: Leanings every human is born with.
  They make some lessons easier but teach nothing by themselves (`PRN-01`).
  - **How it works:** each is a fixed lean in the rules, never a belief:
    - **taste:** sweet and fat taste good and bitter bad (`MND-29`), so many bitter poisons are avoided, though not all;
    - **pain and ready fears:** whatever caused pain, and snakes, heights, the dark and big predators, are feared after one fright, where other fears need several (`MND-08`);
    - **parents and babies** love each other from birth (`MND-07`);
    - **copying:** seeing others do something makes doing it likelier (`CUL-01`);
    - **a hidden someone:** when something important happens and no believed cause explains it, people may come to believe an unseen being did it (`MND-31`), the spiritual more often.

- `MND-26` **Social leanings** *(Decided)*: Social instincts every human is born with, and the only social leanings built into the rules (`CUL-07`):
  - **kin:** the good of kin counts in choices, more for closer kin, as kinship is believed (`MND-24`);
  - **caring:** seeing someone hurt, sick or hungry pulls toward helping, more for kin and friends;
  - **favours:** help received leaves a debt that pulls toward repaying, and taking without returning angers and costs trust (`MND-24`);
  - **own group:** people of one's own band or people, known by shared words, ways and looks, are trusted more than strangers;
  - **raised together:** people who lived closely as small children never want each other as partners;
  - **shared attention:** people follow another's gaze and pointing, which makes teaching work (`MND-13`);
  - **a beat:** a shared beat draws people to move together, which warms them to each other (`CUL-10`).

### 9.3 Mood and feelings

- `MND-29` **Mood and thoughts** *(Decided)*: Mood sums up how a person feels about life.
  It comes from their needs and recent thoughts, each lifting or lowering mood for a while, as in RimWorld.
  - **How it works:**
    - **Thoughts:** events and states give thoughts with a size and a duration, such as "ate cooked meat" (+5 for a day), "slept cold" (−4 for a day), "insulted by Tamo" (−5 for three days) and "my child died" (−20 for 30 days, fading).
      They come from a set list of about 100 kinds (tuned).
    - **Bent by personality** (`MND-20`): the kind feel others' losses more, and the proud feel insults twice over.
      The same thought repeated adds less each time.
    - **Feelings and memories:** a strong feeling brings its thought while it lasts (`MND-19`), and recalling a strong memory brings its thought back for a while (`MND-18`).
    - **Mood** runs from 0 to 100, moving over a few hours toward 50 plus all live thoughts, including those from unmet needs.
    - **What it does:** low mood slows work, sours talk and risks a breakdown (`MND-30`).
      High mood speeds work, and above about 85 it can bring a few days of inspiration, with more experimenting and better-made things (`MND-11`, `MAT-20`).

- `MND-19` **Feelings** *(Decided)*: Seven strong feelings, each with its own duration: fear, anger, grief, joy, love, shame and awe.
  - **How it works:**
    - **What sets them off:** fear, danger close or believed close; anger, harm, insult or a goal blocked; grief, losing someone loved; joy, success, a birth or a feast; love, warmth toward one person, growing with time and kindness shared (`MND-24`); shame, breaking a rule one holds (`CUL-20`) or failing in front of others; awe, something vast or unexplained, such as a great storm, a rite or a strange dream.
    - **Duration:** fear lasts minutes to hours, anger hours to days, joy a day or two, shame and awe days, grief seasons, and love as long as it is fed.
      Personality bends them: the hot-tempered anger faster, and the brave fear less (`MND-20`).
    - **What they do:** they push choices (fear to flee, anger to quarrel or fight, grief to stillness, joy to company, love to stay close, shame to hide, awe to rites and art).
      They also give thoughts (`MND-29`), strengthen memories (`MND-08`), and show in faces and poses (`PRE-27`).

- `MND-30` **Breakdowns** *(Decided)*: When mood stays very low, a person may break: in a rage, by running off, or in despair.
  - **How it works:**
    - **The risk:** below a mood of about 20 a breakdown can start, likelier the lower and the longer mood stays down (tuned).
      The hot-tempered rage, the proud and the brave run off, and the rest despair (`MND-20`).
    - **Rage:** shouting, smashing things, or attacking whoever angered them (`MND-33`).
    - **Running off:** leaving the band alone for a day or more, which can end in a return, in joining another band, or in death in the cold.
    - **Despair:** lying still, refusing work and food, for a day or more.
    - **After:** a short lift in mood.
      The breakdown is remembered by all who saw it (`MND-18`), and others may comfort or shun them (`MND-24`).
  - **Why:** Hard times should show in people and start stories, such as a feud that began with one rage.
  - **Done when:** in a test scene of a starving winter some people break down, and in a good season almost nobody does.

### 9.4 Memory and knowledge

- `MND-03` **Noticing** *(Decided)*: People take in only some of what their senses reach (`BIO-18`): what is close, moving, loud, new, dangerous, or useful for a pressing need.
  - **How it works:** whatever happens is offered to everyone within sight or hearing, and each takes it in or misses it.
    Danger and their own task are always noticed; anything else less when busy, tired, frightened or asleep, and more if curious (`MND-20`).
    A thing's visible characteristics are known on sight (`MND-04`).
    What is noticed can give a thought (`MND-29`), a feeling (`MND-19`), a memory (`MND-18`), news for the mental map (`MND-28`) or a surprise (`MND-10`).

- `MND-18` **Memories** *(Decided)*: People remember events with their importance.
  Memories fade unless they matter or are recalled, and those retold in talk become shared stories.
  - **How it works:**
    - **What is kept:** events that touched the person (a birth, a death, a hunt, a fight, a gift, a first, a dream, a story heard), with what happened, who was there, where, when, and how they felt (`MND-19`).
    - **Importance** comes from the kind of event and the strength of feeling (`MND-08`): a child's death lasts a lifetime, a good meal a few days.
    - **Fading:** memories weaken with time, more slowly the more important they are, and are renewed when recalled, told or dreamt (`MND-12`).
      A person holds about 200 (tuned), and the weakest go first.
    - **Recall:** places, people and things bring back their memories, and with them their thoughts (`MND-29`): passing the river where her son drowned brings back Ama's grief.
    - **Retelling:** hearers keep told memories (`CUL-24`) as stories from that person, weaker than their own.
      Each telling can drift: numbers grow, the teller's part swells, and causes shift toward the teller's beliefs (`MND-31`).
      Stories retold through a band become shared stories, then legends and myths (`CUL-11`).

- `MND-08` **Feelings shape memory** *(Decided)*: The stronger the feeling at the time, the longer a memory lasts.
  A terrifying storm stays for life; an ordinary day fades in days.
  - **How it works:** a memory's importance grows with the strength of the feeling when it was made (`MND-19`).
    Fear also ties itself to the place, animal or person that caused it, so meeting them again brings the fear back, and people may avoid them for years: a band that lost a hunter on a hill shuns the hill.

- `MND-04` **Knowing things** *(Decided)*: People learn what things are like by seeing and using them.
  Visible characteristics, such as size, colour and edge, are known at a glance; hidden ones, such as poison, medicine, fuel and flaking, only by use or by being told (`MAT-03`).
  - **How it works:**
    - **Kinds they know:** for each kind of item they have met (`MAT-10`), its visible characteristics and the hidden ones learned, each with how sure they are.
      Each people names the kinds in its own language (`CUL-18`).
    - **Learning by use:** eating shows food and poison (`MND-05`), burning shows fuel, striking shows flaking, and a wound shows medicine.
      Being told passes it on, weighed by trust (`MND-24`).
    - **Look-alikes:** an untried kind is taken to be like the known kind it looks most like.
      So a band that knows flint may take chert for flint, and a poisonous berry like a safe one is eaten until a sickness teaches otherwise.

- `MND-28` **Mental map** *(Decided)*: What each person knows of places: food, water, stone, shelter and dangers, by season.
  It is learned by going there and shared by talk.
  - **How it works:**
    - **Places,** from a spring to a valley, each with the way there and roughly how long it takes.
      Each holds what is there and when: food plants and when they ripen, game and when it passes, water, stone, clay, wood, shelter, and dangers such as cliffs, bears and strangers: "hazelnuts on the south slope in autumn".
    - **Learning:** by seeing (`MND-03`) and by being told (`CUL-24`).
      Each fact keeps when it was last seen and how sure it is, and old facts can be wrong: the berries eaten, the spring dry.
    - **Using it:** where to gather, hunt, fetch stone, shelter or move comes from it (`MND-09`), and exploring adds to it.
    - **Start and size:** adults start knowing their home range (`BIO-20`).
      A person knows up to a few hundred places (tuned), and the least used fade.

### 9.5 Beliefs

- `MND-27` **Beliefs** *(Decided)*: A belief is something a person holds true, with a strength from a faint guess to certain, and a source.
  - **How it works:**
    - **Kinds:** what things are like (`MND-04`); places and seasons (`MND-28`); causes, including hunches about making things (`MND-05`, `MND-11`); unseen beings (`MND-31`); rules (`CUL-20`); and what others know (`MND-23`).
    - **Sources:** their own experience, something seen, a dream, or being told, and by whom.
      Own experience counts most, a trusted elder strongly, a stranger little.
    - **Clashes:** the stronger belief guides choices, and later evidence settles it.
    - **Shown to you** with its strength, source and the memories behind it, in the details view (`PRE-14`).

- `MND-05` **Beliefs about causes** *(Decided)*: After a strong outcome, people link it to something unusual that came before.
  Later outcomes strengthen or weaken the link.
  This is how real knowledge arises, and also wrong beliefs, taboos and rituals.
  - **How it works:**
    - **Linking:** after a strong outcome (a big hunt, a death, an illness, a storm, a sudden recovery), the person looks over the day or two before for the most unusual thing, something they rarely do or meet.
      It may be a food, a place, an act (a song, a gift, a broken rule), an animal, a person, or a sign in the sky.
      They link it, "this brings that", and the link keeps the memories behind it (`MND-18`).
    - **Testing:** each time the cause comes again, an outcome that follows strengthens the link and its absence weakens it; weak links are forgotten.
      Hits count more than misses, as with real people, so a link that holds only by chance can last.
    - **Wrong beliefs last:** "this brings harm" makes people avoid the cause, so it is never tested again: a taboo (`CUL-20`).
      "This brings luck" costs little to keep doing, and luck comes often enough: a ritual (`CUL-06`).
    - **Shared:** links are told (`CUL-24`), and those many hold become their people's lore.
  - **Example:** Eating a new root the day before a fever teaches "that root brings fever", true or not.
    Singing before a good hunt can become a rite (`MOM-04`).
  - **Done when:** in a test scene, both a real poison and a harmless food eaten before a chance fever come to be avoided, and in some runs a song sung before lucky hunts becomes a habit.

- `MND-31` **Beliefs about the unseen** *(Decided)*: People can come to believe in beings nobody sees: spirits of places, animals and weather, and the dead.
  - **How it works:**
    - **Start:** a big event with no known cause can, through the hidden-someone leaning (`MND-21`), leave a weak belief that an unseen being did it.
      The event shapes it: the spirit of that hill, the storm or the bears, or a dead grandmother (`CUL-05`); dreams of the dead feed belief in ancestors (`CUL-19`).
    - **Growth:** later events of the same kind are put down to the same being, acts before good outcomes are credited with pleasing it (`MND-05`), and dreams and stories strengthen it (`CUL-24`).
    - **Effect:** believers weigh what the being is believed to want (offerings, rites, keeping away from its places), and doing it brings a good thought and a sense of safety (`MND-29`).
    - **Your acts** are explained the same way, and never known as yours (`GOD-06`).
      Beliefs many share grow into religion (`CUL-26`).

### 9.6 Choosing and planning

- `MND-09` **Choosing what to do** *(Decided)*: Every option is scored by how well it serves the person's needs, personality, plans and beliefs, and the best usually wins.
  The top reasons are kept and shown.
  - **How it works:**
    - **When:** a choice comes when an activity ends or is interrupted (a threat, a call, pain, an urgent need), or when a plan's time comes (`TIM-17`): about 10 to 30 times a game day.
    - **Options:** about 30 at most: meeting a need from what they know (the store, the hazel slope, the spring, sleep), a step of a plan or ambition, a known blueprint with things in reach, a request or a group plan, a social act (`MND-33`), play, rest, exploring or experimenting (`MND-11`).
      Only known blueprints, places and beliefs make options (`MND-02`).
    - **Score:** how much the option should meet each need, weighted by how pressing that need is and by personality, plus plans and ambition.
      Beliefs add or take away (luck from a rite, harm from a broken taboo), and so do others' expectations (`CUL-06`).
      Effort, time, distance and risk take away, with risk weighed by bravery and caution, and the whole is scaled by the chance they expect it to work.
    - **Habits:** what they usually do at that place, time and season scores a little higher, so days have a rhythm, and habits a group shares become customs (`CUL-06`).
    - **Picking:** usually the best, sometimes one close behind by chance (`TIM-16`), so people are not machines.
    - **Reasons kept:** the three biggest, such as "thirsty; believes the river is safe at dawn; plans to check the fish trap".
      They are shown on the card and in the details view (`PRE-35`, `PRE-14`), and kept with any event the choice led to (`PRN-13`).
  - **Done when:** in test scenes, hungry people go to the nearest food they know of, frightened people flee, and every choice shows three reasons that name only what the person knows.

- `MND-22` **Plans** *(Decided)*: Short plans tie choices together over days and seasons: store food before winter, build a shelter, make a spear for tomorrow's hunt, fetch flint from the far cliff.
  - **How it works:**
    - **Sources:** needs foreseen through beliefs about the seasons (late winter brings hunger, so store nuts in autumn, `MND-28`); the chain of blueprints a goal needs (a cloak needs a scraper, scraped and dried hides, then sewing); ambitions (`MND-32`); and requests or group plans (`CUL-22`).
    - **A plan** is a goal with steps and times, and each step scores higher in choosing (`MND-09`) until the plan is done, fails or is dropped.
    - **Limits:** about five plans at once, of a few steps each.
      The patient plan further ahead (`MND-20`), and tallies and a calendar let plans reach further (`CUL-03`, `CUL-13`).

- `MND-32` **Ambitions** *(Decided)*: Each adult has a life ambition that colours their choices for years: master a craft, lead, raise a big family, be a great hunter, heal, know the unseen, find new land, grow rich, or avenge a death.
  - **How it works:**
    - **Chosen in youth,** at about 14 (`BIO-04`), from personality and life so far: a curious youth who watched a master knapper may want to master stone, a proud one to lead.
      Big events can change it, as a killing can bring a wish for revenge.
    - **Effect:** options that move it forward score higher (`MND-09`), and it makes plans (`MND-22`).
    - **Fulfilled or lost:** reaching it brings a long, strong good thought (`MND-29`) and a memory for life.
      Losing hope of it brings a lasting bad thought, and perhaps a new ambition.
    - **Shown** on the card, with how close they are (`PRE-35`).

### 9.7 Skills and discovery

- `MND-06` **Experience and skill** *(Decided)*: People get better at things by doing them.
  Each person has experience in 15 sectors and a skill in each blueprint they know, both from 0 (none) to 10 (master).
  - **How it works:**
    - **Sectors:** stone, wood, fire, cooking, hunting, gathering, hides, building, healing, pottery, herding, farming, metal, art and music.
    - **Blueprint skill** starts higher for people experienced in the blueprint's sector.
      Skill and experience both raise its chance of success and the quality of what is made (`MAT-04`, `MAT-20`).
    - **Rising with use:** every try adds a little, more on a success and more when taught (`MND-13`), and less at high levels, so the first steps come fast and mastery takes years.
      Everyday work in a sector, such as caring for the sick or singing, adds to its experience too.
      The hard-working practise more (`MND-20`), and each person learns at their own speed (`BIO-08`), children fastest and the old slower (`BIO-16`).
    - **Fading:** unused skill and experience fade slowly, never below about half their best.
    - **Dying out:** a blueprint is lost with its last holder, though things they made may survive to be copied (`CUL-02`, `MND-11`).
  - **Example:** A girl taught to knap knows the blueprint, but at skill 1 she shatters most stones before one good flake.

- `MND-13` **Learning and teaching** *(Decided)*: People learn blueprints, skills and facts from each other by watching, being taught and talk.
  Being taught is fastest.
  - **How it works:**
    - **Watching** someone use a blueprint you don't know gives a hunch for it (`MND-11`).
      Watching it several times can teach it at low skill, and watching someone more skilled adds a little skill.
    - **Being taught:** someone who knows a blueprint, believes another doesn't (`MND-23`) and wants them to (kin, friends, the kind, or for a gift) can teach, the two spending the activity together.
      A taught try adds several times the skill of a try alone (tuned), more with a better teacher, and the first taught success makes the blueprint known.
    - **Being told:** facts, places and beliefs pass in talk (`CUL-24`), weighed by trust (`MND-24`).
    - **Children** learn fastest, mostly from family and band, helped by pointing and a shared gaze (`MND-26`).
    - **Across a group,** who copies whom, and how crafts spread or die out, is in Culture (`CUL-01`, `CUL-02`).

- `MND-10` **Surprises** *(Decided)*: Something unexpected catches the eye and sticks: a stick smoking as it is drilled, seeds sprouting on the rubbish heap, a bead of shiny metal in the ashes.
  - **How it works:** a result or sight never met before, or one their beliefs did not expect, is noticed more often by the curious (`MND-20`) and less by the busy, tired or frightened (`MND-03`).
    It gives a strong memory (`MND-18`), a pull to look into it (`MND-07`), and a link between what came before and what happened (`MND-05`), often a hunch (`MND-11`).
    With spare time, they may repeat what came before to see whether it happens again.

- `MND-11` **Four routes to discovery** *(Decided)*: Nobody knows a blueprint until they discover it or learn it (`PRN-01`).
  Discoveries come by accident, by experimenting, from a dream's hint, or by copying.
  - **How it works:**
    - **Hunches:** a hunch is a guess that some things, with some action, might give some result, such as "twirling a stick on dry wood might make fire".
      It is a belief with a strength (`MND-27`) that guides experiments, and a person holds a handful at a time.
    - **By accident:** an action on things that happen to fit an unknown blueprint (`MAT-04`) has a small chance to work anyway, or to end in one of its failures that hints at it, such as smoke but no ember.
      If noticed (`MND-10`), a success teaches the blueprint at low skill, and a hint gives a hunch.
      So cracking nuts with a flint can knock off a sharp flake.
    - **By experimenting:** curious people with spare time and a fair mood try actions on things in reach: a hunch first, else an action they know works on something that looks similar (what scrapes wood might scrape bone), else something new.
      A try that fits a blueprint can succeed, less often than a taught one, and a failure still teaches about the things tried (`MND-04`).
      Hard times push people to experiment too.
    - **From a dream's hint:** a dream can join a thing, an action and a needed result from different memories into a hunch (`MND-12`), real or not.
      You can send one whose hint is near the dreamer's experience (`GOD-03`).
    - **By copying:** a thing they can't make, made by others or found in an abandoned camp (`MAT-08`), gives a hunch to make it from things like its materials.
      Seeing it made gives a stronger hunch, with the actions (`MND-13`).
    - **The blueprint decides every try** (`MAT-04`), and a people's first success is a named discovery in the book of ages, with who made it (`MAT-21`).
    - **Pace:** the chances of accidents and untaught tries are tuned so that discoveries come at a pace you can watch (`PRN-17`, `TIM-19`).
  - **Example:** Drilling a hole in dry wood, Ama sees the stick smoke and keeps a hunch.
    In a hard winter, with the band's fire dead, she twirls faster and longer with drier wood until an ember catches, and others watch and are taught (`MOM-01`).
  - **Done when:** in sandbox scenes each route leads to discoveries in some runs, and the sharp-stone test passes (`RES-03`).

- `MND-12` **Dreams** *(Decided)*: Each night a sleeper has one dream, made from recent strong memories.
  Dreams keep memories alive, stir feelings, sometimes hint at something new, and are your lever (`GOD-03`).
  - **How it works:**
    - **Made from** the last few days' strongest memories, by feeling, surprise and need, now and then with an older one; those memories then fade more slowly (`MND-18`).
    - **Feelings:** the dream's feeling lingers as a thought (`MND-29`).
      A nightmare after a wolf attack leaves fear, and a dream of a dead mother leaves grief and feeds belief in the dead (`CUL-19`).
    - **Hints:** a dream can join pieces of memories into a hunch (`MND-11`), more often when a need presses; the chance is small (tuned).
    - **Remembered and told** (`CUL-24`), dreams feed stories and beliefs about the unseen (`MND-31`).
    - **Your lever:** a dream you send replaces that night's own and uses only what the dreamer has lived: a place, an animal, a person, a fear, or a hint at a blueprint near their experience (`GOD-03`).
      Nothing marks it as yours (`GOD-06`).
    - **Animals** dream more simply (`MND-16`).

### 9.8 Life together

- `MND-24` **Relationships** *(Decided)*: People know who is who: kin, friends, rivals, partners and enemies.
  Each has an opinion of everyone they know, with trust and respect, that grow and fade with what happens between them.
  - **How it works:**
    - **For each person they know,** up to about 150: face and name, kinship as believed, an opinion from −100 to +100, trust, respect, favours owed (`MND-26`), shared memories (`MND-18`), and what they know of them (`MND-23`).
    - **Opinion** rises with help, gifts, shared food, kind words and shared success, and falls with insults, harm, cheating and broken promises (`MND-33`).
      It drifts back toward neutral when they don't meet; like personalities get on better, and a hot temper wears on everyone.
    - **Trust** sets how far their word is believed (`MND-27`) and whether they are followed.
      **Respect** follows their skill, success, age and generosity, as their culture values them (`CUL-22`).
    - **Bonds:** lasting high opinion makes friends; love between adults who court makes partners (`CUL-27`); rivalry over status, a partner or a place makes rivals; harm makes enemies, and a killing can start a feud (`CUL-08`).
    - **Shown** on cards and family trees (`PRE-35`, `PRE-10`).

- `MND-33` **Social acts** *(Decided)*: What people do with each other: chat, share, help, comfort, tell, ask, play, court, teach, gossip, insult, quarrel, fight and steal.
  - **How it works:**
    - **Chosen like anything else** (`MND-09`): the sociable chat more, the kind comfort and share, the hot-tempered quarrel, and mood colours it all (`MND-29`).
    - **Effects:** each act moves both people's opinions (`MND-24`) and needs: a chat meets belonging, praise meets status, comfort eases grief, and an insult hurts.
    - **Talk:** what is said (news, places, memories, warnings, requests, lies) is set out in Culture (`CUL-24`), and is heard as a murmur, never as real words.
    - **Courtship:** an adult wanting a partner (`MND-07`) courts someone they like with time together, gifts and help.
      If love grows on both sides, they pair as their customs allow (`CUL-27`, `BIO-15`).
    - **Quarrels** can turn into fights when anger runs high (`MND-19`); fights hurt (`BIO-13`), and others step in, take sides or remember.
    - **Gossip** spreads what someone did, and opinions with it.

- `MND-23` **Who knows what** *(Decided)*: People track what others know, want and feel, as far as they can tell.
  This makes teaching, news, secrets and lies possible.
  - **How it works:**
    - **Tracked** for each person they know (`MND-24`): which of their own blueprints and places that person knows (seen using them, gone there together, or told), what they seem to need, and how they seem to feel toward them.
    - **Teaching and telling** go only to those believed to lack it (`MND-13`, `CUL-24`).
    - **Secrets:** a good flint source, a food store or a broken taboo can be kept from others, who may still find out by watching, following or gossip.
    - **Lies:** people say what they don't believe when they think it pays, the greedy and the proud more often; a lie found out costs trust (`MND-24`).
    - **Children** manage this from about age four.
  - **Example:** Tamo keeps a good flint source secret.
    Ama has noticed the fresh flakes he brings back, and follows him one morning.

### 9.9 Scale

- `MND-14` **Every person has a full mind** *(Decided)*: Every person has every part of this section at all times.
  Nobody gets a cheaper or simpler mind for being far away or unwatched (`WLD-13`).
  - **How it works:** minds stay cheap because they work only when something happens: they choose when an activity ends or is interrupted (`TIM-17`, `MND-09`), take in only what they notice (`MND-03`), and keep capped numbers of memories, places, hunches and plans (`MND-18`, `MND-28`, `MND-11`, `MND-22`).
  - **Check:** the same saved world, run with the camera in different places and at different speeds, gives the same choices (`WLD-13`).

- `MND-15` **Population limit** *(To test)*: How many people a world can hold at a watchable speed is found by measuring the phone (`PLT-04`).
  The design target is about 2,000 people, with at least one game year per real minute for 1,000 people (`TIM-07`).
  - **How it works:** nothing caps births: food, illness and danger set numbers.
    As a world nears the limit, time slows, no mind is ever simplified (`PRN-11`), and the game tells you.
    To fit the target, a whole person, body and mind together, should use about a thousandth of a second of computing per game day, or less.

- `MND-25` **Minds hold records, not sentences** *(Decided)*: Everything in a mind (needs, thoughts, memories, beliefs, plans and the reasons for each choice) is kept as structured records about people, places, things and events, with numbers, never as sentences.
  The writer AI turns them into words for you (`PRE-37`), and nothing it writes is read back (`MND-01`).
  - **How it works:** the details view shows the records as they are or as the writer's text (`PRE-14`), and talk passes them between minds as topics (`CUL-24`).

### 9.10 Animals

- `MND-16` **Animal minds** *(Decided)*: Animals have simpler minds: needs, fear, herd behaviour, learned fear of people, and taming.
  Near people each animal has its own mind; far away, herds are counts that keep only their wariness and their routes (`WLD-32`).
  - **How it works:**
    - **Needs:** hunger, thirst, warmth, rest and safety, plus their kind's urges: herding, pack hunting, guarding young, territory and the breeding season.
    - **Choosing:** the same scoring as people (`MND-09`) over a few options (graze, hunt, drink, rest, flee, follow the herd, fight, play), with no blueprints, talk or plans.
    - **Fear:** danger seen, heard or smelt makes them flee or fight as their kind does, and herds flee together.
      Hunted animals learn to fear people, their smell and their camps, so hunting grows harder where people hunt (`WLD-32`).
    - **Boldness:** each is born more or less bold, which sets how near people it dares to come.
    - **Memory:** a few places, and the people and animals they know, such as the boy who feeds them or the hunter who wounded them.
    - **Taming:** animals fed and not harmed lose their fear and grow attached to particular people.
      Young raised by people grow up tame (`RCK-24`), and lines kept for generations become domestic kinds (`WLD-33`).
    - **Dreams:** a simple replay of one memory; you can send one that draws an animal or herd toward a place, or makes it calmer or bolder (`GOD-12`).
    - **Explained:** an animal's reasons can be seen like a person's (`PRN-13`).

## 10. Culture and society

Culture is what people pass to each other rather than inherit: crafts, words, beliefs, customs and art.
With minds, it is the heart of the game.
Templates give it shapes, and the world's events decide which shapes appear (`CUL-07`).
It spreads, changes, splits and dies.

### 10.1 How culture works

- `CUL-07` **Nothing social is scripted** *(Decided)*: Templates give the shapes; events decide which happen.
  - **What:** The game holds fixed templates for beliefs, customs, roles, dealings between groups, and art, songs and myths.
    A template says what kind of event can lead to what kind of belief, custom or role, such as "an unexplained death may make people believe an angry spirit lives where it happened".
    None is set off by a script, a date or an era: each needs its conditions in the world and people's own choices (`MND-09`), and the real event fills it in.
    Like blueprints, templates never name a particular people, person, place or date (`PRN-07`).
    Templates, art motifs, dance moves and story shapes are catalogues, written in advance and checked by automated tests (`MAT-13`).
    What a group comes to share is named in its language, and its first appearance enters the book of ages (`PRE-05`).
  - **Check:** an automated test finds no template tied to a date, an era or a named people, place or person; sandbox scenes show each template appearing only after its triggering events; and across 20 test worlds, peoples end with different spirits, customs and kinds of leader.

- `CUL-33` **Pace of culture** *(To test)*: When culture first shows, in game years from the start, in typical worlds over many runs, alongside the pace of discovery (`TIM-19`).
  - First shared belief in a spirit or in the dead: within 5 years.
  - First rite kept as a custom: 5–20.
  - First myth: 10–40.
  - First band split: 10–50.
  - First festival: 10–60.
  - First new people: 60–150.
  - First chief: 150–350, after the first villages.
  - **Why:** A lively world shows culture early and keeps changing it (`PRN-17`); these targets show whether templates come too rarely or too often (`RSK-26`).

### 10.2 Passing things on

- `CUL-01` **Learning from others** *(Decided)*: People pass on what they know by watching, teaching and talk (`MND-13`).
  - **How it works:**
    - **Watching:** someone who watches a blueprint being used close by may learn it, more likely with experience in its sector.
      Children learn most this way, copying adults' work in play.
    - **Teaching:** a skilled person works beside a learner, who learns surely and gains skill faster.
      People teach their children, kin and friends, and, where it is the custom, apprentices (`CUL-32`).
    - **Talk:** a listener told that something can be made, and from what, finds it far sooner when they try (`MND-11`); talk also carries news, beliefs, stories and customs (`CUL-24`).
    - **Whom they learn from:** those they trust and respect, the most skilled, and kin (`MND-24`); and most people do things the way their group does.
  - **Example:** The first knapper's daughter learns beside her, other children copy in play, and within a few years half the band can make flakes.

- `CUL-02` **Knowledge can be lost** *(Decided)*: A craft lives only in the people who know it and dies with the last of them, so small or isolated groups lose crafts most easily (`MOM-02`).
  - **How it works:**
    - **Lost:** when the last holder of a blueprint dies, or nobody has used it for so long that the skill has faded (`MND-06`), the people loses it; songs, stories and rites go the same way.
    - **Fragile:** in a band of twenty, one person may hold a craft, and one fever or fall can take it; elders hold the most, so losing them costs most (`BIO-16`).
    - **Regained:** by finding it again (`MND-11`), by learning it from neighbours who kept it (`CUL-16`), or by copying things left in an old camp (`MAT-08`).
      The book of ages marks the loss and any rediscovery (`PRE-05`).

- `CUL-16` **How things spread** *(Decided)*: Crafts, beliefs, customs, songs and words spread only where people meet.
  - **How it works:** they pass in shared camps, visits and festivals (`CUL-29`), with a spouse who moves to another band (`CUL-27`), through trade (`CUL-21`) and with captives (`CUL-31`); a traded or found thing can also be copied by people who never met its maker (`MND-11`).
    Mountains, seas, wide rivers and distance set how often groups meet, and groups that rarely meet grow apart in customs, beliefs, style and new words.

- `CUL-03` **Memory outside heads** *(Decided)*: Marks, tallies and pictures let some knowledge outlast the people who had it.
  Writing is not part of the launch arc.
  - **How it works:**
    - **Tallies** cut in bone or wood count days, kills or debts (`MAT-04`), so people can count the days to a festival or a plan (`CUL-13`, `MND-22`).
    - **Pictures** of real events remind those who see them (`CUL-09`), and painted rocks, piled stones and cut trees mark paths, graves, sacred places and borders (`CUL-23`).
    - **Meaning needs a reader:** when the last person taught what a mark stands for dies, the mark remains but the people lose its meaning, though you can still see it (`PRE-15`).

### 10.3 Language

- `CUL-04` **Language emerges** *(Dropped)*
  - **Dropped because:** each world now has one language from the start, which never changes (`CUL-17`).

- `CUL-17` **A language from the start** *(Decided)*: Each world has one language, made with the world, spoken by all its peoples, and never changing.
  - **How it works:**
    - **Made with the world:** its own sounds, how they join into words, and a few hundred everyday words (kin, body, food, animals and plants, land, actions, feelings, small numbers).
      It copies no real language (`SCP-20`).
    - **Every name** comes from it: of people, places, peoples, spirits, discoveries, festivals and songs (`CUL-18`).
    - **Never changes:** no drift, no dialects and no new grammar, so peoples that split still understand each other.
    - **New words:** each people coins its own for new things, such as a named discovery (`MAT-21`), by joining old words ("stone-that-cuts") or making one in the language's shape.
      So peoples that split come to differ in their newer words, and a craft learned from neighbours usually keeps their word, showing where it came from.
    - **Heard as a murmur** of its sounds, never as real words (`SND-03`); what is said is known as topics (`CUL-24`).

- `CUL-18` **Names** *(Decided)*: People, places, peoples, spirits and things are named in the language, often after events, features or traits, and shown with their meaning in English (`PRE-38`).
  - **How it works:** parents name a child after a trait, an event at the birth or an honoured ancestor (`CUL-19`), and a striking deed can earn a second name, such as "Bear-killer".
    Places are named the first time people talk of them, after a feature ("Red Cliff"), an event ("Where the Boar Died"), a person or a spirit.
    Spirits, discoveries, customs, festivals, songs and peoples are named when they first appear, by whoever first talks of them, and names spread with talk.

- `CUL-24` **Conversations** *(Decided)*: People talk all day; what they say is kept as topics, never as sentences, and heard as a murmur.
  - **How it works:**
    - **Topics:** news (food, water, danger, a death), a memory retold as a story (`MND-18`), a belief, how something is made (`CUL-01`), gossip and opinions of others, a plan or request (hunt together, move camp, marry), a question, comfort, and quarrels.
    - **Weighed by trust:** what is heard is held less firmly than what one lived through, and more firmly the more the speaker is trusted (`MND-24`, `MND-27`).
    - **Effects:** a good talk raises affection, a quarrel lowers it, and gossip changes what listeners think of others.
    - **Secrets and lies:** people keep secrets when telling would cost them, such as a good flint source, and may lie when they believe it pays (`MND-23`); a lie found out costs trust.
    - **For you:** a murmur shaped by the language and the speaker's mood (`SND-03`); the details view of each mind lists the topics (`PRE-14`), which the writer AI can put into English (`PRE-17`).

### 10.4 Belief and religion

- `CUL-05` **Beliefs from events** *(Decided)*: Strong events nobody can explain give rise to spirits, rites, offerings and taboos, shaped by a few templates.
  Your acts as god are explained the same way.
  - **How it works:**
    - **The trigger:** a strong event that those who saw it cannot explain with what they know (`MND-05`): a sudden death or illness, a great or deadly hunt, lightning, a storm, a flood, a drought, a hard winter, a quake, or a vivid dream.
    - **The templates**, each filled in with the real event (`CUL-07`):
      - a spirit of the place where it happened;
      - a spirit of an animal kind, after a great or deadly hunt;
      - a spirit of the sky or the weather;
      - the dead living on (`CUL-19`);
      - a taboo, when harm follows an act (`CUL-20`);
      - a rite, when a good outcome follows an act, which is then repeated before the same task (`MOM-04`);
      - an offering, when a bad time ends after something was given or left.
    - **Who believes:** any witness may, the spiritual, the frightened and the grieving more often (`MND-20`, `MND-19`); a spirit is kind or angry as the event helped or harmed.
    - **Growing and fading:** fitting events and retelling (`CUL-24`) strengthen a belief, events that go against it weaken it, and one that nothing renews fades over years.
      When most of a band holds it, it is the band's, and it gets a name (`CUL-18`).
    - **What beliefs do:** people avoid feared places, keep taboos, hold rites and leave offerings, at a cost in time and things (`MND-09`).
    - **Your acts** reach people only as nature (lightning, luck, dreams), so they feed the same templates (`GOD-06`).
    - **Only beliefs:** spirits exist only in minds (`MND-27`), and nothing in nature answers them (`SCP-19`).
  - **Example:** Your lightning becomes a god (`MOM-03`).

- `CUL-19` **Ancestors** *(Decided)*: Grief and dreams of the dead lead people to believe the dead live on and watch over their kin.
  - **How it works:** those who loved a dead person dream of them (`MND-12`), and each dream strengthens the belief, in the dreamer and in those who hear it told (`CUL-05`).
    Respected elders and leaders are dreamt of most and become the strongest ancestors.
    From the belief come graves with things for the dead (`MAT-08`), gifts and rites at graves, children named after ancestors (`CUL-18`), asking the dead for help, and fear of the dead who were wronged.
    A dream you send of a dead person works like any other (`GOD-03`).

- `CUL-20` **Taboos** *(Decided)*: Acts that come to be forbidden: eating a food, entering a place, killing an animal, working at a sacred time, marrying certain kin (`CUL-27`).
  - **How it works:** harm that follows an act, such as sickness after eating a fish or a death after entering a cave, can make the act forbidden (`MND-05`); taboos also come with spirits and from being told.
    Breaking a taboo one holds brings fear and shame (`MND-19`), and others who hold it may punish the breaker (`CUL-06`).
    A taboo on a poisonous plant protects and one on a good food costs, the people cannot tell which is which, and many taboos outlive their cause.

- `CUL-26` **Religion** *(Decided)*: Shared beliefs grow into religion: rites, sacred places, shamans and later priests, and myths.
  - **How it works:**
    - **A people's religion** is its shared spirits and ancestors, with their rites, taboos, sacred places and myths, named after its greatest spirit.
    - **It grows in steps, each when its conditions hold:**
      1. **Shared spirits** that a band names, fears or thanks (`CUL-05`).
      2. **Rites** done together before the hunt, at graves or at festivals (`CUL-06`, `CUL-29`).
      3. **Sacred places,** such as a struck hill, a spring or a field of graves, marked with paint, stones or offerings.
      4. **A shaman:** the one others turn to about spirits, usually spiritual, respected and known for vivid dreams or for surviving a grave illness, who leads rites, heals with rites and herbs, and whose explanations others believe most.
      5. **Myths** about the spirits and the people's beginnings (`CUL-11`).
      6. **Priests and shrines:** in a village that can feed a full-time specialist (`CUL-32`), the shaman's role can become a priest's, often passed down in a family, with a built shrine and rites on the calendar (`CUL-13`).
    - **Gods:** a spirit a whole people holds strongly, with rites, myths and a sacred place, is a god in all but name.
    - **Change:** religions travel with marriages, trade and conquest (`CUL-16`), and gain and lose spirits as events come and go.
    - **Effects:** shared rites lift mood and bind people, offerings and sometimes lives are given (`CUL-08`), and shamans and priests gain influence (`CUL-22`).

### 10.5 Society

- `CUL-30` **Bands** *(Decided)*: People live in bands of a few families that move, camp and share together; bands split when too big and join others when too small.
  - **How it works:**
    - **Belonging:** children belong to their parents' band, and married people to the one their custom names (`CUL-27`).
    - **Moving:** each season the band chooses where to camp by its leader's plan and its members' mental maps (`CUL-22`, `MND-28`), moving a few times a year until it settles (`CUL-28`).
    - **Splitting:** past about 40 people (tuned), when food runs short, or after a bitter quarrel or a failed challenge to the leader, some families leave to found a new band nearby; the two stay kin and keep meeting.
    - **Joining:** a band below about 10 people (tuned) joins kin in another band.
  - **Why:** Splitting spreads people over the land and begins new peoples (`CUL-23`).

- `CUL-27` **Kin and marriage** *(Decided)*: Everyone knows their kin; who may marry whom, where couples live and what is given are customs that differ between peoples.
  - **How it works:**
    - **Kin:** parents, children, brothers and sisters and partners, and through them grandparents, cousins and in-laws (`MND-24`), favoured in sharing, help and revenge.
    - **Marriage:** two adults drawn to each other court and pair (`MND-24`), and it is a marriage once their families accept it, often with gifts or a feast; pairing itself is never shown (`BIO-15`).
    - **Customs**, set by what most marriages have done (`CUL-06`):
      - who may not marry: always close family, from the inborn aversion to those one was raised with (`MND-21`), and for some peoples cousins or the whole band;
      - where couples live: with the man's kin, the woman's kin, or either;
      - what is given to the partner's family, and whether a marriage can end.
    - **Marriages bind groups:** kin across bands make feuds costlier and alliances easier (`CUL-31`, `MOM-11`).
    - **Work:** customs about who does what arise the same way, and no rule gives work by sex (`BIO-17`).

- `CUL-06` **Customs, norms and punishments** *(Decided)*: What most of a group does the same way becomes a custom; customs people expect are norms, and breaking one brings punishment.
  - **How it works:**
    - **Customs:** something most of a group has done the same way for a few years (tuned), such as sharing meat, burying the dead or holding a rite, is named (`CUL-18`), taught to children (`CUL-01`) and kept after its reason is forgotten (`MOM-04`).
    - **Norms:** people think less of anyone who breaks a custom they expect (`MND-24`).
    - **Punishments**, from mild to harsh: scorn and gossip, being left out of sharing, gifts paid to the wronged, a beating, being driven out, and under some chiefs death (`CUL-08`).
      Who punishes, and how hard, is itself a custom (`CUL-22`).
    - **Change:** customs shift with behaviour, split when groups split, and end when nobody keeps them.

- `CUL-22` **Leaders, councils and chiefs** *(Decided)*: Bands follow leaders, bigger groups decide in councils, and settled or warring groups come to have chiefs.
  - **How it works:**
    - **Status** is how much others respect a person (`MND-24`), from skill, generosity, success, age, courage, a parent's standing or fear; which counts most is a custom of each people.
    - **Leaders:** when a band must choose together (where to camp, whether to fight), it follows its most respected and trusted member, with no title at first.
      A leader whose plans fail loses followers, and can be challenged or left behind (`CUL-30`).
    - **Councils:** at gatherings of bands, in villages and in any group above about 40 people (tuned), the heads of families decide together.
    - **Chiefs:** where there are stores, herds or fields to share out, or raids every few years, one leader may gain lasting power to settle quarrels, lead raids, share out stores and punish (`CUL-06`).
      Once people expect a chief's child to follow, leadership is inherited.

- `CUL-32` **Specialists** *(Decided)*: People known for a craft work for others, and where food allows, some do it full time.
  - **How it works:** someone with much experience in a sector, such as stone, healing or music (`MND-06`), is sought out and repaid with food or gifts (`CUL-21`), and takes apprentices (`CUL-01`).
    Where stores, fields or herds can feed people who neither gather nor hunt, mostly in villages (`CUL-28`), a knapper, potter, healer, priest (`CUL-26`) or copper-worker can live by the craft.
    Specialists reach high skill and quality (`MAT-20`), which makes long chains such as copper practical, but a craft held by one specialist dies with them (`CUL-02`).

- `CUL-21` **Sharing and trade** *(Decided)*: Food is shared, gifts bind people, and groups trade what they have plenty of for what they lack.
  - **How it works:**
    - **Sharing:** big kills are shared across the band by custom (`CUL-06`), since meat rots before one family can eat it (`MAT-19`), and shared food is repaid in lean times (`MND-24`).
    - **Gifts** raise affection and leave a debt to repay (`MND-24`).
    - **Trade:** when groups meet, at festivals or set places, each side swaps what it has plenty of, such as flint, ochre, shells, salt, furs, pots or copper, for what it lacks, valued by need and scarcity, never at a fixed price.
      Regular partners in different bands come to trust and host each other.
    - **Ownership:** what one makes is one's own; once people settle, houses, stores, fields and herds belong to families (`CUL-28`); and a people's land is its own (`CUL-23`).
    - **No money** in the launch arc.

- `CUL-31` **Feuds, raids and alliances** *(Decided)*: Killings breed feuds; hunger, greed and revenge breed raids; marriages, trade and shared enemies breed alliances.
  - **How it works:**
    - **Quarrels:** insults, theft, rivalry in love and unfair sharing sour opinions and can end in fights (`MND-24`).
    - **Feuds:** a killing or a bad wound makes the victim's kin want revenge, weighed against the risk (`MND-09`), and each revenge can bring another.
      A feud ends with payment, a marriage between the sides, a council's or chief's ruling, or one side leaving.
    - **Raids:** a hungry, greedy or vengeful group may raid another when its leader believes it can win by numbers, weapons or surprise, to take food, stores, herds, land or captives (`CUL-08`).
      Raids are rare between kin, common between hostile peoples, and grow with villages worth raiding.
    - **Alliances:** groups on good terms help each other in raids and defence and share hunting grounds, often sealed with a marriage or a feast, until relations sour.
    - **Wars:** years of raids back and forth between two peoples make a war, which the book of ages names (`PRE-05`).

- `CUL-08` **Dark history can happen** *(Decided)*: Violence and war, captivity and slavery, sacrifice, cruelty, infanticide and cannibalism can arise like anything else.
  Sexual acts stay abstract (`BIO-15`).
  What is shown is set by the content setting (`PRE-18`).
  - **How it works:** these come from the same rules as everything else, never from a script: violence is a choice weighed against its cost (`MND-09`); captives taken in raids can be kept to work (`CUL-31`); sacrifice is an offering of a life where fear and belief run high (`CUL-05`); infanticide comes when parents believe they cannot feed a newborn; and cannibalism comes from starvation or rite.
    No action exists for sexual violence.
    Dark events are always stated as plain facts from the data, never written by the writer AI (`PRE-17`).

- `CUL-23` **Peoples and territories** *(Decided)*: Bands that share a name, customs and beliefs make a people with its own land; peoples split, merge and vanish.
  - **How it works:**
    - **The first people:** the starting bands are one people, with a name for themselves (`BIO-03`).
    - **New peoples:** bands that have rarely met, married or feasted with the rest of their people for about two generations (about 50 game years, tuned) become a new people, with its own name from a place, a founder or a spirit.
      From then on its customs, beliefs, style and new words go their own way.
    - **Merging and ending:** a people can be absorbed by marriage or conquest, and it ends when its last band dies out or joins another.
    - **Territory:** the land its bands use (camps, hunting grounds, sacred places, graves), shifting as they move; strangers there are met with caution, and driven off or raided if relations are bad (`CUL-31`).
    - **Relations:** each people stands toward each other one somewhere from friendly to hostile, warmed by marriages, trade and festivals and chilled by raids and killings; people trust their own people most.
    - **Shown** on the map (`PRE-07`) and in the book of ages, with one timeline for each people (`PRE-05`).

- `CUL-28` **Villages** *(Decided)*: A band settles all year in one place once nearby food lasts all year, and much changes when it does.
  - **How it works:**
    - **Settling:** when stores, fields, herds or rich fishing feed the band through every season, staying beats moving (`CUL-30`); after a full year in one place, its camp is a village.
    - **What changes:**
      - lasting houses, stores and pens replace huts (`MAT-04`), and rubbish heaps grow (`MAT-08`);
      - villages grow to a few hundred people, and families own houses, stores, fields and herds, so some grow rich (`CUL-21`);
      - councils, chiefs (`CUL-22`), full-time specialists (`CUL-32`), priests and shrines (`CUL-26`) become possible;
      - crowd illnesses spread (`BIO-05`), stores draw raiders (`CUL-31`), and firewood and game grow scarce nearby.
    - **Abandoned:** drought, failed harvests, illness or raids can empty a village, leaving an old camp for later people to find (`PRE-09`).
    - **Pace:** villages appear 100–300 years from the start in typical worlds (`TIM-19`).

### 10.6 Expression

- `CUL-25` **Expression is real** *(Decided)*: Every work of art is a real thing or a real performance, and keeps what it shows and who made it.
  - **How it works:** paintings, carvings, beads, figures and instruments are made by blueprints from real materials (`MAT-04`) and wear away like any thing (`MAT-20`); songs, dances and told stories leave nothing behind and live on in memory (`CUL-01`).
    Each work keeps the real event or myth it shows, its maker, its people's style and its date, so you can tap it and see its story (`PRE-15`), and people who see or hear it recall what it shows (`CUL-03`).

- `CUL-09` **Visual art** *(Decided)*: Paintings and carvings composed from prepared motifs in each people's style, most often showing real events.
  - **How it works:**
    - **Motifs:** about 100 small pictures drawn by the designers: each animal kind, people in poses (hunting, dancing, carrying, lying dead), tools and weapons, fire, sun, rain, lightning, water, trees, huts and hands, and patterns such as dots, zigzags and spirals.
    - **What is shown:** one of the maker's strongest memories, often a hunt, a death, a flood or a festival, or a myth (`CUL-11`), with motifs for its real animals, people and things set into a scene.
    - **How it looks:** drawn in the people's style (`CUL-12`), in the colours of the pigments used (`RCK-15`, `RCK-16`), as true and fine as the maker's art skill allows (`MND-06`).
    - **Where:** cave and shelter walls, rocks, bone, antler, wood, hides, pots and bodies, and small carved figures and beads.
    - **Why people make it:** after a strong event, at rites, for respect or for play, the playful and the spiritual more often (`MND-20`); art at a sacred place strengthens its beliefs (`CUL-26`).
  - **Example:** A painting that remembers (`MOM-07`).

- `CUL-10` **Music and dance** *(Decided)*: Songs generated in each people's musical style, played on instruments made from blueprints, and danced to with each people's own steps.
  - **How it works:**
    - **Musical style:** each people has a scale of a few notes, favourite rhythms and a pace, taken from its parent people and drifting slowly (`CUL-12`).
    - **Songs:** now and then someone with music skill makes a new one in that style, about what matters to them (a hunt, a death, a spirit, a child, a love, a festival), and names it (`CUL-18`).
      Songs pass on by singing together (`CUL-01`), change a little as they do, and are lost when nobody remembers them.
    - **Instruments** are blueprints (`MAT-04`): bone flutes, hide drums, rattles, clappers and bullroarers, each sounding by its materials (`SND-06`).
    - **Dances** join about 20 prepared moves (steps, stamps, turns, jumps, raised arms) into each people's own dances; dancing together lifts mood and draws people closer.
    - **Uses:** lullabies, work songs, laments, hunting songs and rites (`MOM-04`), heard when you are near (`SND-02`).

- `CUL-11` **Myths and stories** *(Decided)*: Retold memories become stories, and stories of spirits and beginnings become myths, changing a little with each telling.
  - **How it works:**
    - **Stories:** a memory retold often becomes a story the group shares (`MND-18`, `CUL-24`).
    - **Myths** take one of about a dozen story shapes, filled with the people's real events (`CUL-07`): how a spirit came to be, how a gift such as fire came, how the people began, a great flood or winter, a hero's deed, why a taboo is kept, or a journey to new land.
    - **Changing:** a retelling can change one detail: numbers grow, a deed moves to a more famous person, or a spirit's part grows.
    - **Kept by telling** at the fire, at rites and at festivals, learned by children, and lost if nobody tells them.
    - **Written for you** in prose by the writer AI from the myth's facts, adding none (`PRE-17`).
  - **Example:** Ama's first fire becomes "Ama stole the fire that sleeps inside the wood".

- `CUL-15` **Remembered lives** *(Decided)*: Genealogies and legends: who descends from whom, and the remarkable people a culture remembers.
  - **How it works:** people remember their forebears back a few generations, further where ancestors are honoured (`CUL-19`), with gaps and errors (`MND-18`), and chiefs and priests may claim descent from a founder or a spirit (`CUL-22`).
    Legends are stories of remarkable people, growing with each retelling (`CUL-11`).
    The true family tree is kept too, so legend can be set against what happened (`PRE-10`).

- `CUL-12` **Style and ornament** *(Decided)*: Each people has its own look in tools, clothes, huts, art and music, drifting over time, so a thing shows who made it and roughly when.
  - **How it works:** a style is a few choices for each kind of thing, such as proportions, patterns and favourite colours, and the way it draws motifs (`CUL-09`) and makes songs (`CUL-10`).
    A new people starts with its parent's style, slightly changed; each generation copies it with small changes, and peoples in contact borrow from each other (`CUL-16`).
    Every made thing carries its maker's people's style (`PRE-43`), so finds can be told apart and roughly dated (`PRE-09`).
    Beads, pendants, body paint and decorated clothes show status and belonging.

- `CUL-14` **Their maps** *(Decided)*: Maps drawn the way they see the land, right or wrong.
  - **How it works:** someone showing others a far place may draw a map on the ground, a hide, bone or rock: motifs for rivers, hills, camps and herds (`CUL-09`), laid out as the maker remembers them (`MND-28`), so distances bend and mistakes stay.
    Those who see it and know its marks learn those places, as if told (`CUL-24`), and a map on the ground is gone by the next rain.

- `CUL-13` **Their calendar** *(Decided)*: People learn the year's signs, name its seasons and mark the times that matter, which set their plans and festivals.
  - **How it works:** the seasons of the 60-day year (`TIM-18`) show in signs people learn (`MND-28`): first frost, herds passing, nuts falling, the river rising, the longest day.
    A people names its seasons and key moments (`CUL-18`), such as "when the salmon come", and plans by them (`MND-22`), and tallies of days (`CUL-03`) keep festivals on the right days (`CUL-29`).
    Each people's calendar is its own, apart from the game's dates (`TIM-14`).

- `CUL-29` **Festivals** *(Decided)*: Gatherings at set times of their calendar for feasts, rites, songs and dances, marriages and trade.
  - **How it works:**
    - **Beginning:** when bands meet at the same place and season a few years running (tuned), usually where food is then plentiful (a salmon run, a nut harvest, a herd crossing), the meeting becomes a custom (`CUL-06`), held each year by their calendar (`CUL-13`), often at a sacred place (`CUL-26`).
    - **What happens:** feasts, rites to spirits and ancestors, songs and dances (`CUL-10`), myths (`CUL-11`), marriages (`CUL-27`), trade (`CUL-21`), games, and councils of leaders (`CUL-22`).
    - **Effects:** joy and belonging run high for days (`MND-19`), and crafts, beliefs, songs and news spread between bands (`CUL-16`), so peoples that feast together stay alike.
    - **Named** and entered in the book of ages (`PRE-05`); a festival not held for a few years is forgotten.

## 11. Presentation

### 11.1 Visual style

This is how the world looks: the look you approved on the visual-style mockup.
It is written to stand on its own, without needing any image to understand it.

- `PRE-01` **Detailed pixel art** *(Decided)*: Everything on screen is crisp pixel art: limited colours, hard pixel edges, no blur and no smooth gradients.
  - **How it works:** the picture is drawn at a low resolution, about a quarter of the screen's in each direction (`PRE-22`), and enlarged by whole pixels with no smoothing; every colour comes from the material ladders of one palette (`PRE-20`).

- `PRE-02` **Pixel-rendered 3D** *(Decided)*
  - **What:** The world is a real 3D world, drawn at low resolution and enlarged with hard pixel edges.
    It looks like hand-made pixel art but has real depth, scale and structure.
    The camera turns freely and zooms continuously.
  - **How it works:** the picture shows what the world holds at that moment: the ground's shape, rock, water and plants from its areas and world cells (`WLD-12`), each thing from its model (`PRE-42`), and people and animals as they are (`PRE-27`).
    Places nobody has visited are drawn from the seed, exactly as people will find them (`WLD-13`).
    The look was measured smooth on the phone at every zoom.
  - **Why:** Real 3D shows height, depth, sizes and structures (cliffs, caves, huts, later villages) at every zoom.
    The land comes straight from the world instead of being hand-drawn, which suits generated worlds.
  - **Example:** At dusk, from an oblique angle, you see a band's camp below a limestone cliff: the cave mouth in shadow, long shadows across the grass, the river beyond.
    You turn the camera and fly down until one person fills the screen.

- `PRE-20` **Colour in steps** *(Decided)*
  - **What:** Every material has a short ladder of shades, about 4–7 colours, drawn from one master palette.
    Ladders are made automatically from each material's colour (`MAT-10`) and matched to the palette; common materials, such as grass, limestone and water, get hand-picked ladders instead.
    Light chooses a step on the ladder.
    Where two steps meet, a fine pixel pattern blends them in a narrow band only; surfaces are never speckled all over.
    The pattern is fixed to the surface, so it doesn't swim when the camera moves.
  - **How it works:** a ladder runs from a material's darkest shade to its lightest; the light reaching each point of a surface (sun, sky, fire and shadow, `PRE-30`) picks the step, and only in a narrow band at each step's edge does the fine pattern mix two steps.
  - **Why:** Clean colour is what separates pixel art from a shrunken photograph.

- `PRE-21` **Outlines and lit edges** *(Decided)*
  - **What:** A one-pixel dark outline wherever one thing stands in front of another: people, animals, trees, rocks, the top edge of a cliff.
    A one-pixel bright edge where the sun or a fire catches a shape, such as the sunlit rim of a cliff or the fire-facing side of a person.
  - **How it works:** a dark pixel is drawn wherever something near meets something farther behind it, and a bright pixel at a shape's edge where its surface faces the sun or a fire strongly (`PRE-30`).
  - **Why:** Crisp silhouettes keep small things readable on a phone screen.

- `PRE-22` **Stable pixels** *(Decided)*
  - **What:** Pixels never crawl or shimmer while the camera is still or panning: the picture stays locked to its pixel grid, and turns ease to rest.
    During a free turn or zoom, some crawling can't be avoided at this resolution without blur; the fix that best lessens it is chosen on a real world at the first visual review (`PRE-31`).
    One art pixel is always the same size on screen, in portrait and in landscape, so turning the phone only changes the framing.
    About 4 screen pixels make one art pixel.
  - **How it works:** the camera's position is snapped to whole art pixels, and turns ease to rest; each art pixel is a fixed block of about 4 by 4 screen pixels in both orientations.
  - **Why:** Shimmering pixels are the most common flaw of 3D pixel art, and the first thing that makes it look cheap.

- `PRE-23` **Rock faces** *(Decided)*
  - **What:** Cliffs show their geology: horizontal rock layers of different thicknesses, irregular vertical cracks, a few long fissures, lichen, water stains, soot above inhabited caves, grass hanging over the top edge, and scree at the foot.
    The same layers continue underground (`PRE-25`).
  - **How it works:** a cliff is drawn from the rock layers where it stands (`WLD-09`), each with its rock and thickness, cracks and fissures.
    Lichen and water stains come from how wet the face is and which way it faces (`WLD-16`), soot from the smoke of fires below over the years (`MAT-18`), the overhanging grass from the plants growing at the top (`WLD-31`), and the scree from the loose stones at the foot (`WLD-12`).
  - **Why:** Geology is part of the story (`WLD-14`).
    What people can find depends on what the land is made of, and the rock should show it.

- `PRE-24` **Real shapes** *(Decided)*
  - **What:** Overhangs, caves, rock shelters and, later, buildings have real depth.
  - **How it works:** caves, overhangs and shelters are part of the ground's real shape in each area (`WLD-12`), and built things are drawn from their models and materials (`PRE-42`); inside, light comes only from openings and fires (`PRE-30`), so the depths stay dark.
  - **Example:** Looking into a cave mouth from an angle, you see its dark interior, the firelit floor and the hide windbreak across the entrance.

- `PRE-25` **Cut-away view** *(Decided)*
  - **What:** The ground can be sliced open to show what lies beneath: rock layers, soils, underground water, and the buried layers of past life (hearths, tools, bones, graves).
  - **How it works:** a slice along the line you choose shows the rock layers (`WLD-09`), the soil (`WLD-27`), water held in the ground (`WLD-17`), and buried things at their depths, in the layers that buried them (`MAT-08`).
  - **Why:** It is how you see geology and dig through history.
    The view of graves and old camps (`PRE-09`) uses it.

- `PRE-26` **Water** *(Decided)*
  - **What:** Rivers meander and change width, with gravel bars, reeds, lines that follow the current, ripples at fords, glints of sun and drifting mist.
    From far away a river never becomes thinner than one or two art pixels, so it stays readable.
  - **How it works:** a river is drawn from its course, width and flow (`WLD-17`): lines that follow the current's direction and speed, gravel bars and reeds from the ground and plants along it (`WLD-12`), ripples where it runs shallow, glints by the sun's angle (`PRE-30`), and mist where the weather makes fog (`WLD-16`).

- `PRE-27` **People and animals** *(Decided)*
  - **What:** People and animals are small 3D figures built from tiny blocks, with separate parts (head, torso, arms and legs), drawn through the same pixel look.
    They are posed like sprites, about 10 poses a second (`PRE-44`), so they look like crisp pixel art from any angle and turn properly with the camera.
    At the closest zoom, a person is about 40–60 art pixels tall: enough for a face, hair, clothing and gestures.
  - **How it works:** each figure is built from its body's parts (`BIO-13`), shaped by its own build, age and looks (`BIO-08`, `BIO-22`), and wears and carries what that person actually has, drawn from its materials (`PRE-42`).
    Its face shows its strongest feeling (`MND-19`), and its wounds show in how it moves (`PRE-44`).
    Animals are built the same way on their own body pattern, with wings or fins where they have them (`BIO-19`).
  - **Why:** Figures built from parts can play every animation from any side, and show each person's own body and clothes, without a new drawing for each.

- `PRE-28` **Readable from far away** *(Decided)*: As you zoom out, people become tiny outlined figures in their strongest colours, then groups become small markers, then a camp becomes a point that glows if it has a fire.
  - **How it works:** by its size on screen (tuned thresholds), a figure is drawn in full, then as a tiny outlined figure in its strongest colour; a group close together becomes one marker at its centre, and a camp a point at its hearth.

- `PRE-29` **From above** *(Decided)*
  - **What:** As the camera rises, it tilts toward looking straight down, and the land shifts into a clean map look: crisp colours for forest, grassland, rock and water, rivers as lines, shaded hills.
    Map overlays (`PRE-07`) sit on this view.
    At the very top, the whole world appears as a globe (`WLD-02`).
    Close up to globe is one continuous zoom (`PRE-03`).
  - **How it works:** as the camera rises past set heights, its tilt eases toward straight down and the land's drawing blends into the map look: each world cell in a flat colour for its plant cover (`WLD-12`), rivers as lines, and hills shaded from their heights.
  - **Why:** A landscape seen from high up at an angle turns to mush.
    A map stays clear at every height.

- `PRE-30` **Light, time and season** *(Decided)*
  - **What:** One master palette, with versions for each time of day (dawn, day, dusk, night) and each season.
    The sun casts real shadows, the sky tints everything, and distance adds haze.
    A fire lights its surroundings with a warm, flickering glow that fades with distance, warms the faces of people nearby, and sends up smoke and embers.
  - **How it works:** the sun's direction and height come from the time of day, the season (`TIM-18`) and the latitude (`WLD-01`), and the time of day and season pick the palette's version, blended through the changes; shadows come from the 3D scene, and haze grows with distance.
    Each fire is a light as bright as its heat (`MAT-18`), flickering as it burns and fading with distance, with smoke and embers from what it burns.

- `PRE-03` **Seamless zoom** *(Decided)*: One continuous zoom from the whole world, drawn as a globe, down to one person chipping flint.
  - **How it works:** one camera moves continuously from the globe through the world map, a region, a valley and a camp down to one person; the drawing changes with size on screen (`PRE-28`, `PRE-29`), and an area seen for the first time is made from the seed as you arrive, exactly as people will find it (`WLD-13`), so there is never a loading break.

- `PRE-04` **Sharp at every zoom** *(Decided)*: The pixel art stays sharp and readable at every zoom level: the art pixel never changes size (`PRE-22`), small things switch to forms that stay readable (`PRE-28`), and high views become the map (`PRE-29`).

- `PRE-31` **Visual review** *(Decided)*
  - **Done when:** at every milestone stage, screenshots at each zoom level, in both orientations and at every time of day, and short clips of people at work, pass a review for:
    - clean colour, with no speckled surfaces;
    - crisp silhouettes;
    - pixels that stay still while the camera is still or panning, and crawl as little as possible while it turns or zooms;
    - people and animals readable at phone size;
    - things that show what they are made of (`PRE-42`), and animations that read clearly as what they show (`PRE-44`).
  - **How it works:** a tool captures the screenshots and clips on the phone from a fixed set of saved worlds; the review checks them against the list, and you take part as the final judge (`PRC-10`).

### 11.2 Things and movement

- `PRE-42` **Built from their materials** *(Decided)*
  - **What:** Each thing is drawn from a prepared model whose parts take the colours and shapes of the materials actually used.
    A hut of birch poles and hides looks pale and brown; one of reeds looks straw-yellow; more poles make a bigger hut.
  - **How it works:** each named result (`MAT-21`) has one model, and each of its parts stands for one input of its blueprint (`MAT-04`), taking that input's colour ladder (`PRE-20`) and form (a pole, a hide, a bundle, a stone), with the amounts used setting its size and count; its icon, for cards and the book of ages, is drawn from the same model.
    Raw materials and found things are drawn at their real size and colour (`MAT-10`), and plants by their growth stage and season, from bud to bare (`WLD-31`).
    Wear, quality and timers show too: edges chip, bindings fray, a well-made thing looks even, and meat darkens as it dries (`MAT-19`, `MAT-20`).
  - **Why:** You can read a camp at a glance, and two routes to one result look as different as their materials (`MAT-07`).
  - **Done when:** every named result in the launch catalogue has its model, and one result made from two different materials looks clearly different at camp zoom.

- `PRE-43` **Variety** *(Decided)*: No two things look quite alike: each gets its own small differences in proportions, lean, wear and colour, and the style of the people who made it.
  Two huts in one camp differ a little; huts of two peoples differ more.
  - **How it works:** each thing's differences come from its own seed, within limits set for its model, so it looks the same every time you see it; its maker people's style (`CUL-12`) sets proportions and ornament, and shifts slowly as it is copied, so things can be dated by their look.
    Trees, bushes and rocks vary the same way (`WLD-31`).
  - **Why:** Variety makes a generated world believable instead of tiled.

- `PRE-44` **Animations** *(Decided)*
  - **What:** Every base action and everyday activity has its own animation, with variants, posed about 10 times a second.
    You can tell at a glance who is knapping, scraping a hide, carrying wood, tending a child or dancing.
  - **How it works:**
    - **One for each** of the 21 base actions (`MAT-06`) and of the everyday activities, such as walking, eating, sleeping, talking, playing, fighting, fleeing, teaching, singing and dancing (`BIO-21`), lasting as long as the activity does (`TIM-17`).
    - **Variants:** a few versions of each, shaded by who does it: a child or an elder, a limp from a wound (`BIO-13`), slumped in grief or quick in fear (`MND-19`), clumsy or skilled (`MND-06`), hunched in the cold, with small random differences so a crowd never moves in step.
    - **Animals:** each body pattern (four legs, wings, fins) has its own set, from grazing and resting to fighting, fleeing, swimming or flying (`BIO-19`).
    - **At speed:** each figure keeps showing its current activity at a steady pace, even when each stroke would be too fast to see (`TIM-01`).
  - **Why:** Lively, varied movement is what makes a camp feel alive.
  - **Done when:** every base action and everyday activity has an animation with at least two versions, shaded as above, that reads clearly at camp zoom (`PRE-31`).

### 11.3 On the screen

- `PRE-32` **World first** *(Decided)*
  - **What:** The world fills the screen.
    Controls and panels appear only when you ask for them: tap a person, animal, group, thing or place to open its card, or swipe up for the book of ages and other views.
    Nothing stays on screen unless you called it up, apart from a live moment appearing briefly (`PRE-08`) and talk bubbles at close zoom (`PRE-45`).
    A brief touch shows the date, the real speed of time and the time control (`PRE-33`), which fade after a few seconds (tuned).
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

- `PRE-35` **Cards** *(Decided)*: Selecting anything opens a card with what matters about it, and links to the deeper views.
  A person: name, age, people, mood, and what they are doing and why (`MND-09`).
  A thing: what it is made of, its characteristics marked by which ones people know, its wear and quality, and who made it and when (`MAT-10`, `MAT-20`).
  A place: its name and meaning, its land, and what happened there.
  A band or people: its name, numbers and territory, the crafts it knows with how many hold each, its beliefs, rites, calendar and festivals, its leaders, friends and enemies (`CUL-23`).

- `PRE-45` **What they talk about** *(Decided)*: Speech is a murmur, never real words (`SND-03`), so at close zoom a small bubble over the speaker shows a picture of the topic: a deer, fire, a face, a place.
  The speaker's card lists recent talk in plain words, such as "Ama told Tor where flint lies".
  - **How it works:** conversations are held as topics (`CUL-24`); bubbles show only at the person and camp zooms, and the card's lines come from the same records by fixed patterns, without the writer.

- `PRE-40` **Screens** *(Decided)*: Besides the world itself: a first-launch screen, a list of your worlds, and settings.
  Short help cards appear the first time you use something; there is no tutorial (`SCP-02`).
  - **How it works:** the first launch goes straight to making a world, offering the best candidates to choose from (`WLD-10`), and the list shows your worlds (`TIM-08`).
    Settings hold the content level (`PRE-18`), the live-moment level (`PRE-08`), the volume of music, voices and the world, and vibration (`SND-10`); each help card shows once, the first time its control is used.

### 11.4 Following the story

- `PRE-05` **Book of ages** *(Decided)*
  - **What:** The world's chronicle, written as history happens, with a timeline for each people (`CUL-23`).
    It records named discoveries and who made them (`MAT-21`), notable lives and deaths, peoples forming and splitting, feuds and alliances, disasters, villages, religions and festivals.
    Every entry is dated (`TIM-14`) and links to the people, places and things behind it.
  - **How it works:** events the recognisers mark as notable (`PRE-39`) become entries, grouped into ages that begin at turning points set by fixed rules and are named after their defining events (`PRE-41`); the writer words each entry from its records (`PRE-37`), with dark events stated as plain facts (`PRE-17`).
    Your own acts can be shown as separate marked lines, never in the text (`GOD-07`), and tapping one shows what came of it (`GOD-09`).
  - **Why:** It is the main way to read a world's history (`VIS-15`).
  - **Example:** "Year 112, autumn, day 6: Ama of the Tavu struck the first sharp flake by the river.
    They call it *kesh*, 'bite stone'."

- `PRE-06` **Follow a soul** *(Decided)*: Pick anyone and follow their life: the camera can stay with them, their card shows what they do and why (`PRE-35`), and the details of their mind are one tap away (`PRE-14`).
  The people you follow are kept in a list, separate from the camera, so you can follow several and still look elsewhere, and their notable moments come to you as live moments (`PRE-08`).
  When one dies, the game offers to follow those with the strongest bonds to them (`MND-24`).

- `PRE-07` **Map overlays** *(Decided)*: Information spread across the land, over the map look (`PRE-29`), always from the world as it is now:
  - where each people lives and travels (`CUL-23`);
  - who holds each blueprint (`MND-06`);
  - the share of people holding a belief, and how strongly (`CUL-26`), and their average mood (`MND-19`);
  - kin and marriage lines (`CUL-27`);
  - the land as a chosen person or people knows it (`MND-28`);
  - plants, water and stone people can use (`WLD-31`, `WLD-17`), and herds and tame animals (`WLD-32`, `WLD-33`);
  - the sick (`BIO-05`), and the weather and seasons (`WLD-16`);
  - surface rock and deposits (`WLD-14`).

- `PRE-08` **Live moments** *(Decided)*: Only what matters interrupts you: named discoveries, deaths of people you follow, disasters, and big turns in history ("the Tavu have made fire for the first time").
  What can interrupt is one shared list, scored for importance by the story director (`TIM-02`).
  At most about one interruption comes a minute, and the closer you are watching, the higher the score must be.
  Live moments you don't take wait in a list; everything else waits in the book of ages.
  The level can be adjusted in settings, and one tap takes the camera to the moment.

- `PRE-39` **Recognising what emerges** *(Decided)*
  - **What:** The game spots and names what is worth telling, for you only: named discoveries and other firsts, crafts lost and found again, peoples, religions, villages, leaders, feuds and alliances, disasters, and the ages of history.
    A first counts both worldwide and for each people, and a rediscovery after a loss is marked as one.
  - **How it works:**
    - **Named discoveries:** a people's first success with a blueprint is a named discovery (`MAT-21`), and the death of a craft's last holder marks it lost (`CUL-02`).
    - **Other firsts:** any event of a kind the world's or a people's history has never recorded, such as a first burial or festival.
    - **Peoples, religions, villages and ages** are found by fixed measures over the records (`CUL-23`, `CUL-26`, `CUL-28`), with thresholds set in the implementation plan and listed in stage reports.
  - **Why:** The story director (`TIM-02`), live moments (`PRE-08`), the book of ages (`PRE-05`), overlays (`PRE-07`) and the pace tests (`RES-07`) all need to know what happened and how much it matters.
  - **Check:** recognisers only read the world, and a code check finds no path from them back into it (`WLD-13`).

- `PRE-09` **Graves and old camps** *(Decided)*: The dead and the places people left stay in the world, and a list holds them all, by people and by date, so you can visit any of them.
  A grave shows who lies there, how they died, who buried them and what was laid with them; an old camp shows its hearths, rubbish heaps, lost tools and bones, and who lived there and when.
  - **How it works:** they are real places with real things in them (`MAT-08`), buried over time and seen in their layers with the cut-away (`PRE-25`); tapping a grave opens the dead person's card, with the cause of death (`BIO-14`) and a life story written from their records (`PRE-37`), and tapping a find shows who made or left it, and when.
    People in the world find old camps too, and can copy what they find there (`MND-11`).
  - **Example:** The dig in `MOM-09`.

- `PRE-10` **Family trees and legends** *(Decided)*: Family trees across generations, with marriages and lines of teaching, and the legends their culture keeps.
  Each legend can be set side by side with what really happened.
  - **How it works:** the true tree comes from the birth records (`BIO-15`) and marriages (`CUL-27`), and a line of teaching shows who taught each craft to whom (`MND-13`); the people's own remembered genealogies and legends (`CUL-15`) are shown beside it, each linked to the true events it tells of.

- `PRE-11` **Their sky and calendar (view)** *(Dropped)*
  - **Dropped because:** it did not earn its own screen; a people's calendar and festivals show on its card (`PRE-35`) and in the book of ages (`PRE-05`).

- `PRE-12` **Their maps and names (view)** *(Dropped)*
  - **Dropped because:** it did not earn its own screen; names with their meanings show on every card (`PRE-38`), what people know of the land is an overlay (`PRE-07`), and maps they draw show like any art (`PRE-15`).

- `PRE-14` **Details of a mind** *(Decided)*
  - **What:** For anyone, everything in their mind, shown plainly under a short summary in words.
  - **How it works:**
    - **Needs and mood:** each need (`MND-07`), and the mood with each thought that lifts or lowers it and for how long, beside their feelings (`MND-19`) and personality (`MND-20`).
    - **Memories**, most important first (`MND-18`), and **knowledge**: their mental map (`MND-28`) and the blueprints they know, with skill and experience by sector (`MND-06`).
    - **Beliefs**, each with how sure they are and the events behind it (`MND-05`); **plans and ambitions** (`MND-22`); **relationships** (`MND-24`).
    - **Reasons:** the top reasons for what they are doing now, and the options it beat (`MND-09`).
    - **The summary** is written from these records (`PRE-37`); your own acts on them, such as a dream you sent, are marked as yours (`GOD-09`).
  - **Why:** Every choice can be explained (`PRN-13`), and this is where you see how.

- `PRE-15` **Art that remembers** *(Decided)*: Tap a painting or carving to see it, who made it, and the event or myth it shows.
  You can then read what really happened, from the saved events (`PRN-15`).
  A map someone drew is shown beside the real land it describes (`CUL-14`).
  - **How it works:** a picture's record keeps what it shows and the memories or myths its maker drew on (`CUL-25`), and these link to the saved events and stories (`PRN-15`).

- `PRE-16` **Bestiary** *(Decided)*: A page for each of the world's plants and animals.
  - **How it works:** each page shows the species as it is (`WLD-31`, `WLD-32`): its look in each season, where and when it lives, its yields, how dangerous it is, and, for animals, their numbers and herds; tame and domestic kinds get their own pages (`WLD-33`).
    What each people calls it and believes about it shows once that people knows it (`CUL-18`).

- `PRE-36` **Language family tree** *(Dropped)*
  - **Dropped because:** the language no longer changes, so there is no family of languages to show.

- `PRE-13` **Few screens, everything findable** *(Decided)*: Anything the world keeps track of can be found from the views in this section, mostly on a card (`PRN-04`).
  A new screen is added only when no card, overlay or page of the book of ages can show something well.
  - **Check:** each stage review confirms that every kind of record the world keeps shows on at least one card or view.

### 11.5 Text written for you

- `PRE-17` **Descriptions stick to the data** *(Decided)*: The writer AI only turns records into text: the book of ages, life stories, myths, dreams and summaries.
  It never adds facts the world doesn't contain (`PRN-06`).
  - **How it works:** the writer receives only the records a text is about and the voice to use (`PRE-19`), and is told to phrase them and add nothing; every text is checked against those records before it is shown (`PRE-41`); and dark events are never left to it: they are stated as plain facts taken from the data.
  - **Check:** the fact checker runs on every text before it is shown, and a sample of texts is reviewed at each milestone stage for added or changed facts.

- `PRE-37` **The writer AI runs on the phone** *(Decided)*: All text is written by the phone's built-in writer AI, fully offline, with no running cost.
  If the writing turns out too plain for histories worth reading (`VIS-15`), that is raised at a stage review.
  - **How it works:** the phone's own built-in language model writes all text; dark events never go to it (`PRE-17`), and its instructions are tightened when the writer is built.

- `PRE-41` **How text is written** *(Decided)*: Text is written when it is first opened or while the phone is idle, checked, stored beside its records, and never silently rewritten; you can ask for a rewrite.
  The check compares every name, number, cause and event in a text with its records, with no language model involved, and a text that fails is replaced by plain factual text built from the records by fixed patterns.
  What the book of ages covers, and where its ages begin, come from fixed rules (`PRE-39`), not from the writer's taste.
  The writer chooses words and rhythm, never content: every claim, cause, motive, image and name must be in the data (`PRE-17`).

- `PRE-38` **English, with their names** *(Decided)*: The interface and the book of ages are in English.
  People, places, peoples, spirits and discoveries carry their names in their own language (`CUL-18`), shown with the English meaning where the name has one, such as *kesh*, 'bite stone'.
  Kinds of things are called by their English names, such as flint or birch bark, with each people's own word beside them on cards.
  - **How it works:** every name comes from the world's language (`CUL-17`), and its meaning from the words it was made from; a card shows both, and the book of ages gives the meaning the first time a name appears.

- `PRE-19` **Storytelling voices** *(To test)*: Documentary, archaeologist, their own tradition, and intimate.
  Each is tried on real worlds and chosen by reading, and different views may use different voices.
  In the first trial, the documentary voice read best and the tradition voice worst.
  - **How it works:** each voice is a fixed set of instructions to the writer (who speaks, tone and tense); the same real records are written in each voice, and you choose, view by view.

### 11.6 Content

- `PRE-18` **Content setting** *(Decided)*: You choose how much of history's darker side is shown, at one of three levels:
  - **Show:** everything, with pictures and sounds;
  - **Plain:** no graphic pictures or sounds, and factual text;
  - **Gentle:** dark events mentioned briefly, in the book of ages only.

  The world underneath never changes (`CUL-08`), and bodies are drawn without sexual detail at every level.
  - **How it works:** the recognisers tag how dark each event is (violence, injury, captivity, sacrifice or cruelty, `PRE-39`), and the setting filters only what the views show: everything; no graphic pictures or sounds, with injuries drawn without detail and text kept plain; or a brief mention in the book of ages only.

## 12. Sound

Sound comes in layers, added stage by stage, starting with the sounds of the camp.
Like everything you see, everything you hear reflects what is actually happening (`PRN-10`).

### 12.1 The layers

- `SND-01` **A lively camp** *(Decided)*
  - **What:** A camp sounds alive: murmuring voices, children playing and crying, the tap of stone on stone, scraping, chopping and grinding, the fire, dogs, and the animals around it.
    Zoom changes the mix: close up you hear single sounds; further out they blend into the hum of the camp; from the whole world, near silence.
  - **How it works:** every sound comes from something happening near the camera: a person's activity (`SND-06`), talk (`SND-03`), a fire by its heat (`MAT-18`), an animal's call (`WLD-32`), and the place around it (`SND-11`).
    Close up, the nearest and loudest sounds play singly, up to 32 at once, as measured on the phone; further out, sounds of one kind blend into one by how many there are and how loud; from the whole world, near silence (`SND-09`).
  - **Example:** At the camp at dusk: the crackle of the fire, the tap of the knapper's hammerstone, a child laughing, a dog barking at the dark, the murmur of talk, the river beyond, a wolf far off.

- `SND-11` **Ambience** *(Decided)*: Each place has its own background sound, set by its land, plants and water, the weather, the time of day and the season.
  Wind in grass or in pines, a river, the sea on the shore, rain on leaves, thunder, birdsong at a spring dawn, the hush of snow.
  - **How it works:** a small set of sounds for each kind of place is chosen by its plant cover and water (`WLD-31`, `WLD-17`), shaped by the weather as it changes (`WLD-16`), and changed by time of day and season (`TIM-18`); it fades as you rise toward the globe (`SND-09`).

- `SND-03` **The murmur** *(Decided)*: People talk in a murmur built from the sounds of their language, never in real words.
  It rises and falls with mood: quick and loud in anger, soft and slow in grief.
  Laughing, crying, calling and screaming come from the same voices, and what they talk about shows as pictures (`PRE-45`).
  - **How it works:** when someone talks (`CUL-24`), the murmur strings together the sounds and word shapes of their language (`CUL-17`) for as long as they speak, in a natural-sounding voice, chosen by your ear, that keeps as many of those sounds as it can.
    Each voice takes its pitch and tone from the speaker's age, sex and build (`BIO-08`), and its loudness, speed and tune from their feelings (`MND-19`); many voices blend into the camp's murmur, and further out into its hum (`SND-01`).

- `SND-02` **Their music** *(Decided)*: Songs, rhythms and instruments from each people (`CUL-10`), heard when you are near: at a festival, around the fire, at a burial.
  - **How it works:** each song is played from its record: its notes, rhythm and words, in its people's own scale (`CUL-25`).
    Instruments are things made from blueprints (`MAT-21`), each sounding by its own sound blueprint (`SND-06`): a longer flute or a bigger drum is lower, bone is brighter than wood, and a slack hide is duller than a tight one.
    Voices sing on the murmur's sounds (`SND-03`), and music is heard by distance like any sound (`SND-08`).
  - **Example:** At the midwinter festival (`CUL-29`), two bone flutes and a hide drum play the old hunting song while the camp dances.

- `SND-04` **Score** *(Decided)*: Background music made live from the world, from short themes that answer the time of day, the season, events and the people nearby.
  Once a people has music of its own, the score takes up its scales and rhythms.
  It is never the same twice, and you can turn it down or off (`PRE-40`).
  - **How it works:** short composed themes are chosen and varied live by the time of day and season, the story director's current moment (`TIM-02`), events such as a death or a first, and the mood of the people nearby (`MND-19`); once a people's own music exists (`CUL-10`), the score borrows its scales and rhythms.

- `SND-05` **Order of the layers** *(Decided)*: The first sounds and the murmur come with the fire stage (`MIL-03`); the camp and the land grow fuller at each stage after it; their music and the score come with the last stage (`MIL-07`).
  - **How it works:** each layer is a separate part of one mix, switched on when it is built, in this order.

### 12.2 How sound is made

- `SND-06` **Sound blueprints** *(Decided)*
  - **What:** Every sound comes from a small base set of short sounds, picked by what is happening and to what material, and adjusted by the things involved: harder is brighter, heavier and bigger is deeper and longer, wetter is duller.
    Random variation means no two sounds are quite the same.
  - **How it works:**
    - **The base set:** short sounds for each base action and everyday activity (`MAT-06`, `BIO-21`) on each class of material (stone, wood, bone, hide, plant, earth, water, flesh, metal), made by the game from noise shaped to each material, or taken from a small set of free-licence recordings.
    - **A sound blueprint** picks the base sound for an action on a class of material, and sets how the things' characteristics (`MAT-03`) and sizes (`MAT-02`) change it.
      Each blueprint's sound (`MAT-21`) is one of these, so a new route to a known result needs no new sound (`MAT-07`).
    - **Animals** call by species and size (`WLD-32`), and **instruments** sound by kind, size and material (`SND-02`).
    - **The phone's speaker:** a last step tuned to the speaker lifts the deep sounds it plays badly, and switches off with headphones.
  - **Example:** Flint struck on flint gives a sharp, bright click; the same strike on a wet log, a dull thud; a big granite block dropped, a deep, long crunch.

- `SND-07` **Sound follows time** *(Decided)*: At natural speed (`TIM-10`), every sound plays in real time.
  When time runs fast, single sounds give way to the feel of the period: seasons of wind and rain, the hum of a busy camp.
  - **How it works:** at natural speed each sound plays as its activity happens; faster, each kind of sound plays as a blend at the rate it is happening (`SND-01`), so the weather follows the seasons as they pass and a camp becomes its hum.

- `SND-08` **Space and distance** *(Decided)*: Sounds come from where they happen and fade and muffle with distance; caves echo.
  A sound can draw your attention to something off-screen, such as a scream or thunder.
  - **How it works:** each sound plays from its direction, quieter and duller with distance, muffled by land in between, and echoing in caves by their size (`PRE-24`); a loud sound off-screen plays from its direction.

- `SND-09` **Silence** *(Decided)*: Quiet is part of the design.
  Nights are hushed, deep snow muffles everything, and the whole world seen from above is close to silent.
  - **How it works:** quiet comes from the same rules: at night fewer things make sound, since people and most animals sleep; snow muffles what sound there is; and from high above only blends remain, fading with height; nothing adds sound where nothing happens.

- `SND-12` **Sound review** *(Decided)*
  - **Done when:** at every milestone stage that adds sound, recordings at each zoom, by day and night and in each season, pass your review on the phone's speaker and on headphones: sounds match what is on screen, no two strikes or steps sound exactly alike, voices sound like talk but never like real words, and nothing is harsh.
  - **How it works:** a tool records them on the phone from a fixed set of saved worlds, as for the visual review (`PRE-31`).

### 12.3 Touch

- `SND-10` **Vibration for big moments** *(Decided)*: Subtle and optional: thunder, an earthquake, the heartbeat of someone you follow when they are in danger.
  - **How it works:** when the setting is on, the phone's vibration plays a pattern for thunder near the camera, by its loudness; for a quake, by the shaking where the camera is (`WLD-15`); and for someone you follow, a heartbeat while their fear is high (`MND-19`).

## 13. Platform and performance

Kindling is built for one phone, and nothing else is used to play it (`SCP-02`).
The phone must stay smooth, cool and responsive (`VIS-14`, `PRN-11`).
How much fits on it is found by measuring, not guessing, and the pre-tests have already measured the phone itself.

### 13.1 The phone

- `PLT-01` **One phone** *(Decided)*
  - **What:** Built and tuned for your Pixel 11 Pro XL (16 GB of memory, 512 GB of storage), and free to use its own hardware wherever it helps: its graphics chip for the picture (`PRE-02`), its built-in AI for the text (`PRE-37`), and either for the simulation where that helps.
  - **Measured in the pre-tests:**
    - **Cores:** 2 small, 4 middle and 1 fastest.
      The middle ones do the most work for the battery they use; the fastest is 1.7 times as fast as a middle one, at twice the energy per step.
    - **Held speed:** under full load on every core, speed settles within 2 minutes to about 43% of a short burst and holds there, at about 3 W, with the phone cool and using about 15% of the battery an hour.
      The game plans on that held speed, not the burst.
    - **Memory:** the app could take 10 GiB, but only at the edge, with the phone's free memory falling under 1 GB, so the game plans on about 8 GiB.

- `PLT-02` **Portrait and landscape** *(Decided)*: Both are supported, and the layout adapts (`PRE-34`).
  - **How it works:** turning the phone switches between the two layouts of `PRE-34`, keeping the world, the camera and the art pixel's size (`PRE-22`).

- `PLT-03` **Works offline** *(Decided)*: Everything works without a connection, including the text, which the phone's built-in writer AI writes (`PRE-37`).
  - **How it works:** everything the game needs is on the phone: the game itself, its catalogues, the writer AI and every sound; nothing in play makes a network call.

- `PLT-06` **Installing new versions** *(Decided)*: Each new version is a file you download on the phone and install, after allowing installs from your browser once.
  Builds are signed for your free hobbyist developer account with Google, so they keep installing this way under Android's developer rules from 2027 (`RSK-18`).
  No store and no fees.
  - **How it works:** each alpha's build is signed for your developer account and linked from its note (`PRC-11`); you download and install it, and your worlds carry on (`PLT-09`).

### 13.2 Performance

- `PLT-04` **Measured limits** *(To test)*
  - **What:** Measured from the first alpha and reported at every stage (`RES-06`), against these targets:
    - **Smooth:** zooming and panning at the screen's full 120 frames a second (`VIS-14`); in the pre-tests, the settled look kept 97–100% of frames on time at every zoom, using less than half of each frame's time.
    - **Speed:** at least 1 game year per real minute for 1,000 people at the world view, aiming for 2–10 (`TIM-07`).
    - **People:** up to about 2,000 at a watchable speed; beyond that, time slows (`MND-15`).
    - **Memory:** within about 8 GiB (`PLT-01`).
    - **Battery and heat:** an hour's play uses about 25–30% of the battery, and the phone never gets uncomfortably hot (`VIS-14`).
    - **Opening:** the app opens to your world in about three seconds; in the pre-tests, a large world's save would open in about 0.8 seconds.
    - **Making land:** a new world in a few minutes (`WLD-11`), and a new area quickly enough that nobody waits; in the pre-tests, a square kilometre of ground detailed to the metre took about a third of a second.
  - **How it works:** the cloud runs the speed benchmark at every alpha, to catch slowdowns early (`PLT-05`).
    At each stage, the build has a benchmark you start with one tap: it runs fixed saved worlds at each zoom for a few minutes, reads battery and temperature from the phone's own counters, and shows a short result you send back.

### 13.3 Worlds on the phone

- `PLT-07` **Always saved** *(Decided)*: Worlds save continuously, so closing the app or a flat battery never loses anything (`TIM-05`).
  - **How it works:** each event joins the world's history as it happens; on your phone, a small addition took under a tenth of a millisecond.
    The present state is saved whenever the app leaves the screen and at set intervals, each time as a new copy that replaces the old only once complete, so a damaged save is never loaded: in the pre-tests, 1,000 kills mid-save never left one, and a quarter of a large world saved in 0.41 seconds.
    After a crash, the world reopens at its last save and runs forward to where it stopped, repeating exactly (`TIM-16`).

- `PLT-08` **Manual export** *(Decided)*: Export a world, with its present state and its book of ages, as a file whenever you want, and import it on the same phone or a new one.
  The export keeps the full record, so an imported world opens exactly as it was.
  There are no automatic backups.
  - **How it works:** the export holds everything the world keeps (`TIM-08`): its seed and generator version, its present state, the areas people have changed, its history and its written text; import checks the file before opening it.

- `PLT-09` **Worlds across updates** *(Decided)*
  - **What:** The game keeps growing, alpha by alpha (`PRN-14`).
    After a small update, such as fixes, tuning or new blueprints, a world carries on: everything that already happened stays as it was, the world continues under the new rules, and the change is marked in its book of ages.
    A big update, one that changes how worlds are generated or adds new kinds of plants, animals or materials to the land, may need a new world.
    Worlds are only promised to last between big updates.
  - **Why:** Fitting new land or species into a running world would be costly, and could make its past dishonest.
  - **How it works:** each world records the version of the rules it runs under, and each update declares itself small or big; after a big update, an older world's book of ages can still be read, but carrying the world on needs a new one.

- `PLT-10` **Storage** *(Decided)*: Each world keeps its present state, the areas people have changed, and its history (`PRN-15`); unchanged areas are remade from the seed when needed (`WLD-13`).
  The history thins with age by a fixed rule: recent years keep every event, and older years keep what the book of ages and the views use, such as births, deaths and firsts.
  You can delete worlds.
  When the phone nears full, the game warns you and asks what to delete; it never deletes anything by itself.
  - **Why:** Kept whole, the history of 1,000 people would take about 8 GB or more every thousand game years, going by the size of an event measured in the pre-tests.
  - **How it works:** the thinning rule is tuned by measurement (`PLT-04`), and a storage check warns before the phone fills.

### 13.4 The cloud

- `PLT-05` **Tests in the cloud** *(Decided)*: The game also runs without picture or sound in the AI's cloud sessions, several worlds at a time, for the tests (`RES-21`, `RES-07`).
  Follows from `SCP-15`.
  - **What:**
    - The phone build comes first; the cloud build must give the same results, at least statistically (`RES-05`), and in the pre-tests it gave exactly the phone's results.
    - A test's world can be opened on the phone as it stands at the end of its run; it is marked as a test world and shows any switch it used (`RES-10`).
    - You can ask for any test or run in a cloud session, and get its report as a page (`RES-15`).
  - **Measured in the pre-tests:** a cloud session gives about 3.4 cores of steady computing; four worlds side by side ran nearly four times as fast as one; and a run stopped and resumed ended exactly like one that never stopped.
  - **How it works:** the same game, built without picture or sound, runs several worlds side by side, saves its progress often so an interrupted run carries on where it was, and writes the same saved worlds the phone opens (`TIM-08`).

## 14. Testing

How the game shows it does what this file says, alpha by alpha: automated tests, small sandbox scenes, pace tests on whole worlds overnight, speed benchmarks on the phone, and your reviews at each stage.
Testing is sized to what fits in the AI's cloud sessions (`SCP-15`, `PLT-05`).

### 14.1 How testing works

- `RES-01` **Tests lead** *(Decided)*
  - **What:** Every alpha comes with automated tests for what it adds.
    The quick ones run before any work joins the main version, and the long ones overnight (`PRC-10`).
    A feature counts as built only when its tests pass, and every test names the IDs it checks.
  - **Why:** A world where things emerge breaks in quiet ways, such as a craft that is never passed on; tests catch it while the change is still small.
  - **Check:** the coverage check finds a test for every feature and rule built so far (`PRC-12`).

- `RES-21` **Scenes, then whole worlds** *(Decided)*
  - **What:** Most tests run in sandbox scenes: small settings built for one check, such as a band on a riverbank with flint, granite and decoy stones in reach, or a winter camp whose fire is dying.
    Only a scene's setting is chosen (`RES-18`), and it includes decoys and things nobody designed for.
    Scenes are quick and repeat exactly from their seed (`TIM-16`), so each check can run many times.
    Every scene also checks that nobody used a blueprint they didn't know or a fact they hadn't seen (`PRN-01`).
    Whole worlds from the play generator (`WLD-10`), run overnight in the cloud, then confirm that what scenes show also happens in real play; until whole worlds exist (`MIL-04`), the start region and all its bands stand in for them.
  - **Why:** A scene answers one question in minutes, while whole worlds guard against a scene so well arranged that it makes its result likely by design.
  - **Check:** each stage report names, for every result, its scenes and its whole-world confirmation (`RES-06`).

- `RES-18` **Same rules as play** *(Decided)*: Scenes use the same rules, minds and catalogues as play, and the worlds that confirm them come from the play generator (`WLD-10`).
  Nothing in a scene is scripted.
  A result that needs a switch (`RES-10`) or a scripted event doesn't count as passing in play (`PRN-12`).
  - **Check:** scenes and play run on one and the same game, and every run records any switch it used.

- `RES-09` **Pass rules come first** *(Decided)*: Every test states, before it first runs, what it checks (by ID), its scene or worlds, its number of runs, and its pass rule in exact numbers.
  A pass rule is never loosened in the change that makes it pass.
  Loosening one needs a stated reason and the independent reviewer's OK (`PRC-09`), and the pace windows change only with you (`TIM-19`).
  - **Why:** The easiest way to make a failing test pass is to weaken it.
  - **Check:** the review sees every change to a pass rule, with its reason.

- `RES-13` **About 20 runs where chance matters** *(Decided)*: Where chance decides the outcome, a check runs about 20 times, each from a different seed, and its pass rule counts runs, such as "in at least 16 of 20".
  Results are reported as ranges, such as "fire made in 18 of 20 worlds, typically around year 15".
  More runs are used only where 20 can't tell pass from fail, and fewer only when they don't fit, which the report then says.
  - **Check:** every result in a report states its number of runs and its range.

- `RES-05` **Repeatable runs** *(Decided)*: On the same build, a run from the same seed gives the same result every time, so any failure can be replayed and examined step by step (`TIM-16`).
  The cloud must give the same results as the phone, at least statistically (`PLT-05`); in the pre-tests it gave exactly the same.
  - **Check:** at every stage, the phone benchmark's fixed worlds (`PLT-04`) also run in the cloud and the results are compared; a difference beyond the stated tolerance fails the stage (`PRC-10`).

- `RES-10` **Switch-off runs** *(Decided)*: To find out what a result depends on, a scene can run with one thing switched off, such as teaching, copying, dreams or a personality trait.
  They show what is missing when a pace test or a moment fails (`RSK-01`), and whether a mechanism earns its cost.
  Switches exist only in tests (`PRN-12`), and a test world that used one always shows it (`PLT-05`).

- `RES-16` **Tuning the pace** *(Decided)*
  - **What:** The pace is tuned only by changing chances and amounts (`PRN-17`), such as how likely an accident is to be noticed, how much spare time people have, or how fast experience grows.
    Tuning uses a fixed set of tuning seeds, and the pace tests then confirm on fresh seeds never used for tuning (`RES-07`).
    Every tuned value is logged with what it was tuned against.
    If fresh seeds fail where tuning seeds passed, tuning goes on; if it can't fix the pace, the stage report sets out the options for you: a redesign, another window, or accepting it.
  - **Why:** Tuning until the same worlds pass would only fit those worlds.
  - **Check:** the tuning log lists every tuned value, and the pace tests' seeds never appear in it.

- `RES-04` **Reality checklist first** *(Dropped)*
  - **Dropped because:** the reality rules are now checks on the blueprint catalogue (`MAT-17`), run before any work joins the main version (`PRC-10`).

- `RES-08` **What every experiment has** *(Dropped)*
  - **Dropped because:** merged into `RES-09`, which says what every test states.

- `RES-11` **Independent review** *(Dropped)*
  - **Dropped because:** merged into `PRC-09`, whose independent review covers every change, tests included.

- `RES-20` **Your own experiments** *(Dropped)*
  - **Dropped because:** merged into `PLT-05`: you can ask for any test or run in a cloud session.

### 14.2 The tests

- `RES-23` **Every blueprint and behaviour has a scene** *(Decided)*: Each blueprint has a scene in which someone who knows it, with suitable things in reach, makes its result in about its expected time and succeeds about as often as its chances say (`MAT-04`).
  Each chain is also run end to end, such as flake, scraper, scraped hide, dried hide, sewn clothing.
  Everyday behaviour has scenes too: a thirsty person finds water, a cold band keeps its fire alive, a mother feeds her child, a band flees a predator.
  - **Check:** the coverage check finds a scene for every blueprint in the catalogue (`PRC-12`).

- `RES-02` **The sharp-stone test** *(Decided)*: Does a band that has never made a sharp flake discover how, and does the skill spread?
  - **What:** A band with the starting kit (`BIO-02`) by a river, with flint in reach among granite, sandstone and other decoy stones, nuts to crack, carcasses to butcher, and hides and wood to work.
    Nobody in it knows how to make a flake.
    It runs 20 times, for up to 5 game years each.
    A second scene, the same but with no stone that flakes, is the control.
  - **How it works:** the year of the first flake is read from the book of ages (`MAT-21`), and who can make flakes from each adult's skills (`MND-06`).

- `RES-03` **Sharp-stone pass rule** *(Decided)*: Fixed before the test first runs (`RES-09`):
  - **Discovery:** flakes are discovered within 5 game years in at least 16 of 20 runs.
  - **Spread:** in those runs, at least 3 in 4 of the band's adults can make flakes within 2 game years of the first.
  - **Routes:** across the runs, at least two routes of discovery appear (`MND-11`), such as an accident while cracking nuts and deliberate experimenting.
  - **Control:** without stone that flakes, no run ever makes a flake (`RCK-01`).

- `RES-07` **The pace tests** *(Decided)*: One check for each pace target (`TIM-19`), added at the stage that delivers its step (`SCP-16`).
  - **What:** About 20 whole worlds from the play generator (`WLD-10`) run overnight in cloud sessions, for up to 500 game years or as far as the stage's steps need.
    The year each world first reached each step is read from its book of ages (`MAT-21`).
    They run on the latest alpha on any night after the minds, the blueprints or the catalogues changed, and always before a stage closes (`PRC-10`).
    If 20 worlds don't fit in a night, the test runs over more nights or with fewer worlds, and the report says which (`RES-13`).
  - **Pass rule,** for each step:
    - **in the window:** at least half the worlds reach it inside its window;
    - **not too soon:** at most a quarter reach it before its window opens;
    - **not always the same:** where windows overlap, the steps don't come in the same order in every world.
  - **Example:** pottery passes if at least 10 of 20 worlds first fire a pot between game years 60 and 150, and no more than 5 before year 60.
  - **Why:** The pace is what makes history watchable (`PRN-17`), and only whole worlds show it.

- `RES-17` **Signature moments keep happening** *(Decided)*: Each signature moment (`MOM`) has its own scene, and passes if it happens in at least 2 of 20 runs within its time window, unless its own rule says otherwise.
  Each moment's scene runs at the stage it belongs to, and again whenever something it depends on changes; the short ones run before any work joins the main version (`PRC-10`).
  - **Check:** each stage report gives every moment's latest result, and which moments appeared in the whole worlds (`RES-06`).

- `RES-19` **Every promise has a test** *(Decided)*: Everything this file says will arise in play rather than be built directly, such as a taboo, a religion, a feud, a village or a lost craft, gets a scene or a whole-world check by the stage that delivers it, or is marked "possible, not promised".
  - **Check:** the coverage check lists each such promise with its test or its mark (`PRC-12`).

- `RES-14` **Believable outcomes** *(Decided)*: Whole worlds are checked against plausible ranges from real hunter-gatherers and early farmers: band sizes, births and deaths, life spans, how fast numbers grow, how far bands travel and how much they eat.
  The ranges are set where each subject is described, such as in People: bodies and lives.
  A world far outside them is a bug to look into, not a finding (`PRN-02`).
  - **How it works:** the overnight worlds report these measures, and each stage report shows them against their ranges (`RES-06`).

- `RES-12` **Oddities are flagged** *(Decided)*: Whole-world runs flag anything out of the ordinary: a step far outside its window, a result out of order (a pot before any fire), people starving beside plenty, someone stuck repeating one action, or a thing from nothing (`MAT-09`).
  They also flag anything that breaks over long play, such as a crash, memory creeping up, or a save that won't reopen.
  Each is looked into: a bug is fixed with a test that would catch it again, and a good surprise can become a new signature moment (`MOM`).
  - **How it works:** the overnight runs compare every world with its expected ranges and with simple "never" rules, and list what they find in the report (`RES-06`).

### 14.3 Your reviews and reports

- `RES-22` **Your reviews** *(Decided)*: At each stage, you play the latest alpha and judge what tests can't: whether it looks right (`PRE-31`) and sounds right, feels lively and believable, keeps a watchable pace, and has a book of ages worth reading (`VIS-15`).
  Work goes on meanwhile, and the stage closes once you have reviewed it (`PRC-10`).
  - **How it works:** the stage report ends with a short list of what to look at and try, with saved worlds that show it; you answer in a few lines, and what you find goes into the next alphas.
  - **Check:** each closed stage records your review.

- `RES-06` **Stage reports** *(Decided)*: Every stage ends with a short report for you, covering:
  - what was added, and what you can now see and try;
  - the test results with charts, above all the pace (`RES-07`);
  - the phone measurements (`PLT-04`);
  - the signature moments seen, the oddities and the surprises (`RES-17`, `RES-12`);
  - how each principle's check came out (`PRN-16`);
  - the risks (see Risks);
  - what needs your judgement (`RES-22`);
  - links to the worlds and book-of-ages entries it talks about.
  - **How it works:** it is built from the test results, the measurements and the coverage check, checked by an independent AI reviewer (`PRC-09`), and published as a page (`RES-15`).

- `RES-15` **A page on the phone** *(Decided)*: Each report is a short, readable page with charts and plain conclusions, whose links open its worlds and book-of-ages entries in the game.
  A copy is kept in the repository.

## 15. Project and process

How the project is run: you direct, and AI agents build the game in playable alphas that reach your phone.
This section sets out the roles, the documents, how this file changes, and how work flows from an idea to your phone.

### 15.1 Roles

- `PRC-01` **Passion project, built by AI** *(Decided)*: You direct; AI agents write, test and review the code.
  There are no running costs beyond the AI sessions themselves, since the writer AI is built into the phone and there is no store.

- `PRC-02` **Your role** *(Decided)*: You play the alphas when you like, review each stage (`RES-22`), set direction, and approve changes to this file.
  The AI handles building, testing and code review.
  - **Check:** every change to this file names your OK in its commit (`PRC-07`).

- `PRC-03` **Technology** *(Decided)*: Chosen by the AI from what the pre-tests measured (`PRC-08`), and set out in the architecture for your approval.
  - **Check:** the architecture's technology proposal records your approval before building starts.

### 15.2 Documents

- `PRC-04` **Three documents** *(Decided)*: The finished project has three documents.
  This file is the source of truth for what to build; the architecture says how it is built; the implementation plan says in what order, mapping every item to a stage and its alphas, with their tasks.
  Code and tests link back here by ID.
  - **Check:** the repository holds these documents, and each stage review checks that this file holds no implementation details.

- `PRC-06` **A guide for AI agents** *(Decided)*: A short file in the repository (`CLAUDE.md`) that every AI agent reads first.
  It tells them to read this file, follow the principles, link all work to IDs, and never mark anything Decided without you.
  - **How it works:** the guide sits at the top of the repository, where every agent's session reads it first, and changes to it need your OK.

- `PRC-07` **Changes to this file** *(Decided)*: AI agents can suggest additions or changes, marked *Proposed*.
  Nothing becomes *Decided*, and no decided item changes, without your OK.
  How changes are proposed and recorded is set out in How this file works.
  - **Check:** the commit check confirms that every commit changing this file names the changed IDs and why, and that no item became Decided without your OK.

- `PRC-05` **Reviewed with you** *(Decided)*: Changes to this file are worked through with you, section by section or in rounds of questions, and *Proposed* items are confirmed, changed or dropped in those reviews (`PRC-07`).
  - **Check:** every change to this file names, in its commit, the review or instruction from you that it came from.

- `PRC-08` **Next: the architecture and the plan** *(Decided)*: The pre-tests are done.
  Small throwaway tests settled the basic technical choices, such as the language, storing data, the map, drawing, sound, speech and the writer AI, and measured what the phone can sustain.
  The architecture comes next, starting with the technology proposal (`PRC-03`), then the implementation plan, starting with the first stage (`MIL-01`).
  What the pre-tests found moves into the architecture, and the pre-test folder is deleted once the architecture is written.

### 15.3 How work flows

- `PRC-09` **Branches, checks and review** *(Decided)*: AI agents work on separate branches.
  Work joins the main version only after every automatic check passes (`PRC-10`) and an independent AI review approves it.
  The reviewer is a separate agent, not the one that did the work: it reads the change and its tests, looks for flaws, and checks that no test was weakened to pass (`RES-09`).
  You review at each stage (`RES-22`).
  - **Check:** the main version accepts work only from branches whose checks passed and whose review approved them, and each review names a reviewer other than the builder.

- `PRC-10` **The checks** *(Decided)*
  - **Before any work joins the main version:**
    - the quick tests (`RES-01`) and the short scenes, the signature moments' included (`RES-23`, `RES-17`);
    - the catalogue checks, reality rules included (`MAT-17`, `RCK`);
    - the file check: every ID defined once, every reference resolving, every status valid, and no live item citing a dropped one.
  - **Before a stage closes:** the pace tests (`RES-07`), the phone measurements (`PLT-04`), the phone and cloud match (`RES-05`), the signature-moment scenes that are due (`RES-17`), the coverage check (`PRC-12`), the report (`RES-06`) and your review (`RES-22`).
  - **How it works:** the checks run by themselves on every request to join the main version and when a stage closes, and any failure blocks it; your review is the last step of a stage.

- `PRC-11` **Each alpha reaches your phone** *(Decided)*: Every playable alpha (`SCP-03`) ends with a build you can install and play on the phone (`PLT-06`), with a short note: what is new, what to try, and what is still rough.
  You play it when you like; only the stage reviews wait for you (`RES-22`).
  - **How it works:** each alpha's build is signed, linked from its note, and opens your existing worlds (`PLT-09`).
  - **Check:** every alpha's note links its build and names the IDs it delivers.

- `PRC-12` **Nothing gets lost** *(Decided)*: An automatic coverage check, run when each stage closes (`PRC-10`), confirms that:
  - every feature and rule that isn't *Dropped* or *Proposed* is mapped to a stage in the implementation plan, and the current stage's items have tasks;
  - every task names the IDs it delivers, and every ID named in code and tests exists and isn't dropped;
  - every feature and rule built so far has a test (`RES-01`), every blueprint a scene (`RES-23`), and every promise a test or a "possible, not promised" mark (`RES-19`).
  - **How it works:** it reads this file's IDs and statuses, the plan's map of items to stages and tasks, and the IDs named in code and tests, and fails on anything missing.

## 16. Risks

What could stop Kindling from succeeding, how we would notice early, and what we do about it.
Each risk has a rating (likelihood and impact), the early signs to watch for, and a response.
Every stage report reviews them all (`RES-06`), and AI agents may update the ratings there.

### 16.1 The game itself

- `RSK-01` **Discoveries stall** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** People may rarely find blueprints by accident, experiment, dream or copying, or find them and fail to pass them on, so history stops at the first steps.
  - **Signs:** the sharp-stone test failing (`RES-03`); pace tests in which worlds never reach fire or clothing (`RES-07`); skills that never spread beyond whoever found them.
  - **Response:** the sharp-stone test comes first (`MIL-02`); switch-off runs show which mechanism is missing (`RES-10`); and the pace is tuned by chances and amounts, never by scripting (`PRN-17`, `RES-16`).

- `RSK-26` **The pace is off** *(Decided)*
  - **Rating:** likelihood high, impact high.
  - **Risk:** Discoveries come far too fast or too slow, or always in the same order, so history becomes a rush, a long wait, or the same every time.
  - **Signs:** pace tests outside their windows (`RES-07`); every world reaching each step in nearly the same year and order.
  - **Response:** pace tests on whole worlds from the early stages (`RES-07`); tuning on separate seeds (`RES-16`); several routes to each result (`MAT-07`) and varied worlds to keep histories apart; windows that prove wrong are changed with you (`TIM-19`).

- `RSK-19` **Belief fails to emerge** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** Linking strong outcomes to what came before may give no beliefs worth noticing, or only noise: taboos nobody keeps, rites that never settle, no religion (`CUL-05`, `CUL-26`).
  - **Signs:** belief scenes failing (`RES-19`); whole worlds with no shared belief, rite or sacred place after a hundred game years; every people believing the same things.
  - **Response:** belief templates (spirits, ancestors, taboos, rituals, offerings) give the links a shape (`CUL-05`); scenes for each kind of belief, and for your lightning becoming a god (`MOM-03`), from the stage that brings minds and beliefs (`MIL-05`); tuning how strongly outcomes are linked and remembered.

- `RSK-06` **Blueprints give absurd results** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Generic blueprints (`PRN-07`) match things nobody planned for and give nonsense, such as an axe of bark, pots from sand or fire from wet moss.
    Values set by hand (`MAT-05`) can be off too, such as a hide warmer than fur.
  - **Signs:** catalogue checks failing (`MAT-17`); oddities in whole-world runs (`RES-12`); your reviews spotting things that look wrong.
  - **Response:** reality rules on the catalogue (`RCK`), checked on every change (`PRC-10`); narrower characteristic ranges where a blueprint matches too much; every oddity fixed with a test that would catch it again.

- `RSK-07` **People know what they can't** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** Choices may use what people can't know, such as a blueprint nobody taught them or a place they never saw, or the writer AI may slip our own knowledge into the text, so discoveries stop being theirs and the game feels scripted.
  - **Signs:** scenes finding a choice that used an unknown blueprint or an unseen fact (`RES-21`); steps reached suspiciously fast; texts with facts the records don't hold.
  - **Response:** `PRN-01` and `PRN-06`, checked in every scene (`RES-21`), by the writer's fact check (`PRE-17`) and by the independent review (`PRC-09`).

- `RSK-27` **People act oddly** *(Decided)*
  - **Rating:** likelihood high, impact medium.
  - **Risk:** Scored choices can go wrong in ways you notice at once: people dithering between two tasks, starving beside food, all doing the same thing at the same moment, or walking into danger.
  - **Signs:** oddities flagged in whole worlds (`RES-12`); reasons for a choice that make no sense (`PRN-13`); your reviews.
  - **Response:** scenes for everyday behaviour from the first camp (`RES-23`, `MIL-01`); every choice keeps its reasons, so odd ones can be traced (`PRN-13`); each fix comes with a test.

### 16.2 The experience

- `RSK-03` **Real but dull to watch** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** A believable world can still hide its best stories, or have long stretches where nothing seems to happen.
  - **Signs:** in your reviews, you skim the book of ages; few live moments; long quiet stretches.
  - **Response:** a watchable pace (`PRN-17`); the story director and live moments (`TIM-02`, `PRE-08`), the book of ages (`PRE-05`) and following someone (`PRE-06`) bring the stories out; every stage review judges whether they do (`RES-22`).

- `RSK-08` **Writing too plain** *(Decided)*
  - **Rating:** likelihood high, impact medium.
  - **Risk:** The phone's built-in writer (`PRE-37`) writes flat, repetitive text, so the book of ages isn't worth reading (`VIS-15`).
  - **Signs:** entries that read alike; in the pre-tests, both models copied most of their phrasing straight from the data, and you rated half their texts acceptable.
  - **Response:** rich records for the writer to draw on; the documentary voice, which you rated acceptable or good in 5 of 6 pre-test texts (`PRE-19`); tighter instructions; and if it still falls short, options at a stage review.

- `RSK-17` **The writer AI softens dark history** *(Decided)*
  - **Rating:** likelihood high, impact low.
  - **Risk:** The writer may refuse or soften violence, raids or sacrifice (`CUL-08`); in the pre-tests, both models softened forced labour in a raid.
  - **Signs:** vague or missing entries for dark events.
  - **Response:** dark events are never left to it: they are always stated as plain facts from the data (`PRE-17`).

- `RSK-11` **Pixel look hard to keep clean** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Keeping pixel-rendered 3D free of speckle and shimmer at every zoom may be harder than it looks.
    Pixels still crawl while the camera turns or zooms, and the one fix that stopped it in the pre-tests didn't look right to you.
  - **Signs:** visual reviews failing on speckled surfaces, crawling pixels or unreadable figures.
  - **Response:** the fix is chosen on a real world at the first visual review (`PRE-22`, `PRE-31`), and the look is checked at every stage.

- `RSK-28` **Sound falls flat** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** A lively camp needs good sound (`SND-01`), but the phone's speaker loses deep sounds, and in the pre-tests you found the synthetic voice terrible and the drums weaker than the flutes.
  - **Signs:** a camp that sounds thin or fake in your listening reviews.
  - **Response:** a last step that lifts deep sounds on the speaker; the murmur chosen by ear (`SND-03`); sound blueprints tuned with you (`SND-06`); listening at every stage review (`RES-22`).

### 16.3 The phone

- `RSK-02` **Too slow at 2,000 people** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** A full mind for every person (`MND-14`), each doing 10–30 activities a game day, may be too slow.
    The target is at least 1 game year per real minute for 1,000 people (`TIM-07`), and a watchable speed up to about 2,000 (`MND-15`).
  - **Signs:** the benchmark's game years per minute falling below target as people multiply (`PLT-04`); the phone getting hot; overnight runs covering only a few dozen years.
  - **Response:** measure from the first alpha, in the cloud at every alpha and on the phone at every stage (`PLT-04`); keep each choice cheap by weighing only what is in reach and known; time slows rather than detail being cut (`PRN-11`); if needed, the population limit is set lower by measurement (`MND-15`).

- `RSK-15` **The memory limit** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** About 8 GiB must hold the people, the areas in use, the world cells and the picture (`PLT-01`); the pre-tests showed 10 GiB is reachable, but only at the edge.
  - **Signs:** the phone closing the app; areas forgotten and remade too often; fewer people than planned.
  - **Response:** measure memory per person and per area from the start (`PLT-04`); unchanged areas are forgotten and remade from the seed (`WLD-12`); history is kept in storage, not in memory (`PRN-15`).

- `RSK-20` **Saved worlds grow too large** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Long histories and every area people have changed may fill the phone (`PRN-15`).
  - **Signs:** worlds growing by gigabytes every thousand game years.
  - **Response:** measure early; thin old events with age by a fixed rule; ask before deleting anything (`PLT-10`).

- `RSK-12` **Updates change worlds in odd ways** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** A world that carries on under new rules (`PLT-09`) may change suddenly at the point of the update.
  - **Signs:** sudden jumps in a world's state just after an update.
  - **Response:** history before the update is kept and the change is marked (`PLT-09`); every check runs before each new build (`PRC-10`).

- `RSK-21` **Losing a world to a bad update** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** A bug in an update, or a damaged save, could make a world unreadable, and there are no automatic backups (`PLT-08`).
  - **Signs:** worlds failing to open after an update.
  - **Response:** a safety copy before any update touches a world, and tests that open worlds saved by earlier alphas.

- `RSK-22` **The built-in writer changes** *(Decided)*
  - **Rating:** likelihood medium, impact low.
  - **Risk:** The writer AI belongs to the phone's system, not to the game (`PRE-37`), so a system update could change how it writes, or take it away.
    Speed is not the worry: in the pre-tests it began in a quarter of a second and wrote 77 words a second.
  - **Signs:** texts changing in style after a phone update; the writer missing.
  - **Response:** text is stored once written, so old entries never change (`PRE-41`); without the writer, plain factual text is shown, and the game plays the same.

- `RSK-04` **Cloud tests drift from the phone** *(Decided)*
  - **Rating:** likelihood low, impact high.
  - **Risk:** The phone build is tuned on its own (`PLT-05`), so cloud tests may stop showing what happens on the phone.
  - **Signs:** the stage comparison finds different results on the phone and in the cloud (`RES-05`).
  - **Response:** one game for both builds, and the comparison at every stage (`RES-05`); in the pre-tests the cloud gave exactly the phone's results.

- `RSK-18` **New install rules** *(Decided)*
  - **Rating:** likelihood medium, impact low.
  - **Risk:** From 2027, certified Android phones require apps from registered developers, which affects installing by download (`PLT-06`).
  - **Signs:** installs blocked or warned against.
  - **Response:** builds are signed for your free hobbyist developer account (`PLT-06`); the one-off advanced unlock and a USB cable remain as fallbacks.

- `RSK-24` **The phone ages or is replaced** *(Decided)*
  - **Rating:** likelihood low, impact medium.
  - **Risk:** The game is built for one phone (`PLT-01`), which will age, break or be replaced.
  - **Signs:** battery wear; a new phone.
  - **Response:** worlds move by export (`PLT-08`), and moving to a new model is planned with you.

### 16.4 The project

- `RSK-25` **Too much content** *(Decided)*
  - **Rating:** likelihood high, impact high.
  - **Risk:** About 200 items, 150 blueprints, 60 plants, 30 animals and 15 illnesses, with their models, animations, icons and sounds, take far longer to make and check than planned.
  - **Signs:** stages slipping on catalogue work; blueprints waiting for models or sounds; entries that are thin or all alike.
  - **Response:** each stage adds only the content its steps need (`SCP-16`); models are built from parts that take their materials' colours (`PRE-42`), and sounds from a base set (`SND-06`), so one model or sound serves many things; automated checks catch gaps (`MAT-17`); the launch numbers are targets, cut with you if needed.

- `RSK-05` **The scope never ends** *(Decided)*
  - **Rating:** likelihood high, impact medium.
  - **Risk:** A world that can always go deeper never gets finished.
  - **Signs:** stages slipping again and again; alphas that add little you can see.
  - **Response:** playable alphas of a few hours each (`PRN-09`), in stages with fixed goals (`SCP-16`); the launch arc ends at first copper (`VIS-03`); what was cut stays cut (`SCP-21`).

- `RSK-09` **AI-built code drifts** *(Decided)*
  - **Rating:** likelihood medium, impact high.
  - **Risk:** A large codebase built by many AI sessions slowly drifts from what this file says.
  - **Signs:** gaps in the coverage check; reviews finding behaviour that contradicts this file; tests quietly weakened.
  - **Response:** the guide for AI agents (`PRC-06`); IDs in every test and the coverage check (`RES-01`, `PRC-12`); pass rules that can't be quietly loosened (`RES-09`); independent review (`PRC-09`); and modular design (`PRN-14`).

- `RSK-14` **Tests too big for the cloud** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** The pace tests need about 20 whole worlds of up to 500 game years (`RES-07`), which may not fit overnight in the cloud sessions (`SCP-15`).
  - **Signs:** overnight runs that don't finish; pace tests cut short.
  - **Response:** scenes instead of whole worlds wherever a scene can answer (`RES-21`); several sessions side by side; more nights or fewer worlds, stated in the report (`RES-13`); the speed work for the phone speeds the tests too (`RSK-02`); more computing only after asking you (`SCP-15`).

- `RSK-23` **Your time** *(Decided)*
  - **Rating:** likelihood medium, impact medium.
  - **Risk:** Playing alphas, reviews, and judging look and sound all need you, so the project moves only as fast as your time allows.
  - **Signs:** stages waiting on reviews; alphas piling up untried.
  - **Response:** alphas are yours to try when you like, and only stage reviews wait for you (`PRC-11`); each review comes with a short list of what to look at (`RES-22`); reports are short (`RES-15`); the phone benchmark takes one tap (`PLT-04`).

- `RSK-10` **History too slow to watch** *(Dropped)*
  - **Dropped because:** covered by `RSK-02` (speed on the phone) and `RSK-26` (the pace of discovery).

- `RSK-13` **Simplified minds behave differently** *(Dropped)*
  - **Dropped because:** every person always has a full mind (`MND-14`), so there are no simplified minds.

- `RSK-16` **Invented sources** *(Dropped)*
  - **Dropped because:** values are now plausible estimates set by hand, not sourced (`MAT-05`); values that feel wrong are covered by `RSK-06`.

## 17. Not yet decided

### 17.1 Open, or settled by measurement

<!-- generated: open items -->
- **Pacing** (`TIM-07`): how long history takes to watch; measured during development.
- **Pace of discovery** (`TIM-19`): settled by measurement during development.
- **How many people the world can feed** (`WLD-04`): measured in experiments.
- **Population limit** (`MND-15`): settled by measurement during development.
- **Pace of culture** (`CUL-33`): settled by measurement during development.
- **Storytelling voices** (`PRE-19`): tried live and chosen by ear.
- **The phone's limits** (`PLT-04`): measured from the first build.
<!-- end generated -->

### 17.2 Proposals awaiting confirmation

New suggestions from AI agents are marked *Proposed* and listed here until you confirm, change or drop them (`PRC-07`).

<!-- generated: proposals -->
- None at the moment.
<!-- end generated -->

## 18. Glossary

- **Activity:** anything a person or animal does, with a start and an end; its results land when it ends (`TIM-17`).
- **Alpha:** one playable step of the build, a few hours of AI work, ending with a version you can install and play on your phone (`SCP-03`, `PRC-11`).
- **Area:** a patch of land about 256 m across, about 16 to a world cell, detailed down to about a metre: ground, stones, each tree and bush, caves and water (`WLD-12`).
  It is made from the seed when people first go there (`WLD-13`).
- **Art pixel:** one pixel of the low-resolution picture, enlarged on screen (`PRE-22`).
- **Band:** a small group of people, usually family, who live and move together.
- **Base action:** one of the 21 actions people do to things, such as strike, cut, heat or bind (`MAT-06`).
  Everyday activities, such as walking, eating or talking, are not base actions.
- **Belief:** something a person holds true, with more or less certainty, such as a cause and its effect, a spirit, a taboo or what others think; some beliefs are wrong (`MND-27`).
- **Belief template:** a shape people give to what they can't explain: a spirit of a place, animal or weather, an ancestor, a taboo, a ritual or an offering (`CUL-05`).
- **Blueprint:** a hidden rule of the game: if someone does these actions, on things with these characteristics, in these conditions, a named result follows, more surely with experience (`MAT-04`).
  It works for anything with the right characteristics, and nobody knows it until they discover it or learn it (`PRN-01`).
- **Book of ages:** a world's chronicle: named discoveries and who made them, births and deaths, feuds, migrations and disasters, written as prose (`PRE-05`).
- **Catalogue:** one of the game's lists of content, written by AI agents and checked by tests: items, blueprints, plants, animals and illnesses (`MAT-13`).
- **Characteristic:** one of 18 qualities every item has, scored 0–5, such as hardness, edge, warmth or poison (`MAT-03`).
  Some show at a glance; others are learned only by use.
- **Chronicle:** see Book of ages.
- **Details view:** the view into one mind: needs, mood, thoughts, memories, beliefs, and the reasons behind each choice (`PRE-14`).
- **Discovery:** learning a blueprint by accident, by experimenting, from a dream's hint or by copying (`MND-11`).
  A people's first success is a named discovery, named in their language and written in the book of ages (`MAT-21`).
- **Domestic kind:** a line of animals kept by people for generations and born tame, such as the dog from the wolf (`WLD-33`).
- **Dream:** something you can send a sleeping person: a place, an animal, a person, a fear, or a hint of a blueprint close to what they already know (`GOD-03`).
- **Experience:** how practised someone is in one of the 15 sectors; it grows with use, fades slowly without it, and grows faster when someone teaches them (`MND-06`).
- **Game year:** 60 game days, in four seasons of 15 (`TIM-18`).
  Growing up, ageing, pregnancy and the growth of plants are squeezed into it; everything within a day takes its real time.
- **Heat level:** how hot a fire is, from 1, embers, to 5, a furnace with forced air (`MAT-18`).
- **Herd count:** how animals far from people are kept: a number in each world cell that moves with food and season; near people, they become single animals (`WLD-32`).
- **Institution:** a shared, named pattern of behaviour (a norm, role, rank or rite) that people know, teach and enforce (`CUL-06`).
- **Intervention:** anything you do with your powers (see The player as god).
- **Item:** a kind of thing, a material or something made, with its characteristics; about 200 at launch (`MAT-10`).
- **Live moment:** a notable event the game shows you as it happens (`PRE-08`).
- **Mental map:** what a person knows of places: where to find food, water, stone and shelter, and where danger lies, by season (`MND-28`).
- **Milestone:** see Stage.
- **Mood:** how a person feels overall, from their needs and recent thoughts; very low mood can end in a breakdown (see Minds).
- **Murmur:** how speech sounds in the game: a babble made from the language's own sounds and the speaker's mood, never real words (`SND-03`).
- **Need:** something a person must keep up: hunger, thirst, warmth and rest for the body (`BIO-09`), and safety, belonging, status, curiosity and love for the mind (see Minds).
- **Overnight mode:** the world running at top speed, screen dimmed, while the phone charges (`TIM-12`).
- **Pace target:** the span of game years in which typical worlds reach a step, such as making fire in years 5–30 (`TIM-19`).
- **Pace test:** the test that runs whole worlds overnight to check every pace target (`RES-07`).
- **People (a people):** a named group with its own territory, customs, beliefs and style (`CUL-23`).
- **Quality:** how well a thing is made, from 0 to 5, set by its maker's skill and its inputs (`MAT-20`).
- **Reality rule:** a rule the blueprint catalogue must obey, such as "flint flakes and granite doesn't" or "copper needs a furnace", checked by an automated test (`RCK`).
- **Run:** one play-through of a scene or a world for a test (`RES-13`).
- **Scene:** a small setting built for one test, such as a band by a river with flint and decoy stones, run by the game's own rules with nothing scripted (`RES-21`).
- **Season:** a quarter of the game year: 15 days of spring, summer, autumn or winter (`TIM-18`).
- **Sector:** one of 15 fields of experience: stone, wood, fire, cooking, hunting, gathering, hides, building, healing, pottery, herding, farming, metal, art and music (`MND-06`).
- **Seed:** the number a world is generated from; every area is made from it too, so an area comes out the same whenever it is made (`WLD-13`).
- **Signature moment:** a story the game must be able to produce without it being scripted (`MOM`).
- **Skill:** how good someone is at one blueprint they know; it grows with practice and fades without it (`MND-06`).
- **Stage:** one of the seven milestones of the build, from First camp (`MIL-01`) to Herds, fields and villages (`MIL-07`).
  Each is a group of alphas and ends with a report and your review (`RES-06`, `RES-22`).
- **Story director:** sets the speed of time by what is happening, slowing for important moments and racing through quiet years (`TIM-02`).
  It never causes events (`TIM-03`).
- **Switch-off run:** a scene run with one thing switched off, such as teaching or dreams, to see what a result depends on; it exists only in tests (`RES-10`).
- **Taming:** animals fed and kept near people grow tame, and young raised by people grow up tame (`WLD-33`, `RCK-24`).
- **Thought:** a reaction to something that happened, which lifts or lowers mood for a while, such as a fine meal or a friend's death (see Minds).
- **Timer:** a slow change on a thing, such as rotting, drying, cooking, smoking, fermenting, setting or firing, sped up or slowed by conditions (`MAT-19`).
- **Wear:** how used a thing is, from new to broken (`MAT-20`).
- **Weather cell:** a patch of sky about 10 km across, with its own temperature, wind, cloud, rain and snow, updated every game hour (`WLD-12`).
- **World:** one generated planet, about 2,000 km around and 1,000 km from pole to pole, wrapping both ways (see World).
- **World cell:** a square of land about 1 km across, about 2 million to a world, holding its height, rock, soil, plants, water, deposits and passing herds; it is always simulated, at a coarse pace (`WLD-12`).
- **Writer AI:** the phone's built-in AI language model, which turns the game's records into prose (`PRE-37`).
  It never decides anything, and dark events are never left to it (`PRE-17`).
