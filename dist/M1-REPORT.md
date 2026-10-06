# M1 Foundations: the report

M1 is built: eleven steps in five alphas, delivered as 20101 to 20502 on 5 and 6 October 2026.
It is the plumbing the whole game stands on. There is no game to play yet: what you can see are its working parts, each on a page of the app.
This report is for your review (`RES-06`, `RES-22`): accept it, or send it back with what is wrong.

## In short

- **The same bits everywhere.** Your phone and the cloud compute exactly the same numbers, chances and worlds. Your phone's self-check matched the cloud in all seven suites, and in the benchmark all seven worlds ended exactly as the cloud's (`RES-05`).
- **A world that keeps.** A crowd of 10,000 markers lives on its own thread, is saved every 30 seconds, and opens again exactly where it was, even after a crash, an update or a move to another phone. On your phone it reopened in 143 ms.
- **Smooth on your phone.** With the camera touring and 10,000 markers moving, 99.8 to 100% of frames were on time at every speed, and top speed held 4.3 game days a real second.
- **Tests stated before they run.** Scenes say in advance what they check and how they pass, run many worlds at once in the cloud, and report on a page of the app.

## What was added, and what you can try

Each page of the app is one part of the foundations.

| Page | What it shows | Step |
|---|---|---|
| **Check** | The phone computing the same bits as the cloud: maths, chance, the torus, units, a small world, islands; the catalogue's files; the last save and free space; the screen's rate and the heat forecast. Every line green means your phone agrees with the cloud. | α1.1a–α1.4b |
| **Time** | The calendar: 60-day years in four seasons, at any speed from real time to as fast as the phone can, with the speed it really runs at. | α1.2a |
| **Catalogues** | Everything the game knows, read from plain text files with their checks: today only the demonstration's markers and the tuning. | α1.2b |
| **Crowd** | 400 camps of 25 markers walking, resting, meeting, greeting and sleeping, drawn exactly where the world has them, at any speed; tap a camp to call it home. | α1.3a–α1.4a |
| **Worlds** | Several worlds kept, switched, renamed, deleted, exported to a file and imported; a warning before the phone is full. | α1.4b |
| **Reports** | The cloud's last test scene: its rule, its result, a chart for each measure, and the world it ran, which opens on your phone exactly as it ended. | α1.5a |
| **Bench** | The phone benchmark: seven scenarios, about 19 minutes, ending in a code. | α1.5b |

