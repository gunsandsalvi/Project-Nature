//! The JNI entry points of `dev.kindling.app.Native` (A2.5). Every body runs inside `catch_unwind`, logging a panic
//! and returning a safe value (A3.8). The handle is a boxed `Shell` passed to Kotlin as a `jlong`.

use crate::platform::AndroidPlatform;
use jni::JNIEnv;
use jni::objects::{JClass, JFloatArray, JIntArray, JString};
use jni::sys::{jint, jlong, jstring};
use kd_app::{App, AppConfig, AppMsg, InputEvent, InputKind};
use std::panic::{AssertUnwindSafe, catch_unwind};
use std::sync::Arc;

/// What Kotlin's handle points to: the app and the platform whose outbox `takeRequests` drains.
struct Shell {
    app: App,
    platform: Arc<AndroidPlatform>,
}

fn guard<R>(name: &str, fallback: R, f: impl FnOnce() -> R) -> R {
    match catch_unwind(AssertUnwindSafe(f)) {
        Ok(r) => r,
        Err(_) => {
            log::error!(target: "kd::android", "panic in {name}");
            fallback
        }
    }
}

fn with_shell<R>(h: jlong, name: &str, fallback: R, f: impl FnOnce(&mut Shell) -> R) -> R {
    if h == 0 {
        return fallback;
    }
    guard(name, fallback, || {
        // SAFETY: h came from Box::into_raw in `create` and is used only on the GL thread until `destroy`.
        let shell = unsafe { &mut *(h as *mut Shell) };
        f(shell)
    })
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
        let device: String = env.get_string(&device).map(|s| s.into()).unwrap_or_default();
        let platform = Arc::new(AndroidPlatform::default());
        let app = App::new(platform.clone(), AppConfig { device });
        log::info!(target: "kd::android", "Kindling {} started", kd_app::build_line());
        Box::into_raw(Box::new(Shell { app, platform })) as jlong
    })
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_destroy(_: JNIEnv, _: JClass, h: jlong) {
    if h != 0 {
        guard("destroy", (), || {
            // SAFETY: h came from Box::into_raw in `create`; Kotlin calls destroy once, after the GL thread ended.
            drop(unsafe { Box::from_raw(h as *mut Shell) });
        });
    }
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_onResume(_: JNIEnv, _: JClass, h: jlong) {
    with_shell(h, "onResume", (), |s| s.app.handle(AppMsg::Resume));
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_onPause(_: JNIEnv, _: JClass, h: jlong) {
    with_shell(h, "onPause", (), |s| s.app.handle(AppMsg::Pause));
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_back(_: JNIEnv, _: JClass, h: jlong) {
    with_shell(h, "back", (), |s| s.app.handle(AppMsg::Back));
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_glCreated(_: JNIEnv, _: JClass, h: jlong) {
    with_shell(h, "glCreated", (), |s| s.app.gl_ready(crate::gl::context()));
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_glResized(_: JNIEnv, _: JClass, h: jlong, w: jint, ht: jint) {
    with_shell(h, "glResized", (), |s| s.app.resize(w.max(1) as u32, ht.max(1) as u32));
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_glDraw(_: JNIEnv, _: JClass, h: jlong, frame_nanos: jlong) -> jint {
    with_shell(h, "glDraw", 0, |s| s.app.frame(frame_nanos.max(0) as u64) as jint)
}

// MotionEvent's masked actions.
const ACTION_DOWN: jint = 0;
const ACTION_UP: jint = 1;
const ACTION_MOVE: jint = 2;
const ACTION_CANCEL: jint = 3;
const ACTION_POINTER_DOWN: jint = 5;
const ACTION_POINTER_UP: jint = 6;

#[unsafe(no_mangle)]
#[allow(clippy::too_many_arguments)]
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
    with_shell(h, "touch", (), |s| {
        let n = env.get_array_length(&ids).unwrap_or(0).max(0) as usize;
        let (mut id, mut x, mut y) = (vec![0; n], vec![0.0; n], vec![0.0; n]);
        if env.get_int_array_region(&ids, 0, &mut id).is_err()
            || env.get_float_array_region(&xs, 0, &mut x).is_err()
            || env.get_float_array_region(&ys, 0, &mut y).is_err()
        {
            return;
        }
        let t_ns = time_nanos.max(0) as u64;
        let mut send = |kind, i: usize| {
            if i < n {
                s.app.handle(AppMsg::Input(InputEvent {
                    kind,
                    pointer: id[i],
                    x: x[i],
                    y: y[i],
                    t_ns,
                }));
            }
        };
        let i = index.max(0) as usize;
        match action {
            ACTION_DOWN | ACTION_POINTER_DOWN => send(InputKind::Down, i),
            ACTION_UP | ACTION_POINTER_UP => send(InputKind::Up, i),
            ACTION_MOVE => (0..n).for_each(|k| send(InputKind::Move, k)),
            ACTION_CANCEL => (0..n).for_each(|k| send(InputKind::Cancel, k)),
            _ => {}
        }
    });
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_dev_kindling_app_Native_takeRequests(env: JNIEnv, _: JClass, h: jlong) -> jstring {
    with_shell(h, "takeRequests", std::ptr::null_mut(), |s| {
        match s.platform.take_json() {
            Some(json) => env
                .new_string(json)
                .map(|j| j.into_raw())
                .unwrap_or(std::ptr::null_mut()),
            None => std::ptr::null_mut(),
        }
    })
}
