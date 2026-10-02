//! The `glow` context on the phone: functions from `libGLESv3.so` by `dlsym`, else `eglGetProcAddress` (A2.5).

use std::ffi::{CStr, c_void};

type GetProc = unsafe extern "C" fn(*const std::ffi::c_char) -> *const c_void;

/// Builds the context for the current EGL context; call on the GL thread.
pub fn context() -> glow::Context {
    // SAFETY: dlopen and dlsym take NUL-terminated names; handles stay open for the process's life.
    unsafe {
        let gles = libc::dlopen(c"libGLESv3.so".as_ptr(), libc::RTLD_NOW | libc::RTLD_LOCAL);
        let egl = libc::dlopen(c"libEGL.so".as_ptr(), libc::RTLD_NOW | libc::RTLD_LOCAL);
        let egl_get: Option<GetProc> = if egl.is_null() {
            None
        } else {
            let p = libc::dlsym(egl, c"eglGetProcAddress".as_ptr());
            if p.is_null() {
                None
            } else {
                Some(std::mem::transmute::<*mut c_void, GetProc>(p))
            }
        };
        glow::Context::from_loader_function_cstr(|name: &CStr| {
            let mut p: *const c_void = std::ptr::null();
            if !gles.is_null() {
                p = libc::dlsym(gles, name.as_ptr()) as *const c_void;
            }
            if p.is_null()
                && let Some(get) = egl_get
            {
                p = get(name.as_ptr());
            }
            if p.is_null() {
                log::warn!(target: "kd::android", "GL function missing: {}", name.to_string_lossy());
            }
            p
        })
    }
}
