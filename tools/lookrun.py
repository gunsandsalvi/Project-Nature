#!/usr/bin/env python3
"""The look loop's drawing run and checks (PRE-31, PRE-22, A4.8, A5.5). One Godot run on the software Vulkan driver,
with one thread and time standing still, draws the fixed views and the scripted paths (tools/godot-look.gd); this
then reads what it drew:
- each view's target card against its moment, its material picture's numbers, and its golden picture: exact for a
  change of code alone, or else FLIP's verdict, with the two side by side on a lettered grid, so a fault is "C7";
- each path's shimmer: each frame's error against its many-sample picture, the last frame's followed through the
  camera's motion, as the share of pixels whose error changed by more than 0.03; at most 2 in 100.

    python3 tools/lookrun.py                 draw and check every view and path, into build/look/
    python3 tools/lookrun.py --shimmer       only the shimmer check: pans over the meadow and the test board, each
                                             read smooth-pixel and nearest-pixel, on a small screen; the board read
                                             nearest-pixel must be flagged and the meadow read smooth-pixel pass
    python3 tools/lookrun.py --approve       make the views just drawn the golden ones: only with your OK

The colour measures and FLIP are the C++ ones, through `kindling look` (tools/art/look.py); nothing here measures a
picture itself.
"""

import json
import os
import shutil
import subprocess
import sys

import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "art"))
import look  # noqa: E402

ROOT = look.ROOT
OUT = os.path.join(ROOT, "build", "look")
GOLDENS = os.path.join(ROOT, "art", "goldens")
SIZE = (1080, 2404)
MANY = 8  # a many-sample picture's samples each way
PATCH = 128  # a patch's side, in screen pixels
SHIMMER_LINE = 3.0  # an error that changes by more than this, in hundredths of lightness, flickers
SHIMMER_MOST = 2.0  # at most this share of pixels, in percent, may flicker
PIXELS_A_DEGREE = 80.0  # your phone held at 30 cm
CLOSEST = 1.0 / 128.0  # metres a screen pixel at the closest zoom
MEADOW = {"pattern": False}
MIDDLE = [SIZE[0] // 2 - PATCH // 2, SIZE[1] // 2 - PATCH // 2, PATCH]

# The fixed views, each with its moment on the card, and the scripted paths.
VIEWS = [
    {"name": "meadow", "moment": "late_afternoon", "parts": MEADOW, "patches": [MIDDLE]},
    {"name": "meadow-turned", "moment": "late_afternoon", "parts": MEADOW, "heading": 30.0, "patches": [MIDDLE]},
    {"name": "meadow-far", "moment": "late_afternoon", "parts": MEADOW, "metres_per_pixel": CLOSEST * 4.0},
    {"name": "board", "moment": "late_afternoon", "patches": [MIDDLE]},
]
PATHS = [
    {"name": "pan", "path": "pan", "frames": 30, "parts": MEADOW, "patch": MIDDLE},
    {"name": "turn", "path": "turn", "frames": 30, "parts": MEADOW, "patch": MIDDLE},
    {"name": "pinch", "path": "pinch", "frames": 30, "parts": MEADOW, "patch": MIDDLE},
]
# The shimmer check's pans, on a small screen so it runs in under a minute: the meadow and the test board, each read
# smooth-pixel, as the game does, and nearest-pixel. The board, crisp black lines on a checker of single texture
# pixels, is the worst case: read nearest-pixel it must be flagged, and the meadow read smooth-pixel must pass.
SHIMMER_SIZE = (256, 256)
SHIMMER_PATHS = [
    {"name": f"{ground}-{read}", "path": "pan", "frames": 30, "parts": parts, "patch": [64, 64, PATCH],
     "nearest": read == "nearest"}
    for ground, parts in (("meadow", MEADOW), ("board", {}))
    for read in ("smooth", "nearest")
]  # fmt: skip


class RunError(RuntimeError):
    pass


def draw(plan, out):
    """Runs Godot on the plan, drawing into the folder out."""
    godot = os.environ.get("GODOT")
    if not godot or not os.path.exists(godot):
        raise RunError("no Godot: run `. tools/env.sh` first (tools/setup.sh installs it)")
    if not os.path.exists(os.path.join(ROOT, "game", "bin", "libkindling.linux.x86_64.so")):
        raise RunError("no extension in game/bin: build view/ first (tools/check.sh does)")
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    with open(os.path.join(out, "plan.json"), "w") as f:
        json.dump(plan, f)
    w, h = plan["size"]
    args = [
        "xvfb-run", "-a", "-s", f"-screen 0 {max(w, 640) + 64}x{max(h, 480) + 64}x24",
        godot, "--path", os.path.join(ROOT, "game"), "--rendering-method", "mobile", "--rendering-driver", "vulkan",
        "--resolution", f"{w}x{h}", "-s", os.path.join(ROOT, "tools", "godot-look.gd"), "--",
        os.path.join(out, "plan.json"), out,
    ]  # fmt: skip
    env = dict(os.environ, LP_NUM_THREADS="1")
    done = subprocess.run(args, capture_output=True, text=True, env=env, timeout=1800)
    log = done.stdout + done.stderr
    with open(os.path.join(out, "godot.log"), "w") as f:
        f.write(log)
    if done.returncode != 0 or "Look run: done" not in log or "Forward Mobile" not in log:
        tail = "\n".join(line for line in log.splitlines() if "ALSA" not in line)[-2000:]
        raise RunError(f"the drawing run failed ({done.returncode}):\n{tail}")


def rgb(path):
    return np.asarray(Image.open(path).convert("RGB"))


def linear(c):
    c = c / 255.0
    return np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)


