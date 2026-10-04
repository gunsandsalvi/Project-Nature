"""The look's fire and smoke, judged in a picture the cloud draws of P3's model sheet at night (PRE-30, MAT-18,
PRC-10): its flames show in the fire's own colours, its light warms the ground round each fire, and its smoke
greys what stands behind it. Drawn by tools/picture.sh, as the note's pictures are; skipped where no Godot or
virtual screen is set up.
"""

import os
import shutil
import subprocess
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
GODOT = os.path.join(os.path.expanduser("~"), ".cache", "kindling", "godot", "Godot_v4.7.2-stable_linux.x86_64")


def draw(*args):
    """The app's picture with these arguments, as RGB pixels, drawn as the phone shows it and halved."""
    from PIL import Image

    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, "picture.png")
        run = subprocess.run(
            [os.path.join(ROOT, "tools", "picture.sh"), os.path.join(ROOT, "prototypes", "app"), out, "portrait"]
            + list(args),
            capture_output=True,
            text=True,
            timeout=600,
        )
        if run.returncode != 0:
            raise AssertionError(run.stdout + run.stderr)
        image = Image.open(out).convert("RGB")
        # the picture without the buttons and readout along its foot
        return image.crop((0, 0, image.width, int(image.height * 0.86)))


def share(image, test):
    """The share of the picture's pixels that pass the test, given the red, green and blue arrays."""
    import numpy as np

    rgb = np.asarray(image).astype(int)
    return float(np.mean(test(rgb[:, :, 0], rgb[:, :, 1], rgb[:, :, 2])))


@unittest.skipUnless(os.path.exists(GODOT) and shutil.which("xvfb-run"), "needs Godot and a virtual screen")
class FireAtNight(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.night = draw("kit", "hour=night")
        cls.clear = draw("kit", "hour=night", "smoke=off")

    # checks: PRE-30 MAT-18
    def test_flames_show_in_the_fires_colours(self):
        # the fire ramp's bright yellows and oranges, which nothing else on the sheet has at night
        flames = share(self.clear, lambda r, g, b: (r > 230) & (g > 150) & (b < 140))
        self.assertGreater(flames, 0.001)

    # checks: PRE-30 MAT-18
    def test_firelight_warms_the_ground_round_the_fires(self):
        # the night is blue-green; firelit ground leans warm, red above blue, over a wide pool
        warm = share(self.clear, lambda r, g, b: (r > b + 25) & (g > 70))
        self.assertGreater(warm, 0.06)

    # checks: PRE-30
    def test_smoke_greys_what_stands_behind_it(self):
        # the smoke's ramp at night is a grey with a little purple, blue above green: the picture with smoke has
        # several times more of it than the same picture without
        def grey(r, g, b):
            return (abs(r - g) < 22) & (b > g + 6) & (g > 70) & (g < 170)

        self.assertGreater(share(self.night, grey) - share(self.clear, grey), 0.015)


if __name__ == "__main__":
    unittest.main()
