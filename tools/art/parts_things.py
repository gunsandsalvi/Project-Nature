"""The camp's things for the camp family (A6.1, A6.3, A6.4, T2.3c.2): the hearth ring (sheet 16.13), the firewood
pile (16.23), and later the shelter screen and the drying rack. Imported by tools/art/parts_camp.py, which makes the
file; run inside Blender. Implements PRE-46 and PRE-22.

Every part lies on z = 0 (a stone) or is modelled about its own middle (a stick: its axis along x, its middle on the
origin, so a recipe places it by where its middle goes), with texture coordinates in metres (a texture pixel 1/64 m at
band 0) and a baked crease. Forms by size on screen (A6.3), each from the same numbers as the full part: the full
form's own name, `_simple` and `_marker`.

The hearth ring (16.13): twelve stones, each its own part `hearth_stone_01` to `hearth_stone_12` (a tangential width
by a radial depth by a height, from the design's list: long flattened ovals, small cobbles, angular lumps, a
kidney, a wedge), modelled with the stone's depth along y (outward when the recipe places it on its ring) and its width
along x. Warm grey and slate stones wear the stone texture and are darkened to their tone by the part's crease, the
pale buff ones the limestone's, the pale grey ones the granite's.

The firewood pile (16.23): a dead branch of every length and diameter the design lists (`firewood_branch_<cm>_<cm>`:
its length and diameter in centimetres, one slight elbow, broken splintered ends, a stub or two), the three trunk pieces
(`firewood_trunk_15`, `_18`, `_20`), a bundle of kindling (`firewood_kindling`), and for the smallest form a low mound
(`firewood_mound`) standing in for the whole heap's sticks. All of them wear the bark texture.
"""

import math
import os
import sys

import bmesh
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import parts_rocks as rocks  # noqa: E402
from standins import Builder, lathe, noise  # noqa: E402

FORMS = ("", "_simple", "_marker")

# ---- the hearth ring ---------------------------------------------------------------------------------------------

# the design's stones, clockwise from 12 o'clock: id, width (tangential), depth (radial), height in metres, the form,
# the role (its texture) and the tone its crease gives (1 leaves the texture as it is)
HEARTH = [
    (1, 0.30, 0.20, 0.13, "oval", "granite", 0.60),
    (2, 0.18, 0.16, 0.10, "cobble", "limestone", 0.68),
    (3, 0.27, 0.20, 0.20, "angular", "granite", 0.50),
    (4, 0.22, 0.18, 0.12, "kidney", "granite", 0.62),
    (5, 0.16, 0.15, 0.10, "squat", "limestone", 0.66),
    (6, 0.29, 0.20, 0.15, "wedge", "stone", 0.68),
    (7, 0.24, 0.20, 0.12, "oval", "granite", 0.76),
    (8, 0.19, 0.17, 0.11, "cobble", "limestone", 0.67),
    (9, 0.30, 0.20, 0.16, "oval", "granite", 0.46),
    (10, 0.21, 0.18, 0.14, "trapezoid", "stone", 0.82),
    (11, 0.26, 0.20, 0.13, "kidney", "limestone", 0.70),
    (12, 0.17, 0.16, 0.10, "squat", "granite", 0.62),
]
# the dome stones' numbers: how square the footprint is, how flat the crown, how steep the flank, the skew
DOMES = {
    "oval": {"squareness": 2.0, "crown": 9.0, "flank": 0.30, "skew": 0.10, "steep": 0.10},
    "cobble": {"squareness": 2.3, "crown": 5.0, "flank": 0.42, "skew": -0.12, "steep": 0.20},
    "squat": {"squareness": 2.1, "crown": 4.2, "flank": 0.42, "skew": 0.08, "steep": 0.15},
    "kidney": {"squareness": 2.0, "crown": 5.4, "flank": 0.42, "skew": 0.10, "steep": 0.18},
}
DOME_FORMS = [(5, 14), (3, 8), (2, 6)]  # rings and segments of a round stone in the full, simple and marker forms
HULL_FORMS = [(10, 8), (6, 3), (5, 1)]  # the base and top points of an angular stone
FLAT = {"oval": 0.62, "cobble": 0.80, "squat": 0.80, "kidney": 0.74}  # how much lower than the design's height a round
# stone is made (a river stone lies flatter than it looks, and a long one is a slab)
WIDER = {"oval": 1.14, "cobble": 1.08, "squat": 1.08, "kidney": 1.10}  # and how much wider, so it is broad and low


