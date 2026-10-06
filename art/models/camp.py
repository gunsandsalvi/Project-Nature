"""The kit's camp parts (T2.3a.4, PRE-46, PRE-42, A6.1, A6.4): the wood, covers and things of a hunter-gatherer
camp, built in Blender with tools/art/kit.py.

    blender -b --factory-startup --python art/models/camp.py -- art/models/camp.blend

GPT's first pass (art/requests/camp-blend-01.txt) made every part here; the art lane's fixes: a hand axe knapped
as a biface with flake scars on both faces (GPT's was a pyramid of 20 triangles), a whole hide shaped as a deer's
skin comes off (GPT's read as a star), the forked upright closed in its crotch, and the hide over a bar welded into
one piece. No wood is sawn: every end is chopped by a stone axe or broken.
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "art"))
import kit  # noqa: E402

UP = (0, 0, 1)
DOWN = (0, 0, -1)


def broken_reach(radius, sides, seed):
    """How far a broken end's longest splinter reaches past its ring (kit's broken end, by the same seed)."""
    rng = random.Random(seed)
    return max(rng.uniform(0.15, 0.9) * radius for _ in range(sides))


def pole(length, bent=False):
    """A young birch or pine sapling as a pole, 9.5 cm across at its chopped foot to 6.5 cm at its broken top."""
    name = f"pole_{'bent' if bent else 'straight'}_{round(length * 100)}"
    p = kit.Part(
        name,
        group="wood",
        about=f"a {length:g} m sapling pole, {'slightly crooked, ' if bent else ''}9.5 to 6.5 cm across, "
        "chopped at its foot and broken at its top",
    )
    seed = round(length * 100) + (17 if bent else 0)
    n = math.ceil(length / 0.45)
    bottom, top = 0.0475 * 1.2, length - broken_reach(0.0325, 8, 2 * seed + 1)

    def centre(z):
        # the foot and the top stay upright; a gentle, uneven bow between them
        t = max(0.0, min(1.0, (z - bottom) / (top - bottom)))
        ease = t * t * (3 - 2 * t)
        x = (0.065 * ease + 0.014 * math.sin(2 * math.pi * t) ** 2) if bent else 0.0
        y = (0.015 * math.sin(math.pi * t) ** 2) if bent else 0.0
        return (x, y, z)

    pts = [centre(bottom + (top - bottom) * i / n) for i in range(n + 1)]
    if bent:
        pts[1] = (0.0, 0.0, pts[1][2])
        pts[-2] = (0.065, 0.0, pts[-2][2])
    radii = [0.0475 + (0.0325 - 0.0475) * i / n for i in range(n + 1)]
    p.sleeve(pts, radii, sides=8, caps=("chopped", "broken"), cap_role="wood", seed=seed)
    p.joint("base", towards=DOWN)
    z = 0.85 * length  # where tent poles are bound together
    for a, b in zip(pts, pts[1:], strict=False):
        if a[2] <= z <= b[2]:
            f = (z - a[2]) / (b[2] - a[2])
            p.joint("tie", at=tuple(a[k] + f * (b[k] - a[k]) for k in range(3)), towards=(1, 0, 0))
            break
    p.joint("top", at=(pts[-1][0], pts[-1][1], length))
    p.done()


def log(name, length, r0, r1, sides):
    """Dead wood lying along x on the ground, broken at both ends."""
    seed = round(length * 100) + 31
    p = kit.Part(
        name,
        group="wood",
        about=f"dead wood {length:g} m long and {200 * r0:.0f} to {200 * r1:.0f} cm across, lying along x, "
        "broken at both ends",
    )
    left = -length / 2 + broken_reach(r0, sides, 2 * seed)
    right = length / 2 - broken_reach(r1, sides, 2 * seed + 1)
    n = max(3, math.ceil(length / 0.35))
    radii = [
        r0 + (r1 - r0) * i / n + 0.002 * math.sin(math.pi * i / n) * math.sin(3 * math.pi * i / n) for i in range(n + 1)
    ]
    pts = [(left + (right - left) * i / n, 0.008 * math.sin(math.pi * i / n) ** 2, radii[i]) for i in range(n + 1)]
    p.sleeve(pts, radii, sides=sides, caps=("broken", "broken"), cap_role="wood", seed=seed)
    p.rest()
    p.joint("base", towards=DOWN)
    p.joint("end_a", at=(-length / 2, 0, r0), towards=(-1, 0, 0))
    p.joint("end_b", at=(length / 2, 0, r1), towards=(1, 0, 0))
    p.done()


def cover(name, about, outline, joints, role="hide", bend=None, spacing=0.12):
    """A flat cut shape, its texture its own shape in metres, drawn from both sides by the game; bent if `bend`."""
    p = kit.Part(name, group="covers", about=about)
    p.panel(outline, role=role, bend=bend, spacing=spacing)
    for label, at, towards in joints:
        p.joint(label, at=bend(*at) if bend else (at[0], 0, at[1]), towards=towards)
    p.done()


def ragged(outline, seed, nick=0.009):
    """An outline with a small nick between each pair of its points, as a skin's edge is never smooth."""
    rng = random.Random(seed)
    out = []
    for i, a in enumerate(outline):
        b = outline[(i + 1) % len(outline)]
        out.append(a)
        out.append(((a[0] + b[0]) / 2 + rng.uniform(-nick, nick), (a[1] + b[1]) / 2 + rng.uniform(-nick, nick)))
    return out


