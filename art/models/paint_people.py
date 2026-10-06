"""The person's textures, drawn by code to the layout of art/models/people.blend (A6.3, A6.4): the body's skin,
hair and a calm face; the tunic, leggings and boots of sewn hide with fur trims, their seams along each piece's wrap
and hems at its ends.

    blender -b art/models/people.blend --python tools/art/layout.py -- <layout.json>
    python3 art/models/paint_people.py <layout.json>

Writes art/textures/person_body, person_tunic, person_leggings and person_boots, each a material with its levels,
its layout.png and its record. The hides are the camp's own (art/textures/hide_*), laid at their true size in
metres as every texture is (A6.4), so a garment is the same leather as a tent; recipes vary the tints by person
and people (PRE-43), so these are one person's colours. The face is a calm one, about 8 texture pixels from chin to
hairline as the body's proportions give it: the 12 face states of A6.1 are designs drawn later.

Implements PRE-27 and PRE-42, see A6.1, A6.3 and A6.4.
"""

import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "tools", "art"))
import paint  # noqa: E402
import record  # noqa: E402
import texels  # noqa: E402

TRUTH = (
    "art lane, 2026-10-06, looked at enlarged: tailored clothing of sewn hide with fur trims, as finds of the Upper "
    "Palaeolithic show (eyed needles, sewn seams); no metal, buttons, woven cloth or later dress; skin and hair "
    "plain, no paint or beads yet (each people's style adds them, PRE-43)"
)


def hide(name):
    return texels.load(os.path.join(ROOT, "art", "textures", name, "b0.png")).astype(float)


def laid(tile, shape):
    """A tiled material laid on the canvas at its true size, texture pixel for texture pixel."""
    ys = np.arange(shape[0]) % tile.shape[0]
    xs = np.arange(shape[1]) % tile.shape[1]
    return tile[np.ix_(ys, xs)]


def fur(shape, seed):
    """Short pale fur: a cream base with strands 2 or 3 texture pixels long running down the piece."""
    base = paint.shades(
        (214, 200, 172), [(-14, -14, -14), (0, 0, 0), (10, 9, 8)], paint.field(shape, 5, seed), [0.3, 0.75]
    )
    img = base.copy()
    img[paint.strokes(shape, 0, 3, 0.18, seed + 1)] += (16, 15, 13)
    img[paint.strokes(shape, 0, 2, 0.14, seed + 2)] -= (28, 30, 32)
    return np.clip(img, 0, 255)


def sew(img, layout, mask, darker=0.72):
    """Seams and hems on the pieces of `mask`: stitches, every other texture pixel, down each piece's wrapping
    edges, and a darker hem along its ends."""
    left, right, up, down = layout.edges()
    rows = np.arange(img.shape[0])[:, None]
    seam = mask & ((left == 1) | (right == 1)) & (rows % 2 == 0)
    hem = mask & ((up == 1) | (down == 1))
    img[seam] *= darker
    img[hem] *= 0.85
    return img


def body(layout):
    s = layout.side
    shape = (s, s)
    skin = paint.shades((186, 132, 100), [(-9, -8, -7), (0, 0, 0), (6, 5, 4)], paint.field(shape, 6, 11), [0.25, 0.8])
    img = skin.copy()
    hair = layout.where(role="hair")
    strands = paint.shades((62, 44, 33), [(-12, -9, -7), (0, 0, 0), (14, 10, 7)], paint.field(shape, 3, 12), [0.3, 0.7])
    strands[paint.strokes(shape, 0, 3, 0.2, 13)] += (16, 12, 9)
    img[hair] = strands[hair]
    # the hairline: hair beside skin a shade darker, so the face reads
    near = np.zeros(shape, bool)
    skin_mask = layout.where(role="skin")
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        near |= np.roll(np.roll(skin_mask, dy, 0), dx, 1)
    img[hair & near] *= 0.82
    face(img, layout)
    return img


