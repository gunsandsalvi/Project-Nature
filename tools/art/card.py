"""The target card's bands for each moment (A5.5), measured from the pictures you chose. Each data/base/card/<moment>
file names its pictures in art/targets/; this measures them with `kindling look card` and writes the moment's bands,
each from the lowest to the highest of its pictures, widened by the slack in data/base/tuning/card.toml.

    python3 tools/art/card.py            measure the pictures and write each moment's bands
    python3 tools/art/card.py --check    each picture's alarms against its moment; fails if any is red

Implements PRE-01, see A5.5.
"""

import math
import os
import sys
import tomllib

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import look  # noqa: E402

ROOT = look.ROOT
DATA = look.DATA
MOMENTS = os.path.join(DATA, "base", "card")
TARGETS = os.path.join(ROOT, "art", "targets")
# Each band: the card's statistic, its slack's field in tuning/card.toml, and whether it is written as a ratio (in
# percent of OKLab's scale or of the frame, to a tenth) or a whole number.
BANDS = [
    ("lightness", "lightness_slack", True),
    ("dark", "dark_slack", True),
    ("lights_hue", "lights_hue_slack", False),
    ("lights", "yellowness_slack", True),
    ("shade", "yellowness_slack", True),
    ("green", "green_slack", True),
    ("things", "things_slack", False),
    ("texture", "contrast_slack", True),
    ("masses", "contrast_slack", True),
]
SHARES = {"lightness", "dark", "green", "texture", "masses"}  # never below 0 or above 100


def amount(value):
    """A tuning value as the card's number: "3%" is 3, and a whole number is itself."""
    return float(value[:-1]) if isinstance(value, str) else float(value)


def picture(name):
    return np.asarray(Image.open(os.path.join(TARGETS, name + ".webp")).convert("RGB"))


def arc(hues):
    """The shortest arc of the circle holding every hue, as (start, end) in degrees, going round from start."""
    hues = sorted(h % 360.0 for h in hues)
    gaps = [((hues[(i + 1) % len(hues)] - hues[i]) % 360.0, i) for i in range(len(hues))]
    widest, i = max(gaps)
    if len(hues) == 1:
        return hues[0], hues[0]
    return hues[(i + 1) % len(hues)], hues[i]


def band(name, values, slack, ratio):
    """A band's low and high as written in a moment's file."""
    if name == "lights_hue":
        start, end = arc(values)
        if (end - start) % 360.0 + 2 * slack >= 359.0:
            return 0, 359
        return math.floor((start - slack) % 360.0), math.ceil((end + slack) % 360.0) % 360
    low, high = min(values) - slack, max(values) + slack
    if name in SHARES:
        low, high = max(low, 0.0), min(high, 100.0)
    if ratio:
        return f'"{math.floor(low * 10) / 10:.1f}%"', f'"{math.ceil(high * 10) / 10:.1f}%"'
    return max(math.floor(low), 0), math.ceil(high)


def moments():
    """Each moment's name, its file and its pictures."""
    out = []
    for file in sorted(os.listdir(MOMENTS)):
        if file.endswith(".toml"):
            path = os.path.join(MOMENTS, file)
            with open(path, "rb") as f:
                out.append((file[:-5], path, tomllib.load(f)["pictures"].split()))
    return out


def write():
    with open(os.path.join(DATA, "base", "tuning", "card.toml"), "rb") as f:
        tuning = tomllib.load(f)
    for name, path, pictures in moments():
        cards = [look.card(picture(p))[0] for p in pictures]
        with open(path) as f:
            head = [line for line in f.read().splitlines() if line.startswith("#") or line.startswith("pictures")]
        lines = head + [""]
        for stat, slack, ratio in BANDS:
            low, high = band(stat, [c[stat] for c in cards], amount(tuning[slack]), ratio)
            lines += [f"{stat}_low = {low}", f"{stat}_high = {high}"]
        with open(path, "w") as f:
            f.write("\n".join(lines) + "\n")
        print(f"{name}: {len(pictures)} picture{'s' if len(pictures) > 1 else ''} measured")


def check():
    red = 0
    for name, _, pictures in moments():
        for p in pictures:
            values, alarms = look.card(picture(p), name)
            off = [f"{k} {values[k]:.1f} {a}" for k, a in alarms.items() if a != "green"]
            red += sum(a == "red" for a in alarms.values())
            print(f"{name} {p}: {', '.join(off) if off else 'all green'}")
    return red


def main(argv):
    if argv == ["--check"]:
        return 1 if check() else 0
    if argv:
        print(__doc__)
        return 2
    write()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
