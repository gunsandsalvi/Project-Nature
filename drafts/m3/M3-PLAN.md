# Kindling: M3 implementation plan draft

For the M2 closing review (`T2.12a.4`), 8 October 2026.
Proposed replacement for the M3 outline in IMPLEMENTATION.md, incorporating answered Q1–Q10 of 8 October 2026. No repository edit or implemented-milestone acceptance is implied.
The architecture references below name sections in `M3-ARCHITECTURE.md`, which would replace/extend the repository sections after approval.

## M3 The world

**Goal:** make whole worlds from a seed in causal order, choose among the best candidates, and explore the same land from globe to cliff and geological section, through real seasons, weather, water and rare land events (`MIL-10`).

**Serves:** `WLD-01`, `WLD-02`, `WLD-03`, `WLD-06`, `WLD-07`, `WLD-08`, `WLD-09`, `WLD-10`, `WLD-11`, `WLD-12`, `WLD-13`, `WLD-14`, `WLD-15`, `WLD-16`, `WLD-17`, `WLD-22`, `WLD-24`, `WLD-26`, `WLD-27`, `WLD-30`, `PRE-03`, `PRE-25`, `PRE-29`.
It also integrates `PRE-02`, `PRE-23`, `PRE-24`, `PRE-26`, `PRE-30`, `PRE-31`, `PLT-04`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `TIM-14`, `TIM-16`, `TIM-18`, `RES-05`, `RES-06`, `RES-21`, `RES-22`.
The existing “Rules every alpha keeps” still apply in full.

**Entry gate:** M2's accepted projection, height/light/order, streaming and map/globe interfaces, with their actual merged code re-read.
At this audit α2.9a/α2.10a were still planned and α2.8a needed delivery/review.
Do not treat their architecture as completed implementation.
The owner answered Q1–Q10 on 8 October 2026. Apply those decisions below; freeze remaining detailed reference tables and pass rules before tuning.

**Approved scope Q1 (8 October 2026):** complete M3 claims cover physical geography and the world systems built here.
The approved phased acceptance carries full ecological settling and traces to M4, bodies/bands/survival to M5, and controls to M9.
No missing subsystem is implemented as a decorative substitute or declared passed from a mock.
Keep every later check open until its assigned milestone proves it; Q11/Q12 remain the later visual and milestone acceptance reviews.

**You will see:** plates become land and rivers, climate shapes soils and vegetation potential, three qualified world choices, storms and delayed floods, a cold-to-warm world tour, cave entrances/interiors, and a line through the ground showing its true layers and water.

**Estimates:** each lettered delivery below is initially 4–6 builder hours, including focused tests and delivery; 24 deliveries imply roughly 96–144 hours, not a promise of calendar completion.
If a step exceeds six hours, split at a task boundary and give each new letter its own APK and tests.
Numbered alpha/task IDs become fixed only when work starts; version codes below follow the existing formula.

| Step | Title | Delivery | Status |
|---|---|---:|---|
| α3.1a | World contract and boundary tests | 40101 | Proposed |
| α3.1b | Generated-world saves and proof harness | 40102 | Proposed |
| α3.2a | Plates, land and ice | 40201 | Proposed |
| α3.2b | Rock layers, faults and cave envelopes | 40202 | Proposed |
| α3.3a | Drainage, flats and lakes | 40301 | Proposed |
| α3.3b | Erosion and fixed river geometry | 40302 | Proposed |
| α3.4a | Final climate and ocean influences | 40401 | Proposed |
| α3.4b | Soils and geological resources | 40402 | Proposed |
| α3.5a | Biomes and initial life records | 40501 | Proposed |
| α3.5b | Repeatable area facts and coarse ground | 40502 | Proposed |
| α3.6a | Sun, moon and stars | 40601 | Proposed |
| α3.6b | Hourly weather | 40602 | Proposed |
| α3.7a | Water, snow and floods | 40701 | Proposed |
| α3.7b | Sea ice, soil changes and water quality | 40702 | Proposed |
| α3.8a | Natural faults and volcanoes | 40801 | Proposed |
| α3.8b | Disturbance logs and area catch-up | 40802 | Proposed |
| α3.9a | Candidate ranking and start certificates | 40901 | Proposed |
| α3.9b | Choosing, settling and world lifecycle | 40902 | Proposed |
| α3.10a | Generated land from globe to cliff | 41001 | Proposed |
| α3.10b | Caves and the geological slice | 41002 | Proposed |
| α3.11a | Generation and steady-world performance | 41101 | Proposed |
| α3.11b | Held-phone, streaming and recovery route | 41102 | Proposed |
| α3.12a | Held-out worlds and final visual repair | 41201 | Proposed |
| α3.12b | Independent review and M3 close | 41202 | Proposed |

### Delivery rule for every lettered step

Run only touched tests while building, each new test failing once against a planted fault.
At delivery run `tools/check.sh --deliver` once and review the step; at b review its whole numbered alpha too (`PRC-09`, `PRC-10`).
Keep all accepted M1 proof answers, old-save behavior and earlier quick checks intact.
Ship the signed APK and plain note naming task/requirement IDs, what to try, rough edges, measured results and outstanding cross-milestone checks (`PRC-11`).
World-making changes identify a big update and keep old history readable; schema-only compatible changes supply migrations (`PLT-09`).
The builder makes the review and delivery concrete; the coordinator handles commits/merges under the protocol.
These instructions describe future implementation deliveries; this research draft has no APK, commit or performance pass.

## α3.1 The generated-world foundation

### α3.1a World contract and boundary tests

**Goal / architecture:** typed fields, units, IDs and immutable inputs before any terrain algorithm; A7.2–A7.4.

**Tasks:**
1. `T3.1a.1` **Grid and topology (`WLD-01`, `WLD-03`, `WLD-12`).** Implement canonical world/weather/area addresses, the approved 250 m pitch and common polar transport-edge checks without altering `Torus` maths.
2. `T3.1a.2` **Schemas and job interface (`WLD-08`, `WLD-09`, `TIM-16`).** Add validated generation/rock/climate parameter schemas through existing catalogue visitors; exact units, separate world/rules/look fields and stage digests.
3. `T3.1a.3` **Land laboratory (`PRN-04`, `RES-21`).** A clearly labelled small synthetic data page shows layers, units and seam samples through the existing extension, with generation progress/cancellation and measured allocation counters.

