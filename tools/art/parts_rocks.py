"""The art lane's rocks family of parts (A6.1, A6.3, A6.4): what art/models/rocks.blend is made from, a Blender script
whose every number is written here, so the file can be made again and each part judged against its catalogue sheet.
Run inside Blender, headless, as

    blender --background --factory-startup --python tools/art/parts_rocks.py -- <out.blend> [<out.kdkit>]

It makes the parts in memory, keeps them as a Blender file, and exports them as any family is (tools/blender/export.py),
so the exporter's own checks (a part's name, its role slots, its joints) are made at once. Texture coordinates are in
metres with a texture pixel 1/64 m at band 0, and each face takes its own flat projection along its own normal with
height as its vertical (A6.4: "each triangle takes one projection, from the direction it faces"), which carries a
texture pixel over without stretch; every part carries a baked crease. Implements PRE-46 and PRE-22.

The parts, as sheets 2.16 (boulders), 2.18 (scree) and 2.13 (the cliff) show them, every one with its base flat on
z = 0 and its middle on the origin (a part sits on the ground wherever the recipe puts it):
- `boulder_small` (1.0 x 0.8 m, 0.7 m high), `boulder_block` (the limestone block, 2.0 x 1.5 x 1.4 m) and
  `boulder_large` (3.0 x 2.2 m, 2.0 m high, a crack down its front): the sheet's three stones. Granite wears the rind
  (role `stone`, art:granite), the block the cliff's limestone (role `limestone`).
- `boulder_split_a` and `boulder_split_b`: one granite 1.5 x 1.1 m and 1.0 m high cleaved into two matching halves
  with a gap of 0.10 m at the foot, each leaning 15 degrees out from the break so the pale jagged faces open to the
  sky and the far end of each sinks into the ground.
- `boulder_perch_base` (0.7 x 0.6 m, 0.5 m high, a joint `top` where the upper stone sits, leaning 8 degrees) and
  `boulder_perch_cap` (1.4 x 1.0 m, 1.3 m high, a joint `seat` off its middle): the perched stone, which bears on the
  base unevenly.
- `scree_stone_small`, `_medium` and `_large` (14, 28 and 50 cm across): broken limestone, angular, the stones too
  big to be part of the scree's texture (art:scree), which is the ground; and `shale_flake_small` and `_medium` (28 and
  55 cm), thin dark plates of shale that lie at the foot of the cliff's limestone.
- `cliff_lip`: the limestone bed that stands out of the cliff over the shale weathered back under it, 4 m long and
  wrapping at its two ends (the joints `start` and `end`), its front edge uneven and its underside undercut and dark.

Forms by size on screen (A6.3), every one made from the same numbers as the full part: `<name>_simple` (about a fifth
of its triangles) and `<name>_marker` (a few triangles), the same stone from the same seed in each.
"""

import math
import os
import sys

import bmesh
import bpy
from mathutils import Vector

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "blender"))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import export  # noqa: E402
from standins import Builder, joint, noise  # noqa: E402

FORMS = ("", "_simple", "_marker")  # the full form, the simple one and the marker


# ---- faces, each on its own flat projection ----------------------------------------------------------------------


def lay_faces(builder, role_of, crease_of, smooth, project=None):
    """Every face of the builder's mesh given its role, and at every corner its texture coordinates (the face's own
    flat projection in metres: along the face horizontally, up its slope or the height as vertical, so a bed of the
    rock stays level on every face) and its crease. `role_of(face)` and `crease_of(point, normal)` say which. A part
    whose faces lie side by side in one smooth surface (the lip's front) names `project(point, normal)`, a projection
    of its own that every face shares, so no face's texture is shifted against its neighbour's."""
    builder.bm.normal_update()
    for f in builder.bm.faces:
        n = f.normal.copy()
        n.normalize()
        f.smooth = smooth(f) if callable(smooth) else smooth
        f.material_index = builder.slot(role_of(f))
        t = Vector((0.0, 0.0, 1.0)).cross(n) if abs(n.z) < 0.95 else Vector((1.0, 0.0, 0.0))
        t.normalize()
        w = n.cross(t)
        for loop in f.loops:
            p = loop.vert.co
            uv = project(p, n) if project else None
            loop[builder.uv].uv = uv if uv is not None else (p.dot(t), p.dot(w))
            c = crease_of(p, n)
            loop[builder.crease] = (c, c, c, 1.0)


