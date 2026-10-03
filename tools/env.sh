# Kindling's build environment (A2.8), sourced by every build script after tools/setup.sh has run once.
# Everything downloaded lives under $KD_CACHE, never in the repository.
export KD_CACHE="${KD_CACHE:-$HOME/.cache/kindling}"
export PATH="$KD_CACHE/cargo/bin:$PATH"

# Android: the SDK, NDK r30 and Gradle's cache (A2.5, A2.8).
export ANDROID_HOME="$KD_CACHE/android-sdk"
export ANDROID_SDK_ROOT="$ANDROID_HOME"
export ANDROID_NDK_HOME="$KD_CACHE/android-ndk-r30"
export GRADLE_USER_HOME="$KD_CACHE/gradle-home"
export KD_BUILD_TOOLS="$ANDROID_HOME/build-tools/36.1.0"     # zipalign and apksigner

# The NDK's compiler for the phone target, so crates with C code build and lint outside Gradle too.
KD_NDK_BIN="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin"
export CC_aarch64_linux_android="$KD_NDK_BIN/aarch64-linux-android31-clang"
export AR_aarch64_linux_android="$KD_NDK_BIN/llvm-ar"
export CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER="$KD_NDK_BIN/aarch64-linux-android31-clang"

# The session image's Playwright and Chromium, for the screen tests (A15.11).
export PLAYWRIGHT_PATH="${PLAYWRIGHT_PATH:-/opt/node-tools/node_modules/playwright}"
export CHROMIUM_PATH="${CHROMIUM_PATH:-/opt/pw-browsers/chromium}"
