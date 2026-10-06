#!/usr/bin/env python3
"""Fills game/data/ for each build: the catalogue's sources and build.toml, what the phone's self-check compares
itself with (A2.3, A3.6), and the scenes' last reports with a world each (A17).

    python3 tools/gamedata.py <kindling tool>

It checks the catalogue with the cloud's own build of the simulation (the kindling tool) and refuses to go on if it
finds a problem; copies every .toml file under data/ but its scenes into game/data/, in the same folders, and
removes any it no longer holds; runs every proof suite on one thread and on four, refusing to go on if they differ;
and writes build.toml:
- [proof]: each suite's digest;
- [catalogue]: the world-making version, each file the phone reads with its SHA-256, and each source's version and
  rules, world and look digests, as the simulation fingerprints them;
- [bench]: each benchmark scenario's digest at its mark, its world run headless here, which the phone's must match
  (A18.1, RES-05);
- [textures]: each texture file the phone reads with its SHA-256 (A5.4): for now the stand-ins tools/standins.py
  makes (T2.1a.3), written into game/data/textures/;
- [build]: the app's version code, from the export preset, which the benchmark's code carries;
- [calibration]: each calibration scene the Calibrate page runs (A18.1, α2.2a), with its SHA-256: the files of
  data/scenes/look, checked by the kindling tool and copied into game/data/scenes/look/.
Then it runs every scene in data/scenes, saved under the app's version, and puts its report in game/data/reports/,
with the world of its first odd run, or else its first, as a .kindling file the Reports page opens (RES-06, PLT-05):
<scene>.json and <scene>-<run>.kindling. A report whose runs ended as before is left as it was, with its world, so
the Godot step sees nothing changed.
game/data/ is made by the build and never committed (A2.1).
"""

import hashlib
import json
import os
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import standins  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA = os.path.join(ROOT, "data")
OUT = os.path.join(ROOT, "game", "data")
BUILD = os.path.join(OUT, "build.toml")
TEXTURES = os.path.join(OUT, "textures")
SCENES = os.path.join(DATA, "scenes")
# the calibration scenes, which the phone runs rather than the cloud
CALIBRATION = os.path.join(SCENES, "look")
REPORTS = os.path.join(OUT, "reports")
# where the scenes' worlds are kept as they run
RUNS = os.path.join(ROOT, "build", "scenes")


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
    """Every .toml file under data/ but its scenes, which are the cloud's tests (A17), by its path from there, in
    order."""
    out = []
    for dirpath, dirs, names in os.walk(DATA):
        if dirpath == DATA and "scenes" in dirs:
            dirs.remove("scenes")
        for name in names:
            if name.endswith(".toml"):
                out.append(os.path.relpath(os.path.join(dirpath, name), DATA).replace(os.sep, "/"))
    return sorted(out)


def calibration_files():
    """The calibration scenes, by their paths from data/, in order."""
    if not os.path.isdir(CALIBRATION):
        return []
    rel = os.path.relpath(CALIBRATION, DATA).replace(os.sep, "/")
    return sorted(f"{rel}/{name}" for name in os.listdir(CALIBRATION) if name.endswith(".toml"))


def copy_sources(files):
    """game/data/ holds exactly data/'s files, build.toml and the reports."""
    for dirpath, _, names in os.walk(OUT, topdown=False):
        for name in names:
            rel = os.path.relpath(os.path.join(dirpath, name), OUT).replace(os.sep, "/")
            if rel != "build.toml" and not rel.startswith(("reports/", "textures/")) and rel not in files:
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


def bench_digests(tool):
    """Each benchmark scenario's digest at its mark, run headless by the tool: {scenario: digest}."""
    run = subprocess.run([tool, "bench", DATA], capture_output=True, text=True, check=True)
    return {name: digest for name, _, digest in (line.split() for line in run.stdout.splitlines())}


def version_code():
    """The app's version code, as tools/build.sh writes it into the export preset."""
    with open(os.path.join(ROOT, "game", "export_presets.cfg")) as f:
        for line in f:
            if line.startswith("version/code="):
                return int(line.split("=", 1)[1])
    return 0


def textures():
    """The texture files in game/data/textures/, each written only when its bytes change, and any other removed:
    their names."""
    os.makedirs(TEXTURES, exist_ok=True)
    made = standins.files()
    for name, data in made.items():
        path = os.path.join(TEXTURES, name)
        if not os.path.isfile(path) or open(path, "rb").read() != data:
            with open(path, "wb") as f:
                f.write(data)
    for gone in set(os.listdir(TEXTURES)) - set(made):
        os.remove(os.path.join(TEXTURES, gone))
    return sorted(made)


