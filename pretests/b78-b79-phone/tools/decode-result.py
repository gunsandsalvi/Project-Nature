#!/usr/bin/env python3
"""B78/B79: turn the owner's pasted result code back into JSON, plus a short readable summary.
Usage: tools/decode-result.py CODE   (or pipe the code on stdin). Writes the JSON to stdout's end."""
import base64, gzip, json, sys

code = "".join(sys.argv[1:]) if len(sys.argv) > 1 else sys.stdin.read()
r = json.loads(gzip.decompress(base64.b64decode("".join(code.split()))))

def line(*a):
    print(*a, file=sys.stderr)

d = r.get("dev", {})
line(f"app {r.get('app')} lib {r.get('lib')} | {d.get('model')} Android {d.get('rel')} (SDK {d.get('sdk')}), page {d.get('page')} B")
line(f"clusters ({d.get('clSrc')}): {d.get('cl')} | RAM {d.get('ramMiB')} MiB, memoryClass {d.get('mc')}/{d.get('lmc')}")
if "fp" in r:
    f = r["fp"]
    line(f"frames: asked {f.get('req')} Hz, got {f.get('hz')} Hz, p50 {f.get('p50')} ms, p99 {f.get('p99')} ms, missed {f.get('missPct')}%")
if "k" in r:
    k = r["k"]
    line(f"kernels: {len(k.get('lab', []))} configs, same-core repeats match: {k.get('rep')} | all runs match: {k.get('x')} | pin misses {k.get('pinMiss')}")
if "sus" in r:
    s = r["sus"]
    line(f"sustained ({s.get('k')}): minute 10 / minute 1 = {s.get('r10')}, first 'moderate' at {s.get('tMod')} s, "
         f"max status {s.get('stMax')}, headroom max {s.get('hrMax')}, battery {s.get('pctH')}%/h")
if "mem" in r:
    m = r["mem"]
    line(f"memory: ended '{m.get('end')}' at {m.get('mib')} MiB")
for key in ("crash", "err", "uncaught", "trim"):
    if key in r:
        line(f"{key}: {r[key]}")
print(json.dumps(r, indent=1))