def drop_loose(bm):
    """Vertices no face uses, which a hull leaves behind."""
    bmesh.ops.delete(bm, geom=[v for v in bm.verts if not v.link_faces], context="VERTS")


def hull(builder, points, bevel=0.0, floor=False):
    """The convex hull of the points as flat faces, coplanar triangles joined, the face on the ground (normal straight
    down at z = 0) left out unless `floor`, the edges chamfered by `bevel` metres."""
    verts = [builder.bm.verts.new(p) for p in points]
    made = bmesh.ops.convex_hull(builder.bm, input=verts)
    bmesh.ops.delete(builder.bm, geom=made["geom_interior"], context="VERTS")
    drop_loose(builder.bm)
    bmesh.ops.dissolve_limit(
        builder.bm, angle_limit=math.radians(1.0), verts=list(builder.bm.verts), edges=list(builder.bm.edges)
    )
    builder.bm.normal_update()
    if not floor:
        flat = [f for f in builder.bm.faces if f.normal.z < -0.9 and all(v.co.z < 1e-6 for v in f.verts)]
        bmesh.ops.delete(builder.bm, geom=flat, context="FACES")
        drop_loose(builder.bm)
    if bevel > 0.0:  # every edge but those on the ground, which would be driven below it
        edges = [e for e in builder.bm.edges if not all(v.co.z < 1e-6 for v in e.verts)]
        bmesh.ops.bevel(builder.bm, geom=edges, offset=bevel, segments=1, affect="EDGES")
        for v in builder.bm.verts:
            v.co.z = max(0.0, v.co.z)
    # triangles, so each takes the projection of its own plane: a chamfer's face is seldom quite flat
    bmesh.ops.triangulate(builder.bm, faces=list(builder.bm.faces))


# ---- the domes: boulders of granite ------------------------------------------------------------------------------


class Dome:
    """A rounded boulder as a surface of two numbers: `t` (0 at its crown, 1 where it meets the ground) and `phi` (the
    angle round it, 0 to the east, counter-clockwise from above). The footprint is a rounded square of half-width `a`
    (east) and `b` (north) with a few slow swells in its outline; the crown is `high` metres up, off the middle by
    `shift`, and the flank steepens to upright at the ground, as a stone sunk in the ground does. It is not the same on
    every side: `skew` (a share of the height, east to west) raises one shoulder over the other, and `steep` (a share of
    the flank's profile) makes the side toward the angle `steep_phi` fuller and steeper than the one opposite, so the
    stone is lopsided as a weathered one is, not a hemisphere. The same numbers give the same stone in every form."""

    def __init__(
        self,
        a,
        b,
        high,
        seed,
        shift=(0.0, 0.0),
        squareness=2.4,
        crown=3.2,
        flank=0.56,
        skew=0.0,
        steep=0.0,
        steep_phi=0.0,
    ):
        self.a, self.b, self.high, self.seed, self.shift = a, b, high, seed, shift
        self.e, self.p, self.q = squareness, crown, flank
        self.skew, self.steep, self.steep_phi = skew, steep, steep_phi

    def at(self, t, phi):
        c, s = math.cos(phi), math.sin(phi)
        g = (abs(c) ** self.e + abs(s) ** self.e) ** (-1.0 / self.e)
        sd = self.seed
        swell = (
            1.0 + 0.05 * math.sin(2 * phi + sd) + 0.035 * math.sin(3 * phi + 2 * sd) + 0.02 * math.sin(5 * phi + 3 * sd)
        )
        x = self.a * c * g * swell * t + self.shift[0] * (1.0 - t * t)
        y = self.b * s * g * swell * t + self.shift[1] * (1.0 - t * t)
        lump = 1.0 + 0.045 * math.sin(3.1 * x / self.a + sd) * math.cos(2.3 * y / self.b + 1.3 * sd)
        flank = self.q * (1.0 - self.steep * math.cos(phi - self.steep_phi))
        z = self.high * max(0.0, 1.0 - t**self.p) ** flank * lump * (1.0 + self.skew * x / self.a)
        return Vector((x, y, z))

    def normal_at(self, t, phi):
        h = 1e-3
        du = self.at(min(1.0, t + h), phi) - self.at(max(0.0, t - h), phi)
        dv = self.at(t, phi + h) - self.at(t, phi - h)
        n = dv.cross(du)
        n.normalize()
        return n if n.z >= 0.0 or t > 0.97 else -n


