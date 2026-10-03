#!/usr/bin/env python3
"""The bake-off's shared scene (research/03-rendering.md, research/04-art.md): the art from make_art.py, a ground
mesh whose colours carry each point's patch and surface, the stream's water, where each model stands, the grass tufts
and flowers, and the palette. Written to assets/ so that Godot and Bevy draw exactly the same thing. A throwaway
prototype: deterministic, numpy and Pillow only.

    python3 prototypes/bakeoff/make_scene.py
"""
import json
import math
import os
import random
import shutil

import numpy as np

import make_art
from make_art import RAMPS, Mesh

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "assets")
SIZE, STEP = 72.0, 0.5                      # metres across, metres between ground points
N = int(SIZE / STEP) + 1                    # 145 points a side
HALF = SIZE / 2
RNG = random.Random(7)
SURFACES = ["grass", "path", "bank", "bed", "rock"]    # the ground colour's second channel: index / 4


def linear(hexcode):
    c = [int(hexcode[i:i + 2], 16) / 255 for i in (1, 3, 5)]
    return [x / 12.92 if x <= 0.04045 else ((x + 0.055) / 1.055) ** 2.4 for x in c]


def fbm(x, z, seed, scale, octaves=3):
    """Smooth noise from sums of rotated sines: cheap, deterministic, good enough for a meadow."""
    r = random.Random(seed)
    total, amp, norm = 0.0, 1.0, 0.0
    for o in range(octaves):
        for _ in range(3):
            a = r.uniform(0, math.tau)
            f = (2 ** o) / scale
            total = total + amp * np.sin((x * math.cos(a) + z * math.sin(a)) * f * math.tau + r.uniform(0, math.tau))
            norm += amp
        amp *= 0.5
    return total / norm


def cliff_z(x):
    """The cliff's line: the plateau lies north of it (smaller z)."""
    return -7.0 + 2.5 * np.sin(x / 6.0) + 1.0 * np.sin(x / 2.3 + 1.0)


def stream_x(z):
    """The stream's middle, from its spring at the cliff's foot to the south edge."""
    return 6.0 + 3.0 * np.sin((z + 7.0) / 7.0) - 0.12 * (z + 7.0)


PATH = [(-7.0, 2.0), (-4.0, 4.5), (-0.5, 6.0), (3.0, 7.0), (6.0, 7.5), (12.0, 10.0), (20.0, 13.5), (37.0, 16.0)]


def path_distance(x, z):
    best = np.full(np.shape(x), 1e9)
    for (ax, az), (bx, bz) in zip(PATH, PATH[1:]):
        dx, dz = bx - ax, bz - az
        t = np.clip(((x - ax) * dx + (z - az) * dz) / (dx * dx + dz * dz), 0, 1)
        best = np.minimum(best, np.hypot(x - (ax + t * dx), z - (az + t * dz)))
    return best


def stream_half_width(z):
    z0 = cliff_z(stream_x(z))
    return 1.3 + 1.2 * np.exp(-np.maximum(z - z0, 0) / 2.0)


def meadow(x, z):
    return 0.45 * fbm(x, z, 1, 14.0) + 0.12 * fbm(x, z, 2, 4.0) - 0.035 * z


def height(x, z):
    """Ground height (metres): a meadow sloping south, a plateau 3.2 m higher north of the cliff, and the stream's
    channel cut into the meadow."""
    plateau = np.clip((cliff_z(x) - z) / 0.9, 0, 1)
    plateau = plateau * plateau * (3 - 2 * plateau)
    h = meadow(x, z) + 3.2 * plateau
    d = np.abs(x - stream_x(z))
    w = stream_half_width(z)
    south = z > cliff_z(stream_x(z)) - 0.5
    cut = np.where((d < w + 0.6) & south, 0.75 * np.clip(1 - (d / (w + 0.6)) ** 2, 0, 1), 0.0)
    return h - cut


def water_level(z):
    return float(meadow(stream_x(z), z)) - 0.75 + 0.42


