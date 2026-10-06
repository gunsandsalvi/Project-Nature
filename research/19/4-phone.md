# Study 4, round 2: what the phone affords for the chosen look

> Study 4 of [research 19](../19-graphics.md), written on 6 October 2026 and kept as written, its quotes checked ([the check](quote-check.md)).
> `R` was the research session's working folder and is not kept, except your pictures and answers, now in `art/targets/` and `art/reviews/2026-10-06-graphics/`.

M2 research, round 2, study 4 of 7, 6 October 2026. Research only: nothing here is decided until the owner says so (`PRC-07`). Rewritten for direction B after the owner's 32 answers (`R/owner/ask/answers.md`); the earlier draft, which costed drawing the world small, is kept as `R/work2/4/4-phone-old-draft.md`.

## 1. The question, and the answer in brief

**The question:** what can the owner's Pixel 11 Pro XL afford for the look they chose, part by part, and which choices keep 60 frames a second, the battery target and cool hands? The look is direction B: a smooth, sharp 3D world at the phone's full resolution, wearing pixel-art textures (one texture pixel about 2 × 2 screen pixels at the closest zoom), with the liked picture's dense, airy, wild plants, detailed people and animals, smooth light, no outlines, and darkening in corners and under things, which we must build ourselves.

**The answer in brief:**
- **Drawn the plain way, the chosen look does not fit.** Full resolution is 2,596,320 pixels, four times the old 2 × 2 picture, so every cost paid per pixel is four times dearer. The liked camp drawn plainly needs about 13 to 45 ms of the graphics chip a frame, against the 8 ms line.
- **Savings that do not show bring it close, not under.** Corner darkening baked into shapes and maps, fire shadows from a small map per fire, a 2,048-texel sun map with pre-blurred soft shadows and only big things in it, the merged pass kept, a half-resolution mirror and fire glow as sprites take the busy close scenes to about 5–23 ms. My central estimate is about 10 ms at the closest zoom and 9–13 ms at the close camp. So 8 ms holds only if the chip's real costs sit in the cheaper part of my ranges.
- **Two engine changes would close most of the gap.** A depth pre-pass for the leaves does not change the picture; 2 × 2 shading on ground and plants may show faintly close up. The phone's chip supports 2 × 2 shading for each draw, but Godot 4.7.2 never uses it. Together they bring the central estimates to about 6–8 ms. Stock Godot's fallback is drawing the 3D at 0.75 scale in the costliest scenes, which looks softer. Half resolution stays the last lever, used only where needed, as the owner asked.
- **The texture-pixel size costs no time.** 2 × 2 costs the same as 3 × 3 or 4 × 4 would; only texture memory grows.
- **Battery fits; heat decides.** At 8 ms the whole phone draws about 3–5.5 W, 16–28% of the battery an hour, inside the 25–30% target. Comfortable long play needs about 4 W or less, so a 20-minute heat run, not the battery, sets the real line.
- **Textures:** tiles in the chosen style are about 120–190 KB each even stored losslessly, so a few hundred will not fit beside Godot under the 50 MB APK rule. Make them on the phone at first start; ship approved pictures compressed only if the owner sees no loss; never download (`PLT-03`).
- **Measure first, in this order:** the fixed cost with the world hidden, the material at full resolution, the cost of a triangle, the ways of drawing leaves at the liked density, the fires, then the closest-zoom camp, the camp of thirty and a 20-minute heat run. Pass lines are in 6.4, stated before any run (`RES-09`).

**Words used here:**
- *Graphics chip* (GPU) draws; the *processor* (CPU) runs the game; Godot's *main thread* prepares every frame. *ms at a high clock:* graphics time measured at a 120-frame cap, so idle time is not counted.
- *Full resolution:* the screen's own 1080 × 2404 pixels. *Texture pixel:* one square of a pixel-art texture.
- *Pass:* one drawing of the scene into one picture. *The merged pass:* Godot's Mobile renderer can draw the whole picture in one pass that stays on the chip until it is finished; reading the screen or depth, glow or scaling breaks it, and each break costs memory traffic.
- *Cut-out:* a flat card whose see-through texture pixels are thrown away pixel by pixel, the usual way to draw airy leaves and grass. *Hidden-surface removal:* this chip's way of shading only the nearest solid surface in each pixel; cut-outs defeat it.
- *Parameter buffer:* the chip's store, in the phone's memory, of every triangle after it is placed.
- *MSAA:* smoothing the edges of shapes by testing 2 or 4 points in each pixel.
- *2 × 2 shading* (variable-rate shading): working out the colour once for each 2 × 2 block of pixels, while the edges of shapes stay sharp.
- *Engine patch:* a change to Godot's own C++ in our own build of it.
- *Central estimate:* the middle of a range taken by multiplying (the geometric mean), since my ranges are wide.
- *Headroom:* Android's measure of how close the phone is to slowing itself for heat; *light*, *moderate* and *severe* are its throttling levels.

## 2. What `PROJECT.md` asks

- `PLT-04` **Measured limits** (not lifted): "at least 97% of frames on time while zooming, panning and turning, at every zoom, and none more than 50 ms late"; "an hour's play uses about 25–30% of the battery, and the phone never gets uncomfortably hot"; memory within about 8 GiB; a world opens in about 3 seconds. Its benchmark worlds include "a camp of about 30 and a village of about 300 at close camp zoom, a camp in thick forest at camp zoom with the camera turning".
- `PLT-01` **One phone:** the simulation may use up to the four middle cores, "leaving the rest for the picture, sound and the writer"; it plans on "about 3 W".
- `PLT-03` **Works offline:** "Everything the game needs, the writer AI included (`PRE-37`), is on the phone; nothing in play makes a network call."
- `PRN-11`: "When the phone can't keep up, the world doesn't cut detail: time simply runs more slowly." `PRN-10`: nothing is faked. `VIS-14`: "Beautiful, smooth and absorbing in your hand."
- `RES-09`: every test states "its pass rule in exact numbers" before it first runs. `RSK-24`'s warning sign: "the battery over about 40 °C in long sessions".
- **Lifted for this research and touched by cost:** `PRE-22`'s fixed sizes, `PRE-27`'s tiny blocks, A5.2 and A5.3, and "made by code" in `MIL-09` and `PRE-46` (BRIEF2). The cost side of their new wording is in 6.6.
- **The owner's answers that set the cost:** direction B (1) at 2 × 2 (2); the closest zoom as in the liked picture, about 8 m across (5); detailed people (3) and animals (14); the plants "As I liked" (6), refusing all four cheaper ways; no outlines, smooth light, the fine grain and our own corner darkening, "As I liked" (7); rain with all four layers (17); half-resolution shading "only if needed" (25); a camp of thirty at about ten tasks feels "alive" (26).

## 3. Where we stand

**Measured on the owner's phone** (`LESSONS.md`, `dist/M1-REPORT.md`):
- P1's close camp at 4 × 4: 9.5–10.1 ms a frame at 60, partly idle.
- P2's night camp, 30 figures and 3 fires: 5.5 ms for the picture's own pass at 120. P2's forest of 12,000 trees of 44 triangles drawn three times: 17.8 ms, 65% of frames on time.
- P3's model sheet with fire shadows and smoke: 4.3 ms at 120. P1's mirror: at most 0.3 ms.
- M1: 1.0 W in all at real speed; 5.0–5.6 W at top speed raised the heat forecast to 0.77–0.83 within minutes; the screen alone about 1 W; the APK 27.6 MiB.

**What round 1 found** (my note and its addendum, `R/round1/notes/4-phone.md`):
- The 8 ms line is sound. Pre-production's 4,096-texel sun map was probably a large, never-isolated share of P2's and P3's time.
- With the world drawn small, density cost more than resolution: 2.3–6.1 ms at 4 × 4 with moderate plants, up to 8.7–32 ms at 2 × 2 with dense cut-out plants.
- Measure at a 120 cap and by power; Android's 1.0 of heat headroom is *severe* throttling, not the first level.
- The other studies' first answers: study 1 found the liked pictures' brightest colours golden, a quarter of each picture dark, and little lost at 2 × 2; study 2 found a true 2 × 2-sized grid keeps most of the look; study 3 laid the engine out as families on Godot's RenderingServer; study 5 kept the kit made by code; study 6 proposed a target card and quick choices; study 7 a text-only pipeline.

