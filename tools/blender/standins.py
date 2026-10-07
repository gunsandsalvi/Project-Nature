"""The stand-in family of parts, made by a Blender script until the art lane's own parts arrive (A6.1, A6.4): a club,
the poles, hide cover, door flap, stones and binding of a cone tent, each a Blender object with its joints as empties,
its material slots named by role, its texture coordinates in metres and its crease darkening baked in, as the art
lane's parts are held to (tools/blender/export.py). Run by tools/kit.py as

    blender --background --factory-startup --python tools/blender/standins.py -- <out.kdkit> [<out.blend>]

It makes the family in memory, exports it as any family is, and may keep it as a Blender file too. Implements PRE-46.
"""

import math
import os
import sys

import bmesh
import bpy
from mathutils import Euler, Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import export  # noqa: E402

TEXEL = 1.0 / 64.0  # a texture pixel at band 0, in metres


def whole_wraps(circumference):
    """A circumference in metres rounded to a whole number of texture pixels, at least one: the length of texture
    that goes once round a tube, so the wrap ends on a texture pixel's edge (A6.4)."""
    return max(1, round(circumference / TEXEL)) * TEXEL


def noise(*values):
    """A number from -1 to 1 for whole numbers, the same on every machine."""
    h = 2166136261
    for v in values:
        h = ((h ^ (int(v) & 0xFFFFFFFF)) * 16777619) & 0xFFFFFFFF
        h ^= h >> 15
        h = (h * 2246822519) & 0xFFFFFFFF
        h ^= h >> 13
    return h / 0x7FFFFFFF - 1.0


def role(name):
    return bpy.data.materials.get(name) or bpy.data.materials.new(name)


class Builder:
    """A part being built in a bmesh: its faces carry a role, and every corner its texture coordinates in metres and
    its crease."""

    def __init__(self, name):
        self.name = name
        self.bm = bmesh.new()
        self.uv = self.bm.loops.layers.uv.new("UVMap")
        self.crease = self.bm.loops.layers.float_color.new("crease")
        self.roles = []

    def slot(self, role_name):
        if role_name not in self.roles:
            self.roles.append(role_name)
        return self.roles.index(role_name)

    def face(self, points, uvs, role_name, crease, smooth=True):
        """A face of 3 or more points, wound counter-clockwise seen from outside, each with its uv and crease; the
        points are Vectors, or bmesh vertices already made."""
        verts = [p if isinstance(p, bmesh.types.BMVert) else self.bm.verts.new(p) for p in points]
        f = self.bm.faces.new(verts)
        f.smooth = smooth
        f.material_index = self.slot(role_name)
        for loop, uv, c in zip(f.loops, uvs, crease, strict=True):
            loop[self.uv].uv = uv
            loop[self.crease] = (c, c, c, 1.0)
        return f

    def build(self, location):
        mesh = bpy.data.meshes.new(self.name)
        self.bm.to_mesh(mesh)
        self.bm.free()
        obj = bpy.data.objects.new(self.name, mesh)
        obj.location = location
        bpy.context.scene.collection.objects.link(obj)
        for r in self.roles:
            obj.data.materials.append(role(r))
        return obj


def joint(parent, name, at, turn=(0.0, 0.0, 0.0)):
    """An empty parented to a part, named joint_<name>: its place and axes are the joint's."""
    empty = bpy.data.objects.new("joint_" + name, None)
    empty.empty_display_type = "ARROWS"
    empty.empty_display_size = 0.05
    bpy.context.scene.collection.objects.link(empty)
    empty.parent = parent
    empty.location = at
    empty.rotation_euler = Euler(turn, "XYZ")
    return empty


def ring_point(axis, s, r, theta):
    """A point at s along an axis ('x' or 'z') and radius r, at an angle round it, right-handed about the axis."""
    c, s_ = math.cos(theta), math.sin(theta)
    return Vector((s, r * c, r * s_)) if axis == "x" else Vector((r * c, r * s_, s))


