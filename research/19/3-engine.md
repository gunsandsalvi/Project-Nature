# Study 3, round 2: the engine for the chosen look

> Study 3 of [research 19](../19-graphics.md), written on 6 October 2026 and kept as written, its quotes checked ([the check](quote-check.md)).
> `R` was the research session's working folder and is not kept, except your pictures and answers, now in `art/targets/` and `art/reviews/2026-10-06-graphics/`.

*M2 research, round 2, study 3 of seven, 6 October 2026, rewritten after the owner's 32 answers for direction B. Research only: nothing was built or changed. Godot facts were checked in Godot's own source at the tag `4.7.2-stable` (our version; study 4's checkout, commit `ed1daf0`) or in the 4.7 documentation. Quotes are in section 11; each was checked against its saved page or file (`R/work2/3/q*.tsv`, `checkq.py`). Numbers marked "estimate" are mine and need the phone. Scripts and crops are in `R/work2/3/`.*

**Words used here.**
- A *copy* is one placed instance of a shape. A *MultiMesh* is Godot's way to draw many copies of one shape at once; each request to the graphics chip is a *draw*. A *bucket* is one MultiMesh holding the copies of one shape in one place.
- A *pass* is one round of drawing into one picture (the screen, the sun's shadow map, the water's mirror).
- A *texel* is one pixel of a texture. A *texture array* is a stack of same-sized textures that one shader picks from by layer number. *Mip levels* are a texture's smaller versions; here each is *designed* for its *zoom band*, the range of zoom where it shows.
- *MSAA* (multisample antialiasing) smooths the edges of shapes by testing each pixel at 2 or 4 points. *VRS* (variable-rate shading) lets the chip work out a colour once for a block of 2 × 2 pixels instead of for each pixel.
- *HSR* (hidden surface removal) is this chip's way of skipping anything hidden behind solid shapes. A *cut-out* is a shape whose texture says where it is see-through, as for a leaf card.
- *Skinning* bends a mesh with a skeleton. A *bone palette* is one figure's bone positions for one moment, stored where the chip can read them.
- The *light grid* is a small picture over the ground listing which fires reach each square. A *patch* is the few metres of ground cover the world keeps (`WLD-31`). The *view's maps* are small pictures seen from above, such as the light grid, that every family reads.

## 1. The question, and the answer in brief

**The question:** what does the rich look change in the engine's layout, and which parts of round 1's design still hold: families in C++, the change feed, one tree of ground, one camera rig?

**The answer in brief:**
- **Round 1's spine holds:** C++ families drawing through Godot's RenderingServer, changes fed into a copy the view keeps, one tree of ground, one camera rig. The owner's look multiplies copies and work per pixel, so uploading once and then only changes matters more.
- **Direction B removes** the small picture enlarged, the pixel lock and the outline pass. The world is drawn at the full 1080 × 2404 with 2× MSAA. Texture pixels stay steady through their filter (study 2) and a texture level designed for each zoom band. The camera is one perspective rig at every stop, narrow up close, as A8.4's last line already says.
- **Five changes inside the families:** copies only from about 12 screen pixels, with ground cover set out by the chip from patch data; airy plants cut close to their leaves, solid in the middle and drawn after solid things; texture arrays with designed levels; detailed figures skinned on the chip from bone palettes, small ones drawn to read; every fire through the light grid, and our own darkening from baked values and a map seen from above, shared by every family.
- **Honestly:** from study 4's constants per screen pixel, the liked camp costs about 15–56 ms of graphics-chip time a frame built naively, and about 8–22 ms with every saving that does not show, against study 4's 8 ms line (estimates). Full size fits only if the phone lands at the optimistic end.
- **The engine does the rest by letting the phone decide, scene by scene.** Full size is the default. The levers, in the order they show least: effects in smaller maps; shading once per 2 × 2 pixels in smooth or dark areas, if the chip can; the whole picture at 0.75 or 0.5 of the screen's width and height only where a scene needs it (about 4–10 ms at half). The first alpha measures each, and the owner judges what shows.
- **Documents:** proposals for A4.1, A4.2, A5.3, A6, A8.3 and A8.4, and wording for `PRE-22` and `PRE-27`, for the owner's OK (`PRC-07`; section 6.15).

## 2. What `PROJECT.md` asks

- **The zoom** (`PRE-03`): the person stop is "about 8 m across, a person about 58 art pixels tall". The owner's answer 5 now makes that about 200 screen pixels; study 1 proposes the wording.
- **Stable pixels** (`PRE-22`, its fixed sizes lifted): "Pixels never crawl or shimmer while the camera is still or panning: it snaps to whole art pixels and turns ease to rest". Steadiness stays a quality goal. `PLT-02`: turning the phone "keeps the world, the camera and the art pixel's size".
- **Figures** (`PRE-27`, its "tiny blocks" lifted): "Small 3D figures of tiny blocks, with a separate head, torso, arms and legs, posed about 10 times a second". `PRE-44`: "Each figure's timing is offset by its seed, so a crowd never moves in step". `PRE-28`: far off, "people and animals become tiny outlined figures in their strongest colours", "with no jump as its forms change", and "a camp of 30 people stays readable at every zoom stop".
- **Light** (`PRE-30`): shadows "sharp near what casts them and softer as they lengthen"; "A fire is a warm, flickering light as bright as its heat". `PRE-24`: caves and huts are "lit inside only by openings and fires".
- **The kit** (`PRE-46`): "ground cover is drawn by the patch". `WLD-31`: cover is "kept per patch of a few metres: which kinds, how dense, their season state and ripe yield".
- **Rules that stay:** `WLD-13`, "Where you look, and how fast time runs, never change what happens"; `PRN-10`, "Every picture, sound and word shows what is really there and what really happened in the world", and "Nothing is added for show"; `PRN-14`, "Every system grows by adding self-contained pieces"; `CLAUDE.md` rule 4, everything written once.
- **Limits** (`PLT-04`): "at least 97% of frames on time while zooming, panning and turning, at every zoom, and none more than 50 ms late"; "an hour's play uses about 25–30% of the battery, and the phone never gets uncomfortably hot"; about 8 GiB of memory.

## 3. Where we stand

### 3.1 Round 1's answer, in short
C++ families (ground, cover, plants, things, figures, markers, water, sky, effects) draw through the RenderingServer. A change feed fills a copy the view keeps (the "stage"). Walkers move along their legs on the chip. Detail was chosen by metres per art pixel. One rig was orthographic and pixel-locked up close, perspective from the valley out. Render targets came one set per whole art-pixel size, switched, never resized.

### 3.2 This study's first round-2 draft
It was written for "the pixel is the rendering". Its density rule, texture arrays, palette skinning, light grid, forms by size and particles as copies carry over, adjusted below. Its low-resolution targets, pixel lock, orthographic close camera and outline data are dropped, because the owner chose direction B (answer 1) at 2 × 2 (answer 2).

### 3.3 What the other studies found
- **Study 4, round 1's addendum** (estimates): drawing small at 2 × 2 cost 4.8–12.7 ms with moderate plants and 8.7–32 ms with dense ones, against an 8 ms line. Density, not drawing small, is what costs.
- **Study 4, round 2 so far** (its handover): full resolution is 4 times the 2 × 2 picture's pixels, so every per-pixel cost is 4 times dearer. See-through plants are the main cost. Its first figures, worked by hand, are about 9–25 ms naive and 4.5–12 ms with savings that do not show (unverified: that handover has since been rewritten, and only study 1's note repeats the 4.5–12 ms). Its full-resolution model is still being built and will be the reference.
- **Study 2, round 2 so far** (its handover): draw at full size and keep texture pixels steady with a "smooth pixel" filter, 2× MSAA and texture levels chosen by zoom. Build the darkening and the shadows that soften with distance ourselves, since Mobile has no screen-space darkening and no sun shadows of that kind. Nights need a fine dither or 16-bit colour against banding.
- **Study 1:** the liked pictures' figures are about 180 picture pixels tall; strong colour is kept to small accents; a quarter of each picture is dark.
- **Study 5:** Godot re-skins a figure only when its skeleton changes; faces read with the head about 16 pixels tall; walking cycles must come from code.
- **Study 7:** Godot's own mip levels average the stored bytes and darken fine detail, so we make our own; Godot's game build loads ready-made ASTC textures (KTX files) but cannot compress them itself.
- **Study 6:** stability checks move from "whole pixels" to texture shimmer in smooth motion.

### 3.4 The owner's answers, read for the engine

