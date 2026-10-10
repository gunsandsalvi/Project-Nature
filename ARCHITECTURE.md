# Kindling: architecture

The contracts, current implementation limits and facts needed to build the next running game.
PROJECT.md owns requirements; IMPLEMENTATION.md owns sequence and acceptance. Aim for at most 6,000 words here.
Existing A section numbers remain stable because code cites them. Deferred sections preserve obligations, not permission to start work.

## Status (10 October 2026)

M1 accepted; M2 delivered, play answers open. Owner closed M3 partially on 10 October; static independent re-review completed, failed gates/phone evidence carried to M4. IMPLEMENTATION.md retains results and scope. A19 is proposed M4 design, dependent on review of PROJECT amendments; it is not built. Twenty-five adults do not certify full populations/geography/lives. Markers remain diagnostics.

## A1. Overview

### A1.1 What it must deliver

Offline, saved, deterministic independent people through fixed-camera 2D. The labelled seeded valley starts with about 25 adults; full-world/population acceptance remains open.

### A1.2 The big picture

`sim/` owns state, independent of Godot/camera/zoom/speed. `view/` commands and owned snapshots feed `game/` drawing/UI/sound; validated `data/` defines rules. Rendering, caches and view-only chance cannot change history (`WLD-13`).

### A1.3 Decisions

Keep Godot 4.7.2/stock templates, C++20/GDExtension, Compatibility, north-facing 37° camera and current saves through M3. Change implementation for measured blockers; later Mobile comparisons use identical scenes.

## A2. Code layout, builds and delivery

### A2.1 Repository layout

`sim/`: independent rules/tool/tests; `view/`: bridge; `game/`: app/tests; `data/`: TOML/scenes/benchmarks; `art/`: sources/targets/approvals/assets; `tools/`: build/check; `dist/`: delivery. Generated `game/bin/` and `game/data/` are uncommitted.

### A2.2 Builds

CMake/Ninja/ccache build host/arm64. godot-cpp stays 4.5 until a needed API; argument structures belong in its reduced profile. Errors are values, not exceptions. Keep `-funsigned-char`, `-fno-fast-math`, `-fno-math-errno`, final `-ffp-contract=off`. Android: API 24, static C++ runtime, guarded APIs, 16 KiB alignment.
Export unsigned, compress/align/sign; only `tools/signing-key.py` reads secrets. Explicitly include text catalogues/reports; exclude tests/addons, request no permissions. Deflated libraries require `extractNativeLibs=true`. PNG palette/alpha changes require exact decoded RGBA equality; profiled sources keep encoding. Package meadow once; required catalogue deletion is no size fix.

### A2.3 Delivery of each alpha (`PRC-11`, `PLT-06`)

`dev.kindling.app`, same release key; increasing code `(milestone + 1) × 10000 + alpha × 100 + step`, a=1; never reuse distributed codes. APK ≤50 MiB repository limit; alternative distribution/visible compression needs owner decision. Size/note/checksum identify delivery.
Camp starts directly. Once-per-build smoke compares one/four workers, numeric environment/catalogue fingerprints and saved-moment/device diagnostics; Menu → Device check holds the report. Note: What is new/try/rough, APK link, proved/remaining checks. Check-key builds are labelled and never replace the release APK.

### A2.4 A fresh cloud session

Install missing dependencies with `tools/setup.sh`; source `tools/env.sh` each command. Reuse pinned engine/templates/caches; avoid duplicate heavyweight builds and concurrent captures/exports.

## A3. The simulation core

### A3.1 Its boundary

Same library in play/headless: make/open, advance, commands, snapshots/events, save/export/import. It receives bytes/save folder, never calls Godot.

### A3.2 Entities and components

EnTT is behind the entity layer. Never-reused 64-bit IDs encode family in four top bits; transient registry handles are unsaved. References/history/commands use IDs; ended entities leave historical records. Shared descriptors define stable names/versions/fields/units/ranges/links/effects for load/save/digest/inspection; catalogue composition replaces kind class hierarchies.
Person/Place/Home/Activity/Schedule hold names, starting age, appearance, bounds, ml/mg supplies and actual sites. All 25 adults choose/move through Life. CAMP4/LIFE2 extend canonical registry storage; foundation fields remain unchanged. Person worlds require complete valid linked records/activities. Foundation worlds retain bytes/digests inside format 6.
Decisions follow event keys/IDs and explicit ties, never pool order. Create pools in name order; signals maintain indexes, not rules; order fuzzing preserves digests.

### A3.3 Time and events

