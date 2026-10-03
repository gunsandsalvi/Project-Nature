# Research 06: plants and animals

**Question:** how do games fill a big world with believable plants and animals that grow, move, and boom and crash with the weather and each other (`WLD-31`, `WLD-32`, `WLD-18`), without simulating every blade and beast everywhere?

## How others do it

- **Horizon Zero Dawn** places its plants, rocks and wildlife at run time on the GPU, from rules artists write in a graph editor.
  The rules read per-tile maps such as rivers, biomes and big trees.
  Its goals were quick iteration, large variety, a believable look and art direction ([Guerrilla GDC 2017 talk](https://www.guerrilla-games.com/read/gpu-based-procedural-placement-in-horizon-zero-dawn), [80.lv](https://80.lv/articles/the-procedural-nature-of-the-horizon-zero-dawn)).
- **Equilinox** gives every plant and animal its own life cycle, needs and preferred surroundings.
  Plants make the ground more fertile, and balance comes from meeting each species' needs ([TV Tropes](https://www.tvtropes.org/pmwiki/pmwiki.php/VideoGame/Equilinox)).
- **Predator and prey models**, such as NetLogo's wolf-sheep model and Wolfram's agent-based demo, show booms and crashes arising from simple rules of eating, breeding and dying ([NetLogo](https://ccl.netlogo.org/cm/models/predation/info.html), [Wolfram](https://demonstrations.wolfram.com/PredatorPreyEcosystemARealTimeAgentBasedSimulation/)).
- **Herds and flocks** move by Craig Reynolds' three steering rules: keep apart, match heading, stay together.
  Each animal sees only a few neighbours, yet believable herding emerges ([alife.org](https://alife.org/encyclopedia/software-platforms/boids/), [GameDev.net](https://gamedev.net/blogs/entry/1599579-flocking-a-simple-overview)).

## What we take

1. **Each species is a catalogue entry** (research 02, `PRN-14`):
   - its climate, soil, moisture and slope ranges;
   - its seasons, size, yield and life cycle;
   - its model-kit form and colours (research 04).

   A new plant or animal is data, not code.
2. **Placement by rules, from the seed, as in Horizon.**
   Each species' density at a place comes from its world cell's climate, soil, rivers and slope.
   Individual plants are placed from that density by keyed randomness, so the same place always grows the same plants (`WLD-13`).
   Godot draws each species in each chunk as one MultiMesh.
3. **Two levels of life, as the project already plans (`WLD-12`, `MND-16`):**
   - on world cells, plant cover and animal numbers are totals that grow, graze, hunt and die by simple predator-and-prey rules driven by the weather, so booms and crashes happen everywhere, unscripted (`WLD-18`);
   - near people, herds and animals become individuals with their own bodies and minds, made from the cell's totals and returned to them when people leave.
4. **Herds move by Reynolds' three rules plus goals** (graze, drink, flee, rest), so a herd flows round obstacles and splits around a hunter.
5. **Equilinox-style needs.**
   Every species has needs its surroundings meet or fail, and plants change the ground they grow on (cover, soil, fire fuel), feeding into the ecology rather than into a script.
