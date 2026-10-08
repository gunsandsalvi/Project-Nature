#!/usr/bin/env python3
"""Implements PRE-31 PLT-04 (T2.8a.4): inspected terrain engine regressions, not art approval."""

import importlib.util
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("fixture_goldens", ROOT / "tools/fixture-goldens.py")
fixture_goldens = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture_goldens)
VIEWS = (
    "flat-noon",
    "flat-north",
    "flat-west",
    "flat-south",
    "flat-dusk",
    "slope-noon",
    "cliff-noon",
    "cliff-dusk",
    "shelter-noon",
    "shelter-closed",
    "water-noon",
    "water-dusk",
    "cave-night",
    "flat-landscape",
    "water-landscape",
    "debug-receivers",
    "debug-normals",
    "debug-layers",
    "cliff-band",
    "flat-rain",
    "flat-winter",
    "flat-before",
    "cliff-before",
)

if __name__ == "__main__":
    sys.exit(fixture_goldens.main(sys.argv[1:], VIEWS, ROOT / "game/terrain/goldens.json"))
