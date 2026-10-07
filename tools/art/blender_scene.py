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
import sys

import bpy
from mathutils import Vector


def argument():
    return json.load(open(sys.argv[sys.argv.index("--") + 1]))


def ground(cfg):
    """The ground: a flat quad over the metres the picture covers, its pixels read nearest."""
    g = cfg["ground"]
    x0, x1, y0, y1 = g["x0"], g["x1"], g["y0"], g["y1"]
    mesh = bpy.data.meshes.new("ground")
    mesh.from_pydata([(x0, y0, 0), (x1, y0, 0), (x1, y1, 0), (x0, y1, 0)], [], [(0, 1, 2, 3)])
    uv = mesh.uv_layers.new(name="uv")
    for loop, (u, v) in zip(mesh.loops, [(0, 0), (1, 0), (1, 1), (0, 1)], strict=True):
        uv.data[loop.index].uv = (u, v)
    obj = bpy.data.objects.new("ground", mesh)
    bpy.context.scene.collection.objects.link(obj)
    mat = bpy.data.materials.new("ground")
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    nodes.clear()
    tex = nodes.new("ShaderNodeTexImage")
    tex.image = bpy.data.images.load(g["picture"])
    tex.image.colorspace_settings.name = "sRGB"
    tex.interpolation = "Closest"
    tex.extension = "EXTEND"
    bsdf = nodes.new("ShaderNodeBsdfDiffuse")
    out = nodes.new("ShaderNodeOutputMaterial")
    links.new(tex.outputs["Color"], bsdf.inputs["Color"])
    links.new(bsdf.outputs["BSDF"], out.inputs["Surface"])
    mesh.materials.append(mat)


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
                    o.location = Vector(o.location) - Vector((middle.x - o.location.x, middle.y - o.location.y, 0.0))


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
    parts(cfg)
    camera(cfg)
    light(cfg)
    scene.render.filepath = cfg["out"]
    bpy.ops.render.render(write_still=True)


main()
