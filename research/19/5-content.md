# Study 5, round 2: how to make the content for direction B

> Study 5 of [research 19](../19-graphics.md), written on 6 October 2026 and kept as written, its quotes checked ([the check](quote-check.md)).
> `R` was the research session's working folder and is not kept, except your pictures and answers, now in `art/targets/` and `art/reviews/2026-10-06-graphics/`.

*M2 research, round 2, study 5 of 7, 6 October 2026. Resumed after the container restarted at about 11:33; no earlier work was redone. Research only: nothing here is decided until the owner says so (`PRC-07`). Godot facts are checked in Godot's own source and class reference at the tag `4.7.2-stable` (our version). Measurements are small Python scripts in `R/work2/5/meas/` and `R/work2/5/tex/`; their numbers are in `R/work2/5/meas/numbers-round2b.txt`. GPT drew 13 pictures for this study (section 10). `R` is the research folder.*

**Words used here.**
- *Direction B:* the owner's choice (answer 1): a smooth, sharp 3D world drawn at the phone's full resolution, whose surfaces wear textures painted as pixel art.
- *Texel:* one pixel of a texture. At the closest zoom a texel shows as about 2 × 2 screen pixels (answer 2), so textures hold about 67 texels a metre there.
- *Zoom band:* a range of zoom that shares one design. I use three: the **closest zoom** (about 8 m across the screen, as in the liked picture, answer 5), **about 16 m across**, and the **close camp** (about 36 m across).
- *Drawn for its size:* a design made for the size it is seen at, as type designers make small type sturdier and simpler than large type. The opposite is *shrunk*: a big painting made smaller by averaging.
- *Cut-out:* a flat card whose see-through texels are thrown away, the usual way to draw leaves and grass.
- *Mipmaps:* smaller copies of a texture that the graphics chip uses when the texture is seen small. A *hand-made mipmap* is one we draw ourselves instead of letting the chip average.
- *Skeleton, skinning, weights:* a figure's bones; bending its surface with them; how much each bone moves each point of the surface.
- *Target, guide, source:* a picture that shows the look to aim for (target), shows a design to follow (guide), or is prepared by code into a texture (source). None of them ships as it is.

## 1. The question, and the answer in brief

**The question:** how should an AI builder make content rich enough for the feeling of the liked pictures (models, textures, people, animals, plants and movement), now that the strict art rules and the "made by code" method are open, and the owner has chosen direction B?

**The answer in brief:**
- **Code builds the kit; approved pictures paint its surfaces.** Shapes, layouts, skeletons, poses, gaits, variety and wear stay code and catalogue data, which is what keeps `PRE-42`, `PRE-43` and `PRE-46` true. Surfaces come by three routes, as study 7 proposes: code, the world itself, and pictures the owner approved, prepared by code. The owner accepted all three kinds of surface (answer 15).
- **New: every small thing is drawn for the size it is seen at, never only shrunk.** This answers the owner's two problems: small plants lose their charm (answer 21) and the ground turns to speckle (answer 31). Asked for three sizes, GPT redraws each plant with fewer, bolder parts and keeps the flowers and berries, but it cannot draw on the true small grid; a generic filter removes speckle but turns ground into camouflage. So the small bands are designed by code rules and tiny pixel designs written as data, guided by GPT's design sheets, and stored as hand-made mipmap levels.
- **People and animals detailed, as chosen (answers 3, 14), made by code around one skeleton per body pattern,** as Spore did, with garments as shells and a face drawn for each band (about 14 texels tall at the closest zoom). Still poses follow approved pose sheets (answer 20); walking, running and galloping come from numbers per leg.
- **Every picture is checked for Stone Age truth.** GPT drew metal tools after an explicit "no metal", and the Iceman museum now calls his "grass cape" a mat. The checks cost no feeling: the truthful village and the first cave kept it (answers 28, 29).
- **Shelters in several types (answer 13),** each tied to what excavations show: hides across rock-shelter mouths, skin tents held down by stones, round post huts, mammoth-bone circles, brush huts and, for farmers, longhouses.
- **Proposals for the owner:** new wording for `MIL-09` (one wording with study 7), `PRE-46` and `PRE-27` (with studies 1 and 3), and changes to A5.3 and A6 (section 6.11).

## 2. What `PROJECT.md` asks

- **`MIL-09`:** the engine goes "well beyond the art book's pictures", with "the model kit and its textures made by code (`PRE-46`, `PRE-42`, `PRE-43`)". The method is lifted for this round.
- **`PRE-46`, the model kit:** "Everything in the world is drawn from one fixed kit, so the content stays countable." Ground cover "is drawn by the patch (`WLD-31`)"; about 8 plant forms, 6 animal body patterns and one figure with about 8 kinds of garment; "only about 12 signs, also the patterns of `PRE-43`, are drawn by hand."
- **`PRE-27`:** "Small 3D figures of tiny blocks" (open this round), "posed about 10 times a second (`PRE-44`)", with "their strongest feeling on its face (`MND-19`)"; done when "build, age and clothing tell people apart."
- **`PRE-44`:** about 45 movements, "each a loop of 2–6 key poses"; variants "are a few rules that bend any movement, never new animations"; animals have one set per body pattern.
- **`PRE-42` and `PRE-43`:** parts take their materials' colours and shapes; "No two things look quite alike", and each thing "looks the same each time".
- **`MAT-07`:** "routes differ in what they need and how they look (`PRE-42`)", done when "at least two routes each to fire and to huts appear." The owner's several shelter types (answer 13) belong here.
- **`PRE-03` and `PRE-28`:** one continuous zoom; far away, people and animals become "tiny outlined figures in their strongest colours". The person stop's numbers are being reworded by study 1.
- **Rules that stay:** `PRN-10` ("Nothing is added for show"), `PRN-06` (language models never decide), `SCP-20` ("Nothing is copied from real cultures"), `PRC-01` ("no running costs beyond the AI sessions"), and the Stone Age from archaeology (A5.2, rule 8).

## 3. Where we stand

### 3.1 What round 1 and pre-production found

