# Kindling: architecture

The contracts, current implementation limits and facts needed to build the next running game.
PROJECT.md owns requirements; IMPLEMENTATION.md owns sequence and acceptance. Aim for at most 6,000 words here.
Existing A section numbers remain stable because code cites them. Deferred sections preserve obligations, not permission to start work.

## Status (8 October 2026)

M1 is accepted. The owner approved the revised route on 8 October: M2 living camp; M3 discovery slice; M4 seasons and lives; M5 neighbours; M6 small finished valley; M7 wider world; M8 remaining life, crafts and society; M9 presentation, powers and sound; M10 optional copper/full arc.
Earlier prototypes are evidence only; their deleted code is not a production source.
The 2D renderer is partly built. Delivery 30801 shipped; later α2.9a work is merged WIP, without whole-step acceptance.
Camp alpha adds 25 saved idle adults and scoped supplies in a bounded seeded patch. Needs, minds and discoveries are not implemented yet; simulation markers remain diagnostics.

## A1. Overview

### A1.1 What it must deliver

An offline, saved, deterministic simulation of independent people, read through a fixed-camera 2D view.
M2 starts with about 25 adults in a labelled, bounded seeded valley; full-world and population requirements remain open.

### A1.2 The big picture

`sim/` owns authoritative state and never reads Godot, camera, zoom or requested speed.
`view/` sends commands and converts owned snapshots into drawing records.
`game/` presents those records, gestures, UI and sound. `data/` supplies validated catalogues and tuning.
Rendering, cache contents and view-only chance cannot alter history (`WLD-13`).

### A1.3 Decisions

Keep Godot 4.7.2, stock templates, the C++20 simulation/GDExtension, Compatibility renderer, fixed north-facing 37° camera and existing save approach through M3.
Change a local implementation only for a measured blocker. A later Mobile comparison uses identical scenes; it is not another renderer restart.

## A2. Code layout, builds and delivery

### A2.1 Repository layout

`sim/` is engine-independent rules, tool and tests; `view/` is the native bridge; `game/` is the app and app tests.
`data/` holds TOML sources, scenes and benchmarks; `art/` holds targets, sources, approvals and derived assets; `tools/` builds/checks; `dist/` holds the delivered APK/note.
Generated `game/bin/` and `game/data/` are not committed.

### A2.2 Builds

CMake/Ninja/ccache build the host and Android arm64 extension. godot-cpp stays pinned at 4.5 until an actually needed API requires an upgrade; argument structures must be in its reduced binding profile or methods disappear.
Errors are values, never exceptions. Preserve `-funsigned-char`, `-fno-fast-math`, `-fno-math-errno` and final `-ffp-contract=off` on our C++.
Android uses API 24, statically linked C++ runtime, guarded newer APIs and 16 KiB alignment.

Export unsigned, then compress, align and sign through the existing tools; only `tools/signing-key.py` reads the signing secret.
Text catalogues/reports need explicit export inclusion; tests/addons stay excluded; no permissions are requested.
Lossless native-library deflation requires `extractNativeLibs=true`. Runtime PNG levels may use exact palettes or omit opaque alpha only after verifying every decoded RGBA byte; profiled sources retain their encoding. The shared meadow sheet is packaged once; deleting required catalogue records is not a size fix.

### A2.3 Delivery of each alpha (`PRC-11`, `PLT-06`)

Package `dev.kindling.app`, same release key, increasing version code:
`(milestone + 1) × 10000 + alpha × 100 + step`, with a = 1.
Never reuse a distributed code. The committed APK must fit the repository's 50 MiB file limit; a different distribution route or visible compression needs the owner's decision.
The measured size, note and checksum in `dist/` identify the actual delivery.
α2.13a opens the selected Camp alpha directly. A brief once-per-build smoke check compares one/four threads, numerical environment and catalogue fingerprints, and stores device/saved-moment diagnostics; the full report is at Menu → Device check.
The note has What is new, What to try and What is rough plus the APK link; proved behaviour and remaining checks fit those sections. The installable build follows IMPLEMENTATION.md. First-start checks cover deterministic digests, numerical environment, catalogue fingerprints, saved moment and device diagnostics.

### A2.4 A fresh cloud session

Run `tools/setup.sh` if dependencies are missing; source `tools/env.sh` in every independent build/check command.
It supplies pinned tools, engine/templates and shared caches. Preserve cache reuse; avoid duplicate heavyweight builds and concurrent captures/exports.

## A3. The simulation core

### A3.1 Its boundary

The library makes/opens worlds, advances to a game-time goal, accepts commands, publishes snapshots/events and saves/exports/imports.
It receives file bytes and a save folder; it never calls Godot. The headless tool uses the same rules as play.

### A3.2 Entities and components

EnTT registries sit behind the existing entity layer. Never-reused 64-bit IDs contain a family in the top four bits; registry handles are transient and never saved.
References, history and commands use persistent IDs. Ended entities leave historical records.
Component descriptors own stable names, versions, fields, units, ranges, links and effects; loader, save reader/writer, digest and inspection share them.
Catalogue entries assemble components, without a class hierarchy for kinds.

Camp alpha uses Person (name index, age at scene start, stand-in appearance), Place, Home, Activity and Schedule on person-family IDs. Camp retains rectangular traversable bounds, supply amounts in ml/mg and actual source/shelter positions. All 25 adults choose and move through the saved Life state; no renderer decides actions.
The existing registry's positional foundation fields stay unchanged. Descriptor-written Camp/Person extensions share its EnTT storage and canonical ID iteration, in CAMP4 and its required LIFE2 chunk. Older formats are refused (A3.7). Worlds with person IDs require complete valid records; the reader refuses broken links, bounds, quantities and unsupported activities. Worlds without camp records keep their foundation registry bytes and digests; the outer save format is version 4.

Decision order is event key or persistent ID, with explicit ties; EnTT pool order is not canonical.
Pools are created in name order. Signals maintain indexes, never game rules. The order fuzzer must leave digests unchanged.

### A3.3 Time and events

The clock is 64-bit whole game seconds. Events order by `(second, owner ID, owner sequence)`.
Handlers schedule only later keys; cross-owner effects take at least one second. Cancellation checks live owner sequence slots; saves contain live events only.
The frontier excludes its own second: all earlier events are complete, none at or after it.
Activities retain start/end, progress and authoritative position; interruptions apply their kind's partial-work rule exactly once.
The generic Activity machinery does not yet implement interrupted meals or bodily effects.

