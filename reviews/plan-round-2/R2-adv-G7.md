# R2 adversary G7: section 10, Culture and society

Read against `assembled.md` (346.6 KB), `notes/R1-coord.md` and `notes/R1-impl-G7.md`.

**Round 1 check.** Decisions 5, 6, 13, 14, 20, 29, 31 and 32 landed in section 10 as written.
Five gaps remain where section 10 meets other sections:
- `RES-07` gives no stages for the culture targets, and still reads them "in their books of ages" (#1).
- `MND-24`'s four sources of respect differ from `CUL-06`'s six respect answers (#5).
- `MND-33` restates the topics-per-chat number that decision 14 gave `CUL-24` (Cuts).
- The glossary has no "Gathering" and calls a festival "a custom" (#7).
- `CUL-28` restates `BIO-05`'s crowd illnesses but leaves out its condition of about 200 people with herds (Cuts).

**Population.** Bands of 10–40, villages up to 300 and `MND-14`'s cap of 150 known people fit `BIO-04`'s 1,000–3,000 people at Year 400.
Nothing to fix there beyond #4 and #7.

**Expression** is bounded: 1–8 motifs a picture, scales of 4–6 notes, songs of 8–16 notes, 4–8 dance moves and 12 story shapes.
The content behind it can shrink further (Cuts).

**Size.** Section 10 is 33.8 KB now.
I measured a draft with every fix below applied: the fixes add 1.2 KB, the cuts remove 3.0 KB, and the section ends at about 32.0 KB.

### 1. [blocker] CUL-33, RES-07 Culture targets are judged before the stages that build them
Problem: Two rules combine badly:
- `RES-07` judges every `CUL-33` target whose window closes within its years, nightly to Year 60, but its Stages line maps only the `TIM-19` steps.
- `PRC-10` needs the pace tests passed before any stage closes.

So from `MIL-02` the nightly run judges the first shared spirit (within 5), rite (5–20), myth (10–40), band split (10–50) and festival (10–60).
None of these exists before `MIL-05` or `MIL-06` (`SCP-16`), so as written `MIL-02` to `MIL-05` can never close.

The same rows have three more slips:
- "First new people: 60–150" can't be reached. The start bands are linked as of Year 1, a link lasts 25 years, and the parts must then stay unlinked for 50 (`CUL-23`), so the earliest new people is about Year 76.
- "Keeps going" is not a window, and "the pass rule of `TIM-19`" doesn't exist (`RES-07` holds the rule for windows).
- `RES-07` reads the targets "in their books of ages", but new songs and rites never go there (`CUL-07`).

Fix: In `CUL-33`, "First new people: 60–150" becomes "First new people: 80–150".
Replace its Check with:
  "- **Stages** (`RES-07`): spirits, rites and myths count from `MIL-05`; splits, festivals, feuds, new peoples and raids from `MIL-06`; chiefs from `MIL-07`.
  - **Check:** the pace tests (`RES-07`) read these from each world's records; Keeps going passes in at least half the worlds."
In `RES-07`, "in their books of ages" becomes "in their records", and Stages adds "and each culture target from the stage `CUL-33` gives".
Cross-section: `RES-07` (G9).

### 2. [major] CUL-22, CUL-28 Chiefs need a village that "shares out" what families own
Problem: Chiefs appear only "in a village that shares out stores, fields or herds", but no rule ever makes a village share these out.
`CUL-21` and `CUL-28` give stores, fields and herds to families, and the only sharing custom is for a big kill (`CUL-06`).
Built literally, no chief ever appears, so `CUL-33`'s 150–350 window and `MIL-07`'s chiefs fail; otherwise each builder invents a sharing rule.
Three more gaps in the same item:
- The chief's powers "settling quarrels" and "sharing out stores" have no rule: whose stores, when, how much.
- Councils meet "for plans concerning all present", with no list of which plans.
- "The plan with the most respect behind it" doesn't say whose respect counts.

Fix: In `CUL-22`:
  "- **Councils:** in a village without a chief, and at a gathering for a shared rite, hunt or feud, heads of families decide together: each backs the plan they score best, and the plan whose backers the others respect most, in sum, wins.
  - **Chiefs:** in a large village (`CUL-28`), a leader of 10 years becomes chief for life, alone setting group plans, ruling on feuds (`CUL-31`) and punishing (`CUL-06`)."
`CUL-28` adds, once: "From about 60 people (tuned) it is a large village."
`CUL-26`'s priests cite it too (#3).
Villages first appear at 100–300 (`TIM-19`) with about 40 people. With about 0.8% growth a year plus kin moving in (`BIO-04`, `CUL-30`), a village passes 60 within a few decades, so the 150–350 window still holds.
Cross-section: none.

### 3. [major] CUL-32, CUL-26 Specialists and priests have no rule
Problem: `CUL-07` makes "specialist" a role people know and act on, and `MIL-07` delivers specialists.
But `CUL-32` says only that someone "with much experience ... is sought out" and that "full-time work needs stores".
It gives no threshold, and no rule for when someone holds the role or what the role changes.
Priests have three gaps of their own:
- They appear "in a village that can feed a full-time specialist", a measure nobody defines.
- "Living by the role" doesn't say how a priest is fed.
- "The one they taught" has nothing to teach, since rites are not blueprints (`MND-13`).

Fix: `CUL-32`'s How it works becomes:
  "whoever spent most working days of the last year on one sector's work for others, paid with gifts (`MND-33`), is its specialist: those who want such work ask them first, and they teach for gifts (`MND-13`)."
`CUL-26` step 6 becomes:
  "**Priests:** in a large village (`CUL-28`), the shaman becomes a priest, fed by the gifts rites and healing earn as help (`MND-26`), holding rites on the calendar (`CUL-13`) in a house set aside at the sacred place; the successor is whoever joined most of their rites, most often their child."
Cross-section: none (`BIO-23`'s "people known for healing are sought out (`CUL-32`)" stands).

### 4. [major] CUL-30 Splitting has no repeat, no floor and no measure of a quarrel
Problem:
- A band past 40 splits only if families "choose to leave (`MND-09`)". If none does, nothing says when they are asked again, so bands can grow past 40 for ever. That breaks `RES-14`'s band sizes and the 10–50 window.
- "Sooner when food runs short" has no lower size. A band of 16 in a lean season can split into two bands of 8, each below the joining size of 10, which must rejoin kin at once: a loop.
- "A bitter quarrel" is not defined.
- Couples from two bands live where "their custom names", but a custom needs 3 cases first (`CUL-06`). So the first marriages between bands (`MOM-11`, `CUL-27`'s Done when) have nowhere to live.

Fix: In `CUL-30`:
- The Families line ends: "..., and couples to the one their custom names (`CUL-06`), or until there is one, the one holding more of their kin."
- The Splitting line becomes:
  "- **Splitting:** a band that moves splits past about 40 people, a village past about 300, and either past about 20 after a season most adults spent below condition 30 (`BIO-09`), a fight between heads of families or a failed challenge (`CUL-22`).
    The families with the lowest opinion of the leader (`MND-24`), up to about half the band and leaving at least about 10, are offered leaving (`MND-09`), again each season while the band stays past its size, with close kin who follow, ..." (the rest as now).
- In the Done when, "after a quarrel" becomes "after a fight between heads of families".

Cross-section: none.

### 5. [major] CUL-06, MND-24 Custom answers can't be worked out from the rule
Problem: The rule is "the way more than half its cases in the last 5 years went". It fails four ways:
- It can never give the mixed answers the list offers ("either" for where couples live, "anyone" for each sector's work), since each case goes one way. With real strength differences (`BIO-17`), nearly every band's work answer becomes "men" or "women".
- Rare questions never reach 3 cases in 5 years. A band sees a leader die once every decade or two, so "the last leader's child" (`CUL-22`) can never become a custom. Captives, strangers and divorces are hardly better.
- For "who may not marry" and "whether a marriage can end", nothing says what a case is: no cousin marriage in 5 years doesn't show that cousins are forbidden.
- "What earns most respect" offers courage and birth, but `MND-24`'s respect doesn't follow them (it follows skill, success, age and generosity).

Fix: In `CUL-06`:
  "- **A band's answer,** once it has had 3 cases, is the way at least two thirds of its cases went, counting the last 5 years or, where fewer than 5 came in them, its last 5 (tuned); with no way at two thirds, it is the mixed answer (either, anyone) where there is one, else none.
    Who may not marry forbids each kind of match none of those marriages made, and a marriage can end once one has."
The respect answers become "(skill, generosity, success, age)".
In `MND-24`, "as their culture values them (`CUL-22`)" cites `CUL-06` instead.
Cross-section: `MND-24` (G6).

### 6. [major] CUL-06, CUL-08 Punishments and dark acts have no conditions
Problem: Punishments run "from mild to harsh" with no rule for which punishment follows which breach, and "under some chiefs death" doesn't say which chiefs.
`MND-09` offers the dark acts "only in their stated conditions", but `CUL-08` gives no condition that can be built:
- "parents who believe they cannot feed it";
- "when starving";
- "when fear and belief run high", with no rule for whose life is offered.

These conditions set how dark each world's history is (`PRE-18`, `RSK-17`).
Two builders would make very different worlds, and `RES-19` can't test a promise that has no condition.
Fix: In `CUL-06`:
  "- **Punishments,** by whoever the custom names, by the breach: scorn for a broken custom; left out of sharing for theft or a broken taboo; gifts to the wronged or a beating for a wound; driven out (`CUL-22`) for a killing in the band or a third breach in a year; and a chief may punish a killing with death (`CUL-08`)."
In `CUL-08`, the three options become:
  "leaving a newborn, by a mother below condition 30 (`BIO-09`) still nursing a child under 2; eating the dead, below condition 15 with no other food known within a day's walk; a captive's life as an offering (`CUL-31`), only where most adults hold an angry spirit at 70 or more after a second disaster put down to it within a year (`CUL-05`)."
Cross-section: none (`MND-09` already cites `CUL-08`'s conditions).

### 7. [major] CUL-29, CUL-23, CUL-28, CUL-12 Gatherings, links, contact and year-round living have no measure
Problem:
- Five rules count gatherings: festivals "at the same place and season 3 years running", links between bands, alliances, councils and trade. Nothing says what counts as a gathering, what "the same place" is, or how "shared a camp" differs from a gathering.
- `CUL-12` shifts style "toward a people in contact", but contact is undefined.
- `CUL-29` brings bands to gatherings by moving camp, while a village needs a band to have "lived in all year round" for 5 years (`CUL-28`). So a settled band that keeps its yearly festival, as Keeps going expects, may never become a village.
- A people's relation is "its adults' average", without saying whether adults who don't know the other people count. That decides whether any alliance (+40) can form.

Fix:
- `CUL-29`:
  "- **Gatherings:** two or more bands camped within about 2 km of each other for a day or more, a settled band coming as a trip of those who join (`TIM-17`).
    A leader's choice of camp (`CUL-22`) weighs ..." (as now, ending "... where they last met them then").
- In `CUL-29`'s Festivals line: "at the same place (within about 5 km) and season 3 years running".
- `CUL-23`: "linked if they camped together (`CUL-29`) or shared a marriage in the last 25 years"; and "a people's relation to another is the average opinion of its adults who know of it".
- `CUL-12`: "toward a people its bands are linked with, if any (`CUL-23`)".
- `CUL-28`: "a place where one band has kept its camp all year round, whatever trips its members make, for 5 years in a row".

Cross-section: glossary (G9). Add "**Gathering:** bands camped close together for a day or more (`CUL-29`)". In "Festival", "held each year as a custom" becomes "kept each year at its sign", since festivals are not one of the 12 customs.

### 8. [major] CUL-12, CUL-25, CUL-05, CUL-18 Four Done when lines fail by their own rules
Problem: Each of these tests fails even when the rules work as written:
- `CUL-12`: "any two peoples differ in at least 2 of the 6 choices" fails for every people younger than about 50 years, since a new people starts with its parent's style and shifts a step every 25 years.
- `CUL-25`: "every ... song ... links to the saved events it shows" fails after 50 years. `PLT-10` thins old events except those the book, the views and art use, and songs are none of these.
- `CUL-05`: "any spirit the band shares is of the sky" fails whenever, in `MOM-03`'s 30-year scene, a hunt or a death gives the band another spirit, as its own rules expect.
- `CUL-18`: "every ... place ... has a name" fails, since a place gets a name only when someone first talks of it.

Fix:
- `CUL-12`: "any two peoples apart for 75 years or more differ ...".
- `CUL-25`: "every work and myth names its maker and links to the saved events it shows (`PRN-15`), and every song its maker and topic" (unless `CUL-25` is dropped, Cuts 2).
- `CUL-05`: "the spirit the band comes to share from the strike is of the sky".
- `CUL-18`: "every person, spirit, discovery and place talked of has a name".

Cross-section: none.

### 9. [major] CUL-07 The template check is too big and unclear
Problem: The Check gives every template a scene, "in at least 5 of 20 runs after its triggering events and in none without them".
Templates here are the 7 belief templates, 12 custom questions, 7 roles and 12 story shapes: 38 scenes of 20 runs each, or 76 if "none without them" means a control scene for each.
Many run for decades:
- a myth needs 10 years after its story;
- a chief needs a village plus 10 years;
- "how the people began" needs a new people, at 75 years or more.

All of this would sit beside the moment scenes and the pace tests in one cloud session a night (`RES-07`).
Customs and roles have no single "triggering event" to take away, and they already have their own Done when lines (`CUL-06`, `CUL-22`, `CUL-26`, `CUL-32`).
Fix: The Check becomes:
  "a test finds no template tied to a date, an era or a named people, place or person; each belief template and story shape has a scene where it appears in at least 5 of 20 runs after its event, and no scene shows one without its own; customs and roles are read from the pace-test worlds (`RES-07`), which end with peoples of different spirits, customs and kinds of leader."
Cross-section: none.

### 10. [major] CUL-34 "Held in the nearest form" has no mapping
Problem: A rite is the credited act "held in the nearest form", but most credited acts match none of the six forms: burying bones before a hunt (`GOD-06`), eating a root, a walk past a tree.
"Nearest" is not defined, while `PRE-44` needs a closed set of movements for rites.
Fix: "held in the matching form (eating as a shared meal, anything put in a fire as burning a gift, apply as painting, any other act as leaving a gift)".
Cross-section: none (`GOD-06`'s buried bones become a gift left, as its example already implies).

### 11. [minor] CUL-03, MAT-23 Tallies need a blueprint the catalogue may leave out
Problem: `CUL-03` has notched tallies as things people make (`MAT-04`), and the brief keeps tallies in the arc.
`MAT-23` promises "Results other items name are all in the launch list", but its Left out list includes engraving, which a notched tally is.
Builders can't tell whether tallies exist, and `CUL-03`'s Done when ("a band's tallies ... stay where it left them") passes only if they do.
Fix: `MAT-23` (G4) names a notched tally (cut, art) among its 8 art blueprints, separate from the engraving it leaves out.
Cross-section: `MAT-23` (G4).

### Cuts
All measured on a draft of `s10-culture.md` with the fixes above applied.
Together the cuts remove 3.0 KB, so the section ends at about 32.0 KB instead of 33.8 KB, with every rule kept.

1. **Drop `CUL-15`** (0.42 KB). Its line becomes "**Dropped because:** forebears are kin as believed (`MND-24`) and ancestors (`CUL-19`), and legends are in `CUL-11`."
   Its "gaps and errors" had no rule, and remembering 3 generations follows from who people knew.
   `CUL-11` loses "(`CUL-15`)", and `PRE-10` (G8) cites `CUL-11` instead.
2. **Drop `CUL-25`** (0.21 KB; this makes #8's `CUL-25` fix unnecessary). Its line becomes "**Dropped because:** works are things (`MAT-10`) and songs and myths memories (`MND-18`), each keeping its maker and subject (`CUL-09`, `PRE-15`)."
   `PRN-10`'s check and the Done when lines of `CUL-09` and `CUL-11` already cover it.
   `PRE-15` (G8) cites `CUL-09` instead.
3. **Shorten `CUL-07`** (0.27 KB):
   - The Catalogues line becomes "this section's lists are catalogues (`MAT-13`), checked like the others (`MAT-17`) but free to depend on beliefs".
   - The list of roles moves into "In the world, or for you".
   - "None is set off by a script, a date or an era" joins the summary line: "... decide which happen, never a script, a date or an era".
4. **Cut lines that restate other owners' rules** (rule C1, 0.83 KB):
   - `CUL-01`: its first sentence (`MND-13`, `MND-21`).
   - `CUL-20`: the starving clause (`MND-05`).
   - `CUL-21`: "since meat rots ...", "gifts and shared food leave a debt" and "Besides what each person owns" (`MAT-19`, `MND-26`).
   - `CUL-24`: "each held as firmly as the speaker is trusted" (`MND-27`); its For you line becomes "shown as bubbles (`PRE-45`)".
   - `CUL-27`: the work line (`CUL-06`, `BIO-17`).
   - `CUL-05`: the Trigger line, folded into "each filled with the real event behind a link or unseen being (`MND-05`)".
   - `CUL-11`: the Images example, which stays in `PRE-17`, `PRE-19` and `PRN-06`; the line becomes "come only from the people's beliefs and the story's shape (`PRE-17`)". Its Kept line becomes "by telling and lost with the last who remember (`CUL-02`)".
   - `CUL-10`: "hummed or chanted on the language's sounds" (`SND-02`).
   - `CUL-08`: its second line becomes "Sexual violence is not part of the game (`BIO-15`), and what is shown follows `PRE-18`."
5. **Cut descriptions of outcomes that are not rules** (0.90 KB):
   - `CUL-16`: How it works becomes one line: "at gatherings (`CUL-29`), with spouses (`CUL-27`), trade (`CUL-21`) and captives (`CUL-31`), or by copying things found (`MND-11`), so distance, mountains and seas keep groups apart".
   - `CUL-28`: "crowd illnesses return every 10–20 years" (also wrong: `BIO-05` needs about 200 people with herds), "firewood and game grow scarce", and its ownership clause (`CUL-21` keeps it).
   - `CUL-31`: "raids are rare between kin, common between hostile peoples, and drawn by villages" (`CUL-28` keeps "stores draw raiders").
   - `CUL-29`: "joy and belonging run high".
   - `CUL-17`: "so split peoples still understand each other but differ in newer words".
   - `CUL-19`: "asking the dead for help" and the fear of the wronged dead, which have no rule; `CUL-05`'s angry spirits cover them.
   - `CUL-06`: "taught to children and kept after its reason is forgotten".
   - `CUL-09`: "art at a sacred place strengthens its beliefs", which has no number.
   - `CUL-10`: the lullaby and the lament.
   - `CUL-26`: its list of what a religion holds.
   - `CUL-18`: second names for "a striking deed", which is undefined.
6. **Shorten `CUL-03` and `CUL-13`** (0.10 KB). `CUL-13` also becomes buildable: "a yearly sign seen two years running (`MND-28`), such as first frost, herds passing, nuts falling or the full moon (`WLD-07`), is named when first talked of (`CUL-18`), such as "when the salmon come", and plans and festivals keep to it (`MND-22`, `CUL-29`)."
7. **Shorten `CUL-14`'s drop line** (0.05 KB): "places pass by talk (`CUL-24`), and a drawn map is new art for no new effect".
8. **Replace the 27 "(tuned)" markers** with one line in the section intro, as section 9 does: "Numbers here are starting values, tuned in tests (`PRN-17`)." (0.15 KB)
9. **Drop "game" before years and days**: "game years" and "game days" become "years" and "days", since `TIM-14` already makes every time in the file game time (0.07 KB here; about 0.4 KB across the file, with `TIM-18` and the glossary keeping the term).

Optional, if the file still misses the cap:
- Fold `CUL-13` into `CUL-29`'s festival line, about 0.3 KB more; `TIM-14`, `WLD-07`, `CUL-26` and `CUL-34` would cite `CUL-29` instead.
- Tighten the 30 Done when lines, about 0.3 KB.

Content cuts (no KB saved in the file):
- Motifs go from about 100 to about 60: one for each wild and domestic animal kind, plus a few for people, sky and weather, and things.
- Dance moves go from about 20 to about 12.
- `RSK-25` and `PRE-44` follow.

Other sections affected by these cuts:
- `MAT-12` (G4) cites `MND-21` for children copying adults.
- `BIO-17` (G5) cites `CUL-06` for who does what work.
- `MND-33` (G6) drops "a topic or two every few minutes" and cites `CUL-24` (0.03 KB).
