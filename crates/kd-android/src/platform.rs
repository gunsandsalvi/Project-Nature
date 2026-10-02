//! The phone's `Platform` (A2.2): the monotonic clock and the outbox Kotlin polls each frame (A2.5).

use kd_app::{Platform, Request};
use std::sync::Mutex;

#[derive(Default)]
pub struct AndroidPlatform {
    outbox: Mutex<Vec<Request>>,
}

impl AndroidPlatform {
    /// The outbox as JSON, or `None` when empty (`takeRequests`).
    pub fn take_json(&self) -> Option<String> {
        let mut out = self.outbox.lock().unwrap_or_else(|e| e.into_inner());
        if out.is_empty() {
            return None;
        }
        let json = kd_app::requests_json(&out);
        out.clear();
        Some(json)
    }
}

impl Platform for AndroidPlatform {
    fn now_ns(&self) -> u64 {
        let mut ts = libc::timespec { tv_sec: 0, tv_nsec: 0 };
        // SAFETY: clock_gettime writes one timespec we own.
        #[allow(unsafe_code)]
        unsafe {
            libc::clock_gettime(libc::CLOCK_MONOTONIC, &mut ts);
        }
        ts.tv_sec as u64 * 1_000_000_000 + ts.tv_nsec as u64
    }

    fn post(&self, r: Request) {
        self.outbox.lock().unwrap_or_else(|e| e.into_inner()).push(r);
    }
}
