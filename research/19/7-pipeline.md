# Study 7, round 2: the build pipeline for direction B

> Study 7 of [research 19](../19-graphics.md), written on 6 October 2026 and kept as written, its quotes checked ([the check](quote-check.md)).
> `R` was the research session's working folder and is not kept, except your pictures and answers, now in `art/targets/` and `art/reviews/2026-10-06-graphics/`.

M2 research, round 2, study 7 of 7, 6 October 2026 (resumed after the restart). Research only: nothing here is decided until the owner says so (`PRC-07`).

Godot facts are checked in Godot's own source at the tag `4.7.2-stable` (our version) or in the 4.7 documentation. Measurements are small Python scripts in `work2/7/`, named where used. GPT drew 5 pictures for this study (section 10).

**Words used here:**
- A **texture** is a picture wrapped onto a 3D surface. A **texel** is one pixel of a texture.
- **Albedo:** a texture holding only a surface's own colour, with no light or shadow painted in. The engine adds the light.
- **Tileable:** a texture whose right edge continues into its left and its bottom into its top, so copies laid side by side show no seam.
- **Mip levels (mipmaps):** a texture's smaller copies (half, quarter and so on), which the graphics chip uses when the texture is seen small.
- **Zoom band:** a range of zoom in which one level of a texture shows. Band 0 is the closest zoom; each next band has half the texels a metre.
- **Designed level:** a level drawn as pixel art for its own band, instead of averaged from the level above.
- **Re-gridding:** bringing a picture's blocks onto an exact texel grid, one block becoming one texel.
- **Lossless:** stored without any loss (PNG or WebP). **ASTC** is the compressed texture format the phone's chip reads directly; **KTX2** is a standard file that holds such textures.
- **LUT (colour look-up table):** a small table that turns every colour of the finished picture into a graded colour. It is how a "colour grade" is applied.
- **Source record:** a small text file saying where a texture came from: its picture, prompt, the owner's approval and how it was prepared.
- **C2PA:** an open standard for signed "content credentials" inside a picture file.
- **Release file:** a file attached to a GitHub release, outside the repository's history. **Git LFS:** GitHub's large-file storage, with a monthly quota.

## 1. The question, and the answer in brief

**The question:** what build pipeline carries rich content (textures made by code or from approved AI pictures, models, shaders, colour grades and effects) from its sources to the phone, quickly and safely, now that the strict rules are lifted?

**The answer in brief,** for direction B (a smooth, sharp 3D world at full resolution, wearing pixel-art textures):
- **Round 1's spine stays:** one source per element (catalogue text, C++ generators, shader files, or an approved picture with its text record), a warm-up scene and the look lab. Direction B changes the texture path.
- **Every texture becomes pixel art on an exact grid by code.** No source has one, not even the liked pictures, whose "pixels" are painted strokes. Study 5's accepted swatches lose only 2 to 8% when re-gridded.
- **Each zoom band gets a designed level** (answer 31). Averaging is the speckle the owner saw: it cuts the ground's accents from 16 to 12. GPT redrew the close-camp level from the approved closest one in 42 seconds, keeping layout and accents; code matched its colours.
- **No compression at first.** Godot's own documentation says pixel art should not be compressed, even in 3D. Textures ship lossless (about 10 to 140 KiB each) with our own levels, which Godot keeps.
- **Rock: each world's layers laid by code work; GPT's fine layer-free surface does not.** Its texel contrast is a quarter of the liked cliff's. Study 5's chunky swatch comes close to the liked cliff. Pictures for the owner are ready (6.12).
- **The texel density is one number for the owner's eye:** 32 texels a metre (4 screen pixels up close, like the liked picture's own blocks) or 64 (2 pixels). I'd recommend 32: it also needs a quarter of the memory.
- **Grades are small text data, not the feeling** (answer 24), with a true midday.
- **APKs and GPT's originals become release files;** committed APKs already fill 1,023 MB of a 1.06 GiB history. Nothing is downloaded on the phone (`PLT-03`). GPT stays outside the build, every run kept; at a limit the builder waits and never buys credits.

## 2. What `PROJECT.md` asks

- **Not lifted:**
  - nothing is faked (`PRN-10`): cliffs show "the rock layers where they stand", with "layers of different thicknesses, cracks and fissures, lichen and water stains where the face is wet (`WLD-16`), soot above lived-in caves (`MAT-18`), grass hanging over the top and scree at the foot" (`PRE-23`). So a texture cannot carry fixed layers;
  - each cell has "its surface rock and up to two layers below" among about 12 rock kinds (`WLD-09`), so a cliff shows up to three rocks with their own beds;
  - overhangs, caves and shelters are "lit inside only by openings and fires" (`PRE-24`);
  - looking changes nothing (`WLD-13`); Stone Age truth (`PRE-42`); "Adding a new animal later needs one catalogue entry, with its model, sounds and tests" (`PRN-14`), so a picture-made texture must come in as one more entry; the kit stays one countable set of forms (`PRE-46`).
