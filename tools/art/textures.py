"""The art lane's textures as the engine reads them (A5.4): art/textures/<name>/ holds a texture's levels as lossless
pictures and its record.toml; a big surface's middle and far tiles sit in middle/ and far/, and the versions of a tile
in v2/ to v4/, each folder with its own levels and record. This writes those records and reads whole sets back, so
the makers and the previews agree with the loader (sim/src/kd/look/texture.hpp) by one description.

A level's file is named by the band it serves (b2.png), its record's `level` counting from the tile's first band.

Implements PRE-20 and PRE-22, see A5.3 and A5.4.
"""

import os
import tomllib

import tiles

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TEXTURES = os.path.join(ROOT, "art", "textures")

# the tiles of a big surface: the folder below the material's (none for the near tile), metres across, first band
TILES = {"near": ("", 4, 0), "middle": ("middle", 16, 2), "far": ("far", 64, 4)}


def folder(name, tile, version):
    """Where a tile's version lies, from the repository's top: the near tile's first version in the material's own
    folder, a later version in v2/ to v4/ below its tile's folder."""
    parts = ["art", "textures", name]
    if TILES[tile][0]:
        parts.append(TILES[tile][0])
    if version > 1:
        parts.append(f"v{version}")
    return "/".join(parts)


def entry_name(name, tile, version):
    """The record's name in the catalogue: its folder below art/textures/ after "art:"."""
    return "art:" + folder(name, tile, version)[len("art/textures/") :]


def quote(text):
    """A TOML basic string."""
    out = text.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")
    return f'"{out}"'


def strings(items):
    return "[" + ", ".join(quote(i) for i in items) + "]"


def record_text(fields, levels):
    """A record.toml: the fields in the order the schema lists them, then each level's table."""
    lines = []
    for key in ("about", "route", "tile_texels", "texels_a_metre", "first_band", "laid", "acts"):
        if key in fields:
            v = fields[key]
            lines.append(f"{key} = {v}" if isinstance(v, int) else f"{key} = {quote(v)}")
    for key in ("sources", "original_sha256", "c2pa", "requests"):
        lines.append(f"{key} = {strings(fields[key])}")
    for key in ("made", "regrid_loss", "truth", "approved"):
        lines.append(f"{key} = {quote(fields[key])}")
    for level in levels:
        lines.append("")
        lines.append("[[band]]")
        for key in ("level", "file", "sha256", "made_from", "way", "regrid_loss", "calibration"):
            if level.get(key) not in (None, ""):
                v = level[key]
                lines.append(f"{key} = {v}" if isinstance(v, int) else f"{key} = {quote(v)}")
    return "\n".join(lines) + "\n"


def write_texture(name, tile, version, fields, pictures, ways):
    """Writes one tile's version: each level's picture as b<band>.png in its folder, then its record. `pictures` are the
    levels from the first band down; `ways` is, for each, a dict of its record's own words (way, regrid_loss,
    calibration). Returns the folder."""
    where = folder(name, tile, version)
    os.makedirs(os.path.join(ROOT, where), exist_ok=True)
    first = fields["first_band"]
    levels, above = [], ""
    for i, picture in enumerate(pictures):
        rel = f"{where}/b{first + i}.png"
        tiles.write_png(os.path.join(ROOT, rel), picture)
        digest = tiles.sha256(os.path.join(ROOT, rel))
        level = {"level": i, "file": rel, "sha256": digest, "made_from": above}
        level.update(ways[i])
        levels.append(level)
        above = digest
    with open(os.path.join(ROOT, where, "record.toml"), "w") as f:
        f.write(record_text(fields, levels))
    return where


def read_texture(where):
    """A record read back, with each level's picture (RGB, or RGBA for the water's marks): (the record's fields,
    [(level's table, its picture)])."""
    with open(os.path.join(ROOT, where, "record.toml"), "rb") as f:
        record = tomllib.load(f)
    levels = sorted(record["band"], key=lambda b: b["level"])
    return record, [(b, tiles.read_pixels(os.path.join(ROOT, b["file"]))) for b in levels]


def read_set(name):
    """All of a material's tiles and versions: {tile: [version 1's levels, version 2's, ...]} with each level's picture,
    from art/textures/<name>/, the tiles it has only."""
    out = {}
    for tile in TILES:
        versions = []
        for v in range(1, 5):
            where = folder(name, tile, v)
            if not os.path.isfile(os.path.join(ROOT, where, "record.toml")):
                break
            versions.append([picture for _, picture in read_texture(where)[1]])
        if versions:
            out[tile] = versions
    return out
