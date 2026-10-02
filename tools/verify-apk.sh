#!/usr/bin/env bash
# Checks an APK can install over the last one and load its native code on the phone, without a phone (A2.5, A15.3).
# From pretests/phone-r2/tools/verify-apk.sh, cut to Kindling's app.
# Usage: tools/verify-apk.sh app.apk   (exit code 0 only when every check passes)
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/tools/env.sh"
. "$ROOT/android/version.properties"
APK=$1
TC=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
fail=0
ok() { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=1; }
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

echo "$APK ($(stat -c %s "$APK") bytes)"

# checks: PLT-06
SIG=$("$BT/apksigner" verify --verbose --print-certs "$APK" 2>&1 | grep -v JAVA_TOOL)
# apksigner leaves v2 out when minSdk is 28 or more and v3 is present (v3 covers them): see α00's Conflict note.
grep -q "Verified using v2 scheme (APK Signature Scheme v2): true" <<<"$SIG" && ok "signed v2" || echo "  info no v2 block: apksigner omits it at minSdk 31, v3 covers every supported Android"
grep -q "Verified using v3 scheme (APK Signature Scheme v3): true" <<<"$SIG" && ok "signed v3" || bad "signed v3"
grep -q "^Verifies" <<<"$SIG" || bad "apksigner verify"
CERT=$(grep -m1 "certificate SHA-256 digest" <<<"$SIG" | awk '{print $NF}')
if git -C "$ROOT" cat-file -e HEAD:dist/kindling.apk 2>/dev/null; then
  git -C "$ROOT" show HEAD:dist/kindling.apk > "$TMP/last.apk"
  LAST=$("$BT/apksigner" verify --print-certs "$TMP/last.apk" 2>&1 | grep -m1 "certificate SHA-256 digest" | awk '{print $NF}')
  if [ -n "$CERT" ] && [ "$CERT" = "$LAST" ]; then ok "same signing key as the last committed APK ($CERT)"
  else echo "  info key changed: one reinstall needed (PLT-06)"; fi
else
  echo "  info no earlier APK"
fi
"$BT/zipalign" -c -P 16 4 "$APK" > /dev/null 2>&1 && ok "zip entries aligned (16 KB for .so)" || bad "zip alignment (zipalign -c -P 16 4)"
BADGE=$("$BT/aapt2" dump badging "$APK" 2>/dev/null)
grep -q "^package: name='dev.kindling.app' versionCode='$versionCode' versionName='$versionName'" <<<"$BADGE" \
  && ok "package dev.kindling.app, versionCode $versionCode, $versionName" || bad "package/version: $(grep '^package:' <<<"$BADGE")"
grep -q "^minSdkVersion:'31'" <<<"$BADGE" && ok "minSdk 31" || bad "minSdk"
grep -q "^targetSdkVersion:'36'" <<<"$BADGE" && ok "targetSdk 36" || bad "targetSdk"

