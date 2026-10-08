"""Exports a family's parts from the open Blender file into the .kdkit file the engine reads (A6.1, A6.4). Run by
tools/kit.py inside Blender, headless, as

    blender --background --factory-startup <family>.blend --python tools/blender/export.py -- <out.kdkit>

The file's layout is described where the engine reads it,
view/src/kit.hpp. Implements PRE-46.

What a family's Blender file holds (the contract between the art lane's parts and the engine):
- a part is a mesh object with no parent whose name is lower case letters, digits and _, starting with a letter;
  an object whose name starts with _ is a helper, and is left out; every part must be visible in the view layer;
- its joints are empties parented to it, named joint_<name>: the empty's place and axes are the joint's, and its own
  z axis is its main one;
- its material slots are named by role (wood, bark, hide, stone, leaf, grass, skin, hair), by the slot's material's
  name, a Blender suffix such as .001 dropped;
- its first UV map holds texture coordinates in metres, one texture metre to a metre of surface, so a texture pixel
  is 1/64 m at band 0; v is as Blender shows it, running up;
- an optional colour attribute (or float attribute) named crease holds baked darkening, 1 open and 0 dark;
- the object's own place, turn and scale are kept, but its place is taken off, so a part sits anywhere in the file.
Everything is carried into Godot's axes (x east, y up, z south, from Blender's z up and y north), triangles wound as
Godot's front faces are, and written in the order of names, so the same file gives the same bytes.
"""

import re
import struct
import sys

import bpy
from mathutils import Matrix, Vector

VERSION = 1
NAME = re.compile(r"^[a-z][a-z0-9_]*$")
# Blender's axes to Godot's: x stays, Blender's z is up, and its y, north, is Godot's -z
TO_GODOT = Matrix(((1.0, 0.0, 0.0), (0.0, 0.0, 1.0), (0.0, -1.0, 0.0)))


class KitError(Exception):
    pass


def role_of(slot, part):
    """A material slot's role: its material's name, lower case, a Blender suffix such as .001 dropped."""
    if slot.material is None:
        raise KitError(f"the part {part} has a material slot with no material")
    name = re.sub(r"\.\d{3}$", "", slot.material.name).lower()
    if not NAME.match(name):
        raise KitError(f"the part {part} has a material named {slot.material.name!r}, which is no role's name")
    return name


def corner_normals(mesh):
    """The normal at each corner of each face as Blender shows the shading, for Blender 4.0 and for 4.1 and later."""
    if hasattr(
        mesh, "calc_normals_split"
    ):  # Blender 4.0 and earlier compute them on request; 4.1 and later always have them
        mesh.calc_normals_split()
    if hasattr(mesh, "corner_normals") and len(mesh.corner_normals) == len(mesh.loops):
        return [Vector(n.vector) for n in mesh.corner_normals]
    return [Vector(loop.normal) for loop in mesh.loops]


def crease_of(mesh):
    """A function from a corner and its vertex to the crease there, from the colour or float attribute named crease."""
    attr = mesh.color_attributes.get("crease") if hasattr(mesh, "color_attributes") else None
    if attr is not None:
        if attr.domain == "CORNER":
            return lambda corner, vertex: attr.data[corner].color[0]
        return lambda corner, vertex: attr.data[vertex].color[0]
    attr = mesh.attributes.get("crease")
    if attr is not None and attr.data_type == "FLOAT":
        if attr.domain == "CORNER":
            return lambda corner, vertex: attr.data[corner].value
        return lambda corner, vertex: attr.data[vertex].value
    return lambda corner, vertex: 1.0


def part_object_names():
    """The parts' names: top-level meshes not starting with _, in the order of their names."""
    return sorted(
        o.name
        for o in bpy.context.scene.objects
        if o.type == "MESH" and o.parent is None and not o.name.startswith("_")
    )


