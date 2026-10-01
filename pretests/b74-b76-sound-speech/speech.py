#!/usr/bin/env python3
"""B76 pre-test (SND-03, CUL-17): speak a tiny invented language two ways.

S1: the espeak-ng formant synthesiser, fed the language's phonemes directly ([[...]]
    phoneme input, Welsh phoneme table, which has all 20 sounds).
S2: Piper neural voices (ONNX), fed IPA phoneme ids directly, with no text front end:
    one trained on English (904 speakers), one trained on Welsh (one speaker).

The voices only render sentences given to them; they choose nothing (PRN-06).

Usage (the venv from setup.sh):  speech.py clips OUT_DIR | bench | kept OUT_JSON
"""
import json, os, re, resource, subprocess, sys, tempfile, time, wave
from collections import Counter

CACHE = os.environ.get("CACHE", "/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/cache")
VOICES = f"{CACHE}/b76/voices"
HERE = os.path.dirname(os.path.abspath(__file__))

# ---------- the language (CUL-17 stand-in: 15 consonants, 5 vowels, (C)V(N) syllables) ----------
CONSONANTS = "p t k q ʔ m n ŋ s x ɬ l r w j".split()
VOWELS = "a e i o u".split()
LEXICON = {
    "ɬami": "fire", "tuku": "warm", "ŋiro": "child", "wa": "come", "ni": "here",
    "qaxu": "wolf", "lemu": "river", "-se": "at, by", "ʔasa": "danger", "xepi": "where",
    "ʔisan": "flint", "je": "(question)", "nuŋ": "we", "peni": "tonight", "xoma": "meat", "ɬuke": "eat",
}
# (sentence, word-by-word, English). Word order: subject, time, object, verb.
SENTENCES = [
    ("ɬami tuku.", "fire warm", "The fire is warm."),
    ("ŋiro, wa ni!", "child, come here", "Child, come here!"),
    ("qaxu lemuse, ʔasa!", "wolf river-at, danger", "Wolves by the river, watch out!"),
    ("xepi ʔisan je?", "where flint (question)", "Where is the flint?"),
    ("nuŋ peni xoma ɬuke.", "we tonight meat eat", "We eat meat tonight."),
]
ESPEAK = {"ʔ": "?", "ŋ": "N", "ɬ": "l#"}  # all other sounds use their own letter


def words(sentence):
    """[(word, punctuation after it)]"""
    return re.findall(r"([^\s,.!?]+)([,.!?]?)", sentence)


def stressed(word, mark):
    """Stress on the first syllable: the mark goes right before the first vowel (espeak style)."""
    out, done = [], False
    for ch in word:
        if ch in VOWELS and not done:
            out.append(mark)
            done = True
        out.append(ch)
    return out


def espeak_input(sentence):
    """S1: espeak-ng phoneme input, one [[...]] group per clause, punctuation outside."""
    parts, group = [], []
    for w, p in words(sentence):
        group.append("".join(ESPEAK.get(c, c) for c in stressed(w, "'")))
        if p:
            parts.append("[[" + " ".join(group) + "]]" + p)
            group = []
    if group:
        parts.append("[[" + " ".join(group) + "]]")
    return " ".join(parts)


def ipa(sentence):
    """S2: IPA as Piper's training data spelled it (espeak style stress marks)."""
    out = []
    for i, (w, p) in enumerate(words(sentence)):
        if i:
            out.append(" ")
        out += stressed(w, "ˈ") + ([p] if p else [])
    return out


# ---------- S1: espeak-ng ----------
# Three people: a man, a woman, an old man (Klatt-style variant), by pitch and rate.
S1_PEOPLE = [("man", "cy+m3", 38, 145), ("woman", "cy+f2", 62, 160), ("elder", "cy+klatt2", 28, 125)]


def espeak_wav(text, variant, pitch, rate, path):
    subprocess.run(["espeak-ng", "-v", variant, "-p", str(pitch), "-s", str(rate), "-w", path, text], check=True)


# ---------- S2: Piper ----------
class Piper:
    def __init__(self, name, threads=1):
        import numpy as np, onnxruntime as ort
        self.np = np
        self.cfg = json.load(open(f"{VOICES}/{name}.onnx.json"))
        so = ort.SessionOptions()
        so.intra_op_num_threads = threads
        so.inter_op_num_threads = 1
        t0 = time.perf_counter()
        self.sess = ort.InferenceSession(f"{VOICES}/{name}.onnx", so, providers=["CPUExecutionProvider"])
        self.load_s = time.perf_counter() - t0
        self.sr = self.cfg["audio"]["sample_rate"]
        self.multi = self.cfg.get("num_speakers", 1) > 1
        self.size_mb = os.path.getsize(f"{VOICES}/{name}.onnx") / 1e6

    def ids(self, phonemes):
        m = self.cfg["phoneme_id_map"]
        ids = list(m["^"])
        for ph in phonemes:
            ids += m[ph] + m["_"]
        return ids + m["$"]

    def speak(self, phonemes, speaker=0, length=1.0):
        np = self.np
        inf = self.cfg["inference"]
        ids = self.ids(phonemes)
        feed = {
            "input": np.array([ids], dtype=np.int64),
            "input_lengths": np.array([len(ids)], dtype=np.int64),
            "scales": np.array([inf["noise_scale"], length, inf["noise_w"]], dtype=np.float32),
        }
        if self.multi:
            feed["sid"] = np.array([speaker], dtype=np.int64)
        return self.sess.run(None, feed)[0].squeeze()


