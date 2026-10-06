"""The kit's geometry (PRE-22, PRE-46, A6.4): stretch, whole texture pixels round a pole, the 26 projections a stone's
faces take, and the preview's checker, worked by hand."""

import math
import os
import random
import sys
import unittest

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(TOOLS, "art"))
import kitmath  # noqa: E402

TRI = ((0.0, 0.0, 0.0), (0.3, 0.0, 0.0), (0.1, 0.0, 0.2))  # a triangle in the x-z plane, in metres


def flat_uv(p, sx=1.0, sy=1.0, turn=0.0):
    """Texture coordinates for a point of TRI's plane: its x and z, scaled and turned."""
    x, y = p[0] * sx, p[2] * sy
    c, s = math.cos(turn), math.sin(turn)
    return (c * x - s * y, s * x + c * y)


class Stretch(unittest.TestCase):
    # checks: PRE-22
    def test_true_size_is_one_whatever_the_turn(self):
        for turn in (0.0, 0.7, 2.5):
            uv = [flat_uv(p, turn=turn) for p in TRI]
            self.assertAlmostEqual(kitmath.stretch(*TRI, *uv), 1.0, places=9)

    # checks: PRE-22
    def test_a_squash_or_a_spread_in_one_direction_is_its_ratio(self):
        self.assertAlmostEqual(kitmath.stretch(*TRI, *[flat_uv(p, sx=0.5) for p in TRI]), 2.0, places=9)
        self.assertAlmostEqual(kitmath.stretch(*TRI, *[flat_uv(p, sy=1.6) for p in TRI]), 1.6, places=9)

    # checks: PRE-22
    def test_texture_pixels_too_large_everywhere_count_as_stretch(self):
        """Square texture pixels of 1/32 m are no 1/64 m: the figure is 2, though no direction is favoured."""
        uv = [flat_uv(p, sx=0.5, sy=0.5) for p in TRI]
        hi, lo = kitmath.singular_values(*TRI, *uv)
        self.assertAlmostEqual(hi / lo, 1.0, places=9)
        self.assertAlmostEqual(kitmath.stretch(*TRI, *uv), 2.0, places=9)

    # checks: PRE-22
    def test_a_triangle_with_no_area_or_no_texture(self):
        line = ((0, 0, 0), (1, 0, 0), (2, 0, 0))
        self.assertIsNone(kitmath.stretch(*line, (0, 0), (1, 0), (2, 0)))
        self.assertEqual(kitmath.stretch(*TRI, (0, 0), (0, 0), (0, 0)), math.inf)

    # checks: PRE-22
    def test_a_mirrored_texture_is_found(self):
        outward = (0.0, -1.0, 0.0)  # TRI's winding faces -y
        uv = [flat_uv(p) for p in TRI]
        self.assertFalse(kitmath.mirrored(*TRI, *uv, outward))
        self.assertTrue(kitmath.mirrored(*TRI, *[(-u, v) for u, v in uv], outward))


class Wraps(unittest.TestCase):
    # checks: PRE-46
    def test_a_circumference_rounds_to_whole_texture_pixels(self):
        self.assertEqual(kitmath.whole_texels(0.25), 16)
        self.assertEqual(kitmath.whole_texels(2 * math.pi * 0.05), 20)  # 20.1 texture pixels round a 10 cm pole
        self.assertEqual(kitmath.whole_texels(0.001), 3)

    # checks: PRE-46
    def test_an_ellipse_perimeter(self):
        self.assertAlmostEqual(kitmath.ellipse_perimeter(0.1, 0.1), 2 * math.pi * 0.1, places=12)
        self.assertAlmostEqual(kitmath.ellipse_perimeter(2.0, 1.0), 9.688448220547675, places=6)


