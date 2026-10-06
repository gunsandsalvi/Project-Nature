"""A red deer (T2.3a.4, PRE-27, PRE-46, A6.1, A6.3, A6.4): the hoofed body pattern's skeleton of 28 bones, a body of
parts skinned to it with build, age and sex as shape keys, and antlers by age as choices on the head, built in
Blender with tools/art/kit.py.

    blender -b --factory-startup --python art/models/deer.py -- art/models/deer.blend

GPT's first pass (art/requests/deer-blend-01.txt) set the rings' sizes and weights, the shape keys' changes by
region, the ears, the tail, the antlers' beams and tines, and found the bend test's turns by where the bones go. Like
its person, it passed the check only by cutting every face's texture into a patch of its own (no one could draw on
that), with the kit's own functions swapped at run time to do it. The art lane rebuilt it as the person is built:
each chain (body, legs) one table of rings whose shared joint rings are made once and turned by the whole chain's
frames; each part's texture a rectangle of its atlas whose rows are its rings; more rings round the knees and hocks;
the hooves' last stretch upright, so the cloven sole lies flat in every shape key.

The stag stands 1.22 m at the withers, facing -y, its left toward +x, its origin on the ground under its middle.
A calf's or a yearling's height and a hind's smaller frame come from the skeleton's bone lengths, which the game sets
(A6.3); the keys change the flesh. Antlers are worn one set at a time, by age; a hind wears none.
"""

import math
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "art"))
import bpy  # noqa: E402
import kit  # noqa: E402

KEYS = ("build_thin", "build_stout", "sex_female", "age_young", "age_old")
FORWARD = (0.0, 0.0, -1.0)  # for a chain running forward: a ring's second radius is its height
DOWN = (0.0, -1.0, 0.0)  # for a leg running down: a ring's second radius is front to back
BELLY = 3 * math.pi / 2  # the seam under the belly and the throat
BEHIND = math.pi / 2  # the seam behind each leg


def ring(at, r, w, region):
    """One ring of a chain: its centre, its radii (or one distance for each side), its weights and its region."""
    return {"at": tuple(at), "r": (r, r) if isinstance(r, (int, float)) else tuple(r), "w": dict(w), "region": region}


def both(a, b, share=0.5):
    """Weights shared by two bones, `share` to the first."""
    return {a: share, b: 1 - share}


def variant(rg, key):
    """A ring's centre and radii with a shape key at 1."""
    x, y, z = rg["at"]
    region = rg["region"]
    f = 1.0
    if key == "build_thin":
        f = 0.88
    elif key == "build_stout":
        f = 1.15 if region == "barrel" else 1.10 if region in ("haunch", "shoulder") else 1.03
    elif key == "sex_female":  # a hind: a slender neck with no mane, a finer head, a lighter barrel
        f = {"neck": 0.75, "base": 0.8, "barrel": 0.95, "head": 0.92, "muzzle": 0.92}.get(region, 1.0)
    elif key == "age_young":  # a calf or yearling: slighter, a shorter muzzle
        f = 0.95 if region in ("hoof", "ear") else 0.85
        if region == "muzzle":
            y = -0.82 + (y + 0.82) * 0.85
    elif key == "age_old":
        f = 0.92 if region in ("neck", "leg", "base") else 1.0
    r = rg["r"]
    if key == "age_old" and region == "barrel":  # the belly lower
        z -= 0.012
        r = (r[0], r[1] + 0.012)
    return (x, y, z), tuple(v * f for v in r)


def build(part, chain, a, b, sides, seam, front, caps=(None, None), role="hair"):
    """The rings a to b of a chain as one sleeve of a part, turned by the whole chain's frames at rest and in each
    shape key, so a ring it shares with the next part is that part's ring exactly; its texture a rectangle of its
    atlas, each ring a row, its wrap restarting where it narrows fast."""
    rows = chain[a : b + 1]
    keys = {}
    for key in KEYS:
        moved = [variant(rg, key) for rg in chain]
        keys[key] = {
            "points": [m[0] for m in moved[a : b + 1]],
            "radii": [m[1] for m in moved[a : b + 1]],
            "frames": kit.frames([m[0] for m in moved], front)[a : b + 1],
        }
    part.sleeve(
        [rg["at"] for rg in rows],
        [rg["r"] for rg in rows],
        sides=sides,
        seam=seam,
        front=front,
        caps=caps,
        role=role,
        weights=[rg["w"] for rg in rows],
        keys=keys,
        along_frames=kit.frames([rg["at"] for rg in chain], front)[a : b + 1],
        breaks="auto",
    )


