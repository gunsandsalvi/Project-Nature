#!/bin/bash
# B01: ARM builds. Run under the CPU lock: flock $LOCK scripts/build-arm.sh
#  1. the phone library: cargo ndk -t arm64-v8a ... --features android  -> libkbench.so
#  2. an aarch64 Linux command-line tool (clang for C++, like the NDK) to run under qemu
#  3. (best effort) a static Android command-line tool, also run under qemu
set -e
. "$(dirname "$0")/env.sh"
unset RUSTFLAGS KBENCH_CXX_MARCH CXX   # ARM builds use the compilers' default ARM baseline
export ANDROID_NDK_HOME=$(cat $CACHE/android-ndk.ready)
cd $B01/kbench

echo "== 1. cargo ndk (phone library)"
cargo ndk -t arm64-v8a --platform 29 build --release --features android 2>&1 | grep -E "^(warning|error)|Finished|-->" || true
SO=$CARGO_TARGET_DIR/aarch64-linux-android/release/libkbench.so
ls -la $SO
$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-nm -D --defined-only $SO | grep -E "Java_dev_kindling" || true
$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf -d $SO | grep NEEDED || true

echo "== 2. aarch64 Linux command-line tool (for qemu)"
CC_aarch64_unknown_linux_gnu=clang CXX_aarch64_unknown_linux_gnu=clang++ \
CFLAGS_aarch64_unknown_linux_gnu="--target=aarch64-linux-gnu" CXXFLAGS_aarch64_unknown_linux_gnu="--target=aarch64-linux-gnu" \
CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER=aarch64-linux-gnu-gcc \
  cargo build --release --target aarch64-unknown-linux-gnu --bin kbench 2>&1 | grep -E "^(warning|error)|Finished|-->" || true
ls -la $CARGO_TARGET_DIR/aarch64-unknown-linux-gnu/release/kbench

echo "== 3. static Android command-line tool (best effort)"
RUSTFLAGS="-C target-feature=+crt-static" \
  cargo ndk -t arm64-v8a --platform 29 build --release --bin kbench 2>&1 | grep -E "^(warning|error)|Finished|-->" || true
ls -la $CARGO_TARGET_DIR/aarch64-linux-android/release/kbench 2>/dev/null && file $CARGO_TARGET_DIR/aarch64-linux-android/release/kbench || true
