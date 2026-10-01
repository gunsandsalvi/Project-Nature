# Phone test app, round 2

Pre-test app (`PRC-08`), throwaway: deleted once the architecture is written. It runs the phone parts of `B04` and `B11` (storage and terrain), `B74` (sound) and `B73` (the writer AI) in one sitting: one install, one tap, a few ratings, one result code (`RSK-23`). It is built on the round-1 shell (`pretests/b78-b79-phone/`): Kotlin screen, Rust libraries, the same result-code format and crash recovery.

## What's in the app

- Package `dev.kindling.pretests`, version `r2` (code 2), signed with the round-1 test key, so it installs over round 1. arm64 only, minSdk 31, targetSdk 36. Release APK: 28.6 MB (debug: 42 MB). Not committed: `dist/kindling-pretests-r2.apk`.
- Permissions: the network (only to download the Gemma model), network state (to check for Wi-Fi first), AICore's binding (added by ML Kit), and one that AndroidX adds for its own use. ML Kit's usage-statistics upload is removed from the manifest, so nothing else uses the network.
- Native libraries, all 16 KB aligned and loaded straight from the APK: `libkstorage.so` (`B04`/`B11`), `libksound.so` (`B74`), and Google's prebuilt `liblitertlm_jni.so` (Gemma).

It runs, in this order, with a progress line for each test:

1. **Phone details:** model, chip, Android version, free storage, Wi-Fi, media volume, battery, and the AICore, Play services and Private Compute Services versions.
2. **Storage and terrain** (`B04`, `B11`; `PLT-07`, `WLD-11`): the `kstorage` library, exactly as its `INTEGRATION.md` says, with `{"dir": filesDir}`. About 20 s in the cloud. Its JSON is kept whole, including the generation and metre-detail hashes (the cloud expects `895e636495687a48` and `5e3b0c482d789a49`).
3. **Sound** (`B74`; `SND-01`, `SND-08`): the screen first says "Sound test: the phone will play clicks and tones for about 35 seconds." Then the `ksound` library plays its test pattern through AAudio with its default settings and reports output delay, dropouts and load at 8, 32 and 128 voices. The media volume is recorded beside it.
4. **Writer AI, Gemini Nano** (`B73`; `PRE-37`): `WriterTest.kt` from `pretests/b73-writer/phone/`, as its `INTEGRATION.md` says, with all its limits (4.5-minute budget, 60 s a prompt, none started with under 15 s left). The only change is a progress hook for the screen. Its 20 prompts are exactly those of `prompts/v2/all-prompts.json` (a unit test compares them text for text). That file holds no settings; the settings are WriterTest's: temperature 0.3, top-k 20, seed 73, at most 256 new tokens. Availability (ready, downloading, unavailable) is recorded per pass; prompts run only when the model is ready. If Gemini Nano is downloading, it waits up to 90 s, then moves on.
5. **Writer AI, Gemma 4 E2B** (`B73`; `PRE-37`, `RSK-15`, `RSK-22`), following `GEMMA.md`, with LiteRT-LM 0.17.1:
   - **Download:** `gemma-4-E2B-it_Google_Tensor_G6.litertlm` (3,313,938,293 bytes) from `https://huggingface.co/litert-community/gemma-4-E2B-it-litert-lm/resolve/<revision>/<file>`, with the revision pinned to the repository's current state (`b3ca0d2f…`), checked against its public file listing. It first checks for at least 4 GB free and for Wi-Fi (it waits up to 3 minutes), and skips Gemma politely if either is missing. It shows the amount done, speed and time left, resumes after any interruption, even after the app closes, and then checks the file's SHA-256 before using it. A "Skip Gemma" button shows during the download; it gives up after 45 minutes. What was downloaded is kept for next time.
   - **Start-up:** GOOGLE_TENSOR first, then GPU if that fails; each try is recorded with its time, its error and the app's own recent log lines, and the one that worked is named.
   - **Writing:** the same 20 prompts and settings as Gemini Nano (top-p 0.95, as in the cloud stand-in run), each in a fresh conversation with "thinking" off, the same limits, and the same measures. If a backend refuses these settings, it steps down to the engine's own, and records that.
   - **Memory** (decision rule 5): the app's resident memory twice a second, split into anonymous and file-backed parts (the model file is mapped, so much of it is reclaimable), plus PSS, graphics memory and the phone's free memory every 4 s. The peaks are recorded for start-up and for writing, and per text.
   - The model is kept for later rounds. The end screen has a "Delete the model (3.3 GB)" button; nothing else deletes it.
