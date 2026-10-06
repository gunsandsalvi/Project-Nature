"""Building the kit's parts in Blender (A6.1, A6.4): the functions a family's script calls, such as art/models/camp.py
and GPT's first pass at it, so the rules every part keeps are written once.

Run inside Blender 4.0 (bpy, bmesh and mathutils; its Python has no numpy):

    import kit
    kit.start()
    p = kit.Part("pole_straight_200", group="wood", about="a straight pole 2 m long")
    p.sleeve([(0, 0, 0), (0, 0, 2.0)], [0.05, 0.035], sides=8, role="bark", cap_role="wood")
    p.joint("base")
    p.joint("top", at=(0, 0, 2.0))
    p.done()
    kit.finish("camp.blend")

A part is built of pieces: sleeve() for anything round (a pole, a branch, a limb, a tine), lathe() for a basket's
walls, panel() for a cut shape (a hide, a bark sheet), card() for a cut-out card (a grass tuft, a leaf cluster),
hull() and mesh() for a stone's own faces with project() to lay their texture. A figure's parts share an Atlas,
whose pieces never overlap; where a sleeve cannot lay its texture within the line, charts() and
chart_overstretched() lay it by the way each face looks; paint() gives faces a role by where they are.

The rules every part keeps, which these functions keep for it:
- real size in metres, z up, its front toward -y;
- its origin at its main joint (the first joint made, at (0, 0, 0)), and every joint an empty parented to the part,
  named joint_<name> (Blender adds .001 to a name used before; the empty's "joint" property keeps the bare name), its
  z axis pointing the way a part plugged there points;
- texture coordinates in metres in the UV map "UVMap" (one texture pixel 1/64 m at band 0), stretched at most 1.5:1
  (kitmath.stretch); a wrapped sleeve's or lathe's circumference spans whole texture pixels, so its wrap never shows;
- material slots named by role (ROLES), never by a particular material, which recipes choose;
- for a figure, parts skinned to one armature, with vertex groups named after its bones and shape keys named as the
  family's script sets them (build, age and sex).

Implements PRE-46 and PRE-22, see A6.1 and A6.4.
"""

import math
import random

import bmesh
import bpy
import kitmath
from mathutils import Vector
from mathutils.geometry import delaunay_2d_cdt

TEXELS_A_METRE = kitmath.TEXELS_A_METRE
AUTO_TAPER = 1.3  # breaks="auto": the most the rings of one wrap may differ in circumference
# Each role's colour in Blender's viewport only, so an opened file shows its parts' roles apart; the game's materials
# come from recipes and textures, never from these.
ROLES = {
    "wood": (0.66, 0.52, 0.34),
    "bark": (0.38, 0.29, 0.21),
    "hide": (0.68, 0.53, 0.37),
    "stone": (0.56, 0.55, 0.53),
    "leaf": (0.34, 0.46, 0.20),
    "grass": (0.55, 0.57, 0.29),
    "skin": (0.76, 0.56, 0.43),
    "hair": (0.30, 0.22, 0.16),
    "weave": (0.62, 0.51, 0.31),
    "fur": (0.82, 0.76, 0.64),
    "antler": (0.72, 0.64, 0.51),
    "hoof": (0.21, 0.19, 0.17),
    "cord": (0.56, 0.46, 0.31),
}
UV_NAME = "UVMap"


def start():
    """An empty file in metres, and no .blend1 copy left beside it when it is saved."""
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.preferences.filepaths.save_version = 0
    units = bpy.context.scene.unit_settings
    units.system = "METRIC"
    units.scale_length = 1.0
    units.length_unit = "METERS"


def role(name):
    """The material for a role, made once a file, named by the role."""
    if name not in ROLES:
        raise ValueError(f"{name!r} is not a role; the roles are {', '.join(ROLES)}")
    m = bpy.data.materials.get(name)
    if m is None:
        m = bpy.data.materials.new(name)
        m.diffuse_color = (*ROLES[name], 1.0)
    return m


def collection(name):
    """The file's collection of that name, made under the scene's own when first asked for."""
    c = bpy.data.collections.get(name)
    if c is None:
        c = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(c)
    return c


def _v(p):
    return Vector((float(p[0]), float(p[1]), float(p[2])))


def frames(points, front=(0.0, -1.0, 0.0)):
    """A frame (tangent, side, back) at each point of a path, turning as little as the path does (each frame's
    side axis carried from the last and laid across the new tangent), the first frame's back axis pointing away
    from `front`. Around a sleeve, angle 0 is the side axis and 90 degrees the back."""
    pts = [_v(p) for p in points]
    if len(pts) < 2:
        raise ValueError("a path needs two points or more")
    tangents = []
    for i in range(len(pts)):
        a, b = pts[max(0, i - 1)], pts[min(len(pts) - 1, i + 1)]
        t = b - a
        if t.length < 1e-9:
            raise ValueError(f"the path's points {max(0, i - 1)} and {min(len(pts) - 1, i + 1)} coincide")
        tangents.append(t.normalized())
    f = _v(front)
    back = -(f - f.dot(tangents[0]) * tangents[0])
    if back.length < 1e-6:  # a path running along `front`: any square axis serves
        back = tangents[0].orthogonal()
    back.normalize()
    out = []
    for t in tangents:
        back = back - back.dot(t) * t
        if back.length < 1e-9:
            back = t.orthogonal()
        back.normalize()
        side = back.cross(t)
        out.append((t, side, back))
    return out


