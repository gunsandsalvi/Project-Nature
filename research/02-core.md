# Research 02: the simulation core

**Question:** how should the world's data, its clock, its randomness and its saves be built, so that thousands of people run fast on your phone, give the same history every run (`RES-05`), and new things can be added as data (`PRN-14`)?

## How others do it

- **RimWorld** keeps its content in data files ("Defs" in XML) and its behaviour in code.
  A thing is an instance of its Def, built from small components ("Comps") that the data lists, so new things are mostly data ([RimWorld wiki: XML Defs](https://rimworldwiki.com/wiki/Modding_Tutorials/XML_Defs), [ThingComp](https://www.rimworldwiki.com/wiki/Modding_Tutorials/ThingComp)).
- **Dwarf Fortress**, after 20 years and 700,000 lines, calls its class hierarchy for items "ultimately a mistake": it blocks generated hybrids, so it moved toward components that switch on and off.
  It keeps the simulation on one thread to avoid concurrency bugs, tracks connected regions so impossible paths are rejected at once, and fixed its byte sizes early so saves move between machines ([Stack Overflow blog](https://stackoverflow.blog/2021/07/28/700000-lines-of-code-20-years-and-one-developer-how-dwarf-fortress-is-built/)).
- **Songs of Syx** handles tens of thousands of people by finding paths between cached clusters first, then within a cluster, recomputed only when a cluster changes ([Steam discussion](https://steamcommunity.com/app/1162750/discussions/0/3278066352365951865/)).
- **Entity-component systems** store each kind of data in its own tight array, so a system touching only positions reads only positions.
  The two common layouts are archetypes and sparse sets ([habr](https://habr.com/en/articles/651921/), [DEV](https://dev.to/beefedai/designing-a-scalable-entity-component-system-for-modern-games-5240)).
  **EnTT** is the best-known C++ one: header-only, sparse sets, MIT-licensed, C++20, and used in Minecraft, "available literally everywhere" ([EnTT](https://github.com/skypjack/entt)).
- **In Godot**, one MultiMesh draws thousands of copies in a single call ([Godot docs](https://docs.godotengine.org/en/stable/tutorials/performance/using_multimesh.html)).
  C++ plug-ins reach the engine fastest: 11 to 12 ms where Rust took 31 ms in one test ([gdext issue 911](https://github.com/godot-rust/gdext/issues/911)).
  The scene tree is not thread-safe, so work on other threads hands its results back with deferred calls ([Godot docs: thread-safe APIs](https://docs.godotengine.org/zh-tw/4.x/_sources/tutorials/performance/thread_safe_apis.rst.txt)).
- **The same result on phone and cloud.**
  - x86 and ARM can round floating-point maths differently, through fused multiply-adds, compiler choices and library functions.
  - Studios turn off fast-math and fused multiply-adds, write their own maths functions, or use fixed-point numbers where results must match ([Bugnet](https://bugnet.io/blog/how-to-debug-floating-point-determinism-across-platforms)).
- **Randomness by key.**
  A counter-based generator such as Philox computes the Nth number straight from a key and N, with no running state.
  So any person, plant or day can draw its own numbers in any order, on any thread, and get the same ones ([simdrng](https://simdrng.readthedocs.io/en/latest/guides/philox.html)).
- **Saves:**
  - write a format version into every save;
  - upgrade old saves by a chain of migrations, never deleting old steps;
  - test every build against a collection of old saves ([Bugnet](https://bugnet.io/blog/how-to-version-your-save-format-across-game-updates), [GDevelop](https://docs.gdevelop.io/GDJS%20Runtime%20Documentation/variables/gdjs.saveState.CURRENT_SAVE_FORMAT_VERSION.html)).

## What we take

1. **The simulation is one C++ library inside Godot (a GDExtension), separate from the scene.**
   It owns the world's state and its rules; it never reads Godot's nodes.
   Godot only draws a snapshot of it each frame and passes your gestures and powers in as commands.
   So the history depends on nothing on screen (`PRN-11`, `TIM-17`).
2. **Entities and components in EnTT.**
   - People, animals, plants, things and places are entities with generational IDs.
   - Their data are components in tight arrays.
   - Behaviour lives in systems that run in a fixed order.
3. **Content is data (`PRN-07`, `PRN-14`).**
   Materials, plants, animals, blueprints, illnesses and the model kit's shapes are entries in text files, as in RimWorld.
   A new animal is a new entry and its model parameters, with no new code, which answers your question about adding entities easily.
4. **A fixed tick and an event queue.**
   - Game time advances in fixed ticks.
   - Long processes (a hide drying, a pregnancy) are events on a queue, not work done every tick.
   - Faster time means more ticks per frame within a budget, so the screen stays smooth and time slows instead (`PRN-11`).
5. **Threads, deterministically.**
   Work splits into fixed chunks merged in a fixed order, so four cores give the same result as one (`RES-05`).
6. **The same bits everywhere:**
   - no fast-math or fused multiply-adds;
   - our own sine, exponent and the like;
   - randomness keyed by (world seed, entity, purpose, tick);
   - no iteration over unordered hash maps.

   A test runs the same world on the cloud's x86 and on ARM, and compares the hashes.
7. **Saves in versioned chunks with migrations**, and a folder of old saves that every build must still open.
8. **Tests:**
   - C++ unit tests (doctest) run natively in the cloud;
   - Godot's side is tested headless;
   - pictures come from Godot on the software Vulkan driver (research 01).
