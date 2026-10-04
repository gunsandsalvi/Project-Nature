# Research 12: culture, language, belief and society

**Question:** how do games give each people its own words, customs, beliefs and history, made by the world rather than written by hand?
What do real foragers' bands, leaders, gifts, killings and gatherings give us, so that Kindling's societies grow believably (`CUL`, `SCP-18`, `SCP-20`, `PRN-07`)?

## What `PROJECT.md` asks

- **One language per world,** made with it from the murmur's syllables, never changing.
  Each people coins words for new things by joining old ones, as *kesh*, 'bite stone' (`CUL-17`, `CUL-18`).
- **Nothing social is scripted:** templates give shapes, and events decide which happen (`CUL-07`).
- **Beliefs from events:** spirits, taboos, rites and offerings by seven templates; ancestors from grief and dreams; religion grows in steps from shared spirits to shamans to priests (`CUL-05`, `CUL-19`, `CUL-20`, `CUL-34`, `CUL-26`).
- **Customs** answer 12 fixed questions by the way two thirds of a band's cases went; norms and punishments follow (`CUL-06`).
- **Society:**
  - bands that split and join (`CUL-30`);
  - kin and marriage (`CUL-27`);
  - leaders, councils and chiefs (`CUL-22`);
  - sharing and trade (`CUL-21`);
  - feuds, raids and alliances (`CUL-31`);
  - peoples and villages (`CUL-23`, `CUL-28`).
- **Expression:**
  - art (`CUL-09`) and music (`CUL-10`);
  - myths that drift with each telling (`CUL-11`);
  - styles that drift (`CUL-12`);
  - gatherings that become festivals (`CUL-29`).

## How games do it

### Languages

