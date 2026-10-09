#!/usr/bin/env python3
"""Fills game/data/ for each build: the catalogue's sources and build.toml, what the phone's self-check compares
itself with (A2.3, A3.6), and the scenes' last reports with a world each (A17).

    python3 tools/gamedata.py <kindling tool>

It checks the catalogue with the cloud's own build of the simulation (the kindling tool) and refuses to go on if it
finds a problem; copies every .toml file under data/ but its scenes into game/data/, in the same folders, and the art
lane's source beside data/ (art/source.toml, each texture's record under art/textures/ and each model's recipe under
art/models/, A5.4, A6.1) into game/data/art/, and removes any it no longer holds; runs every proof suite on one
thread and on four, refusing to go on if they differ; and writes build.toml:
- [proof]: each suite's digest;
- [catalogue]: the world-making version, each file the phone reads with its SHA-256, and each source's version and
  rules, world and look digests, as the simulation fingerprints them;
- [bench]: each benchmark scenario's digest at its mark, its world run headless here, which the phone's must match
  (A18.1, RES-05);
- [textures]: each texture file the phone reads with its SHA-256 (A5.4), written into game/data/textures/: the
  stand-ins tools/standins.py makes (T2.1a.3), and each of the art lane's textures, its record's levels packed
  largest first with lossless PNG encoding into textures/art/<entry>.kdtex;
  art:meadow/middle becomes textures/art/meadow/middle.kdtex;
- [sheets]: signed-off reference sheets retained for 2D fixture inspection (T2.3a.5), each with its SHA-256,
  written into game/data/sheets/ as <piece>.kdsheet (the WebP under a name Godot's import leaves alone),
  by the art lane's catalogue (art/catalogue/*.toml names each piece's sheet);
- [build]: the app's version code, from the export preset, which the benchmark's code carries.
Obsolete 3D model and calibration exports are excluded and stale files removed. Historical catalogue records
remain available for fingerprinting and original art provenance; no Blender exporter runs during app preparation.
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
import tempfile
import tomllib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import standins  # noqa: E402
import pngpack  # noqa: E402
import sprite_families  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA = os.path.join(ROOT, "data")
ART = os.path.join(ROOT, "art")
OUT = os.path.join(ROOT, "game", "data")
BUILD = os.path.join(OUT, "build.toml")
TEXTURES = os.path.join(OUT, "textures")
SHEETS = os.path.join(OUT, "sheets")
# the pilot's pieces (IMPLEMENTATION.md, α2.3a), by their ids in the art lane's catalogue, each shown beside its sheet
PILOT_PIECES = ("meadow", "river", "river_bed", "club", "hide_tent_cone")
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


def art_textures(records):
    """Each texture record's levels packed into one .kdtex, largest first: {its path in game/data/textures/: its
    bytes}, art:meadow/middle as art/meadow/middle.kdtex."""
    out = {}
    for rel in records:
        if not rel.startswith("art/textures/") or not rel.endswith("/record.toml"):
            continue
        with open(os.path.join(ROOT, rel), "rb") as f:
            levels = sorted(tomllib.load(f)["band"], key=lambda band: band["level"])
        pictures = []
        for band in levels:
            with open(os.path.join(ROOT, band["file"]), "rb") as f:
                pictures.append(pngpack.lossless(f.read()))
        entry = rel[len("art/textures/") : -len("/record.toml")]
        out[f"art/{entry}.kdtex"] = standins.kdtex(pictures)
    return out


def copy_sources(files):
    """game/data/ holds exactly data/'s files and the art lane's source, build.toml, the reports, the textures and
    reference sheets. Obsolete model/calibration exports are removed."""
    for dirpath, _, names in os.walk(OUT, topdown=False):
        for name in names:
            rel = os.path.relpath(os.path.join(dirpath, name), OUT).replace(os.sep, "/")
            if rel != "build.toml" and not rel.startswith(("reports/", "textures/", "sheets/")) and rel not in files:
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


def textures(records=()):
    """The texture files in game/data/textures/, the stand-ins and the art lane's, each written only when its bytes
    change, and any other removed: their paths there."""
    os.makedirs(TEXTURES, exist_ok=True)
    made = standins.files() | art_textures(records)
    for name, data in made.items():
        path = os.path.join(TEXTURES, name)
        if not os.path.isfile(path) or open(path, "rb").read() != data:
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "wb") as f:
                f.write(data)
    for dirpath, _, names in os.walk(TEXTURES, topdown=False):
        for name in names:
            if os.path.relpath(os.path.join(dirpath, name), TEXTURES).replace(os.sep, "/") not in made:
                os.remove(os.path.join(dirpath, name))
        if dirpath != TEXTURES and not os.listdir(dirpath):
            os.rmdir(dirpath)
    return sorted(made)


def pilot_sheets():
    """The picture of each pilot piece's sheet, from the art lane's catalogue, whose [[piece]] tables name it:
    {the piece's id: its path from the repository's top}; a RuntimeError names any piece the catalogue lacks or whose
    sheet is missing."""
    sheets = {}
    folder = os.path.join(ART, "catalogue")
    for name in sorted(os.listdir(folder)) if os.path.isdir(folder) else []:
        if name.endswith(".toml"):
            with open(os.path.join(folder, name), "rb") as f:
                for piece in tomllib.load(f).get("piece", []):
                    if piece.get("id") in PILOT_PIECES and "sheet" in piece:
                        sheets[piece["id"]] = os.path.join(folder, piece["sheet"])
    for piece in PILOT_PIECES:
        if piece not in sheets:
            raise RuntimeError(f"the art lane's catalogue has no sheet for the pilot's piece {piece}")
        if not os.path.isfile(sheets[piece]):
            raise RuntimeError(
                f"the sheet {os.path.relpath(sheets[piece], ROOT)} of the pilot's piece {piece} is missing"
            )
    return {piece: os.path.relpath(sheets[piece], ROOT) for piece in PILOT_PIECES}


def sheets():
    """The pilot's sheets in game/data/sheets/, <piece>.kdsheet, each written only when its bytes change, and any other
    removed: their names there, in order."""
    os.makedirs(SHEETS, exist_ok=True)
    made = {}
    for piece, rel in pilot_sheets().items():
        with open(os.path.join(ROOT, rel), "rb") as f:
            made[f"{piece}.kdsheet"] = f.read()
    for name, data in made.items():
        path = os.path.join(SHEETS, name)
        if not os.path.isfile(path) or open(path, "rb").read() != data:
            with open(path, "wb") as f:
                f.write(data)
    for name in os.listdir(SHEETS):
        if name not in made:
            os.remove(os.path.join(SHEETS, name))
    return sorted(made)


def build_toml(proof, version, files, sources, bench, code, texture_files=(), sheet_files=()):
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
        "[bench]",
    ]
    lines += [f'{name} = "{digest}"' for name, digest in bench.items()]
    lines += [
        "",
        "[textures]",
        "files = " + toml_list(f"{name} {sha256(os.path.join(TEXTURES, name))}" for name in texture_files),
        "sizes = " + toml_list(f"{name} {os.path.getsize(os.path.join(TEXTURES, name))}" for name in texture_files),
        "",
        "[sheets]",
        "files = " + toml_list(f"{name} {sha256(os.path.join(SHEETS, name))}" for name in sheet_files),
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
    made = textures(art_files())
    try:
        shown_sheets = sheets()
    except RuntimeError as e:
        print(f"Game data: {e}")
        return 1
    with open(BUILD, "w") as f:
        f.write(build_toml(one, version, files, sources, bench_digests(tool), version_code(), made, shown_sheets))
    try:
        shown = reports(tool)
        discovery_example(tool)
    except RuntimeError as e:
        print(f"Game data: {e}")
        return 1
    print(
        f"Game data: {os.path.relpath(OUT, ROOT)}/ with {len(files)} catalogue files, {len(sources)} sources, "
        f"{len(one)} proof suites, {len(shown)} scene reports, {len(made)} textures, {len(shown_sheets)} sheets"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
