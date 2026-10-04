"""P4's runs (IMPLEMENTATION α0.3a): the sharp-stone test and its control, one band each run (RES-02, RES-03); fire's
window, counted as TIM-19 counts a step, at its first anywhere in a world of 3 or 4 bands; the tuning of the
discovery factors on 20 fixed seeds and its check on 20 seeds never tuned against (RES-16); and how the pace moves
when each tuned value alone is halved or doubled. Writes the report the app's Reports page draws.

    python3 prototypes/discovery/pace.py            # the runs and the report, with the tuning as it stands
    python3 prototypes/discovery/pace.py --tune     # tunes the discovery factors first, and writes them back

Pre-production code (research 00): thrown away once its answer is in the architecture (A12).
"""

import copy
import hashlib
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
# TIM-19's windows, halved for faster discoveries as you asked on 4 October 2026. They are dates (TIM-14), and Year 1
# begins at the start, so in years from the start: flakes in Years 1 to 3, from 0 up to 3; fire in Years 3 to 15,
# from 2 up to 15
FLAKE_WINDOW = (0.0, 3.0)
FIRE_WINDOW = (2.0, 15.0)
# what tuning aims at: flakes about a year in, as you asked; fire about 9 years in. A world's first fire comes
# anywhere from under a year to past 15 (the first of 3 or 4 bands, each slow to find it), its spread skewed early, so
# its median sits above the window's middle as its years multiply (5.5): there the shares before and after the window
# are about equal, and a quarter's change in any value keeps both within the rule
FLAKE_AIM = 1.0
FIRE_AIM = 9.0
# the runs done so far, by what each was asked: a run of the program reuses them, so a restart loses nothing
# (the folder is ignored by git)
CACHE = d.HERE / ".runs" / "cache.json"
_cache = None
# the values a change is tried on (MND-11's list of what tunes the pace)
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


def mapped(fn, jobs, pool):
    """Each job's result, from the pool or here, and from the cache of runs done before when it is on (main)."""
    if _cache is None:
        return pool.map(fn, jobs) if pool is not None else [fn(j) for j in jobs]
    keys = [hashlib.sha1(json.dumps([fn.__name__, job], sort_keys=True).encode()).hexdigest() for job in jobs]
    todo = [(k, j) for k, j in zip(keys, jobs, strict=True) if k not in _cache]
    if todo:
        done = pool.map(fn, [j for _, j in todo]) if pool is not None else [fn(j) for _, j in todo]
        for (k, _), result in zip(todo, done, strict=True):
            _cache[k] = result
        CACHE.parent.mkdir(exist_ok=True)
        # written whole beside it, then renamed over it, so a run cut short mid-write leaves the last cache intact
        part = CACHE.with_suffix(".part")
        part.write_text(json.dumps(_cache))
        part.replace(CACHE)
    return [_cache[k] for k in keys]


def runs(tuning, seeds, flint=None, years=FLAKE_YEARS, pool=None):
    return mapped(one, [(s, tuning, flint, years) for s in seeds], pool)


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
    return mapped(world, [(s, tuning, years) for s in seeds], pool)


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


def window(years, span):
    """TIM-19's window as RES-07 checks a target: at least half the runs reach it inside its window, at most a
    quarter before it opens."""
    inside = sum(1 for y in years if y is not None and span[0] <= y < span[1])
    early = sum(1 for y in years if y is not None and y < span[0])
    return {
        "inside": inside,
        "early": early,
        "never": sum(1 for y in years if y is None),
        "pass": inside * 2 >= len(years) and early * 4 <= len(years),
    }


