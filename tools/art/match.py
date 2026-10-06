#!/usr/bin/env python3
"""Colour matching (IMPLEMENTATION.md, the art lane): a level fitted to the level above in four numbers (lightness, hue,
colourfulness, contrast) that keep its accents at least 90% of band 0's.

    python3 tools/art/match.py <level.png> <above.png> <band0.png> <out.png>
        prints the four numbers as the record writes them, and the level's measures before and after

Every measure and every change comes from `kindling look` (look.py), so nothing is computed twice. The numbers bring
the level's mean lightness, the hue of its mean colour and its mean colourfulness to those of the level above, and
its spread of lightness toward it, by at most 30% either way; within that, the contrast is raised if the accents
would fall under the line (matching the spread fully lowered the accents, so calibration
keeps accents, not only the spread). A level still short is reported, never stretched into speckle. What rounding to
whole units leaves of a level's colour is settled last by moving the level one unit at most (settle).

Implements PRE-20 and PRE-22, see A5.4.
"""

import itertools
import sys

import numpy as np

import look
import texels

ACCENT_SHARE = 0.9  # each band's accents at least 90% of band 0's
CONTRAST_LEAST, CONTRAST_MOST = 70.0, 130.0  # the contrast change allowed, in percent
CONTRAST_TILE_MOST = 150.0  # a middle or far tile's, fitted to the near tile: your OK of 6 October 2026
DRIFT_LIGHTNESS = 2.0  # hundredths of OKLab lightness a level may drift from band 0's: 0.02
DRIFT_HUE = 5.0  # degrees of hue a level may drift from band 0's


def _wrap(deg):
    return (deg + 180.0) % 360.0 - 180.0


def accents(t):
    """A level's accents, measured with its wrap round it (small levels repeated to at least 64 texture pixels)."""
    return look.stats(look.tiled(t)).get("accents")


def numbers(level_stats, target_stats):
    """The four numbers that bring one picture's measures to another's."""
    s, t = level_stats, target_stats
    hue = _wrap(t["hue"] - s["hue"]) if s["colourfulness"] > 0.5 and t["colourfulness"] > 0.5 else 0.0
    colour = 100.0 * t["colourfulness"] / s["colourfulness"] if s["colourfulness"] > 0.05 else 100.0
    contrast = 100.0 * t["contrast"] / s["contrast"] if s["contrast"] > 0.05 else 100.0
    return [t["lightness"] - s["lightness"], hue, colour, contrast]


def calibrate(level, target_stats, contrast, rounds=3):
    """(the level adjusted toward the target with the given contrast change, its four numbers): lightness, hue and
    colourfulness are brought to the target's and refined over a few rounds so clipping at the ends of the colour
    range is made up, each round applied afresh to the original level."""
    full = numbers(look.stats(level), target_stats)
    total = full[:3] + [contrast]
    out = look.adjust(level, *total)
    for _ in range(rounds - 1):
        d = numbers(look.stats(out), target_stats)
        total = [total[0] + d[0], _wrap(total[1] + d[1]), total[2] * d[2] / 100.0, total[3]]
        out = look.adjust(level, *total)
    return out, total


def settle(level, target_stats):
    """(the level, the move): the level moved by at most one unit of red, green and blue, every texture pixel alike,
    to whichever of the 27 moves brings its lightness and the hue of its mean colour nearest the target's, each
    against the line it may drift by (DRIFT_LIGHTNESS, DRIFT_HUE). Calibration's rounding to whole units leaves a
    small level, or one of a single flat colour, short of its hue by several degrees where it is grey, as birch bark
    is; one unit moves it most of the way, and never shows."""
    t = target_stats
    best = None
    for move in itertools.product((0, -1, 1), repeat=3):
        moved = np.clip(level.astype(np.int16) + np.array(move, np.int16), 0, 255).astype(np.uint8)
        s = look.stats(moved)
        dl = (s["lightness"] - t["lightness"]) / DRIFT_LIGHTNESS
        coloured = s["colourfulness"] > 0.5 and t["colourfulness"] > 0.5
        dh = _wrap(s["hue"] - t["hue"]) / DRIFT_HUE if coloured else 0.0
        key = (round(dl * dl + dh * dh, 6), sum(abs(m) for m in move))
        if best is None or key < best[0]:
            best = (key, moved, move)
    return best[1], best[2]


