# B01/B02 pre-test: shared settings. Source it: . scripts/env.sh
# Big files (build output, PractRand, NDK) live in the shared cache, never in the repo.
# The cache is the session scratchpad's cache/ folder; set KB_CACHE to override.
export CACHE=${KB_CACHE:-$(ls -d /tmp/*/-home-user-Project-Nature/*/scratchpad/cache 2>/dev/null | head -1)}
export LOCK=$CACHE/cpu.lock                 # every CPU-heavy command runs under: flock $LOCK ...
export CARGO_TARGET_DIR=$CACHE/b01-target
export B01=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# Cloud build: clang for C++ (same compiler family as the Android NDK), and x86-64-v2
# (128-bit vectors, no FMA) as the closest x86 match to the phone's 128-bit NEON.
export CXX=clang++
export KBENCH_CXX_MARCH=x86-64-v2
export RUSTFLAGS="-C target-cpu=x86-64-v2"
export KBENCH=$CARGO_TARGET_DIR/release/kbench
export PRACTRAND=$CACHE/practrand/src/RNG_test
export RAW=${RAW:-$CACHE/../b01/raw}      # raw run output (JSON lines), outside the repo
