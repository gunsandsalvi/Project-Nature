# P10 Culture from causes

The tenth prototype (IMPLEMENTATION α0.6b, research 12) asks: do customs, a spirit, a rite and a band split arise
inside `CUL-33`'s windows, each from its own cause?

Items it is about: `CUL-33` (the pace of culture), `CUL-05` (beliefs from events), `CUL-06` (customs), `CUL-30`
(bands), `CUL-34` (rites), with `MND-05` (beliefs about causes), `CUL-19` (ancestors) and `CUL-24` (talk) as its
means.

- **Three bands** (`culture.py`) of 22 to 30 people in families by a river valley, run headless for 100 years in the
  60-day year (`TIM-18`), with P4's ages, births and deaths (`prototypes/discovery`), written once. They hunt, eat a
  rare food in season, meet storms and lightning, fall ill, are born and die, marry at the summer gathering, and
  talk each evening.
- **Beliefs about causes** (`MND-05`), with the project file's own numbers: after a strong outcome, each who meets it
  links it to the most unusual thing of the day or two before; a blow from the sky or a sudden death leaves, about
  one time in three, a belief in an unseen being; a link tested by what follows grows by 15 or weakens by 5, or 10
  for an outcome on more than one day in ten, and is forgotten below 5. A good hunt befalls its hunters, who link it
  to what they did before it; what they do changes nothing.
- **Talk** (`CUL-24`): each evening some adults tell a belief, and the listener takes it up to what the teller's
  conviction lends, by trust, and no further however often told.
- **The dead** (`CUL-19`): those who loved them dream of them for two years, each dream strengthening the belief that
  they live on.
- **What the band shares:** a belief most of its adults hold is a spirit it shares (`CUL-05`); an act most of them
  credit for good hunts a year long, or its way with the dead kept a year, is a rite it keeps (`CUL-34`); the way two
  thirds of its cases went, once it has had 3, is its custom: how the dead are treated, who shares a big kill, where
  couples live (`CUL-06`); a band past 40, or past 20 after a fight between heads of families, splits, the families
  thinking least of the leader leaving (`CUL-30`).
- **Every first kept with its events,** so each traces back to its own cause and a run's story can be told.
- **The values that set the pace** are in `tuning.toml`, each with where it comes from; those `CUL-33` names as its
  levers, and those the project file states, are as it states them.
- **The runs** (`runs.py`): 20 seeds, 100 years each; when each first came, against the windows, and run 1's story.
  It writes `prototypes/app/reports/p10.json`, which the app's Reports page draws.

        python3 prototypes/culture/runs.py    # about a minute and a half on four cores

Its tests (`tests/`) run with the project's checks. Like every prototype it is thrown away once its answer is
written into the architecture (A13).
