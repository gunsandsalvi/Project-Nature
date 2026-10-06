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
and the run's code reads back through `kindling look calibrate`. A shader or script that fails fails the run. A
picture of each scene's first variant is left in build/calibrate/.

    python3 tools/calibrun.py                draw every scene and check what each drew, into build/calibrate/
"""

import json
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "build", "calibrate")
# a fifth of the phone's screen each way: the triangles and draws are the same, the pixels fewer
SIZE = (216, 480)


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
    broken = "SHADER ERROR" in log or "Shader compilation failed" in log or "SCRIPT ERROR" in log
    if done.returncode != 0 or broken or "Calibration run: done" not in log or "Forward Mobile" not in log:
        tail = "\n".join(line for line in log.splitlines() if "ALSA" not in line)[-2000:]
        raise RunError(f"the calibration run failed ({done.returncode}):\n{tail}")
    with open(os.path.join(out, "run.json")) as f:
        return json.load(f)


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
    return {}


def check(run):
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
    variants = sum(len(s["variants"]) for s in run["scenes"])
    print(f"Calibration: {'FAIL' if wrong else 'OK'} ({len(run['scenes'])} scenes, {variants} variants drawn)")
    return 1 if wrong else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
