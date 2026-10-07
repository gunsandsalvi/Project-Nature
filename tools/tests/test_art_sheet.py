"""The catalogue sheet (PRE-46, PRE-22, A5.3): each picture from above with a scale stick drawn by code at its true
length, a close-up, a 3 x 3 repeat, the camera views side by side, the states in rows, and every panel labelled on a
page 1080 pixels wide."""

import json
import os
import sys
import tempfile
import unittest

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import sheet  # noqa: E402

GREEN = (90, 120, 60)


def stick_lengths(picture):
    """The longest run of the stick's two colours along each row of the picture, in pixels."""
    pixels = np.asarray(picture.convert("RGB")).astype(int)
    on = (np.abs(pixels - sheet.IVORY).sum(axis=2) == 0) | (np.abs(pixels - sheet.CHARCOAL).sum(axis=2) == 0)
    best = 0
    for row in on:
        run = 0
        for v in row:
            run = run + 1 if v else 0
            best = max(best, run)
    return best


class Sheet(unittest.TestCase):
    # checks: PRE-46
    def test_a_stick_is_true_to_the_picture_it_is_drawn_on(self):
        for width, across, metres, length in ((508, 4, 1, 127), (508, 16, 10, 318), (336, 4, 1, 84)):
            shown = sheet.with_stick(Image.new("RGB", (width, width), GREEN), across, metres)
            self.assertEqual(stick_lengths(shown), length, f"{metres} m on {across} m shown {width} wide")

    # checks: PRE-46 PRE-22
    def test_a_sheet_shows_its_tiles_its_close_up_its_views_and_its_states(self):
        with tempfile.TemporaryDirectory() as d:
            Image.new("RGB", (1254, 1254), GREEN).save(os.path.join(d, "near.png"))
            Image.new("RGB", (1024, 1536), (120, 110, 70)).save(os.path.join(d, "cam.png"))
            Image.new("RGB", (1254, 1254), (200, 190, 170)).save(os.path.join(d, "snow.png"))
            spec = {
                "number": "1.1",
                "name": "Meadow",
                "about": "A test piece.",
                "palette": [["soil", "#A77950"], ["green", "#62733E"]],
                "tiles": [{"file": "near.png", "label": "Near: 4 m", "metres": 4, "stick": 1}],
                "close_up": {"tile": 0, "metres": 1, "at": [1, 1]},
                "repeat": [0],
                "camera": [["cam.png", "From the south"], ["cam.png", "From the north"]],
                "states": {"title": "States", "metres": 4, "stick": 1, "tiles": [["snow.png", "Snow"]] * 4},
                "notes": ["a note"],
            }
            with open(os.path.join(d, "spec.json"), "w") as f:
                json.dump(spec, f)
            out = os.path.join(d, "sheet.png")
            self.assertEqual(sheet.main(["sheet.py", os.path.join(d, "spec.json"), d, out]), 0)
            page = Image.open(out)
            self.assertEqual(page.width, sheet.WIDTH)
            self.assertGreater(page.height, 2000)
            # the near tile shown half the page wide: 4 m in 508 pixels, so its 1 m stick is 127 pixels long; the
            # close-up shows 1 m in 508 pixels, so its 10 cm stick is about 51
            right = sheet.MARGIN + sheet.HALF + sheet.GAP
            for name, box, length in (
                ("the near tile's 1 m stick", (0, 0, right, page.height), 127),
                ("the close-up's 10 cm stick", (right, 0, page.width, page.height), 51),
            ):
                column = page.crop(box)
                rows = [stick_lengths(column.crop((0, y, column.width, y + 1))) for y in range(column.height)]
                self.assertIn(length, rows, f"{name} is not {length} pixels long")


if __name__ == "__main__":
    unittest.main()
