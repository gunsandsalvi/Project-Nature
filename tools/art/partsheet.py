#!/usr/bin/env python3
"""The preview sheet of a family of the kit's parts (IMPLEMENTATION.md, the art lane): every part at true size beside
a 1 m bar, wearing the checker of 64 texture pixels a metre, for the owner's eye.

    python3 tools/art/partsheet.py <views folder> <check.json> <out.webp> [--textured]

From the pictures preview.py drew and the figures partcheck.py measured: the file's name, its parts' count and worst
stretch, how to read the checker, then each collection's parts in rows, each labelled with its name, size and
worst stretch and drawn at 128 picture pixels a metre, so a texture pixel shows as 2 x 2 screen pixels at 100%, as
on the phone up close; small parts again four times as large; then each figure whole, bare, with each shape key and
in its bend poses; then, in red, any problem the check found. Lossless WebP, 1080 pixels wide. With --textured,
the same for pictures of the parts wearing their textures (preview.py --textures).
A 1 m bar starts every row: lengths across the view are true, and heights show at cos 40 degrees, 0.77, since the
view looks down from the game camera's height.

Implements PRE-46, see A6.5.
"""

import json
import math
import os
import sys

import numpy as np
from PIL import Image

import sheet
import texels

WIDTH = sheet.WIDTH
PAD = sheet.PAD
RED = (170, 40, 30)
LINE = 1.5


def over_paper(path):
    """A preview picture laid on the sheet's paper."""
    with Image.open(path) as im:
        rgba = im.convert("RGBA")
        paper = Image.new("RGBA", rgba.size, (*sheet.PAPER, 255))
        return np.asarray(Image.alpha_composite(paper, rgba).convert("RGB")).copy()


def metre_bar(scale):
    """A bar one metre long at `scale` picture pixels a metre, with ticks every 10 cm."""
    w = int(round(scale))
    bar = np.full((14, w + 3, 3), sheet.PAPER, np.uint8)
    bar[5:9, 1 : w + 1] = sheet.INK
    for k in range(11):  # 0 m at column 1, 1 m at column 1 + scale
        x = 1 + int(round(k * scale / 10))
        top = 0 if k in (0, 5, 10) else 3
        bar[top : 14 - top, x : x + 1] = sheet.INK
    return bar


def worst_text(w):
    if isinstance(w, str):
        return w
    return f"{w:.2f}:1"


CELL = 200  # picture pixels: the least width of a cell, so its label stays readable


def flow(c, items, scale):
    """Pictures in rows across the sheet, each row starting with a 1 m bar: items are (picture, label)."""
    bar = metre_bar(scale)
    room = WIDTH - 2 * PAD
    row, used = [], bar.shape[1] + PAD
    rows = []
    for item in items:
        w = max(CELL, item[0].shape[1])
        if row and used + w > room:
            rows.append(row)
            row, used = [], bar.shape[1] + PAD
        row.append(item)
        used += w + PAD
    if row:
        rows.append(row)
    for r in rows:
        pics = [bar] + [it[0] for it in r]
        labels = ["1 m"] + [it[1] for it in r]
        c.cells(pics, labels, size=18, least=[0] + [CELL] * len(r))


def make(views_dir, check_path, textured=False):
    with open(check_path) as f:
        check = json.load(f)
    with open(os.path.join(views_dir, "views.json")) as f:
        views = json.load(f)["views"]
    parts = {p["name"]: p for p in check["parts"]}
    worst = max((p["worst"] if not isinstance(p["worst"], str) else math.inf) for p in check["parts"])
    failed = [p for p in check["parts"] if p["problems"]]
    c = sheet.Canvas()
    c.text(f"{check['file']}: {len(parts)} parts" + (", wearing their textures" if textured else ""), 40, bold=True)
    c.text(
        f"Worst stretch {worst_text(worst)} (the line is {LINE}:1). {len(failed)} parts fail a check. Every part is "
        "drawn at true size, 128 picture pixels a metre, so at 100% a texture pixel shows as 2 x 2 screen pixels, as "
        "on the phone up close; parts under 0.4 m again four times as large.",
        22,
    )
    if textured:
        c.text(
            "Each part wears the textures a recipe might choose for its roles (art/models/<family>-textures.json), and "
            "a figure its atlases', drawn to its layout; a role with no texture yet keeps a flat colour. The light is "
            "Blender's plain studio light, not the game's. The view looks down 40 degrees, from in front and to the "
            "left, so heights show at 0.77 of the 1 m bar.",
            20,
            colour=sheet.GREY,
        )
    else:
        c.text(
            "The checker is 64 texture pixels a metre, in blocks of 8 (12.5 cm), tinted by each slot's role; a block "
            "that is not square shows stretch. A reddish line marks each metre of u, a greenish one each metre of v; "
            "on a pole the reddish line is its wrap. The view looks down 40 degrees, from in front and to the left, "
            "so heights show at 0.77 of the 1 m bar.",
            20,
            colour=sheet.GREY,
        )
    c.space()
    loose = [v for v in views if v["kind"] in ("part", "enlarged")]
    groups = []
    for v in loose:
        g = parts.get(v["parts"][0], {}).get("group", "")
        if g not in groups:
            groups.append(g)
    for g in groups:
        c.text(g.replace("_", " ").capitalize(), 28, bold=True)
        items = []
        for v in loose:
            p = parts.get(v["parts"][0])
            if p is None or p.get("group", "") != g:
                continue
            pic = over_paper(os.path.join(views_dir, v["view"] + ".png"))
            size = " x ".join(f"{x:.2f}" for x in p["size"])
            if v["kind"] == "enlarged":
                label = f"{p['name']}, x{int(v['scale'] / 128)}"
            else:
                label = f"{p['name']}: {size} m, stretch {worst_text(p['worst'])}"
            if pic.shape[1] > WIDTH - 3 * PAD - 130:
                k = math.ceil(pic.shape[1] / (WIDTH - 3 * PAD - 130))
                pic = pic[::k, ::k]
                label += f" (shown at 1/{k})"
            items.append((pic, label))
        flow(c, items, 128.0)
        c.space()
    figures = [v for v in views if v["kind"] == "figure"]
    names = []
    for v in figures:
        if v["figure"] not in names:
            names.append(v["figure"])
    for name in names:
        c.text(f"{name}: the figure", 28, bold=True)
        mine = [v for v in figures if v["figure"] == name]
        body = [p for p in parts.values() if p.get("armature") == name]
        w = max((p["worst"] if not isinstance(p["worst"], str) else math.inf) for p in body) if body else 1.0
        c.text(f"{len(body)} parts on one skeleton; worst stretch {worst_text(w)}.", 22)
        items = []
        for v in mine:
            pic = over_paper(os.path.join(views_dir, v["view"] + ".png"))
            items.append((pic, v["title"].replace("_", " ")))
        flow(c, items, 128.0)
        c.space()
    if failed:
        c.text("Problems", 28, bold=True)
        for p in failed:
            for prob in p["problems"]:
                c.text(f"{p['name']}: {prob}", 20, colour=RED)
    return c.render()


def main(argv):
    textured = "--textured" in argv
    argv = [a for a in argv if a != "--textured"]
    if len(argv) != 3:
        print(__doc__.split("\n\n")[1])
        return 2
    a = make(argv[0], argv[1], textured)
    texels.save_webp(a, argv[2])
    print(f"{argv[2]}: {a.shape[1]} x {a.shape[0]}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