def surface(x, z, slope):
    d = abs(x - float(stream_x(z)))
    w = float(stream_half_width(z))
    south = z > float(cliff_z(stream_x(z))) - 0.5
    if slope > 0.9:
        return "rock"
    if south and d < w * 0.8:
        return "bed"
    if south and d < w + 0.8:
        return "bank"
    if float(path_distance(x, z)) < 0.7 + 0.25 * float(fbm(np.array(x), np.array(z), 9, 3.0)):
        return "path"
    return "grass"


def patch(x, z):
    """The ground's soft colour patches (art guide, rule 6): a slow noise, 0 to 1, that the shader cuts into three
    greens with a fine pattern only where two meet."""
    return float(np.clip(0.5 + 0.9 * fbm(np.array(x), np.array(z), 3, 11.0), 0, 1))


def ground():
    xs = np.linspace(-HALF, HALF, N)
    X, Z = np.meshgrid(xs, xs)
    H = height(X, Z)
    gz, gx = np.gradient(H, STEP)
    normals = np.dstack([-gx, np.ones_like(H), -gz])
    normals /= np.linalg.norm(normals, axis=2, keepdims=True)
    slope = np.hypot(gx, gz)
    m = Mesh()
    pr = m.prim("ground")
    names = {}
    for r in range(N):
        for c in range(N):
            x, z = float(X[r, c]), float(Z[r, c])
            s = surface(x, z, float(slope[r, c]))
            names[(r, c)] = s
            pr["p"].append([x, float(H[r, c]), z])
            pr["n"].append([float(v) for v in normals[r, c]])
            pr["uv"].append([patch(x, z), 0.0])
            # One weight a surface besides grass, so the shader picks the strongest and never passes through others.
            pr["c"].append([1.0 if s == k else 0.0 for k in ("path", "bank", "bed", "rock")])
    for r in range(N - 1):
        for c in range(N - 1):
            a, b, d, e = r * N + c, r * N + c + 1, (r + 1) * N + c, (r + 1) * N + c + 1
            pr["i"] += [a, d, b, b, d, e]
    m.write(os.path.join(OUT, "ground.glb"))
    return H, slope, names


def water():
    z0 = float(cliff_z(stream_x(np.array(-7.0))))
    zs = np.arange(z0 + 0.3, HALF + 0.01, 0.5)
    m = Mesh()
    pr = m.prim("water")
    for i, z in enumerate(zs):
        xc = float(stream_x(z))
        w = float(stream_half_width(z)) * 0.95
        y = water_level(z)
        for x in (xc - w, xc + w):
            pr["p"].append([x, y, float(z)])
            pr["n"].append([0.0, 1.0, 0.0])
            pr["uv"].append([0.0, 0.0])
        if i:
            a, b, d, e = 2 * i - 2, 2 * i - 1, 2 * i, 2 * i + 1
            pr["i"] += [a, d, b, b, d, e]
    m.write(os.path.join(OUT, "water.glb"))


