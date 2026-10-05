# Kindling

The project file: what Kindling is, and every feature and target it must reach.

It holds no implementation details: those belong in the architecture and the implementation plan, which link back here by ID, as will the code.

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
| *Proposed* | Suggested, for you to confirm, change or drop (`PRC-07`); listed in the Not yet decided section. |
| *To test* | Settled by measurement, not opinion; the result is written into the item, which becomes *Decided* at a stage review with your OK. |

Every item, moments, milestones and risks included, carries exactly one of these in its marker.
An item no longer planned is removed, and its ID retired (IDs and links, rule 3).

### Kinds of item

- **Feature:** something to build, which needs a task in the implementation plan.
- **Rule:** something that must always hold, which needs a check.
- **Context:** explains or records, and needs nothing built.

Each area has a default kind; the exceptions are listed here, in one place:

- **Context:** `VIS`, `MIL` and `RSK`, plus `SCP-01`, `SCP-16`, `GOD-01`, `WLD-04`, `BIO-01`, `MAT-16`, `PRC-01` and `PRC-08`.
- **Rules:** `PRN`, `MOM` (checked by test scenes), `SCP`, `RCK`, `RES` and `PRC`, plus `GOD-05`, `GOD-06`, `GOD-07`, `TIM-03`, `TIM-07`, `TIM-16`, `TIM-17`, `TIM-18`, `TIM-19`, `WLD-13`, `WLD-22`, `WLD-30`, `MAT-05`, `MAT-09`, `MAT-13`, `MAT-14`, `MAT-17`, `BIO-14`, `BIO-17`, `MND-01`, `MND-02`, `MND-14`, `MND-15`, `CUL-07`, `CUL-33`, `PRE-13`, `PRE-17`, `PRE-31` and `SND-12`.
- **Features:** every other area, plus `VIS-14`, `VIS-15`, `VIS-17`, `RES-02`, `RES-06`, `RES-10`, `RES-12`, `RES-16`, `RES-21`, `RES-22`, `RES-23`, `RES-24`, `PRC-06` and `PRC-12`.

### Item format

Every item starts with its ID, a short name and its status.
Detailed items add some of these:

- **What:** what it is, in plain words.
- **How it works:** its rules and numbers, in plain words.
- **Why:** the reason it exists.
- **Example:** a concrete illustration.
- **Done when:** checks proving it is delivered.
- **Check:** how we verify a rule keeps holding.
- **Follows from:** the items it follows from.

*What*, *Done when* and *Check* are binding; *Why* and *Example* only explain; any other label counts as part of *What*.
If a summary and its source disagree, the source wins.
"About" means within 10% unless stated; chances and shares are checked as `RES-13` sets out, and a blueprint's chance as `RES-24` does.
Each item's detailed acceptance criteria are written in the implementation plan, and you approve them in the review that closes the stage before (`RES-22`), the first stage's with the plan itself; loosening one you approved needs your OK again (`RES-09`).

### Changing this file

- AI agents suggest additions or changes as *Proposed* items (`PRC-07`).
- To change a decided item, a **Proposed change:** line goes beneath it, with the new text and the reason.
  Your OK replaces the text.
- Items are edited in place, and keep their IDs.
- Every commit that changes this file ends with a line naming the changed IDs and why, for example `Changed: GOD-04 (blessing cap raised; owner OK)`.
  The commit check enforces it (`PRC-07`), and the coverage check flags work linked to a changed item (`PRC-12`).
- Parts of an item delivered in different stages get separate IDs, split when the plan needs it.
- In the source, each sentence starts on its own line, so changes show clearly.

### IDs and links

1. Every item has a permanent ID: an area code plus a number, such as `WLD-01`.
   Numbers go to three digits after 99, such as `RCK-100`.
2. IDs are never renumbered or reused.
   A new item takes the next free number in its area, wherever it sits in the text.
   A new ID becomes permanent only when it reaches the main version; until then, a branch whose number is taken renumbers its own new items.
3. A cut item is removed, and its ID is retired: never reused, and cited nowhere.
   Retired IDs: `BIO-07`, `CUL-04`, `CUL-13`, `CUL-14`, `CUL-15`, `CUL-25`, `MAT-15`, `MIL-01`, `MIL-02`, `MIL-03`, `MIL-04`, `MIL-05`, `MIL-06`, `MIL-07`, `MND-17`, `MND-25`, `MOM-05`, `MOM-10`, `PRC-05`, `PRE-04`, `PRE-11`, `PRE-12`, `PRE-36`, `PRE-38`, `PRN-08`, `RCK-05`, `RCK-09`, `RCK-17`, `RCK-18`, `RCK-19`, `RCK-20`, `RES-04`, `RES-08`, `RES-11`, `RES-15`, `RES-20`, `RSK-10`, `RSK-12`, `RSK-13`, `RSK-16`, `RSK-17`, `RSK-20`, `RSK-22`, `SCP-13`, `SCP-14`, `SND-04`, `SND-05`, `SND-09`, `SND-10`, `TIM-06`, `TIM-12`, `TIM-13`, `VIS-13`, `WLD-05`, `WLD-19`, `WLD-20`, `WLD-21`, `WLD-23`, `WLD-25`, `WLD-29`, `WLD-34`.
4. The implementation plan maps every feature and rule to a stage, and every task names the IDs it delivers, as the coverage check enforces (`PRC-12`).
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
| `RCK` | Reality rules |
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

### 1.1 The game in brief

- `VIS-01` **In one sentence** *(Decided)*: A realistic life and world game for one phone, in which a few bands of early humans living in caves find their own way to sharp stone, fire, huts, pottery, herds, fields and first copper, while you watch over them as a hidden god.

- `VIS-06` **In one paragraph** *(Decided)*: Kindling is a living world in your pocket: land, weather, plants, animals and people, all running at once.
  It begins with a few family bands of modern humans sheltering in caves, with a language of their own but almost no culture (`SCP-01`).
  Each is a full person, with a name, needs, moods, memories, friends and rivals, and nothing tells them what to do.
  Hidden in the world are about 140 blueprints: what happens when someone strikes, heats, soaks or shapes things with the right characteristics.
  People find them by accident, by experimenting, in dreams and by copying, and each people's first success becomes a named discovery in the book of ages.
  A game year lasts 60 days, and a typical world goes from sharp flakes to first copper in a few hundred game years.
  You watch it all as a hidden god who can send weather, dreams and luck, but can never command anyone, and nobody ever learns you exist.

- `VIS-02` **The fantasy** *(Decided)*: You are a hidden god who acts only through nature: the weather, the luck and the dreams.
  - **What:** You can bring a storm, send lightning, hold back the rain, bless a hunter, or give someone a dream made of their own memories, but you can't speak, appear or work miracles.
  - **Why:** A god who can't command anyone leaves every achievement theirs, and the gods they come to believe in are their own explanations, sometimes of you.

### 1.2 What it feels like

- `VIS-07` **Wonder** *(Decided)*
  - **What:** Awe at a world that runs itself and keeps surprising you, its maker included.

- `VIS-08` **Curiosity** *(Decided)*
  - **What:** The urge to understand why something happened, and to try "what if".

- `VIS-17` **Life** *(Decided)*
  - **What:** The pleasure of a world that is always busy at every zoom, with everyone doing something for reasons of their own.
  - **Example:** At dusk in camp, a man knaps flint by the fire, a woman scrapes a hide, children chase each other round the hearth, and the murmur of talk rises and falls (`SND-01`).
  - **Done when:** whole worlds pass `RES-25`, and in a camp scene of about 30 by day, awake people are doing at least 6 different activities at most moments.

- `VIS-09` **Other feelings** *(Decided)*: Attachment to particular people, and the harshness of nature, will arise from the simulation and are welcome, but the design isn't built around them.
  When design choices conflict, wonder, curiosity and life decide.

### 1.3 How you play

- `VIS-10` **Two rhythms of play** *(Decided)*
  - **Short check-ins (5–15 minutes):** open the app, catch up on the live moments waiting, follow someone for a while, nudge, close.
  - **Long sessions (an hour or more):** watch a season or a century go by, read the book of ages, visit graves and old camps, and try a "what if" with your powers.
  - The world pauses when the app is closed (`TIM-05`), so every session starts exactly where the last one ended.

- `VIS-11` **A session, as a story** *(Decided)*: An illustration, not a script.

  > You open the app.
  > The world is where you left it: Year 19, autumn, day 8, a week before winter, in the valley of two rivers.
  > A live moment is waiting: *the Hazel band has lost its fire*.
  > You tap it, the camera swoops down, and time slows to real speed.
  > Under the cliff it is cold tonight, the murmur of voices is low and worried, and wolves pace at the edge of the scree.
  >
  > You open the memories of Ama, the band's most curious woman.
  > Last summer, twirling a stick against dry wood, she saw smoke curl from its tip.
  > While she sleeps, you give her a dream that sets that smoking stick beside the warmth of a fire.
  > In the morning she twirls a stick again and again, with the driest wood she can find.
  > You pull back to the camp, and the days pass in minutes.
  > On day 12 an ember glows in the dust, and she breathes it into flame.
  > The book of ages records a named discovery: *hesoru*, "fire from wood", first made by Ama in Year 19.
  > By spring, four others can do it.
  >
  > Over the next few evenings you watch from the globe as the centuries race past, tapping in whenever a live moment calls (`TIM-01`, `PRE-08`).
  > By Year 301, 282 years have passed.
  > By the river, a band of the Tavu now lives all year in a village of reed-roofed houses, keeps goats and dogs, and sows wild grain on its old rubbish heaps.
  > In the book of ages, Ama's story has become a myth: *Ama took the fire that sleeps inside the wood*.

### 1.4 Signature moments

- `VIS-12` **Signature moments** *(Decided)*: Stories the game must be able to produce, none of them scripted.
  Each has a sandbox scene that must keep producing it (`RES-17`): its **Check** gives the scene, the years the moment has, and the stage from which the scene runs.
  A moment depends on every item it cites.

  - `MOM-01` **Fire from wood** *(Decided)*: In a hard winter, a band whose fire has died learns to make fire by drilling, a trick one of them stumbled on.
    - **How it works:**
      1. Someone twirling a stick against dry wood while experimenting or at play (`MND-11`) matches the blueprint for fire by drilling (`RCK-02`) and fails in its usual way, with smoke but no ember; the surprise gives a hunch (`MND-10`, `MND-11`).
      2. When the band's found fire dies (`BIO-02`), cold pushes the most curious to try the hunch again and again with the driest wood they can find, until an ember glows: a named discovery (`MAT-21`), passed on by teaching (`MND-13`).
      3. A dream you send can set the smoking stick beside the warmth of a fire, raising the odds but promising nothing (`GOD-03`).
    - **Check:** a band of about 20 with dry wood and tinder, one of them holding a hunch from a smoking stick, loses its fire as winter starts; it makes fire by drilling within 1 year; from `MIL-14`.
  - `MOM-02` **The lost craft** *(Decided)*: A fever kills the last person in a band who can make fine blades.
    For years, sometimes a generation, its tools are cruder, until the craft is found again, learned from neighbours, or copied from an old blade.
    - **How it works:**
      1. A fever (`BIO-05`) kills the last person who knew the blade blueprint (`BIO-14`), a craft that takes long experience in stone (`MND-06`); the knowledge dies with them (`CUL-02`), and the survivors' plain flakes are cruder and wear out sooner (`MAT-20`).
      2. It comes back only by experimenting, from neighbours (`CUL-16`), or by copying an old blade, which gives only a weak hunch (`MND-11`); the book of ages marks the loss and the return (`PRE-39`).
    - **Check:** two bands a day's walk apart, the blade blueprint known to one adult in one of them, where a fever starts; within 40 years the craft is lost, then made again; from `MIL-14`.
  - `MOM-03` **Your lightning becomes a god** *(Decided)*: A lightning strike you sent kills a hunter on a hilltop.
    - **How it works:**
      1. A death from the sky (`GOD-02`) can be put down to an unseen being in the storm (`MND-05`, `CUL-05`), and fear ties itself to the hill (`MND-08`).
      2. They avoid the hill (`CUL-20`); an angry spirit pulls its believers to leave gifts at its place (`CUL-05`), and when the storms stop after one, the gift is credited (`MND-05`) and leaving gifts there becomes a rite (`CUL-34`).
      3. Retold, the story becomes a myth (`CUL-11`), and in time part of their religion (`CUL-26`); nothing marks the strike as yours (`GOD-06`).
    - **Check:** a band whose hunters cross a bare hilltop, where the test's lightning kills one; within 30 years the band avoids the hill, leaves gifts there as a rite and tells a myth of the one in the storm; from `MIL-16`.
  - `MOM-04` **The song that does nothing** *(Decided)*: A band sings before a hunt that goes well.
    The song becomes a hunting rite and is kept for centuries, though it changes nothing.
    - **How it works:**
      1. The song gets the credit for the good hunt (`MND-05`); singing costs little, and hunts go well often enough that the belief survives and spreads (`CUL-01`).
      2. Held by most of the band, it becomes a rite (`CUL-34`), kept long after anyone remembers why (`CUL-06`).
    - **Check:** a band that hunts every few days and sometimes sings; within 20 years, singing before hunts becomes a rite and is kept for at least 10 of them; from `MIL-15`.
  - `MOM-06` **The camp wolf** *(Decided)*: The boldest wolves scavenge at the edge of camp.
    - **How it works:**
      1. Scraps draw the boldest wolves close, and wolves fed and not harmed lose their fear of people (`WLD-32`, `MND-16`); you can send one a dream of the scraps (`GOD-12`).
      2. A she-wolf grown bold dens near camp; when she is killed or driven off, children carry her pups home (`MAT-06`) and feed and play with them (`MND-09`), and young raised by people grow tame (`RCK-24`).
      3. The litter, kept together, breeds near camp; after several generations the line becomes dogs (`WLD-33`), recorded in the book of ages (`PRE-05`).
    - **Check:** a band with a rubbish heap at camp and a wolf pack denning within 2 km; the band keeps dogs within 40 years; from `MIL-15`.
  - `MOM-07` **A painting that remembers** *(Decided)*: A painting of a great hunt outlasts everyone who saw it.
    - **How it works:**
      1. Someone who remembers a great hunt vividly (`MND-18`) paints it in red ochre and fat on a sheltered wall, in their people's style, with the animals and hunters that were really there (`CUL-09`); paint in shelter lasts for centuries (`RCK-16`).
      2. The hunt is in the saved history (`PRN-15`), and a painting of it keeps it from being thinned (`PLT-10`), so tapping the painting shows what it depicts and what really happened (`PRE-15`).
    - **Check:** a band with ochre, fat and a sheltered wall, just after a great hunt; within 80 years a painting of it outlives its last witness, and tapping it shows the saved hunt; from `MIL-16`.
  - `MOM-08` **Seeds on the rubbish heap** *(Decided)*: Seeds thrown on the rubbish heap sprout near camp.
    - **How it works:**
      1. In the growing season, some seeds thrown on the camp's heap sprout in its rich, damp ground (`MAT-08`, `RCK-23`).
      2. People notice food plants growing on the heap, where they always throw that plant's husks and seeds (`MND-10`); the surprise links the plants to what is done at that place (`MND-05`), and gives a hunch that seed thrown on rich ground grows (`MND-11`).
      3. When food runs short, someone tries putting seeds in the ground on purpose; tended plots grow better (`RCK-23`), and sowing spreads by copying and teaching (`CUL-01`).
    - **Check:** a band that camps by wild grain each summer, with a rubbish heap; within 50 years someone sows seed on purpose and harvests it; from `MIL-17`.
  - `MOM-09` **The dig** *(Decided)*: Digging a pit on an old campsite, someone turns up a tool nobody living knows how to make, and copies it.
    - **How it works:**
      1. A band that made eyed bone needles dies out, and its camp and tools are slowly buried where they were left (`MAT-08`).
      2. Generations later, a woman of another band digs a storage pit there and finds a needle; it gives her a weak hunch (`MND-11`), and her experience decides how soon she copies it (`MND-06`).
      3. The book of ages marks a rediscovery (`PRE-39`), and the craft spreads again (`CUL-01`).
    - **Check:** a band without needles stores food in a cave whose floor hides an old camp's eyed needles; within 10 years someone digs one up and makes a needle; from `MIL-14`.
  - `MOM-11` **Rivals, then in-laws** *(Decided)*: Two bands fight over a valley, then marry into each other.
    - **How it works:**
      1. Two bands of peoples long apart rely on one valley's game and nuts (`MND-28`); when they meet there, fear, anger and hunger can make a raid the better choice (`MND-09`, `CUL-31`).
      2. Losses, and too few partners at home, make marrying across the better choice for some (`CUL-27`); kin rarely raid each other, so fights grow rarer (`CUL-31`).
      3. Each side retells the fight through its own memories and beliefs, so their stories differ (`MND-18`, `CUL-11`).
    - **Check:** two bands of peoples long apart share one valley in lean years; within 40 years a raid, then at least two marriages across, then fewer raids than before; from `MIL-15`.
  - `MOM-12` **Metal from green stone** *(Decided)*: Pots painted green come out of a blown kiln flecked with shiny metal, and someone notices.
    - **How it works:**
      1. Potters who know the blown firing (`MAT-23`) blow into their charcoal kilns through pipes, so the kiln reaches heat 5 and fires tougher pots in half the time (`MAT-18`, `MAT-19`).
      2. Pots painted with ground green ore (`CUL-09`) come out flecked with metal (`MAT-19`, `RCK-08`).
      3. The shine is a surprise (`MND-10`) that may lead someone to try again (`MND-11`); the first copper is a named discovery (`MAT-21`) and begins a new age in the book of ages (`PRE-05`).
    - **Check:** potters who know the blown firing and paint pots green, with green ore in reach; within 30 years someone notices copper from a blown kiln (`MAT-21`); from `MIL-17`.

### 1.5 The arc of a world

- `VIS-03` **The arc of a world** *(Decided)*
  - **What:** Every world starts in caves (`SCP-01`).
    Sharp flakes come in the first two years and fire within about 8; pottery, dogs, herds, villages and fields follow over the next century, and first copper about a century in (`TIM-19`).
    After that, history goes on within the launch catalogue, which later layers, such as bronze or writing, can extend (`PRN-14`).
  - **No scripted eras:** each step happens only when the world's rules bring it about (`PRN-17`), so the order differs between worlds, and stalls, lost crafts and peoples dying out are all valid histories.
  - **The phone's limit:** nothing caps births (`BIO-04`); past about 2,000 people time slows rather than detail being cut, and a world nearing the phone's memory limit pauses with a notice (`MND-15`).

### 1.6 Inspirations

