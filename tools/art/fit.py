"""The calibration of a level (A5.4, steps 5 and 6): a few numbers fitted until a level matches the one it follows, with
every measure read from `kindling look` (look.py), never computed here.

- fit_reduction() makes the level below another: averaged, then sharpened by the amount that gives it the accents of
  the level above (plain averaging loses about a fifth of them, A5.3), then moved in lightness, hue and
  colourfulness to the reference's.
- calibrate() moves a picture to a reference in those three numbers only, for a tile's first level, since each tile
  keeps the contrast of its own distance.

Implements PRE-20 and PRE-22, see A5.3 and A5.4.
"""

import look
import tiles


def numbers(stats):
    """The record's words for a fit: the change made, in the four numbers the engine's colour change takes."""
    return "lightness {:+.1f}; hue {:+.1f}; colourfulness {:.0f}%; contrast {:.0f}%".format(*stats)


def moved(picture, reference):
    """The picture moved to the reference's lightness, hue and colourfulness (look.stats of each), and the four
    numbers of the change; contrast is left alone."""
    mine, want = look.stats(look.tiled(picture)), look.stats(look.tiled(reference))
    lightness = want["lightness"] - mine["lightness"]
    hue = want["hue"] - mine["hue"]
    if hue > 180:
        hue -= 360
    elif hue < -180:
        hue += 360
    colourfulness = 100.0 * want["colourfulness"] / mine["colourfulness"] if mine["colourfulness"] else 100.0
    out = look.adjust(picture, lightness, hue, colourfulness, 100.0)
    return out, (lightness, hue, colourfulness, 100.0)


def calibrate(picture, reference):
    """A tile's first level moved to the reference level's colour: (the picture, the record's words)."""
    out, change = moved(picture, reference)
    return out, numbers(change)


def fit_reduction(above, reference=None, rounds=9, contrast=100.0):
    """The level below `above`, from tiles.reduce with the unsharp amount, found by halving between 0 and 4, whose
    accents come nearest the level above's; then moved to `reference` (default: the level above) in colour, and its
    contrast scaled by `contrast` percent (100 keeps the accents the unsharp amount found; less calms a level whose tile
    would otherwise repeat in a visible lattice). Returns the picture, the amount, the change in colour (the four
    numbers) and the record's words."""
    want = look.stats(look.tiled(above))
    lo, hi = 0.0, 4.0
    best = None
    for _ in range(rounds):
        amount = (lo + hi) / 2
        got = look.stats(look.tiled(tiles.reduce(above, amount)))
        miss = (got.get("accents") or 0.0) - (want.get("accents") or 0.0)
        if best is None or abs(miss) < best[0]:
            best = (abs(miss), amount)
        if miss < 0:
            lo = amount
        else:
            hi = amount
    amount = best[1]
    picture, change = moved(tiles.reduce(above, amount), reference if reference is not None else above)
    change = (change[0], change[1], change[2], contrast)
    return picture, amount, change, f"unsharp {amount:.2f}; " + numbers(change)


def reduced(above, amount, change):
    """The level below `above` made with numbers fitted on another picture, so every version of a tile is calibrated
    alike."""
    return look.adjust(tiles.reduce(above, amount), change[0], change[1], change[2], change[3])
