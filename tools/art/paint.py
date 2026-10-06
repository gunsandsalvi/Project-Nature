"""Textures drawn to a layout (A6.4): a figure's skin, hair and garments drawn to fit the pieces its parts lay in
their atlas, read from layout.py's JSON.

A Layout rasterizes one atlas's triangles at 64 texture pixels a metre into a square canvas (its side the atlas's
larger side, rounded up to a power of two), so a painter knows for each texture pixel whether a face lies on it,
which part, role and bone it belongs to, and where it sits on the part at rest: its position and normal in metres.
The canvas is laid as Blender shows texture coordinates on an image, v = 0 along its bottom row. The helpers give
each texture pixel's distance to its piece's edges, so seams and hems fall where the piece wraps or ends, and
spread the pieces' colours into the gaps between them, so a level below never mixes in the background.

Implements PRE-27 and PRE-46, see A6.3 and A6.4.
"""

import json

import numpy as np

TEXELS_A_METRE = 64


def load(path):
    with open(path) as f:
        return json.load(f)


class Layout:
    """One atlas of a layout: covered, part, role, bone (each texture pixel's, -1 where no face lies), position and
    normal (metres, at rest), all arrays of the canvas's size, the image's top row first."""

    def __init__(self, data, atlas, texels_a_metre=TEXELS_A_METRE):
        names = list(data["atlases"])
        if atlas not in names:
            raise ValueError(f"no atlas {atlas!r} in the layout; it has {', '.join(names)}")
        w, h = data["atlases"][atlas]
        self.k = texels_a_metre
        need = max(1, int(np.ceil(max(w, h) * texels_a_metre - 1e-6)))
        self.side = 1 << (need - 1).bit_length()
        self.parts, self.roles, self.bones = data["parts"], data["roles"], data["bones"]
        s = self.side
        self.covered = np.zeros((s, s), bool)
        self.part = np.full((s, s), -1)
        self.role = np.full((s, s), -1)
        self.bone = np.full((s, s), -1)
        self.position = np.zeros((s, s, 3))
        self.normal = np.zeros((s, s, 3))
        a = names.index(atlas)
        for t in data["triangles"]:
            if t[0] == a:
                self._lay(t)

    def _lay(self, t):
        uv = np.array(t[6:12], float).reshape(3, 2) * self.k
        pos = np.array(t[12:21], float).reshape(3, 3)
        nor = np.array(t[21:30], float).reshape(3, 3)
        lo = np.floor(uv.min(axis=0) - 0.5).astype(int)
        hi = np.ceil(uv.max(axis=0) + 0.5).astype(int)
        xs = np.arange(max(0, lo[0]), min(self.side, hi[0]))
        ys = np.arange(max(0, lo[1]), min(self.side, hi[1]))
        if len(xs) == 0 or len(ys) == 0:
            return
        px, py = np.meshgrid(xs + 0.5, ys + 0.5)
        (x0, y0), (x1, y1), (x2, y2) = uv
        det = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2)
        if abs(det) < 1e-12:
            return
        l0 = ((y1 - y2) * (px - x2) + (x2 - x1) * (py - y2)) / det
        l1 = ((y2 - y0) * (px - x2) + (x0 - x2) * (py - y2)) / det
        l2 = 1 - l0 - l1
        inside = (l0 >= -1e-6) & (l1 >= -1e-6) & (l2 >= -1e-6)
        if not inside.any():
            return
        bary = np.stack([l0, l1, l2], -1)[inside]
        cols = (px[inside] - 0.5).astype(int)
        rows = self.side - 1 - (py[inside] - 0.5).astype(int)
        self.covered[rows, cols] = True
        self.part[rows, cols] = t[1]
        self.role[rows, cols] = t[2]
        corner = np.argmax(bary, axis=1)
        self.bone[rows, cols] = np.array(t[3:6])[corner]
        self.position[rows, cols] = bary @ pos
        n = bary @ nor
        self.normal[rows, cols] = n / np.maximum(np.linalg.norm(n, axis=1, keepdims=True), 1e-9)

    def where(self, role=None, part=None):
        """A mask of the texture pixels of a role and/or part, by name (a part's name may be a prefix)."""
        m = self.covered.copy()
        if role is not None:
            m &= self.role == (self.roles.index(role) if role in self.roles else -2)
        if part is not None:
            ids = [i for i, p in enumerate(self.parts) if p == part or p.startswith(part)]
            m &= np.isin(self.part, ids)
        return m

    def edges(self):
        """Each texture pixel's distance, in texture pixels, to the nearest one with no face along its row to the
        left and right, and along its column above and below: (left, right, up, down)."""
        out = []
        for axis, flip in ((1, False), (1, True), (0, False), (0, True)):
            c = self.covered[:, ::-1] if axis == 1 and flip else self.covered
            c = c[::-1] if axis == 0 and flip else c
            run = np.zeros(c.shape, int)
            n = c.shape[axis]
            for i in range(n):
                cur = np.take(c, i, axis=axis)
                prev = np.take(run, i - 1, axis=axis) if i else np.zeros(cur.shape, int)
                val = np.where(cur, prev + 1, 0)
                if axis == 1:
                    run[:, i] = val
                else:
                    run[i, :] = val
            run = run[:, ::-1] if axis == 1 and flip else run
            run = run[::-1] if axis == 0 and flip else run
            out.append(run)
        return tuple(out)


