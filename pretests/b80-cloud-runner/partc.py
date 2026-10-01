#!/usr/bin/env python3
"""B80 / T6 part C: checkpoint and resume (crossroads X11, B07 stand-in).

Run under the CPU lock:  flock <lock> python3 partc.py
Writes results/partc.json and prints a summary.

1. Reference: an uninterrupted run to day D (3 repeats, same checksum each time).
2. Kill trials: start, SIGKILL at a random moment, resume, SIGKILL again at a random moment,
   resume to the end; the final checksum must equal the reference. Trial 0 is the required one
   (1 thread throughout); later trials also switch thread counts between pieces.
3. Kill in the middle of a checkpoint write: the half-written temp file must never be loaded.
4. Damaged checkpoints: a truncated newer one and a bit-flipped newest one must be rejected.
5. Checkpoint size and write time (from the reference runs).
"""
import json
import os
import random
import shutil
import signal
import subprocess
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
CACHE = Path(os.environ.get(
    "CACHE", "/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/cache"))
BIN = os.environ.get("B80_BIN", str(CACHE / "b80-target/release/b80"))
WORK = CACHE / "b80-partc"
SEED, DAYS, EVERY, MIND = 11, 7300, 30, 20  # 20 simulated years, a checkpoint every simulated month
KILL_SEED = 2026_10_01  # seeds the choice of kill moments, so the trial can be repeated
TRIALS = 10


def base(threads):
    return [BIN, "world", "--seed", str(SEED), "--days", str(DAYS), "--every", str(EVERY),
            "--mind-iters", str(MIND), "--threads", str(threads)]


def kv(line):
    return dict(p.split("=", 1) for p in line.split()[1:] if "=" in p)


def parse(out, tag):
    for line in out.splitlines():
        if line.startswith(tag + " "):
            return kv(line)
    return None


def fresh(name):
    d = WORK / name
    shutil.rmtree(d, ignore_errors=True)
    d.mkdir(parents=True)
    return d


def run_to_end(d, threads=1, resume=True):
    cmd = base(threads) + ["--dir", str(d)] + (["--resume"] if resume else [])
    p = subprocess.run(cmd, capture_output=True, text=True, check=True)
    return parse(p.stdout, "FINAL"), parse(p.stdout, "CKPT"), p.stderr


