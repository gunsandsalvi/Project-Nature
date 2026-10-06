#!/usr/bin/env python3
"""The sheet (IMPLEMENTATION.md, the art lane): one material for the owner's eye, art/sheets/<name>.webp, lossless and
1080 pixels wide, the phone's width, so at 100% each texture pixel shown "at true size" is 2 x 2 screen pixels.

    python3 tools/art/sheet.py <texture folder> <out.webp> [--source <picture> --box X0 Y0 X1 Y1 --block B]
                               [--rock] [--checks] [--note TEXT]...

From top to bottom:
1. the material's name and what it is, from its record, and for a big surface its tiles and versions (A5.3);
2. a crop of its source beside band 0 at the same scale, each texture pixel enlarged to the source's block size,
   and for a big surface each middle or far tile's own source beside its first level, the block size read from
   its record; for a texture drawn to a layout, the layout (where its parts' faces lie) beside band 0;
3. for a big surface or a tile with versions, each band (0 to 6) as a strip the phone's full width, from the tile
   that serves the band (near 0 and 1, middle 2 and 3, far 4 to 6), each cell of the ground taking a version by its
   place, so any repeat or seam shows as it would in the game;
4. every band at true size: the same 2 x 2 screen pixels a texture pixel, showing as much ground as the phone shows
   at that band's zoom, flat and under three stand-in lights (true midday, late afternoon, shade);
5. every level of each tile (0 to 8) enlarged until its texture pixels are plain to see;
6. a wrap atlas's strips (wraps/), each repeated round three times, so a wrap would show;
7. with --rock, the surface under stand-in layers: beds of different thicknesses, joints and a shelter, drawn by
   this tool only so the owner can judge a rock surface with layers on, never the engine's own;
8. with --checks, the checks' results (checks.py): how many pass, every failure in full, and what is reported only;
   then the notes given.
The stand-in lights are a flat colour multiplied in linear light; the engine's Lab page later shows the real light.

Implements PRE-20 and PRE-23, see A5.4.
"""

import argparse
import os
import re
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

import checks
import kitmath
import record
import texels

ROOT = checks.ROOT
BLOCKS = re.compile(r"(\d+(?:\.\d+)?)-pixel blocks|blocks of (\d+(?:\.\d+)?) picture pixels")

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

    def cells(self, pictures, labels, size=18, least=200):
        """Pictures side by side, each in a cell as wide as it or `least` (a number, or one for each picture),
        with its label wrapped above it; the row is as tall as its tallest label and its tallest picture together,
        so a long label never runs over a picture."""
        f = font(size)
        least = least if isinstance(least, (list, tuple)) else [least] * len(pictures)
        widths = [max(m, p.shape[1]) for m, p in zip(least, pictures, strict=True)]
        wrapped = [wrap(s, f, w) for s, w in zip(labels, widths, strict=True)]
        step = int(size * 1.25)
        label_h = max(len(w) for w in wrapped) * step
        x, top, tallest = PAD, self.y, 0
        for pic, lines, w in zip(pictures, wrapped, widths, strict=True):
            for i, line in enumerate(lines):
                self.parts.append(("text", line, f, GREY, x, top + i * step))
            self.parts.append(("image", pic, x, top + label_h))
            tallest = max(tallest, pic.shape[0])
            x += w + PAD
        self.y = top + label_h + tallest + PAD

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


TILES = (("near", "", 0), ("middle", "middle", 2), ("far", "far", 4))  # a big surface's tiles and first bands
STRIP_TEXELS = (WIDTH - 2 * PAD) // 2, 110  # a strip: the phone's width at 2 x 2 screen pixels a texture pixel


def load_tile(path):
    """A tile's levels and its versions' (itself first) from its folder, or None if it has none."""
    if not os.path.exists(os.path.join(path, "record.toml")):
        return None
    rec = record.read(os.path.join(path, "record.toml"))
    out = [[texels.load(os.path.join(path, f"b{n}.png")) for n in range(len(rec["band"]))]]
    for v in ("v2", "v3", "v4"):
        sub = os.path.join(path, v)
        if os.path.exists(os.path.join(sub, "record.toml")):
            out.append([texels.load(os.path.join(sub, f"b{n}.png")) for n in range(len(out[0]))])
    return out


