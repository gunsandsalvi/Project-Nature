"""The code reduction (PRE-22, PRE-20, A5.3, A5.4): each level below drawn from its own shades by majority, never
averaged, lone texture pixels cleaned, the tile still wrapping, so a band farther out grows calmer instead of
turning to speckle (answer 31)."""

import os
import sys
import unittest

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import reduce  # noqa: E402

A, B = (200, 180, 120), (60, 90, 40)


def two_shades(side=32, seed=1, share=0.5):
    rng = np.random.default_rng(seed)
    return np.where((rng.random((side, side)) < share)[..., None], np.array(A, np.uint8), np.array(B, np.uint8))


def colours(t):
    return {tuple(int(v) for v in c) for c in t.reshape(-1, 3)}


class Reduction(unittest.TestCase):
    # checks: PRE-22
    def test_each_level_is_half_the_one_above_down_to_one_texture_pixel(self):
        t = np.random.default_rng(2).integers(0, 256, (64, 64, 3)).astype(np.uint8)
        sides = []
        while t.shape[0] > 1:
            t = reduce.reduce(t)
            sides.append(t.shape[:2])
        self.assertEqual(sides, [(32, 32), (16, 16), (8, 8), (4, 4), (2, 2), (1, 1)])
        with self.assertRaises(ValueError):
            reduce.reduce(t)

    # checks: PRE-22 PRE-20
    def test_colours_are_never_averaged(self):
        low = reduce.reduce(two_shades())
        self.assertTrue(colours(low) <= {A, B}, "a mixed colour is the speckle of answer 31")

    # checks: PRE-20
    def test_coarser_levels_get_fewer_shades(self):
        rng = np.random.default_rng(3)
        self.assertLessEqual(len(colours(reduce.reduce(rng.integers(0, 256, (256, 256, 3)).astype(np.uint8)))), 12)
        self.assertLessEqual(len(colours(reduce.reduce(rng.integers(0, 256, (16, 16, 3)).astype(np.uint8)))), 6)

    # checks: PRE-22
    def test_the_tile_still_wraps(self):
        """Moving band 0 by whole blocks moves the level below by the same, wrapping: every texture pixel of the
        result depends only on its own block and its neighbours round the wrap, so a seamless level stays so."""
        t = np.random.default_rng(4).integers(0, 256, (32, 32, 3)).astype(np.uint8)
        low = reduce.reduce(t)
        for dy, dx in ((1, 0), (0, 3), (5, 7)):
            moved = reduce.reduce(np.roll(t, (2 * dy, 2 * dx), axis=(0, 1)))
            np.testing.assert_array_equal(moved, np.roll(low, (dy, dx), axis=(0, 1)))


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


if __name__ == "__main__":
    unittest.main()
