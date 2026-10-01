// B78 shell 2: the JNI glue between Kotlin and the shared C++ core.
#include <jni.h>
#include "kcore.h"

extern "C" JNIEXPORT jlong JNICALL
Java_dev_kindling_shell2_Core_mix(JNIEnv*, jclass, jlong seed, jint rounds) {
    return (jlong)kcore_mix((uint64_t)seed, (uint32_t)rounds);
}
