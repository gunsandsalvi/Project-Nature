"""The hide screen's parts for the camp family (sheet 16.12, A6.1, A6.3, A6.4, T2.3c.2): a curtain of six sewn hides
hung from a ridge pole between two forked posts across the front of a rock shelter, 5 m wide and 2.3 m high, its foot
weighed with eleven stones, with a door flap. Imported by tools/art/parts_camp.py, which makes the file; run inside
Blender. Implements PRE-46 and PRE-22.

The screen's own metres: x along it (its middle on the origin, so the design's x of 0 to 5 is here -2.5 to 2.5), z up,
y from the outside to the inside: the outside, which the sheet draws, is -y (the south, as the tent's door is), the
posts stand at y = 0.05, the pole lies against the front of their forks at y = -0.04, the curtain hangs at y = -0.02
under it (leather 4 mm thick) and the foot stones lie outside it. Every part lies where it belongs, so the recipe lays
each at the origin (a root); the scene turns the whole thing to close the mouth of the rock shelter, its outside
toward the open ground.

The parts, as the sheet's design lists them:
- `screen_hide_a` to `screen_hide_f`: the six unequal hides, each two skins (outer and inner) over the outline the
  design's seams give (nine meandering seams, four three-way junctions, no cross; the seams eased so a sewn strip can
  follow them), meshed as a triangulation of the outline with a point every 12 cm inside, hung from the six ties in
  folds that fan down from each of them and sag in scallops between, its foot lobed and ragged; each hide wears its own
  colour, one of the hide textures (the tent's, and its golden, greyish buff, brown and rusty forms), darkened to the
  hide's tone by the part's crease, which also carries the folds' light and shade;
- `screen_seams`: the nine seams' sewn strips, each a ribbon 6.25 cm wide along the seam on the side of the hide that
  laps over the other, in that hide's texture, whose seam strip (a dark edge and running stitches of sinew) it takes;
- `screen_flap`: the door flap, one deer hide hung from its top at z = 1.53, 1.5 m tall and 0.8 m wide, its foot lobed,
  hung 3.5 cm in front of the curtain with stitches down its sides;
- `screen_post_left`, `screen_post_right`: the forked posts, 2.4 m long with 0.1 m in the ground, the tines splayed 17
  degrees along the pole, the outer one the longer;
- `screen_pole`: the ridge pole, 5 m long and 0.1 m thick;
- `screen_ties`: the six rawhide ties, each a band of turns round the pole, a knot and a short end, and the two thick
  lashings that bind the pole into its forks;
- `screen_stones`: the eleven foot stones as one part, faceted boulders of granite, the design's widths and heights
  (0.24 to 0.40 m, 0.16 to 0.25 m) made a seventh larger as the sheet's picture shows them.

Forms by size on screen (A6.3), each from the same numbers as the full part: `_simple` (the hides meshed coarsely, flat
and without the seams' strips, the posts, pole, flap and stones lighter, no ties) and `_marker`
(`screen_curtain_marker`, the six hides as flat outlines in one part, `screen_flap_marker`, the posts, pole and stones
as the smallest prisms and lumps).
"""

import math

import bmesh
from mathutils import Vector
from mathutils.geometry import delaunay_2d_cdt
from standins import Builder, lathe, noise

import parts_things as things

FORMS = ("", "_simple", "_marker")
SEAM = 4.0 / 64.0  # the hide texture's seam strip, 4 texture pixels wide, in metres
HIDE_FRAME = 0.0625  # the hide texture's field starts after the seam strip's first 4 columns
MIDDLE = 2.5  # the design's x of the screen's middle
THICK = 0.004  # the leather's thickness
LIFT = 0.006  # a sewn seam's welt stands this far off the hides it joins
TOP = 2.15  # where the curtain hangs from the ties
TIES = [0.60, 1.37, 2.15, 2.92, 3.69, 4.47]  # the six rawhide suspension positions (the standard's), design x
# the hides' textures and tones: the sheet draws the six in golden, greyish buff, warm brown, mid brown, light buff and
# rusty colours; the tent's hide (the light buff) and its golden, greyish buff, brown and rusty forms
# (art/textures/hide_gold, hide_buff, hide_brown and hide_rust, moved to a piece of the standard's colours or to its
# palette's) are those, and each hide's tone, which the crease gives, is its lightness among them (the engine's sun
# lifts every colour a little, so the lightest hide stays under 1)
ROLES = {"a": "hide_gold", "b": "hide_buff", "c": "hide_brown", "d": "hide_brown", "e": "hide", "f": "hide_rust"}
TONES = {"a": 0.88, "b": 0.95, "c": 0.90, "d": 0.76, "e": 0.95, "f": 0.88}
FLAP_TONE = 0.90  # the flap's pale deer hide, which shows on the rusty hide behind it
STEP = [0.14, 0.30, 0.45]  # the outline's point spacing in each form
INSIDE = [0.12, 0.45, 0.0]  # the spacing of the points inside a hide in each form (none in the marker)