def _ring(point, frame, radius, sides, seam):
    """The vertices of one ring: a circle (radius r), an ellipse ((across the side axis, across the back axis)), or
    one distance for each vertex, starting at the seam's angle and turning from the side axis toward the back."""
    t, side, back = frame
    p = _v(point)
    out = []
    for k in range(sides):
        a = seam + 2 * math.pi * k / sides
        if isinstance(radius, (int, float)):
            rx = ry = float(radius)
        elif len(radius) == 2:
            rx, ry = float(radius[0]), float(radius[1])
        else:
            rx = ry = float(radius[k])
        out.append(p + side * (rx * math.cos(a)) + back * (ry * math.sin(a)))
    return out


def _perimeter(ring):
    """Distances round a ring from its first vertex to each vertex, and back to the first: sides + 1 numbers."""
    out = [0.0]
    for k in range(len(ring)):
        out.append(out[-1] + (ring[(k + 1) % len(ring)] - ring[k]).length)
    return out


Atlas = kitmath.Atlas  # a texture drawn to a layout, its pieces placed so none overlaps another (in kitmath)


class Part:
    """One part being built: its geometry, texture coordinates, roles, joints, weights and shape keys, made into a
    Blender object by done().

    Every builder below adds to the part. Faces carry a role each; texture coordinates are in metres; a builder's
    `uv_at` moves its texture coordinates. A part given an atlas (a figure's, whose texture is drawn to its layout)
    has each builder place its piece of texture in the atlas instead, never overlapping another."""

    def __init__(self, name, group="parts", about="", armature=None, atlas=None):
        self.name = name
        self.group = group
        self.about = about
        self.armature = armature
        self.atlas = atlas
        self.bm = bmesh.new()
        self.uv = self.bm.loops.layers.uv.new(UV_NAME)
        self.roles = []
        self.joints = []  # (name, at, towards)
        self.weights = {}  # vertex index: {bone: weight}
        self.keys = {}  # key name: {vertex index: position}
        self.wraps = []  # texture pixels round each wrapped sleeve or lathe
        self.smooth = 50.0  # degrees: edges sharper than this stay hard

    # the pieces every builder uses

    def _role(self, name):
        role(name)  # refuses an unknown role
        if name not in self.roles:
            self.roles.append(name)
        return self.roles.index(name)

    def _vert(self, co, weights=None, keys=None):
        v = self.bm.verts.new(_v(co))
        v.index = len(self.bm.verts) - 1
        if weights:
            self.weights[v.index] = dict(weights)
        for name, at in (keys or {}).items():
            self.keys.setdefault(name, {})[v.index] = _v(at)
        return v

    def _face(self, verts, uvs, role_name, smooth=True):
        f = self.bm.faces.new(verts)
        f.material_index = self._role(role_name)
        f.smooth = smooth
        for loop, uv in zip(f.loops, uvs, strict=True):
            loop[self.uv].uv = (float(uv[0]), float(uv[1]))
        return f

    # builders

    def sleeve(
        self,
        points,
        radii,
        sides=8,
        role="bark",
        cap_role=None,
        caps=(True, True),
        wrap=True,
        seam=0.0,
        front=(0.0, -1.0, 0.0),
        uv_at=(0.0, 0.0),
        cap_uv_at=None,
        weights=None,
        keys=None,
        seed=1,
        breaks=(),
        along_frames=None,
    ):
        """A tube along a path, such as a pole, branch, trunk segment, limb or antler tine.

        points: the path in metres, two points or more; radii: one for each point, a number (a circle), a pair (an
        ellipse: across the side axis, across the back axis) or `sides` numbers (any shape); sides: vertices round.
        wrap: round the tube the texture spans the circumference rounded to whole texture pixels (the mean of its
        rings'), the same at every ring, so a tiled bark or wood wraps without a seam; with wrap False each ring
        spans its own true circumference, centred on the column opposite the seam, for a figure's texture drawn to
        its layout. Along the tube the texture runs at the surface's true length from ring to ring (the mean of its
        columns), so a ledge where the tube narrows fast is laid at its own width.
        breaks: ring indices where a wrapped tube's texture starts a new wrap, its span the whole texture pixels of
        that stretch's mean circumference, so a long taper stays within the line in one unbroken mesh (the texture
        steps there, as where a branch narrows); "auto" starts a new wrap wherever the rings of the one before would
        differ more than 1.3 times in circumference, so no wrap squeezes its rings by more than about 1.14, before
        the rounding to whole texture pixels, which counts most on a thin branch.
        seam: the angle of the wrap's seam, from the side axis toward the back (frames()); front: the way the first
        frame's back axis points away from.
        caps: how the first and the last rings end, each one of:
        - None or False: open, as where a limb meets the next part;
        - "flat" or True: a flat disc, for an end never seen cut (a basket's, a figure's), its texture its own
          plane's in metres from cap_uv_at (uv_at if None);
        - "chopped": a short faceted point, as a stone axe leaves a pole (never a sawn face);
        - "broken": a jagged break, as dead wood snaps, its ragged edge varied by `seed`;
        the cut wood takes cap_role (role if None), each facet's texture laid flat on it at true size.
        weights: for a figure, one {bone: weight} for each point; keys: {shape key: {"points": ..., "radii": ...}}
        giving the path or the radii that key moves to (either may be left out), keeping every texture coordinate.
        along_frames: one frame for each point (as frames() makes them), to use instead of the path's own; for
        a part that is a stretch of a longer chain, such as a forearm of an arm, give it the frames of the whole
        chain's path, so the ring it shares with the next part is turned alike in both and they meet exactly; a key
        may give its own "frames" the same way.
        Returns the texture pixels round the tube when it wraps (a list, one for each stretch, when it has breaks),
        else None."""
        n = len(points)
        if len(radii) != n:
            raise ValueError(f"{n} points but {len(radii)} radii")
        if sides < 3:
            raise ValueError("a sleeve needs three sides or more")
        auto = breaks == "auto"
        if not auto and any(not 0 < b < n - 1 for b in breaks):
            raise ValueError(f"breaks must lie between the first ring and the last (1 to {n - 2})")
        fr = along_frames if along_frames is not None else frames(points, front)
        if len(fr) != n:
            raise ValueError(f"{n} points but {len(fr)} frames")
        rings = [_ring(points[i], fr[i], radii[i], sides, seam) for i in range(n)]
        key_rings = {}
        for name, k in (keys or {}).items():
            kp = k.get("points", points)
            kr = k.get("radii", radii)
            kf = k.get("frames") or frames(kp, front)
            key_rings[name] = [_ring(kp[i], kf[i], kr[i], sides, seam) for i in range(n)]
        perims = [_perimeter(r) for r in rings]
        along = [0.0]  # the surface's own length from ring to ring, the mean of its columns, so a ledge is laid true
        for i in range(1, n):
            along.append(along[-1] + sum((rings[i][k] - rings[i - 1][k]).length for k in range(sides)) / sides)
        if auto:
            breaks, start = [], 0
            for i in range(1, n):
                window = [perims[j][-1] for j in range(start, i + 1)]
                if max(window) > AUTO_TAPER * min(window) and i - 1 > start:
                    breaks.append(i - 1)
                    start = i - 1
        bounds = [0, *sorted(set(breaks)), n - 1]
        stretches = list(zip(bounds, bounds[1:], strict=False))
        texels = []
        if wrap:
            for a, b in stretches:
                texels.append(kitmath.whole_texels(sum(perims[i][-1] for i in range(a, b + 1)) / (b - a + 1)))
            self.wraps += texels
        spans = [t / TEXELS_A_METRE for t in texels]
        row_stretch = [next(s for s, (a, b) in enumerate(stretches) if a <= i < b) for i in range(n - 1)]
        if self.atlas is not None:
            if any(c not in (None, False, True, "flat") for c in caps):
                raise ValueError(f"{self.name}: a part drawn to an atlas ends open or flat")
            w = max(spans) if wrap else max(p[-1] for p in perims)
            corner = self.atlas.place(w, along[-1])
            uv_at = (corner[0] + (0.0 if wrap else w / 2), corner[1])
        verts = []
        for i in range(n):
            w = weights[i] if weights else None
            row = []
            for k in range(sides):
                kk = {name: kr[i][k] for name, kr in key_rings.items()}
                row.append(self._vert(rings[i][k], w, kk))
            verts.append(row)

        def u_of(i, k, s):
            if wrap:
                return spans[s] * perims[i][k] / perims[i][-1]
            return perims[i][k] - perims[i][-1] / 2

        for i in range(n - 1):
            s = row_stretch[i]
            for k in range(sides):
                k2 = k + 1
                quad = [verts[i][k], verts[i][k2 % sides], verts[i + 1][k2 % sides], verts[i + 1][k]]
                uvs = [
                    (uv_at[0] + u_of(i, k, s), uv_at[1] + along[i]),
                    (uv_at[0] + u_of(i, k2, s), uv_at[1] + along[i]),
                    (uv_at[0] + u_of(i + 1, k2, s), uv_at[1] + along[i + 1]),
                    (uv_at[0] + u_of(i + 1, k, s), uv_at[1] + along[i + 1]),
                ]
                self._face(quad, uvs, role)
        cap_uv = cap_uv_at if cap_uv_at is not None else uv_at
        for end, kind in ((0, caps[0]), (n - 1, caps[1])):
            kind = "flat" if kind is True else kind
            if not kind:
                continue
            if kind != "flat" and keys:
                raise ValueError("a chopped or broken end cannot follow shape keys")
            first = end == 0
            ring = verts[end]
            ends = {
                "ring": ring,
                "centre": _v(points[end]),
                "frame": fr[end],
                "out": -fr[end][0] if first else fr[end][0],
                "first": first,
                "weights": weights[end] if weights else None,
                "u": [uv_at[0] + u_of(end, k, 0 if first else len(stretches) - 1) for k in range(sides + 1)],
                "v": uv_at[1] + along[end],
                "radius": sum((v.co - _v(points[end])).length for v in ring) / sides,
            }
            if kind == "flat":
                if self.atlas is not None:
                    r = max((v.co - ends["centre"]).length for v in ring)
                    corner = self.atlas.place(2 * r, 2 * r)
                    cap_uv = (corner[0] + r, corner[1] + r)
                self._flat_end(ends, cap_role or role, cap_uv)
            elif kind == "chopped":
                self._chopped_end(ends, cap_role or role)
            elif kind == "broken":
                self._broken_end(ends, role, cap_role or role, 2 * seed + (0 if first else 1))
            else:
                raise ValueError(f"an end is flat, chopped or broken, not {kind!r}")
        if not wrap:
            return None
        return texels[0] if len(texels) == 1 else texels

    def _ordered(self, ring, first):
        """A ring's vertex order seen from outside its end: as made at the last ring, reversed at the first."""
        order = list(range(len(ring)))
        return order[::-1] if first else order

    def _flat_end(self, e, role_name, cap_uv):
        """A flat disc, for ends never seen cut, such as a basket's or a limb's: seen from outside, u along the side
        axis and v along the back axis, or against it at the first end, so it is never mirrored."""
        _, side, back = e["frame"]
        vsign = -1.0 if e["first"] else 1.0
        c = e["centre"]
        uvs = [(cap_uv[0] + (v.co - c).dot(side), cap_uv[1] + vsign * (v.co - c).dot(back)) for v in e["ring"]]
        order = self._ordered(e["ring"], e["first"])
        self._face([e["ring"][k] for k in order], [uvs[k] for k in order], role_name, smooth=False)

    def _facet(self, verts, role_name):
        """A flat facet whose texture is laid on its own plane at true size (stretch 1), as an axe's cut or a break."""
        a, b, c = (v.co for v in verts[:3])
        n = (b - a).cross(c - a)
        d = tuple(n.normalized()) if n.length > 1e-12 else (0.0, 0.0, 1.0)
        return self._face(verts, [kitmath.project(tuple(v.co), d) for v in verts], role_name, smooth=False)

    def _chopped_end(self, e, role_name):
        """A short faceted point, as a stone axe leaves wood, never a sawn face: the end ring drawn in to a third of
        its size over 1.2 radii, one flat facet a side, and a small flat tip."""
        ring, c, out, r = e["ring"], e["centre"], e["out"], e["radius"]
        tip = [self._vert(c + out * (1.2 * r) + (v.co - c) * 0.3, e["weights"]) for v in ring]
        n = len(ring)
        for k in range(n):
            k2 = (k + 1) % n
            quad = [ring[k], ring[k2], tip[k2], tip[k]]
            self._facet(quad if not e["first"] else quad[::-1], role_name)
        order = self._ordered(tip, e["first"])
        self._facet([tip[k] for k in order], role_name)

    def _broken_end(self, e, side_role, role_name, seed):
        """A jagged break, as dead wood snaps: the bark runs on to a ragged edge, each column its own length (up to
        0.9 radii), and the broken wood closes it in flat facets round a point."""
        rnd = random.Random(seed)
        ring, c, out, r = e["ring"], e["centre"], e["out"], e["radius"]
        n = len(ring)
        reach = [rnd.uniform(0.15, 0.9) * r for _ in range(n)]
        jag = [self._vert(ring[k].co + out * reach[k], e["weights"]) for k in range(n)]
        sign = -1.0 if e["first"] else 1.0
        for k in range(n):
            k2 = k + 1
            quad = [ring[k], ring[k2 % n], jag[k2 % n], jag[k]]
            uvs = [
                (e["u"][k], e["v"]),
                (e["u"][k2], e["v"]),
                (e["u"][k2], e["v"] + sign * reach[k2 % n]),
                (e["u"][k], e["v"] + sign * reach[k]),
            ]
            if e["first"]:
                quad, uvs = quad[::-1], uvs[::-1]
            self._face(quad, uvs, side_role)
        point = self._vert(c + out * (rnd.uniform(0.3, 0.6) * r), e["weights"])
        for k in range(n):
            tri = [jag[k], jag[(k + 1) % n], point]
            self._facet(tri if not e["first"] else tri[::-1], role_name)

    def lathe(self, profile, sides=12, role="weave", wrap=True, seam=0.0, uv_at=(0.0, 0.0), caps=(False, False)):
        """A surface turned round the z axis, such as a basket's walls and rim: profile is (radius, height) from the
        first ring to the last, every radius above 0. Round the axis the texture wraps as a sleeve's does (wrap
        True: whole texture pixels of the mean circumference at every ring); along the profile it runs at its true
        length. The surface faces the way the profile turns from: up the outside of a wall it faces out, down the
        inside in. caps closes the first ring with a flat disc facing down and the last with one facing up, each
        with its plane's texture seen from outside."""
        if any(r <= 0 for r, _ in profile):
            raise ValueError("every ring of a lathe needs a radius above 0; close an end with caps")
        if self.atlas is not None:
            raise ValueError(f"{self.name}: a lathe wraps a tiled texture, never an atlas")
        rings = [
            [
                Vector(
                    (
                        r * math.cos(seam + 2 * math.pi * k / sides),
                        r * math.sin(seam + 2 * math.pi * k / sides),
                        z,
                    )
                )
                for k in range(sides)
            ]
            for r, z in profile
        ]
        perims = [_perimeter(r) for r in rings]
        along = [0.0]
        for i in range(1, len(profile)):
            (r0, z0), (r1, z1) = profile[i - 1], profile[i]
            along.append(along[-1] + math.hypot(r1 - r0, z1 - z0))
        texels = kitmath.whole_texels(sum(p[-1] for p in perims) / len(perims)) if wrap else None
        if texels:
            self.wraps.append(texels)
        verts = [[self._vert(co) for co in ring] for ring in rings]
        for i in range(len(profile) - 1):
            for k in range(sides):
                k2 = k + 1
                quad = [verts[i][k], verts[i][k2 % sides], verts[i + 1][k2 % sides], verts[i + 1][k]]

                def u(ii, kk):
                    if texels:
                        return texels / TEXELS_A_METRE * perims[ii][kk] / perims[ii][-1]
                    return perims[ii][kk] - perims[ii][-1] / 2

                uvs = [
                    (uv_at[0] + u(i, k), uv_at[1] + along[i]),
                    (uv_at[0] + u(i, k2), uv_at[1] + along[i]),
                    (uv_at[0] + u(i + 1, k2), uv_at[1] + along[i + 1]),
                    (uv_at[0] + u(i + 1, k), uv_at[1] + along[i + 1]),
                ]
                self._face(quad, uvs, role)
        for end, on in ((0, caps[0]), (len(profile) - 1, caps[1])):
            if not on:
                continue
            ring = verts[end]
            # a disc carries the surface on to the axis: the first ring's faces down, as under a basket, and the
            # last ring's up, as the floor inside one; seen from outside its texture is its plane's, never mirrored
            up = end > 0
            order = list(range(sides)) if up else list(range(sides))[::-1]
            vs = 1.0 if up else -1.0
            uvs = [(uv_at[0] + v.co.x, uv_at[1] + vs * v.co.y) for v in ring]
            self._face([ring[k] for k in order], [uvs[k] for k in order], role, smooth=False)
        return texels

    def panel(self, outline, role="hide", spacing=0.1, bend=None, uv_at=(0.0, 0.0), holes=()):
        """A flat cut shape, such as a hide panel or a bark sheet: outline is its edge as (x, y) points in metres,
        either way round; it lies in the x-z plane facing -y, x across and y up, unless bend maps each (x, y) to
        where that point of the shape lies in space (a drape or a curl, which should keep lengths). Its texture is
        its flat cut shape in metres, so a drape stretches it only as much as the bend stretches the shape.
        spacing: the most a triangle's side may be, so a bend has points to bend at; holes: outlines cut out."""
        loops = [list(outline)] + [list(h) for h in holes]
        pts, edges = [], []
        for loop in loops:
            dense = _densify(loop, spacing)
            start = len(pts)
            pts += dense
            edges += [(start + i, start + (i + 1) % len(dense)) for i in range(len(dense))]
        # points inside, on a grid of the spacing, kept off the edges
        xs = [p[0] for p in outline]
        ys = [p[1] for p in outline]
        step = spacing * 0.9
        y = min(ys) + step / 2
        row = 0
        while y < max(ys):
            x = min(xs) + step / 2 + (step / 2 if row % 2 else 0.0)
            while x < max(xs):
                q = Vector((x, y))
                if _inside(q, loops[0]) and not any(_inside(q, h) for h in loops[1:]):
                    if all(_seg_distance(q, pts[a], pts[b]) > step * 0.45 for a, b in edges):
                        pts.append(q)
                x += step
            y += step * 0.87
            row += 1
        faces = [list(range(len(_densify(loops[0], spacing))))]  # the outline: the first points made
        if self.atlas is not None:
            corner = self.atlas.place(max(xs) - min(xs), max(ys) - min(ys))
            uv_at = (corner[0] - min(xs), corner[1] - min(ys))
        out = delaunay_2d_cdt(pts, edges, faces, 1, 1e-7, True)
        coords, _, tris = out[0], out[1], out[2]
        verts = []
        for c in coords:
            x, yy = c[0], c[1]
            at = bend(x, yy) if bend else (x, 0.0, yy)
            verts.append(self._vert(at))
        for tri in tris:
            if any(_inside(sum((coords[i] for i in tri), Vector((0, 0))) / 3, h) for h in loops[1:]):
                continue
            a, b, c = (coords[i] for i in tri)
            if (b - a).cross(c - a) < 0:
                tri = tri[::-1]
            self._face([verts[i] for i in tri], [(uv_at[0] + coords[i][0], uv_at[1] + coords[i][1]) for i in tri], role)

    def card(self, width, height, role="grass", at=(0.0, 0.0, 0.0), yaw=0.0, lean=0.0, uv_at=(0.0, 0.0), rows=1):
        """A cut-out card, such as a grass tuft's or a leaf cluster's: a quad width across and height tall, its
        bottom edge's middle at `at`, turned yaw radians about z (facing -y at 0) and leaning lean radians back from
        upright about its bottom edge; its texture is its own flat shape in metres from uv_at, which its cut-out design
        fills. rows splits it into strips across, so it can bend later."""
        if self.atlas is not None:
            uv_at = self.atlas.place(width, height)
        c, s = math.cos(yaw), math.sin(yaw)
        across = Vector((c, s, 0.0))
        up = Vector((-s * math.sin(lean), c * math.sin(lean), math.cos(lean)))
        base = _v(at)
        grid = []
        for j in range(rows + 1):
            h = height * j / rows
            grid.append([self._vert(base + across * (x - width / 2) + up * h) for x in (0.0, width)])
        for j in range(rows):
            h0, h1 = height * j / rows, height * (j + 1) / rows
            quad = [grid[j][0], grid[j][1], grid[j + 1][1], grid[j + 1][0]]
            uvs = [
                (uv_at[0], uv_at[1] + h0),
                (uv_at[0] + width, uv_at[1] + h0),
                (uv_at[0] + width, uv_at[1] + h1),
                (uv_at[0], uv_at[1] + h1),
            ]
            self._face(quad, uvs, role)

    def mesh(self, points, faces, role="stone", smooth=True, weights=None):
        """Faces of the script's own making, such as a stone's: points in metres, faces as lists of point indices
        counter-clockwise seen from outside. Their texture coordinates are left for project() to lay."""
        verts = [self._vert(p, weights[i] if weights else None) for i, p in enumerate(points)]
        for f in faces:
            self._face([verts[i] for i in f], [(0.0, 0.0)] * len(f), role, smooth)
        return verts

    def hull(self, points, role="stone", smooth=True, rounds=0, flat_base=None):
        """The convex hull of points, as a stone's faces: angular as it is, or rounded by `rounds` of subdivision
        that moves each new point out toward a smooth surface; flat_base, a height in metres, flattens everything
        below it onto it, so the stone rests. Texture coordinates are left for project() to lay."""
        bm = bmesh.new()
        for p in points:
            bm.verts.new(_v(p))
        bmesh.ops.convex_hull(bm, input=bm.verts, use_existing_faces=False)
        for v in [v for v in bm.verts if not v.link_faces]:
            bm.verts.remove(v)
        for _ in range(rounds):
            bmesh.ops.subdivide_edges(bm, edges=bm.edges[:], cuts=1, use_grid_fill=True, smooth=1.0)
        bmesh.ops.triangulate(bm, faces=bm.faces[:])
        if flat_base is not None:
            for v in bm.verts:
                if v.co.z < flat_base:
                    v.co.z = flat_base
        bmesh.ops.remove_doubles(bm, verts=bm.verts[:], dist=1e-4)
        bmesh.ops.dissolve_degenerate(bm, edges=bm.edges[:], dist=1e-5)
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
        bm.verts.index_update()
        pts = [v.co.copy() for v in bm.verts]
        fs = [[v.index for v in f.verts] for f in bm.faces]
        bm.free()
        return self.mesh(pts, fs, role, smooth)

    def project(self, role="stone"):
        """Texture coordinates for a role's faces, each from the direction it faces (the nearest of kitmath's 26),
        with height as its vertical: the rule for stones and rock (A6.4). Faces sharing a direction share one
        projection, so the texture runs on across them; where directions change it need not line up."""
        if self.atlas is not None:
            raise ValueError(f"{self.name}: projected faces share the world's texture, never an atlas")
        idx = self._role(role)
        self.bm.normal_update()
        for f in self.bm.faces:
            if f.material_index != idx:
                continue
            d = kitmath.DIRECTIONS[kitmath.nearest_direction(tuple(f.normal))]
            for loop in f.loops:
                loop[self.uv].uv = kitmath.project(tuple(loop.vert.co), d)

    def charts(self, test=None, faces=None):
        """Lays the texture of the faces whose centres pass test(centre) (or the faces given) afresh, each from the
        direction it faces (the nearest of kitmath's 26, height as its vertical), every connected group of faces
        sharing a direction one piece of the part's atlas: for a place a sleeve cannot lay within the line, such as a
        foot where the leg turns forward or the crown of a head. Within 1.13:1 at rest; the texture breaks where the
        pieces meet."""
        if self.atlas is None:
            raise ValueError(f"{self.name}: charts are pieces of an atlas; give the part one")
        self.bm.normal_update()
        chosen = list(faces) if faces is not None else [f for f in self.bm.faces if test(f.calc_center_median())]
        direction = {f: kitmath.nearest_direction(tuple(f.normal)) for f in chosen}
        left = set(chosen)
        while left:
            seed = left.pop()
            group, todo = [seed], [seed]
            while todo:
                f = todo.pop()
                for e in f.edges:
                    for g in e.link_faces:
                        if g in left and direction[g] == direction[seed]:
                            left.remove(g)
                            group.append(g)
                            todo.append(g)
            d = kitmath.DIRECTIONS[direction[seed]]
            uvs = {loop: kitmath.project(tuple(loop.vert.co), d) for f in group for loop in f.loops}
            u0 = min(u for u, _ in uvs.values())
            v0 = min(v for _, v in uvs.values())
            u1 = max(u for u, _ in uvs.values())
            v1 = max(v for _, v in uvs.values())
            corner = self.atlas.place(u1 - u0, v1 - v0)
            for loop, (u, v) in uvs.items():
                loop[self.uv].uv = (corner[0] + u - u0, corner[1] + v - v0)

    def overstretched(self, line=1.4):
        """The faces whose texture is stretched past `line` at rest or with any shape key at 1, each quad measured
        both ways it can be cut into triangles."""
        self.bm.verts.index_update()
        found = []
        for f in self.bm.faces:
            uvs = [tuple(loop[self.uv].uv) for loop in f.loops]
            n = len(uvs)
            cuts = [(0, i, i + 1) for i in range(1, n - 1)]
            if n == 4:
                cuts += [(1, 2, 3), (1, 3, 0)]
            for moved in [{}] + list(self.keys.values()):
                pts = [tuple(moved.get(loop.vert.index, loop.vert.co)) for loop in f.loops]
                if any((kitmath.stretch(*(pts[i] for i in c), *(uvs[i] for i in c)) or 1.0) > line for c in cuts):
                    found.append(f)
                    break
        return found

    def chart_overstretched(self, line=1.4):
        """Charts (above) for every face a sleeve could not lay within `line`, found by measuring, never by guessing
        where they are: where a figure's path turns sharply, as at a deer's stifle. Returns how many faces it took."""
        bad = self.overstretched(line)
        if bad:
            self.charts(faces=bad)
        return len(bad)

    def paint(self, role_name, test):
        """Gives a role to the faces whose centres pass test(centre), a Vector in metres, such as hair above a
        hairline; each face keeps its texture coordinates, since a figure's roles share one layout."""
        idx = self._role(role_name)
        for f in self.bm.faces:
            if test(f.calc_center_median()):
                f.material_index = idx

    def weld(self, distance=1e-5):
        """Merges points that lie together, as where pieces of one part meet, so no crack shows and the light runs on
        across the join; every face keeps its own texture coordinates. Only for a part with no weights or shape keys,
        since merging renumbers its points."""
        if self.weights or self.keys:
            raise ValueError(f"{self.name}: a part with weights or shape keys cannot be welded")
        bmesh.ops.remove_doubles(self.bm, verts=self.bm.verts[:], dist=distance)
        self.bm.verts.index_update()

    def rest(self):
        """Moves the geometry so the part stands on its origin: the lowest point at height 0, centred across."""
        cos = [v.co for v in self.bm.verts]
        lo = Vector((min(c.x for c in cos), min(c.y for c in cos), min(c.z for c in cos)))
        hi = Vector((max(c.x for c in cos), max(c.y for c in cos), max(c.z for c in cos)))
        shift = Vector(((lo.x + hi.x) / 2, (lo.y + hi.y) / 2, lo.z))
        for v in self.bm.verts:
            v.co -= shift
        for k in self.keys.values():
            for i in k:
                k[i] = k[i] - shift
        return hi - lo

    def joint(self, name, at=(0.0, 0.0, 0.0), towards=(0.0, 0.0, 1.0)):
        """A joint where another part plugs in: at a point of the part, pointing `towards` (an empty's z axis). The
        first joint made is the main joint and must be at the origin."""
        if not self.joints and _v(at).length > 1e-6:
            raise ValueError(f"{self.name}: the main joint {name!r} is not at the origin")
        if any(j[0] == name for j in self.joints):
            raise ValueError(f"{self.name}: two joints named {name!r}")
        self.joints.append((name, _v(at), _v(towards).normalized()))

    def done(self, place=(0.0, 0.0, 0.0)):
        """The part as a Blender object in its group's collection, placed at `place` in the file only for viewing:
        its mesh, slots, texture coordinates, joints, and for a figure its weights, shape keys and armature."""
        if not self.joints and self.armature is None:
            raise ValueError(f"{self.name}: a part needs a joint, the first at its origin")
        me = bpy.data.meshes.new(self.name)
        self.bm.verts.index_update()
        self.bm.to_mesh(me)
        self.bm.free()
        me.use_auto_smooth = True
        me.auto_smooth_angle = math.radians(self.smooth)
        ob = bpy.data.objects.new(self.name, me)
        collection(self.group).objects.link(ob)
        for r in self.roles:
            me.materials.append(role(r))
        ob["kit_about"] = self.about
        if self.atlas is not None:
            ob["kit_atlas"] = self.atlas.name
            ob["kit_atlas_size"] = f"{self.atlas.width:.3f} x {self.atlas.height:.3f}"
        if self.wraps:
            ob["kit_wrap_texels"] = ", ".join(str(t) for t in self.wraps)
        if self.weights:
            bones = sorted({b for w in self.weights.values() for b in w})
            groups = {b: ob.vertex_groups.new(name=b) for b in bones}
            for i, w in self.weights.items():
                total = sum(w.values())
                for b, x in w.items():
                    if x > 0:
                        groups[b].add([i], x / total, "REPLACE")
        if self.keys:
            ob.shape_key_add(name="Basis", from_mix=False)
            for name in sorted(self.keys):
                kb = ob.shape_key_add(name=name, from_mix=False)
                for i, co in self.keys[name].items():
                    kb.data[i].co = co
        if self.armature is not None:
            ob.parent = self.armature
            mod = ob.modifiers.new("Armature", "ARMATURE")
            mod.object = self.armature
        else:
            ob.location = _v(place)
        for name, at, towards in self.joints:
            e = bpy.data.objects.new("joint_" + name, None)
            e.empty_display_type = "ARROWS"
            e.empty_display_size = 0.05
            e.parent = ob
            e.location = at
            e.rotation_mode = "QUATERNION"
            e.rotation_quaternion = Vector((0.0, 0.0, 1.0)).rotation_difference(towards)
            e["joint"] = name
            collection(self.group).objects.link(e)
        return ob


