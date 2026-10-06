"""tools/standins.py makes the stand-in textures, each level drawn for its size (T2.1a.3, A5.3, A5.4)."""

import os
import struct
import sys
import unittest
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
import standins  # noqa: E402


def split(data):
    """The PNG pictures of a .kdtex file, as the engine's reader splits them (view/src/textures.cpp)."""
    assert data[:4] == b"KDTX"
    version, count = struct.unpack("<II", data[4:12])
    at, out = 12, []
    for _ in range(count):
        (length,) = struct.unpack("<I", data[at : at + 4])
        out.append(data[at + 4 : at + 4 + length])
        at += 4 + length
    assert at == len(data)
    return version, out


def pixels(png):
    """(width, height, rows of RGBA bytes) of a PNG the tool wrote: filter 0 on every row, one IDAT chunk."""
    width, height = struct.unpack(">II", png[16:24])
    at, idat = 8, b""
    while at < len(png):
        (length,) = struct.unpack(">I", png[at : at + 4])
        kind = png[at + 4 : at + 8]
        if kind == b"IDAT":
            idat += png[at + 8 : at + 8 + length]
        at += 12 + length
    raw = zlib.decompress(idat)
    stride = 1 + width * 4
    return width, height, [raw[y * stride + 1 : (y + 1) * stride] for y in range(height)]


class StandIns(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.made = standins.files()

    # checks: PRE-22
    def test_each_texture_holds_every_level_from_256_down_to_one(self):
        self.assertEqual(sorted(self.made), ["standin-meadow.kdtex", "standin-pattern.kdtex"])
        for data in self.made.values():
            version, levels = split(data)
            self.assertEqual(version, standins.VERSION)
            self.assertEqual([pixels(p)[0] for p in levels], [256, 128, 64, 32, 16, 8, 4, 2, 1])
            self.assertEqual([pixels(p)[1] for p in levels], [256, 128, 64, 32, 16, 8, 4, 2, 1])

    # checks: PRE-22
    def test_the_levels_are_drawn_for_their_size_not_averaged(self):
        # an averaged level would hold mixtures of the shades; a drawn one holds only the meadow's own colours
        allowed = {bytes(c) + b"\xff" for c in standins.MEADOW + standins.FLOWERS}
        _, levels = split(self.made["standin-meadow.kdtex"])
        for png in levels:
            width, _, rows = pixels(png)
            seen = {row[i * 4 : i * 4 + 4] for row in rows for i in range(width)}
            self.assertTrue(seen <= allowed, f"level {width}: colours that are mixtures")

    # checks: PRE-22
    def test_the_pattern_names_each_level_by_its_colour(self):
        _, levels = split(self.made["standin-pattern.kdtex"])
        for level, png in enumerate(levels):
            _, _, rows = pixels(png)
            first = tuple(rows[0][4:7]) if len(rows[0]) > 4 else tuple(rows[0][0:3])
            if len(rows) >= 8:
                first = tuple(rows[1][4:7])  # inside the dark lines of every eighth row and column
            self.assertIn(first, {standins.LEVELS[level], tuple(c * 3 // 4 for c in standins.LEVELS[level])})

    # checks: PRE-22
    def test_the_same_textures_every_time(self):
        self.assertEqual(standins.files(), self.made)


if __name__ == "__main__":
    unittest.main()
