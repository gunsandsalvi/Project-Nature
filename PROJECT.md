# Kindling

The binding game contract: features, rules, targets and permanent IDs.
Read the vision, principles and current milestones for direction; consult system sections when building them.
Keep one statement per obligation; shortening or retiring scope needs owner approval.
Implementation belongs in ARCHITECTURE.md and IMPLEMENTATION.md.

## Working brief

Kindling is an offline world you watch and influence indirectly. People choose from their own needs and knowledge; discovery, learning and history arise from actual events. Pictures and generated words describe the world, never decide it.

The current commitment is a small valley game. Keep the accepted foundations; build a living camp and one place dream, then prove stone/fire discovery and learning. Add seasons, a neighbour and a small finish. Readable stand-ins are accepted for these early builds. The wider world, full catalogues, cultures and copper are optional production expansions whose existing requirements remain open until delivered or explicitly retired.

The owner approved the revised order and 22 accompanying decisions on 8 October 2026. The separate testing-policy patch is still a recommendation; current testing rules apply.
Use IMPLEMENTATION.md for the next task and ARCHITECTURE.md for contracts and known faults. This file is the full decision reference. Only the owner can change decided meaning; the next sections explain its IDs and acceptance rules.

## Contents

<!-- generated: contents -->
- [Working brief](#working-brief)
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

- **Feature:** something to build, which needs a task in the plan.
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
- **Done when:** checks proving it is delivered.
- **Check:** how we verify a rule keeps holding.

*What*, *Done when* and *Check* are binding; *Why* and *Example* only explain; any other label counts as part of *What*.
If a summary and its source disagree, the source wins.
"About" means within 10% unless stated; chances and shares are checked as `RES-13` sets out, and a blueprint's chance as `RES-24` does.
Each item's detailed acceptance criteria are written in the plan, and you approve them in the review that closes the stage before (`RES-22`), the first stage's with the plan itself; loosening one you approved needs your OK again (`RES-09`).

### Changing this file

- AI agents suggest additions or changes as *Proposed* items (`PRC-07`).
- To change a decided item, a **Proposed change:** line goes beneath it, with the new text and the reason.
  Your OK replaces the text.
- Items are edited in place, and keep their IDs.
- Every commit that changes this file ends with a line naming the changed IDs and why, for example `Changed: GOD-04 (blessing cap raised; owner OK)`.
  The commit check enforces it (`PRC-07`), and the coverage check flags work linked to a changed item (`PRC-12`).
- Parts delivered across stages may keep one stable ID (`PRC-12`).
  Delivery notes state proved behaviour and remaining checks; full acceptance stays open until all pass.
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

- `VIS-06` **In one paragraph** *(Decided)*: A living world of land, weather, plants, animals and named people with needs, moods, memories, friends and rivals.
  Family bands with a shared language and almost no culture discover about 140 hidden blueprints by accident, experiment, dreams and copying; each people's first success is a named discovery.
  Sixty-day years carry them from sharp flakes to first copper over a few hundred years.
  As their hidden god, you send weather, dreams and luck, never commands; nobody learns you exist (`SCP-01`, `MAT-21`, `GOD-06`).

- `VIS-02` **The fantasy** *(Decided)*: You are a hidden god who acts only through nature: the weather, the luck and the dreams.
  - **What:** You can bring a storm, send lightning, hold back the rain, bless a hunter, or give someone a dream made of their own memories, but you can't speak, appear or work miracles.

### 1.2 What it feels like

- `VIS-07` **Wonder** *(Decided)*
  - **What:** Awe at a world that runs itself and keeps surprising you, its maker included.

- `VIS-08` **Curiosity** *(Decided)*
  - **What:** The urge to understand why something happened, and to try "what if".

- `VIS-17` **Life** *(Decided)*
  - **What:** The pleasure of a world that is always busy at every zoom, with everyone doing something for reasons of their own.
  - **Done when:** whole worlds pass `RES-25`, and in a camp scene of about 30 by day, awake people are doing at least 6 different activities at most moments.

- `VIS-09` **Other feelings** *(Decided)*: Attachment to particular people, and the harshness of nature, will arise from the simulation and are welcome, but the design isn't built around them.
  When design choices conflict, wonder, curiosity and life decide.

### 1.3 How you play

- `VIS-10` **Two rhythms of play** *(Decided)*
  - **Short check-ins (5–15 minutes):** open the app, catch up on the live moments waiting, follow someone for a while, nudge, close.
  - **Long sessions (an hour or more):** watch a season or a century go by, read the book of ages, visit graves and old camps, and try a "what if" with your powers.
  - The world pauses when the app is closed (`TIM-05`), so every session starts exactly where the last one ended.

- `VIS-11` **A session, as a story** *(Decided)*: An illustration, not a script: a live moment brings you to a band whose fire died.
  You connect a woman's memory of a smoking stick with fire's warmth in a dream; she tries again, discovers fire-making and teaches it.
  Centuries later, villagers tell her discovery as a myth (`MOM-01`, `CUL-11`, `PRE-08`).

### 1.4 Signature moments

- `VIS-12` **Signature moments** *(Decided)*: Stories the game must be able to produce, none of them scripted.
  Each has a sandbox scene that must keep producing it (`RES-17`): its **Check** gives the scene, the years the moment has, and the stage from which the scene runs.
  A moment depends on every item it cites.

  - `MOM-01` **Fire from wood** *(Decided)*: In a hard winter, a band whose fire has died learns to make fire by drilling, a trick one of them stumbled on.
    - **How it works:**
      1. Someone twirling a stick against dry wood while experimenting or at play (`MND-11`) matches the blueprint for fire by drilling (`RCK-02`) and fails in its usual way, with smoke but no ember; the surprise gives a hunch (`MND-10`, `MND-11`).
      2. When the band's found fire dies (`BIO-02`), cold pushes the most curious to try the hunch again and again with the driest wood they can find, until an ember glows: a named discovery (`MAT-21`), passed on by teaching (`MND-13`).
      3. A dream you send can set the smoking stick beside the warmth of a fire, raising the odds but promising nothing (`GOD-03`).
    - **Check:** a band of about 20 with dry wood and tinder, one of them holding a hunch from a smoking stick, loses its fire as winter starts; it makes fire by drilling within 1 year; from `MIL-10` once all cited dependencies are complete.
  - `MOM-02` **The lost craft** *(Decided)*: A fever kills the last person in a band who can make fine blades.
    For years, sometimes a generation, its tools are cruder, until the craft is found again, learned from neighbours, or copied from an old blade.
    - **How it works:**
      1. A fever (`BIO-05`) kills the last person who knew the blade blueprint (`BIO-14`), a craft that takes long experience in stone (`MND-06`); the knowledge dies with them (`CUL-02`), and the survivors' plain flakes are cruder and wear out sooner (`MAT-20`).
      2. It comes back only by experimenting, from neighbours (`CUL-16`), or by copying an old blade, which gives only a weak hunch (`MND-11`); the book of ages marks the loss and the return (`PRE-39`).
    - **Check:** two bands a day's walk apart, the blade blueprint known to one adult in one of them, where a fever starts; within 40 years the craft is lost, then made again; from `MIL-15`.
  - `MOM-03` **Your lightning becomes a god** *(Decided)*: A lightning strike you sent kills a hunter on a hilltop.
    - **How it works:**
      1. A death from the sky (`GOD-02`) can be put down to an unseen being in the storm (`MND-05`, `CUL-05`), and fear ties itself to the hill (`MND-08`).
      2. They avoid the hill (`CUL-20`); an angry spirit pulls its believers to leave gifts at its place (`CUL-05`), and when the storms stop after one, the gift is credited (`MND-05`) and leaving gifts there becomes a rite (`CUL-34`).
      3. Retold, the story becomes a myth (`CUL-11`), and in time part of their religion (`CUL-26`); nothing marks the strike as yours (`GOD-06`).
    - **Check:** a band whose hunters cross a bare hilltop, where the test's lightning kills one; within 30 years the band avoids the hill, leaves gifts there as a rite and tells a myth of the one in the storm; from `MIL-15` once all cited simulation and presentation dependencies are complete.
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
      2. The hunt is in the saved history (`PRN-15`), and a painting of it keeps it from being thinned (`PLT-10`), so tapping the painting shows what it depicts and actual events (`PRE-15`).
    - **Check:** a band with ochre, fat and a sheltered wall, just after a great hunt; within 80 years a painting of it outlives its last witness, and tapping it shows the saved hunt; from `MIL-15` once all cited simulation and presentation dependencies are complete.
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
    - **Check:** a band without needles stores food in a cave whose floor hides an old camp's eyed needles; within 10 years someone digs one up and makes a needle; from `MIL-15`.
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
  - **What:** Start in caves; sharp flakes arrive within two years, fire within about eight, pottery, dogs, herds, villages and fields over the next century, and first copper about a century in (`SCP-01`, `TIM-19`).
    History continues within the launch catalogue; later layers may add bronze or writing (`PRN-14`).
  - **No scripted eras:** rules alone produce each step, in varying orders; stalls, lost crafts and extinct peoples are valid (`PRN-17`).
  - **The phone's limit:** births are uncapped; past about 2,000 people, time slows without losing detail; near the memory limit, pause with a notice (`BIO-04`, `MND-15`).

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
  - **What:** People know only their starting knowledge (`BIO-20`) and what they sense, try, learn from others or dream.
    A blueprint must be discovered or taught; the sole exception is your Revelation (`GOD-13`), which gives knowledge but leaves making to practice.
    Choices never use unknown blueprints or facts neither seen nor told (`MND-02`).
  - **Check:** every discovery has its route (`MND-11`); tests find no choice using unknown blueprints or unseen facts.

- `PRN-02` **Believable over exact** *(Decided)*
  - **What:** The world looks, sounds and behaves believably; where exact modelling is costly, simple believable rules win (`SCP-21`).
  - **Check:** each milestone review flags detail with no visible or readable effect.

- `PRN-07` **Generic blueprints** *(Decided)*
  - **What:** Everything made on purpose uses a blueprint asking for actions, characteristics, classes, forms and conditions, never particular items (`MAT-04`).
    Fire, timers and the living world also change things, under fixed catalogue rules shared by everyone (`MAT-18`, `MAT-19`).
    A blueprint works for every fit, allowing unplanned routes; catalogue review checks each fit once for absurdity (`MAT-17`).
    Choice rules never single out blueprints and read only perceived and known facts.
    Blueprint names, such as flake or pottery, appear only in catalogues, tests and player text.
  - **Check:** no blueprint names an input item; choice rules hold no blueprint, discovery or item names; exchanging material names changes no behaviour; a made-up material works in every fit.

- `PRN-05` **Plausible numbers** *(Decided)*
  - **What:** Every value is a hand-set plausible estimate of reality, tuned for play; sourced measurements are unnecessary.
    Yearly crop and animal yields scale to preserve real daily yields (`TIM-18`, `WLD-30`).
  - **Check:** catalogue tests pass (`MAT-17`); every milestone-report claim has a test that can fail (`RES-01`).

- `PRN-12` **Speed up time, never bend the rules** *(Decided)*
  - **What:** Zoom, director and manual speed control time; tuning occurs during development, identically for every world (`PRN-17`).
    Play never changes rules for speed or drama, or causes an event for its story.
  - **Check:** no rule-bending play settings; director on/off leaves the same saved-world result (`TIM-03`).

- `PRN-17` **History at a watchable pace** *(Decided)*
  - **What:** Meet discovery pace targets (`TIM-19`) by tuning chances and amounts identically for all worlds, including experimentation, success chances and food yields.
    Discoveries are never scripted or date-forced.
  - **Check:** pace tests meet typical-world windows in varying orders without scripting (`RES-07`).

### 2.2 The player

- `PRN-03` **You are nature** *(Decided)*
  - **What:** The player acts only through natural means (`GOD-05`), with one exception, Revelation (`GOD-13`), and is never known to exist (`GOD-06`).
  - **Check:** every power but Revelation produces only events the world could produce on its own.

### 2.3 What you see

- `PRN-04` **If the game knows it, you can see it** *(Decided)*
  - **What:** Show every tracked fact, including needs, mood, memories, beliefs, known blueprints, family and teaching lines, graves and old camps.
  - **Check:** each tracked fact has a view, card or mind-detail entry (`PRE-35`, `PRE-14`).

- `PRN-10` **Nothing is faked** *(Decided)*
  - **What:** Every picture, sound and word reflects actual world state and events; nothing is added for show.
    Unvisited places are drawn from the same seed used when visited (`WLD-13`).
  - **Check:** every live moment, chronicle entry, sound and on-screen event traces to world events.

- `PRN-13` **Every choice can be explained** *(Decided)*
  - **What:** Every person or animal choice traces to needs, personality, plans, beliefs and memories; its top reasons are kept for the details view (`PRE-14`).
  - **Check:** current activities and choices behind all saved events show their kept reasons (`PRN-15`).

- `PRN-06` **AI language models describe, never decide** *(Decided)*
  - **What:** Language models only reword record-based pattern sentences for chronicles, life stories, myths and summaries (`PRE-37`).
    They never choose, invent or know for people or animals, or add, drop or change facts (`PRE-41`).
  - **Check:** no model text feeds back into play; each sentence is checked against its source pattern.

### 2.4 How it runs

- `PRN-15` **History is saved, not re-run** *(Decided)*
  - **What:** Keep the chronicle, its events and records needed by past views: graves, camps, families and art.
    Only the present has a full saved state (`PLT-07`); the past cannot be replayed, returned to or remade from the seed.
  - **Check:** all past views read saved records and never rerun history.

- `PRN-11` **Time slows, the screen stays smooth** *(Decided)*
  - **What:** Under load, slow time to prevent stutter; never cut detail.
    Every person remains a full individual; only planned, camera-independent simplifications apply (`MND-14`, `WLD-12`, `WLD-32`, `MND-16`).
  - **Check:** no heavy-load stutter (`PLT-04`); saved-world results match at every speed and zoom (`TIM-17`).

### 2.5 How it's built

- `PRN-09` **Build in steps you can try** *(Decided)*
  - **Owner OK:** 8 October 2026.
  - **What:** Each alpha adds one visible change you can install and try on your phone (`SCP-03`, `PRC-11`).
    The next milestone is a living camp with an indirect power (`MIL-09`), followed by a discovery slice (`MIL-10`).
    Build only what each alpha needs, on foundations later work extends.
  - **Check:** every alpha ends with an installable build, and every implementation task names its IDs.


- `PRN-14` **Modular by design** *(Decided)*
  - **What:** Grow systems through self-contained items, blueprints, species, illnesses, behaviours, views and tests; explain any rewrite of working parts.
  - **Check:** each milestone report lists additions and confirms no earlier rewrite, or explains why it was necessary.

## 3. Scope and non-goals

### 3.1 Where history starts

- `SCP-01` **Starting point** *(Decided)*: Modern minds with almost no culture.
  - **What:** Start with 3–4 family bands, 45–120 people, in one region sharing a language (`BIO-03`, `CUL-17`).
    No shaped tools or clothing, only natural shelter; they cannot make fire, but can keep and carry found embers (`BIO-02`).
    This sole starting point serves play; real early humans had more culture.

### 3.2 Who it's for

- `SCP-02` **Just you** *(Decided)*
  - **What:** Kindling is built for one person, on one phone:
    - no public release, store listing or tutorial, only short help cards (`PRE-40`);
    - no other phones, tablets or computers (cloud tests are a building tool, not a way to play);
    - no accounts, purchases, ads or analytics;
    - free to use your phone's own hardware (`PLT-01`).
  - **Check:** the builds hold no account, purchase, advertising or analytics code, and target only your phone.

### 3.3 How it gets built

- `SCP-03` **Alphas you can try** *(Decided)*: Each alpha delivers one visible change you can install, open and try on your phone (`PRN-09`, `PRC-11`), with automated tests (`RES-01`).
  - **Owner OK:** 8 October 2026.
  Estimate from completed work and split overruns; a new system need not fit a few hours.
  - **Check:** `PRC-11`.


- `SCP-15` **Tests run in the AI's cloud sessions** *(Decided)*
  - **What:** Automated tests run in the building sessions; pace sizes and computing follow `RES-07`.
    Raise any additional computing need with you first.
  - **Check:** milestone reports state test locations and computing used (`RES-06`).

- `SCP-16` **Milestones** *(Decided)*: After accepted foundations (`MIL-08`), build the living camp and discovery slice, then finish a small valley game before choosing wider-world, culture and copper expansions.
  - **Owner OK:** 8 October 2026.
  Full original scope remains open until delivered or explicitly retired.
  Each stage contains several alphas (`SCP-03`) and closes with your report and review (`RES-06`, `RES-22`).
  The stages below set order and scope; the implementation plan maps every feature and rule and details only the next stage.


  - `MIL-18` **Pre-production** *(Decided)*: Risk prototypes, tested riskiest first on phone or in cloud; closed on 5 October 2026.
    The unbuilt writer remains unproved (`PRE-37`); the prototype vertical slice was dropped on 6 October 2026.

  - `MIL-08` **Foundations** *(Decided)*: the app and its delivery to your phone; the simulation library in C++; numbers, time, the 60-day year, dates and chance, with the same bits on the phone and in the cloud (`RES-05`, `TIM-16`, `TIM-18`); entities, activities with an end (`TIM-17`), and catalogues with their checks (`MAT-13`, `MAT-17`); saves, several worlds and export (`PLT-07`, `TIM-08`, `PLT-08`); test scenes, the repeat check and the phone benchmark (`RES-21`, `PLT-04`).
    *Now possible:* the foundations working on your phone: a calendar at any speed, a crowd of markers moving smoothly, and worlds saved and reopened exactly.
  - `MIL-09` **M2: A living camp** *(Decided)*: A saved camp with real daily needs and a place dream, using existing rendering and labelled stand-in art.
    - **Owner OK:** 8 October 2026.
    *Now possible:* watching people meet their needs, nudging through a dream, and continuing the saved camp.

  - `MIL-10` **M3: Discovery slice** *(Decided)*: Stone and fire discovery, learning and idea dreams in a readable playable slice.
    - **Owner OK:** 8 October 2026.
    Full world generation moves to `MIL-14`.
    Physical-world checks remain open there; living settling, resource renewal and real buried traces close only when their complete systems exist (`MIL-15`), as do starting bands and naked-band winter survival.
    Water-only settling and synthetic traces cannot pass those full checks.
    *Now possible:* following a discovery, its cause and its spread, and trying an idea dream.

  - `MIL-11` **M4: A changing camp** *(Decided)*: Seasonal food, a wound/care chain and a family through time, each delivered separately.
    - **Owner OK:** 8 October 2026.
    Full nature and body catalogues remain open for later additions (`MIL-15`).
    *Now possible:* watching seasons, care and family life change the same camp.

  - `MIL-12` **M5: Two camps** *(Decided)*: Two interacting camps with readable knowledge transfer and relationships.
    - **Owner OK:** 8 October 2026.
    Full body and population promises retain later acceptance (`MIL-15`).
    *Now possible:* following what one camp learns from another and why their people meet.

  - `MIL-13` **M6: A small finished valley** *(Decided)*: First-launch flow, history, useful powers and reliable continuation make the small valley game complete.
    - **Owner OK:** 8 October 2026.
    Full minds, culture and world scope are outside this first release and remain open.
    *Now possible:* enjoying and continuing a small game.

  - `MIL-14` **M7: Wider world, if chosen** *(Decided)*: A conditional scope map, later split into short playable stages with people present.
    - **Owner OK:** 8 October 2026.
    Full geography acceptance remains unchanged: generation and candidate choice (`WLD-08`, `WLD-09`, `WLD-10`), layers and detail (`WLD-12`, `WLD-13`), sky, climate, water, seas, soil and geology (`WLD-07`, `WLD-16`, `WLD-17`, `WLD-26`, `WLD-27`, `WLD-14`, `WLD-15`), globe-to-person zoom and cutaways (`WLD-02`, `PRE-03`, `PRE-25`).
    Full living settling and starting-band checks remain open until `MIL-15` supplies their dependencies.

  - `MIL-15` **M8: Lives, animals, crafts and society, if chosen** *(Decided)*: A conditional scope map of remaining nature, bodies, minds, animals, crafts and society; choose one causal chain per future playable stage.
    - **Owner OK:** 8 October 2026.
    Original promises and acceptance remain open, including full ecology and buried traces; starting bands, body and population rules; every mind and culture rule; craft chains, the reality rules, clothing, huts and dogs.
    Related catalogue counts and checks remain binding until explicitly amended.

  - `MIL-16` **M9: Powers, presentation and sound, if chosen** *(Decided)*: Complete remaining powers, presentation and sound as improvements to an existing game.
    - **Owner OK:** 8 October 2026.
    Full power, time/director, history, screen, card, overlay, art, animation and sound acceptance stays open until delivered.
    Controls and history arrive with the behaviour they explain; the writer is optional (`PRE-37`).

  - `MIL-17` **M10: Settlements to copper, if chosen** *(Decided)*: An optional expansion after the small first finish; if commissioned, split pottery, herding, farming, villages and copper into playable chains (`RCK-04`, `WLD-33`, `CUL-28`, `RCK-08`).
    - **Owner OK:** 8 October 2026.
    Full arc acceptance remains: the launch catalogue (`MAT-23`) and every discovery and culture pace target (`TIM-19`, `CUL-33`).

### 3.4 Non-goals

- `SCP-04` **No tech tree** *(Decided)*: Blueprints exist, but they are hidden and generic, found only in play, by accident, by experimenting, in dreams or by copying (`MND-11`), and never chosen from a menu or unlocked with points (`PRN-07`).
  People never choose from a list; the only list of blueprints is the one your Revelation picks from (`GOD-13`).
  - **Check:** the game has no menu, list or tree of discoveries for people to choose from, and nothing on screen shows a blueprint nobody in the world has found, apart from an idea dream and the marks on memories that can become one (`GOD-03`), and the list your Revelation picks from.
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
  - **Check:** every power works through a natural system; none reshapes land beyond what the quake, eruption or flood it sets off would do naturally, and none adds or removes a species.
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
  - **Check:** each milestone review confirms that no part of the game simulates any of these.

## 4. The player as god

Your powers are weather, dreams, animal dreams, fortune and Revelation (`GOD-05`, `GOD-13`).

### 4.1 Your role

- `GOD-01` **Role** *(Decided)*: A distant, invisible god in a sandbox: you watch everything and nudge, but never command or control anyone (`SCP-17`), so every achievement stays theirs.

- `GOD-06` **Never known** *(Decided)*
  - **What:** People and animals meet your acts only as nature, and nothing in the world can tell that an event was yours.
    They explain your acts as they explain any event, through spirits, ancestors or not at all (`MND-05`), and what they believe is their own, right or wrong.
  - **Check:** nothing a person or animal can sense, remember or believe records whether you caused an event.

- `GOD-05` **Only natural means** *(Decided)*
  - **What:** Every act is something the world itself could do there and then, through its own weather, sleep and luck; it never makes anything from nothing (`MAT-09`) or bends a rule (`PRN-12`).
    Revelation (`GOD-13`) alone gives knowledge directly, under long rests.
    There is no power to collect or spend: each works only where its natural cause is present; you choose where and when, and the world how strong; each power's item limits how many run at once and how soon it can be used again; nothing is instant, weather falls on everyone in reach, and nothing can be undone.
  - **Check:** tried across many places, seasons and targets, each power is offered only where its conditions hold and brings what a natural event of its kind and size would; a year of using every power as often as allowed never gives a place more storms, rain, dry spells or cold than its climate's worst year.

### 4.2 Your powers

- `GOD-02` **Weather and disasters** *(Decided)*
  - **What:** Seven powers over weather and land:
    - **Lightning:** under a thunderstorm, you aim its next strike at a spot; it hits the tallest thing within a few metres and can split, burn, wound or kill (`MAT-18`, `BIO-13`).
    - **Rain:** where its weather cell, about 10 km across, has cloud (`WLD-16`), rain falls on it for up to a day, or snow where it is cold enough.
    - **Storm:** where that place's climate has storms in that season, one gathers upwind, arrives within a few hours, with thunder where the air is warm and moist, and moves on with the wind within a day.
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
  - **Done when:** in scenes, each power brings its event in its stated time, and only where its conditions hold (`GOD-11`).

- `GOD-03` **Dreams** *(Decided)*: Choose a person's next dream from their experiences, in five kinds:
  - **Place:** known place draws a visit (`MND-28`).
  - **Animal:** previously met animal draws hunting, watching or feeding.
  - **Person:** known person draws talk, help, courtship or peace.
  - **Fear:** known place, animal or person draws avoidance.
  - **Idea:** select a memory of doing/handling; pair it with a new blueprint's result, such as smoking stick/fire warmth.
    Blueprint starts from that memory; dreamer knows its action and has handled similar inputs.
    Among fits choose strongest sector experience, then likeliest success; wake with a persistent ordinary hunch (`MND-11`).
  - **Pull:** lasts a few days (tuned), tips close choices but never beats hunger, danger or an existing plan (`MND-09`).
  - **Limits:** one/sleeper/night, up to three dreams total including animals.
    Only blueprint-leading memories allow ideas; marks on them are the sole hidden-blueprint hint, never revealing results (`SCP-04`).
    Replace that night's natural dream/hint (`MND-12`); repeats refresh without strengthening.
  - **Done when:** an idea from a remembered dry-wood twirl causes new tries within days in at least 15/20 runs (`RES-13`).

- `GOD-12` **Animal dreams** *(Decided)*
  - **What:** While an animal rests, or at night for a herd far from people, you can send it one of three simple dreams:
    - **toward a place** within its range, such as the scraps at a camp's edge: over the next few days it drifts there, but a herd never leaves its range for that season (`WLD-32`);
    - **calmer:** for a few days it startles less, flees later and fights less, so it is easier to approach, hunt or tame (`WLD-33`);
    - **bolder:** for a few days it comes closer to people and camps and stands its ground, but is also quicker to fight when cornered.
  - **Far from people,** a herd is a count, so calmer or bolder lowers its wariness of people for those days.
  - **Near people,** a herd follows its lead animal (`MND-16`), so a dream toward a place moves it only when sent to the lead, which the ring marks; calmer or bolder changes only the animal dreamt.
  - **Limits and costs:** one dream per animal or herd per night, within the three a night, repeated as `GOD-03` says.
  - **Done when:** a herd sent toward a place in its range reaches it within a few days in at least 15/20 runs (`RES-13`).

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
    One person: they know its blueprint at once, at minimum skill.
    Everyone: every living person knows it at that skill.
    Knowing is not making: they still need the right things, the right place and practice to succeed (`MAT-04`, `MND-06`).
    They explain the sudden knowing their own way, as a dream, a spirit's gift or their own cleverness (`MND-05`, `CUL-05`).
    The book of ages records its first use as their discovery, and only your record shows it was you (`GOD-07`, `GOD-08`).
    It is the one power beyond natural means, the exception that `PRN-01` and `GOD-05` name.
  - **Limits:** after a revelation to one person, about a game year must pass before the next; after one to everyone, about a game century.
    Pace tests never use it (`TIM-19`).
  - **On the phone:** choose Revelation, pick a sector and a named result from the list, then tap a person or "everyone".
  - **Done when:** in a scene, a person given a revelation uses the blueprint within 5 days when it serves their needs and its inputs are in reach, in at least 18/20 runs; a revelation to everyone reaches every living person; the rests hold.

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
  - **Done when:** in scenes, every power the ring leaves out shows its reason, and the world carries out every act the ring offers.

### 4.4 Records of your acts

- `GOD-08` **Recorded behind the scenes** *(Decided)*: Every act is saved with the world's history: when, where, on whom, and every choice you made (`PRN-15`).
  It feeds only your marked lines and the pages of what came of your acts (`GOD-09`), never a mind or a text (`GOD-06`, `GOD-07`).
  - **Done when:** a test world saved and reopened lists every act with its date, place, target and choices.

- `GOD-07` **No trace in the story** *(Decided)*: The book of ages, live moments and every text written for you tell what happened, never that it was your doing.
  If you choose, your acts show beside the story as separate lines marked as yours, never woven into the text (`PRE-05`).
  - **Check:** a test runs a world with many acts and finds no mention of them in any entry, live moment or text, apart from the marked lines.

- `GOD-09` **What came of your acts** *(Decided)*: Each act's page gives date, place, target and recorded consequences, never guesses.
  Open from chronicle act line (`PRE-05`), affected-person details (`PRE-14`) or lasting world mark (`GOD-10`).
  - **Weather/land:** caused deaths, wounds, fires, floods and shelter collapses within its footprint during the act and one day after (`BIO-14`).
  - **Fortune:** each changed roll.
  - **Person dream:** subject-related tries/discoveries and subsequent teaching line (`MND-13`).
  - **Animal dream:** its travel while dream lasts.
  - **Done when:** successful idea page shows discovery/all learners; cold-snap page lists exactly its local cold deaths during the act and one day after.

## 5. Time and history

Time works one way for everything: activities that start and end on one world clock, through a 60-day year, by the same rules at every zoom and speed.

### 5.1 How time works

- `TIM-17` **Activities with an end** *(Decided)*
  - **What:** People and nearby animals act through interruptible start/end activities; results land at end under common, camera/speed-independent rules (`WLD-13`).
  - **Length:** real-life duration (`TIM-18`); blueprint work repeats tries, each result landing at its own end, until the choice's goal is met or about an hour passes (tuned).
    About 10–30 activities/person/day; roll each fitting unknown blueprint once per activity, at end (`MND-11`).
  - **Waiting:** timers need nobody except required tending in short activities; untended required work fails (`MAT-19`).
  - **Between ends:** needs change with time/effort (`BIO-09`); at end choose again (`MND-09`, `MND-16`).
  - **Interrupt only for:** noticed danger (`MND-03`), pain, blows/falls, a call, need <20, timed plan step (`MND-22`) or death.
    All else waits; parallel talk interrupts nothing (`MND-33`).
    A call shouts name/kin word to warn, request help or fetch; passing chat/telling/questions are talk.
  - **Impossible to continue:** also end, preserving progress, when tool breaks, inputs/target run out or are taken, fire dies, what is sought is found/lost from sight, or shared attendance falls below minimum.
  - **Retained progress:** walkers keep position; eating/drinking/rest/warming/talk/teaching/watching give elapsed shares; partial object work remains for anyone to finish; interrupted single strikes/throws do nothing.
  - **Shared:** initiator sets place/time, now or within about a day, and tells/calls others (`CUL-22`, `CUL-34`).
    Joiners hold timed plan steps.
    Start when minimum arrives (two for talk, `CUL-22` for group plans); fail after about an hour waiting (tuned), then everyone chooses again.
    Late joining allowed except rites/dances, whose start/end are simultaneous.
    Each participant receives results on leaving/end (story, skill, meat share); count one activity/person; shared production follows `MAT-04`.
  - **Movement:** route and pace determine instantaneous position for seeing, meeting and attacks.
    Chases/fights are a few seconds (tuned), choosing again as targets move.
  - **World:** land/things follow `WLD-12`, `MAT-18`, `MAT-19` on this clock.
    Results occur at their exact game moments regardless of speed, simultaneous results in fixed order.
  - **Done when:** half-finished meals satisfy about half hunger; another finishes a half-scraped hide in remaining time; called-away learners retain earned skill.
  - **Check:** saved-world results match all speeds/zooms (`TIM-16`); real/top-speed walkers meet, see and flee at identical game seconds.

- `TIM-18` **The game year** *(Decided)*: 60 days, four 15-day seasons (spring, summer, autumn, winter), each day 24 hours.
  - **Duration:** up to about two real weeks keeps real time; months/years take about a sixth; intermediate healing, starvation, scurvy and long illnesses use their items' tuned lengths (`BIO-05`, `BIO-09`, `BIO-10`, `BIO-13`).
    Meat rotting/hide drying keep real time; growth, ageing, pregnancy, crops/trees and grief compress (`BIO-04`, `WLD-31`, `MND-19`).
  - **Daily rates:** eating, drinking, fatigue, work, walking, weather and accidents retain real daily rate/chance.
  - **Yearly totals:** births, crops, animal young, outbreaks, droughts, floods, wildfires, harsh winters and quakes occur as often/game year as real year; scale land yields (`WLD-30`).
    Pre-age deaths match real foragers' yearly rates through illness/birth risks, never extra accidents (`BIO-04`).
  - **Build-ups:** skill, wear, tameness, path wear and drifting opinions accumulate about six times faster daily (`MND-06`, `MAT-20`, `RCK-24`, `MAT-08`, `MND-24`).
  - **Land:** drought lowers springs/streams/lakes and wilts plants about six times faster; moon alone stays one full moon/season (`WLD-17`, `WLD-07`).
  - **Check:** every catalogue duration records real/game lengths and passes the scaling rule; a rainless season dries streams/browns grass like a real dry summer.

- `TIM-14` **Dates** *(Decided)*: Show year, season and day: "Year 112, autumn, day 6".
  - History starts at Year 1, spring, day 1; seasons have days 1–15 (`TIM-18`).
  - Use the starting hemisphere's season names globally; opposite-hemisphere entries add the local season: "winter (their summer)" (`WLD-01`).
  - All ages, durations, targets and tests use game days and years.
  - **Done when:** every displayed date has all three parts; opposite-hemisphere entries add the local season.

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

- `TIM-07` **Speed target** *(To test)*: World-view game years per real minute on your phone:
  - no people: at least 10;
  - about 100–300: at least 5;
  - 1,000: at least 1, aiming for 2–3;
  - about 2,000: at least half;
  - beyond: slow time (`MND-15`), retaining at least a sixth at about 7,000, the Year-250 maximum (`BIO-04`);
  - Year-250 worlds: at least four fifths of a new world's speed at equal population.
  - **What counts:** every person and nearby-animal activity and its results, notices and talk topics, on permitted cores (`PLT-01`).
  - **Check:** `PLT-04` benchmarks at held speed.

### 5.3 The story director

- `TIM-02` **Story director** *(Decided)*: Watches the whole world, adjusting only speed around important moments; quiet time follows zoom.
  A slowdown offers a live moment with one-tap travel (`PRE-08`).
  - **Moments:** named discoveries and other firsts (`MAT-21`); followed-person births/deaths (`PRE-06`); craft transfers/losses (`CUL-02`); last band fire lost; fights/raids/feuds (`CUL-31`); disasters (`WLD-22`); peoples forming/splitting/ending, villages founded (`CUL-23`, `CUL-28`); player-act consequences (`GOD-09`).
  - **Advance signs, exhaustive:** repeated same-hunch tries in one day; a predator stalking a person; storm forming over camp; hostile groups in mutual sight; followed person gravely ill/wounded.
    Never look ahead.
  - **Importance:** tuned event/sign scores increase with affected numbers and followed people; Gentle excludes dark events (`PRE-18`).
  - **Slowdown:** above live-moment threshold, aim for about half a real minute to the imminent event (tuned): near real speed for stalking/hostile meetings, camp speed for birth/fire-stick morning, camp-to-valley for day-long flood.
    Higher scores increase likelihood, not slowness; never faster than zoom.
  - **Shared budget:** live moments and slowdowns at most one/about 3 real minutes, at most a fifth of elapsed time slowed; untapped slowdowns end after about 10 seconds (tuned).
    Events excluded by budget/dial/lock wait in the list; skip stops bypass budget (`TIM-11`).
  - **Done when:** an hour's globe viewing lists all named discoveries/followed deaths, slows for budget-allowed moments and retains four fifths of top speed.

- `TIM-03` **The director never touches events** *(Decided)*: The director sets only the speed and the live moments; it never causes, changes or hides anything.
  - **Check:** the same saved world, run with the director on and off, gives the same results (`TIM-16`), and a code check finds no path from the director into the world.

- `TIM-11` **Skip to the next moment** *(Decided)*: A control that runs time at top speed until the next important moment, for short check-ins (`VIS-10`).
  - **How it works:** at the director's next moment past the bar (`TIM-02`), it slows to the director's speed, even over the dial or the lock, until you tap or it passes, then hands the speed back (`TIM-15`), or after a year with no such moment.
  - **Done when:** in worlds, skip stops at the next moment past the bar or after a year, and only then hands the speed back.

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

- `TIM-19` **Pace of discovery** *(To test)*: Typical worlds first reach each step within these year windows (`TIM-14`):
  - sharp flakes: 1–2;
  - fire-making: 2–8;
  - clothing and huts, each: 3–10;
  - pottery: 15–38;
  - tame dogs: 20–38;
  - herding: 30–63;
  - villages: 25–75;
  - farming: 50–88;
  - copper: 75–125.
  - **Counts:** first chronicle entry anywhere; crafts use `MAT-23`'s marked results, including smelted copper; dogs/herding use `WLD-33`, villages `CUL-28`, farming the first deliberately sown and harvested crop (`RCK-23`).
  - Tune only, never script or force by dates (`PRN-17`, `RES-16`).
  - **Check:** `RES-07`, without player acts.

- `TIM-09` **If everyone dies** *(Decided)*: The world goes on without them, and you can keep watching or start a new world (`WLD-10`).
  - **How it works:** the last death is an important moment and a book-of-ages entry (`TIM-02`, `PRE-05`), and nature runs on, faster.
  - **Done when:** a test world whose last person dies runs on, with that death in the book of ages and the choice offered.

## 6. World

The world is a small planet that wraps around, made once in realistic detail and then kept alive at a pace the phone can carry, in full detail only where people are (`WLD-12`).
Numbers here are starting values, tuned in tests (`PRN-17`).

### 6.1 Shape and size

- `WLD-01` **Torus with latitude** *(Decided)*: The map wraps both ways, with the equator across the middle and the poles along the line where it wraps north to south, so seasons behave as on a planet, reversed between the halves.
  - **The polar seam:** a permanent ice cap about 200 km wide lies along that line, and nothing ever crosses its middle, not even weather or well-supplied people.
  - **Done when:** in a world, a walk due east comes back from the west, and in 100 years nothing crosses the middle of the polar ice.

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
    - **Events** come about as often per game year as on Earth per year; where daily weather gives too few, such as lightning fires and storm floods, the chance per storm is raised to match.
      Weather keeps millimetres per real-length game day; biome climate indices use Earth-equivalent annual precipitation and growing-degree totals, while seasonal totals cover 15 game days. Compressed dry-spell recession acts on reserves, never on wind or river travel.
  - **Check:** every scaled catalogue value is marked, with its Earth value; in a scene, a band of 25 on good temperate land needs about 100–300 km² to forage all year.

- `WLD-04` **How many people it can feed** *(To test)*: Estimated at tens of thousands of foragers, about one per 10 km² of good land, and ten to a hundred times more with farming.
  It is far above what the phone runs (`MND-15`), so the land limits people only locally.

### 6.2 Layers and systems

- `WLD-12` **Map layers** *(Decided)*: Four layers on one clock; make detail only when needed; looking changes nothing (`WLD-13`).
  1. **World cells:** about 2 million, about 1 km across; height, rock, soil, biome, plant cover, water, deposits, snow, fire, paths and passing herds.
  2. **Areas:** exactly 250 m across, 16 per cell, detailed to about 1 m; ground shape/material, stones, individual trees/bushes, cover, caves, overhangs, cliffs and water.
  3. **Things and creatures:** every person, nearby animal and item, in areas or on cell ground where no area exists (`MAT-10`).
  4. **Weather cells:** about 20,000, about 10 km across (`WLD-16`).
  - **Paces:** weather and burning wildfire hourly; water, snow/ice, fuel dryness and herds daily, flood water hourly; cover, small-animal counts, seas and soils every 5 days; creatures and things by activities (`TIM-17`).
    Budget overruns (`PLT-04`) may justify a uniformly coarser pace in a later version, never during play or reduced detail near people.
  - **Area creation:** where people stop to act, their target stands (such as stalked deer), or something happens to them; for pictures only, within about 300 m of camera from camp zoom inward, coarse ground beyond (`PRE-03`).
    Simulation areas hold all layer-2 data except metre-scale ground shape, made only for pictures.
    Between areas, cell slope, cover, fords and paths set walking routes/times; walkers notice cell contents.
  - **Unchanged areas:** exactly seed + cell/neighbours + date; no independent rules.
    Ground, cliffs and woods meet across edges; generated river lines fix agreement.
  - **Kept areas:** retain only people/tame-animal changes: made, moved or left things, carcasses, stumps, pits, hearths, trodden camps, plots, planted/stripped plants and burned ground.
    Marks fade (`MAT-08`); no remaining change makes an area unchanged again; seed/cell supply everything else.
    Run only within about 1 km of someone.
    On approach or display, catch up day by day: recorded cell fire, flood, lava and quakes; timers/marks under usual seasonal weather; chance keyed to each day (`TIM-16`).
    Catch-up ends identically however late/often; pictures save nothing.
  - **Cells:** growth/seasons, wild grazing, wildfire, flood and snow act here.
    Cutting groves, clearing fields and killing animals immediately reduce cover/herds; cell regrowth restores them.
    Count path wear once per traversing walk; unused paths fade after a few years; areas draw cell paths.
  - **Done when:** unchanged-area regeneration is identical on any date; daily versus seasonal camp catch-up matches; neighbouring ground meets within about 0.5 m; rivers never break.

- `WLD-13` **Looking changes nothing** *(Decided)*: Where you look, and how fast time runs, never change what happens; unvisited places are drawn from the seed and their world cell, as people would find them (`PRN-10`).
  - **How it works:** the picture and sound read the world, and world rules never read the camera, the zoom, the speed or whether an area is made (`WLD-12`); "near" always means near a person.
    Counted herds are drawn as that many animals, placed from the seed.
  - **Check:** the same saved world gives exactly the same results whatever the camera and the speed (`TIM-16`).

### 6.3 Making a world

- `WLD-08` **Realistic, not from scratch** *(Decided)*: Each world is made directly in a believable present-day state by fast rules that imitate what deep time would make (`SCP-12`): the stages of `WLD-09`, in order from the seed, make the world and weather cells, never areas (`WLD-12`).
  - **Repeatable:** a seed makes the same world and areas only under the same rules for making them and the same values that place and size land, plants and stones; changing these is a big update (`PLT-09`), while tuning yields, timings or chances is not.
  - **Settling:** once you pick a world, its plant cover, water and herds run for 10 years by the play rules, with no people; animals near the start region begin as wary as hunted ones (`WLD-32`); then history begins (`TIM-14`).
  - **Done when:** in 20 worlds, 95% of rivers draining 50 km² or more reach sea or lake; median slope 0.5–5°, under 1% over 30°; no coast straight for over about 20 km; lakes 1–3% of land; you review them (`MIL-14`).

- `WLD-09` **What generation makes** *(Decided)*: Generate in causal order:
  1. **Plates:** about 6–12, oceanic/continental, setting land share (`WLD-06`); Earth-like ranges, volcano lines, rifts and faults at edges.
  2. **Rock:** about 12 kinds: granite, basalt, lava/ash, obsidian-bearing glassy lava, sandstone, shale, limestone, chalk, quartzite, slate, gravel and silt.
     Each cell has surface rock and up to two underlying layers, determined by plates: worn old land, old sea basins, folded ranges, volcanoes, rifts or sea floor.
  3. **Erosion:** rough-climate rain forms rivers cutting deepest under high flow, steep slopes and soft rock; hollows become lakes, all rivers reaching sea/lake.
     Add floodplains on large rivers, mountain-foot fans and mouth deltas.
     Caves occur in limestone, old lava tubes and soft rock under hard; cells record cave count/sizes, areas their shape (`WLD-12`).
  4. **Climate** (`WLD-16`).
  5. **Soils** (`WLD-27`).
  6. **Deposits** (`WLD-14`).
  7. **Biomes/cover:** regrowth stages since fire/flood (`WLD-31`).
  8. **Animals:** suitable food/cover at ecology limits (`WLD-18`), with seasonal herd ranges (`WLD-32`).
  - **Done when:** in 20 worlds, ranges, volcanoes and faults follow plate edges; chalk/limestone occur only in former seas/basins.

- `WLD-14` **Deposits placed by geology** *(Decided)*: Useful stone, clay, ochre and ore lie where the rocks and rivers put them.
  - **Where each lies:** flint as nodules in chalk, and chert in some limestones; obsidian in young lava from volcanoes with thick, sticky lava; hammer and grinding stones where quartzite, basalt or sandstone shows; clay along rivers, in old river bends and lake beds, and where granite and similar rock rots; red and yellow ochre where iron-rich rock weathers; green and blue copper ore at the weathered tops of copper-bearing rock near granite in volcanic ranges, and rarely, only in those ore cells, a little native copper.
  - **How it works:** each cell records its deposits and how rich each is; rivers carry stones downstream, rounded with distance, so flint turns up in gravels far from its chalk.
    Deposits show where the land is cut, such as banks, cliffs and cave walls, and look like what they are, while their uses are learned (`MAT-03`); deeper ones need digging (`MAT-06`).
    In areas, loose stones lie as ground, per patch like ground cover (`WLD-31`); one someone takes becomes a thing, its quality drawn from the seed (`MAT-20`).
  - **Done when:** in 100 worlds, flint lies only in chalk and gravel traced below it, chert only in eligible limestone and gravel traced below it, obsidian only near young volcanoes, and copper ore only in volcanic ranges near granite. Flint and chert both count as stone that flakes, keeping their precise names.

- `WLD-10` **Generate several, offer the best three** *(Decided)*: Generate, score and never edit candidates.
  "New world" offers the best three small globes with one-line summaries; pick one, let the game pick or enter a seed.
  - **Two passes:** about 20 candidates, adjusted to `WLD-11`, receive plates, rock, erosion, rough climate/biomes and deposits; fully generate and rescore the best few.
  - **Required:** a qualifying start (`WLD-24`); its landmass holds flaking stone, clay, wild grain, wolves, a domestic-capable herd species and copper ore (`WLD-33`, `TIM-19`).
  - **Weighted score:** varied land/climates, separation barriers, uneven resources and start-region score.
  - **Too few:** try up to about 40 candidates; offer qualifying ones even if fewer than three.
    A supplied seed generates one world and uses the same start search, reporting no start if none qualifies.
  - **Done when:** across 100 seeds, at least 1/4 candidates qualifies; all offers meet requirements; a seed always offers the same three.

- `WLD-24` **Where history begins** *(Decided)*: The bands start in a temperate region found by scoring, never placed by hand.
  - **How it works:** a region is judged before settling by what settling doesn't change (climate, biome, soil, caves, water and stone, with food estimated from biome and soil), and is kept through settling (`WLD-08`).
    It qualifies when:
    - **winters** matter, the coldest season averaging about 2–10 °C with frost on a few nights, yet naked, fireless people live through them huddled in caves (`BIO-11`);
    - the 3–4 bands each have a dry **cave** or overhang for their 15–30 people, initially estimated at 2 m² of usable dry floor per person, and **water** that lasts all year within about 2 km;
    - **food** the bands can get with the starting kit (`BIO-02`) is enough within about 10 km of the shelters, in every season with at least a 20% estimated margin, from several kinds, without counting food twice where ranges overlap; actual yields and survival must confirm these estimates when complete living settling and bands arrive (`MIL-15`);
    - **stone that flakes** lies within the same reach, for the sharp-stone test (`RES-02`).
  - **Ranking:** bigger food margins, more kinds of food, and more shelters and water score higher.
  - **Done when:** in 100 worlds, every start region still qualifies after settling.

- `WLD-11` **Generation time** *(To test)*: From "New world" to three globes takes at most about 3 minutes at the phone's held speed, and settling the chosen world, with its bands made (`BIO-03`), at most about 1 minute more.
  The targets are 180 and 60 seconds; outer limits are 198 and 66 seconds. Count fallback search, qualification, previews and entry saving. Measure each implemented phase (`MIL-14`) and repeat with ecology and bands when available; time spent reading choices cannot hide settling.
  - **Done when:** both times are met on the phone once complete generation, settling and bands exist (`MIL-15`) (`PLT-04`).

### 6.4 Sky, climate and weather

- `WLD-06` **Varied within reason** *(Decided)*: Each world has its own axial tilt (15° to 30°, which sets how strong its seasons are), share of land (25% to 50%), continents, seas and stars, all from the seed; every world has one moon, a 24-hour day and the 60-day year (`TIM-18`), and Earth's gravity, air and water.
  - **Done when:** over 100 seeds, tilt and share of land always fall within their ranges and spread across them.

- `WLD-07` **The sky** *(Decided)*: Sun, moon and stars move as they would for the world's tilt and the 60-day year (`TIM-18`): daylight by latitude and date, a moon that waxes and wanes once a season, bright enough at full to walk and hunt by, and a plausible distribution of eclipses; people can learn these cycles (`CUL-29`).
  - **Owner OK:** 8 October 2026.
    Keep a physically coherent sky; review a plausible eclipse distribution when the wider-world sky is built (`MIL-14`).
  - **Done when:** in a world, each season has one full moon, daylight follows Earth's for each latitude and date, and the reviewed eclipse distribution is plausible.
    Count appreciable partial or total alignments above the geometric horizon and report cloud obscuration separately; total solar eclipses are not promised everywhere.

- `WLD-16` **Climate and weather** *(Decided)*: Each place has a climate that never changes (`SCP-21`), and each weather cell its weather every hour: temperature, humidity, wind, cloud, rain and snow.
  - **Climate,** worked out at generation by rules modelled on Earth's, from latitude, height (about 6 °C colder each 1,000 m up), wind belts, sea, mountains with their rain shadows, and currents (`WLD-26`); its record holds, for each place and season, the usual warmth and its extremes, rain, snow, wind and storm days, and its longest dry spell (`GOD-02`).
  - **How it works:**
    - **Seasons** come from latitude and tilt (`WLD-06`), squeezed into the 60-day year, land and sea warming and cooling within it (`TIM-18`); deserts swing most between day and night, coasts least.
      Latitude changes about a degree every 5.6 km, so climate belts are about 100 km wide.
    - **Storms** decide when and where rain and snow fall, not how much: born upwind at rates set so each place gets about its climate's storm days, they move with the winds and drop on each place what its climate gives for a storm of their size.
    - **Thunderstorms** build in warm, moist air and bring lightning (`WLD-28`).
    - **On the ground:** snow lies on each cell until it melts into the rivers (`WLD-17`); each place's weather is adjusted for its height, slope and shelter, with frost pooling in hollows.
    - **Good and bad years:** chance brings runs of wet, dry, warm and cold years as often as `WLD-30` sets, giving droughts and harsh winters (`WLD-22`).
  - **Done when:** in 20 worlds, each band of latitude has about Earth's climates in Earth's shares, and each place's weather over 20 years averages within 10% of its climate's rain and 1 °C of its warmth, which is within about 2 °C of Earth's at the same latitude, height and distance from the sea.
    Compare climate shares in 10° latitude bands, at most 10 percentage points apart in sufficiently sampled bands, matching altitude and maritime/current exposure with declared weighting and scaled distances; use an Earth-tilt case and 15°/30° cases. A dry day has less than 1 mm rain; the saved longest-dry-spell envelope is the 95th percentile of annual maxima, with its exceedance share and storm-day shares checked by `RES-13`. Freeze reference data and event thresholds before tuning.

### 6.5 Water and soil

- `WLD-17` **Fresh water** *(Decided)*: Rivers, lakes, springs, marshes, snow and ice, with floods and dry spells; life gathers around them (`BIO-09`).
  - **How it works:**
    - **Each cell's water:** rain and melt soak in or run off (`WLD-27`), and ground water feeds springs that keep streams flowing in dry spells.
    - **Rivers** keep their courses, each with its line, width and depth (`WLD-12`); water runs down them at real speed, so a storm upstream floods the lower valley later; in a dry spell springs, streams and lakes fall at `TIM-18`'s pace, and small streams dry up.
    - **Lakes** rise and fall with what flows in and out; marshes form on badly drained ground, with reeds and peat.
    - **Floods** from heavy rain or fast melt spread over the floodplain, drowning and carrying things, and leave silt (`MAT-08`).
    - **Ice** on rivers and lakes bears a person only once thick enough, and glaciers stay as generated.
    - **Fouled water** is running water within about 1 km downstream of a camp, a herd's crossing or a carcass in it, while they are there and a few days after, and still pools under about 50 m across in warm weather; it carries illness (`BIO-05`), while springs and water above any camp are clean.
  - **Done when:** in scenes, a storm upstream raises the lower valley about a day later, a spring-fed stream outlasts a dry spell, and drinkers below a camp get gut sickness at `BIO-05`'s chance for fouled water, those at the spring don't.

- `WLD-26` **Seas** *(Decided)*: Seas and oceans with currents that carry warmth, one fixed level with no tides (`SCP-21`), and shores rich in food.
  - **How it works:** currents set at generation warm one side of each ocean and chill the other, and rich water wells up off some coasts; each sea cell keeps its warmth, ice, fish and sea mammals (`WLD-32`), most where water wells up and in shallow seas.
    Shores are beaches, rocks, estuaries and salt marsh, with shellfish beds kept per patch like ground cover (`WLD-31`); cold seas freeze in winter, in places thick enough to walk on.
  - **Done when:** in worlds, warm and cold coasts lie on opposite sides of each ocean, fish are richest where cold water wells up, and stripped shellfish beds regrow within a season.

- `WLD-27` **Soils** *(Decided)*: Each world cell has a soil and a fertility from 0 to 5, which decide what grows there and how well.
  - **How it works:** soil comes from what lies beneath, the climate, the slope and the plants: rich on river silt, wind-blown dust, volcanic ash and old grassland; poor on sand, steep slopes and peat, and where heavy rain in hot lands washes it out.
    Its kind sets how it holds water and how easily it is dug.
    Fertility falls with each harvest carried away, recovers over a few years of rest, and rises fast with ash, dung, rotted waste and flood silt; fields keep their own in their area (`WLD-12`).
  - **Done when:** in a scene, a field cropped every year yields less each year, recovers after a few years' rest, and rises after ash or dung.

### 6.6 Plants and animals

- `WLD-31` **Plants** *(Decided)*: About 60 Earth species or close kin, with real habitats, habits, seasons and sizes: trees, bushes, grasses/wild grains, herbs, roots, reeds, flowers, mushrooms and tinder fungi.
  Each defines growth, yields (fruit, nuts, seeds, wood, bark, fibre, leaves, roots), characteristics and scaling (`MAT-03`, `WLD-30`).
  - **Areas:** individual trees/bushes have growth (seedling, young, grown, old, dead), season (bud, leaf, flower, fruit, bare) and ripening yields.
    Cover patches a few metres across hold grasses, herbs, flowers, reeds, mushrooms and seedlings, their species, density, season and ripe yield.
    Unchanged plants follow cell cover/date (`WLD-12`); changed plants grow, fruit, spread and die independently at cell rates.
    Gathering removes ripe yield as things with seed-derived size/quality, identical whoever gathers; plants remain stripped unless felled/uprooted.
  - **Life:** species set their seasonal timing; grass/herbs grow within a season, bushes over years, trees over decades (`TIM-18`).
    Drought, frost, fire, shade, grazing and age kill; dead wood fuels and rots (`MAT-19`).
  - **Spread:** each season, chance starts nearby same-species plants where ground, light and room permit; about 5,000 individual plants per area maximum; dropped seeds may sprout (`RCK-23`).
  - **Cells:** tree, bush, grass/herb, reed and bare-ground shares, each with up to about four suitable species/shares; tree age and date-based ripe yield.
    Cover regrows toward its biome at species rates after fire, flood and grazing.
  - **Biomes:** climate/soil/wetness determine ice, tundra, northern conifer forest, broadleaf forest, grassland, dry scrub, desert, savanna, tropical forest, marsh, mountain heights, shores and seas.
    Each land biome except ice has at least 6 catalogue plants and 4 animals; seas/shores at least 4 animals; species may serve several biomes.
  - **Done when:** area species/shares match cells; stripped hazel fruits next autumn; burned oak becomes young wood within about 10 years; kept growth is within 10% of cell rates; each present biome in 20 worlds has plant eaters, hunters, birds and fish.

- `WLD-32` **Animals** *(Decided)*: About 30 wild Earth species or close kin, without insects: mammals, birds, fish, shellfish and a few reptiles; 5 domestic kinds count separately (`WLD-33`).
  Include a woolly mammoth in cold open grassland under ordinary food/density rules; add no ice-age/extinction system.
  Each defines diet, group size, speed, danger, lifespan, breeding/moulting/migration/fish-run/winter-sleep seasons and yields (`MAT-10`), including seasonal nesting eggs and herd dung.
  - **Far away:** big animals are herd, pack, flock or solitary counts of adults/young, with condition and wariness.
    Each has seasonal cell ranges and summer/winter migration ranges; daily route activities seek food/water/cover and avoid hunters (`TIM-17`).
  - **Numbers:** scale under `WLD-30`; seasonal young, births and deaths follow `WLD-18`.
  - **Near people:** within about 1 km, convert at existing positions to bodies/simple minds, with age, sex, condition and illness drawn from the count (`BIO-19`, `MND-16`).
    Rejoin counts after a day without nearby people; wounds lower herd condition, fright raises wariness.
    Tame (tameness ≥1) and kept animals remain individuals.
  - **Small animals:** hares, small birds and most fish remain cell/water-stretch counts; catching removes one (`MAT-11`).
  - **Fear:** hunting/chasing increases fleeing distance; years without hunters reduce wariness.
  - **Danger:** hungry, cornered or young-guarding wolves, bears and big cats may attack; numbers, noise and fire deter them.
  - **Done when:** autumn-hunted herds flee at twice unhunted distance within 5 years; count conversion preserves totals; migrants arrive in season.

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
  - **Done when:** in 20 worlds run 100 years without people, each species stays within half to twice its settled total, none vanishes from its biomes, and big hunters are about 1 to 50–200 prey.

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
  - **Done when:** in worlds, grassland burns every 2–5 years, dry forest every 20–50, wet forest rarely; in a scene, a fire stops at a river, and clearing around a camp saves it.

- `WLD-15` **Quakes and eruptions** *(Decided)*: Rare earthquakes strike along the faults made at generation, and volcanoes erupt where geology allows; nothing else reshapes the land (`SCP-21`).
  - **How it works:**
    - **How often:** each fault and volcano has its own chance each year, set by its kind, at about Earth's yearly rate for the same area (`WLD-30`); after each, it stays quiet for a time set by its kind, usually many years.
    - **Quakes** shake hardest near the fault: shelters and stacked things fall (`MAT-11`), and rocks drop from cliffs and cave roofs.
    - **Eruptions** give warning days ahead (small quakes, rumbling, warm springs) that people can notice (`BIO-18`); lava then burns and buries what lies in its path, leaving fresh lava rock, heights unchanged and no new obsidian, and ash smothers plants and fouls water nearby for a season, later enriching the soil (`WLD-27`).
  - **Done when:** in 20 worlds of 100 years, quakes come only on faults and eruptions only at volcanoes, about as often as their kinds set, each followed by its quiet time.

- `WLD-22` **Natural events** *(Decided)*: Lightning, wildfires, storms, droughts, floods and harsh winters, and rarely quakes and eruptions, come from the world's own systems (`WLD-15`, `WLD-16`, `WLD-17`, `WLD-28`), never on a schedule; your powers work through the same systems (`GOD-05`).
  - **How it works:** big events that touch people enter the book of ages (`PRE-05`).
  - **Check:** in each climate, each kind of event comes about as often per game year as on Earth per year (`WLD-30`), and every event traces back to its system.

## 7. Things and blueprints

Everything people make on purpose is made by a blueprint; things also change by themselves, through fire, timers and the living world.
Discovery, practice and teaching are in Minds (`MND-06`, `MND-11`, `MND-13`).

### 7.1 Things

- `MAT-01` **Things are made of materials** *(Decided)*: Every thing has one or more materials: a spear is hazel, flint and sinew.
  - **How it works:** a material has a colour, base values for the 18 characteristics (`MAT-03`), and one of nine classes, setting its sounds (`SND-06`), whether it rots (`MAT-19`) and how long it lasts buried (`MAT-08`): stone (with ore), earth (clay, sand, ochre, ash), wood (with charcoal), plant (with dung), bone (with antler, horn, teeth, shell), hide (with fur, feathers, sinew), flesh (with fat, eggs, milk), metal and water.
  - **Done when:** the Complete check passes (`MAT-17`).

- `MAT-10` **Items** *(Decided)*: An item is a kind of thing in the catalogue, such as flint, sharp flake or sewn cloak: about 190 at launch.
  - **How it works:**
    - **About 90 raw,** each with its 18 values, class, size and a look drawn from its form's shared shape (`PRE-46`); species that yield alike share them.
    - **About 100 made,** each with its form, main material, changes (`MAT-03`), what it breaks into, and its drawing and icon (`MAT-21`).
    - **Also listed:** foods that taste mild despite their poison (`MND-21`), and how much of the body a garment covers (`BIO-11`).
    - **A thing** is one item in an area (`WLD-12`), with its own size, state (no new item, `MAT-19`), wear, quality (`MAT-20`), maker, date and style (`PRE-43`); a heap of small things is one thing.
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
    - **Effect:** for a made thing, at 0–1 the main characteristic its blueprint names is a step lower and it wears twice as fast; at 4–5 it is a step higher, wears half as fast and is prized (`MND-24`, `CUL-21`); quality never changes food or water, but fine food gives a better thought.
      A raw thing's quality moves none of its values and counts only as a fine or poor input.
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
  - **Plain uses:** gather, dig, throw, feed, stack, heat, soak, dry and apply also work without a blueprint, to take, move, wet, warm or dry things, can start timers, and leave heaps, holes such as a pit or grave, and cleared ground; anything else an action makes or changes, such as a frame, a sown plot or a house, needs a blueprint, and everyday activities, such as walking or fighting, are not base actions.
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
    - **Small game** (counts, `WLD-32`): at hunting experience 4, where its kind is at its usual numbers, a set snare or trap takes one a night 1/10 of the time, a baited hook one an hour 1/20, and a throw hits 1/4 (tuned), in proportion to how many live there, each level above or below and fine or poor quality moving it by a tenth of itself; setting is stack or bind, and emptying is gather.
    - **Toppling:** stacks and buildings stand until strong wind, heavy snow or a quake knocks them down, poorly made ones first (`WLD-22`).
  - **Done when:** in scenes, a pot dropped from waist height breaks in at least 15/20 runs, a thrown spear wounds within its catalogue range, and snares catch at their stated chance.

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
    - **Smoking** (in thick smoke from green wood at heat 1–2): about 2 days; smoked or dried meat then keeps about 15 game days in summer, as about three months do in life, and through the winter in the cold (`RCK-14`).
    - **Soaking:** crushed acorns lose their bitterness in about 2 days in running water, 6 in still; a hide with crushed bitter bark becomes leather in about 10 days (`RCK-06`).
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

- `MAT-04` **Blueprints** *(Decided)*: One action on fitting things under fitting conditions, at the maker's sector experience, produces a result.
  - **Fields:**
    1. **Result:** thing, state, ground/body change; amount, quality-sensitive main characteristic (`MAT-20`), input-to-part mapping (`PRE-42`) and leftovers.
    2. **Action:** exactly one base action (`MAT-06`); longer chains leave named intermediate states (`MAT-22`).
    3. **Inputs:** roles (worked thing, tool, binding, fuel, container, body, ground), required classes/forms, characteristic ranges, sizes/amounts, never item names; consumption or retention and retained-tool wear.
    4. **Place:** required heat, water, shelter or growing season (`MAT-18`).
    5. **Sector/difficulty:** one sector (`MND-06`), difficulty 1–10.
    6. **Time:** repeated-try activity (`TIM-17`); each tool-main-characteristic step above the minimum shortens tries a tenth, at most a third; then any timer and required tending (`MAT-19`).
       Result lands when the last part ends.
    7. **Chance:** difficulty versus level; unknown-blueprint discovery factor, 1 unless tuned (`MND-11`).
    8. **Failures:** rates per hour's tries of lost time, spoiled inputs, quality-0 results or hurt (`BIO-13`).
    9. **Hinted by:** plain-use, blueprint, timer or own-failure signs giving hunches.
  - **Fit:** match each input to held, worked or supporting things (anvil, hearth board, ground), by item/material/state/size.
    Wear/quality affect only made things' main characteristic.
    Place conditions apply equally to accidents, experiments and intended work; choices/experiments use only reachable things (`MND-09`).
  - **Chance:** level = average skill and sector experience, fractions allowed.
    Equal difficulty gives 50%; each level difference changes it 10 percentage points, bounded 5–95%.
    Fine inputs (quality 4–5) add a tenth, poor (0–1) subtract a tenth.
    Each of rest <20, clumsy cold (`BIO-11`), darkness beyond firelight and an arm below half health subtracts a tenth and slows work a quarter; pain/illness follow their own rules (`BIO-13`, `BIO-05`).
  - **Unknown:** each fitting unknown blueprint rolls once per activity, at the doer's maker chance times route factor; noticed success teaches skill 1 (`MND-10`).
    A learner beside a teacher has full chance (`MND-13`).
    The intended blueprint settles normally; each other fit rolls separately.
  - **Shared work:** structures, plots and kiln firings require set person-hours, together or in turns; settle at the most skilled worker's level, crediting that worker with discovery (`MAT-21`).
  - **Example:** sharp flake: strike a consumed core (hardness 4–5, flaking 3–5, 8–30 cm) with a retained striker (hardness/toughness 3–5, ≥6 cm), either moving.
    Stone, difficulty 2, about half a minute per try; flake 3–8 cm retains core values except edge = flaking and toughness 1.
    Failures: crumbs 9/10, shattered core 1/10 (chunks ≥8 cm remain cores), plus cut hand 1/100.
    Sharp crumbs hint it; unknown initially (`BIO-20`); flint nut-cracking on a stone anvil fits it.
  - **Done when:** every launch blueprint's trial successes and times match its fields (`RES-24`).

- `MAT-07` **Several routes** *(Decided)*: One named result can come from different materials or from different blueprints, so peoples reach the same things by different paths.
  - **How it works:** routes differ in what they need and how they look (`PRE-42`): an ember comes by drilling or by ploughing a stick along a groove; which a people finds first depends on what lies around it (`WLD-14`) and on chance.
  - **Done when:** across the pace tests (`RES-07`), at least two routes each to fire and to huts appear.

- `MAT-22` **Chains** *(Decided)*: Results feed other blueprints, so most things take a chain of one-action steps, each discovered or learned on its own and made for what the chain's end does for people (`MND-09`).
  A people can stall at any step, or get round it by another route (`MAT-07`).
  - **Example:** a flake (strike) makes a scraper (press); a fresh hide scraped (scrape) and dried on a frame (dry) is bedding; greased (apply) and rubbed soft (press), a wrap; cut (cut), pierced with an awl (drill) and sewn with sinew thread (bind), a cloak, warmth 3, or 5 with the fur on (`RCK-26`).
  - **Done when:** the chain runs end to end in a scene (`RES-23`), and a band that knows every step but the scraper stalls there or finds another route.

- `MAT-21` **Named discoveries** *(Decided)*: Every named result has a name, an icon and its action's sound (`SND-06`); a new thing also has a drawing (`PRE-46`), and a change shows on what it changes (`PRE-42`).
  A people's first noticed success (`MND-10`) at making one is a named discovery, recorded in the book of ages.
  - **How it works:** each result has an English name and each people's own word, coined at its first success (`CUL-18`).
    The entry gives who, when, where, by which route (`MND-11`), from what, and the word (`PRE-05`); a timer's unmeant result counts too, for whoever notices it first (`MOM-12`).
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

- `MAT-13` **The catalogues** *(Decided)*: The game's content is in catalogues agent-written and automatically tested: items (`MAT-10`), blueprints (`MAT-23`), plants (`WLD-31`), animals (`WLD-32`), illnesses (`BIO-05`), pattern sentences (`PRE-37`), and other sections' prepared lists, such as thoughts (`MND-29`) and the culture lists (`CUL-07`).
  - **How it works:** each entry stands alone, in plain words, with its values and checks (`MAT-17`), naming others only as results, never as inputs (`PRN-07`); every rule in them is physical or biological (`SCP-19`), except in the culture lists.
    Hints too name actions, signs and kinds of thing, never entries.
  - **Check:** no entry names another as an input.

- `MAT-14` **Adding without rewriting** *(Decided)*: Adding an item, blueprint, plant, animal or illness never needs the others changed (`PRN-14`): a new item fits every blueprint whose ranges it meets.
  - **Check:** a made-up item added for testing works in every blueprint it fits, with no other entry changed.

- `MAT-16` **The catalogue grows by milestone** *(Decided)*: Each milestone adds only the entries its steps need (`SCP-16`), never rewriting earlier ones, and later layers grow the same way (`VIS-03`).

- `MAT-17` **How the catalogue checks work** *(Decided)*: Automated checks keep the catalogues complete and believable; each added or changed entry comes with its own checks, and all must pass before joining main (`PRC-10`).
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
  - **Check:** every food plant lists its growing season and ground (`WLD-31`); in a scene, seed thrown on a damp heap in season sprouts in at least 15/20 runs, and a tended plot yields at least twice an untended one.

- `RCK-24` **Young animals grow tame** *(Decided)*: Young animals raised and fed by people grow tame; adults rarely do.
  - **Check:** in a scene, wolf pups fed daily from their first days reach tameness 5 within a season (`WLD-33`), while grown wolves fed for a season stay at 2 or below in at least 15/20 runs.

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
    - **Children** know a share of this by age, and learn the rest as they grow.
  - **Done when:** in a new world every adult knows exactly this, and nothing more (`MND-02`).

- `BIO-08` **Everyone is different** *(Decided)*: Individual height, build, strength, stamina, resistance, sight, hearing and learning speed.
  - **Values:** inheritance, sex, age and chance (`BIO-06`, `BIO-17`, `BIO-16`), centred on human averages, two thirds within a fifth.
    Condition <30 for at least one cumulative year before 14 permanently reduces height/strength about a tenth (`BIO-09`).
  - **Done when:** founder spreads match; year-long childhood hunger produces about tenth-shorter/weaker adults.

### 8.2 Staying alive

- `BIO-09` **Needs of the body** *(Decided)*: Hunger, thirst, warmth (`BIO-11`) and rest are mental needs (`MND-07`) with bodily consequences.
  - **Hunger:** about a day's food/day (`BIO-10`), scaled to body size; hard work adds up to half, cold a quarter, growing children a fifth, pregnancy/nursing their own amounts (`BIO-15`).
  - **Condition:** reserve 0 (starved)–100 (padded); no food loses about 5/day; excess food adds about 5 per day's food, capped at half a day's excess/day.
    Well-fed adults starve in about 20 days.
    Below about 30: strength/stamina fall a fifth, healing/resistance worsen, conception stops; 0 kills (`BIO-05`, `BIO-13`, `BIO-14`).
  - **Thirst:** adults about 3 litres/day, up to twice in heat/hard work, drink and food counted; lacking water weakens within a day, kills in about three; salt water worsens it (`WLD-26`).
  - **Rest:** adults about 8 hours, children 10, babies most of the day; cold/hunger/pain disrupt sleep; after about 1.5 days awake, sleep wherever they are.
  - **Done when:** fed adults without food die in about 20 days, without water about 3; adequate food/water/shelter preserve band condition over a mild year.

- `BIO-10` **Food** *(Decided)*: Nourishment follows food value; variety preserves strength; prolonged single-group eating weakens.
  - **Amounts:** each food-value step about doubles nourishment/kg (`MAT-03`); adult daily food: about 4 kg at 2 (berries/raw roots), 2 kg at 3 (raw meat), 1 kg at 4 (nuts/cooked meat), 0.5 kg at 5 (fat).
    Water/kg = about 0.2 litre per water point, berries about 0.8 litre.
  - **Groups:** meat (flesh, fat, fish, eggs, milk); fruit/greens (fruit, leaves, shoots, mushrooms); nuts/seeds including grain; roots.
    Made food inherits its main input's group (`WLD-31`, `WLD-32`).
  - **Variety:** only one group for 10 days cuts strength, stamina and resistance about a fifth until wider eating resumes.
  - **Scurvy:** about 15 days without fresh fruit/greens causes bleeding gums and halted healing; dried/smoked/cooked do not count; a few fresh days cure it, a learnable true cause (`MND-05`).
  - **Done when:** single-group eaters weaken after 10 days; dried-food eaters get scurvy after 15 and recover on fresh greens.

- `BIO-11` **Heat and cold** *(Decided)*: Work, clothing, shelter, fire and huddling warm bodies; cold kills, heat exhausts.
  - **Felt temperature:** air (`WLD-16`), wind up to −10 °C, wet skin −5 °C, sun up to +5 °C.
    Naked resting comfort limit about 24 °C; work lowers it about 10 °C.
  - **Clothing:** each warmth point lowers limit about 6 °C × covered share; cloak/tunic about half, leggings quarter, shoes/hood eighth each (`MAT-03`).
  - **Shelter:** cuts wind/rain, adds wall warmth; caves add a few degrees.
    Bedding acts as sleeper clothing; huddling adds one warmth point all over; campfire adds about 15 °C within about 2 m (`MAT-18`).
  - **Cold:** below limit shiver; about 10 °C below become clumsy/slow; 20 °C below freeze, become confused, sleep and die within hours.
    Bare parts below about −5 °C felt for about 2 hours frostbite (`BIO-13`); children/elders chill faster.
  - **Heat:** above about 32 °C increases water need (`BIO-09`); hard work above about 40 °C risks fainting and lethal heatstroke.
  - **Done when:** cave-huddled naked band survives a 2 °C winter night; lone uncovered/fireless person at −5 °C frostbites or dies; warm clothes allow all-day outdoor work.

- `BIO-12` **Poison and medicine** *(Decided)*: Poison and medicine come from the characteristics of what is eaten or put on the body (`MAT-03`), and some things do both, in amounts that matter.
  - **How it works:**
    - **Poison** acts within hours, by its value and the amount eaten against the eater's size, like a short illness (`BIO-05`).
      A meal at poison 1–2 brings cramps and vomiting for a day; at 3 it makes people ill for days and kills about 1 child in 10; at 4 it kills about 1 adult in 3; and at 5 a mouthful kills most people.
    - **Medicine,** eaten or put on a wound, lowers pain by about 10 for each point for about 6 hours and helps the body against illness; on a wound it also lowers the chance of infection by a tenth for each point (`BIO-13`).
    - **Venom:** a snake's bite poisons by its kind (`WLD-32`).
  - **Done when:** in scenes, a mouthful of poison 5 kills an adult in most runs, poison-2 berries make the eater ill for a day, and medicine on wounds lowers their pain and infections.

### 8.3 Harm and healing

- `BIO-13` **Body parts and wounds** *(Decided)*: Head, torso, two arms and two legs, hands/feet included with limbs; each health 100–0.
  Wounds bleed, infect, heal, cripple or kill.
  - **Kinds/size:** each wound subtracts its size from its part's health until healed; cuts from edges/points, bruises from blows/falls, bites from teeth/horns/claws, burns from fire/hot things/lightning/freezing, broken bones from arm/leg blows or falls over about 25.
    Cause sets damage: strength and weapon weight/hardness/edge (`MAT-03`), animal species or heat.
    Knapping slips about 5, clubs/dog bites about 20, spear thrusts/bear swipes ≥40; lightning burns sized to kill about 1/10 struck.
  - **Hit location:** torso 40%, each arm 15%, each leg 12%, head 6%, unless the event specifies.
    Falls damage a leg about 10 per metre beyond the first; from about 5 m, apply equal damage again to torso and head.
  - **Bleeding:** cuts/bites lose about a tenth of size per minute, halving every 10 minutes.
    Blood starts 100; losing about 25 halves strength/stamina, 35 collapses, 50 kills; restore about 10/day.
    Pressure quarters flow and stops wounds <30 within about 10 minutes; dressing stops any wound (`BIO-23`).
  - **Pain:** sum wound sizes, cap 100; lowers mood (`MND-29`), slows work/walking by half pain as percent; every 20 subtracts a tenth from success (`MAT-04`).
  - **Disability:** broken leg hobbles at quarter speed; broken arm blocks two-handed work (`MAT-12`); limb health 0 is permanently useless.
    Head below half: dazed, half-speed work, no blueprints; torso below half: half strength/stamina.
    Head below quarter: unconscious about an hour; head/torso 0 kills (`BIO-14`).
  - **Infection:** first two days, cuts ≥10 risk about half size as percent; bites/burns ≥10 risk full size; smaller wounds heal clean.
    Washing halves risk, dressing halves again.
    Infection stops healing about 5 days until cleared or becoming wound fever (`BIO-05`).
  - **Healing:** shrink about 7/day, bruises 15; breaks about 10 days; rest, food, warmth and resistance accelerate, hunger/cold slow (`BIO-08`).
  - **Lasting:** wounds ≥30 scar; breaks crooked about 1/2, splinted 1/10, causing lifelong limp/weak arm.
  - **Done when:** untreated cut 40 kills, pressed does not; broken leg hobbles about 10 days; washing halves infections.

- `BIO-05` **Illness** *(Decided)*: About 15 illnesses, each with its routes, a time before it shows, a course, a danger and, for some, immunity; some need crowds, as in real history.
  - **How it works:**
    - **Catching:** a breath contact is a night under the same roof or by the same hearth as the sick, or an hour within about 3 m of them; a touch contact is tending them, sharing their bed or eating food they handled.
      Each contact passes it by the illness's own chance, from a day before the signs show until recovery; the immune don't catch it.
    - **Starting:** everyday illnesses arise in any group about as often per year as in a real year (`TIM-18`), breath ones most in winter, and animal ones from sick animals (`BIO-19`).
      Each illness's entry gives its start rate per year in a band of about 30, the share of its carriers that are sick, or its chance per raw meal (worms: about 1/100), as `MAT-17` checks.
    - **Crowd illnesses** start only in villages of at least about 200 people that keep herds (`CUL-28`).
      They burn out once most people have had them, and return to a village every 10–20 years, once enough children born since lack protection.
    - **While ill,** people work and walk at half speed with a fever and three quarters otherwise, and look ill.
    - **Who dies:** a deadly illness decides once, on its worst day, about a third of the way through its course, whether it kills, by its chance in the list.
      That chance is about 5 times higher for babies under 1 and people over 60, twice as high for each of hunger (condition under 30, `BIO-09`), real cold (10 °C or more below their limit, `BIO-11`) and wounds other than the one the illness came from (pain 20 or more, `BIO-13`), and a fifth higher or lower for each fifth of resistance below or above average (`BIO-08`); these multiply, to at most 9 in 10.
      Then daily care halves it, down to a quarter as the carer's healing experience nears 10 (`BIO-23`, `MND-06`), and each point of medicine taken that day takes off a tenth (`BIO-12`); whoever survives recovers at the end of the course.
  - **The illnesses** (routes; days before the signs show), each a catalogue entry with its signs (`MAT-13`), then how long it lasts and deaths among untreated healthy adults:
    - **Cold** (breath, touch; 1–2 days): about 5 days; harmless; immunity for a year.
    - **Coughing fever** (breath, touch; 1–3 days): about a week; kills 1/50; immunity for a few years.
    - **Chest fever** (not catching; after a cold or coughing fever in about 1/10 babies, old or hungry people and 1/100 others, and after freezing or near-drowning in 1/5): 7–10 days; kills 1/4.
    - **Gut sickness** (fouled water or food; within a day): 2–5 days; kills 1/100; fouled water (`WLD-17`) gives it at about 1/20 chance each day someone drinks it.
    - **Worms** (raw or undercooked meat and fish): condition falls by about 1 a day until medicine of 3 or more, taken for a few days, clears them; cooking prevents them.
    - **Wound fever** (about 1 infected wound in 3, or a birth; 1–3 days): 5–10 days; kills 1/3.
    - **Lockjaw** (about 1/20 cuts or bites of 20 or more from teeth, horn or a wooden point, or dirtied with earth or dung; 3–15 days): 10–20 days; kills 1/2.
    - **Foaming madness** (the bite of a mad wolf, dog or other meat-eater; 5–20 days): always kills, within days, whatever is done; mad animals lose their fear of people.
    - **Hunter's fever** (skinning sick small game or eating it raw; 2–5 days): about 2 weeks; kills 1/20; immunity for life.
    - **Herder's fever** (kept goats, sheep and cattle, through raw milk and helping births; 5–15 days): 15–30 days; kills 1/50; immunity for life.
    - **Sore eyes** (touch; worst in crowded, smoky camps; 2–5 days): about 10 days; from the third bout, each has about 1/5 chance to scar an eye, which takes a quarter off sight (`BIO-18`).
    - **Spotted fever** (crowd; breath; about 10 days): about 10 days; kills 1/10; immunity for life.
    - **Pox** (crowd; breath, touch; about 12 days): about 15 days, leaving scars; kills 3 in 10; immunity for life.
    - **Bloody flux** (crowd; water fouled by a village's waste; 1–3 days): about a week; kills 1/10.
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
    - **Limits:** a person dies only when the body passes one: condition, water or blood running out (`BIO-09`, `BIO-13`), freezing or heatstroke (`BIO-11`), a head or torso at 0, a deadly illness or poison (`BIO-05`, `BIO-12`), too long under water (`BIO-21`), or an old body giving out (`BIO-16`).
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

- `BIO-15` **Pregnancy and birth** *(Decided)*: Couples have children through about 45 days' pregnancy, risky birth and years of nursing.
  Pairing rules are cultural (`CUL-27`); pairing/conception are unseen; no sexual violence.
  - **Conception:** women about 18–38 with male partners, condition ≥30 and not nursing, conceive within about 30 days on average; half rate from about 33 (`BIO-16`, `BIO-09`).
  - **Pregnancy:** about 45 days, food +1/5, slower movement in last 15; about 1/6 ends early, more with hunger, illness and age.
  - **Birth:** hours; hard about 1/10, more for first births and young/old/small/hungry mothers; about 1/5 hard births kill baby.
    About 1/100 births kill mother, half bleeding and half wound fever (`BIO-13`, `BIO-05`).
    These include a helper; alone double risks; helper healing experience approaching 10 halves them (`BIO-23`).
    Twins about 1/80.
  - **Nursing:** milk alone about half a year, then soft food plus milk until weaning about 3; mother food +1/4; hunger reduces milk.
    Band births about 4 years apart; porridge/animal milk may shorten spacing (`CUL-28`, `WLD-33`).
    Orphaned babies require another nursing woman; older ones can live on soft food.
  - **Record:** mother, father, birth date/place (`PRE-10`).
  - **Done when:** band births about 4 years apart; maternal deaths about 1/100; young milkless baby dies unless another mother nurses.

- `BIO-16` **Ageing** *(Decided)*: From about 45, bodies slowly weaken, heal more slowly and fight illness worse.
  Age itself takes no knowledge or skill (only disuse fades them, `MND-06`), though the old learn more slowly.
  - **How it works:**
    - **Decline:** strength, stamina, resistance, sight and hearing fall by about 2% a year (`BIO-08`).
    - **Fertility:** a woman's falls from about 33 and ends at about 38 (`BIO-15`).
    - **Giving out:** from about 55 the body itself can give out, at a chance of about 1/50 a year that doubles every 7 years or so.
  - **Done when:** in scenes, the old heal and recover from illness more slowly than the young.

### 8.5 The sexes

- `BIO-17` **Real biology, culture decides** *(Decided)*: Men and women differ only in real body ways: pregnancy and nursing, and on average size, strength and body fat, with wide overlap.
  Who hunts, gathers, leads or makes things is up to each culture (`CUL-06`), and minds don't differ by sex (`MND-20`).
  - **How it works:**
    - **Bodies:** on average men are about 7% taller and a third stronger, most in the arms, and women carry more body fat.
    - **Attraction:** inborn: most are drawn to partners of the other sex, about 1/20 to their own or to either (`MND-33`).
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
    - **Running and fleeing:** about three times walking speed; running, fighting and heavy work wind people within minutes, and a few minutes' rest restores them.
    - **Fighting,** including a spear thrust at an animal: blows land by chance, by strength and hunting experience (`MND-06`), and wound a part; most fights end when one is hurt, flees or gives up (`MND-33`).
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
  - **How it works:** the hot-tempered rage (shouting, smashing, attacking whoever angered them, `MND-33`); the proud and the brave run off alone for at least a day, to return, join another band or die in the cold; the rest despair, lying still and refusing work and food for at least a day.
  - **Done when:** in a starving-winter scene of 30, at least 2 break down in 16/20 runs; in a good season, at most 1/50 does.

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

- `MND-04` **Knowing things** *(Decided)*: For each kind of item met (`MAT-10`), people know its visible characteristics, such as size, colour and edge, at a glance, and hidden ones, such as poison, fuel and flaking, only by use or by being told (`MND-27`), each with certainty (`MAT-03`).
  - **By use:** eating shows at once how filling a food is, but poison and medicine only by linking what follows (`MND-05`), so people can be wrong; burning shows fuel, striking flaking, wearing warmth, holding water waterproofing.
  - **Look-alikes:** an untried kind is taken for the known kind it looks most like, so a poisonous berry like a safe one is eaten until sickness teaches otherwise.
  - **Done when:** people who burn a new wood learn its fuel value, and a band that knows flint uses chert untaught.

- `MND-28` **Mental map** *(Decided)*: What each person knows of places, by season, from going there and from talk (`CUL-24`).
  - **A place** is a spot worth knowing (a spring, a flint outcrop, a grove, a cave, a camp), at most one to an area (`WLD-12`), or a stretch known roughly (a valley, a ridge), one or more world cells; when full (`MND-14`), the least used fade.
    Each holds up to about eight facts ("hazelnuts here in autumn"), with when last seen and certainty, so old facts can be wrong.
  - **The way:** a direction and roughly how long it takes; the actual way, often a trodden path (`MAT-08`), is found only by walking.
  - **The year remembered:** how each season of the last two years went for them (hunger, cold, danger, what was plentiful where), expected to come again; elders' stories count too.
  - **Done when:** someone told of a spring goes there when thirsty and stops going once it is dry; berries eaten out are not sought until next season.

### 9.5 Beliefs

- `MND-27` **Beliefs** *(Decided)*: Something held true, with a strength from 0 (none) to 100 (certain) and a source: own experience, or a belief heard, which starts at or rises to the teller's strength times the hearer's trust in them out of 100 (`MND-24`), so trust 50 halves it.
  - **How it works:** of two that clash, the stronger guides choices until evidence settles it.
  - **Done when:** a warning about a cave keeps people away more often from a trusted elder than from a stranger.

- `MND-05` **Beliefs about causes** *(Decided)*: Strong outcomes link to unusual preceding causes; later outcomes strengthen/weaken links, producing true/false knowledge, taboos and rites.
  - **Outcome:** own or witnessed kin/friend hunt/find, hurt, illness, birth/death, storm/flood/fire, or end of hunger/cold/pain/illness/storms; size = worst thought, strong at |thought| ≥8 (`MND-29`).
    Need drops and own acts are not outcomes; surprises link only at their place.
  - **Cause:** activity + thing kind, place, person, sick person, animal or weather kind; count occurrence days over the past year.
  - **Explained:** test an existing present-cause link; form nothing new.
    Otherwise choose the most unusual cause in the preceding day or two: <about 1/10 past-year days, or absent in preceding ten days; first occurrence most unusual.
  - **Place surprise:** also link to usual acts/leavings or a past act there, such as discarded seeds and heap plants (`MOM-08`).
  - **Unseen:** with no sufficiently unusual cause, sky/land events (lightning, storm, flood, drought, quake), or sudden death, about 1/3 chance of a weak unseen-being belief, increased by spiritual personality/fear (`MND-31`).
    A lightning death can link both storm being and offending hill climb.
  - **Strength:** new link about 3 × thought size, cap 50; surprise link 20.
    Cause recurs: matching outcome within one/two days +15, absence −5; if outcome frequency >1/10 days, +10/−10 instead.
    Below 5 forget; among two explanatory links, only stronger gains.
  - **Blame:** person causes harm only if stranger/disliked (`MND-24`); sick/dying people link by state, teaching illness avoidance (`BIO-05`).
  - **Caps:** at most 3 links per outcome, within `MND-14`; discard weakest new/old.
  - **Cost:** pressing hunger can defeat food taboo, with shame; no ensuing harm weakens it (`MND-09`).
  - **Memory:** faded evidence leaves a reasonless belief, shown in mind details (`PRE-14`).
  - **Done when:** of 20 band runs, most adults reject real poison within a season in 16, fever-associated food in 6, credit improved spears in 12; nobody starves beside taboo food.

- `MND-31` **Beliefs about the unseen** *(Decided)*: Belief in beings nobody sees: spirits of places, animals and weather, and the dead.
  - **Start:** by the rule in `MND-05`, shaped by the event through the templates of `CUL-05` (the spirit of that hill, the storm, the bears, a dead grandmother).
  - **Growth:** a new being starts at about 20; each later event of its kind is put down to it and adds about 10, and each dream of it or rite held for it about 5; it never fades by time alone; an act linked to a good outcome of its kind (`MND-05`) is held to please it (`CUL-05`).
  - **Effect:** believers weigh what it is believed to want (offerings, rites, keeping off its places), and doing it brings a good thought and a sense of safety (`MND-29`).
  - **Room:** within the caps of `MND-14`, the weakest, new or old, is dropped.
  - **Done when:** where lightning kills a hunter on a hill, some witnesses believe in a storm being, and the band shuns the hill, each in at least 8/20 runs (`MOM-03`).

### 9.6 Choosing and planning

- `MND-09` **Choosing what to do** *(Decided)*: At activity end/interruption (`TIM-17`), score known options by needs, personality, plans and beliefs; usually pick the best.
  - **Options:** at most about 30, from known facts (`MND-02`): meeting needs, plan/ambition steps, reachable known blueprints, group plans/requests (`CUL-22`), social acts (`MND-33`), tending/feeding/playing with kept or nearby young animals, play, rest, exploration, experiments and dark acts only under `CUL-08`'s conditions.
  - **Reach:** carried, owned by self/family, shared band stores (`CUL-21`), within about 30 m, or for known blueprints, known input locations (`MND-28`).
    At most about eight blueprints, serving pressing needs/plans; social targets within about 20 m or one person sought.
  - **Score:** expected need benefit, urgency/personality weights, plans and ambition; add/subtract beliefs, social expectations and dream pull capped at a mild need (`MND-12`); subtract effort, time, distance and risk; scale by expected success.
  - **Worth:** direct need uses (eat, wear, burn, sleep under, store), or known tool/input use up to about three blueprint steps back, diminishing by step.
    Unknown use offers only curiosity; abundance reduces worth; kept animals gain worth after using their yields.
  - **Habits:** customary place/time/season activities score a little higher.
  - **Pick:** usually highest score, sometimes a close alternative by chance (`TIM-16`); fortune never affects selection (`GOD-04`).
  - **Record:** three score parts most ahead of the next option and the two best rejected options, for the current activity and every saved history event (`PRN-13`).
  - **Done when:** adults knowing all four dried-hide steps, with flint/hides, complete them before winter without scripted plans in 16/20 runs; each choice shows three reasons (`MAT-22`).

- `MND-22` **Plans** *(Decided)*: Short plans of a few steps, each step scoring higher until the plan is done, fails or is dropped (`MND-09`): store food before winter, build a shelter, make a spear for tomorrow's hunt.
  - **Sources:** a season remembered as hard (`MND-28`), met by a plan the season before (store food, dry meat, gather fuel, make warm clothes); a goal's chain of blueprints (`MAT-22`); keeping and penning more animals, once kept animals are worth it; ambitions (`MND-32`); requests and group plans (`CUL-22`).
  - **Times:** a step can have a time and place, such as dawn at the ford or a group plan's start; when it is time to go, it interrupts what they do (`TIM-17`), and one missed by over an hour fails.
  - **Looking ahead:** a need a season away counts about half, more for the patient (`MND-20`).
  - **Done when:** a band hungry last winter that can dry meat or store nuts has food stored by winter in 16/20 runs; one never hungry stores little.

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
  - **Done when:** youths who chose to master stone reach stone experience 8 before their peers in 16/20 runs.

### 9.7 Skills and discovery

- `MND-06` **Experience and skill** *(Decided)*: Experience in 15 sectors and a skill in each known blueprint, both from 0 (none) to 10 (master), rising with use.
  - **Sectors:** stone, wood, fire, cooking, hunting, gathering, hides, building, healing, pottery, herding, farming, metal, art and music.
  - **Start:** a newly learned blueprint starts at skill 1; experience counts through each try's level (`MAT-04`), which also sets quality (`MAT-20`).
  - **Rising:** each try adds a little, more on a success or when taught (`MND-13`), less at high levels: practising most days gives level 5 in about 2 years and 10 in about 10 (`TIM-18`); everyday work in a sector, such as nursing or singing, counts too, and each learns at their own speed (`BIO-08`, `BIO-16`).
  - **Fading:** unused, both fade slowly, never below about half their best; a blueprint dies with its last holder, though things they made may be copied (`CUL-02`).
  - **Done when:** someone knapping most days reaches flake skill 5 in 1.5–3 years in 16/20 runs, and 10 years unused leave at least half.

- `MND-13` **Learning and teaching** *(Decided)*: How one person learns from another; how crafts spread through groups is Culture's (`CUL-01`, `CUL-02`).
  - **Watching** an unknown blueprint used, on purpose (the watch activity, most by children and the curious), gives a hunch (`MND-11`), and about five watched uses teach it at skill 1; a use seen while busy counts a quarter; the most skilled, the respected and kin are watched most (`MND-24`).
  - **Being taught:** someone who knows a blueprint, believes another doesn't (`MND-23`) and wants them to (kin, friends, the kind, or for a gift) teaches beside them (`TIM-17`).
    The learner tries at their level's full chance (`MAT-04`); a taught try adds about four times a lone try's skill, more with a better teacher; the first success makes it known.
  - **Being told** how something is made gives a hunch.
  - **Children** learn fastest.
  - **Remembered:** each known blueprint keeps from whom and how it was learned, for the lines of teaching (`PRE-35`, `GOD-09`).
  - **Done when:** with one adult who knows flakes, 3 in 4 adults can make them within 2 years in 16/20 runs (`RES-03`), and taught children learn faster than watchers.

- `MND-10` **Surprises** *(Decided)*: A result or sight never met or not expected, such as a stick smoking as it is twirled against dry wood.
  - **How it works:** noticed about 1/2 of the time, from 1/4 for the least curious to 3 in 4 for the most, and half as often when busy, tired or frightened (`MND-03`), it gives a strong memory, a pull to look into it (`MND-07`), a link at its place (`MND-05`) and a hunch when it is a hint (`MND-11`).
  - **Done when:** in 20 runs where a stick smokes in someone's hands, a curious twirler keeps a hunch for fire in at least 10, an incurious one in fewer.

- `MND-11` **Four routes to discovery** *(Decided)*: Blueprints require discovery, learning or Revelation (`PRN-01`, `GOD-13`); discovery routes are accidents, experiments, dream hints and copying.
  - **Chance:** once per activity fitting an unknown blueprint’s action, inputs and place (`MAT-04`), maker chance at doer's level × route factor: accident about 1/20, experiment 1/5, hunch 1/2, × blueprint discovery factor (1 unless tuned).
    Noticed success teaches skill 1 (`MND-10`).
    Noticed blueprint-hint outcomes from own rolls, plain uses, other blueprints or timers give hunches.
    Unmeant timer results, such as copper, credit the first noticer (`MAT-21`).
  - **Hunch:** action/input-kind/result guess; bounded by `MND-14`; discard after about 10 failures or a year unused.
  - **Accident:** any base-action activity.
  - **Experiment:** an ordinary choice (`MND-09`), pulled by curiosity/play, favoured at mood >50 without pressing needs, or when a need <20 has no known answer.
    In that case aim at it using relevant known characteristics: food for hunger, warmth/burn for cold.
    Curious adults experiment about daily in good times, average adults weekly.
    One activity tries a held hunch; else a known action on a familiar input kind; else any action, often familiar, on one or two reachable things under current conditions.
    Three-plus inputs need accident/hunch; failures still teach characteristics (`MND-04`).
  - **Copy:** unknown made things reveal materials and marked actions (chipped, ground, drilled, sewn, fired), giving weak hunches at experiment chance; watching manufacture gives a full hunch.
  - **Recorded route:** hunch source (accident, natural/sent dream, copying, telling), otherwise experiment.
  - **Tuning:** three route factors, blueprint factors/difficulties, surprise noticing, experiment frequency and dream-hint chance; identical across worlds (`PRN-17`, `RES-16`).
  - **Done when:** each route discovers in at least 2/20 scenes; sharp-stone test passes (`RES-03`).

- `MND-12` **Dreams** *(Decided)*: Each night a sleeper has one dream, from the last few days' strongest memories by feeling, surprise and need, now and then an older one; those memories then fade more slowly.
  - **Feelings:** its feeling lingers as a thought (`MND-29`): a nightmare leaves fear, a dream of the dead grief.
  - **Hints:** about 1 dream in 60, 1/20 while a need is below 20, a dream joins a thing, an action and a needed result from different memories into a hunch (`MND-11`), pointing 1/3 of the time to a real blueprint they don't know, chosen as for an idea dream (`GOD-03`), and otherwise to nothing.
  - **Kept** only if strong (a nightmare, the dead, a hint, a sent dream), then told (`CUL-24`), feeding beliefs about the unseen (`MND-31`).
  - **Done when:** a dream of a dead parent strengthens belief in ancestors, and natural dream hints give hunches at their tuned rate (`RES-13`).

### 9.8 Life together

- `MND-24` **Relationships** *(Decided)*: For each known person (`MND-14`): face and name, kinship as believed, an opinion from −100 to +100, trust and respect from 0 to 100, favours owed (`MND-26`), shared memories and what they know of them (`MND-23`); and an opinion of each people they know of (`CUL-23`).
  - **Opinion:** a day with chats together +1, shared food or help +3, a gift +5, help in danger +10; an insult −5, theft or a broken promise −15, a blow −20, killing kin −80; apart, a point a week back toward 0; together, a point a week up between like personalities and down toward a hot temper.
  - **Trust** sets how far their word is believed (`MND-27`) and whether they are followed; it rises 2 each time their word proves true and falls 10 when it proves false; **respect** comes from the sources of `CUL-06` (skill, generosity, success, age), as their custom weighs them.
  - **Bonds:** lasting high opinion makes friends, rivalry rivals, harm enemies, and a killing can start a feud (`CUL-31`).
  - **Done when:** over a season, people who share food come to like each other and repeated insults turn liking to dislike, in 16/20 runs.

- `MND-33` **Social acts** *(Decided)*: Chat, share, give, trade, ask someone to make a thing, help, comfort, tell, ask, play, court, teach, gossip, insult, quarrel, fight and steal.
  - **Chosen like anything else** (`MND-09`), each moving opinions (`MND-24`) and needs (`MND-07`).
  - **Talk alongside:** chat, tell, ask and gossip run alongside work, walking, eating and rest, between people a few metres apart and out of danger, at the pace of `CUL-24`, ending no activity (`TIM-17`); comforting, courting, teaching, long stories at the fire, quarrels, fights and stealing are activities of their own.
  - **Kept:** each person's last ten or so topics told and heard, for their card (`PRE-45`).
  - **Gossip** moves a listener's opinion of someone about a quarter of the way to the speaker's, by trust.
  - **Giving and trade:** a gift leaves a favour owed (`MND-26`); a trade is a swap both sides value above what they give (`CUL-21`); making a thing for someone is paid with a gift.
  - **Courtship:** unpaired adults court the one they are drawn to (`BIO-17`) and like most, above about +40, with time, shared food and help; when each one's opinion of the other has stayed above about +60 for about 10 days, they pair, as their customs allow (`CUL-27`, `BIO-15`).
  - **Quarrels** turn into fights when anger runs high; fights hurt (`BIO-13`), and others step in, take sides or remember.
  - **Done when:** in a band scene with unpaired adults, a pair forms within a year in 16/20 runs, and people talk while they work.

- `MND-23` **Who knows what** *(Decided)*: People track which of their own blueprints and places each known person also knows (seen using, gone there together, told), teaching and telling only those who lack it (`MND-13`, `CUL-24`); what others need and feel is read from what is seen (`MND-03`), not kept.
  - **Secrets:** people keep quiet about a valued place (a flint source, a food store, a rich patch) when telling those outside their kin could cost them, the greedy and proud most, and about their own theft or broken taboo.
  - **Following:** the curious, and those who distrust someone, may follow them at a distance to see where they go.
  - **Done when:** where a greedy man keeps a flint source secret, a curious watcher finds it by following him in at least 4/20 runs.

### 9.9 Scale

- `MND-14` **Every person has a full mind** *(Decided)*: Everyone has every part of this section always, never a cheaper mind for being far away or unwatched (`WLD-13`).
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
  - **Done when:** a deer herd near camp grazes, drinks and flees together, and near a wolf den children feed and keep a pup in at least 4/20 runs (`MOM-06`).

## 10. Culture and society

Culture is what people pass on rather than inherit: crafts, words, beliefs, customs and art.
With minds, it is the heart of the game.
Numbers here are starting values, tuned in tests (`PRN-17`).

### 10.1 How culture works

- `CUL-07` **Nothing social is scripted** *(Decided)*: Templates give the shapes; events decide which happen, never a script, a date or an era.
  - **How it works:**
    - **Templates** name no particular people, person, place or date (`PRN-07`); each needs its conditions in the world and people's own choices (`MND-09`).
      Each has five parts: its trigger (a kind of event or state); what it takes from the real event (place, animal kind, weather, person or act); what it makes (a belief, custom, role or myth, named in the language); which options it makes score higher or lower; and how it grows, spreads and fades.
    - **Catalogues:** this section's lists are catalogues (`MAT-13`), checked like the others (`MAT-17`) but free to depend on beliefs.
    - **In the world, or for you:** bands, families, peoples, villages, customs, shared spirits, roles (leader, head of family, council, chief, shaman, priest, specialist) and alliances are facts people know and act on; religions, gods, wars and ages are names for you only (`PRE-39`) and change nothing.
    - **Book of ages:** a people's firsts of each kind, and its named spirits, festivals, myths, leaders and chiefs, sized as in `MAT-21`; the rest shows on cards (`PRE-35`).
  - **Check:** a test finds no template tied to a date, an era or a named people, place or person; each belief template and story shape has a scene where it appears in at least 5/20 runs after its event, and no scene shows one without its own; customs and roles are read from the pace-test worlds (`RES-07`), which end with peoples of different spirits, customs and kinds of leader.

- `CUL-33` **Pace of culture** *(To test)*: When culture first shows in typical worlds, in years from the start, beside the pace of discovery (`TIM-19`).
  - First custom a band names (`CUL-06`): within 1.
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
    Breaking one brings fear and shame (`MND-19`), and punishment.
  - **Done when:** in 20 runs of a band most of whose adults hold a taboo on a fish, the band names it, and a member seen eating the fish is punished as its customs say in at least 16.

- `CUL-34` **Rites** *(Decided)*: A rite is one shared activity of about an hour, held before a task, at a place, at a death, or on a day of a people's calendar (`CUL-29`).
  It is made of one to three of six forms: singing, dancing, leaving a gift, burning a gift, painting or marking, and a shared meal.
  - **How it works:**
    - **Where rites come from:** the act a band credits for a good outcome (`MND-05`, `CUL-05`), held in the matching form (singing or dancing as itself, eating as a shared meal, anything put in a fire as burning a gift, applying as painting, any other act as leaving a gift); the burial custom (`CUL-06`); and festivals.
    - **Who leads:** the shaman or priest (`CUL-26`), else the leader (`CUL-22`), else whoever first did it; all who join start and end together (`TIM-17`).
    - **Effects:** each who joins gains belonging and a good thought (`MND-29`), believers feel safer (`MND-31`), and it costs the time and anything given.
  - **Done when:** in at least 2/20 runs of the scene of `MOM-04`, the hunting song becomes a rite most of the band holds, its forms animated and heard.

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
    - **Splitting:** a band that moves splits past about 40 people, a village past about 300, and either past about 20 after a season most adults spent below condition 30 (`BIO-09`), a fight between heads of families or a failed challenge.
      The families with the lowest opinion of the leader (`MND-24`), up to about half the band and leaving at least about 10, are offered leaving (`MND-09`), again each season while the band stays past its size, with close kin who follow, their things and embers, to found a new band at least a day's walk away under their most respected, or to join a village of their people.
    - **Joining:** a band below about 10 people joins kin in another band or village.
  - **Done when:** a band of 45 after a fight between heads of families splits within a year in at least 15/20 runs, and a band of 8 joins kin.

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
    - **Chiefs:** in a large village (`CUL-28`), a leader of 10 years becomes chief for life, alone setting group plans, ruling on feuds (`CUL-31`) and punishing.
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
  - **How it works:** each comes only from a named option: violence, war and captivity from fights and raids (`CUL-31`); cruelty from punishments (`CUL-06`); leaving a newborn, by a mother below condition 30 (`BIO-09`) still nursing a child under 2; eating the dead, below condition 15 with no other food known within a day's walk; a captive's life as an offering, only where most adults hold an angry spirit at 70 or more after a second disaster put down to it within a year (`CUL-05`).
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
  - **Done when:** a band by a salmon river that knows pit houses and storage becomes a village within 20 years in at least 10/20 runs.

### 10.6 Expression

- `CUL-09` **Visual art** *(Decided)*: Paintings and carvings composed from motifs drawn from the art kit (`PRE-46`) in each people's style, most often showing real events.
  - **How it works:**
    - **What is shown:** one of the maker's strongest memories, often a hunt, a death or a flood, or a myth (`CUL-11`): 1–8 motifs for its real animals, people and things, in a row, a ring or a scatter as the style says (`CUL-12`), numbers shown roughly (three deer for a herd).
    - **Skill:** low art skill gives fewer motifs, rough lines and one colour; high skill more motifs, clean lines and up to three colours (`MND-06`), from the pigments at hand, ground green ore among them (`RCK-15`, `MAT-19`).
    - **Kinds:** pictures on walls, rocks, hides and the flat sides of things; ornament, the people's patterns on pots, clothes, tools, beads and bodies; and carved or clay figures.
    - **Made** after strong events, at rites or in play (`MND-20`).
  - **Done when:** in at least 2/20 runs of the scene of `MOM-07`, a painting shows the hunt's real animals and hunters in 1–8 motifs, in its people's style.

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
    - **Gatherings:** two or more bands camped within about 2 km of each other for at least a day, a settled band coming as a trip of those who join (`TIM-17`).
      A leader's choice of camp (`CUL-22`) weighs how many members want to meet kin, friends or a partner in other bands, so in a season of plenty bands head for where they last met them then.
    - **Calendar:** a yearly sign seen two years running (`MND-28`), such as first frost, herds passing, nuts falling or the full moon (`WLD-07`), is named when first talked of (`CUL-18`), such as "when the salmon come", and plans and festivals keep to it (`MND-22`).
    - **Festivals:** a gathering at the same place (within about 5 km) and season 3 years running, with a rite held together (`CUL-34`), becomes a festival, named and kept by its sign, most often that season's full moon; it lasts about 3 days, and is forgotten if not held for 3 years.
    - **What happens:** feasts, rites, songs and dances (`CUL-10`), myths, marriages (`CUL-27`), trade (`CUL-21`) and councils of heads of families, and news and crafts spread (`CUL-16`).
  - **Done when:** three bands near one nut grove hold a named festival within 20 years in at least 10/20 runs.

## 11. Presentation

### 11.1 Visual style

The look, written to stand without any image.

- `PRE-01` **Pixel art** *(Decided)*: The world is fully 2D pixel art, sharp and detailed, under rich changing light (`PRE-30`).
  At the reference portrait size of 1080 × 2400 screen pixels, each world pixel is a crisp 2 × 2 square (`PRE-22`); text and controls stay sharp at the screen's full resolution.
  Only a live pinch may briefly resample the world picture.
  A saving made to fit the phone (`PLT-04`) stays only where you cannot tell it from the full picture in a blind test on your phone.
  - **Done when:** at every zoom stop, world pixels show as crisp 2 × 2 squares at the reference size and nothing in the resting world picture is blurred, and every saving in the build has passed its blind test, recorded with its pictures.

- `PRE-02` **Fixed camera** *(Decided)*: A fully 2D world that shows believable depth, scale and structure.
  The local camera faces north at exactly 37° above the horizon, and never rotates or tilts as you zoom.
  - **Done when:** `PLT-04`'s benchmark worlds stay smooth at every zoom stop while you pan and pinch, and local views keep the same angle.

- `PRE-20` **Colour by design** *(Decided)*: Every material is painted as pixel art in its own shades, made from its colour (`MAT-10`) or by hand for common ones, lights warm and shades cool, and the light (`PRE-30`) falls on it smoothly.
  Its texture is quiet, of marks two or more texture pixels wide that show what it is made of, and carries no sunlight from one side.
  Strong colour stays in small accents, except where the season itself is the colour, as in autumn, and each biome, hour and season has its colour plan.
  - **Done when:** no surface turns to speckle at any zoom stop on your phone, and in a summer daylight scene strong colour stays in small accents.

- `PRE-21` **Edges and contact** *(Decided)*: Up close, visible things stand apart by light, shade and colour, with no drawn outline; obscured band members use the reveal of `PRE-28`.
  A bright edge shows where the sun, the sky or a fire catches a shape (`PRE-30`), such as a cliff's sunlit rim or a person's fire-facing side, and corners, undersides and the ground where things stand darken.
  Small, far figures and obscured band members use the quiet treatments of `PRE-28`.
  - **Done when:** at the person and close camp zooms no unobscured shape has a drawn outline, and every figure in sun or firelight shows its lit edge and its contact shade.

- `PRE-22` **Steady world pixels** *(Decided)*: Pixels never crawl, flicker or shimmer with the camera still or panning.
  Every resting zoom has crisp squares, about 2 by 2 screen pixels, in portrait and landscape.
  Near, middle and far art is drawn at 64, 16 and 4 world pixels a metre, with reviewed and repaired reductions for the steps between and beyond.
  Small things become simpler as you zoom out, keeping their important shapes and accents.
  A pinch moves smoothly, then settles on a crisp zoom step without a jump in the place you were looking at.
  - **Done when:** with time frozen and the camera still, frames are identical; while it pans or pinches, no more than 2 in 100 pixels flicker beyond what the movement explains; pixels are crisp at every resting zoom; and you see no shimmer in the review's clips on your phone.

- `PRE-23` **Rock faces** *(Decided)*: Cliffs show the rock layers where they stand (`WLD-09`), which go on underground (`PRE-25`): layers of different thicknesses, cracks and fissures, lichen and water stains where the face is wet (`WLD-16`), soot above lived-in caves (`MAT-18`), grass hanging over the top and scree at the foot.
  - **Done when:** cliffs of three kinds of rock show their own layers, and a cave lived in for 10 years shows soot.

- `PRE-24` **Height and shelters** *(Decided)*: Continuous slopes, cliffs, simple rock shelters and buildings show their depth and their place in the ground (`WLD-12`).
  Interiors are lit only by openings and fires (`PRE-30`).
  Shallow cutaways show occupied shelters; deep caves have a separate view reached through their entrance.
  Several overhangs stacked above one another come later.
  - **Done when:** feet meet slopes and shelter floors, a shallow cutaway shows a shelter and a hut inside, dark but for openings and fire, and a deep cave opens in its own view.

- `PRE-25` **Cut-away view** *(Decided)*: The ground can be sliced open along a line you choose, showing rock layers (`WLD-09`), soil (`WLD-27`), water in the ground (`WLD-17`), and hearths, tools, bones and graves where they were buried (`MAT-08`).
  - **Done when:** a slice through a camp left 200 years before shows its hearth, bones and tools at their depths.

- `PRE-26` **Water** *(Decided)*: Rivers meander and change width, with gravel bars, reeds, lines that follow the current, ripples at fords, glints of sun and drifting mist, all from the river's course and flow (`WLD-17`) and the weather (`WLD-16`).
  Shallow water shows its bed, deeper water darkens away from the shore, and a thin bright line marks where water meets land or anything standing in it.
  - **Done when:** every river is at least 2 screen pixels wide from valley zoom inward, and farther out those draining about 1,000 km² or more (tuned).

- `PRE-27` **People and animals** *(Decided)*: Detailed pixel-art figures, animated for their activities (`PRE-44`): up close their faces, hair, seams, trims and wear read, as in the pictures you liked, and each zoom band has its own drawing of a face.
  The first set has four drawn facings; one animal tests eight before any wider adoption.
  - **How it works:** a figure shows its body's condition (`BIO-13`) and looks (`BIO-08`, `BIO-22`), wearing and carrying what the person has (`PRE-42`), with their strongest feeling on its face (`MND-19`); animals use their body pattern (`PRE-46`).
    Of the body it shows only build (thin, average or stout), age (grey hair, a bent back), a limp or a sling, a pale dressing or splint, scars or pox marks as a few darker pixels, and blood at the Show level (`PRE-18`).
  - **Done when:** at person zoom, every listed body sign shows, and build, age and clothing tell people apart.

- `PRE-28` **Readable from far away** *(Decided)*: Zooming out, people and animals become small figures drawn to read: outlined in a darker shade of their own colour, faces and light clothes kept light, tools drawn larger; a group or herd close together becomes one marker, and a camp a point at its hearth that glows if it has a fire.
  Up close, people are among the first things you see in any scene, helped by light from the sun, the sky and fires, and by their movement: nothing in the world is changed to make them stand out.
  Crowns and roofs covering the selected person fade, as do roofs of occupied shelters; other obscured band members show as quiet silhouettes.
  This reveals the picture without changing the shelter or anyone's knowledge.
  - **Done when:** a camp of 30 people stays readable at every zoom stop, with no jump as its forms change, and in the review's busy scenes (an autumn wood, a crowded camp, a night camp) you find every person at a glance as they move, including those under crowns or roofs.

- `PRE-29` **From above** *(Decided)*: Wider local views give way to an overhead map and then a globe picture, vivid and textured to the pixel: forests as clumps of crowns, grassland, deserts, rock and snow in their own colours (`WLD-12`), hills shaded, rivers as lines, the sea from deep blue to turquoise shallows with its currents, the clouds of the weather and their shadows (`WLD-16`), and at the top the globe (`WLD-02`); overlays sit on it (`PRE-07`).
  It is lit by the sun where the hour and season put it.
  - **Done when:** rising from valley to globe, the view changes without a jump, and coasts and rivers stay visible.

- `PRE-30` **Light, time and season** *(Decided)*: Light follows the sun, the sky and the air, as in Minecraft's Vibrant Visuals: the sun's and the sky's colours come from the sun's height through the air, and every material's colours at dawn, day, dusk and night in each season are that light on it, with true white-warm light at noon.
  The sun casts real shadows by hour, season (`TIM-18`) and latitude (`WLD-01`), sharp near what casts them and softer as they lengthen; shade takes the sky's colour and hollows are darker; distance adds haze, warmer toward the sun; water reflects the sky and what stands above it, and glints.
  When a day passes in less than about 10 seconds (tuned), the light holds steady from high up and only its tint follows the hour, so the screen never flickers between day and night.
  A fire is a warm, flickering light as bright as its heat (`MAT-18`), warming nearby faces, glowing, and sending up smoke and embers.
  - **Done when:** one place at dawn, noon, dusk and night, in summer and winter, shows its light and its shadows.

- `PRE-03` **Seamless zoom** *(Decided)*: One smooth pinch journey from the globe down to one person chipping flint, settling on crisp zoom steps, through these stops, each with its speed in `TIM-01`:
  - **person:** about 8 m across in portrait, a standing adult about 200 screen pixels and about 100 texture pixels tall;
  - **close camp:** about 20–50 m, a person about 35–90 screen pixels tall, every figure in full;
  - **camp:** a few hundred metres, people as tiny figures (`PRE-28`);
  - **valley,** about 10 km; **region,** about 100 km; the **world map**; the **globe** (`PRE-29`).
  - **How it works,** by distance from where the camera looks: within about 300 m, from camp zoom inward, full areas (`WLD-12`), made for the picture without changing anything (`WLD-13`); out to about 10 km, each area's coarse ground, made from the seed in a moment, shaped every few tens of metres, under its cover (its cell's, or a kept area's own) drawn as forest canopy, scrub, grass or bare ground; beyond, and from region zoom out, the world cells.
    People, herds, camps and buildings show at every distance, and a full area being made shows its coarse ground until its detail fades in, within about a second.
  - **Done when:** a pinch from globe to person over unvisited land never stalls, and full detail is in within about a second.

- `PRE-31` **Visual review** *(Decided)*: At every milestone stage the look is reviewed on a contact sheet made on the phone, on one page, from fixed saved worlds: each zoom stop at noon and dusk in portrait, one landscape view, the art sheet (`PRE-46`), three short clips of people at work, clips of the camera panning and pinching (`PRE-22`), and the busy scenes of `PRE-28`.
  - **Owner OK:** 8 October 2026.
    Early stand-in camp builds are reviewed for readable identity, action, selection and touch use and may be delivered before final art approval.
    Final art-quality acceptance applies to the relevant finished scene; its full requirements remain open.
  - **Check:** finished scenes meet every Done when of 11.1 and 11.2 through the contact sheet, judged by review and then by you (`PRC-10`).
    Early stand-in deliveries pass the readability and touch-use review above; they leave final art acceptance open.

### 11.2 Things and movement

- `PRE-42` **Built from their materials** *(Decided)*: Each thing's drawing shows the colours and shapes of the materials used: a hut of birch poles and hides looks pale and brown, one of reeds straw-yellow, and more poles make a bigger hut.
  - **How it works:** the drawing shows the amount used (`PRE-46`), and its icon shows the same thing.
    States, wear and quality show on it: meat darkens as it dries, edges chip, bindings fray (`MAT-19`, `MAT-20`); a change to ground or a body shows there, such as a sown plot or a dressing.
  - **Done when:** every new thing a named result makes has its drawing, and two routes in different materials look clearly different, huts at close camp zoom.

- `PRE-43` **Variety** *(Decided)*: No two things look quite alike: each varies a little in proportions, lean, wear and colour by its own seed, within its design's limits, and looks the same each time.
  Its maker people's style (`CUL-12`) sets its proportions, lean and colours within each material's shades (`PRE-20`), and puts their pattern, one of about 12 (notches, bands, dots, zigzags, fringes, painted rings), on the parts its design marks as decorated, as much as the style says.
  Trees, bushes, rocks and ground cover vary the same way, without style (`WLD-31`).
  - **Done when:** at person zoom, things of two peoples, or of one people 100 years apart, are clearly told apart.

- `PRE-44` **Animations** *(Decided)*: Every activity has its own animated drawing, so you can tell who is knapping or dancing.
  The first set has a six-frame walk, a few work actions and a small clothing set; later milestones fill out the list below.
  - **The list:** the base actions (`MAT-06`), heat with a variant for blowing on a fire, the everyday activities (`BIO-21`), and wading, swimming, climbing, rage, despair, nursing or carrying a baby, lying hurt and lying dead: about 45 movements, each a loop drawn from 2–6 key poses, as long as the activity (`TIM-17`).
  - **Social acts** (`MND-33`) are talking plus one of about 8 gestures: pointing, giving, embracing, pushing, a raised fist, waving away, holding hands, stroking.
  - **Dances** are 8 moves (step, stamp, turn, sway, crouch, leap, clap, arms raised), strung as `CUL-10` says; rites (`CUL-34`) use movements on the list, done together.
  - **Variants** keep the same activity readable: children quicker, elders stooped and slower; a limp or a still arm from a wound (`BIO-13`); grief slumps, fear quickens, anger stiffens (`MND-19`); cold hunches; skill steadies the strokes (`MND-06`).
    Each figure's timing is offset by its seed, so a crowd never moves in step, except dancers and singers keeping a shared beat (`MND-26`).
  - **Animals:** one set per body pattern (`PRE-46`), timed by the species' size and speed: stand, walk, run, feed, drink, rest, sleep, play, fight, call, fall, and swim or fly.
  - **At speed,** each figure keeps showing its activity at a steady pace (`TIM-01`).
  - **Done when:** every movement reads at person zoom; at close camp zoom, standing and ground work, carrying, walking, resting, fighting and dancing are told apart.

- `PRE-46` **The art kit** *(Decided)*: Everything in the world is drawn from one fixed kit, keeping content countable.
  - **Owner OK:** 8 October 2026.
    Make only art needed by the next playable scene; labelled stand-ins are allowed.
    Review essential sprites together in motion before bulk production, keeping signed sheets and provenance.
  - **Shared designs:** editable pixel-art bodies, garments, attachments, plants, shelters and things, varied in size, material, wear and style, so a few hundred designs make thousands of things.
  - **Shared shapes,** one per form (`MAT-02`), drawn to a thing's size and material: raw items need no design of their own, and ground cover is drawn by the patch (`WLD-31`).
  - **Made things:** a drawing shows the parts and materials of the inputs that made it (`MAT-04`); only a part no shared design fits, such as a pot's body or a blade's outline, needs a new design, and states need none (`MAT-19`).
  - **Shelters:** several types, each a layout of parts true to what excavations show, so the routes to huts (`MAT-07`) look different.
  - **Plants:** about 8 forms (needle tree, broad-leaved tree, bush, grass or grain, herb or flower, reed, root plant, fungus); a species sets its height, crown and colours by season and stage.
  - **Animals:** about 6 body patterns (hoofed, padded, small and quick, bird, fish, legless reptile); a species is its proportions and colours, with antlers, horns or tusks (`WLD-32`).
  - **People:** one figure (`PRE-27`) with about 8 kinds of garment, in child and adult sizes, and hair, beads and paint in each people's style (`CUL-12`).
  - **Pictures and figures:** a painting or carving shows each animal, person or thing as a side-view motif in its people's style (`CUL-09`); a carved or clay figure uses the same design, small, in its material.
    About 12 signs, faces and each band's small designs, also the patterns of `PRE-43`, complete the kit.
  - **Sizes:** anything small (ground cover, flowers, leaves, faces, held tools) has a design for each zoom band, drawn for that size and never only shrunk; flowers, berries and eyes stay at least one texture pixel.
  - **Done when:** every launch thing, plant and animal is drawn from the kit, shown on an art sheet in two materials.
    Before bulk art, you approve six pieces in the running engine: a person, animal, tree, boulder, ground and shelter, with the pending approvals and missing seasonal shapes resolved.

### 11.3 On the screen

- `PRE-32` **World first** *(Decided)*: The world fills the screen; controls and panels appear only when you ask (`PRE-33`), apart from a live moment appearing briefly (`PRE-08`) and talk bubbles (`PRE-45`).
  - **Done when:** a few seconds after any touch, only the world, live moments and bubbles are on screen.

- `PRE-34` **Both orientations** *(Decided)*: Every screen works one-handed in portrait and two-handed in landscape (`VIS-14`, `PLT-02`): each view is one column of panels, full width in portrait with its controls at the bottom, beside the world in landscape.
  - **Done when:** every view works both ways, with its portrait controls in the bottom third of the screen.

- `PRE-33` **Gestures** *(Decided)*:
  - **Owner OK:** 8 October 2026.
  - drag to pan; the north-facing camera does not rotate (`PRE-02`);
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
  - **Done when:** in a world, each kind of card fills every line above from the records.

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
  - **Done when:** in a world, each overlay matches the records at 20 sampled places.

- `PRE-08` **Live moments** *(Decided)*: Only what matters interrupts you: moments and signs from the story director's one list, such as a major named discovery (`MAT-21`), within its one budget (`TIM-02`); the closer you watch, the higher a moment's score must be.
  Moments you don't take wait in a tab of the book of ages, and the level is a setting (`PRE-40`).
  - **Done when:** watched from the globe for an hour, a test world's live moments keep within `TIM-02`'s budget.

- `PRE-39` **Recognising what emerges** *(Decided)*: The game spots, for you only (`CUL-07`), what is worth telling: named discoveries (`MAT-21`) and other firsts, crafts lost with their last holder (`CUL-02`), new peoples and villages (`CUL-23`, `CUL-28`), leaders, feuds and alliances, and disasters.
  It also names religions, gods and wars (`CUL-26`, `CUL-31`).
  - **Firsts** count worldwide and for each people: any event of a kind the history has never recorded; a rediscovery after a loss is marked as one.
  - **Ages** begin only at these turning points: a step of the arc first reached anywhere (`TIM-19`), a new people, the first village, and a war.
    Each is named by a fixed pattern from its defining event and that event's name in their language, such as "The age of *hesoru*, fire from wood", and lasts until the next turning point, however short or long.
  - **Done when:** in the pace tests every age starts at a listed turning point, and a code check finds no path from recognisers back into the world (`WLD-13`).

- `PRE-09` **Graves and old camps** *(Decided)*: The dead and the places people left stay in the world (`MAT-08`); the book of ages lists them by people and date.
  A grave shows who lies there, how they died (`BIO-14`), who buried them and what was laid with them, and opens their card and life story (`PRE-37`); an old camp shows its hearths, rubbish heaps, lost tools and bones, and who lived there and when.
  Tapping a find shows who made or left it, and when.
  - **Done when:** after 200 test years, every grave and old camp listed can be visited, each find naming its maker and date.

- `PRE-10` **Family trees and legends** *(Decided)*: Family trees across generations from the birth records (`BIO-15`) and marriages (`CUL-27`), with the people's own legends (`CUL-11`) beside them, each linked to the true events it tells of.
  - **Done when:** a person's tree shows four generations from the birth records, and their legends link to real events.

- `PRE-14` **Details of a mind** *(Decided)*: For anyone, everything in their mind, shown plainly under a short summary (`PRE-41`): needs (`MND-07`); mood with each thought behind it (`MND-29`); feelings (`MND-19`); personality (`MND-20`); breakdowns (`MND-30`); memories and dreams, most important first (`MND-18`, `MND-12`); the mental map (`MND-28`) and knowledge of things (`MND-04`); blueprints with skill and experience (`MND-06`); hunches (`MND-11`); who knows what (`MND-23`); beliefs with certainty and the events behind them, or "reason forgotten" (`MND-27`, `MND-05`); plans (`MND-22`); ambitions (`MND-32`); relationships (`MND-24`); and the top reasons for what they do now, with the options it beat (`MND-09`).
  Your own acts on them are marked as yours (`GOD-09`).
  - **Done when:** for a test person, every kind of record above shows, and the summary changes once their mood does.

- `PRE-15` **Art that remembers** *(Decided)*: A painting, carving, bead or figure has a card (`PRE-35`) showing the picture, its maker and what it shows, linked to actual events (`CUL-09`, `PRN-15`).
  In the world, paintings and carvings are drawn on the surface they were made on, from their motifs (`PRE-46`).
  - **Done when:** in the scene of `MOM-07`, the painting shows on the wall and its card links to the hunt.

- `PRE-16` **Bestiary** *(Decided)*: Species cards, listed on a page of the book of ages and reached by tapping any plant or animal, show each kind's look in each season, where and when it lives, its yields and danger, and for animals their numbers and herds (`WLD-31`, `WLD-32`); tame and domestic kinds have their own (`WLD-33`).
  What each people calls it and believes about it shows once that people knows it (`CUL-18`).
  - **Done when:** every launch species has a card.

- `PRE-13` **Few screens, everything findable** *(Decided)*: Anything the world keeps track of can be found from the views in this section, mostly on a card (`PRN-04`).
  A new screen is added only when no card, overlay or page of the book of ages can show something well.
  - **Check:** each stage review confirms that every kind of record the world keeps shows on at least one card or view.

### 11.5 Text written for you

- `PRE-37` **Patterns first, the writer polishes** *(Decided)*: Every text starts from pattern sentences, delivered with the cards, mind views and history they describe.
  - **Owner OK:** 8 October 2026.
    The small finished game uses patterns.
    The device writer is an optional later experiment, adopted only if it beats patterns in direct comparison; its checks still apply (`PRE-41`).
  Texts are in English, each name in its people's language with its meaning at first use (`CUL-18`).
  - **Patterns:** a closed catalogue (`MAT-13`) of about 100 kinds of event (the entries of `PRE-39`, the memories of a life story, the story shapes and topics of `CUL-11` and `CUL-24`, and a mind's summary), each with at least 5 phrasings, picked by the event's seed and using all the records hold: names, places, seasons, causes and who was there.
    Labels, card lines (`PRE-45`) and dark events (`PRE-17`) always stay as patterns.
  - **Size:** a text covers at most about 12 events, chosen most important first and told in date order, in at most about 150 words; a long life is told in parts.
  - **Without the writer:** when it is missing, the pattern text is shown and stored, marked as such.
  - **Your choice:** if the writer is tried (`MIL-16`), compare it against patterns in each view and keep whichever reads better.
  - **Done when:** in a world of 100 years every kind of event shows 5 phrasings, each name's first use gives its meaning, and no text passes about 150 words.

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
  - **Done when:** when full sound is delivered (`MIL-16`), 32 sounds at once, with distance, muffling and echo, play on the phone without a break, or the cap and shares are lowered.

- `SND-11` **Ambience** *(Decided)*: Each place has its own background sound from its land, plants and water (`WLD-31`, `WLD-17`), the weather (`WLD-16`), the hour and the season: wind in grass or pines, a river, the sea, rain on leaves, thunder, birdsong at a spring dawn, the hush of snow.
  Birdsong comes from the small birds the place holds (`WLD-32`); the world has no insects or frogs, so nights have no chorus.
  - **Done when:** a test valley sounds different at dawn and at night, in rain and in calm, with birdsong only where small birds live.

- `SND-03` **The murmur** *(Decided)*: People talk in a murmur built from the sounds of their language, never in real words.
  - **How it works:** talk is strung from a bank of spoken syllables in the language's sounds (`CUL-17`), for two base voices, a woman's and a man's, rendered once in the cloud by the natural-sounding voice you chose by ear and shipped with the game, so no voice model runs on the phone.
    It is shifted in pitch and tone for the speaker's age and build (`BIO-08`) and in loudness, speed and tune for their feelings (`MND-19`).
    Laughing, crying, calling, screaming and babies' cries come from a few recordings for each kind of voice (child, woman, man, elder), shifted the same way.
    The voice is chosen by ear when full sound is delivered (`MIL-16`).
  - **Done when:** by ear alone you tell a child, a woman, a man and an elder apart, and anger from grief.

- `SND-02` **Their music** *(Decided)*: Each people's songs, rhythms and instruments (`CUL-10`), heard when you are near.
  - **How it works:** each song plays from its record: its notes and rhythm, in its people's scale.
    Instruments sound by their sound blueprints (`SND-06`): a longer flute is lower, bone brighter than wood, a bigger drum lower and a thicker hide duller; drums, poor in the first trials, are remade before you hear them.
    Voices sing by holding and pitching the murmur's syllables to the notes, with no words (`SND-03`); if that fails your review when full sound is delivered (`MIL-16`), songs are hummed and played, with clapping.
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
    The simulation runs only on processor cores, so it repeats exactly and no language model affects it (`TIM-16`, `RES-05`, `PRN-06`).
  - **Plans on:** 2 small, 4 middle and 1 fastest core at the speed they hold under full load (held speed), about 3 W, and about 8 GiB of memory; the simulation may use up to the four middle cores, leaving the rest for the picture, sound and the writer, and every time budget in this file is time on one middle core at held speed (`MND-15`, `PLT-04`).
  - **Done when:** every alpha installs and runs on your phone, and at every stage its benchmark worlds end exactly as they do in the cloud.

- `PLT-02` **Portrait and landscape** *(Decided)*: Turning the phone switches between the two layouts of `PRE-34`.
  - **Done when:** turning the phone on every screen keeps the world, the camera and the texture pixel's size (`PRE-22`), with no reload.

- `PLT-03` **Works offline** *(Decided)*: Everything the game needs, the writer AI included (`PRE-37`), is on the phone; nothing in play makes a network call.
  - **Done when:** in flight mode, making a world, an hour's play and book-of-ages text all work.

- `PLT-06` **Installing new versions** *(Decided)*: Each alpha is a file you download and install on the phone, once you allow installs from your browser (`PRC-11`).
  - **How it works:** every build is signed with one key for your hobbyist developer account, as Android requires from 2027, set up once before the first alpha and kept outside the cloud sessions with a copy you hold (`RSK-18`, `RSK-29`).
  - **Done when:** each alpha installs over the last one from the phone's browser, keeping every world.

### 13.2 Performance

- `PLT-04` **Measured limits** *(Decided)*
  - **Owner OK:** 8 October 2026.
    Approval decides these requirements; their measured results remain unverified until the stated tests pass.
    Keep all numerical safety and performance requirements, applying each scale gate when its real consumers exist.
    Measure your phone first; schedule a second device only when named and available, and never count missing hardware as a pass.
  - **What:** Reported at every stage (`RES-06`) against these targets:
    - **Speed:** the targets of `TIM-07`.
    - **Scale budgets:** the 1,000-person gates remain at least 2.5, 2, 1.7, 1.5, 1.5 and 1 game years a real minute as full bodies, minds, crafts, society, presentation and the arc arrive (`MIL-15`, `MIL-16`, `MIL-17`).
      A partial camp does not close these gates; the expansion plan schedules each with its complete consumers.
      A stage that misses names its biggest cost and wins it back by the next stage close, or brings it to you.
    - **Shares:** the world's own layers at most about 0.2 s per game day, so the world alone reaches 10 game years a minute on two cores; animals near people within each person's budget (`MND-15`); making areas at most a tenth of the simulation's time.
    - **Smooth:** target 60 frames a second, with a 30-frame mode that keeps the same world pixel size.
      With the world running, at least 97% of frames on time while pinching and panning, at every zoom, and none more than 50 ms late (`PRN-11`).
    - **Memory:** within about 8 GiB (`PLT-01`), with kept areas at most about 1 GiB in a full pace-test world at Year 250 (`WLD-12`).
      Full geography (`MIL-14`) retains its 2 GiB peak-process working limit and separate graphics limits; final-game and kept-area limits remain unchanged.
    - **Storage:** within the target of `PLT-10`.
    - **Sound:** the 32-sound mix within its limit (`SND-01`).
    - **Battery and heat:** an hour's play uses about 25–30% of the battery, and the phone never gets uncomfortably hot.
    - **Opening and land:** your world opens in about 3 seconds, a new world in `WLD-11`'s times, and a new area within about a tenth of a second.
  - **How it works:**
    - **Benchmark worlds:** a camp of about 30 and a village of about 300 at close camp zoom, a camp in thick forest at camp zoom with the camera panning, and worlds of about 100, 500, 1,000, 2,000, 3,000 and 7,000 people with about 10 km² each, on generated land; with complete ecology, also the world with nobody in it.
      Each runs to a set game date, its end state in the result code (`RES-05`).
    - **Old worlds:** with complete society, the pace-test worlds at Year 75; with the full arc, those at Year 250.
      Compare each against a new world with as many people (`TIM-07`) and read its kept areas against the memory limit.
    - **Held speed:** speeds are read after at least 3 minutes at full load, unplugged, battery and heat from the phone's counters.
    - **When:** in the cloud at every alpha (`PLT-05`), and on the phone at every stage: one tap, about 20 minutes, and a short result code to send back.
      Compare the look and sustained performance on a weaker phone when one is named and available.
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

- `RES-01` **Tests lead** *(Decided)*: Every alpha brings automated tests for what it adds, quick ones run before work joins and long ones in the background of the session that builds (`PRC-10`).
  A feature counts as built only when its tests pass, and every test names the IDs it checks.
  - **Check:** the coverage check finds a test for every feature and rule built so far (`PRC-12`).

- `RES-21` **Scenes, then whole worlds** *(Decided)*: Most checks use unscripted sandbox scenes cut from generated land, about 10 km across for one band and wider as needed.
  Include decoys/unplanned things; check no unknown-blueprint or unseen-fact use (`PRN-01`).
  Whole worlds confirm scene outcomes occur in play (`RES-07`).
  - **Done when:** every stage-report result names its scenes and whole-world confirmation (`RES-06`).

- `RES-18` **Same rules as play** *(Decided)*: Scenes use the same rules, minds and catalogues as play, and nothing in a scene is scripted.
  A result that needs a switch (`RES-10`) or a scripted event doesn't count as passing in play (`PRN-12`).
  - **Check:** scenes and play run on one game, and every run records any switch it used.

- `RES-09` **Pass rules come first** *(Decided)*: Before first run, every test specifies IDs, scene/worlds, runs, session-hour budget and exact numeric pass rule.
  Never loosen a rule in the change that makes it pass.
  Owner-approved rules require your OK to loosen; others require reasons checked by independent milestone review (`PRC-09`); pace windows require you (`TIM-19`).
  - **Check:** review sees every pass-rule change and reason.

- `RES-13` **About 20 runs where chance matters** *(Decided)*: Chance checks use 20 seeds and counted pass rules, such as at least 16/20.
  On failure, rerun 20 fresh seeds before blocking; apply rule to all 40 (at least 32/40).
  Down to 10 runs may scale the rule, rounding against passing, and count provisionally, named in stage report (`RES-06`); increase runs only when 20 cannot distinguish pass/fail.
  Full pace uses 10 worlds by design, not provisionally; on failure add 10 and judge all 20 (at least 10 within window, at most 5 early).
  Non-blueprint promised shares use at least 1,000 cases: chances ≥1/20 within a third either way, rarer within half to double; blueprint chance uses `RES-24`.
  - **Check:** report run count and range, such as "fire in 18/20 worlds around Year 15".

- `RES-05` **Repeatable runs** *(Decided)*: On the same build, a run from the same saved world gives the same result every time, on the phone and in the cloud alike, so any failure can be replayed step by step (`TIM-16`, `PLT-05`).
  - **Check:** the repeat check before work joins (`PRC-10`), and at every stage the benchmark worlds (`PLT-04`) end exactly the same on the phone and in the cloud.
    A difference blocks the stage until it is found; settling for matching ranges needs your OK.

- `RES-10` **Switch-off runs** *(Decided)*: To find out what a result depends on, a scene can run with one thing switched off, such as teaching, dreams or a personality trait.
  Switches exist only in tests (`PRN-12`).
  - **Done when:** every run using a switch says so in its report and its world (`PLT-05`), and the play build has no switches.

- `RES-16` **Tuning the pace** *(Decided)*: Tune only chances/amounts identically across worlds (`PRN-17`, `MND-11`).
  Tune late steps in scenes where possible; whole worlds confirm.
  Use 20 fixed seeds shared with long runs; each closing pace test draws fresh never-tuned seeds.
  Log every tuned value, including discovery factors, with its tuning cases.
  If tuning fails, report redesign, new window or acceptance options.
  - **Done when:** log covers every tuned value; no closing seeds occur in it.

### 14.2 The tests

- `RES-24` **Blueprint trials** *(Decided)*: Directly try each blueprint about 200 times each at low/high levels, supplied inputs and no choosing mind.
  Results, times, failures and leftovers match `MAT-04`; successes lie in the 99% range for 200 tries (60%: 100–140; 5%: 3–19).
  Catalogue trials never prove play behaviour (`RES-18`); run before work joins (`PRC-10`).
  - **Done when:** all blueprints have trials; wrong chances/results fail them.

- `RES-23` **Every chain and behaviour has a scene** *(Decided)*: Each chain of the arc (`MAT-22`) and each everyday behaviour, such as a band fleeing a predator, has a scene giving people their own reasons.
  A chain's scene passes if the chain completes in at least half of 20 runs within a stated time.
  - **Done when:** the coverage check finds a trial for every blueprint and a scene for every chain and behaviour (`PRC-12`).

- `RES-02` **The sharp-stone test** *(Decided)*: A band with the starting kit (`BIO-02`), which has never made a sharp flake, lives by a river with stone anvils, nuts to crack, carcasses to butcher, hides and wood to work, and flint among decoy stones such as granite.
  - **How it works:** it runs 20 times, each until a year after the first flake, or 3 years if none comes; a control scene, the same with no stone that flakes, runs 4 years.
    The first flake's year and route come from the book of ages (`MAT-21`), and who can make flakes from each adult's skills (`MND-06`).
  - **Done when:** the scene and its control run in the cloud at every stage from complete stone discovery and learning in `MIL-10`, judged by `RES-03`.

- `RES-03` **Sharp-stone pass rule** *(Decided)*
  - **Discovery:** flakes are discovered within 2 years in at least 16/20 runs.
  - **Spread:** in those runs, at least 3 in 4 of the band's adults can make flakes within a year of the first.
  - **Routes:** at least two routes of discovery appear across the runs (`MND-11`).
  - **Control:** without stone that flakes, no run ever makes a flake (`RCK-01`).
  - **Check:** the sharp-stone test (`RES-02`) passes at every stage from complete stone discovery and learning in `MIL-10` (`PRC-10`).

- `RES-07` **The pace tests** *(Decided)*: Whole worlds from the play generator (`WLD-10`) check each pace target (`TIM-19`) and culture target (`CUL-33`) in their records.
  - **Owner OK:** 8 October 2026.
    Each pace gate begins when its complete behaviour arrives; populations, run lengths, seeds and pass rules remain unchanged.
    Early subsets leave full acceptance open.
  - **Sizes,** each judging the targets whose windows close within its years: after any change to minds, blueprints or catalogues, 20 worlds to Year 30, and at each other stage close, the same on new seeds (`RES-16`); when full society is built (`MIL-15`), 20 worlds to Year 75; and the full test, 10 worlds to Year 250, before `MIL-17` closes and at most once a week of building during it.
  - **Stages:** test each complete behaviour when built: flakes and fire begin with the discovery slice (`MIL-10`); full clothing, huts and society, including shared spirits, rites, myths, dogs, band splits, festivals, feuds, new peoples, raids and Keeps going, remain due in `MIL-15`; pottery, herding, villages, farming, copper and chiefs in `MIL-17`.
    `RES-25` begins with complete history and live moments in the small finish (`MIL-13`); partial mechanics do not pass full play-generator gates.
  - **Computing:** in the session that builds, in the background while it works, never in another session (`SCP-15`): the size after a change takes about 1 session-hour and the Year-75 run about 3, and the full test about 10–25, spread over the last alphas of `MIL-17`, whose stage closes once it has run; each stage report sets the real cost against these (`RES-06`).
  - **Other whole-world checks** read these worlds, at the first size that reaches their years (`BIO-04`, `BIO-06`, `BIO-22`, `MAT-08`, `PRE-39` and the culture items); checks of the world with nobody in it, such as `WLD-15` and `WLD-18`, run in the background at the stage that builds them, and again only when their rules change.
  - **Check,** for each target: at least half the worlds reach it inside its window, and at most a quarter before it opens (with 10 worlds, at least 5 and at most 2); where windows overlap, the steps don't come in the same order in every world.

- `RES-25` **Something to watch** *(To test)*: From Year 5 on, whole worlds average at least one new entry in the book of ages per year, and no 5 years pass without an event important enough for a live moment (`PRE-08`), in at least 16 of 20 worlds.
  - **Check:** the pace tests read it (`RES-07`), and stage reports chart entries and live moments per year (`RES-06`).

- `RES-17` **Signature moments keep happening** *(Decided)*: Each moment has its Check's scene, stage and window; at least 2/20 runs produce it unless its own rule differs.
  Rerun changed dependencies before work joins for windows ≤10 years; longer scenes run in background before stage close (`PRC-10`, `RES-07`).
  - **Check:** reports give every moment's latest result and whole-world appearances (`RES-06`).

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

- `RES-06` **Stage reports** *(Decided)*: Each stage ends with a short phone page and repository copy, independently reviewed (`PRC-09`):
  - additions and what you can try;
  - results/charts, especially pace and watchability (`RES-07`, `RES-25`), test location/computing (`SCP-15`);
  - phone measurements (`PLT-04`);
  - moments, oddities, surprises and every principle's Check result (`RES-17`, `RES-12`, `PRN-16`);
  - risks and content production against future needs (`RSK-25`);
  - your judgement checklist (`RES-22`).
  Links open named worlds/entries.
  - **Done when:** every closed stage has every part above, readable on phone with working links.

## 15. Project and process

How the project is run.

### 15.1 Roles

- `PRC-01` **Passion project, built by AI** *(Decided)*: You direct; AI agents write, test and review the code, with no running costs beyond the AI sessions.

- `PRC-02` **Your role** *(Decided)*: You play the alphas when you like, review each stage (`RES-22`), set direction and approve changes to this file.
  - **Check:** every change to this file names your OK in its commit (`PRC-07`).

- `PRC-03` **Technology** *(Decided)*: Chosen by the AI from what the pre-tests measured (`PRC-08`), set out in the architecture for your approval.
  - **Check:** the architecture's technology proposal records your approval before building starts.

### 15.2 Documents

- `PRC-04` **Three main documents** *(Decided)*: PROJECT.md decides what; ARCHITECTURE.md records how and why; IMPLEMENTATION.md sets order, maps every item and details only the next milestone's alphas and tasks.
  - **Owner OK:** 8 October 2026.
  Short durable decision and evidence records, linked from these authoritative files, and the requested critique may also be kept.
  Code and tests name their IDs and are the index of where each is done (`PRC-12`); no document duplicates that index.
  - **Check:** each stage review checks that the three documents exist and this file holds no implementation details.


- `PRC-06` **A guide for AI agents** *(Decided)*: Every agent first reads the short repository guide, CLAUDE.md.
  - **Owner OK:** 8 October 2026.
  One builder owns the integrated playable result, with at most one bounded supporting task.
  The coordinator may settle implementation choices within approved scope.
  The guide requires agents to read this file, follow its principles, link work to IDs and never mark anything Decided without you; changing it needs your OK.
  - **Done when:** the top-level guide states these rules.


- `PRC-07` **Changes to this file** *(Decided)*: AI agents can suggest additions or changes, marked *Proposed*, as How this file works sets out.
  Nothing becomes *Decided*, and no decided item changes, without your OK.
  - **Check:** the commit check confirms that every commit changing this file names the changed IDs and why, and your OK for anything made Decided.

- `PRC-08` **Production** *(Decided)*: Godot with a C++ simulation was chosen (`PRC-03`).
  Pre-production closed on 5 October 2026; foundations (`MIL-08`) were accepted on 6 October.
  Next: repair known failures, then the living camp (`MIL-09`).
  Open questions remain with their items: card/book backgrounds (`PRE-35`), voice and drums when full sound arrives (`SND-03`, `SND-02`, `MIL-16`), and hobbyist signing (`PLT-06`).

### 15.3 How work flows

- `PRC-09` **Branches, checks and review** *(Decided)*: Agents use separate branches; main requires all automatic checks and review approval (`PRC-10`).
  - **Builder:** review each lettered step and numbered alpha against claimed items, rerun checks and make each new test fail once; final step reviews the whole alpha likewise.
  - **Milestone:** once at end, a fresh independent agent reviews the full milestone, tests/results and claimed What/Done when/Check, without builder reasoning, and checks that no test was weakened to pass (`RES-09`).
  - **Disagreement:** after one fixing round, a second fresh reviewer decides; decision-meaning changes go to you.
  - **Check:** no failing or unapproved work joins main.

- `PRC-10` **The checks** *(Decided)*
  - **Before any work joins the main version,** run the routine check: formats and language lints, host builds and quick tests (`RES-01`), the accepted simulation proof suites on one and four threads (`RES-05`), catalogue validation (`MAT-17`, `RCK`), Godot import, script and interface tests, tool tests, the file and commit checks (`PRC-07`) and the coverage check (`PRC-12`). A delivery also checks its note and its already signed APK's checksum, signature, version and packaging, without requiring a second export.
  - **Separate audit:** the additional compiler and emulated processor builds, sanitizer and randomized-order runs, exhaustive recovery tests, scene and repeat stress, native static analysis and full scenario benchmark run before a milestone closes and when a change to simulation arithmetic, threading, persistence or compiler settings needs them. They do not delay unrelated coding or every delivery. Blueprint trials (`RES-24`) and the scenes for changed physical rules (`RES-23`, `RES-17`) still run with those changes. Known failures are still resolved before work joins.
  - **When a background run ends:** its results are read before anything else.
    A check that passed before and now fails is fixed, or the change behind it undone, before other work joins.
    A pace target not yet met goes to tuning (`RES-16`) and blocks only the stage close.
  - **Before a stage closes:** the pace tests (`RES-07`), the phone measurements (`PLT-04`), the phone and cloud match, the moment scenes that are due, the writer's trap records (`PRE-17`), the visual and sound reviews (`PRE-31`, `SND-12`), the report (`RES-06`) and your review (`RES-22`).
  - **Check:** the checks run by themselves, and any failure blocks the join or the close.

- `PRC-11` **Each alpha reaches your phone** *(Decided)*: Every alpha (`SCP-03`) ends with a build to install and try on the phone (`PLT-06`) and a short note: what is new, what to try and what is still rough.
  Only the stage reviews wait for you (`RES-22`).
  - **How it works:** the note is a page with the download link, and the build opens your worlds unless the note says it is a big update (`PLT-09`).
    The first time an alpha opens, it runs a self-check of a few seconds and, if anything fails, shows a short code to send back.
  - **Check:** every alpha's note links its build and names the IDs it delivers, and no check that passed before was failing on its build.

- `PRC-12` **Nothing gets lost** *(Decided)*: Before work joins and at stage close (`PRC-10`), the automatic ID check verifies traceability; review verifies behavioural acceptance.
  - **Owner OK:** 8 October 2026.
  - Every non-Proposed feature/rule is built or mapped to a stage; current-stage items have tasks.
  - Tasks name their IDs; all code/test IDs exist and are live.
  - Built features/rules require tests (`RES-01`), blueprints trials (`RES-24`), chains scenes (`RES-23`), and promises tests or owner-approved "possible, not promised" marks (`RES-19`).
    The automatic check verifies recorded links; reviewers verify test meaning and results.
  - Changed requirements flag linked work for rechecking.
  - Staged subsets may share a stable ID; delivery notes state proved behaviour and remaining checks.
    Full acceptance remains open until every obligation passes; ID coverage alone never proves completion.
  - **Done when:** the ID check rejects an unmapped feature and a test with a retired ID; catalogue acceptance rejects a blueprint lacking its trial.

## 16. Risks

Stage reports review these risks; AI agents may update the ratings there (`RES-06`).

### 16.1 The game itself

- `RSK-01` **Discoveries stall** *(Decided)*: Likelihood/impact: medium/high.
  - **Signs:** the sharp-stone test failing (`RES-03`); pace tests stuck before fire (`RES-07`).
  - **Response:** switch-off runs (`RES-10`); tuning, never scripting (`RES-16`).

- `RSK-26` **The pace is off** *(Decided)*: Likelihood/impact: high/high.
  - **Signs:** pace tests off their windows, or every world in one order (`RES-07`).
  - **Response:** tuning in scenes (`RES-16`); several routes to a result (`MAT-07`); windows changed with you (`TIM-19`).

- `RSK-19` **Belief fails to emerge** *(Decided)*: Likelihood/impact: medium/high.
  - **Signs:** culture targets missed (`CUL-33`); no religion (`CUL-26`).
  - **Response:** one capped belief rule (`MND-05`) and templates (`CUL-05`); belief scenes when full belief rules arrive in `MIL-15` (`RES-19`).

- `RSK-06` **Blueprints give absurd results** *(Decided)*: Likelihood/impact: medium/medium.
  - **Signs:** catalogue checks or trials failing (`MAT-17`, `RES-24`); oddities (`RES-12`).
  - **Response:** reality rules and the expected-fits check (`RCK`, `MAT-17`).

- `RSK-07` **People know what they can't** *(Decided)*: Likelihood/impact: medium/high.
  - **Signs:** choices using unknown blueprints or unseen places (`RES-21`); texts with facts the records lack.
  - **Response:** `PRN-01` and `PRN-06`, checked in scenes and by the writer's check (`PRE-41`).

- `RSK-27` **People act oddly** *(Decided)*: Likelihood/impact: high/medium.
  - **Signs:** dithering, starving beside food or all doing one thing (`RES-12`).
  - **Response:** everyday scenes from the living camp (`MIL-09`), extending with each behaviour (`RES-23`); reasons kept for what each is doing and every saved event (`MND-09`).

### 16.2 The experience

- `RSK-03` **Real but dull to watch** *(Decided)*: Likelihood/impact: medium/high.
  - **Signs:** worlds falling short of `RES-25`; you skimming the book of ages.
  - **Response:** the story director, live moments and following someone (`TIM-02`, `PRE-08`, `PRE-06`).

- `RSK-08` **The writer falls short** *(Decided)*: Flat or softened text, or changed or removed by a phone update; likelihood high, impact medium.
  - **Signs:** entries alike or vague; texts changing after a phone update.
  - **Response:** pattern sentences, which the writer only rewords, state dark events plainly and show if it is missing (`PRE-37`, `PRE-17`).

- `RSK-11` **Pixel look hard to keep clean** *(Decided)*: Likelihood/impact: medium/medium.
  - **Signs:** visual reviews failing on shimmering texture pixels, speckled ground or people hard to find (`PRE-31`).
  - **Response:** art drawn for its source families, with reviewed reductions (`PRE-22`), small things designed for each band (`PRE-46`), and people found by real light and the approved reveal (`PRE-28`).

- `RSK-28` **Sound falls flat** *(Decided)*: Likelihood/impact: medium/medium.
  - **Signs:** a camp sounding thin or fake in your reviews (`SND-12`).
  - **Response:** deep sounds lifted for the speaker, sounds tuned with you (`SND-06`); the voice chosen by ear (`SND-03`).

### 16.3 The phone

- `RSK-02` **Too slow at 2,000 people** *(Decided)*: Likelihood/impact: medium/high.
  - **Signs:** a stage missing its budget or making areas over its share (`PLT-04`); 2,000 people before Year 150 (`BIO-04`).
  - **Response:** past about 2,000, time slows and the game says so (`MND-15`).

- `RSK-15` **Worlds outgrow the phone** *(Decided)*: Likelihood/impact: medium/medium.
  - **Signs:** memory, kept areas or saves past their targets (`PLT-04`, `PLT-10`).
  - **Response:** kept areas hold only fading changes (`WLD-12`); history thins; worlds pause at the memory limit (`MND-15`).

- `RSK-04` **Runs stop repeating** *(Decided)*: Likelihood/impact: high/high.
  - **Signs:** repeat or phone-and-cloud checks failing (`PRC-10`, `RES-05`); a crash replaying differently.
  - **Response:** the simulation only on the processor cores, by the architecture's rules (`PLT-01`, `PRC-08`).

- `RSK-21` **Updates harm worlds** *(Decided)*: Likelihood/impact: medium/high.
  - **Signs:** a world failing to open, or jumping in state, after an update.
  - **Response:** a safety copy, the change marked, and tests opening older saves (`PLT-09`, `PRC-10`).

- `RSK-29` **The signing key is lost** *(Decided)*: Likelihood/impact: low/high.
  - **Signs:** a build that won't install over the last one; removing the app deletes its worlds.
  - **Response:** one key, two copies (`PLT-06`); worlds exported before any reinstall (`PLT-08`).

- `RSK-18` **New install rules** *(Decided)*: Likelihood/impact: medium/low.
  - **Signs:** from 2027, installs blocked or warned against (`PLT-06`).
  - **Response:** signing for your hobbyist account; the advanced unlock or a USB cable as fallbacks.

- `RSK-24` **The phone ages or is replaced** *(Decided)*: Likelihood/impact: low/medium.
  - **Signs:** the battery over about 40 °C in long sessions, or its health falling.
  - **Response:** time slows rather than the phone overheating (`PRN-11`, `PLT-04`); worlds move by export (`PLT-08`).

- `RSK-30` **The look costs too much** *(Decided)*: The look you chose may need more of the phone's graphics chip than it has; likelihood high, impact high.
  - **Signs:** the busiest close scene over its time line on your phone, or the phone too warm in a long session (`PLT-04`).
  - **Response:** first the savings you cannot tell from the full picture (`PRE-01`); then, as you agreed, changes to the game engine's own code where those are not enough (`PRC-03`); then one planned step under heat that also passes the blind test; the world's density is never cut.

### 16.4 The project

- `RSK-25` **Too much content** *(Decided)*: The launch content takes too long to make and check: items, blueprints, plants, animals and illnesses (`MAT-13`), signs (`PRE-46`), movements and dance moves (`PRE-44`), kinds of thought (`MND-29`), pattern sentences (`PRE-37`) and base sounds (`SND-06`).
  Likelihood high, impact high.
  - **Signs:** content made per alpha short of what later stages need (`RES-06`).
  - **Response:** stages add only what their steps need (`SCP-16`).
    A fallback launch set is agreed now and marked in the catalogues: about two thirds of each count (about 125 items, 95 blueprints, 40 plants, 20 wild animals and 10 illnesses), keeping every result another item names, a full chain to every step of the arc and, for each kind of land, a grazer, a hunter, a food plant and a fibre plant, and passing every catalogue check on its own (`MAT-17`).

- `RSK-05` **The scope never ends** *(Decided)*: Likelihood/impact: high/medium.
  - **Signs:** stages slipping; alphas adding little you can see.
  - **Response:** short alphas in stages with fixed goals (`PRN-09`, `SCP-16`); the arc ends at copper (`VIS-03`); cuts stay cut (`SCP-21`).

- `RSK-09` **AI-built code drifts** *(Decided)*: Likelihood/impact: medium/high.
  - **Signs:** coverage gaps; behaviour contradicting this file; tests quietly weakened.
  - **Response:** the guide, coverage check, firm pass rules, review and modules (`PRC-06`, `PRC-12`, `RES-09`, `PRC-09`, `PRN-14`).

- `RSK-14` **Tests too big for the cloud** *(Decided)*: Likelihood/impact: medium/medium.
  - **Signs:** long runs unfinished or lost; a pace test over its stated budget (`RES-07`).
  - **Response:** scenes wherever they can answer (`RES-21`, `RES-16`); runs that resume (`PLT-05`).

- `RSK-23` **Your time** *(Decided)*: Likelihood/impact: medium/medium.
  - **Signs:** stages waiting on your reviews; alphas piling up untried.
  - **Response:** only stage reviews wait for you (`PRC-11`), with a short list and report (`RES-22`, `RES-06`).

## 17. Not yet decided

### 17.1 Settled by measurement

<!-- generated: open items -->
- **Speed target** (`TIM-07`): measurement remains open.
- **Pace of discovery** (`TIM-19`): measurement remains open.
- **How many people it can feed** (`WLD-04`): measurement remains open.
- **Generation time** (`WLD-11`): measurement remains open.
- **Population limit** (`MND-15`): measurement remains open.
- **Pace of culture** (`CUL-33`): measurement remains open.
- **Something to watch** (`RES-25`): measurement remains open.
<!-- end generated -->

### 17.2 Proposals awaiting confirmation

AI agents' suggestions are marked *Proposed* and listed here until you confirm, change or drop them (`PRC-07`).

<!-- generated: proposals -->
- None at present.
<!-- end generated -->

## 18. Glossary

Terms are defined in their source items; this index avoids a second set of definitions.

| Term | Definition |
|---|---|
| Activity | `TIM-17` |
| Alpha | `SCP-03` |
| Area | `WLD-12`, `WLD-13` |
| Band | `CUL-30` |
| Base action | `MAT-06` |
| Belief | `MND-27` |
| Belief template | `CUL-05` |
| Blueprint | `MAT-04` |
| Book of ages | `PRE-05` |
| Catalogue | `MAT-13` |
| Characteristic | `MAT-03` |
| Close camp | `PRE-03` |
| Count | `WLD-32` |
| Custom | `CUL-06` |
| Details view | `PRE-14` |
| Discovery | `MND-11`, `MAT-21` |
| Domestic kind | `WLD-33` |
| Dream | `GOD-03`, `GOD-12` |
| Experience | `MND-06` |
| Festival | `CUL-29` |
| Game year | `TIM-18` |
| Gathering | `CUL-29` |
| Heat level | `MAT-18` |
| Held speed | `PLT-01`, `PLT-04` |
| Hunch | `MND-11` |
| Item | `MAT-10` |
| Kept area | `WLD-12` |
| Level | `MAT-04` |
| Live moment | `PRE-08` |
| Material class | `MAT-01` |
| Mental map | `MND-28` |
| Milestone | Stage |
| Mood | `MND-29` |
| Motif | `PRE-46` |
| Murmur | `SND-03` |
| Named result | `MAT-04` |
| Need | `BIO-09`, `MND-07` |
| Pace target | `TIM-19` |
| Pace test | `RES-07` |
| Pattern sentence | `PRE-37` |
| People (a people) | `CUL-23` |
| Plain use | `MAT-06` |
| Power | `GOD-05`, `GOD-13` |
| Revelation | `GOD-13` |
| Quality | `MAT-20` |
| Reality rule | `RCK` |
| Region | `PRE-03`, `WLD-24` |
| Rite | `CUL-34` |
| Scene | `RES-21` |
| Season | `TIM-18` |
| Sector | `MND-06` |
| Seed | `WLD-13` |
| Signature moment | `MOM` |
| Skill | `MND-06` |
| Stage | `RES-22`, `SCP-16` |
| State | `MAT-19` |
| Story director | `TIM-02`, `TIM-03` |
| Style | `CUL-12` |
| Taming | `WLD-33`, `RCK-24` |
| Texture pixel | `PRE-22` |
| Thought | `MND-29` |
| Timer | `MAT-19` |
| Top speed | `TIM-01` |
| Topic | `CUL-24` |
| Trial | `RES-24` |
| Village | `CUL-28` |
| Wear | `MAT-20` |
| Weather cell | `WLD-16` |
| World | `WLD-03` |
| World cell | `WLD-12` |
| Writer AI | `PRE-37`, `PRE-17` |