def _densify(loop, spacing):
    out = []
    for i in range(len(loop)):
        a, b = Vector(loop[i]), Vector(loop[(i + 1) % len(loop)])
        steps = max(1, math.ceil((b - a).length / spacing))
        out += [a.lerp(b, s / steps) for s in range(steps)]
    return out


def _inside(q, loop):
    """Whether a 2D point lies inside a polygon (even-odd rule)."""
    x, y = q[0], q[1]
    inside = False
    for i in range(len(loop)):
        x1, y1 = loop[i][0], loop[i][1]
        x2, y2 = loop[(i + 1) % len(loop)][0], loop[(i + 1) % len(loop)][1]
        if (y1 > y) != (y2 > y) and x < x1 + (y - y1) * (x2 - x1) / (y2 - y1):
            inside = not inside
    return inside


def _seg_distance(q, a, b):
    ab = b - a
    t = 0.0 if ab.length_squared == 0 else max(0.0, min(1.0, (q - a).dot(ab) / ab.length_squared))
    return (q - (a + ab * t)).length


def armature(name, bones, group="figures", about=""):
    """A figure's skeleton: bones as (name, head, tail, parent or None), in metres, a parent before its children.
    Bones are not connected, so each may turn about its head; the poser reads them by name."""
    data = bpy.data.armatures.new(name)
    ob = bpy.data.objects.new(name, data)
    collection(group).objects.link(ob)
    ob["kit_about"] = about
    bpy.context.view_layer.objects.active = ob
    bpy.ops.object.mode_set(mode="EDIT")
    for bone, head, tail, parent in bones:
        eb = data.edit_bones.new(bone)
        eb.head = _v(head)
        eb.tail = _v(tail)
        if parent:
            eb.parent = data.edit_bones[parent]
            eb.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    return ob


