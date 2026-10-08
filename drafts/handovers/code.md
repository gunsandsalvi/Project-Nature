# Coding handover — 8 October 2026, wind-down

## Current state and hard limits

Worktree A is `/home/user/Project-Nature/.claude/worktrees/lead-code-a`, branch lead-code-a. All changes are saved, uncommitted. Worktree B is clean and unused. `COMMITS.json` lists every changed A file as one explicitly work-in-progress commit; no file exceeds50MiB. Git is read-only for leads: coordinator commits/merges/pushes. No art files, PROJECT.md or CLAUDE.md were changed. No model names were added to repository files.

Coordinator's14:39 inbox says start nothing new, finish small edits, write this handover/COMMITS, mark READY TO MERGE (WIND-DOWN), end. All agents have finished; no test/build/capture process remains. No review02 was requested. The assignment is NOT complete: α2.8a delivery30801 has not passed the full check and is not ready for the owner. α2.9a through α2.12a are unimplemented; `a29a-plan.md` holds the read-only audit and proposed next interfaces.

## Delivery blocker and saved fix

`CMAKE_BUILD_PARALLEL_LEVEL=4 tools/build.sh 2.8a release` produced a signed30801 candidate, then correctly failed the50MiB cap at53,339KiB. Signature(v2/v3 release key), package/version, API24–36, arm64/16KiB alignment, rotation and no-permissions checks passed. See `a28a/release-build.log`. Build trap deleted that failed temporary signed APK. `dist/kindling.apk` and checksum remain the OLD30301 release (sha256 b580b55cd995c99730b891906577142304b80fca04050d2b34336720c3e55ab4); project.godot/export preset already say30801. Dist note now explicitly warns that30801 is a candidate and linkedAPK is30301. Do not announce it as shipped.

Root cause: catalogue texture payload grew from4.10MB to17.01MB compressed (39→234 entries), plus2.407MB fixture sheets. Those catalogue assets/records support self-checks; do not drop arbitrary files or weaken the size check.

Saved narrow fix, NOT yet independently reviewed or built as a release:
- `tools/apk-compress.py`, `tools/tests/test_apk_compress.py`, `tools/build.sh`: check manifest extractNativeLibs=true, losslessly deflate only Kindling native extension before zipalign and signing. Pinned export flag was verified true. Synthetic test observed failing(copy-only helper did not shrink), then passes; checks entry names/order/raw bytes/metadata and other compression methods. Actual unsignedAPK retained all890 names and uncompressed entry bytes, saving1,941,264 bytes (54,613,352→52,672,088). All other resource bytes unchanged; no C++ compiler flags changed.
- Ground's `game/fixtures/ground.kdsheet` is byte-identical to packed `game/data/sheets/meadow.kdsheet`. Source copy retained. Manifest now names `res://data/sheets/meadow.kdsheet`; atlas resolver validates allowed relative fixture or absolute data/sheets path, rejects traversal/missing/external files; Fixtures inspection uses resolver. Export excludes only redundant fixtures/ground.kdsheet. Should save another1,282,443 compressed bytes, giving about49MiB plus small signing/alignment overhead. This combined size is a prediction: rebuild and verify it.
- New shared-sheet page/path tests failed first, then20/20 Terrain+Fixtures+Crowd game tests pass. APK fixture validator also supports shared sheet path: new13th test failed first, then all13 pass. With compressor test,14 Python packaging tests pass. Format/lint/shell syntax pass. Final export exclusion was added by lead after agent's20-test run; only actual new export verifies it.

Scratch inspection APKs `a28a/inspect-unsigned.apk` and `inspect-compressed.apk` are NOT signed deliveries and MUST NOT be committed. No over-size file is in COMMITS.

## α2.8a corrections and review

The whole inherited7228f47 merge was reviewed against8607855 by coordinator's service. `reviews-a/review-inherited.md`: sevenP2 findings. We fixed all:
1 off-screen actors still cast shadows into visible receivers (bodies collected before sprite cull);
2 all submerged sprites clip inside bounded water footprint, with matching picking/semantic coverage;
3 trunk orders at ground footprint, crown separately uses raised proxy;
4 visible flame touches sampled floor while light emitter stays elevated;
5 moving mask throttle uses monotonic elapsed real time, not accelerated scene time;
6 prepared bed receiver copies follow final z-order;
7 design sheet hidden unless explicitly opened.

