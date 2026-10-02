#!/usr/bin/env bash
# The checks before work joins main (PRC-10, A15.12). Usage: tools/check.sh [--deliver] | --gate <description file>
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd); cd "$ROOT"; . tools/env.sh
[ "${1:-}" = "--gate" ] && exec python3 tools/filecheck.py gate "${2:?description file}"
DELIVER=0; [ "${1:-}" = "--deliver" ] && DELIVER=1
COMMIT=$(git rev-parse --short=12 HEAD); T0=$(date +%s)
git fetch -q origin +refs/heads/main:refs/remotes/origin/main 2>/dev/null || true
BASE=$(git merge-base HEAD origin/main 2>/dev/null || git rev-list --max-parents=0 HEAD | tail -1)
changed() { [ -n "$(git diff --name-only "$BASE" -- "$@")" ]; }   # no pipe into grep -q: pipefail would make it flaky
step() { echo "== $*"; }
step "1 format";          cargo fmt --all --check
step "1 lints";           cargo clippy --workspace --all-targets --locked -- -D warnings
step "1 phone lints";     cargo clippy -p kd-android --target aarch64-linux-android --locked -- -D warnings
step "1 web lints";       cargo clippy -p kd-web --target wasm32-unknown-unknown --locked -- -D warnings
step "1 banned fixture";  tools/check-banned.sh
step "2 layers";          cargo run -q --profile fast -p kd-tools --locked -- check layers && cargo run -q --profile fast -p kd-tools --locked -- check names
step "3 tests";           cargo test --workspace --locked
step "3 arm64 tests";     cargo test -p kd-core --target aarch64-unknown-linux-gnu --locked
step "3 tool tests";      python3 -m unittest discover -s tools/tests -q && python3 tools/filecheck.py selftest && python3 tools/signing-key.py selftest
step "4 catalogue";       cargo run -q --profile fast -p kd-tools --locked -- catalog check
step "5 scenes";          echo "   from α07c"
step "6 repeat";          echo "   α00: kd-core's stored draws and maths on x86 and arm64 (step 3) and wasm (step 9); kd det from α03c"
step "7 file check";      python3 tools/filecheck.py file
step "8 coverage";        python3 tools/filecheck.py ids --merge
step "9 builds"
if [ $DELIVER = 1 ] || changed web crates; then tools/build-web.sh && node tools/screens/smoke.mjs && node tools/screens/golden.mjs; fi
if [ $DELIVER = 1 ] || changed android crates; then tools/build-apk.sh check; fi
mkdir -p results/checks
printf '{"commit":"%s","result":"PASS","minutes":%d,"deliver":%d}\n' "$COMMIT" $(( ($(date +%s) - T0) / 60 )) "$DELIVER" > "results/checks/$COMMIT.json"
echo "Checks: PASS $COMMIT"
