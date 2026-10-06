#!/usr/bin/env python3
"""Band 0's tile (IMPLEMENTATION.md, the art lane): a seamless square of texture pixels built by code from one
source or more, with no strong repeat inside it.

    python3 tools/art/tile.py <out.png> <source.png>... [--size 256] [--step 32] [--overlap 8] [--seed 1]
        quilt the tile from re-gridded sources; prints its seam and repeat numbers
    python3 tools/art/tile.py measure <tile.png>...
        the seam and repeat numbers of tiles already made

The tile is quilted on a torus (Efros and Freeman's image quilting, closed round both edges): patches of the
sources are laid cell by cell, each chosen among those that best match what is already laid where they overlap,
and each joined along the path of least difference, so every texture pixel comes whole from one source pixel and the
pixel art stays crisp, never blended. Patches far from those already used are preferred, so nothing repeats
strongly. A source may carry a mask (white where it may be used) as <source>.mask.png beside it.

The numbers, on red, green and blue (no colour measure is needed for either):
- seams: the step across each wrapping edge, against the median step between neighbours inside, after a blur of
  one texture pixel, the larger of the two directions; 1 is no seam, at most 1.2 is allowed;
- repeat: the strongest autocorrelation of the tile's fine detail (finer than 15 cm) at shifts of more than
  15 cm, 1 being a copy; at most 0.2 is allowed.

Implements PRE-22, see A5.4.
"""

import argparse
import os
import sys

import numpy as np

import texels

TEXELS_A_METRE = 64


def seams(t):
    """The step across the wrapping edges against the median step inside, after a blur of one texture pixel; the
    larger of the two directions."""
    b = texels.blur(t.astype(np.float64), 1.0, wrap=True)
    out = []
    for axis in (1, 0):
        steps = np.abs(np.diff(b, axis=axis)).sum(axis=2).mean(axis=1 - axis)
        first = np.take(b, 0, axis=axis)
        last = np.take(b, -1, axis=axis)
        across = np.abs(first - last).sum(axis=1).mean()
        out.append(float(across / max(1e-9, np.median(steps))))
    return max(out)


def repeat(t, metres_cut=0.15, texels_a_metre=TEXELS_A_METRE):
    """The strongest autocorrelation of the fine detail at shifts longer than metres_cut; the tile wraps."""
    cut = metres_cut * texels_a_metre
    a = t.astype(np.float64)
    detail = a - texels.blur(a, cut, wrap=True)
    h, w = detail.shape[:2]
    ac = np.zeros((h, w))
    for c in range(3):
        f = np.fft.fft2(detail[..., c])
        ac += np.real(np.fft.ifft2(f * np.conj(f)))
    if ac[0, 0] <= 0:
        return 0.0
    ac /= ac[0, 0]
    dy = np.minimum(np.arange(h), h - np.arange(h))[:, None]
    dx = np.minimum(np.arange(w), w - np.arange(w))[None, :]
    far = np.hypot(dy, dx) > cut
    return float(ac[far].max()) if far.any() else 0.0


class Source:
    """A re-gridded source: its texture pixels, as floats, and which patch corners may be used."""

    def __init__(self, t, mask=None):
        self.t = t.astype(np.float64)
        self.mask = np.ones(t.shape[:2], bool) if mask is None else mask.astype(bool)

    def corners(self, p):
        """Top-left corners of patches p x p that lie inside the source and its mask."""
        h, w = self.mask.shape
        if h < p or w < p:
            return np.zeros((0, 0), bool)
        bad = (~self.mask).astype(np.int64)
        s = np.pad(bad.cumsum(0).cumsum(1), ((1, 0), (1, 0)))
        inside = s[p:, p:] - s[:-p, p:] - s[p:, :-p] + s[:-p, :-p]
        return inside == 0


def _correlate(a, k):
    """Sliding sums: out[y, x] = sum of a[y + i, x + j] * k[i, j] over the kernel, for every corner where it fits."""
    h, w = a.shape
    kh, kw = k.shape
    fa = np.fft.rfft2(a, (h + kh, w + kw))
    fk = np.fft.rfft2(k[::-1, ::-1], (h + kh, w + kw))
    full = np.fft.irfft2(fa * fk, (h + kh, w + kw))
    return full[kh - 1 : h, kw - 1 : w]


def _cut(err):
    """The path of least difference down a strip (rows by columns): one column a row, moving at most one a row."""
    h, w = err.shape
    cost = err.copy()
    for y in range(1, h):
        left = np.concatenate([[np.inf], cost[y - 1, :-1]])
        right = np.concatenate([cost[y - 1, 1:], [np.inf]])
        cost[y] += np.minimum(np.minimum(left, cost[y - 1]), right)
    path = np.zeros(h, int)
    path[-1] = int(np.argmin(cost[-1]))
    for y in range(h - 2, -1, -1):
        x = path[y + 1]
        lo, hi = max(0, x - 1), min(w, x + 2)
        path[y] = lo + int(np.argmin(cost[y, lo:hi]))
    return path


