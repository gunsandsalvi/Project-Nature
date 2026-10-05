"""P11's runs (IMPLEMENTATION α0.6c, TIM-02, TIM-03, PRE-39): 20 test worlds, each a valley of P10's three bands
with P4's discovery in three bands of their own beside them, run 100 years and watched from the globe at top speed
by the director; the same worlds run with it on and off; a code check that nothing runs from it into a world; and
the ages its turning points begin. Writes the report the app's Reports page draws.

    python3 prototypes/director/watch.py    # about seven minutes on three cores

Pre-production code (research 00): thrown away once its answer is in the architecture (A14).
"""

import ast
import hashlib
import json
import pickle
import random
import statistics
import sys
from multiprocessing import Pool
from pathlib import Path

import director as dr

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "prototypes" / "discovery"))
sys.path.insert(0, str(ROOT / "prototypes" / "culture"))
import culture as c  # noqa: E402
import discovery as d  # noqa: E402

REPORT = ROOT / "prototypes" / "app" / "reports" / "p11.json"
SEEDS = list(range(1, 21))
YEARS = 100
# P4's bands: each its own people, out of reach of the others and of the valley's, as P4 ran them
FLINT_BANDS = ("Bear", "Stag", "Raven")
# the ways of watching: from the globe, never tapping (TIM-02's done-when); tapping every live moment; and signs scored
# as their end, not as how often it came
WAYS = {"never taps": (False, "expected"), "taps every one": (True, "expected"), "signs at full score": (False, "end")}
LEAST = (20, 10, 5, 3, 1)  # PRE-39's least years for an age, as it stands and shorter
PERSON_WHAT = ("birth", "married", "fight")
BAND_WHAT = ("split", "joined")


class Valley:
    """A test world: P10's bands, and P4's three beside them, never touching, one day at a time."""

    def __init__(self, seed):
        self.seed = seed
        self.culture = c.World(seed, c.load())
        self.flint = [d.Band(seed * 10 + k, d.load(), fire=k == 0) for k in range(len(FLINT_BANDS))]
        self.handed = [0] * (1 + len(self.flint))  # events handed over so far, from each part
        self.told = {}  # each of P10's events: its words, for the story

    def advance(self):
        self.culture.advance()
        for band in self.flint:
            band.advance()

    def run(self, years):
        self.culture.run(years)
        for band in self.flint:
            band.run(years)

    def label(self, bid):
        """A P10 band's name, numbered once the names come round again."""
        name = self.culture.bands[bid].name
        return name if bid < len(c.BANDS) else f"{name} {bid // len(c.BANDS) + 1}"

    def handover(self):
        """The events since the last handover, as plain tuples in the director's form, by their time."""
        out = []
        w = self.culture
        for day, bid, text, _, kind, who, what, hour in w.events[self.handed[0] :]:
            if kind in PERSON_WHAT:
                what = f"v{what}"
            elif kind in BAND_WHAT:
                what = self.label(what)
            ev = (day, hour, "the valley", self.label(bid), kind, f"v{who}" if who >= 0 else "", str(what), "")
            self.told[ev] = text
            out.append(ev)
        self.handed[0] = len(w.events)
        for k, band in enumerate(self.flint):
            name = FLINT_BANDS[k]
            for day, hour, kind, who, what, how in band.events[self.handed[k + 1] :]:
                what = f"b{k}.{what}" if kind == "birth" else str(what)
                out.append((day, hour, name, name, kind, f"b{k}.{who}" if who >= 0 else "", what, how))
            self.handed[k + 1] = len(band.events)
        assert all(plain(ev) for ev in out)
        out.sort(key=dr.time_of)
        return out

    def followed(self):
        """Three people you follow from the start: the youngest adult of two of the valley's bands and of the Bear."""
        w = self.culture
        out = []
        for band in w.bands[:2]:
            out.append(f"v{min(w.adults(band), key=lambda p: (p.age, p.id)).id}")
        bear = [p for p in self.flint[0].people if p.alive and p.age >= d.ADULT]
        out.append(f"b0.{min(bear, key=lambda p: (p.age, p.id)).id}")
        return out

    def name(self, key):
        """A person's name: the valley's own, or one for P4's people from the same syllables, by a chance of its own
        that no world draws from."""
        if key.startswith("v"):
            return self.culture.people[int(key[1:])].name
        rng = random.Random(f"{self.seed}-{key}")
        return "".join(rng.choice(c.SYLLABLES) for _ in range(rng.randint(2, 3))).capitalize()

    def digest(self):
        """Everything the worlds hold, their chance's state too, as one number."""
        return hashlib.sha256(pickle.dumps((self.culture, self.flint))).hexdigest()


