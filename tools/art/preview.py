"""Previews of art as the game shows it (A4.2, A5.3, A8.4): the camera 37 degrees down through a 10 degree lens, 1080 x
2404 like the phone, a texture pixel about 2 x 2 screen pixels at the band's zoom, the late-afternoon sun behind the
camera's left shoulder or ahead of it. The ground is composed in numpy from the tiles' own levels, each cell of a tile
picking one of its versions by a hash of its place (A5.3), and drawn by Blender (blender_scene.py).

    python3 tools/art/preview.py ground <name> <out folder> [--bands 0 1 ...] [--sun behind|ahead] [--samples N]

Needs Blender (BLENDER sets its path) and writes <out folder>/<name>-band<k>-<sun>.png for each band asked for.
Implements PRE-20 and PRE-22, see A4.2, A5.3 and A8.4.
"""

import argparse
import json
import math
import os
import subprocess
import sys
import tomllib

import numpy as np

import textures
import tiles

SIZE = (1080, 2404)
TILT = 37.0
LENS = 10.0
BAND0_MPP = 1.0 / 128.0  # metres a screen pixel at the focus when a texture pixel of band 0 is 2 screen pixels
SUNS = ("behind", "ahead")  # the sun behind the camera's left shoulder, as the light's tuning sets it, or opposite
LIGHT = os.path.join(textures.ROOT, "data", "base", "tuning", "light.toml")  # the engine's own late afternoon
# the tiles of a big surface: its band range, from textures.TILES (first band) and A5.3 (near 0-1, middle 2-3, far 4-6)
SERVES = {"near": 2, "middle": 2, "far": 3}


def blender():
    return os.environ.get("BLENDER", "blender")


def distance(mpp, lens=LENS, width=SIZE[0]):
    """How far the camera stands from the focus, as the rig works it out: the metres across the picture over twice the
    lens's tangent."""
    return mpp * width / 2.0 / math.tan(math.radians(lens) / 2.0)


def ground_at(px, py, mpp, lens=LENS, tilt=TILT, size=SIZE):
    """The flat ground's point under a picture pixel, in metres east and north of the focus, the camera heading north
    (the rig's offset_at)."""
    st, ct = math.sin(math.radians(tilt)), math.cos(math.radians(tilt))
    per = math.tan(math.radians(lens) / 2.0) / (size[0] / 2.0)
    sx, sy = (px - size[0] / 2.0) * per, (py - size[1] / 2.0) * per
    d = distance(mpp, lens, size[0])
    dir_e, dir_n, dir_up = sx, ct - sy * st, -st - sy * ct
    along = d * st / -dir_up
    return along * dir_e, -d * ct + along * dir_n


def footprint(mpp, margin=0.0):
    """The metres the ground in the picture spans: (west, east, south, north)."""
    pts = [ground_at(x, y, mpp) for x in (0, SIZE[0]) for y in (0, SIZE[1])]
    xs, ys = [p[0] for p in pts], [p[1] for p in pts]
    return min(xs) - margin, max(xs) + margin, min(ys) - margin, max(ys) + margin


def band_of(tile_name):
    return textures.TILES[tile_name][2]


def tile_for_band(available, band):
    """The tile and level a band reads: the nearest tile whose designed levels serve it."""
    for tile in ("near", "middle", "far"):
        first = band_of(tile)
        if tile in available and first <= band < first + SERVES[tile]:
            return tile, band - first
    raise ValueError(f"no tile serves band {band}")


def pick(cx, cy, salt, count):
    """Which version a cell takes: a hash of its place (A5.3), the same wherever it is asked."""
    h = tiles.Chance((cx * 73856093) ^ (cy * 19349663) ^ (salt * 83492791) ^ 0x5BD1E995)
    h.next()
    return h.below(count)


