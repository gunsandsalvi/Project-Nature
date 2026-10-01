"""B73 fact checker (Part 2): does a written text stick to its record? (PRE-17, PRE-41, RSK-17, PRE-38)

No language model is involved (PRN-06): word lists, a stemmer and a few patterns, so the same
rules can be ported to the phone. A text passes only if it gets no flag.

Flags
  NAME     a capitalised name that is not in the data
  NUMBER   a number that is not in the data
  WORD     a content word (object, place, action, image) that the data and lexicon don't license
  MOTIVE   a feeling or motive word that isn't in the data
  BELIEF   spirits, gods, ancestors, magic... that aren't in the data
  HIDDEN   a fact the tellers can't know, in the "their own tradition" voice (PRE-14)
  CAUSE    a "because"-type link when the data gives no cause at all
  MODERN   our word for a thing instead of theirs (PRE-38)
  SOFTENED a euphemism in a dark record (RSK-17)
  HEDGED   a hedge ("perhaps", "may have") in a dark record (RSK-17)
  OMITTED  a core event of the record isn't told (with its people)
  ROLE     a core event told with the wrong person doing it
  TELLER   the tradition voice speaks as the wrong band
  LEAK     the prompt's own instructions or labels echoed back
  REFUSED  the model refused

Run 1 (blind, before any model output) had all but ORDINAL handling, the wrong-band check and LEAK;
those three were added in run 2, after reading the stand-in texts (see NOTES.md).
"""
import json
import pathlib
import re

from nltk.stem.porter import PorterStemmer

try:
    from wordfreq import zipf_frequency
except ImportError:  # labels only; checking works without it
    zipf_frequency = None

HERE = pathlib.Path(__file__).resolve().parent
LEX = json.loads((HERE / "data" / "lexicon.json").read_text())
WORD_RE = re.compile(r"[A-Za-z]+(?:['-][A-Za-z]+)*'?|\d+(?:st|nd|rd|th)?")
_stem = PorterStemmer().stem

IRREGULAR = {
    "struck": "strike", "broke": "break", "broken": "break", "fell": "fall", "fallen": "fall", "ran": "run",
    "took": "take", "taken": "take", "gave": "give", "given": "give", "came": "come", "went": "go", "gone": "go",
    "left": "leave", "found": "find", "saw": "see", "seen": "see", "told": "tell", "dreamt": "dream",
    "dreamed": "dream", "woke": "wake", "woken": "wake", "fled": "flee", "made": "make", "led": "lead",
    "slew": "slay", "slain": "slay", "spoke": "speak", "spoken": "speak", "said": "say", "thought": "think",
    "knew": "know", "known": "know", "held": "hold", "stood": "stand", "sat": "sit", "laid": "lay",
    "bore": "bear", "born": "bear", "borne": "bear", "wept": "weep", "kept": "keep", "caught": "catch",
    "brought": "bring", "began": "begin", "begun": "begin", "drove": "drive", "driven": "drive",
    "threw": "throw", "thrown": "throw", "froze": "freeze", "frozen": "freeze", "met": "meet", "felt": "feel",
    "lost": "lose", "sent": "send", "tore": "tear", "torn": "tear", "hid": "hide", "hidden": "hide",
    "rose": "rise", "risen": "rise", "shone": "shine", "shook": "shake", "got": "get", "grew": "grow",
    "grown": "grow", "heard": "hear", "meant": "mean", "sought": "seek", "taught": "teach", "fought": "fight",
    "bound": "bind", "lit": "light", "slept": "sleep", "became": "become", "did": "do", "done": "do",
    "had": "have", "has": "have", "was": "be", "were": "be", "been": "be", "is": "be", "are": "be", "am": "be",
    "children": "child", "men": "man", "women": "woman", "wolves": "wolf", "knives": "knife", "blew": "blow",
    "blown": "blow", "burnt": "burn", "spat": "spit", "dug": "dig", "hung": "hang", "swam": "swim",
    "ate": "eat", "eaten": "eat", "drank": "drink", "sang": "sing", "wore": "wear", "worn": "wear",
    "chose": "choose", "chosen": "choose", "wound": "wound", "dying": "die", "died": "die", "dies": "die",
    "lying": "lie", "lay": "lay", "laying": "lay",
}
NUMWORDS = {
    "two": 2, "three": 3, "four": 4, "five": 5, "six": 6, "seven": 7, "eight": 8, "nine": 9, "ten": 10,
    "eleven": 11, "twelve": 12, "thirteen": 13, "fourteen": 14, "fifteen": 15, "sixteen": 16,
    "seventeen": 17, "eighteen": 18, "nineteen": 19, "twenty": 20, "thirty": 30, "forty": 40, "fifty": 50,
    "sixty": 60, "seventy": 70, "eighty": 80, "ninety": 90, "dozen": 12, "twice": 2, "thrice": 3,
    "pair": 2, "couple": 2, "hundred": 100, "thousand": 1000,
}
ORDINALS = {
    "second": 2, "third": 3, "fourth": 4, "fifth": 5, "sixth": 6, "seventh": 7, "eighth": 8, "ninth": 9,
    "tenth": 10, "eleventh": 11, "twelfth": 12, "thirteenth": 13, "fourteenth": 14, "fifteenth": 15,
    "twentieth": 20, "hundredth": 100,
}
DARK_FLAGS = {"SOFTENED", "HEDGED", "OMITTED", "ROLE", "REFUSED"}


