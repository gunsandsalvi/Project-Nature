"""tools/art/parts_camp.py and parts_rocks.py make the art lane's camp and rocks families of parts (PRE-46, PRE-22,
A6.1, A6.3, A6.4): each part in a Blender file the exporter accepts, the same bytes every time, every texture layout
inside the kit's 1.5 to 1 line. These need Blender (tools/setup.sh); the stretch check also needs the kit's own checker,
kd_kit (set KD_KIT to its path)."""

import os
import re
import shutil
import subprocess
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
SCRIPT = os.path.join(ROOT, "tools", "art", "parts_camp.py")
ROCKS_SCRIPT = os.path.join(ROOT, "tools", "art", "parts_rocks.py")
BLENDER = shutil.which("blender")
KD_KIT = os.environ.get("KD_KIT", os.path.join(ROOT, "build", "view", "kd_kit"))


def make(out_folder, script=SCRIPT, family="camp"):
    """Runs a family script in Blender (one thread, as tools/kit.py does) and returns the exporter's one line and the
    kdkit file's bytes."""
    blend, kdkit = os.path.join(out_folder, family + ".blend"), os.path.join(out_folder, family + ".kdkit")
    run = subprocess.run(
        [BLENDER, "--background", "--factory-startup", "--threads", "1", "--python-exit-code", "1"]
        + ["--python", script, "--", blend, kdkit],
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
        names += ["tent_pole_simple", "hide_cover_cone"]
        names += [f"ring_stone_{size}_{form}" for size in ("small", "medium", "large") for form in ("simple", "marker")]
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
        self.assertEqual([line for line in said.splitlines() if re.search(r"art:(club|hide_tent_cone\w*)", line)], [])
        self.assertRegex(said, r"\d+ recipes", "the checker did not say it looked at the recipes")


@unittest.skipUnless(BLENDER, "Blender is not installed (tools/setup.sh)")
class RocksFamily(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.said, cls.data = make(cls.tmp.name, ROCKS_SCRIPT, "rocks")

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def info(self):
        """Each part's triangles and bounds (Godot's axes: x east, y up, z south) as the kit's own tool lists them."""
        run = subprocess.run(
            [KD_KIT, "info", os.path.join(self.tmp.name, "rocks.kdkit")], capture_output=True, text=True, cwd=ROOT
        )
        found = {}
        for m in re.finditer(
            r"^part (\w+) (\d+) triangles bounds ([-\d.]+) ([-\d.]+) ([-\d.]+) to ([-\d.]+) ([-\d.]+) ([-\d.]+)",
            run.stdout,
            re.M,
        ):
            numbers = [float(n) for n in m.groups()[2:]]
            found[m.group(1)] = (int(m.group(2)), numbers)
        return found

    # checks: PRE-46
    def test_the_boulders_the_stones_the_flakes_and_the_lip_export_in_three_forms_with_their_joints(self):
        names = ["boulder_small", "boulder_block", "boulder_large", "boulder_split_a", "boulder_split_b"]
        names += ["boulder_perch_base", "boulder_perch_cap", "scree_stone_small", "scree_stone_medium"]
        names += ["scree_stone_large", "shale_flake_small", "shale_flake_medium", "cliff_lip"]
        for name in names:
            for form in ("", "_simple", "_marker"):
                self.assertTrue((name + form).encode() in self.data, f"{name + form!r} is not in the exported kit")
        for joint_name in (b"top", b"seat", b"start", b"end"):
            self.assertTrue(joint_name in self.data, f"the joint {joint_name!r} is not in the exported kit")

    # checks: PRE-46
    def test_the_same_script_gives_the_same_bytes(self):
        with tempfile.TemporaryDirectory() as again:
            self.assertTrue(make(again, ROCKS_SCRIPT, "rocks")[1] == self.data, "the exported bytes changed")

    # checks: PRE-22
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_no_triangle_of_any_rock_stretches_its_texture_past_one_and_a_half_to_one(self):
        with tempfile.TemporaryDirectory() as kit:
            shutil.copy(os.path.join(self.tmp.name, "rocks.kdkit"), kit)
            run = subprocess.run(
                [KD_KIT, "check", os.path.join(ROOT, "data"), kit], capture_output=True, text=True, cwd=ROOT
            )
        self.assertNotIn("past 1.5 to 1", run.stdout + run.stderr, run.stdout + run.stderr)

    # checks: PRE-46
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_the_rocks_recipes_fit_the_family(self):
        with tempfile.TemporaryDirectory() as kit:
            shutil.copy(os.path.join(self.tmp.name, "rocks.kdkit"), kit)
            run = subprocess.run(
                [KD_KIT, "check", os.path.join(ROOT, "data"), kit], capture_output=True, text=True, cwd=ROOT
            )
        said = run.stdout + run.stderr
        mine = r"art:(boulders\w*|boulder_split\w*|boulder_perched\w*|cliff_lip\w*|cliff_foot\w*)"
        self.assertEqual([line for line in said.splitlines() if re.search(mine, line)], [])

    # checks: PRE-46 PRE-22
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_each_form_has_fewer_triangles_than_the_one_before_and_each_rock_is_as_big_as_its_sheet_says(self):
        found = self.info()
        for name in ("boulder_small", "boulder_large", "scree_stone_large", "shale_flake_medium", "cliff_lip"):
            self.assertGreater(found[name][0], found[name + "_simple"][0], name)
            self.assertGreater(found[name + "_simple"][0], found[name + "_marker"][0], name)
        # sheet 2.16's stones: width east, height up, depth south, within a tenth of a metre or so
        for name, (wide, high, deep) in {
            "boulder_small": (1.0, 0.7, 0.8),
            "boulder_block": (2.0, 1.4, 1.5),
            "boulder_large": (3.0, 2.0, 2.2),
        }.items():
            x0, y0, z0, x1, y1, z1 = found[name][1]
            self.assertAlmostEqual(x1 - x0, wide, delta=0.15, msg=name)
            self.assertAlmostEqual(y1 - y0, high, delta=0.08, msg=name)
            self.assertAlmostEqual(z1 - z0, deep, delta=0.15, msg=name)
            self.assertAlmostEqual(y0, 0.0, delta=0.01, msg=name + " stands on the ground")
        x0, _, _, x1, _, _ = found["cliff_lip"][1]
        self.assertAlmostEqual(x1 - x0, 4.0, delta=0.01)


if __name__ == "__main__":
    unittest.main()