| Answer | What the engine must do |
|---|---|
| 1, 2: direction B at 2 × 2 | full-resolution picture; texels about 2 × 2 at the closest zoom; no pixel grid |
| 3, 14, 19, 20: detailed people and animals | skinned figures that bend at the joints; variety by age, build and clothes; walking by code |
| 4: small figures drawn to read | designed small forms: outline behind, light face and tunic, enlarged tools |
| 5: closest zoom as liked | about 8 m across, an adult about 170–200 screen pixels tall |
| 6: plants as liked | the liked density and airy, see-through plants; savings must not show |
| 7: edges and light as liked | no outlines up close; smooth light; fine grain; our own corner and contact darkening |
| 9: true midday | short noon shadows, so contact darkening carries the form |
| 10: nights A, B, C, not D | darker nights with few warm pools; every fire still lit truthfully |
| 12: far views A and C | rich far views: dense crowns, herds, camps as glowing points |
| 13: several shelter types | more layouts of shared parts; no new engine path |
| 17: rain's four layers | streaks, splashes and wet shine, ripples, drifting mist |
| 21, 31: small plants and ground lose charm when shrunk | a designed version of each texture for each zoom band |
| 25: half resolution only if needed | full resolution by default; half resolution a lever |
| 26: a camp of thirty at ten tasks is alive | 30 detailed figures with tools at the close camp |
| 27: people found only with effort | readability help in busy scenes, without faking the world |

### 3.5 The current design under direction B
- **A4.1:** "The 3D world renders into a SubViewport at a quarter of the screen's width and height", with "A camera locked to the pixel grid" and an outline pass. Direction B replaces all three.
- **A4.1.5, fire:** "Fires reach figures and tents through a firelight term in our shaders, summing up to four fires (P2)"; "Two maps of the heights round the fires, 48 m across at 512 pixels, are drawn every frame as people move". That fits one camp, not a village.
- **A4.2 and A5.3:** "Every texture is drawn at 16 texture pixels a metre". The closest zoom now shows about 67.
- **A6:** "people from one block figure"; "our C++ poses the rigid parts, with no skeletons". Detailed figures need joints that bend.
- **A6.2:** copies "drawn as MultiMesh copies grouped by area, since a MultiMesh is culled as one". A whole area drawn because one corner shows wastes work up close.
- **A8.3:** "Grass and small stones only near." No rule says how small a copy may get.
- **A8.4** contradicts itself: "Orthographic and pitched for the close stops", and later "Perspective at every stop, each chunk set on the sphere by its vertex shader, exact near the focus (P8's second round)."
- **Missing:** the full-resolution picture and its levers; designed texture levels by zoom; a density rule; how airy plants are drawn on this chip; our own darkening; readability in busy scenes.

### 3.6 The liked density at the new zoom
In crops of the liked camp I counted about 2 to 4 separate things a square metre (stones, clumps, reeds, flower clusters) and about 5 flower heads. Between them lies a carpet of grass blades too fine to count, which is texture. At the owner's zooms (estimates, `zoomsB.py`; portrait, 40° pitch):

| Metres across | Screen px a metre | Texels a metre shown | Ground in view | Smallest copy (12 px) | An adult | Copies at the liked density |
|---|---|---|---|---|---|---|
| 8 (closest) | 135 | 67.5 | ~220 m² | ~9 cm | ~170 px | ~450–900 |
| 16 | 68 | 34 | ~890 m² | ~18 cm | ~85 px | ~1,300–2,500 |
| 20 (close camp) | 54 | 34 | ~1,400 m² | ~22 cm | ~68 px | ~1,600–3,200 |
| 35 | 31 | 17 | ~4,200 m² | ~39 cm | ~39 px | ~2,400–4,800 |
| 50 | 22 | 8 | ~8,700 m² | ~56 cm | ~27 px | bushes, reeds, big stones only |
| 300 (camp) | 3.6 | 2 | ~31 ha | ~3.3 m | ~5 px (drawn ~15–20) | trees, shelters, big rocks |

Bushes, reeds and trees come on top of these counts. Flower heads of about 3 cm stay in the texture at every zoom.

## 4. What others do

### 4.1 Drawing at full resolution on this chip
- **MSAA:** Imagination, the chip's maker: "2x MSAA is virtually free on most PowerVR graphics cores (Rogue and Volcanic onwards), while 4x MSAA+ will noticeably impact performance". Done well, "the application should use a lazily-allocated MSAA frame buffer attachment", so the extra samples never leave the chip.
- **Godot 4.7.2's Mobile renderer** resolves MSAA inside the drawing pass (`pass.resolve_attachments.push_back(color_buffer_id);`). It does not yet make the multisampled buffers lazily allocated: "// TODO: Detect when it is safe to use RD::TEXTURE_USAGE_TRANSIENT_BIT for RB_TEX_DEPTH, RB_TEX_COLOR_MSAA and/or RB_TEX_DEPTH_MSAA." Its documentation: "A value of [constant Viewport.MSAA_2X] or [constant Viewport.MSAA_4X] is best unless targeting very high-end systems", and MSAA "has no effect on shader-induced aliasing or texture aliasing", which is why texture steadiness belongs to the filter.
- **VRS:** the Mobile renderer takes a VRS picture (`textures.push_back(vrs_texture); // 2 - vrs texture.`), but "if hardware does not support VRS this property is ignored". Whether this chip and driver support it is unknown (section 7).
- **Drawing smaller and enlarging:** Godot can draw the 3D world at a lower scale and enlarge it, by bilinear filtering or by FSR 1, of which it says "FSR is slightly more expensive than bilinear, but it produces significantly higher image quality". Changing the scale rebuilds the buffers (`_configure_3d_render_buffers(viewport);`).
- **Hidden surfaces:** this chip removes hidden solid surfaces itself, so an application "should not perform a depth pre-pass as there is no performance benefit". Cut-outs break that: "alpha-tested primitives cannot write data to the depth buffer until the fragment shader has executed and fragment visibility is known". So Imagination advises that "the application renders the opaque objects first, followed by the alpha-tested objects, and finally the transparent geometry".
- **Cut-outs cut close:** "PowerVR hardware has excellent vertex processing capabilities and is designed to handle large amounts of geometry data". Fitting a twelve-sided shape around a round sprite instead of a square, "the amount of wasted fragment processing can be reduced to just 3%" (from 22%). Imagination also says "The discard keyword should be avoided in favour of alpha blending, which is much faster", but blended leaves need sorting and write no depth, which dense layered plants cannot afford.
- **Small triangles:** avoid "especially dipping below 32 pixels per primitive".
- **Compressed textures:** "Reduce the memory footprint and bandwidth cost of the texture assets." At full resolution every texture read happens 4 times as often as in the 2 × 2 picture.

### 4.2 Density
- **Ghost of Tsushima** (GDC 2021) fills fields "by generating individual blades of grass on the GPU". **Horizon Zero Dawn** (GDC 2017, old) places "100.000+ objects in scene" on the chip for "~250µs avg busy load on GPU", "Deterministic". Both set out dressing on the chip from rules, not from stored copies.
- **David Holland** (Godot 4.3, 2024), the closest Godot example of the liked style: "The grass is a number of billboard quads with a grass texture". Those are cut-out cards, the costly kind on this chip.

### 4.3 Materials and per-copy variety
- **Space Marine 2** (Saber, REAC 2025): "Static scene ~5MB, 300K instances", each copy with a "0 to 127 index in a customization palette, used to adjust shading of the instance without de-instancing". Variety is a small index into a table, not a new draw.
- **Terrain3D** (a Godot terrain plugin): "All albedo textures must be the same size, and all normal textures must be the same size". And: "Exported games may not even work since Godot’s image compression libraries only exist in the editor."

### 4.4 Figures and crowds
- **Zugalu's Thrive** (Unity case study, 2025): with vertex baking, "The memory cost is a factor of the number of vertices multiplied by the number of baked frames"; "With bone baking, different characters with the same bone structure can share the baked animation data"; draw calls fell "from 1,000 to 20 instanced draw calls".
- **Assassin's Creed Unity** (GDC 2015, old): "With the limit of 40 real AIs and 120 high resolution models, we could successfully create a scene where 10,000 crowd NPCs are on screen at the same time." Few full figures; the rest cheaper forms.
- **Dead Cells** (2018, old) drew its 3D figures small as pixel art for heroes about 50 pixels tall, and its artist admits "The disappointing level of details is and always will be an issue". Shrinking alone loses detail; small forms need designing, as the owner's answer 4 says.
- **Godot's own guide:** "bones are animated on the CPU and so you end having to calculate thousands of operations every frame and it becomes impossible to have thousands of objects."

