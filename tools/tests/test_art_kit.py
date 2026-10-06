"""The kit's parts in Blender (PRE-46, PRE-22, A6.1, A6.4): kit.py builds parts that keep the rules; partcheck.py
catches each planted fault (a stretched part, a slot named by no role, a main joint off the origin, a vertex weighted
by no bone, a mirrored texture); models.py makes a family's file, stretch report and preview sheet; and every
committed family passes its check. Skipped where Blender is not installed (it joins the cloud's setup at T2.3b.1)."""

import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(TOOLS)
ART = os.path.join(TOOLS, "art")
BLENDER = shutil.which(os.environ.get("BLENDER", "blender"))
XVFB = shutil.which("xvfb-run")
sys.path.insert(0, ART)
import kitmath  # noqa: E402

SCRIPT = """
import json, math, os, random, sys
sys.path.insert(0, {art!r})
import bpy
import kit

kit.start()
facts = {{}}

p = kit.Part("pole", group="wood", about="a pole")
facts["pole_wrap"] = p.sleeve([(0, 0, 0), (0, 0, 1.0), (0, 0, 2.0)], [0.05, 0.045, 0.04], sides=8, role="bark",
                              cap_role="wood")
p.joint("base")
p.joint("top", at=(0, 0, 2.0))
p.done()

c = kit.Part("pole_cut", group="wood", about="a pole chopped at its foot and broken at its top")
c.sleeve([(0, 0, 0.06), (0, 0, 2.0)], [0.05, 0.04], sides=8, role="bark", cap_role="wood", caps=("chopped", "broken"))
c.joint("base", towards=(0, 0, -1))
ob = c.done()
facts["pole_cut_low"] = round(min(v.co.z for v in ob.data.vertices), 4)

br = kit.Part("branch", group="wood", about="a branch tapering 3:1 in three wraps of one mesh")
facts["branch_wraps"] = br.sleeve([(0, 0, 0), (0, 0, 0.4), (0, 0, 0.8), (0, 0, 1.2)], [0.035, 0.025, 0.018, 0.012],
                                  sides=6, caps=(None, "broken"), breaks=(1, 2))
br.joint("base", towards=(0, 0, -1))
ob = br.done()
us = [d.uv.x for d in ob.data.uv_layers[0].data]
facts["branch_u"] = [round(min(us) * 64, 3), round(max(us) * 64, 3)]
facts["pole_u"] = [round(64 * d.uv.x, 3) for d in bpy.data.objects["pole"].data.uv_layers[0].data]
facts["pole_u_bark"] = [
    round(64 * ob2.data.uv_layers[0].data[li].uv.x, 3)
    for ob2 in [bpy.data.objects["pole"]]
    for f in ob2.data.polygons if ob2.material_slots[f.material_index].name == "bark"
    for li in f.loop_indices
]

w = kit.Part("welded", group="covers", about="two panels welded into one")
w.panel([(-0.3, 0.0), (0.3, 0.0), (0.3, 0.3), (-0.3, 0.3)], spacing=0.1)
w.panel([(-0.3, 0.3), (0.3, 0.3), (0.3, 0.6), (-0.3, 0.6)], spacing=0.1)
before = len(w.bm.verts)
w.weld()
facts["weld_merged"] = before - len(w.bm.verts)
places = [tuple(round(c, 6) for c in v.co) for v in w.bm.verts]
facts["weld_together"] = len(places) - len(set(places))
w.joint("bottom", towards=(0, 0, -1))
w.done()

b = kit.Part("basket", group="things", about="a basket")
b.lathe([(0.12, 0.0), (0.16, 0.3), (0.15, 0.3), (0.11, 0.02)], sides=12, role="weave", caps=(True, True))
b.joint("base")
ob = b.done()
ob.data.calc_normals_split()
caps = [f for f in ob.data.polygons if len(f.vertices) == 12]
facts["basket_caps"] = [round(f.normal.z, 3) for f in caps]

h = kit.Part("hide", group="covers", about="a hide")
h.panel([(-0.5, 0.0), (0.5, 0.0), (0.55, 0.8), (0.0, 0.95), (-0.55, 0.8)], role="hide", spacing=0.1,
        bend=lambda x, y: (x, 0.15 * math.sin(y * 2.0), y))
h.joint("top")
h.done()

t = kit.Part("tuft", group="plants", about="a tuft")
for i in range(3):
    t.card(0.3, 0.4, role="grass", yaw=i * math.pi / 3, lean=0.15, uv_at=(0.35 * i, 0.0))
t.joint("base")
t.done()

s = kit.Part("stone", group="stones", about="a stone")
rnd = random.Random(2)
s.hull([(0.2 * rnd.uniform(-1, 1), 0.15 * rnd.uniform(-1, 1), 0.1 * rnd.uniform(-1, 1)) for _ in range(30)],
       rounds=1, flat_base=-0.06)
s.rest()
s.project("stone")
s.joint("base")
s.done()

arm = kit.armature("figure", [("thigh", (0, 0, 0.9), (0, 0, 0.5), None), ("shin", (0, 0, 0.5), (0, 0, 0.08), "thigh")])
skin = kit.Atlas("figure_skin", 2.0, 2.0)
leg = kit.Part("leg", group="figures", about="a leg", armature=arm, atlas=skin)
leg.sleeve([(0, 0, 0.9), (0, 0, 0.6), (0, 0, 0.5), (0, 0, 0.4), (0, 0, 0.08)], [0.08, 0.06, 0.05, 0.05, 0.035],
           sides=10, role="skin", wrap=False, caps=(False, True), seam=math.pi / 2,
           weights=[{{"thigh": 1}}, {{"thigh": 1}}, {{"thigh": 0.5, "shin": 0.5}}, {{"shin": 1}}, {{"shin": 1}}],
           keys={{"build_stout": {{"radii": [0.1, 0.075, 0.06, 0.06, 0.04]}}}})
leg.done()
facts["leg_uv"] = [round(min(x.uv.x for x in bpy.data.objects["leg"].data.uv_layers[0].data), 3)]
kit.pose(arm, "bend_test", {{"shin": (30, 0, 0)}})
foot = kit.Part("foot", group="figures", about="a foot beside the leg in its atlas", armature=arm, atlas=skin)
foot.sleeve([(0, 0, 0.08), (0, -0.2, 0.03)], [0.04, 0.03], sides=8, role="skin", wrap=False, caps=(None, True),
            weights=[{{"shin": 1}}, {{"shin": 1}}])
foot.done()

# two parts of one bent chain, each turned by the whole chain's frames, at rest and in a shape key
chain = [(0.3, 0, 1.0), (0.35, 0, 0.8), (0.42, 0, 0.62), (0.45, -0.05, 0.45)]
keyed = [(0.3, 0, 1.0), (0.36, 0, 0.8), (0.44, 0, 0.62), (0.48, -0.06, 0.45)]
radii = [0.05, 0.045, 0.04, 0.035]
fr, kfr = kit.frames(chain), kit.frames(keyed)
for name, a, b in (("upper", 0, 3), ("lower", 2, 4)):
    q = kit.Part(name, group="figures", about="half of a bent limb", armature=arm, atlas=skin)
    q.sleeve(chain[a:b], radii[a:b], sides=8, role="skin", wrap=False, caps=(None, None),
             weights=[{{"thigh": 1}}] * (b - a), along_frames=fr[a:b],
             keys={{"bulk": {{"points": keyed[a:b], "radii": [r * 1.1 for r in radii[a:b]], "frames": kfr[a:b]}}}})
    q.done()
up, lo = bpy.data.objects["upper"].data, bpy.data.objects["lower"].data
pairs = list(zip(range(16, 24), range(0, 8)))  # the upper's last ring, the lower's first
facts["shared_gap"] = max((up.vertices[i].co - lo.vertices[j].co).length for i, j in pairs)
kb_up, kb_lo = up.shape_keys.key_blocks["bulk"], lo.shape_keys.key_blocks["bulk"]
facts["shared_gap_key"] = max((kb_up.data[i].co - kb_lo.data[j].co).length for i, j in pairs)
alone = kit.frames(chain[2:4])[0]  # the lower half's own first frame differs from the chain's there
facts["own_frame_turn"] = round(math.degrees(alone[0].angle(fr[2][0])), 1)

tapered = kit.Part("tapered", group="wood", about="a branch narrowing 4:1, its wraps placed by itself")
narrowing = [0.04, 0.032, 0.025, 0.02, 0.015, 0.012, 0.01]
facts["auto_wraps"] = tapered.sleeve([(0, 0, 0.2 * i) for i in range(7)], narrowing, sides=6, caps=(None, None),
                                     breaks="auto")
tapered.joint("base", towards=(0, 0, -1))
tapered.done()

ankle = kit.Part("ankle", group="figures", about="a leg turning sharply forward into a foot", armature=arm, atlas=skin)
ankle.sleeve([(0.6, 0, 0.3), (0.6, 0, 0.12), (0.6, 0, 0.07), (0.6, -0.05, 0.035), (0.6, -0.15, 0.025)],
             [0.04, 0.035, 0.035, (0.04, 0.03), (0.04, 0.02)], sides=8, role="skin", wrap=True, caps=(None, True),
             weights=[{{"shin": 1}}] * 5)
facts["over_before"] = len(ankle.overstretched(1.4))
facts["charted"] = ankle.chart_overstretched(1.4)
facts["over_after"] = len(ankle.overstretched(1.4))
ankle.done()

# planted faults, one a part
q = kit.Part("squashed", group="faults", about="a pole stretched after its texture was laid")
q.sleeve([(0, 0, 0), (0, 0, 1.0)], [0.05, 0.05], role="bark")
for v in q.bm.verts:
    v.co.z *= 1.8
q.joint("base")
q.done()
r = kit.Part("painted", group="faults", about="a slot named by a colour, not a role")
r.sleeve([(0, 0, 0), (0, 0, 1.0)], [0.05, 0.05], role="bark")
r.joint("base")
r.done().material_slots[0].material = bpy.data.materials.new("brown paint")
o = kit.Part("off", group="faults", about="its main joint moved off its origin")
o.sleeve([(0, 0, 0), (0, 0, 1.0)], [0.05, 0.05], role="bark")
o.joint("base")
o.done().children[0].location = (0.2, 0.0, 0.0)
m = kit.Part("mirrored", group="faults", about="a texture laid backwards")
m.card(0.3, 0.3, role="leaf", uv_at=(0.0, 0.0))
for f in m.bm.faces:
    for loop in f.loops:
        loop[m.uv].uv = (-loop[m.uv].uv[0], loop[m.uv].uv[1])
m.joint("base")
m.done()
loose = kit.Part("loose", group="faults", about="a vertex weighted by no bone", armature=arm)
loose.sleeve([(0, 0, 0.9), (0, 0, 0.5)], [0.05, 0.05], role="skin", weights=[{{"thigh": 1}}, {{"thigh": 1}}])
loose.done().vertex_groups[0].remove([0])
for name in ("twin_a", "twin_b"):  # two atlases by one name: each places its piece in the same corner
    twin = kit.Part(name, group="faults", about="a piece over another's", atlas=kit.Atlas("shared", 1.0, 1.0))
    twin.card(0.3, 0.3, role="hide")
    twin.joint("base")
    twin.done()
far = kit.Part("far", group="faults", about="a piece pushed out of its atlas", atlas=kit.Atlas("small", 0.5, 0.5))
far.card(0.3, 0.3, role="hide")
for f in far.bm.faces:
    for loop in f.loops:
        loop[far.uv].uv = (loop[far.uv].uv[0] + 0.4, loop[far.uv].uv[1])
far.joint("base")
far.done()

kit.finish(sys.argv[-1])
with open(sys.argv[-1] + ".facts.json", "w") as f:
    json.dump(facts, f)
"""

