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


class Strips(unittest.TestCase):
    # checks: PRE-22
    def test_a_strip_is_the_phones_width_with_the_versions_mixed_by_place(self):
        red, blue = np.zeros((16, 16, 3), np.uint8), np.zeros((16, 16, 3), np.uint8)
        red[...] = (200, 40, 40)
        blue[...] = (40, 40, 200)
        s = sheet.strip([[red], [blue]], 0, size=(160, 48))
        self.assertEqual(s.shape, (96, 320, 3))  # 2 x 2 screen pixels a texture pixel
        cells = s[::32, ::32].reshape(-1, 3)
        self.assertEqual({tuple(c) for c in cells}, {(200, 40, 40), (40, 40, 200)}, "both versions show")
        again = sheet.strip([[red], [blue]], 0, size=(160, 48))
        np.testing.assert_array_equal(s, again)  # a place always takes the same version
        self.assertEqual(sheet.pick(3, 5, 4), sheet.pick(3, 5, 4))
        self.assertGreater(len({sheet.pick(x, y, 4) for x in range(6) for y in range(6)}), 2)

    # checks: PRE-46
    def test_wrap_strips_are_each_repeated_round(self):
        level = np.random.default_rng(5).integers(0, 256, (256, 256, 3)).astype(np.uint8)
        widths = ((16, 0), (8, 16))
        s = sheet.wrap_strips(level, widths, rows=10, turns=3)
        self.assertEqual(s.shape, (10, 16 * 3 + 3 + 8 * 3 + 3, 3))
        np.testing.assert_array_equal(s[:, :16], s[:, 16:32])


class Words(unittest.TestCase):
    # checks: PRE-42
    def test_the_block_size_is_read_from_a_records_words(self):
        self.assertEqual(sheet.block_of("re-gridded at its own 8.00-pixel blocks, its light taken out"), 8.0)
        self.assertEqual(sheet.block_of("re-gridded at 64 a metre (blocks of 4.9 picture pixels)"), 4.9)
        self.assertEqual(sheet.block_of("at 7.2-pixel blocks (at GPT's own blocks of about 10.8 the loss...)"), 7.2)
        self.assertIsNone(sheet.block_of("drawn by code"))

    # checks: PRE-42
    def test_the_checks_are_summed_up_with_each_failure_in_full(self):
        results = [
            ("", "seams", True, "1.05"),
            ("", "contrast", None, "not measured"),
            ("v2", "contrast", None, "not measured"),
            ("v2", "accents", False, "under 90%: band 1 85%"),
        ]
        lines = sheet.summary(results)
        self.assertEqual(lines[0], "1 check passes on its 2 tiles; 1 fails:")
        self.assertIn("FAILS: v2, accents: under 90%: band 1 85%", lines)
        self.assertIn("contrast (near, v2): not measured", lines)

    # checks: PRE-46
    def test_a_drawn_atlas_is_shown_where_its_pieces_lie(self):
        mask = np.zeros((256, 256), bool)
        mask[150:200, 40:90] = True  # the pieces, far from the corner, where the canvas is all gaps
        y, x = sheet.busiest(mask)
        self.assertTrue(y <= 150 and y + 128 >= 200 and x <= 40 and x + 128 >= 90)

    # checks: PRE-22
    def test_a_source_and_its_level_are_shown_at_the_same_scale(self):
        picture = np.zeros((600, 900, 3), np.uint8)
        level = np.random.default_rng(3).integers(0, 256, (256, 256, 3)).astype(np.uint8)
        crop, shown, k = sheet.beside(picture, level, 7.6, box=(0, 0, 900, 200))
        self.assertEqual(k, 8)
        self.assertEqual(crop.shape, shown.shape)
        self.assertEqual(crop.shape[0], 200)
        np.testing.assert_array_equal(shown[:8, :8], np.broadcast_to(level[0, 0], (8, 8, 3)))


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
