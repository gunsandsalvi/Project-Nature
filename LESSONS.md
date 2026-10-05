# Kindling: what pre-production taught

Pre-production ran from 3 to 5 October 2026: thirteen throwaway prototypes, each answering one question on your phone or in the cloud (`MIL-18`).
You closed it on 5 October 2026 with P13, the writer, left unbuilt.
Its code is deleted; git history keeps it at commit `690c5b1`, but production writes its own code from this note and the architecture, and never copies a prototype's.

This note is what production takes from it: each prototype's question, answer and numbers, what went right and wrong, your observations, the traps we fell into, and what is still open.
The decisions it led to are in the architecture (`ARCHITECTURE.md`); this note keeps the evidence and the reasons.

## At a glance

| Prototype | The question | The answer | Your verdict |
|---|---|---|---|
| P1 The look | Can Godot draw the art book's close camp on your phone? | Yes: 99–100% of frames on time at 60, about 10 ms of graphics a frame | Chose outline way C and the camera's "ease" |
| P2 A full scene | Does a busy camp at night with three fires hold 60 frames a second? | Yes, at close and camp zoom; the forest needed trees of 12 triangles | Heat over 10 minutes still unmeasured |
| P3 The kit | Can the model kit be made by code, cheaply, and animated without skeletons? | Yes: 11 shapes, 2 plants and a deer in 23 ms at load; poses 10 times a second | Asked for fire shadows and lit smoke, then smaller turn steps |
| P4 Discovery pace | Can the world's own rules bring flakes and fire on time by tuning alone? | Yes: flakes within 2 years in 19 of 20 runs, fire in Years 2 to 8 in 15 of 20 | Halved the windows twice for faster discoveries |
| P5 The same bits | Do your phone and the cloud compute the same history? | Yes: the same digest on x86-64, arm64 under qemu and your phone, one thread and four | Proved on your phone |
| P6 A thousand minds | Do a thousand minds keep a game year a real minute? | Yes: 6.0 game years a real minute on your phone's four cores | Proved on your phone |
| P7 World generation | Is a world made within `WLD-11`'s times? | Yes: three worlds in 9.3 s and settling in 6.5 s on your phone, about twenty times the room | Proved on your phone |
| P8 The zoom | Does one pinch go from the globe to a person smoothly? | Built twice; the second round, one tree of ground, is the design | Asked for the second round; its Measure not yet sent |
| P9 Ecology | Does nature hold for a century with nobody in it? | Yes: every species within 0.66 and 1.10 of its total in all 20 worlds | Report read |
| P10 Culture from causes | Do customs, spirits, rites and splits come from causes alone? | Yes: a custom in 20 of 20 worlds, a spirit in 20, a rite in 15, a split in 17 | OK'd customs within a year (`CUL-33`) |
| P11 The director | Can a director slow time for the story without touching it? | Yes: 9.5 slowdowns an hour within budget, every world identical with it on and off | OK'd ages of any length (`PRE-39`) |
| P12 The interface | Do reach, gestures and crisp pixel text work both ways up? | Built and passing in the cloud | Your choice of ground and text size still to come |
| P13 The writer | Can the phone's own model word the book of ages? | Not built: pre-production closed first | Carried into production |
| P14 Sound | Do 32 sounds with distance, a cave's echo and the murmur play without breaks? | In the cloud yes: 32 at most, the mix 2.7% of the audio thread's time, 9% at worst | Your ears and Measure still to come |

## P1 The look (α0.2a)

**The question:** can Godot's Mobile renderer draw the art book's close camp, crisp and stable, on your phone's PowerVR chip?

