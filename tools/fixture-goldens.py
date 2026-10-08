#!/usr/bin/env python3
"""Implements PRE-31, RES-05 (T2.7a.4): exact frozen pixel checks for the separate 2D developer fixtures.

Usage: fixture-goldens.py check <captures> [goldens.json]
       fixture-goldens.py record <captures> <goldens.json>
Record only after inspecting new pictures. These are engine regressions, not runtime art approvals.
"""

import hashlib
import json
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
GOLDENS = ROOT / "game/fixtures/golden-2d.json"
VIEWS = ("portrait-noon", "portrait-dusk", "landscape-noon", "landscape-dusk")
PASSES = ("page", "colour", "object", "material")


def read(captures):
    result = {"status": "Developer fixture regressions; no art approval", "views": {}}
    for view in VIEWS:
        metadata = json.loads((captures / f"{view}-capture.json").read_text())
        images = {}
        for kind in PASSES:
            with Image.open(captures / f"{view}-{kind}.png") as image:
                rgba = image.convert("RGBA")
                images[kind] = {"size": list(image.size), "sha256": hashlib.sha256(rgba.tobytes()).hexdigest()}
                if kind in ("object", "material"):
                    table = metadata[f"{kind}_codes"]
                    codes = {
                        r + (g << 8) + (b << 16)
                        for _, (r, g, b, _) in rgba.getcolors(maxcolors=rgba.width * rgba.height)
                    }
                    missing = codes - {0} - {int(code) for code in table}
                    if missing:
                        raise ValueError(f"{view}: {kind} codes missing from CPU table: {sorted(missing)}")
        result["views"][view] = {"engine": metadata["engine"], "renderer": metadata["renderer"], "images": images}
    return result


def check(captures, golden):
    actual = read(captures)
    if actual != golden:
        raise ValueError("Frozen 2D fixture pixels or renderer changed. Inspect before recording new goldens.")
    return actual


def main(args):
    if len(args) not in (2, 3) or args[0] not in ("check", "record"):
        print(__doc__.strip(), file=sys.stderr)
        return 2
    captures = Path(args[1])
    golden = Path(args[2]) if len(args) == 3 else GOLDENS
    try:
        if args[0] == "record":
            if len(args) != 3:
                raise ValueError("record requires an explicit golden file")
            golden.write_text(json.dumps(read(captures), indent=2) + "\n")
        else:
            check(captures, json.loads(golden.read_text()))
    except (OSError, ValueError, KeyError) as error:
        print(str(error), file=sys.stderr)
        return 1
    print("2D fixtures: four views and sixteen passes, exact pixels and mapped semantic codes")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
