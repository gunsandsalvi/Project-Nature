# B66: drawing and gestures

Pre-test. Throwaway: deleted once the architecture is written.

## The question

1. Can WebGL in a browser draw the decided look (`PRE-01` to `PRE-04`, `PRE-20` to `PRE-22`) at the phone's full refresh rate, at every zoom (`VIS-14`)? Or do we need a native engine (Vulkan or OpenGL ES)?
2. How do we stop pixels crawling while the camera turns or zooms (`PRE-22`)?
3. Do the decided gestures (`PRE-33`) feel right in the hand?

The scene is the mockup's camp, copied into `drawing-test.html`. No game systems.

## The approaches

**Speed (part 1).** Measure first, build nothing native. A Benchmark button drives the camera through a fixed 60-second sweep and shows a result code to paste back.

**Crawl (part 2).** Each one is a switch in the page ("Pixel fix"):
- **Base:** the mockup as it is.
- **Steps:** the turn snaps to steps in which a point half a view-width from the turning axis moves one art pixel; the zoom snaps to steps that move the far edges of the view by one art pixel.
- **Fade:** bigger fixed steps (about 1.5° of turn, 4% of zoom), with a brief dithered crossfade between the old and new picture.
- **Majority:** the scene is drawn at twice the resolution (4 samples per art pixel), and each art pixel takes the palette colour most of its samples agree on. A variant, **Sticky**, keeps last frame's colour while at least half the samples still agree with it.

**Gestures (part 3).** All decided gestures, with plain stubs, in the same page.

## Decision rules (written before measuring; not changed afterwards)

**Speed: WebGL or native.**
- The refresh rate is estimated from requestAnimationFrame while nothing is drawn.
- A phase (each zoom stop, the full turn, the zoom sweep) passes if it delivers at least 90% of the refresh rate and no more than 5% of frames come late (longer than 1.5 refresh periods).
- If every phase passes in the base drawing: WebGL is fast enough. No native engine.
- If any phase fails: a native engine is justified. It is not built now; the cheapest route and its cost are written down (below).

**Crawl: which fix.**
- Crawl is counted headless, frame by frame with a fixed time step: art pixels that change colour where the surface under them moved less than one art pixel since the last frame. Two motions at the camp view: a slow turn and a slow zoom.
- A fix must cut crawl by at least half against base, in both the turn and the zoom.
- Among fixes, the lowest crawl wins, unless another fix is within 20% of it and is cheaper on the phone, or you find it looks better in motion. Your eye on the phone has the last word between close ones (`PRE-22` is about the look).
- A fix that cuts crawl but raises the total number of changed pixels over the same motion by more than 20% is flagged "jumpy" for your eye.
- If nothing cuts crawl by half in both motions, `PRE-22` stays open and the next step is named.

**Gestures.** You mark each gesture fine, awkward or broken. Anything broken, and any clash with Android's own gestures, is recorded for the architecture. `PRE-33` doesn't change without your OK.

## Method

- **The page.** `drawing-test.html` is the mockup plus a Benchmark button, the Pixel fix switch and a full-screen "Try the gestures" mode. With Base chosen, it draws exactly as the mockup does.
- **Benchmark** (about 60 s, full screen, every frame drawn): 2.5 s to measure the refresh rate with almost nothing drawn; each zoom stop (Person, Camp, Valley, Region, Planet) for 7 s while turning slowly; a full turn in 8 s; a zoom out to the planet and back in 8 s; then 6 s of the Sticky fix, to price it (not part of the verdict). The first second of each phase is warm-up. Frame times come from requestAnimationFrame. The code also carries the script time per frame and, if the browser allows it, the graphics chip's time per frame.
- **Crawl count** (headless Chromium, WebGL on the CPU): the phone's screen in portrait (1344 × 2992 device pixels, 5 per art pixel, so 269 × 599 art pixels), scene animation frozen so only the camera moves, 60 frames at a 60 Hz step per motion: a slow turn (0.11° a frame), a slow zoom (0.3% a frame), and a slow pan as a control (under 0.2 art pixels a frame; snapping should leave nothing). Each art pixel is compared with the one at the same place on screen in the next frame; the surface it shows is re-projected from the depth buffer to see how far it moved.
- **Gestures:** a headless smoke test drives each one with synthetic touches. That checks the wiring only; the feel is yours to judge.

