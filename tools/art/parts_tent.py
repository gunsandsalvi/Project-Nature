"""The hide tent's parts for the camp family (sheet 16.4, A6.1, A6.4): a small round tent of deer or reindeer hides on a
cone of ten poles, its hem held down by a ring of 24 stones. Imported by tools/art/parts_camp.py, which makes the file;
run inside Blender. Implements PRE-46 and PRE-22.

The tent's own metres: x east, y north, z up, its middle on the ground, the door to the south (Blender's axes; the
exporter carries them into Godot's). The cover is 3.8 m across at the ground and 2.6 m high where it is gathered (its
cone, which comes to a point at 2.76 m, lies outside every pole); ten poles cross at 2.7 m
and their tips reach 3.06 m; the door is a low slit 1.2 m high and 0.5 m wide at the ground; the ring of stones is 4.2 m
across.

The parts, as the sheet's "parts" row shows them:
- the cover, as the eleven hides the sheet sews it from (seven in a lower ring, four in an upper ring, their seams
  uneven and offset from each other, the door slit in the first lower hide): `hide_lower_1` to `hide_lower_7`,
  `hide_upper_1` to `hide_upper_4`. Each lies where it belongs in the tent's own metres, so the recipe lays each at the
  origin (a root). A hide is two skins (outer and inner) over one stretch of the cone, flat-cut in the sheet's laid-out
  sector: its texture coordinates are that flat cut in metres, so a hide's texture pixels do not stretch. Along every
  edge a band 6 cm wide (4 texture pixels) takes the seam strip of the hide's texture (the running stitch), so the
  seams are carried by the part's edge and the shared hide texture, not by a texture of the cover's own.
- `tent_pole`: a pole standing along z with joints at its foot, where it crosses the others, and its tip.
- `tent_binding`: the lashing where the poles cross.
- `tent_door_flap`: the hanging flap, half open.
- `ring_stone_small`, `ring_stone_medium`, `ring_stone_large`: the stones, 25 to 45 cm across.
"""

import math

import bmesh
from mathutils import Matrix, Vector
from standins import Builder, joint, lathe

BASE = 1.9  # the cover's radius at the ground
HIGH = 2.76  # where the cover's cone would come to a point; the cover is gathered 0.2 m of slant short of it, 2.6 m up
SLANT = math.hypot(BASE, HIGH)
NECK = 0.20  # slant distance from the apex of the circle the cover gathers on (a radius of 11 cm)
CROSS = (
    2.70  # where the ten poles cross, on the axis; the cone cover lies outside every pole from the ground to its neck
)
DOOR = -math.pi / 2.0  # south
SEAM = 4.0 / 64.0  # the seam strip of the hide's texture, 4 texture pixels wide, in metres
THICK = 0.012  # the hide's thickness

# The seams, in degrees round from the door (the sheet's map: the first lower hide holds the door; the upper hides'
# seams are offset from the lower ones so no straight spoke runs through both tiers). Every angle is a multiple of 5.
LOWER = [-25, 25, 80, 125, 175, 230, 275, 335]
UPPER = [10, 105, 190, 295, 370]
SLIT = 1.2  # height of the door slit
SLIT_AT_HEM, SLIT_AT_TOP = 8.0, 1.5  # half its width in degrees round, at the hem and at its top
HIDE_FRAME = 0.0625  # the hide texture's seam strip lies in its first 4 columns, so the field starts after them
# The hides' tones: the hide texture is the lightest hide (#C99960), and each hide keeps this share of its light, so the
# sheet's middle hides (#AA7848, about 0.8) and dark ones (#805532, about 0.6) are the same texture darkened by the
# part's baked crease. No two neighbours are alike, and the door's hide is a darker one so the pale flap shows on it.
LOWER_TONES = [0.70, 0.96, 0.52, 0.84, 0.58, 1.00, 0.56]
UPPER_TONES = [0.56, 0.88, 0.66, 1.00]


def rel(degrees):
    """An angle in degrees from the door as an azimuth in radians."""
    return DOOR + math.radians(degrees)


def tier_s(a):
    """The slant distance of the boundary between the upper and lower hides at an azimuth: about 46% of the slant from
    the apex, scalloped by the hide outlines."""
    return 0.46 * SLANT + 0.07 * math.sin(3.0 * a + 0.7) + 0.035 * math.sin(7.0 * a + 2.0)