def read_part(obj, depsgraph):
    """One part: {"sections": {role: {positions, normals, uvs, crease, indices}}, "joints": [...], "lowest",
    "highest"}."""
    name = obj.name
    if not NAME.match(name):
        raise KitError(f"the part {name!r} is not named in lower case letters, digits and _, starting with a letter")
    if not obj.visible_get():
        raise KitError(f"the part {name} is hidden: show it in the view layer so it can be exported")
    take_off = Matrix.Translation(-obj.matrix_world.translation)
    carrying = take_off @ obj.matrix_world
    linear = carrying.to_3x3()
    # a mirrored object turns its faces inside out, which winding them the other way puts right
    mirrored = linear.determinant() < 0.0
    normal_matrix = linear.inverted().transposed()
    evaluated = obj.evaluated_get(depsgraph)
    mesh = evaluated.to_mesh()
    try:
        # the normals first: computing them reallocates the mesh's layers, which would leave any reference
        # taken before it pointing at other data
        normals = corner_normals(mesh)
        mesh.calc_loop_triangles()
        if not mesh.uv_layers:
            raise KitError(f"the part {name} has no UV map")
        uv_layer = mesh.uv_layers.active or mesh.uv_layers[0]
        crease = crease_of(mesh)
        roles = [role_of(slot, name) for slot in evaluated.material_slots]
        if not roles:
            raise KitError(f"the part {name} has no material slot, so it wears no role")
        sections = {}
        for tri in mesh.loop_triangles:
            role = roles[min(tri.material_index, len(roles) - 1)]
            section = sections.setdefault(
                role, {"vertices": {}, "positions": [], "normals": [], "uvs": [], "crease": [], "indices": []}
            )
            order = (0, 1, 2) if mirrored else (0, 2, 1)
            for k in order:
                corner = tri.loops[k]
                vertex = tri.vertices[k]
                p = TO_GODOT @ (carrying @ mesh.vertices[vertex].co)
                n = (TO_GODOT @ (normal_matrix @ normals[corner])).normalized()
                uv = uv_layer.data[corner].uv
                u, v = float(uv[0]), -float(uv[1])
                c = max(0.0, min(1.0, float(crease(corner, vertex))))
                key = (
                    round(p.x * 1e6),
                    round(p.y * 1e6),
                    round(p.z * 1e6),
                    round(n.x * 1e4),
                    round(n.y * 1e4),
                    round(n.z * 1e4),
                    round(u * 1e6),
                    round(v * 1e6),
                    round(c * 255.0),
                )
                index = section["vertices"].get(key)
                if index is None:
                    index = len(section["crease"])
                    section["vertices"][key] = index
                    section["positions"] += [p.x, p.y, p.z]
                    section["normals"] += [n.x, n.y, n.z]
                    section["uvs"] += [u, v]
                    section["crease"].append(round(c * 255.0))
                section["indices"].append(index)
    finally:
        evaluated.to_mesh_clear()
    joints = []
    for child in sorted(obj.children, key=lambda o: o.name):
        if child.type != "EMPTY" or not child.name.startswith("joint_"):
            continue
        joint_name = re.sub(r"\.\d{3}$", "", child.name[len("joint_") :])
        if not NAME.match(joint_name):
            raise KitError(
                f"the joint {child.name!r} of the part {name} is not joint_ and a name in lower case letters, "
                "digits and _"
            )
        placed = take_off @ child.matrix_world
        axes = TO_GODOT @ placed.to_quaternion().to_matrix()
        at = TO_GODOT @ placed.translation
        joints.append((joint_name, [axes[r][c] for r in range(3) for c in range(3)], [at.x, at.y, at.z]))
    joints.sort(key=lambda j: j[0])
    names = [j[0] for j in joints]
    if len(set(names)) != len(names):
        raise KitError(f"the part {name} has two joints of one name")
    points = [
        Vector(section["positions"][i : i + 3])
        for section in sections.values()
        for i in range(0, len(section["positions"]), 3)
    ]
    if not points:
        raise KitError(f"the part {name} has no triangles")
    lowest = [min(p[i] for p in points) for i in range(3)]
    highest = [max(p[i] for p in points) for i in range(3)]
    return {"sections": sections, "joints": joints, "lowest": lowest, "highest": highest}


def text(s):
    data = s.encode("ascii")
    return struct.pack("<H", len(data)) + data


def floats(values):
    # + 0.0 turns a negative zero into a plain one, so the same part is the same bytes however its zeros were reached
    return struct.pack(f"<{len(values)}f", *[v + 0.0 for v in values])


def encode(parts):
    """The .kdkit file's bytes for parts, {name: read_part's}."""
    out = bytearray(b"KDKT" + struct.pack("<II", VERSION, len(parts)))
    for name in sorted(parts):
        part = parts[name]
        out += text(name) + floats(part["lowest"]) + floats(part["highest"])
        out += struct.pack("<I", len(part["sections"]))
        for role in sorted(part["sections"]):
            s = part["sections"][role]
            out += text(role) + struct.pack("<II", len(s["crease"]), len(s["indices"]))
            out += floats(s["positions"]) + floats(s["normals"]) + floats(s["uvs"])
            out += bytes(s["crease"]) + struct.pack(f"<{len(s['indices'])}I", *s["indices"])
        out += struct.pack("<I", len(part["joints"]))
        for joint_name, axes, at in part["joints"]:
            out += text(joint_name) + floats(axes) + floats(at)
    return bytes(out)


def export(path):
    """Writes every part of the open file to path, and says what it wrote; raises KitError naming the first fault."""
    bpy.context.view_layer.update()
    depsgraph = bpy.context.evaluated_depsgraph_get()
    parts = {}
    for name in part_object_names():
        parts[name] = read_part(bpy.data.objects[name], depsgraph)
    if not parts:
        raise KitError("the file has no part: a part is a mesh object with no parent whose name does not start with _")
    data = encode(parts)
    with open(path, "wb") as f:
        f.write(data)
    triangles = sum(len(s["indices"]) // 3 for p in parts.values() for s in p["sections"].values())
    joints = sum(len(p["joints"]) for p in parts.values())
    print(f"KDKIT: {len(parts)} parts, {triangles} triangles, {joints} joints, {len(data)} bytes")


def main():
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    if len(argv) != 1:
        raise SystemExit("usage: blender --background <family>.blend --python tools/blender/export.py -- <out.kdkit>")
    try:
        export(argv[0])
    except KitError as e:
        print(f"KDKIT: {e}", file=sys.stderr)
        raise SystemExit(1) from None


if __name__ == "__main__":
    main()
