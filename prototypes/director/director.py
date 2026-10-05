"""P11 The director (IMPLEMENTATION α0.6c, research 13): recognisers that sift a world's event log for what is worth
telling (PRE-39), and a director that slows time and offers live moments for it within one budget (TIM-02).

It sees nothing but the log. An event is a tuple of plain values, (day, hour, people, band, kind, who, what, how),
handed over once it has happened; this module imports nothing of the worlds and they nothing of it, so it can never
cause, change or hide anything (TIM-03). The recognisers follow Felt's story sifting, patterns of events by kind, who
and order, and Winnow's incremental sifting: a pattern half matched is a sign, so time can slow before an outcome
without looking ahead. Its values are in tuning.toml. Pre-production code (research 00): thrown away once its answer
is in the architecture (A14).
"""

import tomllib
from pathlib import Path

HERE = Path(__file__).parent
YEAR = 60  # days in a game year (TIM-18)
DAY, HOUR, PEOPLE, BAND, KIND, WHO, WHAT, HOW = range(8)
DEATHS = ("death", "killed", "lightning")
FIRE = ("drill", "plough")
# TIM-19's steps of the arc these worlds can reach, by the blueprints that reach them
STEPS = {"flake": "sharp stone flakes", "drill": "making fire", "plough": "making fire"}


def load(path=HERE / "tuning.toml"):
    with open(path, "rb") as f:
        return tomllib.load(f)


def time_of(ev):
    """An event's time, in game days from the start."""
    return ev[DAY] + ev[HOUR] / 24.0


def first_key(ev):
    """What makes an event's kind for PRE-39's firsts, or None for kinds that are never one: a thing made, a custom,
    a rite, a spirit by its being, a band splitting or joining another, a death by aurochs or lightning, a fight."""
    kind = ev[KIND]
    if kind == "learn":
        return ("make", ev[WHAT])
    if kind in ("custom", "rite", "spirit"):
        return (kind, ev[WHAT])
    if kind in ("split", "joined", "fight"):
        return (kind,)
    if kind == "killed" or (kind == "lightning" and ev[WHO]):
        return ("killed by", ev[WHAT] if kind == "killed" else "lightning")
    return None


class Finding:
    """What the recognisers found: a moment, or a sign, a pattern half matched whose end may come."""

    __slots__ = ("kind", "ev", "score", "span", "sign", "what", "record")

    def __init__(self, kind, ev, score, span, sign=None, what=None):
        self.kind = kind
        self.ev = ev
        self.score = score
        self.span = span  # game hours what it shows takes
        self.sign = sign  # for a sign, the pattern half matched
        self.what = ev[WHAT] if what is None else what
        self.record = None  # for a sign, whether its end came, once known


class Pattern:
    """A story-sifting pattern: its steps, each a set of event kinds, matched in order by events that share the
    fields of its key, each within so many game days of the last; from its sign_at-th step on it is a sign of its
    end."""

    def __init__(self, name, steps, key, within, sign_at, ends, followed=False):
        self.name = name
        self.steps = steps
        self.key = key
        self.within = within
        self.sign_at = sign_at
        self.ends = ends
        self.followed = followed  # only for one you follow


# TIM-02's signs these worlds have: someone trying the same hunch again; a storm over a camp; one you follow badly
# hurt or ill. Its others, a predator stalking a person and two hostile groups in sight, need animals and peoples
# these worlds don't have.
PATTERNS = (
    Pattern("a hunch tried again", (("hunch",), ("try",), ("try",), ("learn",)), (WHO, WHAT), 60.0, 3, "discovery"),
    Pattern("a storm over a camp", (("storm",), ("lightning",)), (BAND,), 0.5, 1, "lightning at a camp"),
    Pattern("one you follow hurt or ill", (("wounded", "ill"), DEATHS), (WHO,), 15.0, 1, "death", followed=True),
)