**What it found:**
- **The pipeline:** the picture drawn small (an art pixel to 4 × 4 screen pixels, later 2 × 2 at the person growing with the zoom, `PRE-22`), shown with nearest sampling; a camera locked to whole art pixels, its leftover fraction shifting the image so pans are smooth.
- **The Mobile renderer draws forward,** so the painter's separate passes became one: each material hands its ramp, pattern, openness to the sky and firelight to one shared light function, which picks the step after the sun's shadow. Openness to the sky comes from a height map drawn once from above.
- **Outlines:** four ways were tried. C, a second low-resolution picture of each pixel's facing and depth, compared with its four neighbours, gives outlines and lit edges; it flickered until its picture was drawn first in each frame (a viewport's own viewports draw before it). Every shape needs its copy in that picture, or it reads as all edge.
- **Water:** the mirrored pass costs at most 0.3 ms on your phone, so reflections stay; the shore's line is drawn where the water is thinnest, from the depth texture.
- **Fidelity:** 87% of the close camp's pixels at noon and 76% at dusk came within 3 levels of 255 of the art book's.
- **On your phone** (4 October, 1080 × 2404): every outline way kept 99–100% of frames on time at 60, at 9.5 to 10.1 ms of graphics a frame, C the dearest by half a millisecond. Ways that cost a third more in the cloud cost under a tenth more on the phone: the chip lowers its clock when it has time to spare, so its milliseconds are partly idle.

**What went wrong:** outlines flickered as the camera moved (drawn after the picture, they were a frame late); turns snapped up to 7.5° after you let go, until the steps became 5°.

**Godot's rules met:** front faces wind clockwise, the opposite of three.js; a pass never declares the picture it draws into; varyings are written only in their own stage, and the light function has no vertex position; a shadow bias of 0.08 and a normal bias of 1.6 on a 4096 map keep dusk's low sun free of stripes.

## P2 A full scene (α0.2b)

**The question:** does a busy camp hold 60 frames a second at close and camp zoom?

**What it found:**
- A camp at night with thirty figures and three fires: 5.5 ms a frame at 120 frames a second, 99% on time; at 60 every frame on time.
- The forest at camp zoom, 12,000 trees of 44 triangles drawn three times (picture, outlines, shadow), took 17.8 ms with 65% on time; **trees for camp zoom take 12 triangles**, and then every frame was on time.
- **Godot stops lighting MultiMesh copies** past its per-object light limit, so fires reach figures and tents through a firelight term in our shaders, summing up to four fires; small creatures take 0.65 of a fire's light round their sides, so a figure by a fire reads lit from anywhere.
- Shaders work in world coordinates, so a moved or instanced shape gets its fire, sky and patterns where it stands.

**What went wrong:** P2's and P3's first Measure counted the picture's own pass only, leaving out the outline data, the mirror and the fires' height maps (17–31% more in the cloud); Measure was fixed to sum every pass. The heat over 10 minutes was never measured: Measure was cut to 90 seconds, and the heat forecast was still rising (0.56 to 0.59, then 0.61 to 0.66).

**The cloud's software Vulkan driver** crashed drawing P2's smoke from angles near 15°; the cloud's pictures were taken at 16°.

## P3 The kit (α0.2c)

**The question:** can the model kit be made by code at load, drawn as copies, and animated without skeletons?

**What it found:**
- Eleven shared shapes, two plants and a deer, each in two materials, built at load from a catalogue of parameters in about 23 ms (in GDScript).
- **One hut, two materials:** each MultiMesh copy carries its material's row, its pattern, its wear and a seed, and the shared shader colours the parts flagged to take them.
- **Animation:** walk and carry in 4 key poses, knap, scrape and rest in 2; each pose the angles at the joints, the body's bend, the head's nod and the pelvis's drop; with in-between poses every step shows a new pose, 10 a second (`PRE-44`), where held poses changed only 2.5 to 5 times.
- **Fire shadows, as you asked:** two height maps round the fires (48 m at 512 pixels, the tops seen from above and the undersides from below), drawn every frame; each pixel walks the line to each fire in up to 32 steps. A fire under an overhang lights the ground round it while a tent, a person or a windbreak stops its light.
- **Smoke, as you asked:** a lit volume along a path worked out at load (up from the fire, along the underside of a roof or overhang, then away on the wind), each pixel marching 24 steps; its blend solved against the screen's copy, since Godot blends in linear light.
- **On your phone:** the model sheet at night with three fires, their shadows and smoke, 4.3 ms a frame at 120, every frame on time at 60.