def plain(ev):
    """An event the director may see: a tuple of plain values, holding nothing of a world."""
    return isinstance(ev, tuple) and len(ev) == 8 and all(type(x) in (int, float, str) for x in ev)


def live(valley, days, meddle=None):
    """The events of a world run live, a day at a time, handed over as each day ends; meddle, if given, is called each
    day, as a careless host might, to show what the check catches."""
    for _ in range(days):
        valley.advance()
        if meddle is not None:
            meddle()
        yield from valley.handover()


def careless(valley, director):
    """A careless host, for the control: once the director has slowed time, it picks someone to look at each day with
    the world's own chance, a path from the director into the world that the comparison must catch. One stray draw
    alone can be swallowed by the world's one stream of chance and leave no trace, so it draws every day."""

    def meddle():
        if director.slowdowns:
            valley.culture.rng.random()

    return meddle


def hands_off():
    """TIM-03's and PRE-39's code check: the director imports nothing but the standard library's tomllib and pathlib,
    and the worlds import nothing of it, so no path runs from it or its recognisers into a world."""

    def imports(path):
        tree = ast.parse(path.read_text())
        names = {a.name.split(".")[0] for n in ast.walk(tree) if isinstance(n, ast.Import) for a in n.names}
        return names | {n.module.split(".")[0] for n in ast.walk(tree) if isinstance(n, ast.ImportFrom)}

    worlds = (ROOT / "prototypes" / "discovery" / "discovery.py", ROOT / "prototypes" / "culture" / "culture.py")
    back = any(name in ("director", "watch") for path in worlds for name in imports(path))
    return imports(dr.HERE / "director.py") <= {"tomllib", "pathlib"} and not back


def truth(valley, followed):
    """What the worlds themselves recorded, to check the recognisers against: each people's named discoveries,
    rediscoveries and lost crafts (P4), the valley's firsts (P10), and those followed who died."""
    found, again, lost = set(), set(), set()
    for k, band in enumerate(valley.flint):
        name = FLINT_BANDS[k]
        found |= {(name, bp, day) for bp, (day, _) in band.first.items()}
        again |= {(name, bp, day) for day, bp, _ in band.regained}
        lost |= {(name, bp, day) for day, bp in band.lost}
    firsts = {kind: f["day"] for kind, f in valley.culture.firsts.items()}
    dead = set()
    for key in followed:
        if key.startswith("v"):
            alive = valley.culture.people[int(key[1:])].alive
        else:
            alive = valley.flint[int(key[1 : key.index(".")])].people[int(key[key.index(".") + 1 :])].alive
        if not alive:
            dead.add(key)
    return {"found": found, "again": again, "lost": lost, "firsts": firsts, "dead": dead}


def recognised(rec, real):
    """What the recognisers found, in the same terms as the worlds' own records."""
    by = {}
    for f in rec.found:
        by.setdefault(f.kind, []).append(f)
    found = {
        (f.ev[dr.PEOPLE], f.what, f.ev[dr.DAY])
        for k in ("world first discovery", "named discovery")
        for f in by.get(k, [])
    }
    firsts = {}
    for f in by.get("world first", []):
        kind = f.ev[dr.KIND]
        firsts[kind] = min(firsts.get(kind, f.ev[dr.DAY]), f.ev[dr.DAY])
    return {
        "found": found,
        "again": {(f.ev[dr.PEOPLE], f.what, f.ev[dr.DAY]) for f in by.get("rediscovery", [])},
        "lost": {(f.ev[dr.PEOPLE], f.what, f.ev[dr.DAY]) for f in by.get("craft lost", [])},
        "firsts": {k: v for k, v in firsts.items() if k in real["firsts"]},
        "dead": {f.ev[dr.WHO] for f in by.get("death of one you follow", [])},
    }


