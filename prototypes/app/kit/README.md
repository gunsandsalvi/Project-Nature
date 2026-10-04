# P3 The kit

The third prototype (IMPLEMENTATION α0.2c, research 00) asks: do the kit's shapes, built by code at load, read well as
the art book's sheets do, with the figure's movements and a hut in two materials, at noon and at night?

Items it is about: `PRE-46` (shapes built from parameters at load), `PRE-27` and `PRE-44` (the block figure, its
movements as key poses stepped about 10 times a second, its face), `PRE-42` and `PRE-43` (one hut in two materials,
worn), and `PRE-30` (fire: its light, its shadows and its smoke).

- **The catalogue** (`catalogue.json`) holds each shape's parameters: eleven shared shapes, two plants and a deer, each
  in two materials, and the figure's proportions and its five movements, walk, carry, knap, scrape and rest, as key
  poses of joint angles. `kit_shapes.gd` builds them at load in the look's format (`look/shape.gd`).
- **The model sheet** (`kit.gd`) lays them out in rows on a meadow of their own, with the painter's palette, light and
  look, and draws them as P1 and P2 do: outline C and the "ease" crawl fix. The hour button gives noon, dusk and night.
- **The hut** is one shape drawn as two copies, of birch bark and of reed, each carrying its own material, pattern and
  wear in its per-copy data, as the game's models will be (A6.2).
- **Three fires:** one in a hearth behind the figures, one before them, and one under a lean-to's roof. Each lights
  what stands near it, and people and things cast a shadow from each, from two maps of the heights round the fires:
  their tops seen from above and their undersides seen from below, so light passes under a roof but not through a
  tent or a person. Each fire's smoke is a lit volume: under the lean-to it flows out beneath the roof, then rises.
- **Measure** runs about 40 seconds: the sheet at night at 120 frames a second, then at night and at noon at 60.
- On the command line: `kit` opens it, `hour=night` sets the hour, `close` looks at the figures up close, and
  `shot=<png>` saves the picture, for the cloud's pictures.