BONES = [
    ("root", (0, 0, 0), (0, 0, 0.1), None),
    ("hips", (0, 0.55, 1.10), (0, 0.20, 1.12), "root"),
    ("spine", (0, 0.20, 1.12), (0, -0.20, 1.14), "hips"),
    ("chest", (0, -0.20, 1.14), (0, -0.45, 1.18), "spine"),
    ("neck1", (0, -0.45, 1.18), (0, -0.60, 1.42), "chest"),
    ("neck2", (0, -0.60, 1.42), (0, -0.70, 1.62), "neck1"),
    ("head", (0, -0.70, 1.62), (0, -1.10, 1.50), "neck2"),
    ("tail", (0, 0.72, 1.10), (0, 0.82, 0.98), "hips"),
]
LIMBS = [
    ("ear", (0.06, -0.74, 1.68), (0.17, -0.76, 1.74), "head"),
    ("scapula", (0.13, -0.38, 1.18), (0.15, -0.46, 0.92), "chest"),
    ("upper_front", (0.15, -0.46, 0.92), (0.14, -0.36, 0.70), "scapula"),
    ("fore", (0.14, -0.36, 0.70), (0.13, -0.38, 0.40), "upper_front"),
    ("cannon_front", (0.13, -0.38, 0.40), (0.13, -0.39, 0.11), "fore"),
    ("hoof_front", (0.13, -0.39, 0.11), (0.13, -0.43, 0.0), "cannon_front"),
    ("thigh_hind", (0.13, 0.52, 1.00), (0.14, 0.38, 0.70), "hips"),
    ("shin_hind", (0.14, 0.38, 0.70), (0.13, 0.58, 0.44), "thigh_hind"),
    ("cannon_hind", (0.13, 0.58, 0.44), (0.13, 0.55, 0.11), "shin_hind"),
    ("hoof_hind", (0.13, 0.55, 0.11), (0.13, 0.51, 0.0), "cannon_hind"),
]
for side, sign in (("L", 1), ("R", -1)):
    for name, head, tail, parent in LIMBS:
        parent = parent if parent in ("head", "chest", "hips") else f"{parent}.{side}"
        BONES.append((f"{name}.{side}", (sign * head[0], head[1], head[2]), (sign * tail[0], tail[1], tail[2]), parent))

# The body is three solids that overlap rather than share rings, since the neck rises from the chest, and the head
# from the neck, at angles no ring can turn without tearing its texture: the torso from the rump to the chest, closed
# at both ends; the neck from deep inside the chest to inside the head; the head from inside the neck to the nose.
# Each overlaps the next far enough that no gap opens as the neck and head turn.
TORSO = [
    ring((0, 0.78, 0.99), (0.12, 0.17), {"hips": 1}, "haunch"),
    ring((0, 0.55, 0.99), (0.21, 0.25), {"hips": 1}, "haunch"),
    ring((0, 0.25, 0.99), (0.22, 0.26), both("hips", "spine", 0.75), "barrel"),
    ring((0, 0.12, 0.98), (0.23, 0.27), both("hips", "spine"), "barrel"),
    ring((0, -0.05, 0.97), (0.24, 0.28), {"spine": 1}, "barrel"),
    ring((0, -0.20, 0.96), (0.23, 0.29), both("spine", "chest"), "shoulder"),
    ring((0, -0.30, 0.95), (0.22, 0.29), both("chest", "spine", 0.75), "shoulder"),
    ring((0, -0.42, 0.96), (0.19, 0.26), {"chest": 1}, "shoulder"),
    ring((0, -0.50, 0.95), (0.14, 0.21), {"chest": 1}, "shoulder"),
    ring((0, -0.54, 0.95), (0.07, 0.12), {"chest": 1}, "shoulder"),
]
# The neck's weights spread its turn evenly from the chest to the head, so a lowered neck folds along its length
# rather than at its joints, where the throat would crush.
NECK = [
    ring((0, -0.40, 1.02), (0.11, 0.15), {"chest": 1}, "base"),
    ring((0, -0.50, 1.14), (0.12, 0.16), both("chest", "neck1"), "base"),
    ring((0, -0.57, 1.29), (0.105, 0.145), both("neck1", "neck2", 0.88), "neck"),
    ring((0, -0.60, 1.42), (0.09, 0.13), both("neck1", "neck2", 0.36), "neck"),
    ring((0, -0.65, 1.51), (0.08, 0.105), {"neck2": 1}, "neck"),
    ring((0, -0.68, 1.58), (0.07, 0.085), both("neck2", "head", 0.55), "neck"),
    ring((0, -0.70, 1.63), (0.045, 0.055), both("head", "neck2", 0.75), "neck"),
]
HEAD = [
    ring((0, -0.66, 1.60), (0.06, 0.07), both("head", "neck2"), "head"),
    ring((0, -0.71, 1.60), (0.07, 0.085), both("head", "neck2", 0.75), "head"),
    ring((0, -0.78, 1.59), (0.07, 0.08), {"head": 1}, "head"),
    ring((0, -0.88, 1.565), (0.06, 0.07), {"head": 1}, "head"),
    ring((0, -0.98, 1.535), (0.045, 0.06), {"head": 1}, "muzzle"),
    ring((0, -1.075, 1.505), (0.035, 0.045), {"head": 1}, "muzzle"),
    ring((0, -1.115, 1.493), (0.028, 0.035), {"head": 1}, "muzzle"),
]
UP_FRONT = (0.0, -1.0, 0.0)  # for the neck, running up: a ring's second radius is its depth, throat to mane


