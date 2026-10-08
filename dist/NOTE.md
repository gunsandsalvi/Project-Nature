# Kindling α2.13a: Camp alpha

## What is new

The app opens on a saved camp of 25 named adults, with a bounded patch, water, food plants, stone, fallen wood and natural shelter. Tap a person for their actual saved name, age at start and position. Large Pause and Speed controls stay on screen in both orientations; one Menu holds saved camps and developer tools. Zoom settles over 320 ms, with readable labels throughout.

People and initial quantities survive save/reopen, camp switching and export/import. Old marker worlds remain usable. Save failures visibly stop play; damaged camp records are refused. The baseline import, Examples and texture lifetime repairs are included.

## What to try

1. Install **31301** over the previous build. Tap two different people, drag, pinch, and turn the phone. Report a first-start code if one appears.
2. Try Pause and Speed. Use **Menu → Save camp now**, close the app, then reopen the same camp.
3. Open **Menu → Saved camps · export / import**. Make and switch camps; export one, import its copy, and compare its people. **Camp supplies** shows the initial quantities.

[Download the APK](https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk) · [Checksum](https://github.com/gunsandsalvi/Project-Nature/blob/ccr-13ab6fef-fspju6/dist/kindling.apk.sha256)

## What is rough

Adults are idle stand-ins in a regular starting grid; movement comes next. Needs, consumption, tasks, discovery and survival are still ahead; this is a saved bounded camp, not generated geography. Age is explicitly age at start. Resting zoom still uses the existing discrete levels. Phone frame time, power, heat and touch feel need owner testing; inspected cloud captures establish composition only. Developer fixture pages remain unfinished art.

Validation: full persistence/determinism audit passed, including arm64, one/four workers, races, kill recovery and real corrupt-save tests. Front door tests cover picking, pause/speed, switching, export/import and failed opens. Portrait 1080×2400 and landscape 2400×1080 were captured and inspected.

**Signed release:** 31301, 52,154,001 bytes (49.74 MiB). Lossless PNG/native repacking keeps all resources and decoded pixels.

Rounded wall time, with overlapping work: ~40 min code/build, ~70 min checks, ~6 min self-review/captures, ~9 min packaging (final signed build 88 s). Restart and paused independent-review time are excluded.
