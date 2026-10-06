#!/usr/bin/env python3
"""A material's wrap atlas (A6.4, kitmath.WRAPS): its own levels cut into strips side by side, one for each width a
wrapped pole, branch or trunk of the kit takes, each strip seamless round its own width, so a part's bark or wood
wraps without a seam however thick the part is.

    python3 tools/art/wraps.py <material folder> <out folder>
        writes b0.png to b8.png of the wrap atlas and prints each strip's window and the cost of closing it, and
        each level whose colour was brought back to the material's

For each strip, w texture pixels wide at x in the atlas, in the order kitmath.WRAPS lists them:
1. its window: w + 8 columns of the material's band 0 (w + w / 2 for a strip under 16), round its wrap, starting on
   a multiple of 8 so the levels below cut it at whole texture pixels. Of the windows that close no worse than they
   run on (the closing's cost at most the mean step between the window's own neighbouring columns, so a window with
   marks in it is not passed over for a calm one and the strips keep the material's accents), the one sharing fewest
   columns with the strips chosen before it, so the strips come from across the material and the atlas repeats no
   part of it, then the one closing best; if no window closes that well, the one closing best;
2. its closing: the strip's first 8 columns (w / 2 for a narrow one) are taken from those after it where the path of
   least difference down the strip says so (tile.py's cut), so its right edge runs on into its left as the material
   ran on;
3. its levels: each level of the material cut at the same window at that level's scale and closed the same way,
   the strip (w / 2^n wide) set at x / 2^n; a strip under 2 texture pixels wide is the window's single column.
The atlas's columns past the last strip hold the material's own. In a small level the strips are a few of the
material's columns, which may stray from its colour: where a level of the atlas strays by more than half the line
the checks hold it to (match.DRIFT_LIGHTNESS, match.DRIFT_HUE), its lightness, hue and colourfulness are brought
back to the material's own level's by `kindling look` (hold_colour), its contrast left as it is.

Implements PRE-46 and PRE-22, see A6.4.
"""

import argparse
import os
import sys

import numpy as np

import kitmath
import look
import match
import texels
import tile

OVERLAP = 8
ALIGN = 8
STRAY = 0.5  # share of the drift lines past which an atlas level's colour is brought back to the material's


def window(level, x0, width):
    """Columns x0 to x0 + width of a level, round its wrap."""
    return np.take(level, np.arange(x0, x0 + width) % level.shape[1], axis=1)


def close(cols, w, overlap):
    """A strip w wide made seamless round its width from w + overlap columns: where the path of least difference
    down the overlap says, its first columns are the ones after its right edge. (the strip, the path's cost)"""
    if overlap < 1:
        return cols[:, :w].copy(), 0.0
    a = cols[:, w : w + overlap].astype(np.float64)
    b = cols[:, :overlap].astype(np.float64)
    err = ((a - b) ** 2).sum(axis=2)
    path = tile._cut(err)
    out = cols[:, :w].copy()
    for y, p in enumerate(path):
        out[y, :p] = cols[y, w : w + p]
    return out, float(err[np.arange(len(path)), path].mean())


CLOSING_LINE = 1.0  # a closing at most this times the mean step between the window's own neighbouring columns


def best_window(level, w, taken=None):
    """(the window start, a multiple of ALIGN, its closing cost): of the windows whose closing costs at most
    CLOSING_LINE times the mean step between their own neighbouring columns, the one sharing the smallest share of
    its columns with those `taken` marks (how many strips took each column of the level), then the one closing best
    against its steps; if none closes that well, the one closing best."""
    taken = np.zeros(level.shape[1], int) if taken is None else taken
    ov = min(OVERLAP, w // 2)
    found = []
    for x0 in range(0, level.shape[1], ALIGN):
        cols = window(level, x0, w + ov)
        _, cost = close(cols, w, ov)
        inside = float((np.diff(cols.astype(np.float64), axis=1) ** 2).sum(axis=2).mean()) if w + ov > 1 else 0.0
        shared = float((taken[np.arange(x0, x0 + w + ov) % level.shape[1]] > 0).mean())
        found.append((cost / max(1e-9, inside), shared, x0, cost))
    good = [f for f in found if f[0] <= CLOSING_LINE]
    best = min(good, key=lambda f: (f[1], f[0], f[2])) if good else min(found, key=lambda f: (f[0], f[2]))
    return best[2], best[3]


def atlas(levels):
    """The wrap atlas's levels from a material's levels (b0 first, 256 square): (levels, [(width, x, window, cost)])."""
    size = levels[0].shape[0]
    plan = []
    taken = np.zeros(levels[0].shape[1], int)
    for w, x in kitmath.WRAPS:
        x0, cost = best_window(levels[0], w, taken)
        taken[np.arange(x0, x0 + w + min(OVERLAP, w // 2)) % size] += 1
        plan.append((w, x, x0, cost))
    out = []
    for n, level in enumerate(levels):
        k = 2**n
        side = size // k
        a = level.copy()  # past the last strip, the material's own
        for w, x, x0, _ in plan:
            left, right = x // k, min(side, -(-(x + w) // k))
            wide = right - left
            if wide < 1:
                continue
            if wide < 2:
                a[:, left:right] = window(level, x0 // k, 1)
                continue
            cols = window(level, x0 // k, wide + OVERLAP // k)
            strip, _ = close(cols, wide, min(OVERLAP // k, wide // 2))
            a[:, left:right] = strip
        out.append(a)
    return out, plan


def hold_colour(made, levels):
    """(the atlas's levels, each level's calibration words or None): a level of the atlas whose lightness or hue
    strays from the material's own level by more than STRAY of the drift lines brought back to that level's
    lightness, hue and colourfulness, its contrast unchanged, then settled (match.settle)."""
    out, words = [], []
    for a, level in zip(made, levels, strict=True):
        s, t = look.stats(a), look.stats(level)
        dl = abs(s["lightness"] - t["lightness"])
        coloured = s["colourfulness"] > 0.5 and t["colourfulness"] > 0.5
        dh = abs((s["hue"] - t["hue"] + 180.0) % 360.0 - 180.0) if coloured else 0.0
        if dl <= STRAY * match.DRIFT_LIGHTNESS and dh <= STRAY * match.DRIFT_HUE:
            out.append(a)
            words.append(None)
            continue
        fixed, nums = match.calibrate(a, t, 100.0)
        fixed, move = match.settle(fixed, t)
        out.append(fixed)
        words.append(match.describe(nums, move))
    return out, words


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("material")
    ap.add_argument("out")
    args = ap.parse_args(argv)
    n = 0
    levels = []
    while os.path.exists(os.path.join(args.material, f"b{n}.png")):
        levels.append(texels.load(os.path.join(args.material, f"b{n}.png")))
        n += 1
    made, plan = atlas(levels)
    made, words = hold_colour(made, levels)
    os.makedirs(args.out, exist_ok=True)
    for n, a in enumerate(made):
        texels.save_png(a, os.path.join(args.out, f"b{n}.png"))
    for w, x, x0, cost in plan:
        print(f"strip {w:2d} at x {x:3d}: the material's columns from {x0}, closing cost {cost:.0f}")
    for n, w in enumerate(words):
        if w:
            print(f"level {n}: its colour brought back to the material's ({w})")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
