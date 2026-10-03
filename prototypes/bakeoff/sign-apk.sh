#!/usr/bin/env bash
# Signs a bake-off APK with the game's release key (A15.5), so it installs on the owner's phone like any alpha.
# Usage: sign-apk.sh <unsigned.apk> <signed.apk>. Only tools/signing-key.py reads the passphrase; nothing is printed.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
. "$ROOT/tools/env.sh"
IN="$1"; OUT="$2"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
"$KD_BUILD_TOOLS/zipalign" -f -P 16 4 "$IN" "$TMP/aligned.apk"
python3 "$ROOT/tools/signing-key.py" pk8 "$TMP/key.pk8"
"$KD_BUILD_TOOLS/apksigner" sign --v1-signing-enabled false --v2-signing-enabled true --v3-signing-enabled true \
  --min-sdk-version 31 --key "$TMP/key.pk8" --cert "$ROOT/android/keys/release-cert.der" --out "$OUT" "$TMP/aligned.apk"
rm -f "$TMP/key.pk8"
"$KD_BUILD_TOOLS/apksigner" verify --min-sdk-version 31 "$OUT" && echo "signed: $OUT ($(($(stat -c %s "$OUT") / 1024)) KB)"
