// B78 shell 1: the JNI glue between Kotlin and the shared core.
use jni::objects::JClass;
use jni::sys::{jint, jlong};
use jni::JNIEnv;

#[no_mangle]
pub extern "system" fn Java_dev_kindling_shell1_Core_mix(_env: JNIEnv, _class: JClass, seed: jlong, rounds: jint) -> jlong {
    kcore::mix(seed as u64, rounds as u32) as jlong
}