**My first round-2 draft** costed drawing the world small and enlarging it. It offered four cheaper plant ways, each a GPT repaint of the liked camp (section 10). The owner accepted none: "As I liked". That model is kept as `R/work2/4/faux2.py`; this note uses a new one at full resolution, `R/work2/4/faux3.py` (its output in `faux3.out` and `patches.out`).

**What direction B changes for cost:**
1. **Four times the pixels.** Every cost paid per pixel is four times the 2 × 2 picture's: shading, cut-out leaves, fire shadows, water, smoke, mist, rain.
2. **The texture-pixel size no longer sets the time.** A texture read costs the same whatever its texture pixels' size. The owner's 2 × 2 costs no more time than 3 × 3 or 4 × 4 would; it needs about 2.25 and 4 times their texture memory for the same metres.
3. **One saving comes free.** With no outlines, the second drawing of every shape for the outline picture (A4.1, 3) goes; with dense plants it was one of the dearest parts.
4. **New musts that cost:** smooth soft shadows that widen with distance (Mobile has none built in), corner darkening (Mobile has no screen-space kind), smooth edges at full resolution (MSAA), and the merged pass at full resolution, where each break costs about 20–40 MB of memory traffic a frame.

**What the design says, and what is weak for B:**
- A4.1 draws into a low-resolution SubViewport with an outline pass and light in steps; all three go under B.
- A4.1's fire shadows walk up to 32 steps for each fire in every pixel the fire lights. At full resolution and the closest zoom that is 2.5–5 ms for one fire (5.5).
- A4.1 draws the shore line "from the depth texture", which breaks the merged pass.
- A4.3 says nothing about triangles a frame, cut-outs, MSAA or full-resolution memory traffic.
- A5.3's 16 texture pixels a metre was set for 4 × 4 art pixels; B needs about 64–67 at the closest zoom.
- A18.1 has one aim, "the graphics chip under about 8 ms in the busiest scene, so heat leaves room", with no parts.
- `IMPLEMENTATION.md`'s M2 names as its cost risk "The cost of the outline pass and of mirrored water at the phone's resolution". Under B the outline pass goes; the risks become full-resolution shading, cut-out leaves and fire shadows.
- A3.9's heat guard acts "as the forecast nears the first throttling level", and `heat.toml` calls 100% "the first level at which it slows itself"; Android defines 1.0 as severe.

## 4. What the makers say, and what the phone reports

### 4.1 The chip (Imagination's own guides)

- **Hidden solid layers are nearly free:** "With PowerVR TBDR, Hidden Surface Removal (HSR) will completely remove overdraw regardless of draw call submission order." So no depth pre-pass for solid things: "On PowerVR hardware, there is no performance benefit to rendering a low-poly geometry Z pre-pass to save fragment processing later."
- **Cut-outs are not:** "alpha-tested primitives cannot write data to the depth buffer until the fragment shader has executed and fragment visibility is known." And: "These deferred depth writes can impact performance, as subsequent primitives cannot be processed until the depth buffers are updated with the alpha tested primitive’s values."
- **Shapes cut close to the picture help:** "One method to minimise the impact of several layers of blended sprites is to increase the geometry complexity of the sprites, to reduce the amount of wasted transparent fragments." Also: "PowerVR hardware has excellent vertex processing capabilities and is designed to handle large amounts of geometry data".
- **MSAA:** "2x MSAA is virtually free on most PowerVR graphics cores (Rogue and Volcanic onwards), while 4x MSAA+ will noticeably impact performance." On sharp screens it may not be needed at all: "In some cases, there may be no need for anti-aliasing to be used at all, for example when the target device’s display has high pixels per-inch (PPI)." The phone has 390 dpi. Blended edges cost more: "On edge blend is a costly operation, as the blending is performed for each sample by a shader in software." The best way keeps the extra samples on the chip: "the application should use a lazily-allocated MSAA frame buffer attachment".
- **Triangles are memory traffic:** "The tile list and the transformed vertex data are both stored in an intermediate store called the Parameter Buffer (PB). This store resides in system memory". Small ones are inefficient, "especially dipping below 32 pixels per primitive".
- **Memory is the dear thing:** "System memory accesses use more bandwidth and power than any other graphics operation." Shadow maps: "Techniques that require results to be written to off-chip memory, such as shadow mapping, will usually perform worse than techniques that can be computed entirely in on-chip memory." Changing pictures mid-frame: "changing render targets is a fundamentally expensive operation".
- **Textures:** mipmapping "increases graphics rendering performance by massively improving texture cache efficiency"; "dependent texture reads should be avoided wherever possible for good performance" (the smooth-pixel texture filter is one). Particles: "lots of alpha-blended particles can cause a massive overdraw issue".
- **The chip:** a PowerVR CXTP-48-1536 at 1.29 GHz (Android Authority). About 2 trillion operations a second if its name follows Imagination's DXT naming (unverified, as in round 1). Geekerwan, through Notebookcheck (secondary): "The Tensor G6's Steel Nomad Light result peaks at around 1,200 points at roughly 6W".

### 4.2 Godot 4.7.2's Mobile renderer, read in the 4.7.2-stable source

- **The merged pass** holds only while nothing reads the screen or depth ("// - not reading from SCREEN_TEXTURE/DEPTH_TEXTURE") and there is no glow ("// We don't support glow or auto exposure here, if they are needed, don't use subpasses!") and no 3D scaling ("// can't do blit subpass because we're scaling"). A colour grade through Godot's adjustments and a colour table keeps it.
- **What Godot writes to memory every frame.** The MSAA colour and depth buffers are created discardable, so their contents stay on the chip (they still take memory). But the plain depth and colour buffers are made with `p_discardable = false`, so they are stored every frame, about 10 MB each at full resolution. Godot's own note: "// TODO: Detect when it is safe to use RD::TEXTURE_USAGE_TRANSIENT_BIT for RB_TEX_DEPTH, RB_TEX_COLOR_MSAA and/or RB_TEX_DEPTH_MSAA." With MSAA on, the depth is not stored unless something reads it.
- **2 × 2 shading.** Godot's documentation: "Variable rate shading (VRS) is a method of decreasing this shading cost by reducing the resolution of per-pixel shading (also called fragment shading), while keeping the original resolution for rendering geometry." and "Both Forward+ and Mobile renderers support variable rate shading." But Godot turns it on only when the chip takes a shading-rate *texture* (`if (fsr_capabilities.attachment_supported) {` in `_vrs_detect_method`) or a density map. It never sets a rate for each draw: "// We don't use pipeline/primitive FSR so this really doesn't matter." And: "On unsupported hardware, there is no visual difference when variable rate shading is enabled."
- **No leaf pre-pass.** Mobile has no depth pre-pass of the main view: in its header the depth pass is commented out (`// PASS_MODE_DEPTH,`), and it sets `opaque_prepass_threshold = 0.0` for the picture. Godot's depth pre-pass transparency mode (`TRANSPARENCY_ALPHA_DEPTH_PRE_PASS`) "will discard fragments with an alpha of less than 0.99 during the depth prepass", but only the Forward+ renderer draws that pre-pass. Mobile's materials can only test depth as nearer-or-equal (`COMPARE_OP_GREATER_OR_EQUAL`, the depth being reversed) or inverted; there is no equal test, which a pre-pass of leaves needs.
- **Lights and copies:** Mobile culls at most 8 lights per geometry instance (`MAX_RDL_CULL = 8`), and "A MultiMesh is a single object, therefore the same maximum lights per object restriction applies". So fires reach things through our own firelight term (A4.1), as now.
- **Figures:** each Godot skeleton gets its own skinning work and draws. Godot's own docs: "Animation and vertex animation such as skinning and morphing can be very expensive on some platforms", and for crowds, "bones are animated on the CPU and so you end having to calculate thousands of operations every frame and it becomes impossible to have thousands of objects." Study 5 found that Godot re-skins a figure only when its skeleton changes, so poses held about 10 times a second (`PRE-44`) are cheaper.
- **Textures in the exported game:** the ASTC encoder is built "# Build the encoder only for editor builds"; the Basis Universal transcoder turns shipped files into ASTC 4 × 4 on the phone (`basisu_format = basist::transcoder_texture_format::cTFASTC_4x4_RGBA;`). Godot advises: "Even in 3D, "pixel art" textures should have VRAM compression disabled as it will negatively affect their appearance, without improving performance significantly due to their low resolution."
- **Measuring hooks:** each viewport's GPU time, per-pass timestamps, and counters for triangles, draws and video memory (round 1).

### 4.3 The phone itself, from the Vulkan Hardware Database

