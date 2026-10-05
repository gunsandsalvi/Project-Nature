# P7 World generation

The seventh prototype (IMPLEMENTATION α0.5a, research 06) asks: how long do a candidate world and the settling run
take on your phone?

Items it is about: `WLD-11` (three worlds offered within about 3 minutes, settling about 1 more), `WLD-08` (made
directly in a present-day state, then settled for 10 years), `WLD-09` (the stages in the order of real causes),
`WLD-10` (about 20 candidates, the best few at full size, the best three offered, with logged reasons) and `WLD-24`
(where history begins). It also follows `WLD-06` (tilt and share of land), `WLD-14` (deposits), `WLD-16` (climate),
`WLD-17` (rivers reach the sea or a lake) and `RES-05` (the same results everywhere).

- **The stages** (A7.2), each over the torus's cells (`WLD-01`), 2,048 × 1,024 cells of about 1 km at full size and
  512 × 256 for the candidates:
  - **Plates** (`src/land.cpp`): 6 to 12, each cell to the nearest seed by a warped distance; continental plates lean
    their area to land and oceanic to sea, blurred over about 150 km before a noise field draws the coasts; where
    plates meet, a range, a trench and an arc of volcanoes, a rift or ridge, or a fault, each with its uplift. The
    sea's level is set so the land's share is the seed's own, 27% to 48% (`WLD-06`).
  - **Rock:** a surface and two layers from each cell's place: worn-down shields, old sea basins (the only chalk and
    limestone), folded ranges, volcanic arcs, rifts and sea floor (`WLD-09`).
  - **Uplift and erosion:** the stream power law solved implicitly from the sea upward (Braun and Willett 2013), its
    rivers routed by Priority-Flood (Barnes 2014), so every river reaches the sea or a lake; then silt on flood
    plains and at river mouths, and gravel in valleys and fans.
  - **Climate** (`src/climate.cpp`): warmth by latitude, height and season, tilt and distance inland; the wind belts;
    rain by latitude, distance from the windward coast, and the linear model of rain over mountains (Smith and Barstad
    2004), run through our own Fourier transform (`src/fft.cpp`) for the trades and westerlies of each half; then
    BIOME1's numbers, month by month through a soil bucket.
  - **Life** (`src/life.cpp`): biomes, soils and their fertility, dry caves and overhangs, deposits where the rocks
    put them, flint and chert carried down the rivers' gravel, wild grains, and herds where their food and cover are.
- **Candidates** (`src/choose.cpp`): each world's start region is found by scoring (`WLD-24`): shelters where winters
  average 2 to 10 °C, with water within about 2 km and, within about 10 km, food for a band in the leanest season with
  a margin, and stone that flakes; at least three together. Its landmass must hold the arc's needs (`WLD-10`). Every
  candidate's reasons are logged. The best few that qualify are made again at full size from their worn land, and the
  best three offered.
- **Settling** (`src/settle.cpp`): plant cover, water and herds for 10 years at the play paces, with no people. Its
  rules are stand-ins of about the work production's will do, for the time it takes.
- **The measure** (`src/cli.cpp`, and the app's "P7 World generation" screen through `extension/`): each part's time,
  the maps (`src/maps.cpp`, in the art book's colours) and a digest of the offered worlds.

It uses P5's maths, keyed chance, hashes and arm64 check (`prototypes/samebits`) and P6's thread pool
(`prototypes/minds/src/pool`), written once.

    cmake -S prototypes/worldgen -B build/worldgen -G Ninja && cmake --build build/worldgen
    build/worldgen/worldgen_cli 4 full 1 maps   # four threads, full size, seed 1, the maps into maps/ as PPM

Like every prototype it is thrown away once its answer is written into the architecture (A7.6).