def quilt(sources, size=256, step=32, overlap=8, seed=1, tolerance=0.1):
    """A seamless size x size tile quilted from the sources (texture pixel arrays, or Source objects)."""
    if size % step:
        raise ValueError(f"the tile's side {size} is not a whole number of steps of {step}")
    srcs = [s if isinstance(s, Source) else Source(s) for s in sources]
    p = step + overlap
    n = size // step
    rng = np.random.default_rng(seed)
    out = np.zeros((size, size, 3))
    filled = np.zeros((size, size), bool)
    corners = [s.corners(p) for s in srcs]
    # how near each corner lies to patches already used: 1 at a used corner, falling to 0 a patch's width away
    spread = [np.zeros(ok.shape) for ok in corners]
    near = 1 - np.maximum.outer(np.abs(np.arange(-p + 1, p)), np.abs(np.arange(-p + 1, p))) / p
    for r in range(n):
        for c in range(n):
            ys = (r * step + np.arange(p)) % size
            xs = (c * step + np.arange(p)) % size
            region = out[np.ix_(ys, xs)]
            known = filled[np.ix_(ys, xs)].astype(np.float64)
            choices = []
            for i, s in enumerate(srcs):
                ok = corners[i]
                if not ok.any():
                    continue
                if known.any():
                    err = np.zeros(ok.shape)
                    for ch in range(3):
                        a = s.t[..., ch]
                        err += _correlate(a * a, known) - 2 * _correlate(a, known * region[..., ch])
                    err += (known[..., None] * region**2).sum()
                    err = np.maximum(err, 0) / known.sum()
                else:
                    err = np.zeros(ok.shape)
                choices.append((i, np.where(ok, err, np.inf), spread[i]))
            if not choices:
                raise ValueError(f"no source holds a whole patch of {p} x {p} texture pixels")
            best = min(float(e.min()) for _, e, _ in choices)
            # among patches within the tolerance of the best match, the one least like any patch already used; the
            # last column and row close the torus, meeting laid pixels on two sides, so they take the best match
            closing = r == n - 1 or c == n - 1
            allowed = best * (1 + (0.0 if closing else tolerance)) + 1e-6
            pool = []
            for i, e, near_used in choices:
                for y, x in np.argwhere(e <= allowed):
                    pool.append((near_used[y, x], rng.random(), i, y, x))
            _, _, i, y, x = min(pool)
            h, w = spread[i].shape
            y0, y1, x0, x1 = max(0, y - p + 1), min(h, y + p), max(0, x - p + 1), min(w, x + p)
            spread[i][y0:y1, x0:x1] += near[y0 - y + p - 1 : y1 - y + p - 1, x0 - x + p - 1 : x1 - x + p - 1]
            patch = srcs[i].t[y : y + p, x : x + p]
            take = _where_new(patch, region, filled[np.ix_(ys, xs)], overlap)
            out[np.ix_(ys, xs)] = np.where(take[..., None], patch, region)
            filled[np.ix_(ys, xs)] = True
    return np.round(out).astype(np.uint8)


def best_tile(sources, size=256, step=32, overlap=8, seed=1, tries=8, most_repeat=0.2):
    """The tile with the least seam among `tries` seeds, of those whose repeat stays within most_repeat:
    (tile, seed, seams, repeat)."""
    best = None
    for s in range(seed, seed + tries):
        t = quilt(sources, size, step, overlap, s)
        sm, rp = seams(t), repeat(t)
        key = (rp > most_repeat, sm)
        if best is None or key < best[0]:
            best = (key, t, s, sm, rp)
    return best[1:]


def _where_new(patch, region, known, overlap):
    """Which texture pixels of a patch replace those already laid: everything not yet laid, and in each overlap with
    laid pixels the side of the path of least difference that belongs to the new patch."""
    p = patch.shape[0]
    err = ((patch - region) ** 2).sum(axis=2)
    take = np.ones((p, p), bool)
    if known[:, :overlap].all():  # the left edge meets laid pixels
        path = _cut(err[:, :overlap])
        take &= np.arange(p)[None, :] > path[:, None]
    if known[:overlap, :].all():  # the top edge
        path = _cut(err[:overlap, :].T)
        take &= np.arange(p)[:, None] > path[None, :]
    if known[:, -overlap:].all():  # the right edge, closing the torus
        path = _cut(err[:, -overlap:])
        take &= np.arange(p)[None, :] < (p - overlap + path)[:, None]
    if known[-overlap:, :].all():  # the bottom edge, closing the torus
        path = _cut(err[-overlap:, :].T)
        take &= np.arange(p)[:, None] < (p - overlap + path)[None, :]
    return take | ~known


def main(argv):
    if argv and argv[0] == "measure":
        for path in argv[1:]:
            t = texels.load(path)
            print(f"{path}: seams {seams(t):.2f}, repeat {repeat(t):.2f}")
        return 0
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("out")
    ap.add_argument("sources", nargs="+")
    ap.add_argument("--size", type=int, default=256)
    ap.add_argument("--step", type=int, default=32)
    ap.add_argument("--overlap", type=int, default=8)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--tries", type=int, default=8, help="seeds tried from --seed on; the least seam is kept")
    args = ap.parse_args(argv)
    srcs = []
    for path in args.sources:
        mask_path = path[: -len(".png")] + ".mask.png" if path.endswith(".png") else path + ".mask.png"
        mask = texels.load(mask_path)[..., 0] > 127 if os.path.exists(mask_path) else None
        srcs.append(Source(texels.load(path), mask))
    t, seed, sm, rp = best_tile(srcs, args.size, args.step, args.overlap, args.seed, args.tries)
    texels.save_png(t, args.out)
    print(f"{args.out}: {args.size} x {args.size}, seed {seed}, seams {sm:.2f}, repeat {rp:.2f}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
