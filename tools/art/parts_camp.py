"""The art lane's camp family of parts (A6.1, A6.3, A6.4): what art/models/camp.blend is made from, a Blender script
whose every number is written here, so the file can be made again and each part judged against its catalogue sheet.
Run inside Blender, headless, as

    blender --background --factory-startup --python tools/art/parts_camp.py -- <out.blend> [<out.kdkit>]

It makes the parts in memory, keeps them as a Blender file, and exports them as any family is (tools/blender/export.py),
so the exporter's own checks (a part's name, its role slots, its joints) are made at once. Texture coordinates are in
metres with a texture pixel 1/64 m at band 0, each wrap round a stick a whole number of texture pixels (A6.4); every
part carries a baked crease. The lathe, the builder of faces and the joints are the stand-in family's own helpers
(tools/blender/standins.py), written once. Implements PRE-46 and PRE-22.

The club (sheet 17.15) is a lathe of rings along x, its butt at the origin and its knob at 0.70 m, in three forms by its
size on screen (A6.3): `club`, the full form with its trimmed root stubs, `club_simple` for about 40 to 100 screen
pixels (up close it is about 90 px long) and `club_small` for the camp zoom. Every form shares the full one's
texture layout and joints.

Texture layout (A6.4): the shaft keeps the lathe's own layout, round it and along it, so its grain runs along the stick;
where the surface slopes steeply from the axis (the pommel, the knob and its end) a texture wrapped round it would
stretch past 1.5 to 1 at the seam, so each face there takes its own flat projection in metres, as the stones do.
"""

import math
import os
import sys

import bpy
from mathutils import Euler, Vector

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "blender"))
import export  # noqa: E402
from standins import Builder, joint, lathe  # noqa: E402

# ---- the club ---------------------------------------------------------------------------------------------------

# (distance from the butt, radius) in metres: a small pommel, a shaft of 3 cm across that thickens slowly, then the
# root knob 9 cm across at its widest, rounded at its end (the sheet's flat view)
CLUB = [
    (0.000, 0.0080), (0.003, 0.0140), (0.009, 0.0180), (0.020, 0.0190), (0.030, 0.0160), (0.038, 0.0135),
    (0.060, 0.0138), (0.120, 0.0145), (0.200, 0.0152), (0.280, 0.0156), (0.360, 0.0162), (0.420, 0.0175),
    (0.470, 0.0195), (0.510, 0.0235), (0.540, 0.0290), (0.570, 0.0350), (0.600, 0.0405), (0.630, 0.0440),
    (0.650, 0.0450), (0.670, 0.0440), (0.685, 0.0390), (0.696, 0.0290), (0.702, 0.0150), (0.704, 0.0030),
]  # fmt: skip

# the trimmed root stubs of the knob (the design's four): where along the club, the direction out of it, the radius of
# the cut face, how far it stands out of the knob's surface; every one inside the knob's 9 cm
STUBS = [
    (0.62, (0.15, -0.70, 0.70), 0.0100, 0.012),  # front, upper: the largest cut face, 2 cm
    (0.66, (0.00, -0.70, -0.70), 0.0060, 0.008),  # front, lower, 1.2 cm
    (0.64, (0.00, 0.10, 1.00), 0.0080, 0.010),  # on top, 1.6 cm
    (0.67, (0.00, 0.90, 0.30), 0.0040, 0.006),  # on the reverse, 0.8 cm
]

# the club's own forms by its length on screen: the segments round it, every how-many-th ring of CLUB is kept, and how
# many stubs stand out (A6.3: each form made from the full one, the same layout and joints)
CLUB_FORMS = {"club": (16, 1, 4), "club_simple": (10, 2, 2), "club_small": (6, 4, 0)}


def radius_at(x):
    """The club's radius at a distance from the butt, between the rings of the full profile."""
    for (x0, r0), (x1, r1) in zip(CLUB, CLUB[1:], strict=False):
        if x0 <= x <= x1:
            return r0 + (r1 - r0) * (x - x0) / max(x1 - x0, 1e-9)
    return 0.0


def crease_of(point, normal):
    """Baked darkening (1 open, 0 dark): the underside darker than the top, and the grip, the first 18 cm from the butt,
    a fifth darker than the rest, as the sheet's hand-worn handle is, fading out over the next 7 cm. At the game's
    size this is how the grip's darker, smoother look is carried: by the part, since a texture pixel is wider than
    the stick's own grain."""
    grip = 0.78 + 0.22 * min(1.0, max(0.0, (point.x - 0.18) / 0.07)) if point.x < 0.25 else 1.0
    return (0.60 + 0.40 * (0.5 + 0.5 * normal.z)) * grip