def judge(rec, director, real_s, top, budget):
    """A watch against the budget: its slowdowns, the shortest gap between them, the most slowed in any hour (or the
    whole watch, if shorter), the share of top speed kept; and each kind of moment found, slowed for and listed."""
    slow = director.slowdowns
    starts = [s for s, _, _, _ in slow]
    gaps = [b - a for a, b in zip(starts, starts[1:], strict=False)]
    span = min(budget["window"], real_s)
    worst = max((director.slowed(s, s + span) / span for s in starts), default=0.0)
    game = YEARS * dr.YEAR
    slowed_ids = {id(f) for _, _, _, f in slow}
    listed_ids = {id(f) for _, f in director.listed}
    kinds = {}
    for f in rec.found:
        row = kinds.setdefault(f.kind, [0, 0, 0])
        row[0] += 1
        row[1] += id(f) in slowed_ids
        row[2] += id(f) in listed_ids
    signs = {}
    for name, _, came in rec.signs:
        row = signs.setdefault(name, [0, 0, 0, 0])
        row[0] += 1
        row[1] += came
    for _, _, _, f in slow:
        if f.sign is not None:
            row = signs[f.kind]
            row[2] += 1
            row[3] += f.record[2]
    discoveries = [f for f in rec.found if f.kind in ("world first discovery", "named discovery", "rediscovery")]
    foretold = 0
    for f in discoveries:
        t = dr.time_of(f.ev)
        for _, _, _, g in slow:
            if (
                g.sign is not None
                and g.ev[dr.WHO] == f.ev[dr.WHO]
                and g.what == f.what
                and 0 <= t - dr.time_of(g.ev) <= 0.25
            ):
                foretold += 1
                break
    return {
        "minutes": round(real_s / 60.0, 2),
        "slowdowns": len(slow),
        "per_hour": round(len(slow) * 3600.0 / real_s, 2),
        "least_gap": round(min(gaps), 1) if gaps else None,
        "worst_share": round(worst, 4),
        "kept": round(game / (top * real_s), 4),
        "listed": len(director.listed),
        "listed_per_hour": round(len(director.listed) * 3600.0 / real_s, 1),
        # listed in the first 4 minutes, a world's first 20 years, when everything is a first
        "listed_early": sum(1 for r, _ in director.listed if r < 240.0),
        "kinds": kinds,
        "signs": signs,
        "discoveries": len(discoveries),
        "discoveries_caught": sum(1 for f in discoveries if id(f) in slowed_ids or id(f) in listed_ids),
        "discoveries_slowed": sum(1 for f in discoveries if id(f) in slowed_ids),
        "foretold": foretold,
        "sign_slowdowns": sum(1 for _, _, _, f in slow if f.sign is not None),
    }


def one(seed):
    """One world: run with the director on, live, and off, and compared; then watched again from its record in each
    way; its ages; and the first world's story and timeline."""
    tuning = dr.load()
    top = tuning["speed"]["top"] * dr.YEAR / 60.0  # game days a real second
    days = YEARS * dr.YEAR
    on = Valley(seed)
    followed = on.followed()
    rec = dr.Recognisers(tuning, followed)
    director = dr.Director(tuning, top)
    record = []

    def kept(events):
        for ev in events:
            record.append(ev)
            yield ev

    real_s = dr.watch(kept(live(on, days)), rec, director, days)
    off = Valley(seed)
    off.run(YEARS)
    same = on.digest() == off.digest() and record == off.handover()
    real = truth(off, followed)
    ways = {}
    for way, (taps, signs) in WAYS.items():
        r2 = dr.Recognisers(tuning, followed, signs)
        d2 = dr.Director(tuning, top, taps)
        s2 = dr.watch(iter(record), r2, d2, days)
        ways[way] = judge(r2, d2, s2, top, tuning["budget"])
        if way == "never taps":
            # the director live and from the record do the same
            same = same and [(s, f.kind) for s, _, _, f in d2.slowdowns] == [
                (s, f.kind) for s, _, _, f in director.slowdowns
            ]
    found = recognised(rec, real)
    checks = {k: found[k] == real[k] for k in real}
    turning = [(round(t / dr.YEAR, 2), step) for t, step, _ in rec.turning]
    out = {
        "seed": seed,
        "identical": same,
        "recognised": checks,
        "truth": {k: len(v) for k, v in real.items()},
        "ways": ways,
        "turning": turning,
        "ages": {str(n): [name for _, name in dr.ages(rec.turning, n)] for n in LEAST},
        "people": sum(1 for p in off.culture.people if p.alive)
        + sum(sum(1 for p in b.people if p.alive) for b in off.flint),
    }
    if seed in SEEDS[:3]:
        out["timeline"] = {
            "slowed": [[round(s / 60.0, 2), f.kind] for s, _, _, f in director.slowdowns],
            "listed": [round(r / 60.0, 2) for r, _ in director.listed],
            "minutes": round(real_s / 60.0, 2),
        }
    if seed == SEEDS[0]:
        out["story"] = story(off, director, real_s)
        meddled = Valley(seed)
        d3 = dr.Director(tuning, top)
        dr.watch(live(meddled, days, careless(meddled, d3)), dr.Recognisers(tuning, followed), d3, days)
        out["meddling_caught"] = meddled.digest() != off.digest()
    return out