class Projections(unittest.TestCase):
    # checks: PRE-22
    def test_every_normal_is_within_28_degrees_of_a_direction(self):
        self.assertEqual(len(kitmath.DIRECTIONS), 26)
        rnd = random.Random(7)
        worst = 0.0
        for _ in range(20000):
            n = kitmath.normal((rnd.gauss(0, 1), rnd.gauss(0, 1), rnd.gauss(0, 1)))
            d = kitmath.DIRECTIONS[kitmath.nearest_direction(n)]
            worst = max(worst, math.degrees(math.acos(min(1.0, kitmath.dot(n, d)))))
        self.assertLess(worst, 28.0)
        self.assertLess(1 / math.cos(math.radians(worst)), 1.14)

    # checks: PRE-22
    def test_axes_keep_height_as_the_vertical_and_never_mirror(self):
        for d in kitmath.DIRECTIONS:
            u, v = kitmath.axes(d)
            self.assertAlmostEqual(kitmath.dot(u, v), 0.0, places=9)
            self.assertAlmostEqual(kitmath.dot(u, d), 0.0, places=9)
            self.assertAlmostEqual(kitmath.length(u), 1.0, places=9)
            uv = kitmath.cross(u, v)
            self.assertAlmostEqual(kitmath.dot(uv, d), 1.0, places=9, msg=f"{d}: mirrored")
            if abs(d[2]) < 0.99:
                self.assertAlmostEqual(u[2], 0.0, places=9)  # u level, so v carries all the height
                self.assertGreater(v[2], 0.0)
        self.assertEqual(kitmath.axes((0, 0, 1)), ((1.0, 0.0, 0.0), (0.0, 1.0, 0.0)))  # tops as the ground

    # checks: PRE-22
    def test_a_face_projected_from_its_own_direction_is_true_size(self):
        d = kitmath.normal((1.0, -1.0, 0.0))
        u, v = kitmath.axes(d)
        p0 = (0.0, 0.0, 0.0)
        p1 = kitmath.scale(u, 0.2)
        p2 = kitmath.add(kitmath.scale(u, 0.05), kitmath.scale(v, 0.3))
        uv = [kitmath.project(p, d) for p in (p0, p1, p2)]
        self.assertAlmostEqual(kitmath.stretch(p0, p1, p2, *uv), 1.0, places=9)


class Atlases(unittest.TestCase):
    # checks: PRE-27 PRE-46
    def test_pieces_never_overlap_and_keep_their_gap(self):
        rnd = random.Random(3)
        atlas = kitmath.Atlas("test", 4.0, 4.0, gap=0.03)
        placed = []
        for _ in range(60):
            w, h = rnd.uniform(0.05, 0.6), rnd.uniform(0.02, 0.5)
            x, y = atlas.place(w, h)
            self.assertGreaterEqual(x, 0.03 - 1e-9)
            self.assertGreaterEqual(y, 0.03 - 1e-9)
            self.assertLessEqual(x + w, 4.0 - 0.03 + 1e-9)
            self.assertLessEqual(y + h, 4.0 - 0.03 + 1e-9)
            for px, py, pw, ph in placed:  # apart by at least the gap either side: twice the gap between pieces
                apart = x >= px + pw + 0.06 - 1e-9 or px >= x + w + 0.06 - 1e-9
                apart = apart or y >= py + ph + 0.06 - 1e-9 or py >= y + h + 0.06 - 1e-9
                self.assertTrue(apart, f"({x}, {y}, {w}, {h}) overlaps ({px}, {py}, {pw}, {ph})")
            placed.append((x, y, w, h))

    # checks: PRE-46
    def test_a_short_piece_fills_the_space_beside_a_tall_one(self):
        """A skyline places each piece at the lowest free place, so short pieces sit beside a tall one rather than
        above it, as rows would."""
        atlas = kitmath.Atlas("test", 2.0, 2.0, gap=0.0)
        atlas.place(1.0, 1.5)
        self.assertEqual(atlas.place(0.9, 0.4), (1.0, 0.0))
        self.assertEqual(atlas.place(0.9, 0.4), (1.0, 0.4))

    # checks: PRE-46
    def test_a_piece_too_wide_or_an_atlas_too_full_is_refused(self):
        atlas = kitmath.Atlas("test", 1.0, 1.0)
        with self.assertRaisesRegex(ValueError, "does not fit"):
            atlas.place(1.0, 0.1)
        atlas.place(0.9, 0.9)
        with self.assertRaisesRegex(ValueError, "is full"):
            atlas.place(0.2, 0.2)


class Checker(unittest.TestCase):
    # checks: PRE-46
    def test_the_checker_is_64_texture_pixels_a_metre_in_blocks_of_8(self):
        rows = kitmath.checker()
        self.assertEqual((len(rows), len(rows[0])), (64, 64))

        def tone(x, y):
            return sum(rows[y][x])

        self.assertGreater(tone(2, 2), tone(10, 2) + 0.5)  # a light block beside a dark one
        self.assertGreater(tone(10, 10), tone(2, 10) + 0.5)
        self.assertNotEqual(rows[3][3], rows[3][4])  # the faint checker of single texture pixels
        self.assertGreater(rows[5][0][0], rows[5][0][1])  # the first column leans red: u's metre line
        self.assertGreater(rows[63][5][1], rows[63][5][0])  # the bottom row leans green: v's metre line


if __name__ == "__main__":
    unittest.main()
