# Kindling M3: research and recommendation

Research draft, 8 October 2026. Prepared for the M2 closing task `T2.12a.4`. This is a proposal for the owner, not an adopted design or a report of implemented M3 behavior. The repository was read only; no prototype code was copied from history.

The recommended approach is a bounded causal generator over the two-million-cell world, followed by fixed-cadence weather/water systems and pure on-demand local detail. It should imitate the consequences of geology and climate without simulating a planet's physical history. Global facts must constrain local pictures; pictures must never change the world.

The biggest unresolved issue is milestone scope. Ten years of live vegetation/herd settling, actual start survival, starting bands and a real 200-year buried camp require M4/M5 systems. M3 can supply their geography and interfaces, but a mock cannot pass those tests. Q1 in QUESTIONS.md proposes explicit phased acceptance, preserving every later check. Without owner approval, that conflict blocks the proposed milestone closure.

## 1. Evidence and its limits

The audit read CLAUDE.md, HANDOVER.md, PROJECT.md, ARCHITECTURE.md and IMPLEMENTATION.md, plus the external 2D ENGINE.md. PROJECT.md's principles and requirements control. Current repository decisions supersede earlier pending proposals in ENGINE.md: the fixed north-facing 37° view and torus-to-globe presentation are already agreed. M1 is accepted. M2's later streaming/overview steps are planned interfaces, not existing completed code.

The implementation audit covered exact coordinates and numerical conversion, keyed chance, IDs/components, calendars, world/system ownership, workers, catalogue fingerprints, saves, snapshots, projection and fixture terrain. Specific code findings appear below. The accompanying architecture follows these existing contracts instead of proposing a new engine.

Web research used papers, author/developer accounts, geological/weather agencies and official software documentation. Findings from abstracts are described as such; a paper's desktop speed is never converted into a phone promise. Sources establish physical relationships and algorithm choices. They do not establish Kindling's performance or prove that generated maps look good.

No full generator, phone benchmark or ecological survival experiment was run in this research. The small calculation in `budget-check.py` checks dimensions, storage and workload arithmetic only. The two helper reports, `earth-findings.md` and `systems-findings.md`, retain the broader source/code audit. The five requested documents form the consolidated proposal.

## 2. What the generator has to preserve

| Binding constraint | Consequence for the design |
|---|---|
| `WLD-01–03`, `WLD-12` | Exact M1 torus remains 2000 × 1000 km. Proposed 1 km cells, 250 m areas and 10 km weather cells align exactly. No areas are made during global generation. |
| `WLD-09` | Plates → rock → rough climate and erosion/drainage → final climate → soils → deposits → biomes/cover → animals. Later stages may classify earlier results, not replace them with unrelated noise. |
| `WLD-08`, `WLD-17` | Preserve real lake basins, fixed river courses and geological sediment provenance. Runtime water changes levels, not the generated terrain shape. |
| `WLD-10`, `WLD-24` | Score about 20 coarse candidates, refine the best few, offer up to three qualified worlds. Reject failures without inserting missing caves/resources. Keep the chosen start through settling. |
| `WLD-11` | New world to choices about three minutes; selected world and later bands about one more minute, at held phone speed. Fallback search is inside the measured route. |
| `WLD-12–13`, `PRN-02` | Looking changes nothing. Area generation, camera catch-up and cache eviction cannot consume authoritative IDs, alter timestamps or save state. |
| `WLD-16`, `WLD-30`, `TIM-18` | Fixed climate, hourly weather, short seasons, real wind and river travel. Store explicit units; do not apply a single time/length scale to everything. |
| `SCP-21` | No deep chemistry, live sediment transport, slow live erosion, tides, changing climate or ice ages. |
| `PRE-03`, `PRE-25`, `PRE-29` | Same actual ground from globe to cliff and geological slice, with later real buried records at their saved depths. |