def story(valley, director, real_s):
    """The first world's watch as lines: each slowdown, when it came in real minutes and in the world's years, and what
    it was for, in the world's words where it has them."""
    lines = []
    for s, _, _, f in director.slowdowns:
        ev = f.ev
        when = f"{s / 60.0:.1f} min, {c.when(ev[dr.DAY])}"
        lines.append(f"{when}: {f.kind}: {words(valley, f)}")
    lines.append(f"{len(director.listed)} more moments waited in the list over the {real_s / 60.0:.0f} minutes.")
    return lines


def words(valley, f):
    """What a finding was, in a sentence: the valley's own words, or one made from the event's record."""
    ev = f.ev
    if ev in valley.told:
        return valley.told[ev]
    who = valley.name(ev[dr.WHO]) if ev[dr.WHO] else ""
    band = f"of the {ev[dr.BAND]} band"
    if f.kind == "craft lost":
        return f"{who} {band} died, the last who could make {THINGS.get(f.what, f.what)}"
    again = ", found again" if f.kind == "rediscovery" else ""
    said = {
        "learn": f"{who} {band} made {THINGS.get(f.what, f.what)}, {ROUTES.get(ev[dr.HOW], ev[dr.HOW])}{again}",
        "death": f"{who} {band} died, {ev[dr.HOW]}",
        "try": f"{who} {band} tried a hunch for {THINGS.get(f.what, f.what)} again",
        "fire out": f"the {ev[dr.BAND]} band's fire went out, with no one who can make one",
        "birth": f"{valley.name(ev[dr.WHAT])} {band} bore a child",
    }
    return said.get(ev[dr.KIND], f"{ev[dr.KIND]} {band}")


THINGS = {"flake": "a sharp flake", "drill": "fire by drilling", "plough": "fire by ploughing"}
ROUTES = {
    "accident": "by accident",
    "experiment": "by experimenting",
    "hunch": "from a hunch",
    "dream": "from a dream's hunch",
    "copying": "by copying",
    "watching": "by watching",
    "taught": "taught",
}


def summary(runs, way, budget):
    """A way of watching over all the worlds: the budget's worst, and every kind of moment and sign summed."""
    rows = [r["ways"][way] for r in runs]
    kinds, signs = {}, {}
    for row in rows:
        for k, v in row["kinds"].items():
            kinds[k] = [a + b for a, b in zip(kinds.get(k, [0, 0, 0]), v, strict=True)]
        for k, v in row["signs"].items():
            signs[k] = [a + b for a, b in zip(signs.get(k, [0, 0, 0, 0]), v, strict=True)]
    gaps = [r["least_gap"] for r in rows if r["least_gap"] is not None]
    held = all(g >= budget["gap"] - 1e-6 for g in gaps) and all(
        r["worst_share"] <= budget["share"] + 1e-9 for r in rows
    )
    return {
        "hours": round(sum(r["minutes"] for r in rows) / 60.0, 1),
        "slowdowns": sum(r["slowdowns"] for r in rows),
        "per_hour": round(statistics.mean(r["per_hour"] for r in rows), 2),
        "least_gap": round(min(gaps), 1) if gaps else None,
        "worst_share": max(r["worst_share"] for r in rows),
        "least_kept": min(r["kept"] for r in rows),
        "listed_per_hour": round(statistics.mean(r["listed_per_hour"] for r in rows), 1),
        "listed_early": round(sum(r["listed_early"] for r in rows) / max(1, sum(r["listed"] for r in rows)), 3),
        "discoveries": sum(r["discoveries"] for r in rows),
        "discoveries_caught": sum(r["discoveries_caught"] for r in rows),
        "discoveries_slowed": sum(r["discoveries_slowed"] for r in rows),
        "foretold": sum(r["foretold"] for r in rows),
        "sign_slowdowns": sum(r["sign_slowdowns"] for r in rows),
        "kinds": kinds,
        "signs": {k: v + [round(v[1] / v[0], 4) if v[0] else None] for k, v in signs.items()},
        "held": held and min(r["kept"] for r in rows) >= 1.0 - budget["share"],
    }


