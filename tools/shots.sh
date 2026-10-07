#!/usr/bin/env bash
# Draws pictures of one of the app's pages in the cloud as the steps of a plan ask (PRC-11, A6.5, A17), for the model
# sheet and the notes. Usage: tools/shots.sh <page> <plan.json> <out folder> [<width>x<height>]
# The page is res://pages/<page>.gd, such as kit, and the plan the list of steps tools/godot-shots.gd reads. Drawn by the
# Mobile renderer on the software Vulkan driver under Xvfb, at 1080 × 1500 pixels unless another size is given.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/tools/env.sh"
[ $# -ge 3 ] || { echo "usage: tools/shots.sh <page> <plan.json> <out folder> [<width>x<height>]" >&2; exit 2; }
PAGE="$1"
PLAN="$(cd "$(dirname "$2")" && pwd)/$(basename "$2")"
mkdir -p "$3"
OUT="$(cd "$3" && pwd)"
SIZE="${4:-1080x1500}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
timeout 600 xvfb-run -a -s "-screen 0 3000x3000x24" "$GODOT" --path "$ROOT/game" --rendering-method mobile \
  --rendering-driver vulkan --resolution "$SIZE" -s "$ROOT/tools/godot-shots.gd" -- "$PAGE" "$PLAN" "$OUT" \
  >"$TMP/log" 2>&1 || { grep -v ALSA "$TMP/log" | tail -20; echo "Shots: Godot failed"; exit 1; }
grep -q 'Forward Mobile' "$TMP/log" || { head -5 "$TMP/log"; echo "Shots: not the Mobile renderer"; exit 1; }
if grep -E 'SCRIPT ERROR|^ERROR' "$TMP/log" | grep -v -E 'ALSA|audio' | head -5 | grep -q .; then
  grep -E 'SCRIPT ERROR|^ERROR' "$TMP/log" | grep -v -E 'ALSA|audio' | head -10
fi
grep -E 'Shots:|the call' "$TMP/log" || true
