"""tools/pixel-font.py gives the app the art book's pixel fonts (IMPLEMENTATION α0.7a, PRE-35): every glyph of
font.js, its accented letters as font.js composes them, the handwriting's numbers, and a written file kept current."""

import importlib.util
import os
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("pixel_font", os.path.join(HERE, "..", "pixel-font.py"))
pixel_font = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pixel_font)

SAMPLE = """
const G = {
  A: ['.#.', '#.#', '###', '#.#', '#.#', '#.#', '#.#'],
  e: ['...', '...', '.#.', '#.#', '###', '#..', '.##'],
  g: ['...', '...', '.##', '#.#', '#.#', '#.#', '.##', '..#', '##.'],
  "'": ['#', '#', '.', '.', '.', '.', '.'],
  '"': ['#.#', '#.#', '...', '...', '...', '...', '...'],
};
for (const [ch, base, mark] of [['é', 'e', 'acute']]) {}
const ENDS_LOW = new Set('ae'.split(''));
const STARTS_X = new Set('eg'.split(''));
  return Math.max(0, w - 1) + (hand ? 3 : 0);
  const lean = (oy) => (hand ? Math.floor((6 * scale - oy) / 3.5) : 0);
    const dy = hand ? (rnd() < 0.1 ? -1 : 0) * scale : 0;
export const LINE = 11;
"""


class PixelFont(unittest.TestCase):
    # checks: PRE-35
    def test_every_glyph_and_the_handwritings_numbers(self):
        data = pixel_font.make(SAMPLE)
        self.assertEqual(sorted(data["glyphs"]), ['"', "'", "A", "e", "g", "é"])
        self.assertEqual(len(data["glyphs"]["g"]), 9)
        # é is e with its mark in the top two rows, as font.js composes it
        self.assertEqual(data["glyphs"]["é"][:2], ["..#", ".#."])
        self.assertEqual(data["glyphs"]["é"][2:], data["glyphs"]["e"][2:])
        self.assertEqual((data["line"], data["lean_every"], data["wobble"], data["hand_extra"]), (11, 3.5, 0.1, 3))
        self.assertEqual((data["ends_low"], data["starts_x"]), ("ae", "eg"))

    # checks: PRE-35
    def test_the_apps_glyphs_are_what_font_js_has_now(self):
        data = pixel_font.written(pixel_font.make(pixel_font.FONT.read_text()))
        self.assertEqual(pixel_font.OUT.read_text(), data)
        glyphs = pixel_font.make(pixel_font.FONT.read_text())["glyphs"]
        self.assertTrue(all(len({len(r) for r in rows}) == 1 for rows in glyphs.values()))
        self.assertTrue(all(len(glyphs[c]) == 7 for c in "ABCXYZ0123456789"))


if __name__ == "__main__":
    unittest.main()