TINY = """
import os, sys
sys.path.insert(0, {art!r})
import kit
kit.start()
p = kit.Part("rack_bar", group="wood", about="a bar")
p.sleeve([(0, 0, 0), (1.5, 0, 0.0)], [0.025, 0.025], sides=6, role="bark", cap_role="wood")
p.joint("end")
p.joint("other_end", at=(1.5, 0, 0), towards=(1, 0, 0))
p.done()
s = kit.Part("pebble", group="stones", about="a pebble")
s.hull([(0.05, 0, 0), (-0.05, 0, 0), (0, 0.04, 0), (0, -0.04, 0), (0, 0, 0.03), (0, 0, -0.03)], rounds=1)
s.rest()
s.project()
s.joint("base")
{fault}
s.done()
kit.finish(sys.argv[-1])
"""


def blender_run(*args):
    return subprocess.run([BLENDER, *args], capture_output=True, text=True)


@unittest.skipUnless(BLENDER, "Blender is not installed (set BLENDER to its path)")
class Parts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        d = cls.tmp.name
        script = os.path.join(d, "build.py")
        with open(script, "w") as f:
            f.write(SCRIPT.format(art=ART))
        cls.blend = os.path.join(d, "test.blend")
        done = blender_run("-b", "--factory-startup", "--python-exit-code", "1", "--python", script, "--", cls.blend)
        if done.returncode != 0:
            raise AssertionError(done.stdout[-3000:] + done.stderr[-3000:])
        with open(cls.blend + ".facts.json") as f:
            cls.facts = json.load(f)
        cls.report = os.path.join(d, "stretch.txt")
        check = os.path.join(d, "check.json")
        cls.check = blender_run(
            "-b",
            cls.blend,
            "--python",
            os.path.join(ART, "partcheck.py"),
            "--",
            "--report",
            cls.report,
            "--json",
            check,
        )
        with open(check) as f:
            cls.parts = {p["name"]: p for p in json.load(f)["parts"]}
        cls.layout_path = os.path.join(d, "layout.json")
        cls.layout = blender_run("-b", cls.blend, "--python", os.path.join(ART, "layout.py"), "--", cls.layout_path)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    # checks: PRE-27 PRE-46
    def test_the_layout_names_every_triangle_drawn_to_an_atlas(self):
        """layout.py writes each atlas's triangles with their places in it, so paint.py can draw to them."""
        self.assertEqual(self.layout.returncode, 0, self.layout.stdout[-2000:])
        with open(self.layout_path) as f:
            data = json.load(f)
        self.assertEqual(data["atlases"]["figure_skin"], [2.0, 2.0])
        skin = list(data["atlases"]).index("figure_skin")
        tris = [t for t in data["triangles"] if t[0] == skin]
        self.assertEqual({data["parts"][t[1]] for t in tris}, {"leg", "foot", "upper", "lower", "ankle"})
        self.assertNotIn("pole", {data["parts"][t[1]] for t in data["triangles"]}, "a part with no atlas has none")
        for t in tris:
            for u, v in zip(t[6:12:2], t[7:12:2], strict=True):
                self.assertTrue(0 <= u <= 2.0 and 0 <= v <= 2.0, "a triangle outside its atlas")
        self.assertEqual({data["roles"][t[2]] for t in tris}, {"skin"})

    # checks: PRE-46 PRE-22
    def test_parts_built_by_the_kit_keep_every_rule(self):
        for name in ("pole", "pole_cut", "basket", "hide", "tuft", "stone", "leg", "foot", "upper", "lower", "ankle"):
            p = self.parts[name]
            self.assertEqual(p["problems"], [], name)
            self.assertLessEqual(p["worst"], 1.5, name)
        self.assertEqual(self.parts["pole"]["slots"], ["bark", "wood"])
        self.assertEqual(self.parts["pole"]["joints"], ["base", "top"])
        self.assertEqual(self.parts["leg"]["keys"], ["build_stout"])
        self.assertEqual(self.parts["leg"]["armature"], "figure")
        self.assertEqual(self.parts["pole"]["size"], [0.1, 0.1, 2.0])

    # checks: PRE-46 PRE-22
    def test_a_poles_wrap_is_the_strip_nearest_its_true_circumference(self):
        """An octagon of radius r has a perimeter of 16 r sin(pi / 8); the rings' mean, 17.6 texture pixels, takes
        the 16-pixel strip of the wrap atlas, and the bark's texture lies in that strip and nowhere else."""
        import math

        mean = sum(16 * r * math.sin(math.pi / 8) for r in (0.05, 0.045, 0.04)) / 3
        self.assertEqual(self.facts["pole_wrap"], kitmath.wrap_width(mean))
        self.assertEqual(self.facts["pole_wrap"], 16)
        self.assertEqual(self.parts["pole"]["wrap_texels"], "16")
        left = kitmath.wrap_offset(16)
        self.assertAlmostEqual(min(self.facts["pole_u_bark"]), left, places=3)
        self.assertAlmostEqual(max(self.facts["pole_u_bark"]), left + 16, places=3)

    # checks: PRE-46
    def test_the_wrap_strips_fit_one_tile_side_by_side(self):
        edges = sorted((x, x + w) for w, x in kitmath.WRAPS)
        self.assertEqual(edges[0][0], 0)
        for (_, end), (start, _) in zip(edges, edges[1:], strict=False):
            self.assertLessEqual(end, start, "two strips overlap")
        self.assertLessEqual(edges[-1][1], 256)
        for w, x in kitmath.WRAPS[:-1]:
            self.assertEqual((x % 4, w % 4), (0, 0), f"the {w}-pixel strip is not whole at band 2")
        self.assertEqual(kitmath.wrap_width(47 / 64), 48)
        self.assertEqual(kitmath.wrap_width(5.1 / 64), 6)
        self.assertEqual(kitmath.wrap_width(200 / 64), 64)
        with self.assertRaises(ValueError):
            kitmath.wrap_offset(20)

    # checks: PRE-46 PRE-22
    def test_a_long_taper_wraps_in_stretches_and_pieces_weld_into_one(self):
        """A branch narrowing 3:1 would stretch 1.7:1 in one wrap; broken in three at its second and third rings,
        each stretch takes the strip nearest its own circumference (12, 8, then 6) and the mesh stays one, its
        texture within the strips. Two panels meeting along 0.6 m at 0.1 m spacing share the 7 or more points of
        that edge once welded, and no two points are left in one place."""
        self.assertEqual(self.facts["branch_wraps"], [12, 8, 6])
        self.assertGreaterEqual(self.facts["branch_u"][0], kitmath.wrap_offset(12) - 1e-3)
        self.assertLessEqual(self.facts["branch_u"][1], kitmath.wrap_offset(6) + 6 + 1e-3)
        self.assertEqual(self.parts["branch"]["problems"], [])
        self.assertLessEqual(self.parts["branch"]["worst"], 1.5)
        self.assertGreaterEqual(self.facts["weld_merged"], 7)
        self.assertEqual(self.facts["weld_together"], 0)
        self.assertEqual(self.parts["welded"]["problems"], [])

    # checks: PRE-46
    def test_a_lathes_first_disc_faces_down_and_its_last_up(self):
        self.assertEqual(self.facts["basket_caps"], [-1.0, 1.0])

    # checks: PRE-42 PRE-46
    def test_cut_wood_ends_chopped_or_broken_never_sawn(self):
        """A chopped end draws in to a point 1.2 radii beyond its ring (here from 6 cm up to the ground); a broken one
        is ragged; both lie flat on their facets, so neither stretches nor mirrors its texture."""
        self.assertAlmostEqual(self.facts["pole_cut_low"], 0.0, places=3)
        self.assertEqual(self.parts["pole_cut"]["slots"], ["bark", "wood"])
        self.assertGreater(self.parts["pole_cut"]["size"][2], 2.0)  # the broken top reaches past its last ring

    # checks: PRE-22 PRE-46
    def test_each_planted_fault_fails_with_its_own_message(self):
        wanted = {
            "squashed": "stretched 1.80:1 at rest",
            "painted": "slot 'brown paint' is not a role",
            "off": "no joint at its origin",
            "mirrored": "textures lie mirrored",
            "loose": "1 vertices weighted by no bone",
            "twin_a": "texture pixels of atlas shared lie under two faces",
            "twin_b": "texture pixels of atlas shared lie under two faces",
            "far": "2 triangles lie outside atlas small",
        }
        for name, words in wanted.items():
            problems = "; ".join(self.parts[name]["problems"])
            self.assertIn(words, problems, name)
        self.assertEqual(self.check.returncode, 1, "the check passed a file with failing parts")

    # checks: PRE-27 PRE-46
    def test_two_parts_of_one_chain_meet_exactly_at_rest_and_in_a_key(self):
        """Turned by the whole chain's frames, the ring the upper and lower halves share is one ring in both, so they
        never part; each half's own frames would have turned it differently (the chain bends there)."""
        self.assertLess(self.facts["shared_gap"], 1e-6)
        self.assertLess(self.facts["shared_gap_key"], 1e-6)
        self.assertGreater(self.facts["own_frame_turn"], 5.0)

    # checks: PRE-22 PRE-46
    def test_a_long_taper_finds_its_own_wraps(self):
        """A branch narrowing 4:1 breaks its wrap wherever one wrap's rings would differ by more than 1.3."""
        wraps = self.facts["auto_wraps"]
        self.assertGreaterEqual(len(wraps), 4, wraps)
        self.assertEqual(wraps, sorted(wraps, reverse=True))
        # 1.14 at most from the taper, and the step to the nearest strip on top
        self.assertLessEqual(self.parts["tapered"]["worst"], 1.4)

    # checks: PRE-22 PRE-27
    def test_where_a_sleeve_turns_too_sharply_charts_take_over(self):
        """A leg turning forward into a foot within a few centimetres stretches its sleeve's texture past the line;
        the faces found by measuring take charts, and none is left over the line."""
        self.assertGreater(self.facts["over_before"], 0)
        self.assertEqual(self.facts["charted"], self.facts["over_before"])
        self.assertEqual(self.facts["over_after"], 0)

    # checks: PRE-46 PRE-27
    def test_a_figures_pieces_share_its_atlas_without_overlap(self):
        """The leg and the foot are drawn to one atlas: each piece placed apart, so the check finds no texture pixel
        under two faces, and the leg's piece starts inside the atlas's margin."""
        self.assertEqual(self.facts["leg_uv"], [0.03])
        for name in ("leg", "foot"):
            self.assertFalse([p for p in self.parts[name]["problems"] if "atlas" in p], name)

    # checks: PRE-46
    def test_the_report_leads_with_the_worst_figure(self):
        with open(self.report) as f:
            text = f.read()
        self.assertIn("the worst triangle is stretched 1.80:1 (the line is 1.5:1, A6.4)", text)
        self.assertIn("8 parts fail a check", text)  # the eight planted faults
        self.assertRegex(text, r"\npole +0\.10 x 0\.10 x 2\.00 ")