def srgb(x):
    c = np.where(x <= 0.0031308, x * 12.92, 1.055 * np.power(np.clip(x, 0.0, None), 1 / 2.4) - 0.055)
    return np.clip(np.round(c * 255.0), 0, 255).astype(np.uint8)


def shrink(t, k):
    """A many-sample picture brought to its frame's size: each k by k block's mean, in linear light."""
    h, w = t.shape[0] // k, t.shape[1] // k
    return srgb(linear(t[: h * k, : w * k].astype(np.float64)).reshape(h, k, w, k, 3).mean(axis=(1, 3)))


def homography(src, dst):
    """The projective map taking the four points src to dst, as nine numbers, the last 1."""
    rows, rhs = [], []
    for (x, y), (u, v) in zip(src, dst, strict=True):
        rows.append([x, y, 1, 0, 0, 0, -u * x, -u * y])
        rows.append([0, 0, 0, x, y, 1, -v * x, -v * y])
        rhs += [u, v]
    return [*np.linalg.solve(np.array(rows, float), np.array(rhs, float)), 1.0]


def rgba(t):
    return np.concatenate([t, np.full(t.shape[:2] + (1,), 255, np.uint8)], axis=2).tobytes()


def kindling(args, data):
    path = look.program()
    done = subprocess.run([path, "look", *args], input=data, capture_output=True)
    if done.returncode != 0:
        raise RunError(f"kindling look {args[0]} failed: {done.stderr.decode().strip()}")
    return done.stdout.decode().split()


def shimmer(folder, frames):
    """Each frame pair's share of flickering pixels along a path, in percent."""
    with open(os.path.join(folder, "motion.jsonl")) as f:
        motions = {m["frame"]: m for m in map(json.loads, f)}
    shares = []
    for i in range(1, frames):
        pictures = []
        for n in (i - 1, i):
            frame = rgb(os.path.join(folder, f"{n:03d}.png"))
            many = shrink(rgb(os.path.join(folder, f"{n:03d}-many.png")), MANY)
            pictures += [frame, many]
        h, w = pictures[0].shape[:2]
        m = homography(motions[i]["from"], motions[i]["to"])
        words = kindling(
            ["flicker", str(w), str(h), str(SHIMMER_LINE), *(f"{v:.9g}" for v in m)], b"".join(map(rgba, pictures))
        )
        shares.append(float(words[1]))
    return shares


def letters(n):
    """The grid's column names: A to Z, then AA."""
    out = ""
    n += 1
    while n:
        n, r = divmod(n - 1, 26)
        out = chr(65 + r) + out
    return out


