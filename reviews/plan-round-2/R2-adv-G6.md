# R2 adversary G6: section 9, Minds

**Round 1 check.** In s09 and the items it leans on, D4, D11, D15, D16, D20, D31 and D33 landed as decided.
D12 landed, but its caps limit what a person takes in, not what they must look over first (finding 3).
D13 landed, but the one belief rule leaves its units open and has no trigger for offerings or medicine (findings 2 and 6).
D14 landed with rules still stated twice, four of them differently (finding 8).
D7's milestone text contradicts what minds need at each stage (finding 1).

**Cost check.** With the caps, a person in a camp of 30 costs roughly 0.3–0.6 ms a game day (rough estimate: choosing about a third, walking and noticing most of the rest), inside `MND-15`'s 1 ms.
A villager does not fit until finding 3 is applied, and the 1 ms itself only meets `TIM-07` if the work is spread over about four cores, which no budget item states.

### 1. [blocker] SCP-16, MND-04, MND-05, MND-22, MND-33 The milestone split forces stand-ins that later stages throw away
Problem:
- **Pairing:** `MIL-01` builds "pairing", but pairing exists only as courtship (`MND-33`, at `MIL-05`) and marriage (`CUL-27`, at `MIL-06`), while the nightly pace runs to Year 60 from `MIL-02` (`RES-07`, on the first region, `RES-21`) need new couples within the first years.
  So `MIL-01` needs a pairing rule that `MIL-05` throws away, against `PRN-09` ("foundations that later ones extend without starting over").
- **Poison and medicine:** `MND-04` learns them "only by linking what follows (`MND-05`)", which comes at `MIL-05`.
  From `MIL-02` (items and characteristics) to `MIL-04`, a poisonous berry that looks like a safe one is eaten again and again, and `MIL-04`'s yarrow poultice and willow-bark drink (`BIO-23`) can never be chosen: their inputs ask for medicine, a hidden value (`MAT-03`) nobody can learn yet, and choices use only what is known (`MND-02`).
- **Talk:** talk alongside work (`MND-33`, `CUL-24`) comes with "social acts and conversations" at `MIL-05`.
  But the murmur comes at `MIL-03` and must come "from people really talking" (`PRN-10`, `SND-01`, `SND-03`), `MIL-04`'s group plans are told to the others (`CUL-22`), and opinions, and so friends, teaching's motive and pairing, move only by acts such as sharing food and talk (`MND-24`).
  So `MIL-03` either fakes the murmur, against `PRN-10`, or builds a stand-in talk.
- **Plans:** `MIL-04`'s group hunts need a plan with a time (finding 7), but plans (`MND-22`) come at `MIL-05`.
Fix: `SCP-16` (G1), so each rule is final from the stage it first appears and later stages only add to it:
- `MIL-01`: "births, pairing, growing up" becomes "births to the start's couples, growing up".
- `MIL-02`: after "kin and friends, and who knows what" add "; pairing, sharing food and talk alongside work, with news, places and how things are made (`MND-33`, `CUL-24`); beliefs about causes, so foods, poisons and cures are learned (`MND-05`)".
- `MIL-04`: "leaders and group plans, such as hunting together (`CUL-22`)" becomes "leaders, and group plans with their times, such as hunting together (`CUL-22`, `MND-22`)".
- `MIL-05`: "beliefs about causes and the unseen, plans and ambitions, social acts and conversations" becomes "beliefs about the unseen, ambitions, and the other social acts: gifts, trade, gossip, comfort, quarrels, fights and stealing".
Cross-section: G1 `SCP-16`; G7 `CUL-24` (its topics arrive in two steps: news, places and how things are made at `MIL-02`, the rest at `MIL-05`); G8 `SND-05` unchanged (talk exists before the murmur).

### 2. [major] MND-05 The one belief rule leaves its units open, routine events flood it, and true causes of common outcomes can't get in
Problem:
- **Causes have no unit:** "the most unusual thing ... done or met" could be an act, the act with the thing used, a kind of thing, a place, a person, an animal or the weather.
  The rule counts each by "days in ten over the past year" and tests a link "when the cause recurs", so two builders will form and test different beliefs.
