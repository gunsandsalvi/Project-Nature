#!/usr/bin/env python3
"""Writes game/data/build.toml, what the phone's self-check compares itself with (A2.3, A3.6).

    python3 tools/gamedata.py <kindling tool>

It runs the cloud's own build of the simulation (the kindling tool) for every proof suite on one thread and on
four, refuses to go on if they differ, and writes their digests. game/data/ is made by the build and never committed
(A2.1); later steps copy data/'s sources there too, with their digests.
"""

import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "game", "data", "build.toml")


def digests(tool, threads):
    run = subprocess.run([tool, "proof", "--threads", str(threads)], capture_output=True, text=True, check=True)
    out = {}
    for line in run.stdout.splitlines():
        suite, digest = line.split()[:2]
        out[suite] = digest
    return out


def build_toml(proof):
    # No comments: Godot's ConfigFile, which reads this on the phone until the simulation's own loader does
    # (A3.6), stops at a TOML comment.
    lines = [
        "[build]",
        'about = "Made by tools/gamedata.py for each build: what the phone\'s self-check compares with (A2.3)."',
        "",
        "[proof]",
    ]
    lines += [f'{suite} = "{digest}"' for suite, digest in proof.items()]
    return "\n".join(lines) + "\n"


def main(argv):
    if len(argv) != 1:
        print(__doc__)
        return 2
    one, four = digests(argv[0], 1), digests(argv[0], 4)
    if one != four:
        print(f"Game data: the proof suites differ between one thread and four: {one} against {four}")
        return 1
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w") as f:
        f.write(build_toml(one))
    print(f"Game data: {os.path.relpath(OUT, ROOT)} with {len(one)} proof suites")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