def face(img, layout):
    """A calm face on the head's front: eyes and brows as single dark texture pixels, a nose's shade and a mouth,
    each at the texture pixel nearest its place on the head at rest."""
    head = layout.where(role="skin", part="body_head") & (layout.normal[..., 1] < -0.45)
    pos = layout.position

    def at(x, z, colour):
        d = np.where(head, np.hypot(pos[..., 0] - x, pos[..., 2] - z), np.inf)
        r, c = np.unravel_index(np.argmin(d), d.shape)
        if np.isfinite(d[r, c]) and d[r, c] < 0.02:
            img[r, c] = colour

    for side in (-1, 1):
        at(side * 0.030, 1.556, (38, 28, 24))  # an eye
        at(side * 0.032, 1.575, (120, 82, 62))  # its brow
    at(0.0, 1.533, (160, 108, 82))  # the shade under the nose
    for x in (-0.008, 0.008):
        at(x, 1.508, (128, 74, 62))  # the mouth


def tunic(layout):
    s = layout.side
    img = laid(hide("hide_pale"), (s, s))
    furs = layout.where(role="fur")
    img[furs] = fur((s, s), 21)[furs]
    return sew(img, layout, layout.where(role="hide"))


def leggings(layout):
    s = layout.side
    return sew(laid(hide("hide_dark"), (s, s)), layout, layout.where(role="hide"))


def boots(layout):
    s = layout.side
    img = laid(hide("hide_smoked"), (s, s))
    furs = layout.where(role="fur")
    img[furs] = fur((s, s), 31)[furs]
    sole = layout.where(role="hide") & (layout.normal[..., 2] < -0.7)
    img[sole] = img[sole] * 0.55
    return sew(img, layout, layout.where(role="hide") & ~sole)


def sources(*materials):
    """The sources, digests, C2PA and requests of the camp's materials a texture lays."""
    out = {"sources": [], "original_sha256": [], "c2pa": [], "requests": []}
    for m in materials:
        rec = record.read(os.path.join(ROOT, "art", "textures", m, "record.toml"))
        for i, s in enumerate(rec["sources"]):
            if s not in out["sources"]:
                for k in out:
                    out[k].append(rec[k][i])
    return out


ATLASES = {
    "person_body": (body, "the person's body: skin, hair and a calm face", ()),
    "person_tunic": (tunic, "the person's tunic of pale hide with a fur collar and cuffs, sewn", ("hide_pale",)),
    "person_leggings": (
        leggings,
        "the person's leggings of dark hide, sewn down the inside of each leg",
        ("hide_dark",),
    ),
    "person_boots": (boots, "the person's soft boots of smoked hide with a fur band at the top", ("hide_smoked",)),
}


def main(path):
    data = paint.load(path)
    for name, (draw, about, laid_from) in ATLASES.items():
        layout = paint.Layout(data, name)
        img = np.clip(draw(layout), 0, 255).astype(np.uint8)
        top = {
            "about": about,
            "route": "code",
            **sources(*laid_from),
            "made": (
                f"drawn by code to the {name} atlas of art/models/people.blend (art/models/paint_people.py): "
                + (f"{', '.join(laid_from)} laid at its true size in metres, " if laid_from else "")
                + "with its seams, hems and trims where the layout's pieces wrap and end"
                if laid_from
                else f"drawn by code to the {name} atlas of art/models/people.blend (art/models/paint_people.py): "
                "skin in three close shades, hair in strands on the head's hair faces, and a calm face placed by "
                "where each texture pixel lies on the head at rest"
            ),
            "regrid_loss": "not re-gridded: drawn by code",
            "truth": TRUTH,
            "approved": "waiting",
        }
        made = paint.save(os.path.join(ROOT, "art", "textures", name), img, layout, top, ROOT)
        print(f"{name}: {layout.side} square, {layout.covered.mean():.0%} covered, {len(made)} levels")


if __name__ == "__main__":
    main(sys.argv[1])
