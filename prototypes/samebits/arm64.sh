#!/usr/bin/env bash
# The same bits on arm64 (RES-05, A3.4), for P5 and every prototype after it: a command line program built for arm64
# Linux with the cross compiler from the given sources, run under qemu on one thread and on four, its last line (the
# run's digest or checksum) compared with this machine's run on one thread. Usage:
#   arm64.sh <this machine's program> "<the arguments after the thread count>" <compiler arguments: -I folders, sources>
# Pre-production code (research 00).
set -euo pipefail
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
NATIVE="$1"
ARGS="$2"
shift 2
aarch64-linux-gnu-g++ -std=c++20 -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math -static -pthread "$@" \
  -o "$TMP/arm64"
# shellcheck disable=SC2086 # the arguments are words
"$NATIVE" 1 $ARGS | tail -1 >"$TMP/here"
for threads in 1 4; do
  # shellcheck disable=SC2086
  qemu-aarch64-static "$TMP/arm64" "$threads" $ARGS | tail -1 >"$TMP/arm64-$threads"
  diff -q "$TMP/here" "$TMP/arm64-$threads" >/dev/null || {
    echo "arm64 on $threads threads differs from this machine:"
    diff "$TMP/here" "$TMP/arm64-$threads"
    exit 1
  }
done
echo "arm64 under qemu, 1 and 4 threads: $(cat "$TMP/here")"
