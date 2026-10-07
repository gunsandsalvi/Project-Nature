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
        versions, shift, ops = tiles.make_versions(t, 3, ring=2, overlap=4, patch=16, seed=5)
        self.assertEqual(len(versions), 3)
        self.assertIsNone(ops[0])  # the first version is the tile itself, quilted from nothing
        self.assertGreater(len(ops[1]), 10)
        self.assertTrue(tiles.shares_ring(versions, 2))
        self.assertFalse(tiles.shares_ring(versions, 20), "each version has its own inside")
        self.assertGreater((versions[0] != versions[1]).mean(), 0.3)
        self.assertGreater((versions[1] != versions[2]).mean(), 0.3)
        self.assertEqual(len(shift), 2)

    # checks: PRE-20
    def test_the_same_recipe_gives_the_same_versions(self):
        t = grass(64)
        a, _, _ = tiles.make_versions(t, 2, 2, 4, 16, seed=9)
        b, _, _ = tiles.make_versions(t, 2, 2, 4, 16, seed=9)
        c, _, _ = tiles.make_versions(t, 2, 2, 4, 16, seed=10)
        np.testing.assert_array_equal(a[1], b[1])
        self.assertFalse((a[1] == c[1]).all())

    # checks: PRE-22
    def test_any_two_versions_join_without_a_seam(self):
        t = grass(64)
        versions, _, _ = tiles.make_versions(t, 3, 2, 4, 16, seed=1)
        for a in versions:
            self.assertLessEqual(tiles.wrap_ratio(a), 1.2)
            for b in versions:
                self.assertLessEqual(tiles.join_ratio(a, b), 1.2)

    # checks: PRE-20 PRE-22
    def test_a_quilt_replays_over_levels_drawn_for_each_source(self):
        t, other = grass(64, 3), grass(64, 4)
        versions, shift, ops = tiles.make_versions(t, 3, 4, 8, 16, seed=2, others=[other])
        again = tiles.replay([t, other], versions[0], ops[1], 4, 8, 1)
        np.testing.assert_array_equal(again, versions[1])  # at its own size it makes the same tile
        half = [np.rint(tiles.halve(x)).astype(np.uint8) for x in (t, other)]
        levels = tiles.relevel(half, shift, ops, 4, 8, None, 2)
        self.assertEqual([v.shape[0] for v in levels], [32, 32, 32])
        self.assertTrue(tiles.shares_ring(levels, 2), "the ring is shared at the next level too")
        self.assertFalse(tiles.shares_ring(levels, 8))

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