def stem(word):
    w = word.lower().strip("'")
    if w.endswith("'s"):
        w = w[:-2]
    return _stem(IRREGULAR.get(w, w))


def stems_of(phrase):
    return tuple(stem(t) for t in WORD_RE.findall(phrase))


def english(word):
    return zipf_frequency is not None and zipf_frequency(word.lower(), "en") >= 3.5


def norm(text):
    return (text.replace("’", "'").replace("‘", "'").replace("“", '"')
            .replace("”", '"').replace("—", ", ").replace("–", "-"))


def tokenize(text):
    """Words with their sentence number and whether they start a sentence (or a quote or clause)."""
    toks = []
    for si, sent in enumerate(re.split(r"(?<=[.!?])\s+|\n+", norm(text))):
        for k, m in enumerate(WORD_RE.finditer(sent)):
            w = m.group(0).rstrip("'")
            if w.lower().endswith("'s"):
                w = w[:-2]
            before = sent[: m.start()].rstrip()
            initial = k == 0 or before.endswith((":", '"', "(", "-"))
            toks.append({"w": w, "lw": w.lower(), "sent": si, "initial": initial})
    return toks


def _strings(obj, skip=("id", "ours", "about")):
    if isinstance(obj, str):
        yield obj
    elif isinstance(obj, list):
        for x in obj:
            yield from _strings(x, skip)
    elif isinstance(obj, dict):
        for k, v in obj.items():
            if k not in skip:
                yield from _strings(v, skip)


def _numbers(obj):
    if isinstance(obj, bool):
        return
    if isinstance(obj, int):
        yield obj
    elif isinstance(obj, list):
        for x in obj:
            yield from _numbers(x)
    elif isinstance(obj, dict):
        for v in obj.values():
            yield from _numbers(v)


SYN = [[stems_of(p) for p in g] for g in LEX["synonyms"]]
STYLE = {w for p in LEX["style"] for w in [p]}
FRAME = {stem(w) for w in LEX["tradition_frame"]}
MODERN = {stem(w) for w in LEX["modern"]}
MOTIVE = {stem(w) for w in LEX["motive"]}
BELIEF = {stem(w) for w in LEX["belief"]}


