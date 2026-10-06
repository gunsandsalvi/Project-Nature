"""The sheet (PRE-20, PRE-23, PRE-01, A5.4): one material for your eye on the phone, lossless and 1080 pixels wide,
every band at true size (each texture pixel 2 x 2 screen pixels), flat and under stand-in lights, every level
enlarged, and a rock surface under stand-in layers."""

import os
import sys
import tempfile
import unittest

import numpy as np

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(TOOLS, "art"))
import sheet  # noqa: E402
import texels  # noqa: E402

MEADOW = os.path.join(os.path.dirname(TOOLS), "art", "textures", "meadow")


class Parts(unittest.TestCase):
    # checks: PRE-01
    def test_true_size_is_two_screen_pixels_a_texture_pixel(self):
        level = np.random.default_rng(1).integers(0, 256, (4, 4, 3)).astype(np.uint8)
        shown = sheet.at_true_size(level)
        self.assertEqual(shown.shape, (256, 256, 3))  # 128 texture pixels, the level repeated as the ground does
        np.testing.assert_array_equal(shown[:8, :8], texels.enlarge(level, 2))
        np.testing.assert_array_equal(shown[8:16, :8], texels.enlarge(level, 2))

    # checks: PRE-01
    def test_levels_are_enlarged_by_whole_factors(self):
        big, k = sheet.enlarged(np.zeros((256, 256, 3), np.uint8))
        self.assertEqual((big.shape, k), ((256, 256, 3), 8))
        small, k = sheet.enlarged(np.zeros((4, 4, 3), np.uint8))
        self.assertEqual((small.shape, k), ((256, 256, 3), 64))

    # checks: PRE-20
    def test_stand_in_lights_multiply_in_linear_light(self):
        t = np.random.default_rng(2).integers(0, 256, (16, 16, 3)).astype(np.uint8)
        np.testing.assert_array_equal(sheet.lit(t, (1.0, 1.0, 1.0)), t)
        shade = sheet.lit(t, dict(sheet.LIGHTS)["shade"]).astype(int)
        self.assertTrue((shade <= t).all())
        self.assertGreater((shade[..., 2] - t[..., 2]).mean(), (shade[..., 0] - t[..., 0]).mean(), "shade is blue")

    # checks: PRE-23
    def test_the_stand_in_cliff_lays_beds_and_a_shelter(self):
        surface = np.full((256, 256, 3), 180, np.uint8)
        cliff = sheet.stand_in_cliff(surface)
        self.assertEqual(cliff.shape, (176, 256, 3))
        np.testing.assert_array_equal(cliff, sheet.stand_in_cliff(surface))
        self.assertFalse((cliff == sheet.stand_in_cliff(surface, seed=2)).all())
        rows = cliff.astype(float).mean(axis=(1, 2))
        self.assertLess(rows[-20:].mean(), rows[:100].mean(), "the shelter under the lowest bed is darker")
        self.assertGreater(len(np.unique(rows.round())), 5, "beds, ledges and undercuts shade a flat surface")


class Sheet(unittest.TestCase):
    # checks: PRE-20 PRE-01
    def test_a_sheet_is_lossless_and_as_wide_as_the_phone(self):
        a = sheet.make(MEADOW, notes=["seams: passed"])
        self.assertEqual(a.shape[1], sheet.WIDTH)
        self.assertEqual(sheet.WIDTH, 1080)
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "meadow.webp")
            texels.save_webp(a, path)
            np.testing.assert_array_equal(texels.load(path), a)

    # checks: PRE-23
    def test_a_rock_sheet_adds_the_stand_in_layers(self):
        plain = sheet.make(MEADOW)
        rock = sheet.make(MEADOW, rock=True)
        self.assertGreater(rock.shape[0], plain.shape[0] + 2 * 176)


if __name__ == "__main__":
    unittest.main()
