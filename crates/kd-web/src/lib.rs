//! kd-web: the WebAssembly entry (A2.6): the app behind exports `web/glue.js` calls, on WebGL2 through `glow`.
//! One thread, so every call is direct. α00 draws the test card, takes pointer events and the page's visibility,
//! and offers `web/glue.js` the test hooks of A12.4; saves and audio join with their alphas. Its code compiles for
//! `wasm32` alone, so a workspace build elsewhere makes an empty library.

#[cfg(target_arch = "wasm32")]
mod web;
#[cfg(target_arch = "wasm32")]
pub use web::*;
