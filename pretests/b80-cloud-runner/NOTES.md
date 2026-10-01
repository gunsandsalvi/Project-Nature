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
- **C, checkpoint and resume** (`partc.py`, `b80 world`): a toy world on a wrapping 256 x 256 grid with a diffusing food field. 3,000 agents settle at about 4,700; they move, eat, age, give birth and die. Every random draw is splitmix64 over a hash of (world seed, agent, day, purpose), so nothing about chance needs saving. 20 simulated years (7,300 days), a checkpoint every 30 days, the newest 3 kept. A checkpoint is written to a temporary file, flushed to disk, renamed, and the folder flushed. It is loaded only if its length, header and trailing hash all check out. Tests: 3 uninterrupted reference runs; 10 trials of two `SIGKILL`s at random moments, then resume to the end (trial 0 on 1 thread; the others switch between 1, 2 and 4 threads from piece to piece); a kill in the middle of a checkpoint write; and two damaged checkpoints (a truncated newer file, and one flipped bit in the newest).
- **D, parallel worlds** (`partd.py`): the same world and seed in every layout, so the work is identical and every checksum must match. Three mind costs: 0, 400 and 4,000 hash steps per agent per day (about 0, 2 and 22 microseconds). 1 world on 4 threads uses fixed read-then-write phases (`src/par.rs`). Each layout runs 3 times, interleaved, timed from process start to exit.
- **E, Experiment 1 arithmetic** (`parte.py`): CPU-hours = 3 x 100 x 500 x 365 x 100 person-days x mind cost. Ways of running use the measured cores per session; the rest are stated assumptions.

## Results

(filled in after the runs)

## Verdict

(filled in after the runs)
