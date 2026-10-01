# B01/B02 pre-test: shared settings. Source it: . scripts/env.sh
# Big files (build output, PractRand, NDK) live in the shared cache, never in the repo.
export CACHE=/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/cache
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
