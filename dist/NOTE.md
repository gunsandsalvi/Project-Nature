# Kindling α01b: The ground

## What is new

- A real piece of land, 256 m across, drawn in pixel art: hilly grass with patches gone dry, a 30 m cliff of pale rock crossing it, and scree at its foot. It is lit by the same light model as the light card, at the strip's hour.
- You can move the camera: drag to move the land, pinch to zoom from a few metres up to the whole piece, twist with two fingers to turn, and double-tap then drag to zoom with one thumb. The land stays under your fingers, and a quick drag glides to rest.
- Every drag moves the picture by whole pixels, so nothing shimmers while you move.
- As you zoom, the ground's shape blends smoothly between levels of detail instead of jumping.
- Behind the scenes: positions on the wrap-around world; and the ground is made exactly the same on every device. Your phone checks this when it starts.

## What to try

1. Tap **Download and install** at the top of this page. It installs over α01a. Open Kindling.
2. You should see a valley floor of grass, a pale cliff crossing it and scree at its foot, in late-afternoon light.
3. Drag: the land moves under your finger and glides to rest when you let go quickly.
4. Pinch in to a few metres and out to the whole piece; twist with two fingers. The land stays under your fingers.
5. Pinch slowly from close to far: the ground's shape blends smoothly, with no sudden jumps.
6. Tap once, then touch again and drag down to zoom in, up to zoom out.
7. Tap the strip at the bottom: the light steps through the eight hours over the land.
8. Turn the phone: the land fills the screen either way.
9. Open the web link: the same land in the browser, moved with the mouse.
10. If a box with a code appears, tap Copy and paste the code into your reply.

## What is rough

- No shadows yet. In the late afternoon the cliff's face is in its own shade, but it casts no shadow across the valley. Shadows, darker hollows, outlines and haze come next (α01c).
- Close up, the cliff's top edge shows 1 m steps, because cliffs are still drawn as a height map. Proper cliff shapes come with α02c.
- Up close the ground is plain colour, without stones, tufts or texture (α01d).
- Beyond the piece of land is dark void. The island around it comes in α02a.
- Two timings grew by more than a tenth since α01a, both as expected: building the app takes 50 seconds, up from 40, with the new code; and loading the catalogue takes 2.2 millionths of a second, up from 1.5, as it now holds the ground's surfaces. You won't notice either.
- The gestures use the design's thresholds: a drag starts after a small move, a pinch after the fingers' distance changes by 6%, and a twist after 6 degrees. A second finger landing more than 0.15 s after the first is ignored. Tell me if any of this feels wrong.

## IDs delivered

`PRE-02` (part: a real 3D ground drawn at low resolution), `PRE-03` (part: the person, close camp and camp stops; the pitch), `PRE-20` (part: surfaces in their looks' steps), `PRE-22` (part: snapping, whole-pixel pans, turns easing to rest), `PRE-33` (part: drag, pinch, twist and double-tap drag, following the fingers), `PRE-34` (part: both orientations), `WLD-01` (part: positions on the wrap-around world), `WLD-12` (part: an area's ground to the metre).

## Links

- APK: https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk
- Web: https://claude.ai/artifact/NmypTQyKQUAFZs18TNJELH
- Note: https://claude.ai/artifact/GBackmSHJPak61yAd6we4d
