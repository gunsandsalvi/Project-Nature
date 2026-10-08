"""The preview tool's views that Blender draws (A4.2, A5.3, A8.4), checked without Blender: the material and the ground
pictures that are laid in the scene, and where the scene puts them. Plain colours stand in for tiles and a stub stands
in for the render, so every test runs in a moment."""

import os
import sys
import tempfile
import unittest
from unittest import mock

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import preview  # noqa: E402


def plain(colour, size=32):
    """A material of one colour: a near tile of one version, its levels halving down from `size` to one pixel."""
    levels = []
    while size >= 1:
        levels.append(np.full((size, size, 3), colour, np.uint8))
        size //= 2
    return {"near": [levels]}


class Views(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.pictures = {}
        self.configs = []
        self.sets = {"a": plain(10), "b": plain(200)}

        def write(path, picture):
            self.pictures[os.path.basename(path)] = picture

        def draw(config, scratch):
            self.configs.append(config)

        for patched in (
            mock.patch.object(preview.textures, "read_set", lambda name: self.sets[name]),
            mock.patch.object(preview.tiles, "write_png", write),
            mock.patch.object(preview, "render", draw),
        ):
            patched.start()
            self.addCleanup(patched.stop)

    # checks: PRE-20 PRE-22
    def test_a_material_beside_another_is_a_chessboard_of_the_two(self):
        preview.ground_view("a", 0, "behind", self.dir, beside="b")
        picture = self.pictures["a-beside-b-ground-band0.png"]
        h, w = picture.shape[:2]
        self.assertEqual(picture[h // 4, w // 4, 0], 10)
        self.assertEqual(picture[h // 4, 3 * w // 4, 0], 200)
        self.assertEqual(picture[3 * h // 4, w // 4, 0], 200)
        self.assertEqual(picture[3 * h // 4, 3 * w // 4, 0], 10)
        self.assertTrue(self.configs[0]["out"].endswith("a-beside-b-band0-behind.png"))

    # checks: PRE-20 PRE-22
    def test_a_ground_alone_is_the_one_material(self):
        preview.ground_view("a", 0, "behind", self.dir)
        self.assertEqual(set(np.unique(self.pictures["a-ground-band0.png"])), {10})

    # checks: PRE-20 PRE-22
    def test_a_wall_stands_up_from_the_ground_and_is_taller_than_the_frame_needs(self):
        preview.wall_view("a", "b", 0, "behind", self.dir)
        wall = self.pictures["a-wall-band0.png"]
        floor = self.pictures["b-ground-band0.png"]
        self.assertEqual(set(np.unique(wall)), {10})
        self.assertEqual(set(np.unique(floor)), {200})
        config = self.configs[0]
        self.assertEqual(config["wall"]["z0"], 0.0)
        self.assertLessEqual(config["wall"]["x0"], config["ground"]["x0"] + 1e-9)
        self.assertGreaterEqual(config["wall"]["x1"], config["ground"]["x1"] - 1e-9)
        # the wall reaches higher than half the frame's height in metres at this band, so no sky shows above it
        mpp = preview.BAND0_MPP
        self.assertGreater(config["wall"]["z1"], 0.5 * preview.SIZE[1] * mpp)
        # a texture pixel of the wall is as many metres across as a texture pixel of the ground
        metres = (config["wall"]["x1"] - config["wall"]["x0"]) / wall.shape[1]
        self.assertAlmostEqual(metres, 4.0 / 32, places=6)
        self.assertAlmostEqual((config["wall"]["z1"] - config["wall"]["z0"]) / wall.shape[0], metres, places=6)
        self.assertTrue(config["out"].endswith("a-wall-band0-behind.png"))


if __name__ == "__main__":
    unittest.main()