def grid(before, after, path, cell=120):
    """The golden and the new picture side by side at half size, on a lettered grid of cell pixels."""
    h, w = after.shape[:2]
    half = (w // 2, h // 2)
    sheet = Image.new("RGB", (half[0] * 2 + 48, half[1] + 24), (24, 22, 20))
    draw = ImageDraw.Draw(sheet)
    for i, t in enumerate((before, after)):
        x0 = 24 + i * (half[0] + 24) if i else 24
        sheet.paste(Image.fromarray(t).resize(half, Image.BOX), (x0, 24))
        for c in range(0, half[0], cell // 2):
            draw.line([(x0 + c, 24), (x0 + c, 24 + half[1])], fill=(255, 255, 255), width=1)
            draw.text((x0 + c + 3, 6), letters(c // (cell // 2)), fill=(255, 255, 255))
        for r in range(0, half[1], cell // 2):
            draw.line([(x0, 24 + r), (x0 + half[0], 24 + r)], fill=(255, 255, 255), width=1)
            draw.text((x0 - 20 if i == 0 else x0 - 22, 24 + r + 3), str(r // (cell // 2) + 1), fill=(255, 255, 255))
    sheet.save(path)


def check_views(out):
    lines, report = [], {}
    for view in VIEWS:
        name = view["name"]
        t = rgb(os.path.join(out, "views", f"{name}.png"))
        values, alarms = look.card(t, view["moment"])
        off = [f"{k} {a}" for k, a in alarms.items() if a != "green"]
        material = rgb(os.path.join(out, "views", f"{name}-material.png"))
        words = kindling(["numbers", str(material.shape[1]), str(material.shape[0])], rgba(material))
        found = [int(n) for n in words[0::2]]
        entry = {"card": values, "alarms": alarms, "materials": found}
        golden = os.path.join(GOLDENS, f"{name}.webp")
        if not os.path.exists(golden):
            verdict = "no golden picture yet"
        else:
            before = rgb(golden)
            if before.shape == t.shape and np.array_equal(before, t):
                verdict = "the same as its golden picture"
            else:
                if before.shape != t.shape:
                    verdict = f"a different size from its golden picture ({before.shape[1]} x {before.shape[0]})"
                else:
                    words = kindling(["flip", str(t.shape[1]), str(t.shape[0]), str(PIXELS_A_DEGREE)],
                                     rgba(before) + rgba(t))  # fmt: skip
                    entry["flip"] = {"mean": float(words[1]), "above": float(words[3])}
                    verdict = f"changed: FLIP mean {words[1]}, {float(words[3]):.2f}% of pixels above 0.2"
                    grid(before, t, os.path.join(out, f"grid-{name}.png"))
        entry["golden"] = verdict
        report[name] = entry
        alarm_text = ", ".join(off) if off else "all green"
        lines.append(f"{name}: card {alarm_text} at {view['moment']}; materials {found}; {verdict}")
    return report, lines


def check_paths(out, paths):
    lines, report = [], {}
    for path in paths:
        shares = shimmer(os.path.join(out, "paths", path["name"]), path["frames"])
        mean, most = float(np.mean(shares)), float(np.max(shares))
        report[path["name"]] = {"shimmer": shares, "mean": mean, "most": most, "passes": mean <= SHIMMER_MOST}
        verdict = "passes" if mean <= SHIMMER_MOST else "flickers"
        lines.append(f"{path['name']}: shimmer {mean:.2f}% of pixels a frame, at most {most:.2f}%: {verdict}")
    return report, lines


def run_shimmer(paths=SHIMMER_PATHS):
    out = os.path.join(OUT, "shimmer")
    draw({"size": list(SHIMMER_SIZE), "many": MANY, "paths": paths}, out)
    return check_paths(out, paths)


def main(argv):
    if argv == ["--approve"]:
        os.makedirs(GOLDENS, exist_ok=True)
        for view in VIEWS:
            picture = Image.open(os.path.join(OUT, "all", "views", f"{view['name']}.png")).convert("RGB")
            picture.save(os.path.join(GOLDENS, f"{view['name']}.webp"), lossless=True, method=6)
        print(f"Look run: {len(VIEWS)} golden pictures made from the last run")
        return 0
    if argv not in ([], ["--shimmer"]):
        print(__doc__)
        return 2
    try:
        if argv == ["--shimmer"]:
            report, lines = run_shimmer()
            failed = not report["meadow-smooth"]["passes"] or report["board-nearest"]["passes"]
        else:
            out = os.path.join(OUT, "all")
            draw({"size": list(SIZE), "many": MANY, "views": VIEWS, "paths": PATHS}, out)
            views, lines = check_views(out)
            paths, more = check_paths(out, PATHS)
            lines += more
            report = {"views": views, "paths": paths}
            failed = any(not p["passes"] for p in paths.values())
    except (RunError, look.LookError) as e:
        print(f"Look run: {e}")
        return 1
    with open(os.path.join(OUT, "report.json" if argv == [] else "shimmer.json"), "w") as f:
        json.dump(report, f, indent=1)
    print("\n".join(f"Look run: {line}" for line in lines))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