# checks: PLT-01
unzip -q -o "$APK" 'lib/*' 'classes*.dex' -d "$TMP/x" 2>/dev/null
ABIS=$(ls "$TMP/x/lib" 2>/dev/null | tr '\n' ' ')
[ "$ABIS" = "arm64-v8a " ] && ok "ABIs: $ABIS" || bad "ABIs: $ABIS (want arm64-v8a only)"
# Libraries every Android phone provides to apps (NDK stable APIs).
PUBLIC=" libc.so libm.so libdl.so liblog.so libandroid.so libz.so libEGL.so libGLESv2.so libGLESv3.so libaaudio.so libvulkan.so libnativewindow.so libOpenSLES.so libmediandk.so "
NATIVES=$(sed -n 's/.*external fun \([A-Za-z]*\)(.*/\1/p' "$ROOT/android/app/src/main/java/dev/kindling/app/Native.kt")
check_lib() { # name, JNI symbols...
  local so=$TMP/x/lib/arm64-v8a/$1; shift
  if [ ! -f "$so" ]; then bad "lib/arm64-v8a/$(basename "$so") missing"; return; fi
  local al m sym need syms
  al=$("$TC/llvm-readelf" -lW "$so" | awk '$1=="LOAD"{print $NF}' | sort -u | tr '\n' ' ')
  [ "$al" = "0x4000 " ] && ok "$(basename "$so") ($(stat -c %s "$so") bytes), LOAD align $al" || bad "$(basename "$so") LOAD align $al (want 0x4000)"
  m=$(unzip -v "$APK" "lib/arm64-v8a/$(basename "$so")" | awk 'NR==4{print $2}')
  [ "$m" = "Stored" ] && ok "stored uncompressed" || bad "compressed ($m)"
  syms=$("$TC/llvm-nm" -D --defined-only "$so")
  for sym in "$@"; do
    grep -Eq " T $sym(@@.*)?$" <<<"$syms" && ok "exports $sym" || bad "$sym not exported"
  done
  for need in $("$TC/llvm-readelf" -dW "$so" | awk '/NEEDED/{gsub(/[\[\]]/,"",$NF); print $NF}'); do
    case "$PUBLIC" in *" $need "*) ;; *) bad "needs $need, not a public Android library" ;; esac
  done
  ok "needs only public libraries ($("$TC/llvm-readelf" -dW "$so" | awk '/NEEDED/{gsub(/[\[\]]/,"",$NF); printf "%s ", $NF}'))"
}
# shellcheck disable=SC2046
check_lib libkindling.so $(for n in $NATIVES; do echo "Java_dev_kindling_app_Native_$n"; done)
EXTRA=$(ls "$TMP/x/lib/arm64-v8a" 2>/dev/null | grep -v -E "^libkindling\.so$" | tr '\n' ' ')
[ -z "$EXTRA" ] && ok "no other native libraries" || bad "other native libraries: $EXTRA"
# Names native code looks up at run time must survive R8 (here-strings, not pipes: see pipefail).
DEX=$(for d in "$TMP"/x/classes*.dex; do "$BT/dexdump" "$d" 2>/dev/null; done)
grep -Fq "Class descriptor  : 'Ldev/kindling/app/Native;'" <<<"$DEX" && ok "kept class dev.kindling.app.Native" || bad "class dev.kindling.app.Native missing or renamed"
for n in $NATIVES; do
  awk -v c="Ldev/kindling/app/Native;" -v m="$n" '/Class descriptor/{inc=($4=="\x27"c"\x27")} inc && /name *:/ && $3=="\x27"m"\x27"{f=1} END{exit !f}' <<<"$DEX" \
    && ok "kept dev.kindling.app.Native.$n" || bad "dev.kindling.app.Native.$n missing or renamed"
done
XML=$("$BT/aapt2" dump xmltree --file AndroidManifest.xml "$APK" 2>/dev/null)
grep -q 'extractNativeLibs(0x010104ea)=false' <<<"$XML" && ok "native libraries load from the APK (extractNativeLibs false)" || bad "extractNativeLibs"

# checks: PLT-02
CHANGES=$(grep -m1 'configChanges' <<<"$XML")
grep -q 'configChanges' <<<"$XML" || bad "activity configChanges missing"
# aapt2 prints configChanges as a number: orientation is 0x80, screenSize 0x400.
CC=$(sed -n 's/.*configChanges([^)]*)=\(0x[0-9a-f]*\).*/\1/p' <<<"$CHANGES")
if [ -n "$CC" ] && (( (CC & 0x80) && (CC & 0x400) )); then ok "configChanges $CC holds orientation and screenSize"
else bad "configChanges ${CC:-none} lacks orientation or screenSize"; fi
grep -q 'screenOrientation' <<<"$XML" && bad "a fixed screenOrientation is set" || ok "no fixed screenOrientation"

# checks: PLT-03
PERMS=$("$BT/aapt2" dump permissions "$APK" 2>/dev/null | grep "uses-permission" | sed "s/.*name='\([^']*\)'.*/\1/" | sort | tr '\n' ' ')
WANT=$(grep -v '^\s*$' "$ROOT/android/permissions.txt" | sort | tr '\n' ' ')
[ "$PERMS" = "$WANT" ] && ok "permissions exactly android/permissions.txt: ${PERMS:-none}" || bad "permissions: ${PERMS:-none} (want ${WANT:-none})"
grep -q "android.permission.INTERNET" <<<"$PERMS" && bad "INTERNET permission present" || ok "no INTERNET permission"
grep -q "CctBackendFactory" <<<"$XML" && bad "ML Kit usage upload registered" || ok "no ML Kit usage upload"
SIZE=$(stat -c %s "$APK")
[ "$SIZE" -le $((50 * 1024 * 1024)) ] && ok "size $((SIZE / 1024)) KB (at most 50 MB)" || bad "size $SIZE bytes, over 50 MB"
exit $fail
