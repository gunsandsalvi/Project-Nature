"""The first person (T2.3a.4, PRE-27, PRE-46, A6.1, A6.3, A6.4): one skeleton of 22 bones, a body of parts skinned to
it with build, age and sex as shape keys, and three garments on the same skeleton, built in Blender with
tools/art/kit.py.

    blender -b --factory-startup --python art/models/people.py -- art/models/people.blend

GPT's first pass (art/requests/people-blend-01.txt) set the rings' sizes, the shape keys' changes by region and the
bend test's turns. It passed the check only by cutting every triangle's texture into a scrap of its own (no one
could draw on that), with soles flattened in every key, which tore the feet. The art lane rebuilt it:
- each chain (spine, arm, leg) is one table of rings; each part is a stretch of its chain, turned by the whole
  chain's frames, so the ring two parts share at a joint is one ring in both, at rest and in every shape key, and
  they never part; around each knee and elbow the weights change over five rings, so a bend spreads;
- each part's texture is one piece of its atlas, a rectangle whose rows are its rings (a sleeve's wrap, restarting
  where the part narrows fast), so a figure's texture is drawn row by row; where no sleeve can lay its texture within
  the line (the shoulders' ledge, the crown, the ankles and feet), faces take charts, pieces laid from the way they
  face.

The figure stands 1.65 m, facing -y, its left toward +x, its origin on the ground between its feet. How tall a child
or an elder stands, a child's larger head and an elder's stoop come from the skeleton's bone lengths, scales and
poses, which the game sets (A6.3); the age keys change only the flesh: a child's rounder, softer face and slighter
build, an elder's thinner limbs and lower belly.
"""

import math
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "art"))
import bpy  # noqa: E402
import kit  # noqa: E402

KEYS = ("build_thin", "build_stout", "sex_female", "age_child", "age_elder")
FRONT = (0.0, -1.0, 0.0)


def ring(at, r, w, region):
    """One ring of a chain: its centre, its radii (across the side axis, across the back axis), its weights by bone,
    and the region of the body it is, which the shape keys change."""
    return {"at": tuple(at), "r": (r, r) if isinstance(r, (int, float)) else tuple(r), "w": dict(w), "region": region}


def both(a, b, share=0.5):
    """Weights shared by two bones, `share` to the first."""
    return {a: share, b: 1 - share}


def lerp(p, q, t):
    return tuple(a + (b - a) * t for a, b in zip(p, q, strict=True))


def wlerp(w1, w2, t):
    """Weights part way from one ring's to the next's."""
    return {b: (1 - t) * w1.get(b, 0.0) + t * w2.get(b, 0.0) for b in set(w1) | set(w2)}


# How each shape key changes a ring: its girth by region, and for some regions where it sits.
GIRTH = {
    "build_thin": {"head": 0.95, "jaw": 0.95, "neck": 0.95, "hand": 0.95, "foot": 0.95, "*": 0.85},
    "build_stout": {
        "waist": 1.25,
        "belly": 1.25,
        "hip": 1.12,
        "chest": 1.12,
        "shoulder": 1.08,
        "neck": 1.1,
        "head": 1.06,
        "jaw": 1.06,
        "hand": 1.05,
        "foot": 1.05,
        "*": 1.12,
    },
    "sex_female": {
        "shoulder": 0.9,
        "waist": 0.92,
        "hip": 1.08,
        "neck": 0.9,
        "jaw": 0.9,
        "head": 0.97,
        "hand": 0.92,
        "foot": 0.92,
        "arm": 0.93,
        "leg": 0.93,
        "*": 1.0,
    },
    "age_child": {"head": 1.05, "jaw": 0.95, "neck": 0.92, "shoulder": 0.88, "hip": 0.88, "belly": 0.97, "*": 0.92},
    "age_elder": {"arm": 0.92, "leg": 0.92, "chest": 0.92, "shoulder": 0.95, "neck": 0.88, "*": 1.0},
}