64-bit whole-second clock; event key `(second, owner ID, owner sequence)`. Schedule only later keys; cross-owner effects wait ≥1 second. Cancellation checks live sequence slots; save live events only. Frontier is exclusive: earlier events complete, none at/after it.
Activities retain start/end/progress/position; interruption applies partial-work rules once. Generic machinery alone does not implement meals/body effects. One-worker order is reference. Parallel islands cannot access other owners; merge events/history by key, layers between windows, births canonically. Window/stop/pool/worker proofs remain. Crowd stays serial: island joining costs exceed cheap-event work; profile minds before parallelising.

### A3.4 The same bits everywhere (`RES-05`, `TIM-16`)

Working doubles use pinned correctly rounded `kd::num`; persist whole units, never floats/NaNs. Torus: exactly 200,000,000 × 100,000,000 cm, wrapping 64-bit differences and reversible half-circumference ties. Heights mm, angles 2^32 turn steps, amounts base units, exact probability thresholds. Conversions specify rounding and reject overflow/nonfinite values; every simulation thread resets/asserts numeric environment.
Ban platform maths, unordered decisions, mutable RNG streams, raw-memory hashing/serialization, ambiguous ties and effectful expression order. Digests walk fixed system/entity/field order, little-endian. Accepted M1 digests stay equal across compilers/architectures/workers/reopens; new systems add proofs.

### A3.5 Chance

SplitMix64 draws key world seed/system/entity/moment/purpose/index; stable name hashes separate purposes. New purposes cannot shift other sequences. Pinned draws commit world format.

### A3.6 Catalogues and tuning (`MAT-13`, `MAT-14`, `MAT-17`)

Complete TOML entries, no inheritance; stable source/name identity, sorted numbers only indexes. Exact unit strings, no floats: mg/mm/mm²/ml/mm/s/milli-degrees/seconds/ppm; m=metre. Retain real/game durations (`TIM-18`); “about” means ±10%. Schemas validate/fingerprint/display; unknown fields, ranges/units, duplicates and unresolved links fail at source.
Separate rules/world-making/look fingerprints; placement/size/species change world-making. Saves retain names/versions; new sources currently only add entries. Export manifests hash every file; phone verifies. One authoritative catalogue.
Camp-only packaging excludes unused art and developer pages. Approved sources remain; excluded host harnesses retain renderer regressions. Required cloud/provenance/current-save proofs, performance test and first-launch check remain; no full-renderer acceptance is inferred.

### A3.7 Saves (`TIM-05`, `TIM-08`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`)

Folder: metadata, two recent snapshots, journal, yearly history. Append/sync commands before action. One I/O thread owns writes; copy state between events, compress off-thread. Canonical versioned zstd chunks have whole-file checksums before unpacking; logs stop at damaged framing/checksums. Temporary write/sync/rename/directory-sync; write failure stops writes/world, never masks an unsaved gap.
Save every 30 real seconds and on background/switch/export/quit; background pause mark/snapshot within Android's 10 seconds. Recover newest valid snapshot, set damaged ones aside, deterministically replay to recorded moment. Permanent history survives thinning.
**No older-save conversion** (owner, 9 October 2026; `PLT-09` paused): reject earlier format plainly before parsing/decompression/replay/recovery writes. No upgrades, migration seals, old fixtures/corpus; delete broken obsolete tests. Chunk/component versions are strict; VERS1 reserved field is zero.
Current snapshot/archive/folder metadata format=6. CAMP4 u32 mask: craft=1, learning=2, fire=4, ideas=8. Craft requires CRFT2/KNOW2/HIST2 and LIFE2/DRMS3; learning/fire/ideas require their exact extensions (A10–A12). Refuse unsupported/duplicate/missing chunks, orphan links, invalid bounds/quantities/progress/events and overlapping reservations. Legacy proofs retain rules.
Warn below configured free space (currently 1 GB); delete nothing without user. Archives validate paths/checksums and remove refused partial imports. `ArchiveWriter::next` still synchronously reads/hashes a constituent file before chunk output; tiny exports do not prove smooth large exports. Large immutable copy/compression overlap remains unmeasured.

### A3.8 Talking to Godot

Durable plain commands; single-consumer triple-buffer snapshots. One controller owns a copied publication for views/jobs; never retain recycled slots or touch Godot from simulation workers. Camp sprites/cards sample saved activity ways at displayed time, including interruptions/torus seams, not arbitrary recent positions.
Native integer text, 48 dp dock, full-window 1×/2× raster and 320 ms camera settle retain anchored picking. Process each finger once, ignore emulated mouse; 550 ms hold opens ring, drag/pinch/cancel/pane change clears it. Test safe offsets and paired events.
Lossless events/commands are separate from disposable pictures. Blocking seeks/recovery set screen endpoint before advancing; ordinary lookahead remains .25 real seconds. Two-day regression bounds endpoint knowledge copies at 25 with exact digest.
Power preparation drains/pauses at frontier; revalidate selected memories/sites before durable send. Private ledger keeps request/receipt/sleep/attempt times; failed append cannot appear sent.

