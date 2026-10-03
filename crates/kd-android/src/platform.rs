//! The app's platform on Android (A2.2): the monotonic clock and the outbox `takeRequests` drains.

use std::sync::Mutex;

use kd_app::{Platform, Request};

#[derive(Default)]
pub struct AndroidPlatform {
    outbox: Mutex<Vec<Request>>,
}

impl AndroidPlatform {
    /// Everything posted since the last call, oldest first.
    pub fn take(&self) -> Vec<Request> {
        std::mem::take(&mut *self.outbox.lock().unwrap_or_else(|e| e.into_inner()))
    }
}

impl Platform for AndroidPlatform {
    fn now_ns(&self) -> u64 {
        let mut ts = libc::timespec { tv_sec: 0, tv_nsec: 0 };
        // SAFETY: clock_gettime writes one timespec through a valid pointer.
        unsafe { libc::clock_gettime(libc::CLOCK_MONOTONIC, &mut ts) };
        ts.tv_sec as u64 * 1_000_000_000 + ts.tv_nsec as u64
    }

    fn post(&self, r: Request) {
        self.outbox.lock().unwrap_or_else(|e| e.into_inner()).push(r);
    }
}
