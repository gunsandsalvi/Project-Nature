"""A catalogue sheet (IMPLEMENTATION.md, the catalogue): GPT's panels for one piece laid out on one tall picture
1080 pixels wide, as the owner reads it on the phone, with its name, its colours, its scale sticks and its labels.

The views from above are re-gridded to their true texture pixels (the median colour of each block) and shown as the
phone shows them, a texture pixel 2 x 2 screen pixels, so what is approved is what the game will draw; a close-up shows
one of them enlarged until each texture pixel is a block, and a repeat panel shows how a tile's pattern reads over a
wider ground. The scale stick is one design on every sheet: ivory and charcoal segments, ten to a stick, 10 cm, 1 m or
10 m long.

Usage: python3 tools/art/sheet.py <spec.json> <panel folder> <out.webp>
The spec names the piece and its panels; tools/tests/test_art_sheet.py shows one.

Implements PRE-46, PRE-22, see A5.3.
"""

import json
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

WIDTH = 1080
MARGIN = 24
GAP = 16
GROUND = (236, 232, 224)
INK = (40, 38, 34)
QUIET = (104, 98, 90)
IVORY = (0xE8, 0xDE, 0xCA)
CHARCOAL = (0x45, 0x46, 0x3B)
EDGE = (0x6B, 0x68, 0x5E)
TRUE_SIZE = 2  # screen pixels a texture pixel covers, as the phone shows it (A5.3)
CLOSE_UP = 8


def font(size, bold=False):
    return ImageFont.truetype("DejaVuSans-Bold.ttf" if bold else "DejaVuSans.ttf", size)


def grid(edges, near):
    """The block grid along one axis, from the strength of the colour change between neighbouring pixels: the block's
    size, from half to twice what `near` blocks across would give, and the offset of the first block edge, at which
    the edges fall on the strongest changes. GPT draws its blocks a little larger or smaller than asked, starts with a
    part block, and blurs its edges.

    The score sums each edge's strength above the average, so a grid of blocks twice as big (missing half the edges)
    and one of blocks half as big (adding edges inside blocks, weaker than the average) both score below the true one.
    """
    length = len(edges) + 1
    base = edges.mean()
    expected = length / near
    best = (-np.inf, expected, 0.0)
    # outward from the size asked for, so of equal scores the one nearest it wins
    steps = np.arange(0.0, expected * 1.5, 0.01)
    sizes = [s for d in steps for s in ((expected + d,) if d == 0 else (expected - d, expected + d))]
    for size in (s for s in sizes if expected / 2 <= s <= expected * 2):
        for offset in np.arange(0.0, size, 0.25):
            at = np.round(np.arange(offset if offset >= 1 else offset + size, length - 1, size)).astype(int) - 1
            score = (edges[at[at >= 0]] - base).sum()
            if score > best[0]:
                best = (score, size, offset)
    return best[1], best[2]


def regrid(image, texels):
    """The picture as `texels` x `texels` texture pixels: its own block grid found along each axis, each block's colour
    the median of its middle half, so the blurred edges between GPT's blocks never mix into a texture pixel, then laid
    on `texels` x `texels` by the nearest block, so the tile keeps its size in metres."""
    rgb = np.asarray(image.convert("RGB"), dtype=np.int32)
    h, w, _ = rgb.shape
    starts = []
    for edges, length in (
        (np.abs(np.diff(rgb, axis=1)).sum(axis=2).mean(axis=0), w),
        (np.abs(np.diff(rgb, axis=0)).sum(axis=2).mean(axis=1), h),
    ):
        size, offset = grid(edges, texels)
        first = offset - size if offset >= 1 else offset
        starts.append((np.arange(first, length, size), size))
    (xs, px), (ys, py) = starts
    out = np.zeros((len(ys), len(xs), 3), dtype=np.uint8)
    for j, y0 in enumerate(ys):
        y_lo, y_hi = max(0, int(round(y0 + py / 4))), min(h, int(round(y0 + 3 * py / 4)))
        y_lo, y_hi = (y_lo, y_hi) if y_hi > y_lo else (max(0, int(y0)), min(h, int(y0) + 1))
        for i, x0 in enumerate(xs):
            x_lo, x_hi = max(0, int(round(x0 + px / 4))), min(w, int(round(x0 + 3 * px / 4)))
            x_lo, x_hi = (x_lo, x_hi) if x_hi > x_lo else (max(0, int(x0)), min(w, int(x0) + 1))
            out[j, i] = np.median(rgb[y_lo:y_hi, x_lo:x_hi].reshape(-1, 3), axis=0)
    return Image.fromarray(out).resize((texels, texels), Image.NEAREST)