Also common sprite height/water helper (`game/fixtures/height.gdshaderinc`), thin waterline contact cue, accurate flame receiverID, deterministic capture clock and metadata. The pink sliver beside birch was through transparent source pixels, not over opaque bark (114 person pixels, zero intersections with bark alpha>.1); do not claim that sliver disappeared.

`reviews-a/review-01.md` is CLEAN,0findings. Reviewer reran9Terrain and12APK-resource tests and validated a fresh Android export. That review covered seven fixes and first export/resource checks. It DID NOT cover subsequent semantic clear-background fix, shared-sheet reuse, compression helper/build hook, final golden/images/version/doc additions. Final semantic golden had failed on cave background07101b yielding unknown objectID1773575; `_draw` now uses black(ID0) outside receivers in non-colour passes. Golden recaptures pass; colour/page unchanged. Request review02 of final uncommitted work (or branch diff if wind-down was committed), and freeze source while it runs.

Main files changed: game/pages/{terrain,fixtures}.gd, game/terrain/{drawing.gd,sprite.gdshader,goldens.json}, game/fixtures/{atlas.gd,manifest.json,identity.gdshader,height.gdshaderinc(+uid)}, game/test/{terrain,fixtures}_test.gd; tools/terrain-{picture,light-check}.gd, tools/{apk-fixtures,apk-compress}.py and tests, tools/{build,verify-apk}.sh; export/version; IMPLEMENTATION/distNOTE and7dist review artifacts. See COMMITS for exact list. No C++ changes were necessary.

## Checks and evidence

All evidence below under this handover's folder unless repository path specified.
- Native rebuilt:72view tests,439683assertions; formatting/tidy pass. Shared `build/view` and `build/sim` RelWithDebInfo clang/ccache. Android arm64 NDKr30 extension built clean in build/view-android. Reuse these builds; disk shared, no duplicate builds.
- All8accepted M1 proof suites match main-build manifest on one/four threads.221compile command flags correct,181objects no FMA/platform math. `a28a/m1-proof-result.txt`, lead-sim-build.log, lead-gamedata.log, terrain-core-{build,probe,tidy}.log, android-build.log.
- Before packaging,19game tests passed idle with0fail/errors/orphans. Six new regressions each seen failing: trunk, flame, bed order, off-screen shadow, real-time throttle, submerged objects. A contention run failed existing crowd timing; idle rerun passed without changing criteria. Final shared-sheet run20tests pass: `a28a/tests-sheet-{before,after}.log`. Initial logs `tests-{before,after,final,inherited-before}.log`. `tests-semantic-before.log` was a discarded exploratory modulation test, NOT a retained regression or fix.
- Actual-renderer light probe checks axes/material/emission/fire/contact/clipping; contact probe failed before fix, passes after: `a28a/renderer-light-check.{json,png}`, lighting-render.log, lighting-water-before.log. Not a phone test.
- Initial12APK tests all observed failing under targeted faults, then green: `a28a/apk-fixtures-tests.log`. Shared-sheet13th before/after logs; compressor fail/pass was observed in agent tool output (no separate failing log saved). Final root rerun14Python packaging tests passes in `a28a/packaging-final.log`.
- File/coverage/note checks passed again at wind-down after final edits:364items,2920citations,1301IDs,95gdUnit4/188Python tests indexed. Rerun as part of final delivery check. Full `tools/check.sh --deliver` has NEVER been run in this turn; do it only after provisional commit, not while code is changing.
- Signing preflight correct; only `tools/signing-key.py` reads secret. Never print or manually read passphrase.

## Captures, critique and honest limits

`a28a/REPORT.txt` is terrain agent's detailed evidence handoff; `a28a/visual-review.md` is fresh critic. Final23timed views undera28a/after; independent still repeats underrepeat; accepted game/terrain/goldens.json covers23views/92page+colour+object+material passes, all exact with mapped codes. Four retained Fixtures views underlegacy preserve16existinggoldens. Six water bed/reflection targets repeat exactly. Logs goldens-record-final.log, goldens-check.log, legacy-goldens-check.log, water-target-check.json. Initial goldens-record.log correctly failed unknown cavebackgroundID; fixed captures supersede it. After cave/cliff-band recapture their JSON says still-only; original timed statistics for all23 survive `a28a/timings.json`.

