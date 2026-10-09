# Kindling α2.13e: Tap and hold the camp

## What is new

Tap a person to select them. Hold still for at least 550 milliseconds to open their dream ring. Dragging or pinching cancels the hold; turning the phone clears an unfinished gesture. This fixes the touch failure reported on 31304.

## What to try

Install **31305** over your current build, then open your saved camp upright:

1. Tap two different people and check that the card names change.
2. Hold one person still for a little over half a second. Check the ring names that person, then Cancel. Drag and pinch without opening a ring. Turn the phone and repeat.
3. Hold someone → Dream of a place → choose a remembered place → Send dream. Save and reopen while it waits; Menu → Your dreams keeps the request. Use one hour/sec briefly until they dream, then follow their waking choice at one minute/sec. Details and Your dreams retain the reasons and times.

Did the decision make sense? Did the dream matter without commanding them? Was it comfortable to use?

Separately, the ten-minute phone measurement still waits: Menu → Developer tools → Camp performance test, cool and unplugged, flight mode. Tap Run camp test and leave it running, then Copy report. It uses an isolated camp.

[Download the APK](https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk) · [Checksum](https://github.com/gunsandsalvi/Project-Nature/blob/ccr-13ab6fef-fspju6/dist/kindling.apk.sha256)

## What is rough

Physical-phone touch confirmation and measurements are **pending**. The living camp still uses finite supplies and simple poses; discovery follows in a separate build. M2 play acceptance stays open.

Validation: paired finger and emulated mouse events through the viewport reproduced the old failure and pass with the fix. Six touch regressions cover selection, the hold deadline, drag, two fingers, rotation and cancellation; the shell test now uses viewport touch too. Portrait 1080×2400 captures were inspected first, then landscape 2400×1080: two distinct people, the ring and the dream chooser. These are display-backed software-renderer checks, not physical-phone results. Save layouts and simulation rules are unchanged.

Committed routine checks passed: 94 app tests, 177 tool tests, native/view tests, catalogue, determinism proofs, style and document checks. The final delivery gate follows this APK/note commit.

**Signed release:** 31305, 52,257,161 bytes (49.84 MiB), with the registered release key. All resources are retained.

Measured builder time through packaging: about 21 minutes, including reproduction, fix, tests, self-review and portrait-first captures. Committed routine: 4 minutes; signed build: 105 seconds. No independent review is required for this input-only fix under the current brief.
