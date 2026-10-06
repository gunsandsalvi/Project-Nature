"""The kit's geometry, written once (A6.1, A6.4): the numbers that the Blender scripts building the kit's parts and
the checks measuring them both use. Plain Python with no numpy, since Blender's own Python here has none.

- stretch: how far a triangle's texture pixels are from squares of 1/64 m, the line being 1.5;
- whole_texels: a circumference rounded to whole texture pixels, so a pole's wrap never falls inside one;
- WRAPS, wrap_width and wrap_offset: the strips of a material's wrap atlas, each seamless round its own width, that
  a wrapped pole, branch or trunk takes its texture from, its circumference taking the nearest strip's width;
- DIRECTIONS and axes: the projections a stone's faces take, each from the direction it faces (the nearest of 26),
  with height as its vertical; a face is at most about 28 degrees from its direction, so stretched at most 1.13;
- Atlas: the layout of a texture drawn to a figure, its pieces placed so none overlaps another;
- checker: the preview's checker of 64 texture pixels a metre.

Texture coordinates are in metres everywhere (A6.4): one unit of a part's texture coordinates is one metre of its
surface, and one texture pixel is 1/64 of it at band 0.

Implements PRE-22 and PRE-46, see A6.4.
"""

import math

TEXELS_A_METRE = 64
STRETCH_MOST = 1.5


def sub(a, b):
    return tuple(x - y for x, y in zip(a, b, strict=True))


def add(a, b):
    return tuple(x + y for x, y in zip(a, b, strict=True))


def scale(a, k):
    return tuple(x * k for x in a)


def dot(a, b):
    return sum(x * y for x, y in zip(a, b, strict=True))


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def length(a):
    return math.sqrt(dot(a, a))


def normal(a):
    n = length(a)
    if n == 0:
        raise ValueError("a vector of no length has no direction")
    return scale(a, 1.0 / n)


def singular_values(p0, p1, p2, t0, t1, t2):
    """(largest, smallest) singular value of the map from a triangle's surface to its texture coordinates, both in
    metres, or None for a triangle with no area: 1 and 1 when the texture lies on the surface at its true size."""
    e1 = sub(p1, p0)
    e2 = sub(p2, p0)
    l1 = length(e1)
    n = cross(e1, e2)
    area2 = length(n)
    if l1 == 0 or area2 <= 1e-14:
        return None
    x = scale(e1, 1.0 / l1)
    y = normal(cross(n, e1))
    # the triangle in its own plane: q1 = (l1, 0), q2 = (a, b)
    a, b = dot(e2, x), dot(e2, y)
    d1 = (t1[0] - t0[0], t1[1] - t0[1])
    d2 = (t2[0] - t0[0], t2[1] - t0[1])
    # M maps q1 to d1 and q2 to d2: M = [d1 d2] [q1 q2]^-1, with [q1 q2]^-1 = [[1/l1, -a/(l1 b)], [0, 1/b]]
    m00 = d1[0] / l1
    m10 = d1[1] / l1
    m01 = (d2[0] - a * m00) / b
    m11 = (d2[1] - a * m10) / b
    frob = m00 * m00 + m01 * m01 + m10 * m10 + m11 * m11
    det = m00 * m11 - m01 * m10
    root = math.sqrt(max(0.0, frob * frob - 4 * det * det))
    hi = math.sqrt(max(0.0, (frob + root) / 2))
    lo = math.sqrt(max(0.0, (frob - root) / 2))
    return hi, lo


def stretch(p0, p1, p2, t0, t1, t2):
    """How stretched a triangle's texture pixels are on its surface: 1 when each is a square of 1/64 m, otherwise
    the worst ratio, in any direction, of a texture pixel's side on the surface to 1/64 m or of 1/64 m to it; None
    for a triangle with no area, and infinity for one whose texture coordinates have none."""
    sv = singular_values(p0, p1, p2, t0, t1, t2)
    if sv is None:
        return None
    hi, lo = sv
    if lo <= 1e-12:
        return math.inf
    return max(hi, 1.0 / lo)


def mirrored(p0, p1, p2, t0, t1, t2, outward):
    """Whether a triangle's texture lies on it mirrored, seen from the side its outward normal points to."""
    n = cross(sub(p1, p0), sub(p2, p0))
    turn = (t1[0] - t0[0]) * (t2[1] - t0[1]) - (t1[1] - t0[1]) * (t2[0] - t0[0])
    return (dot(n, outward) >= 0) != (turn >= 0)


def whole_texels(circumference, texels_a_metre=TEXELS_A_METRE, least=3, step=1):
    """A circumference in metres rounded to whole texture pixels (at least `least`), or to whole steps of them."""
    return max(least, step * int(round(circumference * texels_a_metre / step)))


# The wrap strips (A6.4): a wrapped pole, branch, trunk or antler that is not drawn to an atlas takes its texture from
# its material's wrap atlas, a 256-pixel tile cut into strips side by side, one for each of these widths, each strip
# seamless round its own width: (width, the strip's left edge), in texture pixels. Every edge falls on a multiple of
# 4 (the 6-pixel strip last), so the strips stay whole in the levels for bands 1 and 2.
WRAPS = ((64, 0), (48, 64), (40, 112), (32, 152), (24, 184), (16, 208), (12, 224), (8, 236), (4, 244), (6, 248))


