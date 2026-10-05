#!/usr/bin/env bash
# Records a Godot project's main scene in the cloud, picture and sound, for the note's reel (SND-12, PRC-11): Godot's
# Movie Maker at a fixed 30 frames a second, on the Mobile renderer on the software Vulkan driver under Xvfb, the app
# quitting when its tour is done; then made small by ffmpeg (H.264 and AAC in MP4) for the note's page.
# Usage: tools/reel.sh <project> <mp4> <width>x<height> [arguments for the app, such as a screen's name]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/tools/env.sh"
PROJECT="$1"
OUT="$2"
SIZE="$3"
shift 3
TMP="$(mktemp -d)"
# Movie Maker records at the project's window size, so a passing override.cfg sets it to the reel's
trap 'rm -rf "$TMP" "$PROJECT/override.cfg"' EXIT
[ ! -e "$PROJECT/override.cfg" ] || { echo "Reel: $PROJECT/override.cfg is in the way"; exit 1; }
printf '[display]\n\nwindow/size/window_width_override=%s\nwindow/size/window_height_override=%s\n' \
  "${SIZE%x*}" "${SIZE#*x}" >"$PROJECT/override.cfg"
timeout 3600 xvfb-run -a -s "-screen 0 3000x3000x24" "$GODOT" --path "$PROJECT" --rendering-method mobile \
  --rendering-driver vulkan --write-movie "$TMP/reel.avi" --fixed-fps 30 -- "$@" \
  >"$TMP/log" 2>&1 || { grep -v ALSA "$TMP/log" | tail -20; echo "Reel: Godot failed"; exit 1; }
[ -s "$TMP/reel.avi" ] || { tail -20 "$TMP/log"; echo "Reel: Godot recorded nothing"; exit 1; }
ffmpeg -y -loglevel error -i "$TMP/reel.avi" -c:v libx264 -preset slow -crf 30 -pix_fmt yuv420p -c:a aac -b:a 96k \
  -movflags +faststart "$OUT"
echo "Reel: $OUT, $(du -h "$OUT" | cut -f1)"
