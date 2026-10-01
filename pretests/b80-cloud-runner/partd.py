#!/usr/bin/env python3
"""B80 / T6 part D: parallel worlds. World-days per second for:
  - 4 worlds as 4 processes (1 thread each)
  - 4 worlds in 1 process on 4 threads (1 thread per world)
  - 1 world on 4 threads (fixed read-then-write phases, par.rs)
  - 1 world on 1 thread (baseline)
at three per-agent "mind" costs. Every layout runs 3 times, interleaved, timed from outside
(process start to exit). All worlds use the same seed, so the work is identical in every layout and
every checksum must equal the 1-thread baseline (X11).

Run under the CPU lock:  flock <lock> python3 partd.py
Writes results/partd-runs.csv and results/partd-summary.csv.
"""
import csv
import os
import statistics
import subprocess
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
CACHE = Path(os.environ["CACHE"])  # the shared cache folder (see NOTES.md, How to re-run)
BIN = os.environ.get("B80_BIN", str(CACHE / "b80-target/release/b80"))
SEED = 5
REPS = 3
# (name, mind iterations per agent per day, simulated days); days chosen for ~4 s on 1 thread
SETTINGS = [("light", 0, 15000), ("medium", 400, 400), ("heavy", 4000, 50)]
LAYOUTS = ["4 worlds: 4 processes", "4 worlds: 1 process, 4 threads", "1 world: 4 threads", "1 world: 1 thread"]


def kv(line):
    return dict(p.split("=", 1) for p in line.split()[1:] if "=" in p)


def world_cmd(mind, days, threads):
    return [BIN, "world", "--seed", str(SEED), "--days", str(days), "--mind-iters", str(mind),
            "--threads", str(threads)]


def finals(out, tag):
    return [kv(l)["checksum"] for l in out.splitlines() if l.startswith(tag + " ")]


def run_layout(layout, mind, days):
    t0 = time.perf_counter()
    if layout == LAYOUTS[0]:
        ps = [subprocess.Popen(world_cmd(mind, days, 1), stdout=subprocess.PIPE, text=True) for _ in range(4)]
        outs = [p.communicate()[0] for p in ps]
        wall = time.perf_counter() - t0
        return 4, wall, [c for o in outs for c in finals(o, "FINAL")]
    if layout == LAYOUTS[1]:
        cmd = [BIN, "multi", "--worlds", "4", "--seed", str(SEED), "--days", str(days), "--mind-iters", str(mind)]
        out = subprocess.run(cmd, capture_output=True, text=True, check=True).stdout
        return 4, time.perf_counter() - t0, finals(out, "WORLD")
    threads = 4 if layout == LAYOUTS[2] else 1
    out = subprocess.run(world_cmd(mind, days, threads), capture_output=True, text=True, check=True).stdout
    return 1, time.perf_counter() - t0, finals(out, "FINAL")


def main():
    rows = []
    for name, mind, days in SETTINGS:
        base_sum = None
        for rep in range(REPS):
            for layout in LAYOUTS:
                worlds, wall, sums = run_layout(layout, mind, days)
                if layout == LAYOUTS[3] and base_sum is None:
                    base_sum = sums[0]
                rows.append({"setting": name, "mind_iters": mind, "days": days, "layout": layout, "rep": rep,
                             "worlds": worlds, "wall_s": round(wall, 4),
                             "world_days_per_s": round(worlds * days / wall, 2), "checksums": " ".join(sums)})
        for r in rows:
            if r["setting"] == name:
                r["same_as_1_thread"] = all(c == base_sum for c in r["checksums"].split())
    out = HERE / "results"
    out.mkdir(exist_ok=True)
    with open(out / "partd-runs.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)
    summary = []
    for name, mind, days in SETTINGS:
        base = statistics.median(r["world_days_per_s"] for r in rows
                                 if r["setting"] == name and r["layout"] == LAYOUTS[3])
        for layout in LAYOUTS:
            v = [r["world_days_per_s"] for r in rows if r["setting"] == name and r["layout"] == layout]
            summary.append({"setting": name, "mind_iters": mind, "layout": layout,
                            "world_days_per_s_median": round(statistics.median(v), 1),
                            "min": round(min(v), 1), "max": round(max(v), 1),
                            "spread_pct": round(100 * (max(v) - min(v)) / statistics.median(v), 1),
                            "speedup_vs_1_thread": round(statistics.median(v) / base, 2),
                            "ms_per_world_day_1_thread": round(1000 / base, 3),
                            "all_checksums_match": all(r["same_as_1_thread"] for r in rows
                                                       if r["setting"] == name and r["layout"] == layout)})
    with open(out / "partd-summary.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(summary[0].keys()))
        w.writeheader()
        w.writerows(summary)
    for s in summary:
        print(s)


if __name__ == "__main__":
    main()