def lx(x):
    """A design x as the part's x."""
    return x - MIDDLE


# ---- the outline of the curtain ----------------------------------------------------------------------------------


def top_z(x):
    """The curtain's top edge: at the ties it hangs from the pole, between them it sags in a scallop up to 16 cm; past
    the outer ties it falls away a little."""
    if x <= TIES[0]:
        return TOP - 0.02 * (TIES[0] - x) / 0.2
    if x >= TIES[-1]:
        return TOP - 0.02 * (x - TIES[-1]) / 0.2
    for a, b in zip(TIES, TIES[1:], strict=False):
        if x <= b:
            return TOP - 0.16 * math.sin(math.pi * (x - a) / (b - a)) ** 0.8
    return TOP


def foot_z(x):
    """The foot, ragged: low on the ground in the stones' places, lifting in lobes between them."""
    return 0.03 + 0.045 * (1.0 + math.sin(9.0 * x + 0.5)) / 2.0 + 0.04 * max(0.0, math.sin(4.3 * x + 2.0))


def bump(z, centre, width):
    return math.exp(-(((z - centre) / width) ** 2))


def left_x(z):
    """The left edge, with a neck lobe at the upper left and a short leg lobe near 0.9 m."""
    return 0.25 + 0.02 * math.sin(7.0 * z + 1.0) - 0.07 * bump(z, 1.95, 0.18) - 0.05 * bump(z, 0.9, 0.15)


def right_x(z):
    """The right edge, with a long outer leg lobe near 1.3 m and a ragged neck remnant high up."""
    return 4.75 + 0.02 * math.sin(6.0 * z) + 0.06 * bump(z, 1.3, 0.22) + 0.05 * bump(z, 1.95, 0.12)


def corner(x_of_z, z_of_x, z0):
    """Where an edge x(z) meets the top or foot z(x): the point on both."""
    z = z0
    for _ in range(8):
        z = z_of_x(x_of_z(z))
    return Vector((x_of_z(z), z))


def catmull(points, step):
    """A smooth path through the points, sampled about every `step` metres, ends exact (uniform Catmull-Rom)."""
    pts = [points[0] * 2.0 - points[1], *points, points[-1] * 2.0 - points[-2]]
    out = []
    for i in range(1, len(pts) - 2):
        p0, p1, p2, p3 = pts[i - 1 : i + 3]
        n = max(2, math.ceil((p2 - p1).length / step))
        for k in range(n):
            t = k / n
            out.append(
                0.5
                * (
                    2.0 * p1
                    + (p2 - p0) * t
                    + (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t * t
                    + (3.0 * p1 - p0 - 3.0 * p2 + p3) * t**3
                )
            )
    out.append(points[-1].copy())
    return out


def tightest_bend(path):
    """The smallest radius of curvature along a path (through each three points in a row), in metres."""
    worst = 1e9
    for a, b, c in zip(path, path[1:], path[2:], strict=False):
        area2 = abs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x))
        if area2 > 1e-12:
            worst = min(worst, (b - a).length * (c - b).length * (c - a).length / (2.0 * area2))
    return worst


def relaxed(path, radius=0.30, most=40):
    """A path with its bends eased (each point moved half way toward the middle of its neighbours, round after round,
    as few rounds as it takes), ends exact, until no bend is tighter than `radius`: a ribbon of the hide texture's seam
    strip, 6 cm wide, laid along a bend stretches by the bend's width over its radius, and the kit allows one and a half
    to one."""
    pts = [p.copy() for p in path]
    for _ in range(most):
        if tightest_bend(pts) >= radius:
            break
        pts = [pts[0], *[(pts[k - 1] + pts[k + 1]) * 0.25 + pts[k] * 0.5 for k in range(1, len(pts) - 1)], pts[-1]]
    return pts


