# Kindling α01c: Shadows and air

## What is new

- The cliff casts real shadows that follow the hour. A shadow is sharp right at the cliff and softer the farther it reaches, the way the sun's disc makes it.
- Hollows and the foot of the cliff are a little darker and bluer, where the land hides part of the sky.
- Outlines only where something stands in front of what lies behind it: the cliff's edge against the valley below, the land's edge against the dark. A slope seen edge-on gets none. Where the sun catches such an edge, it shines instead of darkening.
- Haze that grows with the real distance through the air, warmer toward the sun and cooler away from it. It is built and tested, but too faint to see yet (see below).
- Behind the scenes: a counter for the pixels that "crawl" (change colour while the land under them hardly moves) as the camera turns or zooms, so the fix chosen at the first visual review can be measured; and test pictures of the valley at five hours.

## What to try

1. Tap **Download and install** at the top of this page. It installs over α01b. Open Kindling.
2. It opens in the late afternoon: the cliff throws a long shadow east across the valley floor, crisp near the cliff and softer far from it. The cliff's foot and the hollows are a little darker and bluer.
3. Tap the strip: at noon the shadows are short; at 18:30 the sun is down and nothing casts a shadow; at night the valley is dark.
4. Pinch slowly in and out: the shadows, the light and the cliff's outline stay on the land; only the pixel grid resamples.
5. Look at the cliff's top edge in the late afternoon: a dark line where it stands in front of the valley below. At 06:30, with the sun on the cliff's side, that edge shines instead.
6. Open the web link: the same in the browser, moved with the mouse.
7. If a box with a code appears, tap Copy and paste the code into your reply.

## What is rough

- You can't see the haze yet. The land in reach is under a kilometre away and over 300 m up, where the air's haze stays under 8 in 100, and the picture shows haze only from 10 in 100. It shows when the zoom reaches out over the island (α02a).
- While you turn or pinch slowly, about 3 in 100 pixels crawl each frame at the camp view, and 2 in 100 close up; a drag moves whole pixels and crawls not at all. Choosing the fix for this is part of the first visual review, where you will see the candidates side by side.
- Working out the light takes longer than planned. The share of open sky over each point (the darker hollows) takes about 0.2 seconds when the land loads, ten times the plan's 20 ms; it is done once. Each tap of the strip works out the new shadows in about 15 ms, seven times the plan's 2 ms; you may see one frame skip. When the clock runs (α03a), the shadows will follow the sun in small steps spread over frames, 2 ms each, which this alpha already builds and tests.
- The app is a little larger: the download is 763 KB, up from 710 KB, and the web page's code 558 KB, up from 488 KB, with the light's new code.
- The cliff is still drawn as a steep slope of 1 m squares, so close up its top edge shows 1 m steps and the shade down its face has saw-tooth edges; real cliff faces come with α02c, as do shadows from things. Up close the ground is still plain colour (α01d), and beyond the piece of land is dark (α02a).

## IDs delivered

`PRE-30` (part: real shadows by the hour, sharp near and softer far; darker hollows; haze by distance, warmer toward the sun), `PRE-21` (part: outlines only where one thing stands in front of another; lit edges), `PRE-22` (part: light and shading holding still while zooming; the crawl slot and its counter), `PRE-02` (part).

## Links

- APK: https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk
- Web: https://claude.ai/artifact/NmypTQyKQUAFZs18TNJELH
- Note: https://claude.ai/artifact/GBackmSHJPak61yAd6we4d
