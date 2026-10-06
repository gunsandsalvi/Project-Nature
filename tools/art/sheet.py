#!/usr/bin/env python3
"""The sheet (IMPLEMENTATION.md, the art lane): one material for the owner's eye, art/sheets/<name>.webp, lossless and
1080 pixels wide, the phone's width, so at 100% each texture pixel shown "at true size" is 2 x 2 screen pixels.

    python3 tools/art/sheet.py <texture folder> <out.webp> [--source <picture> --box X0 Y0 X1 Y1 --block B]
                               [--rock] [--note TEXT]...

From top to bottom:
1. the material's name and what it is, from its record;
2. a crop of its source beside band 0 at the same scale, each texture pixel enlarged to the source's block size;
3. every band (0 to 6) at true size: the same 2 x 2 screen pixels a texture pixel, showing as much ground as the
   phone shows at that band's zoom, flat and under three stand-in lights (true midday, late afternoon, shade);
4. every level (0 to 8) enlarged until its texture pixels are plain to see;
5. with --rock, the surface under stand-in layers: beds of different thicknesses, joints and a shelter, drawn by
   this tool only so the owner can judge a rock surface with layers on, never the engine's own;
6. the notes given, such as the checks' results.
The stand-in lights are a flat colour multiplied in linear light; the engine's Lab page later shows the real light.

Implements PRE-20 and PRE-23, see A5.4.
"""

import argparse
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

import record
import texels

WIDTH = 1080
PAD = 12
SQUARE = 256  # each shown square: 128 texture pixels at 2 x 2 screen pixels
PAPER = (245, 241, 232)
INK = (42, 34, 28)
GREY = (110, 100, 92)
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
# stand-in lights as colours in linear light: white-warm at noon, gold when the sun is low, the sky's blue in shade
LIGHTS = [
    ("true midday", (1.00, 0.97, 0.92)),
    ("late afternoon", (0.95, 0.74, 0.50)),
    ("shade", (0.40, 0.45, 0.54)),
]
TEXELS_A_METRE = 64


def font(size, bold=False):
    path = BOLD if bold else FONT
    return ImageFont.truetype(path, size) if os.path.exists(path) else ImageFont.load_default()


def lit(t, colour):
    """A texture under a stand-in light: its linear colour times the light's."""
    return texels.to_srgb(texels.to_linear(t) * np.array(colour))