def hem_s(a):
    """The slant distance of the hem: nearly the full slant, uneven (it lifts a few centimetres between the stones and
    lies down under them) and never under the ground."""
    return SLANT - 0.035 * (1.0 + math.sin(5.0 * a + 1.0)) - 0.015 * (1.0 + math.sin(11.0 * a + 2.0))


def wobble(seed, v):
    """How far a seam strays round the cone at a fraction v of its length (zero at both ends), in radians."""
    return 0.03 * math.sin(math.pi * v) * math.sin(2.0 * math.pi * (1.3 + 0.37 * (seed % 5)) * v + 0.9 * seed)


def outward(a):
    """The cone's outward normal at an azimuth."""
    return Vector((HIGH * math.cos(a), HIGH * math.sin(a), BASE)).normalized()


def tented(a, s):
    """How far the cover stands out of its cone at a point, in metres: it rides over each of the ten poles and sags
    between them, by up to 3 cm, more toward the hem and none at the neck. The poles stand at the compass bearings
    18 degrees and every 36 after (the recipe's), which in the cone's own angle is 72 degrees and every 36 before."""
    reach = min(1.0, max(0.0, (s - NECK - 0.25) / 0.9))
    return 0.03 * reach * math.cos(10.0 * (a - math.radians(72.0)))


def cone_point(a, s):
    """The point of the cover at an azimuth and a slant distance from the apex: the cone, tented over the poles."""
    r = BASE * s / SLANT
    cone = Vector((r * math.cos(a), r * math.sin(a), HIGH * (1.0 - s / SLANT)))
    return cone + tented(a, s) * outward(a)


class Hide:
    """One hide being built in a part's builder: curves in (azimuth, slant distance) that bound its stretches of the
    cone, each stretch a Coons patch (the surface blended between its four edges) meshed with a thin band round its
    edges."""

    def __init__(self, builder, index, tone):
        self.b = builder
        self.index = index
        self.tone = tone  # how much of the hide texture's light this hide keeps (1 the light hides, down to the dark)
        self.frame = 0.0  # the azimuth the patch's flat cut is turned to
        self.low = (1e9, 1e9)

    @staticmethod
    def coons(top, bottom, left, right):
        """The map (u, v) to (azimuth, slant) of a stretch whose edges are top(u) and bottom(u) from its left to its
        right and left(v) and right(v) from its top to its bottom, each a function to (azimuth, slant)."""
        c00, c10, c01, c11 = top(0.0), top(1.0), bottom(0.0), bottom(1.0)

        def at(u, v):
            t, b, lf, r = top(u), bottom(u), left(v), right(v)
            return tuple(
                (1 - v) * t[i]
                + v * b[i]
                + (1 - u) * lf[i]
                + u * r[i]
                - ((1 - u) * (1 - v) * c00[i] + u * (1 - v) * c10[i] + (1 - u) * v * c01[i] + u * v * c11[i])
                for i in (0, 1)
            )

        return at

    def flat(self, a, s):
        """A point of the cone in the flat cut of the hide's own sector (metres): the cone unrolled, which is exact,
        with the hide's middle turned to run along v."""
        alpha = (a - self.frame) * BASE / SLANT
        return s * math.sin(alpha), s * math.cos(alpha)

    def stretch(self, curves, columns, rows, bands):
        """One stretch gridded: its points as {(i, j): (outer point, inner point, azimuth, slant)}: `columns` x `rows`
        cells, and a thin band of cells (6.25 cm wide, the seam strip's width, in every row and column) along each
        edge `bands` = (top, bottom, left, right) says is a seam: the free edges (the hem, the neck, the door slit)
        have none, being unstitched."""
        at = self.coons(*curves)
        top, bottom, left, right = bands

        def chord(u0, v0, u1, v1):
            return (cone_point(*at(u0, v0)) - cone_point(*at(u1, v1))).length

        def lines(count, length, first, last):
            """The fractions along a run of `length` metres where its cell lines lie: `count` cells between a band of
            6.25 cm at each end that is asked for and the rest."""
            band = min(0.45, SEAM / max(length, 1e-6))
            lo, hi = (band if first else 0.0), 1.0 - (band if last else 0.0)
            out = [0.0] + ([band] if first else [])
            out += [lo + (hi - lo) * k / count for k in range(1, count)]
            out += ([1.0 - band] if last else []) + [1.0]
            return out

        width, height = chord(0.0, 0.5, 1.0, 0.5), chord(0.5, 0.0, 0.5, 1.0)
        nominal_u = lines(columns, width, left, right)
        nominal_v = lines(rows, height, top, bottom)
        nu, nv = len(nominal_u) - 1, len(nominal_v) - 1
        points = {}
        for j in range(nv + 1):
            row = chord(0.0, nominal_v[j], 1.0, nominal_v[j])
            row_u = lines(columns, row, left, right)
            for i in range(nu + 1):
                u = row_u[i]
                col_v = lines(rows, chord(nominal_u[i], 0.0, nominal_u[i], 1.0), top, bottom)
                v = col_v[j]
                a, s = at(u, v)
                p = cone_point(a, s)
                points[(i, j)] = (p, p - THICK * outward(a), a, s)
        for p in points.values():
            x, y = self.flat(p[2], p[3])
            self.low = (min(self.low[0], x), min(self.low[1], y))
        return points, nu, nv

    def crease(self, p):
        """Baked darkening: the hide's own tone (the sheet sews light, middle and dark hides side by side, and the one
        texture is the light hide) and darker toward the ground, where the stones weigh it down."""
        return self.tone * (0.72 + 0.28 * min(1.0, p.z / 0.9))


