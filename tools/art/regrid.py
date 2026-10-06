#!/usr/bin/env python3
"""Re-gridding (IMPLEMENTATION.md, the art lane): a picture brought onto an exact grid of texture pixels, one
block becoming one texture pixel of the block's median colour.

    python3 tools/art/regrid.py swatches <picture>
        the boxes of the swatches GPT drew on its flat grey background
    python3 tools/art/regrid.py find <picture> <out.png> [--box X0 Y0 X1 Y1]
        GPT's blocks: their size (fractional allowed) found from where colours change, and the edges between them
        placed along each axis, so a grid that drifts across the picture is followed; prints the loss
    python3 tools/art/regrid.py fixed <picture> <out.png> --box X0 Y0 X1 Y1 --block B [--squash F]
        a painted picture with no grid of its own, such as the liked picture: blocks of B picture pixels across and
        B * F down, so a ground seen at a tilt is flattened (F is the ground's squash, about 0.55 in the liked camp)
    python3 tools/art/regrid.py delight <in.png> <out.png> [--sigma S]
        painted light taken out: each texture pixel divided by the broad light round it, in linear light, so the
        mean is kept

GPT's grids drift: in its swatches the edges between columns move with x alone and those between rows with y alone,
so one set of column edges and one of row edges, each placed by dynamic programming on the whole picture's profile
of colour change, follows them. The loss is the share of the picture's colour variation (summed over red, green and
blue) that the blocks' medians do not keep: 2 to 8% on swatches GPT drew in
blocks, at most 10% allowed.

Implements PRE-22 and PRE-01, see A5.4.
"""

import argparse
import sys

import numpy as np

import texels

GREY = 128  # the flat background the requests ask for (#808080)


def swatches(a, tolerance=10, min_side=64):
    """Boxes (x0, y0, x1, y1) of the rectangles drawn on a flat grey background, left to right, then top to bottom."""
    grey = (np.abs(a.astype(int) - GREY) <= tolerance).all(axis=2)
    boxes = []
    for x0, x1 in _runs(grey.mean(axis=0) < 0.9, min_side):
        for y0, y1 in _runs(grey[:, x0:x1].mean(axis=1) < 0.9, min_side):
            boxes.append((x0, y0, x1, y1))
    return sorted(boxes, key=lambda b: (round(b[1] / 200), b[0]))


def _runs(flags, min_len):
    out, start = [], None
    for i, f in enumerate(list(flags) + [False]):
        if f and start is None:
            start = i
        elif not f and start is not None:
            if i - start >= min_len:
                out.append((start, i))
            start = None
    return out


def profile(a, axis):
    """The mean colour change across each boundary between neighbours: index i is the boundary after column (axis 1)
    or row (axis 0) i."""
    d = np.abs(np.diff(a.astype(np.float64), axis=axis)).sum(axis=2)
    return d.mean(axis=1 - axis)


