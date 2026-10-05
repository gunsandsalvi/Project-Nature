"""P14's murmur bank (IMPLEMENTATION α0.7c, SND-03): every syllable of the stand-in languages' sounds spoken once,
flat, by two base voices, a woman's and a man's, and the times of each voice's pulses marked, as the cloud will
render the game's bank once from the voice you choose by ear and ship it, so no voice model runs on the phone.

The voices here are stand-ins, open voices of the Piper speech engine: LJ Speech for the woman (its recordings are
in the public domain) and Joe for the man (CC0). The phone strings the syllables into talk and shifts them for age,
build and feeling (prototypes/sound/src/sound.cpp).

    ~/.cache/kindling/piper-venv/bin/python prototypes/sound/bank.py    # about a minute; downloads 130 MB once

It needs a Python with piper-tts 1.8.0 and numpy, made once with:

    python3 -m venv ~/.cache/kindling/piper-venv
    ~/.cache/kindling/piper-venv/bin/pip install piper-tts==1.8.0 numpy

Writes prototypes/app/sound/bank/<voice>.bank, whose layout read_bank() in sound.cpp reads. Pre-production code
(research 00).
"""

import hashlib
import struct
import sys
import urllib.request
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
OUT = ROOT / "prototypes" / "app" / "sound" / "bank"
CACHE = Path.home() / ".cache" / "kindling" / "piper"
SOURCE = "https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US"
# each base voice: its model, and the files' sha256, so a changed download is caught
VOICES = {
    "woman": (
        "ljspeech/medium/en_US-ljspeech-medium",
        "6f52a751e2349abe7a76735eb09dc1875298c77ea2342ffd2fef79ff81b87f22",
        "141d612cc0a95ed7efc1ca936b845c2364967f2e9217c5dbfcf69fc4d6c65860",
    ),
    "man": (
        "joe/medium/en_US-joe-medium",
        "58afce0321b8d9c46d7cdf9c16500cc55a793b4220212dba6b70fb788b3baf06",
        "3d6d5410b3795cb1950595247ef8f06190719e6fdbfa3a2356d8ec368e1aad33",
    ),
}
RATE = 22050
# the stand-in languages' sounds (CUL-17), each written as the voices' own phonemes: a language takes some of them
CONSONANTS = {
    "k": "k",
    "t": "t",
    "p": "p",
    "d": "d",
    "m": "m",
    "n": "n",
    "s": "s",
    "h": "h",
    "l": "l",
    "r": "ɹ",
    "y": "j",
}
VOWELS = {"a": "ɑː", "e": "ɛ", "i": "iː", "o": "ɔː", "u": "uː"}
LONGEST = 0.30  # seconds a syllable's vowel is kept; the phone stretches it when a feeling asks
HOP = 110  # samples between the steady marks where the voice is unvoiced: 5 ms
MAGIC = b"KDBANK1\0"


def syllables():
    """Every syllable of the sounds: each consonant before each vowel, and each vowel alone."""
    out = [(c + v, CONSONANTS[c] + "ˈ" + VOWELS[v]) for c in CONSONANTS for v in VOWELS]
    return out + [(v, "ˈ" + VOWELS[v]) for v in VOWELS]


def fetch(name, sums):
    """A voice's model and its settings, downloaded once into the cache and checked."""
    CACHE.mkdir(parents=True, exist_ok=True)
    paths = []
    for ext, sha in zip((".onnx", ".onnx.json"), sums, strict=True):
        path = CACHE / (Path(name).name + ext)
        if not path.exists():
            urllib.request.urlretrieve(f"{SOURCE}/{name}{ext}", path)
        if hashlib.sha256(path.read_bytes()).hexdigest() != sha:
            sys.exit(f"bank: {path} is not the voice pinned here")
        paths.append(path)
    return paths[0]


def render(voice, phonemes):
    """A syllable spoken by the voice."""
    from piper import SynthesisConfig

    ids = voice.phonemes_to_ids(list(phonemes))
    # little noise: a steady, plain reading
    config = SynthesisConfig(noise_scale=0.4, noise_w_scale=0.3, normalize_audio=False)
    return np.asarray(voice.phoneme_ids_to_audio(ids, config), dtype=np.float64)


def envelope(x, width):
    """The signal's loudness over a moving window, as RMS."""
    kernel = np.ones(width) / width
    return np.sqrt(np.convolve(x * x, kernel, mode="same"))


def trim(x):
    """The syllable without the silence around it, its vowel cut to LONGEST with a short fade; and where its vowel
    begins: where it first grows to half its loudest, as a vowel is louder than the consonant before it. (The voice's
    own alignment puts it up to a tenth of a second late.)"""
    env = envelope(x, 220)
    loud = np.nonzero(env > env.max() * 10 ** (-45 / 20))[0]
    start = max(0, int(loud[0]) - 110)
    end = min(len(x), int(loud[-1]) + 110)
    smooth = envelope(x, 551)
    vowel = int(np.nonzero(smooth >= 0.5 * smooth.max())[0][0])
    vowel = min(max(vowel, start), end - 1)
    keep = vowel + int(LONGEST * RATE)
    if end > keep:
        end = keep
        fade = int(0.04 * RATE)
        x = x.copy()
        x[end - fade : end] *= 0.5 + 0.5 * np.cos(np.linspace(0, np.pi, fade))
    return x[start:end], vowel - start


