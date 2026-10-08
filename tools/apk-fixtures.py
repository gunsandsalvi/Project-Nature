#!/usr/bin/env python3
"""Implements PLT-06, PRC-11, PRE-31: prove fixture files reached the exported APK.

Usage: python3 tools/apk-fixtures.py <apk> <version-code>
Compare raw files with this checkout and follow the packaged texture import mappings.
Older deliveries predate fixtures; terrain material records begin at 30801.
"""

import json
import re
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def resource_path(path):
    if not isinstance(path, str) or not path or path.startswith("/") or any(c in path for c in ["..", ":", "\\"]):
        raise ValueError(f"invalid fixture resource path: {path!r}")
    return path


def remaps(data, member):
    """Read only texture paths in Godot's packaged [remap] section, including platform variants."""
    section = ""
    paths = set()
    for line in data.decode("utf-8").splitlines():
        line = line.strip()
        if line.startswith("[") and line.endswith("]"):
            section = line
        elif section == "[remap]":
            match = re.fullmatch(r'path(?:\.[^=\s]+)?\s*=\s*(".*")', line)
            if match:
                path = json.loads(match[1])
                if not path.startswith("res://"):
                    raise ValueError(f"invalid texture remap in {member}: {path}")
                paths.add("assets/" + resource_path(path.removeprefix("res://")))
    if not paths:
        raise ValueError(f"no texture remap in {member}")
    return paths


def check(apk, code, root=ROOT):
    if code < 30701:
        return None
    with zipfile.ZipFile(apk) as archive:
        names = set(archive.namelist())
        raw_files = set()

        def required(member):
            if member not in names:
                raise ValueError(f"missing {member}")
            data = archive.read(member)
            if not data:
                raise ValueError(f"empty {member}")
            return data

        def raw(path):
            member = "assets/" + path
            data = required(member)
            if data != (Path(root) / "game" / path).read_bytes():
                raise ValueError(f"{member} differs from its source")
            raw_files.add(path)
            return data

        manifest = json.loads(raw("fixtures/manifest.json"))
        if not isinstance(manifest, list) or not manifest:
            raise ValueError("fixture manifest must contain entries")
        if code >= 30801:
            json.loads(raw("terrain/materials.json"))
        textures = set()
        for entry in manifest:
            if not isinstance(entry, dict) or not isinstance(entry.get("actions"), dict) or not entry["actions"]:
                raise ValueError("fixture entry needs action atlas paths")
            if entry.get("sheet"):
                sheet = entry["sheet"]
                raw(resource_path(sheet[6:]) if sheet.startswith("res://") else "fixtures/" + resource_path(sheet))
            levels = list(entry["actions"].values())
            levels.extend(entry.get(channel, {}) for channel in ["normal_levels", "material_levels"])
            for level in levels:
                if not isinstance(level, dict):
                    raise ValueError("fixture atlas levels must be path records")
                textures.update(resource_path(path) for path in level.values())
        if not textures:
            raise ValueError("fixture manifest has no atlas paths")
        for texture in sorted(textures):
            member = "assets/fixtures/" + texture + ".import"
            for target in sorted(remaps(required(member), member)):
                required(target)
    return (len(raw_files), len(textures))


def main(argv):
    if len(argv) != 3:
        sys.exit(__doc__)
    try:
        result = check(argv[1], int(argv[2]))
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        sys.exit(f"APK fixtures: FAILED: {error}")
    if result is None:
        print("   ok  fixture files not required before 30701")
    else:
        print(f"   ok  fixture files ({result[0]} raw files, {result[1]} imported atlases)")


if __name__ == "__main__":
    main(sys.argv)