def placements(H):
    def h_at(x, z):
        return float(height(np.array(x), np.array(z)))

    out = []

    def put(model, x, z, scale=1.0, turn=None, sink=0.0, radius=0.0):
        out.append({"model": model, "x": round(x, 3), "y": round(h_at(x, z) - sink, 3), "z": round(z, 3),
                    "turn": round(RNG.uniform(0, math.tau) if turn is None else turn, 4), "scale": scale,
                    "radius": radius})

    # The cliff: one mesh of columns along its line, laid out here and built by make_art.
    pts, x = [], -HALF - 2.0
    while x < HALF + 2.0:
        zf = float(cliff_z(x)) + 0.2
        k = float(2.5 / 6.0 * math.cos(x / 6.0) + 1.0 / 2.3 * math.cos(x / 2.3 + 1.0))
        top = h_at(x, zf - 2.5)
        foot = h_at(x, zf + 1.2)
        pts.append((x, top, zf - 0.9, -math.atan(k), foot))
        x += RNG.uniform(1.7, 2.4)
    make_art.cliff(pts).write(os.path.join(OUT, "models", "cliff.glb"))
    out.append({"model": "cliff", "x": 0, "y": 0, "z": 0, "turn": 0, "scale": 1, "radius": 0})
    for x, top, z, turn, foot in pts:          # scree and fallen stones at its foot
        if RNG.random() < 0.7:
            fx = x + RNG.uniform(-1, 1)
            fz = float(cliff_z(fx)) + RNG.uniform(1.4, 3.2)
            if abs(fx - float(stream_x(fz))) > 2.6:
                put(RNG.choice(["stone", "stone2", "stone"]), fx, fz, RNG.uniform(0.9, 1.6), radius=0.35)
    # Trees: broadleaves and pines on the plateau and round the meadow, away from the stream and the path.
    spots = [(-16, -14), (-6, -13), (3, -15), (12, -12), (19, -16), (-21, -3), (-19, 9), (-14, 16), (17, 3),
             (21, 19), (-3, 20), (-23, 21), (-10, -20), (8, -21)]
    for _ in range(46):
        a = RNG.uniform(0, math.tau)
        rr = RNG.uniform(24, 34)
        spots.append((rr * math.cos(a), rr * math.sin(a)))
    for tx, tz in spots:
        if abs(tx - float(stream_x(np.array(tz)))) < 4 or float(path_distance(np.array(tx), np.array(tz))) < 3:
            continue
        model = RNG.choice(["broadleaf0", "broadleaf1", "broadleaf2", "broadleaf0", "pine0", "pine1"])
        put(model, tx, tz, RNG.uniform(1.0, 1.35), radius=1.2)
        if RNG.random() < 0.6:
            put(RNG.choice(["bush0", "bush1", "bush2"]), tx + RNG.uniform(-3, 3), tz + RNG.uniform(-3, 3),
                RNG.uniform(0.9, 1.4), radius=0.8)
    for model, x, z, s in (("boulder", -11, 10, 1.3), ("slab", 2.5, 2.5, 1.0), ("boulder", 14, 8, 1.0),
                           ("standing", -12, -11, 1.1), ("standing", 7, -11, 0.9), ("slab", -2, -16, 1.2),
                           ("stone", 10.5, 11.5, 1.6), ("stone2", 4.0, 8.5, 1.8), ("boulder", -4, 12, 0.8)):
        put(model, x, z, s, radius=1.0)
    # The camp at the foot of the cliff: a hide tent, a fire ring with its flames, a log to sit on, a windbreak.
    put("tent", -9.0, -0.5, 1.0, turn=math.radians(200), radius=2.0)
    put("fire_ring", -5.5, 2.5, 1.0, turn=0.3, radius=1.0)
    put("flames", -5.5, 2.5, 1.0, turn=0.0)
    put("log_seat", -3.9, 3.9, 1.0, turn=math.radians(-35), radius=1.1)
    put("windbreak", -6.0, -1.5, 1.0, turn=math.radians(10), radius=2.0)
    return out


