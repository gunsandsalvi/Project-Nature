//! B78: JNI exports for dev.kindling.pretests.Bench, same as the real kbench crate.
use jni::objects::{JClass, JString};
use jni::sys::{jboolean, jint, jstring, JNI_FALSE, JNI_TRUE};
use jni::JNIEnv;

#[no_mangle]
pub extern "system" fn Java_dev_kindling_pretests_Bench_run<'l>(mut env: JNIEnv<'l>, _c: JClass<'l>, config: JString<'l>) -> jstring {
    let out = match env.get_string(&config) {
        Ok(s) => crate::run(&String::from(s)),
        Err(_) => r#"{"error":"could not read config string"}"#.to_string(),
    };
    env.new_string(out).map(|s| s.into_raw()).unwrap_or(std::ptr::null_mut())
}

#[no_mangle]
pub extern "system" fn Java_dev_kindling_pretests_Bench_pinToCpu<'l>(_env: JNIEnv<'l>, _c: JClass<'l>, cpu: jint) -> jboolean {
    if cpu >= 0 && crate::pin_current_thread(cpu as usize) { JNI_TRUE } else { JNI_FALSE }
}
