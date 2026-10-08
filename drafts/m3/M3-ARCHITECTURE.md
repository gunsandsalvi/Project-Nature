# Kindling: M3 architecture draft

Draft for the owner's M2 closing review, 8 October 2026.
This is proposed replacement text for A7 and proposed additions to A8, A17 and A18, serving `MIL-10` and `T2.12a.4`.
Q1–Q10 incorporate the owner's answers of 8 October 2026; other implementation choices remain proposals.
It changes no repository document and claims no implementation acceptance.
A3's accepted arithmetic, chance, events, catalogues, workers and saves remain the foundation.
The questions named here are in `QUESTIONS.md`; the evidence and alternatives are in `M3-RESEARCH.md`.

## A7. The world

### A7.1 What this milestone supplies (`MIL-10`, `PRN-02`, `WLD-08`)

The world is a small, wrapped map with believable present-day geography.
Generation approximates the effects of deep time; it does not simulate planet formation.
Plates explain ranges and rock, rock and runoff explain valleys, and climate and ground explain soils, deposits and vegetation.
Each intermediate field has a named consumer or a test; unused scientific detail is left out.

M3 supplies generated land, natural shelters, drainage, climate, sky, weather, water, soils, deposits, biome and initial-cover records, and natural quake and eruption events.
M2's renderer draws these records in place of its labelled fixture geography.
Living growth, grazing, predation, wildfire and harvested resources are M4; people and their survival are M5; player intervention is M9.
The functions these later milestones need are designed here and exercised with explicit test inputs, never disguised as complete play behaviour.

**Approved phased acceptance (Q1, 8 October 2026):** full `WLD-08` settling needs M4's ecology, `WLD-24` needs its food estimates confirmed and M5's survival, `WLD-11` names bands made in M5, and `PRE-25`'s old camp needs M4's traces.
The owner accepted phased closure: M3 proves the physical world; M4 completes living settling, renewal and traces; M5 completes bands and winter survival.
Every outstanding check stays mapped and open until its assigned milestone proves it; the pass rules are not weakened.

### A7.2 Coordinates, cells and the polar barrier (`WLD-01`, `WLD-02`, `WLD-03`, `WLD-12`)

- Keep the exact 200,000,000 by 100,000,000 centimetre torus.
  World cells are exactly 1,000 m, 2,000 by 1,000; weather cells exactly 10,000 m, 200 by 100.
  Cell IDs are row-major indices of canonical wrapped coordinates, independent of allocation or thread order.
- **Approved area resolution (Q2):** 250 m, four by four to a world cell, with 251 by 251 shared metre-grid vertices when the picture needs them.
  This is within 2.35% of `WLD-12`'s about 256 m and preserves sixteen areas per cell and the accepted torus size.
  The owner approved it on 8 October 2026; never silently change the torus to fit a power of two.
- Latitude is −90° at north coordinate 0, 0° at 500 km, and approaches +90° at 1,000 km.
  Longitude wraps east-west.
  The permanent ice strip occupies the last 100 km on each side of the north-south seam, totalling 200 km.
- **Coordinate wrapping is not permission to cross.**
  Static terrain and rock sampling can use periodic coordinates and shared seam samples.
  A common transport-edge policy forbids crossing the middle of the ice cap for water, storms, moisture, wind transport, animals and later walkers.
  Diagonal edges and long steps test the segment, not just the endpoints.
  Path costs and reachability use this policy; torus shortest distance alone is no route around the barrier.
- Ocean outlets seed drainage; the rectangular map edge is never an artificial outlet.
  Each side of the ice seam has its own no-flux transport boundary.
  Nothing is teleported to the opposite pole.
  Stationary polar cells may have cold, snow and sky state without transporting anything across the seam.
- Globe compression is only A8.5's picture.
  All cells have the same model area at every latitude, and physical routes never use great-circle distance.

### A7.3 Ownership and stored fields (`WLD-12`, `PRN-14`)

Large regular fields use compact arrays outside the entity registries.
Each record has units, bounds and a canonical field-wise encoding.
Do not allocate an entity, heap object, map or vector for every world cell.
Sparse features use flat tables and offset/count spans.
The table below describes proposed records, not existing M1 types.

| Record | What it owns | Changes in play |
|---|---|---|
| World-making header | Seed, making version/digest, dimensions, tilt, land target, calendar hemisphere, candidate identity and start-region certificate | No |
| Base ground | Height in mm, plate/province, rock stack and thicknesses, slope class, fixed glacier/ice mask, coast and basin references | Heights and river courses fixed; explicit allowed damage is separate |
| Geological feature tables | Fault lines, volcano sites/types, cave anchors and dimensions, lithology contacts, resource bodies and provenance | Hazard state and exposure may change; no new obsidian from eruptions |
| Drainage | Receiver/rank, contributing area, basin ID, fixed river line, cross-sections, lake storage curves, springs, floodplain elevations and shore class | Geometry fixed; water state changes |
| Climate | Four seasonal means, extremes, precipitation, snow, winds, storm days and longest dry spell; terrain/sea adjustments | Never changes |
| Cell state | Soil stores/fertility, groundwater, snow/ice, water status, cover and disturbance state; later fire, paths and herds | At the fixed paces below |
| Weather state | Temperature, humidity, wind, cloud, rain/snow, active storm references and wet/dry-year anomaly state | Hourly |
| Area delta | Changed things/marks, local depletion, dated layers, last authoritative update, event cursor | Only actual changes; never a camera cache |
| View products | Coarse ground, metre relief, render pieces, overview pyramids and masks | Disposable; not simulation state |

