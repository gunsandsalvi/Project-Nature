# B80 pre-test: cloud runner and moving worlds (feasibility test T6)

Throwaway test. Serves `PLT-05`, `SCP-15`, `RES-01`, `RES-05`, `PRN-15`; checks crossroads `X11`.

## Question

How much computing do these cloud sessions really give the research experiments? Can a runner survive being killed and still give exactly the same results?

## Approaches compared

- Part A: do the 4 cores hold their speed for 20 minutes, or do they slow down or get "stolen" by other machines?
- Part B: what keeps running: a detached process started with `setsid nohup` (a heartbeat that writes one line a minute).
- Part C: save a checkpoint every N simulated days (write a temporary file, then rename it), kill the run twice with `SIGKILL`, resume, and compare the final checksum.
- Part D: three ways to run worlds in parallel: 4 worlds as 4 processes; 4 worlds as 4 threads in 1 process; 1 world on 4 threads.
- Part E: arithmetic for Experiment 1 (3 setups x 100 worlds x 500 years, about 100 people each).

## Decision rule (written before measuring, not changed afterwards)

**Main rule.** If both hold:
1. the 4 cores keep at least 95% of their first-minute throughput in every one of the 20 minutes, and
2. every resumed run ends bit-identical to the uninterrupted run, and a partly written or damaged checkpoint is never loaded,

then the runner design is: **one process per world, as many processes as cores, a checkpoint every simulated month** (write a temporary file, flush it, rename it), resuming from the newest checkpoint that passes its check.

**If 1 fails:** plan with the slowest minute's rate, not the first minute's. If steal averages over 5%, run one process fewer than there are cores. Say whether the drop is a steady slide (throttling) or one-off dips (neighbours).

**If 2 fails:** the design is rejected. Something outside the saved state changes results, which breaks `X11`. It must be fixed before any experiment runs, and raised for `B02` and `B07`.

**Sub-rules.**
- Layout (Part D): 4 processes for 4 worlds, unless 4 threads in 1 process are more than 5% faster; then threads. 1 world on 4 threads is used only when there are fewer worlds than cores, or if it comes within 5% of the best layout. 1 world on 4 threads must give the same checksum as 1 world on 1 thread; if not, that is an `X11` failure.
- Checkpoint interval: every simulated month, unless one checkpoint costs more than 1% of the computing for a month at the lowest mind cost (100 people x 1 ms x 30 days = 3 s, so 30 ms). Then every simulated year.
- Session length (Part B): if the detached heartbeat outlives 2 hours, long runs use detached processes plus checkpoints. If it dies when the launching command or the session ends, runs are cut into pieces under 2 hours, each resuming from a checkpoint.
- Experiment 1 (Part E): any way of running that gives under 100 CPU-hours a week is marked "needs other computers", which must be raised with the owner first (`SCP-15`).

## Method

All code is Rust with no outside libraries (`src/`), plus small Python drivers. Timed work ran under the shared CPU lock. One cloud session: 4 cores (Xeon, 2.1 GHz base), 15 GiB memory, ext4 on a virtual disk.

- **A, sustained compute** (`parta.sh`, `b80 burn`): 4 threads each repeat one identical unit of work (a diffusion sweep over a 128 x 128 grid plus 2048 keyed random draws). Units are counted every 10 s, with CPU shares from `/proc/stat`; `vmstat` runs alongside every 60 s (its `st` column is steal). Two 10-minute halves, each under the lock.
- **B, what survives** (`heartbeat.sh`): started with `setsid nohup` at 13:25:19 UTC. It writes one line a minute with the time, its process id and the machine's boot id, and sleeps in between.
- **C, checkpoint and resume** (`partc.py`, `b80 world`): a toy world on a wrapping 256 x 256 grid with a diffusing food field. 3,000 agents settle at about 4,700; they move, eat, age, give birth and die. Every random draw is splitmix64 over a hash of (world seed, agent, day, purpose), so nothing about chance needs saving. 20 simulated years (7,300 days), a checkpoint every 30 days, the newest 3 kept. A checkpoint is written to a temporary file, flushed to disk, renamed, and the folder flushed. It is loaded only if its length, header and trailing hash all check out. Tests: 3 uninterrupted reference runs; 10 trials of two `SIGKILL`s at random moments, then resume to the end (trial 0 on 1 thread; the others switch between 1, 2 and 4 threads from piece to piece); a kill in the middle of a checkpoint write; and two damaged checkpoints (a truncated newer file, and one flipped bit in the newest). Extra (`partc_sizes.py`): the same world on grids up to 4096 x 4096, to time checkpoints from 0.4 to 67 MB.
- **D, parallel worlds** (`partd.py`): the same world and seed in every layout, so the work is identical and every checksum must match. Three mind costs: 0, 400 and 4,000 hash steps per agent per day (about 0, 2 and 22 microseconds). 1 world on 4 threads uses fixed read-then-write phases (`src/par.rs`). Each layout runs 3 times, interleaved, timed from process start to exit.
- **E, Experiment 1 arithmetic** (`parte.py`): CPU-hours = 3 x 100 x 500 x 365 x 100 person-days x mind cost. Ways of running use the measured cores per session; the rest are stated assumptions.

