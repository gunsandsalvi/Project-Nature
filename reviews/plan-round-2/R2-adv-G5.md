# Round 2, adversary G5: People: bodies and lives (`s08-bodies.md`)

Read against `assembled.md` as rebuilt at 01:14 (section 8 identical to `s08-bodies.md`, 31.7 KB; `secheck.py` passes).
Numbers marked "simulated" come from small models of the stated rules, kept in the scratchpad folder `g5r2/`.

**Round 1 check.**
Every accepted G5 change is in, and so are the global decisions that touch section 8 (1–4, 11, 12, 19, 25, 31, 33); every ID section 8 cites is live.
What did not land: decision 4's target is not carried into `BIO-15`'s own numbers (finding 1), nor into the runs and budgets that use it (finding 5).
The "stated once" rule (C1) is still broken in a few places; the fixes are in Cuts: sight (`BIO-18` and `MND-03`), small game as counts (`BIO-19`, `MND-16`, `WLD-32`), cooking and quality (`BIO-10`), wet clothing (`BIO-11`), rot (`BIO-12`), what figures show (`BIO-09`, `BIO-13`, `BIO-16`), and how often crowd illnesses return (`CUL-28`).
Two small factual slips are also fixed in Cuts: `BIO-23` says rites "ease pain", though pain is the sum of wound sizes and only medicine lowers it (`BIO-13`, `BIO-12`); and `BIO-21` promises a wading animation that `PRE-44` doesn't list.

