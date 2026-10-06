# Kindling α1.5b: The benchmark

This step ends the foundations (M1). Its report, for your review, is linked at the end.

## What is new

- **Bench.** A new **Bench** page runs the phone benchmark: one tap and about 17 minutes. It runs seven scenarios in turn, each with a world of its own, kept apart from yours and deleted after:
  1. the calendar alone at top speed;
  2. 10,000 markers at real speed, the camera touring: circling, zooming in and out, and turning;
  3. the same at top speed;
  4. the same with the world's thread pinned to the middle cores;
  5. a sweep through every speed, 20 seconds each;
  6. saves every 30 seconds, with an export to a file and the world closed and opened again;
  7. a still camera, for the screen's own power.
- **The same end as the cloud's.** Each scenario's world takes a fingerprint of its whole state at a set moment, and the page compares it with the fingerprint the cloud worked out for the same world. Each line says whether its world ended as the cloud's.
- **Pass lines, set before the first run.** At least 97% of frames on time and none slower than 66 ms while the camera moves; a world opened again within 3 seconds; and every world ending as the cloud's. The page marks any line it missed.
- **A code to paste.** At the end the page shows a code of 218 letters, with a **Copy the code** button. Pasted into the chat, the cloud reads it back, checks every letter, and turns it into your phone's numbers: frames on time, the slowest frame, the speed held, the heat and the fastest core's clock, the battery's current, memory, the world's share of a core, the crowd's drawing time, the saves' pauses, the export and the reopening.
- **The pages in two rows of four,** to make room for Bench.

![The Bench page after a run in the cloud](pictures/a15b-bench.png)

The picture is the cloud's run, a hundred times faster than yours. The cloud draws in software, so its frames are slow and none count as on time; your phone's are the ones that count.

## What to try

1. Install the APK from the link below. It installs over α1.5a, and your worlds carry on.
2. Make sure the battery has at least a third left, unplug the phone, turn on flight mode, and let it cool for a few minutes.
3. Open **Bench** and tap **Run**. Leave the phone alone until the code shows, about 17 minutes. The screen stays on by itself.
4. Each scenario adds a line as it ends. Every line should end "its world ended as the cloud's".
5. Tap **Copy the code** and paste it into the chat. I'll read it and add your phone's numbers to M1's report.

## What is rough

- The power rails are not read yet: the battery's current stands for the phone's whole power.
- The code is long, because it carries every measure; the Copy button saves typing it.
- If you leave the app during the run, its numbers are spoiled: run it again.
- The heat forecast needs a few readings before it can look ahead, so the first scenario's heat may be missing.

## IDs delivered

- `PLT-04`: the phone benchmark, with frames timed by the app itself, the speed held, heat, battery, memory, the world's share of a core, the crowd's drawing, saves, export and reopening, in one code read back in the cloud.
- `RES-05`: each scenario's world ends exactly as the cloud's run of it, compared on the phone.
- `PLT-01`: the world's thread, pinned and unpinned, measured on your phone.
- `RES-09`: the pass lines stated in the scenarios before the first run.
- `RES-06`, `RES-22`: M1's report, for your review.

## Links

- APK: https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk
- M1's report: https://github.com/gunsandsalvi/Project-Nature/blob/ccr-13ab6fef-fspju6/dist/M1-REPORT.md
- This note: https://github.com/gunsandsalvi/Project-Nature/blob/ccr-13ab6fef-fspju6/dist/NOTE.md
- How the benchmark works: https://github.com/gunsandsalvi/Project-Nature/blob/ccr-13ab6fef-fspju6/ARCHITECTURE.md
