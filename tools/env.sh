# Kindling's build environment (A2.4), sourced by every build script after tools/setup.sh has run once.
# Everything downloaded lives under $KD_CACHE, never in the repository.
export KD_CACHE="${KD_CACHE:-$HOME/.cache/kindling}"
export PATH="$KD_CACHE/bin:$PATH"

# Godot 4.7.2, pinned (A2.2); its export templates where Godot looks for them; gdUnit4, its test framework.
export GODOT="$KD_CACHE/godot/Godot_v4.7.2-stable_linux.x86_64"
export KD_GODOT_VERSION="4.7.2.stable.official.ed1daf0bf"
export KD_TEMPLATES="$HOME/.local/share/godot/export_templates/4.7.2.stable"
export KD_GDUNIT="$KD_CACHE/gdunit4-v6.2.1"
# godot-cpp at its 4.5 release, which Godot 4.7 loads, for the C++ prototypes' Godot extensions; doctest's one header,
# for C++ tests (A2.2)
export KD_GODOT_CPP="$KD_CACHE/godot-cpp-4.5"
export KD_DOCTEST="$KD_CACHE/doctest-2.4.11"

# Android: the SDK, NDK r30 and Java (A2.2).
export ANDROID_HOME="$KD_CACHE/android-sdk"
export ANDROID_SDK_ROOT="$ANDROID_HOME"
export ANDROID_NDK_HOME="$KD_CACHE/android-ndk-r30"
export KD_BUILD_TOOLS="$ANDROID_HOME/build-tools/36.1.0"     # zipalign, apksigner and aapt2
export JAVA_HOME="${JAVA_HOME:-/usr/lib/jvm/java-21-openjdk-amd64}"
