#!/usr/bin/env python3
"""The art lane's checks (art/BRIEF.md; IMPLEMENTATION.md, T2.3a.1): every texture in art/textures/ against its
record and the lines the brief sets.

    python3 tools/art/checks.py [--root DIR] [<name>...]
        every check on every texture, or on those named; prints one line a check, exits 1 if any fails

The checks, each with its line:
- record: the record has the brief's keys, whole numbers and texts only; every file it names is there and each
  band's digest is right; each source has its original's digest and whether it carried its C2PA record, a
  request, a truth check and an approval (or "waiting");
- stale: each band was made from the band above as it is now (its made_from is that file's digest);
- loss: the re-grid lost at most 10% of the source's colour detail (the record's regrid_loss);
- seams: band 0 tiles, its step across the wrap at most 1.2 times the median step inside (tile.py);
- light: no painted light in band 0, the plane through the lightness of a 4 x 4 grid of windows sloping at most
  0.02 of OKLab lightness across the tile;
  both are measured on band 0 alone, the tile the brief requires seamless and where a source's light would be:
  in smaller levels the measures are noise (in levels of 16 to 128 texture pixels made from a seamless band 0,
  10 to 33% of the ordinary boundaries inside the tile already step more than 1.2 times the median, and a
  window's mean moves with the material's own marks); the code reduction keeps band 0's wrap by construction
  (test_art_reduce), and a redrawn level's seams are measured when it is prepared and written in its band's way;
- repeat: no strong repeat inside band 0, its autocorrelation beyond 15 cm at most 0.2 (tile.py);
- contrast: band 0's texture pixel contrast within a quarter of its source's (art/textures/<name>/source.png,
  the source on band 0's grid); where no such source is kept, as for a tile cut from parts of a picture, it is
  reported as not measured, neither passed nor failed;
- accents: every band from 1 to 6 keeps at least 90% of band 0's accents (study 6's measure);
- drift: every band's lightness within 0.02 and hue within 5 degrees of band 0's; hue is left out where both are
  too grey for a hue to mean anything (colourfulness under 0.02).
Lightness, hue, colourfulness, texture pixel contrast and accents come from `kindling look` (look.py), the same
C++ the engine's checks use.

Implements PRE-20, PRE-22 and PRE-42, see A5.4.
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
SLOPE_MOST = 2.0  # hundredths of OKLab lightness across the tile: 0.02
REPEAT_MOST = 0.2
CONTRAST_SHARE = 0.25
ACCENT_SHARE = match.ACCENT_SHARE
DRIFT_LIGHTNESS = 2.0  # hundredths: 0.02
DRIFT_HUE = 5.0  # degrees
GREY = 2.0  # colourfulness in hundredths under which a hue means little
MEASURED = 256  # seams and painted light are measured on levels this many texture pixels across: band 0


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


def check_texture(folder, root=ROOT):
    """[(check, passed, what was found)] for one texture folder; passed is None for a check not measured."""
    out = []
    path = os.path.join(folder, "record.toml")
    if not os.path.exists(path):
        return [("record", False, "no record.toml")]
    try:
        rec = record.read(path)
    except Exception as e:  # a record that will not parse fails as a record
        return [("record", False, f"record.toml does not read: {e}")]
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
    tile_texels = rec.get("tile_texels")
    for n, t in enumerate(levels):
        if t is not None and isinstance(tile_texels, int) and t.shape[:2] != (tile_texels >> n, tile_texels >> n):
            bad.append(f"band {n} is {t.shape[1]} x {t.shape[0]}, not {tile_texels >> n} square")
    out.append(("record", not bad, "; ".join(bad) or f"{len(levels)} bands, files and digests right"))
    if any(t is None for t in levels) or not levels:
        return out
    bands = rec["band"]
    stale = [str(n) for n in range(1, len(bands)) if bands[n].get("made_from") != digests[n - 1]]
    out.append(("stale", not stale, f"bands {', '.join(stale)} made from an older level" if stale else "none"))
    m = re.match(r"^\s*([0-9.]+)\s*%", str(rec.get("regrid_loss", "")))
    if m:
        out.append(("loss", float(m.group(1)) <= LOSS_MOST, f"{m.group(1)}% (at most {LOSS_MOST:.0f}%)"))
    else:
        out.append(("loss", "not" in str(rec.get("regrid_loss", "")), f"{rec.get('regrid_loss')!r}"))
    worst = max((tile.seams(t), n) for n, t in enumerate(levels) if min(t.shape[:2]) >= MEASURED)
    out.append(("seams", worst[0] <= SEAMS_MOST, f"at most {worst[0]:.2f}, band {worst[1]} (at most {SEAMS_MOST})"))
    sl = max((slope(t), n) for n, t in enumerate(levels) if min(t.shape[:2]) >= MEASURED)
    out.append(("light", sl[0] <= SLOPE_MOST, f"slope at most {sl[0] / 100:.3f}, band {sl[1]} (at most 0.02)"))
    rp = tile.repeat(levels[0])
    out.append(("repeat", rp <= REPEAT_MOST, f"{rp:.2f} (at most {REPEAT_MOST})"))
    src = os.path.join(folder, "source.png")
    s0 = look.stats(levels[0])
    if os.path.exists(src):
        tc_src = look.stats(texels.load(src))["texel_contrast"]
        share = s0["texel_contrast"] / tc_src if tc_src > 0 else 0.0
        ok = abs(share - 1) <= CONTRAST_SHARE
        out.append(("contrast", ok, f"band 0 {s0['texel_contrast']:.2f}, source {tc_src:.2f}: {share:.0%}"))
    else:
        out.append(("contrast", None, "not measured: no source on band 0's grid kept as source.png"))
    acc0 = match.accents(levels[0])
    short = []
    for n in range(1, min(7, len(levels))):
        a = match.accents(levels[n])
        if a is None or a < ACCENT_SHARE * acc0:
            short.append(f"{n} ({0 if a is None else a / acc0:.0%})")
    what = "bands " + ", ".join(short) + " under 90%" if short else "every band 90% or more"
    out.append(("accents", not short, what))
    drift = []
    for n in range(1, len(levels)):
        s = look.stats(levels[n])
        dl = abs(s["lightness"] - s0["lightness"])
        dh = abs((s["hue"] - s0["hue"] + 180) % 360 - 180)
        coloured = s["colourfulness"] >= GREY and s0["colourfulness"] >= GREY
        if dl > DRIFT_LIGHTNESS or (coloured and dh > DRIFT_HUE):
            drift.append(f"{n} (lightness {dl / 100:+.3f}, hue {dh:.1f})")
    out.append(("drift", not drift, "bands " + ", ".join(drift) if drift else "within 0.02 and 5 degrees"))
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
        for check, ok, what in check_texture(os.path.join(base, name), args.root):
            failed += ok is False
            print(f"{name:14s} {check:9s} {'skip' if ok is None else 'pass' if ok else 'FAIL'}  {what}")
    print(f"{len(names)} textures, {failed} checks failed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
