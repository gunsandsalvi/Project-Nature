# Research 06: making the world

**Question:** how do other generators make believable continents, mountains, rivers, climates, soils, biomes and deposits?
How do they do it quickly enough for your phone, and the same from the same seed every time?
The items: `WLD-01`, `WLD-03`, `WLD-06`, `WLD-08` to `WLD-11`, `WLD-14`, `WLD-16`, `WLD-17`, `WLD-24`, `WLD-26`, `WLD-27`.

## What `PROJECT.md` asks

- A world of about 2,000 by 1,000 km, wrapping both ways with a latitude (`WLD-01`, `WLD-03`).
  It is held as about 2 million cells of about 1 km, with weather cells of about 10 km (`WLD-12`, `WLD-16`).
- **Made in the real order of causes:** plates, rock, erosion, climate, water, soils, biomes and deposits (`WLD-09`).
  The plant cover, water and herds then settle for 10 years (`WLD-08`).
- **About 20 candidate worlds,** the best three offered, within about 3 minutes on the phone (`WLD-10`, `WLD-11`).
- **The same world from the same seed and rules,** and checks such as rivers reaching the sea and few coasts straight for over 20 km (`WLD-08`).

## How others do it

### Whole generators

- **Red Blob Games' mapgen4:**
  - builds on a mesh of triangles;
  - simulates evaporation, wind and rain to place biomes and rivers;
  - runs on several threads.

  Its first lesson: make the elevation "match the desired look instead of tweaking the look to match the elevation" ([Red Blob Games](https://www.redblobgames.com/maps/mapgen4/), [sphere maps](https://simblob.blogspot.com/2018/10/map-generation-on-sphere.html)).
- **World Orogen** runs 16 stages in a few seconds ([GitHub](https://github.com/raguilar011095/planet_heightmap_generation), GPL, so we learn from it but copy nothing):
  - points on a sphere;
  - tectonic plates whose edges raise ranges, ridges, trenches and island arcs;
  - four kinds of erosion (glaciers, rivers with "priority flood", slumping slopes, creeping soil);
  - winds and currents bent by the planet's turn;
  - rain where moist air meets hills;
  - about 30 Köppen climates.
- **Dwarf Fortress:**
  - makes fractal maps of elevation, temperature (by latitude and height), rainfall (with rain shadows) and drainage, then volcanism and "savagery";
  - **rejects worlds** that fail its criteria, such as too few mountain tiles, and starts again;
  - logs each rejection's reason ([Dwarf Fortress wiki: world generation](https://dwarffortresswiki.org/index.php/DF2014:World_gen), [world rejection](https://dwarffortresswiki.org/index.php/40d:World_rejection)).

  This is the model for generating several candidates and keeping the best (`WLD-10`).
- **Undiscovered Worlds** makes a global map first, then regional detail as you zoom in.
  Its author admits "the same seed does not always produce the same planet", a warning about determinism ([forum](https://forum.thegamecreators.com/thread/223804), [GitHub](https://github.com/JonathanCRH/Undiscovered_Worlds)).
- **Azgaar's Fantasy Map Generator** separates settings, generators that make data, and renderers that draw it ([GitHub](https://github.com/Azgaar/Fantasy-Map-Generator)).
- **"Around The World"** is a game whose devlog generates a planet's plates, climate and biomes ([Frozen Fractal: climate](https://frozenfractal.com/blog/2023/12/29/around-the-world-9-climates/), [biomes](https://frozenfractal.com/blog/2025/9/26/around-the-world-26-biomes/)).
  - Its author tested the climate model on Earth's real relief (Earth2014).
    The model made too much cold desert across Asia, but coasts came out "a pleasing diversity of roughly accurate climate zones".
  - Köppen's climate classes did not map cleanly onto real vegetation, so he switched to **BIOME1**.

### Plates and mountains

- **Procedural Tectonic Planets** (Cortial, Peytavie, Galin and Guérin, Eurographics 2019) avoids "computationally demanding physically-based simulations".
  It approximates subduction and collision to deform the crust, producing continents, ocean ridges, ranges and island arcs, then amplifies the result with detail ([Eurographics](https://diglib.eg.org/handle/10.1111/cgf13614)).

### Erosion

- **The stream power law** of geology: rivers cut in proportion to their flow and slope, against the land's uplift.
  - Cordonnier and others (2016) generate large realistic terrains "at a low computational cost" by combining uplift with stream power erosion over a graph of streams ([Eurographics](https://diglib.eg.org/handle/10.1111/cgf12820), [paper](https://www.cs.purdue.edu/homes/bbenes/papers/Cordonier16CGF.pdf)).
  - Tzathas and others (2024) solve it analytically, with landslides and slopes added: "a slider that controls the aging of the input terrain", fast and physically consistent ([Eurographics](https://diglib.eg.org/handle/10.1111/cgf15033)).
- **FastScape** (Braun and Willett, 2013), used by geologists: an "O(n), implicit and parallel method" for the stream power law, stable with large time steps ([Landlab](https://landlab.readthedocs.io/en/latest/generated/api/landlab.components.stream_power.fastscape_stream_power.html), [GFZ](https://gfz.de/en/section/earth-surface-process-modelling/projects/current-projects/fastscape-landscape-evolution-model-development)).

### Water

- **Priority-Flood** (Barnes, Lehman and Mulla, 2014) fills hollows, as water would fill them, by flooding the map inwards from its edges with a priority queue.
  Its pseudocode is 20 lines, and it gives flow directions and watersheds too ([arXiv](https://ar5iv.arxiv.org/html/1511.04463), [reference code](https://github.com/r-barnes/Barnes2013-Depressions)).
  Rivers then follow the accumulated flow, and filled hollows become lakes.

### Climate, soils, biomes and deposits

- **Rain over mountains:** the Smith and Barstad (2004) linear model gives rain shadows from wind, terrain and moisture.
  It needs only a few Fourier transforms of the terrain, so it is fast ([AMS](https://ams.confex.com/ams/pdfpapers/76934.pdf), [HESS](https://hess.copernicus.org/articles/14/2329/2010/)).
- **Other open climate models for generated worlds:**
  - WorldSynth: winds shaped by geography, rain by wind and terrain ([Mindwerks](https://mindwerks.net/projects/worldsynth/));
  - Gleba: yearly rain and temperature statistics from topography ([itch.io](https://calandiel.itch.io/gleba));
  - genworldvoronoi: global winds ([GitHub](https://github.com/Flokey82/genworldvoronoi)).
- **Soils:** Hans Jenny's five factors (1941): climate, organisms, relief, parent rock and time ("CLORPT").
  Deeper soils gather at the foot of slopes, and soils form fastest where it is warm and wet ([Soils 4 Teachers](https://www.soils4teachers.org/formation), [TRU geology](https://environmental-geol.pressbooks.tru.ca/chapter/soil-formation/)).
- **Biomes from plants:** BIOME1 (Prentice and others, 1992) decides which plant types can grow from five numbers ([macroBiome](https://rdrr.io/cran/macroBiome/man/cliBIOMEPoints.html), [Palaeo-Electronica](https://palaeo-electronica.org/2001_1/climate/biome.htm)):
  - the coldest and warmest months' temperatures;
  - the warmth summed over the year above 0 °C and above 5 °C;
  - how well the soil's water meets the air's demand.

  Plant types then dominate in a set order.
- **Deposits by geology** (`WLD-14`):
  - **flint** forms as nodules in chalk and limestone, and is found along their streams and beaches.
    Prehistoric mines followed the chalk, as at Grimes Graves ([Wikipedia: flint](https://en.wikipedia.org/wiki/Flint));
  - **copper's great deposits** lie above subduction zones, in belts along volcanic mountain ranges such as the Andes ([Wikipedia: porphyry copper](https://en.wikipedia.org/wiki/Porphyry_copper_deposit), [PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC5353633));
  - **clay** settles in flood plains, and **obsidian** forms where lava cooled fast.

## Speed on your phone

- The heavy steps are cheap in their best-known forms:
  - FastScape is linear in the number of cells;
  - Priority-Flood needs one pass with a priority queue;
  - the rain model needs a few Fourier transforms.

  For 2 million cells, each pass is a few million operations, under a second on one core.
- So `WLD-09`'s two passes fit the 3-minute target if the tens of erosion steps and the climate are measured, not assumed:
  - **first:** about 20 candidates at a coarse size, scored and rejected as Dwarf Fortress does;
  - **then:** the best few at full size.
- **Same seed, same world** needs the rules of research 03.
  Undiscovered Worlds shows how easily this breaks.
  So the generator runs on the processor, in fixed order: no graphics-chip compute, whose rounding differs.

## What we take

1. **The generator is part of the C++ simulation, in the order of causes:**
   1. plates, as Procedural Tectonic Planets approximates them;
   2. uplift and erosion by the stream power law, FastScape-style, with slopes and glaciers;
   3. Priority-Flood for basins, lakes and flow;
   4. climate: temperature by latitude, height and season on our torus, winds, and rain shadows by a linear orographic model;
   5. soils by Jenny's factors;
   6. plant types by BIOME1's five numbers;
   7. deposits by geology: flint in chalk and limestone, copper in arcs over subduction, clay in flood plains, obsidian at volcanoes.
2. **Tuned for the look, as Red Blob advises:** the map and close-ups are judged against the art bible, not only against plausibility.
   The climate is checked on Earth's real relief, as Around The World did.
3. **Many candidates, scored and rejected with logged reasons,** Dwarf Fortress-style, the best three offered (`WLD-10`).
4. **Deterministic:** processor only, fixed chunks, the rules of research 03.
   The same seed makes the same world on the phone and in the cloud, checked by hashes.
5. **A prototype first:** generation time on your phone for one candidate at coarse and full size, and the settling run.
   Most of `WLD-11`'s risk is there.

## Sources

- Generators:
  - [Red Blob Games: mapgen4](https://www.redblobgames.com/maps/mapgen4/)
  - [Red Blob: sphere maps](https://simblob.blogspot.com/2018/10/map-generation-on-sphere.html)
  - [World Orogen](https://github.com/raguilar011095/planet_heightmap_generation)
  - [Dwarf Fortress wiki: world generation](https://dwarffortresswiki.org/index.php/DF2014:World_gen)
  - [Dwarf Fortress wiki: world rejection](https://dwarffortresswiki.org/index.php/40d:World_rejection)
  - [Undiscovered Worlds forum](https://forum.thegamecreators.com/thread/223804)
  - [Undiscovered Worlds GitHub](https://github.com/JonathanCRH/Undiscovered_Worlds)
  - [Azgaar](https://github.com/Azgaar/Fantasy-Map-Generator)
  - [Frozen Fractal: climate](https://frozenfractal.com/blog/2023/12/29/around-the-world-9-climates/)
  - [Frozen Fractal: biomes](https://frozenfractal.com/blog/2025/9/26/around-the-world-26-biomes/)
- Papers:
  - [Procedural Tectonic Planets](https://diglib.eg.org/handle/10.1111/cgf13614)
  - [Cordonnier et al. 2016](https://diglib.eg.org/handle/10.1111/cgf12820) ([paper](https://www.cs.purdue.edu/homes/bbenes/papers/Cordonier16CGF.pdf))
  - [Tzathas et al. 2024](https://diglib.eg.org/handle/10.1111/cgf15033)
  - [FastScape in Landlab](https://landlab.readthedocs.io/en/latest/generated/api/landlab.components.stream_power.fastscape_stream_power.html)
  - [GFZ: FastScape](https://gfz.de/en/section/earth-surface-process-modelling/projects/current-projects/fastscape-landscape-evolution-model-development)
  - [Priority-Flood](https://ar5iv.arxiv.org/html/1511.04463) ([code](https://github.com/r-barnes/Barnes2013-Depressions))
  - [Smith and Barstad, orographic rain](https://ams.confex.com/ams/pdfpapers/76934.pdf)
  - [HESS: linear model in Norway](https://hess.copernicus.org/articles/14/2329/2010/)
- Climate tools:
  - [WorldSynth](https://mindwerks.net/projects/worldsynth/)
  - [Gleba](https://calandiel.itch.io/gleba)
  - [genworldvoronoi](https://github.com/Flokey82/genworldvoronoi)
- Soils:
  - [Soils 4 Teachers](https://www.soils4teachers.org/formation)
  - [TRU geology](https://environmental-geol.pressbooks.tru.ca/chapter/soil-formation/)
- Biomes:
  - [macroBiome: BIOME1](https://rdrr.io/cran/macroBiome/man/cliBIOMEPoints.html)
  - [Palaeo-Electronica](https://palaeo-electronica.org/2001_1/climate/biome.htm)
- Deposits:
  - [Wikipedia: flint](https://en.wikipedia.org/wiki/Flint)
  - [Wikipedia: porphyry copper](https://en.wikipedia.org/wiki/Porphyry_copper_deposit)
  - [PMC: porphyry copper](https://pmc.ncbi.nlm.nih.gov/articles/PMC5353633)