### 4.5 Many lights
- **Google's Filament** (Android's own renderer) gives each small block of the view its list of lights: "each light in the scene is assigned to any froxel it intersects with"; "On non-OpenGL ES 3.1 devices, lights assignment can be performed efficiently on the CPU". That is our light grid, flat on the ground.
- **Godot's Mobile renderer:** "only 8 omni lights can be displayed on each mesh resource", and more "will result in omni lights flickering in and out as the camera moves". A MultiMesh is one mesh to Godot, so all its copies share those 8.

### 4.6 Uploading only changes
- HypeHype (mobile, REAC 2023): "~90% of the data is unchanged from the previous frame"; "Persistent data: Upload once at startup. Delta update when data changes." Unity's BatchRendererGroup sample: "upload 8 bytes per item instead of 112", "a 14x speed-up". Cities: Skylines II: "4828 out of 6705 draw calls were for shadow mapping".

### 4.7 Godot 4.7.2 facts used below (all checked in its source)
- **No automatic batching on Mobile:** automatic instancing "is only implemented in the Forward+ renderer, not Mobile or Compatibility". Our buckets are the batching.
- **MultiMesh:** "every single instance will always render" once its bucket is in view; the number drawn can change each frame ("Changing this does not change the sizes of the buffers").
- **Draw order:** the Mobile renderer sorts its solid list by a key whose top 8 bits are the material's render priority (`uint64_t priority : 8;`, set as `priority = p_priority - RSE::MATERIAL_RENDER_PRIORITY_MIN; //8 bits`). So a higher priority draws later: "all objects with [member render_priority] [code]1[/code] will render on top of all objects with [member render_priority] [code]0[/code]". Cut-outs can be drawn after every solid thing.
- **Cut-outs stay in the solid pass** unless their shader uses the alpha antialiasing edge (`bool has_base_alpha = (uses_alpha && (!uses_alpha_clip || uses_alpha_antialiasing));`), which would move them to the blended pass. The `alpha_to_coverage` render mode alone keeps them solid but smooths nothing: without the edge, Godot sets a cut-out's alpha to 1 ("// If we are not edge antialiasing, we need to remove the output alpha channel from scissor and hash"), and Godot's standard material writes the edge whenever its alpha antialiasing is on. So a cut-out smoothed by alpha to coverage is drawn in the blended pass.
- **Our own mip levels:** "If [param use_mipmaps] is [code]true[/code], loads the mipmaps for this image from [param data]." Texture arrays need one size, format and mip setting for all layers.
- **Skinning:** Godot's skinning is a compute program per mesh (`skeleton.glsl`), reading buffers only (`layout(set = 2, binding = 0, std430) buffer restrict readonly SkeletonData {`), so it avoids the sampled-image compute that A4.3 rules out on this driver. It re-skins only when the skeleton changes (`if (sk && sk->version != mi->skeleton_version) {`). MultiMesh copies get no skeleton, but a MultiMesh's own shader can read the mesh's bone indices and weights (`buffer = s->skin_buffer;`), so we can skin copies ourselves (read in the source; untested).
- **Seeing through:** spatial shaders may invert the depth test (`actions.render_mode_values["depth_test_inverted"]`) and use the stencil; a material preset "shows a silhouette of the object behind walls", marked "May be affected by future rendering pipeline changes."
- **No texture streaming**, and texture compression only in editor builds (`#ifdef TOOLS_ENABLED` around the ASTC compressor).
- **GPU particles** are a compute program that samples textures (`particles.glsl`), the kind that stopped Bevy before its first frame on this phone and driver, in the chip maker's shader compiler ("Unhandled sampler flag combo", Bevy issue 25788; read through a summary only).
- **Rotation:** Godot's Android game declares `android:configChanges="layoutDirection|locale|orientation|keyboardHidden|screenSize|smallestScreenSize|density|keyboard|navigation|screenLayout|uiMode"`, so turning the phone resizes the window without restarting; a new size rebuilds the 3D buffers once.

## 5. The options

Five ways to lay out the engine for the chosen look:

| Option | Closeness to the feeling | Cost on the phone (estimates) | Effort for an AI builder | Risk | Fit with the rules that stay |
|---|---|---|---|---|---|
| **A. Round 1 unchanged:** small pictures enlarged; every small thing a copy with data; rigid blocks; up to four fires an object | far: the owner chose B, detailed figures, no outlines | not costed at full resolution; per-copy costs multiply | medium | high: rework | good |
| **B. Round 1's spine adapted to direction B** (recommended) | closest: sharp full-resolution world, the liked density, detailed figures, every fire lit, our own darkening | liked camp ~8–22 ms at full size, ~4–10 at half size | medium to high: fitted plants, palettes, maps, levers | medium: fitted plants, the bones path and VRS untested on the phone | good: one rule per thing (rule 4); the world drawn as it is (`PRN-10`) |
| **C. Godot's own features:** a material per material, cut-out cards, a skeleton per figure, a light per fire, GPU particles, visibility ranges | close in a small scene; lights flicker past 8 a mesh; Mobile draws no corner darkening | high: a draw per figure surface and pass; ~12–50 ms at the liked density, with no darkening at all | low at first, high later | high: GPU particles may not start on this driver | poor: second paths and rework (`PRN-14`) |
| **D. GPU-driven:** compute programs cull and place, filling MultiMeshes on the chip | as B | least processor time | high | high: compute on this driver unproved; harder to test | fair |
| **E. B at half size always:** the 3D drawn at 540 × 1202 and enlarged smoothly | softer, as the owner saw in question 25 | ~4–10 ms | as B, less tuning | low | fits, but against answer 25 ("only if needed") |

- **A** is what round 1 would build if nothing had changed. Its targets and pixel lock serve a look the owner did not pick.
- **B** is recommended. It keeps every proven part and changes how the families fill their buckets, what they share, and how the picture is drawn.
- **C** gives the quickest first picture and is the worst at scale. Godot's 8-light cap alone rules it out for a lit village.
- **D** may come later for one family, after the phone proves compute; it is not the base.
- **E** is B with one lever always on. The owner saw that it is softer, so it stays a lever.

**Smaller choices inside B:**

| Choice | Ways | Recommended |
|---|---|---|
| The picture | full size; half size enlarged; full size with VRS | full size with 2× MSAA; the others as levers |
| Steady texture pixels | camera snapped to texels; nearest sampling; a smooth-pixel filter | the filter (study 2), with designed levels per zoom band |
| Ground cover | copies with data per cell; set out by the chip from patch data; cut-out cards everywhere | set out by the chip; data per cell as the fallback |
| Airy plants | plain cards; cards cut to the outline; solid middles with cut-out fringes; blended leaves | cut close, solid middles, fringes only at the fine tips |
| Draw order | Godot's default; solid things first by render priority | solid things first, then cut-outs, then blended |
| Materials | one shader material each; atlases; texture arrays | texture arrays (atlases bleed between tiles in small levels) |
| Corner darkening | screen-space pass; baked per vertex; map from above; none | baked per vertex plus a map from above |
| Figures | Godot skeletons; rigid parts; palette-skinned copies; vertex-animation textures | palette-skinned copies; Godot skeletons as the fallback |
| Tiny figures | separate pictures; the same figure with baked poses, enlarged | the same figure, baked poses |
| Fire light | Godot lights; a short list per object; the light grid | the light grid |
| Fire shadows | walk per pixel; a small shadow map per near fire; none | walk within reach; a shadow map for a fire filling the screen if the phone says so |
| Particles | GPU particles; CPU particles; copies moved in the vertex shader | copies |
| Camera up close | orthographic; perspective with a narrow view | one perspective rig at every stop |
| Fitting the phone | half size everywhere; levers only where needed | levers where needed, chosen by measurement |

## 6. What we'd recommend

### 6.1 What stays from round 1, and what changes (question 8)

| Round 1 or the first draft | Now | Why |
|---|---|---|
| C++ families on the RenderingServer | stays | Mobile batches nothing itself; dense, varied copies need buckets we control |
| The change feed and the view's kept copy | stays, more needed | static copies upload once; looks change only at events |
| In-betweens on the chip | stays | nothing per copy per frame on the processor |
| One tree of ground | stays, and carries patch data and the view's maps | one rule for ground texture, cover, darkening and far colour |
| One camera rig | stays: perspective at every stop, narrow up close, no pixel lock | direction B needs no pixel grid; one projection means no hand-over to jump |
| Detail by metres per art pixel | by screen pixels: copies from about 12 px, figures by height on screen | the chip's 32-pixel rule at full resolution |
| Targets per art-pixel size | gone: the full-size window with 2× MSAA; half size as a lever | answers 1, 2 and 25 |
| The outline data pass | gone; outlines only behind small figures, as geometry | answers 4 and 7 |
| Materials as ramp rows | texture-array layers with designed levels, plus per-copy look data | answers 15, 21 and 31 |
| C++ poses rigid parts | C++ poses skeletons; the chip skins copies; blocks become rigid weights | answer 3 |
| Up to four fires; height maps 48 m round three fires | light grid; height maps over the fires in view | villages; cost at full size |
| Corner darkening, never built | baked per vertex, plus a map from above | answer 7; Mobile has no screen-space version |
| Every shape in every pass | each form declares its passes; small cover in the main pass only | cost |
| Particles not settled | copies moved in the vertex shader | compute risk on this driver |

