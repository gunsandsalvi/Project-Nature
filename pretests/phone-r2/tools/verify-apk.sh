#!/usr/bin/env bash
# Round 2: checks an APK can install and load its native code on the phone, without a phone to try it on.
# - signed (v3) with the same key as round 1, so it installs over it; zip entries 16 KB aligned;
# - arm64-v8a only; every native library each test needs is there, stored uncompressed, 16 KB aligned,
#   exports its JNI entry points, and needs only libraries every Android phone has;
# - the manifest: package, version, SDK levels, permissions, the vendor libraries Gemma may open,
#   the share provider, and no ML Kit usage upload;
# - R8 kept every class and method that native code looks up by name.
# Usage: tools/verify-apk.sh app.apk [round-1.apk]   (exit code 0 only when every check passes)
set -uo pipefail
. "$(dirname "$0")/env.sh"
APK=$1
R1=${2:-$(dirname "$0")/../../b78-b79-phone/dist/kindling-pretests-r1.apk}
TC=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
fail=0
ok() { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=1; }

echo "$APK ($(stat -c %s "$APK") bytes)"
SIG=$("$BT/apksigner" verify --verbose --print-certs "$APK" 2>&1 | grep -v JAVA_TOOL)
grep -q "Verified using v3 scheme (APK Signature Scheme v3): true" <<<"$SIG" && ok "signed v3" || bad "signed v3"
grep -q "^Verifies" <<<"$SIG" || bad "apksigner verify"
CERT=$(echo "$SIG" | grep -m1 "certificate SHA-256 digest" | awk '{print $NF}')
if [ -f "$R1" ]; then
  CERT1=$("$BT/apksigner" verify --print-certs "$R1" 2>&1 | grep -m1 "certificate SHA-256 digest" | awk '{print $NF}')
  [ -n "$CERT" ] && [ "$CERT" = "$CERT1" ] && ok "same signing key as round 1 ($CERT)" || bad "signing key $CERT differs from round 1's $CERT1"
else
  echo "  info round-1 APK not found; key not compared"
fi
"$BT/zipalign" -c -P 16 4 "$APK" > /dev/null 2>&1 && ok "zip entries aligned (16 KB for .so)" || bad "zip alignment (zipalign -c -P 16 4)"

TMP=$(mktemp -d)
unzip -q -o "$APK" 'lib/*' 'classes*.dex' -d "$TMP" 2>/dev/null
ABIS=$(ls "$TMP/lib" 2>/dev/null | tr '\n' ' ')
[ "$ABIS" = "arm64-v8a " ] && ok "ABIs: $ABIS" || bad "ABIs: $ABIS (want arm64-v8a only)"
# Libraries every Android phone provides to apps (NDK stable APIs).
PUBLIC=" libc.so libm.so libdl.so liblog.so libandroid.so libz.so libEGL.so libGLESv2.so libGLESv3.so libaaudio.so libvulkan.so libnativewindow.so libOpenSLES.so libmediandk.so "
check_lib() { # name, piece, JNI symbols...
  local so=$TMP/lib/arm64-v8a/$1 piece=$2; shift 2
  if [ ! -f "$so" ]; then bad "$piece: lib/arm64-v8a/$(basename "$so") missing"; return; fi
  local al m sym need
  al=$("$TC/llvm-readelf" -lW "$so" | awk '$1=="LOAD"{print $NF}' | sort -u | tr '\n' ' ')
  [ "$al" = "0x4000 " ] && ok "$piece: $(basename "$so") ($(stat -c %s "$so") bytes), LOAD align $al" || bad "$piece: $(basename "$so") LOAD align $al (want 0x4000)"
  m=$(unzip -v "$APK" "lib/arm64-v8a/$(basename "$so")" | awk 'NR==4{print $2}')
  [ "$m" = "Stored" ] && ok "$piece: stored uncompressed" || bad "$piece: compressed ($m)"
  local syms
  syms=$("$TC/llvm-nm" -D --defined-only "$so")
  for sym in "$@"; do
    grep -Eq " T $sym(@@.*)?$" <<<"$syms" && ok "$piece: exports $sym" || bad "$piece: $sym not exported"
  done
  for need in $("$TC/llvm-readelf" -dW "$so" | awk '/NEEDED/{gsub(/[\[\]]/,"",$NF); print $NF}'); do
    case "$PUBLIC" in *" $need "*) ;; *) bad "$piece: needs $need, not a public Android library" ;; esac
  done
  ok "$piece: needs only public libraries ($("$TC/llvm-readelf" -dW "$so" | awk '/NEEDED/{gsub(/[\[\]]/,"",$NF); printf "%s ", $NF}'))"
}
check_lib libkstorage.so "B04/B11 storage" Java_dev_kindling_pretests_Storage_run
check_lib libksound.so "B74 sound" Java_dev_kindling_pretests_Sound_run
check_lib liblitertlm_jni.so "B73 Gemma (LiteRT-LM)" Java_com_google_ai_edge_litertlm_LiteRtLmJni_nativeCreateEngine \
  Java_com_google_ai_edge_litertlm_LiteRtLmJni_nativeCreateConversation Java_com_google_ai_edge_litertlm_LiteRtLmJni_nativeSendMessageAsync
