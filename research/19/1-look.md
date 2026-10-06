# Study 1, round 2: the look to aim for

> Study 1 of [research 19](../19-graphics.md), written on 6 October 2026 and kept as written, its quotes checked ([the check](quote-check.md)).
> `R` was the research session's working folder and is not kept, except your pictures and answers, now in `art/targets/` and `art/reviews/2026-10-06-graphics/`.

M2 research, round 2, 6 October 2026. Research only.
`R` is the round's folder (`/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/m2-research`).
My scripts and their outputs are in `R/work2/1/`: `measure12.py` (the twelve pictures), `pair.py` (light, colour, texture and masses), `blocksize.py` and `localgrid.py` (the size of a picture's visible pixels), `halfres.py` (half resolution), `farsmooth.py` (far views without grain), `salience.py` (how much each figure stands out), most with a `.txt` of its output; `blocksize.py`, `localgrid.py`, `halfres.py` and `farsmooth.py` only printed theirs (the quote check re-ran the first three, `R/work/checker/1/rerun-*.txt`; `farsmooth.py`'s numbers are in `look-notes.md`). Round 1's note is `R/round1/notes/1-look.md`.

Words used here:
- **Direction B:** the owner's choice (answer 1): a smooth, sharp 3D world drawn at the phone's full resolution, whose surfaces wear textures painted as pixel art. Direction A drew the world small and enlarged it; direction C was A with soft glow and haze.
- **Texel:** one pixel of a texture. In direction B a texel shows on screen as a small crisp square.
- **Zoom band:** a range of zoom with its own version of each texture, drawn for that distance.
- **Grain:** the size of the marks a texture is made of (a pebble, a blade, a fleck of lichen).
- **Value:** how light or dark a colour is. **Chroma:** how strong a colour is, measured in OKLab, a colour space built so that equal steps look equal; a chroma above 0.12 is a strong colour.
- **Salience:** how much something stands out from what is round it. I measure it as the difference between a small blur at a figure's size and a wide blur of its surroundings, the centre-surround idea behind Itti, Koch and Niebur's saliency model, and give it as a percentile: a person at the 90th percentile stands out more than 90% of the picture's points.
- **Colour plan:** the planned colours of a biome at an hour and a season (often called a colour script).

## 1. The question, and the answer in brief

**The question:** what gives the owner's liked pictures their feeling, and what look, free of the strict rules, carries that feeling into a game drawn live on the phone, at every zoom, hour and season?

**The answer in brief:**
- **The feeling is consistent across all twelve pictures, and measurable.** Every picture has warm lights; strong colour stays in small accents (11% of the picture or less in eleven of the twelve); greens are muted and yellowish; there are real darks; one cool mass (clear water, snow or blue shade) sets off the warm; nature is dense and countable, with quiet grain; and the people stand on a quiet stage. Each moment then adds its own light: a night 86% dark, a winter 58% light, a cave lit only by its fire.
- **Direction B, which the owner chose, is the liked pictures' own look.** GPT's pictures never had a pixel grid. Their grain is 4 to 5 screen pixels in every picture, whatever the scale, as pixel art's is. The owner's second round of answers shows that the game's camera and Stone Age truth cost nothing: the storm with no sky, the mist with no horizon, the truthful village, the first cave and a camp of thirty all keep the feeling.
- **I recommend direction B as follows:** a texel of about 2 × 2 screen pixels, as the owner chose on true-size pictures (answer 35: 64 texels a metre at the closest zoom), drawn in marks of 2 to 3 texels, which gives the liked pictures' own grain of 4 to 5 pixels; each zoom band its own pixel art (64, 32, 16 texels a metre and so on), so the texel stays about 2 screen pixels at every zoom and never turns to speckle; light true to each hour and season, planned per biome; no outlines up close; detailed people and animals; small far figures drawn to read.
- **Readability in busy scenes: the owner chose light only** (answer 33). The world stays as it is, and the engine helps with light. My measures show the gap that remains: in the autumn wood the people rank only 55th to 88th in salience (in the liked camp, 77th to 96th), and light lifted the hunters in the sun (to the 72nd and 80th) while the woman kneeling in shade fell to the 58th. Every light must have a real source (sun, sky, fire), and what people wear follows from their materials and their people's style, never from the camera (`PRE-42`, `CUL-12`, `PRN-10`). Movement, which every still picture lacks, is the strongest help left; people in shade are a problem for M2 to solve on the phone.
- **The ground's speckle at the close camp** came from shrinking a texture by averaging (question 31, shown at about three times true size). Each zoom band needs its own pixel art, and the owner has since chosen one for the close camp: a redraw on a coarse grid with its colours matched by code (answer 36). The rock surface under each world's code-laid layers is still open (answer 34).
- **Truth:** GPT adds later things to most pictures (sawn wood, boats with seats, barrels, chickens, spotted horses), and it ignored an explicit "avoid" line in four of my seven latest pictures, once by keeping a mistake from the picture it was repainting. Target pictures need a written truth check before anyone aims at them (section 6.4).
- **Proposals for the owner** (section 6.5, `PRC-07`): new wording for `PRE-01`, `PRE-02`, `PRE-20`, `PRE-21`, `PRE-22`, `PRE-27`, `PRE-28`, `PRE-03` and `MIL-09`, and A5.2 and A5.3 rewritten as guidance about the feeling.

## 2. What `PROJECT.md` asks

- **The goal:** `MIL-09`, "crisp 3D pixel art (`PRE-01`, `PRE-02`) that goes well beyond the art book's pictures, which were preliminary". The feelings: wonder (`VIS-07`), life (`VIS-17`), a joy on the phone (`VIS-14`).
- **Lifted for this research** (BRIEF2): `PRE-01`'s "limited colours from one palette" and one solid palette colour per art pixel; `PRE-20`'s "about 4–7 shades" and its "no fine grain"; `PRE-21`'s one-pixel outline; `PRE-22`'s fixed art-pixel sizes; `PRE-27`'s "tiny blocks"; A5.2 and A5.3.
  Lifting these touches `PRE-02` too, which says the world is "drawn at low resolution", and `MIL-09`'s list ("the stable pixel grid", "outlines and lit edges", "colour in steps"). I propose wording for both.
- **Not lifted:**
  - `PRN-10`: "Every picture, sound and word shows what is really there and what really happened in the world", and "Nothing is added for show".
  - `CUL-12`: each people has its own style, and "everyone wears their people's ornament (beads, body paint, decorated clothes), the most respected most". `PRE-42`: things take "the colours and shapes of the materials used".
  - `WLD-13`, Stone Age truth (`PRE-42`), the phone's limits (`PLT-01`, `PLT-04`), both orientations (`PLT-02`, `PRE-34`), readability from far away (`PRE-28`), the seamless zoom (`PRE-03`) and the kit's countable structure (`PRE-46`).
- **A stale number in `PRE-03`.** The person stop is "about 8 m across, a person about 58 art pixels tall". At 8 m across the portrait screen's 1,080 pixels, a 1.7 m adult is about 230 screen pixels tall: 58 art pixels fitted the old 4 × 4 art pixel, not the 2 × 2 agreed on 5 October. The owner's answer 5 keeps the 8 m. Section 6.5 gives the stop in screen pixels.

## 3. Where we stand

### 3.1 What round 1 found

- **My round-1 note:** the liked pictures' lights are golden, their greens muted and yellowish, a quarter of each picture dark, strong colour kept to small accents, clear teal water the one cool mass, no flat ground, and quiet texture. I recommended "rich pixel art" on a true 2 × 2 grid. The owner chose direction B instead, so that recommendation is withdrawn; its measures of the feeling stand.
- **The other studies:** study 2 showed the liked camp keeps most of its look on a true 2 × 2 grid with 64 colours; study 4 that density, not drawing small, is what costs, and rich textures cost memory more than time; study 5 three routes to content; study 6 a target card and a loop; study 7 the pipelines. Their round-2 work is moving to direction B.

### 3.2 The owner's 36 answers, read as a look

