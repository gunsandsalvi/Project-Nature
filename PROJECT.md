# Project Nature

The project file: what Project Nature is, and every feature and target it must reach. It is written so that someone with no prior context can read it and understand the whole project.

It contains no implementation details. Those belong in the implementation plan, which will link back to this file by ID, as will the code. Every item has a permanent ID so that no feature gets lost on the way from idea to code.

**Version 2** · 2026-10-01

**Progress:** section 1 is written in full detail. Sections 2–18 are still in their first, short form and are being expanded one at a time, in order.

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
- [Change log](#change-log)

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

### IDs and links

1. Every feature and target has a permanent ID: an area code plus a number, such as `WLD-01`.
2. IDs are never renumbered or reused. A new item takes the next free number in its area, wherever it sits in the text.
3. Items are never deleted. If something is cut, its status becomes *Dropped* with a one-line reason.
4. The implementation plan (a separate file, still to come) names the IDs each task delivers. Every ID that isn't *Dropped* must appear in at least one task.
5. Code and tests name the IDs they implement, so any feature can be followed from this file to the plan to the code, and back.
6. Every change to this file is recorded in the [change log](#change-log).

### Area codes

| Code | Area |
|---|---|
| `VIS` | Vision |
| `MOM` | Signature moments (part of the vision) |
| `PRN` | Principles |
| `SCP` | Scope and non-goals |
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

- `VIS-16` **Name** *(Open)*: "Project Nature" is the working title. Shortlist:
  - **Kindling:** what a fire grows from; small things that catch and spread, like knowledge.
  - **Strata:** layers of rock and layers of history; deep time you can dig through.
  - **Ochre:** humanity's first pigment, and one of the oldest signs of art and symbolic thought.
  - **Untaught:** everything they know, they learned with no one to teach them.
  - **The Long Dawn:** the long beginning of humanity, watched with wonder.
  - **Unseen:** you, the god they never see, and the unseen beings they invent to explain the world.

## 2. Principles

These rules apply everywhere. If a decision conflicts with one of them, the principle wins unless this file is changed.

- `PRN-01` **The world is the only teacher** *(Decided)*: Everything people know comes from their senses, their own trials, other people or their dreams. No recipes, no tech tree, no scripted discoveries.
- `PRN-02` **Depth over breadth** *(Decided)*: A small world simulated deeply beats a large shallow one.
- `PRN-03` **You are nature** *(Decided)*: The player acts only through natural means and is never known to exist.
- `PRN-04` **If the simulation knows it, you can see it** *(Decided)*: Any view the simulation's data can support may exist.
- `PRN-05` **Real numbers, testable claims** *(Decided)*: The physics uses real-world measurements. Every claim about what emerges is checked by experiments that can fail.
- `PRN-06` **AI language models describe, never decide** *(Decided)*: They turn simulation data into readable text. They never choose, invent or know anything on behalf of the people of the world.
- `PRN-07` **No hidden recipes** *(Proposed)*: The vocabulary of a discovery (flake, knapping, fire-making, pottery and so on) never appears in the logic that makes people or animals decide. It may appear only in descriptions of matter and in text written for the player.
- `PRN-08` **Same seed, same history** *(Proposed)*: A history is fully determined by its world's seed and the player's interventions. It plays out identically on the phone and in the cloud.
- `PRN-09` **Only as deep as the next experiment needs** *(Proposed)*: Each system is built to the depth the next experiment requires, on foundations that can go deeper later.

## 3. Scope and non-goals

- `SCP-01` **Starting point** *(Decided)*: Modern minds with very little culture (details in `BIO-02`).
- `SCP-02` **Audience** *(Decided)*: Just you. No public release, onboarding or support for other devices is planned.
- `SCP-03` **Build order** *(Decided)*: Experiments lead (`RES-01`), and a basic phone app grows alongside them.

**Non-goals**

- `SCP-04` **No recipes or tech tree** *(Decided)*.
- `SCP-05` **No other human species** *(Decided)*: There is one human species only.
- `SCP-06` **No AI language model making decisions inside the simulation** *(Decided)*.
- `SCP-07` **No goals, scores, wins or losses** *(Decided)*: It is a pure sandbox.
- `SCP-08` **No worship of the player** *(Decided)*: The player's power does not depend on faith, and the humans never learn the player exists.
- `SCP-09` **No terraforming power** *(Decided)*: The player cannot reshape land, or add or remove species.
- `SCP-10` **No shared online world or multiplayer** *(Decided)*.
- `SCP-11` **No real-Earth map** *(Decided)*: Every world is generated.
- `SCP-12` **No simulated planet formation** *(Decided)*: Worlds are generated directly in a realistic present-day state.

## 4. The player as god

- `GOD-01` **Role** *(Decided)*: A distant, invisible god in a pure sandbox. You nudge; you never command anyone.
- `GOD-02` **Power: nature and disasters** *(Decided)*: Weather, storms, floods, droughts, eruptions and lightning, which might hand them fire. Always within the world's physics.
- `GOD-03` **Power: dreams** *(Decided)*: Plant a dream in one person's sleep. A dream can only recombine things that person has experienced. They still have to work out the "how" themselves.
- `GOD-04` **Power: fortune and fate** *(Decided)*: Bless or curse fertility, health, luck in the hunt, sickness and plague. Luck shifts the odds within what nature allows and never guarantees an outcome.
- `GOD-05` **Only natural means** *(Decided)*: Every act must be something nature could do. No miracles, and no limited supply of power to spend.
- `GOD-06` **Never known** *(Decided)*: People experience your interventions as nature. Any explanation they form, right or wrong, is their own.
- `GOD-07` **No tally of your help in the story view** *(Decided)*: The story view never shows how much you helped.
- `GOD-08` **Interventions recorded behind the scenes** *(Proposed)*: Rewind and branching depend on this record.
- `GOD-09` **Interventions in the scientist's view** *(Open)*: Whether the scientist's view can show your interventions and their effects.
- `GOD-10` **Using your powers on the phone** *(Open)*: Gestures, how you target a place or person, and how you choose the memories a dream combines.

## 5. Time and history

- `TIM-01` **Time follows zoom** *(Decided)*: This is the default. Close up, a day passes in minutes; zoomed out to the whole world, centuries pass in minutes. One gesture controls both space and time.
- `TIM-02` **Story director** *(Decided)*: It slows down for important moments (a first, a death, a war) and races through quiet years.
- `TIM-03` **The director never touches events** *(Proposed)*: It controls speed only. It chooses where to slow down but never makes anything happen.
- `TIM-04` **Manual control** *(Decided)*: You can separate zoom and speed whenever you want.
- `TIM-05` **Pauses when closed** *(Decided)*: The world only moves while the app is open.
- `TIM-06` **Rewind and branch** *(Decided)*: Go back to any moment, change something, then keep both timelines and compare them.
- `TIM-07` **Pacing** *(To test)*: There is no fixed target for how long history takes to watch. It is measured and tuned during development.
- `TIM-08` **Saved worlds and timelines** *(Proposed)*: Keep several worlds and timelines, and switch between them.
- `TIM-09` **If everyone dies** *(Open)*: What happens when humanity dies out in a world.

## 6. World

**Shape and size**

- `WLD-01` **Torus with latitude** *(Decided)*: The map wraps around both east–west and north–south. An equator runs across the middle, and polar ice lies along the line where the map wraps north–south. Climate zones and seasons behave as on a planet, with seasons reversed between the two halves.
- `WLD-02` **Globe view** *(Decided)*: Fully zoomed out, the world is drawn as a globe. The wrap only shows at the poles, which are ice nobody crosses.
- `WLD-03` **Size** *(Decided)*: About 1,000 km pole to pole and about 2,000 km around.
- `WLD-04` **How many people it can feed** *(To test)*: Estimated at roughly 50,000 hunter-gatherers, or around ten million people once farming exists.
- `WLD-05` **Climate on a small world** *(Proposed)*: Climate zones sit closer together than on Earth, a few days' walk apart. Weather follows realistic patterns of rain, temperature and storms for each season, rather than full atmospheric physics.

**The planet**

- `WLD-06` **Varied within reason** *(Decided)*: Each world has its own day length, axial tilt (and so its own seasons), moons, and ratio of land to sea. All stay within ranges that allow human-like life.
- `WLD-07` **The sky** *(Proposed)*: The sun, moon or moons, stars and planets move realistically for each world. They are the raw material for calendars, navigation and myth.

**Making a world**

- `WLD-08` **Realistic, not from scratch** *(Decided)*: Worlds are generated directly in a realistic present-day state, and generating one is cheap.
- `WLD-09` **What generation covers** *(Proposed)*: Tectonic plates and mountain ranges, erosion, rivers and lakes, dry land on the sheltered side of mountains, soils, landscapes, and minerals and ores in geologically plausible places.
- `WLD-10` **Generate many, keep the best** *(Decided)*: The generator makes many candidate worlds and scores them for varied landscapes, natural barriers and unevenly spread resources (flint here, copper there). It keeps the best. Choosing, never editing.
- `WLD-11` **Generation time** *(Proposed)*: About a minute per candidate world on the phone.

**Detail**

- `WLD-12` **Detail where it matters** *(Proposed)*: Coarse where nothing lives and no one is looking. Fine, down to the metre, where people are or where you look.
- `WLD-13` **Nothing changes when you look away** *(Proposed)*: Fine detail is generated the same way every time, so looking away and back changes nothing.

**Natural systems** (all simulated in depth)

- `WLD-14` **Geology and materials** *(Decided)*: Rocks, minerals, soils and ores in realistic places. What can be discovered depends on what's underfoot.
- `WLD-15` **Living geology** *(Proposed)*: Slow change continues during play: erosion, landslides, earthquakes, eruptions, rivers shifting course.
- `WLD-16` **Climate and weather** *(Decided)*: Seasons, storms and droughts. Over thousands of years, ice ages and warm periods move coastlines and push migrations.
- `WLD-17` **Water** *(Decided)*: Rivers, lakes, springs, underground water and floods. Life and settlement gather around them.
- `WLD-18` **Ecology** *(Decided)*: Plants and animals in food webs, with migrations, and populations that boom and crash. What's edible, dangerous or tameable.
- `WLD-19` **Species from Earth families** *(Decided)*: Earth's families of plants and animals are the starting point. Generation adapts them into each world's own species to fit its landscapes.
- `WLD-20` **Heredity in plants and animals** *(Proposed)*: Inheritance continues during play, so adaptation and domestication (wolves into dogs, wild grasses into grain) can happen on their own.
- `WLD-21` **Microbes** *(Proposed)*: Rot, fermentation and disease are microbes at work, and they are part of the same world.
- `WLD-22` **Natural disasters** *(Proposed)*: Eruptions, earthquakes, floods, droughts, storms, wildfires and lightning come from the world's own systems, not only from the player.

How animals think is covered in `MND-16`.

## 7. Matter and physics

- `MAT-01` **Made of real ingredients** *(Decided)*: All matter is built from a few dozen real ingredients with real chemical makeup: for example silica, limestone-type minerals, clays, metal ores, water, salts, plant fibres, starches, sugars, proteins, fats, collagen and tannins. Results come from how these interact, not from rules written for each material.
- `MAT-02` **Structure matters** *(Proposed)*: Matter also records how it's put together: crystal or glass, fibrous, porous or dense, coarse or fine grain, wet or dry. Sand, flint and obsidian are all mostly silica, but only flint and obsidian chip into blades.
- `MAT-03` **Properties are derived** *(Decided)*: Hardness, strength, nutrition, toxicity, and how something breaks, burns or melts all follow from what it's made of and how it's put together.
- `MAT-04` **A few dozen general laws** *(Decided)*: Change comes from general laws, each using real temperatures and conditions. No law names a product. A starting list (*Proposed*): burning, charring, melting, freezing, drying, dissolving, mixing, breaking, wearing away, rotting, fermenting, cooking, smelting.
- `MAT-05` **Real-world values** *(Decided)*: Temperatures, hardness, energy content, toxicity and every other number come from real measurements.
- `MAT-06` **Actions are physical** *(Decided)*: Every action has force, angle, speed, duration and temperature, and the physics decides the outcome. Technique matters: a clumsy strike shatters the stone.
- `MAT-07` **One law, many inventions** *(Proposed)*: Laws are general enough that one law covers many inventions. For example, "metal ores give up their metal when heated hot enough in contact with burning charcoal" covers copper, tin, lead and iron. Each needs its own real conditions, so they become possible in a natural order that nobody wrote down.
- `MAT-08` **Traces last** *(Decided)*: Hearths, tools, bones, graves and rubbish heaps stay in the world and get buried over time. They feed the archaeology view (`PRE-09`).

### Reality checklist

The physics must reproduce every item below without any rule written specially for it. All items are *Proposed*; the list will be extended in the matter deep dive.

- `RCK-01` **Flint chips, granite doesn't**: Flint and obsidian chip into sharp flakes; granite doesn't.
- `RCK-02` **Fire by friction**: Rubbing wood fast enough can light dry tinder.
- `RCK-03` **Cooking helps**: Cooking makes food more nourishing.
- `RCK-04` **Pottery needs fire**: Fired clay becomes pottery; sun-dried clay softens again in water.
- `RCK-05` **Lime**: Burned limestone becomes lime.
- `RCK-06` **Leather**: Hides soaked with oak bark become leather instead of rotting.
- `RCK-07` **Fermentation**: Fruit sugars ferment.
- `RCK-08` **Copper needs a furnace**: Copper smelts in a charcoal furnace with forced air, but not over a campfire.
- `RCK-09` **Rot**: Untreated meat and hides rot, faster when warm and wet.

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

**Look**

- `PRE-01` **Detailed pixel art** *(Decided)*.
- `PRE-02` **Art direction** *(Open)*: To be chosen from the art-direction mockups.
- `PRE-03` **Seamless zoom** *(Decided)*: One continuous zoom from the whole world, drawn as a globe, down to one person chipping flint.
- `PRE-04` **Sharp at every zoom** *(Proposed)*: The pixel art stays sharp and readable at every zoom level.

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
  - **No hidden recipe:** the check in `PRN-07` passes.
- `RES-04` **Reality checklist** *(Proposed)*: The physics must pass every `RCK` item before any discovery that depends on it is trusted.
- `RES-05` **Reproducibility** *(Proposed)*: Re-running a seed with the same interventions gives the same history on the phone and in the cloud (`PRN-08`). Checked from the first build.
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
- **The player as god:** `GOD-09`, `GOD-10`
- **Time and history:** `TIM-07`, `TIM-09`
- **World:** `WLD-04`
- **Minds:** `MND-15` (limits)
- **Presentation:** `PRE-02`, `PRE-19`
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
- **Reality checklist:** real-world changes the physics must reproduce without special rules (`RCK`).
- **Signature moment:** a story the simulation must be able to produce without it being scripted (`MOM`).
- **Live moment:** a notable event the game surfaces to you as it happens (`PRE-08`).
- **No-hidden-recipe check:** confirms that no discovery's vocabulary appears in decision-making logic (`PRN-07`).

## Change log

- **2026-10-01 · v1:** First version, from the first sounding-board session.
- **2026-10-01 · v2:** Section 1 (Vision) written in full: one-paragraph description, feelings, rhythms of play, a session told as a story, signature moments (`MOM-01` to `MOM-12`), what makes it different, inspirations, success criteria and a name shortlist. Added the item format and the `MOM` area code.
