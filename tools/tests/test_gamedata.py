"""The camp package retains its catalogue and proof/report records, while dropping unused art payload."""

import os
import json
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import gamedata  # noqa: E402


class CurrentPackage(unittest.TestCase):
    # checks: PLT-09 PRE-22
    def test_rebuild_removes_unused_payload_but_keeps_reports_and_current_example(self):
        with tempfile.TemporaryDirectory() as folder:
            stale = ("models/camp.kdkit", "scenes/look/c1.toml", "textures/art/ground.kdtex", "sheets/meadow.kdsheet")
            kept = ("reports/greetings.json", "examples/first-flake.kindling", "build.toml")
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
        text = gamedata.build_toml({"clock": "same-bits"}, 1, [], [], 41301)
        self.assertIn('[proof]\nclock = "same-bits"', text)
        self.assertNotIn("[bench]", text)
        self.assertNotIn("[models]", text)
        self.assertNotIn("[calibration]", text)
        self.assertNotIn("[textures]", text)
        self.assertNotIn("[sheets]", text)

    def test_steady_report_still_exports_the_current_build_archive(self):
        with tempfile.TemporaryDirectory() as folder:
            scenes = os.path.join(folder, "scenes")
            reports = os.path.join(folder, "reports")
            runs = os.path.join(folder, "runs")
            os.makedirs(scenes)
            os.makedirs(reports)
            os.makedirs(os.path.join(runs, "greetings"))
            report = {"seconds": 1, "each": [{"index": 0, "oddities": []}]}
            with open(os.path.join(scenes, "greetings.toml"), "w") as file:
                file.write("scene")
            for target in (os.path.join(runs, "greetings", "report.json"), os.path.join(reports, "greetings.json")):
                with open(target, "w") as file:
                    json.dump(report, file)
            target = os.path.join(reports, "greetings-1.kindling")
            with open(target, "wb") as file:
                file.write(b"obsolete archive format")

            def invoke(args, **_kwargs):
                if args[1] == "export":
                    with open(args[3], "wb") as file:
                        file.write(b"this build's archive")
                return mock.Mock(stdout="", stderr="", returncode=0)

            with (
                mock.patch.multiple(gamedata, SCENES=scenes, REPORTS=reports, RUNS=runs),
                mock.patch.object(gamedata, "app_version", return_value="test"),
                mock.patch.object(gamedata.subprocess, "run", side_effect=invoke),
            ):
                self.assertEqual(gamedata.reports("tool"), ["greetings.json"])
            with open(target, "rb") as file:
                self.assertEqual(file.read(), b"this build's archive")


if __name__ == "__main__":
    unittest.main()
