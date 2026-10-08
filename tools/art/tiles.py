"""The pixel work of the texture path (A5.3, A5.4): the tiles of a sheet into the levels and versions the engine reads.

A tile is a square picture of texture pixels as an array of height x width x 3 (8-bit sRGB), one picture pixel a
texture pixel. Everything here is deterministic: the same pictures and numbers give the same bytes, since a texture
change is a look change the owner approves (PLT-09). Colour measures are never computed here: they come from
`kindling look` (look.py), so each is written once (CLAUDE.md, rule 4).

- Versions (A5.3): two to four versions of a tile that join every other without a seam because they share their edges.
  Every version keeps the same ring of `ring` texture pixels round its border, chosen where the tile is most
  ordinary (neutral_shift) and flattened in tone (flatten_border), so the ring never marks a grid on the ground; the
  rest of each version is quilted from the tile and its fellows (quilt), its patches matched to what is already placed
  and cut along the line where they differ least. requilt() makes the versions of a level drawn for the next band the
  same way, at that level's own size, since a level drawn on its own never lines up closely enough with the one above
  for the first level's cuts to be laid over it (they would slice its marks).
- Levels (A5.3): each level is half the one above. reduce() makes a level by code, which fit.py then fits so its
  accents match the level above's, since plain averaging loses a fifth of them (A5.3).
- Checks: wrap_ratio() and join_ratio() measure a seam against the tile's own largest jumps between neighbouring
  columns and rows: at 1 or less a join is no larger than a jump inside the tile.

Implements PRE-20 and PRE-22, see A5.3 and A5.4.
"""

import hashlib

import numpy as np
from PIL import Image


def read_rgb(path):
    """A picture file as an 8-bit RGB array."""
    with Image.open(path) as im:
        return np.asarray(im.convert("RGB")).copy()


def read_pixels(path):
    """A picture file as it is stored: an 8-bit RGB array, or RGBA where the file has an alpha channel (the water's
    marks)."""
    with Image.open(path) as im:
        return np.asarray(im.convert("RGBA" if "A" in im.getbands() else "RGB")).copy()


def write_png(path, pixels):
    """An 8-bit RGB or RGBA array as a lossless PNG."""
    Image.fromarray(np.ascontiguousarray(pixels, dtype=np.uint8)).save(path, format="PNG", optimize=True)


def sha256(path):
    """The SHA-256 of a file's bytes, in hex."""
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


class Chance:
    """A small deterministic source of whole numbers, the same on every machine: splitmix64 from a seed."""

    def __init__(self, seed):
        self.state = seed & 0xFFFFFFFFFFFFFFFF

    def next(self):
        self.state = (self.state + 0x9E3779B97F4A7C15) & 0xFFFFFFFFFFFFFFFF
        z = self.state
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & 0xFFFFFFFFFFFFFFFF
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & 0xFFFFFFFFFFFFFFFF
        return z ^ (z >> 31)

    def below(self, n):
        """A whole number from 0 to n - 1."""
        return self.next() % n


# ---- the picture's own grid -------------------------------------------------------------------------------------


