//! The functions `Native.kt` declares (A2.5). The handle is a boxed `Shell`; every call but `create` and `destroy`
//! comes on the GL thread, so the app is never shared between threads. Each body runs inside `catch_unwind`, logs
//! a panic and returns a safe value (A3.8).

use std::panic::{self, AssertUnwindSafe};
use std::sync::Arc;

use jni::JNIEnv;
use jni::objects::{JClass, JFloatArray, JIntArray, JString};
use jni::sys::{jint, jlong, jstring};
use kd_app::{App, AppConfig, AppMsg};
use kd_view::Insets;

use crate::platform::AndroidPlatform;

struct Shell {
    app: App,
    platform: Arc<AndroidPlatform>,
}

fn guard<T>(what: &str, fallback: T, f: impl FnOnce() -> T) -> T {
    panic::catch_unwind(AssertUnwindSafe(f)).unwrap_or_else(|_| {
        log::error!(target: "kd::android", "{what} panicked; carrying on");
        fallback
    })
}

/// The shell behind a handle from `create`.
fn shell<'a>(h: jlong) -> Option<&'a mut Shell> {
    // SAFETY: Kotlin passes only the handle `create` returned, and calls nothing with it after `destroy`;
    // all calls but those two come on the GL thread, one at a time.
    (h != 0).then(|| unsafe { &mut *(h as *mut Shell) })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_create(
    mut env: JNIEnv,
    _: JClass,
    _files_dir: JString,
    _cache_dir: JString,
    device: JString,
) -> jlong {
    guard("create", 0, || {
        crate::logcat::init();
        let device: String = env.get_string(&device).map(Into::into).unwrap_or_default();
        log::info!(target: "kd::android", "{} on {device}", kd_app::build_line());
        let platform = Arc::new(AndroidPlatform::default());
        let app = App::new(platform.clone(), AppConfig { device });
        Box::into_raw(Box::new(Shell { app, platform })) as jlong
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_destroy(_: JNIEnv, _: JClass, h: jlong) {
    guard("destroy", (), || {
        if h != 0 {
            // SAFETY: the handle came from Box::into_raw in `create`, and Kotlin calls destroy once, last.
            drop(unsafe { Box::from_raw(h as *mut Shell) });
        }
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_onResume(_: JNIEnv, _: JClass, h: jlong) {
    guard("onResume", (), || {
        if let Some(s) = shell(h) {
            s.app.handle(AppMsg::Resume);
        }
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_onPause(_: JNIEnv, _: JClass, h: jlong) {
    guard("onPause", (), || {
        if let Some(s) = shell(h) {
            s.app.handle(AppMsg::Pause);
        }
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_back(_: JNIEnv, _: JClass, h: jlong) {
    guard("back", (), || {
        if let Some(s) = shell(h) {
            s.app.handle(AppMsg::Back);
        }
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_glCreated(_: JNIEnv, _: JClass, h: jlong) {
    guard("glCreated", (), || {
        if let Some(s) = shell(h) {
            match crate::gl::context() {
                Ok(gl) => s.app.gl_ready(gl),
                Err(e) => s.app.gl_failed(e),
            }
        }
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_glResized(
    _: JNIEnv,
    _: JClass,
    h: jlong,
    width: jint,
    height: jint,
) {
    guard("glResized", (), || {
        if let Some(s) = shell(h) {
            s.app.resize(width.max(0) as u32, height.max(0) as u32);
        }
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_glDraw(_: JNIEnv, _: JClass, h: jlong, frame_nanos: jlong) -> jint {
    guard("glDraw", 0, || match shell(h) {
        Some(s) => jint::from(s.app.frame(frame_nanos.max(0) as u64)),
        None => 0,
    })
}

#[allow(clippy::too_many_arguments)]
#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_touch(
    env: JNIEnv,
    _: JClass,
    h: jlong,
    action: jint,
    index: jint,
    ids: JIntArray,
    xs: JFloatArray,
    ys: JFloatArray,
    time_nanos: jlong,
) {
    guard("touch", (), || {
        let Some(s) = shell(h) else { return };
        let n = env.get_array_length(&ids).unwrap_or(0).max(0) as usize;
        let (mut id, mut x, mut y) = (vec![0; n], vec![0.0; n], vec![0.0; n]);
        if env.get_int_array_region(&ids, 0, &mut id).is_err()
            || env.get_float_array_region(&xs, 0, &mut x).is_err()
            || env.get_float_array_region(&ys, 0, &mut y).is_err()
        {
            return;
        }
        for e in crate::touch::events(action, index, &id, &x, &y, time_nanos.max(0) as u64) {
            s.app.handle(AppMsg::Input(e));
        }
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_insets(
    _: JNIEnv,
    _: JClass,
    h: jlong,
    left: jint,
    top: jint,
    right: jint,
    bottom: jint,
) {
    guard("insets", (), || {
        if let Some(s) = shell(h) {
            let u = |v: jint| v.max(0) as u32;
            s.app.handle(AppMsg::Insets(Insets {
                left: u(left),
                top: u(top),
                right: u(right),
                bottom: u(bottom),
            }));
        }
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_takeRequests(env: JNIEnv, _: JClass, h: jlong) -> jstring {
    guard("takeRequests", std::ptr::null_mut(), || {
        let Some(s) = shell(h) else { return std::ptr::null_mut() };
        let requests = s.platform.take();
        if requests.is_empty() {
            return std::ptr::null_mut();
        }
        env.new_string(kd_app::requests_json(&requests))
            .map(|j| j.into_raw())
            .unwrap_or(std::ptr::null_mut())
    })
}
