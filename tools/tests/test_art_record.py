"""A texture's record (PRE-42, PRE-22, PRE-20, A5.3, A5.4): integers and texts only, never a float, only the plan's
keys in its order, the band its first level is drawn for, each source with its original's digest, C2PA and request,
every band after the first naming the level it was made from, and a redrawn band its own re-grid loss."""

import copy
import os
import sys
import tempfile
import tomllib
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import record  # noqa: E402

GOOD = {
    "about": 'short wild meadow grass and bare earth, "quiet"',
    "route": "picture",
    "tile_texels": 256,
    "texels_a_metre": 64,
    "first_band": 0,
    "sources": ["art/sources/meadow/meadow-01.webp"],
    "original_sha256": ["0" * 64],
    "c2pa": ["present"],
    "requests": ["art/requests/meadow-01.txt"],
    "made": "re-gridded and quilted by code",
    "regrid_loss": "3.9%",
    "truth": "art lane, 2026-10-06: wild grasses and bare earth only; nothing countable",
    "approved": "waiting",
    "band": [
        {"level": 0, "file": "art/textures/meadow/b0.png", "sha256": "a" * 64, "way": "re-gridded"},
        {
            "level": 1,
            "file": "art/textures/meadow/b1.png",
            "sha256": "b" * 64,
            "made_from": "a" * 64,
            "way": "GPT redraw of band 0",
            "regrid_loss": "4.4%",
            "calibration": "lightness +1.3%, hue -6.2 degrees, colourfulness 103%, contrast 90%",
        },
    ],
}


def changed(**changes):
    rec = copy.deepcopy(GOOD)
    rec.update(changes)
    return rec


class Records(unittest.TestCase):
    # checks: PRE-42
    def test_a_record_reads_back_as_written(self):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "record.toml")
            record.write(path, GOOD)
            self.assertEqual(record.read(path), GOOD)
        self.assertEqual(record.problems(GOOD), [])

    # checks: PRE-42
    def test_keys_come_in_the_briefs_order(self):
        shuffled = dict(reversed(list(GOOD.items())))
        text = record.dumps(shuffled)
        keys = [line.split(" = ")[0] for line in text.split("\n") if " = " in line][: len(record.TOP)]
        self.assertEqual(keys, record.TOP)
        self.assertEqual(text, record.dumps(GOOD))

    # checks: PRE-42
    def test_floats_and_unknown_keys_are_refused(self):
        for bad in (changed(tile_texels=256.0), changed(regrid_loss=3.9), changed(colour="green")):
            with self.assertRaises(ValueError):
                record.dumps(bad)
        band = copy.deepcopy(GOOD)
        band["band"][1]["loss"] = "1%"
        with self.assertRaises(ValueError):
            record.dumps(band)
        floaty = tomllib.loads(record.dumps(GOOD).replace("tile_texels = 256", "tile_texels = 256.0"))
        self.assertIn("tile_texels holds a float", record.problems(floaty))

    # checks: PRE-42
    def test_shape_problems_are_named(self):
        missing = copy.deepcopy(GOOD)
        del missing["truth"]
        self.assertIn("no truth", record.problems(missing))
        uneven = changed(c2pa=["present", "absent"])
        self.assertTrue(any("not one entry each" in p for p in record.problems(uneven)))
        self.assertTrue(any("route" in p for p in record.problems(changed(route="painted"))))
        self.assertTrue(any("whole number" in p for p in record.problems(changed(tile_texels=True))))
        self.assertIn("no [[band]]", record.problems(changed(band=[])))
        self.assertIn("first_band is 1, not one of 0, 2, 4", record.problems(changed(first_band=1)))
        self.assertIn("no first_band", record.problems({k: v for k, v in GOOD.items() if k != "first_band"}))

    # checks: PRE-22
    def test_every_band_after_band_0_names_the_level_it_came_from(self):
        rec = copy.deepcopy(GOOD)
        del rec["band"][1]["made_from"]
        self.assertIn("band 1: no made_from", record.problems(rec))
        rec = copy.deepcopy(GOOD)
        rec["band"][1]["level"] = 2
        self.assertIn("band 1: level is 2, not 1", record.problems(rec))


if __name__ == "__main__":
    unittest.main()
