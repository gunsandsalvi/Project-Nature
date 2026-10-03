//! The passes, each a struct holding its program and targets (A11.2, A11.13), drawn in A11.2's order: the scene
//! (the ground, or the light card), post, the crawl slot, the upscale and the UI; the light fields are worked out on
//! the CPU (`field`), and object shadows join with the first things.

pub mod crawl;
pub mod post;
pub mod scene;
pub mod ui;
pub mod upscale;