class Recognisers:
    """Story sifting over the event log (PRE-39): firsts worldwide and for each people, named discoveries and
    rediscoveries (MAT-21), crafts lost with their last holder (CUL-02), a band left without fire, the lives of those
    you follow (PRE-06), feuds (CUL-31), the steps of the arc that begin ages; and, half matched, the signs of what
    may come (TIM-02). Each event is seen once, in order, and only what has been seen is kept."""

    def __init__(self, tuning, followed=(), signs="expected"):
        self.moments = tuning["moments"]
        self.precision = tuning["signs"]
        self.signs_by = signs  # "expected": a sign's score is its end's times how often it came; "end": its end's
        self.followed = frozenset(followed)
        self.firsts = set()  # kinds the history has recorded, worldwide
        self.own = set()  # (people, kind): and for each people
        self.knows = {}  # (people, who): what they can make, from what they learned
        self.holders = {}  # (people, what): how many living can make it
        self.lost = set()  # (people, what): lost with its last holder
        self.fights = set()  # (people, one, other): who have fought
        self.partial = {}  # (pattern, key): [last step's time, steps matched, its signs]
        self.turning = []  # (time, step, finding): steps of the arc first reached anywhere (TIM-19)
        self.signs = []  # every sign: [pattern, time, whether its end came within what a slowdown shows]
        self.found = []  # every moment

    def moment(self, kind, ev, what=None):
        score, span = self.moments[kind]
        f = Finding(kind, ev, score, span, what=what)
        self.found.append(f)
        return f

    def see(self, ev):
        """The moments this event completes and the signs it half matches."""
        out = []
        kind, people, who, what = ev[KIND], ev[PEOPLE], ev[WHO], ev[WHAT]
        key = first_key(ev)
        if kind == "learn":
            out += self._learn(ev, key)
        elif key is not None and (people, key) not in self.own:
            self.own.add((people, key))
            world = key not in self.firsts
            self.firsts.add(key)
            out.append(self.moment("world first" if world else "people's first", ev))
        if kind in DEATHS and who:
            out += self._death(ev)
        if kind == "lightning":
            out.append(self.moment("lightning at a camp", ev))
        elif kind == "birth" and what in self.followed:
            out.append(self.moment("birth to one you follow", ev))
        elif kind == "fire out" and not any(self.holders.get((people, bp), 0) for bp in FIRE):
            out.append(self.moment("last fire", ev))
        elif kind == "fight":
            pair = (people, min(who, what), max(who, what))
            if pair in self.fights:
                out.append(self.moment("feud", ev))
            self.fights.add(pair)
        return out + self._patterns(ev)

    def _learn(self, ev, key):
        """A blueprint learned: a people's first is a named discovery, a world first if no people made it before
        (MAT-21); one learned again after it was lost is a rediscovery (PRE-39)."""
        out = []
        people, who, what = ev[PEOPLE], ev[WHO], ev[WHAT]
        if (people, key) not in self.own:
            self.own.add((people, key))
            world = key not in self.firsts
            self.firsts.add(key)
            f = self.moment("world first discovery" if world else "named discovery", ev)
            out.append(f)
            if world and STEPS.get(what) and all(s != STEPS[what] for _, s, _ in self.turning):
                self.turning.append((time_of(ev), STEPS[what], f))
        elif (people, what) in self.lost:
            self.lost.discard((people, what))
            out.append(self.moment("rediscovery", ev))
        held = self.knows.setdefault((people, who), set())
        if what not in held:
            held.add(what)
            self.holders[(people, what)] = self.holders.get((people, what), 0) + 1
        return out

    def _death(self, ev):
        """A death: what only they could make is lost with them (CUL-02), and one you follow is mourned (PRE-06)."""
        out = []
        people, who = ev[PEOPLE], ev[WHO]
        for craft in sorted(self.knows.pop((people, who), ())):
            self.holders[(people, craft)] -= 1
            if self.holders[(people, craft)] == 0:
                self.lost.add((people, craft))
                out.append(self.moment("craft lost", ev, what=craft))
        if who in self.followed:
            out.append(self.moment("death of one you follow", ev))
        return out

    def _patterns(self, ev):
        """Each pattern advanced by the event: a start, a step, a step tried again, or its end; a sign from its
        sign_at-th step, scored by what its end would be worth."""
        out = []
        t = time_of(ev)
        for pat in PATTERNS:
            if pat.followed and ev[WHO] not in self.followed:
                continue
            key = (pat.name, tuple(ev[i] for i in pat.key))
            state = self.partial.get(key)
            if state is not None and t - state[0] > pat.within:
                del self.partial[key]
                state = None
            if ev[KIND] in pat.steps[0]:
                if self._worth(pat, ev):
                    self.partial[key] = [t, 1, []]
                    state = self.partial[key]
                    if pat.sign_at <= 1:
                        out.append(self._sign(pat, ev, state, t))
                continue
            if state is None:
                continue
            n = state[1]
            if ev[KIND] in pat.steps[n]:
                state[0], state[1] = t, n + 1
                if state[1] == len(pat.steps):
                    self._came(pat, ev, state, t)
                    del self.partial[key]
                elif state[1] >= pat.sign_at:
                    out.append(self._sign(pat, ev, state, t))
            elif n >= pat.sign_at and ev[KIND] in pat.steps[n - 1]:
                state[0] = t
                out.append(self._sign(pat, ev, state, t))  # the same step again: tried again
        return out

    def _worth(self, pat, ev):
        """A hunch is worth watching only for what its people cannot yet make; any other pattern always."""
        if pat.ends != "discovery":
            return True
        return ev[WHAT] != "nothing" and self.holders.get((ev[PEOPLE], ev[WHAT]), 0) == 0

    def _end(self, pat, ev):
        """The moment a pattern's end would be, from what has been seen."""
        if pat.ends == "discovery":
            key = ("make", ev[WHAT])
            if (ev[PEOPLE], key) in self.own:
                return "rediscovery"
            return "named discovery" if key in self.firsts else "world first discovery"
        return "death of one you follow" if pat.ends == "death" else pat.ends

    def _sign(self, pat, ev, state, t):
        end = self._end(pat, ev)
        score, span = self.moments[end]
        if self.signs_by == "expected":
            score *= self.precision[pat.name]
        record = [pat.name, t, False]
        self.signs.append(record)
        state[2].append(record)
        f = Finding(pat.name, ev, score, span, sign=pat)
        f.record = record
        return f

    def _came(self, pat, ev, state, t):
        """A pattern's end came: each sign of it within the span its end's slowdown shows came true."""
        span = self.moments[self._end(pat, ev)][1] / 24.0
        for record in state[2]:
            if t - record[1] <= span:
                record[2] = True


