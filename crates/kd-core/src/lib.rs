//! kd-core: game time, uids, handles, stores, keyed chance, maths, coordinates, the sky, collections, errors and switches; the `Pool` trait (A3, A4).
//! α00b builds the numbers (`num`, `m`), game time (`time`) and keyed chance (`chance`); the rest join with their
//! alphas (IMPLEMENTATION.md).

#![deny(unsafe_code)]

pub mod bits;
pub mod chance;
pub mod m;
pub mod num;
pub mod time;
