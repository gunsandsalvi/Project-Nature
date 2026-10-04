# Research 11: things, crafts and discovery

**Question:** how do games give things materials and let new things follow from rules, not from lists?
How do people in games discover, learn and lose crafts?
What do archaeology's experiments give as real numbers, so that Kindling's blueprints and reality rules are right (`MAT`, `RCK`, `MND-06`, `MND-10`, `MND-11`, `MND-13`, `CUL-01`, `CUL-02`, `CUL-16`, `TIM-19`, `RES-02`, `RES-03`)?

## What `PROJECT.md` asks

- **Things made of materials, with 18 characteristics,** from 0 to 5: hardness, edge, flaking, burn, warmth, stickiness and so on.
  A thing's form and size count as much as its material (`MAT-01`, `MAT-02`, `MAT-03`).
- **21 base actions;** each blueprint uses exactly one (`MAT-06`).
  Blueprints match inputs by characteristic ranges, class and form, never by a named item.
  So a new item fits every blueprint whose ranges it meets (`MAT-04`, `MAT-14`).
- **About 190 items and 140 blueprints** from caves to first copper, with several routes to one result and chains of steps (`MAT-07`, `MAT-22`, `MAT-23`).
- **Reality rules** that must always hold, such as "flint flakes, granite doesn't" and "copper needs a furnace" (`RCK-01` to `RCK-26`).
- **Nobody knows a blueprint until they discover or learn it,** by accident, by experimenting, from a dream's hint, or by copying (`MND-11`).
  - Skill rises with practice, from 0 to 10 (`MND-06`).
  - Crafts pass by watching and teaching (`MND-13`).
  - A craft dies with its last holder (`CUL-02`).
- **The arc's pace comes from tuning alone:** sharp flakes in Years 1–5, fire 5–30, pottery 60–150, copper 300–500 (`TIM-19`).

## How games model things

