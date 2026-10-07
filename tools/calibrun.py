#!/usr/bin/env python3
"""The calibration scenes drawn in the cloud (A18.1, A17, α2.2a). One Godot run on the software Vulkan driver opens the
Calibrate page on a small screen and runs every variant of every scene in data/scenes/look as the phone does, a
hundred times faster (tools/godot-calibrate.gd); this then holds what each variant drew to what its scene states:
- nothing: no draw at all in the world;
- field: the ground drawn;
- rocks: exactly their triangles in the main pass, as many again in the sun's shadow pass when it has one, and none
  there when not;
- copies: exactly their draws in the main pass and in the sun's shadow pass, and in the mirror's pass when it has
  three, the mirror adding no shadow pass of its own;
- leaves: each way adding to the bare ground exactly the draws and triangles its plants hold, casting no shadow, and
  the ways that cut no leaf (close-cut cards and solid cores) drawing the picture plain cards draw;
- fires: every way drawing the same ground under its fires' things, each way that draws fire shadows darkening some
  of the picture, and the fires' maps darkening nothing the walk at every pixel leaves lit;
- figures: a draw for each figure on Godot's skeletons and one for all on palettes, in the main pass and in the sun's
  shadow pass, every figure's triangles in both, over the same ground each time;
- reads: one draw of exactly its points, and no shadow pass;
and the bone palettes bend every vertex of a figure within 1 cm of where Godot's own skeleton bends it, at two moments
of its walk; and the run's code reads back through `kindling look calibrate`. A shader or script that fails fails the
run. A picture of each scene's first variant, and of each way of the plants and the fires, is left in
build/calibrate/.

    python3 tools/calibrun.py                draw every scene and check what each drew, into build/calibrate/
"""

import json
import os
import shutil
import subprocess
import sys

import numpy as np
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "build", "calibrate")
# a fifth of the phone's screen each way: the triangles and draws are the same, the pixels fewer
SIZE = (216, 480)
# The ways of drawing plants that cut no leaf, so must draw plain cards' picture, and the share of its pixels that may
# differ from it by more than two levels of 255 in a channel: a few where rounding decides, at a card's foot on the
# ground (α2.2b: 4 of 103,680 at the cloud's size).
SAME_AS_PLAIN = ("close", "cores")
SAME_LEVELS = 2
SAME_SHARE_MOST = 0.001
# Fire shadows the cloud must see: each way that draws them darkens at least SHADOWS_LEAST of the picture to under
# SHADOWED of its brightness without them, and the maps darken at most MAP_ONLY_MOST of it that the walk at every pixel
# leaves lit (α2.2b, three fires at the cloud's size: 1.6% by the walk, 1.3% at half resolution, 0.95% from the maps,
# none from the maps alone). The fires' flicker moves their light by a tenth at most, far from these lines.
SHADOWED = 0.6
SHADOWS_LEAST = 0.003
MAP_ONLY_MOST = 0.001
# The bone palettes against Godot's skeletons (α2.2b's test): every vertex within PALETTE_MOST metres, and the figure
# walking between the two moments, some vertex moving at least MOVED_LEAST, so the check is not of a figure at rest
# (α2.2b: within 2 mm, the view keeping a place to about 4 mm; the walk moves a vertex 55 cm). A pixel no vertex was
# drawn on keeps the view's black, which reads as -1 m in each coordinate, below any vertex of a standing figure.
PALETTE_MOST = 0.01
MOVED_LEAST = 0.05
BLANK_BELOW = -0.5


class RunError(RuntimeError):
    pass


def kindling():
    return os.environ.get("KINDLING", os.path.join(ROOT, "build", "sim", "kindling"))


