#!/usr/bin/env bash
# Everything a fresh cloud session needs to build, test and deliver Kindling (A2.4).
# Safe to run again: each part installs only what is missing, into $KD_CACHE (never the repository), and the script
# says nothing when all is present. Every download is pinned to a version and checked by its checksum or commit;
# the system's packages come from its Ubuntu release, the C++ format and lint tools by their version, 18.
# It prints one line for each part it installs; on a failure, the command, its exit code and its output.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/tools/env.sh"
mkdir -p "$KD_CACHE/bin"

fail() { echo "Setup: FAILED: $1 (exit $2)" >&2; exit 1; }
quiet() {   # runs a command, showing its output only if it fails
  local log rc
  log="$(mktemp)"
  "$@" >"$log" 2>&1
  rc=$?
  [ "$rc" -eq 0 ] || { tail -40 "$log" >&2; rm -f "$log"; fail "$*" "$rc"; }
  rm -f "$log"
}
as_root() { if [ "$(id -u)" -eq 0 ]; then "$@"; else sudo -n "$@"; fi; }
fetch() {   # url, file, checksum (SHA-512 or SHA-256): downloads once, then checks the file
  if [ ! -s "$2" ]; then
    quiet curl -fsSL --retry 5 --retry-delay 2 -o "$2.part" "$1"
    quiet mv "$2.part" "$2"
  fi
  case "${#3}" in
    128) echo "$3  $2" | sha512sum -c --quiet - >/dev/null 2>&1 || fail "the checksum of $2" 1 ;;
    64) echo "$3  $2" | sha256sum -c --quiet - >/dev/null 2>&1 || fail "the checksum of $2" 1 ;;
    *) fail "no checksum for $2" 1 ;;
  esac
}

# 1. System packages: Xvfb and the software Vulkan driver, so Godot draws pictures without a graphics chip; qemu and
#    the arm64 compiler for the same-bits check (A3.4); the C++ build with its compiler cache, format and lint tools;
#    Java for export.
need=()
command -v Xvfb >/dev/null || need+=(xvfb)
[ -f /usr/share/vulkan/icd.d/lvp_icd.json ] || need+=(mesa-vulkan-drivers)
command -v qemu-aarch64-static >/dev/null || need+=(qemu-user-static)
command -v aarch64-linux-gnu-g++ >/dev/null || need+=(g++-aarch64-linux-gnu)
command -v cmake >/dev/null || need+=(cmake)
command -v ninja >/dev/null || need+=(ninja-build)
command -v ccache >/dev/null || need+=(ccache)
command -v clang-format-18 >/dev/null || need+=(clang-format-18)
command -v clang-tidy-18 >/dev/null || need+=(clang-tidy-18)
[ -x "$JAVA_HOME/bin/java" ] || need+=(openjdk-21-jdk-headless)
if [ "${#need[@]}" -gt 0 ]; then
  echo "Setup: installing ${need[*]}"
  quiet as_root apt-get update -qq
  quiet as_root env DEBIAN_FRONTEND=noninteractive apt-get install -y -qq "${need[@]}"
fi
python3 -c 'import cryptography' 2>/dev/null || fail "python3's cryptography module, which signing needs, is missing" 1

# 2. Godot 4.7.2 (A2.2), checked against the release's published SHA-512
if [ "$("$GODOT" --version 2>/dev/null | tail -1)" != "$KD_GODOT_VERSION" ]; then
  echo "Setup: installing Godot 4.7.2"
  ZIP="$KD_CACHE/Godot_v4.7.2-stable_linux.x86_64.zip"
  fetch https://github.com/godotengine/godot/releases/download/4.7.2-stable/Godot_v4.7.2-stable_linux.x86_64.zip "$ZIP" \
    9aa00f7a605200940bce3027a567b782f49bd8e940dd06ae9e987bd65aee1b1467edd56ed84fcdcbdd44354bf613bdbb4e5d2913e925850368e150c59ed54c65
  mkdir -p "$KD_CACHE/godot"
  quiet unzip -o -q "$ZIP" -d "$KD_CACHE/godot"
  rm -f "$ZIP"