- **Outcomes have no kinds,** yet "at most 3 links per kind of outcome" and "the outcome within a day or two" need them.
- **Flooding:** every need below 20 brings "a strong bad thought" (`MND-07`); if that is 8 or more, every hungry winter links hunger daily to whatever was unusual, and since hunger then follows nearly every day, the link reaches certainty: "meeting the Ketu brings hunger".
  Every surprise (`MND-10`), which the curious meet daily, also links to the most unusual thing before it ("crossing the ford brings new plants"); only the at-a-place rule and hunches need surprises, and a surprise has no thought, so its link has no starting strength.
- **Saturation:** a hit adds 15 and a miss takes 5, so any cause followed by its outcome more than one time in four grows to certain, true or not.
  For a common good outcome, such as a good hunt in a band that hunts every few days, every unusual act before one becomes a certain rite; each person's 3 slots for it fill for good, and a true cause found later (a better spear) starts weaker than all three and is pushed out at once.
- **No relief:** the end of a bad time is not an outcome, yet `CUL-05`'s offering template ("when a bad time ends after something was given"), `MOM-03` step 2 ("when the storms stop after one, the gift is credited (`MND-05`)") and `BIO-23` (healing rites credited with recoveries) rest on it.
  Medicine can't be learned either: it tastes bitter (`MND-21`), and its effect is pain or illness easing.
- **Scurvy:** `BIO-10` calls fresh greens "a true cause people can learn (`MND-05`)", but greens are eaten on most days of a year, so they are never unusual enough to link.
Fix: `MND-05`, replace the **Strong outcome** line with:
  "- **Outcomes:** what befalls them, or kin and friends they see: a hunt or a find, a hurt, an illness, a birth, a death, a storm, flood or fire, or the end of days of hunger, cold, pain, illness or storms, as big as its worst thought.
    It is strong at a thought of 8 or more either way (`MND-29`); a need falling or their own act is not an outcome, and a surprise links only at its place (below).
  - **Causes** are kinds done or met: an activity with the kind of thing used (ate hazelnuts, sang), a place, a person, someone sick, or a kind of animal or weather, each counted by the days of the past year it came up on."
- **Explained or not:** "if any was done or met on fewer than about one day in ten over the past year, a first time most" becomes "if one came up on fewer than about one day in ten over the past year, or not in the ten days before, a first time most".
- **Strength:** "the outcome within a day or two adds 15, its absence takes 5" becomes "the outcome within a day or two adds 15 and its absence takes 5, or 10 each for an outcome that comes on more than one day in ten"; add "a link from a surprise starts at 20".
- **Room:** "the weakest pushed out" becomes "the weakest, new or old, dropped".
- **Done when** becomes: "in 20 band-scene runs, most adults shun a real poison within a season in 16 and a food eaten before a chance fever in 6, credit better spears for better hunts in 12, and never starve beside taboo food."
Cross-section: `CUL-05` (G7) keeps "by the rule of `MND-05`", which now covers offerings; `MOM-03`, `MOM-04` (G1) and `BIO-10`, `BIO-23` (G5) fit unchanged; `RSK-19` (G9) may use the new Done when as a sign.

### 3. [major] MND-03, MND-09, MND-14, MND-15 A villager still costs several campers, and the budget hides a core count
Problem:
- `MND-03` caps what is taken in (about four new things an hour) but not what is looked over to pick them, "the closest, newest and most surprising first", from everything in sight.
  In a village of 300 (`CUL-28`) that is every person's activity, every animal and fire, about ten times a camp of 30's, every hour, for every villager.
- `MND-09`'s "in reach: carried, in camp or within about 30 m": in a village, "in camp" means every thing in every house and store, searched for up to eight blueprints at each of 10–30 choices a day.
- So `MND-15`'s own Check (a villager costs at most about twice a camper) fails exactly when villages come (Years 100–300), with no rule that meets it.
- `MND-14`'s "a few hundred places" is spent at the start: the home range a start adult knows (`BIO-20`, about 10 km around) is about 300 world cells, each a place (`MND-28`).
- `MND-15`'s budget, about 1 ms of one middle core per person per game day, makes 1,000 people a full core-second per game day: all of `TIM-07`'s floor of 1 game year a minute, before the world's own layers (up to a quarter of the time, `PLT-04`) and making areas (a tenth).
  It fits only if the work is shared over about four cores, as `PRC-10`'s repeat check assumes and no budget item states.
