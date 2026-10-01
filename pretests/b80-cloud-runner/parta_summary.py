#!/usr/bin/env python3
"""B80 / T6 part A summary: merge the two 10-minute halves into 20 one-minute rows.
Writes results/parta-minutes.csv and results/parta-summary.json."""
import csv
import json
import re
import statistics
from pathlib import Path

R = Path(__file__).resolve().parent / "results"


def minutes(half):
    with open(R / f"parta-{half}.csv") as f:
        rows = list(csv.DictReader(f))
    nthreads = sum(1 for k in rows[0] if k.startswith("units_t"))
    out = []
    for m in range(len(rows) // 6):
        chunk = rows[m * 6:(m + 1) * 6]
        out.append({
            "units": sum(int(r["units_total"]) for r in chunk),
            "per_thread": [sum(int(r[f"units_t{t}"]) for r in chunk) for t in range(nthreads)],
            "steal_pct": statistics.mean(float(r["steal_pct"]) for r in chunk),
            "idle_pct": statistics.mean(float(r["idle_pct"]) for r in chunk),
        })
    return out


def vmstat(half):
    lines = (R / f"parta-{half}-vmstat.txt").read_text().splitlines()
    cols = next(l for l in lines if l.split()[:2] == ["r", "b"]).split()
    data = [l.split() for l in lines if re.match(r"^\s*\d", l)]
    stamps = [l.split(" ", 1)[1] for l in lines if l.startswith(("start ", "end "))]
    return [int(d[cols.index("st")]) for d in data[1:]], stamps  # first row is since boot


def main():
    rows = []
    stamps = {}
    for half in ("half1", "half2"):
        st, stamps[half] = vmstat(half)
        for i, m in enumerate(minutes(half)):
            rows.append({"minute": len(rows) + 1, "half": half, "units": m["units"],
                         "units_per_thread": " ".join(map(str, m["per_thread"])),
                         "steal_pct_procstat": round(m["steal_pct"], 3),
                         "vmstat_st": st[i] if i < len(st) else "",
                         "idle_pct": round(m["idle_pct"], 2)})
    first = rows[0]["units"]
    for r in rows:
        r["ratio_to_minute_1"] = round(r["units"] / first, 4)
    with open(R / "parta-minutes.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)
    ratios = [r["ratio_to_minute_1"] for r in rows]
    per_thread_min = [min(map(int, r["units_per_thread"].split())) / (r["units"] / 4) for r in rows]
    s = {
        "minutes": len(rows),
        "minute_1_units": first,
        "units_per_core_second_minute_1": round(first / 4 / 60, 1),
        "ratio_min": min(ratios), "ratio_median": statistics.median(ratios), "ratio_max": max(ratios),
        "minutes_below_95pct": sum(1 for x in ratios if x < 0.95),
        "steal_pct_mean": round(statistics.mean(r["steal_pct_procstat"] for r in rows), 3),
        "steal_pct_max_minute": max(r["steal_pct_procstat"] for r in rows),
        "vmstat_st_max": max(r["vmstat_st"] for r in rows if r["vmstat_st"] != ""),
        "slowest_core_vs_mean_min": round(min(per_thread_min), 4),
        "half_times_utc": stamps,
    }
    (R / "parta-summary.json").write_text(json.dumps(s, indent=1) + "\n")
    print(json.dumps(s, indent=1))


if __name__ == "__main__":
    main()