def variant(rg, key, offset=0.0):
    """A ring's centre and radii with a shape key at 1; `offset` is a garment's distance out from the body."""
    x, y, z = rg["at"]
    rx, ry = rg["r"]
    f = GIRTH[key].get(rg["region"], GIRTH[key]["*"])
    if key == "sex_female" and rg["region"] == "chest":  # breasts: the chest fuller in front
        ry, y = ry + 0.012, y - 0.01
    if key == "age_elder" and rg["region"] == "belly":  # the belly softer and lower
        ry, z = ry * 1.06, z - 0.015
    return (x, y, z), (rx * f + offset, ry * f + offset)


def build(part, chain, a, b, sides, seam, caps=(None, None), role="skin", offset=0.0, breaks="auto"):
    """The rings a to b of a chain as one sleeve of a part, turned by the whole chain's frames at rest and in each
    shape key, so a ring it shares with the next part is that part's ring exactly. Its texture is a rectangle of
    its atlas, each ring a row, its wrap restarting where it narrows fast (kit's breaks="auto"). Returns the number
    of its first vertex."""
    rows = chain[a : b + 1]
    frames = kit.frames([rg["at"] for rg in chain], FRONT)[a : b + 1]
    keys = {}
    for key in KEYS:
        moved = [variant(rg, key, offset) for rg in chain]
        keys[key] = {
            "points": [m[0] for m in moved[a : b + 1]],
            "radii": [m[1] for m in moved[a : b + 1]],
            "frames": kit.frames([m[0] for m in moved], FRONT)[a : b + 1],
        }
    first = len(part.bm.verts)
    part.sleeve(
        [rg["at"] for rg in rows],
        [(rg["r"][0] + offset, rg["r"][1] + offset) for rg in rows],
        sides=sides,
        seam=seam,
        caps=caps,
        role=role,
        weights=[rg["w"] for rg in rows],
        keys=keys,
        along_frames=frames,
        breaks=breaks,
    )
    return first


# The skeleton: 22 bones, the .R ones mirroring the .L in x.
BONES = [
    ("root", (0, 0, 0), (0, 0, 0.1), None),
    ("hips", (0, 0, 0.92), (0, 0, 1.02), "root"),
    ("spine", (0, 0, 1.02), (0, 0, 1.20), "hips"),
    ("chest", (0, 0, 1.20), (0, 0, 1.40), "spine"),
    ("neck", (0, 0, 1.40), (0, 0, 1.48), "chest"),
    ("head", (0, 0, 1.48), (0, 0, 1.65), "neck"),
]
LIMBS = [
    ("clavicle", (0.02, 0, 1.38), (0.17, 0, 1.37), "chest"),
    ("upper_arm", (0.17, 0, 1.37), (0.27, 0, 1.10), "clavicle"),
    ("forearm", (0.27, 0, 1.10), (0.352, 0, 0.875), "upper_arm"),
    ("hand", (0.352, 0, 0.875), (0.41, 0, 0.715), "forearm"),
    ("thigh", (0.09, 0, 0.86), (0.10, 0, 0.47), "hips"),
    ("shin", (0.10, 0, 0.47), (0.10, 0, 0.085), "thigh"),
    ("foot", (0.10, 0, 0.085), (0.10, -0.12, 0.025), "shin"),
    ("toe", (0.10, -0.12, 0.025), (0.10, -0.19, 0.02), "foot"),
]
for side, sign in (("L", 1), ("R", -1)):
    for name, head, tail, parent in LIMBS:
        parent = parent if parent in ("chest", "hips") else f"{parent}.{side}"
        BONES.append((f"{name}.{side}", (sign * head[0], head[1], head[2]), (sign * tail[0], tail[1], tail[2]), parent))

