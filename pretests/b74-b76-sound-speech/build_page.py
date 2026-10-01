#!/usr/bin/env python3
"""B74/B76: build listen.html (every clip as Ogg Opus in a data: URI) and results/summary.json
(medians of the three timing runs). Usage: build_page.py RUN_DIR (clips from run_all.sh)."""
import base64, glob, html, json, os, statistics, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
RES = f"{HERE}/results"
RUN = sys.argv[1] if len(sys.argv) > 1 else "."
sys.path.insert(0, HERE)
from speech import SENTENCES, CONSONANTS, VOWELS, LEXICON, S1_PEOPLE  # noqa: E402


def load(name, default=None):
    try:
        return json.load(open(f"{RES}/{name}", encoding="utf-8"))
    except Exception:
        return default


# ---------- medians of the timing runs ----------
def med(xs):
    xs = [x for x in xs if x is not None]
    return statistics.median(xs) if xs else None


runs = [load(f"bench-run{i}.json") for i in (1, 2, 3)]
runs = [r for r in runs if r]
summary = {"b74_runs": len(runs), "sounds": {}, "mixer": {}}
if runs:
    for row in runs[0]["sounds"]:
        k = f'{row["sound"]}/{row["approach"]}'
        vals = [next(x for x in r["sounds"] if f'{x["sound"]}/{x["approach"]}' == k) for r in runs]
        summary["sounds"][k] = {"core_share_per_voice": med([v["core_share_per_voice"] for v in vals]), "voices_per_core": med([v["voices_per_core"] for v in vals]), "runs": [round(v["voices_per_core"]) for v in vals]}
    for row in runs[0]["mixer"]:
        k = f'{row["approach"]}/{row["voices"]}'
        vals = [next(x for x in r["mixer"] if f'{x["approach"]}/{x["voices"]}' == k) for r in runs]
        summary["mixer"][k] = {"core_share": med([v["core_share"] for v in vals]), "voices_per_core": med([v["voices_per_core"] for v in vals]), "runs": [round(v["voices_per_core"]) for v in vals]}
sruns = [load(f"speech-bench-run{i}.json") for i in (1, 2, 3)]
sruns = [r for r in sruns if r]
summary["b76_runs"] = len(sruns)
summary["speech"] = {}
for k in ("s1_espeak", "s2_english", "s2_welsh"):
    if sruns and k in sruns[0]:
        summary["speech"][k] = {"core_share": med([r[k]["core_share"] for r in sruns]), "runs": [round(r[k]["core_share"], 4) for r in sruns]}
        for extra in ("model_mb", "load_s"):
            if extra in sruns[0][k]:
                summary["speech"][k][extra] = med([r[k][extra] for r in sruns])
if sruns:
    summary["speech"]["max_rss_mb"] = med([r["max_rss_mb"] for r in sruns])
kept = load("kept.json", {})
for k in ("s1_espeak_cy", "s1_espeak_en-us", "s2_english", "s2_welsh"):
    if f"{k}_kept" in kept:
        summary.setdefault("kept", {})[k] = {"count": kept[f"{k}_kept"], "bent": [s for s in kept["sounds"] if not kept[k][s]["kept"]]}
json.dump(summary, open(f"{RES}/summary.json", "w"), indent=1, ensure_ascii=False)

# ---------- clips ----------
def wav_levels(path):
    """(active RMS in dBFS over 20 ms windows within 30 dB of the loudest, peak in dBFS)."""
    import wave
    import numpy as np
    with wave.open(path) as w:
        a = np.frombuffer(w.readframes(w.getnframes()), dtype="<i2").astype(np.float64) / 32767
        sr = w.getframerate()
    n = int(0.02 * sr)
    p = np.array([np.mean(a[i:i + n] ** 2) for i in range(0, len(a) - n, n)]) + 1e-20
    act = p[p > p.max() * 1e-3]
    return 10 * np.log10(act.mean()), 20 * np.log10(np.abs(a).max() + 1e-12)


GAIN = {}


def match(paths, target=-20.0, ceiling=-1.0):
    """Give a comparison group the same active loudness, as close to `target` as peaks allow,
    so a by-ear choice isn't swayed by one clip being louder."""
    lv = {p: wav_levels(p) for p in paths}
    t = min([target] + [rms + (ceiling - peak) for rms, peak in lv.values()])
    for p, (rms, _) in lv.items():
        GAIN[p] = t - rms


