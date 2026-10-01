"""B73 Part 2: test the checker on faithful texts and planted errors (PRE-17, RSK-17).

Usage: python run_planted.py [--out results/planted-run1.json]
A planted error counts as caught if the edited text gets at least one flag that the faithful text
did not get. Dark planted errors (soften, omit, hedge, and role on a dark record) count only if a
dark flag (SOFTENED, HEDGED, OMITTED, ROLE, REFUSED) appears.
"""
import argparse
import collections
import json
import pathlib

from checker import DARK_FLAGS, check
from writer import load_records

HERE = pathlib.Path(__file__).resolve().parent


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="results/planted.json")
    args = ap.parse_args()
    recs = {r["id"]: r for r in load_records()}
    faithful = {t["rec"]: t for t in json.loads((HERE / "data" / "faithful.json").read_text())["texts"]}
    planted = json.loads((HERE / "data" / "planted.json").read_text())["planted"]

    out = {"faithful": [], "planted": []}
    passed = 0
    for rid, t in faithful.items():
        fl = check(recs[rid], t["voice"], t["text"])
        passed += not fl
        out["faithful"].append({"rec": rid, "voice": t["voice"], "flags": fl})
    by_type = collections.defaultdict(lambda: [0, 0])
    dark_types = {"soften", "omit", "hedge"}
    for p in planted:
        base = faithful[p["rec"]]
        text = base["text"]
        for old, new in p["edits"]:
            assert old in text, (p["id"], old)
            text = text.replace(old, new)
        before = {tuple(f) for f in check(recs[p["rec"]], base["voice"], base["text"])}
        flags = [f for f in check(recs[p["rec"]], base["voice"], text) if tuple(f) not in before]
        is_dark = p["type"] in dark_types or (p["type"] == "role" and recs[p["rec"]]["dark"])
        caught = any(f in DARK_FLAGS for f, _ in flags) if is_dark else bool(flags)
        by_type[p["type"]][0] += caught
        by_type[p["type"]][1] += 1
        out["planted"].append({"id": p["id"], "type": p["type"], "caught": caught, "new_flags": flags})

    total = sum(v[1] for v in by_type.values())
    caught = sum(v[0] for v in by_type.values())
    dark = [x for x in out["planted"] if x["type"] in dark_types]
    out["summary"] = {
        "faithful_passed": f"{passed}/{len(faithful)}",
        "planted_caught": f"{caught}/{total} ({100 * caught / total:.0f}%)",
        "dark_caught": f'{sum(x["caught"] for x in dark)}/{len(dark)}',
        "by_type": {k: f"{v[0]}/{v[1]}" for k, v in sorted(by_type.items())},
        "missed": [x["id"] for x in out["planted"] if not x["caught"]],
    }
    path = HERE / args.out
    path.parent.mkdir(exist_ok=True)
    path.write_text(json.dumps(out, indent=1) + "\n")
    print(json.dumps(out["summary"], indent=1))
    for f in out["faithful"]:
        if f["flags"]:
            print("FAITHFUL FLAGGED", f["rec"], f["flags"])


if __name__ == "__main__":
    main()