### 6.2 The parts
Round 1's parts stay: feed, stage, families, kit, space, rig, pick, passes and telemetry. Four shared parts grow, each written once:
- **Materials:** texture arrays by size class with designed levels, the material table, the per-copy look layout, and one texture-sampling include.
- **Poser:** skeletons by body pattern, the C++ poser at 10 Hz, the baked pose library and the palette texture.
- **Lights:** the fire list, the light grid, the fire height maps, the sun and moon, and cloud shadows from the weather.
- **Maps:** the small pictures seen from above, built from the stage and read by every family: patch cover, the light grid, openness for darkening, a push map of where people and animals stand, and the weather's wetness and snow.

The families are ground, cover, plants, things, figures, creatures, markers, water, sky and effects. Each declares its forms and, per form, the passes it joins.

### 6.3 The picture (question 5)
- **One full-size picture:** 1080 × 2404 in portrait, drawn by the Mobile renderer straight into Godot's window with 2× MSAA. The interface draws over it in the same window. There are no low-resolution pictures, no pixel lock and no outline pass.
- **Beside it, maps at their own sizes:** the sun's shadow map (sized so a shadow texel is about 2 screen pixels), the water's mirror at half size, the fire height maps, and the view's maps. Things that are smooth anyway are worked out in these maps, never per screen pixel. This is the main way the engine fits full size: per-pixel searches cost 4 times more than at 2 × 2, while a map costs the same whatever the screen.
- **Memory:** Godot keeps the multisampled buffers in memory: about 42 MB at 2× MSAA plus 21 MB for the resolved picture and depth (estimate). Whether it also writes them out each frame is for the phone (section 7).
- **The levers, in the order they show least:**
  1. **Effects in maps** (above): always on.
  2. **Shading once per 2 × 2 pixels (VRS)** where the picture is smooth or dark: water away from the shore line, mist, smoke, deep night shade. Only if the chip supports it. I simulated it on the owner's own full-size picture from question 25 (`R/work2/3/vrs-sim-closeup.png`, made by code): it does not blur like half size, it turns areas blocky, close to look A. Real VRS keeps the edges of shapes sharp, but it would also make cut-out leaf edges blocky. So it belongs only where nothing fine shows.
  3. **The whole 3D picture at 0.75 or 0.5 scale**, enlarged with FSR 1 or bilinear, only in the scenes, zooms or heat states that need it. A change rebuilds Godot's buffers, so it is switched only when the fingers lift. If that rebuild shows as a stutter, two prepared pictures are kept and switched instead (about 15 MB more at half size, estimate).
- **Turning the phone** (`PLT-02`): Godot resizes its window without restarting, rebuilding the 3D buffers once. The rig keeps focus, turn and metres per screen pixel, so a texture pixel keeps its size. The longest frame at rotation is measured (section 7).

### 6.4 Density and airy plants (question 1)
- **The rule:** a form is drawn as copies only while each copy covers about 12 screen pixels (6 texels) or more, a tuning value. Below that, the ground's texture carries it. The texture always shows the cover's small marks, so copies add standing relief, never new content. As a form shrinks past the rule, its copies thin out by hash over the same marks: no fade, no jump. Section 3.6 gives the counts: about 450–900 copies at the closest zoom, at most about 5,000 small copies at any zoom (estimate).
- **Who places what** (`WLD-13`, `PRN-10`): the simulation's area maker places what rules read: single trees, bushes, stones and plants people gather. Those are copies with data, uploaded once per area and changed at events. Grass tufts, flower clumps, pebbles, fallen leaves and seedlings are set out by the chip from each patch's kinds, density, season state and ripe yield, and a hash of the place. Rules read the patch, never a tuft, so every tuft is the patch's own cover, the same on every visit.
- **How ground cover is drawn:** each area brings a small patch picture (4 m patches: 64 × 64 texels, about 32 KB, estimate). The ground shader and the cover shader both read it, so the ground's colour and its tufts always agree. One MultiMesh per cover form spans the view. Each frame the processor writes a small table of the visible cells; the shader places each copy from its number, sways it in the wind and drops empty slots. That is about 10 draws with no per-copy uploads. If the chip's vertex stage proves slow, the fallback is copies with data per cell, built on worker threads: same rule, more uploads.
- **Airy plants, kept as liked** (answer 6), in ways that do not show:
  1. **Cut close:** each plant's shape follows its leaves' outline, made by code from the texture's see-through mask, so the chip wastes little on empty corners (Imagination: from 22% to 3% for a round sprite).
  2. **Solid middles:** blades and leaves wide enough on screen are solid geometry; only the finest tips are cut-outs.
  3. **Solid things first:** cut-out materials get a higher render priority than every solid material, so Godot draws them last in its solid pass and the chip skips their hidden parts behind solid things. Their edges could be smoothed by alpha to coverage under MSAA, but in Godot's source that moves them to the blended pass (section 4.7).
  4. **No shadow pass for small plants:** they are lit once per plant at their foot and darkened by the openness map. Only big shapes cast into the sun's map.
  5. **No depth pre-pass:** the chip gains nothing from one.

  What these save is an estimate until the first stress scene: in my rough check (section 6.12), the plants' hidden layers fall from about 7–37 ms to about 2–9 ms.
