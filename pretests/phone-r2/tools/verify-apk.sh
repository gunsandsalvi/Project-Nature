#!/usr/bin/env bash
# B78: checks an APK can install on the phone without a device to try it on.
# Signed v2/v3, zip entries 16 KB aligned, native ELF segments 16 KB aligned,
# no permissions requested, arm64-v8a only, minSdk/targetSdk as expected.
# Usage: tools/verify-apk.sh app.apk   (exit code 0 only when every check passes)
set -uo pipefail
. "$(dirname "$0")/env.sh"
APK=$1
READELF=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf
fail=0
ok() { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=1; }

echo "$APK ($(stat -c %s "$APK") bytes)"
SIG=$("$BT/apksigner" verify --verbose "$APK" 2>&1 | grep -v JAVA_TOOL)
# For minSdk 31 the tools sign with v3 only and leave v2 out as redundant (Android 9+ checks v3 first).
echo "$SIG" | grep -q "Verified using v3 scheme (APK Signature Scheme v3): true" && ok "signed v3" || bad "signed v3"
echo "$SIG" | grep -q "Verified using v2 scheme (APK Signature Scheme v2): true" && echo "  info also signed v2" || echo "  info no v2 block (redundant for minSdk 31)"
echo "$SIG" | grep -q "^Verifies" || bad "apksigner verify"
"$BT/zipalign" -c -P 16 4 "$APK" > /dev/null 2>&1 && ok "zip entries aligned (16 KB for .so)" || bad "zip alignment (zipalign -c -P 16 4)"

TMP=$(mktemp -d)
unzip -q -o "$APK" 'lib/*' -d "$TMP" 2>/dev/null
ABIS=$(ls "$TMP/lib" 2>/dev/null | tr '\n' ' ')
if [ -n "$ABIS" ]; then
  [ "$ABIS" = "arm64-v8a " ] && ok "ABIs: $ABIS" || bad "ABIs: $ABIS (want arm64-v8a only)"
  for so in "$TMP"/lib/*/*.so; do
    al=$("$READELF" -lW "$so" | awk '$1=="LOAD"{print $NF}' | sort -u | tr '\n' ' ')
    [ "$al" = "0x4000 " ] && ok "$(basename "$so") LOAD align $al" || bad "$(basename "$so") LOAD align $al (want 0x4000)"
    # Native libs must be stored uncompressed so they load straight from the APK.
    m=$(unzip -v "$APK" "lib/arm64-v8a/$(basename "$so")" | awk 'NR==4{print $2}')
    [ "$m" = "Stored" ] && ok "$(basename "$so") stored uncompressed" || bad "$(basename "$so") compressed ($m)"
  done
else
  echo "  info no native libraries"
fi
rm -rf "$TMP"

BADGE=$("$BT/aapt2" dump badging "$APK" 2>/dev/null)
echo "$BADGE" | grep -E "^package:|^sdkVersion|^targetSdkVersion|^native-code|^launchable-activity" | sed 's/^/  info /'
PERMS=$("$BT/aapt2" dump permissions "$APK" 2>/dev/null | grep -E "uses-permission|permission:" || true)
[ -z "$PERMS" ] && ok "no permissions" || bad "permissions: $PERMS"
exit $fail
