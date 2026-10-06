# Research 19: the graphics engine

**Question:** how should the graphics engine (`MIL-09`) draw the world so that it carries the feeling of the pictures you liked, live on your phone, at every zoom, hour and season?
What do others do, what can the phone afford, and what does Kindling take?

On 6 October 2026 seven studies researched the engine in two rounds.
Round 1 worked under the decided pixel-art rules: one palette, shades in steps, outlines, and the world drawn small and enlarged.
While it ran you saw GPT's first test pictures and said "I would like something like that", then "But what if we do faux 3d pixel art? Where the pixel is just the rendering?", then "Remove the strict rules … It's more about the feeling rather then stricy guidelines".
So round 2 put the feeling first, with the rules that stay (the principles, Stone Age truth and the phone's limits) and the lifted ones open.
The studies asked GPT, through the builder, for 62 example pictures to test their questions; with the 2 you liked and 10 more in their style, there were 74.
Every request was checked for its purpose and detail before it ran: two were edited for safety, five were restyled to the look you chose, and one was held because your answer had already settled it.
You then answered 36 questions on labelled pictures (`art/reviews/2026-10-06-graphics/`), and the studies finished with your answers.

The seven studies' own notes, with every measurement, option and source, are in `research/19/`:
1. [The feeling](19/1-look.md): what gives the pictures their feeling, and the look that carries it.
2. [Drawing it](19/2-drawing.md): how the engine draws that look on Godot's Mobile renderer.
3. [The engine](19/3-engine.md): the engine's parts and what they cost on the graphics chip.
4. [The phone](19/4-phone.md): what the phone affords, part by part, with budgets and the first measurements.
5. [Content](19/5-content.md): how an AI builder makes the models, textures, people, animals and plants.
6. [The loop](19/6-loop.md): how the builder reaches the look you love and keeps it.
7. [The pipeline](19/7-pipeline.md): how content gets from its sources to the phone.

Their numbers are estimates made in the cloud unless they say measured; the phone settles them.
Their quotes are checked against their pages ([the check](19/quote-check.md)).
The check changed no recommendation; it found that a potter's wheel and white woolly sheep are not clearly later than a first-copper village, so a village target dates them case by case, and that smoothing the edges of cut-out leaves moves them to Godot's blended pass, so it costs time the leaves' calibration scene measures.
The working files the notes name under `R/` (scripts, crops, GPT's originals) stayed in the research session and are not kept; the pictures you judged are in `art/reviews/2026-10-06-graphics/`, and the targets in `art/targets/`.

## What `PROJECT.md` asks of the graphics engine

When the research began, `MIL-09` named it: crisp 3D pixel art that goes well beyond the art book's preliminary pictures, with steady pixels, outlines and lit edges, colour in steps under the light of the hours and seasons, rock faces, water, and the model kit and its textures made by code.
Since your OK on this research, the items below carry the look you chose ([What you decided](#what-you-decided), 1).

**Lifted for this research,** as you asked, and to be reworded only with your OK (`PRC-07`):
- `PRE-01`'s one palette, each art pixel one solid colour, no smooth gradients;
- `PRE-20`'s 4 to 7 shades a material and no fine grain;
- `PRE-21`'s one-pixel outline wherever things overlap;
- `PRE-22`'s fixed art-pixel sizes, keeping steady pixels as a goal;
- `PRE-27`'s tiny blocks for people and animals;
- the art bible (A5.2) and its one texture density (A5.3);
- `MIL-09`'s and `PRE-46`'s "made by code" for textures, keeping the kit as one countable set.

**Not lifted:**
- the principles, above all nothing faked (`PRN-10`) and looking changes nothing (`WLD-13`);
- Stone Age truth from archaeology (`PRE-42`);
- the phone's limits (`PLT-01`, `PLT-04`): 60 frames a second, an hour's play at about 25–30% of the battery, never uncomfortably hot;
- portrait and landscape (`PLT-02`), the gestures (`PRE-33`), readability from far away (`PRE-28`), the zoom to the globe (`PRE-03`), and the kit's structure (`PRE-46`).

## What you chose

Your 36 answers, by theme, with each question's number in brackets; the pictures and your exact answers are in `art/reviews/2026-10-06-graphics/answers.md`.

**The look:**
- **Direction B:** a smooth, sharp 3D world at the phone's full resolution, wearing textures painted as pixel art (1), not the world drawn small and enlarged.
- **Texture pixels about 2 × 2 screen pixels** (2), and as fine up close: 64 a metre at the closest zoom (35).
- **Detailed people and animals** (3, 14), with people as big up close as in the picture you liked, about 8 m across the screen (5); small far figures drawn to read (4).

**Everything you liked, kept visibly:**
- the density, and the airy, wild plants (6);
- smooth light, no outlines up close, the fine grain, and darkening in corners and under things (7);
- both the sun and the plants carry the feeling (8).

To both questions that offered cheaper looks you answered "As I liked", and to half resolution "only if needed" (25), so savings must come from ways that don't show.

**Hours, seasons and distance:**
- true midday light (9);
- three nights, from moonlit to nine tenths dark, but not the one lit by many fires (10);
- the softer winter with clear water (11);
- far views as rich as the dusk valley pictures (12);
- facing a low sun keeps the feeling (16);
- rain with all four parts: streaks, splashes and wet shine, ripples, drifting mist (17).

**Truth costs nothing:** the lake mist without a horizon (18), the camp of thirty (26, "alive"), the first cave (28), the village without the later things (29) and the storm without a sky (30) all keep the feeling.

**Content:**
- several shelter types for people to choose from (13);
- the family sheet and the deer's poses (19, 20) and the birch sheet's style (22) feel right;
- surfaces made by code from your meadow, and swatches from words or redrawn from your picture, feel like yours; GPT's seamless repaint of the meadow did not (15).

**Problems to solve, not to cut:**
- small plants shrunk to game size lose their charm (21);
- the ground turns to speckle at the close camp (31); your pick there is GPT's redraw with its colours matched by code (36);
- in a busy autumn wood people are found only with effort (27); you chose to help with light only, leaving the world as it is (33);
- each world's rock layers laid on by code keep your cliff's feel, but neither rock surface tried under them does yet (34).

**Methods:**
- a dusk made by relighting an approved picture, not by repainting the engine's frame (23);
- a colour change alone doesn't bring the feeling (24);
- half-resolution shading only if needed (25).

## How others do it, and what we found

### 1. The feeling

What the liked pictures share was measured across all twelve:
- warm lights, and strong colour only in small accents (11% of the picture or less in eleven of the twelve);
- muted, yellowish greens;
- real darks (about a quarter of a daylight picture, 86% of a night);
- one cool mass of clear water, snow or blue shade;
- nature dense and countable, with quiet grain;
- people on a quiet stage, easy to find: in the camp you liked they rank between the 77th and 96th percentile of how much each point of the picture stands out.

Their visible "pixels" are 4 to 5 screen pixels in every picture, at any scale, and lie on no grid.
So direction B is the liked pictures' own look: sharp shapes with pixel-art surfaces.
Drawn in marks of 2 to 3 texture pixels of 2 screen pixels each, a texture gives the same grain.

Ten of the twelve pictures carry later or borrowed things: sawn wood, chickens, a winch well, boats with seats, spotted horses, tipi-like cones.
GPT also ignored explicit "avoid" lines, keeping striped piglets in autumn even when told not to.
So every target picture gets a written truth check before anyone aims at it ([study 1](19/1-look.md), 6.4).

### 2. Drawing it

- **Full resolution, straight to the screen,** 1080 × 2404 pixels, with 2× multisampling for smooth edges, which the chip maker calls "virtually free"; no blur passes, no glow, no outline pass.
- **Steady texture pixels come from how a texture is read, not from the camera:** a "smooth pixel" filter keeps each texture pixel crisp inside and softens only its edge over one screen pixel.
  In simulations it flickers on 0.4–1.1% of pixels as the camera moves, against 17–28% for plain sampling, and keeps 94–98% of the crispness.
- **Each texture has a level drawn for each zoom band:** 64 texture pixels a metre up close, then 32, 16 and so on, so a texture pixel stays about 2 screen pixels at every zoom.
  Shrinking a fine picture instead is what turned the ground to speckle and took the plants' charm.
- **The camera up close needs a narrow view** (about 5° to 10° across) or none: a wide lens at this tilt squashes texture pixels at the top of a portrait screen.
- **Light from one shared light function:** the sun by its height, the sky's fill, light bounced from the ground, backlight on leaves and rims, firelight, moonlight, haze, mist, snow and wetness.
  The Mobile renderer has neither soft shadows that widen with distance nor darkening in corners, so the engine builds both.
- **Your chosen moments set the light's targets:**
  - the nights are 85–87% dark with blue shade, and a small hearth's light halves within about 2 m;
  - sunlit snow is cream and shaded snow blue;
  - the phone's 10-bit picture steps visibly in the darkest tones, so nights need a fine dither ("debanding").

### 3. The engine

- **Round 1's layout holds:**
  - C++ families drawing through Godot's rendering server;
  - changes fed to a copy the view keeps;
  - one tree of ground;
  - one camera rig.

  The richer look makes uploading once and then only changes matter more.
- **What changes inside:**
  - small things are separate shapes only while they cover about 12 screen pixels or more; anything smaller is painted into the ground's band texture, so nothing pops;
  - ground cover is set out by the graphics chip from each patch's data, so the world never tracks single tufts;
  - airy plants are cut close to their leaves, solid in the middle and drawn after solid things;
  - detailed figures are bent on the chip from bone palettes posed in C++ at 10 a second;
  - every fire lights through one shared grid;
  - darkening is baked into each shape and read from a map seen from above.
- **Readability tools,** from least to most added:
  1. small figures drawn to read;
  2. plants parting around walkers, as real grass does;
  3. faint silhouettes of people behind leaves;
  4. crowns thinned near the camera;
  5. a hold that marks every person.

  The first two fit the rules as they stand; the rest draw something only for your eye.

### 4. The phone

- **Drawn plainly, the look doesn't fit:** full resolution is four times the pixels of the old 2 × 2 picture, and the liked camp needs about 13–45 ms of the graphics chip a frame, against an 8 ms line.
- **Savings that don't show bring it close, not under:** about 5–23 ms, with a central estimate of about 10 ms at the closest zoom and 9–13 ms at the close camp.
  The largest are:
  - darkening baked into shapes and maps (2–5 ms);
  - fire shadows from a small map per fire;
  - a sun shadow map with only big things in it;
  - the water's mirror at half resolution;
  - fire's glow drawn as sprites;
  - no outline pass.
- **Two changes to Godot itself would close most of the gap:**
  - a first pass that settles which leaves are hidden (it changes nothing on screen);
  - shading once per 2 × 2 pixels on ground and plants, which the phone's driver reports it supports but stock Godot never uses, and which may show faintly up close.

  Together they bring the central estimates to about 6–8 ms.
- **Fallbacks, in the order they show:**
  1. the 3D drawn at 0.75 scale in the costliest scenes only, which looks softer;
  2. half resolution, which you could see, last.

  Density is never cut.
- **Battery fits, heat decides:**
  - at 8 ms the whole phone draws about 3–5.5 W, 16–28% of the battery an hour;
  - comfortable long play needs about 4 W or less, so a 20-minute heat run sets the true line.
- **The texture pixel's size costs no time,** only memory.

### 5. Content

- **Code builds the kit and moves it; three routes paint its surfaces:** code, the world itself (rock layers, soot, wetness, snow, traces), and pictures you approved, prepared by code.
  Every picture is a checked guide or source, never finished art.
- **Every small thing is drawn for each band it is seen at, never only shrunk:**
  - asked for three sizes, GPT redraws a plant with fewer, bolder parts and keeps its flowers and berries;
  - but it can't draw on the true small grid, and a filter that removes speckle turns ground into camouflage;
  - so the small sizes come from code rules and tiny pixel designs written as data (about 100), with GPT's sheets as guides.
- **People and animals:**
  - made by code around one skeleton per body pattern, with garments as shells, as Spore built its creatures;
  - at the closest zoom a person is about 100 texture pixels tall and a face about 14, drawn as a small design for each band;
  - still poses follow approved sheets; walking, running and galloping come from numbers for each leg.
- **Shelters in six types, each tied to an excavation:**
  - hides closing off a rock shelter;
  - skin tents held down by stone rings;
  - round post huts;
  - mammoth-bone circles;
  - brush huts;
  - longhouses, for farmers.
- **Truth:** GPT drew metal tools despite "no metal", and the Iceman museum now calls his "grass cape" a mat, so the grass rain cape stays out until other evidence turns up.

### 6. The loop

- **Targets:** one picture you approve for each place, relit for its other hours and seasons.
  Six of the nine relit pictures you judged kept the feeling, and you chose a relit dusk over a repainted one.
  Repainting the engine's own frames drifts darker and busier round after round, and can't add what a plain frame lacks.
- **A target card measured from your pictures warns, never decides:** tested on your 36 answers, it would have flagged 12 of the 18 pictures you turned down, but also 6 of the 17 you picked.
- **New checks:**
  - shimmer as the camera moves;
  - accents on the ground at every zoom: the speckled ground scores 11–12, against 23–30 for ground you accepted;
  - how much people stand out.
- **A saving stays only if it can't be seen:** half resolution, which you could see, sets the scale, and each saving passes a blind test on your phone.
  Half resolution applied to textures also brings the shimmer back (9–16% of pixels), so any half-resolution saving may touch only smooth things: light, shadow, haze.
- **An AI judge advises, never decides:** in a May 2026 test the best model named both the best and the worst picture of a set, whatever the order they were shown in, in 26.5% of tasks, against 68.9% for experts, so it first sits an exam on your answers.

### 7. The pipeline

- **One source per graphics element, as before:** catalogue text, C++ generators, shader files, or a picture you approved with a small record of where it came from.
- **Every picture-made texture is brought onto an exact pixel grid by code.** No picture has one, not even yours; study 5's swatches lose only 2–8% of their colour detail.
- **Each zoom band gets a designed level,** checked against the level above: averaging loses about a fifth of the ground's colour accents.
  GPT redrew the close camp's level from the closest one in under a minute, keeping its layout and accents, and code matched its colours: your pick for question 36.
- **Pixel art is stored without lossy compression,** as Godot's own documentation advises, with our own levels.
- **Colour grades are small text settings** for each moment and season, with a true midday: never the look itself.
- **GPT stays outside the build:** every run is recorded, and at a limit the builder waits, never buying credits.
- **Committed phone builds already fill 1,023 MB** of the repository's 1.06 GiB history.

### Where the studies disagreed, and how it is settled

| Question | The views | Settled by |
|---|---|---|
| Texture pixel size up close | 64 a metre, about 2 screen pixels (study 2) or 32, about 4, like the liked picture's own blocks and a quarter of the memory (study 7) | you: 64 (answer 35) |
| A first pass that hides leaves | saves 0.7–4.5 ms (study 4) or gains nothing on this chip (study 3) | the phone: the leaves' calibration scene |
| Shading once per 2 × 2 pixels | the driver reports it (study 4) or unknown (study 3); in a simulation on your picture it looked blocky, not soft | the phone's self-check lists it; your eye judges it where it is used |
| Fire shadows | walked through height maps (round 1) or a small map per fire (studies 2 and 4) | the phone: the fires' calibration scene; the engine keeps both |
| The camera up close | about 5° or none (study 2) or 5–10° (study 3) | one perspective rig, narrow up close; your eye against the liked pictures' flat look |
| Many figures | Godot's own skeletons for up to a few dozen (study 4) or bone palettes on the chip (study 3) | the phone: the figures' calibration scene; both paths stay |
| People in busy scenes | the world route lifted all four people (studies 1 and 2) or light only | you: light only (answer 33); every light needs a real source |
| How textures reach the phone | lossless in the app, delivered as release files (study 7) or made on the phone at first start (study 4) | made on the phone where they can be; see decision 4 below |
| Wordings for `PROJECT.md` | each study proposed its own | merged into one set (below) |

## What we take

**Decided by your answers:**
1. **The look:** direction B, a sharp 3D world at the phone's full resolution wearing pixel-art textures, texture pixels about 2 screen pixels at every zoom (64 a metre up close, then 32, 16 and so on), each band's level drawn, never shrunk.
2. **Keep what you liked, visibly:** dense, airy, wild plants; smooth light; no outlines up close; the fine grain; darkening in corners, under things and where they meet the ground.
3. **Light true to the hour and season:** true midday; dark nights with a few warm pools; soft winters; rain in four layers; dusk made by relighting an approved picture.
4. **Detailed people and animals,** as big up close as in your picture; small far figures drawn to read; people helped by real light only.
5. **Several shelter types,** each from an excavation; every small thing designed per band.
6. **Half resolution only where needed;** a colour grade is a finishing touch, never the look.

**The design, for the architecture:**
7. **The picture:** full resolution with 2× multisampling, the interface drawn over it, Godot's single on-chip pass kept; nothing in the main picture reads the screen or its depth.
8. **Steady texture pixels:** one shared way to read every texture (the smooth-pixel filter), levels drawn for each band and loaded as each texture's own mipmaps; moving patterns (water, flames, foam) step in whole texture pixels about 10 times a second if your eye agrees.
9. **The camera:** one perspective rig at every zoom, narrow up close and widening as it rises; no pixel lock; the ease that settles turns and zooms stays, now only for feel.
10. **Light:** one shared light function; soft shadows that widen with distance from a height map of big things, and Godot's sun map for small ones; fire light through a grid of every fire; darkening baked into shapes and read from a map from above; a tone curve that keeps snow and fire textured; debanding at night.
11. **Water, rain, snow, fire, smoke, lightning and foam** as study 2 sets out, all from the world's own weather, heat and water.
12. **Plants:** the liked density; small things into the ground's band texture below about 12 screen pixels; leaves cut close, solid in the middle, drawn after solid things; small plants cast no sun shadow.
13. **Figures:** a skeleton per body pattern, posed in C++ at 10 a second and bent on the chip; forms by height on screen (full, simple, small drawn to read, tiny, marker); held things ride on bones.
14. **Content:** code builds the kit; surfaces from code, the world, or approved pictures prepared by code; small things designed per band; every picture checked for truth, with its record.
15. **The pipeline:** picture-made textures re-gridded, given designed levels, calibrated, stored losslessly with their records; code-made and world-made textures made on the phone; GPT outside the build.
16. **The phone:** an 8 ms planning line for the busiest close scene, split into parts with a lever each; the savings that don't show first, then our own build of Godot where they are not enough, then one planned step under heat (your decisions 2 and 3); at most about 4 W over a 20-minute heat run; pass lines written before the first run (`RES-09`).
17. **The loop:** one approved picture per place, relit for its hours and seasons; the target card as a warning; checks for shimmer, ground accents and how much people stand out; savings proved invisible by a blind test on your phone; the AI judge advises only.

**M2's order,** merged from the studies:
1. **Measuring:** the bench's new readings (time for each pass, power, heat), the target card and its checks, and the AI judge's exam on your answers.
2. **Calibration on stand-in content, each with its decision written before the run:**
   - the fixed cost;
   - the full material at full resolution;
   - the cost of a triangle;
   - leaves at the liked density;
   - fires;
   - many figures.
3. **First light:** the camp under the cliff by the river at the closest zoom, in direction B, on your phone, with texture arrays and designed levels from the first day.
4. **The phone's risks:** the liked camp, the camp of thirty and night by the fire as stress scenes, then the 20-minute heat run; each lever that may show judged by you.
5. **The camp at every hour and season,** against its relit targets.
6. **The zoom bands and your three problems** (small plants, the ground's speckle, people in busy scenes), and landscape.

## What you decided

On 6 October 2026 you answered decisions 1, 2, 3 and 5; decisions 4, 6 and 7 wait until their time comes.

**1. The wording in `PROJECT.md`: "OK, all 13".**
The 13 changes below are written into their items.
Four more followed from them, and you OK'd them too: `PRE-31`'s review sheet adds clips of the camera moving and the busy scenes; `PRE-43`'s colours sit within each material's shades, as the ladders are gone; `RSK-11` gets new signs and a response, as the crawl fix it named is gone; and a new risk, `RSK-30`, says what happens if the look costs too much.
- `PRE-01` becomes *Pixel-art surfaces*: the sharp 3D world at full resolution with pixel-art textures; a saving stays only if you can't tell it apart in a blind test.
- `PRE-02` is drawn at full resolution, not low.
- `PRE-03`'s near stops are measured in screen pixels: a person about 200 tall up close.
- `PRE-20` becomes *Colour by design*: quiet painted materials, strong colour in accents, a colour plan per biome, hour and season.
- `PRE-21` becomes *Edges and contact*: no outlines up close, lit edges and contact darkening, small far figures outlined.
- `PRE-22` becomes *Steady texture pixels*: about 2 screen pixels at every zoom, designed levels per band, and a measured shimmer check.
- `PRE-26`'s rivers are at least 2 screen pixels wide.
- `PRE-27`'s figures are detailed, with a face for each band.
- `PRE-28`'s far figures are drawn to read, and people are found by real light and movement.
- `PRE-30` has no master palette, and noon is true white-warm light.
- `PRE-46` adds a design per zoom band for small things, and several shelter types.
- `MIL-09` is reworded for the chosen look and the three routes to textures.
- `PLT-02` keeps the texture pixel's size instead of the art pixel's.

**2. Your own build of Godot: "Yes, if needed".**
The two changes that close most of the gap (hiding leaves first, and shading per 2 × 2 pixels on ground and plants), and a third that keeps buffers off memory, are changes to Godot's own code.
Research 01 chose Godot unchanged.
You allowed our own build of Godot 4.7.2 for these patches only, built and tested in the cloud like everything else, if the calibration scenes show they are needed.

**3. If the picture alone heats the phone: "One planned step".**
The choice was between bringing the graphics line down for good, so the picture never changes under load, and one planned, logged step under heat, such as distant fires casting no shadows.
You chose the step.
As `PRE-01` keeps a saving only where you can't tell it from the full picture, the step is chosen among savings that pass the blind test; if none is enough, it comes back to you.

**4. How builds and textures reach your phone (when it arises).**
Builds are committed to the repository, which caps a build at 50 MB.
Picture-made textures at 64 a metre may not fit beside Godot within that.
- Start as study 4 recommends: make code-made and world-made textures on the phone at first start, and ship picture-made ones losslessly while they fit.
- If they stop fitting, either ship them compressed where you can't tell the difference in a blind test, or deliver builds as release files instead of commits (study 7), lifting the cap to about 150 MB.

  Making release files has not been tried from this cloud session, so that option needs a test first.

**5. Help in busy scenes that draws only for your eye: "Only if needed".**
Real light, plants parting around walkers, contact shadows and the clothes people really make all fit the rules.
Leaves thinning over a person only in the picture, silhouettes behind leaves, or a hold that briefly marks every person do not change the world (`WLD-13`), but show something that isn't there for show's sake (`PRN-10`).
So the first set is built, the phone measures how quickly you find people in shade, and you are asked only if that falls short.

**6. Later, on your phone, by eye:**
- the camera's view up close;
- whether figures hold each pose for a tenth of a second or glide between poses;
- whether moving patterns step in whole texture pixels;
- each lever that may show;
- the small plants per band;
- the close camp's ground in the engine;
- the cliff's rock surface (still open).

**7. Smaller questions:**
- whether worlds have mammoths, since a mammoth-bone shelter only makes sense where they live;
- for your information: OpenAI's current terms leave you owning the pictures GPT draws for you, and in the US purely AI-made pictures can't be copyrighted (study 5).

## What only the phone can settle

- What the full material costs a pixel at full resolution, and what a triangle costs in each pass.
- What cut-out leaves cost against solid ones, whether hiding them first helps, and what smoothing their edges costs in the blended pass.
- Whether the chip shades per 2 × 2 pixels as its driver reports, and whether you can see it.
- How many watts a millisecond costs, the phone's own throttling levels, and how long the busiest scene takes to reach them.
- Whether texture shimmer shows at the level the cloud measures.
- Whether nights band, and which savings are invisible.
- Whether the liked density reads at arm's length.
- Whether people in shade can be found at a glance.
- How long the first start takes to make the textures.
- Whether the driver handles every technique (the cloud's driver crashed on one smoke test in pre-production).

## Sources

The studies' notes list every source, each quote checked against its page; the main ones:

- The look and the feeling:
  - [Itti, Koch and Niebur 1998: a model of saliency-based visual attention](https://ilab.usc.edu/publications/Itti_etal98pami.html)
  - [Minecraft: Vibrant Visuals](https://news.xbox.com/en-us/2025/03/25/minecraft-vibrant-visuals/) and [its lighting settings](https://learn.microsoft.com/en-us/minecraft/creator/documents/vibrantvisuals/lightingcustomization?view=minecraft-bedrock-stable)
  - [Tegel and others 2012: early Neolithic water wells](https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0051374)
  - [Peters and others 2022: the origins and spread of domestic chickens](https://pmc.ncbi.nlm.nih.gov/articles/PMC9214543)
  - [Ludwig and others 2009: coat colours at the beginning of horse domestication](https://pmc.ncbi.nlm.nih.gov/articles/PMC5102060)
  - [O'Sullivan and others 2016: the animal sources of the Iceman's leather](https://pmc.ncbi.nlm.nih.gov/articles/PMC4989873)
- Drawing on the phone:
  - [Godot 4.7: 3D anti-aliasing](https://docs.godotengine.org/en/4.7/tutorials/3d/3d_antialiasing.html)
  - [Godot 4.7.2 source](https://github.com/godotengine/godot/tree/4.7.2-stable)
  - [Imagination: MSAA performance](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/msaa-performance.html), [hidden surface removal](https://docs.imgtec.com/starter-guides/powervr-architecture/html/topics/hidden-surface-removal-efficiency.html) and [triangle size](https://docs.imgtec.com/performance-guides/graphics-recommendations/html/topics/triangle-size.html)
  - [Vulkan hardware database: fragment shading rate on Android](https://vulkan.gpuinfo.org/listdevicescoverage.php?extension=VK_KHR_fragment_shading_rate&platform=android)
  - [Android: ADPF thermal](https://developer.android.com/games/optimize/adpf/thermal)
- Content and pipeline:
  - [Godot 4.7: importing images](https://docs.godotengine.org/en/4.7/tutorials/assets_pipeline/importing_images.html)
  - [Castaño: computing alpha mipmaps](https://ludicon.com/castano/blog/articles/computing-alpha-mipmaps/)
  - [South Tyrol Museum of Archaeology: the Iceman's clothing](https://www.iceman.it/en/oetzi/clothing)
  - [OpenAI: terms of use](https://openai.com/policies/row-terms-of-use/)