def edge(p0, p1, curve, step):
    """An edge from p0 to p1 whose points between follow `curve(t)` (t from 0 to 1 along the run), ends exact."""
    n = max(2, math.ceil((p1 - p0).length / step))
    return [p0.copy(), *[curve(k / n) for k in range(1, n)], p1.copy()]


def outline(form):
    """The six hides' outlines as lists of points (design x, z), counter-clockwise seen from outside, and the nine
    seams as lists from their start to their end."""
    step = STEP[form]
    v = lambda x, z: Vector((x, z))  # noqa: E731
    tl = corner(left_x, top_z, 2.1)
    tr = corner(right_x, top_z, 2.1)
    bl = corner(left_x, foot_z, 0.1)
    br = corner(right_x, foot_z, 0.1)
    l1 = v(left_x(0.95), 0.95)
    r1 = v(right_x(0.90), 0.90)
    t1, t2 = v(1.40, top_z(1.40)), v(3.40, top_z(3.40))
    f1, f2 = v(1.80, foot_z(1.80)), v(3.20, foot_z(3.20))
    j1, j2, j3, j4 = v(1.45, 1.25), v(2.10, 0.83), v(3.10, 1.35), v(3.55, 0.75)
    seams = {
        "s1": [l1, v(0.70, 1.10), v(1.00, 0.88), j1],
        "s2": [j1, v(1.25, 1.60), v(1.65, 1.80), t1],
        "s3": [j1, v(1.80, 1.15), v(1.70, 0.97), j2],
        "s4": [j2, v(1.90, 0.60), v(2.05, 0.35), f1],
        "s5": [j2, v(2.40, 0.95), v(2.70, 1.20), j3],
        "s6": [j3, v(3.00, 1.60), v(3.30, 1.85), t2],
        "s7": [j3, v(3.50, 1.10), v(3.35, 0.88), j4],
        "s8": [j4, v(4.10, 0.60), v(4.45, 0.90), r1],
        "s9": [j4, v(3.50, 0.40), v(3.15, 0.25), f2],
    }
    seams = {
        name: relaxed(catmull(points, 0.12)) if step < 0.2 else catmull(points, step) for name, points in seams.items()
    }

    def along_top(a, b):
        return edge(a, b, lambda t: v(a.x + (b.x - a.x) * t, top_z(a.x + (b.x - a.x) * t)), step)

    def along_foot(a, b):
        return edge(a, b, lambda t: v(a.x + (b.x - a.x) * t, foot_z(a.x + (b.x - a.x) * t)), step)

    def along_left(a, b):
        return edge(a, b, lambda t: v(left_x(a.y + (b.y - a.y) * t), a.y + (b.y - a.y) * t), step)

    def along_right(a, b):
        return edge(a, b, lambda t: v(right_x(a.y + (b.y - a.y) * t), a.y + (b.y - a.y) * t), step)

    def chain(*pieces):
        out = []
        for piece in pieces:
            for p in piece:
                if not out or (p - out[-1]).length > 1e-9:
                    out.append(p)
        if (out[0] - out[-1]).length < 1e-9:
            out.pop()
        return out

    def rev(piece):
        return piece[::-1]

    s = seams
    loops = {
        "a": chain(s["s1"], s["s2"], rev(along_top(tl, t1)), rev(along_left(l1, tl))),
        "b": chain(s["s3"], s["s5"], s["s6"], rev(along_top(t1, t2)), rev(s["s2"])),
        "c": chain(s["s7"], s["s8"], along_right(r1, tr), rev(along_top(t2, tr)), rev(s["s6"])),
        "d": chain(rev(along_left(bl, l1)), along_foot(bl, f1), rev(s["s4"]), rev(s["s3"]), rev(s["s1"])),
        "e": chain(along_foot(f1, f2), rev(s["s9"]), rev(s["s7"]), rev(s["s5"]), s["s4"]),
        "f": chain(along_foot(f2, br), along_right(br, r1), rev(s["s8"]), s["s9"]),
    }
    for name, loop in loops.items():
        area = sum(p.x * q.y - q.x * p.y for p, q in zip(loop, [*loop[1:], loop[0]], strict=True))
        assert area > 0.0, f"hide {name} is not counter-clockwise ({area})"
    return loops, seams