One-worker key order is the reference. Parallel islands may neither read nor write another island's owners; owned events/history merge by key.
World layers run between windows; entity creation follows canonical allocation order.
Existing proofs cover varied windows, stops, pool orders and one/four workers.
Crowd uses one worker: islands cost more for its cheap events. Profile actual minds before enabling parallelism; long windows currently make island joining particularly expensive.

### A3.4 The same bits everywhere (`RES-05`, `TIM-16`)

Working arithmetic is double with pinned correctly rounded functions behind `kd::num`; persisted state uses whole units, never floats/NaNs.
Positions are centimetres on the exact 200,000,000 × 100,000,000 torus; differences wrap in 64-bit arithmetic. Half-circumference ties produce reversible paths.
Heights are millimetres, angles 2^32 turn steps, amounts whole base units, probabilities exact thresholds. Checked conversions specify rounding and reject nonfinite/overflow values.
Each simulation thread resets/asserts its numerical environment.

Keep existing bans/checks for platform maths, unordered decision order, mutable random streams, raw-memory serialization/hashing, ambiguous ties and effectful expression order.
Canonical digests walk systems, entities and fields in fixed order, little-endian.
Accepted M1 proof digests remain unchanged across compilers, architectures, worker counts and reopenings; new physical systems get new proof suites.

### A3.5 Chance

Key every draw by world seed, system, entity, moment, purpose and index through the existing SplitMix64-based API.
Stable name hashes distinguish systems/purposes. New draw purposes cannot shift another sequence. Pinned draws are world-format commitments.

### A3.6 Catalogues and tuning (`MAT-13`, `MAT-14`, `MAT-17`)

TOML entries are complete, without inheritance; source/name is stable identity, sorted load numbers are only array indexes.
Quantities are exact unit strings, not floats. Base units include mg, mm, mm², ml, mm/s, milli-degrees, seconds and parts per million; `m` means metre.
Durations retain real-life and game lengths under `TIM-18`; the existing validator interprets “about” as ±10%, not an implementation-specific guess.
Schemas drive loading, links, fingerprinting and display. Unknown fields, bad ranges/units, duplicates and unresolved links fail with source location.

Rules, world-making and look fingerprints are separate. Placement/size/species changes are world-making changes; cosmetic pages are look changes.
Saves keep catalogue names and versions. New sources currently add entries only.
Build manifests hash each exported file; the phone verifies them. Keep one authoritative catalogue, not competing editor metadata.

The α3.13a cleanup removed the old developer menu, marker/time/rendering pages, catalogue and art/terrain inspectors, composed examples and their unused capture tools. Approved art remains in the repository; unused encoded textures and sheets are neither prepared nor packaged. Renderer drawing/stream regressions use excluded `game/test/support/` harnesses; their native bindings build only for host tests. The camp performance test and first-launch self-check remain. `PLT-05`/`RES-06` keep current cloud reports and a plain test-world view; deterministic proofs, cloud scenes, catalogue/provenance and current-save regressions remain required. This is a camp slice, not full renderer/performance acceptance.

### A3.7 Saves (`TIM-05`, `TIM-08`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`)

A world folder holds metadata, two recent snapshots, command journal and yearly history.
Commands append and sync before acting. One I/O thread owns writes; authoritative state copies between events, then compresses off-thread.
Snapshots use versioned chunks, canonical fields, zstd and whole-file checksums validated before unpacking. Logs stop at damaged framing/checksums.
Writes use temporary file, sync, rename and directory sync. Any write failure stops further writes and the world; no snapshot may hide an unsaved gap.

Save every 30 real seconds and on backgrounding, switching, export and quit. Backgrounding writes a pause mark then a snapshot within Android's 10-second allowance.
Recovery uses the newest valid snapshot, sets damaged ones aside and deterministically replays journal/history to the recorded moment.
**Older saves are not converted** (owner, 9 October 2026; `PLT-09` paused during development). A build opens only saves in its own format: it checks the format version before reading anything else and refuses older saves with a plain message, never a half-read world. Write no new upgrades, migration seals, old-save fixtures or corpus tests; delete an old-save test when a format change breaks it. The cleanup removed conversion APIs, migration seals, alternate migration snapshots and old-save fixtures. Current component/chunk versions are strict; VERS1 retains a reserved zero field, with no migration names or conversion path. History thinning never removes records marked permanent.
The current snapshot and archive envelopes are version 4; a folder's recovery metadata begins with `format = 4`. The format gate runs before decompression, metadata parsing, log replay or recovery writes. CAMP4 appends a feature mask. Its craft bit requires CRFT1 (items/work), KNOW1 (personal evidence) and HIST1 (result history), plus LIFE2 and DRMS1. Extension readers reject unsupported versions, duplicate/missing chunks, orphan links, invalid bounds and overlapping reservations. Discovery camps now use these extensions for autonomous work and immutable display records; the legacy living-camp proofs retain their unchanged rules.

Warn below the configured free-space line (currently 1 GB); delete nothing without the user.

Archives contain all authoritative files, validate paths/checksums and remove refused partial imports.
**Limit:** archive output yields in chunks, but `ArchiveWriter::next` first reads/hashes a whole constituent file synchronously. A tiny demo's export timing does not prove smooth large exports.
Full-world snapshots will also need measured immutable-state copying/compression overlap; current code does not stream arbitrary huge terrain state for free.

### A3.8 Talking to Godot

Commands are durable plain records. Snapshots use a single-consumer triple buffer; one controller copies the latest publication and all views/jobs use its owned data.
Never retain a recycled slot. No simulation worker touches Godot objects. Camp identities and supplies are copied into the same owned display snapshot; cards and sprites read that copy. Camp labels/cards use native integer bitmap text; the dock keeps 48 dp controls in either orientation. World panes retain the full window’s 1×/2× raster scale; a 320 ms camera settle keeps existing powers and anchored picking. Camp handles each finger once, ignoring its emulated mouse events; a 550 ms hold opens the ring, and dragging, pinching, cancellation or a changed pane clears that hold. Viewport tests include safe-area offsets and paired finger/mouse events.
Sample the saved activity way at displayed time, including interruptions and torus seams, rather than interpolating arbitrary recent positions.
Lossless events/commands are separate from disposable picture updates.

**M2 blocker before the first power:** the current command frontier can be a quarter real-second of requested speed ahead of the display—hours in game time.
Record displayed ID/time, pause/cap/drain safely and journal the defined execution second. A place dream must not silently target a later world.

### A3.9 Threads, speed and budgets

