#!/usr/bin/env bash
# Builds, signs and checks the APK (A15.3, A15.5). Usage: tools/build-apk.sh release|check
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd); cd "$ROOT"; MODE=${1:-release}
[ -f "${KD_CACHE:-$HOME/.cache/kindling}/android-ndk.ready" ] || tools/setup.sh
. tools/env.sh; . android/version.properties
export KD_BUILD="$versionName $(git rev-parse --short HEAD)"
env -u KINDLING_SIGNING_PASSPHRASE gradle -p android --no-daemon --console=plain -q assembleRelease
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
"$BT/zipalign" -f -P 16 4 android/app/build/outputs/apk/release/app-release-unsigned.apk "$TMP/aligned.apk"
SIGN=("$BT/apksigner" sign --v1-signing-enabled false --v2-signing-enabled true --v3-signing-enabled true --min-sdk-version 31 --out "$TMP/kindling.apk")
if [ "$MODE" = check ]; then
  keytool -genkeypair -keystore "$TMP/k.p12" -storetype PKCS12 -storepass session -alias s -keyalg EC -groupname secp256r1 -validity 2 -dname "CN=check" >/dev/null 2>&1
  "${SIGN[@]}" --ks "$TMP/k.p12" --ks-pass pass:session --ks-key-alias s "$TMP/aligned.apk"; KEY=session
else
  "${SIGN[@]}" --ks android/keys/throwaway.p12 --ks-pass pass:kindling-throwaway --ks-key-alias throwaway "$TMP/aligned.apk"; KEY=throwaway
fi
tools/verify-apk.sh "$TMP/kindling.apk"
if [ "$MODE" = release ]; then
  mkdir -p dist; cp "$TMP/kindling.apk" dist/kindling.apk; (cd dist && sha256sum kindling.apk > kindling.apk.sha256)
  echo "APK: dist/kindling.apk ($(( $(stat -c %s dist/kindling.apk) / 1024 )) KB, versionCode $versionCode, key $KEY)"
else echo "APK check: OK (versionCode $versionCode, key $KEY)"; fi
