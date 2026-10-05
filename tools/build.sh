#!/usr/bin/env bash
# Builds, signs and checks a step's APK (PLT-06, PRC-11, A2.3). Usage: tools/build.sh <step> release|check
#   release  signed with the release key that tools/signing-key.py derives from the passphrase secret, and copied to
#            dist/kindling.apk with its SHA-256
#   check    signed with a key made for this build and then dropped, to check a build without the secret
# The step, such as 0.1a, sets the version (tools/version.sh): the project's own (shown by the app) and its export
# preset's code, both committed with the step. The project is pre-production's app until the game exists.
# Godot exports the APK headless and unsigned; zipalign and apksigner finish it, so only tools/signing-key.py
# reads the secret.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
MODE="${2:-}"
case "$MODE" in release | check) ;; *) echo "usage: tools/build.sh <step> release|check" >&2; exit 2 ;; esac
VERSION="$(tools/version.sh "${1:-}")"
read -r CODE NAME <<<"$VERSION"
. tools/env.sh
[ -x "$GODOT" ] && [ -x "$KD_BUILD_TOOLS/apksigner" ] && [ -s "$KD_TEMPLATES/android_release.apk" ] || tools/setup.sh
PROJECT=prototypes/app
[ -f game/project.godot ] && PROJECT=game
T0=$(date +%s)
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

sed -i "s/^config\/version=.*/config\/version=\"$NAME\"/" "$PROJECT/project.godot"
sed -i "s/^version\/code=.*/version\/code=$CODE/" "$PROJECT/export_presets.cfg"
grep -qx "config/version=\"$NAME\"" "$PROJECT/project.godot" || { echo "Build: no config/version in $PROJECT/project.godot"; exit 1; }
grep -qx "version/code=$CODE" "$PROJECT/export_presets.cfg" || { echo "Build: no version/code in $PROJECT/export_presets.cfg"; exit 1; }

# The app's Godot extensions, each C++ prototype's extension/ built for the phone's arm64 Android with the NDK, and
# for this machine, whose library Godot's import loads, in the folder tools/check.sh builds the prototype in, so
# neither compiles it twice; each build puts its library where the app looks for it, so none is committed (A2.2).
for EXT in prototypes/*/extension; do
  [ -f "$EXT/CMakeLists.txt" ] || continue
  D="$(dirname "$EXT")"
  B="build/${D//\//-}"
  { cmake -S "$D" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && cmake --build "$B"; } >"$TMP/log" 2>&1 || { tail -40 "$TMP/log"; echo "Build: $D failed"; exit 1; }
  B="build/android-$(basename "$D")"
  cmake -S "$EXT" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=Release -DGODOT_CPP="$KD_GODOT_CPP" \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-24 >"$TMP/log" 2>&1 || { tail -40 "$TMP/log"; echo "Build: $EXT's configure failed"; exit 1; }
  cmake --build "$B" >"$TMP/log" 2>&1 || { tail -40 "$TMP/log"; echo "Build: $EXT failed"; exit 1; }
done

# The export preset leaves out addons/ and test/, so gdUnit4, which the checks copy in, never reaches the phone.
timeout 600 "$GODOT" --headless --path "$PROJECT" --import >"$TMP/log" 2>&1 || { tail -40 "$TMP/log"; exit 1; }
timeout 900 "$GODOT" --headless --path "$PROJECT" --export-release Android "$TMP/unsigned.apk" >"$TMP/log" 2>&1 \
  || { tail -40 "$TMP/log"; echo "Build: Godot's export failed"; exit 1; }
[ -s "$TMP/unsigned.apk" ] || { tail -40 "$TMP/log"; echo "Build: Godot made no APK"; exit 1; }

"$KD_BUILD_TOOLS/zipalign" -f -P 16 4 "$TMP/unsigned.apk" "$TMP/aligned.apk"
SIGN=("$KD_BUILD_TOOLS/apksigner" sign --v1-signing-enabled false --v2-signing-enabled true
  --v3-signing-enabled true --out "$TMP/kindling.apk")
if [ "$MODE" = release ]; then
  python3 tools/signing-key.py pk8 "$TMP/key.pk8"
  CERT=android/keys/release-cert.der
else
  python3 tools/signing-key.py throwaway "$TMP/key.pk8" "$TMP/cert.der"
  CERT="$TMP/cert.der"
fi
"${SIGN[@]}" --key "$TMP/key.pk8" --cert "$CERT" "$TMP/aligned.apk" >"$TMP/log" 2>&1 \
  || { rm -f "$TMP/key.pk8"; cat "$TMP/log"; echo "Build: signing failed"; exit 1; }
rm -f "$TMP/key.pk8"
tools/verify-apk.sh "$TMP/kindling.apk" "$MODE" "$NAME"
SECS=$(($(date +%s) - T0))
if [ "$MODE" = release ]; then
  mkdir -p dist
  cp "$TMP/kindling.apk" dist/kindling.apk
  (cd dist && sha256sum kindling.apk >kindling.apk.sha256)
  echo "Build: dist/kindling.apk, $NAME ($CODE), $(($(stat -c %s dist/kindling.apk) / 1024)) KB, $SECS s"
else
  echo "Build check: OK, $NAME ($CODE), signed with a key made for it, $SECS s"
fi
