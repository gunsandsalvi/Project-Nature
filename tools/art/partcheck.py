"""The kit's part check (A6.4, PRE-46): every part of a Blender file measured against the rules its parts keep, and
its stretch report written.

    blender -b <file.blend> --python tools/art/partcheck.py -- [--report out.txt] [--json out.json]

Run inside Blender 4.0. A part is a mesh object at the top of the file, or one skinned to an armature; its joints are
the empties parented to it, or for a skinned part its armature's bones. For each part:
- slots: one or more, each named by a role (kit.ROLES);
- uv: one UV map, UVMap;
- joints: one or more, each named joint_..., the main one at the part's origin (within a millimetre);
- weights: for a skinned part, every vertex weighted by one to four bones of its armature, summing to 1;
- stretch: every triangle's texture pixels within 1.5:1 of squares of 1/64 m (kitmath.stretch), at rest, with each
  shape key at 1, and in each pose the file keeps as an action named bend_... on its armature;
- mirrored: no triangle's texture lies mirrored seen from its front, so a face or a design never reads backwards;
- atlas: for the parts drawn to one atlas (a figure's skin and hair, a garment's), no texture pixel lies under two
  faces and every face lies inside the atlas.
It prints one line a part, writes the report and the figures as JSON (for the preview sheet), and ends with exit
status 1 if any part fails, as tools/art/models.py reads it.

Implements PRE-46 and PRE-22, see A6.4.
"""

import json
import math
import os
import sys

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import kit  # noqa: E402
import kitmath  # noqa: E402

LINE = kitmath.STRETCH_MOST
NEAR = 1.25  # the report counts the triangles over this, short of the line
ORIGIN = 0.001  # metres: how near the origin the main joint must be


def parts():
    """The file's parts, in the order of its collections and their objects."""
    seen, out = set(), []
    for c in [bpy.context.scene.collection] + list(bpy.context.scene.collection.children_recursive):
        for o in c.objects:
            if o.type != "MESH" or o.name in seen or o.name.startswith("joint_"):
                continue
            if o.parent is None or o.parent.type == "ARMATURE":
                seen.add(o.name)
                out.append(o)
    return out


def armature_of(o):
    for m in o.modifiers:
        if m.type == "ARMATURE" and m.object is not None:
            return m.object
    return o.parent if o.parent is not None and o.parent.type == "ARMATURE" else None


def triangles(me, positions):
    """The worst stretch of a mesh's triangles at the given vertex positions, with the share over NEAR, the count of
    triangles with no area and of those whose texture lies mirrored: (worst, share, flat, mirrored, count)."""
    me.calc_loop_triangles()
    uv = me.uv_layers.get(kit.UV_NAME)
    worst, over, flat, flipped = 1.0, 0, 0, 0
    if uv is None:
        return math.inf, 1.0, 0, 0, len(me.loop_triangles)
    for tri in me.loop_triangles:
        p = [tuple(positions[i]) for i in tri.vertices]
        t = [tuple(uv.data[li].uv) for li in tri.loops]
        s = kitmath.stretch(*p, *t)
        if s is None:
            flat += 1
            continue
        flipped += kitmath.mirrored(*p, *t, kitmath.cross(kitmath.sub(p[1], p[0]), kitmath.sub(p[2], p[0])))
        worst = max(worst, s)
        over += s > NEAR
    n = len(me.loop_triangles)
    return worst, (over / n if n else 0.0), flat, flipped, n


