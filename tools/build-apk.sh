#!/usr/bin/env bash
# Builds, signs and checks the APK (A15.3, A15.5). Usage: tools/build-apk.sh release|check
#   release  signed with the release key derived from the passphrase secret; copied to dist/ with its SHA-256
#   check    signed with a key made for this build and dropped, to check a build without the secret
# Gradle and cargo never see the secret: only tools/signing-key.py reads it (A15.5).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
MODE="${1:-}"
case "$MODE" in release | check) ;; *) echo "usage: tools/build-apk.sh release|check" >&2; exit 2 ;; esac
. tools/env.sh
if [ ! -x "$KD_BUILD_TOOLS/apksigner" ] || [ ! -f "$ANDROID_NDK_HOME/source.properties" ]; then
  tools/setup.sh
fi
. android/version.properties
export KD_BUILD="$versionName · $versionCode · $(git rev-parse --short HEAD)"
T0=$(date +%s)
env -u KINDLING_SIGNING_PASSPHRASE gradle -p android --no-daemon --console=plain -q assembleRelease
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
"$KD_BUILD_TOOLS/zipalign" -f -P 16 4 android/app/build/outputs/apk/release/app-release-unsigned.apk "$TMP/aligned.apk"
SIGN=("$KD_BUILD_TOOLS/apksigner" sign --v1-signing-enabled false --v2-signing-enabled false
  --v3-signing-enabled true --min-sdk-version 31 --out "$TMP/kindling.apk")
if [ "$MODE" = release ]; then
  python3 tools/signing-key.py pk8 "$TMP/key.pk8"
  "${SIGN[@]}" --key "$TMP/key.pk8" --cert android/keys/release-cert.der "$TMP/aligned.apk"
else
  python3 tools/signing-key.py throwaway "$TMP/key.pk8" "$TMP/cert.der"
  "${SIGN[@]}" --key "$TMP/key.pk8" --cert "$TMP/cert.der" "$TMP/aligned.apk"
fi
rm -f "$TMP/key.pk8"
tools/verify-apk.sh "$TMP/kindling.apk" "$MODE"
SECS=$(($(date +%s) - T0))
if [ "$MODE" = release ]; then
  mkdir -p dist
  cp "$TMP/kindling.apk" dist/kindling.apk
  (cd dist && sha256sum kindling.apk >kindling.apk.sha256)
  echo "APK: dist/kindling.apk ($(($(stat -c %s dist/kindling.apk) / 1024)) KB, versionCode $versionCode, ${SECS} s)"
else
  echo "APK check: OK (versionCode $versionCode, a key made for this build, ${SECS} s)"
fi