Climate may be stored as compact per-cell seasonal values or shared profiles plus exact modifiers if profiling proves identical answers.
An index optimization cannot replace a place's actual climate with its biome's average.
Stored cover includes species identities and shares when the relevant approved entries exist; habitat potential is not an animal population.
Any initialization subset is named in the world header and never presented as the completed launch ecology.

Every physical record can be inspected on the developer land page or its world overlay (`PRN-04`).
Rock and deposit inspections explain what is there and its formation context, not what an unknowing person knows how to use.
The full player cards remain M4/M9.

### A7.4 A reproducible generation job (`TIM-16`, `WLD-08`, `RES-05`)

Generation is a C++ job over immutable input catalogues, with no Godot dependency.
Its stages have fixed work counts from the versioned configuration; wall time measures them but never decides how many candidates or iterations to run.
Each completed stage publishes a field digest, timings, memory peak and diagnostics.
Generation checkpoints are build artifacts, not a replacement for play saves.
They contain seed, configuration digest, candidate index, stage and canonical fields; resuming must match an uninterrupted job.

Use existing `kd::num` operations and checked conversions.
Working doubles are permitted under A3.4; stored state remains whole base units.
Use the existing `kd::chance::Draws`, with namespaced system/purpose, stable cell/feature identity, a defined generation moment, and explicit draw index.
Do not introduce a new random-number library or a mutable global stream.
Hash-based feature keys are distinct from never-reused entity IDs; when a real entity is created, allocate its entity ID canonically.

Workers write disjoint fixed chunks; results gather in chunk/cell/feature order.
Sorting and heaps have total keys, such as (filled height, cell ID).
A sum of runoff or sediment contributions has defined order and checked width.
The one-worker answer is the reference.
Generation kernels can use `Workers::for_each`; they do not need interacting-entity islands.

### A7.5 Plates and rock (`WLD-06`, `WLD-09`, `WLD-30`)

1. Draw tilt within 15–30°, target land fraction within 25–50%, and 6–12 plate seeds from separate keyed purposes.
2. Partition a periodic coarse domain into plates, with bounded boundary perturbations; give each continental/oceanic character, age/province and a velocity vector.
3. At each shared boundary, relative normal and tangential motion classify collision, subduction, rift or transform.
   Continental collision builds broad ranges; subduction builds a trench and an offset volcanic range on the overriding side; extension builds rifts; transform records a fault with modest relief.
4. Build crustal height, continental shelves and broad old-land relief from these features.
   Choose one sea datum as part of generation to meet the seed's land target; then freeze it.
   Erosion may move shore outlines, so final acceptance checks the final share.
   Range widths are tens of kilometres, peaks normally about 3,000–4,500 m, and most land under 1,000 m, as `WLD-30` requires.
5. Assign about twelve named rock kinds from geological province, with surface rock and at most two underlying layers.
   Keep an ancient basin/sea provenance mask: present-day sea coverage cannot explain an inland chalk outcrop by itself.
   Sedimentary layers fold/tilt in ranges; metamorphic rock follows the appropriate province; volcanic rock follows recorded volcanic type and age.

This is a bounded causal construction, not plate motion integrated over millions of years.
Noise supplies small irregularities within features; noise alone does not choose ore, faults or rivers.
A three-layer stack is a schematic section, not an exact geologic reconstruction.
Regional contact functions interpolate shared boundaries so a cliff and its neighbouring slice show the same layers.
The catalogue lists erosion resistance and porosity separately; visual colour never supplies a physical property.

### A7.6 Rough climate, erosion, basins and fixed waterways (`WLD-08`, `WLD-09`, `WLD-17`)

A first climate estimates runoff from latitude, height, sea distance and broad winds.
It precedes erosion; the final climate follows the finished relief.
Use a fixed small number of terrain/drainage passes, benchmarked before increasing them.
No stage runs until the picture merely looks finished.

**Drainage construction:**

- Preserve the original height field.
  Priority-Flood computes spill elevations from sea cells over allowed neighbours and builds a depression hierarchy.
  Its filled routing surface is separate from physical bed height.
  Otherwise filling every depression would erase the lakes the game promises.
- Receivers have a strictly decreasing routing key: spill level first, then a deterministic drainage rank through flats.
  This proves acyclicity even when physical elevations tie.
  The rank is routing metadata, never a visible artificial slope.
- Accumulate contributing area/runoff in topological order.
  Endorheic basins terminate in a registered lake or seasonal basin with a storage/outlet record, never an unexplained dead river.
  Keep physical bed, water level and spill height distinct.
- Cut valleys using a bounded stream-power approximation, sensitive to drainage area, slope and rock resistance.
  An implicit downstream-to-upstream solve with exponent one is the first candidate; more expensive exponents need evidence that they improve the visible land.
  Stable ties and quantization are explicit.
  A final routing pass follows the last bed change.
