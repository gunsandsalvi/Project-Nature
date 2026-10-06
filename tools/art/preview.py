"""The kit's preview pictures (IMPLEMENTATION.md, the art lane): every part of a Blender file drawn at true size,
wearing a checker of 64 texture pixels a metre that shows any stretch, for the preview sheet (partsheet.py).

    xvfb-run -a blender -b <file.blend> --python tools/art/preview.py -- <out folder> [--scale 128]
                                                                          [--textures <textures.json>]

Run inside Blender 4.0; its Workbench renderer needs a display here, which xvfb-run gives. Each part is drawn alone
from the game camera's height (40 degrees down), from in front and 30 degrees round to its left (+x), orthographic
at `scale` picture pixels a metre: at 128 a texture pixel shows as 2 x 2, as on the phone at the closest zoom. Parts
under 0.4 m are drawn again four times as large. A figure (an armature's skinned parts) is drawn whole at rest,
without its garments (parts whose kit_layer is "garment"), with each of its choices (kit_layer "choice", worn one at
a time, as antlers by age), with each shape key at 1, and in each bend pose the file keeps. The checker
(kitmath.checker) is tinted by each slot's role, so roles show apart; its red and green lines mark each metre of u
and v. With --textures, each part wears its textures instead (textured()): a figure's its atlas's, drawn to its
layout, and any other slot the material the JSON chooses for its role, as a recipe would.
Writes <out folder>/<view>.png with clear backgrounds, and views.json naming each view and its parts.

Implements PRE-46 and PRE-22, see A6.4 and A6.5.
"""

import json
import math
import os
import sys

import bpy
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import kit  # noqa: E402
import kitmath  # noqa: E402
import partcheck  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TILT = 40.0  # degrees below level, the game camera's
TURN = 30.0  # degrees round from the part's front (-y) toward its left side (+x)
MARGIN = 6  # picture pixels round each view
SMALL = 0.4  # metres: parts smaller than this are drawn again, enlarged
ENLARGE = 4


def checker_material(role_name):
    """A material showing the checker tinted by a role's colour, with each texture pixel kept square and sharp."""
    name = f"preview_{role_name}"
    m = bpy.data.materials.get(name)
    if m is not None:
        return m
    tint = kit.ROLES.get(role_name, (0.7, 0.7, 0.7))
    top = max(tint)
    rows = kitmath.checker()
    size = len(rows)
    img = bpy.data.images.new(name, size, size, alpha=False)
    flat = []
    for row in reversed(rows):  # Blender's pixels run from the bottom row up
        for c in row:
            flat += [min(1.0, c[i] * (0.6 + 0.4 * tint[i] / top)) for i in range(3)] + [1.0]
    img.pixels = flat
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    nt = m.node_tree
    tex = nt.nodes.new("ShaderNodeTexImage")
    tex.image = img
    tex.interpolation = "Closest"
    tex.extension = "REPEAT"
    uv = nt.nodes.new("ShaderNodeUVMap")
    uv.uv_map = kit.UV_NAME
    nt.links.new(uv.outputs["UV"], tex.inputs["Vector"])
    bsdf = nt.nodes.get("Principled BSDF")
    nt.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
    nt.nodes.active = tex
    return m


def dress(objects):
    """Every slot of the objects given the checker of its role."""
    for o in objects:
        for slot in o.material_slots:
            r = slot.material.name if slot.material else "stone"
            slot.material = checker_material(r)


def texture_material(path):
    """A material showing a texture's band 0, each texture pixel square and sharp, the texture repeating."""
    name = f"textured_{os.path.relpath(path, ROOT)}"
    m = bpy.data.materials.get(name)
    if m is not None:
        return m
    img = bpy.data.images.load(path)
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    nt = m.node_tree
    tex = nt.nodes.new("ShaderNodeTexImage")
    tex.image = img
    tex.interpolation = "Closest"
    tex.extension = "REPEAT"
    bsdf = nt.nodes.get("Principled BSDF")
    nt.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
    nt.nodes.active = tex
    return m