- **Naming languages:** Martin O'Leary's generator, made for his procedural map bot, "generates a naming language based on a bunch of tables which specify phonemes & orthographic mappings along with some other rules":
  - sound classes such as "C: ptkmnh" and "V: aeiou";
  - syllable shapes;
  - spellings;
  - morphemes with meanings that combine into names.

  Because the "words are all generated from the same rules, they should be consistent with each other" ([O'Leary's notes](https://www.mewo2.com/notes/naming-language/), [port and docs](https://codeberg.org/asmaloney/naming-language-gen)).
- **Fuller languages:** Vulgarlang generates the sounds, words for English meanings and grammar rules from one seed ([Vulgarlang](https://www.vulgarlang.com/how-it-works/)).
- **Dwarf Fortress:** words are shared English roots, each with noun, verb and adjective forms.
  Each language spells every root its own way (`T_WORD`).
  A civilisation prefers some word groups and avoids others: dwarves pick names from words of "artifice", such as hammer and steel, and never from "flowery" ones, such as butterfly ([DF wiki: language token](https://dwarffortresswiki.org/index.php/Language_token), [raws](https://dwarffortresswiki.org/index.php/Raw)).

### Customs and beliefs as data

- **Dwarf Fortress ethics:** "Ethic tags are used in the entity raw files to determine how different civilizations feel about various issues", such as killing a member, eating the dead, slavery or treason.
  - Answers run from "acceptable" and "personal matter" through "shun" and "appalling" to "punish exile", "punish capital" and "unthinkable".
  - "Similar ethics result in friendship, while conflicting ethics result in animosity", and strong conflicts start wars during world generation ([DF wiki: ethic](https://dwarffortresswiki.org/index.php/Ethic)).
  - The ethics are fixed per race, written by hand.
- **RimWorld Ideology:** "Precepts directly determine how colonists will react to certain actions":
  - a precept against cannibalism gives "−20 Ate human meat" for a day;
  - one for it gives "+6";
  - precepts cover corpses, executions, drugs, marriage, animals and rites.

  The player designs the ideoligion or takes a randomised preset ([RimWorld wiki: precepts](https://rimworldwiki.com/wiki/Precepts)).
- **Crusader Kings III** lets a ruler merge two cultures into a hybrid or split one off, choosing its traditions ([PC Gamer](https://pcgamer.com/crusader-kings-3-ck3-hybrid-culture)).
- **The difference for Kindling:**
  - all three are designed, by the developer or the player;
  - in Kindling, a custom is the way most of a band's real cases went (`CUL-06`), and a belief comes from an event (`CUL-05`).
  - What we borrow is the shape: a fixed list of questions, each with a few answers, read by minds as approval or disapproval, as RimWorld's precepts are.

### History

- **Dwarf Fortress simulates its history,** so that players can "interrupt its simulated history at any point to start play".
- **Caves of Qud does the reverse** (Grinblat and Bucklew, FDG 2017) ([paper](https://pcgworkshop.com/archive/grinblat2017subverting.pdf), [GDC](https://gdcvault.com/play/1024990/Procedurally-Generating-History-in-Caves)):
  - "a state machine and replacement grammar" first generates events and "rationalizes them ex post facto";
  - its accounts "originate from inherently limited perspectives and represent particular points of view".
- **Kindling must simulate,** as Dwarf Fortress does, since every entry traces to what happened (`PRN-10`, `PRN-15`).
  Qud's insight still applies to myths: accounts are told from a point of view, which `CUL-11`'s drift toward the teller gives.

## What real foragers give

### Bands, kin and leaders

- **Bands are mostly not close kin.** Hill and others (Science 2011) counted who lives with whom in 32 foraging societies, 5,067 people ([abstract](https://mindblog.dericbownds.net/2011/03/new-view-of-early-human-social.html), [ASU](https://news.asu.edu/20110310_hunter-gatherers)):
  - "mean experienced band size = 28.2 adults";
  - "either sex may disperse or remain in their natal group";
  - "adult brothers and sisters often co-reside";
  - "most individuals in residential groups are genetically unrelated".

  This supports bands splitting past about 40 (`CUL-30`).
  It also means our glossary's "band: mostly kin" should read "a few families, linked by kin and marriage" (below).
- **Where couples live:** Marlowe (2004) "challenges an earlier finding that hunter-gatherers are predominantly virilocal".
  Foragers are "more multilocal than nonforagers", which he links to bride service, small groups, little wealth and little warfare ([HRAF](https://hraf.yale.edu/documents/371)).
  This supports `CUL-06`'s answers "with his kin, hers, either".
- **Leaders who cannot rule:** Boehm's "reverse dominance hierarchy": "the pyramid is flipped. The politically-united subordinates dominate the alpha types with threat of force".
  Punishment "runs the gamut from criticism to ridicule to ostracism to execution" ([Boehm, quoted by Henderson](https://www.robkhenderson.com/p/reverse-dominance-hierarchies)).
  This supports `CUL-22`: leaders are followed for trust and respect, and the band backs whoever it trusts more when a rival challenges.

### Gifts, trade and stores

- **Gifts as insurance:** among the !Kung, hxaro is "a regional system of reciprocity for reducing risk" (Wiessner 1977).
  Delayed gifts between partners in other camps give each person a claim on distant land in hard times ([Glottolog record](https://glottolog.org/resource/reference/id/25961), [HG Cosmos: reciprocity](https://www.hgcosmos.org/assets/topics/Reciprocity.pdf)).
  This supports gifts leaving favours owed (`MND-26`, `CUL-21`).
- **Trade by distance:** in Renfrew's study of Neolithic obsidian, "areas less than 300 km from the geological source area … are in the 'supply zone' because obsidian represents over 80% of the material".
  Beyond, amounts fall off with distance, passed "down the line" from group to group ([MapAspects](https://mapaspects.org/book/export/html/1901/index.html)).
  This supports trade only where people meet (`CUL-16`, `CUL-21`): a good stone thins out with each band it passes.
- **Stores change everything:** Testart (1982) "points out that more than half of the hunter–gatherer societies known to ethnology … share the same characteristics as agricultural societies: a sedentary society … an increased demographic density … significant hierarchies".
  He sorts societies by "whether or not their economies rely on the large scale stockpiling of a seasonal, basic food resources" ([Wikipedia: Alain Testart](https://en.wikipedia.org/wiki/Alain_Testart)).
  - The Natufians "supported a sedentary or semi-sedentary population even before the introduction of agriculture" and are "the first to exhibit evidence of food storage" ([Wikipedia: Natufian culture](https://en.wikipedia.org/wiki/Natufian_culture)).
  - Both support `CUL-28`: villages come from stores, fields, herds or rich fishing, and chiefs only in large villages.

### Killings, feuds and raids

- **Most killings are personal:** Fry and Söderberg (Science 2013) studied 148 killings in 21 mobile foraging societies ([Scientific American](https://www.scientificamerican.com/blog/cross-check/new-study-of-foragers-undermines-claim-that-war-has-deep-evolutionary-roots/), [HRAF](https://hraf.yale.edu/ehc/documents/868)):
  - "only two out of 148 killings stemmed from a fight over 'resources'";
  - "revenge for a previous attack" was the most common specific cause;
  - three societies had no killings at all.
- **But raids happen:** at Nataruk, Kenya, 9,500 to 10,500 years ago, 27 people, "including at least eight women and six children", were killed by blows and arrows.
  It is "unique evidence that warfare was part of the repertoire of inter-group relations among some prehistoric hunter-gatherers" (Lahr and others, Nature 2016) ([Cambridge](https://www.cam.ac.uk/node/165522)).
- **What this means:** feuds from killings come first and most often; raids are possible but rarer among mobile bands, and grow with stores and villages to raid.
  `CUL-31` already has these causes, and `CUL-33`'s order (first feud Years 20–100, first raid 60–200) matches.

### Gatherings

- **Göbekli Tepe** was "constructed by hunter-gatherers right after the end of the last Ice Age".
  "No typical domestic structures have yet been found, leading to the interpretation of Göbekli Tepe as a ritual centre for gathering and feasting", with "enormous amounts of meat, most likely during feasts".
  "Work forces necessary for such collaborative projects can be gathered with the prospect of lavish feasts" ([DAI: Tepe Telegrams](https://www.dainst.blog/the-tepe-telegrams/2017/08/02/neolithic-gathering-and-feasting-at-the-beginning-of-food-production/)).
  This supports `CUL-29`: gatherings where food is plentiful, kept at the same place and season, become festivals.

### Belief, rites and stories

- **Rites from lucky coincidences:** Skinner (1948) fed hungry pigeons on a clock, whatever they did.
  - "In six out of eight cases" each bird repeated whatever it had happened to be doing: one turned "counter-clockwise about the cage", another "repeatedly thrust its head into one of the upper corners".
  - He compared it to people: "A few accidental connections between a ritual and favorable consequences suffice to set up and maintain the behavior in spite of many unreinforced instances" ([York University: Classics in the History of Psychology](https://www.yorku.ca/pclassic/Skinner/Pigeon/)).

  This is `CUL-34`'s rule: the act a band credits for a good outcome becomes a rite.
- **Two kinds of religion:** Whitehouse's "modes of religiosity" ([Whitehouse](https://www.harveywhitehouse.com/books/modes-of-religiosity-a-cognitive-theory-of-religious-transmission)):
  - In the "imagistic mode", rare, intense rites "have a lasting impact on people's minds" and form "small, exclusive" groups.
  - In the "doctrinal mode", "religious knowledge is primarily spread through intensive and repetitive teaching; religious communities are contrastingly large, inclusive, and centrally regulated".

  This supports `CUL-26`'s steps: a shaman's vivid rites in bands, a priest's calendar rites in large villages.
- **Stories drift toward the familiar:** in Bartlett's serial retelling of *The War of the Ghosts* (1932), elements that did not fit the listener's expectations "were omitted from the recollection, or transformed into more familiar forms": "canoes" became "boats" ([Wikipedia: Frederic Bartlett](https://en.wikipedia.org/wiki/Frederic_Bartlett)).
  This supports `CUL-11`'s drift: a cause shifts toward the teller's beliefs, a deed to a more famous person.
- **Gossip travels best:** passed along chains of people, "gossip … was transmitted with significantly greater accuracy and in significantly greater quantity than equivalent non-social information" (Mesoudi, Whiten and Dunbar 2006) ([St Andrews](https://research-portal.st-andrews.ac.uk/en/publications/a-bias-for-social-information-in-human-cultural-transmission/)).
  This supports gossip as a topic of its own (`CUL-24`, `MND-33`), and suggests social news should be kept more faithfully than other news.

### Style

- **Styles drift by copying:**
  - Bentley and Shennan (2003) found that random copying "predicts the frequencies of pottery decorations remarkably well over a 400-year span" of early farming villages in Germany's Merzbach valley, with "an anti-conformist, or pro-novelty, bias" later ([Cambridge](https://www.cambridge.org/core/journals/american-antiquity/article/cultural-transmission-and-stochastic-network-growth/26F58B33F4DBBA4BA4715D37325A214E)).
  - Neiman (1995) showed drift explains decoration changes in Illinois Woodland pottery ([Cambridge](https://resolve.cambridge.org/core/journals/american-antiquity/article/stylistic-variation-in-evolutionary-perspective-inferences-from-decorative-diversity-and-interassemblage-distance-in-illinois-woodland-ceramic-assemblages/69A948F595717901C75F84D033F1A1C9)).

  This supports `CUL-12`: styles drift a step at a time, toward linked peoples, so a thing shows who made it and roughly when.

## Can Godot do it?

- **The culture is simulation in the C++ library** (research 03); none of it needs the engine.
- **What Godot shows:**
  - names and their meanings as text;
  - bubbles of topics (`PRE-45`);
  - art from the model kit (research 17);
  - music and the murmur (research 15).
- **One check is needed:** the language's spellings must use only letters the game's pixel font has.
  Godot's fonts can fall back to a second font, but a missing letter would show as a box.
  So the language generator draws only from the font's letters, and a test checks every word of many worlds (`PRE-37`).

## What we take

1. **A naming language in O'Leary's way,** from the seed:
   - sounds from the murmur's syllables (`SND-03`);
   - syllable shapes;
   - a spelling the pixel font can show;
   - a few hundred everyday words with meanings.

   New words join old ones, as Dwarf Fortress joins roots, and each people prefers its own words for names (`CUL-17`, `CUL-18`).
2. **Customs as fixed questions with a few answers,** like Dwarf Fortress's ethics and RimWorld's precepts.
   But each answer comes from a band's own cases, never from design (`CUL-06`).
   Minds read customs as approval or disapproval of acts seen (`MND-09`, `MND-24`).
3. **History simulated, never rationalised after the fact;** myths told from a point of view, as Qud's accounts are, through `CUL-11`'s drift.
4. **Beliefs and rites from coincidences,** as Skinner's pigeons show: the act before a good outcome becomes a rite, harm after an act a taboo (`CUL-05`, `CUL-20`, `CUL-34`).
   Small bands keep vivid, rare rites; large villages keep regular ones led by priests, as Whitehouse's two modes describe (`CUL-26`).
5. **Societies with real numbers:**
   - bands of about 28 adults, mostly not close kin, with either sex moving at marriage (Hill, Marlowe);
   - leaders kept in check by the band (Boehm);
   - gifts as insurance (Wiessner);
   - stone thinning out with distance (Renfrew);
   - villages and chiefs only where stores allow (Testart, the Natufians).

   The pace tests check that worlds land near them (`RES-07`).
6. **Violence in its real order:** personal killings and revenge first; raids rarer among mobile bands and growing with stores (Fry and Söderberg, Nataruk) (`CUL-31`, `CUL-33`).
7. **Stories and gossip drift as transmission chains do:** stories toward the familiar (Bartlett); gossip kept better than other news (Mesoudi) (`CUL-11`, `CUL-24`).
8. **Styles drift by copying with small random changes** (Bentley and Shennan, Neiman) (`CUL-12`).
9. **A culture prototype before production:** two or three bands with simple minds, run headless for a hundred years.
   It checks that customs, a spirit, a rite and a band split arise inside `CUL-33`'s windows, and from their own causes.

## For your decision

- **Glossary, "Band":** it says "a small group, mostly kin".
  Hill's census of 32 foraging peoples found "most individuals in residential groups are genetically unrelated".
  The suggested wording is "a small group of a few families, linked by kin and marriage, who live and move together".
  It changes no rule, since `CUL-30` already speaks of families.

## Sources

- Languages:
  - [O'Leary: naming languages](https://www.mewo2.com/notes/naming-language/)
  - [naming-language-gen port](https://codeberg.org/asmaloney/naming-language-gen)
  - [Vulgarlang](https://www.vulgarlang.com/how-it-works/)
  - [DF wiki: language token](https://dwarffortresswiki.org/index.php/Language_token)
  - [DF wiki: raws](https://dwarffortresswiki.org/index.php/Raw)
- Customs and history in games:
  - [DF wiki: ethic](https://dwarffortresswiki.org/index.php/Ethic)
  - [RimWorld wiki: precepts](https://rimworldwiki.com/wiki/Precepts)
  - [PC Gamer: CK3 hybrid cultures](https://pcgamer.com/crusader-kings-3-ck3-hybrid-culture)
  - [Grinblat and Bucklew 2017](https://pcgworkshop.com/archive/grinblat2017subverting.pdf)
  - [GDC: history in Caves of Qud](https://gdcvault.com/play/1024990/Procedurally-Generating-History-in-Caves)
- Bands, kin and leaders:
  - [Hill et al. 2011 abstract](https://mindblog.dericbownds.net/2011/03/new-view-of-early-human-social.html)
  - [ASU: Hill et al.](https://news.asu.edu/20110310_hunter-gatherers)
  - [HRAF: Marlowe 2004](https://hraf.yale.edu/documents/371)
  - [Henderson on Boehm](https://www.robkhenderson.com/p/reverse-dominance-hierarchies)
- Gifts, trade and stores:
  - [Glottolog: Wiessner 1977](https://glottolog.org/resource/reference/id/25961)
  - [HG Cosmos: reciprocity](https://www.hgcosmos.org/assets/topics/Reciprocity.pdf)
  - [MapAspects: Renfrew's zones](https://mapaspects.org/book/export/html/1901/index.html)
  - [Wikipedia: Alain Testart](https://en.wikipedia.org/wiki/Alain_Testart)
  - [Wikipedia: Natufian culture](https://en.wikipedia.org/wiki/Natufian_culture)
- Violence:
  - [Scientific American: Fry and Söderberg](https://www.scientificamerican.com/blog/cross-check/new-study-of-foragers-undermines-claim-that-war-has-deep-evolutionary-roots/)
  - [HRAF: Fry and Söderberg 2013](https://hraf.yale.edu/ehc/documents/868)
  - [Cambridge: Nataruk](https://www.cam.ac.uk/node/165522)
- Gatherings: [DAI: Göbekli Tepe feasting](https://www.dainst.blog/the-tepe-telegrams/2017/08/02/neolithic-gathering-and-feasting-at-the-beginning-of-food-production/)
- Belief and stories:
  - [Skinner 1948](https://www.yorku.ca/pclassic/Skinner/Pigeon/)
  - [Whitehouse: modes of religiosity](https://www.harveywhitehouse.com/books/modes-of-religiosity-a-cognitive-theory-of-religious-transmission)
  - [Wikipedia: Frederic Bartlett](https://en.wikipedia.org/wiki/Frederic_Bartlett)
  - [Mesoudi et al. 2006](https://research-portal.st-andrews.ac.uk/en/publications/a-bias-for-social-information-in-human-cultural-transmission/)
- Style:
  - [Bentley and Shennan 2003](https://www.cambridge.org/core/journals/american-antiquity/article/cultural-transmission-and-stochastic-network-growth/26F58B33F4DBBA4BA4715D37325A214E)
  - [Neiman 1995](https://resolve.cambridge.org/core/journals/american-antiquity/article/stylistic-variation-in-evolutionary-perspective-inferences-from-decorative-diversity-and-interassemblage-distance-in-illinois-woodland-ceramic-assemblages/69A948F595717901C75F84D033F1A1C9)
