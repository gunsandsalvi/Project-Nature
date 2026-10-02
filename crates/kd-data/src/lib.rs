//! kd-data: catalogue schemas, the blob loader and, behind feature `compile`, the catalogue compiler (A3.6);
//! implements MAT-13, MAT-17 and PLT-09 in part.
#![deny(unsafe_code)]

pub mod blob;
#[cfg(feature = "compile")]
pub mod compile;
pub mod kinds;
pub mod schema;

pub use blob::{Catalogue, LoadError};