## Results

All timings are from runs under the lock. Raw data is in `results/`.

**A. Sustained compute** (20 one-minute readings, all 4 cores busy)

- Minute 1: 15.7 million units of work, about 65,000 per core per second.
- The other minutes against minute 1: median 98%, lowest 93.7%, highest 101.3%. Only minute 11 (the first of the second half) was under 95%. Minutes 6, 7 and 12 were at 96%.
- Steal: 1.5% on average, at most 1.9% in any minute. vmstat's `st` showed 1 or 2 every minute.
- No core lagged: each stayed within 3% of the average in every minute.
- The halves ran 13:39–13:49 and 13:53–14:03 UTC. Parts C and D kept the cores busy in the gap.

**B. What survives**

- The heartbeat started at 13:25:19 UTC in its own session; its parent is the system's first process. It kept a steady 60-second beat through every run, long after the command that started it had ended. At 14:07 UTC, the end of this test, it had run 42 minutes without missing a beat (43 lines, one boot id), and it is left running.
- The machine had already been up 3 h 22 min when I started. So while a session is in use, its container outlives the 2-hour limit on a single background command.
- Still open: whether a detached process lasts past 2 hours, and past the session going idle. In the log, a gap in the minutes means the container was paused. A new boot id, or no log at all, means a new machine.

**C. Checkpoint and resume** (20 simulated years, checkpoint every 30 days)

| Test | Result |
|---|---|
| 3 uninterrupted runs | same checksum each time |
| 10 trials, 2 random kills each (20 of 20 landed), thread count changed between pieces | all 10 identical to the uninterrupted run |
| Kill halfway through writing a checkpoint | half-written temp file (226 KB) never loaded, then removed; resumed from the checkpoint before; identical |
| Truncated newer file, and one flipped bit in the newest | both rejected (length, hash); resumed from the one before; identical |
| Size of one checkpoint | 375 KB (food field 256 KB, 24 bytes per agent) |
| Time to write one | median 2.0 ms (1.2 ms of it flushing to disk), worst 6.2 ms; medians of the 3 runs: 2.00, 2.01 and 2.15 ms |

Write time against size (median of 3 runs, each run's median over 10 writes):

| Size | Time | Rate |
|---|---|---|
| 0.4 MB | 1.8 ms | 213 MB/s |
| 4.3 MB | 12 ms | 358 MB/s |
| 17 MB | 47 ms | 361 MB/s |
| 67 MB | 187 ms | 360 MB/s |

Above 1 MB it grows in step with size, at about 360 MB/s (2.8 ms per MB): half of it is the toy's simple encoding, half is writing and flushing. Spread across the 3 runs: 3–8% from 4 MB up, up to 20% for the small sizes.

**D. Parallel worlds** (world-days per second, median of 3; in brackets, the speed-up over 1 world on 1 thread)

| Layout | Light minds | Medium | Heavy |
|---|---|---|---|
| 4 worlds, 4 processes | 13,870 (3.87x) | 366 (3.84x) | 47.2 (3.67x) |
| 4 worlds, 1 process | 13,038 (3.64x) | 365 (3.83x) | 46.0 (3.58x) |
| 1 world, 4 threads | 3,961 (1.11x) | 315 (3.30x) | 43.2 (3.36x) |
| 1 world, 1 thread | 3,581 | 95.3 | 12.9 |

- Mind cost per agent per day: light 0, medium about 2 µs, heavy about 22 µs. On 1 thread a world-day took 0.28, 10.5 and 78 ms.
- Spread (fastest minus slowest, over the median) was 1–11% for medium and heavy, and up to 19% for the short light runs; see `results/partd-summary.csv`.
- All 36 runs gave the same checksum as 1 world on 1 thread, in every layout.

**E. Experiment 1** (3 setups x 100 worlds x 500 years x 100 people = 5.5 billion person-days)

- Computing needed: 1,521 CPU-hours at 1 ms per person per day, 7,604 at 5 ms, and 76,042 at 50 ms.
- One world alone takes 5 h, 25 h or 253 h on one core. A background command lasts at most 2 hours, so every world has to move between sessions many times.
- A session gives 3.44 effective cores: 3.67 (4 processes, heavy minds, part D) times 0.937 (the slowest minute, part A).
- In "session-weeks" (one session running all week gives about 506 CPU-hours): 3 at 1 ms, 15 at 5 ms, 150 at 50 ms.

| Way of running | CPU-h a week | 1 ms | 5 ms | 50 ms |
|---|---|---|---|---|
| a) owner's sessions only | 39, under 100 | 39 wk | 196 wk | 1,965 wk |
| b) a routine each hour, 2 h each | 1,011 | 1.5 wk | 7.5 wk | 75 wk |
| b2) one routine session at a time | 433 | 3.5 wk | 17.5 wk | 175 wk |
| c) 4 sessions at once, all week | 2,023 | 0.8 wk | 3.8 wk | 38 wk |

