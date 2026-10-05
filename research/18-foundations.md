# Research 18: the foundations

**Question:** what must production's first milestone build so that every later one stands on it without rework, for the whole game and for what may come after it?
How do others build the same parts, and what does Kindling take?

Research 03 chose the parts of the simulation core; this note goes deeper into each, with what pre-production measured (`LESSONS.md`).
On 5 October 2026 seven studies looked at the same bits, entities, events and speed, catalogues, saves, the bridge to Godot, and the phone benchmark.
Each read the primary sources (standards, compiler and library source, Android's and Godot's own code) and ran small experiments in the cloud: x86-64 with clang 18 and GCC 13, arm64 under qemu with GCC 13, and the phone's own compiler (NDK r30, clang 21) under qemu.
Their numbers are the cloud's; the phone's are measured by the foundations' benchmark.

## What `PROJECT.md` asks of the foundations

`MIL-08` names them: the app and its delivery; the simulation library; numbers, time, the 60-day year, dates and chance, with the same bits on the phone and in the cloud; entities, activities with an end, and catalogues with their checks; saves, several worlds and export; test scenes, the repeat check and the phone benchmark.
Its items, grouped:

- **The app on your phone** (`PLT-01`, `PLT-03`, `PLT-06`):
  - each alpha installs over the last from the phone's browser, keeping every world;
  - nothing in play makes a network call;
  - the simulation runs only on processor cores, up to the four middle ones, and every time budget is time on one middle core at the speed the phone holds under load.
- **The same bits** (`RES-05`, `TIM-16`, `PRC-10`):
  - the same saved world gives the same result every time, on the phone and in the cloud;
  - every chance draw depends only on its world, system, being, moment and purpose;
  - the repeat check before any work joins: one scene and one benchmark world each run twice, once on one core and once on four with a stop and resume between, and must end identical.
- **Time** (`TIM-14`, `TIM-17`, `TIM-18`):
  - a game year is 60 days in four seasons of 15, a day 24 hours;
  - what takes up to about two weeks in life takes its real time, and longer spans about a sixth, each duration in the catalogues recording both lengths, checked by a test;
  - dates read "Year 112, autumn, day 6", from Year 1, spring, day 1;
  - everything people and animals near people do is an activity with a start and an end, its results landing at its end;
  - an activity ends early only for the one list of interruptions, and keeps what it reached: a walker stands where they got to, what builds up gives its share, work stays in the thing, a single act cut short does nothing;
  - whatever ends at a given moment ends then at any speed, and things ending at the same moment are always settled in the same order.
- **Content as data** (`MAT-05`, `MAT-13`, `MAT-14`, `MAT-17`, `PRN-14`):
  - the game's content is in catalogues written by AI agents and checked by automated tests;
  - each entry stands alone, naming others only as results, never as inputs, and adding one never needs the others changed;
  - the catalogue checks, each proven by a test catalogue with one planted fault for each;
  - every value plausible, keeping the real order of things;
  - every tunable number in a catalogue or a tuning file, never in code.
- **Saves** (`TIM-05`, `TIM-08`, `PLT-07`, `PLT-08`, `PLT-09`, `PLT-10`, `PRN-15`):
  - closing the app saves the world at that moment, and reopening carries on from it, every activity where it was;
  - the present state saved every 30 real seconds and whenever the app leaves the screen, each event joining the history as it happens;
  - a damaged save never loaded; after a crash the world opens at its last save within about 3 seconds and catches up, repeating exactly; killing the app 100 times at random never loses an event;
  - several worlds, each with its seed and the version of its making rules and values;
  - export and import as one file, a damaged file refused;
  - small updates carry worlds on, big ones keep their books of ages readable, and a new version keeps each world's last save until it has run an hour;
  - history thinned by a fixed rule after 25 years, a full world at Year 250 in about 4 GB;
  - the past kept as it happened, never re-run.
- **Tests and measurements** (`RES-06`, `RES-09`, `RES-10`, `RES-12`, `RES-13`, `RES-21`, `RES-22`, `PLT-04`, `PLT-05`):
  - scenes, then whole worlds, under the same rules as play, each with its pass rule stated before its first run and counted over about 20 seeds where chance matters;
  - switch-off runs only in tests; oddities flagged; runs that keep checkpoints and resume as if never stopped;
  - the phone benchmark: one tap, about 20 minutes, speeds at held speed after at least 3 minutes, a short result code;
  - a report at each stage's end, and your review.

## What the later milestones will ask of the core

The foundations are built once, so they must already fit what comes after them (`PRN-09`, `PRN-14`):
- **The graphics engine** (M2) draws a snapshot each frame and never feeds anything back (`WLD-13`).
- **The world** (M3): about 2 million world cells and 20,000 weather cells on a torus, each layer at its own pace on one world clock; areas made on demand, the same from the seed, the cell and the date however late or often (`WLD-12`, `WLD-13`); world generation in parallel, ending the same everywhere.
- **Things and living nature** (M4): hundreds of thousands of things, each with timers that change pace only when moved or when a condition changes step (`MAT-19`).
- **People and minds** (M5, M6): thousands of people choosing at each activity's end; talk, calls and interruptions between people at the same moment; family trees and graves naming the dead for centuries.
- **Crafts** (M7): catalogue checks walking the whole web of blueprints, and trials running a blueprint thousands of times headless (`RES-24`).
- **The game** (M9): the director and the book of ages read the stream of events and never write to the world (`TIM-03`); speed set by pause, skip, the dial or lock, the director and zoom (`TIM-15`), from one game second a real second up to top speed (`TIM-01`, `TIM-10`).
- **The whole arc** (M10): 20 worlds to Year 30 and 10 to Year 250, headless in the cloud, stopped and resumed across sessions (`RES-07`, `PLT-05`).

## What may come after the game

- **Later layers,** such as bronze or writing, extend the launch catalogue (`VIS-03`, `PRN-14`): no rule may name an era, a species or an item, and every list the game reads grows by adding entries.
- **More people and more cores:** identifiers, counters and parallel work must not assume today's numbers, and results must never depend on the number of threads.
- **Content from elsewhere,** as mods are in other games: catalogues from more than one source, in order, each recorded in the world that uses it.
- **Long-lived worlds,** saved by one version and opened by later ones: every save carries its versions, every format its migrations.