def cloven(across, length, sides=8, seam=BEHIND):
    """A hoof's sole: an ellipse `across` by `length` metres with a notch at the toe between the two claws, one
    distance for each side of a ring that starts at the seam (behind) and turns toward the front."""
    out = []
    for k in range(sides):
        a = seam + 2 * math.pi * k / sides  # 0 across the side axis, pi/2 behind, 3pi/2 the toe
        r = 1 / math.sqrt((math.cos(a) / across) ** 2 + (math.sin(a) / length) ** 2)
        out.append(r * (0.72 if abs(math.sin(a) + 1) < 1e-6 else 1.0))
    return out


def leg(side, sign, kind):
    """A leg from inside the body down to the sole: its fetlock's ring shared with the hoof, the weights changing
    over rings either side of the knee (or hock)."""

    def b(n):
        return f"{n}_{kind}.{side}" if n in ("cannon", "hoof") else f"{n}.{side}"

    def at(x, y, z):
        return (sign * x, y, z)

    if kind == "front":
        y = -0.39
        rows = [
            ring(at(0.13, -0.38, 1.09), 0.07, {b("scapula"): 1}, "leg"),
            ring(at(0.15, -0.46, 0.92), 0.06, both(b("scapula"), b("upper_front")), "leg"),
            ring(at(0.146, -0.41, 0.81), 0.055, both(b("upper_front"), b("fore"), 0.75), "leg"),
            ring(at(0.14, -0.36, 0.70), 0.055, both(b("upper_front"), b("fore")), "leg"),
            ring(at(0.137, -0.365, 0.58), 0.045, both(b("fore"), b("upper_front"), 0.8), "leg"),
            ring(at(0.134, -0.372, 0.47), 0.04, both(b("fore"), b("cannon"), 0.68), "leg"),
            ring(at(0.13, -0.38, 0.40), 0.036, both(b("fore"), b("cannon")), "leg"),
            ring(at(0.13, -0.382, 0.33), (0.03, 0.033), both(b("cannon"), b("fore"), 0.68), "leg"),
            ring(at(0.13, -0.385, 0.24), (0.022, 0.028), both(b("cannon"), b("hoof"), 0.8), "leg"),
        ]
    else:
        y = 0.55
        rows = [
            ring(at(0.13, 0.52, 1.05), 0.08, {b("thigh_hind"): 1}, "leg"),
            ring(at(0.135, 0.46, 0.86), 0.075, both(b("thigh_hind"), b("shin_hind"), 0.75), "leg"),
            ring(at(0.14, 0.38, 0.70), 0.06, both(b("thigh_hind"), b("shin_hind")), "leg"),
            ring(at(0.135, 0.48, 0.57), 0.045, both(b("shin_hind"), b("cannon"), 0.8), "leg"),
            ring(at(0.132, 0.53, 0.50), 0.04, both(b("shin_hind"), b("cannon"), 0.65), "leg"),
            ring(at(0.13, 0.58, 0.44), 0.035, both(b("shin_hind"), b("cannon")), "leg"),
            ring(at(0.13, 0.575, 0.37), (0.028, 0.032), both(b("cannon"), b("shin_hind"), 0.68), "leg"),
            ring(at(0.13, 0.565, 0.26), (0.022, 0.03), both(b("cannon"), b("hoof"), 0.8), "leg"),
        ]
    rows += [
        ring(at(0.13, y, 0.11), 0.026, both(b("cannon"), b("hoof")), "leg"),  # the fetlock, shared
        ring(at(0.13, y - 0.008, 0.078), 0.022, both(b("hoof"), b("cannon"), 0.75), "hoof"),
        ring(at(0.13, y - 0.02, 0.035), (0.03, 0.04), {b("hoof"): 1}, "hoof"),
        ring(at(0.13, y - 0.02, 0.0), cloven(0.035, 0.05), {b("hoof"): 1}, "hoof"),
    ]
    return rows


