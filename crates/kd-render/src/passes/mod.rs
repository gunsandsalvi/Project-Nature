//! The passes, each a struct holding its program and targets (A11.2, A11.13), drawn in A11.2's order.
//! α01a has the scene (the light card), post and the upscale; light fields, object shadows, the crawl slot and the
//! UI join with their alphas.

pub mod post;
pub mod scene;
pub mod upscale;