def ring_fractions(rings):
    """Where the rings of a dome lie from crown to ground: closer toward the ground, where the flank is steep."""
    return [1.0 - (1.0 - k / rings) ** 1.5 for k in range(rings + 1)]


def dome_faces(builder, dome, rings, segments, phis=None):
    """The dome's skin in triangles about the crown and quads beyond; with `phis` (the first and last angle) only the
    part of it between those angles, round whose cut edges the caller builds its wall. Returns the grid of vertices."""
    ts = ring_fractions(rings)
    full = phis is None
    lo, hi = (0.0, 2.0 * math.pi) if full else phis
    count = segments if full else segments + 1
    grid = [[None] * count for _ in ts]
    for i, t in enumerate(ts):
        for j in range(count):
            phi = lo + (hi - lo) * j / segments
            grid[i][j] = builder.bm.verts.new(dome.at(t, phi)) if i or j == 0 else grid[0][0]
    for j in range(segments):
        n = (j + 1) % count if full else j + 1
        if full or True:
            builder.bm.faces.new([grid[0][0], grid[1][j], grid[1][n]])
    for i in range(1, rings):
        for j in range(segments):
            n = (j + 1) % count if full else j + 1
            builder.bm.faces.new([grid[i][j], grid[i + 1][j], grid[i + 1][n], grid[i][n]])
    return grid


def dome_crease(high):
    """Baked darkening: dark at the ground, where a stone sits in its own shade, light on the crown."""

    def crease(p, n):
        return 0.55 + 0.45 * min(1.0, max(0.0, p.z / (high * 0.8)))

    return crease


def crack_ribbon(builder, dome, path, width, lift, steps):
    """A crack drawn on the skin as a dark ribbon lying `lift` metres off it, along a path of (t, phi) points (a list of
    pairs, joined smoothly), `width` wide at its start and thinning to a third at its end; its faces take the crease of
    a shadow, so the crack reads at the size of a texture pixel where a groove in so coarse a mesh could not."""
    samples = []
    for k in range(steps + 1):
        s = k / steps
        i = min(len(path) - 2, int(s * (len(path) - 1)))
        f = s * (len(path) - 1) - i
        t = path[i][0] + (path[i + 1][0] - path[i][0]) * f
        phi = path[i][1] + (path[i + 1][1] - path[i][1]) * f
        samples.append((t, phi + 0.045 * noise(77, k, 6) * math.sin(math.pi * s)))  # a crack is never quite smooth
    left, middle, right = [], [], []
    for k, (t, phi) in enumerate(samples):
        before = dome.at(*samples[max(0, k - 1)])
        after = dome.at(*samples[min(steps, k + 1)])
        along = after - before
        along.normalize()
        n = dome.normal_at(t, phi)
        side = n.cross(along)
        side.normalize()
        half = 0.5 * width * (1.0 - 0.66 * k / steps)
        centre = dome.at(t, phi) + n * lift
        left.append(builder.bm.verts.new(centre - side * half))
        middle.append(builder.bm.verts.new(centre - n * 0.5 * lift))
        right.append(builder.bm.verts.new(centre + side * half))
    made = []
    for k in range(steps):
        n = dome.normal_at(*samples[k])
        for quad in (
            [left[k], middle[k], middle[k + 1], left[k + 1]],
            [middle[k], right[k], right[k + 1], middle[k + 1]],
        ):
            a, b, c = (v.co for v in quad[:3])
            if (b - a).cross(c - b).dot(n) < 0.0:
                quad = quad[::-1]
            made.append(builder.bm.faces.new(quad))
    return made, set(middle)