# The spine's chain: the torso from the crotch to the neck (rings 0 to 8), then the head (8 to 13), the neck's ring
# shared; the shoulders' ring is 6, the crown's 12.
SPINE = [
    ring((0, 0, 0.80), (0.155, 0.10), {"hips": 1}, "hip"),
    ring((0, 0, 0.88), (0.17, 0.11), {"hips": 1}, "hip"),
    ring((0, 0, 1.02), (0.14, 0.095), both("hips", "spine"), "waist"),
    ring((0, 0, 1.10), (0.145, 0.10), {"spine": 1}, "belly"),
    ring((0, 0, 1.25), (0.16, 0.11), both("chest", "spine", 0.75), "chest"),
    ring((0, 0, 1.33), (0.17, 0.105), {"chest": 1}, "chest"),
    ring((0, 0, 1.38), (0.19, 0.09), {"chest": 1}, "shoulder"),
    ring((0, 0, 1.42), (0.07, 0.06), both("neck", "chest", 0.6), "neck"),
    ring((0, 0, 1.47), (0.055, 0.055), both("neck", "head"), "neck"),
    ring((0, 0, 1.50), (0.062, 0.075), both("head", "neck", 0.75), "jaw"),
    ring((0, 0, 1.55), (0.072, 0.092), {"head": 1}, "head"),
    ring((0, 0, 1.60), (0.074, 0.094), {"head": 1}, "head"),
    ring((0, 0, 1.635), (0.055, 0.07), {"head": 1}, "head"),
    ring((0, 0, 1.648), (0.03, 0.04), {"head": 1}, "head"),
]


def arm_chain(side, sign):
    """Shoulder to fingertips: the elbow's ring (4) and the wrist's (8) shared, the weights changing over the rings
    either side of the elbow; the hand flat, its palm to the thigh, so its rings are thin across the side axis and
    wide across the back axis."""

    def b(n):
        return f"{n}.{side}"

    sh, el, wr, tip = (0.17, 0, 1.37), (0.27, 0, 1.10), (0.352, 0, 0.875), (0.41, 0, 0.715)

    def at(p):
        return (sign * p[0], p[1], p[2])

    up, fore, hand = b("upper_arm"), b("forearm"), b("hand")
    return [
        ring(at(sh), 0.05, both(up, b("clavicle"), 0.6), "arm"),
        ring(at(lerp(sh, el, 0.35)), 0.047, {up: 1}, "arm"),
        ring(at(lerp(sh, el, 0.65)), 0.042, both(up, fore, 0.85), "arm"),
        ring(at(lerp(sh, el, 0.85)), 0.038, both(up, fore, 0.65), "arm"),
        ring(at(el), 0.036, both(up, fore), "arm"),
        ring(at(lerp(el, wr, 0.12)), 0.037, both(fore, up, 0.65), "arm"),
        ring(at(lerp(el, wr, 0.3)), 0.037, both(fore, up, 0.85), "arm"),
        ring(at(lerp(el, wr, 0.7)), (0.024, 0.03), both(fore, hand, 0.75), "arm"),
        ring(at(wr), (0.02, 0.027), both(fore, hand), "hand"),
        ring(at(lerp(wr, tip, 0.3)), (0.016, 0.045), both(hand, fore, 0.75), "hand"),
        ring(at(lerp(wr, tip, 0.65)), (0.013, 0.044), {hand: 1}, "hand"),
        ring(at(tip), (0.01, 0.036), {hand: 1}, "hand"),
    ]


