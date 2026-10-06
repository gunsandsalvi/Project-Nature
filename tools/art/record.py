"""A texture's record (IMPLEMENTATION.md, the art lane): record.toml in each tile's folder (art/textures/<name>/, and
for a big surface its middle/ and far/ tiles and its versions v2/ to v4/), written and read.

Integers and strings only, never a float, since the catalogue's loader refuses floats (A3.6), and only the keys the
plan lists, in its order. first_band is the band the tile's first level is drawn for: 0 for a near tile, 2 for a
middle one and 4 for a far one (A5.3). Each [[band]] names its level, file and digest; every band but the first also
names the digest of the level it was made from, so a level whose source changed is stale; a redrawn band carries its
own re-grid loss, and designed bands their four calibration numbers, as words.

Implements PRE-20 and PRE-42, see A5.3 and A5.4.
"""

import tomllib

TOP = [
    "about",
    "route",
    "tile_texels",
    "texels_a_metre",
    "first_band",
    "sources",
    "original_sha256",
    "c2pa",
    "requests",
    "made",
    "regrid_loss",
    "truth",
    "approved",
]
BAND = ["level", "file", "sha256", "made_from", "way", "regrid_loss", "calibration"]
INTEGERS = {"tile_texels", "texels_a_metre", "first_band", "level"}
LISTS = {"sources", "original_sha256", "c2pa", "requests"}
ROUTES = ("picture", "code", "world")
FIRST_BANDS = (0, 2, 4)  # near, middle and far tiles (A5.3)


def problems(rec):
    """What is wrong with a record's shape: unknown or missing keys, wrong types, lists out of step."""
    out = []
    for k, v in rec.items():
        if k == "band":
            continue
        if k not in TOP:
            out.append(f"unknown key {k}")
        elif isinstance(v, float) or isinstance(v, list) and any(isinstance(x, float) for x in v):
            out.append(f"{k} holds a float")
        elif k in INTEGERS and (not isinstance(v, int) or isinstance(v, bool)):
            out.append(f"{k} is not a whole number")
        elif k in LISTS and not (isinstance(v, list) and all(isinstance(x, str) for x in v)):
            out.append(f"{k} is not a list of texts")
        elif k not in INTEGERS and k not in LISTS and not isinstance(v, str):
            out.append(f"{k} is not a text")
    for k in TOP:
        if k not in rec:
            out.append(f"no {k}")
    lengths = {k: len(rec[k]) for k in LISTS if isinstance(rec.get(k), list)}
    if len(set(lengths.values())) > 1:
        out.append("sources, original_sha256, c2pa and requests are not one entry each: " + str(lengths))
    if rec.get("route") not in ROUTES:
        out.append(f"route is {rec.get('route')!r}, not one of {', '.join(ROUTES)}")
    if "first_band" in rec and rec["first_band"] not in FIRST_BANDS:
        out.append(f"first_band is {rec['first_band']!r}, not one of {', '.join(map(str, FIRST_BANDS))}")
    bands = rec.get("band", [])
    if not isinstance(bands, list) or not bands:
        out.append("no [[band]]")
        return out
    for i, b in enumerate(bands):
        for k, v in b.items():
            if k not in BAND:
                out.append(f"band {i}: unknown key {k}")
            elif isinstance(v, float):
                out.append(f"band {i}: {k} holds a float")
            elif k in INTEGERS and (not isinstance(v, int) or isinstance(v, bool)):
                out.append(f"band {i}: {k} is not a whole number")
            elif k not in INTEGERS and not isinstance(v, str):
                out.append(f"band {i}: {k} is not a text")
        for k in ("level", "file", "sha256", "way"):
            if k not in b:
                out.append(f"band {i}: no {k}")
        if b.get("level") != i:
            out.append(f"band {i}: level is {b.get('level')}, not {i}")
        if i > 0 and "made_from" not in b:
            out.append(f"band {i}: no made_from")
    return out


def _text(v):
    return '"' + str(v).replace("\\", "\\\\").replace('"', '\\"') + '"'


def _value(k, v):
    if k in INTEGERS:
        if not isinstance(v, int) or isinstance(v, bool):
            raise ValueError(f"{k} must be a whole number, not {v!r}")
        return str(v)
    if isinstance(v, list):
        return "[" + ", ".join(_text(x) for x in v) + "]"
    if isinstance(v, float):
        raise ValueError(f"{k} is a float; the loader refuses floats (A3.6)")
    return _text(v)


def dumps(rec):
    """A record as TOML text, its keys in the brief's order; refuses floats and unknown keys."""
    bad = [p for p in problems(rec) if "unknown" in p or "float" in p or "whole number" in p]
    if bad:
        raise ValueError("; ".join(bad))
    lines = [f"{k} = {_value(k, rec[k])}" for k in TOP if k in rec]
    for b in rec.get("band", []):
        lines += ["", "[[band]]"] + [f"{k} = {_value(k, b[k])}" for k in BAND if k in b]
    return "\n".join(lines) + "\n"


def write(path, rec):
    with open(path, "w") as f:
        f.write(dumps(rec))


def read(path):
    with open(path, "rb") as f:
        return tomllib.load(f)