def bar_bend(x, s):
    """A hide over a bar of 3 cm radius along x: s is the distance along the hide from the fold; each side falls
    from the bar with an outward sag and a gentle wave."""
    r = 0.03
    a = abs(s)
    sign = 1 if s >= 0 else -1
    arc = r * math.pi / 2
    if a <= arc:
        return (x, sign * r * math.sin(a / r), r * (math.cos(a / r) - 1))
    d = a - arc
    y = sign * (
        r
        + 0.06 * (1 - math.exp(-d / 0.3)) ** 2
        + 0.008 * math.sin(2 * math.pi * x / 1.2) * (1 - math.exp(-d / 0.2)) ** 2
    )
    return (x, y, -r - d)


def door_bend(x, z):
    """A curtain hung from its top edge, falling in three soft folds that grow toward its hem."""
    return (x, 0.035 * (-z / 1.5) * math.sin(5 * math.pi * (x + 0.5)), z)


def dome_bend(x, z):
    """A panel laid on a dome hut's frame, a sphere of 1.6 m radius."""
    r = 1.6
    a, b = x / r, z / r
    return (r * math.cos(b) * math.sin(a), r * (1 - math.cos(b) * math.cos(a)), r * math.sin(b))


def hand_axe():
    """A hand axe knapped from flint, standing tip up: a teardrop outline whose edge wavers a little, each face a low
    ridge of flat flake scars (the triangles between points scattered over it at its own thickness), crisp."""
    rng = random.Random(701)
    length, half_width, half_thick = 0.14, 0.0425, 0.0175

    def width(z):  # the outline's half width at height z: a rounded butt, then tapering to the tip
        if z <= 0.045:
            return half_width * math.sqrt(max(0.0, 1 - ((z - 0.045) / 0.045) ** 2))
        return half_width * max(0.0, 1 - ((z - 0.045) / (length - 0.045)) ** 1.4)

    def depth(x, z):  # the face's height above the edge's plane: thickest toward the butt
        w = width(z)
        across = 1 - (x / w) ** 2 if w > 1e-6 else 0.0
        along = math.sin(math.pi * min(1.0, z / length) ** 0.75)
        return half_thick * max(0.0, across) ** 0.6 * along

    outline = []
    steps = 22
    for k in range(steps):  # counter-clockwise in x-z, from the butt's middle round to the tip and back
        a = 2 * math.pi * k / steps
        z = length / 2 * (1 - math.cos(a))
        side = 1 if a < math.pi else -1
        if k in (0, steps // 2):
            x = 0.0
        else:
            x = side * width(z)
        outline.append((x, z))
    points = [(x, rng.uniform(-0.0012, 0.0012) if x else 0.0, z) for x, z in outline]  # the edge wavers a little
    for face_sign in (-1, 1):  # the front face looks to -y, the back to +y
        scars = []
        while len(scars) < 10:
            z = rng.uniform(0.01, length - 0.018)
            x = rng.uniform(-1, 1) * width(z) * 0.8
            if all(math.dist((x, z), q) > 0.015 for q in scars):
                scars.append((x, z))
        # each scar's point a little proud of the face or sunk in it, so the facets between them differ
        points += [(x, face_sign * depth(x, z) * rng.uniform(0.85, 1.1), z) for x, z in scars]
    p = kit.Part("hand_axe", group="things", about="a hand axe knapped from flint, 14 x 8.5 x 3.5 cm, tip up")
    p.smooth = 20
    p.hull(points, smooth=False)  # a biface is convex: the hull's facets are its flake scars
    p.project("stone")
    p.joint("grip", towards=DOWN)
    p.joint("tip", at=(0, 0, length))
    p.done()


kit.start()

for length in (1.5, 2, 2.5, 3):
    pole(length)
for length in (2, 3):
    pole(length, True)
log("log_seat_160", 1.6, 0.13, 0.105, 14)
log("log_fire_50", 0.5, 0.035, 0.032, 8)
log("log_fire_80", 0.8, 0.045, 0.04, 8)

# A whole red deer skin as it comes off the animal, hair side to -y: a body longer than wide, the neck at the top,
# the legs' skin as long strips at its corners (the forelegs reaching out and up, the hind legs out and down),
# the flanks a little hollow between them, a short tail.
deer_skin = [
    (0.11, 0.0),
    (0.13, -0.2),
    (0.27, -0.29),
    (0.43, -0.22),
    (0.62, -0.25),
    (0.64, -0.33),
    (0.46, -0.38),
    (0.36, -0.46),
    (0.38, -0.7),
    (0.35, -0.94),
    (0.38, -1.1),
    (0.5, -1.2),
    (0.62, -1.37),
    (0.57, -1.45),
    (0.44, -1.36),
    (0.29, -1.3),
    (0.08, -1.37),
    (0.05, -1.5),
]
deer_skin += [(-x, y) for x, y in reversed(deer_skin)]
deer_skin = [(0.0, 0.0)] + [q for q in deer_skin if q != (0.0, 0.0)]
cover(
    "hide_whole",
    "the whole skin of a red deer, 1.5 m long and 1.3 m across its legs, neck at the top, flat",
    ragged(deer_skin, 12000),
    [
        ("neck", (0, 0), UP),
        ("leg_fl", (-0.64, -0.33), (-1, 0, 0.3)),
        ("leg_fr", (0.64, -0.33), (1, 0, 0.3)),
        ("leg_hl", (-0.6, -1.41), (-1, 0, -1)),
        ("leg_hr", (0.6, -1.41), (1, 0, -1)),
        ("tail", (0, -1.5), DOWN),
    ],
)
rect = [(0, 0), (0.36, 0.009), (0.66, 0), (0.7, -0.04), (0.692, -0.3), (0.705, -0.6), (0.69, -0.86), (0.65, -0.9)]
rect += [(0.25, -0.889), (-0.22, -0.908), (-0.65, -0.9), (-0.7, -0.86), (-0.689, -0.55), (-0.703, -0.25)]
rect += [(-0.7, -0.04), (-0.66, 0), (-0.32, -0.008)]
cover(
    "hide_panel_rect",
    "a hide trimmed for sewing into a cover, 1.4 x 0.9 m, its stone-cut edges wavering a little, flat",
    rect,
    [
        ("top", (0, 0), UP),
        ("corner_tl", (-0.68, -0.02), (-1, 0, 1)),
        ("corner_tr", (0.68, -0.02), (1, 0, 1)),
        ("corner_bl", (-0.68, -0.88), (-1, 0, -1)),
        ("corner_br", (0.67, -0.88), (1, 0, -1)),
    ],
)
fan = [(0, 0), (0.15, 0), (0.65, -2.25), (0.43, -2.282), (0, -2.3), (-0.43, -2.282), (-0.65, -2.25), (-0.15, 0)]
cover(
    "hide_panel_fan",
    "a panel cut for a conical tent's cover, 2.3 m tall, 0.3 m wide at the top and 1.3 m at its bowed foot, flat",
    fan,
    [("top", (0, 0), UP), ("corner_bl", (-0.65, -2.25), (-1, 0, -1)), ("corner_br", (0.65, -2.25), (1, 0, -1))],
)
p = kit.Part(
    "hide_draped_bar",
    group="covers",
    about="a hide 1.2 x 1.6 m folded over a bar of 3 cm radius, hanging 0.9 m one side and 0.7 m the other",
)
arc = 0.03 * math.pi / 2
for lower, upper, spacing in [(-0.7, -arc, 0.12), (-arc, arc, 0.015), (arc, 0.9, 0.12)]:  # fine only at the fold
    p.panel([(-0.6, lower), (0.6, lower), (0.6, upper), (-0.6, upper)], bend=bar_bend, spacing=spacing)
p.weld()
p.joint("bar")
p.done()
cover(
    "hide_draped_door",
    "a hide 1.0 x 1.5 m hanging as a door curtain from its top edge, in three soft folds",
    [(-0.5, 0), (0, 0), (0.5, 0), (0.5, -1.5), (-0.5, -1.5)],
    [("top", (0, 0), UP), ("corner_tl", (-0.5, 0), (-1, 0, 0)), ("corner_tr", (0.5, 0), (1, 0, 0))],
    bend=door_bend,
)
cover(
    "hide_curved_dome",
    "a hide panel cut 1.2 x 1.0 m, laid over a dome hut's frame of 1.6 m radius",
    [(-0.6, 0), (0, 0), (0.6, 0), (0.6, -1), (-0.6, -1)],
    [("top", (0, 0), UP), ("corner_bl", (-0.6, -1), (-1, 0, -1)), ("corner_br", (0.6, -1), (1, 0, -1))],
    bend=dome_bend,
    spacing=0.09,
)
for name, width, height, curled in [("bark_sheet_curled", 0.45, 0.6, True), ("bark_sheet_flat", 0.5, 0.7, False)]:

    def bark_bend(x, z, curled=curled, height=height):
        r = 0.18 + 0.12 * (-z / height) if curled else 0.8
        return (r * math.sin(x / r), r * (1 - math.cos(x / r)), z)

    cover(
        name,
        f"a sheet of birch bark {width:g} x {height:g} m, "
        + ("freshly peeled and still curled round its trunk's shape" if curled else "nearly flat, laid on a hut"),
        [(-width / 2, 0), (0, 0), (width / 2, 0), (width / 2, -height), (-width / 2, -height)],
        [("top", (0, 0), UP), ("bottom", (0, -height), DOWN)],
        role="bark",
        bend=bark_bend,
        spacing=0.045,
    )

p = kit.Part("basket", group="things", about="a round basket of plant fibre, 0.34 m across and 0.30 m tall, open")
rim = [(0.12, 0), (0.122, 0.035), (0.145, 0.17), (0.164, 0.28), (0.17, 0.29), (0.167, 0.297), (0.16, 0.3)]
rim += [(0.153, 0.297), (0.15, 0.29), (0.149, 0.28), (0.13, 0.17), (0.107, 0.035), (0.105, 0.015)]
p.lathe(rim, sides=20, role="weave", caps=(True, True))
p.joint("base", towards=DOWN)
p.joint("rim", at=(0, 0, 0.3))
p.done()

for length in (1.5, 2):
    p = kit.Part(
        f"rack_bar_{round(length * 100)}",
        group="things",
        about=f"a peeled sapling bar of a drying rack, {length:g} m long and 5 cm across, chopped at both ends",
    )
    n = math.ceil(length / 0.5)
    p.sleeve(
        [(0.03 + (length - 0.06) * i / n, 0, 0) for i in range(n + 1)],
        [0.025] * (n + 1),
        sides=8,
        role="bark",
        cap_role="wood",
        caps=("chopped", "chopped"),
    )
    p.joint("end_a", towards=(-1, 0, 0))
    p.joint("end_b", at=(length, 0, 0), towards=(1, 0, 0))
    for i in range(1, 5):
        p.joint(f"hang_{i}", at=(length * i / 5, 0, -0.025), towards=DOWN)
    p.done()

p = kit.Part(
    "rack_upright_forked",
    group="things",
    about="a forked stick 1.6 m tall holding a drying rack's bar, 6 cm across at its chopped foot",
)
p.sleeve(
    [(0, 0, 0.036), (0, 0, 0.48), (0.005, 0, 0.93), (0, 0, 1.35)],
    [0.03, 0.028, 0.025, 0.0225],
    sides=8,
    cap_role="wood",
    caps=("chopped", "flat"),  # its top hidden in the crotch, closed so no hole shows between the prongs
)
for sign in (-1, 1):
    angle, reach = math.radians(20), 0.25
    p.sleeve(
        [(0, 0, 1.31), (sign * math.sin(angle) * reach, 0, 1.31 + math.cos(angle) * reach)],
        [0.0175, 0.0125],
        sides=8,
        cap_role="wood",
        caps=(None, "broken"),
        seed=42 + sign,
    )
p.joint("base", towards=DOWN)
p.joint("fork", at=(0, 0, 1.35))
p.done()

hand_axe()
kit.finish(sys.argv[-1])
