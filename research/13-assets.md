# Research 13: making models, textures and animation by code

**Question:** with no artist and no asset packs, how are every texture, model and animation made, so that new things are easy to add (`PRE-42` to `PRE-46`)?

## How others do it, and what we already proved

- **Our bake-off art** (research 04, `prototypes/bakeoff/make_art.py`) was made entirely by code:
  - pixel textures drawn at 16 pixels a metre in hue-shifted ramps;
  - trees as leaf cards on crowns;
  - rocks as chiselled spheres;
  - a layered cliff, a hide tent, a fire ring;
  - all written as glTF and imported by Godot unchanged.
- **Godot animates skeletons** with AnimationPlayer and AnimationTree, and bends them by code with SkeletonModifier3D.
  Since Godot 4.6 (January 2026) inverse kinematics is part of that modifier stack, so a foot can be placed on uneven ground by code ([StraySpark](https://www.strayspark.studio/blog/godot-46-inverse-kinematics-procedural-animation), [Godot docs](https://docs.godotengine.org/en/latest/classes/class_skeletonik3d.html)).
- **Rule-based variety.**
  Horizon's placement rules (research 06) and Caves of Qud's grammars (research 08) show variety coming from rules over data rather than hand-made items.

## What we take

1. **The model kit is code plus data** (`PRE-46`).
   - A generator in the tool chain (Python, as in the bake-off, or C++ later) builds every shape from parameters in the catalogue.
   - Plants come from about 8 forms, animals from about 6 body patterns, people from one figure of blocks, things from layouts of parts.
   - Adding a thing is adding its entry; its model follows.
2. **Textures are drawn by code into ramps**, one per material, at the one texel density.
   A contact sheet of every texture is rendered with each change for you to see.
3. **Animation as code-made key poses** (`PRE-44`):
   - each movement is 2 to 6 key poses of the block figure, held about 10 times a second;
   - the variants bend them by rule (elders stoop, a limp shortens one step);
   - IK puts feet on the ground.
4. **Variety by seed** (`PRE-43`): each thing varies its proportions, lean, wear and colour within its ramp, by its own keyed chance.
5. **A model sheet** of every kit shape in two materials, rendered in the cloud each time the kit changes, so you judge the art as a whole.