The database holds report 51167 from a Pixel 11 Pro XL: the PowerVR C-Series CXTP-48-1536 MC1, driver version 1.662.3024 ("25.3@6908880"), Vulkan 1.4.317, Android 17. That is the owner's chip and driver, as their self-check gives them (A4.3). It reports:
- `VK_KHR_fragment_shading_rate` with **shading rates for each draw** (`pipelineFragmentShadingRate` true, blocks up to 4 × 4), but **no shading-rate texture** (`attachmentFragmentShadingRate` false) and **no density map** (`VK_EXT_fragment_density_map` absent).
- So 2 × 2 shading is possible on this chip, but only through an engine patch that sets a rate for each material. Stock Godot's VRS does nothing here.
- A lazily allocated memory type exists (Imagination's advice for MSAA), and 16-bit float maths (`shaderFloat16` true).

### 4.4 Android on the phone (API level 37)

- **Heat:** headroom "only attempts to track the headroom of slow-moving sensors, such as the skin temperature sensor": it is the app's view of how hot the hand feels. "A value of 1.0 indicates that the device is (or will be) throttled at {@link #ATHERMAL_STATUS_SEVERE}." Since API 35 the phone gives its own light and moderate thresholds; "Starting in Android 16, this polling API may return different results when called depending on the device", and since API 36 a listener pushes changes.
- **How busy the graphics chip is:** since API 36, `ASystemHealth_getGpuHeadroom` "Provides an estimate of available GPU capacity headroom of the device", from 0 to 100, "where 0 indicates no more gpu resources can be granted". Some phones may not offer it.
- **Power:** BatteryManager's current and charge, as M1 uses; and the phone's power rails, refreshed at most every 30 s with up to 10 J of noise (`MAX_POWER_MONITOR_AGE_MILLIS = 30_000;`, `MAX_RANDOM_NOISE_UWS = 10_000_000;`).
- **A frame counter the owner can see:** the Pixel's Game Dashboard (Android's developer page); Godot's export marks the app as a game.

### 4.5 Evidence on power, heat and crowds

- **The phone in games:** on the Pixel 11, Genshin Impact "crept up to over 40°C after five minutes", and "Asphalt Legends and Genshin Impact sadly can’t lock a 60fps figure in" (Android Authority). The Pixel 10 Pro XL drew "an unsustainable 7.2W average during gameplay that will drain the battery quickly".
- **The owner's own phone:** 5.0–5.6 W for about four minutes raised the forecast to 0.77–0.83 (M1).
- **The battery:** "The Pixel 11 Pro XL is powered by a 5,115mAh battery" (GSMArena): about 19.7 Wh, so 25–30% an hour is 4.9–5.9 W on average.
- **Crowds in Godot** (a desktop post, Godot 4.2, January 2026): "it runs at 36 fps with almost 300 units, so 50-100 would be safe". Its author was not sure, but thought the cost lay more on the processor than on the graphics chip.

## 5. The options

### 5.1 Costs part by part, at full resolution

From `faux3.py`, for the liked camp at the closest zoom (S1, 8 m across), graphics-chip ms at a high clock. **C** = certain (read in source or counted); **E** = estimated; **P** = only the phone can settle it.

