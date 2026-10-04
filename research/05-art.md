# Research 05: the art bible

**Question:** what exactly should Kindling look like, so that every texture and model belongs to one world?
How do teams write that down so that many hands, here code and AI builders, keep to it?
Studios write an art bible before making assets (research 00).

**Your choice (3 October 2026):** no voxels.
A mix of the soft painted nature of t3ssel8r and David Holland with the pixel-textured models of PixelageGames, Lettier and the Godot shader on Reddit, with the textures made by code.

## How art bibles work

- **Four public art bibles share five rules** ([Makko](https://blog.makko.ai/what-public-game-art-bibles-say/)):
  - 0 A.D., a 3D strategy game;
  - the Liberated Pixel Cup style guide;
  - Battle for Wesnoth's unit art guide;
  - Riot's League of Legends effects guide.

  | Rule | Example |
  |---|---|
  | A tie-breaker when two rules conflict | "Gameplay trumps Realism when the two topics disagree" (0 A.D.) |
  | The light direction, with its frame of reference spelled out | "the artist's (not the unit's!) upper right" (Wesnoth) |
  | The shadow colour as an actual value | #322125 at 60% opacity (the Pixel Cup) |
  | A smallest detail worth making, tied to a real object | "Anything smaller than a human hand should not be modeled" (0 A.D.) |
  | A named list of common mistakes | "things that are important enough to be worth repeating" |

  They also fix technical limits:
  - texture budgets by importance (0 A.D., 128 to 512 pixels);
  - a fixed camera angle (the Pixel Cup, about 60°, orthographic);
  - canvas sizes and animation frame counts (Wesnoth, "4 frames minimum, 6 optimal").
- **The Liberated Pixel Cup's shadows** are dark purple (#2a1722) at about half opacity, or black only in caves ([OpenGameArt](https://opengameart.org/node/125066)).
- **Style chosen for a purpose:** Team Fortress 2's illustrated style serves readability.
  "Even when viewed only in silhouette with no internal shading at all, the characters are readily identifiable to players", and shading uses "variation in luminance and hue" so players can read the scene in any light ([Valve, NPAR 2007](https://www.cs.princeton.edu/courses/archive/fall07/cos597B/papers/mitchell-team-fortress.pdf)).

## Colour

- **Ramps with hue shift:** each ramp of a pixel palette shifts its hue as it brightens, up to about 20° a step.
  - Brightness rises steadily.
  - Saturation peaks in the middle of the ramp and eases toward both ends, so the lightest colours never burn.
  - Ramps without hue shift ("straight ramps") look dull and do not harmonise with each other.
  - Neutral, desaturated colours are added apart ([Slynyrd: Pixelblog 1](https://www.slynyrd.com/blog/2018/1/10/pixelblog-1-color-palettes)).
- **Shadows lean cool, lights lean warm:** toward blue and purple in shade, toward yellow in light ([Wayline](https://www.wayline.io/learn/color-palettes/2), [Pixnote](https://pixnote.net/en/learn/shading)).
- **A palette from a real place:** A Short Hike sampled its palette from photographs of the Canadian Shield in autumn ([Wikipedia](https://en.wikipedia.org/wiki/A_Short_Hike)).
- **Colour rules for generated worlds:** No Man's Sky's art director built "a really complex kind of color theory system so they all obey rules, like leaves having complimentary colors compared to the grass" ([Engadget](https://www.engadget.com/2016-04-22-no-mans-sky-art-video.html)).
  A planet's colour "bleeds into the creatures" through tags ([Kill Screen](https://killscreen.com/how-no-mans-sky-paints-18-quintillion-worlds-algorithmic-brush)).

## Art for generated content

- **Constraints, not randomness:** complete randomness, says No Man's Sky's art director, "is useless".
  The game funnels random choices "into a box of maths" that sets heights, gaits and bone counts by rules ([Kill Screen](https://killscreen.com/how-no-mans-sky-paints-18-quintillion-worlds-algorithmic-brush), [GDC 2015](https://www.nomanssky.com/2015/02/no-mans-sky-at-gdc/)).
- **Review at scale:** its art director had "hundreds of tiny drones" land on planets and record a short animated picture of each, for review on one board.
  This is the model for our contact sheet and model sheet (`PRE-31`).
- **Variety must be seen, not just exist:** Kate Compton's "10,000 bowls of plain oatmeal".
  Every bowl may be mathematically unique, yet the player sees only oatmeal.
  Generated things must differ in ways the eye notices: silhouette, colour, size ([Emily Short](https://emshort.blog/2016/09/21/bowls-of-oatmeal-and-text-generation/), [FlowingData](https://flowingdata.com/2016/08/04/building-a-generator-for-stuff), [Wikipedia: procedural generation](https://en.wikipedia.org/wiki/Procedural_generation)).
  This bears on `PRE-43`.

## One texel density

Every texture is drawn at the same number of pixels per metre.
Mixed densities make objects look as if they came from different games ([RebusFarm](https://rebusfarm.net/blog/texel-density-basics-every-artist-should-know)).
At Kindling's close camp zoom, 16 texture pixels a metre puts one texture pixel at about one art pixel.

## The Stone Age, from archaeology

- **Shelters became habitual in the Upper Palaeolithic,** with the harsh climate of the last glacial maximum ([tDAR](https://core.tdar.org/document/450967/built-environments-in-the-middle-and-early-upper-paleolithic)):
  - rings of mammoth bones in eastern Europe;
  - lighter tents in western Europe.

  A hut had three parts: a hearth, working places, and a sleeping area along the walls.
  Domes that could be put up quickly suit people who move with the seasons.
- **The oldest known picture of a camp,** engraved 13,800 years ago at Molí del Salt in Spain, shows seven semicircular huts.
  The number fits estimates of band sizes ([CS Monitor](https://www.csmonitor.com/Science/2015/1203/What-was-life-like-for-hunter-gatherers-13-800-year-old-sketch-offers-clues)).
- **Their traces are light:** often only hearths, post holes and working floors survive.
  This is the trace `PRE-25` and `MAT-08` draw on.

## What the references do

- **t3ssel8r's style, recreated in Unity** ([Unity forum](https://discussions.unity.com/t/recreating-t3ssel8rs-3d-pixel-art/928878/2)):
  - calm, flat green ground;
  - chiselled lavender-grey rocks, light on top and darker on the sides;
  - a scatter of small purple flowers;
  - few grass tufts;
  - soft shadows.
- **David Holland** ([article](https://www.davidhol.land/articles/3d-pixel-art-rendering/)):
  - round, fluffy trees of many leaf clusters, yellow-green on top and dark teal underneath;
  - turquoise water;
  - light shafts through haze;
  - grass in soft patches.
- **PixelageGames** ([itch.io](https://pixelagegames.itch.io/3d-pixelart-vegetation)) and **Lettier** ([book](https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html)):
  - every made thing carries a pixel-drawn texture;
  - warm golden or dusky purple light;
  - long shadows.
- **The Godot shader on Reddit** (r/godot, June 2024):
  - a wooden hut and props with pixel textures;
  - a clear pond;
  - outlines on every object.

## What we take: the art bible's contents

The art bible is written as a document of its own in pre-production, with pictures, and judged by you.
Its rules, in the shape the public bibles share:
1. **Tie-breaker:** your reference look wins over realism.
   Readability wins over detail: a person, an animal or a fire must read at every zoom (`PRE-28`).
2. **Light:**
   - a warm sun;
   - shade filled by a cool purple-blue sky light, never black;
   - three bands of light.

   The light direction for every texture is the world's own sun, and the default camera looks with the sun behind its left shoulder.
   The pictures fix both.
3. **Shadow colour as a value:** the deepest shade is a set dark purple, not black.
   Black only deep inside caves, as the Pixel Cup does.
4. **The smallest thing worth making:** about a fist-sized stone, one art pixel being about 6 cm at the close camp zoom.
5. **Ramps:** each material has a ramp of 4 to 7 shades with hue shift and mid-ramp saturation, from one master palette (`PRE-20`).
   Leaves and grass are kept complementary as No Man's Sky does, and sampled, where possible, from photographs of the real biomes.
6. **Nature soft, made things crisp:**
   - ground, grass, leaves and water in soft patches of colour;
   - rock, wood, hide, reed and bone with pixel-drawn textures at 16 pixels a metre.
7. **Edges:** outlines a darker shade of the same colour, never black; outward edges catch a lighter shade.
8. **The Stone Age, from archaeology:** hide tents and windbreaks, dome huts, hearth rings and working floors, as excavated camps show (`PRE-42`).
9. **Variety that shows:** silhouette, size and colour vary within a kind; invisible differences don't count (`PRE-43`).
10. **Mistakes to avoid:**
    - a carpet of grass noise;
    - flat, toy-like colours on made things;
    - pure black outlines or shadows;
    - saturated cartoon green everywhere;
    - smooth gradients over large areas;
    - textures at different pixel sizes side by side;
    - oatmeal: many things that differ only in ways nobody can see.
11. **Review:** a contact sheet and a model sheet, made by code from fixed scenes, at every change of the art, as No Man's Sky's drones did for its planets.

## Sources

- Art bibles:
  - [Makko: four public art bibles](https://blog.makko.ai/what-public-game-art-bibles-say/)
  - [OpenGameArt: Liberated Palette](https://opengameart.org/node/125066)
  - [Valve: TF2](https://www.cs.princeton.edu/courses/archive/fall07/cos597B/papers/mitchell-team-fortress.pdf)
- Colour:
  - [Slynyrd: colour palettes](https://www.slynyrd.com/blog/2018/1/10/pixelblog-1-color-palettes)
  - [Wayline: palettes](https://www.wayline.io/learn/color-palettes/2)
  - [Pixnote: shading](https://pixnote.net/en/learn/shading)
  - [A Short Hike: Wikipedia](https://en.wikipedia.org/wiki/A_Short_Hike)
- Generated content:
  - [Engadget: No Man's Sky's art](https://www.engadget.com/2016-04-22-no-mans-sky-art-video.html)
  - [Kill Screen: No Man's Sky](https://killscreen.com/how-no-mans-sky-paints-18-quintillion-worlds-algorithmic-brush)
  - [No Man's Sky at GDC](https://www.nomanssky.com/2015/02/no-mans-sky-at-gdc/)
  - [Emily Short: bowls of oatmeal](https://emshort.blog/2016/09/21/bowls-of-oatmeal-and-text-generation/)
  - [FlowingData: building a generator](https://flowingdata.com/2016/08/04/building-a-generator-for-stuff)
  - [Wikipedia: procedural generation](https://en.wikipedia.org/wiki/Procedural_generation)
- [RebusFarm: texel density](https://rebusfarm.net/blog/texel-density-basics-every-artist-should-know)
- Archaeology:
  - [tDAR: built environments in the Palaeolithic](https://core.tdar.org/document/450967/built-environments-in-the-middle-and-early-upper-paleolithic)
  - [CS Monitor: Molí del Salt](https://www.csmonitor.com/Science/2015/1203/What-was-life-like-for-hunter-gatherers-13-800-year-old-sketch-offers-clues)
- References:
  - [Unity forum: t3ssel8r](https://discussions.unity.com/t/recreating-t3ssel8rs-3d-pixel-art/928878/2)
  - [David Holland](https://www.davidhol.land/articles/3d-pixel-art-rendering/)
  - [PixelageGames](https://pixelagegames.itch.io/3d-pixelart-vegetation)
  - [Lettier](https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html)