def boulder(name, spec, rings, segments, crack_steps):
    """One form of a granite boulder: the dome, its crack when it has one and the form is big enough to show it."""
    a, b, high, seed, shift, crack, lopsided = spec
    dome = Dome(a, b, high, seed, shift, **lopsided)
    builder = Builder(name)
    dome_faces(builder, dome, rings, segments)
    ribbon, centres = (
        crack_ribbon(builder, dome, crack, 0.09, 0.006, crack_steps) if crack and crack_steps else ([], ())
    )
    for f in ribbon:
        f.tag = True
    drop_loose(builder.bm)
    lay_faces(builder, lambda f: "stone", dome_crease(high), True)
    for f in builder.bm.faces:
        if f.tag:
            for loop in f.loops:  # dark in the middle of the crack, the lit lips of it at its edges paler
                v = loop.vert.co
                c = (
                    0.03
                    if loop.vert in centres
                    else 0.07 + 0.10 * abs(noise(round(v.x * 40), round(v.y * 40), round(v.z * 40)))
                )
                loop[builder.crease] = (c, c, c, 1.0)
    return builder.build((0.0, 0.0, 0.0))


# the three granite boulders of sheet 2.16 (the sheet's own metres): half-width east, half-width north, height, seed,
# where the crown lies off the middle, and the crack as (t, phi) points from the crown down the front
BOULDERS = {
    "boulder_small": (
        0.50,
        0.42,
        0.60,
        31,
        (0.16, 0.0),
        None,
        {"skew": 0.20, "steep": 0.30, "steep_phi": math.pi, "crown": 4.4, "flank": 0.64, "squareness": 2.0},
    ),
    "boulder_large": (
        1.50,
        1.10,
        1.83,
        32,
        (-0.42, -0.08),
        [(0.10, -1.05), (0.42, -1.18), (0.72, -0.95), (0.88, -0.80)],
        {"skew": -0.12, "steep": 0.26, "steep_phi": 0.0, "crown": 3.6, "flank": 0.6},
    ),
}
# a form's rings and segments round a dome, by the dome's own size (the crack's steps, none where it would not show):
# full, simple and marker
DOME_FORMS = {
    "boulder_small": [(8, 24, 0), (4, 12, 0), (2, 8, 0)],
    "boulder_large": [(12, 32, 14), (6, 16, 6), (3, 10, 0)],
}


def boulders():
    """The two granite boulders in their three forms."""
    out = []
    for name, spec in BOULDERS.items():
        for suffix, (rings, segments, steps) in zip(FORMS, DOME_FORMS[name], strict=True):
            out.append(boulder(name + suffix, spec, rings, segments, steps))
    return out


# ---- the limestone block ------------------------------------------------------------------------------------------

# the block of sheet 2.16, 2.0 m across (x), 1.5 m deep (y), 1.4 m high: a broad top that slopes down to the right, the
# upper left corner clipped, a steep right face; its base and top as points (x, y, z). The marker keeps only the
# corners of the front.
BLOCK_BASE = [(-0.97, -0.72, 0.0), (0.93, -0.78, 0.0), (1.00, 0.52, 0.0), (-0.08, 0.75, 0.0), (-1.00, 0.38, 0.0)]
BLOCK_TOP = [
    (-0.86, -0.58, 1.30), (0.82, -0.64, 0.96), (0.86, 0.44, 0.98), (-0.04, 0.58, 1.18), (-0.88, 0.28, 1.40),
]  # fmt: skip
BLOCK_CLIP = [(-0.99, -0.50, 1.02)]  # the clipped upper left corner: a point lower down the left, which the hull cuts
# four more points a little out of its faces, so no face is a flat plane and its outline is not straight
BLOCK_EXTRA = [(-0.60, -0.76, 0.55), (0.98, -0.25, 0.45)]
# where on the limestone's near tile (metres east, metres up) the block is cut from: the 2 x 1.4 m stretch of it whose
# beds run most nearly level (found by the share of its gradient that is vertical), so each face shows straight beds
BLOCK_FROM = (3.625, 3.21)


def roughened(points, seed, amount):
    """The block's corners each moved a little (the base's along the ground only), the same every time."""
    return [
        (
            x + amount * noise(seed, k, 1),
            y + amount * noise(seed, k, 2),
            z + (amount * noise(seed, k, 3) if z > 0.0 else 0.0),
        )
        for k, (x, y, z) in enumerate(points)
    ]


BLOCK_FORMS = [
    (roughened(BLOCK_BASE + BLOCK_TOP + BLOCK_CLIP + BLOCK_EXTRA, 71, 0.025), 0.04),
    (roughened(BLOCK_BASE + BLOCK_TOP, 71, 0.025), 0.0),
    (BLOCK_BASE[:4] + BLOCK_TOP[:4], 0.0),
]