def angular_points(spec, seed, base_points, top_points):
    """The corners of an angular stone: its base round the footprint (z = 0) a little irregular, its top points drawn in
    and lower, a wedge narrowing toward one end and a trapezoid blunt."""
    _, w, d, h, kind, _, _ = spec
    a, b = w / 2.0, d / 2.0
    points = []
    for k in range(base_points):
        t = 2.0 * math.pi * (k + 0.35 * noise(seed, k, 1)) / base_points
        r = 0.78 + 0.24 * noise(seed, k, 2)
        x, y = a * math.cos(t) * r * 1.12, b * math.sin(t) * r * 1.12
        if kind == "wedge":
            y *= 0.40 + 0.60 * (x + a) / (2.0 * a)
        if kind == "trapezoid":
            x, y = max(-a, min(a, x * 1.15)), max(-b, min(b, y * 1.15))
        points.append((x, y, 0.0))
    for k in range(top_points):
        t = 2.0 * math.pi * (k + 0.5 + 0.3 * noise(seed, k, 3)) / max(1, top_points)
        r = 0.52 + 0.30 * abs(noise(seed, k, 4))
        x, y = a * math.cos(t) * r, b * math.sin(t) * r
        z = h * (0.76 + 0.24 * abs(noise(seed, k, 5)))
        if kind == "wedge":
            z = h * (0.45 + 0.55 * (x + a) / (2.0 * a))
            y *= 0.40 + 0.60 * (x + a) / (2.0 * a)
        points.append((x, y, z))
    return points


def hearth_stone(spec, form):
    """One stone of the ring in one form (0 full, 1 simple, 2 marker)."""
    number, w, d, h, kind, role, tone = spec
    name = f"hearth_stone_{number:02d}{FORMS[form]}"
    builder = Builder(name)
    seed = 70 + number

    def crease(p, n):
        return tone * (0.60 + 0.40 * min(1.0, max(0.0, p.z / (0.8 * h)))) * (0.88 + 0.12 * max(0.0, n.z))

    if kind in DOMES:
        rings, segments = DOME_FORMS[form]
        dome = rocks.Dome(
            WIDER[kind] * w / 2.0,
            WIDER[kind] * d / 2.0,
            FLAT[kind] * h,
            seed,
            shift=(0.12 * (w / 2.0) * (number % 3 - 1), 0.0),
            steep_phi=0.4 * number,
            **DOMES[kind],
        )
        rocks.dome_faces(builder, dome, rings, segments)
        rocks.drop_loose(builder.bm)
        if kind == "kidney":  # the footprint curved, as a kidney is
            for v in builder.bm.verts:
                v.co.y += (0.30 if number % 2 else -0.30) * (d / 2.0) * (1.0 - (v.co.x / (w / 2.0)) ** 2)
        rocks.lay_faces(builder, lambda f: role, crease, True)
    else:
        base_points, top_points = HULL_FORMS[form]
        rocks.hull(builder, angular_points(spec, seed, base_points, top_points), bevel=0.0 if form else 0.004)
        rocks.lay_faces(builder, lambda f: role, crease, False)
    return builder.build((0.0, 0.0, 0.0))


def hearth_stones():
    return [hearth_stone(spec, form) for form in range(3) for spec in HEARTH]


# ---- the firewood ------------------------------------------------------------------------------------------------

# the design's branches in four layers: lengths in metres for each layer; the diameters cycle 0.03, 0.05, 0.07, 0.10
LAYERS = [
    [1.2, 1.1, 1.0, 0.9, 0.8, 0.7, 0.6, 1.0],
    [1.2, 1.1, 1.0, 0.9, 0.8, 0.7, 1.0],
    [1.1, 1.0, 0.9, 0.8, 0.7],
    [1.0, 0.9, 0.8, 0.6],
]
DIAMETERS = [0.04, 0.06, 0.08, 0.11]  # the design's 0.03 to 0.10 made a little stouter, as the sheet's heap shows
TRUNKS = [0.15, 0.18, 0.20]  # the fallen trunk's pieces, each 0.8 m long
# the sticks the heap needs beyond the design's twenty-four to be as dense as the sheet's picture of it: more in each
# layer, and so a heap of 24 + 22 sticks and three trunk pieces
EXTRA = [[1.0, 0.8, 0.9, 0.7, 0.6, 0.9], [0.9, 0.8, 0.7, 0.6, 0.7, 0.8], [0.9, 0.8, 0.7], [0.8, 0.7, 0.6]]


def cm(metres):
    return round(metres * 100.0)


def branch_name(length, diameter, form=0):
    return f"firewood_branch_{cm(length):03d}_{cm(diameter):02d}{FORMS[form]}"