def opus(path, kbps, stereo=False):
    if path not in GAIN:
        match([path])
    out = subprocess.run(["ffmpeg", "-v", "error", "-i", path, "-af", f"volume={GAIN[path]:.2f}dB", "-ac", "2" if stereo else "1", "-c:a", "libopus", "-b:a", f"{kbps}k",
                          "-vbr", "on", "-application", "audio", "-f", "ogg", "-"], capture_output=True, check=True).stdout
    return "data:audio/ogg;base64," + base64.b64encode(out).decode()


CLIPS = f"{RUN}/clips"
SPEECH = f"{RUN}/speech"
for k in ("flint", "granite", "wood", "bone"):
    match([f"{CLIPS}/impact-{k}-modal.wav", f"{CLIPS}/impact-{k}-noise.wav"])
match([f"{CLIPS}/camp-modal.wav", f"{CLIPS}/camp-noise.wav"])
for n in range(1, 6):
    match([f"{SPEECH}/s1-espeak-{n}.wav", f"{SPEECH}/s2-english-{n}.wav", f"{SPEECH}/s2-welsh-{n}.wav"])
match([f"{SPEECH}/s1-espeak-people.wav", f"{SPEECH}/s2-english-people.wav"])
desc = load("describe.json", {})
mats = {m["material"]: m for m in desc.get("materials", [])}
flutes = {f["flute"]: f for f in desc.get("flutes", [])}
drums = {d["drum"]: d for d in desc.get("drums", [])}
meta = load("speech-meta.json", {})
e = html.escape


def player(src, caption, title):
    return f'<figure class="clip"><figcaption><b>{e(title)}</b> {caption}</figcaption><audio controls preload="none" src="{src}"></audio></figure>'


def hz(x):
    return f"{x/1000:.1f} kHz" if x >= 1000 else f"{x:.0f} Hz"


def vpc(key):
    v = summary["sounds"].get(key, {}).get("voices_per_core")
    return f"{v:,.0f}" if v else "–"


parts = []
# Impacts
MAT = {
    "flint": ("Flint", "a 16 cm slab being worked", "a glassy ring after the click"),
    "granite": ("Granite", "a 30 cm anvil stone on the ground", "a short, heavy clack with grit"),
    "wood": ("Dry wood", "a 50 cm stick held in one hand", "a hollow knock with a brief pitch"),
    "bone": ("Bone", "a 24 cm hollow long bone, held", "a dry, higher tock than wood"),
}
cards = []
for k, (name, what, listen) in MAT.items():
    m = mats.get(k, {})
    facts = ""
    if m:
        facts = f'<p class="facts">Contact {m["contact_us"]:.0f} µs · lowest modes {", ".join(hz(f) for f in m["lowest_modes_hz"][:3])} · strongest {hz(m["strongest_mode_hz"])}, rings {m["strongest_ring_s"]:.2f} s</p>'
    a1 = player(opus(f"{CLIPS}/impact-{k}-modal.wav", 40), f"Modes worked out from the {name.lower()}'s stiffness, density, size and damping. Listen for {listen}.", "A1 modal ·")
    a2 = player(opus(f"{CLIPS}/impact-{k}-noise.wav", 40), "The same three strikes as noise shaped by the same numbers. Listen for whether it still sounds like this material.", "A2 noise ·")
    cards.append(f'<article class="card"><h3>{name} <span>{e(what)}, struck three times by a hammerstone</span></h3>{facts}<div class="pair">{a1}{a2}</div></article>')
parts.append(f'<section id="impacts"><h2>Struck materials <small>B74 · SND-06</small></h2><p class="lede">Each clip is the same three strikes (hard, soft, harder) by a quartzite hammerstone. A1 builds the sound from the object\'s ringing modes; A2 is the cheap one. Tell us, material by material, which sounds more like the real thing, and whether the four are easy to tell apart.</p>{"".join(cards)}'
             + player(opus(f"{CLIPS}/impact-flint-sizes.wav", 40), "Flint slabs of 6, 10 and 22 cm, struck the same way (A1). Listen for the pitch dropping as the stone grows.", "Size ·")
             + "</section>")

# Instruments
def notes_line(f):
    if not f:
        return ""
    ns = [n for n in f["notes"] if n["register"] == 1]
    worst = max(abs(n["error_cents"]) for n in f["notes"])
    hzs = ", ".join("%.0f" % n["computed_hz"] for n in ns)
    over = f["notes"][-1]["computed_hz"]
    return f'<p class="facts">Notes from the bore and holes: {hzs} Hz; overblown {over:.0f} Hz. Played pitch within {worst:.1f} cents of the worked-out pitch.</p>'