def fit(level, above, band0_accents, rounds=3):
    """(the level fitted to the level above, its four numbers, its accents, whether the accents hold).
    The contrast follows the level above but changes by at most 30% either way, and is raised within that bound
    if the accents would fall under the line; stretching a level's contrast further only blows its few remaining
    marks up into speckle, so a shortfall past the bound is reported instead (look at the sheet)."""
    target = look.stats(above)
    floor = ACCENT_SHARE * band0_accents
    wanted = min(CONTRAST_MOST, max(CONTRAST_LEAST, numbers(look.stats(level), target)[3]))
    best = None
    for contrast in sorted({wanted, *[c for c in (110.0, 120.0, CONTRAST_MOST) if c > wanted]}):
        out, nums = calibrate(level, target, contrast, rounds)
        a = accents(out)
        if a is None:
            return out, nums, None, True
        if best is None or a > best[2]:
            best = (out, nums, a)
        if a >= floor:
            return out, nums, a, True
    return best[0], best[1], best[2], False


def fit_to_near(level, near, rounds=3, step=10.0, reference=None):
    """(a level drawn from its own picture fitted to the level it follows, its four numbers, its accents, whether
    they hold): a middle or far tile's first level to the near tile's band 0, or a redrawn level to the level above
    it. Lightness, hue and colourfulness are brought to `near`'s, and the contrast set as fit() sets it, then raised
    a step at a time while the accents fall under 90% of the reference's (the near tile's band 0, A5.3; `near` itself
    if None) and its texture pixel contrast stays within `near`'s own. Such a level is GPT's own picture of the
    material, fitted to another, not stretched from the level above, so its contrast may pass fit()'s bound, up to
    CONTRAST_TILE_MOST."""
    near_accents = accents(near if reference is None else reference)
    target = look.stats(near)
    out, nums, a, ok = fit(level, near, near_accents, rounds)
    contrast = nums[3]
    while not ok and contrast + step <= CONTRAST_TILE_MOST:
        contrast += step
        trial, trial_nums = calibrate(level, target, contrast, rounds)
        if look.stats(trial)["texel_contrast"] > target["texel_contrast"]:
            break
        out, nums, a = trial, trial_nums, accents(trial)
        ok = a is None or a >= ACCENT_SHARE * near_accents
    return out, nums, a, ok


def describe(nums, move=(0, 0, 0)):
    """The four numbers as the record writes them, and settle's move if there was one."""
    lightness, hue, colour, contrast = nums
    words = (
        f"lightness {lightness:+.1f}%, hue {hue:+.1f} degrees, colourfulness {colour:.0f}%, contrast {contrast:.0f}%"
    )
    if any(move):
        words += f", then every texture pixel moved {tuple(int(m) for m in move)} in red, green and blue for rounding"
    return words


def main(argv):
    if len(argv) != 4:
        print(__doc__.split("\n\n")[1])
        return 2
    level, above, band0 = (texels.load(p) for p in argv[:3])
    b0 = accents(band0)
    out, nums, a, ok = fit(level, above, b0)
    texels.save_png(out, argv[3])
    before, after, target = look.stats(level), look.stats(out), look.stats(above)
    print(describe(nums))
    for name, s in (("level", before), ("matched", after), ("above", target)):
        print(f"  {name:8s}" + " ".join(f"{k} {v:.2f}" for k, v in s.items()))
    held = "n/a" if a is None else f"{a:.2f} against band 0's {b0:.2f} ({a / b0:.0%})"
    print(f"  accents {held}{'' if ok else ': UNDER the 90% line'}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
