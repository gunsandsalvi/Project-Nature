//! kd-android: libkindling.so: JNI, EGL loading and logcat (A2.5, A3.8); implements PLT-01, PLT-02 in part.
//! Its modules exist only on Android, so `cargo clippy --workspace` on x86 builds it as an empty library.

#[cfg(target_os = "android")]
mod gl;
#[cfg(target_os = "android")]
mod jni;
#[cfg(target_os = "android")]
mod logcat;
#[cfg(target_os = "android")]
mod platform;
