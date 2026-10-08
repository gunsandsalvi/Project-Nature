#!/usr/bin/env bash
# Implements PRE-31 PLT-04 (T2.8a.4): frozen Compatibility terrain, including all active targets.
# Usage: tools/terrain-picture.sh prefix portrait|landscape noon|dusk flat|slope|cliff|shelter|water|cave [direction] [debug] [clip]
set -euo pipefail
TASK_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
export KD_CAPTURE_SCRIPT="$TASK_ROOT/tools/terrain-picture.gd"
exec "$TASK_ROOT/tools/fixture-picture.sh" "$@"
