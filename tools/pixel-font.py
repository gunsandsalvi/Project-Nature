"""The game's pixel fonts for Godot (IMPLEMENTATION α0.7a, PRE-35, A15): reads the glyphs the art book draws with,
in art/book/paint/www/font.js, the one place they are designed, and writes them as data the app builds its fonts
from, prototypes/app/interface/glyphs.json: each glyph's rows of pixels, and how the handwriting leans, wobbles and
joins. Python's standard library only.

    python3 tools/pixel-font.py           # writes the glyphs
    python3 tools/pixel-font.py check     # fails if the glyphs written are not what font.js has now
"""

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FONT = ROOT / "art" / "book" / "paint" / "www" / "font.js"
OUT = ROOT / "prototypes" / "app" / "interface" / "glyphs.json"
KEY = r"""^\s*('(?:[^'\\]|\\.)*'|"[^"]*"|[A-Za-z0-9]+)\s*:\s*\[([^\]]*)\]"""


def number(text, pattern):
    """A number font.js's drawing code holds, found by the code around it."""
    m = re.search(pattern, text)
    if m is None:
        raise SystemExit(f"pixel-font: font.js no longer has {pattern!r}")
    return float(m.group(1))


def make(text):
    """The fonts' data from font.js's text: its glyph table, the accented letters it composes, and the
    handwriting's lean, wobble, joins and line height."""
    start = text.index("const G = {")
    table = text[start : text.index("};", start)]
    glyphs = {}
    for m in re.finditer(KEY, table, re.M):
        key = m.group(1)
        if key[0] in "'\"":
            key = key[1:-1].replace("\\'", "'")
        glyphs[key] = re.findall(r"'([.#]*)'", m.group(2))
    # letters with accents, as font.js composes them: the base letter with a mark in its top two rows
    for ch, base in re.findall(r"\['(.)', '(.)', 'acute'\]", text):
        g = list(glyphs[base])
        w = len(g[0])
        mid = w // 2
        g[0] = "".join("#" if i == min(w - 1, mid + 1) else "." for i in range(w))
        g[1] = "".join("#" if i == mid else "." for i in range(w))
        glyphs[ch] = g
    sets = dict(re.findall(r"const (ENDS_LOW|STARTS_X) = new Set\('([a-z]+)'", text))
    return {
        "source": "art/book/paint/www/font.js",
        "line": int(number(text, r"export const LINE = (\d+)")),
        "cap": 7,
        "lean_every": number(text, r"\(6 \* scale - oy\) / ([0-9.]+)"),
        "wobble": number(text, r"rnd\(\) < ([0-9.]+) \? -1"),
        "hand_extra": int(number(text, r"hand \? (\d+) : 0\)")),
        "ends_low": sets["ENDS_LOW"],
        "starts_x": sets["STARTS_X"],
        "glyphs": glyphs,
    }


def written(data):
    return json.dumps(data, ensure_ascii=False, indent=1, sort_keys=True) + "\n"


def main():
    data = written(make(FONT.read_text()))
    if sys.argv[1:] == ["check"]:
        if not OUT.exists() or OUT.read_text() != data:
            raise SystemExit(f"pixel-font: {OUT.relative_to(ROOT)} is not what font.js has; run tools/pixel-font.py")
        print("pixel-font: the glyphs are current")
        return
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(data)
    print(f"pixel-font: {OUT.relative_to(ROOT)}, {len(json.loads(data)['glyphs'])} glyphs")


if __name__ == "__main__":
    main()