| Part | Cost | How sure |
|---|---|---|
| **Fixed:** the interface pass, the copy to the screen, the main pass's stores | 0.9–1.7 ms; 62 MB of memory traffic a frame with MSAA 2×, 73 MB without (3.7–4.4 GB/s) | C that it happens and its megabytes; E the ms; P with the world hidden |
| **Shading:** the material through the smooth-pixel filter, sun and sky light, soft shadows, darkening from maps, haze, the light of up to four fires, the grade | 1.7–4.0 ms with pre-blurred soft shadows (0.8–1.9 clocks a pixel); 2.4–6.5 with shadows softened by a wide search | E; **P: the second biggest unknown** |
| **MSAA 2×** | about 0–0.9 ms, more where edges are many; it also stops the depth store | C Imagination's "virtually free"; E; P |
| **Sun shadow map** | 2,048 texels with big casters only: 0.3–0.7 ms; 4,096 with every leaf cut out in it: 1.8–6.4 | C the sizes; E the ms |
| **Geometry,** all passes | 2.5–10 ns a main-pass triangle; S1 0.6–5.7 ms depending on how leaves are drawn | E (high end near P2's forest); **P: the biggest unknown** |
| **Leaves' hidden layers** | plain cut-out cards 1.5–6.0 ms; opaque cores 0.6–2.1; a leaf pre-pass 0.5–1.8 (5.4) | C that cut-outs defeat hidden-surface removal; E; P |
| **Fires** | the design's walk 2.5–5.0 ms a fire at the closest zoom; a small map per fire 0.2–0.5 (5.5) | E; P |
| **Water and mirror** | 0.5–1.8 ms with a half-resolution mirror; 1.0–3.1 with a full one | E; P1 measured the mirror at most 0.3 ms at 4 × 4 |
| **Corner darkening** | baked into the kit's shapes plus openness and contact maps: 0.03–0.1 ms; our own screen-space kind: 2.0–5.0 ms, and it breaks the merged pass | C Mobile has none; E |
| **Haze, grade, glow** | haze in the light function about 0; the grade 0.02–0.05 ms, merged pass kept; Godot's glow 0.5–1.2 ms, merged pass lost | C the merge rules; E |
| **A broken merged pass** | 0.4–1.2 ms (about 20–40 MB a frame), whenever anything reads the screen or depth | C what breaks it; E |
| **The effects of the pictures** | rain's four layers: two full-screen streak sheets 0.4–1.7 ms (a few thousand thin streak particles instead: about 0.06), splashes 0.05–0.15, rings on water 0.1–0.3, drifting mist 0.2–0.7; snowfall or embers 0.05–0.15; snow cover in the material, about 0; smoke 1–3 clocks a covered pixel (0.1–0.3 ms by a hearth); lake mist in two layers 0.5–2.0; foam 0.3–1.0 clocks a water pixel; lightning 0.01–0.05, its flash a change of light | E |
| **Texture-pixel size** 2, 3 or 4 screen pixels | the same time; texture memory 4, 1.8 and 1 times | C |

### 5.2 The scenes, drawn plainly and with the savings that do not show

Nine scenes from the owner's answers and `PLT-04`'s benchmark worlds. *Plainly* means the straightforward way, with pre-production's settings where it had them: plain cut-out cards for every plant, a 4,096-texel sun map with every leaf in it, shadows softened by a wide search, our own screen-space darkening, Godot's glow, the design's fire walk, the shore from depth, a full mirror, smoke against a screen copy, MSAA 2×. *With the savings* means the savings of 5.3, with the leaves drawn in whichever stock way calibration finds cheapest (5.4).

| Scene | Plainly | With the savings (central) | Triangle-passes |
|---|---|---|---|
| S1 closest zoom, the liked camp by day, 7 people, 1 fire | 13.3–38.3 | **5.5–16.6 (9.6)** | 609 k |
| S2 the same at night, three fires | 16.5–45.0 | **5.9–17.9 (10.3)** | 609 k |
| S3 close camp, the camp of thirty at ten tasks, by day | 12.9–39.1 | **6.2–19.7 (11.1)** | 741 k |
| S4 close camp, the village of 300 at night, about 90 in view, 16 fires | 12.9–36.0 | **5.7–17.3 (9.9)** | 592 k |
| S5 close camp in a storm: rain's four layers, lightning, a burning oak | 13.3–40.8 | **6.7–21.5 (12.0)** | 613 k |
| S6 close camp in winter, soft light, clear water | 10.8–30.7 | **5.1–14.8 (8.7)** | 387 k |
| S7 lake in morning mist, reeds, canoes | 11.3–33.3 | **6.0–17.9 (10.4)** | 212 k |
| S8 autumn wood with people, falling leaves | 12.8–42.5 | **6.9–23.1 (12.6)** | 860 k |
| S9 camp zoom, a camp in thick forest, turning; crowns as closed shapes | 9.8–29.2 | **4.0–10.6 (6.5)** | 380 k |

The ranges are wide because each part's range is wide, and they add. The share of each part is steadier than the totals. With the savings, at the closest zoom, shading is about a quarter to a third of the frame, geometry and leaves together a third to a half, and the fixed part a tenth to a sixth.

### 5.3 The savings that do not show

Each one alone, taken from the plain frame (ms saved):

| Saving | S1 | S3 | What it changes |
|---|---|---|---|
| Corner darkening baked into the kit's shapes and read from openness and contact maps, not worked out on screen | 2.0–4.9 | 2.0–4.9 | nothing, if done well (studies 2, 3); it also keeps the merged pass |
| Fire shadows from a small map made once per fire, read once or twice per pixel, not walked per pixel | 1.7–3.4 (S2's three fires at night: 4.6–9.0) | 0.5–1.0 | softer fire-shadow edges far from the fire (study 2) |
| No fire shadows where the sun outshines the fire | 1.8–3.6 | 0.6–1.2 | nothing by day |
| Sun map of 2,048 texels, not 4,096 | 1.4–4.5 | 1.4–4.8 | nothing with soft shadows: a texel is about 1.6 screen pixels at the closest zoom |
| Soft shadows from a pre-blurred map, not a wide search | 0.7–2.5 | 0.7–2.5 | softness follows distance less exactly (study 2) |
| Small plants out of the sun map; only trees, bushes, reeds, people, tents and rocks cast | 0.4–2.2 | 0.6–2.8 | a tuft's own tiny shadow becomes darkening at its foot |
| Shore line from the height map, smoke drawn in the pass: the merged pass kept | 0.4–1.2 | 0.4–1.2 | nothing |
| Mirror at half resolution with a reduced set (no grass) | 0.6–1.9 | 0.6–2.1 | plainer reflections, broken by ripples anyway |
| Fire glow as soft sprites in the pass, not Godot's glow | 0.5–1.1 | 0.5–1.1 | glow only round fires and embers |
| Leaves in the cheapest stock way (5.4) | 0.6–3.0 | about 0 | nothing: the same silhouettes |
| The outline pass gone | free | free | the owner chose no outlines up close |
| MSAA 2× rather than 4× | 0.2–0.9 | 0.2–1.1 | slightly less smooth edges |
| Rain streaks as a few thousand thin particles, not full-screen sheets (S5) | — | (S5: 0.3–1.6) | discrete streaks, as in study 2's rain picture |
| The still part of the sun map kept between frames | about 0 | 0.1–0.3 | not worth its complexity on my numbers |
| **All together** (not additive) | **7.8–21.7** | **6.6–19.4** | |

### 5.4 Leaves at the liked density

The owner kept the density and the airy, wild plants. So the saving must come from how leaves are drawn, not how many. Everything else here carries the savings:

| Way | S1 frame | S1 leaves | S1 geometry | S1 triangle-passes | S3 frame | S3 triangle-passes |
|---|---|---|---|---|---|---|
| Plain cut-out cards | 5.9–18.0 | 1.5–6.0 | 0.6–2.8 | 312 k | 6.3–19.8 | 455 k |
| Cards cut tight to the leaf outline (8–12 sides) | 5.6–16.7 | 0.9–3.5 | 1.0–4.2 | 460 k | 6.2–19.7 | 741 k |
| An opaque core per leaf cluster with a cut-out fringe | 5.5–16.6 | 0.6–2.1 | 1.3–5.7 | 609 k | 6.5–20.9 | 1,026 k |
| A depth pre-pass of the leaves, then each pixel shaded once (engine patch) | 4.8–13.6 | 0.5–1.8 | 0.7–2.9 | 330 k | 5.2–15.2 | 491 k |
| Outlines traced at the texture pixel, no cut-outs at all | 7.1–23.6 | 0.2–0.5 | 3.3–14.5 | 1,500 k | 9.9–36.1 | 2,740 k |

The ways that trade cut-out work for triangles win only if triangles prove cheap; at the close camp they barely beat plain cards. The pre-pass wins on both counts, but needs an engine patch (4.2). Which wins rests on two numbers only the phone gives: the cost of a triangle and of a cut-out layer (C2 and C3 in 6.4).

### 5.5 Fires

| Fires in view | The design's walk (A4.1) | The walk at half resolution | A small map per fire |
|---|---|---|---|
| 1 at the closest zoom (its light reaches most of the screen) | 2.5–5.0 | 0.8–1.7 | 0.2–0.5 |
| 3 at the closest zoom | 7.3–14.7 | 2.0–4.1 | 0.4–1.1 |
| 5 at the closest zoom | 12.2–24.4 | 3.2–6.5 | 0.6–1.7 |
| 16 at the close camp (the village) | 2.5–5.0 | — | 0.5–1.2 |

The map per fire is made each frame from the two height maps the design already draws. How it looks against the walk is study 2's question.

### 5.6 Figures

On the graphics chip, detailed figures are affordable if their detail falls with their size: about 3,000–4,000 triangles at the closest zoom (about 200 screen pixels tall), 1,000–1,500 at the close camp, and at most about 60 far away, including the outline copy behind a small figure drawn to read (answer 4). The camp of thirty adds about 0.2–0.9 ms of geometry.

On the main thread, the way of animating decides (ms a frame at 60, the whole main thread in brackets):

| Scene | Godot's skeletons, posed by our code about 10 times a second | Our own batched animation (bones in a texture, MultiMesh) |
|---|---|---|
| S1, 7 figures | 0.2–0.4 (3.5–6.2) | about 0 (3.3–5.8) |
| S3, the camp of thirty | 0.9–2.0 (4.2–7.8) | about 0.1 (3.3–6.0) |
| S4, the village: about 90 in view | 2.2–5.4 (5.5–11.2) | 0.1–0.5 (3.4–6.2) |
| 300 in view | 7.5–18 (10.8–23.8) | 0.3–1.5 (3.6–7.3) |

Godot's own skeletons suit the camp of thirty. The village and crowds need our batched way.

### 5.7 Levers on top of the savings: engine patches, and levers that may show

| Lever | S1 saves | S3 saves | What shows | Needs |
|---|---|---|---|---|
| A depth pre-pass of the leaves | 0.7–3.0 | 1.0–4.5 | nothing | engine patch: a pre-pass and an equal depth test |
| 2 × 2 shading on ground and plants (70% of the screen) | 1.2–3.0 | 1.6–4.2 | perhaps faint 2 × 2 steps in light and in texture-pixel edges, close up; shape edges stay sharp | engine patch: a rate for each material, which the chip supports |
| 2 × 2 shading everywhere | 1.9–4.5 | 2.3–6.1 | the same, on faces too | engine patch |
| 3D drawn at 0.75 scale and enlarged smoothly | 1.0–3.4 | 1.3–4.7 | everything slightly softer; the merged pass lost | stock Godot |
| 3D drawn at half resolution (the owner's q25 B) | 2.1–6.8 | 2.5–8.5 | visibly softer: the owner said "only if needed" | stock Godot |
| MSAA off | 0–0.9 | 0–1.0 | edges crawl a little in motion | stock Godot |

### 5.8 The ways of drawing direction B, compared

Central estimates over the busiest close scenes (S1 to S8), ms:

| Way | Closeness to the pictures' feeling | Central ms (range) | 60 frames and cool hands | Builder's effort | Risk | Fit with the rules |
|---|---|---|---|---|---|---|
| **A.** Plainly, as pre-production drew, at full resolution | the look itself | 18–27 (11–45) | no | low | it fails | fits, but fails `PLT-04` |
| **B.** Every saving that does not show, stock Godot | the same, by design | 9–13 (5–23) | only at the cheap end of my ranges | medium: our own darkening maps, fire maps, shadow blur, mirror | high | fits |
| **C.** B with the leaf pre-pass (engine patch) | the same | 8–10 (5–17) | near the line: about 8 at the closest zoom, up to about 10 in the storm | high: our own build of Godot | medium | fits |
| **D.** C with 2 × 2 shading on ground and plants (engine patch) | nearly the same; to judge on the phone | 6–8 (4–14) | likely | high | medium | fits; the owner's "only if needed" applies |
| **E.** B with 3D at 0.75 scale in the costliest scenes | softer in those scenes | 7–10 (4–18) | likely | low | low | fits if the owner accepts the softness there |
| **F.** 3D at half resolution everywhere | softer everywhere (q25 B) | 5.5–7 (3–12) | yes | low | low | against "only if needed" |

**A** is ruled out: two to five times the line. **B** is the base: it keeps every quality the owner liked and is all our own code, but it meets 8 ms only if the chip is cheap. **C** removes the plants' main cost without changing the picture. It needs our own build of Godot's export templates, which also allows a smaller fix Godot itself lists as a TODO: buffers that need never reach memory. **D** is the most powerful lever, and the chip can do it. Shape edges stay sharp, and texture pixels are already 2 × 2 screen pixels, so it may not show; the owner should judge it on the phone. **E** is stock Godot's fallback, scene by scene. **F** is what the owner saw as softer and asked to use only where needed.

### 5.9 Where the textures come from

Direction B's textures are larger and richer than the old design's. Measured on studies 5 and 7's tiles in the owner's chosen style (`R/work2/4/texsize/`), a tile 256 to 350 texture pixels across holds 20,000–66,000 colours and compresses losslessly to 118–193 KB (WebP lossless 118–163 KB, PNG 139–193 KB). Three or four hundred such tiles come to about 35–70 MB. The APK is 27.6 MiB against the repository's rule ("over the 50 MB a committed file may have"), so about 22 MiB of room.

| Route | App size | First start | Memory in play | `PLT-03` | Content it allows | Risk |
|---|---|---|---|---|---|---|
| **Made on the phone at first start** from code recipes in the catalogue (A6.1), kept in the app's storage | unchanged | an estimated few seconds, once | 120–300 MB uncompressed, with each zoom band's version | fits | what code makes, including tiles from the palette and patterns of approved pictures (study 7's code route; the owner's pick A in question 15) | making time; code-made quality |
| **Shipped losslessly** | +118–193 KB a tile: 35–70 MB for 300–400 tiles, over the room under the 50 MB rule | fast | as above | fits | any approved swatch, exactly (the owner's picks C and D) | the 50 MB rule must change (study 7) |
| **Shipped as Basis Universal UASTC**, "An 8 bits/pixel LDR high quality mode", made smaller by its optional rate-distortion step | an estimated quarter to a third of lossless (unverified) | fast: becomes ASTC 4 × 4 | 1 byte a texture pixel | fits | any approved picture | crisp texture-pixel edges may soften: the owner's eye must check |
| **Downloaded** at first start | unchanged | needs a network once | as shipped | **breaks it** | any | hosting, failure |

Texture time is small either way: at 2 × 2 screen pixels a texture pixel, each texture pixel serves about four screen pixels, so three maps read about 0.5–1 GB a second. Compression saves memory and storage, not frames.

## 6. What we'd recommend

### 6.1 The plain verdict

- **The 8 ms line.** Keep it as the planning line for the busiest close scene. Drawn plainly, direction B cannot meet it. With every saving that does not show, it meets it only if the chip's real costs are in the cheaper part of my ranges; my central estimates are 9–13 ms. Plan from the start for the leaf pre-pass and 2 × 2 shading on ground and plants as the two levers, both engine patches, and for 3D at 0.75 scale as the stock fallback in the costliest scenes only. Never cut the density: the owner refused it.
- **Battery and heat.** At 8 ms of graphics the whole phone draws about 3.1–5.6 W (screen 0.9–1.4, chip 1.6–2.9, processor for the picture 0.4–0.8, simulation 0.2–0.5): about 16–28% of the battery an hour, inside the target. At 10 ms it is 3.5–6.3 W. Heat binds first: comfortable long play needs about 4 W or less. So the 20-minute heat run sets the true line. If the phone stays cool at 9 or 10 ms, the line can rise; if it heats at 8, the line comes down for good, never under load (`PRN-11`).
- **The savings that do not show,** largest first: darkening baked and from maps (2–5 ms); fire shadows from a small map per fire (1.7–3.4 ms for one fire, 4.6–9.0 for S2's three); the 2,048 sun map with only big casters and pre-blurred softness (2.2–8.1 ms together); the leaf pre-pass (0.7–4.5 ms more, engine patch); the mirror at half resolution (0.6–2.1); glow as sprites (0.5–1.1); the merged pass kept (0.4–1.2); the outline pass gone (free).
- **The savings that may show,** only where needed: 2 × 2 shading on ground and plants (1.2–4.2 ms); 3D at 0.75 scale (1.0–4.7); half resolution (2.1–8.5), the look the owner already judged softer.

### 6.2 Budgets for M2

**Graphics chip,** the busiest close scene at full resolution, ms at a 120-frame cap (every viewport summed, plus a run with the world hidden for the fixed part):

| Part | Allowance | My estimate with the savings | Lever if over |
|---|---|---|---|
| Fixed: interface pass, copy, stores | 1.1 | 0.9–1.7 | a lighter interface; Godot's store TODO (engine patch) |
| Shading: material, light, soft shadows, grade | 2.3 | 1.7–4.0 | fewer reads; 16-bit maths; 2 × 2 shading on ground and plants |
| Sun shadow map, 2,048, big casters | 0.4 | 0.3–1.5 | a smaller map at far zooms |
| Geometry, all passes | 1.6 | 1.2–7.7 | detail by size; fewer leaf triangles |
| Leaves' hidden layers | 0.8 | 0.6–8.0 | the leaf pre-pass |
| Fires: maps and shadows | 0.4 | 0.1–1.2 | the nearest four per pixel |
| Water and mirror | 0.6 | 0.1–3.3 | mirror coverage |
| Effects: smoke, mist, rain, snow, glow | 0.5 | 0.1–3.3 | rain streaks as particles; mist at half resolution |
| Corner darkening maps | 0.1 | 0.03–0.1 | |
| Reserve | 0.2 | | |
| **Line** | **8.0** | | |

**Triangles:** at most 0.4 million triangle-passes a frame, all passes counted, until C3 sets the line. Detail falls with size on screen, so that triangles of only a few pixels stay rare: Imagination says efficiency drops "especially dipping below 32 pixels per primitive".

**Main thread,** mean at 60 frames: Godot's culling and draw recording 3.0 (about 300 draws over all passes); figures 1.0; plants' and ground patches' buffers 0.5; scripts and interface 1.0; Godot's other work 0.5; other view work 0.5; reserve 1.5. **Line 8 ms mean, 12 ms at the 99th percentile.** Godot's own skeletons for as many figures as fit that 1.0 ms (my estimate 15 to 40; C6 decides); beyond that, our batched way.

**Figures,** within the triangle line: closest zoom at most 10 in view at up to 4,000 triangles; close camp at most 100 in view at 1,000–1,500; camp zoom up to 3,000 tiny figures at up to 60, drawn without sun shadows.

**Power and heat,** close zooms: the whole phone **at most 4.0 W** on average over the 20-minute run (about 20% of the battery an hour): screen about 1.0–1.4, graphics chip at most 2.0, processor for the picture at most 0.6, simulation at most 0.3. The heat forecast stays at least 0.05 below the phone's own *light* threshold, and the battery ends at or below 40 °C.

**Memory and size:** textures at most 300 MB uncompressed; render targets at most about 150 MB (at full resolution with 2× MSAA about 100 MB, the MSAA buffers 42 MB of it, since Godot allocates them in memory although it never writes them); the app at most 1 GiB in all; the APK at most 50 MB unless study 7 changes the rule.

### 6.3 Textures

Make them on the phone at first start from the kit's recipes and keep them (A6.1, `PLT-03`), uncompressed, as Godot advises for pixel art. Where a texture must come from an approved picture rather than a recipe, ship it as Basis UASTC if the owner sees no loss, or losslessly if study 7 lifts the 50 MB rule. Never download.

### 6.4 What to measure first, with pass lines stated now (`RES-09`)

All runs on the owner's phone, unplugged, in flight mode, at a brightness the owner fixes, after the phone has cooled, on a scripted camera path (a pan, a full turn in eased steps, a pinch through the stop). Each runs once at a 120-frame cap for chip time and once at 60 for frames, power and heat. Checks: `PLT-04`, `VIS-14`, `PRN-11`. Phone time about 40 minutes, plus 20 for the heat run.

**First, in this order: calibration scenes.** Each must produce its number; the decision that follows is stated now.

1. **C4 Fixed cost:** the world hidden (sky only), the interface on and off, MSAA off and 2×. Expected 0.9–1.7 ms. Over 2.0 ms: look at the interface pass and Godot's stores first.
2. **C1 The material at full resolution:** a flat field of ground filling the screen with the full material and pre-blurred soft shadows; MSAA off, 2× and 4×; 3D at 1.0, 0.75 and 0.5 scale. Expected 1.7–4.0 ms at 1.0 with 2×.
   - At most 2.5 ms: full resolution everywhere stays the plan.
   - 2.5–4.0 ms: build 2 × 2 shading on ground and plants (engine patch), and the owner flips it on and off.
   - Over 4.0 ms: simplify the material first.
3. **C3 Triangles:** the same field of solid rocks at 100, 200, 400 and 800 thousand triangles a pass, MSAA 2×, the shadow pass on and off. Expected 2.5–10 ns a main-pass triangle.
   - At most 4 ns: the triangle line is 0.6 million triangle-passes, and the leaf ways that add triangles stay open.
   - 4–6 ns: the line stays 0.4 million.
   - Over 6 ns: the line is 0.3 million, and leaves stay cards or get the pre-pass.
4. **C2 Leaves at the liked density:** S1's layout with its leaves as plain cut-out cards, tight cards and opaque cores with fringes (and the pre-pass once built), with everything else equal. Expected leaves' cost 0.6–6.0 ms. The cheapest way the owner cannot tell from plain cards becomes the default. If the cheapest still costs over 1.5 ms for the leaves, build the pre-pass.
5. **C5 Fires:** 1, 3 and 5 fires at the closest zoom by the design's walk, the walk at half resolution, and a map per fire. Expected 2.5–24, 0.8–6.5 and 0.2–1.7 ms. The map per fire becomes the way unless the owner sees a difference.
6. **C6 Figures:** 30, 100 and 300 detailed figures by Godot's skeletons and by our batched way. The main thread's ms a figure sets how many Godot skeletons may be in view, keeping figures within 1.0 ms.

**Then the stress scenes S1 to S9 of 5.2,** S1 and S3 first, with these pass lines:
- **Frames:** at least 97% on time, none over 66.7 ms (M1's frame meter).
- **Graphics chip:** at most 8.0 ms mean and 9.5 ms at the 95th percentile in each scene; S9 at most 6.0 ms. Each part over its allowance (6.2) is named in the report. That is not a failure in itself.
- **Main thread:** at most 8 ms mean and 12 ms at the 99th percentile.
- **Counts:** within C3's triangle line and 300 draws.
- **Memory:** the app at most 1 GiB.
- **First start after an install:** textures made and kept within 10 s; a warm open within 3 s; no shader pipeline compiled while drawing.

**Last, H1 heat:** 20 minutes of the costliest S scene that passed, at 60 frames. The 10-second forecast never reaches the phone's light threshold less 0.05; the thermal status stays at none; the battery ends at or below 40 °C; the whole phone averages at most 4.0 W; and every minute has at least 97% of frames on time.

**If a scene misses 8 ms,** in this order: any saving of 5.3 not yet used; the leaf pre-pass; 2 × 2 shading on ground and plants; 3D at 0.75 in that scene only. The owner judges each lever that may show on the phone before it stays.

### 6.5 Measuring without a computer, and the heat guard's fix

**One tap, one code,** as M1's Bench, adding:
1. **Time:** every viewport's GPU time and Godot's per-pass timestamps; a run with the world hidden; each part switched off in turn (drawing switches only, which never touch the world, `WLD-13`).
2. **How busy the chip is at 60:** Android's GPU headroom, where offered.
3. **Counts:** triangles, draws and video memory from Godot's monitors.
4. **Power:** battery current times voltage every 2 s, the charge counter over each run, and the power rails if offered; runs of at least 4 minutes, so the rails' noise stays under about 0.04 W.
5. **Heat:** headroom now and in 10 s, the phone's thresholds, every change of thermal status, the battery's temperature at start and end, minutes to the light threshold.
6. **What the owner sees:** each lever that may show has an on-off switch, so the owner can flip it and judge (study 6); the Pixel's Game Dashboard frame counter is a check of its own.
7. **Last resort:** the phone's System Tracing app; Android GPU Inspector and Imagination's PVRTune need a computer.

**The heat guard's fix** (A3.9, `view/src/heat.cpp`, `heat.toml`):
1. **Use the phone's own levels.** Read the thresholds (API 35) and register the headroom listener (API 36). Cut the simulation's share when the 10-second forecast reaches the *light* threshold less a margin (0.05 to start), not 0.85 of *severe*.
2. **A missing reading is no reading:** `NaN` means the phone was asked too often or does not offer the reading, never that it is cool.
3. **Know what it can cool.** Slowing time cools the phone at far zooms at top speed, where the simulation is the load. At close zooms the picture is the load, so the graphics budget must pass the 20-minute run; the guard cannot rescue it.
4. **Record** the thresholds, every status change and the minutes to light throttling, in the bench code and the self-check.

### 6.6 Proposals for the owner

1. *Proposal (A18.1, A4.3, A17):* replace the single 8 ms aim with the budgets of 6.2, and add the calibration and stress scenes of 6.4, with their pass lines, to M2's first build.
2. *Proposal (A4.1, A4.3):* draw the world at full resolution with MSAA 2×, keeping the merged pass. Corner darkening is baked and read from maps; fire shadows come from a map per fire; the sun map is 2,048 texels with big casters only and pre-blurred softness; the shore line comes from the height map; the mirror is drawn at half resolution with a reduced set; fire glow is sprites. Studies 2 and 3 choose the techniques; these are their costs.
3. *Proposal (A2.2, builds):* allow our own build of Godot 4.7.2's export templates for two patches that do not show and one that may: a leaf pre-pass with an equal depth test; buffers that need never reach memory (Godot's own TODO); and a shading rate for each material. The last is used only where needed, as the owner said. Study 7 says what the build costs.
4. *Proposal (A3.9, `heat.toml`):* the heat guard reads the phone's thresholds and acts near the light level.
5. *Proposal (`PRN-11`, `PLT-04`, the owner's choice):* if the 20-minute run shows the picture alone heats the phone, either **(a)** the graphics line comes down for good, so the picture never changes under load (my recommendation), or **(b)** the owner allows one planned, logged step under heat that keeps 60 frames, such as distant fires casting no shadows.
6. *Proposal (`PLT-04`, How it works; from round 1):* add a 20-minute run of the busiest close scene, for heat, to the benchmark worlds.
7. *Proposal for lifted `PRE-22`, the cost side only.* Proposed wording: *The world is drawn at the screen's full resolution, its textures pixel art, one texture pixel about 2 by 2 screen pixels at the closest zoom; shading at a lower resolution only where the phone's measured cost requires it, with your OK for each.* Pixels that do not crawl or shimmer stay the goal; study 1 proposes the full wording.
8. *Proposal for lifted A5.3.* Proposed wording: *Textures at about 64 texture pixels a metre at the closest zoom, with a version for each zoom band and mipmaps.* The cost: texture memory only, at most about 300 MB.
9. *Proposal for lifted `MIL-09` and `PRE-46` ("made by code"), the cost side only:* textures are made on the phone at first start from the kit's recipes and kept; approved pictures that must ship go as Basis UASTC or losslessly within the APK rule; nothing is downloaded (`PLT-03`). Study 5 proposes the wording.
10. *Proposal for lifted `PRE-27`, the cost side only:* detailed figures are affordable on the graphics chip if their detail falls with their size; more than a few dozen in view need our own batched animation.
11. *Proposal (`IMPLEMENTATION.md`, M2):* M2's cost risks become full-resolution shading, cut-out leaves and fire shadows in place of the outline pass. The first alpha builds the measuring and the calibration scenes on stand-in content, so the owner sees each lever's cost, and judges each lever that may show, on the phone before any art is made.

## 7. Questions only the phone can answer

1. **What the full material costs a pixel at full resolution** (C1): the second biggest unknown, and the one 2 × 2 shading would cut.
2. **What a triangle costs in each pass** (C3): the biggest unknown; it decides how leaves are drawn.
3. **What a cut-out leaf layer costs against a solid one** (C2).
4. **What the fixed part costs:** the interface pass, the copy and the stores (C4).
5. **Whether 2 × 2 shading on ground and plants shows** to the owner's eye, once patched in (C1, S1).
6. **Whether the phone offers GPU headroom and power rails** to apps; list them in the self-check.
7. **How many watts a millisecond costs** at 60 frames.
8. **The phone's own throttling thresholds,** and how long the busiest scene takes to reach light throttling (H1).
9. **The main thread's cost a draw and a figure** for each animation way (C6).
10. **How long the first start takes** to make and keep the textures.
11. **Whether the screen's 120 Hz costs power** against 60 Hz (M1's amber line).
12. **Whether the driver handles every technique:** the cloud's driver crashed on P2's smoke near 15°; every stress scene runs on the phone first.

## 8. Risks

| Risk | How to retire it early |
|---|---|
| The material costs the high end at full resolution | C1 first; then 2 × 2 shading on ground and plants, or a simpler material |
| Triangles are dear, so no leaf way beats plain cards | C3 and C2 in the first build; the leaf pre-pass |
| Our own build of Godot proves hard to keep | study 7 costs it; stock fallback E (3D at 0.75 where needed) |
| The phone heats before 20 minutes at 8 ms | H1 in the first build; lower the line for good (6.6, 5) |
| The fire map per fire looks different from the walk | C5 on the phone, the owner's eye; the walk at half resolution as the second way |
| Godot's skeletons swamp the main thread in the village | C6; our batched animation from the start for crowds |
| Textures too big to ship and too slow to make | time the first start; UASTC if the owner sees no loss |
| Android changes its thresholds while playing | the headroom listener (API 36) |
| My model is wrong in shape, not only in range | every number is replaced by C1 to C6 and S1 to S9 before any art is made |

## 9. For other studies

- **Study 1 (look):** the texture-pixel size costs no time, only memory. Of the owner's "only if needed" levers, 2 × 2 shading keeps shape edges sharp, unlike the smooth enlargement shown in q25, and may not show where texture pixels are 2 × 2 anyway. Snow at noon costs more screen power than a night camp.
- **Study 2 (drawing):** the costs of 5.1 to 5.7. Godot 4.7.2's Mobile renderer has no main-view depth pre-pass and no equal depth test, so a leaf pre-pass needs a patch. Its VRS works only with a shading-rate texture, which this chip lacks; the chip does support a rate for each draw (4.3). MSAA 2× is "virtually free" by Imagination's account, and with it Godot stops storing depth. Blended edges with MSAA run in software. Fire shadows from a small map per fire are the single largest saving at the closest zoom by night. Rain streaks as thin particles cost about a tenth of full-screen sheets.
- **Study 3 (engine):** three engine patches (4.2, 6.6 proposal 3), and our batched animation for more than a few dozen figures. Measuring hooks: viewport and per-pass times, Godot's counters, Android's GPU headroom, thermal thresholds and the headroom listener. Detail by size on screen in every family.
- **Study 5 (content):** figure triangle budgets (5.6). Tiles in the chosen style are about 120–190 KB each losslessly, so recipes made on the phone beat shipped pictures for the APK.
- **Study 6 (loop):** every lever that may show needs an on-off switch on the phone, so the owner judges it there. The stress scenes give the owner each scene's cost beside its look.
- **Study 7 (pipeline):** the APK is 27.6 MiB against the 50 MB rule; three or four hundred lossless tiles would need 35–70 MB more. Our own export templates, if the engine patches are taken. Basis UASTC encoding happens in the cloud.

## 10. Pictures from GPT

I asked for four pictures (4 of my 16). Each repaints the owner's liked camp from above (`R/owner/liked-camp-from-above.png`) and changes one thing about the plants. All four were put to the owner as question 6 (`R/owner/ask/q06-plants.jpg`), with my old cost estimates. The owner answered "As I liked": none of the four is accepted, and savings must come from ways that do not show. I asked for no more: the remaining questions (whether a leaf pre-pass, 2 × 2 shading or a fire map shows) can only be judged in motion on the phone.

Measured by code (`R/work2/4/compare.py`; rough colour classes, so trends only), against a control: study 2's outline repaint, which changed nothing about plants (strong green 7.6% against the original's 6.7%, fine detail on land −8%):

| Picture | The question | What it showed |
|---|---|---|
| `R/work2/4/gpt/density-half.png` | Does the feeling hold with about half the plants? | Strong green −39%, fine detail −24%. A tidy park; the lushness goes. |
| `R/work2/4/gpt/closed-clumps.png` | Does it hold if every plant is a closed clump? | Strong green +70%, detail −19%. Cosy and tidy, like a farm game; the wild, airy look goes. |
| `R/work2/4/gpt/dense-edges.png` | Does the density live in the edges? | Strong green −27%, detail −17%. A lived-in camp with wild edges. |
| `R/work2/4/gpt/closed-ground.png` | Can only the low cover be closed, with trees and reeds airy? | Strong green +61%, detail −15%, the most fine detail kept; the grass reads a little like moss cushions. |

## 11. Sources

**Repository** (read 6 October 2026): `PROJECT.md`; `ARCHITECTURE.md` (A3.9, A4, A5, A6, A8, A18); `IMPLEMENTATION.md` (M2); `LESSONS.md`; `dist/M1-REPORT.md`; `dist/kindling.apk` (28,985,099 bytes); `data/base/tuning/heat.toml`; `tools/verify-apk.sh`; round 1's notes in `R/round1/notes/`; the owner's answers in `R/owner/ask/answers.md`.
- `PROJECT.md`: "at least 97% of frames on time while zooming, panning and turning, at every zoom, and none more than 50 ms late" / "an hour's play uses about 25–30% of the battery, and the phone never gets uncomfortably hot" / "a camp of about 30 and a village of about 300 at close camp zoom, a camp in thick forest at camp zoom with the camera turning" (`PLT-04`); "leaving the rest for the picture, sound and the writer" / "about 3 W" (`PLT-01`); "Everything the game needs, the writer AI included (`PRE-37`), is on the phone; nothing in play makes a network call." (`PLT-03`); "When the phone can't keep up, the world doesn't cut detail: time simply runs more slowly." (`PRN-11`); "Beautiful, smooth and absorbing in your hand." (`VIS-14`); "its pass rule in exact numbers" (`RES-09`); "the battery over about 40 °C in long sessions" (`RSK-24`)
- `IMPLEMENTATION.md` (M2): "The cost of the outline pass and of mirrored water at the phone's resolution."
- `ARCHITECTURE.md`: "the graphics chip under about 8 ms in the busiest scene, so heat leaves room" (A18.1); "as the forecast nears the first throttling level" (A3.9); "from the depth texture" (A4.1); `heat.toml`: "where 100% is the first level at which it slows itself"; `tools/verify-apk.sh`: "over the 50 MB a committed file may have"

**Imagination Technologies** (saved in `R/work/4/pages/` and `R/work2/4/pages/`; checked 6 October 2026)
- [Vertex Processing (Tiler)](https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/vertex-processing.html): "The tile list and the transformed vertex data are both stored in an intermediate store called the Parameter Buffer (PB). This store resides in system memory"
- [Hidden Surface Removal Efficiency](https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/hidden-surface-removal-efficiency.html): "With PowerVR TBDR, Hidden Surface Removal (HSR) will completely remove overdraw regardless of draw call submission order."
- [Z Pre-pass and PowerVR](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/z-pre-pass-and-powervr.html): "On PowerVR hardware, there is no performance benefit to rendering a low-poly geometry Z pre-pass to save fragment processing later."
- [Do Not Use Discard](https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/rules/do-not-use-discard.html): "alpha-tested primitives cannot write data to the depth buffer until the fragment shader has executed and fragment visibility is known." / "These deferred depth writes can impact performance, as subsequent primitives cannot be processed until the depth buffers are updated with the alpha tested primitive’s values."
- [Efficient Sprite Rendering](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/efficient-sprite-rendering.html): "One method to minimise the impact of several layers of blended sprites is to increase the geometry complexity of the sprites, to reduce the amount of wasted transparent fragments." / "PowerVR hardware has excellent vertex processing capabilities and is designed to handle large amounts of geometry data"
- [MSAA Performance](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/msaa-performance.html): "2x MSAA is virtually free on most PowerVR graphics cores (Rogue and Volcanic onwards), while 4x MSAA+ will noticeably impact performance." / "In some cases, there may be no need for anti-aliasing to be used at all, for example when the target device’s display has high pixels per-inch (PPI)." / "On edge blend is a costly operation, as the blending is performed for each sample by a shader in software."
- [Performing MSAA on PowerVR using Vulkan](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/performing-msaa-on-powervr-using-vulkan.html): "the application should use a lazily-allocated MSAA frame buffer attachment"
- [Triangle Size](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/triangle-size.html): "especially dipping below 32 pixels per primitive"
- [Do Perform Clear](https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/rules/do-perform-clear.html): "System memory accesses use more bandwidth and power than any other graphics operation."
- [Preferred Shadowing Solution](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/preferred-shadowing-solution.html): "Techniques that require results to be written to off-chip memory, such as shadow mapping, will usually perform worse than techniques that can be computed entirely in on-chip memory."
- [Using Render Passes and Sub-passes](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/using-render-passes-and-sub-passes.html): "changing render targets is a fundamentally expensive operation"
- [Do Use Mipmapping](https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/rules/do-use-mipmapping.html): "increases graphics rendering performance by massively improving texture cache efficiency"
- [Texture Sampling](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/texture-sampling.html): "dependent texture reads should be avoided wherever possible for good performance."
- [Optimal Particle Rendering on PowerVR](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/optimal-particle-rendering-on-powervr.html): "lots of alpha-blended particles can cause a massive overdraw issue"

**Godot 4.7.2** (source at tag `4.7.2-stable`, `https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/<path>`, read in a checkout of that tag; documentation at docs.godotengine.org/en/4.7)
- `servers/rendering/renderer_rd/forward_mobile/render_forward_mobile.cpp`: "// - not reading from SCREEN_TEXTURE/DEPTH_TEXTURE" / "// can't do blit subpass because we're scaling" / `p_render_data->scene_data->opaque_prepass_threshold = 0.0;`; `render_forward_mobile.h`: `// PASS_MODE_DEPTH,` (commented out) / `MAX_RDL_CULL = 8, // maximum number of reflection probes, decals or lights we can cull per geometry instance`
- `servers/rendering/renderer_rd/renderer_scene_render_rd.cpp`: "// We don't support glow or auto exposure here, if they are needed, don't use subpasses!"
- `servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.cpp`: "// TODO: Detect when it is safe to use RD::TEXTURE_USAGE_TRANSIENT_BIT for RB_TEX_DEPTH, RB_TEX_COLOR_MSAA and/or RB_TEX_DEPTH_MSAA."; `render_scene_buffers_rd.h`: `bool p_discardable = false`
- `servers/rendering/rendering_device.cpp` (`_vrs_detect_method`): `if (fsr_capabilities.attachment_supported) {`
- `drivers/vulkan/rendering_device_driver_vulkan.cpp`: "// We don't use pipeline/primitive FSR so this really doesn't matter."
- `servers/rendering/renderer_rd/forward_mobile/scene_shader_forward_mobile.cpp`: `depth_stencil_state.depth_compare_operator = RD::COMPARE_OP_GREATER_OR_EQUAL;`
- `doc/classes/BaseMaterial3D.xml`: "The material will use the texture's alpha value for transparency, but will discard fragments with an alpha of less than 0.99 during the depth prepass and fragments with an alpha less than 0.1 during the shadow pass."
- `doc/classes/MultiMesh.xml`: "A MultiMesh is a single object, therefore the same maximum lights per object restriction applies."
- `modules/astcenc/SCsub`: "# Build the encoder only for editor builds"; `modules/basis_universal/image_compress_basisu.cpp`: "basisu_format = basist::transcoder_texture_format::cTFASTC_4x4_RGBA;"
- `platform/android/export/export_plugin.cpp`: `encode_uint32(app_category == APP_CATEGORY_GAME, &p_manifest.write[iofs + 16]);` (the app is marked as a game by default)
- [Variable rate shading](https://docs.godotengine.org/en/4.7/tutorials/3d/variable_rate_shading.html): "Variable rate shading (VRS) is a method of decreasing this shading cost by reducing the resolution of per-pixel shading (also called fragment shading), while keeping the original resolution for rendering geometry." / "Both Forward+ and Mobile renderers support variable rate shading." / "On unsupported hardware, there is no visual difference when variable rate shading is enabled."
- [GPU optimization](https://docs.godotengine.org/en/4.7/tutorials/performance/gpu_optimization.html): "Even in 3D, "pixel art" textures should have VRAM compression disabled as it will negatively affect their appearance, without improving performance significantly due to their low resolution."
- [Optimizing 3D performance](https://docs.godotengine.org/en/4.7/tutorials/performance/optimizing_3d_performance.html): "Animation and vertex animation such as skinning and morphing can be very expensive on some platforms."
- [Animating thousands of fish with MultiMeshInstance3D](https://docs.godotengine.org/en/4.7/tutorials/performance/vertex_animation/animating_thousands_of_fish.html) (checked in the docs' 4.7 source): "bones are animated on the CPU and so you end having to calculate thousands of operations every frame and it becomes impossible to have thousands of objects."

**The phone**
- [Vulkan Hardware Database, report 51167](https://vulkan.gpuinfo.org/displayreport.php?id=51167) (Google Pixel 11 Pro XL; data, not quotes): deviceName "PowerVR C-Series CXTP-48-1536 MC1"; driverVersion 1.662.3024; driverInfo "25.3@6908880"; apiVersion 1.4.317; Android 17.0; `pipelineFragmentShadingRate` true, `primitiveFragmentShadingRate` false, `attachmentFragmentShadingRate` false; `maxFragmentSize` [4,4]; `min/maxFragmentShadingRateAttachmentTexelSize` [0,0]; a memory type with LAZILY_ALLOCATED_BIT; `shaderFloat16` true. The same database's [coverage list](https://vulkan.gpuinfo.org/listdevicescoverage.php?extension=VK_KHR_fragment_shading_rate&platform=android) shows `VK_KHR_fragment_shading_rate` on the Pixel 11, 11 Pro and 11 Pro XL (CXTP-48-1536) and the Pixel 10 family (DXT-48-1536); no PowerVR device lists `VK_EXT_fragment_density_map`.

**Android**
- [thermal.h](https://android.googlesource.com/platform/frameworks/native/+/refs/heads/main/include/android/thermal.h): "Note that this only attempts to track the headroom of slow-moving sensors, such as the skin temperature sensor." / "A value of 1.0 indicates that the device is (or will be) throttled at {@link #ATHERMAL_STATUS_SEVERE}." / "Starting in Android 16, this polling API may return different results when called depending on the device."
- [system_health.h](https://android.googlesource.com/platform/frameworks/native/+/refs/heads/main/include/android/system_health.h): "Provides an estimate of available GPU capacity headroom of the device." / "where 0 indicates no more gpu resources can be granted"
- [PowerStatsService.java](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/powerstats/PowerStatsService.java): "private static final long MAX_POWER_MONITOR_AGE_MILLIS = 30_000;" / "private static final long MAX_RANDOM_NOISE_UWS = 10_000_000;"
- [Game Dashboard](https://developer.android.com/games/gamedashboard/aboutdashboard): lists a live frame counter among its tools, on Pixels from Android 12 (paraphrased).

**Reviews, measurements and developers' posts**
- [Android Authority: Tensor G6 tests](https://www.androidauthority.com/tensor-g6-benchmarks-tests-3699714/) (20 August 2026): "PowerVR CXTP-48-1536 (1.29GHz)"
- [Android Authority: Pixel 11 gaming](https://www.androidauthority.com/pixel-11-gaming-test-3708496/) (10 September 2026): "the phone crept up to over 40°C after five minutes" / "Asphalt Legends and Genshin Impact sadly can’t lock a 60fps figure in"
- [Android Authority: Pixel 10 Pro XL gaming](https://www.androidauthority.com/pixel-10-pro-xl-gaming-benchmarks-3609172/) (26 October 2025): "an unsustainable 7.2W average during gameplay that will drain the battery quickly"
- [Notebookcheck, from Geekerwan](https://notebookcheck.net/Pixel-11-s-Tensor-G6-and-Pixel-10-s-Tensor-G5-still-fall-behind-in-performance-per-watt-testing-shows.1381970.0.html) (29 August 2026, secondary): "The Tensor G6's Steel Nomad Light result peaks at around 1,200 points at roughly 6W"
- [GSMArena: Pixel 11 Pro XL review](https://www.gsmarena.com/google_pixel_11_pro_xl-review-2994p3.php): "The Pixel 11 Pro XL is powered by a 5,115mAh battery"
- [Godot forum: crowds of animated rigged characters](https://forum.godotengine.org/t/performance-of-crowds-of-animated-rigged-characters-in-3d/130409) (January 2026, Godot 4.2, desktop): "it runs at 36 fps with almost 300 units, so 50-100 would be safe"
- [Basis Universal](https://github.com/BinomialLLC/basis_universal): "A roughly .3-3bpp low to medium quality supercompressed mode" / "An 8 bits/pixel LDR high quality mode."

**Could not verify:** the Pixel 11's own throttling thresholds; whether it offers GPU headroom or a GPU power rail to apps; its memory bandwidth; the watts a millisecond of graphics costs (my rule, 6 W × ms ÷ 16.7 × 0.55 to 1.0, is an estimate); the 100–300 bytes of memory traffic a triangle; the cost of a figure on Godot's skeletons on this phone; Basis UASTC's size on pixel-art tiles; the first-start making time; whether 2 × 2 shading shows to the owner. Every millisecond in sections 5 and 6 is an estimate until the calibration and stress scenes run. A Godot issue on skeletons under OpenGL and Vulkan, cited in my earlier draft, could not be reached from this session and is dropped.
