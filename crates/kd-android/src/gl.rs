//! The GL context (A2.5): each function from `libGLESv3.so`, else through `eglGetProcAddress`. The libraries
//! stay open for the life of the process.

use std::ffi::{CStr, c_char, c_void};

type GetProc = unsafe extern "C" fn(*const c_char) -> *const c_void;

pub fn context() -> Result<glow::Context, String> {
    // SAFETY: dlopen and dlsym get valid, NUL-terminated names; eglGetProcAddress has the signature EGL defines,
    // and is called only with the NUL-terminated names glow passes.
    unsafe {
        let gles = libc::dlopen(c"libGLESv3.so".as_ptr(), libc::RTLD_NOW | libc::RTLD_LOCAL);
        let egl = libc::dlopen(c"libEGL.so".as_ptr(), libc::RTLD_NOW | libc::RTLD_LOCAL);
        if gles.is_null() && egl.is_null() {
            return Err("neither libGLESv3.so nor libEGL.so opened".to_string());
        }
        let get_proc: Option<GetProc> = if egl.is_null() {
            None
        } else {
            let p = libc::dlsym(egl, c"eglGetProcAddress".as_ptr());
            (!p.is_null()).then(|| std::mem::transmute::<*mut c_void, GetProc>(p))
        };
        Ok(glow::Context::from_loader_function_cstr(|name: &CStr| {
            let p = if gles.is_null() {
                std::ptr::null_mut()
            } else {
                libc::dlsym(gles, name.as_ptr())
            };
            if !p.is_null() {
                return p as *const c_void;
            }
            match get_proc {
                Some(f) => f(name.as_ptr()),
                None => std::ptr::null(),
            }
        }))
    }
}