def compose(versions, level, cell_metres, box):
    """The ground over `box` as one picture of the level's texture pixels, each cell of `cell_metres` taking a version
    of the tile by its place. Returns the picture and the metres it covers (west, east, south, north)."""
    size = versions[0][level].shape[0]
    texel = cell_metres / size
    c0, c1 = int(math.floor(box[0] / texel)), int(math.ceil(box[1] / texel))
    r0, r1 = int(math.floor(-box[3] / texel)), int(math.ceil(-box[2] / texel))  # rows run south
    out = np.zeros((r1 - r0, c1 - c0, 3), np.uint8)
    for cy in range(r0 // size, (r1 - 1) // size + 1):
        for cx in range(c0 // size, (c1 - 1) // size + 1):
            tile = versions[pick(cx, cy, 1, len(versions))][level]
            ya, yb = max(r0, cy * size), min(r1, (cy + 1) * size)
            xa, xb = max(c0, cx * size), min(c1, (cx + 1) * size)
            out[ya - r0 : yb - r0, xa - c0 : xb - c0] = tile[
                ya - cy * size : yb - cy * size, xa - cx * size : xb - cx * size
            ]
    return out, (c0 * texel, c1 * texel, -r1 * texel, -r0 * texel)


def render(config, scratch):
    """Hands a config to Blender and waits for the picture."""
    path = os.path.join(scratch, "config.json")
    with open(path, "w") as f:
        json.dump(config, f)
    done = subprocess.run(
        [
            blender(),
            "-b",
            "--factory-startup",
            "--python",
            os.path.join(os.path.dirname(os.path.abspath(__file__)), "blender_scene.py"),
            "--",
            path,
        ],
        capture_output=True,
    )
    if done.returncode != 0 or not os.path.exists(config["out"]):
        raise RuntimeError("Blender failed:\n" + done.stdout.decode()[-2000:] + done.stderr.decode()[-2000:])


def linear(hex_colour):
    """A colour written #rrggbb in sRGB, as the linear red, green and blue Blender's lights take."""
    h = hex_colour.lstrip("#")
    out = []
    for i in (0, 2, 4):
        c = int(h[i : i + 2], 16) / 255.0
        out.append(c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4)
    return out


def scene_config(out, sun, samples):
    """The light as the engine's own tuning sets it (data/base/tuning/light.toml, written once there): the sun's height
    and bearing, its colour and its energy (Godot's 1.0 is Blender's pi), the ambient light's colour and energy; the
    sun `ahead` is the one on the other side. The haze and the bounce are not drawn, a flat ground having no distance
    to speak of and nothing below it."""
    with open(LIGHT, "rb") as f:
        light = tomllib.load(f)
    percent = {k: float(light[k].rstrip("%")) / 100.0 for k in ("sun_energy", "ambient_energy")}
    turn = float(light["sun_turn"]) + (180.0 if sun == "ahead" else 0.0)
    return {
        "out": out,
        "size": list(SIZE),
        "samples": samples,
        "sun": {
            "azimuth": turn % 360.0,
            "elevation": float(light["sun_height"]),
            "colour": linear(light["sun_colour"]),
            "strength": percent["sun_energy"] * math.pi,
        },
        "sky": {"colour": linear(light["ambient_colour"]), "strength": percent["ambient_energy"]},
    }


def ground_view(name, band, sun, out_folder, samples=24):
    """One band's view of a material's ground; returns the picture's path."""
    available = textures.read_set(name)
    tile, level = tile_for_band(available, band)
    mpp = BAND0_MPP * (2**band)
    box = footprint(mpp, margin=2 * (2**band) / 64.0)
    versions = [levels for levels in available[tile]]
    picture, covers = compose(versions, level, textures.TILES[tile][1], box)
    os.makedirs(out_folder, exist_ok=True)
    scratch = os.path.join(out_folder, "scratch")
    os.makedirs(scratch, exist_ok=True)
    ground_png = os.path.join(scratch, f"{name}-ground-band{band}.png")
    tiles.write_png(ground_png, picture)
    out = os.path.join(out_folder, f"{name}-band{band}-{sun}.png")
    config = scene_config(out, sun, samples)
    config["camera"] = {"distance": distance(mpp), "tilt": TILT, "lens": LENS}
    config["ground"] = {"picture": ground_png, "x0": covers[0], "x1": covers[1], "y0": covers[2], "y1": covers[3]}
    render(config, scratch)
    return out


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("what", choices=["ground"])
    ap.add_argument("name")
    ap.add_argument("out")
    ap.add_argument("--bands", type=int, nargs="*", default=[0])
    ap.add_argument("--sun", choices=sorted(SUNS), default="behind")
    ap.add_argument("--samples", type=int, default=24)
    args = ap.parse_args(argv[1:])
    for band in args.bands:
        print(ground_view(args.name, band, args.sun, args.out, args.samples))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