def wrap_width(circumference, texels_a_metre=TEXELS_A_METRE):
    """The wrap strip for a circumference in metres: the width (in texture pixels) nearest it by ratio, so the
    texture round it is stretched as little as the strips allow."""
    true = max(1e-9, circumference * texels_a_metre)
    return min((w for w, _ in WRAPS), key=lambda w: (abs(math.log(w / true)), w))


def wrap_offset(width):
    """The left edge of the strip `width` texture pixels wide in a wrap atlas, in texture pixels."""
    for w, x in WRAPS:
        if w == width:
            return x
    widths = ", ".join(str(w) for w, _ in WRAPS)
    raise ValueError(f"no wrap strip is {width} texture pixels wide; the strips are {widths}")


def ellipse_perimeter(rx, ry):
    """The perimeter of an ellipse (Ramanujan's second formula, within a millionth for these shapes)."""
    h = ((rx - ry) / (rx + ry)) ** 2 if rx + ry > 0 else 0.0
    return math.pi * (rx + ry) * (1 + 3 * h / (10 + math.sqrt(4 - 3 * h)))


# The 26 directions a face can be projected from: the axes, the edges' and the corners' diagonals of a cube.
DIRECTIONS = tuple(
    normal((i, j, k)) for i in (-1, 0, 1) for j in (-1, 0, 1) for k in (-1, 0, 1) if (i, j, k) != (0, 0, 0)
)
UP = (0.0, 0.0, 1.0)


def nearest_direction(n):
    """The index in DIRECTIONS of the direction nearest a face's normal."""
    n = normal(n)
    return max(range(len(DIRECTIONS)), key=lambda i: dot(DIRECTIONS[i], n))


def axes(d):
    """The texture's (u, v) axes for faces projected from direction d (the way they face): v is height, the world's
    up laid on the plane across d, and u runs to its right as seen from outside, so nothing is mirrored; faces that
    look straight up or down take the world's x and y, as the ground does."""
    d = normal(d)
    if abs(d[2]) > 1 - 1e-9:
        return ((1.0, 0.0, 0.0), (0.0, 1.0 if d[2] > 0 else -1.0, 0.0))
    u = normal(cross(UP, d))
    v = cross(d, u)
    return u, v


def project(p, d):
    """A point's texture coordinates in metres under the projection from direction d."""
    u, v = axes(d)
    return (dot(p, u), dot(p, v))


class Atlas:
    """A texture drawn to a layout, such as a figure's skin and hair or a garment's: width x height metres, its
    pieces (each sleeve's, panel's, card's or chart's) placed as they are made, each at the lowest free place across
    the atlas (a skyline: the top edge of what is placed so far), `gap` metres apart, so none overlaps another. Its
    name goes on each part using it, for the check."""

    def __init__(self, name, width, height, gap=0.03):
        self.name = name
        self.width = float(width)
        self.height = float(height)
        self.gap = float(gap)
        self.skyline = [(0.0, self.width, 0.0)]  # (x, width, height reached), left to right across the atlas

    def place(self, w, h):
        """The lower left corner of a free piece w x h metres."""
        g = self.gap
        need, tall = w + 2 * g, h + 2 * g
        if need > self.width:
            raise ValueError(f"a piece {w:.2f} m wide does not fit the atlas {self.name}, {self.width:.2f} m wide")
        best = None
        for i, (x, _, _) in enumerate(self.skyline):
            if x + need > self.width + 1e-9:
                break
            top = 0.0
            for sx, sw, sy in self.skyline[i:]:  # the highest step under the piece's width
                top = max(top, sy)
                if sx + sw >= x + need - 1e-9:
                    break
            if best is None or (top, x) < best:
                best = (top, x)
        if best is None or best[0] + tall > self.height + 1e-9:
            raise ValueError(f"the atlas {self.name} ({self.width:.2f} x {self.height:.2f} m) is full")
        y, x = best
        new = []
        for sx, sw, sy in self.skyline:  # the steps the piece covers give way to its top
            lo, hi = max(sx, x), min(sx + sw, x + need)
            if hi <= lo:
                new.append((sx, sw, sy))
                continue
            if sx < lo:
                new.append((sx, lo - sx, sy))
            if hi < sx + sw:
                new.append((hi, sx + sw - hi, sy))
        new.append((x, need, y + tall))
        self.skyline = sorted(new)
        return (x + g, y + g)


def checker(size=TEXELS_A_METRE, block=8):
    """The preview's checker, one metre of it: `size` texture pixels a side, so it shows 64 a metre where texture
    coordinates are in metres. Blocks of `block` texture pixels alternate a light warm tone and a dark cool one, each
    holding a faint checker of single texture pixels, and the first column and the bottom row are tinted toward red
    and green, so the metre lines show which way u and v run. Rows of (r, g, b), top row first, each value 0 to 1."""
    light, dark = (0.86, 0.80, 0.70), (0.36, 0.40, 0.50)
    red, green = (0.85, 0.25, 0.20), (0.25, 0.70, 0.30)
    rows = []
    for y in range(size):
        row = []
        for x in range(size):
            base = light if ((x // block) + (y // block)) % 2 == 0 else dark
            k = 1.06 if (x + y) % 2 == 0 else 0.94
            c = [min(1.0, v * k) for v in base]
            if x == 0:
                c = [0.55 * a + 0.45 * b for a, b in zip(c, red, strict=True)]
            elif y == size - 1:
                c = [0.55 * a + 0.45 * b for a, b in zip(c, green, strict=True)]
            row.append(tuple(c))
        rows.append(row)
    return rows
