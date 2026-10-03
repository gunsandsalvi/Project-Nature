# Research 05: making and drawing the world

**Question:** how do other generators make believable continents, mountains, rivers, climates and biomes quickly (`WLD-08`, `WLD-09`, `WLD-11`), and how is ground that big drawn from one person up to the globe (`PRE-03`, `PRE-29`)?

## How others do it

- **Red Blob Games' mapgen4** builds its map on a triangle mesh (Delaunay and Voronoi).
  It simulates evaporation, wind and rainfall to place biomes and rivers, and runs on several threads to stay fast.
  Its first lesson is to make the elevation "match the desired look instead of tweaking the look to match the elevation" ([Red Blob Games](https://www.redblobgames.com/maps/mapgen4/), [sphere maps](https://simblob.blogspot.com/2018/10/map-generation-on-sphere.html)).
- **World Orogen**, an open planet generator, runs 16 stages in a few seconds ([GitHub](https://github.com/raguilar011095/planet_heightmap_generation), GPL, so we learn from it but copy nothing):
  1. points spread on a sphere and joined into triangles;
  2. tectonic plates, with their colliding, parting and sliding edges raising ranges, ridges, trenches and island arcs;
  3. four kinds of erosion: glaciers carving fjords, rivers carving valleys (filling basins in order of height, "priority flood"), slopes slumping, and soil creeping;
  4. winds that bend with the planet's turn, ocean currents, rain that falls where moist air meets hills, and temperature from latitude and height;
  5. about 30 Köppen climate types, coloured as biomes.
- **Azgaar's Fantasy Map Generator** splits its code four ways: settings feed generators, generators produce the world's data, and renderers draw it ([GitHub](https://github.com/Azgaar/Fantasy-Map-Generator)).
- **Undiscovered Worlds** makes a global map first, then makes regional detail as you zoom in ([forum](https://forum.thegamecreators.com/thread/223804)).
  Its new version admits "the same seed does not always produce the same planet", a warning about determinism ([GitHub](https://github.com/JonathanCRH/Undiscovered_Worlds)).
- **Terrain3D**, Godot's leading terrain plug-in, is C++ and draws ground with geometry clipmaps, as The Witcher 3 did.
  It has regions up to 65.5 km a side, 10 levels of detail, and instanced plants with their own levels of detail.
  It builds for mobile ([Godot store](https://store.godotengine.org/asset/tokisangames/terrain3d/), [GameFromScratch](https://gamefromscratch.com/terrain3d-a-new-terrain-engine-for-godot/)).
- **Large worlds in Godot:** double-precision builds cost speed and memory, and are aimed at desktops.
  On weaker devices Godot recommends moving the world's origin with the camera instead ([Godot docs](https://docs.godotengine.org/en/stable/tutorials/physics/large_world_coordinates.html)).

## What we take

1. **The generator runs in the C++ simulation, in World Orogen's order of causes,** which is also the order `WLD-09` asks for:
   - plates;
   - elevation;
   - erosion;
   - climate (wind, rain shadow, temperature by latitude and height on our torus, `WLD-01`);
   - rivers by flow accumulation;
   - soils and biomes;
   - deposits by geology (`WLD-14`).

   It works on a mesh of world cells about 1 km across, as mapgen4 does on its triangles, on several threads in a fixed order, so the same seed gives the same world on every machine.
2. **Tuned for the look, as Red Blob advises.**
   The generator's numbers are tuned until the map and the close-up views look like the art guide (research 04), not only until they are plausible.
3. **Detail on demand, from the seed.**
   As in Undiscovered Worlds, close-up ground is made from its world cell and the seed when it is needed, the same every time, so looking changes nothing (`WLD-13`).
4. **Our own ground drawing at first, with Terrain3D as the model to study.**
   Our ground comes from the generator, not from hand-painted height maps, and needs our pixel-art shaders.
   So we start with chunks in a few rings of detail round the camera, as clipmaps do, and adopt Terrain3D if ours falls short.
5. **The world's origin moves with the camera.**
   The simulation keeps its own exact coordinates (research 02), and Godot draws everything relative to a nearby origin, which suits the phone better than a double-precision build.
6. **Far views are their own drawings:** the map look of flat-coloured cells and river lines (`PRE-29`), and the globe as a sphere painted from the world map (`WLD-02`).
   Plants, rocks and grass fade out by distance, with simple stand-ins in between.
