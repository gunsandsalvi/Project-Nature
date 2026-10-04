# Research 03: the simulation core

**Question:** how should the world's data, its clock, its randomness, its threads and its saves be built?
The goals:
- thousands of people run fast on your phone;
- the same history comes out every time, on the phone and in the cloud (`RES-05`, `TIM-16`);
- nothing is ever lost (`PLT-07`, `PLT-09`);
- new things are added as data (`PRN-14`, `MAT-13`).

## How others do it

### Data: entities, components, and content as data

- **RimWorld** keeps content in XML "Defs" and behaviour in code.
  A thing is an instance of its Def, built from small components ("Comps") that the data lists ([RimWorld wiki: Defs](https://rimworldwiki.com/wiki/Modding_Tutorials/XML_Defs), [ThingComp](https://www.rimworldwiki.com/wiki/Modding_Tutorials/ThingComp)).
- **Dwarf Fortress** defines creatures, materials, tissues and bodies in plain-text "raws" of bracketed tokens, such as `[CREATURE:DWARF]` ([Dwarf Fortress wiki: raws](https://www.dwarffortresswiki.org/index.php/Raw)).
  After 20 years, its creator calls its class hierarchy for items "ultimately a mistake", because it blocked generated hybrids.
  The game moved toward components that switch on and off ([Stack Overflow blog](https://stackoverflow.blog/2021/07/28/700000-lines-of-code-20-years-and-one-developer-how-dwarf-fortress-is-built/)).
- **Factorio** defines every machine and item as a "prototype" in Lua data files ([Factorio Lua API](https://lua-api.factorio.com/latest/auxiliary/migrations.html)).
- **Entity-component systems** store each kind of data in its own tight array, so a system touching only positions reads only positions ([habr](https://habr.com/en/articles/651921/)).
  - **EnTT** is the best-known C++ one: header-only, sparse sets, MIT-licensed, used in Minecraft ([EnTT](https://github.com/skypjack/entt)).
  - **Flecs** is the other major one, written in C, with its own threading ([SaaSHub](https://www.saashub.com/entt-alternatives)).
- **Data-oriented design:** "the transformation of data is the only purpose of any program".
  Arrange data for the cache, not for human convenience ([Mike Acton, CppCon 2014](https://cppcon.org/?p=356)).

### Time

- **RimWorld** runs 60 ticks a real second at normal speed, 2,500 ticks to the game hour.
  Things that need little attention update every 250 ticks ("rare") or every 2,000 ("long").
  When the computer cannot keep up, the game runs "as close to the target as possible" ([RimWorld wiki: time](https://rimworldwiki.com/wiki/Time), [ticks](https://rimworldwiki.com/wiki/Template:Ticks/doc)).
- **Many waiting things:** a hide drying for days, or a pregnancy.
  - Operating systems and Kafka keep millions of timers in **hierarchical timing wheels**, wheels of hours, minutes and seconds, so adding and expiring a timer costs almost nothing ([Varghese and Lauck, 1987](https://blog.acolyer.org/2015/11/23/hashed-and-hierarchical-timing-wheels/)).
  - A priority queue ordered by time does the same job more simply for fewer timers.
- **A fixed simulation step with a free frame rate:** the screen interpolates between the last two simulation states.
  This gives the same simulation on every machine, no "spiral of death" when a frame runs late, and smooth motion ([Gaffer on Games: fix your timestep](https://gafferongames.com/post/fix_your_timestep/)).

### The same result everywhere

- **Lockstep games need "absolutely identical results on every client, down to the least-significant bit of the mantissa"**, so that a checksum of the whole state matches ([Gaffer on Games](https://gafferongames.com/post/floating_point_determinism/)).
- **Box2D (2024) shows floating point is enough on modern processors,** without fixed-point maths ([Box2D: determinism](https://box2d.org/posts/2024/08/determinism/)).
  It avoids three traps:
  - **fast-math,** which will "jumble your arithmetic in pursuit of higher performance", gave no real speed-up in its tests, and should never be used;
  - **fused multiply-add,** which compilers use differently: turned off with `-ffp-contract=off`;
  - **the C library's trigonometry:** in its tests sine and cosine matched, but atan2 differed between platforms, so it uses its own versions.
    The square root is the same everywhere.

  It tests on x64 and ARM with three compilers: a scene runs until everything sleeps, and the step count and a hash of all positions must match.
  For threads, each worker writes its own bit array, merged by the main thread in a fixed order, with "no noticeable performance impact".
- **Correctly rounded maths:** the CORE-MATH project's functions (sine, exponent, logarithm and more) return the correctly rounded answer, at speeds close to the usual libraries' ([CORE-MATH](https://core-math.gitlabpages.inria.fr/), [talk](https://members.loria.fr/PZimmermann/talks/core-math-raim2022.pdf)).
  That answer is unique, so it is the same on every machine: LLVM's maths library notes that consistency across platforms "will be satisfied automatically if the implementation is correctly rounded" ([LLVM libc](https://llvm.googlesource.com/llvm-project/+/refs/tags/llvmorg-16.0.0/libc/docs/math.rst)).
- **Factorio:**
  - Its early builds took a checksum of the whole map every tick and checked it against replays ([Friday Facts 47](https://factorio.com/blog/post/fff-47)).
  - In 2024 a desync turned out to depend on the number of processor cores.
    It was found by pretending one computer had fewer cores, and had been hidden since 2017 ([Friday Facts 415](https://direct.factorio.com/blog/post/fff-415)).
- **Frontier (Warhammer Age of Sigmar: Realms of Ruin, GDC 2024)** built a thread-safe ECS and tools to find desyncs, across PC, Xbox, PlayStation and Linux ([GDC Vault](https://gdcvault.com/play/1034229/Cross-Platform-Determinism-in-Warhammer)).
- **Deterministic parallel work** follows a few rules ([Bugnet: Unity jobs](https://bugnet.io/blog/how-to-fix-unity-burst-job-nondeterminism-breaking-sim-replay), [Unity: consistency sorting](https://docs.unity.cn/ScriptReference/PhysicsJobOptions2D-useConsistencySorting.html)):
  - each job writes only what it owns;
  - proposed changes are gathered in parallel, then applied in one pass sorted by entity;
  - systems that write the same data run in a fixed order.

### Randomness

- **Counter-based generators** compute the Nth number straight from a key and N, with no running state.
  So any thread can draw any number in any order.
  - Philox, from the Random123 library, passes the strictest statistical test suite (BigCrush) and parallelises perfectly ([Salmon et al., SC11](https://www.thesalmons.org/john/random123/releases/latest/docs/), [paper](https://users.cs.utah.edu/~hari/teaching/bigdata/random123sc11.pdf)).
- **Squirrel Eiserloh's "noise-based RNG"** (GDC 2017) makes the same case for games: "unordered access, better reseeding, record/playback, … lock-free parallelization", in a few lines of hashing ([GDC Vault](https://gdcvault.com/play/1024365/Math-for-Game-Programmers-Noise), [Squirrel Noise 5](https://npmjs.com/package/squirrel-noise-5)).

### Saves

- **Factorio's migrations** come in two kinds ([Factorio: migrations](https://lua-api.factorio.com/latest/auxiliary/migrations.html)):
  - small JSON files rename prototypes;
  - Lua scripts change the loaded state.

  Each save remembers which migrations it has had, so none runs twice.
- **RimWorld** saves readable XML, with back-compatibility converters for old versions ([RimWorld wiki: save file](https://rimworldwiki.com/wiki/Save_file)).
- **Never a half-written save:** write a temporary file, flush it to storage, check it, then rename it over the old one.
  The rename is atomic, so a crash or flat battery leaves the old save intact.
  Keep one backup ([Bugnet](https://bugnet.io/blog/fix-game-save-file-corruption-on-power-loss), [Android: AtomicFile](https://developer.android.com/reference/android/util/AtomicFile.html)).
- **On a phone, the save made as the app is suspended must be small and fast,** inside the brief grace Android allows ([Bugnet](https://bugnet.io/blog/how-to-fix-mobile-save-corruption-from-write-during-suspend), [Unity discussions](https://discussions.unity.com/t/save-on-suspend-can-catastrophically-fail/674180)).

### What makes big simulations slow

- **Dwarf Fortress's "FPS death":** the game becomes unplayable after a year or two.
  The usual causes:
  - pathfinding for many walkers, worst through narrow bottlenecks;
  - temperature calculations;
  - line-of-sight checks;
  - simply the number of creatures, dead ones and ghosts included ([Steam discussions](https://steamcommunity.com/app/975370/discussions/0/3727324491572010578), [Lag (wiki)](https://catsplode.com/index.php/Lag)).
- **Songs of Syx** handles tens of thousands of people by finding paths between cached clusters first, recomputed only when a cluster changes ([Steam discussion](https://steamcommunity.com/app/1162750/discussions/0/3278066352365951865/)).

### The boundary with Godot

Covered in research 01:
- Godot's scene tree must not be touched from other threads;
- its RenderingServer only in a thread model with known bugs.

So the simulation runs on its own threads and hands over a finished snapshot once a frame.

## What we take

1. **The simulation is a C++ library separate from Godot.**
   It holds the world's state and rules, and never reads Godot or the camera (`WLD-13`).
2. **Entities and components in EnTT,** with data laid out for the cache, and systems run in a fixed order each tick.
   A thing's kind is its catalogue entry, and its parts are the components the entry lists, as RimWorld's Defs and Comps.
   There is no class hierarchy of kinds, which Dwarf Fortress regretted.
3. **Content as data:** text catalogues read and checked at start, with stable names, so adding an entry never needs code.
4. **Time in fixed ticks**, with slow processes as timers on one queue ordered by (time, entity, sequence), as RimWorld's rare ticks and timing wheels do.
   - Faster time means more ticks per frame, within a budget.
   - When the phone cannot keep up, time slows (`PRN-11`), as RimWorld runs "as close to the target as possible".
   - The screen interpolates between ticks for smooth motion.
5. **The same bits everywhere,** following Box2D:
   - no fast-math;
   - `-ffp-contract=off`;
   - no platform maths library for anything but square roots: our own approximations, or CORE-MATH's correctly rounded functions;
   - no iteration over unordered hash maps;
   - parallel work in fixed partitions, gathered then applied in entity order.
6. **The proof, as Box2D and Factorio do it:**
   - a seeded world runs on x86 and on ARM, and on one core and on four;
   - checksums of the whole state are compared at checkpoints;
   - the phone runs the same check in its self-test.
7. **Randomness keyed by (world seed, system, being, tick, purpose, index),** through a counter-based generator: Philox, or a Squirrel-style hash.
8. **Saves as Factorio and Android advise:**
   - versioned chunks;
   - migrations recorded in each save;
   - written to a temporary file, flushed, checked, then renamed, with one backup;
   - a small, fast save on suspending;
   - the history appended as it happens.
9. **Watch the known killers:** pathfinding at scale (research 10), temperature fields and line-of-sight, from the first benchmark on.
10. **Prototypes before production:**
    - the same bits on your phone's chip and in the cloud;
    - the cost of a thousand simple agents on your phone, at held speed.

## Sources

- Data:
  - [RimWorld wiki: Defs](https://rimworldwiki.com/wiki/Modding_Tutorials/XML_Defs)
  - [RimWorld wiki: ThingComp](https://www.rimworldwiki.com/wiki/Modding_Tutorials/ThingComp)
  - [Dwarf Fortress wiki: raws](https://www.dwarffortresswiki.org/index.php/Raw)
  - [Stack Overflow blog: Dwarf Fortress](https://stackoverflow.blog/2021/07/28/700000-lines-of-code-20-years-and-one-developer-how-dwarf-fortress-is-built/)
  - [EnTT](https://github.com/skypjack/entt)
  - [habr: ECS layouts](https://habr.com/en/articles/651921/)
  - [Mike Acton: data-oriented design (CppCon 2014)](https://cppcon.org/?p=356)
- Time:
  - [RimWorld wiki: time](https://rimworldwiki.com/wiki/Time)
  - [RimWorld wiki: ticks](https://rimworldwiki.com/wiki/Template:Ticks/doc)
  - [Varghese and Lauck: timing wheels](https://blog.acolyer.org/2015/11/23/hashed-and-hierarchical-timing-wheels/)
  - [Gaffer on Games: fix your timestep](https://gafferongames.com/post/fix_your_timestep/)
- Determinism:
  - [Box2D](https://box2d.org/posts/2024/08/determinism/)
  - [Gaffer on Games: floating point determinism](https://gafferongames.com/post/floating_point_determinism/)
  - [CORE-MATH](https://core-math.gitlabpages.inria.fr/)
  - [CORE-MATH talk](https://members.loria.fr/PZimmermann/talks/core-math-raim2022.pdf)
  - [LLVM libc maths](https://llvm.googlesource.com/llvm-project/+/refs/tags/llvmorg-16.0.0/libc/docs/math.rst)
  - [Factorio FFF 47](https://factorio.com/blog/post/fff-47)
  - [Factorio FFF 415](https://direct.factorio.com/blog/post/fff-415)
  - [Frontier, GDC 2024](https://gdcvault.com/play/1034229/Cross-Platform-Determinism-in-Warhammer)
  - [Bugnet: Unity jobs](https://bugnet.io/blog/how-to-fix-unity-burst-job-nondeterminism-breaking-sim-replay)
  - [Unity: consistency sorting](https://docs.unity.cn/ScriptReference/PhysicsJobOptions2D-useConsistencySorting.html)
- Randomness:
  - [Random123](https://www.thesalmons.org/john/random123/releases/latest/docs/)
  - [Salmon et al. paper](https://users.cs.utah.edu/~hari/teaching/bigdata/random123sc11.pdf)
  - [Squirrel Eiserloh, GDC 2017](https://gdcvault.com/play/1024365/Math-for-Game-Programmers-Noise)
- Saves:
  - [Factorio migrations](https://lua-api.factorio.com/latest/auxiliary/migrations.html)
  - [RimWorld wiki: save file](https://rimworldwiki.com/wiki/Save_file)
  - [Bugnet: power loss](https://bugnet.io/blog/fix-game-save-file-corruption-on-power-loss)
  - [Bugnet: suspend](https://bugnet.io/blog/how-to-fix-mobile-save-corruption-from-write-during-suspend)
  - [Android AtomicFile](https://developer.android.com/reference/android/util/AtomicFile.html)
  - [Unity: save on suspend](https://discussions.unity.com/t/save-on-suspend-can-catastrophically-fail/674180)
- Performance:
  - [Steam: Dwarf Fortress FPS](https://steamcommunity.com/app/975370/discussions/0/3727324491572010578)
  - [Lag](https://catsplode.com/index.php/Lag)
  - [Songs of Syx](https://steamcommunity.com/app/1162750/discussions/0/3278066352365951865/)