6. **Your rating** (`B73` rule 6, `RSK-08`): a short tone, then the feud killing, the dream and the sky-fire sign (`r04`, `r06`, `r08`) in both voices, one at a time, with Poor, Acceptable and Good buttons. Gemini Nano's texts, or Gemma's if Gemini Nano wrote none; if both wrote, both, model by model (12 at most). Each text says which model wrote it and in which voice. "Show the facts" reveals the data it was written from, and "Back" lets you change a rating.

Then the result code, with a Copy button.

## How it stays safe unattended

- As in round 1, a marker names the running test and each result is saved as soon as it is known. After a crash, the next launch reports it and offers Continue.
- Inside the Gemma test, the marker names the part too (download, start-up on one backend, one prompt). A crash there is recorded against that part, and Continue carries on from the next part: the next backend, or the next prompt. After 3 crashes in that test, it is given up.
- Every test runs off the screen thread, inside its own time limit: storage 10 minutes, sound 150 s, Gemini Nano 400 s (beyond its own 270 s budget), Gemma start-up 240 s per backend. A start-up that never finishes is left to end on its own, and the model is never loaded twice.
- A library that fails to load (`kstorage`, `ksound`, ML Kit or LiteRT-LM) is recorded with its reason, and the run goes on. An uncaught exception is written down and reported on the next launch.
- The screen stays on, Back is blocked during the run, rotation can't restart a test, and the results live in one store per app process.
- Round 2 keeps its files in `files/r2/`, so round 1's results on the phone are untouched.

## The result code