def flat_material(role_name):
    """A material of a role's own flat colour, for a role with no texture yet."""
    name = f"flat_{role_name}"
    m = bpy.data.materials.get(name)
    if m is None:
        m = bpy.data.materials.new(name)
        m.diffuse_color = (*kit.ROLES.get(role_name, (0.7, 0.7, 0.7)), 1.0)
    return m


def textured(objects, chosen):
    """Every slot of the objects given its texture: a part drawn to an atlas its atlas's own (art/textures/<atlas>),
    any other slot the material `chosen` gives its role or its part ({"roles": {role: folder}, "parts": {name prefix:
    {role: folder}}}), and a role with none its flat colour. Each part's texture coordinates, in metres, are scaled to
    its texture's side for Workbench, which reads an image texture straight from the active UV map."""
    for o in objects:
        atlas = o.get("kit_atlas")
        side = None
        for slot in o.material_slots:
            r = slot.material.name if slot.material else "stone"
            folder = None
            if atlas is not None and os.path.exists(os.path.join(ROOT, "art", "textures", str(atlas), "b0.png")):
                folder = os.path.join("art", "textures", str(atlas))
            else:
                for prefix, roles in chosen.get("parts", {}).items():
                    if o.name.startswith(prefix) and r in roles:
                        folder = roles[r]
                folder = folder or chosen.get("roles", {}).get(r)
            path = os.path.join(ROOT, folder, "b0.png") if folder else None
            if path and os.path.exists(path):
                slot.material = texture_material(path)
                side = side or slot.material.node_tree.nodes.active.image.size[0]
            else:
                slot.material = flat_material(r)
        side = side or 256
        me = o.data
        layer = me.uv_layers.get("preview_uv") or me.uv_layers.new(name="preview_uv")
        source = me.uv_layers[kit.UV_NAME].data
        k = kitmath.TEXELS_A_METRE / side
        for i, d in enumerate(layer.data):
            d.uv = (source[i].uv.x * k, source[i].uv.y * k)
        me.uv_layers.active = layer
        layer.active_render = True


def setup(scene):
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "TEXTURE"
    scene.display.shading.show_cavity = False
    scene.display.shading.show_object_outline = False
    scene.display.render_aa = "OFF"
    scene.render.film_transparent = True
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.view_settings.view_transform = "Standard"
    scene.render.resolution_percentage = 100
    cam_data = bpy.data.cameras.new("preview")
    cam_data.type = "ORTHO"
    cam = bpy.data.objects.new("preview", cam_data)
    scene.collection.objects.link(cam)
    scene.camera = cam
    return cam


def view_axes():
    """The camera's look direction, its right and its up, in the file's space."""
    tilt, turn = math.radians(TILT), math.radians(TURN)
    look = Vector((-math.sin(turn) * math.cos(tilt), math.cos(turn) * math.cos(tilt), -math.sin(tilt)))
    right = look.cross(Vector((0, 0, 1))).normalized()
    up = right.cross(look).normalized()
    return look, right, up


def corners(objects):
    """The world corners of the objects' evaluated shapes (with their shape keys and poses)."""
    dg = bpy.context.evaluated_depsgraph_get()
    out = []
    for o in objects:
        ev = o.evaluated_get(dg)
        m = ev.to_mesh()
        out += [ev.matrix_world @ v.co for v in m.vertices]
        ev.to_mesh_clear()
    return out


