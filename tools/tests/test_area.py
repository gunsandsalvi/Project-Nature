"""The pilot's stand-in area drawn in the cloud (PRE-22, PRE-23, PRE-26, A5.3): the Pilot page's meadow and river, one
run of tools/shots.sh over a plan, and what the pictures must show.

- the ladder's tile for each band, read from the debug picture (kd_picture 3: red byte // 32 the version, green byte //
  32 the tile): the near tile at bands 0 and 1, the middle at 2 and 3, the far at 4 to 6, each cell of a tile picking
  its version, and the cells' borders lying where the tiles' widths put them;
- the cell border steps no more than any other texture-pixel edge on the same line (the question of 7 October 2026:
  the screen's centre column is the one line of edges that stays upright on tilted ground, so its edges read whole in
  every row, which is why it is compared with edges of the same line and not with the picture's average);
- the river's marks move an even number of screen pixels a tick, whole 2-pixel texture pixels;
- the camp's tent on the meadow: the hollow under its hem and in its doorway is darker than open ground and never
  black, and the cast shadow is darker than the lit ground (A4.8).

These need Godot, Xvfb, the built extension and the game's data (tools/gamedata.py)."""

import json
import os
import shutil
import subprocess
import tempfile
import unittest

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))

READY = (
    os.path.exists(os.environ.get("GODOT", "")) and shutil.which("xvfb-run") is not None
    and os.path.exists(os.path.join(ROOT, "game", "bin", "libkindling.linux.x86_64.so"))
    and os.path.exists(os.path.join(ROOT, "game", "data", "build.toml"))
)  # fmt: skip

# The screen is 1080 by 1500, its middle (the focus) at column 540 and row 750 (tools/shots.sh).
CENTRE_COLUMN = 540
CENTRE_ROW = 750
# Metres a screen pixel at band 0, the closest zoom (a texture pixel 2 pixels wide at 64 a metre), and the meadow's
# place north of the river, in centimetres (game/pages/pilot.gd).
BAND_ZERO = 1.0 / 128.0
MEADOW_NORTH = 4000
# The tile each band reads (A5.3), and a cell's width on screen at each band: a tile's width in metres (4, 16 or 64)
# over the metres a screen pixel is.
TILE_OF_BAND = [0, 0, 1, 1, 2, 2, 2]
CELL_PIXELS = [512, 256, 512, 256, 512, 256, 128]
# Where the view is for the border question, in centimetres east: 0 is a cell border of the near tile (cells are 4 m
# from the world's centre); the others are the middle of a cell and stand on a texture pixel's edge (a texture pixel
# at band 0 is 1/64 m, 1.5625 cm: 100 cm is 64 of them).
BORDER_AT = 0
INTERIOR_AT = [100, 200, -200]
# Rows of the meadow picture that read the near tile's first level, under the screen's centre
BORDER_ROWS = (700, 1250)


def plan():
    steps = [{"call": "hold_water", "args": [0]}, {"call": "look_at_place", "args": ["Meadow"]}]
    steps.append({"call": "set_band", "args": [0]})
    for east in [BORDER_AT, *INTERIOR_AT]:
        steps.append({"call": "view_at", "args": [east, MEADOW_NORTH, 0.0, BAND_ZERO]})
        steps.append({"shot": f"east-{east}.png"})
    steps.append({"call": "look_at_place", "args": ["River"]})
    steps.append({"call": "set_band", "args": [0]})
    for tick in range(3):
        steps.append({"call": "hold_water", "args": [tick]})
        steps.append({"shot": f"tick-{tick}.png"})
    steps.append({"call": "look_at_place", "args": ["Camp"]})
    steps.append({"call": "set_band", "args": [0]})
    steps.append({"shot": "camp.png"})
    steps.append({"call": "look_at_place", "args": ["Meadow"]})
    steps.append({"call": "set_picture", "args": [3]})
    for band in range(7):
        steps.append({"call": "set_band", "args": [band]})
        steps.append({"shot": f"ladder-b{band}.png"})
    return steps


def picture(folder, name):
    return np.asarray(Image.open(os.path.join(folder, name)).convert("RGB")).astype(float)


