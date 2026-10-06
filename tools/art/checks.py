#!/usr/bin/env python3
"""The art lane's checks (IMPLEMENTATION.md, T2.3a.1): every material in art/textures/ against its records and the
lines the plan sets, tile by tile.

    python3 tools/art/checks.py [--root DIR] [<name>...]
        every check on every material, or on those named; prints one line a check, exits 1 if any fails

A material is its folder: its near tile (art/textures/<name>/), and for a big surface its middle/ and far/ tiles
(A5.3); each tile may have versions v2/ to v4/; a material for wrapped parts has its wrap atlas in wraps/ (A6.4).
Each tile has its own record and levels. A big surface is seen at bands 0 and 1 in its near tile, 2 and 3 in its
middle tile and 4 to 6 in its far tile; a material with one tile at bands 0 to 6 in it.

The checks, each with its line:
- record: the record has the plan's keys, whole numbers and texts only; every file it names is there and each
  level's digest is right; each source has its original's digest and whether it carried its C2PA record, a request,
  a truth check and an approval (or "waiting"); its first band is its tile's (0 near, 2 middle, 4 far) and its texture
  pixels a metre that band's; each level is half the one above;
- stale: each level was made from the level it names as it is now: the level above, or for a version or a wrap
  atlas the tile's own level of the same size;
- loss: the re-grid lost at most 10% of the source's colour detail, for the first level and for each redrawn level
  (each band's own regrid_loss); a loss the owner kept by name is reported, not failed;
- seams: the first level tiles, its step across the wrap at most 1.2 times the median step inside (tile.seams);
  a texture drawn to a figure's layout (its folder keeps layout.png) is not tiled, so its seams, light and repeat
  are not measured, and a regular texture drawn by code, such as a weave, repeats by its rule, which is reported;
  a redrawn level's wrap steps no more than its own ordinary boundaries do (tile.wrap_rank, at most 1): in a smaller
  level the 1.2 line is noise, 10 to 33% of a level's ordinary boundaries already passing it;
- light: no painted light in the first level, the plane through the lightness of a 4 x 4 grid of windows sloping at
  most 0.02 of OKLab lightness across the tile;
- repeat: no strong repeat inside the first level, its autocorrelation beyond 15 cm at most 0.2, or at most its
  source's own where the material's grain repeats (a bark's fissures), so only tiling fails it;
- contrast: the first level's texture pixel contrast within a quarter of its source's (source.png, the source on
  its grid); a version or a wrap atlas, made from the tile's own levels, has none to compare;
- accents: every band the material is seen at, from the tile that serves it, at least 90% of the near tile's band
  0 (A5.3); for a material with one tile, bands 1 to 3, its bands 4 to 6 reported only, since there its texture
  pixel is larger than its marks; for a texture drawn to a layout, reported only, its canvas being mostly the gaps
  between its pieces;
- drift: every level of every tile within 0.02 of lightness and 5 degrees of hue of the near tile's band 0, so the
  ground keeps its colour across bands and tiles; hue is left out where both are too grey for a hue to mean anything
  (colourfulness under 0.02);
- joints (a tile with versions): every version joins every other at every band it serves, its step where they meet
  at most 1.2 times the median step inside (tile.joints).
Lightness, hue, colourfulness, texture pixel contrast and accents come from `kindling look` (look.py), the same C++
the engine's checks use.

Implements PRE-20, PRE-22 and PRE-42, see A5.3 and A5.4.
"""

import argparse
import os
import re
import sys

import numpy as np

import look
import match
import record
import texels
import tile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HEX = re.compile(r"^[0-9a-f]{64}$")
LOSS_MOST = 10.0  # percent
SEAMS_MOST = 1.2
RANK_MOST = 1.0
SLOPE_MOST = 2.0  # hundredths of OKLab lightness across the tile: 0.02
REPEAT_MOST = 0.2
CONTRAST_SHARE = 0.25
ACCENT_SHARE = match.ACCENT_SHARE
DRIFT_LIGHTNESS = match.DRIFT_LIGHTNESS  # hundredths: 0.02
DRIFT_HUE = match.DRIFT_HUE  # degrees
GREY = 2.0  # colourfulness in hundredths under which a hue means little
KEPT = "kept by the owner"
FIRST_BAND = {"near": 0, "middle": 2, "far": 4}
SERVES = {"near": (0, 1), "middle": (2, 3), "far": (4, 5, 6)}  # the bands each tile of a big surface serves
VERSIONS = ("v2", "v3", "v4")