### A3.9 Threads, speed and budgets

Reread goals between batches. Steady-clock screen time never exceeds frontier; overload slows time without detail loss. Pause settles at frontier; ordinary lookahead ≤.25 real seconds. Workers: explicit 8 MiB stacks, unpinned after measured M1 comparison.
Heat guard: light threshold−0.05, ten-second forecast, missing readings never cool. Play/isolated measurement share governor/poll interval. Report frames, CPU/GPU, memory, clocks, heat/battery; viewport GPU costs need disabled-pass comparisons. Power-rail callbacks remain unimplemented; cloud software times are not phone evidence.

## A4. Drawing

Accepted foundation diagnostics and shared texture decoding remain; obsolete private 3D art routes are removed.
The stopped 2D work includes projection, terrain/light fixtures, streaming and UI, but has open faults listed in A18.2.

### A4.1 The picture (`PRE-01`, `PLT-02`)

Reference world: 540×1200 portrait, nearest 2× to 1080×2400; reversed landscape, native UI. Other sizes crop/reveal whole pixels with safe insets; only live pinch resamples. Resting anchors/raster offsets align and picking undoes them.

### A4.2 Pixels and animation (`PRE-22`, `PRE-44`)

One grid for terrain/objects/effects; project shared edges before rounding, never accumulate rounded pitches or squash projected art. Preserve attachment pivots. Walk follows distance, work Activity phase, idle separate look keys.

### A4.3 Light (`PRE-20`, `PRE-21`, `PRE-30`)

Aligned albedo/material/normal pages: `albedo × (sky × sky_visibility + sun × normal_response × sun_visibility) + fire + emission`. Contact affects indirect light only; overlapping shadows never multiply black. Explicit east/south/up normal basis; Compatibility needs sRGB decode/encode. Hour/weather fixtures are labelled; fast-time relief light is presentation.

### A4.4 Shadows, overlap and reveal (`PRE-21`, `PRE-24`, `PRE-28`, `PRE-30`)

Height-aware receivers separate roofs/floors; include off-screen casters/angle reach, no universal 14 m halo. Proxies: cylinder/cone/dome, five sun/24 sky rays; birch six sprays/2 m depth provisional. Contact .025–.12 m. Faded covering crowns/roofs and obscured silhouettes reveal selection without changing obstruction/shelter.

### A4.5 Water, fire and weather (`PRE-26`, `PRE-30`)

Water/bed/feet/clipping/picking share surface/footprint. Refraction reads bed, not foreground people; reflection/depth/shores incomplete. Fire base touches ground, emitter may rise; walls block light. GPU rain/smoke never changes rules.

### A4.6 The bridge, jobs and caches (`WLD-13`, `TIM-17`, `PLT-09`)

Immutable jobs carry world/data/look/renderer/format/epoch/revision/dependency identity; missed publications resynchronise. No live-state reads. Place/Activity is 2D; fixture up is cm, future physical heights mm: declare units/surfaces.
Keep coarse parents until replacements ready/uploaded; publish colour/material/normal atomically. Tickets charge old/new overlap until acknowledgement and detachment/disposal. Bound all queues/staging/caches, counting reductions/gutters/channels; eviction leaves saves/digests unchanged. Adapter: one decoder/upload channel per frame, retry/backoff, two-frame disposal; frame-headroom proof open.

### A4.7 Godot and phones (`PLT-04`)

Record renderer/driver; fallback is not Mobile evidence. Owner phone first; name/obtain second device before claiming evidence. Cheaper passes/same-grid 30 fps precede visible-change proposals. Report unavailable counters.

### A4.8 Tests (`PRE-31`, `RES-05`)

Cloud captures are regressions, not phone feel. Test normal/direct entry, both orientations, slow motion/pinch release. Picking undoes all transforms; CPU table maps GPU IDs, never 64-bit IDs through floats. Equal-second outcomes match across views/workers/reopen/eviction.

## A5. The look

### A5.1 The pictures

Owner targets in `art/targets/` define light/colour/density/composition, neither shipped nor traced; flagged anachronisms are unapproved.

### A5.2 Guidance about the feeling

Quiet pixels, muted varied greens, small strong accents, true hour/season light, dark nights/warm pools; readable motion/light/reveal. No heavy ordinary figure outlines. Archaeology/`SCP-20` constrain dress/objects/motifs.

### A5.3 Three art families (`PRE-22`)

