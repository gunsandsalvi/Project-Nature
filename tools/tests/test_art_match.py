"""Colour matching (PRE-20, PRE-22, A5.4): a level fitted to the level above in four numbers (lightness, hue,
colourfulness, contrast) that keep its accents at least 90% of band 0's, the contrast changed by at most 30% either
way, every measure and change made by `kindling look`."""

import os
import sys
import unittest

import numpy as np

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(TOOLS, "art"))
import look  # noqa: E402
import match  # noqa: E402
import texels  # noqa: E402

MEADOW = os.path.join(os.path.dirname(TOOLS), "art", "textures", "meadow", "b0.png")


class Numbers(unittest.TestCase):
    # checks: PRE-20
    def test_the_four_numbers_bring_one_picture_to_another(self):
        level = {"lightness": 60.0, "hue": 350.0, "colourfulness": 10.0, "contrast": 8.0}
        above = {"lightness": 65.0, "hue": 10.0, "colourfulness": 12.0, "contrast": 6.0}
        np.testing.assert_allclose(match.numbers(level, above), [5.0, 20.0, 120.0, 75.0])  # hue turns the short way

    # checks: PRE-20
    def test_a_grey_picture_keeps_its_hue_and_scale(self):
        grey = {"lightness": 50.0, "hue": 200.0, "colourfulness": 0.03, "contrast": 0.01}
        above = {"lightness": 50.0, "hue": 90.0, "colourfulness": 8.0, "contrast": 5.0}
        self.assertEqual(match.numbers(grey, above), [0.0, 0.0, 100.0, 100.0])

    # checks: PRE-20
    def test_the_numbers_are_written_as_the_record_writes_them(self):
        self.assertEqual(
            match.describe([1.2581, -6.2060, 103.1290, 82.8631]),
            "lightness +1.3%, hue -6.2 degrees, colourfulness 103%, contrast 83%",
        )


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class Fitting(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.band0 = texels.load(MEADOW)
        cls.accents = match.accents(cls.band0)

    # checks: PRE-20 PRE-22
    def test_a_level_off_in_colour_is_brought_back_with_its_accents(self):
        off = look.adjust(self.band0, -6.0, 15.0, 70.0, 100.0)
        out, nums, accents, ok = match.fit(off, self.band0, self.accents)
        got, want = look.stats(out), look.stats(self.band0)
        self.assertAlmostEqual(got["lightness"], want["lightness"], delta=0.3)
        self.assertAlmostEqual(got["hue"], want["hue"], delta=1.0)
        self.assertAlmostEqual(got["colourfulness"] / want["colourfulness"], 1.0, delta=0.03)
        self.assertTrue(ok)
        self.assertGreaterEqual(accents, 0.9 * self.accents)

    # checks: PRE-20
    def test_a_level_too_flat_is_reported_never_stretched_into_speckle(self):
        flat = look.adjust(self.band0, 0.0, 0.0, 100.0, 30.0)
        out, nums, accents, ok = match.fit(flat, self.band0, self.accents)
        self.assertFalse(ok)
        self.assertEqual(nums[3], match.CONTRAST_MOST)
        self.assertLess(accents, 0.9 * self.accents)


if __name__ == "__main__":
    unittest.main()