# which hide laps each seam, and whether the seam, as listed, runs counter-clockwise round that hide (so the hide's
# inside is on the left of the way along it): the loops in `outline` take s1, s3, s5 and s6 as listed, the others
# reversed
SEAM_LAPPER = {"s1": "a", "s2": "b", "s3": "b", "s4": "d", "s5": "b", "s6": "b", "s7": "e", "s8": "f", "s9": "e"}
SEAM_FORWARD = {
    "s1": True,
    "s2": False,
    "s3": True,
    "s4": False,
    "s5": True,
    "s6": True,
    "s7": False,
    "s8": False,
    "s9": False,
}


# ---- the curtain's surface -----------------------------------------------------------------------------------------


def fan_folds(x, z):
    """The folds of a hide gathered into its ties, in -1 to 1: from each tie the cloth falls in folds that run nearly
    straight down and spread apart as they fall (a ridge every 40 cm at the tie, every 70 cm a metre below it), dying
    out with the distance down and to the side, fading in over the first 15 cm below the tie and out above the foot."""
    total = 0.0
    for k, t in enumerate(TIES):
        dz = TOP - 0.02 - z
        if dz <= 0.0:
            continue
        spread = 0.40 + 0.30 * dz
        side = math.exp(-(((x - t) / 0.55) ** 2)) * max(0.0, min(1.0, (z - 0.25) / 0.35))
        total += math.exp(-dz / 1.6) * side * min(1.0, dz / 0.15) * math.sin(2.0 * math.pi * (x - t) / spread + 1.7 * k)
    return max(-1.0, min(1.0, 1.4 * total))


def drape(x, z, detail=1.0):
    """Where the curtain hangs (y): soft vertical folds, deeper toward the foot, the folds that fan down from the ties,
    and a slight bulge over the stones; and how far its shade is up or down by the folds (-1 to 1). `detail` 0 is the
    curtain without its folds, for the forms whose meshes are too coarse to hold them."""
    h = (TOP - z) / TOP
    if detail <= 0.0:
        return -0.02 - 0.02 * h**3, 0.0
    amp = 0.020 + 0.030 * h
    f = 0.6 * math.sin(2.0 * math.pi * x / 0.62 + 0.8 + 0.25 * math.sin(1.7 * x)) + 0.4 * math.sin(
        2.0 * math.pi * x / 1.07 + 2.1
    )
    gather = 0.025 * (1.0 - h) ** 2 * sum(math.exp(-(((x - t) / 0.10) ** 2)) for t in TIES)  # pulled in at each tie
    fan = fan_folds(x, z)
    return -0.02 - amp * f - 0.02 * h**3 + gather - 0.030 * fan, 0.25 * f + 1.0 * fan


def skin_crease(tone, x, z, f):
    """Baked darkening: the hide's tone, darker toward the ground and in the valleys of its folds."""
    return tone * (0.80 + 0.20 * min(1.0, z / TOP)) * (0.32 + 0.68 * (0.5 + 0.5 * max(-1.0, min(1.0, f))))


def in_polygon(p, loop):
    inside = False
    for a, b in zip(loop, [*loop[1:], loop[0]], strict=True):
        if (a.y > p.y) != (b.y > p.y) and p.x < a.x + (p.y - a.y) / (b.y - a.y) * (b.x - a.x):
            inside = not inside
    return inside


def distance_to_loop(p, loop):
    best = 1e9
    for a, b in zip(loop, [*loop[1:], loop[0]], strict=True):
        ab = b - a
        t = max(0.0, min(1.0, (p - a).dot(ab) / max(ab.dot(ab), 1e-12)))
        best = min(best, (p - (a + ab * t)).length)
    return best