def leg_chain(side, sign):
    """Hip to toes: the knee's ring (4) and the ankle's (9) shared, the weights changing over the rings either side
    of the knee; the foot turns forward from the ankle, its sole on the ground."""

    def b(n):
        return f"{n}.{side}"

    def at(x, y, z):
        return (sign * x, y, z)

    thigh, shin = b("thigh"), b("shin")
    return [
        ring(at(0.09, 0, 0.86), 0.085, both(thigh, "hips", 0.6), "leg"),
        ring(at(0.095, 0, 0.70), 0.075, {thigh: 1}, "leg"),
        ring(at(0.10, 0, 0.58), 0.062, both(thigh, shin, 0.85), "leg"),
        ring(at(0.10, 0, 0.52), 0.055, both(thigh, shin, 0.65), "leg"),
        ring(at(0.10, 0, 0.47), 0.05, both(thigh, shin), "leg"),
        ring(at(0.10, 0, 0.42), (0.05, 0.054), both(shin, thigh, 0.65), "leg"),
        ring(at(0.10, 0, 0.36), (0.052, 0.058), both(shin, thigh, 0.85), "leg"),
        ring(at(0.10, 0, 0.19), 0.04, {shin: 1}, "leg"),
        ring(at(0.10, 0, 0.13), 0.034, both(shin, b("foot"), 0.75), "leg"),
        ring(at(0.10, 0, 0.085), (0.032, 0.036), both(shin, b("foot")), "foot"),
        ring(at(0.10, -0.055, 0.05), (0.042, 0.048), both(b("foot"), shin, 0.75), "foot"),
        ring(at(0.10, -0.12, 0.027), (0.047, 0.026), both(b("foot"), b("toe")), "foot"),
        ring(at(0.10, -0.19, 0.02), (0.04, 0.018), {b("toe"): 1}, "foot"),
    ]


UP_SEAM = math.pi / 2  # the back, for a chain running up


def limb_seam(sign):
    return 0.0 if sign > 0 else math.pi  # the inner side, for a limb running down


# Where a sleeve cannot lay its texture within the line, faces take charts (kit.Part.charts): the shoulders' ledge,
# where the torso narrows to the neck in 4 cm; the crown; and from just above the ankle down, where the leg turns
# forward into the foot.
def on_ledge(c):
    return 1.375 < c.z < 1.425


def on_crown(c):
    return c.z > 1.625


def at_ankle(c):
    return c.z < 0.14


kit.start()
arm = kit.armature("person", BONES, group="person", about="an adult 1.65 m tall, standing at rest")
skin = kit.Atlas("person_body", 4.0, 2.0)


def part(name, about, atlas=skin):
    return kit.Part(name, group="person", about=about, armature=arm, atlas=atlas)


p = part("body_torso", "the torso from the crotch to the neck")
build(p, SPINE, 0, 8, 12, UP_SEAM, caps=("flat", None))
p.charts(on_ledge)
p.done()
p = part("body_head", "the head from the neck to the crown, its face drawn later on its front")
build(p, SPINE, 8, 13, 12, UP_SEAM, caps=(None, "flat"))
p.paint("hair", lambda c: c.z > 1.55 - 0.05 * max(-1.0, min(1.0, c.y / 0.09)))  # brow in front, nape behind
p.charts(on_crown)
p.done()

ARMS, LEGS = {}, {}
for side, sign in (("L", 1), ("R", -1)):
    ar = ARMS[side] = arm_chain(side, sign)
    lg = LEGS[side] = leg_chain(side, sign)
    p = part(f"body_upper_arm.{side}", "the upper arm")
    build(p, ar, 0, 4, 8, limb_seam(sign))
    p.done()
    p = part(f"body_forearm.{side}", "the forearm")
    build(p, ar, 4, 8, 8, limb_seam(sign))
    p.done()
    p = part(f"body_hand.{side}", "the hand, a mitten with a thumb, its palm to the thigh")
    build(p, ar, 8, 11, 8, limb_seam(sign), caps=(None, "flat"))
    thumb = [
        ring((sign * 0.372, -0.032, 0.837), 0.012, {f"hand.{side}": 1}, "hand"),
        ring((sign * 0.380, -0.049, 0.810), 0.011, {f"hand.{side}": 1}, "hand"),
        ring((sign * 0.387, -0.060, 0.790), 0.009, {f"hand.{side}": 1}, "hand"),
    ]
    build(p, thumb, 0, 2, 6, UP_SEAM, caps=(None, "flat"))
    p.done()
    p = part(f"body_thigh.{side}", "the thigh")
    build(p, lg, 0, 4, 10, limb_seam(sign))
    p.done()
    p = part(f"body_shin.{side}", "the shin and calf")
    build(p, lg, 4, 9, 10, limb_seam(sign))
    p.charts(at_ankle)
    p.done()
    p = part(f"body_foot.{side}", "the foot, its sole on the ground")
    build(p, lg, 9, 12, 10, limb_seam(sign), caps=(None, "flat"))
    p.charts(at_ankle)
    p.done()