class Allowed:
    """What the writer was given: the record, minus hidden facts for the tradition voice."""

    def __init__(self, rec, voice):
        view = {k: v for k, v in rec.items() if not (k == "hidden" and voice != "documentary")}
        texts = list(_strings(view))
        self.data_text = " \n".join(texts).lower()
        self.stems, self.names, self.nums = set(), set(), set(_numbers(view))
        self.ordinals = set()  # run 2: "third child" must match an ordinal in the data, not any 3
        self.person_names = {p["name"].lower() for p in rec["people"]}
        for s in texts:
            for k, t in enumerate(tokenize(s)):
                w = t["w"]
                if w[0].isdigit():
                    self.nums.add(int(re.match(r"\d+", w).group()))
                    if not w.isdigit():
                        self.ordinals.add(int(re.match(r"\d+", w).group()))
                    continue
                if t["lw"] in ORDINALS:
                    self.ordinals.add(ORDINALS[t["lw"]])
                for part in [w] + w.split("-"):
                    self.stems.add(stem(part))
                if w[0].isupper() and (k > 0 or not english(w)):
                    self.names.add(t["lw"])
                self.nums.update(v for x, v in {**NUMWORDS, **ORDINALS}.items() if x == t["lw"])
            # a count the data implies: "Rena and Oshi" are 2 people
            n = len({t["lw"] for t in tokenize(s)} & self.person_names)
            if n >= 2:
                self.nums.add(n)
        self.names |= self.person_names
        self.names |= {pl["name"].lower() for pl in rec["places"]}
        self.stems |= {stem(w) for w in ("year", "old", "age")}  # ages render as "N years old"
        seq = [stem(t["w"]) for s in texts for t in tokenize(s)]
        for group in SYN:  # a concept in the data licenses all its renderings (PRE-41)
            if any(_contains(seq, g) for g in group):
                for g in group:
                    self.stems.update(g)
        self.stems -= {stem(w) for c in rec.get("concepts", []) for w in c["ours"]}
        self.hidden = set()
        if voice != "documentary":
            for h in rec.get("hidden", []):
                self.hidden |= {stem(t["w"]) for t in tokenize(h["text"])} - self.stems
        self.modern = MODERN | {stem(w) for c in rec.get("concepts", []) for w in c["ours"]}
        self.has_cause = any(b.get("role") for b in rec.get("beliefs", [])) or bool(
            re.search(r"\b(so|why|because)\b", self.data_text))


def _contains(seq, sub):
    n = len(sub)
    return any(tuple(seq[i:i + n]) == sub for i in range(len(seq) - n + 1))


def _label(lw, st, al):
    if st in al.modern:
        return "MODERN"
    if st in MOTIVE:
        return "MOTIVE"
    if st in BELIEF:
        return "BELIEF"
    if st in al.hidden:
        return "HIDDEN"
    return "WORD"


def _number_runs(toks):
    """Yield (index, value, surface, is_ordinal) for numbers as digits or words ("twenty-six", "4th", "fourth")."""
    i = 0
    while i < len(toks):
        lw = toks[i]["lw"]
        if lw[0].isdigit():
            yield i, int(re.match(r"\d+", lw).group()), toks[i]["w"], not lw.isdigit()
            i += 1
            continue
        parts = lw.split("-")
        if all(p in NUMWORDS or p in ORDINALS or p == "one" for p in parts) and not lw == "one":
            total, cur, j, words = 0, 0, i, []
            while j < len(toks):
                ps = toks[j]["lw"].split("-")
                if all(p in NUMWORDS or p in ORDINALS or p == "one" for p in ps):
                    for p in ps:
                        v = NUMWORDS.get(p, ORDINALS.get(p, 1))
                        if v in (100, 1000):
                            cur = max(cur, 1) * v
                        else:
                            cur += v
                    words.append(toks[j]["w"])
                    j += 1
                elif toks[j]["lw"] == "and" and j + 1 < len(toks) and toks[j + 1]["lw"] in NUMWORDS:
                    j += 1
                else:
                    break
            total += cur
            yield i, total, " ".join(words), any(p in ORDINALS for w in words for p in w.lower().split("-"))
            i = j
            continue
        i += 1


def _verb_forms(verb):
    verbs = verb if isinstance(verb, list) else [verb]
    forms = set()
    for v in verbs:
        st = stems_of(v)
        forms.add(st)
        for group in SYN:
            if st in group:
                forms.update(group)
    return forms


