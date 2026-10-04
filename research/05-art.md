# Research 04: the art guide

**Question:** what exactly should Kindling look like, so that every texture and model belongs to one world?
Studios write this down as an art bible before making assets (research 00).

**Your choice (3 October 2026):** no voxels; a mix of the soft painted nature of t3ssel8r and David Holland with the pixel-textured models of PixelageGames, Lettier and the Godot shader on Reddit, with the textures made by me.

## What the references do

- **t3ssel8r's style, recreated in Unity** ([Unity forum](https://discussions.unity.com/t/recreating-t3ssel8rs-3d-pixel-art/928878/2)): calm, flat green ground; chiselled lavender-grey rocks, light on top and darker on the sides; a scatter of small purple flowers; few grass tufts; soft shadows.
- **David Holland** ([article](https://www.davidhol.land/articles/3d-pixel-art-rendering/)): round, fluffy trees made of many leaf clusters, yellow-green on top and dark teal underneath; turquoise water reflecting the trees and rocks; light shafts through haze; grass in soft patches.
- **PixelageGames** ([itch.io](https://pixelagegames.itch.io)) and **Lettier** ([book](https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html)): every made thing carries a pixel-drawn texture: planks, shingles, cobbles, bricks; warm golden or dusky purple light; long shadows.
- **The Godot shader on Reddit** (r/godot, June 2024): a wooden hut and props with pixel textures, a clear pond showing the grass under it, and outlines on every object.

## The rules

1. **Nature is soft, made things are crisp.**
   - Ground, grass, leaves and water are painted in soft patches of colour, with no texture noise.
   - Rocks, wood, hides, reeds and bone carry pixel-drawn textures.
2. **One texel density.**
   Every texture is drawn at **16 pixels per metre**, so at the close camp zoom one texture pixel is about one art pixel.
   Mixed densities make objects look as if they came from different games ([RebusFarm](https://rebusfarm.net/blog/texel-density-basics-every-artist-should-know)).
3. **Hue-shifted ramps.**
   Each material has a ramp of 4 to 6 shades: the shadows lean cooler (toward blue and purple) and the lights warmer (toward yellow), rather than only darker and lighter.
   Pixel artists call this the biggest single upgrade over flat shading ([Wayline](https://www.wayline.io/learn/color-palettes/2), [Pixnote](https://pixnote.net/en/learn/shading)).
4. **Light:**
   - a warm sun;
   - shade filled with a cool, purple-blue sky light, never black;
   - three bands of light (research 03).
5. **Edges:**
   - outlines are a darker shade of the same colour, never black;
   - outward edges catch a lighter shade;
   - rock and wood show the most edges, leaves and grass none.
6. **The ground:**
   - two or three greens in big soft patches, with a fine pixel pattern only where two meet (`PRE-20`);
   - grass as scattered tufts and small flowers, not a carpet;
   - paths as packed dirt with small pebbles.
7. **Trees and bushes:**
   - a bark-textured trunk and branches;
   - crowns of many leaf-cluster cards, lit as one round shape: yellow-green on top, dark teal underneath;
   - a gentle sway.
8. **Rocks and cliffs:**
   - chiselled shapes with flat facets;
   - a texture of horizontal layers that runs on from rock to rock, so a cliff shows its strata (`PRE-23`);
   - moss on the top faces, lighter top edges.
9. **Made things belong to the Stone Age:** tents of stitched hides on poles, windbreaks of branches, reed bundles, logs, fire rings of stones.
   Each is drawn from its material's texture (`PRE-42`).
10. **Water:** turquoise, darker where deep, with a foam line at the bank, sparkles, and a tint of the sky and of what stands above it.

## Mistakes to avoid

- A carpet of grass noise (the first bake-off picture).
- Flat, toy-like colours with no texture on made things.
- Pure black outlines or shadows.
- Saturated cartoon green everywhere.
- Smooth gradients over large areas.
- Textures at different pixel sizes side by side.
