//! kd-data: the catalogue (A3.6): the game's content as data, compiled from `data/**/*.md` into one blob the game
//! loads at start. `schema` holds the compiled entries, `world` the land's and `tuning` the tuned numbers, `blob` the
//! blob's format and its loader; behind feature `compile`, which only kd-tools, build scripts and tests turn on
//! (A3.9), the compiler (`compile`), which file holds which kind of entry (`kinds`) and the values with units the
//! compiler reads (`units`).
//!
//! Implements MAT-13, see A3.6: the content is in catalogues, each entry standing alone with its values and checks.

#![deny(unsafe_code)]

pub mod blob;
#[cfg(feature = "compile")]
pub mod compile;
#[cfg(feature = "compile")]
pub mod kinds;
pub mod schema;
pub mod tuning;
#[cfg(feature = "compile")]
pub mod units;
pub mod world;

pub use blob::BlobError;
pub use schema::{Air, Catalogue, Colour, Look, Surface, Versions};
pub use tuning::{Measure, Tuning};
pub use world::{Biome, Deposit, Land, Rock, Soil};
