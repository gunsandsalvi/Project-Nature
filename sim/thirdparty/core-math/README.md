# CORE-MATH, vendored

The correctly rounded maths functions the simulation uses (`RES-05`, A3.4), from [CORE-MATH](https://core-math.gitlabpages.inria.fr/) at commit `e072473ef919f1884b8cd0ac96817f068df793f7` (5 October 2026), under its MIT licence (`LICENSE`).

- `src/binary64/<function>/`: each function's C file and the headers it includes, unchanged, in upstream's folders.
- `hard-cases.inc`: 256 inputs a function, spread evenly through the ones its `.wc` file lists as the hardest to round, which the oracle test (`sim/tests/oracle.cpp`) and the numbers' proof suite run.

They are kept here because their host is the one source a cloud session might not reach (A2.4).
`tools/core-math.py` copies them from a checkout of the pinned commit and writes the hard cases; to update, change its commit, run it, and let the oracle test check every function against MPFR again.
`sim/src/kd/num/maths.cpp` wraps each function once, and nothing else calls them.
