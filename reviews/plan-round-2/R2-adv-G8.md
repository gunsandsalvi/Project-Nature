# R2 adversary G8: section 11, Presentation, and section 12, Sound

Read: `ROUND.md` (with its Round 2 part), `BRIEF.md`, `notes/R1-coord.md`, `notes/R1-adv-G8.md`, `notes/R1-impl-G8.md`, all of `assembled.md`, the pre-test notes `B11`, `B66`, `B73` and `B74`/`B76`, and the mockup's code for its zoom stops, trees and cut-away.
Byte figures were measured by applying every change below to copies of s11 and s12 in memory; nothing in `sections/` was touched.

## Round 1: what landed

Landed as decided: D18 (`PRE-46`), D20 in `PRE-37`, `PRE-41`, `PRE-17` and `PRE-19`, D21 (`PRE-03`, cited by `TIM-01`), D23 (`PRE-08`), D28 (`SND-04` dropped), D29 (`PRE-05`, `PRE-19`, `PRE-39`), D31 to D33.

Lost or half-landed, each fixed below:
- R1 #3's "with herds, camps and people as in `PRE-28`" is missing from `PRE-03`: nothing says people show beyond the full areas (finding 1).
- R1 #10's "one for each pair of action and material class that makes a sound" is missing from `SND-06`, so "about 40" reads as 9 × 9 (finding 6).
- R1 #6's dance moves became "the moves of `CUL-10`", while `CUL-10` says "about 20 prepared moves (`PRE-44`)": nobody lists them (finding 7).
- D20 did not reach `MND-01` ("the writer AI only reads finished records") or `PRN-06`'s list of texts (finding 2).
- `PRE-45` and `SND-07` cite `PRE-28` for "drawn in full", which `PRE-03` defines (finding 1).
- Restatements left in my sections against C1: `SND-01` and `MND-16` (calls), `PRE-08` and `TIM-02` (the list), `PRE-42` and `PRE-46` (parts), `PRE-43` and `CUL-12` (style), `SND-05` and `SCP-16` (order), `SND-09` (see Cuts).

## Findings

### 1. [major] PRE-03, PRE-28, PRE-26, WLD-12, PLT-04 The land drawing has no outer tier and no bound near the camera
Problem:
- `PRE-03` draws "full areas, with every tree, stone and thing, ... within about 1 km of the camera from camp zoom inward".
  That is about 50 areas, each with up to about 5,000 single plants (`WLD-31`): up to about 250,000 trees and bushes in full, and about 50 areas made for the picture whenever the view moves.
  The settled mockup, the only scene measured (`B66`), draws a 160 × 100 m scene in full, single trees out to about 300–400 m, and the map's canopy beyond.
- "Farther out, an area is coarse ground" has no outer edge.
  At region zoom (about 100 km across) that is about 160,000 areas, and a tilted valley view runs to the horizon.
  `PRE-29` blends into the map "as the camera rises", but no item says where coarse ground stops and world cells start.
- R1's "with herds, camps and people as in `PRE-28`" did not land: beyond the full areas nothing says people, herds, camps or buildings are drawn (`PRN-10`).
- "Within about 1 km of the camera": at camp zoom the camera is hundreds of metres up, so a sphere round it and a circle round the camp differ.
- `PRE-26`'s Done when, "every river is at least one art pixel wide at every zoom stop", would cover the globe in lines: rivers start at 50 km² of drainage (`WLD-08`).
- `PRE-45` and `SND-07` cite `PRE-28` for "drawn in full"; `PRE-03` defines it.
Fix:
- `PRE-03`, How it works becomes:
  "- **How it works,** by distance from where the camera looks, as in the mockup: within about 300 m, from camp zoom inward, full areas (`WLD-12`), made for the picture without changing anything (`WLD-13`); out to about 10 km, each area's coarse ground, made from the seed in a moment, shaped every few tens of metres, under its cover (its cell's, or a kept area's own) drawn as forest canopy, scrub, grass or bare ground; beyond, and from region zoom out, the world cells (`PRE-29`).
    People, herds, camps and buildings show at every distance (`PRE-28`), and a full area being made shows its coarse ground until its detail fades in, within about a second."
