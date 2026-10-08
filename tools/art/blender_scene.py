"""Blender's side of the previews (A5.3, A8.4): one picture of a ground and the parts standing on it, drawn the way
the game draws it up close: the camera 37 degrees down through a narrow lens, a texture pixel about 2 x 2 screen pixels,
low warm sun and a cool sky, no tone curve and nothing else laid on. Run inside Blender, never by hand:

    blender -b --factory-startup --python tools/art/blender_scene.py -- <config.json>

The config (written by preview.py) names the picture to write and its size, the camera (distance, tilt and lens), the
sun and sky, the ground (a picture with the metres it covers, read nearest-pixel), and, for parts, the .blend files to
bring in with where each stands. Implements PRE-20, PRE-22 and PRE-46, see A4.2 and A8.4.
"""

import json
import math
import random
import re
import sys

import bpy
from mathutils import Matrix, Vector


def argument():
    return json.load(open(sys.argv[sys.argv.index("--") + 1]))


def textured_quad(name, corners, picture):
    """A flat quad with a picture read nearest-pixel over it, its four corners (counter-clockwise seen from the front:
    bottom left, bottom right, top right, top left) taking the picture's corners; lit like the rest."""
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(corners, [], [(0, 1, 2, 3)])
    uv = mesh.uv_layers.new(name="uv")
    for loop, (u, v) in zip(mesh.loops, [(0, 0), (1, 0), (1, 1), (0, 1)], strict=True):
        uv.data[loop.index].uv = (u, v)
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    nodes.clear()
    tex = nodes.new("ShaderNodeTexImage")
    tex.image = bpy.data.images.load(picture)
    tex.image.colorspace_settings.name = "sRGB"
    tex.interpolation = "Closest"
    tex.extension = "EXTEND"
    bsdf = nodes.new("ShaderNodeBsdfDiffuse")
    out = nodes.new("ShaderNodeOutputMaterial")
    links.new(tex.outputs["Color"], bsdf.inputs["Color"])
    links.new(bsdf.outputs["BSDF"], out.inputs["Surface"])
    mesh.materials.append(mat)


def ground(cfg):
    """The ground: a flat quad over the metres the picture covers, its pixels read nearest."""
    g = cfg["ground"]
    x0, x1, y0, y1 = g["x0"], g["x1"], g["y0"], g["y1"]
    textured_quad("ground", [(x0, y0, 0), (x1, y0, 0), (x1, y1, 0), (x0, y1, 0)], g["picture"])


def wall(cfg):
    """Walls standing across the ground's middle line, facing the camera (south): each a vertical quad over the metres
    its picture covers, `x0` to `x1` east and `z0` to `z1` up, its pixels read nearest, at `y` metres north of the line
    (0 if not said). The config's `wall` is one wall, its `walls` any number (a cliff's layers, each set back as the
    weathering has left it)."""
    for k, w in enumerate([cfg["wall"]] if "wall" in cfg else cfg.get("walls", [])):
        x0, x1, z0, z1, y = w["x0"], w["x1"], w["z0"], w["z1"], w.get("y", 0.0)
        textured_quad(f"wall{k}", [(x0, y, z0), (x1, y, z0), (x1, y, z1), (x0, y, z1)], w["picture"])


def camera(cfg):
    """The camera as the rig sets it (view/src/rig.cpp): from `distance` metres, `tilt` degrees below the horizon,
    north, through a lens `lens` degrees across the picture's width, looking at the focus."""
    c = cfg["camera"]
    tilt = math.radians(c["tilt"])
    cam = bpy.data.cameras.new("camera")
    cam.sensor_fit = "HORIZONTAL"
    cam.angle = math.radians(c["lens"])
    cam.clip_start = c["distance"] * 0.2
    cam.clip_end = c["distance"] * 20
    obj = bpy.data.objects.new("camera", cam)
    bpy.context.scene.collection.objects.link(obj)
    d = c["distance"]
    obj.location = (c.get("x", 0.0), c.get("y", 0.0) - d * math.cos(tilt), d * math.sin(tilt))
    obj.rotation_euler = (math.radians(90.0) - tilt, 0.0, 0.0)
    bpy.context.scene.camera = obj