def lathe(
    builder, axis, profile, segments, role_name, crease, bumps=0.0, seed=0, caps=(True, True), seam=-math.pi / 2.0
):
    """A tube of revolution about an axis through the profile's rings, each (s along the axis, radius), the surface
    smooth, its texture coordinates u round it and v along it in metres, each ring's circumference a whole number of
    texture pixels, its u running from half a wrap before the middle of the side opposite the seam to half a wrap after,
    which halves the shear between rings of different radii; the seam, where the wrap ends, at an angle round the
    axis, underneath by default; flat caps at its ends with planar texture coordinates. crease(point, normal) gives the
    darkening at a corner."""
    rings = []
    for k, (s, r) in enumerate(profile):
        ring = []
        for j in range(segments):
            bump = 1.0 + bumps * noise(seed, k, j)
            ring.append(builder.bm.verts.new(ring_point(axis, s, r * bump, seam + 2.0 * math.pi * j / segments)))
        rings.append(ring)
    along = [0.0]
    for k in range(1, len(profile)):
        along.append(along[-1] + math.hypot(profile[k][0] - profile[k - 1][0], profile[k][1] - profile[k - 1][1]))
    for k in range(len(profile) - 1):
        wrap_a = whole_wraps(2.0 * math.pi * profile[k][1])
        wrap_b = whole_wraps(2.0 * math.pi * profile[k + 1][1])
        for j in range(segments):
            n = (j + 1) % segments
            quad = [rings[k][j], rings[k][n], rings[k + 1][n], rings[k + 1][j]]
            u = [
                wrap_a * (j / segments - 0.5),
                wrap_a * ((j + 1) / segments - 0.5),
                wrap_b * ((j + 1) / segments - 0.5),
                wrap_b * (j / segments - 0.5),
            ]
            v = [along[k], along[k], along[k + 1], along[k + 1]]
            normals = [
                (p.co - ring_point(axis, p.co.x if axis == "x" else p.co.z, 0.0, 0.0)).normalized() for p in quad
            ]
            builder.face(
                quad,
                list(zip(u, v, strict=True)),
                role_name,
                [crease(p.co, nn) for p, nn in zip(quad, normals, strict=True)],
            )
    for end in (0, 1):
        if not caps[end]:
            continue
        ring = rings[0 if end == 0 else -1]
        points = [v.co.copy() for v in ring]
        flat = [(p.y, p.z) if axis == "x" else (p.x, p.y) for p in points]
        up = Vector((1.0, 0.0, 0.0)) if axis == "x" else Vector((0.0, 0.0, 1.0))
        order = list(range(segments)) if end == 1 else list(reversed(range(segments)))
        builder.face(
            [points[i] for i in order],
            [flat[i] for i in order],
            role_name,
            [crease(points[i], up if end == 1 else -up) for i in order],
            smooth=False,
        )


def club():
    """A wooden club lying along x, grip to the left and its heavy head to the right, a pommel at its far end;
    0.70 m long. Its grip and head are its joints."""
    b = Builder("club")
    profile = [
        (0.000, 0.0200), (0.030, 0.0225), (0.060, 0.0205), (0.100, 0.0170), (0.150, 0.0160), (0.220, 0.0165),
        (0.310, 0.0175), (0.380, 0.0205), (0.440, 0.0270), (0.500, 0.0340), (0.560, 0.0400), (0.620, 0.0435),
        (0.670, 0.0440), (0.700, 0.0420),
    ]  # fmt: skip
    lathe(b, "x", profile, 16, "wood", lambda p, n: 0.62 + 0.38 * (0.5 + 0.5 * n.z), bumps=0.03, seed=3)
    obj = b.build((0.0, 0.0, 0.0))
    joint(obj, "grip", (0.22, 0.0, 0.0))
    joint(obj, "head", (0.60, 0.0, 0.0))
    return obj


