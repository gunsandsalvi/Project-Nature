"""P4's runs (IMPLEMENTATION α0.3a): the sharp-stone test and its control, one band each run (RES-02, RES-03); fire's
window, counted as TIM-19 counts a step, at its first anywhere in a world of 3 or 4 bands; the tuning of the
discovery factors on 20 fixed seeds and its check on 20 seeds never tuned against (RES-16); and how the pace moves
when each tuned value alone is halved or doubled. Writes the report the app's Reports page draws.

    python3 prototypes/discovery/pace.py            # the runs and the report, with the tuning as it stands
    python3 prototypes/discovery/pace.py --tune     # tunes the discovery factors first, and writes them back

Pre-production code (research 00): thrown away once its answer is in the architecture (A12).
"""

import copy
import json
import math
import re
import statistics
import sys
from multiprocessing import Pool
from pathlib import Path

import discovery as d

ROOT = Path(__file__).resolve().parents[2]
REPORT = ROOT / "prototypes" / "app" / "reports" / "p4.json"
TUNING = d.HERE / "tuning.toml"
SEEDS = list(range(1, 21))  # the 20 fixed tuning seeds (RES-16)
FRESH = list(range(1001, 1021))  # 20 seeds never tuned against, for the closing check
# the halvings and doublings run on 40 seeds, the 20 above and 20 more, so a median moves by the change and not by
# chance
SWEEP = list(range(1, 41))
FLAKE_YEARS = 7  # RES-02: each run until 2 years after its first flake, or 5 years if none comes; the control 7
YEARS = 60  # a world runs until its first fire, or this long
# what tuning aims at: flakes about a year and a half in; fire about 12 years in, the middle of its window as its
# years multiply, so that halving or doubling a value keeps both inside
FLAKE_AIM = 1.5
FIRE_AIM = 12.0
FIRE_WINDOW = (5.0, 30.0)
# the values a halving or doubling is tried on (MND-11's list of what tunes the pace)
KNOBS = {
    "accident": [("routes", "accident")],
    "experiment": [("routes", "experiment")],
    "hunch": [("routes", "hunch")],
    "flake factor": [("blueprints", "flake", "factor")],
    "drill factor": [("blueprints", "drill", "factor")],
    "plough factor": [("blueprints", "plough", "factor")],
    "noticing": [("minds", "notice_least"), ("minds", "notice_most")],
    "experimenting": [("minds", "experiment_curious"), ("minds", "experiment_average")],
    "dream hints": [("minds", "dream_hint"), ("minds", "dream_hint_need")],
}


def one(job):
    """One band's sharp-stone run (RES-02): until 2 years after its first flake, when the share of its adults who can
    make flakes is taken, or 5 years if none comes."""
    seed, tuning, flint, years = job
    seen = {}

    def stop(band):
        if "flake" in band.first:
            if band.day == band.first["flake"][0] + 2 * d.YEAR:
                seen["spread"] = d.adults_who_know(band, "flake")
                return True
            return False
        return band.day >= 5 * d.YEAR and flint is None

    band = d.Band(seed, tuning, flint).run(years, stop)
    flake = band.first.get("flake")
    return {
        "seed": seed,
        "flake": round(flake[0] / d.YEAR, 2) if flake else None,
        "flake_route": flake[1] if flake else None,
        "spread": round(seen["spread"], 3) if "spread" in seen else None,
        "lost": [[round(day / d.YEAR, 2), bp] for day, bp in band.lost],
    }


def runs(tuning, seeds, flint=None, years=FLAKE_YEARS, pool=None):
    jobs = [(s, tuning, flint, years) for s in seeds]
    return pool.map(one, jobs) if pool is not None else [one(j) for j in jobs]


def world(job):
    """A world's first fire, as TIM-19 counts a step, at its first entry anywhere: the world's 3 or 4 bands
    (BIO-03) each run apart until it finds fire or passes the first found so far, the first band starting with a fire
    taken from lightning (BIO-02)."""
    seed, tuning, years = job
    bands = 3 + seed % 2
    best = None
    for k in range(bands):
        cap = years if best is None else best[0] / d.YEAR
        band = d.Band(seed * 10 + k, tuning, fire=k == 0).run(cap, lambda b: d.fire_first(b) is not None)
        found = d.fire_first(band)
        if found is not None and (best is None or found[0] < best[0]):
            best = found
    return {
        "seed": seed,
        "bands": bands,
        "fire": round(best[0] / d.YEAR, 2) if best else None,
        "fire_way": best[1] if best else None,
        "fire_route": best[2] if best else None,
    }