def run_and_kill(d, after_s, threads=1):
    p = subprocess.Popen(base(threads) + ["--dir", str(d), "--resume"],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    time.sleep(after_s)
    p.send_signal(signal.SIGKILL)
    _, err = p.communicate()
    return p.returncode, err


def resumed_day(err):
    for line in err.splitlines():
        if line.startswith("RESUME "):
            return int(kv(line)["day"])
    return 0


def listing(d):
    return sorted((f.name, f.stat().st_size) for f in d.iterdir())


def main():
    rng = random.Random(KILL_SEED)
    res = {"seed": SEED, "days": DAYS, "every": EVERY, "mind_iters": MIND, "kill_seed": KILL_SEED}

    # 1. Reference runs.
    refs = []
    for i in range(3):
        fin, ck, _ = run_to_end(fresh(f"ref{i}"), resume=False)
        refs.append({"checksum": fin["checksum"], "wall_s": float(fin["wall_s"]),
                     "agents": int(fin["agents"]), "ckpt": ck})
    ref = refs[0]["checksum"]
    t_ref = sorted(r["wall_s"] for r in refs)[1]
    res["reference"] = refs
    res["reference_identical"] = all(r["checksum"] == ref for r in refs)

    # 2. Kill trials: two SIGKILLs at random moments, then resume to the end.
    trials = []
    for t in range(TRIALS):
        d = fresh(f"kill{t}")
        threads = [1, 1, 1] if t == 0 else [rng.choice([1, 2, 4]) for _ in range(3)]
        pieces = []
        start_day = 0
        for piece in range(2):
            # rough speed-up of this toy world on 2 and 4 threads, so the kill lands before the end
            remaining = t_ref * (1 - start_day / DAYS) / {1: 1.0, 2: 1.3, 4: 1.6}[threads[piece]]
            after = round(rng.uniform(0.1, 0.9) * remaining, 3)
            rc, err = run_and_kill(d, after, threads[piece])
            pieces.append({"threads": threads[piece], "killed_after_s": after, "returncode": rc,
                           "resumed_from_day": resumed_day(err),
                           "newest_ckpt_after_kill": max((n for n, _ in listing(d) if n.endswith(".bin")),
                                                         default=None)})
            start_day = max([int(n[5:15]) for n, _ in listing(d) if n.endswith(".bin")] or [0])
        fin, _, err = run_to_end(d, threads[2])
        trials.append({"pieces": pieces, "final_threads": threads[2], "final_resumed_from_day": resumed_day(err),
                       "checksum": fin["checksum"], "identical": fin["checksum"] == ref})
    res["kill_trials"] = trials

    # 3. Kill in the middle of writing a checkpoint.
    d = fresh("midwrite")
    p = subprocess.Popen(base(1) + ["--dir", str(d), "--slow-write-ms", "400"],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    target = 5 * EVERY
    while True:
        line = p.stderr.readline()
        if not line:
            break
        if line.startswith("CKPT-WRITING") and int(kv(line)["day"]) == target:
            time.sleep(0.1)
            p.send_signal(signal.SIGKILL)
            break
    p.communicate()
    files_after_kill = listing(d)
    fin, _, err = run_to_end(d, 1)
    res["mid_write"] = {
        "killed_while_writing_day": target,
        "files_after_kill": files_after_kill,
        "temp_file_left_bytes": dict(files_after_kill).get(f"ckpt-{target:010d}.bin.tmp"),
        "resume_log": [l for l in err.splitlines() if l.split(" ")[0] in ("RESUME", "REJECT", "REMOVED-TEMP")],
        "resumed_from_day": resumed_day(err),
        "checksum": fin["checksum"],
        "identical": fin["checksum"] == ref,
        "partial_never_loaded": resumed_day(err) == target - EVERY and not any(
            n == f"ckpt-{target:010d}.bin" for n, _ in files_after_kill),
    }

    # 4. Damaged checkpoints: truncate a fake newer one, flip a byte in the real newest one.
    d = fresh("damaged")
    run_and_kill(d, round(0.5 * t_ref, 3), 1)
    ck = [n for n, _ in listing(d) if n.endswith(".bin")]
    newest = max(ck)
    nday = int(newest[5:15])
    raw = (d / newest).read_bytes()
    fake = f"ckpt-{nday + EVERY:010d}.bin"
    (d / fake).write_bytes(raw[: int(len(raw) * 0.6)])
    flipped = bytearray(raw)
    flipped[len(raw) // 2] ^= 0x01
    (d / newest).write_bytes(bytes(flipped))
    fin, _, err = run_to_end(d, 1)
    res["damaged"] = {
        "truncated_fake": fake, "bit_flipped": newest,
        "resume_log": [l for l in err.splitlines() if l.split(" ")[0] in ("RESUME", "REJECT", "REMOVED-TEMP")],
        "resumed_from_day": resumed_day(err), "expected_day": nday - EVERY,
        "checksum": fin["checksum"], "identical": fin["checksum"] == ref,
    }

    # 5. Summary.
    ck = refs[0]["ckpt"]
    res["summary"] = {
        "reference_checksum": ref,
        "reference_wall_s_median": t_ref,
        "all_kill_trials_identical": all(t["identical"] for t in trials),
        "kills_landed": sum(p["returncode"] == -9 for t in trials for p in t["pieces"]),
        "kills_tried": sum(len(t["pieces"]) for t in trials),
        "mid_write_ok": res["mid_write"]["identical"] and res["mid_write"]["partial_never_loaded"],
        "damaged_ok": res["damaged"]["identical"] and res["damaged"]["resumed_from_day"] == nday - EVERY,
        "ckpt_bytes": int(ck["bytes_last"]),
        "ckpt_total_ms_median_per_run": sorted(float(r["ckpt"]["total_us_median"]) / 1000 for r in refs),
        "ckpt_fsync_ms_median_per_run": sorted(float(r["ckpt"]["fsync_us_median"]) / 1000 for r in refs),
        "ckpt_share_of_wall_per_run": sorted(float(r["ckpt"]["share_of_wall"]) for r in refs),
    }
    out = HERE / "results" / "partc.json"
    out.parent.mkdir(exist_ok=True)
    out.write_text(json.dumps(res, indent=1) + "\n")
    print(json.dumps(res["summary"], indent=1))


if __name__ == "__main__":
    main()
