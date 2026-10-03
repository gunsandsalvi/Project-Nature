# Research 03: drawing 3D as pixel art

**Question:** how do the people behind your reference pictures get that look, and what must our renderer do to match it?

## The recipe others use

1. **Draw the 3D scene small, then scale it up.**
   The scene renders into a low-resolution image (640 × 360 for David Holland and for a Unity 6 pipeline built on t3ssel8r's ideas; 480 rows in another Unity guide), then each pixel is blown up to a block of screen pixels ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/), [JoonyoungSeo](https://github.com/JoonyoungSeo/unity-isometric-pixel-pipeline), [smallnightlight](https://smallnightlight.github.io/Pages/Page1/pixelart3d.html), [Lettier](https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html)).
   Your project file sets one art pixel at about 4 × 4 screen pixels (`PRE-22`).
2. **A straight-on (orthographic) camera, locked to the pixel grid.**
   The camera moves only in whole art pixels, so nothing shimmers while it pans.
   The leftover fraction is added back as a tiny shift of the final image, so panning still looks smooth ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/), [JoonyoungSeo](https://github.com/JoonyoungSeo/unity-isometric-pixel-pipeline)).
   This only fully works with an orthographic camera; with perspective, things at different depths drift at different speeds.
   A March 2026 paper, "Texel Splatting", fixes perspective, at the cost of gaps where its fixed viewpoint can't see ([arXiv](https://arxiv.org/abs/2603.14587)).
   Turning and zooming still cause some crawl, which your project file already accepts (`PRE-22`).
3. **Outlines from depth and normals, one pixel wide.**
   After the scene is drawn, a pass compares each pixel with its 4 neighbours (up, down, left, right):
   - a jump in depth darkens the nearer pixel, making the outline;
   - a sharp change of surface direction on an outward (convex) edge lightens it, making the bright rim.

   t3ssel8r's rims "only appear on the convex edges" ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).
   Open examples: Kody King's three.js pixel pass ([three.js](https://threejs.org/examples/webgl_postprocessing_pixel.html)) and Leo Peltola's MIT-licensed Godot shader ([GitHub](https://github.com/leopeltola/Godot-3d-pixelart-demo)), which does exactly this.
   This is your `PRE-21`.
4. **Light in a few steps (toon or cel shading).**
   The light on a surface is cut into about 3 bands; a 4 × 4 Bayer pattern can soften the change between bands ([JoonyoungSeo](https://github.com/JoonyoungSeo/unity-isometric-pixel-pipeline)).
   This is your `PRE-20`'s ladder of shades.
5. **Real shadows from a shadow map, softened against flicker.**
   Holland smooths the sun's falloff so a slowly moving sun doesn't make shadows flicker, and adds a little noise to the normals to stop shadows popping on flat ground.
   Cloud shadows are a scrolling noise texture over everything ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).
6. **Grass is thousands of small flat cards.**
   Each card carries a grass picture, turns to face the camera, takes the colour and shadow of the ground at its foot so it blends into the terrain, and sways with noise-driven wind (about 35,000 in the Unity demo) ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/), [JoonyoungSeo](https://github.com/JoonyoungSeo/unity-isometric-pixel-pipeline)).
   A spawn map keeps grass out from under rocks and buildings ([Unity forum](https://discussions.unity.com/t/recreating-t3ssel8rs-3d-pixel-art/928878/2)).
7. **Trees are leaf cards on a canopy shape.**
   Leaf cards are scattered over the canopy's surface by Poisson-disk sampling, so they are spread evenly, and are lit like the grass ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).
8. **Water reflects and refracts.**
   It has a depth-tinted colour, orthographic planar reflections, foam where it meets the shore, and a wave texture.
   Its outline must still work in the see-through pass ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).
9. **Extras:** god rays by marching through the shadow map (about 1.7 ms on a GTX 1060 laptop, so probably too dear for a phone), hand-animated 2D effects for rain, splashes and dust, and a vignette and film grain at night ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).

## What this means for Kindling

- **The look comes from real 3D models.**
  Ground, rocks, cliffs, trees, grass, people and huts are all models drawn by this recipe.
  Our old renderer had no models; it shaded the ground and painted the trees on it.
- **The renderer must let us write our own passes** (outlines, grass, water, the grid-locked camera).
  Even in Godot, Holland had to change the engine's source to get shadows into the grass and outlines onto water.
- **Your phone's PowerVR chip** works better on Vulkan than on OpenGL ES (research 01).
  Its graphics memory is tiled, so we keep the full-screen passes few, since each one reads the depth and the image back.
- **Not covered by any reference:** your zoom out to the valley, the map and the globe (`PRE-03`, `PRE-29`).
  That needs levels of detail: grass only near the camera, simpler trees farther out, and the map look beyond.
  This belongs to the world-generation research (05) and the renderer design.

## The bake-off scene (what both engines must draw)

Built from the same files in both engines:
- **The ground:** about 48 × 48 m with a 3 m cliff step, a meadow, a dirt path and a stream (one generated mesh with its colours).
- **On it:** rocks, a few trees and bushes, flowers, a campfire and a tent, from Kenney's free Nature Kit (CC0, free for any use) ([Kenney](https://kenney.nl/assets/nature-kit)).
- **The recipe:**
  - steps 1 to 6 as above;
  - the water of step 8, at least its tint, foam and reflection.
- **Control:** drag to pan, pinch to zoom, twist to turn, at 4 × 4 screen pixels an art pixel, with a frame-time readout.

**Judged by:**
- you, by eye on your phone, against your reference pictures;
- the frame time on your phone;
- the size of the app;
- how hard each engine was to build in.