def block_crease(p, n):
    """The block is light on top and under its beds; darkening at the ground, and on faces turned from the light."""
    return (0.62 + 0.38 * min(1.0, max(0.0, p.z / 1.1))) * (0.84 + 0.16 * n.z)


def square_to_side(p, n):
    """A face on the plane of the side it most nearly faces (south or north: along x, up z; east or west: along y, up z;
    up: along x and y), so the beds, which run level, run straight across every face of one side, a chamfer or a face
    a little out of true included, and a texture pixel is carried over by at most 1 / cos 44 degrees. A face turned
    further than that from every side (a corner's chamfer) takes the projection of its own plane (None)."""
    if max(abs(n.x), abs(n.y), abs(n.z)) < 0.72:  # a corner's chamfer, turned too far from every side: its own plane
        return None
    u, v = BLOCK_FROM
    if abs(n.z) >= max(abs(n.x), abs(n.y)):
        return (p.x + u, p.y + v)
    return (p.x + u, p.z + v) if abs(n.y) >= abs(n.x) else (p.y + u, p.z + v)


def block(name, points, bevel):
    builder = Builder(name)
    hull(builder, points, bevel)
    lay_faces(builder, lambda f: "limestone", block_crease, False, square_to_side)
    return builder.build((0.0, 0.0, 0.0))


def blocks():
    return [
        block("boulder_block" + suffix, points, bevel)
        for suffix, (points, bevel) in zip(FORMS, BLOCK_FORMS, strict=True)
    ]


# ---- the split boulder and the perched one ---------------------------------------------------------------------


def jag(y, z):
    """How far the broken face stands off its plane at a place, in metres: both halves use the same function, so they
    fit as the two sides of a break do."""
    return 0.06 * (
        noise(41, round(y * 7), round(z * 7))
        + 0.6 * noise(43, round(y * 16), round(z * 16))
        + 0.3 * noise(45, round(y * 40), round(z * 40))
    )


def split_half(name, side, rings, segments, rows):
    """One half of the cleaved granite: its dome cut along the plane x = 0, then moved off it by half the gap, and the
    pale jagged face where it was broken, which carries the lightest crease so it reads paler than the weathered
    skin. `side` is +1 for the half to the east, -1 for the other."""
    dome = Dome(0.75, 0.55, 0.95, 33, (0.0, 0.0), crown=2.4, flank=0.60, steep=0.2, steep_phi=-math.pi / 2.0)
    builder = Builder(name)
    lo = -math.pi / 2.0 if side > 0 else math.pi / 2.0  # the east half's angles run from -90 to +90 degrees
    grid = dome_faces(builder, dome, rings, segments, (lo, lo + math.pi))
    # the dome's outline in the cut plane, from one ground end over the crown to the other
    profile = (
        [grid[i][0] for i in range(rings, 0, -1)] + [grid[0][0]] + [grid[i][segments] for i in range(1, rings + 1)]
    )

    def below(v, r):
        """The wall's vertex at row r under the outline's vertex v: straight down, the broken face's jag across it."""
        z = v.co.z * (1.0 - r / rows)
        return builder.bm.verts.new((v.co.x + (jag(v.co.y, z) if r < rows else 0.0), v.co.y, z))

    wall = [[v if r == 0 else below(v, r) for v in profile] for r in range(rows + 1)]
    faces = []
    for r in range(rows):
        for k in range(len(profile) - 1):
            face = builder.bm.faces.new([wall[r][k], wall[r + 1][k], wall[r + 1][k + 1], wall[r][k + 1]])
            face.normal_update()
            if face.normal.x * side > 0.0:  # it must face the other half: west of an east half, east of a west one
                face.normal_flip()
            face.tag = True
            faces.append(face)
    # the broken face in triangles, each on the projection of its own plane (its jag leaves no quad flat)
    for f in bmesh.ops.triangulate(builder.bm, faces=faces)["faces"]:
        f.tag = True
    faces = [f for f in builder.bm.faces if f.tag]
    drop_loose(builder.bm)
    lay_faces(builder, lambda f: "stone", dome_crease(1.0), lambda f: not f.tag)
    for f in faces:
        for loop in f.loops:
            loop[builder.crease] = (1.0, 1.0, 1.0, 1.0)
    obj = builder.build((0.0, 0.0, 0.0))
    lean = math.radians(LEAN)
    for v in obj.data.vertices:  # leaning out about the foot of the break, so its far end sinks into the ground
        x, z = v.co.x, v.co.z
        v.co.x = x * math.cos(lean) + side * z * math.sin(lean) + 0.05 * side
        v.co.z = z * math.cos(lean) - side * x * math.sin(lean)
    return obj


