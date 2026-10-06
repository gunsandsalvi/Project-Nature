"""The art lane's checks (PRE-20, PRE-22, PRE-42, A5.4; IMPLEMENTATION.md α2.3a): each catches its planted fault,
a missing record, a stale level, painted light, a seam, a repeat and an averaged level whose accents fall to about
77% of band 0's, and also a wrong digest, too much re-grid loss, contrast far from the source's and drift between
bands. The texture they start from is the meadow's committed levels, which pass, under a record of their own."""

import contextlib
import io
import os
import re
import shutil
import sys
import tempfile
import unittest

import numpy as np

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(TOOLS)
sys.path.insert(0, os.path.join(TOOLS, "art"))
import checks  # noqa: E402
import look  # noqa: E402
import record  # noqa: E402
import texels  # noqa: E402

MEADOW = os.path.join(ROOT, "art", "textures", "meadow")
NAME = "ground"
SAME = object()  # the meadow's own source on band 0's grid


def averaged(t, k):
    """The level k times smaller made by averaging, as mipmaps are: what the brief forbids."""
    h, w = t.shape[0] // k, t.shape[1] // k
    return np.round(t[: h * k, : w * k].astype(float).reshape(h, k, w, k, 3).mean(axis=(1, 3))).astype(np.uint8)


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class PlantedFaults(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.levels = [texels.load(os.path.join(MEADOW, f"b{n}.png")) for n in range(9)]
        cls.source = texels.load(os.path.join(MEADOW, "source.png"))

    def setUp(self):
        d = tempfile.TemporaryDirectory()
        self.addCleanup(d.cleanup)
        self.root = d.name
        self.folder = os.path.join(self.root, "art", "textures", NAME)

    def make(self, levels=None, source=SAME, loss="4.0%"):
        """A texture folder in the scratch root, its record written as the brief sets it out; with source None, no
        source.png is kept."""
        levels = self.levels if levels is None else levels
        os.makedirs(self.folder, exist_ok=True)
        for rel in (f"art/sources/{NAME}/{NAME}-01.webp", f"art/requests/{NAME}-01.txt"):
            os.makedirs(os.path.dirname(os.path.join(self.root, rel)), exist_ok=True)
            with open(os.path.join(self.root, rel), "w") as f:
                f.write("a stand-in\n")
        bands, above = [], None
        for n, t in enumerate(levels):
            rel = f"art/textures/{NAME}/b{n}.png"
            texels.save_png(t, os.path.join(self.root, rel))
            b = {"level": n, "file": rel, "sha256": texels.sha256(os.path.join(self.root, rel)), "way": "test"}
            if n:
                b["made_from"] = above
                b["calibration"] = "lightness +0.0%, hue +0.0 degrees, colourfulness 100%, contrast 100%"
            bands.append(b)
            above = b["sha256"]
        if source is not None:
            texels.save_png(self.source if source is SAME else source, os.path.join(self.folder, "source.png"))
        rec = {
            "about": "a test ground",
            "route": "picture",
            "tile_texels": 256,
            "texels_a_metre": 64,
            "sources": [f"art/sources/{NAME}/{NAME}-01.webp"],
            "original_sha256": ["0" * 64],
            "c2pa": ["present"],
            "requests": [f"art/requests/{NAME}-01.txt"],
            "made": "the meadow's levels, for a test",
            "regrid_loss": loss,
            "truth": "art lane: a test",
            "approved": "waiting",
            "band": bands,
        }
        record.write(os.path.join(self.folder, "record.toml"), rec)

    def results(self):
        return {check: (ok, what) for check, ok, what in checks.check_texture(self.folder, self.root)}

    def failed(self, check):
        ok, what = self.results()[check]
        self.assertIs(ok, False, f"{check} should fail: {what}")
        return what

    # checks: PRE-20 PRE-22 PRE-42
    def test_a_clean_texture_passes_every_check(self):
        self.make()
        got = self.results()
        expected = ["record", "stale", "loss", "seams", "light", "repeat", "contrast", "accents", "drift"]
        self.assertEqual(list(got), expected)
        for check, (ok, what) in got.items():
            self.assertIs(ok, True, f"{check}: {what}")

    # checks: PRE-42
    def test_a_missing_record(self):
        self.make()
        os.remove(os.path.join(self.folder, "record.toml"))
        self.assertEqual(checks.check_texture(self.folder, self.root), [("record", False, "no record.toml")])

    # checks: PRE-42
    def test_a_level_whose_digest_is_not_its_files(self):
        self.make()
        changed = self.levels[5].copy()
        changed[0, 0] = 255 - changed[0, 0]
        texels.save_png(changed, os.path.join(self.folder, "b5.png"))
        self.assertIn("band 5: its digest is not its file's", self.failed("record"))

    # checks: PRE-22
    def test_a_stale_level(self):
        self.make()
        changed = self.levels[3].copy()
        changed[0, 0] = 255 - changed[0, 0]
        texels.save_png(changed, os.path.join(self.folder, "b3.png"))
        rec = record.read(os.path.join(self.folder, "record.toml"))
        rec["band"][3]["sha256"] = texels.sha256(os.path.join(self.folder, "b3.png"))  # band 3 redone, 4 not
        record.write(os.path.join(self.folder, "record.toml"), rec)
        self.assertIs(self.results()["record"][0], True)
        self.assertEqual(self.failed("stale"), "bands 4 made from an older level")

    # checks: PRE-20
    def test_painted_light(self):
        lit = texels.to_srgb(texels.to_linear(self.levels[0]) * np.linspace(0.7, 1.3, 256)[None, :, None])
        self.make([lit] + self.levels[1:])
        self.assertIn("slope", self.failed("light"))

    # checks: PRE-22
    def test_a_seam(self):
        """Band 0 cut straight from its source, never made seamless."""
        cut = texels.load(os.path.join(ROOT, "art", "sources", "meadow", "meadow-00.webp"))[300:556, 300:556]
        self.make([cut] + self.levels[1:])
        self.failed("seams")

    # checks: PRE-22
    def test_a_repeat(self):
        """Band 0 made by repeating a smaller tile: seamless, but the same 2 m four times over."""
        self.make([np.tile(self.levels[1], (2, 2, 1))] + self.levels[1:])
        self.assertIs(self.results()["seams"][0], True)
        self.failed("repeat")

    # checks: PRE-20 PRE-22
    def test_an_averaged_level_loses_its_accents(self):
        """Band 2 made as a mipmap is, by averaging band 0: research 19 measured 77% of band 0's accents."""
        self.make(self.levels[:2] + [averaged(self.levels[0], 4)] + self.levels[3:])
        what = self.failed("accents")
        share = int(re.search(r"\b2 \((\d+)%\)", what).group(1))
        self.assertTrue(70 <= share <= 85, what)

    # checks: PRE-22
    def test_too_much_regrid_loss(self):
        self.make(loss="12.0%")
        self.assertEqual(self.failed("loss"), "12.0% (at most 10%)")

    # checks: PRE-20
    def test_contrast_far_from_the_sources(self):
        self.make(source=look.adjust(self.source, contrast=170.0))
        self.failed("contrast")
        shutil.rmtree(self.folder)
        self.make(source=None)
        self.assertIsNone(self.results()["contrast"][0], "not measured without a source")

    # checks: PRE-20
    def test_drift_between_bands(self):
        self.make(self.levels[:5] + [look.adjust(self.levels[5], lightness=4.0)] + self.levels[6:])
        self.assertIn("5 (lightness", self.failed("drift"))

    # checks: PRE-42
    def test_the_command_fails_when_a_check_fails(self):
        self.make(loss="12.0%")
        with contextlib.redirect_stdout(io.StringIO()) as out:
            self.assertEqual(checks.main(["--root", self.root]), 1)
        self.assertIn("FAIL", out.getvalue())
        self.make()
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(checks.main(["--root", self.root, NAME]), 0)


if __name__ == "__main__":
    unittest.main()
