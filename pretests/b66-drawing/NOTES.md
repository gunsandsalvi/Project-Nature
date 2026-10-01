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
- **Steps:** the turn snaps to steps that move the left and right edges of the view by one art pixel; the zoom snaps to steps that move the top and bottom edges by one art pixel.
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

(to fill in)

## Results

(to fill in)

## Verdict so far

(to fill in)

## Caveats

(to fill in)

## For the owner

(to fill in)

## How to re-run

(to fill in)
