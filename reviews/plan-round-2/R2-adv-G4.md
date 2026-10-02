# R2 adversary G4: Things and blueprints (section 7)

Read: ROUND.md (with its Round 2 part), BRIEF.md, notes/R1-coord.md, notes/R1-impl-G4.md and my round 1 findings, then assembled.md in full.
Section 7 was read line by line, and every citation of a `MAT` or `RCK` ID elsewhere (about 190) was checked against what the item now says.
Result: 2 blockers, 8 majors, then cuts.

## Did round 1 land?

Landed as decided, and matching the other sections:
- D1 and D3: timer lengths are game values (`TIM-18`), and `RCK-23` cites `WLD-30`.
- D7: `MAT-16`'s stages match `SCP-16` and `RES-07`.
- D11: one catch rule in `MAT-11`, cited by `WLD-32`, `BIO-02`, `BIO-19` and `MND-16`.
- D15: one action per blueprint, roles, class and form, four kinds of result, values from the main input, five firings, the hint field; `PRN-07`, `MND-11` and the glossary follow.
- D16, D17 as fire rules, D18's counts (`MAT-10`, `MAT-23`, `VIS-06`, `SCP-13`, `RSK-25`), D19, D29, D31, D32 and D33.
- All 25 of my round 1 findings are in the text.

