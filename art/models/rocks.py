"""The kit's stones and rocks (T2.3a.4, PRE-46, A6.1, A6.4): hearth stones, tent-ring stones, boulders, limestone
blocks fallen from the cliff and scree, built in Blender with tools/art/kit.py.

    blender -b --factory-startup --python art/models/rocks.py -- art/models/rocks.blend

GPT's first pass (art/requests/rocks-blend-01.txt) set the names, groups, sizes and seeds; the art lane's fixes gave
every stone its own shape: GPT's boulders, blocks and scree were each one shape scaled, and its stones were built
from regular rings of points, so they read as cushions and boxes. Each stone here is the convex hull of points of its
own, lumpy where water wore it, cut flat where it broke, rounded where it weathered; its texture is laid from the
direction each face looks (A6.4), and it rests on its origin.
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "art"))
import kit  # noqa: E402


def direction(rng):
    """A random direction, evenly over the sphere."""
    while True:
        d = (rng.gauss(0, 1), rng.gauss(0, 1), rng.gauss(0, 1))
        n = math.sqrt(sum(x * x for x in d))
        if n > 1e-6:
            return tuple(x / n for x in d)


def lumpy(rng, half, n, bumps=3, amp=0.14):
    """n points on an ellipsoid of half-sizes `half`, pushed out and in by a few broad bumps, so each stone's outline
    is its own: an egg, a loaf, a stone with one fuller end."""
    hills = [(direction(rng), rng.uniform(-amp, amp)) for _ in range(bumps)]
    pts = []
    for _ in range(n):
        d = direction(rng)
        f = 1 + sum(w * max(0.0, sum(a * b for a, b in zip(d, c, strict=True))) ** 2 for c, w in hills)
        pts.append(tuple(d[i] * half[i] * f for i in range(3)))
    return pts


def cut(pts, rng, planes, keep=(0.6, 0.85), level=False):
    """The points with flat faces broken into them: each plane, its normal random (level ones keep near the
    horizontal), lies at a share `keep` of the points' reach that way, and every point beyond it is laid onto it."""
    for _ in range(planes):
        d = direction(rng)
        if level:
            d = (d[0], d[1], d[2] * 0.25)
            m = math.sqrt(sum(x * x for x in d))
            d = tuple(x / m for x in d)
        reach = max(sum(p[i] * d[i] for i in range(3)) for p in pts)
        at = reach * rng.uniform(*keep)
        out = []
        for p in pts:
            s = sum(p[i] * d[i] for i in range(3)) - at
            out.append(tuple(p[i] - s * d[i] for i in range(3)) if s > 0 else p)
        pts = out
    return pts


def stone(name, group, about, pts, rounds, smooth, flat_base_share=None):
    """A stone from its points: their hull, rounded `rounds` times, flattened underneath by a share of its height so
    it rests, stood on its origin, its texture laid from the way each face looks, and its joint where it rests."""
    p = kit.Part(name, group=group, about=about)
    p.smooth = smooth
    lo = min(q[2] for q in pts)
    hi = max(q[2] for q in pts)
    base = None if flat_base_share is None else lo + flat_base_share * (hi - lo)
    p.hull(pts, rounds=rounds, flat_base=base)
    size = p.rest()
    p.project("stone")
    p.joint("base", towards=(0, 0, -1))
    p.about = f"{about}, {size.x:.2f} x {size.y:.2f} x {size.z:.2f} m"
    p.done()


kit.start()

# Hearth stones: water-worn cobbles of hard pale rock, each its own egg or loaf, flattened just enough to rest.
hearth = [(0.18, 0.12, 0.08), (0.22, 0.14, 0.10), (0.25, 0.17, 0.11), (0.28, 0.16, 0.12), (0.30, 0.20, 0.14)]
hearth += [(0.24, 0.19, 0.09)]
for i, (x, y, z) in enumerate(hearth):
    rng = random.Random(12000 + i)
    pts = lumpy(rng, (x / 2, y / 2, z / 2), 22, bumps=3, amp=0.18)
    if i in (2, 5):  # a flatter face, as a cobble split or ground flat along one side
        pts = cut(pts, rng, 1, keep=(0.75, 0.85), level=True)
    stone(f"hearth_stone_{chr(97 + i)}", "hearth", "a water-worn river cobble that rings a hearth", pts, 1, 60, 0.12)

# Ring stones: heavier bank and cliff-foot stones that hold a tent's edge down, subangular: broken faces, blunt edges.
ring = [(0.25, 0.18, 0.12), (0.29, 0.22, 0.16), (0.34, 0.24, 0.19), (0.39, 0.26, 0.22), (0.45, 0.30, 0.25)]
ring += [(0.36, 0.28, 0.15)]
for i, (x, y, z) in enumerate(ring):
    rng = random.Random(13000 + i)
    pts = lumpy(rng, (x / 2, y / 2, z / 2), 26, bumps=4, amp=0.22)
    pts = cut(pts, rng, rng.randint(2, 4), keep=(0.55, 0.8))
    stone(f"ring_stone_{chr(97 + i)}", "ring", "a subangular stone that weighs down a tent's edge", pts, 1, 35, 0.15)

