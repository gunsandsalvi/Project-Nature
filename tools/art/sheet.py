"""A catalogue sheet (IMPLEMENTATION.md, the catalogue): GPT's pictures of one piece laid out on one tall picture 1080
pixels wide, as the owner reads it on the phone, with its name, its colours, its scale sticks and its labels.

Each picture from above shows a known area, so its scale stick is drawn here by code at its true length: one design on
every sheet, ivory and charcoal segments, ten to a stick, 10 cm, 1 m or 10 m long. The tiles are shown about as the
phone shows them where each is used (4 m, 16 m and 64 m across, each about 512 screen pixels); a close-up shows part of
a tile enlarged, and a repeat panel how a tile's pattern reads over wider ground. The camera views come as GPT drew
them, their own sticks measured before they are used.

Usage: python3 tools/art/sheet.py <spec.json> <picture folder> <out>
The spec names the piece and its pictures; tools/tests/test_art_sheet.py shows one.

Implements PRE-46, PRE-22, see A5.3.
"""

import json
import os
import sys

from PIL import Image, ImageDraw, ImageFont

WIDTH = 1080
MARGIN = 24
GAP = 16
HALF = (WIDTH - 2 * MARGIN - GAP) // 2
THIRD = (WIDTH - 2 * MARGIN - 2 * GAP) // 3
GROUND = (236, 232, 224)
INK = (40, 38, 34)
QUIET = (104, 98, 90)
IVORY = (0xE8, 0xDE, 0xCA)
CHARCOAL = (0x45, 0x46, 0x3B)
EDGE = (0x6B, 0x68, 0x5E)


def font(size, bold=False):
    return ImageFont.truetype("DejaVuSans-Bold.ttf" if bold else "DejaVuSans.ttf", size)


def length_label(metres):
    return f"{metres:g} m" if metres >= 1 else f"{metres * 100:g} cm"


def stick(draw, x, y, length, label, height=12):
    """The scale stick lying left to right from (x, y), `length` screen pixels long, its label above it."""
    seg = length / 10
    for i in range(10):
        colour = IVORY if i % 2 == 0 else CHARCOAL
        draw.rectangle([round(x + i * seg), y, round(x + (i + 1) * seg) - 1, y + height - 1], fill=colour)
    tag = font(16, True)
    box = draw.textbbox((0, 0), label, font=tag)
    tx, ty = x, y - 7 - (box[3] - box[1])
    draw.rectangle([tx - 4, ty - 3, tx + box[2] - box[0] + 4, ty + box[3] - box[1] + 4], fill=IVORY)
    draw.text((tx, ty - box[1]), label, fill=CHARCOAL, font=tag)


def with_stick(picture, metres_across, metres):
    """The picture, which shows `metres_across` metres from side to side, with a stick `metres` long at its bottom
    left: true to the picture, whatever size it is shown at."""
    shown = picture.convert("RGB")
    stick(ImageDraw.Draw(shown), 14, shown.height - 26, shown.width * metres / metres_across, length_label(metres))
    return shown


def wrapped(draw, text, width, face):
    """The text broken into lines no wider than `width`."""
    lines, line = [], ""
    for word in text.split():
        trial = f"{line} {word}".strip()
        if draw.textlength(trial, font=face) <= width or not line:
            line = trial
        else:
            lines.append(line)
            line = word
    if line:
        lines.append(line)
    return lines


class Sheet:
    """A tall page built top to bottom: each block is drawn when its height is known."""

    def __init__(self):
        self.blocks = []

    def add(self, height, paint):
        self.blocks.append((height, paint))

    def text(self, text, size, colour=INK, bold=False, after=8):
        face = font(size, bold)
        probe = ImageDraw.Draw(Image.new("RGB", (1, 1)))
        lines = wrapped(probe, text, WIDTH - 2 * MARGIN, face)
        step = int(size * 1.3)

        def paint(page, draw, y):
            for k, line in enumerate(lines):
                draw.text((MARGIN, y + k * step), line, fill=colour, font=face)

        self.add(step * len(lines) + after, paint)

    def pictures(self, row, after=GAP):
        """A row of (picture, label) side by side, each at its own size; the row as tall as its tallest."""
        label_h = 28
        height = max(p.height for p, _ in row) + label_h

        def paint(page, draw, y):
            x = MARGIN
            for picture, label in row:
                if label:
                    draw.text((x, y), label, fill=QUIET, font=font(19, True))
                page.paste(picture, (x, y + label_h))
                x += picture.width + GAP

        self.add(height + after, paint)

    def render(self):
        height = MARGIN + sum(h for h, _ in self.blocks) + MARGIN
        page = Image.new("RGB", (WIDTH, height), GROUND)
        draw = ImageDraw.Draw(page)
        y = MARGIN
        for h, paint in self.blocks:
            paint(page, draw, y)
            y += h
        return page