fi

# 3. Its Android export templates, where Godot looks for them
if [ "$(cat "$KD_TEMPLATES/version.txt" 2>/dev/null)" != "4.7.2.stable" ] || [ ! -s "$KD_TEMPLATES/android_release.apk" ]; then
  echo "Setup: installing Godot's Android export templates"
  TPZ="$KD_CACHE/Godot_v4.7.2-stable_export_templates.tpz"
  fetch https://github.com/godotengine/godot/releases/download/4.7.2-stable/Godot_v4.7.2-stable_export_templates.tpz "$TPZ" \
    ca4d71c4d7b81dfc15d1a98baa07534aa95b03fdda78a0075b06672e1648d2e5f40980c9adc28d23e1b92e732ee7bf3461997aa804af74ec2fcd7a93ccb84079
  mkdir -p "$KD_TEMPLATES"
  quiet unzip -o -q -j "$TPZ" templates/version.txt templates/android_release.apk templates/android_debug.apk -d "$KD_TEMPLATES"
  rm -f "$TPZ"
fi

# 4. The Android SDK: command-line tools 16111833, platform tools, build tools 36.1.0 and platform 36
SDKM="$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager"
if [ ! -x "$SDKM" ]; then
  echo "Setup: installing the Android command-line tools"
  ZIP="$KD_CACHE/commandlinetools-linux-16111833_latest.zip"
  fetch https://dl.google.com/android/repository/commandlinetools-linux-16111833_latest.zip "$ZIP" \
    0877a1d048fe4a24efe2eff536ca4223f7adeb58648bb81909d33c446918cfa8
  rm -rf "$KD_CACHE/cmdline-tools.tmp" "$ANDROID_HOME/cmdline-tools/latest"
  quiet unzip -q "$ZIP" -d "$KD_CACHE/cmdline-tools.tmp"
  mkdir -p "$ANDROID_HOME/cmdline-tools"
  quiet mv "$KD_CACHE/cmdline-tools.tmp/cmdline-tools" "$ANDROID_HOME/cmdline-tools/latest"
  rm -rf "$KD_CACHE/cmdline-tools.tmp" "$ZIP"
fi
if [ ! -d "$ANDROID_HOME/platforms/android-36" ] || [ ! -x "$KD_BUILD_TOOLS/apksigner" ] \
   || [ ! -d "$ANDROID_HOME/platform-tools" ]; then
  echo "Setup: installing the Android SDK's platform 36 and build tools 36.1.0"
  yes 2>/dev/null | "$SDKM" --sdk_root="$ANDROID_HOME" --licenses >/dev/null 2>&1
  quiet "$SDKM" --sdk_root="$ANDROID_HOME" "platform-tools" "build-tools;36.1.0" "platforms;android-36"
fi

# 5. NDK r30 (30.0.16248370), for the C++ simulation on the phone
if ! grep -q '30.0.16248370' "$ANDROID_NDK_HOME/source.properties" 2>/dev/null; then
  echo "Setup: installing the Android NDK r30"
  ZIP="$KD_CACHE/android-ndk-r30-linux.zip"
  fetch https://dl.google.com/android/repository/android-ndk-r30-linux.zip "$ZIP" \
    753611f410d002cfcd3f3dc2ef49aad532089d3180b436c060a90bf0fcb64df2
  rm -rf "$ANDROID_NDK_HOME"
  quiet unzip -q "$ZIP" -d "$KD_CACHE"
  rm -f "$ZIP"
  grep -q '30.0.16248370' "$ANDROID_NDK_HOME/source.properties" || fail "the NDK at $ANDROID_NDK_HOME is not r30" 1
fi

