#!/usr/bin/env python3
"""B80 / T6 part C extra: checkpoint write time against checkpoint size.
The toy world's grid is enlarged (field of 0.25 to 64 MB) to see how the write cost grows, because
a real world's saved state (B07) will be much bigger than the toy's 375 KB.
Run under the CPU lock:  flock <lock> python3 partc_sizes.py
Writes results/partc-sizes.csv (3 runs per size; 10 checkpoints per run)."""
import csv
import os
import shutil
import statistics
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
CACHE = Path(os.environ["CACHE"])  # the shared cache folder (see NOTES.md, How to re-run)
BIN = os.environ.get("B80_BIN", str(CACHE / "b80-target/release/b80"))
WORK = CACHE / "b80-partc-sizes"


def kv(line):
    return dict(p.split("=", 1) for p in line.split()[1:] if "=" in p)


def main():
    rows = []
    for side in (256, 512, 1024, 2048, 4096):
        runs = []
        for rep in range(3):
            d = WORK / f"{side}-{rep}"
            shutil.rmtree(d, ignore_errors=True)
            out = subprocess.run([BIN, "world", "--seed", "3", "--w", str(side), "--h", str(side), "--days", "30",
                                  "--every", "3", "--dir", str(d)], capture_output=True, text=True, check=True).stdout
            runs.append(kv(next(l for l in out.splitlines() if l.startswith("CKPT "))))
            shutil.rmtree(d, ignore_errors=True)
        tot = [float(r["total_us_median"]) / 1000 for r in runs]
        mb = int(runs[0]["bytes_last"]) / 1e6
        rows.append({"grid": f"{side}x{side}", "checkpoint_mb": round(mb, 2),
                     "total_ms_median": round(statistics.median(tot), 2), "total_ms_min": round(min(tot), 2),
                     "total_ms_max": round(max(tot), 2),
                     "fsync_ms_median": round(statistics.median(float(r["fsync_us_median"]) / 1000 for r in runs), 2),
                     "encode_ms_median": round(statistics.median(float(r["encode_us_median"]) / 1000 for r in runs), 2),
                     "mb_per_s": round(mb / (statistics.median(tot) / 1000), 0)})
        print(rows[-1])
    shutil.rmtree(WORK, ignore_errors=True)
    with open(HERE / "results" / "partc-sizes.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)


if __name__ == "__main__":
    main()