def worlds(tuning, seeds, years=YEARS, pool=None):
    jobs = [(s, tuning, years) for s in seeds]
    return pool.map(world, jobs) if pool is not None else [world(j) for j in jobs]


def sharp_stone(results, control):
    """RES-03's pass rule: flakes within 5 years in at least 16 of 20 runs; in those, 3 in 4 adults able to make them
    within 2 years of the first; at least two routes; and never a flake without stone that flakes."""
    found = [r for r in results if r["flake"] is not None and r["flake"] <= 5.0]
    spread = [r for r in found if r["spread"] is not None and r["spread"] >= 0.75]
    routes = sorted({r["flake_route"] for r in found})
    checks = {
        "discovery": len(found) * 5 >= len(results) * 4,
        "spread": len(found) > 0 and len(spread) == len(found),
        "routes": len(routes) >= 2,
        "control": all(r["flake"] is None for r in control),
    }
    return {
        "within_5": len(found),
        "spread_ok": len(spread),
        "routes": routes,
        "control_flakes": sum(1 for r in control if r["flake"] is not None),
        "checks": checks,
        "pass": all(checks.values()),
    }


def fire_window(results):
    """TIM-19's window as RES-07 checks a target: at least half the worlds reach it inside its window, at most a
    quarter before it opens."""
    years = [r["fire"] for r in results]
    inside = sum(1 for y in years if y is not None and FIRE_WINDOW[0] <= y <= FIRE_WINDOW[1])
    early = sum(1 for y in years if y is not None and y < FIRE_WINDOW[0])
    ways = sorted({r["fire_way"] for r in results if r["fire_way"]})
    return {
        "inside": inside,
        "early": early,
        "never": sum(1 for y in years if y is None),
        "ways": ways,
        "pass": inside * 2 >= len(years) and early * 4 <= len(years),
    }


def median_year(results, key, cap):
    """The median year a step came, a run that never found it counting as the cap."""
    return statistics.median(r[key] if r[key] is not None else cap for r in results)


def setting(tuning, path, value=None, scale=None):
    node = tuning
    for k in path[:-1]:
        node = node[k]
    if scale is not None:
        node[path[-1]] *= scale
    elif value is not None:
        node[path[-1]] = value
    return node[path[-1]]


def tune(tuning, pool):
    """RES-16: each discovery factor found by bisection in its logarithm, on the 20 tuning seeds, until the median
    year meets its aim: the flake's factor on single bands; drill and plough together on worlds, plough kept at the
    drill's factor."""
    tuned = copy.deepcopy(tuning)
    for name, paths, aim in (
        ("flake", [("blueprints", "flake", "factor")], FLAKE_AIM),
        ("fire", [("blueprints", "drill", "factor"), ("blueprints", "plough", "factor")], FIRE_AIM),
    ):
        lo, hi = math.log(1e-4), math.log(1.0)
        for _ in range(14):
            mid = (lo + hi) / 2.0
            for path in paths:
                setting(tuned, path, value=math.exp(mid))
            if name == "flake":
                year = median_year(runs(tuned, SEEDS, pool=pool), "flake", 5.0)
            else:
                year = median_year(worlds(tuned, SEEDS, pool=pool), "fire", YEARS)
            print(f"  {name} factor {math.exp(mid):.4g}: median year {year:.2f}", flush=True)
            if year > aim:
                lo = mid
            else:
                hi = mid
        value = float(f"{math.exp((lo + hi) / 2.0):.3g}")
        for path in paths:
            setting(tuned, path, value=value)
        print(f"Tuned {name}: factor {value}", flush=True)
    return tuned


def write_factors(tuning):
    """The tuned factors written back into tuning.toml, each line keeping its comment."""
    text = TUNING.read_text()
    for bp in ("flake", "drill", "plough"):
        value = tuning["blueprints"][bp]["factor"]
        pattern = rf"(\[blueprints\.{bp}\](?:\n[^\[\n][^\n]*)*?\nfactor = )[0-9.e-]+"
        text, n = re.subn(pattern, rf"\g<1>{value}", text)
        assert n == 1, bp
    TUNING.write_text(text)


