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

## Method

- Toolchain into the shared cache: Android command-line tools, build-tools 36.1.0, platform android-36, platform-tools, NDK r30 (already installed there by another test, so reused), CMake 3.31.6, the Rust Android target and cargo-ndk 4.1.2. Gradle 8.14.3 was preinstalled; Android Gradle Plugin 8.13.2 and Kotlin 2.3.21 came from Maven.
- Four minimal apps (`shells/`), one screen each, calling one native function from a shared core (`shells/core`, Rust; `shells/s2-kotlin-cpp/core`, C++). Each core also runs headless on Linux.
- Every APK is checked by `tools/verify-apk.sh`: signature, 16 KB alignment of the zip entries and of the native code, arm64 only, no permissions.
- Timings: `tools/measure-shells.sh`, 3 rounds per shell, each under the shared CPU lock. Clean build = outputs deleted, cold Java VM, download caches warm. One-line change = one constant edited in the core, rebuilt with a warm Gradle daemon. I checked that the edit really reaches the APK (the library's hash changes, and changes back).
- Glue lines: `tools/count-glue.py` (non-blank, non-comment lines outside the core).
- Downloads: sizes from Google's package index and the servers (`results/downloads.csv`), plus the Gradle cache.

## Results (B78)

| | 1 Kotlin + Rust | 2 Kotlin + C++ | 3 Rust only | 4 WebView |
|---|---|---|---|---|
| Gates G1, G2 | pass | pass | pass | pass |
| Attempts to a working build | 2 | 1 | 1 | 1 |
| One-line rebuild, median (min–max) | 1.7 s (1.5–2.0) | 1.7 s (1.6–2.5) | 1.3 s (1.3–1.3) | 1.9 s (1.7–2.5) |
| Clean build, median (min–max) | 32.6 s (32.5–44.0) | 30.2 s (29.7–30.8) | 18.1 s (17.9–18.5) | 36.5 s (31.1–36.5) |
| Glue lines | 118 | 102 | 76 | 135 |
| Java-only services | direct | direct | hand-written JNI | direct |
| Download | 1.41 GB | 1.41 GB | 1.08 GB | 1.41 GB |
| APK | 338 KB | 44 KB | 742 KB | 338 KB |
| **Score** | **11** | **12** | **11** | **12** |

- Approach 1's first attempt failed only because Maven Central refused Gradle's burst of downloads ("429 Too Many Requests" on the shared cloud connection). Google's mirror of Maven Central fixed it, and every Gradle project here now uses it first.
- Downloads: 1.05 GB is common to all (mostly the NDK, 739 MB). The Gradle approaches add 325 MB of plugins; without Gradle preinstalled they would add 137 MB more (1.54 GB).
- Rust libraries carry about 300 KB of Rust's standard library; the C++ shell has no C++ runtime at all.
- Godot 4.7: editor 76 MB plus export templates 1,279 MB (1.36 GB), within the limit. Not tried, for lack of time.

## Verdict

**B78, by the rule:** approach 2 (Kotlin + C++) wins with 12 points, tied with approach 4 (WebView); the tie goes to the smaller APK. Approach 1 is one point behind, so it is not a bad base, and round 1 is built on it.

**The rule proved badly framed**, so read the result with care:
- It hardly separates the approaches. Every shell builds, signs and aligns here, cleans in 18 to 37 s and rebuilds in under 2 s. The one-point gaps come from which Gradle shell happened to meet the Maven rate limit first, and from Rust's 300 KB standard library.
- It mixes two questions. The shell (Kotlin, pure native or WebView) is `B78`'s. The core's language (Rust or C++) belongs to `B01`'s rule R3 (speed, then checks).
- What the data does show: building is not the problem. A Kotlin screen reaches Android's Java-only services directly (battery, thermal headroom, the file picker for export, the phone's AI model); round 1 already needs the first two. Pure native would need hand-written JNI for each, and can't show text without a UI library. WebView or native drawing is `B66`'s call.
- **Suggestion (not decided):** a Kotlin screen plus a native core, with the core's language taken from `B01` R3.

**B79:** waiting for your result code. The decision rule above says what each number will decide.

## The round-1 app

- Package `dev.kindling.pretests`, version r1; arm64 only; minSdk 31, targetSdk 36; 16 KB aligned; signed with the throwaway key in `test-key/`; no permissions; 1.1 MB.
- Uses the real benchmark library (`B01`/`B02`) and its Java kernels; the stub in `app/native/stub-kbench` is a fallback (`-Pkbench=stub`).
- Steps, about 16 minutes:
  - phone details;
  - the library's self-test;
  - 10 s resting baseline;
  - 20 s OpenGL smoothness test;
  - 30 s drawing test of `mockups/visual-style.html` in a WebView;
  - the library's 44-run plan plus 3 Java runs, each on one core per cluster (with a short repeat to compare checksums) and on all cores, measuring battery current meanwhile (about 5 minutes);
  - 10 minutes of all-core load, sampled every second.
