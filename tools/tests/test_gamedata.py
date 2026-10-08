"""The current app package drops obsolete derived 3D files while preserving source catalogues and 2D resources."""

import os
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import gamedata  # noqa: E402


class CurrentPackage(unittest.TestCase):
    # checks: PLT-09 PRE-22
    def test_rebuild_removes_stale_3d_exports_but_keeps_2d_and_proofs(self):
        with tempfile.TemporaryDirectory() as folder:
            stale = ("models/camp.kdkit", "scenes/look/c1.toml")
            kept = ("textures/art/ground.kdtex", "sheets/meadow.kdsheet", "reports/greetings.json", "build.toml")
            for name in stale + kept:
                path = os.path.join(folder, name)
                os.makedirs(os.path.dirname(path), exist_ok=True)
                with open(path, "wb") as file:
                    file.write(b"existing")
            with mock.patch.object(gamedata, "OUT", folder):
                gamedata.copy_sources([])
            self.assertTrue(all(not os.path.exists(os.path.join(folder, name)) for name in stale))
            self.assertTrue(all(os.path.isfile(os.path.join(folder, name)) for name in kept))

    # checks: PLT-09 MAT-13
    def test_manifest_keeps_proof_sections_without_obsolete_3d_manifests(self):
        text = gamedata.build_toml({"clock": "same-bits"}, 1, [], [], {"saved": "same-world"}, 30801)
        self.assertIn('[proof]\nclock = "same-bits"', text)
        self.assertIn('[bench]\nsaved = "same-world"', text)
        self.assertNotIn("[models]", text)
        self.assertNotIn("[calibration]", text)


if __name__ == "__main__":
    unittest.main()