| What the owner did | Answers |
|---|---|
| **Chose** | B, a smooth world with pixel textures (1); 2 × 2, the finest (2); detailed people (3) and animals (14); small far figures drawn to read (4); the closest zoom as in the liked picture, about 8 m across (5); true midday (9); nights A, B and C, not D (10); winter B, softer (11); far views A and C (12); several shelter types (13); surfaces A, C and D (15); dusk by relighting an approved picture (23); light only to help people read, the world left as it is (33); each world's rock layers laid on by code (34b); texels of about 2 screen pixels, 64 a metre up close (35); the close camp's ground redrawn for its band, colours matched by code (36) |
| **Kept** | the plants' density, airiness and wildness (6, "As I liked"); no outlines up close, smooth light, the fine grain, darkening in corners and under things (7, "As I liked"); both the sun and the plants (8); all four rain layers (17) |
| **Found free** | facing a low sun (16); lake mist with no horizon (18); a camp of thirty, "alive" (26); the first cave, "as beautiful as the rest" (28); the village without later things (29); the storm with no sky (30) |
| **Problems to solve, not cut** | small plants lose their charm at game size (21); people found only "with effort" in a busy wood (27), and, by my measure, light alone (33) does not yet help those in shade; the ground turns to speckle at the close camp (31); the rock surface under the layers is not right yet (34) |
| **Not enough on its own** | colour alone (24); half resolution "only if needed" (25); rock without its layers, "can't judge yet" (32) |

**My reading:** the owner refuses every trade-off that shows and accepts every limit of truth and of the game's camera. So the look must be won on light, density and grain, and every saving must be invisible.

### 3.3 What the current design says, and what is wrong for direction B

- **A4.1, the picture.** The world "renders into a SubViewport at a quarter of the screen's width and height", "shown scaled up with nearest sampling", with "A camera locked to the pixel grid". Direction B draws at full resolution, so the low-resolution picture, the nearest enlargement and the pixel lock go (study 2).
- **A4.1, outlines and light.** Outline way C and "Light in clean steps", with "Each material's ramp of 4 to 7 shades", contradict answer 7 (no outlines up close, smooth light).
- **A4.2, the ground:** "flat colours of its cover (meadow, dry grass, path, bank, bed, rock) by surface weights, meeting in clean edges". The liked pictures have no flat ground anywhere.
- **A5.2's rules:** rule 6, "Nature soft, made things crisp", puts ground, grass, leaves and water "in flat patches"; rule 5 keeps 4 to 7 shades; rule 7 keeps outlines.
- **A5.3:** "Every texture is drawn at 16 texture pixels a metre, about one to one art pixel at the close camp zoom". At the closest zoom such a texel would be about 8 screen pixels across: chunkier than the bottom row of question 31, which the owner called speckle.
- **A8.3's far forms** ("tiny figures outlined in a darker shade of their own colour") already fit answer 4.
- **Missing:** a rule for readability in busy scenes; colour plans per biome, hour and season; a truth check of target pictures; texture levels per zoom band.

### 3.4 The twelve pictures, measured

The two liked camps and the ten pictures of other moments (`R/owner/contexts/`), measured with round 1's measures (`measure12.txt`, `notan-sheet.png`).
"Dark" is the share of pixels with OKLab lightness under 0.45; "light" above 0.70; "lights" is the yellow-blue tint of the brightest fifth (positive is warm); "strong colour" the share with chroma above 0.12; "big masses" the share of light-dark variation that survives a blur of 1/64 of the width.

| Picture | Dark | Light | Lights | Strong colour | Greens (share, hue) | Big masses |
|---|---|---|---|---|---|---|
| Liked camp, from above | 27% | 24% | +0.088 | 10.9% | 34%, 111° | 0.28 |
| Liked camp, at sunset | 51% | 7% | +0.065 | 3.7% | 16%, 113° | 0.29 |
| 01 A person at dawn | 48% | 13% | +0.072 | 5.7% | 19%, 112° | 0.29 |
| 02 The camp at night | 86% | 2% | +0.018 | 2.1% | 1% | 0.22 |
| 03 A winter steppe | 18% | 58% | +0.028 | 1.2% | 0% | 0.23 |
| 04 A lake in morning mist | 42% | 17% | +0.072 | 4.4% | 17%, 113° | 0.43 |
| 05 An autumn forest | 53% | 9% | +0.090 | 16.6% | 8%, 106° | 0.16 |
| 06 A river valley | 30% | 22% | +0.087 | 9.8% | 27%, 105° | 0.19 |
| 07 A coast at dusk | 61% | 7% | +0.055 | 8.0% | 0% | 0.24 |
| 08 A village, first copper | 26% | 35% | +0.083 | 7.4% | 23%, 118° | 0.19 |
| 09 A thunderstorm | 67% | 3% | +0.032 | 2.0% | 11%, 106° | 0.19 |
| 10 A painted cave | 75% | 1% | +0.089 | 9.1% | 0% | 0.51 |