Assumptions: (a) 15 hours of active sessions a week, and experiments get 3 of the 4 cores. It would take 39 hours a week to reach 100 CPU-hours. (b) a routine fires every hour and each session runs 2 hours, so 2 run at once; 15 minutes of every 2 hours go on setup and saving. (b2) one 1-hour session an hour, with 15 minutes of setup. (c) 4 sessions kept going all week. None of this checks the account's usage limits, which are unknown from here.

## Verdict

- **Condition 1 failed, but only just.** 19 of 20 minutes held 95%; one fell to 93.7%. It was not a slide: the next minutes recovered to 96–100%, steal stayed at 1.5%, and no core lagged. It looks like the host's clock speed varying with its other tenants. As the rule says, plan with the slowest minute (94%). Steal is under 5%, so all 4 cores stay busy.
- **Condition 2 held.** Every resumed run was bit-identical, even when the thread count changed at a resume. No partial or damaged checkpoint was ever loaded.
- **So the runner design stands:** one process per world, 4 at a time per session, and a checkpoint every simulated month (temporary file, flush, rename, check on load). Plan with 3.4 effective cores per session.
- **Layout:** 4 processes were never slower than 4 threads in 1 process (equal, up to 6% faster), so processes. 1 world on 4 threads reached 92% of the best layout with heavy minds but only 29% with light ones. Use it only when there are fewer worlds than cores, as on the phone (`B05`).
- **Checkpoint interval:** at 2 ms, a checkpoint is far under the 30 ms budget, so monthly. At 2.8 ms per MB, monthly stays under 1% while a world's saved state is under about 10 MB at 1 ms minds, 50 MB at 5 ms, or 500 MB at 50 ms. Above that, yearly.
- **Session length:** not settled yet. The heartbeat outlived its launching command and kept running throughout. The lead should check it after 15:25 UTC (2 hours), and again after the session has been idle.
- **Experiment 1:** using only the owner's sessions gives about 39 CPU-hours a week, under the 100 threshold. Experiment 1 would then need other computers, which must be raised with the owner first (`SCP-15`). Scheduled or parallel sessions could finish it in weeks, but only if minds cost about 5 ms or less, and only if usage limits allow. At 50 ms, even 4 sessions at once take 38 weeks (`RSK-14`).
- **The rule was framed too strictly.** One noisy minute out of 20 tripped condition 1. A test on the median or the trend would have fitted better. The design is the same either way; only the planning rate drops by 6%.
- **What made resume exact (for `B02`, `B05`, `B07`):**
  - draws keyed by (seed, being, day, purpose), so there is no generator state;
  - read-then-write phases with fixed splits;
  - counts kept as whole numbers, so their order cannot matter;
  - births, deaths and saving run in one fixed order;
  - scratch buffers rebuilt every day;
  - the full state saved in a fixed byte layout with a hash.

## Caveats

- The toy world is far simpler than the real one, so its 375 KB checkpoint is a floor. The mind costs are assumed, not measured (test `T1` measures them).
- Everything was measured in one session, on one machine type, in one afternoon. Other sessions may get different hardware or busier hosts.
- Other tests shared this container. The lock kept their heavy work out, but their light work may have added noise.
- Disk timings come from a virtual disk.
- Part E rests on assumptions not checked here: usage limits for routines and parallel sessions, whether a session nobody watches really runs 2 hours, and setup time.
- A fresh session starts on an empty machine. Between sessions, checkpoints must be kept somewhere outside the container. Where is not decided.

## What only the phone can answer

- Whether runs stay bit-identical when threads move between the phone's fast and slow cores (`X11`; `B02`, `B05`).
- Checkpoint time on the phone's storage, and whether Android killing the app mid-save is just as safe (`B78`).
- Sustained speed before the phone heats up (`B79`). The cloud cores held steady; the phone's will not.
- Moving saved moments to the phone, and the phone and cloud statistical match (`RES-05`).

## How to re-run

From this folder, with `CACHE` set to the shared cache folder (the scripts read it):

```
export CACHE=/path/to/shared/cache
LOCK=$CACHE/cpu.lock
flock $LOCK env CARGO_TARGET_DIR=$CACHE/b80-target cargo build --release
setsid nohup ./heartbeat.sh $CACHE/heartbeat.log >/dev/null 2>&1 < /dev/null &
flock $LOCK sh parta.sh half1
flock $LOCK sh parta.sh half2
python3 parta_summary.py
flock $LOCK python3 partc.py
flock $LOCK python3 partc_sizes.py
flock $LOCK python3 partd.py
python3 parte.py
```