- `PRE-28`: "people and animals become tiny outlined figures in their strongest colours, a group or herd close together one marker, ...".
- `PRE-26`, Done when: "every river is at least one art pixel wide from valley zoom inward, and farther out those draining about 1,000 km² or more (tuned)."
- `PRE-45`, `SND-07`: "drawn in full (`PRE-03`)".
Cross-section: `WLD-12` (G3): "for the picture only within about 300 m of where the camera looks, from camp zoom inward (`PRE-03`)"; `PLT-04` (G9): the benchmarks add "a camp in thick forest at camp zoom, turning".
Areas made for the picture fall from about 50 to about 5 a view, and full single plants from up to about 250,000 to about 22,000.

### 2. [major] PRE-41, PRE-17, PRE-37, PRN-06, MND-01, CUL-11 The text check says what it should catch, not what it compares
Problem:
- `PRE-41`: "The check compares every name, number, cause, event and image in a text with its records, with no language model involved."
  Without a language model only names, numbers, dates and places can be found in free English; causes, events and images can't.
  `B73`'s checker worked another way: every word of the text had to come from the data, a synonym list or about 13 neutral words, and even so it missed wrong statements made only of the data's words (a swapped giver, "Kelo" as the killer).
- "A sentence that adds, drops or swaps anything" says what to catch, not how, and "the doer before the act" needs the act found in the new sentence.
  One builder compares names and numbers only, so `B73`'s softened "made them carry" passes; another needs a language model, which `PRN-06` forbids.
- The flagship image passes `PRE-17` only "when it comes from" a belief, which a word check can't see unless the pattern sentence already holds those words.
- Contradictions: `MND-01` "the writer AI only reads finished records" against `PRE-17` "gets only a text's pattern sentences"; `PRN-06` lists "dreams" among reworded texts, `PRE-41` overnight and living summaries; `PRE-37` "the most important first" against `PRE-17` "built in date order".
Fix:
- `PRE-41`: "The check" and "Who did what" become:
  "- **The check,** with no language model: the writer rewords each pattern sentence on its own, in order.
    Each new sentence must keep its pattern's names, numbers, dates and places, in the same order, and add none; keep the words its pattern marks as needed, such as the act and words like *first* or *made*, or a synonym listed for them; and use no word, in any form, beyond its pattern's, those synonyms and a short fixed list of joining words.
    A sentence that fails is shown as its pattern sentence."
- `PRE-17`, first two sentences: "... and chooses words and rhythm, never content (`PRN-06`).
  Images, causes and motives appear only where a pattern sentence holds them, from a myth's story shape (`CUL-11`) or the tellers' beliefs (`MND-27`), such as "the fire that sleeps inside the wood"."
- `PRE-37`, Size: "chosen most important first and told in date order".
Cross-section: `MND-01` (G6): "the writer AI only rewords pattern sentences built from finished records"; `PRN-06` (G1): "for the book of ages, life stories, myths and summaries (`PRE-37`)"; `CUL-11` (G7): images are "written into the myth's pattern sentences".

### 3. [major] PRE-41, PRE-37, PRE-05, TIM-12, PLT-04 Most history would be stored as pattern text and never worded
Problem:
- `PRE-41`: history texts "are written when first opened or while the phone is idle"; `PRE-37`: when the writer "is missing, busy or fails the check, the pattern text is shown and stored".
- `B73`: the phone's writer works only while the app is on screen, so nothing is worded "while the phone is idle", and each entry takes about half a second to a second (0.26 s to the first word, 77 words a second).
- A night of a few hundred game years (`TIM-07`) leaves hundreds of new entries (`RES-25`: at least one a game year, more with many peoples).
  The first page you open finds the writer busy with its first entry, so by `PRE-37` the rest are stored as pattern text for good.
  Builders must guess between a wait, a swap, or that.
Fix:
- `PRE-41`: "History texts ... are checked, stored beside their records and never silently rewritten" (drop "written when first opened or while the phone is idle"), and before its Done when:
  "- **When:** texts are worded while the app is open, overnight mode included (`TIM-12`), since the phone's writer works only then, the page you open first and then the newest; a page shows its pattern text at once, and each checked sentence replaces its pattern as it arrives."