def pose(arm, action_name, turns, frame=1):
    """A still pose kept in the file as an action, such as the bend test the preview shows: turns is {bone: (x, y, z)
    in degrees}, each about the bone's own axes."""
    if arm.animation_data is None:
        arm.animation_data_create()
    act = bpy.data.actions.new(action_name)
    act.use_fake_user = True
    arm.animation_data.action = act
    for bone, (x, y, z) in turns.items():
        pb = arm.pose.bones[bone]
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = (math.radians(x), math.radians(y), math.radians(z))
        pb.keyframe_insert("rotation_euler", frame=frame)
    for pb in arm.pose.bones:
        pb.rotation_euler = (0.0, 0.0, 0.0)
    arm.animation_data.action = None
    return act


def lay_out(gap=0.3):
    """Places the file's parts for viewing only, never for the game: each collection's parts in a row along x in the
    order they were made, the rows one behind the other; a figure's skinned parts stay on their armature."""
    y = 0.0
    for c in bpy.context.scene.collection.children:
        tops = [o for o in c.objects if o.parent is None and o.type in ("MESH", "ARMATURE")]
        if not tops:
            continue
        x, depth = 0.0, 0.0
        for o in tops:
            lo, hi = _extent(o)
            o.location = (x - lo.x, y - lo.y, 0.0)
            x += (hi.x - lo.x) + gap
            depth = max(depth, hi.y - lo.y)
        y += depth + 2 * gap


def _extent(o):
    """The low and high corners of an object and its skinned parts, in its own space."""
    obs = [o] + [c for c in o.children if c.type == "MESH"]
    cs = [Vector(c) for ob in obs for c in ob.bound_box]
    if not cs:
        return Vector((0, 0, 0)), Vector((0, 0, 0))
    lo = Vector((min(c.x for c in cs), min(c.y for c in cs), min(c.z for c in cs)))
    hi = Vector((max(c.x for c in cs), max(c.y for c in cs), max(c.z for c in cs)))
    return lo, hi


def finish(path):
    """Lays the parts out for viewing and saves the file, compressed, with no .blend1 beside it."""
    lay_out()
    bpy.ops.wm.save_as_mainfile(filepath=path, compress=True, check_existing=False)
