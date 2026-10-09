#!/usr/bin/env python3
"""Check current camp resources and excluded payload in the APK (PLT-06 PRC-11 MAT-13).

Usage: python3 tools/apk-camp.py <apk> <version-code>
"""

import hashlib
import json
import re
import struct
import sys
import tomllib
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OBSOLETE_PAGES = ("bench", "catalogues", "crowd", "examples", "fixtures", "terrain", "time")


def resource_path(path):
    if not isinstance(path, str) or not path or path.startswith("/") or any(c in path for c in ("..", ":", "\\")):
        raise ValueError(f"invalid resource path: {path!r}")
    return path


def remaps(data, member):
    section, paths = "", set()
    for line in data.decode().splitlines():
        line = line.strip()
        if line.startswith("[") and line.endswith("]"):
            section = line
        elif section == "[remap]":
            match = re.fullmatch(r'path(?:\.[^=\s]+)?\s*=\s*(".*")', line)
            if match:
                path = json.loads(match[1])
                if not path.startswith("res://"):
                    raise ValueError(f"invalid remap in {member}: {path}")
                paths.add("assets/" + resource_path(path.removeprefix("res://")))
    if not paths:
        raise ValueError(f"no resource remap in {member}")
    return paths


def check(apk, code, root=ROOT):
    root = Path(root)
    with zipfile.ZipFile(apk) as archive:
        names = set(archive.namelist())

        def required(member):
            if member not in names:
                raise ValueError(f"missing {member}")
            data = archive.read(member)
            if not data:
                raise ValueError(f"empty {member}")
            return data

        def raw(path):
            data = required("assets/" + resource_path(path))
            if data != (root / "game" / path).read_bytes():
                raise ValueError(f"assets/{path} differs from its source")
            return data

        for name in names:
            if name.startswith(
                (
                    "assets/test/",
                    "assets/addons/",
                    "assets/fixtures/",
                    "assets/terrain/",
                    "assets/data/textures/",
                    "assets/data/sheets/",
                )
            ):
                raise ValueError(f"obsolete payload: {name}")
            if any(name.startswith(f"assets/pages/{page}.gd") for page in OBSOLETE_PAGES):
                raise ValueError(f"obsolete page: {name}")
        build = tomllib.loads(raw("data/build.toml").decode())
        if build["build"]["code"] != code:
            raise ValueError("catalogue manifest names another build")
        if "textures" in build or "sheets" in build:
            raise ValueError("unused art manifest remains")
        files = build["catalogue"]["files"]
        if not files or not build.get("proof"):
            raise ValueError("catalogue files and proof digests are required")
        for entry in files:
            path, digest = entry.split()
            if hashlib.sha256(raw("data/" + resource_path(path))).hexdigest() != digest:
                raise ValueError(f"catalogue checksum differs: {path}")
        captured = json.loads(raw("data/examples/first-flake.json"))
        if captured.get("captured") is not True or captured.get("switches") != []:
            raise ValueError("First flake must be a captured ordinary run with no switches")
        if any(not isinstance(captured.get(key), str) or not captured[key].isdigit() for key in ("actor", "result")):
            raise ValueError("First flake identities must be exact integer strings")
        source = (root / "sim/src/kd/save/snapshot.hpp").read_text()
        version = int(re.search(r"kSnapshotVersion\s*=\s*(\d+)", source)[1])
        example = raw("data/examples/first-flake.kindling")
        if len(example) < 12 or example[:8] != b"KINDLWLD" or struct.unpack_from("<I", example, 8)[0] != version:
            raise ValueError("First flake archive is not this build's format")
        member = "assets/ui/fonts/kindling-ui-16.fnt.import"
        for target in remaps(required(member), member):
            required(target)
    return len(files)


def main(argv):
    if len(argv) != 3:
        sys.exit(__doc__)
    try:
        count = check(argv[1], int(argv[2]))
    except (OSError, ValueError, KeyError, TypeError, zipfile.BadZipFile) as error:
        sys.exit(f"APK camp resources: FAILED: {error}")
    print(f"   ok  camp resources ({count} catalogue files, current example, font; obsolete payload absent)")


if __name__ == "__main__":
    main(sys.argv)
