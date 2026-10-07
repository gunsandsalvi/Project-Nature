"""Keeps a picture as a source (A5.4): the picture, as lossless WebP in art/sources/, with the digest of its original
file and whether it carried its C2PA record, which are what a texture's record names.

    python3 tools/art/ingest.py <picture.png> <art/sources/<name>/<file>.webp> "<what it is>"

Prints the destination, the original's SHA-256 and "present" or "absent" for its C2PA record, and writes them with the
note into art/sources/<name>/sources.toml, one table for each file, which make_tiles.py reads to fill a record. A PNG
carries C2PA in its caBX chunk. The WebP is checked to hold the same pixels.
Implements PRE-20 and PRE-42, see A5.4.
"""

import os
import struct
import sys
import tomllib

import numpy as np
from PIL import Image

import textures
import tiles

MANIFEST = "sources.toml"


def has_c2pa(path):
    """Whether a PNG file carries a caBX chunk, where C2PA keeps its record."""
    with open(path, "rb") as f:
        data = f.read()
    pos = 8
    while pos + 8 <= len(data):
        (length,) = struct.unpack(">I", data[pos : pos + 4])
        if data[pos + 4 : pos + 8] == b"caBX":
            return True
        pos += 12 + length
    return False


def ingest(source, destination):
    """Writes the lossless WebP of a picture; returns the original's digest and its C2PA word."""
    with Image.open(source) as im:
        rgb = im.convert("RGB")
        os.makedirs(os.path.dirname(destination), exist_ok=True)
        rgb.save(destination, format="WEBP", lossless=True, quality=100, method=6)
    with Image.open(destination) as back:
        if not (np.asarray(back.convert("RGB")) == np.asarray(rgb)).all():
            raise ValueError(f"{destination} does not hold the same pixels as {source}")
    return tiles.sha256(source), "present" if has_c2pa(source) else "absent"


def keep(destination, digest, c2pa, note):
    """Writes a source's entry into the manifest beside it, keeping the others."""
    path = os.path.join(os.path.dirname(destination), MANIFEST)
    entries = {}
    if os.path.exists(path):
        with open(path, "rb") as f:
            entries = tomllib.load(f)
    entries[os.path.basename(destination)] = {"sha256": digest, "c2pa": c2pa, "note": note}
    with open(path, "w") as f:
        f.write(
            "# The sources beside this file (A5.4): each picture's original's SHA-256, whether it carried its C2PA\n"
        )
        f.write("# record, and what it is. Written by tools/art/ingest.py.\n")
        for name in sorted(entries):
            e = entries[name]
            f.write(f"\n[{textures.quote(name)}]\n")
            for key in ("sha256", "c2pa", "note"):
                f.write(f"{key} = {textures.quote(e[key])}\n")


def entry(path):
    """A kept source's manifest entry, by its path from the repository's top."""
    full = os.path.join(textures.ROOT, path)
    with open(os.path.join(os.path.dirname(full), MANIFEST), "rb") as f:
        return tomllib.load(f)[os.path.basename(path)]


def main(argv):
    if len(argv) != 4:
        print(__doc__)
        return 2
    digest, c2pa = ingest(argv[1], argv[2])
    keep(argv[2], digest, c2pa, argv[3])
    print(argv[2], digest, c2pa)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