- Add floodplain, fan, delta and gravel/silt masks from valley gradient, upstream supply and receiving water.
  Track source-rock provenance and downstream travel distance to place transported stones truthfully.
  A versioned lithology/distance rule gives their rounding class; matched stones become no less rounded along longer transport paths. This affects appearance/material form, without a live abrasion simulation.
  This is a generation approximation; no ongoing sediment-transport solver enters play (`SCP-21`).
- Freeze a world-scale river graph and shared reach centerlines, junctions, widths, depths and monotone bed profiles.
  Deterministic bends fit their valley corridor; they cannot cross a divide or make a river climb.
  A reach owns its geometry across all cell/area/render boundaries.
  Stream confluences and lake mouths use shared anchors, not separately random endpoints.

Lake records hold a monotone level-to-storage curve and fixed outlets.
A lake can span cells; it is one body with one surface level, not many independent puddles.
Generation fixes glaciers and glacial landform masks in cold high country; there is no ice-age or moving-glacier simulation.

Caves follow soluble limestone, lava tubes, or soft layers beneath hard caps.
After final erosion, each cell records anchor, entrance, floor/roof bounds, usable floor area and type before any area is made.
Rock-stage cave envelopes are provisional; final entrances must intersect the finished relief, and dryness is certified against the final climate/water regime and rechecked after settling.
Area detail expands that certificate rather than rolling again for the existence or size of a cave.
Reject a start with unusable shelters; never carve a cave to rescue a candidate after scoring.

### A7.7 Final climate, seas, soils and life potential (`WLD-16`, `WLD-26`, `WLD-27`, `WLD-09`)

**Climate:** compute seasonal temperature from latitude, tilt, height (starting lapse about 6 °C/km), continentality and fixed ocean warmth.
A low-resolution basin-aware ocean pass sets warm/cold boundary currents and upwelling before the final coastal-temperature pass.
Use prevailing wind belts and a bounded moisture sweep for rain shadows.
A periodic longitude solve uses a fixed iteration count or fixed residual criterion with a hard deterministic bound, not a sweep whose seam becomes the source of all moisture.
No moisture crosses the polar barrier.
The Smith–Barstad linear orographic model is a reference and possible later refinement; the first implementation need not include its spectral machinery.

Store each season's normal warmth, extremes, rain/snow, wind, storm days and longest dry spell.
An extreme is a climate envelope from which weather and later powers draw; it is not a guarantee that each season attains it.
Four seasons are game time; monthly Earth data are grouped by annual phase for validation.
Q3 (8 October 2026) fixes the comparison method: 10-degree latitude bands, at most 10 percentage points of climate-class share difference per sufficiently sampled band, about 2°C matched-bin warmth, an Earth-tilt case and separate 15°/30° cases.
Freeze matched latitude/altitude/maritime/current-exposure references and scaled distances before tuning. The separate every-place 20-year weather bounds remain ±10% rain and ±1°C against its own climate.
Q9 fixes daily precipitation units and Earth-equivalent annual indices for biome classification; saved seasonal totals cover 15 game days.
Do not compare the equal-area torus's raw latitude histogram with the unequal-area cells of an Earth raster.

**Seas:** use one fixed sea level and no tides.
Store depth, coastal class, current direction/thermal influence and upwelling; sea warmth updates every five days, while freshwater and sea ice update daily under `WLD-12`.
Fish, sea mammals and shellfish population dynamics remain M4.
Their initial habitat/capacity records follow shallow water and upwelling, not decorative random shoals.

**Soils:** use parent material, sediment provenance, slope, climate and potential vegetation class.
Compute a preliminary vegetation potential from climate first, use it to inform soil, then perform one fixed final biome/cover pass.
This resolves the soil/plants dependency without an unbounded equilibrium loop or changing `WLD-09`'s final stage order.
Store soil kind, effective thickness, fertility 0–5, infiltration/retention and diggability.
Silt, loess, ash and old grassland favour fertility; sand, steep slopes, peat and leached hot/wet soils limit it.
No element or chemical balance is simulated.
Later crop, ash, dung and waste inputs use the same soil mutation interface.

**Deposits:** formation predicates are catalogue data over geological and drainage facts.
Under answered Q10, flint requires chalk and chert eligible limestone, or gravels traced below their respective sources; both count as the flaking-stone family; obsidian requires a young silicic volcanic source; copper ore requires a weathered copper-bearing province near granite in volcanic ranges, with native copper only there.
Clay follows old bends, lake sediment and weathered rock; ochre follows iron-rich weathering.
Hammer/grinding stone patches follow exposed quartzite, basalt or sandstone and source-derived gravel, with the same stable patch identity as other useful stone.
A deposit keeps source/provenance, abundance, depth and exposure; an exposed bank or cave wall samples the same body seen in a slice.
Taking material later removes it from a stable patch/deposit record, never rerolls a fresh supply.

