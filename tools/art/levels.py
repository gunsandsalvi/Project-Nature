#!/usr/bin/env python3
"""A tile's levels (IMPLEMENTATION.md, the art lane): each level after the ones drawn for their bands made by the code
reduction from the level above, its colours matched to the material's, and only as many of its marks drawn again as
hold the accents, so the band farther out is bolder and calmer, never speckled and never averaged (A5.3).

    python3 tools/art/levels.py <out folder> <level 0.png> [<level 1.png>...] [--reference <band 0.png>]
        the levels given are the tile's first and any designed after it; writes b<n>.png for the rest, down to one
        texture pixel, and prints each one's way, calibration and accents

For each level made:
1. reduce.Reduction of the level above: its background and its marks, strongest first.
2. Its four colour numbers found on the background alone (match.calibrate: its spread following the level above's
   within 30% either way, its lightness, hue and colourfulness brought to the reference's, the near tile's band 0),
   so its marks never dim its stains; matching every level to band 0 itself, not to the level above, keeps what
   each calibration leaves from adding up level by level.
3. Its marks drawn again with the same contrast, its lightness, hue and colourfulness brought to the reference's
   with them in, none at first, then 1, 2, 4 and so on, until its accents reach 90% of the reference's (A5.3), so
   it takes as few as hold them; if all it may take still fall short, its contrast is raised within the bound as
   match.fit raises it, and the shortfall is reported, never stretched into speckle.
4. What rounding to whole units leaves of its colour settled by a move of one unit at most (match.settle).
A texture drawn to a figure's layout (paint.py) takes none of its marks again and keeps the contrast the reduction
leaves: a figure's eyes and seams fade with distance instead of growing, and its spread is mostly its gaps'.

Implements PRE-22 and PRE-20, see A5.3 and A5.4.
"""

import argparse
import os
import sys

import look
import match
import reduce
import texels

COLOUR = ("lightness", "hue", "colourfulness")


def way(marks, of):
    """The record's words for a level the code reduction made."""
    kept = f"{marks} of its strongest marks drawn again, each 2 texture pixels or more" if marks else "no marks kept"
    return (
        "code reduction of the level above (majority of its own shades, lone texture pixels cleaned, "
        f"{kept}), colours matched to band 0's by code"
    )


def calibration(step):
    """The record's words for a made level's calibration: its four numbers, and the settling move if it took one."""
    return match.describe(step["numbers"], step.get("move", (0, 0, 0)))


def next_level(above, reference_accents, k=12, colour=None, drawn=False, fill=None):
    """The level below `above`: {level, numbers, move, marks, most, accents, ok}. `colour` holds the measures whose
    lightness, hue and colourfulness every level keeps (the reference band 0's); the level above's if None. For a
    texture `drawn` to a layout, none of its marks is drawn again, so a figure's eyes and seams fade with distance
    and never grow, and its contrast is left as the reduction leaves it, its spread being mostly its gaps'. `fill`,
    if given, is done to the reduction's level before its colours are matched (a drawn atlas's gaps filled again)."""
    r = reduce.Reduction(above, k)

    def draw(marks):
        return fill(r.draw(marks)) if fill else r.draw(marks)

    spread = look.stats(above)
    target = dict(spread)
    if colour is not None:
        target.update({key: colour[key] for key in COLOUR})
    plain = draw(0)
    wanted = min(match.CONTRAST_MOST, max(match.CONTRAST_LEAST, match.numbers(look.stats(plain), spread)[3]))
    _, nums = match.calibrate(plain, target, 100.0 if drawn else wanted)
    floor = 0.0 if drawn else match.ACCENT_SHARE * reference_accents
    tries, n = [0], 1
    while n < r.most and not drawn:
        tries.append(n)
        n *= 2
    if r.most and not drawn:
        tries.append(r.most)
    best = None
    for marks in tries:
        # the contrast found on the background alone; lightness, hue and colourfulness brought to the reference's
        # with the marks in, so the level's colour holds whatever marks it takes
        out, numbers = match.calibrate(draw(marks), target, nums[3])
        a = match.accents(out)
        if a is None or a >= floor:
            best = dict(level=out, numbers=numbers, marks=marks, most=r.most, accents=a, ok=True)
            break
        if best is None or a > best["accents"]:
            best = dict(level=out, numbers=numbers, marks=marks, most=r.most, accents=a, ok=False)
    if not best["ok"]:
        _, nums, a, _ = match.fit(draw(r.most), above, reference_accents)  # contrast raised within the bound
        if a is None or a > best["accents"]:
            out, nums = match.calibrate(draw(r.most), target, nums[3])  # its colour brought to the reference's
            a = match.accents(out)
            best = dict(level=out, numbers=nums, marks=r.most, most=r.most, accents=a, ok=a is None or a >= floor)
    best["level"], best["move"] = match.settle(best["level"], target)
    return best


def make(given, reference, k=12):
    """The levels after `given` (the tile's first levels, largest first) down to one texture pixel, each as
    next_level's dictionary; `reference` is the band 0 whose colour and accents they keep."""
    acc0 = match.accents(reference)
    colour = look.stats(reference)
    out, above = [], given[-1]
    while min(above.shape[:2]) > 1:
        step = next_level(above, acc0, k, colour)
        out.append(step)
        above = step["level"]
    return out


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("out")
    ap.add_argument("levels", nargs="+")
    ap.add_argument("--reference", help="the band 0 whose accents the levels keep (the first level given if left out)")
    ap.add_argument("--shades", type=int, default=12)
    args = ap.parse_args(argv)
    given = [texels.load(p) for p in args.levels]
    reference = texels.load(args.reference) if args.reference else given[0]
    acc0 = match.accents(reference)
    os.makedirs(args.out, exist_ok=True)
    short = 0
    for n, step in enumerate(make(given, reference, args.shades), start=len(given)):
        path = os.path.join(args.out, f"b{n}.png")
        texels.save_png(step["level"], path)
        share = "n/a" if step["accents"] is None else f"{step['accents'] / acc0:.0%}"
        short += not step["ok"]
        print(
            f"{path}: {way(step['marks'], step['most'])}; {calibration(step)}; accents {share}"
            f"{'' if step['ok'] else ' UNDER the 90% line'}"
        )
    return 1 if short else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
