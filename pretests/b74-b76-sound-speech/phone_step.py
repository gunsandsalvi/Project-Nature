#!/usr/bin/env python3
"""B74 drums, round 2 (SND-02, SND-06): a last step before the phone's own speaker.

  phone_step.py CLIPS_DIR   writes the drum and camp clips through the step into CLIPS_DIR,
                            and the measurements to results/phone-step.json

The step, in the order a phone would run it on the finished mix:
  1. a high-pass at 150 Hz, since the speaker can't play below that and those notes would
     only use up the limiter's room;
  2. phone bass (optional): overtones 2 to 5 of everything below 200 Hz, kept to 250-1,500 Hz,
     so the ear still hears the deep note;
  3. a gain, then a limiter that looks 2 ms ahead and lets go over 80 ms, so a strike's first
     instant is held back and the rest of it can play louder.
Loudness is measured through a stand-in for the phone's speaker (fourth-order roll-off below
about 350 Hz), with the standard K-weighting, as the mean of the loudest fifth of 400 ms windows.
"""
import json, os, sys, wave
import numpy as np

SR = 48_000
HERE = os.path.dirname(os.path.abspath(__file__))
CEILING_DB = -1.0
MAX_LIMIT_DB = 12.0  # the decision rule's limit on limiting


def read(path):
    with wave.open(path) as w:
        ch = w.getnchannels()
        x = np.frombuffer(w.readframes(w.getnframes()), dtype="<i2").astype(np.float64) / 32768
    return x.reshape(-1, ch)


def write(path, x):
    x = np.clip(x, -1.0, 1.0)
    with wave.open(path, "wb") as w:
        w.setnchannels(x.shape[1])
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes((x * 32767).astype("<i2").tobytes())


# ---------- filters (RBJ biquads) ----------
def _coefs(kind, f, q=0.7071, gain_db=0.0):
    w = 2 * np.pi * f / SR
    c, s = np.cos(w), np.sin(w)
    al = s / (2 * q)
    if kind == "lp":
        b, a = [(1 - c) / 2, 1 - c, (1 - c) / 2], [1 + al, -2 * c, 1 - al]
    elif kind == "hp":
        b, a = [(1 + c) / 2, -(1 + c), (1 + c) / 2], [1 + al, -2 * c, 1 - al]
    else:  # high shelf
        A = 10 ** (gain_db / 40)
        r = 2 * np.sqrt(A) * al
        b = [A * ((A + 1) + (A - 1) * c + r), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - r)]
        a = [(A + 1) - (A - 1) * c + r, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - r]
    return np.array(b) / a[0], np.array(a) / a[0]


def iir(x, kind, f, q=0.7071, gain_db=0.0):
    """Causal biquad, sample by sample, as the phone would run it (x: samples x channels)."""
    b, a = _coefs(kind, f, q, gain_db)
    y = np.empty_like(x)
    x1 = x2 = y1 = y2 = np.zeros(x.shape[1])
    for i in range(len(x)):
        v = x[i]
        o = b[0] * v + b[1] * x1 + b[2] * x2 - a[1] * y1 - a[2] * y2
        x2, x1, y2, y1 = x1, v, y1, o
        y[i] = o
    return y


def response(kind, f, freqs, q=0.7071, gain_db=0.0):
    b, a = _coefs(kind, f, q, gain_db)
    z = np.exp(-2j * np.pi * freqs / SR)
    return (b[0] + b[1] * z + b[2] * z * z) / (1 + a[1] * z + a[2] * z * z)


def through(x, stages):
    """Filter in the frequency domain (for measuring only), padded so nothing wraps."""
    n = len(x) + SR
    X = np.fft.rfft(x, n=n, axis=0)
    freqs = np.fft.rfftfreq(n, 1 / SR)
    H = np.ones_like(freqs, dtype=complex)
    for st in stages:
        H *= response(*st, freqs) if len(st) == 2 else response(st[0], st[1], freqs, *st[2:])
    return np.fft.irfft(X * H[:, None], n=n, axis=0)[: len(x)]


PHONE = [("hp", 300), ("hp", 400), ("lp", 14_000)]
K_WEIGHT = [("shelf", 1681.97, 0.7071, 4.0), ("hp", 38.1, 0.5)]


