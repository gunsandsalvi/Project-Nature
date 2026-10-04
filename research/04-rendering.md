# Research 04: drawing 3D as pixel art, in Godot, on your phone

**Question:** how do the makers of your reference pictures get that look?
How much of it can Godot's Mobile renderer do on your phone, and at what cost (`PRE-01` to `PRE-30`, `VIS-14`)?

## The recipe others use

1. **Draw the 3D scene small, then scale it up.**
   - The scene renders into a low-resolution image: 640 × 360 for David Holland and for a Unity pipeline built on t3ssel8r's ideas; 480 rows in another guide.
   - Each pixel is then blown up to a block of screen pixels ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/), [JoonyoungSeo](https://github.com/JoonyoungSeo/unity-isometric-pixel-pipeline), [smallnightlight](https://smallnightlight.github.io/Pages/Page1/pixelart3d.html), [Lettier](https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html)).
   - A Short Hike went further: 160 × 140, flat shading, no anti-aliasing, soft outlines, and a palette sampled from photos of the Canadian Shield in autumn ([PlayStation Blog](https://blog.playstation.com/2021/08/05/crafting-a-tiny-open-world-a-look-behind-the-scenes-at-the-creation-of-a-short-hike/), [Wikipedia](https://en.wikipedia.org/wiki/A_Short_Hike)).
   - Ours is one art pixel to 4 × 4 screen pixels, 336 × 748 in portrait (`PRE-22`).
2. **An orthographic camera locked to the pixel grid.**
   - The camera moves only in whole art pixels, so the same pixels render the same colours as it moves.
   - The leftover fraction is added back as a shift of the final image, so panning stays smooth ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/), [UPixelator docs](https://github.com/Radivarig/UPixelator_Documentation)).
   - This only fully works with an orthographic camera.
     With perspective, pixels at different depths drift at different speeds.
   - Texel Splatting (March 2026) fixes perspective and rotation by rendering into a cubemap from a fixed point.
     It leaves gaps where that point cannot see ([arXiv](https://arxiv.org/abs/2603.14587)).
   - Turning and zooming still crawl a little, which `PRE-22` accepts ([Godot forums](https://godotforums.org/d/36180-subpixel-snapping-in-a-3d-pixel-art-game), [Unity discussions](https://discussions.unity.com/t/help-with-pixel-swimming-in-3d-pixel-art-game/952547)).
3. **Outlines and lit edges, one pixel wide** (`PRE-21`).
   A pass compares each pixel's depth and normal with its four neighbours:
   - a jump in depth darkens the nearer pixel, making the outline;
   - a convex fold lightens it, making the bright rim.

   Holland isolates convex edges by "taking the cross product of neighbouring texels".
   Open examples: Kody King's three.js pass ([three.js](https://threejs.org/examples/webgl_postprocessing_pixel.html)) and Leo Peltola's MIT-licensed Godot shader ([GitHub](https://github.com/leopeltola/Godot-3d-pixelart-demo)).
4. **Light in a few steps (cel shading)** (`PRE-20`).
   - The light on a surface is cut into about three bands, with a 4 × 4 Bayer pattern where two bands meet.
   - In Godot this is a `light()` function using the light's `ATTENUATION` (shadow included), with a set number of cuts ([Godot Shaders: flexible toon](https://godotshaders.com/shader/flexible-toon-shader-godot-4/), [ultimate toon](https://godotshaders.com/shader/ultimate-toon-shader/)).
   - Valve's Team Fortress 2 shows why: shading chosen so the scene "reads" in any light ([Valve, NPAR 2007](https://www.cs.princeton.edu/courses/archive/fall07/cos597B/papers/mitchell-team-fortress.pdf)).
5. **Real shadows from the sun's shadow map,** hard-edged in our look.
   - Holland smooths the sun's falloff so a slowly moving sun does not flicker shadows.
   - He adds noise to the normals so shadows do not pop on flat ground.
   - Cloud shadows are scrolling noise over everything ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).
6. **Grass and flowers as thousands of small cards** (`PRE-46`).
   - Each card takes the light and shadow of the ground at its foot (Godot's `LIGHT_VERTEX`, which Holland's work brought into Godot) and sways in noise-driven wind.
   - A spawn map keeps grass out from under rocks ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/), [Unity forum](https://discussions.unity.com/t/recreating-t3ssel8rs-3d-pixel-art/928878/2)).
   - Grass in Godot is drawn in chunks of a few metres with MultiMesh, thinning with distance down to a flat painted patch (an impostor), with dithered transitions.
     One such system took "just under 2 ms", about 12% of a 60-frame budget, on its author's computer ([hexaquo](https://hexaquo.at/pages/grass-rendering-series-part-4-level-of-detail-tricks-for-infinite-plains-of-grass-in-godot/), [GodotGrass](https://github.com/2Retr0/GodotGrass)).
7. **Trees as leaf cards on a crown shape,** scattered evenly over the crown by Poisson-disk sampling and lit as one round mass ([Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)).
8. **Water.**
   - Stylised water for phones uses a flat plane, a colour by depth, foam where the depth to the shore is small, and moving texture lines, with no reflections ([Godot Shaders: pixel art water](https://godotshaders.com/shader/pixel-art-water-shader/), [Unity: stylized water for mobile](https://discussions.unity.com/t/stylized-water-shader-desktop-mobile-vr-built-in-rp/639248)).
   - Holland adds mirror reflections, which needed a change to Godot's source (research 01).
9. **Distance:** Godot's visibility ranges swap or hide meshes by distance.
   Dithered fading is cheaper than alpha blending ([Godot docs: visibility ranges](https://docs.godotengine.org/en/stable/tutorials/3d/visibility_ranges.html)).

## What Godot's Mobile renderer changes

From research 01: on your phone we use the Mobile renderer, on Vulkan.

- **No normal buffer.**
  The outline pass must rebuild normals from depth ([atyuwen](https://atyuwen.github.io/posts/normal-reconstruction/)), or draw outlines from depth alone.
- **Reading the depth or the screen costs bandwidth.**
  - The Mobile renderer keeps each tile of the image in fast on-chip memory through several "subpasses".
  - Reading the screen or depth texture forces the image out to main memory, "a notable performance penalty" ([Godot docs: internal rendering architecture](https://docs.godotengine.org/fr/4.x/engine_details/architecture/internal_rendering_architecture.html)).
  - In 3D, Godot copies the screen after the opaque pass and before the transparent one ([Godot docs: screen-reading shaders](https://docs.godotengine.org/en/stable/tutorials/shaders/screen-reading_shaders.html)).
  - The cost scales with the pixel count, so drawing at a quarter of the width and height cuts it 16-fold.
- **Ways to draw outlines on Mobile,** to be measured on your phone:
  - **(a)** a full-screen pass reading depth, with rebuilt normals: the reference look, with the bandwidth cost above;
  - **(b)** the same with depth-only outlines: cheaper, without the bright rims on folds;
  - **(c)** a second low-resolution camera drawing normals and depth into its own image: the geometry is drawn twice, but no screen read;
  - **(d)** outlines drawn as slightly enlarged back faces, as many mobile games do: no screen read, but not exactly one pixel.
- **Shader stutter:** since 4.4, Godot precompiles pipelines and falls back to "ubershaders" while specialised ones compile, which "completely avoids shader stutter".
  This needs Vulkan, so it covers the Mobile renderer, not Compatibility ([Godot docs: pipeline compilations](https://docs.godotengine.org/en/4.7/tutorials/performance/pipeline_compilations.html), [Godot 4.4](https://godotengine.org/releases/4.4/index.html)).
- **No compute shaders that sample images** on your phone's driver (research 02).
  So effects are full-screen fragment passes, not compositor compute effects.
- **Shadows:** Godot's mobile defaults halve the shadow map's size and turn off filtering ([Godot docs: mobile limitations](https://docs.godotengine.org/en/3.5/tutorials/platform/mobile_rendering_limitations.html)).
  Our hard, pixel-sized shadows want exactly that.
  Their cost on your chip is measured in the prototype.
- **Low resolution in Godot:** either a SubViewport scaled up, which allows the sub-pixel shift for smooth pans, or 4.7's built-in nearest scaling of the 3D buffer, which does not (research 01).

## Not covered by any reference

- **The zoom out to the valley, the map and the globe** (`PRE-03`, `PRE-29`).
  It needs:
  - levels of detail: grass only near the camera, simpler trees farther out;
  - the map look beyond;
  - a camera that turns from orthographic to perspective as it rises.

  This is research 07.
- **Hundreds of people and animals moving at once,** drawn as block figures (research 01, 17).

## What we take

1. **The recipe of the references:**
   - low resolution;
   - a pixel-locked orthographic camera with a sub-pixel image shift;
   - one-pixel outlines and lit edges;
   - three bands of light, meeting in clean edges since you chose the sharp look in the art book (`PRE-20`);
   - hard sun shadows and cloud shadows;
   - grass and leaf cards lit from their roots or crowns;
   - stylised water without true reflections.
2. **Built for the Mobile renderer from the start:**
   - normals rebuilt from depth;
   - no compute effects;
   - shadow maps at mobile sizes;
   - pipelines precompiled at load.
3. **A look prototype on your phone before anything is built on it.**
   It measures the four outline methods and the full-scene frame time, and you judge the look against the art book.
4. **Distance by visibility ranges and dithered fades,** grass in chunks thinning to painted patches.

## Sources

- References:
  - [David Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)
  - [JoonyoungSeo: Unity pipeline](https://github.com/JoonyoungSeo/unity-isometric-pixel-pipeline)
  - [smallnightlight](https://smallnightlight.github.io/Pages/Page1/pixelart3d.html)
  - [Lettier](https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html)
  - [UPixelator docs](https://github.com/Radivarig/UPixelator_Documentation)
  - [Unity forum: recreating t3ssel8r](https://discussions.unity.com/t/recreating-t3ssel8rs-3d-pixel-art/928878/2)
  - [three.js pixel pass](https://threejs.org/examples/webgl_postprocessing_pixel.html)
  - [Leo Peltola's Godot demo](https://github.com/leopeltola/Godot-3d-pixelart-demo)
  - [A Short Hike: PlayStation Blog](https://blog.playstation.com/2021/08/05/crafting-a-tiny-open-world-a-look-behind-the-scenes-at-the-creation-of-a-short-hike/)
  - [A Short Hike: Wikipedia](https://en.wikipedia.org/wiki/A_Short_Hike)
  - [Valve: TF2 rendering](https://www.cs.princeton.edu/courses/archive/fall07/cos597B/papers/mitchell-team-fortress.pdf)
  - [Texel Splatting](https://arxiv.org/abs/2603.14587)
- Camera crawl:
  - [Godot forums: subpixel snapping](https://godotforums.org/d/36180-subpixel-snapping-in-a-3d-pixel-art-game)
  - [Unity: pixel swimming](https://discussions.unity.com/t/help-with-pixel-swimming-in-3d-pixel-art-game/952547)
- Godot:
  - [internal rendering architecture](https://docs.godotengine.org/fr/4.x/engine_details/architecture/internal_rendering_architecture.html)
  - [screen-reading shaders](https://docs.godotengine.org/en/stable/tutorials/shaders/screen-reading_shaders.html)
  - [pipeline compilations](https://docs.godotengine.org/en/4.7/tutorials/performance/pipeline_compilations.html)
  - [Godot 4.4](https://godotengine.org/releases/4.4/index.html)
  - [visibility ranges](https://docs.godotengine.org/en/stable/tutorials/3d/visibility_ranges.html)
  - [mobile limitations (3.5)](https://docs.godotengine.org/en/3.5/tutorials/platform/mobile_rendering_limitations.html)
- Shaders:
  - [flexible toon](https://godotshaders.com/shader/flexible-toon-shader-godot-4/)
  - [ultimate toon](https://godotshaders.com/shader/ultimate-toon-shader/)
  - [pixel art water](https://godotshaders.com/shader/pixel-art-water-shader/)
  - [Unity stylized water](https://discussions.unity.com/t/stylized-water-shader-desktop-mobile-vr-built-in-rp/639248)
  - [atyuwen: normals from depth](https://atyuwen.github.io/posts/normal-reconstruction/)
- Grass:
  - [hexaquo: grass level of detail](https://hexaquo.at/pages/grass-rendering-series-part-4-level-of-detail-tricks-for-infinite-plains-of-grass-in-godot/)
  - [GodotGrass](https://github.com/2Retr0/GodotGrass)
