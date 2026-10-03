//! The passes, each a struct holding its program and targets (A11.2, A11.13), drawn in A11.2's order.
//! α00 has the scene (the test card) and the upscale; light fields, object shadows, post, the crawl slot and the
//! UI join with their alphas.

pub mod scene;
pub mod upscale;
