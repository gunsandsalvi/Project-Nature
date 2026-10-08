"""Checks PRE-42 PRE-43 PRE-46: source families become validated runtime catalogue records."""

import hashlib
import importlib.util
import json
import tempfile
import tomllib
import unittest
from pathlib import Path

from PIL import Image

SPEC = importlib.util.spec_from_file_location("sprite_families", Path(__file__).parents[1] / "sprite_families.py")


class SpriteFamilies(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.manifest = {
            "normal_basis": "world east, south, up; signed unit XYZ encoded as round((n+1)*127.5)",
            "material_encoding": "R exact ID, G=B=0, A colour coverage",
            "approval": "pending owner runtime review",
            "entries": [],
        }
        self.family = {"family": "far", "page_size": [4, 4], "pivot": [2, 3.75], "records": {}}
        self.manifest["entries"].append(
            {"id": "birch_summer", "sheet": "art/catalogue/sheets/tree.webp", "families": [self.family]}
        )
        for channel, colour in {
            "colour": (10, 20, 30, 255),
            "normal": (128, 128, 255, 255),
            "material": (3, 0, 0, 255),
        }.items():
            rel = f"art/textures/tree/far/{channel}/record.toml"
            path = self.root / rel
            path.parent.mkdir(parents=True)
            lines = ["tile_texels = 4", "texels_a_metre = 4", 'approved = "pending owner runtime review"']
            for level, size in enumerate([4, 2, 1]):
                picture = path.parent / f"b{level}.png"
                Image.new("RGBA", (size, size), colour).save(picture)
                lines += [
                    "[[band]]",
                    f"level = {level}",
                    f"file = {json.dumps(str(picture.relative_to(self.root)))}",
                    f'sha256 = "{hashlib.sha256(picture.read_bytes()).hexdigest()}"',
                ]
            path.write_text("\n".join(lines))
            self.family["records"][channel] = rel

    def derive(self):
        module = importlib.util.module_from_spec(SPEC)
        SPEC.loader.exec_module(module)
        return module.derive(self.root, self.manifest)

    # checks: PRE-42 PRE-43 PRE-46
    def test_fixed_point_pivot_and_source_basis_reach_catalogue(self):
        made = self.derive()
        self.assertEqual(len(made), 1)
        record = tomllib.loads(next(iter(made.values())))
        self.assertEqual(record["cells"][0]["pivot_y_256"], 960)
        self.assertEqual(record["normal_basis"], "world-east-south-up")
        self.assertEqual(record["material_map"], "fixture27-v1")
        self.assertEqual(record["colour"], "art:tree/far/colour")
        self.assertEqual(record["approved"], "pending owner runtime review")
        self.assertEqual(record["season"], "summer")

    # checks: PRE-42 PRE-43 PRE-46
    def test_missing_channel_and_misaligned_page_are_rejected(self):
        normal = self.family["records"].pop("normal")
        with self.assertRaisesRegex(ValueError, "channels"):
            self.derive()
        self.family["records"]["normal"] = normal
        self.family["page_size"] = [4, 8]
        with self.assertRaisesRegex(ValueError, "page"):
            self.derive()

    # checks: PRE-42 PRE-43 PRE-46
    def test_tampered_pixels_and_incomplete_chain_are_rejected(self):
        path = self.root / self.family["records"]["normal"]
        picture = path.parent / "b1.png"
        original = picture.read_bytes()
        Image.new("RGBA", (2, 2), (255, 0, 0, 255)).save(picture)
        with self.assertRaisesRegex(ValueError, "hash"):
            self.derive()
        picture.write_bytes(original)
        path.write_text(path.read_text().split("[[band]]")[0] + "[[band]]" + path.read_text().split("[[band]]")[1])
        with self.assertRaisesRegex(ValueError, "chain"):
            self.derive()

    # checks: PRE-42 PRE-43 PRE-46
    def test_unknown_basis_and_escaping_record_are_rejected(self):
        self.manifest["normal_basis"] = "camera space"
        with self.assertRaisesRegex(ValueError, "basis"):
            self.derive()
        self.manifest["normal_basis"] = "world east, south, up; signed unit XYZ encoded as round((n+1)*127.5)"
        self.family["records"]["normal"] = "../outside/record.toml"
        with self.assertRaisesRegex(ValueError, "path"):
            self.derive()

    # checks: PRE-42 PRE-43 PRE-46
    def test_split_parts_keep_independent_references_and_pivots(self):
        self.family["parts"] = {"front": {"records": dict(self.family["records"]), "pivot": [1.5, 3.5]}}
        made = self.derive()
        self.assertEqual(len(made), 2)
        part = tomllib.loads(made["art/sprites/fixtures27/birch_summer/far/front/record.toml"])
        self.assertEqual(part["part"], "front")
        self.assertEqual(part["cells"][0]["pivot_x_256"], 384)

    # checks: PRE-42 PRE-43 PRE-46
    def test_export_hyphens_become_valid_catalogue_ids(self):
        self.manifest["entries"][0]["id"] = "birch-summer"
        made = self.derive()
        self.assertIn("art/sprites/fixtures27/birch_summer/far/whole/record.toml", made)