def spread(image, covered, rounds=8):
    """The pieces' colours spread into the texture pixels round them with no face, one ring a round, so a level
    below never takes the background into a piece's edge."""
    img = image.copy()
    have = covered.copy()
    for _ in range(rounds):
        grown = have.copy()
        for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            src_have = np.roll(np.roll(have, dy, 0), dx, 1)
            src = np.roll(np.roll(img, dy, 0), dx, 1)
            take = src_have & ~grown
            img[take] = src[take]
            grown |= take
        have = grown
    return img


GAP_ROUNDS = 4  # texture pixels the pieces' colours spread into the gaps round them, at every level


def fill_gaps(image, covered, flat, rounds=GAP_ROUNDS):
    """The pieces' colours spread `rounds` texture pixels into the gaps round them, the rest of the canvas `flat`."""
    out = spread(np.asarray(image, np.uint8), covered, rounds)
    reached = spread(covered[..., None].astype(np.uint8), covered, rounds)[..., 0] > 0
    out[~reached] = flat
    return out


def halve(covered):
    """The covered mask of the level below: a texture pixel is covered where any of the 2 x 2 above it is."""
    h, w = covered.shape[0] // 2, covered.shape[1] // 2
    return covered[: 2 * h, : 2 * w].reshape(h, 2, w, 2).any(axis=(1, 3))


def save(folder, image, layout, top, root):
    """A drawn atlas as a material: its pieces' colours spread 4 texture pixels into the gaps round them and the rest
    of the canvas one flat colour (their mean), band 0, and the levels below it by the code reduction (levels.py,
    as a drawn texture: no marks drawn again, its contrast as the reduction leaves it), each with its pieces' colours
    spread again into its own gaps before its colours are matched, so a piece's edge never takes the flat colour at
    any level; layout.png (white
    where a face lies, so the checks know it is drawn to a layout, not tiled) and its record, whose `top` holds the
    record's keys above its bands. Returns the levels."""
    import os

    import levels
    import look
    import match
    import record
    import texels

    os.makedirs(folder, exist_ok=True)
    flat = np.round(np.asarray(image, float)[layout.covered].mean(axis=0)).astype(np.uint8)
    b0 = fill_gaps(image, layout.covered, flat)
    acc0, colour = match.accents(b0), look.stats(b0)
    made, above, covered = [], b0, layout.covered
    while min(above.shape[:2]) > 1:
        covered = halve(covered)
        step = levels.next_level(
            above, acc0, colour=colour, drawn=True, fill=lambda t, c=covered: fill_gaps(t, c, flat)
        )
        made.append(step)
        above = step["level"]
    all_levels = [b0] + [s["level"] for s in made]
    bands = []
    for n, t in enumerate(all_levels):
        path = os.path.join(folder, f"b{n}.png")
        texels.save_png(t, path)
        b = {"level": n, "file": os.path.relpath(path, root), "sha256": texels.sha256(path)}
        if n:
            b["made_from"] = bands[-1]["sha256"]
            b["way"] = (
                levels.way(made[n - 1]["marks"], made[n - 1]["most"])
                + f"; before its colours were matched, its pieces' colours spread {GAP_ROUNDS} texture pixels into "
                "its gaps again, the rest flat"
            )
            b["calibration"] = levels.calibration(made[n - 1])
        else:
            b["way"] = (
                f"drawn by code to the atlas's layout, its pieces' colours spread {GAP_ROUNDS} texture pixels into "
                "the gaps round them, the rest of the canvas their mean colour"
            )
        bands.append(b)
    mask = np.repeat((layout.covered * 255).astype(np.uint8)[..., None], 3, axis=2)
    texels.save_png(mask, os.path.join(folder, "layout.png"))
    rec = dict(top)
    rec["tile_texels"] = layout.side
    rec["texels_a_metre"] = TEXELS_A_METRE
    rec["first_band"] = 0
    rec["band"] = bands
    record.write(os.path.join(folder, "record.toml"), rec)
    return all_levels


def field(shape, scale, seed):
    """Smooth noise with features about `scale` texture pixels across, deviation 1, wrapping round the canvas."""
    rng = np.random.default_rng(seed)
    h, w = shape
    f = np.hypot(np.fft.fftfreq(w)[None, :], np.fft.fftfreq(h)[:, None])
    spec = np.exp(-0.5 * (f * scale) ** 2) * (rng.normal(size=(h, w)) + 1j * rng.normal(size=(h, w)))
    out = np.real(np.fft.ifft2(spec))
    return (out - out.mean()) / (out.std() + 1e-12)


def shades(base, steps, field_, cuts):
    """A colour for each texture pixel from a field cut into len(steps) shades: base plus each step (RGB)."""
    idx = np.searchsorted(np.quantile(field_, cuts), field_)
    table = np.clip(np.array(base, float)[None, :] + np.array(steps, float), 0, 255)
    return table[idx]


def strokes(shape, along, length, share, seed):
    """A mask of short strokes `length` texture pixels long, running along axis `along` (0 down the columns, 1
    across the rows), covering about `share` of the canvas: hair, fur and grain."""
    rng = np.random.default_rng(seed)
    out = np.zeros(shape, bool)
    count = int(share * shape[0] * shape[1] / length)
    ys = rng.integers(0, shape[0], count)
    xs = rng.integers(0, shape[1], count)
    for k in range(length):
        if along == 0:
            out[(ys + k) % shape[0], xs] = True
        else:
            out[ys, (xs + k) % shape[1]] = True
    return out