**Tests / done when:** every one of 2,000 × 1,000 cell addresses maps to exactly sixteen 250 m areas; randomized wrapped queries preserve coordinates; all seam-crossing adjacent, diagonal and swept transport cases are refused; east-west cases pass.
Catalogue planted faults fail with source locations; cancellation changes no saved world.
No phone timing claim comes from this synthetic page.

**On the phone:** open Land laboratory, inspect a cell and its sixteen areas, pan across east-west and inspect the polar stop.
**Risk:** mixing grid/storage sizes with the accepted world size; keep exact dimensions in the proof.

### α3.1b Generated-world saves and proof harness

**Goal / architecture:** real saved layer state on M1's lifecycle; A7.14, A17.

**Tasks:**
1. `T3.1b.1` **Layer owner extension (`TIM-16`, `WLD-12`, `PLT-09`).** Append owner IDs, version owner schedules and preserve old owners; a new generated-world wrapper attaches systems while demos retain their original systems.
2. `T3.1b.2` **Saved base and mutable chunks (`PLT-07`, `PLT-08`, `PLT-09`).** Critical base chunk, locally versioned system payloads inside SYST, candidate metadata, checked sizes and migrations; immutable shared bytes where safe; no foreign cache files in archives.
3. `T3.1b.3` **Scenes and new proofs (`RES-05`, `RES-21`, `RES-12`).** Add generated-world scene measures and stage-resume harness; retain all M1 proof suites unchanged; add one new generated-layer suite and corruption faults.

**Tests / done when:** one/four workers, save/reopen/export/import and stage resume yield identical canonical state; old corpus loads with original outcomes; unknown critical chunks/oversized counts are refused before allocation.
Owner arrays migrate with no new demo events; capture snapshot copy/peak bytes.

**On the phone:** save the test layer, switch worlds, reopen/export/import, and run its matching-digest self-check.
**Risk:** `owners::count` and small history payloads are concrete extension work, not pre-existing general world storage.

## α3.2 Land from plates

### α3.2a Plates, land and ice

**Goal / architecture:** recognizable generated continents and ranges, with causal labels; A7.5.

**Tasks:**
1. `T3.2a.1` **Plates and boundaries (`WLD-06`, `WLD-09`).** Periodic plate partition, motion types and boundary-derived uplift/rift/subduction/transform features.
2. `T3.2a.2` **Ground and sea datum (`WLD-03`, `WLD-08`, `WLD-30`).** Shelves, worn continents, range profiles and one frozen sea level; no generation of detailed areas.
3. `T3.2a.3` **Permanent cap and previews (`WLD-01`, `WLD-02`, `PRE-29`).** Generate the 200 km ice strip; show real height and plate overlays through the shared overview provider.

**Tests / done when:** 100 parameter seeds stay within tilt/land ranges and contain 6–12 plates; plate edges and feature references agree at both seams; no partial area generation occurs.
Report relief/land share at full resolution; ordinary plateau noise cannot create an unrelated high range.
The same 100 seeds are checked again after erosion: final land remains 25–50%, and ten equal bins of both tilt and land share each contain at least one world (proposed minimum spread check, frozen before tuning).

**On the phone:** choose a seed, watch the plate/height layers, compare two seeds and inspect the cap on the globe.
**Risk:** straight cellular plate borders visible in final relief; boundary perturbations must retain shared identities.

### α3.2b Rock layers, faults and cave envelopes

**Goal / architecture:** rocks and shelters have a consistent cause before local detail; A7.5–A7.6.

**Tasks:**
1. `T3.2b.1` **Geological provinces (`WLD-09`, `PRE-23`).** Named rock catalogue, coherent three-layer columns and ancient marine/basin provenance, with erosion resistance and permeability.
2. `T3.2b.2` **Feature descriptors (`WLD-09`, `WLD-15`, `WLD-24`).** Fault/volcano identities and type, plus provisional cave/overhang host and size envelopes; final existence/entrance geometry follows erosion in α3.3b, and dryness follows final water.
3. `T3.2b.3` **Rock inspection and checks (`PRE-25`, `RES-05`).** Provisional geological profiles and provenance inspection; pin stage hashes and test neighbouring contact samples.

**Tests / done when:** 20 worlds have ranges/faults/volcanoes in their declared boundary influence zones and chalk/limestone only in permitted ancient basin settings; no stack exceeds three rock layers; sampled shared contacts agree exactly.
Provisional cave envelopes fit eligible hosts and dimensional bounds; reject contradictory descriptors. These are not yet final dry-shelter certificates.

**On the phone:** tap a range, volcano or provisional cave envelope to see its source and rock stack.
**Risk:** visual cave beauty must not invent dry floor area the certificate lacks.

## α3.3 Drainage and landform

### α3.3a Drainage, flats and lakes

**Goal / architecture:** all flow has a destination without flattening lakes away; A7.6.

**Tasks:**
1. `T3.3a.1` **Rough runoff (`WLD-09`, `WLD-16`).** First climate/runoff fields from latitude, relief and sea, explicitly separate from final climate.
2. `T3.3a.2` **Basin graph (`WLD-08`, `WLD-17`).** Sea-seeded Priority-Flood, original bed retained, spill hierarchy and deterministic flat rank; terminal basins explicit.
3. `T3.3a.3` **Flow proofs (`RES-05`, `WLD-01`).** Topological accumulation, lake storage curves, origin/destination debug overlay and adversarial flat/seam basin tests.

**Tests / done when:** every receiver chain terminates, no directed cycle or illegal polar transfer; all lake curves are monotone; nested basins fill/spill with exact modeled volume accounting.
Reversed receiver and lost-basin faults fail; one/four workers and reversed task issue order match.

**On the phone:** follow water from a ridge to a lake and then its outlet; inspect original bed versus spill level.
**Risk:** the filled routing surface must never overwrite physical ground.

### α3.3b Erosion and fixed river geometry

**Goal / architecture:** branching valleys, floodplains, fans, deltas and continuous rivers; A7.6, A7.13.

