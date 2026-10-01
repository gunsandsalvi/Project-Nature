# B78 and B79: toolchain, app shell, phone budgets

Pre-test, wave 0. Throwaway: deleted once the architecture is written.

## The question

- **B78** (crossroads `X10`): what is the best way to build the phone app from these cloud sessions? Can a session fetch the Android tools and build, sign and deliver an installable app (`PLT-06`)?
- **B79**: what can the phone sustain (processor, graphics, memory, battery, heat)? This round builds the test app that measures it; the answers come back in your result code.
- The test app also runs the phone side of `B01` (fast numbers) and `B02` (random draws), including the same-result check (`X11`).

## The approaches (B78)

1. Kotlin screen plus a Rust native library (Gradle with cargo-ndk).
2. Kotlin screen plus a C++ native library (Gradle with CMake).
3. Pure native, no Kotlin (Rust with the android-activity crate).
4. A web page in a WebView, calling native code through Kotlin.
5. Godot 4, only if cheap (see the decision rule).

## Decision rule (written before measuring; not changed afterwards)

**B78: which shell.**

An approach is out if it fails a gate:
- **G1:** it can't produce a signed (v2/v3), 16 KB-aligned arm64 APK here within 5 build attempts.
- **G2:** the same native core source can't also build and run headless on x86-64 Linux (the cloud).

The rest score 0 to 2 points on each line; the highest total wins.

| Criterion | 2 points | 1 point | 0 points |
|---|---|---|---|
| Attempts to the first working build | 1 | 2 to 3 | 4 to 5 |
| Rebuild after a one-line native change (median) | 20 s or less | 60 s or less | more |
| Clean build, download caches warm (median) | 90 s or less | 240 s or less | more |
| Glue code (non-blank lines outside the core) | 150 or less | 400 or less | more |
| Reaching Android's Java-only services (battery, the file picker for export, the phone's AI model) | directly from Kotlin | only through hand-written JNI calls | not at all |
| Toolchain download | 1.5 GB or less | 3 GB or less | more |

- Ties go to the smaller APK, then to approach 1.
- The Java-services line is judged, not measured.
- The WebView shell's score covers only the shell. Whether the game draws with WebGL in a WebView is `B66`'s call; the round-1 WebView frame times feed it.
- Godot is tried only if its editor plus Android export templates come to under about 1.5 GB.
- **Round-1 base:** approach 1, unless it fails a gate or scores 3 or more points below the best.

**B79: what the phone results will decide** (when your result code comes back):
- **Sustained speed:** all-core throughput in minute 10 compared with minute 1.
  - 80% or more: plan on the cool speed.
  - 50% to 80%: plan the simulation's steady load on the minute-10 speed.
  - Under 50%, or the thermal status reaches "severe": all-core load isn't sustainable; plan the steady load on fewer cores (from the per-cluster numbers), and flag `RSK-02`.
- **Heat warnings:** if the thermal headroom reading gives a number on at least 90% of samples and rises with load, it becomes the signal to slow time early (`PRN-11`). Otherwise use the thermal status plus battery temperature.
- **Frame pacing:** at the top refresh rate, a 99th-percentile frame interval within 1.5 refresh periods and under 1% missed frames means full refresh is a realistic target (`VIS-14`). Otherwise record the rate the phone gives and raise it.
- **Battery:** if all-core load alone drains more than 30% an hour, the simulation can't run flat out during play (`VIS-14` budget is 25 to 30% an hour for everything).
- **Memory:** if 10 GiB of native memory is reached without the app being killed, the 10 GiB of `PLT-01` holds. Otherwise the last step reached is the measured limit, raised with you, since `PLT-01` is decided.
- **Same result (`X11`):** any single-thread repeat on the same core with a different checksum is a failure, reported to `B01`/`B02`.
- **Delivery (`B78`):** if the code comes back and decodes, the install, run and paste loop works.