def garment(name, about, atlas):
    return kit.Part(name, group="person", about=about, armature=arm, atlas=atlas)


def mark(p):
    """A garment's object, marked as one so the preview can show the body bare."""
    ob = p.done()
    ob["kit_layer"] = "garment"
    return ob


def dressed(rings, out, joint, parent, child, reach, step=0.03):
    """A garment's rings over a limb's: each `out` metres outside the body's (a list, ring by ring), more of them
    (at most `step` apart), and round the joint ring their weights a straight ramp from the parent bone to the child
    over `reach` metres either side, wider and gentler than the body's, since cloth farther out than the skin folds
    harder in the same bend (a bend crushes the inside of a fold in step with how fast the weights change). Its own
    rings, not the body's: a garment shares no joint ring with anything. Rings are added only where the ramp is."""
    coarse = [0.0]
    for i in range(1, len(rings)):
        coarse.append(coarse[-1] + math.dist(rings[i - 1]["at"], rings[i]["at"]))
    fine = []
    for i, rg in enumerate(rings):
        here = dict(rg, r=(rg["r"][0] + out[i], rg["r"][1] + out[i]), w=dict(rg["w"]))
        if i:
            prev = fine[-1]
            ramped = min(abs(coarse[i - 1] - coarse[joint]), abs(coarse[i] - coarse[joint])) < reach
            n = max(1, math.ceil(math.dist(prev["at"], here["at"]) / step)) if ramped else 1
            for k in range(1, n):
                t = k / n
                r = tuple(a + (b - a) * t for a, b in zip(prev["r"], here["r"], strict=True))
                region = prev["region"] if t < 0.5 else here["region"]
                fine.append(ring(lerp(prev["at"], here["at"], t), r, wlerp(prev["w"], here["w"], t), region))
        if i == joint:
            centre = here["at"]
        fine.append(here)
    along, s = [], 0.0
    for i, rg in enumerate(fine):
        s += math.dist(fine[i - 1]["at"], rg["at"]) if i else 0.0
        along.append(s)
    at_joint = along[next(i for i, rg in enumerate(fine) if rg["at"] == centre)]
    for rg, s in zip(fine, along, strict=True):
        d = s - at_joint
        if abs(d) <= reach:
            c = (d + reach) / (2 * reach)
            rg["w"] = {parent: 1 - c, child: c}
    return fine


# The tunic: a body from the shoulders to mid-thigh, its skirt hanging free over the legs, and sleeves to
# mid-forearm; fur at the hem and the cuffs.
tunic = garment(
    "garment_tunic",
    "a pullover tunic of hide to mid-thigh, sleeves to mid-forearm, fur-trimmed",
    kit.Atlas("person_tunic", 2.0, 2.0),
)
skirt = [
    ring((0, 0, 0.62), (0.205, 0.14), {"hips": 1}, "hip"),
    ring((0, 0, 0.655), (0.204, 0.14), {"hips": 1}, "hip"),
    ring((0, 0, 0.74), (0.20, 0.138), {"hips": 1}, "hip"),
]
coat = skirt + [dict(rg, r=(rg["r"][0] + 0.022, rg["r"][1] + 0.022)) for rg in SPINE[1:8]]
first = build(tunic, coat, 0, 1, 12, UP_SEAM, role="fur")
build(tunic, coat, 1, len(coat) - 1, 12, UP_SEAM, role="hide")
tunic.bm.verts.ensure_lookup_table()
for i in range(first, len(tunic.bm.verts)):  # the skirt follows each thigh a quarter, so the legs move under it
    v = tunic.bm.verts[i]
    if v.co.z < 0.80 and abs(v.co.x) > 0.04:
        tunic.weights[i] = {"hips": 0.75, ("thigh.L" if v.co.x > 0 else "thigh.R"): 0.25}
