#!/usr/bin/env python3
"""Stand-in textures made by code for the engine's first pictures (T2.1a.3), until the art lane's materials come:
a meadow and a test pattern, each a seamless tile of 256 x 256 texture pixels at 64 a metre (4 m) with every level
down to 1 x 1, each level drawn for its own size rather than averaged (A5.3), written as .kdtex files.

    python3 tools/standins.py <folder>    writes <folder>/standin-meadow.kdtex and <folder>/standin-pattern.kdtex

A .kdtex file holds a texture's levels as PNG pictures, largest first: "KDTX", then the format's version, the
number of levels and each level's length in bytes as 32-bit little-endian numbers, each followed by its PNG. The
engine reads it (view/src/textures.cpp); Godot ships it untouched, since it imports no file of that name (A5.4).
Implements PRE-22, see A5.3 and A5.4.
"""

import struct
import sys
import zlib

TILE = 256  # band 0's side: 4 m at 64 texture pixels a metre
VERSION = 1

# The meadow's shades, darkest first: earth between the blades, shaded grass, grass, lit grass, dry tips; and the
# accents, a few single texture pixels: yellow and white flowers.
MEADOW = [(62, 58, 34), (84, 92, 44), (112, 122, 54), (142, 148, 66), (176, 168, 92)]
FLOWERS = [(226, 196, 64), (236, 232, 214)]
# The pattern's colour for each level, so the level read shows at a glance; two shades make each texture pixel's
# checker, and every eighth row and column is dark.
LEVELS = [
    (236, 236, 236),
    (240, 214, 80),
    (110, 200, 90),
    (80, 200, 210),
    (90, 120, 230),
    (200, 100, 210),
    (220, 80, 70),
    (150, 150, 150),
    (60, 60, 60),
]


def png(width, height, rgba):
    """A PNG picture, 8-bit RGBA, of rows of bytes."""
    rows = b"".join(b"\0" + bytes(rgba[y * width * 4 : (y + 1) * width * 4]) for y in range(height))

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) + chunk(b"IDAT", zlib.compress(rows, 9)) + chunk(b"IEND", b"")


def kdtex(levels):
    """A .kdtex file of PNG pictures, largest first."""
    out = b"KDTX" + struct.pack("<II", VERSION, len(levels))
    for picture in levels:
        out += struct.pack("<I", len(picture)) + picture
    return out


def hash32(*values):
    """A small integer hash of whole numbers, the same on every machine."""
    h = 2166136261
    for v in values:
        h = ((h ^ (v & 0xFFFFFFFF)) * 16777619) & 0xFFFFFFFF
        h ^= h >> 15
        h = (h * 2246822519) & 0xFFFFFFFF
        h ^= h >> 13
    return h


def noise(x, y, cells, seed):
    """Value noise in 0 to 1 that tiles every `cells` cells, at a point given in cells."""
    x0, y0 = int(x) % cells, int(y) % cells
    x1, y1 = (x0 + 1) % cells, (y0 + 1) % cells
    fx, fy = x - int(x), y - int(y)
    sx, sy = fx * fx * (3 - 2 * fx), fy * fy * (3 - 2 * fy)

    def v(i, j):
        return hash32(i, j, seed) / 0xFFFFFFFF

    top = v(x0, y0) + (v(x1, y0) - v(x0, y0)) * sx
    bottom = v(x0, y1) + (v(x1, y1) - v(x0, y1)) * sx
    return top + (bottom - top) * sy


def meadow_level(n):
    """The meadow at a level of n x n texture pixels over the 4 m tile: the same clumps of grass at every level,
    with detail only down to the level's own texture pixel, and flowers as single texture pixels of the level, about
    as many in each screen's worth whatever the level, so every band looks alike on screen."""
    out = bytearray()
    for j in range(n):
        for i in range(n):
            u, v = (i + 0.5) / n, (j + 0.5) / n  # where in the tile, 0 to 1
            value = 0.0
            weight = 0.0
            # octaves of 2 m, 1 m, 50 cm, ... down to two texture pixels of this level
            cells, amplitude = 2, 1.0
            while cells <= max(2, n // 2):
                value += amplitude * noise(u * cells, v * cells, cells, cells)
                weight += amplitude
                cells, amplitude = cells * 2, amplitude * 0.62
            shade = min(len(MEADOW) - 1, int(value / weight * len(MEADOW) * 1.15 - 0.4))
            colour = MEADOW[max(0, shade)]
            h = hash32(i, j, n, 7)
            if h % 1000 < 25:  # 2.5% of texture pixels, each a flower of the level's own size
                colour = FLOWERS[(h >> 12) % 2]
            out += bytes(colour) + b"\xff"
    return out


def pattern_level(n, level):
    """The test pattern at a level: its level's colour in a checker of single texture pixels, a dark line every
    eighth texture pixel."""
    base = LEVELS[min(level, len(LEVELS) - 1)]
    dim = tuple(c * 3 // 4 for c in base)
    out = bytearray()
    for j in range(n):
        for i in range(n):
            colour = base if (i + j) % 2 == 0 else dim
            if n >= 8 and (i % 8 == 0 or j % 8 == 0):
                colour = (30, 30, 30)
            out += bytes(colour) + b"\xff"
    return out


def levels(draw):
    sizes = []
    n = TILE
    while n >= 1:
        sizes.append(n)
        n //= 2
    return [png(n, n, draw(n, i)) for i, n in enumerate(sizes)]


def files():
    """Each stand-in's .kdtex file by its name."""
    return {
        "standin-meadow.kdtex": kdtex(levels(lambda n, i: meadow_level(n))),
        "standin-pattern.kdtex": kdtex(levels(pattern_level)),
    }


def main(argv):
    if len(argv) != 2:
        print(__doc__.strip().splitlines()[3].strip())
        return 2
    for name, data in files().items():
        with open(f"{argv[1]}/{name}", "wb") as f:
            f.write(data)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