**Tasks:**
1. `T3.3b.1` **Bounded incision (`WLD-08`, `WLD-09`, `WLD-30`).** Fixed erosion passes with lithology/runoff response, slope relaxation and generated frozen-glacier forms; final routing after height edits.
2. `T3.3b.2` **Sediment provenance and waterways (`WLD-14`, `WLD-17`).** Depositional masks, transported-rock provenance and distance-dependent rounding classes; fixed reaches, junction anchors, width/depth and monotone bed profiles; finalize cave existence and entrances against the eroded ground.
3. `T3.3b.3` **Map measures (`WLD-08`, `RES-09`).** Implement the pre-approved slope, coast, drainage and lake metrics, with landform diagnostics and a 20-world training report.

**Tests / done when:** the G1 terrain measures below pass on the development set; rivers stay connected across 1 km/250 m borders and the longitude seam; no curve exits its valley/divide envelope or climbs its bed profile.
Cut through a provisional cave envelope in an erosion fixture: final entrances/host/floors must follow the changed ground; later flooded-start checks must reject a wet shelter.
The final held-out G1 confirmation remains α3.12a.

**On the phone:** follow a large river from mountain valley through floodplain to delta; compare hard- and soft-rock valleys.
**Risk:** more erosion iterations can cost time without making better land; retain them only with visible evidence.

## α3.4 Climate, soil and stone

### α3.4a Final climate and ocean influences

**Goal / architecture:** temperature, rain shadows and contrasting coasts explained by land and sky; A7.7.

**Tasks:**
1. `T3.4a.1` **Ocean descriptors (`WLD-26`, `WLD-16`).** Basin-aware warm/cold boundary influence, current direction, upwelling and coast classes.
2. `T3.4a.2` **Four-season climate (`WLD-06`, `WLD-16`, `WLD-30`).** Tilt/latitude/lapse, continentality and winds, bounded moisture transport with no polar crossing; store normal/extreme weather envelopes.
3. `T3.4a.3` **Reference harness (`WLD-16`, `SCP-11`, `RES-09`).** Q3/Q9's frozen research-only Earth comparison and synthetic mountain/coast tests; weather in mm per real-length game day, Earth-equivalent annual biome indices, and 15-day saved seasonal totals; no Earth map in playable data.

**Tests / done when:** lee dries versus matched windward terrain; height lapse and continental temperature range respond in the right direction; seasonal reversal holds; repeated longitude shifts create no privileged seam.
Reference bins and climate-share diagnostics use stated denominators; use answered Q3: 10-degree bands, ≤10 percentage-point class-share difference per sufficiently sampled band, about 2°C matched-bin warmth, and Earth-tilt plus 15°/30° cases.

**On the phone:** switch seasonal temperature/rain/wind overlays and inspect opposite ocean coasts.
**Risk:** exact Earth averages on a tiny variable-tilt world are not a self-defining test.

### α3.4b Soils and geological resources

**Goal / architecture:** useful ground follows causes and has readable depth/exposure; A7.7.

**Tasks:**
1. `T3.4b.1` **Soil data (`WLD-27`, `WLD-09`).** Parent sediment, slope/climate/potential vegetation, kind/thickness/fertility/retention/diggability.
2. `T3.4b.2` **Deposits (`WLD-14`).** Host-constrained flint/chert/obsidian, hammer/grinding stones, clay/ochre and copper/native copper with source-rock provenance in gravels. Answered Q10 keeps flint in chalk and chert in eligible limestone, both counted as the flaking-stone family.
3. `T3.4b.3` **Exposure and checks (`PRE-23`, `PRE-25`, `RES-05`).** Shared underground/exposed sample API; geological invalidity traps, independent hammer/grinding-stone host and matched upstream/downstream rounding checks; resource overlays and canonical encoding.

**Tests / done when:** 100 worlds have no illegal deposit occurrence, including transported gravel and native-copper cases; no material is regenerated just by inspecting it.
Fertility stays 0–5 and soil/rock units validate; source and downstream gravel agree.

**On the phone:** inspect flint in chalk, gravel downstream, a glassy volcanic source and weathered copper; slice their depth in the diagnostic profile.
**Risk:** finding a rare required ore must not turn into adding it to a failed candidate.

## α3.5 Biomes and local facts

### α3.5a Biomes and initial life records

**Goal / architecture:** the world's initial vegetation and animal records have habitat causes; A7.7, Q1/Q4.

**Tasks:**
1. `T3.5a.1` **Potential biomes (`WLD-09`, `WLD-16`, `WLD-27`).** Earth-equivalent temperature/moisture/growing-season indices, soil/wetness and shore/highland overrides.
2. `T3.5a.2` **Initialization contract (`WLD-09`, `WLD-10`, `WLD-30`).** Approved plant cover shares/regrowth-age fields, approved species occurrence/count/range descriptors and food-potential inputs; one schema extended by M4, no parallel species catalogue.
3. `T3.5a.3` **Visible scope and coverage (`PRN-10`, `PRE-29`).** Draw real supplied cover; clearly label unfinished ecology; list missing biome species and the later M4 acceptance checks.

**Tests / done when:** species occur only within catalogue habitat bounds; cover shares total their defined whole; sea and shore kinds use appropriate capacity tags; all initialized counts obey the sixth-scale convention.
Full 6-plant/4-animal biome coverage and ecological stability are not passed by initialization alone.
Answered Q4 includes one woolly mammoth species in cold open grassland within the roughly 30 wild-species budget; M4 proves its reviewed catalogue entry, food demand and density.

**On the phone:** visit forest, grassland, desert, marsh and tundra and inspect why each grows there.
**Risk:** initial cover is not a proved living food web; keep the M4 ledger explicit.

### α3.5b Repeatable area facts and coarse ground

**Goal / architecture:** on-demand queries agree across every edge and load order; A7.13, A8.8.

**Tasks:**
1. `T3.5b.1` **Canonical area queries (`WLD-12`, `WLD-13`).** Shared cave/river/deposit facts and world-coordinate placement keys; distinguish generated feature IDs from allocated entity IDs.
2. `T3.5b.2` **Coarse ground (`PRE-03`, `WLD-12`).** Tens-of-metres sampling from constrained parent relief, wrapped halos, fixed channel/cave anchors and pure material queries.
3. `T3.5b.3` **Order/cache proof (`RES-05`, `WLD-13`).** Request reversals, randomized partitions, cache erasure and neighbour placement competition; record bytes and cold preparation times.

**Tests / done when:** 10,000 sampled shared borders/corners agree exactly; river junctions do not break; repeated queries on the same day are byte-identical and change no world/save/ID counter.
Different cache tile sizes and one/four jobs yield identical canonical results.