Author 64/16/4 internal px/m; review 32/8/2 and farther reductions, doubled physically at 2×. Pages share alpha/trim/pivots/gutters; categorical IDs never interpolate, normals renormalise. Each authored family needs consecutive halvings (64→16→4 is not a mip chain). Encoded size is not GPU residency.

### A5.4 Art production (`PRE-20`, `PRE-42`, `PRE-46`)

Build needed pieces with signed sheets, sources/prompts/provenance/approvals; sheet/runtime/engine approval differs. Early stand-ins do not sign designs. Physical properties come from validated catalogues, never colour.
129/373 designs signed. Meadow/boulder/birch seasons/covered tent pass static review, not engine approval; hazel 8.1 signed. Person/deer critic sheets await owner, not walk loops. First/full-age people and tent interior/back/frame/floor missing.
Tent 16.4 cone: ring 4.2 m, cover 3.8×2.6 m, crossing 2.7 m, tips 3.1 m; ring/tips outside opaque shadow. Birch: 20 m height/7 m spread/.25 m shaft/.38 m collar, one trunk across seasons.
Ground 1.3/1.4/1.5 accepted exports: six packs, three variants/families, 1,458 aligned pages, 4/16/64 m spans; runtime batch review pending. Hearth 1.6/mud 1.7 WIP: inspect pack contracts, no full approval. Separate optional binary water; soil material 2/charcoal-wood 5. Wet≤1.25 m/drying≤1 m puddle fixes and >2 m joins need whole-chain review.

### A5.5 Targets and the loop

Compare changed views to approved targets; data-driven statistics warn, not approve. Visible-saving phone blind test: ten random pairs, eight correct means visible. Heat-driven visual steps must be planned/logged/accepted invisible.

### A5.6 Truth in pictures (`PRE-42`, `SCP-20`)

Check doubtful dress/species/things before runtime; no borrowed motifs, pre-copper metal or invented equipment. Rejected studies remain provenance, never approved sources.

## A6. The art kit and animation

### A6.1 The kit (`PRE-46`)

Records compose bodies/garments/tools/conditions; offline rigs optional. Full breadth deferred to PROJECT.

### A6.2 Appearance (`PRE-42`, `PRE-43`)

Look keys vary approved art; actual equipment/inputs/age/condition determine appearance. Shared pivots/facings/trim.

### A6.3 Movement (`PRE-27`, `PRE-44`)

Baseline four facings/six-frame walk/idle-carry-cut-tend poses/small clothing set. Compare eight facings on one animal first; full motion open.

### A6.4 Surfaces and picking (`PRE-24`, `PRE-33`)

Undo presentation transforms; pick surface order/bounds/alpha, not flat inverse slopes/roof floors. Deep caves use separate views; arbitrary stacked overhangs out of scope.

### A6.5 Sheets and review (`PRE-31`)

Review true/enlarged sheets, frames/masks/normals/running scene. Readable early stand-ins do not confer final approval.

## A7. The world

The following contracts are deferred to M7 unless needed earlier by a scoped camp behaviour. PROJECT.md owns all full-world numerical acceptance; the small valley closes none of it.

### A7.1 What this milestone supplies (`MIL-14`, `PRN-02`, `WLD-08`)

M7 supplies geography/weather/habitat; M8 full ecology, living settling, animals and biological starts. Checks await consumers. Reuse simulator; no planet formation/live tectonics.

### A7.2 Coordinates, cells and the polar barrier (`WLD-01`, `WLD-02`, `WLD-03`, `WLD-12`)

Exact torus: 1 km cells, 10 km weather, 250 m areas/251 shared metre-grid vertices. One polar-ice transport barrier covers diagonal/swept water/weather/lives. Globe distortion is presentation.

### A7.3 Ownership and stored fields (`WLD-12`, `PRN-14`)

Canonical regular arrays/sparse feature tables, no entity/allocation per cell. Separate immutable ground, physical stores, saved area deltas and disposable pictures. Biome potential is not population.

### A7.4 A reproducible generation job (`TIM-16`, `WLD-08`, `RES-05`)

Versioned inputs fix work counts/candidates, never wall time. Resume stage digests exactly; canonical chunks use existing maths/chance.

### A7.5 Plates and rock (`WLD-06`, `WLD-09`, `WLD-30`)

PROJECT owns causal geology; rocks/resources retain provenance. Colour is not a physical property.

### A7.6 Rough climate, erosion, basins and fixed waterways (`WLD-08`, `WLD-09`, `WLD-17`)

Separate bed, spill/routing and water levels; drainage filling preserves lakes. Shared reaches join across boundaries. Cave certificates use finished terrain/water, never candidate-rescue insertions.

### A7.7 Final climate, seas, soils and life potential (`WLD-16`, `WLD-26`, `WLD-27`, `WLD-09`)

