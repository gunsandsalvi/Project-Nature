#!/usr/bin/env python3
"""The code reduction (IMPLEMENTATION.md, the art lane): the level below a level, drawn from its own shades, never
averaged, with bolder marks and fewer of them, so a band keeps clean clusters and its accents instead of turning to
speckle (A5.3).

    python3 tools/art/reduce.py <level.png> <out.png> [--shades 12] [--marks N]

1. The level's own shades: its colours grouped into a few by k-means on their red, green and blue values, seeded so
   the same level always gives the same shades; coarser levels get fewer.
2. Its background: each 2 x 2 block of texture pixels becomes its most common shade, a tie going to the shade
   nearest the block's mean colour, so the level's overall colour holds; then lone texture pixels, with no
   neighbour of their own shade, take their neighbours' most common shade, the tile wrapping, so a coarser band
   grows calmer; levels under 8 texture pixels are left as they are, since every texture pixel of them counts.
3. Its marks: runs of one shade that stand out from the shades around them (at least twice the level's mean step
   from its surroundings) and are small (at most 24 texture pixels): crumbs, grit, lenticels, charcoal. The
   majority drops them; the strongest are drawn again on the background, each at least 2 texture pixels and never
   touching another, at most half as many as the level above had and covering at most 3% of the level, so the band
   keeps its accents with bolder marks and fewer of them, never single-pixel speckle. A level smaller than 32
   texture pixels takes none, since its marks would stand in a grid.
How many marks to draw is the caller's choice (levels.py draws only as many as hold the accents); with none, the
result is the background alone. Each texture pixel of the result covers exactly 2 x 2 of the level above, and the
tile still wraps.

Implements PRE-22 and PRE-20, see A5.3 and A5.4.
"""

import argparse
import sys

import numpy as np

import texels

SMALL = 8  # levels smaller than this keep every texture pixel; from this size on, lone ones are cleaned
MARKS_FROM = 32  # levels smaller than this take no marks
LARGEST = 24  # texture pixels: a larger run is part of the background, not a mark
STRONG = 2.0  # a mark stands at least this many typical steps from the shade around it
MOST_SHARE = 0.03  # marks cover at most this share of a level


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


def surroundings(label, k, r=2):
    """Each texture pixel's most common shade within r texture pixels of it, the tile wrapping."""
    counts = np.zeros((k,) + label.shape, np.int32)
    for dy in range(-r, r + 1):
        for dx in range(-r, r + 1):
            moved = np.roll(np.roll(label, dy, 0), dx, 1)
            for j in range(k):
                counts[j] += moved == j
    return counts.argmax(0)


def runs(mask, label):
    """The 4-connected runs of one shade where mask is set, the tile wrapping: arrays of (y, x)."""
    h, w = mask.shape
    seen = np.zeros(mask.shape, bool)
    out = []
    for y0, x0 in np.argwhere(mask):
        if seen[y0, x0]:
            continue
        stack, cells = [(y0, x0)], []
        seen[y0, x0] = True
        while stack:
            y, x = stack.pop()
            cells.append((y, x))
            for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                yy, xx = (y + dy) % h, (x + dx) % w
                if mask[yy, xx] and not seen[yy, xx] and label[yy, xx] == label[y0, x0]:
                    seen[yy, xx] = True
                    stack.append((yy, xx))
        out.append(np.array(cells))
    return out


class Reduction:
    """The level below t: its background, and its marks ready to be drawn on it, strongest first.

    background: shade indices of the level below; colours: the shades; marks: (strength, shade, cells) with cells
    the texture pixels of the level below that the mark takes, at least 2 and never touching an earlier mark's;
    most: how many of them the level may take (at most half the strong runs above, within MOST_SHARE)."""

    def __init__(self, t, k=12):
        if t.shape[0] < 2 or t.shape[1] < 2:
            raise ValueError("a level of one texture pixel has no level below it")
        size = min(t.shape[:2]) // 2
        halvings = max(0, int(np.log2(64 / size))) if size < 64 else 0
        self.colours, label = shades(t, max(4, k - 2 * halvings))
        low = majority(label, t, self.colours)
        if min(low.shape) >= SMALL:
            low = clean(clean(low))
        self.background = low
        self.marks, self.most = [], 0
        if min(low.shape) >= MARKS_FROM:
            self._find_marks(label)

    def _find_marks(self, label):
        k = len(self.colours)
        cf = self.colours.astype(np.float64)
        around = surroundings(label, k)
        step = np.sqrt(((cf[label] - cf[around]) ** 2).sum(-1))
        differs = label != around
        if not differs.any():
            return
        typical = float(step.mean())  # the level's typical step from its surroundings: small where it is calm
        strong = differs & (step >= STRONG * typical)
        found = []
        for cells in runs(strong, label):
            if len(cells) > LARGEST:
                continue
            ys, xs = cells[:, 0], cells[:, 1]
            # strongest first; ties by shade, size and place, so the same level always gives the same marks
            found.append((-float(step[ys, xs].mean()) * np.sqrt(len(cells)), int(label[ys[0], xs[0]]), cells))
        found.sort(key=lambda m: (m[0], m[1], len(m[2]), tuple(m[2].min(axis=0))))
        hh, ww = self.background.shape
        room = MOST_SHARE * hh * ww
        taken = np.zeros((hh, ww), bool)  # each drawn mark and the ring round it, so no two touch
        used = 0
        for strength, shade, cells in found[: max(1, len(found) // 2)]:
            if used >= room:
                break
            below = sorted({(int(y) // 2 % hh, int(x) // 2 % ww) for y, x in cells})
            if len(below) < 2:  # grown to 2 texture pixels along the mark's own direction
                y, x = below[0]
                wide = np.ptp(cells[:, 1]) >= np.ptp(cells[:, 0])
                below.append((y, (x + 1) % ww) if wide else ((y + 1) % hh, x))
            if any(taken[p] for p in below) or used + len(below) > room:
                continue
            for y, x in below:
                taken[(y + np.array([-1, -1, -1, 0, 0, 0, 1, 1, 1])) % hh, (x + np.array([-1, 0, 1] * 3)) % ww] = True
            self.marks.append((-strength, shade, below))
            used += len(below)
        self.most = len(self.marks)

    def draw(self, marks=None):
        """The level below with its `marks` strongest marks drawn on the background (all it may take if None)."""
        low = self.background.copy()
        for _, shade, cells in self.marks[: self.most if marks is None else min(marks, self.most)]:
            for y, x in cells:
                low[y, x] = shade
        return self.colours[low]


def reduce(t, k=12, marks=0):
    """The level below t, half its size each way: its background, with its `marks` strongest marks drawn again (all
    it may take if None)."""
    return Reduction(t, k).draw(marks)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("level")
    ap.add_argument("out")
    ap.add_argument("--shades", type=int, default=12)
    ap.add_argument("--marks", type=int, help="how many marks to draw again (all the level may take if left out)")
    args = ap.parse_args(argv)
    r = Reduction(texels.load(args.level), args.shades)
    low = r.draw(args.marks)
    texels.save_png(low, args.out)
    print(f"{args.out}: {low.shape[1]} x {low.shape[0]}, {min(r.most, args.marks or r.most)} of {r.most} marks")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