def hide_patch(builder, hide, curves, columns, rows, offset, bands):
    """Builds one stretch into the builder: both skins, the seam strip's band along its edges, and the field's flat cut
    (metres, after the seam strip's first 4 columns of the texture, `offset` metres along v)."""
    points, nu, nv = hide.stretch(curves, columns, rows, bands)
    top, bottom, left, right = bands
    verts = {key: (builder.bm.verts.new(p[0]), builder.bm.verts.new(p[1])) for key, p in points.items()}

    def run(keys):
        """The points along an edge of the stretch and the distance along it to each."""
        pts = [points[k][0] for k in keys]
        total, along = 0.0, [0.0]
        for p, q in zip(pts, pts[1:], strict=False):
            total += (q - p).length
            along.append(total)
        return pts, along

    edges = {
        "left": run([(0, j) for j in range(nv + 1)]),
        "right": run([(nu, j) for j in range(nv + 1)]),
        "top": run([(i, 0) for i in range(nu + 1)]),
        "bottom": run([(i, nv) for i in range(nu + 1)]),
    }

    def strip(key, edge):
        """A point's place in the seam strip along an edge: measured from the edge's own vertex in the same column (or
        row) in that vertex's frame, how far along the edge (its tangent there) and how far across, so each corner of a
        band cell is placed by the frame of the edge vertex it hangs from and the cells follow the edge's bends; where
        the band is wider than the texture's seam strip, the plain hide that follows the strip shows."""
        pts, along = edges[edge]
        ci, cj = key
        k = {"left": cj, "right": cj, "top": ci, "bottom": ci}[edge]
        p = points[key][0]
        d = pts[min(k + 1, len(pts) - 1)] - pts[max(k - 1, 0)]
        t = d.normalized() if d.length > 1e-9 else Vector((1.0, 0.0, 0.0))
        offset = p - pts[k]
        along_here = offset.dot(t)
        return ((offset - t * along_here).length, along[k] + along_here)

    def uv_of(key, i, j):
        """The texture coordinates of a corner of the cell (i, j): in the band along an edge, the seam strip (across the
        edge in the strip's width, and along it in metres); elsewhere the field's flat cut."""
        p, _, a, s = points[key]
        # a band cell next to a corner (the corner cell and the one beside it, where two seams meet at any angle) is
        # plain field: a strip laid flat cannot turn the corner without stretching
        in_band = [
            name
            for name, here in (
                ("left", left and i == 0 and (not top or j >= 2) and (not bottom or j <= nv - 3)),
                ("right", right and i == nu - 1 and (not top or j >= 2) and (not bottom or j <= nv - 3)),
                ("top", top and j == 0 and (not left or i >= 2) and (not right or i <= nu - 3)),
                ("bottom", bottom and j == nv - 1 and (not left or i >= 2) and (not right or i <= nu - 3)),
            )
            if here
        ]
        if in_band:
            return strip(key, in_band[0])
        x, y = hide.flat(a, s)
        return (HIDE_FRAME + (x - hide.low[0]), (y - hide.low[1]) + offset)

    def seam_shade(c):
        """1 in the field, falling to 0.78 at a seam's edge across the 6 cm band: the stitched edge folds in, and the
        shadow is the part's own, so every seam is one continuous line and not only the texture's stitches."""
        ci, cj = c
        near = (ci == 0 and left) or (ci == nu and right) or (cj == 0 and top) or (cj == nv and bottom)
        return 0.78 if near else 1.0

    for j in range(nv):
        for i in range(nu):
            corners = [(i, j), (i, j + 1), (i + 1, j + 1), (i + 1, j)]
            uvs = [uv_of(c, i, j) for c in corners]
            shades = [hide.crease(points[c][0]) * seam_shade(c) for c in corners]
            builder.face([verts[c][0] for c in corners], uvs, "hide", shades)
            builder.face(list(reversed([verts[c][1] for c in corners])), list(reversed(uvs)), "hide", [0.35] * 4)