def inside_points(loop, spacing, seed):
    """Points inside an outline on a jittered grid, none closer to the edge than 0.6 of the spacing."""
    if spacing <= 0.0:
        return []
    xs, zs = [p.x for p in loop], [p.y for p in loop]
    out = []
    for i in range(math.floor(min(xs) / spacing), math.ceil(max(xs) / spacing) + 1):
        for j in range(math.floor(min(zs) / spacing), math.ceil(max(zs) / spacing) + 1):
            p = Vector(
                ((i + 0.5 * (j % 2) + 0.3 * noise(seed, i, j)) * spacing, (j + 0.3 * noise(seed, i, j + 977)) * spacing)
            )
            if in_polygon(p, loop) and distance_to_loop(p, loop) > 0.6 * spacing:
                out.append(p)
    return out


def triangulate(loop, interior):
    """The outline's triangles with the points inside it: (the points, the triangles as index triples, each seen from
    outside counter-clockwise)."""
    coords = [Vector(p) for p in [*loop, *interior]]
    verts, _, faces, _, _, _ = delaunay_2d_cdt(coords, [], [list(range(len(loop)))], 1, 1e-5)
    triangles = []
    for f in faces:
        a, b, c = (verts[i] for i in f)
        cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)
        triangles.append(list(f) if cross > 0.0 else [f[0], f[2], f[1]])
    return verts, triangles


def skin(builder, loop, name, form, seed, inner, v_shift=0.0):
    """One hide hung: the outer skin and (in the forms that have one) the inner skin of the triangulated outline, each
    corner at the curtain's drape, the hide texture's field laid flat (metres) with the hide's own offset."""
    tone = TONES[name]
    detail = 1.0 if form == 0 else 0.0
    verts, triangles = triangulate(loop, inside_points(loop, INSIDE[form], seed))
    x_min = min(p.x for p in verts)
    outer, behind = [], []
    for p in verts:
        y, _ = drape(p.x, p.y, detail)
        outer.append(builder.bm.verts.new(Vector((lx(p.x), y, p.y))))
        behind.append(builder.bm.verts.new(Vector((lx(p.x), y + THICK, p.y))) if inner else None)
    for tri in triangles:
        pts = [verts[i] for i in tri]
        uvs = [(HIDE_FRAME + (p.x - x_min), TOP - p.y + v_shift) for p in pts]
        creases = [skin_crease(tone, p.x, p.y, drape(p.x, p.y, detail)[1]) for p in pts]
        builder.face([outer[i] for i in tri], uvs, ROLES[name], creases)
        if inner:
            builder.face(
                [behind[i] for i in reversed(tri)],
                list(reversed(uvs)),
                ROLES[name],
                [0.85 * c for c in reversed(creases)],
            )


def hide(name, form):
    loops, _ = outline(form)
    b = Builder(f"screen_hide_{name}{FORMS[form]}")
    skin(b, loops[name], name, form, 300 + ord(name), form < 2, 0.37 * "abcdef".index(name))
    return b.build((0.0, 0.0, 0.0))


def ribbon(builder, path, inside, width, surface, role="hide"):
    """A ribbon along a path of (design x, z) points on the side `inside` says (+1 left of the way along it, -1 right),
    `width` wide, with the hide texture's seam strip across it (its dark edge on the path) and metres along it.
    `surface(point)` gives a point's y and crease."""
    here = []
    for k, p in enumerate(path):
        t = (path[min(k + 1, len(path) - 1)] - path[max(k - 1, 0)]).normalized()
        here.append((p, p + Vector((-t.y, t.x)) * inside * width))
    high = [[surface(point) for point in pair] for pair in here]  # (y, crease) of each side of each cross-cut
    run = [0.0]
    for k, (a, b) in enumerate(zip(path, path[1:], strict=False)):
        run.append(run[-1] + math.hypot((b - a).length, high[k + 1][0][0] - high[k][0][0]))
    across = [math.hypot(width, h[1][0] - h[0][0]) for h in high]
    corners = []
    for pair, heights in zip(here, high, strict=True):
        corners.append(
            [
                (builder.bm.verts.new(Vector((lx(point.x), y, point.y))), crease)
                for point, (y, crease) in zip(pair, heights, strict=True)
            ]
        )
    for k in range(len(path) - 1):
        quad = [corners[k][0], corners[k + 1][0], corners[k + 1][1], corners[k][1]]
        uvs = [(0.0, run[k]), (0.0, run[k + 1]), (across[k + 1], run[k + 1]), (across[k], run[k])]
        flat = [path[k], path[k + 1], here[k + 1][1], here[k][1]]
        if sum(p.x * q.y - q.x * p.y for p, q in zip(flat, [*flat[1:], flat[0]], strict=True)) < 0.0:
            quad, uvs = quad[::-1], uvs[::-1]
        builder.face([c[0] for c in quad], uvs, role, [c[1] for c in quad])


