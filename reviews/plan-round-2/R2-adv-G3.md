# R2 adversary G3: section 6, World

Read: ROUND.md, BRIEF.md, R1-coord.md, R1-impl-G3.md, R1-adv-G3.md, and all of `assembled.md` (01:14): section 6 in full, and every item that cites a `WLD` ID.

**Round 1 check.** Decisions 3, 8, 9, 10, 11, 18, 19, 30, 31 and 32 landed in section 6 as the memo wrote them.
The items that lean on them agree in wording and numbers: `PRN-05`, `TIM-17`, `TIM-18`, `TIM-19`, `BIO-05`, `BIO-10`, `BIO-19`, `MAT-11`, `RCK-23`, `MND-16`, `PRE-03`, `PLT-04`, `PLT-09`, the header lists and the glossary (Area, Seed, Kept area, Biome, Region, Herd count).
Each rule lives once: the land's numbers in `WLD-30`, fouled water in `WLD-17`, catches in `MAT-11`, zoom stops in `PRE-03`.
What follows is where the landed text still contradicts itself or leaves builders guessing.
Findings 1 and 2 break decisions 3 and 9 as written; 3 to 8 are gaps in the area model and the land's numbers.

### 1. [blocker] WLD-30, WLD-09, WLD-18, WLD-31 The land's numbers contradict themselves: food alone takes animals, roots and wood to six times WLD-30's
Problem: `WLD-30` puts wild animals at about a sixth of Earth's numbers per km², breeding once a game year, so that meat per game day matches a real day.
But animals eat a real day's food each game day (`TIM-18`, `BIO-19`) from plants that also give per game day what they give per real day, so food would feed Earth's numbers.
`WLD-09` step 8 places them "in the numbers that food feeds", and `WLD-18` limits them only by what "each cell feeds", with well-fed animals breeding well; herds growing a fifth to a third a game year get there within the 10 game years of settling (`WLD-08`).
A builder following `WLD-09` and `WLD-18` gets six times `WLD-30`'s grazers: six times the meat per day, undoing `WLD-04` and `WLD-30`'s own 100–300 km² Check, and six times the animals near people, likely past `PLT-04`'s quarter.
Hunters then grow on that yield to about six times Earth's ratio to their prey, so `WLD-18`'s "1 to 50–200 prey" can't hold.
Plants have the same gap: in `WLD-31` a plant someone "picks, digs, cuts" becomes a thing at its real size, while stands grow back in game years (an oak wood in about 10), so roots, reeds, bark and wood taken whole give about six times the daily yield `WLD-30` promises.
Fix:
- `WLD-30` The land's numbers, add: "Animals eat a real day's food each game day, so food alone would carry them to Earth's numbers: `WLD-18` holds each kind to its sixth.
  What people dig, cut or strip from a plant (roots, reeds, bark, fibre, and the dead wood a stand drops) is its yield, like fruit; only felling a tree or digging out a plant to move it takes it whole."
- `WLD-09` step 8: "in the numbers that food feeds" becomes "at the numbers `WLD-18` holds them to".
- `WLD-18` Limits begins: "each cell holds at most a set number of each kind for its cover, a sixth of what such land holds on Earth (`WLD-30`); below it, well-fed animals breed well, and hungry ones less and die first in a hard winter; a hunter kills ..." (the rest as now).
- `WLD-31`: "Gathering takes the ripe yield from the patch or plant (`MAT-09`), which becomes things (`MAT-10`) of a size and quality drawn from the seed, the same whoever takes it; the plant stays, stripped, unless felled or dug out (`WLD-30`)."
Cross-section: `MAT-06` dig, `MAT-09` (G4: digging roots takes the yield; fine as worded), `MND-15`, `PLT-04` (G6, G9: the animal cost assumes the sixth).

### 2. [blocker] WLD-28, WLD-12, WLD-13, MAT-18 The wildfire rule reads whether an area is made, so watching a fire changes what burns
Problem: Unchanged areas "run no rules of their own; nothing in the world reads whether one is made" (`WLD-12`, `WLD-13`).
But `WLD-28` runs the front "in made areas" patch by patch, where "a stream, bare ground" stops it, "and the cell counts only what burned".
Areas are made for the picture within about 1 km of the camera, so a fire you watch can stop at a stream inside a viewed area and leave more of its cell unburned than the same fire unwatched.
That breaks `WLD-13`'s Check and `WLD-12`'s own Done when; `MAT-18` repeats the rule ("in an area a wildfire moves as a front").
Fix: `WLD-28`: "**In made areas** the front enters" becomes "**Where people are,** in the areas they are in, the front enters"; then add: "Elsewhere areas follow their cell, and kept ones take the fire when brought up to date (`WLD-12`)."
Cross-section: `MAT-18` Spreading (G4): "in an area people are in, a wildfire moves as a front ...".