Fix:
- `MND-03` **The rest:** "at most about four new things an hour, the closest, newest and most surprising first" becomes "each hour they look over only the nearest few dozen people, animals and things in sight, and take in at most about four new ones, the newest and most surprising first".
- `MND-09` **In reach:** "carried, in camp or within about 30 m" becomes "carried, theirs or their family's, in their band's shared stores (`CUL-21`), or within about 30 m".
- `MND-14`: "a few hundred places (`MND-28`)" becomes "about 500 places (`MND-28`)".
- `MND-15` **Budget,** add: "So 1,000 people take about one core-second per game day, and `TIM-07` needs the work shared over about four cores (`PRC-10`, `PLT-04`)."
Cross-section: `PLT-04` (G9) states the core count beside its shares, and adds a camp of 30 to its benchmark worlds, which `MND-15`'s Check compares with the village of 300.

### 4. [major] MND-24, MND-33, MND-19, MND-07 The numbers that drive social life are missing, births included
Problem:
- **Pairing:** `MND-33` pairs two people "if love grows both ways", but love has no number: `MND-19` says it is "held for particular people (`MND-24`)", and `MND-24` holds opinion, trust and respect, not love.
  Pairing sets births, so `BIO-04`'s growth (about 0.8% a year, 1,000–3,000 people at Year 400) rests on a rule two builders will code differently.
- **Opinion, trust and respect:** `MND-24` names what moves opinion but no amounts, and nothing moves trust or respect except a lie found out (`MND-23`).
  Yet leaders (`CUL-22`: the highest sum of trust and respect, a challenge at a fifth more), splits (`CUL-30`: the lowest opinion of the leader), marriages (`CUL-27`: a parent below −20) and captives (`CUL-31`: below −20) all compare these numbers with fixed thresholds.
- **Respect:** `MND-24` says it follows "skill, success, age and generosity", `CUL-06` lists six sources (adding courage and birth), and `CUL-22` says "as the custom says".
- **Mind needs** (safety, belonging, status, curiosity, love) have no rates, so how often people seek company or something new, the texture of a camp, is a guess.
Fix:
- `MND-24` **Opinion** becomes: "**Opinion,** starting values: a chat +1, shared food or help +3, a gift +5, help in danger +10; an insult −5, theft or a broken promise −15, a blow −20, killing kin −80; apart, a point a week back toward 0; together, a point a week up between like personalities and down toward a hot temper."
- `MND-24` **Trust** line: after "whether they are followed" add "; it rises 2 each time their word proves true and falls 10 when it proves false".
  "**respect** follows skill, success, age and generosity, as their culture values them (`CUL-22`)" becomes "**respect** comes from the six sources of `CUL-06`, as seen and as their custom weighs them".
- `MND-24` **Bonds:** cut "courting adults in love partners (`CUL-27`),".
- `MND-33` **Courtship** becomes: "unpaired adults court the one they like most above about +40, with time, gifts and help; when each one's opinion of the other has stayed above about +60 for about 10 days, they pair, as their customs allow (`CUL-27`, `BIO-15`)."
- `MND-19` **Toward people:** "love and grief are held for particular people (`MND-24`), grief for each one lost" becomes "love is an opinion above about +60 that has lasted (`MND-24`), and grief is for each one lost, as deep as that love".
- `MND-07`, add: "- **Rates:** belonging, curiosity and love fall about 10 a day unmet and rise about 10 with a shared task, something new or a day with those loved; safety drops at once with danger and recovers within hours; status follows respect (`MND-24`)."
Cross-section: `CUL-22`, `CUL-27`, `CUL-30`, `CUL-31` (G7) now compare stated amounts; `CUL-06` (G7) owns respect's sources; `BIO-15` (G5) unchanged.

