"""Re-gridding (PRE-22, PRE-01, A5.4): GPT's drifting blocks found and brought onto an exact grid of texture
pixels, each block's median colour a texture pixel; a painted picture sampled in fixed blocks; painted light taken
out; and the accepted swatches re-gridded within their measured loss."""

import os
import sys
import unittest

import numpy as np

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(TOOLS)
sys.path.insert(0, os.path.join(TOOLS, "art"))
import regrid  # noqa: E402
import texels  # noqa: E402


def drifting(n=24, p=9.3, drift=1.5, noise=3.0, seed=4):
    """A picture drawn as GPT draws one asked for blocks: n x n blocks of about p pixels, the column edges wandering
    with x and the row edges with y, a little noise inside each, cut half a block in at every side. Returns the
    picture, the true colours and the true edges in the picture's own pixels."""
    rng = np.random.default_rng(seed)
    true = rng.integers(0, 256, (n + 2, n + 2, 3)).astype(np.float64)
    k = np.arange(n + 3)
    ex = np.round(k * p + drift * np.sin(k / 3.0)).astype(int)
    ey = np.round(k * p + drift * np.cos(k / 4.0)).astype(int)
    h, w = ey[-1], ex[-1]
    col = np.searchsorted(ex, np.arange(w), side="right") - 1
    row = np.searchsorted(ey, np.arange(h), side="right") - 1
    a = np.clip(np.round(true[row][:, col] + rng.normal(0, noise, (h, w, 3))), 0, 255).astype(np.uint8)
    off = int(p / 2)
    return a[off : h - off, off : w - off], true, ex - off, ey - off


class Blocks(unittest.TestCase):
    # checks: PRE-22 PRE-01
    def test_a_drifting_grid_is_followed_block_by_block(self):
        a, true, ex, ey = drifting()
        grid, p, strength, lost = regrid.find(a)
        self.assertLess(abs(p - 9.3), 0.5)
        xs = regrid.edges(regrid.profile(a, 1), p)
        ys = regrid.edges(regrid.profile(a, 0), p)
        self.assertTrue(set(xs) <= set(ex) and set(ys) <= set(ey), "every edge placed on a true edge")
        cols = np.searchsorted(ex, (xs[:-1] + xs[1:]) / 2, side="right") - 1
        rows = np.searchsorted(ey, (ys[:-1] + ys[1:]) / 2, side="right") - 1
        self.assertEqual(len(set(cols)), len(cols), "no block skipped or taken twice")
        self.assertEqual(len(set(rows)), len(rows))
        self.assertGreaterEqual(min(grid.shape[:2]), 23)  # at most the cut blocks at the ends are left out
        self.assertLessEqual(np.abs(grid.astype(float) - true[rows][:, cols]).max(), 1.0)
        self.assertLess(lost, 0.01)

    # checks: PRE-22
    def test_the_peak_is_strong_for_blocks_and_weak_for_noise(self):
        a, _, _, _ = drifting()
        self.assertGreater(regrid.block_size(a)[1], 0.4)
        noise = np.random.default_rng(1).integers(0, 256, (200, 200, 3)).astype(np.uint8)
        self.assertLess(regrid.block_size(noise)[1], 0.15)

    # checks: PRE-22
    def test_a_picture_that_tiles_fills_its_blocks_exactly(self):
        rng = np.random.default_rng(2)
        n, p = 20, 12.93
        true = rng.integers(0, 256, (n, n, 3)).astype(np.uint8)
        side = int(round(n * p))
        idx = np.minimum((np.arange(side) / p).astype(int), n - 1)
        pic = np.roll(true[idx][:, idx], (5, 7), axis=(0, 1))
        grid, _, _, lost = regrid.find(pic, tiles=True)
        self.assertEqual(grid.shape, (n, n, 3))
        self.assertEqual(lost, 0.0)
        shifts = [(sy, sx) for sy in (-1, 0, 1) for sx in (-1, 0, 1)]
        self.assertTrue(
            any((np.roll(true, s, axis=(0, 1)) == grid).all() for s in shifts),
            "the same blocks, turned back to within a block of where the picture began",
        )

    # checks: PRE-22
    def test_blocks_that_cannot_fill_a_tile_exactly_are_refused(self):
        with self.assertRaises(ValueError):
            regrid.edges(np.ones(4), 10.0, closed=True)

    # checks: PRE-01
    def test_a_painted_picture_is_sampled_in_fixed_blocks(self):
        rng = np.random.default_rng(3)
        true = rng.integers(0, 256, (6, 5, 3)).astype(np.uint8)
        np.testing.assert_array_equal(regrid.fixed(texels.enlarge(true, 7), 7), true)
        tilted = np.repeat(np.repeat(true, 4, axis=0), 8, axis=1)  # blocks 8 across and 4 down
        np.testing.assert_array_equal(regrid.fixed(tilted, 8, squash=0.5), true)

    # checks: PRE-01
    def test_swatches_are_found_on_the_grey_background(self):
        a = np.full((300, 700, 3), regrid.GREY, np.uint8)
        a[20:220, 30:230] = 40
        a[20:220, 260:460] = 200
        a[30:250, 490:690] = (10, 120, 30)
        self.assertEqual(regrid.swatches(a), [(30, 20, 230, 220), (260, 20, 460, 220), (490, 30, 690, 250)])


