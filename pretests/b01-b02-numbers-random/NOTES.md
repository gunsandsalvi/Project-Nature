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

- **Library.** `kbench/` is a Rust crate; build.rs compiles the C++ kernels into the same library twice: `cpp` (compiler defaults, as an NDK build) and `cppnc` (fused multiply-add off, for checksum comparison only). `run(json) -> json`, a command-line tool, and JNI exports for `dev.kindling.pretests.Bench` (feature `android`). `{"list":true}` lists kernels and a ready-made phone run plan; `{"selftest":true}` checks known answers and that Rust and C++ draws agree.
- **Kernels.** heat: 512 x 512 torus, 5-point stencil, alpha 0.2, double buffer, 64 steps per rep. walk: 100,000 agents, 1024 x 1024 torus, direction from the top 2 bits of draw(agent, step), 16 steps per rep. sum: 2^24 values (magnitudes 2^-16 to 2^15), fixed tree (blocks of 4,096, then the 4,096 block results). learn: 4,096 learners x 64 weights, delta rule, learning rate 1/64, dot product in 8 fixed lanes, 32 steps per rep. rng: beings 0 to 65,535 at moments 0 to 15.
- **Timing.** One untimed warm-up rep, then reps for at least 1 s (at least 3). Every rep restarts from the same state, so its checksum must equal the warm-up's. Threads get a fixed contiguous share and meet at a spinning barrier between steps.
- **Cloud build.** x86-64-v2 (128-bit vectors, no FMA: the closest x86 match to the phone's 128-bit NEON), clang 18 for C++ (the NDK's compiler family), Rust 1.97, OpenJDK 21 for Java. Speeds: 3 interleaved rounds of 1 s runs under the shared CPU lock; median and spread (max - min) / median.
- **Determinism (`X11`).** Checksums from the 3 timed runs on 1 and 4 threads, plus short runs on 2 and 3 threads, for Rust, C++, C++ without FMA and Java.
- **ARM.** An aarch64 Linux build (clang for C++, like the NDK) runs under qemu-aarch64-static; its checksums are compared with x86. The phone library itself builds with `cargo ndk -t arm64-v8a --platform 29 build --release --features android`.
- **Random quality (`B02`).** PractRand pre0.95, `RNG_test stdin64 -tlmax 34 -multithreaded` (2^34 bytes = 16 GiB per stream): (i) one key, moments 0, 1, 2...; (ii) beings 0, 1, 2... at one moment. Extra: (iii) each chance event's first draw interleaved with its fortune retry (`GOD-04`), to 2^32 bytes.
- **How each generator takes the key.** World, system and purpose are folded once per loop into a stream key k0 (purpose's top bit marks a fortune retry, so a retry is simply another key). Per draw:
  - splitmix: mix(mix(k0 xor being) + (moment + 1) x golden ratio constant);
  - Philox4x32-10: counter = (moment, being), all 128 bits; key = k0;
  - Squares64: key made from k0 by Widynski's digit rules; counter = mix(k1 xor being) + moment;
  - PCG-style: pcg(pcg(k0 xor being) + moment), pcg = one LCG step + the RXS-M-XS output;
  - wyhash-style: wyhash of the 16 bytes (being, moment) with seed k0;
  - ChaCha8: key = (world, system, purpose) as is; counter = moment; nonce = being; first 64 bits of the block.

## Results

(Filled in after the runs.)

## Verdict

(Filled in after the runs.)

## Caveats

(Filled in after the runs.)

## What only the phone can answer

(Filled in after the runs.)

## How to re-run

```
cd pretests/b01-b02-numbers-random
. scripts/env.sh                                   # cache paths, lock, compilers
flock $LOCK bash -c 'cd kbench && cargo build --release && cargo test --release'
flock $LOCK scripts/build-practrand.sh             # PractRand into the shared cache
flock $LOCK scripts/build-arm.sh                   # phone .so (cargo-ndk) + aarch64 tool for qemu
scripts/run-all.sh                                 # all runs, each slot under the lock
python3 scripts/bench.py summarize <raw dir>       # rewrites results/*.csv
```

One run: `$KBENCH '{"kernel":"rust:heat","format":"f32","threads":4,"seconds":2}'`. Java: `java jvm/Kernels.java '{"kernel":"java:heat","format":"fx32"}'`. On the phone: `Bench.run(json)`, where `{"list":true}` gives the plan and `"cpus":[7]` pins the workers.
