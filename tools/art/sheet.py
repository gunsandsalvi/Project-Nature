"""A catalogue sheet (IMPLEMENTATION.md, the catalogue): GPT's pictures of one piece laid out on one tall picture 1080
pixels wide, as the owner reads it on the phone, with its name, its colours, its scale sticks and its labels.

Every scale stick is drawn here by code at its true length, one design on every sheet: ivory and charcoal segments, ten
to a stick, 10 cm, 1 m or 10 m long.
- Surfaces: each picture from above shows a known area, so its stick is true to it. The tiles are shown about as the
  phone shows them where each is used (4 m, 16 m and 64 m across, each about 512 screen pixels); a close-up shows part
  of a tile enlarged, and a repeat panel how a tile's pattern reads over wider ground. A piece that runs along a
  course (a brook, a shore) can give `strips`, each {file, label, metres, stick}: a picture of the tile laid several
  times along the course, drawn alone in a row as wide as the page, its stick true to its `metres`. The camera views
  come as GPT drew them, their own sticks measured before they are used.
- Objects: GPT draws each view on flat magenta, as the art book's own sheets do. Here each is cut out and scaled from
  the size in metres the spec gives it (its height, or its width across the view), so every view on a sheet stands at
  one scale beside an upright stick and, for anything big, a standing adult 1.7 m tall drawn by code. A strip of key
  poses is scaled as one, from its first pose. The camera's view is also shown at its true size on the phone at each
  zoom the game sees it from.

Usage: python3 tools/art/sheet.py <spec.json> <picture folder> <out>
The spec names the piece and its pictures; tools/tests/test_art_sheet.py shows both kinds.

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


def fitting(metres, px_per_m, room):
    """The stick's length in metres: the one asked for or, for a small thing shown large, a tenth of it, and so on down
    to 1 cm, until it fits within `room` screen pixels."""
    while metres * px_per_m > room and metres > 0.011:
        metres = round(metres / 10, 4)
    return metres


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


KEY = (255, 0, 255)  # the flat magenta GPT draws objects on
ASIDE = 230  # screen pixels beside an object's views for the adult, the stick and their labels
ADULT = 1.7  # metres: the standing adult beside anything big
FIGURE = (150, 143, 130)
TALLEST = 520  # screen pixels the tallest thing on an object sheet may stand
ZOOMS = [("Up close", 128), ("The close camp", 32), ("The camp", 8)]  # screen pixels a metre at each zoom's centre


def solid(picture):
    """Where the picture is not the key colour. Where the drawing was blended into the key, its edge still leans to
    magenta; such edge pixels count as key too, peeled twice, so no pink fringe is left round the cut-out."""
    rgb = np.asarray(picture.convert("RGB")).astype(np.int32)
    on = np.abs(rgb - KEY).sum(axis=2) > 120
    pink = (rgb[:, :, 0] - rgb[:, :, 1] > 60) & (rgb[:, :, 2] - rgb[:, :, 1] > 60)
    for _ in range(2):
        inner = on.copy()
        inner[1:, :] &= on[:-1, :]
        inner[:-1, :] &= on[1:, :]
        inner[:, 1:] &= on[:, :-1]
        inner[:, :-1] &= on[:, 1:]
        on &= ~(pink & ~inner)
    return on


def spans(mask, least):
    """The runs of indices where `mask` holds and that are longer than `least`, each (start, end), end exclusive."""
    out, start = [], None
    for i, on in enumerate(list(mask) + [False]):
        if on and start is None:
            start = i
        elif not on and start is not None:
            if i - start > least:
                out.append((start, i))
            start = None
    return out


def clear(picture):
    """The picture as RGBA with the key colour made clear."""
    rgb = np.asarray(picture.convert("RGB"))
    return np.dstack([rgb, (solid(picture) * 255).astype(np.uint8)])


def cut_out(picture):
    """The object GPT drew on flat magenta, cropped to itself with the magenta made clear. Rows and columns with fewer
    than three solid pixels are left out of its box, so a stray speck doesn't widen it."""
    on = solid(picture)
    cols = np.nonzero(on.sum(axis=0) >= 3)[0]
    rows = np.nonzero(on.sum(axis=1) >= 3)[0]
    if not len(cols) or not len(rows):
        raise ValueError("the picture holds nothing but the key colour")
    return Image.fromarray(clear(picture), "RGBA").crop((cols[0], rows[0], cols[-1] + 1, rows[-1] + 1))


