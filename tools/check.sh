#!/usr/bin/env bash
# The checks before work joins main (PRC-10, A15.12), in A15.12's nine numbered steps; a step whose tool has not
# been built yet says which alpha brings it. Stops at the first failure.
# Usage: tools/check.sh [--deliver | --gate <description>]
#   --deliver             also builds the web page and an APK, and checks the note and the committed release APK
#                         (step 9)
#   --gate <description>  only the merge gate (A15.13 step 6) on a saved pull request description, or the pull
#                         request as the API gives it: the approved head, its checks, and the right reviewer
# It writes results/checks/<commit>.json and ends with "Checks: PASS <commit>".
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
. tools/env.sh
DELIVER=0
case "${1:-}" in
  "") ;;
  --deliver) DELIVER=1 ;;
  --gate)
    [ -f "${2:-}" ] || { echo "usage: tools/check.sh --gate <description file>" >&2; exit 2; }
    git fetch -q origin +refs/heads/main:refs/remotes/origin/main 2>/dev/null || true
    exec python3 tools/filecheck.py gate "$2" ;;
  *) echo "usage: tools/check.sh [--deliver | --gate <description>]" >&2; exit 2 ;;
esac
COMMIT="$(git rev-parse --short=12 HEAD)"
T0=$(date +%s)
git fetch -q origin +refs/heads/main:refs/remotes/origin/main 2>/dev/null || true
BASE="$(git merge-base HEAD origin/main 2>/dev/null || git rev-list --max-parents=0 HEAD | tail -1)"
# Whether the branch changed any of these paths since it left main.
changed() { [ -n "$(git diff --name-only "$BASE" -- "$@")" ]; }
step() { echo "== $*"; }
later() { echo "   from $1"; }

step "1 format";          cargo fmt --all --check
step "1 lints";           cargo clippy --workspace --all-targets --locked -q -- -D warnings
step "1 phone lints";     cargo clippy -p kd-android --target aarch64-linux-android --locked -q -- -D warnings
step "1 web lints";       cargo clippy -p kd-web --target wasm32-unknown-unknown --locked -q -- -D warnings
step "1 banned items";    tools/check-banned.sh
step "2 layers, names"
cargo build --profile fast -p kd-tools --locked -q
target/fast/kd check layers
target/fast/kd check names
step "3 tests"
LOG="$(mktemp)"
if ! cargo test --workspace --locked >"$LOG" 2>&1; then cat "$LOG"; rm -f "$LOG"; exit 1; fi
echo "   $(grep -c '^test .* ok$' "$LOG") Rust tests passed"
rm -f "$LOG"
step "3 tool tests";      python3 -m unittest discover -s tools/tests -q && python3 tools/signing-key.py selftest
SELF="$(python3 tools/filecheck.py selftest)" || { echo "$SELF"; exit 1; }
echo "   ${SELF##*$'\n'}"
step "4 catalogue";       target/fast/kd catalog check
step "5 scenes";          later α07c
step "6 cross-target"
LOG="$(mktemp)"
# One test thread: qemu 8.2 crashes now and then running several (3 runs in 20, measured at α00b).
if ! cargo test -p kd-core --locked --target aarch64-unknown-linux-gnu -- --test-threads=1 >"$LOG" 2>&1; then cat "$LOG"; rm -f "$LOG"; exit 1; fi
rm -f "$LOG"
echo "   the core's stored bits equal on arm64 under qemu; in the browser with the smoke test (step 9)"
step "6 repeat";          later α07c
step "7 file check";      python3 tools/filecheck.py file
step "8 coverage";        python3 tools/filecheck.py ids --merge
step "9 builds"
if [ "$DELIVER" = 1 ] || changed web crates/kd-web crates/kd-app crates/kd-render crates/kd-core; then
  tools/build-web.sh
  node tools/screens/smoke.mjs
  node tools/screens/golden.mjs
fi
if [ "$DELIVER" = 1 ]; then
  # The note has its headings and the APK's link (PRC-11); the build still works, and the delivered APK, as
  # committed, is a correct release build.
  python3 tools/filecheck.py note
  tools/build-apk.sh check
  (cd dist && sha256sum --quiet -c kindling.apk.sha256)
  tools/verify-apk.sh dist/kindling.apk release
elif changed android crates/kd-android crates/kd-app crates/kd-render; then
  tools/build-apk.sh check
fi

MINUTES=$((($(date +%s) - T0 + 59) / 60))
mkdir -p results/checks
printf '{"commit": "%s", "result": "PASS", "deliver": %s, "minutes": %d}\n' \
  "$COMMIT" "$([ "$DELIVER" = 1 ] && echo true || echo false)" "$MINUTES" >"results/checks/$COMMIT.json"
echo "Checks: PASS $COMMIT"