- **My round-1 note** kept the kit made by our C++ code at load (A6.1), added a kit lab for looking at content, and offered three looks, from strict pixel art (A) to fully painted (C). It proposed widening `MIL-09` so that approved pictures, processed by code, could supply painted detail, as Spore laid code-made paint over hand-painted textures. It found no place for Blender or AI 3D generators, and kept movement as key poses in data.
- **The other studies of round 1:** the liked pictures' brightest colours are golden, a quarter of each is dark, and texture is quiet (study 1); the liked camp keeps most of its look on a true grid of 64 colours (study 2); a layout of C++ families over Godot's RenderingServer (study 3); density, not drawing small, is what costs on the phone (study 4); a target card and quick A/B choices (study 6); a text-first pipeline with a look lab (study 7).
- **Pre-production** built everything by code: the art book's painter, and P3's 11 shapes, 2 plants and a deer from catalogue numbers in about 23 ms. The owner's comments on P3 included "figures in their own clothes, not one brown" (`LESSONS.md`).
- **Round 2 so far** (notes written before the owner's answers): study 3 keeps one path for block and detailed figures (skinned copies posed in C++ at 10 a second) and draws anything under about 6 art pixels across in the ground's texture; study 4 puts textures at about 64 texels a metre and finds detail costs most in triangles and see-through leaf layers; study 7 proposes three texture routes and the rule that a texture holds only its material's background, while the world places paths, flowers and stones as things.

### 3.2 The owner's answers that bear on content

| Answer | What it means for content |
|---|---|
| 1, 2: direction B, texels about 2 × 2 screen pixels | Textures are pixel art at about 67 texels a metre at the closest zoom; shapes are smooth at full resolution. |
| 3, 14: people and animals detailed | `PRE-27`'s "tiny blocks" give way to shaped figures; detail lives mostly in textures. |
| 4: small figures far off drawn to read | A design for each size: outline, light face and tunic, enlarged tool. The same rule fits plants. |
| 5: closest zoom as in the liked picture | An adult is about 200 screen pixels (about 100 texels) tall; a head about 14 texels tall. |
| 6, 7: "As I liked" | The density, airy plants, fine grain and darkening under things all stay; savings must not show. |
| 13: shelters, "Have multiple types that they can choose from, all of these are nice" | Several shelter layouts, each checked against excavations. |
| 15: surfaces A, C, D accepted; B not | Code-made ground from an approved picture, swatches from words, and swatches redrawn from the liked picture are all valid sources. GPT's own seamless ground repaint was not picked. |
| 19, 20, 22: family, deer poses, birch style accepted | GPT sheets can set a people's look, an animal's still poses and a species' style. Walking is animated by code. |
| 21: small plants lose their charm at game size | A problem to solve, not to cut. |
| 27: people found only with effort in a busy autumn wood | Content can help: clothing values and colours, and figures' detail (section 6.5). |
| 28, 29: the first cave and the truthful village keep the feeling | Stone Age truth costs no feeling. |
| 31: ground turns to speckle at the close camp | Textures need a design per zoom band, not just shrinking. |
| 32: rock without its layers can't be judged | Rock's look comes from layers laid on by code (study 7). |

### 3.3 The current design, and what is weak or missing for direction B

- **A6.1:** "Each form's shared shape is built at load by our C++ in `view/` as an `ArrayMesh`" and people come "from one block figure". A6.3: "our C++ poses the rigid parts, with no skeletons." Detailed figures need bodies that bend at the joints.
- **A5.3:** "Every texture is drawn at 16 texture pixels a metre", and "Grass, reeds, flowers and flames are sized in metres, so they keep their true size at every zoom." At the owner's closest zoom textures need about 67 texels a metre, and keeping things only "in metres" is exactly what makes small plants lose their charm: a 30 cm tuft is 40 screen pixels tall at the closest zoom and 9 at the close camp, drawn from the same design.
- **Missing:**
  1. a design for each zoom band for anything small: plants, flowers, leaves, faces, tools in hand, and the marks in ground textures;
  2. a way to make detailed figures and animals by code, with weights and textures;
  3. a vetting step for every picture, since pictures now feed the game;
  4. a list of shelter types tied to excavations;
  5. terms of use for pictures used as sources.

## 4. What others do

### 4.1 Small things at small sizes: draw them for their size

- **Type designers draw each size.** Adobe, on giving Source Serif optical sizes (2021): "all formal design decisions — not just spacing, but also letter proportions and shape details — have been tailored to a specific application size." It describes the change from "the sturdy, simplified details and optical compensations visible in the Caption style" to "an elegantly swelling high-contrast variant with fine details in the Display style."
- **Icon designers too.** Microsoft's app-icon guidance asks for "a silhouette that’s distinctive, yet legible at small sizes", and "When adding detail, care should be taken to maintain legibility at small sizes." Windows ships each icon at several sizes because "Including more icon sizes with your app means Windows will more often have a pixel-perfect match, and reduce the amount of scaling applied to scaled icons."
- **Pixel artists abstract plants.** Raymond Schlitter (Slynyrd, 2019): "When it comes to pixels, you’ll rarely have the scale/resolution to explicitly depict the leaves of a specific species, especially in dealing with dense foliage, but it’s always good to have a reference point and abstract as needed." And: "Not every leaf needs to be represented. Just enough to give the impression of a leafy texture and the mind will automatically fill in the gaps where there are less defined large clusters." He warns: "Too much variation will create displeasing noise."
- **The best automatic shrinking still falls short.** Kopf, Shamir and Peers (SIGGRAPH Asia 2013, old but still the reference) built a downscaler that keeps fine detail, and note that ordinary shrinking "exhibits washed out colors due to excessive smoothing". Even so: "our results cannot reach the quality that well-trained experts achieve when manually hinting fonts and manually creating pixel art". Their method is a costly optimisation, and they say it does not always do better on images that contain "structured textures".

### 4.2 Plants and ground cover in 3D

- **Cut-out leaves vanish with distance unless their mipmaps keep coverage.** Ignacio Castaño, on the trees of The Witness (2010, old, still standard): "As the trees moved farther away from the camera, the leafs faded out becoming almost transparent." The cause: "each mipmap has a different alpha test coverage." The fix: "find a scale factor that preserves the original alpha test coverage as best as possible."
- **Godot 4.7.2 does not keep coverage on its own.** Its mipmaps are plain averages of four texels (`core/io/image.cpp`), and its texture importer has no coverage option. Its materials do offer alpha to coverage. With the filter `TEXTURE_FILTER_NEAREST_WITH_MIPMAPS` the chip "blends between the nearest 2 mipmaps" unless a project-wide setting picks the nearest one, which "will result in visible seams appearing between mipmap stages", and "This property is only read when the project starts." Hand-made levels can be loaded: `Image.create_from_data` "loads the mipmaps for this image from" the data given.
- **A Godot grass experiment** (GodotGrass, MIT licence) swaps blade density and mesh by distance, borrowing from *Ghost of Tsushima*: "Grass blades which are perpendicular to the camera are stretched horizontally in view space as suggested in the "Ghost of Tsushima" GDC talk." Its honest caution: "Unfortunately, LOD swapping is very noticable due to the tiled nature of the system." The talk itself (GDC 2021) was not readable here; its claims are unverified.

### 4.3 Characters: simple shapes, rich textures, bodies from skeletons

- **Hytale** (art director's guide, December 2025) builds models from "2 primitives: Cubes (6 sides) Quads (2 sides)" and paints the detail in textures. Characters get twice the texture density of props ("Density will be 64px per unit" against "Density will be 32px per unit"), because "Higher density for characters helps them detach from the environment and enhances the overall readability of the scene". It also says: "We avoid noise, too much grain, or perfectly flat surfaces."
- **Spore** made creature bodies by code around a skeleton. Chris Hecker: "I chose a blobby implicit surface (sometimes called metaballs) to represent the skin." And: "We generate bone weights for the vertices based on which body parts generated which metaballs. This works well for limbs, but sometimes big spine segments don't generate smooth weights, and the torso on fat creatures can shear." Its hand-made parts came from an artist who "would be given a simple pencil sketch of what the rigblock should look like" (Jane Ng). Its gaits were numbers per foot: "The duty factor is the fraction of the duration of the overall gait cycle time that the foot is on the ground" (SIGGRAPH 2008).
- **Godot's own note on crowds:** with skeletons, "bones are animated on the CPU and so you end having to calculate thousands of operations every frame and it becomes impossible to have thousands of objects" (a 4.7 tutorial page marked outdated). Godot 4.7.2 re-skins a mesh only when its skeleton changes (`mesh_storage.cpp` compares the skeleton's version), so poses held for a tenth of a second cost little. Study 3 covers how crowds are drawn.

### 4.4 AI-made content, and its limits

- **Everything by code:** Claude-of-Duty (2026) has "no art assets", and admits "Surfaces read as procedural noise rather than photographed" and "Enemies read as mannequins at distance."
- **Image models and grids:** "Current AI image models can't understand grid-based pixel art", and "The grid resolution can drift over time" (Pixel Snapper's readme). My native-size test confirms it for GPT (section 5.2).
- **Claude's own eye:** "Claude might hallucinate or make mistakes when interpreting low-quality, rotated, or very small images under 200 pixels"; "Animations are unsupported, and only the first frame is used"; and counts are approximate, "especially with large numbers of small objects." Small designs must therefore be checked enlarged and by code, and movement as filmstrips.

### 4.5 What excavations show of shelters and clothing

| Shelter type | What was excavated | Kit layout | Source |
|---|---|---|---|
| Hides across a rock shelter or cave mouth | Magdalenians used shelters and cave mouths; "All they needed to do was close off the habitat with light structures and hides – possibly using loops pierced in the walls." | Poles and hides fixed to the rock | French Ministry of Culture |
| Small round tent or hut of skins held down by stones | "small circular residential structures (tents or huts) that were covered with skins held to the ground by stones" (Pincevent, Étiolles); at Gönnersdorf "Brace-holes for a lightweight frame, probably made of wood" | Ring of poles, as a cone or a dome; hide cover; stone ring; hearth in the middle | French Ministry of Culture |
| Round post hut | Mount Sandel (Ireland; dated by its excavator to the mid 7th millennium BC, uncorrected radiocarbon): "round huts approx 6 metres across with central hearths", winter huts "large and more substantial than the flimsy summer constructions"; Star Carr (at least 8,500 BC): a "3.5 metres circular structure" that "had post holes around a central hollow which would have been filled with organic matter such as reeds, and possibly a fireplace" | Posts or bent rods in a ring; bark, reed or hide cover | Woodman's excavation reports; University of York |
| Mammoth-bone circle | "Circular features made from mammoth bone are known from across Upper Palaeolithic Eastern Europe, and are widely identified as dwellings"; a 2020 study reopens their function | Ring of bones and tusks under hides, only where mammoths lived | *Antiquity* (2020) |
| Brush hut | Ohalo II, 23,000 years ago, preserved "organic traces of huts or shelters in which people lived" | Brush and branches over a shallow oval floor | *PNAS* (2004) |
| Longhouse (farmers) | "thick oak posts sunk deep into the ground", roofs "covered with straw, reeds, tree bark or other available materials", walls of wickerwork with "clay, or daub" | Post rows, wattle walls, thatch or bark roof | A German museum's audio guide |

**What this means:** excavations show the floor (stone rings, post holes, hearths, bedding), not the roof. So a cone or a dome over a ring of stones are both fair readings, which fits the owner's "all of these are nice". The crossed pole tips of the liked tents follow from how a cone of poles is tied; what must not be borrowed are a present-day people's details, such as smoke flaps, painted covers or beadwork (`SCP-20`).

**Clothing:** the Iceman's "clothing was made solely from leather, hide and braided grass"; his coat "was made from light and dark strips of goat and sheep hide stitched together with animal sinews", a natural pattern with strong contrast. And: "The mat, made from alpine swamp grass, was initially thought to be a grass cape." (South Tyrol Museum of Archaeology.)

### 4.6 Terms of use for GPT's pictures

- **OpenAI's terms of 11 December 2024** (read from a copy, since openai.com refuses fetches): "you (a) retain your ownership rights in Input and (b) own the Output"; "output may not be unique and other users may receive similar output from our Services"; and users may not "Represent that Output was human-generated when it was not."
- **The current terms: unverified first-hand.** A third-party monitoring site (ConductAtlas, checked by it on 9 July 2026) shows the rest-of-world terms (last updated 5 March 2026) and the European terms (11 May 2026) both still say "We hereby assign to you all our right, title, and interest, if any, in and to Output." The Internet Archive holds a copy of 2 October 2026 that I could not reach. Which set applies depends on where the owner lives.
- **Copyright, in the United States** (Copyright Office, January 2025): "Copyright does not extend to purely AI-generated material, or material where there is insufficient human control over the expressive elements", and "prompts do not alone provide sufficient control." Other countries may differ; I did not check them. For a game made for the owner alone (`SCP-02`), this mostly means the pictures may be used but not owned exclusively. Keeping each picture's record (study 7) is worthwhile anyway.

## 5. The options

### 5.1 How to make the content, overall

| Way | Closeness to the feeling | On the phone | Effort for an AI builder | Risk | Fit with the rules that stay |
|---|---|---|---|---|---|
| **1. All by code** (round 1's look A) | Shapes yes; painted surfaces fall short ("procedural noise") | Smallest | A painter per material class | Looks generic | Full |
| **2. Code kit, surfaces from three routes** (code, the world, approved pictures prepared by code) | Close: the owner accepted all three kinds of surface (answer 15) | Textures about 13 to 50 MB (study 7) | Medium: requests, vetting, preparation, records | Style drift, painted light, truth errors | Needs `MIL-09` widened |
| **3. Way 2, plus a design for each zoom band for everything small** | Closest: keeps the charm at every zoom (answers 21, 31) | No more memory: hand-made mipmaps replace computed ones | Way 2 plus about 100 small designs and a rule per material class | Swaps between bands could show while zooming | Needs A5.3 changed |
| **4. Pictures as finished art** (a painted sprite per view) | Best in one view, wrong from others | Many textures | Every angle, stage and season painted | No variety, truth hard to control | Breaks `PRE-42`, `PRE-43`, `PRE-46` |
| **5. Outside 3D tools** (Blender scripts, AI 3D generators) | Better organic anatomy, but at 100 texels tall the detail is in the textures | More triangles | A second toolchain; generators need large graphics cards and paid keys | Welded parts, painted-in light | Breaks the code kit's variety |
| **6. Paid pixel-art generators** (they hold a grid at small sizes) | Good small sprites | Small | Low | New terms; a fixed set | Breaks `PRC-01` (running costs) |

### 5.2 Small things across the zoom

| Way | What I found | Verdict |
|---|---|---|
| **A. Shrink a detailed painting** | The owner's verdict: no charm (answer 21). GPT drew each plant at the same picture size whatever its real height, so a 30 cm tuft is shrunk four times more than 1.5 m reeds. At the closest zoom the result is fair; at 16 m and the close camp it is mush, with an orange fringe from GPT's soft edges. | Only for the closest band |
| **B. A code filter per band** (few shades, the commonest shade per texel, lone texels removed) | On eight accepted swatches it cut speckle at the close camp from 3 to 15% of texels to 0.2 to 3.4%, but by eye it turned pebbles, cracks, flowers and moss into camouflage blotches. | No: a filter is not a design |
| **C. GPT paints each band at its true size** | It redrew each size with fewer, bolder parts (daisies about 7, 4 and 3 flowers; bilberries about 12, 5 and 1 berries), and the accents read at the smallest size. But it drew about three times larger than asked, on no regular grid. | Good as a guide, not as the design |
| **D. Each band designed at its true size,** by code rules and tiny pixel designs, guided by C | With the same reduction, designs made for their size kept more accents: at the close camp 5 white daisy texels against 1 and 2 berry texels against none; at 16 m 18 against 8 and 11 against 5. Blades and fronds gained little: at 4 to 10 texels, only placing each texel on purpose works. | **Recommended** |

### 5.3 Figures

| Way | Closeness to the liked figures | Effort | Notes |
|---|---|---|---|
| Blocks (decided `PRE-27`) | Not chosen (answer 3) | Lowest | Kept as the far form |
| **Body shaped by code around the skeleton,** weights from the bones, garments as shells, face drawn per band | Close: proportions from the accepted family sheet, detail in textures | Medium | Spore's way; tidy texture layouts because the rings follow the bones |
| Metaball skin (Spore exactly) | Close | Medium | Smooth, but its surface needs texture layouts worked out afterwards, and Hecker reports shearing torsos |
| A sculpted base mesh from outside (Blender, AI 3D, a library) | Closest anatomy | High; outside tools and licences | Loses build and age by code unless reshaped |

**In short:** way 3 keeps what the owner liked at every zoom while keeping the kit countable; way D is how it handles small things; a body shaped by code is how figures stay one kit. Ways 4 to 6 each break a rule that stays.

## 6. What we'd recommend

### 6.1 The method, in one line

**Code builds the kit and moves it; three routes paint its surfaces; every small thing is drawn for each band it is seen at; and every picture is a checked guide or source, never finished art.**

| Content | Shape and movement | Surfaces | Small bands |
|---|---|---|---|
| Ground and rock | The world (`WLD-12`) | Background from an approved picture or code; layers, soot, wet and snow from the world (study 7) | Calm background per band, plus marks designed per band |
| Ground cover | Placed by the patch (`WLD-31`) | Each form's designs per band, recoloured by species and season | Designed (way D) |
| Trees and bushes | Branches grown by code | Bark from sources; leaf clusters designed per band | Designed |
| People | One skeleton, body by code, garments as shells | Hide, fur and hair from sources; faces designed per band | Faces and held tools designed |
| Animals | One skeleton per body pattern, body by code | Coats from approved sheets; markings by code | Far forms drawn to read |
| Made things and shelters | Layouts of parts (`PRE-46`) | Material textures; wear and style by code | Small parts dropped or drawn per band |
| Movement | Key poses as data; gaits as numbers | n/a | Poses read at true size (`PRE-44`) |

### 6.2 Designs for each band

- **The bands:** the closest zoom (about 67 texels a metre), about 16 m across (about 34), and the close camp (about 17). Each is half the one before, so each band's design can be one level of a hand-made mipmap chain, with no extra memory.
- **What each band holds:**
  - *closest:* the full design, from approved pictures prepared by code (fair, as section 5.2 found);
  - *16 m:* half the parts, one texel wide, light tips and dark bases kept;
  - *close camp:* the silhouette, a light top, a dark base and the accents, which stay at least one texel in their own clean colour even if that is larger than true scale (as the owner chose for far figures, answer 4).
- **Who designs them:** code rules for repeated structure (blades, fronds, leaf clusters, cracks, pebbles), and tiny pixel designs written as data where charm needs a placed texel (flower heads, berries, eyes, held tools). GPT sheets like `plants-native-size.png` say what to keep at each size. About 8 plant forms × 3 bands × 4 variants is about 100 small designs, recoloured by species and season (estimate).
- **How they reach the screen:** as hand-made mipmap levels, or by swapping designs at band edges. Left to itself, the chip moves to the next level only when texels shrink below one screen pixel, so keeping them near 2 × 2 means choosing each level one step later. How the engine shows the change between bands (blending two levels, picking one, or swapping) is study 2's to choose and study 4's to cost. At the close camp, small plants under study 3's 6-art-pixel line become marks in the ground's band texture, set from each patch's kinds, density and season, so nothing is faked (`PRN-10`).
- **Cut-outs:** edges hardened by code (GPT's transparent mode gives a soft edge: 99.9% of the plant's texels partly see-through and a faint halo), colour bled into the see-through texels, and each level's alpha scaled to keep the source's coverage (Castaño). My reduction kept coverage within 0.006 of the source at every band, where a plain average drifted by up to 0.07.

### 6.3 Textures

- **Routes:** as study 7 proposes: code, the world, or an approved picture prepared by code; each texture holds only its material's background, and paths, flowers and stones are placed things.
- **What GPT gives, measured on my sheets:**
  - in the liked style, and repeatable: a second request gave the same six materials' average lightness within 9 levels in 255 (grass 107.3 against 107.4; hide the furthest, 154.2 against 145.3);
  - swatches redrawn from the liked picture hold a much clearer pixel grid than swatches from words (my grid measure 1.3 to 1.9 against 1.06 to 1.15, where 1 means none);
  - none tiles: the step across each seam is 2.7 to 21 times the step inside;
  - corners darken despite the prompt, by up to 54 levels on hide and birch, and a few carry a strong light slope (granite and birch about 0.7 of their own contrast);
  - GPT copies its input's mistakes: the hide's repeating cross pattern came back after I asked for it to go.
- **So code prepares every source:** seams made by code, the vignette and light slope removed (divide by a heavily blurred copy), colours fitted to the material's range under the engine's light (study 7), and repeats broken by mixing two to four maps by seed.
- **Consistency across hundreds of materials:**
  - every request uses the liked picture or an accepted sheet as its style input, with the same wording;
  - each material's lightness, contrast and grid are recorded and compared in the look lab;
  - seasons come as matched sets: the spring, summer, autumn and winter grass of one sheet share clump shapes.
- **Per band:** the background is kept calm at the close camp (few broad patches, no lone texels), and its readable marks (pebbles, cracks, flower dots) are designed for the band, not filtered.

### 6.4 Plants

- **Trees and bushes:** branches grown by code for each species, stage and a few variants, as round 1 proposed. Leaf clusters are cut-out cards designed per band, each with a season value. The accepted birch sheet (answer 22) is the style guide; one sheet per species sets its crown, colours by season and growth stages.
- **Ground cover:** designed per band (section 6.2), with each patch's kinds, density, season and ripeness from `WLD-31`.
- **Density as liked (answer 6):** stays, with the savings coming from ways that do not show, such as marks in the ground's band texture at the close camp (study 3) and fewer parts per band.

### 6.5 People

- **One figure on one skeleton, made by code** (answers 3, 19):
  - the body is built as rings around each bone, like sleeves, widened by build, age and sex from catalogue numbers; each ring follows its bone, and rings near a joint follow both bones in part, so the joint bends smoothly;
  - garments are shells over it (8 kinds, child and adult sizes); hair, beads and paint follow each people's style (`CUL-12`);
  - about 1,000 to 1,500 triangles at the closest zoom (estimate), fewer further out.
- **Proportions by age** come from the accepted family sheet, read by the builder and set as numbers.
- **Faces drawn per band:**
  - at the closest zoom a head is about 14 texels tall, between the 8- and 16-pixel rows the owner saw (question 5b);
  - there the 12 states of my faces sheet (the 7 feelings, calm, asleep, hurt, dead, and exhausted with cold) are small pixel designs, drawn for that size, not shrunk;
  - at 16 m they keep eyes, brows and mouth as single texels;
  - at the close camp, feeling moves to the pose.
- **Clothes and colour:** within what the archaeology shows, garments vary in material and colour (pale and dark hide, light and dark fur, striped coats like the Iceman's), not one brown. That also helps people stand out from busy ground (answer 27). One more option for the owner to judge: give figures finer texels than the world, as Hytale does for readability.
- **Truth:** each garment is checked against excavations. The grass cape against rain in my family sheet rests on an interpretation the Iceman museum has dropped, so it stays out until other evidence is found.

### 6.6 Animals

- **Six body patterns, each one skeleton,** with bodies made by code in the same way as people. A species is its proportions, colours and markings; small generators make antlers (tines by age), horns, tusks and manes.
- **Coats** come from approved sheets such as the accepted animals sheet (answer 14), prepared by code, with markings varied by seed.
- **Movement:**
  - gaits are numbers per leg (how long each foot stays down, and when it lifts), so walk, trot and gallop come from numbers, and a limp is a layer;
  - feet are planted by two-bone solving; birds and fish move by key poses and waves along the body;
  - still poses (feed, drink, rest, sleep, call, fight, play, fall) follow approved pose sheets, as the deer's did (answer 20).

### 6.7 Made things and shelters

- **Layouts of parts** from shared shapes, as `PRE-46` says, guided by sheets such as the made-things sheet, whose layouts were right even where its materials were wrong.
- **Shelters:** one layout per type in section 4.5, so that the routes to huts (`MAT-07`) look different. Each type records what excavation it rests on and what is reconstruction: cone or dome over a stone ring, post hut, bone circle, brush hut, and later the longhouse. Windbreaks and lean-tos keep the owner's P3 comments: poles, bar and brush; a roof in overlapping courses.

### 6.8 Pictures: targets, guides and sources

- **Targets** show the feeling; **guides** show designs to follow (proportions, poses, species, what to keep at each size); **sources** become textures. None ships as drawn.
- **Every picture is checked** before it is used:
  - materials (no metal before copper; GPT drew metal tools despite "no metal");
  - clothing against finds;
  - shelters against excavations;
  - borrowed real-culture details (`SCP-20`);
  - later things.

  The check is noted in the picture's record (study 7).
- **Budget:** about 100 to 200 pictures for the launch kit (estimate): one sheet per species, garment set, body pattern, material family and plant form, with retries.

### 6.9 Movement

- Key poses are joint angles in data, 2 to 6 a movement, held 10 times a second (`PRE-44`).
- Rules bend them (stoop, limp, slump, hunch), and seeds offset their timing.
- Gaits are numbers per leg.
- The builder reads still poses from approved sheets; Claude's eye cannot judge motion, so the look lab shows filmstrips.

### 6.10 What M2 builds first

1. **The look lab's sheets per band:** every entry at true size and enlarged, at the three bands.
2. **One meadow strip, three ways, on the phone** (shrunk, filtered, designed per band), beside the liked picture: the answer to question 21.
3. **One figure** at three ages, with three garments and the faces at the closest zoom, beside the accepted family sheet.
4. **One deer** walking, trotting and galloping by numbers, with the accepted still poses.
5. **Two shelter types** in two materials (`PRE-42`'s Done when).

### 6.11 Proposals for the owner (`PRC-07`)

1. **`MIL-09`** (one wording with study 7): "…and the model kit made by code (`PRE-46`, `PRE-42`, `PRE-43`), its textures made by code, by the world, or prepared by code from pictures you approved, each traced to its picture, prompt and your approval, and every small thing drawn for each size it is seen at." The rest of `MIL-09`'s look words are studies 1 and 2's.
2. **`PRE-46`, two additions:**
   - "**Sizes:** anything small (ground cover, flowers, leaves, faces, held tools) has a design for each band of zoom, drawn for that size and never only shrunk; flowers, berries and eyes stay at least one texture pixel."
   - "**Shelters:** several types, each a layout of parts true to what excavations show, so the routes to huts (`MAT-07`) look different."

   The hand-drawn list grows from "about 12 signs" to include the small designs of each band (about 100) and the faces.
3. **`PRE-27`** (with studies 1 and 3, whose wording I support): study 3 proposes "3D figures as detailed as in the pictures the owner liked, with a separate head, torso, arms and legs that bend at the joints, posed about 10 times a second (`PRE-44`), so they look like crisp pixel art from any angle", and study 1 a close wording; I would add: "with a face drawn for each band of zoom."
4. **A5.3:** "Textures hold about 67 texels a metre where a material is seen closest (about 2 × 2 screen pixels each), with a design for each band of zoom as hand-made mipmap levels; grass, reeds, flowers and flames keep their size in metres, but their design follows the band, and accents never fall below one texel." Studies 3, 4 and 7 propose close wordings; one should be chosen.
5. **A6.1 and A6.3:** bodies made by code around one skeleton per body pattern, weights from the bones; gaits as numbers per leg; still poses may follow approved pose sheets.
6. **A5.2, rule 8, strengthened:** every target, guide and source picture is checked for Stone Age truth and borrowed dress, and the check is recorded.

## 7. Questions only the phone can answer

1. **Do small plants keep their charm at true size?** A meadow strip on the phone at the three bands: shrunk, filtered and designed per band, at noon; the owner's verdict.
2. **Do band changes show while pinching?** A scripted pinch through each band edge: count changed pixels per frame there, and the owner's eye.
3. **Do cut-out plants thin out with distance?** Screenshots at each band; the share of plant pixels with and without coverage-kept mipmaps.
4. **Can people be found at a glance in a busy autumn wood?** Time to find every person in A/B scenes: one-brown clothes against varied clothes, and figures at the world's texel size against finer.
5. **Do faces at about 14 texels show their states?** The owner names the 12 faces from a sheet on the phone at true size.
6. **Do bodies made by code bend cleanly** at 10 poses a second under a moving camera? Filmstrips in the cloud, then the owner's eye.
7. **How long does the kit take to build at first start** with all bands, and how much memory does it use? It must stay inside `PLT-04`'s 3 seconds to open a world.

## 8. Risks

1. **Designs per band triple the drawing work.** *Retire early:* time one plant form and one material through all three bands in M2's first content step; use code rules wherever structure repeats.
2. **GPT cannot draw on the true small grid.** *Retired here:* `plants-native-size.png` shows it; plan for code rules and written designs at the small bands.
3. **Pictures bring in Stone Age errors** (metal tools, a dropped grass cape, present-day tent details). *Retire early:* the checklist in section 6.8 on every picture, with the result in its record.
4. **Bodies made by code look like mannequins.** *Retire early:* one figure at three ages beside the accepted family sheet, judged by the owner on the phone.
5. **Filters pass for designs.** *Retired here:* the filtered close-camp ground lost its meaning; judge each band by eye, not by a speckle count alone.
6. **Terms of use change,** or the owner's country treats AI pictures differently. *Retire early:* the owner reads the current terms where they live; records kept for every picture.
7. **Cut-out plants cost too much on the phone.** *Retire early:* study 4's stress scenes; fewer parts per band also helps.
8. **GPT's allowance limits production.** *Retire early:* a picture budget per step (study 7), and code where code is enough.

## 9. For other studies

- **Study 1 (the look):**
  - at the owner's closest zoom an adult is about 100 texels tall and a head about 14;
  - "drawn for its size" is the rule behind their far-figure choice, and it applies to plants and faces too;
  - Hytale's finer texels for characters are a readability option for busy scenes (answer 27);
  - `PRE-27`'s wording (section 6.11).
- **Study 2 (drawing):**
  - band designs as hand-made mipmap levels; in Godot 4.7.2 picking the nearest level is a project-wide setting read at start, so per-material control means sampling levels ourselves in our shaders;
  - blending two different designs can ghost;
  - coverage-kept alpha for cut-outs, and the view-space stretching of thin blades from *Ghost of Tsushima* (via GodotGrass).
- **Study 3 (engine):**
  - bodies made by code with weights from the bones (Spore's way) fit its one figure path;
  - faces as texture layers per state and band;
  - at the close camp, ground-cover marks belong in the ground's band texture, which fits its 6-art-pixel rule.
- **Study 4 (phone):** hand-made mipmaps add no memory; cut-out cards with hard edges and coverage-kept levels; about 1,000 to 1,500 triangles per detailed figure at the closest zoom (estimate).
- **Study 6 (the loop):**
  - the "shrunk against designed" meadow strip is a ready A/B for the owner;
  - GPT design sheets are guides, judged at true size;
  - the vetting checklist joins the target card.
- **Study 7 (pipeline):**
  - hand-made mipmap levels are data, loaded with `Image.create_from_data`;
  - GPT's transparent mode gives soft edges, so harden them and bleed colour into the see-through texels;
  - my swatches never tile and their corners darken;
  - a filter is not a band design;
  - one wording for `MIL-09`.

## 10. Pictures from GPT

All 13 were run by the builder through Codex on the owner's plan; requests and logs sit beside each picture in `R/work2/5/gpt/`. Unless noted, each used the liked picture as its style input.

1. **`textures-a.png`:** *can GPT make flat, seamless swatches in the liked style from words?* Twelve materials, in style and quiet. None tiles; some corners darken (granite, hide, birch); the limestone came out as a masonry wall, the hide with a repeating cross pattern, and flowers were painted into the grass. The owner accepted it as a surface source (answer 15, C).
2. **`person-turnaround.png`:** *does one person stay the same across views and poses?* Yes: face, clothes and proportions held across four views and four poses. Shrunk by code, its silhouettes and garments held at small sizes; set beside the art book's block people (question 3b), it helped the owner choose detailed people (answer 3).
3. **`birch-sheet.png`:** *one species through stages and seasons?* Consistent; the owner found its style right (answer 22). Cutting out by a magenta key failed: magenta reached 94% of edge pixels.
4. **`animals-sheet.png`:** *the body patterns as one set?* True to each species and consistent; the owner chose detailed animals from it (answer 14).
5. **`made-things-sheet.png`:** *Stone Age things, when told exactly what to draw?* The layouts were right (stone-ringed cone, bark dome, windbreak, hearth, hide frame, rack, containers, dugout), but the axe head, spear point, arrowheads and blades came out as shiny metal after "no metal". The owner welcomed all its shelter shapes (answer 13).
6. **`groundcover-sheet.png`:** *small plants as cut-outs in four seasons?* The seasons of each plant match, but magenta reached 82% of edge pixels. Shrunk to game size, they lost their charm (answer 21).
7. **`textures-b.png`** (input: textures-a): *repeatable, seasonal, and fixable by wording?* Average lightness repeated within 9 levels in 255; the four seasons of one ground matched; the cliff came out more natural, but the hide's cross pattern and the dark corners came back.
8. **`groundcover-alpha.png`** (transparent mode): *a real see-through background?* Yes, but soft: almost every plant pixel is partly see-through, with a faint halo and an orange fringe. Code must harden the edge.
9. **`faces-sheet.png`** (input: the turnaround): *which feelings survive at game size?* The same woman in 12 states, consistent. Shrunk by code, feelings partly read at 16 pixels wide and not at 8 or 4 (the owner saw this as question 5b).
10. **`delight-liked.png`** (input: a crop of the liked picture): *can GPT take the sun out of the liked picture's own materials?* Six even, in-style swatches with a clearer pixel grid than those from words; none tiles; the hide's corners darken. The owner accepted it (answer 15, D).
11. **`people-variety.png`** (input: the turnaround): *one people across ages, builds and garments?* Yes, and they tell apart at 200, 90 and 40 screen pixels (answer 19). But everyone wears one tan, and the grass cape against rain rests on a dropped interpretation.
12. **`deer-poses.png`** (input: the animals sheet): *can GPT draw motion?* The still poses read and the owner found them right (answer 20). The walk repeats one side's two poses instead of four beats, and the dead deer, asked for "lying on its side, legs out", came out lying on its belly. So cycles come from numbers and pictures guide the still poses.
13. **`plants-native-size.png`:** *can GPT paint small plants at their true game sizes, each redrawn for its size?* It redrew each smaller size with fewer, bolder parts, and the flowers and berries still read at the smallest. But the top row has no regular grid, and every size came out about three times larger than asked (the tuft 327, 192 and 86 picture pixels tall against 120, 60 and 30). Brought to true size, its designs kept more flowers and berries than shrunk paintings; blades and fronds did not gain. GPT can guide designs per band but not draw them.

## 11. Sources

**The owner and the repository** (read 6 October 2026):
- `R/owner/ask/answers.md`: answer 13, "Have multiple types that they can choose from, all of these are nice"; answers 6 and 7, "As I liked".
- `PROJECT.md`:
  - `MIL-09`: "well beyond the art book's pictures"; "and the model kit and its textures made by code (`PRE-46`, `PRE-42`, `PRE-43`)."
  - `PRE-46`: "Everything in the world is drawn from one fixed kit, so the content stays countable."; "is drawn by the patch (`WLD-31`)"; "only about 12 signs, also the patterns of `PRE-43`, are drawn by hand."
  - `PRE-27`: "Small 3D figures of tiny blocks"; "posed about 10 times a second (`PRE-44`)"; "their strongest feeling on its face (`MND-19`)"; "build, age and clothing tell people apart."
  - `PRE-44`: "each a loop of 2–6 key poses"; "are a few rules that bend any movement, never new animations".
  - `PRE-43`: "No two things look quite alike"; "looks the same each time".
  - `MAT-07`: "routes differ in what they need and how they look (`PRE-42`)"; "at least two routes each to fire and to huts appear."
  - `PRE-28`: "tiny outlined figures in their strongest colours".
  - `PRN-10`: "Nothing is added for show."; `SCP-20`: "Nothing is copied from real cultures"; `PRC-01`: "no running costs beyond the AI sessions".
- `ARCHITECTURE.md`:
  - A6.1: "Each form's shared shape is built at load by our C++ in `view/` as an `ArrayMesh`"; "from one block figure".
  - A6.3: "our C++ poses the rigid parts, with no skeletons."
  - A5.3: "Every texture is drawn at 16 texture pixels a metre"; "Grass, reeds, flowers and flames are sized in metres, so they keep their true size at every zoom."
  - A5.2, rule 8: "The Stone Age from archaeology".
- `LESSONS.md`, P3: "figures in their own clothes, not one brown".
- Round 1's notes (`R/round1/notes/`) and round 2's notes of studies 3, 4, 6 and 7 (`R/notes/`).

**Drawing small things:**
- https://blog.adobe.com/en/publish/2021/03/04/source-serif-gets-optical-sizes (Frank Grießhammer, 4 March 2021):
  - "all formal design decisions — not just spacing, but also letter proportions and shape details — have been tailored to a specific application size."
  - "the sturdy, simplified details and optical compensations visible in the Caption style"
  - "an elegantly swelling high-contrast variant with fine details in the Display style."
- https://learn.microsoft.com/en-us/windows/apps/design/iconography/app-icon-design:
  - "a silhouette that’s distinctive, yet legible at small sizes"
  - "When adding detail, care should be taken to maintain legibility at small sizes."
- https://learn.microsoft.com/en-us/windows/apps/design/iconography/app-icon-construction: "Including more icon sizes with your app means Windows will more often have a pixel-perfect match, and reduce the amount of scaling applied to scaled icons."
- https://www.slynyrd.com/blog/2019/3/7/pixelblog-15-plant-life (Raymond Schlitter, 19 March 2019):
  - "When it comes to pixels, you’ll rarely have the scale/resolution to explicitly depict the leaves of a specific species, especially in dealing with dense foliage, but it’s always good to have a reference point and abstract as needed."
  - "Not every leaf needs to be represented. Just enough to give the impression of a leafy texture and the mind will automatically fill in the gaps where there are less defined large clusters."
  - "Too much variation will create displeasing noise."
- https://johanneskopf.de/publications/downscaling/paper/downscaling.pdf (Kopf, Shamir, Peers, SIGGRAPH Asia 2013):
  - "exhibits washed out colors due to excessive smoothing"
  - "our results cannot reach the quality that well-trained experts achieve when manually hinting fonts and manually creating pixel art"
  - "structured textures"

**Plants and ground cover in 3D:**
- https://ludicon.com/castano/blog/articles/computing-alpha-mipmaps/ (Ignacio Castaño, first published on The Witness's site, September 2010):
  - "As the trees moved farther away from the camera, the leafs faded out becoming almost transparent."
  - "each mipmap has a different alpha test coverage."
  - "find a scale factor that preserves the original alpha test coverage as best as possible."
- https://github.com/2Retr0/GodotGrass (README):
  - "Grass blades which are perpendicular to the camera are stretched horizontally in view space as suggested in the "Ghost of Tsushima" GDC talk."
  - "Unfortunately, LOD swapping is very noticable due to the tiled nature of the system."
- https://gdcvault.com/play/1027214/Advanced-Graphics-Summit-Procedural-Grass (Eric Wohllaib, GDC 2021): the talk's page only; its contents unverified.

**Godot 4.7.2** (https://github.com/godotengine/godot/tree/4.7.2-stable):
- `core/io/image.cpp`: mipmaps by `_generate_po2_mipmap` with `average_4_uint8` (a plain average of four texels); no coverage option.
- `editor/import/resource_importer_texture.cpp`: options `mipmaps/generate`, `mipmaps/limit`, `process/fix_alpha_border`; no coverage option.
- `scene/resources/material.cpp`: `alpha_to_coverage` and `alpha_to_coverage_and_one` for alpha scissor and alpha hash.
- `doc/classes/BaseMaterial3D.xml`: "blends between the nearest 2 mipmaps"; "This makes the texture look pixelated from up close, and smooth from a distance."
- `doc/classes/ProjectSettings.xml`, `rendering/textures/default_filters/use_nearest_mipmap_filter`: "which will result in visible seams appearing between mipmap stages"; "This property is only read when the project starts."
- `doc/classes/Image.xml`: "loads the mipmaps for this image from"
- `servers/rendering/renderer_rd/storage_rd/mesh_storage.cpp`: a mesh instance is updated when `sk->version != mi->skeleton_version`.
- https://docs.godotengine.org/en/4.7/tutorials/performance/vertex_animation/animating_thousands_of_fish.html (marked outdated): "bones are animated on the CPU and so you end having to calculate thousands of operations every frame and it becomes impossible to have thousands of objects"

**Characters:**
- https://hytale.com/news/2025/12/an-introduction-to-making-models-for-hytale (22 December 2025):
  - "When making models, we only use 2 primitives: Cubes (6 sides) Quads (2 sides)"
  - "Density will be 64px per unit"; "Density will be 32px per unit"
  - "Higher density for characters helps them detach from the environment and enhances the overall readability of the scene"
  - "We avoid noise, too much grain, or perfectly flat surfaces."
- https://www.chrishecker.com/My_Liner_Notes_for_Spore:
  - "I chose a blobby implicit surface (sometimes called metaballs) to represent the skin."
  - "We generate bone weights for the vertices based on which body parts generated which metaballs. This works well for limbs, but sometimes big spine segments don't generate smooth weights, and the torso on fat creatures can shear."
- https://www.cse.chalmers.se/edu/year/2011/course/TDA361/Advanced%20Computer%20Graphics/027-hecker%20copy.pdf (Hecker et al., SIGGRAPH 2008): "The duty factor is the fraction of the duration of the overall gait cycle time that the foot is on the ground"
- https://janeng.com/?p=143 (Jane Ng, 2013): "I would be given a simple pencil sketch of what the rigblock should look like, and then I would proceed to model and UV them."

**AI-made content:**
- https://github.com/mshumer/Claude-of-Duty: "There are no art assets."; "Surfaces read as procedural noise rather than photographed"; "Enemies read as mannequins at distance."
- https://github.com/Hugo-Dz/spritefusion-pixel-snapper (README): "Current AI image models can't understand grid-based pixel art."; "The grid resolution can drift over time."
- https://platform.claude.com/docs/en/build-with-claude/vision:
  - "Claude might hallucinate or make mistakes when interpreting low-quality, rotated, or very small images under 200 pixels."
  - "Animations are unsupported, and only the first frame is used."
  - "especially with large numbers of small objects."

**Shelters and clothing:**
- https://archeologie.culture.gouv.fr/sculpture-prehistoire/en/habitat (French Ministry of Culture):
  - "All they needed to do was close off the habitat with light structures and hides – possibly using loops pierced in the walls."
  - "small circular residential structures (tents or huts) that were covered with skins held to the ground by stones"
  - "Brace-holes for a lightweight frame, probably made of wood"
- https://excavations.ie/report/1973/Derry/0000148/ (P. C. Woodman, Mount Sandel):
  - "round huts approx 6 metres across with central hearths"
  - "large and more substantial than the flimsy summer constructions"
- https://excavations.ie/report/1974/Derry/0000187/: "circular with a central hearth and about 6m across."
- https://www.york.ac.uk/news-and-events/news/2010/research/earliest-house/ (10 August 2010):
  - "3.5 metres circular structure"
  - "had post holes around a central hollow which would have been filled with organic matter such as reeds, and possibly a fireplace."
- https://www.cambridge.org/core/journals/antiquity/article/chronology-and-function-of-a-new-circular-mammothbone-structure-at-kostenki-11/F6A3DA5935550AFA04671CA944EB511F (Pryor et al., *Antiquity* 94, 2020; abstract read through a fetch tool): "Circular features made from mammoth bone are known from across Upper Palaeolithic Eastern Europe, and are widely identified as dwellings."
- https://pmc.ncbi.nlm.nih.gov/articles/PMC409895/ (Frank Hole, *PNAS*, 2004, a commentary on Nadel et al., "Stone Age hut in Israel yields world's oldest evidence of bedding"): "organic traces of huts or shelters in which people lived"
- https://www.museum.de/en/audioguide/65/35/EN/0 (a museum's audio guide, station 35):
  - "thick oak posts sunk deep into the ground"
  - "covered with straw, reeds, tree bark or other available materials"
  - "clay, or daub"
- https://www.iceman.it/en/oetzi/clothing (South Tyrol Museum of Archaeology):
  - "clothing was made solely from leather, hide and braided grass"
  - "was made from light and dark strips of goat and sheep hide stitched together with animal sinews"
  - "The mat, made from alpine swamp grass, was initially thought to be a grass cape."

**Terms and copyright:**
- OpenAI Terms of Use, effective 11 December 2024, read at https://open.windriver.com/info/uni-license-list/licenses/openai-tou-20241211.html (openai.com refuses fetches):
  - "you (a) retain your ownership rights in Input and (b) own the Output"
  - "output may not be unique and other users may receive similar output from our Services"
  - "Represent that Output was human-generated when it was not."
- Secondary, a third-party monitoring site, used because OpenAI's pages refuse fetches; unverified first-hand:
  - https://conductatlas.com/platform/openai/terms-of-use-row/provision/CA-P-017154/openai-assigns-output-ownership-to-user/
  - https://conductatlas.com/platform/openai/openai-eu-terms-of-use/provision/CA-P-059306/openai-assigns-output-ownership-to-user/
  - Both: "We hereby assign to you all our right, title, and interest, if any, in and to Output."
- https://www.copyright.gov/ai/Copyright-and-Artificial-Intelligence-Part-2-Copyrightability-Report.pdf (U.S. Copyright Office, January 2025):
  - "Copyright does not extend to purely AI-generated material, or material where there is insufficient human control over the expressive elements."
  - "prompts do not alone provide sufficient control."

**My measurements** (`R/work2/5/`):
- `meas/plants_at_scale.py`, `meas/plants_native_compare.py`, `meas/textures_per_band.py`, `tex/measure.py`, `meas/spill.py`;
- the pictures `meas/plants-at-scale-x3.png`, `meas/plants-native-vs-shrunk-x3.png` (and `-1to1`), `meas/textures-closest-vs-closecamp.png`;
- the numbers in `meas/numbers-round2b.txt` and `meas/tex-describe.json`.
