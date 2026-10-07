"""The calibration scenes drawn in the cloud (tools/calibrun.py): what each variant must draw, and, where Godot and the
software driver are here, every scene run on the Calibrate page with its counts as its file states them."""

import os
import shutil
import sys
import tempfile
import unittest

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import calibrun  # noqa: E402


def variant(**switches):
    return {"triangles": 0, "copies": 0, "passes": 2, "shadows": True, **switches}


class WhatEachVariantMustDraw(unittest.TestCase):
    # checks: PLT-04
    def test_rocks_give_their_triangles_in_each_pass_they_have(self):
        rocks = {"draws": "rocks"}
        self.assertEqual(
            calibrun.expected(rocks, variant(triangles=200)),
            {"triangles": 200_000, "shadow_triangles": 200_000, "mirror_draws": 0},
        )
        self.assertEqual(calibrun.expected(rocks, variant(triangles=200, shadows=False))["shadow_triangles"], 0)

    # checks: PLT-04
    def test_copies_give_their_draws_in_each_pass_and_the_mirror_no_shadow_pass(self):
        copies = {"draws": "copies"}
        three = calibrun.expected(copies, variant(copies=300, passes=3))
        self.assertEqual(three, {"draws": 300, "shadow_draws": 300, "mirror_draws": 300, "mirror_shadow_draws": 0})
        self.assertEqual(calibrun.expected(copies, variant(copies=300))["mirror_draws"], 0)

    # checks: PLT-04 PRE-27
    def test_reads_give_one_draw_of_their_points_and_no_shadow_pass(self):
        reads = {"draws": "reads"}
        self.assertEqual(
            calibrun.expected(reads, variant(vertices=100, reads=12, shadows=False)),
            {"draws": 1, "triangles": 100_000, "shadow_draws": 0, "shadow_triangles": 0},
        )

    # checks: PLT-04
    def test_a_count_off_its_statement_is_named(self):
        run = {
            "scenes": [{"name": "c4", "draws": "nothing", "variants": [{"name": "bare", **variant()}]}],
            "counted": [[{"draws": 2, "triangles": 0, "shadow_draws": 0, "mirror_draws": 0}]],
            "code": "",
        }
        wrong = calibrun.check(run)
        self.assertIn("c4/bare: draws 2, where its scene states 0", wrong)


class WhatThePlantsWaysMustDraw(unittest.TestCase):
    def leaves(self):
        ways = ["none", "plain", "close", "cores", "coverage"]
        return {"name": "c2", "draws": "leaves", "variants": [{"name": w, "way": w, **variant()} for w in ways]}

    def counts(self, draws, triangles, plants_draws, plants_triangles):
        return {
            "draws": draws,
            "triangles": triangles,
            "shadow_draws": 0,
            "content_draws": plants_draws,
            "content_triangles": plants_triangles,
        }

    def pictures(self, folder, differing):
        """Plain cards' picture and the other ways', the close-cut one with this many pixels changed."""
        base = np.full((100, 100, 3), 90, dtype=np.uint8)
        for way in ("plain", "close", "cores", "coverage"):
            picture = base.copy()
            if way == "close":
                picture.reshape(-1, 3)[:differing] = 200
            Image.fromarray(picture).save(os.path.join(folder, f"c2-{way}.png"))

    # checks: PLT-04 PRE-46
    def test_each_way_adds_its_plants_to_the_bare_ground_and_cuts_no_leaf(self):
        scene = self.leaves()
        drew = [self.counts(4, 8, 0, 0)] + [self.counts(8, 608, 4, 600)] * 4
        with tempfile.TemporaryDirectory() as folder:
            self.pictures(folder, 5)
            self.assertEqual(calibrun.plants_wrong(scene, drew, folder), [])
            # a way drawing a plant more than it holds, and one casting a sun shadow
            drew[2] = self.counts(9, 608, 4, 600)
            drew[3] = dict(drew[3], shadow_draws=4)
            wrong = calibrun.plants_wrong(scene, drew, folder)
            self.assertIn("c2/close: 5 draws more than the bare ground, where its plants hold 4", wrong)
            self.assertIn("c2/cores: plants in the sun's shadow pass", wrong)

    # checks: PLT-04 PRE-46
    def test_a_way_that_changes_the_picture_past_the_line_is_named(self):
        scene = self.leaves()
        drew = [self.counts(4, 8, 0, 0)] + [self.counts(8, 608, 4, 600)] * 4
        with tempfile.TemporaryDirectory() as folder:
            # 50 of 10,000 pixels is 0.5%, past the 0.1% line; alpha to coverage may change the picture
            self.pictures(folder, 50)
            wrong = calibrun.plants_wrong(scene, drew, folder)
            self.assertEqual(wrong, ["c2/close: 0.50% of its pixels differ from plain cards', where at most 0.1% may"])
            os.remove(os.path.join(folder, "c2-cores.png"))
            self.assertIn("c2/cores: no picture to hold to plain cards'", calibrun.plants_wrong(scene, drew, folder))