EXTRA=$(ls "$TMP/lib/arm64-v8a" | grep -v -E "^(libkstorage|libksound|liblitertlm_jni)\.so$" | tr '\n' ' ')
[ -z "$EXTRA" ] && ok "no other native libraries" || echo "  info other native libraries: $EXTRA"

# Names native code looks up at run time must survive R8 (class.method, as dexdump prints them).
DEX=$(for d in "$TMP"/classes*.dex; do "$BT/dexdump" "$d" 2>/dev/null; done)
# (Here-strings, not pipes: with pipefail, "grep -q" closing a pipe early would read as a failure.)
need_class() { grep -Fq "Class descriptor  : 'L$1;'" <<<"$DEX" && ok "kept class ${1//\//.}" || bad "class ${1//\//.} missing or renamed"; }
need_method() { # class, method
  awk -v c="L$1;" -v m="$2" '/Class descriptor/{inc=($4=="\x27"c"\x27")} inc && /name *:/ && $3=="\x27"m"\x27"{f=1} END{exit !f}' <<<"$DEX" \
    && ok "kept ${1//\//.}.$2" || bad "${1//\//.}.$2 missing or renamed"
}
need_class dev/kindling/pretests/Storage; need_method dev/kindling/pretests/Storage run
need_class dev/kindling/pretests/Sound; need_method dev/kindling/pretests/Sound run
need_class dev/kindling/pretests/WriterTest
need_class com/google/mlkit/genai/prompt/Generation
need_class com/google/ai/edge/litertlm/LiteRtLmJni; need_method com/google/ai/edge/litertlm/LiteRtLmJni nativeCreateEngine
need_class com/google/ai/edge/litertlm/LiteRtLmJniException
for m in getTopK getTopP getTemperature getSeed; do need_method com/google/ai/edge/litertlm/SamplerConfig $m; done
for m in getEnableThinking getThinkingTokenBudget; do need_method com/google/ai/edge/litertlm/ThinkingConfig $m; done
for m in onMessage onDone onError; do need_method 'com/google/ai/edge/litertlm/LiteRtLmJni$JniMessageCallback' $m; done
need_method 'com/google/ai/edge/litertlm/InputData$Text' getText
need_class com/google/ai/edge/litertlm/BenchmarkInfo
rm -rf "$TMP"

BADGE=$("$BT/aapt2" dump badging "$APK" 2>/dev/null)
grep -q "^package: name='dev.kindling.pretests' versionCode='2' versionName='r2'" <<<"$BADGE" && ok "package dev.kindling.pretests, versionCode 2, r2" || bad "package/version: $(echo "$BADGE" | grep '^package:')"
grep -q "^minSdkVersion:'31'" <<<"$BADGE" && ok "minSdk 31" || bad "minSdk"
grep -q "^targetSdkVersion:'36'" <<<"$BADGE" && ok "targetSdk 36" || bad "targetSdk"
echo "$BADGE" | grep -E "^launchable-activity|^native-code" | sed 's/^/  info /'
PERMS=$("$BT/aapt2" dump permissions "$APK" 2>/dev/null | grep "uses-permission" | sed "s/.*name='\([^']*\)'.*/\1/" | sort | tr '\n' ' ')
WANT="android.permission.ACCESS_NETWORK_STATE android.permission.INTERNET com.google.android.apps.aicore.service.BIND_SERVICE dev.kindling.pretests.DYNAMIC_RECEIVER_NOT_EXPORTED_PERMISSION "
[ "$PERMS" = "$WANT" ] && ok "permissions: $PERMS" || bad "permissions: $PERMS (want $WANT)"
XML=$("$BT/aapt2" dump xmltree --file AndroidManifest.xml "$APK" 2>/dev/null)
for lib in libOpenCL.so libOpenCL-pixel.so libOpenCL-car.so libvndksupport.so; do
  grep -q "\"$lib\"" <<<"$XML" && ok "may open $lib" || bad "uses-native-library $lib missing"
done
# this round runs Gemma on the GPU, then the CPU; the Tensor AI unit's libraries are not declared
grep -q 'libedgetpu' <<<"$XML" && bad "a Tensor AI-unit library is still declared" || ok "no Tensor AI-unit libraries declared"
grep -q '"dev.kindling.pretests.files"' <<<"$XML" && ok "share provider dev.kindling.pretests.files" || bad "FileProvider missing"
grep -q "CctBackendFactory" <<<"$XML" && bad "ML Kit usage upload still registered" || ok "ML Kit usage upload removed (no network use but the model download)"
grep -q 'extractNativeLibs(0x010104ea)=false' <<<"$XML" && ok "native libraries load from the APK" || bad "extractNativeLibs"
exit $fail