def draw(out=OUT):
    """Runs the Calibrate page in Godot, writing out/run.json: the scenes, what each variant drew, and the code."""
    godot = os.environ.get("GODOT")
    if not godot or not os.path.exists(godot):
        raise RunError("no Godot: run `. tools/env.sh` first (tools/setup.sh installs it)")
    if not os.path.exists(os.path.join(ROOT, "game", "bin", "libkindling.linux.x86_64.so")):
        raise RunError("no extension in game/bin: build view/ first (tools/check.sh does)")
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    w, h = SIZE
    args = [
        "xvfb-run", "-a", "-s", f"-screen 0 {max(w, 640) + 64}x{max(h, 480) + 64}x24",
        godot, "--path", os.path.join(ROOT, "game"), "--rendering-method", "mobile", "--rendering-driver", "vulkan",
        "--resolution", f"{w}x{h}", "-s", os.path.join(ROOT, "tools", "godot-calibrate.gd"), "--",
        os.path.join(out, "run.json"),
    ]  # fmt: skip
    env = dict(os.environ, LP_NUM_THREADS="1")
    done = subprocess.run(args, capture_output=True, text=True, env=env, timeout=1800)
    log = done.stdout + done.stderr
    with open(os.path.join(out, "godot.log"), "w") as f:
        f.write(log)
    broken = "SHADER ERROR" in log or "Shader compilation failed" in log or "SCRIPT ERROR" in log or errors(log)
    if done.returncode != 0 or broken or "Calibration run: done" not in log or "Forward Mobile" not in log:
        tail = "\n".join(line for line in log.splitlines() if "ALSA" not in line)[-2000:]
        raise RunError(f"the calibration run failed ({done.returncode}):\n{tail}")
    with open(os.path.join(out, "run.json")) as f:
        return json.load(f)


def errors(log):
    """Godot's errors in a run's log, but the one the cloud always gives: no sound device (its ALSA driver)."""
    lines = log.splitlines()
    return [
        line
        for i, line in enumerate(lines)
        if line.startswith("ERROR:") and not (i + 1 < len(lines) and "audio_driver_alsa" in lines[i + 1])
    ]


def expected(scene, variant):
    """What a variant must have drawn, by the counts the run writes: {count: number}."""
    draws, thousands, copies = scene["draws"], int(variant["triangles"]), int(variant["copies"])
    shadowed = bool(variant["shadows"])
    if draws == "nothing":
        return {"draws": 0, "triangles": 0, "shadow_draws": 0, "mirror_draws": 0}
    if draws == "rocks":
        rocks = thousands * 1000
        return {"triangles": rocks, "shadow_triangles": rocks if shadowed else 0, "mirror_draws": 0}
    if draws == "copies":
        mirrored = int(variant["passes"]) == 3
        return {
            "draws": copies,
            "shadow_draws": copies if shadowed else 0,
            "mirror_draws": copies if mirrored else 0,
            "mirror_shadow_draws": 0,
        }
    if draws == "reads":
        # points count one a primitive
        return {"draws": 1, "triangles": int(variant["vertices"]) * 1000, "shadow_draws": 0, "shadow_triangles": 0}
    return {}


def differing_share(one, other):
    """The share of two pictures' pixels differing by more than SAME_LEVELS in a channel; None if one is missing."""
    if not (os.path.exists(one) and os.path.exists(other)):
        return None
    a = np.asarray(Image.open(one).convert("RGB"), dtype=np.int16)
    b = np.asarray(Image.open(other).convert("RGB"), dtype=np.int16)
    if a.shape != b.shape:
        return 1.0
    return float((np.abs(a - b).max(axis=2) > SAME_LEVELS).mean())