def grass(H, names, placed):
    """Tufts and flowers (art guide, rule 6): scattered in clumps rather than a carpet, each tuft a shade lighter than
    the green under it, flowers in patches. float32 (x, y, z, r, g, b, scale, sprite + phase / tau) each."""
    rows = []
    blockers = [(p["x"], p["z"], p["radius"]) for p in placed if p["radius"] > 0]
    step = 0.55
    for z in np.arange(-HALF + 0.3, HALF - 0.3, step):
        for x in np.arange(-HALF + 0.3, HALF - 0.3, step):
            jx, jz = x + RNG.uniform(-0.25, 0.25), z + RNG.uniform(-0.25, 0.25)
            r, c = int(round((jz + HALF) / STEP)), int(round((jx + HALF) / STEP))
            if names[(min(max(r, 0), N - 1), min(max(c, 0), N - 1))] != "grass":
                continue
            clump = float(fbm(np.array(jx), np.array(jz), 5, 2.5))
            if RNG.random() > 0.3 + 0.55 * max(clump, 0):
                continue
            if any((jx - bx) ** 2 + (jz - bz) ** 2 < br * br for bx, bz, br in blockers):
                continue
            y = float(height(np.array(jx), np.array(jz)))
            flowers = float(fbm(np.array(jx), np.array(jz), 6, 6.0))
            phase = RNG.random() * 0.99
            if flowers > 0.45 and RNG.random() < 0.45:
                sprite = 8 + RNG.choice([0, 0, 1, 2, 3, 4, 5]) if flowers > 0.6 else 8 + RNG.randrange(8)
                rows.append((jx, y, jz, 1.0, 1.0, 1.0, RNG.uniform(0.8, 1.0), sprite + phase))
                continue
            tone = min(int(patch(jx, jz) * 3), 2) + 2
            col = linear(RAMPS["grass"][min(tone, 4)])
            rows.append((jx, y, jz, *col, RNG.uniform(0.85, 1.2), RNG.randrange(8) + phase))
    arr = np.asarray(rows, np.float32)
    with open(os.path.join(OUT, "grass.bin"), "wb") as f:
        f.write(arr.tobytes())
    return len(rows)


def main():
    if os.path.isdir(OUT):
        shutil.rmtree(OUT)
    os.makedirs(os.path.join(OUT, "models"))
    make_art.main()
    H, slope, names = ground()
    water()
    placed = placements(H)
    n = grass(H, names, placed)
    sun_el, sun_az = math.radians(38), math.radians(230)
    ramp = {k: [linear(c) for c in v] for k, v in RAMPS.items()}
    scene = {
        "about": "Kindling engine bake-off scene (research/03-rendering.md, research/04-art.md); all art made by code.",
        "size_m": SIZE, "art_pixel_screen_px": 4, "texture_px_per_m": make_art.PPM,
        "camera": {"target": [-3.0, 0.5, 2.0], "yaw_deg": 20.0, "pitch_deg": 40.0, "view_width_m": 21.0,
                   "min_width_m": 6.0, "max_width_m": 48.0},
        "sun": {"direction_to_sun": [round(math.cos(sun_el) * math.sin(sun_az), 4), round(math.sin(sun_el), 4),
                                     round(-math.cos(sun_el) * math.cos(sun_az), 4)]},
        "light": {"sun": "#fff0d2", "sky": "#a8c8e8", "ambient": "#7c86b8", "steps": [0.12, 0.5]},
        "ramps": ramp,
        "ground": {"file": "ground.glb", "patch": "UV.x", "weights": "COLOR = (path, bank, bed, rock); grass is the rest"},
        "materials": {
            "rock": {"shader": "rock", "side": "textures/rock.png", "top": "textures/rock_top.png"},
            "bark": {"shader": "textured", "texture": "textures/bark.png", "uv_metres": [1.0, 2.0]},
            "hide": {"shader": "textured", "texture": "textures/hide.png", "uv_metres": [2.0, 2.0]},
            "wood_end": {"shader": "textured", "texture": "textures/wood_end.png", "uv_metres": [1.0, 1.0]},
            "leaf": {"shader": "leaf", "ramp": "leaf"}, "leaf_pine": {"shader": "leaf", "ramp": "pine"},
            "leaf_dry": {"shader": "leaf", "ramp": "hide"}, "flame": {"shader": "flame"},
        },
        "sprites": "textures/sprites.png", "dirt": "textures/dirt.png",
        "grass": {"file": "grass.bin", "floats_per_card": 8, "count": n, "height_m": 0.5,
                  "note": "sprite 0-7 tufts tinted by r,g,b; 8-15 flowers in their own colours"},
        "water": {"file": "water.glb", "colours": {"deep": "#2a6f8a", "shallow": "#46a6b2", "foam": "#e6f6ee"}},
        "models": placed,
    }
    with open(os.path.join(OUT, "scene.json"), "w") as f:
        json.dump(scene, f, indent=1)
    print(f"ground {N}x{N} points, {len(placed)} models, {n} tufts and flowers")


if __name__ == "__main__":
    main()
