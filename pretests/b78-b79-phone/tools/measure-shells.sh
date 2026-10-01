#!/usr/bin/env bash
# B78 (X10): build times of the four app shells, under the shared CPU lock (one shell per hold).
# Per round: clean build (outputs deleted, cold JVM, download caches warm), then a one-line
# change to the native core rebuilt with a warm Gradle daemon. Appends to results/build-times.csv.
# Usage: tools/measure-shells.sh [shell ...]     (default: all four; REPS=3)
set -euo pipefail
HERE=$(cd "$(dirname "$0")/.." && pwd)
. "$HERE/tools/env.sh"
OUT=$HERE/results/build-times.csv
REPS=${REPS:-3}

tag() { # set the shell's core tag constant (the one-line native change)
  if [ "$1" = s2-kotlin-cpp ]; then
    sed -i "s/constexpr uint64_t KCORE_TAG = [0-9]*;/constexpr uint64_t KCORE_TAG = $2;/" "$HERE/shells/s2-kotlin-cpp/core/kcore.h"
  else
    sed -i "s/pub const TAG: u64 = [0-9]*;/pub const TAG: u64 = $2;/" "$HERE/shells/core/src/lib.rs"
  fi
}
clean() {
  local d=$HERE/shells/$1
  case $1 in
    s3-rust-native) rm -rf "$d/target" "$d/build" ;;
    s2-kotlin-cpp) rm -rf "$d/app/build" "$d/.gradle" "$d/app/.cxx" ;;
    *) rm -rf "$d/app/build" "$d/.gradle" "$d/rust/target" ;;
  esac
}
build() { # shell, daemon|cold
  local d=$HERE/shells/$1
  if [ "$1" = s3-rust-native ]; then (cd "$d" && ./build.sh > /dev/null 2>&1)
  elif [ "$2" = cold ]; then (cd "$d" && gradle assembleRelease -q --no-daemon > /dev/null 2>&1)
  else (cd "$d" && gradle assembleRelease -q > /dev/null 2>&1); fi
}
timed() { # shell, kind, run, mode
  local s e
  s=$(date +%s.%N); build "$1" "$4"; e=$(date +%s.%N)
  echo "$1,$2,$3,$(awk "BEGIN{printf \"%.1f\", $e-$s}")" | tee -a "$OUT"
}

if [ "${1:-}" = --inner ]; then
  sh=$2
  for r in $(seq 1 "$REPS"); do
    tag "$sh" 0; clean "$sh"
    timed "$sh" clean "$r" cold
    [ "$sh" = s3-rust-native ] || build "$sh" daemon # start the daemon (nothing to rebuild)
    tag "$sh" "$r"
    timed "$sh" one-line-change "$r" daemon
  done
  tag "$sh" 0
  [ "$sh" = s3-rust-native ] || (cd "$HERE/shells/$sh" && gradle --stop -q > /dev/null 2>&1 || true)
  exit 0
fi

[ -f "$OUT" ] || echo "shell,kind,run,seconds" > "$OUT"
for sh in ${*:-s1-kotlin-rust s2-kotlin-cpp s3-rust-native s4-webview}; do
  flock "$LOCK" "$0" --inner "$sh"
done
