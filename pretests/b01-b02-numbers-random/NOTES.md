# B01 + B02 pre-test: numbers and random draws

Throwaway test. Deleted once the architecture is written.

## Question

- `B01`: which number format (f32, f64, fx32 = Q16.16, fx64 = Q32.32) and which language (Rust, C++, Java) give the most simulation per second for typical work (heat flow, random walks, big sums, learning updates)? Do results repeat bit for bit across runs and thread counts (`X11`)? Which results also match bit for bit between the cloud (x86) and an ARM build (`X1`, `RES-05`)?
- `B02`: which keyed generator should give every chance its own draw from (world, system, being, moment, purpose) (`TIM-06`, `X6`, `X7`, `GOD-04`)?

## Approaches

- Five kernels (heat, walk, sum, learn, rng), each in Rust and C++ in one library (`kbench`), in four formats where they apply; Java versions of heat (f32, fx32) and the splitmix draw (`jvm/Kernels.java`).
- Generators: splitmix64, Philox4x32-10, Squares (Widynski), PCG-style hash, wyhash-style hash, ChaCha8. Added after the first results: "wysafe", the wyhash-style hash with wyhash's guarded multiply (see R1).

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
- **Cloud build.** x86-64-v2 (128-bit vectors, no FMA: the closest x86 match to the phone's 128-bit NEON), clang 18 for C++ (the NDK's compiler family), Rust 1.97, OpenJDK 21 for Java. Speeds: 6 interleaved rounds of 1 s runs (two batches of 3) under the shared CPU lock; median and spread (max - min) / median.
- **Determinism (`X11`).** Checksums from the 6 timed runs on 1 and 4 threads, plus short runs on 2 and 3 threads, for Rust, C++, C++ without FMA and Java.
- **ARM.** An aarch64 Linux build (clang for C++, like the NDK) runs under qemu-aarch64-static; its checksums are compared with x86. The phone library itself builds with `cargo ndk -t arm64-v8a --platform 29 build --release --features android`.
- **Random quality (`B02`).** PractRand pre0.95, `RNG_test stdin64 -tlmax 34 -multithreaded` (2^34 bytes = 16 GiB per stream): (i) one key, moments 0, 1, 2...; (ii) beings 0, 1, 2... at one moment. Extra: (iii) each chance event's first draw interleaved with its fortune retry (`GOD-04`), to 2^32 bytes; wy's two streams also to 2^35.
- **How each generator takes the key.** World, system and purpose are folded once per loop into a stream key k0 (purpose's top bit marks a fortune retry, so a retry is simply another key). Per draw:
  - splitmix: mix(mix(k0 xor being) + (moment + 1) x golden ratio constant);
  - Philox4x32-10: counter = (moment, being), all 128 bits; key = k0;
  - Squares64: key made from k0 by Widynski's digit rules; counter = mix(k1 xor being) + moment;
  - PCG-style: pcg(pcg(k0 xor being) + moment), pcg = one LCG step + the RXS-M-XS output;
  - wyhash-style: wyhash of the 16 bytes (being, moment) with seed k0;
  - ChaCha8: key = (world, system, purpose) as is; counter = moment; nonce = being; first 64 bits of the block.

## Results

Cloud machine only (x86, 4 shared cores). Provisional until the phone runs the same library. Full data: `results/speed.csv`, `results/determinism.csv`, `results/practrand/`.

**Speed.** Million operations per second, median of 6 runs of 1 s; spread (max - min) / median in brackets. Columns: language and thread count.

| Kernel | Rust 1 | C++ 1 | Rust 4 | C++ 4 |
|---|---|---|---|---|
| heat f32 | 2,508 (38%) | 2,515 (37%) | 6,746 (35%) | 7,644 (35%) |
| heat f64 | 1,218 (30%) | 1,204 (17%) | 3,743 (43%) | 4,012 (48%) |
| heat fx32 | 1,768 (25%) | 872 (43%) | 5,016 (53%) | 2,759 (27%) |
| heat fx64 | 804 (27%) | 574 (20%) | 2,192 (39%) | 1,534 (39%) |
| walk f32 | 91 (48%) | 96 (26%) | 268 (30%) | 291 (35%) |
| walk f64 | 79 (31%) | 86 (7%) | 231 (24%) | 254 (30%) |
| walk fx32 | 87 (56%) | 91 (32%) | 249 (10%) | 270 (27%) |
| walk fx64 | 69 (26%) | 82 (10%) | 209 (22%) | 238 (11%) |
| sum f32 | 3,990 (33%) | 4,222 (32%) | 12,938 (34%) | 13,006 (17%) |
| sum f64 | 1,662 (30%) | 1,863 (23%) | 6,961 (29%) | 7,189 (30%) |
| sum fx32 | 3,194 (14%) | 3,284 (20%) | 10,344 (22%) | 9,634 (29%) |
| sum fx64 | 1,777 (45%) | 1,888 (20%) | 6,755 (42%) | 6,713 (31%) |
| learn f32 | 32 (36%) | 30 (28%) | 80 (40%) | 77 (11%) |
| learn f64 | 23 (29%) | 25 (16%) | 67 (43%) | 67 (39%) |
| learn fx32 | 16 (22%) | 16 (25%) | 45 (29%) | 46 (18%) |

Java on the desktop JVM: heat f32 746 (1 thread) and 1,885 (4 threads); heat fx32 571 and 1,579; splitmix draws 447 and 1,111.

**Keyed draws (`B02`).** Million draws per second (rng kernel) and agent-steps per second (walk f32), 1 thread, best of Rust and C++; PractRand per stream (one key's moments; neighbouring beings; first draw interleaved with its fortune retry).

| Generator | Draws | Walk | One key | Neighbours | Retry |
|---|---|---|---|---|---|
| splitmix | 319 | 96 | pass 2^34 | pass 2^34 | pass 2^32 |
| philox | 191 | 40 | pass 2^34 | pass 2^34 | pass 2^32 |
| squares | 319 | 77 | pass 2^34 | pass 2^34 | pass 2^32 |
| pcg | 254 | 91 | pass 2^34 | pass 2^34 | pass 2^32 |
| wy | 1,152 | 136 | pass 2^35 | pass 2^35 | pass 2^32 |
| chacha8 | 24 | 12 | pass 2^34 | pass 2^34 | pass 2^32 |
| wysafe (extra) | 451 | 110 | pass 2^34 | pass 2^34 | pass 2^32 |

No FAIL and nothing "suspicious" anywhere. Five of the 23 streams had one "unusual" flag (the mildest level), each in a different test and never repeated: noise under R1. On 4 threads the draws scale about 3 times (wy 2,860, splitmix 986).

**Determinism (`X11`).**
- All 87 combinations of kernel, format, generator and language (Java included) gave one checksum across all runs, and 85 of them were checked on 1, 2, 3 and 4 threads (the other 2 on 2 and 3). Every rep matched its run's warm-up rep.
- All languages agree on x86: Rust, C++, C++ without FMA and Java give identical checksums.
- ARM (aarch64 build under qemu) against x86: Rust 26 of 26 identical, floats included; C++ without FMA 26 of 26; C++ with default settings 22 of 26. The 4 that differ are heat and learn in f32 and f64: clang fuses a*b+c into one instruction on ARM (20 such instructions in the phone library's C++ heat, 112 in learn; none in Rust). All integer and fixed-point results match.

**Rounding.**
- Sum of the 2^24 test values: exact 3,057,372.846. f32 fixed tree 3,057,371.0; f32 plain loop 3,057,589.5. f64: the tree is 6 times closer than the plain loop; both differ in the last bits. Integer formats give the same bits either way, but fx32 rounds tiny values down (3,057,284.9).
- The fixed tree is faster than the plain loop, not slower: f32 4.0 to 4.2 billion values per second against 1.4 on 1 thread, because it vectorizes.
- Heat should keep its total. Change per step: f32 -1e-11, f64 0, fx64 -2e-12, fx32 -1.5e-7, always downward (about -14% over a million steps), because a plain shift rounds down.

## Verdict

Provisional, from cloud numbers; the phone settles speed.

- **R1, generator: the rule picks wy (wyhash-style), but use the guarded version, wysafe.** All six candidates pass both streams to 2^34 bytes, so the rule picks the fastest: wy, 1,152 million draws per second on one thread, 3.6 times splitmix and Squares (319), and also the fastest inside the walk (136 against 96). The rule proved badly framed in one way: it only sees statistics and speed. Plain wy multiplies (being xor a constant) by (moment xor the stream's seed), so at the one moment equal to the seed every being gets the same draw. With moments up to 2^41 that hits about one stream in 8 million: rare, but across many worlds it will happen. wysafe (wyhash's own guarded multiply) removes it, passes the same tests, keeps 81% of wy's speed inside the walk (110 against 136) and is still faster there than splitmix (96) and the others.
- **Fortune retries (`GOD-04`).** A retry is the same key with the purpose's top bit flipped: its own independent draw that changes nothing else. PractRand finds no link between first draws and their retries for any generator (to 2^32 bytes).
- **R2, format: f32 for all four kinds of work.** f32 is the fastest format in every kernel on 1 and 4 threads (heat 2,515 against fx32's 1,768; sum 4,222 against fx32's 3,284; learn 32 against f64's 25) and passes every check. In walk, f64 and fx32 also come within 15% (96, 86, 91); the rule's order picks f32.
- **R3, language: Rust.** Rust/C++ speed ratio over 52 pairs: geometric mean 1.00 (single loops range 0.84 to 2.03, from per-loop compiler choices, not the language). A tie, so points: Rust 3 (built-in tests; one cargo command each for phone and cloud; floats identical on x86 and ARM), C++ 2 (one CMake command each; but it needs a test framework, and its floats differ on ARM unless fused multiply-add is switched off).
- **R4, determinism: pass.** Nothing depends on thread count or timing. Bonus: with Rust (or C++ with FMA off) the cloud reproduces ARM results bit for bit, floats included, as long as kernels use only + - x / (library maths such as exp and sin can differ between platforms).
- **R5, managed code: out for simulation kernels.** Java heat runs at 25 to 32% of native speed even on the desktop JVM (the bar is 85%). Fine for the app shell. Java's keyed draw beat native here (1.4x) only because LLVM vectorizes 64-bit multiplies badly at this x86 level.

## Caveats

- Cloud, not phone: a shared x86 VM. Single 1 s runs vary by 10 to 30% (sometimes more); medians of 6 runs are used. Each pick's margin is larger than that, except walk, where three formats tie.
- This x86 level lacks NEON's native 32 x 32 to 64-bit vector multiply, so fx32 looks worse here (C++ heat fx32 most of all) than it may on the phone.
- The rng kernel here vectorizes 64-bit multiplies with slow emulation (Java's scalar splitmix is 1.4x faster), which understates splitmix, PCG and Squares. The walk numbers are the fairer in-context view.
- PractRand reached 2^34 bytes (2^31 draws) per stream, 2^35 for wy. A world draws far more; 2^40 would take about 2 hours per stream here.
- splitmix and Squares give each being a random 64-bit starting counter, so two beings' sequences can overlap with a lag (about 2^-24 per pair of beings over 2^40 moments). Philox and ChaCha8 take (being, moment) whole, so a stream never repeats; wy and PCG hash both, so only chance collisions.
- ChaCha8 here spends a whole 64-byte block on one draw; 8 draws per block would cut its cost about 8 times. Still the slowest.
- Fixed point with plain shifts drains conserved totals; a fixed-point field would need flux-form updates or rounding to nearest.
- qemu checks ARM arithmetic, not speed. It used clang 18; the NDK's newer clang should fuse the same expressions, but the phone run will confirm. Java ran on HotSpot, not Android's ART.

## What only the phone can answer

- Speed on each core type (pin workers with `"cpus"`), scaling to 7 cores, and energy per operation (`B01` asks per watt).
- Whether fx32 catches up with f32 on NEON (R2), and whether C++'s fused multiply-add gives it a float edge there; Rust can opt in with `mul_add` and stay deterministic (R3).
- The generator ranking on ARM's 64-bit multipliers (R1): wy's lead may shrink.
- ART's speed for the Java kernels (R5).
- Whether the phone's checksums equal the ARM column in `results/determinism.csv`. If so, the cloud can replay phone runs exactly (Rust, or C++ with FMA off).
- The graphics chip (not covered here).

## How to re-run

```
cd pretests/b01-b02-numbers-random
. scripts/env.sh                                   # cache paths, lock, compilers
flock $LOCK bash -c 'cd kbench && cargo build --release && cargo test --release'
flock $LOCK scripts/build-practrand.sh             # PractRand into the shared cache
flock $LOCK scripts/build-arm.sh                   # phone .so (cargo-ndk) + aarch64 tool for qemu
scripts/run-all.sh                                 # slots A-F, each under the lock (needs ~50 min of lock time)
python3 scripts/bench.py summarize <raw dir>       # rewrites results/*.csv
```

One run: `$KBENCH '{"kernel":"rust:heat","format":"f32","threads":4,"seconds":2}'`. Java: `java jvm/Kernels.java '{"kernel":"java:heat","format":"fx32"}'`. On the phone: `Bench.run(json)`, where `{"list":true}` gives the plan and `"cpus":[7]` pins the workers.
