"""The code reduction (PRE-22, PRE-20, A5.3, A5.4): each level below drawn from its own shades by majority, never
averaged, lone texture pixels cleaned, the tile still wrapping, and its strongest marks drawn again bolder and fewer,
so a band farther out grows calmer and keeps its accents instead of turning to speckle."""

import os
import sys
import unittest

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import reduce  # noqa: E402

A, B = (200, 180, 120), (60, 90, 40)
SOIL, CRUMB, GRIT = (170, 130, 90), (60, 40, 30), (230, 210, 170)


def two_shades(side=32, seed=1, share=0.5):
    rng = np.random.default_rng(seed)
    return np.where((rng.random((side, side)) < share)[..., None], np.array(A, np.uint8), np.array(B, np.uint8))


def colours(t):
    return {tuple(int(v) for v in c) for c in t.reshape(-1, 3)}


def crumbs(side=128, count=200, seed=5):
    """A calm soil in two close shades, with single dark crumbs and pale grains scattered on it."""
    rng = np.random.default_rng(seed)
    t = np.tile(np.array(SOIL, np.uint8), (side, side, 1))
    t[side // 2 :, :] = (178, 138, 96)  # a broad stain a shade lighter
    for y, x in rng.integers(0, side, (count, 2)):
        t[y, x] = CRUMB if rng.random() < 0.7 else GRIT
    return t


def count_runs(mask):
    """How many 4-connected runs a mask holds, the tile wrapping."""
    return len(reduce.runs(mask, np.zeros(mask.shape, int)))


class Reduction(unittest.TestCase):
    # checks: PRE-22
    def test_each_level_is_half_the_one_above_down_to_one_texture_pixel(self):
        t = np.random.default_rng(2).integers(0, 256, (64, 64, 3)).astype(np.uint8)
        sides = []
        while t.shape[0] > 1:
            t = reduce.reduce(t, marks=None)
            sides.append(t.shape[:2])
        self.assertEqual(sides, [(32, 32), (16, 16), (8, 8), (4, 4), (2, 2), (1, 1)])
        with self.assertRaises(ValueError):
            reduce.reduce(t)

    # checks: PRE-22 PRE-20
    def test_colours_are_never_averaged(self):
        low = reduce.reduce(two_shades(), marks=None)
        self.assertTrue(colours(low) <= {A, B}, "a mixed colour is speckle")

    # checks: PRE-20
    def test_coarser_levels_get_fewer_shades(self):
        rng = np.random.default_rng(3)
        self.assertLessEqual(len(colours(reduce.reduce(rng.integers(0, 256, (256, 256, 3)).astype(np.uint8)))), 12)
        self.assertLessEqual(len(colours(reduce.reduce(rng.integers(0, 256, (16, 16, 3)).astype(np.uint8)))), 6)

    # checks: PRE-22
    def test_the_tile_still_wraps(self):
        """Moving band 0 by whole blocks moves the background below by the same, wrapping: every texture pixel of
        it depends only on its own block and its neighbours round the wrap, so a seamless level stays so."""
        t = np.random.default_rng(4).integers(0, 256, (32, 32, 3)).astype(np.uint8)
        low = reduce.reduce(t)
        for dy, dx in ((1, 0), (0, 3), (5, 7)):
            moved = reduce.reduce(np.roll(t, (2 * dy, 2 * dx), axis=(0, 1)))
            np.testing.assert_array_equal(moved, np.roll(low, (dy, dx), axis=(0, 1)))


class Marks(unittest.TestCase):
    # checks: PRE-22 PRE-20
    def test_the_majority_alone_drops_single_crumbs(self):
        """What batch 1's reduction did, and why its bands lost their accents: the crumbs vanish."""
        low = reduce.reduce(crumbs(), marks=0)
        self.assertNotIn(CRUMB, colours(low))

    # checks: PRE-22 PRE-20
    def test_the_strongest_marks_come_back_bolder_and_fewer(self):
        t = crumbs()
        r = reduce.Reduction(t)
        self.assertGreater(r.most, 0)
        low = r.draw()
        dark = (np.abs(low.astype(int) - CRUMB).sum(-1) < 40) | (np.abs(low.astype(int) - GRIT).sum(-1) < 40)
        above = (np.abs(t.astype(int) - CRUMB).sum(-1) < 40) | (np.abs(t.astype(int) - GRIT).sum(-1) < 40)
        self.assertLessEqual(count_runs(dark), count_runs(above) // 2, "fewer marks")
        for _, _, cells in r.marks:
            self.assertGreaterEqual(len(cells), 2, "never a single-pixel mark")
        self.assertLessEqual(dark.mean(), reduce.MOST_SHARE + 1e-9, "the marks cover at most their share")

    # checks: PRE-22
    def test_marks_never_touch_one_another(self):
        r = reduce.Reduction(crumbs(count=900, seed=6))
        owner = -np.ones(r.background.shape, int)
        for i, (_, _, cells) in enumerate(r.marks):
            for y, x in cells:
                owner[y, x] = i
        h, w = owner.shape
        for y, x in np.argwhere(owner >= 0):
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    o = owner[(y + dy) % h, (x + dx) % w]
                    self.assertIn(o, (-1, owner[y, x]), f"marks {owner[y, x]} and {o} touch at ({y}, {x})")

    # checks: PRE-22
    def test_as_many_marks_as_asked_and_none_on_a_small_level(self):
        r = reduce.Reduction(crumbs())
        drawn = [int((r.draw(n) != r.draw(0)).any(-1).sum()) for n in (0, 1, 3)]
        self.assertEqual(drawn[0], 0)
        self.assertLess(drawn[1], drawn[2])
        small = reduce.Reduction(crumbs(side=32, count=40))
        self.assertEqual(small.most, 0, "a 16-pixel level's marks would stand in a grid")


class Steps(unittest.TestCase):
    # checks: PRE-22
    def test_each_block_takes_its_most_common_shade(self):
        shades = np.array([A, B], np.uint8)
        label = np.array([[0, 0], [0, 1]])
        t = shades[label]
        self.assertEqual(reduce.majority(label, t, shades)[0, 0], 0)

    # checks: PRE-20
    def test_a_tie_goes_to_the_shade_nearest_the_blocks_mean(self):
        shades = np.array([[0, 0, 0], [50, 50, 50], [200, 200, 200], [255, 255, 255]], np.uint8)
        label = np.array([[0, 1], [2, 3]])  # one vote each; the mean is 126, nearest 200 (74 away) not 50 (76)
        self.assertEqual(reduce.majority(label, shades[label], shades)[0, 0], 2)

    # checks: PRE-20
    def test_lone_texture_pixels_take_their_neighbours_shade(self):
        label = np.zeros((8, 8), int)
        label[3, 3] = 1  # alone
        label[6, 1] = label[6, 2] = 2  # a pair keeps each other
        out = reduce.clean(label)
        self.assertEqual(out[3, 3], 0)
        self.assertEqual((out[6, 1], out[6, 2]), (2, 2))

    # checks: PRE-20 PRE-22
    def test_small_levels_keep_every_texture_pixel(self):
        big = np.tile(np.array(A, np.uint8), (32, 32, 1))
        big[10:12, 10:12] = B  # one block of another shade: alone in the level below
        self.assertNotIn(B, colours(reduce.reduce(big)), "cleaned at 16 texture pixels")
        small = np.tile(np.array(A, np.uint8), (8, 8, 1))
        small[2:4, 2:4] = B
        self.assertIn(B, colours(reduce.reduce(small)), "kept at 4 texture pixels, where each one counts")

    # checks: PRE-22
    def test_runs_join_round_the_wrap(self):
        mask = np.zeros((6, 6), bool)
        mask[2, 0] = mask[2, 5] = True  # neighbours across the wrap
        self.assertEqual(count_runs(mask), 1)


if __name__ == "__main__":
    unittest.main()
