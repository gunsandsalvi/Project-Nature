#!/usr/bin/env bash
# The APK check (PLT-06, PRC-11, A2.3): an APK is the step's, signed, for your phone alone, and offline.
# Usage: tools/verify-apk.sh <apk> release|check <step>
#   release  it must carry the release certificate registered for dev.kindling.app (android/keys/)
#   check    any key, as a check build has one made for it
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/tools/env.sh"
APK="$1"
MODE="$2"
VERSION="$("$ROOT/tools/version.sh" "$3")"
read -r CODE NAME <<<"$VERSION"
AAPT2="$KD_BUILD_TOOLS/aapt2"
NDK_BIN="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
fail() { echo "APK check: FAILED: $*" >&2; exit 1; }
ok() { echo "   ok  $*"; }

# checks: PLT-06
# Signed by the scheme v2 or v3; a release carries the registered release certificate, so it installs over the
# last alpha; and it is the step's: package dev.kindling.app, the step's version code and name.
SIGS="$("$KD_BUILD_TOOLS/apksigner" verify --verbose --print-certs "$APK" 2>&1)" || fail "apksigner: $SIGS"
grep -q 'Verified using v2 scheme (APK Signature Scheme v2): true' <<<"$SIGS" || fail "no v2 signature"
grep -q 'Verified using v3 scheme (APK Signature Scheme v3): true' <<<"$SIGS" || fail "no v3 signature"
if [ "$MODE" = release ]; then
  GOT="$(sed -n 's/^Signer #1 certificate SHA-256 digest: //p' <<<"$SIGS")"
  WANT="$(cut -d' ' -f1 "$ROOT/android/keys/release-cert.sha256")"
  [ "$GOT" = "$WANT" ] || fail "signed with $GOT, not the release certificate $WANT"
fi
ok "signed (scheme v2 and v3) with the $MODE key"
BADGING="$("$AAPT2" dump badging "$APK" 2>/dev/null)"
grep -q "^package: name='dev.kindling.app' versionCode='$CODE' versionName='$NAME'" <<<"$BADGING" \
  || fail "not dev.kindling.app $NAME ($CODE): $(head -1 <<<"$BADGING")"
MIN="$(sed -n "s/^minSdkVersion:'\([0-9]*\)'/\1/p" <<<"$BADGING")"
TARGET="$(sed -n "s/^targetSdkVersion:'\([0-9]*\)'/\1/p" <<<"$BADGING")"
ok "dev.kindling.app $NAME ($CODE), Android API $MIN to $TARGET"

# checks: PLT-01
# For your phone alone: native code for arm64 only, with its loaded segments aligned to 16 KB pages, and the zip
# aligned for them.
LIBS="$(unzip -Z1 "$APK" 'lib/*' 2>/dev/null || true)"
[ -n "$LIBS" ] || fail "no native libraries"
OTHER="$(grep -v '^lib/arm64-v8a/[^/]*\.so$' <<<"$LIBS" || true)"
[ -z "$OTHER" ] || fail "native code for other processors: $(tr '\n' ' ' <<<"$OTHER")"
"$KD_BUILD_TOOLS/zipalign" -c -P 16 4 "$APK" >/dev/null || fail "the zip is not aligned for 16 KB pages"
for lib in $LIBS; do
  unzip -p "$APK" "$lib" >"$TMP/lib.so"
  for a in $("$NDK_BIN/llvm-readelf" -lW "$TMP/lib.so" | awk '$1 == "LOAD" { print $NF }'); do
    [ $((a)) -ge 16384 ] || fail "$lib has a loaded segment aligned to $a, under 16 KB"
  done
done
ok "arm64 only ($(wc -l <<<"$LIBS") libraries), aligned for 16 KB pages"

# checks: PLT-02
# Turning the phone turns the screen: the activity follows the sensor as your rotation lock allows (fullUser, 13),
# and handles the turn itself (configChanges holds orientation and screenSize).
MANIFEST="$("$AAPT2" dump xmltree --file AndroidManifest.xml "$APK" 2>/dev/null)"
ORIENT="$(grep -o 'screenOrientation([^)]*)=[-0-9]*' <<<"$MANIFEST" | sed 's/.*=//' | head -1)"
[ "$ORIENT" = 13 ] || fail "the screen does not follow the phone as the rotation lock allows (screenOrientation ${ORIENT:-unset})"
CFG="$(grep -o 'configChanges([^)]*)=0x[0-9a-fA-F]*' <<<"$MANIFEST" | sed 's/.*=//' | head -1)"
[ -n "$CFG" ] && (((CFG & 0x480) == 0x480)) || fail "configChanges lacks orientation or screenSize"
ok "follows the phone's turning (screenOrientation $ORIENT, configChanges $CFG)"

# checks: PLT-03
# Offline: it asks for no permission at all, so never for the network; a step that needs one names it here.
ALLOWED=""
PERMS="$("$AAPT2" dump permissions "$APK" 2>/dev/null | sed -n "s/^uses-permission: name='\([^']*\)'.*/\1/p" | sort)"
if grep -qx 'android.permission.INTERNET' <<<"$PERMS"; then fail "it asks for the network (INTERNET)"; fi
for p in $PERMS; do grep -qwx "$p" <<<"$ALLOWED" || fail "it asks for $p, which no step needs"; done
ok "no permissions"

SIZE="$(stat -c %s "$APK")"
[ "$SIZE" -le $((50 * 1024 * 1024)) ] || fail "$((SIZE / 1024)) KB, over the 50 MB a committed file may have"
ok "$((SIZE / 1024 / 1024)) MB, within 50 MB"

# checks: PLT-06 PRC-11 PRE-31
# Fixture deliveries need their raw metadata and design sheets, plus every imported atlas channel.
# The validator leaves earlier deliveries alone; material records begin with the terrain delivery.
python3 "$ROOT/tools/apk-fixtures.py" "$APK" "$CODE" || fail "packaged fixture resources"
echo "APK check: OK"
