"""Band 0's tile (PRE-22, PRE-01, A5.3, A5.4): quilted by code into a seamless square with no strong repeat, every
texture pixel whole from a source pixel, never blended; its versions, sharing its edges so any joins any other, laid
out alike down its levels; and the seam, joint and repeat numbers the checks use."""

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


def levels_of(t, count=3):
    """A tile's first levels, each the 2 x 2 majority of the one above, enough for versions to follow."""
    import reduce

    out = [t]
    for _ in range(count - 1):
        out.append(reduce.reduce(out[-1]))
    return out


class Versions(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        t = tile.quilt([ground()], size=128, step=32, overlap=8, seed=2)
        cls.levels = levels_of(t)
        cls.versions = [cls.levels] + [tile.version(cls.levels, seed)[0] for seed in (5, 6, 7)]

    # checks: PRE-22
    def test_versions_share_the_tiles_edges(self):
        for v in self.versions[1:]:
            for n, (a, b) in enumerate(zip(v, self.levels, strict=True)):
                f = max(1, 4 >> n)
                for edge in (a[:f] - b[:f], a[-f:] - b[-f:], a[:, :f] - b[:, :f], a[:, -f:] - b[:, -f:]):
                    self.assertFalse(edge.any(), f"level {n}: an edge differs")

    # checks: PRE-22
    def test_any_version_joins_any_other_without_a_seam(self):
        own = tile.seams(self.levels[0])
        for a in self.versions:
            for b in self.versions:
                self.assertLessEqual(tile.joints(a[0], b[0]), max(1.2, own + 0.05))

    # checks: PRE-22
    def test_each_version_differs_inside(self):
        for v in self.versions[1:]:
            self.assertGreater((v[0] != self.levels[0]).any(-1).mean(), 0.5)
        self.assertGreater((self.versions[1][0] != self.versions[2][0]).any(-1).mean(), 0.5)

    # checks: PRE-22 PRE-01
    def test_a_versions_levels_follow_its_own_layout(self):
        """Level 1 of a version is the tile's own level 1 laid out as its level 0 was: it matches the version's level
        0 brought to its size far better than it matches the tile's."""
        v = self.versions[1]
        half = levels_of(v[0], 2)[1]
        mine = (v[1] == half).all(-1).mean()
        theirs = (self.levels[1] == half).all(-1).mean()
        self.assertGreater(mine, theirs + 0.2)
        colours = {tuple(c) for c in self.levels[1].reshape(-1, 3)}
        self.assertTrue({tuple(c) for c in v[1].reshape(-1, 3)} <= colours, "a level's colours all its own")

    # checks: PRE-22
    def test_a_joint_between_tiles_that_do_not_match_is_caught(self):
        a = self.levels[0]
        self.assertAlmostEqual(tile.joints(a, a), tile.seams(a), places=6)
        self.assertGreater(tile.joints(a, a[::-1]), 1.2)


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