**On the phone:** jump between distant unvisited places, clear the picture cache and return; the same ground and resources return.
**Risk:** a per-area random stream would change border objects when neighbours load in another order.

## α3.6 The sky and weather

### α3.6a Sun, moon and stars

**Goal / architecture:** physical hour, latitude and season drive the existing light; A7.10, A8.8.

**Tasks:**
1. `T3.6a.1` **Solar sky (`WLD-07`, `TIM-14`, `TIM-18`).** Turn-angle sun geometry, seed tilt, local solar hour, start-hemisphere phase and polar day/night limits.
2. `T3.6a.2` **Moon and eclipse geometry (`WLD-07`, `WLD-06`).** Fifteen-day phase, inclination/node parameters and seeded stars; answered Q6 requires 3–8 visible solar/lunar eclipses at every place in 70 game years, regardless of cloud.
3. `T3.6a.3` **View connection (`PRE-30`, `PRE-29`, `WLD-13`).** Physical sky replaces light fixtures only in generated worlds; preserve fast-time presentation light and fixture regression pages; build the full-location eclipse coverage proof after the sampled diagnostic.

**Tests / done when:** four full moons in each 60-day year; seasonal hemispheres reverse; equinox/solstice and high-latitude samples match the approved geometry oracle; Q6's 70-year visibility proof requires a minimum of 3 and maximum of 8 at every place, including intra-cell visibility boundaries.
Exact same sky state on every proof build; fast display light changes no physical temperature/daylight.

**On the phone:** watch one place at dawn/noon/dusk/night in summer and winter, then visit the opposite hemisphere and polar ice.
**Risk:** the adjustable orbit family is structurally compatible with Q6, but no fitted parameter set yet proves the 3–8 range everywhere. Fixed phase alone can cause repeating artifacts; fit inclination/precession/apparent sizes, then certify geometry. Report an unsuccessful fit without changing the requirement.

### α3.6b Hourly weather

**Goal / architecture:** storms move and deliver the climate's rain; A7.10, A7.14.

**Tasks:**
1. `T3.6b.1` **Weather layer (`WLD-16`, `WLD-22`, `TIM-16`).** Hourly cell fields, storm objects, bounded persistent anomaly state and keyed births/decay.
2. `T3.6b.2` **Swept rain and lightning (`WLD-16`, `WLD-30`).** Real wind speeds, integrated crossed-cell exposure, snow/rain partition and natural lightning records with causes.
3. `T3.6b.3` **Display/history integration (`PRE-29`, `PRE-30`, `PLT-07`).** Saved trajectories and phase, bracketing snapshots at displayed time, climate-envelope inspection and 20-year diagnostics; implement G4b natural storm/lightning/drought/harsh-winter rate measures, storm-day and dry-spell checks.

**Tests / done when:** multi-cell storm path wets every intersected cell in its integrated footprint, wraps longitude and never crosses the polar boundary; no rain without its source.
20-world/20-year G4 weather averages pass before alpha completion, or stay visibly failed pending tuning; no per-year quota repair.
Save during motion resumes identically; skipped snapshots cannot show a future storm at the old displayed time.

**On the phone:** follow an upwind storm through a valley, inspect hourly rain and snow, and run the climate-average report.
**Risk:** instantaneous cell hopping and incorrect seasonal units can both look plausible while failing the simulation.

## α3.7 Water and soil in motion

### α3.7a Water, snow and floods

**Goal / architecture:** water stores respond and downstream water arrives later; A7.11.

**Tasks:**
1. `T3.7a.1` **Normal daily water (`WLD-17`, `WLD-12`).** Infiltration/groundwater/spring buckets, snow/melt and lake storage, exact residual accounting.
2. `T3.7a.2` **Reach routing and flood promotion (`WLD-17`, `TIM-18`).** Saved delayed arrivals, fixed hourly active-flood routing, connected floodplain wetting and dated silt intents.
3. `T3.7a.3` **Generated water scenes (`RES-21`, `RES-18`).** Extract real valleys, test spring-fed versus runoff-only streams and snowmelt with their actual rules; add naturally occurring flood rates to G4b; injected-rain unit tests remain labelled diagnostics.

**Tests / done when:** no negative store or unaccounted water; daily↔hourly transitions conserve in-flight volumes; natural upstream storm scene reaches its declared downstream lag without an inserted fixed delay.
A spring-fed stream outlasts the matched runoff-only one over the registered dry-spell interval; snow loss equals melt input after allowed sinks.
The 21.6–26.4 h diagnostic interval follows the general 10% rule, not a universal river transit time.

**On the phone:** watch rain upstream, skip forward to the later downstream flood, and compare a spring-fed stream through a dry season.
**Risk:** daily topological routing must not deliver rain across the whole continent at once.

### α3.7b Sea ice, soil changes and water quality

**Goal / architecture:** coasts, soil and cleanliness have the shared physical state later life needs; A7.11.

**Tasks:**
1. `T3.7b.1` **Sea and ice state (`WLD-26`, `WLD-17`).** Five-day sea warmth and daily sea/freshwater ice, fixed sea level, depth and upwelling capacity; explicit safe-thickness query for later bodies.
2. `T3.7b.2` **Soil transitions (`WLD-27`).** Harvest removal/rest/ash/dung/waste/silt inputs through one bounded fertility rule and fixed cadence; fields store their own fertility in area deltas over the cell baseline.
3. `T3.7b.3` **Water-quality API (`WLD-17`, `WLD-15`).** Downstream source/carcass/crossing influence, warm small pools, removal expiry and ash contamination; test event feeds and inspections.

**Tests / done when:** warm/cold opposite ocean-coast fixtures differ; upwelling/shallow water raises the specified capacity, not an invented fish population; sea level never oscillates as a tide.
Repeated harvest-input years lower fertility, rest restores it and additions raise it within 0–5; two fields in one cell can differ without mutating each other.
Sea and freshwater ice respond on intervening days between five-day sea-warmth updates, including save/reopen halfway through that cycle.
Contamination follows ≤about 1 km river distance and expiry, never jumps upstream.
Actual crops, shellfish recovery and gut sickness remain later consumer checks.

**On the phone:** inspect winter ice, a spring versus downstream water, and the soil response graphs.
**Risk:** API tests demonstrate state transitions, not living yield or disease results.