def ladder_codes(folder, band):
    """What each pixel along the centre row reads in the ladder's debug picture: 8 for each tile after the near one,
    and the version from 0."""
    a = picture(folder, f"ladder-b{band}.png")[CENTRE_ROW].astype(int)
    return (a[:, 1] // 32) * 8 + (a[:, 0] // 32)


# The rows of the camp's picture that show the world, above the page's buttons (1500 rows, the controls from about 1240)
CAMP_ROWS = 1180


def camp_lightness(folder):
    """Each pixel of the camp's picture as the mean of its channels, over the rows that show the world."""
    return picture(folder, "camp.png").mean(axis=2)[:CAMP_ROWS]


def centre_step(folder, name):
    """The mean lightness step between the columns either side of the screen's centre line, over the rows that read
    the near tile at band 0."""
    lightness = picture(folder, name).mean(axis=2)[BORDER_ROWS[0] : BORDER_ROWS[1]]
    return float(np.abs(lightness[:, CENTRE_COLUMN] - lightness[:, CENTRE_COLUMN - 1]).mean())


def marks(folder, name):
    """The river's marks: the light pixels of the water in the middle of the picture."""
    water = picture(folder, name).mean(axis=2)[500:1000]
    return (water > np.percentile(water, 92)).astype(float)


def shift_between(earlier, later):
    """How many screen pixels east the later picture's marks are of the earlier one's, the shift at which they overlap
    most."""
    best = max(range(40), key=lambda s: float((np.roll(earlier, s, axis=1) * later).sum()))
    return best


@unittest.skipUnless(READY, "needs Godot, Xvfb, the built extension and the game's data")
class ThePilotsArea(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.folder = tempfile.mkdtemp()
        steps = os.path.join(cls.folder, "plan.json")
        with open(steps, "w") as f:
            json.dump(plan(), f)
        run = subprocess.run(
            [os.path.join(ROOT, "tools", "shots.sh"), "pilot", steps, cls.folder], capture_output=True, text=True
        )
        if run.returncode != 0:
            raise RuntimeError(run.stdout + run.stderr)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.folder)

    # checks: PRE-22 PRE-23
    def test_each_band_reads_its_tile_and_a_version_under_the_centre(self):
        for band in range(7):
            code = int(ladder_codes(self.folder, band)[CENTRE_COLUMN])
            self.assertEqual(code // 8, TILE_OF_BAND[band], f"band {band}'s tile")
            self.assertLess(code % 8, 4, f"band {band}'s version: the meadow has four of each tile")

    # checks: PRE-22 PRE-23
    def test_the_cells_borders_lie_a_tile_s_width_apart_and_their_versions_differ(self):
        seen = set()
        for band in range(7):
            codes = ladder_codes(self.folder, band)
            seen.update(int(c) for c in codes)
            for x in range(1, codes.size):
                if codes[x] != codes[x - 1]:
                    # a border: the pixel's own column is the first of the next cell, 540 + k a cell's width on
                    # screen, give or take the one pixel the edge falls between
                    k = round((x - CENTRE_COLUMN) / CELL_PIXELS[band])
                    self.assertLessEqual(
                        abs(x - (CENTRE_COLUMN + k * CELL_PIXELS[band])), 1, f"band {band}: a border at column {x}"
                    )
        # at band 0 the three cells in view do not all pick one version
        self.assertGreaterEqual(len(set(int(c) for c in ladder_codes(self.folder, 0))), 2)
        # and each tile of the ladder is read somewhere in the bands
        self.assertEqual({c // 8 for c in seen}, {0, 1, 2})

    # checks: PRE-22
    def test_the_cell_border_steps_no_more_than_the_texture_pixel_edges_on_its_own_line(self):
        border = centre_step(self.folder, f"east-{BORDER_AT}.png")
        interior = [centre_step(self.folder, f"east-{east}.png") for east in INTERIOR_AT]
        self.assertLessEqual(border, 1.1 * float(np.mean(interior)), f"border {border:.2f}, interior {interior}")

    # checks: PRE-26 PRE-22
    def test_the_marks_step_a_tick_in_whole_texture_pixels(self):
        first, second, third = (marks(self.folder, f"tick-{n}.png") for n in range(3))
        step = shift_between(first, second)
        self.assertGreater(step, 0)
        # a texture pixel is 2 screen pixels at band 0
        self.assertEqual(step % 2, 0, f"the marks moved {step} pixels")
        self.assertEqual(shift_between(second, third), step)

    # checks: PRE-21 PRE-24 PRE-30
    def test_the_hollows_under_the_tent_and_its_shadow_are_darker_than_open_ground_and_never_black(self):
        lightness = camp_lightness(self.folder)
        open_ground = float(np.median(lightness))
        # the darkest half of one per cent of the picture: the doorway's cavity and the shadow's core
        darkest = float(np.percentile(lightness, 0.5))
        self.assertLess(darkest, 0.4 * open_ground)
        # never black: even the darkest tenth of one per cent keeps colour (A4.8)
        self.assertGreaterEqual(float(np.percentile(lightness, 0.1)), 10.0)


if __name__ == "__main__":
    unittest.main()