Keep approved climate/short-year/species/soil rules; freeze external references before tuning. Habitat estimates cannot certify ecology.

### A7.8 Candidates and the start region (`WLD-10`, `WLD-24`)

Ranking never edits candidates. Distinguish root/candidate/index/making digest. Keep one full working candidate plus checked artifacts; cancellation preserves saved world. Explicit seeds face identical gates.

### A7.9 Settling and the beginning of history (`WLD-08`, `TIM-14`, `WLD-11`)

Settle exactly 600 days under play rules before bands/history; water-only is incomplete. Frontier starts zero: negative dates do not support negative events. Version prehistory offset without resetting hazards/demo proofs.

### A7.10 Sky and weather (`WLD-07`, `WLD-16`, `WLD-22`, `WLD-30`)

Universal eclipse quota removed (owner, 8 October); coherent sky/reviewed plausible distribution remains. Integrate hourly swept storms/polar barrier. Wind/travel is not seasonally compressed; natural-rate tests exclude forcing.

### A7.11 Water and soil in play (`WLD-17`, `WLD-26`, `WLD-27`)

Fixed cadences/stores/residuals/in-transit water and canonical reductions. Flood-hour promotion conserves water. “About a day” upstream: 21.6–26.4 hours. Cultivated fertility is field-local.

### A7.12 Quakes and eruptions (`WLD-15`, `WLD-22`)

Saved hazard warnings/cooldowns, game-time opportunities, never camera loads. Keep catch-up disturbances through history thinning; compact only equivalent state.

### A7.13 Detail on demand and kept areas (`WLD-12`, `WLD-13`, `PRE-03`)

Separate canonical facts/picture detail/saved changes; absolute samples/shared halos avoid seams/order effects. View catch-up copies/saves nothing; activation equals no viewing. Count depletion once; inactive usual weather need not equal active hourly weather.

### A7.14 Events, saves and revisions (`PLT-07`, `PLT-08`, `PLT-09`, `RES-05`)

Append layer owners preserving order; owner-count encoding changes require older-save refusal (A3.7). Old demos attach no systems. SYST holds payloads; rich disturbances need bounded versioned records beyond history's two integers. Publish screen-time immutable bases/revisions; caches never archive dependencies.

## A8. From a person to the globe

### A8.1 Ground, height and order (`PRE-03`, `PRE-23`, `PRE-24`)

Disposable render chunks differ from 250 m areas. Split trunk/crown, cliff cap/face, shelter layers; cross-chunk order uses ID ties, not plain Y-sort for roofs/shelves. Variants/stamps/ecology reflect records, never decorative resource guesses.

### A8.2 A nearby origin

Subtract exact nearby origin before floats; north becomes drawing south. Rebase every 4096 m, retaining absolute/raster origins; no rule/save dependency.

### A8.3 Zoom steps and forms (`PRE-03`, `PRE-28`)

WIP: 19 stops, 2^-12–64, 160 ms anchored logarithmic settle; owner dislikes snapping, phone acceptance open. Wide records retain IDs/sorted members/time/epoch/revision/truncation; limits 512 rows/8192 members, no wide-art proof.

### A8.4 One projection (`PRE-02`, `PRE-33`)

Fixed north-facing 37°; local east X/south Y/height Z/density s: `x=centre_x+sX`, `y=centre_y+s(sin(37°)Y−cos(37°)Z)`, subtract projected focus. Native projection/inverse/bounds shared with art; UI hits first.

### A8.5 World map and globe (`PRE-29`, `WLD-01`, `WLD-02`, `WLD-03`)

M7/M9 deferred: shared 2:1 map/globe-disc overview; repeat longitude, clamp latitude, never blend polar rows. Inverse taps return torus coordinates; no sphere routing/migration.

### A8.6 Time by zoom (`TIM-01`, `TIM-04`, `TIM-15`)

Pace precedence: pause/skip/manual-lock/director/zoom. Zoom requests speed only. Existing M2 power remains; director awaits consumer.

### A8.7 Budget

A18.1 owns graphics limits; measure busy camps, not six-object fixtures. Comparisons await consumers/devices.

### A8.8 Replacing the M2 fixtures (`PRE-03`, `PRE-29`, `PRE-30`)

Real camp providers precede M7 land; preserve regression fixtures/single consumer. Deferred detail ≤300 m, coarse to about 10 km, overview beyond; retain parents, no generation in gestures.

### A8.9 Cliffs, caves and the geological slice (`PRE-23`, `PRE-24`, `PRE-25`)

Deferred shared surface feeds feet/water/drawing/picking/shadows. Sections sample actual rock/soil/water/cave/dated facts with declared scales/width; synthetic burial cannot certify 200-year history.

