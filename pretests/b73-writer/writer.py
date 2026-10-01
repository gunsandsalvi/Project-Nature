"""B73 writer AI pipeline: record -> DATA block -> prompt (PRE-37, PRE-41, PRE-19).

The DATA block is built by fixed rules from the record, so it is also the plain
factual text shown when a written text fails its check (PRE-41).
The "their own tradition" voice never sees facts hidden from the people (PRE-14).
"""
import json
import pathlib

HERE = pathlib.Path(__file__).resolve().parent
VOICES = ("documentary", "tradition")
PROMPTS = "v2"  # v1: first round; v2: tightened once after the stand-in run, as the decision rule says


def load_records():
    return json.loads((HERE / "data" / "records.json").read_text())["records"]


def _person(p):
    bits = [p["sex"]]
    if "age" in p:
        bits.append(f'{p["age"]} years old')
    bits.append(f'of the {p["group"]}')
    s = f'- {p["name"]}: ' + ", ".join(bits)
    for k in ("kin", "role"):
        if k in p:
            s += f"; {p[k]}"
    return s + "."


def _belief(b):
    sure = "and was sure" if b["certainty"] == "sure" else "but was not sure"
    s = f'- {b["who"]} believed ({sure}): {b["belief"]}.'
    if "role" in b:
        s += f' ({b["role"][0].upper() + b["role"][1:]}.)'
    return s


def render_data(rec, voice):
    """The facts the writer may use, one per line (PRE-17)."""
    w = rec["when"]
    when = ", ".join([f'year {w["year"]}'] + [w[k] for k in ("season", "time") if k in w])
    lines = [f'Event: {rec["kind"]}.', f"When: {when}."]
    for i, pl in enumerate(rec["places"]):
        label = "Place" if i == 0 else "Other place"
        lines.append(f'{label}: {pl["name"]} (their word; it means "{pl["means"]}"), {pl["is"]}.')
    lines.append("People:")
    lines += [_person(p) for p in rec["people"]]
    if rec.get("context"):
        lines.append("Background:")
        lines += [f"- {c}" for c in rec["context"]]
    lines.append("What happened, in order:")
    lines += [f'- {e["text"]}' for e in rec["events"]]
    if "myth" in rec:
        m = rec["myth"]
        lines.append(f'The story: {m["name"]} (it means "{m["means"]}"), first told {m["first_told"]}.')
        lines.append("- The old telling: " + " ".join(m["old_version"]))
        lines.append("- The telling now: " + " ".join(m["new_version"]))
        lines.append("- What changed: " + "; ".join(m["changes"]) + ".")
    if rec.get("beliefs"):
        lines.append("Beliefs (not facts):")
        lines += [_belief(b) for b in rec["beliefs"]]
    if voice == "documentary" and rec.get("hidden"):
        lines.append("Known to the world but not to the people:")
        lines += [f'- {h["text"]}' for h in rec["hidden"]]
    if rec.get("concepts"):
        terms = ", ".join(f'"{c["theirs"]}"' for c in rec["concepts"])
        lines.append(f"Their own words for things (use these): {terms}.")
    return "\n".join(lines)


def build_prompt(rec, voice, version=PROMPTS):
    tpl = (HERE / "prompts" / version / f"{voice}.txt").read_text()
    return tpl.replace("{data}", render_data(rec, voice)).replace("{tellers}", rec["tellers"])


def all_prompts(version=PROMPTS):
    """The 20 prompts of this pre-test: 10 records x 2 voices."""
    return [
        {"id": f'{r["id"]}-{v[:3]}', "rec": r["id"], "voice": v, "dark": r["dark"], "prompt": build_prompt(r, v, version)}
        for r in load_records() for v in VOICES
    ]


if __name__ == "__main__":
    recs = load_records()
    print(build_prompt(recs[3], "documentary"))
    print("=" * 60)
    print(build_prompt(recs[3], "tradition"))