def stick(draw, x, y, length, label, height=14):
    """The scale stick lying left to right from (x, y), `length` screen pixels long, its label above it."""
    seg = length / 10
    for i in range(10):
        colour = IVORY if i % 2 == 0 else CHARCOAL
        draw.rectangle([x + i * seg, y, x + (i + 1) * seg, y + height], fill=colour)
    draw.rectangle([x, y, x + length, y + height], outline=EDGE, width=2)
    for end in (x, x + length):
        draw.line([end, y - 5, end, y + height + 5], fill=CHARCOAL, width=3)
    tag = font(18, True)
    box = draw.textbbox((0, 0), label, font=tag)
    tx, ty = x, y - 8 - (box[3] - box[1])
    draw.rectangle([tx - 4, ty - 3, tx + box[2] - box[0] + 4, ty + box[3] - box[1] + 5], fill=IVORY)
    draw.text((tx, ty - box[1]), label, fill=CHARCOAL, font=tag)


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
        label_h = 30
        height = max(p.height for p, _ in row) + label_h

        def paint(page, draw, y):
            x = MARGIN
            for picture, label in row:
                if label:
                    draw.text((x, y), label, fill=QUIET, font=font(20, True))
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


def tile_panel(tile, spec):
    """A tile from above as the phone shows it, with a scale stick of the size that suits it."""
    shown = tile.resize((tile.width * TRUE_SIZE, tile.height * TRUE_SIZE), Image.NEAREST)
    draw = ImageDraw.Draw(shown)
    metres = spec["stick"]
    length = metres * spec["texels_a_metre"] * TRUE_SIZE
    stick(draw, 16, shown.height - 30, length, f"{metres:g} m" if metres >= 1 else f"{metres * 100:g} cm")
    return shown


def compose(spec, folder):
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

    tiles = []
    for t in spec.get("tiles", []):
        picture = Image.open(os.path.join(folder, t["file"]))
        tiles.append((regrid(picture, t.get("texels", 256)), t))
    if tiles:
        sheet.text(
            "From above, as the phone shows it: each texture pixel 2 x 2 screen pixels, flat light.",
            26,
            bold=True,
        )
        panels = [(tile_panel(tile, t), t["label"]) for tile, t in tiles]
        if "close_up" in spec:
            c = spec["close_up"]
            tile, t = tiles[c.get("tile", 0)]
            n = c.get("texels", 64)
            x0, y0 = c.get("at", [0, 0])
            crop = tile.crop((x0, y0, x0 + n, y0 + n)).resize((n * CLOSE_UP, n * CLOSE_UP), Image.NEAREST)
            draw = ImageDraw.Draw(crop)
            stick(draw, 16, crop.height - 30, 0.1 * t["texels_a_metre"] * CLOSE_UP, "10 cm")
            metres = n / t["texels_a_metre"]
            panels.append((crop, f"Close-up: {metres:g} m of the {t['label'].split(':')[0].lower()}, enlarged"))
        for k in range(0, len(panels), 2):
            sheet.pictures(panels[k : k + 2])

    repeats = spec.get("repeat", [])
    if repeats:
        sheet.text("Repeated 3 x 3, to show how the pattern reads over wider ground.", 26, bold=True)
        row = []
        for k in repeats:
            tile, t = tiles[k]
            big = Image.new("RGB", (tile.width * 3, tile.height * 3))
            for j in range(3):
                for i in range(3):
                    big.paste(tile, (i * tile.width, j * tile.height))
            shown = big.resize((512, 512), Image.LANCZOS)
            draw = ImageDraw.Draw(shown)
            span = 3 * tile.width / t["texels_a_metre"]
            stick(draw, 16, shown.height - 30, 10 * 512 / span, "10 m")
            row.append((shown, f"{t['label'].split(':')[0]} tile, {span:g} m across"))
        sheet.pictures(row)

    camera = spec.get("camera", [])
    if camera:
        sheet.text("The game's camera, 35 to 40 degrees down, from two sides, late afternoon.", 26, bold=True)
        half = (WIDTH - 2 * MARGIN - GAP) // 2
        sheet.pictures([(fit(Image.open(os.path.join(folder, f)), half), label) for f, label in camera])

    for title, files in spec.get("rows", []):
        sheet.text(title, 26, bold=True)
        for f, label in files:
            sheet.pictures([(fit(Image.open(os.path.join(folder, f)), WIDTH - 2 * MARGIN), label)])

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
