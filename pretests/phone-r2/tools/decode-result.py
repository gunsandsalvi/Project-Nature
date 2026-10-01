#!/usr/bin/env python3
"""Round 2: turn the owner's result code (or the shared results file) back into full JSON.

Usage: tools/decode-result.py CODE-or-FILE [OUT_DIR]
  CODE-or-FILE: the pasted code (gzip + base64), a file holding it, or the shared kindling-r2-results.txt.
Prints a short readable summary, and writes to OUT_DIR (default: the current folder):
  phone-r2.json          the results, expanded (texts back in place, long key names)
  phone-r2-nano.json     Gemini Nano's runs, shaped for pretests/b73-writer/check_texts.py
  phone-r2-gemma.json    Gemma's runs, the same shape
Then, for example:  (cd ../b73-writer && python check_texts.py ../phone-r2/phone-r2-gemma.json)
"""
import base64
import gzip
import json
import os
import pathlib
import sys

HERE = pathlib.Path(__file__).resolve().parent
EXPECT = {"gen": "895e636495687a48", "detail": "5e3b0c482d789a49"}  # B04/B11 cloud hashes
LONG = {"i": "id", "f": "ttfw_ms", "n": "total_ms", "w": "words", "s": "wps", "a": "wps_all", "e": "finish",
        "t": "text", "b": "battery_c", "h": "thermal", "c": "error_code", "x": "error", "m": "mem_mb", "k": "tok",
        "ch": "chunks"}
RATING = ["poor", "acceptable", "good"]


def load(arg):
    text = arg
    p = pathlib.Path(arg)
    if len(arg) < 4096 and p.is_file():
        text = p.read_text()
    text = text.strip()
    if text.startswith("{"):  # the shared file: full results, not compacted
        doc = json.loads(text)
        return doc.get("results", doc), True
    return json.loads(gzip.decompress(base64.b64decode("".join(text.split())))), False


def prompts_info():
    """Voice and darkness of each prompt, from the writer test's prompt file."""
    f = HERE.parent.parent / "b73-writer" / "prompts" / "v2" / "all-prompts.json"
    try:
        return {p["id"]: (p["voice"], p["dark"]) for p in json.loads(f.read_text())}
    except OSError:
        return {}


def expand_runs(runs, tx, info, missing):
    out = []
    for r in runs or []:
        o = {}
        for k, v in r.items():
            key = LONG.get(k, k)
            if key == "text":
                if isinstance(v, int) and 0 <= v < len(tx):
                    v = tx[v]
                elif isinstance(v, int):
                    missing.append(r.get("i") or r.get("id"))
                    continue
            o[key] = v
        rid = o.get("id", "")
        if rid in info:
            o.setdefault("voice", info[rid][0])
            o.setdefault("dark", info[rid][1])
        o.setdefault("rec", rid.split("-")[0])
        out.append(o)
    return out


def expand(r, full):
    if full:
        return r, []
    info = prompts_info()
    tx = r.pop("tx", [])
    missing = []
    for p in (r.get("nano") or {}).get("passes", []):
        p["runs"] = expand_runs(p.get("runs"), tx, info, missing)
    if r.get("gm"):
        r["gm"]["runs"] = expand_runs(r["gm"].get("runs"), tx, info, missing)
    return r, missing