def seams():
    """The nine seams' ribbons in one part, each on the side of the hide that laps."""
    _, paths = outline(0)
    b = Builder("screen_seams")
    for name, path in paths.items():
        tone = TONES[SEAM_LAPPER[name]] * 0.92

        def surface(p, tone=tone):
            y, f = drape(p.x, p.y)
            return y - LIFT, skin_crease(tone, p.x, p.y, f)

        ribbon(b, path, 1.0 if SEAM_FORWARD[name] else -1.0, SEAM, surface, ROLES[SEAM_LAPPER[name]])
    return b.build((0.0, 0.0, 0.0))


# ---- the flap ------------------------------------------------------------------------------------------------------


def flap(form):
    """The door flap: a deer hide hung from its top at z = 1.53, 1.5 m tall and 0.8 m wide, its foot tapering and lobed,
    hung a little outside the curtain in soft vertical folds, swinging out toward its foot."""
    v = lambda x, z: Vector((x, z))  # noqa: E731
    step = STEP[form]
    pts = [
        v(3.55, 1.53),
        v(3.53, 1.20),
        v(3.57, 0.80),
        v(3.62, 0.45),
        v(3.68, 0.20),
        v(3.80, 0.10),
        v(3.90, 0.17),
        v(4.00, 0.07),
        v(4.12, 0.14),
        v(4.22, 0.22),
        v(4.28, 0.45),
        v(4.33, 0.80),
        v(4.37, 1.20),
        v(4.35, 1.53),
    ]
    side = catmull(pts, step)
    top = [v(4.35 - 0.8 * k / 8.0, 1.53) for k in range(1, 8)]
    loop = [*side, *top]
    verts, triangles = triangulate(loop, inside_points(loop, INSIDE[form], 41))
    b = Builder(f"screen_flap{FORMS[form]}")

    def place(p, back):
        t = (1.53 - p.y) / 1.5  # 0 at the top, 1 at the foot
        fold = 0.024 * math.sin(2.0 * math.pi * (p.x - 3.55) / 0.34 + 0.4) * (0.3 + 0.7 * t)
        # hung 3.5 cm in front of the curtain wherever the curtain's folds put it, swinging out toward its foot
        y = (
            drape(p.x, p.y, 1.0 if form == 0 else 0.0)[0]
            - 0.035
            - 0.030 * t**1.5
            - 0.5 * fold
            + (THICK if back else 0.0)
        )
        return Vector((lx(p.x), y, p.y)), 0.80 + 0.20 * (
            0.5 + 0.5 * math.sin(2.0 * math.pi * (p.x - 3.55) / 0.34 + 0.4)
        )

    outer = [b.bm.verts.new(place(p, False)[0]) for p in verts]
    inner = [b.bm.verts.new(place(p, True)[0]) for p in verts] if form < 2 else None
    for tri in triangles:
        pts3 = [verts[i] for i in tri]
        uvs = [(HIDE_FRAME + (p.x - 3.55), 1.53 - p.y + 0.6) for p in pts3]
        crease = [FLAP_TONE * place(p, False)[1] for p in pts3]
        b.face([outer[i] for i in tri], uvs, "hide", crease)
        if inner:
            b.face([inner[i] for i in reversed(tri)], list(reversed(uvs)), "hide", [0.85 * c for c in reversed(crease)])
    if form == 0:  # the edge stitches, small and dim, down both sides (the lobed foot has none)

        def surface(p):
            at, f = place(p, False)
            return at.y - 0.004, FLAP_TONE * 0.92 * f

        ribbon(b, catmull(pts[1:5], step), 1.0, SEAM, surface)
        ribbon(b, catmull(pts[10:13], step), 1.0, SEAM, surface)
    return b.build((0.0, 0.0, 0.0))


# ---- the frame: posts, pole, ties ------------------------------------------------------------------------------------