The runner rereads goals between batches. Screen time advances by steady-clock time but never beyond the frontier; overload slows time without dropping detail.
The pace controller normally targets up to a quarter real-second ahead. Pausing currently lets the display catch the frontier; power selection needs A3.8's correction.
Workers have explicit 8 MiB stacks. M1's phone benchmark favoured unpinned workers (4.3 versus 4.1 game days/second); keep them unpinned.

The heat guard uses the light threshold minus 0.05 where available, with a 10-second forecast; missing readings never mean cool. Camp play and the isolated camp measurement feed the same governor at the catalogue reading interval; measurement reports reuse those readings.
Telemetry records frame distribution, CPU/GPU timings, memory, clocks, thermal status and battery estimates. Whole-viewport GPU timings require disabled-pass comparisons; they are not per-pass timers.
Power-rail callbacks remain unimplemented. Cloud software-renderer times are not phone performance.

## A4. Drawing

Accepted foundation diagnostics and shared texture decoding remain; obsolete private 3D art routes are removed.
The stopped 2D work includes projection, terrain/light fixtures, streaming and UI, but has open faults listed in A18.2.

### A4.1 The picture (`PRE-01`, `PLT-02`)

At reference portrait size draw a 540 × 1200 world viewport, nearest-scaled 2× to 1080 × 2400; reverse dimensions in landscape. UI remains native-resolution.
Other sizes reveal/crop whole world pixels and honour safe insets, never fractionally stretch at rest.
Only live pinch resamples; settle to a resting density with aligned anchors. Continuous camera movement ends in whole-pixel raster offsets, also undone by picking.

### A4.2 Pixels and animation (`PRE-22`, `PRE-44`)

One projected grid serves terrain, objects and effects. Project shared edges before rounding; never accumulate rounded tile pitches or squash already-projected sprites again.
Foot/attachment pivots survive frames and families. Walk animation follows sampled distance; work follows Activity phase; cosmetic idle uses separate stable look keys.

### A4.3 Light (`PRE-20`, `PRE-21`, `PRE-30`)

Aligned neutral albedo, categorical material and mild normal pages feed:
`albedo × (sky × sky_visibility + sun × normal_response × sun_visibility) + fire + emission`.
Contact affects indirect light only; overlapping shadows do not multiply into black. Normal basis must be explicit across east/south/up art and simulation coordinates.
Compatibility currently needs explicit sRGB decode/encode around lighting; `source_color` alone did not fix it.
Hour/weather fixtures are labelled until real sky exists. Fast-time steady relief light is presentation only.

### A4.4 Shadows, overlap and reveal (`PRE-21`, `PRE-24`, `PRE-28`, `PRE-30`)

Receiver-aware masks use actual surface heights and separate roofs/floors. Include off-screen casters and angle-dependent reach; 14 m is not a universal shadow halo.
Current proxies use cylinder/cone/dome intersections, five sun rays and 24 sky rays; birch's six sparse sprays and 2 m spray depth remain provisional interpretation.
Contact scale is 0.025–0.12 m. Reveal selected people by fading covering crowns/roofs and quiet obscured-band silhouettes, without changing obstruction or shelter rules.

### A4.5 Water, fire and weather (`PRE-26`, `PRE-30`)

Water, bed, feet, clipping and picking use the same surface/footprint. Refraction reads the prepared bed, not foreground people. Reflection, depth and shore readability remain incomplete.
Fire's visible base touches sampled ground; its emitter may be elevated. Walls block firelight.
GPU rain/smoke never wet cells or decide outcomes.

### A4.6 The bridge, jobs and caches (`WLD-13`, `TIM-17`, `PLT-09`)

Jobs own immutable inputs and carry full world/data/look/renderer/format/epoch/revision/dependency identity.
Skipped publications require revision resynchronisation; workers never consult live state.
Current Place/Activity coordinates are 2D; fixture up-fields are centimetres, future physical heights millimetres. Add explicit units and surface references.

Keep coarse parents until replacements are fully prepared/uploaded; publish colour/material/normal channels atomically.
Allocation tickets include old/new overlap and remain charged until worker acknowledgement and actual detachment/disposal.
Bound queued work, staging and every cache; count reductions, gutters and all channels. Deleting render caches must leave saves/digests unchanged.
The current adapter has one decoder, one channel upload/frame, retry/backoff and two-frame delayed disposal; quotas still need frame-headroom evidence.

### A4.7 Godot and phones (`PLT-04`)

Record actual renderer/driver; fallback is not a Mobile result. Measure the owner's phone first; a second device is evidence only once named and available.
Use cheaper passes and the same-grid 30 fps mode before proposing visible changes. Missing timers/counters are reported as unavailable.

### A4.8 Tests (`PRE-31`, `RES-05`)

Frozen cloud captures prove rendering regressions, not phone feel. Check changed scenes through normal navigation and direct entry, both orientations, slow motion and pinch release.
Picking must undo every transform and return persistent IDs; GPU IDs map through a CPU table, never 64-bit IDs packed into floats.
Compare outcomes at equal game seconds across views, workers, save/reopen and cache eviction.

## A5. The look

### A5.1 The pictures

`art/targets/` supplies owner-chosen light, colour, density and composition. Targets are neither shipped nor traced; flagged anachronisms are not approved content.

### A5.2 Guidance about the feeling

Quiet pixel texture, muted varied greens, small strong-colour accents, truthful hour/season light, dark nights with small warm pools and people readable through real motion/light/reveal.
Ordinary near figures have no heavy outlines. Archaeological truth and `SCP-20` constrain objects, clothing and motifs.

### A5.3 Three art families (`PRE-22`)

Author at 64/16/4 internal pixels/metre; review 32/8/2 and farther reductions. At 2× these are twice the physical densities.
Colour/material/normal pages share alpha, trim, pivots and gutters. Never interpolate categorical material IDs; renormalise normals.
Pack complete halving chains separately for each authored family: 64→16→4 is not a valid consecutive mip chain. Lossless file size does not measure GPU residency.

### A5.4 Art production (`PRE-20`, `PRE-42`, `PRE-46`)

Build the next scene's needed pieces. Preserve signed sheets, exact source images/prompts, provenance and approval records.
Sheet, runtime-page and engine approval are distinct; readable early stand-ins are allowed by the approved `PRE-31` change.
Physical catalogue rules stay in the validated data pipeline. Do not infer heights/materials from colour.