LEAN = 15.0  # degrees each half of the split boulder leans from the break
SPLIT_FORMS = [(8, 12, 5), (4, 6, 2), (2, 4, 1)]


def splits():
    out = []
    for suffix, (rings, segments, rows) in zip(FORMS, SPLIT_FORMS, strict=True):
        out.append(split_half("boulder_split_a" + suffix, -1, rings, segments, rows))
        out.append(split_half("boulder_split_b" + suffix, 1, rings, segments, rows))
    return out


def perched():
    """The perched stone: a small low dome and, on it, the upper stone, whose underside is closed so it shows if it
    overhangs. The base has a joint `top` at its crown, the cap a joint `seat` on its underside."""
    out = []
    forms = [(7, 20, 7, 20), (4, 10, 4, 10), (2, 8, 2, 8)]
    for suffix, (br, bs, cr, cs) in zip(FORMS, forms, strict=True):
        base_dome = Dome(0.35, 0.30, 0.50, 34, (0.06, 0.0), skew=0.14, steep=0.2, steep_phi=math.pi)
        b = Builder("boulder_perch_base" + suffix)
        dome_faces(b, base_dome, br, bs)
        drop_loose(b.bm)
        lay_faces(b, lambda f: "stone", dome_crease(0.5), True)
        obj = b.build((0.0, 0.0, 0.0))
        joint(obj, "top", tuple(base_dome.at(0.0, 0.0) - Vector((0.0, 0.0, 0.02))), (0.0, math.radians(-8.0), 0.0))
        out.append(obj)
        cap_dome = Dome(
            0.70, 0.50, 1.25, 35, (-0.16, 0.0), crown=2.4, flank=0.66, skew=-0.12, steep=0.22, steep_phi=0.0
        )
        c = Builder("boulder_perch_cap" + suffix)
        grid = dome_faces(c, cap_dome, cr, cs)
        underside = [grid[cr][j] for j in range(cs)]
        centre = c.bm.verts.new((cap_dome.shift[0], cap_dome.shift[1], 0.0))
        under = [c.bm.faces.new([centre, underside[(j + 1) % cs], underside[j]]) for j in range(cs)]
        drop_loose(c.bm)
        shade = dome_crease(1.3)
        lay_faces(c, lambda f: "stone", shade, True)
        for f in under:
            for loop in f.loops:
                loop[c.crease] = (0.30, 0.30, 0.30, 1.0)
        obj = c.build((0.0, 0.0, 0.0))
        joint(obj, "seat", (0.30, 0.05, 0.0))  # it bears on the base off its own middle, so it sits unevenly
        out.append(obj)
    return out


# ---- scree stones and shale flakes ------------------------------------------------------------------------------


def chip_points(seed, lx, ly, lz, base_points, top_points):
    """The corners of a broken stone: `base_points` on the ground (z = 0), irregular round a rough rectangle, and
    `top_points` above them, drawn in from the base's outline, each moved by the stone's seed."""
    points = []
    for k in range(base_points):
        angle = 2.0 * math.pi * (k + 0.6 * noise(seed, k, 1)) / base_points
        r = 0.5 * (0.78 + 0.22 * noise(seed, k, 2))
        points.append((lx * r * math.cos(angle) * 1.15, ly * r * math.sin(angle) * 1.15, 0.0))
    for k in range(top_points):
        angle = 2.0 * math.pi * (k + 0.5 + 0.4 * noise(seed, k, 3)) / top_points
        r = 0.5 * (0.62 + 0.28 * abs(noise(seed, k, 4)))
        height = lz * (0.45 + 0.55 * abs(noise(seed, k, 5)))
        points.append((lx * r * math.cos(angle), ly * r * math.sin(angle), height))
    return points