**Biomes and initial cover:** classify ice, tundra, conifer forest, broadleaf forest, grassland, scrub, desert, savanna, tropical forest, marsh and high mountains, with shores/seas separate.
Climate envelopes select eligible catalogue species, then soil/wetness and cover competition select shares.
Initial disturbance ages set coherent regrowth patches, not independent noise per plant.
Generation places coarse animal counts and seasonal-range descriptors only for approved catalogue entries; M4 supplies their live rules.
The 6-plant/4-animal biome coverage and full food-web checks remain M4 acceptance.
Q4 (8 October 2026) includes one woolly mammoth species in cold open grassland within the roughly 30 wild-species budget. M4 supplies its reviewed catalogue entry, food demand and density; art follows its normal review.

### A7.8 Candidates and the start region (`WLD-10`, `WLD-24`)

- A root seed derives an ordered candidate stream by the accepted keyed chance API.
  Start with 20 coarse candidates at a proposed 4 km spacing (500 by 250 cells), retaining plates, province fields and coarse drainage/climate/deposit/biome measures.
  These are approximations of each candidate's world, not separate unrelated seeds.
- Rank by versioned integer scores for land/climate variety, barriers, resource unevenness and start quality.
  Start with equal category weights; normalize each category before weighting and publish every component on the review page.
  Weights and normalization ranges must be frozen before closing seed tests.
- Refine the best six first to the 1 km world grid, in rank order.
  Refinement preserves plate identities and broad provinces but recomputes drainage and climate at full resolution; coarse scores are not final certificates.
  If fewer than three qualify, refine the remaining first twenty in their coarse rank order, one at a time until three qualify.
  If still short, make the next twenty coarse candidates, rank that batch, and refine in that order until three qualify or all forty have been tested.
  This continuation policy is fixed in the making version, not selected by the phone's speed; no further candidates are refined once the stated stopping condition is met.
- Every offered candidate passes the **full-resolution** hard gates.
  Score a start from fixed climate, soil, caves, water and biome food estimates, with several food kinds and a seasonal margin.
  Find 3–4 shelter catchments for 15–30 people each, without double-counting the food in overlapping 10 km ranges.
  Each has a dry, large-enough cave/overhang, year-round water within 2 km and flaking stone within 10 km.
  Approved Q5 food gate: each season supplies at least 120% of starting-kit demand; shelters provide at least 2 m² usable dry floor per person as a tunable estimate.
  The exact starting-kit estimator and body-demand envelope must be confirmed by M4/M5 under Q1 before those biological checks pass.
- Coldest-season mean must be 2–10 °C with a few frost nights; `BIO-11` survival remains a real scene test in M5.
  A mean alone cannot prove a naked band survives a particular winter.
- Connected landmass membership uses the polar transport boundary and fixed sea/lake masks.
  The starting landmass must contain flaking stone, clay, wild grain, wolves, at least one eligible domestic ancestor and copper ore.
  An actual species occurrence, not just a colour labelled habitat, must eventually certify the species clauses.
- Rank fully qualified worlds again, tie by candidate index, and show the best three fully qualified of those evaluated.
  Never edit, add a resource or move a shelter after scoring.
  At the cap, offer the one or two that qualify, or say none qualifies; do not loop forever.
- Keep root seed, selected candidate seed/index and making digest distinct in UI/save metadata.
  “Let the game pick” chooses the top-ranked offered world; explicit world seed generates one world, passes the same final tests, and either gives its start or explains that it has none.
  Root-seed replay reproduces the same ordered choices under the same making version.

Only the selected candidate becomes the saved play world.
Retain coarse metadata, previews and at most one full candidate working set at a time; store retained candidates as checked compressed temporary artifacts.
Do not keep six complete mutable worlds and copies in memory.
Cancellation leaves the current saved world untouched and resumes or discards only the generation job.

### A7.9 Settling and the beginning of history (`WLD-08`, `TIM-14`, `WLD-11`)

The completed game runs the selected world's water, cover and herds for exactly ten 60-day years using play rules, without people.
This is 600 days, not a short special convergence loop.
The scored start region stays fixed; after settling it must still qualify.
Initial herd wariness around it is the hunted level; species-specific behaviour is M4's responsibility.
Bands are made only after settling, in M5.

Use an explicit prehistory origin in the new generated-world wrapper, with deterministic event keys across 600 days; reset neither hazard cooldowns nor water stores at history start.
The date mapping must present Year 1, spring, day 1 in the chosen start hemisphere while keeping the internal event order and chance moments intact.
The existing calendar supports negative dates, but `World` begins at frontier zero today: merely scheduling negative events into it is invalid.
Implement a versioned world-origin facility or an explicit elapsed-prehistory offset in the new wrapper, with tests of saves and event scheduling; do not patch demo golden results.
The final choice is an implementation detail recorded with α3.9b, not an assertion that M1 already does it.

Before M4, show “water settling; ecology pending” in the developer build.
Do not show fake herds or call a water-only run full `WLD-08` completion.
The production New World path and its report disclose the outstanding phase according to the owner's Q1 answer.

### A7.10 Sky and weather (`WLD-07`, `WLD-16`, `WLD-22`, `WLD-30`)