**What is consistent (the feeling):**
- **Warm lights in all twelve,** even at night (the fires) and in winter.
- **Strong colour as accents:** 11% of the picture or less in eleven of the twelve. The exception is the autumn forest (17%), the picture whose people are hardest to find (section 3.7).
- **Greens muted and yellowish,** hue 105° to 118° where there are greens (the art book's close camp: 121°, one green over 60% of the frame).
- **Real darks:** in daylight, a quarter to a half of each picture (winter, 18%, is the one light world).
- **One cool mass against the warm:** water, snow, mist or blue shade.
- **Grain of 4 to 5 screen pixels** everywhere (section 3.5), and no flat ground.
- **A quiet stage under the people:** a trodden path and floor in the camps, snow under the file in winter, the bare cave floor. The three-value maps (`notan-sheet.png`) show it as a light streak through the camp from the path to the hearth.

**What each moment adds:**
- **Dawn:** a person close up at work, long low light, smoke, a dog asleep: intimacy.
- **Night:** 86% dark, half of it very dark; the fire's warm pools are small (warm colour 18% of the picture), the moon on the water.
- **Winter:** the one light world (58%); people and the dog as dark marks on white, the easiest picture to read.
- **Mist:** the biggest calm masses (0.43, after the cave's darkness, 0.51): mist and water as one soft field, a golden glare.
- **Autumn:** the warmest (61% warm) and most broken-up picture (masses 0.16): the richest, and the hardest to read.
- **The valley:** the land as a pattern of river, woods and meadows; camps read as a smoke column over a pale ring, herds as clusters of dots.
- **The coast at dusk:** wet sand and pools holding the sky's colour; a shell midden.
- **The village:** the most light after winter (35%) and the most order: fences, plots, paths, many tasks at once.
- **The storm:** 67% dark and cool, one bright event (the burning tree) to draw the eye, and movement in every part: bent grass, rain, a galloping herd.
- **The cave:** darkness as the composition (masses 0.51), the light coming only from the fire and lamps.

### 3.5 GPT's grain, and the texel the look needs

**GPT's pictures have no pixel grid, and their grain is the same size on screen at every scale.**
- On 64 × 64-pixel windows, a clear local grid shows in 0% to 2% of windows in my round-2 pictures and in 19% (weak, mixed sizes) in the liked ones; the code-snapped versions show it in 100% (`localgrid.py`).
- The distance between strong colour edges, soft edges merged, is 4 to 5 pixels in every picture: the liked camp material by material (cliff 4, meadow 4, crowns 4, hides 5, water 5, people 5), my close camp at about 17 m across (4), the far views at hundreds of metres (4), the valley (4) (`blocksize.py`; on exact grids the measure reads 2, 3 and 4 for 2 × 2, 3 × 3 and 4 × 4).
- So GPT paints a grain of constant size on the screen, as pixel art does, whatever the distance. Shown across the phone's width, that is 4 to 5 screen pixels, with soft edges.

**What it means for direction B.** In a game the texture is fixed to the world, so a texel's size on screen changes as the camera zooms. Keeping the grain the same on screen needs each zoom band to have its own texture, drawn as pixel art for that band: a band's texture is not the next one averaged down, which is exactly what turned to speckle in question 31.

**Which texel size.** The evidence:
- The owner chose "2x2, the finest" (answer 2) among hard grids laid over my close camp, a view about 17 m across: there 2 screen pixels are about 1/31 m.
- In the liked picture, about 9 m across, the grain is 4 to 5 pixels: about 27 marks a metre.
- The art book, whose grain was about 3 screen pixels on the phone, drew this in its second edition (git `7874f41`, 4 October): "The owner found some shapes hard to read through grainy textures".
- Pixel artists build texture from small clusters, not single pixels: "Not every leaf needs to be represented" (Slynyrd, 2019).

My reading: a crisp texel of **about 2 screen pixels at the closest zoom** (about 64 a metre), drawn in marks of 2 to 3 texels, gives the liked pictures' 4 to 5 pixel grain while keeping single-texel detail for eyes, glints and flower specks; a crisp 4-pixel texel would look chunkier than GPT's soft 4-pixel blobs. Studies 2 and 7 split the same way (64 against 32 a metre), and the owner settled it on true-size pictures: **about 2 screen pixels, 64 a metre** (answer 35). With zoom bands of 64, 32, 16 texels a metre and so on, the texel stays about 2 screen pixels at every zoom.

On the phone (390 dots an inch, held at about 32 cm), a screen pixel is 0.065 mm, about 0.7 minutes of arc: below what the eye resolves (about one minute, round 1's sources). A 2-pixel texel is 1.4 minutes, just visible: texture at that size reads as richness if it is quiet, as the liked pictures' is.

### 3.6 What the owner picked, measured

The pictures picked and passed over in the answers, side by side (`pair-picks.txt`):

| Question | Picked | Passed over | What separates them |
|---|---|---|---|
| Night (10) | three nights 85–87% dark, 35–50% very dark, warm colour 5–14% | night D: 77% dark, warm 25%, strong colour 4.9% | the chosen nights are dark and cool, with firelight in a few small pools |
| Winter (11) | B: very dark 5%, strong colour 1.6%, shade only faintly blue | A: very dark 11%, strong colour 5.3%, deep blue shade and river | the chosen winter is soft and low in contrast |
| Noon (9) | true midday: warm colour 22%, cool 28%, greens at 123° | the late afternoon (48% warm) and a noon kept warm by design | the owner wants each hour true, not golden all day |
| Far views (12) | A and C: dark 38–45%, green 11–28% | B, a repainted engine frame: green 76% of the frame, strong colour 26% | one saturated green over most of the frame loses the feeling, as A5.2 rule 10 already warns |

**Half resolution (25).** The owner saw that B made at half resolution and enlarged smoothly is softer. Measured, the picture the owner saw keeps 60% of the detail at one-pixel scale (0.126 against 0.210) and loses about 40% of the strength of its sharpest edges (`halfres.py`). That stand-in is harsher than an engine would be: shading at half rate with edges kept at full resolution would keep every silhouette sharp. With texels of about 2 pixels, little texture detail sits below 2 pixels, but a shading grid of the same size can make texel edges crawl as the camera moves (section 7, question 4).

**The ground's speckle (31).** The sheet showed both rows at the same size. Shown across the phone's width, the closest zoom's texels were about 1.5 screen pixels (three quarters of true size) and the close camp's about 7 (more than three times true size, where they would be about 2). The verdict stands as a warning that one texture shrunk by averaging fails. The owner then chose, for the close camp, GPT's redraw of the meadow on a coarse grid with its colours matched by code, over the same texture shrunk or reduced by code (answer 36): a level drawn for its band, as this section argues.

**Far views without grain.** By code I smoothed away the grain of my far views while keeping their edges (`farsmooth.py`, `far-camp-zoom-far-pixel-vs-smooth.png`). At true size the change is hard to see (mean change 0.012 to 0.014): far away, the feeling lives in shapes, light and life, and the far ground can be plain filtered colour under crisp crowns, rocks, herds and smoke.

### 3.7 Readability, measured

Each person's salience, as a percentile of all points in the picture (`salience.txt`, `salience2.txt`, `salience3.txt`):

| Picture | People | Median | Animals |
|---|---|---|---|
| Liked camp, from above (people about 185 pixels tall) | 77, 84, 90, 93, 96 | 90 | none |
| My close camp, late afternoon (about 95) | 77, 80, 83, 84, 93, 93, 99 | 84 | deer 74, 86 |
| Camp of thirty, direction B (about 40) | 15 measured: 91 to 99.9, and one in the shelter's shade at 75 | 99 | |
| 05 Autumn forest, owner's context (about 150) | 66, 75, 90, 91 | 82 | stag 95, boar 99 |
| Autumn, direction B (`autumn-close.png`, about 135) | 55, 68, 78, 88 | 73 | stag 99, boar 96 |
| The same with light aids only (study 2's `read-aids.png`; **picked**, answer 33) | 58, 72, 80, 83 | 76 | stag 99.5, boar 95 |
| The same with the world changed: calmer leaves, trodden ground, lighter clothes, light (my `autumn-readable.png`; passed over) | 81, 84, 88, 89 | 86 | stag 99.9, boar 93 |

**What it shows:**
- **Size is not what makes people readable.** In the camp of thirty, people 40 pixels tall rank 91st to 99.9th; in the autumn wood, people 135 pixels tall rank 55th to 88th, below the stag and the boar.
- **The ground under them is.** In the camp the people stand on a quiet trodden floor; in the wood they are brown on brown litter amid a quarter of the picture in strong orange and red (24%, against 11% in the liked camp). Wolfe and Horowitz's review of visual search says it plainly: "Salience of a target increases with difference from the distractors (target-distractor – TD- heterogeneity) and with the homogeneity of the distractors (distractor-distractor –DD- homogeneity) along basic feature dimensions."
- **Light alone helps only where light falls.** Study 2's light aids lifted the hunters in the sun (55th to 72nd, 68th to 80th) but the woman kneeling in the hazels' shade fell (78th to 58th). The owner also kept the season's strong colour (24% of the picture as drawn, 26% in the light-only repaint, against 13% in my calmer repaint and 11% in the liked camp): in autumn the colour is the field, not an accent.
- **The owner's choice (answer 33): light only.** The owner picked the light-only repaint and passed over mine and "both together". So nothing in the world is changed for readability: no calmer leaves, no extra trodden ground, no lighter clothes. And the light must have a source: study 2 found that a fill light that follows people has none and breaks `PRN-10`, while a rim from the real sun is fine. Thinning leaves over a person is a view aid, like a cut-away, not a change to the world.
- **The gap is people in shade,** where no sunlit rim reaches. There the sky's light, a fire, light bounced from sunlit ground and, above all, movement must carry them. I did not ask for another picture of this: GPT paints light by eye, so only the engine's own light can show whether real sources are enough (section 7, question 3).
- **Clothes are not the look's to choose.** My `autumn-readable.png` lifted all four people mostly by dressing them in pale buckskin. What people wear follows from their materials and processes (a hide's colour depends on the animal and on whether it was smoked) and from their people's style (`CUL-12`'s "two favourite colours"), and `PRN-10` forbids anything "added for show". So a pale tunic is something the world sometimes gives, never something the engine may promise. The engine may only show each material's true colour faithfully, with each people's ornament as small accents.
- **Movement.** Wolfe and Horowitz list motion with colour among the attributes that most surely guide attention. Every picture here is still; in the game the people move at their tasks while leaves sway, so they will be found faster than these numbers say. How much faster is a phone question (section 7).

### 3.8 What carries into the live game, zoom by zoom

- **Kept as it is:** the logic of light and colour (warm lights, real darks, strong colour as accents, one cool mass); clear water over its bed; the grain, as pixel textures; detailed figures as big as the liked ones at the closest zoom (answer 5); and, as the owner's answers 18 and 28 to 30 show, the feeling without sky or horizon and with truthful content.
- **Approximated:**
  - *Composition:* no painter frames the view. The camera (the sun behind its shoulder, A5.2 rule 2), the world's own lines (rivers, cliffs, paths) and the quiet ground where people work stand in; study 3's director frames live moments.
  - *Light painted for one view:* the painter's rim on every figure and darkened corners become light the world could give: rims from sun, sky and fire, contact darkening and our own corner darkening (answers 7 and 16).
  - *Density:* many copies of the kit's shapes, each varied by its seed (`PRE-43`), drawn as each zoom band's own pixel art.
  - *Each hour and season:* colour plans per biome, hour and season, true midday included (answers 9 to 11 and 23).
- **Lost:** unique painted detail everywhere (every stone painted once), light painted for one view, and people spaced perfectly for the eye. None of these is what the owner named as the reason they like the pictures.

| Zoom stop | A standing adult | What carries the feeling | What reads |
|---|---|---|---|
| Closest, about 8 m across | about 200 screen pixels (about 100 texels) | faces, clothes, tools, the grain of every surface, the water's bed | everything, down to a face's feeling |
| Close camp, about 20 to 50 m | about 35 to 90 | light and colour, density, the camp's trodden stage | tasks, poses, clothing colours (the camp of thirty: "alive") |
| Camp, a few hundred metres | about 10 to 30 in the far views the owner kept; figures drawn to read (answer 4) | light, big masses, the river, smoke, herds | people as small marks, groups, a hearth glowing at dusk |
| Valley and beyond | marks | the land's pattern and light | a camp's smoke over its pale ring, a herd as a cluster (picture 06) |

## 4. What others do

### 4.1 Pixel textures on a sharp 3D world

| Work | What it does that bears on us |
|---|---|
| **Minecraft, Vibrant Visuals** (2025; `PRE-30`'s model) | Pixel textures under a modern sun, sky and fog, with the pixel look kept in the light too. Its art director, Jasper Boerstra: "When you play, you'll notice that the shadows in the game are pixelated. All the reflections are pixelated, too." And: "We wanted to keep it *Minecraft*-y". |
| **Hytale** (art director Thomas Frick, December 2025) | The closest match to direction B: "A modern, stylized voxel game, with retro pixel-art textures", "at the intersection of low-definition pixel art and hand-painted 3D". "We paint lights and shadows inside textures and use real lights/shadows to bring everything together"; effects such as "SSAO, fog, and bloom" are "the cherry on top", and "even without any effects, our models should look good on their own". Two texel densities: "64px per unit" for characters and tools, "32px per unit" for props and blocks. "We avoid noise, too much grain, or perfectly flat surfaces." |
| **Valheim** (Iron Gate, 2021) | Low-detail 3D with low-resolution textures; its maker Richard Svensson: "the low-detail stylings of the world are spruced up quite a bit by a plethora of ambient effects and post-processing" |
| **t3ssel8r** (2020–2023) | A shader for "anti-aliasing for pixel-art textures in low-poly games like Minecraft": crisp texels without shimmer, the filter direction B needs (study 2). t3ssel8r's rain is "a matter of layering together a bunch of individual effects": streaks, splashes, ripples and blowing mist are four of the eight, the same four the owner said all matter (answer 17). |
| **Project Shadowglass** (2025–2026) | A 3D world turned into a pixel grid (direction A's family): "Characters and objects sometimes have to look very different far away than they do up close, otherwise they can read as pixelated noise instead of coherent forms." |
| **A Short Hike** (2019) | Small figures kept readable with an outline: "I added a soft outline effect to objects to help them stand out, and stay readable with so few pixels." The owner's answer 4 asks the same for small far figures. |

**What this teaches us:**
- **Direction B is an established look,** not an experiment: Minecraft ships it in Vibrant Visuals, and Hytale's art director describes the same recipe. Both keep the pixel character in the light as well as the textures, and both rely on painted texture first, with effects on top.
- **Two densities are possible in this look.** Hytale gives characters twice the world's texel density, while Kingdom's maker holds that "pixels should be the same size across the model". With the owner's 2-pixel texel, one density is right for us (section 6.3).
- **Far versions are designed, not shrunk** (Shadowglass), as the owner's answer 4 and the speckle of question 31 both say.

### 4.2 Detail by distance, colour and clarity

- **Each distance drawn its own way.** Slynyrd (June 2026) on landscape planes: "In the nearest plane, there are thick apparent blades of vegetation, while the next plane only uses one, or two pixel tall clusters at the most", and "The subsequent planes no longer depict single blades of vegetation at such a great distance". That is the zoom band rule, and the cure for answers 21 and 31.
- **Quiet texture.** "You only need a few colors to make a rich texture. Too many colors and the texture will become blurry"; "A busy texture next to another busy texture may exhaust the eyes with too much noise. Negative space is your friend" (Slynyrd, 2019).
- **Colour discipline.** Valve's Team Fortress 2 paper: "muted colors dominating and small areas of saturation to give further visual interest"; "Shadows go to cool, not black". The liked pictures follow both.
- **Clarity first.** Riot: "the most important thing at any given moment (like a major ultimate or CC) should draw the most attention", and "Finally, noise should be kept minimal". Valve validated its characters by silhouette: "Even when viewed only in silhouette with no internal shading at all, the characters are readily identifiable to players."

## 5. The options

### 5.1 The way of drawing

The owner chose B (answer 1). The table records why, on the brief's measures.

| | A. Pixels as the rendering | **B. Sharp world, pixel textures** | C. A with soft glow and haze |
|---|---|---|---|
| **Closeness to the feeling** | Close at 2 × 2 (the liked picture changes by 0.037 on average on a true 2 × 2 grid), but every edge stair-steps | Closest: the liked pictures' own look, sharp edges, grain on surfaces, smooth light | As A, with the pictures' soft air |
| **Owner** | not picked | **picked** | not picked |
| **Cost on the phone** (study 4) | lowest: about 0.65 million pixels shaded a frame | about 2.6 million pixels, 4 times A's per-pixel work; study 4's first rough estimate, not yet modelled: 4.5–12 ms at the closest zoom with savings that do not show | A plus soft passes |
| **Effort for an AI builder** | medium | medium to high: textures per material and zoom band, a texture filter, our own corner darkening | medium |
| **Risk** | pixels crawl in turns | heat and battery at full resolution; shimmer if the texture filter fails | crawl plus blur |
| **Fit with the rules that stay** | fits | fits; rewrites `PRE-01`, `PRE-02`, `PRE-20` to `PRE-22` | fits |
| **Needs from the engine and content** | a low-resolution picture locked to its grid; textures one texel to one art pixel | full-resolution drawing, a texel filter, zoom-band texture levels, built corner and contact darkening, colour plans | as A, plus glow and haze passes |
| **Examples** | Project Shadowglass, t3ssel8r | Minecraft's Vibrant Visuals, Hytale, Valheim | HD-2D's soft light and depth (round 1) |
| **How judged** | the engine's frame beside the liked picture on the phone | the same, with the measures of 6.2 on every contact sheet, then the owner's eye | as A |

### 5.2 The texel's size within B

| | **X. About 2 screen pixels at the closest zoom, one density** | Y. World about 4, figures about 2 | Z. A5.3 as written, 16 a metre |
|---|---|---|---|
| **At the closest zoom** | world and figures 64 texels a metre (2.1 pixels) | world 32 (4.2 pixels), figures 64 (2.1) | 16 (8.4 pixels) |
| **Closeness to the feeling** | the owner's "finest"; marks of 2–3 texels give the liked 4–5 pixel grain | the liked picture's grain directly, Hytale's split; chunkier than GPT's soft blobs | the chunky speckle of question 31 |
| **Memory** | 4 times Y's for the world's textures | a quarter of X's | least |
| **Risk** | finer grain shimmers more if the filter is weak | a figure finer than its ground may look pasted on | fails the owner's answers |

**The owner chose X** on true-size pictures (answer 35), as I recommended. It needs every zoom band's texture drawn as pixel art.

### 5.3 Readability in busy scenes

| | **R1. Light only (picked, answer 33)** | R2. Changing the world | R3. A glance aid |
|---|---|---|---|
| **What** | a rim of the real sun or a fire on figures, sky light and light bounced from the ground, contact shadows; leaves thinning over a person as a view aid (study 2's `read-aids`) | calmer leaves, trodden ground under people, lighter clothes, plus R1 (my `autumn-readable`) | on a touch, the people briefly ring or glow, as an interface aid, off by default |
| **Measured** | hunters in the sun up 12 to 18 points; the woman in shade down 19 | all four people at the 81st–89th percentile, mostly from the clothes | not drawn |
| **Fit with the rules** | fits if every light has a real source: a fill that follows people has none and breaks `PRN-10` (study 2) | the look cannot choose clothes (`PRE-42`, `CUL-12`, `PRN-10`); the owner passed it over | an interface layer, like `PRE-28`'s markers, outside the world's picture |
| **Cost** | small (shader terms) | small | small |

**The owner chose R1,** and I recommend it as chosen: judged in motion on the phone, where people in shade are the gap to solve (section 7, question 3), with R3 in reserve if the owner still has to search.
I asked for one more picture, the world's levers with the clothes kept (`autumn-levers.txt`); the builder held it, rightly, since the owner had passed over changing the world.

## 6. What we'd recommend

### 6.1 The look, in one paragraph

Direction B: a sharp 3D world at the phone's full resolution, whose every surface is quiet pixel art of about 2 × 2 screen pixels at every zoom, drawn in marks of 2 to 3 texels and redrawn for each zoom band; light true to the hour and season and planned per biome, smooth, with real darks, shade cool and lights warm; strong colour kept to small accents except where the season is the colour, greens muted and warm, one cool mass of water, snow or shade; nature dense, airy and wild, as liked, with every plant, stone and creature reading on its own; no outlines up close, but darkening in corners, under things and where things meet the ground; people and animals detailed and true to the archaeology, found at a glance by the light that really falls on them and by their movement, and drawn to read when far away.

### 6.2 The art bible, as guidance about the feeling

Proposed to replace A5.2's rules. Each line is guidance, with a starting number to calibrate against the owner's verdicts (study 6's target card), not a hard rule.

1. **The feeling first, then the rules.** When a rule and the owner's picture disagree, the picture wins, unless truth (`PRE-42`) or a principle says otherwise.
2. **Light true to the hour.** True midday at noon, gold only when the sun is low (answers 9 and 23). Shade takes the sky's colour; lights are warm; hollows, corners and contacts darken.
3. **Real darks.** In daylight about a quarter of the picture is dark (the liked camp 27%); nights about 85% dark with firelight in a few small pools, warm colour under about a fifth of the picture (the chosen nights 5–14%, the owner's night camp 18%, the night passed over 25%; answer 10); winter soft, with very dark under about 5% (answer 11).
4. **Strong colour as accents,** except where the season itself is the colour. In summer daylight, strong colour (chroma above 0.12) on no more than about an eighth of the picture (the liked camp 11%), carried by flowers, fire, dyes, ochre and beads. In autumn the owner kept the wood's colour as the field (24–26%, answer 33).
5. **Greens muted and warm,** never one green over most of the frame (the far view the owner passed over: 76%).
6. **Each biome, hour and season has its colour plan,** built from the true colours of its species by season (`WLD-31`).
7. **Quiet pixel texture.** Marks of 2 to 3 texels; low contrast within a material; never single-texel speckle; each zoom band drawn as its own pixel art, never shrunk by averaging (answers 21, 31 and 36).
8. **Density with a stage.** Nature dense, airy and wild (answer 6), but people stand on quiet ground where the world has made it: trodden paths, working floors, snow, cave floors.
9. **People read first.** In every scene the people are among the first things the eye finds. As a starting number: the people's median salience at about the 80th percentile or more, and none below about the 70th (the liked camp: median 90, lowest 77; the camp of thirty: 99 and 75; the wood the owner searched: 73 and 55). Only by light from real sources (sun, sky, fire) and by movement: nothing in the world is changed to make people stand out (answer 33), and what they wear follows from their materials and style.
10. **Edges by light.** No drawn outlines up close; a bright edge where sun, sky or fire grazes a shape; contact darkening. Small far figures alone are outlined (answer 4).
11. **Life in motion.** Wind, water, smoke, rain in its four layers (answer 17), people and animals; nothing moves that the world does not move.
12. **Truth before beauty in content.** Shapes, materials and dress from archaeology (`PRE-42`), style generated (`CUL-12`), no real culture's motifs (`SCP-20`); every target picture checked (section 6.4).

### 6.3 Texels by zoom band

Proposed to replace A5.3:
- A texel shows as about 2 × 2 screen pixels at every zoom (answer 35): 64 texels a metre at the closest zoom (about 8 m across), then 32, 16 and so on by zoom band, so it stays between about 1.5 and 3 screen pixels as the bands change. Each band has its own texture level, drawn as pixel art for that band (bolder marks, fewer of them), never the closer level averaged down, and bands blend so no change pops. The owner's choice for the close camp's ground (answer 36) shows one way to make a band's level: a redraw on the band's grid, its colours matched by code to the approved picture.
- One density for figures and world: at 2 screen pixels a texel, a standing adult is about 100 texels tall up close, enough for faces (study 5); twice the density for figures, as Hytale does, would make their texels a single screen pixel.
- Grass, reeds, flowers and flames keep their true size in metres, as now.

### 6.4 Checking target pictures for truth (`PRE-42`)

GPT drew later or borrowed things in ten of the owner's twelve pictures, and slipped on the season in an eleventh. Six of my seven latest pictures (the lake, the five restyled scenes and the autumn repaint) carry a slip, and four ignored an explicit "avoid" line: seats in a dugout and a flat-topped stump; long flowing manes; striped piglets in autumn, twice, the second time kept from the picture it was repainting. It also drew a live stag where I asked for a deer carried on a pole. What I found, by looking at enlarged crops:

| Kind | Found in | Why it is wrong, with the first-hand source |
|---|---|---|
| **Sawn wood:** flat stump tops, flat log ends, plank bridges and benches, squared blocks | liked sunset, 02, 04, 07, 08; lake mist | early farmers split and adzed their timber: "The logs were split first in half with wooden wedges", using "Stone adzes with transversely hafted blades" (Tegel and others, on wells of 5469 to 5098 BC) |
| **Built wells** with a winch, rope and hooped bucket | 08 | the oldest wells are "chest-like well linings" of notched split timbers (Tegel); hoops and staves are coopers' work, known from a painting in the Egyptian tomb of Hesy, about 2600 BC (Wikipedia, secondary), where "real cooper's work is intended, — barrels with bevelled staves" (Quibell, the excavator, 1913); whether any are older is still to check |
| **Chickens** | 08 | "the first unambiguous domestic chicken bones are found at Neolithic Ban Non Wat in central Thailand dated to ∼1650 to 1250 BCE", reaching "Mediterranean Europe by ∼800 BCE" (Peters and others, 2022) |
| **Spotted (pinto) horses, long falling manes** | 09; storm, direction B | wild horses were "bay or bay-dun", and "The Tobiano spotting was first found in a single Eastern European sample (3500 to 3000 yr B.P.)" (Ludwig and others, 2009); today's wild horse has "a dark zebra-like erect mane and no forelock" (the Smithsonian's National Zoo, which breeds them; Britannica, secondary: "The mane is short and erect with no forelock") |
| **Striped piglets in autumn** | 05; my autumn picture and both repaints of it, though asked not to | "These stripes are lost when the piglet is 3 to 4 months old", and farrowing "peaks in April" (GWCT) |
| **Boats with seats and ribs** | 02, 04, 07; lake mist | the oldest boat is a dugout: "It was made from a single Scots pine log", 8040 to 7510 BC (Wikipedia on the Pesse canoe, secondary) |
| **Metal-age tools and goods:** a pot hung over the fire, a metal sickle, lattice windows | 02, 08 | later than the scenes; dates to check before any village target is used |
| **A potter's wheel,** a jar shaped on it | 08 | not clearly later: in the southern Levant "the first potter's wheels" date to the "second half of the 5th mill. BC", the Late Chalcolithic (Copper Age), and shaped mainly "V-shaped bowls" (Roux and Harush, 2022); a jar on a wheel is still to date |
| **White woolly sheep** | 08 | not clearly later: the start of wool production is "still largely unclear", in "later Neolithic and Chalcolithic societies", and first written of at the "end of the 4th to beginning of the 3rd millennium BCE" in Mesopotamia (Becker and others, 2016); on the cautious view woolly breeds were probably absent from Western Europe before the 3rd millennium BC (Anaya and others, 2024); the white fleece is still to check |
| **A knapping mallet with a handle** | 01 | knappers strike with a hand-held stone or a billet "made ... out of the base of a mule deer antler" (Denoyer, Archaeology Southwest, a modern knapper) |
| **Borrowed dress and gear:** red, white and blue bead yokes, a modern rucksack, a slatted sled, maize-like cobs | 03; village, direction B | glass-bead colours and later gear; to check case by case |
| **Real cultures' signs:** tipi-like cones with smoke-flap poles; Lascaux-like animals and hand stencils | liked sunset, 02, 06; 10 | a Plains tipi's smoke holes "had adjustable, ear-like flaps on each side" (Center of the West); paintings in Kindling come from the kit's own models in each people's style (`CUL-09`, `PRE-46`), never copies (`SCP-20`) |
| **The caveman cliché:** sleeveless fur dresses, fur skirts | liked sunset | tailored clothing, as the precise prompt got right; a Copper Age man wore leather mostly "from domestic ungulate species (cattle, sheep and goat)" (O'Sullivan and others, 2016) |

**How to check a target picture before anyone aims at it:**
1. **Ask truthfully:** the prompt's truth lines, as in my restyled requests, which reduce but do not prevent these slips.
2. **Look closely:** every made thing, animal and garment at twice size, against the list above.
3. **Date anything doubtful** against a first-hand source, and write the verdict beside the picture.
4. **Keep the picture's feeling, not its mistakes:** a target is approved for its light, colour, density and composition; its flagged things are listed as not to copy.
5. **Never take motifs** from real cultures' art or dress (`SCP-20`).

The owner's answers 28 and 29 show that truth costs no feeling.

### 6.5 Proposed wording for `PROJECT.md`

Each is a proposal for the owner (`PRC-07`), as a **Proposed change:** line under the item, to replace its text only with the owner's OK.

- **P-1, `PRE-01` Detailed pixel art.** "Everything on screen is a sharp, detailed 3D world drawn at the screen's full resolution, whose surfaces wear textures painted as pixel art (`PRE-22`), under smooth light (`PRE-30`), with no blur."
  Done when: "at every zoom stop, texture pixels show as crisp squares and nothing in the world is blurred."
  *Why:* the owner chose direction B (answer 1); "limited colours from one palette" and "no smooth gradients" no longer hold.
- **P-2, `PRE-02` Pixel-rendered 3D** (title "Pixel-textured 3D"). "A real 3D world drawn at the screen's full resolution with pixel-art textures, so it looks like hand-made pixel art with real depth, scale and structure; the camera turns freely and zooms continuously." Done when unchanged.
  *Why:* "drawn at low resolution" is direction A.
- **P-3, `PRE-20` Colour in steps** (title "Colour by design"). "Every material is painted as pixel art in its own shades, made from its colour (`MAT-10`) or by hand for common ones, with lights warm and shades cool, and the light (`PRE-30`) falls on it smoothly. Its texture is quiet, of marks two or more texture pixels wide that show what it is made of, and each zoom band's texture is drawn anew, never shrunk. Strong colour stays in small accents, except where the season itself is the colour, as in autumn, and each biome, hour and season has its colour plan."
  Done when: "no surface is speckled at any zoom stop on your phone, and in a summer daylight scene strong colour stays in small accents."
  *Why:* answers 6, 7, 21, 31, 33 and 36; the measures of 3.4.
- **P-4, `PRE-21` Outlines and lit edges** (title "Edges and contact"). "Up close nothing is outlined: things stand apart by their light, shade and colour. A bright edge shows where the sun, the sky or a fire catches a shape (`PRE-30`), such as a cliff's sunlit rim or a person's fire-facing side, and corners, undersides and the ground where things stand darken. Small far figures are outlined (`PRE-28`)."
  Done when: "at the person and close camp zooms no shape has a drawn outline, and every figure in sun or firelight shows its lit edge and its contact shade."
  *Why:* answer 7 ("As I liked": no outlines, darkening in corners and under things).
- **P-5, `PRE-22` Stable pixels.** "Texture pixels never crawl or shimmer while the camera is still, panning, turning or zooming: each stays a crisp square moving smoothly with its surface. A texture pixel shows as about 2 by 2 screen pixels at every zoom: textures hold about 64 of them a metre at the person zoom, and each zoom band farther out has its own textures, drawn as pixel art at half the density of the band before and blended so nothing pops, so small things are drawn more simply as you zoom out, in portrait and landscape."
  Done when: "with the camera still, panning, turning or zooming, no texture crawls or shimmers on your phone, and a texture pixel measures 1.5 to 3 screen pixels at every zoom stop."
  *Why:* answers 1, 2 and 35; questions 31 and 36.
- **P-6, `PRE-27` People and animals.** "Detailed 3D figures, with a separate head, torso, arms and legs, posed about 10 times a second (`PRE-44`), wearing the world's pixel-art textures: up close their faces, hair, seams, trims and wear read, as in your liked pictures." How it works and Done when unchanged.
  *Why:* answers 3, 14, 19 and 20. *For animation:* at 200 screen pixels a figure's joints must show no gaps or blocks, and walking comes from code, since GPT's walk repeated two poses (answer 20, study 5); whether 10 poses a second suits a smooth world is section 7's question 7.
- **P-7, `PRE-28` Readable from far away.** "Zooming out, people and animals become small figures drawn to read: outlined in a darker shade of their own colour, faces and light clothes kept light, tools drawn larger; a group or herd close together becomes one marker, and a camp a point at its hearth that glows if it has a fire. Up close, people are among the first things you see in any scene, helped only by light from the sun, the sky and fires, and by their movement; nothing in the world is changed to make them stand out."
  Done when: "a camp of 30 people stays readable at every zoom stop, with no jump as its forms change, and in the review's busy scenes (an autumn wood, a crowded camp, a night camp) you find every person at a glance as they move."
  *Why:* answers 4, 27 and 33.
- **P-8, `PRE-03` Seamless zoom, the near stops.** "person: about 8 m across in portrait, a standing adult about 200 screen pixels and about 100 texture pixels tall (about 13 mm on your phone); close camp: about 20–50 m, a person about 35–90 screen pixels tall, every figure in full".
  *Why:* answers 5 and 35; the art-pixel counts were set for the old 4 × 4 art pixel (section 2). At 64 texture pixels a metre a 1.65 m adult is about 106 texture pixels, each about 2 screen pixels.
- **P-9, `MIL-09` The graphics engine.** "a sharp 3D world at the screen's full resolution wearing pixel-art textures (`PRE-01`, `PRE-02`) that goes well beyond the art book's pictures, which were preliminary, toward the pictures you liked: steady texture pixels (`PRE-22`), edges by light and shade (`PRE-21`), colour by design under the light of the hours and seasons (`PRE-20`, `PRE-30`), rock faces (`PRE-23`), water (`PRE-26`), and the model kit and its textures (`PRE-46`, `PRE-42`, `PRE-43`)." How the kit and textures are made is study 5's proposal.
- **Knock-on changes for the owner's OK:** `PRE-26`'s Done when, "every river is at least one art pixel wide", becomes "at least 2 screen pixels wide"; `PLT-02`'s "the art pixel's size (`PRE-22`)" becomes "the texture pixels' size"; the glossary's "Art pixel" becomes "Texture pixel: one pixel of a texture, shown as about 2 by 2 screen pixels at every zoom (`PRE-22`)".

### 6.6 Proposed changes to `ARCHITECTURE.md`

- **A5.2** replaced by section 6.2's guidance, and **A5.3** by section 6.3.
- **A4.1 and A4.2** (study 2's to write): full resolution, texture filtering, our own corner and contact darkening, ground as what covers it, no outline pass.
- **A8.3:** far figures "drawn to read" as in P-7.

## 7. Questions only the phone can answer

1. **The chosen texel in the engine.** Answer 35 was judged on still pictures: the same camp drawn by the engine with its own texture filter and zoom bands, at the closest zoom and the close camp, beside the liked picture, to confirm that 2 screen pixels still reads as the liked grain and not as noise.
2. **Steady texels.** A free turn, a pinch and a pan over dense ground and a cliff, recorded on the phone and viewed frame by frame: any crawl, shimmer or pop between zoom bands.
3. **People at a glance, in motion and in shade.** The busy autumn wood and a crowded camp, with people both in sun and in shade, still and moving, with the light aids on and off (only light with a real source: the sun's rim, the sky, a fire, ground bounce): how long the owner takes to point at every person (a stopwatch is enough); target about a second. This decides whether R3's glance aid is needed.
4. **Half-rate shading.** B shaded at full rate and at half rate with full-resolution edges, side by side: can the owner tell, and do texel edges crawl? Question 25 used a harsher stand-in.
5. **Zoom bands in motion.** The close camp's ground as chosen (answer 36) and the closest zoom's, while pinching between them: does the change of band show as a pop or a swim?
6. **Nights on the screen.** The chosen nights at night-time brightness: do the darks hold their steps or band, and do people still read by firelight?
7. **Ten poses a second up close.** A detailed figure 200 pixels tall stepping 10 times a second against a smooth world: charming or jerky? 10, 15 and 30 side by side.
8. **Far ground with and without grain** (`far-camp-zoom-far-pixel-vs-smooth.png`): does the owner see a difference at true size?

## 8. Risks

| Risk | How to retire it early |
|---|---|
| Direction B at full resolution with the liked density does not fit the phone's frame time, heat and battery (study 4's first estimate: 4.5–12 ms at the closest zoom with savings, against an 8 ms line) | Study 4's stress scenes first; the savings that do not show; half-rate shading where it cannot be seen (question 4) |
| Texels shimmer or zoom bands pop | The texture filter and band blending built first, judged on recorded frames (question 2) |
| People lost in busy scenes, above all in shade, where light alone did not help (answer 33) | Rule 9 of 6.2 measured on every contact sheet; question 3 on the phone, in motion; R3 in reserve |
| GPT's pictures carry later things into targets and assets | The truth check of 6.4 before any picture becomes a target |
| Summer colour drifts toward GPT's: my daylight direction-B pictures hold 13–18% strong colour against the liked camp's 11% | The liked pictures stay the reference; rule 4 of 6.2 on every sheet |
| Grain reads as speckle (the art book's "grainy" verdict, question 31) | Marks of 2 to 3 texels, quiet contrast, each band's own level (answer 36); question 5 |
| A light aid with no source in the world | Every light from the sun, the sky or a fire; a fill that follows people breaks `PRN-10` (study 2) |
| The rock surface under the layers is not right yet (answer 34) | Studies 5 and 7 try surfaces under the code-laid layers, judged at true size beside the liked cliff |

## 9. For other studies

- **Study 2 (how to draw):** the texture filter for crisp texels without shimmer, at about 2 screen pixels at every zoom (answer 35); zoom-band levels drawn as pixel art and blended; for readability, the sun's and fires' rims, sky light, ground bounce and contact shadows, with no sourceless fill (your own finding), and leaves thinning over a person as a view aid like a cut-away; colour plans as data read by the shared light; the far ground may be plain filtered colour under crisp things (3.6); half-rate shading risks crawling texel edges.
- **Study 3 (the engine):** colour plans per biome, hour and season as catalogue data; traces (trodden paths, working floors) drawn where the world made them; figures drawn to read at a distance; an optional glance aid (R3), outside the world's picture.
- **Study 4 (the phone):** texture memory at 64 texels a metre up close (answer 35), a quarter of it for each band farther out; half-rate shading's risk to texel edges; people's movement as the cheapest readability aid.
- **Study 5 (content):** marks of 2 to 3 texels; a pixel-art level per zoom band, drawn rather than shrunk (answers 21, 31 and 36); the rock surface under the code-laid layers (answer 34); species' true season colours; materials' true colour ranges (pale and smoked hides), so the world's own variety sometimes gives contrast; the truth check of 6.4 for every GPT source picture.
- **Study 6 (the loop):** the measured guidance of 6.2 for the target card (dark share, strong colour share, greens, nights' warm share, winter's very dark share, and people's salience: a median of about the 80th percentile, none below about the 70th); show the owner sheets at true size, since question 31's scale changed the verdict's basis.
- **Study 7 (the pipeline):** texel densities by band, 64 a metre at the closest band and half at each band farther out (answer 35); a level per band made as in answer 36 (redrawn on the band's grid, colours matched by code); target pictures stored with their truth flags.

## 10. Pictures from GPT

Sixteen of my 32 used; a seventeenth request (`autumn-levers.txt`) was held by the builder and not drawn. All are in `R/work2/1/gpt/`, each beside its request (`.txt`).

| Picture | Question | What it showed |
|---|---|---|
| `close-camp.png` | the liked look from the game's camera at the close camp, about 17 m across | the feeling holds; people about 95 pixels tall read |
| `camp-zoom.png` | the same at the camp zoom | the feeling holds far away; picked in answer 12 |
| `close-camp-night.png` | night | dark and cool with small fire pools; picked in answer 10 |
| `close-camp-winter.png` | winter | brighter snow, deep blue river; not picked (answer 11) |
| `close-camp-noon.png` | true midday | picked over a golden noon (answer 9) |
| `close-camp-overcast.png` | without the sun | worse (answer 8) |
| `close-camp-sparse.png` | without the dense plants | worse (answer 8) |
| `camp-zoom-far.png` | the camp zoom at dusk, wider | picked (answer 12) |
| `d2-pixel-paint.png` | direction B itself | picked (answer 1); grain 4–5 pixels, though 3 was asked |
| `lake-mist-close.png` | mist with no horizon | keeps the feeling (answer 18) |
| `first-cave.png` | the first scene of every world, nothing made | "as beautiful as the rest" (answer 28) |
| `village-truthful.png` | the village without later things | keeps the feeling (answer 29) |
| `storm-close.png` | the storm with no sky | keeps the drama (answer 30) |
| `camp-thirty.png` | thirty people at ten tasks | "alive" (answer 26); people rank 91st–99.9th in salience |
| `autumn-close.png` | people in a busy autumn wood | found "with effort" (answer 27); people 55th–88th |
| `autumn-readable.png` | the same wood with the world changed: calmer leaves, trodden ground, paler clothes, light | people 81st–89th, mostly from the clothes, which the look cannot choose (3.7); passed over for light only (answer 33) |
| `autumn-levers.txt` (not drawn) | the world's levers with the clothes kept | held by the builder, since the owner had passed over changing the world |

## 11. Sources

**The owner and the project**
- `R/owner/ask/answers.md`, the owner's 36 answers of 6 October 2026: "As I liked" (answers 6 and 7); "Have multiple types that they can choose from, all of these are nice" (answer 13).
- `PROJECT.md`: `MIL-09`, `PRE-01` to `PRE-03`, `PRE-20` to `PRE-28`, `PRE-30`, `PRE-42`, `PRN-10`, `CUL-12`; `ARCHITECTURE.md` A4.1, A4.2, A5.2, A5.3, A8.3. Quotes in sections 2 and 3.3 are from these files as of commit `4320212`.
- Git commit `7874f41` (4 October 2026, the art book's second edition): "The owner found some shapes hard to read through grainy textures".

**Pixel textures on a sharp 3D world**
- Joe Skrebels, "Minecraft: Vibrant Visuals Transforms the Game Into What You've Always Imagined in Your Head", Xbox Wire, 25 March 2025, quoting art director Jasper Boerstra: https://news.xbox.com/en-us/2025/03/25/minecraft-vibrant-visuals/
  - "When you play, you'll notice that the shadows in the game are pixelated. All the reflections are pixelated, too."
  - "We wanted to keep it *Minecraft*-y, and Vibrant Visuals is a great way to bridge familiar elements with new features."
- Thomas Frick (art director), "An introduction to making models for Hytale", Hytale, 22 December 2025: https://hytale.com/news/2025/12/an-introduction-to-making-models-for-hytale
  - "A modern, stylized voxel game, with retro pixel-art textures"
  - "By fully leveraging modern game engine capabilities while preserving the charm of our old-school pixel art, we are at the intersection of low-definition pixel art and hand-painted 3D."
  - "We paint lights and shadows inside textures and use real lights/shadows to bring everything together."
  - "The shading modes, shadowmap, SSAO, fog, and bloom we apply to our world are the cherry on top, making Hytale feel cozy and vibrant."
  - "Ultimately, even without any effects, our models should look good on their own."
  - "Make a Character/Attachment (Cosmetic, Tool, Weapon, Food item) = Density will be 64px per unit"
  - "Make a Prop/block (anything else from cubes to furniture) = Density will be 32px per unit"
  - "We avoid noise, too much grain, or perfectly flat surfaces."
- Iain Harris, "Valheim devs were inspired by Zelda: Breath of the Wild and PS1 graphics", PCGamesN, 11 March 2021, quoting Richard Svensson of Iron Gate: https://www.pcgamesn.com/valheim/ps1-graphics
  - "That being said, the low-detail stylings of the world are spruced up quite a bit by a plethora of ambient effects and post-processing."
- t3ssel8r, video descriptions on YouTube (read from the channel's feed, saved in round 1 as `R/work/1/t3-feed.xml`):
  - "Crafting a Better Shader for Pixel Art Upscaling", 26 May 2023, https://www.youtube.com/watch?v=d6tp43wZqps: "Applications for this technique range from anti-aliasing for pixel-art textures in low-poly games like Minecraft, to games that primarily feature pixel art sprites in 3D environments, like Octopath Traveler."
  - "Pixel Art Rain Shader", 30 November 2020, https://www.youtube.com/watch?v=ony4o3E0J20: "Selling the atmosphere of rain is a matter of layering together a bunch of individual effects:"; "streaks representing rain drops"; "splashes on all gently-sloped ground"; "ripples in the water"; "mist blowing around the scene".
- 80 Level, interview with Dominick John on Project Shadowglass, 19 May 2026: https://80.lv/articles/interview-how-project-shadowglass-creates-its-impossible-fully-3d-pixel-art-look
  - "Characters and objects sometimes have to look very different far away than they do up close, otherwise they can read as pixelated noise instead of coherent forms."
- Adam Robinson-Yu, "Crafting a tiny open world: A look behind the scenes at the creation of A Short Hike", PlayStation Blog, 5 August 2021: https://blog.playstation.com/2021/08/05/crafting-a-tiny-open-world-a-look-behind-the-scenes-at-the-creation-of-a-short-hike/
  - "I added a soft outline effect to objects to help them stand out, and stay readable with so few pixels."
- 80 Level, "Kingdom Developer on Creating Pixel Environments" (Thomas van den Berg), 11 May 2023: https://80.lv/articles/kingdom-developer-on-creating-pixel-environments/
  - "pixels should be the same size across the model"

**Detail by distance, colour and clarity**
- Slynyrd, "Pixelblog 62 - Landscape Backgrounds", 3 June 2026: https://www.slynyrd.com/blog/2026/5/27/pixelblog-62-landscape-backgrounds
  - "In the nearest plane, there are thick apparent blades of vegetation, while the next plane only uses one, or two pixel tall clusters at the most."
  - "The subsequent planes no longer depict single blades of vegetation at such a great distance."
- Slynyrd, "Pixelblog 15 - Plant Life", 2019: https://www.slynyrd.com/blog/2019/3/7/pixelblog-15-plant-life
  - "Not every leaf needs to be represented."
- Slynyrd, "Pixelblog 20 - Top Down Tiles", 2019: https://www.slynyrd.com/blog/2019/8/27/pixelblog-20-top-down-tiles
  - "You only need a few colors to make a rich texture. Too many colors and the texture will become blurry."
  - "A busy texture next to another busy texture may exhaust the eyes with too much noise. Negative space is your friend."
- Jason Mitchell, Moby Francke, Dhabih Eng, "Illustrative Rendering in Team Fortress 2", Valve, 2007 (older, still the clearest): https://www.cs.princeton.edu/courses/archive/fall07/cos597B/papers/mitchell-team-fortress.pdf
  - "with muted colors dominating and small areas of saturation to give further visual interest."
  - "Shadows go to cool, not black."
  - "Even when viewed only in silhouette with no internal shading at all, the characters are readily identifiable to players."
- Riot Games, "Clarity in League", 12 March 2021: https://www.leagueoflegends.com/en-us/news/dev/clarity-in-league/
  - "the most important thing at any given moment (like a major ultimate or CC) should draw the most attention"
  - "Finally, noise should be kept minimal."

**Seeing and finding**
- Jeremy M. Wolfe and Todd S. Horowitz, "Five factors that guide attention in visual search", Nature Human Behaviour, 2017 (author's copy on PMC): https://pmc.ncbi.nlm.nih.gov/articles/PMC9879335
  - "Salience of a target increases with difference from the distractors (target-distractor – TD- heterogeneity) and with the homogeneity of the distractors (distractor-distractor –DD- homogeneity) along basic feature dimensions."
  - "There are probably only a couple dozen attributes that can guide attention." (Its Table 1 lists colour and motion among the "Undoubted Guiding Attributes".)
- Laurent Itti, Christof Koch, Ernst Niebur, "A Model of Saliency-Based Visual Attention for Rapid Scene Analysis", IEEE Transactions on Pattern Analysis and Machine Intelligence, 1998 (older; the basis of my measure): https://ilab.usc.edu/publications/Itti_etal98pami.html
  - "Multiscale image features are combined into a single topographical saliency map."
- The phone's numbers (0.065 mm a screen pixel; one minute of arc; 32 to 36 cm) are round 1's, with their sources there (Webvision; Bababekova and others, 2011).

**Stone Age truth**
- Willy Tegel and others, "Early Neolithic Water Wells Reveal the World's Oldest Wood Architecture", PLOS ONE, 2012: https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0051374
  - "A total of 151 oak timbers preserved in a waterlogged environment were dated between 5469 and 5098 BC"
  - "The logs were split first in half with wooden wedges that were hammered in using wooden mauls."
  - "Stone adzes with transversely hafted blades were used, and the felling cuts were placed just above breast height."
  - "All of the chest-like well linings were constructed using notched timbers that were either cogged or interlocked at their corner joints (Figure S17)."
- Joris Peters and others, "The biocultural origins and dispersal of domestic chickens", PNAS, 2022: https://pmc.ncbi.nlm.nih.gov/articles/PMC9214543
  - "the first unambiguous domestic chicken bones are found at Neolithic Ban Non Wat in central Thailand dated to ∼1650 to 1250 BCE"
  - "in Ethiopia and Mediterranean Europe by ∼800 BCE"
- Arne Ludwig and others, "Coat Color Variation at the Beginning of Horse Domestication", Science, 2009 (author's copy on PMC): https://pmc.ncbi.nlm.nih.gov/articles/PMC5102060
  - "We found no variation in the Siberian and European Pleistocene horses, suggesting that these horses were bay or bay-dun in color."
  - "The Tobiano spotting was first found in a single Eastern European sample (3500 to 3000 yr B.P.) and later also in Asia."
- Encyclopaedia Britannica, "Przewalski's horse" (secondary): https://www.britannica.com/EBchecked/topic/481051
  - "The mane is short and erect with no forelock."
- Smithsonian's National Zoo and Conservation Biology Institute, "Przewalski's horse" (first-hand: the zoo breeds them): https://nationalzoo.si.edu/animals/przewalskis-horse
  - "They are dun-colored with a dark zebra-like erect mane and no forelock."
- Game & Wildlife Conservation Trust, species of the month, June 2015 (wild boar): https://gwct.org.uk/wildlife/species-of-the-month/2015/june/
  - "These stripes are lost when the piglet is 3 to 4 months old, when the piglet then takes on a red colouration (reminiscent of a red squirrel's colouration) until it becomes an adult at approximately one year of age."
  - "Farrowing (giving birth) can occur at just about any time but peaks in April, with a typical litter size usually being between 4-6 piglets."
- Wikipedia, "Pesse canoe" (secondary): https://en.wikipedia.org/wiki/Pesse_canoe
  - "It was made from a single Scots pine log."
  - "Carbon dating indicates that the boat was constructed during the early Mesolithic period between 8040 BC and 7510 BC."
- Gene Ball, "The Tipi Blends Function and Elegance", on the Plains tipi, Buffalo Bill Center of the West, Points West, Summer 2016 (written in 1979; online 13 March 2020): https://centerofthewest.org/2020/03/13/points-west-tipi-function-elegance/
  - "Smoke holes extended down the front and had adjustable, ear-like flaps on each side."
- Niall J. O'Sullivan and others, "A whole mitochondria analysis of the Tyrolean Iceman's leather provides insights into the animal sources of Copper Age clothing", Scientific Reports, 2016: https://pmc.ncbi.nlm.nih.gov/articles/PMC4989873
  - "Results indicate that the majority of the samples originate from domestic ungulate species (cattle, sheep and goat)"
  - "Intriguingly, the hat and quiver samples were produced from wild species, brown bear and roe deer respectively."
- Allen Denoyer, "Hands-on Archaeology: How to Make Flintknapping Tools", Archaeology Southwest, 27 October 2016: https://archaeologysouthwest.org/2016/10/27/hands-on-archaeology-how-to-make-flintknapping-tools/
  - "The tool just left of the pecking stone is called a billet, and I made it out of the base of a mule deer antler."
- J. E. Quibell, "Excavations at Saqqara (1911–12): The Tomb of Hesy", Cairo, 1913, page 26 (the excavator's report on a tomb of King Neterkhet's Third Dynasty): https://archive.org/details/cu31924028671299
  - "It will be noted that real cooper's work is intended, — barrels with bevelled staves."
- Wikipedia, "Cooper (profession)" (secondary, for the date): https://en.wikipedia.org/wiki/Cooper_(profession)
  - "An Egyptian wall-painting in the tomb of Hesy-Ra, dating to 2600 BC, shows a wooden tub made of staves, bound together with wooden hoops, and used to measure."
- Valentine Roux and Ortal Harush, "Unveiling the sign value of early potter's wheels based on a 3-D morphometric analysis of Late Chalcolithic vessels from the southern Levant", Journal of Archaeological Science: Reports 45, 2022 (its abstract, on Bar-Ilan University's research portal): https://cris.biu.ac.il/en/publications/unveiling-the-sign-value-of-early-potters-wheels-based-on-a-3-d-m/
  - "The sign value of the first potter's wheels used in the southern Levant (second half of the 5th mill. BC) is explored through the production modalities of V-shaped bowls, the main category of vessels shaped on the wheel at that time."
- Cornelia Becker and others, "The Textile Revolution. Research into the Origin and Spread of Wool Production between the Near East and Central Europe", eTopoi, Journal for Ancient Studies, Special Volume 6, 2016 (its abstract): https://edition-topoi.org/article/1092-the-textile-revolution
  - "The objective of the research group (A-4) Textile Revolution is to contribute to research on the still largely unclear introduction of wool production in later Neolithic and Chalcolithic societies from Western Asia to Central Europe."
  - "For the later part of the presumably long-lasting development of wool production, written sources are available, the earliest of which date to the Late Uruk and Jemdet Nasr periods (end of the 4th to beginning of the 3rd millennium BCE) in Mesopotamia."
- Gabriel Anaya and others, "Ancient DNA Reveals the Earliest Evidence of Sheep Flocks During the Late Fourth and Third Millennia BC in Southern Iberia", Animals 14, 3693, 2024: https://pmc.ncbi.nlm.nih.gov/articles/PMC11672771
  - "with a more conservative view suggesting the probable absence of woolly sheep breeds in Western Europe before the 3rd millennium BCE"
- Not verified: the dates of lattice windows, a pot hung over the fire, a metal sickle, a jar shaped on a wheel and white fleece in a first-copper village, and whether any coopered vessel is older than Hesy's; each is marked "to check" in 6.4, as are the borrowed dress and gear, case by case.