## α3.8 Hazards and lasting changes

### α3.8a Natural faults and volcanoes

**Goal / architecture:** rare events arise only where generated geology permits; A7.12.

**Tasks:**
1. `T3.8a.1` **Quake hazards (`WLD-15`, `WLD-22`).** Per-kind annual rates, quiet-until state, keyed opportunities and fault intensity/rockfall intents.
2. `T3.8a.2` **Eruption sequence (`WLD-15`, `WLD-27`).** Days of warning, lava material/burial route, windblown ash and season-long water fouling; fixed heights and no new obsidian.
3. `T3.8a.3` **Rate and cause report (`RES-13`, `RES-05`).** Twenty generated worlds for 100 years, rates by type/eligible area, quiet intervals and saved warning/active/cooldown phases.

**Tests / done when:** every event references an eligible fault/volcano; every quiet interval holds; event-rate shares meet `RES-13` once sufficient opportunities are counted; no height or obsidian delta exists.
Natural runs prove rates; forced diagnostic eruption shows the consumer intents but is not counted as natural occurrence.

**On the phone:** inspect a volcano's warning in a saved natural run, follow its lava/ash and see the cooldown record.
**Risk:** missing M4/M5 damage consumers must not be reported as tested shelter collapse or death.

### α3.8b Disturbance logs and area catch-up

**Goal / architecture:** returning late gives the same changed ground, whether or not it was watched; A7.12–A7.14.

**Tasks:**
1. `T3.8b.1` **Delta/log format (`WLD-12`, `PLT-10`, `PRN-15`).** Area changes, dated disturbance payloads, cursor/index and retention independent of public-history thinning.
2. `T3.8b.2` **Daily catch-up (`WLD-12`, `WLD-13`).** One routine for inactive kept areas using seasonal usual weather and day-keyed chance; display catches up copies only.
3. `T3.8b.3` **Proof and old-area report (`RES-05`, `PLT-07`).** Daily versus delayed/camera-only replay, exact once-only cell ledger and saved cursors; byte-growth diagnostics before choosing compaction.

**Tests / done when:** 20 changed-area scenes updated daily or once after a season match exactly; arbitrary camera visits leave authoritative saves unchanged; no resource/depletion input is applied twice.
A disturbance older than 25 years still reaches a dependent area; an area with a remaining buried effect is not discarded.
M4 adds actual object timers/marks to the same routine and reruns the proof.

**On the phone:** leave a changed test patch, watch a season elsewhere, return and compare daily/delayed versions.
**Risk:** a renderer that “helpfully” commits catch-up would make observation alter history.

## α3.9 Choosing a world

### α3.9a Candidate ranking and start certificates

**Goal / architecture:** choose good worlds without repairing them; A7.8.

**Tasks:**
1. `T3.9a.1` **Two-pass search (`WLD-10`, `WLD-11`).** Ordered 20-candidate coarse pass, six initial finalists and deterministic fallback to 40; versioned scores, total ties and logged rejection reasons.
2. `T3.9a.2` **Start certificate (`WLD-24`, `WLD-10`, `WLD-30`).** Full-resolution shelters/water/stone, season food margin with overlap correction, start-landmass arc resources and Q1's species evidence status.
3. `T3.9a.3` **Seed survey (`RES-09`, `RES-05`).** One hundred root seeds, candidate pass counts, proposed G3 definition, own-seed refusal, score distribution and same-three reproduction.

**Tests / done when:** every offered world passes all available hard gates; ≥25% of the declared candidate denominator qualifies; every root seed replays the same ordered result under the same version; no post-score terrain/resource changes.
Force the 0/1/2/3-result and fallback paths with test inputs, separate from natural qualification statistics.

**On the phone:** inspect the ranked candidates, their short summaries and reasons a rejected candidate failed; enter a world seed.
**Risk:** qualification by coarse habitat alone would be a false full-resolution certificate.

### α3.9b Choosing, settling and world lifecycle

**Goal / architecture:** a usable New World path and honest staged settling; A7.9, A7.14.

**Tasks:**
1. `T3.9b.1` **Three-globe choice (`WLD-10`, `PRE-29`).** Show offered previews, one-line fact summaries, choose/top-ranked pick/explicit seed, cancel/resume and no-result explanation in both orientations.
2. `T3.9b.2` **Ten-year settling lifecycle (`WLD-08`, `WLD-11`, `TIM-14`).** Implement the prehistory origin/offset without negative events in today's zero-frontier world; 600-day water/current-system settling, fixed start retained and available gates rechecked.
3. `T3.9b.3` **Saves and honest scope (`PLT-07`, `PLT-08`, `WLD-24`).** Selected candidate becomes one saved world; temporary finalist storage bounded; header records pending ecology/bands and Q1's acceptance status.

**Tests / done when:** cancel/reopen preserves existing worlds, choice never regenerates a different candidate, uninterrupted/resumed settling matches and history begins on the correct spring day.
Carry weather stores, quiet times and arrivals through the history boundary without redrawing chance.
No area is built during world generation; visible areas may be requested after opening.
Full living settling and post-settled food/bands remain open per Q1.

**On the phone:** make a world, choose it, watch labelled settling, save/switch/export it, and replay its seed.
**Risk:** restarting at Year 1 must not erase hazards or make world generation depend on time spent viewing the three choices.

## α3.10 One land at every zoom

### α3.10a Generated land from globe to cliff

**Goal / architecture:** replace α2.9a/α2.10a fixture inputs with generated surfaces and revisions; A8.8, A7.13–A7.14.

**Tasks:**
1. `T3.10a.1` **Fine relief and surfaces (`PRE-03`, `PRE-23`, `WLD-12`).** Metre picture detail constrained to world river/coast/cave/layer anchors; common normals/water/receiver geometry.
2. `T3.10a.2` **Published revisions and bounded jobs (`WLD-13`, `PLT-04`).** Owned immutable base/state, current revision manifests and stale-epoch rejection; coarse parents and measured upload headroom.
3. `T3.10a.3` **Shared overview and whole zoom (`WLD-02`, `PRE-26`, `PRE-29`, `PRE-30`).** Coasts, relief, real cover, rivers, water/ice/weather at the proper bands, physical light and inverse selection at every stop.

