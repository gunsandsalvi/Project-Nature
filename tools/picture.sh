#!/usr/bin/env bash
# Draws a Godot project's main scene in the cloud as your phone shows it, for the note (PRC-11, A2.3, A17).
# Usage: tools/picture.sh <project> <png> portrait|landscape [arguments for the app, such as a screen's name]
# Drawn at the phone's 1344 × 2992 pixels (research 02) by the Mobile renderer on the software Vulkan driver under
# Xvfb, then halved for the note. Godot needs --rendering-method beside a rendering driver, or it falls back to
# Forward+; its Movie Maker mode records at the project's base size, hence tools/godot-picture.gd.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/tools/env.sh"
PROJECT="$1"
OUT="$2"
ORIENT="${3:-}"
case "$ORIENT" in
  portrait) SIZE=1344x2992 ;;
  landscape) SIZE=2992x1344 ;;
  *) echo "usage: tools/picture.sh <project> <png> portrait|landscape [app arguments]" >&2; exit 2 ;;
esac
shift 3
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
timeout 300 xvfb-run -a -s "-screen 0 3000x3000x24" "$GODOT" --path "$PROJECT" --rendering-method mobile \
  --rendering-driver vulkan --resolution "$SIZE" -s "$ROOT/tools/godot-picture.gd" -- "$TMP/full.png" 12 "$@" \
  >"$TMP/log" 2>&1 || { grep -v ALSA "$TMP/log" | tail -20; echo "Picture: Godot failed"; exit 1; }
grep -q 'Forward Mobile' "$TMP/log" || { head -5 "$TMP/log"; echo "Picture: not the Mobile renderer"; exit 1; }
python3 - "$TMP/full.png" "$OUT" <<'EOF'
import sys
from PIL import Image
im = Image.open(sys.argv[1]).convert("RGB")
im.resize((im.width // 2, im.height // 2), Image.LANCZOS).save(sys.argv[2], optimize=True)
EOF
echo "Picture: $OUT ($ORIENT)"