def drum_line(d):
    if not d:
        return ""
    lows = sorted(set(round(x) for x in d["lowest_modes_hz"]))[:4]
    return f'<p class="facts">Strongest low modes {", ".join(str(x) for x in lows)} Hz, struck 60% of the way out.</p>'


inst = [
    f'<article class="card"><h3>Bone flute A <span>22 cm, 8.4 mm bore, five holes spaced evenly by eye</span></h3>{notes_line(flutes.get("a"))}'
    + player(opus(f"{CLIPS}/flute-a.wav", 40), "Each hole opened in turn from the bottom, the overblown octave, then a short tune. Listen for a breathy bone flute, and whether its uneven scale is believable.", "Flute A ·") + "</article>",
    f'<article class="card"><h3>Bone flute B <span>31 cm, wider bore, four larger holes</span></h3>{notes_line(flutes.get("b"))}'
    + player(opus(f"{CLIPS}/flute-b.wav", 40), "The same pattern on a longer, wider bone. Listen for lower, fuller notes than flute A.", "Flute B ·") + "</article>",
    f'<article class="card"><h3>Hide drum, small and tight <span>30 cm across, 3,500 N/m</span></h3>{drum_line(drums.get("small"))}'
    + player(opus(f"{CLIPS}/drum-small.wav", 40), "Hand at the centre twice, near the edge twice, then hand and stick. Listen for the centre deep and short, the edge ringing.", "Drum ·") + "</article>",
    f'<article class="card"><h3>Hide drum, large and slack <span>60 cm across, 1,200 N/m</span></h3>{drum_line(drums.get("large"))}'
    + player(opus(f"{CLIPS}/drum-large.wav", 40), "The same strikes on a bigger, looser hide. Listen for a deeper boom than the small drum (a phone speaker can't play its lowest notes).", "Drum ·") + "</article>",
]
parts.append(f'<section id="instruments"><h2>Instruments from their shapes <small>B74 · CUL-10</small></h2><p class="lede">The flutes\' notes come only from the bone\'s length, bore and holes, and the drums\' from the hide\'s size, tension and weight. Nothing is tuned to a scale.</p>{"".join(inst)}</section>')

# Mix
parts.append('<section id="mix"><h2>Many sounds at once <small>B74 · SND-01, SND-08</small></h2><p class="lede">Thirty-two sounds around you, 1.5 to 25 m away: further ones are quieter and duller. Headphones help.</p>'
             + player(opus(f"{CLIPS}/camp-modal.wav", 64, stereo=True), "Knapping, wood, bone, the anvil stone, drums and a flute, all A1. Listen for a believable busy camp.", "Camp, A1 ·")
             + player(opus(f"{CLIPS}/camp-noise.wav", 64, stereo=True), "The same camp with A2 impacts. Listen for whether the scene loses its materials.", "Camp, A2 ·")
             + "</section>")

# Speech
inv = f'<p class="ipa big">{" ".join(CONSONANTS)}<br>{" ".join(VOWELS)}</p>'
lex = ", ".join(f'<span class="ipa">{e(w)}</span> {e(m)}' for w, m in LEXICON.items())
scards = []
people = meta.get("clips", [])
for i, (sent, gloss, eng) in enumerate(SENTENCES):
    n = i + 1
    who = people[i]["s1"] if i < len(people) else S1_PEOPLE[i % 3][0]
    vo = people[i]["s2_english"] if i < len(people) else "one speaker"
    clips = (player(opus(f"{SPEECH}/s1-espeak-{n}.wav", 24), f"espeak-ng fed the phonemes, {e(who)}. Listen for every sound present but a robotic voice.", "S1 synthetic ·")
             + player(opus(f"{SPEECH}/s2-english-{n}.wav", 24), f'Piper, trained on English, {e(vo)}. Listen for whether <span class="ipa">ɬ x q r</span> and plain <span class="ipa">e o</span> survive.', "S2 English-trained ·")
             + player(opus(f"{SPEECH}/s2-welsh-{n}.wav", 24), 'Piper, trained on Welsh. Listen for <span class="ipa">ɬ</span> and <span class="ipa">x</span> kept, and whether it just sounds Welsh.', "S2 Welsh-trained ·"))
    scards.append(f'<article class="card"><h3 class="ipa sent">{e(sent)}</h3><p class="gloss">{e(gloss)} · <i>{e(eng)}</i></p>{clips}</article>')
