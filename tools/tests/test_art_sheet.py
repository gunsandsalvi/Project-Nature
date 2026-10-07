"""The catalogue sheet (PRE-46, PRE-22, A5.3): views from above re-gridded to their true texture pixels and shown at
2 x 2 screen pixels each, a close-up enlarged until each texture pixel is a block, a 3 x 3 repeat, the camera views
side by side, rows of states, and every panel labelled on a page 1080 pixels wide."""

import json
import os
import sys
import tempfile
import unittest

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import sheet  # noqa: E402


def blocks(texels, block, seed=1):
    """A picture of `texels` x `texels` random colours, each a `block` x `block` square, and the colours."""
    rng = np.random.default_rng(seed)
    colours = rng.integers(0, 256, (texels, texels, 3), dtype=np.uint8)
    return Image.fromarray(colours.repeat(block, axis=0).repeat(block, axis=1)), colours


class Sheet(unittest.TestCase):
    # checks: PRE-22
    def test_a_tile_drawn_in_even_blocks_regrids_to_its_texture_pixels_exactly(self):
        picture, colours = blocks(16, 4)
        self.assertTrue((np.asarray(sheet.regrid(picture, 16)) == colours).all())

    # checks: PRE-22
    def test_a_stray_pixel_in_a_block_does_not_change_its_colour(self):
        picture, colours = blocks(8, 5)
        pixels = np.asarray(picture).copy()
        pixels[2, 2] = [255, 0, 255]
        self.assertTrue((np.asarray(sheet.regrid(Image.fromarray(pixels), 8)) == colours).all())

    # checks: PRE-46 PRE-22
    def test_a_sheet_shows_its_tiles_at_true_size_its_close_up_and_its_rows(self):
        with tempfile.TemporaryDirectory() as d:
            near, colours = blocks(256, 4)
            near.save(os.path.join(d, "near.png"))
            Image.new("RGB", (1024, 1536), (90, 120, 60)).save(os.path.join(d, "cam.png"))
            Image.new("RGB", (1536, 1024), (200, 190, 170)).save(os.path.join(d, "states.png"))
            spec = {
                "number": "1.1",
                "name": "Meadow",
                "about": "A test piece.",
                "palette": [["soil", "#A77950"], ["green", "#62733E"]],
                "tiles": [{"file": "near.png", "label": "Near: 4 m", "texels_a_metre": 64, "stick": 1}],
                "close_up": {"tile": 0, "texels": 32, "at": [0, 0]},
                "repeat": [0],
                "camera": [["cam.png", "From the south"], ["cam.png", "From the north"]],
                "rows": [["States", [["states.png", "spring to autumn"]]]],
                "notes": ["a note"],
            }
            with open(os.path.join(d, "spec.json"), "w") as f:
                json.dump(spec, f)
            out = os.path.join(d, "sheet.png")
            self.assertEqual(sheet.main(["sheet.py", os.path.join(d, "spec.json"), d, out]), 0)
            page = Image.open(out)
            self.assertEqual(page.width, sheet.WIDTH)
            self.assertGreater(page.height, 2000)
            # the near tile, 256 texture pixels, shown at 2 x 2 screen pixels: 512 pixels of its own colours
            pixels = np.asarray(page.convert("RGB"))
            row = [tuple(c) for c in colours[100]]
            found = any(
                tuple(pixels[y, sheet.MARGIN + 2 * 40]) == row[40]
                and tuple(pixels[y, sheet.MARGIN + 2 * 41]) == row[41]
                for y in range(page.height)
            )
            self.assertTrue(found, "the near tile is not shown at 2 x 2 screen pixels a texture pixel")


if __name__ == "__main__":
    unittest.main()