def summary(flakes, fires):
    """A sweep's runs in short: the median years, and whether each step still meets its rule, scaled to the runs'
    number: flakes within 5 years in 4 runs of 5 (RES-03), fire inside its window in half the worlds and early in at
    most a quarter (TIM-19)."""
    within = sum(1 for r in flakes if r["flake"] is not None and r["flake"] <= 5.0)
    fire = fire_window(fires)
    return {
        "flake_median": median_year(flakes, "flake", 5.0),
        "fire_median": median_year(fires, "fire", YEARS),
        "flake_within_5": within,
        "fire_inside": fire["inside"],
        "fire_early": fire["early"],
        "fire_never": fire["never"],
        "flake_pass": within * 5 >= len(flakes) * 4,
        "fire_pass": fire["pass"],
    }


def sensitivity(tuning, pool):
    """How the pace moves when each tuned value alone is halved or doubled, over 40 runs and 40 worlds. A value holds
    the pace on a knife's edge where halving or doubling it breaks a step's rule."""
    out = [{"knob": "as tuned", "1.0": summary(runs(tuning, SWEEP, pool=pool), worlds(tuning, SWEEP, pool=pool))}]
    for name, paths in KNOBS.items():
        row = {"knob": name}
        for scale in (0.5, 2.0):
            varied = copy.deepcopy(tuning)
            for path in paths:
                setting(varied, path, scale=scale)
            row[str(scale)] = summary(runs(varied, SWEEP, pool=pool), worlds(varied, SWEEP, pool=pool))
        row["edge"] = not all(row[s][k] for s in ("0.5", "2.0") for k in ("flake_pass", "fire_pass"))
        out.append(row)
        low, high = row["0.5"], row["2.0"]
        print(
            f"  {name}: flakes {low['flake_median']:.2f} to {high['flake_median']:.2f} years, fire "
            f"{low['fire_median']:.1f} to {high['fire_median']:.1f} ({low['fire_never']} and {high['fire_never']} "
            f"never){' EDGE' if row['edge'] else ''}",
            flush=True,
        )
    return out


def main():
    tuning = d.load(TUNING)
    # two workers, leaving room for the rest of the session's work
    with Pool(2) as pool:
        if "--tune" in sys.argv:
            tuning = tune(tuning, pool)
            write_factors(tuning)
        base = runs(tuning, SEEDS, pool=pool)
        control = runs(tuning, SEEDS, flint=0.0, pool=pool)
        fires = worlds(tuning, SEEDS, pool=pool)
        fresh = runs(tuning, FRESH, pool=pool)
        fresh_control = runs(tuning, FRESH, flint=0.0, pool=pool)
        fresh_fires = worlds(tuning, FRESH, pool=pool)
        print("Sensitivity, each value halved and doubled:", flush=True)
        sens = sensitivity(tuning, pool)
    stone = sharp_stone(base, control)
    fire = fire_window(fires)
    stone_fresh = sharp_stone(fresh, fresh_control)
    fire_fresh = fire_window(fresh_fires)
    edge = [row["knob"] for row in sens if row.get("edge")]
    report = {
        "title": "P4 Discovery pace",
        "question": "Can tuning alone make sharp flakes come within 5 years and fire within its window?",
        "tuning": {bp: tuning["blueprints"][bp]["factor"] for bp in ("flake", "drill", "plough")},
        "runs": base,
        "control": control,
        "worlds": fires,
        "fresh": fresh,
        "fresh_worlds": fresh_fires,
        "sharp_stone": stone,
        "fire": fire,
        "sharp_stone_fresh": stone_fresh,
        "fire_fresh": fire_fresh,
        "sensitivity": sens,
        "knife_edge": edge,
        "pass": stone["pass"] and fire["pass"] and stone_fresh["pass"] and fire_fresh["pass"] and not edge,
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=1) + "\n")
    print(
        f"Flakes: {stone['within_5']}/20 within 5 years (fresh seeds {stone_fresh['within_5']}/20), "
        f"spread in {stone['spread_ok']}, routes {', '.join(stone['routes'])}, control flakes "
        f"{stone['control_flakes']}"
    )
    print(
        f"Fire, first in a world: {fire['inside']}/20 in years 5-30, {fire['early']} before (fresh "
        f"{fire_fresh['inside']}/20, {fire_fresh['early']}), ways {', '.join(fire['ways'])}"
    )
    print(f"Knife's edge: {', '.join(edge) or 'none'}")
    print(f"P4: {'PASS' if report['pass'] else 'FAIL'}, report {REPORT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
