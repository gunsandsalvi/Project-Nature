# Kindling α1.5b: The benchmark

This step ends the foundations (M1). Its report, for your review, is linked at the end.

## What is new

- **Bench.** A new **Bench** page runs the phone benchmark: one tap and about 19 minutes. It runs seven scenarios in turn, each with a world of its own, kept apart from yours and deleted after:
  1. the calendar alone at top speed;
  2. 10,000 markers at real speed, the camera touring: circling, zooming in and out, and turning;
  3. the same at top speed, its speed read after 3 minutes;
  4. the same with the world's thread pinned to the middle cores;
  5. a sweep through every speed, 20 seconds each;
  6. saves every 30 seconds, with an export to a file and the world closed and opened again;
  7. a still camera, for the screen's own power.
- **The same end as the cloud's.** Each scenario's world takes a fingerprint of its whole state at a set moment, and the page compares it with the fingerprint the cloud worked out for the same world. Each line says whether its world ended as the cloud's.
- **Pass lines, set before the first run.** At least 97% of frames on time and none slower than 66 ms while the camera moves; a world opened again within 3 seconds; and every world ending as the cloud's. The page marks any line it missed, and the code turns yellow if one was.
- **A code to paste.** At the end the page shows a code of 218 letters, with a **Copy the code** button. Pasted into the chat, the cloud reads it back, checks every letter, and turns it into your phone's numbers.
- **Safer saves,** from M1's independent review: if the phone's storage fails or fills, the world stops and says so, and opens again whole where it was last saved; an imported world is made safe on the phone as it arrives.
- **The pages in two rows of four,** to make room for Bench; the open page's tab now shows pressed rather than greyed out.

![The Bench page after a run in the cloud](pictures/a15b-bench.png)

The picture is the cloud's run, a hundred times faster than yours. The cloud draws in software, so none of its frames are on time and its frame lines show as missed, in red; on your phone every one was met. Every world still ended as the cloud's own.

## Your phone's run

You ran the first build of 20502 this morning: all 18 pass lines met, every world ending as the cloud's, top speed 4.3 game days a second, and every frame on time but a handful. The numbers are in M1's report. There is no need to run it again.

## What to try

1. Install the APK from the link below, over this morning's 20502. Your worlds carry on.
2. Open **Check**: every line should be green.
3. Open **Crowd**, then **Worlds**: everything is where you left it.

## What is rough

- The power rails are not read yet: the battery's current stands for the phone's whole power.
- The screen still runs at 120 Hz while the app asks for 60 frames a second, which costs some battery: for M2.
- The Catalogues page shows raw numbers, such as "20000000 mm" for 20 km.
- If you leave the app during a benchmark run, its numbers are spoiled: run it again.

## IDs delivered

- `PLT-04`: the phone benchmark, with frames timed by the app itself, the speed held, heat, battery, memory, the world's share of a core, the crowd's drawing, saves, export and reopening, in one code read back in the cloud.
- `RES-05`: each scenario's world ends exactly as the cloud's run of it, compared on the phone, and on yours it did.
- `PLT-01`: the world's thread, pinned and unpinned, measured on your phone; it stays unpinned.
- `RES-09`: the pass lines stated in the scenarios before the first run.
- `PLT-07`, `PLT-08`: saves stop whole after a failed write; imports made safe as they arrive.
- `RES-06`, `RES-22`: M1's report, for your review.

## Links

- APK: https://github.com/gunsandsalvi/Project-Nature/raw/ccr-13ab6fef-fspju6/dist/kindling.apk
- M1's report: https://github.com/gunsandsalvi/Project-Nature/blob/ccr-13ab6fef-fspju6/dist/M1-REPORT.md
- This note: https://github.com/gunsandsalvi/Project-Nature/blob/ccr-13ab6fef-fspju6/dist/NOTE.md
- How the benchmark works: https://github.com/gunsandsalvi/Project-Nature/blob/ccr-13ab6fef-fspju6/ARCHITECTURE.md
