# P1 The look (α0.2a)

The question (IMPLEMENTATION α0.2a): can Godot's Mobile renderer on your phone draw the art book's close camp at its
look within the frame's budget, and which outline method and crawl fix should the game use?

Items it is about: `PRE-01` and `PRE-02` (pixel art drawn from 3D), `PRE-20` (colour in steps), `PRE-21` (outlines and
lit edges), `PRE-22` (stable pixels), `PRE-26` (water), `PRE-30` (light by the hour), `PRE-31` (the look judged
beside the art book), `PLT-04` and `VIS-14` (measured on the phone).

- **The scene** is the art book's close camp itself: the painter (`art/book/paint`) exports it, 50 m wider than its
  picture so the camera can turn, `make_scene.py` packs it and `build_scene.gd` builds `close_camp.scn`;
  `make-scene.sh` does all three. The painter's own plate is unchanged by the export.
- **The look** is the painter's, ported to Godot's forward Mobile renderer (`shaders/`): each material works out its
  material, pattern, sky and fire light, and its light function adds the sun after its shadow, rounds the light to a
  step of the material's ramp, gives it the hour's colour and the haze. The sky's share under an overhang comes from a
  map of heights drawn once from above.
- **The switches** on the screen: the hour (noon, dusk); the outline method (none; A, normals rebuilt from depth;
  B, depth only; C, a second camera drawing normals and depths; D, enlarged back faces); the mirrored water; and the
  crawl fix for free turns and zooms (free; whole steps of 15° and 1.25 times; easing to rest on those steps;
  snapping to the grid only at rest).

Like every prototype it is thrown away once its answer is written into the architecture.
