"""The look loop's drawing run (PRE-31, PRE-22, A4.8): the camera's motion found from four pixels, many-sample
pictures shrunk in linear light, the lettered grid; and the shimmer check itself, which flags the test board read
nearest-pixel and passes the meadow read smooth-pixel."""

import os
import shutil
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import lookrun  # noqa: E402

SQUARE = [[0.0, 0.0], [127.0, 0.0], [0.0, 127.0], [127.0, 127.0]]


def apply(m, x, y):
    w = m[6] * x + m[7] * y + m[8]
    return (m[0] * x + m[1] * y + m[2]) / w, (m[3] * x + m[4] * y + m[5]) / w


class Pieces(unittest.TestCase):
    # checks: PRE-22
    def test_the_motion_is_the_projective_map_through_four_pixels(self):
        np.testing.assert_allclose(
            lookrun.homography(SQUARE, [[x + 4.0, y] for x, y in SQUARE]), [1, 0, 4, 0, 1, 0, 0, 0, 1], atol=1e-12
        )
        # a slanted pan, wider at the bottom as on tilted ground: every corner lands where it was found
        to = [[4.02, 0.0], [131.02, 0.0], [4.51, 127.0], [131.51, 127.0]]
        m = lookrun.homography(SQUARE, to)
        for (x, y), (u, v) in zip(SQUARE, to, strict=True):
            np.testing.assert_allclose(apply(m, x, y), (u, v), atol=1e-9)

    # checks: PRE-22
    def test_a_many_sample_picture_shrinks_in_linear_light(self):
        t = np.zeros((2, 2, 3), np.uint8)
        t[0, 0] = t[1, 1] = 255
        # half white in linear light is 188 in sRGB, not the 128 of averaging the bytes
        np.testing.assert_array_equal(lookrun.shrink(t, 2), np.full((1, 1, 3), 188, np.uint8))

    # checks: PRE-31
    def test_the_grid_names_columns_by_letters_and_rows_by_numbers(self):
        self.assertEqual([lookrun.letters(i) for i in (0, 1, 25, 26, 27)], ["A", "B", "Z", "AA", "AB"])
        d = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, d)
        a = np.zeros((240, 120, 3), np.uint8)
        lookrun.grid(a, a + 50, os.path.join(d, "grid.png"))
        self.assertTrue(os.path.exists(os.path.join(d, "grid.png")))


READY = (
    os.path.exists(os.environ.get("GODOT", "")) and shutil.which("xvfb-run") is not None
    and os.path.exists(os.path.join(lookrun.ROOT, "game", "bin", "libkindling.linux.x86_64.so"))
    and os.path.exists(lookrun.look.program())
)  # fmt: skip


@unittest.skipUnless(
    READY and os.environ.get("KD_CHECK_QUICK") != "1",
    "visual audit: needs Godot, Xvfb, built tools and a run without KD_CHECK_QUICK=1",
)
class TheShimmerCheck(unittest.TestCase):
    # checks: PRE-22 PRE-31
    def test_it_flags_the_board_read_nearest_pixel_and_passes_the_meadow_read_smooth_pixel(self):
        report, _ = lookrun.run_shimmer()
        self.assertGreater(report["board-nearest"]["mean"], 10.0)
        self.assertFalse(report["board-nearest"]["passes"])
        self.assertLessEqual(report["meadow-smooth"]["mean"], lookrun.SHIMMER_MOST)
        self.assertTrue(report["meadow-smooth"]["passes"])
        # the smooth pixel always flickers less than the nearest one
        for ground in ("meadow", "board"):
            self.assertLess(report[f"{ground}-smooth"]["mean"], report[f"{ground}-nearest"]["mean"])


if __name__ == "__main__":
    unittest.main()