kit.start()
arm = kit.armature("deer", BONES, group="deer", about="a red deer stag, 1.22 m at the withers, standing square")
coat = kit.Atlas("deer_coat", 4.0, 4.0)


def part(name, about):
    return kit.Part(name, group="deer", about=about, armature=arm, atlas=coat)


p = part("body_torso", "the body from the rump to the base of the neck, and the short tail")
build(p, TORSO, 0, len(TORSO) - 1, 12, BELLY, FORWARD, caps=("flat", "flat"))
tail = [
    ring((0, 0.73, 1.10), (0.032, 0.04), both("hips", "tail"), "tail"),
    ring((0, 0.80, 1.03), (0.029, 0.035), {"tail": 1}, "tail"),
    ring((0, 0.83, 0.97), (0.018, 0.022), {"tail": 1}, "tail"),
]
build(p, tail, 0, 2, 8, BEHIND, DOWN, caps=(None, "flat"))
p.chart_overstretched()
p.done()
p = part("body_neck", "the neck, thick and maned in a stag, from deep in the chest into the head")
build(p, NECK, 0, len(NECK) - 1, 12, BELLY, UP_FRONT, caps=(None, "flat"))
p.chart_overstretched()
p.done()
p = part("body_head", "the head from inside the neck to the nose, with its ears")
build(p, HEAD, 0, len(HEAD) - 1, 12, BELLY, FORWARD, caps=(None, "flat"))
for side, sign in (("L", 1), ("R", -1)):
    ear = [
        ring((sign * 0.055, -0.745, 1.67), (0.035, 0.015), both("head", f"ear.{side}"), "ear"),
        ring((sign * 0.115, -0.75, 1.715), (0.045, 0.013), {f"ear.{side}": 1}, "ear"),
        ring((sign * 0.195, -0.755, 1.765), (0.032, 0.012), {f"ear.{side}": 1}, "ear"),
        ring((sign * 0.216, -0.758, 1.775), (0.016, 0.008), {f"ear.{side}": 1}, "ear"),
    ]
    build(p, ear, 0, 3, 8, BEHIND, DOWN, caps=(None, "flat"))
p.paint("skin", lambda c: c.y < -1.075)  # the bare nose
p.chart_overstretched()
p.done()
for side, sign in (("L", 1), ("R", -1)):
    for kind in ("front", "hind"):
        rows = leg(side, sign, kind)
        fetlock = len(rows) - 4
        p = part(f"leg_{kind}.{side}", f"the {kind} leg, from inside the body to the fetlock")
        build(p, rows, 0, fetlock, 8, BEHIND, DOWN)
        p.chart_overstretched()
        p.done()
        p = part(f"hoof_{kind}.{side}", f"the {kind} hoof, from the fetlock to its cloven sole")
        build(p, rows, fetlock, len(rows) - 1, 8, BEHIND, DOWN, caps=(None, "flat"), role="hoof")
        p.chart_overstretched()
        p.done()


