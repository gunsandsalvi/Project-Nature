"""Current APK resources cannot omit camp assets or carry obsolete payload (PLT-06 PRC-11 MAT-13)."""

import hashlib
import importlib.util
import json
import struct
import tempfile
import unittest
import zipfile
from pathlib import Path

SPEC = importlib.util.spec_from_file_location("apk_camp", Path(__file__).parents[1] / "apk-camp.py")
apk_camp = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(apk_camp)


class CampPackage(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.apk = self.root / "camp.apk"
        source = self.root / "sim/src/kd/save/snapshot.hpp"
        source.parent.mkdir(parents=True)
        source.write_text("inline constexpr auto kSnapshotVersion = 2;")
        catalogue = b'name = "camp"\n'
        digest = hashlib.sha256(catalogue).hexdigest()
        self.files = {
            "data/build.toml": (
                '[build]\ncode = 41301\n[proof]\nsmoke = "same-bits"\n'
                f'[catalogue]\nfiles = ["base/source.toml {digest}"]\n'
            ).encode(),
            "data/base/source.toml": catalogue,
            "data/examples/first-flake.json": json.dumps(
                {"captured": True, "switches": [], "actor": "3458764513820540952", "result": "6917529027641083043"}
            ).encode(),
            "data/examples/first-flake.kindling": b"KINDLWLD" + struct.pack("<I", 2) + b"current archive",
        }
        self.members = {
            "assets/ui/fonts/kindling-ui-16.fnt.import": b'[remap]\npath="res://.godot/imported/camp.fontdata"\n',
            "assets/.godot/imported/camp.fontdata": b"bitmap font",
        }

    def write(self, omit=(), altered=None):
        members = {"assets/" + path: data for path, data in self.files.items()} | self.members | (altered or {})
        for path, data in self.files.items():
            target = self.root / "game" / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        with zipfile.ZipFile(self.apk, "w") as archive:
            for path, data in members.items():
                if path not in omit:
                    archive.writestr(path, data)

    # checks: PLT-06 PRC-11 MAT-13
    def test_current_camp_package(self):
        self.write()
        self.assertEqual(apk_camp.check(self.apk, 41301, self.root), 1)

    # checks: PLT-06 MAT-13
    def test_missing_or_changed_catalogue_and_example_fail(self):
        for path in self.files:
            with self.subTest(path=path):
                self.write(omit=["assets/" + path])
                with self.assertRaisesRegex(ValueError, "missing"):
                    apk_camp.check(self.apk, 41301, self.root)
                self.write(altered={"assets/" + path: b"altered"})
                with self.assertRaisesRegex(ValueError, "differs from its source"):
                    apk_camp.check(self.apk, 41301, self.root)

    # checks: PLT-06 MAT-13
    def test_wrong_build_or_checksum_fail(self):
        self.write()
        with self.assertRaisesRegex(ValueError, "another build"):
            apk_camp.check(self.apk, 41302, self.root)
        self.files["data/base/source.toml"] = b"changed source and packaged bytes"
        self.write()
        with self.assertRaisesRegex(ValueError, "checksum differs"):
            apk_camp.check(self.apk, 41301, self.root)

    # checks: PLT-06 PRE-40
    def test_font_remaps_must_stay_local_and_reach_nonempty_assets(self):
        for omit, altered in (
            (["assets/ui/fonts/kindling-ui-16.fnt.import"], {}),
            (["assets/.godot/imported/camp.fontdata"], {}),
            ([], {"assets/.godot/imported/camp.fontdata": b""}),
            ([], {"assets/ui/fonts/kindling-ui-16.fnt.import": b'[remap]\npath="../outside"\n'}),
        ):
            self.write(omit, altered)
            with self.assertRaises(ValueError):
                apk_camp.check(self.apk, 41301, self.root)

    # checks: PLT-06 PRC-11
    def test_removed_payload_cannot_return(self):
        for path in (
            "test/unused.gdc",
            "addons/test.gdc",
            "fixtures/tree.png",
            "terrain/materials.json",
            "data/textures/art/unused.kdtex",
            "data/sheets/unused.kdsheet",
            "pages/crowd.gdc",
        ):
            self.write(altered={"assets/" + path: b"unused payload"})
            with self.assertRaisesRegex(ValueError, "obsolete"):
                apk_camp.check(self.apk, 41301, self.root)

    # checks: PLT-06 PRC-11
    def test_example_must_be_current_and_unscripted_with_exact_identities(self):
        for change in ({"captured": False}, {"switches": ["force_flake"]}, {"actor": 3458764513820540952}):
            original = self.files["data/examples/first-flake.json"]
            self.files["data/examples/first-flake.json"] = json.dumps(json.loads(original) | change).encode()
            self.write()
            with self.assertRaises(ValueError):
                apk_camp.check(self.apk, 41301, self.root)
            self.files["data/examples/first-flake.json"] = original
        self.files["data/examples/first-flake.kindling"] = b"KINDLWLD" + struct.pack("<I", 1) + b"old archive"
        self.write()
        with self.assertRaisesRegex(ValueError, "not this build's format"):
            apk_camp.check(self.apk, 41301, self.root)


if __name__ == "__main__":
    unittest.main()
