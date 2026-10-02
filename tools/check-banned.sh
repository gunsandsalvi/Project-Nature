#!/usr/bin/env bash
# The banned fixture (A2.3 rule 6, A3.2, A15.1): clippy must flag each entry of clippy.toml once in tests/banned/,
# so a mistyped path in clippy.toml can't silently ban nothing. Matches the wording of Rust 1.97.0's clippy.
# Usage: tools/check-banned.sh   (prints "Banned: OK (44 of 44)" or exits 1)
# checks: PRN-14
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd); cd "$ROOT"
WANT=$(python3 -c 'import tomllib; c = tomllib.load(open("clippy.toml", "rb")); print(len(c["disallowed-types"]) + len(c["disallowed-methods"]))')
# Clippy's exit status is ignored: the fixture must fail, and the count says whether it failed for every ban.
OUT=$(env -u KINDLING_SIGNING_PASSPHRASE cargo clippy -q --manifest-path tests/banned/Cargo.toml --locked -- \
  -D clippy::disallowed_methods -D clippy::disallowed_types 2>&1)
GOT=$(grep -c '^error: use of a disallowed' <<<"$OUT")
if [ "$GOT" = "$WANT" ]; then echo "Banned: OK ($GOT of $WANT)"; exit 0; fi
grep '^error' <<<"$OUT"
echo "Banned: FAIL ($GOT of $WANT flagged by clippy)"
exit 1
