"""A family's atlas layouts (A6.4), written from Blender for paint.py: every triangle of every part drawn to an
atlas, with its texture coordinates in metres, its corners' rest positions and normals, its role and the bone that
moves each corner most, so a figure's texture can be drawn to fit its parts and joints.

    blender -b art/models/<family>.blend --python tools/art/layout.py -- <out.json>

The JSON holds {"atlases": {name: [width, height] in metres}, "parts": [names], "roles": [names], "bones":
[names], "triangles": [[atlas, part, role, b0, b1, b2, u0, v0, u1, v1, u2, v2, x0, y0, z0, x1, y1, z1, x2, y2, z2,
nx0, ny0, nz0, nx1, ny1, nz1, nx2, ny2, nz2], ...]}, the indices into those lists; positions are the part's own
rest shape in metres, before any armature or shape key moves it.

Implements PRE-27 and PRE-46, see A6.3 and A6.4.
"""

import json
import sys

import bpy


def index(names, name):
    if name not in names:
        names.append(name)
    return names.index(name)


def strongest(me, groups, bones, vi):
    """The bone that moves a vertex most, as an index into bones, or -1 for none."""
    best = max(me.vertices[vi].groups, key=lambda g: g.weight, default=None)
    return index(bones, groups[best.group]) if best is not None else -1


def main(out):
    atlases, parts, roles, bones, triangles = {}, [], [], [], []
    for ob in sorted(bpy.data.objects, key=lambda o: o.name):
        if ob.type != "MESH" or "kit_atlas" not in ob:
            continue
        name = str(ob["kit_atlas"])
        size = [float(x) for x in str(ob.get("kit_atlas_size", "0 x 0")).split(" x ")]
        atlases.setdefault(name, size)
        a = list(atlases).index(name)
        me = ob.data
        me.calc_loop_triangles()
        uv = me.uv_layers["UVMap"].data
        groups = {g.index: g.name for g in ob.vertex_groups}
        p = index(parts, ob.name)
        for tri in me.loop_triangles:
            slot = ob.material_slots[tri.material_index].name if ob.material_slots else ""
            row = [a, p, index(roles, slot)] + [strongest(me, groups, bones, vi) for vi in tri.vertices]
            for li in tri.loops:
                row += [round(uv[li].uv.x, 6), round(uv[li].uv.y, 6)]
            for vi in tri.vertices:
                row += [round(c, 5) for c in me.vertices[vi].co]
            for vi in tri.vertices:
                row += [round(c, 4) for c in me.vertices[vi].normal]
            triangles.append(row)
    with open(out, "w") as f:
        json.dump(
            {"atlases": atlases, "parts": parts, "roles": roles, "bones": bones, "triangles": triangles},
            f,
            separators=(",", ":"),
        )
    print(f"{out}: {len(atlases)} atlases, {len(parts)} parts, {len(triangles)} triangles")


if __name__ == "__main__":
    main(sys.argv[sys.argv.index("--") + 1])