def write_wav(path, audio, sr):
    import numpy as np
    a = np.asarray(audio, dtype=np.float32)
    a = a / max(1e-9, float(np.abs(a).max())) * 0.7
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sr)
        w.writeframes((a * 32767).astype("<i2").tobytes())


def wav_seconds(path):
    with wave.open(path) as w:
        return w.getnframes() / w.getframerate()


def median_f0(audio, sr):
    """Rough median pitch of voiced frames by autocorrelation (to pick distinct speakers)."""
    import numpy as np
    a = np.asarray(audio, dtype=np.float64)
    fr, hop = int(0.04 * sr), int(0.01 * sr)
    f0s = []
    for s in range(0, len(a) - fr, hop):
        x = a[s:s + fr] - a[s:s + fr].mean()
        if np.sqrt((x * x).mean()) < 0.02 * np.abs(a).max():
            continue
        ac = np.correlate(x, x, "full")[fr - 1:]
        lo, hi = int(sr / 400), int(sr / 70)
        k = lo + int(np.argmax(ac[lo:hi]))
        if ac[k] > 0.5 * ac[0]:
            f0s.append(sr / k)
    return float(np.median(f0s)) if f0s else 0.0


def pick_speakers(en, n_try=48):
    """Three LibriTTS speakers far apart in pitch: lowest, middle, highest median pitch."""
    ph = ipa(SENTENCES[0][0])
    f0 = sorted((median_f0(en.speak(ph, s), en.sr), s) for s in range(n_try))
    f0 = [x for x in f0 if x[0] > 0]
    return [(f0[0][1], f0[0][0]), (f0[len(f0) // 2][1], f0[len(f0) // 2][0]), (f0[-1][1], f0[-1][0])]


EN = "en_US-libritts_r-medium"
CY = "cy_GB-gwryw_gogleddol-medium"


def clips(out):
    os.makedirs(out, exist_ok=True)
    en, cy = Piper(EN, threads=4), Piper(CY, threads=4)
    spk = pick_speakers(en)
    people = [("low voice", spk[0][0], 1.1), ("middle voice", spk[1][0], 1.0), ("high voice", spk[2][0], 0.92)]
    meta = {"speakers": [{"label": p[0], "id": p[1], "median_f0_hz": round(s[1])} for p, s in zip(people, spk)], "clips": []}
    for i, (sent, gloss, eng) in enumerate(SENTENCES):
        who, variant, pitch, rate = S1_PEOPLE[i % 3]
        espeak_wav(espeak_input(sent), variant, pitch, rate, f"{out}/s1-espeak-{i + 1}.wav")
        label, sid, length = people[i % 3]
        write_wav(f"{out}/s2-english-{i + 1}.wav", en.speak(ipa(sent), sid, length), en.sr)
        write_wav(f"{out}/s2-welsh-{i + 1}.wav", cy.speak(ipa(sent), 0, [1.0, 1.15, 0.9][i % 3]), cy.sr)
        meta["clips"].append({"n": i + 1, "s1": who, "s2_english": label, "espeak_input": espeak_input(sent), "ipa": "".join(ipa(sent))})
    # the same sentence by three people, each way
    import numpy as np
    sent = SENTENCES[1][0]
    gap = lambda sr: np.zeros(int(0.5 * sr), dtype=np.float32)
    parts = []
    for who, variant, pitch, rate in S1_PEOPLE:
        with tempfile.NamedTemporaryFile(suffix=".wav") as t:
            espeak_wav(espeak_input(sent), variant, pitch, rate, t.name)
            with wave.open(t.name) as w:
                sr1 = w.getframerate()
                parts += [np.frombuffer(w.readframes(w.getnframes()), dtype="<i2").astype(np.float32) / 32767, gap(sr1)]
    write_wav(f"{out}/s1-espeak-people.wav", np.concatenate(parts), sr1)
    parts = []
    for label, sid, length in people:
        a = en.speak(ipa(sent), sid, length)
        parts += [a / np.abs(a).max(), gap(en.sr)]
    write_wav(f"{out}/s2-english-people.wav", np.concatenate(parts), en.sr)
    json.dump(meta, open(f"{out}/speech-meta.json", "w"), ensure_ascii=False, indent=1)
    print(json.dumps(meta, ensure_ascii=False))


def bench():
    """Cost per second of speech on one core: CPU seconds / audio seconds."""
    res = {}
    # S1: one espeak-ng process per person, all five sentences, pinned to one core
    with tempfile.TemporaryDirectory() as d:
        cpu = audio = 0.0
        for who, variant, pitch, rate in S1_PEOPLE:
            text = " ".join(espeak_input(s[0]) for s in SENTENCES)
            r0 = resource.getrusage(resource.RUSAGE_CHILDREN)
            subprocess.run(["taskset", "-c", "0", "espeak-ng", "-v", variant, "-p", str(pitch), "-s", str(rate), "-w", f"{d}/a.wav", text], check=True)
            r1 = resource.getrusage(resource.RUSAGE_CHILDREN)
            cpu += (r1.ru_utime - r0.ru_utime) + (r1.ru_stime - r0.ru_stime)
            audio += wav_seconds(f"{d}/a.wav")
        res["s1_espeak"] = {"core_share": cpu / audio, "audio_s": audio}
    # S2: one inference thread
    os.sched_setaffinity(0, {0})
    for name, key in [(EN, "s2_english"), (CY, "s2_welsh")]:
        p = Piper(name, threads=1)
        p.speak(ipa(SENTENCES[0][0]))  # warm-up
        cpu = audio = 0.0
        for k in range(3):
            for s in SENTENCES:
                c0 = time.process_time()
                a = p.speak(ipa(s[0]), speaker=k)
                cpu += time.process_time() - c0
                audio += len(a) / p.sr
        res[key] = {"core_share": cpu / audio, "audio_s": audio, "load_s": p.load_s, "model_mb": p.size_mb}
    res["max_rss_mb"] = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 1024
    print(json.dumps(res))


# ---------- sounds kept ----------
def units(ipa_text):
    """Split espeak IPA into sound units: drop stress and length marks, keep diphthongs
    (a vowel then ɪ or ʊ) and affricates together."""
    t = re.sub(r"[ˈˌː\s\-,.!?;:'\"()\[\]0-9]", " ", ipa_text)
    out = []
    for w in t.split():
        i = 0
        while i < len(w):
            two = w[i:i + 2]
            if len(two) == 2 and (two in ("tʃ", "dʒ") or (two[0] in "aeiouæɑɐɔəɛɜɪʊʌɒ" and two[1] in "ɪʊ")):
                out.append(two)
                i += 2
            else:
                out.append(w[i])
                i += 1
    return out


def espeak_ipa(voice, text):
    return subprocess.run(["espeak-ng", "-v", voice, "-q", "--ipa"], input=text, capture_output=True, text=True, check=True).stdout


def kept(out_json):
    sounds = CONSONANTS + VOWELS
    res = {"sounds": sounds}
    # S1: does the phoneme table give each sound its own symbol? (Welsh table; English for contrast)
    for voice in ("cy", "en-us"):
        got = {}
        for s in sounds:
            m = ESPEAK.get(s, s)
            probe = f"[[{m}'a]]" if s in CONSONANTS else f"[[p'{m}]]"
            got[s] = espeak_ipa(voice, probe).strip()
        res[f"s1_espeak_{voice}"] = {s: {"ipa": g, "kept": s in g.replace("ˈ", "")} for s, g in got.items()}
    # S2: did the voice's training language use each sound on its own? (share of all units)
    corpora = {"english": (open(f"{HERE}/../../PROJECT.md", encoding="utf-8").read(), "en-us"),
               "welsh": (open(f"{CACHE}/b76/corpus-cy.txt", encoding="utf-8").read(), "cy")}
    for lang, (text, voice) in corpora.items():
        text = re.sub(r"\{\{.*?\}\}|\[\[[^\]|]*\||<[^>]+>|https?://\S+|[`*#|=\[\]{}_]", " ", text, flags=re.S)
        c = Counter(units(espeak_ipa(voice, text)))
        total = sum(c.values())
        res[f"s2_{lang}"] = {s: {"share_pct": round(100 * c[s] / total, 3), "kept": 100 * c[s] / total >= 0.1} for s in sounds}
        res[f"s2_{lang}"]["_units"] = total
        res[f"s2_{lang}"]["_top"] = [u for u, _ in c.most_common(45)]
    for k in ("s1_espeak_cy", "s1_espeak_en-us", "s2_english", "s2_welsh"):
        res[f"{k}_kept"] = sum(1 for s in sounds if res[k][s]["kept"])
    json.dump(res, open(out_json, "w"), ensure_ascii=False, indent=1)
    print({k: v for k, v in res.items() if k.endswith("_kept")})


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd == "clips":
        clips(sys.argv[2])
    elif cmd == "bench":
        bench()
    elif cmd == "kept":
        kept(sys.argv[2])
    elif cmd == "show":
        for s in SENTENCES:
            print(s[0], "|", espeak_input(s[0]), "|", "".join(ipa(s[0])))
    else:
        print(__doc__)