class WhatTheFiresWaysMustDraw(unittest.TestCase):
    def fires(self):
        variants = [
            {"name": f"fires3-{w}", "fires": 3, "way": w, **variant()} for w in ("none", "walk", "walk-half", "map")
        ]
        return {"name": "c5", "draws": "fires", "variants": variants}

    def pictures(self, folder, shadows):
        """The fires without shadows, lit at 100, and each way with its shadowed pixels: a list of flat indices."""
        for way in ("none", "walk", "walk-half", "map"):
            picture = np.full((100, 100, 3), 100, dtype=np.uint8)
            picture.reshape(-1, 3)[shadows.get(way, [])] = 20
            Image.fromarray(picture).save(os.path.join(folder, f"c5-fires3-{way}.png"))

    # checks: PLT-04 PRE-30
    def test_each_way_draws_its_shadows_and_the_maps_none_the_walk_has_not(self):
        scene = self.fires()
        drew = [{"draws": 49, "content_draws": 45}] * 4
        with tempfile.TemporaryDirectory() as folder:
            self.pictures(folder, {"walk": list(range(100)), "walk-half": list(range(80)), "map": list(range(60))})
            self.assertEqual(calibrun.fires_wrong(scene, drew, folder), [])
            # maps casting nothing; then shadows the walk does not have
            self.pictures(folder, {"walk": list(range(100)), "walk-half": list(range(80)), "map": [500, 501]})
            wrong = calibrun.fires_wrong(scene, drew, folder)
            self.assertEqual(wrong, ["c5/fires3-map: 0.02% of its picture in shadow, where its fires cast some"])
            self.pictures(
                folder, {"walk": list(range(100)), "walk-half": list(range(80)), "map": list(range(200, 400))}
            )
            wrong = calibrun.fires_wrong(scene, drew, folder)
            self.assertEqual(wrong, ["c5/fires3-map: 2.00% of its picture shadowed where the walk's is lit"])

    # checks: PLT-04 PRE-30
    def test_a_way_drawing_other_things_is_named(self):
        scene = self.fires()
        drew = [{"draws": 49, "content_draws": 45}] * 3 + [{"draws": 50, "content_draws": 45}]
        with tempfile.TemporaryDirectory() as folder:
            self.pictures(folder, {"walk": list(range(100)), "walk-half": list(range(80)), "map": list(range(60))})
            self.assertIn(
                "c5: its ways with 3 fires draw [49, 50] things, not alike", calibrun.fires_wrong(scene, drew, folder)
            )


