# Research 09: people's bodies and lives

**Question:** how do games model needs, wounds, illness, birth, ageing, death and inheritance?
What do real hunter-gatherers' lives give us as numbers, so that Kindling's people live believable lives (`BIO-04` to `BIO-23`, `MND-07`, `RES-14`, `PRN-05`)?

## How games do it

- **Needs that run down:** in The Sims, decisions are driven by eight "motives":
  - hunger, hygiene, fun, energy, bladder, social, comfort and room;
  - each meter runs from −100 to +100 and runs down all the time, each at its own rate.
  - **Objects advertise what they offer:** "a bed will say sleep on me to get 10 energy", and Sims match the adverts to their needs.

  Will Wright modelled this on SimAnt's pheromone trails, putting intelligence in the environment ([GMTK transcript](https://gameindustrylibrary.com/documents/gmtk-the-genius-ai-behind-the-sims), [Amara](https://amara.org/videos/cVoJS4OdmVql/en/4330642)).
- **Wounds in layers:** Dwarf Fortress gives each body part layers of skin, fat, muscle and bone over organs and nerves ([Dwarf Fortress wiki: wound](https://dwarffortresswiki.org/index.php/Wound)):
  - cut skin bleeds a little;
  - cut fat bleeds more;
  - cut muscle or tendon can disable a limb, and a hurt leg makes a creature fall;
  - broken bone is the worst pain, usually knocking out.
- **Illness as a race:** in RimWorld, an illness's severity and the body's immunity both climb each day, and some diseases are "a race between the increasing severity of the disease and the increasing immunity to it" ([RimWorld wiki: disease](https://rimworldwiki.com/wiki/Disease)).
  For example, a wound infection gains +0.84 severity a day against +0.644 immunity, and good treatment takes 0.53 a day off.
  Wounds can each be infected separately.
- **Inheritance:** Crusader Kings III ([GameWatcher](https://www.gamewatcher.com/crusader-kings-3-marriage-and-genetics-guide), [Glitchout](https://glitchout.blog/2020/09/28/crusader-kings-3-eugenics-at-play/)):
  - "congenital" traits pass to children by chance, more often when both parents carry them;
  - traits can lie hidden and skip a generation.
- **A warning about populations:** in Banished, births depend on free houses.
  Slow building leaves the old in their homes and the young childless, so the town ages and collapses in waves ([Steam discussion](https://steamcommunity.com/app/242920/discussions/0/540734254927418463), [Play the Past](https://www.playthepast.org/?p=5805)).
  A population built on one gate oscillates.

## What real hunter-gatherers' lives give us

- **Lifespans** (Gurven and Kaplan, 2007: 3,328 deaths across foraging peoples) ([paper](https://gurven.anth.ucsb.edu/sites/secure.lsit.ucsb.edu.anth.d7_gurven/files/sitefiles/papers/GurvenKaplan2007pdr.pdf), [HRAF](https://hraf.yale.edu/ehc/documents/1294)):
  - 57% of children born reach 15;
  - of those, 64% reach 45;
  - life expectancy at birth is 21 to 37 years, yet those reaching 45 live about 20 more;
  - adult deaths peak at about seven decades;
  - illness causes more than half of all deaths, mostly respiratory, stomach and fever.
    Accidents and violence are next, and in old age degenerative disease.
- **Births** (Ache, Agta, Hadza, Hiwi and !Kung) ([Cambridge: Hadza demography](https://www.cambridge.org/core/books/demography-and-evolutionary-ecology-of-hadza-huntergatherers/fertility/42E2C841770F8C8191A2AFAF997AEF92), [arXiv](https://arxiv.org/pdf/2601.13442)):
  - first births around 20, last around 38;
  - about 3.1 years between births (2.8 to 3.3);
  - about 6 children per woman on average, from 4.7 (!Kung) to 8.1 (Ache).
  - The gaps come mainly from breastfeeding, which delays ovulation.
    The !Kung's four-year gap is put down to breastfeeding alone.
- **Energy:** Pontzer and others (2012) measured the Hadza with doubly labelled water ([PMC](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3405064/)).
  They are more active than Westerners, yet burn about the same energy a day for their size.
  Over 95% of their food is wild: tubers, berries, game, baobab fruit and honey.

## What we take

1. **Needs as levels that run down at their own rates,** with The Sims' insight: things and places advertise what they offer.
   A spring offers water and a fire offers warmth, and minds score these adverts (research 10).
   Bodies keep levels of food, water, warmth, rest and health (`BIO-09`).
2. **Energy by real numbers:** daily energy by body size and activity, at Western-like totals, as Pontzer found.
   Food values come from the catalogue.
3. **Wounds by body part and layer,** Dwarf Fortress-style (`BIO-13`), with bleeding, pain, disabled limbs, and infection per wound.
4. **Illness as a race between severity and immunity,** RimWorld-style, with plain care slowing severity (`BIO-05`, `BIO-23`).
   Illnesses spread by contact, water, food and wounds, by catalogue values.
5. **Births by biology, not by a gate:** fertility from age and nourishment, and gaps from breastfeeding.
   This gives Gurven and Kaplan's and the Hadza's numbers as results, not inputs, and avoids Banished's waves (`BIO-15`, `BIO-04`).
6. **Inheritance:** most traits blended from both parents with variation, and a few rare traits passed by chance and able to skip a generation, as in Crusader Kings (`BIO-06`).
7. **Checked against the real numbers:** whole-world runs must land near them (`RES-14`):
   - about half of children reaching 15;
   - adult deaths peaking near 70;
   - three-year birth gaps;
   - illness the main cause of death.

   A world far outside them is a bug to look into, not a finding.

## Sources

- Games:
  - [GMTK: the AI behind The Sims](https://gameindustrylibrary.com/documents/gmtk-the-genius-ai-behind-the-sims)
  - [Amara transcript](https://amara.org/videos/cVoJS4OdmVql/en/4330642)
  - [Dwarf Fortress wiki: wound](https://dwarffortresswiki.org/index.php/Wound)
  - [RimWorld wiki: disease](https://rimworldwiki.com/wiki/Disease)
  - [GameWatcher: CK3 genetics](https://www.gamewatcher.com/crusader-kings-3-marriage-and-genetics-guide)
  - [Glitchout: CK3](https://glitchout.blog/2020/09/28/crusader-kings-3-eugenics-at-play/)
  - [Steam: Banished population](https://steamcommunity.com/app/242920/discussions/0/540734254927418463)
  - [Play the Past: Banished](https://www.playthepast.org/?p=5805)
- People:
  - [Gurven and Kaplan 2007](https://gurven.anth.ucsb.edu/sites/secure.lsit.ucsb.edu.anth.d7_gurven/files/sitefiles/papers/GurvenKaplan2007pdr.pdf)
  - [HRAF summary](https://hraf.yale.edu/ehc/documents/1294)
  - [Cambridge: Hadza fertility](https://www.cambridge.org/core/books/demography-and-evolutionary-ecology-of-hadza-huntergatherers/fertility/42E2C841770F8C8191A2AFAF997AEF92)
  - [arXiv: menopause simulation study](https://arxiv.org/pdf/2601.13442)
  - [Pontzer et al. 2012](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3405064/)