## A9. Living things

M2 needs real scoped supplies and bounded renewal, consumed exactly once. Later species/biome/ecology obligations remain in PROJECT: M7 supplies physical habitat inputs; M8 completes living ecology and settling. Camera-near is never person-near.

## A10. People: bodies and lives

Current Life: food/water/rest with integer remainders; 4 kg berries/3 L daily, eight-hour sleep restores sixteen awake hours. Saved gather/carry/eat/drink/rest phases conserve partial work; cumulative intake/depletion settles before clamping. Need<20 interrupts, 36 awake hours forces rest. Body score tags 100–140 retain needs/benefits/costs/exclusions; universal kept reasons remain incomplete (A19.1).
Metre-grid/cm positions, saved rock and deterministic four-neighbour routes/sight. Choices use remembered supplies, refreshed on arrival; notice ≤3/hour, day 50 m/night 5 m, with source/time. Founders scatter by stable keys. Upstream/root-water/crop budgets are finite; absent stands never regrow. No midnight refill or unknown container. LIFE2/DRMS3 require matching people/activities/events. DRMS3 keeps typed ended-person identities and original founder facts per camp; pending place/idea targets must be live people or recorded former people of that camp. Retyped shared serials fail; prior DRMS2 is refused without conversion.
FIRE2/THER2 require all camp/person records, including zero-person camps; validated identities/mass/links/deadlines. Slots: 0 renewal, 2 ambient/fire, 3 food. Fuel→ash, water→vapour, quenched fuel→charcoal; reserved portions conserve mass, banking never extends indefinitely.
Heat2+ adds 15 °C within 2 m once across overlaps; settle warmth/water remainders before changes. Cooking exposure survives moves/splits: first-hour chance, failure stays raw, burnt after two hours or one at heat4. Physical change precedes noticing. Cold-hearth setup consumes 5 kg, grants no knowledge. M4 bodies/renewal: A19.4; full catalogues remain M8.

## A11. Minds

Choices use personally known facts at activity ends/interruptions. Current serial category precedence is incomplete common scoring; A19.1 replaces it. Minds retain evidence sources/dates, never player provenance.
Observation settles before changes/finish: torus range inclusive 500 cm, rock sight, 06:00–20:00 daylight or heat2+ light within 2 m of both. Weights deliberate4/busy1/asleep0; stable ties. Duration-normalised millionth-quarter credit: 20 quarters teaches skill1/source/route, first watch gives hunch. Saved work/try cursors prevent duplicates; cancelled strikes give none, interrupted gradual work persists.
Teachers use visible evidence/replies, learners their own state. Telling gives hunch; kindness≥60 T offers, comfortable learners reserve/walk, ≤30 minutes/daylight/sight/2 m. First supervised success teaches skill1; practice effort4×(1+teacher level/10), success2×failure. KNOW2/LEARN1 validate participants/work/events. Caps: 200 memories,5 hunches; expire hunch at10 failures/year unused. Practice T: levels1→5 180 hours,5→10 780; unused half-life5 years, floor half-best, retain fractional fading. Founder curiosity/kindness independently20–80.
Place dreams: three remembered sites, natural draw1/60/night, weakest need/newest/stable ties. Pull60, three days, no stacking; urgency/exhaustion wins. Private DRMS3 caps3 queued,1/person/night,3/night, reset06:00. Revalidate at sleep. Frontier confirmation, durable receipt≤1 game second; cancel restores pause, sleep interval preserved. Record actual later choice/pull/arrival, never infer causation.
Idea command3 contains person/memory; execution/sleep require handled input/action, experienced benefit and compatible unknown fit. Natural/sent share construction/nightly draw; refresh preserves failures. Only elapsed matching work counts; reopen preserves others' plans. Keep real-handling, no-send and missing-input proofs. Camp measurement remains ten minutes,200 s/rate,5 s warmup; cloud smoke is not phone evidence.

## A12. Crafts and discovery

M3 matches characteristics, not blueprint names, conserving material and personal discovery/learning. Craft/thermal food and fire maintenance selection use canonical physical/perceived characteristic fits, including renamed inputs. Keep `RCK`/`MAT-17` valid/invalid evidence, no scripted dates; remaining crafts/pace M8/M10, injury M4.
Discovery scene records finite additions and materialises aggregate stock once. Reservations/tools exclude competing work/meals. Physical predicates determine fits, personal evidence choices. Saved known tries/one roll per unknown activity; cancelled strikes yield none, gradual work resumes. Nutrition/wear retain remainders; inputs/leftovers/breakage conserve mass. History records actual maker/inputs/time/route; only noticed surprise teaches.
Copied item/work/knowledge changes sample screen time, hide unborn results, never read producer state. Craft remains serial under four workers because births share IDs; equal continuation is not parallel speed. Ordinary-seed First flake in own format proves no frequency.
Batch reservations; index active items by position/owner. Events publish `Context::item_changed`; raw edits invalidate, run/open rebuild. Sight groups keep first insertion/last physical evidence in ID order. Bounded mutex route cache keys geometry/endpoints. Stationary exposure batches exact daylight seconds; moving sight checks each second. Caches never enter saves/digests.