def pole():
    """A birch pole standing along z, its foot at the origin, 3.75 m long, thinner toward its tip: joints at its foot,
    where it crosses the others, 3.27 m up, and at its tip."""
    b = Builder("pole")
    profile = [(z * 0.25, 0.0350 - 0.0150 * (z * 0.25) / 3.75) for z in range(16)]
    lathe(b, "z", profile, 12, "wood", lambda p, n: 0.70 + 0.30 * min(1.0, p.z / 0.8), bumps=0.025, seed=5)
    obj = b.build((0.0, 0.0, 0.0))
    joint(obj, "foot", (0.0, 0.0, 0.0))
    joint(obj, "bind", (0.0, 0.0, 3.27))
    joint(obj, "tip", (0.0, 0.0, 3.75))
    return obj


COVER_BASE = 1.9  # radius at the ground
COVER_HIGH = 2.6
DOOR_AT = -math.pi / 2.0  # south
DOOR_HALF = math.radians(13.0)
DOOR_HIGH = 1.25
THICK = 0.02


def cover():
    """The tent's hide cover: a cone 3.8 m across and 2.6 m high, open at its hem and at its door, an inner skin
    behind its outer, unrolled flat for its texture coordinates, which is exact for a cone. Joints: its apex, and the
    top of its door, turned to lie along the cone's slope."""
    b = Builder("cover")
    slant = math.hypot(COVER_BASE, COVER_HIGH)
    sector = 2.0 * math.pi * COVER_BASE / slant  # the angle of the unrolled cone
    rings, segments = 12, 48

    def outward(theta):
        return Vector((COVER_HIGH * math.cos(theta), COVER_HIGH * math.sin(theta), COVER_BASE)) / slant

    def point(k, j):
        s = slant * k / rings
        theta = 2.0 * math.pi * (j % segments) / segments
        r, z = COVER_BASE * s / slant, COVER_HIGH * (1.0 - s / slant)
        return Vector((r * math.cos(theta), r * math.sin(theta), z)), theta

    def development(k, j):
        s = slant * k / rings
        alpha = sector * j / segments
        return (s * math.cos(alpha), s * math.sin(alpha))

    def in_door(k, j):
        centre = 2.0 * math.pi * (j + 0.5) / segments
        gap = abs((centre - DOOR_AT + math.pi) % (2.0 * math.pi) - math.pi)
        return gap < DOOR_HALF and point(k + 1, j)[0].z < DOOR_HIGH

    # one vertex for the apex and one for each point of each ring, shared by the faces round it, so the surface shades
    # smooth; the texture coordinates are each corner's own, so the seam still unrolls
    grids = {}
    for skin, depth in (("outer", 0.0), ("inner", THICK)):
        for k in range(rings + 1):
            for j in range(segments):
                if (k, j) in grids.get(skin, {}) or (k == 0 and j > 0):
                    continue
                p, theta = point(k, j)
                grids.setdefault(skin, {})[(k, j)] = b.bm.verts.new(p - depth * outward(theta))
    for k in range(rings):
        for j in range(segments):
            if k > 0 and in_door(k, j):
                continue
            corners = [(k, j), (k + 1, j), (k + 1, j + 1), (k, j + 1)]
            if k == 0:
                corners = corners[:3]  # at the apex the four corners are three
            uvs = [development(kk, jj) for kk, jj in corners]
            outer = [grids["outer"][(kk, jj % segments if kk > 0 else 0)] for kk, jj in corners]
            inner = [grids["inner"][(kk, jj % segments if kk > 0 else 0)] for kk, jj in corners]
            b.face(outer, uvs, "hide", [0.62 + 0.38 * min(1.0, point(kk, jj)[0].z / 0.9) for kk, jj in corners])
            b.face(list(reversed(inner)), list(reversed(uvs)), "hide", [0.30] * len(corners))
    obj = b.build((0.0, 0.0, 0.0))
    joint(obj, "apex", (0.0, 0.0, COVER_HIGH))
    reach = COVER_BASE * (1.0 - DOOR_HIGH / COVER_HIGH)
    slope = math.atan2(COVER_BASE, COVER_HIGH)
    joint(obj, "door_top", (0.0, -reach, DOOR_HIGH), (-slope, 0.0, 0.0))
    return obj


