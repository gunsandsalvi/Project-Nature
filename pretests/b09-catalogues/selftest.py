#!/usr/bin/env python3
"""B09 pre-test: does the checker catch planted errors? (throwaway)

Plants one error at a time in copies of the entries, in memory, and counts how many
the checker flags. Uses the fetch cache, so run check.py once first (RSK-16, PRN-05).
"""
import copy
import random
import re
import sys
import tomllib
from pathlib import Path

import yaml

sys.path.insert(0, str(Path(__file__).resolve().parent))
import check  # noqa: E402

random.seed(9)
YAML = sorted((check.ENTRIES / "yaml").glob("*.yaml"))
entries = {p.stem: yaml.safe_load(p.read_text()) for p in YAML}
results = {}


def tally(kind, caught):
    results.setdefault(kind, [0, 0])
    results[kind][0] += bool(caught)
    results[kind][1] += 1


def quote_found(entry, p):
    url = entry["sources"][p["source"]]["url"]
    m, text = check.source_text(url)
    if text is None:
        return None
    variants = {name: f(text) for name, f in check.TIERS}
    return check.find_quote(p["quote"], variants)


for stem, e in entries.items():
    for i, p in enumerate(e["properties"]):
        if p["property"] in check.CATEGORICAL:
            continue
        # 1. a wrong number (typo or misreading): value changed by 10%
        bad = copy.deepcopy(e)
        bad["properties"][i]["value"] = round(p["value"] * 1.1, 3)
        _, _, mism = check.validate(bad, stem)
        tally("wrong value (x1.1)", bool(mism))
        # 2. a misquote: one word of the quote changed
        words = p["quote"].split()
        k = random.randrange(len(words))
        bad_p = dict(p, quote=" ".join(words[:k] + ["zzz"] + words[k + 1:]))
        tally("quote with one word changed", quote_found(e, bad_p) is None)
        # 3. a wrong unit spelling
        bad = copy.deepcopy(e)
        bad["properties"][i]["unit"] = p["unit"].upper() + "x"
        errs, _, _ = check.validate(bad, stem)
        tally("unknown unit", bool(errs))
        # 4. a source key that doesn't exist
        bad = copy.deepcopy(e)
        bad["properties"][i]["source"] = "no-such-source"
        errs, _, _ = check.validate(bad, stem)
        tally("missing source", bool(errs))

# 5. the YAML number trap: 2.5e3 is read as text by YAML 1.1 loaders
for snippet in ["value: 2.5e3", "value: 1e-3", "value: 2,600", "value: '2.6'"]:
    loaded = yaml.safe_load(snippet)["value"]
    bad = copy.deepcopy(entries["flint"])
    bad["properties"][0]["value"] = loaded
    errs, _, _ = check.validate(bad, "flint")
    tally("YAML number read as text", any("not a number" in x for x in errs))

# 6. TOML: a number written as text is a type error too
bad_toml = (check.ENTRIES / "toml" / "flint.toml").read_text().replace("value = 2.60", 'value = "2.60"', 1)
bad = tomllib.loads(bad_toml)
errs, _, _ = check.validate(bad, "flint")
tally("TOML number written as text", any("not a number" in x for x in errs))

# 7. Markdown: the table people read drifts from the machine block
for md in sorted((check.ENTRIES / "md").glob("*.md")):
    text = md.read_text()
    rows = [ln for ln in text.splitlines() if ln.startswith("| ") and re.search(r"\| \S*\d", ln)
            and "Property" not in ln]
    for row in rows:
        drifted = text.replace(row, re.sub(r"(\| [^|]*?)(\d)", lambda m: m.group(1) + str((int(m.group(2)) + 1) % 10), row, 1), 1)
        block = check.FENCE.findall(drifted)[0]
        probs = check.md_cross_check(drifted, yaml.safe_load(block))
        tally("Markdown table drifts from block", bool(probs))

# 8. formats disagree: one number differs between YAML and TOML
for stem in entries:
    t = tomllib.loads((check.ENTRIES / "toml" / f"{stem}.toml").read_text())
    t["properties"][0]["value"] = t["properties"][0]["value"] + 0.01
    tally("YAML and TOML disagree", check.canonical(t) != check.canonical(entries[stem]))

# 9. KNOWN BLIND SPOT: the value swapped for another number from its own quote
#    (a column mix-up in a table). Reported, not counted in the total.
blind = [0, 0]
for stem, e in entries.items():
    for i, p in enumerate(e["properties"]):
        if p["property"] in check.CATEGORICAL or p.get("basis") == "midpoint":
            continue
        others = sorted(n for n in check.numbers_in(p["quote"]) if not check.close(n, float(p["value"])))
        if not others:
            continue
        bad = copy.deepcopy(e)
        bad["properties"][i]["value"] = others[0]
        bad["properties"][i].pop("low", None), bad["properties"][i].pop("high", None)
        _, _, mism = check.validate(bad, stem)
        blind[0] += bool(mism)
        blind[1] += 1

width = max(map(len, results))
print("planted error".ljust(width), " caught")
for kind, (c, n) in results.items():
    print(kind.ljust(width), f" {c}/{n}")
total_c = sum(c for c, _ in results.values())
total_n = sum(n for _, n in results.values())
print("TOTAL".ljust(width), f" {total_c}/{total_n}")
print(f"blind spot: value swapped for another number in its own quote: caught {blind[0]}/{blind[1]}")
sys.exit(0 if total_c == total_n else 1)