- Gzip then base64 of the results JSON, as in round 1, but compact: no spaces, short keys, decimals to 4 significant digits, and each text kept once in a shared list (runs point to it). The texts are all in it, for the cloud fact checker.
- **Size:** with real model texts (the writer test's cloud stand-in runs), 9.6 KB for both models' 20 texts, and 11.5 KB if Gemini Nano's fast variant also writes all 20. The aim is 15.5 KB at most. Above that, the least needed parts go first, in a fixed order: the fast variant's texts, Gemma's log lines, the sound's per-core and offline figures, per-text heat readings, the ML Kit device block, and the storage detail. Gemini Nano's main texts and Gemma's are never dropped. If anything is dropped, the screen says so and shows a "Share results file" button, which sends everything as one file through Android's share sheet.
- **Decoding:** `tools/decode-result.py '<code>'` (or the shared file) prints a summary, compares the terrain hashes with the cloud's, and writes `phone-r2-nano.json` and `phone-r2-gemma.json` for `pretests/b73-writer/check_texts.py`.

## Checks done without a phone

- **Builds:** debug and minified release. The release keeps LiteRT-LM whole, since its native code finds Kotlin methods by name (`SamplerConfig.getTopK`, the message callback, and others), keeps ML Kit whole, as its `INTEGRATION.md` advises, and keeps the app's own classes, so crash traces read plainly. LiteRT-LM is built with Kotlin 2.4, so R8 is pinned to 9.1.56; AGP 8.13's own R8 can't read Kotlin 2.4 metadata.
- **Android lint:** 0 errors, 17 harmless warnings (no translations, no icon, newer library versions, ChromeOS): `results/lint-release.txt`.
- **JVM unit tests:** 25 pass, in debug and release. They cover:
  - the prompts, against `all-prompts.json`;
  - the result code: size, round trip, deduplication and trimming, with real texts;
  - the rating plan;
  - the Gemma measures, the same as WriterTest's;
  - the backend order after a crash;
  - the results store across crashes;
  - the download, against a local server shaped like Hugging Face's: through a redirect, a cut connection, a partial file from an earlier run, a server that ignores ranges, a damaged file, Skip, a missing file, and no Wi-Fi.
- **The native crates' own tests:** pass on x86, and on arm64 under qemu (`kstorage` 1, `ksound` 5).
- **The storage library on arm64 under qemu**, run as the app calls it (`tools/native-run`): the default run works, and gives the cloud's generation hash, metre-detail hash and saved-world content hash bit for bit (`results/storage-arm64-qemu.json`).
- **The APK** (`tools/verify-apk.sh`, `results/verify-apk-release.txt`, 53 checks):
  - It is signed with round 1's key, and its zip entries are 16 KB aligned.
  - It holds arm64 libraries only. All three native libraries are present, uncompressed and 16 KB aligned; each exports its entry points and needs only Android's public libraries.
  - R8 kept every class and method that native code looks up.
  - The package, version and SDK levels are right, and the permissions are exactly the four expected.
  - It declares the vendor GPU and Tensor libraries LiteRT-LM may open, and the share provider. The ML Kit upload is gone.
- **End to end:** a sample code made by the app's code decodes with `tools/decode-result.py`, and the decoded texts go through the writer test's fact checker.
- **Dependencies:** ML Kit and LiteRT-LM share coroutines 1.11.0 and the Kotlin 2.4.0 library, with no version split. Removing ML Kit's upload only drops its events: its library logs a warning and never throws (checked in its bytecode).

## What remains untested (only the phone can tell)

- Nothing in this app has run on a phone: no screen, no JNI call, no ML Kit or LiteRT-LM call, no AAudio stream, no download over the phone's Wi-Fi, no share sheet, no crash recovery. Round 1's shell did run cleanly, and round 2 reuses it.
- **Gemma may not run at all; see the next section.**
- Whether Gemini Nano is ready on the phone, and how its quotas and safety filter treat the dark prompts.
- Whether the phone's speed and timings match the estimates: about 15 to 30 minutes in all, mostly the download.
- The memory readings on the phone, especially graphics memory, which depends on the driver.
- The terrain hashes on the real chip, rather than under emulation.

## For the lead: Gemma's GPU fallback cannot work with this file

I checked the Tensor G6 file without downloading it, by reading its section table and parts of its main section with ranged requests:
- Its decoder is compiled for the Tensor chip's AI unit: it is a `DISPATCH_OP` holding Edge TPU programs and firmware.
- The GPU backend can't run that, so if GOOGLE_TENSOR fails, the GPU try will fail too, and there will be no Gemma texts this round.
- The GOOGLE_TENSOR backend ("GOOGLE_TENSOR_ARTISAN") is closed source. LiteRT-LM's public code says it was enabled by an internal change, and its public engine list has no entry for it, so it may well fail.
- The documented way to run this file is the NPU backend with Google's Tensor dispatch library, which is not in the Maven package. It comes with the Google Tensor SDK (beta, by sign-up) or the LiteRT release files, which are not reachable from here.

The app records each failure, with the runtime's own log lines, so the code will say exactly why. If you want Gemma texts from this round, two options:
- add a second download of the general build (`gemma-4-E2B-it.litertlm`, 2.6 GB; runs on GPU or CPU) when both backends fail; or
- use that build with GPU, then CPU, from the start.

Both change the storage needed (6 GB or 3.3 GB) and the delete button's text. Note that Google's own sample app turns GPU off on the Pixel 10, so CPU may be the safe second choice.

## For the owner

**Before you run it:**
1. In the Play Store, update **Android AICore**, **Private Compute Services** and **Google Play services** (search for each; tap Update if it shows).
2. Be on **Wi-Fi**: the app downloads a 3.3 GB model once.
3. Have **at least 4 GB free** (Settings, Storage).
4. **Plug in the charger**, or have the battery **above 60%**.
5. Set the **media volume to about half**: one test plays clicks and tones.

**Install it over round 1:**
1. Open the file `kindling-pretests-r2.apk` that the lead sends you, and tap it. Don't uninstall round 1 first.
2. Android asks whether to update "Kindling tests": tap **Update**. If it says your browser or files app can't install apps, tap Settings, turn on "Allow from this source", go back, and tap Update.
3. If Play Protect warns about it, tap "More details", then "Install anyway".

**Run it:**
1. Open "Kindling tests" and tap **Run all tests**. Then leave the app open, with the phone on a table. The screen stays on.
2. What happens, in about 15 to 30 minutes:
   - storage tests, with nothing to see (under a minute);
   - a notice, then about 35 seconds of clicks and tones;
   - Gemini Nano writes 20 short texts (up to 5 minutes);
   - Gemma's model downloads, once. That takes a few minutes on fast Wi-Fi; you can tap "Skip Gemma" if it is too slow. Then Gemma starts and writes 20 texts (usually a few minutes).
3. If the app closes, open it again and tap **Continue**.

**Rate the texts:** when the phone beeps, it shows 6 short texts, or 12 if both models wrote. Read each one and tap **Poor**, **Acceptable** or **Good**. Judge how well it reads as an entry in the game's history, and whether its voice comes through. The facts are checked separately. "Show the facts" shows what it was written from, if you're curious, and "Back" lets you change your mind.

**Paste back:** tap **Copy result code** and paste it into the chat. If the screen says the code could not hold everything, also tap **Share results file** and send that file in the chat.

**Afterwards:** the Gemma model stays on the phone for later rounds. To free its 3.3 GB, tap **Delete the model (3.3 GB)**.

## How to re-run

```
cd pretests/phone-r2
export CACHE=<the shared download cache folder>   # toolchain from pretests/b78-b79-phone/tools/setup-toolchain.sh
. tools/env.sh
(cd app && flock $LOCK gradle assembleDebug assembleRelease testDebugUnitTest testReleaseUnitTest lintRelease)
cp app/app/build/outputs/apk/release/app-release.apk dist/kindling-pretests-r2.apk
tools/verify-apk.sh dist/kindling-pretests-r2.apk
tools/decode-result.py '<pasted code>' <out folder>
```

The native checks: each crate's `cargo test`, then the same with `--target aarch64-unknown-linux-gnu`, run under qemu, with `CARGO_TARGET_DIR` in the cache. The storage library's phone entry point: `tools/native-run` (`native-run storage '{"dir":"<tmp>"}'`), on x86 or under `qemu-aarch64-static`. Gradle builds the other pre-tests' crates from their own folders, with `--locked`, and puts every output in the cache.

## Files

- `app/`: the Gradle project.
  - `MainActivity.kt`: the screen and the run.
  - `Store.kt`: results and crash markers.
  - `Logic.kt`: the result code, the rating plan and helpers.
  - `Probe.kt`: phone readings.
  - `Natives.kt`: the storage library.
  - `Sound.kt`: copied from `b74`.
  - `WriterTest.kt`: copied from `b73`, plus a progress hook.
  - `GemmaTest.kt`: the Gemma test.
  - `ModelDownload.kt`: the resumable, checked download.
- `tools/`:
  - `env.sh`: the build environment.
  - `verify-apk.sh`: the APK checks.
  - `decode-result.py`: decodes the owner's code.
  - `native-run/`: runs the storage and sound libraries' entry points.
- `results/`:
  - `r2-build.json`: sizes, hashes, versions and checks.
  - `lint-release.txt`: the lint report.
  - `verify-apk-release.txt`: the APK check report.
  - `storage-arm64-qemu.json`, `storage-x86.json`, `sound-x86-offline.json`: the native runs.
