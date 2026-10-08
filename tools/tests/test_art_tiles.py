"""The texture path's pixel work (PRE-20, PRE-22, A5.3, A5.4): a sheet's tile put on its grid, versions that share
their ring and
join without a seam, levels that halve down to one texture pixel, and the records the engine's loader reads. Small
pictures made by chance stand in for tiles, so every test runs in a second."""

import os
import sys
import tempfile
import tomllib
import unittest

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import ingest  # noqa: E402
import textures  # noqa: E402
import tiles  # noqa: E402


def grass(n=64, seed=3):
    """A wrapping tile of short marks, made by chance: smoothed noise on a torus, then stretched in contrast."""
    rng = np.random.default_rng(seed)
    noise = rng.random((n, n))
    for _ in range(2):
        for axis in (0, 1):
            noise = (np.roll(noise, 1, axis) + 2 * noise + np.roll(noise, -1, axis)) / 4
    noise = (noise - noise.min()) / (noise.max() - noise.min())
    return np.stack([60 + 100 * noise, 90 + 120 * noise, 40 + 40 * noise], axis=2).astype(np.uint8)


class Grid(unittest.TestCase):
    # checks: PRE-20
    def test_a_picture_in_blocks_is_found_and_put_on_its_grid(self):
        t = grass(32)
        big = np.repeat(np.repeat(t, 4, 0), 4, 1)
        self.assertEqual(tiles.block_size(big), 4)
        back, loss = tiles.regrid(big, 4)
        np.testing.assert_array_equal(back, t)
        self.assertEqual(loss, 0.0)
        self.assertEqual(tiles.block_size(t), 1)

    # checks: PRE-20
    def test_a_blurred_block_counts_as_loss(self):
        t = grass(32)
        big = np.repeat(np.repeat(t, 4, 0), 4, 1).copy()
        big[::4, ::4] = 255 - big[::4, ::4]  # one pixel in sixteen is wrong
        _, loss = tiles.regrid(big, 4)
        self.assertAlmostEqual(loss, 1 / 16, delta=0.02)
        with self.assertRaises(ValueError):
            tiles.regrid(big[:30], 4)


