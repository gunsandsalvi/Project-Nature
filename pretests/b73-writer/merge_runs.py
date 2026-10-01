"""B73: merge stand-in runs done in parts (each part holds the shared CPU lock under 15 minutes).
Usage: python merge_runs.py results/standin-NAME.json results/standin-NAME-a.json results/standin-NAME-b.json"""
import json
import pathlib
import sys

out, parts = pathlib.Path(sys.argv[1]), [json.loads(pathlib.Path(p).read_text()) for p in sys.argv[2:]]
merged = dict(parts[0])
merged["runs"] = sorted((r for p in parts for r in p["runs"]), key=lambda r: r["id"])
merged["load_s"] = [p["load_s"] for p in parts]
out.write_text(json.dumps(merged, indent=1, ensure_ascii=False) + "\n")
for p in sys.argv[2:]:
    pathlib.Path(p).unlink()
print(f"{len(merged['runs'])} runs -> {out}")