def shoot(scene, cam, objects, path, scale):
    """One view of the objects alone at `scale` picture pixels a metre; returns its size in pixels."""
    shown = set(objects)
    for o in scene.objects:
        if o.type in ("MESH", "EMPTY", "ARMATURE"):
            o.hide_render = o not in shown
    look, right, up = view_axes()
    cs = corners(objects)
    xs = [c.dot(right) for c in cs]
    ys = [c.dot(up) for c in cs]
    w = max(xs) - min(xs)
    h = max(ys) - min(ys)
    rx = int(math.ceil(w * scale)) + 2 * MARGIN
    ry = int(math.ceil(h * scale)) + 2 * MARGIN
    rx += rx % 2
    ry += ry % 2
    scene.render.resolution_x = rx
    scene.render.resolution_y = ry
    cam.data.ortho_scale = max(rx, ry) / scale
    mid = right * ((max(xs) + min(xs)) / 2) + up * ((max(ys) + min(ys)) / 2)
    depth = sum(c.dot(look) for c in cs) / len(cs)
    cam.location = mid + look * (depth - 50.0)
    cam.rotation_euler = look.to_track_quat("-Z", "Y").to_euler()
    cam.data.clip_end = 200.0
    scene.render.filepath = path
    bpy.ops.render.render(write_still=True)
    return rx, ry


def main(argv):
    args = argv[argv.index("--") + 1 :]
    out = args[0]
    scale = float(args[args.index("--scale") + 1]) if "--scale" in args else 128.0
    os.makedirs(out, exist_ok=True)
    scene = bpy.context.scene
    cam = setup(scene)
    parts = partcheck.parts()
    if "--textures" in args:
        with open(args[args.index("--textures") + 1]) as f:
            textured(parts, json.load(f))
    else:
        dress(parts)
    views = []
    loose = [p for p in parts if partcheck.armature_of(p) is None]
    for p in loose:
        name = p.name
        size = shoot(scene, cam, [p], os.path.join(out, f"{name}.png"), scale)
        views.append({"view": name, "parts": [name], "kind": "part", "scale": scale, "pixels": size})
        if max(p.dimensions) < SMALL:
            size = shoot(scene, cam, [p], os.path.join(out, f"{name}-x{ENLARGE}.png"), scale * ENLARGE)
            views.append(
                {
                    "view": f"{name}-x{ENLARGE}",
                    "parts": [name],
                    "kind": "enlarged",
                    "scale": scale * ENLARGE,
                    "pixels": size,
                }
            )
    for arm in [o for o in scene.objects if o.type == "ARMATURE"]:
        body = [p for p in parts if partcheck.armature_of(p) is arm]
        if not body:
            continue
        worn = [p for p in body if p.get("kit_layer", "body") != "choice"]
        bare = [p for p in worn if p.get("kit_layer", "body") != "garment"]
        choices = [p for p in body if p.get("kit_layer") == "choice"]  # parts worn one at a time, as antlers by age
        keys = sorted({kb.name for p in body if p.data.shape_keys for kb in p.data.shape_keys.key_blocks[1:]})
        shots = [("rest", worn, None, None), ("body", bare, None, None)]
        shots += [(f"with {c.name}", worn + [c], None, None) for c in choices]
        shots += [(f"key {k}", worn, k, None) for k in keys]
        if arm.animation_data is None:
            arm.animation_data_create()
        bent = worn + choices[-1:]
        shots += [(a.name, bent, None, a) for a in bpy.data.actions if a.name.startswith("bend")]
        for title, obs, key, act in shots:
            for p in body:
                if p.data.shape_keys:
                    for kb in p.data.shape_keys.key_blocks[1:]:
                        kb.value = 1.0 if kb.name == key else 0.0
            arm.animation_data.action = act
            scene.frame_set(1)
            view = f"{arm.name}-{title.replace(' ', '-')}"
            size = shoot(scene, cam, obs, os.path.join(out, f"{view}.png"), scale)
            views.append(
                {
                    "view": view,
                    "parts": [o.name for o in obs],
                    "kind": "figure",
                    "title": title,
                    "scale": scale,
                    "pixels": size,
                    "figure": arm.name,
                }
            )
        arm.animation_data.action = None
    with open(os.path.join(out, "views.json"), "w") as f:
        json.dump({"file": os.path.basename(bpy.data.filepath), "views": views}, f, indent=1)
    print(f"{len(views)} views in {out}")
    return 0


if __name__ == "__main__":
    code = main(sys.argv)
    sys.stdout.flush()
    os._exit(code)