**Retained asset state:** 129 of 373 catalogue designs are signed. Meadow, boulder, both birch seasons and covered tent passed static production review; engine approval is separate. Hazel 8.1 is signed after its tall-sheet correction.
Person/deer proposal sheets passed critic review but await owner sign-off; they are not production walk loops. First-people and full-age art, tent interior/back/frame/floor remain missing.
The tent is the signed 16.4 cone, not the old 16.5 dome: ring 4.2 m, opaque cover 3.8 m across × 2.6 m high, crossing 2.7 m, tips 3.1 m; exclude ring/tips from opaque shadow volume.
Birch dimensions remain 20 m high, 7 m spread, shaft 0.25 m, collar 0.38 m; preserve one trunk across seasons.

Ground 1.3 earth, 1.4 bank gravel and 1.5 river-bed states are accepted exports: six packs, three variants/families, 1,458 aligned pages, 4/16/64 m family spans. Runtime batch visual review is pending.
Ground 1.6 hearth fresh/old and 1.7 mud wet/drying are partial WIP. Inspect their source-index/pack-contract before use; no complete export/review is claimed.
Their optional binary water coverage is separate; soil stays material 2, charcoal/wood 5. Puddle sizes drifted across families; wet ≤1.25 m/drying ≤1 m fixes and joins exceeding 2 m still need whole-chain review.
No blanket unsigned-to-production approval exists; use the approved early stand-in scope without marking designs signed.

### A5.5 Targets and the loop

Compare changed engine views with approved targets; visual statistics warn, never approve.
Keep existing data-driven target-card definitions rather than copying their thresholds into prose.
A visible saving requires the owner's blind phone comparison: ten random pairs, eight correct means visible. A heat-driven visual step must be planned, logged and already accepted as invisible.

### A5.6 Truth in pictures (`PRE-42`, `SCP-20`)

Inspect made things, species and dress against their evidence. Check doubtful content before runtime production; no borrowed cultural motifs, metal before copper or invented equipment.
Keep rejected studies as provenance, never silently promote them to approved sources.

## A6. The art kit and animation

### A6.1 The kit (`PRE-46`)

Compose bodies, garments, tools and condition overlays from records; optional offline rigs are tools, not runtime requirements. Full catalogue breadth remains deferred under PROJECT.md.

### A6.2 Appearance (`PRE-42`, `PRE-43`)

Stable look keys choose approved variation; actual inputs, equipment, age and condition determine appearance. Attachments share pivots/facings/trim.

### A6.3 Movement (`PRE-27`, `PRE-44`)

Initial production baseline: four facings, six-frame walk, idle/carry/cut/tend-fire poses and a small clothing set. Compare eight facings on one animal before expanding it. Full motion requirements remain open.

### A6.4 Surfaces and picking (`PRE-24`, `PRE-33`)

Pick visible surface order, object bounds and alpha after undoing presentation transforms. Flat inverse projection cannot pick slopes or floors below roofs.
Deep caves have separate surfaces/views; arbitrary stacked overhangs remain outside the scoped terrain.

### A6.5 Sheets and review (`PRE-31`)

Review true-size and enlarged pages, frames, masks/normals and the changed running scene. Stand-in readability permits early delivery; it does not confer final art approval.

## A7. The world

The following contracts are deferred to M7 unless needed earlier by a scoped camp behaviour. PROJECT.md owns all full-world numerical acceptance; the small valley closes none of it.

### A7.1 What this milestone supplies (`MIL-14`, `PRN-02`, `WLD-08`)

M7 owns physical geography/weather and available ecological inputs. M8 owns full ecology/living settling, remaining individual/herd animals and biological start/survival acceptance. Each full check awaits its real consumers.
Reuse the existing simulator, not a second world engine. No planet formation or live tectonic solver.

### A7.2 Coordinates, cells and the polar barrier (`WLD-01`, `WLD-02`, `WLD-03`, `WLD-12`)

Retain exact torus coordinates; cells are 1 km, weather cells 10 km and areas exactly 250 m (251 shared metre-grid vertices).
Coordinate wrapping does not permit transport across the middle of the polar ice. One edge policy must cover diagonal/swept water, weather and animal/person movement. Globe distortion is display only.

### A7.3 Ownership and stored fields (`WLD-12`, `PRN-14`)

Use compact canonical arrays for regular fields and flat sparse feature tables, not an entity/allocation per cell.
Separate immutable ground, changing physical stores, authoritative area deltas and disposable picture products. Biome potential is not actual species population.

### A7.4 A reproducible generation job (`TIM-16`, `WLD-08`, `RES-05`)

Versioned inputs determine work counts; wall time never chooses iterations/candidates. Publish stage digests and resume identically. Fixed chunks gather by stable cell/feature order using existing numeric/chance APIs.

### A7.5 Plates and rock (`WLD-06`, `WLD-09`, `WLD-30`)

PROJECT.md defines causal generation and geological obligations. Keep provenance for rocks/resources; visual colour is not a physical property.

### A7.6 Rough climate, erosion, basins and fixed waterways (`WLD-08`, `WLD-09`, `WLD-17`)

Separate physical bed, routing/spill metadata and water levels; filling drainage depressions must not erase lakes.
Shared reach geometry owns river joins across boundaries. Cave certificates refer to finished terrain/water, never caves inserted to rescue a scored start.

### A7.7 Final climate, seas, soils and life potential (`WLD-16`, `WLD-26`, `WLD-27`, `WLD-09`)

Retain PROJECT's approved climate comparison, short-year units, species and soil/deposit rules. Initial habitat estimates cannot pass ecology acceptance. Freeze external reference definitions before tuning.

### A7.8 Candidates and the start region (`WLD-10`, `WLD-24`)

Versioned ranking never edits a candidate. Keep root seed, candidate seed/index and making digest distinct.
Retain at most one full working candidate plus checked temporary artifacts. Cancellation leaves the saved world untouched. Explicit seeds face the same hard gates.

### A7.9 Settling and the beginning of history (`WLD-08`, `TIM-14`, `WLD-11`)

Full settling is exactly 600 days under play rules, then bands/history. Water-only settling is labelled incomplete.
Current World starts at frontier zero; negative calendar dates do not mean negative events are supported. Introduce a versioned prehistory origin/offset without resetting hazard state or changing demo proofs.

### A7.10 Sky and weather (`WLD-07`, `WLD-16`, `WLD-22`, `WLD-30`)

The owner removed universal eclipse-count certification on 8 October; coherent sky and a reviewed plausible distribution remain.
Hourly storms must integrate swept footprints, respecting the polar boundary. Real wind/travel speeds do not receive seasonal compression; natural-rate tests exclude forced events.

### A7.11 Water and soil in play (`WLD-17`, `WLD-26`, `WLD-27`)