def at_true_size(level, texels_shown=128):
    """A square of `texels_shown` texture pixels of a level, repeated as the ground repeats it, each 2 x 2 pixels."""
    reps = -(-texels_shown // level.shape[0]), -(-texels_shown // level.shape[1]), 1
    return texels.enlarge(np.tile(level, reps)[:texels_shown, :texels_shown], 2)


def enlarged(level, size=SQUARE):
    """A level enlarged by a whole factor to fill the square: a corner of big levels, the whole of small ones."""
    k = max(1, size // max(level.shape[:2])) if max(level.shape[:2]) <= size // 8 else 8
    part = level[: size // k, : size // k]
    return texels.enlarge(part, k), k


def stand_in_cliff(surface, seed=1, w=256, h=176):
    """A stand-in cliff 4 m wide (w texture pixels) laid over a rock surface: beds of
    different thicknesses that pinch and swell, each taking the surface at its own offset, a ledge catching the light
    along each bed's top and an undercut darkening its foot, joints that lean a little and stop at the bed, blocks
    turned slightly to the light, and a rock shelter under the lowest bed. Only for the sheet; the world lays the real
    layers (PRE-23)."""
    rng = np.random.default_rng(seed)
    xs_all = np.arange(w)
    lin = texels.to_linear(surface)
    # bed boundaries: wavy lines across the cliff, the last bed above the shelter
    tops = [np.zeros(w)]
    y = 0.0
    while y < h - 62:
        y += rng.uniform(22, 48)
        wave = rng.uniform(1.5, 4.0) * np.sin(2 * np.pi * (xs_all / rng.uniform(90, 220) + rng.uniform(0, 1)))
        tops.append(np.clip(y + wave, tops[-1] + 10, h - 30))
    tops.append(np.full(w, float(h)))
    rows = np.arange(h)[:, None]
    out = np.zeros((h, w, 3))
    shade = np.ones((h, w))
    for i in range(len(tops) - 1):
        top, foot = tops[i][None, :], tops[i + 1][None, :]
        inside = (rows >= top) & (rows < foot)
        oy, ox = rng.integers(0, surface.shape[0]), rng.integers(0, surface.shape[1])
        ys = (np.arange(h) + oy) % surface.shape[0]
        xs = (xs_all + ox) % surface.shape[1]
        out = np.where(inside[..., None], lin[np.ix_(ys, xs)], out)
        depth = rows - top  # texture pixels below the bed's top
        shade *= np.where(inside & (depth < 2), 1.14, 1.0)  # the ledge catching the light
        shade *= np.where(inside & (foot - rows <= 3), 0.62, 1.0)  # the undercut at the bed's foot
        x = 0
        while x < w:  # blocks between joints, each turned a little to the light
            bw = int(rng.integers(34, 100))
            lean = rng.uniform(-0.12, 0.12)
            edge = x + lean * depth
            block = inside & (xs_all[None, :] >= edge) & (xs_all[None, :] < edge + bw)
            shade *= np.where(block, rng.uniform(0.88, 1.07), 1.0)
            if x > 0:
                shade *= np.where(inside & (np.abs(xs_all[None, :] - edge) < 0.6), 0.5, 1.0)
            x += bw
    # the shelter: a low hollow under the last bed, darker inward, lit only from its mouth
    last = tops[-2][None, :]
    cx, half = w * 0.52, w * 0.3
    roof = last + 4 + 10 * (1 - ((xs_all[None, :] - cx) / half) ** 2).clip(0, 1)
    hollow = (np.abs(xs_all[None, :] - cx) < half) & (rows > roof)
    shade *= np.where(hollow, 0.3 + 0.25 * (1 - np.abs(xs_all[None, :] - cx) / half), 1.0)
    return texels.to_srgb(out * shade[..., None])


class Canvas:
    def __init__(self):
        self.parts = []  # (image, x, y)
        self.y = PAD

    def text(self, s, size=26, bold=False, colour=INK, width=WIDTH - 2 * PAD):
        f = font(size, bold)
        for line in wrap(s, f, width):
            self.parts.append(("text", line, f, colour, PAD, self.y))
            self.y += int(size * 1.3)

    def row(self, pictures, labels=None, size=22):
        """Pictures side by side, each with a small label above it."""
        x = PAD
        top = self.y
        tallest = 0
        for i, pic in enumerate(pictures):
            label_h = 0
            if labels and labels[i]:
                for line in wrap(labels[i], font(size), pic.shape[1]):
                    self.parts.append(("text", line, font(size), GREY, x, top + label_h))
                    label_h += int(size * 1.25)
            self.parts.append(("image", pic, x, top + max(label_h, int(size * 1.25) * 2 if labels else 0)))
            tallest = max(tallest, pic.shape[0] + (int(size * 1.25) * 2 if labels else 0))
            x += pic.shape[1] + PAD
        self.y = top + tallest + PAD

    def space(self, n=PAD * 2):
        self.y += n

    def render(self):
        im = Image.new("RGB", (WIDTH, self.y + PAD), PAPER)
        d = ImageDraw.Draw(im)
        for p in self.parts:
            if p[0] == "text":
                _, s, f, colour, x, y = p
                d.text((x, y), s, font=f, fill=colour)
            else:
                _, pic, x, y = p
                im.paste(Image.fromarray(pic), (x, y))
        return np.asarray(im)


def wrap(s, f, width):
    words, lines, line = s.split(), [], ""
    for w in words:
        trial = (line + " " + w).strip()
        if f.getlength(trial) <= width or not line:
            line = trial
        else:
            lines.append(line)
            line = w
    if line:
        lines.append(line)
    return lines


def make(folder, source=None, box=None, block=None, rock=False, notes=()):
    rec = record.read(os.path.join(folder, "record.toml"))
    name = os.path.basename(os.path.normpath(folder))
    levels = [texels.load(os.path.join(folder, f"b{n}.png")) for n in range(len(rec["band"]))]
    c = Canvas()
    c.text(name.replace("_", " "), 40, bold=True)
    c.text(rec["about"], 26)
    c.text(
        f"Route: {rec['route']}. Band 0 is a seamless tile of {rec['tile_texels']} x {rec['tile_texels']} texture "
        f"pixels, {rec['tile_texels'] // rec['texels_a_metre']} metres, at {rec['texels_a_metre']} a metre.",
        22,
        colour=GREY,
    )
    c.text(f"Truth: {rec['truth']}", 22, colour=GREY)
    c.space()
    if source is not None:
        c.text("Its source, beside band 0 at the same scale", 28, bold=True)
        pic = texels.load(source)
        if box:
            pic = pic[box[1] : box[3], box[0] : box[2]]
        half = (WIDTH - 3 * PAD) // 2
        crop = pic[:half, :half]
        k = max(1, int(round(block or 4)))
        b0 = texels.enlarge(levels[0][: -(-half // k), : -(-half // k)], k)[:half, :half]
        c.row([crop, b0], ["its source, as drawn (a crop)", f"band 0, each texture pixel {k} x {k}"])
        c.space()
    c.text("Every band at true size: one texture pixel is 2 x 2 screen pixels", 28, bold=True)
    c.text("Each square shows the ground the phone shows in that much screen at that band's zoom.", 22, colour=GREY)
    for n, level in enumerate(levels[:7]):
        dens = TEXELS_A_METRE >> n
        metres = 128 // dens if dens <= 128 else 128 / dens
        flat = at_true_size(level)
        pics = [flat] + [at_true_size(lit(level, colour)) for _, colour in LIGHTS[:3]]
        each = "texture pixel" if dens == 1 else "texture pixels"
        c.text(f"Band {n}: {dens} {each} a metre, {metres} m across each square", 24, bold=True)
        c.row(pics, ["flat"] + [f"{label} (stand-in)" for label, _ in LIGHTS])
    c.space()
    c.text("Every level enlarged", 28, bold=True)
    big = []
    labels = []
    for n, level in enumerate(levels):
        pic, k = enlarged(level)
        big.append(pic)
        labels.append(f"level {n}: {level.shape[1]} x {level.shape[0]}, x{k}")
    for i in range(0, len(big), 4):
        c.row(big[i : i + 4], labels[i : i + 4])
    if rock:
        c.space()
        c.text("Under stand-in layers (drawn here only to judge the surface)", 28, bold=True)
        cliff = stand_in_cliff(levels[0])
        w = (WIDTH - 3 * PAD) // 2
        c.row(
            [texels.enlarge(cliff, 2)[:, :w], texels.enlarge(lit(cliff, LIGHTS[1][1]), 2)[:, :w]],
            ["flat, true size (2 m wide)", "late afternoon (stand-in), true size"],
        )
        c.row([texels.enlarge(cliff[:, : w // 4], 4)], ["enlarged 4 times"])
    if notes:
        c.space()
        c.text("Checks", 28, bold=True)
        for note in notes:
            c.text(note, 22)
    return c.render()


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("folder")
    ap.add_argument("out")
    ap.add_argument("--source")
    ap.add_argument("--box", type=int, nargs=4)
    ap.add_argument("--block", type=float)
    ap.add_argument("--rock", action="store_true")
    ap.add_argument("--note", action="append", default=[])
    args = ap.parse_args(argv)
    a = make(args.folder, args.source, args.box, args.block, args.rock, args.note)
    texels.save_webp(a, args.out)
    print(f"{args.out}: {a.shape[1]} x {a.shape[0]}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