def stick(name, length, diameter, seed, form, stubs=2, bend=0.04, taper=0.18, role="bark"):
    """A dead branch or a trunk piece lying along x with its middle on the origin and its axis on z = 0: a tube whose
    radius tapers a little and swells and shrinks by its seed, its axis bent by one slow elbow, its two ends broken
    (every point of the end ring pulled out or in by its own amount, the way a snapped stick splinters), and, in the
    full form, `stubs` broken twig stubs. Its faces take the bark's texture coordinates, round it and along it."""
    segments = (8, 5, 4)[form]
    rings = max(2, round(length / 0.22)) if form == 0 else (3 if form == 1 else 2)
    r0 = diameter / 2.0
    profile = []
    for k in range(rings + 1):
        s = length * k / rings
        swell = 1.0 + 0.10 * math.sin(2.7 * s / max(length, 0.3) * math.pi + seed) + 0.05 * noise(seed, k, 8)
        profile.append((s - length / 2.0, r0 * (1.0 - taper * s / length) * swell))
    builder = Builder(name)

    def crease(p, n):
        underside = 0.38 + 0.62 * (0.5 + 0.5 * n.z) ** 1.2  # round: lit on top, shaded on its underside
        return underside * (0.88 + 0.12 * min(1.0, abs(p.x) / (length / 2.0)))

    lathe(builder, "x", profile, segments, role, crease, bumps=0.03, seed=seed, caps=(True, True))
    ends = [profile[0][0], profile[-1][0]]
    for v in builder.bm.verts:
        x, y, z = v.co
        along = (x + length / 2.0) / length
        elbow = bend * length * math.sin(math.pi * along) * (1.0 if seed % 2 else -1.0)
        v.co.z += elbow * 0.6
        v.co.y += elbow * 0.8
        for end_x in ends:
            if abs(x - end_x) < 1e-6 and form == 0:  # a snapped end: each point of it pulled by its own amount
                angle = math.atan2(z, y)
                spike = abs(noise(seed, round(angle * 3.0), 13 if end_x > 0 else 14)) > 0.7  # a few short splinters
                pull = diameter * 0.10 * noise(seed, round(angle * 4.0), 11 if end_x > 0 else 12)
                pull += min(0.42 * diameter, 0.028) if spike else 0.0  # a splinter is never longer than 3 cm
                v.co.x += pull if end_x > 0 else -pull
    if form == 0:
        for k in range(stubs):  # broken twig stubs: a short four-sided cone from the side of the tube
            at = (0.25 + 0.5 * abs(noise(seed, k, 20))) * length - length / 2.0
            angle = math.pi * (0.2 + 1.6 * abs(noise(seed, k, 21)))
            base = Vector((at, r0 * 1.1 * math.cos(angle), r0 * 1.1 * math.sin(angle)))
            direction = Vector((0.3, math.cos(angle), math.sin(angle))).normalized()
            tip = base + direction * (0.035 + 0.02 * abs(noise(seed, k, 22)))
            side = direction.cross(Vector((1.0, 0.0, 0.0))).normalized() * (0.006 + 0.03 * r0)
            up = direction.cross(side).normalized() * side.length
            vs = [builder.bm.verts.new(c) for c in (base + side, base + up, base - side, base - up)]
            apex = builder.bm.verts.new(tip)
            for i in range(4):
                builder.bm.faces.new([vs[i], vs[(i + 1) % 4], apex]).tag = True
    builder.bm.normal_update()

    def is_cap(f):  # an end: more than four sides, or four all at one distance along the stick
        return len(f.verts) > 4 or all(abs(v.co.x - f.verts[0].co.x) < 1e-9 for v in f.verts)

    for f in builder.bm.faces:  # along the tube the texture follows the vertices where the ends' splinters put them
        if len(f.verts) == 4 and not is_cap(f):
            for loop in f.loops:
                loop[builder.uv].uv = (loop[builder.uv].uv[0], loop.vert.co.x + length / 2.0)
    caps = [f for f in builder.bm.faces if is_cap(f)]  # the broken ends, in triangles, each on its own plane
    stub_faces = [f for f in builder.bm.faces if f.tag]  # (a bmesh operator clears the tags, so the list comes first)
    flat = bmesh.ops.triangulate(builder.bm, faces=caps)["faces"] + stub_faces
    builder.bm.normal_update()
    own_plane(builder, flat[: len(flat) - len(stub_faces)], "granite", 0.92)  # broken ends: pale weathered wood
    own_plane(builder, stub_faces, role, 0.78)
    return builder.build((0.0, 0.0, 0.0))


