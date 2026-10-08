"""Packaged fixture checks: PLT-06, PRC-11, PRE-31."""

import importlib.util
import json
import tempfile
import unittest
import zipfile
from pathlib import Path

SPEC = importlib.util.spec_from_file_location("apk_fixtures", Path(__file__).parent.parent / "apk-fixtures.py")
apk_fixtures = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(apk_fixtures)


class ApkFixtures(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.apk = self.root / "fixture.apk"
        self.manifest = [{"sheet": "tree.kdsheet", "actions": {"walk": {"64": "tree.png"}}}]
        self.files = {
            "terrain/materials.json": b'{"values": [[1, 1, 0, 0]]}',
            "fixtures/tree.kdsheet": b"raw WebP sheet bytes",
        }
        self.members = {
            "assets/fixtures/tree.png.import": b'[remap]\npath="res://.godot/imported/tree.ctex"\n',
            "assets/.godot/imported/tree.ctex": b"imported atlas bytes",
        }

    def write(self, omit=(), altered=None):
        self.files["fixtures/manifest.json"] = json.dumps(self.manifest).encode()
        members = {"assets/" + path: data for path, data in self.files.items()}
        members.update(self.members)
        members.update(altered or {})
        for path, data in self.files.items():
            target = self.root / "game" / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        with zipfile.ZipFile(self.apk, "w") as archive:
            for path, data in members.items():
                if path not in omit:
                    archive.writestr(path, data)

    # checks: PLT-06 PRC-11 PRE-31
    def test_complete_archive(self):
        self.write()
        self.assertEqual(apk_fixtures.check(self.apk, 30801, self.root), (3, 1))

    # checks: PLT-06 PRC-11 PRE-31
    def test_shared_catalogue_sheet(self):
        self.manifest[0]["sheet"] = "res://data/sheets/meadow.kdsheet"
        self.files["data/sheets/meadow.kdsheet"] = self.files.pop("fixtures/tree.kdsheet")
        self.write()
        self.assertEqual(apk_fixtures.check(self.apk, 30801, self.root), (3, 1))
        self.write(omit=["assets/data/sheets/meadow.kdsheet"])
        with self.assertRaisesRegex(ValueError, "missing assets/data/sheets/meadow.kdsheet"):
            apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_missing_sheet(self):
        self.write(omit=["assets/fixtures/tree.kdsheet"])
        with self.assertRaisesRegex(ValueError, "missing assets/fixtures/tree.kdsheet"):
            apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_missing_atlas_import(self):
        self.write(omit=["assets/fixtures/tree.png.import"])
        with self.assertRaisesRegex(ValueError, "missing assets/fixtures/tree.png.import"):
            apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_missing_remapped_atlas(self):
        self.write(omit=["assets/.godot/imported/tree.ctex"])
        with self.assertRaisesRegex(ValueError, "missing assets/.godot/imported/tree.ctex"):
            apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_optional_normals_and_materials_must_reach_archive(self):
        for channel in ["normal", "material"]:
            with self.subTest(channel=channel):
                self.manifest[0][channel + "_levels"] = {"64": channel + ".png"}
                path = f"assets/fixtures/{channel}.png.import"
                target = f"assets/.godot/imported/{channel}.ctex"
                self.members[path] = f'[remap]\npath="res://.godot/imported/{channel}.ctex"\n'.encode()
                self.members[target] = b"channel atlas"
                self.write(omit=[target])
                with self.assertRaisesRegex(ValueError, "missing " + target):
                    apk_fixtures.check(self.apk, 30801, self.root)
                self.write()
                apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_raw_metadata_must_match_source(self):
        for path in ["fixtures/manifest.json", "terrain/materials.json", "fixtures/tree.kdsheet"]:
            with self.subTest(path=path):
                self.write(altered={"assets/" + path: b"different content"})
                with self.assertRaisesRegex(ValueError, "differs from its source"):
                    apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_historical_delivery_predates_fixtures(self):
        self.assertIsNone(apk_fixtures.check(self.apk, 30301, self.root))

    # checks: PLT-06 PRC-11 PRE-31
    def test_first_fixture_delivery_predates_terrain_materials(self):
        self.write(omit=["assets/terrain/materials.json"])
        self.assertEqual(apk_fixtures.check(self.apk, 30701, self.root), (2, 1))

    # checks: PLT-06 PRC-11 PRE-31
    def test_missing_material_record(self):
        self.write(omit=["assets/terrain/materials.json"])
        with self.assertRaisesRegex(ValueError, "missing assets/terrain/materials.json"):
            apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_import_needs_a_valid_resource_remap(self):
        for content in [b"[remap]\n", b'[remap]\npath="../outside.ctex"\n']:
            with self.subTest(content=content):
                self.write(altered={"assets/fixtures/tree.png.import": content})
                with self.assertRaises(ValueError):
                    apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_all_exported_platform_remaps_need_their_assets(self):
        import_path = "assets/fixtures/tree.png.import"
        self.members[import_path] = (
            b'[remap]\npath.etc2="res://.godot/imported/tree-etc2.ctex"\n'
            b'path.astc="res://.godot/imported/tree-astc.ctex"\n'
        )
        self.members["assets/.godot/imported/tree-etc2.ctex"] = b"ETC2 texture"
        self.members["assets/.godot/imported/tree-astc.ctex"] = b"ASTC texture"
        self.write(omit=["assets/.godot/imported/tree-astc.ctex"])
        with self.assertRaisesRegex(ValueError, "missing assets/.godot/imported/tree-astc.ctex"):
            apk_fixtures.check(self.apk, 30801, self.root)
        self.write()
        apk_fixtures.check(self.apk, 30801, self.root)

    # checks: PLT-06 PRC-11 PRE-31
    def test_empty_atlas_asset_fails(self):
        self.write(altered={"assets/.godot/imported/tree.ctex": b""})
        with self.assertRaisesRegex(ValueError, "empty assets/.godot/imported/tree.ctex"):
            apk_fixtures.check(self.apk, 30801, self.root)


if __name__ == "__main__":
    unittest.main()
