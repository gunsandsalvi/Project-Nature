# B74 phone test: how to bundle it

A Rust library, `libksound.so`, that plays a camp test pattern through Android's low-latency audio (AAudio) and reports the output delay and the audio work for 8, 32 and 128 sounds at once (`SND-01`, `SND-08`). Same shape as the B01 `kbench` library: JSON in, JSON out.

## Build

From this folder, with the B78 environment (`. ../b78-b79-phone/tools/env.sh`):

```
cargo ndk -t arm64-v8a -P 31 -o <app>/build/rustJniLibs build --release --lib --features android
```

- Add it as one more Gradle `Exec` task next to `cargoKphone`, before `preBuild`.
- Output: `libksound.so`, about 0.5 MB, 16 KB aligned (see `.cargo/config.toml`). It links `libaaudio.so` (API 26+; usage and content type need API 28, so the app's API 31 floor covers it).
- Checked here: it builds, exports `Java_dev_kindling_pretests_Sound_run`, and needs only Android's own libraries (`results/android-build.txt`).

## Kotlin

Copy `android/Sound.kt` into `dev.kindling.pretests`. Then, on a background thread:

```kotlin
val result = if (Sound.loaded) Sound.run(Sound.DEFAULT_CONFIG) else """{"error":"${Sound.loadError}"}"""
```

- No permissions are needed (output only).
- It takes about 35 seconds and plays sound through the speaker: ask the owner to set media volume to about half first.
- Put the whole result JSON into the result code.

## Config (all optional)

| Key | Default | Meaning |
|---|---|---|
| `voices` | `[8,32,128]` | Sounds at once, one phase each |
| `seconds_each` | `8` | Length of each phase |
| `gain` | `0.5` | Loudness (scaled down for more sounds) |
| `approach` | `"modal"` | `"noise"` for the cheap A2 sounds |
| `exclusive` | `true` | Ask for exclusive use of the audio output |
| `offline_seconds` | `1` | Offline mixing test per voice count |
| `cpus`, `per_cpu_voices`, `per_cpu_seconds` | all cores, `32`, `0.5` | Offline test pinned to each core in turn |

## What comes back

- `offline`: share of one core used to mix 8, 32 and 128 always-sounding voices, and voices per core.
- `per_cpu`: voices per core when pinned to each core (big and little cores differ).
- `audio.stream`: sample rate, burst size, buffer size, and whether the low-latency and exclusive modes were granted.
- `audio.phases`: for each voice count, `latency_ms_p50` (output delay), `xruns` (dropouts), `load_mean` and `load_p99` (audio work as a share of the time available), `thread_cpu_share`, and the cores the audio thread ran on.
- `error` instead, if the stream couldn't open; the offline numbers still come back.

## How the result is read (NOTES.md, B74 rule 5)

- Median output delay 50 ms or less, and no dropouts at 8 and 32 sounds: AAudio stays the plan.
- The cap on sounds at once: the largest of 8, 32 and 128 whose `load_mean` stays at 25% or less.