def loudness(x, phone=True):
    y = through(x, (PHONE if phone else []) + K_WEIGHT)
    p = (y ** 2).sum(axis=1)
    win, hop = int(0.4 * SR), int(0.1 * SR)
    ms = np.array([p[i:i + win].mean() for i in range(0, max(1, len(p) - win), hop)])
    top = np.sort(ms)[-max(1, len(ms) // 5):]
    return float(-0.691 + 10 * np.log10(top.mean() + 1e-20))


def share_below(x, f_cut=350.0):
    X = (np.abs(np.fft.rfft(x, axis=0)) ** 2).sum(axis=1)
    f = np.fft.rfftfreq(len(x), 1 / SR)
    return float(X[f < f_cut].sum() / X.sum())


# ---------- the step ----------
def phone_bass(x):
    """Overtones 2 to 5 of the band below 200 Hz, kept to 250-1,500 Hz. Chebyshev polynomials
    of the band divided by its own envelope turn each deep note into its overtones at a level
    that follows the note's."""
    low = iir(iir(x, "lp", 200), "lp", 200)
    env = np.empty_like(low)
    e = np.full(x.shape[1], 1e-9)
    att, rel = np.exp(-1 / (0.0005 * SR)), np.exp(-1 / (0.03 * SR))
    for i, v in enumerate(np.abs(low)):
        e = np.where(v > e, att * e + (1 - att) * v, rel * e + (1 - rel) * v)
        env[i] = e
    u = np.clip(low / np.maximum(env, 1e-9), -1, 1)
    t2 = 2 * u * u - 1
    t3 = 4 * u ** 3 - 3 * u
    t4 = 8 * u ** 4 - 8 * u * u + 1
    t5 = 16 * u ** 5 - 20 * u ** 3 + 5 * u
    h = env * (1.0 * t2 + 0.8 * t3 + 0.6 * t4 + 0.45 * t5)
    for kind, f in (("hp", 250), ("hp", 250), ("lp", 1500), ("lp", 1500)):
        h = iir(h, kind, f)
    return h * 0.7


def limit(x, gain_db):
    """Gain, then a 2 ms look-ahead limiter at the ceiling with an 80 ms release.
    Returns the output and the deepest gain reduction in dB."""
    y = x * 10 ** (gain_db / 20)
    ceiling = 10 ** (CEILING_DB / 20)
    peak = np.abs(y).max(axis=1)
    need = np.minimum(1.0, ceiling / np.maximum(peak, 1e-12))
    L = int(0.002 * SR)
    padded = np.concatenate([need, np.ones(L - 1)])
    ahead = np.lib.stride_tricks.sliding_window_view(padded, L).min(axis=1)
    rel = np.exp(-1 / (0.08 * SR))
    held = np.empty_like(ahead)
    g = 1.0
    for i, a in enumerate(ahead):
        g = a if a < g else rel * g + (1 - rel) * a
        held[i] = g
    # averaging the held gain over the look-ahead keeps it smooth and never above what's needed
    c = np.concatenate([[0.0], np.cumsum(np.concatenate([np.ones(L - 1), held]))])
    smooth = (c[L:] - c[:-L]) / L
    out = y * smooth[:, None]
    return out, float(-20 * np.log10(smooth.min()))


def fit(x, target):
    """How loud x can get on the phone stand-in within the limit on limiting, and the gain that
    brings it to `target` (or as close as that limit allows)."""
    lo, hi = -30.0, 60.0
    for _ in range(22):
        mid = (lo + hi) / 2
        if limit(x, mid)[1] > MAX_LIMIT_DB:
            hi = mid
        else:
            lo = mid
    top_gain = lo
    most = loudness(limit(x, top_gain)[0])
    gain = top_gain
    if most > target:
        lo, hi = -30.0, top_gain
        for _ in range(22):
            mid = (lo + hi) / 2
            if loudness(limit(x, mid)[0]) > target:
                hi = mid
            else:
                lo = mid
        gain = lo
    y, gr = limit(x, gain)
    return y, gain, gr, most


def page_gain(x):
    """What build_page.py does to a lone clip: active RMS to -20 dB unless its peak stops it."""
    a = x.mean(axis=1)
    n = int(0.02 * SR)
    p = np.array([np.mean(a[i:i + n] ** 2) for i in range(0, len(a) - n, n)]) + 1e-20
    rms = 10 * np.log10(p[p > p.max() * 1e-3].mean())
    peak = 20 * np.log10(np.abs(x).max() + 1e-12)
    return min(-20.0, rms + (CEILING_DB - peak)) - rms


def main():
    clips = sys.argv[1]
    res = {"ceiling_db": CEILING_DB, "max_limit_db": MAX_LIMIT_DB, "clips": {}}
    flutes = []
    for name in ("flute-a", "flute-b"):
        x = read(f"{clips}/{name}.wav")
        x = x * 10 ** (page_gain(x) / 20)
        flutes.append(loudness(x))
        res["clips"][name] = {"as_before_phone_lu": round(flutes[-1], 1)}
    target = float(np.mean(flutes))
    res["target_phone_lu"] = round(target, 1)
    for name in ("drum-small", "drum-large", "drum-small-ring", "drum-large-ring", "camp-noise"):
        x = read(f"{clips}/{name}.wav")
        before = x * 10 ** (page_gain(x) / 20)
        hp = iir(iir(x, "hp", 150), "hp", 150)
        row = {"as_before_phone_lu": round(loudness(before), 1), "as_before_full_lu": round(loudness(before, phone=False), 1),
               "share_below_350hz": round(share_below(x), 3)}
        for tag, pre in (("louder", hp), ("bass", hp + phone_bass(x))):
            y, gain, gr, most = fit(pre, target)
            write(f"{clips}/{name}-{tag}.wav", y)
            row[tag] = {"phone_lu": round(loudness(y), 1), "gain_db": round(gain, 1), "deepest_limit_db": round(gr, 1),
                        "most_phone_lu": round(most, 1), "below_flutes_db": round(target - loudness(y), 1)}
        res["clips"][name] = row
        print(name, json.dumps(row), flush=True)
    # the rules: within 3 dB of the flutes at most 12 dB of limiting; bass worth at least 6 dB more
    for law, sfx in (("as_before", ""), ("rings_longer", "-ring")):
        d = {k: res["clips"][f"drum-{k}{sfx}"] for k in ("small", "large")}
        bass_extra = round(d["large"]["bass"]["most_phone_lu"] - d["large"]["louder"]["most_phone_lu"], 1)
        step = "bass" if bass_extra >= 6.0 else "louder"
        res[law] = {"bass_extra_db_large": bass_extra, "bass_passes": bass_extra >= 6.0, "step_used": step,
                    "passes": all(target - v[step]["most_phone_lu"] <= 3.0 for v in d.values())}
    json.dump(res, open(f"{HERE}/results/phone-step.json", "w"), indent=1)
    print(json.dumps({k: res[k] for k in ("target_phone_lu", "as_before", "rings_longer")}))


if __name__ == "__main__":
    main()
