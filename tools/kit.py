#!/usr/bin/env python3
"""Exports the model kit's parts for the game (A6.1): each family's Blender file into the .kdkit file the engine reads,
and the stand-in family the Blender script makes until the art lane's own parts arrive.

    python3 tools/kit.py build <out folder>

Every art/models/<family>.blend is opened in Blender, headless, and its parts, joints, texture layouts and roles
written to <out folder>/<family>.kdkit by tools/blender/export.py; the stand-in family, standin_camp, is made and
exported by tools/blender/standins.py. Files whose bytes did not change are left as they were, and any other .kdkit in
the folder is removed. Blender comes from tools/setup.sh. The kit's own checks of what this writes are kd_kit's
(view/tools/kit.cpp), which tools/gamedata.py runs. Implements PRE-46.
"""

import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART_MODELS = os.path.join(ROOT, "art", "models")
EXPORT = os.path.join(ROOT, "tools", "blender", "export.py")
STANDINS = os.path.join(ROOT, "tools", "blender", "standins.py")
STANDIN_FAMILY = "standin_camp"


def families(models=ART_MODELS):
    """The art lane's families: {name: its .blend file}, in the order of their names."""
    if not os.path.isdir(models):
        return {}
    return {
        name[: -len(".blend")]: os.path.join(models, name)
        for name in sorted(os.listdir(models))
        if name.endswith(".blend")
    }


def read(path):
    """A file's bytes, or None if there is no such file."""
    if not os.path.isfile(path):
        return None
    with open(path, "rb") as f:
        return f.read()


def blender(arguments):
    """Runs Blender headless with these arguments after its standard ones; its last line of output, or the fault."""
    command = ["blender", "--background", "--factory-startup", "--python-exit-code", "1", *arguments]
    run = subprocess.run(command, capture_output=True, text=True, cwd=ROOT)
    if run.returncode != 0:
        tail = "\n".join((run.stdout + run.stderr).strip().splitlines()[-12:])
        raise RuntimeError(f"Blender failed ({' '.join(arguments[:2])}):\n{tail}")
    said = [line for line in run.stdout.splitlines() if line.startswith("KDKIT:")]
    return said[-1] if said else ""


def build(out, models=ART_MODELS, standins=True):
    """Every family's .kdkit in a folder, each only rewritten when its bytes change, and none left that is no
    family's: {family: what Blender said}."""
    os.makedirs(out, exist_ok=True)
    said = {}
    scratch = os.path.join(out, ".making")
    os.makedirs(scratch, exist_ok=True)
    try:
        made = {}
        for name, blend in families(models).items():
            target = os.path.join(scratch, name + ".kdkit")
            said[name] = blender([blend, "--python", EXPORT, "--", target])
            made[name] = target
        if standins and os.path.isfile(STANDINS):
            target = os.path.join(scratch, STANDIN_FAMILY + ".kdkit")
            said[STANDIN_FAMILY] = blender(["--python", STANDINS, "--", target])
            made[STANDIN_FAMILY] = target
        for name, target in made.items():
            data = read(target)
            path = os.path.join(out, name + ".kdkit")
            if read(path) != data:
                with open(path, "wb") as f:
                    f.write(data)
        for name in os.listdir(out):
            if name.endswith(".kdkit") and name[: -len(".kdkit")] not in made:
                os.remove(os.path.join(out, name))
    finally:
        for name in os.listdir(scratch):
            os.remove(os.path.join(scratch, name))
        os.rmdir(scratch)
    return said


def main(argv):
    if len(argv) != 2 or argv[0] != "build":
        print(__doc__)
        return 2
    try:
        said = build(argv[1])
    except RuntimeError as e:
        print(f"Kit: {e}")
        return 1
    for name, line in said.items():
        print(f"Kit: {name}: {line.removeprefix('KDKIT: ')}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
