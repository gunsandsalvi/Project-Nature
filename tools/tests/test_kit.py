"""tools/kit.py and the Blender scripts export the model kit's parts as the engine reads them (PRE-46, A6.1, A6.4): a
part's triangles by role, its texture coordinates in metres, its joints and its crease, in Godot's axes; the stand-in
family the build makes; and the kit's own check, kd_kit, which holds each part's texture pixels to 1.5:1.

These need Blender (tools/setup.sh) and, for what the engine reads, the extension's build folder (kd_kit)."""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
TOOLS = os.path.abspath(os.path.join(HERE, ".."))
sys.path.insert(0, TOOLS)
import kit  # noqa: E402

ROOT = os.path.dirname(TOOLS)
KD_KIT = os.environ.get("KD_KIT", os.path.join(ROOT, "build", "view", "kd_kit"))
BLENDER = shutil.which("blender")

# A scene as an artist's file holds it, made in Blender and exported as any family is: a plank 2 m by 1 m of two roles
# set far from the origin and turned, with a joint at one end, a crease painted on its corners, and a helper object that
# is no part.
PLANK = """
import sys
sys.path.insert(0, {tools!r})
import bpy, bmesh, math
from mathutils import Vector
import export

for o in list(bpy.data.objects):
    bpy.data.objects.remove(o)
bm = bmesh.new()
uv = bm.loops.layers.uv.new("UVMap")
crease = bm.loops.layers.float_color.new("crease")
a = [bm.verts.new(Vector(p)) for p in ((0, 0, 0), (2, 0, 0), (2, 1, 0), (0, 1, 0))]
b = [bm.verts.new(Vector(p)) for p in ((0, 0, 1), (2, 0, 1), (2, 1, 1), (0, 1, 1))]
for verts, role, (u0, v0) in ((a, 0, (0.0, 0.0)), (b, 1, (5.0, 5.0))):
    f = bm.faces.new(verts)
    f.material_index = role
    for loop, (u, v) in zip(f.loops, ((0, 0), (2, 0), (2, 1), (0, 1))):
        loop[uv].uv = (u0 + u, v0 + v)
        loop[crease] = (0.5, 0.5, 0.5, 1.0)
mesh = bpy.data.meshes.new("plank")
bm.to_mesh(mesh)
obj = bpy.data.objects.new("plank", mesh)
obj.location = (10.0, 20.0, 30.0)
bpy.context.scene.collection.objects.link(obj)
for name in ("wood", "hide.001"):
    obj.data.materials.append(bpy.data.materials.new(name))
end = bpy.data.objects.new("joint_end", None)
end.parent = obj
end.location = (1.0, 2.0, 3.0)
bpy.context.scene.collection.objects.link(end)
helper = bpy.data.objects.new("_scratch", bpy.data.meshes.new("_scratch"))
bpy.context.scene.collection.objects.link(helper)
export.export({out!r})
"""


def blender_scene(script, out):
    """Runs a script in Blender that exports to out; the exporter's last words."""
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "scene.py")
        with open(path, "w") as f:
            f.write(script.format(tools=os.path.join(TOOLS, "blender"), out=out))
        run = subprocess.run(
            ["blender", "--background", "--factory-startup", "--python-exit-code", "1", "--python", path],
            capture_output=True,
            text=True,
        )
    return run