**Tests / done when:** 10,000 border samples match, generated rivers never break, no black hole during cold pinch, parent/fine forms retain selected focus and fixed camera; minimum river visibility holds.
Skipped snapshots, cache deletion and rapid world swaps never publish stale land or alter digests.
Full detail arrives within G7's limit on the phone, not merely in cloud screenshots.

**On the phone:** one continuous journey from globe to an unvisited cliff, pan over a river edge and switch worlds mid-load.
**Risk:** unique near-resolution terrain colour explodes memory; shared art and small disposable pieces are mandatory.

### α3.10b Caves and the geological slice

**Goal / architecture:** the chosen line exposes real underground records; A8.9.

**Tasks:**
1. `T3.10b.1` **Generated shelters (`PRE-24`, `WLD-09`).** Materialize certificate geometry into M2's floor/back/roof/opening contracts; deep-cave entry/exit preserves exterior focus.
2. `T3.10b.2` **Section query (`PRE-25`, `WLD-17`, `WLD-27`).** Arbitrary chosen line, seam-safe distance, analytical feature intersections, rock contacts/soil/caves/water head, labelled scale and section width.
3. `T3.10b.3` **Buried-record adapter and review (`PRE-25`, `PRE-31`).** Show true-depth test records through the future trace interface, clearly marked synthetic; no claim of a simulated 200-year camp.

**Tests / done when:** a cliff face and intersecting section show identical contacts; cave openings/floor/roof and water-table values match queries; seam lines use correct local distance; zero-length/very long cuts are bounded/refused clearly.
All included records obey the displayed section width and retain stored depth; repeated cutaways change no state.

**On the phone:** draw a line through a cliff/cave and river, inspect the layers and water, enter the cave and return.
**Risk:** roof reveal is not a geological slice; coloured rock bands are not saved archaeology.

## α3.11 Fit the phone

### α3.11a Generation and steady-world performance

**Goal / architecture:** measure the costs before claiming the targets; A18 additions.

**Tasks:**
1. `T3.11a.1` **Cost ledger (`PLT-04`, `WLD-11`).** Stage wall/core time, full/fallback search counts, physical field/feature/scratch/cache/save bytes and worst-case overflow reporting.
2. `T3.11a.2` **Measured optimization (`WLD-12`, `WLD-13`, `RES-05`).** Remove duplicate arrays/serializations, improve ordered kernels and exact unchanged-cell work; no camera-dependent cadence or timed candidate cutoff.
3. `T3.11a.3` **Worst-case scenes (`WLD-17`, `WLD-15`, `PLT-10`).** Many basins/reaches, large floods/ash footprints, old changed-area logs and snapshot/export overlap; compare optimized/reference digests.

**Tests / done when:** every optimization gives identical state to its reference; counts/allocations cannot grow without a declared bound or memory notice; report normal daily and flood-day world-layer cost against 0.2 core-s/day.
Cloud numbers only guide work; no phone pass is claimed here.

**On the phone:** open the generation/world cost report and run the short timing sampler.
**Risk:** frequency times two million cells dominates even when individual rules are cheap.

### α3.11b Held-phone, streaming and recovery route

**Goal / architecture:** measured end-to-end performance on the actual phone, with M2 visuals active; A18, A17.

**Tasks:**
1. `T3.11b.1` **Phone route (`WLD-11`, `PLT-04`, `RES-05`).** G7 fixed roots, fallback cases and final hashes after at least three minutes of load, unplugged/in flight mode; report median/p95/max and stage times.
2. `T3.11b.2` **Concurrent view/save stress (`PRE-03`, `WLD-13`, `PLT-07`).** Twenty-minute generated-world pan/pinch/weather run, cold area requests, cache churn, periodic saves, switch/reopen/export and frame/heat/memory ledgers.
3. `T3.11b.3` **Recovery and corpus (`PLT-08`, `PLT-09`, `RES-05`).** One hundred random kill points in cloud, corrupted chunks and valid prior-state recovery; phone subset and old-world reopen, retained M1 proofs.

**Tests / done when:** G7 numbers are present as met/missed; phone/cloud state hashes match; every frame minute and memory peak reported; unavailable sensors explicitly unavailable.
Warm open ≤3.3 s approved outer limit under Q7, target 3 s; no catch-up silently changes history.
Repeat affected renderer route on the named weaker M2 phone where available; its absence is reported, not invented.

**On the phone:** run the one-tap review route and send its code; try the cold globe-to-cliff journey and reopen afterward.
**Risk:** cloud timing or an average frame rate cannot prove held-phone latency and heat.

## α3.12 Prove and review the world

### α3.12a Held-out worlds and final visual repair

**Goal / architecture:** numerical and visual evidence from worlds never tuned against; A17, A8.8–A8.9.

**Tasks:**
1. `T3.12a.1` **Closing seeds (`WLD-08`, `WLD-09`, `WLD-10`, `WLD-14`, `WLD-16`, `WLD-24`).** Freeze build/config and draw new recorded seed sets; run G1–G6 with checkpointed reports and failed-world exports.
2. `T3.12a.2` **Phone contact sheet (`PRE-31`, `PRE-03`, `PRE-25`, `PRE-29`).** Every stop at noon/dusk and full-moon night, one landscape, cliff/cave/section, coast/river/lake, polar seam and panning/pinching clips; real weather/seasons, moving cloud shadows and visible currents; retain the approved M2 PRE-31 route.
3. `T3.12a.3` **Fix and preserve gates (`RES-09`, `RES-05`).** Repair faults, rerun affected tests and new held-out tests after tuning; retain failures and unchanged thresholds in the report.

**Tests / done when:** applicable G1–G7 gates pass without weakened rules, owner judges land credible, all new features have a cause/data inspection and held-out results name their actual counts.
Missing ecology/bodies/archaeology claims remain in the explicit Q1 ledger.

**On the phone:** compare the worlds and contact sheet, follow rivers to their destination and inspect the geology.
**Risk:** selecting only attractive screenshots hides generator failures; include fixed random seeds and every worst diagnostic case.

### α3.12b Independent review and M3 close

**Goal / architecture:** an accepted milestone and the next detailed plan; existing A17/A18 and `PRC-09`.