Keep fixed cadences, explicit stores/residuals/in-transit water and deterministic reductions. Daily→flood-hour promotion cannot duplicate or lose water.
The upstream test's “about a day” is 21.6–26.4 hours. Local cultivated fertility belongs to the field, not every field in its world cell.

### A7.12 Quakes and eruptions (`WLD-15`, `WLD-22`)

Hazards use saved warning/cooldown state and fixed game-time opportunities, never camera loads.
Dated disturbances needed for area catch-up survive public-history thinning; compact only to proven equivalent state.

### A7.13 Detail on demand and kept areas (`WLD-12`, `WLD-13`, `PRE-03`)

Separate canonical rule facts, pure picture detail and saved changes. Absolute-coordinate samples and shared halos prevent seams/load-order dependence.
View catch-up runs on a copy and saves nothing. Later activation must equal no viewing. Count global depletion once; inactive-area usual-weather rules need not equal active hourly-weather rules.

### A7.14 Events, saves and revisions (`PLT-07`, `PLT-08`, `PLT-09`, `RES-05`)

Append layer-owner IDs, preserving old owners/order. Changing the owner-count encoding makes earlier saves unreadable; refuse them (A3.7). Old demos attach no new systems.
Current SYST holds system payloads; rich disturbances exceed history's small type/two-integer payload. Add bounded versioned payloads deliberately.
Publish immutable bases and revision manifests sampled at displayed time; caches are never authoritative archive dependencies.

## A8. From a person to the globe

### A8.1 Ground, height and order (`PRE-03`, `PRE-23`, `PRE-24`)

Render pieces are disposable, separate from 250 m simulation areas. Split trunks/crowns, cliff caps/faces and shelter layers; order overlaps across chunks with persistent-ID ties. Ordinary Y-sort cannot solve raised shelves/roofs.
Quiet base variants, edges, broad stamps and ecology detail draw actual records. Resource inspection cannot rely on decorative guesses.

### A8.2 A nearby origin

Subtract an exact nearby origin before drawing floats; negate simulation north into drawing south. Current camera rebases exactly every 4096 m and preserves both absolute and raster origin. Saves/rules never depend on rendering arithmetic.

### A8.3 Zoom steps and forms (`PRE-03`, `PRE-28`)

Current WIP has 19 internal density stops, 2^-12 through 64, with 160 ms anchored logarithmic settling. Owner found snapping too obvious; no phone acceptance.
Wide records preserve actual IDs, sorted membership, sampled time/epoch/revision and truthful truncation; limits are 512 rows/8192 members. This does not prove wide art/navigation.

### A8.4 One projection (`PRE-02`, `PRE-33`)

Camera faces north at 37°, never rotates. For local east X, south Y, height Z and internal density s:
`x = centre_x + sX`; `y = centre_y + s(sin(37°)Y − cos(37°)Z)`, subtracting projected focus.
One native helper owns projection/inverse/bounds; art tools follow it. UI hits precede world gestures.

### A8.5 World map and globe (`PRE-29`, `WLD-01`, `WLD-02`, `WLD-03`)

Deferred M7 geography/M9 presentation: one 2:1 overview feeds map and 2D globe disc. Repeat longitude, clamp latitude; never blend opposite polar rows. Inverse taps return true torus coordinates; no sphere routing or save migration.

### A8.6 Time by zoom (`TIM-01`, `TIM-04`, `TIM-15`)

Keep existing pace data and request precedence: pause, skip, manual/lock, director, zoom as consumers arrive. Zoom changes requested speed only. The first power is M2; the director remains later.

### A8.7 Budget

A18.1 owns graphics lines. Measure real busy camps, not only six-object fixtures; broad comparisons wait for their consumer and available device.

### A8.8 Replacing the M2 fixtures (`PRE-03`, `PRE-29`, `PRE-30`)

M2 connects real camp records to existing providers. M7 adds generated land behind that interface; preserve fixtures for regressions and one snapshot consumer.
Full detail within 300 m/coarse ground to about 10 km/overview beyond remains deferred. Parents stay visible; no generation in gestures.

### A8.9 Cliffs, caves and the geological slice (`PRE-23`, `PRE-24`, `PRE-25`)

Deferred M7: one surface provider feeds drawing, feet, water, picking and shadows. Sections sample actual rock/soil/water/cave/dated records with declared scales/width; a synthetic buried object cannot prove a 200-year historical camp.

## A9. Living things

M2 needs real scoped supplies and bounded renewal, consumed exactly once. Later species/biome/ecology obligations remain in PROJECT: M7 supplies physical habitat inputs; M8 completes living ecology and settling. Camera-near is never person-near.

## A10. People: bodies and lives

M2 adds stable people, hunger/thirst/fatigue, senses and eating/drinking/resting through Activity events. M4 adds scoped care, generations and consequences; M8 finishes the remaining body catalogue.
Use PROJECT's demographic and birth-spacing targets; old architecture estimates are not alternate tuning targets.

**α2.13b contract (before code):** the bounded adult camp uses whole remaining food/water units and awake seconds, with retained division remainders. Catalogue daily rates are 4 kg raw berries and 3 L water, with eight hours of sleep restoring sixteen awake hours. These are scoped estimates, not complete health, diet or ecology. Short activities keep real-life duration. Need scores use expected benefit and urgency minus travel/work time; stable option order breaks ties. Idle watching is a timed fallback, never a food source.

Actions have saved goals and phases: walk each reachable route segment to a keyed position in the catalogue's small interaction area, gather berries into hands, carry them to shelter, eat; walk to water and drink there; walk to shelter and rest. No container or unknown craft is granted. Eating/drinking/rest give elapsed shares, tracked cumulatively so interruption cannot apply a share twice. Intake and bodily depletion combine before clamping the elapsed interval. Gathering removes only available material at settlement; an interrupted gather keeps its earned handful. Walking keeps its sampled position. A need crossing below 20 wakes an interruption timer; an independent hard-limit timer interrupts at thirty-six awake hours and forces rest at the current reachable position. Decisions and their two best alternatives retain actual need values, benefits, time costs, exclusions and scores. History tags 100–103 carry the chosen need and packed need values, 110–113 the scores, 120–122 benefits, 130–132 time costs, and 140 packed exclusion reasons.

Camp paths use a bounded one-metre grid with centimetre coordinates and a saved rock rectangle, with deterministic four-neighbour routing and blocked sight lines. Choices read remembered locations/amounts, not remote stock; arrival updates stale knowledge. At most three supplies are noticed per game hour, by day within 50 m and by night within 5 m. Each remembered fact keeps its source and observation second; own-task observations are immediate. Starting adults know the shelter and gathering skill 3, with no invented craft. New positions are keyed scatter beside shelter.