POST_X = (0.20, 4.80)  # the posts' centres, design x
POST_Y = 0.05
FORK = 1.93  # where a post forks: the trunk ends here, its tines seat the pole at z = 2.20
POLE_Z = 2.20
POLE_Y = (
    POST_Y - 0.09
)  # the pole lies against the front of the forks (the outside), lashed to them, the curtain hanging
# straight under it
WOOD = lambda p, n: 0.70 + 0.30 * min(1.0, max(0.0, p.z) / 0.8)  # noqa: E731


def turn_about_y(builder, first, degrees, about):
    """The vertices of `builder` from index `first` on turned about the y axis through `about`."""
    c, s = math.cos(math.radians(degrees)), math.sin(math.radians(degrees))
    for vert in list(builder.bm.verts)[first:]:
        x, z = vert.co.x - about.x, vert.co.z - about.z
        vert.co.x, vert.co.z = about.x + x * c + z * s, about.z - x * s + z * c


def post(side, form):
    """A forked post, 2.4 m long with 0.1 m in the ground, the outer tine the longer; side -1 left, +1 right."""
    name = "left" if side < 0 else "right"
    segments = (8, 6, 4)[form]
    b = Builder(f"screen_post_{name}{FORMS[form]}")
    rings = [(-0.10, 0.050), (0.45, 0.052), (1.05, 0.049), (1.55, 0.047), (FORK, 0.045)]
    if form:
        rings = [rings[0], rings[2], rings[-1]]
    lathe(b, "z", rings, segments, "wood", WOOD, bumps=0.04, seed=33 + side, caps=(True, True))
    for tine_side, length in ((-1, 0.39 if side < 0 else 0.30), (1, 0.30 if side < 0 else 0.39)):
        first = len(b.bm.verts)
        profile = [(0.0, 0.038), (length * 0.5, 0.033), (length, 0.026)]
        if form == 2:
            profile = [profile[0], profile[-1]]
        lathe(b, "z", profile, segments, "wood", lambda p, n: 0.80, bumps=0.05, seed=35 + tine_side, caps=(False, True))
        turn_about_y(b, first, 17.0 * tine_side, Vector((0.0, 0.0, 0.0)))
        for vert in list(b.bm.verts)[first:]:
            vert.co += Vector((0.020 * tine_side, 0.0, FORK))
    for vert in b.bm.verts:
        vert.co += Vector((lx(POST_X[0 if side < 0 else 1]), POST_Y, 0.0))
    return b.build((0.0, 0.0, 0.0))


def pole(form):
    """The ridge pole, 5 m long and 0.1 m thick, slightly irregular, no sag."""
    b = Builder(f"screen_pole{FORMS[form]}")
    count = (14, 5, 3)[form]
    profile = [(lx(5.0 * k / (count - 1)), 0.050 * (1.0 + 0.03 * noise(37, k, 1))) for k in range(count)]
    lathe(b, "x", profile, (8, 6, 4)[form], "wood", lambda p, n: 0.78, bumps=0.03, seed=37, caps=(True, True))
    for vert in b.bm.verts:
        vert.co += Vector((0.0, POLE_Y, POLE_Z))
    return b.build((0.0, 0.0, 0.0))


def blob(builder, centre, radii, crease):
    """A small lump (a knot): an icosphere of one subdivision stretched to `radii`, flat-shaded, in the hide's role."""
    geom = bmesh.ops.create_icosphere(builder.bm, subdivisions=1, radius=1.0)
    verts = [g for g in geom["verts"]]
    for vert in verts:
        vert.co = Vector(
            (centre.x + vert.co.x * radii[0], centre.y + vert.co.y * radii[1], centre.z + vert.co.z * radii[2])
        )
    builder.bm.normal_update()
    things.own_plane(builder, list({f for vert in verts for f in vert.link_faces}), "hide", crease)


