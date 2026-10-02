#!/usr/bin/env bash
# Kindling: everything a fresh cloud session needs to build and test (A2.8, A15.2). Safe to re-run.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd); cd "$ROOT"
export KD_CACHE=${KD_CACHE:-$HOME/.cache/kindling}
mkdir -p "$KD_CACHE/cargo"; export PATH="$KD_CACHE/cargo/bin:$PATH"
as_root() { if [ "$(id -u)" = 0 ]; then "$@"; else sudo -n "$@"; fi; }
# 1. Rust 1.97.0 with its components and targets (rust-toolchain.toml)
rustup toolchain install 1.97.0 --profile minimal --component clippy,rustfmt \
  --target aarch64-linux-android,wasm32-unknown-unknown,aarch64-unknown-linux-gnu
# 2. cargo-ndk 4.1.2 and the wasm-bindgen CLI at Cargo.lock's version
[ "$(cargo ndk --version 2>/dev/null || true)" = "cargo-ndk 4.1.2" ] || \
  cargo install --locked cargo-ndk --version 4.1.2 --root "$KD_CACHE/cargo"
WB=$(sed -n '/^name = "wasm-bindgen"$/{n;s/^version = "\(.*\)"$/\1/p;q;}' "$ROOT/Cargo.lock")
[ -n "$WB" ] || { echo "Setup: Cargo.lock names no wasm-bindgen"; exit 1; }
[ "$(wasm-bindgen --version 2>/dev/null || true)" = "wasm-bindgen $WB" ] || \
  cargo install --locked wasm-bindgen-cli --version "$WB" --root "$KD_CACHE/cargo"
# 3. qemu and the arm64 linker
{ command -v qemu-aarch64-static && command -v aarch64-linux-gnu-gcc; } >/dev/null || \
  { as_root apt-get update -qq && as_root apt-get install -y -qq qemu-user-static gcc-aarch64-linux-gnu; }
# 4. Android SDK and NDK r30 (B78's script)
"$ROOT/tools/setup-toolchain.sh"
# 5. Report
. "$ROOT/tools/env.sh"
echo "rust      $(rustc --version)"
echo "targets   $(rustup target list --installed | tr '\n' ' ')"
echo "tools     $(cargo ndk --version), $(wasm-bindgen --version)"
echo "android   ndk $(basename "$ANDROID_NDK_HOME"), platforms $(ls "$ANDROID_HOME/platforms" | tr '\n' ' ')build-tools $(ls "$ANDROID_HOME/build-tools" | tr '\n' ' ')"
echo "jvm       $(java -version 2>&1 | grep -m1 version), $(gradle --version 2>/dev/null | grep -m1 '^Gradle')"
echo "web       node $(node --version), chromium $([ -x "$CHROMIUM_PATH" ] && echo ok || echo MISSING), python $(python3 -c 'import cryptography; print("ok")' 2>/dev/null || echo 'cryptography MISSING')"
[ -x "$CHROMIUM_PATH" ] || { echo "Setup: Chromium missing at $CHROMIUM_PATH"; exit 1; }
echo "Setup: OK"