def block_size(picture):
    """The block, in picture pixels, that a picture of enlarged texture pixels was drawn in: the largest whole size at
    which every block holds one colour (1 if none), found from the picture's edges: no colour changes inside a block."""
    best = 1
    h, w = picture.shape[:2]
    for size in range(2, 17):
        if h % size or w % size:
            continue
        blocks = picture.reshape(h // size, size, w // size, size, -1)
        if (blocks == blocks[:, :1, :, :1, :]).all():
            best = size
    return best


def regrid(picture, block):
    """The texture pixels of a picture drawn in blocks of `block` picture pixels (whole numbers only): each block's
    median colour, and the loss, the share of picture pixels that differ from their block's colour by more than a
    little (the sum of the channels' differences over 24)."""
    h, w = picture.shape[:2]
    if h % block or w % block:
        raise ValueError(f"a {w} x {h} picture is no whole number of blocks of {block}")
    blocks = picture.reshape(h // block, block, w // block, block, 3).transpose(0, 2, 1, 3, 4)
    flat = blocks.reshape(h // block, w // block, block * block, 3)
    median = np.median(flat, axis=2).round().astype(np.uint8)
    far = np.abs(flat.astype(np.int32) - median[:, :, None, :].astype(np.int32)).sum(axis=3) > 24
    return median, float(far.mean())


# ---- the ring ---------------------------------------------------------------------------------------------------


def ring_mask(n, ring):
    """True for the `ring` texture pixels round the border of an n x n tile."""
    m = np.zeros((n, n), bool)
    m[:ring, :] = True
    m[-ring:, :] = True
    m[:, :ring] = True
    m[:, -ring:] = True
    return m


def roll(tile, dy, dx):
    """The tile moved by whole texture pixels round its wrap, which a tile that wraps survives."""
    return np.roll(np.roll(tile, dy, axis=0), dx, axis=1)


def _blur_wrap(a, radius):
    """A box blur of a 2-D array that wraps."""
    out = a
    for axis in (0, 1):
        total = np.zeros_like(out)
        for d in range(-radius, radius + 1):
            total += np.roll(out, d, axis=axis)
        out = total / (2 * radius + 1)
    return out


def _ring_scorer(tile, ring, tone_radius, accent_share):
    """A function of a shift (dy, dx) of the tile that says how far its ring, cut there, lies from the whole tile in
    low tones, share of rare colours and jump from pixel to pixel, and how rough the join is that the cut puts either
    side of the wrap (smaller is better)."""
    n = tile.shape[0]
    f = tile.astype(np.float64)
    tone = _blur_wrap(f.mean(axis=2), tone_radius)
    away = np.sqrt(((f - f.reshape(-1, 3).mean(axis=0)) ** 2).sum(axis=2))
    rare = (away > np.quantile(away, 1.0 - accent_share)).astype(np.float64)
    across = np.abs(f - np.roll(f, -1, axis=1)).sum(axis=2)  # each pixel's jump to its right neighbour
    down = np.abs(f - np.roll(f, -1, axis=0)).sum(axis=2)
    jump = across + down
    col_join = across.mean(axis=0) / across.mean()  # the jump between each column and the next, over the mean
    row_join = down.mean(axis=1) / down.mean()
    ys, xs = np.nonzero(ring_mask(n, ring))

    def score(dy, dx):
        sy, sx = (ys - dy) % n, (xs - dx) % n
        return (
            abs(tone[sy, sx].mean() - tone.mean()) / (tone.std() + 1e-9)
            + abs(rare[sy, sx].mean() - rare.mean()) / (rare.mean() + 1e-9)
            + abs(jump[sy, sx].mean() - jump.mean()) / (jump.mean() + 1e-9)
            + 1.5 * (col_join[(-dx - 1) % n] + row_join[(-dy - 1) % n])
        )

    return score


def neutral_shift(tile, ring, step=4, tone_radius=12, accent_share=0.07, deeper=()):
    """Where to cut a wrapping tile so its ring is its most typical part: the shift (dy, dx) of the tile whose ring
    is nearest the whole tile's in its low tones, its share of rare colours and its jump from pixel to pixel, and whose
    join (the neighbouring columns and rows that the cut puts either side of the wrap) is one of the tile's smooth
    ones. A ring that is calmer, brighter or darker than the rest, or holds a mark, would show as a grid of lines
    across the ground once every version shares it, and a join that jumps would show as a seam at every cell. The
    same cut is made at every level drawn for the tile, so `deeper` (those levels' pictures of the first source, each
    a whole number of times smaller) count as much as the tile itself: a join smooth at the first level and rough at
    the next would show at the next band."""
    n = tile.shape[0]
    scorers = [(1, _ring_scorer(tile, ring, tone_radius, accent_share))]
    for level in deeper:
        f = n // level.shape[0]
        scorers.append((f, _ring_scorer(level, max(1, ring // f), max(1, tone_radius // f), accent_share)))
    best = None
    for dy in range(0, n, step):
        for dx in range(0, n, step):
            score = sum(scorer(dy // f, dx // f) for f, scorer in scorers)
            if best is None or score < best[0]:
                best = (score, dy, dx)
    return best[1], best[2]


# ---- versions by quilting ---------------------------------------------------------------------------------------


def _cut(diff, least=0):
    """The cheapest cut through a strip's differences, from its first row to its last: for each row, the first column
    taken from the new patch, never before column `least`, a column's neighbours in the next row only (dynamic
    programming)."""
    rows, cols = diff.shape
    cost = diff.astype(np.float64).copy()
    cost[:, :least] = np.inf
    for i in range(1, rows):
        above = cost[i - 1]
        left = np.concatenate([[np.inf], above[:-1]])
        right = np.concatenate([above[1:], [np.inf]])
        cost[i] += np.minimum(above, np.minimum(left, right))
    path = np.zeros(rows, dtype=int)
    path[-1] = int(np.argmin(cost[-1]))
    for i in range(rows - 2, -1, -1):
        j = path[i + 1]
        lo, hi = max(j - 1, 0), min(j + 2, cols)
        path[i] = lo + int(np.argmin(cost[i, lo:hi]))
    return path


def _near(shape, cy, cx, r):
    """Which offsets of a wrapping source lie within r of (cy, cx) both ways."""
    h, w = shape
    yy = np.arange(h)[:, None]
    xx = np.arange(w)[None, :]
    dy = np.minimum((yy - cy) % h, (cy - yy) % h)
    dx = np.minimum((xx - cx) % w, (cx - xx) % w)
    return (dy < r) & (dx < r)


def _run(flags):
    """How many of a row of flags, from its start, are true."""
    n = 0
    while n < len(flags) and flags[n]:
        n += 1
    return n


def patch_starts(n, patch, overlap):
    """Where the patches of a row (or column) of an `n`-pixel tile begin: the first at 0 and the last ending at the far
    edge, the rest evenly between, as few as keep every two neighbours overlapping by at least `overlap`. Each patch
    beside the tile's border therefore reaches into it by `overlap` and is cut from it along a line of its own: a last
    patch that stopped short of the border, or an odd one added after the rest, would leave a hard line where it meets
    the border."""
    if n <= patch:
        return [0]
    count = -(-(n - patch) // (patch - overlap)) + 1
    return sorted({round(i * (n - patch) / (count - 1)) for i in range(count)})


def quilt(
    sources,
    first,
    ring,
    overlap,
    patch,
    chance,
    natural,
    taken,
    tolerance=0.15,
    apart=24,
    tone=6.0,
    tone_radius=5,
    open_sources=False,
):
    """A new tile of the sources' marks that keeps the ring of `first`: patches of `patch` texture pixels cut from the
    wrapping sources (the first of them the one `first` was made from), placed in rows over the whole tile, each chosen
    among those that match what is already there within `tolerance` of the best and cut from it along the line where
    the two differ least. What is there to begin with is `first`'s border of `overlap` pixels, of which only the outer
    `ring` are fixed, so every patch beside the border has a cut it may move. `natural` is the shift that made `first`
    from the first source, so the patch that would only copy it back is never taken, and `taken` (a dict, kept between
    versions) holds each place's earlier choices, which are never taken again within `apart`. A candidate's error is
    its pixels' difference from what is there plus `tone` times the difference of both blurred over `tone_radius`, so
    the broad tone of one version carries over the ring into the next and no seam of tone shows where two versions
    meet. With `open_sources` the pictures do not wrap (a drawing seldom does), so no patch is cut across a picture's
    own edge, and `natural` may be None. Returns the tile."""
    srcs = [np.asarray(x, dtype=np.float64) for x in sources]
    h, w, _ = srcs[0].shape
    n = first.shape[0]
    if not 0 < ring <= overlap < patch:
        raise ValueError("the ring must lie within the overlap, and the overlap within a patch")
    canvas = np.zeros((n, n, 3))
    known = ring_mask(n, overlap)
    canvas[known] = first[known]
    fixed = ring_mask(n, ring)
    starts = patch_starts(n, patch, overlap)
    lows = [np.stack([_blur_wrap(src[:, :, c], tone_radius) for c in range(3)], axis=2) for src in srcs]
    ffts = [
        (
            [np.fft.rfft2(src[:, :, c] ** 2) for c in range(3)],
            [np.fft.rfft2(src[:, :, c]) for c in range(3)],
            [np.fft.rfft2(low[:, :, c] ** 2) for c in range(3)],
            [np.fft.rfft2(low[:, :, c]) for c in range(3)],
        )
        for src, low in zip(srcs, lows, strict=True)
    ]
    for y0 in starts:
        for x0 in starts:
            win_known = known[y0 : y0 + patch, x0 : x0 + patch]
            win_fixed = fixed[y0 : y0 + patch, x0 : x0 + patch]
            win = canvas[y0 : y0 + patch, x0 : x0 + patch]
            m = win_known.astype(np.float64)
            mask = np.zeros((h, w))
            mask[:patch, :patch] = m
            fmask = np.fft.rfft2(mask)
            weight = _blur_wrap(known.astype(np.float64), tone_radius)
            plain = []
            broad = []
            for c in range(3):
                mv = np.zeros((h, w))
                mv[:patch, :patch] = m * win[:, :, c]
                plain.append(np.fft.rfft2(mv))
                low = _blur_wrap(np.where(known, canvas[:, :, c], 0.0), tone_radius) / np.maximum(weight, 1e-6)
                lw = low[y0 : y0 + patch, x0 : x0 + patch]
                lv = np.zeros((h, w))
                lv[:patch, :patch] = m * lw
                broad.append((np.fft.rfft2(lv), (m * lw**2).sum()))
            errs = []
            for si, (fsq, fsrc, flsq, flow) in enumerate(ffts):
                err = np.zeros((h, w))
                for c in range(3):
                    err += np.fft.irfft2(fsq[c] * np.conj(fmask), s=(h, w))
                    err -= 2 * np.fft.irfft2(fsrc[c] * np.conj(plain[c]), s=(h, w))
                    err += (m * win[:, :, c] ** 2).sum()
                    err += tone * np.fft.irfft2(flsq[c] * np.conj(fmask), s=(h, w))
                    err -= 2 * tone * np.fft.irfft2(flow[c] * np.conj(broad[c][0]), s=(h, w))
                    err += tone * broad[c][1]
                err = np.maximum(err, 0) / max(m.sum(), 1.0)
                if open_sources:  # a patch may not run over the picture's own edge
                    err[h - patch + 1 :, :] = np.inf
                    err[:, w - patch + 1 :] = np.inf
                if si == 0 and natural is not None:
                    err = np.where(_near((h, w), (y0 - natural[0]) % h, (x0 - natural[1]) % w, apart), np.inf, err)
                for tsi, ty, tx in taken.get((y0, x0), []):
                    if tsi == si:
                        err = np.where(_near((h, w), ty, tx, apart), np.inf, err)
                errs.append(err)
            best = min(float(e.min()) for e in errs)
            ok = [
                (si, cy, cx) for si, e in enumerate(errs) for cy, cx in np.argwhere(e <= best * (1 + tolerance) + 1.0)
            ]
            si, cy, cx = ok[chance.below(len(ok))]
            taken.setdefault((y0, x0), []).append((int(si), int(cy), int(cx)))
            new = srcs[si][np.ix_((cy + np.arange(patch)) % h, (cx + np.arange(patch)) % w)]
            diff = ((new - win) ** 2).sum(axis=2)
            use = np.ones((patch, patch), bool)
            cols, rows = win_known.all(axis=0), win_known.all(axis=1)
            fcols, frows = win_fixed.all(axis=0), win_fixed.all(axis=1)
            wl, wr, wt, wb = _run(cols), _run(cols[::-1]), _run(rows), _run(rows[::-1])
            if wl:
                path = _cut(diff[:, :wl], _run(fcols))
                for i in range(patch):
                    use[i, : path[i]] = False
            if wt:
                path = _cut(diff[:wt, :].T, _run(frows))
                for j in range(patch):
                    use[: path[j], j] = False
            if wr:
                path = _cut(diff[:, patch - wr :][:, ::-1], _run(fcols[::-1]))
                for i in range(patch):
                    use[i, patch - path[i] :] = False
            if wb:
                path = _cut(diff[patch - wb :, :][::-1, :].T, _run(frows[::-1]))
                for j in range(patch):
                    use[patch - path[j] :, j] = False
            use |= ~win_known
            use &= ~win_fixed
            canvas[y0 : y0 + patch, x0 : x0 + patch] = np.where(use[:, :, None], new, win)
            known[y0 : y0 + patch, x0 : x0 + patch] = True
    out = np.clip(np.rint(canvas), 0, 255).astype(np.uint8)
    out[fixed] = first[fixed]
    return out


def flatten_border(tile, depth, radius=12):
    """The tile with the broad tone of its border taken out, in two steps, each in full within `depth` of the edge and
    fading to nothing over the same distance again: the tile's own blur over `radius`, less the tile's mean, removed
    from the pixels; then the mean of each row and column, less the tile's mean. The fine grain stays. A ring that every
    version shares would otherwise carry the tile's swathes of tone to every edge, the same at each, and a straight line
    of them, lighter or darker than the rest, would show along every join."""
    if depth <= 0:  # a ground of big marks (cobbles) is not flattened: it would wash out whatever lies by the edge
        return tile
    n = tile.shape[0]
    f = tile.astype(np.float64)
    mean = f.reshape(-1, 3).mean(axis=0)
    low = np.stack([_blur_wrap(f[:, :, c], radius) for c in range(3)], axis=2)
    taper = np.clip(2.0 - np.minimum(np.arange(n), np.arange(n)[::-1]) / float(depth), 0.0, 1.0)
    f = f - np.maximum(taper[:, None], taper[None, :])[:, :, None] * (low - mean)
    f = f - taper[:, None, None] * (f.mean(axis=1) - mean)[:, None, :]
    f = f - taper[None, :, None] * (f.mean(axis=0) - mean)[None, :, :]
    return np.clip(np.rint(f), 0, 255).astype(np.uint8)


def make_versions(tile, count, ring, overlap, patch, seed, flatten=None, others=(), deeper=()):
    """`count` versions of a wrapping tile that share their ring: the first is the tile itself, shifted so its ring is
    its most ordinary part and its border's broad tone flattened over `flatten` pixels (the overlap if none), and each
    after it is quilted from the tile (and from any `others`, further tiles of the same material) round that ring.
    `deeper` are the pictures drawn for the levels below, of the first source (see neutral_shift). Returns the
    versions and the shift."""
    dy, dx = neutral_shift(tile, overlap, deeper=deeper)
    first = flatten_border(roll(tile, dy, dx), overlap if flatten is None else flatten)
    chance = Chance(seed)
    taken = {}
    versions = [first]
    for _ in range(count - 1):
        versions.append(quilt([tile, *others], first, ring, overlap, patch, chance, (dy, dx), taken))
    return versions, (dy, dx)


def requilt(sources, shift, count, ring, overlap, patch, seed, flatten=None, scale=2):
    """`count` versions of a level drawn for each source, quilted at that level's own size, with its own `ring`,
    `overlap` and `patch` in its texture pixels: the first is the first source shifted as the first level was (`shift`
    is the first level's, `scale` times larger) and flattened at its border; each after it is quilted from the sources
    round that ring, then flattened at its border again with the ring kept, so the broad tone near the edges is the
    same on every version and no dark or light line shows along the joins. The cuts follow this level's own gaps
    between marks, which is why they are not the first level's laid over it: a level drawn on its own seldom lines up
    with the one above closely enough for that, and its marks would be sliced."""
    dy, dx = shift[0] // scale, shift[1] // scale
    depth = overlap if flatten is None else flatten
    first = flatten_border(roll(sources[0], dy, dx), depth)
    fixed = ring_mask(first.shape[0], ring)
    chance = Chance(seed)
    taken = {}
    versions = [first]
    for _ in range(count - 1):
        made = quilt(
            sources,
            first,
            ring,
            overlap,
            patch,
            chance,
            (dy, dx),
            taken,
            apart=max(4, 24 // scale),
            tone_radius=max(1, 5 // scale),
        )
        made = flatten_border(made, depth)
        made[fixed] = first[fixed]
        versions.append(made)
    return versions


# ---- drawings that sit on no exact grid and do not wrap (a picture from the image tool) ------------------------------


def _cell_edges(jumps, block):
    """Where the cells of pixel art begin along one axis: the first pixel of each whole cell, from the profile of how
    much the picture jumps between neighbouring pixels (jumps[i] is the jump from pixel i to i + 1). The period is
    searched within a tenth of `block` either side and the phase over one period, for the grid whose cell edges
    carry the most jump; the partial cells at either end are left out. Returns the cells' first pixels and the end of
    the last whole cell."""
    n = len(jumps) + 1
    best = None
    for period in np.arange(block * 0.9, block * 1.1, block * 0.0005):
        for phase in np.arange(0.0, period, 0.25):
            edges = np.round(phase + np.arange(0, int((n - phase) / period) + 1) * period).astype(int)
            edges = edges[(edges >= 1) & (edges <= n - 1)]
            score = jumps[edges - 1].mean()
            if best is None or score > best[0]:
                best = (score, period, phase)
    _, period, phase = best
    edges = np.round(phase + np.arange(0, int((n - phase) / period) + 1) * period).astype(int)
    return edges[edges <= n]


def snap(picture, block, cells=None):
    """A drawing put on its own grid of texture pixels: a picture from the image tool (about 1254 pixels across for 1024
    asked) is pixel art in cells of about `block` pixels, some a pixel wider or narrower, and not an exact multiple of
    anything. The cells' edges are found (_cell_edges), each cell becomes one texture pixel, the median colour of the
    middle of its area, and the loss is the share of the picture's pixels (within the cells) that differ from their
    cell's colour by more than a little (the sum of the channels' differences over 24). Where the edges are too soft
    to find (a level drawn bigger and blurrier than asked), `cells` fixes the number of cells across the whole
    picture instead, as many as the drawing was asked to have, evenly spaced. Returns (texels, loss); the texels do not
    wrap."""
    f = picture.astype(np.int32)
    if cells:
        ey = np.round(np.arange(cells + 1) * picture.shape[0] / cells).astype(int)
        ex = np.round(np.arange(cells + 1) * picture.shape[1] / cells).astype(int)
        block = picture.shape[0] / cells
    else:
        ey = _cell_edges(np.abs(np.diff(f, axis=0)).sum(axis=2).mean(axis=1), block)
        ex = _cell_edges(np.abs(np.diff(f, axis=1)).sum(axis=2).mean(axis=0), block)
    middle = max(1, int(round(block * 0.5)))

    def centres(edges):
        widths = np.diff(edges)
        starts = edges[:-1] + (widths - middle) // 2
        return np.clip(starts[:, None] + np.arange(middle)[None, :], 0, None)

    ys, xs = centres(ey), centres(ex)
    gathered = picture[ys[:, None, :, None], xs[None, :, None, :]]  # rows of cells, columns of cells, y, x, colour
    texels = np.median(gathered.reshape(len(ys), len(xs), middle * middle, 3), axis=2).round().astype(np.uint8)
    rows = np.repeat(np.arange(len(ys)), np.diff(ey))
    cols = np.repeat(np.arange(len(xs)), np.diff(ex))
    inside = f[ey[0] : ey[-1], ex[0] : ex[-1]]
    far = np.abs(texels[rows[:, None], cols[None, :]].astype(np.int32) - inside).sum(axis=2) > 24
    return texels, float(far.mean())


def frame_of(source, overlap, n):
    """The border of an `n` x `n` tile that wraps, cut from a picture that does not: from the middle `n` x `n` of the
    picture, a band of 2 x `overlap` rows from where its edges are most ordinary (neutral_shift), whose upper half lies
    along the bottom of the tile and its lower half along the top, so the two continue each other across the wrap, and
    in the same way a band of columns for the left and right; the four corners are the four quarters of the square
    where the two bands cross, so where four tiles meet they are neighbours in the picture. Along a strip's two ends,
    where it meets a corner, there is a small mismatch in the outer pixels. Returns the tile with only its border
    filled in."""
    h, w = source.shape[:2]
    cy, cx = (h - n) // 2, (w - n) // 2
    crop = source[cy : cy + n, cx : cx + n]
    dy, dx = neutral_shift(crop, overlap)
    by = min(max((-dy) % n - overlap, 0), n - 2 * overlap)
    bx = min(max((-dx) % n - overlap, 0), n - 2 * overlap)
    o = overlap
    rows = crop[by : by + 2 * o, :]
    cols = crop[:, bx : bx + 2 * o]
    corner = crop[by : by + 2 * o, bx : bx + 2 * o]  # where the two bands cross
    frame = np.zeros_like(crop)
    frame[o : n - o, :o] = cols[o : n - o, o:]
    frame[o : n - o, n - o :] = cols[o : n - o, :o]
    frame[:o, o : n - o] = rows[o:, o : n - o]
    frame[n - o :, o : n - o] = rows[:o, o : n - o]
    # the four corners are the four quarters of the crossing, so where four tiles meet they are neighbours in it
    frame[:o, :o] = corner[o:, o:]
    frame[:o, n - o :] = corner[o:, :o]
    frame[n - o :, :o] = corner[:o, o:]
    frame[n - o :, n - o :] = corner[:o, :o]
    return frame


def make_versions_open(sources, count, ring, overlap, patch, seed, n, scale=1):
    """`count` versions of a tile `n` pixels across, quilted from drawings that do not wrap (all of one size, larger
    than the tile, such as snap() gives): they share the ring of a frame cut from the first drawing (frame_of), so any
    two join without a seam, and every one is made the same way, none being the drawing itself. `scale` is how many
    times smaller the level is than the first one (the exclusion distance and the tone's blur shrink with it)."""
    frame = frame_of(sources[0], overlap, n)
    chance = Chance(seed)
    taken = {}
    return [
        quilt(
            sources,
            frame,
            ring,
            overlap,
            patch,
            chance,
            None,
            taken,
            apart=max(4, 24 // scale),
            tone_radius=max(1, 5 // scale),
            open_sources=True,
        )
        for _ in range(count)
    ]


def seam_strip(texels, width, period):
    """The seam strip of a hide from a drawing of it: the first `width` columns of the drawing, a pattern that repeats
    every `period` rows (a running stitch), each place of the pattern the colour most of its repeats have, so a stray
    cell in one repeat is not carried. Returns `period` rows of `width` pixels."""
    repeats = texels.shape[0] // period
    out = np.zeros((period, width, texels.shape[2]), np.uint8)
    for r in range(period):
        for c in range(width):
            colours, counts = np.unique(texels[r : repeats * period : period, c], axis=0, return_counts=True)
            out[r, c] = colours[counts.argmax()]
    return out


def lay_strip(tile, strip):
    """The tile with a seam strip down its first columns, the strip's rows repeated to the tile's height, which must be
    a whole number of repeats so the strip wraps with the tile."""
    if tile.shape[0] % strip.shape[0]:
        raise ValueError(f"a strip of {strip.shape[0]} rows does not wrap a tile {tile.shape[0]} rows high")
    out = tile.copy()
    out[:, : strip.shape[1]] = np.tile(strip, (tile.shape[0] // strip.shape[0], 1, 1))
    return out


# ---- levels -----------------------------------------------------------------------------------------------------


def halve(level):
    """The level below by plain averaging of each 2 x 2 block (the start of a designed level, never its end)."""
    f = level.astype(np.float64)
    a = (f[0::2, 0::2] + f[1::2, 0::2] + f[0::2, 1::2] + f[1::2, 1::2]) / 4.0
    return a


def sharpen(a, amount):
    """A small tile's detail raised by an unsharp mask that wraps: the tile plus `amount` times its difference from its
    own 3 x 3 blur."""
    blur = a
    kernel = {-1: 0.25, 0: 0.5, 1: 0.25}
    for axis in (0, 1):
        blur = sum(np.roll(blur, d, axis=axis) * k for d, k in kernel.items())
    return a + amount * (a - blur)


def reduce(level, amount):
    """The level below made by code: averaged, then sharpened by `amount` and rounded to 8 bits."""
    return np.clip(np.rint(sharpen(halve(level), amount)), 0, 255).astype(np.uint8)


def complete_chain(levels):
    """A tile's levels down to one texture pixel, the engine's rule: the designed levels as given, each one after them
    the average of the one before."""
    out = list(levels)
    while out[-1].shape[0] > 1:
        out.append(np.clip(np.rint(halve(out[-1])), 0, 255).astype(np.uint8))
    return out


def impose_ring(level, ring_level, ring):
    """The level with the ring of another level put in its place, since versions share their ring at every level."""
    out = level.copy()
    mask = ring_mask(level.shape[0], ring)
    out[mask] = ring_level[mask]
    return out


# ---- marks: light marks on see-through ground (the water's current, foam and ripples, A4.5) -------------------
#
# A picture of marks is drawn on a background of one key colour. Everything above (versions, rings, quilting) works
# on it as it is, the key colour a colour like any other; only at the end is it made see-through (to_rgba). The levels
# below the first are made here, since averaging marks would mix them with the key.


def key_colour(text):
    """A colour written #rrggbb as three 8-bit numbers."""
    h = text.lstrip("#")
    return np.array([int(h[i : i + 2], 16) for i in (0, 2, 4)], np.uint8)


def key_mask(picture, key):
    """Where a picture of marks holds a mark: every pixel that is not the key colour."""
    return (picture != np.asarray(key, np.uint8)).any(axis=2)


def to_rgba(picture, key, bleed):
    """A picture of marks on its key colour as texture pixels with alpha: each mark opaque, everything else wholly
    see-through with the colour `bleed`, so no fringe of the key colour or of black shows where the engine blends a
    mark with its neighbours."""
    mask = key_mask(picture, key)
    out = np.empty(picture.shape[:2] + (4,), np.uint8)
    out[..., :3] = np.where(mask[..., None], picture, np.asarray(bleed, np.uint8))
    out[..., 3] = np.where(mask, 255, 0)
    return out


def _marks_of(mask):
    """The marks of a wrapping mask as lists of their pixels: eight-connected runs of marked pixels, found by flood fill
    in reading order (so the same mask always gives the same list)."""
    n = mask.shape[0]
    seen = np.zeros_like(mask)
    found = []
    for y, x in np.argwhere(mask):
        if seen[y, x]:
            continue
        stack, body = [(int(y), int(x))], []
        seen[y, x] = True
        while stack:
            cy, cx = stack.pop()
            body.append((cy, cx))
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    ny, nx = (cy + dy) % n, (cx + dx) % n
                    if mask[ny, nx] and not seen[ny, nx]:
                        seen[ny, nx] = True
                        stack.append((ny, nx))
        found.append(body)
    return found


def reduce_marks(picture, key, keep, seed):
    """The level below a picture of marks, made by code: each 2 x 2 block is a mark where two or more of its four
    texture pixels are (so a single speck is gone and a streak is its length over two, bolder for its thickness being
    the new texture pixel's), then of the marks that remain only `keep` percent are kept, chosen by a hash of each
    one's first pixel and the seed, so the level shows bolder marks and fewer of them (A5.3), never the level above
    averaged. A kept mark takes the lightest colour of its block."""
    n = picture.shape[0]
    h = n // 2
    mask = key_mask(picture, key)
    blocks = picture.reshape(h, 2, h, 2, 3).transpose(0, 2, 1, 3, 4).reshape(h, h, 4, 3).astype(np.int32)
    marked = mask.reshape(h, 2, h, 2).transpose(0, 2, 1, 3).reshape(h, h, 4)
    lightest = np.where(marked, blocks.sum(axis=3), -1).argmax(axis=2)
    colours = np.take_along_axis(blocks, lightest[:, :, None, None], axis=2)[:, :, 0, :]
    pooled = marked.sum(axis=2) >= 2
    kept = np.zeros_like(pooled)
    for body in _marks_of(pooled):
        y, x = body[0]
        if Chance(seed * 1000003 + y * 4099 + x).below(100) < keep:
            for by, bx in body:
                kept[by, bx] = True
    out = np.empty((h, h, 3), np.uint8)
    out[:] = np.asarray(key, np.uint8)
    out[kept] = colours[kept].astype(np.uint8)
    return out


def coverage(pixels):
    """The share of a picture of marks (RGBA) that is marked, from its alpha."""
    return float((pixels[..., 3] > 0).mean())


# ---- seams ------------------------------------------------------------------------------------------------------


def _jump(a, b):
    """The mean absolute colour difference between two equal strips of pixels, as a sum over the channels."""
    return float(np.abs(a.astype(np.int32) - b.astype(np.int32)).sum(axis=-1).mean())


def _pair_jumps(tile):
    """The mean jump between each neighbouring pair of columns and of rows inside the tile: (columns, rows)."""
    t = tile.astype(np.int32)
    return np.abs(np.diff(t, axis=1)).sum(axis=2).mean(axis=0), np.abs(np.diff(t, axis=0)).sum(axis=2).mean(axis=1)


def wrap_ratio(tile, across=True):
    """How the tile joins itself, the worse of its two directions: the jump across its wrap over the 90th percentile of
    the jumps between its neighbouring columns (or rows), so 1 or less is no seam. With `across` False only the wrap
    from the bottom to the top counts (a hide's field, whose left edge is next to its seam strip, wraps only so)."""
    cols, rows = _pair_jumps(tile)
    down = _jump(tile[-1, :], tile[0, :]) / np.percentile(rows, 90)
    return max(down, _jump(tile[:, -1], tile[:, 0]) / np.percentile(cols, 90)) if across else down


def join_ratio(a, b, across=True):
    """How tile `a` joins tile `b` laid to its right and below it, as wrap_ratio does, against the larger of the two
    tiles' 90th percentiles; with `across` False only below it."""
    ac, ar = _pair_jumps(a)
    bc, br = _pair_jumps(b)
    cols = max(np.percentile(ac, 90), np.percentile(bc, 90))
    rows = max(np.percentile(ar, 90), np.percentile(br, 90))
    down = _jump(a[-1, :], b[0, :]) / rows
    return max(_jump(a[:, -1], b[:, 0]) / cols, down) if across else down


def inner_seams(tile):
    """The hard rows and columns inside a level, as (axis name, the pair's first index, its step, the 99th percentile of
    the steps, the next largest step): a row or column pair whose mean lightness step is above 1.6 times the 99th
    percentile of all of them, above 1.3 times the next largest and above 8, where the level is big enough (32 texture
    pixels) to have a 99th percentile of its own. A version whose quilted patches stop short of its shared border
    shows one, a straight line at every cell's edge (the builder's rule, written once here)."""
    if tile.shape[0] < 32:
        return []
    lightness = tile[..., :3].astype(np.float64).mean(axis=2)
    found = []
    for axis, label in ((0, "row"), (1, "col")):
        step = np.abs(np.diff(lightness, axis=axis)).mean(axis=1 - axis)
        order = np.argsort(step)[::-1]
        top, second = step[order[0]], step[order[1]]
        p99 = float(np.percentile(step, 99))
        if top > 1.6 * p99 and top > 1.3 * second and top > 8:
            found.append((label, int(order[0]), float(top), p99, float(second)))
    return found


def shares_ring(versions, ring):
    """Whether every version keeps the same ring: all pixels of the border strips equal."""
    mask = ring_mask(versions[0].shape[0], ring)
    return all((v[mask] == versions[0][mask]).all() for v in versions[1:])