def block_size(a, lo=3, hi=40):
    """The block size in picture pixels (fractional), and the strength of its peak (0 to 1): the first strong peak
    of the autocorrelation of where colours change, both axes together. Unlike a comb laid at one phase, it is not
    misled when GPT's grid drifts across the picture; a block size and its multiples all peak, so the smallest strong
    peak wins. The strength is for reading beside the loss: GPT's swatches score about 0.3 to 0.6 and plain noise
    about 0.1, but a smooth picture with no grid can score 0.2, so the loss decides whether a picture re-grids."""
    reach = 4 * hi + 2
    acs = []
    for axis in (1, 0):
        prof = profile(a, axis)
        d = prof - prof.mean()
        full = np.correlate(d, d, mode="full")[len(d) - 1 :]
        full = np.concatenate([full, np.zeros(max(0, reach + 1 - len(full)))])
        acs.append(full[: reach + 1] / max(1e-9, full[0]))
    ac = (acs[0] + acs[1]) / 2
    peaks = [lag for lag in range(lo, hi + 1) if ac[lag] >= ac[lag - 1] and ac[lag] >= ac[lag + 1] and ac[lag] > 0]
    if not peaks:
        return float(lo), 0.0
    top = max(ac[lag] for lag in peaks)
    first = next(lag for lag in peaks if ac[lag] >= 0.6 * top)
    # the size refined from the peaks at one to four block sizes, each placed by a parabola through its neighbours
    ks, places = [], []
    for k in range(1, 5):
        c = int(round(k * first))
        span = max(1, int(first // 3))
        if c + span + 1 >= len(ac):
            break
        lag = c - span + int(np.argmax(ac[c - span : c + span + 1]))
        y0, y1, y2 = ac[lag - 1], ac[lag], ac[lag + 1]
        den = y0 - 2 * y1 + y2
        ks.append(k)
        places.append(lag + (float(np.clip(0.5 * (y0 - y2) / den, -0.5, 0.5)) if den < 0 else 0.0))
    ks, places = np.array(ks, float), np.array(places)
    return float((ks * places).sum() / (ks * ks).sum()), float(max(0.0, ac[first]))


def edges(prof, p, slack=0.35, pull=0.1, closed=False):
    """Block edges along one axis: positions 0 < e1 < e2 < ... < n (a block spans [e_k, e_k+1)), spaced p apart
    within the slack, placed where colours change most; partial blocks at either end are dropped. Closed, the
    blocks run from the first pixel to the last exactly, as in a picture that tiles."""
    n = len(prof) + 1
    gmin, gmax = max(1, int(np.floor(p * (1 - slack)))), int(np.ceil(p * (1 + slack)))
    # an edge earns its change above that of a typical boundary, so weak boundaries cost and the edges are not
    # packed as close as the slack allows
    base = float(np.percentile(prof, 60))
    scale = max(1e-9, float(np.std(prof)))
    strength = np.concatenate([[0.0], prof - base, [0.0]])  # strength[i]: the boundary before pixel i
    score = np.full(n + 1, -np.inf)
    back = np.full(n + 1, -1)
    if closed:
        score[0] = 0.0
    else:
        for i in range(1, min(gmax, n) + 1):  # the first edge: anywhere in the first block
            score[i] = strength[i] / scale
    for i in range(1, n + 1):
        for g in range(gmin, gmax + 1):
            j = i - g
            if j >= (0 if closed else 1) and score[j] > -np.inf:
                s = score[j] + strength[i] / scale - pull * (g - p) ** 2
                if s > score[i]:
                    score[i], back[i] = s, j
    ends = [n] if closed else [i for i in range(max(1, n - gmax), n + 1) if score[i] > -np.inf]
    if score[ends[0]] == -np.inf and closed:
        raise ValueError(f"no blocks of about {p:.1f} pixels fill {n} pixels exactly")
    i = max(ends, key=lambda k: score[k])
    out = [i]
    while back[i] >= 0:
        i = back[i]
        out.append(i)
    return np.array(out[::-1])


def medians(a, xs, ys):
    """Each block's median colour: block (r, c) spans rows ys[r]..ys[r+1] and columns xs[c]..xs[c+1]."""
    out = np.zeros((len(ys) - 1, len(xs) - 1, 3), np.uint8)
    for r in range(len(ys) - 1):
        band = a[ys[r] : ys[r + 1]]
        for c in range(len(xs) - 1):
            block = band[:, xs[c] : xs[c + 1]].reshape(-1, 3)
            out[r, c] = np.round(np.median(block, axis=0))
    return out


def loss(a, xs, ys, grid):
    """The share of the picture's colour variation inside the blocks that their medians do not keep."""
    part = a[ys[0] : ys[-1], xs[0] : xs[-1]].astype(np.float64)
    rows = np.repeat(np.arange(len(ys) - 1), np.diff(ys))
    cols = np.repeat(np.arange(len(xs) - 1), np.diff(xs))
    back = grid[rows][:, cols].astype(np.float64)
    total = ((part - part.reshape(-1, 3).mean(axis=0)) ** 2).sum()
    return float(((part - back) ** 2).sum() / max(1e-9, total))


def find(a, p=None, tiles=False):
    """GPT's blocks re-gridded: (texture pixels, block size, the strength of its peak or None when the size was
    given, loss). For a picture that tiles, the picture is first turned round its wrap so a strong edge comes first,
    and the blocks then fill it exactly, so nothing is lost at its edges and the grid wraps as the picture does; the
    grid is turned back to within half a block of where the picture began."""
    strength = None
    if p is None:
        p, strength = block_size(a)
    turned = [0, 0]
    if tiles:
        for axis in (1, 0):  # start at the strongest edge within the first block
            prof = profile(a, axis)
            turned[axis] = 1 + int(np.argmax(prof[: int(np.ceil(p))]))
            a = np.roll(a, -turned[axis], axis=axis)
    xs = edges(profile(a, 1), p, closed=tiles)
    ys = edges(profile(a, 0), p, closed=tiles)
    grid = medians(a, xs, ys)
    lost = loss(a, xs, ys, grid)
    if tiles:  # turned back, so texture pixel 0 lies where the picture began and the layout keeps its place
        for axis in (1, 0):
            grid = np.roll(grid, int(round(turned[axis] * grid.shape[axis] / a.shape[axis])), axis=axis)
    return grid, p, strength, lost


def fixed(a, block, squash=1.0):
    """A picture with no grid of its own sampled in blocks of `block` picture pixels across and block * squash down:
    each texture pixel the median of the picture pixels whose centres fall in its block, or the nearest one."""
    bx, by = float(block), float(block) * float(squash)
    h, w, _ = a.shape
    nx, ny = int(w // bx), int(h // by)
    out = np.zeros((ny, nx, 3), np.uint8)
    for r in range(ny):
        y0, y1 = r * by, (r + 1) * by
        rows = np.arange(int(np.ceil(y0 - 0.5)), int(np.ceil(y1 - 0.5)))
        if len(rows) == 0:
            rows = np.array([min(h - 1, int((y0 + y1) / 2))])
        for c in range(nx):
            x0, x1 = c * bx, (c + 1) * bx
            cols = np.arange(int(np.ceil(x0 - 0.5)), int(np.ceil(x1 - 0.5)))
            if len(cols) == 0:
                cols = np.array([min(w - 1, int((x0 + x1) / 2))])
            block = a[rows[0] : rows[-1] + 1, cols[0] : cols[-1] + 1].reshape(-1, 3)
            out[r, c] = np.round(np.median(block, axis=0))
    return out


def delight(t, sigma=None, wrap=True, mask=None):
    """Painted light taken out: each texture pixel divided by the broad light round it (linear-light luminance
    blurred over a sixth of the picture), scaled so the mean light is kept. Colour, grain and small hollows stay.
    With a mask, only the pixels it keeps count towards the broad light and its mean."""
    if sigma is None:
        sigma = min(t.shape[:2]) / 6.0
    lin = texels.to_linear(t)
    y = texels.luminance(t)
    m = np.ones(y.shape) if mask is None else mask.astype(np.float64)
    broad = texels.blur(y * m, sigma, wrap=wrap) / np.maximum(texels.blur(m, sigma, wrap=wrap), 1e-6)
    mean = (y * m).sum() / max(1e-9, m.sum())
    return texels.to_srgb(lin * (mean / np.maximum(broad, 1e-4))[..., None])


def fit(t, w, h):
    """A grid brought to exactly w x h texture pixels by repeating or dropping whole rows and columns, spread evenly,
    never by mixing colours."""
    ys = np.minimum((np.arange(h) + 0.5) * t.shape[0] / h, t.shape[0] - 1).astype(int)
    xs = np.minimum((np.arange(w) + 0.5) * t.shape[1] / w, t.shape[1] - 1).astype(int)
    return t[ys][:, xs]


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("mode", choices=["swatches", "find", "fixed", "delight"])
    ap.add_argument("picture")
    ap.add_argument("out", nargs="?")
    ap.add_argument("--box", type=int, nargs=4)
    ap.add_argument("--block", type=float)
    ap.add_argument("--squash", type=float, default=1.0)
    ap.add_argument("--sigma", type=float)
    args = ap.parse_args(argv)
    a = texels.load(args.picture)
    if args.box:
        x0, y0, x1, y1 = args.box
        a = a[y0:y1, x0:x1]
    if args.mode == "swatches":
        for b in swatches(a):
            print(*b)
        return 0
    if not args.out:
        ap.error("an output picture is needed")
    if args.mode == "find":
        grid, p, strength, lost = find(a, args.block)
        size = f"{grid.shape[1]} x {grid.shape[0]} texels"
        print(f"blocks {p:.2f} px (peak {strength or 0:.2f}), {size}, loss {lost:.1%}")
    elif args.mode == "fixed":
        if not args.block:
            ap.error("--block is needed")
        grid = fixed(a, args.block, args.squash)
        print(f"{grid.shape[1]} x {grid.shape[0]} texels")
    else:
        grid = delight(a, args.sigma)
        print(f"{grid.shape[1]} x {grid.shape[0]} texels, light taken out")
    texels.save_png(grid, args.out)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