def chip(name, seed, size, base_points, top_points, role):
    dark = 0.62 if role == "shale" else 1.0  # a flake is a dark plate: its crease darker than a limestone chip's
    builder = Builder(name)
    hull(builder, chip_points(seed, *size, base_points, top_points))
    high = size[2]

    def shade(p, n):
        return dark * (0.58 + 0.42 * min(1.0, max(0.0, p.z / (0.9 * high)))) * (0.82 + 0.18 * n.z)

    lay_faces(builder, lambda f: role, shade, False)
    return builder.build((0.0, 0.0, 0.0))


# name, seed, size (length, width, height) in metres, role; its three forms' base and top points
CHIPS = {
    "scree_stone_small": (51, (0.14, 0.10, 0.05), "limestone", [(5, 4), (4, 2), (3, 1)]),
    "scree_stone_medium": (52, (0.28, 0.20, 0.09), "limestone", [(6, 5), (5, 3), (3, 1)]),
    "scree_stone_large": (53, (0.50, 0.36, 0.15), "limestone", [(7, 5), (5, 3), (4, 1)]),
    "shale_flake_small": (54, (0.34, 0.22, 0.03), "shale", [(6, 4), (5, 2), (3, 1)]),
    "shale_flake_medium": (55, (0.70, 0.46, 0.045), "shale", [(7, 5), (5, 3), (4, 1)]),
}


def chips():
    return [
        chip(name + suffix, seed, size, *counts, role)
        for name, (seed, size, role, forms) in CHIPS.items()
        for suffix, counts in zip(FORMS, forms, strict=True)
    ]


# ---- the cliff's limestone lip ---------------------------------------------------------------------------------

LIP = 4.0  # its length, wrapping: every swell has a whole number of waves along it, so two lips lie end to end
BACK = 0.55  # how far behind the face above its back lies: in the cliff, where the shale has weathered back to


def wave(x, seed, orders):
    """A slow irregular swell along the lip that is the same at its two ends: a sum of waves of whole numbers of
    cycles in LIP metres."""
    return sum(amp * math.sin(2.0 * math.pi * k * x / LIP + seed * k) for k, amp in orders)


LIP_PHASES = (1.3, 3.1, 5.2)  # where each of the three lips' own swells begin


def window(x):
    """0 at both ends of the lip and 1 in its middle, so what is multiplied by it leaves the two ends as they are."""
    return math.sin(math.pi * x / LIP) ** 2


def nose(x, variant=0):
    """How far the lip's front stands out of the face above it, in metres: a swell common to the three lips and, away
    from the ends, one of each lip's own, so any two lips lie end to end without a step and no two are alike."""
    own = wave(x, LIP_PHASES[variant], [(1, 0.09), (2, 0.08), (3, 0.05), (5, 0.02)])
    return 0.24 + wave(x, 1.3, [(1, 0.05)]) + window(x) * own


def thick(x, variant=0):
    """How thick the bed is at its front, in metres, made as the nose is."""
    own = wave(x, LIP_PHASES[variant] + 0.7, [(1, 0.04), (2, 0.05), (3, 0.04)])
    return 0.46 + wave(x, 0.7, [(1, 0.04)]) + window(x) * own


