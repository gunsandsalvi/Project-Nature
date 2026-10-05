# P11 The director

The eleventh prototype (IMPLEMENTATION α0.6c, research 13) asks: does the director keep its budget on recorded worlds
while catching every named discovery, without changing them?

Items it is about: `TIM-02` (the story director), `TIM-03` (the director never touches events), `PRE-39` (recognising
what emerges), with `PRE-08` (live moments), `PRE-06` (following someone) and `MAT-21` (named discoveries) as what it
shows.

- **The test worlds** (`watch.py`): each is a valley of P10's three bands (`prototypes/culture`) with P4's discovery
  (`prototypes/discovery`) in three bands of their own beside them, each its own people, out of reach as P4 ran them,
  run 100 years. Both models now keep a log of what happens, each event with its day and hour, kind, who and what,
  written as it happens and drawing on no chance, so their results are what they were.
- **The recognisers** (`director.py`): story-sifting patterns over the log, as Felt and Winnow sift: firsts worldwide
  and for each people, named discoveries, rediscoveries, crafts lost with their last holder, a band losing its last
  fire, the births and deaths of three people followed from the start, and feuds. Patterns half matched are signs: a
  hunch tried again, a storm over a camp, one you follow hurt or ill. Each recogniser keeps only what it has seen, so
  nothing looks ahead.
- **The director:** a moment or sign past the bar slows time, if the one budget allows, so that what it shows takes
  about half a minute: at most one slowdown every 3 real minutes, never more than a fifth of the time slowed, and 10
  seconds untapped; after each, the bar stands higher for a while, so a higher score slows time sooner, never slower;
  the rest wait in the list. A sign's score is what its end would be worth times how often it came.
- **The watch:** each world from the globe at 5 game years a real minute, `TIM-07`'s least for 100 to 300 people,
  never tapping, as `TIM-02`'s done-when watches; again tapping every live moment; and again with signs scored as
  their end.
- **Hands off** (`TIM-03`): each world runs live with the director on and again straight with it off, and the two
  must end identical, their logs and everything they hold; a host that lets the director reach a world's chance is
  caught; and a code check finds that the director imports nothing of the worlds, and they nothing of it.
- **What it checks against:** the worlds' own records, each people's firsts, crafts lost and found again (P4's), the
  valley's first custom, spirit, rite and split (P10's), and the deaths of those followed.
- **Ages** (`PRE-39`): from the steps of the arc first reached anywhere, with `PRE-39`'s least of 20 years and
  shorter.
- **Its values** are in `tuning.toml`: the budget as `TIM-02` gives it, and the scores, the bar and the speed it leaves
  to be tuned, as stand-ins.

It writes `prototypes/app/reports/p11.json`, which the app's Reports page draws.

        python3 prototypes/director/watch.py    # about seven minutes on three cores

Its tests (`tests/`) run with the project's checks. Like every prototype it is thrown away once its answer is
written into the architecture (A14).