def light(cfg):
    """A low warm sun from a compass bearing and height, and a uniform cool sky."""
    s = cfg["sun"]
    az, el = math.radians(s["azimuth"]), math.radians(s["elevation"])
    toward = Vector((math.sin(az) * math.cos(el), math.cos(az) * math.cos(el), math.sin(el)))
    lamp = bpy.data.lights.new("sun", "SUN")
    lamp.color = s["colour"]
    lamp.energy = s["strength"]
    lamp.angle = math.radians(s.get("size", 0.6))
    obj = bpy.data.objects.new("sun", lamp)
    obj.rotation_euler = (-toward).to_track_quat("-Z", "Y").to_euler()
    bpy.context.scene.collection.objects.link(obj)
    world = bpy.data.worlds.new("sky")
    world.use_nodes = True
    bg = world.node_tree.nodes["Background"]
    bg.inputs["Color"].default_value = (*cfg["sky"]["colour"], 1.0)
    bg.inputs["Strength"].default_value = cfg["sky"]["strength"]
    bpy.context.scene.world = world


def role_material(role, picture):
    """The material a role wears in the preview: its picture read nearest-pixel, laid by the part's texture coordinates
    in metres (the picture's `metres` across), multiplied by the part's baked crease as the engine does (1 open, 0
    dark), lit like the ground."""
    mat = bpy.data.materials.new(role)
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    nodes.clear()
    coords = nodes.new("ShaderNodeTexCoord")
    mapping = nodes.new("ShaderNodeMapping")
    mapping.inputs["Scale"].default_value = (1.0 / picture["metres"], 1.0 / picture["metres"], 1.0)
    tex = nodes.new("ShaderNodeTexImage")
    tex.image = bpy.data.images.load(picture["picture"])
    tex.image.colorspace_settings.name = "sRGB"
    tex.interpolation = "Closest"
    tex.extension = "REPEAT"
    crease = nodes.new("ShaderNodeAttribute")
    crease.attribute_name = "crease"
    mix = nodes.new("ShaderNodeMix")
    mix.data_type = "RGBA"
    mix.blend_type = "MULTIPLY"
    mix.inputs["Factor"].default_value = 1.0
    bsdf = nodes.new("ShaderNodeBsdfDiffuse")
    out = nodes.new("ShaderNodeOutputMaterial")
    links.new(coords.outputs["UV"], mapping.inputs["Vector"])
    links.new(mapping.outputs["Vector"], tex.inputs["Vector"])
    links.new(tex.outputs["Color"], mix.inputs["A"])
    links.new(crease.outputs["Color"], mix.inputs["B"])
    links.new(mix.outputs["Result"], bsdf.inputs["Color"])
    links.new(bsdf.outputs["BSDF"], out.inputs["Surface"])
    return mat


def parts(cfg):
    """Parts brought in from .blend files: every object of each, turned about z by `turn` degrees and put where the
    config says (`at`, in metres east, north and up of the focus), each material slot named by its role wearing the
    role's picture (config `roles`: {role: {picture, metres}}), the first UV map being in metres."""
    roles = {r: role_material(r, p) for r, p in cfg.get("roles", {}).items()}
    for item in cfg.get("parts", []):
        with bpy.data.libraries.load(item["file"], link=False) as (src, dst):
            dst.objects = [n for n in src.objects if not item.get("only") or n in item["only"]]
        for o in dst.objects:
            if o is None:
                continue
            bpy.context.scene.collection.objects.link(o)
            if not item.get("keep"):  # what assemble() takes its parts from, and hides; a `keep` item is seen as put
                o["kd_part"] = True
            if o.type == "MESH":
                for slot in o.material_slots:
                    name = slot.material.name.split(".")[0] if slot.material else ""
                    if name in roles:
                        slot.material = roles[name]
            if o.parent is None:
                turn = math.radians(item.get("turn", 0.0))
                o.rotation_euler = (o.rotation_euler[0], o.rotation_euler[1], o.rotation_euler[2] + turn)
                # where it stands, not where the file lays it in its row
                o.location = Vector(item.get("at", (0, 0, 0)))
                if item.get("centre") and o.type == "MESH":  # its middle, seen from above, over that place
                    bpy.context.view_layer.update()
                    corners = [o.matrix_world @ Vector(c) for c in o.bound_box]
                    middle = sum(corners, Vector()) / 8.0
                    across = 0.0 if item["centre"] == "x" else middle.y - o.location.y  # "x": only along the way east
                    o.location = Vector(o.location) - Vector((middle.x - o.location.x, across, 0.0))


def joint_frame(part, name):
    """A joint of a part in the part's own metres (parts() leaves the part at the origin, unturned): its matrix, or the
    part's own origin and axes for no joint."""
    if not name:
        return Matrix.Identity(4)
    for child in part.children:
        if child.type == "EMPTY" and re.sub(r"\.\d{3}$", "", child.name) == "joint_" + name:
            return child.matrix_world.copy()
    raise ValueError(f"the part {part.name} has no joint {name}")


def squared(matrix):
    """The three axes of a frame made square again: its first and second kept, the third across them."""
    m = matrix.to_3x3()
    x = m.col[0].normalized()
    z = x.cross(m.col[1]).normalized()
    return Matrix((x, z.cross(x), z)).transposed()


