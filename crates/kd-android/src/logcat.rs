//! A `log` logger writing to logcat (tag `kindling`) and a panic hook that logs (A3.8).

use std::ffi::{CString, c_char, c_int};

#[link(name = "log")]
unsafe extern "C" {
    fn __android_log_write(prio: c_int, tag: *const c_char, text: *const c_char) -> c_int;
}

const TAG: &[u8] = b"kindling\0";

fn write(prio: c_int, text: &str) {
    let text = CString::new(text.replace('\0', " ")).unwrap_or_default();
    // SAFETY: both pointers are valid NUL-terminated strings for the call.
    unsafe {
        __android_log_write(prio, TAG.as_ptr() as *const c_char, text.as_ptr());
    }
}

struct Logcat;

impl log::Log for Logcat {
    fn enabled(&self, m: &log::Metadata) -> bool {
        m.level() <= log::Level::Info
    }
    fn log(&self, r: &log::Record) {
        if self.enabled(r.metadata()) {
            let prio = match r.level() {
                log::Level::Error => 6,
                log::Level::Warn => 5,
                log::Level::Info => 4,
                log::Level::Debug => 3,
                log::Level::Trace => 2,
            };
            write(prio, &format!("{}: {}", r.target(), r.args()));
        }
    }
    fn flush(&self) {}
}

static LOGGER: Logcat = Logcat;

/// Installs the logger and the panic hook, once.
pub fn init() {
    if log::set_logger(&LOGGER).is_ok() {
        log::set_max_level(log::LevelFilter::Info);
        std::panic::set_hook(Box::new(|info| write(6, &format!("panic: {info}"))));
    }
}