spk = ", ".join(f'{s["label"]} ({s["median_f0_hz"]} Hz)' for s in meta.get("speakers", []))
parts.append(f'<section id="speech"><h2>Speech in an invented language <small>B76 · SND-03, CUL-17</small></h2>'
             f'<p class="lede">A tiny language made for this test: 15 consonants and 5 vowels, syllables like <span class="ipa">ta</span> or <span class="ipa">tan</span>, stress on the first syllable, verb last. Sounds English lacks are there on purpose: <span class="ipa">q ʔ x ɬ r</span>, word-initial <span class="ipa">ŋ</span>, and plain <span class="ipa">e o</span>.</p>{inv}<p class="lex">{lex}</p>'
             + "".join(scards)
             + '<h3 class="sub">Different people</h3>'
             + player(opus(f"{SPEECH}/s1-espeak-people.wav", 24), "“Child, come here!” by a man, a woman and an old man (pitch, speed and voice type). Listen for three different people.", "S1 ·")
             + player(opus(f"{SPEECH}/s2-english-people.wav", 24), f"The same by three of the 904 English-trained speakers, picked by pitch: {e(spk)}. Listen for three different people.", "S2 ·")
             + "</section>")

# Costs
def pct(x):
    return f"{100*x:.2f}%" if x is not None else "–"


rows = []
for k, label in [("flint/modal", "Flint, A1"), ("flint/noise", "Flint, A2"), ("granite/modal", "Granite, A1"), ("granite/noise", "Granite, A2"),
                 ("wood/modal", "Wood, A1"), ("wood/noise", "Wood, A2"), ("bone/modal", "Bone, A1"), ("bone/noise", "Bone, A2"),
                 ("flute/harmonics+noise", "Flute"), ("drum/modal", "Drum")]:
    s = summary["sounds"].get(k, {})
    rows.append(f'<tr><td>{label}</td><td>{pct(s.get("core_share_per_voice"))}</td><td>{vpc(k)}</td></tr>')
mixrows = []
for ap in ("Modal", "Noise"):
    for n in (128, 512):
        m = summary["mixer"].get(f"{ap}/{n}", {})
        if m:
            mixrows.append(f'<tr><td>Camp mix, {"A1" if ap == "Modal" else "A2"}, {n} sounds</td><td>{pct(m["core_share"])}</td><td>{m["voices_per_core"]:,.0f}</td></tr>')
sp = summary.get("speech", {})
kp = summary.get("kept", {})
srows = []
for k, label, kk in [("s1_espeak", "S1 espeak-ng (Welsh table)", "s1_espeak_cy"), ("s2_english", "S2 Piper, English-trained", "s2_english"), ("s2_welsh", "S2 Piper, Welsh-trained", "s2_welsh")]:
    c = sp.get(k, {}).get("core_share")
    kk_ = kp.get(kk, {})
    bent = " ".join(kk_.get("bent", [])) or "none"
    srows.append(f'<tr><td>{label}</td><td>{pct(c)}</td><td>{kk_.get("count", "–")} of 20</td><td class="ipa">{e(bent)}</td></tr>')
parts.append('<section id="costs"><h2>What it costs <small>cloud, one core, median of 3 runs</small></h2>'
             '<div class="scroll"><table><thead><tr><th>Sound</th><th>Share of a core per sound</th><th>Sounds per core</th></tr></thead><tbody>'
             + "".join(rows + mixrows) + '</tbody></table></div>'
             '<div class="scroll"><table><thead><tr><th>Speech</th><th>Share of a core while speaking</th><th>Sounds kept</th><th>Bent</th></tr></thead><tbody>'
             + "".join(srows) + '</tbody></table></div>'
             '<p class="note">“Sounds kept”: the synthetic voice has its own sound for each phoneme; a neural voice keeps a sound only if its training language uses it on its own. The phone\'s own numbers come back from the test app.</p></section>')