class Light(unittest.TestCase):
    # checks: PRE-20
    def test_painted_light_is_taken_out_and_the_mean_kept(self):
        rng = np.random.default_rng(5)
        grain = 0.35 + 0.1 * rng.random((96, 96, 1)) * np.ones((1, 1, 3))
        ramp = np.linspace(0.6, 1.4, 96)[None, :, None]  # light painted from the left
        t = texels.to_srgb(grain * ramp)
        out = regrid.delight(t, wrap=False)
        before = texels.luminance(t).mean(axis=0)
        after = texels.luminance(out).mean(axis=0)
        self.assertLess(after.std(), 0.25 * before.std())
        self.assertLess(abs(texels.luminance(out).mean() - texels.luminance(t).mean()), 0.01)

    # checks: PRE-20
    def test_a_mask_keeps_shade_out_of_the_broad_light(self):
        t = np.full((64, 64, 3), 186, np.uint8)
        t[20:44, 20:44] = 40  # a shaded hollow
        keep = np.ones((64, 64), bool)
        keep[20:44, 20:44] = False
        masked = regrid.delight(t, mask=keep)
        self.assertLessEqual(np.abs(masked[keep].astype(int) - 186).max(), 1)
        plain = regrid.delight(t)
        self.assertGreater(np.abs(plain[keep].astype(int) - 186).max(), 5)


class Fitting(unittest.TestCase):
    # checks: PRE-22 PRE-01
    def test_a_grid_is_fitted_by_whole_rows_and_columns(self):
        t = np.zeros((97, 97, 3), np.uint8)
        t[..., 0] = np.arange(97)[:, None]  # each texture pixel names its row and column
        t[..., 1] = np.arange(97)[None, :]
        for side in (128, 32):
            out = regrid.fit(t, side, side)
            self.assertEqual(out.shape, (side, side, 3))
            ys, xs = out[:, 0, 0].astype(int), out[0, :, 1].astype(int)
            np.testing.assert_array_equal(out[..., 0], np.repeat(ys[:, None], side, axis=1))  # whole rows
            np.testing.assert_array_equal(out[..., 1], np.repeat(xs[None, :], side, axis=0))  # whole columns
            for steps in (np.diff(ys), np.diff(xs)):  # in order, and spread evenly
                self.assertGreaterEqual(steps.min(), 0)
                self.assertLessEqual(steps.max() - steps.min(), 1)


class StudyFive(unittest.TestCase):
    # checks: PRE-22
    def test_study_5s_swatches_lose_2_to_8_percent(self):
        """The accepted swatches lose 2 to 8%. The
        committed copy is lossy WebP, which adds about a point inside each block: the originals lose 2.9 to 7.1%
        with this tool and the committed copy 3.0 to 8.6%, so each is held to the brief's 10% and their median to
        2 to 8%."""
        a = texels.load(os.path.join(ROOT, "art", "targets", "delight-liked.webp"))
        boxes = regrid.swatches(a)
        self.assertEqual(len(boxes), 6)
        losses = []
        for x0, y0, x1, y1 in boxes:
            grid, p, _, lost = regrid.find(a[y0 + 24 : y1 - 24, x0 + 24 : x1 - 24])
            self.assertLess(abs(p - 6), 1.5, "GPT drew blocks of about 6 pixels (4 asked)")
            losses.append(100 * lost)
        self.assertTrue(all(2 <= x <= 10 for x in losses), losses)
        self.assertTrue(2 <= float(np.median(losses)) <= 8, losses)


if __name__ == "__main__":
    unittest.main()