**Sky:** compute solar direction from longitude, latitude, annual phase and tilt, using A3.4's turn-angle functions.
Use a circular seasonal orbit with the chosen hemisphere in spring at history start.
Local solar hour depends on longitude; the UI retains one calendar date.
Polar day/night has explicit limiting branches, avoiding tangent singularities.
Full moon repeats every 15 days.
A seed-derived star layout and a simple inclined lunar orbit with node precession supply moon position and eclipses.
Eclipses are geometric alignments, never independent chance flashes.
Enumerate alignments over 70 years, then certify local visibility-count bounds over the complete torus, including horizon and footprint boundaries; a 100-place sample is only an early diagnostic.
Use canonical 1 km cell locations for the exhaustive report and conservative bounds/refinement inside cells to catch subcell visibility regions; an uncertified region is a failure of the proof, not assumed coverage.
Q6 (8 October 2026) requires 3–8 visible solar or lunar eclipses at every place in 70 game years, clouds or not; total solar eclipses are not promised everywhere.
Use the geometric horizon for this count, reporting obscuration separately. An exactly repeating 15-day/60-day orbit without node precession would produce an implausible repeating pattern.
Compatibility check: the phase period does not fix the alignment rate; the proposed orbit leaves inclination, node precession and apparent sizes adjustable, so this answer forces no structural model change. No parameter set has yet been fitted or certified to meet 3–8 everywhere. G5 must prove it before acceptance; a failed fit is reported without changing the target.

**Weather:** one layer event each game hour, with weather-cell state and explicit moving storm records.
Storm birth is keyed by cell, hour and purpose, conditional on climate; it is not a preauthored event date list.
Intensity, footprint and lifetime come from climate tables; thunder needs warm moist air.
Temperature combines seasonal/daily cycles, weather anomaly, coast damping and terrain correction.
Fog, local wind shelter and frost hollows are deterministic place queries over that weather, not per-frame simulation.

Winds keep real speeds; a 50 km storm can travel across several 10 km cells in one hour.
Integrate swept footprints or use a fixed bounded transport substep within the hourly update; destination-only movement would skip rainfall and jump the polar barrier.
The substep rule is a versioned constant, never chosen from frame rate.
Distribute rainfall by integrated coverage and normalize the catalogue's storm frequency/intensity to the climate's long-run amount.
Do not force every year to the same total, clip the last storm to make a statistical test pass, or let stochastic errors accumulate without a budget test.
Wet/dry/warm/cold-year anomalies use bounded persistent stochastic state.
A separate natural-rate suite checks storms/storm days, lightning, droughts, floods and harsh winters by climate against frozen Earth reference definitions and per-game-year targets.
It measures dry-spell duration as well as total rainfall; equal yearly rain cannot excuse daily drizzle or missing droughts.
Answered Q3 defines a dry day as below 1 mm rain and the saved longest-dry-spell envelope as the 95th percentile of annual maxima; test the exceedance share under RES-13.
Every event has an originating weather/water record and explicit exposure denominator; forced diagnostics never enter natural-rate counts.
Wildfire occurrence and lightning-to-fire conversion are added by M4 under Q1.

Daily weather rates keep real-day behaviour; rare event totals are calibrated per 60-day game year (`TIM-18`, `WLD-30`).
No blanket multiplication by six is applied to wind, travel speed or every rainfall value.
Snowfall accumulates as water-equivalent snow; daily temperature-driven melt supplies hydrology.
Natural lightning records location, time and cause; M4 consumes it for fire, M5 for bodies, and M9 routes allowed interventions through the same event path.
Nothing in an event delivered to a mind labels it as the player's act.

### A7.11 Water and soil in play (`WLD-17`, `WLD-26`, `WLD-27`)

Normal water, groundwater, snow, freshwater ice and sea ice update daily; active floods update hourly.
Sea warmth/ecological state and soil fertility update every five days; the sea record does not move ice off its daily schedule.
The cadence is identical for watched and unwatched places and fixed across a version.
Within a layer event, fixed chunks read old state and write new state; cross-cell contributions reduce in canonical order.

- Rain and melt split into infiltration and runoff by soil/cover and ground saturation.
  Groundwater storage has a slow recession; springs discharge where geology and elevation permit.
  Store rounding residuals where repeated integer fluxes otherwise lose water.
- Rivers route discharge over fixed reaches using a travel-time queue or equivalent bounded reservoir delays.
  Arrival times follow reach length and water speed in seconds, not six-times seasonal drying.
  A storm in the declared upstream test valley peaks downstream 21.6–26.4 hours later (the existing 10% interpretation of “about a day”).
  The normal daily step must retain timed in-transit water so flood-hour promotion neither duplicates nor loses it.
- Lakes integrate inflow, outflow and evaporation against their storage curves.
  Flood stage intersects precomputed connected floodplain elevations; no shallow-water PDE or moving riverbed is required.
  A flood lays a dated silt event; slow sediment transport is absent.
- Dry-spell recession and plant stress use the compressed seasonal response of `TIM-18`.
  Separate that coefficient from the real-speed reach travel and event clock.
- Snow melts into actual stores; freshwater/sea ice thickness determines later walkability, while glaciers stay fixed.
  A drawn sheet of ice cannot certify it bears a person.