class Versions(unittest.TestCase):
    # checks: PRE-20 PRE-22
    def test_versions_share_their_ring_and_differ_inside_it(self):
        t = grass(64)
        versions, shift = tiles.make_versions(t, 3, ring=2, overlap=4, patch=16, seed=5)
        self.assertEqual(len(versions), 3)
        self.assertTrue(tiles.shares_ring(versions, 2))
        self.assertFalse(tiles.shares_ring(versions, 20), "each version has its own inside")
        self.assertGreater((versions[0] != versions[1]).mean(), 0.3)
        self.assertGreater((versions[1] != versions[2]).mean(), 0.3)
        self.assertEqual(len(shift), 2)

    # checks: PRE-20
    def test_the_same_recipe_gives_the_same_versions(self):
        t = grass(64)
        a, _ = tiles.make_versions(t, 2, 2, 4, 16, seed=9)
        b, _ = tiles.make_versions(t, 2, 2, 4, 16, seed=9)
        c, _ = tiles.make_versions(t, 2, 2, 4, 16, seed=10)
        np.testing.assert_array_equal(a[1], b[1])
        self.assertFalse((a[1] == c[1]).all())

    # checks: PRE-22
    def test_any_two_versions_join_without_a_seam(self):
        t = grass(64)
        versions, _ = tiles.make_versions(t, 3, 2, 4, 16, seed=1)
        for a in versions:
            self.assertLessEqual(tiles.wrap_ratio(a), 1.2)
            for b in versions:
                self.assertLessEqual(tiles.join_ratio(a, b), 1.2)

    # checks: PRE-20 PRE-22
    def test_a_level_drawn_for_each_source_is_quilted_at_its_own_size(self):
        t, other = grass(64, 3), grass(64, 4)
        half = [np.rint(tiles.halve(x)).astype(np.uint8) for x in (t, other)]
        _, shift = tiles.make_versions(t, 2, 4, 8, 16, seed=2, others=[other])
        levels = tiles.requilt(half, shift, 3, 2, 4, 8, seed=3, scale=2)
        self.assertEqual([v.shape[0] for v in levels], [32, 32, 32])
        self.assertTrue(tiles.shares_ring(levels, 2), "the ring is shared at the next level too")
        self.assertFalse(tiles.shares_ring(levels, 8), "and each version has its own inside")
        again = tiles.requilt(half, shift, 3, 2, 4, 8, seed=3, scale=2)
        np.testing.assert_array_equal(again[2], levels[2])  # the same recipe gives the same bytes
        rolled = tiles.roll(half[0], shift[0] // 2, shift[1] // 2)
        np.testing.assert_array_equal(levels[0][16, 16], rolled[16, 16])  # the first is the first source, shifted

    # checks: PRE-20
    def test_the_shift_counts_the_levels_drawn_below_as_it_counts_the_tile(self):
        t = grass(64, 3)
        plain = tiles.neutral_shift(t, 4, step=8)
        self.assertEqual(plain, tiles.neutral_shift(t, 4, step=8, deeper=()))
        half = np.rint(tiles.halve(grass(64, 7))).astype(np.uint8)  # a level of another picture, so another best cut
        both = tiles.neutral_shift(t, 4, step=8, deeper=[half])
        self.assertEqual(both[0] % 8, 0)
        self.assertEqual(both[1] % 8, 0)

    # checks: PRE-22
    def test_a_hard_seam_is_seen(self):
        t = grass(64)
        torn = np.concatenate([t[:, :32], 255 - t[:, :32]], axis=1)  # a different picture beside it
        self.assertGreater(tiles.join_ratio(t, torn), 1.5)

    # checks: PRE-20
    def test_a_ring_with_a_flat_tone_has_no_broad_tone_left(self):
        t = grass(64)
        flat = tiles.flatten_border(t, 8)
        mean = t.astype(float).mean()
        rows = flat.astype(float).mean(axis=(1, 2))
        self.assertLess(abs(rows[0] - mean), 0.6)
        self.assertLess(abs(flat.astype(float).mean(axis=(0, 2))[0] - mean), 0.6)
        self.assertGreater(abs(t.astype(float).mean(axis=(1, 2))[0] - mean), 0.6, "the tile's own edge row is not flat")
        np.testing.assert_array_equal(flat[32, 32], t[32, 32])  # the middle is left alone

    # checks: PRE-20
    def test_the_shift_keeps_a_wrapping_tile_wrapping(self):
        t = grass(64)
        dy, dx = tiles.neutral_shift(t, 4, step=8)
        self.assertEqual(sorted(tiles.roll(t, dy, dx).ravel().tolist()), sorted(t.ravel().tolist()))

    # checks: PRE-20
    def test_a_border_of_depth_nothing_is_not_flattened(self):
        t = grass(64)
        np.testing.assert_array_equal(tiles.flatten_border(t, 0), t)
        versions, _ = tiles.make_versions(t, 2, 4, 8, 32, 5, flatten=0)
        self.assertTrue(tiles.shares_ring(versions, 4))


class Drawings(unittest.TestCase):
    """Pictures from the image tool: pixel art in cells of about four pixels, no exact grid, no wrap."""

    @staticmethod
    def drawn(cells=48, seed=2):
        """A blocky picture of `cells` x `cells` flat cells, each four pixels wide and every seventh five (as the tool
        returns them), and the cells it was made from."""
        rng = np.random.default_rng(seed)
        small = rng.integers(0, 256, (cells, cells, 3)).astype(np.uint8)
        widths = [5 if k % 7 == 3 else 4 for k in range(cells)]
        rows = np.repeat(np.arange(cells), widths)
        return small, small[rows[:, None], rows[None, :]]

    # checks: PRE-20
    def test_a_drawing_on_no_exact_grid_is_put_on_its_cells(self):
        small, big = self.drawn()
        texels, loss = tiles.snap(big, 4)
        self.assertEqual(texels.shape, small.shape)
        np.testing.assert_array_equal(texels, small)
        self.assertLess(loss, 0.02)

    # checks: PRE-20
    def test_a_drawing_with_cells_cut_at_the_ends_loses_only_those(self):
        small, big = self.drawn()
        cut = big[2:, 3:]  # the first cells partly gone, as a crop leaves them
        texels, _ = tiles.snap(cut, 4)
        self.assertLessEqual(abs(texels.shape[0] - small.shape[0]), 1)
        self.assertGreater((texels[:40, :40] == small[1:41, 1:41]).all(axis=2).mean(), 0.97)

    # checks: PRE-20
    def test_a_drawing_with_a_cell_count_is_cut_in_that_many_even_cells(self):
        small, big = self.drawn(cells=48)
        texels, loss = tiles.snap(big, 4, cells=48)
        self.assertEqual(texels.shape, small.shape)
        # the cells the tool made are a pixel wider or narrower now and then, so a few of them are a little off
        self.assertGreater((texels == small).all(axis=2).mean(), 0.9)
        self.assertLess(loss, 0.1)
        half, _ = tiles.snap(big, 4, cells=24)
        self.assertEqual(half.shape[:2], (24, 24))

    # checks: PRE-22 PRE-46
    def test_a_join_weight_makes_the_cut_keep_clear_of_a_hard_crack_and_nothing_less_does(self):
        t = grass(64, 3).copy()
        t[:, 30] = 255  # a crack down every row, two columns wide, black beside white
        t[:, 31] = 0
        across = np.abs(t.astype(float) - np.roll(t.astype(float), -1, axis=1)).sum(axis=2)
        columns = across.mean(axis=0) / across.mean()  # how hard each column jumps to the next
        cracked = (-33 - 1) % 64  # the shift that puts the cut across the crack
        self.assertGreater(columns[cracked], 10.0)
        heavy = tiles._ring_scorer(t, 4, 12, 0.07, 3.0)
        none = tiles._ring_scorer(t, 4, 12, 0.07, 0.0)
        self.assertGreater(heavy(0, 33) - heavy(0, 10), none(0, 33) - none(0, 10) + 30.0)
        dy, dx = tiles.neutral_shift(t, 4, step=1, join_weight=3.0)
        self.assertLess(columns[(-dx - 1) % 64], 1.0, "the cut does not run across the crack")
        # open versions made with no weight are still shared at the ring and have no seam
        versions = tiles.make_versions_open([grass(120, 4)], 3, 4, 8, 32, 9, 96, join_weight=0.0)
        self.assertTrue(tiles.shares_ring(versions, 4))
        self.assertEqual(tiles.inner_seams(versions[1]), [])

    # checks: PRE-22
    def test_the_four_corners_of_a_frame_are_neighbours_in_the_picture(self):
        n, o = 64, 8
        ys, xs = np.mgrid[0:96, 0:96]
        coordinates = np.stack([ys, xs, np.zeros_like(ys)], axis=2).astype(np.uint8)  # each pixel says where it is
        frame = tiles.frame_of(coordinates, o, n)
        # tile a's bottom right, tile b's bottom left: the same row, one column along
        self.assertEqual(frame[n - 1, n - 1, 0], frame[n - 1, 0, 0])
        self.assertEqual(int(frame[n - 1, n - 1, 1]) + 1, int(frame[n - 1, 0, 1]))
        # a's bottom right and its lower neighbour's top right: the same column, one row down
        self.assertEqual(frame[n - 1, n - 1, 1], frame[0, n - 1, 1])
        self.assertEqual(int(frame[n - 1, n - 1, 0]) + 1, int(frame[0, n - 1, 0]))
        # the strips run on over the wrap, bottom into top
        self.assertEqual(int(frame[n - 1, 20, 0]) + 1, int(frame[0, 20, 0]))
        self.assertEqual(int(frame[20, n - 1, 1]) + 1, int(frame[20, 0, 1]))

    # checks: PRE-22 PRE-46
    def test_versions_quilted_from_drawings_that_do_not_wrap_share_a_ring_and_have_no_seam(self):
        pictures = [grass(120, 4), grass(120, 5)]
        versions = tiles.make_versions_open(pictures, 3, 4, 8, 32, 9, 96)
        self.assertEqual(len(versions), 3)
        self.assertTrue(tiles.shares_ring(versions, 4))
        self.assertFalse((versions[0] == versions[1]).all())
        for v in versions:
            self.assertEqual(tiles.inner_seams(v), [])
        worst = max(tiles.join_ratio(a, b) for a in versions for b in versions)
        self.assertLess(worst, 1.2)

    # checks: PRE-20 PRE-22
    def test_a_drawing_that_wraps_is_one_tile_with_a_shift_not_a_quilt_from_a_bigger_picture(self):
        import make_tiles

        spec = {"snap": 4, "wraps": True, "versions": 3, "ring": 4, "overlap": 8, "patch": 32, "serves": 1}
        chains, _, shift, block, _ = make_tiles.make_tile(spec, None, 5, given=(grass(64, 6), 4, 0.0))
        self.assertIsNotNone(shift, "a wrapping drawing is shifted round its wrap, as any wrapping tile is")
        self.assertEqual(block, 4)
        self.assertEqual(chains[0][0].shape, (64, 64, 3))
        self.assertTrue(tiles.shares_ring([c[0] for c in chains], 4))
        for c in chains:
            self.assertLessEqual(tiles.wrap_ratio(c[0]), 1.2)
        _, _, open_shift, _, _ = make_tiles.make_tile(
            {**spec, "wraps": False, "texels": 48}, None, 5, given=(grass(96, 6), 4, 0.0)
        )
        self.assertIsNone(open_shift, "a drawing that does not wrap is quilted from, with no shift")

    # checks: PRE-22 PRE-46
    def test_a_flattened_ring_of_open_versions_is_at_the_tiles_mean_and_still_shared(self):
        swell = np.sin(np.arange(120) / 9.0)[None, :, None] * 40.0  # a swathe of tone across the drawing's columns
        pictures = [np.clip(grass(120, 4).astype(float) + swell, 0, 255).astype(np.uint8)]
        plain = tiles.make_versions_open(pictures, 3, 4, 8, 32, 9, 96)
        flat = tiles.make_versions_open(pictures, 3, 4, 8, 32, 9, 96, flatten=16)
        self.assertTrue(tiles.shares_ring(flat, 4))

        def ring_tone(v):
            columns = v.astype(float).mean(axis=(0, 2))
            return abs(columns[:4].mean() - columns.mean())

        self.assertLess(ring_tone(flat[1]), ring_tone(plain[1]) + 1e-9)
        self.assertLess(ring_tone(flat[1]), 1.5)


class Seams(unittest.TestCase):
    # checks: PRE-22
    def test_a_hard_row_inside_a_level_is_found_and_a_plain_tile_has_none(self):
        t = grass(64)
        self.assertEqual(tiles.inner_seams(t), [])
        torn = t.copy()
        torn[40:] = np.clip(torn[40:].astype(int) + 70, 0, 255)  # everything below row 39 lighter by a step
        found = tiles.inner_seams(torn)
        self.assertEqual([(s[0], s[1]) for s in found], [("row", 39)])
        self.assertEqual(
            tiles.inner_seams(torn[:16, :16]), [], "a level under 32 pixels has no 99th percentile to judge"
        )

    # checks: PRE-22
    def test_patches_reach_into_the_border_on_every_side_and_overlap_everywhere(self):
        for n, patch, overlap in ((256, 48, 8), (256, 64, 8), (128, 24, 4), (256, 112, 32), (64, 64, 8), (32, 48, 8)):
            starts = tiles.patch_starts(n, patch, overlap)
            self.assertEqual(starts[0], 0)
            self.assertEqual(starts[-1], max(n - patch, 0), "the last patch ends at the far edge")
            for a, b in zip(starts, starts[1:], strict=False):
                self.assertGreaterEqual(a + patch - b, overlap, "neighbours overlap by at least the overlap")

    # checks: PRE-22
    def test_quilted_versions_have_no_hard_row_or_column_where_patches_meet_the_border(self):
        t = grass(256, 11)
        versions, _ = tiles.make_versions(t, 3, 4, 8, 48, 5)
        for v in versions:
            self.assertEqual(tiles.inner_seams(v), [])


class Strips(unittest.TestCase):
    """A hide's seam strip: a few columns of a pattern that repeats down the tile, laid over its first columns."""

    def drawn(self):
        pattern = np.zeros((4, 4, 3), np.uint8)
        pattern[0, :3] = (230, 190, 120)  # the stitch, three texture pixels wide
        pattern[0, 3] = (80, 55, 35)  # its dark end
        pattern[1:, 0] = (60, 40, 30)  # the seam's own edge
        drawing = np.tile(pattern, (16, 1, 1))
        drawing[5, 1] = (0, 0, 0)  # one stray cell in one repeat
        return pattern, np.concatenate([drawing, np.full((64, 60, 3), 150, np.uint8)], axis=1)

    # checks: PRE-46
    def test_the_strip_is_the_colour_most_repeats_have_and_a_stray_cell_is_not_carried(self):
        pattern, drawing = self.drawn()
        np.testing.assert_array_equal(tiles.seam_strip(drawing, 4, 4), pattern)

    # checks: PRE-46
    def test_a_strip_is_laid_down_the_first_columns_and_wraps_with_the_tile(self):
        pattern, _ = self.drawn()
        tile = grass(64)
        laid = tiles.lay_strip(tile, pattern)
        np.testing.assert_array_equal(laid[:, 4:], tile[:, 4:])
        for start in (0, 4, 60):
            np.testing.assert_array_equal(laid[start : start + 4, :4], pattern)
        with self.assertRaises(ValueError):
            tiles.lay_strip(grass(66)[:66, :66], pattern)

    # checks: PRE-22
    def test_a_field_wraps_down_alone_when_its_left_edge_is_a_strip(self):
        t = grass(64)
        torn = t.copy()
        torn[:, 0] = 255  # the left edge differs from the right: a seam across, none down
        self.assertGreater(tiles.wrap_ratio(torn), 1.2)
        self.assertLess(tiles.wrap_ratio(torn, across=False), 1.2)
        self.assertLess(tiles.join_ratio(torn, torn, across=False), 1.2)


class Marks(unittest.TestCase):
    KEY = np.array([255, 0, 255], np.uint8)

    def streaks(self, n=64):
        """Light streaks, one texture pixel high and 6 to 8 long, on the key colour."""
        pic = np.empty((n, n, 3), np.uint8)
        pic[:] = self.KEY
        for k, y in enumerate(range(3, n, 8)):
            x = (k * 13) % (n - 10)
            pic[y, x : x + 6 + k % 3] = (232, 222, 202)
        pic[10, 40] = (170, 196, 182)  # a single speck
        return pic

    # checks: PRE-20 PRE-46
    def test_marks_become_opaque_and_the_rest_see_through_with_the_bleed_colour(self):
        pic = self.streaks()
        out = tiles.to_rgba(pic, self.KEY, (170, 196, 182))
        self.assertEqual(out.shape, (64, 64, 4))
        self.assertEqual(set(np.unique(out[..., 3]).tolist()), {0, 255})
        np.testing.assert_array_equal(out[0, 0], [170, 196, 182, 0])
        np.testing.assert_array_equal(out[3, 0:6, :3], pic[3, 0:6])
        self.assertAlmostEqual(tiles.coverage(out), tiles.key_mask(pic, self.KEY).mean())

    # checks: PRE-20 PRE-22
    def test_the_level_below_marks_is_half_the_size_with_no_speck_and_fewer_marks(self):
        pic = self.streaks()
        all_kept = tiles.reduce_marks(pic, self.KEY, 100, 1)
        self.assertEqual(all_kept.shape, (32, 32, 3))
        self.assertFalse(tiles.key_mask(all_kept, self.KEY)[5, 20], "the single speck is gone")
        fewer = tiles.reduce_marks(pic, self.KEY, 40, 1)
        self.assertLess(tiles.key_mask(fewer, self.KEY).sum(), tiles.key_mask(all_kept, self.KEY).sum())
        np.testing.assert_array_equal(fewer, tiles.reduce_marks(pic, self.KEY, 40, 1))  # nothing is random

    # checks: PRE-22
    def test_versions_of_marks_share_their_ring_on_the_key_colour(self):
        pic = self.streaks(96)
        versions, _ = tiles.make_versions(pic, 3, 4, 8, 32, 5, flatten=0)
        self.assertTrue(tiles.shares_ring(versions, 4))
        for v in versions:  # quilting copies marks, never mixes them with the key
            colours = {tuple(c) for c in v.reshape(-1, 3).tolist()}
            self.assertLessEqual(colours, {(255, 0, 255), (232, 222, 202), (170, 196, 182)})


class Levels(unittest.TestCase):
    # checks: PRE-22
    def test_levels_halve_down_to_one_texture_pixel(self):
        t = grass(64)
        chain = tiles.complete_chain([t, tiles.reduce(t, 0.5)])
        self.assertEqual([c.shape[0] for c in chain], [64, 32, 16, 8, 4, 2, 1])
        self.assertEqual(chain[0].dtype, np.uint8)

    # checks: PRE-22
    def test_sharpening_keeps_more_contrast_than_averaging(self):
        t = grass(64)
        plain = tiles.reduce(t, 0.0).astype(float)
        sharp = tiles.reduce(t, 1.5).astype(float)
        self.assertGreater(sharp.std(), plain.std())
        self.assertAlmostEqual(sharp.mean(), plain.mean(), delta=1.0)

    # checks: PRE-20
    def test_a_ring_can_be_put_on_a_level(self):
        a, b = grass(32, 1), grass(32, 2)
        out = tiles.impose_ring(a, b, 2)
        np.testing.assert_array_equal(out[:2], b[:2])
        np.testing.assert_array_equal(out[:, -2:], b[:, -2:])
        np.testing.assert_array_equal(out[8:24, 8:24], a[8:24, 8:24])


class Ladder(unittest.TestCase):
    """A far ground made from a nearer tile's coarser level, and a level's contrast boosted after it is made."""

    def versions_with_levels(self):
        versions, _ = tiles.make_versions(grass(128, 21), 3, 4, 8, 32, 7)
        return [tiles.complete_chain([v]) for v in versions]

    # checks: PRE-20, PRE-22
    def test_a_mosaic_of_a_nearer_levels_versions_is_a_whole_tile_that_wraps_without_a_seam(self):
        import make_tiles

        chains = self.versions_with_levels()
        mosaic = make_tiles.coarse_ground(chains, 1, seed=5)
        self.assertEqual(mosaic.shape, (4 * 64, 4 * 64, 3))
        picked = {c[1].tobytes() for c in chains}
        for cy in range(4):
            for cx in range(4):
                self.assertIn(mosaic[cy * 64 : (cy + 1) * 64, cx * 64 : (cx + 1) * 64].tobytes(), picked)
        self.assertLess(tiles.wrap_ratio(mosaic), 1.2)
        np.testing.assert_array_equal(mosaic, make_tiles.coarse_ground(chains, 1, seed=5))

    # checks: PRE-20
    def test_a_boost_scales_every_version_alike_so_their_shared_ring_stays_shared_and_nothing_new_is_a_seam(self):
        versions, _ = tiles.make_versions(grass(128, 22), 3, 4, 8, 32, 8)
        pivot = versions[0].reshape(-1, 3).mean(axis=0)
        boosted = [tiles.boost(v, 110.0, pivot) for v in versions]
        self.assertTrue(tiles.shares_ring(boosted, 4))
        self.assertGreater(boosted[0].astype(float).std(), versions[0].astype(float).std())
        np.testing.assert_array_equal(tiles.boost(versions[0], 100.0, pivot), versions[0])
        self.assertEqual(tiles.inner_seams(boosted[1]), [])


class Calm(unittest.TestCase):
    """A far ground's contrast grouped by broad swells, calm ground between the groups."""

    # checks: PRE-20 A4.8
    def test_a_calm_field_is_one_on_average_between_calm_and_twice_less_calm_and_one_at_the_border(self):
        field = tiles.calm_field(128, 5, 0.55, edge=4 / 256, ramp=24 / 256)
        self.assertAlmostEqual(field[64, 64], field[64, 64])  # a field of numbers
        self.assertGreaterEqual(field.min(), 0.55 - 1e-9)
        self.assertLessEqual(field.max(), 1.45 + 1e-9)
        self.assertLess(field.min(), 0.8, "there is calm ground")
        self.assertGreater(field.max(), 1.2, "and busy ground")
        self.assertAlmostEqual(float(field.mean()), 1.0, delta=0.15)
        self.assertEqual(set(np.unique(field[:2])), {1.0}, "the ring is left alone")
        self.assertEqual(set(np.unique(field[:, -2:])), {1.0})
        np.testing.assert_array_equal(field, tiles.calm_field(128, 5, 0.55, edge=4 / 256, ramp=24 / 256))
        self.assertFalse((field == tiles.calm_field(128, 6, 0.55, edge=4 / 256, ramp=24 / 256)).all())

    # checks: PRE-20 PRE-22 A4.8
    def test_each_version_calmed_by_a_field_of_its_own_keeps_the_ring_they_share_and_the_marks_between_them_differ(
        self,
    ):
        versions, _ = tiles.make_versions(grass(128, 31), 3, 4, 8, 32, 9)
        pivot = versions[0].reshape(-1, 3).mean(axis=0)
        made = [
            tiles.calmed(v, tiles.calm_field(128, 7 * (k + 1), 0.5, 4 / 128, 24 / 128), pivot)
            for k, v in enumerate(versions)
        ]
        self.assertTrue(tiles.shares_ring(made, 4))
        self.assertEqual(tiles.inner_seams(made[1]), [])
        self.assertLess(tiles.wrap_ratio(made[2]), 1.2)
        # the contrast of its patches is spread wider than before: it is grouped
        spread = [
            float(
                np.abs(m.astype(float) - pivot).mean(axis=2)[32:96, 32:96].reshape(8, 8, 8, 8).mean(axis=(1, 3)).std()
            )
            for m in made
        ]
        plain = float(
            np.abs(versions[1].astype(float) - pivot)
            .mean(axis=2)[32:96, 32:96]
            .reshape(8, 8, 8, 8)
            .mean(axis=(1, 3))
            .std()
        )
        self.assertGreater(min(spread), plain, "the contrast of its patches is less alike than it was")


class Records(unittest.TestCase):
    FIELDS = {
        "about": 'a "quoted" thing',
        "route": "picture",
        "tile_texels": 256,
        "texels_a_metre": 64,
        "first_band": 0,
        "sources": ["art/sources/x/a.webp"],
        "original_sha256": ["ab"],
        "c2pa": ["present"],
        "requests": [],
        "made": "made",
        "regrid_loss": "0%",
        "truth": "truth",
        "approved": "waiting",
    }

    # checks: PRE-20 PRE-42
    def test_a_record_reads_back_as_written(self):
        levels = [
            {"level": 0, "file": "art/textures/x/b0.png", "sha256": "aa", "made_from": "", "way": "drawn"},
            {
                "level": 1,
                "file": "art/textures/x/b1.png",
                "sha256": "bb",
                "made_from": "aa",
                "way": "by code",
                "calibration": "lightness +1.0",
            },
        ]
        record = tomllib.loads(textures.record_text(self.FIELDS, levels))
        self.assertEqual(record["about"], 'a "quoted" thing')
        self.assertEqual(record["tile_texels"], 256)
        self.assertEqual(record["requests"], [])
        self.assertEqual([b["level"] for b in record["band"]], [0, 1])
        self.assertNotIn("made_from", record["band"][0])  # the first level has no level above it
        self.assertEqual(record["band"][1]["made_from"], "aa")

    # checks: PRE-20
    def test_each_tile_and_version_has_its_own_folder_and_name(self):
        self.assertEqual(textures.folder("meadow", "near", 1), "art/textures/meadow")
        self.assertEqual(textures.folder("meadow", "near", 3), "art/textures/meadow/v3")
        self.assertEqual(textures.folder("meadow", "middle", 2), "art/textures/meadow/middle/v2")
        self.assertEqual(textures.folder("meadow", "far", 1), "art/textures/meadow/far")
        self.assertEqual(textures.entry_name("meadow", "middle", 2), "art:meadow/middle/v2")

    # checks: PRE-20 PRE-42
    def test_a_source_is_kept_exactly_with_its_digest_and_its_provenance_word(self):
        with tempfile.TemporaryDirectory() as d:
            png = os.path.join(d, "a.png")
            tiles.write_png(png, grass(32))
            digest, c2pa = ingest.ingest(png, os.path.join(d, "out", "a.webp"))
            self.assertEqual(digest, tiles.sha256(png))
            self.assertEqual(c2pa, "absent")
            np.testing.assert_array_equal(tiles.read_rgb(os.path.join(d, "out", "a.webp")), grass(32))
            with open(png, "rb") as f:
                data = f.read()
            marked = data[:33] + b"\x00\x00\x00\x04caBXabcd\x00\x00\x00\x00" + data[33:]
            with open(os.path.join(d, "b.png"), "wb") as f:
                f.write(marked)
            self.assertTrue(ingest.has_c2pa(os.path.join(d, "b.png")))


if __name__ == "__main__":
    unittest.main()