- `VIS-04` **Inspirations** *(Decided)*: Where this file leaves a design question open, start from what these games do, adapted to the principles: [world-sim](https://world.world-sim.uk) (named souls, graves, a book of ages, crafts discovered), Dwarf Fortress (personalities, memories, legends, art of real events), RimWorld (needs, moods from thoughts, the 60-day year, pacing), WorldBox (peoples rising in a world in your hand), The Sims (needs, an animation for every action), Black & White (a god whose acts shape belief) and Ancestors: The Humankind Odyssey (early humans learning by trial).

### 1.7 Success

- `VIS-14` **A joy on the phone** *(Decided)*
  - **What:** Beautiful, smooth and absorbing in your hand.
  - **Done when:** the phone measurements of `PLT-04` for smoothness, opening, battery and heat pass, and every screen works in both orientations (`PRE-34`).

- `VIS-15` **Histories worth reading** *(Decided)*
  - **What:** Every world produces a history you would want to read, and no two are alike.
  - **Done when** (judged by you at milestone reviews):
    - you'd choose to read a world's book of ages for pleasure;
    - worlds from different seeds tell clearly different stories;
    - every entry in the book of ages can be traced back to the events behind it.

- `VIS-05` **Quality bar** *(Decided)*: A believable game, tested at every step, and craft polished as far as the tools allow.
  - **Tested:** what the game is meant to do is checked by automated tests that can fail: its rules, blueprint chains, behaviours and speed at every alpha, and its pace at every stage (`RES-01`).
  - **Craft:** art, animation, sound, interface and performance polished as far as AI agents can take them, judged by you at every visual review (`PRE-31`).

### 1.8 Name

- `VIS-16` **Name** *(Decided)*: **Kindling**, what a fire grows from: small things that catch and spread, like knowledge.

## 2. Principles

- `PRN-16` **Principles come first** *(Decided)*: These rules apply to every part of the game, and outrank everything else in this file: if any decision conflicts with a principle, the principle wins.
  A principle changes only if you change it here.
  - **Check:** every milestone report lists each principle with the result of its Check line (`RES-06`).

### 2.1 The world

- `PRN-01` **The world is the only teacher** *(Decided)*
  - **What:** Everything the people of the world know, they learned inside it: from their senses, their own tries, other people or their dreams.
    Nothing is handed to them beyond the little they know at the start (`BIO-20`).
    Nobody knows a blueprint until they discover it or learn it from someone (`MND-11`).
    The one exception is your Revelation (`GOD-13`): a person, or everyone, can come to know a blueprint because you chose it, and must still learn to make it.
    No choice is ever built on a blueprint they don't know, or on a fact they haven't seen or been told (`MND-02`).
  - **Why:** A discovery only means something if it was really made.
  - **Check:** every discovery can be traced to its route (`MND-11`), and tests find no choice built on an unknown blueprint or an unseen fact.

- `PRN-02` **Believable over exact** *(Decided)*
  - **What:** The world looks, sounds and behaves like the real one, without simulating how the real one works underneath.
    Where an exact model would be costly, a simple rule that gives a believable result wins (`SCP-21`).
  - **Example:** Flint gives sharp flakes and granite doesn't because flint's characteristics say it flakes well (`RCK-01`), not because the game simulates cracks running through stone.
  - **Check:** each milestone review flags any system whose detail changes nothing you can see or read.

- `PRN-07` **Generic blueprints** *(Decided)*
  - **What:** Everything people make on purpose is made by a blueprint, and every blueprint is generic: it asks for an action, characteristics, material classes and forms, and conditions, never particular items (`MAT-04`).
    Things also change by themselves, through fire, timers and the living world (`MAT-18`, `MAT-19`), by fixed rules in the catalogue, the same for everyone.
    So one blueprint works for everything that fits, and people can find routes nobody wrote as a recipe; the catalogue check shows each fit to a reviewer once, so none is absurd (`MAT-17`).
    The rules that decide what people and animals do never single out a blueprint by name, and see only what people perceive and know.
    Names such as "flake" or "pottery" appear only in the catalogue, in tests and in text written for you.
  - **Example:** The blueprint for a scraper asks for something hard that flakes well and has a good edge, never for flint by name.
    Obsidian and chert work too, and so would a stone added years later.
  - **Check:**
    - the catalogue check finds no blueprint naming an item (`MAT-17`);
    - the rules that decide what people and animals do hold no blueprint, discovery or item names;
    - swapping two materials' names changes no behaviour;
    - a made-up material works in every blueprint it fits.

- `PRN-05` **Plausible numbers** *(Decided)*
  - **What:** Every value in the game, such as a material's characteristics, a plant's yield, an animal's speed or an illness's danger, is a plausible estimate, set by hand from what the real thing is like and tuned so the game feels right.
    Amounts that come once a year in life, such as a crop or a herd's young, are scaled so the land yields per game day about what it yields per real day (`TIM-18`, `WLD-30`).
    No value needs a sourced measurement.
  - **Check:** the catalogue tests pass (`MAT-17`), and every claim in a milestone report is backed by a test that can fail (`RES-01`).

- `PRN-12` **Speed up time, never bend the rules** *(Decided)*
  - **What:** In play, how fast you see history pass comes only from controlling time: zoom, the story director and manual speed (see Time and history).
    Tuning happens when the game is made, the same for every world (`PRN-17`).
    The rules never change during play to make things faster or more dramatic, and nothing happens because it would make a better story.
  - **Check:** play has no rule-bending settings, and the same saved world runs the same with the story director on or off (`TIM-03`).

- `PRN-17` **History at a watchable pace** *(Decided)*
  - **What:** Discoveries come at a pace you can watch, set by the pace targets (`TIM-19`).
    It comes from tuning chances and amounts, the same for every world, such as how often people experiment, how likely a try is to succeed and how much food the land gives.
    No discovery is ever scripted or forced by a date.
  - **Why:** A world where nothing changes for ten thousand years may be true to history, but it is no fun to watch.
  - **Check:** the pace tests show each step landing in its window in typical worlds, not always in the same order, with nothing scripted (`RES-07`).

### 2.2 The player

- `PRN-03` **You are nature** *(Decided)*
  - **What:** The player acts only through natural means (`GOD-05`), with one exception, Revelation (`GOD-13`), and is never known to exist (`GOD-06`).
  - **Check:** every power but Revelation produces only events the world could produce on its own.

### 2.3 What you see

- `PRN-04` **If the game knows it, you can see it** *(Decided)*
  - **What:** Anything the game keeps track of can be shown to you: a person's needs, mood, memories, beliefs and the blueprints they know; family trees; who taught whom; graves and old camps.
  - **Example:** The game records who taught whom to drill fire (`MND-13`), so you can follow the chain back to the first person who did it.
  - **Check:** everything the game keeps track of has at least one view that shows it, if only a card (`PRE-35`) or the details view of a mind (`PRE-14`).

- `PRN-10` **Nothing is faked** *(Decided)*
  - **What:** Every picture, sound and word shows what is really there and what really happened in the world.
    Places nobody has visited are drawn from the world's seed, just as they are made when someone first goes there (`WLD-13`).
    Nothing is added for show.
  - **Why:** Histories are only worth reading (`VIS-15`) if they are true to the world, and curiosity only works if every clue is real.
  - **Example:** The murmur at a camp comes from people really talking there (`SND-03`).
  - **Check:** every live moment, entry in the book of ages, sound and on-screen event can be traced back to something that happened in the world.

- `PRN-13` **Every choice can be explained** *(Decided)*
  - **What:** Why anyone, person or animal, does something can be traced to their needs, personality, plans, beliefs and memories.
    Each choice keeps its top reasons, which the details view shows (`PRE-14`).
  - **Check:** for any activity under way, and for the choices behind every saved event (`PRN-15`), the details view shows the reasons kept with it.

- `PRN-06` **AI language models describe, never decide** *(Decided)*
  - **What:** AI language models are used only to reword the sentences the game builds from its own records, for the book of ages, life stories, myths and summaries (`PRE-37`).
    They never choose, invent or know anything for the people or animals of the world, and never add, drop or change a fact (`PRE-41`).
  - **Why:** A language model knows our history.
    If it did their thinking, our knowledge would leak into their world and their discoveries would no longer be theirs.
  - **Example:** The model can tell you how, as the Tavu tell it, Ama "took the fire that sleeps inside the wood".
    It cannot decide that she tries drilling.
  - **Check:** nothing a language model writes ever feeds back into the game, and each sentence it writes is checked against the pattern sentence it rewords (`PRE-41`).

### 2.4 How it runs

- `PRN-15` **History is saved, not re-run** *(Decided)*
  - **What:** The past is kept as the book of ages and the events behind it, with what each view of the past needs: graves, old camps, family trees, art.
    The world's full state is kept only for the present (`PLT-07`), so the past can't be replayed or returned to.
    History is kept as it happened, never re-made from the seed.
  - **Why:** Replaying the past would need every old version of the rules kept forever, and saving what matters avoids that cost.
  - **Check:** every view of the past reads what was saved, and nothing re-runs the past.

- `PRN-11` **Time slows, the screen stays smooth** *(Decided)*
  - **What:** The screen never stutters.
    When the phone can't keep up, the world doesn't cut detail: time simply runs more slowly.
    Every person is a full individual at all times, watched or not (`MND-14`).
    The only simplifications are the planned ones, the same whether you look or not (`WLD-12`, `WLD-32`, `MND-16`).
  - **Why:** A smooth screen is part of the joy on the phone (`VIS-14`), and cutting detail under load would make history depend on how busy the phone is.
  - **Check:** measurements show no stutter under heavy load (`PLT-04`), and the same saved world gives the same results at every speed and zoom (`TIM-17`).

### 2.5 How it's built

- `PRN-09` **Build in steps you can try** *(Decided)*
  - **What:** The game is built as a series of alphas, each ending with something you can install, open and try on your phone (`SCP-03`): a test screen or a scene to look at in the first milestones, then a world to explore and watch, and the game to play from `MIL-16`.
    Each alpha builds only what it needs, on foundations that later ones extend without starting over.
  - **Example:** Copper waits for the last milestone (`MIL-17`), but things and blueprints are designed from the start so it can be added without rework.
  - **Check:** every alpha ends with a build you can install and try (`PRC-11`), and every task in the implementation plan names the items it delivers.

- `PRN-14` **Modular by design** *(Decided)*
  - **What:** Every system grows by adding self-contained pieces (items, blueprints, plants, animals, illnesses, behaviours, views, tests), and never rewrites what already works without a stated reason.
  - **Example:** Adding a new animal later needs one catalogue entry, with its model, sounds and tests.
    Hunting, taming and herding already work for it, because their rules never named a species.
  - **Check:** every milestone report lists what was added and confirms that nothing earlier had to be rewritten, or explains why it had to be.

## 3. Scope and non-goals

### 3.1 Where history starts

- `SCP-01` **Starting point** *(Decided)*: Modern minds with almost no culture.
  - **What:** Every world begins with 3–4 family bands, 45–120 people in all, in one start region, sharing one language (`BIO-03`, `CUL-17`).
    They have no shaped tools, no clothing and only natural shelter, and cannot make fire, though they can keep a found fire alive and carry its embers (`BIO-02`).
    This is the only starting point, chosen for play rather than taken from history: real early humans had more culture than this.
  - **Why:** Their minds are already modern, so the great early discoveries happen in play.

### 3.2 Who it's for

- `SCP-02` **Just you** *(Decided)*
  - **What:** Kindling is built for one person, on one phone:
    - no public release, store listing or tutorial, only short help cards (`PRE-40`);
    - no other phones, tablets or computers (cloud tests are a building tool, not a way to play);
    - no accounts, purchases, ads or analytics;
    - free to use your phone's own hardware (`PLT-01`).
  - **Check:** the builds hold no account, purchase, advertising or analytics code, and target only your phone (`PLT-01`).

### 3.3 How it gets built

- `SCP-03` **Alphas you can try** *(Decided)*: The game is built as a series of alphas, each a few hours of AI work, each ending with something you can install, open and try on your phone (`PRN-09`, `PRC-11`).
  Every alpha comes with its automated tests (`RES-01`).
  - **Check:** the check of `PRC-11` passes.

- `SCP-15` **Tests run in the AI's cloud sessions** *(Decided)*
  - **What:** Automated tests run in the same cloud sessions where the AI builds the game.
    The pace tests' sizes and computing are set in `RES-07`.
  - **Why:** There's nothing extra to set up or maintain; any need for more computing is raised with you first.
  - **Check:** every milestone report states where its tests ran and how much computing they used (`RES-06`).

- `SCP-16` **Milestones** *(Decided)*: The game is built in two phases, in order: pre-production (`MIL-18`), a prototype for each risk and then a vertical slice; then production, bottom up, in ten milestones, the foundations first and the game itself last.
  Each milestone is a stage of several alphas (`SCP-03`) ending with a report you review (`RES-06`).
  This file keeps each milestone's contents and order, below; the implementation plan maps every item to a milestone, and plans the next milestone's alphas and tasks.

  - `MIL-18` **Pre-production** *(Decided)*: a throwaway prototype for each risk, riskiest first, each answering one question on your phone or in the cloud: the look, a full scene, the model kit, the pace of discovery, the same bits, a thousand minds, world generation, the zoom, ecology, culture, the director, the interface, the writer and sound (`TIM-19`, `RES-05`, `TIM-07`, `WLD-11`, `PRE-03`); then the vertical slice: one band at a cliff camp through a day, at the art book's look, built in the real architecture on your phone (`PRE-01`, `VIS-14`).
    *Now possible:* seeing on your phone that the game can be made as this file describes, and judging the slice, which sets the quality bar for production.
  - `MIL-08` **Foundations** *(Decided)*: the app and its delivery to your phone; the simulation library in C++; numbers, time, the 60-day year, dates and chance, with the same bits on the phone and in the cloud (`RES-05`, `TIM-16`, `TIM-18`); entities, activities with an end (`TIM-17`), and catalogues with their checks (`MAT-13`, `MAT-17`); saves, several worlds and export (`PLT-07`, `TIM-08`, `PLT-08`); test scenes, the repeat check and the phone benchmark (`RES-21`, `PLT-04`).
    *Now possible:* the foundations working on your phone: a calendar at any speed, a crowd of markers moving smoothly, and worlds saved and reopened exactly.
  - `MIL-09` **The graphics engine** *(Decided)*: crisp 3D pixel art at the art book's look (`PRE-01`, `PRE-02`): the stable pixel grid (`PRE-22`), outlines and lit edges (`PRE-21`), colour in steps with real shadows and the light of the hours and seasons (`PRE-20`, `PRE-30`), rock faces (`PRE-23`), water (`PRE-26`), and the model kit and its textures made by code (`PRE-46`, `PRE-42`, `PRE-43`), grown from the vertical slice.
    *Now possible:* the art book's scenes drawn by the game, in portrait and landscape, that you judge against the art book before anything is built on them.
  - `MIL-10` **The world** *(Decided)*: whole worlds generated from a seed in the order of real causes (`WLD-08`, `WLD-09`), the best three offered (`WLD-10`); the map layers, and detail made on demand (`WLD-12`, `WLD-13`); the sky, climate and weather, fresh water, seas and soils (`WLD-07`, `WLD-16`, `WLD-17`, `WLD-26`, `WLD-27`); quakes and eruptions (`WLD-15`); one zoom from a cliff face to the globe (`PRE-03`, `WLD-02`), and the cut-away view (`PRE-25`).
    *Now possible:* making a world, choosing it, and flying over it through its weather and seasons.
  - `MIL-11` **Things and living nature** *(Decided)*: materials and things, with their shapes, characteristics, wear, simple physics, timers and traces (`MAT-01`, `MAT-10`, `MAT-11`, `MAT-19`, `MAT-08`); fire (`MAT-18`); plants and animals everywhere, as catalogue entries placed by rules, with their seasons and their ecology (`WLD-31`, `WLD-32`, `WLD-18`); wildfire (`WLD-28`); a card for anything you tap (`PRE-35`).
    *Now possible:* a living world with nobody in it: herds migrating, wolves hunting, plants through the seasons, and a lightning fire running through dry grass.
  - `MIL-12` **People: bodies and lives** *(Decided)*: the start's bands (`SCP-01`, `BIO-03`), with their bodies, needs and senses (`BIO-09`, `BIO-18`); everyday activities and base actions (`BIO-21`, `MAT-06`); choosing by needs, with the reasons on each person's card (`MND-07`, `MND-09`, `PRE-35`), and the mental map (`MND-28`); health, wounds, illness and plain care (`BIO-13`, `BIO-05`, `BIO-23`); pairing, births, growing up, inheritance, ageing and death (`BIO-04`, `BIO-15`, `BIO-06`, `BIO-16`); names from the language (`CUL-17`, `CUL-18`); the animals near people, with bodies and simple minds (`BIO-19`, `MND-16`).
    *Now possible:* watching bands live through their days and seasons, their births and their deaths.
  - `MIL-13` **Minds** *(Decided)*: personality, mood, feelings and breakdowns (`MND-20`, `MND-29`, `MND-19`, `MND-30`); noticing, memories and dreams (`MND-03`, `MND-18`, `MND-12`); knowledge with its source, and who knows what (`MND-04`, `MND-23`); beliefs about causes (`MND-05`); plans and ambitions (`MND-22`, `MND-32`); relationships, social acts and talk (`MND-24`, `MND-33`, `CUL-24`); the details of a mind (`PRE-14`).
    *Now possible:* reading anyone's mind, and watching friendships, quarrels and talk.
  - `MIL-14` **Crafts and discovery** *(Decided)*: blueprints, their routes and their chains (`MAT-04`, `MAT-07`, `MAT-22`); experience and skill, surprises and the four routes to discovery (`MND-06`, `MND-10`, `MND-11`); learning and teaching, and knowledge lost (`MND-13`, `CUL-01`, `CUL-02`); the reality rules (`RCK`); sharp flakes, fire-making, cooking, cord, hafting, clothing and huts; the sharp-stone test (`RES-02`).
    *Now possible:* watching someone find that struck flint gives a sharp edge, and the skill spread or be lost.
  - `MIL-15` **Culture and society** *(Decided)*: beliefs from events and about the unseen, ancestors, taboos, rites and religion (`CUL-05`, `MND-31`, `CUL-19`, `CUL-20`, `CUL-34`, `CUL-26`); kin and marriage, customs, leaders and specialists (`CUL-27`, `CUL-06`, `CUL-22`, `CUL-32`); sharing, trade, feuds and raids (`CUL-21`, `CUL-31`); bands splitting into peoples with territories (`CUL-30`, `CUL-23`); art, music, myths, style and festivals (`CUL-09`, `CUL-10`, `CUL-11`, `CUL-12`, `CUL-29`); tame dogs (`WLD-33`).
    *Now possible:* peoples spreading, splitting, fighting and marrying, with rites, myths, songs and paintings of their own.
  - `MIL-16` **The game** *(Decided)*: your powers as nature (`GOD-02`, `GOD-03`, `GOD-04`, `GOD-12`, `GOD-13`, `GOD-10`); time following your zoom, the story director, live moments and skip (`TIM-01`, `TIM-02`, `PRE-08`, `TIM-11`); the book of ages from pattern sentences, worded by the phone's writer (`PRE-05`, `PRE-37`, `PRE-41`); every screen, card and overlay (`PRE-40`, `PRE-35`, `PRE-07`); sound and the murmur (`SND-01`, `SND-11`, `SND-03`).
    *Now possible:* playing as nature: sending lightning or a dream and following what comes of it in the book of ages, with the sounds of the camp.
  - `MIL-17` **The whole arc** *(Decided)*: pottery, herding, farming, villages and copper (`RCK-04`, `WLD-33`, `CUL-28`, `RCK-08`); the full launch catalogue (`MAT-23`); every pace target met (`TIM-19`, `CUL-33`).
    *Now possible:* the whole arc, from caves to first copper, in about a century, at a watchable speed.

### 3.4 Non-goals

- `SCP-04` **No tech tree** *(Decided)*: Blueprints exist, but they are hidden and generic, found only in play, by accident, by experimenting, in dreams or by copying (`MND-11`), and never chosen from a menu or unlocked with points (`PRN-07`).
  People never choose from a list; the only list of blueprints is the one your Revelation picks from (`GOD-13`).
  - **Check:** the game has no menu, list or tree of discoveries for people to choose from, and nothing on screen shows a blueprint nobody in the world has found, apart from an idea dream and the marks on memories that can become one (`GOD-03`), and the list your Revelation picks from (`GOD-13`).
- `SCP-05` **No other human species** *(Decided)*: There is one human species, so the story stays about how one people learns.
  - **Check:** the game holds one human species, and no catalogue adds another.
- `SCP-06` **No AI language model making decisions** *(Decided)*: Our own knowledge would leak into their world (`PRN-06`).
  - **Check:** the check of `MND-01` passes.
- `SCP-07` **No goals, scores, wins or losses** *(Decided)*: It is a sandbox; the story is whatever happens.
  - **Check:** the app shows no goal, score, win or loss, and the game keeps none for you.
- `SCP-08` **No worship of the player** *(Decided)*: Your power doesn't depend on their faith, and they never learn you exist (`GOD-06`).
  - **Check:** no power's reach, strength or rest depends on what anyone believes about spirits or gods, and the check of `GOD-06` passes.
- `SCP-09` **No terraforming** *(Decided)*: You can't reshape land or add or remove species.
  You act only as nature could (`GOD-05`).
  - **Check:** every power works through a natural system (`GOD-05`); none reshapes land beyond what the quake, eruption or flood it sets off would do naturally, and none adds or removes a species.
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
  - **Check:** a search of the game finds no authored events, quests or campaign.
- `SCP-19` **No magic in the world** *(Decided)*: Nothing supernatural exists in the world's rules.
  Spirits and gods exist only in people's beliefs, and the only unseen force is you, acting through nature.
  - **Check:** every blueprint and world rule is physical or biological (`MAT-13`), and no rule of the world reads people's beliefs about spirits.
- `SCP-20` **No borrowed real cultures** *(Decided)*: Their peoples, names, languages and customs are their own.
  Nothing is copied from real cultures, and descriptions never compare them to real peoples.
  - **Check:** each milestone review finds nothing copied from or compared with a real people in names, words, customs, looks or descriptions (`CUL-17`, `BIO-22`, `PRE-17`).
- `SCP-21` **No deep science simulation** *(Decided)*: Kindling is a believable game, not a science simulation (`PRN-02`).
  It does not simulate chemistry, the balance of elements and energy, how cracks run through stone, microbes, heredity and evolution in plants and animals, insects, ice ages, tides, people changing the climate, slow changes in the land beyond rare quakes and eruptions, sediment carried by water and the slow reshaping of land, or language changing over time; things are buried by a simple rate per place and dated flood or ash layers (`MAT-08`).
  - **Why:** Each would cost a great deal and show little (`PRN-02`).
  - **Check:** each milestone review confirms that no part of the game simulates any of these.

## 4. The player as god

You are an invisible force of nature, with four kinds of power: weather, dreams, animal dreams and fortune.

### 4.1 Your role

- `GOD-01` **Role** *(Decided)*: A distant, invisible god in a sandbox: you watch everything and nudge, but never command or control anyone (`SCP-17`), so every achievement stays theirs.

- `GOD-06` **Never known** *(Decided)*
  - **What:** People and animals meet your acts only as nature, and nothing in the world can tell that an event was yours.
    They explain your acts as they explain any event, through spirits, ancestors or not at all (`MND-05`), and what they believe is their own, right or wrong.
  - **Check:** nothing a person or animal can sense, remember or believe records whether an event was your act.

- `GOD-05` **Only natural means** *(Decided)*
  - **What:** Every act is something the world itself could do there and then, through its own weather, sleep and luck; it never makes anything from nothing (`MAT-09`) or bends a rule (`PRN-12`).
    The one exception is Revelation (`GOD-13`), which gives knowledge directly, under long rests.
    There is no power to collect or spend: each works only where its natural cause is present; you choose where and when, and the world how strong; each power's item limits how many run at once and how soon it can be used again; nothing is instant, weather falls on everyone in reach, and nothing can be undone.
  - **Check:** tried across many places, seasons and targets, each power is offered only where its conditions hold and brings what a natural event of its kind and size would; a year of using every power as often as allowed never gives a place more storms, rain, dry spells or cold than its climate's worst year.

### 4.2 Your powers

- `GOD-02` **Weather and disasters** *(Decided)*
  - **What:** Seven powers over weather and land:
    - **Lightning:** under a thunderstorm, you aim its next strike at a spot; it hits the tallest thing within a few metres and can split, burn, wound or kill (`MAT-18`, `BIO-13`).
    - **Rain:** where its weather cell, about 10 km across, has cloud (`WLD-16`), rain falls on it for up to a day, or snow where it is cold enough.
    - **Storm:** where that place's climate has storms in that season, one gathers upwind, arrives within a few hours (`WLD-16`), with thunder where the air is warm and moist, and moves on with the wind within a day.
    - **Drought:** only where that place's climate has dry spells that long in that season, rain is held away from a stretch of land up to about 50 km across for up to a season.
    - **Cold snap:** a stretch of land up to about 50 km across gets up to three days as cold as that place ever gets in that season, so no frost where that season never has any.
    - **Flood:** where a storm could come, it stalls over a river's upper valley for a day, and over the next day or two the river spills over its banks downstream, as far as its valley allows (`WLD-17`).
    - **Quake or eruption:** only on a fault or at a volcano the world has (`WLD-15`); you choose when, and the world sets how big.
  - **Rest:**
    - after your lightning strikes, none of yours within about 10 km for a day;
    - one storm of yours at a time, a flood's included; after it passes, none within about 50 km for three days (tuned), and never more in a place's season than its climate's stormiest season gives;
    - one rain of yours at a time, none on the same weather cell for a day after, and never more rain in a place's season than its climate's wettest season gives;
    - one drought and one cold snap at a time; no drought of yours on a place within a year of the last one ending there, nor a cold snap within a season of the last;
    - one quake or eruption of yours at a time in the world, none within a year of your last (tuned), and a fault or volcano you set off then rests as long as after a natural one.
  - **Done when:** in test scenes, each power brings its event in its stated time, and only where its conditions hold (`GOD-11`).

- `GOD-03` **Dreams** *(Decided)*
  - **What:** You can choose someone's next dream, from what they have seen and done, in one of five kinds:
    - **a place** they know (`MND-28`): they feel drawn to go there;
    - **an animal** they have met: they feel drawn to hunt, watch or feed it;
    - **a person** they know: they feel drawn to seek them out, to talk, help, court or make peace;
    - **a fear** of a place, an animal or a person they know: they keep away from it;
    - **an idea:** you pick a memory of something they did or handled, and the dream sets it beside what a blueprint new to them could give, such as the smoking stick beside a fire's warmth.
      The blueprint must start from that memory, and they must know its action and have handled things like its inputs; if several fit, the one in their most experienced sector, then the one they are likeliest to succeed at.
      They wake with a hunch that lasts and is tried as any hunch is (`MND-11`).
  - **How strong:** a dream's pull lasts a few days (tuned).
    It can tip a close choice (`MND-09`), but never beats hunger, danger or a plan already under way.
  - **Limits and costs:**
    - one dream per sleeper per night, and up to three dreams a night in all, animal dreams included;
    - only memories that lead to such a blueprint can become an idea; marking them is the only hint of hidden blueprints the game gives, and never shows what they make (`SCP-04`);
    - your dream replaces that night's own and any hint it might have brought (`MND-12`);
    - sending the same dream again keeps it fresh but doesn't make it stronger.
  - **Done when:** in a scene where someone once twirled a stick against dry wood, an idea dream of it leads to new tries within a few days in at least 15 of 20 runs (`RES-13`).

- `GOD-12` **Animal dreams** *(Decided)*
  - **What:** While an animal rests, or at night for a herd far from people, you can send it one of three simple dreams:
    - **toward a place** within its range, such as the scraps at a camp's edge: over the next few days it drifts there, but a herd never leaves its range for that season (`WLD-32`);
    - **calmer:** for a few days it startles less, flees later and fights less, so it is easier to approach, hunt or tame (`WLD-33`);
    - **bolder:** for a few days it comes closer to people and camps and stands its ground, but is also quicker to fight when cornered.
  - **Far from people,** a herd is a count, so calmer or bolder lowers its wariness of people for those days (`WLD-32`).
  - **Near people,** a herd follows its lead animal (`MND-16`), so a dream toward a place moves it only when sent to the lead, which the ring marks; calmer or bolder changes only the animal dreamt.
  - **Limits and costs:** one dream per animal or herd per night, within the three a night, repeated as `GOD-03` says.
  - **Done when:** a herd sent toward a place in its range reaches it within a few days in at least 15 of 20 runs (`RES-13`).

- `GOD-04` **Fortune** *(Decided)*
  - **What:** You can bless or curse one person for a day, a season or a year.
    Fortune changes only their luck: which possible outcome comes about in what they do and what befalls them, such as a hunt, a try at a blueprint, a birth or an illness.
    It never changes what is possible, and never touches their choices.
  - **How strong:** a blessed person's rolls are made twice and the outcome better for them is kept; for a cursed person, half the time a roll is made twice and the worse is kept.
    Better and worse follow the order the rule gives its outcomes for that person, and the second roll is its own draw (`TIM-16`); where one roll matters to two people with your fortune, their fortunes cancel.
    So a blessing never more than doubles a chance and a curse never more than halves one: 10% becomes about 19% or 5.5%.
  - **Limits:** one fortune per person at a time, none on them again until as long has passed as it lasted, and at most three people with your fortune at once.
  - **Done when:** over at least 1,000 test rolls, blessed and cursed chances match the rule as `RES-13` sets out, and nothing outside chance changes.

- `GOD-13` **Revelation** *(Decided)*: You can make one person, or everyone alive, know a blueprint at once.
  - **What:** You pick a named result from the catalogue and a scope.
    One person: they know its blueprint at once, at the lowest skill.
    Everyone: every living person knows it at that skill.
    Knowing is not making: they still need the right things, the right place and practice to succeed (`MAT-04`, `MND-06`).
    They explain the sudden knowing their own way, as a dream, a spirit's gift or their own cleverness (`MND-05`, `CUL-05`).
    The book of ages records its first use as their discovery, and only your record shows it was you (`GOD-07`, `GOD-08`).
    It is the one power beyond natural means, the exception that `PRN-01` and `GOD-05` name.
  - **Limits:** after a revelation to one person, about a game year must pass before the next; after one to everyone, about a game century.
    Pace tests never use it (`TIM-19`).
  - **On the phone:** choose Revelation, pick a sector and a named result from the list, then tap a person or "everyone".
  - **Done when:** in a scene, a person given a revelation uses the blueprint within 5 days when it serves their needs and its inputs are in reach, in at least 18 of 20 runs; a revelation to everyone reaches every living person; the rests hold.

### 4.3 Using your powers

- `GOD-10` **Using your powers on the phone** *(Decided)*
  - **Touch first:** long-press a person, animal, herd or place, and a ring shows the powers possible there (`PRE-33`, `GOD-11`).
    While open, it shows faint marks of your acts at work nearby and, at region zoom, the faults and volcanoes that can be set off.
  - **Weather:** pick a power and tap the spot, aiming lightning at camp zoom or closer; for a drought or cold snap, draw the stretch of land and pick how long.
  - **Dreams:** on a person, asleep or awake, pick the kind of dream and its subject; an awake person's dream waits for their next sleep, and is dropped if its subject is gone by then.
    Each kind offers at most about 8 subjects, strongest and most recent first.
  - **Animal dreams:** on a resting animal (for a herd near people, its lead), or a herd far from people at night, pick calmer, bolder, or a place, then tap the place.
  - **Fortune:** on a person, pick bless or curse, then a day, a season or a year.
  - **Confirm or cancel:** time pauses while you choose (`TIM-15`), and cancelling leaves everything as it was.
    A drought, cold snap or fortune can later be ended early from its page (`GOD-09`); a storm runs its course.
  - **Done when:** on the phone, every power takes at most five touches, or four and a drawn shape.

- `GOD-11` **What's possible here** *(Decided)*: The ring offers only what nature could do there, or to that being, right now.
  Each power that isn't possible says why in a few words, such as "no storm overhead" or "resting: ready in 2 days".
  - **Done when:** in test scenes, every power the ring leaves out shows its reason, and the world carries out every act the ring offers.

### 4.4 Records of your acts

- `GOD-08` **Recorded behind the scenes** *(Decided)*: Every act is saved with the world's history: when, where, on whom, and every choice you made (`PRN-15`).
  It feeds only your marked lines and the pages of what came of your acts (`GOD-09`), never a mind or a text (`GOD-06`, `GOD-07`).
  - **Done when:** a test world saved and reopened lists every act with its date, place, target and choices.

- `GOD-07` **No trace in the story** *(Decided)*: The book of ages, live moments and every text written for you tell what happened, never that it was your doing.
  If you choose, your acts show beside the story as separate lines marked as yours, never woven into the text (`PRE-05`).
  - **Check:** a test runs a world with many acts and finds no mention of them in any entry, live moment or text, apart from the marked lines.

- `GOD-09` **What came of your acts** *(Decided)*
  - **What:** For each act, a page shows its date, place and target, and what followed, such as what your lightning hit, each roll your fortune turned, what a dreamer did and any discovery it led to, or where a herd you drew went.
    You reach it from the act's marked line in the book of ages (`PRE-05`), from the details of a person it touched (`PRE-14`), or from its mark in the world while it lasts (`GOD-10`).
  - **How it works:** a page shows only what the world records with its cause, never a guess: for weather and land, the deaths, wounds, fires, floods and fallen shelters it caused in the land it covered, while it lasted and a day after (`BIO-14`).
    For a dream, the dreamer's tries at its subject and any discovery, with its line of teaching (`MND-13`); for an animal dream, where it went while it lasted.
  - **Done when:** in a test scene, the page of an idea dream that led to a discovery shows the discovery and everyone taught it since, and a cold snap's page lists exactly the deaths by cold in its stretch while it lasted and a day after.

## 5. Time and history

Time works one way for everything: activities that start and end on one world clock, through a 60-day year, by the same rules at every zoom and speed.

### 5.1 How time works

- `TIM-17` **Activities with an end** *(Decided)*
  - **What:** Everything people, and animals near people, do is an activity with a start and an end: a walk to the spring, knapping flint, a meal, a night's sleep.
    Its results land when it ends, and anything can be interrupted, by the same rules for everyone, wherever they are and at every zoom and speed, so looking changes nothing (`WLD-13`).
  - **How it works:**
    - **Length:** each activity lasts as long as it would in life (`TIM-18`).
      Work with one blueprint is one activity of repeated tries, each landing as it ends, until the aim the choice was for is met (such as the flakes a task needs) or about an hour has passed (tuned).
      So a person's day holds about 10–30 activities, and each unknown blueprint the work fits is rolled once, at its end (`MND-11`).
    - **Waiting:** a blueprint's timer (`MAT-04`, `MAT-19`) needs nobody unless it asks for tending, such as feeding a kiln's fire, done in short activities while it runs; untended, it fails.
    - **Between ends:** needs change with time and effort (`BIO-09`), and at each end the doer chooses again (`MND-09`, `MND-16`).
    - **Interruptions,** the one list: an activity ends early only for danger noticed (`MND-03`), pain, a blow or a fall, a call to the doer, a need falling below 20 (`MND-07`), a plan's time (`MND-22`) or death.
      Anything else waits for the next choice, and talk alongside work ends no activity (`MND-33`).
    - **A call** is a shout to someone by name or kin word, to warn them, ask for help or fetch them; chat, telling and asking in passing are talk (`MND-33`).
    - **Can't go on:** an activity also ends, keeping what it reached, when its tool breaks, its inputs or the thing it works run out or are taken, its fire dies, what it seeks is found or lost from sight, or a shared one falls below its fewest.
    - **What an interruption keeps:** a walker stands where they got to; what builds up as it goes (eating, drinking, rest, warming, talk, teaching, watching) gives the share reached; work on a thing stays in it, so a half-scraped hide needs only the time left from whoever takes it up; a single act cut short, such as a strike or a throw, does nothing.
    - **Shared activities:** whoever starts one sets its place and time, now or within about a day (`CUL-22`, `CUL-34`), and tells or calls the others; each who joins (`MND-09`) holds it as a plan step for that time (`MND-22`).
      It begins once its fewest are there (two for a talk, `CUL-22` for a group plan), and fails if they aren't there within about an hour of its time (tuned), each then choosing again.
      Latecomers may join, except for a rite or dance, which all start and end together (`CUL-34`).
      Results land for each as they leave or as it ends (a story heard, a skill taught, a share of the meat); it counts as one activity for each, and shared work on one result follows `MAT-04`.
    - **On the way:** where a walker is at any moment follows from the way and their pace, so they can be seen, met or attacked on the way (`MND-03`).
    - **Chases and fights** are short activities of a few seconds (tuned), chosen again as the other moves, so a pursuer follows a turning quarry.
    - **Things and the land** follow `MAT-18`, `MAT-19` and `WLD-12`, on the same clock.
    - **Same at any speed:** whatever ends at a given moment ends then, at any speed, and things that end at the same moment are always settled in the same order.
  - **Done when:** in test scenes, a meal cut short halfway meets about half the hunger, a half-scraped hide is finished by another in the time left, and a learner called away keeps the skill reached so far.
  - **Check:** the same saved world, run at different speeds and zooms, gives the same results (`TIM-16`), and a scene run at real speed and at top speed has every walker meet, see and flee at the same game second.

- `TIM-18` **The game year** *(Decided)*
  - **What:** A game year is 60 days: four seasons of 15 days, spring, summer, autumn and winter, and a day has 24 hours.
  - **The rule:** what takes up to about two weeks in life takes its real time; anything measured in months or years takes about a sixth of it, so a year becomes a game year.
    In between, such as healing, starving, scurvy and long illnesses, each item gives its game length, tuned (`BIO-05`, `BIO-09`, `BIO-10`, `BIO-13`).
    So meat rots and hides dry in real time, while growing up, ageing, pregnancy, the growth of trees and crops, and grief are squeezed (`BIO-04`, `WLD-31`, `MND-19`).
  - **Daily rates, yearly totals:** eating, drinking, tiring, the time work takes, walking, weather and accidents keep their real rate or chance per day.
    What comes a few times a year in life comes as often per game year: births, a crop, a herd's young, illness outbreaks starting, droughts, floods, wildfires, harsh winters and quakes; the land's yields are scaled to match (`WLD-30`).
    Deaths before old age add up per game year to real foragers' yearly rate through illness and birth risks (`BIO-04`, `BIO-05`), never by more accidents.
  - **Slow build-ups,** such as skill, wear, tameness, trodden paths and opinions drifting, change about six times as much a day as in life (`MND-06`, `MAT-20`, `RCK-24`, `MAT-08`, `MND-24`).
  - **The land** too: in a dry spell, springs, streams and lakes fall and plants wilt about six times as fast as in life (`WLD-17`, `WLD-31`); the one exception is the moon, full once a season (`WLD-07`).
  - **Check:** each duration in the catalogues records its length in life and in the game, and a test checks the rule; in a scene, a season without rain dries small streams and browns grass as a dry summer would in life.

- `TIM-14` **Dates** *(Decided)*: Dates give the year, the season and the day, such as "Year 112, autumn, day 6".
  - **How it works:**
    - **Years** count from the start of history, which begins on Year 1, spring, day 1; each season's days run from 1 to 15 (`TIM-18`).
    - **One calendar:** seasons are named as in the half of the world where history begins, and are reversed in the other half (`WLD-01`), but dates keep the same names everywhere.
      Where a people's own season differs, the book of ages adds it, such as "Year 140, winter (their summer), day 3".
    - **Game time throughout:** ages, durations, targets and tests in this file are in game days and game years (`TIM-18`).
  - **Done when:** every date shown reads as year, season and day, and entries from the other half of the world add their own season.

### 5.2 How fast time runs

- `TIM-01` **Time follows zoom** *(Decided)*
  - **What:** By default, the closer you look, the slower time runs, and the further out, the faster.
    One gesture sets both where you look and how fast history moves (`PRE-33`).
  - **Speeds zoom asks for** at the stops of `PRE-03`, changing smoothly in between:
    - **person:** real speed (`TIM-10`);
    - **close camp:** about an hour a minute;
    - **camp:** a day in a few minutes;
    - **valley:** a season in about a minute;
    - **region:** a few years a minute;
    - **world map and globe:** top speed, as fast as the phone can (`TIM-07`).
  - **How it works:** time runs at the speed asked, or as fast as the phone can while the screen stays smooth, whichever is slower (`PRN-11`), and the speed shown is always the real one.
  - **Done when:** on the phone, each zoom reaches its speed or shows the real one, and zooming changes speed with no stutter (`PLT-04`).

- `TIM-10` **Natural speed up close** *(Decided)*: At the closest zoom, one game second passes each real second, so you can watch a flake come off the stone.
  - **Done when:** at the person zoom on the phone, a game minute takes a real minute, within about a second, with every figure moving.

- `TIM-04` **Manual control** *(Decided)*: You can unlink speed from zoom whenever you want (`PRE-33`).
  - **How it works:** pause stops time; play hands the speed back to zoom; the dial sets a speed, from real to top speed, that holds wherever you look; and the lock keeps the current speed while you move the camera.
  - **Done when:** on the phone, the dial's and the lock's speeds hold as you zoom and move, and play hands the speed back to zoom.

- `TIM-15` **Who sets the speed** *(Decided)*: Your controls beat the story director, and the director beats zoom.
  - **How it works:** the first of these that is active sets the speed, and none can make time run faster than the phone can (`PRN-11`):
    1. **pause,** yours or while you choose a power (`GOD-10`): no time passes;
    2. **skip** (`TIM-11`): top speed until the next important moment, then the director's speed for it;
    3. **the dial or the lock** (`TIM-04`): the speed you set;
    4. **the story director** (`TIM-02`): a slower speed around an important moment;
    5. **zoom** (`TIM-01`): at all other times.
  - **Done when:** a test sets each pair of controls at once and finds the stated order.

- `TIM-07` **Speed target** *(To test)*: How fast history can run at the world view on your phone, in game years per real minute, by the number of people.
  - **Targets:**
    - **the world alone,** with no people: at least 10;
    - **about 100–300 people:** at least 5;
    - **1,000 people:** at least 1, aiming for 2–3;
    - **about 2,000 people:** at least half;
    - **beyond,** time slows (`MND-15`), down to at least a sixth at about 7,000, the most a world holds by Year 250 (`BIO-04`);
    - **an old world:** a world 250 years old runs at least four fifths as fast as a new one with as many people.
  - **What counts:** every activity of people and of animals near people, every result inside an activity, every notice and every topic passed in talk, on the cores `PLT-01` allows.
  - **Check:** the benchmark worlds of `PLT-04`, on your phone at held speed (`PLT-01`).

### 5.3 The story director

- `TIM-02` **Story director** *(Decided)*
  - **What:** The director watches the whole world for important moments and sets the speed of time around them.
    Quiet years race past at the speed your zoom asks; when something important happens, or is about to, time slows, a live moment appears (`PRE-08`), and one tap takes you there.
  - **What counts as important,** as the game recognises it (`PRE-39`): named discoveries and other firsts (`MAT-21`); births and deaths among those you follow (`PRE-06`); crafts reaching a new people or lost with their last holder (`CUL-02`); a band losing its last fire (`VIS-11`); fights, raids, feuds (`CUL-31`) and disasters (`WLD-22`); peoples forming, splitting or dying out, and villages founded (`CUL-23`, `CUL-28`); what follows your acts (`GOD-09`).
  - **Signs,** the only ones it watches, so time can slow before an outcome without ever looking ahead: someone trying the same hunch several times in a day; a predator stalking a person; a storm forming over a camp; two groups on bad terms within sight of each other; someone you follow badly wounded or gravely ill.
  - **How it works:**
    - **Importance:** each kind of moment or sign has a score, higher the more people it touches and when you follow them, tuned with you; at the Gentle level, dark events don't count (`PRE-18`).
    - **Slowing down:** when a score passes the bar for live moments (`PRE-08`), time slows so that what is about to happen would take about half a minute of real time (tuned): near real speed for a stalk or two hostile groups meeting, camp speed for a birth or a morning at a fire stick, and between camp and valley speed for a flood rising over a day.
      A higher score makes a slowdown likelier, not slower.
    - **One budget** for slowdowns and live moments: at most one every about 3 real minutes, and never more than a fifth of the time slowed; a slowdown you don't tap ends after about 10 seconds (tuned).
    - **The list:** moments the budget, the dial or the lock keeps from slowing time wait in the list (`PRE-08`), and skip's stop is outside the budget (`TIM-11`).
    - **Only slower:** the director never asks for a faster speed than zoom does.
  - **Done when:** watched from the globe for an hour, test worlds list every named discovery and death of someone you follow, slow for those the budget allows, and keep four fifths of top speed.

- `TIM-03` **The director never touches events** *(Decided)*: The director sets only the speed and the live moments; it never causes, changes or hides anything.
  - **Check:** the same saved world, run with the director on and off, gives the same results (`TIM-16`), and a code check finds no path from the director into the world.

- `TIM-11` **Skip to the next moment** *(Decided)*: A control that runs time at top speed until the next important moment, for short check-ins (`VIS-10`).
  - **How it works:** at the director's next moment past the bar (`TIM-02`), it slows to the director's speed, even over the dial or the lock, until you tap or it passes, then hands the speed back (`TIM-15`), or after a year with no such moment.
  - **Done when:** in test worlds, skip stops at the next moment past the bar or after a year, and only then hands the speed back.

### 5.4 While you're away

- `TIM-05` **Pauses when closed** *(Decided)*: When the app is closed or in the background, the world stops and is saved at that moment (`PLT-07`).
  Reopening carries on from that moment, every activity where it was.
  - **Done when:** closing and reopening the app at any moment gives the same history as never closing it (`TIM-16`).

### 5.5 Worlds and chance

- `TIM-16` **Chance is local** *(Decided)*: Each chance event belongs to one being and one moment, so on the same version, the same saved state always gives the same result.
  - **How it works:** every chance draw depends only on its world, system, being, moment and purpose.
  - **Check:** the repeat check (`PRC-10`) and the phone and cloud match (`RES-05`).

- `TIM-08` **Saved worlds** *(Decided)*: Several worlds are kept on the phone, and you can switch between them.
  - **How it works:** each world keeps its seed and the version of its making rules and values (`WLD-08`) and what `PLT-10` lists; switching saves the current world and opens the other.
  - **Done when:** switching among three saved worlds opens each exactly where it was left, with its book of ages.

### 5.6 Pace and endings

- `TIM-19` **Pace of discovery** *(To test)*: In typical worlds, each step of the arc first happens within its window of years.
  - **The windows,** as dates (`TIM-14`):
    - sharp stone flakes: Years 1–2;
    - making fire: 2–8;
    - clothing and huts, each: 3–10;
    - pottery: 15–38;
    - tame dogs: 20–38;
    - herding: 30–63;
    - villages: 25–75;
    - farming: 50–88;
    - copper: 75–125.
  - **When a step counts:** at its first entry in the book of ages anywhere in the world, with crafts counted by the results `MAT-23` marks for each step (smelted copper, not hammered native copper), tame dogs and herding as `WLD-33` defines them, villages as `CUL-28` defines them, and farming at the first crop sown and harvested on purpose (`RCK-23`).
  - **How it is met:** by tuning alone, never by scripting or dates (`PRN-17`, `RES-16`).
  - **Check:** the pace tests (`RES-07`), run with no acts of yours.

- `TIM-09` **If everyone dies** *(Decided)*: The world goes on without them, and you can keep watching or start a new world (`WLD-10`).
  - **How it works:** the last death is an important moment and a book-of-ages entry (`TIM-02`, `PRE-05`), and nature runs on, faster.
  - **Done when:** a test world whose last person dies runs on, with that death in the book of ages and the choice offered.

## 6. World

The world is a small planet that wraps around, made once in realistic detail and then kept alive at a pace the phone can carry, in full detail only where people are (`WLD-12`).
Numbers here are starting values, tuned in tests (`PRN-17`).

### 6.1 Shape and size

- `WLD-01` **Torus with latitude** *(Decided)*: The map wraps both ways, with the equator across the middle and the poles along the line where it wraps north to south, so seasons behave as on a planet, reversed between the halves.
  - **The polar seam:** a permanent ice cap about 200 km wide lies along that line, and nothing ever crosses its middle, not even weather or well-supplied people.
  - **Done when:** in a test world, a walk due east comes back from the west, and in 100 years nothing crosses the middle of the polar ice.

- `WLD-02` **Globe view** *(Decided)*: Fully zoomed out, the world is a globe, the polar ice hiding the seam; the globe squeezes the polar lands in the picture only.
  - **Done when:** zooming out from anywhere ends on a globe with no seam in view, and world cells are about 1 km across at every latitude.

- `WLD-03` **Size** *(Decided)*: About 1,000 km from pole to pole and 2,000 km around, about 2 million km² of land and sea.
  - **Done when:** every generated world is about 2,000 by 1,000 world cells (`WLD-12`).

- `WLD-30` **What scales with the world** *(Decided)*: Distances are scaled to this small world, about 1 to 20, and what the land gives in a year to the 60-day year (`TIM-18`); bodies, materials and heights keep their real size, and winds their real speed.
  - **How it works:**
    - **Distances:** storm systems are about 50 km across, not 1,000, and herds migrate tens of kilometres, not hundreds; thunderstorms keep their real size.
    - **Heights:** peaks reach about 3,000–4,500 m and most land lies below 1,000 m, in ranges a few tens of kilometres across.
    - **The land's numbers:** a plant's yield in a game year and a field's harvest are about a sixth of a real year's, and wild animals live at about a sixth of their real numbers per km², breeding once a game year.
      Animals eat a real day's food each game day, so food alone would carry them to Earth's numbers: `WLD-18` holds each kind to its sixth.
      What people dig, cut or strip from a plant (roots, reeds, bark, fibre, and the dead wood a stand drops) is its yield, like fruit; only felling a tree or digging out a plant to move it takes it whole.
      So the land yields per game day, meat, roots and wood included, about what it does per real day, and feeds about as many people per km² as on Earth (`WLD-04`).
    - **Events** come about as often per game year as on Earth per year (`TIM-18`); where daily weather gives too few, such as lightning fires and storm floods, the chance per storm is raised to match.
  - **Check:** every scaled catalogue value is marked, with its Earth value; in a scene, a band of 25 on good temperate land needs about 100–300 km² to forage all year.

- `WLD-04` **How many people it can feed** *(To test)*: Estimated at tens of thousands of foragers, about one per 10 km² of good land, and ten to a hundred times more with farming.
  It is far above what the phone runs (`MND-15`), so the land limits people only locally.

### 6.2 Layers and systems

- `WLD-12` **Map layers** *(Decided)*: The world is held in four layers, each at its own pace, with detail made only where needed, and looking changes nothing (`WLD-13`).
  1. **World cells,** about 1 km across, about 2 million: height, rock, soil, biome and plant cover, water, deposits, snow, fire, paths, and the herds passing through.
  2. **Areas,** about 256 m across, 16 to a cell, detailed to about 1 m: the ground's shape and material, stones, single trees and bushes, ground cover, caves, overhangs, cliffs and water.
  3. **Things and creatures,** in areas, or on the cells' ground where no area is made: every person, every animal near people, every item (`MAT-10`).
  4. **Weather cells,** about 10 km across, about 20,000 (`WLD-16`).
  - **How it works:**
    - **Paces,** on one world clock, the same in every world and run: weather hourly; a wildfire hourly while it burns; water, snow and ice, fuel dryness and herds daily, water hourly in a flood; plant cover, small animals' counts, sea cells and soils every 5 days; people, animals and things by their activities (`TIM-17`).
      If a layer runs over its share in `PLT-04`'s measurements, a later version makes its pace coarser everywhere alike, never during play and never as less detail where people are.
    - **Where areas are made:** where people stop to do something, or where what they act on stands, such as a stalked deer, or something happens to them; and for the picture only, within about 300 m of where the camera looks, from camp zoom inward, with coarse ground farther out (`PRE-03`).
      An area made for people holds only what the rules read, all of layer 2 except the ground's shape at metre scale, which is made only for the picture (`WLD-13`).
      Walks between areas cross the cells' ground, whose slope, cover, fords and paths set their time and route (`TIM-17`); walkers notice what cells hold.
    - **Unchanged areas** are exactly what the seed, the world cell and its neighbours, and the date give, and run no rules of their own (`WLD-13`).
      Ground, cliffs and woods run on across edges, and each river's line is fixed at generation, so every area along it agrees.
    - **What is kept:** only what people and their tame animals changed: things made, moved or left, carcasses, and marks such as stumps, pits, hearths, trodden camp ground, plots, planted or stripped plants, and burned ground.
      Each mark fades in its own time (`MAT-08`), and an area with nothing left is unchanged again; the rest of a kept area comes from the seed and its cell.
      A kept area runs only while someone is within about 1 km.
      When someone comes near, or the picture shows it, it is brought up to date day by day: what its cell went through (fire, flood, lava, a quake) acts on it, and its timers and marks run on its cell's usual weather for each season (`WLD-16`), each chance drawn for its day (`TIM-16`), so it comes out the same however late or often, and the picture saves nothing (`WLD-13`).
    - **On the cell:** growth and seasons, wild grazing, wildfire, floods and snow act on the world cell; a grove cut, a field cleared or an animal killed comes off its cover or herd when it happens, and the cell's own regrowth gives it back.
      Paths are worn on the cells a walk crosses, counted once per walk, and fade after a few unused years (`MAT-08`); areas draw them from their cell.
  - **Done when:** an unchanged area remade on any day is identical; a kept camp brought up to date daily, or once after a season, ends the same; neighbouring areas meet within about 0.5 m; rivers never break.

- `WLD-13` **Looking changes nothing** *(Decided)*: Where you look, and how fast time runs, never change what happens; unvisited places are drawn from the seed and their world cell, as people would find them (`PRN-10`).
  - **How it works:** the picture and sound read the world, and nothing in the world reads the camera, the zoom, the speed or whether an area is made (`WLD-12`); "near" always means near a person.
    Counted herds are drawn as that many animals, placed from the seed.
  - **Check:** the same saved world gives exactly the same results whatever the camera and the speed (`TIM-16`).

### 6.3 Making a world

- `WLD-08` **Realistic, not from scratch** *(Decided)*: Each world is made directly in a believable present-day state by fast rules that imitate what deep time would make (`SCP-12`): the stages of `WLD-09`, in order from the seed, make the world and weather cells, never areas (`WLD-12`).
  - **Repeatable:** a seed makes the same world and areas only under the same rules for making them and the same values that place and size land, plants and stones; changing these is a big update (`PLT-09`), while tuning yields, timings or chances is not.
  - **Settling:** once you pick a world, its plant cover, water and herds run for 10 years by the play rules, with no people; animals near the start region begin as wary as hunted ones (`WLD-32`); then history begins (`TIM-14`).
  - **Done when:** in 20 test worlds, 95% of rivers draining 50 km² or more reach sea or lake; median slope 0.5–5°, under 1% over 30°; no coast straight for over about 20 km; lakes 1–3% of land; you review them (`MIL-10`).

- `WLD-09` **What generation makes** *(Decided)*: Generation follows the real order of causes, each stage built on those before:
  1. **Plates,** about 6 to 12, ocean or continental, making the world's share of land (`WLD-06`), with ranges, volcano lines, rifts and faults along their edges as on Earth.
  2. **Rock,** about 12 kinds: granite, basalt, lava and ash, glassy lava that gives obsidian, sandstone, shale, limestone, chalk, quartzite, slate, river gravel and silt.
     Each cell has its surface rock and up to two layers below, set by its place among the plates: old worn-down land, old sea basins, folded ranges, volcanoes, rifts or sea floor.
  3. **Erosion:** rain from a first, rough climate gathers into rivers that cut valleys, deepest where most water flows on steep slopes and soft rock, and hollows fill into lakes, so every river reaches the sea or a lake; simple rules then add floodplains along big rivers, fans at mountain feet and deltas at river mouths.
     Caves form in limestone, old lava tubes and soft rock under hard; each cell records its caves and their sizes, their shape coming with their area (`WLD-12`).
  4. **Climate** (`WLD-16`).
  5. **Soils** (`WLD-27`).
  6. **Deposits** (`WLD-14`).
  7. **Biomes and plant cover** (`WLD-31`), each cell's cover at some stage of regrowth since its last fire or flood.
  8. **Animals,** placed where their food and cover are, at the numbers `WLD-18` holds them to, each herd with its seasonal ranges (`WLD-32`).
  - **Done when:** in 20 test worlds, ranges, volcanoes and faults lie along plate edges, and chalk and limestone only where seas or basins lay.

- `WLD-14` **Deposits placed by geology** *(Decided)*: Useful stone, clay, ochre and ore lie where the rocks and rivers put them.
  - **Where each lies:** flint as nodules in chalk, and chert in some limestones; obsidian in young lava from volcanoes with thick, sticky lava; hammer and grinding stones where quartzite, basalt or sandstone shows; clay along rivers, in old river bends and lake beds, and where granite and similar rock rots; red and yellow ochre where iron-rich rock weathers; green and blue copper ore at the weathered tops of copper-bearing rock near granite in volcanic ranges, and rarely, only in those ore cells, a little native copper.
  - **How it works:** each cell records its deposits and how rich each is; rivers carry stones downstream, rounded with distance, so flint turns up in gravels far from its chalk.
    Deposits show where the land is cut, such as banks, cliffs and cave walls, and look like what they are, while their uses are learned (`MAT-03`); deeper ones need digging (`MAT-06`).
    In areas, loose stones lie as ground, per patch like ground cover (`WLD-31`); one someone takes becomes a thing, its quality drawn from the seed (`MAT-20`).
  - **Done when:** in 100 test worlds, flint lies only in chalk, some limestone and gravel below them, obsidian only near young volcanoes, and copper ore only in volcanic ranges near granite.

- `WLD-10` **Generate several, offer the best three** *(Decided)*: The game makes several candidate worlds, scores them and never edits them; "New world" shows the best three as small globes, each with a one-line summary, and you pick one, let the game pick, or enter a seed.
  - **How it works:**
    - **Two passes:** about 20 candidates (set to fit `WLD-11`) go through plates, rock, erosion, a rough climate, deposits and rough biomes; the best few get every stage and are scored again.
    - **Must have:** a start region that qualifies (`WLD-24`) and, on its landmass, all the arc needs (`TIM-19`): stone that flakes, clay, wild grains, wolves, a herd animal with a domestic kind (`WLD-33`), and copper ore.
    - **Scores,** weighted: varied land and climates, barriers that let separate peoples form, unevenly spread resources, and the start region's own score.
    - **If fewer than three qualify,** more are made, at most about 40, and those that qualify are offered; your own seed makes one world and finds its start region the same way, or says it has none.
  - **Done when:** over 100 seeds, at least 1 candidate in 4 qualifies, every offered world has every must-have, and a seed always offers the same three.

- `WLD-24` **Where history begins** *(Decided)*: The bands start in a temperate region found by scoring, never placed by hand.
  - **How it works:** a region is judged before settling by what settling doesn't change (climate, biome, soil, caves, water and stone, with food estimated from biome and soil), and is kept through settling (`WLD-08`).
    It qualifies when:
    - **winters** matter, the coldest season averaging about 2–10 °C with frost on a few nights, yet naked, fireless people live through them huddled in caves (`BIO-11`);
    - each band has a dry **cave** or overhang big enough for it, and **water** that lasts all year within about 2 km;
    - **food** the bands can get with the starting kit (`BIO-02`) is enough within about 10 km of the shelters, in every season with a margin, from several kinds;
    - **stone that flakes** lies within the same reach, for the sharp-stone test (`RES-02`).
  - **Ranking:** bigger food margins, more kinds of food, and more shelters and water score higher.
  - **Done when:** in 100 test worlds, every start region still qualifies after settling.

- `WLD-11` **Generation time** *(To test)*: From "New world" to three globes takes at most about 3 minutes at the phone's held speed, and settling the chosen world, with its bands made (`BIO-03`), at most about 1 minute more.
  - **Done when:** both times are met on the phone at every stage from `MIL-10` (`PLT-04`).

### 6.4 Sky, climate and weather

- `WLD-06` **Varied within reason** *(Decided)*: Each world has its own axial tilt (15° to 30°, which sets how strong its seasons are), share of land (25% to 50%), continents, seas and stars, all from the seed; every world has one moon, a 24-hour day and the 60-day year (`TIM-18`), and Earth's gravity, air and water.
  - **Done when:** over 100 seeds, tilt and share of land always fall within their ranges and spread across them.

- `WLD-07` **The sky** *(Decided)*: Sun, moon and stars move as they would for the world's tilt and the 60-day year (`TIM-18`): daylight by latitude and date, a moon that waxes and wanes once a season, bright enough at full to walk and hunt by, and an eclipse a few times in a lifetime at any place; people can learn these cycles (`CUL-29`).
  - **Done when:** in a test world, each season has one full moon, daylight follows Earth's for each latitude and date, and each place sees a few eclipses in 70 years.

- `WLD-16` **Climate and weather** *(Decided)*: Each place has a climate that never changes (`SCP-21`), and each weather cell its weather every hour: temperature, humidity, wind, cloud, rain and snow.
  - **Climate,** worked out at generation by rules modelled on Earth's, from latitude, height (about 6 °C colder each 1,000 m up), wind belts, sea, mountains with their rain shadows, and currents (`WLD-26`); its record holds, for each place and season, the usual warmth and its extremes, rain, snow, wind and storm days, and its longest dry spell (`GOD-02`).
  - **How it works:**
    - **Seasons** come from latitude and tilt (`WLD-06`), squeezed into the 60-day year, land and sea warming and cooling within it (`TIM-18`); deserts swing most between day and night, coasts least.
      Latitude changes about a degree every 5.6 km, so climate belts are about 100 km wide.
    - **Storms** decide when and where rain and snow fall, not how much: born upwind at rates set so each place gets about its climate's storm days, they move with the winds and drop on each place what its climate gives for a storm of their size.
    - **Thunderstorms** build in warm, moist air and bring lightning (`WLD-28`).
    - **On the ground:** snow lies on each cell until it melts into the rivers (`WLD-17`); each place's weather is adjusted for its height, slope and shelter, with frost pooling in hollows.
    - **Good and bad years:** chance brings runs of wet, dry, warm and cold years as often as `WLD-30` sets, giving droughts and harsh winters (`WLD-22`).
  - **Done when:** in 20 test worlds, each band of latitude has about Earth's climates in Earth's shares, and each place's weather over 20 years averages within 10% of its climate's rain and 1 °C of its warmth, which is within about 2 °C of Earth's at the same latitude, height and distance from the sea.

### 6.5 Water and soil

- `WLD-17` **Fresh water** *(Decided)*: Rivers, lakes, springs, marshes, snow and ice, with floods and dry spells; life gathers around them (`BIO-09`).
  - **How it works:**
    - **Each cell's water:** rain and melt soak in or run off (`WLD-27`), and ground water feeds springs that keep streams flowing in dry spells.
    - **Rivers** keep their courses, each with its line, width and depth (`WLD-12`); water runs down them at real speed, so a storm upstream floods the lower valley later; in a dry spell springs, streams and lakes fall at `TIM-18`'s pace, and small streams dry up.
    - **Lakes** rise and fall with what flows in and out; marshes form on badly drained ground, with reeds and peat (`WLD-27`).
    - **Floods** from heavy rain or fast melt spread over the floodplain, drowning and carrying things, and leave silt (`WLD-27`, `MAT-08`).
    - **Ice** on rivers and lakes bears a person only once thick enough, and glaciers stay as generated.
    - **Fouled water** is running water within about 1 km downstream of a camp, a herd's crossing or a carcass in it, while they are there and a few days after, and still pools under about 50 m across in warm weather; it carries illness (`BIO-05`), while springs and water above any camp are clean.
  - **Done when:** in scenes, a storm upstream raises the lower valley about a day later, a spring-fed stream outlasts a dry spell, and drinkers below a camp get gut sickness at `BIO-05`'s chance for fouled water, those at the spring don't.

- `WLD-26` **Seas** *(Decided)*: Seas and oceans with currents that carry warmth, one fixed level with no tides (`SCP-21`), and shores rich in food.
  - **How it works:** currents set at generation warm one side of each ocean and chill the other, and rich water wells up off some coasts; each sea cell keeps its warmth, ice, fish and sea mammals (`WLD-32`), most where water wells up and in shallow seas.
    Shores are beaches, rocks, estuaries and salt marsh, with shellfish beds kept per patch like ground cover (`WLD-31`); cold seas freeze in winter, in places thick enough to walk on.
  - **Done when:** in test worlds, warm and cold coasts lie on opposite sides of each ocean, fish are richest where cold water wells up, and stripped shellfish beds regrow within a season.

- `WLD-27` **Soils** *(Decided)*: Each world cell has a soil and a fertility from 0 to 5, which decide what grows there and how well.
  - **How it works:** soil comes from what lies beneath, the climate, the slope and the plants: rich on river silt, wind-blown dust, volcanic ash and old grassland; poor on sand, steep slopes and peat, and where heavy rain in hot lands washes it out.
    Its kind sets how it holds water and how easily it is dug.
    Fertility falls with each harvest carried away, recovers over a few years of rest, and rises fast with ash, dung, rotted waste and flood silt; fields keep their own in their area (`WLD-12`).
  - **Done when:** in a scene, a field cropped every year yields less each year, recovers after a few years' rest, and rises after ash or dung.

### 6.6 Plants and animals

- `WLD-31` **Plants** *(Decided)*: About 60 Earth species or close kin, with their real habits, seasons and sizes: trees, bushes, grasses including wild grains, herbs, roots, reeds and flowers, with a few mushrooms and tinder fungi.
  Each has where it grows, its growth and its yields (fruit, nuts, seeds, wood, bark, fibre, leaves or roots), each with its characteristics (`MAT-03`) and scaled by `WLD-30`.
  - **How it works:**
    - **Single plants and ground cover:** in areas, trees and bushes are single plants, each with a growth stage (seedling, young, grown, old or dead), a season state (bud, leaf, flower, fruit or bare) and ripening yields; grass, herbs, flowers, reeds, mushrooms and seedlings are ground cover, kept per patch of a few metres: which kinds, how dense, their season state and ripe yield.
      Unchanged, both follow the cell's cover and the date (`WLD-12`); plants people changed grow, fruit, spread and die by their own rules at their cell's rates.
      Gathering takes the ripe yield from the patch or plant (`MAT-09`), which becomes things (`MAT-10`) of a size and quality drawn from the seed, the same whoever takes it; the plant stays, stripped, unless felled or dug out (`WLD-30`).
    - **Life:** each species buds, flowers, fruits and goes bare at its own time; grasses and herbs grow within a season, bushes over a few years, trees over decades (`TIM-18`); they die of drought, frost, fire, shade, grazing or age, and dead wood lies as fuel and rots (`MAT-19`).
    - **Spreading:** each season a plant may start a new one of its kind nearby, where the ground suits it and there is light and room, up to about 5,000 single plants an area; seeds people drop can sprout too (`RCK-23`).
    - **Cover per cell:** the share of trees, bushes, grass and herbs, reeds and bare ground, each with up to about four species and their shares, drawn at generation from those the cell suits; the trees' age; and what is ripe, from the species and the date.
      It grows back toward its biome at each species' pace after fire, flood or grazing.
    - **Biomes,** what a cell's climate, soil and wetness would grow if left alone: ice, tundra, northern conifer forest, broadleaf forest, grassland, dry scrub, desert, savanna, tropical forest, marsh and mountain heights, plus shores and seas.
      Each land biome but ice has at least 6 plants and 4 animals of the catalogue, and seas and shores 4 animals, a species serving several.
  - **Done when:** an area made from a cell holds its species in about their shares; in scenes, a stripped hazel fruits next autumn, a burned oak wood is young wood within about 10 years, and kept plants grow within 10% of their cell's rates; and in 20 test worlds each biome present has plant eaters, hunters, birds and fish.

- `WLD-32` **Animals** *(Decided)*: About 30 wild Earth species or close kin, no insects: mammals, birds, fish, shellfish and a few reptiles, plus their 5 domestic kinds, counted apart (`WLD-33`).
  Each has its diet, group size, speed, danger, life span, seasons (breeding, moulting, migrating, fish runs, winter sleep) and yields (`MAT-10`), with eggs from nesting birds in season and dung from herd animals.
  - **How it works:**
    - **Far from people,** big animals live in herds, packs, flocks or alone, each kept as a count of adults and young with its condition and wariness of people.
      Each herd keeps a home range of world cells for each season, summer and winter ranges where it migrates; each day's move toward food, water and cover and away from hunters is an activity along its route (`TIM-17`).
    - **Numbers** are scaled (`WLD-30`): young come in their season, and births and deaths follow `WLD-18`.
    - **Near people,** within about 1 km of a person, a herd's animals become individuals where they stand (`WLD-12`), each with a body (`BIO-19`) and a simple mind (`MND-16`), its age, sex, condition and any illness drawn from the count.
      They rejoin the count a day after nobody is near, a wound lowering the herd's condition and a fright raising its wariness; only tame (tameness 1 or more, `WLD-33`) or kept animals stay individuals.
    - **Small animals,** such as hares, small birds and most fish, stay counts everywhere, per cell or stretch of water; one caught is taken from the count (`MAT-11`).
    - **Fear of people:** a herd that loses animals to hunters, or is chased, flees sooner; where no one hunts, its wariness fades over the years.
    - **Danger:** wolves, bears and big cats may attack people when hungry, cornered or guarding young, kept off by numbers, noise and fire (`MND-16`).
  - **Done when:** in scenes, a herd hunted every autumn flees at twice an unhunted one's distance within 5 years, a herd made individuals and back keeps its total, and migrants reach their ranges in season.

- `WLD-33` **Taming and domestic kinds** *(Decided)*: Animals fed and kept near people grow tame, young born to tame animals kept by people are tame from birth, and a line kept for several generations becomes a domestic kind, such as wolf to dog.
  - **How it works:**
    - **Tameness** runs from 0 (wild) to 5 (tame): food and time near people without harm raise it slowly, and harm, or a season apart from people, lowers it a step; young raised by people tame fast and grown animals rarely (`RCK-24`), and young taken from the wild count as raised by people (`MAT-06`).
    - **Tame animals** stay near people, can be penned or tethered, and breed when kept together; tame nursing females can be milked (`MAT-23`).
    - **Generations:** a young animal born among people counts one more than the lower of its parents' counts, a wild or wild-born parent counting 0; at 5 it is of its domestic kind.
    - **Domestic kinds** exist only for wolf, wild goat, wild sheep, wild cattle and wild boar; deer and others are tamed one by one, never into a kind.
      Domestic animals are born at tameness 5, flee half as far, and breed a year younger and every year, with their own look.
    - **A kept herd** is at least 5 tame animals of a herd species kept by one people, with young born among them; it grazes the cover around it and needs water and guarding.
  - **Done when:** in 20 runs of a scene where a band raises wolf pups and keeps their young, at least 16 have dogs within about 15 years; deer kept the same way never become a kind.

- `WLD-18` **Ecology** *(Decided)*: Plant and animal numbers boom and crash with the weather and with each other, never by script.
  - **How it works:**
    - **Plant eaters** eat the cover of their cells (grazers grass, browsers bushes and young trees, others fruit, nuts and roots), and heavy grazing thins it.
    - **Hunters** take prey by chance each day, more often when both are plentiful and the prey is young or weak; each kill is a whole animal, and its carcass feeds scavengers.
    - **Limits:** each cell holds at most a set number of each kind for its cover, a sixth of what such land holds on Earth (`WLD-30`); below it, well-fed animals breed well, and hungry ones less and die first in a hard winter; a hunter kills no more than it and its young can eat, and each year animals spread into empty, suitable cells next to occupied ones.
    - **Illness** in animals is part of their death rate by condition; nothing spreads through counted herds, and the few sick animals of kinds that carry an illness appear only near people (`BIO-05`, `BIO-19`).
  - **Done when:** in 20 test worlds run 100 years without people, each species stays within half to twice its settled total, none vanishes from its biomes, and big hunters are about 1 to 50–200 prey.

### 6.7 Fire, quakes and other events

- `WLD-28` **Fire in the landscape** *(Decided)*: Lightning and dry fuel start wildfires that spread with wind and slope, and the land regrows after them.
  - **How it works:**
    - **Fuel** is each cell's plant cover, drying or wetting with the weather: dead grass within a day of sun, forest litter over days, logs over weeks.
    - **Starting:** lightning (`WLD-16`, at the rate `WLD-30` sets), lava (`WLD-15`), or a fire people leave or set, where fuel is dry enough.
    - **Spreading:** each hour a fire burns across a cell from the side it entered, as fast as fuel, wind and slope allow (dry grass in wind up to about 5 km an hour, forest litter about 0.5), lighting the next cells as its front reaches them; water, bare rock, snow, burned ground and rain stop it.
    - **Where people are,** in the areas they are in, the front enters from that side at that time and burns across patches of ground cover, taking their plants (`MAT-18`); whatever stops it there, a stream, bare ground or people beating it out, stops it, and the cell counts only what burned.
      Elsewhere areas follow their cell, and kept ones take the fire when brought up to date (`WLD-12`).
      A fire people start spreads into the cell when it reaches its area's edge; burning land on purpose is theirs to discover (`PRN-01`).
    - **After fire,** ash raises fertility for a few years (`WLD-27`) and the cover regrows (`WLD-31`).
  - **Done when:** in test worlds, grassland burns every 2–5 years, dry forest every 20–50, wet forest rarely; in a scene, a fire stops at a river, and clearing around a camp saves it.

- `WLD-15` **Quakes and eruptions** *(Decided)*: Rare earthquakes strike along the faults made at generation, and volcanoes erupt where geology allows; nothing else reshapes the land (`SCP-21`).
  - **How it works:**
    - **How often:** each fault and volcano has its own chance each year, set by its kind, at about Earth's yearly rate for the same area (`WLD-30`); after each, it stays quiet for a time set by its kind, usually many years.
    - **Quakes** shake hardest near the fault: shelters and stacked things fall (`MAT-11`), and rocks drop from cliffs and cave roofs.
    - **Eruptions** give warning days ahead (small quakes, rumbling, warm springs) that people can notice (`BIO-18`); lava then burns and buries what lies in its path, leaving fresh lava rock, heights unchanged and no new obsidian, and ash smothers plants and fouls water nearby for a season, later enriching the soil (`WLD-27`).
  - **Done when:** in 20 test worlds of 100 years, quakes come only on faults and eruptions only at volcanoes, about as often as their kinds set, each followed by its quiet time.

- `WLD-22` **Natural events** *(Decided)*: Lightning, wildfires, storms, droughts, floods and harsh winters, and rarely quakes and eruptions, come from the world's own systems (`WLD-15`, `WLD-16`, `WLD-17`, `WLD-28`), never on a schedule; your powers work through the same systems (`GOD-05`).
  - **How it works:** big events that touch people enter the book of ages (`PRE-05`).
  - **Check:** in each climate, each kind of event comes about as often per game year as on Earth per year (`WLD-30`), and every event traces back to its system.

## 7. Things and blueprints

Everything people make on purpose is made by a blueprint; things also change by themselves, through fire, timers and the living world.
Discovery, practice and teaching are in Minds (`MND-06`, `MND-11`, `MND-13`).

### 7.1 Things

- `MAT-01` **Things are made of materials** *(Decided)*: Every thing is made of one or more materials: a spear is hazel, flint and sinew.
  - **How it works:** a material has a colour, base values for the 18 characteristics (`MAT-03`), and one of nine classes, setting its sounds (`SND-06`), whether it rots (`MAT-19`) and how long it lasts buried (`MAT-08`): stone (with ore), earth (clay, sand, ochre, ash), wood (with charcoal), plant (with dung), bone (with antler, horn, teeth, shell), hide (with fur, feathers, sinew), flesh (with fat, eggs, milk), metal and water.
  - **Done when:** the Complete check passes (`MAT-17`).

- `MAT-10` **Items** *(Decided)*: An item is a kind of thing in the catalogue, such as flint, sharp flake or sewn cloak: about 190 at launch.
  - **How it works:**
    - **About 90 raw,** each with its 18 values, class, size and a look drawn from its form's shared shape (`PRE-46`); species that yield alike share them.
    - **About 100 made,** each with its form, main material, changes (`MAT-03`), what it breaks into, and its model and icon (`MAT-21`).
    - **Also listed:** foods that taste mild despite their poison (`MND-21`), and how much of the body a garment covers (`BIO-11`).
    - **A thing** is one item in an area (`WLD-12`), with its own size, state (never a new item, `MAT-19`), wear, quality (`MAT-20`), maker, date and style (`PRE-43`); a heap of small things is one thing.
  - **Done when:** the Complete check passes (`MAT-17`).

- `MAT-02` **Shape and size matter** *(Decided)*: A thing's form (lump, flake, blade, point, rod, pole, sheet, strand, powder, paste, liquid, container or structure) and its size, in real units, count as much as its material.
  - **How it works:** form shifts some characteristics (`MAT-03`); size sets weight and amounts, and blueprints ask for sizes, such as a hut pole 2–4 m long (`MAT-04`).
  - **Done when:** the Complete check passes (`MAT-17`).

- `MAT-03` **Characteristics** *(Decided)*: Every item has the same 18 characteristics, each from 0 (none) to 5 (as much as any launch material has).
  They come from its material and its form: a flint nodule and a flint flake share flaking 5 but differ in edge.
  - **The 18,** each with a typical value:
    1. **hardness:** resists scratching and wear, the harder grinding the softer: flint 5.
    2. **edge:** how well it cuts or pierces now: flint flake 5.
    3. **toughness:** takes blows and loads unbroken: quartzite cobble 5.
    4. **flaking:** breaks into sharp, predictable flakes: granite 0.
    5. **flexibility:** bends and springs back: green hazel rod 5.
    6. **weight:** heaviness against water: 0 a tenth (feathers), 1 half (dry wood), 2 equal (flesh), 3 twice (bone), 4 nearly three times (stone), 5 nine times (copper).
    7. **burn:** how readily it catches fire: dry grass 5, rods and strands a step above their material, poles and lumps a step below.
    8. **fuel:** heat given, and for how long: charcoal 5.
    9. **food:** nourishment: fat 5.
    10. **water:** water held or given: berries 4.
    11. **poison:** harm when eaten: nightshade berries 5.
    12. **medicine:** help in healing: willow bark 3.
    13. **warmth:** warmth kept in, worn, slept on or as a wall: fur 5.
    14. **fibre:** long, strong fibre given or held: grass 2.
    15. **stickiness:** how well it glues: birch tar 5.
    16. **plasticity:** how well it takes and keeps a shape: wet clay 5.
    17. **waterproof:** how well it keeps water in or out: basket 1.
    18. **pigment:** how strongly it colours: green copper ore 3.
  - **Seen or learned:** hardness, edge, flexibility, weight, water, fibre, stickiness, plasticity and pigment are known on sight or in the hand, the rest by use or by being told (`MND-04`).
  - **Made things** start from their main input's values and change only those their blueprint sets, to a number or a step up or down (`MAT-04`); a joined thing takes each value from the part its blueprint names (`RCK-25`).
  - **Done when:** the Complete check passes (`MAT-17`).

- `MAT-20` **Wear and quality** *(Decided)*: Things wear with use and break; quality, how well a thing is made, comes from its maker's skill and its inputs.
  - **How it works:**
    - **Wear** runs from 0 (new) to 5 (broken), each step taking a step off its main characteristic; a broken thing becomes what its item breaks into, such as sherds, and blueprints can mend.
      A tool takes the wear its blueprint sets per use, halved for toughness 4–5, doubled for 0–1, so a flake dulls after butchering about one deer; clothing and bedding wear a step about every 24 days of use, a hut's cover a step a season, a basket a step every 10 days carried, and things left out by their class (`MAT-08`).
    - **Quality,** 0 to 5, is set when a thing is made: half the maker's level, rounded down (`MAT-04`), a step up for fine inputs (4–5 on average) or down for poor ones (0–1), a step either way by chance, a step up if the maker is inspired (`MND-29`); raw things take their source's quality (`WLD-14`).
    - **Effect:** for a made thing, at 0–1 the main characteristic its blueprint names is a step lower and it wears twice as fast; at 4–5 it is a step higher, wears half as fast and is prized (`MND-24`, `CUL-21`); quality never changes food or water, but fine food gives a better thought (`MND-29`).
      A raw thing's quality moves none of its values and counts only as a fine or poor input (`MAT-04`).
  - **Done when:** in trials (`RES-24`), flakes made at level 10 cut a step better and last about four times as long as at level 2.

- `MAT-09` **Nothing from nothing** *(Decided)*: Every result uses up its inputs, and nothing appears from nowhere.
  - **How it works:** a blueprint uses up what it works, while tools are kept but wear (`MAT-20`); no result outweighs its inputs and the water they soaked up.
    Gathering, butchering and digging take from plants, animals and the ground (`WLD-31`, `WLD-32`, `WLD-14`); growth and breeding are the one place new matter enters, so a sown plot weighs only its seed.
  - **Check:** running every blueprint and timer on test things finds no result heavier than its inputs or without them.

### 7.2 Actions

- `MAT-06` **Base actions** *(Decided)*: People change things with 21 base actions; every blueprint uses exactly one (`MAT-04`).
  - **The 21:**
    1. **gather:** pick up things or yields, fill a container, or take a small or young animal that can't get away.
    2. **dig:** move earth.
    3. **strike:** hit one thing with another, such as to knap stone.
    4. **press:** push or squeeze hard, such as to flake an edge with bone.
    5. **cut:** draw an edge through something.
    6. **scrape:** take a layer off with an edge, such as flesh from a hide.
    7. **grind:** rub hard against something, such as a stick along a groove.
    8. **twist:** twist fibres into cord or thread.
    9. **bind:** tie or sew, such as a point to a shaft.
    10. **weave:** interlace strands.
    11. **shape:** form something soft by hand, such as clay into a pot.
    12. **drill:** turn a pointed stick back and forth, such as to make fire.
    13. **heat:** put things in, on or by a fire, or blow on or into one by mouth, pipe or bellows (`MAT-18`).
    14. **soak:** put things in still or running water (`MAT-19`).
    15. **dry:** lay or hang things in sun and wind or by a fire.
    16. **mix:** combine soft or loose things, such as fat and pigment into paint.
    17. **stack:** set things into a heap or structure, such as a hut frame.
    18. **plant:** put seeds, roots or cuttings into the ground on purpose (`RCK-23`).
    19. **throw:** send things through the air, such as a spear at prey; a thrust is fighting (`BIO-21`).
    20. **feed:** give food to an animal or a person, or fuel to a fire.
    21. **apply:** put one thing onto another, such as paint on rock.
  - **Knowing an action:** a person knows an action once they have done it in any way, as dreams and experiments ask (`GOD-03`, `MND-11`).
  - **Plain uses:** gather, dig, throw, feed, stack, heat, soak, dry and apply also work without a blueprint, to take, move, wet, warm or dry things, can start timers (`MAT-19`), and leave heaps, holes such as a pit or grave, and cleared ground; anything else an action makes or changes, such as a frame, a sown plot or a house, needs a blueprint, and everyday activities, such as walking or fighting, are not base actions (`BIO-21`).
  - **Done when:** each base action has its animation (`PRE-44`), and its sound as `SND-06` maps it.

- `MAT-12` **What a body can do** *(Decided)*: A body does base actions within its strength, hands and health, never by rules for each blueprint.
  - **How it works:** gather, cut, throw, feed and apply need one hand, the rest two; a broken arm stops two-handed work, and other hurts, tiredness, cold and darkness slow work as `MAT-04` says.
    Heavy work, such as felling or digging pits, is slower for the weak, children and the old, and some is beyond them (`BIO-16`); children work as `BIO-04` says and copy adults in play (`MND-21`).
  - **Done when:** in scenes, someone with a broken arm does only one-handed actions, and children under 5 do no work (`BIO-04`).

- `MAT-11` **Simple physics** *(Decided)*: Things are carried, fall, float and topple by a few simple rules from their characteristics and size, with no fracture physics and no exact paths (`SCP-21`).
  - **How it works:**
    - **Carrying:** as much as the body can bear (`BIO-21`); small loose things need a container, and liquids one with waterproof 3 or more.
    - **Falling:** a dropped thing breaks if not tough enough for the fall, so a pot (toughness 1) dropped from waist height usually breaks; people who fall are hurt (`BIO-13`).
    - **Floating:** weight 0–1 floats, 2 and empty containers float low, 3–5 sinks (`RCK-21`); floating things drift (`WLD-17`).
    - **Throwing:** reach and harm come from the thrower's strength and the thing's weight and edge, within its catalogue range.
    - **Small game** (counts, `WLD-32`): at hunting experience 4, where its kind is at its usual numbers, a set snare or trap takes one a night 1 time in 10, a baited hook one an hour 1 in 20, and a throw hits 1 in 4 (tuned), in proportion to how many live there, each level above or below and fine or poor quality moving it by a tenth of itself; setting is stack or bind, and emptying is gather.
    - **Toppling:** stacks and buildings stand until strong wind, heavy snow or a quake knocks them down, poorly made ones first (`WLD-22`).
  - **Done when:** in scenes, a pot dropped from waist height breaks in at least 15 of 20 runs, a thrown spear wounds within its catalogue range, and snares catch at their stated chance.

### 7.3 Fire, timers and traces

- `MAT-18` **Fire** *(Decided)*: A fire is fuel burning at a heat level from 0 to 5: 0 out, 1 embers, 2 small fire, 3 campfire, 4 pit or kiln, 5 furnace.
  - **How it works:**
    - **Highest level:** an open fire reaches 3, a walled, covered pit or kiln 4 with fuel 3 or more, and any enclosed charcoal fire 5 while air is blown in the whole time through at least two reed or clay pipes (tuned) or a hide bellows, a blower at each, in turns; it falls to 4 within minutes when blowing stops.
    - **Level:** tinder or twigs alone burn at 2, and sticks or logs bring a fire to its highest within about 10 minutes; it drops to 1 when its fuel is spent, and to 0 a few hours later unless banked under ash, which keeps embers overnight.
    - **Lighting:** an ember in dry tinder (burn 4–5), such as a tinder nest, flares into a small fire within a minute while blown, and unblown dies within minutes about half the time (tuned, `RCK-22`); things with water 3 or more don't catch, and damp a fire.
    - **Burning:** a thing of burn 2 or more in a fire of 2 or more catches, feeds it by its fuel value and leaves ash; a campfire burns about 5 kg of dry wood an hour.
    - **Boiling:** water boils within about 20 minutes in a container of waterproof 3 or more on a fire of 2 or more, or, the only way in hide or bark, with fist-sized stones from a fire of 3.
    - **Spreading:** a fire of 2 or more lights things within about a metre, sooner the higher their burn, wind carries sparks a few metres, and a hearth ring stops it creeping; a wildfire follows `WLD-28`.
    - **Putting out:** earth, sand or water puts it out, leaving half-burnt wood as charcoal; rain lowers an open fire a level an hour, and heavy rain puts out an unroofed one.
    - **Carrying fire:** embers in tinder fungus or rotten wood last about a day on the move (`BIO-20`), a torch about an hour.
  - **Done when:** in scenes, an unfed fire dies within hours, a banked one lives through the night, and dry grass by a campfire catches while green wood does not.

- `MAT-19` **Timers** *(Decided)*: Slow changes run on things by themselves while their conditions hold, meant or not.
  - **How it works:**
    - **A timer** has a usual time (`TIM-18`), sped up, slowed or stopped by conditions (wet, heat, smoke, sealed, cold, frozen), and changes pace only when its thing is moved or a condition changes step; a heap shares one timer.
    - **States:** a finished timer gives its thing a state, such as dried, cooked, rotten or tanned, shifting set values for its class and form, so only crushed things leach (`RCK-13`); only five firings make a new item, each written in its input's own entry (`MAT-13`): fired clay, red ochre, charcoal, tar and copper.
    - **Rotting** (flesh, fresh hide, fruit, leaves, dead wood, damp nuts and grain): food falls and poison rises; fresh meat rots in about 3 days in summer, 2 weeks near freezing, never frozen; dried or smoked things far slower, dead wood over a few years; kept dry, nuts and grain last about a year.
    - **Drying:** meat strips in 2–3 days, a stretched hide or shaped pot in about 2, green poles in a season; rain stops it.
    - **Cooking** (at heat 2–3, or in boiling water): about an hour; food rises a step and some poisons fall (`RCK-03`); twice as long, or at heat 4, burns it to food 0.
    - **Smoking** (in thick smoke from green wood at heat 1–2): about 2 days; smoked or dried meat then keeps about 15 game days in summer, as about three months do in life (`TIM-18`), and through the winter in the cold (`RCK-14`).
    - **Soaking:** crushed acorns lose their bitterness in about 2 days in running water, 6 in still (`RCK-13`); a hide with crushed bitter bark becomes leather in about 10 days (`RCK-06`).
    - **Fermenting** (crushed sweet fruit or grain mash with water, closed and warm): about 3 days, giving a mild drink that lifts the mood (`MND-29`, `RCK-07`).
    - **Setting:** tar and resin glue set as they cool, in about an hour.
    - **Firing:** clay at heat 3 for about 4 hours becomes fired clay, a step tougher at 4 and again at 5, where it takes half the time (`RCK-04`); yellow ochre at 2 for an hour turns red (`RCK-15`); flaking stone buried under a fire at 2 for half a day is treated (`RCK-10`); covered from the air, wood at 3 for a day becomes charcoal and birch bark at 2–3 for an hour tar (`RCK-12`); green ore among charcoal at 5 for an hour gives copper, a bead from a lump and bright specks from paint of ground ore (`RCK-08`); wet clay, or stone heated fast, cracks.
    - **Meant or not:** a timer a blueprint starts, such as firing pots, is settled at the maker's level (`MAT-04`); one nobody meant, at level 0 of the blueprint its thing and conditions fit; one no blueprint fits, such as rotting, always runs; so meat fallen in the fire, or copper in a blown kiln, can be noticed (`MND-10`, `MOM-12`).
  - **Done when:** in scenes, each timer runs in its stated time under each condition, and each firing change happens at its heat and never below it.

- `MAT-08` **Traces last** *(Decided)*: Paths, rubbish heaps, old camps and graves last, slowly buried and decaying by their materials, for later people, and you, to find (`PRE-09`).
  - **How it works:**
    - **Paths:** a route walked often becomes a trodden path that people follow, worn on the cells it crosses (`WLD-12`) and growing over after a few unused years.
    - **Rubbish heaps:** food waste, bones, ash and broken things make rich ground, where thrown seeds sprout (`RCK-23`) and wolves scavenge (`MOM-06`).
    - **Old camps:** hearths, hut remains, tools and graves (`CUL-19`) stay where they were left, to be found and copied (`MND-11`); loose leftovers merge into heaps as they are buried.
    - **Burial:** a cave floor rises a few centimetres a century, a river flat with each flood (`WLD-17`); digging turns buried things up (`MOM-09`), and you can see the layers (`PRE-25`).
    - **What survives:** stone, fired clay and copper last; bone lasts in caves and lime-rich ground, but rots within centuries in acid ground; wood, hide and fibre rot within a few years unless waterlogged, frozen or very dry (`WLD-27`).
  - **Done when:** in a whole world after 200 years, an abandoned cave camp's stone and fired clay remain, buried a few centimetres, while its hides and wood are gone.

### 7.4 Blueprints

- `MAT-04` **Blueprints** *(Decided)*: A blueprint says: if someone does this action, on things with these characteristics, in these conditions, with this much experience in this sector, then this happens.
  - **The fields:**
    1. **Result:** a new thing (`MAT-21`), a new state, such as dried (`MAT-19`), or a change to ground or a body, such as a sown plot or a dressed wound; with how much, the main characteristic quality moves (`MAT-20`), which input makes each part (`PRE-42`), and leftovers.
    2. **Action:** exactly one base action (`MAT-06`); longer work is a chain of blueprints, each leaving a named state (`MAT-22`).
    3. **Inputs:** each with its role (worked thing, tool, binding, fuel, container, body or ground), a class (`MAT-01`) and form (`MAT-02`) where needed, ranges of characteristics, size and amount, never a named item, and whether used up or kept, a kept tool taking the wear set here (`MAT-20`).
    4. **Place:** such as by a fire of some heat (`MAT-18`), in water, sheltered or in the growing season.
    5. **Sector and difficulty:** one sector (`MND-06`) and a difficulty from 1 to 10.
    6. **Time:** the work, one activity of repeated tries (`TIM-17`), each a tenth shorter for each step the tool's main characteristic is above its range's lowest, at most a third, then any waiting, a timer on the thing (`MAT-19`), with any tending, such as feeding a kiln; results land as the last part ends.
    7. **Chance:** from the difficulty and the maker's level, and, for a blueprint not yet known, a discovery factor, 1 unless tuned (`MND-11`).
    8. **Failures:** what failed tries give and how often: lost time, spoiled inputs, a poor result (quality 0) or a hurt (`BIO-13`), at a rate set for the tries made in an hour.
    9. **Hinted by:** outcomes of plain uses, blueprints or timers, or its own failure sign, that give a hunch for it (`MND-11`), such as smoke while drilling.
  - **How it works:**
    - **What fits:** an action's things are what the doer holds, works and rests it on (an anvil, a hearth board, the ground), and each input must match one of them by item, material, state and size, wear and quality moving only a made thing's main characteristic (`MAT-20`); a blueprint fits only while its place holds, for accidents and experiments as for makers; people choose blueprints, and experiment, only with things in reach (`MND-09`).
    - **Chance:** a try's level is the average of skill in the blueprint and experience in its sector (`MND-06`), fractions allowed; at a level equal to the difficulty one try in two succeeds, each level above or below adding or taking a tenth, within 5% and 95%; fine inputs (quality 4–5) add a tenth, poor ones (0–1) take a tenth; tiredness (rest below 20), clumsy cold (`BIO-11`), darkness beyond firelight and an arm below half health each take a tenth and slow work by a quarter, and pain and sickness act as `BIO-13` and `BIO-05` say.
    - **Unknown blueprints:** an activity ending on things that fit an unknown blueprint gives it the chance a maker at the doer's level would have, times the factor for how it was tried (`MND-11`), rolled once; a success, if noticed (`MND-10`), teaches it at skill 1, and a learner beside a teacher has the full chance (`MND-13`).
      When one action fits several blueprints, the one being done settles as usual and each other rolls its own chance.
    - **Working together:** a big result, such as a structure, a plot or a kiln firing, needs set person-hours of work, by several people at once or in turns, and is settled at the end at the level of the most skilled worker, who is credited with any discovery (`MAT-21`).
  - **Example:** sharp flake: strike a core (hardness 4–5, flaking 3–5, 8–30 cm, used up) with a kept striker (hardness 3–5, toughness 3–5, 6 cm or more), either moving; stone, difficulty 2, about half a minute a try; the flake, 3–8 cm, keeps its core's values but its main characteristic, edge, equals its flaking, and toughness is 1; failures give crumbs 9 times in 10, shatter the core 1 in 10 (chunks of 8 cm or more stay cores), and 1 in 100 also cut the hand (`BIO-13`).
    Its hint is sharp crumbs; nobody knows it at the start (`BIO-20`), but cracking nuts on a stone anvil with a flint cobble fits it (`MND-11`).
  - **Done when:** in trials (`RES-24`), every launch blueprint succeeds about as often, and takes about as long, as its fields say.

- `MAT-07` **Several routes** *(Decided)*: One named result can come from different materials or from different blueprints, so peoples reach the same things by different paths.
  - **How it works:** routes differ in what they need and how they look (`PRE-42`): an ember comes by drilling or by ploughing a stick along a groove; which a people finds first depends on what lies around it (`WLD-14`) and on chance.
  - **Done when:** across the pace tests (`RES-07`), at least two routes each to fire and to huts appear.

- `MAT-22` **Chains** *(Decided)*: Results feed other blueprints, so most things take a chain of one-action steps, each discovered or learned on its own and made for what the chain's end does for people (`MND-09`).
  A people can stall at any step, or get round it by another route (`MAT-07`).
  - **Example:** a flake (strike) makes a scraper (press); a fresh hide scraped (scrape) and dried on a frame (dry) is bedding; greased (apply) and rubbed soft (press), a wrap; cut (cut), pierced with an awl (drill) and sewn with sinew thread (bind), a cloak, warmth 3, or 5 with the fur on (`RCK-26`).
  - **Done when:** the chain runs end to end in a scene (`RES-23`), and a band that knows every step but the scraper stalls there or finds another route.

- `MAT-21` **Named discoveries** *(Decided)*: Every named result has a name, an icon and its action's sound (`SND-06`); a new thing also has a model (`PRE-46`), and a change shows on what it changes (`PRE-42`).
  A people's first noticed success (`MND-10`) at making one is a named discovery, written in the book of ages.
  - **How it works:** each result has an English name and each people's own word, coined at its first success (`CUL-18`).
    The entry gives who, when, where, by which route (`MND-11`), from what, and the word (`PRE-05`); a timer's unmeant result counts too, for whoever notices it first (`MND-11`, `MOM-12`).
    Steps of the arc (`TIM-19`) and world firsts are major entries and live moments (`PRE-08`), and a people's first of something others make is short.
  - **Done when:** in the sharp-stone test (`RES-02`), every first flake gives an entry with who, when, where, route, inputs and word, and in a scene of two peoples, the second's first flake a short one.

- `MAT-23` **The launch blueprints** *(Decided)*: About 140 blueprints cover the arc from caves to first copper; each thing is one named result, its routes under it (`MAT-07`).
  - **How it works:**
    - **Sectors** (`MND-06`) follow the craft: knapping and grinding stone is stone, fire-making and charcoal fire, hide, sinew and bone for clothing hides, clay and kilns pottery, copper metal; food, plots, animals, healing, art and music go by purpose.
    - **How many:** stone 14, wood 13, fire 10, cooking 14, hunting 10, gathering 7, hides 16, building 12, healing 7, pottery 7, herding 4, farming 6, metal 5, art 8, music 3; the fallback set is `RSK-25`'s.
    - **Known at the start** (`BIO-20`): crack nuts and bones with a stone, butcher a carcass with a broken stone or bare hands (slowly, wasting much), make a bed of grass or leaves, bank a fire, and carry embers.
    - **Results other items name** are all in the launch list, among them milk: pressing a tame nursing female of a kind the catalogue marks as giving milk, such as a goat (herding, difficulty 2), gives milk, food 3 and water 4, souring within a day (`WLD-33`); the blown firing (pottery: a charcoal kiln tended by blowers taking turns at its pipes, hinted by blown embers flaring, `MOM-12`); and a notched tally (art: notches cut in bone or wood, `CUL-03`).
    - **Steps of the arc** (`TIM-19`) count: for flakes, a sharp flake or blade; fire, an ember by drilling or ploughing; clothing, any worn hide or fur; huts, any roofed shelter people build, not a windbreak; pottery, a fired pot; copper, smelted copper, not hammered native copper.
  - **Done when:** the catalogue holds the launch blueprints and every result another item names, and its checks pass (`MAT-17`).

### 7.5 Values and catalogues

- `MAT-05` **Plausible values** *(Decided)*: Every value is a plausible estimate, set by hand and tuned (`PRN-05`).
  - **How it works:** values keep the real order of things, such as flint harder than bone; the reality rules fix the orders that decide what is possible (7.6).
  - **Check:** the catalogue checks hold every value to those orders (`MAT-17`).

- `MAT-13` **The catalogues** *(Decided)*: The game's content is in catalogues written by AI agents and checked by automated tests: items (`MAT-10`), blueprints (`MAT-23`), plants (`WLD-31`), animals (`WLD-32`), illnesses (`BIO-05`), pattern sentences (`PRE-37`), and other sections' prepared lists, such as thoughts (`MND-29`) and the culture lists (`CUL-07`).
  - **How it works:** each entry stands alone, in plain words, with its values and checks (`MAT-17`), naming others only as results, never as inputs (`PRN-07`); every rule in them is physical or biological (`SCP-19`), except in the culture lists.
    Hints too name actions, signs and kinds of thing, never entries.
  - **Check:** no entry names another as an input (`MAT-17`).

- `MAT-14` **Adding without rewriting** *(Decided)*: Adding an item, blueprint, plant, animal or illness never needs the others changed (`PRN-14`): a new item fits every blueprint whose ranges it meets.
  - **Check:** a made-up item added for testing works in every blueprint it fits, with no other entry changed.

- `MAT-16` **The catalogue grows by milestone** *(Decided)*: Each milestone adds only the entries its steps need (`SCP-16`), never rewriting earlier ones, and later layers grow the same way (`VIS-03`).

- `MAT-17` **How the catalogue checks work** *(Decided)*: Automated checks keep the catalogues complete and believable; each new or changed entry comes with its own checks, and no change joins the main version unless all pass (`PRC-10`).
  - **How it works:**
    - **Complete:** every item has all `MAT-10` lists for its kind, every blueprint every field of `MAT-04`, naming no item as an input (`PRN-07`), and every illness entry its start rate, its carriers' share or its chance per raw meal (`BIO-05`).
    - **Reachable:** from what the bands know at the start (`BIO-20`) and what the land can hold (`WLD-10`, `WLD-14`), blueprints and timers make every named result by some route, and no chain needs its own result first.
    - **Discoverable:** each blueprint has a first route the check names, needing only the start knowledge and earlier routes: an accident (a plain use or a knowable blueprint fits it), a hint (an outcome of either, or its own failure sign, gives a hunch for it) or an experiment (any base action on one or two things in reach, `MND-11`); one with three or more inputs needs an accident or a hint, and one with no route fails.
    - **Worth in reach:** every launch result has a direct use as `MND-09` lists them, or is at most three blueprints from one that has; a longer gap fails.
    - **Possible:** each heat needed can be reached with a fuel and setting the catalogue has (`MAT-18`), and every input size exists.
    - **Reality rules:** each rule in 7.6 is checked against every item in every state it can take, new and worn, at quality 0 and 5.
    - **Expected fits:** a list kept with the checks, never read by the game, names the blueprints each item is meant to fit, and each new entry adds its lines; any fit not on it fails until the ranges are narrowed or the fit is added with a reason, so an axe of bark fails (`RSK-06`).
    - **Fallback set:** every check also runs on the fallback set alone (`RSK-25`).
  - **Check:** a test catalogue with one planted fault for each check above, a broken reality rule included, fails every one.

### 7.6 Reality rules

Rules on the catalogues that must always hold, each with its automated check (`MAT-17`).
Checks that need chance use trials of 200 tries a level (`RES-24`), at level 10 where a rule says something happens and at every level where it says it never does, or scenes of 20 runs (`RES-13`).

- `RCK-01` **Flint flakes, granite doesn't** *(Decided)*: Stones that flake, such as flint, chert and obsidian, give sharp flakes; others, such as granite and sandstone, never do.
  - **Check:** only stones on the check's own list have flaking 3 or more in any state, no blueprint gives edge 4 or more from a stone below flaking 3, and without them the sharp-stone test never makes a flake (`RES-03`).

- `RCK-02` **Fire by friction** *(Decided)*: Dry wood, drilled or ploughed, gives an ember; green or wet wood never does, and wet tinder never catches.
  - **Check:** fire-making accepts only wood and tinder with water 0–1; in trials, dry wood gives embers within about 5 minutes a try, green wood none, and tinder with water 3 or more never catches (`MAT-18`).

- `RCK-22` **Air feeds fire** *(Decided)*: Blowing on embers makes them flare, and blowing into an enclosed charcoal fire makes it hotter; smothering puts a fire out.
  - **Check:** in trials, embers in dry tinder make a small fire within a minute in at least 150 of 200 tries when blown and at most 120 when not, a charcoal kiln blown through two pipes reaches 5 and falls to 4 within minutes of stopping, and earth puts a campfire out (`MAT-18`).

- `RCK-08` **Copper needs a furnace** *(Decided)*: Green copper ore gives copper only at heat 5, a blown charcoal fire (`MAT-18`); native copper can be hammered cold.
  - **Check:** every route to copper starts from native copper or from ore at heat 5; in trials, ore never gives copper at heat 4 or below, and does in at least 150 of 200 tries blown for an hour.

- `RCK-03` **Cooking helps** *(Decided)*: Cooked meat, roots and grain nourish more than raw, and cooking lowers some poisons.
  - **Check:** every cooked meat, root and grain has food a step above raw, and no more poison.

- `RCK-14` **Keeping food** *(Decided)*: Dried or smoked meat and fish keep about 15 days in summer and through the winter in the cold, and dry grain and nuts a year in a dry store.
  - **Check:** dried and smoked meat keeps at least 5 times as long as fresh, and stored dry grain and nuts a year (`MAT-19`).

- `RCK-13` **Leaching** *(Decided)*: Soaking crushed acorns in running water draws out their bitterness.
  - **Check:** crushed acorns go from poison 2 to 0 at the times in `MAT-19`, and whole ones hardly change.

- `RCK-07` **Fermenting** *(Decided)*: Crushed sweet fruit or grain mash, kept warm and closed, ferments in a few days; cold slows it, frost stops it, and dry things never ferment.
  - **Check:** only plant things with food 2 or more and water 3 or more ferment, and never frozen (`MAT-19`).

- `RCK-21` **Floating** *(Decided)*: Dry wood floats; stone sinks.
  - **Check:** every dry wood, bark, reed and charcoal item has weight 0–1, and every stone, earth and metal item 3 or more (`MAT-11`).

- `RCK-10` **Heat-treated stone** *(Decided)*: Flaking stone buried under a fire and cooled slowly flakes better; put straight into flames, it cracks.
  - **Check:** every stone of flaking 3 or more has a treated state a step higher, or quality if flaking is 5, and no other stone's treated state reaches 3; in trials, stone put into a campfire cracks in at least 150 of 200 tries.

- `RCK-11` **Cord** *(Decided)*: Fibres twisted into cord are far stronger than loose ones, and only fibrous things make cord.
  - **Check:** only items with fibre 3 or more fit the cord blueprints, and every cord is at least two steps tougher than its fibres.

- `RCK-12` **Glue from bark** *(Decided)*: Birch bark heated without air gives tar, a glue; burning in the open, it leaves a little tar on a smooth stone or bone beside it, and otherwise only burns.
  - **Check:** tar comes only from birch bark covered from the air at heat 2–3, or condensed on a smooth stone or bone beside birch bark burning in the open (`MAT-19`, `MAT-07`); birch bark burnt in the open with nothing beside it leaves only ash.

- `RCK-25` **Hafting** *(Decided)*: A head bound or glued to a shaft gives the head's edge with the shaft's reach.
  - **Check:** every hafted result takes its edge from its head and its toughness from the weakest of head, binding and shaft, with bindings of fibre 3 or more or glues of stickiness 4 or more.

- `RCK-04` **Pottery needs fire** *(Decided)*: Shaped clay dried in the sun softens again in water; only clay fired at heat 3 or more stays hard and holds water.
  - **Check:** unfired clay has waterproof 0 and, soaked, falls apart into wet clay, what it breaks into (`MAT-20`); fired clay has waterproof 3 or more, and no route makes waterproof clay without heat 3 (`MAT-19`).

- `RCK-06` **Leather** *(Decided)*: Hides soaked with crushed bitter, staining bark, such as oak, or worked with an animal's brain, stretched as they dry and then smoked, become leather that stays supple and doesn't rot; in plain water they rot.
  - **Check:** tanning accepts only plant sheets or powders with poison 2–3 and pigment 2 or more, or brain worked into a scraped hide followed by smoking; its leather doesn't rot, and a hide in plain water rots on time (`MAT-19`).

- `RCK-26` **Warmth from the material** *(Decided)*: Clothes, bedding and shelters keep warmth by what they are made of, fur most, and far less when wet.
  - **Check:** fur has warmth 5, hide and leather 3, woven plants 1–2; shelters and beds take their covering's warmth, clothes their outer layer's, halved when wet (water 3 or more, `BIO-11`).

- `RCK-15` **Ochre turns red** *(Decided)*: Yellow ochre heated at 2 or more turns red.
  - **Check:** yellow ochre fires to red ochre at heat 2 or more, and nothing else turns it red (`MAT-19`).

- `RCK-16` **Paint that lasts** *(Decided)*: Charcoal or ochre paint on rock lasts for centuries where sheltered; on open rock, rain wears it away within years.
  - **Check:** a painting in a cave or under an overhang wears at most a step a century, one on open rock at least a step every few years (`MAT-20`).

- `RCK-23` **Seeds grow** *(Decided)*: Seeds in fertile, moist ground in the growing season sprout; planted and tended ones grow better, which is farming.
  - **How it works:** seed thrown on rich, damp ground, such as a rubbish heap, gives a few plants the next season (`MOM-08`); dug in and covered, most sprout; weeded and watered by plain uses (`MAT-06`), a plot yields about twice an untended one (tuned), at the scale of `WLD-30`; roots and cuttings grow the same way.
  - **Check:** every food plant lists its growing season and ground (`WLD-31`); in a scene, seed thrown on a damp heap in season sprouts in at least 15 of 20 runs, and a tended plot yields at least twice an untended one.

- `RCK-24` **Young animals grow tame** *(Decided)*: Young animals raised and fed by people grow tame; adults rarely do.
  - **Check:** in a scene, wolf pups fed daily from their first days reach tameness 5 within a season (`WLD-33`), while grown wolves fed for a season stay at 2 or below in at least 15 of 20 runs.

## 8. People: bodies and lives

Numbers are plausible starting values, tuned in tests (`PRN-05`); every age and time follows `TIM-18`'s rule.
Shares in Done when lines are judged as `RES-13` sets out.

### 8.1 Who they are

- `BIO-01` **Modern humans** *(Decided)*: The people are one human species, with bodies and minds as able as ours (`SCP-05`).
  Only their culture starts almost empty (`SCP-01`).

- `BIO-02` **Starting kit** *(Decided)*: The first people have almost no culture, but they share one language (`CUL-17`) and know a few blueprints and their home range (`BIO-20`).
  - **Fire:** they can feed a fire, bank it under ash and carry its embers, but cannot make one (`MAT-18`).
    Exactly one band starts with a fire taken from lightning; the others must find one (`WLD-28`).
    Food that falls by the fire can cook (`MAT-19`), but nobody cooks on purpose until someone learns how (`MND-11`).
  - **Tools, clothing and shelter:** no shaped tools, only stones for bashing and sticks for digging; no clothing; caves and overhangs, with beds of grass or leaves.
  - **Food:** gathering, scavenging, and small game taken with thrown stones and sticks (`MAT-11`).
  - **Beliefs:** none about spirits or hidden causes.
  - **Done when:** every new world's bands start with exactly this kit, and the sharp-stone test runs from it (`RES-02`).

- `BIO-03` **Starting population** *(Decided)*: 3–4 family bands of 15–30 people, 45–120 in all, each with its own shelter and home range in one start region (`WLD-24`).
  - **How it works:**
    - **Families** are made directly, not played out from earlier generations: each band is 3–6 couples aged about 18–45 with their children, born about 4 years apart, plus a few grandparents over 45, a few orphans and widowed people, and, for about one adult in three, a brother or sister in another band (`CUL-27`, `CUL-30`).
    - **Ages:** about two in five are children under 14, and men and women are about equal in number.
    - **Bodies:** traits and looks come from made-up grandparents by `BIO-06` and `BIO-22`, and about 1 adult in 10 has an old scar, a limp or a crooked arm.
  - **Done when:** every start has 3–4 bands of 15–30 people, every child has a living parent or kin to care for it, and every mother was 18–38 at each child's birth.

- `BIO-20` **Starting knowledge** *(Decided)*: Each adult starts knowing a few blueprints, their home range, which local things are food or poison, and their kin.
  Every starting fact is true, so every mistake comes from play.
  - **How it works:**
    - **Blueprints:** the five every forager knows (`MAT-23`), at skill 3.
    - **Experience:** gathering 3, hunting 2 and fire 1 (`MND-06`), plus 1 in each for every 15 years over 20.
    - **Mental map:** their home range within about 10 km of their shelter, as about 40 places at world-cell scale or larger (valleys, ridges, springs, caves, stone), with water, food by season and dangers read from the cells (`WLD-12`, `MND-28`), and where the other bands camp; no area is made in advance.
    - **Things:** which of the 10 most common plants and 5 most common animals of their range are food, poison or dangerous (`MND-04`).
    - **People:** their band and kin, with a few friendships and grudges (`MND-24`).
    - **Memories:** none from before history begins (`MND-18`).
    - **Children** know a share of this by age, and learn the rest as they grow (`MND-06`).
  - **Done when:** in a new world every adult knows exactly this, and nothing more (`MND-02`).

- `BIO-08` **Everyone is different** *(Decided)*: Each person has their own height, build, strength, stamina, resistance, sight, hearing and learning speed.
  - **How it works:** each is drawn around the human average, two people in three within a fifth of it, from inheritance (`BIO-06`), sex (`BIO-17`), age (`BIO-16`) and chance.
    Long hunger in childhood (condition under 30 for a year or more in all before 14, `BIO-09`) takes about a tenth off height and strength for life.
  - **Done when:** each number's spread in a starting population matches the stated range, and children hungry for a year or more grow up about a tenth shorter and weaker.

### 8.2 Staying alive

- `BIO-09` **Needs of the body** *(Decided)*: Hunger, thirst, warmth (`BIO-11`) and rest, each felt by the mind as a need (`MND-07`), with real effects on the body when unmet.
  - **How it works:**
    - **Hunger:** the body uses about a day's food a day (`BIO-10`), more for the big (`BIO-08`), up to half as much again in hard work, a quarter more in cold and a fifth more in growing children, plus what pregnancy and nursing need (`BIO-15`).
    - **Condition:** the body's reserve, from 0 (starved) to 100 (well padded).
      A day without food costs about 5, and each day's food eaten beyond need adds about 5, with at most half a day's food extra a day, so a well-fed adult starves in about 20 days.
      Below about 30, strength and stamina drop by about a fifth, healing and fighting illness are worse, and women stop conceiving (`BIO-05`, `BIO-13`, `BIO-15`); at 0 they die (`BIO-14`).
    - **Thirst:** an adult needs about 3 litres of water a day, up to twice that in heat or hard work, from drink and food (`BIO-10`); without it, people weaken within a day and die in about three, and salt water makes it worse (`WLD-26`).
    - **Rest:** adults sleep about 8 hours a day, children about 10 and babies most of the day; cold, hunger and pain spoil sleep.
      After about a day and a half awake, people fall asleep where they are.
  - **Done when:** in scenes, a well-fed adult without food dies in about 20 days and one without water in about 3, and a band with enough food, water and shelter keeps its condition through a mild year.

- `BIO-10` **Food** *(Decided)*: A thing feeds by its food value; varied food keeps people strong, and one kind of food for long makes them weak.
  - **How it works:**
    - **Food value:** each step up the food characteristic (`MAT-03`) about doubles what a kilogram gives, so a day's food for an adult is about 4 kg at food 2 (berries, raw roots), 2 kg at 3 (raw meat), 1 kg at 4 (nuts, cooked meat) or half a kilogram at 5 (fat).
      Each kilogram also gives about a fifth of a litre of water per point of its water characteristic (berries, about 0.8 litre).
    - **Four food groups,** read from where a food comes from: meat (flesh, fat, fish, eggs and milk), fruit and greens (a plant's fruit, leaves and shoots, and mushrooms), nuts and seeds (with grain), and roots (`WLD-31`, `WLD-32`); a made food takes the group of its main input.
    - **Variety:** someone who has eaten from only one group over the last 10 days grows weak: strength, stamina and resistance drop by about a fifth until they eat more widely.
    - **Scurvy:** after about 15 days without fresh fruit and greens (not dried, smoked or cooked), gums bleed and wounds stop healing, until a few days of them cure it, a true cause people can learn (`MND-05`).
  - **Done when:** in scenes, people on one food group for 10 days grow weak, and people on dried food for 15 days get scurvy and recover on fresh greens.

- `BIO-11` **Heat and cold** *(Decided)*: Bodies keep warm by work, clothing, shelter, fire and each other; cold can kill, and heat exhausts.
  - **How it works:**
    - **Felt temperature** is the air's (`WLD-16`), up to about 10 °C lower in wind and 5 °C lower on wet skin, and up to 5 °C higher in sun.
      Naked and at rest, people are comfortable down to about 24 °C of it, and work lowers that limit by about 10 °C.
    - **Clothing:** each point of warmth in what they wear (`MAT-03`) lowers the limit by about 6 °C, counted by the share of the body it covers, as its item says: a cloak or tunic about half, leggings a quarter, shoes and a hood an eighth each.
    - **Shelter, bedding and fire:** a shelter cuts wind and rain and adds the warmth of its walls, a cave a few degrees; bedding counts like clothing for sleepers, huddling as one point of warmth all over, and a campfire adds about 15 °C within about 2 m (`MAT-18`).
    - **Cold:** below the limit people shiver; about 10 °C below, they grow clumsy and slow; about 20 °C below, they freeze, grow confused and fall asleep, and die within a few hours.
      A bare part below about −5 °C felt for about 2 hours gets frostbite, a burn from cold (`BIO-13`), and children and the old chill faster.
    - **Heat:** above about 32 °C felt, people need more water (`BIO-09`), and hard work above about 40 °C brings fainting and heatstroke, which can kill.
  - **Done when:** in winter scenes, a naked band huddled in a cave survives a 2 °C night, a lone person out at −5 °C without fire or cover gets frostbite or dies, and people in warm clothes work outside all day.

- `BIO-12` **Poison and medicine** *(Decided)*: Poison and medicine come from the characteristics of what is eaten or put on the body (`MAT-03`), and some things do both, in amounts that matter.
  - **How it works:**
    - **Poison** acts within hours, by its value and the amount eaten against the eater's size, like a short illness (`BIO-05`).
      A meal at poison 1–2 brings cramps and vomiting for a day; at 3 it makes people ill for days and kills about 1 child in 10; at 4 it kills about 1 adult in 3; and at 5 a mouthful kills most people.
    - **Medicine,** eaten or put on a wound, lowers pain by about 10 for each point for about 6 hours and helps the body against illness (`BIO-05`); on a wound it also lowers the chance of infection by a tenth for each point (`BIO-13`).
    - **Venom:** a snake's bite poisons by its kind (`WLD-32`).
  - **Done when:** in scenes, a mouthful of poison 5 kills an adult in most runs, poison-2 berries make the eater ill for a day, and medicine on wounds lowers their pain and infections.

### 8.3 Harm and healing

- `BIO-13` **Body parts and wounds** *(Decided)*: Every body has six parts, head, torso, two arms and two legs (hands and feet count with their limbs), each with its own health from 100 (sound) to 0; wounds bleed, can get infected, heal, cripple or kill.
  - **How it works:**
    - **Wounds** take their size from their part's health until healed, and are of five kinds: cuts (edges and points), bruises (blows and falls), bites (teeth, horns and claws), burns (fire, hot things, lightning and freezing cold) and broken bones (a blow or fall of more than about 25 on an arm or leg).
    - **Size** comes from the cause: the striker's strength and the weapon's weight, hardness or edge (`MAT-03`), the animal's kind (`WLD-32`) or the heat (`MAT-18`); a knapping slip gives about 5, a club or a dog bite about 20, and a spear thrust or a bear's swipe 40 or more.
      A lightning strike burns its victim, sized so that about 1 person in 10 struck dies.
    - **Where blows land:** torso 40%, each arm 15%, each leg 12% and head 6%, unless the event decides.
      A fall does about 10 for each metre beyond the first to a leg, and from about 5 m as much again to the torso and the head.
    - **Bleeding:** a cut or bite bleeds about a tenth of its size a minute, halving every 10 minutes.
      Blood counts from 100: losing about 25 halves strength and stamina, 35 makes people collapse and 50 kills, and it comes back at about 10 a day.
      Pressing cuts the flow to a quarter and stops a wound under 30 within about 10 minutes, and a dressing stops any wound (`BIO-23`).
    - **Pain** is the sum of a person's wound sizes, up to 100, and lowers mood (`MND-29`).
      Work and walking slow by half the pain in percent, and each 20 of pain makes a try a tenth less likely to succeed (`MAT-04`).
    - **Harm by part:** a broken leg allows only a hobble at a quarter speed, a broken arm stops two-handed work (`MAT-12`), and an arm or leg at 0 is useless for life.
      Below half health a head dazes its owner (work at half speed, no blueprints) and a torso halves strength and stamina; below a quarter a head knocks its owner out for about an hour; either at 0 kills (`BIO-14`).
    - **Infection:** in its first two days a cut of 10 or more gets infected with a chance of about half its size in percent, and a bite or burn of 10 or more its full size; smaller wounds heal clean; washing halves this, and a dressing halves it again (`BIO-23`).
      An infected wound stops healing for about 5 days while the body clears it, or turns to wound fever (`BIO-05`).
    - **Healing:** a wound shrinks by about 7 a day (bruises 15), and a broken bone takes about 10 days; faster with rest, food, warmth and resistance (`BIO-08`), slower with hunger and cold.
    - **Lasting harm:** wounds of 30 or more scar, and a broken bone heals crooked about 1 time in 2, or 1 in 10 if splinted, leaving a limp or a weak arm for life.
  - **Done when:** in scenes, a cut of 40 left alone kills while a pressed one doesn't, a broken leg hobbles its owner for about 10 days, and washed wounds get infected about half as often.

- `BIO-05` **Illness** *(Decided)*: About 15 illnesses, each with its routes, a time before it shows, a course, a danger and, for some, immunity; some need crowds, as in real history.
  - **How it works:**
    - **Catching:** a breath contact is a night under the same roof or by the same hearth as the sick, or an hour within about 3 m of them; a touch contact is tending them, sharing their bed or eating food they handled.
      Each contact passes it by the illness's own chance, from a day before the signs show until recovery; the immune don't catch it.
    - **Starting:** everyday illnesses arise in any group about as often per year as in a real year (`TIM-18`), breath ones most in winter, and animal ones from sick animals (`BIO-19`).
      Each illness's entry gives its start rate per year in a band of about 30, the share of its carriers that are sick, or its chance per raw meal (worms: about 1 in 100), as `MAT-17` checks.
    - **Crowd illnesses** start only in villages of at least about 200 people that keep herds (`CUL-28`).
      They burn out once most people have had them, and return to a village every 10–20 years, once enough children born since lack protection.
    - **While ill,** people work and walk at half speed with a fever and three quarters otherwise, and look ill.
    - **Who dies:** a deadly illness decides once, on its worst day, about a third of the way through its course, whether it kills, by its chance in the list.
      That chance is about 5 times higher for babies under 1 and people over 60, twice as high for each of hunger (condition under 30, `BIO-09`), real cold (10 °C or more below their limit, `BIO-11`) and wounds other than the one the illness came from (pain 20 or more, `BIO-13`), and a fifth higher or lower for each fifth of resistance below or above average (`BIO-08`); these multiply, to at most 9 in 10.
      Then daily care halves it, down to a quarter as the carer's healing experience nears 10 (`BIO-23`, `MND-06`), and each point of medicine taken that day takes off a tenth (`BIO-12`); whoever survives recovers at the end of the course.
  - **The illnesses** (routes; days before the signs show), each a catalogue entry with its signs (`MAT-13`), then how long it lasts and deaths among untreated healthy adults:
    - **Cold** (breath, touch; 1–2 days): about 5 days; harmless; immunity for a year.
    - **Coughing fever** (breath, touch; 1–3 days): about a week; kills 1 in 50; immunity for a few years.
    - **Chest fever** (not catching; after a cold or coughing fever in about 1 in 10 babies, old or hungry people and 1 in 100 others, and after freezing or near-drowning in 1 in 5): 7–10 days; kills 1 in 4.
    - **Gut sickness** (fouled water or food; within a day): 2–5 days; kills 1 in 100; fouled water (`WLD-17`) gives it at about 1 chance in 20 each day someone drinks it.
    - **Worms** (raw or undercooked meat and fish): condition falls by about 1 a day until medicine of 3 or more, taken for a few days, clears them (`BIO-12`); cooking prevents them.
    - **Wound fever** (about 1 infected wound in 3, or a birth; 1–3 days): 5–10 days; kills 1 in 3.
    - **Lockjaw** (about 1 in 20 cuts or bites of 20 or more from teeth, horn or a wooden point, or dirtied with earth or dung; 3–15 days): 10–20 days; kills 1 in 2.
    - **Foaming madness** (the bite of a mad wolf, dog or other meat-eater; 5–20 days): always kills, within days, whatever is done; mad animals lose their fear of people.
    - **Hunter's fever** (skinning sick small game or eating it raw; 2–5 days): about 2 weeks; kills 1 in 20; immunity for life.
    - **Herder's fever** (kept goats, sheep and cattle, through raw milk and helping births; 5–15 days): 15–30 days; kills 1 in 50; immunity for life.
    - **Sore eyes** (touch; worst in crowded, smoky camps; 2–5 days): about 10 days; from the third bout, each has about 1 chance in 5 to scar an eye, which takes a quarter off sight (`BIO-18`).
    - **Spotted fever** (crowd; breath; about 10 days): about 10 days; kills 1 in 10; immunity for life.
    - **Pox** (crowd; breath, touch; about 12 days): about 15 days, leaving scars; kills 3 in 10; immunity for life.
    - **Bloody flux** (crowd; water fouled by a village's waste; 1–3 days): about a week; kills 1 in 10.
    - **Wasting cough** (crowd; long close living, and sick cattle's milk; a season or more): one to three years; kills about half.
  - **Done when:** in scenes, each illness spreads only by its routes and kills about its stated share, and a crowd illness dies out in a band but returns to a village of 200 with herds every 10–20 years.

- `BIO-23` **Care and healing** *(Decided)*: The hurt and the sick do better with care, though only the body heals; some treatments are blueprints to discover.
  - **How it works:**
    - **Plain care,** which anyone can do, drawn by the caring leaning (`MND-26`): pressing a bleeding wound (`BIO-13`), and giving food, water, warmth, carrying and company, which lowers deaths from illness (`BIO-05`).
    - **Healing blueprints** (`MAT-23`, `MND-11`): washing, dressings (on a burn, a fat salve) and splints, each as `BIO-13` says, and herbal medicines (`BIO-12`).
    - **Rites and comfort** lift mood (`MND-29`) but cure nothing, so belief in healing rites rests on recoveries that would have come anyway (`MND-05`, `CUL-34`).
  - **Done when:** in scenes, the sick cared for daily die half as often as those left alone, and splinted breaks heal crooked a fifth as often.

- `BIO-14` **Every death has a cause** *(Decided)*: Every death comes through the body.
  - **How it works:**
    - **Limits:** a person dies only when the body passes one: condition, water or blood running out (`BIO-09`, `BIO-13`), freezing or heatstroke (`BIO-11`), a head or torso at 0 (`BIO-13`), a deadly illness or poison (`BIO-05`, `BIO-12`), too long under water (`BIO-21`), or an old body giving out (`BIO-16`).
    - **The record:** each death keeps its cause and how it came about, such as "bleeding, from a boar's tusk, while hunting" (`PRE-05`).
  - **Check:** every death in test runs and whole-world runs names its cause and how it came about.

### 8.4 A life

- `BIO-04` **Life cycle** *(Decided)*: Childhood to about 14, adulthood, old age from about 45 (`BIO-16`), and death.
  - **How it works:**
    - **Stages:** babies are carried and nursed (`BIO-15`); a child walks at about 1 and talks at about 2; from about 5, children help with gathering and carrying, play a lot, and learn by watching and being taught (`MND-13`).
    - **Growth:** children grow toward their inherited height and build (`BIO-06`), fully grown by about 16.
  - **Target ranges** on whole worlds (`RES-14`), coming out of the rules (`TIM-18`), never set:
    - about 1 baby in 5 dies in its first year, and about 2 children in 5 before 14;
    - those who reach 14 live on average to about 50–60, most die before 70, and few pass 80;
    - a woman who lives through her childbearing years has about 6 children (tuned to the next line);
    - numbers grow by about 1.6% a year on average, so a world holds about 1,000–3,000 people at Year 200 and about 2,500–7,000 at Year 250; growth wins when the others pull against it, and nothing caps births (`MND-15`).
  - **Done when:** the pace tests' 20 worlds to Year 75 meet the death, life-span and birth ranges in at least 16, and at least 8 of the full test's 10 hold 1,000–3,000 people at Year 200 (`RES-07`).

- `BIO-15` **Pregnancy and birth** *(Decided)*: Children come from couples, through a pregnancy of about 45 days, a birth with real risks, and years of nursing.
  Who may pair with whom is cultural (`CUL-27`), and pairing and conception are never shown, so sexual violence is not part of the game.
  - **How it works:**
    - **Conception:** a woman of about 18 to 38 with a male partner (`BIO-16`), in condition 30 or more (`BIO-09`) and not nursing, conceives within about 30 days on average, half as fast from about 33.
    - **Pregnancy** lasts about 45 days; the mother needs a fifth more food and in the last 15 days moves slower; about 1 in 6 ends early, more in hungry, ill or older mothers.
    - **Birth** takes hours; about 1 in 10 is hard, more for a first child or a young, old, small or hungry mother, and about 1 hard birth in 5 kills the baby.
      In all, about 1 birth in 100 kills the mother, half through bleeding in a hard birth (`BIO-13`) and half through wound fever after it (`BIO-05`).
      These risks are for a birth with a helper; alone they double, and a helper's healing experience halves them as it nears 10 (`BIO-23`).
      Twins come about once in 80.
    - **Nursing:** milk alone feeds a baby for about half a year, then with soft food until weaning at about 3; the mother needs about a quarter more food, and a hungry one makes less milk.
      Births come about every 4 years in bands, sooner where porridge or animal milk lets babies wean early (`CUL-28`, `WLD-33`); a baby whose mother dies lives only if another nursing woman feeds it, or, once older, on soft food.
    - **The record:** mother, father, date and place of each birth (`PRE-10`).
  - **Done when:** in whole worlds, births in bands come about every 4 years and 1 in 100 kills the mother, and a young baby without milk dies unless another mother feeds it.

- `BIO-16` **Ageing** *(Decided)*: From about 45, bodies slowly weaken, heal more slowly and fight illness worse.
  Age itself takes no knowledge or skill (only disuse fades them, `MND-06`), though the old learn more slowly.
  - **How it works:**
    - **Decline:** strength, stamina, resistance, sight and hearing fall by about 2% a year (`BIO-08`).
    - **Fertility:** a woman's falls from about 33 and ends at about 38 (`BIO-15`).
    - **Giving out:** from about 55 the body itself can give out, at a chance of about 1 in 50 a year that doubles every 7 years or so.
  - **Done when:** in scenes, the old heal and recover from illness more slowly than the young.

### 8.5 The sexes

- `BIO-17` **Real biology, culture decides** *(Decided)*: Men and women differ only in real body ways: pregnancy and nursing, and on average size, strength and body fat, with wide overlap.
  Who hunts, gathers, leads or makes things is up to each culture (`CUL-06`), and minds don't differ by sex (`MND-20`).
  - **How it works:**
    - **Bodies:** on average men are about 7% taller and a third stronger, most in the arms, and women carry more body fat.
    - **Attraction:** inborn: most are drawn to partners of the other sex, about 1 in 20 to their own or to either (`MND-33`).
  - **Check:** a review of the rules finds a person's sex used only for pregnancy, nursing, size, strength, body fat, looks, voice and whom they are drawn to, and scenes show no task or role given by sex.

### 8.6 Senses and activities

- `BIO-18` **Senses** *(Decided)*: Sight, hearing, smell and taste (`MND-21`), with ranges that change with light, weather, the person and age.
  They decide what reaches each mind (`MND-03`).
  - **How it works:**
    - **Sight:** within about 50 m (a camp), people see everything not behind rock or walls.
      Beyond, they see only people, big animals, fire and smoke, out to about 1 km in the open, 200 m in open woods and 50 m in thick forest or scrub, by the plant cover (`WLD-31`).
      Night cuts these ranges to about a tenth outside firelight, and fog, rain, snow and smoke to a third or less (`WLD-16`).
      Small things, such as a flint nodule, are seen within about 30 m only while searching or gathering, or when they are new.
    - **Hearing:** a shout carries about 1 km in still air, talk about 50 m and footsteps about 20 m; wind, rain and rushing water drown sounds.
    - **Smell:** smoke and rot carry a few hundred metres downwind.
  - **Done when:** in scenes, people spot a deer in the open at about 1 km by day and far nearer at night or in fog, and wolves find people from farther downwind than upwind.

- `BIO-21` **Everyday activities** *(Decided)*: The body's side of the everyday activities: walk, carry, eat, drink, sleep, talk, play, fight, flee, care for someone, teach, watch, sing and dance.
  Everyone can do them from the start, and none is a blueprint (`MAT-06`).
  - **How it works:**
    - **Walking:** about 4–5 km an hour on open, flat ground, slower uphill, in forest, marsh or snow, or loaded, so a day's walk covers 20–30 km; the young and the old go slower.
      Walking includes wading, swimming and climbing (`PRE-44`).
    - **Water and heights:** people wade to waist depth; deeper water must be swum, which tires and can drown the weak in cold or fast water, and about 3 minutes under water drowns anyone (`BIO-14`); a slip while climbing is a fall (`BIO-13`).
    - **Carrying:** an adult carries about a quarter of their own weight all day, more for a short way, by their strength (`BIO-08`).
    - **Running and fleeing:** about three times walking speed; running, fighting and heavy work wind people within minutes, and a few minutes' rest restores them (`BIO-08`).
    - **Fighting,** including a spear thrust at an animal: blows land by chance, by strength and hunting experience (`MND-06`), and wound a part (`BIO-13`); most fights end when one is hurt, flees or gives up (`MND-33`).
    - **Play and dance** tire like work.
  - **Done when:** in scenes, people walk 20–30 km a day on open ground and less loaded or uphill, and someone fleeing a bear is winded within minutes.

### 8.7 Inheritance

- `BIO-06` **Inherited traits** *(Decided)*: Looks, build and personality leanings come from both parents, with variation; families resemble each other, but nothing evolves.
  - **How it works:**
    - **What passes on:** the body numbers of `BIO-08` and the twelve personality traits (`MND-20`); looks pass on by their own rule (`BIO-22`).
    - **How:** a child's value is its parents' average, pulled about halfway back toward the human average, plus chance.
      Children inherit their parents' inborn values, never what hunger or illness made of them.
  - **Done when:** in whole worlds run to Year 250, the average and spread of every inherited body number and personality trait stay within about 5% of the start, and children resemble their parents.

- `BIO-22` **Looks** *(Decided)*: Skin, hair, eyes and faces are inherited, so children look like a mix of their parents, and no people looks like a copy of a real one (`SCP-20`).
  - **How it works:**
    - **Two copies:** each person carries two values for each feature (skin tone, hair colour, hair form, eye colour and face shape), one from each parent, shows a blend of them, and passes one, by chance, to each child.
    - **Founders:** a people grown from a few families keeps their looks, which never help anyone survive.
    - **Mixed at the start:** the first people's looks are drawn from wide ranges and mixed, so that no group matches any real people's typical look.
  - **Done when:** children visibly mix their parents' looks.

### 8.8 Animals

- `BIO-19` **Animal bodies** *(Decided)*: Animals near people have bodies on the same pattern: a head, a body and legs, or none (snakes), with wings or fins where they have them.
  - **How it works:**
    - **Wounds** work as for people (`BIO-13`): a speared animal bleeds and slows, or runs off to die later, leaving a blood trail hunters can follow.
    - **Needs and life** follow the same rules with their kind's numbers and life spans (`WLD-32`); they die of the same causes as people (`BIO-14`), and their condition (`BIO-09`) shows in the fat they yield (`WLD-18`).
    - **Illness:** a few animals of the kinds that carry an illness are sick and pass it on (`BIO-05`).
    - **Senses** have their kind's ranges: wolves and dogs smell people about 1 km downwind, and birds of prey see farther than people.
  - **Done when:** in scenes, a speared deer bleeds, slows and can be tracked, and a mad wolf can pass foaming madness by a bite.

## 9. Minds

How people think, and more simply how animals think: the heart of the game.
Numbers here are starting values, tuned in tests (`PRN-17`).

### 9.1 Ground rules

- `MND-01` **No AI language model thinks for them** *(Decided)*: Every choice, belief and discovery comes from this section's rules (`PRN-06`); minds hold records with numbers, never sentences, passed in talk as topics (`CUL-24`); the writer AI only rewords pattern sentences built from finished records, and nothing it writes comes back (`PRE-17`).
  - **Check:** an automated check finds no call from the simulation to a language model and no path from the writer's text back into it, and no record in any mind holds a sentence.

- `MND-02` **Knowledge only from inside the world** *(Decided)*: A mind knows only its starting knowledge (`BIO-20`) and what it has since seen, done, been told or dreamt (`PRN-01`), and its choices use only that, never the catalogues.
  - **Check:** every scene's check of `RES-21` passes, and kept reasons name only what the person knows (`MND-09`).

### 9.2 Needs and personality

- `MND-07` **Needs** *(Decided)*: Nine needs, each from 0 (desperate) to 100 (met): hunger, thirst, warmth and rest follow the body (`BIO-09`, `BIO-11`); safety, belonging, status, curiosity and love follow life:
  - **safety** falls with danger seen or believed near, and rises in shelter, by a fire and among many;
  - **belonging** falls alone or shunned, and rises with company, talk, shared work and rites;
  - **status** follows the respect shown (`MND-24`), and **curiosity** falls with sameness and rises with anything new;
  - **love** falls apart from partner, children and close friends; adults want a partner, and children, the playful and the kind can love a young animal they feed (`MND-16`).
  - **Rates:** belonging, curiosity and love fall about 10 a day unmet and rise about 10 with a shared task, something new or a day with those loved; safety drops at once with danger and recovers within hours.
  - **Urgent:** below 20, a need interrupts what they do (`TIM-17`) and brings a strong bad thought.
  - **Weights:** bravery and caution set safety's pull, sociability belonging's, pride status's, the curious trait curiosity's; love's is the same for all (`MND-20`).
  - **Children:** babies feel only the body's needs, safety and love; the rest grow in through childhood.
  - **Done when:** someone alone for days seeks company, and a need falling below 20 stops their activity at once.

- `MND-20` **Personality** *(Decided)*: Twelve traits, each from −3 to +3, most people near the middle, the low end being the opposite (timid, generous, calm).
  - **Inherited** with variation (`BIO-06`); a big event moves a trait one step, at most about three times in a life.
  - **In choosing** (`MND-09`), besides the needs they weigh (`MND-07`):
    - **curious:** new things pull harder;
    - **brave:** danger and pain count less;
    - **cautious:** the new seems riskier, and custom and taboos weigh more;
    - **patient:** the future counts more (`MND-22`);
    - **hard-working:** they practise more;
    - **playful:** play, music and dance pull harder, and play turns into experiments;
    - **sociable:** they talk and visit more;
    - **kind:** they share, help, comfort and teach more;
    - **greedy:** they keep more, bargain hard and may steal;
    - **hot-tempered:** anger comes fast and lasts, so they quarrel and fight;
    - **proud:** insults hurt more, and they want to lead;
    - **spiritual:** unseen beings seem likelier (`MND-31`), and rites pull harder.
  - **Done when:** in a year, the most curious experiment at least five times as often as the least, and the hot-tempered quarrel twice as often as the calm.

- `MND-21` **Inborn leanings** *(Decided)*: Fixed leanings everyone is born with, never beliefs, which ease some lessons but teach nothing alone (`PRN-01`):
  - **taste:** food tastes good by its food value, fat most; poison or medicine of 2 or more tastes bitter (`MAT-03`), except in fruit and the few the catalogue marks mild-tasting; bitter things are spat out unless hunger is high or a belief says they help, so most bitter poisons are avoided, but not poisonous berries;
  - **ready fears:** pain's cause, snakes, heights, the dark and big predators are feared after one fright, other things after several (`MND-08`);
  - **parents and babies** love each other from birth;
  - **copying:** seeing others do something makes doing it likelier (`CUL-01`).
  - **Done when:** the fed spit out a bitter root the starving eat, and one snake fright, unlike one deer fright, keeps a person away.

- `MND-26` **Social leanings** *(Decided)*: The only social instincts built into the rules (`CUL-07`):
  - **kin:** kin's good counts, more for closer kin, as kinship is believed (`MND-24`);
  - **caring:** seeing someone hurt, sick or hungry pulls toward helping, more for kin and friends;
  - **favours:** help leaves a debt that pulls toward repaying, and taking without returning angers;
  - **ownership:** what one made, found or was given is one's own, and taking it angers;
  - **own group:** one's own band and people are trusted over strangers;
  - **raised together:** those who lived closely while one of them was a small child, brothers and sisters or parent and child, never want each other as partners;
  - **shared attention:** people follow another's gaze and pointing, which makes teaching work (`MND-13`);
  - **a beat:** a shared beat draws people to move together, warming them to each other (`CUL-10`).
  - **Done when:** in 20 band-scene runs, no two raised together pair, and kin help the hurt before strangers do.

### 9.3 Mood and feelings

- `MND-29` **Mood and thoughts** *(Decided)*: Mood, from 0 to 100, moves over a few hours toward 50 plus all live thoughts, as in RimWorld.
  - **Thoughts** have a size and a length, such as "ate cooked meat" (+5, a day), "insulted by Tamo" (−5, three days) or "my child died" (−20, 30 days, fading); about 100 kinds, a catalogue checked by tests (`MAT-13`).
  - **Sources:** unmet needs, events, feelings (`MND-19`) and memories recalled (`MND-18`); the kind feel others' losses more, the proud insults twice over, and a repeated thought adds less each time.
  - **Effects:** low mood slows work, sours talk and risks a breakdown (`MND-30`); high mood speeds work, and above about 85 can bring a few days of inspiration, with more experiments and better-made things (`MAT-20`).
  - **Done when:** a feast lifts a band's mood and a death lowers it for about the stated times, and a band starving through winter averages below 30.

- `MND-19` **Feelings** *(Decided)*: Seven strong feelings: fear, anger, grief, joy, love, shame and awe.
  - **Set off by:** danger (fear); harm, insult or a blocked goal (anger); losing someone loved (grief); success, a birth or a feast (joy); time together and kindness (love); breaking a rule one holds (`CUL-20`) or failing before others (shame); something vast or unexplained, such as a great storm (awe).
  - **Toward people:** love is an opinion above about +60 that has lasted (`MND-24`), and grief is for each one lost, as deep as that love.
  - **Lengths** follow `TIM-18`'s rule: fear fades within hours, anger within days, joy in a day or two, shame and awe in days; grief, and love fading apart, take seasons.
  - **Effects:** they push choices (fear to flee, anger to fight, grief to stillness, joy to company, love to stay close, shame to hide, awe to rites and art), deepen memories (`MND-08`) and show in faces and poses (`PRE-27`).
  - **Done when:** fear of a wolf seen at dusk is gone by morning, and a mother's grief for her child lasts at least a season.

- `MND-30` **Breakdowns** *(Decided)*: Below mood 20 a person may break, about one chance in ten a day, one in three below 10, at most once a season.
  - **How it works:** the hot-tempered rage (shouting, smashing, attacking whoever angered them, `MND-33`); the proud and the brave run off alone for a day or more, to return, join another band or die in the cold; the rest despair, lying still and refusing work and food for a day or more.
  - **Done when:** in a starving-winter scene of 30, at least 2 break down in 16 of 20 runs; in a good season, at most 1 in 50 does.

### 9.4 Memory and knowledge

- `MND-03` **Noticing** *(Decided)*: People take in only some of what their senses reach (`BIO-18`), and only that can become a thought, memory, fact or surprise.
  - **Always:** danger, the moment it comes within sight, hearing or smell, worked out from both their ways, the same at every speed (`TIM-17`); their own task; talk or a call to them.
  - **The rest:** each hour they look over only the nearest few dozen people, animals and things in sight, and take in at most about four new ones, the newest and most surprising first; fewer when busy, tired or frightened, and asleep only what wakes them.
  - **Done when:** a bear walking into camp is seen at the same game second at real and top speed, and nobody in a camp of 30 notices over four new things an hour.

- `MND-18` **Memories** *(Decided)*: People remember events that touched them (a birth, a death, a hunt, a fight, a gift, a first, a strong dream, a story heard): what happened, where, when, how they felt, and who was there, up to about five who mattered most and how many others.
  - **Fading:** slower the more important (`MND-08`), so a child's death lasts a lifetime and a good meal a few days; recalling, telling or dreaming renews (`MND-12`); when full (`MND-14`), the weakest go.
  - **Recall:** places, people and things bring back memories and their thoughts.
  - **Retold** memories pass and change as `CUL-24` and `CUL-11` set out.
  - **Done when:** someone who saw a child drown recalls it passing the river 20 years later, and an ordinary meal is forgotten within a season.

- `MND-08` **Feelings shape memory** *(Decided)*: The stronger the feeling, the longer the memory lasts; fear also ties itself to its cause, a place, animal or person, so meeting it again brings the fear back, and people may avoid it for years.
  - **Done when:** a hunter mauled at a ford avoids it for at least a year, while a quiet day there is forgotten within a season.

- `MND-04` **Knowing things** *(Decided)*: For each kind of item met (`MAT-10`), people know its visible characteristics, such as size, colour and edge, at a glance, and hidden ones, such as poison, fuel and flaking, only by use or by being told (`MND-27`), each with how sure they are (`MAT-03`).
  - **By use:** eating shows at once how filling a food is, but poison and medicine only by linking what follows (`MND-05`), so people can be wrong; burning shows fuel, striking flaking, wearing warmth, holding water waterproofing.
  - **Look-alikes:** an untried kind is taken for the known kind it looks most like, so a poisonous berry like a safe one is eaten until sickness teaches otherwise.
  - **Done when:** people who burn a new wood learn its fuel value, and a band that knows flint uses chert untaught.

- `MND-28` **Mental map** *(Decided)*: What each person knows of places, by season, from going there and from talk (`CUL-24`).
  - **A place** is a spot worth knowing (a spring, a flint outcrop, a grove, a cave, a camp), at most one to an area (`WLD-12`), or a stretch known roughly (a valley, a ridge), one or more world cells; when full (`MND-14`), the least used fade.
    Each holds up to about eight facts ("hazelnuts here in autumn"), with when last seen and how sure, so old facts can be wrong.
  - **The way:** a direction and roughly how long it takes; the actual way, often a trodden path (`MAT-08`), is found only by walking.
  - **The year remembered:** how each season of the last two years went for them (hunger, cold, danger, what was plentiful where), expected to come again; elders' stories count too.
  - **Done when:** someone told of a spring goes there when thirsty and stops going once it is dry; berries eaten out are not sought until next season.

### 9.5 Beliefs

- `MND-27` **Beliefs** *(Decided)*: Something held true, with a strength from 0 (none) to 100 (certain) and a source: own experience, or a belief heard, which starts at or rises to the teller's strength times the hearer's trust in them out of 100 (`MND-24`), so trust 50 halves it.
  - **How it works:** of two that clash, the stronger guides choices until evidence settles it.
  - **Done when:** a warning about a cave keeps people away more often from a trusted elder than from a stranger.

- `MND-05` **Beliefs about causes** *(Decided)*: After a strong outcome, people link it to something unusual that came before, and later outcomes strengthen or weaken the link: the one rule behind real knowledge, wrong beliefs, taboos and rites.
  - **Outcomes:** what befalls them, or kin and friends they see: a hunt or a find, a hurt, an illness, a birth, a death, a storm, flood or fire, or the end of days of hunger, cold, pain, illness or storms, as big as its worst thought.
    It is strong at a thought of 8 or more either way (`MND-29`); a need falling or their own act is not an outcome, and a surprise links only at its place (below).
  - **Causes** are kinds done or met: an activity with the kind of thing used (ate hazelnuts, sang), a place, a person, someone sick, or a kind of animal or weather, each counted by the days of the past year it came up on.
  - **Explained or not:** if a link of theirs whose cause was present explains it, that link is tested and nothing new forms.
    Otherwise they link it to the most unusual thing of the day or two before, if one came up on fewer than about one day in ten over the past year, or not in the ten days before, a first time most.
  - **At a place:** a surprise at a place also links to what they usually do or leave there, or once did there, so plants on the heap are linked to the seeds thrown there (`MOM-08`).
  - **The unseen:** when nothing was unusual enough, or it came from the sky or the land (lightning, storm, flood, drought, quake), or was a sudden death, there is about one chance in three, more for the spiritual and frightened, of a weak belief that an unseen being did it (`MND-31`).
    So a lightning death can leave both a storm being and the climb up the hill that angered it.
  - **Strength:** a new link starts at about three times the thought's size, at most 50, and a link from a surprise at 20; when the cause recurs, the outcome within a day or two adds 15 and its absence takes 5, or 10 each for an outcome that comes on more than one day in ten, and below 5 the link is forgotten; of two links explaining one outcome, only the stronger gains.
  - **Who is blamed:** a person is linked to harm only if a stranger or disliked (`MND-24`), and someone sick or dying by that state, so people learn to avoid the sick (`BIO-05`).
  - **Room:** at most 3 links per kind of outcome, within the caps of `MND-14`, the weakest, new or old, dropped.
  - **Avoiding costs** (`MND-09`): hunger strong enough eats the forbidden food, with shame, and if nothing bad follows, the link weakens.
  - **Memories:** once a link's memories fade, it stands without its reason, as the details view shows (`PRE-14`).
  - **Done when:** in 20 band-scene runs, most adults shun a real poison within a season in 16 and a food eaten before a chance fever in 6, credit better spears for better hunts in 12, and never starve beside taboo food.

- `MND-31` **Beliefs about the unseen** *(Decided)*: Belief in beings nobody sees: spirits of places, animals and weather, and the dead.
  - **Start:** by the rule in `MND-05`, shaped by the event through the templates of `CUL-05` (the spirit of that hill, the storm, the bears, a dead grandmother).
  - **Growth:** a new being starts at about 20; each later event of its kind is put down to it and adds about 10, and each dream of it or rite held for it about 5; it never fades by time alone; an act linked to a good outcome of its kind (`MND-05`) is held to please it (`CUL-05`).
  - **Effect:** believers weigh what it is believed to want (offerings, rites, keeping off its places), and doing it brings a good thought and a sense of safety (`MND-29`).
  - **Room:** within the caps of `MND-14`, the weakest, new or old, is dropped.
  - **Done when:** where lightning kills a hunter on a hill, some witnesses believe in a storm being, and the band shuns the hill, each in at least 8 of 20 runs (`MOM-03`).

### 9.6 Choosing and planning

- `MND-09` **Choosing what to do** *(Decided)*: When an activity ends or is interrupted (`TIM-17`), the person scores each option by what it does for their needs, personality, plans and beliefs, and usually takes the best.
  - **Options,** about 30 at most, all from what they know (`MND-02`): meeting a need where they know they can; a step of a plan or ambition; a known blueprint with things in reach; joining a group plan or request (`CUL-22`); a social act (`MND-33`); tending, feeding or playing with an animal they keep or a young one near them (`MND-16`); play, rest, exploring, experimenting (`MND-11`); and the dark acts of `CUL-08`, only in their stated conditions.
  - **In reach:** carried, theirs or their family's, in their band's shared stores (`CUL-21`), or within about 30 m, and for a known blueprint also places their mental map says hold such things (`MND-28`); blueprints count only for results serving their most pressing needs or plans, at most about eight, and social acts only toward people within about 20 m or one person sought.
  - **Score:** what it should do for each need, weighted by how pressing it is and by personality, plus plans and ambition; beliefs, others' expectations (`CUL-06`) and a recent dream's pull, at most a mild need's (`MND-12`), add or take away; effort, time, distance and risk take away; all is scaled by the chance they expect it to work.
  - **Worth:** a thing is worth what it does for their needs, directly (eaten, worn, burned, slept under, stored) or as a tool or input to blueprints they know whose results do, up to about three steps back, less for each step.
    A thing of no known use is worth only the curiosity of trying it, a plentiful one less, and a kept animal what it gives, once they have used that.
  - **Habits:** what they usually do at that place, time and season scores a little higher, so days have a rhythm.
  - **Picking:** usually the best, sometimes one close behind, by chance (`TIM-16`), never changed by fortune (`GOD-04`).
  - **Reasons kept:** the three parts of the score that most put it ahead of the next best, such as "thirsty; believes the river is safe at dawn; plans to check the fish trap", and the two best options it beat.
    They are kept for the current activity and every event the history keeps (`PRN-13`).
  - **Done when:** adults knowing the four steps to dried hide, given flint and hides, make all four before winter, unplanned, in 16 of 20 runs (`MAT-22`), each choice showing three reasons.

- `MND-22` **Plans** *(Decided)*: Short plans of a few steps, each step scoring higher until the plan is done, fails or is dropped (`MND-09`): store food before winter, build a shelter, make a spear for tomorrow's hunt.
  - **Sources:** a season remembered as hard (`MND-28`), met by a plan the season before (store food, dry meat, gather fuel, make warm clothes); a goal's chain of blueprints (`MAT-22`); keeping and penning more animals, once kept animals are worth it; ambitions (`MND-32`); requests and group plans (`CUL-22`).
  - **Times:** a step can have a time and place, such as dawn at the ford or a group plan's start (`CUL-22`); when it is time to go, it interrupts what they do (`TIM-17`), and one missed by over an hour fails.
  - **Looking ahead:** a need a season away counts about half, more for the patient (`MND-20`).
  - **Done when:** a band hungry last winter that can dry meat or store nuts has food stored by winter in 16 of 20 runs; one never hungry stores little.

- `MND-32` **Ambitions** *(Decided)*: Each adult picks a life ambition at about 14 (`BIO-04`), from personality and life so far; a big event can change it, as a killing brings a wish for revenge.
  - **Each, with its goal and what it favours:**
    - master a craft: experience 8 in a sector; practice, the best teacher;
    - lead: being the one the band follows (`CUL-22`); generosity, success, challenges;
    - raise a big family: four children grown to 14; courting, child care, bringing food;
    - be a great hunter: hunting experience 8; big game;
    - heal: healing experience 7, or ten people nursed to health; caring for the sick;
    - know the unseen: being the one asked about spirits (`CUL-26`); rites, sacred places, dreams;
    - find new land: camping beyond anywhere their band knew (`MND-28`); exploring, leading a split (`CUL-30`);
    - grow rich: more things, stores or animals than anyone in their band (`CUL-21`); making, trading, keeping;
    - avenge a death: the killer or their kin harmed or paying (`CUL-31`).
  - **Effect:** favoured options score higher (`MND-09`) and make plans (`MND-22`).
  - **Reached or lost:** reaching it brings a long, strong good thought and a lifelong memory; losing it, out of reach or after about 20 years without progress, a lasting bad thought and perhaps a new ambition.
  - **Done when:** youths who chose to master stone reach stone experience 8 before their peers in 16 of 20 runs.

### 9.7 Skills and discovery

- `MND-06` **Experience and skill** *(Decided)*: Experience in 15 sectors and a skill in each known blueprint, both from 0 (none) to 10 (master), rising with use.
  - **Sectors:** stone, wood, fire, cooking, hunting, gathering, hides, building, healing, pottery, herding, farming, metal, art and music.
  - **Start:** a newly learned blueprint starts at skill 1; experience counts through each try's level (`MAT-04`), which also sets quality (`MAT-20`).
  - **Rising:** each try adds a little, more on a success or when taught (`MND-13`), less at high levels: practising most days gives level 5 in about 2 years and 10 in about 10 (`TIM-18`); everyday work in a sector, such as nursing or singing, counts too, and each learns at their own speed (`BIO-08`, `BIO-16`).
  - **Fading:** unused, both fade slowly, never below about half their best; a blueprint dies with its last holder, though things they made may be copied (`CUL-02`).
  - **Done when:** someone knapping most days reaches flake skill 5 in 1.5–3 years in 16 of 20 runs, and 10 years unused leave at least half.

- `MND-13` **Learning and teaching** *(Decided)*: How one person learns from another; how crafts spread through groups is Culture's (`CUL-01`, `CUL-02`).
  - **Watching** an unknown blueprint used, on purpose (the watch activity, most by children and the curious), gives a hunch (`MND-11`), and about five watched uses teach it at skill 1; a use seen while busy counts a quarter; the most skilled, the respected and kin are watched most (`MND-24`).
  - **Being taught:** someone who knows a blueprint, believes another doesn't (`MND-23`) and wants them to (kin, friends, the kind, or for a gift) teaches beside them (`TIM-17`).
    The learner tries at their level's full chance (`MAT-04`); a taught try adds about four times a lone try's skill, more with a better teacher; the first success makes it known.
  - **Being told** how something is made gives a hunch.
  - **Children** learn fastest.
  - **Remembered:** each known blueprint keeps from whom and how it was learned, for the lines of teaching (`PRE-35`, `GOD-09`).
  - **Done when:** with one adult who knows flakes, 3 in 4 adults can make them within 2 years in 16 of 20 runs (`RES-03`), and taught children learn faster than watchers.

- `MND-10` **Surprises** *(Decided)*: A result or sight never met or not expected, such as a stick smoking as it is twirled against dry wood.
  - **How it works:** noticed about 1 time in 2, from 1 in 4 for the least curious to 3 in 4 for the most, and half as often when busy, tired or frightened (`MND-03`), it gives a strong memory, a pull to look into it (`MND-07`), a link at its place (`MND-05`) and a hunch when it is a hint (`MND-11`).
  - **Done when:** in 20 runs where a stick smokes in someone's hands, a curious twirler keeps a hunch for fire in at least 10, an incurious one in fewer.

- `MND-11` **Four routes to discovery** *(Decided)*: Nobody knows a blueprint until they discover or learn it (`PRN-01`), or you reveal it (`GOD-13`); discoveries come by accident, by experimenting, from a dream's hint, or by copying.
  - **Chances:** when an activity ends that fits the action, inputs and place of a blueprint the doer doesn't know (`MAT-04`), it gets the chance a maker at their level would have, times about 1 in 20 by accident, 1 in 5 when experimenting, 1 in 2 with a hunch for it, times the blueprint's own discovery factor (1 unless tuned, `MAT-04`), rolled once per activity.
    A success, if noticed (`MND-10`), teaches it at skill 1.
    Any outcome a blueprint names as its hint (`MAT-04`), from its own roll, a plain use, another blueprint or a timer, gives whoever notices it a hunch for it; a timer's unmeant result, such as a copper bead, is a named discovery for whoever notices it first (`MAT-21`).
  - **Hunches:** guesses that some kinds of things, with an action, might give a result; a person holds a few (`MND-14`), dropping one after about 10 failed tries or a year unused.
  - **By accident:** any activity using a base action.
  - **By experimenting:** an option like any other (`MND-09`), pulled by curiosity and play, scoring more in good times (mood above 50, nothing pressing) and when a need below 20 has no known answer, then aimed at it, trying things whose known characteristics bear on that need (food for hunger, warmth or burn for cold); a curious adult in good times experiments about once a day, an average one once a week.
    A try is one activity: a hunch if they hold one; else a known action (`MAT-06`) on a thing like what it works on; else any base action, often a familiar one, on one or two things in reach, where they stand (by a fire, in water), so three or more inputs need an accident or a hunch; a failure still teaches about the things (`MND-04`).
  - **By copying:** a made thing they can't make shows its materials and the actions its marks show (chipped, ground, drilled, sewn, fired), giving a weak hunch, tried at the experimenting chance; seeing it made gives a full one.
  - **The route** of a discovery is its hunch's source (an accident, a dream, a dream you sent, copying, being told), else experimenting.
  - **Discovery pace** is tuned by these values, the same in every world: the three factors, each blueprint's discovery factor and difficulty, the chance a surprise is noticed, how often people experiment and the dream-hint chance (`PRN-17`, `RES-16`).
  - **Done when:** each route gives a discovery in at least 2 of 20 scene runs, and the sharp-stone test passes (`RES-03`).

- `MND-12` **Dreams** *(Decided)*: Each night a sleeper has one dream, from the last few days' strongest memories by feeling, surprise and need, now and then an older one; those memories then fade more slowly.
  - **Feelings:** its feeling lingers as a thought (`MND-29`): a nightmare leaves fear, a dream of the dead grief.
  - **Hints:** about 1 dream in 60, 1 in 20 while a need is below 20, a dream joins a thing, an action and a needed result from different memories into a hunch (`MND-11`), pointing 1 time in 3 to a real blueprint they don't know, chosen as for an idea dream (`GOD-03`), and otherwise to nothing.
  - **Kept** only if strong (a nightmare, the dead, a hint, a sent dream), then told (`CUL-24`), feeding beliefs about the unseen (`MND-31`).
  - **Done when:** a dream of a dead parent strengthens belief in ancestors, and natural dream hints give hunches at their tuned rate (`RES-13`).

### 9.8 Life together

- `MND-24` **Relationships** *(Decided)*: For each person they know (`MND-14`): face and name, kinship as believed, an opinion from −100 to +100, trust and respect from 0 to 100, favours owed (`MND-26`), shared memories and what they know of them (`MND-23`); and an opinion of each people they know of (`CUL-23`).
  - **Opinion:** a day with chats together +1, shared food or help +3, a gift +5, help in danger +10; an insult −5, theft or a broken promise −15, a blow −20, killing kin −80; apart, a point a week back toward 0; together, a point a week up between like personalities and down toward a hot temper.
  - **Trust** sets how far their word is believed (`MND-27`) and whether they are followed; it rises 2 each time their word proves true and falls 10 when it proves false; **respect** comes from the sources of `CUL-06` (skill, generosity, success, age), as their custom weighs them.
  - **Bonds:** lasting high opinion makes friends, rivalry rivals, harm enemies, and a killing can start a feud (`CUL-31`).
  - **Done when:** over a season, people who share food come to like each other and repeated insults turn liking to dislike, in 16 of 20 runs.

- `MND-33` **Social acts** *(Decided)*: Chat, share, give, trade, ask someone to make a thing, help, comfort, tell, ask, play, court, teach, gossip, insult, quarrel, fight and steal.
  - **Chosen like anything else** (`MND-09`), each moving opinions (`MND-24`) and needs (`MND-07`).
  - **Talk alongside:** chat, tell, ask and gossip run alongside work, walking, eating and rest, between people a few metres apart and out of danger, at the pace of `CUL-24`, ending no activity (`TIM-17`); comforting, courting, teaching, long stories at the fire, quarrels, fights and stealing are activities of their own.
  - **Kept:** each person's last ten or so topics told and heard, for their card (`PRE-45`).
  - **Gossip** moves a listener's opinion of someone about a quarter of the way to the speaker's, by trust.
  - **Giving and trade:** a gift leaves a favour owed (`MND-26`); a trade is a swap both sides value above what they give (`CUL-21`); making a thing for someone is paid with a gift.
  - **Courtship:** unpaired adults court the one they are drawn to (`BIO-17`) and like most, above about +40, with time, shared food and help; when each one's opinion of the other has stayed above about +60 for about 10 days, they pair, as their customs allow (`CUL-27`, `BIO-15`).
  - **Quarrels** turn into fights when anger runs high; fights hurt (`BIO-13`), and others step in, take sides or remember.
  - **Done when:** in a band scene with unpaired adults, a pair forms within a year in 16 of 20 runs, and people talk while they work.

- `MND-23` **Who knows what** *(Decided)*: People track which of their own blueprints and places each person they know also knows (seen using, gone there together, told), teaching and telling only those who lack it (`MND-13`, `CUL-24`); what others need and feel is read from what is seen (`MND-03`), not kept.
  - **Secrets:** people keep quiet about a valued place (a flint source, a food store, a rich patch) when telling those outside their kin could cost them, the greedy and proud most, and about their own theft or broken taboo.
  - **Following:** the curious, and those who distrust someone, may follow them at a distance to see where they go.
  - **Done when:** where a greedy man keeps a flint source secret, a curious watcher finds it by following him in at least 4 of 20 runs.

### 9.9 Scale

- `MND-14` **Every person has a full mind** *(Decided)*: Everyone has every part of this section at all times, never a cheaper mind for being far away or unwatched (`WLD-13`).
  - **How it works:** minds stay cheap because every part has a limit: capped choosing (`MND-09`), noticing (`MND-03`) and talk (`MND-33`); slow changes, such as mood, fading and drifting opinions, settled about once a game hour; and at most about 200 memories (`MND-18`), a few hundred places (`MND-28`), 40 cause links (`MND-05`), 5 spirits and 5 of their own dead (`MND-31`, `CUL-19`), 5 hunches (`MND-11`), 5 plans (`MND-22`) and 150 people (`MND-24`).
  - **Check:** no mind passes these caps.

- `MND-15` **Population limit** *(To test)*: How many people a world holds at a watchable speed is measured on the phone (`PLT-04`), aiming at about 2,000 at the speeds of `TIM-07`.
  - **Nothing caps births:** food, illness and danger set numbers, growth tuned in `BIO-04`; past about 2,000, time slows, no mind is simplified (`PRN-11`), and the game says so.
  - **Memory limit:** a world nearing it pauses with a notice, to read its history or start a new world, never crashing or simplifying.
  - **Budget:** a whole person, body, mind and talk, with their share of the animals near people (each about a twentieth of a person or less), uses about a thousandth of a second per game day (`PLT-01`), or less, so 1,000 people take about one core-second per game day, spread over the middle cores.
  - **Check:** the phone benchmark reports each mind part's share of the time in a camp of 30 and a village of 300, and a villager costs at most about twice a camper.

### 9.10 Animals

- `MND-16` **Animal minds** *(Decided)*: Simpler minds: needs, fear, herd ways, learned fear of people, and taming (`WLD-33`); near people each big animal has its own, and the rest are counts (`WLD-32`).
  - **Choosing:** people's scoring (`MND-09`) of the body's needs, safety and their kind's urges (herding, pack hunting, guarding young, territory, breeding) over a few options (graze, hunt, drink, rest, flee, follow, fight, play), with no blueprints, talk or plans.
  - **Herds choose as one,** led by the lead animal, each member keeping its own body, fear, memory, boldness and tameness; an animal chooses alone only when apart, hurt, cornered, hunting alone, tame or kept; herds and lone animals choose at most about once a game hour, or at once for danger.
  - **Fear:** danger seen, heard or smelt makes them flee or fight as their kind does.
  - **Calls** come only with what they do: an alarm as they flee, a call as a herd gathers or in the breeding season, and kinds that call at dusk, such as wolves (`SND-01`).
  - **Boldness** from birth sets how near people they come; **memory** holds a few places and the people and animals they know.
  - **Done when:** a deer herd near camp grazes, drinks and flees together, and near a wolf den children feed and keep a pup in at least 4 of 20 runs (`MOM-06`).

## 10. Culture and society

Culture is what people pass on rather than inherit: crafts, words, beliefs, customs and art.
With minds, it is the heart of the game.
Numbers here are starting values, tuned in tests (`PRN-17`).

### 10.1 How culture works

- `CUL-07` **Nothing social is scripted** *(Decided)*: Templates give the shapes; events decide which happen, never a script, a date or an era.
  - **How it works:**
    - **Templates** name no particular people, person, place or date (`PRN-07`); each needs its conditions in the world and people's own choices (`MND-09`).
      Each has five parts: its trigger (a kind of event or state); what it takes from the real event (place, animal kind, weather, person or act); what it makes (a belief, custom, role or myth, named in the language); which options it makes score higher or lower (`MND-09`); and how it grows, spreads and fades.
    - **Catalogues:** this section's lists are catalogues (`MAT-13`), checked like the others (`MAT-17`) but free to depend on beliefs.
    - **In the world, or for you:** bands, families, peoples, villages, customs, shared spirits, roles (leader, head of family, council, chief, shaman, priest, specialist) and alliances are facts people know and act on; religions, gods, wars and ages are names for you only (`PRE-39`) and change nothing.
    - **Book of ages:** a people's firsts of each kind, and its named spirits, festivals, myths, leaders and chiefs, sized as in `MAT-21`; the rest shows on cards (`PRE-35`).
  - **Check:** a test finds no template tied to a date, an era or a named people, place or person; each belief template and story shape has a scene where it appears in at least 5 of 20 runs after its event, and no scene shows one without its own; customs and roles are read from the pace-test worlds (`RES-07`), which end with peoples of different spirits, customs and kinds of leader.

- `CUL-33` **Pace of culture** *(To test)*: When culture first shows in typical worlds, in years from the start, beside the pace of discovery (`TIM-19`).
  - First shared spirit or belief in the dead: within 3.
  - First rite a band keeps (`CUL-34`): 3–10.
  - First myth: 5–20.
  - First band split: 5–25.
  - First festival: 5–30.
  - First feud, a killing answered by a killing: 10–50.
  - First new people: 40–75.
  - First raid: 30–100.
  - First chief: 75–175, after the first villages.
  - **Keeps going:** after Year 50, each people adds a new rite, myth or song at least every 12 years, and holds a gathering or festival most years.
  - **Tuned by** split sizes, the custom threshold, belief strength and the pull of gatherings, alike in every world (`PRN-17`).
  - **Check:** the pace tests (`RES-07`) read these from each world's records, each from the stage `RES-07` gives it; Keeps going passes in at least half the worlds.
  - **Proposed change:** add "First custom a band names (`CUL-06`): within 1", first in the list.
    P10 found every world's first custom in its first year, Years 0.1 to 0.6 in 20 runs, from a band's first three big kills, and the list gives customs no window.

### 10.2 Passing things on

- `CUL-01` **Learning from others** *(Decided)*: How groups keep their own ways; how one person learns from another, and from whom, is in `MND-13`.
  - **How it works:** of several routes to one result (`MAT-07`), the one most of their band uses scores higher (`MND-09`).
  - **Done when:** in 20 runs of a band that ploughs for fire, with one adult who drills, most children learn ploughing first in at least 16.

- `CUL-02` **Knowledge can be lost** *(Decided)*: Crafts, songs, stories and rites die with the last who know them, so small or isolated groups lose most (`MOM-02`, `MND-18`); only a Revelation of yours can bring a lost craft back at once (`GOD-13`).
  - **How it works:** a people loses a blueprint when its last holder in that people dies.
    A lost craft returns only by finding it again (`MND-11`), learning it from neighbours (`CUL-16`) or copying old things (`MAT-08`).
  - **Done when:** the scene of `MOM-02` passes, and the book of ages marks each loss and return (`PRE-05`).

- `CUL-16` **How things spread** *(Decided)*: Crafts, beliefs, customs, songs and words spread only where people meet.
  - **How it works:** at gatherings (`CUL-29`), with spouses (`CUL-27`), trade (`CUL-21`) and captives (`CUL-31`), or by copying things found (`MND-11`), so distance, mountains and seas keep groups apart.
  - **Done when:** in 20 runs, a craft reaches a band its holders meet yearly within 10 years in at least 16, and never one beyond a mountain range.

- `CUL-03` **Memory outside heads** *(Decided)*: Marks, tallies and pictures let some knowledge outlast the people who had it; writing is not part of the launch arc.
  - **How it works:** tallies cut in bone or wood are things people make (`MAT-04`) and change no choice.
    Painted rocks, piled stones and cut trees mark paths, graves, sacred places and borders (`CUL-23`), understood only by their makers' people, who learn them with their customs.
  - **Done when:** a band's tallies and marks stay where it left them (`PRE-09`), and another people that finds them gains no belief or place from them.

### 10.3 Language

- `CUL-17` **A language from the start** *(Decided)*: Each world has one language, made with the world, spoken by all its peoples, and never changing.
  - **How it works:** its sounds are drawn from the murmur's syllable bank (`SND-03`), with rules for joining them into words and a few hundred everyday words; it copies no real language (`SCP-20`).
    Each people coins its own words for new things, by joining old words, as *kesh*, 'bite stone' (`MAT-21`), or in the language's shape, and a craft learned from neighbours keeps their word.
  - **Done when:** worlds from different seeds have different sounds and words, and every name in a book of ages is made from its world's words.

- `CUL-18` **Names** *(Decided)*: People, places, peoples, spirits and things are named in the language, often after events, features or traits, and shown with their meaning in English (`PRE-37`).
  - **How it works:** a child is named after a trait, an event at the birth or an ancestor (`CUL-19`).
    Anything else is named by whoever first talks of it; at the start every place the bands know has a name.
    A name belongs to the people that coined it, and all its members use it.
  - **Done when:** in the pace-test worlds (`RES-07`), every person, spirit, discovery and place talked of has a name made from the language, used by all its people.

- `CUL-24` **Conversations** *(Decided)*: People talk all day; what they say is kept as topics, never as sentences, and heard as a murmur (`SND-03`).
  - **How it works:**
    - **Topics,** a closed list of 11: news of food, water, danger or a death; a place (`MND-28`); a memory, as a story (`CUL-11`); a belief; how something is made (`MND-13`); gossip; a plan or request (`CUL-22`); a question; comfort; a quarrel; a song or myth.
    - **Pace:** chats run alongside other activities (`MND-33`), about one topic every 10–15 minutes, and at most about 60 a person a day.
    - **For you:** shown as bubbles (`PRE-45`).
  - **Done when:** in 20 runs, news of a new spring told by one adult is known to most of the band's adults within 5 days in at least 16.

### 10.4 Belief and religion

- `CUL-05` **Beliefs from events** *(Decided)*: Strong events give rise to spirits, rites, offerings and taboos, shaped by seven templates; your acts as god are explained the same way (`GOD-06`).
  - **How it works:**
    - **The templates,** each filled with the real event behind a link or unseen being (`MND-05`): a spirit of the place; of an animal kind; of the sky or weather; the dead living on (`CUL-19`); a taboo, when harm follows an act (`CUL-20`); a rite, when a good outcome follows an act (`CUL-34`); an offering, when a bad time ends after something was given or left.
    - **Which template:** the one fitting the event's most unusual part: from the sky, a sky spirit; in a hunt, an animal spirit; at a place, a place spirit; a loved one dead and then dreamt of, the dead.
    - **Kind or angry,** as its event helped or harmed: an angry spirit pulls its believers to shun its place or leave gifts there, a kind one to thank it.
    - **Shared:** a belief most of a band's adults hold is the band's, and is named (`CUL-18`); a band shares at most about 5 spirits, a new one pushing out the least held.
  - **Done when:** in 20 runs of the scene of `MOM-03`, the spirit the band comes to share from the strike is of the sky, and no band in any test world shares more than about 5.

- `CUL-19` **Ancestors** *(Decided)*: Grief and dreams of the dead lead people to believe the dead live on and watch over their kin.
  - **How it works:** those who loved the dead dream of them (`MND-12`), each dream strengthening the belief in the dreamer and in those who hear it.
    The belief brings graves with things for the dead (`MAT-08`), rites at graves (`CUL-34`) and children named after ancestors.
  - **Done when:** in 20 runs of a band whose most respected elder dies, most adults hold her as an ancestor within 5 years in at least 10, and leave gifts at her grave.

- `CUL-20` **Taboos** *(Decided)*: Acts that come to be forbidden: eating a food, entering a place, killing an animal, working on a festival day (`CUL-29`), marrying certain kin (`CUL-06`).
  - **How it works:** harm that follows an act can make it forbidden, by the rule of `MND-05`; taboos also come with spirits and from being told.
    Breaking one brings fear and shame (`MND-19`), and punishment (`CUL-06`).
  - **Done when:** in 20 runs of a band most of whose adults hold a taboo on a fish, the band names it, and a member seen eating the fish is punished as its customs say in at least 16.

- `CUL-34` **Rites** *(Decided)*: A rite is one shared activity of about an hour, held before a task, at a place, at a death, or on a day of a people's calendar (`CUL-29`).
  It is made of one to three of six forms: singing, dancing, leaving a gift, burning a gift, painting or marking, and a shared meal.
  - **How it works:**
    - **Where rites come from:** the act a band credits for a good outcome (`MND-05`, `CUL-05`), held in the matching form (singing or dancing as itself, eating as a shared meal, anything put in a fire as burning a gift, applying as painting, any other act as leaving a gift); the burial custom (`CUL-06`); and festivals (`CUL-29`).
    - **Who leads:** the shaman or priest (`CUL-26`), else the leader (`CUL-22`), else whoever first did it; all who join start and end together (`TIM-17`).
    - **Effects:** each who joins gains belonging and a good thought (`MND-29`), believers feel safer (`MND-31`), and it costs the time and anything given.
  - **Done when:** in at least 2 of 20 runs of the scene of `MOM-04`, the hunting song becomes a rite most of the band holds, its forms animated and heard.

- `CUL-26` **Religion** *(Decided)*: Shared beliefs grow into religion in steps, each when its conditions hold.
  - **How it works:**
    1. **Shared spirits** a band names, fears or thanks (`CUL-05`).
    2. **Rites** held together (`CUL-34`).
    3. **Sacred places:** where a shared spirit's event happened, or rites have been held 3 times.
    4. **A shaman:** once a band shares 2 spirits, the adult highest in spiritual (`MND-20`), respect, and memories of vivid dreams or a grave illness survived becomes its shaman, who heals with rites and herbs (`BIO-23`); others weigh the shaman's word on spirits as if from their closest kin.
    5. **Myths,** by the rule of `CUL-11`.
    6. **Priests:** in a large village (`CUL-28`), the shaman becomes a priest, fed by the gifts rites and healing earn as help (`MND-26`), holding rites on the calendar (`CUL-29`) in a house set aside at the sacred place; the successor is whoever joined most of their rites, most often their child.
    - **For you only** (`CUL-07`): a people's religion is named after its greatest spirit, and a spirit most of its adults hold, with a rite, a myth and a sacred place, is named a god.
  - **Done when:** of the 20 pace-test worlds run to Year 30 (`RES-07`), at least 10 have a people with a sacred place, a shaman and a myth, and no step ever comes before its conditions hold.

### 10.5 Society

- `CUL-30` **Bands** *(Decided)*: People live in bands of a few families that move, camp and share together.
  - **How it works:**
    - **Families:** a couple or a lone parent with their children under 14, and any old parent living with them, headed by its most respected adult; children belong to their parents' band, and couples to the one their custom names (`CUL-06`), or until there is one, the one holding more of their kin.
    - **Moving:** the leader picks each next camp (`CUL-22`) from what members know (`MND-28`), a few times a year until the band settles (`CUL-28`).
    - **Splitting:** a band that moves splits past about 40 people, a village past about 300, and either past about 20 after a season most adults spent below condition 30 (`BIO-09`), a fight between heads of families or a failed challenge (`CUL-22`).
      The families with the lowest opinion of the leader (`MND-24`), up to about half the band and leaving at least about 10, are offered leaving (`MND-09`), again each season while the band stays past its size, with close kin who follow, their things and embers, to found a new band at least a day's walk away under their most respected, or to join a village of their people.
    - **Joining:** a band below about 10 people joins kin in another band or village.
  - **Done when:** a band of 45 after a fight between heads of families splits within a year in at least 15 of 20 runs, and a band of 8 joins kin.

- `CUL-27` **Kin and marriage** *(Decided)*: Everyone knows their kin; who may marry whom, where couples live and what is given are customs that differ between peoples (`CUL-06`).
  - **How it works:** kin are parents, children, brothers and sisters and partners, and through them grandparents, cousins and in-laws (`MND-26`).
    Two adults who court and pair (`MND-33`) are married once their families accept it, which they do unless a parent's opinion of the partner is below −20 or the match breaks a custom they hold; pairing is never shown (`BIO-15`).
  - **Done when:** in 20 runs of two bands with unpaired adults that meet yearly, a marriage joins them within 10 years in at least 16, never between close kin.

- `CUL-06` **Customs, norms and punishments** *(Decided)*: What most of a group does the same way becomes a custom; customs people expect are norms, and breaking one brings punishment.
  - **How it works:**
    - **Customs** answer 12 fixed questions:
      - how the dead are treated (left, under stones, buried, buried with things);
      - who shares a big kill (the hunter's family, the band);
      - who may not marry (close family, also cousins, also the band);
      - where couples live (with his kin, hers, either);
      - what a marriage brings (nothing, gifts, a feast);
      - whether a marriage can end;
      - who does each sector's work (men, women, anyone);
      - who leads after a leader dies (the most respected, the leader's child);
      - who punishes (the wronged's kin, the leader, the council);
      - how strangers are met (welcomed, watched, driven off);
      - what is done with captives (killed, kept, taken in);
      - what earns most respect (skill, generosity, success, age).
    - **A band's answer,** once it has had 3 cases, is the way at least two thirds of its cases went, counting the last 5 years or, where fewer than 5 came in them, its last 5; with no way at two thirds, it is the mixed answer (either, anyone) where there is one, else none.
      Who may not marry forbids each kind of match none of those marriages made, and a marriage can end once one has.
      Each answer is named (`CUL-18`); a new band starts with its parent's answers, and a people's answer is the one most of its bands hold.
      The respect answer starts even, and each new leader's biggest source of respect gains weight.
    - **Norms:** following a custom scores a little higher (`MND-09`), and each person who sees it broken loses about 10 opinion of the breaker (`MND-24`).
    - **Punishments,** by whoever the custom names, by the breach: scorn for a broken custom; left out of sharing for theft or a broken taboo; gifts to the wronged or a beating for a wound; driven out (`CUL-22`) for a killing in the band or a third breach in a year; and a chief may punish a killing with death (`CUL-08`).
  - **Done when:** a band that buries at least two thirds of its dead for 5 years names burial its custom, and each adult seeing a body left unburied loses about 10 opinion of those who left it.

- `CUL-22` **Leaders, councils and chiefs** *(Decided)*: Bands follow leaders, bigger groups decide in councils, and settled groups come to have chiefs.
  - **How it works:**
    - **Group plans,** a fixed list: move camp, hunt, gather or build together, hold a rite (`CUL-34`), go to a gathering (`CUL-29`), raid, defend, drive someone out, and settle a feud.
      Whoever decides for the group sets one, with a time and place to meet, most often the next dawn, and tells the others (`CUL-24`), and each member chooses whether to join (`MND-09`), pulled by their needs, trust in who decided, belonging and what most others do.
      Each plan is one shared activity naming the fewest who must come (about four hunters for big game), and starts or fails as `TIM-17` says; leaving a dangerous one early costs respect.
    - **Leaders:** a band follows the adult its adults trust and respect most, in sum (`MND-24`), respect being earned as the custom says, or the last leader's child where that is the custom (`CUL-06`).
      When a rival's sum passes the leader's by a fifth, a proud or ambitious rival (`MND-32`) challenges, and the band follows whichever more adults trust; the loser may fight, and their closest may leave (`CUL-30`).
    - **Councils:** in a village without a chief, and at a gathering for a shared rite, hunt or feud, heads of families decide together: each backs the plan they score best, and the plan whose backers the others respect most, in sum, wins.
    - **Chiefs:** in a large village (`CUL-28`), a leader of 10 years becomes chief for life, alone setting group plans, ruling on feuds (`CUL-31`) and punishing (`CUL-06`).
  - **Done when:** in 20 runs, hunters told of a hunt meet at its time and place and kill a red deer in at least 12.

- `CUL-32` **Specialists** *(Decided)*: People known for a craft work for others, and where food allows, some do it full time.
  - **How it works:** whoever spent most working days of the last year on one sector's work for others, paid with gifts (`MND-33`), is its specialist: those who want such work ask them first, and they teach for gifts (`MND-13`).
  - **Done when:** in 20 runs of a village of about 300 with full stores, someone works most days at one craft, fed by others, within 20 years in at least 10.

- `CUL-21` **Sharing and trade** *(Decided)*: Food is shared, gifts bind people, and groups trade what they have plenty of for what they lack.
  - **How it works:** big kills are shared as the custom says (`CUL-06`).
    At gatherings (`CUL-29`), people swap spare goods, such as flint, ochre or furs, each side valuing by its own needs and stock, with no fixed price and no money.
    Settled families own their houses, stores, fields and herds (`CUL-28`), and a people its land (`CUL-23`).
  - **Done when:** in 20 runs, two bands meeting, one with spare flint and one with ochre, swap some in at least 16.

- `CUL-31` **Feuds, raids and alliances** *(Decided)*: Killings breed feuds; hunger, greed and revenge breed raids; marriages, trade and shared enemies breed alliances.
  - **How it works:**
    - **Feuds:** a killing or a bad wound makes the victim's kin want revenge, weighed against the risk (`MND-09`), and each revenge can bring another; a feud ends with gifts the victim's kin value above revenge, a marriage between the sides, a council's or chief's ruling, or one side leaving.
    - **Raids** are a group plan (`CUL-22`): a hungry, greedy or vengeful band raids when its leader believes it can win by numbers or surprise, for food, stores, herds or captives.
    - **Captives** are killed, kept or taken in as the custom says (`CUL-06`); kept ones live in the raiders' band against their will, work and share by its customs, try to flee while their opinion of their captors is below −20, and may marry in; their children belong at birth.
    - **Alliances:** two peoples whose relation is above +40 (`CUL-23`) and who shared a marriage or a gathering in the last 10 years are allies, sharing hunting grounds and joining each other's defence when near, until it falls below +20.
    - **Wars,** for you only: three or more raids each way between two peoples within 10 years are named a war in the book of ages (`PRE-05`).
  - **Done when:** in 20 runs after a killing between two bands, revenge is tried within 5 years in at least 5, and gifts, a marriage or a ruling end the feud in at least 5.

- `CUL-08` **Dark history can happen** *(Decided)*: Dark events are violence and killing, war, captivity and slavery, sacrifice, cruelty, infanticide and cannibalism; they arise like anything else, and are told only in pattern sentences, never by the writer AI (`PRE-17`).
  Sexual violence is not part of the game (`BIO-15`), and what is shown follows `PRE-18`.
  - **How it works:** each comes only from a named option: violence, war and captivity from fights and raids (`CUL-31`); cruelty from punishments (`CUL-06`); leaving a newborn, by a mother below condition 30 (`BIO-09`) still nursing a child under 2; eating the dead, below condition 15 with no other food known within a day's walk; a captive's life as an offering (`CUL-31`), only where most adults hold an angry spirit at 70 or more after a second disaster put down to it within a year (`CUL-05`).
    Infanticide, cannibalism and sacrifice are possible, not promised (`RES-19`).
  - **Done when:** in 20 runs of two hostile bands in a lean year, a raid with a death comes in at least 5, stated in the book of ages by its pattern sentence.

- `CUL-23` **Peoples and territories** *(Decided)*: Linked bands make a people, with its own name, customs and land; peoples split, merge and vanish.
  - **How it works:**
    - **The first people:** the starting bands, linked to each other as of Year 1, with a name for themselves (`BIO-03`).
    - **New peoples:** two bands are linked if they camped together (`CUL-29`) or shared a marriage in the last 25 years; when a people's bands fall into two parts with no link between them for 50 years, the smaller becomes a new people, named after a place, a founder or a spirit, and its customs, beliefs, style and new words go their own way.
    - **Merging and ending:** when, over 50 years, most of a smaller people's marriages are with one bigger people, it joins that people (links alone never merge peoples); a people ends when its last band dies out or joins another.
    - **Territory:** the land its bands use (camps, hunting grounds, sacred places, graves), where strangers are met as its custom says (`CUL-06`).
    - **Relations:** each adult holds an opinion of each people they know of, from −100 to +100, moved by marriages, trade, gatherings, raids and killings lived or told (`MND-24`); a people's relation to another is the average opinion of its adults who know of it.
  - **Done when:** in 20 runs of the start's bands parted by a mountain range from Year 1, two named peoples exist by Year 50 in at least 16.

- `CUL-28` **Villages** *(Decided)*: A village is a place where one band has kept its camp all year round, whatever trips its members make, for 5 years in a row, in lasting houses it built (pit or post houses, `MAT-23`).
  A cave or hut camp lived in all year is a home, not a village.
  - **How it works:** a band settles when stores, fields, herds or rich fishing feed it through every season, so staying beats moving (`MND-09`).
    Villages grow by births and by kin moving in, and split past about 300 (`CUL-30`); stores draw raiders (`CUL-31`).
    From about 60 people it is a large village, where chiefs and priests can arise (`CUL-22`, `CUL-26`).
    Drought, failed harvests, illness or raids can empty a village, leaving an old camp (`PRE-09`).
  - **Done when:** a band by a salmon river that knows pit houses and storage becomes a village within 20 years in at least 10 of 20 runs.

### 10.6 Expression

- `CUL-09` **Visual art** *(Decided)*: Paintings and carvings composed from motifs drawn from the model kit (`PRE-46`) in each people's style, most often showing real events.
  - **How it works:**
    - **What is shown:** one of the maker's strongest memories, often a hunt, a death or a flood, or a myth (`CUL-11`): 1–8 motifs for its real animals, people and things, in a row, a ring or a scatter as the style says (`CUL-12`), numbers shown roughly (three deer for a herd).
    - **Skill:** low art skill gives fewer motifs, rough lines and one colour; high skill more motifs, clean lines and up to three colours (`MND-06`), from the pigments at hand, ground green ore among them (`RCK-15`, `MAT-19`).
    - **Kinds:** pictures on walls, rocks, hides and the flat sides of things; ornament, the people's patterns on pots, clothes, tools, beads and bodies; and carved or clay figures (`PRE-46`).
    - **Made** after strong events, at rites or in play (`MND-20`).
  - **Done when:** in at least 2 of 20 runs of the scene of `MOM-07`, a painting shows the hunt's real animals and hunters in 1–8 motifs, in its people's style.

- `CUL-10` **Music and dance** *(Decided)*: Songs in each people's musical style, played on instruments made from blueprints, and danced to with each people's own steps.
  - **How it works:**
    - **Musical style:** a scale of 4–6 notes, 2–3 favourite rhythms from a set of about 20, and a pace, part of the people's style (`CUL-12`).
    - **Songs:** now and then someone with music skill makes and names one: a tune of 8–16 notes, repeated with small changes for up to a minute, about a hunt, a death, a spirit or a love but with no words, played on flutes, drums or rattles (`SND-02`, `MAT-23`).
      Before a people has songs, people chant with no set tune.
    - **Singing together** lifts mood and belonging for all who join (`MND-29`).
    - **Dances:** 4–8 of the 8 dance moves of `PRE-44`, in a ring or a line, in step with the beat (`MND-26`).
  - **Done when:** in 20 runs of a band with a skilled singer, a song it makes is still sung 10 years later in at least 10, always in its people's scale and rhythms.

- `CUL-11` **Myths and stories** *(Decided)*: Retold memories become stories, and stories of spirits, beginnings and remarkable people become myths and legends, changing a little with each telling.
  - **How it works:**
    - **Retelling:** a hearer keeps a told memory as a story from the teller, weaker than their own (`MND-18`); a story most of a band's adults hold is the band's.
    - **Drift:** about one telling in five changes one detail: numbers grow, the teller's part swells, a deed moves to a more famous person, a spirit's part grows, or a cause shifts toward the teller's beliefs.
    - **Myths and legends:** a shared story about a spirit, the people's beginnings or a first, still told 10 years on, becomes a myth; one about a person, still told after their death, a legend.
    - **Story shapes (12):** how a spirit came to be; how a gift came, taken from a spirit or given by one; how the people began; the great flood; the great winter or drought; the great hunt; a hero's deed; why a taboo is kept; why a rite is kept; the journey to new land; the fight with another people; the first death.
      Each has five roles (hero, spirit, gift, place, foe), filled from the story's records.
    - **Images** come only from the people's beliefs and the story's shape, written into the myth's pattern sentences (`PRE-17`).
    - **Kept** by telling and lost with the last who remember (`CUL-02`).
  - **Done when:** in 20 runs of a band that learns fire by drilling, most adults tell a myth of how fire came 20 years later in at least 5, every detail from a record or belief.

- `CUL-12` **Style and ornament** *(Decided)*: Each people has its own look in things, art and music, drifting over time, so a thing shows who made it and roughly when.
  - **How it works:** a style is six choices: proportions (squat to tall), lean, favourite pattern, two favourite colours, how motifs are drawn (outline or filled, thin or bold) and how much ornament, plus the musical style (`CUL-10`).
    A new people starts with its parent's; about every 12 years one choice shifts a step, toward a people its bands are linked with, if any (`CUL-23`).
    Every made thing carries its maker's people's style (`PRE-43`), and everyone wears their people's ornament (beads, body paint, decorated clothes), the most respected most.
  - **Done when:** in the pace-test worlds run to Year 75 (`RES-07`), any two peoples apart for 38 years or more differ in at least 2 of the 6 choices.

- `CUL-29` **Gatherings and festivals** *(Decided)*: Bands meet where food is plentiful, and meetings kept at the same place and season become festivals.
  - **How it works:**
    - **Gatherings:** two or more bands camped within about 2 km of each other for a day or more, a settled band coming as a trip of those who join (`TIM-17`).
      A leader's choice of camp (`CUL-22`) weighs how many members want to meet kin, friends or a partner in other bands, so in a season of plenty bands head for where they last met them then.
    - **Calendar:** a yearly sign seen two years running (`MND-28`), such as first frost, herds passing, nuts falling or the full moon (`WLD-07`), is named when first talked of (`CUL-18`), such as "when the salmon come", and plans and festivals keep to it (`MND-22`).
    - **Festivals:** a gathering at the same place (within about 5 km) and season 3 years running, with a rite held together (`CUL-34`), becomes a festival, named and kept by its sign, most often that season's full moon; it lasts about 3 days, and is forgotten if not held for 3 years.
    - **What happens:** feasts, rites, songs and dances (`CUL-10`), myths, marriages (`CUL-27`), trade (`CUL-21`) and councils of heads of families (`CUL-22`), and news and crafts spread (`CUL-16`).
  - **Done when:** three bands near one nut grove hold a named festival within 20 years in at least 10 of 20 runs.

## 11. Presentation

### 11.1 Visual style

The look, written to stand without any image.

- `PRE-01` **Detailed pixel art** *(Decided)*: Everything on screen is crisp pixel art, drawn in art pixels (`PRE-22`): limited colours from one palette (`PRE-20`), hard edges, no blur, no smooth gradients.
  - **Done when:** each art pixel of the world is one solid palette colour.

- `PRE-02` **Pixel-rendered 3D** *(Decided)*: A real 3D world drawn at low resolution, so it looks like hand-made pixel art with real depth, scale and structure; the camera turns freely and zooms continuously.
  - **Done when:** `PLT-04`'s benchmark worlds stay smooth at every zoom stop as the camera turns.

- `PRE-20` **Colour in steps** *(Decided)*: Every material has a ladder of about 4–7 shades from one master palette, made from its colour (`MAT-10`) or by hand for common ones, and the light (`PRE-30`) picks the step.
  Two steps meet in a clean edge, with no pattern mixing them, and surfaces carry no fine grain, only the lines and broad shapes of what they are made of, such as rock layers, seams and courses of thatch.
  - **Done when:** no surface is speckled, and the edges between steps hold still as the camera pans.

- `PRE-21` **Outlines and lit edges** *(Decided)*: A one-pixel dark outline wherever one thing stands in front of another, and a one-pixel bright edge where the sun or a fire catches a shape (`PRE-30`), such as a cliff's sunlit rim or a person's fire-facing side.
  - **Done when:** at every zoom stop, each figure, tree and rock in front of something is outlined.

- `PRE-22` **Stable pixels** *(Decided)*: Pixels never crawl or shimmer while the camera is still or panning: it snaps to whole art pixels and turns ease to rest; an art pixel is about 2 by 2 screen pixels at the person, growing with the zoom to about 6 by 6 at the globe, so small things blend into the pixels as you zoom out, in portrait and landscape.
  Some crawling in a free turn or zoom can't be avoided without blur; the fix that best lessens it is chosen at the first visual review (`PRE-31`).
  - **Done when:** with the camera still or panning, frames change only where something moved, or by whole pixels.

- `PRE-23` **Rock faces** *(Decided)*: Cliffs show the rock layers where they stand (`WLD-09`), which go on underground (`PRE-25`): layers of different thicknesses, cracks and fissures, lichen and water stains where the face is wet (`WLD-16`), soot above lived-in caves (`MAT-18`), grass hanging over the top and scree at the foot.
  - **Done when:** cliffs of three kinds of rock show their own layers, and a cave lived in for 10 years shows soot.

- `PRE-24` **Real shapes** *(Decided)*: Overhangs, caves, rock shelters and buildings have real depth, as part of the ground's shape (`WLD-12`) or from their models (`PRE-46`), lit inside only by openings and fires (`PRE-30`).
  - **Done when:** circling a cave and a hut at close camp zoom, you see into both, dark but for openings and fire.

- `PRE-25` **Cut-away view** *(Decided)*: The ground can be sliced open along a line you choose, showing rock layers (`WLD-09`), soil (`WLD-27`), water in the ground (`WLD-17`), and hearths, tools, bones and graves where they were buried (`MAT-08`).
  - **Done when:** a slice through a camp left 200 years before shows its hearth, bones and tools at their depths.

- `PRE-26` **Water** *(Decided)*: Rivers meander and change width, with gravel bars, reeds, lines that follow the current, ripples at fords, glints of sun and drifting mist, all from the river's course and flow (`WLD-17`) and the weather (`WLD-16`).
  Shallow water shows its bed, deeper water darkens away from the shore, and a thin bright line marks where water meets land or anything standing in it.
  - **Done when:** every river is at least one art pixel wide from valley zoom inward, and farther out those draining about 1,000 km² or more (tuned).

- `PRE-27` **People and animals** *(Decided)*: Small 3D figures of tiny blocks, with a separate head, torso, arms and legs, posed about 10 times a second (`PRE-44`), so they look like crisp pixel art from any angle.
  - **How it works:** a figure is built from its body's parts (`BIO-13`) and looks (`BIO-08`, `BIO-22`), wearing and carrying what the person has (`PRE-42`), with their strongest feeling on its face (`MND-19`); animals use their body pattern (`PRE-46`).
    Of the body it shows only build (thin, average or stout), age (grey hair, a bent back), a limp or a sling, a pale dressing or splint, scars or pox marks as a few darker pixels, and blood at the Show level (`PRE-18`).
  - **Done when:** at person zoom, each body sign above shows, and build, age and clothing tell people apart.

- `PRE-28` **Readable from far away** *(Decided)*: Zooming out, people and animals become tiny outlined figures in their strongest colours, a group or herd close together one marker, and a camp a point at its hearth that glows if it has a fire.
  - **Done when:** a camp of 30 people stays readable at every zoom stop, with no jump as its forms change.

- `PRE-29` **From above** *(Decided)*: As the camera rises it tilts toward straight down, and the land shows what can be seen from above at every height, vivid and textured to the pixel: forests as clumps of crowns, grassland, deserts, rock and snow in their own colours (`WLD-12`), hills shaded, rivers as lines, the sea from deep blue to turquoise shallows with its currents, the clouds of the weather and their shadows (`WLD-16`), and at the top the globe (`WLD-02`); overlays sit on it (`PRE-07`).
  It is lit by the sun where the hour and season put it.
  - **Done when:** rising from valley to globe, the view changes without a jump, and coasts and rivers stay visible.

- `PRE-30` **Light, time and season** *(Decided)*: Light follows the sun, the sky and the air, as in Minecraft's Vibrant Visuals: the sun's and the sky's colours come from the sun's height through the air, and the master palette's versions for dawn, day, dusk and night in each season are that light on every material.
  The sun casts real shadows by hour, season (`TIM-18`) and latitude (`WLD-01`), sharp near what casts them and softer as they lengthen; shade takes the sky's colour and hollows are darker; distance adds haze, warmer toward the sun; water reflects the sky and what stands above it, and glints.
  When a day passes in less than about 10 seconds (tuned), the light holds steady from high up and only its tint follows the hour, so the screen never flickers between day and night.
  A fire is a warm, flickering light as bright as its heat (`MAT-18`), warming nearby faces, glowing, and sending up smoke and embers.
  - **Done when:** one place at dawn, noon, dusk and night, in summer and winter, shows each palette and its shadows.

- `PRE-03` **Seamless zoom** *(Decided)*: One continuous zoom from the globe down to one person chipping flint, through these stops, each with its speed in `TIM-01`:
  - **person:** about 8 m across, a person about 58 art pixels tall;
  - **close camp:** about 20–50 m, a person about 10–25 art pixels tall, every figure in full;
  - **camp:** a few hundred metres, people as tiny figures (`PRE-28`);
  - **valley,** about 10 km; **region,** about 100 km; the **world map**; the **globe** (`PRE-29`).
  - **How it works,** by distance from where the camera looks: within about 300 m, from camp zoom inward, full areas (`WLD-12`), made for the picture without changing anything (`WLD-13`); out to about 10 km, each area's coarse ground, made from the seed in a moment, shaped every few tens of metres, under its cover (its cell's, or a kept area's own) drawn as forest canopy, scrub, grass or bare ground; beyond, and from region zoom out, the world cells (`PRE-29`).
    People, herds, camps and buildings show at every distance (`PRE-28`), and a full area being made shows its coarse ground until its detail fades in, within about a second.
  - **Done when:** a pinch from globe to person over unvisited land never stalls, and full detail is in within about a second.

- `PRE-31` **Visual review** *(Decided)*: At every milestone stage the look is reviewed on a contact sheet made on the phone, on one page, from fixed saved worlds: each zoom stop at noon and dusk in portrait, one landscape view, the model sheet (`PRE-46`), and three short clips of people at work.
  - **Check:** the contact sheet meets every Done when of 11.1 and 11.2, judged by the review and then by you (`PRC-10`).

### 11.2 Things and movement

- `PRE-42` **Built from their materials** *(Decided)*: Each thing is drawn from its model, whose parts take the colours and shapes of the materials used: a hut of birch poles and hides looks pale and brown, one of reeds straw-yellow, and more poles make a bigger hut.
  - **How it works:** parts are sized by the amount used (`PRE-46`), and icons come from the same model.
    States, wear and quality show on it: meat darkens as it dries, edges chip, bindings fray (`MAT-19`, `MAT-20`); a change to ground or a body shows there, such as a sown plot or a dressing.
  - **Done when:** every new thing a named result makes has its model, and two routes in different materials look clearly different, huts at close camp zoom.

- `PRE-43` **Variety** *(Decided)*: No two things look quite alike: each varies a little in proportions, lean, wear and colour by its own seed, within its model's limits, and looks the same each time.
  Its maker people's style (`CUL-12`) sets its proportions, lean and colours within each ladder, and puts their pattern, one of about 12 (notches, bands, dots, zigzags, fringes, painted rings), on the parts its model marks as decorated, as much as the style says.
  Trees, bushes, rocks and ground cover vary the same way, without style (`WLD-31`).
  - **Done when:** at person zoom, things of two peoples, or of one people 100 years apart, are clearly told apart.

- `PRE-44` **Animations** *(Decided)*: Every activity has its own movement, posed about 10 times a second, so you can tell who is knapping or dancing.
  - **The list:** the base actions (`MAT-06`), heat with a variant for blowing on a fire, the everyday activities (`BIO-21`), and wading, swimming, climbing, rage, despair, nursing or carrying a baby, lying hurt and lying dead: about 45 movements, each a loop of 2–6 key poses, as long as the activity (`TIM-17`).
  - **Social acts** (`MND-33`) are talking plus one of about 8 gestures: pointing, giving, embracing, pushing, a raised fist, waving away, holding hands, stroking.
  - **Dances** are 8 moves (step, stamp, turn, sway, crouch, leap, clap, arms raised), strung as `CUL-10` says; rites (`CUL-34`) use movements on the list, done together.
  - **Variants** are a few rules that bend any movement, never new animations: children quicker, elders stooped and slower; a limp or a still arm from a wound (`BIO-13`); grief slumps, fear quickens, anger stiffens (`MND-19`); cold hunches; skill steadies the strokes (`MND-06`).
    Each figure's timing is offset by its seed, so a crowd never moves in step, except dancers and singers keeping a shared beat (`MND-26`).
  - **Animals:** one set per body pattern (`PRE-46`), timed by the species' size and speed: stand, walk, run, feed, drink, rest, sleep, play, fight, call, fall, and swim or fly.
  - **At speed,** each figure keeps showing its activity at a steady pace (`TIM-01`).
  - **Done when:** every movement reads at person zoom; at close camp zoom, standing and ground work, carrying, walking, resting, fighting and dancing are told apart.

- `PRE-46` **The model kit** *(Decided)*: Everything in the world is drawn from one fixed kit, so the content stays countable.
  - **Shared shapes,** one per form (`MAT-02`), stretched to a thing's size and coloured by its material: raw items need no model of their own, and ground cover is drawn by the patch (`WLD-31`).
  - **Made things:** a model is a layout of parts, each from the shared shape of the input that made it (`MAT-04`); only a part no shape fits, such as a pot's body or a blade's outline, is drawn for the model, and states need none (`MAT-19`).
  - **Plants:** about 8 forms (needle tree, broad-leaved tree, bush, grass or grain, herb or flower, reed, root plant, fungus); a species sets its height, crown and colours by season and stage (`WLD-31`).
  - **Animals:** about 6 body patterns (hoofed, padded, small and quick, bird, fish, legless reptile); a species is its proportions and colours, with antlers, horns or tusks (`WLD-32`).
  - **People:** one figure (`PRE-27`) with about 8 kinds of garment, in child and adult sizes, and hair, beads and paint in each people's style (`CUL-12`).
  - **Pictures and figures:** a painting or carving shows each animal, person or thing as its model seen from the side, drawn flat in its people's style (`CUL-09`, `CUL-12`), and a carved or clay figure is the model itself, small, in its material; only about 12 signs, also the patterns of `PRE-43`, are drawn by hand.
  - **Done when:** every launch thing, plant and animal is drawn from the kit, shown on a model sheet in two materials.

### 11.3 On the screen

- `PRE-32` **World first** *(Decided)*: The world fills the screen; controls and panels appear only when you ask (`PRE-33`), apart from a live moment appearing briefly (`PRE-08`) and talk bubbles (`PRE-45`).
  - **Done when:** a few seconds after any touch, only the world, live moments and bubbles are on screen.

- `PRE-34` **Both orientations** *(Decided)*: Every screen works one-handed in portrait and two-handed in landscape (`VIS-14`, `PLT-02`): each view is one column of panels, full width in portrait with its controls at the bottom, beside the world in landscape.
  - **Done when:** every view works both ways, with its portrait controls in the bottom third of the screen.

- `PRE-33` **Gestures** *(Decided)*:
  - drag to move, and twist with two fingers to turn;
  - pinch to zoom, or double-tap and drag with one thumb, which also sets the speed of time (`TIM-01`);
  - tap to select what is under the finger and open its card (`PRE-35`);
  - long-press for your powers at that spot (`GOD-10`), including drawing an area, so a drag always moves the camera;
  - swipe up on, or tap, the small handle above the bottom edge for the views (the edge itself is the phone's);
  - every touch also shows the date, the real speed and the time controls (`TIM-04`, `TIM-11`), fading after a few seconds.
  - **Done when:** in a scripted test no gesture is read as another, and zoom, select, views and time each work with one thumb.

- `PRE-35` **Cards** *(Decided)*: Selecting anything opens a card with what matters about it, in English with the people's own names and words beside, linked to deeper views.
  - **A person:** name, age, people, mood, what they do and why (`MND-09`), their ambition and how close they are (`MND-32`), and their body: body words, each part's health, wounds, illnesses and condition (`BIO-08`, `BIO-09`, `BIO-13`).
  - **A thing:** its materials, its characteristics marked by which ones its owner's people know, wear and quality, and who made it and when (`MAT-10`, `MAT-20`).
  - **A craft:** who holds it, and the line of teaching back to its first maker (`PRN-04`, `MND-13`).
  - **A place:** its name and meaning, its land, and what happened there.
  - **A band or people:** name, numbers, territory, crafts with how many hold each, customs, beliefs, rites, myths, songs, art, calendar, festivals, leaders, and how it stands with other peoples (`CUL-23`).
  - **Done when:** in a test world, each kind of card fills every line above from the records.

- `PRE-45` **What they talk about** *(Decided)*: Speech is a murmur, never real words (`SND-03`), so a small bubble over the speaker shows a picture of the topic (`CUL-24`): one for each kind of topic, or the icon of the thing, plant or animal, or the face of the person talked about.
  Bubbles show while figures are drawn in full (`PRE-03`); at speed each stays about 2 seconds, at most about 4 at once, nearest first.
  The speaker's card lists recent talk by fixed patterns (`PRE-37`).
  - **Done when:** at close camp zoom, a chat on each kind of topic shows its bubble and appears on the speaker's card.

- `PRE-40` **Screens** *(Decided)*: Besides the world: a first-launch screen that goes straight to choosing among the best candidate worlds (`WLD-10`), a list of your worlds (`TIM-08`), settings and credits.
  Settings hold the content level (`PRE-18`), the live-moment level (`PRE-08`) and the volume of their music, voices and the world; the credits list every recording with its source (`SND-06`).
  Settings also open the review page, which runs a stage's phone checks in one go (`PLT-04`, `PRE-17`, `PRE-31`, `SND-12`).
  Short help cards appear the first time you use something; there is no tutorial (`SCP-02`).
  - **Done when:** from a fresh install, a new world is chosen and opened without help.

### 11.4 Following the story

- `PRE-05` **Book of ages** *(Decided)*: The world's chronicle, recorded as history happens and worded as `PRE-41` sets out, with a timeline for each people (`CUL-23`).
  What `PRE-39` marks becomes entries, sized as in `MAT-21`, grouped into ages, dated (`TIM-14`) and linked to the people, places and things behind them.
  - **How it works:** your own acts can show as separate marked lines, never in the text (`GOD-07`), and tapping one shows what came of it (`GOD-09`).
  - **Example:** "Year 3, summer, day 9: Ama of the Hazel band, of the Tavu, struck the first sharp flake by the river.
    They call it *kesh*, 'bite stone'."
  - **Done when:** a test world of 100 years lists its entries by date and by people, each opening what it names.

- `PRE-06` **Follow a soul** *(Decided)*: Pick anyone and follow their life, the camera staying with them if you wish.
  Those you follow are a tab of the book of ages, apart from the camera, so you can follow several; their notable moments come as live moments (`PRE-08`), and when one dies, the game offers those closest to them (`MND-24`).
  - **Done when:** following three people, their notable moments come as live moments, and a death offers their closest.

- `PRE-07` **Map overlays** *(Decided)*: Six overlays on the map look (`PRE-29`), always from the world as it is now:
  - peoples and territories (`CUL-23`), marked with the sick (`BIO-05`), each camp's mood (`MND-29`), and graves and old camps (`PRE-09`);
  - who knows a craft (`MND-06`);
  - a chosen belief: the share holding it, and how strongly (`MND-27`, `CUL-26`);
  - what a chosen person or people knows of the land (`MND-28`);
  - plants, water, stone and deposits, herds and tame animals (`WLD-31`, `WLD-17`, `WLD-14`, `WLD-32`, `WLD-33`);
  - weather and seasons (`WLD-16`).
  - **Done when:** in a test world, each overlay matches the records at 20 sampled places.

- `PRE-08` **Live moments** *(Decided)*: Only what matters interrupts you: moments and signs from the story director's one list, such as a major named discovery (`MAT-21`), within its one budget (`TIM-02`); the closer you watch, the higher a moment's score must be.
  Moments you don't take wait in a tab of the book of ages, and the level is a setting (`PRE-40`).
  - **Done when:** watched from the globe for an hour, a test world's live moments keep within `TIM-02`'s budget.

- `PRE-39` **Recognising what emerges** *(Decided)*: The game spots, for you only (`CUL-07`), what is worth telling: named discoveries (`MAT-21`) and other firsts, crafts lost with their last holder (`CUL-02`), new peoples and villages (`CUL-23`, `CUL-28`), leaders, feuds and alliances, and disasters.
  It also names religions, gods and wars (`CUL-26`, `CUL-31`).
  - **Firsts** count worldwide and for each people: any event of a kind the history has never recorded; a rediscovery after a loss is marked as one.
  - **Ages** begin only at these turning points: a step of the arc first reached anywhere (`TIM-19`), a new people (`CUL-23`), the first village (`CUL-28`), and a war (`CUL-31`).
    Each is named by a fixed pattern from its defining event and that event's name in their language, such as "The age of *hesoru*, fire from wood", and lasts at least 20 years (tuned) before another begins.
  - **Done when:** in the pace tests every age starts at a listed turning point, and a code check finds no path from recognisers back into the world (`WLD-13`).

- `PRE-09` **Graves and old camps** *(Decided)*: The dead and the places people left stay in the world (`MAT-08`); the book of ages lists them by people and date.
  A grave shows who lies there, how they died (`BIO-14`), who buried them and what was laid with them, and opens their card and life story (`PRE-37`); an old camp shows its hearths, rubbish heaps, lost tools and bones, and who lived there and when.
  Tapping a find shows who made or left it, and when.
  - **Done when:** after 200 test years, every grave and old camp listed can be visited, each find naming its maker and date.

- `PRE-10` **Family trees and legends** *(Decided)*: Family trees across generations from the birth records (`BIO-15`) and marriages (`CUL-27`), with the people's own legends (`CUL-11`) beside them, each linked to the true events it tells of.
  - **Done when:** a person's tree shows four generations from the birth records, and their legends link to real events.

- `PRE-14` **Details of a mind** *(Decided)*: For anyone, everything in their mind, shown plainly under a short summary (`PRE-41`): needs (`MND-07`); mood with each thought behind it (`MND-29`); feelings (`MND-19`); personality (`MND-20`); breakdowns (`MND-30`); memories and dreams, most important first (`MND-18`, `MND-12`); the mental map (`MND-28`) and knowledge of things (`MND-04`); blueprints with skill and experience (`MND-06`); hunches (`MND-11`); who knows what (`MND-23`); beliefs with how sure they are and the events behind them, or "reason forgotten" (`MND-27`, `MND-05`); plans (`MND-22`); ambitions (`MND-32`); relationships (`MND-24`); and the top reasons for what they do now, with the options it beat (`MND-09`).
  Your own acts on them are marked as yours (`GOD-09`).
  - **Done when:** for a test person, every kind of record above shows, and the summary changes once their mood does.

- `PRE-15` **Art that remembers** *(Decided)*: A painting, carving, bead or figure has a card (`PRE-35`) showing the picture, its maker and what it shows, linked to what really happened (`CUL-09`, `PRN-15`).
  In the world, paintings and carvings are drawn on the surface they were made on, from their motifs (`PRE-46`).
  - **Done when:** in the scene of `MOM-07`, the painting shows on the wall and its card links to the hunt.

- `PRE-16` **Bestiary** *(Decided)*: Species cards, listed on a page of the book of ages and reached by tapping any plant or animal, show each kind's look in each season, where and when it lives, its yields and danger, and for animals their numbers and herds (`WLD-31`, `WLD-32`); tame and domestic kinds have their own (`WLD-33`).
  What each people calls it and believes about it shows once that people knows it (`CUL-18`).
  - **Done when:** every launch species has a card.

- `PRE-13` **Few screens, everything findable** *(Decided)*: Anything the world keeps track of can be found from the views in this section, mostly on a card (`PRN-04`).
  A new screen is added only when no card, overlay or page of the book of ages can show something well.
  - **Check:** each stage review confirms that every kind of record the world keeps shows on at least one card or view.

### 11.5 Text written for you

- `PRE-37` **Patterns first, the writer polishes** *(Decided)*: Every text is first built from pattern sentences: cards from `MIL-11`, the details view of a mind from `MIL-13`, and the book of ages from `MIL-16`, when the phone's built-in writer AI begins to reword them into flowing prose (`PRE-41`).
  Texts are in English, each name in its people's language with its meaning at first use (`CUL-18`).
  - **Patterns:** a closed catalogue (`MAT-13`) of about 100 kinds of event (the entries of `PRE-39`, the memories of a life story, the story shapes and topics of `CUL-11` and `CUL-24`, and a mind's summary), each with at least 5 phrasings, picked by the event's seed and using all the records hold: names, places, seasons, causes and who was there.
    Labels, card lines (`PRE-45`) and dark events (`PRE-17`) always stay as patterns.
  - **Size:** a text covers at most about 12 events, chosen most important first and told in date order, in at most about 150 words; a long life is told in parts.
  - **Without the writer:** when it is missing, the pattern text is shown and stored, marked as such.
  - **Your choice:** at the game stage (`MIL-16`) you compare pattern and writer text in each view, and keep whichever reads better.
  - **Done when:** in a test world of 100 years every kind of event shows 5 phrasings, each name's first use gives its meaning, and no text passes about 150 words.

- `PRE-41` **How text is written** *(Decided)*: History texts (entries, life stories of the dead and myths) are checked, stored beside their records and never silently rewritten; you can ask for a rewrite.
  Texts about the present, such as a living person's summary (`PRE-14`), are written afresh when opened if their records have changed, and are not kept.
  - **The check,** with no language model: the writer rewords each pattern sentence on its own, in order.
    Each new sentence must keep its pattern's names, numbers, dates and places, in the same order, and add none; keep the words its pattern marks as needed, such as the act and words like *first* or *made*, or a synonym listed for them; and use no word, in any form, beyond its pattern's, those synonyms and a short fixed list of joining words.
    A sentence that fails is shown as its pattern sentence.
  - **When:** texts are worded only while the app is open: the page you open first, then the newest; a page shows its pattern text at once, and each checked sentence replaces its pattern as it arrives.
  - **Done when:** a history text opened twice reads the same, and every failing sentence of `PRE-17`'s trap set shows as its pattern.

- `PRE-17` **Descriptions stick to the data** *(Decided)*: The writer AI gets only a text's pattern sentences (`PRE-37`) and the one voice (`PRE-19`), and chooses words and rhythm, never content (`PRN-06`).
  Images, causes and motives appear only where a pattern sentence holds them, from a myth's story shape (`CUL-11`) or the tellers' beliefs (`MND-27`), such as "the fire that sleeps inside the wood".
  Dark events are those in `CUL-08`'s list, and the writer never receives them: each is its own plain sentence by fixed pattern, in date order among the writer's text, such as "Year 54, spring, day 3: men of the Ketu killed Tor by the river."
  - **Check:** each text's check (`PRE-41`) confirms that every dark event in its records appears as its plain sentence.
    A fixed set of about 50 trap records (two people in opposite roles, a teacher and a learner, a dark event beside a happy one) is written whenever the writer's instructions change and at every stage, and no swapped role, softened fact or missing event may reach the screen.

- `PRE-19` **One storytelling voice** *(Decided)*: Every text uses the documentary voice.
  Myths are told in it as what a people tells, such as "The Tavu tell that Ama took the fire that sleeps inside the wood."
  Other voices may be tried after launch.
  - **Done when:** every pattern sentence and the writer's instructions use the documentary voice.

### 11.6 Content

- `PRE-18` **Content setting** *(Decided)*: You choose how much of history's darker side is shown; the world underneath never changes, and bodies are drawn without sexual detail at every level.
  - **Show:** everything: wound marks and blood on figures and the ground (`PRE-27`), the dead lying where they fell until buried, and screams.
  - **Plain:** no blood or wound marks, the dead lie still without marks, and screams become shouts.
  - **Gentle:** as Plain, and dark events (`CUL-08`) never come to you: no live moment, no slowing by the director (`TIM-02`), and one short line each in the book of ages.
  - **Done when:** a test raid watched at each level looks and sounds as listed, and leaves the same world.

## 12. Sound

Everything you hear comes from something happening in the world (`PRN-10`), in layers added stage by stage (`SCP-16`).

### 12.1 The layers

- `SND-01` **A lively camp** *(Decided)*: A camp sounds alive: murmuring voices, children playing and crying, the tap of stone on stone, scraping and chopping, the fire, dogs, a wolf far off.
  - **How it works:** each sound comes from something under way near the camera: an activity (`SND-06`), talk (`SND-03`), a fire by its heat (`MAT-18`), an animal's call (`MND-16`), or the place (`SND-11`).
    The 32 sounds the phone plays at once are shared: up to 6 for place and weather, 4 for single voices, 8 for music, 2 kept free for a sudden scream, thunder or rockfall, and at least 12 for work, fire, footsteps and animals; when a share is full, its quietest sound joins its blend (`SND-07`).
  - **Done when:** at the game stage (`MIL-16`), 32 sounds at once, with distance, muffling and echo, play on the phone without a break, or the cap and shares are lowered.

- `SND-11` **Ambience** *(Decided)*: Each place has its own background sound from its land, plants and water (`WLD-31`, `WLD-17`), the weather (`WLD-16`), the hour and the season: wind in grass or pines, a river, the sea, rain on leaves, thunder, birdsong at a spring dawn, the hush of snow.
  Birdsong comes from the small birds the place holds (`WLD-32`); the world has no insects or frogs, so nights have no chorus.
  - **Done when:** a test valley sounds different at dawn and at night, in rain and in calm, with birdsong only where small birds live.

- `SND-03` **The murmur** *(Decided)*: People talk in a murmur built from the sounds of their language, never in real words.
  - **How it works:** talk is strung from a bank of spoken syllables in the language's sounds (`CUL-17`), for two base voices, a woman's and a man's, rendered once in the cloud by the natural-sounding voice you chose by ear and shipped with the game, so no voice model runs on the phone.
    It is shifted in pitch and tone for the speaker's age and build (`BIO-08`) and in loudness, speed and tune for their feelings (`MND-19`).
    Laughing, crying, calling, screaming and babies' cries come from a few recordings for each kind of voice (child, woman, man, elder), shifted the same way.
    The voice is chosen by ear at the game stage (`MIL-16`).
  - **Done when:** by ear alone you tell a child, a woman, a man and an elder apart, and anger from grief.

- `SND-02` **Their music** *(Decided)*: Each people's songs, rhythms and instruments (`CUL-10`), heard when you are near.
  - **How it works:** each song plays from its record: its notes and rhythm, in its people's scale.
    Instruments sound by their sound blueprints (`SND-06`): a longer flute is lower, bone brighter than wood, a bigger drum lower and a thicker hide duller; drums, poor in the first trials, are remade before you hear them.
    Voices sing by holding and pitching the murmur's syllables to the notes, with no words (`SND-03`); if that fails your review at the game stage (`MIL-16`), songs are hummed and played, with clapping.
  - **Done when:** at the game stage, songs of two peoples are told apart by ear, and singing and drums pass your review.

### 12.2 How sound is made

- `SND-06` **Sound blueprints** *(Decided)*: Every sound comes from a small base set, picked by what is happening and to what, and changed by the things involved: harder is brighter, heavier and bigger is deeper and longer, wetter (`MAT-19`) is duller, and random variation makes no two the same.
  - **Made by the game** from shaped noise, about 44 base sounds: nine work sounds (striking, chopping, cutting, scraping, grinding, drilling, digging, dropping, stacking), each on hard (stone, bone, metal), woody (wood, plant) or soft things (earth, hide, flesh, water), and a rustle; footsteps on rock, earth, grass, sand, mud, water, snow and ice; fire by its heat; wind, rain, flowing water and thunder; flutes, drums and rattles.
  - **Recordings:** at most about 120, under licences needing at most a credit line (`PRE-40`): about 2 calls for each species that calls, people's sounds other than talk (`SND-03`), and birds for the ambience.
  - **A sound blueprint** is a base sound and how characteristics (`MAT-03`) and size (`MAT-02`) change it; base actions with no work sound borrow one (throwing, feeding and gathering as dropping, planting as digging, soaking as water, heating as fire, the rest as the rustle), and each result's sound (`MAT-21`) is its action's.
  - **The phone's speaker:** a last step lifts the deep sounds it plays badly, and is off with headphones.
  - **Done when:** everything that makes a sound has its sound blueprint, and 20 flint strikes in a row all differ.

- `SND-07` **Sound follows time** *(Decided)*: At natural speed (`TIM-10`) each sound plays as its activity happens; faster, sound follows the picture.
  While figures are drawn in full (`PRE-03`), each one near the camera sounds in step with its animation (`PRE-44`), and talk is murmured at a natural pace; once figures are tiny, sounds of one kind blend by how many and how loud (`SND-01`), so a camp becomes its hum.
  - **Done when:** above real speed at close camp zoom a knapper's taps keep in step with the animation, at valley zoom the camp is one hum, and the globe is near silent.

- `SND-08` **Space and distance** *(Decided)*: Each sound comes from its direction, quieter and duller with distance, muffled by land in between and echoing in caves (`PRE-24`), so a scream or thunder off-screen can draw your eye.
  - **Done when:** on headphones, a scream off-screen to the left comes from the left, and a voice in a cave echoes.

- `SND-12` **Sound review** *(Decided)*: At every stage that adds sound, a tool records on the phone, from fixed saved worlds, one reel of about 3 minutes touring the zooms: the close camp at dusk and at night, a valley in a storm, the globe.
  - **Check:** heard once on the speaker and once on headphones, the reel passes your review: sounds match the screen, voices sound like talk but never like real words, and nothing is harsh.

## 13. Platform and performance

Kindling is built for one phone (`SCP-02`), kept smooth, cool and responsive (`VIS-14`, `PRN-11`); what fits on it is measured.

### 13.1 The phone

- `PLT-01` **One phone** *(Decided)*
  - **What:** Built for your Pixel 11 Pro XL (16 GB of memory, 512 GB of storage), its graphics chip drawing the picture (`PRE-02`) and its built-in AI wording the text (`PRE-37`).
    The simulation runs only on processor cores, so it repeats exactly and no language model touches it (`TIM-16`, `RES-05`, `PRN-06`).
  - **Plans on:** 2 small, 4 middle and 1 fastest core at the speed they hold under full load (held speed), about 3 W, and about 8 GiB of memory; the simulation may use up to the four middle cores, leaving the rest for the picture, sound and the writer, and every time budget in this file is time on one middle core at held speed (`MND-15`, `PLT-04`).
  - **Done when:** every alpha installs and runs on your phone, and at every stage its benchmark worlds end exactly as they do in the cloud (`RES-05`).

- `PLT-02` **Portrait and landscape** *(Decided)*: Turning the phone switches between the two layouts of `PRE-34`.
  - **Done when:** turning the phone on every screen keeps the world, the camera and the art pixel's size (`PRE-22`), with no reload.

- `PLT-03` **Works offline** *(Decided)*: Everything the game needs, the writer AI included (`PRE-37`), is on the phone; nothing in play makes a network call.
  - **Done when:** in flight mode, making a world, an hour's play and book-of-ages text all work.

- `PLT-06` **Installing new versions** *(Decided)*: Each alpha is a file you download and install on the phone, once you allow installs from your browser (`PRC-11`).
  - **How it works:** every build is signed with one key for your hobbyist developer account, as Android requires from 2027, set up once before the first alpha and kept outside the cloud sessions with a copy you hold (`RSK-18`, `RSK-29`).
  - **Done when:** each alpha installs over the last one from the phone's browser, keeping every world.

### 13.2 Performance

- `PLT-04` **Measured limits** *(To test)*
  - **What:** Reported at every stage (`RES-06`) against these targets:
    - **Speed:** the targets of `TIM-07`.
    - **Stage budgets:** at the close of `MIL-12` to `MIL-17`, the 1,000-person world, with what the game has so far, runs at least 2.5, 2, 1.7, 1.5, 1.5 and 1 game years a real minute.
      A stage that misses names its biggest cost and wins it back by the next stage close, or brings it to you.
    - **Shares:** the world's own layers at most about 0.2 s per game day, so the world alone reaches 10 game years a minute on two cores; animals near people within each person's budget (`MND-15`); making areas at most a tenth of the simulation's time.
    - **Smooth:** with the world running, at least 97% of frames on time while zooming, panning and turning, at every zoom, and none more than 50 ms late (`PRN-11`).
    - **Memory:** within about 8 GiB (`PLT-01`), with kept areas at most about 1 GiB in a full pace-test world at Year 250 (`WLD-12`).
    - **Storage:** within the target of `PLT-10`.
    - **Sound:** the 32-sound mix within its limit (`SND-01`).
    - **Battery and heat:** an hour's play uses about 25–30% of the battery, and the phone never gets uncomfortably hot.
    - **Opening and land:** your world opens in about 3 seconds, a new world in `WLD-11`'s times, and a new area within about a tenth of a second.
  - **How it works:**
    - **Benchmark worlds:** a camp of about 30 and a village of about 300 at close camp zoom, a camp in thick forest at camp zoom with the camera turning, and worlds of about 100, 500, 1,000, 2,000, 3,000 and 7,000 people with about 10 km² each, on generated land; from `MIL-11` also the world with nobody in it.
      Each runs to a set game date, its end state in the result code (`RES-05`).
    - **Old worlds:** from `MIL-15`, the pace-test worlds at Year 75, and from `MIL-17` the full test's at Year 250, each against a new world with as many people (`TIM-07`), their kept areas read against the memory line.
    - **Held speed:** speeds are read after at least 3 minutes at full load, unplugged (`PLT-01`), battery and heat from the phone's counters.
    - **When:** in the cloud at every alpha (`PLT-05`), and on the phone at every stage: one tap, about 20 minutes, and a short result code to send back.
  - **Done when:** at every stage, the phone benchmark's result shows each target above, met or missed.

### 13.3 Saved worlds

- `PLT-07` **Always saved** *(Decided)*: Worlds save continuously, so closing the app or a flat battery never loses anything (`TIM-05`).
  - **How it works:** each event joins the world's history as it happens.
    The present state is saved every 30 real seconds and whenever the app leaves the screen; a damaged save is never loaded.
    After a crash, the world opens at its last save within about 3 seconds and catches up, under a short note, repeating exactly (`TIM-16`).
  - **Done when:** killing the app 100 times at random never loses an event or leaves a world that won't open.

- `PLT-08` **Manual export** *(Decided)*: Export a world with all it keeps (`TIM-08`) as a file at any time, and import it on this phone or a new one; there are no automatic backups.
  - **Done when:** an exported world, imported, runs on exactly as it would have (`TIM-16`), and a damaged file is refused with a message.

- `PLT-09` **Worlds across updates** *(Decided)*
  - **What:** After a small update, such as fixes, new blueprints or tuning that leaves the land alone, a world carries on under the new rules, the change marked in its book of ages.
    A big update, changing how worlds or areas are made, the values that place and size land, plants and stones, or the kinds of plants, animals or materials, may need a new world; the old one's book of ages can still be read.
    Each update says if it is small or big.
    A new version keeps a copy of each world's last save until the world has run an hour under it.
  - **Done when:** worlds saved by the previous alpha open and carry on after a small update, and after a big one their books of ages open.

- `PLT-10` **Storage** *(Decided)*: Each world keeps its present state, kept areas and history (`PRN-15`); unchanged areas are remade from the seed (`WLD-13`).
  History thins with age by a fixed rule: the last 25 years keep every event, and older years keep what the book of ages and views use, such as births, deaths, firsts and events art shows (`PRE-15`).
  Target: a full pace-test world at Year 250 fits in about 4 GB, history and kept areas together (measured in `PLT-04`).
  When the phone is nearly full, the game warns you and asks which worlds to delete; it never deletes anything by itself.
  - **Done when:** every full pace-test world, saved at Year 250, fits its target, and the warning comes before the phone is full.

### 13.4 The cloud

- `PLT-05` **Tests in the cloud** *(Decided)*: The game also runs without picture or sound in the AI's cloud sessions, several worlds at a time, for the tests, ending exactly as on the phone (`SCP-15`, `RES-05`, `RES-21`, `RES-07`).
  - **What:** a test's world opens on the phone as it ends, marked as a test world with any switch it used (`RES-10`); you can ask for any test or run and get its report as a page (`RES-06`).
  - **How it works:** a run keeps checkpoints as it goes, so one cut off by a restart, or spread over several nights, resumes from its last checkpoint and ends as if it had never stopped (`TIM-16`); where checkpoints and results are kept is the architecture's.
    The night's runs start by themselves on the main version as it stands, and the next session reads their results first (`PRC-10`).
  - **Done when:** a pace run stopped with its session and resumed in a new one ends identical to an unbroken run, and opens on the phone as a test world.

## 14. Testing

How each alpha proves what it adds, within the AI's cloud sessions (`SCP-15`).

### 14.1 How testing works

- `RES-01` **Tests lead** *(Decided)*: Every alpha brings automated tests for what it adds, quick ones run before any work joins and long ones in the background of the session that builds (`PRC-10`).
  A feature counts as built only when its tests pass, and every test names the IDs it checks.
  - **Check:** the coverage check finds a test for every feature and rule built so far (`PRC-12`).

- `RES-21` **Scenes, then whole worlds** *(Decided)*
  - **What:** Most tests run in sandbox scenes, small settings built for one check, on land cut from a generated world, about 10 km across for one band and as wide as its setting needs for more.
    Each holds decoys and things nobody designed for, and checks that nobody used an unknown blueprint or an unseen fact (`PRN-01`).
    Whole worlds (`RES-07`) confirm that what scenes show also happens in play.
  - **Done when:** each stage report names, for every result, its scenes and its whole-world confirmation (`RES-06`).

- `RES-18` **Same rules as play** *(Decided)*: Scenes use the same rules, minds and catalogues as play, and nothing in a scene is scripted.
  A result that needs a switch (`RES-10`) or a scripted event doesn't count as passing in play (`PRN-12`).
  - **Check:** scenes and play run on one game, and every run records any switch it used.

- `RES-09` **Pass rules come first** *(Decided)*: Every test states, before it first runs, what it checks (by ID), its scene or worlds, its runs, its budget in session-hours, and its pass rule in exact numbers.
  A pass rule is never loosened in the change that makes it pass.
  Loosening one you approved needs your OK; loosening any other needs a stated reason and the independent reviewer's OK (`PRC-09`); the pace windows change only with you (`TIM-19`).
  - **Check:** the review sees every change to a pass rule, with its reason.

- `RES-13` **About 20 runs where chance matters** *(Decided)*: Where chance decides the outcome, a check runs 20 times from different seeds, and its pass rule counts runs, such as "in at least 16 of 20".
  A failed check reruns on 20 fresh seeds before it blocks anything, its rule applied to all 40, such as at least 32 of 40.
  Fewer runs, down to 10, scale the rule, rounded against passing, and mark the result provisional, which counts but is named in the stage report (`RES-06`); more runs only where 20 can't tell pass from fail.
  The full pace test uses 10 worlds by design and is not provisional; if it fails, 10 more run and its rule is applied to all 20 (at least 10 inside each window, at most 5 before it).
  Shares an item promises, other than a blueprint's chance (`RES-24`), are checked over at least 1,000 cases: chances of 1 in 20 or more to within a third either way, and rarer ones to within half to double.
  - **Check:** every result in a report states its runs and its range, such as "fire in 18 of 20 worlds, around Year 15".

- `RES-05` **Repeatable runs** *(Decided)*: On the same build, a run from the same saved world gives the same result every time, on the phone and in the cloud alike, so any failure can be replayed step by step (`TIM-16`, `PLT-05`).
  - **Check:** the repeat check before any work joins (`PRC-10`), and at every stage the benchmark worlds (`PLT-04`) end exactly the same on the phone and in the cloud.
    A difference blocks the stage until it is found; settling for matching ranges needs your OK.

- `RES-10` **Switch-off runs** *(Decided)*: To find out what a result depends on, a scene can run with one thing switched off, such as teaching, dreams or a personality trait.
  Switches exist only in tests (`PRN-12`).
  - **Done when:** every run using a switch says so in its report and its world (`PLT-05`), and the play build has no switches.

- `RES-16` **Tuning the pace** *(Decided)*
  - **What:** The pace is tuned only by changing chances and amounts (`PRN-17`, `MND-11`), the same in every world.
    Late steps are tuned in scenes set where the step becomes possible; whole worlds only confirm.
    Tuning uses 20 fixed seeds, shared with the long runs; each closing pace test, every time it runs, draws new seeds never tuned against.
    Every tuned value, discovery factors included (`MND-11`), is logged with what it was tuned against; if tuning can't fix the pace, the stage report gives you the options: a redesign, another window, or accepting it.
  - **Done when:** the tuning log lists every tuned value, and no closing test's seeds appear in it.

### 14.2 The tests

- `RES-24` **Blueprint trials** *(Decided)*: Each blueprint is tried directly about 200 times at a low level and 200 at a high level, with its inputs in place and no mind choosing.
  Its results, times, failures and leftovers must match its fields (`MAT-04`), and its successes must fall where 200 tries of that chance land 99 times in 100, such as 100–140 for a 60% chance and 3–19 for a 5% one.
  Trials test the catalogue, not behaviour, never count as passing in play (`RES-18`), and run before any work joins (`PRC-10`).
  - **Done when:** every blueprint in the catalogue has its trials, and one given a wrong chance or result fails them.

- `RES-23` **Every chain and behaviour has a scene** *(Decided)*: Each chain of the arc (`MAT-22`) and each everyday behaviour, such as a band fleeing a predator, has a scene whose setting gives people their own reason.
  A chain's scene passes if the chain completes in at least half of 20 runs within a stated time.
  - **Done when:** the coverage check finds a trial for every blueprint and a scene for every chain and behaviour (`PRC-12`).

- `RES-02` **The sharp-stone test** *(Decided)*: A band with the starting kit (`BIO-02`), which has never made a sharp flake, lives by a river with stone anvils, nuts to crack, carcasses to butcher, hides and wood to work, and flint among decoy stones such as granite.
  - **How it works:** it runs 20 times, each until a year after the first flake, or 3 years if none comes; a control scene, the same with no stone that flakes, runs 4 years.
    The first flake's year and route come from the book of ages (`MAT-21`), and who can make flakes from each adult's skills (`MND-06`).
  - **Done when:** the scene and its control run in the cloud at every stage from `MIL-14`, judged by `RES-03`.

- `RES-03` **Sharp-stone pass rule** *(Decided)*
  - **Discovery:** flakes are discovered within 2 years in at least 16 of 20 runs.
  - **Spread:** in those runs, at least 3 in 4 of the band's adults can make flakes within a year of the first.
  - **Routes:** at least two routes of discovery appear across the runs (`MND-11`).
  - **Control:** without stone that flakes, no run ever makes a flake (`RCK-01`).
  - **Check:** the sharp-stone test (`RES-02`) passes at every stage from `MIL-14` (`PRC-10`).

- `RES-07` **The pace tests** *(Decided)*: Whole worlds from the play generator (`WLD-10`) check each pace target (`TIM-19`) and culture target (`CUL-33`) in their records.
  - **Sizes,** each judging the targets whose windows close within its years: after any change to minds, blueprints or catalogues, 20 worlds to Year 30, and at each other stage close, the same on new seeds (`RES-16`); at the close of `MIL-15`, 20 worlds to Year 75; and the full test, 10 worlds to Year 250, before `MIL-17` closes and at most once a week of building during it.
  - **Stages:** each target counts from the stage that builds it: flakes, fire, clothing, huts and `RES-25` `MIL-14`; shared spirits, rites, myths, dogs, band splits, festivals, feuds, new peoples, raids and Keeps going `MIL-15`; pottery, herding, villages, farming, copper and chiefs `MIL-17` (`SCP-16`, `TIM-19`, `CUL-33`).
  - **Computing:** in the session that builds, in the background while it works, never in another session (`SCP-15`): the size after a change takes about 1 session-hour and the Year-75 run about 3, and the full test about 10–25, spread over the last alphas of `MIL-17`, whose stage closes once it has run; each stage report sets the real cost against these (`RES-06`).
  - **Other whole-world checks** read these worlds, at the first size that reaches their years (`BIO-04`, `BIO-06`, `BIO-22`, `MAT-08`, `PRE-39` and the culture items); checks of the world with nobody in it, such as `WLD-15` and `WLD-18`, run in the background at the stage that builds them, and again only when their rules change.
  - **Check,** for each target: at least half the worlds reach it inside its window, and at most a quarter before it opens (with 10 worlds, at least 5 and at most 2); where windows overlap, the steps don't come in the same order in every world.

- `RES-25` **Something to watch** *(To test)*: From Year 5 on, whole worlds average at least one new entry in the book of ages per year, and no 5 years pass without an event important enough for a live moment (`PRE-08`), in at least 16 of 20 worlds.
  - **Check:** the pace tests read it (`RES-07`), and stage reports chart entries and live moments per year (`RES-06`).

- `RES-17` **Signature moments keep happening** *(Decided)*: Each signature moment (`MOM`) has its own scene, run from the stage and within the window its Check line gives, passing if it happens in at least 2 of 20 runs unless its own rule says otherwise.
  It reruns whenever what it depends on changes, before work joins if it lasts up to 10 years (`PRC-10`), otherwise in the background before its stage closes (`RES-07`).
  - **Check:** each stage report gives every moment's latest result and its appearances in whole worlds (`RES-06`).

- `RES-19` **Every promise has a test** *(Decided)*: Everything this file says will arise in play, such as a religion or a lost craft, gets a scene or whole-world check by the stage that builds it, or is marked "possible, not promised" with your OK (`PRC-07`).
  - **Check:** the coverage check lists each such promise with its test or its mark (`PRC-12`).

- `RES-14` **Believable outcomes** *(Decided)*: Whole worlds are checked against the ranges of `BIO-04` (births, deaths, life spans, growth) and `CUL-30` (band sizes); a world far outside them is a bug to look into, not a finding (`PRN-02`).
  - **Check:** stage reports show these from the pace-test worlds against their ranges (`RES-06`).

- `RES-12` **Oddities are flagged** *(Decided)*: Whole-world runs flag oddities by expected ranges and "never" rules, such as a step far outside its window, a pot before any fire, starving beside plenty, one action repeated endlessly, or a thing from nothing (`MAT-09`).
  They also flag a crash, memory creeping up or a save that won't reopen; each bug is fixed with a test that would catch it again.
  - **Done when:** a scene with one of each kind of oddity planted has every one flagged (`RES-06`).

### 14.3 Your reviews and reports

- `RES-22` **Your reviews** *(Decided)*: At each stage, you play the latest alpha and judge what tests can't: whether it looks right (`PRE-31`) and sounds right (`SND-12`), feels lively and believable, keeps a watchable pace, and has a book of ages worth reading (`VIS-15`).
  You answer the report's short list of what to try in a few lines; work goes on meanwhile, and the stage closes once you have (`PRC-10`).
  - **Done when:** each closed stage records your review.

- `RES-06` **Stage reports** *(Decided)*: Every stage ends with a short report for you, covering:
  - what was added, and what you can now see and try;
  - the test results with charts, above all the pace (`RES-07`) and something to watch (`RES-25`), and where tests ran and the computing used (`SCP-15`);
  - the phone measurements (`PLT-04`);
  - the moments seen, the oddities and the surprises (`RES-17`, `RES-12`), and how each principle's check came out (`PRN-16`);
  - the risks, with content made against what later stages need (`RSK-25`);
  - what needs your judgement (`RES-22`).
  - **How it works:** an independent AI reviewer checks it (`PRC-09`), and it is published as a short page that opens on the phone, its links opening the worlds and entries it names, with a copy in the repository.
  - **Done when:** every closed stage has its report, with each part above, opening on the phone with working links.

## 15. Project and process

How the project is run.

### 15.1 Roles

- `PRC-01` **Passion project, built by AI** *(Decided)*: You direct; AI agents write, test and review the code, with no running costs beyond the AI sessions.

- `PRC-02` **Your role** *(Decided)*: You play the alphas when you like, review each stage (`RES-22`), set direction and approve changes to this file.
  - **Check:** every change to this file names your OK in its commit (`PRC-07`).

- `PRC-03` **Technology** *(Decided)*: Chosen by the AI from what the pre-tests measured (`PRC-08`), set out in the architecture for your approval.
  - **Check:** the architecture's technology proposal records your approval before building starts.

### 15.2 Documents

- `PRC-04` **Three documents** *(Decided)*: This file says what to build; the architecture says how and why, citing the research notes behind each choice (`research/`); and the implementation plan says in what order, mapping every item to a milestone and planning the next milestone's alphas and tasks.
  Code and tests name the items they implement and check, so the code is the index of where each is done (`PRC-12`), and no document keeps that list.
  - **Check:** each stage review checks that the three documents exist and this file holds no implementation details.

- `PRC-06` **A guide for AI agents** *(Decided)*: A short file in the repository (`CLAUDE.md`) that every AI agent reads first.
  It tells them to read this file, follow the principles, link all work to IDs, and never mark anything Decided without you; changes to it need your OK.
  - **Done when:** the guide sits at the top of the repository and says each of these.

- `PRC-07` **Changes to this file** *(Decided)*: AI agents can suggest additions or changes, marked *Proposed*, as How this file works sets out.
  Nothing becomes *Decided*, and no decided item changes, without your OK.
  - **Check:** the commit check confirms that every commit changing this file names the changed IDs and why, and your OK for anything made Decided.

- `PRC-08` **Next: pre-production, then production** *(Decided)*: The research is done (`research/`), the art book fixes the look (`art/book/`), the bake-off chose Godot with the simulation in C++ (`PRC-03`), and the architecture and the plan are written from them.
  Pre-production (`MIL-18`) comes next: it removes the old code, which git keeps, proves each risk with a prototype, and builds the vertical slice; production starts with the foundations (`MIL-08`) once you accept the slice.
  A few questions stay open, each carried by its item: the fix for crawling pixels, chosen with the look prototype (`PRE-22`); the voice (`SND-03`) and the drums (`SND-02`), chosen by ear at `MIL-16`; and signing for your hobbyist account (`PLT-06`).

### 15.3 How work flows

- `PRC-09` **Branches, checks and review** *(Decided)*: AI agents work on separate branches, and work joins the main version only after every automatic check passes (`PRC-10`) and a review approves it.
  An alpha that is split into lettered steps is reviewed by its builder, step by step; an independent AI review verifies the whole alpha once, at its last step.
  - **How it works:** at each step the builder reviews its own change against the items it claims, re-running the checks and making each new test fail once.
    At the alpha's last step, a fresh, separate agent reviews the whole alpha, its tests and results, and the items it claims, without the builder's reasoning, against those items' What, Done when and Check lines, and checks that no test was weakened to pass (`RES-09`).
    If builder and reviewer still disagree after one round of fixes, a second fresh reviewer decides; anything that changes what this file means goes to you.
  - **Check:** nothing joins the main version with a failing check or without its review's approval.

- `PRC-10` **The checks** *(Decided)*
  - **Before any work joins the main version,** within about 20 minutes on one cloud machine, anything longer running in the background of the same session:
    - the quick tests (`RES-01`), the blueprint trials (`RES-24`) and the scenes of up to 10 years, the signature moments' included (`RES-23`, `RES-17`);
    - the catalogue checks, reality rules included (`MAT-17`, `RCK`);
    - the repeat check: one scene and one benchmark world each run twice, once on one core and once on four with a stop and resume between, and must end identical (`RES-05`);
    - the file check: every ID defined once, every reference resolving, every status valid, and no retired ID used or cited;
    - the commit check (`PRC-07`) and the coverage check (`PRC-12`).
  - **If they outgrow 20 minutes,** scenes of items the change doesn't touch move to the background first; the trials, the scenes of the items it touches, and the catalogue, repeat, file, commit and coverage checks always run before joining.
  - **When a background run ends:** its results are read before anything else.
    A check that passed before and now fails is fixed, or the change behind it undone, before other work joins.
    A pace target not yet met goes to tuning (`RES-16`) and blocks only the stage close.
  - **Before a stage closes:** the pace tests (`RES-07`), the phone measurements (`PLT-04`), the phone and cloud match (`RES-05`), the moment scenes that are due (`RES-17`), the writer's trap records (`PRE-17`), the visual and sound reviews (`PRE-31`, `SND-12`), the report (`RES-06`) and your review (`RES-22`).
  - **Check:** the checks run by themselves, and any failure blocks the join or the close.

- `PRC-11` **Each alpha reaches your phone** *(Decided)*: Every alpha (`SCP-03`) ends with a build to install and try on the phone (`PLT-06`) and a short note: what is new, what to try and what is still rough.
  Only the stage reviews wait for you (`RES-22`).
  - **How it works:** the note is a page with the download link, and the build opens your worlds unless the note says it is a big update (`PLT-09`).
    The first time an alpha opens, it runs a self-check of a few seconds and, if anything fails, shows a short code to send back.
  - **Check:** every alpha's note links its build and names the IDs it delivers, and no check that passed before was failing on its build.

- `PRC-12` **Nothing gets lost** *(Decided)*: An automatic coverage check, reading only IDs, runs before any work joins and at each stage close (`PRC-10`), and confirms that:
  - every feature and rule that isn't *Proposed* is built already or mapped to a stage in the implementation plan, and the current stage's items have tasks;
  - every task names the IDs it delivers, and every ID named in code and tests exists and isn't retired;
  - every feature and rule built so far has a test (`RES-01`), every blueprint a trial (`RES-24`), every chain a scene (`RES-23`), and every promise a test or a "possible, not promised" mark (`RES-19`);
  - work linked to an item changed in this file is flagged for re-checking.
  - **Done when:** a plan with one unmapped feature, a test naming a retired ID and a blueprint without a trial fails the check on all three.

## 16. Risks

Stage reports review these risks; AI agents may update the ratings there (`RES-06`).

### 16.1 The game itself

- `RSK-01` **Discoveries stall** *(Decided)*: Likelihood medium, impact high.
  - **Signs:** the sharp-stone test failing (`RES-03`); pace tests stuck before fire (`RES-07`).
  - **Response:** switch-off runs (`RES-10`); tuning, never scripting (`RES-16`).

- `RSK-26` **The pace is off** *(Decided)*: Likelihood high, impact high.
  - **Signs:** pace tests off their windows, or every world in one order (`RES-07`).
  - **Response:** tuning in scenes (`RES-16`); several routes to a result (`MAT-07`); windows changed with you (`TIM-19`).

- `RSK-19` **Belief fails to emerge** *(Decided)*: Likelihood medium, impact high.
  - **Signs:** culture targets missed (`CUL-33`); no religion (`CUL-26`).
  - **Response:** one capped belief rule (`MND-05`) and templates (`CUL-05`); belief scenes from `MIL-13` (`RES-19`).

- `RSK-06` **Blueprints give absurd results** *(Decided)*: Likelihood medium, impact medium.
  - **Signs:** catalogue checks or trials failing (`MAT-17`, `RES-24`); oddities (`RES-12`).
  - **Response:** reality rules and the expected-fits check (`RCK`, `MAT-17`).

- `RSK-07` **People know what they can't** *(Decided)*: Likelihood medium, impact high.
  - **Signs:** choices using unknown blueprints or unseen places (`RES-21`); texts with facts the records lack.
  - **Response:** `PRN-01` and `PRN-06`, checked in scenes and by the writer's check (`PRE-41`).

- `RSK-27` **People act oddly** *(Decided)*: Likelihood high, impact medium.
  - **Signs:** dithering, starving beside food or all doing one thing (`RES-12`).
  - **Response:** everyday scenes from `MIL-12` (`RES-23`); reasons kept for what each is doing and every saved event (`MND-09`).

### 16.2 The experience

- `RSK-03` **Real but dull to watch** *(Decided)*: Likelihood medium, impact high.
  - **Signs:** worlds falling short of `RES-25`; you skimming the book of ages.
  - **Response:** the story director, live moments and following someone (`TIM-02`, `PRE-08`, `PRE-06`).

- `RSK-08` **The writer falls short** *(Decided)*: Flat or softened text, or changed or removed by a phone update; likelihood high, impact medium.
  - **Signs:** entries alike or vague; texts changing after a phone update.
  - **Response:** pattern sentences, which the writer only rewords, state dark events plainly and show if it is missing (`PRE-37`, `PRE-17`).

- `RSK-11` **Pixel look hard to keep clean** *(Decided)*: Likelihood medium, impact medium.
  - **Signs:** visual reviews failing on crawling pixels or unreadable figures (`PRE-31`).
  - **Response:** the fix chosen at the first visual review (`PRE-22`).

- `RSK-28` **Sound falls flat** *(Decided)*: Likelihood medium, impact medium.
  - **Signs:** a camp sounding thin or fake in your reviews (`SND-12`).
  - **Response:** deep sounds lifted for the speaker, sounds tuned with you (`SND-06`); the voice chosen by ear (`SND-03`).

### 16.3 The phone

- `RSK-02` **Too slow at 2,000 people** *(Decided)*: Likelihood medium, impact high.
  - **Signs:** a stage missing its budget or making areas over its share (`PLT-04`); 2,000 people before Year 150 (`BIO-04`).
  - **Response:** past about 2,000, time slows and the game says so (`MND-15`).

- `RSK-15` **Worlds outgrow the phone** *(Decided)*: Likelihood medium, impact medium.
  - **Signs:** memory, kept areas or saves past their targets (`PLT-04`, `PLT-10`).
  - **Response:** kept areas hold only fading changes (`WLD-12`); history thins (`PLT-10`); worlds pause at the memory limit (`MND-15`).

- `RSK-04` **Runs stop repeating** *(Decided)*: Likelihood high, impact high.
  - **Signs:** repeat or phone-and-cloud checks failing (`PRC-10`, `RES-05`); a crash replaying differently.
  - **Response:** the simulation only on the processor cores, by the architecture's rules (`PLT-01`, `PRC-08`).

- `RSK-21` **Updates harm worlds** *(Decided)*: Likelihood medium, impact high.
  - **Signs:** a world failing to open, or jumping in state, after an update.
  - **Response:** a safety copy, the change marked, and tests opening older saves (`PLT-09`, `PRC-10`).

- `RSK-29` **The signing key is lost** *(Decided)*: Likelihood low, impact high.
  - **Signs:** a build that won't install over the last one; removing the app deletes its worlds.
  - **Response:** one key, two copies (`PLT-06`); worlds exported before any reinstall (`PLT-08`).

- `RSK-18` **New install rules** *(Decided)*: Likelihood medium, impact low.
  - **Signs:** from 2027, installs blocked or warned against (`PLT-06`).
  - **Response:** signing for your hobbyist account (`PLT-06`); the advanced unlock or a USB cable as fallbacks.

- `RSK-24` **The phone ages or is replaced** *(Decided)*: Likelihood low, impact medium.
  - **Signs:** the battery over about 40 °C in long sessions, or its health falling.
  - **Response:** time slows rather than the phone overheating (`PRN-11`, `PLT-04`); worlds move by export (`PLT-08`).

### 16.4 The project

- `RSK-25` **Too much content** *(Decided)*: The launch content takes too long to make and check: items, blueprints, plants, animals and illnesses (`MAT-13`), signs (`PRE-46`), movements and dance moves (`PRE-44`), kinds of thought (`MND-29`), pattern sentences (`PRE-37`) and base sounds (`SND-06`).
  Likelihood high, impact high.
  - **Signs:** content made per alpha short of what later stages need (`RES-06`).
  - **Response:** stages add only what their steps need (`SCP-16`).
    A fallback launch set is agreed now and marked in the catalogues: about two thirds of each count (about 125 items, 95 blueprints, 40 plants, 20 wild animals and 10 illnesses), keeping every result another item names, a full chain to every step of the arc and, for each kind of land, a grazer, a hunter, a food plant and a fibre plant, and passing every catalogue check on its own (`MAT-17`).

- `RSK-05` **The scope never ends** *(Decided)*: Likelihood high, impact medium.
  - **Signs:** stages slipping; alphas adding little you can see.
  - **Response:** short alphas in stages with fixed goals (`PRN-09`, `SCP-16`); the arc ends at copper (`VIS-03`); cuts stay cut (`SCP-21`).

- `RSK-09` **AI-built code drifts** *(Decided)*: Likelihood medium, impact high.
  - **Signs:** coverage gaps; behaviour contradicting this file; tests quietly weakened.
  - **Response:** the guide, coverage check, firm pass rules, review and modules (`PRC-06`, `PRC-12`, `RES-09`, `PRC-09`, `PRN-14`).

- `RSK-14` **Tests too big for the cloud** *(Decided)*: Likelihood medium, impact medium.
  - **Signs:** long runs unfinished or lost; a pace test over its stated budget (`RES-07`).
  - **Response:** scenes wherever they can answer (`RES-21`, `RES-16`); runs that resume (`PLT-05`).

- `RSK-23` **Your time** *(Decided)*: Likelihood medium, impact medium.
  - **Signs:** stages waiting on your reviews; alphas piling up untried.
  - **Response:** only stage reviews wait for you (`PRC-11`), with a short list and report (`RES-22`, `RES-06`).

## 17. Not yet decided

### 17.1 Settled by measurement

<!-- generated: open items -->
- **Speed target** (`TIM-07`): game years per real minute by number of people; measured on the phone.
- **Pace of discovery** (`TIM-19`): settled by measurement during development.
- **How many people the world can feed** (`WLD-04`): measured in a scene of the land's numbers.
- **Generation time** (`WLD-11`): how long a new world takes to make; measured on the phone.
- **Population limit** (`MND-15`): settled by measurement during development.
- **Pace of culture** (`CUL-33`): settled by measurement during development.
- **The phone's limits** (`PLT-04`): measured from the first build.
- **Something to watch** (`RES-25`): how often worlds bring something new; measured in the pace tests.
<!-- end generated -->

### 17.2 Proposals awaiting confirmation

AI agents' suggestions are marked *Proposed* and listed here until you confirm, change or drop them (`PRC-07`).

<!-- generated: proposals -->
- **Pace of culture** (`CUL-33`): a window for the first custom, within a year, as P10 found.
<!-- end generated -->

## 18. Glossary

- **Activity:** anything a person or animal does, from start to end, with results at the end (`TIM-17`).
- **Alpha:** one step of the build you can try on your phone, a few hours of AI work (`SCP-03`).
- **Area:** a patch about 256 m across, detailed to about a metre, made only where needed and as the seed gives it while unchanged (`WLD-12`, `WLD-13`).
- **Art pixel:** one pixel of the low-resolution picture, enlarged on screen (`PRE-22`).
- **Band:** a small group of a few families, linked by kin and marriage, who live and move together (`CUL-30`).
- **Base action:** one of the 21 actions people do to things, such as strike, cut or heat (`MAT-06`).
- **Belief:** something a person holds true, such as a cause or a spirit, rightly or not (`MND-27`).
- **Belief template:** the shape a belief born of an event takes: a spirit, the dead, a taboo, a rite or an offering (`CUL-05`).
- **Blueprint:** a hidden rule, known only once discovered or learned, by which one base action on things in set ranges and conditions gives a named result (`MAT-04`).
- **Book of ages:** a world's chronicle of firsts, births, deaths, feuds and disasters (`PRE-05`).
- **Catalogue:** one of the game's content lists, such as items, blueprints, illnesses, thoughts or pattern sentences, written by AI agents and checked by tests (`MAT-13`).
- **Characteristic:** one of the 18 values, 0–5, of every item, such as hardness, edge or warmth (`MAT-03`).
- **Close camp:** the zoom between person and camp, where every figure is drawn in full (`PRE-03`).
- **Count:** animals kept as a number per world cell, such as a herd far from people or hares anywhere (`WLD-32`).
- **Custom:** what most of a group has done the same way for years, named and taught (`CUL-06`).
- **Details view:** the view into one mind, with the reasons behind each choice (`PRE-14`).
- **Discovery:** learning a blueprint by accident, experiment, a dream's hint or copying, a people's first being a named discovery (`MND-11`, `MAT-21`).
- **Domestic kind:** a line of animals kept for generations and born tame, such as the dog (`WLD-33`).
- **Dream:** what you send a sleeper: a place, animal, person, fear or blueprint's hint, or for an animal a pull or a change of mood (`GOD-03`, `GOD-12`).
- **Experience:** how practised someone is in one of the 15 sectors (`MND-06`).
- **Festival:** a gathering at one place and season, kept each year at its sign (`CUL-29`).
- **Game year:** 60 days in four seasons of 15, with spans over about two weeks shrunk to about a sixth (`TIM-18`).
- **Gathering:** bands camped close together for a day or more (`CUL-29`).
- **Heat level:** how hot a fire is, from 0, out, to 5, a furnace with air blown in (`MAT-18`).
- **Held speed:** the speed the phone keeps after a few minutes at full load, at which every speed target is read (`PLT-01`, `PLT-04`).
- **Hunch:** a guess that an action on some things might give a result (`MND-11`).
- **Item:** a kind of thing in the catalogue, such as flint or a sewn cloak (`MAT-10`).
- **Kept area:** an area people changed, keeping only their fading changes (`WLD-12`).
- **Level:** how able someone is at a blueprint, from skill and experience, setting its chance of success (`MAT-04`).
- **Live moment:** a notable event the game shows you as it happens (`PRE-08`).
- **Material class:** one of nine kinds of material, such as stone, wood or hide (`MAT-01`).
- **Mental map:** what a person knows of places: food, water, stone, shelter and danger, by season (`MND-28`).
- **Milestone:** see Stage.
- **Mood:** how a person feels overall, from needs and recent thoughts (`MND-29`).
- **Motif:** the side view of a kit model, or one of about 12 signs (`PRE-46`).
- **Murmur:** speech in the game, a babble of the language's sounds, never real words (`SND-03`).
- **Named result:** what a blueprint gives: a new thing, a new state of a thing, or a change to ground or a body (`MAT-04`).
- **Need:** hunger, thirst, warmth and rest for the body (`BIO-09`), and safety, belonging, status, curiosity and love for the mind (`MND-07`).
- **Pace target:** the window of years in which typical worlds reach a step, such as fire in Years 2–8 (`TIM-19`).
- **Pace test:** whole worlds run in the background against the pace and culture targets (`RES-07`).
- **Pattern sentence:** a set sentence filled from a world's records, which every text starts from (`PRE-37`).
- **People (a people):** a named group with its own territory, customs, beliefs and style (`CUL-23`).
- **Plain use:** a base action done without a blueprint, such as digging a grave, which makes nothing new but heaps, holes and cleared ground (`MAT-06`).
- **Power:** one of your ways to act: weather, dreams, animal dreams, fortune or Revelation (`GOD-05`, `GOD-13`).
- **Revelation:** your one power beyond natural means: a person, or everyone, comes to know a blueprint you pick, at the lowest skill (`GOD-13`).
- **Quality:** how well a thing is made, 0–5, from its maker's skill and inputs (`MAT-20`).
- **Reality rule:** a rule the blueprint catalogue must obey, such as "flint flakes and granite doesn't" (`RCK`).
- **Region:** land about 100 km across, a zoom stop, as in the start region (`PRE-03`, `WLD-24`).
- **Rite:** a shared act, such as a song, a dance or a meal, kept at set times or places (`CUL-34`).
- **Scene:** a small setting built for one test, with nothing scripted (`RES-21`).
- **Season:** 15 days of spring, summer, autumn or winter (`TIM-18`).
- **Sector:** one of the 15 fields of experience, such as stone, fire or herding (`MND-06`).
- **Seed:** the number a world is made from, which always gives the same unchanged land (`WLD-13`).
- **Signature moment:** a story the game must be able to produce unscripted (`MOM`).
- **Skill:** how good someone is at one blueprint they know (`MND-06`).
- **Stage:** pre-production or one of the ten production milestones, `MIL-18` and `MIL-08` to `MIL-17`, each a group of alphas ending in your review (`RES-22`).
- **State:** a condition of a thing, such as dried or rotten, that shifts some values without making a new item (`MAT-19`).
- **Story director:** what sets the speed of time by what is happening, never causing events (`TIM-02`, `TIM-03`).
- **Style:** a people's own way of shaping and decorating things (`CUL-12`).
- **Taming:** animals fed and kept near people, or raised by them, growing tame (`WLD-33`, `RCK-24`).
- **Thought:** a reaction to an event that lifts or lowers mood for a while (`MND-29`).
- **Timer:** a slow change, such as rotting or firing, sped up or slowed by conditions (`MAT-19`).
- **Top speed:** as fast as the phone can run the world with the screen smooth (`TIM-01`).
- **Topic:** what talk is about, from a closed list, such as news or a request (`CUL-24`).
- **Trial:** a blueprint tried directly, many times, with no mind choosing (`RES-24`).
- **Village:** a band's year-round home of lasting houses it built (`CUL-28`).
- **Wear:** how used a thing is, from new to broken (`MAT-20`).
- **Weather cell:** a patch of sky about 10 km across (`WLD-16`).
- **World:** one generated planet, about 2,000 by 1,000 km, wrapping both ways (`WLD-03`).
- **World cell:** a square of land about 1 km across, always simulated at a coarse pace (`WLD-12`).
- **Writer AI:** the phone's built-in language model, which only rewords pattern sentences and never writes dark events (`PRE-37`, `PRE-17`).
