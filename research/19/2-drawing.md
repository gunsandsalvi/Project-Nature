# Study 2: how to draw it (round 2, direction B)

> Study 2 of [research 19](../19-graphics.md), written on 6 October 2026 and kept as written, its quotes checked ([the check](quote-check.md)).
> `R` was the research session's working folder and is not kept, except your pictures and answers, now in `art/targets/` and `art/reviews/2026-10-06-graphics/`.

Research for M2, the graphics engine (`MIL-09`). Study 2 of seven, round 2, 6 October 2026.
Godot facts are checked in Godot's own source at the tag `4.7.2-stable` (our version; a full checkout of the tag in `R/work/4/godot`) and in the 4.7 documentation.
Numbers marked *computed here* come from small scripts in `R/work2/2/` (`steady/`, `light/`, `camera/`, `read/`); nothing was built or drawn for the game.

**Words used here:**
- *Texture pixel* (texel): one square pixel of a texture painted as pixel art. In direction B it shows as about 2 × 2 screen pixels up close.
- *Filter:* how the graphics chip reads a texture between its pixels. *Nearest* takes the one texel under the screen pixel; *Linear* blends the four nearest; *mipmaps* are smaller copies of a texture that the chip uses when texels shrink below a screen pixel.
- *MSAA* (multisample anti-aliasing): the chip tests each screen pixel's coverage at 2 or 4 points, so the edges of shapes come out smooth, while the pixel's colour is still worked out once.
- *Shadow map:* a picture of the scene seen from the sun, used to tell what is in shadow.
- *The merged pass:* Godot's Mobile renderer draws solid things, see-through things and the final colour step in one go without the picture leaving the chip, unless something breaks it. Keeping it saves memory traffic, which costs battery.
- *Cut-out card:* a flat shape with a leaf or grass texture whose see-through parts are thrown away pixel by pixel.

## 1. The question, and the answer in brief

**The question:** how should the engine draw direction B, a smooth, sharp 3D world at the phone's full resolution wearing pixel-art textures at about 2 × 2 screen pixels per texture pixel, so that texture pixels stay steady and the liked feeling is kept, on Godot 4.7's Mobile renderer?

**The answer in brief:**
1. **Draw straight to the screen at full resolution** (1080 × 2404, 2.6 million pixels) with 2× MSAA for smooth edges; Godot resolves it on the chip and Imagination calls 2× "virtually free". No low-resolution picture, no pixel lock, no outline pass up close, never FXAA or TAA (they blur texels).
2. **Steady texels come from the filter, not the camera:** a "smooth pixel" filter, crisp inside each texel and blended over one screen pixel at its edge. *Computed here:* it equals the ideal average, cuts Nearest's flicker by 38–47% and keeps 94–98% of its crispness.
3. **Texels stay about 2 screen pixels at every zoom and across the screen,** through levels drawn for each size (64, 32, 16 … texels a metre), loaded as each texture's mipmaps. Never shrink a fine picture: that is the speckle and lost charm of answers 21 and 31. Up close the camera needs a long lens (about 5° across) or none: P8's 25° lens, at the pictures' 40° tilt, squashes texels at the top of a portrait screen to a fourteenth of their height at the bottom.
4. **Smooth light, worked out per pixel by one shared function,** with soft shadows that widen with distance and corner darkening built by us, since Mobile has neither: long shadows from a height map made when the sun moves, darkening baked into the kit, the ground and each area.
5. **The chosen moments set the light:** nights 85–87% dark, blue in shade, a hearth's light halving by about 2 m; winter snow cream in sun, blue in shade, near the top of the range. Mobile's 10-bit picture steps 3–7 screen levels at a time in the darkest tones, so nights need debanding (a fine dither) or 16-bit colour.
6. **Plants as dense and airy as liked,** saved only in ways that don't show; effects as copies moved by the chip; fire's glow worked out in the light, with no glow pass; one on-chip pass kept.
7. **People in busy scenes read best through the world itself:** study 1's calmer leaves, trodden ground and lighter clothes lifted all four people in the autumn wood; light-only aids helped two and hurt two.
8. **What changes:** new wording for `PRE-01`, `PRE-02`, `PRE-20`, `PRE-21`, `PRE-22`, and changes to `ARCHITECTURE.md` A4, A5.3 and A8.4, all proposals for the owner (section 6).

## 2. What `PROJECT.md` asks

- `PRE-01` (lifted): "crisp pixel art, drawn in art pixels", "limited colours from one palette", "no blur, no smooth gradients".
- `PRE-02` (decided, not lifted): "A real 3D world drawn at low resolution"; "the camera turns freely and zooms continuously". Direction B contradicts its first half (proposal 2).
- `PRE-20` (lifted): 4–7 shades, the light picks the step, "surfaces carry no fine grain".
- `PRE-21` (lifted): one-pixel outlines and lit edges.
- `PRE-22` (lifted in its sizes): "Pixels never crawl or shimmer while the camera is still or panning"; an art pixel "about 2 by 2 screen pixels at the person, growing with the zoom to about 6 by 6 at the globe"; "in portrait and landscape". Steady pixels stay a goal.
- `PRE-30`: light from the sun's height through the air "as in Minecraft's Vibrant Visuals"; shadows "sharp near what casts them and softer as they lengthen"; "shade takes the sky's colour and hollows are darker"; haze warmer toward the sun; water reflects and glints; a fire warms faces, glows, sends up smoke and embers; *Done when* one place at dawn, noon, dusk and night, in summer and winter.
- `PRE-23` rock faces, `PRE-24` caves dark but for openings and fire, `PRE-26` water, `PRE-28` readable from far away, `PRE-29` from above, `PRE-03` the seamless zoom.
- `PLT-04`: 60 frames a second, 97% on time, none more than 50 ms late; an hour at 25–30% of the battery; never uncomfortably hot. `PLT-02`: portrait and landscape.
- `PRN-10` "Nothing is faked": "Every picture, sound and word shows what is really there"; "Nothing is added for show". `WLD-13` "Looking changes nothing": "the picture and sound read the world, and nothing in the world reads the camera". Both bear on readability aids (section 6.11).
- `CLAUDE.md` rule 4: each piece of shader maths written once, so the texel filter and the light are shared functions.

## 3. Where we stand

### 3.1 What round 1 found, and what direction B keeps of it

My round-1 note built the look on art pixels: the world drawn small and enlarged, a camera locked to the grid, outlines from a data picture, light in steps, and colour codes turned into exact palette colours in one final step, because Godot's 10-bit picture shifts dark palette colours. Under direction B there are no art pixels, so most of that pipeline goes. What carries over:
- one shared light function, firelight through our own term fed by nearby fires, and fire shadows walked through height maps (P2, P3);
- the river bed drawn "under water" by the ground's own shader, and a mirrored pass for reflections, with no reads of the screen or the depth picture;
- particles as copies moved by the chip, never Godot's GPU particles: they are a compute program that reads textures (`particles.glsl` begins `#[compute]`), which A4.3 already rules out on this driver (another engine's crash on this phone and driver, Bevy issue 25788, read through a summarising fetch in round 1 and again by the quote checker);
- the Vibrant Visuals model of light keyed by the sun's height, the sky as fill light, cloud shadows;
- Godot's steadied sun shadow (section 4.6) and the phone rules of A4.3.
- The 10-bit picture still matters, but now for banding in smooth dark light, not for exact colours (section 6.7).

### 3.2 What round 2 has settled

The owner judged 32 picture questions (`R/owner/ask/answers.md`). Those that bear on drawing:
- **Direction B** (answer 1) at **2 × 2** (answer 2): "a smooth world with pixel textures".
- **No outlines up close, smooth light, the fine grain kept, darkening in corners and under things kept** (answer 7, "As I liked"), so we build the darkening ourselves.
- **Plants "As I liked"** (answer 6): none of the four cheaper plant ways; savings must not show.
- **True midday** (9); **nights** A, B and C, not D's many glowing pools (10); **winter** B only, the softer one (11); **facing a low sun** keeps the feeling (16); **rain**: all four layers matter, streaks, splashes and wet shine, ripples, drifting mist (17); **dusk** by relighting (23).
- **Half-resolution shading only if needed** (25): the owner saw it is softer.
- **Problems to solve, not cut:** small plants lose their charm at game size (21); the ground turns to speckle at the close camp (31); people in a busy autumn wood are found "with effort" (27).
- **Far away, small figures drawn to read:** outline, light face and tunic, enlarged tools (4).

### 3.3 The current design against direction B

