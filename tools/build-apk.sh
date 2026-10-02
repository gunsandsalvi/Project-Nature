#!/usr/bin/env bash
# Builds, signs and checks the APK (A15.3, A15.5). Usage: tools/build-apk.sh release|check
# release signs with the release key from KINDLING_SIGNING_PASSPHRASE (only tools/signing-key.py reads it), and fails
# without it once android/keys/release-cert.der exists (the throwaway key signed only before that); check signs with a
# key made for the run and thrown away.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd); cd "$ROOT"; MODE=${1:-release}
# Once the release certificate exists, a delivered APK signed with any other key could not update the phone (A15.5).
if [ "$MODE" = release ] && [ -f android/keys/release-cert.der ] && [ -z "${KINDLING_SIGNING_PASSPHRASE:-}" ]; then
  echo "APK: release builds need KINDLING_SIGNING_PASSPHRASE (A15.5); tools/build-apk.sh check signs with a session key"
  exit 1
fi
[ -f "${KD_CACHE:-$HOME/.cache/kindling}/android-ndk.ready" ] || tools/setup.sh
. tools/env.sh; . android/version.properties
export KD_BUILD="$versionName $(git rev-parse --short HEAD)"
env -u KINDLING_SIGNING_PASSPHRASE gradle -p android --no-daemon --console=plain -q assembleRelease
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
"$BT/zipalign" -f -P 16 4 android/app/build/outputs/apk/release/app-release-unsigned.apk "$TMP/aligned.apk"
SIGN=(env -u KINDLING_SIGNING_PASSPHRASE "$BT/apksigner" sign --v1-signing-enabled false --v2-signing-enabled true --v3-signing-enabled true --min-sdk-version 31 --out "$TMP/kindling.apk")
if [ "$MODE" = check ]; then
  keytool -genkeypair -keystore "$TMP/k.p12" -storetype PKCS12 -storepass session -alias s -keyalg EC -groupname secp256r1 -validity 2 -dname "CN=check" >/dev/null 2>&1
  "${SIGN[@]}" --ks "$TMP/k.p12" --ks-pass pass:session --ks-key-alias s "$TMP/aligned.apk"; KEY=session
elif [ -n "${KINDLING_SIGNING_PASSPHRASE:-}" ]; then
  python3 tools/signing-key.py pk8 "$TMP/release.pk8"
  "${SIGN[@]}" --key "$TMP/release.pk8" --cert android/keys/release-cert.der "$TMP/aligned.apk"; KEY=release
else
  "${SIGN[@]}" --ks android/keys/throwaway.p12 --ks-pass pass:kindling-throwaway --ks-key-alias throwaway "$TMP/aligned.apk"; KEY=throwaway
fi
tools/verify-apk.sh "$TMP/kindling.apk"
if [ "$MODE" = release ]; then
  mkdir -p dist; cp "$TMP/kindling.apk" dist/kindling.apk; (cd dist && sha256sum kindling.apk > kindling.apk.sha256)
  echo "APK: dist/kindling.apk ($(( $(stat -c %s dist/kindling.apk) / 1024 )) KB, versionCode $versionCode, key $KEY)"
else echo "APK check: OK (versionCode $versionCode, key $KEY)"; fi
