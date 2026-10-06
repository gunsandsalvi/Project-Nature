"""The calibration scenes drawn in the cloud (tools/calibrun.py): what each variant must draw, and, where Godot and the
software driver are here, every scene run on the Calibrate page with its counts as its file states them."""

import os
import shutil
import sys
import unittest

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

    # checks: PLT-04
    def test_a_count_off_its_statement_is_named(self):
        run = {
            "scenes": [{"name": "c4", "draws": "nothing", "variants": [{"name": "bare", **variant()}]}],
            "counted": [[{"draws": 2, "triangles": 0, "shadow_draws": 0, "mirror_draws": 0}]],
            "code": "",
        }
        wrong = calibrun.check(run)
        self.assertIn("c4/bare: draws 2, where its scene states 0", wrong)


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
        self.assertEqual([s["name"] for s in run["scenes"]], ["c1", "c3", "c3-draws", "c4"])


if __name__ == "__main__":
    unittest.main()
