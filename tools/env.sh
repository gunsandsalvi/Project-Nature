# Kindling: build environment (A2.8, A15.2; from B78). Source it: `. tools/env.sh`
# Big downloads and caches live in $KD_CACHE, never in the repository.
export KD_CACHE=${KD_CACHE:-$HOME/.cache/kindling}
CACHE=$KD_CACHE
export ANDROID_HOME=$CACHE/android-sdk
export ANDROID_SDK_ROOT=$ANDROID_HOME
if [ -f "$CACHE/android-ndk.ready" ]; then
  export ANDROID_NDK_HOME=$(cat "$CACHE/android-ndk.ready")
  export ANDROID_NDK_ROOT=$ANDROID_NDK_HOME
  # The NDK's compiler for the phone target, so crates with C code build and lint outside Gradle too.
  TC=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
  export CC_aarch64_linux_android=$TC/aarch64-linux-android31-clang AR_aarch64_linux_android=$TC/llvm-ar
  export CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER=$TC/aarch64-linux-android31-clang
fi
export GRADLE_USER_HOME=$CACHE/gradle-home
export BT=$ANDROID_HOME/build-tools/36.1.0
export LOCK=$CACHE/cpu.lock
export PATH=$KD_CACHE/cargo/bin:$BT:$ANDROID_HOME/platform-tools:$PATH
export PLAYWRIGHT_PATH=${PLAYWRIGHT_PATH:-/opt/node-tools/node_modules/playwright}
export CHROMIUM_PATH=${CHROMIUM_PATH:-/opt/pw-browsers/chromium-1194/chrome-linux/chrome}
