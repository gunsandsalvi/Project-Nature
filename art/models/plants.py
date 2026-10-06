"""The kit's plant parts (T2.3a.4, PRE-46, A6.1, A6.4): a downy birch's trunk in segments, branches, grass tufts
and leaf clusters as cut-out cards, built in Blender with tools/art/kit.py.

    blender -b --factory-startup --python art/models/plants.py -- art/models/plants.blend

GPT's first pass (art/requests/plants-blend-01.txt) made every part here; the art lane's fix: each trunk segment and
branch is one mesh whose texture wraps afresh where it narrows (kit's breaks), where GPT built sleeves end to end.
Cards' texture coordinates are their own size in metres, each card its own place in its tuft's or cluster's
design, drawn later with see-through gaps (phase 2).
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "art"))
import kit  # noqa: E402

DOWN = (0, 0, -1)
UP = (0, 0, 1)
rng = random.Random(12000)


def interp(points, radii, z):
    """The path's point and radius at height z."""
    for i in range(len(points) - 1):
        if points[i][2] <= z <= points[i + 1][2]:
            t = (z - points[i][2]) / (points[i + 1][2] - points[i][2])
            return tuple(a + (b - a) * t for a, b in zip(points[i], points[i + 1], strict=True)), radii[i] + t * (
                radii[i + 1] - radii[i]
            )
    raise ValueError(z)


def trunk(name, about, points, radii, breaks=(), sockets=(), broken=False):
    """A trunk segment along its path, open where it joins the next (or the ground), its texture wrapping afresh at
    each break; sockets are (height, angle round, degrees from upright) where branches plug in, on its surface."""
    p = kit.Part(name, group="trunks", about=about)
    p.joint("base", towards=DOWN)
    p.sleeve(
        points,
        radii,
        sides=16,
        role="bark",
        cap_role="wood",
        caps=(None, "broken" if broken else None),
        breaks=breaks,
        seed=113,
    )
    p.joint("top", at=points[-1], towards=UP)
    for i, (z, angle, inclination) in enumerate(sockets, 1):
        c, r = interp(points, radii, z)
        a = math.radians(angle)
        facet = (a % (2 * math.pi / 16)) - math.pi / 16  # on the faceted ring, not the circle round it
        surface_r = r * math.cos(math.pi / 16) / math.cos(facet)
        at = (c[0] + surface_r * math.cos(a), c[1] + surface_r * math.sin(a), z)
        t = math.radians(inclination)
        towards = (math.sin(t) * math.cos(a), math.sin(t) * math.sin(a), math.cos(t))
        p.joint(f"branch_{i}", at=at, towards=towards)
    p.done()


