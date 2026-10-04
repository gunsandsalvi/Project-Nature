# Kindling α0.2a: P1 The look

## What is new

- The first prototype, **P1 The look**, asks one question: can Godot draw the art book's look on your phone fast enough, and which way of drawing outlines and which fix for crawling pixels should the game use?
- Tap it in the menu and Godot draws the art book's close camp, from the same scene the art book was painted from: the cliff, the camp under the overhang, the stream and the meadow.
  - Light falls in clean steps, with hard sun shadows, purple-blue shade, haze in the distance, the fire's glow and the hearth's smoke, at noon or at dusk.
  - One finger pans; two fingers turn and pinch to zoom.
- **Outline** switches between five ways of drawing the dark outlines and the bright sunlit edges:
  - **none**;
  - **A**: rebuilt from each pixel's depth, with lit edges;
  - **B**: from depth alone, outlines but no lit edges;
  - **C**: from a second small picture of each pixel's facing and depth, the art book's own way;
  - **D**: each shape drawn again slightly larger behind itself, so outlines run round shapes only.
- **Mirror** shows what stands above the stream reflected in it. Either way the stream is clear water: the bed shows in the shallows, deeper water darkens in steps, and a bright line marks the shore.
- **Crawl fix** switches between four ways of keeping pixels from shimmering as you turn and zoom:
  - **free**: the view locks to whole pixels as it pans, and turns and zooms freely;
  - **steps**: turns in steps of 15° and zooms in steps of 1.25 times;
  - **ease**: turns and zooms freely, then eases to the nearest step when you lift your fingers;
  - **rest**: moves smoothly, and locks to whole pixels only when it stops.
- **Measure** pans, turns and zooms by itself: ten runs of 7 seconds, each outline way with and without the mirror. Then it copies a short line of the graphics times for the chat.
- The line above the buttons shows the frames a second and the graphics time as you go.

Godot's pictures below come from the cloud, drawn in software by the same renderer your phone uses, at the art book's size. Each sits beside the art book's picture.

![The close camp at noon: Godot with outline C (left) and the art book (right)](pictures/p1-noon.png)

![The same at dusk](pictures/p1-dusk.png)

![The camp under the overhang at twice the size: none and A (top), B and C (middle), D and the art book (bottom)](pictures/p1-outlines.png)

![The stream at three times the size, without the mirror (top) and with it (bottom)](pictures/p1-mirror.png)

## What to try

1. Tap **Download and install** at the top of this page. It installs over α0.1a.
2. Open Kindling and tap **P1 The look**.
3. Tap **Measure** and don't touch the screen for about 70 seconds while the view moves by itself. When it shows "Copied for the chat", paste the line into your reply.
4. Then look round yourself, comparing with the pictures above:
   - tap **Outline** to go through none, A, B, C and D, and **Hour** for noon and dusk;
   - tap **Mirror** to see the stream's reflections;
   - tap **Crawl fix** to go through free, steps, ease and rest, and turn and zoom with each, watching the edges for shimmer.
5. Tell me:
   - which outline way looks closest to the art book;
   - which crawl fix you prefer;
   - anything that looks unlike the pictures above; a screenshot (power and volume down) helps.

## What is rough

- **The cloud's times mean little:** it draws in software, at about 300 ms a frame where your phone should take a few, so they only compare the ways: against no outlines, A and B cost about the same, D about a tenth more and C about a third more, and the mirror adds about a quarter. Your phone's line is the answer: P1 passes if one way draws the look in under about 8 ms of graphics time while panning.
- **How close it is:** 87% of the pixels at noon and 76% at dusk are within 3 levels of 255 of the art book's, too little to see. Most of the rest are shadows, outlines and edges a pixel apart, and one thing more:
  - at dusk, the long shadows across the meadow's lower left are cast by trees beyond the art book's frame, added round the scene so that you can pan; the art book has no trees there.
- **Nothing moves:** the people, the fire and the smoke stand still; life comes with later prototypes.
- **The scene ends:** it reaches about 50 m beyond the art book's frame, so pan far and the land stops.
- **Measure skips each run's first second,** so a pause while it switches ways doesn't count.
- **Plain controls:** Godot's own font and plain buttons; the art book's interface comes with P12 (α0.7a).

## IDs delivered

None for good: P1 is a prototype, thrown away once it has answered.
Its question is about `PRE-01`, `PRE-02`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-26`, `PRE-30`, `PRE-31`, `PLT-04` and `VIS-14`.
Your line from Measure and your choices go into the architecture (A4.1, A4.2): the game's way of drawing outlines and its crawl fix.

## Links

- APK: https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk
- Note: https://claude.ai/artifact/GBackmSHJPak61yAd6we4d
