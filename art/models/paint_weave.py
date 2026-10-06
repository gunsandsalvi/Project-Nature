"""The basket's weave, drawn by code (A6.4): twined plant fibre, its stakes upright every 4 texture pixels and its
weft rows across them, so a basket's wrap of any whole step of 4 texture pixels (kit.lathe) is seamless.

    python3 art/models/paint_weave.py

Writes art/textures/basket_weave, a material with its levels and its record. Each row of weft is 4 texture pixels:
two strands twisting round each stake, their twist slanting one way in one row and the other way in the next, as
twining does, with a shadow under the row; each row takes its own shade, as strands gathered at different times do.
Band 1 is drawn again for its size, its rows of 2; the code reduction makes the levels below it.

Implements PRE-42 and PRE-46, see A6.4.
"""

import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "tools", "art"))
import levels  # noqa: E402
import match  # noqa: E402
import record  # noqa: E402
import texels  # noqa: E402

SIDE = 256
STAKE = np.array((150, 118, 74))
LIGHT = np.array((204, 168, 110))
MID = np.array((180, 144, 90))
LOW = np.array((158, 124, 76))
GAP = np.array((104, 80, 50))


# One stitch of twining between two stakes, 3 texture pixels wide and 3 tall, its light strand slanting down to the
# right (S); the next row's slants the other way (Z). 0 light, 1 mid, 2 low.
S = [[0, 1, 2], [1, 0, 1], [2, 1, 0]]
Z = [row[::-1] for row in S]


def weave(seed=5):
    rng = np.random.default_rng(seed)
    img = np.zeros((SIDE, SIDE, 3))
    tint = 1 + 0.05 * rng.standard_normal(SIDE // 4)  # each row its own shade
    tones = (LIGHT, MID, LOW)
    for y in range(SIDE):
        row, line = divmod(y, 4)
        stitch = S if row % 2 == 0 else Z
        for x in range(SIDE):
            k = x % 4
            if line == 3:  # the shadow under the row
                c = GAP * (0.9 if k == 0 else 1.0)
            elif k == 0:  # a stake, crossed by the row
                c = STAKE * (1.06 if line == 0 else 0.94 if line == 2 else 1.0)
            else:
                c = tones[stitch[line][k - 1]]
            img[y, x] = c * tint[row]
    return np.clip(np.round(img), 0, 255).astype(np.uint8)


def band1(seed=5):
    """The weave drawn again for band 1, never reduced: each row 2 texture pixels, its strands one line over the
    shadow under it, so the rows still read; the levels below it come from the code reduction."""
    rng = np.random.default_rng(seed)
    tint = 1 + 0.05 * rng.standard_normal(SIDE // 4)
    side = SIDE // 2
    img = np.zeros((side, side, 3))
    for y in range(side):
        img[y, :] = ((LIGHT + MID) / 2 if y % 2 == 0 else GAP) * tint[y // 2]
    return np.clip(np.round(img), 0, 255).astype(np.uint8)


def main():
    folder = os.path.join(ROOT, "art", "textures", "basket_weave")
    os.makedirs(folder, exist_ok=True)
    b0 = weave()
    drawn = [b0, match.fit_to_near(band1(), b0)[0]]
    made = levels.make(drawn, drawn[0])
    bands = []
    ways = [
        "drawn by code: stakes every 4 texture pixels, twined weft rows of 4 across them",
        "drawn by code for its band: rows of 2, the strands one line over the shadow under it, its colours fitted to "
        "band 0 by code",
    ]
    for n, t in enumerate(drawn + [s["level"] for s in made]):
        path = os.path.join(folder, f"b{n}.png")
        texels.save_png(t, path)
        b = {"level": n, "file": os.path.relpath(path, ROOT), "sha256": texels.sha256(path)}
        if n:
            b["made_from"] = bands[-1]["sha256"]
        if n < len(drawn):
            b["way"] = ways[n]
        else:
            step = made[n - len(drawn)]
            b["way"] = levels.way(step["marks"], step["most"])
            b["calibration"] = levels.calibration(step)
        bands.append(b)
    rec = {
        "about": "a basket's weave: twined plant fibre, stakes upright and weft rows across them",
        "route": "code",
        "tile_texels": SIDE,
        "texels_a_metre": 64,
        "first_band": 0,
        "sources": [],
        "original_sha256": [],
        "c2pa": [],
        "requests": [],
        "made": (
            "drawn by code (art/models/paint_weave.py): stakes every 4 texture pixels, so any wrap of whole steps of "
            "4 is seamless, and weft rows of 4, two strands twisting round each stake, slanting one way a row and the "
            "other the next, a shadow under each row, each row its own shade"
        ),
        "regrid_loss": "not re-gridded: drawn by code",
        "truth": (
            "art lane, 2026-10-06, looked at enlarged: twined basketry of plant fibre, as Mesolithic finds show "
            "(twined fish traps and baskets of willow and lime bast); no dyes, no coiled clay or later work"
        ),
        "approved": "waiting",
        "band": bands,
    }
    record.write(os.path.join(folder, "record.toml"), rec)
    print(f"basket_weave: {len(bands)} levels")


if __name__ == "__main__":
    main()
