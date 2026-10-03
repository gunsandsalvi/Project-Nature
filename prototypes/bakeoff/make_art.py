#!/usr/bin/env python3
"""The bake-off's art, made by code to the art guide (research/04-art.md): pixel textures drawn at 16 pixels a metre
in hue-shifted ramps, and models (leaf-card trees and bushes, chiselled rocks, cliff columns, a hide tent, a fire ring,
a log seat, a windbreak) written as glTF. A throwaway prototype; numpy and Pillow only.

    python3 prototypes/bakeoff/make_art.py
"""
import json
import math
import os
import random
import struct

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "assets")
TEX = os.path.join(OUT, "textures")
MOD = os.path.join(OUT, "models")
PPM = 16                                     # texture pixels a metre (art guide, rule 2)


def hexrgb(h):
    return tuple(int(h[i:i + 2], 16) for i in (1, 3, 5))


# Hue-shifted ramps, darkest first (art guide, rule 3): shadows lean cool, lights warm.
RAMPS = {
    "grass": ["#2c5449", "#3d7350", "#5a9450", "#7fb35a", "#aacd6a"],
    "leaf": ["#22443f", "#2e6249", "#47864b", "#70aa52", "#a5cc62"],
    "pine": ["#1d3a3c", "#264f45", "#346a4c", "#4f8a52", "#78aa5c"],
    "rock": ["#3f3a4e", "#5d566a", "#807888", "#a69e9e", "#cbc3b4"],
    "sand": ["#6a5a4e", "#8f7a62", "#b39c78", "#d2bd92"],
    "bark": ["#33262c", "#523a34", "#72523e", "#94704c"],
    "wood": ["#6e5038", "#93704a", "#b9935e", "#dcb67a"],
    "hide": ["#5e3e30", "#83593e", "#a9774e", "#cb9a66", "#e6bd86"],
    "reed": ["#6e6236", "#94823f", "#b8a252", "#d8c272"],
    "dirt": ["#4e3c34", "#6d5444", "#8c6e56", "#ab8c6c"],
    "moss": ["#3a5a3a", "#557a40", "#78984a"],
    "flower": ["#8a5ab8", "#c492e0", "#e8c448", "#f2eee2", "#c84e48"],
    "flame": ["#c83c28", "#ec7a2c", "#f8b840", "#fff0a0"],
}


def ramp(name, i):
    r = RAMPS[name]
    return hexrgb(r[max(0, min(len(r) - 1, i))])


# --- Textures. Every one tiles, and is drawn at 16 pixels a metre. ---

def save(img, name):
    os.makedirs(TEX, exist_ok=True)
    img.save(os.path.join(TEX, name))


