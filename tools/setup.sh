#!/usr/bin/env bash
# Everything a fresh cloud session needs to build, test and deliver Kindling (A2.8, A15.2).
# Safe to run again: each part installs only what is missing, into $KD_CACHE (never the repository).
# On a failure it prints the command and its exit code, which the session passes to the owner (A15.2).
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/tools/env.sh"
mkdir -p "$KD_CACHE/cargo/bin"
SIZES="$KD_CACHE/downloads.csv"

fail() { echo "Setup: FAILED: $1 (exit $2)" >&2; exit 1; }
run() { "$@"; local rc=$?; [ "$rc" -eq 0 ] || fail "$*" "$rc"; }
as_root() { if [ "$(id -u)" -eq 0 ]; then "$@"; else sudo -n "$@"; fi; }
fetch() {   # url, file, name: downloads once and logs the size
  [ -s "$2" ] && return 0
  run curl -fsSL --retry 5 --retry-delay 2 -o "$2.part" "$1"
  run mv "$2.part" "$2"
  [ -f "$SIZES" ] || echo "item,bytes" > "$SIZES"
  echo "$3,$(stat -c %s "$2")" >> "$SIZES"
}

# 1. Rust 1.97.0 with its components and the three extra targets (rust-toolchain.toml)
run rustup toolchain install 1.97.0 --profile minimal --component clippy,rustfmt \
  --target aarch64-linux-android,wasm32-unknown-unknown,aarch64-unknown-linux-gnu

# 2. cargo-ndk 4.1.2, and the wasm-bindgen CLI at exactly Cargo.lock's version
if [ "$(cargo ndk --version 2>/dev/null)" != "cargo-ndk 4.1.2" ]; then
  run cargo install --locked cargo-ndk --version 4.1.2 --root "$KD_CACHE/cargo"
fi
WB="$(awk '$0 == "name = \"wasm-bindgen\"" { getline; gsub(/version = |"/, ""); print; exit }' "$ROOT/Cargo.lock")"
[ -n "$WB" ] || fail "reading wasm-bindgen's version from Cargo.lock" 1
if [ "$(wasm-bindgen --version 2>/dev/null)" != "wasm-bindgen $WB" ]; then
  run cargo install --locked wasm-bindgen-cli --version "$WB" --root "$KD_CACHE/cargo"
fi

# 3. qemu and the arm64 linker, for the arm64 tests (A15.9)
if ! command -v qemu-aarch64-static >/dev/null || ! command -v aarch64-linux-gnu-gcc >/dev/null; then
  run as_root apt-get update -qq
  run as_root apt-get install -y -qq qemu-user-static gcc-aarch64-linux-gnu
fi

# 4. The Android SDK: command-line tools 16111833, platform tools, build tools 36.1.0 and platform 36
SDKM="$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager"
if [ ! -x "$SDKM" ]; then
  fetch https://dl.google.com/android/repository/commandlinetools-linux-16111833_latest.zip \
    "$KD_CACHE/commandlinetools-linux-16111833_latest.zip" cmdline-tools
  rm -rf "$KD_CACHE/cmdline-tools.tmp" "$ANDROID_HOME/cmdline-tools/latest"
  run unzip -q "$KD_CACHE/commandlinetools-linux-16111833_latest.zip" -d "$KD_CACHE/cmdline-tools.tmp"
  mkdir -p "$ANDROID_HOME/cmdline-tools"
  run mv "$KD_CACHE/cmdline-tools.tmp/cmdline-tools" "$ANDROID_HOME/cmdline-tools/latest"
  rm -rf "$KD_CACHE/cmdline-tools.tmp"
fi
if [ ! -d "$ANDROID_HOME/platforms/android-36" ] || [ ! -x "$KD_BUILD_TOOLS/apksigner" ] \
   || [ ! -d "$ANDROID_HOME/platform-tools" ]; then
  yes 2>/dev/null | "$SDKM" --sdk_root="$ANDROID_HOME" --licenses >/dev/null
  run "$SDKM" --sdk_root="$ANDROID_HOME" "platform-tools" "build-tools;36.1.0" "platforms;android-36"
fi

# 5. NDK r30 (30.0.16248370)
if [ ! -f "$ANDROID_NDK_HOME/source.properties" ]; then
  fetch https://dl.google.com/android/repository/android-ndk-r30-linux.zip "$KD_CACHE/android-ndk-r30-linux.zip" ndk-r30
  rm -rf "$ANDROID_NDK_HOME"
  run unzip -q "$KD_CACHE/android-ndk-r30-linux.zip" -d "$KD_CACHE"
fi
grep -q '30.0.16248370' "$ANDROID_NDK_HOME/source.properties" || fail "the NDK at $ANDROID_NDK_HOME is not 30.0.16248370" 1

# 6. What the session image holds already (A2.8): JDK 21, Gradle 8.14.3, Node with Playwright and Chromium,
#    and Python's cryptography for signing (A15.5)
command -v java >/dev/null || fail "java (JDK 21) is missing from the session image" 1
command -v gradle >/dev/null || fail "gradle (8.14.3) is missing from the session image" 1
command -v node >/dev/null || fail "node is missing from the session image" 1
[ -d "$PLAYWRIGHT_PATH" ] || fail "Playwright is missing at $PLAYWRIGHT_PATH" 1
[ -x "$CHROMIUM_PATH" ] || fail "Chromium is missing at $CHROMIUM_PATH" 1
python3 -c 'import cryptography' 2>/dev/null || fail "python3's cryptography module is missing" 1

echo "rust      $(rustc --version)"
echo "targets   $(rustup target list --installed | tr '\n' ' ')"
echo "tools     $(cargo ndk --version), $(wasm-bindgen --version)"
echo "android   ndk $(sed -n 's/^Pkg.Revision = //p' "$ANDROID_NDK_HOME/source.properties"), platforms $(ls "$ANDROID_HOME/platforms" | tr '\n' ' ')build-tools $(ls "$ANDROID_HOME/build-tools" | tr '\n' ' ')"
echo "jvm       $(java -version 2>&1 | grep -m1 'version'), $(gradle --version 2>/dev/null | grep -m1 '^Gradle')"
echo "web       node $(node --version), playwright and chromium present, python cryptography present"
echo "Setup: OK"
