"""Textures drawn to a layout (PRE-27, PRE-46, A6.3, A6.4): an atlas's triangles rasterized at 64 texture pixels a
metre, each texture pixel knowing its part, role, bone, place and normal; its pieces' edges for seams and hems; their
colours spread into the gaps; and the drawn texture saved as a material the checks read."""

import os
import sys
import tempfile
import unittest

import numpy as np

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(TOOLS, "art"))
import checks  # noqa: E402
import look  # noqa: E402
import paint  # noqa: E402
import record  # noqa: E402


def square(u0, v0, side, z, role, part=0):
    """Two triangles making a square piece of the atlas, side metres, its place at height z and facing -y."""
    tris = []
    corners = [(u0, v0), (u0 + side, v0), (u0 + side, v0 + side), (u0, v0 + side)]
    for a, b, c in ((0, 1, 2), (0, 2, 3)):
        row = [0, part, role, 0, 0, 0]
        for i in (a, b, c):
            row += list(corners[i])
        for i in (a, b, c):
            row += [corners[i][0], 0.0, z + corners[i][1] - v0]
        row += [0.0, -1.0, 0.0] * 3
        tris.append(row)
    return tris


def layout():
    """An atlas 1 x 0.5 m with a skin square of 0.25 m and a hair square of 0.125 m beside it."""
    return {
        "atlases": {"test": [1.0, 0.5]},
        "parts": ["head", "body"],
        "roles": ["skin", "hair"],
        "bones": ["head"],
        "triangles": square(0.0, 0.0, 0.25, 1.5, 0) + square(0.5, 0.0, 0.125, 1.7, 1, part=1),
    }


class Rasterizing(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.layout = paint.Layout(layout(), "test")

    # checks: PRE-46
    def test_the_canvas_is_square_and_the_pieces_lie_where_their_coordinates_say(self):
        L = self.layout
        self.assertEqual(L.side, 64)  # 1 m at 64 a metre, its larger side
        self.assertEqual(int(L.where(role="skin").sum()), 16 * 16)
        self.assertEqual(int(L.where(role="hair").sum()), 8 * 8)
        rows, cols = np.nonzero(L.where(role="skin"))
        self.assertEqual((rows.min(), rows.max(), cols.min(), cols.max()), (48, 63, 0, 15))  # v = 0 the bottom row
        self.assertEqual(int(L.where(part="body").sum()), 64)

    # checks: PRE-27
    def test_each_texture_pixel_knows_where_it_lies_at_rest(self):
        L = self.layout
        top = L.position[48, 0]
        bottom = L.position[63, 0]
        self.assertAlmostEqual(bottom[2], 1.5 + 0.5 / 64, places=4)  # the bottom row's centre, half a pixel up
        self.assertGreater(top[2], bottom[2] + 0.2)
        np.testing.assert_allclose(L.normal[50, 3], (0.0, -1.0, 0.0), atol=1e-6)

    # checks: PRE-46
    def test_edges_count_to_the_pieces_wrap_and_ends(self):
        left, right, up, down = self.layout.edges()
        self.assertEqual((left[50, 0], right[50, 15]), (1, 1))
        self.assertEqual((left[50, 5], right[50, 5]), (6, 11))
        self.assertEqual((up[48, 3], down[63, 3]), (1, 1))

    # checks: PRE-46
    def test_colours_spread_into_the_gaps_round_a_piece(self):
        img = np.zeros((64, 64, 3), np.uint8)
        img[self.layout.covered] = (200, 100, 50)
        out = paint.spread(img, self.layout.covered, rounds=2)
        self.assertEqual(tuple(out[50, 17]), (200, 100, 50))  # two pixels right of the skin piece
        self.assertEqual(tuple(out[50, 20]), (0, 0, 0))  # past the rounds asked


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class Saving(unittest.TestCase):
    # checks: PRE-42 PRE-46
    def test_a_drawn_atlas_is_a_material_the_checks_read_as_drawn(self):
        L = paint.Layout(layout(), "test")
        rng = np.random.default_rng(2)
        img = np.zeros((64, 64, 3))
        img[...] = (180, 130, 100)
        img += rng.integers(-12, 12, (64, 64, 1))
        img[L.where(role="hair")] = (60, 40, 30)
        with tempfile.TemporaryDirectory() as root:
            folder = os.path.join(root, "art", "textures", "test")
            top = {
                "about": "a test atlas",
                "route": "code",
                "sources": [],
                "original_sha256": [],
                "c2pa": [],
                "requests": [],
                "made": "drawn by code, for a test",
                "regrid_loss": "not re-gridded: drawn by code",
                "truth": "art lane: a test",
                "approved": "waiting",
            }
            made = paint.save(folder, np.clip(img, 0, 255), L, top, root)
            self.assertEqual([t.shape[0] for t in made], [64, 32, 16, 8, 4, 2, 1])
            self.assertTrue(os.path.exists(os.path.join(folder, "layout.png")))
            rec = record.read(os.path.join(folder, "record.toml"))
            self.assertEqual((rec["tile_texels"], rec["first_band"]), (64, 0))
            got = {c: (ok, w) for _, c, ok, w in checks.check_texture(folder, root)}
            self.assertIs(got["record"][0], True, got["record"][1])
            self.assertIsNone(got["seams"][0], "a drawn atlas is not tiled")
            self.assertIsNone(got["repeat"][0])

    # checks: PRE-27 PRE-46
    def test_a_figures_small_marks_fade_and_its_pieces_keep_their_colour_round_them(self):
        """An eye of one texture pixel never grows into a bolder mark at the levels below, and round the hair's
        piece the gaps take the hair's colour at every level, never the flat colour of the empty canvas."""
        L = paint.Layout(layout(), "test")
        img = np.zeros((64, 64, 3))
        img[...] = (180, 130, 100)
        img[L.where(role="hair")] = (60, 40, 30)
        img[55, 4] = (20, 15, 10)  # the eye, inside the skin's piece
        top = {k: "x" for k in ("about", "route", "made", "regrid_loss", "truth", "approved")}
        top.update({"sources": [], "original_sha256": [], "c2pa": [], "requests": []})
        with tempfile.TemporaryDirectory() as root:
            made = paint.save(os.path.join(root, "art", "textures", "t"), img, L, top, root)
        for n, t in enumerate(made):
            dark = int((t.astype(int).sum(-1) < 100).sum())
            self.assertLessEqual(dark * 4**n, 1, f"level {n}: the eye covers more of the figure than at band 0")
        below = made[2].astype(int)  # 16 square: the hair's piece at columns 8 and 9 of the bottom rows
        right = below[15, 10]
        self.assertLess(np.abs(right - (60, 40, 30)).sum(), np.abs(right - (180, 130, 100)).sum())


if __name__ == "__main__":
    unittest.main()