## A13. Culture

M5 proves knowledge/relationships between two camps; M8 finishes culture. Names use supported font letters. No scripted historical outcomes; retain actual causes and individual holders.

## A14. Story, the book of ages and the writer

HIST2 keeps immutable fire/warming/craft reasons, actor/second, winner/two supplied alternatives, scores/needs/effort/certainty. KNOW2/work/thermal/results link choices through interruption. Readers reject mismatched actors/forged IDs/current reasons. Body choices clear craft explanations; complete category comparison/score contributions remain A19.1 work.

M3 reads retained HIST2 events through factual templates and links actual actors, sources, inputs and results. Missing sources say “no source recorded”; spent inputs remain inspectable without being drawn as usable stock. Public History omits unnoticed uses and private player attribution; Your dreams owns the private ledger. Back restores the prior selection. M6 finishes the small book with pattern text. Director changes speed/moments only, never outcomes. Optional later writer only rewords verified records (`PRE-37`); no provider/API is selected here.

## A15. The interface

World first, truthful person cards, pause/speed, save/world controls and one indirect dream in M2. The camp menu gives direct access to Device check, Camp performance test and Test reports.
Use one gesture reader/theme, safe insets, ≥48 dp targets with 8 dp gaps and both orientations.
Current bitmap font has 135 glyphs (ASCII 95 included), base 16/line 20/baseline 15, integer physical sizing and retained licence/provenance. Arbitrary Unicode lacks coverage; the Japanese probe aliases a question mark. Production fallback remains unresolved.
Deferred layout batching avoids the previous quadratic catalogue freeze. M3’s normal person card has name/activity and a short reason; Details scrolls within 60% of portrait safe height. Controls stay at least 48 logical pixels, with no horizontal scroll. Rotation keeps selection and the active History/Dream sheet. Larger text and mute are presentation preferences outside camp storage; neither changes simulation state.

## A16. Sound

M3 uses three small original procedural stand-ins: tap, handling and a fire loop. Four work voices plus one fire voice follow sampled action phases, actual heat and world-space camera distance; pause/mute stop them. Audio never advances or chooses work. This fits the 32-voice bound. M9 completes sound obligations; listening review, full mix, language murmur and phone cost are unproved.

## A17. Testing and checks

Testing policy: owner OK 8 October 2026. Keep the routine/delivery/audit split; IMPLEMENTATION.md owns delivery instructions.
Routine tests cover native/view/app/tools, accepted deterministic proofs, catalogues and document structure and ID traceability checks. Native lint and source rules are in the routine gate. The milestone-end audit now adds only cross-compiler digests (including retained M3 scenes), TSan, shuffled ties and kill/scene/repeat recovery (owner, 10 October 2026). It skips emulated full suites, repeated routine checks, long render benchmarks, instruction scans and a second throwaway export. Exact binary/data/checker fingerprints retain passes; changes rerun affected checks.
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
| G7 phone | ≥3 minutes held load, unplugged; freeze 20 ordinary roots plus 5 fallback roots. Include settling for each. Area/detail/open target/outer limits: 0.1/0.11 s, 1/1.1 s, 3/3.3 s. Every minute of 20-minute route meets frame line; proposed TIM-07 gate ≥20 world-alone game years/minute on permitted cores; every digest matches cloud. Repeat as ecology/bands arrive. | Generation suite separately about 100–110 minutes plus setup; resumable |

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

Legacy renderer review does not certify the camp; the pre-review routine passed, while repair routine/phone evidence remain incomplete.

Wide data exist, but tiny figures/group/camp art, transitions, ecology details and named wide navigation are unfinished. `wide_study.gd` is only a study.
Explore can overflow landscape; the source viewer overlays the scene. Recent screen fixes need fresh captures. Owner reported tiny/busy UI, grass stripes, questionable birch/boulder/tent shadows and duplicated Camp title; no all-screen approval exists.
A correct light equation does not approve the camp composition. Latest software-renderer captures and native tests do not prove phone heat, speed or touch comfort.

### A18.3 Wider-world budgets (`WLD-11`, `WLD-12`, `PLT-04`)