- **Lifted for this research:** `MIL-09`'s "the model kit and its textures made by code", and the one texture density of `ARCHITECTURE.md` A5.3.
- **The phone:** "nothing in play makes a network call" (`PLT-03`); each alpha is a file the owner installs over the last (`PLT-06`); a world opens in about 3 seconds, within about 8 GiB, at 60 frames a second (`PLT-04`, `PLT-01`; the 60 is `ARCHITECTURE.md` A18.1's frame budget); each update says if it is small or big (`PLT-09`).
- **How we work:** "no running costs beyond the AI sessions" (`PRC-01`); every check within about 20 minutes before work joins (`PRC-10`); each alpha reaches the phone (`PRC-11`); the look is reviewed on the phone (`PRE-31`); and the credits already list every recording with its source (`PRE-40`, `SND-06`), a precedent for recording where every picture came from.

## 3. Where we stand

### 3.1 What round 1 found

- **My round-1 note:** everything is text with one source; the look is made on the phone at load; shaders are files with a warm-up scene and Godot's pipeline counters; and Godot's game build **cannot compress textures** (the ASTC, ETC2 and Basis encoders exist only in editor builds).
- **Study 1** measured the liked pictures: golden brights, muted yellowish greens, a quarter genuinely dark, strong colour only in accents, no flat ground, quiet texture, a person about 180 picture pixels tall.
- **Study 2:** the liked camp keeps most of its look on a true grid with 64 colours.
- **Study 4:** rich textures cost memory more than time.
- **Study 5:** the kit made by code, plus pictures the owner approves, processed by code. **Study 6:** machines measure, the AI describes, the owner decides; the target card; the taste log.

### 3.2 What this study found before the owner's answers

- **Three ways from an approved picture to a texture** (on a crop of the liked meadow): code only (keeps colours, but ghosts and repeats); GPT repainting the crop as a seamless texture (flat light, close colours, but fine detail and repeated flowers); GPT drawing a meadow at the game's scale (flat, no strong repeat).
- **GPT does not calibrate colour:** its "neutral daylight" rock was 0.12 lighter than the approved sunlit rock.
- **Godot's mipmaps average the stored bytes** with no sRGB conversion (`p_out = static_cast<uint8_t>((p_a + p_b + p_c + p_d + 2) >> 2);`), which darkens fine light-and-dark detail in every smaller level.
- **Every GPT picture carries a signed C2PA record** (a `caBX` chunk signed by "OpenAI OpCo, LLC", naming "ChatGPT" and "gpt-image"), but not the prompt.
- **The repository's history is already too big** (section 3.5).

### 3.3 The owner's answers that steer the pipeline

All 32 are in `owner/ask/answers.md`. These bear on the pipeline:

| Answer | What the owner said | What it means here |
|---|---|---|
| 1, 2 | Direction B; "2x2, the finest" (judged on study 1's close camp, about 16 to 19 m across) | Textures are pixel art; the texel density is one number to set (6.4) |
| 5 | The closest zoom as in the liked picture, about 8 m across | Band 0 is this zoom |
| 3, 14, 19, 20 | Detailed people and animals; age, build and clothes tell people apart | Figures' textures are materials with bands too |
| 7 | "As I liked": no outlines, smooth light, the fine grain, darkening in corners | Light is the engine's; the grain is the texture's |
| 9 | True midday | The grade set includes an honest noon |
| 13 | "Have multiple types that they can choose from" | More kit data and more materials (hide, bark, brush) |
| 15 | Surfaces A (code-made from the liked meadow), C (GPT swatches from words) and D (GPT swatches redrawn from the liked picture) feel right; B (GPT's seamless repaint) does not | Three source routes accepted; the fine repaint is out |
| 21 | Small plants lose their charm at game size | Plant cards need designed levels too (study 5's problem) |
| 23 | Dusk targets by relighting an approved picture | Targets are study 6's; the pipeline stores them |
| 24 | "colour alone doesn't bring the feeling" | A grade is tuning, not the look's source |
| 25 | Half resolution only where needed | Texture lookups stay at full resolution (study 6) |
| 31 | Neither ground works: both turn to speckle at the close camp | A designed level for each zoom band |
| 32 | Can't judge the layer-free rock until the layers are on | Made by code, section 5.6 and 6.12 |

### 3.4 What the design says now

- **Data** reaches the phone as TOML through `tools/gamedata.py`, listed with SHA-256 digests in `build.toml`; each source has a **look** digest, and a look-only change is a small update (A3.6, `PLT-09`).
- **Builds:** "Android export needs ETC2 and ASTC texture imports on" (A2.2).
- **Delivery:** the APK is "committed to `dist/` on the work branch and linked from the note" (A2.3); `tools/verify-apk.sh` fails any APK "over the 50 MB a committed file may have".
- **Textures:** "Every texture is drawn at 16 texture pixels a metre, about one to one art pixel at the close camp zoom" (A5.3).
- **Checks:** golden pictures on the cloud's software driver (A4.4); the whole check takes "5 to 15 minutes of the 4 cores" (`dist/M1-REPORT.md`).

### 3.5 What is weak, wrong or missing for direction B

1. **No path for pictures:** where they live, how they are approved, how a texture is traced back, who checks their Stone Age truth.
2. **No exact grid:** pixel textures need one, and no source has it (5.2).
3. **No designed levels:** A5.3 has one density and Godot's averaged mipmaps, which the owner called speckle.
4. **The import defaults are wrong for pixel art:** Godot's "Detect 3D" switches any texture used in 3D to "VRAM Compressed", and its own documentation says pixel art should not be compressed (section 4).
5. **The checks assume a palette** (off-palette counts), which direction B does not have.
6. **The history is too big:** 1.06 GiB of objects, of which 1,023 MB are 47 distinct APKs from 49 commits between 2 and 6 October (measured with `git`). GitHub: "We recommend repositories remain small, ideally less than 1 GB, and less than 5 GB is strongly recommended." Every fresh cloud session clones it.
7. **GPT runs leave only scratch files,** and the plan's limits are nowhere in our process.
8. **The owner's loop is long:** checks, a build, a download of about 29 MB and an install for every look change.

## 4. What others do

- **Godot itself, on pixel-art textures** (4.7 documentation): "Even in 3D, "pixel art" textures should have VRAM compression disabled as it will negatively affect their appearance, without improving performance significantly due to their low resolution." Lossless storage is "the recommended setting for pixel art". Its memory table: a 256 × 256 RGBA8 texture with mipmaps takes 341 KiB lossless and 85 KiB VRAM-compressed.
- **Designed levels are an old, supported practice.** Godot keeps the levels inside DDS files: "If mipmaps are present in the texture, they will be loaded directly. This can be used to achieve effects using custom mipmaps." Microsoft's content pipeline guide (2013, old) names the trade: "When you do not need to specify the image content of each MIP level manually—as you might do to achieve certain effects—generating mipmaps at build time ensures that mipmap contents never become out-of-sync". So designed levels are allowed, and keeping them in step with their base is the pipeline's job.
- **Shipping files untouched:** Godot's import can be set to "Keep File (exported as is)", and "files with this resource type will be preserved as is during project export".
- **Mipmaps offline, in linear light:** Imagination, maker of the phone's chip: "Ideally mipmaps should be created offline using a tool like PVRTexTool." NVIDIA's *GPU Gems 3* (2007, old) on averaging stored, gamma-encoded values: "If you are in a nonlinear color space with a gamma, say of 2.0, then that coarse-level texel with a value of 0.5 will be displayed at only 25 percent of the brightness."
- **Compression is for memory and bandwidth.** Imagination: "Reduce the memory footprint and bandwidth cost of the texture assets." Arm: ASTC blocks "range from 4x4 texels up to 12x12 texels, which all compress into 128-bit output blocks". Khronos's `ktx create` writes KTX2 and makes mipmaps on request ("--generate-mipmap"). Basis Universal: "UASTC LDR is a common subset of both BC7 and ASTC".
- **Removing painted light is a standard step.** Adobe's Substance 3D Sampler (April 2026): "The Delighter allows you to remove lighting information from the base color channel."
- **Tiling without ghosts:** Heitz and Neyret (HPG 2018) on blending patches: it "usually produces visual artifacts such as ghosting, softened discontinuities and reduced contrast, or introduces new colors not present in the input".
- **Grades from pictures:** Unreal Engine 5.8 grades with "a 16x16x16 color neutral LUT unwrapped to a 256x16 texture", made by grading a representative screenshot.
- **Provenance built in:** the C2PA specification (2.2): "The C2PA Manifest Store shall be embedded using an ancillary, private, not safe to copy, chunk type of 'caBX'". *Measured:* all five of this study's pictures carry it, signed by OpenAI, without the prompt.
- **Big files on GitHub:** release files: "Each file included in a release must be under 2 GiB. There is no limit on the total size of a release, nor bandwidth usage." Git LFS: 10 GiB of storage and of downloads a month on Free and Pro, and "If you use more than your included quota of bandwidth per month without a payment method on file, Git LFS support is disabled on your account until the next month."
- **The plan's limits** (OpenAI's Codex pricing page): "Image generations use included limits 3-5x faster on average than similar turns without image generation, depending on image quality and size." For its current model it estimates 15 to 160 local messages per five hours on Plus; "Pro plans currently have no five-hour limit"; "Weekly limits may also apply." At a limit, "ChatGPT Plus and Pro users who reach their usage limit can purchase additional credits", and "you can use /status" to see what remains.
- **Who owns the pictures:** OpenAI's terms: "you (a) retain your ownership rights in Input and (b) own the Output", and "output may not be unique".

## 5. The options

Options are compared on closeness to the feeling of the liked pictures, cost on the phone, effort for an AI builder, risk, and fit with the rules that stay.

### 5.1 Where each texture comes from

| Route | Owner's verdict | Closeness to the feeling | Effort | Risk | Fit |
|---|---|---|---|---|---|
| **T1. Code from rules** (C++ painters from catalogue data, at load) | not yet shown | Fair; the painted feel is the hard part (study 5) | A painter per material kind | Looks generic | Best: text, varied by seed |
| **T2. Code-prepared from an approved picture** | A: yes (q15) | High: it is the liked surface | Low | Ghosts and repeats; the picture's own light | Good |
| **T3. GPT swatch from words** | C: yes (q15) | Good | Low: one picture holds 6 to 12 materials | Anachronisms; no approved source to trace | Good, with vetting |
| **T4. GPT swatch redrawn from an approved picture** | D: yes (q15) | High; chunky, near pixel art | Low | Copies the input's quirks | Good |
| **T5. GPT's seamless repaint of a ground** | B: no (q15) | Low: fine, busy detail | Low | — | Dropped |
| **T6. The world draws it** (rock layers, soot, stains, wet patches, snow) | cliffs: to judge (6.12) | True by construction | Medium: code per feature | Code quality | Required by `PRN-10` |

These combine per material. A cliff is T4 (its rock surface) under T6 (each world's layers); a meadow is T2 or T4 (the grass and earth) under placed plants and stones (the kit).

### 5.2 Bringing a source onto the texel grid (measured)

Direction B's textures are pixel art, so each texel is one flat colour on an exact grid. I measured 20 sources (`grid.py`, `grid_run.py`, `grid_local_phase.py`, `band2_measure.py`). A *clear local grid* means colour changes fall on block edges in a 64-pixel window, at least 1.4 times more often than between them.

| Source | Exact grid over the whole picture? | Windows with a clear local grid | Block size | Loss when snapped (one grid / a grid per window) |
|---|---|---|---|---|
| Study 1's snapped pictures (control) | yes | 100% | 2, 3, 4 | 0 |
| The liked picture (meadow, cliff, centre) | no | 0 to 4% (cliff crop 50%, mixed sizes) | painted, about 4 to 5 (study 1) | at 2 / 4 / 6 px: 5–7% / 15–19% / 20–25% |
| Study 1's direction-B picture (d2-pixel-paint) | no | 0% | painted | 7% / 18% / 25% |
| A (code route, accepted) and B (GPT repaint, not picked) | no | 0 to 1% | painted | — |
| C swatches (textures-a, accepted) | no | 6 to 88%, mixed sizes | 4 asked, not held | 6–24% at 4 px |
| D swatches (delight-liked, accepted) | no | **100%** | **6** (4 asked) | 4–21% / **2–8%** |
| GPT's close-camp level (meadow-band2) | no | **100%** | **12** (8 asked) | 11% / **3.9%** |

What this shows:
- **No GPT picture holds one grid.** Its blocks are real but their grid drifts: in study 5's D sheet, 19 to 26 different phases appear among 64 windows. GPT also ignores the asked block size: 6 for 4, and 12 for 8.
- **Asked for blocks, GPT draws near-pixel-art** that code re-grids with little loss (2 to 8%). Asked for nothing, it paints strokes with no grid, like the liked pictures themselves.
- **Re-gridding is a fixed step** for every picture source: find the block size from the spacing of colour edges (a fractional size is allowed), find the phase window by window, and take each block's median colour as its texel.
- **For a painted source** (T2), the texel size decides what is lost. On the liked meadow, a grid of 64 texels a metre keeps all but 5 to 7% of the colour variation; a grid of 32 a metre keeps all but 15 to 19%. Most of the loss is soft shading inside strokes, which the engine's own light replaces.

### 5.3 Designed levels for the zoom bands (measured)

The owner answered q31 "neither": at the close camp both grounds turn to speckle, which the answers file reads as textures needing "a design for each zoom band, not just shrinking". I made the close camp's level (4 times coarser) of the meadow the owner saw in q31 in four ways (`bands.py`, `band2_measure.py`, `calibrate.py`). "Accents" is study 6's measure: the colour difference of the most striking 1% of texels from their surroundings, × 100.

| Way | Accents | Layout kept | Colour against band 0 | What it looks like (`band2-compare.png`) |
|---|---|---|---|---|
| Band 0, the approved closest level | 16.0 | — | — | blades, clumps, earth patches |
| **L1. Averaged** (what mipmaps do; what the owner saw) | **12.4** | by construction | lightness +0.005; contrast −38% | soft, muddy mosaic: the speckle |
| **L2. Code: each block takes its most common shade,** lone texels cleaned | 18.1 | by construction | lighter; the dark gaps lost | clean but flat, pale camouflage |
| **L3. GPT redraws the level,** band 0 given as input, then re-gridded | 18.8 | **0.85** (an unrelated patch: about 0) | lightness −0.013, hue 6° greener, contrast +22% | bold clumps, golden tops, clear earth; busy |
| **L4. L3, then its colours matched to band 0 by code** (four numbers) | 15.9 | 0.85 | matched | designed shapes in the approved colours |
| L5. Code from rules at each band's scale (T1) | not tested | — | — | study 5's to try |

What this shows:
- **Averaging is the speckle,** and a check catches it: the accents fall by more than a fifth. Four simpler statistics I tried did not separate what the owner accepted from what they called speckle (lone texels, neighbour likeness, fine-scale share, edge direction: `speckle.py` to `speckle3.py`). Study 6's accent measure did.
- **GPT can design a coarser level.** It kept the layout (so the band change during a zoom will not jump), tiled (seam steps 1.06 and 0.98), and took 42 seconds. It is busier than the approved level, and code can tune that. Matching the contrast fully also lowered the accents, so calibration should keep accents, not just spread.
- **Code alone gives clean but shapeless levels.** L2 is a fallback for small materials, not the main route.
- **Steadiness needs levels too.** Study 6's simulation: zoomed out without a level made for the zoom, every filter flickers on 10 to 43% of pixels; with one, the "smooth pixel" filter flickers on 0.1%.
- **Only the owner can judge L3 and L4 against L1** (6.12).

### 5.4 Storage, loading and compression

| Way | Closeness to the feeling | Memory on the phone (256 × 256 with levels) | Effort | Risk | Fit |
|---|---|---|---|---|---|
| **S1. Lossless with our own levels through Godot's import** (a DDS file holding the levels; "Lossless" mode) | Exact | 341 KiB | Low | One editor import per fresh session; "Detect 3D" must be off | Good |
| **S2. Lossless files kept as-is,** read by our loader in `view/` (`Image.create_from_data` with all levels, then texture arrays) | Exact | 341 KiB | Low to medium: a small loader | None new: the same path as the catalogues, with digests | **Best** |
| **S3. ASTC 4 × 4 in KTX2 from a pinned cloud tool** | Close, with block errors at texel edges | 85 KiB | Medium | Godot's KTX loader takes no arrays; to prove on the phone | Good later, if memory or bandwidth demands |
| **S4. Basis Universal (Godot's)** | As S3, plus transcoding loss | 85 KiB | Low | Godot's defaults: fastest, lowest quality | Fair |
| **S5. Compress on the phone** (Arm's encoder in `view/`) | As S3 | 85 KiB | Medium | Slower first start | Only for large code-made sets |

*Checked in Godot 4.7.2's source:*
- `Image::initialize_data` takes the whole chain: with mipmaps on, the data must be the size of every level together (`_get_dst_image_size(p_width, p_height, p_format, mm, p_use_mipmaps ? -1 : 0)`). `create_from_data` and `load_png_from_buffer` are bound for code.
- The renderer keeps the levels given: `texture.mipmaps = p_image->get_mipmap_count() + 1;`.
- The importer makes levels only when there are none: `if (!image->has_mipmaps() || p_force_normal)`. Lossless mode then stores every level (`for (int i = 0; i < p_image->get_mipmap_count() + 1; i++)`).
- Texture arrays need "All images must share the same format" and "All images must share the usage of mipmaps".
- Godot's KTX loader is in the game build, but its documentation says "Cubemaps, texture arrays, and de-padding are not supported".

On disk, pixel-art textures are small (`sizes`, this study): study 5's re-gridded swatches (68 × 67 texels) are 8 to 11 KiB; GPT's close-camp level (103 × 103) 23 to 26 KiB; a soft 256 × 256 tile 115 to 140 KiB.

### 5.5 Where it all lives

| Way | Cost on the phone | Effort | Risk | Fit |
|---|---|---|---|---|
| **W1. Everything committed, as now** | none | none | History past GitHub's advice already; "GitHub blocks files larger than 100 MiB" | Poor |
| **W2. Git LFS for APKs and pictures** | none | Low | 10 GiB of downloads a month; every fresh session downloads; beyond it LFS is off until next month | Poor (`PRC-01`) |
| **W3. Release files for APKs and GPT's originals; git for text and prepared textures** | none | Low to medium: `gh release` in the delivery script (to prove from a cloud session) | Releases from cloud sessions untested | **Best** |
| **W4. Code-made textures made on the phone at first start** | a slower first start | — | — | Part of every option; pictures must ship (`PLT-03`) |

### 5.6 Rock: a surface with each world's layers laid on by code (measured)

To answer the owner's question 32, `cliff_layers.py` draws a 4 × 2.2 m piece of cliff for three worlds, as a stand-in for the engine:
- **World 1:** limestone beds over a soft shale that has weathered back into a rock shelter, with soot above its mouth.
- **World 2:** sandstone with thin red silt partings, dipping 7°.
- **World 3:** chalk with bands of flint.

Code lays the beds (thicknesses that pinch and swell), joints that slant and sometimes stop, each block a facet turned its own way, lichen, water stains, hanging grass, and a dark shelter (`PRE-23`, `PRE-24`, `WLD-09`). The rock surface goes on the texel grid; light is smooth, from a low sun at the upper left.

| Surface | Texel contrast at 64 / 32 a metre | By eye |
|---|---|---|
| The liked cliff (reference) | 0.056 / 0.060 | chunky cream and lilac clusters |
| GPT's layer-free surface (q32 B) | **0.016 / 0.015** | smooth, sandy; the same as the control |
| No surface at all (control) | 0 | smooth sculpted stone |
| Study 5's D limestone (accepted in q15) | 0.051 (its own grid) | chunky; close to the liked cliff, closest at 32 a metre |

What this shows:
- **The layers work:** the three worlds read as different rocks, with overhangs, partings and soot.
- **GPT's fine surface adds almost nothing at the game's density.** A contrast check against the approved source would have caught it before it was shown.
- **A chunky surface is needed,** drawn on a grid (T4), not a fine repaint (T5).
- **These are code mock-ups, not the engine.** Plants on ledges (kit things) are left out, and the view is from the front, not from 40° above.

### 5.7 Colour grades

| Way | Closeness to the feeling | Cost on the phone | Effort | Fit |
|---|---|---|---|---|
| **G1. A LUT made at load from a small text grade per key light,** applied by Godot's colour correction | The broad colour only | Under 0.1 ms (study 4); one 3D texture | Low | Best: a text source |
| **G2. Two LUTs blended as the sun moves** (our final pass) | As G1, smoother | Slightly more | Medium | Good |
| **G3. A LUT fitted to a GPT recolour of an engine frame** | The broad colour only | As G1 | Low | A starting point at most |

*The regrade test* (`regrade_check.py`, `regrade_align.py`, `regrade_place.py`). GPT was asked to change only the colours of study 6's engine frame:
- **It moved the content:** the picture is squeezed by 0.26% down its height and shifted about 2 pixels. Realigned, its content sits within a third of an art pixel.
- **Then one table explains 90% of its per-pixel colour,** against 99.6% for a true recolour made by code.
- **But a table fitted on the top half predicts only 48% of the bottom** (a true recolour: 85%), and 89% when fitted on alternate bands (99.5%). So GPT recolours partly by place, as its prompt's "sun from the upper left" invited.
- **The owner's verdict settles the method** (answer 24): the grade is tuning. The feeling comes from light, content, textures and density.

### 5.8 The owner's loop

| Way | What the owner gets | Time from a change to their eye | Risk |
|---|---|---|---|
| **I1. As now** | Each alpha | About 30 minutes or more | Slow rounds |
| **I2. The look lab in every build** (round 1) | Variants side by side, live sliders, a look code to paste back | Instant within a build | None |
| **I3. Cloud pictures on the note page** | A first look without installing | Minutes | The cloud's software driver is not the phone's chip |
| **I4. A lab app beside the game** (`dev.kindling.lab`: the same code, test scenes only, from the work branch, as a release file) | Look changes between alphas | About 5 to 10 minutes (estimate) | None to the worlds: a separate app cannot read the game's storage |

## 6. What we'd recommend

### 6.1 The rule, updated

**Every graphics element has one source:** catalogue text, C++ generators, shader files, or a picture the owner approved with its text record.
- The phone makes from text whatever it can, at load or when an area is made.
- Picture-made textures are re-gridded, given their designed levels and calibrated in the cloud, then shipped lossless in the APK.
- Nothing is downloaded on the phone, and no build ever calls GPT.

### 6.2 The texture path, step by step

1. **Request** (text, committed): one or several materials per picture, from a template (6.9). It always gives the area in metres and asks for blocks of 8 to 12 picture pixels, so each block is clear.
2. **Run** by the builder through Codex, outside the build: about 40 to 55 seconds a picture. Kept: the request, the prompt exactly as passed, a run record (Codex version, model, times, tokens) and the original file with its C2PA record.
3. **Vet** (the builder, recorded): Stone Age truth against studies 1 and 5's lists; nothing countable in a ground texture (paths, flowers, stones are placed by the world); scale as asked.
4. **Re-grid** (a deterministic cloud tool): block size and phase window by window, median colour per block. Then a seam blend where needed, and a light check (slope at most 0.02). Lossless.
5. **Designed levels:** one per zoom band where the material is seen. For big surfaces, GPT redraws each band with the level above as input (L3), then steps 2 to 4 again. For the rest, the code reduction (L2), or code from rules (T1). Each level records the digest of the level it was made from, so a change upstream marks it stale.
6. **Calibrate in the lab:** draw the material lit at the target card's hours; fit a few numbers (lightness, hue, colourfulness, contrast) until each band matches band 0 and the card. The numbers go into the record, never a second picture.
7. **Approve:** the note shows each texture on a lab sheet: flat and lit, at every band, at true phone size and enlarged, beside its approved source. The owner's words and the date go into the record.
8. **Ship:** the prepared levels are committed as lossless files and listed with their SHA-256 in `build.toml`, inside the look digest.
9. **On the phone:** `view/` reads each file through `FileAccess`, builds one image holding all its levels (`Image.create_from_data`), gathers each material kind into a texture array, and uploads them before the warm-up. The self-check compares digests.

### 6.3 A record

One per picture-made texture: a catalogue kind, so the loader's checks apply. Where it is registered is study 3's.

```toml
# data/base/texture/meadow_grass.toml  (an example: the values are illustrative)
about = "short wild meadow grass and bare earth: the background of the meadow cover"
metres = 4.0                      # the side of band 0's tile
texels_a_metre = 32               # band 0; each next band halves it (6.4)
original = { release = "art-2026-10", file = "meadow_grass-original.png", sha256 = "...", c2pa = "present" }
made = { tool = "Codex 0.160.1, its image tool", when = "2026-10-06T10:50Z", request = "art/requests/meadow_grass.txt" }
regrid = { block_px = 12.0, loss = 0.039 }
stone_age = { by = "builder", when = "2026-10-06", note = "wild grasses only; no path, flowers or shoe prints" }

[[band]]                          # band 0: the closest zoom
file = "art/textures/meadow_grass.b0.png"
sha256 = "..."
[[band]]                          # band 2: the close camp
file = "art/textures/meadow_grass.b2.png"
sha256 = "..."
made_from = { band = 0, sha256 = "..." }        # stale if band 0 changes
way = "GPT redraw, re-gridded, calibrated"
calibrated = { lightness = 0.013, hue_deg = -6.2, chroma = 1.03, contrast = 0.90 }
approved = { by = "owner", when = "2026-10-07", words = "yes to D" }
```

### 6.4 The texel density: one number, set by the owner's eye

Everything in the pipeline follows one number: texels a metre at band 0, halving at each band. Two readings of the owner's answers:

| | 64 texels a metre | 32 texels a metre |
|---|---|---|
| A texel at the closest zoom (8 m across) | about 2 screen pixels | about 4 screen pixels |
| What it matches | the builder's reading of answers 1, 2 and 5 | the liked picture's own blocks (4 to 5 pixels for every material, people included: study 1's `blocksize.py`); answer 2, chosen on a close-camp picture (about 30 a metre); the closest the cliff came to the liked one (5.6) |
| A face (about 23 cm) | about 15 texels high | about 7 texels high; the outline stays smooth, from the geometry |
| Memory, 150 materials, one 4 m tile each with its levels | about 50 MiB lossless | about 13 MiB |
| GPT's blocks for band 0 | 4 to 6 pixels (GPT drew 6 when asked 4) | 8 to 12 (GPT drew 12 when asked 8) |

**I'd recommend 32,** unless the owner's eye says otherwise on the true-size cliffs (6.12, comparison 2). It matches the pictures they love and costs a quarter of the memory. At 2 screen pixels, a texel is about 0.13 mm on the phone's 390 dpi screen, close to the smallest detail the eye can see at reading distance. The choice belongs to studies 1 and 2's wording of the lifted `PRE-22`; the pipeline needs only the number.

### 6.5 Budgets

- **Memory** (Godot's table: 341 KiB for 256 × 256 with levels, lossless):
  - at 32 a metre, about 150 materials with their levels take about 13 MiB, or up to about 40 MiB if coarse bands use larger tiles against repetition;
  - at 64 a metre, about 50 to 150 MiB.
  - ASTC would be a quarter of these. First budget: 64 MiB of textures, to measure on the phone.
- **The APK:** about 10 to 30 MB of lossless textures (estimate: about 150 materials, 2 to 3 levels, 8 to 140 KiB each). First budget: 150 MB for the whole APK.
- **GPT pictures** for M2's texture set (estimate):
  - band 0 for about 150 materials at 6 to 12 swatches a picture: 15 to 25 pictures;
  - designed coarse levels for the 30 to 40 large-surface materials, 2 to 3 bands each: 60 to 120 pictures, fewer if levels come as sheets (untested);
  - with redos, about 100 to 200 pictures: 3 to 6 hours of runs at today's pace, spread over days if the plan's limits are met.

### 6.6 Where it all lives

| What | Where | Why |
|---|---|---|
| Catalogue entries, tuning, records, requests, prompts, run records | git (`data/`, `art/requests/`) | one source per thing |
| Prepared levels (lossless) | git (`art/textures/`), about 8 to 140 KiB each | the build needs no network |
| GPT's originals (with C2PA), targets, references | release files, one "art" release per batch, digests in the records | provenance; never shipped; keeps git small |
| Code-made and world-made textures | made on the phone, at load or with the area; cached by look digest | `PRN-10`; a small APK |
| APKs | release files, one pre-release per alpha; the note links it | stops the history growing |

- **Updates** (`PLT-09`): a texture or grade change only changes the look digest, so it is a small update.
- **Offline** (`PLT-03`): everything is in the APK or made on the phone.
- **The 47 APKs already in the history** stay unless the owner asks to rewrite it: that would break old links and branches.

### 6.7 Shaders and grades

- **Unchanged from round 1:** shader files per surface kind, a warm-up scene drawing every kind in every pass, Godot's pipeline counters with a pass line of no compilation during play.
- **Added:** each material kind samples a texture array (one layer per material), so more materials add layers, not shaders. The warm-up binds the real arrays and turns the LUT on, since colour correction is a switch compiled into the tonemap shader (`use_color_correction`).
- **Grades:** G1 by default, G2 if one LUT at a time is not smooth enough through the hours; a grade set per season with a true midday (answer 9). G3 only to start a grade. Dusk targets come from relighting an approved picture (answer 23, study 6).

### 6.8 Checks without a palette

Each runs only when what it reads changed, as `tools/check.sh` already does. The lines come from today's measurements and are to be refitted on the M2 sample.

| Check | Where | Starting line |
|---|---|---|
| Every texture traces to its record; every record to its levels, original (release file, digest, C2PA), request, approval and Stone Age check | cloud, at build | none missing |
| No stale level (made from an older version of the level above) | cloud, at build | none |
| Re-grid loss | cloud, at preparation | at most 10% |
| Seams | cloud, at preparation | step at most 1.2 |
| No painted light | cloud, at preparation | slope at most 0.02 |
| No strong repeat inside | cloud, at preparation | at most 0.2 |
| **Texel contrast at the band's density,** against the approved source | cloud, at preparation | within ±25% (GPT's rock surface, a quarter, fails) |
| **Accents at every band,** against band 0 (study 6's measure) | cloud, at preparation | at least 90% (the averaged level, 77%, fails) |
| Colour drift between bands | cloud, after calibration | lightness within 0.02; hue within 5° |
| Density and budgets | cloud, at build | 6.4 and 6.5 |
| Target card on lit lab scenes | cloud on each look change; the phone in the self-check | study 6's card |
| Golden pictures within a perceptual tolerance | cloud | against mistakes, not taste |

A statistic becomes a check only after it agrees with the owner's verdicts: four plausible speckle measures did not (5.3). The statistics both the cloud and the phone compute are written once, in C++ (`CLAUDE.md` rule 4).

### 6.9 The GPT step

- **Its place:** before the build, never in it. A failed or refused run, or a limit, only delays new sources.
- **The request template** keeps BRIEF2's detailed form (camera, scene, light, look, Stone Age truth, what to avoid). It adds:
  - straight down or straight on, orthographic;
  - the area in metres and the blocks in picture pixels (8 to 12);
  - background material only; even, overcast light; tileable;
  - an approved picture or the level above as input wherever one exists.
- **Never relied on:** the size, the block size, the scale or a perfect seam. GPT returned 1254 × 1254 for 1024 × 1024 asked, and blocks of 12 for 8.
- **Three uses, kept apart:** references (kept, never measured); targets (study 6's card, never shipped); texture sources (re-gridded, approved, shipped). Only sources need the full record.
- **Its budget:** each step lists its pictures in its plan. The builder checks `/status` in Codex before a batch, and every run is recorded. Today: 72 pictures between 10:00 and 11:57, none failed, no limit met.
- **At a limit:** the builder finishes the turn, records which requests wait, carries on with existing sources and code-made textures, and resumes after the reset. It never buys credits: only the owner can choose a running cost (`PRC-01`).
- **Terms:** the owner owns the output; it may not be unique; nothing secret goes into a prompt.

### 6.10 What stays from round 1, and what changes

| Round 1 | Now |
|---|---|
| One text source per element | Stays; a picture is a source too, with its record |
| Generators in C++, in a Godot-free part of `view/` | Stay: shapes, poses, code textures, world textures (rock layers, soot), grades |
| Shader files, a warm-up scene, the pipeline counters | Stay; arrays bound and the LUT on in the warm-up |
| The look lab, look codes | Stay; the lab adds material sheets, flat and lit, at every band and at true size |
| Palette textures and ramps; off-palette checks | Become grades from text and the checks of 6.8 |
| "Nothing generated is committed" | Prepared levels are committed as sources; GPT's originals are release files |
| Textures at 16 texels a metre | One number at band 0 (32 recommended), halving at each band, every level designed |
| Godot's import for pictures (ETC2 and ASTC) | Lossless files with our own levels; Godot's import only for the icon and interface |
| The APK committed (50 MB line) | Release files; an APK budget |
| Models made by code at load | Stay (study 5); several shelter types and detailed figures are more kit data |
| Effects made by code | Stay: fire follows its heat (`MAT-18`), so no AI flipbooks |

### 6.11 Proposals for the owner

Each is a proposal (`PRC-07`); nothing changes without the owner's OK.

1. **Delivery** (A2.3, `tools/verify-apk.sh`; `PLT-06` and `PRC-11` keep their meaning): each alpha's APK becomes a release file linked from the note, no longer committed; the 50 MB line becomes an APK budget of 150 MB. Worth doing now, whatever the look.
2. **`MIL-09`:** "the model kit and its textures made by code" becomes "the model kit made by code, and its textures made by code, by the world, or prepared by code from pictures you approved, each traced to its picture, prompt and your approval". Study 5 proposes a similar widening; one wording should be chosen.
3. **A5.3:** "Every texture is drawn at 16 texture pixels a metre" becomes "Every texture is pixel art on an exact grid: 32 texels a metre at the closest zoom (the number set by the owner's eye), halving at each zoom band, with a level designed for each band where the material is seen".
4. **A new rule in A5 and A6:** a ground texture holds only its background material; anything countable (paths, flowers, stones, tufts) is a thing the world places (`PRN-10`, `PRE-46`, `WLD-31`).
5. **A3.6 and A6:** a `texture` catalogue kind holding the record; each material names its route (picture, code or world), its levels and how they layer.
6. **A2.2:** "Android export needs ETC2 and ASTC texture imports on" becomes: textures are lossless data files with their own levels and digests, read by `view/`; Godot's import only for the icon and interface pictures, with "Detect 3D" off; ASTC only if the phone shows the need (7).
7. **A4.4 and A17:** the checks of 6.8.
8. **Process:** a picture budget per step, the run record, and never credits without the owner.
9. **Optional:** the lab app (I4).
10. **Still standing from round 1:** add to `PLT-04`'s "Smooth" that no shader is compiled while drawing once a world is open.

### 6.12 Pictures for the owner's eye

All are in `work2/7/`.

1. **Cliffs with each world's layers laid on by code** (follows question 32). Shown at twice the phone's size at the closest zoom:
   - A: the liked cliff (`crop-cliff-1x.png`, enlarged twice);
   - B: GPT's layer-free surface with world 1's layers (`cliff-w1-limestone-over-shale-B.png`);
   - C: study 5's accepted limestone swatch with the same layers (`cliff-w1-limestone-over-shale-D.png`);
   - D: world 2, sandstone with silt (`cliff-w2-sandstone-and-silt-D.png`);
   - E: world 3, chalk with flint (`cliff-w3-chalk-with-flint-D.png`);
   - F: C at 32 texels a metre (`cliff-w1-limestone-over-shale-D-32tpm.png`).

   *To ask: do B to F keep the feel of the liked cliff, and which surface, B or C?*
2. **How big a texture pixel up close?** Show at true size, 1:1, not enlarged:
   - A: 64 texels a metre (`cliff-w1-limestone-over-shale-D-true-size.png`);
   - B: 32 texels a metre, like the liked picture's own blocks (`cliff-w1-limestone-over-shale-D-32tpm-true-size.png`).
3. **The close camp's ground** (follows question 31): `band2-compare.png`, each at true phone size and enlarged:
   - A: averaged (what the owner saw);
   - B: reduced by code;
   - C: redrawn by GPT on a coarse grid;
   - D: C with its colours matched to the approved level.

   *To ask: which reads as the liked meadow, seen from farther away?*

## 7. Questions only the phone can answer

1. **Do lossless textures with our own levels load right?** A lab page with the sample set: colours read back and compared with the cloud's; every level present; arrays built.
2. **How long does loading take?** Trace sections in the self-check: reading, building and uploading the texture set, and making code textures at first start and after an update; against `PLT-04`'s 3 seconds.
3. **How much memory?** Godot's texture memory monitor, with the sample set and the full budget.
4. **Does compression pay?** Frame time, power and heat over ten minutes with the sample lossless and as ASTC 4 × 4 (study 4). Godot expects little gain for pixel art.
5. **Do the bands switch without a jump?** A slow zoom through every band, recorded; changed pixels counted at each switch (study 6's shimmer measure), and the owner's eye.
6. **Does a texture shimmer as the camera turns?** By density (32 or 64) and filter (study 2).
7. **What does the grade cost and look like?** Colour correction on and off; one LUT against two blended.
8. **How long is the first start after an update?** Caches remade, code textures and shaders.
9. **Do the phone's code textures match the cloud's?** Their fingerprints in the self-check.
10. **How long does a 60 to 150 MB APK take to download and install** on the owner's connection?

## 8. Risks

| Risk | How it could be retired early |
|---|---|
| GPT's levels drift from their band 0 in colour or character | The "made from" digest, calibration, the accent and drift checks, the owner's lab sheet |
| GPT's grid or block size is off | Re-gridding finds the real size; the loss check |
| Designed levels pop or ghost as the camera zooms | Levels keep band 0's layout (GPT kept 0.85); the band-switch test (7.5) |
| Releases cannot be made from cloud sessions | Try `gh release` with a test file in M2's first step; meanwhile commit APKs only while under 100 MiB |
| GPT's limits stop work mid-milestone | Batches in the plan, `/status` first, code-made fallbacks; the build never waits on GPT |
| Anachronisms in AI sources (picture 8 had chickens and sawn planks; study 5's axe came out metal) | The vetting step and the owner's approval |
| A statistic passes what the owner dislikes | Checks only after they agree with the owner's verdicts (6.8) |
| Memory at 64 texels a metre with many bands | Density 32; ASTC if needed; bands as levels of one tile where repetition allows |
| A picture cannot be made again | The original with its C2PA record kept as a release file; the prompt in git |
| The history keeps growing | Proposal 1 |

## 9. For other studies

- **Study 1:** the liked picture paints every material, people included, with blocks of 4 to 5 pixels; at its 8 m view that is about 25 to 32 a metre. The owner chose "2x2" on a close-camp picture (about 30 a metre). Both point to 32 texels a metre at band 0 (6.4). The true-size cliff pair (6.12, comparison 2) lets the owner decide.
- **Study 2:** textures arrive on an exact grid with designed levels, so the filter can treat each level as clean pixel art. Bands that are mip levels of one tile switch by the chip's level choice; bands with larger tiles need a blend at the switch. Godot's own mipmaps average stored bytes, so they are not used.
- **Study 3:** textures as lossless data files read through `FileAccess`, one image per material holding all its levels (`Image.create_from_data`), arrays per material kind (`Texture2DArray.create_from_images`: one format and one mipmap use). `view/`'s godot-cpp build profile must name those classes. Also the `texture` kind's registration and caches in the phone's storage by look digest.
- **Study 4:** at 32 texels a metre, about 13 to 40 MiB of lossless textures; at 64, about 50 to 150 MiB; ASTC a quarter. Godot's documentation expects little speed from compressing pixel art; the phone should confirm (7.4).
- **Study 5:**
  - GPT asked for blocks draws near-pixel-art that re-grids with 2 to 8% loss, but never at the asked block size;
  - GPT can redraw a coarser band from the level above, keeping layout and accents;
  - a fine layer-free rock surface adds nothing at the game's density, while a chunky swatch works;
  - the code reduction (L2) is the fallback, and code from rules at each band's scale (L5) is untested;
  - small plants (answer 21) need designed levels for their cards too.
- **Study 6:** its accent measure separates the averaged level (12.4) from designed ones (16 to 19); four other measures did not. Calibrating contrast fully lowered accents, so calibration targets should include accents. GPT's colour-only regrade moved the picture by up to about 6 pixels, so any paired measure needs registration first.

## 10. Pictures from GPT

All were run by the builder through Codex 0.160.1 and its image tool. Each prompt was passed exactly as written, and each file carries its C2PA record. Five of 16 used.

1. **`work2/7/gpt/ground-from-approved.png`** (request `ground-from-approved.txt`; input: a crop of the liked meadow and path, enlarged three times). *Can GPT turn an approved picture into a flat, tileable ground texture?*
   - Showed: a flat, evenly lit meadow (light slope 0.007) with near-invisible seams and close colours. Its path and flower clumps repeat every 2.5 m, and turn to speckle at the close camp.
   - Owner: not picked as a surface (q15 B); speckle at the close camp (q31 A). 43 seconds.
2. **`work2/7/gpt/rock-detail-from-approved.png`** (input: the liked cliff, enlarged twice). *Can GPT strip an approved cliff to a layer-free rock surface?*
   - Showed: no layers, no light, near-perfect seams; but 0.12 lighter than the approved rock, and too fine: its texel contrast at the game's density is a quarter of the liked cliff's. With layers laid on by code it looks the same as no surface (5.6).
   - Owner: "can't judge yet" (q32); the pictures with layers are ready (6.12). 53 seconds.
3. **`work2/7/gpt/meadow-at-game-scale.png`** (same input). *Does asking at the game's scale, without path or flowers, keep the detail at the close camp?*
   - Showed: a flat meadow of clumps and bare patches with no strong repeat, but no more contrast at the close camp's density than picture 1.
   - Owner: speckle at the close camp (q31 B). 42 seconds.
4. **`work2/7/gpt/regrade-only.png`** (input: study 6's engine frame at noon). *Can a grade come from GPT if it changes only colours?*
   - Showed: the layout kept but the picture squeezed by 0.26% and shifted about 2 pixels. Realigned, one table explains 90% of its colour (a true recolour: 99.6%), but a table from one half predicts the other only 48% (true: 85%): GPT recolours partly by place.
   - Owner: "colour alone doesn't bring the feeling" (q24).
5. **`work2/7/gpt/meadow-band2.png`** (request `meadow-band2.txt`; input: band 0 of picture 3's meadow at 64 texels a metre, 8 m). *Can GPT redraw a closest-zoom ground as the close camp's level on a grid four times coarser, with shapes designed at that size?* Asked because answer 31 needs designed levels and code alone gave only shapeless camouflage.
   - Showed: a clean grid of 12-pixel blocks (8 asked; about 13 texels a metre for 16 asked) that re-grids with 3.9% loss. It kept the layout (0.85) and the accents (18.8 against band 0's 16.0, where averaging gives 12.4), and tiles. It was 22% more contrasty and 6° greener; matched by code in four numbers. 42 seconds, 23,585 tokens.
   - Owner: to judge (6.12, comparison 3).

## 11. Sources

**Repository** (read 6 October 2026):
- `CLAUDE.md`, `PROJECT.md` (`PRN-10`, `PRN-14`, `PRE-01` to `PRE-46`, `WLD-09`, `MIL-09`, `PLT-01` to `PLT-10`, `PRC-01` to `PRC-12`, `SND-06`, `PRE-40`), `ARCHITECTURE.md` (A2, A3.6, A4, A5, A6, A17, A18), `IMPLEMENTATION.md`, `LESSONS.md`, `dist/M1-REPORT.md` ("5 to 15 minutes of the 4 cores"), `tools/verify-apk.sh` ("over the 50 MB a committed file may have").
- Measured with `git count-objects` and `git cat-file`: 1.06 GiB of objects; 49 commits changing `dist/kindling.apk` across all branches, 47 distinct APKs, 1,023 MB.

**This round's files:**
- The owner's answers, `owner/ask/answers.md`.
- Study 1's `blocksize.py` and `blocks.txt`; study 5's swatches in `work2/5/tex/`; study 6's `speckle2.py` (the accent measure) and its round-2 draft (the shimmer simulation).
- The builder's run logs (`work2/*/gpt/run.log`, `owner/contexts/run.log`) and each picture's `.log` (Codex 0.160.1, its model, tokens, times); the pictures' `caBX` chunks, read with Python.

**Godot 4.7.2 source** (tag `4.7.2-stable`):
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/core/io/image.cpp
  - `p_out = static_cast<uint8_t>((p_a + p_b + p_c + p_d + 2) >> 2);` (used by `_generate_po2_mipmap` for 8-bit formats)
  - `int64_t size = _get_dst_image_size(p_width, p_height, p_format, mm, p_use_mipmaps ? -1 : 0);` (in `Image::initialize_data`)
  - (`create_from_data`, `load_png_from_buffer` and `load_ktx_from_buffer` bound)
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/editor/import/resource_importer_texture.cpp
  - `if (!image->has_mipmaps() || p_force_normal) {`
  - `for (int i = 0; i < p_image->get_mipmap_count() + 1; i++) {` (lossless mode)
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/storage_rd/texture_storage.cpp
  - `texture.mipmaps = p_image->get_mipmap_count() + 1;`
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/scene/resources/image_texture.cpp
  - "All images must share the same format"
  - "All images must share the usage of mipmaps"
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/modules/ktx/texture_loader_ktx.cpp and `modules/ktx/register_types.cpp` (the KTX loader in the game build)
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/modules/basis_universal/register_types.cpp (the Basis encoder only inside `#ifdef TOOLS_ENABLED`)
- https://raw.githubusercontent.com/godotengine/godot/4.7.2-stable/servers/rendering/renderer_rd/shaders/effects/tonemap_mobile.glsl
  - `layout(constant_id = 3) const bool use_color_correction = false;`

**Godot 4.7 documentation:**
- https://docs.godotengine.org/en/4.7/tutorials/assets_pipeline/importing_images.html
  - "Even in 3D, "pixel art" textures should have VRAM compression disabled as it will negatively affect their appearance, without improving performance significantly due to their low resolution."
  - "This is also the recommended setting for pixel art."
  - "If mipmaps are present in the texture, they will be loaded directly. This can be used to achieve effects using custom mipmaps."
  - "Cubemaps, texture arrays, and de-padding are not supported."
  - (the memory table: 256×256, 341 KiB lossless, 85 KiB VRAM Compressed; and Detect 3D changing the mode to VRAM Compressed)
- https://docs.godotengine.org/en/4.7/tutorials/assets_pipeline/import_process.html
  - "files with this resource type will be preserved as is during project export."
- https://docs.godotengine.org/en/4.7/classes/class_environment.html
  - "Color correction does not currently support HDR output due to only supporting values in the SDR (0.0 to 1.0) range."
- https://docs.godotengine.org/en/4.7/tutorials/performance/pipeline_compilations.html
  - "make sure to have a scene that uses them as early as possible"

**Textures, levels and colour:**
- Microsoft, How to: Export a Texture that Contains Mipmaps (Visual Studio 2013, archived; old): https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2013/dn449511(v=vs.120)
  - "When you do not need to specify the image content of each MIP level manually—as you might do to achieve certain effects—generating mipmaps at build time ensures that mipmap contents never become out-of-sync"
- Imagination, Do Use Mipmapping: https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/rules/do-use-mipmapping.html
  - "Ideally mipmaps should be created offline using a tool like PVRTexTool."
- Imagination, Do Use Texture Compression: https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/rules/do-use-texture-compression.html
  - "Reduce the memory footprint and bandwidth cost of the texture assets."
- NVIDIA, GPU Gems 3, chapter 24 (Gritz and d'Eon, 2007; old): https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-24-importance-being-linear
  - "If you are in a nonlinear color space with a gamma, say of 2.0, then that coarse-level texel with a value of 0.5 will be displayed at only 25 percent of the brightness."
- Arm, ASTC format overview: https://github.com/ARM-software/astc-encoder/blob/main/Docs/FormatOverview.md
  - "The 2D block footprints in ASTC range from 4x4 texels up to 12x12 texels, which all compress into 128-bit output blocks."
- Khronos, `ktx create`: https://github.khronos.org/KTX-Software/ktxtools/ktx_create.html
  - "--generate-mipmap Causes mipmaps to be generated during texture creation."
- Binomial, Basis Universal README: https://github.com/BinomialLLC/basis_universal
  - "Transcoding UASTC LDR to ASTC LDR and BC7 is particularly fast and simple, because UASTC LDR is a common subset of both BC7 and ASTC."
- Heitz and Neyret, HPG 2018: https://eheitzresearch.wordpress.com/722-2/
  - "The key to this approach is the blending operation that usually produces visual artifacts such as ghosting, softened discontinuities and reduced contrast, or introduces new colors not present in the input."
- Adobe, Substance 3D Sampler, Delight (AI Powered) (updated 7 April 2026): https://experienceleague.adobe.com/en/docs/substance-3d-sampler/using/filters/tools/delight-ai-powered
  - "The Delighter allows you to remove lighting information from the base color channel."
- Epic Games, Unreal Engine 5.8, Using Lookup Tables for Color Grading: https://dev.epicgames.com/documentation/en-us/unreal-engine/using-lookup-tables-for-color-grading-in-unreal-engine
  - "a 16x16x16 color neutral LUT unwrapped to a 256x16 texture"
- C2PA Technical Specification 2.2: https://spec.c2pa.org/specifications/specifications/2.2/specs/C2PA_Specification.html
  - "The C2PA Manifest Store shall be embedded using an ancillary, private, not safe to copy, chunk type of 'caBX'"

**GitHub and OpenAI:**
- GitHub Docs, About large files on GitHub: https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github
  - "We recommend repositories remain small, ideally less than 1 GB, and less than 5 GB is strongly recommended."
  - "GitHub blocks files larger than 100 MiB."
- GitHub Docs, About releases: https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases
  - "Each file included in a release must be under 2 GiB. There is no limit on the total size of a release, nor bandwidth usage."
- GitHub Docs, Git LFS billing: https://docs.github.com/en/billing/concepts/product-billing/git-lfs
  - (the table: GitHub Free and Pro, 10 GiB of bandwidth and 10 GiB of storage)
  - "If you use more than your included quota of bandwidth per month without a payment method on file, Git LFS support is disabled on your account until the next month."
- OpenAI, Codex pricing (https://developers.openai.com/codex/pricing, redirecting to https://learn.chatgpt.com/docs/pricing; read 6 October 2026):
  - "Image generations use included limits 3-5x faster on average than similar turns without image generation, depending on image quality and size."
  - "Pro plans currently have no five-hour limit."
  - "Weekly limits may also apply."
  - "ChatGPT Plus and Pro users who reach their usage limit can purchase additional credits to continue working without needing to upgrade their existing plan."
  - "you can use /status"
  - (the table: the Plus plan, 15-160 local messages per five hours)
- OpenAI, Terms of Use: https://openai.com/policies/row-terms-of-use/ (the page refuses our tools; on 6 October 2026 a search engine's copy of it held both quotes, and a copy of the 11 December 2024 version at https://open.windriver.com/info/uni-license-list/licenses/openai-tou-20241211.html holds both in full)
  - "As between you and OpenAI, and to the extent permitted by applicable law, you (a) retain your ownership rights in Input and (b) own the Output."
  - "Due to the nature of our Services and artificial intelligence generally, output may not be unique and other users may receive similar output from our Services."


## Addendum by the builder, after this note: the owner's answers to its questions

Answered on 6 October 2026, recorded in full in `R/owner/ask/answers.md`:
- **35, texture pixel size up close:** A, about 2 screen pixels (64 texels a metre at the closest zoom).
- **34, rock surface under code-laid layers:** neither B (GPT's fine surface) nor C (study 5's chunky swatch) keeps the liked cliff's feel yet. **34b, three worlds' cliffs with code-laid layers:** yes, they keep its feel.
- **36, the close camp's ground:** D, GPT's redraw with its colours matched by code.
- **33, people in the busy autumn wood:** A, light only (study 2's read-aids), not the changed world.
