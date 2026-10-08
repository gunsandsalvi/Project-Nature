"""tools/art/parts_camp.py and parts_rocks.py make the art lane's camp and rocks families of parts (PRE-46, PRE-22,
A6.1, A6.3, A6.4): each part in a Blender file the exporter accepts, the same bytes every time, every texture layout
inside the kit's 1.5 to 1 line. These need Blender (tools/setup.sh); the stretch check also needs the kit's own checker,
kd_kit (set KD_KIT to its path)."""

import math
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


def kit_info(kdkit):
    """Each part's triangles and bounds (Godot's axes: x east, y up, z south) as the kit's own tool lists them."""
    run = subprocess.run([KD_KIT, "info", kdkit], capture_output=True, text=True, cwd=ROOT)
    found = {}
    for m in re.finditer(
        r"^part (\w+) (\d+) triangles bounds ([-\d.]+) ([-\d.]+) ([-\d.]+) to ([-\d.]+) ([-\d.]+) ([-\d.]+)",
        run.stdout,
        re.M,
    ):
        numbers = [float(n) for n in m.groups()[2:]]
        found[m.group(1)] = (int(m.group(2)), numbers)
    return found


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
        names += ["tent_pole_simple", "tent_pole_tip", "hide_cover_cone"]
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

    # checks: PRE-46 PRE-22
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_the_hearth_stones_are_low_and_the_firewood_is_as_big_as_its_sheet_says(self):
        found = kit_info(os.path.join(self.tmp.name, "camp.kdkit"))
        for number in range(1, 13):  # sheet 16.13: twelve stones, none above 20 cm
            for form in ("", "_simple", "_marker"):
                name = f"hearth_stone_{number:02d}{form}"
                self.assertIn(name, found)
                self.assertLessEqual(found[name][1][4], 0.2 + 1e-3, name)  # y1 is the top, in Godot's axes
        for name in ("firewood_branch_120_04", "firewood_branch_100_11", "firewood_trunk_15", "firewood_trunk_20"):
            self.assertIn(name, found)
        for name, length in (("firewood_trunk_18", 0.8), ("firewood_branch_120_04", 1.2)):  # sheet 16.23's lengths
            x0, _, _, x1, _, _ = found[name][1]
            self.assertAlmostEqual(x1 - x0, length, delta=0.06, msg=name)
        self.assertGreater(found["firewood_branch_120_04"][0], found["firewood_branch_120_04_simple"][0])
        self.assertIn("firewood_mound_marker", found)  # the heap's smallest form is one lump, not marker sticks
        self.assertGreater(found["hearth_stone_03"][0], found["hearth_stone_03_simple"][0])
        self.assertGreater(found["hearth_stone_03_simple"][0], found["hearth_stone_03_marker"][0])

    # checks: PRE-46
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_the_hearth_and_the_firewood_recipes_fit_the_family(self):
        with tempfile.TemporaryDirectory() as kit:
            shutil.copy(os.path.join(self.tmp.name, "camp.kdkit"), kit)
            run = subprocess.run(
                [KD_KIT, "check", os.path.join(ROOT, "data"), kit], capture_output=True, text=True, cwd=ROOT
            )
        said = run.stdout + run.stderr
        self.assertEqual([line for line in said.splitlines() if re.search(r"art:(hearth_ring|firewood)\w*", line)], [])

    # checks: PRE-46 PRE-22
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_the_screen_is_five_metres_wide_and_two_and_a_bit_high_its_six_hides_flap_posts_and_pole_all_there(self):
        found = kit_info(os.path.join(self.tmp.name, "camp.kdkit"))
        for name in "abcdef":  # sheet 16.12's six unequal hides, in the full and the simple form
            self.assertGreater(found[f"screen_hide_{name}"][0], found[f"screen_hide_{name}_simple"][0], name)
        self.assertIn("screen_curtain_marker", found)
        x0, y0, _, x1, y1, _ = found["screen_pole"][1]
        self.assertAlmostEqual(x1 - x0, 5.0, delta=0.02)  # the ridge pole, 5 m
        self.assertAlmostEqual(y0 + (y1 - y0) / 2.0, 2.20, delta=0.01)  # lying at z = 2.20
        for name in ("screen_post_left", "screen_post_right"):
            _, y0, _, _, y1, _ = found[name][1]
            self.assertAlmostEqual(y1, 2.30, delta=0.03, msg=name)  # 2.3 m showing
            self.assertAlmostEqual(y0, -0.10, delta=0.01, msg=name)  # and 0.1 m in the ground
        x0, y0, _, x1, y1, _ = found["screen_flap"][1]
        self.assertAlmostEqual(y1 - y0, 1.5, delta=0.05)  # the flap, 1.5 m tall
        self.assertAlmostEqual(x1 - x0, 0.84, delta=0.06)  # and 0.8 m wide
        lows, highs = [], []
        for name in "abcdef":
            x0, y0, _, x1, y1, _ = found[f"screen_hide_{name}"][1]
            lows.append(x0)
            highs.append(x1)
            self.assertLessEqual(y1, 2.16, name)  # hung from the ties at 2.15
        self.assertAlmostEqual(max(highs) - min(lows), 4.6, delta=0.15)  # the curtain, x 0.25 to 4.75 and its lobes
        x0, y0, _, x1, y1, _ = found["screen_stones"][1]
        self.assertAlmostEqual(x1 - x0, 4.45, delta=0.2)  # eleven stones along the foot
        self.assertAlmostEqual(y1, 0.28, delta=0.03)  # the highest 0.25 m made a seventh larger

    # checks: PRE-46
    @unittest.skipUnless(os.path.exists(KD_KIT), "kd_kit is not built (set KD_KIT to its path)")
    def test_the_screens_recipes_fit_the_family(self):
        with tempfile.TemporaryDirectory() as kit:
            shutil.copy(os.path.join(self.tmp.name, "camp.kdkit"), kit)
            run = subprocess.run(
                [KD_KIT, "check", os.path.join(ROOT, "data"), kit], capture_output=True, text=True, cwd=ROOT
            )
        said = run.stdout + run.stderr
        self.assertEqual([line for line in said.splitlines() if re.search(r"art:shelter_screen\w*", line)], [])


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
        return kit_info(os.path.join(self.tmp.name, "rocks.kdkit"))

    # checks: PRE-46
    def test_the_boulders_the_stones_the_flakes_and_the_lip_export_in_three_forms_with_their_joints(self):
        names = ["boulder_small", "boulder_block", "boulder_large", "boulder_split_a", "boulder_split_b"]
        names += ["boulder_perch_base", "boulder_perch_cap", "scree_stone_small", "scree_stone_medium"]
        names += [
            "scree_stone_large",
            "shale_flake_small",
            "shale_flake_medium",
            "cliff_lip",
            "cliff_lip_b",
            "cliff_lip_c",
        ]
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
        self.assertAlmostEqual(x1 - x0, 4.3, delta=0.01)  # 4 m between its joints and 15 cm of tail beyond each

    # checks: PRE-46
    def test_the_cliff_foot_has_no_rings_and_its_boulders_are_sunk_and_turned_every_way(self):
        import tomllib

        for name in ("cliff_foot", "cliff_foot_simple", "cliff_foot_small"):
            with open(os.path.join(ROOT, "art", "models", name, "record.toml"), "rb") as f:
                places = tomllib.load(f)["place"]
            self.assertTrue(
                all(p["rule"] == "ring" and p.get("count", 1) == 1 for p in places), name
            )  # no arcs of stones
            boulders = [p for p in places if p["parts"][0].startswith("boulder_")]
            self.assertEqual(len(boulders), 4)
            self.assertTrue(all(p["base"].startswith("-") for p in boulders), name)  # each sunk in the ground
            self.assertGreater(len({p["face"] for p in boulders}), 3, name)  # and turned its own way
            # how far out from the cliff (the east-west line through the origin) each stone lies, from its radius and
            # its bearing
            out = sorted(
                -float(p["radius"].split()[0]) * math.cos(math.radians(p["turn"]))
                for p in places
                if not p["parts"][0].startswith("boulder_")
            )
            self.assertLess(
                out[len(out) // 2], 1.4, name
            )  # half the stones lie within 1.4 m of the lip: it thins outward
            self.assertGreater(out[int(len(out) * 0.95)], 2.5, name)  # and the last of them lie much farther out


if __name__ == "__main__":
    unittest.main()
