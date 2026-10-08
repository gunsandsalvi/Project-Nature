"""Numeric ground reduction probes (PRE-22 PRE-46); no substitute art."""

import os
import sys
import unittest
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import fixtures  # noqa: E402
import ground_pack  # noqa: E402


def bundle(rgb, ids):
    alpha = np.full(ids.shape, 255, np.uint8)
    return {
        "colour": np.dstack([rgb, alpha]),
        "material": fixtures.material_page(ids, alpha),
        "normal": fixtures.normals(alpha, True),
    }


class GroundReductions(unittest.TestCase):
    # checks: PRE-22 PRE-46
    def test_reduction_cleanup_moves_material_with_isolated_colour(self):
        rgb = np.full((16, 16, 3), (80, 60, 40), np.uint8)
        ids = np.full((16, 16), 2, np.uint8)
        for y, x in [(4, 4), (10, 10)]:
            rgb[y : y + 2, x : x + 2] = (50, 80, 50)
            ids[y : y + 2, x : x + 2] = 3
        recipe = {"id": "probe", "class": "ground", "palette": [["#503C28", 2], ["#325032", 3]]}
        for chain in ground_pack.ground_chains(recipe, [bundle(rgb, ids)] * 3):
            for page in chain[1:]:
                if len(page["colour"]) > 2:
                    self.assertFalse(fixtures.colour_flecks(page["colour"]).any())
                np.testing.assert_array_equal(page["material"][..., 0], np.where(page["colour"][..., 0] == 50, 3, 2))

    # checks: PRE-22 PRE-46
    def test_variants_keep_compatible_borders_through_all_levels(self):
        sources = []
        for chip in [(90, 70, 50), (150, 130, 110), (180, 160, 140)]:
            rgb = np.full((16, 16, 3), (120, 100, 80), np.uint8)
            rgb[1:-1, 1:-1] = chip
            sources.append(bundle(rgb, np.full((16, 16), 2, np.uint8)))
        recipe = {
            "id": "probe",
            "class": "ground",
            "palette": [["#5A4632", 2], ["#786450", 2], ["#96826E", 2], ["#B4A08C", 2]],
        }
        chains = ground_pack.ground_chains(recipe, sources)
        for level in range(1, len(chains[0])):
            pages = [chain[level]["colour"] for chain in chains]
            for a in pages:
                for b in pages:
                    np.testing.assert_array_equal(a[:, 0], b[:, -1])
                    np.testing.assert_array_equal(a[0], b[-1])


if __name__ == "__main__":
    unittest.main()
