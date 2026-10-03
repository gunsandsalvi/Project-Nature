#!/usr/bin/env bash
# Every ban in the root clippy.toml trips on tests/banned/ (A2.3 rule 6, A15.1): clippy runs on the fixture, which
# uses each banned type and method once, and this fails unless every path clippy.toml names is flagged, so a
# mistyped path can't silently ban nothing. Usage: tools/check-banned.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="$(mktemp)"
trap 'rm -f "$OUT"' EXIT
cargo clippy --manifest-path tests/banned/Cargo.toml --locked --quiet --message-format=short >"$OUT" 2>&1 || true
python3 - "$OUT" <<'EOF'
import re
import sys
import tomllib

out = open(sys.argv[1], encoding="utf-8").read()
conf = tomllib.load(open("clippy.toml", "rb"))
banned = [e["path"] for key in ("disallowed-types", "disallowed-methods") for e in conf.get(key, [])]
flagged = set(re.findall(r"use of a disallowed (?:type|method) `([^`]+)`", out))
missing = [p for p in banned if p not in flagged]
if not banned or missing:
    print(out)
    sys.exit("Banned items: FAIL: not flagged on tests/banned/: " + (", ".join(missing) or "no bans read"))
print(f"Banned items: OK ({len(banned)} bans, each flagged on tests/banned/)")
EOF