- Water-quality records mark warm small still pools and about 1 km downstream of camps, crossings and carcasses, expiring a few days after removal.
  Transport follows the river graph, not a Euclidean radius.
  M3 tests the propagation/expiry API; gut sickness needs M5's body/illness consumer.
- Fertility changes from removal, rest, ash, dung, rotted waste and silt are one rule with explicit inputs.
  A cultivated field keeps its own fertility in the authoritative area delta; the cell supplies baseline soil and uncultivated fertility. Changing one field must not fertilize every field in its cell.
  M3 tests those transitions with ledger events; farming yields remain the later farming check.

These are game water stores, not a full energy or chemical conservation simulation (`SCP-21`).
Tests still require no negative stores and no unexplained duplication in the modeled inflow/outflow ledger.
Weather, stores and hazards all save their residuals, future arrivals and next update seconds.

### A7.12 Quakes and eruptions (`WLD-15`, `WLD-22`)

Fault and volcano type set annual chance, strength envelope and quiet interval.
Implement a conditional hazard per fixed game-time opportunity, converted from the annual probability, with keyed draws and a saved quiet-until time.
Do not draw a chance every render frame or every area load.
Natural opportunities continue without people or a camera.

Quakes emit an intensity footprint tied to the recorded fault, plus rockfall/cave-roof damage intents where susceptible geometry exists.
Eruptions have a saved warning state lasting days: small tremors, warm springs and later sound cues.
Lava follows downhill eligible surface corridors and emits burn/burial changes; ash uses wind, falls onto cells, fouls water for a season and later enriches soil.
Heights stay unchanged and no new obsidian is created, exactly as `WLD-15` states.
Do not add a runtime crater, tectonic uplift, new river course or fluid lava solver.
M4 handles things/plants/fire affected by those intents; M5 handles bodily harm.

Keep a dated, keyed disturbance ledger indexed by affected cell/area for later catch-up.
An old area still needs a quake/flood/ash record even if the public history has thinned it; authoritative catch-up input is not deleted by book-of-ages thinning.
Compaction requires a proved equivalent accumulated state/checkpoint, never a dropped event.

### A7.13 Detail on demand and kept areas (`WLD-12`, `WLD-13`, `PRE-03`)

There are three different products:

1. **Canonical area facts:** caves, material/deposit patches and stable object-placement keys, from the world-making seed, the cell and its shared neighbours.
   A rule may request these where a person acts; no metre height mesh is needed.
2. **Pure picture detail:** metre-scale heights, crack/scree arrangement and projected pieces, evaluated from the same geological and water constraints with look-specific variation.
   It never writes to the world, allocates entity IDs or advances timers.
3. **Authoritative area deltas:** changed things and marks, depleted resources, dated layers and local plants, with last processed day and event cursor.
   These are saved, independent of any render tile.

Stable placement uses world-coordinate microcells and a fixed candidate count per microcell.
Spacing checks read a fixed neighbour halo and resolve competing candidates by priority then coordinate, so traversal and cache order cannot decide which tree or stone survives.
Changing density/size/placement values is a world-making change, not a cosmetic update.
A picture's flowers and stones refer to real patch kinds/amounts; only uncountable texture variation is purely decorative.

Terrain is a constrained hierarchy: a world field, shared coarse area samples tens of metres apart, then metre relief that vanishes at protected anchors and obeys rock/cliff/river/cave envelopes.
Sample absolute coordinates, not tile-local seeds.
Adjacent tiles request identical border and derivative samples, with sufficient halo for filtering and normals.
Do not generate local rivers or local erosion independently: they would disagree at edges.
The coarse parent and fine child share low-frequency shape; fine detail cannot move a coastline or block the authoritative channel.

For a kept area, use one deterministic daily catch-up function over its last committed state, its cell disturbance log and the season's usual weather, with draws keyed to the day.
This is the stated `WLD-12` model for inactive areas, not replay of the entire past world.
Only people within about 1 km activate normal rule updates.
A camera request catches up a **copy** for display and saves nothing; the authoritative area remains at its last committed day until simulation needs it.
Repeated display, eviction and future activation must produce the same result as no display.
An active versus inactive area follows the project's intended near-person distinction; tests must not silently demand actual-hour weather equal seasonal usual weather.

Keep cell cover/depletion bookkeeping exactly once when an action happens.
Later catch-up changes the delta, not the original global removal again.
An area becomes unchanged only when every persisted effect is gone, including a buried item or a disturbance dependency.
Rules operate on a typed ground/surface query, never on GPU relief.
Actual multi-segment walking and bodily cliff risk remain M5; the new surface API must not change M1 demonstration activities.

### A7.14 Events, saves and revisions (`PLT-07`, `PLT-08`, `PLT-09`, `RES-05`)

Add layer systems through `World::set_layer`, with canonical `digest`, `save`, `load` and `opened` implementations.
The existing reserved owners are daylight and commands; they are not already a weather scheduler.
Preserve their IDs and append new owner IDs, including a documented ordering for weather, flood water, normal water, sea/soil and hazards.
A command currently precedes appended layers within a second; later powers must explicitly define whether they use pre-update conditions and schedule effects after the current key.
Never insert an earlier same-second event.

