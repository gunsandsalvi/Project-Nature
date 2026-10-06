"""A tile's levels (PRE-22, PRE-20, A5.3, A5.4): each made by the code reduction from the level above, its colours
matched to it, and only as many of its marks drawn again as keep the band's accents at 90% of band 0's."""

import os
import sys
import unittest

import numpy as np

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(TOOLS, "art"))
sys.path.insert(0, os.path.join(TOOLS, "tests"))
import levels  # noqa: E402
import look  # noqa: E402
import match  # noqa: E402
from test_art_reduce import crumbs  # noqa: E402
from test_art_wraps import bark  # noqa: E402


def stains(side=128, seed=3):
    """Soil in broad stains of three close shades, with no small marks at all."""
    rng = np.random.default_rng(seed)
    f = np.fft.ifft2(
        np.fft.fft2(rng.normal(size=(side, side)))
        * np.exp(-0.5 * (np.hypot(*np.meshgrid(np.fft.fftfreq(side), np.fft.fftfreq(side))) * 16) ** 2)
    ).real
    shades = np.array([(150, 112, 78), (166, 126, 88), (180, 138, 98)], np.uint8)
    return shades[np.digitize(f, np.quantile(f, [0.33, 0.66]))]


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class Levels(unittest.TestCase):
    # checks: PRE-22 PRE-20
    def test_crumbs_keep_the_accents_with_bolder_marks(self):
        b0 = crumbs()
        made = levels.make([b0], b0)
        self.assertEqual([s["level"].shape[0] for s in made], [64, 32, 16, 8, 4, 2, 1])
        acc0 = match.accents(b0)
        for n, step in enumerate(made[:2], start=1):
            self.assertTrue(step["ok"], f"band {n}: {step['accents'] / acc0:.0%} of band 0's accents")
            self.assertGreaterEqual(step["accents"], 0.9 * acc0)
        self.assertGreater(made[0]["marks"], 0, "band 1 needs its crumbs back")
        self.assertLessEqual(made[0]["marks"], made[0]["most"])

    # checks: PRE-22
    def test_no_marks_are_drawn_where_none_are_needed(self):
        b0 = stains()
        made = levels.make([b0], b0)
        self.assertEqual([s["marks"] for s in made], [0] * len(made))

    # checks: PRE-20
    def test_every_level_keeps_band_0s_colour_down_to_one_texture_pixel(self):
        """Each level is matched to band 0 itself, not to the level above, so what each calibration leaves never
        adds up, and rounding is settled: a pale bark, grey enough for one unit to turn its hue, keeps it."""
        for name, b0 in (("crumbs", crumbs()), ("pale bark", bark())):
            r0 = look.stats(b0)
            for n, step in enumerate(levels.make([b0], b0), start=1):
                s = look.stats(step["level"])
                self.assertLessEqual(abs(s["lightness"] - r0["lightness"]), match.DRIFT_LIGHTNESS, f"{name} {n}")
                hue = abs((s["hue"] - r0["hue"] + 180) % 360 - 180)
                self.assertLessEqual(hue, match.DRIFT_HUE, f"{name} band {n}'s hue")

    # checks: PRE-42
    def test_the_way_names_what_was_done(self):
        self.assertIn("3 of its strongest marks drawn again", levels.way(3, 10))
        self.assertIn("no marks kept", levels.way(0, 10))
        self.assertTrue(levels.way(0, 0).startswith("code reduction of the level above"))
        step = {"numbers": [0.5, -1.0, 101.0, 98.0], "move": (0, 1, 0)}
        self.assertIn("moved (0, 1, 0) in red, green and blue for rounding", levels.calibration(step))
        self.assertNotIn("moved", levels.calibration(dict(step, move=(0, 0, 0))))


if __name__ == "__main__":
    unittest.main()
