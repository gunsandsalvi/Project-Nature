#!/usr/bin/env python3
"""The code reduction (art/BRIEF.md, steps 5 and 6): the level below a level, drawn from its own shades, never
averaged, so a band keeps clean clusters instead of turning to the speckle of answer 31.

    python3 tools/art/reduce.py <level.png> <out.png> [--shades 12]

1. The level's own shades: its colours grouped into a few by k-means on their red, green and blue values, seeded so
   the same level always gives the same shades.
2. Each 2 x 2 block of texture pixels becomes its most common shade; a tie goes to the shade nearest the block's
   mean colour, so the level's overall colour holds.
3. Lone texture pixels, with no neighbour of their own shade, take their neighbours' most common shade, the tile
   wrapping, so a coarser band grows calmer instead of turning to speckle (answer 31); levels under 8 texture
   pixels are left as they are, since every texture pixel of them counts.
Each texture pixel of the result covers exactly 2 x 2 of the level above, and the tile still wraps.

Implements PRE-22, see A5.3 and A5.4.
"""

import argparse
import sys

import numpy as np

import texels

SMALL = 8  # levels smaller than this keep every texture pixel; from this size on, lone ones are cleaned


def shades(t, k=12, seed=1, rounds=25):
    """(the shades as an array of colours, each texture pixel's shade index)."""
    x = t.reshape(-1, 3).astype(np.float64)
    distinct = np.unique(x, axis=0)
    k = min(k, len(distinct))
    rng = np.random.default_rng(seed)
    centres = [distinct[rng.integers(len(distinct))]]
    for _ in range(1, k):  # k-means++: each new centre far from those chosen
        d = ((distinct[:, None, :] - np.array(centres)[None]) ** 2).sum(-1).min(axis=1)
        centres.append(distinct[rng.choice(len(distinct), p=d / d.sum())] if d.sum() > 0 else distinct[0])
    c = np.array(centres)
    for _ in range(rounds):
        label = ((x[:, None, :] - c[None]) ** 2).sum(-1).argmin(axis=1)
        for j in range(k):
            if (label == j).any():
                c[j] = x[label == j].mean(axis=0)
    label = ((x[:, None, :] - c[None]) ** 2).sum(-1).argmin(axis=1)
    return np.round(c).astype(np.uint8), label.reshape(t.shape[:2])


def majority(label, t, colours):
    """Each 2 x 2 block's most common shade; a tie goes to the shade nearest the block's mean colour, so the level's
    colour holds."""
    h, w = label.shape[0] // 2, label.shape[1] // 2
    blocks = label[: 2 * h, : 2 * w].reshape(h, 2, w, 2).transpose(0, 2, 1, 3).reshape(h, w, 4)
    k = len(colours)
    votes = np.stack([(blocks == j).sum(-1) for j in range(k)], -1).astype(np.float64)
    means = t[: 2 * h, : 2 * w].astype(np.float64).reshape(h, 2, w, 2, 3).mean(axis=(1, 3))
    near = ((means[:, :, None, :] - colours.astype(np.float64)[None, None]) ** 2).sum(-1)
    score = votes + 0.5 * (1 - near / (near.max() + 1e-9))  # a tie-break worth less than one vote
    score[votes == 0] = -np.inf
    return score.argmax(-1)


def clean(label):
    """Lone texture pixels (no 4-neighbour of the same shade) take the most common shade of their 8 neighbours."""
    same = np.zeros(label.shape, bool)
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        same |= np.roll(np.roll(label, dy, 0), dx, 1) == label
    around = np.stack([np.roll(np.roll(label, dy, 0), dx, 1) for dy in (-1, 0, 1) for dx in (-1, 0, 1) if dy or dx])
    k = int(label.max()) + 1
    counts = np.stack([(around == j).sum(0) for j in range(k)], -1)
    return np.where(same, label, counts.argmax(-1))


def reduce(t, k=12):
    """The level below t, half its size each way. Coarser levels get fewer shades (k at 64 texture pixels and more,
    then two fewer at each halving, at least 4) and are cleaned twice, so they grow calmer as they shrink."""
    if t.shape[0] < 2 or t.shape[1] < 2:
        raise ValueError("a level of one texture pixel has no level below it")
    size = min(t.shape[:2]) // 2
    halvings = max(0, int(np.log2(64 / size))) if size < 64 else 0
    colours, label = shades(t, max(4, k - 2 * halvings))
    low = majority(label, t, colours)
    if min(low.shape) >= SMALL:
        low = clean(clean(low))
    return colours[low]


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("level")
    ap.add_argument("out")
    ap.add_argument("--shades", type=int, default=12)
    args = ap.parse_args(argv)
    low = reduce(texels.load(args.level), args.shades)
    texels.save_png(low, args.out)
    print(f"{args.out}: {low.shape[1]} x {low.shape[0]}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
