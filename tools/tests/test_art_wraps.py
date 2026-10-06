"""A material's wrap atlas (PRE-46, PRE-22, A6.4): its own levels cut into the kit's wrap strips, each seamless round
its own width, so a wrapped pole or branch shows no seam however thick it is."""

import os
import sys
import unittest

import numpy as np

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(TOOLS, "art"))
import kitmath  # noqa: E402
import look  # noqa: E402
import match  # noqa: E402
import wraps  # noqa: E402


def bark(side=256, seed=4):
    """A tiling bark: pale, with dark dashes running across, in a few shades."""
    rng = np.random.default_rng(seed)
    t = np.tile(np.array((220, 210, 195), np.uint8), (side, side, 1))
    t[rng.random((side, side)) < 0.3] = (205, 195, 180)
    for _ in range(300):
        y, x, n = rng.integers(0, side), rng.integers(0, side), rng.integers(2, 7)
        t[y, (x + np.arange(n)) % side] = (70, 55, 45)
    return t


def periodic_bark():
    """The bark with its first 104 columns repeating every 8, so every window there closes perfectly."""
    t = bark()
    t[:, 8:104] = np.tile(t[:, 0:8], (1, 12, 1))
    return t


def windows_taken(plan, side=256):
    """How many strips' windows take each column of the material."""
    taken = np.zeros(side, int)
    for w, _, x0, _ in plan:
        taken[np.arange(x0, x0 + w + min(wraps.OVERLAP, w // 2)) % side] += 1
    return taken


def levels_of(t):
    """A plain pyramid of levels by taking every other texture pixel, enough for the strips' bookkeeping."""
    out = [t]
    while out[-1].shape[0] > 1:
        out.append(out[-1][::2, ::2].copy())
    return out


def wrap_step(strip):
    """The step round a strip's wrap against the largest step between neighbouring columns inside it."""
    s = strip.astype(np.float64)
    inside = np.abs(np.diff(s, axis=1)).sum(axis=2).mean(axis=0)
    across = np.abs(s[:, 0] - s[:, -1]).sum(axis=1).mean()
    return across / max(1e-9, inside.max())


class Atlas(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.levels = levels_of(bark())
        cls.made, cls.plan = wraps.atlas(cls.levels)

    # checks: PRE-46
    def test_every_level_keeps_its_size_and_every_strip_its_place(self):
        self.assertEqual([a.shape for a in self.made], [lv.shape for lv in self.levels])
        self.assertEqual([(w, x) for w, x, _, _ in self.plan], list(kitmath.WRAPS))
        for _, _, x0, _ in self.plan:
            self.assertEqual(x0 % wraps.ALIGN, 0)

    # checks: PRE-22 PRE-46
    def test_each_strip_wraps_round_its_own_width_without_a_seam(self):
        b0 = self.made[0]
        for w, x in kitmath.WRAPS:
            strip = b0[:, x : x + w]
            self.assertLessEqual(wrap_step(strip), 1.0, f"the {w}-pixel strip shows its wrap")
        plain = self.levels[0][:, 0:16]  # the material's own columns, never closed: a seam round 16
        self.assertGreater(wrap_step(plain), 1.0)

    # checks: PRE-22 PRE-46
    def test_the_strips_come_from_across_the_material(self):
        """Even where one part of the material closes perfectly at every width, the strips are not all taken from
        it, which would repeat it across the atlas: the four widest share no column, and every column serves."""
        for name, t in (("bark", bark()), ("periodic bark", periodic_bark())):
            _, plan = wraps.atlas(levels_of(t))
            taken = windows_taken(plan[:4])
            self.assertLessEqual(int(taken.max()), 1, f"{name}: two of the widest strips share columns")
            self.assertTrue((windows_taken(plan) > 0).all(), f"{name}: a column no strip takes")

    # checks: PRE-01
    def test_every_texture_pixel_comes_whole_from_the_material(self):
        def colours(t):
            return {tuple(c) for c in t.reshape(-1, 3)}

        self.assertTrue(colours(self.made[0]) <= colours(self.levels[0]))

    # checks: PRE-22
    def test_a_strip_closes_where_its_columns_agree(self):
        cols = np.tile(np.arange(20, dtype=np.uint8)[None, :, None] * 10, (8, 1, 3))
        cols[:, 16:20] = cols[:, 0:4]  # the four after its edge repeat its first four: a perfect closing
        strip, cost = wraps.close(cols, 16, 4)
        self.assertEqual(cost, 0.0)
        np.testing.assert_array_equal(strip, cols[:, :16])
        broken = cols.copy()
        broken[:, 16:20] = 255 - broken[:, 16:20]  # nothing after its edge agrees with its start
        self.assertGreater(wraps.close(broken, 16, 4)[1], 0.0)


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class Colour(unittest.TestCase):
    # checks: PRE-20 PRE-46
    def test_a_small_level_that_strays_is_brought_back_and_one_that_holds_is_left(self):
        """A 4-pixel level whose strips took the material's two lighter columns twice strays in lightness; it is
        brought back to the material's level, and a level that holds its colour is left exactly as it was."""
        level = np.zeros((4, 4, 3), np.uint8)
        level[:, 0::2] = (150, 120, 90)
        level[:, 1::2] = (110, 86, 62)
        strayed = level.copy()
        strayed[:, 1::2] = (140, 112, 84)
        made, words = wraps.hold_colour([level.copy(), strayed], [level, level])
        np.testing.assert_array_equal(made[0], level)
        self.assertIsNone(words[0])
        self.assertIsNotNone(words[1])
        got, want = look.stats(made[1]), look.stats(level)
        self.assertLessEqual(abs(got["lightness"] - want["lightness"]), wraps.STRAY * match.DRIFT_LIGHTNESS)


if __name__ == "__main__":
    unittest.main()