CSS = """
/* Layout: one reading column; cards hold one material, instrument or sentence each. */
:root{--bg:#eef0ec;--surface:#f8f9f6;--ink:#1c2223;--muted:#5b6563;--line:#d3d8d2;--ochre:#9a6a12;--flint:#2f4a57;
--display:"Gentium Book Plus","Charis SIL","Noto Serif",Georgia,serif;--body:"Atkinson Hyperlegible",system-ui,-apple-system,"Segoe UI",sans-serif}
@media (prefers-color-scheme:dark){:root:not([data-theme="light"]){--bg:#141819;--surface:#1c2224;--ink:#e6e3da;--muted:#9aa4a1;--line:#2f3739;--ochre:#d9a441;--flint:#9cc0cf;color-scheme:dark}}
:root[data-theme="dark"]{--bg:#141819;--surface:#1c2224;--ink:#e6e3da;--muted:#9aa4a1;--line:#2f3739;--ochre:#d9a441;--flint:#9cc0cf;color-scheme:dark}
body{background:var(--bg);color:var(--ink);font:16px/1.55 var(--body);margin:0}
main{max-width:44rem;margin:0 auto;padding:1.5rem 16px 3rem;display:flex;flex-direction:column;gap:2.25rem}
header{display:flex;flex-direction:column;gap:.5rem}
.eyebrow{font-size:.78rem;letter-spacing:.08em;text-transform:uppercase;color:var(--ochre);margin:0;font-weight:700}
h1{font:700 2rem/1.15 var(--display);margin:0;text-wrap:balance;color:var(--flint)}
h2{font:700 1.45rem/1.2 var(--display);margin:0;text-wrap:balance;color:var(--flint);display:flex;flex-wrap:wrap;gap:.25rem .6rem;align-items:baseline}
h2 small{font:600 .72rem var(--body);letter-spacing:.06em;text-transform:uppercase;color:var(--muted)}
h3{font:700 1.08rem/1.3 var(--body);margin:0;display:flex;flex-direction:column;gap:.1rem}
h3 span{font-weight:400;font-size:.88rem;color:var(--muted)}
h3.sub{margin-top:.5rem}
section{display:flex;flex-direction:column;gap:1rem}
p{margin:0}
.lede{max-width:65ch}
.card{background:var(--surface);border:1px solid var(--line);border-radius:10px;padding:1rem;display:flex;flex-direction:column;gap:.75rem;min-width:0}
.pair{display:grid;grid-template-columns:repeat(auto-fit,minmax(15rem,1fr));gap:.75rem}
.clip{margin:0;display:flex;flex-direction:column;gap:.35rem;min-width:0}
.clip figcaption{font-size:.9rem;color:var(--ink)}
.clip figcaption b{color:var(--ochre);font-weight:700}
audio{width:100%;height:40px}
.facts{font-size:.85rem;color:var(--muted);font-variant-numeric:tabular-nums}
.ipa{font-family:var(--display)}
.big{font-size:1.5rem;letter-spacing:.12em;color:var(--flint);line-height:1.5}
.sent{font:700 1.35rem/1.3 var(--display);color:var(--flint)}
.gloss{color:var(--muted);font-size:.92rem}
.lex{font-size:.9rem;color:var(--muted)}
.scroll{overflow-x:auto;border:1px solid var(--line);border-radius:10px;background:var(--surface)}
table{border-collapse:collapse;width:100%;font-size:.9rem;font-variant-numeric:tabular-nums}
th,td{text-align:left;padding:.5rem .75rem;border-bottom:1px solid var(--line);vertical-align:top}
th{font-size:.75rem;letter-spacing:.05em;text-transform:uppercase;color:var(--muted);font-weight:700}
tbody tr:last-child td{border-bottom:0}
.note{font-size:.85rem;color:var(--muted)}
footer{font-size:.8rem;color:var(--muted)}
a{color:var(--flint)}
:focus-visible{outline:2px solid var(--ochre);outline-offset:2px}
"""
body = "".join(parts)
page = f"""<title>Kindling Sound and Speech</title>
<link rel="preconnect" href="https://fonts.googleapis.com"><link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Atkinson+Hyperlegible:wght@400;700&family=Gentium+Book+Plus:wght@400;700&display=swap">
<style>{CSS}</style>
<main>
<header><p class="eyebrow">Pre-test B74 and B76 · listening page</p><h1>Struck stone, bone flutes and an invented tongue</h1>
<p class="lede">Every sound here is made from numbers, not recordings: what things are made of, their shape and size, and how they are struck. Your ear makes the final choice. For each section, tell us which version sounds right, or that neither does.</p></header>
{body}
<footer>Clips are Ogg Opus at 24 to 64 kbps, and each comparison is matched for loudness. Material values are plausible stand-ins, not sourced. Made by the B74 and B76 pre-test; code in pretests/b74-b76-sound-speech.</footer>
</main>
"""
open(f"{HERE}/listen.html", "w", encoding="utf-8").write(page)
print(f"listen.html: {len(page.encode()) / 1e6:.2f} MB")
print(json.dumps({k: summary[k] for k in ("sounds", "mixer", "speech", "kept") if k in summary}, indent=1, ensure_ascii=False)[:3000])