for side, sign in (("L", 1), ("R", -1)):
    ar = ARMS[side]

    def down(t, ar=ar):  # the arm's ring t of the way from its ring at 30% of the forearm to its ring at 70%
        r = tuple(a + (b - a) * t for a, b in zip(ar[6]["r"], ar[7]["r"], strict=True))
        return ring(lerp(ar[6]["at"], ar[7]["at"], t), r, wlerp(ar[6]["w"], ar[7]["w"], t), "arm")

    # loose at the shoulder, fitted at the elbow so its inner fold does not crush, easing to a fur cuff at 45% of
    # the forearm, the band's top at about 41%
    arm_rings = ar[:7] + [down(0.21), down(0.375)]
    out = (0.02, 0.019, 0.017, 0.015, 0.014, 0.015, 0.017, 0.025, 0.026)
    fine = dressed(arm_rings, out, 4, f"upper_arm.{side}", f"forearm.{side}", 0.16)
    band = next(i for i, rg in enumerate(fine) if rg["at"] == arm_rings[7]["at"])
    build(tunic, fine, 0, band, 8, limb_seam(sign), role="hide")
    build(tunic, fine, band, len(fine) - 1, 8, limb_seam(sign), role="fur")
tunic.charts(on_ledge)
mark(tunic)

# The leggings: two tubes from inside the tunic down over the thighs and shins to the ankle.
leggings = garment(
    "garment_leggings", "hide leggings, each leg from the hip to the ankle", kit.Atlas("person_leggings", 2.0, 1.0)
)
for side, sign in (("L", 1), ("R", -1)):
    out = (0.01, 0.01, 0.01, 0.008, 0.007, 0.008, 0.01, 0.01, 0.01, 0.01)
    hose = dressed(LEGS[side][:10], out, 4, f"thigh.{side}", f"shin.{side}", 0.16, step=0.04)
    build(leggings, hose, 0, len(hose) - 1, 10, limb_seam(sign), role="hide")
leggings.charts(at_ankle)
mark(leggings)

# The boots: soft moccasin boots from mid-calf over the ankle and the foot, a fur band at the top, outside the
# leggings.
boots = garment(
    "garment_boots", "soft hide boots to mid-calf, a fur band at the top", kit.Atlas("person_boots", 2.0, 1.0)
)
for side, sign in (("L", 1), ("R", -1)):
    lg = LEGS[side]
    top = lerp(lg[6]["at"], lg[7]["at"], 0.4)
    shaft = [
        ring(top, (0.05, 0.054), {f"shin.{side}": 1}, "leg"),
        ring(lerp(top, lg[7]["at"], 0.18), (0.048, 0.052), {f"shin.{side}": 1}, "leg"),
    ] + lg[7:]
    build(boots, shaft, 0, 1, 10, limb_seam(sign), role="fur", offset=0.02)
    build(boots, shaft, 1, len(shaft) - 1, 10, limb_seam(sign), caps=(None, "flat"), role="hide", offset=0.02)
boots.charts(at_ankle)
mark(boots)

# The bend test: knees and elbows bent 60 degrees, hips 30 forward, the neck 15 down (signs found in world space).
turns = {"neck": (15, 0, 0)}
for side in ("L", "R"):
    turns[f"thigh.{side}"] = (-30, 0, 0)
    turns[f"shin.{side}"] = (60, 0, 0)
    turns[f"forearm.{side}"] = (-60, 0, 0)
kit.pose(arm, "bend_test", turns)
bpy.context.view_layer.update()
kit.finish(sys.argv[-1])
