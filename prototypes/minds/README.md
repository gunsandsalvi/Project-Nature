# P6 A thousand minds

The sixth prototype (IMPLEMENTATION α0.4b, research 03 and 10) asks: do a thousand simple minds with needs, choice,
talk and paths keep a game year a real minute on your phone at held speed?

Items it is about: `TIM-07` (the speed target: at least 1 game year a real minute with 1,000 people, aiming for 2 to
3), `MND-15` (a whole person costs about a thousandth of a second a game day), `MND-09` (choosing by what each option
does for the needs, with the top reasons kept), `MND-14` (every mind full, within its caps), `TIM-17` (activities
that end at events) and `RES-05` (the same results on one thread and four).

- **The land** (`src/land.cpp`): 4 km a side in cells of 4 m, made from the seed: open and rough ground, rock and
  thicket no one crosses (a ninth of it), a river crossed only at its fords, and 1,972 spots of water, berries, nuts,
  roots, game, firewood and flint that use takes and the seasons bring back; 40 camps near water.
- **The people** (`src/minds.cpp`): 40 bands of 25, three in five adults. Each has nine needs that run down at their
  own rates, faster with effort, in the cold and at night (`BIO-09`, `MND-07`); five traits; what they carry; up to
  256 places they know, each with who told them of it; up to 150 people they know, with an opinion of each; and up to
  200 memories (`MND-14`).
- **Activities and events** (`TIM-17`): everything they do is an activity with an end, and they choose again only
  then. Its end comes sooner if a need it doesn't meet will fall below 20 first. Each five minutes of game time, the
  activities ending in it land in order of their ends (food eaten, berries gathered, a fire fed, a talk heard), then
  their people choose again in parallel, in fixed pieces of 16, reading the world as it stood; so one thread and four
  end every day the same, which the tests check.
- **Choice** (`MND-09`, `src/actions.hpp`): 50 actions as data, each with what it is done to, when it can be done,
  how long it takes, what it does for each need, its effort and risk, and the trait that draws a person to it. Every
  action they could do now is scored by response curves: what it does for each need, weighted by how pressing it is
  and by their nature, with the hour, effort, distance and risk. They usually take the best, sometimes one close
  behind, by keyed chance; the three parts that most put it ahead, and the two options it beat, are kept as its
  reasons (`PRN-13`).
- **Talk** (`MND-33`): someone within 20 m to chat, tell news, groom, court or teach passes on up to three topics,
  a place they know, their opinion of someone, and their latest news; the listener learns the place with who told
  it, moves their own opinion by how much they trust the speaker, and remembers the news.
- **Paths in levels** (`src/paths.cpp`, A11): connected regions reject an impossible trip at once; the land's
  clusters of 32 × 32 cells, split into the parts joined inside them, meet at entrances, and each entrance's field of
  distances over its part is made at the start; a trip between two parts follows the path between them over the
  entrances, found by A* once and cached for every thread, plus the two fields' distances at its ends.
- **The measure** (`src/cli.cpp`, and the app's "P6 A thousand minds" screen through `extension/`): game years a real
  minute, decisions a game day, and each part's share of the time: results landing, choice, paths, talk, and the rest.

It uses P5's own maths, keyed chance and hashes (`prototypes/samebits/src`), written once.

    cmake -S prototypes/minds -B build/minds -G Ninja && cmake --build build/minds
    build/minds/minds_cli 4 30       # four threads, 30 game days: the speed, each part's share, the checksum

Like every prototype it is thrown away once its answer is written into the architecture (A3.3, A3.9, A11).
