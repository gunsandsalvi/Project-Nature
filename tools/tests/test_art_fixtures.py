"""The 2D fixture preparation rejects corrupt maps and keeps categorical reductions (PRE-20, PRE-22, PRE-46).

These are synthetic numeric probes, not code-drawn substitute art. Art approval remains a separate review.
"""

import os
import sys
import unittest
from unittest import mock

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import fixtures  # noqa: E402


def probe():
    alpha = np.full((4, 4), 255, np.uint8)
    colour = np.full((4, 4, 4), (80, 96, 48, 255), np.uint8)
    ids = np.full((4, 4), 3, np.uint8)
    return {"colour": colour, "material": fixtures.material_page(ids, alpha), "normal": fixtures.normals(alpha, True)}


class FixtureMaps(unittest.TestCase):
    # checks: PRE-22 PRE-46
    def test_exact_proposal_fit_preserves_occupied_rows_after_tiny_reduction(self):
        source = Image.new("RGB", (20, 20), tuple(fixtures.sheet.KEY))
        pixels = np.asarray(source).copy()
        pixels[0, 0] = [140, 100, 70]
        pixels[3:20, 5:15] = [140, 100, 70]
        fitted = fixtures.fit_design_cell(
            Image.fromarray(pixels), {"logical_height": 5, "logical_width": 5, "exact_pixel_size": True}
        )
        occupied = fixtures.sheet.cut_out(fitted, least=1)
        self.assertEqual(occupied.size, (5, 5))

    # checks: PRE-22 PRE-46
    def test_declared_piece_cleanup_keeps_attached_thin_tines_and_two_antlers(self):
        rgba = np.zeros((20, 24, 4), np.uint8)
        rgba[5:15, 4:8] = [180, 140, 90, 255]
        rgba[2:8, 6] = [180, 140, 90, 255]
        rgba[5:15, 15:19] = [180, 140, 90, 255]
        rgba[17:19, 21:23] = [120, 100, 70, 255]
        clean = fixtures.keep_drawn_components(rgba, 2)
        self.assertTrue(np.all(clean[2:8, 6, 3] == 255))
        self.assertTrue(np.all(clean[5:15, 15:19, 3] == 255))
        self.assertFalse(clean[17:19, 21:23, 3].any())

    # checks: PRE-46
    def test_selected_fixture_originals_have_checked_provenance_and_requests(self):
        registered = {p["file"]: p for p in fixtures.read_json("art/sources/fixtures27/provenance.json")}
        for recipe in fixtures.read_json(fixtures.RECIPE)["fixtures"]:
            sources = set(recipe.get("sources", {}).values()) | set(recipe.get("shared_wood_sources", {}).values())
            for source in sources:
                self.assertIn(source, registered)
                self.assertEqual(fixtures.tiles.sha256(fixtures.ROOT / source), registered[source]["sha256"])
                self.assertTrue((fixtures.ROOT / registered[source]["request"]).is_file())

    # checks: PRE-20 PRE-22
    def test_shelter_reduction_preserves_half_covered_doorway(self):
        rgba = np.full((8, 8, 4), (120, 100, 70, 255), np.uint8)
        rgba[2:8, 3:5] = 0
        recipe = {"id": "tent", "palette": [["#786446", 7]]}
        reduced = fixtures.resize_drawn(rgba, (4, 4), recipe)
        self.assertTrue(np.all(reduced[1:, 1:3, 3] == 0))

    # checks: PRE-46
    def test_proposal_length_fit_keeps_declared_two_axis_size(self):
        source = Image.new("RGB", (20, 20), (255, 0, 255))
        source.paste((120, 100, 70), (5, 2, 15, 18))
        result = fixtures.fit_design_cell(source, {"logical_height": 97, "logical_width": 134})
        cropped = fixtures.sheet.cut_out(result, least=1)
        self.assertEqual(cropped.size, (134, 97))

    # checks: PRE-22 PRE-46
    def test_deer_profile_fit_honours_withers_and_tips_independently(self):
        source = Image.new("RGB", (10, 20), (120, 100, 70))
        source.paste((180, 140, 90), (0, 0, 10, 10))
        result = fixtures.fit_design_cell(
            source,
            {
                "logical_height": 97,
                "logical_width": 134,
                "vertical_landmarks": [[0.5, 1 - 1.2 / 1.9]],
            },
        )
        cropped = np.asarray(fixtures.sheet.cut_out(result, least=1))
        self.assertEqual(tuple(cropped[35, 67, :3]), (180, 140, 90))
        self.assertEqual(tuple(cropped[36, 67, :3]), (120, 100, 70))

    # checks: PRE-22 PRE-46
    def test_overhead_deer_body_and_rack_width_fit_independently(self):
        source = Image.new("RGB", (20, 10), (180, 140, 90))
        source.paste((120, 100, 70), (5, 0, 15, 10))
        result = fixtures.fit_design_cell(
            source,
            {
                "logical_height": 134,
                "logical_width": 58,
                "horizontal_landmarks": [[0.25, 11 / 58], [0.75, 46 / 58]],
            },
        )
        cropped = np.asarray(fixtures.sheet.cut_out(result, least=1))
        self.assertEqual(np.count_nonzero(np.all(cropped[50, :, :3] == [120, 100, 70], axis=1)), 35)

    # checks: PRE-20 PRE-46
    def test_tent_sewing_does_not_cut_plain_upper_hide(self):
        recipe = next(r for r in fixtures.read_json(fixtures.RECIPE)["fixtures"] if r["id"] == "tent")
        bundle, _ = fixtures.source_bundle(recipe, recipe["sources"]["near"], "near", save_cleaned=False)
        for x, y in [(249, 330), (240, 350), (215, 369), (273, 344)]:
            self.assertEqual(bundle["material"][y, x, 0], 7)

    # checks: PRE-20 PRE-22
    def test_fitted_tent_flap_normals_stay_on_the_drawn_flap(self):
        recipe = next(r for r in fixtures.read_json(fixtures.RECIPE)["fixtures"] if r["id"] == "tent")
        bundle, _ = fixtures.source_bundle(recipe, recipe["sources"]["near"], "near", save_cleaned=False)
        normal = bundle["normal"][:, :, :3].astype(float) / 127.5 - 1
        for x, y in [(257, 425), (256, 428), (255, 430)]:
            self.assertGreater(normal[y, x, 2], 0.45)

    # checks: PRE-20 PRE-22
    def test_birch_last_mip_remains_covered(self):
        b = probe()
        recipe = {"class": "sprite", "birch_repair": True, "palette": [["#506030", 3]]}
        last = fixtures.complete_chain(recipe, b)[-1]
        self.assertEqual(last["colour"].shape, (1, 1, 4))
        self.assertEqual(last["colour"][0, 0, 3], 255)
        self.assertEqual(last["material"][0, 0, 3], 255)
        self.assertEqual(last["normal"][0, 0, 3], 255)

    # checks: PRE-20 PRE-46
    def test_cached_wood_preparation_does_not_save_a_winter_output(self):
        recipe = next(r.copy() for r in fixtures.read_json(fixtures.RECIPE)["fixtures"] if r["id"] == "birch-winter")
        recipe["birch_repair"] = False
        with mock.patch.object(Image.Image, "save") as save:
            fixtures.source_bundle(recipe, recipe["sources"]["far"], "far", save_cleaned=False)
        save.assert_not_called()

    # checks: PRE-20 PRE-46
    def test_raw_neutral_bark_is_not_lost_into_a_similar_dark_leaf_chip(self):
        rgba = np.zeros((4, 4, 4), np.uint8)
        rgba[:2] = [76, 83, 55, 255]
        rgba[2:] = [86, 102, 56, 255]
        recipe = {"birch_repair": True, "palette": [["#34482F", 3], ["#566638", 3], ["#383D35", 4]]}
        colour = fixtures.resize_drawn(rgba, [4, 4], recipe)
        _, ids = fixtures.quantise(colour, recipe)
        self.assertEqual(ids[1, 1], 4)
        self.assertTrue(np.all(ids[2:] == 3))

    # checks: PRE-22 PRE-46
    def test_birch_registered_axis_has_no_rectangular_upper_splice(self):
        left, widths, _ = fixtures.birch_stem_profile([512, 1023], 64, 1024)
        right = left + widths - 1
        self.assertLessEqual(np.max(np.abs(np.diff(left[150:570]))), 1)
        self.assertLessEqual(np.max(np.abs(np.diff(right[150:570]))), 1)

    # checks: PRE-20 PRE-22
    def test_quiet_reduction_keeps_semantic_material_with_overlapping_ramps(self):
        b = probe()
        b["colour"][:] = [80, 96, 48, 255]
        b["material"][:, :, 0] = 8
        b = {kind: np.tile(page, (2, 2, 1)) for kind, page in b.items()}
        recipe = {"class": "sprite", "quiet_colour_flecks": True, "palette": [["#506030", 3], ["#506030", 8]]}
        for level in fixtures.complete_chain(recipe, b):
            self.assertEqual(set(np.unique(level["material"][:, :, 0])), {8})

    # checks: PRE-20 PRE-22 PRE-46
    def test_birch_bare_shaft_keeps_real_width_and_one_seasonal_bark(self):
        for family, band, ppm in (("near", 0, 64), ("middle", 2, 16), ("far", 4, 4)):
            colours = [
                np.asarray(
                    Image.open(fixtures.ROOT / f"art/textures/fixtures27/birch_{season}/{family}/colour/b{band}.png")
                )
                for season in ("summer", "winter")
            ]
            material = np.asarray(
                Image.open(fixtures.ROOT / f"art/textures/fixtures27/birch_summer/{family}/material/b{band}.png")
            )
            n = colours[0].shape[0]
            row = round(n - 2 * fixtures.COS37 * ppm)
            expected = max(1, round(0.25 * ppm))
            for colour in colours:
                self.assertEqual(np.count_nonzero(colour[row, :, 3]), expected)
                bottom = np.nonzero(colour[:, :, 3])[0].max()
                self.assertEqual(np.count_nonzero(colour[bottom, :, 3]), max(2, round(0.38 * ppm)))
            start = round(n - 6 * fixtures.COS37 * ppm)
            lo, hi = round(n / 2 - 0.4 * ppm), round(n / 2 + 0.4 * ppm)
            np.testing.assert_array_equal(colours[0][start:, lo:hi], colours[1][start:, lo:hi])
            self.assertFalse(np.any(material[start:, lo:hi, 0] == 3))
            # An exposed upper interval must match the lower shaft, not retain the old wide stem.
            upper = {"near": 500, "middle": 128, "far": 31}[family]
            shaft_left = round(n / 2 - expected / 2)
            for colour in colours:
                self.assertTrue(np.all(colour[upper, shaft_left : shaft_left + expected, 3] == 255))
                if ppm >= 16:
                    occupied = colour[upper, :, 3] > 0
                    left = right = n // 2 - 1
                    while left and occupied[left - 1]:
                        left -= 1
                    while right + 1 < n and occupied[right + 1]:
                        right += 1
                    self.assertEqual(right - left + 1, expected)

    # checks: PRE-20
    def test_birch_volume_field_has_no_proxy_seam_or_remote_branch_tilt(self):
        colour = np.zeros((256, 256, 4), np.uint8)
        ids = np.zeros((256, 256), np.uint8)
        colour[20:190, 60:195] = [118, 131, 68, 255]
        ids[20:190, 60:195] = 3
        colour[40:250, 126:130] = [200, 196, 178, 255]
        ids[40:250, 126:130] = 4
        normal = fixtures.birch_normals(colour, ids, [128, 255.75], 16)
        before = normal[100, 126:130].copy()
        colour[100, 85:120] = [200, 196, 178, 255]
        ids[100, 85:120] = 4
        after = fixtures.birch_normals(colour, ids, [128, 255.75], 16)
        np.testing.assert_array_equal(before, after[100, 126:130])
        vectors = normal[:, :, :3].astype(float) / 127.5 - 1
        leaf_pair = (ids[:-1] == 3) & (ids[1:] == 3)
        dot = np.sum(vectors[:-1] * vectors[1:], axis=2)
        lengths = np.linalg.norm(vectors[:-1], axis=2) * np.linalg.norm(vectors[1:], axis=2)
        angles = np.degrees(np.arccos(np.clip(dot[leaf_pair] / lengths[leaf_pair], -1, 1)))
        self.assertLess(angles.max(), 5)

    # checks: PRE-20 PRE-22
    def test_ground_colour_fleck_cleanup_keeps_an_intentional_cluster(self):
        colour = np.full((16, 16, 4), (80, 96, 48, 255), np.uint8)
        colour[2:5, 2:5, :3] = [100, 110, 65]
        colour[9, 9, :3] = [100, 110, 65]
        clean, count = fixtures.merge_colour_flecks(colour)
        self.assertEqual(count, 1)
        np.testing.assert_array_equal(clean[2:5, 2:5], colour[2:5, 2:5])
        self.assertFalse(fixtures.colour_flecks(clean).any())

    # checks: PRE-02 PRE-22
    def test_ground_repeat_projects_the_full_span_once_without_rounded_tile_pitch(self):
        plane = np.zeros((768, 768, 4), np.uint8)
        projected = fixtures.project_ground(plane)
        self.assertEqual(projected.shape[:2], (463, 768))
        self.assertNotEqual(projected.shape[0], round(256 * fixtures.SIN37) * 3)
        shown = fixtures.metre_gauge(8)
        self.assertEqual(shown.width, 8)

    # checks: PRE-20
    def test_dome_normals_turn_top_front_and_flanks_under_opposite_lights(self):
        alpha = np.full((32, 32), 255, np.uint8)
        n = fixtures.normals(alpha, shape="dome")[:, :, :3].astype(float) / 127.5 - 1
        np.testing.assert_allclose(np.linalg.norm(n, axis=2), 1, atol=0.015)
        self.assertGreater(n[0, 16, 2], 0.95)
        self.assertGreater(n[-1, 16, 1], 0.90)
        self.assertLess(n[16, 0, 0], -0.45)
        self.assertGreater(n[16, -1, 0], 0.45)

    # checks: PRE-20 PRE-22
    def test_sprite_colour_cleanup_preserves_alpha_at_edges(self):
        rgba = np.zeros((16, 16, 4), np.uint8)
        rgba[4:12, 4:12] = [100, 110, 65, 255]
        rgba[4, 4] = [90, 96, 60, 255]
        cleaned, _ = fixtures.merge_colour_flecks(rgba)
        np.testing.assert_array_equal(cleaned[:, :, 3], rgba[:, :, 3])

    # checks: PRE-20 PRE-46
    def test_corrupt_alpha_ids_palette_and_normal_each_fail(self):
        def failures(b):
            return fixtures.check_bundle(b, {(80, 96, 48)}, {3})

        self.assertEqual(failures(probe()), [])
        bad = probe()
        bad["colour"][1, 1, 3] = 128
        self.assertIn("solid colour alpha is not binary", failures(bad))
        bad = probe()
        bad["material"][1, 1, 3] = 0
        self.assertIn("colour, material and normal alpha differ", failures(bad))
        bad = probe()
        bad["material"][1, 1, 0] = 5
        self.assertIn("material page has an undeclared or blended ID", failures(bad))
        bad = probe()
        bad["colour"][1, 1, :3] = [40, 30, 20]
        self.assertIn("colour is outside declared palette", failures(bad))
        bad = probe()
        bad["normal"][1, 1, :3] = [128, 128, 128]
        self.assertIn("normal is not a unit vector", failures(bad))

    # checks: PRE-20 PRE-22
    def test_reduction_never_averages_material_ids(self):
        b = probe()
        b["material"][::2, :, 0] = 8
        small = fixtures.halve_bundle(b)
        self.assertTrue(set(np.unique(small["material"][:, :, 0])) <= {3, 8})
        self.assertNotIn(5, small["material"][:, :, 0])
        self.assertNotIn(6, small["material"][:, :, 0])

    # checks: PRE-20 PRE-22
    def test_palette_cleanup_cannot_change_a_material_with_the_same_colour(self):
        b = probe()
        recipe = {"palette": [["#506030", 8], ["#506030", 3]]}
        colour, ids = fixtures.quantise(b["colour"], recipe, b["material"][:, :, 0])
        np.testing.assert_array_equal(ids, b["material"][:, :, 0])
        np.testing.assert_array_equal(colour, b["colour"])

    # checks: PRE-20 PRE-22
    def test_reduced_normals_are_renormalised_not_averaged_rgb(self):
        b = probe()
        b["normal"][::2, :, :3] = [204, 128, 230]
        b["normal"][1::2, :, :3] = [51, 128, 230]
        small = fixtures.halve_bundle(b)
        vec = small["normal"][:, :, :3].astype(float) / 127.5 - 1
        np.testing.assert_allclose(np.linalg.norm(vec, axis=2), 1, atol=0.015)
        self.assertGreater(int(small["normal"][0, 0, 2]), 250)

    # checks: PRE-20 PRE-22
    def test_transparent_black_does_not_darken_a_thin_feature(self):
        b = probe()
        b["colour"][:] = 0
        b["material"][:] = 0
        b["colour"][0, 0] = [80, 96, 48, 255]
        b["material"][0, 0] = [3, 0, 0, 255]
        b["normal"][:, :, 3] = b["colour"][:, :, 3]
        small = fixtures.halve_bundle(b)
        np.testing.assert_array_equal(small["colour"][0, 0], [80, 96, 48, 255])
        np.testing.assert_array_equal(small["material"][0, 0], [3, 0, 0, 255])
        self.assertEqual(np.count_nonzero(small["colour"][:, :, 3]), 1)

    # checks: PRE-20
    def test_single_specks_are_removed_but_connected_tips_survive(self):
        b = probe()["colour"]
        b[:] = 0
        b[0, 0] = [80, 96, 48, 255]
        b[2, 2] = b[3, 3] = [80, 96, 48, 255]
        clean, removed = fixtures.remove_specks(b)
        self.assertEqual(removed, 1)
        self.assertEqual(clean[0, 0, 3], 0)
        self.assertEqual(clean[2, 2, 3], 255)
        self.assertEqual(clean[3, 3, 3], 255)

    # checks: PRE-20
    def test_ground_faces_world_up_not_the_camera(self):
        vectors = fixtures.normals(np.full((4, 4), 255, np.uint8), True)
        np.testing.assert_array_equal(vectors[0, 0], [128, 128, 255, 255])
        actor = fixtures.normals(np.full((4, 4), 255, np.uint8), False)
        self.assertGreater(int(actor[0, 0, 1]), 225)
        self.assertLess(int(actor[0, 0, 2]), 210)

    # checks: PRE-22 PRE-46
    def test_scale_ignores_padding_and_rejects_wrong_metres(self):
        recipe = {"class": "sprite", "measure": "vertical", "metres": 1.7}
        padded = np.zeros((128, 128, 4), np.uint8)
        padded[20:107, 50:70] = [80, 96, 48, 255]
        self.assertEqual(fixtures.check_scale(padded, recipe, 64), [])
        self.assertTrue(fixtures.check_scale(padded, {**recipe, "metres": 1.0}, 64))
        self.assertTrue(fixtures.check_scale(np.zeros_like(padded), recipe, 64))

    # checks: PRE-46
    def test_designs_cannot_claim_runtime_or_engine_approval(self):
        recipe = fixtures.read_json(fixtures.RECIPE)
        exports = fixtures.read_json(fixtures.MANIFEST)
        self.assertEqual(exports["approval"], "no runtime or engine approval")
        self.assertEqual({p["id"] for p in recipe["pending_designs"]}, {"body", "red_deer"})
        for p in recipe["pending_designs"]:
            self.assertIn("owner", p["status"])
            self.assertFalse((fixtures.ROOT / "art/textures/fixtures27" / p["id"]).exists())
        for entry in exports["entries"]:
            for f in entry["families"]:
                self.assertTrue(f["review"].startswith("pending"))

    # checks: PRE-20 PRE-22
    def test_area_cleanup_keeps_a_connected_thin_branch_and_real_materials(self):
        raw = np.zeros((32, 32, 4), np.uint8)
        raw[2:30, 15:17] = [80, 96, 48, 255]
        raw[7:9, 3:16] = [160, 150, 130, 255]
        recipe = {"palette": [["#506030", 3], ["#A09682", 4]]}
        small = fixtures.resize_drawn(raw, (8, 8), recipe)
        self.assertFalse(fixtures.isolated(small[:, :, 3] > 0).any())
        self.assertGreater(np.count_nonzero(small[1:3, :4, 3]), 2)
        self.assertEqual(set(np.unique(small[:, :, 3])), {0, 255})
        self.assertTrue({tuple(p) for p in small[small[:, :, 3] > 0, :3]} <= {(80, 96, 48), (160, 150, 130)})

    # checks: PRE-20
    def test_shape_normals_change_direction_but_stay_mild_and_unit_length(self):
        alpha = np.full((16, 16), 255, np.uint8)
        n = fixtures.normals(alpha, shape="rounded")[:, :, :3].astype(float) / 127.5 - 1
        np.testing.assert_allclose(np.linalg.norm(n, axis=2), 1, atol=0.015)
        self.assertLess(n[8, 0, 0], 0)
        self.assertGreater(n[8, -1, 0], 0)
        self.assertGreater(n[0, 8, 2], n[-1, 8, 2])
        self.assertLess(np.abs(n[:, :, 0]).max(), 0.25)

    # checks: PRE-22 PRE-46
    def test_cutouts_reconstruct_colour_and_maps_without_double_coverage(self):
        b = probe()
        for name in ("birch-summer", "tent"):
            b["material"][:, :, 0] = 6 if name == "tent" else 4
            pieces = fixtures.split_parts({"id": name}, b, [2, 4], 0.25)
            self.assertEqual(len(pieces), 2)
            coverage = sum(p["colour"][:, :, 3].astype(int) for p in pieces.values())
            np.testing.assert_array_equal(coverage, b["colour"][:, :, 3])
            for kind in fixtures.KINDS:
                combined = sum(p[kind].astype(int) for p in pieces.values())
                np.testing.assert_array_equal(combined, b[kind])

    # checks: PRE-22 PRE-46
    def test_top_to_front_contact_includes_ground_depth_and_vertical_height(self):
        r = {"class": "sprite", "measure": "across", "metres": 3, "height_m": 2, "anchor_ground_offset_m": [0, 1.1]}
        c = np.zeros((16, 16, 4), np.uint8)
        c[3:12, 2:14] = [80, 96, 48, 255]
        self.assertEqual(fixtures.check_scale(c, r, 4), [])
        c[:3, 2:14] = [80, 96, 48, 255]
        self.assertTrue(fixtures.check_scale(c, r, 4))


if __name__ == "__main__":
    unittest.main()
