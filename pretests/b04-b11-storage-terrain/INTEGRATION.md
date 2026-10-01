# B04/B11 phone part: how to bundle it

The crate is `phone/` (package and library `kstorage`). It builds on its own, like the B01/B02 `kbench` crate.

## Build

```
cd pretests/b04-b11-storage-terrain/phone
cargo ndk -t arm64-v8a -P 31 -o <jniLibs folder> build --release --features android
```

This gives `libkstorage.so` (SQLite and zstd are compiled in from C by the NDK's clang; no other files).

## Call it

Java/Kotlin class `dev.kindling.pretests.Storage`:

```kotlin
object Storage {
    init { System.loadLibrary("kstorage") }
    @JvmStatic external fun run(configJson: String): String
}
```

Run it off the main thread. Config, all fields optional except `dir`:

```json
{"dir": "<context.filesDir>", "div": 2, "reps": 3, "gen_w": 1024,
 "formats": ["custom-none", "custom-lz4", "sqlite-lz4"],
 "parts": ["save", "fsync", "gen", "detail"]}
```

- `dir`: a writable folder on the phone's own storage (use `filesDir`, not the cache folder). The test makes and deletes `dir/b04b11/`. It needs about 400 MB free while it runs.
- `div: 2` means a quarter-size world (about 100 MB of state). `div: 1` is full size (about 400 MB) and takes several times longer.
- The result is one line of JSON: sizes, median/min/max times in ms, and hashes. Paste it back whole.

## How long it runs

About 1 minute on the cloud core at the defaults (see NOTES.md, "Phone part"). The phone should take between 1 and 3 minutes; if it goes over 3 minutes, use `"reps": 2` or `"div": 4`.

## What to compare

- `gen.plates.hash` and `detail.threads_1.hash` against the cloud values in NOTES.md: equal means generation is bit-identical on the phone and in the cloud.
- `save.*.read_ms`: the time to open a saved moment, against the 3-second opening target (`VIS-14`).
