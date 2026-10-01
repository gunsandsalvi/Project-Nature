# B04/B11 phone part: how to bundle it

The crate is `phone/` (package and library `kstorage`). It builds on its own, like the B01/B02 `kbench` crate.

## Build

```
cd pretests/b04-b11-storage-terrain/phone
cargo ndk -t arm64-v8a -P 31 -o <jniLibs folder> build --release --features android
```

This gives `libkstorage.so`, 3.3 MB (SQLite and zstd are compiled in from C by the NDK's clang; no other files). Checked with NDK r30.

## Call it

Java/Kotlin class `dev.kindling.pretests.Storage`:

```kotlin
object Storage {
    init { System.loadLibrary("kstorage") }
    @JvmStatic external fun run(configJson: String): String
}
```

Run it off the main thread. Only `dir` is needed; the rest are the defaults:

```json
{"dir": "<context.filesDir>", "div": 2, "reps": 5, "gen_w": 1024,
 "formats": ["custom-none", "custom-zstd"],
 "parts": ["save", "fsync", "gen", "detail"]}
```

- `dir`: a writable folder on the phone's own storage (use `filesDir`, not the cache folder). The test makes and deletes `dir/b04b11/`. It needs about 150 MB of free storage and about 0.5 GB of memory while it runs.
- `div: 2` is a quarter-size world (118 MB of state). `div: 1` is full size (403 MB) and takes about 4 times as long for the save part.
- Other formats: `custom-lz4`, `sqlite-none`, `sqlite-lz4`, `sqlite-zstd` (SQLite takes about 4 times as long).
- The result is one line of JSON: sizes, median/min/max times in ms, and hashes. Paste it back whole.

## How long it runs

20 s on one cloud core at the defaults. The phone should take under a minute; if it ever goes over 3 minutes, use `"reps": 3`.

## What to compare

- `gen.plates.hash` should be `895e636495687a48` and `detail.threads_1.hash` `5e3b0c482d789a49`, as in the cloud. Equal means terrain generation is bit-identical on the phone.
- `save.custom-zstd.read_ms` is the time to open a quarter-size saved moment; a full one is about 3.5 times larger, against the 3-second opening target (`VIS-14`).
- `fsync.append_day_1000p.fsync_ms` is the cost of flushing one day of the history log.