A 250 m area is 2.34% smaller than the approximate 256 m requirement and gives exactly sixteen areas per cell. Four exact 256 m areas do not fit a kilometre. Q2 makes that interpretation reviewable. It does not change M1's torus.

The 200 km permanent polar band straddles the north/south seam. Static terrain is periodic, but transport cannot cross the band's middle. These are separate contracts: wrapped coordinates do not authorize a weather parcel, river, path, animal or fire front to cross. Test full swept segments and diagonals as well as immediate neighbours. The globe compresses polar geography only for display; it cannot change cell areas or distances.

## 3. Plates, heights and rock

Cortial and colleagues' *Procedural Tectonic Planets* demonstrates a procedural treatment of tectonic phenomena rather than expensive full physical evolution. Its publisher abstract supports a causal feature approach; its spherical model is not a drop-in torus implementation. The large author PDF was not text-inspected because the browser rejected its size. [Publisher paper](https://onlinelibrary.wiley.com/doi/abs/10.1111/cgf.13614), [author PDF](https://perso.liris.cnrs.fr/eric.galin/Articles/2019-planets.pdf).

For Kindling, propose 6–12 periodic plate domains with stable IDs, crust/age categories and motion vectors. Relative boundary motion classifies convergence, divergence and shear, which create bounded mountain ribbons, rifts, volcanic arcs and fault sources. Old continental/marine provinces supply sedimentary provenance. A fixed number of global passes can produce present-day consequences without geological timesteps.