**Your comments taken:** the windbreak as the art book's poles, bar and brush, not a flat hide; the lean-to's roof in overlapping courses; figures in their own clothes, not one brown; turn steps of 5°.

## P4 Discovery pace (α0.3a)

**The question:** do the world's own rules, tuned, bring sharp flakes and fire inside `TIM-19`'s windows (`RSK-01`)?

**What it found** (a model of one band, in Python):
- **Flakes:** within 2 years in 19 of 20 runs, the median about eight months in; 20 of 20 on seeds never tuned against; never without stone that flakes. 17 came by accident, 2 by a dream's hunch, 1 by experiment; a year on, every adult could make them.
- **Fire,** counted at its first anywhere in a world of 3 or 4 bands: in Years 2 to 8 in 15 of 20 worlds and none before, the median about 4½ years in.
- **What sets the pace:** one discovery factor a blueprint, 0.413 for flakes and 0.136 for each way of making fire, tuned on 20 seeds and checked on 20 new ones.
- **No value holds it on a knife's edge:** each tuned value changed by a quarter either way keeps both steps' rules; the flake factor, noticing and experimenting fire are the strong levers.

**What went wrong:** with dreams unable to point at fire, the tuned factor brought fire into its window in 2 of 20 worlds and 7 never found it in 60 years (α0.3's review); a dream's hunch going to the sector the dreamer knows best is what makes most first fires begin with a dream.

**What production takes:** each blueprint's discovery factor in its catalogue entry; a step counted at its first anywhere in the world (`RES-07`); tuning runs that keep what they have done, keyed by the model's and the catalogue's version (`RSK-14`). P4's factors don't carry over: production re-tunes with the whole catalogue and dreams' choice among all blueprints, and `RSK-01` stays open until M7's scenes.

## P5 The same bits (α0.4a)

**The question:** does the same seed give the same history on your phone and in the cloud (`RES-05`, `RSK-04`)?

**What it found:**
- A toy world of 4,096 walkers on events, with keyed chance and our own sine, cosine, exponent, logarithm and power, each evening's sums gathered in fixed chunks of 256, ended each of its 30 days with the same checksum on x86-64 and on arm64 under qemu, on one thread and four (digest 1bbbe1d787b4d4fe).
- **On your phone** (5 October): the same digest on one thread and four, through the app's extension built with the NDK; P6's thousand minds ended their 3,546 game days on your phone with the checksum the cloud's replay reached.
- Our functions use only IEEE adds, multiplies and divides, with no contraction into fused multiply-adds, and agree with the platform's to within a few last bits.
- **Keyed chance:** SplitMix64's finaliser over the seed, the being, the tick, the purpose and an index.
- **The build:** each C++ library is also a Godot extension, built natively for the cloud's tests and for arm64 Android with the NDK, 16 KB aligned, so the app runs the very code the cloud runs; libraries are built, never committed.

## P6 A thousand minds (α0.4b)

**The question:** do a thousand minds keep a game year a real minute (`TIM-07`, `MND-15`, `RSK-02`)?

**What it found:**
- A thousand people in 40 bands, each with nine needs, 50 actions scored by response curves with their reasons kept, talk passing places, opinions and news, and trips by paths in levels: about 32,000 decisions a game day, about 32 each.
- **In the cloud:** one core about 7 game years a real minute, four about 12: a person costs about a seventh of `MND-15`'s thousandth of a second a game day.
- **On your phone** (5 October, four cores, 10 minutes): 6.0 game years a real minute once warm, 5.5 over the first 2; choosing 62% of the time, talk 17%, paths 9%, results landing 3%, the rest 10%; the heat forecast rose from 0.39 to 0.49 and never slowed it.
- So production's fuller minds may cost about 2½ times P6's before the speed falls below 2½ game years a minute.

**What went wrong:** paths first took four fifths of the time, with A* inside the first and last clusters of every trip and a cache per thread; fields of distances from each entrance, made at the start, and one cache shared by every thread brought them to a seventh. Four cores give less than twice one: what must happen in order between batches (results landing, talk, the snapshot of where people stand) is about a third of the time.

**What production takes:** decisions in batches of five game minutes, in fixed chunks, applied in entity order; a place in another connected region passed over before scoring; the planner for jobs of several steps still to build.

## P7 World generation (α0.5a)

**The question:** is a world generated within `WLD-11`'s 3 minutes, and settled within 1?

**What it found** (every stage of A7.2 in a first, simple form):
- **In the cloud,** four cores: 20 candidates at 512 × 256 cells in about 5 s, the best 4 again at 2,048 × 1,024 in about 12, so three worlds in about 17 s; settling 10 years about 10 s more. A candidate takes about 0.8 s on one core, a full world about 9: erosion half, the plates and rock a quarter, the climate an eighth.
- **On your phone,** four cores: three worlds in 9.3 s (candidates 2.5, the best 4 in 6.8) and settling in 6.5, faster than the cloud, its digest the cloud's; the heat forecast rose from 0.44 to 0.54.
- So generation has about twenty times the room `WLD-11` gives it, for richer stages: more erosion at full size, glaciers, and settling's real rules.

**What went wrong:** P7's map lit its hills from the south-east by mistake, which can make ridges read as valleys; maps are lit from the north-west.

## P8 The zoom (α0.5b, α0.5c)

**The question:** does one pinch go from the globe to a person over unvisited land, smooth at every stop (`PRE-03`)?

**The first round** built rings of ground as clipmaps do (every metre to 128 m, every 2 m to 300 m, every 40 m to 5 km, every 80 m to 16 km, then the map) handing over by a 4 × 4 ordered dither, and a flat map bending onto the globe for the last step. You saw the hand-overs and the map's bend jump.

**The second round,** after your verdict on 5 October, is the design:
- **One tree of ground:** 4 × 2 roots of 500 km, fourteen levels down to chunks 30.5 m across, each 33 × 33 points with skirts; a chunk splits while the camera is nearer than 2.4 of its sides, and morphs into its parent's grid over the last third of its reach (CDLOD), so nothing pops.
- **A planet at every scale,** never unrolled; each chunk's vertex shader sets it on the sphere by haversine forms exact near the focus.
- **Made as needed** on worker threads, nearest first, at most six at once; at most 900 kept, freed by when the tree last reached them. Freeing by when a chunk was last drawn freed the ones between, so a deep zoom asked for them again without end: the eight minutes the first pictures took.
- **Only what the camera can see:** at the river where the descent ends, 25 chunks drawn at the person, 38 at the close camp, 83 at the camp, 66 at the valley, 70 at the region, 21 at the world map and 8 at the globe, where without culling the person drew 989; choosing them takes about 1 to 2 ms a frame in the cloud.
- **Pixels:** the picture is drawn at the size of the pixels shown, an art pixel about 2 × 2 at the person growing to about 6 × 6 at the globe (`PRE-22`, your OK).
- **Clouds** from the climate (noise warped by noise, heaped, twisted round storm tracks, carried by the trade winds and the westerlies), marched through their 3D shapes and lit through themselves, in three variants; **air** scattering as Earth's does; **the sea** by depth; **rivers** as curves as wide as the land they drain.
- **Trees, small plants and the camp** from one rule for each thing, written once on the graphics chip, read by everything that draws them, so trees, plants, ground and camp agree.

**Your observations** (5 October, on P8's camp):
- The ground's colours must not just appear: green because grass grows there, yellow because it is sand, brown because the earth is bare or trodden.
- Everything crisp, clear and detailed, as in your picture of a pixel-art river glade: every plant, stone, stump and animal reads on its own, with a clean shape, a few colours of its own and a dark edge; nothing a smear.
- All textures are made again from scratch for production: P8's colours, ramps and patterns are stand-ins.
- **Your pictures for inspiration:** a planet game, vibrant, with 3D clouds, biomes that differ and water that moves (not to be copied); a pixel-art river glade, dense layered greens of many plants, clear water over dark depths with stepping stones, ripples and fish, a bare-earth path with cracks, animals everywhere.

**What I take from them, for production:**
- Each place is drawn as what covers it: grass as blades, tufts and flowers, sand as sand, mud, gravel, stones and fallen leaves as themselves; a change of colour is always the cover changing.
- Each cover at every zoom: up close as things, further out as marks in the ground's texture, from far as its colour; one rule decides the cover, and everything that draws the ground reads it.
- A crispness check for every asset at its smallest size on screen: a readable shape, its own colours and its outline.

**P8's camp against the art book:** its grass tufts are about a quarter of the art book's (0.4 m across, up to 0.45 m tall, one every 2 or 3 m²) and sparser; its close camp shows no small plants and its river no reeds, where the art book shows grass as specks of a pixel or two, flowers as dots and reeds 6 to 12 pixels tall; its meadow's patches are too big and flat; dark patches on the trodden floor read as shadows; the hearth's stones read as blue spikes; the band crowds the hearth where the art book spreads people at their work; outlines and smoke were not yet in the zoom, and dusk and night not yet compared.

**Still open:** P8's Measure on your phone was never sent.

## P9 Ecology (α0.6a)

**The question:** does nature hold its numbers for a century with nobody in it (`WLD-18`, `WLD-30`, `WLD-31`, `WLD-32`)?

**What it found** (P7's worlds from 20 seeds at cells of about 4 km, run 30 years into their present state, settled 10, then 100 more, in steps of five game days):
- Every species stayed within 0.66 and 1.10 of its settled total for 100 years, in every biome it lived in; grass, browse and trees within 0.83 and 1.09; 82 to 99 big plant eaters for each hunter, where `WLD-18` asks 50 to 200.
- The world's totals hold because its regions' bad years fall at different times: round each start region, about 64 km across, numbers swung from 0.21 to 2.31 of their settled level, crashing in a drought or hard winter and mending over 10 to 20 years.
- **The rules,** all on the cells' totals: plant cover by the season's warmth and the soil's water, thinned by fire; weather years at three scales (a region of about 64 km, a pattern shared over several hundred km, the whole world); 14 plant eaters at a sixth of Damuth's density for their weight; 4 hunters by Holling's second type, the weak and those floundering in snow taken first, their room by Carbone and Gittleman's rule; breeding once a year by condition, fewer as a cell fills; the young leaving for less crowded cells next door.
- **Its cost:** 8 to 19 s a world for its 141 years on four cores of the cloud.

**Lessons for production:**
- A world made from a rough start drifts for decades, so production makes it directly near its balance (`WLD-08`).
- A game winter must cost an animal what a real one does, though it lasts 20 game days, or the weather moves nothing: at first every species held within 2%.
- Dry country's plants live on less water than a meadow's: grown as a meadow short of rain, they starved the animals there in every normal year; they now grow by the soil's water partly against the place's own.
- Young animals leave every cell for less crowded ones, more from crowded cells, or land emptied by a hard year stays empty.
- In forests with mild winters numbers barely moved: production adds hunters that can starve when their prey thins, and cycles like the hare's and the lynx's.
- Not yet tried: fish, birds and bears; and the 1 km cells with herds that `WLD-32` describes.

## P10 Culture from causes (α0.6b)

**The question:** do customs, shared spirits, rites and band splits come from events alone (`CUL-05`, `CUL-06`, `CUL-30`, `CUL-33`, `CUL-34`, `MND-05`, `RSK-19`)?

**What it found** (three bands of 22 to 30 people in families, run 100 years in each of 20 worlds, in Python):
- **A custom** in all 20, Years 0.1 to 0.6, from a band's first three big kills.
- **A shared spirit** in all 20, by Year 2.3, most often a being in the storm after lightning struck a camp, else the dead living on after grief and dreams.
- **A rite a band keeps** in 15, inside Years 3 to 10; 5 came earlier, from a burial custom named in the first two years.
- **A band split** in 17, inside Years 5 to 25, when a band grew past 40.
- **What a band shares:** a belief most of its adults hold is its spirit; an act most credit for good hunts a year long, or its way with the dead kept a year, is a rite it keeps.
- **Its cost:** 20 runs of 100 years in about a minute and a half on four cores.

**Lessons for production:**
- `MND-05`'s numbers let an act repeated before a common outcome sustain itself once believed (a hit adds 15, a miss takes 5, so hunts that succeed three times in ten keep it): only those an outcome befalls link it, and a band credits it only through talk.
- Talk lends a listener the teller's conviction by trust and no more, however often told; adding it at each telling made every told belief permanent.
- A rite a band keeps needs a test of keeping: credited, or done as the band's way, a year long.
- Realistic rates matter: hunting deaths at ten times the real rate (about 0.4% a year for a hunter) flooded the bands with sudden deaths, spirits of the aurochs and burial cases.
- Customs come within months from the commonest cases, and the way with the dead within a year or two: `CUL-33` now gives customs a year (your OK).
- Measure how often a credited act becomes a rite, beside `CUL-34`'s hunting song.

## P11 The director (α0.6c)

**The question:** can a director that reads the world slow time for its story, within budget, without ever changing it (`TIM-02`, `TIM-03`, `PRE-39`)?

**What it found** (20 test worlds of P10's bands and P4's discovery, 100 years, watched from the globe):
- At 5 game years a real minute, 6.8 hours in all, never tapping: 9.5 slowdowns an hour, never two within 3 minutes, at most 3% of any watch slowed, at least 96% of top speed kept; tapping every live moment, 9.1 an hour and at least 90% kept.
- It caught all 208 named discoveries (60 world firsts, 113 a people's first of what others made, 35 rediscoveries), slowing for 34, and all 60 deaths of those followed, slowing for 16.
- Every world ended identical with it on and off, its log and all it holds; a host that let it reach a world's chance was caught; a code check found no path from it into a world.
- **The recognisers:** Felt's patterns over the log, each kind of event with what makes its kind for a first; state kept from what was seen, such as who can make what. **The signs:** Winnow's half-matched patterns.
- **The rules:** a moment or sign past the bar slows time if the budget allows, for about half a minute; after each slowdown the bar stands higher, by 50 at 3 minutes and falling to nothing by 10; a watch opens rested.
- **Its cost:** 20 worlds of 100 years, each on and off, in about 7 minutes on three cores.

**Lessons for production:**
- **Signs must earn their slowdowns:** a hunch tried again ended in its discovery 0.3% of the time, a storm over a camp in lightning 1.3%, one you follow hurt or ill in their death never. Scored at their end's worth they took 64 of 79 slowdowns, 2 of which came true. Each sign is scored by how often it comes true, measured in the pace tests.
- **Accidents give no warning:** most first flakes come by accident, so they are caught as they happen.
- **The list fills fast:** about 200 moments an hour at the globe, two thirds in a world's first 20 years; production gathers repeats into one line, ranks by score, and may score a kind lower each time it recurs.
- **A watch opens rested:** with the bar at rest at the start, world-first discoveries slowed for rose from 20 to 31 of 60.
- **A first needs its kind spelled out:** a custom by its answer, a spirit by its being, a death by its cause, a thing by what it is.
- **The log is the director's only window:** each event with its day and hour, kind, who and what; an activity logged at its start and its result at its end.
- **One stream of chance can hide a path:** one stray draw by a careless host left no trace in a short world; chance keyed by being and moment has no shared stream, and the repeat check compares logs as they go.
- **Ages and the pace:** fire comes within 8 years of the first flakes, so 20 years between ages never gave an age of fire: an age lasts until the next turning point (`PRE-39`, your OK).
- The fifth of time slowed never binds while each slowdown lasts under 36 s: the 3-minute gap is the working rule.

## P12 The interface (α0.7a)

**The question:** do thumb reach, gestures and crisp pixel text work both ways up on your phone (`PRE-32` to `PRE-35`, `PLT-02`)?

**What it found** (in the cloud; your verdict to come):
- **Drawn in art pixels:** the whole screen into one picture, a whole number of screen pixels to an art pixel (3 on your phone's 1080 × 2404 setting, 4 on the full panel), enlarged with hard edges.
- **The pixel fonts from one source,** the art book's `font.js`: the plain font as a bitmap font drawn only at whole multiples; the handwriting drawn as `font.js` draws it.
- **One gesture reader on raw touches** tells apart a tap, a slow tap and two taps; drags and a flick; a long press and drawing after it; a double tap dragged both ways; a pinch with a slight turn; a twist with a slight spread; the handle tapped and swiped.
- **Every control at least 48 dp and 8 dp apart,** in portrait in the bottom third, on both screens, both ways up, with each panel open.
- **What the rules changed from the plates:** the plates' time controls are about 25 dp, so they spread wider; the live moment and the book's tabs come down within a thumb's reach; the views fit their rows into the bottom third; the time controls wait while a panel is open in portrait.

**Lessons for production:**
- Your phone's own setting sets the pixel: a layout reads the screen it gets, never the plates' 336 × 748.
- Text is small but whole: the plain font's capitals are 7 art pixels, about 1.4 mm on your phone, near the smallest Android suggests.
- Sideways, panels are short: a card in landscape scrolls; production lays landscape panels out for their height.
- Godot measures a control before its font is set: sizes come from our own font's measure.

**Still open:** which ground for the card and the book (Paper, Night, Hide, Slate or Glass, after you doubted the whitish paper), and whether reading text should go up to twice its size.

## P13 The writer (not built)

**The question it would have asked:** does the phone's own model (Gemini Nano through ML Kit's Prompt API) reword the book of ages' pattern sentences, passing the strict check, fast enough and within its battery quota (`PRE-41`, `RSK-08`)?

**Carried into production:** the book of ages works from pattern sentences alone (`PRE-37`); the writer is proved when M9 builds the book, and if it falls short, pattern text stands alone.

## P14 Sound (α0.7c)

**The question:** do 32 sounds at once with distance filters, a cave's echo and the murmur play without breaks on your phone (`SND-01`, `SND-03`, `SND-08`, `SND-11`, `SND-12`)?

**What it found** (in the cloud; your ears and Measure to come):
- **Base sounds made by code:** strike, scrape, chop, step, drum, stream, rain and thunder from shaped noise and ringing modes, 78 of them in about 0.3 s; each `SND-06` rule held (harder brighter, bigger deeper and longer, wetter duller), and 20 flint strikes in a row all differ. Brightness is measured as the RMS of the spectrum's frequency, not by zero crossings, which follow a log's low ring.
- **Live synthesis on the audio thread:** the fire and the wind as audio streams of our own, following heat and speed, about 0.04% of a core each.
- **The murmur from a bank:** 60 syllables for each of two base voices, a woman's and a man's, rendered in the cloud by an open speech engine (Piper; LJ Speech, public domain, and Joe, CC0, as stand-ins), each voice pulse marked by zero-frequency filtering; 1.8 MB for both. The phone strings them into phrases and moves each pulse's grain to a new pitch, formants shifted by squeezing (TD-PSOLA), on worker threads: about 1.5 ms for a 1.5 s phrase. A child (pitch 324 Hz), a woman (227), a man (109) and an old man (98) come apart as asked; anger quicker, louder and higher, grief slower, softer and lower.
- **Space:** each sound a 3D player, inverse distance with its own low-pass; an area with reverb over the rock shelter; the cliff's muffling.
- **The voice manager** holds `SND-01`'s shares (6 place, 4 voices, 8 music, 2 free, the rest work), a hum taking a place in its share when it starts; asked for more than the cap, it plays 32 at most, 31.5 on average.
- **A limiter at the end of the master bus,** and after it a probe timing the audio thread's own CPU for each block: with 32 sounds, 2.7% of each block's time on average and 9% at worst in the cloud.
- **The reel** recorded in the cloud by Godot's Movie Maker, picture and sound, and played on the note's page.

**What went wrong, and the lessons:**
- **Godot's 3D players are silent without a camera:** they reckon their volume for each camera in the world, and an audio listener alone is not enough. The first reel and the first cloud timings were of silent players.
- **godot-cpp's build profile must name `AudioFrame`,** or the audio callbacks that take it are dropped without a word.
- **The engine's alignment puts a vowel's start up to 0.1 s late;** where the syllable first grows to half its loudest is right.
- **Short sounds rarely fill the cap:** the camp's 32 makers, all at once, filled at most 23 of the 32 places; the cap is reached with music, thunder and the hums, so Measure asks for more than the cap on purpose.
- **32 sounds at once add up past full scale:** a limiter belongs at the end of the master bus.
- **Recording a movie, Godot mixes on the main thread** between frames, so the audio thread's timing means nothing there.
- **Files that are not Godot resources,** such as the voice bank, are left out of the export unless its include filter names them.
- The voice you choose by ear for the game must have a licence that allows shipping it.

## Your choices during pre-production

- The look's starting point: the art book, accepted on 4 October.
- Outline way C and the camera's "ease", left to me on 4 October; turn steps of 5°.
- Fire shadows from every fire and smoke as a lit volume (after P2).
- The pace halved twice: the first village about an hour into play.
- An art pixel about 2 × 2 at the person, growing to about 6 × 6 at the globe (`PRE-22`), and the map vivid and textured, lit by the hour (`PRE-29`).
- The world a sphere at every scale, never unrolled (P8's second round).
- Customs within a year (`CUL-33`); ages of any length (`PRE-39`).
- Five grounds for the panels, after you doubted the whitish paper.
- P13 left for later, then pre-production closed without it; the deferred reviews of α0.4 and α0.5 dropped with the prototypes' code; the art book kept.

## Traps for production

- **Godot:**
  - Movie Maker records at the project's window size: an `override.cfg` with the window size override sets it; a single picture at the phone's size is read from the screen by our own script.
  - A rendering driver named on the command line brings Forward+ unless the Mobile renderer is named beside it.
  - Godot 4.7 can abort as it exits after importing new files (its Android plug-in finds no adb daemon in the cloud); a second import exits cleanly.
  - The headless audio driver does mix, so audio can be tested headless; a picture or a 3D sound needs a camera.
  - `call_deferred` from a worker thread and `WorkerThreadPool` tasks each waited for exactly once.
  - The screen's own `_draw` lies behind its children: what must show over a picture is drawn in the same `_draw` or by a child.
  - A label that wraps measures itself at zero width until laid out: panels sized by containers, not by hand.
- **godot-cpp:** a class's methods that take a native structure appear only if the build profile names the structure; `OS` is needed by godot-cpp's own printing; a local class cannot hold a member template.
- **The phone:** the screen runs at 120 Hz unless capped; its chip lowers its clock with time to spare, so its milliseconds are partly idle; thermal headroom through Godot's Android runtime, no plug-in needed.
  - Found afterwards (research 18): Android forecasts heat only while an app has asked within the last 10 s, so the prototypes, asking every 10 s, mostly read the current heat, not a forecast; ask every 2 s.
  - Also from research 18: Godot 4.7.2 sets the project's frame cap before the screen's swapchain exists, so the screen kept voting 120 Hz; the cap must be set again at run time.
- **The cloud:** the software Vulkan driver can crash on some shaders at some angles; `sleep` chains are blocked, so waits are loops with a time limit; heavy jobs run one at a time.
- **C++:** no fast-math, no contraction; our own transcendental functions; clang-tidy's analyzer misreads doctest's own strings as leaked in some tests.
- **Python:** a model's whole state hashed for its repeat check; one stream of chance can hide a stray draw.

## How we worked

- **Phone first:** every answer that matters is measured on your phone, from a line copied into the chat, with the heat forecast beside it.
- **Checks that cost what changed:** ccache for C++, lint and tests rerun only for what changed; about 20 s with nothing changed.
- **One question a prototype,** stopped for your verdict after each; the note with pictures, and for sound a reel, on its page.
- **What went less well:** Measure runs cut short left questions open (P2's heat); timings once counted only part of the frame (P2, P3) or silent sounds (P14); verdicts on the phone lag behind the cloud's, so several are still open.

## Still open, carried into production

1. P12: which ground for the card and the book, and whether reading text should be larger.
2. P14: your ears on the camp and the voices, and Measure's line from your phone.
3. P8: Measure's line from your phone.
4. P2: the heat of a busy scene over ten minutes.
5. P13: the writer, proved when M9 builds the book.
6. `RSK-01`: discovery's pace with the whole catalogue, at M7's scenes.
