#!/usr/bin/env bash
# B78 shell 3: build and package without Gradle: cargo-ndk, aapt2, zipalign (16 KB), apksigner.
set -euo pipefail
cd "$(dirname "$0")"
. ../../tools/env.sh
OUT=build
rm -rf "$OUT/apk" && mkdir -p "$OUT/apk/lib/arm64-v8a"
cargo ndk -t arm64-v8a -P 31 build --release
cp target/aarch64-linux-android/release/libmain.so "$OUT/apk/lib/arm64-v8a/"
"$BT/aapt2" link -o "$OUT/base.apk" -I "$ANDROID_HOME/platforms/android-36/android.jar" --manifest AndroidManifest.xml
(cd "$OUT/apk" && zip -q -0 -r ../base.apk lib)
"$BT/zipalign" -f -P 16 4 "$OUT/base.apk" "$OUT/aligned.apk"
"$BT/apksigner" sign --ks "$KINDLING_KS" --ks-pass "pass:$KINDLING_KS_PASS" --ks-key-alias pretests \
  --out "$OUT/shell3.apk" "$OUT/aligned.apk"
echo "built $OUT/shell3.apk"
