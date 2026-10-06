"""The art lane's checks (PRE-20, PRE-22, PRE-42, A5.3, A5.4; IMPLEMENTATION.md T2.3a.1): each catches its planted
fault, a missing record, a stale level, painted light, a seam, a repeat, an averaged level whose accents fall to
about 77% of band 0's, a wrong digest, too much re-grid loss for the first level or a redrawn one, contrast far from
the source's and drift between bands; for a big surface, a tile with the wrong first band, a middle tile short of
the near tile's accents, drift across tiles and a version that does not join the others. The texture they start
from is the meadow's committed levels, which pass, under a record of their own."""

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
import tile  # noqa: E402

MEADOW = os.path.join(ROOT, "art", "textures", "meadow")
NAME = "ground"
SAME = object()  # the meadow's own source on band 0's grid


def averaged(t, k):
    """The level k times smaller made by averaging, as mipmaps are: what the plan forbids."""
    h, w = t.shape[0] // k, t.shape[1] // k
    return np.round(t[: h * k, : w * k].astype(float).reshape(h, k, w, k, 3).mean(axis=(1, 3))).astype(np.uint8)


class Fixture(unittest.TestCase):
    """Texture folders made in a scratch root from the meadow's levels."""

    @classmethod
    def setUpClass(cls):
        cls.levels = [texels.load(os.path.join(MEADOW, f"b{n}.png")) for n in range(9)]
        cls.source = texels.load(os.path.join(MEADOW, "source.png"))

    def setUp(self):
        d = tempfile.TemporaryDirectory()
        self.addCleanup(d.cleanup)
        self.root = d.name
        self.folder = os.path.join(self.root, "art", "textures", NAME)

    def make(self, levels=None, source=SAME, loss="4.0%", sub="", first_band=0, made_from=None, band_loss=None):
        """A tile folder in the scratch root (the material's own, or `sub` inside it), its record written as the
        plan sets it out; with source None, no source.png is kept; made_from gives each level's origin digests (a
        version's); band_loss gives a redrawn level's own re-grid loss, {level: text}."""
        levels = self.levels if levels is None else levels
        folder = os.path.join(self.folder, sub)
        os.makedirs(folder, exist_ok=True)
        for rel in (f"art/sources/{NAME}/{NAME}-01.webp", f"art/requests/{NAME}-01.txt"):
            os.makedirs(os.path.dirname(os.path.join(self.root, rel)), exist_ok=True)
            with open(os.path.join(self.root, rel), "w") as f:
                f.write("a stand-in\n")
        bands, above = [], None
        for n, t in enumerate(levels):
            rel = os.path.relpath(os.path.join(folder, f"b{n}.png"), self.root)
            texels.save_png(t, os.path.join(self.root, rel))
            b = {"level": n, "file": rel, "sha256": texels.sha256(os.path.join(self.root, rel)), "way": "test"}
            if made_from is not None:
                b["made_from"] = made_from[n]
            elif n:
                b["made_from"] = above
            if n and band_loss and n in band_loss:
                b["way"] = "GPT redraw of the level above, for a test"
                b["regrid_loss"] = band_loss[n]
            if n:
                b["calibration"] = "lightness +0.0%, hue +0.0 degrees, colourfulness 100%, contrast 100%"
            bands.append(b)
            above = b["sha256"]
        if source is not None:
            texels.save_png(self.source if source is SAME else source, os.path.join(folder, "source.png"))
        rec = {
            "about": "a test ground",
            "route": "picture",
            "tile_texels": 256,
            "texels_a_metre": 64 >> first_band,
            "first_band": first_band,
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
        record.write(os.path.join(folder, "record.toml"), rec)
        return [b["sha256"] for b in bands]

    def results(self, where=""):
        return {c: (ok, what) for w, c, ok, what in checks.check_texture(self.folder, self.root) if w == where}

    def failed(self, check, where=""):
        ok, what = self.results(where)[check]
        self.assertIs(ok, False, f"{where} {check} should fail: {what}")
        return what


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class PlantedFaults(Fixture):
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
        self.assertEqual(checks.check_texture(self.folder, self.root), [("", "record", False, "no record.toml")])

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
        self.assertEqual(self.failed("stale"), "levels 4 made from an older level")

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
    def test_a_redrawn_levels_seam(self):
        """A redrawn level whose wrap steps more than any of its own boundaries inside."""
        half = self.levels[1].copy()
        half[:, 64:] = look.adjust(half[:, 64:], lightness=8.0)
        self.make(self.levels[:1] + [half] + self.levels[2:], band_loss={1: "4.0%"})
        self.assertIn("redrawn level 1's wrap", self.failed("seams"))

    # checks: PRE-22
    def test_a_repeat(self):
        """Band 0 made by repeating a smaller tile: seamless, but the same 2 m four times over."""
        self.make([np.tile(self.levels[1], (2, 2, 1))] + self.levels[1:])
        self.assertIs(self.results()["seams"][0], True)
        self.failed("repeat")

    # checks: PRE-20 PRE-22
    def test_an_averaged_level_loses_its_accents(self):
        """Band 2 made as a mipmap is, by averaging band 0: about 77% of band 0's accents."""
        self.make(self.levels[:2] + [averaged(self.levels[0], 4)] + self.levels[3:])
        what = self.failed("accents")
        share = int(re.search(r"band 2 (\d+)%", what).group(1))
        self.assertTrue(70 <= share <= 85, what)

    # checks: PRE-22
    def test_too_much_regrid_loss(self):
        self.make(loss="12.0%")
        self.assertEqual(self.failed("loss"), "first level 12.0%")
        shutil.rmtree(self.folder)
        self.make(band_loss={1: "11.5%"})
        self.assertEqual(self.failed("loss"), "level 1 11.5%")

    # checks: PRE-22 PRE-42
    def test_no_loss_past_the_line_is_kept_by_any_note(self):
        self.make(loss="21.3%, kept by the owner's word of 6 October 2026")
        self.assertEqual(self.failed("loss"), "first level 21.3%")

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


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class BigSurfaces(Fixture):
    """A big surface: the meadow's levels as its near tile, and again, or changed, as its middle tile."""

    def big(self, middle=None, first_band=2):
        self.make()
        self.make(middle or self.levels, sub="middle", first_band=first_band, source=None)

    # checks: PRE-22 PRE-42
    def test_a_middle_tile_passes_and_judges_only_the_bands_it_serves(self):
        self.big()
        got = self.results("middle")
        self.assertIs(got["record"][0], True, got["record"][1])
        self.assertIn("band 2", got["accents"][1])
        self.assertIn("band 3", got["accents"][1])
        self.assertNotIn("band 4", got["accents"][1])
        near = self.results()["accents"][1]
        self.assertIn("band 1", near)
        self.assertNotIn("band 2", near)

    # checks: PRE-42
    def test_a_tile_with_the_wrong_first_band(self):
        self.big(first_band=4)
        self.assertIn("first_band is 4, not 2 for a middle tile", self.failed("record", "middle"))

    # checks: PRE-20 PRE-22
    def test_a_middle_tile_short_of_the_near_tiles_accents(self):
        flat = [look.adjust(t, contrast=40.0) for t in self.levels]
        self.big(middle=flat)
        self.assertIn("under 90%", self.failed("accents", "middle"))

    # checks: PRE-20
    def test_drift_across_tiles(self):
        darker = [look.adjust(t, lightness=-5.0) for t in self.levels]
        self.big(middle=darker)
        self.assertIn("2 (lightness", self.failed("drift", "middle"))

    # checks: PRE-22
    def test_versions_that_join_and_one_that_does_not(self):
        own = self.make()
        version, _ = tile.version(self.levels, seed=4)
        self.make(version, sub="v2", source=None, made_from=own)
        got = self.results()
        self.assertIs(got["joints"][0], True, got["joints"][1])
        self.assertIs(self.results("v2")["stale"][0], True)
        off = [np.roll(t, t.shape[0] // 3, axis=1) for t in version]  # its edges no longer the tile's
        shutil.rmtree(os.path.join(self.folder, "v2"))
        self.make(off, sub="v2", source=None, made_from=own)
        self.failed("joints")
        stale = list(own)
        stale[1] = "0" * 64
        shutil.rmtree(os.path.join(self.folder, "v2"))
        self.make(version, sub="v2", source=None, made_from=stale)
        self.assertEqual(self.failed("stale", "v2"), "levels 1 made from an older level")


if __name__ == "__main__":
    unittest.main()