Layers must be coherent interface surfaces, not three independently random materials per cell. Erosion exposes an underlying layer by intersecting those interfaces. A cliff face and a cutaway query the same boundaries. Generated caves receive host, entrance, usable floor, roof, wetness and size descriptors before any area exists. Limestone karst and lava tubes have different origins; that supports host-conditioned cave forms without a chemistry simulator. [USGS karst aquifers](https://www.usgs.gov/mission-areas/water-resources/science/karst-aquifers?page=0).

Set sea level from the generation's global height distribution, then freeze it. Recheck the final land fraction after erosion. Do not choose a new sea level in play, and do not lower local water simply to make a start suitable.

## 4. Drainage, erosion and lakes

Priority-Flood supplies a well-studied depression/watershed algorithm and deterministic queue ordering can be added with stable cell IDs. Its usual edge-seeded treatment needs adaptation to a torus: the sea provides outlets, while the blocked polar seam provides no artificial drain. Most importantly, filled spill height must be a routing field separate from physical terrain height, or all depressions disappear. [Barnes, Lehman and Mulla, 2014](https://rbarnes.org/sci/2014_depressions.pdf), [authors' reference implementation](https://github.com/r-barnes/Barnes2013-Depressions).

Store each genuine basin's membership, spill level, outlet and volume/level curve. Preserve terminal inland basins explicitly. Flat terrain needs a strict drainage rank rather than floating epsilon height changes. Fill–Spill–Merge shows how a depression hierarchy can retain fill/spill relationships; start with the smallest basin graph that represents generated worlds and add full nesting only if evidence requires it. Neither algorithm alone models channel flow time or flood damage. [Barnes, Callaghan and Wickert, 2021](https://esurf.copernicus.org/articles/9/105/2021/index.html).

FastScape's implicit stream-power incision method motivates a fixed number of stable erosion sweeps instead of innumerable simulated rain droplets. Its reported linear-time incision solve is not a complete landscape generator. Use discharge, slope and rock resistance, with bounded slope relaxation and separately declared depositional/glacial forms; recompute drainage after height changes. [Braun and Willett, 2013](https://www.sciencedirect.com/science/article/pii/S0169555X12004618).

Hydrology-based terrain work also shows the value of preserving a river hierarchy while constructing continuous surrounding terrain. Kindling should use that insight as a constraint: final global river lines and channel profiles control fine terrain, so detail cannot invent a ridge across a river at an area border. The paper's river-first authoring order does not replace WLD-09's plate-first generation. [Génevaux and colleagues, 2013](https://doi.org/10.1145/2461912.2461996).

Sediment in generation can be categorical: source lithology plus transport/deposition setting produces gravel, alluvium, fans and deltas. Retain provenance and routed travel distance for deposit tests; matched stones round with longer transport, and hammer/grinding-stone patches follow their exposed quartzite/basalt/sandstone hosts. Runtime silt/ash burial is a dated change, not a new live terrain-evolution model.

Live water needs snow, soil, groundwater, lake and river stores with explicit transfers and rounding residuals. Groundwater supports baseflow and springs; a river is not merely today's rain routed downhill. [USGS groundwater and rivers](https://www.usgs.gov/special-topics/water-science-school/science/rivers-contain-groundwater).

Save delayed downstream arrivals. A single daily topological pass that sends all upstream water immediately to the coast fails real travel time. In a suitable valley, the length/speed relation should naturally produce the roughly one-day downstream flood lag. Hourly flood mode must conserve exactly the same accounted water when entering/leaving daily mode. Compressed seasonal recession applies where the project requires it; it cannot speed river motion sixfold.

## 5. Climate, vegetation and the compressed world

NOAA describes broad circulation belts and their wet/dry patterns, with land/sea distribution complicating the idealized bands. This is a useful causal baseline for a cheap climate model: latitude/tilt establish seasonal insolation, height changes temperature, seas moderate it, wind exposure and topography redistribute moisture. Latitude stripes alone are insufficient. [NOAA global circulation](https://www.noaa.gov/jetstream/global/global-atmospheric-circulations).

Smith and Barstad's linear orographic precipitation model is evidence that uplift, advection and precipitation delay produce windward rain and lee effects. M3 need not implement its full spectral model; a bounded moisture-advection and uplift/rainout sweep is an explicit approximation to test. [Original paper](https://journals.ametsoc.org/doi/abs/10.1175/1520-0469%282004%29061%3C1377%3AALTOOP%3E2.0.CO%3B2).

Generated ocean circulation/upwelling should influence nearby coasts and marine abundance, while sea level remains fixed. A directed basin/coastal influence field is enough for the proposed scope; it is not an ocean fluid solver. NOAA explains wind-driven surface circulation and nutrient-bearing upwelling. [NOAA ocean circulation](https://www.noaa.gov/jetstream/ocean/circulations), [NOAA upwelling](https://oceanservice.noaa.gov/facts/upwelling.html).

WorldClim 2.1 provides monthly 1970–2000 temperature, precipitation and other climate normals at several resolutions, including elevation. Beck and colleagues provide high-resolution Köppen–Geiger maps. These are useful independent test references, not runtime world data or a reason to reproduce Earth's continents. Match samples by latitude, altitude and maritime exposure and state the area weighting. [WorldClim data documentation](https://www.worldclim.org/data/worldclim21.html), [Beck and colleagues, 2018](https://www.nature.com/articles/sdata2018214).

A torus has equal-area latitude rows; an Earth latitude–longitude raster does not. Raw raster counts therefore give the wrong reference shares. The smaller continents, variable 15–30° tilt and imposed permanent ice create further unavoidable interpretation choices. Q3 proposes conditional latitude-band comparisons with a fixed reference table frozen before tuning. WLD-16's weather-to-own-climate 20-year bounds remain separate from this Earth comparison.

Biome classification should use seasonal temperature extremes, growing opportunity and moisture balance, followed by soil and species tolerances. The original BIOME work provides a physiological-constraint precedent, and macroBiome's documented implementation names useful climate indices. It does not authorize importing its catalogue or equations without adapting units and checking Kindling requirements. [BIOME paper record](https://publications.pik-potsdam.de/pubman/faces/ViewItemFullPage.jsp?itemId=item_11085_1), [macroBiome reference](https://search.r-project.org/CRAN/refmans/macroBiome/html/cliBIOMEPoints.html).

A 60-day year cannot be inserted directly into Earth annual rainfall or growing-degree thresholds. Q9 proposes ordinary daily water quantities and Earth-equivalent annual indices for classification, while saved seasonal totals cover actual fifteen-day seasons. This is a required unit convention, not proof that calibration will pass. The full twenty-year weather test must report every failing place; broad regional averages cannot conceal failures of the specified local bound.

## 6. Soil, deposits and living-world handoffs

NRCS describes soil as the result of parent material, climate, organisms, relief and time; transported parent material may differ from bedrock. Therefore retain sediment parent material independently, then derive soil texture/depth, drainage, retention, fertility and digging difficulty. [NRCS soil facts](https://www.nrcs.usda.gov/resources/education-and-teaching-materials/soil-facts).

The causal cycle between vegetation and soil needs a bounded solution. Derive provisional potential vegetation from climate for soil formation, then final biomes and species from finished soils. Runtime vegetation/fertility is separate from baseline soil and potential biome. This follows WLD-09 without pretending organisms never affect soil.

BGS defines flint narrowly as nodular chert in chalk. Q10 preserves that distinction while testing the broader flaking-stone family in eligible limestone and source-derived gravel. [BGS flint definition](https://webapps.bgs.ac.uk/bgsrcs/rcs_details.cfm?code=FLNT).

USGS sources associate substantial obsidian with suitable silica-rich volcanic settings; a generic volcano should not grant it. Generated glassy volcanic provenance is the proposed gate. Runtime eruptions add no new obsidian, as WLD-15 already requires. [USGS volcanic glasses](https://www.usgs.gov/publications/volcanic-glasses-their-origins-and-alteration-processes), [USGS Yellowstone obsidian](https://www.usgs.gov/observatories/yvo/news/yellowstones-tool-making-lava-flows).

USGS's porphyry copper model supports intrusive/convergent-province associations, not an assertion that copper occurs only there. Use geological province/host/depth gates for the selected game deposit families, record why a deposit is valid, and test against independent catalogue predicates. [USGS porphyry copper model](https://www.usgs.gov/publications/porphyry-copper-deposit-model).

M3 should output real species keys and initial coarse abundance/habitat records where the catalogue exists. Mammoths remain an owner decision (Q4 recommends one woolly species in cold open habitat). Such initial records do not prove future food renewal, realistic animal food demand or ten years of live herd settling. M4 owns those rules; M5 owns starting bodies and survival.

## 7. Weather, sky and rare events

Use fixed seasonal climate plus spatially coherent weather anomalies, advected storms and explicit cloud/rain/snow/wind state. At 10 m/s a storm travels 36 km in an hour, more than three weather cells. Sampling only its new position misses intervening rain and obstacles; swept deposition and transport-barrier tests are necessary. Hourly updates do not imply one-cell hourly motion.

Solar direction/day length can be checked independently against latitude, declination and hour-angle formulae. Handle polar day/night explicitly rather than allowing an inverse cosine outside its range. [NASA day length mathematics](https://www.grc.nasa.gov/www/K-12/Numbers/Math/Mathematical_Thinking/sun12b.htm), [NOAA solar equations](https://gml.noaa.gov/grad/solcalc/solareqns.PDF).

Lunar phase does not by itself determine eclipses: orbital inclination and node alignment matter. Keep WLD-07's 15-day phase cycle, add an explicit inclined-orbit/node model, and measure local horizon-visible alignments against the owner's chosen frequency definition. Q6's proposed count is a game acceptance target, not an Earth astronomy statistic. [NASA eclipse explanation](https://science.nasa.gov/moon/eclipses/), [NASA eclipse periodicity](https://eclipse.gsfc.nasa.gov/LEsaros/LEperiodicity.html).

Quakes and eruptions must originate from generated faults/volcanoes, with rate, warning and quiet-time state owned by those systems. Landslides can follow wetness/shaking/slope/material conditions, rather than arbitrary geographic dice. The physical sources support the causes, not the project's exact event frequencies. [USGS landslide explanation](https://www.usgs.gov/programs/landslide-hazards/what-a-landslide), [USGS volcanic hazards](https://pubs.usgs.gov/fs/2018/3075/fs2018-3075.pdf).

Correct mean rainfall does not establish correct storm frequency, drought persistence or natural flood rates. G4b therefore checks each M3 event kind by climate, with frozen reference definitions and yearly exposure units; M4 later adds wildfire. Climdex documents frost, heavy-rain and consecutive-dry-day indices useful for defining those comparisons. Its Earth-duration thresholds need the declared short-year conversion. [Climdex index definitions](https://www.climdex.org/learn/indices/).

NASA's LIS/OTD products provide gridded lightning climatology. Dataset coverage and whether a quantity counts total flashes or ground strikes must be recorded; total-flash rates cannot be used directly as fire ignitions. This research identifies references but has not fitted a per-climate event-rate catalogue. Reference preparation and independent rate checks are explicit implementation tasks. [NASA LIS/OTD monthly climatology](https://doi.org/10.5067/LIS/LIS-OTD/DATA303).

A runtime eruption changes surface material, ash, damage and burial while preserving fixed terrain heights. Save dated disturbance footprints independently of the readable historical log: a kept area still needs the flood or ash event after ordinary history has been thinned. Catch-up must not reconstruct yesterday's flood from today's water level.

## 8. What game precedents do and do not establish

Dwarf Fortress's developer record describes fitting detailed rivers/tributaries into larger world structures. This is a useful multi-scale precedent: global drainage constrains local rivers. It does not establish a modern phone budget or license opaque candidate repair. [Bay 12 development record](https://www.bay12games.com/dwarves/dev_2007.html).

Factorio's developers explain cliff representation, constrained tile patterns and the need to preserve shared chunk edges. Their presentation also uses varied tile sizes and overlays to reduce visible grid repetition. These lessons support shared borders and bounded surface pieces, but Factorio's simplified cliff mechanics do not provide Kindling's continuous height, cave or slice model. [Factorio cliffs](https://www.factorio.com/blog/post/fff-219).

CrossCode's developer account shows a height-map tool producing cliffs with terrain-specific transitions. It is evidence that coherent height and terrain metadata help a fixed-view pixel-art workflow, not evidence of a causal global generator. [CrossCode mountain/terrain development](https://www.radicalfishgames.com/?p=3598).

The Around the World developer's climate account provides a practical procedural-climate comparison. Its simplifications are useful experiments, not a replacement for Kindling's own quantitative climate tests. [Developer climate article](https://frozenfractal.com/blog/2023/12/29/around-the-world-9-climates/).

## 9. M1 integration: determinism and persistence

| Existing code | Finding and proposed consequence |
|---|---|
| `sim/src/kd/num/torus.hpp`, `whole.hpp`, `convert.hpp` | Whole-centimetre wrapping, negative floor division and explicit rounding already exist. Subtract a nearby exact origin before converting view values. |
| `sim/src/kd/chance/chance.hpp` and implementation | Existing Draws uses keyed SplitMix64-style mixing, not Philox. Key by seed/system/feature/date/purpose/index; do not replace accepted randomness or share a mutable draw stream. |
| `sim/src/kd/world/world.hpp`, `ecs/id.hpp` | Systems own authoritative work and IDs. Pure detail uses stable generated feature keys; a later physical action promotes a feature in defined order and saves the association. |
| `sim/src/kd/run/workers.cpp` | Existing workers establish numerical mode and join tasks. Fixed outputs and frozen neighbour buffers are still required; reduction order and queue ties cannot depend on completion order. |
| `sim/src/kd/world/parts.hpp` | Place currently has a 2D point, without the needed cave floor identity. Add/migrate a real SurfaceRef; the view's highest-floor fixture rule is insufficient. |
| `sim/src/kd/time/calendar.hpp`, world frontier | Calendar can represent negative dates, but current world starts at frontier zero. Settling needs an explicit date-origin wrapper or a proved prehistory extension, not negative scheduled work slipped into M1. |
| `sim/src/kd/data/schema.hpp`, `save/versions.cpp` | Making/rules/look fingerprints are distinct. Declare every new catalogue field's category; never silently regenerate old terrain after a making change. |
| `sim/src/kd/save/keeper.cpp`, `snapshot.cpp` | Current save captures full world chunks and whole snapshot/compression buffers. Large static ground requires measured copy costs and an integrated immutable-blob strategy with crash recovery. |

Counter-based random design is an established response to scheduling-dependent random streams. It supports M1's existing keyed approach, not an algorithm replacement. [Random123 authors' paper](https://www.thesalmons.org/john/random123/papers/random123sc11.pdf).

Keep A3's accepted double intermediate arithmetic through kd::num, with saved integers and explicit rounding. “Use integers everywhere” would misstate the existing numeric contract. Use one/four-worker and compiler/architecture digests to expose differences. Owner IDs and schedule arrays need explicit extension/versioning; M1 proof scenes and their existing digests remain unchanged.

The current SYST save chunk contains named system blobs; it does not automatically version each system payload. New world systems need local payload versions, bounded decoding and migrations. Static generated terrain is essential saved state: regenerating a three-minute world during a three-second reopen is not a viable cache strategy. Reusing immutable serialized bytes is a starting approach; separate-file/incremental storage, if required by measurements, is a keeper/archive change requiring atomicity and interruption proofs.

## 10. On-demand detail, catch-up and view publication

A global periodic sampling function gives shared border points identical inputs. Its lattice period must divide the actual torus dimensions. Neighbour features are queried through a declared halo in stable order. Rivers, caves and strata constrain the fine shape; unconstrained terrain noise cannot supply those guarantees. [Red Blob Games terrain discussion](https://www.redblobgames.com/maps/terrain-from-noise/).

Separate immutable generated facts, mutable world cells, logical area rules, disposable metre terrain and saved local changes. Metre geometry remains picture-only. Resource positions must have stable identities and match counted resources; decorative grass is a different output. Global priority resolves cross-border placement spacing, never the order in which the camera loads areas.

Kept areas catch up day by day with usual seasonal weather, keyed chance and actual dated disturbances. Camera inspection does this on a private copy. Only an authorized simulation activation commits the result and its timestamp. Daily visits, season-late visits and arbitrary camera tours must end with identical state. Coarse-cell harvest/depletion is charged once at the original action, not again while replaying a picture.

M2's triple buffer is one-producer/one-consumer and can skip publications. Its receipt revision is not a terrain revision. Publish explicit world-open epoch, world identity, frontier/display times, immutable handles and per-layer revision manifests. Jobs retain owned immutable data, reject stale dependencies and never read the live simulation. A 64 × 64-cell chunk grid has only 512 revision entries, so a complete manifest is practical.

The displayed walkers can lag the simulation frontier. A correctly versioned but future-state flood is still wrong around a past-state walker; retain timed transitions or define a synchronized display barrier. Bound in-flight jobs, completed CPU output, GPU uploads and cache memory independently. Godot's documentation cautions against worker access to active scene trees and GPU work; check the exact calls against the pinned engine. [Godot thread-safe API guidance](https://docs.godotengine.org/en/stable/tutorials/performance/thread_safe_apis.html).

## 11. Globe to cliff and geological section

M2 provides the projection/pixel-art structure. M3 supplies generated providers in place of fixture geography. Full area detail is distance-gated to about 300 m; coarse ground continues to about 10 km; cells supply the regional map and globe. A single low-density local screen can extend beyond the 300 m radius, so screen mode alone is not a sufficient detail rule. At 540 × 1200 internal pixels and 2 px/m, the flat-ground footprint is approximately 270 × 997 m before height/overscan.

Replace fixture material-ID ranges, scripted weather and sine-wave actors with typed terrain and dated published state. The fixture's 32-body cap and all-pairs surface ordering are not whole-world limits. Use spatial candidate selection and existing split surface pieces; picking must intersect what is drawn. Revisit the fixture's fixed shadow reach for generated high terrain, with declared dependency/quality bounds.

Rivers keep true rule widths/depths; minimum visible pixel width is presentation only. Clouds/water cannot be baked into immutable overview imagery. Globe poles clamp to their respective map edge; they do not blend opposite seam rows or introduce a new physical spherical world.

Caves need actual floor/roof/opening surfaces and a saved surface identity. Shallow roof reveal and deep-cave view remain distinct. The geological slice samples the same strata, soil, cave voids and groundwater head along a chosen line. Show distance/depth and any vertical exaggeration. Groundwater is saturated rock/soil, not automatically an open underground lake. Later saved graves/tools remain at their real depths; M3's synthetic trace fixture stays labelled.

## 12. Budgets and checks that can falsify the proposal

The proposed combined base/dynamic/seasonal cell envelope is 128 bytes per cell: 244.14 MiB for two million cells. Weather adds 2.44 MiB. Feature tables, scratch, area data, textures/targets/staging, process reserve and snapshot overlap bring the initial ledger to about 1.46 GiB. A 2 GiB measured M3 peak-process line is proposed in Q8; it is not approved and is not evidence that the actual layout fits.

Prebuilding all 32 million possible areas with only one 251 × 251 four-byte height grid each would already cost about 7.33 TiB. On-demand detail is essential. Twenty-four such grids are only about 5.77 MiB, but normals, materials, masks, meshes and duplicated in-flight outputs must also count. The architecture allocates a 32 MiB complete CPU-area budget, separately from the M2 graphics budgets.

The planning split is 45 seconds for twenty coarse candidates, 90 for six full candidates, 35 for fallback/refinement and 10 overhead. This is an allocation to test, not a forecast from a measured generator. Sequential full refinement limits memory; retain summaries and only the top qualified artifacts. Work counts and tie-breaks are deterministic; wall-clock time never decides which candidate wins.

If candidate qualification were independent with probability one quarter, fewer than three successes would occur about 9.13% of the time after twenty candidates and 0.10% after forty. Real candidates may be correlated, so this calculation is illustrative only. It explains why fallback cost cannot be omitted and why the specification permits fewer than three choices.

Daily work is also a serious risk: hourly weather gives 480,000 weather-cell updates/day, plus two million daily water cells and five-day layers. The target is 0.2 single-middle-core seconds/game day. At that target twenty worlds run for one hundred sixty-day years consume about 6.67 core-hours before diagnostic overhead. Long validation therefore needs explicit background/session budgets and resumable seed runs.

The plan freezes generation measures, seed sets and pass rules before tuning. Its seven acceptance groups cover terrain/hydrology; geology/resources; candidates/starts; climate/weather; sky/hazards; borders/catch-up; and phone performance/recovery. Required statistics include drainage termination, slope distribution, lake share, coastline straightness, candidate qualification and climate consistency. Definitions such as slope scale and coast chord tolerance require owner approval; choosing a convenient definition after seeing results would invalidate the check.

Landscape metrics can compare distributions and diagnose defects, but no single statistic certifies believability. Geomorphological terrain-class comparisons are one published evaluation direction, not a replacement for visual judgement. [Procedural terrain realism research](https://arxiv.org/abs/1909.04610). Review fixed contact sheets at globe/region/valley/cliff scales, with river junctions/outlets, shadows, caves and sections. The owner decides whether the land looks right after numerical gates pass.

The independent critic's findings and resulting corrections are recorded in REVIEW.md. All research conclusions remain proposals until the owner accepts them. No phone performance, M3 implementation or repository adoption is claimed by this delivery.
