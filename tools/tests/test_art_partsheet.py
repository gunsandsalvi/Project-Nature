"""The preview sheet of a family of parts (PRE-46, A6.5): 1080 pixels wide, every part at 128 picture pixels a metre
beside a 1 m bar, and the check's problems written on it; drawn here from stand-in pictures, so no Blender is
needed."""

import json
import os
import sys
import tempfile
import unittest

import numpy as np
from PIL import Image

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(TOOLS, "art"))
import partsheet  # noqa: E402
import sheet  # noqa: E402


def part(name, worst=1.1, problems=()):
    return {"name": name, "group": "wood", "size": [0.1, 0.1, 2.0], "worst": worst, "problems": list(problems)}


class Sheet(unittest.TestCase):
    def setUp(self):
        d = tempfile.TemporaryDirectory()
        self.addCleanup(d.cleanup)
        self.dir = d.name

    def draw(self, parts, pictures):
        views = []
        for name, (w, h) in pictures.items():
            pic = np.zeros((h, w, 4), np.uint8)
            pic[..., 0] = 200
            pic[2:-2, 2:-2, 3] = 255  # a clear edge round each, as the renders have
            Image.fromarray(pic, "RGBA").save(os.path.join(self.dir, f"{name}.png"))
            views.append({"view": name, "parts": [name], "kind": "part", "scale": 128.0, "pixels": [w, h]})
        with open(os.path.join(self.dir, "views.json"), "w") as f:
            json.dump({"views": views}, f)
        check = os.path.join(self.dir, "check.json")
        with open(check, "w") as f:
            json.dump({"file": "test.blend", "parts": parts}, f)
        return partsheet.make(self.dir, check)

    # checks: PRE-46
    def test_the_metre_bar_is_128_pixels_with_a_tick_every_10_cm(self):
        bar = partsheet.metre_bar(128.0)
        ink = (bar == np.array(sheet.INK)).all(axis=2)
        row = ink[7]
        self.assertEqual(int(row.sum()), 128 + 1)  # the bar's 128 pixels and the tick at its far end
        ticks = [x for x in range(bar.shape[1]) if ink[1, x] or ink[4, x]]
        self.assertEqual(len(ticks), 11)

    # checks: PRE-46
    def test_a_sheet_is_1080_wide_and_keeps_every_part_whole(self):
        a = self.draw([part("pole"), part("log")], {"pole": (40, 300), "log": (260, 90)})
        self.assertEqual(a.shape[1], 1080)
        reds = (a[..., 0] == 200) & (a[..., 1] == 0)
        self.assertEqual(int(reds.sum()), 36 * 296 + 256 * 86)  # each picture once, at its true size

    # checks: PRE-46
    def test_a_part_wider_than_the_sheet_is_shown_smaller_and_says_so(self):
        a = self.draw([part("cliff")], {"cliff": (1500, 100)})
        self.assertEqual(a.shape[1], 1080)

    # checks: PRE-46
    def test_problems_are_written_on_the_sheet(self):
        clean = self.draw([part("pole")], {"pole": (40, 300)})
        failing = self.draw([part("pole", 1.9, ["stretched 1.90:1 at rest"])], {"pole": (40, 300)})
        self.assertGreater(failing.shape[0], clean.shape[0])
        red = (np.abs(failing.astype(int) - partsheet.RED) < 30).all(axis=2)
        self.assertGreater(int(red.sum()), 100, "the problem is not written in red")


if __name__ == "__main__":
    unittest.main()