- **Things and figures:** buckets sized per form: about 16–32 m for full forms near the camera, a whole area for far forms. Static and moving copies stay apart. Uploads go by changed byte ranges, within a per-frame budget (estimate: under 256 KB a frame; an area's 2 MB of static copies over about eight frames).
- **Variety** (`PRE-43`): each copy's 20 floats hold its place (12), then layer, tint row, wear and seed (4), then season, growth stage, style pattern and a spare (4). If the number of draws proves the bottleneck, Saber's way is the fallback: copy the visible copies into one buffer per form each frame.

### 6.5 Materials (question 2)
- **Texture arrays by size class:** for example ground covers at 1024², made things at 256², small things at 64², figures' garments and faces at their own sizes. A copy names its layer; Godot requires one size, format and mip setting per array.
- **Density:** about 64–68 texels a metre at the closest zoom (exactly 67.5 for a texel of 2 × 2 screen pixels at 8 m across; study 7's 64 is a handier power of two), then half as many in each next band: about 34, 17 and 8 (section 3.6). The number is one tuning value; study 7 settles it with studies 1 and 2.
- **A designed level for each zoom band** (answers 21 and 31): every level is drawn as pixel art for the size it shows at, not shrunk by Godot. Godot accepts our own levels. The rig publishes the zoom band, and the one texture-sampling include picks the level from it, so every surface changes design at the same zoom. How two levels hand over while the zoom moves is study 2's.
- **Materials are data:** about a dozen family shaders, each compiled for its passes, about 40–60 pipelines (estimate). A new material is a new layer and a table row, never a shader (`PRN-14`). Wear, wetness, snow, soot and burn are per-copy or per-map values over the layer.
- **Memory** (estimates): about 100 MB as ASTC (compressed, 1 byte a texel) or about 400 MB uncompressed, for one map per material. That fits in 8 GiB, so everything stays loaded; Godot has no streaming anyway. At full size, compression also cuts texture traffic about 4 times. Godot's game build cannot compress, so textures are compressed at build time (study 7's KTX2 route), or kept uncompressed if the phone shows that it costs nothing visible (section 7).

### 6.6 Figures (question 3)
- **One path:** a skinned mesh per body pattern: people, and the six animal patterns. Garments, hair and beads are separate skinned meshes on the same skeleton. Held tools and carried things are ordinary copies that name their figure and bone, so the chip moves them with the hand. Rigid weights would give blocks, so the old look remains a special case of the same data.
- **Posing stays C++ at 10 Hz** (`PRE-44`, A6.3): key poses, the bending rules (stoop, limp, slump), seed offsets, and walking by code (answer 20). Each figure in full writes its bone palette (about 24 bones × three half-float texels) into a data texture at its own pose step. The chip skins every vertex from the palette named in the copy's data. Whether figures show each pose for a tenth of a second or glide between the last two is a setting on the chip; it is a look question (section 10).
- **Tiny figures and crowds** read a baked library: every movement's poses at 10 a second, made once at load by the same poser. That is about 900 poses, 0.5 MB for people and about 3 MB for six animal patterns (estimate). A copy holds only its movement, start time and seed, like Zugalu's shared bone baking.
- **Forms by height on screen** (tuning values; each switch where both forms give about the same pixels, so nothing pops):

| Form | Height on screen | What it is |
|---|---|---|
| full | ~100 px and up (closest zooms) | ~1,500 triangles; textures ~100 texels tall; faces with expressions as layers |
| simple | ~40–100 px | ~500 triangles; the textures' next designed level |
| small, drawn to read (answer 4) | ~15–40 px | ~150 triangles; the tool and head a little larger by bone scale; a designed level with lighter face and tunic; an outline drawn behind as a slightly larger shell, growing from nothing at ~50 px to 1–2 px, so no outline shows up close (answer 7) |
| tiny | below ~15 px | enlarged up to ~4×, baked poses, strongest colours (`PRE-28`) |
| marker | groups and herds close together | one mark (`PRE-28`) |

- **Costs** (estimates, `figuresB.py`; two passes, no outline pass):

| Case | Triangles a pass | Palette reads a frame | Posing | Uploads |
|---|---|---|---|---|
| closest zoom, 6 people in full | ~10,000 | ~0.4 M | ~0.1 ms a second | ~35 KB/s |
| close camp, camp of thirty | ~15,000 | ~0.3 M | ~0.4 ms a second | ~170 KB/s |
| close camp, village (100 seen) | ~30,000 | ~0.6 M | ~1.4 ms a second | ~0.6 MB/s |
| camp zoom, 2,000 tiny figures | ~80,000 | ~0.9 M | none (baked) | none |

- **The fallback** is Godot's own skeletons for figures near the camera. Godot re-skins only when a skeleton changes, so poses held for a tenth of a second are cheap. But each figure costs a draw per surface per pass: about 180–240 draws for thirty people in two passes (estimate), and one skinning job each. It suits a few dozen figures, never crowds. Vertex-animation textures would take about 18 MB for one 1,200-vertex mesh across 900 poses, and could not bend by rules.
- **Imagination's warning applies:** "Vertex shader texture lookups always count as dependent texture reads". So palettes are half floats, and far forms use one bone a vertex.

### 6.7 Light and darkening (question 4)
- **Smooth light** (answer 7) from one shared light function, written once: sun or moon, sky, haze, firelight and cloud shadows. No steps.
- **Fire light:** the stage keeps every fire (place, heat, flicker seed). A light grid of about 4 m squares over the view lists up to four fires per square; it is rebuilt when a fire or the view changes. It is a 64 × 64 texture plus a fire table, a few kilobytes, read by every family, cover included. The owner's nights (answer 10: A, B and C, not D) are darker with few warm pools; the grid lights whatever fires the world really has, so the look comes from the light's values, not from leaving fires out.
- **Fire shadows:** one pair of height maps (tops and undersides, as P3 built them) covers the fires in view. Each lit pixel walks toward the fires whose reach it is in. Fires in huts and tents light their doorways, because the walls stand in the height maps (`PRE-24`). At full size the walk is the costliest light term: one fire whose light fills 80% of the screen costs about 2.4–4.8 ms (estimate, from study 4's constants). For that case, a small shadow map drawn from the fire would cost about the same whatever the screen size. Study 2 chooses the technique and study 4 the numbers; the engine keeps the fire list, the height maps and a pass slot either way. From the camp zoom out, fire shadows are off and pools of light remain.
- **Sun and moon:** one directional light, its shadow map sized to the texel. Only big shapes cast into it. True midday (answer 9) makes shadows short, so the darkening below carries the form at noon. Soft shadows that widen with distance (`PRE-30`) are ours to build (study 2). Cloud shadows come from the weather's cloud texture, with no pass.
- **Darkening in corners and under things** (answer 7), with no screen-space pass, which would cost about 2.4–5.8 ms at full size and split Godot's single drawing pass (estimate):
  1. **Baked per vertex:** when the kit builds a shape, its own folds, insides and crevices get darker values (a tent's inside, a garment's folds, a rock's cracks). Made once and cached.
  2. **A map from above:** an openness map around the focus (for example 1024², about 6 cm a texel at the closest zoom and coarser as the camera rises, so it always covers the view; 1 to 2 MB), made from the ground's heights and the tops of what stands there, the same heights the fires use. The still part is made when an area loads or changes; moving people and animals are added at their pose steps. Every family reads it at its own place, so baskets, rocks, tents and people all darken the ground at their foot, and plants low between tall things darken too.

### 6.8 Forms and hand-overs, out to M3's globe (question 6)
- **Each family lists its forms by size on screen:** full, simple, small, texture or marker. Thresholds live in tuning files, with a margin so forms do not flicker. Godot's visibility ranges are not used: they fade by transparency.
- **Hand-overs** happen where both forms give about the same few pixels. Small cover hands over to the ground texture by thinning. Each zoom band's texture level is designed for its band, so a hand-over changes design, not sharpness.
- **Far views as rich as the liked ones** (answer 12, A and C): trees at the camp zoom are low-poly crowns with painted textures (P2's 12-triangle trees), dense enough to read as forest; herds are tiny animals with baked poses, merging into markers; camps glow as points. Impostors wait for a measured need.
- **The join with M3:** the same tree of ground, the same forms rule, the same light function and the same maps. Patch pictures exist for full areas within about 300 m. Beyond, the ground shows its cell's cover as texture (canopy, grassland, rock, snow).

### 6.9 The camera (question 7)
- **One perspective rig at every stop**, as A8.4's last line already says. The orthographic close stage was there for the pixel lock, which direction B does not need, so there is no hand-over between projections to jump.
- **A narrow view up close** keeps the picture nearly isometric, like the liked ones. With a field of view of about 5–10° (a tuning value), texels stay within about 5–11% either way of 2 × 2 from the bottom of the screen to the top (estimate). The view widens as the camera rises and tilts toward straight down (`PRE-29`). The near and far planes hug the scene, so depth stays precise and the sun's shadow map is spent on what is seen. The owner judges this view against the liked pictures' flat look in the first alpha.
- **No pixel lock.** The ease to rest on 5° and 1.25× steps when the fingers lift stays as the owner chose it, now for calm framing rather than steady pixels.
- **The rig's state:** focus in whole centimetres, turn, pitch, field of view and metres per screen pixel. It publishes the zoom band, the sun, time and the moving origin to every shader.

### 6.10 Readability in busy scenes (answer 27)
The owner found the people in the busy autumn wood only with effort. The engine can offer these tools, ordered by how little they add to the world:
1. **Small forms drawn to read** (answer 4): designed levels with value contrast, an outline shell, slightly larger tools. These are data per form, decided already.
2. **Plants parting around walkers:** tall cover bends away from people and animals standing or walking in it, read from the push map. Real grass does this, so nothing is invented, and it opens a little space round each figure.
3. **People seen through leaves:** where a person stands behind a crown or a bush, a faint silhouette of the hidden part, using Godot's stencil or an inverted depth test. It shows only what is really there, but it is drawn for the player's eye, like a selection ring.
4. **Crowns thinned near the camera** when they hide people, as many top-down games do.
5. **Interface help:** the card and ring on a tap (`PRE-35`), or a hold that briefly marks every person in view.

Tools 1 and 2 fit the rules as they stand. Tools 3 to 5 draw something only for the player's eye; whether that sits with `PRN-10` ("Nothing is added for show") is the owner's to say. Studies 1 and 6 judge the look.

### 6.11 What the other answers need from the engine
- **Rain with all four layers** (answer 17): streaks are copies in a volume round the camera, moved in the vertex shader; splashes are short-lived copies on the ground at hashed places; wet shine and puddles are material values from the weather's wetness map; ripples are a water-shader term; drifting mist is a few soft layers. All follow the weather the world really has.
- **Winter B** (answer 11): snow on the ground is a ground material; snow on things is a per-map value on upward faces; clear water stays the water family's.
- **Several shelter types** (answer 13): each type is a kit layout of shared parts (poles, hide panels, bark sheets, a stone ring). Adding one is a catalogue entry, with no engine change.
- **A camp of thirty at ten tasks** (answer 26): thirty skinned figures with tools on bones, about 15,000 triangles a pass (section 6.6).

### 6.12 What fits the phone, honestly
These are my rough checks, using study 4's own constants applied per screen pixel (`full.py`; study 4 owns the milliseconds and is rebuilding its model for full size). All in graphics-chip milliseconds a frame, against study 4's 8 ms line; a frame at 60 a second lasts 16.7 ms.

| The liked camp at the closest zoom, by day | Estimate |
|---|---|
| naive: full size, plain cut-out cards, a screen-space darkening pass, every shape in every pass | ~15–56 ms |
| with the engine's savings that do not show (fitted plants, solid first, our darkening, big casters only, a small mirror) | ~8–22 ms |
| the same with the whole picture at half size | ~4–10 ms |

| Other scenes, with the savings | Full size | Half size |
|---|---|---|
| closest zoom by a fire at dawn | ~8–19 ms | ~3–8 ms |
| close camp of thirty, by day | ~7–20 ms | ~4–10 ms |
| camp at night, 20 people, 5 fires | ~8–21 ms | ~4–10 ms |
| storm with rain's four layers | ~7–17 ms | ~3–8 ms |
| camp zoom, forest and 30 tiny people | ~4–8 ms | ~2–6 ms |

So, plainly:
- **Built naively, the look does not fit** the 8 ms line, and at the high end it would miss frames too.
- **With the savings, it fits only if the phone lands at the optimistic end.** Study 4's own first figures, worked by hand, are about half of mine (4.5–12 ms); either way, 8 ms is reached only at the low end.
- **The far zooms probably fit at full size;** the closest zooms with dense plants are the hard case.
- **So the engine is built to let the phone decide, scene by scene:** full size everywhere it fits; then the levers of section 6.3 where it does not, measured, and judged by the owner. Half size is the last lever, used only where needed, as the owner asked.

### 6.13 Budgets for the closest zooms (estimates)

| What | Budget |
|---|---|
| Draws, all passes | about 300–500 a frame (to be measured) |
| Triangles | about 300,000 a pass in all; small cover 50,000–220,000 in the main pass only |
| Uploads | under about 256 KB a frame; areas spread over frames |
| Memory | textures ~100 MB as ASTC or ~400 MB uncompressed; the picture with 2× MSAA ~63 MB; sun shadow 16–64 MB; mirror and maps ~10–20 MB; copies ~20 MB; palettes and library ~4 MB |
| Main thread | study 4's line: at most about 8 ms on average |

### 6.14 What M2 builds first
The spine end to end, as round 1 proposed (feed, stage, the things family, rig, passes, overlay), with texture arrays and designed levels from the first day so there is one material path. Then three stress scenes in the first alpha, each measured on the phone and judged by the owner:
1. **The liked camp at full size:** the closest zoom and the close camp at the liked density, with switches for plain or fitted plants, draw order, MSAA off, 2× or 4×, scale 1, 0.75 or 0.5, and VRS if the chip has it. This answers what fits.
2. **Thirty at ten tasks:** thirty detailed people with tools at the close camp, then 2,000 tiny ones at the camp zoom, on the palette path and on Godot's skeletons.
3. **Night by the fire:** one big fire and four hearths, the light grid, fire shadows walked and mapped, the openness map, and the dark night's banding.

Then rotation and switching the levers (the longest frames), and rain's four layers. Only after that, the look work of studies 1, 2 and 6.

### 6.15 Proposals for the owner
All need the owner's OK (`PRC-07`). Study 1's wording governs the look; mine only has to fit it.
1. **A4.1, the picture:** the 3D world drawn at the screen's full size with 2× MSAA, the interface over it; no pixel lock and no outline pass; smooth things worked out in maps at their own sizes; three levers (VRS in smooth or dark areas, a smaller 3D picture where needed, and a prepared second picture if switching stutters), each measured, and each use judged by the owner.
2. **A4.1.5, fire:** light from a light grid of every fire; fire shadows from height maps over the fires in view, walked within each fire's reach, with a shadow map from a fire if the phone says so.
3. **A4.2:** materials as texture arrays with per-copy layers; cut-outs drawn after solid things; each form declares its passes; darkening baked per vertex and read from a map from above.
4. **A5.3** (lifted), proposed: "Textures are pixel art at about 64–68 texels a metre at the closest zoom, with a level designed for each zoom band, so a texel shows as about 2 × 2 screen pixels at every zoom; arrays of one size each; the density is one tuning value." (Studies 1, 2 and 7 may refine it.)
5. **A6.1 to A6.3:** a skeleton per body pattern; C++ poses at 10 Hz, written once; the chip skins copies from bone palettes; tiny figures read a baked library; tools and carried things ride on bones; blocks are rigid weights on the same path.
6. **A6.2:** buckets sized per form; ground cover set out by the chip from patch data.
7. **A8.3:** "Copies only while they cover about 12 screen pixels; smaller detail is the ground's texture. Rules read what the world keeps (single plants, stones, patches), never a tuft." Figures by height on screen, small ones drawn to read.
8. **A8.4:** delete "Orthographic and pitched for the close stops"; one perspective rig at every stop, narrow up close and widening as it rises.
9. **A4.3, effects:** particles are copies moved in the vertex shader; no compute program ships without a phone test.
10. **`PRE-22`** (lifted part), proposed: "Texture pixels never crawl or shimmer while the camera is still, panning or turning. A texture pixel shows as about 2 by 2 screen pixels at the closest zoom and stays near that size at every zoom, each zoom band with its own version of the textures, in portrait and landscape." `PLT-02`'s Done when would then read "the texture pixel's size" for "the art pixel's size".
11. **`PRE-27`'s first sentence** (lifted part), proposed: "3D figures as detailed as in the pictures the owner liked, with a separate head, torso, arms and legs that bend at the joints, posed about 10 times a second (`PRE-44`), so they look like crisp pixel art from any angle."
12. **`PROJECT.md`:** nothing else from this study.

## 7. Questions only the phone can answer
1. **What fits:** stress scene 1 at full size, with the savings on and off; each viewport's chip time and the main thread's time from trace sections.
2. **Plants:** plain cards, cards cut close, and solid middles with fringes, each with solid things first and without. Measure chip time at 1, 2 and 5 layers of cover.
3. **MSAA:** off, 2× and 4× at full size. Measure chip time and memory traffic, to see whether Godot writes the multisampled buffers out each frame.
4. **VRS:** does the driver offer fragment shading rate or fragment density maps? Add both extensions to M1's self-check. If yes, measure the saving on water, mist and night shade.
5. **The smaller picture:** chip time at scale 0.75 and 0.5 (bilinear and FSR 1), and the longest frame when the scale changes.
6. **Textures:** 64 and 256 layers of 1024² and 256², as ASTC and uncompressed, at full size. Measure chip time, memory and traffic.
7. **Vertex-stage reads:** 100,000 to 1 million vertices reading 1, 3 and 12 texels (patch data, rigid and blended palettes), against the same copies without reads.
8. **Draws:** main-thread milliseconds for 100, 300 and 1,000 MultiMesh draws in two and three passes.
9. **Figures:** 30, 100 and 300 skinned people in full, and 2,000 tiny ones, on the palette path and on Godot's skeletons.
10. **Fires:** 1, 5 and 15 fires; shadows walked or mapped; up to 4 per grid square. Measure chip time, then run 20 minutes for heat.
11. **Rotation:** the longest frame when the phone is turned at the closest zoom.
12. **GPU particles:** does one GPUParticles3D node start on this driver at all? One throwaway scene; a failure confirms copies.
13. **Areas arriving:** frame times when an area's 2 MB of copies arrive with per-frame budgets of 128, 256 and 512 KB.

## 8. Risks
1. **Full size does not fit the dense scenes.** Retire with stress scene 1 in the first alpha. The fallback is the levers, ending at half size where the owner judges it acceptable.
2. **Cut-out plants still dominate** after cutting close and ordering. Retire with question 2. Fallbacks: more solid middles; fewer plant layers where the owner sees no loss; VRS only if it does not show.
3. **Draws multiply** (forms × buckets × passes). Retire with question 8. Fall back to Saber's visible copies per form.
4. **Vertex-stage reads are slow on this chip.** Retire with question 7. Fallbacks: Godot's skeletons for near figures; ground cover as copies with data per cell.
5. **The bones path for MultiMesh is read in the source but untested.** One small scene in the first week, in the cloud and on the phone.
6. **Texture memory, traffic or load time grow.** Compress at build time (study 7); measure uncompressed first.
7. **Switching the levers or turning the phone stutters.** Retire with questions 5 and 11. Fallback: two prepared pictures.
8. **Thinning or level hand-overs show.** Thin by hash over matching texture marks; the owner judges in stress scene 1.
9. **Busy scenes stay hard to read.** Tools 1 and 2 of section 6.10 first; tools 3 to 5 only with the owner's word.
10. **A compute program fails on the driver.** Ship none untested; particles as copies; question 12.
11. **Light leaks through walls** if a hut's fire is not walked. Keep huts and tents in the height maps; check with a debug view of fire reach.
12. **Godot moves under us.** The version stays pinned, with regression scenes for the bones path, uploads, draw order and the levers.

## 9. Pictures from GPT
Two of my 16 are used; I asked for none since resuming. The owner's 32 answers settled what pictures could, and the rest of my questions are for the phone.
1. **`R/work2/3/gpt/figure-sizes.png`** (request `figure-sizes.txt`). *Question:* what must a figure keep at each size? *What it showed:* the same man and red stag at about 95, 70, 44, 20 and 14 art pixels tall. The small ones keep a dark outline, a light face and tunic against dark hair and legs, and an enlarged spear point. My own shrinking by code (`fig_ladder_compare.png`) loses the figure against grass at 14–20 and leaves a smudge at 6–10. The owner saw both as question 4 and chose "drawn to read" (A). *For the engine:* the small form of section 6.6.
2. **`R/work2/3/gpt/night-fires.png`** (request `night-fires.txt`). *Question:* with fifteen fires in view at the close camp, what carries the night? *What it showed:* every fire a warm pool, doorways glowing, long shadows only round the big fire; 37% of the picture dark and 7% brightly firelit. GPT drew metal-looking pots, which a Stone Age camp would not have. The owner saw it as question 10's D and did not pick it. *For the engine:* the light grid stays the mechanism, since the world decides how many fires there are, and the night's look comes from its values.

One comparison was made by code, not by GPT: `R/work2/3/vrs-sim-closeup.png`, the owner's full-size picture from question 25, at half size enlarged, and shaded once per 2 × 2 block (section 6.3). It is not for the owner: it cannot keep shape edges sharp as real VRS would.

## 10. For other studies
- **Study 1 (look):** the zoom table of section 3.6 for `PRE-03`'s new wording (an adult about 170–200 screen pixels at 8 m across, about 68 at 20 m, about 27 at 50 m). Whether figures step their poses 10 times a second or glide between them is a new look question for the owner under direction B. Readability tools 3 to 5 of section 6.10 need the owner's word.
- **Study 2 (drawing):** the engine assumes the smooth-pixel filter in one sampling include, levels chosen from the rig's zoom band, cut-outs drawn after solid things with alpha to coverage, and our darkening from per-vertex values and the map from above. At full size, fire shadows walked per pixel cost about 2.4–4.8 ms for a fire filling the screen (estimate); a fire's own shadow map may be cheaper. A zoom step of 2^⅓ (1.26×) instead of 1.25× would land every third resting zoom exactly on a band's design size; that is study 2's to weigh.
- **Study 4 (phone):** questions 1 to 13 above. My estimates in 6.12 apply study 4's round-2 constants per screen pixel, and come out at about twice its first figures worked by hand; its full-resolution model is the reference. Godot keeps its MSAA buffers in memory; whether it writes them out each frame matters at full size. VRS support needs the self-check.
- **Study 5 (content):** skeletons and weights by code (rigid for blocks, blended at joints); garments as separate skinned meshes; tools that ride on bones; the baked pose library made by the poser; per-band designed levels for figures, with the small form's lighter face and tunic; plant shapes cut to their leaves from each texture's mask; several shelter types as kit layouts.
- **Study 6 (loop):** stress scene 1 is where the owner judges whether a lever shows; texture shimmer is measured in the engine's own motion.
- **Study 7 (pipeline):** texture arrays need one size, format and level count per array; our own levels go in through `Image` data. If uncompressed textures cost visible time at full size (question 6), the KTX2 route at build time is needed early. Generate the copy-layout and skinning includes from C++. godot-cpp's profile must include the texture-array and RenderingDevice classes.

## 11. Sources

**Kindling's own:** `PROJECT.md` (`PRE-03`, `PRE-22`, `PRE-24`, `PRE-27`, `PRE-28`, `PRE-30`, `PRE-43`, `PRE-44`, `PRE-46`, `WLD-13`, `WLD-31`, `PRN-10`, `PRN-14`, `PLT-02`, `PLT-04`); `ARCHITECTURE.md` (A4.1, A4.2, A4.3, A5.3, A6, A8.3, A8.4); `LESSONS.md` (P1 to P3, P8); round 1's notes and brief; `BRIEF2.md`; `RESUME.md`; the owner's answers and pictures in `R/owner/ask/`; the other studies' handovers in `R/work2/*/PAUSED.md`. My scripts and files: `R/work2/3/full.py`, `zoomsB.py`, `figuresB.py`, `zooms.py`, `figures.py`, `vrs-sim-closeup.png`, `crop_meadow.png`, `crop_shore.png`, `fig_ladder_code.png`, `fig_ladder_compare.png`.

`PROJECT.md` quotes used: "about 8 m across, a person about 58 art pixels tall"; "Pixels never crawl or shimmer while the camera is still or panning: it snaps to whole art pixels and turns ease to rest"; "keeps the world, the camera and the art pixel's size"; "Small 3D figures of tiny blocks, with a separate head, torso, arms and legs, posed about 10 times a second"; "Each figure's timing is offset by its seed, so a crowd never moves in step"; "people and animals become tiny outlined figures in their strongest colours"; "with no jump as its forms change"; "a camp of 30 people stays readable at every zoom stop"; "sharp near what casts them and softer as they lengthen"; "A fire is a warm, flickering light as bright as its heat"; "lit inside only by openings and fires"; "ground cover is drawn by the patch"; "kept per patch of a few metres: which kinds, how dense, their season state and ripe yield"; "Where you look, and how fast time runs, never change what happens"; "Every picture, sound and word shows what is really there and what really happened in the world."; "Nothing is added for show."; "Every system grows by adding self-contained pieces"; "at least 97% of frames on time while zooming, panning and turning, at every zoom, and none more than 50 ms late"; "an hour's play uses about 25–30% of the battery, and the phone never gets uncomfortably hot".

`ARCHITECTURE.md` quotes used: "The 3D world renders into a SubViewport at a quarter of the screen's width and height"; "A camera locked to the pixel grid"; "Fires reach figures and tents through a firelight term in our shaders, summing up to four fires (P2)"; "Two maps of the heights round the fires, 48 m across at 512 pixels, are drawn every frame as people move"; "Every texture is drawn at 16 texture pixels a metre"; "people from one block figure"; "our C++ poses the rigid parts, with no skeletons"; "drawn as MultiMesh copies grouped by area, since a MultiMesh is culled as one"; "Grass and small stones only near."; "Orthographic and pitched for the close stops"; "Perspective at every stop, each chunk set on the sphere by its vertex shader, exact near the focus (P8's second round)."

**Godot 4.7.2 source** (https://github.com/godotengine/godot/tree/4.7.2-stable):
- `doc/classes/Viewport.xml`: "A value of [constant Viewport.MSAA_2X] or [constant Viewport.MSAA_4X] is best unless targeting very high-end systems."; "This has no effect on shader-induced aliasing or texture aliasing."; "Note, if hardware does not support VRS this property is ignored."; "FSR is slightly more expensive than bilinear, but it produces significantly higher image quality."
- `servers/rendering/renderer_rd/forward_mobile/render_forward_mobile.cpp`: "textures.push_back(vrs_texture); // 2 - vrs texture."; "pass.resolve_attachments.push_back(color_buffer_id);"
- `servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.cpp`: "// TODO: Detect when it is safe to use RD::TEXTURE_USAGE_TRANSIENT_BIT for RB_TEX_DEPTH, RB_TEX_COLOR_MSAA and/or RB_TEX_DEPTH_MSAA."
- `servers/rendering/renderer_viewport.cpp`: in `viewport_set_scaling_3d_scale`, "viewport->scaling_3d_scale = CLAMP(p_scaling_3d_scale, 0.1, 2.0);" then "_configure_3d_render_buffers(viewport);"
- `doc/classes/Image.xml`: "If [param use_mipmaps] is [code]true[/code], loads the mipmaps for this image from [param data]."
- `doc/classes/Material.xml`: "all objects with [member render_priority] [code]1[/code] will render on top of all objects with [member render_priority] [code]0[/code]."
- `servers/rendering/renderer_rd/forward_mobile/render_forward_mobile.h`: "uint64_t priority : 8;"; "return (A->sort.sort_key2 == B->sort.sort_key2) ? (A->sort.sort_key1 < B->sort.sort_key1) : (A->sort.sort_key2 < B->sort.sort_key2);"
- `servers/rendering/renderer_rd/forward_mobile/scene_shader_forward_mobile.cpp`: "priority = p_priority - RSE::MATERIAL_RENDER_PRIORITY_MIN; //8 bits"; "actions.render_mode_values["depth_test_inverted"]"
- `servers/rendering/renderer_rd/forward_mobile/scene_shader_forward_mobile.h`: "bool has_base_alpha = (uses_alpha && (!uses_alpha_clip || uses_alpha_antialiasing));"
- `servers/rendering/renderer_rd/shaders/forward_mobile/scene_forward_mobile.glsl`: "// If we are not edge antialiasing, we need to remove the output alpha channel from scissor and hash" (followed by `alpha = half(1.0);`); `scene_shader_forward_mobile.cpp`: "actions.usage_flag_pointers["ALPHA_ANTIALIASING_EDGE"] = &uses_alpha_antialiasing;"
- `scene/resources/material.cpp`: "ALPHA_ANTIALIASING_EDGE = alpha_antialiasing_edge;", written when the standard material's alpha antialiasing is not off
- `doc/classes/BaseMaterial3D.xml`: "Stencil preset which shows a silhouette of the object behind walls."; "May be affected by future rendering pipeline changes."
- `servers/rendering/renderer_rd/shaders/skeleton.glsl`: "#[compute]"; "layout(set = 2, binding = 0, std430) buffer restrict readonly SkeletonData {"
- `servers/rendering/renderer_rd/storage_rd/mesh_storage.cpp`: "if (sk && sk->version != mi->skeleton_version) {"; "buffer = s->skin_buffer;"
- `servers/rendering/renderer_rd/shaders/particles.glsl`: "#[compute]"; "texture(sampler2D(height_field_texture, SAMPLER_LINEAR_CLAMP), uv_pos)"
- `doc/classes/OmniLight3D.xml`: "When using the Mobile rendering method, only 8 omni lights can be displayed on each mesh resource. Attempting to display more than 8 omni lights on a single mesh resource will result in omni lights flickering in and out as the camera moves."
- `doc/classes/MultiMesh.xml`: "As a drawback, if the instances are too far away from each other, performance may be reduced as every single instance will always render (they are spatially indexed as one, for the whole object)."; "Limits the number of instances drawn, -1 draws all instances. Changing this does not change the sizes of the buffers."
- `doc/classes/ImageTextureLayered.xml`: "The first image decides the width, height, image format and mipmapping setting. The other images [i]must[/i] have the same width, height, image format and mipmapping setting."
- `modules/astcenc/register_types.cpp`: "#ifdef TOOLS_ENABLED" around "Image::_image_compress_astc_func = _compress_astc;"
- `platform/android/java/app/src/main/AndroidManifest.xml`: "android:configChanges="layoutDirection|locale|orientation|keyboardHidden|screenSize|smallestScreenSize|density|keyboard|navigation|screenLayout|uiMode""

**Godot 4.7 documentation:**
- https://docs.godotengine.org/en/4.7/tutorials/performance/optimizing_3d_performance.html: "*This is only implemented in the Forward+ renderer, not Mobile or Compatibility.*"
- https://docs.godotengine.org/en/4.7/tutorials/performance/vertex_animation/animating_thousands_of_fish.html: "bones are animated on the CPU and so you end having to calculate thousands of operations every frame and it becomes impossible to have thousands of objects."

**Imagination Technologies (PowerVR):**
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/msaa-performance.html: "2x MSAA is virtually free on most PowerVR graphics cores (Rogue and Volcanic onwards), while 4x MSAA+ will noticeably impact performance."
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/performing-msaa-on-powervr-using-vulkan.html: "the application should use a lazily-allocated MSAA frame buffer attachment"
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/sorting-geometry-effectively-on-powervr.html: "it is advised that the application renders the opaque objects first, followed by the alpha-tested objects, and finally the transparent geometry."
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/efficient-sprite-rendering.html: "The discard keyword should be avoided in favour of alpha blending, which is much faster."; "PowerVR hardware has excellent vertex processing capabilities and is designed to handle large amounts of geometry data"; "the amount of wasted fragment processing can be reduced to just 3%."
- https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/rules/do-not-use-depth-pre-pass.html: "should not perform a depth pre-pass as there is no performance benefit."
- https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/rules/do-not-use-discard.html: "alpha-tested primitives cannot write data to the depth buffer until the fragment shader has executed and fragment visibility is known."
- https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/rules/do-use-texture-compression.html: "Reduce the memory footprint and bandwidth cost of the texture assets."
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/triangle-size.html: "especially dipping below 32 pixels per primitive."
- https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/texture-sampling.html: "Vertex shader texture lookups always count as dependent texture reads"

**Others:**
- Saber, REAC 2025 slides, https://enginearchitecture.org/downloads/REAC_2025_Saber.pdf: "Static scene ~5MB, 300K instances"; "0 to 127 index in a customization palette, used to adjust shading of the instance without de-instancing"; "Copy visible instances on this frame to scratch buffer"
- Terrain3D, https://terrain3d.readthedocs.io/en/latest/docs/texture_prep.html: "All albedo textures must be the same size, and all normal textures must be the same size."; "Exported games may not even work since Godot’s image compression libraries only exist in the editor."
- Thomas Vasseur, Dead Cells (2018, old), https://www.gamedeveloper.com/production/art-design-deep-dive-using-a-3d-pipeline-for-2d-animation-in-i-dead-cells-i-: "The disappointing level of details is and always will be an issue in my eyes, but we decided to favor the animations over that and we take full responsibility for the choice."
- Unity, Zugalu case study (25 June 2025), https://unity.com/resources/zugalu-entertainment-thrive-heavy-lies-the-crown: "The memory cost is a factor of the number of vertices multiplied by the number of baked frames."; "With bone baking, different characters with the same bone structure can share the baked animation data."; "When moving from vertex baking to bone baking, the team reduced the number of character draw calls from 1,000 to 20 instanced draw calls."
- GDC 2015 (old), https://gdcvault.com/play/1022141/Massive-Crowd-on-Assassin-s: "With the limit of 40 real AIs and 120 high resolution models, we could successfully create a scene where 10,000 crowd NPCs are on screen at the same time."
- GDC 2021, https://gdcvault.com/play/1027214/Advanced-Graphics-Summit-Procedural-Grass: "To this end, Sucker Punch Productions chose to render their fields by generating individual blades of grass on the GPU that could each have their own procedural appearance and animation."
- Guerrilla, GDC 2017 (old), https://www.guerrilla-games.com/media/News/Files/GDC2017_VanMuijden_GPUBasedProceduralPlacementInHorizonZeroDawn.pdf: "Deterministic"; "100.000+ objects in scene"; "~250µs avg busy load on GPU"
- David Holland (2024), https://www.davidhol.land/articles/3d-pixel-art-rendering/: "The grass is a number of billboard quads with a grass texture, evenly lit to blend in with the terrain, creating pleasant boundaries and correctly layered tufts of grass."
- Google Filament, https://google.github.io/filament/Filament.md.html: "Before rendering a frame, each light in the scene is assigned to any froxel it intersects with. The result of the lights assignment pass is a list of lights for each froxel."; "On non-OpenGL ES 3.1 devices, lights assignment can be performed efficiently on the CPU."
- HypeHype, REAC 2023 slides, https://enginearchitecture.org/downloads/reac2023_modern_mobile_rendering_at_hypehype.pdf: "~90% of the data is unchanged from the previous frame"; "Persistent data: Upload once at startup. Delta update when data changes."
- Unity, https://unity.com/blog/engine-platform/batchrenderergroup-sample-high-frame-rate-on-budget-devices: "At the end, upload 8 bytes per item instead of 112. This leads to a 14x speed-up during GPU data upload."
- Cities: Skylines II analysis (2023, old), https://blog.paavo.me/cities-skylines-2-performance/: "in my test frame 4828 out of 6705 draw calls were for shadow mapping, a staggering 72%."
- Bevy issue 25788 (open), https://github.com/bevyengine/bevy/issues/25788: title "Pixel 11 Pro XL (PowerVR C-Series) aborts before the first frame: get_pixel10_driver_version matches one literal adapter name"; error "E spvcompiler: Unhandled sampler flag combo"; driver "25.3@6908880". Read through a summarising fetch, in round 1 and again on 6 October 2026, never as the raw page (GitHub's page and API are closed to this session). The quote checker read it twice more through WebFetch: both reads give these exact words, and show the issue open, opened on 14 September 2026; they match study 2's round-1 note and our phone's driver build.

**Not verified:** whether this chip offers VRS; whether Godot writes its MSAA buffers out each frame; how fast this chip reads textures in the vertex stage; what a MultiMesh draw costs on Godot's Mobile renderer here; whether a MultiMesh shader that reads bone attributes works end to end (read in the source only); whether GPUParticles3D fails on our phone (inferred from Bevy's report on the same driver); every millisecond, megabyte and count marked "estimate".