def line(a0, s0, a1, s1):
    """A straight run in (azimuth, slant)."""
    return lambda t: (a0 + (a1 - a0) * t, s0 + (s1 - s0) * t)


def seam_curve(seed, a, s_top, s_bottom):
    """A seam at an azimuth from slant s_top to s_bottom, straying a little round the cone by a smooth wobble."""
    return lambda t: (a + wobble(seed, t), s_top + (s_bottom - s_top) * t)


def along_function(a0, a1, function):
    """A curve of the cone running between two azimuths whose slant is a function of the azimuth."""
    return lambda t: (a0 + (a1 - a0) * t, function(a0 + (a1 - a0) * t))


def polyline(points):
    """A curve through points in (azimuth, slant), each leg taking a share of the run in proportion to its length."""
    legs = [abs(q[1] - p[1]) + 1e-6 for p, q in zip(points, points[1:], strict=False)]
    total = sum(legs)

    def at(t):
        run = t * total
        for k, leg in enumerate(legs):
            if run <= leg or k == len(legs) - 1:
                f = min(1.0, run / leg)
                p, q = points[k], points[k + 1]
                return (p[0] + (q[0] - p[0]) * f, p[1] + (q[1] - p[1]) * f)
            run -= leg
        return points[-1]

    return at


def lower_hide(index):
    """Lower hide `index` (0 to 6): between its two seams, from the tier boundary to the hem; the first holds the
    door."""
    name = f"hide_lower_{index + 1}"
    a0, a1 = rel(LOWER[index]), rel(LOWER[index + 1])
    b = Builder(name)
    h = Hide(b, index, LOWER_TONES[index])
    h.frame = (a0 + a1) / 2.0
    left = seam_curve(index, a0, tier_s(a0), hem_s(a0))
    right = seam_curve((index + 1) % 7, a1, tier_s(a1), hem_s(a1))
    columns = max(3, round((LOWER[index + 1] - LOWER[index]) / 10.0))
    if index > 0:
        curves = (along_function(a0, a1, tier_s), along_function(a0, a1, hem_s), left, right)
        hide_patch(b, h, curves, columns, 6, 0.37 * index, (True, False, True, True))
        return b, h
    # the first lower hide holds the door slit: a hide each side of it and a lintel over it
    ah = math.radians(SLIT_AT_HEM)
    at = math.radians(SLIT_AT_TOP)
    s_top = SLANT * (1.0 - SLIT / HIGH)
    mid = rel(0.0)
    for side in (-1, 1):
        slit = polyline(
            [
                (mid + side * at, tier_s(mid + side * at)),
                (mid + side * at, s_top),
                (mid + side * ah, hem_s(mid + side * ah)),
            ]
        )
        if side < 0:  # left of the slit: the seam is its left edge, the slit's side its right
            curves = (along_function(a0, mid - at, tier_s), along_function(a0, mid - ah, hem_s), left, slit)
        else:
            curves = (along_function(mid + at, a1, tier_s), along_function(mid + ah, a1, hem_s), slit, right)
        hide_patch(b, h, curves, 4, 6, 0.0, (True, False, side < 0, side > 0))
    lintel = (
        along_function(mid - at, mid + at, tier_s),
        line(mid - at, s_top, mid + at, s_top),
        line(mid - at, tier_s(mid - at), mid - at, s_top),
        line(mid + at, tier_s(mid + at), mid + at, s_top),
    )
    hide_patch(b, h, lintel, 1, 2, 0.0, (False, False, False, False))
    return b, h