World's owner schedules and their serialization currently depend on `owners::count`.
Extend the encoding with a version and migration that reads old owner counts and initializes new inactive layers safely.
Old demo worlds attach no new systems and keep their original digests byte for byte.
Add new generated-world proof suites rather than replacing accepted golden answers.

Keep immutable generated ground in a critical checked snapshot chunk initially, with mutable system state in explicitly versioned system payloads inside the existing SYST chunk.
M1 does not yet provide separately versioned outer chunks for each system; add bounded payload schemas and migrations deliberately.
This works with the current archive format without an invented external asset dependency.
Serialize unchanged ground bytes once into a shared immutable blob per world; reuse them while encoding/compressing snapshots off-thread where the existing APIs permit.
Measure snapshot copy and duplicated compressed/uncompressed buffers; if incremental immutable chunk storage is needed, extend the keeper/archive atomically with crash tests before claiming the budget.
Do not assume the current whole-snapshot writer already streams huge terrain files without extra memory.

History records currently carry a small type and two integer payloads.
Rich disturbances need versioned system payload storage referenced by stable event ID, or a deliberate record-format extension with migration.
A pointer or renderer event queue is not history.
World export contains every authoritative base/delta/log piece; losing a cache is harmless.

Published drawing snapshots carry world epoch, frontier/display interval, an immutable base handle and revision manifests for mutable tiles.
One controller owns the triple-buffer copy; workers keep immutable handles, not recycled slot references.
A skipped revision requests a current complete tile from an immutable publication; it must not read the live simulation.
Water and sky sampling respect displayed game time rather than drawing the simulation frontier's storm hours early.
Keep bracketing state or the deterministic storm trajectory for the bounded display lag.

## A8 additions: generated land at every distance

### A8.8 Replacing the M2 fixtures (`PRE-03`, `PRE-29`, `PRE-30`)

M2's α2.9a supplies caches/streaming and α2.10a supplies overview/disc mapping.
M3 adds a generated-world provider behind those inputs; retain fixture providers for regression tests.
No new GDExtension, second world loop or second snapshot consumer is required.
Confirm the actual merged M2 interfaces at implementation start, since those steps were still planned at this audit.

Within about 300 m of focus at camp zoom inward, request full picture detail.
Out to about 10 km, draw coarse area ground and real cover.
Farther away and from region out, draw world cells and shared overview levels.
The request scheduler derives coverage from the projected portrait footprint plus height, overscan and shadow reach, not a fixed count of near squares.
Parent geometry remains visible until its children are ready and uploaded; detail fades in within about a second, with no generated work in a gesture callback.

Overview channels include elevation/relief, surface material/biome/cover, sea depth, fixed river geometry, snow/ice and weather.
Reduce categorical fields by a declared dominant/coverage rule; never interpolate material IDs.
Rivers remain vector/coverage features with the minimum two screen pixels `PRE-26` asks for, even when physically narrower.
This is an explicit visual width, not extra water or a wider ford.
Coasts and lake surfaces come from the same water and ground query as close views.
The globe repeats longitude, clamps latitude and hides the seam under the generated permanent cap.
Map, disc, markers and inverse taps share one mapping; local art always faces north at 37°.

Light receives the physical sun/moon/cloud state.
At rapid game time retain M2's presentation-only stable relief light; frost, rain, melt and physical daylight remain real state.
A static overview can cache albedo/relief; it cannot bake yesterday's cloud or water animation into immutable ground.

### A8.9 Cliffs, caves and the geological slice (`PRE-23`, `PRE-24`, `PRE-25`)

A single generated surface provider supplies floor height, cliff cap/face, shelter floor/back/roof/opening and deep-cave entrance/interior records.
Their normals, water intersections, foot contacts, picking and shadow receivers agree.
M2's ordered surface pieces remain the rendering method; split occluders across tile edges rather than sorting each tile alone.
Shallow roof reveal retains physical obstruction.
A deep cave uses its separate view; arbitrary stacked overhangs remain out of scope.

The geological cut-away is a separate 2D section panel, chosen by two points/a dragged line on the land view.
It samples the same rock contacts, soil thickness, cave voids, water-table head and dated deposits used elsewhere, then draws them against distance and depth.
Show true horizontal distance, a depth scale and any declared vertical exaggeration.
Use local unwrapped coordinates for a section crossing the east-west seam; a polar-seam crossing stops at the barrier.
Sample feature intersections analytically or at their certified bounds so a small cave is not missed merely by a coarse section step.

Groundwater is drawn as saturated material/head, not an invented open underground lake.
Actual cave pools use their own water surface.
The line's finite inspection width is displayed, and objects included within it keep their real depth and projected distance; no grave is moved onto the line to improve a picture.
M3 can prove rock/soil/water/cave sections and a labelled synthetic buried-record fixture.
The 200-year camp acceptance uses M4's actual traces when available; a synthetic tool is never described as a generated historic find.

## A17 additions: world proofs

- Pin new canonical stage digests for small generated worlds and a bounded full-world sample on all existing compiler/architecture builds, with one and four workers, reordered tasks and resumed stages.
  M1 proof digests remain unchanged.