# Boulders on the bank: weathered, broad gently curved faces, one or two flatter sides, set into the ground.
for i, length in enumerate((0.6, 1.0, 1.5)):
    rng = random.Random(14000 + i)
    half = (length / 2, length * rng.uniform(0.28, 0.38), length * rng.uniform(0.22, 0.3))
    pts = lumpy(rng, half, 34, bumps=5, amp=0.2)
    pts = cut(pts, rng, rng.randint(1, 2), keep=(0.7, 0.85))
    stone(f"boulder_{chr(97 + i)}", "boulders", "a weathered boulder on the river bank", pts, 1, 40, 0.2)


def block(rng, length):
    """A limestone block fallen from the cliff: beds broken along joints at near right angles, so a slab with a
    nearly flat top and bottom tilted as it fell, steep sides with a few corners chipped off, its sides not quite
    plane; and a thinner bed above it broken back, a step. Returns the two slabs' points."""
    width = length * rng.uniform(0.55, 0.68)
    height = length * rng.uniform(0.33, 0.45)
    dip_x, dip_y = math.radians(rng.uniform(-6, 6)), math.radians(rng.uniform(-5, 5))

    def slab(x0, x1, y0, y1, z0, z1, chips):
        pts = []
        for x in (x0, x1):
            for y in (y0, y1):
                for z in (z0, z1):
                    pts.append((x, y, z))
        # the sides: points pushed out, so broken faces are not ruler-flat
        for _ in range(16):
            x, y = rng.uniform(x0, x1), rng.uniform(y0, y1)
            side = rng.randrange(4)
            x = (x0, x1, x, x)[side] + (-1, 1, 0, 0)[side] * rng.uniform(0.0, 0.05) * length
            y = (y, y, y0, y1)[side] + (0, 0, -1, 1)[side] * rng.uniform(0.0, 0.05) * length
            pts.append((x, y, rng.uniform(z0, z1)))
        # the bed's face: a few points a little proud of it, as a parting plane is never quite flat
        for _ in range(4):
            pts.append((rng.uniform(x0, x1) * 0.8, rng.uniform(y0, y1) * 0.8, z1 + rng.uniform(0.0, 0.02) * length))
        pts = cut(pts, rng, chips, keep=(0.72, 0.9))
        # the beds' tilt, as the block fell
        return [(x, y, z + x * math.tan(dip_x) + y * math.tan(dip_y)) for x, y, z in pts]

    main = slab(-length / 2, length / 2, -width / 2, width / 2, 0.0, height * 0.72, rng.randint(4, 7))
    back = rng.uniform(0.15, 0.35) * width
    top = slab(-length / 2 * 0.92, length / 2 * 0.88, -width / 2 + back, width / 2, height * 0.68, height, 3)
    return main, top


for i, length in enumerate((1.5, 2.2, 3.0)):
    rng = random.Random(15000 + i)
    p = kit.Part(f"cliff_piece_{chr(97 + i)}", group="cliff", about="a limestone block fallen from the cliff")
    p.smooth = 30
    for pts in block(rng, length):
        p.hull(pts, rounds=0)
    size = p.rest()
    p.project("stone")
    p.joint("base", towards=(0, 0, -1))
    p.about = f"a limestone block fallen from the cliff, its upper bed broken back, {size.x:.2f} x {size.y:.2f} m"
    p.done()

# Scree: sharp flattish chips and slabs of limestone, each a few broken faces, lying flat.
for i, length in enumerate((0.05, 0.08, 0.12, 0.16, 0.20, 0.25)):
    rng = random.Random(16000 + i)
    width = length * rng.uniform(0.5, 0.75)
    thick = length * rng.uniform(0.15, 0.3)
    corners = rng.randint(5, 7)
    pts = []
    for k in range(corners):
        a = 2 * math.pi * k / corners + rng.uniform(-0.3, 0.3)
        r = rng.uniform(0.75, 1.0)
        x, y = r * math.cos(a) * length / 2, r * math.sin(a) * width / 2
        pts.append((x, y, 0.0))
        if rng.random() < 0.8:
            s = rng.uniform(0.6, 0.95)  # the top face smaller where an edge broke at a slant
            pts.append((x * s, y * s, thick * rng.uniform(0.8, 1.0)))
    stone(f"scree_{chr(97 + i)}", "scree", "an angular limestone chip from the scree", pts, 0, 30)

kit.finish(sys.argv[-1])
