# P4 Discovery pace

The fourth prototype (IMPLEMENTATION α0.3a, research 00 and 11) asks the riskiest design question (`RSK-01`): can
tuning alone make sharp flakes come within 5 years and fire within its window, with the world's own rules?

Items it is about: `TIM-19` (the pace of discovery), `RES-02` and `RES-03` (the sharp-stone test and its pass rule),
`RES-16` (tuning the pace), `RES-06` (its report, on the app's Reports page), `MND-06` (skill), `MND-11` (the four
routes to discovery), `MND-13` (learning and teaching), `RCK-01` and `RCK-02` (flint flakes, granite doesn't; fire by
friction with dry wood).

- **The band** (`discovery.py`): about 25 people by a river, with the starting kit alone (`BIO-02`, `BIO-20`). Each
  day of the 60-day year they gather, crack nuts and bones with stones on anvils, butcher what they scavenge, rest,
  keep a fire when they have one, and experiment by their curiosity in good times, or aimed at the cold when they have
  no fire and nothing to answer it. Children play, bashing stones as their elders do. People are born and die, and a
  craft dies with its last holder (`CUL-02`).
- **The world:** a share of the stones in reach flake (flint among granite and sandstone), wood is dry or green by
  season, and wildfires from lightning sometimes bring fire to take, which rain and neglect put out.
- **Discovery** follows `MND-11`: an activity that fits an unknown blueprint gets the chance a maker at the doer's
  level would have (`MAT-04`), times 1 in 20 by accident, 1 in 5 by experimenting or 1 in 2 with a hunch, times the
  blueprint's discovery factor, and teaches it if noticed (`MND-10`); a failed try may show its hint, a sharp chip or
  smoke, which gives a hunch. Dreams join memories into hunches, a third of them for a real blueprint whose action the
  dreamer knows (`MND-12`). Watching about five uses teaches a blueprint, and the kind teach kin first (`MND-13`).
- **Every value that sets the pace** is in `tuning.toml`, with what it was tuned against (`RES-16`).
- **The runs** (`pace.py`): the sharp-stone test on the 20 tuning seeds and its control without stone that flakes,
  fire's window, the same on 20 seeds never tuned against, and each tuned value halved and doubled over 40 runs. It
  writes `prototypes/app/reports/p4.json`, which the app's Reports page draws.

    python3 prototypes/discovery/pace.py           # the runs and the report, a few minutes
    python3 prototypes/discovery/pace.py --tune    # tunes the discovery factors first, and writes them back

Its tests (`tests/`) run with the project's checks.
