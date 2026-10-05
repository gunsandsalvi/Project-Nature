#!/usr/bin/env python3
"""Fills game/data/ for each build: the catalogue's sources and build.toml, what the phone's self-check compares
itself with (A2.3, A3.6).

    python3 tools/gamedata.py <kindling tool>

It checks the catalogue with the cloud's own build of the simulation (the kindling tool) and refuses to go on if it
finds a problem; copies every .toml file under data/ into game/data/, in the same folders, and removes any it no longer
holds; runs every proof suite on one thread and on four, refusing to go on if they differ; and writes build.toml:
- [proof]: each suite's digest;
- [catalogue]: the world-making version, each file the phone reads with its SHA-256, and each source's version and
  rules, world and look digests, as the simulation fingerprints them.
game/data/ is made by the build and never committed (A2.1).
"""

import hashlib
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA = os.path.join(ROOT, "data")
OUT = os.path.join(ROOT, "game", "data")
BUILD = os.path.join(OUT, "build.toml")


def digests(tool, threads):
    run = subprocess.run([tool, "proof", "--threads", str(threads)], capture_output=True, text=True, check=True)
    out = {}
    for line in run.stdout.splitlines():
        suite, digest = line.split()[:2]
        out[suite] = digest
    return out


def sources_of(tool):
    """Each source's line, "<id> <version> <rules> <world> <look>", and the world-making version."""
    run = subprocess.run([tool, "catalogue", "fingerprint", DATA], capture_output=True, text=True, check=True)
    version, sources = None, []
    for line in run.stdout.splitlines():
        words = line.split()
        if words[:2] == ["world-making", "version"]:
            version = int(words[2])
        elif words[:1] == ["source"]:
            # source <id> version <n> rules <hex> world <hex> look <hex>
            sources.append(" ".join([words[1], words[3], words[5], words[7], words[9]]))
    return version, sources


def data_files():
    """Every .toml file under data/, by its path from there, in order."""
    out = []
    for dirpath, _, names in os.walk(DATA):
        for name in names:
            if name.endswith(".toml"):
                out.append(os.path.relpath(os.path.join(dirpath, name), DATA).replace(os.sep, "/"))
    return sorted(out)


def copy_sources(files):
    """game/data/ holds exactly data/'s files, and build.toml."""
    for dirpath, _, names in os.walk(OUT, topdown=False):
        for name in names:
            rel = os.path.relpath(os.path.join(dirpath, name), OUT).replace(os.sep, "/")
            if rel != "build.toml" and rel not in files:
                os.remove(os.path.join(dirpath, name))
        if dirpath != OUT and not os.listdir(dirpath):
            os.rmdir(dirpath)
    for rel in files:
        target = os.path.join(OUT, rel)
        os.makedirs(os.path.dirname(target), exist_ok=True)
        shutil.copyfile(os.path.join(DATA, rel), target)


def sha256(path):
    with open(path, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


def toml_list(items):
    return "[" + ", ".join(f'"{i}"' for i in items) + "]"


def build_toml(proof, version, files, sources):
    # No comments: Godot's ConfigFile, which reads this on the phone, stops at a TOML comment. Lists hold "a b" texts
    # rather than tables, which ConfigFile also reads.
    lines = [
        "[build]",
        'about = "Made by tools/gamedata.py for each build: what the phone\'s self-check compares with (A2.3)."',
        "",
        "[proof]",
    ]
    lines += [f'{suite} = "{digest}"' for suite, digest in proof.items()]
    lines += [
        "",
        "[catalogue]",
        f"world_making_version = {version}",
        "files = " + toml_list(f"{rel} {sha256(os.path.join(DATA, rel))}" for rel in files),
        "sources = " + toml_list(sources),
    ]
    return "\n".join(lines) + "\n"


def main(argv):
    if len(argv) != 1:
        print(__doc__)
        return 2
    tool = argv[0]
    check = subprocess.run([tool, "catalogue", "check", DATA], capture_output=True, text=True)
    if check.returncode != 0:
        print(check.stdout + check.stderr + "Game data: the catalogue has problems")
        return 1
    one, four = digests(tool, 1), digests(tool, 4)
    if one != four:
        print(f"Game data: the proof suites differ between one thread and four: {one} against {four}")
        return 1
    version, sources = sources_of(tool)
    files = data_files()
    os.makedirs(OUT, exist_ok=True)
    copy_sources(files)
    with open(BUILD, "w") as f:
        f.write(build_toml(one, version, files, sources))
    print(
        f"Game data: {os.path.relpath(OUT, ROOT)}/ with {len(files)} catalogue files, {len(sources)} sources and "
        f"{len(one)} proof suites"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
