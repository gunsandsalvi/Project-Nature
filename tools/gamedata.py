#!/usr/bin/env python3
"""Prepare the camp catalogue, deterministic proof records, required cloud reports and current First flake.

Usage: python3 tools/gamedata.py <kindling tool>
Approved source art stays in the repository. Unused textures, sheets and obsolete exports leave game/data/.
The manifest fingerprints the exact packed catalogue; examples are captured again with this build's format.
"""

import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sprite_families  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA = os.path.join(ROOT, "data")
ART = os.path.join(ROOT, "art")
OUT = os.path.join(ROOT, "game", "data")
BUILD = os.path.join(OUT, "build.toml")
SCENES = os.path.join(DATA, "scenes")
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


def sources_of(tool, folder=None):
    """Each source's line, "<id> <version> <rules> <world> <look>", and the world-making version."""
    run = subprocess.run([tool, "catalogue", "fingerprint", folder or DATA], capture_output=True, text=True, check=True)
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


def art_files():
    """The art lane's source as the catalogue reads it beside data/ (kd::data::read_art): art/source.toml, each
    texture's record under art/textures/ and each model's recipe under art/models/, by their paths from the
    repository's top, in order; none without art/source.toml."""
    if not os.path.isfile(os.path.join(ART, "source.toml")):
        return []
    out = ["art/source.toml"]
    for kind in ("textures", "models"):
        for dirpath, _, names in os.walk(os.path.join(ART, kind)):
            if "record.toml" in names:
                out.append(os.path.relpath(os.path.join(dirpath, "record.toml"), ROOT).replace(os.sep, "/"))
    return sorted(out)


def origin(rel):
    """Where a file game/data/ holds comes from: the art lane's under the repository's top, every other under
    data/."""
    return os.path.join(ROOT if rel.startswith("art/") else DATA, rel)


def copy_sources(files):
    """Keep the current catalogue, build manifest, reports and captured example; remove unused payload."""
    for dirpath, _, names in os.walk(OUT, topdown=False):
        for name in names:
            rel = os.path.relpath(os.path.join(dirpath, name), OUT).replace(os.sep, "/")
            if rel != "build.toml" and not rel.startswith(("reports/", "examples/")) and rel not in files:
                os.remove(os.path.join(dirpath, name))
        if dirpath != OUT and not os.listdir(dirpath):
            os.rmdir(dirpath)
    for rel in files:
        target = os.path.join(OUT, rel)
        os.makedirs(os.path.dirname(target), exist_ok=True)
        shutil.copyfile(origin(rel), target)


def sha256(path):
    with open(path, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


def toml_list(items):
    return "[" + ", ".join(f'"{i}"' for i in items) + "]"


def version_code():
    """The app's version code, as tools/build.sh writes it into the export preset."""
    with open(os.path.join(ROOT, "game", "export_presets.cfg")) as f:
        for line in f:
            if line.startswith("version/code="):
                return int(line.split("=", 1)[1])
    return 0


def build_toml(proof, version, files, sources, code):
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
        "files = " + toml_list(f"{rel} {sha256(os.path.join(OUT, rel))}" for rel in files),
        "sources = " + toml_list(sources),
        "",
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


def contents(path):
    with open(path, "rb") as file:
        return file.read()


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
        # A stable report can accompany an obsolete archive format. Export the fresh run
        # every time, keeping the packaged bytes only when this build writes the same ones.
        folder = os.path.join(out, f"run-{shown:03d}")
        target_world = os.path.join(REPORTS, world)
        with tempfile.TemporaryDirectory(prefix="kindling-report-export-") as export_folder:
            candidate = os.path.join(export_folder, world)
            subprocess.run([tool, "export", folder, candidate], check=True)
            if not os.path.isfile(target_world) or contents(candidate) != contents(target_world):
                shutil.copyfile(candidate, target_world)
        kept |= {f"{name}.json", world}
    for gone in set(os.listdir(REPORTS)) - kept:
        os.remove(os.path.join(REPORTS, gone))
    return sorted(k for k in kept if k.endswith(".json"))


def discovery_example(tool):
    """Capture the declared ordinary seed again with this build's exact format/rules."""
    with open(os.path.join(ROOT, "tools", "examples", "first-flake.json")) as file:
        declared = json.load(file)
    destination = os.path.join(OUT, "examples")
    os.makedirs(destination, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="kindling-first-flake-") as temporary:
        world = os.path.join(temporary, "world")
        captured = subprocess.run(
            [tool, "discovery", str(declared["seed"]), world, DATA, app_version()], capture_output=True, text=True
        )
        if captured.returncode:
            raise RuntimeError(
                "the ordinary First flake example did not reproduce\n" + captured.stdout + captured.stderr
            )
        with open(os.path.join(world, "capture.json")) as file:
            metadata = json.load(file)
        if metadata["switches"] or not metadata["captured"]:
            raise RuntimeError("First flake must be captured without test switches")
        archive = os.path.join(temporary, "first-flake.kindling")
        subprocess.run([tool, "export", world, archive], check=True)
        for name, source in (
            ("first-flake.kindling", archive),
            ("first-flake.json", os.path.join(world, "capture.json")),
        ):
            target = os.path.join(destination, name)
            if not os.path.isfile(target) or contents(source) != contents(target):
                shutil.copyfile(source, target)


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
    files = data_files() + art_files()
    os.makedirs(OUT, exist_ok=True)
    copy_sources(files)
    try:
        derived = sprite_families.derive(ROOT)
        for rel, text in derived.items():
            target = os.path.join(OUT, rel)
            os.makedirs(os.path.dirname(target), exist_ok=True)
            with open(target, "w") as output:
                output.write(text)
        files += sorted(derived)
        # Validate and fingerprint the exact packed catalogue, including generated sprite records.
        # build.toml, reports and packed texture bytes are not catalogue source files.
        with tempfile.TemporaryDirectory(prefix="kindling-catalogue-") as staging:
            for rel in files:
                target = os.path.join(staging, rel)
                os.makedirs(os.path.dirname(target), exist_ok=True)
                shutil.copyfile(os.path.join(OUT, rel), target)
            checked = subprocess.run([tool, "catalogue", "check", staging], capture_output=True, text=True)
            if checked.returncode:
                raise RuntimeError(checked.stdout + checked.stderr)
            version, sources = sources_of(tool, staging)
    except (RuntimeError, ValueError, OSError, KeyError) as error:
        print(f"Game data: sprite catalogue: {error}")
        return 1
    with open(BUILD, "w") as f:
        f.write(build_toml(one, version, files, sources, version_code()))
    try:
        shown = reports(tool)
        discovery_example(tool)
    except RuntimeError as e:
        print(f"Game data: {e}")
        return 1
    print(
        f"Game data: {os.path.relpath(OUT, ROOT)}/ with {len(files)} catalogue files, {len(sources)} sources, "
        f"{len(one)} proof suites, {len(shown)} scene reports, current First flake; no unused art payload"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