def slope(t, n=4):
    """The painted light left in a level: a plane through the lightness of an n x n grid of windows (from kindling
    look), its rise across the whole tile in x plus that in y, in hundredths of OKLab lightness."""
    h, w = t.shape[:2]
    pts, ls = [], []
    for i in range(n):
        for j in range(n):
            win = t[i * h // n : (i + 1) * h // n, j * w // n : (j + 1) * w // n]
            ls.append(look.stats(win)["lightness"])
            pts.append(((j + 0.5) / n, (i + 0.5) / n))
    a = np.column_stack([np.ones(len(pts)), np.array(pts)])
    coef, *_ = np.linalg.lstsq(a, np.array(ls), rcond=None)
    return float(abs(coef[1]) + abs(coef[2]))


def tiles(folder):
    """The material's tiles: [(where, kind, folder, its origin's where or None)], the near tile first. kind is near,
    middle or far; a version's origin is the tile it was made from, and a wrap atlas's the near tile."""
    out = [("", "near", folder, None)]
    for kind in ("middle", "far"):
        sub = os.path.join(folder, kind)
        if os.path.isdir(sub):
            out.append((kind, kind, sub, None))
    for where, kind, path, _ in list(out):
        for v in VERSIONS:
            sub = os.path.join(path, v)
            if os.path.isdir(sub):
                out.append((os.path.join(where, v) if where else v, kind, sub, where))
    if os.path.isdir(os.path.join(folder, "wraps")):
        out.append(("wraps", "near", os.path.join(folder, "wraps"), ""))
    return out


def loss_check(text, what):
    """(passed, words) for a re-grid loss as a record writes it."""
    text = str(text)
    m = re.match(r"^\s*([0-9.]+)\s*%", text)
    if not m:
        return ("not" in text, f"{what} {text!r}")
    if KEPT in text:
        return (None, f"{what} {m.group(1)}%, over the {LOSS_MOST:.0f}% line, {text[m.end() :].strip(' ,;')}")
    return (float(m.group(1)) <= LOSS_MOST, f"{what} {m.group(1)}%")


def read_tile(path, root):
    """(record or None, levels, digests, problems): the tile's record and its levels as files."""
    rec_path = os.path.join(path, "record.toml")
    if not os.path.exists(rec_path):
        return None, [], [], ["no record.toml"]
    try:
        rec = record.read(rec_path)
    except Exception as e:  # a record that will not parse fails as a record
        return None, [], [], [f"record.toml does not read: {e}"]
    bad = record.problems(rec)
    levels, digests = [], []
    for b in rec.get("band", []):
        f = os.path.join(root, b.get("file", ""))
        if not os.path.exists(f):
            bad.append(f"band {b.get('level')}: no file {b.get('file')}")
            levels.append(None)
            digests.append(None)
            continue
        d = texels.sha256(f)
        digests.append(d)
        if d != b.get("sha256"):
            bad.append(f"band {b.get('level')}: its digest is not its file's")
        levels.append(texels.load(f))
    return rec, levels, digests, bad


def check_tile(path, kind, root, near, origin=None, big=False, wraps=False):
    """[(check, passed, what was found)] for one tile: `near` is the material's near tile as read_tile gives it, the
    reference for accents and drift; `origin` likewise the tile a version or wrap atlas was made from."""
    rec, levels, digests, bad = read_tile(path, root)
    if rec is None:
        return [("record", False, bad[0])]
    for k in ("sources", "requests"):
        for p in rec.get(k, []):
            if p and not os.path.exists(os.path.join(root, p)):
                bad.append(f"no {p}")
    for d in rec.get("original_sha256", []):
        if not HEX.match(d):
            bad.append(f"original digest {d!r} is not a SHA-256")
    for c in rec.get("c2pa", []):
        if c not in ("present", "absent"):
            bad.append(f"c2pa is {c!r}, not present or absent")
    for k in ("truth", "approved"):
        if not str(rec.get(k, "")).strip():
            bad.append(f"no {k} written")
    first = rec.get("first_band")
    if isinstance(first, int) and first != FIRST_BAND[kind]:
        bad.append(f"first_band is {first}, not {FIRST_BAND[kind]} for a {kind} tile")
    if isinstance(first, int) and rec.get("texels_a_metre") != 64 >> first:
        bad.append(f"texels_a_metre is {rec.get('texels_a_metre')}, not {64 >> first} at band {first}")
    size = rec.get("tile_texels")
    for n, t in enumerate(levels):
        if t is not None and isinstance(size, int) and t.shape[:2] != (size >> n, size >> n):
            bad.append(f"band {n} is {t.shape[1]} x {t.shape[0]}, not {size >> n} square")
    out = [("record", not bad, "; ".join(bad) or f"{len(levels)} levels, files and digests right")]
    if any(t is None for t in levels) or not levels:
        return out
    bands = rec["band"]
    if origin is None:
        stale = [str(n) for n in range(1, len(bands)) if bands[n].get("made_from") != digests[n - 1]]
        out.append(("stale", not stale, f"levels {', '.join(stale)} made from an older level" if stale else "none"))
    else:
        theirs = origin[2]
        stale = [str(n) for n in range(len(bands)) if n >= len(theirs) or bands[n].get("made_from") != theirs[n]]
        out.append(("stale", not stale, f"levels {', '.join(stale)} made from an older level" if stale else "none"))
    losses = [loss_check(rec.get("regrid_loss", ""), "first level")]
    losses += [loss_check(b["regrid_loss"], f"level {n}") for n, b in enumerate(bands) if "regrid_loss" in b]
    failed = [w for ok, w in losses if ok is False]
    kept = [w for ok, w in losses if ok is None]
    ok = False if failed else (None if kept else True)
    out.append(
        ("loss", ok, "; ".join(failed + kept) or "; ".join(w for _, w in losses) + f" (at most {LOSS_MOST:.0f}%)")
    )
    drawn = os.path.exists(os.path.join(path, "layout.png"))  # drawn to a figure's layout, never tiled
    src_path = os.path.join(path, "source.png")
    src = texels.load(src_path) if os.path.exists(src_path) else None
    if drawn:
        out.append(("seams", None, "drawn to its layout (layout.png), not tiled: no wrap to join"))
        out.append(("repeat", None, "drawn to its layout, not tiled"))
    else:
        sm = tile.seams(levels[0])
        words = [f"first level {sm:.2f} (at most {SEAMS_MOST})"]
        seams_ok = sm <= SEAMS_MOST
        for n, b in enumerate(bands):
            if n and ("regrid_loss" in b or "redraw" in b.get("way", "")):
                rank = tile.wrap_rank(levels[n])
                seams_ok &= rank <= RANK_MOST
                words.append(f"redrawn level {n}'s wrap {rank:.2f} of its ordinary steps (at most {RANK_MOST:.0f})")
        out.append(("seams", seams_ok, "; ".join(words)))
        if origin is None and not wraps:
            sl = slope(levels[0])
            out.append(("light", sl <= SLOPE_MOST, f"slope {sl / 100:.3f} (at most 0.02)"))
        rp = tile.repeat(levels[0])
        line = REPEAT_MOST
        if src is not None:
            own = tile.repeat(src)
            line = max(REPEAT_MOST, own)
        if rec.get("route") == "code" and origin is None and not wraps and rp > line:
            out.append(("repeat", None, f"{rp:.2f}: drawn by code to a regular rule, such as a weave's"))
        else:
            out.append(("repeat", rp <= line, f"{rp:.2f} (at most {line:.2f})"))
    s0 = look.stats(levels[0])
    if src is not None:
        tc_src = look.stats(src)["texel_contrast"]
        share = s0["texel_contrast"] / tc_src if tc_src > 0 else 0.0
        ok = abs(share - 1) <= CONTRAST_SHARE
        out.append(("contrast", ok, f"first level {s0['texel_contrast']:.2f}, source {tc_src:.2f}: {share:.0%}"))
    elif origin is not None or wraps:
        out.append(("contrast", None, "not measured: made from the tile's own levels, with no source of its own"))
    elif drawn or rec.get("route") == "code":
        out.append(("contrast", None, "not measured: drawn by code, with no picture for a source"))
    else:
        out.append(("contrast", None, "not measured: no source on its grid kept as source.png"))
    ref = near[1][0]
    acc0 = match.accents(ref)
    judged, reported = [], []
    first = FIRST_BAND[kind]
    for n in range(len(levels)):
        band = first + n
        if big:
            if band not in SERVES[kind]:
                continue
            (judged if band else reported).append((band, n))
        elif 1 <= band <= 3 and not drawn:
            judged.append((band, n))
        elif 1 <= band <= 6:
            reported.append((band, n))
    short, notes = [], []
    for band, n in judged + reported:
        a = match.accents(levels[n])
        share = 0.0 if a is None or not acc0 else a / acc0
        if (band, n) in judged and share < ACCENT_SHARE:
            short.append(f"band {band} {share:.0%}")
        else:
            notes.append(f"band {band} {share:.0%}" + ("" if (band, n) in judged else " (reported)"))
    if not acc0:
        out.append(("accents", None, "band 0 has no accents to keep"))
    elif judged or reported:
        words = ("under 90%: " + ", ".join(short) + "; " if short else "") + ", ".join(notes)
        out.append(("accents", not short if judged else None, words + " of the near tile's band 0"))
    drift = []
    r0 = look.stats(ref)
    for n, t in enumerate(levels):
        s = look.stats(t)
        dl = abs(s["lightness"] - r0["lightness"])
        dh = abs((s["hue"] - r0["hue"] + 180) % 360 - 180)
        coloured = s["colourfulness"] >= GREY and r0["colourfulness"] >= GREY
        if dl > DRIFT_LIGHTNESS or (coloured and dh > DRIFT_HUE):
            drift.append(f"{first + n} (lightness {dl / 100:+.3f}, hue {dh:.1f})")
    out.append(("drift", not drift, "bands " + ", ".join(drift) if drift else "within 0.02 and 5 degrees"))
    return out


def joint_ratio(a, b):
    """(how far over its line the joint of two versions' same level is, the joint): the line is 1.2, or the tiles'
    own wraps where a small level's are higher, so any version joins any other as well as it joins itself."""
    line = max(SEAMS_MOST, tile.seams(a), tile.seams(b)) + 0.02
    j = tile.joints(a, b)
    return j / line, j


def check_texture(folder, root=ROOT):
    """[(where, check, passed, what was found)] for one material folder; passed is None for a check reported only."""
    found = tiles(folder)
    near = read_tile(found[0][2], root)
    if near[0] is None or not near[1] or any(t is None for t in near[1]):
        return [("", c, ok, w) for c, ok, w in check_tile(found[0][2], "near", root, near)]
    big = any(kind != "near" for _, kind, _, _ in found)
    read = {"": near}
    out = []
    for where, kind, path, origin in found:
        if where not in read:
            read[where] = read_tile(path, root)
        src = read.get(origin) if origin is not None else None
        for c, ok, w in check_tile(path, kind, root, near, src, big, wraps=where == "wraps"):
            out.append((where, c, ok, w))
    for base in [w for w, _, _, o in found if o is None]:
        group = [w for w, _, _, o in found if w == base or o == base and w != "wraps"]
        if len(group) < 2:
            continue
        kind = next(k for w, k, _, _ in found if w == base)
        levels = [read[w][1] for w in group]
        if any(not lv or any(t is None for t in lv) for lv in levels):
            continue
        first = FIRST_BAND[kind]
        worst = (0.0, None, 0.0)  # (how far over its line a joint is, band, the joint)
        for n in range(len(levels[0])):
            band = first + n
            if (band not in SERVES[kind]) if big else band > 6:
                continue
            for a in levels:
                for b in levels:
                    if len(a) > n and len(b) > n and min(a[n].shape[:2]) >= 8:
                        ratio, j = joint_ratio(a[n], b[n])
                        worst = max(worst, (ratio, band, j))
        out.append(
            (
                base,
                "joints",
                worst[0] <= 1.0,
                f"{len(group)} versions join at most {worst[0]:.2f} of their line (the joint {worst[2]:.2f} at band "
                f"{worst[1]}; the line 1.2, or the tiles' own wraps where higher)",
            )
        )
    return out


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("names", nargs="*")
    ap.add_argument("--root", default=ROOT)
    args = ap.parse_args(argv)
    base = os.path.join(args.root, "art", "textures")
    names = args.names or sorted(d for d in os.listdir(base) if os.path.isdir(os.path.join(base, d)))
    failed = 0
    for name in names:
        for where, check, ok, what in check_texture(os.path.join(base, name), args.root):
            failed += ok is False
            label = os.path.join(name, where) if where else name
            print(f"{label:22s} {check:9s} {'note' if ok is None else 'pass' if ok else 'FAIL'}  {what}")
    print(f"{len(names)} materials, {failed} checks failed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