def ties():
    """The six rawhide ties: a band of turns round the pole, a knot on the outside and a short end hanging."""
    b = Builder("screen_ties")
    for tx in TIES:
        x = lx(tx)
        first = len(b.bm.verts)  # the turns as one band 5 cm wide
        lathe(
            b,
            "x",
            [(x - 0.025, 0.057), (x + 0.025, 0.057)],
            8,
            "hide",
            lambda p, n: 0.50,
            caps=(False, False),
        )
        for vert in list(b.bm.verts)[first:]:
            vert.co += Vector((0.0, POLE_Y, POLE_Z))
        blob(b, Vector((x, POLE_Y - 0.064, POLE_Z - 0.012)), (0.020, 0.016, 0.018), 0.42)
        top, bottom, half = POLE_Z - 0.02, POLE_Z - 0.15, 0.007
        y = POLE_Y - 0.066
        quad = [
            Vector((x - half, y, bottom)),
            Vector((x + half, y, bottom)),
            Vector((x + half, y, top)),
            Vector((x - half, y, top)),
        ]
        b.face(quad, [(p.x, p.z) for p in quad], "hide", [0.42] * 4)
    for post_x in POST_X:  # the two thick lashings that bind the pole into its forks, a band and a knot each
        x = lx(post_x)
        first = len(b.bm.verts)
        lathe(b, "x", [(x - 0.055, 0.064), (x + 0.055, 0.064)], 8, "hide", lambda p, n: 0.46, caps=(False, False))
        for vert in list(b.bm.verts)[first:]:
            vert.co += Vector((0.0, POLE_Y, POLE_Z))
        blob(b, Vector((x, POLE_Y - 0.072, POLE_Z - 0.02)), (0.032, 0.022, 0.026), 0.40)
    return b.build((0.0, 0.0, 0.0))


# ---- the foot stones -------------------------------------------------------------------------------------------------

# centre x, width, depth, height (the design's lists), the shape and the stone's role and tone
FOOT = [
    (0.38, 0.24, 0.20, 0.16, "angular", "granite", 0.88),
    (0.77, 0.31, 0.26, 0.22, "wedge", "granite", 0.84),
    (1.17, 0.21, 0.20, 0.18, "angular", "granite", 0.90),
    (1.60, 0.37, 0.32, 0.25, "angular", "granite", 0.92),
    (2.04, 0.28, 0.24, 0.19, "trapezoid", "granite", 0.86),
    (2.48, 0.34, 0.30, 0.23, "angular", "granite", 0.88),
    (2.89, 0.22, 0.21, 0.16, "wedge", "granite", 0.90),
    (3.30, 0.40, 0.32, 0.25, "angular", "granite", 0.92),
    (3.70, 0.26, 0.24, 0.20, "angular", "granite", 0.86),
    (4.14, 0.32, 0.28, 0.24, "trapezoid", "granite", 0.88),
    (4.57, 0.23, 0.22, 0.17, "angular", "granite", 0.90),
]
# broad facets: 12 points round the foot and 10 over the crown, rounded over like a heavy boulder (the full form)
FACETS = (10, 9, (0.30, 0.30), (0.78, 0.22), True)


STONE_SCALE = (
    1.15,
    1.15,
    1.10,
)  # the design's widths, depths and heights made larger, as the sheet's picture shows them


def stones(form):
    """The eleven stones in one part, on the ground at y = -0.10 in front of the curtain's foot."""
    b = Builder(f"screen_stones{FORMS[form]}")
    for k, (x, w, d, h, kind, role, tone) in enumerate(FOOT):
        spec = (k + 1, w * STONE_SCALE[0], d * STONE_SCALE[1], h * STONE_SCALE[2], kind, role, tone)
        one = things.shape_stone("stone", spec, form, 90 + k, domes={}, facets=FACETS)
        things.merge(b, one, Vector((lx(x), -0.10, 0.0)))
        one.bm.free()
    return b.build((0.0, 0.0, 0.0))


# ---- the curtain at the smallest zoom ---------------------------------------------------------------------------


def curtain_marker():
    """The six hides and the flap as flat outlines in one part, each its tone, at the curtain's own surface: a few
    triangles, for the camp and the close camp (bands 3 and 4)."""
    loops, _ = outline(2)
    b = Builder("screen_curtain_marker")
    for k, name in enumerate("abcdef"):
        skin(b, loops[name], name, 2, 300 + ord(name), False, 0.37 * k)
    return b.build((0.0, 0.0, 0.0))


def make_parts():
    """Every part of the screen, in its three forms."""
    out = []
    for form in (0, 1):
        out += [hide(name, form) for name in "abcdef"]
        out += [flap(form), post(-1, form), post(1, form), pole(form), stones(form)]
    out += [seams(), ties()]
    out += [curtain_marker(), flap(2), post(-1, 2), post(1, 2), pole(2), stones(2)]
    return out
