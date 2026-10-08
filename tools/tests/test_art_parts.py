"""tools/art/parts_camp.py makes the art lane's camp family of parts (PRE-46, PRE-22, A6.1, A6.3, A6.4): each part in a
Blender file the exporter accepts, the same bytes every time, every texture layout inside the kit's 1.5 to 1 line. These
need Blender (tools/setup.sh); the stretch check also needs the kit's own checker, kd_kit (set KD_KIT to its path)."""

import os
import re
import shutil
import subprocess
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
SCRIPT = os.path.join(ROOT, "tools", "art", "parts_camp.py")
BLENDER = shutil.which("blender")
KD_KIT = os.environ.get("KD_KIT", os.path.join(ROOT, "build", "view", "kd_kit"))


def make(out_folder):
    """Runs the family script in Blender (one thread, as tools/kit.py does) and returns the exporter's one line and the
    kdkit file's bytes."""
    blend, kdkit = os.path.join(out_folder, "camp.blend"), os.path.join(out_folder, "camp.kdkit")
    run = subprocess.run(
        [BLENDER, "--background", "--factory-startup", "--threads", "1", "--python-exit-code", "1"]
        + ["--python", SCRIPT, "--", blend, kdkit],
        capture_output=True,
        text=True,
    )
    assert run.returncode == 0, run.stdout[-1500:] + run.stderr[-1500:]
    said = [line for line in run.stdout.splitlines() if line.startswith("KDKIT:")]
    with open(kdkit, "rb") as f:
        return said[-1], f.read()


@unittest.skipUnless(BLENDER, "Blender is not installed (tools/setup.sh)")
class CampFamily(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.said, cls.data = make(cls.tmp.name)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    # checks: PRE-46
    def test_the_club_and_its_two_smaller_forms_export_with_their_joints(self):
        parts, triangles, joints = (int(n) for n in re.findall(r"\d+", self.said)[:3])
        self.assertGreaterEqual(parts, 3)  # club, club_simple, club_small
        self.assertGreaterEqual(joints, 6)  # a grip and a head on each form
        self.assertGreater(triangles, 500)
        for name in (b"club", b"club_simple", b"club_small", b"grip", b"head"):
            self.assertTrue(name in self.data, f"{name!r} is not in the exported kit")

    # checks: PRE-46
    def test_the_tent_exports_as_eleven_hides_ten_poles_worth_of_pole_a_lashing_a_flap_and_three_stones(self):
        names = [f"hide_lower_{i}" for i in range(1, 8)] + [f"hide_upper_{i}" for i in range(1, 5)]
        names += ["tent_pole", "tent_binding", "tent_door_flap", "ring_stone_small", "ring_stone_medium"]
        names += ["ring_stone_large", "door_top", "crossing", "foot", "bind", "tip", "centre", "hinge"]
        for name in names:
            self.assertTrue(name.encode() in self.data, f"{name!r} is not in the exported kit")

    # checks: PRE-46
    def test_the_same_script_gives_the_same_bytes(self):
        with tempfile.TemporaryDirectory() as again:
            self.assertTrue(make(again)[1] == self.data, "the exported bytes changed between two runs")

    # checks: PRE-22
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_no_triangle_of_any_part_stretches_its_texture_past_one_and_a_half_to_one(self):
        with tempfile.TemporaryDirectory() as kit:
            shutil.copy(os.path.join(self.tmp.name, "camp.kdkit"), kit)
            run = subprocess.run(
                [KD_KIT, "check", os.path.join(ROOT, "data"), kit], capture_output=True, text=True, cwd=ROOT
            )
        self.assertNotIn("past 1.5 to 1", run.stdout + run.stderr, run.stdout + run.stderr)

    # checks: PRE-46
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_the_clubs_and_the_tents_recipes_fit_the_family(self):
        with tempfile.TemporaryDirectory() as kit:
            shutil.copy(os.path.join(self.tmp.name, "camp.kdkit"), kit)
            run = subprocess.run(
                [KD_KIT, "check", os.path.join(ROOT, "data"), kit], capture_output=True, text=True, cwd=ROOT
            )
        said = run.stdout + run.stderr  # the stand-in family's recipes may find no stand-in file here; ours must fit
        self.assertEqual([line for line in said.splitlines() if re.search(r"art:(club|hide_tent_cone)\b", line)], [])
        self.assertRegex(said, r"\d+ recipes", "the checker did not say it looked at the recipes")


if __name__ == "__main__":
    unittest.main()