def measures(o):
    """Every figure the check and the sheet use for one part."""
    me = o.data
    problems = []
    slots = [s.material.name if s.material else "" for s in o.material_slots]
    if not slots:
        problems.append("no material slot")
    for s in slots:
        if s not in kit.ROLES:
            problems.append(f"slot {s!r} is not a role")
    if [u.name for u in me.uv_layers] != [kit.UV_NAME]:
        problems.append(f"UV maps {[u.name for u in me.uv_layers]}, not one named {kit.UV_NAME}")
    arm = armature_of(o)
    joints = []
    for c in o.children:
        if c.type != "EMPTY":
            continue
        if not c.name.startswith("joint_"):
            problems.append(f"an empty named {c.name!r}, not joint_...")
        joints.append((c.get("joint", c.name[len("joint_") :].split(".")[0]), tuple(c.location)))
    if arm is None:
        if not joints:
            problems.append("no joint")
        elif not any(math.dist(at, (0, 0, 0)) <= ORIGIN for _, at in joints):
            problems.append("no joint at its origin (its main joint)")
    else:
        bones = {b.name for b in arm.data.bones}
        groups = {g.index: g.name for g in o.vertex_groups}
        for g in groups.values():
            if g not in bones:
                problems.append(f"vertex group {g!r} is no bone of {arm.name}")
        unweighted = heavy = unsummed = 0
        for v in me.vertices:
            ws = [g.weight for g in v.groups if g.weight > 0 and g.group in groups]
            if not ws:
                unweighted += 1
            if len(ws) > 4:
                heavy += 1
            if ws and abs(sum(ws) - 1) > 0.01:
                unsummed += 1
        if unweighted:
            problems.append(f"{unweighted} vertices weighted by no bone")
        if heavy:
            problems.append(f"{heavy} vertices weighted by more than four bones")
        if unsummed:
            problems.append(f"{unsummed} vertices whose weights do not sum to 1")
    rest = [v.co for v in me.vertices]
    worst, share, flat, flipped, count = triangles(me, rest)
    if flipped:
        problems.append(f"{flipped} triangles' textures lie mirrored, seen from their front")
    where = "at rest"
    keys = []
    if me.shape_keys:
        for kb in me.shape_keys.key_blocks[1:]:
            keys.append(kb.name)
            w = triangles(me, [d.co for d in kb.data])[0]
            if w > worst:
                worst, where = w, f"as {kb.name}"
    if arm is not None and arm.animation_data is not None:
        for act in [a for a in bpy.data.actions if a.name.startswith("bend")]:
            arm.animation_data.action = act
            bpy.context.scene.frame_set(1)
            dg = bpy.context.evaluated_depsgraph_get()
            ev = o.evaluated_get(dg)
            m2 = ev.to_mesh()
            w = triangles(m2, [v.co for v in m2.vertices])[0]
            ev.to_mesh_clear()
            if w > worst:
                worst, where = w, f"in {act.name}"
            arm.animation_data.action = None
            bpy.context.scene.frame_set(1)
    if worst > LINE:
        problems.append(f"stretched {worst:.2f}:1 {where}, over the line of {LINE}")
    lo = [min(v.co[i] for v in me.vertices) for i in range(3)] if me.vertices else [0, 0, 0]
    hi = [max(v.co[i] for v in me.vertices) for i in range(3)] if me.vertices else [0, 0, 0]
    return {
        "name": o.name,
        "group": o.users_collection[0].name if o.users_collection else "",
        "about": o.get("kit_about", ""),
        "size": [round(hi[i] - lo[i], 4) for i in range(3)],
        "low": [round(x, 4) for x in lo],
        "high": [round(x, 4) for x in hi],
        "triangles": count,
        "worst": round(worst, 3) if worst != math.inf else "infinite",
        "where": where,
        "near_share": round(share, 4),
        "flat": flat,
        "slots": slots,
        "joints": [j for j, _ in joints] if arm is None else [],
        "armature": arm.name if arm is not None else "",
        "keys": keys,
        "wrap_texels": o.get("kit_wrap_texels", ""),
        "problems": problems,
    }


def atlases(objects):
    """{part: [problems]} for the parts drawn to an atlas (kit.Atlas): every texture pixel of an atlas may lie under
    one face only, among all the parts that share it, and every face must lie inside it. Each texture pixel is
    sampled at its centre, nudged by a small odd offset so that no sample falls on an edge two faces share."""
    found = {}
    shared = {}
    for o in objects:
        if o.get("kit_atlas"):
            shared.setdefault(o["kit_atlas"], []).append(o)
    for name, obs in shared.items():
        w, h = (float(x) for x in obs[0]["kit_atlas_size"].split(" x "))
        cover = {}
        twice, outside = {}, {}
        for o in obs:
            me = o.data
            me.calc_loop_triangles()
            uv = me.uv_layers.get(kit.UV_NAME)
            for tri in me.loop_triangles:
                t = [tuple(uv.data[li].uv) for li in tri.loops]
                if any(not (-1e-4 <= u <= w + 1e-4 and -1e-4 <= v <= h + 1e-4) for u, v in t):
                    outside[o.name] = outside.get(o.name, 0) + 1
                for x, y in _texels_under(t):
                    if (x, y) in cover:
                        twice[o.name] = twice.get(o.name, 0) + 1
                        other = cover[(x, y)]
                        if other != o.name:
                            twice[other] = twice.get(other, 0) + 1
                    else:
                        cover[(x, y)] = o.name
        for part, n in twice.items():
            found.setdefault(part, []).append(f"{n} texture pixels of atlas {name} lie under two faces")
        for part, n in outside.items():
            found.setdefault(part, []).append(f"{n} triangles lie outside atlas {name} ({w:.2f} x {h:.2f} m)")
    return found