- **Materials that carry their values:**
  - Dwarf Fortress gives each material a density, and yield and fracture points for impact, shear, bending and other loads ([DF wiki: material science](https://dwarffortresswiki.org/index.php/v0.34:Material_science)).
    A weapon's harm follows from its material: its density for force, and its impact yield and fracture for blunt blows.
  - RimWorld's "stuff" lets "a material … be chosen before creation of an item … and the stats of the chosen material affect the stats of the end product".
    It has five main categories: stony, metallic, woody, leathery and fabric.
    So wood "is more flammable than stone", and stone beds are less comfortable ([RimWorld wiki: stuff](https://rimworldwiki.com/wiki/Stuff)).
- **Recipes that match properties, not names:** a Dwarf Fortress reaction names its reagents by item type plus a material class or flag ([DF wiki: reaction](https://dwarffortresswiki.org/index.php/Reaction)).
  - `[REAGENT:B:1:BOULDER:NONE:NONE][REACTION_CLASS:FLUX]` takes any boulder whose material is a flux.
  - `ANY_STONE_MATERIAL` takes any stone.

  It is the same idea as our blueprints' characteristic ranges (`MAT-04`, `PRN-07`).
- **Rules, not scripts:** Breath of the Wild's "chemistry engine" (GDC 2017, Fujibayashi, Dohta and Takizawa) splits the world into elements (fire, water, ice, electricity, wind) and materials (trees, rocks, weapons).
  It has three rules ([Thumbsticks](https://www.thumbsticks.com/gdc-17-breath-of-the-wild-science-lies), [Tom's Guide](https://www.tomsguide.com/uk/us/zelda-breath-wild-design,news-24586.html)):
  1. elements change materials' states;
  2. elements change each other;
  3. materials cannot change each other.

  "The model is an extremely simple one but allows for the expression of all sorts of events."
  "Creating puzzles with solutions that reference the way the real world works can help players solve them instinctively."
  The third rule keeps the combinations in check, as our expected-fits list does (`MAT-17`, `RSK-06`).
- **Quality from skill and inputs:**
  - Star Wars Galaxies' resources came with varying stats, and each schematic weighs different ones.
    "In order to maximize the outcome of the given item, you must input the qualities of resource that the schematic calls for", and resource quality limits how far experimentation can push the result ([SWG Restoration wiki](https://swgr.org/wiki/crafting), [SWG wiki](https://swg.fandom.com/wiki/Crafting)).
  - In Wurm Online, higher quality lowers "the rate at which items are damaged by use, attack, and decay" and "the time it takes to use or to create something".
    It also raises the chance of success when making things with them ([Wurmpedia: quality level](https://wurmpedia.com/index.php/Quality_Level)).
  - Both match `MAT-20`.
- **A map of crafting systems:** Grow, Dickinson, Pagnutti, Wardrip-Fruin and Mateas (2017) compare crafting on seven dimensions ([DHQ](https://dhq.digitalhumanities.org/vol/11/4/000339/000339.html)):
  - how fixed the recipes are;
  - how detailed the actions are;
  - what resources constrain them;
  - how much the outcome varies;
  - how far other systems "understand" what was made;
  - how much room for expression there is;
  - how crafting changes over time.

  Kindling sits at the far end of most:
  - recipes by ranges;
  - one real action per step;
  - quality varies;
  - every made thing works in the world;
  - progress only by discovery.

## How games handle discovery

- **Ancestors: The Humankind Odyssey** (Panache, 2019) lets you find tools by trying things on objects, with few prompts:
  - "The true beauty of the gameplay lies in these revelations and moments of successful experimentation", but "the game isn't fun unless you're comfortable with figuring the mechanics out on your own" ([Den of Geek](https://www.denofgeek.com/?p=8675)).
  - Its "neuronal energy … works like XP and is used to unlock new abilities".
    Players can be "left feeling aimless" when systems stay unclear ([GameRevolution](https://www.gamerevolution.com/review/583921-ancestors-the-humankind-odyssey-pc-ps4-xbox-one-brutal-survival-game)).
- **Dawn of Man** (Madruga Works, 2019) gives knowledge points for reaching milestones, spent on technologies, "from the Stone Age … up to the Iron Age" ([Steam](https://store.steampowered.com/app/858810/Dawn_of_Man/)).
  Players advise "to maximize more knowledge points on every era before advancing", so the tree rewards repeating tasks, not insight ([Steam discussion](https://steamcommunity.com/app/858810/discussions/0/1648792158829175445)).
- **Civilization VI's eurekas:** doing something related to a technology pays half its cost (40% since the Rise and Fall expansion).
  Killing a unit with a slinger boosts archery, and founding a coastal city boosts sailing ([Steam guide](https://steamcommunity.com/sharedfiles/filedetails/comments/886630163)).
  Our hints are the same idea, but held by one person, not a civilisation (`MND-11`).
- **Learning from materials:**
  - Minecraft's recipe book (version 1.12, June 2017): "you obtain an item, the recipes that consume it light up" ([CraftDex](https://craftdex.net/articles/the-recipe-book-and-unlocks)).
  - "Valheim normally unlocks recipes and building pieces as players discover materials" ([Nodecraft](https://nodecraft.com/support/games/valheim/setup/unlock-all-recipes-and-buildings-on-a-valheim-server)).
- **Learning from made things:** in Caves of Qud, "whenever you disassemble an item, you have a 25% chance to learn how to build the item" with the Reverse Engineer skill ([Qud wiki](https://wiki.cavesofqud.com/wiki/Reverse_Engineer)).
  It is a precedent for copying (`MND-11`).
- **Secrets by seed:** Noita's two rare potions each need three materials that "vary for each run based on the world seed, but typically include two liquids and one powder" ([Noita wiki](https://noita.wiki.gg/wiki/Alchemic_Precursor), [Lively Concoction](https://noita.wiki.gg/wiki/Lively_Concoction)).
  Kindling keeps the real world's rules in every world instead (`MAT-05`), so what you know of nature helps you read it.
- **Sapiens** (Majic Jungle) is the closest in subject, a prehistoric tribe sim: "Find new resources and send your sapiens to investigate, leading to new breakthroughs that unlock new things to build and craft" ([Steam](https://store.steampowered.com/app/1060230/Sapiens/)).
- **Vintage Story** knaps stone and forms clay on a grid of tiny cubes, then fires pots in a pit kiln ([knapping](https://wiki.vintagestory.at/Special:MyLanguage/knapping), [clay forming](https://wiki.vintagestory.at/Clay_Forming)).
  Granite, andesite, chert, basalt, peridotite, flint and obsidian can all be knapped, "Obsidian > Flint > All other stone types".
  That suits play; `RCK-01` forbids it.
- **The lesson:** in every one of these, the player discovers.
  In Kindling, people discover through their own lives while you watch (`PRN-01`).
  No game found does this for whole peoples, so the pace is new ground: it needs a prototype.
  Ancestors and Dawn of Man warn of the two failures: aimless trial and error, and grinding.

## What archaeology's experiments give

### Accidents and learning

- **Accidental flakes are real:** wild bearded capuchins in Brazil break stones and "unintentionally" produce "sharp-edged flakes and cores" that look like early human tools.
  They were "not observed using" them (Proffitt and others, Nature 2016) ([PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC5429872), [Scientific American](https://www.scientificamerican.com/article/monkeys-make-stone-ldquo-tools-rdquo-that-bear-a-striking-resemblance-to-early-human-artifacts/?redirect=1)).
  This supports `MAT-04`'s example: cracking nuts with a flint cobble fits the flake blueprint, and someone must notice (`MND-10`).
- **Teaching beats watching:** Morgan and others (Nature Communications 2015) taught Oldowan knapping to 184 people along chains, five ways ([PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC4338549), [St Andrews](https://news.st-andrews.ac.uk/archive/school-of-rock/)):
  1. seeing only the flakes;
  2. watching;
  3. guided hands;
  4. gestures;
  5. talk.
  - With talk, learners made 25.22 viable flakes against 15.76 from seeing flakes alone, about 60% more.
  - Skill still decayed along the chain.
  - It supports `MND-13`'s order: taught learners above watchers.
    Our "four times a lone try" is a starting value to tune against such numbers.
- **How long mastery takes:**
  - Khambhat's stone-bead knappers in India: "The duration of their apprenticeship period is ten years" for high-quality beads, and three years for low-quality ones.
    Expertise showed in the hands, "the values of acceleration of the hammer", more than in planning (Roux, Bril and Dietrich 1995) ([OpenEdition](https://books.openedition.org/editionsmsh/8727)).
  - The stone-adze masters of Langda in Papua "had passed through apprenticeships of 5 to 10 years" (Stout) ([Science News](https://sciencenews.org/?p=28423)).
  - Modern trainees learning handaxes need "social support, motivation, persistence, and self-control" (Pargeter, Khreisheh and Stout 2019) ([Emory](https://emoryanthropology.org/2019/07/15/dr-justin-pargeter-and-colleagues-present-a-multidisciplinary-study-of-late-acheulean-handaxe-making-skill-acquisition-in-the-latest-edition-of-the-journal-of-human-evolution/)).

  These fit `MND-06`: level 5 in about 2 years, 10 in about 10.
- **Skill curves:** the power law of practice (Snoddy 1928, Crossman's Cuban cigar roller 1959, Newell and Rosenbloom 1981) says improvement per try shrinks as practice grows ([Wikipedia](https://en.wikipedia.org/wiki/Power_law_of_practice)).
  - `MND-06` says the same: "less at high levels".
  - RimWorld makes skills above 10 drain faster the higher they are: 30 experience a day at 10, 60 at 11, 120 at 12 and 180 at 13.
    Passion multiplies learning from 35% to 150% ([RimWorld wiki: skills](https://rimworldwiki.com/wiki/Skills)).

### Fire, glue and heat

- **Fire by friction:** one long inquiry tried 370 hand-drill combinations: spindles of 52 native and 22 non-native species, on four hearthboards ([Primitive Ways](https://www.primitiveways.com/hand_drill.html)).
  - The average effort was "longer than around four minutes", often over several sockets.
  - Softer spindles worked better: the best averaged density 0.42, against 0.53 for all.
  - Without a notch, embers came only 28% of the time.
  - Methods worldwide are the hand drill, bow drill, fire plough, fire saw and pump drill, plus striking pyrite ([Wikipedia: fire making](https://en.wikipedia.org/wiki/Fire_making)).

  This fits `RCK-02`'s "within about 5 minutes a try" and `MAT-07`'s two routes.
- **When fire came:**
  - fire kept: burnt bone and plant ash "1 million-year-old", deep in Wonderwerk Cave, the bones "heated to about 500° Celsius" ([Science News](https://www.sciencenews.org/article/ashes-oldest-controlled-fire));
  - fire made at will: wear from "striking a piece of flint against a piece of pyrite" on Neanderthal handaxes "of about 50,000 years in age" (Sorensen and others 2018) ([Leiden](https://universiteitleiden.nl/en/news/2018/07/neanderthals-could-make-fire));
  - possibly 400,000 years ago in England ([Wikipedia: fire making](https://en.wikipedia.org/wiki/Fire_making)).
- **Birch tar:**
  - Tar forms from birch bark at "as low as 250–300 °C" up to over 500 °C.
  - Three Palaeolithic methods were tried (Kozowyk and others 2017) ([PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC5579016/)):
    - an ash mound gave up to about 1 g per 100 g of bark;
    - a pit roll up to 2.4 g;
    - a raised structure up to 9.6 g.
  - Tight control at 340–370 °C "is thus not as necessary as previously thought".
  - **The finding that matters:** Schmidt and others (PNAS 2019) burned birch bark near "flat river stones" in the open, and "scraped off the surface of the stones" a tar that "formed a stronger glue than tar produced in more complex oxygen-free processes".
    "This method of making birch tar is so simple that early humans could have easily discovered it by accident in the course of their everyday activities" ([NYU](https://engineering.nyu.edu/news/one-argument-neanderthal-genius-debunked), [Wikipedia: birch bark tar](https://en.wikipedia.org/wiki/Birch_bark_tar)).
  - Neanderthals made tar "as early as 200,000 years ago".
  - `RCK-12` says birch bark "in an open fire … only burns"; this needs your decision (below).
- **Heat-treated stone:** at Pinnacle Point, South Africa, people "fired their silcrete stone at temperatures between 300 and 400 degrees" ([UNSW](https://www.unsw.edu.au/newsroom/news/2009/08/fiery-imaginings-72-000-years-ago)).
  They did so "at 72,000 years ago, and perhaps as early as 164,000 years ago", because "heating stone makes it easier to flake" (Brown and others, Science 2009) ([ASU](https://news.asu.edu/content/early-modern-humans-use-fire-engineer-tools), [PubMed](https://pubmed.ncbi.nlm.nih.gov/19679810/)).
  This supports `RCK-10`.
- **Ochre:**
  - Heating turns yellow goethite into red hematite, at roughly 250 to 350 °C (Pomiès, Menu and Vignaud 1999) ([Cambridge](https://www.cambridge.org/core/services/aop-cambridge-core/content/view/S0026461X00021770)).
  - In the record, the change is "extremely rare", yet some Mousterian people "chose to gather remote yellow lumps for heating" ([ORBi](https://orbi.uliege.be/handle/2268/100103)).

  This supports `RCK-15`.
- **Pottery:** "Temperatures recorded in non-kiln bonfirings range between 600°C and 900°C" ([EXARC: bonfirings](https://exarc.net/ark:/88735/10427)).
  The oldest pots, from Xianren Cave in China "between 20,000 and 19,000 years before present", were made by "mobile foragers who hunted and gathered their food", long before farming ([Wikipedia: Xianren Cave](https://en.wikipedia.org/wiki/Xianren_Cave)).
  This supports `RCK-04`, and pottery before farming in `TIM-19`.
- **Copper** (Davey and Hayes 2023) ([Buried History](https://www.bhjournal.au/ojs/index.php/bhjournal/article/download/6/215/262)):
  - Copper melts at 1,085 °C.
  - Egyptian tomb scenes show "six operators pointing their blowpipes at the front openings of two crucibles", and "three operators exhaling in succession would have generated a fairly constant air stream".
  - In a replica, charcoal brought a crucible "to 900°C after about eight minutes", and a mechanical air jet reached 1,140 °C.
  - The oldest secure smelting is at Belovode, Serbia, "from 7,000 years ago": "five pieces of copper slag" and "a drop of once-molten metal" of pure copper (Radivojević and others 2010) ([Science News](https://sciencenews.org/?p=6225)).

  All of this supports `RCK-08` and `MAT-18`'s blowers taking turns at two or more pipes.

### Hides, cord and needles

- **Tanning:**
  - Bark tanning was slow: historically it "could take between 12 and 18 months", one month with modern extracts ([Quai Branly](https://croyan.quaibranly.fr/en/the-workshop-tanning-hides)).
  - Animal brain "will equally well replace plant tannins" ([KEAP](https://keap.umk.pl/en/experiments-with-tanning/)).
    One guide's brain-tanning steps take hours each: braining "two to eight hours", stretching while it dries "three to nine hours", then smoking ([Outdoor Life](https://www.outdoorlife.com/blogs/survivalist/2011/11/survival-skills-brain-tanning-hides/)).
  - `RCK-06` allows only bark tanning; this needs your decision (below).
- **Cord:** a Neanderthal cord from Abri du Maras, France, more than 40,000 years old, is "three bundles of twisted fibres, plied together to create one cord", "probably from coniferous trees" (Hardy and others 2020) ([CNRS](https://www.cnrs.fr/en/press/neanderthal-cord-weaver)).
  This supports `RCK-11`.
- **Needles:** a bird-bone needle with an eye, about 50,000 years old, from Denisova Cave ([Wikipedia: sewing needle](https://en.wikipedia.org/wiki/Sewing_needle)).

### The real order

| Step | First known | Kindling's window (`TIM-19`) |
|---|---|---|
| Sharp flakes | 3.3 million years ago, Lomekwi, Kenya ([Wikipedia](https://en.wikipedia.org/wiki/Lomekwi)) | Years 1–5 |
| Fire kept | about 1 million years ago | — |
| Fire made at will | about 50,000 years ago, maybe 400,000 | 5–30 |
| Cord, sewing | about 40,000–50,000 years ago | clothing 10–40 |
| Pottery | about 20,000 years ago, by foragers | 60–150 |
| Smelted copper | about 7,000 years ago | 300–500 |

The game squeezes three million years into five hundred, but keeps the real order and dependencies.

## Crafts gained and lost

- **The Tasmanian case** (Henrich 2004) ([PDF](https://gwern.net/doc/sociology/2004-henrich.pdf), [Cambridge](https://cambridge.org/core/journals/american-antiquity/article/abs/demography-and-cultural-evolution-how-adaptive-cultural-processes-can-produce-maladaptive-lossesthe-tasmanian-case/8CD08CC61E6FACF59EC02659B2BDA43C)):
  - After rising seas cut Tasmania off, its people lost over some 8,000 years "bone tools, cold-weather clothing, hafted tools, nets, fishing spears, barbed spears, spear-throwers, and boomerangs".
  - Henrich's model: each learner copies the most skilled person of the generation before, but copies imperfectly.
    A systematic loss α pulls skill down, and the spread β of lucky errors lets a few surpass their model.
  - The average change is Δz = −α + β(0.577 + ln N), so skill grows only when the number of learners N exceeds e^(α/β − 0.577).
    Harder crafts need more learners.
    For Tasmania's roughly 4,000 people he estimates N at about 800.
- **The Polar Inuit** (Inughuit) of north-west Greenland had lost "the qajaq and the larger so-called woman's boat, the umiaq, as well as the bird spear, fish leister, and bow and arrow".
  Migrants from Baffin Island, led by the shaman Qillarsuaq, set out "probably" in 1859 and brought them back in the 1860s ([Nunatsiaq News](https://nunatsiaq.com/stories/article/taissumanni_july_10/), [Wikipedia: Inughuit](https://en.wikipedia.org/wiki/Inughuit)).
  It is `MOM-02`, `CUL-02` and `CUL-16` in one true story.
- **Size and contact:**
  - Powell, Shennan and Thomas (Science 2009) argue that density and migration, not brains, explain where modern behaviour first appears ([UCL](https://discovery.ucl.ac.uk/id/eprint/168654/)).
  - Across 10 Oceanic societies, bigger populations had more and more complex fishing tools, and contact mattered more to small ones (Kline and Boyd 2010) ([HRAF](https://hraf.yale.edu/ehc/documents/1011)).
  - In a lab game, "in larger groups of players, higher cultural complexity and cultural trait diversity are maintained, and improvements to existing cultural traits are more frequent" (Derex and others, Nature 2013) ([IDEAS](https://ideas.repec.org/a/nat/nature/v503y2013i7476d10.1038_nature12774.html)).
- **The caution:** Vaesen and others (PNAS 2016) find the models "support a relationship between population size and cultural complexity only for a restricted set of extremely implausible conditions" ([Leiden](https://universiteitleiden.nl/en/news/2016/04/population-size-fails-to-explain-the-evolution-of-complex-culture), [PDF](https://abdn.elsevierpure.com/files/100834691/PNAS_2016_Vaesen_E2241_7.pdf)).
  So Kindling must not script the effect.
  Its rules (copying the skilled, crafts dying with their last holder, spread only where people meet) should let it emerge, and the pace tests measure whether it does.

## Can Godot do it?

- **Nothing here touches the engine:**
  - things, blueprints, timers, skills and discovery are simulation in the C++ library (research 03);
  - catalogues are data files, checked by our tools (`MAT-17`).
- **Godot only shows the results:** models, icons and animations (research 17), and sounds (research 15).
- **The risks are design and volume, not the engine:**
  - the pace of discovery (`TIM-19`);
  - absurd fits (`RSK-06`);
  - the work of writing and checking 190 items and 140 blueprints (`RSK-25`).

## What we take

1. **Materials carry their values,** Dwarf Fortress- and RimWorld-style: an item's 18 characteristics come from its material and form, and made things inherit from their inputs (`MAT-03`).
2. **Blueprints match characteristics and classes, never names,** as Dwarf Fortress's reactions match material classes (`MAT-04`, `PRN-07`).
   Breath of the Wild's lesson: a few rules that follow the real world give many events, and players read them by instinct.
   Its third rule shows the combinations must be fenced; the expected-fits list does that (`MAT-17`).
3. **Every reality rule has a real experiment behind it,** cited in its catalogue check:
   - flakes from flaking stone only (capuchins, Lomekwi): `RCK-01`;
   - friction fire within minutes with soft dry wood: `RCK-02`;
   - heat-treated stone at 300–400 °C: `RCK-10`;
   - ochre turning red at about 250–350 °C: `RCK-15`;
   - pots fired in the open at 600–900 °C: `RCK-04`;
   - copper above 1,085 °C with blowers in turns: `RCK-08`;
   - cord twisted and plied: `RCK-11`.
4. **Quality from skill and inputs,** as Star Wars Galaxies and Wurm do: fine inputs and skilled hands make things that work better and last longer (`MAT-20`).
5. **Discovery belongs to people, never to the player:**
   - accidents happen as the capuchins' flakes did;
   - hints are personal eurekas, as in Civilization VI;
   - copying a found thing works as Qud's reverse engineering does (`MND-11`).
6. **Skill grows by the power law,** checked against real apprenticeships: a few years to competence, five to ten to mastery (Khambhat, Langda) (`MND-06`).
   Teaching beats watching, as Morgan's chains show (`MND-13`).
7. **Loss and return emerge, never scripted:**
   - better teachers are copied;
   - crafts die with their last holder;
   - they come back only by rediscovery, neighbours or copying old things (`CUL-02`, `CUL-16`).

   The Polar Inuit's lost and returned kayaks are the model for the `MOM-02` scene.
   Whether small, cut-off bands lose more is measured in the pace tests, not assumed, given Vaesen's caution.
8. **A discovery-pace prototype before production,** the riskiest design question here:
   - one band, simple minds, and only the stone and fire blueprints, run headless for many simulated years;
   - tune the discovery factors until the sharp-stone test (`RES-02`, `RES-03`) and the fire window pass;
   - report how sensitive the pace is to each factor.

   Ancestors and Dawn of Man show the two failures to avoid; the prototype shows whether tuning alone can meet `TIM-19`.

## Decided with you

Two decided rules disagreed with the experiments; on 4 October 2026 you agreed to give each a second route (`PRC-07`):

- **`RCK-12` Glue from bark:** birch bark burning in the open beside a smooth stone leaves tar on the stone (Schmidt 2019).
  It is a likely accident, and fits `MAT-07`'s several routes.
- **`RCK-06` Leather:** a hide worked with animal brain, stretched as it dries, then smoked, becomes soft leather, with no bark needed.

## Sources

- Games: things and crafting:
  - [DF wiki: material science](https://dwarffortresswiki.org/index.php/v0.34:Material_science)
  - [DF wiki: reaction](https://dwarffortresswiki.org/index.php/Reaction)
  - [RimWorld wiki: stuff](https://rimworldwiki.com/wiki/Stuff)
  - [Thumbsticks: Breath of the Wild at GDC](https://www.thumbsticks.com/gdc-17-breath-of-the-wild-science-lies)
  - [Tom's Guide](https://www.tomsguide.com/uk/us/zelda-breath-wild-design,news-24586.html)
  - [SWG Restoration: crafting](https://swgr.org/wiki/crafting)
  - [SWG wiki: crafting](https://swg.fandom.com/wiki/Crafting)
  - [Wurmpedia: quality level](https://wurmpedia.com/index.php/Quality_Level)
  - [Grow et al.: Crafting in Games](https://dhq.digitalhumanities.org/vol/11/4/000339/000339.html)
- Games: discovery:
  - [Den of Geek: Ancestors](https://www.denofgeek.com/?p=8675)
  - [GameRevolution: Ancestors](https://www.gamerevolution.com/review/583921-ancestors-the-humankind-odyssey-pc-ps4-xbox-one-brutal-survival-game)
  - [Steam: Dawn of Man](https://store.steampowered.com/app/858810/Dawn_of_Man/)
  - [Steam discussion: Dawn of Man](https://steamcommunity.com/app/858810/discussions/0/1648792158829175445)
  - [Steam guide: Civilization VI eurekas](https://steamcommunity.com/sharedfiles/filedetails/comments/886630163)
  - [CraftDex: Minecraft recipe book](https://craftdex.net/articles/the-recipe-book-and-unlocks)
  - [Nodecraft: Valheim recipes](https://nodecraft.com/support/games/valheim/setup/unlock-all-recipes-and-buildings-on-a-valheim-server)
  - [Qud wiki: Reverse Engineer](https://wiki.cavesofqud.com/wiki/Reverse_Engineer)
  - [Noita wiki: Alchemic Precursor](https://noita.wiki.gg/wiki/Alchemic_Precursor)
  - [Noita wiki: Lively Concoction](https://noita.wiki.gg/wiki/Lively_Concoction)
  - [Steam: Sapiens](https://store.steampowered.com/app/1060230/Sapiens/)
  - [Vintage Story wiki: knapping](https://wiki.vintagestory.at/Special:MyLanguage/knapping)
  - [Vintage Story wiki: clay forming](https://wiki.vintagestory.at/Clay_Forming)
- Learning and skill:
  - [Proffitt et al. 2016, discussed](https://pmc.ncbi.nlm.nih.gov/articles/PMC5429872)
  - [Scientific American: capuchin flakes](https://www.scientificamerican.com/article/monkeys-make-stone-ldquo-tools-rdquo-that-bear-a-striking-resemblance-to-early-human-artifacts/?redirect=1)
  - [Morgan et al. 2015](https://pmc.ncbi.nlm.nih.gov/articles/PMC4338549)
  - [St Andrews: School of Rock](https://news.st-andrews.ac.uk/archive/school-of-rock/)
  - [OpenEdition: Khambhat knapping skills](https://books.openedition.org/editionsmsh/8727)
  - [Science News: Langda adze makers](https://sciencenews.org/?p=28423)
  - [Emory: Pargeter et al. 2019](https://emoryanthropology.org/2019/07/15/dr-justin-pargeter-and-colleagues-present-a-multidisciplinary-study-of-late-acheulean-handaxe-making-skill-acquisition-in-the-latest-edition-of-the-journal-of-human-evolution/)
  - [Wikipedia: power law of practice](https://en.wikipedia.org/wiki/Power_law_of_practice)
  - [RimWorld wiki: skills](https://rimworldwiki.com/wiki/Skills)
- Fire, glue and heat:
  - [Primitive Ways: hand drill inquiry](https://www.primitiveways.com/hand_drill.html)
  - [Wikipedia: fire making](https://en.wikipedia.org/wiki/Fire_making)
  - [Science News: Wonderwerk](https://www.sciencenews.org/article/ashes-oldest-controlled-fire)
  - [Leiden: Neanderthal fire](https://universiteitleiden.nl/en/news/2018/07/neanderthals-could-make-fire)
  - [Kozowyk et al. 2017](https://pmc.ncbi.nlm.nih.gov/articles/PMC5579016/)
  - [NYU: birch tar](https://engineering.nyu.edu/news/one-argument-neanderthal-genius-debunked)
  - [Wikipedia: birch bark tar](https://en.wikipedia.org/wiki/Birch_bark_tar)
  - [UNSW: Pinnacle Point](https://www.unsw.edu.au/newsroom/news/2009/08/fiery-imaginings-72-000-years-ago)
  - [ASU: heat treatment](https://news.asu.edu/content/early-modern-humans-use-fire-engineer-tools)
  - [Brown et al. 2009](https://pubmed.ncbi.nlm.nih.gov/19679810/)
  - [Cambridge: Pomiès et al. 1999](https://www.cambridge.org/core/services/aop-cambridge-core/content/view/S0026461X00021770)
  - [ORBi: heated ochre](https://orbi.uliege.be/handle/2268/100103)
  - [EXARC: bonfirings](https://exarc.net/ark:/88735/10427)
  - [Wikipedia: Xianren Cave](https://en.wikipedia.org/wiki/Xianren_Cave)
  - [Davey and Hayes 2023](https://www.bhjournal.au/ojs/index.php/bhjournal/article/download/6/215/262)
  - [Science News: Belovode](https://sciencenews.org/?p=6225)
- Hides, cord and needles:
  - [Quai Branly: tanning](https://croyan.quaibranly.fr/en/the-workshop-tanning-hides)
  - [KEAP: tanning experiments](https://keap.umk.pl/en/experiments-with-tanning/)
  - [Outdoor Life: brain tanning](https://www.outdoorlife.com/blogs/survivalist/2011/11/survival-skills-brain-tanning-hides/)
  - [CNRS: Neanderthal cord](https://www.cnrs.fr/en/press/neanderthal-cord-weaver)
  - [Wikipedia: sewing needle](https://en.wikipedia.org/wiki/Sewing_needle)
  - [Wikipedia: Lomekwi](https://en.wikipedia.org/wiki/Lomekwi)
- Gained and lost:
  - [Henrich 2004 (PDF)](https://gwern.net/doc/sociology/2004-henrich.pdf)
  - [Cambridge: Henrich 2004](https://cambridge.org/core/journals/american-antiquity/article/abs/demography-and-cultural-evolution-how-adaptive-cultural-processes-can-produce-maladaptive-lossesthe-tasmanian-case/8CD08CC61E6FACF59EC02659B2BDA43C)
  - [Nunatsiaq News: Qillarsuaq](https://nunatsiaq.com/stories/article/taissumanni_july_10/)
  - [Wikipedia: Inughuit](https://en.wikipedia.org/wiki/Inughuit)
  - [UCL: Powell et al. 2009](https://discovery.ucl.ac.uk/id/eprint/168654/)
  - [HRAF: Kline and Boyd 2010](https://hraf.yale.edu/ehc/documents/1011)
  - [IDEAS: Derex et al. 2013](https://ideas.repec.org/a/nat/nature/v503y2013i7476d10.1038_nature12774.html)
  - [Leiden: population size](https://universiteitleiden.nl/en/news/2016/04/population-size-fails-to-explain-the-evolution-of-complex-culture)
  - [Vaesen et al. 2016](https://abdn.elsevierpure.com/files/100834691/PNAS_2016_Vaesen_E2241_7.pdf)
