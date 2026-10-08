#!/usr/bin/env bash
# Implements PRE-31, RES-05 (T2.7a.4): exact native 2D captures, with no image resampling.
# Usage: tools/fixture-picture.sh <prefix> portrait|landscape noon|dusk [second] [facing] [walk|work] [piece|clip]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/tools/env.sh"
if [ "$#" -lt 3 ]; then
  echo 'usage: fixture-picture.sh <prefix> portrait|landscape noon|dusk [second] [facing] [walk|work] [piece|clip]' >&2
  exit 2
fi
OUT="$1"
ORIENT="$2"
HOUR="$3"
shift 3
case "$ORIENT" in
  portrait) SIZE=1080x2400 ;;
  landscape) SIZE=2400x1080 ;;
  *) echo 'choose portrait or landscape' >&2; exit 2 ;;
esac
case "$HOUR" in noon|dusk) ;; *) echo 'choose noon or dusk' >&2; exit 2 ;; esac
mkdir -p "$(dirname "$OUT")"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
export LIBGL_ALWAYS_SOFTWARE=1
xvfb-run -a -s '-screen 0 2400x2400x24 -nolisten tcp' "$GODOT" --audio-driver Dummy --path "$ROOT/game" \
  --rendering-method gl_compatibility --rendering-driver opengl3 --resolution "$SIZE" \
  -s "$ROOT/tools/fixture-picture.gd" -- "$OUT" "$HOUR" "$@" >"$TMP/log" 2>&1 \
  || { tail -30 "$TMP/log" >&2; exit 1; }
if rg -n 'SCRIPT ERROR|^ERROR:' "$TMP/log"; then exit 1; fi
rg -q 'Compatibility.*llvmpipe' "$TMP/log" || { cat "$TMP/log" >&2; exit 1; }
echo "2D fixture capture: $OUT ($ORIENT, $HOUR)"