def fire_window(results):
    """Fire's window over a set of worlds, and the ways it was first made."""
    out = window([r["fire"] for r in results], FIRE_WINDOW)
    out["ways"] = sorted({r["fire_way"] for r in results if r["fire_way"]})
    return out


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
    number: flakes within 5 years in 4 runs of 5 (RES-03) and inside their window in half (TIM-19), fire inside its
    window in half the worlds and early in at most a quarter (TIM-19)."""
    within = sum(1 for r in flakes if r["flake"] is not None and r["flake"] <= 5.0)
    flake = window([r["flake"] for r in flakes], FLAKE_WINDOW)
    fire = fire_window(fires)
    return {
        "flake_median": median_year(flakes, "flake", 5.0),
        "fire_median": median_year(fires, "fire", YEARS),
        "flake_within_5": within,
        "flake_inside": flake["inside"],
        "fire_inside": fire["inside"],
        "fire_early": fire["early"],
        "fire_never": fire["never"],
        "flake_pass": within * 5 >= len(flakes) * 4 and flake["pass"],
        "fire_pass": fire["pass"],
    }


def holds(row, scales):
    """Whether both steps keep their rules with the value changed by each of the scales."""
    return all(row[str(x)][k] for x in scales for k in ("flake_pass", "fire_pass"))


def sensitivity(tuning, pool):
    """How the pace moves when each tuned value alone is changed, over 40 runs and 40 worlds. A value holds the pace on
    a knife's edge where a change of a quarter either way (times 0.8 or 1.25) breaks a step's rule, inside its window
    in half the runs and early in at most a quarter; it is a strong lever where halving or doubling it does."""
    out = [{"knob": "as tuned", "1.0": summary(runs(tuning, SWEEP, pool=pool), worlds(tuning, SWEEP, pool=pool))}]
    for name, paths in KNOBS.items():
        row = {"knob": name}
        for scale in (0.8, 1.25, 0.5, 2.0):
            varied = copy.deepcopy(tuning)
            for path in paths:
                setting(varied, path, scale=scale)
            row[str(scale)] = summary(runs(varied, SWEEP, pool=pool), worlds(varied, SWEEP, pool=pool))

        row["edge"] = not holds(row, (0.8, 1.25))
        row["lever"] = not row["edge"] and not holds(row, (0.5, 2.0))
        out.append(row)
        low, high = row["0.5"], row["2.0"]
        print(
            f"  {name}: flakes {low['flake_median']:.2f} to {high['flake_median']:.2f} years, fire "
            f"{low['fire_median']:.1f} to {high['fire_median']:.1f} ({low['fire_never']} and {high['fire_never']} "
            f"never){' EDGE' if row['edge'] else (' lever' if row['lever'] else '')}",
            flush=True,
        )
    return out


def main():
    global _cache
    _cache = json.loads(CACHE.read_text()) if CACHE.exists() else {}
    tuning = d.load(TUNING)
    # three workers, leaving a core for the rest of the session's work
    with Pool(3) as pool:
        if "--tune" in sys.argv:
            tuning = tune(tuning, pool)
            write_factors(tuning)
        base = runs(tuning, SEEDS, pool=pool)
        control = runs(tuning, SEEDS, flint=0.0, pool=pool)
        fires = worlds(tuning, SEEDS, pool=pool)
        fresh = runs(tuning, FRESH, pool=pool)
        fresh_control = runs(tuning, FRESH, flint=0.0, pool=pool)
        fresh_fires = worlds(tuning, FRESH, pool=pool)
        print("Sensitivity, each value changed by a quarter, halved and doubled:", flush=True)
        sens = sensitivity(tuning, pool)
    stone = sharp_stone(base, control)
    stone["window"] = window([r["flake"] for r in base], FLAKE_WINDOW)
    fire = fire_window(fires)
    stone_fresh = sharp_stone(fresh, fresh_control)
    fire_fresh = fire_window(fresh_fires)
    edge = [row["knob"] for row in sens if row.get("edge")]
    levers = [row["knob"] for row in sens if row.get("lever")]
    report = {
        "title": "P4 Discovery pace",
        "question": f"Can tuning alone make sharp flakes come within {FLAKE_WINDOW[1]:g} years and fire in Years "
        f"{FIRE_WINDOW[0] + 1:g} to {FIRE_WINDOW[1]:g}, their windows, with the world's own rules?",
        "windows": {"flake": list(FLAKE_WINDOW), "fire": list(FIRE_WINDOW)},
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
        "levers": levers,
        "pass": stone["pass"]
        and stone["window"]["pass"]
        and fire["pass"]
        and stone_fresh["pass"]
        and fire_fresh["pass"]
        and not edge,
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=1) + "\n")
    print(
        f"Flakes: {stone['within_5']}/20 within 5 years (fresh seeds {stone_fresh['within_5']}/20), "
        f"{stone['window']['inside']}/20 in their window, Years 1-{FLAKE_WINDOW[1]:g}; spread in "
        f"{stone['spread_ok']}, routes {', '.join(stone['routes'])}, control flakes {stone['control_flakes']}"
    )
    print(
        f"Fire, first in a world: {fire['inside']}/20 in its window, Years {FIRE_WINDOW[0] + 1:g}-"
        f"{FIRE_WINDOW[1]:g}, {fire['early']} before (fresh {fire_fresh['inside']}/20, {fire_fresh['early']}), "
        f"ways {', '.join(fire['ways'])}"
    )
    print(f"Knife's edge: {', '.join(edge) or 'none'}; strong levers: {', '.join(levers) or 'none'}")
    print(f"P4: {'PASS' if report['pass'] else 'FAIL'}, report {REPORT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
