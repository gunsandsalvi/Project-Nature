#!/usr/bin/env python3
"""P1's scene (IMPLEMENTATION α0.2a): the art book's close camp, as the painter exports it (art/book/paint, the
"export" option), packed for Godot. Equal vertices are merged, attributes packed into bytes and half floats, and
every mesh cut into pieces of at most 65,535 vertices, so Godot keeps 16-bit indices.

    python3 make_scene.py <painter export folder> <out folder>

The ground keeps only its position, normal and cover weights (its materials are the mesh's layers); everything else
is cropped to the ground's extent, since beyond it there is nothing to stand on. Writes pack.json (the meshes'
layout, and the painter's camera, light, look and palette) and pack.bin, which build_scene.gd turns into
close_camp.scn. Pre-production code (research 00): thrown away with the prototypes.
"""

import json
import os
import sys

import numpy as np

LIMIT = 65535


def main(src, out):
    meta = json.load(open(os.path.join(src, "scene.json")))
    raw = open(os.path.join(src, "scene.bin"), "rb").read()

    def arr(e, k):
        a = e["attrs"][k]
        n = a["count"] * a["size"]
        return np.frombuffer(raw, dtype=np.float32, count=n, offset=a["offset"]).reshape(a["count"], a["size"])

    def index(e, n):
        if not e["index"]:
            return np.arange(n, dtype=np.int64)
        return np.frombuffer(raw, dtype=np.uint32, count=e["index"]["count"], offset=e["index"]["offset"]).astype(
            np.int64
        )

    chunks, blob, at = [], [], 0

    def put(a):
        nonlocal at
        b = np.ascontiguousarray(a).tobytes()
        blob.append(b)
        start = at
        at += len(b)
        pad = (-at) % 4
        if pad:
            blob.append(b"\0" * pad)
            at += pad
        return start

    def pieces(kind, cols, tris, extra):
        """Merges equal vertices of the columns (each a 2-D array, one row a vertex), then cuts the triangles into
        pieces of at most LIMIT vertices; writes each piece's columns and 16-bit indices."""
        key = np.hstack([c.view(np.uint8).reshape(len(c), -1) for c in cols])
        _, first, inv = np.unique(key, axis=0, return_index=True, return_inverse=True)
        inv = inv.reshape(-1)
        tri = inv[tris].reshape(-1, 3)
        start = 0
        while start < len(tri):
            # grow the piece triangle by triangle in blocks, until its vertices would pass the limit
            end = len(tri)
            used = np.unique(tri[start:end])
            while len(used) > LIMIT:
                end = start + (end - start) // 2
                used = np.unique(tri[start:end])
            step = max(1, (end - start) // 8)
            while end < len(tri):
                more = np.unique(tri[start : min(len(tri), end + step)])
                if len(more) > LIMIT:
                    break
                end = min(len(tri), end + step)
                used = more
            remap = np.full(len(first), -1, dtype=np.int64)
            remap[used] = np.arange(len(used))
            rows = first[used]
            piece = {"kind": kind, "vertices": len(used), "index": put(remap[tri[start:end]].astype(np.uint16))}
            piece["triangles"] = end - start
            piece["columns"] = [put(c[rows]) for c in cols]
            piece.update(extra)
            chunks.append(piece)
            start = end

    ground = next(e for e in meta["meshes"] if e["kind"] == "solid" and any(e["layerPat"]))
    gp = arr(ground, "position")
    lo, hi = gp[:, [0, 2]].min(axis=0), gp[:, [0, 2]].max(axis=0)
    meta["extent"] = [float(lo[0]), float(lo[1]), float(hi[0]), float(hi[1])]

    def inside(points, tris):
        """The triangles with a corner over the ground (a river's long pieces cross it with their ends outside)."""
        corners = points[tris.reshape(-1, 3)][:, :, [0, 2]]
        keep = np.any(np.all((corners >= lo) & (corners <= hi), axis=2), axis=1)
        return tris.reshape(-1, 3)[keep].reshape(-1)

    for e in meta["meshes"]:
        kind = e["kind"]
        n = list(e["attrs"].values())[0]["count"]
        tris = index(e, n)
        if e is ground:
            pos = arr(e, "position").astype(np.float32)
            nrm = np.clip(np.round(arr(e, "normal") * 127), -127, 127).astype(np.int8)
            nrm = np.hstack([nrm, np.zeros((n, 1), np.int8)])
            w = (arr(e, "aW") * 255).round().clip(0, 255).astype(np.uint8)
            pieces("ground", [pos, nrm, w], tris, {"layers": e["layers"], "layerPat": e["layerPat"]})
            continue
        centre = arr(e, "aCenter") if kind in ("cards", "puffs") else arr(e, "position")
        tris = inside(centre, tris)
        if len(tris) == 0:
            continue
        if kind == "solid":
            pos = arr(e, "position").astype(np.float32)
            nrm = np.clip(np.round(arr(e, "normal") * 127), -127, 127).astype(np.int8)
            nrm = np.hstack([nrm, np.zeros((n, 1), np.int8)])
            mat = arr(e, "aMat")[:, 0]
            bias = arr(e, "aBias")[:, 0]
            pat = arr(e, "aPat")[:, 0]
            flag = arr(e, "aFlag")[:, 0]
            obj = arr(e, "aObj")[:, 0].astype(np.int64)
            c0 = np.stack([mat, bias + 128, pat, flag], axis=1).round().clip(0, 255).astype(np.uint8)
            loc = np.hstack([arr(e, "aLoc"), np.zeros((n, 1), np.float32)]).astype(np.float16)
            w = (arr(e, "aW") * 255).round().clip(0, 255).astype(np.uint8)
            o = np.stack([obj & 255, (obj >> 8) & 255, (obj >> 16) & 255, np.zeros(n, np.int64)], axis=1).astype(
                np.uint8
            )
            extra = {"layers": e["layers"], "layerPat": e["layerPat"]}
            pieces("solid", [pos, nrm, c0, loc, w, o], tris, extra)
        elif kind == "cards":
            ctr = arr(e, "aCenter").astype(np.float32)
            nrm = np.clip(np.round(arr(e, "aNrm") * 127), -127, 127).astype(np.int8)
            nrm = np.hstack([nrm, np.zeros((n, 1), np.int8)])
            cs = np.hstack([arr(e, "aCorner"), arr(e, "aSize")]).astype(np.float16)
            c1 = np.stack(
                [arr(e, "aTile")[:, 0], arr(e, "aMat")[:, 0], arr(e, "aBias")[:, 0] + 128, arr(e, "aFlag")[:, 0]],
                axis=1,
            )
            c1 = c1.round().clip(0, 255).astype(np.uint8)
            us = np.hstack([arr(e, "aUpright"), arr(e, "aSpin")]).astype(np.float16)
            obj = arr(e, "aObj")[:, 0].astype(np.int64)
            o = np.stack([obj & 255, (obj >> 8) & 255, (obj >> 16) & 255, np.zeros(n, np.int64)], axis=1).astype(
                np.uint8
            )
            pieces("cards", [ctr, nrm, cs, c1, us, o], tris, {})
        elif kind == "water":
            pos = arr(e, "position").astype(np.float32)
            fl = np.hstack([arr(e, "aFlow"), np.full((n, 1), e["sea"], np.float32)]).astype(np.float16)
            pieces("water", [pos, fl], tris, {})
        elif kind == "puffs":
            ctr = arr(e, "aCenter").astype(np.float32)
            p = np.hstack([arr(e, "aCorner"), arr(e, "aSize"), arr(e, "aTone"), arr(e, "aAlpha")]).astype(np.float16)
            p = np.hstack([p, np.zeros((n, 3), np.float16)])
            pieces("puffs", [ctr, p], tris, {"row": e["row"], "shift": e["shift"]})

    a = meta["atlas"]
    atlas = np.frombuffer(raw, dtype=np.uint8, count=a["size"] * a["size"] * 4, offset=a["offset"])
    meta["atlas"] = {"offset": put(atlas), "size": a["size"]}
    meta["meshes"] = chunks
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "pack.bin"), "wb") as f:
        for b in blob:
            f.write(b)
    with open(os.path.join(out, "pack.json"), "w") as f:
        json.dump(meta, f)
    kinds = {}
    for c in chunks:
        k = kinds.setdefault(c["kind"], [0, 0, 0])
        k[0] += 1
        k[1] += c["vertices"]
        k[2] += c["triangles"]
    print(f"pack: {at / 1e6:.1f} MB; pieces, vertices, triangles: {kinds}")


if __name__ == "__main__":
    main(*sys.argv[1:])