def level(x, vowel):
    """The syllable as loud as the others in its vowel's loud part, its peaks kept below clipping."""
    env = envelope(x, 551)
    part = x[vowel:][env[vowel:] >= 0.3 * env.max()]
    x = x * (0.1 / max(float(np.sqrt(np.mean(part**2))), 1e-6))
    peak = float(np.abs(x).max())
    return x * (0.9 / peak) if peak > 0.9 else x


def frames(x, rate=RATE):
    """For each 5 ms frame, of 30 ms: whether it is voiced, and its period in samples (0 where unvoiced)."""
    width, lo, hi = int(0.03 * rate), rate // 400, rate // 60
    energy, voiced, period = [], [], []
    for at in range(0, len(x), HOP):
        f = x[max(0, at - width // 2) : at + width // 2]
        f = f - f.mean()
        energy.append(float(np.dot(f, f)))
        best, lag = 0.0, 0
        if len(f) > hi + 10:
            for k in range(lo, hi):
                a, b = f[:-k], f[k:]
                r = float(np.dot(a, b)) / (np.sqrt(float(np.dot(a, a)) * float(np.dot(b, b))) + 1e-12)
                if r > best:
                    best, lag = r, k
        voiced.append(best)
        period.append(lag)
    top = max(energy) or 1.0
    on = [(r > 0.5 and e > 0.01 * top) for r, e in zip(voiced, energy, strict=True)]
    # a lone frame is not a voice
    on = [on[i] and (on[max(0, i - 1)] or on[min(len(on) - 1, i + 1)]) for i in range(len(on))]
    return on, [p if v else 0 for p, v in zip(period, on, strict=True)]


def zff(x, period):
    """Zero-frequency filtering (Murty and Yegnanarayana, 2008): the signal's differences through two resonators at
    zero frequency, their slow trend taken away three times; its rising zero crossings fall at the voice's pulses.
    The resonators lose a little each sample, so the trend stays small over a syllable."""
    y = np.diff(x, prepend=x[:1])
    r = 0.999
    for _ in range(2):
        out = np.empty_like(y)
        a = b = 0.0
        for i, v in enumerate(y):
            c = v + 2.0 * r * a - r * r * b
            out[i] = c
            b, a = a, c
        y = out
    width = int(1.5 * period) | 1
    kernel = np.ones(width) / width
    for _ in range(3):
        y = y - np.convolve(y, kernel, mode="same")
    return y


def marks(x):
    """The middle of each grain the phone cuts: each pulse of the voice where it is voiced, every HOP samples where it
    is not; with whether each is a pulse."""
    on, periods = frames(x)
    voiced_periods = [p for p in periods if p]
    out = []
    if voiced_periods:
        period = float(np.median(voiced_periods))
        z = zff(x, period)
        rising = np.nonzero((z[:-1] < 0) & (z[1:] >= 0))[0] + 1
        for at in rising:
            frame = min(len(on) - 1, int(round(at / HOP)))
            if on[frame] and (not out or at - out[-1][0] > 0.7 * period):
                out.append((int(at), 1))
    # steady marks through every stretch without pulses, kept clear of the pulses on either side
    pulses = [m for m, _ in out]
    edges = [-HOP] + pulses + [len(x) - 1 + HOP]
    steady = []
    for a, b in zip(edges, edges[1:], strict=False):
        if b - a > 2 * HOP:
            steady.extend((m, 0) for m in range(a + HOP, b - HOP // 2, HOP) if 0 <= m < len(x))
    return sorted(out + steady)


def pack(name, units):
    """The bank's bytes: its rate, the voice's middle pitch and each syllable (bank.py's layout, read_bank())."""
    periods = []
    for _, _, _, ms in units:
        pulses = [m for m, v in ms if v]
        periods += [b - a for a, b in zip(pulses, pulses[1:], strict=False) if b - a < RATE / 60]
    pitch = RATE / float(np.median(periods))
    out = bytearray(MAGIC)
    out += struct.pack("<IfI", RATE, pitch, len(units))
    for text, x, vowel, ms in units:
        pcm = np.clip(np.round(x * 32767), -32767, 32767).astype("<i2")
        out += struct.pack("<B", len(text)) + text.encode("ascii")
        out += struct.pack("<II", vowel, len(pcm)) + pcm.tobytes()
        out += struct.pack("<I", len(ms))
        out += np.array([m for m, _ in ms], dtype="<i4").tobytes()
        out += bytes(v for _, v in ms)
    print(f"bank: {name}, {len(units)} syllables, {sum(len(u[1]) for u in units) / RATE:.1f} s, pitch {pitch:.0f} Hz")
    return bytes(out)


def main():
    from piper import PiperVoice

    OUT.mkdir(parents=True, exist_ok=True)
    for name, (model, *sums) in VOICES.items():
        voice = PiperVoice.load(fetch(model, sums))
        units = []
        for text, phonemes in syllables():
            x, vowel = trim(render(voice, phonemes))
            x = level(x, vowel)
            units.append((text, x, vowel, marks(x)))
        (OUT / f"{name}.bank").write_bytes(pack(name, units))


if __name__ == "__main__":
    main()