class WhatTheFiguresWaysMustDraw(unittest.TestCase):
    def figures(self):
        variants = [
            {"name": f"{w}{n}", "figures": n, "way": w, **variant()} for w in ("godot", "palette") for n in (30, 100)
        ]
        return {"name": "c6", "draws": "figures", "variants": variants}

    def counts(self, figures, draws, ground=(4, 8), shadow_draws=None):
        triangles = figures * 1472
        return {
            "draws": ground[0] + draws,
            "triangles": ground[1] + triangles,
            "shadow_draws": draws if shadow_draws is None else shadow_draws,
            "shadow_triangles": triangles,
            "content_draws": draws,
            "content_triangles": triangles,
        }

    # checks: PLT-04 PRE-27
    def test_a_figure_on_a_skeleton_is_a_draw_and_the_palettes_one_for_all(self):
        scene = self.figures()
        drew = [self.counts(30, 30), self.counts(100, 100), self.counts(30, 1), self.counts(100, 1)]
        self.assertEqual(calibrun.figures_wrong(scene, drew), [])
        # a figure missing from the shadow pass, and the ground drawn differently
        drew[1] = self.counts(100, 100, shadow_draws=99)
        drew[2] = self.counts(30, 1, ground=(5, 10))
        wrong = calibrun.figures_wrong(scene, drew)
        self.assertIn("c6/godot100: shadow_draws 99, where its figures alone make 100", wrong)
        self.assertIn("c6: the ground under the figures draws differently from variant to variant", wrong)
        # palettes drawing a figure a draw
        drew = [self.counts(30, 30), self.counts(100, 100), self.counts(30, 30), self.counts(100, 1)]
        self.assertIn(
            "c6/palette30: its figures make 30 draws, where its way makes 1", calibrun.figures_wrong(scene, drew)
        )


class ThePalettesAgainstGodotsSkeletons(unittest.TestCase):
    def bent(self, shift=0.0, walk=0.3, blank=0):
        """A figure of 6 vertices bent both ways at two moments, the palette's moved by `shift` metres in x, the
        walk moving the first vertex by `walk`, and the palette's first `blank` vertices not drawn."""
        rest = np.array(
            [[0.0, 0.1, 0.0], [0.1, 0.9, 0.0], [0.0, 1.7, 0.0], [0.2, 1.2, 0.1], [-0.2, 1.2, 0.1], [0, 0.5, 0]]
        )
        moments = []
        for moved in (0.0, walk):
            godot = rest.copy()
            godot[0, 2] += moved
            palette = godot + [shift, 0.0, 0.0]
            palette[:blank] = -1.0
            moments.append({"godot": godot.ravel().tolist(), "palette": palette.ravel().tolist()})
        return {"vertices": 6, "moments": moments}

    # checks: PRE-27
    def test_palettes_within_a_centimetre_of_godots_skeleton_pass(self):
        wrong, apart = calibrun.bent_wrong(self.bent(shift=0.004))
        self.assertEqual(wrong, [])
        self.assertAlmostEqual(apart, 0.004)

    # checks: PRE-27
    def test_a_vertex_bent_too_far_a_vertex_not_drawn_and_a_figure_standing_still_are_named(self):
        wrong, _ = calibrun.bent_wrong(self.bent(shift=0.02))
        self.assertEqual(wrong, ["the palettes bend a vertex 2.0 cm from Godot's skeleton, where 1 cm at most may"])
        wrong, _ = calibrun.bent_wrong(self.bent(blank=2))
        self.assertIn("moment 0: 2 of the figure's 6 vertices not drawn palette's way", wrong)
        wrong, _ = calibrun.bent_wrong(self.bent(walk=0.01))
        self.assertIn("the figure bent godot's way moves 1.0 cm in half a second of its walk", wrong)
        self.assertEqual(calibrun.bent_wrong(None)[0], ["the bone palettes were not checked against Godot's skeletons"])


READY = (
    os.path.exists(os.environ.get("GODOT", "")) and shutil.which("xvfb-run") is not None
    and os.path.exists(os.path.join(calibrun.ROOT, "game", "bin", "libkindling.linux.x86_64.so"))
    and os.path.exists(calibrun.kindling())
)  # fmt: skip


@unittest.skipUnless(READY, "needs Godot, Xvfb, the built extension and the kindling tool")
class TheScenesDrawnInTheCloud(unittest.TestCase):
    # checks: PLT-04 RES-09
    def test_every_variant_draws_what_its_scene_states_and_the_code_reads(self):
        run = calibrun.draw()
        self.assertEqual(calibrun.check(run), [])
        self.assertEqual(
            [s["name"] for s in run["scenes"]], ["c1", "c2", "c3", "c3-draws", "c4", "c5", "c6", "c6-reads"]
        )


if __name__ == "__main__":
    unittest.main()