# 6. Godot's editor settings: where the Android SDK and Java are, for export
quiet python3 - "$HOME/.config/godot/editor_settings-4.7.tres" "$ANDROID_HOME" "$JAVA_HOME" <<'EOF'
import os, re, sys
path, sdk, java = sys.argv[1:]
want = {"export/android/android_sdk_path": sdk, "export/android/java_sdk_path": java}
text = open(path).read() if os.path.exists(path) else '[gd_resource type="EditorSettings" format=3]\n\n[resource]\n'
for key, value in want.items():
    line = f'{key} = "{value}"'
    text, n = re.subn(rf"^{re.escape(key)} = .*$", line, text, flags=re.M)
    if not n:
        text = text.rstrip("\n") + "\n" + line + "\n"
os.makedirs(os.path.dirname(path), exist_ok=True)
if not os.path.exists(path) or open(path).read() != text:
    open(path, "w").write(text)
EOF

# 7. The formatters and linters of GDScript, gdtoolkit 4.3.3, and of Python, ruff 0.15.20, in their own Python
#    environment
if [ "$("$KD_CACHE/bin/gdformat" --version 2>/dev/null)" != "gdformat 4.3.3" ] \
   || [ "$("$KD_CACHE/bin/ruff" --version 2>/dev/null)" != "ruff 0.15.20" ]; then
  echo "Setup: installing gdtoolkit 4.3.3 and ruff 0.15.20"
  quiet python3 -m venv --system-site-packages "$KD_CACHE/py"
  quiet "$KD_CACHE/py/bin/pip" install -q --disable-pip-version-check gdtoolkit==4.3.3 ruff==0.15.20
  for tool in gdformat gdlint ruff; do ln -sf "$KD_CACHE/py/bin/$tool" "$KD_CACHE/bin/$tool"; done
fi

# 8. gdUnit4 6.2.1, Godot's test framework, at its release's commit
GDUNIT_COMMIT=08ffc7c65b61b1b2edd545616061a99973c13ce1
if [ "$(git -C "$KD_GDUNIT" rev-parse HEAD 2>/dev/null)" != "$GDUNIT_COMMIT" ]; then
  echo "Setup: installing gdUnit4 6.2.1"
  rm -rf "$KD_GDUNIT"
  quiet git clone -q -c advice.detachedHead=false --depth 1 --branch v6.2.1 https://github.com/MikeSchulze/gdUnit4 "$KD_GDUNIT"
  [ "$(git -C "$KD_GDUNIT" rev-parse HEAD)" = "$GDUNIT_COMMIT" ] || fail "gdUnit4 v6.2.1 is not at commit $GDUNIT_COMMIT" 1
fi

# 9. godot-cpp at its 4.5 release, for the game's Godot extension (view/, A2.2): Godot 4.7 loads extensions built
#    for 4.5, the newest release godot-cpp has tagged
GODOT_CPP_COMMIT=e83fd0904c13356ed1d4c3d09f8bb9132bdc6b77
if [ "$(git -C "$KD_GODOT_CPP" rev-parse HEAD 2>/dev/null)" != "$GODOT_CPP_COMMIT" ]; then
  echo "Setup: installing godot-cpp 4.5"
  rm -rf "$KD_GODOT_CPP"
  quiet git clone -q -c advice.detachedHead=false --depth 1 --branch godot-4.5-stable https://github.com/godotengine/godot-cpp "$KD_GODOT_CPP"
  [ "$(git -C "$KD_GODOT_CPP" rev-parse HEAD)" = "$GODOT_CPP_COMMIT" ] || fail "godot-cpp 4.5 is not at commit $GODOT_CPP_COMMIT" 1
fi

# 10. doctest 2.4.11's one header, for C++ tests, checked against its SHA-256
if [ ! -s "$KD_DOCTEST/doctest.h" ]; then
  echo "Setup: installing doctest 2.4.11"
  mkdir -p "$KD_DOCTEST"
fi
fetch https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h "$KD_DOCTEST/doctest.h" \
  44faa038e9c3f9728efbda143748d01124ea0a27f4bf78f35a15d8fab2e039fb
exit 0
