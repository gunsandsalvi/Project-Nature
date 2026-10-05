"""P10's runs (IMPLEMENTATION α0.6b, CUL-33): 20 worlds of three bands, each run 100 years with the tuning as it
stands; when a custom, a shared spirit, a rite and a band split first came in each, against CUL-33's windows, and
the events behind each; and one run's story. Writes the report the app's Reports page draws.

    python3 prototypes/culture/runs.py    # about a minute on four cores

Pre-production code (research 00): thrown away once its answer is in the architecture (A13).
"""

import json
import statistics
from multiprocessing import Pool
from pathlib import Path

import culture as c

ROOT = Path(__file__).resolve().parents[2]
REPORT = ROOT / "prototypes" / "app" / "reports" / "p10.json"
SEEDS = list(range(1, 21))
YEARS = 100
# CUL-33's windows, in years from the start; it gives none for customs, so a year is proposed (as P10 found them)
WINDOWS = {"custom": [0.0, 1.0], "spirit": [0.0, 3.0], "rite": [3.0, 10.0], "split": [5.0, 25.0]}
SAYS = {"custom": "a custom", "spirit": "a shared spirit", "rite": "a rite a band keeps", "split": "a band split"}


def one(seed):
    """One world's run: each first, with its year, its name and the events behind it, and what it had by its end."""
    w = c.World(seed, c.load())
    w.run(YEARS)
    firsts = {}
    for kind in WINDOWS:
        f = w.firsts.get(kind)
        if f is not None:
            story = w.story(kind)
            firsts[kind] = {
                "year": round(f["day"] / c.YEAR, 2),
                "name": f["name"],
                "band": w.bands[f["band"]].name,
                "story": story,
                # traced: at least one event before the first itself
                "traced": len(story) >= 2,
            }
    alive = [b for b in w.bands if b.alive]
    end = {
        "bands": len(alive),
        "people": sum(1 for p in w.people if p.alive),
        "spirits": sum(len(b.spirits) for b in alive),
        "rites": sum(len(b.rites) for b in alive),
        "customs": sum(len(b.customs) for b in alive),
    }
    return {"seed": seed, "firsts": firsts, "end": end}


def report(runs):
    kinds = {}
    for kind, (lo, hi) in WINDOWS.items():
        years = [r["firsts"][kind]["year"] for r in runs if kind in r["firsts"]]
        inside = sum(1 for y in years if lo <= y <= hi)
        traced = sum(1 for r in runs if kind in r["firsts"] and r["firsts"][kind]["traced"])
        kinds[kind] = {
            "says": SAYS[kind],
            "window": [lo, hi],
            "inside": inside,
            "came": len(years),
            "traced": traced,
            "median": round(statistics.median(years), 1) if years else None,
            "early": sum(1 for y in years if y < lo),
            "late": sum(1 for y in years if y > hi) + len(runs) - len(years),
        }
    passed = all(k["inside"] * 2 >= len(runs) and k["traced"] == k["came"] for k in kinds.values())
    return {
        "title": "P10 Culture from causes",
        "question": (
            "Do customs, a spirit, a rite and a band split arise inside their windows, each from its own cause?"
        ),
        "years": YEARS,
        "kinds": kinds,
        "runs": [
            {"seed": r["seed"], "firsts": {k: v["year"] for k, v in r["firsts"].items()}, "end": r["end"]} for r in runs
        ],
        "story": {k: {"name": v["name"], "band": v["band"], "lines": v["story"]} for k, v in runs[0]["firsts"].items()},
        "pass": passed,
    }


if __name__ == "__main__":
    with Pool(4) as pool:
        runs = pool.map(one, SEEDS)
    r = report(runs)
    REPORT.write_text(json.dumps(r, indent=1) + "\n")
    for k in r["kinds"].values():
        print(
            f"{k['says']}: {k['inside']} of {len(runs)} inside Years {k['window'][0]:g} to {k['window'][1]:g}, "
            f"median {k['median']}, {k['early']} early, {k['late']} late, {k['traced']} traced"
        )
    print("PASS" if r["pass"] else "FAIL", REPORT)