def plants_wrong(scene, drew, out):
    """A leaves scene's ways against its bare ground and against plain cards: a list of what is wrong, in words."""
    wrong = []
    name_of = {v["way"]: v["name"] for v in scene["variants"]}
    bare = next((c for v, c in zip(scene["variants"], drew, strict=True) if v["way"] == "none"), None)
    for variant, counts in zip(scene["variants"], drew, strict=True):
        if bare is None or variant["way"] == "none" or not counts:
            continue
        for count in ("draws", "triangles"):
            added = int(counts[count]) - int(bare[count])
            if added != int(counts[f"content_{count}"]):
                wrong.append(
                    f"{scene['name']}/{variant['name']}: {added} {count} more than the bare ground, where its plants "
                    f"hold {counts[f'content_{count}']}"
                )
        if int(counts["shadow_draws"]) != 0:
            wrong.append(f"{scene['name']}/{variant['name']}: plants in the sun's shadow pass")
    for way in SAME_AS_PLAIN:
        if way not in name_of or "plain" not in name_of:
            continue
        share = differing_share(
            os.path.join(out, f"{scene['name']}-{name_of['plain']}.png"),
            os.path.join(out, f"{scene['name']}-{name_of[way]}.png"),
        )
        if share is None:
            wrong.append(f"{scene['name']}/{name_of[way]}: no picture to hold to plain cards'")
        elif share > SAME_SHARE_MOST:
            wrong.append(
                f"{scene['name']}/{name_of[way]}: {share:.2%} of its pixels differ from plain cards', where at most "
                f"{SAME_SHARE_MOST:.1%} may"
            )
    return wrong


def luma(path):
    """A picture's brightness at each pixel, from 0 to 255, or None if it is missing."""
    if not os.path.exists(path):
        return None
    return np.asarray(Image.open(path).convert("RGB"), dtype=np.float64) @ np.array([0.2126, 0.7152, 0.0722])


def fires_wrong(scene, drew, out):
    """A fires scene's ways against each other: a list of what is wrong, in words."""
    wrong = []
    # the ways differ only in how they shade and the passes they add, so with as many fires they draw alike in the
    # main pass; what lies outside the view, Godot leaves out
    draws = {}
    for variant, counts in zip(scene["variants"], drew, strict=True):
        if counts:
            draws.setdefault(int(variant["fires"]), set()).add(int(counts["draws"]))
    for fires, counted in sorted(draws.items()):
        if len(counted) > 1:
            wrong.append(f"{scene['name']}: its ways with {fires} fires draw {sorted(counted)} things, not alike")
    name_of = {(int(v["fires"]), v["way"]): v["name"] for v in scene["variants"]}
    for (fires, way), name in name_of.items():
        if way == "none" or (fires, "none") not in name_of:
            continue
        lit = luma(os.path.join(out, f"{scene['name']}-{name_of[(fires, 'none')]}.png"))
        drawn = luma(os.path.join(out, f"{scene['name']}-{name}.png"))
        if lit is None or drawn is None or lit.shape != drawn.shape:
            wrong.append(f"{scene['name']}/{name}: no picture to hold to the fires' without shadows")
            continue
        shadowed = drawn < SHADOWED * lit
        if shadowed.mean() < SHADOWS_LEAST:
            wrong.append(
                f"{scene['name']}/{name}: {shadowed.mean():.2%} of its picture in shadow, where its fires cast some"
            )
        walk = luma(os.path.join(out, f"{scene['name']}-{name_of.get((fires, 'walk'), '')}.png"))
        if way == "map" and walk is not None and walk.shape == lit.shape:
            alone = (shadowed & ~(walk < SHADOWED * lit)).mean()
            if alone > MAP_ONLY_MOST:
                wrong.append(f"{scene['name']}/{name}: {alone:.2%} of its picture shadowed where the walk's is lit")
    return wrong


def figures_wrong(scene, drew):
    """A figures scene's ways against what each must draw: a list of what is wrong, in words."""
    wrong = []
    grounds = set()
    for variant, counts in zip(scene["variants"], drew, strict=True):
        if not counts:
            continue
        name = f"{scene['name']}/{variant['name']}"
        # a figure on Godot's skeleton is an instance of its own; the palettes' are one MultiMesh
        draws = int(variant["figures"]) if variant["way"] == "godot" else 1
        if int(counts["content_draws"]) != draws:
            wrong.append(f"{name}: its figures make {counts['content_draws']} draws, where its way makes {draws}")
        shadowed = bool(variant["shadows"])
        for count, want in (
            ("shadow_draws", draws if shadowed else 0),
            ("shadow_triangles", int(counts["content_triangles"]) if shadowed else 0),
        ):
            if int(counts[count]) != want:
                wrong.append(f"{name}: {count} {counts[count]}, where its figures alone make {want}")
        # what the main pass draws besides the figures is the ground, the same in every variant
        grounds.add(
            (
                int(counts["draws"]) - int(counts["content_draws"]),
                int(counts["triangles"]) - int(counts["content_triangles"]),
            )
        )
    if len(grounds) > 1:
        wrong.append(f"{scene['name']}: the ground under the figures draws differently from variant to variant")
    return wrong


