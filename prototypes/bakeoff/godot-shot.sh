#!/usr/bin/env bash
# Draws the Godot scene on the software Vulkan driver at the phone's size and saves shots/<name>.png and -art.png.
# Usage: godot-shot.sh <name> [extra user args such as --yaw=60]
set -euo pipefail
cd "$(dirname "$0")/godot"
GODOT="${GODOT:-$HOME/.cache/bakeoff/Godot_v4.7.2-stable_linux.x86_64}"
mkdir -p ../shots
NAME="$1"; shift
timeout 300 xvfb-run -a -s "-screen 0 1400x3000x24" "$GODOT" --path . --rendering-driver vulkan --resolution 1344x2992 \
  -- --shot="$PWD/../shots/$NAME.png" --frames=40 "$@" 2>&1 | grep -E 'ERROR|SCRIPT|Parse|error' | grep -v ALSA || true
ls -la "../shots/$NAME.png"