def upper_hide(index):
    """Upper hide `index` (0 to 3): between its two seams, from the gathered neck to the tier boundary."""
    name = f"hide_upper_{index + 1}"
    a0, a1 = rel(UPPER[index]), rel(UPPER[index + 1])
    b = Builder(name)
    h = Hide(b, 7 + index, UPPER_TONES[index])
    h.frame = (a0 + a1) / 2.0
    left = seam_curve(10 + index, a0, NECK, tier_s(a0))
    right = seam_curve(10 + (index + 1) % 4, a1, NECK, tier_s(a1))
    curves = (line(a0, NECK, a1, NECK), along_function(a0, a1, tier_s), left, right)
    hide_patch(
        b,
        h,
        curves,
        max(4, round((UPPER[index + 1] - UPPER[index]) / 10.0)),
        4,
        0.41 * index,
        (False, True, True, True),
    )
    return b, h


def frame_joint(builder_obj, name, a, s):
    """A joint of the cover at an azimuth and slant distance on the cone, its own z running down the slope and its x
    round the cone."""
    down = Vector((BASE * math.cos(a), BASE * math.sin(a), -HIGH)).normalized()
    x = Vector((-math.sin(a), math.cos(a), 0.0))
    y = down.cross(x)
    matrix = Matrix((x, y, down)).transposed()
    return joint(builder_obj, name, tuple(cone_point(a, s)), tuple(matrix.to_euler("XYZ")))


def hide_parts():
    """The cover's eleven hides as objects, with the joints the other parts meet them at."""
    objects = []
    for index in range(7):
        builder, _ = lower_hide(index)
        obj = builder.build((0.0, 0.0, 0.0))
        if index == 0:
            frame_joint(obj, "door_top", DOOR, SLANT * (1.0 - SLIT / HIGH))
        objects.append(obj)
    for index in range(4):
        builder, _ = upper_hide(index)
        obj = builder.build((0.0, 0.0, 0.0))
        if index == 0:
            joint(obj, "crossing", (0.0, 0.0, CROSS))
        objects.append(obj)
    return objects


def door_flap():
    """The hanging flap: a hide hung from just above the slit's top and pulled aside to its left, as the sheet shows it,
    its free edge lifted out from the cone so the dark opening shows beside it, and its lower quarter turned back up
    over itself, so the pale lining shows in a fold; its hinge joint lies where the cover's door_top does. Its texture
    coordinates are the flat cut of the hide as if it hung straight."""
    b = Builder("tent_door_flap")
    a1 = DOOR - math.radians(5.0)  # its free edge, beside the opening; its outer edge fans out toward the hem
    s0, s1 = SLANT * (1.0 - SLIT / HIGH) - 0.13, SLANT - 0.02
    columns, rows = 4, 8
    turn_at = 0.75  # where the hide is turned back on itself

    def where(x, t):
        """The azimuth, slant distance and lift of the hanging hide at a fraction across (from its outer edge to its
        free one) and a fraction down."""
        a = DOOR - math.radians(16.0 + 18.0 * t) + (a1 - (DOOR - math.radians(16.0 + 18.0 * t))) * x
        s = s0 + (s1 - 0.12 * (1.0 - x) - s0) * t  # the outer corner is cut short and the free corner hangs lowest
        fold = 0.07 * max(0.0, math.sin(math.pi * (0.55 * x + 0.85 * t - 0.15))) * t  # one broad diagonal fold
        lift = 0.015 + 0.08 * t**1.6 + 0.12 * x**1.4 * t**1.2 + 0.20 * x**3 * t**3 + fold  # a lifted lower corner
        return a, s, lift

    grid = {}
    frame = DOOR
    lows = [1e9, 1e9]
    for j in range(rows + 1):
        t = j / rows
        for i in range(columns + 1):
            x = i / columns
            a, s_flat, lift = where(x, t)
            s = s_flat
            if t > turn_at:  # the turned-back part lies over the hide above it, a little off it, as wide as it was
                _, s, lift = where(x, 2.0 * turn_at - t)
                lift += 0.03
            p = cone_point(a, s) + lift * outward(a)
            a_flat = a
            alpha = (a_flat - frame) * BASE / SLANT
            flat = (s_flat * math.sin(alpha), s_flat * math.cos(alpha))
            lows = [min(lows[0], flat[0]), min(lows[1], flat[1])]
            grid[(i, j)] = (p, a, flat, lift)
    verts = {k: (b.bm.verts.new(v[0]), b.bm.verts.new(v[0] - THICK * outward(v[1]))) for k, v in grid.items()}
    for j in range(rows):
        for i in range(columns):
            corners = [(i, j), (i, j + 1), (i + 1, j + 1), (i + 1, j)]
            uvs = [(HIDE_FRAME + grid[c][2][0] - lows[0], grid[c][2][1] - lows[1] + 0.9) for c in corners]
            crease = [0.80 + 0.20 * min(1.0, grid[c][3] / 0.35) for c in corners]  # the light hide, lighter as it lifts
            b.face([verts[c][0] for c in corners], uvs, "hide", crease)
            # the lining, which the turned-back part shows: pale suede
            b.face(list(reversed([verts[c][1] for c in corners])), list(reversed(uvs)), "hide", [0.85] * 4)
    obj = b.build((0.0, 0.0, 0.0))
    frame_joint(obj, "hinge", DOOR, SLANT * (1.0 - SLIT / HIGH))
    return obj