def pick(cx, cy, count):
    """The version a cell of the ground takes, by a hash of its place (A5.3), as the engine's own hash would; here
    only to show the versions mixed."""
    h = (cx * 73856093) ^ (cy * 19349663) ^ 0x5BD1E995
    h = (h ^ (h >> 13)) * 0x5BD1E995 & 0xFFFFFFFF
    return (h ^ (h >> 15)) % count


def strip(versions, n, size=STRIP_TEXELS):
    """The ground a phone shows across its width at a band: the level n of each version laid cell by cell, each
    cell taking its version by its place; size is (width, height) in texture pixels."""
    w, h = size
    cell = versions[0][n].shape[0]
    out = np.zeros((h, w, 3), np.uint8)
    for cy in range(-(-h // cell)):
        for cx in range(-(-w // cell)):
            t = versions[pick(cx, cy, len(versions))][n]
            ys, xs = cy * cell, cx * cell
            out[ys : ys + cell, xs : xs + cell] = t[: h - ys, : w - xs]
    return texels.enlarge(out, 2)


def wrap_strips(level, widths, rows=96, turns=3):
    """Each strip of a wrap atlas repeated round `turns` times, side by side, its top `rows` texture pixels."""
    pics = []
    for w, x in widths:
        pics.append(np.tile(level[:rows, x : x + w], (1, turns, 1)))
        pics.append(np.full((rows, 3, 3), PAPER, np.uint8))
    return np.concatenate(pics, axis=1)


def block_of(words):
    """The picture pixels a texture pixel, as a record's words give the re-grid's blocks, or None."""
    m = BLOCKS.search(words or "")
    return float(m.group(1) or m.group(2)) if m else None


def beside(picture, level, block, box=None):
    """A crop of a source picture and the same ground in a level, each texture pixel enlarged to the block size,
    both half the sheet wide."""
    if box:
        picture = picture[box[1] : box[3], box[0] : box[2]]
    half = (WIDTH - 3 * PAD) // 2
    crop = picture[:half, :half]
    k = max(1, int(round(block or 4)))
    shown = texels.enlarge(level[: -(-half // k), : -(-half // k)], k)[: crop.shape[0], :half]
    return crop, shown, k


def busiest(mask, size=128, step=16):
    """(row, column) of the size x size window, on a grid of `step`, holding most of a layout's faces."""
    h, w = mask.shape[:2]
    best = (-1, 0, 0)
    for y in range(0, max(1, h - size + 1), step):
        for x in range(0, max(1, w - size + 1), step):
            best = max(best, (int(mask[y : y + size, x : x + size].sum()), -y, -x))
    return -best[1], -best[2]


def summary(results):
    """The checks' results (checks.check_texture's) as a sheet's lines: how many pass, each failure in full, and what
    is reported only, each said once with the tiles it holds for."""
    passed = sum(ok is True for _, _, ok, _ in results)
    failed = [(where, c, what) for where, c, ok, what in results if ok is False]
    noted = {}
    for where, c, ok, what in results:
        if ok is None:
            noted.setdefault((c, what), []).append(where or "near")
    tiles = len({where for where, _, _, _ in results})
    lines = [
        (f"{passed} checks pass" if passed != 1 else "1 check passes")
        + f" on its {tiles} tile{'s' if tiles > 1 else ''}"
        + (f"; {len(failed)} {'fails' if len(failed) == 1 else 'fail'}:" if failed else "; none fails.")
    ]
    lines += [f"FAILS: {where or 'near'}, {c}: {what}" for where, c, what in failed]
    if noted:
        lines.append("Reported only:")
        lines += [f"{c} ({', '.join(wheres)}): {what}" for (c, what), wheres in noted.items()]
    return lines


def make(folder, source=None, box=None, block=None, rock=False, notes=(), checked=None):
    """The sheet of the material in `folder`; `checked` is checks.check_texture's results, shown in summary."""
    rec = record.read(os.path.join(folder, "record.toml"))
    name = os.path.basename(os.path.normpath(folder))
    tiles = {kind: load_tile(os.path.join(folder, sub)) for kind, sub, _ in TILES}
    big = tiles["middle"] is not None or tiles["far"] is not None
    levels = tiles["near"][0]
    drawn = os.path.exists(os.path.join(folder, "layout.png"))
    c = Canvas()
    c.text(name.replace("_", " "), 40, bold=True)
    c.text(rec["about"], 26)
    if drawn:
        c.text(
            f"Route: {rec['route']}. Band 0 is drawn to its parts' layout, {rec['tile_texels']} x "
            f"{rec['tile_texels']} texture pixels at {rec['texels_a_metre']} a metre: not a tile, so it is shown "
            "whole, and at true size as a stand-in repeat only to judge its texture pixels.",
            22,
            colour=GREY,
        )
    elif big:
        kept = ", ".join(f"{kind} ({len(tiles[kind])} versions)" for kind, _, _ in TILES if tiles[kind])
        c.text(
            f"Route: {rec['route']}. A big surface with a tile for each distance (A5.3): near, 4 m, for bands 0 and "
            f"1; middle, 16 m, for bands 2 and 3; far, 64 m, for bands 4 to 6; each 256 x 256 texture pixels at its "
            f"first band, each drawn from its own picture. Tiles: {kept}.",
            22,
            colour=GREY,
        )
    else:
        c.text(
            f"Route: {rec['route']}. Band 0 is a seamless tile of {rec['tile_texels']} x {rec['tile_texels']} "
            f"texture pixels, {rec['tile_texels'] // rec['texels_a_metre']} metres, at {rec['texels_a_metre']} a "
            "metre.",
            22,
            colour=GREY,
        )
    c.text(f"Truth: {rec['truth']}", 22, colour=GREY)
    c.space()
    if source is not None:
        c.text("Its source, beside band 0 at the same scale", 28, bold=True)
        crop, b0, k = beside(texels.load(source), levels[0], block, box)
        c.row([crop, b0], ["its source, as drawn (a crop)", f"band 0, each texture pixel {k} x {k}"])
        c.space()
    for kind, sub, first in TILES[1:]:
        path = os.path.join(folder, sub, "record.toml")
        if tiles[kind] is None or not os.path.exists(path):
            continue
        own = record.read(path)
        found = [p for p in own.get("sources", []) if os.path.exists(os.path.join(ROOT, p))]
        if not found:
            continue
        k = block_of(own.get("made"))
        crop, first_level, k = beside(texels.load(os.path.join(ROOT, found[0])), tiles[kind][0][0], k)
        c.text(f"The {kind} tile's source, beside its first level (band {first}) at the same scale", 28, bold=True)
        c.row(
            [crop, first_level], [f"{os.path.basename(found[0])}, as drawn (a crop)", f"each texture pixel {k} x {k}"]
        )
        c.space()
    oy, ox = 0, 0  # where the views at true size and enlarged start: a drawn atlas's busiest corner
    if drawn:
        mask = texels.load(os.path.join(folder, "layout.png"))
        oy, ox = busiest(mask[..., 0] > 127)
        half = (WIDTH - 3 * PAD) // 2
        k = max(1, half // levels[0].shape[0])
        c.text("Its layout: where its parts' faces lie, beside band 0", 28, bold=True)
        c.row(
            [texels.enlarge(mask, k), texels.enlarge(levels[0], k)],
            ["white where a face lies; the rest is gaps", f"band 0, the whole atlas, each texture pixel {k} x {k}"],
        )
        c.space()

    def shown(level, n):
        """A level moved so its views start at the busiest corner of a drawn atlas (no move for a tile)."""
        return np.roll(level, (-(oy >> n), -(ox >> n)), axis=(0, 1))

    serving = []  # (band, tile kind, level index)
    for band in range(7):
        kind, first = "near", 0
        if big:
            for k, _, f in TILES:
                if tiles[k] is not None and band >= f:
                    kind, first = k, f
        serving.append((band, kind, band - first))
    if big or len(tiles["near"]) > 1:
        c.text("The ground as the phone shows it, the versions mixed", 28, bold=True)
        c.text(
            "Each strip is the phone's width at that band's zoom, 2 x 2 screen pixels a texture pixel, laid from the "
            "tile that serves the band, each cell of the ground taking one of its versions by its place, as the ground "
            "mixes them; any repeat or seam shows here as it would in the game.",
            22,
            colour=GREY,
        )
        for band, kind, n in serving:
            versions = tiles[kind]
            dens = TEXELS_A_METRE >> band
            across = STRIP_TEXELS[0] / dens
            each = "texture pixel" if dens == 1 else "texture pixels"
            c.text(
                f"Band {band}: {dens} {each} a metre, {across:.0f} m across; the {kind} tile, "
                f"{len(versions)} version{'s' if len(versions) > 1 else ''}",
                22,
                bold=True,
            )
            c.row([strip(versions, n)])
        c.space()
    c.text("Every band at true size: one texture pixel is 2 x 2 screen pixels", 28, bold=True)
    c.text("Each square shows the ground the phone shows in that much screen at that band's zoom.", 22, colour=GREY)
    for band, kind, n in serving:
        level = shown(tiles[kind][0][n], n)
        dens = TEXELS_A_METRE >> band
        metres = 128 // dens if dens <= 128 else 128 / dens
        flat = at_true_size(level)
        pics = [flat] + [at_true_size(lit(level, colour)) for _, colour in LIGHTS[:3]]
        each = "texture pixel" if dens == 1 else "texture pixels"
        whose = f", the {kind} tile" if big else ""
        c.text(f"Band {band}: {dens} {each} a metre, {metres} m across each square{whose}", 24, bold=True)
        c.row(pics, ["flat"] + [f"{label} (stand-in)" for label, _ in LIGHTS])
    c.space()
    for kind, _, first in TILES:
        if tiles[kind] is None:
            continue
        c.text(f"Every level of the {kind} tile enlarged" if big else "Every level enlarged", 28, bold=True)
        pics, labels = [], []
        for n, level in enumerate(tiles[kind][0]):
            pic, k = enlarged(shown(level, n))
            pics.append(pic)
            labels.append(f"level {n} (band {first + n}): {level.shape[1]} x {level.shape[0]}, x{k}")
        for i in range(0, len(pics), 4):
            c.row(pics[i : i + 4], labels[i : i + 4])
    wraps = load_tile(os.path.join(folder, "wraps"))
    if wraps is not None:
        c.space()
        c.text("Its wrap strips, for poles, branches and trunks", 28, bold=True)
        c.text(
            "Each strip of the wrap atlas, as wide as a part's circumference (A6.4), repeated round three times at "
            "true size, so its wrap shows if it would: widths "
            + ", ".join(str(w) for w, _ in kitmath.WRAPS)
            + " texture pixels.",
            22,
            colour=GREY,
        )
        c.row([texels.enlarge(wrap_strips(wraps[0][0], kitmath.WRAPS[:6]), 2)])
        c.row([texels.enlarge(wrap_strips(wraps[0][0], kitmath.WRAPS[6:]), 2)])
        c.row([enlarged(wraps[0][0])[0]], ["the wrap atlas, band 0, x8 (a corner)"])
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
    lines = (summary(checked) if checked else []) + list(notes)
    if lines:
        c.space()
        c.text("Checks", 28, bold=True)
        for line in lines:
            c.text(line, 22, colour=INK if line.startswith("FAILS") or line == lines[0] else GREY)
    return c.render()


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("folder")
    ap.add_argument("out")
    ap.add_argument("--source")
    ap.add_argument("--box", type=int, nargs=4)
    ap.add_argument("--block", type=float)
    ap.add_argument("--rock", action="store_true")
    ap.add_argument("--checks", action="store_true", help="run the checks and show their results")
    ap.add_argument("--note", action="append", default=[])
    args = ap.parse_args(argv)
    checked = checks.check_texture(args.folder) if args.checks else None
    a = make(args.folder, args.source, args.box, args.block, args.rock, args.note, checked)
    texels.save_webp(a, args.out)
    print(f"{args.out}: {a.shape[1]} x {a.shape[0]}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
