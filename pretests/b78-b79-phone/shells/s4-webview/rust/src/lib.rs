// B78 shell 4: the JNI glue between the Kotlin bridge and the shared core.
use jni::objects::JClass;
use jni::sys::{jint, jlong};
use jni::JNIEnv;

#[no_mangle]
pub extern "system" fn Java_dev_kindling_shell4_Core_mix(_env: JNIEnv, _class: JClass, seed: jlong, rounds: jint) -> jlong {
    kcore::mix(seed as u64, rounds as u32) as jlong
}
