# P2 A full scene

The second prototype (IMPLEMENTATION α0.2b, research 00) asks: does a busy camp hold 60 frames a second on your phone,
at night with three fires, and how fast does the phone heat?

Items it is about: `PRE-27` and `PRE-44` (figures, moving in steps), `PRE-28` (tiny figures at camp zoom), `PRE-30`
and `MAT-18` (fires lighting what stands near them), `PLT-01` and `PLT-04` (measured on the phone).

- **The scene** is P1's close camp (`look/`), and the screen extends P1's, so it draws the same way, with P1's
  answers: outline D and the "rest" crawl fix.
- **The camp:** thirty stand-in block figures, built at load as one instanced shape; eighteen walk between spots and
  twelve bend at their work, all moving in steps of a tenth of a second. Two tents and two more fires stand by the
  shelter.
- **Fire light:** our firelight term in the shared light function, fed by the fires' list, or Godot's own omni lights,
  to see whether the instanced figures stay lit.
- **The forest:** 12,000 instanced stand-in trees 1.2 km across, on a wider plain, round the painter's scene, seen at camp
  zoom, a metre an art pixel.
- **Measure** runs about 90 seconds: the close camp at 120 frames a second, then the close camp and the forest at 60,
  the camera turning, reading Android's forecast of the phone's heat every 10 seconds through Godot's Android runtime.
