# P5 The same bits

The fifth prototype (IMPLEMENTATION α0.4a, research 03) asks: do your phone and the cloud, on one thread and on four,
end a world identically?

Items it is about: `RES-05` (the same results everywhere, checked by checksums) and `TIM-16` (keyed chance, and our
own maths, so a world replays exactly).

- **The toy world** (`src/world.cpp`): 4,096 walkers on a torus act at events in the order of their ticks, a tie by
  walker; each turns, walks until its next event, tires and eats where it stops. Every draw is keyed chance
  (`src/chance.hpp`): a hash of the world's seed, the walker, the tick and the draw's purpose, so it never depends on
  what was drawn before or on which thread draws it (A3.5). Each evening the walkers' pace and the food's regrowth are
  worked out in chunks of 256 on one thread or several, and the day's sums gathered chunk by chunk in order (A3.4).
- **Our own maths** (`src/maths.cpp`): sine, cosine, exponent, logarithm and power from IEEE adds, multiplies and
  divides only, compiled with no fast-math and no fused multiply-adds, so every chip that rounds as IEEE says gives
  the same bits; the platform's versions may differ in their last bit.
- **The checks:** the whole state is hashed at the end of each game day (FNV-1a, `src/hash.hpp`). Its tests
  (`tests/`) compare one thread with four and with the cloud's recorded digest (`src/expected.hpp`); `arm64.sh` builds
  the same run for arm64 and runs it under qemu, on one thread and four, against this machine's.
- **On the phone:** `extension/` makes the same sources a Godot extension, built with godot-cpp at its 4.5 release
  (Godot 4.7 loads it), natively for the cloud's tests and for arm64 Android by `tools/build.sh`; the prototype app's
  "P5 The same bits" screen runs it on one thread and four and sets the digests beside the cloud's. The libraries are
  built, never committed.

    cmake -S prototypes/samebits -B build/samebits -G Ninja && cmake --build build/samebits
    build/samebits/samebits_cli 4 30       # four threads, 30 days: each day's checksum and the digest

P6 uses its maths, keyed chance, hashes and `arm64.sh` too, written once. Like every prototype it is thrown away once
its answer is written into the architecture (A3.4).
