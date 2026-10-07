"""The colour measures come from `kindling look` (PRE-20, A5.4; CLAUDE.md, rule 4): a picture handed over as raw
RGBA, its measures read by name, its four-number change written back, and plain errors when the program is missing
or fails. A stand-in program checks the handing over; the real one, when built, its measures."""

import os
import stat
import sys
import tempfile
import textwrap
import unittest
from unittest import mock

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import look  # noqa: E402

STAND_IN = textwrap.dedent(
    """\
    #!{python}
    import sys
    _, look, command, w, h, *numbers = sys.argv
    data = sys.stdin.buffer.read()
    if look != "look" or len(data) != int(w) * int(h) * 4:
        sys.stderr.write("bad picture")
        sys.exit(2)
    if command == "stats":
        print("lightness", data[0], "colourfulness 1.5 hue 90 contrast", data[3], "accents", len(numbers))
    elif command == "adjust" and len(numbers) == 4:
        out = bytearray(data)
        out[0::4], out[2::4] = data[2::4], data[0::4]
        sys.stdout.buffer.write(bytes(out))
    else:
        sys.exit(2)
    """
)
FAILING = "#!{python}\nimport sys\nsys.stderr.write('it broke')\nsys.exit(1)\n"


def program(folder, name, text):
    path = os.path.join(folder, name)
    with open(path, "w") as f:
        f.write(text.format(python=sys.executable))
    os.chmod(path, os.stat(path).st_mode | stat.S_IXUSR)
    return path


class HandingOver(unittest.TestCase):
    def setUp(self):
        d = tempfile.TemporaryDirectory()
        self.addCleanup(d.cleanup)
        self.dir = d.name
        self.picture = np.random.default_rng(1).integers(0, 256, (5, 7, 3)).astype(np.uint8)

    def using(self, path):
        patch = mock.patch.dict(os.environ, {"KINDLING": path})
        patch.start()
        self.addCleanup(patch.stop)

    # checks: PRE-20
    def test_measures_are_read_by_name_from_rgba_rows(self):
        self.using(program(self.dir, "kindling", STAND_IN))
        s = look.stats(self.picture)
        self.assertEqual(s["lightness"], self.picture[0, 0, 0])  # the top left pixel's red comes first
        self.assertEqual(s["contrast"], 255)  # then green, blue and an opaque alpha
        self.assertEqual(s["accents"], 0)  # no numbers follow `stats <w> <h>`
        self.assertEqual(s["hue"], 90)

    # checks: PRE-20
    def test_a_change_comes_back_as_a_picture(self):
        self.using(program(self.dir, "kindling", STAND_IN))
        out = look.adjust(self.picture, 1.2581, -6.206, 103.129, 82.8631)
        np.testing.assert_array_equal(out, self.picture[..., ::-1])

    # checks: PRE-20
    def test_a_missing_or_failing_program_is_a_plain_error(self):
        self.using(os.path.join(self.dir, "nowhere"))
        with self.assertRaisesRegex(look.LookError, "KINDLING"):
            look.stats(self.picture)
        self.using(program(self.dir, "broken", FAILING))
        with self.assertRaisesRegex(look.LookError, "it broke"):
            look.stats(self.picture)

    # checks: PRE-20
    def test_small_levels_are_measured_round_their_wrap(self):
        small = self.picture[:4, :4]
        big = look.tiled(small)
        self.assertEqual(big.shape, (64, 64, 3))
        np.testing.assert_array_equal(big[4:8, 8:12], small)
        self.assertEqual(look.tiled(np.zeros((100, 70, 3), np.uint8)).shape, (100, 70, 3))


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class TheRealProgram(unittest.TestCase):
    # checks: PRE-20
    def test_a_flat_grey_has_no_colour_and_no_contrast(self):
        s = look.stats(np.full((64, 64, 3), 128, np.uint8))
        self.assertLess(s["colourfulness"], 0.5)
        self.assertLess(s["contrast"], 0.5)
        self.assertIn("accents", s)
        self.assertNotIn("accents", look.stats(np.full((16, 16, 3), 128, np.uint8)), "under 29 pixels")

    # checks: PRE-20
    def test_no_change_changes_nothing_and_lightness_adds(self):
        t = np.random.default_rng(2).integers(40, 220, (32, 32, 3)).astype(np.uint8)
        self.assertLessEqual(np.abs(look.adjust(t).astype(int) - t).max(), 1)
        before = look.stats(t)["lightness"]
        self.assertAlmostEqual(look.stats(look.adjust(t, lightness=5.0))["lightness"] - before, 5.0, delta=0.5)

    # checks: PRE-20 PRE-22
    def test_a_drawn_level_is_brought_to_the_contrast_above_and_then_a_share_more(self):
        import fit  # noqa: E402

        rng = np.random.default_rng(4)
        above = rng.integers(60, 200, (64, 64, 3)).astype(np.uint8)
        calm = (128 + (above.astype(int) - 128) * 0.5).astype(np.uint8)  # the same marks at half the contrast
        want = look.stats(look.tiled(above))["contrast"]
        matched = look.stats(look.tiled(fit.calibrate(calm, above, match=True)[0]))["contrast"]
        more = look.stats(look.tiled(fit.calibrate(calm, above, match=True, more=130.0)[0]))["contrast"]
        self.assertAlmostEqual(matched, want, delta=0.12 * want)
        self.assertGreater(more, 1.15 * matched)


if __name__ == "__main__":
    unittest.main()