### 3. [major] WLD-12, WLD-13, TIM-07, PRN-10 Bringing a kept area up to date is undefined, depends on when it happens, and grows with history
Problem: "A kept area with nobody within about 1 km is brought up to date when someone comes near, and at least once a season" doesn't say what is applied: timers on what weather, whether the fires, floods, lava and quakes its cell went through meanwhile burn, carry off or bury its things, or whether chances are drawn per day or per update.
So two builders make different camps, and one camp comes out differently with when someone happened to pass.
The camera is not "someone" (`WLD-13`): within 1 km of it the picture either shows a left camp's meat fresh for up to a season (against `PRN-10`), or updates it (against `WLD-13`, unless updating comes out the same whenever it is done).
Updating every kept area each season grows with history, since stone tools and fired clay keep old camps for good (`MAT-08`), against `TIM-07`'s 500-year-old world at four fifths of a new one's speed.
And the cell, updated every 5 game days, "counts the plants people changed as they are" from kept areas up to a season stale.
Fix: In `WLD-12` What is kept, replace the "brought up to date" sentence with:
"A kept area runs only while someone is within about 1 km.
When someone comes near, or the picture shows it, it is brought up to date day by day: what its cell went through (fire, flood, lava, a quake) acts on it, and its timers and marks run on its cell's usual weather for each season (`WLD-16`), each chance drawn for its day (`TIM-16`), so it comes out the same however late or often this is done, and the picture saves nothing (`WLD-13`)."
On the cell becomes: "growth and seasons, wild grazing, wildfire, floods and snow act on the world cell; a grove cut, a field cleared or an animal killed comes off its cover or herd when it happens, and the cell's own regrowth gives it back."
Done when, add: "a kept camp brought up to date daily, or once after a season, ends the same".
Cross-section: `TIM-17` (G2: fine), `PLT-04` (G9: kept areas cost nothing while nobody is near), `PRE-03` (G8: fine).

### 4. [major] WLD-34, RES-07, RES-21 The first region has no edge, no room upwind and no animals named, and fills up within the runs it stands in for
Problem: Before `MIL-04` the 40 by 40 km first region stands in for whole worlds, including the nightly runs of 20 worlds to Year 60 (`RES-07`, `RES-21`).
Nothing says what lies past its edge, yet explorers (`MND-09`, `MND-32`), herds that migrate "tens of kilometres" (`WLD-30`), wildfires, and storms about 50 km across that are "born upwind" (`WLD-16`) all reach it.
Its 1,600 km² barely holds the start: `WLD-24` wants food within about 10 km of each of 3–4 shelters (about 300 km² each), and `WLD-30`'s Check gives 100–300 km² per 25 people, so 45–120 people need up to about 1,400 km², and at 0.8% a year (`BIO-04`) about 2,300 km² by Year 60.
It also doesn't say whether its cells hold herds and small game before `MIL-04`, though `BIO-02` lives partly on scavenging and small game and `RES-02` needs carcasses, nor whether it settles (`WLD-08`).
Fix: `WLD-34` becomes:
"- `WLD-34` **The first region** *(Decided)*: Until whole worlds are generated (`MIL-04`), play and test scenes run on a first region: an island about 60 by 60 km (tuned) in a sea about 50 km wide on every side, which nobody can cross, its world-cell values, herds and small game included, and its seasonal climate set by hand from a few numbers, meeting `WLD-24`.
  - **How it works:** it settles as a world does (`WLD-08`), areas are made from it by the same rules as later (`WLD-12`), and from `MIL-03` its weather runs by `WLD-16`, storms forming over its sea; at `MIL-04` the generator fills the same values.
  - **Done when:** a region set by hand and the same values from a generated world make identical areas, and in 20 runs to Year 60 its bands still find food within their home ranges (`WLD-24`)."
