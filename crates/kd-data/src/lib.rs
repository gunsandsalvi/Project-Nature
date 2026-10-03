//! kd-data: the catalogue (A3.6): the game's content as data, compiled from `data/**/*.md` into one blob the game
//! loads at start. `schema` holds the compiled entries, `blob` the blob's format and its loader, `kinds` which file
//! holds which kind of entry, and behind feature `compile` the compiler (`compile`) and the values with units it
//! reads (`units`).
//!
//! Implements MAT-13, see A3.6: the content is in catalogues, each entry standing alone with its values and checks.

#![deny(unsafe_code)]

pub mod blob;
#[cfg(feature = "compile")]
pub mod compile;
pub mod kinds;
pub mod schema;
#[cfg(feature = "compile")]
pub mod units;

pub use blob::BlobError;
pub use schema::{Air, Catalogue, Colour, Look, Surface, Versions};
