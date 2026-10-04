# Research 16: testing

**Question:** how should a Godot game with a C++ simulation be tested by AI builders in the cloud, so each step is proven before it reaches your phone?
What can run without a graphics chip, and what must be measured on the phone itself (`RES-01`, `RES-05`, `RES-13`, `RES-21`, `PLT-04`, `PLT-05`, `PRC-10`)?

## What `PROJECT.md` asks

- **Tests lead:** every alpha brings tests for what it adds; quick ones before any work joins, long ones in the background (`RES-01`, `PRC-10`).
- **Scenes, then whole worlds,** with the same rules as play and nothing scripted (`RES-21`, `RES-18`).
- **About 20 runs where chance matters,** judged by counts such as "16 of 20" (`RES-13`).
- **Repeatable runs:** the same saved world gives the same result every time, on the phone and in the cloud (`RES-05`).
- **Switch-off runs, trials, flagged oddities, pace tests** (`RES-10`, `RES-24`, `RES-12`, `RES-07`).
- **Cloud runs without picture or sound,** several worlds at a time (`PLT-05`).
- **Limits measured on the phone** (`PLT-04`), and your reviews of look and sound (`RES-22`, `PRE-31`, `SND-12`).

## How others do it

- **Factorio** leaned on automated tests to tame multiplayer desyncs: "we try to cover the most tricky situations and the most complicated parts of the code first, so it helped to find lot of problems already" ([Factorio Friday Facts #62](https://www.factorio.com/blog/post/fff-62)).
  - Its build server runs "the automated test suites in all the systems and possible configurations".
  - That release alone fixed "8 desync bugs".
  - Our same-results check across cloud and phone is the same fight (research 03).
- **Fast C++ tests:** doctest is a "single-header testing framework", "ultra light on compile times both in terms of including the header and writing thousands of asserts" ([GitHub: doctest](https://github.com/doctest/doctest)).
- **Tests that search for counterexamples:** RapidCheck is "a C++ framework for property based testing inspired by QuickCheck".
  It generates random cases for a rule that must always hold, and "automatically shrinks" a failure "to the smallest counterexample" ([GitHub: RapidCheck](https://github.com/emil-e/rapidcheck)).
  That suits the reality rules and "nothing from nothing" (`RCK`, `MAT-09`).
- **Godot's test frameworks:**
  - **GUT** "allows you to write tests for your gdscript in gdscript", with a command-line runner ([GitHub: GUT](https://github.com/bitwes/Gut)).
  - **gdUnit4** adds a scene runner that can "simulate different kinds of inputs and actions", including "Touch screen interactions" ([GitHub: gdUnit4](https://github.com/MikeSchulze/gdUnit4)).
    It has a "Command Line Tool" for CI and a "JUnit XML Report".

## Can Godot do it?

| Need | What exists | Verdict |
|---|---|---|
| Runs with no screen | Godot's `--headless`: "Useful for servers and with `--script`" ([command line](https://docs.godotengine.org/en/stable/tutorials/editor/command_line_tutorial.html)) | Yes; most simulation tests skip Godot entirely and run the C++ library alone |
| Frames independent of speed | `--fixed-fps`: "This setting disables real-time synchronization" | Yes |
| The same pictures and reels every time | Movie Maker mode renders offline with "perfect frame pacing; it will never exhibit dropped frames or stuttering", to PNG frames plus WAV ([Godot docs: creating movies](https://docs.godotengine.org/en/stable/tutorials/animation/creating_movies.html)) | Yes: golden pictures, contact sheets (`PRE-31`) and sound reels (`SND-12`) |
| Drawing in the cloud without a graphics chip | Mesa's llvmpipe, "a software rasterizer that uses LLVM", "multithreaded", and lavapipe, its Vulkan driver ([Mesa: llvmpipe](https://docs.mesa3d.org/drivers/llvmpipe.html)) | Yes, as in the bake-off; pictures are compared within a tolerance, since the phone's chip may round differently |
| Tapping and swiping in tests | gdUnit4's scene runner with simulated touch | Yes |
| Seeing inside the phone | Perfetto traces CPU scheduling and frequency, power rails, thermal readings, GPU counters and "application trace events", viewed in an offline browser UI ([Perfetto](https://perfetto.dev/docs/)) | Yes |
| The graphics chip in detail | Android GPU Inspector: "Imagination® PowerVR™ GPUs are supported", with Vulkan frame profiling ([AGI](https://developer.android.com/agi)) | Yes, for your phone's PowerVR chip |

**Verdict:** everything the tests need exists.
The one limit: the cloud cannot show what the phone's own chip draws, so golden pictures only guard our code, and the look is judged on the phone (`PRE-31`).

## What we take

1. **C++ tests with doctest** for the simulation library, plus **property tests with RapidCheck** for rules that must always hold.
   Examples: no result heavier than its inputs (`MAT-09`), and no stone below flaking 3 giving an edge (`RCK-01`).
   Shrinking hands the builder the smallest failing case.
2. **Scenes and whole worlds run by the C++ library alone,** with no Godot, many at once in the cloud (`PLT-05`).
   Their pass rules count runs (`RES-13`).
3. **Same results everywhere:** state hashes per tick compared between x86 and arm64 (under qemu), and between one and four threads, as research 03 sets out (`RES-05`).
   Factorio's desync war shows why this must exist from the first step.
4. **The Godot side tested with gdUnit4:**
   - gestures by simulated touch (`PRE-33`);
   - cards and views opened from records (`PRE-35`);
   - headless, with JUnit reports.
5. **Pictures and reels by Movie Maker mode at a fixed frame rate:**
   - golden pictures drawn on lavapipe in the cloud, approved by you when they change;
   - the same tool on the phone makes the contact sheet and the sound reel for your reviews (`PRE-31`, `SND-12`).
6. **Phone measurements:** the in-app benchmark (`PLT-04`) writes our own trace events into Perfetto traces, so each frame's and each mind part's time sits beside the chip's speed and temperature.
   AGI digs into the PowerVR chip when a frame is slow.
7. **One command before anything joins.**
   Today's `tools/check.sh` still runs the old Rust stack, so it must be rebuilt for C++ and Godot.
   That is foundations work (`PRC-10`).
8. **The test harness is part of pre-production:** the vertical slice must pass through every kind of check above before production starts.

## Sources

- Practice:
  - [Factorio Friday Facts #62](https://www.factorio.com/blog/post/fff-62)
  - [GitHub: doctest](https://github.com/doctest/doctest)
  - [GitHub: RapidCheck](https://github.com/emil-e/rapidcheck)
  - [GitHub: GUT](https://github.com/bitwes/Gut)
  - [GitHub: gdUnit4](https://github.com/MikeSchulze/gdUnit4)
- Godot and tools:
  - [Godot docs: command line](https://docs.godotengine.org/en/stable/tutorials/editor/command_line_tutorial.html)
  - [Godot docs: creating movies](https://docs.godotengine.org/en/stable/tutorials/animation/creating_movies.html)
  - [Mesa: llvmpipe](https://docs.mesa3d.org/drivers/llvmpipe.html)
  - [Perfetto](https://perfetto.dev/docs/)
  - [Android GPU Inspector](https://developer.android.com/agi)