def antlers(name, about, beams, tines):
    """A pair of antlers on the head, worn as one choice by age: each beam and tine a wrapped sleeve of the tiled
    antler texture ending in a short point, skinned whole to the head."""
    p = kit.Part(name, group="deer", about=about, armature=arm)
    for points, radii in beams + tines:
        p.sleeve(
            points,
            radii,
            sides=8,
            role="antler",
            cap_role="antler",
            caps=(None, "chopped"),
            weights=[{"head": 1}] * len(points),
            breaks="auto",
        )
    ob = p.done()
    ob["kit_layer"] = "choice"


def pair(points, radii, scale=1.0, base=(0.05, -0.76, 1.70)):
    """A left and a right sleeve from one side's points, scaled about the pedicle by `scale`."""
    out = []
    for sign in (1, -1):
        pts = [tuple(base[i] + (q[i] - base[i]) * scale for i in range(3)) for q in points]
        out.append(([(sign * x, y, z) for x, y, z in pts], radii))
    return out


BEAM = [(0.05, -0.76, 1.70), (0.09, -0.71, 1.84), (0.16, -0.63, 2.00), (0.24, -0.53, 2.16), (0.32, -0.50, 2.31)]
BEAM += [(0.375, -0.55, 2.43), (0.36, -0.64, 2.50)]
BROW = [(0.08, -0.73, 1.81), (0.10, -0.88, 1.86), (0.12, -1.02, 1.99)]
BEZ = [(0.13, -0.65, 1.95), (0.16, -0.81, 2.02), (0.18, -0.92, 2.15)]
TREZ = [(0.23, -0.54, 2.14), (0.28, -0.70, 2.23), (0.31, -0.81, 2.36)]
CROWN = [[(0.33, -0.51, 2.34), (0.28, -0.65, 2.42), (0.26, -0.72, 2.49)], [(0.33, -0.51, 2.34), (0.375, -0.51, 2.43)]]
CROWN[1] += [(0.37, -0.56, 2.50)]
antlers(
    "antlers_spike",
    "a yearling's spikes, about 22 cm each",
    pair([(0.05, -0.76, 1.70), (0.075, -0.73, 1.81), (0.105, -0.72, 1.91)], [0.0125, 0.010, 0.0075]),
    [],
)
young_radii = [0.0175, 0.016, 0.014, 0.012, 0.010, 0.009, 0.0075]
antlers(
    "antlers_young",
    "a young stag's antlers, beams about 55 cm, brow tines and a fork at the top: 6 points",
    pair(BEAM, young_radii, 0.62),
    pair(BROW, [0.010, 0.009, 0.0075], 0.62) + pair(CROWN[0], [0.010, 0.009, 0.0075], 0.62),
)
antlers(
    "antlers_mature",
    "a royal stag's antlers, beams about 90 cm, brow, bez and trez tines and a crown of three: 12 points",
    pair(BEAM, [0.025, 0.023, 0.020, 0.017, 0.014, 0.011, 0.008]),
    sum((pair(t, [0.012, 0.009, 0.0075]) for t in (BROW, BEZ, TREZ, *CROWN)), []),
)


def turn(bone, angle, axis, toward):
    """The bend test's turn of a bone about its x axis, its sign chosen so its tail moves `toward` along the world
    axis given (0 x, 1 y, 2 z), as GPT's first pass found them, never from a guess at the bone's roll."""
    pb = arm.pose.bones[bone]
    pb.rotation_mode = "XYZ"
    before = pb.tail.copy()
    moves = []
    for s in (-1, 1):
        pb.rotation_euler = (math.radians(s * angle), 0, 0)
        bpy.context.view_layer.update()
        moves.append((toward * (pb.tail[axis] - before[axis]), s))
    pb.rotation_euler = (0, 0, 0)
    bpy.context.view_layer.update()
    return (max(moves)[1] * angle, 0, 0)


# The bend test, as if grazing: the neck and head down, the left foreleg's knee folded back mid-stride, the right
# hind leg's hock bent.
turns = {
    "neck1": turn("neck1", 30, 2, -1),
    "neck2": turn("neck2", 30, 2, -1),
    "head": turn("head", 20, 2, -1),
    "cannon_front.L": turn("cannon_front.L", 60, 1, 1),
    "cannon_hind.R": turn("cannon_hind.R", 30, 1, -1),
}
kit.pose(arm, "bend_test", turns)
bpy.context.view_layer.update()
kit.finish(sys.argv[-1])
