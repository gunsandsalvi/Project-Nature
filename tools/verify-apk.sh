#!/usr/bin/env bash
# Checks an APK before it is delivered (A2.5, A15.3). Usage: tools/verify-apk.sh <apk> release|check
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/tools/env.sh"
APK="$1"
MODE="${2:-release}"
. "$ROOT/android/version.properties"
AAPT2="$KD_BUILD_TOOLS/aapt2"
NDK_BIN="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
fail() { echo "APK verify: FAILED: $*" >&2; exit 1; }
ok() { echo "  ok  $*"; }

# checks: PLT-06
# One key, scheme v3 alone (A15.3, A15.5); a release build carries the registered release certificate.
SIGS="$("$KD_BUILD_TOOLS/apksigner" verify --verbose --print-certs "$APK")" || fail "apksigner verify"
grep -q 'Verified using v3 scheme (APK Signature Scheme v3): true' <<<"$SIGS" || fail "no v3 signature"
if [ "$MODE" = release ]; then
  GOT="$(sed -n 's/^Signer #1 certificate SHA-256 digest: //p' <<<"$SIGS")"
  WANT="$(cut -d' ' -f1 "$ROOT/android/keys/release-cert.sha256")"
  [ "$GOT" = "$WANT" ] || fail "signed with $GOT, not the release certificate $WANT"
fi
ok "signed with scheme v3, the $MODE key"
BADGING="$("$AAPT2" dump badging "$APK")"
grep -q "package: name='dev.kindling.app' versionCode='$versionCode' versionName='$versionName'" <<<"$BADGING" \
  || fail "package or version differs from android/version.properties"
grep -q "minSdkVersion:'31'" <<<"$BADGING" || fail "minSdk is not 31"
grep -q "targetSdkVersion:'36'" <<<"$BADGING" || fail "targetSdk is not 36"
ok "dev.kindling.app $versionName ($versionCode), minSdk 31, targetSdk 36"

# checks: PLT-01
# arm64 only; libkindling.so stored uncompressed and 16 KB aligned, in the zip and in its loaded segments;
# every JNI name Native.kt declares exported, and the class kept by R8 (A2.5).
LIBS="$(unzip -Z1 "$APK" 'lib/*' 2>/dev/null | tr '\n' ' ')"
[ "$LIBS" = "lib/arm64-v8a/libkindling.so " ] || fail "native libraries: $LIBS"
unzip -Zv "$APK" lib/arm64-v8a/libkindling.so | grep -Eq 'compression method:[[:space:]]+none \(stored\)' \
  || fail "libkindling.so is compressed"
"$KD_BUILD_TOOLS/zipalign" -c -P 16 4 "$APK" >/dev/null || fail "zip entries are not 16 KB aligned"
unzip -p "$APK" lib/arm64-v8a/libkindling.so >"$TMP/lib.so"
for a in $("$NDK_BIN/llvm-readelf" -lW "$TMP/lib.so" | awk '$1 == "LOAD" { print $NF }'); do
  [ $((a)) -ge 16384 ] || fail "a loaded segment of libkindling.so is aligned to $a, under 16 KB"
done
"$NDK_BIN/llvm-nm" -D --defined-only "$TMP/lib.so" >"$TMP/symbols"
unzip -p "$APK" classes.dex >"$TMP/classes.dex"
"$KD_BUILD_TOOLS/dexdump" "$TMP/classes.dex" >"$TMP/dex" 2>/dev/null
grep -q "Class descriptor  : 'Ldev/kindling/app/Native;'" "$TMP/dex" || fail "R8 removed dev.kindling.app.Native"
FUNS="$(sed -n 's/.*external fun \([A-Za-z]*\).*/\1/p' "$ROOT/android/app/src/main/java/dev/kindling/app/Native.kt")"
for f in $FUNS; do
  grep -q " Java_dev_kindling_app_Native_$f\$" "$TMP/symbols" || fail "libkindling.so does not export $f"
  grep -q "name          : '$f'" "$TMP/dex" || fail "R8 removed Native.$f"
done
ok "arm64 only, stored, 16 KB aligned; $(wc -w <<<"$FUNS") JNI names exported and kept"

# checks: PLT-02
# Turning the phone never restarts the activity: configChanges holds orientation and screenSize, and nothing
# fixes the orientation.
MANIFEST="$("$AAPT2" dump xmltree --file AndroidManifest.xml "$APK")"
CFG="$(grep -o 'configChanges([^)]*)=0x[0-9a-fA-F]*' <<<"$MANIFEST" | sed 's/.*=//')"
[ -n "$CFG" ] && (((CFG & 0x480) == 0x480)) || fail "configChanges lacks orientation or screenSize"
if grep -q 'screenOrientation' <<<"$MANIFEST"; then fail "the activity fixes its orientation"; fi
ok "rotation handled in place (configChanges $CFG)"

# checks: PLT-03
# Exactly the permissions in android/permissions.txt, and never INTERNET (A2.5).
GOT="$("$AAPT2" dump permissions "$APK" | sed -n "s/^uses-permission: name='\([^']*\)'.*/\1/p" | sort)"
WANT="$(grep -v '^[[:space:]]*$' "$ROOT/android/permissions.txt" | sort || true)"
[ "$GOT" = "$WANT" ] || fail "permissions [$GOT] differ from android/permissions.txt [$WANT]"
if grep -qx 'android.permission.INTERNET' <<<"$GOT"; then fail "the APK asks for INTERNET"; fi
ok "permissions exactly android/permissions.txt (${GOT:-none})"

SIZE="$(stat -c %s "$APK")"
[ "$SIZE" -le $((50 * 1024 * 1024)) ] || fail "$((SIZE / 1024)) KB, over 50 MB"
ok "$((SIZE / 1024)) KB, at most 50 MB"
echo "APK verify: OK"
