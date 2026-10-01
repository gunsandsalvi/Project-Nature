#!/usr/bin/env python3
"""B80 / T6 part E: the arithmetic for Experiment 1 (SCP-15, RSK-14).
3 setups x 100 worlds x 500 simulated years, about 100 people per world, at three mind costs.
Uses the measured effective cores per session (parts A and D); the ways of running are assumptions,
stated below. Writes results/parte.csv and prints a table. Cheap: no lock needed."""
import csv
import json
from pathlib import Path

R = Path(__file__).resolve().parent / "results"
SETUPS, WORLDS, YEARS, DAYS_PER_YEAR, PEOPLE = 3, 100, 500, 365, 100
MIND_MS = [1, 5, 50]  # computing per person per simulated day, on one cloud core
THRESHOLD = 100       # CPU-hours a week; below this Experiment 1 needs other computers (SCP-15)

# Measured: cores a session really delivers = (4-world speed-up over 1 world, part D heavy minds)
# x (slowest minute of 20 / first minute, part A).
a = json.loads((R / "parta-summary.json").read_text())
with open(R / "partd-summary.csv") as f:
    d = [r for r in csv.DictReader(f) if r["setting"] == "heavy" and r["layout"] == "4 worlds: 4 processes"][0]
EFF_CORES = float(d["speedup_vs_1_thread"]) * min(1.0, a["ratio_min"])

# Assumptions for the ways of running (CPU-hours a week = sessions x cores x share x hours).
MODES = [
    ("a) only while the owner's sessions are active: 15 h a week, experiments get 3 of 4 cores",
     EFF_CORES * 3 / 4 * 15),
    ("b) scheduled fresh sessions: one routine an hour, each runs 2 h (so 2 at once), "
     "15 min of each spent on setup and saving", 2 * EFF_CORES * (105 / 120) * 168),
    ("b2) same, but only one session at a time (1-hour runs, 15 min setup)", EFF_CORES * (45 / 60) * 168),
    ("c) 4 sessions at once, kept going all week, 15 min per 2 h on setup", 4 * EFF_CORES * (105 / 120) * 168),
]


def main():
    person_days = SETUPS * WORLDS * YEARS * DAYS_PER_YEAR * PEOPLE
    rows = []
    for ms in MIND_MS:
        cpu_h = person_days * ms / 1000 / 3600
        world_h_1core = YEARS * DAYS_PER_YEAR * PEOPLE * ms / 1000 / 3600
        for mode, per_week in MODES:
            rows.append({"mind_ms": ms, "cpu_hours": round(cpu_h), "hours_per_world_on_1_core": round(world_h_1core, 1),
                         "mode": mode, "cpu_hours_per_week": round(per_week), "weeks": round(cpu_h / per_week, 1),
                         "under_threshold": per_week < THRESHOLD})
    with open(R / "parte.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)
    print(f"effective cores per session: {EFF_CORES:.2f}; person-days: {person_days:.4g}")
    print("| way of running | CPU-h/week | 1 ms | 5 ms | 50 ms |")
    print("|---|---|---|---|---|")
    for mode, per_week in MODES:
        cells = [f"{r['weeks']:g} wk" for r in rows if r["mode"] == mode]
        flag = " (under 100: needs other computers)" if per_week < THRESHOLD else ""
        print(f"| {mode.split(':')[0]} | {per_week:.0f}{flag} | " + " | ".join(cells) + " |")
    for ms in MIND_MS:
        r = [x for x in rows if x["mind_ms"] == ms][0]
        print(f"{ms} ms: {r['cpu_hours']} CPU-hours in all; one world takes {r['hours_per_world_on_1_core']} h on one core")
    print("break-even for (a): hours of active session a week for 100 CPU-h:",
          round(THRESHOLD / (EFF_CORES * 3 / 4), 1))
    per_session_week = EFF_CORES * (105 / 120) * 168  # one session kept running all week
    for ms in MIND_MS:
        cpu_h = person_days * ms / 1000 / 3600
        print(f"{ms} ms: {cpu_h / per_session_week:.0f} session-weeks "
              f"(one session running all week gives {per_session_week:.0f} CPU-h)")


if __name__ == "__main__":
    main()
