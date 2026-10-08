"""The helpers the art lane's Blender part scripts build with (A6.1, A6.4), written once: a builder of faces that keeps
each one's texture coordinates in metres, its material slot named by role and its crease darkening, the joints as
empties, the lathe of rings round an axis, and the texture pixel's size (a whole number of them to every wrap). Imported
by tools/art/parts_camp.py and tools/art/parts_tent.py, which make the parts and export them as any family is
(tools/blender/export.py). The file made the pilot's stand-in family of parts before the art lane's own arrived; the
stand-ins are gone and the helpers stay under their old name. Implements PRE-46.
"""

import math

import bmesh
import bpy
from mathutils import Euler, Vector

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