Landed wrong, or only in part:
- D15's Discoverable check, as worded, fails fire-making (#1).
- D17 landed as fire rules only: nothing makes anyone blow a kiln, or puts ore in one (#2).
- My own round 1 wording, "wear and quality moving only the main characteristic", opened a hole in `RCK-01` (#3).
- D18's fallback is stated twice, in `MAT-23` and `RSK-25`, with different rules (#6).
- Only pain has a number for spoiling a try (`BIO-13`); `BIO-09` and `MAT-12` send the reader to `MAT-04` for the rest, and `MAT-04` sends them back (#7).
- Size: section 7 is 39.6 KB against its 36 KB target (Cuts).

## Findings

### 1. [blocker] MAT-17 MAT-04 MOM-01 MND-11 The Discoverable check fails fire-making, the arc's second step
Problem:
`MAT-17` passes a blueprint only if it is "fitted, or hinted, by a plain use or a blueprint people can know".
Fire by drilling uses drill, and fire by ploughing uses grind; neither is a plain use (`MAT-06`), and no blueprint people can know before fire drills or grinds dry wood against dry wood.
Their hint, smoke while drilling, is their own failure, met only while experimenting or at play (`MOM-01`, `MND-10`, `MND-11`).
So the check, which blocks every change it fails (`MAT-15`), fails both fire routes, and every other blueprint whose only first route is an experiment, which `MND-11` allows ("any base action ... on one or two things in reach").
Read loosely instead, so that a blueprint's own failure counts as a hint "from a blueprint people can know", the check can no longer fail anything.
"Reachable" also starts "from what the start region holds", though clay, wild grain and copper ore need only be on its landmass (`WLD-10`).
Fix:
- `MAT-17` Discoverable: "**Discoverable:** each blueprint has a first route the check names, needing only the start knowledge and earlier routes: an accident (a plain use or a knowable blueprint fits it), a hint (the outcome of such an activity, or the blueprint's own failure sign, gives a hunch for it) or an experiment (any base action on one or two things in reach, `MND-11`); a blueprint with three or more inputs needs an accident or a hint, and any with no route fails."
- `MAT-17` Reachable, opening: "from what the bands know at the start (`BIO-20`) and what the land can hold (`WLD-10`, `WLD-14`), ...".
- `MAT-04` field 9: "**Hinted by:** outcomes of plain uses, blueprints or timers, or its own failure sign, that give a hunch for it (`MND-11`), such as smoke while drilling."
Cross-section: G6 (`MND-11` Chances: "a failure sign, or another outcome the blueprint marks as its hint, is a surprise ...").

### 2. [blocker] MAT-18 MAT-19 MAT-23 RCK-22 MOM-12 Copper: heat 5 can exist, but nobody has a reason to make it, and nothing puts ore in it
Problem:
Round 1 made heat 5 possible: an enclosed charcoal fire blown through pipes the whole time (`MAT-18`).
Nothing makes anyone do it:
- Blowing is a plain use (`MAT-06`), a firing asks only for feeding (`MAT-04`, `TIM-17`), and no rule says what a firing gains at 5, so no choice (`MND-09`) ever scores an hour of blowing in turns.
- `MAT-18` lights tinder from an ember within a minute whether or not anyone blows, so lighting never needs breath, and `RCK-22`'s check can't tell (#5).
- Nothing puts ore "gathered for their colour" (`MOM-12`) among a kiln's fuel.
- No blueprint holds the practice, so `MOM-12`'s scene ("potters who blow their charcoal kilns") can't be set up without scripting (`RES-18`).
So copper happens in no whole world, and the full pace test (`RES-07`) must fail the last step of the arc.
`MAT-18` also never says how many pipes a furnace needs, so one builder's furnace is one person with a reed and another's a crew.
Fix:
- `MAT-18` Lighting: "an ember in dry tinder (burn 4–5), such as a tinder nest, flares into a small fire within a minute while blown, and unblown dies within minutes about half the time (tuned, `RCK-22`); things with water 3 or more don't catch, and damp a fire."
- `MAT-18` Highest level: "... and any enclosed charcoal fire 5 while air is blown in the whole time through at least two reed or clay pipes (tuned) or a hide bellows, a blower at each, in turns; ...".
- `MAT-19` Firing: "clay at heat 3 for about 4 hours becomes fired clay, a step tougher at 4 and again at 5, where it takes half the time (`RCK-04`)"; and "green ore among charcoal at 5 for an hour gives copper, a bead from a lump and bright specks from paint of ground ore (`RCK-08`)".
- `MAT-23` Results other items name, add: "the blown firing (pottery: a charcoal kiln fired with blowers at its pipes in turns as its tending, hinted by blown embers flaring, `MOM-12`)".
Copper then has a route a world can walk: potters learn the blown firing for tougher pots, paint pots with ground green ore (`CUL-09`), and notice specks of metal.
Cross-section: G1 (`MOM-12` step 1: "Potters who know the blown firing blow into their charcoal kilns through pipes, so it reaches heat 5 (`MAT-18`, `MAT-19`)"; step 2: "Pots painted with ground green ore (`CUL-09`) come out flecked with metal (`MAT-19`, `RCK-08`)"; Check: "potters who know the blown firing and paint their pots green"); G7 (`CUL-09`: pot ornament may use green ore); G9 (`RES-16`'s example stands).

### 3. [major] MAT-20 MAT-04 MAT-17 RCK-01 RCK-10 RCK-11 Quality can push a thing across a reality rule, and the checks look only at items
Problem:
`MAT-04` lets "wear and quality" move "the main characteristic", and raw things take their source's quality (`MAT-20`), drawn from the seed (`WLD-14`, `WLD-31`).
No raw item names its main characteristic.
A builder who takes flaking as a stone's gives a quality-5 quartzite cobble (flaking 2) flaking 3: it fits the flake blueprint, `RCK-01` breaks, and the sharp-stone control (`RES-03`) can fail.
The same hole gives fine grass (fibre 2) the fibre 3 that cord needs (`RCK-11`).
`MAT-17` checks each reality rule "against every item", never against states, wear or quality, so no check can catch it.
`MAT-20` also takes wear "off an edge" while `MAT-04` moves the main characteristic; for a cloak the two disagree.
And quartzite did flake in life, coarsely, so naming it beside granite as never flaking is a believability slip.
Fix:
- `MAT-20` Effect, opening: "For a made thing, at 0–1 its main characteristic, as its blueprint names it, is a step lower ..."; and at its end: "a raw thing's quality moves none of its values and counts only as a fine or poor input (`MAT-04`)."
- `MAT-20` Wear: "each step taking a step off its main characteristic".
- `MAT-04` What fits: "... wear and quality moving only a made thing's main characteristic (`MAT-20`)".
- `MAT-17` Reality rules: "each rule in 7.6 is checked against every item in every state it can take, new and worn, at quality 0 and 5".
- `RCK-01`: "others, such as granite and sandstone, never do"; Check: "only stones on the check's own list have flaking 3 or more in any state, no blueprint gives edge 4 or more from a stone below flaking 3, and without them the sharp-stone test never makes a flake (`RES-03`)."
- `RCK-10` Check, first clause: "every stone of flaking 3 or more has a treated state a step higher, or quality if flaking is 5, and no other stone's treated state reaches 3".
Cross-section: none (`WLD-14` and `WLD-31` keep "quality drawn from the seed").

### 4. [major] MAT-22 MAT-17 MND-09 Long chains stall: a step more than three back from any use is worth nothing
Problem:
`MND-09` values a thing "directly (eaten, worn, burned, slept under, stored) or as a tool or input to blueprints they know whose results do, up to about three steps back".
With one action per blueprint, `MAT-22`'s cloak takes twelve steps; a greased hide is four steps back from the cloak, and a fresh one seven.
Unless a dried or softened hide has a use of its own, nobody greases or scrapes a hide for a cloak, so `MAT-22`'s Done when and the cloak's chain scene (`RES-23`) fail while every catalogue check passes.
The same can befall pit and post houses and the blown firing, which villages and copper need; `MND-22`'s "short plans of a few steps" don't clearly carry a twelve-step chain either.
Fix:
- `MAT-17`, new bullet: "**Worth in reach:** every launch result has a direct use as `MND-09` lists them, or is at most three blueprints from one that has; a longer gap fails."
- `MAT-22` Example, one line for its two: "a flake (strike) makes a scraper (press); a fresh hide scraped (scrape) and dried on a frame (dry) is bedding; greased (apply) and rubbed soft (press), a wrap; cut (cut), pierced with an awl (drill) and sewn with sinew thread (bind), a cloak, warmth 3, or 5 with the fur on (`RCK-26`)."
Cross-section: G6 (`MND-11` copying: a weak hunch for each step whose mark the thing shows, so a copied cloak doesn't stall at its first step).

### 5. [major] RCK-22 RCK-08 RCK-14 RCK-02 RCK-04 MAT-17 Some reality checks can't fail, and none says at what level its trials run
Problem:
- `RCK-22`: tinder catches from an ember whether or not anyone blows (`MAT-18`), so "blown embers make a small fire within a minute in at least 150 of 200 tries" passes even if blowing does nothing.
- Trials run at a low and a high level (`RES-24`), but no reality check says which; 150 of 200 needs a level about three above the difficulty, so `RCK-08`'s copper trial fails at any low level and passes at a high one, by the builder's choice.
- `RCK-14` checks "stored dry grain and nuts a year (`MAT-19`)", but `MAT-19` says nothing of grain or nuts, which no rule lets rot.
- `RCK-02`'s "wet tinder never catches" is checked only as an input range, not where tinder catches (`MAT-18`).
- `RCK-04`'s unfired clay "turns back to wet clay when soaked": a new item from soaking, which `MAT-19` forbids ("only five firings make a new item").
- `MAT-17`'s own Check restates `PRC-10` and never shows that a check can fail.
Fix:
- 7.6 intro, second line: "Checks that need chance use trials of about 200 tries (`RES-24`), at level 10 where a rule says something happens and at every level where it says it never does, or scenes of 20 runs (`RES-13`)."
- `RCK-22` Check: "in trials, embers in dry tinder make a small fire within a minute in at least 150 of 200 tries when blown and at most 120 when not, a charcoal kiln blown through two pipes reaches 5 and falls to 4 within minutes of stopping, and earth puts a campfire out (`MAT-18`)."
- `MAT-19` Rotting: add "damp nuts and grain" to its list, and "kept dry, nuts and grain last about a game year".
- `RCK-02` Check, add: "and tinder with water 3 or more never catches (`MAT-18`)".
- `RCK-04` Check, opening: "unfired clay has waterproof 0 and, soaked, falls apart into wet clay, what it breaks into (`MAT-20`); ...".
- `MAT-17` Check: "a test catalogue with one planted fault for each check above, a broken reality rule included, fails every one."
Cross-section: none.

### 6. [major] MAT-23 RSK-25 MAT-17 The fallback set is two different rules in two places, and can't be met as written
Problem:
Decision 18 gave the fallback to `RSK-25`, yet `MAT-23` states its own: "about two thirds of each [sector], keeping every result other items name".
`RSK-25` says "about two thirds of each count ..., keeping every chain the pace steps need and, for each kind of land, a grazer, a hunter, a food plant and a fibre plant".
Two thirds of each sector can't keep every named result: all 3 music results are named (`CUL-10`, `SND-06`), 6 of 7 healing ones (`BIO-23`), and herding's milk, pens and tethers (`WLD-33`).
Neither rule makes the fallback set pass the catalogue checks, and nothing marks which entries are in it, so "agreed now" agrees on nothing.
Fix:
- `MAT-23` How many: end at "music 3", then "; the fallback set is `RSK-25`'s."
- `RSK-25` Response, the fallback sentence: "A fallback launch set is agreed now and marked in the catalogues: about two thirds of each count (about 125 items, 95 blueprints, 40 plants, 20 wild animals and 10 illnesses), keeping every result another item names, a full chain to every step of the arc and, for each kind of land, a grazer, a hunter, a food plant and a fibre plant, and passing every catalogue check on its own (`MAT-17`)."
- `MAT-17`, end of How it works: "The checks run on the full launch set and on the fallback set alone (`RSK-25`)."
Cross-section: G9 (`RSK-25`).

### 7. [major] MAT-04 MAT-11 MAT-12 BIO-09 Missing numbers: what spoils a try, what a good tool saves, where a blueprint fits, what a snare catches
Problem:
- Chance: "tiredness, pain, cold and darkness take more" has a number only for pain (`BIO-13`); `BIO-09` and `MAT-12` send the reader to `MAT-04`, which sends them back.
- Time: "an activity shorter with better tools" has no rule, so `MAT-04`'s Done when ("takes about as long ... as its fields say") has nothing to measure.
- Fit: neither `MAT-04`'s What fits nor `MND-11` ("fits the action and inputs") mentions the place, so one builder lets sowing in winter, or cooking on embers, teach a blueprint, and another doesn't.
- Small game: `MAT-11` gives a catch "by a chance set by how many live there, its quality and the hunter's skill" with no number, yet its Done when checks snares "at their stated chance", and small game is the bands' first meat (`BIO-02`).
Fix:
- `MAT-04` Chance, last clause: "...; tiredness (rest below 20), clumsy cold (`BIO-11`), darkness beyond firelight and an arm below half health each take a tenth and slow work by a quarter, and pain and sickness act as `BIO-13` and `BIO-05` say."
- `MAT-04` field 6: "**Time:** the work, an activity a tenth shorter for each step the tool's main characteristic is above its range's lowest, at most a third, then any waiting ...".
- `MAT-04` What fits, add: "a blueprint fits only while its place holds, for accidents and experiments as for makers".
- `MAT-11` Small game, the chance clause: "a set snare or trap takes one a night about 1 time in 10, a baited hook one an hour 1 in 20, and a throw hits 1 in 4 (tuned), for hunting experience 4 where its kind is at its usual numbers, in proportion to how many live there, each level above or below and fine or poor quality moving it by a tenth of itself".
Cross-section: G6 (`MND-11`: "fits the action, inputs and place").

### 8. [major] MAT-19 MAT-13 MAT-14 RCK-13 Timers: unmeant results, the five firings and states still read two ways
Problem:
- "Without [a blueprint], the result is as for no skill" doesn't say whose chance settles a timer nobody meant, or whether a timer no blueprint fits, such as rotting, always runs; one builder cooks meat dropped by the fire every time, another at a level-0 chance.
- "Only five firings make a new item" doesn't say where they are written: as a table they name clay, birch bark and ore as inputs, which `MAT-13`'s check fails, and a bark added later could never give tar (`MAT-14`).
- States shift "set values for its class", but `RCK-13` needs crushed acorns to leach and whole ones not, so the shift depends on form too.
Fix:
- `MAT-19` States: "a finished timer gives its thing a state, such as dried, cooked, rotten or tanned, shifting set values for its class and form, so only crushed things leach (`RCK-13`); only five firings make a new item, each written in its input's own entry (`MAT-13`): fired clay, red ochre, charcoal, tar and copper."
- `MAT-19` Meant or not: "**Meant or not:** a blueprint can start a timer, such as firing pots, its chance at the maker's level settling the result (`MAT-04`); a timer nobody meant is settled at level 0 of the blueprint its thing and conditions fit, and one no blueprint fits, such as rotting, always runs, so meat fallen in the fire, or copper in a blown kiln, can be noticed (`MND-10`, `MOM-12`)."
Cross-section: none.

### 9. [major] MAT-17 MAT-13 MAT-04 Expected fits and hints break the catalogue's own naming rule
Problem:
`MAT-13` lets an entry name others "only as results, never as inputs", and its Check fails any entry naming another as an input.
`MAT-17`'s Expected fits has "each new item or blueprint" name "in its own entry what it is meant to fit": a blueprint naming the items it should fit names inputs, which `MAT-13`'s Check fails, and an item naming blueprints names them neither as results nor as inputs, which its rule forbids, so every launch entry breaks one or the other.
`MAT-04`'s Hinted by invites the same breach whenever a hint names another blueprint.
Fix:
- `MAT-17` Expected fits: "**Expected fits:** a list kept with the checks, never read by the game, names the blueprints each item is meant to fit; each new item or blueprint adds its own lines (`MAT-15`), and any fit not on it fails until the ranges are narrowed or the fit is added with a reason, so an axe of bark fails (`RSK-06`)."
- `MAT-13` How it works, add: "hints too name actions, signs and kinds of thing, never entries."
Cross-section: none (`RSK-06` stands).

### 10. [major] MAT-06 MAT-21 GOD-03 MND-11 RES-02 Knowing an action, what a plain use makes, and when a first counts are unsaid
Problem:
- `GOD-03` lets a dream hint a blueprint only if the dreamer knows "its action", and `MND-11`'s experiments try "a known action" first, but nothing says what knowing an action is: all 21 from birth, or each once done.
- Plain uses "take, move, wet, warm or dry things", and "anything else an action makes or changes needs a blueprint"; whether a dug hole is plain (graves, storage pits, `MOM-09`) or a blueprint's ground change (`PRE-42`: "such as a pit") is open, and burial customs (`CUL-06`) wait on the answer.
- `MAT-21` names a people's "first success", while `MAT-04` teaches a success only "if noticed"; an unnoticed accidental flake could count for the pace test (`TIM-19`) though nobody learned anything.
- `MAT-21`'s Done when wants "a second people's first" from the sharp-stone test, which has one band for at most 7 years (`RES-02`).
Fix:
- `MAT-06`, a line after the list: "**Knowing an action:** a person knows an action once they have done it in any way, as dreams and experiments ask (`GOD-03`, `MND-11`)."
- `MAT-06` Plain uses: "... can start timers (`MAT-19`), and leave heaps, holes such as a grave, and cleared ground; anything else an action makes or changes, such as a frame, a sown plot or a house, needs a blueprint, ...".
- `MAT-21`: "A people's first noticed success (`MND-10`) at making one is a named discovery, ...".
- `MAT-21` Done when: "in the sharp-stone test (`RES-02`), every first flake gives an entry with who, when, where, route, inputs and word, and in a scene of two peoples, the second's first flake a short one."
Cross-section: G8 (`PRE-42`: "such as a pit or a dressing" becomes "such as a sown plot or a dressing"); G6 (`MND-11`'s "known action" cites `MAT-06`).

### Cuts

Section 7 is 39.6 KB, with a 36 KB target.
The cuts below save about 2.8 KB; the fixes above add about 2.5 KB, mostly as replacement lines (#7 and #10 most), so section 7 nets about −0.3 KB, and about −2.1 KB with the deeper cuts after the list.
Nothing needed is lost: each cut is stated by its owner elsewhere, cited by nobody, or replaced by a fix.

1. `MAT-16`: the seven-line list becomes one line: "Each milestone adds only the entries its steps need (`SCP-16`), never rewriting earlier ones, and later layers grow the same way (`VIS-03`)." `SCP-16` holds each stage's contents and `RES-07` each step's stage. **0.6 KB.**
2. `MAT-22` Example: two lines become #4's one line (the shorter routes it drops reappear in it as bedding and a wrap). **0.2 KB.**
3. `MAT-23` Results other items name: drop the example list ("such as the tinder nest ... pit and post houses (`CUL-28`), and"), keeping the rule, the milk definition and #2's blown firing; the Done when checks every named result. **0.17 KB.**
4. `MAT-04` opening: drop its second sentence ("Nobody knows one until ... fits its ranges (`PRN-07`)."); `PRN-01`, `SCP-04` and `PRN-07` own all three facts. **0.17 KB.**
5. `MAT-05`: its summary becomes "Every value is a plausible estimate, set by hand and tuned (`PRN-05`)."; drop ", and tuned values are logged with what they were tuned against (`RES-16`)", which `RES-16` says. **0.17 KB.**
6. `MAT-12`: "and a hurt arm, tiredness, cold, pain, sickness and darkness slow work and lower its chance (`MAT-04`, `BIO-13`)" becomes "and the rest slow it as `MAT-04` says" (numbered there by #7); drop "from about 5, children do light work ... (`CUL-01`)", which `BIO-04` states, and let the Done when cite `BIO-04`. **0.15 KB.**
7. `MAT-18` Spreading: replace "; in an area a wildfire moves as a front ... it follows `WLD-28`" with "; a wildfire follows `WLD-28`", which states the front. **0.15 KB.**
8. `MAT-23`: the "Left out" line; no item cites a cut result. **0.13 KB.**
9. `MAT-20`: the line "It shows on the thing (`PRE-42`), so a band that loses its best knapper sees cruder blades (`MOM-02`)."; `PRE-42` and `MOM-02` say it. **0.11 KB.**
10. `RCK-09` **Rot** becomes *Dropped*: "**Dropped because:** rotting is a timer (`MAT-19`), whose times and frozen rule its Done when checks." **0.11 KB.**
11. `MAT-18` opening: drop its second sentence ("It burns fuel by the fuel's value, spreads ... and is fed or smothered."); Burning, Spreading and Putting out say each part. **0.1 KB.**
12. `MAT-01`: the line "Each part of a made thing keeps its material (`PRE-42`): a broken spear leaves a shaft and a point."; `PRE-42` and `MAT-20` cover it. **0.1 KB.**
13. `MAT-10` Also listed: drop "; food groups follow from a food's source (`BIO-10`), sounds from action and class (`SND-06`)"; their owners say it. **0.1 KB.**
14. 7.6: the five group headings (Stone, fire and food; Crafts; Colour and art; Growing and taming; Dropped). **0.1 KB.**
15. `MAT-06` opening: drop ", and each has its animation (`PRE-44`) and sounds by material (`SND-06`)"; its Done when says it. **0.07 KB.**
16. `MAT-09` Check: drop "; whole-world runs flag any thing from nothing outside growth (`RES-12`)"; `RES-12` says it. **0.07 KB.**
17. `MAT-18`: the three temperatures in brackets in its opening line. **0.07 KB.**
18. 7.6 intro: drop ", fixing the orders that decide what is possible (`MAT-05`, `RSK-06`)"; `MAT-05` says it. **0.07 KB.**
19. `MAT-21`: drop ", and a lost craft and its return are marked (`CUL-02`)" (`CUL-02` and `PRE-39` own it) and "and shown with its meaning" (`PRE-38`). **0.08 KB.**
20. `MAT-02`: drop ", so a flake has an edge and a nodule hasn't"; `MAT-03` says it. **0.05 KB.**
21. `MAT-11` Toppling: drop "; burning follows `MAT-18`". **0.03 KB.**

Deeper cuts, if the coordinator needs section 7 nearer 36 KB, each with what it loses:
- `MAT-04`'s worked example to the catalogue: 0.6 KB; loses the only fully worked blueprint, which two builders most need.
- `MAT-06`: one example per action instead of two to four: 0.5 KB; loses round 1's one-home-per-example map.
- `MAT-03`: one typical value per characteristic: 0.35 KB; loses one anchor of each scale.
- `MAT-15` becomes *Dropped* ("merged into `MAT-17` and `PRC-10`"): 0.2 KB; `MAT-17` and the header's rules list stop citing it.
- `MAT-11`'s throwing ranges to the catalogue: 0.2 KB; its Done when then cites the catalogue.
