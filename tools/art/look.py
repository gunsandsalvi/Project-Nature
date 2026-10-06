"""The colour measures, from `kindling look`: the same C++ the engine's checks use, so each measure is written once
(art/BRIEF.md, rule 7; CLAUDE.md, rule 4). Nothing here computes a colour measure itself.

The program is found through the KINDLING path (default build/sim/kindling). It reads a picture as raw 8-bit RGBA on
standard input, row by row from the top left:
- `kindling look stats <w> <h>` prints one line of `name value` pairs, read here by name: lightness, colourfulness,
  contrast and texel_contrast in hundredths of OKLab's scale, hue in degrees, and accents (study 6's measure), which
  is left out for pictures smaller than 29 pixels either way;
- `kindling look adjust <w> <h> <lightness> <hue> <colourfulness> <contrast>` writes the picture back with lightness
  added, hue turned in degrees, and colourfulness and contrast scaled in percent.

Implements PRE-20, see A5.4.
"""

import os
import subprocess

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


class LookError(RuntimeError):
    pass


def program():
    return os.environ.get("KINDLING", os.path.join(ROOT, "build", "sim", "kindling"))


def _run(command, numbers, t):
    """Runs `kindling look <command> <w> <h> <numbers...>` on a picture and returns what it wrote."""
    h, w = t.shape[:2]
    rgba = np.concatenate([t.astype(np.uint8), np.full((h, w, 1), 255, np.uint8)], axis=2)
    path = program()
    if not os.path.exists(path):
        raise LookError(f"no kindling at {path}: build it, or set KINDLING to its path")
    args = [path, "look", command, str(w), str(h), *numbers]
    done = subprocess.run(args, input=rgba.tobytes(), capture_output=True)
    if done.returncode != 0:
        raise LookError(f"kindling look {command} failed ({done.returncode}): {done.stderr.decode().strip()}")
    return done.stdout


def stats(t):
    """The measures of a picture of texture pixels (one picture pixel a texture pixel), by name."""
    words = _run("stats", [], t).decode().split()
    if len(words) % 2:
        raise LookError(f"kindling look stats printed an odd line: {' '.join(words)}")
    return {words[i]: float(words[i + 1]) for i in range(0, len(words), 2)}


def adjust(t, lightness=0.0, hue=0.0, colourfulness=100.0, contrast=100.0):
    """The picture with the four-number change made in OKLab by kindling look."""
    out = _run("adjust", [f"{lightness:.4f}", f"{hue:.4f}", f"{colourfulness:.4f}", f"{contrast:.4f}"], t)
    h, w = t.shape[:2]
    if len(out) != h * w * 4:
        raise LookError(f"kindling look adjust wrote {len(out)} bytes for {w} x {h}")
    return np.frombuffer(out, np.uint8).reshape(h, w, 4)[..., :3].copy()


def tiled(t, least=64):
    """A small level repeated until it is at least `least` texture pixels each way, so measures that look round a
    texture pixel see its neighbours across the wrap, as on the ground."""
    ny = -(-least // t.shape[0])
    nx = -(-least // t.shape[1])
    return np.tile(t, (ny, nx, 1))
