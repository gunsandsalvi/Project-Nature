# Research 12: testing

**Question:** how should a Godot game with a C++ simulation be tested by AI builders in the cloud, so each step is proven before it reaches your phone (`RES-01`, `PRC-10`)?

## How others do it

- **Godot's test frameworks:**
  - GUT, which is GDScript-first;
  - gdUnit4, for GDScript and C#, with scene testing.

  Both run headless from the command line and report pass or fail with file and line for CI, for example `godot --headless --path . -s res://addons/gdUnit4/bin/GdUnitCmdTool.gd --run-tests` ([gdUnit4](https://github.com/MikeSchulze/gdUnit4), [summaries](https://skills.sh/randroids-dojo/skills/godot)).
- **Golden images:** the first run saves a reference picture; later runs must match it, and a deliberate change is approved by replacing the reference.
- **Simulation games** test with seeded scenarios and replays.
  The same seed and inputs must give the same history (research 02), so a failure can be replayed tick by tick.

## What we take

1. **C++ unit tests (doctest) for the simulation**, built natively and run in the cloud in seconds: maths, keyed chance, time, storage, rules.
2. **Scenes**, small seeded settings run by the simulation alone, headless, with pass rules stated before they run (`RES-09`, `RES-21`).
3. **gdUnit4 for the Godot side:** gestures, cards and views, driven headless.
4. **Golden pictures** from Godot on the software Vulkan driver at the phone's art size, already working in the bake-off.
   They are compared within a small tolerance, since the software driver and the phone's chip may round differently; you approve every new reference by eye.
5. **The same-history check:** a seeded world is run on x86 and on ARM (under qemu) and the state hashes are compared, so the phone and the cloud agree (`RES-05`).
6. **One command, `tools/check.sh`,** runs all of it before anything joins the main version (`PRC-10`).