## How others do it, and what we found

### 1. The same bits everywhere

- **The basic operations are safe; compilers and libraries are not.** IEEE 754 rounds + − × ÷ and √ correctly, so they give the same bits on both chips.
  But "It is incredibly naive to write arbitrary floating point code in C or C++ and expect it to give exactly the same result across different compilers or architectures" ([Gaffer on Games](https://gafferongames.com/post/floating_point_determinism/)).
- **Fused multiply-add is on by default.** Experiment: clang 18 for arm64 and the phone's clang 21 fuse `a*b+c` within one expression; GCC 13 even fuses across statements in C++.
  Clang 21's manual: "This permits operation fusing, and Clang takes advantage of this by default (on)" ([clang 21](https://releases.llvm.org/21.1.0/tools/clang/docs/UsersManual.html)); GCC 13: "The default is -ffp-contract=fast" ([GCC 13.3](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Optimize-Options.html)).
  `-ffp-contract=off` stops all of it, but must be the last floating-point flag: `-ffp-contract=off -ffp-model=precise` silently fuses again, since precise means "FP contraction (FMA) is enabled".
- **One copy of an inline function wins at link time.** Experiment: a header function compiled with `-ffp-contract=off` in `sim/` and with defaults in `view/` returned the fused result in the simulation when `view/`'s object came first.
  So every file that includes the simulation's headers needs the same flags, and the check must read the built binary.
- **Flush-to-zero is inherited.** "The floating-point environment shall be inherited from the creating thread" ([POSIX](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_create.html)); Godot's raycast module turns flush-to-zero on for x86 ([Godot](https://raw.githubusercontent.com/godotengine/godot/master/modules/raycast/raycast_occlusion_cull.cpp)), and what the phone's graphics driver does is unknown.
  So each simulation thread sets the default environment itself and asserts it.
- **The chips disagree on what the standard leaves open** (experiments): the bits of a NaN (`0xfff8…` on x86-64, `0x7ff8…` on arm64); out-of-range float-to-integer casts (−2147483648 against saturation); whether plain `char` is signed; `fmin` and `fmax` with signed zeros; the order in which GCC evaluates arguments on x86-64; and `long double`, 80-bit on x86-64 and 128-bit on arm64.
  Clang also folds `sin` of a constant with the build machine's own library, so a folded value differed from the phone's run-time one.
- **The standard libraries disagree.** The cloud's libstdc++ and the phone's libc++ differ in `std::sort`'s order of equal keys, `partial_sort`, `nth_element`, `unordered_map` order, `std::hash`, `<random>`'s distributions, `std::shuffle` and `std::reduce`.
  "The algorithms for producing each of the specified distributions are implementation-defined" ([C++ draft](https://eel.is/c++draft/rand.dist.general)); libc++ even randomizes tie order on demand, since "Google has measured couple of thousands of tests to be dependent on the stability of sorting and selection algorithms" ([libc++](https://libcxx.llvm.org/DesignDocs/UnspecifiedBehaviorRandomization.html)).
  `stable_sort`, `mt19937` and the parsing and printing of a million decimal strings agreed.
- **The platform's maths library disagrees.** Over 200,000 inputs each, glibc and the phone's bionic differed in 3% of `sin`, 18% of `atan2` and 50% of `cbrt` results, and glibc changes results with the CPU's FMA unit.
  glibc "does not aim for correctly rounded results for functions in the math library" ([glibc](https://sourceware.org/glibc/manual/latest/html_node/Errors-in-Math-Functions.html)), and the phone's library ships with Android and changes with its updates ([bionic](https://android.googlesource.com/platform/bionic/+/refs/heads/main/apex/Android.bp)).
  Factorio: "We internally use a custom implementation of most math functions because different compilers/operating systems implement them slightly differently" ([Factorio forum](https://forums.factorio.com/viewtopic.php?p=380343)).
- **Correct rounding makes the answer unique.** CORE-MATH's functions are correctly rounded, MIT-licensed, one C file each ([CORE-MATH](https://core-math.gitlabpages.inria.fr/)): "the output bit pattern for any given input is mathematically unique" ([LLVM libc](https://devblogs.microsoft.com/cppblog/bringing-correctly-rounded-math-to-production-with-llvm-libc/)).
  Experiment: CORE-MATH gave identical bits on all four builds over 3 million values, agreed with LLVM libc's correctly rounded headers on every value, and ran at 5–35 ns a call; P5's own series were identical everywhere but up to 184 ulp off for sine and 316 for power.
- **The phone's exact compiler output runs in the cloud.** Experiment: static executables built by the NDK's clang 21 run under `qemu-aarch64-static`, threads included, so the cloud can test the very code the phone runs; the phone stays the final proof, since qemu emulates the floating-point unit in software.
- **Parallel sums depend on how work is cut.** Experiment: a sum split by thread count changed its last digit with 1, 2, 3 or 4 threads; cut into fixed chunks and added in chunk order, all agreed.
  Factorio's 2024 desync depended on "the number of cores" and was found "by artificially pretending I had less CPU cores than I did" ([FFF 415](https://factorio.com/blog/post/fff-415)).
- **Checksums:** xxHash "produces identical hashes on all platforms" ([xxHash](https://github.com/Cyan4973/xxHash)); XXH3 ran at about 21–30 GB/s, against 0.8 for FNV-1a.

### 2. Numbers

- Factorio's positions are "a fixed-size 32 bit integer, with 8 bits reserved for decimal precision" ([MapPosition](https://lua-api.factorio.com/latest/types/MapPosition.html)); OpenTTD's vehicles hold `int32_t` coordinates ([vehicle_base.h](https://raw.githubusercontent.com/OpenTTD/OpenTTD/master/src/vehicle_base.h)).
- The world is 2,000 km = 200,000,000 cm across, within a 32-bit integer by a factor of 10.7, while a float's step at that distance is 16 cm.
- A game year is 5,184,000 s, so 32-bit seconds run out after 414 game years: about 3 real hours at top speed.
- A probability written as "0.413" becomes the whole number ⌊0.413 × 2⁶⁴⌋, read exactly from the text, and a 64-bit draw fires below it: no float, no rounding question.

### 3. Entities

- **EnTT v4.0.0** (22 July 2026, MIT, C++20) keeps each component in its own pool of packed arrays, with views, groups and 32-bit ids of 20 index and 12 version bits ([EnTT](https://github.com/skypjack/entt/blob/v4.0.0/docs/md/entity.md)).
  Measured: adding and removing a tag on a rich person 13 ns (flecs moves the person between tables in about 3 µs); creating a thing 91–175 ns; looping 2–18 ns an entity.
- **Its order is not canonical.** It is the same every run of one build, but shifts with unrelated changes: a view "is iterated along the pool that contains the smallest number of elements", and removal moves the last element into the hole.
  An order fuzzer that scrambled the pools before each batch caught an order-dependent rule.
- **Its snapshot loader has a bug:** the documented `orphans()` step re-releases free ids, so a reopened registry hands out different ids from the saved one.
- **Identity:** EnTT recycles ids, and its version wraps after 4,095 reuses, when a stale handle becomes valid again.
  Factorio's `unit_number` is "A unique number identifying this entity for the lifetime of the save ... not re-used" ([Factorio](https://lua-api.factorio.com/latest/classes/LuaEntity.html)); Bevy advises to "insert a secondary identifier as a component" ([Bevy](https://docs.rs/bevy_ecs/latest/bevy_ecs/entity/struct.Entity.html)); Dwarf Fortress keeps the dead as historical figures ([wiki](https://dwarffortresswiki.org/index.php/Historical_figure)).
- **Memory:** 7,000 people on recycled ids scattered among a million things took 265 MB, against 105 MB in two separate registries.
- **Parallel:** workers that only read and fill per-chunk command lists, applied in chunk order, gave the same checksum on 1, 2 and 4 threads, under clang and GCC, with the fuzzer on.
- **Content as data:** RimWorld creates "a new instance of each compClass" a Def lists ([RimWorld](https://rimworldwiki.com/wiki/Modding_Tutorials/ThingComp)); Space Station 14's data fields "are written and read to and from yaml, but are also used for copy, validation & composition operations" ([SS14](https://docs.spacestation14.com/en/robust-toolbox/serialization.html)).

### 4. Events, parallel work and speed

- **A unique key decides the order, never the structure.** The two libraries break heap ties differently: libstdc++ moves the hole to the right child, libc++ to the left ([libc++](https://github.com/llvm/llvm-project/blob/main/libcxx/include/__algorithm/sift_down.h), [libstdc++](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13/libstdc++-v3/include/bits/stl_heap.h)), so a phone and a cloud run would split at the first tie the structure decided.
  ns-3's key is time plus a unique id ([ns-3](https://www.nsnam.org/doxygen/d6/d4d/classns3_1_1_scheduler.html)); OMNeT++ counts insertions because the heap "is not stable" ([OMNeT++](https://github.com/omnetpp/omnetpp/blob/master/include/omnetpp/ceventheap.h)).
- **The structure is a speed choice.** Benchmark (10% of events cancelled, delays from minutes to weeks): binary heaps 120–230 ns a handled event at 5,000–50,000 live events, beating the 4-ary heap and `std::priority_queue`; minute buckets for a day plus a heap 51–125 ns, at 2.4 times the memory.
  At `TIM-07`'s speeds the queue costs 1–4% of one core whichever is used; "For modest execution times ... the choice of priority queue is usually not significant" ([ns-3](https://www.nsnam.org/docs/manual/html/events.html)).
- **Cancel lazily.** Each owner keeps the sequence number it expects; a popped event that doesn't match is skipped. Dead entries reached 2.8 times the live ones, so the queue is rebuilt when they pass a quarter, Go's rule ([Go](https://github.com/golang/go/blob/bacd25051cbe2f71e334a27b48e6c3ba6ff9e6a1/src/runtime/time.go)).
  A save holds only the live events, sorted; reloaded into another structure, a run went on with an identical checksum.
- **Windows of reading a snapshot change history.** P6 decided in parallel from the state at the start of 5-minute windows.
  Inside a window people cannot see each other, a pursuer cannot follow a turning quarry (`TIM-17`'s chases), and a pause or save inside a window would change the outcome; "the results of digital simulations ... differ greatly when time is discrete as opposed to continuous" ([Huberman and Glance](https://doi.org/10.1073/pnas.90.16.7716)).
- **Exact parallel work exists.** "if each LP adheres to the local causality constraint, execution of the simulation program on a parallel computer will produce exactly the same results as an execution on a sequential computer" ([Fujimoto](https://informs-sim.org/wsc01papers/018.PDF)).
  Factorio keeps what interacts on one thread: networks sharing an entity "had to be in the same update group, be updated by the same thread" ([FFF 421](https://factorio.com/blog/post/fff-421)).
  So groups too far apart to touch each other within a window can run in parallel, each in key order, and give the one-thread result for any window, thread count or speed.
- **Speed.** RimWorld "will run as close to the target as possible" ([wiki](https://rimworldwiki.com/wiki/Time)); Factorio "will automatically slow down when the computer that is running the game is unable to do all needed calculations" ([wiki](https://wiki.factorio.com/Game-tick)); none shows the real speed except as ticks a second.
  Kindling's events let the simulation run ahead of the screen by a bounded lead while the screen places each walker along its activity, so nothing runs per frame.

### 5. Catalogues

- **Parsing floats differs in what it refuses.** Experiment: 167,816 hard float literals parsed with correct bits on the cloud's and the phone's libraries alike, but toml++'s and toml11's default routes on the phone refused all 9,280 tiny (subnormal) values the cloud accepted, through how bionic reports underflow and libc++ reacts to it.
  TOML itself only recommends "at least IEEE 754 binary64 values", while integers should be "handled losslessly" ([TOML 1.1](https://toml.io/en/v1.1.0)).
- **Units as text read exactly.** Cataclysm: DDA writes quantities "as a string with a numerical value and an abbreviation of the unit" ([CDDA](https://github.com/CleverRaven/Cataclysm-DDA/blob/master/doc/JSON/JSON_INFO.md)); Factorio writes energy as `"5MJ"` ([Factorio](https://lua-api.factorio.com/latest/types/Energy.html)).
  Experiment: an exact reader of strings such as "3.5 kg", "1 h 30 min" and "1 in 100" into whole base units gave the same digest on both stacks over 220,007 strings.
- **toml++** passes the TOML test suite, works without exceptions, keeps every value's line and column, and keeps tables in sorted order on both stacks ([toml++](https://github.com/marzer/tomlplusplus)); it parsed a 575 KB launch-size catalogue in 22 ms and adds 114 KB on arm64.
  toml11 needs exceptions, which godot-cpp turns off, its tables iterate in different orders on the two stacks, and it was 11 times slower.
- **The strict games make their own types the schema.** RimWorld: "All XML nodes inside the <ThingDef> tag are a field in the ThingDef class", and an unknown tag fails ([RimWorld](https://rimworldwiki.com/wiki/Modding_Tutorials/XML_Defs)); Space Station 14's linter "start[s] up two integration instances and call[s] a prototype manager method" ([SS14](https://docs.spacestation14.com/en/maintainer-meetings/maintainer-meeting-2022-03-19.html)).
  Experiment: one `visit()` function per kind loaded, checked and fingerprinted 1,000 entry files in 25–33 ms, with the same fingerprints on both stacks, and refused ten planted faults each at its file, line and column.
- **Inheritance between entries:** Cataclysm: DDA's own guideline warns that chained `copy-from` ends "essentially recreating the level of redundancy we'd like to eliminate" ([CDDA](https://github.com/CleverRaven/Cataclysm-DDA/blob/master/doc/JSON/JSON_INHERITANCE.md)), and for Kindling a parent would be an input to its child's values, which `MAT-13` forbids.
- **Names, not numbers, in saves:** Minecraft's 1.13 Flattening "removed numeric IDs", and its chunks keep a "palette" of the names used ([Minecraft](https://minecraft.wiki/w/Chunk_format)); Factorio's "JSON migrations allow changing one prototype into another" ([Factorio](https://lua-api.factorio.com/latest/auxiliary/migrations.html)).
- **Onto the phone:** Godot exports `.toml` only through a filter ([Godot](https://docs.godotengine.org/en/stable/tutorials/export/exporting_projects.html)), and listing `res://` folders after export "may behave unexpectedly" ([Godot](https://docs.godotengine.org/en/stable/classes/class_diraccess.html)).

### 6. Saves

- **A file's own `fsync` does not make its name durable:** "For that an explicit fsync() on a file descriptor for the directory is also needed" ([fsync(2)](https://man7.org/linux/man-pages/man2/fsync.2.html)).
  Pixels keep their data on f2fs, mounted `fsync_mode=nobarrier` on the Pixel 8 ([fstab](https://android.googlesource.com/device/google/zuma/+/refs/heads/main/conf/f2fs/fstab.zuma.f2fs)), which unlike ext4 has no safety net for a missing `fsync` ([f2fs](https://docs.kernel.org/filesystems/f2fs.html)).
  Godot's `FileAccess` never calls `fsync` ([Godot](https://github.com/godotengine/godot/blob/4.7.2-stable/drivers/unix/file_access_unix.cpp)).
- **Android's time:** "an application is not in the killable state until its onStop() has returned" ([Activity](https://developer.android.com/reference/android/app/Activity)); cached apps "are frozen 10 seconds after entering the cached state" ([AOSP](https://source.android.com/docs/core/perf/cached-apps-freezer)); a swipe from Recents kills within about a second; a flat battery is a clean shutdown ([BatteryService](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/BatteryService.java)).
  Godot 4.7.2 sends `NOTIFICATION_APPLICATION_PAUSED` on its main thread between frames; Back quits by default, and quitting kills the process.
- **A deterministic world needs to sync only its commands.** Everything after the last snapshot can be re-made by re-simulating; "If only your app crashes, your data still reaches the disk" ([Android](https://developer.android.com/topic/performance/sqlite-performance-best-practices)).
  Experiment: a journal whose records carry a length, type, sequence number and checksum survived 100 random kills with nothing lost, and 38 planted damages were each caught and cut back to the last good record, as LevelDB frames its log ([LevelDB](https://github.com/google/leveldb/blob/main/doc/log_format.md)).
- **Formats:** protobuf "serialization is not (and cannot be) canonical" ([protobuf](https://protobuf.dev/programming-guides/serialization-not-canonical/)) and FlatBuffers' fields "MUST be added to the end" ([FlatBuffers](https://flatbuffers.dev/evolution/)); PNG's chunks let a reader skip what it doesn't know ([PNG](https://www.w3.org/TR/png-3/)).
  zstd at level 1 compresses at about 430 MB/s and decompresses at about 1,200 MB/s on a phone ([zstd 1.5.4](https://github.com/facebook/zstd/releases/tag/v1.5.4)), and its output is the same only within one version, so the state is hashed before compression.
- **Old saves:** Factorio's saves remember "which migrations from which mods have been applied" ([Factorio](https://lua-api.factorio.com/latest/auxiliary/migrations.html)); Minecraft keeps a data version in each chunk ([Minecraft](https://minecraft.wiki/w/Data_version)); OpenRCT2 runs a corpus of replays against every pull request, 22 files and 1.36 MB ([OpenRCT2](https://github.com/OpenRCT2/OpenRCT2/wiki/Replay-System)).
- **Export:** in Godot 4.7, `DisplayServer.file_dialog_show` on Android "uses the Android Storage Access Framework (SAF)", and its URI "can be passed directly to [FileAccess]" ([Godot](https://github.com/godotengine/godot/blob/4.7.2-stable/doc/classes/DisplayServer.xml)); Godot's zip writer stops at 4 GB.
- **Size:** a world of 7,000 people at Year 250 comes to about 3.5–4.1 GB by estimate, against `PLT-10`'s 4 GB: its history fits only at about 17 events a person a game day at the measured 13.6 bytes an event, or about 30 with a tighter encoding.

### 7. The bridge to Godot

- **godot-cpp** 4.5 loads in Godot 4.7, and a new line, 10.0.0 (15 September 2026), "can target Godot 4.3 or later" ([godot-cpp](https://github.com/godotengine/godot-cpp/releases/tag/10.0.0-stable)); a test extension built unchanged against it.
  A trimmed build profile built godot-cpp and the extension in 37–62 s, and the phone's library came to 248 KB with one exported symbol; godot-cpp passes `-fno-exceptions` to everything that links it.
- **The C++ runtime:** Godot's APK already ships `libc++_shared.so`, built with NDK r29; the NDK warns "An application should not use more than one C++ runtime" ([NDK](https://developer.android.com/ndk/guides/cpp-support)), which is safe for a static copy only while nothing C++ crosses Godot's plain-C extension boundary.
- **Files:** `res://` lives inside the APK, read through `AAssetManager`, so the simulation takes bytes, never paths; `user://` is the app's private folder, where `fsync` and `rename` work.
- **Threads:** bionic's default stack is about 1 MiB against glibc's usual 8, so stacks are set explicitly; priorities and affinity are allowed, but Google advises "it's generally best to avoid manually setting CPU affinities" ([AGI](https://developer.android.com/agi/sys-trace/threads-scheduling)).
- **Snapshots:** a triple buffer never blocks either side and always gives the newest complete snapshot ([triple_buffer](https://docs.rs/triple_buffer/latest/triple_buffer/)); in Godot it published 10,681 snapshots of 10,000 walkers to 900 frames with no torn read, and ThreadSanitizer found no race.
- **The crowd:** for 10,000 copies, filling the buffer took 0.15 ms and uploading it 0.11 ms with a custom bounding box (0.35 ms without); "there is no screen or frustum culling possible for individual instances" ([Godot](https://docs.godotengine.org/en/4.7/tutorials/performance/using_multimesh.html)), so crowds are split by area.
- **Calls:** an extension method from GDScript costs 113–166 ns and a signal with its handler about 550 ns, so the interface is a few calls a frame, never one per walker.
- **The 60 Hz screen:** Godot 4.7.2 applies the project's frame cap before the swapchain exists, so Swappy keeps voting 120 Hz; setting `Engine.max_fps` again at run time should reach it (read from source; the phone confirms).

### 8. The phone benchmark

- **Heat:** "A value of 1.0 indicates that the device is (or will be) throttled at ATHERMAL_STATUS_SEVERE" ([thermal.h](https://android.googlesource.com/platform/frameworks/native/+/refs/heads/main/include/android/thermal.h)); Android samples skin temperature only while an app asked in the last 10 s and needs 3 samples to forecast ([ThermalManagerService](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/power/ThermalManagerService.java)), so a forecast asked every 10 s is mostly the current reading; 0.1 of headroom is about 3 °C of skin.
- **Power:** since Android 15 an app can read the phone's own power rails, for the CPU clusters, GPU, display and memory, but they refresh only every 30 s and carry up to 10 J of noise ([PowerStatsService](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/powerstats/PowerStatsService.java)).
- **Frames:** Godot 4.7.2 exposes no frame statistics, and Android's frame timeline does not cover the surface Godot draws into ([Perfetto](https://perfetto.dev/docs/data-sources/frametimeline)); a GDExtension can run code once a frame after the present ([godot-cpp](https://github.com/godotengine/godot-cpp/blob/godot-4.5-stable/include/godot_cpp/godot.hpp)), and Godot's `delta` is smoothed and can hide jitter.
- **Godot 4.4–4.7.2 swaps its two frame-pacing modes** in code; the default happens to keep a fixed rate, and 4.8 fixes it ([PR 123300](https://github.com/godotengine/godot/pull/123300)).
- **Tracing with no computer:** the phone's System Tracing app records an app's own trace sections even in a release build ([Traceur](https://android.googlesource.com/platform/packages/apps/Traceur/+/refs/heads/main/src_common/com/android/traceur/PerfettoUtils.java)).
- **Codes people paste:** Crockford's base32 reads either case and "hyphens are ignored" ([Crockford](https://www.crockford.com/base32.html)); chat apps treat `*`, `_` and `~` as formatting ([WhatsApp](https://faq.whatsapp.com/539178204879377)).
  Experiment: a code with a checksum decoded through case changes, line breaks and swapped dashes, and caught every injected error.

## What we take

1. **The same bits by rule, proved on the outputs.** All C++ that includes the simulation's headers is built with `-std=c++20 -O2 -fno-exceptions -fno-fast-math -fno-math-errno -ffp-contract=off`, the last as the last floating-point flag; the check reads the compile commands, scans the built code for fused instructions and the platform's maths symbols, and runs the same seeded worlds on four builds: x86-64 with clang and with GCC (undefined-behaviour checks on), and arm64 with GCC and with the phone's own compiler, under qemu, on one to four threads, stopped and resumed (`RES-05`, `TIM-16`).
2. **A banned list with replacements:** no `long double`, no `float` in simulation state, no platform maths but square roots, no `fmin`/`fmax`, no casts from floating to integer outside one checked function, no float parsing, no `<random>` distributions, no `std::hash` or unordered order deciding anything, sorts only with strict total orders, no thread count or clock in decisions, no NaN in state.
3. **Correctly rounded maths:** CORE-MATH's double functions, vendored at a pinned commit, checked against MPFR in the cloud.
4. **Each simulation thread sets the default floating-point environment** and asserts it at checkpoints; the phone's self-check reports it.
5. **Numbers:** positions as 32-bit centimetres on the torus, wrapping in 64-bit arithmetic; time as 64-bit game seconds; amounts as 64-bit whole base units; probabilities as 64-bit thresholds read exactly from text; `double` only for working values.
6. **Checksums:** XXH3 over a canonical stream, each system in a fixed order, entities by id, fields little-endian, never raw memory; a digest per system and for the whole state at each checkpoint.
7. **Entities in EnTT v4.0.0** behind a thin layer: beings and things in two registries; every entity with a never-reused 64-bit id, the only handle that components, events, history and saves hold; one descriptor per component (name, version, fields with types, units, ranges and links) that drives catalogue loading, saves, checksums and the details view; pools made at start in name order.
8. **Nothing decides by EnTT's order:** decisions follow the event queue or lists sorted by id, every sort breaks ties by id, and an order fuzzer in the checks scrambles the pools.
9. **One event queue** keyed by (game second, owner's id, owner's sequence), unique, so the structure never decides a tie: a binary heap at first; cancelled events skipped by the owner's expected sequence, and the heap rebuilt when dead entries pass a quarter of the live ones; saves hold only live events, sorted.
10. **Events happen one at a time in key order; that run is the reference.** A handler schedules only keys after its own, so an effect on someone else lands at least a second later.
    Parallel work comes from islands: at each window, owners that could touch each other or the same thing within it join one island; islands run side by side, each in key order, and merge by key; the result equals the one-thread run for any window, thread count, speed or pause, as a test proves.
11. **The speed loop:** the simulation runs at most about a quarter of a real second ahead of the screen; the screen places every walker along its activity and never waits; when it catches up, time slows; the speed shown is measured from what was drawn; pausing lets the screen glide to the simulation's frontier, so a power acts on exactly the world shown.
12. **Catalogues in TOML 1.0, read by toml++ behind one file,** with no floats: whole numbers as integers, quantities and ratios as unit strings read exactly into whole base units; one `visit()` per kind is the only schema, walked by the loader (unknown keys refused, every error at file, line and column), a schema writer and a fingerprinter.
13. **No templates between entries;** names namespaced (`base:flint_nodule`), numbered by sorted name at load, chance keyed by a stable hash of the name, saves holding name palettes, renames in a file.
14. **Fingerprints decide small and big updates:** rules, world and look digests per source, a hand-raised world-making version guarded by a golden-world test; each world keeps its sources and digests (`TIM-08`, `PLT-09`).
15. **Durations record both lengths,** `{ life = "3 month", game = "15 d" }`, checked by `TIM-18`'s rule, "about" read as within 10%.
16. **Data reaches the phone as text:** the build copies `data/` into the Godot project with a list of files and their digests, the app reads each through `FileAccess`, and the self-check compares the phone's digests with the build's; the heavy catalogue checks run in the cloud.
17. **Saves of our own:** one I/O thread owns every file; snapshots in versioned chunks, compressed with zstd level 1 and hashed before compression; logs framed by length, type, sequence and checksum; commands synced at once, history at each snapshot; a snapshot every 30 real seconds and when the app leaves the screen; write, sync, rename, sync the folder; a damaged file moved aside, never loaded.
18. **Crash recovery by catching up:** the newest valid snapshot, then the command journal re-applied at its moments, the re-made history compared with what was written.
19. **Old saves:** a version in every chunk, named whole-world migrations recorded in the save, and a corpus of small exported worlds that every build opens.
20. **Export and import as one `.kindling` file** with a checksum per part, through Android's file picker; several worlds, each a folder with a small manifest; the app keeps its data on uninstall only with your consent (`retain_data_on_uninstall`).
21. **The bridge:** `sim/` a static library; `view/` one shared library with godot-cpp 4.5 and a trimmed profile, the C++ runtime linked statically; three classes for GDScript (the world, the crowd, the device); a triple buffer for snapshots, a lossless queue for events; crowds as one MultiMesh per area with a custom bounding box.
22. **Simulation threads:** up to four, each with an explicit 8 MiB stack, a name and a slightly lower priority than Godot's main thread; pinned to the middle cores or not as the benchmark decides; Godot's own worker pool kept small.
23. **Lifecycle:** on pause the simulation stops at the next event and saves synchronously; Back never quits outright; the 60 Hz screen asked again at run time.
24. **Telemetry in `view/`:** heat headroom every 2 s with a 10-s forecast, the thermal status, battery and power rails, the cores' clocks, our threads' CPU time, memory, and every frame's interval; trace sections for the phone's System Tracing.
25. **Frames by our own measure:** a frame is on time when it arrives within a frame period plus half a refresh, a stall counts every period it skipped, and "more than 50 ms late" is a gap over 66.7 ms.
26. **The result code:** a version, a fixed layout and a checksum in Crockford base32, readable through what chat apps do to text.
27. **Heat slows time before the phone does:** the simulation's working share is cut fast and given back slowly as the forecast nears the first throttling level; nothing is cut but time (`PRN-11`).

## Sources

- Same bits:
  - [Gaffer on Games: floating point determinism](https://gafferongames.com/post/floating_point_determinism/)
  - [Box2D: determinism](https://box2d.org/posts/2024/08/determinism/)
  - [clang 21 manual](https://releases.llvm.org/21.1.0/tools/clang/docs/UsersManual.html)
  - [GCC 13.3: optimize options](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Optimize-Options.html)
  - [LLVM vectorizers](https://llvm.org/docs/Vectorizers.html)
  - [POSIX: pthread_create](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_create.html)
  - [Godot: raycast occlusion culling](https://raw.githubusercontent.com/godotengine/godot/master/modules/raycast/raycast_occlusion_cull.cpp)
  - [C++ draft: conv.fpint](https://eel.is/c++draft/conv.fpint)
  - [C++ draft: rand.dist.general](https://eel.is/c++draft/rand.dist.general)
  - [C++ draft: reduce](https://eel.is/c++draft/reduce)
  - [libc++: unspecified behaviour randomization](https://libcxx.llvm.org/DesignDocs/UnspecifiedBehaviorRandomization.html)
  - [glibc: errors in maths functions](https://sourceware.org/glibc/manual/latest/html_node/Errors-in-Math-Functions.html)
  - [bionic Runtime APEX](https://android.googlesource.com/platform/bionic/+/refs/heads/main/apex/Android.bp)
  - [Factorio forum: maths functions](https://forums.factorio.com/viewtopic.php?p=380343)
  - [CORE-MATH](https://core-math.gitlabpages.inria.fr/)
  - [Microsoft: correctly rounded maths with LLVM libc](https://devblogs.microsoft.com/cppblog/bringing-correctly-rounded-math-to-production-with-llvm-libc/)
  - [Factorio FFF 415](https://factorio.com/blog/post/fff-415)
  - [OpenTTD: desyncs](https://raw.githubusercontent.com/OpenTTD/OpenTTD/master/docs/desync.md)
  - [xxHash](https://github.com/Cyan4973/xxHash)
  - [AAPCS64](https://raw.githubusercontent.com/ARM-software/abi-aa/main/aapcs64/aapcs64.rst)
- Numbers:
  - [Factorio: MapPosition](https://lua-api.factorio.com/latest/types/MapPosition.html)
  - [OpenTTD: vehicle_base.h](https://raw.githubusercontent.com/OpenTTD/OpenTTD/master/src/vehicle_base.h)
- Entities:
  - [EnTT v4.0.0: entity documentation](https://github.com/skypjack/entt/blob/v4.0.0/docs/md/entity.md)
  - [EnTT v4.0.0: snapshot.hpp](https://github.com/skypjack/entt/blob/v4.0.0/src/entt/entity/snapshot.hpp)
  - [flecs v4.1.6: systems](https://github.com/SanderMertens/flecs/blob/v4.1.6/docs/Systems.md)
  - [Frontier, GDC 2024: cross-platform determinism](https://media.gdcvault.com/gdc2024/Slides/GDC+slide+presentations/Pollard_Bradley_CrossPlatformDeterminism+2024-03-26+09.34.19.pdf)
  - [Factorio: LuaEntity](https://lua-api.factorio.com/latest/classes/LuaEntity.html)
  - [Bevy: Entity](https://docs.rs/bevy_ecs/latest/bevy_ecs/entity/struct.Entity.html)
  - [Dwarf Fortress wiki: historical figures](https://dwarffortresswiki.org/index.php/Historical_figure)
  - [RimWorld wiki: ThingComp](https://rimworldwiki.com/wiki/Modding_Tutorials/ThingComp)
  - [Space Station 14: serialization](https://docs.spacestation14.com/en/robust-toolbox/serialization.html)
- Events, parallel work and speed:
  - [Brown: calendar queues (1988)](https://doi.org/10.1145/63039.63045)
  - [Rönngren and Ayani: priority queues compared (1997)](http://www.cs.odu.edu/~cmo/classes/old/cs475sp05/papers/ronngren97.pdf)
  - [Higiro, Gebre and Rao: multi-tier priority queues (2017)](https://pc2lab.cec.miamioh.edu/raodm/pubs/confs/pads17.pdf)
  - [Larkin, Sen and Tarjan: priority queues (2014)](https://arxiv.org/abs/1403.0252)
  - [Varghese and Lauck: timing wheels (1987)](http://www.cs.columbia.edu/~nahum/w6998/papers/sosp87-timing-wheels.pdf)
  - [ns-3: events](https://www.nsnam.org/docs/manual/html/events.html)
  - [ns-3: Scheduler](https://www.nsnam.org/doxygen/d6/d4d/classns3_1_1_scheduler.html)
  - [OMNeT++: event heap](https://github.com/omnetpp/omnetpp/blob/master/include/omnetpp/ceventheap.h)
  - [libc++: sift_down.h](https://github.com/llvm/llvm-project/blob/main/libcxx/include/__algorithm/sift_down.h)
  - [libstdc++ 13: stl_heap.h](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13/libstdc++-v3/include/bits/stl_heap.h)
  - [Go runtime: timers](https://github.com/golang/go/blob/bacd25051cbe2f71e334a27b48e6c3ba6ff9e6a1/src/runtime/time.go)
  - [Fujimoto: parallel and distributed simulation (2001)](https://informs-sim.org/wsc01papers/018.PDF)
  - [Huberman and Glance: discrete and continuous time (1993)](https://doi.org/10.1073/pnas.90.16.7716)
  - [Paradox: multi-threading in Paradox games (ACCU 2023)](https://accu.org/conf-docs/PDFs_2023/XMultiThreadingModelinParadoxGamesPastPresentandFuture.pdf)
  - [Factorio FFF 421](https://factorio.com/blog/post/fff-421)
  - [Block-STM (2022)](https://arxiv.org/abs/2203.06871)
  - [RimWorld wiki: time](https://rimworldwiki.com/wiki/Time)
  - [Factorio wiki: game tick](https://wiki.factorio.com/Game-tick)
  - [Gaffer on Games: fix your timestep](https://gafferongames.com/post/fix_your_timestep/)
- Catalogues:
  - [TOML 1.1.0](https://toml.io/en/v1.1.0)
  - [toml++](https://github.com/marzer/tomlplusplus)
  - [toml11](https://github.com/ToruNiina/toml11)
  - [tomlc17](https://github.com/cktan/tomlc17)
  - [bionic: strtod](https://android.googlesource.com/platform/bionic/+/refs/heads/main/libc/upstream-openbsd/lib/libc/gdtoa/strtod.c)
  - [RimWorld wiki: XML Defs](https://rimworldwiki.com/wiki/Modding_Tutorials/XML_Defs)
  - [RimWorld wiki: PatchOperations](https://rimworldwiki.com/wiki/Modding_Tutorials/PatchOperations)
  - [Factorio: data lifecycle](https://lua-api.factorio.com/latest/auxiliary/data-lifecycle.html)
  - [Factorio: migrations](https://lua-api.factorio.com/latest/auxiliary/migrations.html)
  - [Factorio: Energy](https://lua-api.factorio.com/latest/types/Energy.html)
  - [Cataclysm: DDA: JSON info](https://github.com/CleverRaven/Cataclysm-DDA/blob/master/doc/JSON/JSON_INFO.md)
  - [Cataclysm: DDA: JSON inheritance](https://github.com/CleverRaven/Cataclysm-DDA/blob/master/doc/JSON/JSON_INHERITANCE.md)
  - [Space Station 14: maintainer meeting, 19 March 2022](https://docs.spacestation14.com/en/maintainer-meetings/maintainer-meeting-2022-03-19.html)
  - [Minecraft wiki: chunk format](https://minecraft.wiki/w/Chunk_format)
  - [Minecraft wiki: resource location](https://minecraft.wiki/w/Resource_location)
  - [Godot: exporting projects](https://docs.godotengine.org/en/stable/tutorials/export/exporting_projects.html)
  - [Godot: DirAccess](https://docs.godotengine.org/en/stable/classes/class_diraccess.html)
- Saves:
  - [fsync(2)](https://man7.org/linux/man-pages/man2/fsync.2.html)
  - [rename(2)](https://man7.org/linux/man-pages/man2/rename.2.html)
  - [f2fs](https://docs.kernel.org/filesystems/f2fs.html)
  - [Pixel 8 fstab](https://android.googlesource.com/device/google/zuma/+/refs/heads/main/conf/f2fs/fstab.zuma.f2fs)
  - [Godot 4.7.2: file_access_unix.cpp](https://github.com/godotengine/godot/blob/4.7.2-stable/drivers/unix/file_access_unix.cpp)
  - [Android: Activity](https://developer.android.com/reference/android/app/Activity)
  - [Android: activity lifecycle](https://developer.android.com/guide/components/activities/activity-lifecycle)
  - [AOSP: cached apps freezer](https://source.android.com/docs/core/perf/cached-apps-freezer)
  - [AOSP: BatteryService](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/BatteryService.java)
  - [Android: SQLite best practices](https://developer.android.com/topic/performance/sqlite-performance-best-practices)
  - [LevelDB: log format](https://github.com/google/leveldb/blob/main/doc/log_format.md)
  - [Redis: persistence](https://redis.io/docs/latest/operate/oss_and_stack/management/persistence/)
  - [ALICE, OSDI 2014](https://www.usenix.org/conference/osdi14/technical-sessions/presentation/pillai)
  - [protobuf: serialization is not canonical](https://protobuf.dev/programming-guides/serialization-not-canonical/)
  - [FlatBuffers: evolution](https://flatbuffers.dev/evolution/)
  - [PNG 3](https://www.w3.org/TR/png-3/)
  - [zstd](https://github.com/facebook/zstd)
  - [zstd 1.5.4 notes](https://github.com/facebook/zstd/releases/tag/v1.5.4)
  - [Minecraft wiki: data version](https://minecraft.wiki/w/Data_version)
  - [OpenRCT2: replay system](https://github.com/OpenRCT2/OpenRCT2/wiki/Replay-System)
  - [Godot 4.7.2: DisplayServer](https://github.com/godotengine/godot/blob/4.7.2-stable/doc/classes/DisplayServer.xml)
  - [Android: app-specific storage](https://developer.android.com/training/data-storage/app-specific)
- The bridge:
  - [godot-cpp 10.0.0](https://github.com/godotengine/godot-cpp/releases/tag/10.0.0-stable)
  - [Godot 4.7: the .gdextension file](https://docs.godotengine.org/en/4.7/engine_details/engine_api/gdextension/gdextension_file.html)
  - [Godot 4.7: godot-cpp with CMake](https://docs.godotengine.org/en/4.7/tutorials/scripting/cpp/build_system/cmake.html)
  - [godot-cpp 4.5: build profiles](https://github.com/godotengine/godot-cpp/blob/godot-4.5-stable/build_profile.py)
  - [godot-cpp 4.5: compiler flags](https://github.com/godotengine/godot-cpp/blob/godot-4.5-stable/cmake/common_compiler_flags.cmake)
  - [NDK: C++ library support](https://developer.android.com/ndk/guides/cpp-support)
  - [NDK: newer APIs](https://developer.android.com/ndk/guides/using-newer-apis)
  - [Android: 16 KB page sizes](https://developer.android.com/guide/practices/page-sizes)
  - [Godot 4.7.2: file_access_android.cpp](https://github.com/godotengine/godot/blob/4.7.2-stable/platform/android/file_access_android.cpp)
  - [Godot 4.7: thread-safe APIs](https://docs.godotengine.org/en/4.7/tutorials/performance/thread_safe_apis.html)
  - [Android: thread scheduling](https://developer.android.com/agi/sys-trace/threads-scheduling)
  - [AOSP: Performance Hint API](https://source.android.com/docs/core/perf/performance-hint-api)
  - [triple_buffer](https://docs.rs/triple_buffer/latest/triple_buffer/)
  - [Godot 4.7: MultiMesh](https://docs.godotengine.org/en/4.7/tutorials/performance/using_multimesh.html)
  - [Godot 4.7.2: Vulkan driver (Swappy)](https://github.com/godotengine/godot/blob/4.7.2-stable/drivers/vulkan/rendering_device_driver_vulkan.cpp)
  - [Android: frame rate](https://developer.android.com/media/optimize/performance/frame-rate)
  - [Godot forum: 120 Hz at a 60 FPS cap](https://forum.godotengine.org/t/android-godot-why-fps-limit-display-refresh-rate-on-120-hz-devices-terraria-mobile-legends-example/132320)
- The phone benchmark:
  - [AOSP: thermal.h](https://android.googlesource.com/platform/frameworks/native/+/refs/heads/main/include/android/thermal.h)
  - [AOSP: ThermalManagerService](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/power/ThermalManagerService.java)
  - [Android: ADPF thermal](https://developer.android.com/games/optimize/adpf/thermal)
  - [AOSP: PowerStatsService](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/powerstats/PowerStatsService.java)
  - [AOSP: BatteryManager](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/core/java/android/os/BatteryManager.java)
  - [Perfetto: frame timeline](https://perfetto.dev/docs/data-sources/frametimeline)
  - [Perfetto: atrace](https://perfetto.dev/docs/data-sources/atrace)
  - [godot-cpp 4.5: godot.hpp](https://github.com/godotengine/godot-cpp/blob/godot-4.5-stable/include/godot_cpp/godot.hpp)
  - [Godot PR 123300: swapped Swappy modes](https://github.com/godotengine/godot/pull/123300)
  - [Traceur: PerfettoUtils](https://android.googlesource.com/platform/packages/apps/Traceur/+/refs/heads/main/src_common/com/android/traceur/PerfettoUtils.java)
  - [Android: on-device tracing](https://developer.android.com/topic/performance/tracing/on-device)
  - [Crockford: base32](https://www.crockford.com/base32.html)
  - [WhatsApp: formatting](https://faq.whatsapp.com/539178204879377)