Renewal is physical and bounded: the spring transfers a catalogue flow from a saved finite upstream water budget into its capped pool. A living berry stand grows at a catalogue rate only while saved root water and a saved seasonal production budget remain, capped at its initial standing crop; picked-out fruit can regrow, an absent stand cannot. Both budgets and all transferred/consumed quantities are saved. No midnight refill. Their hourly events and person actions share one camp island, preserving key order on one/four workers. CAMP4 requires critical LIFE2 and DRMS1 extensions without changing foundation registry layouts. Readers validate bodies, pending activities and matching live events. Older formats are refused; current-format corruption recovery still uses the newest whole snapshot and journal replay.

**T3.13c.1 storage:** format 4 refuses earlier camps. Craft camps require critical FIRE1/THER1 with the CAMP fire bit. Fire and food exposure stay on physical item identities; mass, references, bounds, ordering and live camp deadlines are validated before opening. THER1 covers every person and camp. Slot 2 owns the earliest ambient/fire transition and slot 3 the earliest food deadline; hourly renewal keeps slot 0. Fire/thermal state survives a labelled zero-person setup because feature detection follows camp records, not the last mind. T3.13c.3 settles fuel into conserved ash, wet inputs into a vapour ledger and quenched fuel into physical charcoal. Saved deadlines cover ignition, heating, exhaustion and ember expiry. Finite tending reserves and splits only its portion; banking cannot extend the same ember repeatedly. Felt temperature adds 15 °C within two metres of heat 2+, once regardless of overlapping fires. Fixed-point thermal sampling settles before activity/heat changes and retains extra-water remainders. Actual awake warmth becomes handling evidence. Roots/meat retain elapsed heat through moves and splits: one first-hour chance, raw failure, and burnt state after two hours or one hour at heat 4. Physical change precedes any noticing; history credits actual noticers. Copied fire/timer/thermal ways supply display-time quantities without reading producer registries. A fresh cold-hearth variant burns the same initial five kilograms into ash and adds no knowledge.

## A11. Minds

M2 chooses among known reachable actions from data-defined needs/affordances and retains actual reasons for inspection. Per-person knowledge has source/date; choices cannot read unknown world truth.
Decide at action ends/interruptions, not frames. Save memory, pending actions and dream influence. Add planning/social depth only with the M3–M5 consumer; M8 closes remaining obligations.

**T3.13b.2 observation:** `demo/learning.cpp` settles exposure before the observer changes activity and before the maker resolves an intentional try or interrupts. It uses torus positions along saved activities, the camp rock sight test, a 500-cm inclusive limit and the existing 06:00–20:00 daylight window, or actual heat-2+ task light within two metres of both people with wall sight checks. Each visible whole-second interval weighs four for that person’s deliberate `watch_craft` target, one while busy, zero while asleep. Ordinary watch choices can select a visible working person, with stable-ID ties. At the demonstrated end, accumulated exposure becomes millionths of a quarter, divided by the full use’s duration; twenty quarters teach skill 1 with the maker/event and watched route. The first partly or fully observed use gives a separate hunch. A saved per-work/try cursor prevents counting exposure twice; completed records are removed, and the saved result marker rejects repeat application. Cancelled strikes give no demonstration; paused gradual work retains its pending exposure until completion. Accidental fits retain their existing personal discovery route; observation learning follows intentional demonstrated uses. No camera or hourly site survey grants credits, and no global history index unlocks a mind.

**T3.13b.3 teaching:** kind adults ask nearby awake peers about personally known crafts; a truthful answer supplies absence evidence and telling supplies only a hunch. Offers consult that evidence. Comfortable learners with no unfinished plan collect reserved inputs and walk the ordinary route to the teacher. Shared attendance is capped at thirty minutes within two metres and daylight/sight; urgent needs or calls settle both bodies and credit each elapsed second once. Gradual work keeps its progress across interruption/reopen; cancelled strikes keep no result. First supervised success teaches the recipe at full maker chance, with its actual source. Shared effort uses the fourfold/teacher multiplier; ordinary successful effort counts twice failure. Choices settle saved curiosity decline, hunch expiry and skill/sector fading. The existing generic scrape recipe remains the hide consumer; no new craft names enter choice rules.

**α2.13c contract:** a place dream uses only one of a person's three remembered supply/shelter sites. At sleep it replaces that night's ordinary place dream through the same thought/choice function. At the first rest in each night, a keyed 1-in-60 natural place dream keeps the weakest known need’s site (newest observation, then stable subject order breaks ties); other place dreams are unkept. Ordinary strong place dreams can occur naturally; a kept thought has a fixed mild score pull (60), expires after three days, and refreshes without adding strength. It can favour a known use or an ordinary visit/watch choice; urgent need below 20 and exhaustion always override it. It never interrupts work, teleports, grants knowledge or starts a requested activity.

A separate critical DRMS extension and CAMP 3 preserve personal ordinary dream thoughts plus a private camp ledger: requested/received/executed moments, subject position, status and observed choice/arrival. The ledger alone owns queued requests (at most three worldwide in this one-camp slice), one sent dream per sleeper/night and three per night. Nights turn over at 06:00; queued work is checked again at the next sleep and cancels for a vanished subject or exhausted cap. Minds contain no sender, request number or player provenance. Older CAMP formats are refused before loading. The cleanup removed the unused migration-seal APIs and old-format snapshots.

Opening a power chooser pauses the runner between batches and explicitly settles the display to its final frontier before presenting subjects. Confirmation records that displayed whole second, durably journals one command, and drains that command within one game second; sleep delay is separately shown. Power requests are synchronous and cannot accumulate runner jobs. Cancel restores the previous pause state without issuing a command. The only supported camp rates remain 1, 60 and 3600 game seconds/real second. The private record observes the first later choice, its ordinary dream contribution and actual arrival; it does not claim a changed outcome from correlation. Personal cards date recorded choices and remembered dreams in plain clock words. A dream delivered during sleep publishes its thought at the delivery event while preserving the original sleep interval; earlier display times retain the previous thought. Arrival must lie within the remembered site’s use area, so bringing gathered food home never counts as reaching the food site again. It is observed even when urgent needs give the dream zero pull; the record still says needs led the choice.

The separate Camp performance test runs a fresh isolated camp for ten real minutes (200 seconds at each supported rate), with five seconds of frame warm-up per phase. It keeps frames, clocks, CPU thread time, memory, battery, heat and dream reception latency in a copyable report, distinct from the short owner play route. Accelerated smoke results carry their time scale and platform and cannot stand in for a sustained phone result.

