"""The red deer's coat, drawn by code to the layout of art/models/deer.blend (A6.3, A6.4): a summer coat of short
red-brown hair, darker along the back and the neck, pale beneath, a cream rump patch with a dark rim, grey-brown legs
and face, a dark muzzle, nose and hooves; its hair in short strands running down each piece, as the hair lies.

    blender -b art/models/deer.blend --python tools/art/layout.py -- <layout.json>
    python3 art/models/paint_deer.py <layout.json>

Writes art/textures/deer_coat, a material with its levels, its layout.png and its record. Each texture pixel takes
its colour by where it lies on the deer at rest (A6.1: a species is its proportions, colours and markings); the
deer faces -y, its rump toward +y.

Implements PRE-27 and PRE-46, see A6.1, A6.3 and A6.4.
"""

import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "tools", "art"))
import paint  # noqa: E402

COAT = (160, 104, 66)
BACK = (128, 80, 50)
NECK = (122, 88, 64)
BELLY = (186, 156, 118)
RUMP = (214, 196, 156)
RIM = (74, 50, 36)
LEGS = (130, 102, 80)
FACE = (140, 106, 78)
MUZZLE = (70, 55, 44)
NOSE = (44, 38, 35)
HOOF = (40, 35, 32)


def coat(layout):
    s = layout.side
    shape = (s, s)
    pos, nor = layout.position, layout.normal
    x, y, z = pos[..., 0], pos[..., 1], pos[..., 2]
    img = np.zeros(shape + (3,))
    img[:] = COAT
    torso = layout.where(part="body_torso")
    neck = layout.where(part="body_neck")
    head = layout.where(part="body_head")
    legs = layout.where(part="leg_")
    up, down = nor[..., 2], -nor[..., 2]
    img[(torso | neck) & (up > 0.55)] = BACK
    img[neck & (up <= 0.55)] = NECK
    img[torso & (down > 0.35)] = BELLY
    img[legs] = LEGS
    img[legs & (np.abs(x) < 0.12) & (nor[..., 0] * np.sign(x + 1e-9) < -0.3)] = BELLY  # the inner legs, paler
    img[legs & (z > 0.75) & (y > 0.3)] = COAT  # the haunches carry the coat down
    rump = torso & (y > 0.62) & (z > 0.80) & (nor[..., 1] > -0.2)
    rim = torso & (y > 0.56) & (z > 0.74) & ~rump & (nor[..., 1] > -0.4)
    img[rim] = RIM
    img[rump] = RUMP
    img[head] = FACE
    img[head & (y < -0.98)] = MUZZLE
    img[layout.where(role="skin")] = NOSE
    hooves = layout.where(role="hoof")
    img[hooves] = HOOF
    # an eye on each side of the head, the dark texture pixel nearest its place at rest
    for side in (-1, 1):
        d = np.where(head, np.hypot(np.hypot(x - side * 0.115, y + 0.84), z - 1.66), np.inf)
        r, c = np.unravel_index(np.argmin(d), d.shape)
        if d[r, c] < 0.03:
            img[r, c] = (30, 24, 20)
    # the hair: close shades in soft patches, and short strands running down each piece
    hair = layout.where(role="hair")
    tone = paint.field(shape, 6, 41)[..., None]
    img[hair] += (tone * 4.0)[hair]
    img[hair & paint.strokes(shape, 0, 2, 0.12, 42)] *= 1.10
    img[hair & paint.strokes(shape, 0, 3, 0.10, 43)] *= 0.86
    img[hooves & paint.strokes(shape, 1, 2, 0.15, 44)] *= 1.25
    return np.clip(img, 0, 255)


def main(path):
    data = paint.load(path)
    layout = paint.Layout(data, "deer_coat")
    img = coat(layout).astype(np.uint8)
    top = {
        "about": "the red deer's summer coat: red-brown hair, a pale belly and rump patch, dark muzzle and hooves",
        "route": "code",
        "sources": [],
        "original_sha256": [],
        "c2pa": [],
        "requests": [],
        "made": (
            "drawn by code to the deer_coat atlas of art/models/deer.blend (art/models/paint_deer.py): each texture "
            "pixel coloured by where it lies on the deer at rest, its back and neck darker, its belly and rump patch "
            "pale, its legs and face grey-brown, with short strands of hair running down each piece"
        ),
        "regrid_loss": "not re-gridded: drawn by code",
        "truth": (
            "art lane, 2026-10-06, looked at enlarged: a red deer's summer coat as the species wears it (red-brown, "
            "pale rump patch and belly, no spots on the adult); the guide pictures' deer are its reference"
        ),
        "approved": "waiting",
    }
    made = paint.save(os.path.join(ROOT, "art", "textures", "deer_coat"), img, layout, top, ROOT)
    print(f"deer_coat: {layout.side} square, {layout.covered.mean():.0%} covered, {len(made)} levels")


if __name__ == "__main__":
    main(sys.argv[1])
