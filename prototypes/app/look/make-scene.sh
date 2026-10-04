#!/usr/bin/env bash
# Makes P1's close camp, look/close_camp.scn, from the art book's painter: the painter's export of its close camp
# (built 50 m wider, so the camera can turn and pan), packed by make_scene.py and built into Godot's own format by
# build_scene.gd. The result is committed, so builds need neither Node nor the painter; run this again only when
# the painter's close camp changes. Needs Node and Playwright's Chromium, which the session image has.
# Pre-production code (research 00): thrown away with the prototypes.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
. "$ROOT/tools/env.sh"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
PAINT="$ROOT/art/book/paint"
[ -d "$PAINT/node_modules/three" ] || (cd "$PAINT" && npm install --silent)
(cd "$PAINT" && NODE_PATH="$(npm root -g)" node paint.js "$TMP/export" zoom:noon:closecamp:export) >"$TMP/log" 2>&1 \
  || { cat "$TMP/log"; exit 1; }
python3 "$HERE/make_scene.py" "$TMP/export" "$TMP/pack"
"$GODOT" --headless --path "$HERE/.." --import >/dev/null 2>&1
"$GODOT" --headless --path "$HERE/.." -s res://look/build_scene.gd -- "$TMP/pack" "$HERE/close_camp.scn" 2>&1 | grep '^Scene'
