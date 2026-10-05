"""P10 Culture from causes (IMPLEMENTATION α0.6b, research 12): three bands of simple minds, run headless for a
hundred years, to see whether customs, a spirit, a rite and a band split arise inside CUL-33's windows, each from its
own cause.

People live in families within bands (CUL-30): they hunt, gather and eat, are born, marry, fall ill and die, meet
storms and the camp's places, and talk each evening. After a strong outcome, each who meets it links it to the most
unusual thing of the day or two before or, for a blow from the sky or a sudden death, sometimes to an unseen being;
later outcomes strengthen or weaken the link, and talk passes it on (MND-05, CUL-24). Grief and dreams make the dead
live on (CUL-19). What most of a band's adults hold is the band's: a spirit (CUL-05); an act it credits for a good
outcome, and its way with the dead, become rites it keeps (CUL-34); the way two thirds of its cases went becomes its
custom (CUL-06); and a band grown too big, or riven by a fight between heads of families, splits (CUL-30). Every
first is kept with the events that caused it. The values that set the pace are in tuning.toml. Pre-production code
(research 00): thrown away once its answer is in the architecture (A13).
"""

import random
import sys
import tomllib
from collections import deque
from pathlib import Path

HERE = Path(__file__).parent
sys.path.insert(0, str(HERE.parent / "discovery"))
# P4's year, ages and deaths, written once
from discovery import ADULT, SEASON, YEAR, death_rate  # noqa: E402

SEASONS = ("spring", "summer", "autumn", "winter")
PLACES = (
    "the red cliff", "the alder spring", "the long meadow", "the flint beach", "the bear cave", "the river bend",
    "the birch hill", "the reed marsh", "the old oak", "the salt lick", "the pine ridge", "the white rocks",
    "the elk ford", "the hazel wood", "the black pool", "the high camp",
)  # fmt: skip
BANDS = ("Hawk", "Otter", "Elk", "Fox", "Crane", "Lynx", "Heron", "Boar", "Swan", "Wolf", "Owl", "Beaver")
ACTS = (
    "sang", "danced", "painted their faces", "left a gift at the spring", "burned fat in the fire",
    "touched the red stone", "fasted",
)  # fmt: skip
RARE_FOOD = ("eggs", "berries", "mushrooms", "fish")  # by season, spring to winter
SYLLABLES = ("a", "ka", "ri", "to", "mu", "sa", "ne", "lo", "ti", "ra", "ku", "me", "na", "so", "ha", "yu", "e", "do")
# the 3 of CUL-06's 12 questions these bands meet often enough to answer: how the dead are treated, who shares a big
# kill, where couples live; the way the dead are treated that is a rite at the grave (CUL-34)
QUESTIONS = ("dead", "kill", "home")
MIXED = {"home": "either"}
RITE_DEAD = ("under stones", "buried", "buried with things")
OUTCOMES = {"good hunt": 10.0, "hurt": -10.0, "fever": -10.0, "death": -20.0, "lightning": -12.0, "birth": 10.0}


def load(path=HERE / "tuning.toml"):
    with open(path, "rb") as f:
        return tomllib.load(f)


def season_of(day):
    return (day % YEAR) // SEASON


def when(day):
    """A day as the story tells it: Year 1 begins at the start (TIM-14)."""
    return f"Year {day // YEAR + 1}, {SEASONS[season_of(day)]}"


class Person:
    __slots__ = (
        "id", "name", "age", "female", "alive", "band", "partner", "mother", "father", "kind", "spiritual",
        "links", "beliefs", "opinion", "grieving",
    )  # fmt: skip

    def __init__(self, pid, name, age, female, band, rng):
        self.id = pid
        self.name = name
        self.age = age
        self.female = female
        self.alive = True
        self.band = band
        self.partner = None
        self.mother = None
        self.father = None
        self.kind = rng.random()
        self.spiritual = rng.random()
        self.links = {}  # outcome: {cause: [strength, event]} (MND-05)
        self.beliefs = {}  # unseen beings and the dead: key: [strength, event] (CUL-05, CUL-19)
        self.opinion = {}  # person: opinion, beside the kin's and a first impression (MND-24)
        self.grieving = []  # (the dead, until day, the death's event)


