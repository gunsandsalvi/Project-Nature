"""Lossless runtime PNG packing preserves colour, alpha, dimensions and decoded bytes."""

import io
import sys
import unittest
from pathlib import Path

from PIL import Image, PngImagePlugin

sys.path.insert(0, str(Path(__file__).parents[1]))
import pngpack  # noqa: E402


class PixelPacking(unittest.TestCase):
    def _pack(self, colours):
        image = Image.new("RGBA", (128, 128))
        image.putdata([colours[index % len(colours)] for index in range(128 * 128)])
        buffer = io.BytesIO()
        image.save(buffer, format="PNG", compress_level=0)
        before = buffer.getvalue()
        after = pngpack.lossless(before)
        decoded = Image.open(io.BytesIO(after))
        self.assertEqual(decoded.size, image.size)
        self.assertEqual(decoded.convert("RGBA").tobytes(), image.tobytes())
        self.assertLess(len(after), len(before))
        self.assertEqual(pngpack.lossless(after), after)
        return decoded

    # checks: PLT-06 PRC-11
    def test_exact_palette_preserves_transparency_and_hidden_rgb(self):
        decoded = self._pack([(23, 37, 59, 0), (24, 38, 60, 127), (25, 39, 61, 255)])
        self.assertEqual(decoded.mode, "P")

    # checks: PLT-06 PRC-11
    def test_more_than_256_colours_can_drop_only_an_opaque_alpha_channel(self):
        colours = [(index % 256, index // 256, 71, 255) for index in range(512)]
        self.assertEqual(self._pack(colours).mode, "RGB")
        colours[0] = (0, 0, 71, 127)
        self.assertEqual(self._pack(colours).mode, "RGBA")

    # checks: PLT-06 PRC-11
    def test_colour_profile_metadata_retains_original_encoding(self):
        buffer = io.BytesIO()
        info = PngImagePlugin.PngInfo()
        info.add(b"gAMA", (45455).to_bytes(4, "big"))
        Image.new("RGBA", (16, 16), (42, 57, 93, 255)).save(buffer, format="PNG", pnginfo=info)
        original = buffer.getvalue()
        self.assertEqual(pngpack.lossless(original), original)


if __name__ == "__main__":
    unittest.main()