def carrying(linear, origin, place):
    """The matrix that takes a part's metres to the thing's: the part's point `origin` goes to `place`, the part turned
    (and stretched) by `linear` about it."""
    return Matrix.Translation(place) @ linear.to_4x4() @ Matrix.Translation(-origin)


def assemble(cfg):
    """A recipe's places put together as the engine's assembler does (view/src/kit_assemble.cpp, A6.1) in Blender's
    axes (x east, y north, z up). The strays and the choices between a place's parts come from this preview's own dice,
    since the engine's cannot be asked, so the picture is a thing the recipe makes, not the seed's own. The parts,
    brought in by parts() at the origin, are hidden and every placed copy shares their meshes and so their role
    materials."""
    dice = random.Random(cfg["assemble"].get("seed", 1))

    def either():
        return dice.uniform(-1.0, 1.0)

    bpy.context.view_layer.update()
    originals = {o.name: o for o in bpy.data.objects if o.type == "MESH" and o.parent is None and o.get("kd_part")}
    about = Matrix.Rotation(math.radians(cfg["assemble"].get("turn", 0.0)), 4, "Z")  # the whole thing turned
    placed = {}
    for place in cfg["assemble"]["places"]:
        rule, name = place["rule"], place["name"]
        turn = math.radians(place.get("turn", 0.0))
        face = place.get("face", 0.0)
        if rule == "plug":
            onto, _, target_joint = place["onto"].partition(".")
            count = len(placed[onto])
        else:
            count = place.get("count", 1) if rule != "root" else 1
        copies = []
        for k in range(count):
            part = originals[dice.choice(place["parts"])]
            own = joint_frame(part, place.get("joint", ""))
            at = own.translation
            lift = place.get("height", 0.0)
            if rule == "root":
                linear = Matrix.Rotation(-turn, 3, "Z") @ Matrix.Rotation(math.radians(place.get("tilt", 0.0)), 3, "X")
                matrix = carrying(linear, at, Vector((0.0, 0.0, lift)))
            elif rule == "plug":
                frame = placed[onto][k]["matrix"] @ joint_frame(placed[onto][k]["part"], target_joint)
                axes = squared(frame)
                linear = axes @ Matrix.Rotation(-turn, 3, "Z") @ squared(own).transposed()
                matrix = carrying(linear, at, frame.translation + axes.col[2] * lift)
            else:
                azimuth = math.radians(
                    place.get("turn", 0.0) + 360.0 * k / count + place.get("jitter_turn", 0.0) * either()
                )
                r = place["radius"] * (1.0 + place.get("jitter_radius", 0.0) * either())
                on_circle = Vector((r * math.sin(azimuth), r * math.cos(azimuth), place.get("base", 0.0)))
                if rule == "ring":
                    linear = Matrix.Rotation(-(azimuth + math.radians(face)), 3, "Z")
                else:
                    along = joint_frame(part, place["to_joint"]).translation - at
                    span = Vector((0.0, 0.0, lift)) - on_circle
                    thick = 1.0 + place.get("jitter_size", 0.0) * either()
                    axis, heading = along.normalized(), span.normalized()
                    stretch = span.length / along.length
                    shape = Matrix(
                        [
                            [(thick if i == j else 0.0) + (stretch - thick) * axis[i] * axis[j] for j in range(3)]
                            for i in range(3)
                        ]
                    )
                    roll = Matrix.Rotation(math.radians(face + place.get("jitter_turn", 0.0) * either()), 3, heading)
                    linear = roll @ axis.rotation_difference(heading).to_matrix() @ shape
                matrix = carrying(linear, at, on_circle)
            copy = bpy.data.objects.new(f"{name}.{k}", part.data)
            copy.matrix_world = about @ matrix
            bpy.context.scene.collection.objects.link(copy)
            copies.append({"matrix": matrix, "part": part})
        placed[name] = copies
    for o in originals.values():
        o.hide_render = True


def main():
    cfg = argument()
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = cfg.get("samples", 24)
    scene.cycles.use_denoising = False
    scene.cycles.filter_width = 1.0
    scene.render.resolution_x, scene.render.resolution_y = cfg["size"]
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_depth = "8"
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "None"
    scene.display_settings.display_device = "sRGB"
    if "ground" in cfg:
        ground(cfg)
    if "wall" in cfg or "walls" in cfg:
        wall(cfg)
    parts(cfg)
    if "assemble" in cfg:
        assemble(cfg)
    camera(cfg)
    light(cfg)
    scene.render.filepath = cfg["out"]
    bpy.ops.render.render(write_still=True)


main()