def ages(turning, least):
    """PRE-39's ages: each begins at a step of the arc first reached anywhere, and lasts at least `least` years before
    another begins; a turning point inside an age begins none. Each is (time, name)."""
    out = []
    for t, step, _ in turning:
        if not out or t - out[-1][0] >= least * YEAR:
            out.append((t, f"The age of {step}"))
    return out


class Director:
    """Sets only the speed of time and the live moments (TIM-02, TIM-03). When a moment or a sign passes the bar and
    the one budget allows, time slows so that what it shows takes about half a minute, until you tap or it passes, or
    for about 10 seconds untapped; every other moment past the bar waits in the list. After each slowdown the bar
    stands higher for a while and falls back, Left 4 Dead's rest after a peak, so a higher score slows time sooner
    after the last, never slower; and it never asks for a faster speed than zoom's."""

    def __init__(self, tuning, top, taps=False):
        self.b = tuning["budget"]
        self.bar = tuning["bar"]["globe"]
        self.top = top  # zoom's speed, game days a real second
        self.taps = taps  # whether you tap every live moment
        self.slowdowns = []  # (start, end, speed, finding), in real seconds
        self.listed = []  # (real second, finding) kept from slowing time

    def speed(self, r):
        """The speed asked at real second r, in game days a second, and until when it holds."""
        if self.slowdowns:
            start, end, v, _ = self.slowdowns[-1]
            if start <= r < end:
                return v, end
        return self.top, None

    def rest(self, r):
        """How much higher than the zoom's the bar stands at real second r: by the whole rest once the gap since the
        last slowdown has passed, falling to nothing by rest_end; a watch opens rested, as if one had just passed, so
        its first live moments are major ones."""
        b = self.b
        since = r - self.slowdowns[-1][0] if self.slowdowns else r + b["gap"]
        return b["rest"] * min(1.0, max(0.0, (b["rest_end"] - since) / (b["rest_end"] - b["gap"])))

    def slowed(self, start, end):
        """The real seconds slowed between start and end."""
        return sum(max(0.0, min(e, end) - max(s, start)) for s, e, _, _ in self.slowdowns)

    def consider(self, f, r):
        """A finding at real second r: a slowdown if the budget allows, else, for a moment, the list."""
        if f.score < self.bar:
            return False
        b = self.b
        length = b["moment"] if self.taps else min(b["untapped"], b["moment"])
        last = self.slowdowns[-1] if self.slowdowns else None
        ok = f.score >= self.bar + self.rest(r) and (last is None or (r >= last[1] and r - last[0] >= b["gap"]))
        if ok and self.slowed(r + length - b["window"], r) + length <= b["share"] * b["window"]:
            self.slowdowns.append((r, r + length, min(self.top, f.span / 24.0 / b["moment"]), f))
            return True
        if f.sign is None:
            self.listed.append((r, f))
        return False


def watch(events, recognisers, director, end):
    """Plays a world's events to the recognisers and the director in order, at the speed the director asks, from the
    globe, until game day `end`: each event is handed over once its time has come, and nothing goes back. Returns the
    real seconds the watch took."""
    r = g = 0.0
    for ev in events:
        r, g = _run_to(director, r, g, time_of(ev))
        for f in recognisers.see(ev):
            director.consider(f, r)
    r, g = _run_to(director, r, g, end)
    return r


def _run_to(director, r, g, target):
    """Real and game time once game time has reached target, at the speeds asked on the way."""
    while g < target:
        v, until = director.speed(r)
        need = (target - g) / v
        if until is None or r + need <= until:
            return r + need, target
        g += (until - r) * v
        r = until
    return r, g