## A12. Crafts and discovery

M3 implements one generic characteristic/blueprint chain, flaking then fire, with material conservation, actual discovery/observation and learning. Blueprint names cannot replace predicates.
Keep `RCK` evidence and `MAT-17` valid/invalid fits; no scripted discovery dates. Later chains/catalogue and full pace acceptance remain M8/M10.

The α3.13a Discovery scene records finite additions and materialises aggregate stone/wood once into existing things. Portion reservations and retained tools exclude competing work and meals. Actual fits use physical predicates; choices use only personal evidence. Known tries settle at saved events, unknown fits roll once per whole activity, cancelled strikes produce nothing, and paused gradual work retains elapsed progress. Nutrition and tool wear keep division remainders; inputs, leftovers and broken pieces conserve mass. Result history records actual maker, inputs, time and discovery route; only noticed surprises grant knowledge.

World events publish copied item, work and knowledge changes. Display queries select records at screen time, hide unborn results and never consult mutable producer registries. Craft camps currently use serial event order even with four workers configured because thing births share global IDs; equal continuation is proved, parallel craft speed is not. First flake is recaptured from an ordinary declared seed in each build's own format, with no switches, and establishes neither frequency nor the full M3 gate. Observation/shared practice and physical fire/cooking are built; injury remains M4.

The learning path batches reservation reads once per decision and indexes active items by position and owner. Event mutations publish `Context::item_changed`; raw edits outside events invalidate the derived index, and run/open boundaries rebuild it. Sight groups preserve the first insertion and last physical evidence in canonical ID order. Blocked routes use a bounded, mutex-protected cache keyed by all geometry and endpoints. Stationary observation counts the same daylight seconds in one interval; moving sight keeps whole-second checks. None of these caches enters saves or digests.

## A13. Culture

M5 proves knowledge/relationships between two camps; M8 finishes culture. Names use supported font letters. No scripted historical outcomes; retain actual causes and individual holders.

## A14. Story, the book of ages and the writer

M3 starts event-backed history; M6 finishes the small book with pattern text. Director changes speed/moments only, never outcomes. Optional later writer only rewords verified records (`PRE-37`); no provider/API is selected here.

## A15. The interface

World first, truthful person cards, pause/speed, save/world controls and one indirect dream in M2. The camp menu gives direct access to Device check, Camp performance test and Test reports.
Use one gesture reader/theme, safe insets, ≥48 dp targets with 8 dp gaps and both orientations.
Current bitmap font has 135 glyphs (ASCII 95 included), base 16/line 20/baseline 15, integer physical sizing and retained licence/provenance. Arbitrary Unicode lacks coverage; the Japanese probe aliases a question mark. Production fallback remains unresolved.
Deferred layout batching avoids the previous quadratic catalogue freeze.

## A16. Sound

M3 introduces sounds from actual action/fire; M9 completes sound obligations. World-space distance survives a 2D picture. Keep the 32-voice bound; full mix, language murmur and phone cost are unproved.

## A17. Testing and checks

Testing policy: owner OK 8 October 2026. Keep the routine/delivery/audit split; IMPLEMENTATION.md owns delivery instructions.
Routine tests cover native/view/app/tools, accepted deterministic proofs, catalogues and document structure and ID traceability checks. Native lint and source rules are in the routine gate. The full audit adds compiler/architecture/thread/sanitizer/kill/repeat/render stress and runs only at the milestone end (owner, 9 October 2026).
Optional ID annotations locate evidence; they do not certify full acceptance. Current-save corruption/recovery and determinism regressions remain mandatory; older saves are refused.

### A17.0 Traps met so far

- Direct Examples startup once hit parent-busy `add_child`; the shared stream attaches deferred and both entry routes have regressions. Test both routes.
- Load the bitmap theme at runtime, after import; loading it as the project theme blocked a fresh font import. Commit generated script UIDs; a second import succeeding is not clean-import reproducibility.
- Movie Maker uses project/override window dimensions. Assert actual viewport size; naming a driver alone can select Forward+ unless renderer is also named.
- Headless input drops touch events; use the display-backed harness. A control draws behind its children; wrapping labels need container layout before measurement.
- Reapply the runtime frame cap after swapchain creation; the phone otherwise stayed at 120 Hz. Thermal forecasts require regular polling.
- Keep all save/sample I/O on the keeper thread: competing direct writes previously caused a real race.
- Retain regression coverage for off-screen shadows, underwater clipping/picking, separate trunk/crown order, grounded flames, real-time mask throttles and final bed order. Semantic captures clear to ID zero.
- Art reductions must preserve singleton coverage at 1×1; preparing cached winter inputs must not overwrite repaired outputs. Trace exposed bark locally; a global low-chroma classifier greys leaves.

### A17.1 Generated-world proofs (`MIL-14`, `RES-05`, `RES-21`)

Deferred M7: add stage digests, structural/natural-rate scenes and camera/cache/catch-up equivalence. Preserve M1 proofs (older saves are refused, A3.7); test barriers, river continuity, conserved stores and canonical placement with actual failure cases.

Retained gate details from the previously adopted world plan apply at M7's physical-world consumer and M8's complete ecology/settling consumer, not M2/M3. PROJECT's numerical checks also remain; these details fix denominators and stricter checks absent there. No test sample is silently reduced. Initial session-hour caps require checkpoints and an overrun report, not a claimed pass.