### 1. [blocker] BIO-15, BIO-16, BIO-03, BIO-04 The birth rules give about 9 children, not 5, and three times the growth
Problem: `BIO-15` lets a woman conceive from about 15 to 45, once she isn't nursing a baby under about 2, within about 30 days, after which she is pregnant for 45 days.
That is a birth about every 3.25 game years, which matches its Done when ("about every 3 years").
Simulated (with 1 pregnancy in 6 lost, 1 baby in 5 dying in its first year, and fertility falling from the late 30s), a woman who lives to 45 has about 9 children; `BIO-04` says about 5.
With `BIO-04`'s own death ranges, 9 children give growth of about 2.5–3% a year instead of 0.8%.
A world then reaches 2,000 people by about Year 95–150 instead of Year 350–475, and every speed target and test cost after that is out by centuries (`TIM-07`, `RES-07`, `RSK-02`).
The two Done whens can't both pass: births every 3 years from 15 to 45 can't come to 5 children, so a tuner must break one, and nothing says which.
Also, "1 birth in 100 kills the mother" is checked in whole worlds, yet "a helper halves these risks".
With a helper at most births, worlds measure about 1 in 200, right at the edge of what `RES-13` allows ("half to double").
Fix:
- `BIO-15` Conception: "a woman of about 18 to 38 with a male partner (`BIO-16`), in condition 30 or more (`BIO-09`) and not nursing, conceives within about 30 days on average, half as fast from about 33."
- `BIO-15` Nursing: "until weaning at about 3"; "Births come about every 4 years in bands, sooner where ..."; Done when "about every 4 years".
- `BIO-15` Birth: replace "A helper halves these risks, down to a quarter as the helper's healing experience nears 10 (`BIO-23`)" with "These risks are for a birth with a helper; alone they double, and a helper's healing experience halves them as it nears 10 (`BIO-23`)".
- `BIO-16` Fertility: "falls from about 33 and ends at about 38".
- `BIO-03`: "born about 4 years apart"; Done when "every mother was 18–38 at each child's birth".
Simulated, these give about 5.3 children and a birth every 4.0 years; time spent without a partner brings that to about 5.
Cross-section: none (`TIM-18`'s Example still holds).

### 2. [major] BIO-05 "Who dies" multiplies past certainty, and two of its factors almost always apply
Problem:
- The factors multiply with no ceiling.
  With babies and people over 60 at 5 times, chest fever (1 in 4) comes to 1.25, pox 1.5, wound fever 1.7, and lockjaw and wasting cough 2.5.
  So every baby or old person who catches one of these dies, and daily care (a half) still leaves lockjaw and wasting cough certain.
  Builders who clamp at 1, or who work on odds, get different death rates.
- "The shivering" means anyone below their comfort limit (`BIO-11`), which is 24 °C for a naked person at rest.
  That is most nights of the year before clothing, so the factor of 2 applies to nearly every illness in the first 10–40 years.
- Wound fever always comes with its own wound: for a wound of 20 or more, pain is 20 or more.
  So its listed "1 in 3" is really 2 in 3, and the Done when's "kills about its stated share" can't be measured for it.
Fix: replace the second and third lines of Who dies with:
"That chance is about 5 times higher for babies under 1 and people over 60, twice as high for each of hunger (condition under 30, `BIO-09`), real cold (10 °C or more below their limit, `BIO-11`) and wounds other than the one the illness came from (pain 20 or more, `BIO-13`), and a fifth higher or lower for each fifth of resistance below or above average (`BIO-08`); these multiply, to at most 9 in 10.
Then daily care halves it, down to a quarter as the carer's healing experience nears 10 (`BIO-23`, `MND-06`), and each point of medicine taken that day takes off a tenth (`BIO-12`); whoever survives recovers at the end of the course."
Cross-section: `BIO-12` (poison kills "like a short illness", so the same ceiling applies).

### 3. [major] BIO-13, BIO-05, MAT-04 Small cuts kill: a learner's knapping slips alone kill about 1 in 8 a year
Problem: `MAT-04`'s sharp flake cuts the hand on 1 failure in 100, at about half a minute a try.
A new knapper (skill 1, stone 0, so level 0.5 and 35% success) fails about 78 times an hour, which comes to about 0.8 cuts an hour.
Each slip is about 5 (`BIO-13`), so it gets infected 1 time in 40; 1 infected wound in 3 turns to wound fever, which kills 1 in 3 (`BIO-05`).
That is about 1 death in 360 slips.
A learner practising an hour on most days (`MND-06`) then has about a 12% chance a game year of dying from slips, and a skilled knapper (level 5) about 4%.
`TIM-18` wants all deaths before old age to add up to real foragers' rate, which is about 1–1.5% a year.
Every craft whose failures hurt adds to this.
Lockjaw "from a fall" breaks `BIO-13`'s own kinds: a fall gives bruises or broken bones, which never get infected, and closed wounds don't bring lockjaw.
Fix:
- `BIO-13` Infection: "in its first two days a cut of 10 or more gets infected with a chance of about half its size in percent, and a bite or burn of 10 or more its full size; smaller wounds heal clean; washing halves this, and a dressing halves it again (`BIO-23`)."
- `BIO-05` Lockjaw: "(about 1 in 20 cuts or bites of 20 or more from teeth, horn or a wooden point, or dirtied with earth or dung; 3–15 days)".
Cross-section: `MAT-04` (G4 can keep its hurt rates, since craft slips under 10 no longer get infected).

### 4. [major] SCP-16, BIO-05 Illness arrives at MIL-04, after the fire pace is tuned on worlds where hardly anyone dies
Problem: `TIM-18` makes illness and birth risks the source of deaths before old age, but illness only enters at `MIL-04` (`SCP-16`).
From `MIL-01` to `MIL-03` the first region's bands lose only some babies and mothers at birth, the old, the starving and the frozen.
So most children grow up, and numbers grow about 2.5% a year (about 4.5% with `BIO-15` as written).
Before `MIL-04` the nightly pace runs use the first region (`RES-07`, `RES-21`).
In them, 45–120 people become about 100–260 by Year 30 and 220–590 by Year 60, but the first region's 1,600 km² feeds only about 130–400 (`WLD-30`: a band of 25 needs 100–300 km²).
The fire window (Years 5–30) is tuned at `MIL-03` on these runs, then shifts when illness cuts growth to a third at `MIL-04`, so it has to be tuned twice.
Also, `MIL-01` lists "pairing", but the only rule for pairing is courtship (`MND-33`), which arrives at `MIL-05`.
Fix (G1, `SCP-16`):
- `MIL-03` adds "the everyday illnesses and who dies of them (`BIO-05`)", meaning the ones people catch from each other, water, food and wounds; they need only the wounds, weather and water already there.
- `MIL-04` reads "illnesses from animals, and the healing blueprints".
- "Pairing" moves from `MIL-01` to `MIL-02`, beside kin and friends, as `MND-33`'s courtship.
Crowd illnesses need villages, so by their own rule they come with `MIL-07`.
Cross-section: `SCP-16`, `MIL-01` to `MIL-04` (G1); `MND-33` (G6: courtship delivered at `MIL-02`).

### 5. [major] BIO-04, RES-07, PLT-04, PLT-10 The population target is checked by runs that don't exist, and sized for worlds it never makes
Problem: `BIO-04`'s Done when asks for "at least 16 of 20" whole worlds at Year 400.
But `RES-07` runs 20 worlds only as far as Year 150, and only 10 to Year 500, so the Year 400 range gets 10 runs and stays "provisional" for good (`RES-13`).
Growth of 0.8% a year doesn't stop at Year 400: worlds hold about 2,200–6,700 people at Year 500, when the full pace test ends, and tens of thousands by Year 800 unless the memory pause comes first (`MND-15`).
Yet `PLT-04` sizes kept areas "after 500 game years with 2,000 people", and `PLT-10` sizes storage for "a world of 1,000 people after 1,000 game years".
`BIO-04` never makes either of those worlds.
The last century of the full test also runs above the 2,000 design point, where speed falls below half a game year a minute (`TIM-07`), and `RES-07`'s budget must allow for that.
Fix:
- `BIO-04` Done when: "the pace tests' 20 worlds to Year 150 meet the death, life-span and birth ranges in at least 16, and at least 8 of the full test's 10 hold 1,000–3,000 people at Year 400 (`RES-07`)." (196 characters.)
- G9: `PLT-04` Memory "after 500 game years grown as `BIO-04` sets (about 2,000–7,000 people)".
- G9: `PLT-10` target "a world grown as `BIO-04` sets, after 500 game years".
- G9: `RES-07` states the full test's session-hours with its last century at 2,000–7,000 people.
Cross-section: `RES-07`, `PLT-04`, `PLT-10`, `RSK-14` (G9).

### 6. [major] BIO-20, MND-28, MND-14 The start map fills each mind's place memory before play begins
Problem: `BIO-20` gives each adult "the world cells within about 10 km of their shelter": about 314 cells, each with its water, caves, food by season, stone and dangers.
`MND-28` counts a place as a spot in an area or "a stretch known roughly ... at most one to a world cell", and `MND-14` caps places at "a few hundred".
One builder will make 314 one-cell places per adult.
That fills the cap on Year 1: the least-used parts of the home range fade within the first seasons, and each new spring or flint bank pushes out start knowledge.
Another builder will make a few dozen stretches.
"The other bands' camps" is read from cells within 10 km.
But 3–4 bands, each needing its 10 km (`WLD-24`), in a first region of 40 by 40 km (`WLD-34`) often camp more than 10 km apart, so the line often gives nobody the other camps, though one adult in three has a brother or sister there (`BIO-03`).
Fix:
- `BIO-20` Mental map: "their home range within about 10 km of their shelter, as about 40 places at world-cell scale or larger (valleys, ridges, springs, caves, stone), with water, food by season and dangers read from the cells (`WLD-12`, `MND-28`), and where the other bands camp; no area is made in advance."
- G6: `MND-28` "a stretch known roughly (a valley, a ridge), one or more world cells".
Cross-section: `MND-28`, `MND-14` (G6).

### 7. [major] BIO-22 The looks Done when can't pass under its own rule
Problem: the Done when asks that "the average of at least one look feature differs by a fifth of its range between peoples apart for 200 years".
Two-copy inheritance drifts slowly.
Simulated (a people of 150, a new people founded by 10–20 of them, about 8 generations, 5 features, both growing 0.8% a year), the largest of the five differences reached a fifth of the range in only about 20–50% of pairs, and a tenth in 85–97%.
Real peoples took far longer than 200 years to look different, so the rule is right and the test is wrong.
Only the full test's 10 worlds have peoples apart that long, so the test can't run anywhere else either.
Fix: replace the "Peoples apart" line with "**Founders:** a people grown from a few families keeps their looks, which never help anyone survive."
End the Done when at "children visibly mix their parents' looks."
If the coordinator wants the promise checked: "... and in at least 6 of the full test's 10 worlds, two peoples apart for 200 years differ by a tenth of the range in some look feature (`RES-07`)."
Cross-section: none.

### 8. [major] BIO-05, BIO-19, MAT-17 The numbers that start illnesses are missing, and they decide most deaths
Problem: `TIM-18` makes illness the main killer before old age, so how often each illness starts decides `BIO-04`'s ranges.
`BIO-05` gives a principle ("about as often per game year as in a real year"; crowd ones "at a small chance each year") but no rate for any illness.
`BIO-19` gives no share of carrier animals that are sick ("a few", "a small share"), and worms give no chance per raw meal.
A builder who reads worms' route as certain gives every forager worms, which never clear without medicine 3 that nobody knows at the start.
Everyone then loses about 1 condition a day, which means a fifth more food, until cooking (Years 5–30).
`MAT-17`'s Complete check covers items and blueprints but not illness entries, so nothing makes these numbers exist.
Fix: add to `BIO-05` Starting: "Each illness's entry gives its start rate per game year in a band of about 30, the share of its carriers that are sick, or its chance per raw meal (worms: about 1 in 100), as the catalogue check requires (`MAT-17`)."
Cross-section: `MAT-17` (G4: the Complete check covers illness entries), `WLD-32` (G3: which kinds carry which illness).

### Cuts
Section 8 is 31.7 KB.
The cuts below take out about 2.8 KB, and the fixes above add about 0.6 KB, which leaves about 29.5 KB.
Each cut removes a line another item already owns (C1) or an explanation (C3); no rule or number is lost.
1. Section intro: cut the first line ("Every body must be fed, ..."), a summary of the headings. 0.13 KB.
2. `BIO-01`: end the second sentence at "(`SCP-01`)."; the rest repeats `SCP-01`. 0.10 KB.
3. `BIO-03`: cut "Together they are one people, with one language and a name for themselves", which `CUL-23` owns ("The first people"). 0.10 KB.
4. `BIO-08`: cut the Uses line, since each use is stated where it applies (`BIO-05`, `BIO-13`, `BIO-18`, `BIO-21`, `MND-06`); `BIO-09` Hunger gains "more for the big (`BIO-08`)". Net 0.24 KB.
5. `BIO-09`, `BIO-13`, `BIO-16`: cut "seen as thin or stout (`PRE-27`)", "shown on the figure and card (`PRE-27`, `PRE-35`)" and "and hair greys and backs bend (`PRE-27`)"; `PRE-27` owns what figures show. 0.13 KB.
6. `BIO-10`: cut "Cooking raises food a step (`RCK-03`), and quality never changes it (`MAT-20`)."; both owners say so. 0.09 KB.
7. `BIO-11`: cut "; wet clothing keeps half its warmth (`RCK-26`)". 0.05 KB.
8. `BIO-12`: cut "Both are hidden until tried or tasted" (`MAT-03` and `MND-04` own it) and "; rotting raises poison, and cooking and leaching lower it" (`MAT-19`); "helping in small amounts and harming in large ones", which no rule backs, becomes "in amounts that matter". 0.16 KB.
9. `BIO-05`: cut ", with every time a game value (`TIM-18`)" (the intro says it); cut the babies' numbers in coughing fever and gut sickness (the 5-times rule gives them); cut "so others may keep away (`MND-05`)" (`MND-05` owns it). 0.14 KB.
10. `BIO-05` list, optional: move the symptom words (sniffles and cough, aches and cough, hard breathing, cramps and the runs, and so on) to the catalogue (C2), keeping routes, waits, courses, deaths and immunity. 0.24 KB.
11. `BIO-23`: cut "people known for healing are sought out (`CUL-32`)".
    Shorten the named healing results to "washing, dressings, splints and burn salves, each as `BIO-13` says, and herbal medicines (`BIO-12`)" (names go to the catalogue, C2).
    Write "Rites and comfort lift mood (`MND-29`) but cure nothing ... (`MND-05`, `CUL-34`)", which fixes "ease pain".
    Cut the Done when's washed-wounds clause, which repeats `BIO-13`'s. 0.22 KB.
12. `BIO-14`: shorten the summary to "Every death comes through the body." 0.03 KB.
13. `BIO-04`: cut "from about 5, children help ... (`MND-13`)", which `MAT-12` owns; cut ", mostly before 70, in game years (`TIM-18`)" from the summary (the ranges and the intro say it). 0.16 KB.
14. `BIO-15` The record: "mother, father, date and place of each birth (`PRE-10`)". 0.02 KB.
15. `BIO-16`: cut "so elders matter as keepers ... (`CUL-02`)"; cut "with frailty in illness, cold and falls, this gives the ages at death of `BIO-04`"; cut the Done when's "ages at death on whole worlds match `BIO-04`", which is `BIO-04`'s own check. 0.20 KB.
16. `BIO-18`: cut the Taste line and write "Sight, hearing, smell and taste (`MND-21`)", since `MND-21` owns taste. 0.08 KB.
17. `BIO-21`: "Walking includes wading, swimming and climbing (`PRE-44`)", which also stops promising a wading animation. 0.03 KB.
18. `BIO-06`: cut the No evolution line (the summary says nothing evolves, and How says why); cut "so tall parents have tall children, though less tall". 0.17 KB.
19. `BIO-22`: as finding 7. 0.19 KB.
20. `BIO-19`: cut the Counts line (`WLD-32` owns counts, `MAT-11` owns catches).
    Illness becomes "a few of the kinds that carry an illness are sick and pass it on (`BIO-05`)", since the rates go to the entries (finding 8).
    Cut the Done when's "weak animals die first in a hard winter" (`WLD-18`'s rule). 0.32 KB.

Outside section 8, the same once-only rule (C1):
- `MND-03`'s Sight line repeats `BIO-18`; write "**Sight:** as `BIO-18` sets out" (G6, 0.13 KB).
- `MND-16`'s "far-off herds, and small animals everywhere, are counts ... (`MAT-11`)" repeats `WLD-32` (G6, 0.13 KB).
- `CUL-28`'s "every 10–20 years" repeats `BIO-05` (G7, 0.02 KB).
