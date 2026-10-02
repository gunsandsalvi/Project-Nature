//! kd-web: the WebAssembly entry: exports and WebGL2 (A2.6); implements PRC-11 in part.
//! Its modules exist only on wasm32, so `cargo clippy --workspace` on x86 builds it as an empty library.

#[cfg(target_arch = "wasm32")]
mod web;
#[cfg(target_arch = "wasm32")]
pub use web::*;
