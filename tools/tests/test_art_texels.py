"""The art lane's shared helpers (PRE-22, PRE-01, A5.4): pictures saved losslessly with no metadata, so a level's
digest depends on its pixels alone; texture pixels enlarged as crisp squares; a blur that wraps round a tile; and the
sRGB curve."""

import os
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import texels  # noqa: E402

# what a saved file must never carry, since a maker's or model's name travels in them: text chunks, EXIF, XMP and a
# C2PA record
METADATA = [b"tEXt", b"iTXt", b"zTXt", b"eXIf", b"EXIF", b"XMP", b"caBX", b"jumb", b"c2pa", b"C2PA"]


def picture(h=24, w=32, seed=1):
    return np.random.default_rng(seed).integers(0, 256, (h, w, 3)).astype(np.uint8)


class Files(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.addCleanup(self.dir.cleanup)

    def path(self, name):
        return os.path.join(self.dir.name, name)

    # checks: PRE-22 PRE-01
    def test_png_keeps_every_pixel_and_no_metadata(self):
        a = picture()
        texels.save_png(a, self.path("a.png"))
        np.testing.assert_array_equal(texels.load(self.path("a.png")), a)
        with open(self.path("a.png"), "rb") as f:
            data = f.read()
        for word in METADATA:
            self.assertNotIn(word, data)

    # checks: PRE-22
    def test_a_digest_depends_on_the_pixels_alone(self):
        a = picture()
        texels.save_png(a, self.path("one.png"))
        texels.save_png(a.copy(), self.path("two.png"))
        self.assertEqual(texels.sha256(self.path("one.png")), texels.sha256(self.path("two.png")))
        b = a.copy()
        b[3, 4, 0] ^= 1
        texels.save_png(b, self.path("three.png"))
        self.assertNotEqual(texels.sha256(self.path("one.png")), texels.sha256(self.path("three.png")))

    # checks: PRE-01
    def test_webp_is_lossless_unless_a_quality_is_given(self):
        a = picture()
        texels.save_webp(a, self.path("a.webp"))
        np.testing.assert_array_equal(texels.load(self.path("a.webp")), a)
        texels.save_webp(a, self.path("b.webp"), quality=85)
        self.assertEqual(texels.load(self.path("b.webp")).shape, a.shape)
        for name in ("a.webp", "b.webp"):
            with open(self.path(name), "rb") as f:
                data = f.read()
            for word in METADATA:
                self.assertNotIn(word, data, name)

    # checks: PRE-01
    def test_alpha_is_dropped_on_loading(self):
        from PIL import Image

        rgba = np.dstack([picture(), np.full((24, 32), 7, np.uint8)])
        Image.fromarray(rgba, "RGBA").save(self.path("rgba.png"))
        self.assertEqual(texels.load(self.path("rgba.png")).shape, (24, 32, 3))


class Pixels(unittest.TestCase):
    # checks: PRE-01
    def test_enlarging_makes_crisp_squares(self):
        a = picture(2, 3)
        big = texels.enlarge(a, 4)
        self.assertEqual(big.shape, (8, 12, 3))
        for y in range(2):
            for x in range(3):
                block = big[4 * y : 4 * y + 4, 4 * x : 4 * x + 4].reshape(-1, 3)
                self.assertTrue((block == a[y, x]).all())

    # checks: PRE-22
    def test_the_blur_wraps_round_a_tile(self):
        a = picture(32, 32).astype(np.float64)
        moved = texels.blur(np.roll(a, (5, 9), axis=(0, 1)), 2.0)
        np.testing.assert_allclose(moved, np.roll(texels.blur(a, 2.0), (5, 9), axis=(0, 1)), atol=1e-9)
        self.assertAlmostEqual(texels.blur(a, 2.0).mean(), a.mean(), places=6)

    # checks: PRE-22
    def test_the_blur_of_a_picture_reflects_at_its_edges(self):
        ramp = np.tile(np.arange(32, dtype=np.float64)[None, :, None], (8, 1, 3))
        out = texels.blur(ramp, 1.5, wrap=False)
        self.assertLess(abs(out[:, 31].mean() - 31), 1.5)  # not pulled toward column 0 as a wrap would
        np.testing.assert_allclose(texels.blur(ramp, 0), ramp)

    # checks: PRE-20
    def test_the_srgb_curve_goes_both_ways(self):
        every = np.arange(256, dtype=np.uint8)
        np.testing.assert_array_equal(texels.to_srgb(texels.to_linear(every)), every)
        self.assertAlmostEqual(float(texels.luminance(np.array([255, 255, 255], np.uint8))), 1.0)
        self.assertEqual(float(texels.luminance(np.array([0, 0, 0], np.uint8))), 0.0)


if __name__ == "__main__":
    unittest.main()