No item in the plan names a boat, and swimming tires and can drown (`BIO-21`), so 50 km of sea is an edge the rules already keep, not a scripted wall.
Cross-section: `RES-07`, `RES-21` (G9: fine), `MAT-16` (G4: the first region's herds and small game from `MIL-01`), glossary "Region" (G9: fine).

### 5. [major] WLD-32, WLD-12, WLD-33 Animals near people stand in areas nobody makes, and a tamed animal stays an individual for life
Problem: `WLD-32` turns a herd within about 1 km of a person into individuals "in their areas", but areas are made only where people stop or something happens to them (`WLD-12`), and layer 3 lets creatures stand outside areas only "while walking between them".
So a builder must either make areas wherever such a herd wanders for its day, a dozen or more for each herd met, at about 5 ms each (`PLT-04` gives all area making a tenth of the time), or put grazing deer in no area.
A hunter stalking a deer 300 m away stops in his own area, while the deer's need not be made.
Tameness never falls apart from people (`WLD-33`), so a wolf fed scraps once (tameness 1, `MOM-06`) stays an individual, with body and mind, all its life wherever it roams.
Fix:
- `WLD-12` layer 3: "while walking between them" becomes "where no area is made"; Where areas are made: "where people stop to do something, or where what they act on stands, such as a stalked deer, or something happens to them".
- `WLD-32` Near people: "become individuals in their areas" becomes "become individuals where they stand (`WLD-12`)".
- `WLD-33` Tameness: "and harm lowers it" becomes "and harm, or a season apart from people, lowers it a step".
Cross-section: `MND-16` (G6: fine), `MOM-06` (G1: the litter stays near camp; fine).

### 6. [major] WLD-12, WLD-11, PRN-11 Paces and candidates read as changed while playing, and some layers have no pace
Problem: "If a layer runs over its share (`PLT-04`), its pace grows coarser everywhere alike" and "if it runs long, fewer candidates are made" (`WLD-11`) read as rules applied while running.
Then a busy screen, a hot phone or the cloud's faster cores change the world's paces and the worlds offered: against `PRN-11` ("cutting detail under load would make history depend on how busy the phone is"), `RES-05` (phone and cloud identical), `WLD-13`, and `WLD-10` ("a seed always offers the same three").
The paces also leave out small animals' counts, sea cells, soils and ice, so builders pick anything from hourly to yearly.
Fix: `WLD-12` Paces becomes:
"**Paces,** on one world clock, the same in every world and run: weather hourly; a wildfire hourly while it burns; water, snow and ice, fuel dryness and herds daily, water hourly in a flood; plant cover, small animals' counts, sea cells and soils every 5 game days; people, animals and things by their activities (`TIM-17`).
If a layer runs over its share in `PLT-04`'s measurements, a later version makes its pace coarser everywhere alike, never less detail where people are."
`WLD-11`: cut its How it works line (Cuts, C7); `WLD-10` already tunes the number of candidates.
Cross-section: `PLT-04` (G9: fine).

### 7. [major] WLD-12, MAT-08 Paths are kept-area marks, but walks don't make areas
Problem: `WLD-12` keeps paths as marks in areas, yet walks "cross the cells' ground, whose slope, cover, fords and paths set their time and route", and only stopping makes an area.
A path between a camp and a flint outcrop 5 km off (`MAT-08`: "a route walked often, counted once per walk") is walked, not stopped on, so it forms in no area, unless every walk makes and keeps the 20 or so areas it crosses, the cost decision 9 removed.
Builders can't tell where a path lives or where a walk is counted.
Fix: `WLD-12` What is kept: "paths" becomes "trodden camp ground"; On the cell, add: "Paths are worn on the cells a walk crosses, counted once per walk, and fade after a few unused years (`MAT-08`); areas draw them from their cell."
Cross-section: `MAT-08` Paths (G4: cite `WLD-12`), `MND-28` (G6: fine).

### 8. [major] WLD-31, WLD-12 A cell's cover names one leading species, so areas can't be made with what grows there
Problem: Unchanged areas take their plants from the cell's cover (`WLD-12`), which holds only "the leading species" and "the food ripe now".
An oak wood with hazel, bramble and wild garlic, or grassland with wild grain, herbs and roots, holds several food plants.
From one species a cell, each builder invents the rest, so food variety (`BIO-10`), "hazelnuts here in autumn" (`MND-28`) and what herds eat differ between builds, and "the food ripe now" can't be worked out at all.
Fix: `WLD-31` Cover per cell: "the leading species, the trees' age and the food ripe now" becomes "each with up to about four species and their shares, drawn at generation from those the cell suits; the trees' age; and what is ripe, from the species and the date".
Done when, first clause: "an area made from a cell holds its species in about their shares".
Cross-section: `PRE-07` (G8: the plants overlay reads the shares).

### 9. [major] WLD-08, PLT-09 Every food tuning counts as a big update, though only shapes and places make seams
Problem: `WLD-08` repeats a seed only under "the same plant, animal and rock values", and `PLT-09` makes changing them a big update that may need a new world.
But the pace is tuned by amounts, "how much food the land gives" among them (`PRN-17`, `RES-16`), so every such tuning would put your worlds at risk.
Only values that shape and place things in areas make seams; a yield or a ripening date changes everywhere at once, like any rule, and animal values never touch areas (herds are saved, not remade).
Fix: `WLD-08` Repeatable: "a seed makes the same world and areas only under the same rules for making them and the same values that place and size land, plants and stones; changing these is a big update (`PLT-09`), while tuning yields, timings or chances is not."
Cross-section: `PLT-09` (G9: "the plant, animal and rock values they use" becomes "the values that place and size land, plants and stones"), `TIM-08` (G2: "generator version" becomes "the version of its making rules and values").

### 10. [major] WLD-17, BIO-05 Fouled water has no chance of making anyone ill, so WLD-17's Done when can't be judged
Problem: `WLD-17` defines fouled water and wants drinkers below a camp to "get gut sickness at about its rate", but no rate exists.
`BIO-05` gives chances per contact with the sick, and none per day of drinking fouled water.
Fix: `BIO-05` Gut sickness (G5): "fouled water (`WLD-17`): about 1 chance in 20 each day someone drinks it (tuned)".
`WLD-17` Done when: "drinkers below a camp get gut sickness at `BIO-05`'s chance for fouled water, those at the spring don't".
Cross-section: `BIO-05` (G5).

### 11. [major] RCK-14, MAT-19, TIM-18, WLD-30 How long dried meat keeps is a life length or a game length, six times apart
Problem: `RCK-14` and `MAT-19` keep dried or smoked meat "about a month in summer", checked as "at least 10 times as long as fresh" (fresh: about 3 days), so 30 game days, as every time in the file is a game time (`TIM-14`).
But `TIM-18` turns a month of life into about 5 game days, and nothing says whether "a month" is the life length or the game one.
Builders get 5 or 30 days against a 15-day winter, so storing for winter (`MND-22`), settling on stores (`CUL-28`) and the balance of stores against the year that `WLD-30` assumes come out six times apart.
Fix (G4): `MAT-19` Smoking and `RCK-14`: "smoked or dried meat keeps about 15 game days in summer, as about three months in life do (`TIM-18`), and through the winter in the cold".
`RCK-14` Check: "at least 5 times as long as fresh, and stored dry grain and nuts a game year".
Cross-section: `RCK-14`, `MAT-19` (G4).

### 12. [minor] WLD-23, WLD-31 Ice and seas can't hold 6 plants and 4 animals
Problem: `WLD-23` needs every biome of `WLD-31` to hold at least 6 plants and 4 animals of the catalogue, but the list includes ice, where nothing grows, and seas, while all 60 plants are land plants: the catalogue check can never pass.
Fix: "each land biome but ice has at least 6 plants and 4 animals of the catalogue, and seas and shores 4 animals, a species serving several" (merged into `WLD-31`, Cuts C4).
Cross-section: none.

### Cuts
Section 6 is 35.2 KB.
These cuts take about 3.3 KB of its current text; the fixes above add about 1.9 KB, so it ends about 1.4 KB lower (about 33.8 KB).
Each cut is a repeat of another item, an illustration, or a small mechanic no other item reads; none of the dropped items is cited outside section 6.

- C1 `WLD-29` becomes *Dropped*: "it only summed up what `WLD-16`, `WLD-17`, `WLD-18` and `WLD-28` say"; `WLD-16`'s good and bad years and `WLD-18`'s Done when test the same: 0.32 KB.
- C2 `WLD-05` becomes *Dropped*, "merged into `WLD-16`": the Seasons line takes "land and sea warming and cooling within it (`TIM-18`); latitude changes about a degree every 5.6 km, so climate belts are about 100 km wide", and the Done when ends "which is within about 2 °C of Earth's at the same latitude, height and distance from the sea": 0.20 KB.
- C3 `WLD-19` becomes *Dropped*, "merged into `WLD-31` and `WLD-32`": their first lines read "About 60 Earth species or close kin, with their real habits, seasons and sizes" and "About 30 wild Earth species or close kin"; both already cite `WLD-30`, and `MAT-17` checks the catalogue: 0.20 KB.
- C4 `WLD-23` becomes *Dropped*, "merged into `WLD-31`": Biomes takes finding 12's line, and the Done when adds "and in 20 test worlds each biome present has plant eaters, hunters, birds and fish": 0.17 KB.
- C5 `WLD-15`: "otherwise land doesn't wear down, rivers don't wander, slopes don't slide and coasts stay put" becomes "nothing else reshapes the land (`SCP-21`)"; the three eruption lines become one: "**Eruptions** give warning days ahead (small quakes, rumbling, warm springs) that people can notice (`BIO-18`); lava then burns and buries what lies in its path, leaving fresh lava rock, heights unchanged and no new obsidian, and ash smothers plants and fouls water nearby for a season, later enriching the soil (`WLD-27`)."
  This drops the great eruption that cools the whole world, a second climate mode for an event once in centuries: 0.29 KB.
- C6 `WLD-14`: cut the Example (`MOM-12` tells it); "Deposits show where the land is cut, such as banks, cliffs and cave walls, and look like what they are, while their uses are learned (`MAT-03`)"; cut salt from the title and the list, since salting is left out (`MAT-23`) and nothing at launch uses salt (G4 can drop it from `MAT-01`'s earth class): 0.29 KB.
- C7 `WLD-11`: cut the How it works line, which repeats `WLD-10`'s two passes (its runtime clause is finding 6): 0.15 KB.
- C8 `WLD-33`: cut Firsts (`TIM-19` counts each step at its first book-of-ages entry, and `PRE-39` records each people's firsts) and "(dog, goat, sheep, cattle and pig)": 0.22 KB.
- C9 Section intro: cut its second line, a pointer the contents already give: 0.08 KB.
- C10 `WLD-27`: cut "and what buried things keep: peat ..., acid soils eat it (`MAT-08`)", which `MAT-08`'s What survives says: 0.10 KB.
- C11 `WLD-22`: cut "storms fell trees and flatten shelters (`MAT-11`)", which `MAT-11`'s Toppling covers: 0.05 KB.
- C12 `WLD-07` in one sentence plus its Done when: "Sun, moon and stars move as they would for the world's tilt and the 60-day year (`TIM-18`): daylight by latitude and date, a moon that waxes and wanes once a season, bright enough at full to walk and hunt by, and an eclipse a few times in a lifetime at any place; people can learn these cycles (`CUL-13`)."
  Comets go, since nothing reads them: 0.20 KB.
- C13 Small mechanics nothing reads: `WLD-28`'s plants that need fire (title and After fire), `WLD-31`'s "earlier after a warm spell", `WLD-17`'s burst ice dams and lakes turning salty, and `WLD-26`'s storm floods on low shores: 0.19 KB.
- C14 `WLD-09`: cut "continents are pieced from older blocks, with worn-down ranges, basins and ragged coasts" (`WLD-08`'s coast rule already forces ragged coasts) and "wide valleys with lakes below the glaciers of high mountains and polar lands" (glaciers stay as generated, `WLD-17`): 0.17 KB.
- C15 `WLD-17`: "rain and melt soak in or run off (`WLD-27`), and ground water feeds springs that keep streams flowing in dry spells"; Rivers "each with its line, width and depth (`WLD-12`)": 0.13 KB.
- C16 `WLD-26`: "currents set at generation warm one side of each ocean and chill the other, and rich water wells up off some coasts"; cut "salt water can't be drunk (`BIO-09`)", which `BIO-09` says: 0.10 KB.
- C17 `WLD-24`: cut "each band gets one shelter as its home and the land around it as its home range (`BIO-03`)", which `BIO-03` says, and the Done when's winter-scene clause, which `BIO-11`'s Done when tests: 0.19 KB.
- C18 `WLD-08`: cut "the start region found before settling is kept (`WLD-24`)", which `WLD-24` says: 0.06 KB.
- C19 `WLD-04`: its second sentence becomes "It is far above what the phone runs (`MND-15`), so the land limits people only locally": 0.11 KB.
- C20 `WLD-13`: cut "and the same rules run at every speed and zoom (`TIM-17`)", which its What says; `WLD-32`: the yields list goes to the catalogue, "and yields (`MAT-10`), with eggs from nesting birds in season and dung from herd animals": 0.10 KB.

Total: about 3.3 KB.