def own_plane(builder, faces, role, crease):
    """The given faces each on the flat projection of its own plane (metres: along the face, up its slope), in `role`,
    with a crease of `crease`: the broken ends of a stick and its stubs, which the tube's wrapped texture does not
    fit."""
    for f in faces:
        n = f.normal.copy()
        n.normalize()
        f.smooth = False
        f.material_index = builder.slot(role)
        t = Vector((0.0, 0.0, 1.0)).cross(n) if abs(n.z) < 0.95 else Vector((1.0, 0.0, 0.0))
        t.normalize()
        w = n.cross(t)
        for loop in f.loops:
            loop[builder.uv].uv = (loop.vert.co.dot(t), loop.vert.co.dot(w))
            loop[builder.crease] = (crease, crease, crease, 1.0)


def kindling(form):
    """Twelve dry twigs, 0.5 m long and 8 to 15 mm thick, loose in a small heap, none bound."""
    builder = Builder("firewood_kindling" + FORMS[form])
    count = 12 if form == 0 else 6 if form == 1 else 3
    for k in range(count):
        thick = 0.008 + 0.007 * abs(noise(40, k, 1))
        t = 2.0 * math.pi * k / count
        centre = Vector((0.07 * noise(41, k, 2), 0.06 * noise(41, k, 3) + 0.04 * math.sin(t), thick + 0.012 * (k % 3)))
        bearing = math.radians(90.0 + 22.0 * noise(42, k, 4))
        axis = Vector((math.sin(bearing), math.cos(bearing), 0.0))
        side = Vector((axis.y, -axis.x, 0.0))
        segs = 5 if form == 0 else 4
        rings = [(-0.25, 0.8), (0.0, 1.0), (0.25, 0.7)] if form == 0 else [(-0.25, 0.8), (0.25, 0.7)]
        ring_verts = []
        for s, f in rings:
            ring = []
            for j in range(segs):
                a = 2.0 * math.pi * j / segs
                p = centre + axis * s + side * (thick * f * math.cos(a)) + Vector((0, 0, thick * f * math.sin(a)))
                ring.append(builder.bm.verts.new(p))
            ring_verts.append(ring)
        for r in range(len(rings) - 1):
            for j in range(segs):
                n = (j + 1) % segs
                builder.bm.faces.new([ring_verts[r][j], ring_verts[r][n], ring_verts[r + 1][n], ring_verts[r + 1][j]])
        for ring in (ring_verts[0], ring_verts[-1]):
            builder.bm.faces.new(ring)
    bmesh.ops.triangulate(builder.bm, faces=list(builder.bm.faces))
    builder.bm.normal_update()
    rocks.lay_faces(builder, lambda f: "bark", lambda p, n: 0.65 + 0.35 * min(1.0, p.z / 0.04), False)
    return builder.build((0.0, 0.0, 0.0))


def mound(form):
    """The firewood heap as one low lump for the smallest form: 1.4 m by 0.9 m, 0.6 m high at its crest off to the left
    of the middle, built as a convex hull, its sticks and trunks seen only as the bark's grain."""
    builder = Builder("firewood_mound" + FORMS[form])
    pts = [(-0.70, 0.0, 0.0), (-0.55, -0.40, 0.0), (0.0, -0.45, 0.0), (0.65, -0.40, 0.0), (0.72, 0.05, 0.0)]
    pts += [(0.60, 0.42, 0.0), (0.0, 0.45, 0.0), (-0.55, 0.38, 0.0)]
    pts += [(-0.45, -0.18, 0.30), (-0.10, -0.15, 0.58), (-0.15, 0.12, 0.60), (-0.50, 0.15, 0.32), (0.30, -0.12, 0.30)]
    pts += [(0.35, 0.14, 0.28), (0.62, -0.05, 0.20)]
    rocks.hull(builder, pts)
    rocks.lay_faces(builder, lambda f: "bark", lambda p, n: 0.6 + 0.4 * min(1.0, p.z / 0.5), False)
    return builder.build((0.0, 0.0, 0.0))


def firewood_parts():
    """Every part of the firewood: the sticks the design lists, the trunks, the kindling and the mound, in forms."""
    out = []
    seen = set()
    for layer in [*LAYERS, *EXTRA]:
        for k, length in enumerate(layer):
            diameter = DIAMETERS[k % 4]
            if (length, diameter) in seen:
                continue
            seen.add((length, diameter))
            for form in range(2):  # the marker form of the whole heap is the mound
                out.append(
                    stick(branch_name(length, diameter, form), length, diameter, 100 + cm(length) + cm(diameter), form)
                )
    for dia in TRUNKS:
        for form in range(3):
            out.append(
                stick(f"firewood_trunk_{cm(dia):02d}{FORMS[form]}", 0.8, dia, 200 + cm(dia), form, 1, 0.012, 0.06)
            )
    out += [kindling(form) for form in range(3)]
    out += [mound(form) for form in (0, 2)]
    return out


def make_parts():
    """Every part of the camp's things so far."""
    return [*hearth_stones(), *firewood_parts()]