def lip(name, segments, front_rows, variant=0):
    """The limestone bed: along x from 0 to 4 m, its back (y = +0.55, in the cliff behind the shale's weathered face)
    and its front out to y = -nose, the top flush with the face above at z = 0 and the underside, undercut and rising
    toward the back, dark
    with the shade it casts. Faces: the top, the front in `front_rows` rows with a swell to each, the underside, and the
    two ends, which are flat."""
    builder = Builder(name)
    xs = [LIP * k / segments for k in range(segments + 1)]

    def front(x, r):
        """A point of the front at a fraction r down it (0 at the top edge, 1 at the underside's edge)."""
        bulge = 0.03 * window(x) * math.sin(math.pi * r) * (0.6 + 0.4 * noise(61 + variant, round(x * 4), round(r * 7)))
        return Vector((x, -nose(x, variant) - bulge, -thick(x, variant) * r - 0.015 * (1 - r)))

    cols = [
        {
            "back_top": builder.bm.verts.new((x, BACK, 0.0)),
            "front": [builder.bm.verts.new(front(x, r / front_rows)) for r in range(front_rows + 1)],
            "back_under": builder.bm.verts.new((x, BACK, -thick(x, variant) + 0.16)),
        }
        for x in xs
    ]

    def face(points, outward):
        """A face turned to face `outward` (a rough direction)."""
        f = builder.bm.faces.new(points)
        f.normal_update()
        if f.normal.dot(Vector(outward)) < 0.0:
            f.normal_flip()
        return f

    for a, b in zip(cols, cols[1:], strict=False):
        face([a["back_top"], b["back_top"], b["front"][0], a["front"][0]], (0.0, -0.2, 1.0))
        for r in range(front_rows):
            face([a["front"][r], b["front"][r], b["front"][r + 1], a["front"][r + 1]], (0.0, -1.0, 0.0))
        face([a["front"][-1], b["front"][-1], b["back_under"], a["back_under"]], (0.0, -0.3, -1.0))
    for column, outward in ((cols[0], (-1.0, 0.0, 0.0)), (cols[-1], (1.0, 0.0, 0.0))):
        face([column["back_top"], *column["front"], column["back_under"]], outward)

    def shade(p, n):
        """Dark under the bed, as the shadow it casts is; the front a little darker toward its foot."""
        if n.z < -0.3:
            return 0.30 + 0.20 * (1.0 + n.z)
        return 0.72 + 0.28 * min(1.0, max(0.0, (p.z + 0.55) / 0.55))

    def along(p, n):
        """The front and the faces beside it from the south, the top and the underside from above: the bed runs
        level and unbroken along the whole lip."""
        if abs(n.x) > 0.7:  # an end of the bed, seen from the side
            return (p.y, p.z)
        return (p.x, p.z) if abs(n.z) < 0.7 else (p.x, -p.y)

    lay_faces(builder, lambda f: "limestone", shade, lambda f: abs(f.normal.x) < 0.5 and f.normal.z < 0.5, along)
    stride = front_rows + 3  # the vertices of a column: its top at the back, its front rows, its underside at the back
    where = {c * stride + 1 + j: (x, j / front_rows) for c, x in enumerate(xs) for j in range(front_rows + 1)}

    def surface_normal(x, r):
        """The normal of the front's smooth surface at a place, from the surface itself, which goes on past the ends of
        the part as the next lip's front does: so a vertex at an end is shaded as the join's two sides are, not as the
        edge of one."""
        h = 1e-3
        n = (front(x, min(1.0, r + h)) - front(x, max(0.0, r - h))).cross(front(x + h, r) - front(x - h, r))
        n.normalize()
        return n if n.y < 0.0 else -n

    obj = builder.build((0.0, 0.0, 0.0))
    mesh = obj.data
    if hasattr(mesh, "use_auto_smooth"):  # Blender 4.0 and earlier need it for custom normals; 4.1 and later do not
        mesh.use_auto_smooth = True
    normals = []
    for poly in mesh.polygons:
        for corner in poly.loop_indices:
            vertex = mesh.loops[corner].vertex_index
            smooth_front = poly.use_smooth and vertex in where and abs(poly.normal.y) > 0.5
            normals.append(surface_normal(*where[vertex]) if smooth_front else poly.normal.copy())
    mesh.normals_split_custom_set(normals)
    joint(obj, "start", (0.0, 0.0, 0.0))
    joint(obj, "end", (LIP, 0.0, 0.0))
    return obj


LIP_FORMS = [(40, 3), (16, 2), (8, 1)]


def lips():
    """The three lips (`cliff_lip`, `cliff_lip_b`, `cliff_lip_c`), each in its three forms."""
    return [
        lip("cliff_lip" + kind + suffix, *form, variant)
        for variant, kind in enumerate(("", "_b", "_c"))
        for suffix, form in zip(FORMS, LIP_FORMS, strict=True)
    ]


# ---- the family ------------------------------------------------------------------------------------------------


def make_family():
    """Every part of the rocks family, laid in a row in the file, which the exporter does not care about: the
    boulders first, then the stones, the flakes and the lip, each part's own place 5 m from the last."""
    parts = [*boulders(), *blocks(), *splits(), *perched(), *chips(), *lips()]
    x = 0.0
    for obj in parts:
        obj.location = (x, 0.0, 0.0)
        x += 5.0
    return parts


def main():
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    if not 1 <= len(argv) <= 2:
        raise SystemExit("usage: blender --background --python tools/art/parts_rocks.py -- <out.blend> [<out.kdkit>]")
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