Deferred M7 obligations: PROJECT's 180/198-second generation and 60/66-second settling lines include fallback search/entry saving; proposed world-speed budgets now follow TIM-07/PLT-04, replacing 0.2 core-seconds/day. Rule-area preparation ≤10% of simulation, about 0.1 s/area, visible detail within about 1 s.
The prior 45/90/35/10-second stage split and 128-byte/cell design were estimates, not measured allocations or requirements.
Keep the 2 GiB peak-process line; measure structs, temporary candidates, GPU residency and snapshot overlap together. Two million cells' daily work is a larger risk than their storage.

### A18.4 Wider-world risks

Freeze hard gates before tuning; report rare qualification instead of repairing candidates. Revalidate full-resolution starts after settling.
Storage measurements/owner disposition: [M3 result](IMPLEMENTATION.md#m3-result). Bounded live storage: A19.2 (PLT-10 amended 10 October 2026). Terrain expansion retains cross-chunk ordering/shadow tests.
Outstanding biology, ecology and historical-trace checks remain open until actual consumers prove them; a partial stage never certifies the full item.

## A19. M4 design (approved 10 October 2026, not built)

### A19.1 Choices and evidence

Build one bounded candidate set (≤30, ≤8 known blueprints); score in stable order before committing reservations. Keep winner, two best rejected candidates and three decisive score differences, including body/teaching choices. Inputs carry perceived value/certainty/source, never physical-cache truth. Validate pending dreams against live person or typed ended-person records; a globally allocated serial alone proves nothing. Chain observers retain causal fire/input identities, not just timestamps.

### A19.2 Work and storage bounds

Reuse World's next-event queue. Index active fuel/food/timers, nearby makers and reservations; publish invalidations on moves/splits/consumption/evidence/heat changes. Separate physical caches from person-evidence caches. Wake on next depletion/need/season/visibility boundary, not arbitrary seconds; exact integer interval integration must match the scalar reference, including rounding. Batch contiguous records in canonical order; no changed RNG opportunities.

Archive zero-mass spent items after active references expire; retain stable typed identity, provenance and needed inspection facts. Never discard positive mass/ash. Immutable history pages own pinned reasons and item records; live work/dreams/current views pin dependencies. Keep unlinked choices for two game days T, then discard; routine choice diagnostics must not accidentally pin everything. Public 25-year events and permanent firsts/family links remain. IDs are never reused; replace contiguous-choice indexing with stable lookup. Logical digest covers archival facts independent of page layout; both packed and reference paths apply identical retention, excluding expired diagnostics. Their continuations must match.

Copy only changed display data and bounded history pages; replace event×choice scans with indexed lookup. Snapshot/journal manifests reference immutable pages, written by the existing keeper with checksums/atomic publication; crashes cannot leave dangling references. Save adaptation, deadlines and remainders; rebuild disposable indexes. Bump format/refuse older saves. Instrument visits, wake-ups, CPU/year, page bytes/year and copy peaks; gates live in IMPLEMENTATION.

### A19.3 In-run motivation

Fixed law per build, per-person state: last update, eligible opportunities, observed successes/need relief, answered requests, filtered learning progress, pressure and integer remainders. Update at dawn from own evidence only; save/replay every update. No hidden holder count, recipe names, target year or cross-world state.

Initial T: seven-day observation window, motivation gain 1–2, maximum daily change 1/16; freeze before judging. Define pressure P initially zero in [0,1]; daily P'=clamp(P+(D−R)/16,0,1), gain=1+P. D/R are eligible-opportunity-normalised experienced deficits/relief, each in [0,1], separately for exploration and teaching; teaching deficits require actual requests/observed inability. Save window counters and division remainders. No opportunity means no accumulation; saturation prevents wind-up. Repeated failures without new evidence reduce that action's learning-progress value. Score effects remain below urgent survival/existing-plan protection; never change physical chance/yield or grant prerequisites. Details names observations/gain. Disabled-law runs are tests only.

### A19.4 Seasons, bodies and family

Camp season state saves phase, local weather schedule and finite patch water/biomass/fertility budgets. Renewal consumes declared external inputs and records transfers; harvest debits once. Food saves nutritional group, thermal/spoilage exposure and processing identity through splits/moves. People's seasonal memory saves observed shortage/time/location, never global forecasts.

Add versioned condition/water/blood/part wounds, pain, exposure and illness deadlines; hazards keyed once per eligible interval with saved survival/remainders. Care is timed work with finite inputs and observations/replies; death atomically cancels work/lessons/dreams/reservations and records causes. Pregnancy/nursing/age use absolute dates and staged events; parents/traits/feeding links validate across deaths. Children learn through existing perception/teaching. Current-format open and interruption must reproduce the same next event; no statistical replacement people in M4.
