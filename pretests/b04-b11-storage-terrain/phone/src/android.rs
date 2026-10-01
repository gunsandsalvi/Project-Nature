//! B04/B11: JNI export for dev.kindling.pretests.Storage, same pattern as the B78 kbench crate.
use jni::objects::{JClass, JString};
use jni::sys::jstring;
use jni::JNIEnv;

#[no_mangle]
pub extern "system" fn Java_dev_kindling_pretests_Storage_run<'l>(mut env: JNIEnv<'l>, _c: JClass<'l>, config: JString<'l>) -> jstring {
    let out = match env.get_string(&config) {
        Ok(s) => crate::run(&String::from(s)),
        Err(_) => r#"{"block":"B04-B11","error":"could not read config string"}"#.to_string(),
    };
    env.new_string(out).map(|s| s.into_raw()).unwrap_or(std::ptr::null_mut())
}