def _core_checks(rec, toks):
    flags = []
    pnames = {p["name"].lower() for p in rec["people"]}
    seq = [stem(t["w"]) for t in toks]
    low = [t["lw"] for t in toks]
    for ev in rec["events"]:
        if not ev.get("core") or "verb" not in ev:
            continue
        forms = _verb_forms(ev["verb"])
        who, whom = ev.get("who", "").lower(), ev.get("whom", "").lower()
        occ = [i for i in range(len(seq)) for f in forms if tuple(seq[i:i + len(f)]) == f]
        ok = swapped = False
        for i in occ:
            s = toks[i]["sent"]
            prev = next((low[j] for j in range(i - 1, max(-1, i - 40), -1) if low[j] in pnames), None)
            after = [low[j] for j in range(i + 1, len(toks)) if toks[j]["sent"] == s]
            near = {low[j] for j in range(len(toks)) if toks[j]["sent"] in (s, s - 1)}
            if who in pnames:
                if prev == who:
                    ok = True
                elif prev is not None and prev != who:
                    if whom in pnames and prev == whom and "by" in after and who in after[after.index("by"):]:
                        ok = True  # passive: "Moro was killed by Anuk"
                    elif (whom in pnames and prev == whom and who in after) or (whom not in pnames):
                        swapped = True
            elif whom not in pnames or whom in near:
                ok = True
        if not ok:
            flags.append(("ROLE" if swapped else "OMITTED", ev["text"]))
    return flags


def _phrase(text_low, phrase):
    return re.search(r"(?<![a-z])" + re.escape(phrase) + r"(?![a-z])", text_low) is not None


def check(rec, voice, text):
    """Return a list of (flag, what) pairs; an empty list means the text passes."""
    al = Allowed(rec, voice)
    toks = tokenize(text)
    low = norm(text).lower()
    flags = []
    for p in LEX["refusal"]:
        if _phrase(low, p):
            flags.append(("REFUSED", p))
    for p in LEX.get("leak", []):
        if p in low:
            flags.append(("LEAK", p))
    if voice != "documentary":  # run 2: the tellers speak as their own band, never as another
        own = rec["tellers"].lower().split()[0]
        for m in re.finditer(r"\bwe(?:,| of)? the (\w+)", low):
            if m.group(1) != own and m.group(1) in al.names:
                flags.append(("TELLER", f"told as the {m.group(1)}, not the {own}"))
    num_idx = set()
    for i, val, surf, ordinal in _number_runs(toks):
        num_idx.add(i)
        if val not in al.nums or (ordinal and val not in al.ordinals):
            flags.append(("NUMBER", surf))
    for i, t in enumerate(toks):
        w, lw = t["w"], t["lw"]
        if i in num_idx or lw[0].isdigit() or lw in NUMWORDS or lw in ORDINALS:
            continue
        if lw in STYLE or (voice != "documentary" and stem(lw) in FRAME):
            continue
        if lw in al.names:
            continue
        parts = [lw] + ([p for p in lw.split("-") if p] if "-" in lw else [])
        st = stem(lw)
        if st in al.modern:
            flags.append(("MODERN", w))
            continue
        if st in al.stems or (len(parts) > 1 and all(stem(p) in al.stems or p in STYLE for p in parts[1:])):
            continue
        if w[0].isupper() and not english(lw) and _label(lw, st, al) == "WORD":
            flags.append(("NAME", w))
        else:
            flags.append((_label(lw, st, al), w))
    if not al.has_cause:
        for p in LEX["cause"]:
            if _phrase(low, p):
                flags.append(("CAUSE", p))
    if rec["dark"]:
        for p in LEX["euphemism"]:
            if _phrase(low, p) and p not in al.data_text:
                flags.append(("SOFTENED", p))
        for p in LEX["hedge"]:
            if _phrase(low, p) and p not in al.data_text:
                flags.append(("HEDGED", p))
    flags += _core_checks(rec, toks)
    return flags


def dark_failure(rec, flags):
    """RSK-17: a dark record softened, hedged, left out, told with the wrong killer, or refused."""
    return rec["dark"] and any(f in DARK_FLAGS for f, _ in flags)


if __name__ == "__main__":
    import sys
    from writer import load_records
    recs = {r["id"]: r for r in load_records()}
    rid, voice, text = sys.argv[1], sys.argv[2], sys.argv[3]
    for f in check(recs[rid], voice, text):
        print(*f)