def report(runs):
    tuning = dr.load()
    budget = tuning["budget"]
    ways = {way: summary(runs, way, budget) for way in WAYS}
    main = ways["never taps"]
    recognised_all = all(all(r["recognised"].values()) for r in runs)
    identical = sum(1 for r in runs if r["identical"])
    first = runs[0]
    fire_ages = {str(n): sum(1 for r in runs if any("fire" in a for a in r["ages"][str(n)])) for n in LEAST}
    fires = sum(1 for r in runs if any(step == "making fire" for _, step in r["turning"]))
    # the worlds whose first fire came inside TIM-19's window, Years 2 to 8, from 1 year in up to 8
    on_time = [r for r in runs if any(step == "making fire" and 1.0 <= t < 8.0 for t, step in r["turning"])]
    fire_ages_on_time = {str(n): sum(1 for r in on_time if any("fire" in a for a in r["ages"][str(n)])) for n in LEAST}
    caught_all = all(w["discoveries_caught"] == w["discoveries"] for w in ways.values())
    passed = (
        all(w["held"] for w in ways.values())
        and caught_all
        and recognised_all
        and identical == len(runs)
        and first["meddling_caught"]
        and hands_off()
    )
    return {
        "title": "P11 The director",
        "question": (
            "Does the director keep its budget on recorded worlds while catching every named discovery, without "
            "changing them?"
        ),
        "years": YEARS,
        "top": tuning["speed"]["top"],
        "budget": budget,
        "bar": tuning["bar"]["globe"],
        "ways": ways,
        "recognised": recognised_all,
        "identical": identical,
        "meddling_caught": first["meddling_caught"],
        "hands_off": hands_off(),
        "fire_worlds": fires,
        "fire_ages": fire_ages,
        "fire_on_time": len(on_time),
        "fire_ages_on_time": fire_ages_on_time,
        "least": list(LEAST),
        "runs": [
            {
                "seed": r["seed"],
                "per_hour": r["ways"]["never taps"]["per_hour"],
                "worst_share": r["ways"]["never taps"]["worst_share"],
                "kept": r["ways"]["never taps"]["kept"],
                "listed_per_hour": r["ways"]["never taps"]["listed_per_hour"],
                "discoveries": r["ways"]["never taps"]["discoveries"],
                "caught": r["ways"]["never taps"]["discoveries_caught"],
                "identical": r["identical"],
                "recognised": all(r["recognised"].values()),
                "turning": r["turning"],
                "people": r["people"],
            }
            for r in runs
        ],
        "timelines": [dict(r["timeline"], seed=r["seed"]) for r in runs if "timeline" in r],
        "story": first["story"],
        "pass": passed and main["discoveries"] > 0,
    }


if __name__ == "__main__":
    # three workers, leaving a core for the rest of the session's work
    with Pool(3) as pool:
        runs = pool.map(one, SEEDS)
    r = report(runs)
    REPORT.write_text(json.dumps(r, indent=1) + "\n")
    for way, w in r["ways"].items():
        print(
            f"{way}: {w['hours']} hours, {w['per_hour']} slowdowns an hour, least gap {w['least_gap']} s, worst "
            f"share slowed {w['worst_share']}, least speed kept {w['least_kept']}, {w['listed_per_hour']} listed an "
            f"hour; discoveries {w['discoveries_caught']}/{w['discoveries']} caught, {w['discoveries_slowed']} slowed "
            f"for, {w['foretold']} foretold; sign slowdowns {w['sign_slowdowns']}; held {w['held']}"
        )
        print("  kinds:", w["kinds"])
        print("  signs:", w["signs"])
    print(
        f"recognised {r['recognised']}, identical {r['identical']}/20, meddling caught {r['meddling_caught']}, "
        f"hands off {r['hands_off']}, fire ages {r['fire_ages']} of {r['fire_worlds']} worlds with fire, "
        f"{r['fire_ages_on_time']} of {r['fire_on_time']} with fire on time"
    )
    print("PASS" if r["pass"] else "FAIL", REPORT)