| Gate | Additional retained check | Initial budget |
|---|---|---|
| G1 terrain | 100 final land/tilt seeds: ten equal bins each represented; 20 fresh worlds all pass geometry. Rivers enumerate every ≥50 km² channel independently of registration, split only at headwaters/confluences/terminals; missing geometry fails. Coast straightness: >22 km staying within 500 m of endpoint chord fails. Primary slope uses 1 km central differences on land excluding permanent cap; report cap and 25 m/1 m slopes separately. Lake fraction denominator is dry land plus freshwater lakes, excluding permanent ice. | 2 session-hours |
| G2 geology | 20 rock/100 deposit worlds, no invalid occurrence. Independently check causal envelopes, hosts/exposure/provenance; matched stones never become less rounded with longer routed transport. | 4 hours |
| G3 candidates | 100 roots, all first 20 candidates fully qualified offline: ≥25% of 2,000 qualify; report each root, not only finalists. Paired replay preserves offered order; biological confirmation awaits real yields/lives. The former 2–5 frost-night interpretation remains proposed, not a new decided requirement. | 8 hours |
| G4 climate | Every place in 20 worlds ×20 years; report near-zero rain cells with the approved allowance of one stored precipitation unit over the whole run. Matched warmth target ±2°C/outer ±2.2°C; independent rain-shadow/coast checks. | 2 hours |
| G4b natural rates | Reuse 20 worlds ×100 years; ≥1,000 eligible cases per kind/climate for storms, lightning, drought, floods and harsh winters, plus wildfire when supported. Freeze references, thresholds, de-duplication, exposure and short-year conversion; no pooling away a failing climate or counting injected events. Check the dry-spell envelope's 5% exceedance with RES-13; source flash-to-ground-strike conversion separately. | 2 extra hours |
| G5 sky/hazards | Sky direction error ≤0.1°, daylight ≤2 minutes in nonsingular cases; explicit polar cases. 20 worlds ×100 years: zero barrier crossings, valid hazard locations/origins/cooldowns, ≥1,000 opportunities per kind with RES-13 shares (rare shares half-to-double). Universal eclipse certification is removed by owner approval. | 4 hours |
| G6 areas/water/sections | 10,000 shared edges exact; 20 kept-area daily/season catch-ups exact; at least 16/20 natural valleys meet declared flood/spring behaviour, subject to RES-13 reruns. Section sample values exact; zero camera-caused save changes. | 2 hours |
| G7 phone | ≥3 minutes held load, unplugged; freeze 20 ordinary roots plus 5 fallback roots. Include settling for each. Area/detail/open target/outer limits: 0.1/0.11 s, 1/1.1 s, 3/3.3 s. Every minute of 20-minute route meets frame line; ≥10 world-alone game years/minute on two middle cores; every digest matches cloud. Repeat as ecology/bands arrive. | Generation suite separately about 100–110 minutes plus setup; resumable |

G7's 50-ms-late rule means maximum frame duration 66.7 ms at 60 fps or 83.3 ms at 30; retain the separately stated graphics line in A18.1 until reconciled by measured acceptance. Train on named seeds, close on fresh ones, retain failed samples under RES-13.

## A18. Budgets and risks

### A18.1 Budgets

These existing graphics lines remain obligations to measure when their consumer exists, not results or a matrix for every delivery:

| Measure | Line |
|---|---|
| 60 fps | 16.67 ms; GPU mean ≤8 ms, p95 ≤9.5 ms |
| Main thread | mean ≤8 ms, p99 ≤12 ms |
| Frames | ≥97% on time, slowest ≤66.7 ms |
| Busy woodland camp | GPU mean ≤6 ms |
| Draw count | initial diagnostic ceiling 300 |
| 30 fps | 33.33 ms, unchanged world grid |
| Sustained route | 20 min; mean ≤4 W; final battery ≤40 °C; thermal status none; every minute meets frame line |
| Opening | texture preparation ≤10 s after install; warm world ≤3 s |
| Fixture process memory | ≤1 GiB; generated world ≤2 GiB; full-game PROJECT limit unchanged |

Deferred full graphics evidence retains seven-person day, three-fire night, a 30-person woodland camp with water/smoke/weather, crowd stress, map streaming and repeated pinches. Compare identical Compatibility/Mobile scenes at 60/30 fps when that renderer work is scheduled and the required phones are available.

Current stream caps: queued 32, preparing 1, input 8 MiB, prepared 64 MiB, staging 4 MiB, resident 128 MiB, targets 32 MiB. Resident partitions: maps 48/sprites 48/ground 16/masks 16 MiB; partitions are planning allocations.
Ground demand is at most 256 rows over authored 4/16/64 m cells. Count backend residency separately from ticket accounting.

Retained deferred visual lines: ground accents about 20 and ≥90% of near at each resting band; repeat correlation ≤0.2; median person salience about percentile 80, none below 70. These calibrate against owner verdicts, not substitute for them.
Small-hearth light halves around 2 m and reaches a tenth around 3.25–4.25 m. Globe incremental GPU goal <1 ms is unmeasured.

M1 phone evidence only: 10,000 markers, 99.8–100% on-time frames, worst 49 ms, 330–410 MB, about 1 W real speed/5.6 W top speed; snapshot copy ≤20 ms, open 143 ms. These are not proofs of minds or the 2D camp.
History's current 68-byte records threaten the full 4 GB target; measure actual lives/history before compacting, preserving permanent events.

### A18.2 Risks

**Repair before relying on stopped WIP:** `_background_plane`'s hidden/empty return and `_receivers`' null-texture branch retain old colour/normal/material references. Detach before retiring allocation tickets; hiding a node does not release its textures.
The first streaming benchmark sampled preceding frames and its readiness timings are invalid. Rerun after aligning below-2-density ground-slot readiness and frame completion; receiver skipping below that density has no final timing proof.

Whole α2.9a review remains open, including committed changes after accepted α2.8a; native 57/57 alone does not accept the step.
The reviewed routine baseline fails formatting/lint, two app cases (fixture orientation and terrain selection). IMPLEMENTATION.md begins with repair; no passing check is claimed here.

Wide data exist, but tiny figures/group/camp art, transitions, ecology details and named wide navigation are unfinished. `wide_study.gd` is only a study.
Explore can overflow landscape; the source viewer overlays the scene. Recent screen fixes need fresh captures. Owner reported tiny/busy UI, grass stripes, questionable birch/boulder/tent shadows and duplicated Camp title; no all-screen approval exists.
A correct light equation does not approve the camp composition. Latest software-renderer captures and native tests do not prove phone heat, speed or touch comfort.

### A18.3 Wider-world budgets (`WLD-11`, `WLD-12`, `PLT-04`)

Deferred M7 obligations: PROJECT's 180/198-second generation and 60/66-second settling lines include fallback search/entry saving; world layers ≤0.2 one-middle-core seconds/game day; rule-area preparation ≤10% of simulation, about 0.1 s/area, visible detail within about 1 s.
The prior 45/90/35/10-second stage split and 128-byte/cell design were estimates, not measured allocations or requirements.
Keep the 2 GiB peak-process line; measure structs, temporary candidates, GPU residency and snapshot overlap together. Two million cells' daily work is a larger risk than their storage.

### A18.4 Wider-world risks

Freeze hard gates before tuning; report rare qualification instead of repairing candidates. Revalidate full-resolution starts after settling.
Save/catch-up growth needs byte/year measurements and equivalent compaction. Terrain expansion needs early cross-chunk ordering/shadow fixtures.
Outstanding biology, ecology and historical-trace checks remain open until actual consumers prove them; a partial stage never certifies the full item.
