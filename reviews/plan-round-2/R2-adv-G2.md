# R2 adversary G2: sections 4 (The player as god) and 5 (Time and history)

**Round 1 check.** In sections 4–5, decisions 1, 2, 5, 9, 10, 20–24, 27, 29–31 and 33 landed as written, each stated once and cited elsewhere.
That covers: `TIM-18`'s rule and daily rates; `TIM-19` citing `CUL-28`, `MAT-23`, `WLD-33` and `RES-07`; `TIM-17` citing `WLD-12` and `WLD-13` and holding the one interruption list; `TIM-01`'s speeds at `PRE-03`'s stops; the `TIM-07` targets; the `TIM-02` budget; `TIM-12`'s "how far" and 40 °C pause; `TIM-14`'s Year 1 and other-half seasons; Done when lines; the header lists.
Sizes are s04 12,493 B and s05 18,975 B, and secheck passes.
The squeezed year now holds in bodies, illness, plant growth and yields, animal lives, fire, timers and food (`BIO-04`, `BIO-05`, `BIO-09`, `BIO-10`, `BIO-13`, `BIO-15`, `WLD-30`–`WLD-33`, `MAT-18`–`MAT-20`, `MND-06`, `MND-19`).
These did not land, or landed inconsistently:
- decision 12: "a call" and "a plan's time" are defined nowhere (#3);
- decision 14: shared activities can't do what `CUL-22` and `CUL-34` ask of them (#3);
- decision 22: the targets don't add up against `MND-15` and `PLT-04` (#1);
- decision 11: herds choosing as one break `GOD-12` near people (#6);
- decision 7: all five dream kinds stayed at `MIL-03` (#6);
- decision 24: `CUL-33` still cites a pass rule that `TIM-19` no longer has (#8).

Eight findings follow, then cuts.

### 1. [major] TIM-07, MND-15, PLT-01, PLT-04, MND-33, CUL-24, WLD-12 The speed numbers don't add up: the cores are never stated, the world's share is too loose, and talk isn't counted
Problem:
- `MND-15` gives each person about 1 ms of one middle core per game day.
  So 1,000 people at 1 game year a minute (one game day a real second) fill exactly one core, and so do 2,000 at half: `TIM-07`'s targets for people assume one core.
- `PLT-04` lets the world's own layers take a quarter of the simulation's time at 1,000 people, up to about 0.38 s of a core per game day.
  At that cost, the world alone at 10 game years a minute (`TIM-07`) needs about 3.9 middle cores, and 300 people at 5 need about 3.7: these targets assume four cores.
- Nothing says how many cores the simulation may use.
  Only `PRC-10`'s repeat check ("once on one core and once on four") hints at four, and the picture, sound and writer AI share the same phone.
  Even on four cores, "aiming for 2–4" at 1,000 people tops out at about 2.6 within `MND-15`'s budget.
- Talk is the largest cost left uncounted.
  `MND-33` and `CUL-24` pass "a topic or two every few minutes" during work, walking, eating and rest.
  That is about 100–400 topics a person per waking day, and each one changes the hearer's beliefs, places, memories or opinions and both people's who-knows-what (`MND-23`).
  At a few microseconds each, talk alone takes a third to all of the 1 ms.
  `TIM-07`'s "What counts" names activities, results and notices, and talk alongside work is none of these.
- `PLT-04` allows making areas a tenth of the time at about 5 ms each, which is about 30 areas a game day for 1,000 people, under one a band.
  Yet bands stop at several patches daily, and `WLD-12` doesn't say whether an area someone just used stays made, so each visit could remake it.
- `BIO-04`'s growth of 0.8% a year from 45–120 people gives about 2,400–6,500 people at Year 500, inside copper's window.
  But `TIM-07` stops at 2,000, while `PLT-04` measures a 3,000-person world against "the targets of `TIM-07`".
  The overnight target names no population, so it fails for these worlds.
- `PLT-04`'s stage budgets need a 1,000-person world at `MIL-01`–`MIL-03`, "placed on generated land", which doesn't exist before `MIL-04` (`WLD-34`).

Fix:
- `PLT-01` (G9) adds: "The simulation may use up to the four middle cores at held speed, leaving the rest of the phone for the picture, sound and the writer AI; every time budget in this file is in time of one middle core (`MND-15`)."
- `PLT-04` (G9), Shares: replace "the world's own layers at most a quarter of the simulation's time" with "the world's own layers at most about 0.2 s of one middle core per game day, so the world alone reaches 10 game years a minute on two cores".
  Benchmark worlds add: "before `MIL-04`, on the first region (`WLD-34`)".
  With these numbers, 1,000 people at 1 need about 1.35 cores, and 300 at 5 need about 2.75.
- `WLD-12` (G3), Where areas are made, adds: "an area made stays made while anyone is within about 1 km and for a few days after (tuned), as animals near people do (`WLD-32`)".
- `TIM-07`:
  - "1,000 people: at least 1, aiming for 2–3";
  - "about 2,000 people: at least half";
  - new line: "about 3,000 people: at least a quarter; beyond that, time slows further (`MND-15`)";
  - "overnight, about 8 hours, with up to about 1,000 people: a few hundred game years or more (`TIM-12`)";
  - What counts: "every activity of people and of animals near people, every result inside an activity, every notice and every topic passed in talk".
- `MND-33` (G6) and `CUL-24` (G7): replace "a topic or two every few minutes" with "about one topic every 10–15 minutes of talk alongside (tuned), and at most about 60 a person a game day".
  `CUL-24`'s Done when (news of a spring known within 5 game days) still passes easily.

Cross-section: G3 `WLD-12`; G6 `MND-15`, `MND-33`; G7 `CUL-24`; G9 `PLT-01`, `PLT-04`, `RSK-02`.

### 2. [major] TIM-17, MAT-04, MND-11 Nothing sets how long a work activity lasts, though discovery is rolled once per activity, and activities that can't go on never end
Problem:
- `MND-11` and `MAT-04` roll each unknown blueprint "once per activity" that fits it, so the length of an activity secretly sets the pace.
- `MAT-04` makes a blueprint's work "an activity" (its example: "about half a minute a try").
  `TIM-17`'s Repeats line puts "flake after flake" in one activity, with no limit.
- A builder who makes each nut cracked an activity rolls the flake blueprint hundreds of times in an hour of cracking nuts (the accident route of `RES-03`).
  A builder who makes the hour one activity rolls once.
  That gap changes how fast blueprints with short and long tries are found compared with each other, and what `TIM-07` counts as activities.
- `TIM-17` names the only early ends, but an activity must also stop when it can't go on.
  That happens when its tool breaks (`MAT-20`), its core or inputs run out (`MAT-09`), the cooking fire dies (`MAT-18`), the quarry is lost from sight, the thing being worked is taken, or a search finds what it sought.
  None of these is listed, so each builder will add their own.

Fix: in `TIM-17`, replace the Length and Repeats lines with:
"- **Length:** each activity lasts as long as it would in life (`TIM-18`): a meal some minutes, a walk until it arrives, a night's sleep.
  Work with one blueprint is one activity of repeated tries, each landing as it ends, until the aim the choice was for is met (such as the flakes a task needs) or about an hour has passed (tuned).
  So a person's day holds about 10–30 activities, and each unknown blueprint the work fits is rolled once, at its end (`MND-11`)."
After Interruptions, add:
"- **Can't go on:** an activity also ends, keeping what it reached, when its tool breaks, its inputs or the thing it works run out or are taken, its fire dies, what it seeks is found or lost from sight, or a shared one falls below its fewest."
In `MAT-04` (G4), field 6 becomes "the work, one activity of repeated tries (`TIM-17`), shorter with better tools, then any waiting...".

Cross-section: G4 `MAT-04`; G6 `MND-11`; G9 `RES-24` (trials count tries).

### 3. [major] TIM-17, MND-22, MND-03, CUL-22, CUL-34 Shared activities can't gather people, and nothing defines "a call" or "a plan's time"
Problem:
- `TIM-17` says "one person starts it, and others join by choosing it (`MND-09`)".
  But people choose only when their own activity ends, which may be an hour or a night away.
  So a group hunt or a rite begins with one person, and the rest drift in.
- `CUL-22` says each plan "names the fewest who must come ... and fails without them".
  Nothing says when that is judged, where people meet or how long the others wait.
- `CUL-34` says "all who join start and end together (`TIM-17`)".
  `TIM-17` doesn't say this, and its joining rule contradicts it.
- `TIM-17`'s list of interruptions includes "a call to the doer" and "a plan's time (`MND-22`)".
  No item says what a call is: `MND-03` always notices "talk or a call", but talk alongside work must not interrupt (`MND-33`).
  `MND-22`'s plans have no times.
- So the only two ways to gather people for a hunt, rite, raid, move or festival (`CUL-22`, `CUL-29`, `CUL-30`, `CUL-31`) are undefined.

Fix: in `TIM-17`, replace the first sentence of Shared activities with:
"- **Shared activities:** whoever starts one sets its place and time, now or later that day (`CUL-22`, `CUL-34`), and tells or calls the others; each who joins (`MND-09`) holds it as a plan step for that time (`MND-22`).
  It begins once its fewest are there (two for a talk, `CUL-22` for a group plan), and fails if they aren't there within about an hour of its time (tuned), each then choosing again.
  Latecomers may join, except for a rite or dance, which all start and end together (`CUL-34`)."
After Interruptions, add:
"- **A call** is a shout to someone by name or kin word, to warn them, ask for help or fetch them to a shared activity; chat, telling and asking in passing are talk, not calls (`MND-33`)."
`MND-22` (G6) adds: "A step can be set for a time of day or a sign (`CUL-13`), and that time ends what they are doing (`TIM-17`)."

Cross-section: G6 `MND-03`, `MND-22`; G7 `CUL-22`, `CUL-29`, `CUL-34` (their citations then hold).

### 4. [major] TIM-18, WLD-17, WLD-31, WLD-07, MAT-08, MND-24 The rule stops at the catalogues: slow changes in the land and slow build-ups get no game length
Problem:
- `TIM-18`'s Check covers "each duration in the catalogues", but the land's slow changes are in no catalogue: springs, streams and lakes falling in a dry spell, and plants wilting.
  `WLD-17` says only "in a long dry spell small streams dry up", and that water "runs down them at real speed".
- At the speed of life, a rainless game season (15 days) dries nothing, since real streams take months.
  So droughts may never "bite" (`TIM-02`), whether the world's own (`WLD-22`, as often per game year as per real year) or yours (`GOD-02`, "up to a season").
  Whether `WLD-29`'s check ("a drought year lowers rivers, cover and herds' condition") passes depends on the builder.
- "Daily rates" keeps "work" at its real rate per day.
  Yet where an item gives numbers, what builds up from days of work is squeezed: skill (`MND-06`, level 5 in 2 game years), wear (`MAT-20`, a cloak lasts 2 game years, not 12) and tameness (`RCK-24`, pups tame within a season).
  Where an item gives no numbers, builders guess: how many walks make a trodden path (`MAT-08`), and how fast opinions fade between people apart (`MND-24`).
- The moon waxes and wanes every 15 days (`WLD-07`).
  That is neither real (29.5 days) nor a sixth (5 days), and the rule doesn't name it as an exception.

Fix in `TIM-18`:
- Daily rates: "eating, drinking, tiring, the time work takes, walking, weather and accidents keep their real rate or chance per day."
- Add: "- **Slow build-ups:** what builds up from many days over months or years, such as skill, wear, tameness, trodden paths and opinions drifting, changes about six times as much a day as in life, so it takes as many game years as it takes years (`MND-06`, `MAT-20`, `RCK-24`, `MAT-08`, `MND-24`)."
- Add: "- **The land** follows the rule too: in a dry spell, springs, streams and lakes fall and plants wilt about six times as fast as in life, so a dry season bites like a dry summer (`WLD-17`, `WLD-31`); the one exception is the moon, full once a season (`WLD-07`)."
- Check adds: "and in a scene, a season without rain dries small streams and browns grass as a dry summer would in life".

Cross-section: G3 `WLD-17` (cites `TIM-18` for dry spells), `WLD-29`; G4 `MAT-08`; G6 `MND-24`.

### 5. [major] GOD-02, GOD-04, GOD-05, WLD-16, BIO-13 Some limits don't hold, and some conditions have no data behind them
Problem:
- **Rain** rests a day per weather cell but has no tie to the climate, and `GOD-05`'s Check counts storms, dry spells and cold, but not rain.
  So any cell with "cloud or moist air" can get rain every other day all year, wetter than its wettest year, greening a steppe (against `GOD-05` and `SCP-09`).
  "Moist air" has no measure, so `GOD-11` can't decide when to offer rain.
- **Quakes and eruptions** rest only per fault or volcano.
  Nothing stops you setting off every one the world has in one sitting.
  A great eruption cools the whole world for a year or two (`WLD-15`), so `GOD-05`'s Check fails.
- **Fortune:** `GOD-05` says each power is limited in "how soon it can be used again", but fortune (`GOD-04`) has no rest.
  A person can be blessed for a year, then blessed again, for life.
- **Drought** needs "dry spells that long in that season", but `WLD-16`'s climate record holds warmth, rain, snow, wind and storm days, not dry spells.
- **Lightning** "can split, burn, wound or kill (`BIO-13`)", but `BIO-13` gives a strike no wound, so its chance to kill is a builder's guess.

Fix:
- `GOD-02` Rain: "where its weather cell has cloud (`WLD-16`), rain falls ...".
  Rest:
  - "one rain of yours at a time, none on the same weather cell for a day after, and never more rain in a place's season than its climate's wettest season gives;"
  - "one quake or eruption of yours at a time in the world, none within a game year of your last (tuned), and a fault or volcano you set off then rests as long as after a natural one."
- `GOD-04` Limits: "one fortune per person at a time, none on the same person again until as long has passed as it lasted, and up to three people carrying your fortune at once."
- `GOD-05` Check: "... more storms, rain, dry spells or cold than its climate's worst year."
- `WLD-16` (G3): the climate record adds "and its longest dry spell".
- `BIO-13` (G5): adds lightning to the causes of burns, sized so that about 1 person in 4 struck dies (tuned).

Cross-section: G3 `WLD-16`; G5 `BIO-13`.

### 6. [major] GOD-12, GOD-03, GOD-10, MND-16, SCP-16 Dreams act on things that aren't built yet, or that now work differently
Problem:
- Decision 11 made a herd near people choose as one, led by its lead animal (`MND-16`).
  A member chooses alone only when apart, hurt, cornered, hunting alone, tame or kept.
- `GOD-10` lets you send a dream only to "a resting animal" near people.
  A dream "toward a place" sent to any member but the lead then changes nothing, unless a builder lets one member steer the whole herd.
- `SCP-16` brings "dreams" at `MIL-03`, so all five kinds in `GOD-03` are due there.
  But a dream of a person works through talking, helping, courting and making peace (`MND-33`), which come at `MIL-05`.
  A dream of a fear works through fear tied to its cause (`MND-08`, with feelings at `MIL-05`).
  A dream of an animal needs animals and hunting, which come at `MIL-04`.
  Round 1's G2 #17 was accepted, but `SCP-16` kept "dreams" whole.

Fix:
- `GOD-12` adds: "- **Near people,** a herd follows its lead animal (`MND-16`), so a dream toward a place moves it only when sent to the lead, which the ring marks; calmer or bolder changes only the animal dreamt."
- `GOD-10` Animal dreams: "on a resting animal (for a herd near people, its lead), or a herd far from people at night, ...".
- `SCP-16` (G1):
  - `MIL-03`: "your first powers: lightning, and dreams of a place or an idea";
  - `MIL-04`: "your other weather powers, animal dreams and dreams of an animal";
  - `MIL-05`: "the other dreams, of a person or a fear; fortune".

Cross-section: G1 `SCP-16`; G6 `MND-16`.

### 7. [major] GOD-09 Nothing limits "what followed" an act
Problem: `GOD-09` shows "what followed" each act, "followed on through the history the world keeps".
The What gives examples for lightning, fortune, dreams and herds, but says nothing for a storm, rain, drought, cold snap, flood, quake or eruption.
One builder shows a single line.
Another traces causes through every later event (who starved in the drought's third week, which band moved because of it), a research-grade feature with no limit.
The Done when tests only an idea dream.

Fix: replace `GOD-09`'s How it works with:
"- **How it works:** a page shows only what the world records with its cause, never a guess: for weather and land, the deaths, wounds, fires, floods and fallen shelters in the land it covered while it lasted and a day after (`BIO-14`); for a dream, the dreamer's tries at its subject and any discovery, with its line of teaching (`MND-13`); for fortune, each roll it turned; for an animal dream, where the animal or herd went while it lasted."
Done when adds: "and the page of a cold snap lists exactly the deaths by cold in its stretch while it lasted."

Cross-section: G5 `BIO-14` (each death already records its cause, such as freezing, lightning or drowning).

### 8. [minor, factual] TIM-02, TIM-11, TIM-12, TIM-15, CUL-33 Small contradictions left by round 1
Problem:
- `TIM-02` says moments that "the budget or a higher control (`TIM-15`) keeps from slowing time wait in the list".
  Overnight mode is a higher control, yet `TIM-12` sends the night's moments to the book of ages, "not the list", and `TIM-15` keeps them "for the summary".
- `TIM-11` stops skip at "the next moment past the bar", but `TIM-02`'s single budget allows only one slowdown every 3 minutes.
  So skip pressed within 3 minutes of a slowdown can't stop.
- `TIM-02` slows time "so that what is about to happen would take about half a minute".
  Yet its own example, "valley speed for a flood rising", makes a rise over a day (`GOD-02`: "over the next day or two") pass in about 4 seconds, since the valley view runs a season a minute.
- `CUL-33`'s Check cites "the pass rule of `TIM-19`", which `TIM-19` lost in round 1 (decision 24); the rule now lives in `RES-07`.

Fix:
- `TIM-02` The list: "moments the budget, the dial or the lock keeps from slowing time wait in the list; overnight's go to its summary (`TIM-12`), and skip's stop is outside the budget (`TIM-11`)."
- `TIM-15` item 2: "**overnight mode** (`TIM-12`): top speed;".
- `TIM-02` Slowing down examples: "near real speed for a stalk or two hostile groups meeting, camp speed for a birth or a morning at a fire stick, and between camp and valley speed for a flood rising over a day."
- `CUL-33` (G7): "with the pass rule of `RES-07`".

Cross-section: G7 `CUL-33`.

### Cuts
Each cut removes an example, a Why that restates the What, or a restatement of a rule another ID owns (C1).
Nothing a builder needs is lost: the owner ID keeps the content, and the item keeps its citation.

**s04**
- C1 (0.08 KB): cut the second intro line, "You act only as nature could ..." (`GOD-05` and `PRN-03` say it).
- C2 (0.12 KB): cut `GOD-01`'s Example (`VIS-11` and `MOM-01` tell it).
- C3 (0.20 KB): cut `GOD-06`'s Example (`MOM-03` and `MOM-04` show beliefs that come from your acts and from luck).
- C4 (0.37 KB): fold `GOD-05`'s seven lines ("There is no power ..." plus the six labelled lines) into one:
  "There is no power to collect or spend: each works only where its natural cause is present; you choose where and when, and the world how strong; each power's item limits how many run at once and how soon it can be used again; nothing is instant, weather falls on everyone in reach, and nothing can be undone."
- C5 (0.09 KB): `GOD-04` worked numbers become: "So a blessing never more than doubles a chance and a curse never more than halves one: 10% becomes about 19% or 5.5%."
- C6 (0.09 KB): `GOD-11`'s examples become: "such as "no storm overhead" or "resting: ready in 2 days"" (`GOD-10` already says an awake person's dream waits for the night).
- C7 (0.04 KB): `GOD-08`'s second line becomes: "It feeds only your marked lines and the pages of what came of your acts (`GOD-09`), never a mind or a text (`GOD-06`, `GOD-07`)."
- C8 (0.09 KB): `GOD-12` Limits becomes: "one dream per animal or herd per night, within the three a night, repeated as `GOD-03` says" (the wolf line restates "bolder").
- C9 (0.04 KB): `GOD-10` Confirm becomes: "time pauses while you choose (`TIM-15`), and cancelling leaves everything as it was."
- C10 (0.11 KB): the third line of `GOD-03`'s idea dream becomes: "They wake with a hunch that lasts and is tried as any hunch is (`MND-11`)."
- C11 (0.04 KB): `GOD-03`'s "replaces" line becomes: "your dream replaces that night's own and any hint it might have brought (`MND-12`);".

**s05**
- C12 (0.15 KB): cut the second intro line, "After the camera ..." (`PRN-11` and `TIM-01` say it).
- C13 (0.12 KB): cut `TIM-17`'s "One world clock" line (`TIM-01` says the speed is the clock's pace).
- C14 (0.08 KB): merge `TIM-17`'s second and third What lines into: "Its results land when it ends, and anything can be interrupted, by the same rules for everyone, wherever they are and at every zoom and speed, so looking changes nothing (`WLD-13`)."
- C15 (0.23 KB): `TIM-17` "Work and waiting" becomes: "**Waiting:** a blueprint's timer (`MAT-04`, `MAT-19`) needs nobody unless it asks for tending, such as feeding a kiln's fire, done in short activities while it runs; untended, it fails." (`MAT-04`'s field 6 holds the rest.)
- C16 (0.06 KB): `TIM-17` "Between ends" becomes: "needs change with time and effort (`BIO-09`), and at each end the doer chooses again (`MND-09`, `MND-16`)."
- C17 (0.29 KB): cut `TIM-18`'s Why and Example.
- C18 (0.10 KB): cut `TIM-14`'s "Their own calendars" (`CUL-13` owns it).
- C19 (0.28 KB): cut `TIM-01`'s "Only the pace changes" line and its Why (`TIM-17`, `PRN-10` and `PRE-44` own these).
- C20 (0.15 KB): cut `TIM-10`'s How it works (`TIM-17`, `PRE-44` and `SND-07` own it).
- C21 (0.05 KB): `TIM-15` item 2 as in #8.
- C22 (0.07 KB): cut `TIM-07`'s "Speed never comes from simpler minds or bodies" (`PRN-11` owns it).
- C23 (0.15 KB): `TIM-02`'s seven-line "What counts as important" becomes one line with the same items and IDs.
- C24 (0.25 KB): cut `TIM-02`'s Why and Example (`VIS-11` shows a live moment, and `RSK-03` gives the reason).
- C25 (0.08 KB): `TIM-02` Slowing down examples as in #8.
- C26 (0.05 KB): cut `TIM-03`'s "Follows from" line.
- C27 (0.08 KB): `TIM-11` How it works becomes: "at the director's next moment past the bar (`TIM-02`), it slows to the director's speed, even over the dial or the lock, until you tap or it passes, then hands the speed back (`TIM-15`), or after a game year with no such moment."
- C28 (0.14 KB): `TIM-08` How it works becomes: "each world keeps what `PLT-10` lists; switching saves the current world and opens the other."
- C29 (0.23 KB): `TIM-19`'s "Typical worlds" and "How it is met" become: "**How it is met:** by tuning alone, never by scripting or dates (`PRN-17`, `RES-16`)." (`VIS-03` owns valid stalls, and `RES-09` owns changing windows.)
- C30 (0.05 KB): `TIM-09` How it works becomes: "the last death is an important moment and a book-of-ages entry (`TIM-02`, `PRE-05`), and nature runs on, faster."

**Totals:** cuts about 3.9 KB (s04 1.3, s05 2.6).
The fixes above add about 2.3 KB to s04 and s05, so the two files fall by about 1.6 KB net, to about 29.9 KB.
The changes asked of other groups (`MND-33`, `CUL-24`, `PLT-01`, `PLT-04`, `WLD-12`, `WLD-16`, `MND-22`, `SCP-16`, `BIO-13`, `CUL-33`) add or change about 0.6 KB.
