# Kindling α0.2b: P2 A full scene

## What is new

**This is α0.2b's second build,** after your first try:
- **Our fire light works on everything near a fire:** the figures and tents by the fires are lit now. The shaders worked out each moved shape's place wrongly, so only the ground got fire light.
- **Outline C, without its flicker:** its outline data was drawn a frame late, so outlines jumped whenever the camera moved. Now it's drawn first. C is the art book's own way: outlines on every shape, tree crowns too, and bright edges where the sun or a fire catches a shape.
- **Crawl fix "ease":** panning locks to whole pixels, and a turn or zoom settles on a whole step when you lift your fingers.
- **Lighter trees at camp zoom:** 12 triangles each instead of 44, after the forest view took 17.8 ms a frame on your phone.

What P2 is: P1's close camp, alive, at night. Thirty stand-in figures walk and work round three fires and two tents. **Fire light** switches between our fire light and Godot's built-in lights. **View** zooms out to 12,000 trees. **Measure** takes about 90 seconds and copies one line for the chat.

![The camp at night: the fires light the figures, the tents and the ground near them](pictures/p2-night.png)

## What to try

1. Tap **Download and install** at the top of this page. It installs over the first build.
2. Open Kindling, tap **P2 A full scene**, then **Measure**, and don't touch the screen for about 90 seconds. Paste the line it copies into your reply.
3. Pan and turn round the fires: the figures near them should glow warm, and the outlines should hold still as the view moves.

## What is rough

- **Your first build's numbers** (4 October): the camp at night cost 6.1 ms a frame at 120 frames a second, every frame on time, so it fits the 8 ms aim. The forest took 17.8 ms with 65% of frames on time, hence the lighter trees. The phone's heat forecast went from 0.56 to 0.59 of the way to slowing itself, so no heat trouble.
- **Stand-ins:** the figures are plain blocks without faces, and the trees simple shapes; the real shapes come with P3, the kit (α0.2c).
- **The forest's edge:** the art book's scene sits in the middle on its own ground, so a faint square shows round it at camp zoom.
- **Godot's lights** don't step like the art book's colours; they are there only to compare.

## IDs delivered

None for good: P2 is a prototype, thrown away once it has answered.
Its question is about `PRE-27`, `PRE-28`, `PRE-30`, `PRE-44`, `MAT-18`, `PLT-01` and `PLT-04`.

## Links

- APK: https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk
- Note: https://claude.ai/artifact/GBackmSHJPak61yAd6we4d