@unittest.skipUnless(BLENDER and XVFB, "Blender or xvfb-run is not installed")
class Families(unittest.TestCase):
    def family(self, fault=""):
        d = tempfile.TemporaryDirectory()
        self.addCleanup(d.cleanup)
        folder = os.path.join(d.name, "art", "models")
        os.makedirs(folder)
        with open(os.path.join(folder, "tiny.py"), "w") as f:
            f.write(TINY.format(art=ART, fault=fault))
        done = subprocess.run(
            [sys.executable, os.path.join(ART, "models.py"), "tiny", "--root", d.name], capture_output=True, text=True
        )
        return done, folder

    # checks: PRE-46
    def test_a_family_is_built_checked_and_put_on_its_sheet(self):
        done, folder = self.family()
        self.assertEqual(done.returncode, 0, done.stdout + done.stderr)
        for name in ("tiny.blend", "tiny-stretch.txt", "tiny-sheet.webp"):
            self.assertTrue(os.path.exists(os.path.join(folder, name)), name)
        self.assertFalse(any(f.endswith(".blend1") for f in os.listdir(folder)))
        from PIL import Image

        with Image.open(os.path.join(folder, "tiny-sheet.webp")) as im:
            self.assertEqual(im.width, 1080)

    # checks: PRE-46
    def test_a_family_with_a_failing_part_fails(self):
        done, folder = self.family(fault="next(iter(s.bm.verts)).co.z += 0.3")
        self.assertEqual(done.returncode, 1, done.stdout + done.stderr)
        with open(os.path.join(folder, "tiny-stretch.txt")) as f:
            self.assertIn("1 parts fail a check", f.read())


@unittest.skipUnless(BLENDER, "Blender is not installed (set BLENDER to its path)")
class Committed(unittest.TestCase):
    # checks: PRE-46 PRE-22
    def test_every_committed_family_passes_its_check(self):
        folder = os.path.join(ROOT, "art", "models")
        blends = sorted(f for f in os.listdir(folder) if f.endswith(".blend")) if os.path.isdir(folder) else []
        if not blends:
            self.skipTest("no family committed yet")
        for name in blends:
            done = blender_run("-b", os.path.join(folder, name), "--python", os.path.join(ART, "partcheck.py"))
            self.assertEqual(done.returncode, 0, f"{name}: " + done.stdout[-2000:])


if __name__ == "__main__":
    unittest.main()
