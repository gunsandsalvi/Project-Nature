"""P4 Discovery pace (IMPLEMENTATION α0.3a, research 00 and 11): one band of about 25 with simple minds, run headless
for game years by a river with flint among granite, dry and green wood, and fires from lightning.

They start with the starting kit alone (BIO-02, BIO-20): they crack nuts and bones with stones, butcher, make beds
and keep a fire they find, but make no flake and no fire. A sharp flake (RCK-01) and an ember by drilling or
ploughing (RCK-02) come only as MND-11 says: by accident, by experimenting, from a dream's hint, or by copying;
watching and teaching spread them (MND-13), skill grows with use (MND-06), and a blueprint dies with its last holder
(CUL-02). Every value that sets the pace is in tuning.toml (RES-16). Pre-production code (research 00): thrown away
once its answer is in the architecture (A12).
"""

import random
import tomllib
from pathlib import Path

HERE = Path(__file__).parent
YEAR = 60  # days in a game year (TIM-18)
SEASON = 15
SLOTS = 6  # activities in a waking day, about two hours each
ADULT = 14.0
FIRE = ("drill", "plough")
# MAT-06's 21 base actions; the familiar ones, done every day, come to mind three times as often
ACTIONS = (
    "gather", "dig", "strike", "press", "cut", "scrape", "grind", "twist", "bind", "weave", "shape",
    "drill", "heat", "soak", "dry", "mix", "stack", "plant", "throw", "feed", "apply",
)  # fmt: skip
FAMILIAR = ("gather", "strike", "cut", "stack", "feed")
# what lies in reach at camp, by how much of it there is
THINGS = (("stone", 3), ("wood", 2), ("grass", 2), ("nuts", 1), ("bone", 1), ("hide", 1), ("earth", 1), ("water", 1))
# what can warm, for experiments aimed at the cold (MND-11)
BURNABLE = (("wood", 1), ("grass", 1))
# skill from practice (MND-06): a newly learned blueprint at 1, level 5 after about 2 years of knapping most days and
# 10 after about 10; a lone try adds 1, a success 2
SKILL_A = 0.68
SKILL_K = 0.4
START = (1.0 / SKILL_A) ** (1.0 / SKILL_K)
# chances of dying in a game year by age (BIO-04's ranges, as foragers live), and of a birth to a woman of 16 to 40
DEATHS = ((1.0, 0.2), (5.0, 0.05), (14.0, 0.02), (45.0, 0.015), (60.0, 0.03), (999.0, 0.08))
BIRTH = 0.3
# how much each season asks for cracking nuts and bones, spring to winter
CRACK = (0.4, 0.3, 1.5, 1.2)


def load(path=HERE / "tuning.toml"):
    with open(path, "rb") as f:
        return tomllib.load(f)


def chance(level, difficulty):
    """A try's chance (MAT-04): one in two at a level equal to the difficulty, a tenth more or less for each level
    above or below, within 5% and 95%."""
    return min(0.95, max(0.05, 0.5 + 0.1 * (level - difficulty)))


def death_rate(age):
    """The chance of dying in a game year at an age."""
    return next(rate for limit, rate in DEATHS if age < limit)


def skill(practice):
    """A blueprint's skill from its practice (MND-06): 0 until learned, then rising ever more slowly to 10."""
    return 0.0 if practice <= 0.0 else min(10.0, SKILL_A * practice**SKILL_K)


