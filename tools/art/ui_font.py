"""Rasterize licensed glyphs from an explicit stable list (PRE-26 PRE-30)."""

from pathlib import Path
import argparse
import hashlib
import json
from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--sources", type=Path, required=True)
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
SOURCE = args.sources
ROOT = args.output
ROOT.mkdir(parents=True, exist_ok=True)
primary = ImageFont.truetype(str(SOURCE / "PixelifySans.ttf"), 16)
fallback_path = SOURCE / "DejaVuSans.ttf"
fallback = ImageFont.truetype(str(fallback_path), 16)
glyph_input = json.loads((SOURCE / "glyphs.json").read_text())
if hashlib.sha256((SOURCE / "PixelifySans.ttf").read_bytes()).hexdigest() != glyph_input["source_sha256"]:
    raise ValueError("primary font source hash differs from glyph input")
characters = set(glyph_input["characters"])
if len(characters) != len(glyph_input["characters"]):
    raise ValueError("duplicate declared character")
missing = bytes(primary.getmask("\U0010ffff"))
asc, desc = primary.getmetrics()
line_height = asc + desc
atlas = Image.new("RGBA", (512, 256), (255, 255, 255, 0))
records = []
fallback_chars = []
x = y = 1
row_height = 0
question_aliases = []
for c in sorted(characters, key=ord):
    font = primary
    if c != " " and bytes(primary.getmask(c)) == missing:
        font = fallback
        fallback_chars.append(c)
        if bytes(fallback.getmask(c)) == bytes(fallback.getmask("\U0010ffff")):
            question_aliases.append(c)
            font = primary
    drawn = "?" if c in question_aliases else c
    bbox = font.getbbox(drawn, anchor="ls")
    left, top, right, bottom = bbox
    w = max(1, right - left)
    h = max(1, bottom - top)
    if x + w + 1 > atlas.width:
        x = 1
        y += row_height + 2
        row_height = 0
    if y + h + 1 > atlas.height:
        raise ValueError("atlas full")
    mask = Image.new("1", (w, h))
    draw = ImageDraw.Draw(mask)
    draw.fontmode = "1"
    draw.text((-left, -top), drawn, font=font, fill=1, anchor="ls")
    rgba = Image.new("RGBA", (w, h), (255, 255, 255, 0))
    rgba.putalpha(mask.convert("L"))
    atlas.paste(rgba, (x, y))
    records.append(
        f"char id={ord(c)} x={x} y={y} width={w} height={h} xoffset={left} "
        f"yoffset={asc + top} xadvance={round(font.getlength(drawn))} page=0 chnl=15"
    )
    x += w + 2
    row_height = max(row_height, h)
atlas.save(ROOT / "kindling-ui-16.png")
lines = (
    [
        'info face="Kindling UI Bitmap" size=16 bold=0 italic=0 charset="" unicode=1 '
        "stretchH=100 smooth=0 aa=1 padding=0,0,0,0 spacing=1,1",
        f"common lineHeight={line_height} base={asc} scaleW=512 scaleH=256 pages=1 packed=0",
        'page id=0 file="kindling-ui-16.png"',
        f"chars count={len(records)}",
    ]
    + records
    + ["kernings count=0"]
)
(ROOT / "kindling-ui-16.fnt").write_text("\n".join(lines) + "\n")
(ROOT / "glyphs.json").write_text(
    json.dumps(
        {
            "base_size": 16,
            "line_height": line_height,
            "baseline": asc,
            "characters": "".join(sorted(characters, key=ord)),
            "fallback_symbols": "".join(fallback_chars),
            "question_aliases": "".join(question_aliases),
            "source_sha256": hashlib.sha256((SOURCE / "PixelifySans.ttf").read_bytes()).hexdigest(),
            "missing_fallback": (
                "replace unsupported display character with U+003F question mark; retain full underlying text"
            ),
            "licenses": ["OFL.txt", "DejaVu-LICENSE.txt"],
            "filter": "nearest",
            "scale": "integer only",
        },
        indent=2,
        ensure_ascii=False,
    )
    + "\n"
)
# Show the atlas glyphs with exact integer presentation from the same records.
records_by_char = {
    chr(int(r.split()[1].split("=")[1])): dict(part.split("=") for part in r.split()[1:]) for r in records
}
spec = Image.new("RGBA", (1080, 660), "#17241e")
for baseline_y, scale, text in [
    (24, 2, "Camp  River  Shelter  Noon  Dusk  Explore"),
    (120, 3, "Camp  River  Shelter"),
    (210, 3, "The camp is quiet beside the river."),
    (300, 3, "0123456789  α × °  ← →  ✓ ✗  …"),
    (390, 2, "Time  Catalogues  Reports  Check"),
    (470, 2, "At noon, light falls across the camp."),
]:
    px = 24
    for c in text:
        r = records_by_char[c]
        box = (int(r["x"]), int(r["y"]), int(r["x"]) + int(r["width"]), int(r["y"]) + int(r["height"]))
        glyph = atlas.crop(box)
        glyph = glyph.resize((glyph.width * scale, glyph.height * scale), Image.Resampling.NEAREST)
        # Color glyphs with the same quiet UI off-white.
        coloured = Image.new("RGBA", glyph.size, "#eee9df")
        coloured.putalpha(glyph.getchannel("A"))
        spec.alpha_composite(coloured, (px + int(r["xoffset"]) * scale, baseline_y + int(r["yoffset"]) * scale))
        px += int(r["xadvance"]) * scale
spec.convert("RGB").save(ROOT / "kindling-ui-16-specimen.png")
print(len(records), "glyphs", line_height, "line height", "fallback symbols", "".join(fallback_chars))
