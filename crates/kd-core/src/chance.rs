//! Keyed chance (A3.3): every draw is a pure function of (world, system, purpose, subject, moment), with no
//! generator position, so threads, work order and skipped draws never change another draw.
//! Implements `TIM-16` in part.

pub mod hash;
pub mod purposes;
pub mod registry;
pub mod stream;

#[cfg(test)]
mod b02_reference;
#[cfg(test)]
mod tests;

pub use hash::{draw, stream_seed};
pub use registry::{Fortune, Purpose, SubjectKind, check_registry, systems};
pub use stream::{Stream, moment};