class Person:
    __slots__ = (
        "id", "age", "female", "curious", "kind", "mother", "alive", "practice", "exp", "hunches",
        "watched", "how", "actions", "handled", "flint", "youngest",
    )  # fmt: skip

    def __init__(self, pid, age, female, curious, kind, mother, flint):
        self.id = pid
        self.age = age
        self.female = female
        self.curious = curious
        self.kind = kind
        self.mother = mother
        self.alive = True
        self.practice = {}  # blueprint: practice, for each one known (MND-06)
        older = max(0.0, (age - 20.0) // 15.0)
        # BIO-20: gathering 3, hunting 2 and fire 1, plus 1 in each for every 15 years over 20; children start at 0
        grown = age >= ADULT
        self.exp = {"gathering": 3.0 + older if grown else 0.0, "fire": 1.0 + older if grown else 0.0, "stone": 0.0}
        self.hunches = {}  # blueprint, or a dream's nothing: [source, failed tries, last day, weak]
        self.watched = {}  # blueprint: uses watched (MND-13)
        self.how = {}  # blueprint: (route, day)
        self.actions = {"gather", "strike", "cut", "stack", "feed"} if grown else {"gather"}
        self.handled = {"stone", "wood", "grass", "nuts", "bone"}
        self.flint = flint  # whether the stone they bash with flakes
        self.youngest = -999.0  # the day of a woman's last birth
        if flint:
            self.handled.add("flint")

    def knows(self, bp):
        return bp in self.practice

    def level(self, bp, sector):
        return (skill(self.practice.get(bp, 0.0)) + self.exp.get(sector, 0.0)) / 2.0


class Band:
    """One band in one run: its people, its fire, and what it has found."""

    def __init__(self, seed, tuning, flint=None, fire=False):
        self.rng = random.Random(seed)
        self.t = tuning
        self.scene = dict(tuning["scene"])
        if flint is not None:
            self.scene["flint"] = flint
        self.day = 0
        self.fire = fire  # BIO-02: one band of a world starts with a fire taken from lightning
        self.people = []
        self.first = {}  # blueprint: (day, route)
        self.lost = []  # (day, blueprint) whenever the last holder dies
        self.known_by = {bp: 0 for bp in tuning["blueprints"]}
        self.yearly = []  # for each year: how many know each blueprint, and how many live
        rng = self.rng
        # BIO-03 and BIO-04: about 25, children, adults and a few old
        ages = [rng.uniform(0, 14) for _ in range(11)] + [rng.uniform(14, 45) for _ in range(11)]
        ages += [rng.uniform(45, 65) for _ in range(3)]
        for age in ages:
            self._born(age, None)
        # the camp's anvils, flaking ones rarer as big stones
        self.anvils = [rng.random() < self.scene["flint"] * 0.5 for _ in range(4)]

    def _born(self, age, mother):
        rng = self.rng
        curious = min(1.0, max(0.0, rng.gauss(0.5, 0.2)))
        p = Person(len(self.people), age, rng.random() < 0.5, curious, rng.random(), mother, False)
        p.flint = rng.random() < self.scene["flint"]
        if p.flint:
            p.handled.add("flint")
        self.people.append(p)
        return p

    # --- what people find and learn -------------------------------------------------------------------------------

    def notice(self, p, busy):
        """MND-10: whether a surprise is noticed, by curiosity, half as often when busy."""
        m = self.t["minds"]
        n = m["notice_least"] + (m["notice_most"] - m["notice_least"]) * p.curious
        return self.rng.random() < (n * 0.5 if busy else n)

    def learn(self, p, bp, route):
        """A blueprint learned at skill 1 (MND-11, MND-13), the band's first noted with its route."""
        if p.knows(bp):
            return
        p.practice[bp] = START
        p.how[bp] = (route, self.day)
        p.hunches.pop(bp, None)
        self.known_by[bp] += 1
        if bp not in self.first:
            self.first[bp] = (self.day, route)

    def hunch(self, p, bp, source, weak=False):
        """A hunch (MND-11): held among at most a few, the oldest dropped for a new one."""
        if bp is not None and (p.knows(bp) or bp in p.hunches):
            return
        held = p.hunches
        if len(held) >= self.t["minds"]["hunches_held"]:
            held.pop(next(iter(held)))
        key = bp if bp is not None else ("nothing", self.day, self.rng.random())
        held[key] = [source, 0, self.day, weak]

    def roll(self, p, bp, factor, busy, source, hints=True):
        """One activity that fits a blueprint p doesn't know (MND-11): the chance a maker at p's level would have,
        times the route's and the blueprint's factors, rolled once; a success, if noticed, teaches it; a failure may
        show the blueprint's hint (MAT-04), which, if noticed, gives a hunch."""
        b = self.t["blueprints"][bp]
        c = chance(p.level(bp, b["sector"]), b["difficulty"]) * factor * b["factor"]
        if self.rng.random() < c:
            if self.notice(p, busy):
                self.learn(p, bp, source)
                return True
        elif hints and self.rng.random() < b["hint"] and self.notice(p, busy):
            self.hunch(p, bp, source)
        return False

    def strike(self, p, flaking, busy, factor, source):
        """A strike with or on stone: it fits the flake blueprint only when a stone that flakes is struck (RCK-01)."""
        p.actions.add("strike")  # MAT-06: an action done once, in any way, is known
        if flaking and not p.knows("flake"):
            p.handled.add("flint")
            self.roll(p, "flake", factor, busy, source)

    def rub(self, p, bp, season, busy, factor, source):
        """A stick drilled or ploughed against wood: it fits a fire blueprint only when both are dry (RCK-02)."""
        p.actions.add(self.t["blueprints"][bp]["action"])
        dry = self.scene["dry"][season]
        if self.rng.random() < dry * dry and not p.knows(bp):
            self.roll(p, bp, factor, busy, source)

    # --- a day -----------------------------------------------------------------------------------------------------

    def run(self, years, stop=None):
        """Runs day by day for years, or until stop(band) says so."""
        while self.day < years * YEAR:
            self.step()
            self.day += 1
            if self.day % YEAR == 0:
                live = [p for p in self.people if p.alive]
                self.yearly.append({bp: n for bp, n in self.known_by.items()} | {"people": len(live)})
            if stop is not None and stop(self):
                break
        return self

    def step(self):
        rng = self.rng
        t = self.t
        m = t["minds"]
        season = (self.day % YEAR) // SEASON
        live = [p for p in self.people if p.alive]
        adults = [p for p in live if p.age >= ADULT]
        # the fire: lost to rain or neglect, or taken from a wildfire in the dry seasons (WLD-28, BIO-02)
        if self.fire and rng.random() < self.scene["fire_lost"]:
            self.fire = False
        if not self.fire and season in (1, 2) and rng.random() < self.scene["lightning"]:
            self.fire = True
        if not self.fire:
            for p in adults:
                if any(p.knows(bp) for bp in FIRE):
                    self.make_fire(p, live)
                    if self.fire:
                        break
        cold = season == 3 or (season in (0, 2) and rng.random() < 0.3)
        freezing = cold and not self.fire
        carcass = rng.random() < self.scene["carcass"]
        # who experiments today (MND-11): by curiosity in good times, and aimed at the cold when it has no answer
        tries = {}
        good = (0.5 if season == 3 else 0.8) if not freezing else 0.0
        no_answer = freezing and not any(p.knows(bp) for p in adults for bp in FIRE)
        for p in adults:
            rate = m["experiment_average"] * (m["experiment_curious"] / m["experiment_average"]) ** (2 * p.curious - 1)
            if rng.random() < good and rng.random() < rate:
                tries[p.id] = (rng.randrange(SLOTS), False)
            elif no_answer and rng.random() < min(1.0, 2.0 * rate):
                tries[p.id] = (rng.randrange(SLOTS), True)
        flakes_around = self.known_by["flake"] > 0
        for slot in range(SLOTS):
            uses = []  # (person, blueprint) for each use of a craft others can watch
            doing = {}
            for p in rng.sample(live, len(live)):
                if p.age < 5:
                    continue
                if p.id in tries and tries[p.id][0] == slot:
                    self.experiment(p, season, tries[p.id][1])
                    doing[p.id] = "experiment"
                    continue
                act = self.choose(p, slot, season, carcass)
                doing[p.id] = act
                if act == "crack":
                    if rng.random() < 0.05:
                        p.flint = rng.random() < self.scene["flint"]
                    anvil = self.anvils[rng.randrange(len(self.anvils))]
                    self.strike(p, p.flint or anvil, True, t["routes"]["accident"], "accident")
                    p.exp["gathering"] = min(10.0, p.exp["gathering"] + 0.002)
                elif act == "play" and rng.random() < 0.3:
                    # children copy adults in play (MND-21): bashing stones as their elders crack nuts
                    self.strike(p, rng.random() < self.scene["flint"], False, t["routes"]["accident"], "accident")
                elif act == "knap":
                    self.use(p, "flake")
                    uses.append((p, "flake"))
            for p, bp in uses:
                self.watch_and_teach(p, bp, live, doing)
            if flakes_around:
                # MND-11's copying: a flake lying about shows how it was made, giving a weak hunch
                for p in live:
                    if p.age >= 5 and not p.knows("flake") and rng.random() < 0.01:
                        self.hunch(p, "flake", "copying", weak=True)
        self.night(live, freezing)
        self.age_and_births(live)

    def choose(self, p, slot, season, carcass):
        """Simple choice (MND-09): what serves the day's needs, weighted, drawn by chance."""
        if p.age < ADULT:
            # children copy adults in play (MND-21), knapping too once they can
            options = (("gather", 1.0), ("play", 2.0), ("rest", 1.0), ("knap", 0.5 if p.knows("flake") else 0.0))
        else:
            knap = (0.8 if p.knows("flake") else 0.0) * (2.0 if carcass else 1.0)
            options = (
                ("gather", 2.5 if slot < 3 else 1.0),
                ("crack", CRACK[season] + (1.0 if carcass else 0.0)),
                ("butcher", 1.0 if carcass and slot < 4 else 0.0),
                ("rest", 1.5),
                ("tend", 0.4 if self.fire else 0.0),
                ("knap", knap),
            )
        total = sum(w for _, w in options)
        r = self.rng.random() * total
        for act, w in options:
            r -= w
            if r < 0.0:
                return act
        return options[0][0]

    def experiment(self, p, season, aimed):
        """One try (MND-11): a hunch if one is held; else a known action on things like what it works on; else any
        base action on one or two things in reach; aimed at the cold, on things that burn."""
        rng = self.rng
        routes = self.t["routes"]
        m = self.t["minds"]
        for key in [k for k, h in p.hunches.items() if self.day - h[2] > m["hunch_days"]]:
            del p.hunches[key]
        if p.hunches:
            key = next(reversed(p.hunches))
            h = p.hunches[key]
            h[2] = self.day
            found = False
            if key == "flake":
                found = self.scene["flint"] > 0.0 and rng.random() < 0.9
                if found:
                    found = self.roll(p, "flake", routes["experiment" if h[3] else "hunch"], False, h[0], False)
            elif key in FIRE:
                p.actions.add(self.t["blueprints"][key]["action"])
                if rng.random() < self.scene["dry"][season]:
                    found = self.roll(p, key, routes["experiment" if h[3] else "hunch"], False, h[0], False)
            if not found and key in p.hunches:
                h[1] += 1
                if h[1] >= m["hunch_tries"]:
                    del p.hunches[key]
            return
        if aimed:
            things = [self.pick(BURNABLE), self.pick(BURNABLE)]
            action = self.pick_action(p, False)
        elif rng.random() < 0.5:
            action = rng.choice(sorted(p.actions))
            things = {"strike": ["stone", "stone"], "drill": ["wood", "wood"], "grind": ["wood", "wood"]}.get(
                action, []
            )
        else:
            action = self.pick_action(p, False)
            things = [self.pick(THINGS) for _ in range(rng.randint(1, 2))]
        p.actions.add(action)
        if action == "strike" and things.count("stone") >= 1:
            flaking = any(rng.random() < self.scene["flint"] for _ in range(things.count("stone")))
            self.strike(p, flaking, False, routes["experiment"], "experiment")
        elif action in ("drill", "grind") and things.count("wood") == 2:
            self.rub(p, "drill" if action == "drill" else "plough", season, False, routes["experiment"], "experiment")

    def pick(self, weighted):
        total = sum(w for _, w in weighted)
        r = self.rng.random() * total
        for thing, w in weighted:
            r -= w
            if r < 0.0:
                return thing
        return weighted[-1][0]

    def pick_action(self, p, known_only):
        """Any base action, the familiar ones three times as likely (MND-11)."""
        weighted = [(a, 3.0 if a in FAMILIAR else 1.0) for a in ACTIONS if not known_only or a in p.actions]
        return self.pick(weighted)

    def use(self, p, bp):
        """A known blueprint used: its skill and its sector's experience grow (MND-06)."""
        b = self.t["blueprints"][bp]
        ok = self.rng.random() < chance(p.level(bp, b["sector"]), b["difficulty"])
        grow = (2.0 if ok else 1.0) * (1.5 if p.age < ADULT else 1.0)
        p.practice[bp] += grow
        p.exp[b["sector"]] = min(10.0, p.exp.get(b["sector"], 0.0) + 0.01 * grow)
        return ok

    def make_fire(self, p, live):
        """Fire made by one who knows how, when the band has none."""
        for bp in FIRE:
            if p.knows(bp):
                if self.use(p, bp):
                    self.fire = True
                self.watch_and_teach(p, bp, live, {})
                return

    def watch_and_teach(self, p, bp, live, doing):
        """MND-13: those near who don't know it watch, children and the curious most, a quarter as much while busy;
        about five watched uses teach it. One who knows and is kind may teach someone beside them, kin first: the
        learner tries at their own level's full chance, and their first success makes it theirs."""
        rng = self.rng
        m = self.t["minds"]
        b = self.t["blueprints"][bp]
        learners = []
        for q in live:
            if q is p or q.age < 3 or q.knows(bp) or rng.random() > 0.6:
                continue
            learners.append(q)
            free = doing.get(q.id, "rest") in ("rest", "play")
            kin = p.id == q.mother or p.mother == q.id
            want = (0.5 if q.age < ADULT else 0.1 + 0.3 * q.curious) * (1.5 if kin else 1.0)
            seen = 1.0 if free and rng.random() < want else (0.25 if rng.random() < 0.3 else 0.0)
            if seen > 0.0:
                q.watched[bp] = q.watched.get(bp, 0.0) + seen
                if seen == 1.0:
                    self.hunch(q, bp, "copying")
                if q.watched[bp] >= m["watch_uses"]:
                    self.learn(q, bp, "watching")
        if learners and rng.random() < 0.3 * p.kind:
            kin = [q for q in learners if q.mother == p.id or p.mother == q.id]
            q = rng.choice(kin or learners)
            if rng.random() < chance(q.level(bp, b["sector"]), b["difficulty"]):
                self.learn(q, bp, "taught")
                q.practice[bp] += m["taught"]

    def night(self, live, freezing):
        """Dreams (MND-12): about 1 in 60, 1 in 20 while a need is below 20, join memories into a hunch, 1 time in 3
        for a real blueprint they don't know whose action they know and whose things they have handled, in their
        most experienced sector, then the one they'd likeliest succeed at; otherwise for nothing."""
        rng = self.rng
        m = self.t["minds"]
        blueprints = self.t["blueprints"]
        for p in live:
            if p.age < 5 or rng.random() >= (m["dream_hint_need"] if freezing else m["dream_hint"]):
                continue
            if rng.random() >= m["dream_real"]:
                self.hunch(p, None, "dream")
                continue
            fit = []
            for bp, b in blueprints.items():
                needs = "flint" if bp == "flake" else "wood"
                if not p.knows(bp) and b["action"] in p.actions and needs in p.handled:
                    level = p.level(bp, b["sector"])
                    fit.append((p.exp.get(b["sector"], 0.0), chance(level, b["difficulty"]), bp))
            if fit:
                self.hunch(p, max(fit)[2], "dream")

    def age_and_births(self, live):
        """A day of life (BIO-04): everyone a day older, deaths by age, births to women of 16 to 40 whose youngest
        is past two and a half; a blueprint dies with its last holder (CUL-02)."""
        rng = self.rng
        for p in live:
            p.age += 1.0 / YEAR
            if rng.random() < death_rate(p.age) / YEAR:
                self.die(p)
                continue
            if p.female and 16.0 <= p.age <= 40.0 and self.day - p.youngest > 2.5 * YEAR:
                if rng.random() < BIRTH / YEAR:
                    p.youngest = self.day
                    self._born(0.0, p.id)

    def die(self, p):
        """A death: what only they knew dies with them (CUL-02)."""
        p.alive = False
        for bp in p.practice:
            self.known_by[bp] -= 1
            if self.known_by[bp] == 0:
                self.lost.append((self.day, bp))


def first_year(band, bp):
    """The year a blueprint was first found, counting from Year 1 (TIM-14), or None."""
    return band.first[bp][0] / YEAR if bp in band.first else None


def fire_first(band):
    """The first ember by drilling or ploughing (MAT-23): its year, blueprint and route, or None."""
    found = [(band.first[bp][0], bp, band.first[bp][1]) for bp in FIRE if bp in band.first]
    return min(found) if found else None


def adults_who_know(band, bp):
    """The share of the band's adults who know a blueprint."""
    adults = [p for p in band.people if p.alive and p.age >= ADULT]
    return sum(1 for p in adults if p.knows(bp)) / max(1, len(adults))