**Tasks:**
1. `T3.12b.1` **Fresh milestone critic (`RES-05`, `RES-09`, `RES-22`).** Independent subagent gets original approved requirements/plan, diff and test evidence, draws its own affected views, checks requirement claims and threshold changes; fix findings.
2. `T3.12b.2` **Milestone report (`RES-06`, `PRN-16`, `PLT-04`).** Added work, numerical and phone results, every principle's check, costs, risks and explicit deferred obligations; plain things for the owner to try.
3. `T3.12b.3` **M4 handoff and delivery (`PRN-14`, `RES-22`, `PRC-11`).** Plan M4 in detail with A9, the single ecology/catch-up/trace interfaces and Q1 closure checks; deliver 41202 and request owner's stage acceptance.

**Tests / done when:** final delivery check and independent review approve; report links open; owner accepts M3 under the agreed scope and M4's plan.
A report or this research critic is not the independent implementation milestone review.

**On the phone:** make and choose a world, tour it through weather, slice its ground, read the short result report and decide whether to close M3.
**Risk:** closing on a design alone; the build, phone evidence and owner review are all required.

## Acceptance matrix: answered decisions and remaining implementation definitions

The numerical rules from PROJECT remain binding.
Where PROJECT gives “about,” the default tolerance is 10%; the table states a stricter nominal target and any proposed interpretation rather than silently replacing it.
Q1–Q10 decisions below are approved as of 8 October 2026; remaining detailed thresholds, reference tables and scene definitions must be frozen under `RES-09`/`RES-22` before tuning.
Failures of chance tests follow `RES-13` fresh-seed reruns; never use repeated retries to discard the failed sample.

| Gate | Data / exact proposed check | Runs and budget | Task owner |
|---|---|---|---|
| G1 Terrain (`WLD-08`, `WLD-09`) | Final 100-seed land/tilt range-and-spread check from α3.2a, then each terrain world: ≥95% of independently enumerated river reaches with contributing area ≥50 km² terminate at sea/lake; no routing cycles; land-cell median slope 0.5–5°, <1% >30°; lakes 1–3% of land; no coast run >22 km whose contour stays within 500 m of its endpoint chord. Primary slope: central differences at 1 km, land-only, permanent cap excluded; report including cap too. Fine 25 m/1 m slopes and cliff footprints separately, never substitute them after a failure. Land denominator excludes sea; lake fraction = freshwater lake footprint / (dry land + freshwater lakes), excluding permanent ice. | 20 fresh full worlds, all meet geometric gates; initial 2 session-hour cap, checkpoint and report overrun | T3.3b.3, T3.12a.1 |
| G2 Geology (`WLD-09`, `WLD-14`) | Every range/fault/volcano refers to its causal boundary/province and falls within its recorded influence envelope; chalk/limestone have marine/basin provenance. Every flint/chert/gravel, hammer/grinding stone, obsidian and copper occurrence passes its host/exposure/provenance predicate; matched lithology/size stones never become less rounded with greater routed transport distance. Inspect independently built expected-host tests, not only call generator predicates again. | 20 worlds for rock, 100 for deposits; 4 session-hours; no invalid occurrence | T3.2b.3, T3.4b.3 |
| G3 Candidates/start (`WLD-10`, `WLD-24`) | 100 root seeds; ≥25% qualify in aggregate among the first 20 candidates of each root using full-resolution qualification in the offline audit (2,000 candidates), with per-root results also reported, so the denominator cannot be improved by counting only finalists. Every offered world passes final gates; same seed/version gives identical ordering. Q5 approved food margin ≥20%, shelters ≥2 m²/person, cold mean 2–10°C, water ≤2 km, accessible food/stone ≤10 km; physical/food gates recheck selected start after real settling as available. Frost “few” proposed 2–5 nights per cold season. | 100 roots and paired determinism replay; 8 session-hours initial cap, may require owner-approved larger audit budget; full biological confirmation remains Q1 | T3.9a.3, T3.9b.3 |
| G4 Climate/weather (`WLD-16`) | 20 worlds ×20 years, every place's accumulated rain within ±10% of its climate and mean temperature ±1°C; online per-cell accumulators, not selected attractive locations. For near-zero rainfall use an explicitly approved absolute rounding allowance of one stored precipitation unit over the full run, report all such cells. Earth matched-bin warmth target ±2°C (about: outer ±2.2°C), ≤10 percentage-point climate-class share difference in matched 10-degree bands as answered Q3; rain-shadow/coastal tests independent of that comparison. | 2 session-hours initial cap; fixed natural runs, checkpointed, failed chance checks rerun per RES-13 | T3.4a.3, T3.6b.3 |
| G5 Sky/hazards (`WLD-07`, `WLD-15`, `WLD-22`) | Four full moons/year; approved sky oracle error ≤0.1° direction and ≤2 minutes daylight in non-singular test cases, explicit polar cases. Eclipse Q6 requires 3–8 visible solar/lunar events at every place over 70 game years, clouds or not: 100 stratified places for diagnostics, then every canonical world-cell location plus certified bounds over intra-cell horizon/footprint crossings; report spatial minimum/maximum and fail any uncertified region. The complete 100-year runs record zero polar-barrier crossings by every implemented transport consumer. Every quake/eruption has valid location, origin and cooldown; per-kind event rate judged by RES-13 over ≥1,000 opportunities, using the promised share tolerance (rare shares half-to-double). | 20 worlds ×100 years for hazards; 4 session-hours; use analytical alignment/visibility queries for the eclipse proof rather than run unrelated ecology; initial 2 additional session-hours for full spatial coverage, checkpoint/report any overrun | T3.6a.3, T3.8a.3 |
| G6 Area/water/slice (`WLD-12`, `WLD-13`, `WLD-17`, `PRE-25`) | 10,000 shared edges exact (stronger than ≤0.55 m outer tolerance); unchanged same-date regeneration identical; 20 kept-area scenes daily vs after season exact; 20 natural generated-valley scenes, at least 16 meet declared flood/spring behavior, rerun as RES-13 says. Section contacts/water/depth match source values exactly at sample points; zero camera-caused save changes. | 2 session-hours; small deterministic kernels in quick checks, longer natural scenes background | T3.5b.3, T3.7a.3, T3.8b.3, T3.10b.3 |
| G7 Phone (`WLD-11`, `PLT-04`, `PRE-03`) | Held speed after ≥3 minutes load, unplugged: 20 ordinary root searches plus 5 saved roots known to use fallback, fixed before measurement; all target ≤180 s and outer ≤198 s under answered Q7. Selected settling target ≤60/outer66 s, ecological/band completion later. Areas target ≤0.1/outer0.11 s; visible detail ≤1/outer1.1 s; warm open ≤3/outer3.3 s. ≥97% frames on time at the selected 60/30 fps mode, none more than 50 ms late (maximum frame duration 66.7/83.3 ms respectively), every minute of the 20-minute route; world layers ≤0.2 core-s/game day nominal, plus actual world-alone throughput ≥10 game years/minute on two middle cores for the current systems. M4 repeats with full ecology. Peak process ≤2 GiB under answered Q8. GPU/heat lines retain M2's approved route. Every physical digest matches cloud. | Approximately 20-minute play route; 25-search generation suite separately about 100 minutes at nominal limits or 110 minutes at outer limits including one selected settling run per root, plus setup; resumable between roots. Do not conceal it in the promised short review route. | T3.11b.1–3 |