def fit(picture, width):
    return picture.resize((width, round(picture.height * width / picture.width)), Image.LANCZOS)


def compose(spec, folder):
    def load(name):
        return Image.open(os.path.join(folder, name)).convert("RGB")

    sheet = Sheet()
    sheet.text(f"{spec['number']}  {spec['name']}", 40, bold=True, after=6)
    sheet.text(spec["about"], 24, QUIET, after=14)

    chips = spec.get("palette", [])
    if chips:

        def paint_chips(page, draw, y):
            x = MARGIN
            for name, hex_code in chips:
                colour = tuple(int(hex_code[i : i + 2], 16) for i in (1, 3, 5))
                draw.rectangle([x, y, x + 128, y + 44], fill=colour, outline=EDGE)
                draw.text((x, y + 48), hex_code, fill=INK, font=font(16, True))
                draw.text((x, y + 68), name, fill=QUIET, font=font(15))
                x += 128 + 8
                if x + 128 > WIDTH - MARGIN:
                    break

        sheet.add(100, paint_chips)

    tiles = [(load(t["file"]), t) for t in spec.get("tiles", [])]
    if tiles:
        sheet.text("From above, flat light: each tile about as the phone shows it where it is used.", 26, bold=True)
        panels = [(with_stick(fit(p, HALF), t["metres"], t["stick"]), t["label"]) for p, t in tiles]
        if "close_up" in spec:
            c = spec["close_up"]
            picture, t = tiles[c.get("tile", 0)]
            per_metre = picture.width / t["metres"]
            x0, y0 = (round(v * per_metre) for v in c.get("at", [0, 0]))
            side = round(c["metres"] * per_metre)
            crop = picture.crop((x0, y0, x0 + side, y0 + side)).resize((HALF, HALF), Image.NEAREST)
            name = t["label"].split(":")[0].lower()
            panels.append(
                (with_stick(crop, c["metres"], c.get("stick", 0.1)), f"Close-up: {c['metres']:g} m of the {name}")
            )
        for k in range(0, len(panels), 2):
            sheet.pictures(panels[k : k + 2])

    repeats = spec.get("repeat", [])
    if repeats:
        sheet.text("Repeated 3 x 3, to show how the pattern reads over wider ground.", 26, bold=True)
        row = []
        for k in repeats:
            picture, t = tiles[k]
            small = fit(picture, HALF // 3 + 1)
            big = Image.new("RGB", (small.width * 3, small.height * 3))
            for j in range(3):
                for i in range(3):
                    big.paste(small, (i * small.width, j * small.height))
            shown = big.crop((0, 0, HALF, HALF))
            span = 3 * t["metres"] * HALF / big.width
            row.append(
                (with_stick(shown, span, t.get("repeat_stick", 10)), f"{t['label'].split(':')[0]}, {span:.0f} m across")
            )
        sheet.pictures(row)

    camera = spec.get("camera", [])
    if camera:
        sheet.text("The game's camera, about 37 degrees down, from two sides, late afternoon.", 26, bold=True)
        sheet.pictures([(fit(load(f), HALF), label) for f, label in camera])

    states = spec.get("states")
    if states:
        sheet.text(states["title"], 26, bold=True)
        panels = [
            (with_stick(fit(load(f), THIRD), states["metres"], states["stick"]), label) for f, label in states["tiles"]
        ]
        for k in range(0, len(panels), 3):
            sheet.pictures(panels[k : k + 3])

    notes = spec.get("notes", [])
    if notes:
        sheet.text("Notes", 26, bold=True, after=4)
        for line in notes:
            sheet.text("- " + line, 21, QUIET, after=4)
    return sheet.render()


def main(argv):
    if len(argv) != 4:
        print(__doc__.split("\n\n")[2])
        return 2
    with open(argv[1], encoding="utf-8") as f:
        spec = json.load(f)
    page = compose(spec, argv[2])
    page.save(argv[3], quality=92)
    print(f"{argv[3]}: {page.width} x {page.height}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
