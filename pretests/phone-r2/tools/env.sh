# Phone test app, round 2: build environment. Source it: `. tools/env.sh` (needs CACHE, see NOTES.md, How to re-run).
# Same toolchain as round 1 (pretests/b78-b79-phone/tools/setup-toolchain.sh installs it into the shared cache).
# Big downloads and caches live in the shared cache, never in the repo.
export CACHE=${CACHE:?set CACHE to the shared download cache folder first (see NOTES.md, How to re-run)}
export ANDROID_HOME=$CACHE/android-sdk
export ANDROID_SDK_ROOT=$ANDROID_HOME
export ANDROID_NDK_HOME=$(cat "$CACHE/android-ndk.ready")
export ANDROID_NDK_ROOT=$ANDROID_NDK_HOME
export GRADLE_USER_HOME=$CACHE/gradle-home
export BT=$ANDROID_HOME/build-tools/36.1.0
export LOCK=$CACHE/cpu.lock
export PATH=$BT:$ANDROID_HOME/platform-tools:$PATH
# The same throwaway pre-test key as round 1, so round 2 installs over it (b78-b79-phone/test-key/README.md).
export KINDLING_KS=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)/b78-b79-phone/test-key/kindling-pretests.jks
export KINDLING_KS_PASS=kindling-pretests