### 5. [major] MND-11, MND-10, MND-12, MND-20 Discovery can't be tuned step by step, and two hint routes are missing
Problem:
- **A floored knob:** an unknown blueprint's level is half its sector experience (`MAT-04`, skill 0), which is 0 in sectors nobody has yet (pottery, herding, farming, metal), and `MAT-04`'s 5% floor then gives every blueprint of difficulty 5 or more the same discovery chance.
  Difficulty, the only per-blueprint knob `MND-11` names, can't slow pottery (Years 60–150) without slowing copper (300–500), and moving it also changes how hard the craft is to master.
- **Hints from elsewhere:** `MAT-04` field 9 lets a plain use, a known blueprint or a timer hint a blueprint, and `MAT-17`'s Discoverable check passes blueprints hinted only that way, but `MND-11` gives hunches only from an unknown blueprint's own failed roll.
  The hard lump in an old hearth (pottery), the bead in a blown kiln (`MOM-12`, `MAT-19`) and food plants on the heap (`MOM-08`) have no rule making a hunch, `MND-10`'s "often a hunch" has no number, and `MAT-21` makes a timer's unmeant result a named discovery without saying whose.
- **Knobs with no value:** "the chance a surprise is noticed" and "the dream-hint chance", both named in **Pace**, have none, and dream hints are "real or not" with no share real, so the dream route's rate is a guess.
  Experimenting "aimed at" a need gives no rule for what is tried, though fire's window rests on it (cold, no fire, the curious trying things).
- **Contradiction:** `MND-11` has a curious adult experiment about once a day and an average one once a week, seven times as often, while `MND-20`'s Done when has the most curious experiment "twice as often as the least".
- **Pace** says discovery is tuned "only" by these values, but dogs, herding and villages (`TIM-19`) are not discoveries, and `PRN-17` tunes amounts too.
Fix:
- `MND-11` **Chances:** after "1 in 2 with a hunch for it," add "times the blueprint's own discovery factor (1 unless tuned, `MAT-04`),".
  Its second sentence becomes: "A success, if noticed (`MND-10`), teaches it at skill 1.
  Any outcome a blueprint names as its hint (`MAT-04`), from its own roll, a plain use, another blueprint or a timer, gives whoever notices it a hunch for it; a timer's unmeant result, such as a copper bead, is a named discovery for whoever notices it first (`MAT-21`)."
- `MND-11` **By experimenting:** "then aimed at it" becomes "then aimed at it, trying things whose known characteristics bear on that need (food for hunger, warmth or burn for cold)".
- `MND-11` **Pace** becomes: "**Discovery pace** is tuned by these values, the same in every world: the three factors, each blueprint's discovery factor and difficulty, the chance a surprise is noticed, how often people experiment and the dream-hint chance (`PRN-17`, `RES-16`)."
- `MND-10`: "noticed more by the curious, less by the busy, tired or frightened (`MND-03`)" becomes "noticed about 1 time in 2, from 1 in 4 for the least curious to 3 in 4 for the most, and half as often when busy, tired or frightened (`MND-03`)"; "and often a hunch (`MND-11`)" becomes "and a hunch when it is a hint (`MND-11`)".
- `MND-12` **Hints:** "now and then, more when a need presses" becomes "about 1 dream in 60, 1 in 20 while a need is below 20"; "real or not" becomes "pointing 1 time in 3 to a real blueprint they don't know, chosen as for an idea dream (`GOD-03`), and otherwise to nothing".
- `MND-20` Done when: "twice as often as the least" becomes "at least five times as often as the least".
Cross-section: `MAT-04` (G4) field 7 adds "and, for a blueprint not yet known, a discovery factor, 1 unless tuned (`MND-11`)"; `MAT-19` and `MAT-21` (G4) cite `MND-11` for who is credited; `MOM-08`, `MOM-12` (G1) fit unchanged; `RES-16` (G9) logs discovery factors.

### 6. [major] MND-31, MND-27, MND-14 Religion's seeds: beings have no numbers, the dead crowd out spirits, and told beliefs have no strength
Problem:
- `MND-31` **Growth** gives no amounts for what a being gains per event, dream or story, nor whether it fades, so `MOM-03` (a myth of the one in the storm within 30 game years), `CUL-26` (a shaman once a band shares 2 spirits) and `CUL-33` (a first shared spirit within 5 years) have nothing to tune.
- **Five slots for spirits and the dead together** (`MND-14`, `MND-31`): every dream of a dead parent, child or partner strengthens that ancestor (`MND-12`, `CUL-19`), while a storm spirit gains mostly from storms and stories.
  An adult who has lost several kin fills up with ancestors and the least held spirit is pushed out, so religion can stall at the dead.
  And `CUL-05`'s "a band shares at most about 8 spirits" can hardly be reached: 8 spirits each held by most adults needs every adult to hold at least 4 of them, nearly all of their 5 slots.
