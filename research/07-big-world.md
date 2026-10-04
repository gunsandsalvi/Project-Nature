# Research 07: drawing a world 2,000 km wide, from one person to the globe

**Question:** how do games draw ground this big, and zoom without a break from a person chipping flint to the whole globe (`PRE-03`, `PRE-28`, `PRE-29`, `WLD-02`, `WLD-12`)?
Can Godot do it on your phone?

## What `PRE-03` asks

- **One pinch from the globe to a person,** through these stops:
  - person, about 8 m across;
  - close camp, 20 to 50 m;
  - camp, a few hundred metres;
  - valley, about 10 km;
  - region, about 100 km;
  - the world map;
  - the globe.
- **What is drawn at each distance:**
  - within about 300 m, full areas made for the picture;
  - out to about 10 km, coarse ground under its cover;
  - beyond, the world cells in the map look.
- **The globe** hides the wrap of our torus under the polar ice (`WLD-01`, `WLD-02`).
- **People, herds and camps stay readable at every distance** (`PRE-28`).

## How others do it

- **Strategic zoom** began with Supreme Commander (2007): "seamlessly scale from the relatively closely-cropped camera view … all the way up to a full map view", units turning into map symbols.
  Players "can't live without it afterwards" ([Matchstick Eyes](https://www.matchstickeyes.com/tag/rts-zoom/)).
- **Google Maps** (2018) morphs its flat map into a 3D globe as you zoom out, in the browser ([TechRadar](https://www.techradar.com/news/google-maps-on-desktop-now-shows-the-earth-as-a-3d-globe), [Geogarage](https://blog.geogarage.com/2018_07_29_archive.html)).
  It is the model for our map look turning into the globe.
- **Ground in rings of detail:**
  - Geometry clipmaps (Losasso and Hoppe, SIGGRAPH 2004) draw terrain as square rings centred on the viewer.
    Each ring is twice as wide as the last at half the detail, refilled as the viewer moves, with "uniform frame rate" and "graceful degradation" ([Hoppe](https://hhoppe.com/proj/geomclipmap/)).
  - CDLOD (Strugar) uses a quadtree of grids instead.
    Detail follows the true 3D distance, with smooth transitions and no stitching ([GitHub](https://github.com/fstrugar/CDLOD)).
  - Terrain3D brings clipmaps to Godot in C++, with 10 levels of detail (research 01).
- **Planets in Godot:**
  - "Procedural Planet: Chunked LOD" flies "from orbit down to the surface" on a full-size planet, in plain GDScript with two shaders ([Godot Asset Library](https://godotengine.org/asset-library/asset/4942), [store](https://store.godotengine.org/asset/cuberact/procedural-planet-chunked-lod/)).
    It has a quadtree, chunk pooling, horizon culling and **origin shifting**.
  - A 2025 study built two Godot planet generators with quadtree level of detail, using Godot's double-precision build ([arXiv](https://arxiv.org/html/2510.24764v1)).
- **A moving origin:** Kerbal Space Program's "Krakensbane" moves "the center of the universe with the ship" whenever the ship passes a distance threshold.
  This killed the precision errors that shook ships apart far from the origin ([KSP wiki](https://wiki.kerbalspaceprogram.com/wiki/API:Krakensbane), [KSP forum](https://forum.kerbalspaceprogram.com/topic/91108-how-does-ksp-work)).
  Godot's own advice for weaker devices is the same, rather than double precision (research 01).
- **Distant trees:** octahedral impostors draw a whole tree as one flat card that shows the right view from any angle.
  A Godot plug-in renders "a forest of 1400 trees using only impostor planes" ([GitHub](https://github.com/SIsilicon/Godot-Octahedral-Impostors)).
  Godot's visibility ranges swap models by distance, and dithered fades are cheaper than transparent ones ([Godot docs](https://docs.godotengine.org/en/stable/tutorials/3d/visibility_ranges.html)).
- **The map look:**
  - Cartography's hillshading lights each cell by slope, aspect and a sun direction.
  - Swiss-style relief uses several light directions.
  - Cartographers also bend the light by aspect so relief reads everywhere ([Atlas](https://atlas.co/courses/gis-basics/hillshade-and-3d-visualization/), [MapTiler](https://docs.maptiler.com/guides/map-design/terrain/hillshading/), [Mapbox](https://blog.mapbox.com/new-in-studio-dynamic-hillshading-2027c77781d8)).
  - Rusinkiewicz's "exaggerated shading" adjusts the light per region to show every bump ([Princeton](https://gfx.cs.princeton.edu/pubs/Rusinkiewicz_2006_ESF/exaggerated_shading.pdf)).

## What this means for Kindling

- **Feasible in Godot, but ours to build:** a Godot project already flies from orbit to ground, so the engine is not the obstacle.
  But no plug-in does our mix:
  - the pixel look up close;
  - our generated ground;
  - the map look in the middle;
  - a torus shown as a globe.
- **Godot draws in single precision around an origin that moves with the camera,** as KSP and the planet project do.
  The simulation's exact coordinates never depend on it (research 03).
- **Ground, by distance:**
  - near areas as detailed chunks in clipmap-like rings, made from the seed (research 06);
  - coarse chunks out to about 10 km;
  - the world cells as one map mesh beyond.

  Each level fades into the next with dithering, so the pixel look never blurs.
- **Things, by distance:**
  - grass and small stones only near;
  - trees as models, then impostor cards, then the cover colour;
  - people, herds and camps as models, then the outlined tiny figures and markers of `PRE-28`.
- **The camera:**
  - orthographic and pitched for the close stops;
  - tilting toward straight down as it rises (`PRE-29`);
  - turning perspective for the globe.

  The flat map is bent onto a sphere for the last step, as Google Maps morphs to its globe.
  The globe's picture squeezes the polar lands and hides the seam under the ice (`WLD-02`).
- **The map look:** cells in flat cover colours, rivers as lines, and hills shaded cartographers' way, lit by the art bible's palette.
- **Budget:** each level's cost is measured on your phone at every zoom stop (`PLT-04`).
  Making a full area within about a second (`PRE-03`) is measured too.

## What we take

1. **Our own level-of-detail system in C++,** driving Godot's RenderingServer directly: clipmap-like rings for near ground, coarse chunks, then the map mesh and the globe.
   Terrain3D and the planet project are the models to study.
2. **A moving origin, single precision in Godot.**
3. **Impostors and visibility ranges for distant things;** dithered fades everywhere.
4. **The map look by hillshading in the art bible's palette,** morphing onto a globe at the top.
5. **A zoom prototype before production:** one pinch from the globe to a person on a generated world, on your phone, measured at every stop.
   It is one of the riskiest pieces.

## Sources

- Strategic zoom: [Matchstick Eyes](https://www.matchstickeyes.com/tag/rts-zoom/)
- Globe:
  - [TechRadar: Google Maps globe](https://www.techradar.com/news/google-maps-on-desktop-now-shows-the-earth-as-a-3d-globe)
  - [Geogarage](https://blog.geogarage.com/2018_07_29_archive.html)
- Terrain:
  - [Losasso and Hoppe: geometry clipmaps](https://hhoppe.com/proj/geomclipmap/)
  - [Strugar: CDLOD](https://github.com/fstrugar/CDLOD)
- Godot:
  - [Procedural Planet: Chunked LOD](https://godotengine.org/asset-library/asset/4942)
  - [its store page](https://store.godotengine.org/asset/cuberact/procedural-planet-chunked-lod/)
  - [Comparative analysis of Godot planet generators](https://arxiv.org/html/2510.24764v1)
  - [Octahedral impostors](https://github.com/SIsilicon/Godot-Octahedral-Impostors)
  - [visibility ranges](https://docs.godotengine.org/en/stable/tutorials/3d/visibility_ranges.html)
- Origin: [KSP wiki: Krakensbane](https://wiki.kerbalspaceprogram.com/wiki/API:Krakensbane), [KSP forum](https://forum.kerbalspaceprogram.com/topic/91108-how-does-ksp-work)
- Map look:
  - [Atlas: hillshade](https://atlas.co/courses/gis-basics/hillshade-and-3d-visualization/)
  - [MapTiler: hillshading](https://docs.maptiler.com/guides/map-design/terrain/hillshading/)
  - [Mapbox: dynamic hillshading](https://blog.mapbox.com/new-in-studio-dynamic-hillshading-2027c77781d8)
  - [Rusinkiewicz: exaggerated shading](https://gfx.cs.princeton.edu/pubs/Rusinkiewicz_2006_ESF/exaggerated_shading.pdf)
