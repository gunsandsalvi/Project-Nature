//! Logs to logcat under the tag `kindling`, at `info` and above (A3.8), and panics logged before unwinding.

use std::ffi::{CString, c_char, c_int};

#[link(name = "log")]
unsafe extern "C" {
    fn __android_log_write(priority: c_int, tag: *const c_char, text: *const c_char) -> c_int;
}

struct Logcat;

impl log::Log for Logcat {
    fn enabled(&self, m: &log::Metadata) -> bool {
        m.level() <= log::Level::Info
    }

    fn log(&self, r: &log::Record) {
        if !self.enabled(r.metadata()) {
            return;
        }
        let priority = match r.level() {
            log::Level::Error => 6,
            log::Level::Warn => 5,
            log::Level::Info => 4,
            log::Level::Debug => 3,
            log::Level::Trace => 2,
        };
        let text = format!("{}: {}", r.target(), r.args()).replace('\0', " ");
        if let Ok(text) = CString::new(text) {
            // SAFETY: both strings are NUL-terminated and outlive the call.
            unsafe { __android_log_write(priority, c"kindling".as_ptr(), text.as_ptr()) };
        }
    }

    fn flush(&self) {}
}

static LOGCAT: Logcat = Logcat;

/// Once a process: later calls find the logger set and change nothing.
pub fn init() {
    if log::set_logger(&LOGCAT).is_ok() {
        log::set_max_level(log::LevelFilter::Info);
        std::panic::set_hook(Box::new(|info| log::error!(target: "kd::panic", "{info}")));
    }
}