- Add generated-world scenes to the existing scene kind/measures registry, with declared runs, seed sets, time limits, session-hour budgets and pass rules before tuning.
  Full seed sweeps run in the background; small structural scenes remain quick checks.
- Plant faults that reverse a river edge, erase a real basin, cross the polar seam, change a border sample, consume chance by load order, miss a revision, lose an in-flight flood or accept an ore without provenance.
  Each intended check must first catch its fault.
- Compare equal game seconds with camera absent, still, touring, entering caves and slicing, with cache eviction and different speed requests.
  Saved physical state, journals and digests match.
- Resume a kept area daily and after a season, with view-only copies made on arbitrary days; end state and cell-accounting deltas match.
  A view-only request must not change a save byte or ID counter.
- Test saves during storm motion, flood promotion, warning/eruption, catch-up and generation cancellation.
  Export/import, old-world corpus and kill/recovery preserve state and logs.
- Geometry statistics and Earth-reference diagnostics support the owner's review; they do not decide whether the landscape looks right.
  The full acceptance matrix is in the M3 plan.

## A18 additions: M3 budgets and risks

The Q7 timing and Q8 memory lines below were approved on 8 October 2026; stage allocations and storage estimates remain proposals, not measured production results.
P7's 9.3-second generation and 6.5-second settling are historical prototype evidence only; its code is not reused.

| Work | Initial planning allocation |
|---|---|
| Twenty coarse candidates | 45 s total held-phone wall time |
| First six full candidates, all stages and qualification | 90 s total |
| Extra candidates/refinements, final ranking, previews | 35 s total |
| UI/publication/checkpoint overhead | 10 s total |
| New world to offered globes | 180 s target, 198 s outer limit (answered Q7) |
| Selected world settling and later bands | 60 s target, 66 s outer limit (answered Q7); not yet fully testable before M5 |
| World layers | At most 0.2 one-middle-core seconds per game day, measured separately from generation |
| Area preparation | About 0.1 s each; visible full detail within about 1 s; making rule areas at most 10% of simulation time |

The fallback search to 40 candidates is part of the same end-to-end timing report.
A failure names the costly stage; no runtime time cutoff silently changes the offered worlds.
Reduce temporary allocation, use better algorithms and retune fixed work for a later making version before proposing a lower-quality requirement.
Settling while the owner reads choices cannot be used to hide the one-minute measurement.

**Memory ledger, initial envelope:** 2,000,000 cells at a proposed 128 bytes of combined base/dynamic/seasonal storage is 244.14 MiB; 20,000 weather cells at 128 bytes is 2.44 MiB.
Feature tables get 96 MiB and a hard counted diagnostic; generation scratch 192 MiB; a 24-area metre-height cache about 5.77 MiB for heights alone, with a 32 MiB complete CPU-area budget.
M2's sampled textures 128 MiB, targets 32 MiB and staging 4 MiB remain separately counted.
Allow 512 MiB for process/engine/other retained state and 256 MiB for snapshot/compression overlap as a starting audit reserve.
Together these planned allocations are about 1.46 GiB before later kept areas, people and history.
The approved Q8 M3 peak-process working line is 2 GiB, within the game's about 8 GiB envelope; it supersedes the M2 fixture-only 1 GiB line while retaining the separate graphics budgets.
This is arithmetic, not evidence these records fit or the phone stays cool.
All real struct sizes, heap overhead, temporary duplicates and GPU residency must be measured.

The world's normal daily work is the larger risk than storing two million cells.
Twenty-four updates of 20,000 weather cells are 480,000 weather-cell steps a game day, before the two-million-cell daily water pass.
Four-season climate arrays avoid expensive climate solving during play.
Use compact stores, active flood lists and precomputed immutable coefficients; don't drop unwatched cells or change the fixed cadence under load.
Sparse exact updates may skip a provably unchanged value, not an unobserved place.

| Risk | Proof or response |
|---|---|
| Candidate qualification becomes too rare | Report every rejection; tune generation globally on training seeds; never repair a scored candidate |
| Polar topology leaks into weather, river flow or future paths | One tested forbidden-edge policy, including swept/diagonal motion; preserve exact coordinate maths |
| Numeric maps pass but land looks artificial | Fixed-scale contact sheets of branching valleys, coasts, lake outlets and rock sections, reviewed by the owner |
| Final climate changes coarse-candidate suitability | Full-resolution recertification before offering, stored rejection reason |
| Weather averages or eclipse rate cannot meet wording | Apply answered Q3/Q6; freeze remaining reference/orbit parameters before testing and report failures without retargeting |
| Catch-up logs or saves grow without bound | Count bytes/year, retain needed disturbances, prove compact checkpoints; raise a real limit before changing history rules |
| Full-world saves stall rendering | Reuse immutable bytes, measure copying and compression overlap; extend keeper only with recovery proofs |
| Generated cliffs defeat M2 ordering/shadows | Generated stress fixtures early; improve piece splits before broadening terrain forms |
| Cross-milestone acceptance is mistaken for completion | Q1 and the plan's deferred-check ledger remain explicit in every report |
