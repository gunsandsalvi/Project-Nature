# Research 17: making models, textures and animation by code

**Question:** with no artist and no asset packs, how is every model, texture, icon and animation made from data, so that a new thing needs only its catalogue entry?
Can Godot build and draw them on your phone at run time (`PRE-42`, `PRE-43`, `PRE-44`, `PRE-46`, `PRE-27`)?

## What `PROJECT.md` asks

- **One fixed model kit** (`PRE-46`):
  - one shared shape per form, stretched to size and coloured by material;
  - made things as layouts of their inputs' shapes;
  - about 8 plant forms, 6 animal body patterns and one figure for people;
  - about 12 hand-drawn signs.
- **Things built from their materials** (`PRE-42`):
  - parts sized by the amount used, so more poles make a bigger hut;
  - states and wear shown on the model;
  - icons from the same model.
- **Variety by seed and style:** proportions, lean, wear and colour vary per thing, and each people's style and pattern shows (`PRE-43`).
- **People and animals as small figures of tiny blocks,** posed about 10 times a second (`PRE-27`, `PRE-44`):
  - about 45 movements of 2–6 key poses each;
  - about 8 gestures and 8 dance moves;
  - variants bend them by rule;
  - animals have one set per body pattern.

## How others do it

- **Plants from rules:**
  - Prusinkiewicz and Lindenmayer's *The Algorithmic Beauty of Plants* (1990) models plants with L-systems.
    It is freely available as a PDF ([Algorithmic Botany](https://algorithmicbotany.org/papers/#abop)).
  - Runions, Lane and Prusinkiewicz's space colonisation algorithm (2007) grows branches toward free space and "generates surprisingly realistic tree structures" ([Algorithmic Botany](https://algorithmicbotany.org/papers/colonization.egwnp2007.html)).
  - For a pixel look with 8 forms, simple parametric forms are enough, and these algorithms can grow branch frames for the broad-leaved trees offline.
- **Textures from nodes:** Material Maker is "a procedural textures authoring and 3D model painting tool based on the Godot game engine", under the MIT licence ([GitHub: Material Maker](https://github.com/RodZill4/material-maker)).
  Our textures are small ramps drawn by code (research 05), so it is a tool for trying ideas, not a dependency.
- **Figures from rigid blocks:** Minecraft draws creatures as cuboid parts, each turned about its own pivot, with no skinning (research 01).
- **Few key poses go far:** David Rosen's GDC talk on Overgrowth shows how "to achieve interactive and fluid animations using very few key frames", with simple procedural layers ([GDC Vault](https://www.gdcvault.com/play/1020583/Animation-Bootcamp-An-Indie-Approach)).
  That is `PRE-44`'s plan: 2–6 key poses a movement, bent by rules.

## Can Godot do it?

| Need | What Godot has | Verdict |
|---|---|---|
| Shapes made by code | `ArrayMesh`, "slightly faster than using a SurfaceTool", for "static geometry (meshes) that don't change over time" ([Godot docs: procedural geometry](https://docs.godotengine.org/en/stable/tutorials/3d/procedural_geometry/index.html)) | Yes, built once at load from the kit's parameters |
| Many copies of shared shapes | `MultiMesh`: "thousands of instances with a single draw call" ([MultiMesh](https://docs.godotengine.org/en/stable/classes/class_multimesh.html)) | Yes, grouped by area, since a MultiMesh is "spatially indexed as one" |
| Colour, wear and style per copy | Per-instance colour and custom data, read in shaders as `INSTANCE_CUSTOM` | Yes |
| Parts sized by amount | Each copy's own transform | Yes |
| Posed block figures | Rigid parts as MultiMesh copies, posed by our C++ (research 01) | Yes, with no skeletons |
| Feet on uneven ground | Godot 4.6 added IK solvers such as `TwoBoneIK3D` and `FABRIK3D` for skeletons ([Godot 4.6](https://godotengine.org/releases/4.6/)) | Not needed: our figures have no skeletons, and the two-bone sum is a few lines of C++ |
| Icons from models | A `SubViewport` set to update "Once" renders "an image once" to a texture ([Godot docs: viewports](https://docs.godotengine.org/en/stable/tutorials/rendering/viewports.html)) | Yes, one icon per kind |
| Firelight on many copies | "Once the maximum lights are consumed by one or more instances, the rest of the MultiMesh instances will **not** receive any lighting" ([MultiMesh](https://docs.godotengine.org/en/stable/classes/class_multimesh.html)) | **A real risk:** a camp lit by several fires at night may need our own firelight term in the shader |

**Verdict:** Godot can build and draw the whole kit at run time.
The firelight limit on MultiMesh is the one risk, to be tested in the look prototype.

## What we take

1. **The kit is code plus data** (`PRE-46`):
   - each form's shared shape is built at load by our C++ as an `ArrayMesh` from the kit's parameters;
   - plants come from 8 parametric forms, with branch frames grown offline by space colonisation where a crown needs them;
   - animals come from 6 body patterns, and people from one block figure.
   - A new thing is a catalogue entry; its model follows.
2. **Models assembled at run time** as layouts of shared shapes, drawn as MultiMesh copies grouped by area.
   Each copy carries its material's colour, its wear and its people's style in per-instance data, so huts of birch and of reed look different with no new art (`PRE-42`, `PRE-43`).
3. **Textures drawn by code into the art bible's ramps** (research 05); Material Maker only for trying ideas.
4. **Animation as data:**
   - each movement is 2 to 6 key poses of the block figure, stepped about 10 times a second (`PRE-44`);
   - our C++ poses the rigid parts;
   - rules bend them: stoop, limp, slump, hunch;
   - each figure's seed offsets its timing;
   - feet meet the ground by simple two-bone sums.
5. **Icons rendered once per kind from the model,** through a SubViewport set to update once (`PRE-42`).
6. **Firelight designed early:** if Godot's per-object light limit leaves copies unlit, fires reach the figures and huts through a firelight term in our shaders, fed by a short list of nearby fires.
   The look prototype decides.
7. **Sheets for your review:** the model sheet of every kit shape in two materials and the animation sheet, rendered by Movie Maker mode in the cloud each time the kit changes (research 16, `PRE-31`).
8. **A kit prototype before production,** on your phone:
   - about 10 shapes, 2 plants, 1 animal, the figure with 5 movements, and a hut in two materials;
   - seen at camp zoom at noon, and at night with three fires;
   - it checks the frame time and the firelight.

## Sources

- Practice:
  - [Algorithmic Botany: The Algorithmic Beauty of Plants](https://algorithmicbotany.org/papers/#abop)
  - [Runions et al. 2007: space colonisation](https://algorithmicbotany.org/papers/colonization.egwnp2007.html)
  - [GitHub: Material Maker](https://github.com/RodZill4/material-maker)
  - [GDC Vault: An Indie Approach to Procedural Animation](https://www.gdcvault.com/play/1020583/Animation-Bootcamp-An-Indie-Approach)
- Godot:
  - [Godot docs: procedural geometry](https://docs.godotengine.org/en/stable/tutorials/3d/procedural_geometry/index.html)
  - [Godot class: MultiMesh](https://docs.godotengine.org/en/stable/classes/class_multimesh.html)
  - [Godot 4.6 release](https://godotengine.org/releases/4.6/)
  - [Godot docs: viewports](https://docs.godotengine.org/en/stable/tutorials/rendering/viewports.html)