def living_branch(name, length, diameter, count, twigs):
    """A living branch growing from a trunk's socket along +z, bowing gently forward, narrowing to 1.5 cm in `count`
    wraps (each within the 1.5 line); its base open inside the trunk and its tip open where leaf clusters sit."""
    p = kit.Part(
        name,
        group="branches",
        about=f"a downy birch branch {length} m long, {diameter * 100:g} to 1.5 cm across",
    )
    directions = []
    for i in range(count):
        theta = math.radians(8 + 6 * i)
        directions.append((0.025 * math.sin(i * 1.4), -math.sin(theta), math.cos(theta)))
    points = [(0.0, 0.0, 0.0)]
    for i in range(2 * count):
        d = directions[min((i + 1) // 2, count - 1)]
        norm = math.sqrt(sum(x * x for x in d))
        step = length / (2 * count)
        points.append(tuple(a + step * b / norm for a, b in zip(points[-1], d, strict=True)))
    radii = [diameter / 2 * (0.015 / diameter) ** (i / (2 * count)) for i in range(2 * count + 1)]
    p.joint("base", towards=DOWN)
    p.sleeve(points, radii, sides=6, role="bark", caps=(None, None), breaks=tuple(range(2, 2 * count, 2)))
    for i in range(1, twigs + 1):
        t = i / (twigs + 1) * 2 * count
        j = min(int(t), len(points) - 2)
        f = t - j
        c = tuple(a + (b - a) * f for a, b in zip(points[j], points[j + 1], strict=True))
        r = radii[j] + (radii[j + 1] - radii[j]) * f
        sign = 1 if i % 2 else -1
        p.joint(f"twig_{i}", at=(c[0] + sign * r, c[1], c[2]), towards=(sign * 0.65, -0.30, 0.70))
    p.joint("tip", at=points[-1], towards=tuple(b - a for a, b in zip(points[-2], points[-1], strict=True)))
    p.done()


def tuft(name, width, heights, step, lean_range, rows):
    """Meadow grass as crossed cut-out cards round the origin, each leaning out, each its own place in the design."""
    p = kit.Part(
        name,
        group="tufts",
        about=f"a tuft of wild meadow grass {max(heights):g} m tall and {width:g} m wide, {len(heights)} cards",
    )
    p.joint("base", towards=DOWN)
    for i, height in enumerate(heights):
        lean = math.radians(rng.uniform(*lean_range))
        p.card(
            width, height, role="grass", yaw=math.radians(i * step), lean=lean, uv_at=(i * (width + 0.02), 0), rows=rows
        )
    p.done()


def cluster(name, size, count):
    """Birch leaves as cut-out cards round a short stem, turned every way, so a clump reads round from any side."""
    across = 0.25 if count == 4 else 0.45
    p = kit.Part(
        name,
        group="leaves",
        about=f"a clump of downy birch leaves about {across:g} m across, on {count} cut-out cards",
    )
    p.joint("stem", towards=DOWN)
    u = 0.0
    for i in range(count):
        width = size if count == 4 else rng.uniform(0.25, 0.35)
        height = size if count == 4 else rng.uniform(0.25, 0.32)
        yaw = math.radians(i * 360 / count + 12)
        lean = math.radians(83 if i == 0 else rng.uniform(30, 60))
        up = (-math.sin(yaw) * math.sin(lean), math.cos(yaw) * math.sin(lean), math.cos(lean))
        centre_z = 0.13 if count == 4 else 0.23
        at = tuple((0, 0, centre_z)[j] - up[j] * height / 2 for j in range(3))
        p.card(width, height, role="leaf", at=at, yaw=yaw, lean=lean, uv_at=(u, 0))
        u += width + 0.02
    p.done()


kit.start()

trunk(
    "birch_trunk_base",
    "the lowest 1.5 m of a downy birch, 38 cm across at its root flare to 27 cm",
    [(0, 0, 0), (0, 0, 0.15), (0.008, 0.004, 0.30), (0.018, 0.008, 0.70), (0.025, -0.003, 1.10)]
    + [(0.03, -0.006, 1.30), (0.03, -0.006, 1.50)],
    [0.19, 0.168, 0.15, 0.146, 0.14, 0.137, 0.135],
)
trunk(
    "birch_trunk_middle",
    "the next 2.5 m of a downy birch, wandering a little, 27 to 20 cm across",
    [(0, 0, 0), (0, 0, 0.35), (0.014, -0.005, 0.75), (0.026, 0.003, 1.15), (0.015, 0.017, 1.55)]
    + [(0.035, 0.024, 1.95), (0.04, 0.02, 2.15), (0.04, 0.02, 2.5)],
    [0.135, 0.13, 0.124, 0.119, 0.113, 0.107, 0.104, 0.10],
    sockets=[(0.65, 0, 45), (1.35, 135, 52), (2.05, 247.5, 58)],
)
trunk(
    "birch_trunk_top",
    "the crown's trunk of a downy birch, 2.5 m, 20 to 11 cm across, its leading shoot broken",
    [(0, 0, 0), (0, 0, 0.35), (0.012, 0.004, 0.75), (0.022, 0.012, 1.15), (0.032, 0.020, 1.55)]
    + [(0.048, 0.012, 1.95), (0.055, 0.01, 2.15), (0.055, 0.01, 2.5)],
    [0.10, 0.093, 0.086, 0.079, 0.072, 0.065, 0.061, 0.055],
    breaks=(3,),
    sockets=[(0.45, 67.5, 42), (1.0, 180, 48), (1.65, 292.5, 55), (2.15, 90, 60)],
    broken=True,
)
trunk(
    "birch_trunk_snag",
    "a dead downy birch's broken trunk, 1.4 m, 22 to 18 cm across, leaning",
    [(0, 0, 0), (0.018, 0.005, 0.35), (0.039, 0.009, 0.70), (0.062, 0.012, 1.05), (0.084, 0.015, 1.4)],
    [0.11, 0.105, 0.1, 0.095, 0.09],
    broken=True,
)

living_branch("branch_long_150", 1.5, 0.07, 4, 5)  # 7 cm to 1.5 cm needs four wraps: 4.67 ** (1 / 4) = 1.47
living_branch("branch_short_80", 0.8, 0.035, 2, 3)

p = kit.Part(
    "branch_dead_100",
    group="branches",
    about="a dead birch branch fallen to the ground, 1 m long, 4 to 2 cm across, with a broken side stub",
)
points = [(-0.5, 0, 0.02), (-0.25, 0.012, 0.02), (0, 0.006, 0.02), (0.25, 0, 0.02), (0.5, -0.005, 0.02)]
p.sleeve(
    points,
    [0.02, 0.0175, 0.015, 0.0125, 0.01],
    sides=8,
    caps=("broken", "broken"),
    role="bark",
    cap_role="wood",
    seed=77,
    breaks=(2,),
)
p.sleeve(
    [(0, 0.006, 0.02), (0.018, 0.08, 0.02), (0.025, 0.152, 0.02)],
    [0.01, 0.01, 0.01],
    sides=6,
    caps=(None, "broken"),
    role="bark",
    cap_role="wood",
    seed=79,
)
p.joint("base", towards=DOWN)
p.joint("end_a", at=points[0], towards=(-1, 0, 0))
p.joint("end_b", at=points[-1], towards=(1, 0, 0))
p.done()

tuft("tuft_short", 0.30, [0.25] * 3, 60, (8, 15), 1)
tuft("tuft_tall", 0.40, [0.50, 0.40, 0.50, 0.40], 45, (10, 20), 1)
tuft("tuft_seeding", 0.35, [0.70] * 3, 60, (5, 10), 3)
cluster("leaf_cluster_small", 0.20, 4)
cluster("leaf_cluster_large", 0.30, 6)

p = kit.Part(
    "leaf_spray_hanging",
    group="leaves",
    about="a downy birch's hanging spray of fine twigs and leaves, 0.6 m long and 0.3 m wide, three cards",
)
p.joint("stem", towards=UP)
for i in range(3):
    p.card(
        0.30,
        0.60,
        role="leaf",
        yaw=math.radians((i - 1) * 15),
        lean=math.radians(172 - i * 3),
        rows=3,
        uv_at=(i * 0.32, 0),
    )
p.done()

kit.finish(sys.argv[-1])
