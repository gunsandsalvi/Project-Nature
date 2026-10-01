#!/usr/bin/env bash
# B78 (X10): fetch the Android toolchain into the shared cache, never into the repo.
# Safe to re-run: every step is skipped when already done.
# Writes download sizes to results/downloads.csv (for the toolchain-size criterion).
set -euo pipefail

CACHE=${CACHE:-/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/cache}
HERE=$(cd "$(dirname "$0")/.." && pwd)
SDK=$CACHE/android-sdk
CSV=$HERE/results/downloads.csv
mkdir -p "$CACHE" "$SDK" "$HERE/results"
[ -f "$CSV" ] || echo "item,bytes" > "$CSV"

log_size() { # name, file
  if ! grep -q "^$1," "$CSV"; then echo "$1,$(stat -c %s "$2")" >> "$CSV"; fi
}

# 1. Command-line tools (sdkmanager).
CLT_ZIP=$CACHE/commandlinetools-linux-16111833_latest.zip
if [ ! -x "$SDK/cmdline-tools/latest/bin/sdkmanager" ]; then
  [ -f "$CLT_ZIP" ] || curl -fsSL -o "$CLT_ZIP.part" \
    https://dl.google.com/android/repository/commandlinetools-linux-16111833_latest.zip && \
    { [ -f "$CLT_ZIP" ] || mv "$CLT_ZIP.part" "$CLT_ZIP"; }
  rm -rf "$SDK/cmdline-tools/tmp" && mkdir -p "$SDK/cmdline-tools/tmp"
  unzip -q "$CLT_ZIP" -d "$SDK/cmdline-tools/tmp"
  mv "$SDK/cmdline-tools/tmp/cmdline-tools" "$SDK/cmdline-tools/latest"
  rmdir "$SDK/cmdline-tools/tmp"
fi
log_size cmdline-tools "$CLT_ZIP"

# 2. SDK packages. sdkmanager prints the archive it fetches; sizes come from its cache below.
SDKM="$SDK/cmdline-tools/latest/bin/sdkmanager --sdk_root=$SDK"
yes | $SDKM --licenses > /dev/null 2>&1 || true
BT=${BUILD_TOOLS:-36.1.0}
PKGS=("platform-tools" "build-tools;$BT" "platforms;android-36")
if [ "${WITH_CMAKE:-0}" = 1 ]; then PKGS+=("cmake;${CMAKE_VER:-3.31.6}"); fi
$SDKM --install "${PKGS[@]}" > "$CACHE/sdkmanager-install.txt" 2>&1 || {
  tail -20 "$CACHE/sdkmanager-install.txt"; exit 1; }

# 3. NDK r30, shared with other tests: one installer at a time, ready file when done.
NDK_READY=$CACHE/android-ndk.ready
(
  flock 9
  if [ -f "$NDK_READY" ] && [ -d "$(cat "$NDK_READY")" ]; then
    echo "NDK already installed at $(cat "$NDK_READY")"
  else
    NDK_ZIP=$CACHE/android-ndk-r30-linux.zip
    [ -f "$NDK_ZIP" ] || { curl -fsSL -o "$NDK_ZIP.part" \
      https://dl.google.com/android/repository/android-ndk-r30-linux.zip && mv "$NDK_ZIP.part" "$NDK_ZIP"; }
    rm -rf "$CACHE/ndk-unzip" && mkdir -p "$CACHE/ndk-unzip"
    unzip -q "$NDK_ZIP" -d "$CACHE/ndk-unzip"
    NDK_DIR=$(ls -d "$CACHE"/ndk-unzip/android-ndk-*)
    mv "$NDK_DIR" "$CACHE/"
    rmdir "$CACHE/ndk-unzip"
    echo "$CACHE/$(basename "$NDK_DIR")" > "$NDK_READY"
  fi
) 9> "$CACHE/ndk-install.lock"
[ -f "$CACHE/android-ndk-r30-linux.zip" ] && log_size ndk-r30 "$CACHE/android-ndk-r30-linux.zip"

echo "SDK: $SDK"
echo "NDK: $(cat "$NDK_READY")"