`a28a/review.mp4` / dist/pictures/a28a-review.mp4:96frames,12fps,8seconds,1080×2400; sun directions→shelter closed/reveal→water→cave enter/exit. `review-clip.json` preserves exact state sequence. Native cave-view restoration tests also pass. Six dist stills include flat/cliff lettered comparisons, shelter/water/cave/receiverdebug. Flat/cliff comparisons baseline UNLIT DIAGNOSTIC fixture, not ownerapproved art. FLIP means.122973/.231432 at80pixels/degree; percentages above.2=5.5843/34.3759. Baseline source pairs available inafter/pictures; grids and JSON in a28a.

Critic prefers lit flat/cliff and fixed contact in both image orders, no new obvious ordering fault. Bed visibly excludes sprites; reflection target contains inverted sprites; nonperson clipping and waterline cue visible. Composite reflections/depth remain faint, water reads green rectangle, ground noisy, cliff plain/dark, repeated colour dropdowns ambiguous. These are explicitly documented roughness. Existing sheet crops/diagram people+animals are developer art, NOT final art/style approval. Old dome shelter crop remains temporary; intended reviewed tent is16.4 cone,4.2m ring/3.1m tips. Coordinator14:12 suggested cheap swap; it was not cheap without regenerated atlas/pivots, so deferred to verified art handoff. Restricted shelter receivers are separate from domecrop.

Cloud renderer Compatibility/Mesa llvmpipe,80measured frames after10settling; frozen60fpsclock/10Hzmasks. Flat noon mean script/viewportCPU/summedGPU4.47/20.57/36.91ms; water4.05/33.72/50.10ms including bed8.81/reflection2.07GPU. These software counters are NOT phone frame times. CPU mask bytes84,450flat/101,350water vs1,352,000cap.32body outdoor median maskCPU2.68–5.50ms. No phone power/thermal/framebudget pass claimed.

## Exact next steps

1 Read PROTOCOL, INBOX and this handover. Determine whether coordinator already made wind-down commit; inspect git readonly. Do not repeat finished tests/builds unnecessarily.
2 Inspect/review final packaging changes, including APK extraction guard and excluded duplicate shared sheet. Request external review02 via reviews-a/request-02.txt (one-line summary); if committed, coordinator must arrange review of committed diff because ordinary service only reviews uncommitted changes. Review01 cannot approve these later changes. Freeze source during review; fix/review findings up to3rounds per protocol.
3 Build signed candidate after review/source stabilization: `CMAKE_BUILD_PARALLEL_LEVEL=4 tools/build.sh 2.8a release > <folder>/a28a/release-build-final.log 2>&1`. This regenerates game/data,imports,exports; avoid concurrent runtime captures. Confirm final APK<=50MiB, fixture/resource validator passes, release signature/version checks pass and checksumupdated. If stilloversize, measure archive rather than drop required records. Existing lossless prediction should have~1MiBmargin.
4 Remove candidate warning/update note only when actual build passes; update IMPLEMENTATION account with final review/size/check evidence. Preserve pendingart/style/phone gates and avoid claiming wholeM2done. Golden content remains valid because shared ground sheetbytesidentical; changeddefaultfixtureTree remainsunchanged.
5 Approved coordinator exception from13:57: write normal COMMITS.json, append READY TO COMMIT (PROVISIONAL): lead-code-a. Coordinator commits only, replies COMMITTED hash(notmerged). Then `CMAKE_BUILD_PARALLEL_LEVEL=4 tools/check.sh --deliver > <folder>/a28a/check-deliver.log 2>&1`. It refuses uncommittedgamefiles; do not weaken check. If wind-down commit already exists, final signedAPK/docs still need propercommit first. Full check runs sevenC++configurations/TSan/QEMU/samebits/scenes/Godot allbench/tools/file/coverage/APK. Only on PASS append READY TO MERGE exacthash, then READY FOR OWNER. Wait MERGED before editing frozenlistedfiles.
6 Owner instructions: install30801(overold), firststartcheck; Terrainflat noon/dusk/sundirection/contact/fire first, then slope/cliff/shelter/water, Selectperson/Reveal/caveentryexit; Fixturespan/pinch/turn/action/facings. Ask flatstyle then terrainapproval, not finalart. Real phone performance requires actualdevice route.
7 Continueα2.9a thenα2.10–12 in order, one mergepoint each. `a29a-plan.md` saves important audit: fractional density and separate sourcefamilies, immutableownedjobs/cachekeys/revisions/cancellation/parents/byteledgers, existingtimepriority resolver gap, normalbasis east/south/up vsengineeast/north/up, materialIDadapter,117artrecords cost125.238MiB ifalleagerloaded. No prototypehistory copying. Artlaneapproval and namedweakerphone remain externalgates; no query has yet selected thatphone.