# the poles: 3.65 m, thicker at the foot and stout at the tip, the tips 3.06 m up when leaned in the tent
POLE_LENGTH = 3.65
POLE_BIND = 3.22


def pole():
    """A pole standing along z, its foot at the origin, a little knotted, thinner toward its tip: joints at its foot,
    where it crosses the others, and its tip."""
    b = Builder("tent_pole")
    rings = 14
    profile = [(POLE_LENGTH * k / (rings - 1), 0.0340 - 0.0070 * (k / (rings - 1))) for k in range(rings)]
    lathe(b, "z", profile, 8, "wood", lambda p, n: 0.70 + 0.30 * min(1.0, p.z / 0.8), bumps=0.04, seed=11)
    obj = b.build((0.0, 0.0, 0.0))
    joint(obj, "foot", (0.0, 0.0, 0.0))
    joint(obj, "bind", (0.0, 0.0, POLE_BIND))
    joint(obj, "tip", (0.0, 0.0, POLE_LENGTH))
    return obj


def binding():
    """The lashing where the poles cross: a short thick band of rawhide round the bundle, its joint in its middle."""
    b = Builder("tent_binding")
    profile = [(0.0, 0.090), (0.05, 0.093), (0.10, 0.093), (0.15, 0.090)]
    lathe(b, "z", profile, 12, "hide", lambda p, n: 0.78, bumps=0.012, seed=9)
    obj = b.build((0.0, 0.0, 0.0))
    joint(obj, "centre", (0.0, 0.0, 0.075))
    return obj


def river_stone(name, seed, half_x, half_y, high):
    """A river stone half buried, rounder than the stand-ins': a lump of 320 smooth-shaded faces, its bottom on z = 0,
    its form leaning a little off round by a few slow swells, not by noise in every corner; each face has its own flat
    texture projection along its own normal, which carries a texture pixel over without stretch, and its crease
    darkens it toward the ground."""
    b = Builder(name)
    bmesh.ops.create_icosphere(b.bm, subdivisions=3, radius=1.0)
    for v in b.bm.verts:
        x, y, z = v.co
        lump = 1.0 + 0.10 * math.sin(2.3 * x + seed) * math.cos(1.7 * y + 2.0 * seed) + 0.05 * math.sin(3.1 * z + seed)
        lump += 0.05 * math.sin(5.3 * y + 3.0 * seed) * math.cos(4.1 * x)
        v.co = Vector((x * half_x * lump, y * half_y * lump, max(0.0, z) * high * lump))
    b.bm.normal_update()
    for f in b.bm.faces:
        f.smooth = True
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
    return b.build((0.0, 0.0, 0.0))


def stones():
    """Three river stones, half buried, 25 to 45 cm across: a small, a medium and a large one."""
    return [
        river_stone("ring_stone_small", 21, 0.125, 0.095, 0.075),
        river_stone("ring_stone_medium", 22, 0.175, 0.135, 0.100),
        river_stone("ring_stone_large", 23, 0.225, 0.170, 0.130),
    ]


def make_parts():
    """Every part of the tent, in a list."""
    return [*hide_parts(), door_flap(), pole(), binding(), *stones()]
