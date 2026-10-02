# AR5: review of P7 (A11 Drawing, A12 Screens, views and text)

### 1. [major] A11.4 Light over time: hour-long dithered blends at real speed, full-screen strobing at speed
Problem: The palette row changes by a per-pixel Bayer crossfade over each 4° border and across the whole −6° to 6° twilight row ("Palette row: blend"). At the person stop the whole picture is a 4 × 4 mix of two palettes for about an hour each dawn and dusk (12° of sun at the first region's 46° takes about 70 game minutes). PRE-20's Done when forbids speckle outside narrow bands, and the owner already rejected B66's Fade for an 80 ms dither of the same kind.
At speed the opposite fails. The sun is taken at display time "at every speed", so the whole screen goes from day to night every 4 s at the valley (6 game hours a second), 3 times a second at the region (72 h), and 1.4 to 10 times a second at the world map and globe (A16.3's and `TIM-07`'s top speeds). Each lightning strike also gives a full frame of the `lighter` table, a few a second at camp speed under a storm. Full-screen flashing at 3 Hz or more breaks `VIS-14` and is a known seizure trigger.
Fix: (a) Precompute in-between palette rows, 8 per transition, for each season (about 128 rows of 256 colours, 128 KB), and switch whole frames between them; rows are never dithered.
(b) Low-pass the shown light by speed. While a game day lasts at least 30 real seconds, light follows `T_d`. Faster, it eases over 1 s to a steady light for that place and season: the day row dimmed by the share of night, and the sun fixed at the season's noon height for shadows. Seasons do the same when one passes in under 10 real seconds. The screen's overall light never changes more than once a real second.
(c) Lightning flashes only at speeds up to 1 game hour a real second, at most one flash a real second. Faster, strikes are short marks with no flash.
Part: P7

### 2. [major] A11.5, A11.4 Calls into A5 that A5 does not provide
Problem: A11.5 builds view areas with `make_area(seed, AreaId, &CellInputs)`, `apply_marks`, `coarse_area` (9 × 9 heights, under 50 µs) and a continuous signed `density(x, y, z)`. A5.3 provides something else: `skeleton`, `contents`, `ground`, and `whole(cx, sk)` with the kept record inside `AreaCtx`, plus `height_at`, `material_at` and `solid_at`. Its 3D pieces are stored as per-column air gaps, so there is no signed density, and there is no coarse function.
The map mirror holds 16 bytes a cell, but `whole()` reads the fixed layers and changing state of the area's cell and its 4 × 4 neighbours (A5.2: 32 + 32 bytes a cell). So A5.5's test, "a view area hashes equal to the world's", cannot pass.
A11.4 calls `kd_core::geo::sun_moon(t, lat_deg)`, which does not exist. A5.8 caches sun height and daylight per row and day, with no azimuth and no longitude. Yet A11.5's globe draws a night side (`planetFS`), which implies local time by longitude.
Fix: P7 uses A5.3's names. The view builder calls `skeleton` and `whole`, and the record in `AreaCtx` replaces `apply_marks`. 3D blocks are meshed by surface nets on `solid_at`: z crossings exact from the decimetre gaps, x and y crossings at midpoints, then one relaxation pass.
kd-app shares the fixed layers read-only (an `Arc` made at generation). The mirror holds exactly the changing fields `whole()` reads, listed by A5 (about 20–32 bytes a cell).
A5 adds `coarse(cx, a) -> CoarseArea` (9 × 9 heights 32 m apart, equal to `whole()` at those points, a cover class, ≤ 50 µs). It also adds one pure `kd_core::geo::sun_moon(t, lat, lon) -> SkyNow { sun_dir, moon_dir, moon_phase }`, used by A5.8's daylight and by the renderer, after stating whether local time follows longitude. If it does not, the globe has no terminator.
Part: P7, P3

### 3. [major] A11.5 View builder: no fallback at MIL-01, and its costs and threads are too optimistic
Problem: Until a view area is ready, "coarse ground shows" (A5.5: "that land is drawn from its cells"). But coarse ground and cells are first needed at `MIL-04`, so from `MIL-01` to `MIL-03` land that is not ready is void.
The builder is one thread on the small cores, costed at "about 40 ms an area (B11)". A5.3 gives ≤ 40 ms for a plain area and ≤ 400 ms for one by a cliff, on a held middle core, and a small core does about 0.75 of a middle one (A1.4). So an area by the cliff takes about 0.53 s, and the 12–16 camp-stop areas around `MIL-01`'s cliff camp take 2–4 s on one thread. That is before meshing, which has no cost at all: height-field chunks, normals, surface nets at 1 m and 0.5 m, strata.
On the web, the 2 ms building slice is missing from A2.6's frame budget.
Fix: Build by A5.3's 16 m buckets instead of whole areas: the skeleton first, then each bucket's ground, contents and mesh, nearest the view centre first. The centre's detail then lands within about 0.1 s (`PLT-04`), and a camp view within about 1 s.
Run two builder threads, one per small core, below the audio thread's priority. State the cost of each step, meshing included, and measure it in the `MIL-01` phone benchmark with a pan along the cliff at camp zoom.
Move coarse ground (from `coarse()`, finding 2) to `MIL-01` as the placeholder. A2.6's simulation budget subtracts the building slice.
Part: P7

### 4. [major] A11.8, A11.11 More than 48 full figures has no design
Problem: Full figures are "up to 48 in view, a draw call each". Each person has a unique mesh and uploads its own 12 bone matrices, and the shadow pass draws every figure again.
`PLT-04`'s benchmark puts a village of about 300 at close camp. A gathering, or an ordinary day, puts 75–300 people in that 35 × 130 m view. That is 150–600 figure draws a frame, and draws go through ANGLE here (A1.4), so they exceed the 1.0 ms allowed for "culling and draws".
Fix: Draw full figures the way tiny ones are drawn. Each person's mesh lives in a shared vertex buffer (32-bit indices) with a per-vertex figure slot. Bone matrices and the four ladders come from one RGBA32F texture a frame (12 bones × 4 texels × up to 512 figures, 384 KB). The visible figures' index ranges are joined, so all full figures draw in 1–2 calls a pass, however many there are.
Budget figures by faces (for example 400,000 a frame). Use the 0.073 m mesh at close camp, and the refined mesh only at the person stop.
Part: P7

### 5. [major] A11.5, A11.11 The camp stop was never measured and has no geometry budget
Problem: B66's phases sat at about 0.03, 0.064, 0.5 and 21 m art pixels, plus the planet, all over the mockup's 172 × 112 m scene. P7's camp stop (1.1 m art pixels, about 300 × 810 m in portrait at 55°) is not among them. It draws full areas with every tree as an instanced level-1 mesh (trees only join the canopy at 1.25 m), and shadows stay on below 3.2 m.
A5.3 allows up to 5,000 single plants an area, and the first region has 0.55 tree cover, so 8,000–18,000 trees and bushes can be in view. At the mockup's level-1 meshes (pine 74, birch 370 triangles) that is 1.5–3 million triangles a pass, twice over with shadows: several times the mockup's heaviest view. The GPU split (scene 2.2 ms, shadow 0.8 ms) was never measured; B66 had no GPU timer.
The 250 draw calls count one pass only. The shadow pass repeats ground, things, trees and figures, and the valley's 3,360 coarse areas would take hundreds of ground draws.
Fix: Add per-pass budgets, tuned at `MIL-01`: the scene pass at most 1.0 million triangles and the shadow pass at most 0.5 million; draw calls at most 250 in the scene pass plus 150 in the shadow pass.
Give trees a third detail level by on-screen height: under about 12 art pixels, 20–30 triangles or the map look's crowns. At the camp stop, trees cast shadows only within 300 m of the view centre. Batch coarse ground into 4 km tiles.
Add a dense-forest camp stop, turning, to the `MIL-01` phone benchmark.
Part: P7

### 6. [major] A11.8 The pose clock cannot put the strike on the flake
Problem: The pose clock is unpaused real time, offset by `hash64(uid)`. So at real speed a knapper's contact pose falls at a random moment relative to the activity's end, which is when the flake appears and A13 plays the contact mark (results apply at the activity's end, A4.4).
A4.13 promises that "a flake comes off exactly as the strike ends", and `TIM-10` that each activity "plays over its real length with its animation". The person stop is `PRE-03`'s signature view.
Fix: While display speed is at most 4 times real, a movement's phase is display time since its activity started. Its loop is stretched so a whole number of loops fills the planned length, putting the last contact or stroke mark on the end. Faster, the phase comes from real time with the uid offset, as now. On a switch, carry the phase over and ease the rate over 0.5 s.
One function in kd-view, `loop_phase(uid, act, start, end, shared, t_d, real_s, speed)`, serves as A13.2's `AnimClock`.
Test: at real speed, the contact pose is on screen within one pose step (100 ms) of the flake appearing.
Part: P7

### 7. [major] A11.9, A11.8, A12.4 Herds kept as counts are never drawn or tapped
Problem: A4.13 hands herds over "as counts and kinds", and A7.7's leg gives a herd's place at any moment. But A11 draws only `FigureView` beings, and `Pick` has no herd. A herd more than 1 km from people is invisible at camp and close camp, A12.4's herd card cannot be opened, and the mockup's deer and wolf group marks are not ported.
Fix: Add `HerdView { uid, species, counts: [u16; 5], leg, wariness }`. The renderer places each animal at the seeded offset A7.8 uses when it comes out (keyed on the herd's uid and the animal's ordinal), around the leg's position, with a slow seeded wander. Each animal is posed by its pattern's graze, walk or rest loop and drawn on the tiny-figure path, one mesh per species.
`Pick::Herd(uid)` opens the herd card. At the valley stop, herds become the mockup's 5 × 5 marks.
Test: a herd coming within 1 km of a person shows no jump as its animals become individuals.
Part: P7

### 8. [major] A11.5, A11.7 Paintings and carvings on surfaces are not designed (`PRE-15`)
Problem: `PRE-15` and `CUL-09` put pictures "on walls, rocks, hides and the flat sides of things", and `MOM-07`'s Done when needs the painting on the cave wall at `MIL-05`. A11 has no way to draw an artwork on a rock face, the ground or an object; the only mention is one line in A12.4's art card, and the face shader's list has no paint.
Fix: An artwork on a surface is A9.12's record plus the anchor, normal and size from where it was made. It is drawn once on the CPU from its motifs, like an icon, into a decal of palette indices: at most 64 × 64 texels at about 2 cm, kept in a 1024² atlas (256 decals, 4 MB).
The face, ground and object shaders test each fragment against up to 16 decal boxes in view (a uniform array). Where the decal is painted they take its index; where it is carved, one ladder step darker.
Test: `MOM-07`'s scene shows the painting on the wall at person and close camp zoom, matching its card.
Part: P7

### 9. [major] A11.7 The layout format for made things is undefined
Problem: About 100 layouts must be catalogue entries. Each part binds to an input role, takes its shape from that input's form ("hides drape as sheets, reed bundles lie as thatch, poles stay poles") and its count from the amount. Yet A11.7 names no part primitives, parameters or fill rules.
The mockup's `hideFrame`, `rack`, `woodpile`, `windbreak` and `hearth` are hand-written voxel loops with their materials built in. Without a format, each builder writes code for each layout, and `PRE-42`'s test that two materials differ by 30% has nothing to vary.
Fix: Define the schema in A11.7. A part is `{ role, slot 0–3, primitive, params, count, decorated }`.
- Primitives: `line`, `ring`, `sheet_between` (a skin between two lines, with sag), `cone`, `dome`, `box`, `heap`, `hang`.
- Params are in voxels, scaled by the size band, and `count = f(amount band)`.
- The fill follows the input's form: a pole or rod is a solid line; a sheet a one-voxel skin; a strand thatch rows on surfaces and wraps on lines; a lump stacked ellipsoids; flakes and powder points; paste a smear.
- A `custom = "name"` part calls a registered Rust builder, only for the few parts no primitive fits.
The catalogue check builds every layout for every form its roles allow.
Part: P7

### 10. [major] A12.7 The word check is too loose and the name check too tight
Problem: Check 5 ports B73's single synonym list for every kind of event. It groups give with offer, hand, trade and exchange; see with watch and witness; help with tend. So "Ama gave Tor flint" can become "Ama traded flint with Tor" and pass every check. That is a changed fact `PRE-17` forbids, and name order cannot catch it (`PRE-41`: "the check alone can't see who did what"; B73's checker missed a giving swap).
Check 2 demands exactly the pattern's names in each sentence, so the writer cannot use a pronoun, and its prose stays as choppy as the patterns. B73's models already copied 65–71% of their four-word runs from the data (`RSK-08`).
Fix: List synonyms per event kind in `data/text/`: each kind names the words its sentences may swap. Drop the single shared list. A catalogue check fails any set that holds a converse or broader verb from a fixed list (give, trade or receive; teach or learn; kill or die; take or receive; lead or follow).
Check 2 allows a pronoun in place of a name only when that person was the doer in the previous sentence, the pronoun matches their sex, and nobody else of that sex is named in either sentence.
The trap set adds give-to-trade, teach-to-learn and pronoun traps.
Part: P7

### 11. [minor] A12.7 Twelve-sentence requests, and texts that change after they are shown
Problem: A text may hold 12 facts and about 150 words, but every request keeps B73's cap of 256 new tokens. B73 never went past 7 facts or 129 words. With numbering and invented names, 12 sentences can pass 256 tokens, so the last sentences fall back to patterns every time.
A history text shows its pattern and "gives way if a checked text arrives within 5 s". But a result that arrives later, or a stored text failing a re-check after an update (A14.8), changes what you read. That breaks `PRE-41`'s "opened twice reads the same" and "never silently rewritten".
Fix: Send at most 6 sentences a request (a 12-fact text in two), with max tokens 1.6 times the patterns' tokens. Put two 12-fact records in the trap set.
Store the text the view settles on: the writer's if it is checked within 5 s of opening, else the pattern, marked as such. Drop a later result unless you ask for a rewrite.
Re-check a stored text only against its record's hash, with the checker version it was stored with.
Part: P7

### 12. [minor] A11.10 The crawl counter and the four fixes are deleted with `pretests/`
Problem: A11.10 has `tools/screens/crawl.mjs` count crawl every alpha, and the Tests screen offer all five fixes. But the counting runs inside `drawing-test.html` (`window.__b66.crawl`, `crawlPair`). So do Steps' quantiser, Fade's 80 ms crossfade, and the 2 × 2 resolve shader of Majority and Sticky.
A2.9 recovers only `crawl.mjs`, and `window.kd` (A12.4) has no crawl hook.
Fix: A2.9 recovers those parts of `drawing-test.html` to `crates/kd-render/src/crawl/reference/` and `tools/screens/`. `window.kd` gains `crawl({ motion, rate, frames, fix })`, which freezes animation, captures each frame's indices and depth, and returns B66's counts.
Part: P7

### 13. [minor] A11.5, A12.2 Zoom: speed set by width, the globe, rivers far out
Problem: A12.2 sends the view's width to A4.11. But the globe's view (2.9 km × 272 pixels, about 790 km) is narrower than the world map's (7.6 km × 272, about 2,070 km), so a speed set by width falls from map to globe.
A11.5 draws no beings at the globe, though `PRE-28`'s Done when needs a camp readable at every stop, and the mockup keeps the camp glow on the planet.
The map texture's coarser levels keep the "dominant class", which drops rivers one cell wide, against `PRE-29`'s "coasts and rivers stay visible".
Fix: `SimControl` carries the zoom (0–1), and A4.11 sets speed by it. Camp points stay on the globe. The coarser map levels keep cover by dominant class but water flags by any (OR), and rivers are drawn as lines at least one art pixel wide at the region, map and globe stops.
Part: P7

### 14. [minor] A12.1 Layout against `PRE-34`, and long text
Problem: Portrait controls "in the bottom 45%" contradict `PRE-34`'s Done when (the bottom third). A12.2's "24-pixel bar" mixes screen and UI pixels.
The text size is fine: a 7-pixel cap is 1.8 mm at 0.26 mm a UI pixel, the same as 16 sp body text, with 48 characters a line. Drawing text in the engine is also right, since native text would break `PRE-01`, the web build and screenshots.
But an 11-pixel line leaves 2 pixels between lines for 150-word entries and long mind pages. Landscape's 200-pixel panel gives about 36 characters a line, there is no larger size, and Monogram is published as a monospaced font, not a proportional one.
Fix: Controls in the bottom third (201 of 603 UI pixels), and every size in A12 given in UI pixels. Running text on a 13-pixel line, lists on 11. A "larger text" setting with a 9-pixel-cap font on a 15-pixel line.
The book and details views take 300 of 601 pixels in landscape. Font candidates must be proportional (Pixel Operator, or glyphs drawn in-house). The `MIL-02` contact sheet shows a 150-word entry and a long card, for the owner's verdict.
Part: P7

### 15. [minor] A11.11, A15.10 The phone benchmark does not cover what `PRE-02` and `PLT-04` test
Problem: `PRE-02`'s Done when needs every zoom stop smooth while the camera turns, and `PLT-04` adds "none more than 50 ms late". A15.10 measures only the camp and valley stops, for 30 s each, and A11.11 adds close camp and globe. Neither measures the person, region and map stops, a turn, `PRE-03`'s pinch, or the worst frame.
Fix: The phone benchmark runs each of the seven stops for 10 s while turning. It adds a pinch from globe to person over unvisited land (no frame over 50 ms, detail within 1 s), the village of 300 at close camp, and a dense forest at the camp stop. It records the worst frame and the GPU time per pass, reported as unknown where timer queries are missing.
Part: P9

### 16. [minor] A11.7, A11.11, A16.4 Drawing memory does not add up
Problem: The 384 MB drawing line must hold:
- the mesh cache (128 MB);
- person meshes (40);
- view areas (40 by A5.5, 19 by A11.5);
- the mirror (32);
- the map texture (8);
- a 2048² shadow map stored as RGBA8 packed depth plus a depth buffer (32);
- art targets and atlases (about 12);
- ground and 3D meshes, which have no budget: a 1 m area mesh is about 1.3 MB, so 96 areas would take 126 MB.
Fix: Budget ground and 3D meshes (for example ≤ 64 MB, built at the detail level in view and dropped beyond 1.5 km). Draw shadows into a depth-only texture, which GLES 3.0 and WebGL2 sample directly (the RGBA8 packing was for WebGL1): 16 MB, and less memory traffic. Reconcile the area cache with A5.5, and list the sub-budgets in A11.11.
Part: P7

### 17. [minor] A6.4, A11.7, A11.9 Made things don't record what their parts need
Problem: A11.7 needs, for each layout slot, the input's item, material, form and amount (A11.9: `slots: [SlotUse; 4]`). But A6.4's Made row keeps only "part materials", so "more poles, a bigger hut" and shape by form cannot be drawn.
Fix: A6's Made row keeps, for each drawn part (at most 4), the input's item kind, material and an amount band (u8). The catalogue check limits a made item to 4 drawn parts.
Part: P4

### 18. [minor] A11.5 Soot has no record (`PRE-23`)
Problem: The face shader draws "soot above hearth marks", but A5.4's list of marks has no hearth or soot mark. `PRE-23`'s Done when needs soot that builds up over years of use ("a cave lived in for 10 years").
Fix: A5.4 adds a soot mark on the rock face above any fire under an overhang or in a cave. When a fire ends, its hours add to the mark's strength, and the mark fades over about 500 game years. The face shader darkens by it.
Part: P3

### 19. [minor] A12.4 Cards miss lines `PRE-35` lists
Problem: `PRE-35`'s Done when needs every line it lists. The person card lacks their ambition and how close they are (`MND-32`), and the body words and each part's health (`BIO-08`, `BIO-09`). The band or people card lacks customs (`CUL-06`).
Fix: Add them to A12.4's table, each with the record it reads.
Part: P7

### 20. [minor] A11.1, A11.8, A11.11 Names and numbers that disagree across parts
Problem:
- `upload_area` allows 1 ms a frame, but A11.11 gives uploads 0.2 ms.
- `Renderer::new` and `draw` differ from A2.2's sketch, and A2.2's `Request` has no `RenderMode`.
- A4.12's `SimControl` has no list of the area versions the renderer holds, which A11.9 relies on.
- A13.2's planner takes an `AnimClock` that A11.8 never defines.
- A2.2 starts `kd-player` at `MIL-03`, but its recognisers and moments are needed at `MIL-02`.
Fix:
- Uploads take at most 0.2 ms a frame, with the rest spread over later frames.
- A2.2 takes A11.1's signatures and adds `Request::RenderMode(OnDemand | Continuous)`.
- `SimControl` gains `held: SmallVec<[(AreaId, u32); 96]>`.
- `AnimClock` is finding 6's `loop_phase`.
- `kd-player` is first needed at `MIL-02`.
Part: P7, P1, P2
