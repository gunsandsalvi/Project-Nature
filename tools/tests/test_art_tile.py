"""Band 0's tile (PRE-22, PRE-01, A5.4): quilted by code into a seamless square with no strong repeat, every
texture pixel whole from a source pixel, never blended; and the seam and repeat numbers the checks use."""

import os
import sys
import unittest

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import texels  # noqa: E402
import tile  # noqa: E402

PALETTE = np.array(
    [[70, 60, 40], [110, 90, 60], [140, 120, 80], [90, 110, 60], [170, 150, 110], [60, 80, 50]], np.uint8
)


def ground(side=160, seed=7):
    """A re-gridded source as the tile gets one: marks of a few texture pixels in six shades, not tiling."""
    rng = np.random.default_rng(seed)
    field = texels.blur(rng.normal(0, 1, (side, side, 1)), 3.0, wrap=False)[..., 0]
    return PALETTE[np.digitize(field, np.quantile(field, np.linspace(0, 1, 7)[1:-1]))]


def colours(t):
    return {tuple(c) for c in t.reshape(-1, 3)}


class Quilting(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.src = ground()
        cls.tile, cls.seed, cls.seams, cls.repeat = tile.best_tile([cls.src], size=128, step=32, overlap=8, tries=4)

    # checks: PRE-01
    def test_every_texture_pixel_comes_whole_from_a_source(self):
        self.assertEqual(self.tile.shape, (128, 128, 3))
        self.assertTrue(colours(self.tile) <= colours(self.src), "no colour blended")

    # checks: PRE-22
    def test_the_tile_is_seamless_where_a_plain_crop_is_not(self):
        self.assertLessEqual(self.seams, 1.2)
        self.assertAlmostEqual(self.seams, tile.seams(self.tile))
        self.assertGreater(tile.seams(self.src[:128, :128]), 1.2)

    # checks: PRE-22
    def test_the_tile_has_no_strong_repeat(self):
        self.assertLessEqual(self.repeat, 0.2)
        self.assertAlmostEqual(self.repeat, tile.repeat(self.tile))

    # checks: PRE-22
    def test_the_same_seed_makes_the_same_tile(self):
        again = tile.quilt([self.src], size=128, step=32, overlap=8, seed=self.seed)
        np.testing.assert_array_equal(again, self.tile)

    # checks: PRE-01
    def test_a_mask_keeps_parts_of_a_source_out(self):
        src = self.src.copy()
        src[:, 100:] = (255, 0, 255)  # a part that must not be used, such as a plant or a shadow
        keep = np.ones(src.shape[:2], bool)
        keep[:, 100:] = False
        t = tile.quilt([tile.Source(src, keep)], size=64, step=32, overlap=8)
        self.assertNotIn((255, 0, 255), colours(t))

    # checks: PRE-22
    def test_impossible_tiles_are_refused(self):
        with self.assertRaises(ValueError):
            tile.quilt([self.src], size=100, step=32)
        with self.assertRaises(ValueError):
            tile.quilt([self.src[:30, :30]], size=64, step=32, overlap=8)


class Numbers(unittest.TestCase):
    # checks: PRE-22
    def test_a_tile_that_wraps_has_no_seam(self):
        rng = np.random.default_rng(8)
        smooth = texels.blur(rng.normal(128, 40, (64, 64, 3)), 2.0, wrap=True).clip(0, 255).astype(np.uint8)
        self.assertLess(tile.seams(smooth), 1.2)
        self.assertLess(tile.seams(np.roll(smooth, (20, 33), axis=(0, 1))), 1.2)
        half = smooth.copy()
        half[:, 32:] = smooth[::-1, 32:]  # the right half turned over: its wrapping edge meets the wrong pixels
        self.assertGreater(tile.seams(half), 1.2)

    # checks: PRE-22
    def test_a_repeat_is_caught(self):
        rng = np.random.default_rng(9)
        noise = rng.integers(0, 256, (256, 256, 3)).astype(np.uint8)
        self.assertLess(tile.repeat(noise), 0.2)
        copies = np.tile(noise[:64, :64], (4, 4, 1))
        self.assertGreater(tile.repeat(copies), 0.9)


if __name__ == "__main__":
    unittest.main()
