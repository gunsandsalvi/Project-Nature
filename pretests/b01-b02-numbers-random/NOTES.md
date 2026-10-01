# B01 + B02 pre-test: numbers and random draws

Throwaway test. Deleted once the architecture is written.

## Question

- `B01`: which number format (f32, f64, fx32 = Q16.16, fx64 = Q32.32) and which language (Rust, C++, Java) give the most simulation per second for typical work (heat flow, random walks, big sums, learning updates)? Do results repeat bit for bit across runs and thread counts (`X11`)? Which results also match bit for bit between the cloud (x86) and an ARM build (`X1`, `RES-05`)?
- `B02`: which keyed generator should give every chance its own draw from (world, system, being, moment, purpose) (`TIM-06`, `X6`, `X7`, `GOD-04`)?

## Approaches

- Kernels, each in Rust and C++ in one library (`kbench`): heat (2D diffusion on a 512 x 512 torus), walk (100,000 agents on a 1024 x 1024 torus, keyed draws), sum (2^24 values, fixed tree), learn (4,096 delta-rule learners with 64 weights), rng (one keyed draw).
- Java versions of heat (f32, fx32) and of the splitmix draw (`jvm/Kernels.java`).
- Generators: splitmix64, Philox4x32-10, Squares (Widynski), PCG-style hash, wyhash-style hash, ChaCha8.

## Decision rules (written before any measurement; not changed afterwards)

Determinism checks (`X11`), used by R2 and R4. A kernel/format/language passes if its checksum is identical (a) between the first and last repetition inside a run, (b) across at least 3 separate runs, and (c) across 1, 2, 3 and 4 threads.

- **R1, generator (`B02`).** A generator qualifies if both PractRand streams (one key's moments 0, 1, 2...; neighbouring beings 0, 1, 2... at one moment) reach the tested length (at least 2^34 bytes) with no FAIL, and with no result rated "very suspicious" or worse at the final length. Milder flags ("unusual", "suspicious") are noise unless the same test is flagged at the last two lengths; then the generator is disqualified too. The pick is the qualified generator with the most draws per second on one thread (best of Rust and C++). If two are within 10% of each other, pick the one whose input takes being and moment without squeezing them into 64 bits (Philox, ChaCha8), then the shorter code. Provisional on cloud numbers; final on the phone's fast core.
- **R2, number format (`B01`).** For each kernel (heat = fields, walk = agents, sum = totals, learn = learning), a format qualifies if it passes the determinism checks and its best speed (best language) is within 15% of the fastest format, both on 1 thread and on 4 threads. Among qualifying formats the pick is the first in this order: f32 (native on the graphics chip, wide range, simplest code), fx32, f64, fx64. Provisional on cloud numbers; final on the phone's fast cores.
- **R3, Rust or C++ (`X10`).** Take the speed ratio Rust/C++ for every kernel, format and thread count, and their geometric mean. If the mean is more than 10% from 1, the faster language wins. Otherwise score three points, one each: (1) unit tests without an extra framework; (2) one command builds the same code for the phone and the cloud; (3) float checksums equal on x86 and ARM with default compiler settings (measured here). The higher score wins; a tie goes to Rust (memory-safe threads).
- **R4, determinism (`X11`).** Any failed determinism check is a finding that blocks that kernel/format/language combination and is raised with the lead. Cross-machine matches (x86 against ARM under qemu) are reported, not required (`X1`); R3 uses them only as point (3).
- **R5, managed code.** Java counts as fast enough for simulation kernels only if java:heat is within 15% of the best native heat for the same format and thread count. The desktop JVM is usually faster than Android's runtime, so the cloud number can only rule Java out; ruling it in needs the phone.

## Method

(Filled in after the runs.)

## Results

(Filled in after the runs.)

## Verdict

(Filled in after the runs.)

## Caveats

(Filled in after the runs.)

## What only the phone can answer

(Filled in after the runs.)

## How to re-run

(Filled in after the runs.)