def info(path):
    """What the engine's reader makes of a kit file, as kd_kit says it."""
    run = subprocess.run([KD_KIT, "info", path], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr
    return run.stdout


@unittest.skipUnless(BLENDER, "Blender is not installed: tools/setup.sh installs it")
class Export(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.dir)

    # checks: PRE-46
    @unittest.skipUnless(os.path.isfile(KD_KIT), "the extension's build folder has no kd_kit")
    def test_a_part_comes_out_by_role_with_its_joints_in_godots_axes_and_its_place_taken_off(self):
        out = os.path.join(self.dir, "plank.kdkit")
        run = blender_scene(PLANK, out)
        self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
        said = info(out)
        lines = said.splitlines()
        # one part, the helper left out, its two roles by their slots' names, a Blender suffix dropped
        self.assertEqual([line for line in lines if line.startswith("part ")], [lines[0]])
        self.assertTrue(lines[0].startswith("part plank 4 triangles"), lines[0])
        self.assertIn("  role hide 4 vertices 2 triangles", lines)
        self.assertIn("  role wood 4 vertices 2 triangles", lines)
        # its place (10, 20, 30) taken off: Blender's (x, y, z) are Godot's (x, z, -y)
        self.assertIn("bounds 0.000 0.000 -1.000 to 2.000 1.000 0.000", lines[0])
        # a joint at Blender (1, 2, 3) from the part's origin is at Godot (1, 3, -2)
        self.assertIn("  joint end at 1.000 3.000 -2.000", lines)

    # checks: PRE-46
    def test_a_part_with_no_uv_map_a_bad_name_or_no_role_is_refused_naming_it(self):
        bad = {
            "no UV map": ("plank", "obj.data.uv_layers.remove(obj.data.uv_layers[0])", "no UV map"),
            "a bad name": ("Plank", "pass", "not named in lower case"),
            "no role": ("plank", "obj.data.materials.clear()", "no material slot"),
        }
        for why, (name, change, expected) in bad.items():
            script = PLANK.replace(
                'bpy.data.objects.new("plank", mesh)', f'bpy.data.objects.new("{name}", mesh)'
            ).replace("export.export(", f"{change}\nexport.export(")
            run = blender_scene(script, os.path.join(self.dir, "bad.kdkit"))
            self.assertNotEqual(run.returncode, 0, why)
            self.assertIn(expected, run.stdout + run.stderr, why)

    # checks: PRE-46
    def test_the_stand_in_family_comes_out_the_same_every_time_and_rewrites_nothing_that_did_not_change(self):
        first = os.path.join(self.dir, "first")
        second = os.path.join(self.dir, "second")
        said = kit.build(first, models=os.path.join(self.dir, "none"))
        self.assertEqual(list(said), ["standin_camp"])
        kit.build(second, models=os.path.join(self.dir, "none"))
        with (
            open(os.path.join(first, "standin_camp.kdkit"), "rb") as a,
            open(os.path.join(second, "standin_camp.kdkit"), "rb") as b,
        ):
            self.assertEqual(a.read(), b.read())
        # a second build into the same folder leaves the file as it was, and takes away a stray one
        path = os.path.join(first, "standin_camp.kdkit")
        before = os.stat(path).st_mtime_ns
        with open(os.path.join(first, "gone.kdkit"), "wb") as f:
            f.write(b"x")
        kit.build(first, models=os.path.join(self.dir, "none"))
        self.assertEqual(os.stat(path).st_mtime_ns, before)
        self.assertEqual(sorted(os.listdir(first)), ["standin_camp.kdkit"])

    # checks: PRE-46 PRE-22
    @unittest.skipUnless(os.path.isfile(KD_KIT), "the extension's build folder has no kd_kit")
    def test_the_stand_in_parts_have_their_joints_and_roles_and_pass_the_stretch_check(self):
        out = os.path.join(self.dir, "kit")
        kit.build(out, models=os.path.join(self.dir, "none"))
        said = info(os.path.join(out, "standin_camp.kdkit"))
        for expected in ("part club", "part pole", "part cover", "part door_flap", "part stone_a", "part binding"):
            self.assertIn(expected, said)
        for expected in (
            "joint foot",
            "joint bind",
            "joint tip",
            "joint apex",
            "joint door_top",
            "joint hinge",
            "joint centre",
            "role stone",
            "role hide",
            "role wood",
        ):
            self.assertIn(expected, said)
        # an empty data folder has no recipes: only the parts' stretch is held to its line
        data = os.path.join(self.dir, "data")
        os.makedirs(os.path.join(data, "base"))
        with open(os.path.join(data, "base", "source.toml"), "w") as f:
            f.write('id = "base"\nversion = 1\nabout = "none"\n')
        run = subprocess.run([KD_KIT, "check", data, out], capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
        self.assertIn("worst stretch", run.stdout)


if __name__ == "__main__":
    unittest.main()