def tex_rock():
    """Strata (art guide, rule 8): horizontal layers of different thickness, each with a lighter top row where it
    catches the light and a dark seam below, short cracks, and a few speckles. 64 x 64 pixels, 4 m square."""
    r = random.Random(11)
    w = h = 64
    img = np.zeros((h, w, 3), np.uint8)
    y = 0
    bands = []
    while y < h:
        t = r.choice([2, 3, 3, 4, 5, 6, 7])
        t = min(t, h - y)
        tone = r.choice([1, 2, 2, 3, 3])
        sandy = r.random() < 0.25
        bands.append((y, t, tone, sandy))
        y += t
    for y0, t, tone, sandy in bands:
        for yy in range(y0, y0 + t):
            for x in range(w):
                if sandy:
                    c = ramp("sand", 2 if yy > y0 else 3)
                else:
                    c = ramp("rock", tone + (1 if yy == y0 else 0))
                if yy == y0 + t - 1 and t > 2:
                    c = ramp("rock", max(tone - 1, 0))
                img[yy, x] = c
        # A wobble: the layer's top edge steps up or down by a pixel here and there.
        for x in range(w):
            if r.random() < 0.12 and y0 > 0:
                img[y0, x] = img[y0 - 1, x]
    for _ in range(26):                       # cracks
        x, y0 = r.randrange(w), r.randrange(h)
        for k in range(r.randint(2, 5)):
            img[(y0 + k) % h, (x + (k // 3)) % w] = ramp("rock", 0)
    for _ in range(60):                       # speckles
        x, yy = r.randrange(w), r.randrange(h)
        img[yy, x] = ramp("rock", r.choice([1, 4]))
    save(Image.fromarray(img), "rock.png")


def tex_rock_top():
    """The top faces of rock: pale stone with lichen speckles and patches of moss. 32 x 32 pixels, 2 m square."""
    r = random.Random(12)
    n = 32
    img = np.zeros((n, n, 3), np.uint8)
    for yy in range(n):
        for x in range(n):
            img[yy, x] = ramp("rock", 3 if r.random() < 0.85 else 4)
    for _ in range(5):                        # moss patches, wrapping round the tile
        cx, cy, rad = r.randrange(n), r.randrange(n), r.uniform(2.5, 5.5)
        for yy in range(n):
            for x in range(n):
                dx = min(abs(x - cx), n - abs(x - cx))
                dy = min(abs(yy - cy), n - abs(yy - cy))
                d = math.hypot(dx, dy) + r.uniform(-0.8, 0.8)
                if d < rad:
                    img[yy, x] = ramp("moss", 2 if d < rad * 0.4 else (1 if d < rad * 0.8 else 0))
    for _ in range(30):
        img[r.randrange(n), r.randrange(n)] = hexrgb("#c8c47a")
    save(Image.fromarray(img), "rock_top.png")


def tex_bark():
    """Bark: vertical ridges with breaks. 16 x 32 pixels, 1 m by 2 m."""
    r = random.Random(13)
    w, h = 16, 32
    img = np.zeros((h, w, 3), np.uint8)
    cols = [r.choice([1, 2, 2, 3]) for _ in range(w)]
    for x in range(w):
        for yy in range(h):
            tone = cols[x]
            if r.random() < 0.08:
                tone = 0
            img[yy, x] = ramp("bark", tone)
        if cols[x] == 3:
            img[r.randrange(h), x] = ramp("bark", 3)
    for _ in range(10):
        yy = r.randrange(h)
        x0 = r.randrange(w)
        for k in range(r.randint(2, 4)):
            img[yy, (x0 + k) % w] = ramp("bark", 0)
    save(Image.fromarray(img), "bark.png")


def tex_wood_end():
    """A log's cut end: rings round the heart. 8 x 8 pixels, half a metre."""
    img = np.zeros((8, 8, 3), np.uint8)
    for yy in range(8):
        for x in range(8):
            d = math.hypot(x - 3.5, yy - 3.5)
            img[yy, x] = ramp("bark", 1) if d > 3.4 else ramp("wood", 3 - int(d) % 2 - (1 if d < 1 else 0))
    save(Image.fromarray(img), "wood_end.png")


def tex_hide():
    """Hides stitched together (art guide, rule 9): patches in the hide ramp, darker stitches along their seams, a few
    spots. 32 x 32 pixels, 2 m square, tiling."""
    r = random.Random(14)
    n = 32
    pts = [(r.uniform(0, n), r.uniform(0, n), r.choice([1, 2, 2, 3])) for _ in range(6)]
    img = np.zeros((n, n, 3), np.uint8)
    owner = np.zeros((n, n), int)
    for yy in range(n):
        for x in range(n):
            best, bi = 1e9, 0
            for i, (px, py, _) in enumerate(pts):
                dx = min(abs(x - px), n - abs(x - px))
                dy = min(abs(yy - py), n - abs(yy - py))
                d = dx * dx + dy * dy
                if d < best:
                    best, bi = d, i
            owner[yy, x] = bi
            img[yy, x] = ramp("hide", pts[bi][2] + (1 if r.random() < 0.06 else 0))
    for yy in range(n):
        for x in range(n):
            if owner[yy, x] != owner[yy, (x + 1) % n] or owner[yy, x] != owner[(yy + 1) % n, x]:
                img[yy, x] = ramp("hide", 0) if (x + yy) % 2 == 0 else ramp("hide", 1)
    for _ in range(14):
        img[r.randrange(n), r.randrange(n)] = ramp("hide", 0)
    save(Image.fromarray(img), "hide.png")


def tex_dirt():
    """Packed dirt with small pebbles (art guide, rule 6). 32 x 32 pixels, 2 m square."""
    r = random.Random(15)
    n = 32
    img = np.zeros((n, n, 3), np.uint8)
    for yy in range(n):
        for x in range(n):
            img[yy, x] = ramp("dirt", 2 if r.random() < 0.8 else r.choice([1, 3]))
    for _ in range(18):
        x, yy = r.randrange(n), r.randrange(n)
        img[yy, x] = ramp("rock", 4)
        img[yy, (x + 1) % n] = ramp("rock", 3)
        img[(yy + 1) % n, x] = ramp("rock", 1)
    save(Image.fromarray(img), "dirt.png")


def blob(px, w, h, cx, cy, rx, ry, base, alpha=True):
    for yy in range(h):
        for x in range(w):
            d = ((x + 0.5 - cx) / rx) ** 2 + ((yy + 0.5 - cy) / ry) ** 2
            if d <= 1.0:
                shade = base
                if (x + 0.5 - cx) / rx + (yy + 0.5 - cy) / ry < -0.9:
                    shade = min(base + 40, 255)
                elif (x + 0.5 - cx) / rx + (yy + 0.5 - cy) / ry > 0.9:
                    shade = base - 45
                px[x, yy] = (shade, shade, shade, 255)


def tex_sprites():
    """The sprite sheet, 64 x 64 (art guide, rules 6 and 7):
    rows 0 to 15: four leaf clusters, 16 x 16, in grey for the leaf colour to tint, lighter where a leaf faces up-left;
    rows 16 to 23: eight grass tufts, 8 x 8, grey, tips lightest;
    rows 24 to 31: eight flowers, 8 x 8, in their own colours;
    rows 32 to 47: four flame frames, 16 x 16."""
    r = random.Random(16)
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    for k in range(4):
        tile = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
        px = tile.load()
        lobes = [(8 + 4.2 * math.cos(a), 8 + 4.2 * math.sin(a), r.uniform(2.6, 3.6))
                 for a in [j / 6 * math.tau + r.uniform(-0.3, 0.3) for j in range(6)]] + [(8, 8, 4.4)]
        for yy in range(16):
            for x in range(16):
                inside = [((x + 0.5 - cx) ** 2 + (yy + 0.5 - cy) ** 2) / (rr * rr) for cx, cy, rr in lobes]
                if min(inside) <= 1.0:
                    v = 228
                    up = (x + 0.5 - 8) + (yy + 0.5 - 8)
                    if min(inside) > 0.55 and up < -3:
                        v = 255
                    elif min(inside) > 0.55 and up > 4:
                        v = 196
                    elif r.random() < 0.06:
                        v = 208
                    px[x, yy] = (v, v, v, 255)
        img.paste(tile, (k * 16, 0))
    for k in range(8):                        # grass tufts
        tile = Image.new("RGBA", (8, 8), (0, 0, 0, 0))
        px = tile.load()
        for _ in range(r.randint(3, 5)):
            x0 = r.randint(1, 6)
            top = r.randint(1, 4)
            lean = r.choice([-1, 0, 0, 1])
            for yy in range(7, top - 1, -1):
                x = x0 + (lean if yy < 4 else 0)
                if 0 <= x < 8:
                    v = 255 if yy == top else (215 if yy < top + 2 else 170)
                    px[x, yy] = (v, v, v, 255)
        img.paste(tile, (k * 8, 16))
    petals = [RAMPS["flower"][i] for i in (0, 1, 2, 3, 4, 1, 2, 3)]
    for k in range(8):                        # flowers: a stem and a head of petals
        tile = Image.new("RGBA", (8, 8), (0, 0, 0, 0))
        px = tile.load()
        stem = ramp("grass", 1) + (255,)
        for yy in range(3, 8):
            px[4, yy] = stem
        px[3, 6] = stem
        pc = hexrgb(petals[k]) + (255,)
        for dx, dy in ((0, -1), (-1, 0), (1, 0), (0, 1)):
            px[4 + dx, 2 + dy] = pc
        px[4, 2] = hexrgb("#f4e070") + (255,)
        img.paste(tile, (k * 8, 24))
    for k in range(4):                        # flames
        tile = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
        px = tile.load()
        for yy in range(16):
            half = (16 - yy) / 16 * (5.5 + r.uniform(-1, 1)) * (1 if yy > 3 else 0.6)
            cx = 8 + math.sin(yy * 0.7 + k * 1.6) * 1.2
            for x in range(16):
                d = abs(x + 0.5 - cx)
                if d < half:
                    t = d / max(half, 0.1)
                    tone = 3 if t < 0.3 and yy > 6 else (2 if t < 0.6 else (1 if t < 0.85 else 0))
                    px[x, yy] = ramp("flame", tone) + (255,)
        img.paste(tile, (k * 16, 32))
    save(img, "sprites.png")


SPRITE_UV = {                                  # (u0, v0, u1, v1) in the sheet
    "leaf": [(k * 16 / 64, 0, (k + 1) * 16 / 64, 16 / 64) for k in range(4)],
    "tuft": [(k * 8 / 64, 16 / 64, (k + 1) * 8 / 64, 24 / 64) for k in range(8)],
    "flower": [(k * 8 / 64, 24 / 64, (k + 1) * 8 / 64, 32 / 64) for k in range(8)],
    "flame": [(k * 16 / 64, 32 / 64, (k + 1) * 16 / 64, 48 / 64) for k in range(4)],
}


# --- glTF writing: several primitives, each with its material's name. ---

class Mesh:
    def __init__(self):
        self.prims = {}

    def prim(self, material):
        return self.prims.setdefault(material, {"p": [], "n": [], "uv": [], "i": [], "c": []})

    def tri(self, material, a, b, c, na=None, nb=None, nc=None, uva=(0, 0), uvb=(0, 0), uvc=(0, 0)):
        """A triangle, counter-clockwise seen from its front; with no normals given it is flat-shaded."""
        pr = self.prim(material)
        a, b, c = np.asarray(a, float), np.asarray(b, float), np.asarray(c, float)
        if na is None:
            n = np.cross(b - a, c - a)
            n = n / (np.linalg.norm(n) + 1e-9)
            na = nb = nc = n
        base = len(pr["p"])
        for p, nn, uv in ((a, na, uva), (b, nb, uvb), (c, nc, uvc)):
            pr["p"].append([float(v) for v in p])
            nn = np.asarray(nn, float)
            pr["n"].append([float(v) for v in nn / (np.linalg.norm(nn) + 1e-9)])
            pr["uv"].append([float(uv[0]), float(uv[1])])
        pr["i"] += [base, base + 1, base + 2]

    def quad(self, material, a, b, c, d, n=None, uvs=((0, 1), (1, 1), (1, 0), (0, 0))):
        """a, b, c, d counter-clockwise; n, if given, is every corner's normal."""
        self.tri(material, a, b, c, n, n, n, uvs[0], uvs[1], uvs[2])
        self.tri(material, a, c, d, n, n, n, uvs[0], uvs[2], uvs[3])

    def write(self, path):
        chunks, views, accessors, prims, materials, offset = [], [], [], [], [], 0

        def add(arr, typ, comp, target, minmax=False):
            nonlocal offset
            data = arr.tobytes()
            views.append({"buffer": 0, "byteOffset": offset, "byteLength": len(data), "target": target})
            acc = {"bufferView": len(views) - 1, "componentType": comp, "count": int(arr.shape[0]), "type": typ}
            if minmax:
                acc["min"] = [float(v) for v in arr.min(0)]
                acc["max"] = [float(v) for v in arr.max(0)]
            accessors.append(acc)
            data += b"\0" * (-len(data) % 4)
            chunks.append(data)
            offset += len(data)
            return len(accessors) - 1

        for name, pr in self.prims.items():
            if not pr["i"]:
                continue
            materials.append({"name": name, "doubleSided": name in ("leaf", "leaf_pine", "leaf_dry", "flame"),
                              "pbrMetallicRoughness": {"baseColorFactor": [1, 1, 1, 1], "metallicFactor": 0,
                                                       "roughnessFactor": 1}})
            attributes = {
                "POSITION": add(np.asarray(pr["p"], np.float32), "VEC3", 5126, 34962, True),
                "NORMAL": add(np.asarray(pr["n"], np.float32), "VEC3", 5126, 34962),
                "TEXCOORD_0": add(np.asarray(pr["uv"], np.float32), "VEC2", 5126, 34962)}
            if pr["c"]:
                attributes["COLOR_0"] = add(np.asarray(pr["c"], np.float32), "VEC4", 5126, 34962)
            prims.append({"attributes": attributes,
                "indices": add(np.asarray(pr["i"], np.uint32), "SCALAR", 5125, 34963),
                "material": len(materials) - 1})
        blob_ = b"".join(chunks)
        name = os.path.splitext(os.path.basename(path))[0]
        doc = {"asset": {"version": "2.0", "generator": "kindling bake-off make_art.py"}, "scene": 0,
               "scenes": [{"nodes": [0]}], "nodes": [{"mesh": 0, "name": name}],
               "meshes": [{"name": name, "primitives": prims}], "materials": materials,
               "buffers": [{"byteLength": len(blob_)}], "bufferViews": views, "accessors": accessors}
        js = json.dumps(doc, separators=(",", ":")).encode()
        js += b" " * (-len(js) % 4)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "wb") as f:
            f.write(struct.pack("<III", 0x46546C67, 2, 12 + 8 + len(js) + 8 + len(blob_)))
            f.write(struct.pack("<II", len(js), 0x4E4F534A) + js)
            f.write(struct.pack("<II", len(blob_), 0x004E4942) + blob_)


# --- Shapes. ---

def icosphere(subdiv=1):
    t = (1 + 5 ** 0.5) / 2
    v = [(-1, t, 0), (1, t, 0), (-1, -t, 0), (1, -t, 0), (0, -1, t), (0, 1, t), (0, -1, -t), (0, 1, -t),
         (t, 0, -1), (t, 0, 1), (-t, 0, -1), (-t, 0, 1)]
    v = [np.asarray(p, float) / np.linalg.norm(p) for p in v]
    f = [(0, 11, 5), (0, 5, 1), (0, 1, 7), (0, 7, 10), (0, 10, 11), (1, 5, 9), (5, 11, 4), (11, 10, 2), (10, 7, 6),
         (7, 1, 8), (3, 9, 4), (3, 4, 2), (3, 2, 6), (3, 6, 8), (3, 8, 9), (4, 9, 5), (2, 4, 11), (6, 2, 10),
         (8, 6, 7), (9, 8, 1)]
    for _ in range(subdiv):
        cache, nf = {}, []

        def mid(a, b):
            key = (min(a, b), max(a, b))
            if key not in cache:
                m = v[a] + v[b]
                v.append(m / np.linalg.norm(m))
                cache[key] = len(v) - 1
            return cache[key]
        for a, b, c in f:
            ab, bc, ca = mid(a, b), mid(b, c), mid(c, a)
            nf += [(a, ab, ca), (b, bc, ab), (c, ca, bc), (ab, bc, ca)]
        f = nf
    return v, f


def rock(mesh, r, size, at=(0, 0, 0), material="rock", subdiv=1, rough=0.22, sink=0.12):
    """A chiselled rock (art guide, rule 8): a sphere of flat facets, pushed in and out at random, squashed to its
    size, its foot sunk into the ground."""
    v, f = icosphere(subdiv)
    pts = []
    for p in v:
        k = 1 + r.uniform(-rough, rough)
        q = p * k * np.asarray(size) * 0.5
        q[1] = max(q[1], -size[1] * 0.5 * (1 - sink * 2))
        pts.append(q + np.asarray(at) + np.array([0, size[1] * 0.5 - sink * size[1], 0]))
    for a, b, c in f:
        mesh.tri(material, pts[a], pts[c], pts[b])


def column(mesh, r, base, width, depth, height, turn):
    """A cliff column: an irregular six-sided prism, stepped twice on its way up, its top tilted (art guide, rule 8)."""
    k = 7
    rings = []
    levels = [-0.6, height * r.uniform(0.35, 0.5), height * r.uniform(0.7, 0.85), height]
    rad = [(r.uniform(0.85, 1.15)) for _ in range(k)]
    for li, y in enumerate(levels):
        shrink = [1.0, r.uniform(0.9, 1.05), r.uniform(0.85, 1.0), r.uniform(0.75, 0.9)][li]
        ring = []
        for j in range(k):
            a = turn + j / k * math.tau + r.uniform(-0.12, 0.12)
            rx = width * 0.5 * rad[j] * shrink * r.uniform(0.95, 1.05)
            rz = depth * 0.5 * rad[j] * shrink * r.uniform(0.95, 1.05)
            lx, lz = math.cos(a) * rx, math.sin(a) * rz
            x = base[0] + lx * math.cos(turn) - lz * math.sin(turn)
            z = base[2] + lx * math.sin(turn) + lz * math.cos(turn)
            ring.append(np.array([x, base[1] + y + (r.uniform(-0.25, 0.1) if li == 3 else 0), z]))
        rings.append(ring)
    for li in range(len(rings) - 1):
        lo, hi = rings[li], rings[li + 1]
        for j in range(k):
            a, b = lo[j], lo[(j + 1) % k]
            c, d = hi[(j + 1) % k], hi[j]
            mesh.tri("rock", a, c, b)
            mesh.tri("rock", a, d, c)
    top = rings[-1]
    centre = sum(top) / k + np.array([0, r.uniform(0.0, 0.2), 0])
    for j in range(k):
        mesh.tri("rock", top[j], centre, top[(j + 1) % k])


def prism(mesh, material, p0, p1, r0, r1, sides=6, uv_scale=1.0):
    """A tapered prism from p0 to p1, flat-shaded, with bark-style UVs: u round it, v along it (metres / 2)."""
    p0, p1 = np.asarray(p0, float), np.asarray(p1, float)
    axis = p1 - p0
    length = np.linalg.norm(axis)
    axis /= length
    side = np.cross(axis, [0, 0, 1]) if abs(axis[2]) < 0.9 else np.cross(axis, [1, 0, 0])
    side /= np.linalg.norm(side)
    other = np.cross(axis, side)
    circ = math.tau * (r0 + r1) / 2
    for j in range(sides):
        a0, a1 = j / sides * math.tau, (j + 1) / sides * math.tau
        d0 = side * math.cos(a0) + other * math.sin(a0)
        d1 = side * math.cos(a1) + other * math.sin(a1)
        u0, u1 = j / sides * circ, (j + 1) / sides * circ
        mesh.quad(material, p0 + d0 * r0, p0 + d1 * r0, p1 + d1 * r1, p1 + d0 * r1,
                  uvs=((u0, length / 2 * uv_scale), (u1, length / 2 * uv_scale), (u1, 0), (u0, 0)))
    return side, other


def cards(mesh, r, material, centre, radii, count, size, light_centre, sprites="leaf", up_bias=0.0):
    """Leaf cards over an ellipsoid crown (art guide, rule 7): spread evenly by a golden-angle spiral, each facing out
    from the crown with a random twist; every corner's normal points out from the whole canopy's middle, so the
    crown is lit as one round shape."""
    centre, radii = np.asarray(centre, float), np.asarray(radii, float)
    for i in range(count):
        y = 1 - 2 * (i + 0.5) / count
        y = y * (1 - up_bias) + up_bias * abs(y)
        rad = math.sqrt(max(1 - y * y, 0))
        a = i * math.pi * (3 - 5 ** 0.5) + r.uniform(-0.2, 0.2)
        d = np.array([math.cos(a) * rad, y, math.sin(a) * rad])
        p = centre + d * radii * r.uniform(0.82, 1.02)
        lc = np.asarray(light_centre)
        face = d + np.array([r.uniform(-0.4, 0.4), r.uniform(-0.2, 0.4), r.uniform(-0.4, 0.4)])
        face /= np.linalg.norm(face)
        t1 = np.cross(face, [0, 1, 0]) if abs(face[1]) < 0.95 else np.cross(face, [1, 0, 0])
        t1 /= np.linalg.norm(t1)
        t2 = np.cross(face, t1)
        tw = r.uniform(0, math.tau)
        t1, t2 = t1 * math.cos(tw) + t2 * math.sin(tw), -t1 * math.sin(tw) + t2 * math.cos(tw)
        s = size * r.uniform(0.85, 1.15) * 0.5
        u0, v0, u1, v1 = r.choice(SPRITE_UV[sprites])
        corners = [p - t1 * s - t2 * s, p + t1 * s - t2 * s, p + t1 * s + t2 * s, p - t1 * s + t2 * s]
        ns = [(c - lc) / (np.linalg.norm(c - lc) + 1e-9) for c in corners]
        uvs = ((u0, v1), (u1, v1), (u1, v0), (u0, v0))
        mesh.tri(material, corners[0], corners[1], corners[2], ns[0], ns[1], ns[2], uvs[0], uvs[1], uvs[2])
        mesh.tri(material, corners[0], corners[2], corners[3], ns[0], ns[2], ns[3], uvs[0], uvs[2], uvs[3])


# --- Models. ---

def broadleaf(seed, height=3.2):
    r = random.Random(seed)
    m = Mesh()
    top = np.array([r.uniform(-0.15, 0.15), height, r.uniform(-0.15, 0.15)])
    prism(m, "bark", (0, -0.2, 0), top, 0.22, 0.12)
    crowns = [(top + np.array([0, 0.9, 0]), np.array([1.6, 1.25, 1.6]))]
    for k in range(r.randint(3, 4)):
        a = k / 4 * math.tau + r.uniform(-0.4, 0.4)
        start = np.array([0, height * r.uniform(0.6, 0.85), 0])
        end = start + np.array([math.cos(a) * 1.3, r.uniform(0.7, 1.1), math.sin(a) * 1.3])
        prism(m, "bark", start, end, 0.09, 0.05, sides=5)
        crowns.append((end + np.array([0, 0.35, 0]), np.array([1.15, 0.95, 1.15]) * r.uniform(0.85, 1.1)))
    middle = sum(c for c, _ in crowns) / len(crowns) - np.array([0, 0.3, 0])
    for c, rad in crowns:
        cards(m, r, "leaf", c, rad, int(62 * rad[0]), 0.95, middle)
    return m


def pine(seed, height=5.0):
    r = random.Random(seed)
    m = Mesh()
    prism(m, "bark", (0, -0.2, 0), (0, height, 0), 0.18, 0.06)
    tiers = 5
    for t in range(tiers):
        y = height * (0.3 + 0.7 * t / tiers)
        rad = 1.5 * (1 - t / tiers) + 0.35
        centre = np.array([0, y, 0])
        cards(m, r, "leaf_pine", centre, (rad, 0.55, rad), int(16 + 20 * rad), 0.85, (0, y - 0.8, 0), up_bias=0.35)
    cards(m, r, "leaf_pine", (0, height + 0.2, 0), (0.3, 0.45, 0.3), 6, 0.7, (0, height - 0.5, 0))
    return m


def bush(seed):
    r = random.Random(seed)
    m = Mesh()
    w = r.uniform(0.9, 1.3)
    for k in range(r.randint(2, 3)):
        c = np.array([r.uniform(-0.35, 0.35), 0.45 + r.uniform(0, 0.15), r.uniform(-0.35, 0.35)])
        cards(m, r, "leaf", c, (w * 0.75, 0.55, w * 0.75), 30, 0.75, (0, 0.0, 0))
    return m


def rocks():
    out = {}
    for name, size, seed, sub, rough in (("boulder", (1.7, 1.1, 1.4), 1, 1, 0.24), ("slab", (2.2, 0.6, 1.6), 2, 1, 0.18),
                                         ("standing", (0.9, 1.9, 0.8), 3, 1, 0.2), ("stone", (0.45, 0.3, 0.4), 4, 0, 0.25),
                                         ("stone2", (0.35, 0.22, 0.3), 5, 0, 0.3)):
        m = Mesh()
        rock(m, random.Random(seed), size, subdiv=sub, rough=rough)
        out[name] = m
    return out


def cliff(points, seed=21):
    """The cliff: a row of columns along its line, overlapping, of varied width and height."""
    r = random.Random(seed)
    m = Mesh()
    for x, y, z, turn, foot in points:
        width = r.uniform(1.9, 3.6)
        dz = r.uniform(-0.5, 0.4)
        column(m, r, (x, foot - 0.2, z + dz), width, r.uniform(1.8, 2.8), y - foot + r.uniform(-0.6, 0.9), turn)
        if r.random() < 0.35:                  # a lower ledge in front
            column(m, r, (x + r.uniform(-0.6, 0.6), foot - 0.2, z + 1.1), width * 0.6, 1.2,
                   (y - foot) * r.uniform(0.3, 0.55), turn + r.uniform(-0.3, 0.3))
    return m


def tent(seed=31):
    """A hide tent (art guide, rule 9): a cone of stitched hides on poles that cross above its top, the door flap
    folded back."""
    r = random.Random(seed)
    m = Mesh()
    k, rad, h = 14, 1.7, 3.0
    slant = math.hypot(rad, h)
    apex = np.array([0, h, 0])
    for j in range(k):
        if j in (0,):                         # the door
            continue
        a0, a1 = j / k * math.tau, (j + 1) / k * math.tau
        p0 = np.array([math.cos(a0) * rad, 0, math.sin(a0) * rad])
        p1 = np.array([math.cos(a1) * rad, 0, math.sin(a1) * rad])
        u0, u1 = a0 * rad / 2, a1 * rad / 2
        n0 = np.array([math.cos(a0) * h, rad, math.sin(a0) * h])
        n1 = np.array([math.cos(a1) * h, rad, math.sin(a1) * h])
        m.tri("hide", p0, apex, p1, n0, (n0 + n1) / 2, n1, (u0, slant / 2), ((u0 + u1) / 2, 0), (u1, slant / 2))
    for j in range(7):                         # poles
        a = j / 7 * math.tau + 0.2
        foot = np.array([math.cos(a) * rad * 1.02, -0.1, math.sin(a) * rad * 1.02])
        tip = apex + (apex - foot) * 0.22
        prism(m, "bark", foot, tip, 0.045, 0.035, sides=4)
    return m


def fire_ring(seed=41):
    r = random.Random(seed)
    m = Mesh()
    for j in range(10):
        a = j / 10 * math.tau
        rock(m, r, (0.32, 0.22, 0.28), at=(math.cos(a) * 0.62, 0, math.sin(a) * 0.62), subdiv=0, rough=0.25)
    for j in range(4):
        a = j / 4 * math.tau + 0.4
        foot = np.array([math.cos(a) * 0.45, 0.02, math.sin(a) * 0.45])
        prism(m, "bark", foot, (0, 0.42, 0), 0.06, 0.04, sides=5)
    return m


def flames():
    """Two crossed cards of the flame sprite; the shader steps through its four frames."""
    m = Mesh()
    u0, v0, u1, v1 = SPRITE_UV["flame"][0]
    for a in (0.0, math.pi / 2):
        d = np.array([math.cos(a), 0, math.sin(a)]) * 0.32
        n = np.array([-math.sin(a), 0.3, math.cos(a)])
        m.quad("flame", -d, d, d + [0, 0.75, 0], -d + [0, 0.75, 0], n=n, uvs=((u0, v1), (u1, v1), (u1, v0), (u0, v0)))
    return m


def log_seat(seed=51):
    m = Mesh()
    length, rad, sides = 2.0, 0.22, 8
    p0, p1 = np.array([-length / 2, rad * 0.8, 0]), np.array([length / 2, rad * 0.8, 0])
    prism(m, "bark", p0, p1, rad, rad, sides=sides)
    for end, sgn in ((p0, -1), (p1, 1)):
        ring = []
        for j in range(sides):
            a = j / sides * math.tau
            ring.append((end + np.array([0, math.cos(a) * rad, math.sin(a) * rad]),
                         (0.5 + 0.5 * math.cos(a), 0.5 + 0.5 * math.sin(a))))
        for j in range(sides):
            (a, ua), (b, ub) = ring[j], ring[(j + 1) % sides]
            if sgn > 0:
                m.tri("wood_end", end, a, b, uva=(0.5, 0.5), uvb=ua, uvc=ub)
            else:
                m.tri("wood_end", end, b, a, uva=(0.5, 0.5), uvb=ub, uvc=ua)
    return m


def windbreak(seed=61):
    """A windbreak: poles leaning into a row, branches woven along them, dry leaves caught in it."""
    r = random.Random(seed)
    m = Mesh()
    for j in range(9):
        x = -1.8 + j * 0.45
        foot = np.array([x, -0.1, r.uniform(-0.05, 0.05)])
        prism(m, "bark", foot, foot + np.array([r.uniform(-0.1, 0.1), 1.5, -0.55]), 0.045, 0.03, sides=4)
    for k in range(4):
        y = 0.3 + k * 0.3
        prism(m, "bark", (-1.9, y, -0.11 * y), (1.9, y + r.uniform(-0.1, 0.1), -0.11 * y), 0.03, 0.025, sides=4)
    cards(m, r, "leaf_dry", (0, 0.8, -0.2), (1.9, 0.6, 0.15), 40, 0.45, (0, 0.6, -1.0))
    return m


def main():
    tex_rock(), tex_rock_top(), tex_bark(), tex_wood_end(), tex_hide(), tex_dirt(), tex_sprites()
    for i in range(3):
        broadleaf(100 + i, height=r_height(i)).write(os.path.join(MOD, f"broadleaf{i}.glb"))
    for i in range(2):
        pine(200 + i, height=4.6 + i).write(os.path.join(MOD, f"pine{i}.glb"))
    for i in range(3):
        bush(300 + i).write(os.path.join(MOD, f"bush{i}.glb"))
    for name, m in rocks().items():
        m.write(os.path.join(MOD, f"{name}.glb"))
    tent().write(os.path.join(MOD, "tent.glb"))
    fire_ring().write(os.path.join(MOD, "fire_ring.glb"))
    flames().write(os.path.join(MOD, "flames.glb"))
    log_seat().write(os.path.join(MOD, "log_seat.glb"))
    windbreak().write(os.path.join(MOD, "windbreak.glb"))
    print("art written:", sorted(os.listdir(TEX)), sorted(os.listdir(MOD)))


def r_height(i):
    return [3.0, 3.6, 2.7][i]


if __name__ == "__main__":
    main()