| Design point | Under direction B |
|---|---|
| A4.1.1 Low resolution, enlarged with nearest sampling | Goes: the world draws at full resolution |
| A4.1.2 Camera locked to the pixel grid, "ease" | The lock goes; ease becomes a feel setting, no longer needed for steadiness |
| A4.1.3 Outline way C (a data picture compared per pixel) | Goes up close; small far figures get their own outline (section 6.11) |
| A4.1.4 Light in clean steps; hard sun shadows | Smooth light; soft shadows that widen, built by us |
| A4.1.5 Fire, fire shadows by height maps | Stays; the walks move into small world-space maps (section 6.4) |
| A4.1.6 Water with a mirrored pass and a shore line from the depth picture | Stays without the depth read: the shore line comes from height |
| A4.1.8 Smoke as a lit volume blended against a screen copy | Changes: soft cards, or the volume at half resolution |
| A4.2 Materials: "never fine grain" | Fine grain kept, as texture pixels |
| A5.3 One texel density, 16 a metre | Becomes a ladder of designed levels from 64 a metre |
| A8.4 Perspective at every stop (P8: 25° across) | Up close, a long lens or orthographic (section 6.3) |

### 3.4 What the other studies have found that bears on drawing

- **Study 4:** at full resolution every per-pixel cost is four times the 2 × 2 picture's; plants' hidden see-through layers and their triangles are what cost most; the texel's size no longer changes the cost, only memory.
- **Study 3:** small things become part of the ground's texture below about 6 texels on screen; one set of render targets per orientation; every fire through our own light grid.
- **Study 7:** asks whether the texel density is about 64 or about 32 a metre (answered in section 6.2); textures made from approved pictures need their own mipmaps.
- **Study 6:** the steadiness check moves from "whole pixels" to texture shimmer under smooth motion.
- **Study 1:** each zoom band needs its own pixel-art version of each texture, since the liked far views keep visible texture pixels.

## 4. What others do

### 4.1 t3ssel8r: anti-aliased pixel art in 3D

t3ssel8r, whose game in Unity is one of the best-known examples of 3D pixel art, made a video in 2023, "Crafting a Better Shader for Pixel Art Upscaling". Its description, from the channel's own feed: "In this episode, we explore a subject that was only tangentially related to my game project: rendering pixel art with anti-aliasing. Applications for this technique range from anti-aliasing for pixel-art textures in low-poly games like Minecraft, to games that primarily feature pixel art sprites in 3D environments, like Octopath Traveler." That is our case exactly: pixel-art textures on 3D shapes at a screen size that is not a whole number of pixels. The video itself could not be watched here; its chapters list a box filter, "The problem", "The solution" and "Transparency".

His earlier notes add three things we use:
- **Rain is layers** (2020): "Selling the atmosphere of rain is a matter of layering together a bunch of individual effects": streaks, "splashes on all gently-sloped ground", spatter on top edges, ripples, "mist blowing around the scene", bobbing leaves, lightning, a colour grade. His were mostly a screen-space pass, and "The downside is that the effect does not show well while the camera is rotating." Our camera turns freely, so ours live in the world.
- **Fire in steps:** "Different parts of the fire (core, flames, embers) run at different frame rates to emulate a hand-animation look."
- **A camera blend:** an orthographic camera for nearby things blended into a perspective one for distant things keeps "crisp pixel positioning in the foreground", and "A depth-based fog helps tie the scene together."

### 4.2 CptPotato's "smooth pixel" filter for Godot

A short, public Godot shader: "A shader snippet allowing for "smooth pixelated" filtering, eliminating most aliasing artifacts." Its readme: "Integer scales will remain pixel-perfect while non-integer scales produce a softer, antialiased look", and "In 3D the same benefits apply but are often more impactful because of the varying perspective and sometimes steep camera angles." The algorithm reads the texture once with the chip's own blending, at a coordinate moved so that the blend happens only within one screen pixel of a texel's edge, and uses the true slope of the coordinates to pick the mipmap (`textureGrad`). It was written for Godot 3 ("Only the shader using "manual" filtering will work on GLES 2") and is undated, so it is old; but it is plain maths that Godot 4.7.2's shading language supports (`textureGrad`, `dFdx` are in `shader_language.cpp`). RetroArch's "sharp-bilinear" shader and Sassone's 2021 post (round 1) describe the same idea for flat pictures: "apply neighbour/nearest filter on pixels that are fully inside a texel, and use bilinear with custom weights at the edges!"

### 4.3 Hytale: pixel-art textures on a smooth, modern world

Hytale's art director describes it as "A modern, stylized voxel game, with retro pixel-art textures" (Hytale blog, 22 December 2025): the closest commercial example of direction B. Three of its rules bear on us:
- "We paint lights and shadows inside textures and use real lights/shadows to bring everything together", and "we often paint/bake shadows, ambient occlusion, and highlights directly into the texture to simulate more complex lighting than there really is in the game." For us, painted darkening in creases is welcome; painted *sunlight from one side* is not, since our sun goes round and the camera turns (section 6.4).
- "We avoid applying effects that would damage the handcrafted texture style": no blur passes over the texture pixels.
- One texel density, within limits: "Blocks and Characters texture texel density ranges from simple to double".

### 4.4 Minecraft's Vibrant Visuals (named by `PRE-30`)

Pixel-art textures under modern light, at the screen's full resolution. From round 1, checked again: sun and moon colours key-framed through the day ("the key is a number from 0 (noon) to 1 (the next noon, 24 hours later)"); "The sky contributes significantly to indirect lighting"; lighting and grading blend between biomes; and shadows can be "quantized" to the texture's grid, "which represents the resolution in texels (texture units) to which shadows will be quantized". That last one is an option for us (section 5.4).

### 4.5 Imagination Technologies on its own chips

- **MSAA:** "2x MSAA is virtually free on most PowerVR graphics cores (Rogue and Volcanic onwards), while 4x MSAA+ will noticeably impact performance." The cost comes partly from "a reduction in tile dimensions", so more tiles to sort. "Performing MSAA becomes costlier when there is an alpha blended edge"; so "submit all opaque geometry first". And: "In some cases, there may be no need for anti-aliasing to be used at all, for example when the target device's display has high pixels per-inch (PPI)."
- **Done right, the resolve stays on chip:** "the MSAA resolve is performed on-chip and only then written out to system memory".
- **Screen-space anti-aliasing** (FXAA, SMAA): "They do require more memory bandwidth, which is usually at a premium on mobile and embedded devices."
- **Screen-space corner darkening:** "it is recommended that the algorithm implements a form of hierarchical-Z buffer (Hi-Z, HZB) optimisation".
- **Shadows:** "Techniques that require results to be written to off-chip memory, such as shadow mapping, will usually perform worse than techniques that can be computed entirely in on-chip memory."
- **Sprites:** fit the mesh to the picture: with a quad round a circular sprite "22% of the fragments processed are redundant", with a twelve-sided shape 3%.

### 4.6 Godot 4.7.2's Mobile renderer, checked in the source