![The Crowd page](pictures/a14a-crowd.png)
![The Reports page, with the greetings scene's charts](pictures/a15a-reports.png)
![The Bench page after a run in the cloud](pictures/a15b-bench.png)

Underneath, in the simulation library (C++, the same code on phone and cloud):
- numbers with correctly rounded maths, a world that wraps round like a torus, units and durations, and chance drawn by key so it never depends on order;
- entities with ids never reused, one event queue, activities that always end, and islands that give exactly the one-core result on any number of cores;
- catalogues read exactly from text, with their checks and planted faults, and sources with fingerprints;
- saves that write your commands at once, a journal, snapshots off the screen's thread, damage refused, updates that carry old worlds on, history that thins after 25 game years;
- scenes, runs, checkpoints, the repeat check, reports; and the benchmark.

## The tests, and where they ran

All ran in the cloud sessions where M1 was built (`SCP-15`): one container with 4 cores (Intel Xeon at 2.1 GHz) and 15 GB of memory, nothing else. M1 took about 12 hours of session time, from 5 October at 20:00 to 6 October at 08:30 (UTC). The whole check, `tools/check.sh`, takes 5 to 15 minutes of the 4 cores, depending on what its caches can skip, and every step passed it before it was delivered (`PRC-10`).

- **105 C++ test cases** (95 for the simulation, 10 for the app's own C++), with millions of checks inside them; **31 tests of the app**, its pages run headless in Godot; **37 tests of the tools**.
- The simulation is built **seven ways** and tested in each: with two compilers, one of them with a checker for undefined behaviour; with a thread checker; two ways for the phone's processor, run under an emulator, where its numbers must match the cloud's bit for bit; and twice more with the order of equal keys shuffled, so no result depends on it.
- **Every new test was made to fail once** on purpose, by a fault planted in the code it guards, so none passes by accident.
- **The repeat check**, in every check: a scene and the 10,000-marker world each run twice, once on one core and once on four with a stop and restart in between, and must end identical to the byte.
- **The greetings scene** runs 20 worlds of the crowd for 20 game days each, and passes when at least 16 meet its rule. Its report, with a chart for each measure, is on the Reports page (picture above).
- **The pace** (`RES-07`) and **something to watch** (`RES-25`) need a game with people; they start with M5.

## Your phone's numbers

From your benchmark run of 20502 on 6 October (its code is at the end), on a phone with 7 cores (the fastest at 4.1 GHz), a PowerVR C-Series graphics chip and Android API level 37. Every one of its 18 pass lines was met.

| Scenario | Frames on time | Slowest frame | Speed held | Crowd drawn in | Power | Heat forecast |
|---|---|---|---|---|---|---|
| Calendar alone, top speed | 100% | 25 ms | 965 game days a second | — | 3.4 W | 0.60 |
| 10,000 markers, real speed, touring | 100% | 26 ms | real time | 3.6 ms | 1.0 W | 0.61 |
| Top speed, touring | 100% | 25 ms | 4.3 game days a second | 3.6 ms | 5.6 W | 0.77 |
| Top speed, pinned to the middle cores | 99.9% | 49 ms | 4.1 game days a second | 3.6 ms | 5.0 W | 0.83 |
| Sweep through every speed | 99.8% | 34 ms | (each speed in turn) | 3.1 ms | 2.5 W | 0.83 |
| Saves, export and reopening | 99.9% | 227 ms, at the reopening | 6 game hours a second | 2.4 ms | 1.2 W | 0.82 |
| Still camera, real speed | 100% | 22 ms | real time | 3.4 ms | 1.1 W | 0.73 |

- **Smoothness** (`PLT-04`): at least 97% of frames on time and none over 66 ms while the camera moves: met everywhere, with room to spare.
- **Opening** (`PLT-04`): your world reopened in 143 ms, against 3 seconds; the export took 23 ms, and a save paused the world 20 ms at most.
- **Heat:** the forecast stayed under 0.85 of the first throttling level, so the heat guard never had to slow time.
- **Battery** (`PLT-04`: an hour's play about 25 to 30% of the battery): at real speed about 1 W, a few percent an hour; at top speed 5.6 W, about 30% an hour, at the edge of the target.
- **Memory:** 330 to 410 MB, far within 8 GB.
- **Pinning** the world to the middle cores gave nothing (4.1 against 4.3 game days a second), so the world stays unpinned, as Android advises.
- **The cost to watch:** drawing the crowd takes 3.4 to 3.6 ms of each frame on your phone, against the 0.26 ms hoped for and 1 to 1.6 ms in the cloud. It fits today; it is the first cost to win back when figures replace squares (M2).

Your phone's self-check, on the same build: every line green, the same bits as the cloud in all seven suites, the graphics driver and cores as expected. Its one amber line is the screen: it still runs at 120 Hz while the app asks for 60 frames a second, which costs some battery; that is for M2, where the frame rate is set with the drawing.

## Moments, oddities and surprises

- **Your phone is twice as fast as the cloud** at the crowd: 4.3 game days a second at top speed against about 2, its fastest core beating the cloud's.
- **Islands were slower, not faster.** Running the crowd on four cores in islands gives exactly the one-core result, as designed, but takes longer, on your phone too (1,027 ms on four threads against 696 on one): the markers' events are too light to share. The crowd stays on one core; islands wait for people's minds (M6), whose events weigh more.
- **A race found by its own test.** In α1.5a the test that plants faults in a run found memory corruption: two threads used the same store of files at once. It is fixed (the keeper now does all of a world's reading and writing), and a thread checker now runs over the simulation's tests in every check; the app's own threads are not yet under it.
- **History is bigger than estimated.** A record of the history takes 68 bytes as written, so a crowd's world grows about 31 MB a game year. It is capped by thinning after 25 game years; the real fix, compressing each year as it closes and smaller records, is planned for M5, when people's lives fill the history.
- **The independent review found a gap in saving** (below): after a write failed, as on a full phone, saving carried on past the gap and could lose history. Fixed before M1 closes.

## The principles

Each principle's check, as it came out for M1 (`PRN-16`). Most need people, crafts or a story, which M1 does not have yet.

| Principle | Its check in M1 |
|---|---|
| `PRN-01` The world is the only teacher | Not yet testable: no discoveries or minds. |
| `PRN-02` Believable over exact | Nothing built whose detail you cannot see or read: every part has a page. |
| `PRN-07` Generic blueprints | Not yet testable: no blueprints. The catalogue's checks that will enforce it exist (`MAT-17`). |
| `PRN-05` Plausible numbers | The catalogue tests pass, with planted faults; every claim here is backed by a test that can fail, or by your phone's benchmark (`RES-01`). |
| `PRN-12` Speed up time, never bend the rules | Held: the same world gives the same result at every speed and on any number of cores; test switches exist only in the cloud's test builds, and a check keeps them out of the app. |
| `PRN-17` History at a watchable pace | Not yet testable: no pace tests before people (M5). |
| `PRN-03` You are nature | Not yet testable: no powers. Your one command, calling a camp home, is a test command of the demonstration, and acts slightly ahead of what you see (below). |
| `PRN-04` If the game knows it, you can see it | Held for what exists: every part has a page, and the self-check shows the hidden ones. |
| `PRN-10` Nothing is faked | Held: the crowd is drawn exactly where the world has each marker, tested at 1,780 moments. |
| `PRN-13` Every choice can be explained | Not yet testable: no choices. |
| `PRN-06` Language models describe, never decide | Held: no language model in the game. |
| `PRN-15` History is saved, not re-run | Held: the history is written as it happens and read back, never re-run; after a crash only what came after the last save is worked out again, checked against what was written. |
| `PRN-11` Time slows, the screen stays smooth | Held on your phone: every scenario smooth at every speed; the heat guard was never needed, and a test shows it slowing time as the forecast nears throttling. |
| `PRN-09` Build in steps you can try | Held: every alpha ended with a build to install, and every task names its items. |
| `PRN-14` Modular by design | Held: each part was added on top of the last; earlier parts were extended, and none had to be rewritten. |

## Risks

- **Drawing:** 3.6 ms a frame for 10,000 squares on your phone; figures will cost more, so M2 must draw the crowd more cheaply.
- **History size:** compression and smaller records are needed by M5.
- **Islands** must prove faster once events are heavy (M6); until then one core runs the world, with the same results.
- **Battery at top speed:** about 30% an hour, at the edge of `PLT-04`'s target; the full game's world costs more, so top speed may need to give some speed for battery.
- **Content for later stages** (`RSK-25`): the catalogue, saves and scenes were built for the whole game's kinds of content, not only the markers; none of it needed changing to take the demonstration in.

## The independent review

One independent reviewer, given only M1's changes, its plan as it stood when M1 began, and the items it claims, checked the whole milestone, ran its tests, drew every page at several hours, and judged them as a pixel artist and art director would.

**Its verdict: approve with fixes.** It found no determinism bug, no race and no save lost in normal running, and every claimed item built and tested, but for what only you can do: your review (`RES-22`) and, at the time, your phone's numbers.

What it asked, and what was done:
- **The report overstated a few things** (blocking): fixed here. The same bits are now shown on your phone; the cloud's numbers are its current ones (1.4 µs an event, 1 to 1.6 ms of drawing); the cloud's "every frame on time" came from a run without drawing, so the smoothness claims now rest on your phone.
- **A failed write didn't stop saving:** on a full or broken phone, saving went on past the gap, so history could be lost behind a later snapshot. Now a failed write stops all saving: your command is refused, the world stops and says so, and the folder opens whole where it was last safe. A new test fails one write and checks the world reopens whole; it fails against the old code.
- **An import wasn't made safe on disk:** each part is now synced as it ends, and the world's new folder name with it.
- **A damaged snapshot could ask for 4 GB of memory** before its checksum was read: the whole file's hash is now checked first.
- **The check could skip the old-saves test** when only the catalogue changed: the tests now rerun whenever the data or the old-saves corpus changes.
- **The benchmark couldn't settle pinning fairly,** and its sweep took its fingerprint before its fast speeds: top speed is now read after 3 minutes, as `PLT-04` asks, and the sweep's world is fingerprinted at day 30. Your run settled pinning anyway: it gave nothing.
- **A test was shortened without a reason:** the 1,000-marker test runs 30 game days, not the plan's 60; the reason (check time, with the scenes running the crowd longer) is now written with it.
- **One palette written in seven scripts:** now one file, with headings in their own colour apart from the warning yellow; the heat guard's numbers come only from its tuning file.
- **Smaller notes,** fixed: the save counter read across threads is now safe; the greetings scene no longer claims an item it doesn't test; the old-saves corpus has an α1.5b world.
- **Left for later, and written down:** your tap on a camp acts at the world's frontier, slightly ahead of what you see, hours ahead at top speed: before the first real power (M9), a tap will pause the world and act at the moment shown. Islands' guard against reading another island needs fixing before minds use islands (M6), and two speed-ups wait for when the crowd is heavier. The heat readings the reviewer worried about worked on your phone.

**The look,** in its words, in short: a calm, consistent dark theme with text that reads well upright; Reports is the best page. Against it: the open page's tab looks disabled; at the opening zoom the 10,000 markers are specks; 400 camps in 400 pastel colours read as confetti; Catalogues shows raw numbers ("20000000 mm" for 20 km); the landscape view shrinks all text; and a finished benchmark's code stays green even when scenarios miss. These are plain plumbing pages, replaced by the game's own look and pages from M2 on. Two are fixed now: the open page's tab shows pressed, and the code turns yellow when a scenario missed a line; the catalogue's raw numbers go into the next step.

## What needs your judgement

1. **Accept M1, or send it back** with what looks wrong (`RES-22`).
2. **What comes next,** when you are ready: M2 waits, as you asked. The plan proposes the vertical slice before M2: one band at a cliff camp through a day, at the art book's look, built on these foundations, as the bar for the rest. Either needs the same first answer: the best way for Claude to make models, textures, shaders and meshes, which M2's research will find. Say which comes first, and when.
3. **This report as a page on your phone** (`RES-06`): publishing it as a page stops my work until you allow it, so it stays in the repository for now. Say if you want it published.
4. **Once, if not yet done:** register the package `dev.kindling.app` and the release certificate's fingerprint in your developer account, and make `main` the default branch on GitHub.

---

Your benchmark's code, layout 1 (20502 as first built, 4de8ea2):
`0581C-F98YM-TT2E2-ZMG6G-01D6F-5NC8Y-G0DM0-MYS80-2DZMG-6R020-1DT9S-0ZCM4-40PM0-HCRHF-MG6G0-19980-8017C-MPRGR-YS1CP-9ZM0C-G0A98-80801-ACMM7-GRTS1-CP5ZK-G8R0G-00CS3-3SACM-A9GSM-B16N3-FM1S0-0N778-5W399-WM4ZG-RC58Y-CKG58-06014-7T82W-00G0P-X4WGJ-PA250-BX08N-88QTC-8NG`
