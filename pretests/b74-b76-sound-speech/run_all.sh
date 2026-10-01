#!/usr/bin/env bash
# B74/B76: every CPU-heavy step in one go, to run under the shared lock (at most 14 minutes):
#   timeout 3600 flock "$CACHE/cpu.lock" timeout 840 ./run_all.sh
# Build outputs, clips and logs go to the cache; small results to results/.
set -uo pipefail
CACHE=${CACHE:-/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/cache}
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=$CACHE/b74-run
export CARGO_TARGET_DIR=$CACHE/b74-target
mkdir -p "$OUT" "$HERE/results"
cd "$HERE"
PY=$CACHE/b76/venv/bin/python
step() { echo "== $(date +%T) $*"; }
STEPS=${STEPS:-rust bench android speech}

if [[ $STEPS == *rust* ]]; then
  step "B74 host build and tests"
  if cargo test --release -q > "$OUT/test.log" 2>&1 && cargo build --release -q --bin render >> "$OUT/test.log" 2>&1; then
    tail -3 "$OUT/test.log"
    R=$CARGO_TARGET_DIR/release/render
    step "B74 clips"; $R clips "$OUT/clips"
    step "B74 describe"; $R describe > "$HERE/results/describe.json"
  else
    echo "RUST BUILD FAILED"; grep -E "^(error|warning: unused)" -A6 "$OUT/test.log" | head -60
  fi
fi

if [[ $STEPS == *bench* ]] && [ -x "$CARGO_TARGET_DIR/release/render" ]; then
  for i in 1 2 3; do
    step "B74 bench run $i (one core)"
    taskset -c 1 "$CARGO_TARGET_DIR/release/render" bench > "$HERE/results/bench-run$i.json"
  done
fi

if [[ $STEPS == *android* ]]; then
  step "B74 Android build"
  . "$HERE/../b78-b79-phone/tools/env.sh" 2>/dev/null || true
  export ANDROID_NDK_HOME=${ANDROID_NDK_HOME:-$(cat "$CACHE/android-ndk.ready")}
  T0=$(date +%s)
  if cargo ndk -t arm64-v8a -P 31 build --release --lib --features android > "$OUT/android.log" 2>&1; then
    SO=$CARGO_TARGET_DIR/aarch64-linux-android/release/libksound.so
    LLVM=$(ls -d "$ANDROID_NDK_HOME"/toolchains/llvm/prebuilt/*/bin | head -1)
    {
      echo "built in $(( $(date +%s) - T0 )) s; size $(stat -c %s "$SO") bytes"
      echo "exports:"; "$LLVM/llvm-nm" -D --defined-only "$SO" | grep -i java_
      echo "needs:"; "$LLVM/llvm-readelf" -d "$SO" | grep NEEDED
      echo "load alignment:"; "$LLVM/llvm-readelf" -lW "$SO" | grep LOAD | awk '{print $NF}' | sort -u
    } | tee "$HERE/results/android-build.txt"
  else
    echo "ANDROID BUILD FAILED"; grep -E "^error" -A8 "$OUT/android.log" | head -60
  fi
fi

if [[ $STEPS == *speech* ]]; then
  step "B76 sounds kept"; $PY speech.py kept "$HERE/results/kept.json"
  step "B76 clips"; $PY speech.py clips "$OUT/speech" > "$OUT/speech-clips.log" 2>&1; tail -c 600 "$OUT/speech-clips.log"; echo
  cp "$OUT/speech/speech-meta.json" "$HERE/results/" 2>/dev/null
  for i in 1 2 3; do
    step "B76 bench run $i (one core)"
    $PY speech.py bench > "$HERE/results/speech-bench-run$i.json"
  done
fi

if [[ $STEPS == *page* ]]; then
  step "listening page"; $PY build_page.py "$OUT"
fi
step done
