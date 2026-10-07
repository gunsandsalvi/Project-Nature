"""Labelled panels on one picture, so a piece can be put beside its sheet as the game shows it and enlarged (the art
lane's report): each panel a picture, or a crop of one, with a label above it.

    python3 tools/art/panels.py <out.png> <columns> <panel width> "label=path[@x,y,w,h]"...

A crop is in the picture's own pixels and is shown enlarged by whole nearest-pixel steps when the panel is wider than
it, and reduced smoothly when it is narrower, so crops at the phone's size come out true. Implements PRE-46, see A5.4.
"""

import os
import sys

from PIL import Image, ImageDraw, ImageFont

GROUND = (236, 232, 224)
INK = (40, 38, 34)


def panel(spec, width):
    """One panel's label and picture, cropped as the spec says and fitted to the width."""
    label, rest = spec.split("=", 1)
    path, _, box = rest.partition("@")
    im = Image.open(path).convert("RGB")
    if box:
        x, y, w, h = (int(v) for v in box.split(","))
        im = im.crop((x, y, x + w, y + h))
    if width >= im.width and width % im.width == 0:
        im = im.resize((width, im.height * (width // im.width)), Image.NEAREST)
    elif width != im.width:
        im = im.resize((width, round(im.height * width / im.width)), Image.LANCZOS)
    return label, im


def lay_out(specs, columns, width, gap=8):
    """The panels in rows of `columns`, each label above its picture."""
    font = ImageFont.truetype("DejaVuSans.ttf", 15)
    panels = [panel(s, width) for s in specs]
    rows = [panels[i : i + columns] for i in range(0, len(panels), columns)]
    heights = [max(p[1].height for p in r) + 24 for r in rows]
    sheet = Image.new("RGB", (columns * (width + gap) - gap, sum(heights) - 0), GROUND)
    draw = ImageDraw.Draw(sheet)
    y = 0
    for r, h in zip(rows, heights, strict=True):
        for i, (label, im) in enumerate(r):
            x = i * (width + gap)
            draw.text((x + 2, y + 3), label, fill=INK, font=font)
            sheet.paste(im, (x, y + 24))
        y += h
    return sheet


def main(argv):
    if len(argv) < 5:
        print(__doc__)
        return 2
    out, columns, width = argv[1], int(argv[2]), int(argv[3])
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    sheet = lay_out(argv[4:], columns, width)
    sheet.save(out)
    print(out, sheet.size)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