## Results

**Speed on the phone** (first test app, 1 October; `pretests/b78-b79-phone/results/phone-r1.json`, key `wv`). The mockup as it is, in an Android WebView, for 30 s: half a turn and two zoom sweeps from close up to the valley.
- 120 Hz screen; 3,543 frames, about 118 a second: 98% of the refresh rate.
- Frame interval: median 8.3 ms, 99% under 16.6 ms, worst 58.3 ms. 51 late frames (1.4%).
- The page's script took 2.0 ms a frame (99% under 4.4 ms) of the 8.3 ms available.
- A plain OpenGL ES scene in the same app: 0.2% late, 99% under 10.6 ms.
- Drawn at 224 × 279 art pixels (the mockup's box, not the full screen). Graphics: PowerVR, through ANGLE on Vulkan.

**Crawl** (`results/crawl-*.csv`; 60 frames per motion, 269 × 599 art pixels):

## Verdict so far

(to fill in)

## Caveats

- Speed here means nothing: the cloud draws on the CPU. Part 1 waits for your code.
- The benchmark runs in the browser, perhaps inside the claude.ai frame. The code says whether it ran full screen (`fs=1`) and in a frame (`if=1`), and the drawing size (`art=`). The app's own WebView (`B78`) should behave about the same; a native app avoids the browser's compositing step.
- 60 seconds says nothing about heat or battery (`VIS-14`); `B79` measures those.
- The crawl count rewards big jumps: when a turn or zoom moves most pixels by more than one art pixel at once, those changes don't count. Steps and Fade work exactly that way, so their low counts must pass your eye, especially against "turns ease to rest" (`PRE-22`).
- One scene (the camp at dusk), one turn speed and one zoom speed; people, fire and smoke were frozen in the count.
- In Fade, a fast turn makes steps faster than the 80 ms crossfade, so the picture mixes the last two steps.
- Android owns a thin strip at the very bottom of the screen (its home gesture). "Swipe up from the bottom edge" has to start a finger's width above it, in the browser and in an app alike.

## For the owner

1. Open the page on the phone, in Chrome if you can. Wait until it says "Ready".
2. **Benchmark:** tap Benchmark, put the phone down and don't touch it for about a minute (a touch stops it). Then tap "Copy code" and paste the code back to me. If copying is blocked, press and hold the code, Select all, Copy.
3. **Pixel fix:** at the Camp zoom, pick Base, then Steps, Fade, Majority and Sticky. With each, turn slowly (drag sideways on the picture) and zoom slowly (the slider). Tell me which one looks steadiest, and whether any of them feels jerky.
4. **Gestures:** tap "Try the gestures". The world fills the screen. Try each, and mark it fine, awkward or broken:
   - drag with one finger to move; twist two fingers to turn (let go mid-twist: it should ease to rest);
   - pinch to zoom; or tap twice and keep the thumb down on the second tap, then drag down to zoom in and up to zoom out;
   - tap something: a yellow square marks it, and a line says what it is;
   - press and hold: a small menu opens; choose "Draw an area", then draw a loop with one finger;
   - swipe up from just above the bottom edge: a Views panel opens;
   - any brief touch: the date, the speed of time and the time control show for a few seconds.
   Try it upright and sideways. To leave, use the back gesture or "Leave the test" in the Views panel.

## How to re-run

From `pretests/b66-drawing`, each under the shared CPU lock:
- `node tools/crawl.mjs --motion turn`, then `--motion zoom`, then `--motion pan` (about 5 minutes each). Results go to `results/`.
- `node tools/smoke.mjs` (about 2 minutes): a shortened benchmark and every gesture, headless.

Both need Playwright for Node and its Chromium; set `PLAYWRIGHT_PATH` and `CHROMIUM_PATH` if they aren't at the paths at the top of each tool.
