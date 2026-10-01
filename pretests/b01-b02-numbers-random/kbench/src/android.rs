//! JNI exports for the phone test app (B78): class dev.kindling.pretests.Bench
//!   static native String run(String configJson);
//!   static native boolean pinToCpu(int cpu);
use jni::objects::{JClass, JString};
use jni::sys::{jboolean, jint, jstring, JNI_FALSE, JNI_TRUE};
use jni::JNIEnv;

#[no_mangle]
pub extern "system" fn Java_dev_kindling_pretests_Bench_run<'local>(
    mut env: JNIEnv<'local>,
    _class: JClass<'local>,
    config: JString<'local>,
) -> jstring {
    let cfg: String = match env.get_string(&config) {
        Ok(s) => s.into(),
        Err(_) => r#"{"error":"could not read config string"}"#.to_string(),
    };
    let out = if cfg.starts_with(r#"{"error""#) { cfg } else { crate::run(&cfg) };
    match env.new_string(out) {
        Ok(s) => s.into_raw(),
        Err(_) => std::ptr::null_mut(),
    }
}

/// Pins the calling thread to one CPU. Worker threads started by `run` inherit the
/// caller's CPU set; the config key "cpus" pins each worker separately.
#[no_mangle]
pub extern "system" fn Java_dev_kindling_pretests_Bench_pinToCpu<'local>(
    _env: JNIEnv<'local>,
    _class: JClass<'local>,
    cpu: jint,
) -> jboolean {
    if cpu >= 0 && crate::pin_current_thread(cpu as usize) {
        JNI_TRUE
    } else {
        JNI_FALSE
    }
}