def build_toml(proof, version, files, sources, bench, code, texture_files=(), calibration=()):
    # No comments: Godot's ConfigFile, which reads this on the phone, stops at a TOML comment. Lists hold "a b" texts
    # rather than tables, which ConfigFile also reads.
    lines = [
        "[build]",
        'about = "Made by tools/gamedata.py for each build: what the phone\'s self-check compares with (A2.3)."',
        f"code = {code}",
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
        "",
        "[bench]",
    ]
    lines += [f'{name} = "{digest}"' for name, digest in bench.items()]
    lines += [
        "",
        "[textures]",
        "files = " + toml_list(f"{name} {sha256(os.path.join(TEXTURES, name))}" for name in texture_files),
        "",
        "[calibration]",
        "files = " + toml_list(f"{rel} {sha256(os.path.join(DATA, rel))}" for rel in calibration),
    ]
    return "\n".join(lines) + "\n"


def app_version():
    """The app's version, as tools/build.sh writes it into the project: the scenes' worlds are saved under it, so the
    phone opens them as its own."""
    with open(os.path.join(ROOT, "game", "project.godot")) as f:
        for line in f:
            if line.startswith("config/version="):
                return line.split("=", 1)[1].strip().strip('"')
    return "kindling"


def steady(report):
    """A report but for the real seconds it took, which differ from one run to the next."""
    return {k: v for k, v in report.items() if k != "seconds"}


def reports(tool):
    """Each scene run, its report and one of its worlds put in game/data/reports/: their names."""
    os.makedirs(REPORTS, exist_ok=True)
    version = app_version()
    kept = set()
    for scene in sorted(f for f in os.listdir(SCENES) if f.endswith(".toml")):
        name = scene[:-5]
        out = os.path.join(RUNS, name)
        command = [tool, "scene", os.path.join(SCENES, scene), "--out", out, "--data", DATA, "--build", version]
        run = subprocess.run([*command, "--fresh"], capture_output=True, text=True)
        made = os.path.join(out, "report.json")
        if not os.path.isfile(made):
            raise RuntimeError(f"the scene {name} left no report\n{run.stdout}{run.stderr}")
        with open(made) as f:
            report = json.load(f)
        odd = [r for r in report["each"] if r["oddities"]]
        shown = (odd or report["each"])[0]["index"]
        world = f"{name}-{shown + 1}.kindling"
        target = os.path.join(REPORTS, f"{name}.json")
        before = None
        if os.path.isfile(target):
            with open(target) as f:
                before = json.load(f)
        if before is None or steady(before) != steady(report) or not os.path.isfile(os.path.join(REPORTS, world)):
            shutil.copyfile(made, target)
            folder = os.path.join(out, f"run-{shown:03d}")
            subprocess.run([tool, "export", folder, os.path.join(REPORTS, world)], check=True)
        kept |= {f"{name}.json", world}
    for gone in set(os.listdir(REPORTS)) - kept:
        os.remove(os.path.join(REPORTS, gone))
    return sorted(k for k in kept if k.endswith(".json"))


def main(argv):
    if len(argv) != 1:
        print(__doc__)
        return 2
    tool = argv[0]
    check = subprocess.run([tool, "catalogue", "check", DATA], capture_output=True, text=True)
    if check.returncode != 0:
        print(check.stdout + check.stderr + "Game data: the catalogue has problems")
        return 1
    check = subprocess.run([tool, "look", "calibrate", "check", DATA], capture_output=True, text=True)
    if check.returncode != 0:
        print(check.stdout + check.stderr + "Game data: the calibration scenes have problems")
        return 1
    one, four = digests(tool, 1), digests(tool, 4)
    if one != four:
        print(f"Game data: the proof suites differ between one thread and four: {one} against {four}")
        return 1
    version, sources = sources_of(tool)
    files = data_files()
    calibration = calibration_files()
    os.makedirs(OUT, exist_ok=True)
    copy_sources(files + calibration)
    made = textures()
    with open(BUILD, "w") as f:
        f.write(build_toml(one, version, files, sources, bench_digests(tool), version_code(), made, calibration))
    try:
        shown = reports(tool)
    except RuntimeError as e:
        print(f"Game data: {e}")
        return 1
    print(
        f"Game data: {os.path.relpath(OUT, ROOT)}/ with {len(files)} catalogue files, {len(sources)} sources, "
        f"{len(one)} proof suites, {len(shown)} scene reports, {len(made)} textures and {len(calibration)} "
        "calibration scenes"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