def _texels_under(t):
    """The texture pixels whose nudged centres a triangle's texture coordinates (in metres) cover."""
    p = [(u * kitmath.TEXELS_A_METRE, v * kitmath.TEXELS_A_METRE) for u, v in t]
    (x0, y0), (x1, y1), (x2, y2) = p
    den = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2)
    if abs(den) < 1e-12:
        return
    for x in range(math.floor(min(x0, x1, x2)), math.ceil(max(x0, x1, x2))):
        for y in range(math.floor(min(y0, y1, y2)), math.ceil(max(y0, y1, y2))):
            sx, sy = x + 0.5 + 0.0137, y + 0.5 + 0.0071
            a = ((y1 - y2) * (sx - x2) + (x2 - x1) * (sy - y2)) / den
            b = ((y2 - y0) * (sx - x2) + (x0 - x2) * (sy - y2)) / den
            if a >= 0 and b >= 0 and a + b <= 1:
                yield x, y


def report(path, results):
    """The stretch report: the file's worst figure first, then one line a part, then every problem."""
    worst = max((r["worst"] if isinstance(r["worst"], float) else math.inf for r in results), default=1.0)
    failed = [r for r in results if r["problems"]]
    lines = [
        f"# Stretch report: {os.path.basename(bpy.data.filepath)}",
        "",
        f"{len(results)} parts; the worst triangle is stretched {worst:.2f}:1 (the line is {LINE}:1, A6.4);",
        f"{len(failed)} parts fail a check.",
        "Stretch is the worst ratio, in any direction, of a texture pixel's side on the surface to 1/64 m, or the",
        "reverse: 1 means square texture pixels of 1/64 m. Each part is measured at rest, with each shape key at 1",
        "and in each bend pose its armature keeps.",
        "",
        f"{'part':28s} {'size (m)':20s} {'tris':>5s} {'worst':>6s} {'over 1.25':>9s}  where, slots, joints",
    ]
    for r in results:
        size = " x ".join(f"{x:.2f}" for x in r["size"])
        w = r["worst"] if isinstance(r["worst"], str) else f"{r['worst']:.2f}"
        joints = ", ".join(r["joints"]) if r["joints"] else f"bones of {r['armature']}"
        extra = f"; wraps {r['wrap_texels']} texture pixels" if r["wrap_texels"] else ""
        keys = f"; keys {', '.join(r['keys'])}" if r["keys"] else ""
        lines.append(
            f"{r['name']:28s} {size:20s} {r['triangles']:5d} {w:>6s} {r['near_share']:9.1%}  {r['where']}; "
            f"{', '.join(r['slots'])}; {joints}{extra}{keys}"
        )
    if failed:
        lines += ["", "Problems:"]
        for r in failed:
            lines += [f"- {r['name']}: {p}" for p in r["problems"]]
    with open(path, "w") as f:
        f.write("\n".join(lines) + "\n")


def main(argv):
    args = argv[argv.index("--") + 1 :] if "--" in argv else []
    out_report = args[args.index("--report") + 1] if "--report" in args else None
    out_json = args[args.index("--json") + 1] if "--json" in args else None
    objects = parts()
    results = [measures(o) for o in objects]
    shared = atlases(objects)
    for r in results:
        r["problems"] += shared.get(r["name"], [])
    for r in results:
        state = "FAIL" if r["problems"] else "pass"
        print(f"{state} {r['name']}: worst {r['worst']} {r['where']}; " + "; ".join(r["problems"]))
    if out_report:
        report(out_report, results)
    if out_json:
        with open(out_json, "w") as f:
            json.dump({"file": os.path.basename(bpy.data.filepath), "parts": results}, f, indent=1)
    failed = sum(1 for r in results if r["problems"])
    print(f"{len(results)} parts, {failed} failed")
    return 1 if failed or not results else 0


if __name__ == "__main__":
    code = main(sys.argv)
    sys.stdout.flush()
    os._exit(code)