def bent_wrong(bent):
    """The bone palettes against Godot's skeletons, from the run's figure bent both ways at two moments: a list of
    what is wrong, in words, and the most a vertex lies apart, in metres."""
    if not bent or len(bent.get("moments", [])) != 2:
        return ["the bone palettes were not checked against Godot's skeletons"], None
    wrong = []
    apart = 0.0
    places = []
    for i, moment in enumerate(bent["moments"]):
        both = {}
        for way in ("godot", "palette"):
            p = np.asarray(moment[way], dtype=np.float64).reshape(-1, 3)
            blank = int((p[:, 1] < BLANK_BELOW).sum())
            if p.shape[0] != int(bent["vertices"]) or blank:
                wrong.append(f"moment {i}: {blank} of the figure's {bent['vertices']} vertices not drawn {way}'s way")
            both[way] = p
        if both["godot"].shape == both["palette"].shape:
            apart = max(apart, float(np.linalg.norm(both["godot"] - both["palette"], axis=1).max()))
        places.append(both)
    if apart > PALETTE_MOST:
        wrong.append(f"the palettes bend a vertex {apart * 100:.1f} cm from Godot's skeleton, where 1 cm at most may")
    for way in ("godot", "palette"):
        if places[0][way].shape == places[1][way].shape:
            moved = float(np.linalg.norm(places[0][way] - places[1][way], axis=1).max())
            if moved < MOVED_LEAST:
                wrong.append(f"the figure bent {way}'s way moves {moved * 100:.1f} cm in half a second of its walk")
    return wrong, apart


def check(run, out=OUT):
    """Each variant's drawing against its scene's statement: a list of what is wrong, in words."""
    wrong = []
    for scene, drew in zip(run["scenes"], run["counted"], strict=True):
        for variant, counts in zip(scene["variants"], drew, strict=True):
            name = f"{scene['name']}/{variant['name']}"
            if not counts:
                wrong.append(f"{name}: nothing was counted")
                continue
            for count, want in expected(scene, variant).items():
                if int(counts[count]) != want:
                    wrong.append(f"{name}: {count} {counts[count]}, where its scene states {want}")
            if scene["draws"] == "field" and (int(counts["draws"]) == 0 or int(counts["triangles"]) == 0):
                wrong.append(f"{name}: the ground was not drawn")
        if scene["draws"] == "leaves":
            wrong += plants_wrong(scene, drew, out)
        if scene["draws"] == "fires":
            wrong += fires_wrong(scene, drew, out)
        if scene["draws"] == "figures":
            wrong += figures_wrong(scene, drew)
    wrong += bent_wrong(run.get("figures_bent"))[0]
    read = subprocess.run([kindling(), "look", "calibrate", run["code"]], capture_output=True, text=True)
    if read.returncode != 0:
        wrong.append(f"the run's code does not read: {read.stderr.strip()}")
    return wrong


def main(argv):
    if argv:
        print(__doc__)
        return 2
    try:
        run = draw()
    except RunError as e:
        print(f"Calibration: {e}")
        return 1
    wrong = check(run)
    for line in wrong:
        print(f"Calibration: {line}")
    apart = bent_wrong(run.get("figures_bent"))[1]
    if apart is not None:
        print(f"Calibration: the bone palettes bend every vertex within {apart * 1000:.1f} mm of Godot's skeleton")
    variants = sum(len(s["variants"]) for s in run["scenes"])
    print(f"Calibration: {'FAIL' if wrong else 'OK'} ({len(run['scenes'])} scenes, {variants} variants drawn)")
    return 1 if wrong else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
