"""Previews of art as the game shows it (A4.2, A5.3, A8.4): the camera 37 degrees down through a 10 degree lens, 1080 x
2404 like the phone, a texture pixel about 2 x 2 screen pixels at the band's zoom, the late-afternoon sun behind the
camera's left shoulder or ahead of it. The ground is composed in numpy from the tiles' own levels, each cell of a tile
picking one of its versions by a hash of its place (A5.3), and drawn by Blender (blender_scene.py).

    python3 tools/art/preview.py ground <name> <out folder> [--bands 0 1 ...] [--sun behind|ahead] [--samples N]
                                       [--beside <other material>]
    python3 tools/art/preview.py wall <material> <out folder> [--ground <name>] [--bands ...] [--sun behind|ahead]
    python3 tools/art/preview.py cliff <foot recipe> <out folder> [--ground <name>] [--bands ...] [--sun behind|ahead]
    python3 tools/art/preview.py water <bed> <out folder> [--marks <marks>] [--depth m] [--bands ...]
    python3 tools/art/preview.py part <recipe> <out folder> [--ground <name>] [--forms <part>...] [--turn <degrees>]
                                      [--bands ...]

`wall` lays a material on a vertical wall (a cliff's face) with a ground below it; `cliff` puts the camp's cliff
together, limestone over shale with its lip between them and a foot recipe's boulders and scree at its foot. `water`
draws a river of one depth: the bed tinted by the water over it and the marks' picture on top. `part` lays a recipe's
parts (art/models/<recipe>/record.toml) on a ground, each role wearing its texture; a recipe with a ring, a span or a
plug is put together in Blender by the assembler's rules, with the preview's own strays. `--light` and `--water` try
other numbers for the light and the water without touching the tuning.

Needs Blender (BLENDER sets its path) and writes <out folder>/<name>-band<k>-<sun>.png for each band asked for.
Implements PRE-20, PRE-22, PRE-26 and PRE-46, see A4.2, A4.5, A5.3 and A8.4.
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


def compose(versions, level, cell_metres, box, salt=1):
    """The ground over `box` as one picture of the level's texture pixels (RGB, or RGBA for the water's marks), each
    cell of `cell_metres` taking a version of the tile by its place. Returns the picture and the metres it covers (west,
    east, south, north)."""
    size = versions[0][level].shape[0]
    texel = cell_metres / size
    c0, c1 = int(math.floor(box[0] / texel)), int(math.ceil(box[1] / texel))
    r0, r1 = int(math.floor(-box[3] / texel)), int(math.ceil(-box[2] / texel))  # rows run south
    out = np.zeros((r1 - r0, c1 - c0, versions[0][level].shape[2]), np.uint8)
    for cy in range(r0 // size, (r1 - 1) // size + 1):
        for cx in range(c0 // size, (c1 - 1) // size + 1):
            tile = versions[pick(cx, cy, salt, len(versions))][level]
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


def scene_config(out, sun, samples, change=None):
    """The light as the engine's own tuning sets it (data/base/tuning/light.toml, written once there): the sun's height
    and bearing, its colour and its energy (Godot's 1.0 is Blender's pi), the ambient light's colour and energy; the
    sun `ahead` is the one on the other side. The haze and the bounce are not drawn, a flat ground having no distance
    to speak of and nothing below it. `change` (a dict of the tuning's own keys and values) tries other numbers for a
    preview, as the builder may try them, without touching the tuning."""
    with open(LIGHT, "rb") as f:
        light = tomllib.load(f)
    light.update(change or {})
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


def ground_view(name, band, sun, out_folder, samples=24, change=None, suffix="", beside=None):
    """One band's view of a material's ground; returns the picture's path. With `beside` (another material's name) the
    frame is a chessboard of the two, each in two of its four quarters, so the joins between them and what each looks
    like beside the other show."""
    available = textures.read_set(name)
    tile, level = tile_for_band(available, band)
    mpp = BAND0_MPP * (2**band)
    box = footprint(mpp, margin=2 * (2**band) / 64.0)
    versions = [levels for levels in available[tile]]
    picture, covers = compose(versions, level, textures.TILES[tile][1], box)
    if beside:
        other_set = textures.read_set(beside)
        other_tile, other_level = tile_for_band(other_set, band)
        other, _ = compose(other_set[other_tile], other_level, textures.TILES[other_tile][1], box)
        h, w = picture.shape[:2]
        swap = np.zeros((h, w), bool)
        swap[: h // 2, w // 2 :] = True
        swap[h // 2 :, : w // 2] = True
        picture = np.where(swap[..., None], other, picture)
        name = f"{name}-beside-{beside}"
    os.makedirs(out_folder, exist_ok=True)
    scratch = os.path.join(out_folder, "scratch")
    os.makedirs(scratch, exist_ok=True)
    ground_png = os.path.join(scratch, f"{name}-ground-band{band}.png")
    tiles.write_png(ground_png, picture)
    out = os.path.join(out_folder, f"{name}-band{band}-{sun}{suffix}.png")
    config = scene_config(out, sun, samples, change)
    config["camera"] = {"distance": distance(mpp), "tilt": TILT, "lens": LENS}
    config["ground"] = {"picture": ground_png, "x0": covers[0], "x1": covers[1], "y0": covers[2], "y1": covers[3]}
    render(config, scratch)
    return out


def wall_view(name, ground_name, band, sun, out_folder, samples=24, change=None, suffix=""):
    """One band's view of a material laid on a wall, a cliff's face: a vertical wall standing on the frame's middle
    line, facing the camera, its tiles laid as the ground's are (each cell taking a version by its place) over metres
    east and up, with the ground below it (`ground_name`'s material, the meadow by default), under the engine's light.
    Returns the picture's path."""
    available = textures.read_set(name)
    tile, level = tile_for_band(available, band)
    mpp = BAND0_MPP * (2**band)
    box = footprint(mpp, margin=2 * (2**band) / 64.0)
    height = 1.25 * (SIZE[1] / 2.0) * mpp / math.cos(math.radians(TILT))
    face, covers = compose(available[tile], level, textures.TILES[tile][1], (box[0], box[1], 0.0, height))
    ground_set = textures.read_set(ground_name)
    ground_tile, ground_level = tile_for_band(ground_set, band)
    floor, floor_covers = compose(ground_set[ground_tile], ground_level, textures.TILES[ground_tile][1], box)
    os.makedirs(out_folder, exist_ok=True)
    scratch = os.path.join(out_folder, "scratch")
    os.makedirs(scratch, exist_ok=True)
    wall_png = os.path.join(scratch, f"{name}-wall-band{band}.png")
    ground_png = os.path.join(scratch, f"{ground_name}-ground-band{band}.png")
    tiles.write_png(wall_png, face)
    tiles.write_png(ground_png, floor)
    out = os.path.join(out_folder, f"{name}-wall-band{band}-{sun}{suffix}.png")
    config = scene_config(out, sun, samples, change)
    config["camera"] = {"distance": distance(mpp), "tilt": TILT, "lens": LENS}
    config["ground"] = {
        "picture": ground_png,
        "x0": floor_covers[0],
        "x1": floor_covers[1],
        "y0": floor_covers[2],
        "y1": floor_covers[3],
    }
    config["wall"] = {"picture": wall_png, "x0": covers[0], "x1": covers[1], "z0": 0.0, "z1": covers[3]}
    render(config, scratch)
    return out


CLIFF_FORMS = {
    0: "",
    1: "",
    2: "_simple",
}  # which form of the cliff's parts each band draws (A6.3), the small one after
CLIFF_CONTACT = 1.6  # metres up the cliff where the limestone's lip stands over the shale
CLIFF_SETBACK = 0.5  # how far the shale has weathered back under it, in metres


def cliff_view(foot, ground_name, band, sun, out_folder, samples=24, change=None, suffix=""):
    """The camp's cliff as the plan lays it (limestone over shale, the shale weathered back under the limestone's lip,
    boulders and scree and flakes at its foot): the shale wall standing `CLIFF_SETBACK` behind the line and the
    limestone above it on the line, each its own rock's tiles under the engine's hash (every cell one of four
    versions, each layer moved over by a constant of its own so the beds of the two never line up), the limestone's
    lip laid end to end along the contact, and the recipe `foot` (put together as the assembler does) over
    `ground_name`, in the form each part has at the band. Returns the picture's path."""
    form = CLIFF_FORMS.get(band, "_small")
    parts_suffix = "_marker" if form == "_small" else form
    with open(os.path.join(textures.ROOT, "art", "models", foot + form, "record.toml"), "rb") as f:
        record = tomllib.load(f)
    blend = os.path.join(textures.ROOT, "art", "models", record["family"] + ".blend")
    roles = {}
    for material in record["material"]:
        where = material["textures"][0].split(":", 1)[1]
        texture, levels = textures.read_texture("art/textures/" + where)
        table, _ = levels[min(band, len(levels) - 1)]
        roles[material["role"]] = {
            "picture": os.path.join(textures.ROOT, table["file"]),
            "metres": texture["tile_texels"] / texture["texels_a_metre"],
        }
    mpp = BAND0_MPP * (2**band)
    box = footprint(mpp, margin=2 * (2**band) / 64.0)
    top = 1.25 * (SIZE[1] / 2.0) * mpp / math.cos(math.radians(TILT))
    os.makedirs(out_folder, exist_ok=True)
    scratch = os.path.join(out_folder, "scratch")
    os.makedirs(scratch, exist_ok=True)

    def laid(name, z0, z1, y, shift):
        """A rock's wall from z0 to z1 as a picture, its tiles moved over by `shift` (metres east, metres up)."""
        available = textures.read_set(name)
        tile, level = tile_for_band(available, band)
        moved = (box[0] + shift[0], box[1] + shift[0], z0 + shift[1], z1 + shift[1])
        face, covers = compose(available[tile], level, textures.TILES[tile][1], moved)
        path = os.path.join(scratch, f"{name}-cliff-band{band}.png")
        tiles.write_png(path, face)
        return {
            "picture": path,
            "x0": covers[0] - shift[0],
            "x1": covers[1] - shift[0],
            "z0": covers[2] - shift[1],
            "z1": covers[3] - shift[1],
            "y": y,
        }

    walls = [
        laid("shale", 0.0, CLIFF_CONTACT - 0.3, CLIFF_SETBACK, (1.7, 0.9)),
        laid("limestone", CLIFF_CONTACT, top, 0.0, (0.0, 0.0)),
    ]
    available = textures.read_set(ground_name)
    tile, level = tile_for_band(available, band)
    picture, covers = compose(available[tile], level, textures.TILES[tile][1], box)
    ground_png = os.path.join(scratch, f"{ground_name}-ground-band{band}.png")
    tiles.write_png(ground_png, picture)
    out = os.path.join(out_folder, f"cliff-band{band}-{sun}{suffix}.png")
    config = scene_config(out, sun, samples, change)
    config["camera"] = {"distance": distance(mpp), "tilt": TILT, "lens": LENS}
    config["ground"] = {"picture": ground_png, "x0": covers[0], "x1": covers[1], "y0": covers[2], "y1": covers[3]}
    config["walls"] = walls
    config["roles"] = roles
    lips = int(math.ceil((box[1] - box[0]) / 4.0)) + 1
    first = -4.0 * (lips // 2)
    config["parts"] = [{"file": blend, "at": [0.0, 0.0, 0.0]}] + [
        {"file": blend, "only": ["cliff_lip" + parts_suffix], "at": [first + 4.0 * k, 0.0, CLIFF_CONTACT], "keep": True}
        for k in range(lips)
    ]
    config["assemble"] = {"places": [place_in_metres(place) for place in record["place"]], "turn": 0.0}
    render(config, scratch)
    return out


WATER = os.path.join(textures.ROOT, "data", "base", "tuning", "water.toml")


def to_linear(srgb):
    """8-bit sRGB pixels as linear light, floats from 0 to 1."""
    c = np.asarray(srgb, np.float64) / 255.0
    return np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)


def to_srgb(lin):
    """Linear light as 8-bit sRGB pixels."""
    c = np.clip(lin, 0.0, 1.0)
    return np.rint(255.0 * np.where(c <= 0.0031308, c * 12.92, 1.055 * c ** (1 / 2.4) - 0.055)).astype(np.uint8)


def water_tuning(change=None):
    """The water's numbers as the engine's tuning gives them (data/base/tuning/water.toml): how fast each colour fades
    in a metre of water and how fast the water's own deep colour takes over, the deep colour in linear light, and the
    share of the sky the surface mirrors. `change` tries other values (the tuning's own keys and text)."""
    with open(WATER, "rb") as f:
        tune = tomllib.load(f)
    tune.update(change or {})

    def metres(text):
        return float(text.split()[0])

    return {
        "absorb": np.array([1.0 / metres(tune[k]) for k in ("fade_red", "fade_green", "fade_blue")]),
        "murk": 1.0 / metres(tune["murk"]),
        "deep": np.array(linear(tune["deep_colour"])),
        "sky": float(tune["sky_share"].rstrip("%")) / 100.0,
    }


def water_over(bed, marks, depth, tune, sky=(0.72, 0.78, 0.83)):
    """The bed seen through `depth` metres of water with the marks over it, as the engine draws them (A4.5, the
    ground's shader tints the bed by the depth, red first, and the water's deep colour takes over; the surface carries
    the marks and mirrors a little sky), in 8-bit sRGB: `bed` RGB pixels, `marks` RGBA pixels."""
    lin = to_linear(bed)
    through = lin * np.exp(-depth * tune["absorb"]) + tune["deep"] * (1.0 - math.exp(-depth * tune["murk"]))
    alpha = marks[..., 3:4].astype(np.float64) / 255.0
    surface = through * (1.0 - alpha) + to_linear(marks[..., :3]) * alpha
    surface = surface * (1.0 - tune["sky"]) + to_linear(np.array(sky) * 255.0) * tune["sky"]
    return to_srgb(surface)


def water_view(bed, marks, band, sun, out_folder, depth, samples=24, change=None, suffix="", water=None):
    """One band's view of a river of one depth: the bed's ground tinted by the water over it and the marks' picture on
    top, lit as the ground is; returns the picture's path."""
    beds, flows = textures.read_set(bed), textures.read_set(marks)
    tile, level = tile_for_band(beds, band)
    mark_tile, mark_level = tile_for_band(flows, band)
    mpp = BAND0_MPP * (2**band)
    box = footprint(mpp, margin=2 * (2**band) / 64.0)
    ground, covers = compose(beds[tile], level, textures.TILES[tile][1], box)
    over, covers_marks = compose(flows[mark_tile], mark_level, textures.TILES[mark_tile][1], box, salt=2)
    if covers != covers_marks:
        raise RuntimeError("the bed's and the marks' tiles do not cover the same ground")
    picture = water_over(ground, over, depth, water_tuning(water))
    os.makedirs(out_folder, exist_ok=True)
    scratch = os.path.join(out_folder, "scratch")
    os.makedirs(scratch, exist_ok=True)
    ground_png = os.path.join(scratch, f"water-ground-band{band}.png")
    tiles.write_png(ground_png, picture)
    out = os.path.join(out_folder, f"water-{bed}-band{band}-{sun}-{int(depth * 100)}cm{suffix}.png")
    config = scene_config(out, sun, samples, change)
    config["camera"] = {"distance": distance(mpp), "tilt": TILT, "lens": LENS}
    config["ground"] = {"picture": ground_png, "x0": covers[0], "x1": covers[1], "y0": covers[2], "y1": covers[3]}
    render(config, scratch)
    return out


def metres_of(text):
    """A length written in a recipe, such as "50 mm" or "1.2 m", in metres."""
    value, unit = text.split()
    return float(value) * {"mm": 0.001, "cm": 0.01, "m": 1.0}[unit]


def place_in_metres(place):
    """A recipe's placement for Blender: its lengths in metres ("1.75 m", "60 mm"), its shares as fractions ("12%"),
    its angles as they are."""
    out = dict(place)
    for key, value in place.items():
        if key in ("radius", "height", "base"):
            out[key] = metres_of(value)
        elif key in ("jitter_radius", "jitter_size"):
            out[key] = float(value.rstrip("%")) / 100.0
    return out


def part_view(recipe, ground_name, band, sun, out_folder, samples=24, change=None, suffix="", forms=None, turn=0.0):
    """A recipe's parts lying on a ground as the game shows them at a band: the family's Blender file brought into the
    scene, each role wearing its texture's level for the band (read nearest-pixel through the part's own texture
    coordinates in metres); returns the picture's path. A recipe of root places only has each part turned and lifted
    as its place says, its middle over the focus; a recipe with a ring, a span or a plug is put together in Blender by
    the engine assembler's own rules (blender_scene.assemble), the parts where the recipe puts them."""
    with open(os.path.join(textures.ROOT, "art", "models", recipe, "record.toml"), "rb") as f:
        record = tomllib.load(f)
    blend = os.path.join(textures.ROOT, "art", "models", record["family"] + ".blend")
    roles = {}
    for material in record["material"]:
        where = material["textures"][0].split(":", 1)[1]
        texture, levels = textures.read_texture("art/textures/" + where)
        table, _ = levels[min(band, len(levels) - 1)]
        roles[material["role"]] = {
            "picture": os.path.join(textures.ROOT, table["file"]),
            "metres": texture["tile_texels"] / texture["texels_a_metre"],
        }
    items = []
    put_together = any(place["rule"] != "root" for place in record["place"])
    if put_together:  # the whole family comes in at the origin and the places are worked out in Blender
        items.append({"file": blend, "at": [0.0, 0.0, 0.0]})
    for place in [] if put_together else record["place"]:
        for part in forms or place["parts"]:
            items.append(
                {
                    "file": blend,
                    "only": [part],
                    "turn": float(place.get("turn", 0.0)),
                    "at": [0.0, 0.0, metres_of(place["height"]) if "height" in place else 0.0],
                    "centre": True,
                }
            )
    available = textures.read_set(ground_name)
    tile, level = tile_for_band(available, band)
    mpp = BAND0_MPP * (2**band)
    box = footprint(mpp, margin=2 * (2**band) / 64.0)
    picture, covers = compose(available[tile], level, textures.TILES[tile][1], box)
    os.makedirs(out_folder, exist_ok=True)
    scratch = os.path.join(out_folder, "scratch")
    os.makedirs(scratch, exist_ok=True)
    ground_png = os.path.join(scratch, f"{ground_name}-ground-band{band}.png")
    tiles.write_png(ground_png, picture)
    out = os.path.join(out_folder, f"{recipe}-band{band}-{sun}{suffix}.png")
    config = scene_config(out, sun, samples, change)
    config["camera"] = {"distance": distance(mpp), "tilt": TILT, "lens": LENS}
    config["ground"] = {"picture": ground_png, "x0": covers[0], "x1": covers[1], "y0": covers[2], "y1": covers[3]}
    config["roles"] = roles
    config["parts"] = items
    if put_together:
        config["assemble"] = {"places": [place_in_metres(place) for place in record["place"]], "turn": turn}
    render(config, scratch)
    return out


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("what", choices=["ground", "water", "part", "wall", "cliff"])
    ap.add_argument("--ground", default="meadow", help="the ground a part lies on (part)")
    ap.add_argument("--beside", default=None, help="another material to lay beside this one in a chessboard (ground)")
    ap.add_argument("--forms", nargs="*", default=None, help="the parts to lay instead of the recipe's own (part)")
    ap.add_argument("name", help="the material; for water, the bed's, with --marks the marks'")
    ap.add_argument("out")
    ap.add_argument("--marks", default="river_marks")
    ap.add_argument("--depth", type=float, default=0.5, help="the water's depth in metres (water)")
    ap.add_argument("--bands", type=int, nargs="*", default=[0])
    ap.add_argument(
        "--turn", type=float, default=0.0, help="degrees to turn a put-together thing about its middle (part)"
    )
    ap.add_argument("--sun", choices=sorted(SUNS), default="behind")
    ap.add_argument("--samples", type=int, default=24)
    ap.add_argument(
        "--light",
        nargs="*",
        default=[],
        metavar="KEY=VALUE",
        help="try other numbers for the light (the tuning's own keys, such as sun_colour=#fff6e8 sun_energy=170%%); "
        "the pictures are named with a -light suffix",
    )
    ap.add_argument(
        "--water",
        nargs="*",
        default=[],
        metavar="KEY=VALUE",
        help="try other numbers for the water (the tuning's own keys, such as fade_red='0.5 m' deep_colour=#154447); "
        "the pictures are named with a -water suffix",
    )
    args = ap.parse_args(argv[1:])
    change = dict(item.split("=", 1) for item in args.light)
    other = dict(item.split("=", 1) for item in args.water)
    for band in args.bands:
        suffix = "-light" if change else ""
        if args.what == "water":
            suffix += "-water" if other else ""
            print(
                water_view(
                    args.name, args.marks, band, args.sun, args.out, args.depth, args.samples, change, suffix, other
                )
            )
        elif args.what == "part":
            suffix += f"-turn{args.turn:g}" if args.turn else ""
            print(
                part_view(
                    args.name,
                    args.ground,
                    band,
                    args.sun,
                    args.out,
                    args.samples,
                    change,
                    suffix,
                    args.forms,
                    args.turn,
                )
            )
        elif args.what == "cliff":
            print(cliff_view(args.name, args.ground, band, args.sun, args.out, args.samples, change, suffix))
        elif args.what == "wall":
            print(wall_view(args.name, args.ground, band, args.sun, args.out, args.samples, change, suffix))
        else:
            print(ground_view(args.name, band, args.sun, args.out, args.samples, change, suffix, args.beside))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
