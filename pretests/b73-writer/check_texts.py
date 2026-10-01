"""B73: run the fact checker over a set of written texts (cloud stand-in or phone result) (PRE-17, RSK-17).

Usage:
  python check_texts.py results/standin-qwen2.5-1.5b.json      # a stand-in run
  python check_texts.py phone-result.json                       # the "b73" part of the phone's result code
Prints one line per text and a summary, and writes <input>.checked.json next to the input.
"""
import json
import pathlib
import statistics
import sys

from checker import check, dark_failure, tokenize
from writer import load_records, render_data


def copy_share(rec, voice, text):
    """RSK-08 sign of flat text: the share of the text's 4-word runs copied verbatim from the DATA."""
    data = [t["lw"] for t in tokenize(render_data(rec, voice))]
    words = [t["lw"] for t in tokenize(text)]
    grams = {tuple(data[i:i + 4]) for i in range(len(data) - 3)}
    runs = [tuple(words[i:i + 4]) for i in range(len(words) - 3)]
    return round(sum(r in grams for r in runs) / len(runs), 2) if runs else None


def texts_of(doc):
    """Yield (pass name, run) from a stand-in file or a phone result."""
    if "runs" in doc:
        for r in doc["runs"]:
            yield doc.get("model", "standin"), r
    for p in doc.get("passes", []):
        for r in p.get("runs", []):
            yield f'phone-{p.get("preference")}', r


def main(path):
    path = pathlib.Path(path)
    doc = json.loads(path.read_text())
    doc = doc.get("b73_result", doc)
    recs = {r["id"]: r for r in load_records()}
    rows = []
    for name, r in texts_of(doc):
        rid = r.get("rec") or r["id"].split("-")[0]
        voice = r.get("voice") or ("documentary" if r["id"].endswith("doc") else "tradition")
        rec = recs[rid]
        text = r.get("text") or ""
        flags = check(rec, voice, text) if text else [("REFUSED", "no text: " + r.get("error_code", "?"))]
        rows.append({"pass": name, "id": r["id"], "dark": rec["dark"], "flags": flags,
                     "copied": copy_share(rec, voice, text) if text else None,
                     "dark_fail": dark_failure(rec, flags) or (rec["dark"] and not text),
                     "ttfw_s": r.get("ttfw_s", r["ttfw_ms"] / 1000 if r.get("ttfw_ms") is not None else None) if text else None,
                     "wps": r.get("words_per_s", r.get("wps")) if text else None, "words": r.get("words")})
    for row in rows:
        mark = "PASS" if not row["flags"] else "FLAG"
        print(f'{row["pass"]:>14} {row["id"]:8} {mark} {"DARK-FAIL " if row["dark_fail"] else ""}'
              + "; ".join(f"{f}:{w[:40]}" for f, w in row["flags"]))
    by_pass = {}
    for row in rows:
        by_pass.setdefault(row["pass"], []).append(row)
    summary = {}
    for name, rs in by_pass.items():
        wps = [r["wps"] for r in rs if r["wps"]]
        summary[name] = {
            "texts": len(rs),
            "passed_check": sum(not r["flags"] for r in rs),
            "flagged": sum(bool(r["flags"]) for r in rs),
            "dark_texts": sum(r["dark"] for r in rs),
            "dark_failed": sum(r["dark_fail"] for r in rs),
            "median_first_word_s": round(statistics.median(t), 2) if (t := [r["ttfw_s"] for r in rs if r["ttfw_s"] is not None]) else None,
            "median_words_per_s": round(statistics.median(wps), 2) if wps else None,
            "median_copied": statistics.median([r["copied"] for r in rs if r["copied"] is not None] or [0]),
            "flag_counts": {},
        }
        for r in rs:
            for f, _ in r["flags"]:
                summary[name]["flag_counts"][f] = summary[name]["flag_counts"].get(f, 0) + 1
    print(json.dumps(summary, indent=1))
    out = path.with_suffix(".checked.json")
    out.write_text(json.dumps({"summary": summary, "rows": rows}, indent=1, ensure_ascii=False) + "\n")


if __name__ == "__main__":
    main(sys.argv[1])