class Band:
    def __init__(self, bid, name, place):
        self.id = bid
        self.name = name
        self.place = place
        self.range = []
        self.members = []
        self.alive = True
        self.leader = None
        self.cases = {q: [] for q in QUESTIONS}  # (day, answer, event)
        self.customs = {}  # question: answer
        self.spirits = {}  # key: day shared
        self.rites = {}  # name: day kept
        self.rite_acts = set()  # acts the band does before each hunt
        self.credited = {}  # act: seasons in a row most adults have credited it for good hunts
        self.dead_since = None  # when the band named its way with the dead, and the cases behind it
        self.counts = {}  # cause: days of the past year it came up on (MND-05)
        self.last_seen = {}
        self.log = deque()  # the past year's days, each the causes that came up
        self.outcome_days = {}  # outcome: days of the past year it came on
        self.outcome_log = deque()
        self.fought = None  # the event of a fight between heads of families this season


class World:
    """Three bands by a river valley in one run, for years, with every event kept."""

    def __init__(self, seed, tuning, bands=3):
        self.rng = random.Random(seed)
        self.t = tuning
        self.day = 0
        self.people = []
        self.bands = []
        self.events = []  # (day, band, text, causes)
        self.firsts = {}  # kind: {day, band, name, causes}
        self.places_used = set()
        rng = self.rng
        places = list(PLACES)
        rng.shuffle(places)
        for b in range(bands):
            band = self._band(places[b * 4 : b * 4 + 4])
            n = rng.randint(self.t["people"]["band_least"], self.t["people"]["band_most"])
            self._families(band, n)
        for band in self.bands:
            self._choose_leader(band)

    # --- people and bands -----------------------------------------------------------------------------------------

    def _band(self, places):
        band = Band(len(self.bands), BANDS[len(self.bands) % len(BANDS)], places[0])
        band.range = list(places)
        self.places_used.update(places)
        self.bands.append(band)
        return band

    def _name(self):
        rng = self.rng
        return "".join(rng.choice(SYLLABLES) for _ in range(rng.randint(2, 3))).capitalize()

    def _person(self, age, female, band):
        p = Person(len(self.people), self._name(), age, female, band.id, self.rng)
        self.people.append(p)
        band.members.append(p.id)
        return p

    def _families(self, band, n):
        """A band of n people in families: couples, their children, a few old (CUL-30)."""
        rng = self.rng
        while len(band.members) < n - 1:
            a = self._person(rng.uniform(18, 45), False, band)
            b = self._person(max(16.0, a.age + rng.uniform(-6, 3)), True, band)
            a.partner, b.partner = b.id, a.id
            for _ in range(min(rng.choice((0, 1, 1, 2, 2, 3)), n - len(band.members))):
                child = self._person(rng.uniform(0, min(14.0, b.age - 15)), rng.random() < 0.5, band)
                child.mother, child.father = b.id, a.id
            if len(band.members) < n and rng.random() < 0.2:
                self._person(rng.uniform(50, 65), rng.random() < 0.5, band)

    def adults(self, band):
        return [self.people[i] for i in band.members if self.people[i].alive and self.people[i].age >= ADULT]

    def kin(self, a, b):
        """Close kin: partners, parents and children, and siblings."""
        if a.partner == b.id or b.partner == a.id or a.mother == b.id or a.father == b.id:
            return True
        if b.mother == a.id or b.father == a.id:
            return True
        return a.mother is not None and (a.mother == b.mother or a.father == b.father)

    def opinion(self, a, b):
        """a's opinion of b (MND-24): kin start fond, others at a first impression of their own; events move it."""
        if b.id not in a.opinion:
            first = 25.0 if self.kin(a, b) else random.Random(a.id * 7919 + b.id * 104729).uniform(-10.0, 30.0)
            a.opinion[b.id] = first
        return a.opinion[b.id]

    def nudge(self, a, b, by):
        a.opinion[b.id] = max(-100.0, min(100.0, self.opinion(a, b) + by))

    def _choose_leader(self, band):
        """The adult the band's adults respect most, in sum (CUL-22)."""
        adults = self.adults(band)
        if not adults:
            band.leader = None
            return
        band.leader = max(adults, key=lambda c: sum(self.opinion(a, c) for a in adults if a is not c) + c.age * 0.1).id

    def heads(self, band):
        """Each family's head: a couple's or lone parent's most respected adult (CUL-30)."""
        seen = set()
        out = []
        for p in self.adults(band):
            if p.id in seen:
                continue
            seen.add(p.id)
            if p.partner is not None:
                seen.add(p.partner)
            out.append(p)
        return out

    def family(self, head):
        """A head, a partner, and the children under 14 living with them."""
        ids = {head.id}
        if head.partner is not None and self.people[head.partner].alive:
            ids.add(head.partner)
        for i in self.bands[head.band].members:
            c = self.people[i]
            if c.alive and c.age < ADULT and (c.mother in ids or c.father in ids):
                ids.add(c.id)
        return ids

    # --- events and firsts ----------------------------------------------------------------------------------------

    def event(self, band, text, causes=(), kind="", who=-1, what="", hour=12.0):
        """An event kept for the story, with its kind, who and what for the director's recognisers (P11), and the
        hour it came at."""
        self.events.append((self.day, band.id, text, tuple(causes), kind, who, what, hour))
        return len(self.events) - 1

    def first(self, kind, band, name, causes):
        if kind not in self.firsts:
            self.firsts[kind] = {"day": self.day, "band": band.id, "name": name, "causes": list(causes)}

    # --- beliefs about causes (MND-05) ------------------------------------------------------------------------------

    def unusual(self, band, causes):
        """Of the causes that came up, the most unusual: on fewer than about one day in ten over the past year, or not
        in the ten days before, a first time most."""
        best = None
        best_days = None
        for c in sorted(causes):
            days = band.counts.get(c, 0)
            seen = band.last_seen.get(c, -999)
            rare = days < YEAR / 10 or self.day - seen > 10
            if rare and (best is None or days < best_days):
                best, best_days = c, days
        return best

    def outcome(self, band, kind, people, recent, ev, unseen=None):
        """A strong outcome met by people (MND-05): a link of theirs whose cause came up explains it and grows;
        otherwise a new link to the most unusual thing of the day or two before, or, for a blow from the sky or a
        sudden death, about one time in three a belief in an unseen being."""
        t = self.t["links"]
        size = abs(OUTCOMES[kind])
        new_cause = self.unusual(band, recent)
        for p in people:
            held = p.links.setdefault(kind, {})
            explaining = [c for c in held if c in recent]
            if explaining:
                c = max(explaining, key=lambda k: held[k][0])
                held[c][0] = min(t["most"], held[c][0] + t["hit"])
            elif new_cause is not None:
                held[new_cause] = [min(t["most"], t["start"] * size), ev]
                if len(held) > 3:
                    del held[min(held, key=lambda k: held[k][0])]
            if unseen is not None and self.rng.random() < t["unseen"] * (0.5 + p.spiritual):
                b = p.beliefs.setdefault(unseen, [0.0, ev])
                b[0] = min(t["most"], b[0] + min(t["most"], t["start"] * size) * 0.5)

    def test_links(self, band):
        """Two days after a cause came up, each link from it to an outcome that did not follow weakens."""
        if len(band.log) < 3:
            return
        then = band.log[-3]
        followed = set().union(*list(band.outcome_log)[-3:]) if band.outcome_log else set()
        t = self.t["links"]
        for p in self.adults(band):
            for kind, held in p.links.items():
                if kind in followed:
                    continue
                frequent = band.outcome_days.get(kind, 0) > YEAR / 10
                for c in [c for c in held if c in then]:
                    held[c][0] -= t["miss_frequent"] if frequent else t["miss"]
                    if held[c][0] < t["forget"]:
                        del held[c]

    def talk(self, band):
        """Each evening some adults tell another what they believe, who takes it by trust (CUL-24)."""
        rng = self.rng
        t = self.t["talk"]
        adults = self.adults(band)
        if len(adults) < 2:
            return
        for p in adults:
            if rng.random() >= t["chance"]:
                continue
            told = [("b", k, v) for k, v in p.beliefs.items() if v[0] >= self.t["links"]["held"]]
            told += [("l", (o, c), v) for o, held in p.links.items() for c, v in held.items() if v[0] >= 15.0]
            if not told:
                continue
            what, key, v = rng.choice(told)
            q = rng.choice(adults)
            if q is p:
                continue
            # hearing it raises the listener's belief to what the teller's conviction lends, by trust, and no
            # further however often it is told
            trust = max(0.2, min(1.0, (self.opinion(q, p) + 50.0) / 100.0))
            lent = v[0] * t["take"] * trust
            if lent < self.t["links"]["forget"]:
                continue
            if what == "b":
                b = q.beliefs.setdefault(key, [0.0, v[1]])
                b[0] = max(b[0], lent)
            else:
                held = q.links.setdefault(key[0], {})
                if key[1] in held:
                    held[key[1]][0] = max(held[key[1]][0], lent)
                elif len(held) < 3:
                    held[key[1]] = [lent, v[1]]

    # --- what the band shares -----------------------------------------------------------------------------------

    def share(self, band):
        """A belief or a credited act most of the band's adults hold becomes the band's: a spirit, or a rite it keeps
        before each hunt (CUL-05, CUL-34)."""
        adults = self.adults(band)
        if len(adults) < 4:
            return
        half = len(adults) / 2.0
        spirits = {}
        acts = {}
        for p in adults:
            for k, v in p.beliefs.items():
                if v[0] >= self.t["links"]["forget"]:
                    spirits.setdefault(k, []).append(v[1])
            for c, v in p.links.get("good hunt", {}).items():
                if v[0] >= self.t["links"]["held"] and c[0] == "did":
                    acts.setdefault(c[1], []).append(v[1])
        for k, evs in spirits.items():
            if len(evs) > half and k not in band.spirits:
                band.spirits[k] = self.day
                name = self.spirit_name(k)
                ev = self.event(
                    band,
                    f"most adults of the {band.name} band now hold {name}",
                    [min(evs)],
                    "spirit",
                    what="the dead" if k[0] == "dead" else k[1],
                    hour=20.0,
                )
                self.first("spirit", band, name, [min(evs), ev])
        if self.day % SEASON != 0:
            return
        # the way a band treats its dead, kept a year, is a rite at the grave (CUL-34)
        custom = band.customs.get("dead")
        if custom in RITE_DEAD and band.dead_since and self.day - band.dead_since[0] >= YEAR:
            name = f"the dead {custom}, with the band at the grave"
            if name not in band.rites:
                band.rites[name] = self.day
                ev = self.event(
                    band,
                    f"the {band.name} band now keeps a rite: {name}",
                    band.dead_since[1][-1:],
                    "rite",
                    what=name,
                    hour=20.0,
                )
                self.first("rite", band, name, band.dead_since[1] + [ev])
        # an act most adults have credited for good hunts a year long becomes a rite the band keeps
        band.credited = {a: band.credited.get(a, 0) + 1 for a, evs in acts.items() if len(evs) > half}
        for act, seasons in band.credited.items():
            name = f"they {act} before a hunt"
            if seasons >= 4 and name not in band.rites:
                band.rites[name] = self.day
                band.rite_acts.add(act)
                ev = self.event(
                    band,
                    f"the {band.name} band now keeps a rite: {name}",
                    [min(acts[act])],
                    "rite",
                    what=name,
                    hour=20.0,
                )
                self.first("rite", band, name, [min(acts[act]), ev])

    @staticmethod
    def spirit_name(key):
        kind, what = key
        return {
            "sky": f"a being in the {what}",
            "animal": f"a spirit of the {what}",
            "place": f"a spirit of {what}",
            "dead": f"{what}, who lives on among the dead",
        }[kind]

    def case(self, band, question, answer, ev):
        """A case of one of CUL-06's questions: once a band has had 3, the way two thirds of them went, counting the
        last 5 years or, where fewer than 5 came in them, its last 5, is its custom."""
        t = self.t["custom"]
        band.cases[question].append((self.day, answer, ev))
        cases = band.cases[question]
        if len(cases) < t["cases"]:
            return
        recent = [c for c in cases if self.day - c[0] <= 5 * YEAR]
        if len(recent) < 5:
            recent = cases[-5:]
        answers = [c[1] for c in recent]
        top = max(sorted(set(answers)), key=answers.count)
        # at least two thirds, compared in whole cases so 2 of 3 counts
        custom = top if answers.count(top) * t["share"][1] >= t["share"][0] * len(answers) else MIXED.get(question)
        if custom is not None and band.customs.get(question) != custom:
            band.customs[question] = custom
            name = self.custom_name(question, custom)
            ev2 = self.event(band, f"the {band.name} band names its custom: {name}", (), "custom", what=name, hour=20.0)
            self.first("custom", band, self.custom_name(question, custom), [c[2] for c in recent] + [ev2])
            if question == "dead":
                band.dead_since = (self.day, [c[2] for c in recent] + [ev2])

    @staticmethod
    def custom_name(question, answer):
        return {
            "dead": f"the dead are {answer}" if answer != "left" else "the dead are left where they fell",
            "kill": f"a big kill is shared with {answer}",
            "home": f"couples live with {answer}" if answer != "either" else "couples live with either's kin",
        }[question]

    def choose(self, band, question, own):
        """A choice on one of the questions: the custom, once there is one; before, often the way most of the band's
        cases went; else the chooser's own way (MND-09)."""
        t = self.t["custom"]
        rng = self.rng
        custom = band.customs.get(question)
        if custom is not None and custom != MIXED.get(question):
            return custom if rng.random() < t["follow"] else own
        cases = band.cases[question]
        if cases and rng.random() < t["copy"]:
            answers = [c[1] for c in cases[-5:]]
            return max(sorted(set(answers)), key=answers.count)
        return own

    def breach(self, band, chooser, question, answer):
        """A custom broken: each adult who sees it thinks less of the one who broke it (CUL-06, MND-24)."""
        custom = band.customs.get(question)
        if custom is None or custom == MIXED.get(question) or custom == answer:
            return
        for a in self.adults(band):
            if a is not chooser:
                self.nudge(a, chooser, -self.t["custom"]["breach"])

    # --- a day ---------------------------------------------------------------------------------------------------

    def run(self, years):
        while self.day < years * YEAR:
            self.advance()

    def advance(self):
        """One day."""
        self.step()
        self.day += 1

    def step(self):
        rng = self.rng
        t = self.t
        season = season_of(self.day)
        if self.day % YEAR == 0:
            for p in self.people:
                if p.alive:
                    p.age += 1.0
        if self.day % YEAR == SEASON:
            self.marry()
        for band in list(self.bands):
            if not band.alive:
                continue
            today = {("place", band.place)}
            happened = set()
            if self.day % SEASON == 0:
                self.season_start(band)
            members = [self.people[i] for i in band.members if self.people[i].alive]
            adults = [p for p in members if p.age >= ADULT]
            # the weather: storms, and lightning near the camp, from the sky
            if rng.random() < t["weather"]["storm"][season]:
                today.add(("weather", "storm"))
                self.event(
                    band, f"a storm gathered over the camp at {band.place}", (), "storm", what=band.place, hour=11.0
                )
                if rng.random() < t["weather"]["lightning"]:
                    struck = rng.choice(members) if members and rng.random() < t["weather"]["lightning_kills"] else None
                    text = f"lightning struck the camp at {band.place}"
                    if struck is not None:
                        text += f" and killed {struck.name}"
                    ev = self.event(band, text, (), "lightning", -1 if struck is None else struck.id, band.place, 13.0)
                    happened.add("lightning")
                    self.outcome(band, "lightning", adults, today, ev, unseen=("sky", "storm"))
                    if struck is not None:
                        self.die(band, struck, ev, sudden=("sky", "storm"))
            # food: a rare one sometimes, by season
            if rng.random() < 0.3:
                today.add(("ate", RARE_FOOD[season]))
            # a hunt: what the hunters do before it changes nothing; the band's rites are done each time
            hunters = [p for p in adults if not p.female and p.age <= 55]
            if len(hunters) >= 2 and rng.random() < t["hunt"]["days"]:
                done = set(band.rite_acts)
                for h in hunters:
                    for c, v in h.links.get("good hunt", {}).items():
                        if c[0] == "did" and rng.random() < min(0.9, v[0] / 50.0):
                            done.add(c[1])
                    if rng.random() < t["acts"]["spontaneous"]:
                        done.add(rng.choice(ACTS))
                for act in done:
                    today.add(("did", act))
                recent = today | (band.log[-1] if band.log else set())
                if rng.random() < t["hunt"]["good"]:
                    hunter = rng.choice(hunters)
                    did = f", after they {' and '.join(sorted(done))}" if done else ""
                    ev = self.event(
                        band, f"{hunter.name}'s hunters brought down an aurochs{did}", (), "kill", hunter.id, hour=14.0
                    )
                    happened.add("good hunt")
                    # it befalls the hunters, who link it to what they did before it; the band hears of it in talk
                    self.outcome(band, "good hunt", hunters, recent, ev)
                    own = "the band" if hunter.kind > 0.5 else "the hunter's family"
                    answer = self.choose(band, "kill", own)
                    self.breach(band, hunter, "kill", answer)
                    if answer == "the band":
                        for a in adults:
                            if a is not hunter:
                                self.nudge(a, hunter, 2.0)
                    self.case(band, "kill", answer, ev)
                for h in hunters:
                    if rng.random() < t["hunt"]["killed"]:
                        ev = self.event(
                            band, f"an aurochs killed {h.name} in the hunt", (), "killed", h.id, "aurochs", 14.0
                        )
                        self.outcome(band, "hurt", hunters, recent, ev, unseen=("animal", "aurochs"))
                        self.die(band, h, ev, sudden=("animal", "aurochs"))
                    elif rng.random() < t["hunt"]["hurt"]:
                        ev = self.event(band, f"{h.name} was gored in the hunt", (), "wounded", h.id, hour=14.0)
                        happened.add("hurt")
                        hurt = [h] + ([self.people[h.partner]] if h.partner is not None else [])
                        self.outcome(band, "hurt", hurt, recent, ev)
            # fevers
            for p in members:
                if p.alive and rng.random() < t["ill"]["chance"]:
                    ev = self.event(band, f"{p.name} fell ill with a fever", (), "ill", p.id, hour=9.0)
                    happened.add("fever")
                    self.outcome(band, "fever", [p], today | (band.log[-1] if band.log else set()), ev)
            # births and deaths
            for p in members:
                if not p.alive:
                    continue
                if rng.random() < death_rate(p.age) / YEAR:
                    ev = self.event(
                        band,
                        f"{p.name} died, a baby" if p.age < 1.0 else f"{p.name} died, aged {int(p.age)}",
                        (),
                        "death",
                        p.id,
                        hour=20.0,
                    )
                    self.die(band, p, ev)
                elif (
                    p.female
                    and 16.0 <= p.age <= 40.0
                    and p.partner is not None
                    and rng.random() < t["people"]["births"] / YEAR
                ):
                    self.birth(band, p, today)
            # grief and dreams of the dead (CUL-19)
            for p in members:
                if not p.alive or not p.grieving:
                    continue
                p.grieving = [g for g in p.grieving if g[1] > self.day]
                for dead, _, death in p.grieving:
                    if rng.random() < t["dead"]["dream"]:
                        key = ("dead", self.people[dead].name)
                        if key not in p.beliefs:
                            dreamt = self.event(
                                band, f"{p.name} dreamt of {key[1]}", [death], "dream", p.id, key[1], 3.0
                            )
                            p.beliefs[key] = [0.0, dreamt]
                        b = p.beliefs[key]
                        b[0] = min(50.0, b[0] + t["dead"]["dream_gain"])
            # fights between heads of families (CUL-30)
            self.quarrel(band)
            self.talk(band)
            self.remember(band, today, happened)
            self.test_links(band)
            if self.day % 5 == 0:
                self.share(band)

    def remember(self, band, today, happened):
        band.log.append(today)
        for c in today:
            band.counts[c] = band.counts.get(c, 0) + 1
            band.last_seen[c] = self.day
        if len(band.log) > YEAR:
            for c in band.log.popleft():
                band.counts[c] -= 1
        band.outcome_log.append(happened)
        for o in happened:
            band.outcome_days[o] = band.outcome_days.get(o, 0) + 1
        if len(band.outcome_log) > YEAR:
            for o in band.outcome_log.popleft():
                band.outcome_days[o] -= 1

    def birth(self, band, mother, today):
        child = self._person(0.0, self.rng.random() < 0.5, band)
        child.mother = mother.id
        child.father = mother.partner
        ev = self.event(band, f"{mother.name} bore {child.name}", (), "birth", child.id, mother.id, 4.0)
        parents = [mother] + ([self.people[mother.partner]] if mother.partner is not None else [])
        self.outcome(band, "birth", parents, today | (band.log[-1] if band.log else set()), ev)

    def die(self, band, p, ev, sudden=None):
        """A death: kin grieve and dream of the dead, a sudden one may be put down to an unseen being, and the closest
        kin treat the body as they choose (CUL-06, CUL-19)."""
        if not p.alive:
            return
        p.alive = False
        rng = self.rng
        kin = [self.people[i] for i in band.members if self.people[i].alive and self.kin(self.people[i], p)]
        for k in kin:
            k.grieving.append((p.id, self.day + 2 * YEAR, ev))
        if p.partner is not None:
            self.people[p.partner].partner = None
        if kin and p.age < self.t["custom"]["dead_from"]:
            recent = {("place", band.place)} | (band.log[-1] if band.log else set())
            self.outcome(band, "death", [k for k in kin if k.age >= ADULT], recent, ev, unseen=sudden)
        elif kin:
            recent = {("place", band.place)} | (band.log[-1] if band.log else set())
            self.outcome(band, "death", [k for k in kin if k.age >= ADULT], recent, ev, unseen=sudden)
            chooser = max(kin, key=lambda k: k.age)
            believes = any(k[0] == "dead" and v[0] >= 20.0 for k, v in chooser.beliefs.items())
            if believes:
                own = "buried with things" if rng.random() < 0.5 else "buried"
            else:
                own = rng.choices(("left", "under stones", "buried"), (0.4, 0.35, 0.25))[0]
            answer = self.choose(band, "dead", own)
            self.breach(band, chooser, "dead", answer)
            told = f"{chooser.name}'s kin: {p.name} {answer}"
            self.case(band, "dead", answer, self.event(band, told, [ev], "burial", chooser.id, answer, 21.0))
        if band.leader == p.id:
            self._choose_leader(band)

    def marry(self):
        """Each summer the bands meet; the unpartnered adults pair, not with close kin, and each couple lives with his
        kin or hers, as the custom says or, until there is one, with whichever band holds more of their kin (CUL-30)."""
        rng = self.rng
        single = [p for p in self.people if p.alive and p.partner is None and 16.0 <= p.age <= 45.0]
        men = [p for p in single if not p.female]
        women = [p for p in single if p.female]
        rng.shuffle(men)
        rng.shuffle(women)
        for m in men:
            match = next((w for w in women if not self.kin(m, w)), None)
            if match is None:
                continue
            women.remove(match)
            m.partner, match.partner = match.id, m.id
            if m.band == match.band:
                continue
            band_m, band_w = self.bands[m.band], self.bands[match.band]
            kin_m = sum(1 for i in band_m.members if self.people[i].alive and self.kin(self.people[i], m))
            kin_w = sum(1 for i in band_w.members if self.people[i].alive and self.kin(self.people[i], match))
            own = "his kin" if kin_m >= kin_w else "her kin"
            host = band_m if own == "his kin" else band_w
            answer = self.choose(host, "home", own)
            if answer == "either":
                answer = own
            stays, moves = (m, match) if answer == "his kin" else (match, m)
            target = self.bands[stays.band]
            ev = self.event(
                target, f"{m.name} and {match.name} married and live with {answer}", (), "married", m.id, match.id, 10.0
            )
            self.case(target, "home", answer, ev)
            self.move(moves, target)

    def move(self, p, target):
        old = self.bands[p.band]
        if p.id in old.members:
            old.members.remove(p.id)
        target.members.append(p.id)
        p.band = target.id

    def quarrel(self, band):
        """Two heads of families who think little of each other may fight (CUL-30)."""
        t = self.t["split"]
        heads = self.heads(band)
        if len(heads) < 2 or self.rng.random() >= t["fight"] * len(heads):
            return
        a, b = self.rng.sample(heads, 2)
        if self.opinion(a, b) < t["fight_below"] and self.opinion(b, a) < t["fight_below"]:
            ev = self.event(band, f"{a.name} and {b.name}, heads of families, fought", (), "fight", a.id, b.id, 18.0)
            self.nudge(a, b, -20.0)
            self.nudge(b, a, -20.0)
            band.fought = ev

    def season_start(self, band):
        """Each season the band moves camp, chooses its leader, and splits if it has grown too big, or after a fight
        between heads of families; a band too small joins kin (CUL-22, CUL-30)."""
        rng = self.rng
        t = self.t["split"]
        band.place = rng.choice(band.range)
        self._choose_leader(band)
        live = [self.people[i] for i in band.members if self.people[i].alive]
        band.members = [p.id for p in live]
        if len(live) > t["size"]:
            ev = self.event(
                band, f"the {band.name} band had grown to {len(live)}", (), "grew", what=len(live), hour=6.0
            )
            self.split(band, live, ev)
        elif band.fought is not None and len(live) > t["after_fight"]:
            self.split(band, live, band.fought)
        elif len(live) < t["join_below"] and len(self.bands) > 1:
            others = [b for b in self.bands if b.alive and b is not band]
            if others:
                into = max(others, key=lambda b: sum(1 for p in live for i in b.members if self.kin(p, self.people[i])))
                told = f"the {band.name} band, down to {len(live)}, joined the {into.name} band"
                self.event(band, told, (), "joined", what=into.id, hour=6.0)
                for p in live:
                    self.move(p, into)
                band.alive = False
        band.fought = None

    def split(self, band, live, cause):
        """The families thinking least of the leader, up to about half the band and leaving at least about 10, found
        a new band a day's walk away under their most respected, keeping the band's customs (CUL-30)."""
        if band.leader is None:
            return
        leader = self.people[band.leader]
        heads = [h for h in self.heads(band) if h.id != leader.id and h.id not in self.family(leader)]
        heads.sort(key=lambda h: self.opinion(h, leader))
        leaving = set()
        for h in heads:
            fam = self.family(h)
            if len(leaving) + len(fam) > len(live) / 2 or len(live) - len(leaving) - len(fam) < 10:
                break
            leaving |= fam
        if len(leaving) < 6:
            return
        free = [p for p in PLACES if p not in self.places_used]
        home = free[0] if free else self.rng.choice(PLACES)
        new = self._band([home] + self.rng.sample(PLACES, 3))
        new.customs = dict(band.customs)
        new.rites = dict(band.rites)
        new.rite_acts = set(band.rite_acts)
        for i in sorted(leaving):
            self.move(self.people[i], new)
        self._choose_leader(new)
        ev = self.event(
            band,
            f"{len(leaving)} of the {band.name} band, those who thought least of {leader.name}, left to found "
            f"the {new.name} band at {home}",
            [cause],
            "split",
            leader.id,
            new.id,
            6.0,
        )
        self.first("split", band, f"the {new.name} band, from the {band.name}", [cause, ev])

    # --- the story -------------------------------------------------------------------------------------------------

    def story(self, kind):
        """A first told from its events: what happened, and when."""
        f = self.firsts.get(kind)
        if f is None:
            return []
        lines = []
        seen = set()
        for e in f["causes"]:
            day, b, text, causes = self.events[e][:4]
            for c in causes:
                if c not in seen and c not in f["causes"]:
                    seen.add(c)
                    d2, b2, t2, _ = self.events[c][:4]
                    lines.append(f"{when(d2)}: {t2}.")
            if e not in seen:
                seen.add(e)
                lines.append(f"{when(day)}: {text}.")
        return lines
