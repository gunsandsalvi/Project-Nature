#!/usr/bin/env bash
# A step's APK version (A2.3): prints its code and its name. Usage: tools/version.sh <step>, such as 0.1a or α0.1a.
# The code is (milestone + 1) × 10000 + alpha × 100 + step (a = 1), so α0.1a is 10101; the name is the step.
set -euo pipefail
S="${1#α}"
[[ "$S" =~ ^([0-9]{1,2})\.([0-9]{1,2})([a-e])$ ]] || { echo "version: '$1' is not a step such as 0.1a" >&2; exit 2; }
LETTER=$(($(printf '%d' "'${BASH_REMATCH[3]}") - 96))
echo "$(((10#${BASH_REMATCH[1]} + 1) * 10000 + 10#${BASH_REMATCH[2]} * 100 + LETTER)) α$S"
