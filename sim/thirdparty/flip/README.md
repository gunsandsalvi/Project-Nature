# FLIP, vendored

NVIDIA's FLIP, which says how different two pictures look to someone at a given distance (A4.8), from [its repository](https://github.com/NVlabs/flip) at commit `b475eb4bf394ab877c42166c9eb0a84a02cc5b14` (7 November 2025), under its BSD 3-clause licence (`LICENSE`).

- `FLIP.h`: upstream's `src/cpp/FLIP.h`, unchanged.
- `kd_flip.h` and `kd_flip.cpp`: the one call into it, which `sim/src/kd/look/frame.cpp` makes: two 8-bit sRGB pictures in, made linear as FLIP's own tool makes them, and the mean error and the share of pixels above 0.2 out.

FLIP works in `float` with the platform's maths, which the simulation's own code never uses (A3.4), so it is kept here, and its results stay out of the digests the cloud and the phone must share.
To update, copy `FLIP.h` from a newer commit, change the commit above, and run the look's tests, whose FLIP pair was measured with FLIP's own tool.