def median(xs):
    xs = sorted(x for x in xs if isinstance(x, (int, float)))
    return xs[len(xs) // 2] if xs else None


def summary(r, missing):
    out = []
    say = out.append
    d = r.get("dev", {})
    say(f"app {r.get('app')} | {d.get('model')} ({d.get('soc')}), Android {d.get('rel')} (SDK {d.get('sdk')}), "
        f"{d.get('freeGB')} GB free, Wi-Fi {d.get('wifi')}, media volume {d.get('vol')}, battery {d.get('bat0')}% plug {d.get('plug')}")
    say(f"AICore {d.get('aicore')}, Play services {d.get('gms')}, Private Compute Services {d.get('pcs')}")
    st = r.get("st") or {}
    if "error" in st and st.get("error"):
        say(f"storage/terrain: ERROR {st['error']}")
    elif st:
        g = (st.get("gen") or {}).get("plates", {}).get("hash")
        det = (st.get("detail") or {}).get("threads_1", {}).get("hash")
        z = (st.get("save") or {}).get("custom-zstd", {})
        say(f"storage/terrain ({st.get('total_s')} s): generation hash {g} ({'same as cloud' if g == EXPECT['gen'] else 'DIFFERS'}), "
            f"metre detail {det} ({'same as cloud' if det == EXPECT['detail'] else 'DIFFERS'}); "
            f"quarter-world save {(z.get('total_ms') or {}).get('median')} ms, read {(z.get('read_ms') or {}).get('median')} ms")
    snd = r.get("snd") or {}
    a = snd.get("audio") or {}
    if snd.get("error") or a.get("error"):
        say(f"sound: ERROR {snd.get('error') or a.get('error')}")
    elif a:
        s = a.get("stream") or {}
        say(f"sound: {s.get('sample_rate')} Hz, burst {s.get('frames_per_burst')}, exclusive {s.get('exclusive')}, low latency {s.get('low_latency')}")
        for ph in a.get("phases", []):
            say(f"  {ph.get('voices')} sounds: delay {ph.get('latency_ms_p50')} ms, dropouts {ph.get('xruns')}, load {ph.get('load_mean')} (p99 {ph.get('load_p99')})")
    nano = r.get("nano") or {}
    for p in nano.get("passes", []):
        runs = p.get("runs", [])
        ok = [x for x in runs if "error_code" not in x]
        say(f"Gemini Nano {p.get('preference')}: {p.get('status') or p.get('skipped')} {p.get('base_model') or ''} | "
            f"{len(ok)}/{len(runs)} texts, first word {median([x.get('ttfw_ms') for x in ok])} ms, "
            f"{median([x.get('wps') for x in ok])} words/s{' | stopped: ' + p['stopped'] if p.get('stopped') else ''}"
            f"{' | ' + p['error'] if p.get('error') else ''}")
    gm = r.get("gm") or {}
    if gm:
        dl = gm.get("dl") or {}
        say(f"Gemma: {gm.get('status')} | download {dl.get('status')} {dl.get('got')} bytes in {dl.get('s')} s ({dl.get('mbps')} MB/s), "
            f"resumes {dl.get('resumes', 0)}, check {dl.get('hashS')} s")
        for t in gm.get("tries", []):
            say(f"  {t.get('b')}: {'worked' if t.get('ok') else 'failed'} in {t.get('ms')} ms {t.get('e', '')}")
            for line in t.get("log", [])[:12]:
                say(f"    log: {line}")
        runs = gm.get("runs", [])
        ok = [x for x in runs if "error_code" not in x]
        mem = gm.get("mem") or {}
        say(f"  on {gm.get('be')}: {len(ok)}/{len(runs)} texts, first word {median([x.get('ttfw_ms') for x in ok])} ms, "
            f"{median([x.get('wps') for x in ok])} words/s, settings level {gm.get('cfg', 0)}")
        if mem:
            # peaks are kept per part and backend, e.g. "init-GPU" (start-up) and "write-GPU" (writing)
            parts = ", ".join(f"{k} {v}" for k, v in mem.items() if isinstance(v, list))
            say(f"  memory peaks (MiB: rss, anon, file, pss, graphics): {parts or 'none recorded'}; "
                f"phone's free memory at least {mem.get('availMin')} MiB")
    for x in r.get("rate", []):
        say(f"rating: {'Gemini Nano' if x.get('m') == 'n' else 'Gemma'} {x.get('i')}: {RATING[x.get('r', 0)]}")
    for key in ("crash", "partCrash", "err", "uncaught", "trim"):
        if r.get(key):
            say(f"{key}: {r[key]}")
    if missing:
        say(f"texts not in the code (trimmed; see the shared file): {', '.join(str(m) for m in missing)}")
    return "\n".join(out)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    r, full = load(sys.argv[1])
    outdir = pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else os.getcwd())
    outdir.mkdir(parents=True, exist_ok=True)
    r, missing = expand(r, full)
    (outdir / "phone-r2.json").write_text(json.dumps(r, indent=1, ensure_ascii=False) + "\n")
    if r.get("nano"):
        (outdir / "phone-r2-nano.json").write_text(json.dumps(r["nano"], indent=1, ensure_ascii=False) + "\n")
    if r.get("gm", {}).get("runs"):
        doc = {"passes": [{"preference": f"GEMMA-{r['gm'].get('be')}", "runs": r["gm"]["runs"]}]}
        (outdir / "phone-r2-gemma.json").write_text(json.dumps(doc, indent=1, ensure_ascii=False) + "\n")
    print(summary(r, missing))
    print(f"(written to {outdir})", file=sys.stderr)


if __name__ == "__main__":
    main()