The 20-world weather gate quantifies an unusually strong **every place** promise.
If it fails from stochastic variance or the Earth protocol cannot fit the small world, report that evidence to the owner before changing quantifiers, tolerances or climate structure.
No literature result guarantees this acceptance rate.
Whole-world event runs use no player interventions or test switches.
Test-only injected events prove mechanisms, with reports carrying their switch, never natural frequency (`RES-18`).

**G1 audit denominator:** derive all channels draining at least 50 km² from the final drainage graph, independently of the renderer or generator's river-registration threshold. Split reaches only at headwaters, confluences and terminal water bodies, with segmentation frozen before tuning. Missing registered geometry is a failure, not a removed denominator entry.

**G4b natural rates (`WLD-16`, `WLD-22`, `WLD-30`):** T3.6b.3 and T3.7a.3 own a shared suite for storms/storm days, lightning, droughts, naturally triggered floods and harsh winters, each by climate. Reuse G5's 20-world/100-year runs where possible, with 2 additional session-hours initially for reference preparation and diagnostics; retain checkpoints and seek a larger budget if necessary. Freeze per-climate Earth targets, event thresholds, storm identity/de-duplication, exposure denominators and short-year conversion before tuning. Every event must trace to its natural system; no injected weather counts. Apply RES-13 to each promised rate with at least 1,000 eligible cases per tested kind/climate, report uncertainty and missing coverage rather than pooling away a failing climate. Wildfire rate and lightning-to-fire conversion remain M4 under Q1.

Use the approved Q3 wet-day threshold (1 mm/day) and report each place's mean seasonal storm days and longest annual dry-spell distribution. Approved Q3 definitions: mean storm days within the RES-13 share bounds of the saved climate; the saved longest-dry-spell envelope denotes the 95th percentile of annual maxima, with its observed exceedance share checked against 5% under RES-13. Compare drought/harsh-winter event definitions and flood exceedance thresholds to independent Earth reference definitions, not generator labels. Q3/Q9 approved the dry-day/dry-spell interpretation and unit convention on 8 October 2026; freeze the remaining per-climate reference values and event thresholds before tuning. A correct rain total with wrong persistence must fail. For lightning, total-flash data and ground-strike/fire opportunities are different quantities; conversion needs an explicit sourced rule.

The session-hour allocations are initial caps per suite, not claims about measured runtime.
Record actual core count, CPU time, wall time, version and storage for each.
A 100-year ×20 empty-world run is 120,000 game days; even at 0.2 core-s/day it is 6.67 core-hours, so parallelism and checkpointing matter.
If a suite exceeds its stated allocation, keep its checkpoint and raise the needed budget; do not shrink its sample and call the original check passed.
Train on fixed named seeds, close on new seeds; candidate-quality tests deliberately audit candidates not only selected winners.

## Coverage and remaining acceptance

| Item group | Delivered mechanism in M3 | Full check that remains elsewhere under Q1 |
|---|---|---|
| WLD-01/02/03/06 | Grid/barrier, real overview/disc input, dimensions/tilt/land | Future walker/herd/fire consumers must rerun no-cross checks in M4/M5 |
| WLD-07 | Sun/moon/stars/eclipses | Learning sky cycles in later culture |
| WLD-08/09 | Whole causal generator and 600-day lifecycle; physical systems settling | Full cover/herds play-rule settling in M4; no false claim of already complete ecology |
| WLD-10/24 | Rejection/ranking/start certificates and available physical/initial-species gates | Actual food/yield sufficiency and settled species occurrence M4; naked-band winter survival M5 |
| WLD-11 | Full generation-to-choice timing and current-system settling | Full ecological settling timing M4; bands-included time M5, rechecked each stage |
| WLD-12/13 | Layers, paces, pure queries, deltas/catch-up and immutable view publication | M4 plants/things/timers and M5 near-person activation add consumers to the same proof |
| WLD-14 | Geologic deposits and stable patch identity/exposure | Digging/gathering creates real things in M4/M5 |
| WLD-15/22 | Natural hazards, rates, warning/quiet times and effects records | Things/plants/fire M4; injuries/deaths M5; god controls M9 |
| WLD-16 | Fixed climate and live hourly weather | Powers use its bounds in M9, no second weather system |
| WLD-17 | Water/snow/ice/floods/springs and water-quality inputs | Floating/carrying things M4; fouled-water illness and bodily ice safety M5 |
| WLD-26 | Sea/currents/upwelling/ice and habitat capacity | Actual fish/mammal totals and stripped shellfish regrowth M4 |
| WLD-27 | Soil/fertility rule with actual weather and declared mutation inputs | Actual crop removal/yield/recovery plus dung/waste consumers M4/M10 |
| WLD-30 | Geographic, event-rate, climate-index and initial capacity scaling | Actual foraging 25-person/100–300 km² scene M5; every yield/animal entry M4 |
| PRE-03/29 | One generated landscape and weather from cliff to globe | Later real people/herd/building aggregate consumers at their own milestones |
| PRE-25 | True geological/water section and buried-record adapter | 200-year real hearth/bones/tools/graves from M4 traces, then M5 human-world confirmation |

Every M3-served item above has tasks and tests; a partial acceptance line is deliberately still open.
The principles review must show that caches never create facts, generation never patches a failed candidate, natural events keep their causes, history is stored, and no model output enters the running world's decisions.
The research helper/critic work here does not substitute for the fresh implementation critic at α3.12b.
