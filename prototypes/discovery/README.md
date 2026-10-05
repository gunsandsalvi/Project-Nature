# P4 Discovery pace

The fourth prototype (IMPLEMENTATION α0.3a, research 00 and 11) asks the riskiest design question (`RSK-01`): can
tuning alone make sharp flakes and fire come within their windows, with the world's own rules? The windows are
`TIM-19`'s, halved on 4 October 2026 for faster discoveries and again on 5 October, so the first village comes about
an hour into play, as you asked: flakes within 2 years, which is also the sharp-stone test's bar (`RES-03`), and fire
in Years 2 to 8. They are dates (`TIM-14`), so Year 2 begins a year in.

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
- **The values that set the pace** and that the sweep tries are in `tuning.toml`, with what each was tuned against
  (`RES-16`); a few more are constants in `discovery.py`, outside the sweep: the 0.9 chance a flake hunch finds stone
  that flakes, play bashing stones 0.3 of the time, teaching at 0.3 times kindness, copying at 0.01, the good times'
  0.8 and 0.5, and tries aimed at the cold made twice as often. The factors' third figure is noise: the fire median
  flips between 3.5 and 4.5 years for a change of under 0.1%.
- **The runs** (`pace.py`): the sharp-stone test on the 20 tuning seeds and its control without stone that flakes,
  fire's window, counted at its first in a world of 3 or 4 bands as `TIM-19` counts a step, the same on 20 seeds
  never tuned against, and each tuned value changed alone over 40 runs and 40 worlds. A value holds the pace on a
  knife's edge if changing it by a quarter either way breaks a step's rule; it is a strong lever if only halving or
  doubling it does. It writes `prototypes/app/reports/p4.json`, which the app's Reports page draws, and keeps the
  runs it has done in `.runs/` (not committed), so a run cut short resumes where it stopped.

    python3 prototypes/discovery/pace.py           # the runs and the report: about 20 minutes fresh on three cores,
                                                   # a second from the cache
    python3 prototypes/discovery/pace.py --tune    # tunes the discovery factors first, and writes them back

Its tests (`tests/`) run with the project's checks.