def to_scale(cut, metres, px_per_m, measure="tall"):
    """The cut-out scaled so its height, or with "across" its width, is `metres` at `px_per_m`."""
    k = metres * px_per_m / (cut.height if measure == "tall" else cut.width)
    size = (max(1, round(cut.width * k)), max(1, round(cut.height * k)))
    return cut.resize(size, Image.LANCZOS if k < 1 else Image.NEAREST)


def poses(picture, metres, px_per_m):
    """A strip of key poses on flat magenta, split at the empty columns between them and scaled as one, so its first
    pose is `metres` tall; each keeps its height above the strip's ground line."""
    on = solid(picture)
    rows = np.nonzero(on.sum(axis=1) >= 3)[0]
    top, bottom = rows[0], rows[-1] + 1
    found = spans(on[top:bottom].sum(axis=0) >= 3, on.shape[1] // 100)
    a, b = found[0]
    first = np.nonzero(on[top:bottom, a:b].sum(axis=1) >= 1)[0]
    k = metres * px_per_m / (first[-1] - first[0] + 1)
    rgba = clear(picture)
    out = []
    for a, b in found:
        piece = Image.fromarray(rgba[top:bottom, a:b], "RGBA")
        out.append(piece.resize((max(1, round(piece.width * k)), max(1, round(piece.height * k))), Image.LANCZOS))
    return out


def adult(height):
    """A standing adult `height` screen pixels tall, a plain figure drawn by code: the size beside big pieces."""
    h = height
    w = max(6, round(h * 0.3))
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cx = w / 2
    head = h * 0.13
    d.ellipse([cx - head * 0.42, 0, cx + head * 0.42, head], fill=FIGURE)
    neck = head * 1.05
    d.rounded_rectangle([cx - w * 0.36, neck, cx + w * 0.36, h * 0.54], radius=max(1, w * 0.12), fill=FIGURE)
    for side in (-1, 1):
        x0, x1 = sorted((cx + side * w * 0.4, cx + side * w * 0.5))
        d.rectangle([x0, neck + h * 0.03, x1, h * 0.5], fill=FIGURE)
        x0, x1 = sorted((cx + side * w * 0.04, cx + side * w * 0.3))
        d.rectangle([x0, h * 0.5, x1, h - 1], fill=FIGURE)
    return im


def label_box(label):
    tag = font(16, True)
    box = ImageDraw.Draw(Image.new("RGB", (1, 1))).textbbox((0, 0), label, font=tag)
    return tag, box


def upright(length, label, width=12):
    """An upright scale stick `length` screen pixels tall with its label above, as a picture standing on its foot."""
    tag, box = label_box(label)
    tw, th = box[2] - box[0], box[3] - box[1]
    w, h = max(width, tw + 8), length + th + 14
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    x = (w - width) // 2
    for i in range(10):
        colour = IVORY if i % 2 == 0 else CHARCOAL
        d.rectangle([x, h - round((i + 1) * length / 10), x + width - 1, h - round(i * length / 10) - 1], fill=colour)
    d.rectangle([0, 0, w - 1, th + 6], fill=IVORY)
    d.text(((w - tw) // 2, 3 - box[1]), label, fill=CHARCOAL, font=tag)
    return im


def lying(length, label, height=12):
    """A scale stick lying `length` screen pixels long with its label above, as a picture resting on the ground."""
    tag, box = label_box(label)
    th = box[3] - box[1]
    im = Image.new("RGBA", (length + 8, height + th + 14), (0, 0, 0, 0))
    stick(ImageDraw.Draw(im), 4, th + 14, length, label, height)
    return im


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

    def figures(self, items, after=GAP):
        """A row of (picture, label) standing on one ground line, each at its own size, so things drawn at one scale
        stay comparable; it wraps to another line when wider than the page."""
        label_h, width = 28, WIDTH - 2 * MARGIN
        probe = ImageDraw.Draw(Image.new("RGB", (1, 1)))
        lines, line, used = [], [], 0
        for picture, label in items:
            slot = max(picture.width, round(probe.textlength(label, font=font(19, True))) + 8 if label else 0)
            if line and used + GAP + slot > width:
                lines.append(line)
                line, used = [], 0
            line.append((picture, label, slot))
            used += (GAP if used else 0) + slot
        if line:
            lines.append(line)
        for line in lines:
            height = max(p.height for p, _, _ in line)

            def paint(page, draw, y, line=line, height=height):
                ground = y + label_h + height
                draw.line([MARGIN, ground, WIDTH - MARGIN, ground], fill=EDGE, width=2)
                x = MARGIN
                for picture, label, slot in line:
                    if label:
                        draw.text((x, y), label, fill=QUIET, font=font(19, True))
                    page.paste(picture, (x, ground - picture.height), picture if picture.mode == "RGBA" else None)
                    x += slot + GAP

            self.add(label_h + height + 2 + after, paint)

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
    """The picture at `width`: shrunk smoothly, or enlarged pixel for pixel so pixel art stays sharp."""
    size = (width, round(picture.height * width / picture.width))
    return picture.resize(size, Image.NEAREST if width > picture.width else Image.LANCZOS)


def object_scale(spec, cut):
    """Screen pixels a metre for an object sheet, a whole number: as many as let its tallest view, and the adult
    when shown, stand within TALLEST, its widest fit the page beside the adult and the stick, and its views stand in
    one row with them."""
    room = WIDTH - 2 * MARGIN - ASIDE

    def size(item):
        c = cut(item["file"])
        if "tall" in item:
            return item["tall"] * c.width / c.height, item["tall"]
        return item["across"], item["across"] * c.height / c.width

    views = spec["views"]["items"]
    items = list(views) + list(spec.get("camera_objects", {}).get("items", []))
    items += [spec["above"]] if "above" in spec else []
    items += [i for g in spec.get("groups", []) if g.get("scale", 1) == 1 for i in g["items"]]
    best = (room - GAP * (len(views) - 1)) / sum(size(i)[0] for i in views)
    if spec["views"].get("adult"):
        best = min(best, TALLEST / ADULT)
    for item in items:
        w, h = size(item)
        best = min(best, TALLEST / h, room / w)
    return max(1, int(best))


def compose_object(spec, sheet, folder):
    """The object's sections: its views at one scale with the adult and an upright stick, from above, the camera's view
    and its true size on the phone at each zoom, its groups of parts or states (a group seen from above gets a lying
    stick), and its strips of key poses."""
    cache = {}

    def cut(name):
        if name not in cache:
            cache[name] = cut_out(Image.open(os.path.join(folder, name)))
        return cache[name]

    def shown(item, px_per_m):
        measure = "tall" if "tall" in item else "across"
        return to_scale(cut(item["file"]), item[measure], px_per_m, measure), item.get("label", "")

    def standing(row, px_per_m, metres, with_adult):
        if with_adult:
            row.append((adult(round(ADULT * px_per_m)), "Adult, 1.7 m"))
        metres = fitting(metres, px_per_m, TALLEST)
        row.append((upright(round(metres * px_per_m), length_label(metres)), ""))
        return row

    s = object_scale(spec, cut)
    views = spec["views"]
    sheet.text(f"Seen flat on, in flat light, at one scale: {s} screen pixels a metre.", 26, bold=True)
    row = [shown(i, s) for i in views["items"]]
    sheet.figures(standing(row, s, views.get("stick", 1), views.get("adult", False)))
    if "above" in spec:
        above = spec["above"]
        sheet.text("From above, at the same scale.", 26, bold=True)
        metres = fitting(above.get("stick", 1), s, HALF)
        sheet.figures([shown(above, s), (lying(round(metres * s), length_label(metres)), "")])
    camera = spec.get("camera_objects")
    if camera:
        sheet.text("The game's camera, about 37 degrees down, late afternoon, at the same scale.", 26, bold=True)
        metres = fitting(camera.get("stick", 1), s, HALF)
        sheet.figures([shown(i, s) for i in camera["items"]] + [(lying(round(metres * s), length_label(metres)), "")])
        first = camera["items"][0]
        row = [(shown(first, k)[0], f"{label}: 1 m = {k} px") for label, k in ZOOMS]
        row = [(p, label) for p, label in row if p.width <= WIDTH - 2 * MARGIN - 40]
        if row:
            about = "True size on the phone at each zoom (scaled here; each band gets pixel art of its own)."
            sheet.text(about, 26, bold=True)
            sheet.figures(row)
    for group in spec.get("groups", []):
        k = group.get("scale", 1) * s
        title = group["title"] + ("" if k == s else f" ({k:g} screen pixels a metre)")
        sheet.text(title, 26, bold=True)
        row = [shown(i, k) for i in group["items"]]
        if group.get("above"):
            metres = fitting(group.get("stick", 1), k, HALF)
            sheet.figures(row + [(lying(round(metres * k), length_label(metres)), "")])
        else:
            sheet.figures(standing(row, k, group.get("stick", 1), group.get("adult", False)))
    for strip in [s for s in spec.get("strips", []) if "tall" in s]:
        sheet.text(strip["title"], 26, bold=True)
        figures = poses(Image.open(os.path.join(folder, strip["file"])), strip["tall"], s)
        sheet.figures(standing([(f, "") for f in figures], s, strip.get("stick", 1), False))


def compose(spec, folder):
    def load(name):
        return Image.open(os.path.join(folder, name)).convert("RGB")

    sheet = Sheet()
    sheet.text(f"{spec['number']}  {spec['name']}", 40, bold=True, after=6)
    sheet.text(spec["about"], 24, QUIET, after=14)

    chips = spec.get("palette", [])
    per_row = (WIDTH - 2 * MARGIN + 8) // 136
    for start in range(0, len(chips), per_row):

        def paint_chips(page, draw, y, row=chips[start : start + per_row]):
            x = MARGIN
            for name, hex_code in row:
                colour = tuple(int(hex_code[i : i + 2], 16) for i in (1, 3, 5))
                draw.rectangle([x, y, x + 128, y + 44], fill=colour, outline=EDGE)
                draw.text((x, y + 48), hex_code, fill=INK, font=font(16, True))
                if name:
                    draw.text((x, y + 68), name, fill=QUIET, font=font(15))
                x += 128 + 8

        sheet.add(100, paint_chips)

    if "views" in spec:
        compose_object(spec, sheet, folder)

    tiles = [(load(t["file"]), t) for t in spec.get("tiles", [])]
    if tiles:
        about = "From above, flat light: each tile about as the phone shows it where it is used."
        sheet.text(spec.get("tiles_title", about), 26, bold=True)
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
                (with_stick(crop, c["metres"], c.get("stick", 0.1)), f"Close-up: {c['metres']:g} m of the {name} tile")
            )
        for k in range(0, len(panels), 2):
            sheet.pictures(panels[k : k + 2])

    # a surface's strips: a tile laid several times along its course, each alone in a row as wide as the page; an
    # object's strips of key poses (with "tall") are drawn by compose_object
    strips = [s for s in spec.get("strips", []) if "metres" in s]
    if strips:
        about = "Laid three times along its course, to show how it repeats."
        sheet.text(spec.get("strips_title", about), 26, bold=True)
        for strip in strips:
            picture = fit(load(strip["file"]), WIDTH - 2 * MARGIN)
            sheet.pictures([(with_stick(picture, strip["metres"], strip["stick"]), strip["label"])])

    repeats = spec.get("repeat", [])
    if repeats:
        about = "Repeated 3 x 3, to show how the pattern reads over wider ground."
        sheet.text(spec.get("repeat_title", about), 26, bold=True)
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
                (
                    with_stick(shown, span, t.get("repeat_stick", 10)),
                    f"{t['label'].split(':')[0]} tile, {span:.0f} m across",
                )
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