def rotation_to(direction):
    """The matrix that turns the z axis to a direction."""
    d = Vector(direction).normalized()
    return Vector((0.0, 0.0, 1.0)).rotation_difference(d).to_matrix().to_4x4()


def knob_asymmetry(ring_vertices):
    """The knob made irregular, as a root is: its rings squeezed and pushed a little to one side, the same everywhere in
    one form, smooth round the axis."""
    for v in ring_vertices:
        x = v.co.x
        grow = min(1.0, max(0.0, (x - 0.46) / 0.14))  # nothing on the shaft, all of it at the knob
        phi = math.atan2(v.co.z, v.co.y)
        scale = 1.0 + grow * (0.07 * math.cos(phi - 0.9) + 0.05 * math.cos(2.0 * phi + 0.4))
        v.co.y *= scale
        v.co.z = v.co.z * scale + grow * 0.004  # the knob's middle a little above the shaft's


def project_faces(builder, steep):
    """Each face where `steep(x)` says the surface slopes too steeply for the lathe's layout takes its own flat
    projection in metres (its plane's two axes, the second running toward the knob), which carries a texture pixel over
    without stretch; the texture pixels of two such faces need not line up where they meet, as A6.4 allows."""
    builder.bm.normal_update()
    for f in builder.bm.faces:
        centre = f.calc_center_median()
        if not steep(centre.x):
            continue
        n = f.normal.normalized()
        t = Vector((1.0, 0.0, 0.0)).cross(n)
        if t.length < 0.3:  # a face looking along the club: its plane's own axes
            t = Vector((0.0, 1.0, 0.0))
        t.normalize()
        w = n.cross(t)
        if w.x < 0.0:
            t, w = -t, -w
        for loop in f.loops:
            loop[builder.uv].uv = (loop.vert.co.dot(t), loop.vert.co.dot(w))


def club_part(name, segments, every, stubs):
    """One form of the club: a lathe of CLUB's rings (every `every`-th, always the last), its knob made irregular, and
    `stubs` of the trimmed root stubs, each a short tube with a flat cut face at its end."""
    profile = CLUB[::every]
    if profile[-1] != CLUB[-1]:
        profile = [*profile, CLUB[-1]]
    b = Builder(name)
    lathe(b, "x", profile, segments, "wood", crease_of, bumps=0.035, seed=3, caps=(True, True))
    knob_asymmetry(list(b.bm.verts))
    project_faces(b, lambda x: x < 0.04 or x > 0.50)  # the pommel, the knob and its end
    for x, direction, r, out in STUBS[:stubs]:
        before = set(b.bm.verts)
        top = radius_at(x) + out
        lathe(b, "z", [(radius_at(x) * 0.8, r), (top, r)], max(5, segments // 3), "wood", crease_of, caps=(False, True))
        turn = rotation_to(direction)
        for v in b.bm.verts:
            if v not in before:
                v.co = turn @ v.co + Vector((x, 0.0, 0.0))
    obj = b.build((0.0, 0.0, 0.0))
    # the joints, their own z axis along the club, toward its knob: where the hand holds it, a hand's width from the
    # butt, and the middle of the knob, which strikes
    along = Euler((0.0, math.pi / 2.0, 0.0), "XYZ")
    joint(obj, "grip", (0.11, 0.0, 0.0), tuple(along))
    joint(obj, "head", (0.64, 0.0, 0.0), tuple(along))
    return obj


def make_family():
    """Every part of the camp family, laid in a row in the file, which the exporter does not care about."""
    parts = [club_part(name, *spec) for name, spec in CLUB_FORMS.items()]
    x = 0.0
    for obj in parts:
        obj.location = (x, 0.0, 0.0)
        x += 1.0
    return parts


def main():
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    if not 1 <= len(argv) <= 2:
        raise SystemExit("usage: blender --background --python tools/art/parts_camp.py -- <out.blend> [<out.kdkit>]")
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj)
    make_family()
    bpy.context.preferences.filepaths.save_version = 0  # no .blend1 backup beside the file
    bpy.ops.wm.save_as_mainfile(filepath=argv[0], compress=True)
    if len(argv) == 2:
        try:
            export.export(argv[1])
        except export.KitError as e:
            print(f"KDKIT: {e}", file=sys.stderr)
            raise SystemExit(1) from None


if __name__ == "__main__":
    main()