- `PRE-37`, Without the writer: "when it is missing" (a failed sentence is `PRE-41`'s, and busy no longer applies).
- `PRE-05`: "worded as `PRE-41` sets out" for "worded when first read (`PRE-37`)".
Cross-section: `TIM-12` (G2): "the night's texts are worded as it runs (`PRE-41`)"; `PLT-04` (G9): the overnight measurement runs with the writer wording.

### 4. [major] PRE-37, MAT-13, RSK-25 Pattern sentences are now all the text, and nothing counts them
Problem:
- Since round 1 every text starts as pattern sentences (`PRE-37`), so they are the game's text content, but nothing bounds them: "each kind of event has at least 5 phrasings", and no item lists the kinds.
- Kinds come from at least six places: the recognisers' entries (`PRE-39`, `CUL-07`, `TIM-02`), the memories and life events a life story tells (`MND-18`, causes of death in `BIO-14`), the 12 story shapes (`CUL-11`), the 12 topics on cards (`CUL-24`), a living mind's summary (`PRE-14`) and dark events (`CUL-08`).
  That is about 100 kinds and about 500 sentences, each with its needed words and synonyms (finding 2), all passing the trap set (`PRE-17`).
- `RSK-25` and `MAT-13` don't list them, and `PRE-37`'s Done when ("every kind of event shows 5 phrasings") can't be checked without a list.
Fix: `PRE-37`, Patterns: "a closed catalogue (`MAT-13`) of about 100 kinds of event (the entries of `PRE-39`, the memories of a life story, the story shapes and topics of `CUL-11` and `CUL-24`, and a mind's summary), each with at least 5 phrasings, picked by the event's seed and using all the records hold: ...".
Cross-section: `MAT-13` (G4) and `RSK-25` (G9) add "pattern sentences (`PRE-37`)"; they grow by stage like the other catalogues (`MAT-16`).

### 5. [major] PRE-46, PRE-15, CUL-09, RSK-25 Art is a second, hand-drawn picture stream outside the kit
Problem:
- `PRE-46` bounds the models, but paintings and carvings are made from "about 100 prepared motifs" (`CUL-09`): about 100 more pictures, drawn by hand in each people's line (outline or filled, thin or bold, `CUL-12`), which must match the 35 animal kinds the kit already draws.
- Nothing in section 11 says how a motif is drawn; `PRE-15` draws paintings "from their motifs (`CUL-09`)".
- Rock art shows animals and people mostly side-on, which the kit already holds, so the second stream buys little.
Fix:
- `PRE-46`: the People bullet ends at "(`CUL-12`)", and a bullet is added before the Done when:
  "- **Pictures and figures:** a painting or carving shows each animal, person or thing as its model seen from the side, drawn flat in its people's style (`CUL-09`, `CUL-12`), and a carved or clay figure is the model itself, small, in its material; only about 12 signs, also the patterns of `PRE-43`, are drawn by hand."
- `PRE-15`: "from their motifs (`PRE-46`)", dropping the body-paint clause, which `PRE-46` People already holds.
Cross-section: `CUL-09` (G7): "composed from motifs drawn from the model kit (`PRE-46`)", and its figure clause cites `PRE-46` only; glossary "Motif" (G9): "the side view of a kit model, or one of about 12 signs".
Content: about 100 hand-made motifs become about 12 signs.

### 6. [major] SND-06, MAT-06, MAT-01 The sound base set doesn't add up, and most actions have no sound
Problem:
- `SND-06`: "about 40 base sounds: striking, cutting, scraping, grinding, chopping, drilling, digging, dropping and stacking on the classes of `MAT-01`", then footsteps, fire, weather and instruments.
  Nine sounds on nine classes is 81, about 97 in all, not 40.
- `MAT-06` gives every base action "sounds by material (`SND-06`)", with Done when "each base action has its animation and sounds".
  But 14 of the 21 (gather, press, twist, bind, weave, shape, heat, soak, dry, mix, plant, throw, feed, apply) have no base sound, and chopping and dropping are not base actions.
Fix:
- `SND-06`, Made by the game: "about 40 base sounds: nine work sounds (striking, chopping, cutting, scraping, grinding, drilling, digging, dropping, stacking), each on hard (stone, bone, metal), woody (wood, plant) or soft things (earth, hide, flesh, water), and a rustle; footsteps on ...; fire by its heat; wind, rain, flowing water and thunder; flutes, drums and rattles" (27 + 1 + 8 + 1 + 4 + 3 = 44).
- `SND-06`, A sound blueprint: "is a base sound and how characteristics (`MAT-03`) and size (`MAT-02`) change it; base actions with no work sound borrow one (throwing, feeding and gathering as dropping, planting as digging, soaking as water, heating as fire, the rest as the rustle), and each result's sound (`MAT-21`) is its action's."
Cross-section: `MAT-06` (G4): "its sound (`SND-06`)", and Done when "each base action has its animation, and its sound as `SND-06` maps it".

### 7. [major] PRE-44, CUL-10, BIO-21, MND-16, RSK-25 The movement list is still open
Problem:
- `PRE-44`: "Dances are built from the moves of `CUL-10`"; `CUL-10`: "4–8 of about 20 prepared moves (`PRE-44`)".
  Each cites the other, and neither lists them.
- `BIO-21`: "Walking includes wading, swimming and climbing, each with its own animation (`PRE-44`)", but `PRE-44` has no wading.
- `MND-16` lets animals play (the pups of `MOM-06`); `PRE-44`'s animal set has no play.
- At close camp zoom figures are 10–25 art pixels tall: 20 dance moves can't be told apart there, while 8, strung 4–8 at a time, still give each people its own dances.
Fix:
- `PRE-44`: "**Dances** are 8 moves (step, stamp, turn, sway, crouch, leap, clap, arms raised), strung as `CUL-10` says; rites ...".
- The list: "and wading, swimming, climbing, ...".
- Animals: "... rest, sleep, play, fight, ...".
- Opening, shorter: "so you can tell who is knapping or dancing".
Cross-section: `CUL-10` (G7): "4–8 of the 8 dance moves of `PRE-44`"; `RSK-25` cites `PRE-44` for dance moves (G9).
Content: 20 dance moves become 8; the closed total is about 45 movements, 8 gestures, 8 dance moves, and 12 loops for each of 6 animal patterns.

### 8. [major] PRE-13, PRE-35, PRE-40, PRE-16, PRE-17, CUL-07 Songs, myths and art have no card, and the phone checks have no screen
Problem:
- `PRE-13`'s Check wants every kind of record on a card or view.
  `CUL-07` says a people's songs, myths and art show "on cards (`PRE-35`)", but `PRE-35`'s people card lists none of them.
- Four phone tools have no home: the stage benchmark (`PLT-04`, "one tap"), the contact sheet (`PRE-31`, "a tool makes on the phone"), the sound reel (`SND-12`) and the writer's trap set (`PRE-17`), which can run only on the phone because the writer exists only there (`B73`).
  `PRE-40` lists only the launch screen, the world list, settings and credits.
- `PRE-16`'s "index of species cards" is a screen that a page of the book of ages can hold, by `PRE-13`'s own rule.
Fix:
- `PRE-35`, people card: "customs, beliefs, rites, myths, songs, art, calendar, ...".
- `PRE-40` adds: "Settings also open the review page, which runs a stage's phone checks in one go (`PLT-04`, `PRE-17`, `PRE-31`, `SND-12`)."
- `PRE-16`: "Species cards, listed on a page of the book of ages and reached by tapping any plant or animal, ...".
Cross-section: `MND-13` (G6) can cite `PRE-35` rather than `PRE-10` for lines of teaching.

### 9. [major] SND-03, WLD-11 Making each world's voices has no time
Problem: `SND-03`: "each world's voices are made with it: about 200 short murmured phrases for each of two base voices".
`B76`: the neural voice takes about 9% of a core for each second of speech (the phone may differ two times either way) and about 1.2 s to load, so 400 phrases are roughly a minute of one core.
`WLD-11`'s times (3 minutes to three globes, 1 minute to settle) leave it out, so builders either stall the first talk or make phrases during play.
Fix: `SND-03`: "each world's voices are made while it settles (`WLD-11`): ...".
Cross-section: `WLD-11` (G3): "and settling the chosen world, with its bands (`BIO-03`) and voices (`SND-03`), at most about 1 minute more".

### Cuts

Each cut drops only a restatement, an example or a summary; what it says stays in the item named.
Measured on s11 and s12 as they stand (37.3 KB together):

- **K1** `PRE-04` *Dropped* ("it only summed up `PRE-22`, `PRE-28` and `PRE-29`"); nothing cites it: 0.15 KB.
- **K2** `SND-05` *Dropped* ("the milestones say which sounds come when, `SCP-16`"), and the section 12 intro loses "(`SND-05`)"; `MIL-03` and `MIL-05` already say it: 0.25 KB.
- **K3** `SND-09` *Dropped* ("quiet already follows from `SND-01`, `SND-07` and `SND-11`"); "the globe near silent" joins `SND-07`'s Done when: 0.20 KB.
- **K4** `PRE-08`'s own list becomes "moments and signs from the story director's one list, such as a major named discovery (`MAT-21`), within its one budget (`TIM-02`)": 0.20 KB.
- **K5** `PRE-38` *Dropped*, merged: `PRE-37` gets "Texts are in English, each name in its people's language with its meaning at first use (`CUL-18`)" and its Done when, `PRE-35` "in English with the people's own names and words beside"; `CUL-18` (G7) cites `PRE-37`: 0.27 KB.
- **K6** `PRE-02`'s "The mockup's scene was measured ..." sentence; its Done when and `PLT-04` say it: 0.15 KB.
- **K7** `SND-02`'s Example; `CUL-10` and `CUL-29` carry festival songs: 0.15 KB.
- **K8** `SND-01`'s animal-call sentence becomes "an animal's call (`MND-16`)"; `MND-16` holds the same sentence: 0.13 KB.
- **K9** `PRE-05`'s tabs line; `PRE-06`, `PRE-08`, `PRE-09` (and `PRE-16`, finding 8) each name their page: 0.12 KB.
- **K10** `PRE-09`'s cut-away clause and "people in the world find old camps too"; `MAT-08` and `MND-11` own them: 0.12 KB.
- **K11** `PRE-06`'s card and mind clauses become "the camera staying with them if you wish"; `PRE-35` and `PRE-14` say the rest: 0.11 KB.
- **K12** `PRE-43`'s restated style list becomes "sets its proportions, lean and colours within each ladder, and puts their pattern ... as much as the style says"; `CUL-12` owns the six choices: 0.08 KB.
- **K13** `PRE-42`'s part rule becomes "parts are sized by the amount used (`PRE-46`), and icons come from the same model"; `PRE-46` Made things owns it: 0.10 KB.
- **K14** Small doubles in sound: `SND-12`'s "no two strikes or steps sound alike" (`SND-06`'s Done when), `SND-07`'s "the weather follows the seasons", `SND-03`'s "up to 4 voices ... nearest first" (`SND-01`'s share): 0.09 KB.
- **K15** `PRE-39`'s "found by fixed measures ... names for you only and change nothing" becomes "for you only (`CUL-07`)": 0.08 KB.
- **K16** Drop lines shortened: `SND-04` ("a score is sound added for show, `PRN-10`"), `PRE-11` ("they show on each people's card, `PRE-35`"), `PRE-36` ("the language never changes, `CUL-17`"): 0.16 KB.
- **K17** `SND-03`'s "quick and loud in anger, soft and slow in grief" (its How it works says it) and `SND-02`'s "at a festival, around the fire, at a burial": 0.09 KB.
- **K18** `PRE-01`'s screen-resolution clause becomes "drawn in art pixels (`PRE-22`)": 0.06 KB.
- **K19** `PRE-33`'s list of time controls becomes "(`TIM-04`, `TIM-11`, `TIM-12`)": 0.06 KB.
- **K20** `PRE-18`'s "text states them plainly at every level (`PRE-17`)": 0.05 KB.
- **K21** `PRE-37`'s "offline and at no running cost"; `PLT-03` and `PRC-01` own it: 0.03 KB.

Cuts total about 2.7 KB; findings 1 to 9 add about 1.25 KB.
s11 ends at about 28.8 KB and s12 at about 7.1 KB: 35.9 KB together, 1.4 KB less than now.
Still short of round 1's targets (26 and 7): what remains are the lists that bound content (movements, kit, overlays, the mind view, the 32-sound shares) and one Done when per feature.

Last resort, only if the file is still over 320 KB:
- **K22** `SND-10` *Dropped* ("not part of the launch sound; it can come with a later layer"), with `PRE-40` losing "and vibration (`SND-10`)": 0.21 KB.

Cuts in others' files that my findings free:
- `CUL-09`'s figure clause cites `PRE-46` instead of restating it (G7): about 0.09 KB.
- `TIM-01`'s "each figure keeps showing what it is doing, however fast (`PRE-44`)" restates `PRE-44`'s At speed (G2): about 0.07 KB.

Cuts in content, not bytes:
- About 100 hand-made motifs become about 12 signs (finding 5).
- 20 dance moves become 8 (finding 7).
- Areas made for the picture fall from about 50 to about 5 a view, and full single plants near the camera from up to about 250,000 to about 22,000 (finding 1).
