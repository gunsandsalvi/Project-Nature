//! kd-android: `libkindling.so`, the Android entry (A2.5): the JNI functions `Native.kt` declares, the GL
//! context through EGL, the outbox Kotlin drains each frame, and logs to logcat. Every entry catches panics and
//! returns a safe value (A3.8). Only `touch` compiles everywhere, so its mapping is tested on the cloud machine;
//! the rest compiles for Android alone, and a workspace build elsewhere makes an empty library.

pub mod touch;

#[cfg(target_os = "android")]
mod entry;
#[cfg(target_os = "android")]
mod gl;
#[cfg(target_os = "android")]
mod logcat;
#[cfg(target_os = "android")]
mod platform;
