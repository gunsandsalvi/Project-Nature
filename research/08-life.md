# Research 08: plants and animals

**Question:** how do games fill a big world with plants and animals that grow, move, and boom and crash with the weather and with each other, without simulating every blade and beast everywhere?
The items: `WLD-31`, `WLD-32`, `WLD-18`, `WLD-28`, `MND-16`, `BIO-19`, `WLD-30`, `WLD-13`.

## What `PROJECT.md` asks

- **About 30 wild animal species and their plants,** as catalogue entries with diets, groups, seasons and yields (`WLD-31`, `WLD-32`).
- **Herds far from people kept as counts:** with their condition, home ranges and seasonal migrations.
  Near people (within about 1 km), the animals become individuals with bodies and simple minds, and rejoin the count a day after nobody is near (`WLD-32`, `MND-16`).
- **Numbers that boom and crash by weather and by each other, never by script.**
  Each cell holds at most a sixth of what such land holds on Earth (`WLD-18`, `WLD-30`).
  In 100 test years without people, every species stays between half and twice its settled total, with about 1 big hunter to 50 to 200 prey.

## How others do it

### Plants

- **Horizon Zero Dawn** places its plants, rocks and wildlife on the graphics chip at run time, from rules artists write in a graph editor, reading maps of rivers, biomes and big trees ([Guerrilla, GDC 2017](https://www.guerrilla-games.com/read/gpu-based-procedural-placement-in-horizon-zero-dawn), [80.lv](https://80.lv/articles/the-procedural-nature-of-the-horizon-zero-dawn)).
- **Plants placed by their ecology:** Deussen and others (SIGGRAPH 1998) distribute plants "reflecting the interactions of plants with each other and with their environment".
  Plants compete for space, and crowded plants thin themselves ("self-thinning").
  Similar plants are then drawn as copies of a few representatives ([Algorithmic Botany](https://algorithmicbotany.org/papers/ecosys.sig98.html), [Stanford](https://graphics.stanford.edu/papers/ecosys/)).
- **Which plants grow where:** BIOME1's five climate numbers decide which plant types can grow, and dominance decides which win (research 06).

### Animals and their numbers

- **Predator and prey:** NetLogo's wolf-sheep model and Wolfram's agent-based demo show booms and crashes arising from eating, breeding and dying alone ([NetLogo](https://ccl.netlogo.org/cm/models/predation/info.html), [Wolfram](https://demonstrations.wolfram.com/PredatorPreyEcosystemARealTimeAgentBasedSimulation/)).
- **How many animals a land holds:** Damuth's law (Nature, 1981), "an instant and lasting classic".
  Among mammal plant-eaters, population density falls with body mass to the power of −0.75.
  So the energy a species' population uses is about the same whatever its size ([Damuth 1981](https://pdodds.w3.uvm.edu/files/papers/others/1981/damuth1981a.pdf), [UCSB](https://chancellor.ucsb.edu/memos/2024-04-19-sad-news-dr-john-damuth)).
  - This gives each species' natural density from its weight, and `WLD-30` takes a sixth of it, with no number guessed by hand.
- **Games:**
  - **Eco** (Strange Loop Games): every player's action "has the potential to affect the thousands of simulated plants and animals who occupy the game world" ([Cliqist](https://cliqist.com/2017/04/26/strange-loop-games-challenges-climate-deniers-eco/)).
    Over-harvesting can wipe a species out for good ([Wikipedia](https://en.wikipedia.org/wiki/Eco_(2018_video_game))).
  - **Equilinox** gives every species a life cycle, needs and preferred surroundings, and plants change the soil ([TV Tropes](https://www.tvtropes.org/pmwiki/pmwiki.php/VideoGame/Equilinox)).
  - **Red Dead Redemption 2:** about 200 species, each with its own behaviour.
    Herds cross the plains, "scavengers quickly sniff out carrion", "wolves attack in packs surrounding their prey" ([Gematsu](https://gematsu.com/2018/09/red-dead-redemption-2-details-wildlife-hunting-fishing-and-horses), [Game Informer](https://www.gameinformer.com/preview/2018/09/24/a-glimpse-into-red-dead-redemption-iis-amazing-wilderness)).
  - **Of Life and Land**, a Godot game, has animals that hunger, thirst, sleep and form social groups (research 01).

### Animals' days and their journeys

- **theHunter: Call of the Wild:**
  - each species has "need zones" for feeding, drinking and resting, each up to 200 m across;
  - each has set hours: whitetail deer feed from 5:00 to 9:30 and 15:00 to 21:30, and drink from 12:00 to 15:30;
  - almost all drink at least once a day;
  - some zones are visited only once or twice a week ([guide](https://gameplay.tips/guides/4433-the-hunter-call-of-the-wild.html), [Steam discussion](https://steamcommunity.com/app/518790/discussions/0/1519260397774535970)).

  It is a cheap, readable model of a herd's day.
- **Migration as surfing the green wave:** across 61 populations of four hoofed species, migrants follow spring's green-up when it advances as a clear wave, "not too broad, not too rapid and clearly progressive in space and time" ([USGS: Aikens and others, Current Biology 2020](https://pubs.usgs.gov/publication/70228944), [Current Biology dispatch](https://par.nsf.gov/servlets/purl/10248484)).
  Simulations of geese show the rule does not explain every migration ([PMC](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6522631/)).
  Agent-based models of Sahel herders track grazing over rainfall ([CoMSES](https://miracle.comses.net/codebases/?tags=Sahel)).
- **Herds and flocks** move by Craig Reynolds' three steering rules: keep apart, match heading, stay together ([alife.org](https://alife.org/encyclopedia/software-platforms/boids/), [GameDev.net](https://gamedev.net/blogs/entry/1599579-flocking-a-simple-overview)).

### Simulating near and far

- **S.T.A.L.K.E.R.'s A-Life** runs creatures in full ("online") within about 150 m of the player.
  Beyond, they move "offline" on a coarse graph while time passes, so events happen elsewhere ([Game Developer](https://www.gamedeveloper.com/design/a-life-an-insight-into-ambitious-ai), [interview](https://www.gamedeveloper.com/pc/interview-inside-the-ai-of-i-s-t-a-l-k-e-r-i-)).
  - Kindling's "near people, not near the camera" rule is the same idea.
    Ours must also give the same history whatever the camera does (`WLD-13`), so near means near a person.

## What we take

1. **Each species is a catalogue entry:** climate and soil ranges, seasons, size, diet, group size, yields, life cycle, model-kit form and colours.
   A new species is data.
2. **Plants by their ecology:**
   - which types can grow comes from BIOME1's numbers (research 06);
   - which win comes from dominance;
   - how dense from competition and self-thinning.

   Individual plants are placed from that density by keyed chance, Horizon-style, so a place always grows the same plants (`WLD-13`).
   Godot draws each species in each chunk as one MultiMesh (research 04).
3. **Animal numbers from Damuth's law:** each species' natural density from its body mass, and a sixth of it per cell (`WLD-30`).
   Predator-prey rules then run on the cells' totals, driven by the weather (`WLD-18`).
4. **Herds' days by need zones and hours,** as in theHunter.
   **Their years by following the green-up,** when it runs as a wave, and by summer and winter ranges.
   **Their movement by Reynolds' rules** plus those goals.
5. **Near people, individuals; far, counts.**
   It is A-Life's split, keyed to people rather than the camera, with the herd's condition and wariness carried both ways (`WLD-32`).
6. **Watched with numbers:** 20 worlds run 100 years without people.
   Each species stays within half and twice its total, and hunters to prey within `WLD-18`'s range.
   This is a scene in the cloud from the first ecology step.
7. **A prototype first:** the ecology on world cells alone, run headless for 100 years.
   It answers whether the totals stay believable before anything draws them.

## Sources

- Plants:
  - [Guerrilla: Horizon placement](https://www.guerrilla-games.com/read/gpu-based-procedural-placement-in-horizon-zero-dawn)
  - [80.lv](https://80.lv/articles/the-procedural-nature-of-the-horizon-zero-dawn)
  - [Deussen et al. 1998](https://algorithmicbotany.org/papers/ecosys.sig98.html) ([Stanford](https://graphics.stanford.edu/papers/ecosys/))
- Numbers:
  - [NetLogo: wolf-sheep](https://ccl.netlogo.org/cm/models/predation/info.html)
  - [Wolfram: predator-prey](https://demonstrations.wolfram.com/PredatorPreyEcosystemARealTimeAgentBasedSimulation/)
  - [Damuth 1981](https://pdodds.w3.uvm.edu/files/papers/others/1981/damuth1981a.pdf)
  - [UCSB: Damuth](https://chancellor.ucsb.edu/memos/2024-04-19-sad-news-dr-john-damuth)
- Games:
  - [Wikipedia: Eco](https://en.wikipedia.org/wiki/Eco_(2018_video_game))
  - [Cliqist: Eco](https://cliqist.com/2017/04/26/strange-loop-games-challenges-climate-deniers-eco/)
  - [TV Tropes: Equilinox](https://www.tvtropes.org/pmwiki/pmwiki.php/VideoGame/Equilinox)
  - [Game Informer: RDR2 wilderness](https://www.gameinformer.com/preview/2018/09/24/a-glimpse-into-red-dead-redemption-iis-amazing-wilderness)
  - [Gematsu: RDR2 wildlife](https://gematsu.com/2018/09/red-dead-redemption-2-details-wildlife-hunting-fishing-and-horses)
  - [theHunter need zones guide](https://gameplay.tips/guides/4433-the-hunter-call-of-the-wild.html)
  - [Steam discussion](https://steamcommunity.com/app/518790/discussions/0/1519260397774535970)
- Movement:
  - [USGS: wave-like green-up and ungulate movement (Aikens and others, 2020)](https://pubs.usgs.gov/publication/70228944)
  - [Current Biology dispatch on green waves](https://par.nsf.gov/servlets/purl/10248484)
  - [PMC: waterfowl](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6522631/)
  - [CoMSES: Sahel herds](https://miracle.comses.net/codebases/?tags=Sahel)
  - [alife.org: boids](https://alife.org/encyclopedia/software-platforms/boids/)
  - [GameDev.net: flocking](https://gamedev.net/blogs/entry/1599579-flocking-a-simple-overview)
- Near and far:
  - [Game Developer: A-Life](https://www.gamedeveloper.com/design/a-life-an-insight-into-ambitious-ai)
  - [Interview: S.T.A.L.K.E.R. AI](https://www.gamedeveloper.com/pc/interview-inside-the-ai-of-i-s-t-a-l-k-e-r-i-)