- **MSAA works on Mobile and resolves on the chip.** The render pass adds a resolve target ("Add color resolve.") and the final colour step reads the resolved colour as an input attachment ("// Read from our (resolved) color buffer."). The multisampled pictures are marked discardable, so they are not written out, but they are still allocated: "// TODO: Detect when it is safe to use RD::TEXTURE_USAGE_TRANSIENT_BIT for RB_TEX_DEPTH, RB_TEX_COLOR_MSAA and/or RB_TEX_DEPTH_MSAA." *Computed here:* at full resolution 2× MSAA allocates 41.5 MB. Godot's own documentation is more cautious than Imagination: "2× MSAA may be usable in some circumstances, but higher MSAA levels are unlikely to run smoothly on mobile GPUs." And MSAA leaves cut-out edges as they are: "Therefore, MSAA does not reduce transparency aliasing for materials using the Alpha Scissor transparency mode (1-bit transparency)."
- **What breaks the merged pass:** reading the screen or depth picture ("// can't use our last two subpasses because we're reading from screen texture or depth texture"), glow, auto exposure, depth of field ("// can't do blit subpass because we're using post processes"), FXAA and SMAA ("// Can't do blit subpass because we're using screen space AA."), and 3D scaling. MSAA does not.
- **Alpha to coverage** (MSAA's way of smoothing cut-out edges) exists on Mobile, but Godot turns blending on for it ("// If any form of Alpha Antialiasing is enabled, set the blend mode to alpha to coverage."; the blend mode's attachment sets `enable_blend = true`) and draws such materials in the see-through pass (`has_base_alpha = (uses_alpha && (!uses_alpha_clip || uses_alpha_antialiasing))`). On this chip that is the costly "alpha blended edge". The docs: "Alpha to coverage has a moderate performance cost".
- **No soft sun shadows that widen:** "PCSS for directional lights is only supported in the Forward+ rendering method, not Mobile or Compatibility." Mobile's constant soft filter is off by default ("on mobile devices, due to performance concerns or driver support") but can be set.
- **The sun's shadow holds still:** Godot fits a sphere round the view ("radius *= texture_size / (texture_size - 2.0); //add a texel by each side") and snaps it to whole shadow texels ("// This trick here is what stabilizes the shadow (make potential jaggies to not move)"), so shadow edges hold still in pans and turns.
- **The sun can shadow only some things:** "The light will only cast shadows using objects in the selected layers." (Light3D's caster mask).
- **No screen-space corner darkening on Mobile:** "This feature is only available when using the Forward+ and Compatibility renderers, not Mobile."
- **Banding:** debanding "uses a fast dithering filter just before transforming floating point color values to integer color values to make banding significantly less visible"; on Mobile it works in the materials and in the final step; it is skipped when the picture is 16-bit ("if (rb->get_use_debanding() && !using_hdr) {"). HDR 2D (16-bit floats) "substantially improves the appearance of effects requiring highly detailed gradients".
- **A colour grade costs nothing extra:** the final step on Mobile applies a 3D colour table ("tonemap.use_color_correction = true;") inside the merged pass, after one of Godot's tone curves (Linear, Reinhard, Filmic, ACES or AgX: "layout(constant_id = 12) const bool tonemapper_agx = false;").
- **Texture filters:** Nearest "makes the texture look pixelated from up close, and grainy from a distance (due to mipmaps not being sampled)". Our own mipmaps can be loaded with a picture: `Image.create_from_data` "loads the mipmaps for this image from" the data.
- **Decals are not for crowds:** "When using the Mobile rendering method, only 8 decals can be displayed on each mesh resource."
- **Variable-rate shading** (shading once per 2 × 2 pixels where allowed) is offered, but "if hardware does not support VRS this property is ignored": only the phone can say.

## 5. The options

Each table compares the options on how close they come to the feeling of the pictures, cost on the phone, effort for an AI builder, risk, and fit with the rules that stay. Costs are estimates for study 4 to settle; the phone settles them all.

### 5.1 Reading a texture: which filter keeps texture pixels steady

*Computed here* (`steady/steady.py`): a pixel-art texture is moved under a full-resolution camera by small pans, a 5° turn and a 1.25× zoom, and each fixed point of the world is followed over 24 frames. "Flicker" is how much its shown lightness changes (lower is steadier); "crisp" is the contrast between neighbouring screen pixels, as a share of Nearest's. Two textures: a rich one (the liked camp's ground, re-gridded) and flat pixel art (the art book's).

| Filter | At 2 screen pixels a texel, rich texture: flicker (pan / turn / zoom), crisp | Flat pixel art: flicker, crisp | Closeness to the feeling | Cost | Effort | Risk | Fit |
|---|---|---|---|---|---|---|---|
| A. Nearest (Godot's built-in) | 3.2 / 2.9 / 2.9, 100% | 1.5 / 1.3 / 1.3, 100% | crisp when still; columns 2 and 3 pixels wide that shimmer in motion | 1 read | none | shimmer, grain far off | weak |
| B. Linear | 1.4 / 1.3 / 1.2, 78–81% | 0.7 / 0.7 / 0.6, 86–88% | soft: the pixel look fades | 1 read | none | the softness the owner rejected (25) | weak |
| **C. Smooth pixel** (section 4.2) | **1.8 / 1.7 / 1.8, 94–96%** | **0.8 / 0.8 / 0.8, 97–98%** | crisp texels, edges one pixel soft | 1 read and a few sums | low: one shared function | grazing angles, cut-out edges | best |
| D. Four samples a pixel (supersampling) | as C | as C | as C | 4 times the reads | low | cost | poor value |
| E. Ideal: each pixel's exact average (reference only) | 1.8 / 1.7 / 1.8, 94–96% | 0.8 / 0.8 / 0.8, 97–98% | — | — | — | — | — |

**A** is what the art book's style would do and what Godot gives by default. With a texel of 2.3 screen pixels it shows columns of 2 and 3 pixels that swap as the camera moves: Sassone's "pixel ‘shimmering’ happening when scaling or moving a pixel art image with non-integer values". **B** is steadier only because it is blurrier. **C** matches the ideal average within 0.1 at 1.5, 2 and 3 pixels a texel: it is as steady as one read a pixel can be, and almost as crisp as Nearest. Some flicker remains in any filter (the ideal has it too), and the rich texture flickers about twice as much as flat pixel art, because its neighbouring texels differ more: the cost of the fine grain the owner kept. **E**, the ideal, shows that no filter reading once a pixel can do better than C. A sixth idea, lighting each texel as one (Vibrant Visuals' "quantized" shadows), is a look variant on top of C, not a filter (section 5.4).

### 5.2 Keeping texels about 2 screen pixels at every zoom

At the closest zoom (about 8 m across 1080 pixels, 135 screen pixels a metre), a texture at 64 texels a metre shows each texel as 2.1 screen pixels. At the close camp (20–50 m across) the same texture's texels are 0.3 to 1 screen pixel: they must be replaced.

| Option | Closeness to the feeling | Cost | Effort | Risk | Fit |
|---|---|---|---|---|---|
| A. One texture, Godot's averaged mipmaps | the grey mottle the owner called speckle (answer 31): averages lose 8–28% of the texture's contrast (*computed here*) | none | none | the look fails past the closest zoom | weak |
| B. One texture, Nearest, no mipmaps | sharp but shimmering: about 1.5 to 3.5 times the flicker of mipmapped reading when texels are a screen pixel or smaller (*computed here*) | none | none | shimmer, "grainy from a distance" | weak |
| C. Designed levels, one level for the whole view, swapped as the zoom passes each size | right at each zoom | 1 read | content work | a visible swap mid-pinch; uneven where the view is uneven | fair |
| **D. Designed levels loaded as the texture's own mipmaps, picked per pixel by the texel's size on screen, blended over a short range** | right at every zoom and on every slope | 1–2 reads | content work; one shared function | the blend band shows two designs at once | best |

The designed levels are pixel art drawn for their size: at 32, 16, 8, 4, 2 and 1 texels a metre, each texel of one level covering exactly 2 × 2 of the level below, so a level change is a simplification, not a jump. *Computed here*, a crude level made by code (each block takes its most typical texel instead of the average) keeps 78–95% of the contrast where averages keep 72–92%; levels drawn by intent can keep more, choosing what survives (a tuft's silhouette, not its noise). The same rule applies to plants, stones and figures as forms: a smaller drawn version for each size, never a large picture shrunk, which is what made the small plants of answer 21 lose their charm.

### 5.3 The camera up close

The texels' evenness across the screen depends on the lens. *Computed here* (`camera/lens.py`), for a portrait screen and a camera looking down at 40° (P8's zoom set its field of view across the width):

| Camera | A ground texel at the top of the screen, against one at the bottom: width / height | Closeness to the feeling | Cost | Effort | Risk | Fit |
|---|---|---|---|---|---|---|
| A. Perspective, 25° across (P8's lens; A8.4 keeps perspective) | 0.26 / 0.07; the view reaches 14° below the horizon | the far ground squashed and tiny; nothing like the pictures | none | none | texels at the top nearly four times narrower than at the bottom | as decided |
| B. 10° across | 0.62 / 0.39 | closer | none | low | still uneven | fair |
| **C. Long lens, about 5° across** | **0.79 / 0.63** | like the pictures, with a little depth | none | low: a setting, a farther camera | camera far from the ground | good |
| D. Orthographic | 1.00 / 1.00 | the pictures' own view | none | low | the zoom rig and the bent world must take it (A8.4) | good |

On any camera a ground texel looks lower than it is wide, by the sine of the camera's tilt: 0.64 at 40°, 0.57 at 35°. GPT painted square texture pixels everywhere, which no real 3D view can show; the engine's ground will look slightly flattened (section 8). As the camera rises it can widen its lens step by step, since there the ground is far and even (`PRE-29`).

### 5.4 Edges and light

| Option | Closeness to the feeling | Cost | Effort | Risk | Fit |
|---|---|---|---|---|---|
| Edges: none | jagged edges that crawl in motion, small at 390 dpi | none | none | crawl on long edges | fair |
| **Edges: 2× MSAA** | smooth, "MSAA does not introduce any blurriness whatsoever" | "virtually free" (Imagination); 41.5 MB allocated | a setting | smaller tiles; cut-out edges not smoothed | best |
| Edges: 4× MSAA | smoother | "will noticeably impact performance" | a setting | cost | fair |
| Edges: FXAA or SMAA | blur texels, against Hytale's rule and the owner's | a full-screen pass, merged pass lost | a setting | blur | poor |
| **Light smooth, per screen pixel** | the owner's choice (7) | as now | low | — | best |
| Light in steps | rejected (7) | — | — | — | — |
| Shadows and light quantized to texels (Vibrant Visuals) | pixel-art shadow edges; a variant to show the owner once | tiny | low | may read as stepped | option |

### 5.5 Shadows that are "sharp near what casts them and softer as they lengthen"

| Option | Closeness to the feeling | Cost | Effort | Risk | Fit |
|---|---|---|---|---|---|
| A. Godot's sun map, "Hard" (Mobile's default) | hard everywhere; long tree shadows too crisp | low | none | acne at a low sun | misses `PRE-30` |
| B. Godot's map with a soft filter | equally soft everywhere | a few more reads | a setting | grain at low quality | partial |
| **C. B for small casters, plus our own height-field map for big ones** | soft that widens with distance, crisp at the foot | one read a pixel; the map remade when the sun moves a step | medium | the split must not show | best |
| D. Our own shadow map with soft-shadow searching | full control | a second scene pass plus a search per pixel | high: Godot gives shaders no access to its map | parallel system | poor |

### 5.6 Darkening in corners and under things

The owner's answer 7 kept it, and it matters: *computed here*, removing it in the `no-corner-shade` repaint raised the picture's mean lightness from 0.57 to 0.63 and halved its dark share, from 27% to 14%.

| Option | Closeness to the feeling | Cost | Effort | Risk | Fit |
|---|---|---|---|---|---|
| A. Godot's screen-space darkening | not on Mobile | — | — | — | — |
| B. Our own screen-space darkening at full resolution | good, includes moving things | study 4's 0.6–1.45 ms at 2 × 2 becomes about 2.4–5.8 ms (estimate), and the merged pass is lost | high | heat | poor |
| C. The same at half resolution with Imagination's Hi-Z | a little soft | about 0.6–1.5 ms (estimate), merged pass lost | high | as B | fallback |
| **D. Built in: creases baked into the kit's corners at load, painted into textures, a contact map on the ground under things, each area's openness to the sky and its hollows** | good; moving figures need the contact map | near zero a frame; some time at load | medium | gaps where nothing is baked | best |

### 5.7 Dense, airy plants

The owner kept the density and airiness "As I liked" (answer 6), so the only options are savings that do not show.

| Way | What it saves | Shows? |
|---|---|---|
| Solid things drawn first, cut-outs after (Imagination's order) | wasted shading behind leaves | no |
| Cut-out cards meshed tight to their leaves | most of the thrown-away pixels (Imagination: from 22% to 3% for a round sprite) | no |
| Crowns and bushes as a solid core with cut-out cards only at the edge | most hidden leaf layers | no, if the edge stays airy |
| Grass and flowers lit once, as the ground at their foot (Godot's `LIGHT_VERTEX`, as Holland did) | the per-pixel light of every blade | no |
| Plants under about 6 texels on screen drawn into the ground's designed level (study 3) | most copies at the close camp | no, if the level is drawn for it |
| Only large things casting sun shadows; small plants darkened at their foot | shadow-pass work | barely |
| Godot's alpha to coverage for smooth cut edges | nothing: it adds blending | to be measured (section 7) |

### 5.8 Colour, darks and glow

| Option | Closeness to the feeling | Cost | Effort | Risk | Fit |
|---|---|---|---|---|---|
| A. Godot's default 10-bit picture, no debanding | bands in smooth dark light | none | none | the chosen nights band | weak |
| **B. 10-bit with debanding in materials and the final step** | no bands; a fine dither in the darkest tones | a few sums a pixel | a setting | dither visible up close? | best start |
| C. HDR 2D (16-bit) | no bands, no dither | the 2D picture doubles to 20.8 MB a write; half-rate formats on this chip (study 4) | a setting | cost | if B fails |
| **Gentle colour grade by a 3D table in the final step** | finishing touch only: "colour alone doesn't bring the feeling" (answer 24) | about nothing, merged pass kept | low | over-grading | best |
| Godot's glow round fires | soft glow | merged pass lost; a chain of passes at full resolution | low | heat | poor |
| **Glow worked out in the light function**, as light scattered in the air near each fire | the same soft halo | a few sums a pixel near fires | medium | — | best |

### 5.9 People in busy scenes

Two pictures of the same autumn frame (answer 27's) were drawn: mine, `read-aids`, adds only light (a warm rim on each figure's sunlit edge, a soft light on faces, leaves thinned over people, contact shadows); study 1's `autumn-readable` changes the world (calmer leaf colours with a few strong accents, a trodden stage under the people, clothes a step lighter than the litter, light catching heads and shoulders). *Computed here* with study 1's own measure (each figure's centre-against-surround contrast, as a percentile among all points of the picture; higher reads better):

| Figure | Before (`autumn-close`) | Light aids (`read-aids`) | World route (`autumn-readable`) |
|---|---|---|---|
| bow hunter, on the dark trail | 67.7 | 79.9 | 89.0 |
| spear hunter, on the dark trail | 54.5 | 72.2 | 88.5 |
| woman gathering, before a bright bush | 77.5 | 58.4 | 81.2 |
| girl gathering | 87.8 | 82.8 | 84.4 |
| **people, mean** | **71.9** | **73.3** | **85.8** |
| stag / boar | 99.4 / 95.9 | 99.5 / 95.1 | 99.9 / 92.5 |

The light aids lifted the two hunters against the dark trail and lost the two gatherers in front of a bright hazel: a light rim helps only against a darker background. GPT also softened the whole `read-aids` frame (*computed here*: fine detail down 39%, against 16% for `autumn-readable`), which is not part of the aid. The world route lifted all four, by giving each figure a different value from what is behind it. To my eye it also keeps the feeling better: its detail stays crisper and its colours stay within the wood's golds and coppers, only calmer (colour strength 0.081 to 0.069), where `read-aids` looks hazier, with a soft golden rim round each figure. The owner will see both side by side. How each fits `PRN-10` and `WLD-13` is in section 6.11.

## 6. What we'd recommend

### 6.1 The picture

- **The world draws straight into the screen's own picture at full resolution,** 1080 × 2404 in portrait, with no low-resolution picture in between; the interface draws over it as now.
- **2× MSAA on the 3D world** (`rendering/anti_aliasing/quality/msaa_3d`). No FXAA, SMAA or TAA: they blur texels, and the first two end the merged pass. No Godot glow, auto exposure or depth of field, for the same reason.
- **Keep the merged pass:** nothing in the main picture reads the screen or the depth picture. The water's shore line comes from height (section 6.8); smoke is drawn without a screen copy (section 6.10).
- **Drawing order:** solid things, then cut-out plants, then the few blended things (smoke, mist, glints), as Imagination advises.
- **Half resolution only where it cannot show** (answer 25): the water's mirror, a big smoke volume, and a screen-space darkening pass only if the phone ever needs one. Never the main picture.
- **Portrait and landscape:** the texel's size on screen is set from the screen's short side, so it is the same in both (`PLT-02`).

### 6.2 Steady texture pixels

- **One shared sampling function** (rule 4) reads every texture: the smooth-pixel filter, on textures stored with blending and mipmaps on (Godot's `filter_linear_mipmap` hint), read with `textureGrad`.
- **The level** comes from the texel's size on screen, kept between about 1.4 and 2.8 screen pixels, with a short blend to the next level, each read the same crisp way.
- **The ladder:** 64 texels a metre for the closest zoom (2.1 screen pixels at 8 m across), then levels drawn at 32, 16, 8, 4, 2 and 1 a metre. *Computed here:* at 12 m across the view moves from 64 to 32 a metre, at 24 m to 16, at 50 m to 8, at 100 m to 4, and at the camp zoom (300 m) to 2. A person up close is about 100 texels tall. **For study 7:** the base is 64 a metre, not 32; 32 a metre would show 4.2 screen pixels a texel at the closest zoom, coarser than the 2 × 2 the owner chose, and is the close camp's level.
- **Levels are drawn, not averaged,** each texel of a level covering exactly 2 × 2 of the level below, loaded as the texture's own mipmaps.
- **Moving patterns step in whole texels** about 10 times a second: the river's flow lines, foam, ripples, flames and smoke cards, as poses do (`PRE-44`) and as t3ssel8r's fire does. Gliding by fractions of a texel smears the pixel look. This is a look choice for the owner's eye at the first review.
- **Thin things** (blades, twigs, poles, shafts) are at least one texel wide; below about one screen pixel they switch to a coarser drawn form.
- **Glints** on water, wet stone and snow are whole texels that live a few frames, placed by a hash of their place in the world, so they sparkle but never shimmer.
- Texels sit on surfaces, so swaying grass and walking people carry them along without swimming. Sun shadows hold still in pans and turns (Godot's sphere and snaps); during a pinch their texels change size, which the soft filter hides (a phone check, section 7).

### 6.3 The camera up close

- **A long lens, about 5° across in portrait, or orthographic,** from the person to the close camp, tilted about 35–40° as in the pictures; the lens widens step by step as the camera rises (`PRE-29`), and the globe stays in perspective. A long lens is still a perspective camera, so the sphere world of A8.4 is kept.
- **No pixel lock.** Turns and zooms are free; "ease" to 5° and 1.25× steps is no longer needed for steadiness and becomes a feel setting.
- Ground texels look about 0.6 as tall as wide at this tilt; the level is chosen by the texel's area on screen.

### 6.4 Light: one shared light function

Each material hands the function its texel colour (read as in 6.2), its facing, its openness to the sky, its crease darkening, its wetness and snow, and the function adds:
- **The sun,** its colour and strength keyed by its height (Vibrant Visuals; P8's sky maths). True midday is the real white-warm noon (answer 9).
- **The sky's fill,** coloured by the sky and scaled by openness. *Measured on the chosen pictures:* by day the shade is close to neutral (OKLab yellowness −0.003 to +0.019), at true midday a little blue (−0.013), at night clearly blue (−0.032 to −0.051).
- **Light bounced from the ground,** tinted by the sunlit cover below: the "warm light bounced from the meadow into the shade" of the chosen look.
- **Backlight:** leaves and grass glowing when the sun is behind them, rims on edges that catch a low sun, glints on water. This is what keeps the feeling when facing the sun (answer 16).
- **Fire,** from the light grid (study 3), colour and flicker by heat. *Measured on nights A and B* (`light/fire_reach.out`): a small hearth's light over the moonlit ground halves by about 1.75–2.25 m and falls to a tenth by about 3.25–4.25 m; its warm tint ends at about 2–2.5 m. Fire shadows come from a small map round each fire (about 128 × 128 texels), remade 10–20 times a second as people move, instead of round 1's walk of up to 32 steps for every pixel, which full resolution makes four times dearer.
- **Fire's glow** as light scattered in the air near each fire, worked out per pixel from the same fire list: the soft halo without a glow pass.
- **The moon** as the same directional light at night: silver-blue, its strength by phase (`WLD-07`). *Measured on nights A–C:* 85–87% of the picture dark, greens almost gone (2–6% of pixels), the brightest twentieth of pixels only up to lightness 0.55–0.60, people and tents still readable.
- **Haze and mist** by distance, warmer toward the sun; mist banks over water and hollows, drifting with the wind.
- **Snow** as a cover on faces turned up, by slope and shelter, trampled where the world's traces say. *Measured on winter B:* sunlit snow at lightness 0.93 and slightly cream (+0.036), shaded snow at 0.79 and blue (−0.041); 13% of pixels near the top of the range, so the tone curve needs a soft shoulder.
- **Textures carry colour, grain and crease darkening, never sunlight from one side.** Hytale paints light into textures; our sun goes round and the camera turns, so painted sun would face the wrong way half the day.

*Measured targets by moment, on the chosen pictures* (`light/light_measures2.out`; study 6's target card holds the full set):

| Moment (picture) | Mean lightness | Share dark | Lights | Shade |
|---|---|---|---|---|
| Late afternoon, direction B (`d2-pixel-paint`) | 0.55 | 32% | golden (+0.108) | neutral, slightly warm (+0.019) |
| True midday (study 1's `close-camp-noon`) | 0.61 | 22% | warm (+0.076) | slightly blue (−0.013) |
| Dusk, relit (study 6's `relight-dusk`) | 0.42 | 66% | warm and red (+0.084, red +0.059) | neutral (−0.009) |
| Night A, B, C | 0.32–0.36 | 85–87% | — | blue (−0.032 to −0.051) |
| Winter B | 0.66 | 21% | pale gold (+0.031) | blue (−0.019) |
| Rain (`rain`) | 0.47 | 46% | grey, the gold gone (+0.016) | neutral (−0.003) |

### 6.5 Shadows

- **Small things** (people, animals, tools, baskets, stones, tufts) cast through Godot's sun map, its caster mask set to their layers only, with a soft filter set on Mobile (the `.mobile` override, default "Hard") and sized so the soft edge is about one texel at the closest zoom.
- **Big things** (ground, cliffs, trees, tents, huts) cast through our own **height-field sun map:** for each texel of the area in view, the steepest angle of anything between it and the sun, read from the area's height maps of tops and undersides (the kind P3 drew round fires). The shadow fades over a band that widens with the distance to what casts it, so long shadows go soft and shadows at the foot stay sharp (`PRE-30`). It is remade only when the sun moves a step, as a small two-dimensional pass (no compute program), and read once per pixel. Walls and crowns above the ground read a stored shadow height for their texel.
- **Cloud shadows** as the weather's clouds moving over the ground (round 1).
- **A variant to show the owner once:** shadows quantized to texels, as Vibrant Visuals can.

### 6.6 Darkening in corners and under things

- **The kit:** darkening baked into each shape's corners, creases and undersides when the code builds it at load.
- **Textures:** crease and cavity darkening painted in, as Hytale does (study 5).
- **On the ground:** a small top-down contact map round the camera. Each thing's footprint darkens the ground under it: stones, logs and baskets once; people and animals every frame as soft ellipses. One read in the ground's shader. Godot's decals cannot do it (8 a mesh on Mobile).
- **Per area,** baked when the area is made: openness to the sky and the darkness of hollows, so light enters under overhangs and stays out of caves (`PRE-24`), as round 1 proposed.
- **Plants** darker toward their base; the ground under dense cover darker by the cover's density.
- **No screen-space pass,** unless the phone shows gaps; then at half resolution with Imagination's Hi-Z.

### 6.7 Colour

- **A tone curve with a soft shoulder** (Godot's AgX or Filmic, or our own built into the colour table), so snow, fire and glints keep their texture.
- **A gentle colour table per moment and biome,** blended as Vibrant Visuals blends biomes: a finishing touch, never the look itself (answer 24).
- **Debanding on,** in materials and the final step (Mobile uses both). *Computed here* (`light/banding.out`): in Godot's 10-bit picture one step spans 6–7 screen levels in the darkest tones (levels 0–12), 3–5 in levels 12–24 and 2–3 in levels 24–36; nights A–C put 2–14% of their pixels below level 36 and 30–41% below level 60. Smooth moonlit gradients there would band. If the dither shows, or bands remain, switch to HDR 2D.
- The interface stays in the 8-bit 2D picture, exact as drawn.

### 6.8 Water

- **The bed** is drawn by the ground's own shader below the water's level, tinted toward teal and dark by depth, its texture coordinates gently wobbled for refraction: clear water with no screen read (round 1's proposal, kept).
- **The surface** draws only its features: the sky's colour by angle; reflections from the mirrored pass at half resolution, with a reduced set of things in it (study 4); ripples, flow lines along the current and foam at fords, all stepping in whole texels; glints of one texel; rain rings; and the thin bright line where water meets land or anything in it, worked out from height (`PRE-26`).
- **Mist** on water at dawn, from the haze function.

### 6.9 Plants and small things

- All the savings of 5.7, none of the cuts: solid first, tight meshes, solid cores for crowns and bushes, light once per card for grass and flowers, small plants into the ground's levels below about 6 texels, sun shadows only from large things.
- **Small plants drawn for their size** (answers 21 and 31): a tuft about 40 screen pixels tall at the closest zoom is drawn as about 20 texels of pixel art; the close camp has its own smaller drawn form; never a large painting shrunk (study 5).
- Wind sways the vertices, so texels travel with the leaves.
- Trees far off become impostor cards (A8.3), drawn as pixel art for their size, never a shrunk render.

### 6.10 Effects in the ten pictures

- **Rain, all four layers** (answer 17), from the world's weather (`WLD-16`):
  - streaks: thin copies one texel wide, slanting with the wind, moved by the chip from a seed and the time;
  - splashes and wet shine: brief one-texel splashes on faces turned up, in the materials' own shaders; surfaces darken and gain glossy highlights with wetness; puddles in hollows reflect the sky;
  - ripples: rings in the water shader;
  - drifting mist: the haze function with moving noise, or a few soft cards.
  
  *Measured on `rain`:* lightness 0.47, 46% dark, the gold gone from the lights, and colour strength among the lowest measured (only the storm's is lower).
- **Snow falling:** flakes of one or two texels as copies; the cover as in 6.4.
- **Fire:** code-made pixel-art flames at about 10 frames a second, core, flames and embers at different rates; embers as copies; the glow in the light.
- **Smoke:** soft, lit cards meshed tight to their shape, drawn last; a wildfire's smoke as round 1's marched volume at half resolution in its own picture, used as a texture by cards in the main pass.
- **Lightning:** the flash is a change of the light for two or three frames (a cool, bright light with sharp shadows); the bolt is a bright mesh.
- **Sea foam:** a band along the shore from the distance to it, stepping in whole texels.
- **Never Godot's GPU particles:** a compute program that reads textures, which A4.3 rules out.

### 6.11 People in busy scenes, and `PRN-10` and `WLD-13`

- **The world route (study 1) fits both rules when it is true:** trodden ground where people really walk and work (the world's traces, `MAT-08`), clothes in colours the people really make, leaf colours the species really have, the real sun on heads and shoulders. Nothing reads the camera. It lifted all four people (mean 71.9th to 85.8th percentile).
- **Light aids, one by one:**
  - *A rim of light from the real sun or fire:* fits both rules if every surface gets it from the shared light function. People then stand out because hair, fur and hide catch backlight, a fact of their materials. It lifted the hunters on the dark trail.
  - *A soft light that follows each person:* fails `PRN-10`, "Nothing is added for show": a light with no source, which at night would suggest a fire that is not there. It does not break `WLD-13`, since the world never reads it. Not recommended.
  - *Leaves thinning over a person, only in the picture:* `WLD-13` holds (the leaves stay in the world, and nothing reads the camera), but the picture shows less than is there. It is a viewing convention, like the cut-away (`PRE-25`). Recommended only as a tool the owner calls up, for example on the person they follow or tap, and only with their OK.
  - *Contact shadows:* real; fit both.
- **They combine:** the world route first, with the real sun's rim from the shared function; no follow light.
- **Small figures far off** keep answer 4's way, which `PRE-28` already decides: a simplified form with a dark outline (its back faces drawn a little larger behind it), a light face and tunic, an enlarged tool, for small sizes only.

### 6.12 Round 1's proposals, under direction B

| Round 1 proposal | Now |
|---|---|
| 1. Rich pixel art on a grid (B), graded pixels (C) on a switch | Replaced by direction B |
| 2. Orthographic camera through the camp stop | Kept in a new form: a long lens or orthographic up close, for even texels, not for a pixel lock |
| 3. Art-pixel bands of whole sizes | Replaced by designed texture levels picked per pixel |
| 4. Exact colours by codes, HDR 2D and a resolve step | Dropped: no palette; debanding instead |
| 5. Water drawn in the light function, no screen reads | Kept |
| 6. Smoke and mist as steps | Changed: soft cards and haze |
| 7. Sun shadows at texel centres, softened by height fields | Softening kept as the height-field map; texel-centred shadows become a variant |
| 8. Openness baked per area | Kept, and more important |
| 9. Outline data with part IDs | Dropped up close; small far figures only |
| 10. Texture levels by zoom band | Kept, as designed mipmaps picked per pixel |
| 11. `PRE-03`'s 58-pixel person | Study 1's wording (a person about 200 screen pixels tall up close) |

### 6.13 Proposals for the owner

Each needs the owner's OK (`PRC-07`). Study 1 proposes wording for the same items from the look's side; the builder will merge the two.

1. **`PRE-01` Detailed pixel art becomes "Pixel-art surfaces":** "The world is a smooth, sharp 3D world drawn at the screen's full resolution, and every surface wears a texture painted as pixel art, its square texture pixels about 2 by 2 screen pixels at every zoom (`PRE-22`); light and shade are smooth, and nothing blurs the texture pixels." *Done when:* at every zoom stop, texture pixels on ground, rock and figures measure 1.5 to 3 screen pixels across, and none is smeared.
2. **`PRE-02` Pixel-rendered 3D:** "A real 3D world drawn at low resolution" becomes "A real 3D world drawn at the screen's full resolution", the rest unchanged. It was not among the lifted items, but direction B needs it.
3. **`PRE-20` Colour in steps becomes "Painted surfaces":** "Every material wears a texture painted as pixel art in its own shades, with fine grain at the texture pixel, made from its colour (`MAT-10`) or by hand for common ones; the light (`PRE-30`) is smooth, and textures carry no sunlight from one side." *Done when:* texture pixels hold still as the camera pans, and no ground turns to speckle at any zoom.
4. **`PRE-21` Outlines and lit edges becomes "Edges":** "No outlines up close; edges are smooth, and the sun or a fire lights the rims of what it catches from behind (`PRE-30`). Small, far figures are drawn to read: an outline, a light face and tunic, and enlarged tools (`PRE-28`)."
5. **`PRE-22` Stable pixels becomes "Steady texture pixels":** "Texture pixels never crawl, flicker or shimmer as the camera pans, turns or zooms: each is crisp inside and blended over at most one screen pixel at its edge, and each zoom uses a level of its texture drawn for that size, so a texture pixel stays between about 1.5 and 3 screen pixels, in portrait and landscape." *Done when:* a pan, a turn and a pinch over the busiest scene flicker no more than the ideal average (the check in section 7) and the owner approves them on the phone.
6. **`MIL-09`** follows: "a smooth, sharp 3D world with pixel-art textures, steady at every zoom, smooth light with real shadows of the hours and seasons, rock faces, water, and the model kit and its textures".
7. **`ARCHITECTURE.md` A4.1:** points 1 to 4 and 8 rewritten as sections 6.1 to 6.7 and 6.10; point 5 keeps fire, with fire shadows from small maps; point 6 keeps water without the depth read.
8. **A4.2:** one shared sampling function and one shared light function; "never fine grain" removed from the made-things material.
9. **A4.3, the phone rules:** add 2× MSAA; no FXAA, SMAA, TAA, glow, auto exposure or depth of field; nothing in the main pass reads the screen or depth picture; debanding on.
10. **A4.4, tests:** add the steady-texel check, the texel-size check per zoom stop, and the night banding check (section 7).
11. **A5.3 One texel density becomes "A texel ladder":** 64 texture pixels a metre at the closest zoom, with levels drawn at 32, 16, 8, 4, 2 and 1 a metre.
12. **A8.4 The camera:** from the person to the close camp, a long lens (about 5° across in portrait) or orthographic, tilted about 35–40°, widening as it rises; perspective at the globe; no pixel lock; ease a feel setting.

## 7. Questions only the phone can answer

1. **What does the full-resolution picture cost?** The liked camp at the closest zoom and the busiest close camp, at 1080 × 2404 with 2× MSAA, timed by Godot's per-viewport GPU time at a 120-frame cap (P1's method) and Perfetto; with and without plants; and once at half resolution, to split the cost per pixel from the cost per triangle (study 4's stress scenes).
2. **Is 2× MSAA nearly free on this chip?** The same scenes with MSAA off, 2× and 4×, dense plants included, since smaller tiles mean more tiling work.
3. **Which way to cut out leaves?** One dense meadow drawn four ways: cut-out by discard, Godot's alpha to coverage (blended), meshes cut tight with discard, and solid cores with cut-out edges. Time each, and judge the edges in motion.
4. **Do texture pixels hold still?** A scripted pan, a 5° turn and a 1.25× pinch over the busiest scene, captured frame by frame on the phone. The engine knows where each world point is, so the flicker of section 5.1 can be computed on real frames, smooth against Nearest, and compared with the cloud's ideal render (study 6). Then the owner's eye on the same clips.
5. **Do level changes show in a pinch?** A slow pinch from 8 m to 50 m across over ground, rock and a meadow; per-frame differences at fixed points; the owner's eye.
6. **Do the nights band?** Nights A–C rebuilt in the engine, at the owner's usual brightness: 10-bit plain, with debanding, and HDR 2D. Count the distinct screen levels across a moonlit slope, and look.
7. **Are shadows steady and clean?** A 5° sun and a pinch: acne, and edges that move; the cost of Godot's soft filter levels on Mobile.
8. **Does remaking the sun's height-field map hitch?** The slowest frame while the sun steps, at real speed and at speed.
9. **Heat:** the 20-minute run at full resolution (study 4).
10. **Does this chip offer variable-rate shading,** and would shading only the softest parts once per 2 × 2 show? Godot reports support; if offered, a side by side for the owner's eye, used only if needed (answer 25).
11. **The display:** judge on fixed settings ("Natural" or "Adaptive", the same each time), as round 1 found.

## 8. Risks

1. **Full resolution with the liked density costs too much** (study 4: plants dominate). *Retire early:* questions 1–3 in M2's first build, before content grows. Fallbacks, in order: the savings of 5.7, a lighter mirror and smoke, then half-resolution shading of the softest parts, or variable-rate shading, where the owner approves it.
2. **The fine grain still flickers too much in turns.** *Retire:* question 4 early, with the owner; levels drawn a little calmer near the end of their range.
3. **Designed levels are much content work.** *Retire:* code-made levels for code-made textures first (study 5); ground, rock and plants first; one level checked at each zoom stop in every review.
4. **Ground texels look flattened by the camera's tilt,** where GPT painted them square. *Retire:* an engine frame of the liked camp early (study 6's "first light"), at tilts of 35°, 40° and 50°, for the owner's eye.
5. **A long lens or an orthographic camera clashes with the zoom to the globe** (A8.4). *Retire:* build the camera rig first and fly the whole zoom in the cloud and on the phone.
6. **The chosen nights band.** *Retire:* question 6; debanding first, HDR 2D second.
7. **The PowerVR driver mishandles MSAA, alpha to coverage or `textureGrad`.** Round 1 found Godot issues of this kind on related chips, read through a summary: 115171, a crash creating the pipeline cache on a Pixel 10 Pro, and 123614, a workaround for PowerVR shader compilers that are not thread-safe. *Retire:* a crash-safe probe of each feature in the first phone build, noted before it is tried (A4.3).
8. **People stay hard to find in busy scenes.** *Retire:* the world route; study 6's salience check on every busy test scene; see-through foliage only as a tool, with the owner's OK.
9. **A light that follows people, or other aids "for show", creep in.** *Retire:* every aid named in the shared light function, and checked against `PRN-10` at review.

## 9. For other studies

- **Study 1 (the look):** the camera's lens and tilt (6.3) are look decisions; ground texels show about 0.6 as tall as wide at 35–40°; the light targets of 6.4; the readability results of 5.9 and 6.11 (the world route works); textures should not carry sunlight from one side; my proposed wording for `PRE-01`, `PRE-02`, `PRE-20`, `PRE-21` and `PRE-22` (6.13), to merge with study 1's own.
- **Study 3 (the engine):**
  - one shared sampling function and one shared light function in the shader library;
  - designed levels as texture arrays whose mipmaps are our own data (`Image.create_from_data`);
  - the small world-space maps (the sun's height-field map, fire shadow maps, the contact map, openness per area) as two-dimensional passes into small pictures, no compute;
  - the light grid carrying fires and, for contact darkening, the soft footprints of people and animals;
  - the camera rig with a long lens up close; the texel's screen size set from the screen's short side in both orientations;
  - small far figures outlined by their own back faces, for small sizes only;
  - answer to study 3's question on foreshortening at 40°: nothing to lock; levels by texel area; accept the flattened look or tilt more.
- **Study 4 (the phone):**
  - 2,596,320 pixels shaded once each, plus extra shading at the edges MSAA finds;
  - about 10–14 texture reads a pixel in the main pass (texture with its level blend, sun shadow taps, height-field map, clouds, openness, contact, fire grid and fire shadows, haze), estimated;
  - 2× MSAA allocates 41.5 MB, resolves on chip, and may shrink tiles;
  - Godot's alpha to coverage runs blended, in the see-through pass;
  - HDR 2D doubles the 2D picture to 20.8 MB a write;
  - the world-space maps cost little and only when remade; the mirror at half resolution; no glow pass; fire glow and debanding are a few sums a pixel;
  - darker nights also save screen power on OLED.
- **Study 5 (content):**
  - textures and sprites drawn at the size they show: 64 texels a metre up close, a 40-pixel tuft as about 20 texels;
  - every coarser level drawn for its size, each texel covering 2 × 2 of the level below;
  - crease darkening painted in, sunlight never;
  - small plants' smaller drawn forms for the close camp (answer 21);
  - people's clothes and the ground under them chosen so figures differ in value from what is behind them, truthfully (the world route).
- **Study 6 (the loop):**
  - the steady-texel check of question 4 (flicker at fixed world points against the cloud's ideal render);
  - the texel-size check per zoom stop (1.5 to 3 screen pixels);
  - the night banding check;
  - the salience check on busy scenes;
  - smooth against Nearest in motion as an early picture for the owner.
- **Study 7 (the pipeline):**
  - the texel density is 64 a metre at the base, with designed levels below (6.2);
  - designed mipmaps travel with each texture (KTX2 can carry them, or they are made at load);
  - Godot's own advice is that "pixel art" textures "should have VRAM compression disabled as it will negatively affect their appearance" (study 4's round-1 source); whether ASTC 4 × 4 keeps our 2-pixel texels crisp is for the owner's eye on the phone.

## 10. Pictures from GPT

Eleven of my 16 pictures used. All are repaints of an input picture; GPT keeps the layout but redraws every pixel.

| Picture | Path (under `R/work2/2/gpt/`) | The question | What it showed | The owner's answer |
|---|---|---|---|---|
| outlines | `outlines.png` | does the feeling need outlines and lit edges? | the liked camp with clean outlines; crisper, more "game-like" | no outlines up close (7) |
| stepped-light | `stepped-light.png` | smooth or stepped light? | three flat levels of light; harder, toon-like | smooth (7) |
| outlines-small | `outlines-small.png` | do outlines help at close-camp size? | outlines on a busy lake scene shrunk to game size | answered by study 3's picture: drawn to read, far off only (4) |
| noon | `noon.png` | can noon keep the feeling if kept warm? | a warm, golden noon | not chosen: true midday instead (9) |
| calm-texture | `calm-texture.png` | how calm can surfaces be? | less fine grain, steadier | the fine grain kept (7) |
| no-corner-shade | `no-corner-shade.png` | how much do corners and contact darkening carry? | without it: lighter, flatter, dark share halved (27% to 14%) | kept: we build it (7) |
| backlight | `backlight.png` | does facing a low sun keep the feeling? | rims, long shadows toward the viewer, a glittering river | yes (16) |
| night | `night.png` | how dark, how blue, how far firelight reaches? | 85% dark, blue moonlight, the hearth's light halving by about 1.75 m and fading by about 3.25 m | chosen with A and C (10) |
| winter | `winter.png` | how snow and winter light look | cream snow in sun, blue in shade, slush on paths, a melted ring round the hearth, snow off steep faces | chosen, the only one (11) |
| rain | `rain.png` | which layers of rain matter? | streaks, splashes and wet shine, ripples, mist; the gold gone | all four (17) |
| read-aids | `read-aids.png` | can light alone help people read in a busy wood? | gentle rims and lighter faces; the hunters lifted, the gatherers lost; the whole frame softened | measured in 5.9; to be put before the owner beside study 1's `autumn-readable` |

## 11. Sources

**Godot 4.7.2 source** (tag `4.7.2-stable`, read in a full checkout of the tag):
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/forward_mobile/render_forward_mobile.cpp
  - "// Add color resolve."
  - "blit_pass.input_attachments.push_back(color_buffer_id); // Read from our (resolved) color buffer."
  - "// can't use our last two subpasses because we're reading from screen texture or depth texture"
  - "// can't do blit subpass because we're using post processes"
  - "// Can't do blit subpass because we're using screen space AA."
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.cpp
  - "// TODO: Detect when it is safe to use RD::TEXTURE_USAGE_TRANSIENT_BIT for RB_TEX_DEPTH, RB_TEX_COLOR_MSAA and/or RB_TEX_DEPTH_MSAA."
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/forward_mobile/scene_shader_forward_mobile.cpp
  - "// If any form of Alpha Antialiasing is enabled, set the blend mode to alpha to coverage."
  - "// These flags should only go through if we have some form of MSAA."
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/forward_mobile/scene_shader_forward_mobile.h
  - "bool has_base_alpha = (uses_alpha && (!uses_alpha_clip || uses_alpha_antialiasing));"
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/storage_rd/material_storage.cpp
  - "case BLEND_MODE_ALPHA_TO_COVERAGE: {" followed by "attachment.enable_blend = true;"
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_scene_cull.cpp
  - "radius *= texture_size / (texture_size - 2.0); //add a texel by each side"
  - "// This trick here is what stabilizes the shadow (make potential jaggies to not move)"
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/renderer_scene_render_rd.cpp
  - "tonemap.use_color_correction = true;"
  - "if (rb->get_use_debanding() && !using_hdr) {"
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/shaders/effects/tonemap_mobile.glsl
  - "layout(constant_id = 12) const bool tonemapper_agx = false;"
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/shaders/particles.glsl
  - "#[compute]"
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/shader_language.cpp
  - the built-in function table's lines for `textureGrad`, `textureLod` and `dFdx`, for example "{ "textureGrad", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC2, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }"
- Class reference sources at the same tag (`doc/classes/`):
  - https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/doc/classes/BaseMaterial3D.xml: "This makes the texture look pixelated from up close, and grainy from a distance (due to mipmaps not being sampled)."
  - https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/doc/classes/Light3D.xml: "PCSS for directional lights is only supported in the Forward+ rendering method, not Mobile or Compatibility."; "The light will only cast shadows using objects in the selected layers."
  - https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/doc/classes/ProjectSettings.xml: "uses a fast dithering filter just before transforming floating point color values to integer color values to make banding significantly less visible."; "on mobile devices, due to performance concerns or driver support."; "substantially improves the appearance of effects requiring highly detailed gradients."
  - https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/doc/classes/Decal.xml: "When using the Mobile rendering method, only 8 decals can be displayed on each mesh resource."
  - https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/doc/classes/Image.xml: "loads the mipmaps for this image from"
  - https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/doc/classes/Viewport.xml: "Note, if hardware does not support VRS this property is ignored."

**Godot 4.7 documentation:**
- https://docs.godotengine.org/en/4.7/tutorials/3d/3d_antialiasing.html
  - "MSAA does not introduce any blurriness whatsoever."
  - "Therefore, MSAA does not reduce transparency aliasing for materials using the Alpha Scissor transparency mode (1-bit transparency)."
  - "Alpha to coverage has a moderate performance cost, but it's effective at reducing aliasing on transparent materials without introducing any blurriness."
  - "2× MSAA may be usable in some circumstances, but higher MSAA levels are unlikely to run smoothly on mobile GPUs."
- https://docs.godotengine.org/en/4.7/tutorials/3d/environment_and_post_processing.html (the section on screen-space ambient occlusion)
  - "This feature is only available when using the Forward+ and Compatibility renderers, not Mobile."
- https://docs.godotengine.org/en/4.7/tutorials/performance/gpu_optimization.html
  - "Even in 3D, "pixel art" textures should have VRAM compression disabled as it will negatively affect their appearance, without improving performance significantly due to their low resolution."

**Imagination Technologies, PowerVR Graphics Recommendations** (undated pages):
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/msaa-performance.html
  - "2x MSAA is virtually free on most PowerVR graphics cores (Rogue and Volcanic onwards), while 4x MSAA+ will noticeably impact performance."
  - "This is partly due to the increased on-chip memory footprint, which results in a reduction in tile dimensions"
  - "Performing MSAA becomes costlier when there is an alpha blended edge"
  - "To mitigate these costs, submit all opaque geometry first"
  - "In some cases, there may be no need for anti-aliasing to be used at all, for example when the target device’s display has high pixels per-inch (PPI)."
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/performing-msaa-on-powervr-using-vulkan.html
  - "the MSAA resolve is performed on-chip and only then written out to system memory"
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/preferred-analytical-aa-solution.html
  - "They do require more memory bandwidth, which is usually at a premium on mobile and embedded devices."
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/screen-space-ambient-occlusion.html
  - "it is recommended that the algorithm implements a form of hierarchical-Z buffer (Hi-Z, HZB) optimisation"
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/preferred-shadowing-solution.html
  - "Techniques that require results to be written to off-chip memory, such as shadow mapping, will usually perform worse than techniques that can be computed entirely in on-chip memory."
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/efficient-sprite-rendering.html
  - "if a sprite is circular in shape and is rendered using the most optimal fitting quad, 22% of the fragments processed are redundant"
  - "the amount of wasted fragment processing can be reduced to just 3%"

**t3ssel8r** (video descriptions as published in the channel's own YouTube feed, https://www.youtube.com/feeds/videos.xml?channel_id=UCIjUIjWig0r5DIixQrt6A3A; the videos themselves were not watched; 2020 to 2023, so not new):
- "Crafting a Better Shader for Pixel Art Upscaling", 26 May 2023, https://www.youtube.com/watch?v=d6tp43wZqps
  - "In this episode, we explore a subject that was only tangentially related to my game project: rendering pixel art with anti-aliasing. Applications for this technique range from anti-aliasing for pixel-art textures in low-poly games like Minecraft, to games that primarily feature pixel art sprites in 3D environments, like Octopath Traveler."
- "Pixel Art Rain Shader", 30 November 2020, https://www.youtube.com/watch?v=ony4o3E0J20
  - "Selling the atmosphere of rain is a matter of layering together a bunch of individual effects:"
  - "splashes on all gently-sloped ground"
  - "mist blowing around the scene"
  - "The downside is that the effect does not show well while the camera is rotating."
- "Procedural Pixel Art Fire", 23 November 2020, https://www.youtube.com/watch?v=erQ8PvxIjS8
  - "Different parts of the fire (core, flames, embers) run at different frame rates to emulate a hand-animation look."
- "Parallax Effect for 3D Pixel Art Engine", 11 December 2020, https://www.youtube.com/watch?v=cCUCMBmc9yQ
  - "while maintaining crisp pixel positioning in the foreground"
  - "A depth-based fog helps tie the scene together."

**CptPotato, "Smooth Pixel Filtering"** (GodotThings repository, undated, written for Godot 3): https://github.com/CptPotato/GodotThings/tree/master/SmoothPixelFiltering, readme read at https://raw.githubusercontent.com/CptPotato/GodotThings/master/SmoothPixelFiltering/README.md
- "A shader snippet allowing for "smooth pixelated" filtering, eliminating most aliasing artifacts."
- "Integer scales will remain pixel-perfect while non-integer scales produce a softer, antialiased look."
- "In 3D the same benefits apply but are often more impactful because of the varying perspective and sometimes steep camera angles."
- "*Note: Only the shader using "manual" filtering will work on GLES 2*"

**Hytale, "An Introduction to Making Models for Hytale"** (Thomas Frick, art director, 22 December 2025): https://hytale.com/news/2025/12/an-introduction-to-making-models-for-hytale
- "A modern, stylized voxel game, with retro pixel-art textures"
- "We avoid applying effects that would damage the handcrafted texture style."
- "We paint lights and shadows inside textures and use real lights/shadows to bring everything together."
- "we often paint/bake shadows, ambient occlusion, and highlights directly into the texture to simulate more complex lighting than there really is in the game."
- "Blocks and Characters texture texel density ranges from simple to double"

**Minecraft creator documentation, Vibrant Visuals** (Microsoft Learn; carried over from round 1 and checked again in its saved pages):
- https://learn.microsoft.com/en-us/minecraft/creator/documents/vibrantvisuals/keyframejsonsyntax?view=minecraft-bedrock-stable: "the key is a number from 0 (noon) to 1 (the next noon, 24 hours later)"
- https://learn.microsoft.com/en-us/minecraft/creator/documents/vibrantvisuals/lightingcustomization?view=minecraft-bedrock-stable: "The sky contributes significantly to indirect lighting"
- https://learn.microsoft.com/en-us/minecraft/creator/documents/vibrantvisuals/shadowscustomization?view=minecraft-bedrock-stable: "which represents the resolution in texels (texture units) to which shadows will be quantized"
- https://learn.microsoft.com/en-us/minecraft/creator/documents/vibrantvisuals/biomecustomization?view=minecraft-bedrock-stable: "As the player approaches the Forest biome, their lighting, atmospherics and color grading settings will interpolate between the Plains biome settings and the forest biome settings."

**Others** (carried over from round 1, checked again in their saved pages):
- Gabriel Sassone, "Pixel Art Filtering", 12 April 2021: https://jorenjoestar.github.io/post/pixel_art_filtering/
  - "the pixel ‘shimmering’ happening when scaling or moving a pixel art image with non-integer values"
  - "apply neighbour/nearest filter on pixels that are fully inside a texel, and use bilinear with custom weights at the edges!"
- RetroArch's sharp-bilinear shader: https://raw.githubusercontent.com/libretro/glsl-shaders/master/interpolation/shaders/sharp-bilinear.glsl
  - "Does a bilinear stretch, with a preapplied Nx nearest-neighbor scale"
- David Holland, "3D Pixel Art Rendering", 24 September 2024: https://www.davidhol.land/articles/3d-pixel-art-rendering/
  - "The grass is a number of billboard quads with a grass texture, evenly lit to blend in with the terrain"
- Bevy issue 25788 (open), https://github.com/bevyengine/bevy/issues/25788: read in round 1 through a summarising fetch, and again by the quote checker through WebFetch, twice with the same words: a crash in PowerVR's shader compiler while creating a compute pipeline, before the first frame, on a Pixel 11 Pro XL with driver 25.3@6908880. GitHub refuses the raw page to this session.

**The project's own** (no URL): `PROJECT.md` (`PRE-01` to `PRE-03`, `PRE-20` to `PRE-30`, `PRN-10`, `WLD-13`, `PLT-02`, `PLT-04`, `MIL-09`); `ARCHITECTURE.md` A4, A5.3, A8.4; `LESSONS.md` P1 to P3 and P8; the zoom prototype at `29beda4^` (`prototypes/app/zoom/zoom.gd`: `const FOV := 25.0`, `_cam.keep_aspect = Camera3D.KEEP_WIDTH`, a tilt of 30° at the camp); the owner's answers in `R/owner/ask/answers.md`.

**Computed here** (scratch scripts and their outputs in `R/work2/2/`, nothing for the game):
- `steady/steady.py`, `steady/levels.py`: the flicker and crispness of texture filters under a moving full-resolution camera, and averaged against code-drawn coarser levels (outputs `steady.out`, `levels.out`).
- `camera/lens.py`: texel evenness by lens and tilt, the texel ladder, MSAA and HDR 2D memory (`lens.out`).
- `light/light_measures.py`, `light/light_measures2.py`: study 1's light measures (OKLab) on all the chosen pictures; `light/snow.py` (snow in sun and shade), `light/fire_reach.py` (firelight's fall-off on nights A and B), `light/banding.py` (10-bit steps near black and the nights' dark tones).
- `read/change.py` and study 1's `salience.py` on `autumn-close`, `read-aids` and `autumn-readable` (`read/salience-read.out`, `read/change.out`).
- Quote checks: `checkq.py` against the saved pages (`q_round1.tsv`, `q_new.tsv`, `q_round2.tsv`, `q_round2b.tsv`), all found.


## Addendum by the builder, after this note: the owner's answers to its questions

Answered on 6 October 2026, recorded in full in `R/owner/ask/answers.md`:
- **35, texture pixel size up close:** A, about 2 screen pixels (64 texels a metre at the closest zoom).
- **34, rock surface under code-laid layers:** neither B (GPT's fine surface) nor C (study 5's chunky swatch) keeps the liked cliff's feel yet. **34b, three worlds' cliffs with code-laid layers:** yes, they keep its feel.
- **36, the close camp's ground:** D, GPT's redraw with its colours matched by code.
- **33, people in the busy autumn wood:** A, light only (study 2's read-aids), not the changed world.