- The memory test is a separate button.
- Results are saved after every step, and a marker is written before each step. After a crash, the next launch names the step and offers to continue.
- The result code is about 3.7 KB. Full detail (per-second samples, full checksums) stays in the app's files, which later rounds can read because they install over this one.
- Checksums are compared on the phone with the ones predicted by the cloud's ARM build (`results/determinism.csv` of `B01`): the "p" field.
- Decode a pasted code with `tools/decode-result.py`.

## Caveats

- Frame pacing is timed on the drawing thread, between frames. It is a fair sign of smoothness, but it is not the display's own presentation timing.
- Battery current is the phone's fuel-gauge reading. Over the 1-second kernel runs it is rough, and best read per cluster rather than per kernel.
- Kernel runs are about 1 s each (the plan suggests 1.5 s), to fit the 5-minute budget.
- The drawing test also lets the mockup's own animation loop draw about 12 extra frames a second.
- The app was built against the benchmark library as it was at build time (source fingerprint in `results/round1-build.json`).

## What only the phone can answer

- Whether the app installs and runs at all: no emulator here.
- Everything in `B79`: sustained speed and throttling, whether the thermal headroom reading works, frame pacing at the top refresh rate, the memory limit, battery drain.
- `B01`/`B02` on real hardware: speed and energy per cluster, and whether checksums repeat and match the predictions (`X11`).
- Whether Android lets the app read the CPU frequency files.

## Checked without a phone

- Native code: unit tests pass on x86, and on arm64 Linux under qemu. The real library's self-test also passes on arm64 under qemu, `cpus` pinning works, and its heat checksum matches x86.
- App logic: 12 JVM unit tests pass, covering cluster grouping, the run budget, the result code (a full-size result fits in 3.7 KB), checksum predictions, and crash recovery in the results store.
- Android lint: 0 errors and 9 harmless warnings (`results/lint-release.txt`).
- The APK: R8 kept every JNI and reflection name, and both libraries export the matching symbols.
- My review against the robustness rules: every step and every tap is wrapped; uncaught exceptions are written down; the OpenGL code catches its own errors; a WebView renderer crash is handled; there is a timeout on every wait; the screen stays on; Back is blocked during a run; rotation can't restart a test.

**Still untested:** none of the Kotlin screen code, the OpenGL shaders, the WebView test, the JNI calls at run time, or the battery and thermal calls has ever run. The memory test was tried only at small sizes. Installing a later round over this one depends on keeping the same key and package.

## For the owner

**Install (once):**
1. On the phone, signed in to GitHub in your browser, open this link (it downloads the app directly): https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/pretests/b78-b79-phone/dist/kindling-pretests-r1.apk
2. Or browse to it: the repository, branch `ccr-13ab6fef-fspju6`, then `pretests`, `b78-b79-phone`, `dist`, `kindling-pretests-r1.apk`, and tap the download button (top right).
3. When it finishes, tap the download (or open Files, then Downloads).
4. The first time, Android says the browser can't install apps: tap Settings, turn on "Allow from this source", go back, and tap Install.
5. If Play Protect warns about an unknown app, tap "More details", then "Install anyway".

**Run (about 15 minutes):**
1. Unplug the charger. Battery above 50%. Close other apps. Lay the phone flat on a table.
2. Open "Kindling tests" and tap "Run all tests (about 15 minutes)". Don't touch the phone; the screen stays on. For about a minute part of the screen shows a turning coloured grid, then the game's mockup moving by itself: that's expected.
3. When it says "All tests done", tap "Memory test (may close the app)", then Start. It takes about a minute. If the app closes, open it again.
4. Tap "Copy result code" and paste it into the chat. That's all.

If the app closes during the main tests, open it again and tap "Continue with the remaining tests".

## Later: signing for your hobbyist account (PLT-06)

Not set up now. When it is, you will need to:
- Create the free hobbyist developer account with Google, using your Google account. Google's exact steps may change by then; check at the time.
- Register the game's package name and the fingerprint of the key that signs it. Google may ask for an APK signed with that key, to prove we hold it.
- Agree where the release key lives: a secret in the cloud environment that builds the game, plus a backup you keep. It must never sit in the repository, unlike this pre-test key. If it's lost, updates can no longer install over the game.
- The pre-test key and package (`dev.kindling.pretests`) stay separate from the game.

## How to re-run

```
cd pretests/b78-b79-phone
export CACHE=<the shared download cache folder>
tools/setup-toolchain.sh              # Android tools into the shared cache (skips what's there)
. tools/env.sh
(cd app && gradle assembleRelease testReleaseUnitTest lintRelease)
tools/verify-apk.sh app/app/build/outputs/apk/release/app-release.apk
tools/measure-shells.sh               # B78 timings, under the shared CPU lock
python3 tools/count-glue.py
(cd app/native && cargo test && cargo test --target aarch64-unknown-linux-gnu)   # x86, then arm64 via qemu
tools/decode-result.py '<pasted code>'
```
Every build needs `rustup target add aarch64-linux-android aarch64-unknown-linux-gnu`, `cargo install cargo-ndk`, and `apt install qemu-user-static gcc-aarch64-linux-gnu` for the arm64 tests.