def flap():
    """The door flap: a thin hide hanging 1.25 m from its hinge, at the middle of its top, 0.8 m wide, flat in the
    x z plane; its hinge joint is the origin."""
    b = Builder("door_flap")
    w, h, t = 0.4, DOOR_HIGH, 0.01
    front = [Vector((-w, -t, -h)), Vector((w, -t, -h)), Vector((w, -t, 0.0)), Vector((-w, -t, 0.0))]
    back = [Vector((-w, t, -h)), Vector((-w, t, 0.0)), Vector((w, t, 0.0)), Vector((w, t, -h))]
    uvs = [(-w, -h), (w, -h), (w, 0.0), (-w, 0.0)]
    crease = [0.6, 0.6, 0.9, 0.9]
    b.face(front, uvs, "hide", crease, smooth=False)
    b.face(back, [uvs[0], uvs[3], uvs[2], uvs[1]], "hide", [crease[0], crease[3], crease[2], crease[1]], smooth=False)
    obj = b.build((0.0, 0.0, 0.0))
    joint(obj, "hinge", (0.0, 0.0, 0.0))
    return obj


def stone(name, seed, half_x, half_y, high):
    """A river stone half buried, a lump of 80 flat faces whose bottom lies on z = 0, each face with its own texture
    projection along its own normal, which carries its texture pixels over without stretch."""
    b = Builder(name)
    bmesh.ops.create_icosphere(b.bm, subdivisions=2, radius=1.0)
    for i, v in enumerate(b.bm.verts):
        lump = 1.0 + 0.16 * noise(seed, i)
        v.co = Vector((v.co.x * half_x * lump, v.co.y * half_y * lump, max(0.0, v.co.z) * high * lump))
    b.bm.normal_update()
    # the faces' own texture projections and crease, with the faces' roles
    for f in b.bm.faces:
        f.smooth = False
        f.material_index = b.slot("stone")
        n = f.normal.copy()
        n.normalize()
        ref = Vector((0.0, 0.0, 1.0)) if abs(n.z) < 0.95 else Vector((1.0, 0.0, 0.0))
        t = ref.cross(n).normalized()
        w = n.cross(t)
        for loop in f.loops:
            p = loop.vert.co
            loop[b.uv].uv = (p.dot(t), p.dot(w))
            c = 0.55 + 0.45 * min(1.0, max(0.0, p.z / (high * 0.8)))
            loop[b.crease] = (c, c, c, 1.0)
    obj = b.build((0.0, 0.0, 0.0))
    return obj


def binding():
    """The lashing round the poles where they cross: a short thick band of rawhide, its joint at its foot."""
    b = Builder("binding")
    lathe(b, "z", [(0.0, 0.088), (0.065, 0.088), (0.13, 0.088)], 12, "hide", lambda p, n: 0.8, bumps=0.02, seed=9)
    obj = b.build((0.0, 0.0, 0.0))
    joint(obj, "centre", (0.0, 0.0, 0.0))
    return obj


def make_family():
    """Every stand-in part, laid in a row in the file, which the exporter does not care about."""
    parts = [
        binding(),
        club(),
        cover(),
        flap(),
        pole(),
        stone("stone_a", 1, 0.22, 0.17, 0.12),
        stone("stone_b", 2, 0.16, 0.14, 0.10),
        stone("stone_c", 3, 0.26, 0.20, 0.14),
    ]
    x = 0.0
    for obj in parts:
        obj.location = (x, 0.0, 0.0)
        x += 5.0
    return parts


def main():
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    if not 1 <= len(argv) <= 2:
        raise SystemExit("usage: blender --background --python tools/blender/standins.py -- <out.kdkit> [<out.blend>]")
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj)
    make_family()
    if len(argv) == 2:
        bpy.ops.wm.save_as_mainfile(filepath=argv[1])
    try:
        export.export(argv[0])
    except export.KitError as e:
        print(f"KDKIT: {e}", file=sys.stderr)
        raise SystemExit(1) from None


if __name__ == "__main__":
    main()