- "A new being pushing out the least held" (and `MND-05`'s "the weakest pushed out") doesn't say whether a newcomer weaker than all held gets in, so firm beliefs either churn or never change.
- Spirits spread mostly by talk (`CUL-24`: "held as firmly as the speaker is trusted"), but `MND-27`'s "a trusted elder's word strongly, a stranger's little" gives no strength, so how fast a band comes to share a spirit is a guess.
Fix:
- `MND-31` **Growth** becomes: "a new being starts at about 20; each later event of its kind is put down to it and adds about 10, and each dream of it or rite held for it about 5; it never fades by time alone; an act linked to a good outcome of its kind (`MND-05`) is held to please it (`CUL-05`)."
- `MND-31` **Room** becomes: "within the caps of `MND-14`, the weakest, new or old, is dropped."
- `MND-14`: "5 unseen beings (`MND-31`)" becomes "5 spirits and 5 of their own dead (`MND-31`, `CUL-19`)".
- `MND-27`: "own experience counts most, a trusted elder's word strongly, a stranger's little" becomes "own experience, or a belief heard, which starts at or rises to the teller's strength times the hearer's trust in them out of 100 (`MND-24`), so trust 50 halves it".
Cross-section: `CUL-05` (G7): "at most about 8 spirits" becomes "at most about 5", in its How it works and its Done when; `CUL-19`, `CUL-24` (G7) keep their citations.

### 7. [major] MND-22, CUL-22, TIM-17 Plans have no times, so group plans can't gather anyone
Problem: `TIM-17` ends an activity at "a plan's time (`MND-22`)", but `MND-22`'s plans have steps and no times.
`CUL-22`'s group plans (hunt, move camp, rite, raid) run as one shared activity that "fails without" the fewest who must come, but nothing says when it starts or how long it waits.
One person starts it, and the others, busy with their own activities, see it only at their next choice, spread over an hour or more (`TIM-17`).
Group hunting is a `MIL-04` goal, and rites, raids and festivals (`CUL-34`, `CUL-31`, `CUL-29`) need the same.
Fix: `MND-22`, add: "- **Times:** a step can have a time and place, such as dawn at the ford or a group plan's start (`CUL-22`); when it is time to go, it interrupts what they do (`TIM-17`), and one missed by over an hour fails."
`CUL-22` (G7), **Group plans:** "sets one and tells the others (`CUL-24`)" becomes "sets one, with a time and place to meet, most often the next dawn, and tells the others (`CUL-24`)"; "and fails without them" becomes "and starts at its time with those who came, if enough did, or fails".
Cross-section: G7 `CUL-22`; `TIM-17` (G2) unchanged, its "a plan's time" now defined; `SCP-16` per finding 1.

### 8. [major] MND-04, MND-05, MND-09, MND-12, MND-13, MND-24, MND-31, MND-33 and Culture Rules still stated twice, four of them differently
Problem: D14 gave each rule one owner, but these remain doubled (marked * where the two say different things):
- talk's pace: `MND-33` ("a topic or two every few minutes") and `CUL-24` ("1–2 topics in a few minutes"); owner `CUL-24`.
- what pulls members to join a group plan: `MND-09` and `CUL-22`; owner `CUL-22`.
- band sharing ("most of a band's adults"): `MND-05` and `CUL-05`; owner `CUL-05`.
- later events going to the same being: `MND-31` and `CUL-05`; owner `MND-31`.
- dreams of the dead feeding belief: `MND-12`, `MND-31`, `CUL-05` and `CUL-19`; owner `CUL-19`.
  * `CUL-19` also sets what is dreamt ("respected elders and leaders are dreamt of most") beside `MND-12`'s strongest memories.
- * whom people learn from: `CUL-01` ("kin, the trusted and respected, and the most skilled") against `MND-13`'s teachers (kin, friends, the kind, a gift); D14 gave learning between two people to `MND-13`.
- told things weighed by trust: `MND-04`, `MND-13`, `MND-27` and `CUL-24`; owner `MND-27` (finding 6).
- * respect's sources: `MND-24` (four) against `CUL-06` (six); owner `CUL-06` (finding 4).
- * `MND-09`'s "shared habits become customs" against `CUL-06`'s closed list of 12 custom questions.
- breaking a food taboo when starving: `MND-05` and `CUL-20`; owner `MND-05`.
- pairing: `MND-24` ("courting adults in love partners") and `MND-33`; owner `MND-33` (finding 4), marriage `CUL-27`.
- one scene tested two or three times: the hunting song (`MND-05`'s Done when, `CUL-34`'s, `MOM-04`'s Check), and sickness after a food (`MND-05`'s and `CUL-20`'s Done when).
- animal calls, word for word: `MND-16` and `SND-01`; owner `MND-16`.
- `RSK-27`'s "reasons kept for every choice" against `MND-09` (kept for the current activity and every saved event).
- the glossary's "Belief template: a shape for what people can't explain" against `CUL-05`, whose taboo, rite and offering templates shape explained links.
Fix, in s09 (G6):
- `MND-33`: "a topic or two every few minutes from the list of `CUL-24`" becomes "at `CUL-24`'s pace"; cut **Shown** (`PRE-44` says it).
- `MND-09`: cut ", weighing what it does for them, trust in who asks and what most do" and "; shared habits become customs (`CUL-06`)".
- `MND-05`: cut **Shared**; its Done when loses the song (finding 2 rewrites it).
- `MND-12`: cut " and belief in them (`CUL-19`)"; `MND-31`: cut "; dreams of the dead feed belief in ancestors (`CUL-19`)".
- `MND-13` **Being told:** cut "; facts, places and beliefs pass in talk (`CUL-24`), weighed by trust (`MND-24`)"; **Watching,** add "the most skilled, the respected and kin are watched most (`MND-24`)".
- `MND-04`: "(weighed by trust, `MND-24`)" becomes "(`MND-27`)".
Fix, elsewhere:
- `CUL-01` (G7): cut "people learn most from kin, the trusted and respected, and the most skilled (`MND-24`);".
- `CUL-05` (G7): cut "; an event like one already put down to a spirit goes to that spirit (`MND-31`)".
- `CUL-19` (G7): cut "; respected elders and leaders are dreamt of most".
- `CUL-20` (G7): cut "; the starving break a food taboo, with shame (`MND-05`)"; its Done when tests the band naming and punishing a taboo, not `MND-05`'s scene again.
- `SND-01` (G8): its calls sentence becomes "and animal calls (`MND-16`)".
- `RSK-27` (G9): "reasons kept for every choice" becomes "reasons kept for what each is doing and every saved event (`MND-09`)".
- Glossary (G9): "**Belief template:** the shape a belief born of an event takes: a spirit, the dead, a taboo, a rite or an offering (`CUL-05`)."
Cross-section: G7, G8, G9 as listed.

### Cuts

s09 is 35.8 KB.
The cuts below give about 3.3 KB, 3.7 KB with the optional one; the fixes above add about 2.5 KB in s09, measured (finding 2: 0.65, 3: 0.27, 4: 0.52 of which the needs' rates are 0.25, 5: 0.56, 6: 0.23, 7: 0.21, 8: 0.07).
So s09 lands near 35.0 KB, or 34.6 with the optional cut; finding 8 also cuts about 0.45 KB from s10 and s12.
Minds are the centrepiece and the fixes fill numbers builders need, so most of the 26 KB has to come from other sections; if s09 must give more, finding 4's needs' rates (0.25 KB) can wait for tuning.

- `MND-06` **Example** (0.18 KB): restates `MAT-04`'s chance and `MAT-20`'s quality; the Done when tests the curve.
- `MND-27` first How-it-works sentence, the list of what beliefs cover, and "; each shows in the details view with its source and memories (`PRE-14`)" (0.24 KB): an index of other items, and `PRE-14` already shows beliefs with sources and events.
- `MND-25` merged into `MND-01` (0.21 KB net): `MND-01` adds "; minds hold records with numbers, never sentences, passed in talk as topics (`CUL-24`)" and its Check adds "and no record in any mind holds a sentence"; `MND-25` becomes *Dropped* ("merged into `MND-01`"), and the header (G9) drops it from the rules.
  One rule, two items, overlapping Checks; the writer's pattern sentences are `PRE-37`'s.
- `MND-17` *Dropped* (0.10 KB net): "its point is made by `MND-11` and `CUL-01`"; context only, and the header (G9) drops it from Context.
- `MND-03` **Sight** (0.17 KB): `BIO-18` says it word for word, and the item's first sentence already cites `BIO-18`.
- `MND-28` **Start** (0.12 KB): `BIO-20` states the start map; learning an area on the spot is "from going there".
- `MND-14` Check, cut "the same saved world, run with the camera in different places and at different speeds, gives the same choices (`WLD-13`), and" (0.13 KB): `WLD-13` and `TIM-17` check exactly that.
- `MND-16` **Dreams** (0.12 KB): `GOD-12` holds animal dreams, and `PRN-13` already covers animals' reasons.
- `MND-31` **Your acts** (0.13 KB): `GOD-06` and `CUL-05` say it, and religion's growth is `CUL-26`'s.
- `MND-12` **Your lever** (0.09 KB): `GOD-03` says a sent dream replaces the night's own.
- `MND-21` **a hidden someone** (0.10 KB): `MND-05`'s **The unseen** is the working rule.
- `MND-30`: cut "Afterwards mood lifts a little; witnesses remember it and may comfort or shun them (`MND-24`)." (0.10 KB): memories and social acts already do this, and "at most once a season" stops repeats.
- `MND-16` **Taming** (0.10 KB): `WLD-33` and `RCK-24` hold taming, and "attached to particular people" clashes with `WLD-33`'s one tameness number.
- `MND-11` **Hunches:** cut " from hints, dreams (`MND-12`, `GOD-03`), copying, watching or being told (`MND-13`)"; **The route:** cut "; a people's first success is a named discovery (`MAT-21`)" (0.14 KB): each route line says where its hunch comes from, and `MAT-21` owns named discoveries.
- `MND-33` **Chosen like anything else** becomes "(`MND-09`), each moving opinions (`MND-24`) and needs (`MND-07`)." (0.08 KB): finding 4 gives the amounts.
- `MND-05` **Wrong beliefs last** becomes "**Avoiding costs** (`MND-09`): hunger strong enough eats the forbidden food, with shame, and if nothing bad follows, the link weakens." (0.13 KB): its first sentence only explains what the rule already does, and the item's opening names taboos and rites.
- Finding 8's s09 cuts (0.57 KB): the doubled clauses in `MND-09`, `MND-05`, `MND-33`, `MND-13`, `MND-04`, `MND-12`, `MND-31` and `MND-24`.
- Small repeats (0.54 KB): `MND-09` "; anything else noticed waits for this" (`TIM-17`) and ", shown on the card and in the details view (`PRE-35`, `PRE-14`)"; `MND-19` "; the hot-tempered anger faster and the brave fear less" (`MND-20`); `MND-20` ", and surprises are noticed more (`MND-10`)"; `MND-11` ", so cracking nuts with a flint cobble can knock off a sharp flake" (`MAT-04`, `PRN-01`); `MND-13` ", helped by pointing and shared gaze (`MND-26`)"; `MND-32` ", and the card shows how far along they are (`PRE-35`)"; `MND-10` "; with spare time, they may repeat what came before" (hunches, `MND-11`); `MND-16` "; hunting makes them warier (`WLD-32`)"; `MND-22` "; plans up to the cap of `MND-14`"; `MND-23` "Children manage this from about age four."
- `MND-02` Check (0.09 KB): becomes "every scene's check of `RES-21` passes, and kept reasons name only what the person knows (`MND-09`)."
- Optional, your call (0.38 KB): cut `MND-23`'s **Lies** and **Found out**, keeping secrets and following; `CUL-24`'s topic list loses "a lie (`MND-23`)" (G7).
  Lost: three kinds of lie and the opinion cost of being caught; secrets, the following scene and its Done when stay.
