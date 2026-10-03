//! kd-core: game time, uids, handles, stores, keyed chance, maths, coordinates, the sky, collections, errors and switches; the `Pool` trait (A3, A4).
//! α00b builds the numbers (`num`, `m`), game time (`time`) and keyed chance (`chance`); the rest join with their
//! alphas (IMPLEMENTATION.md).
//!
//! Implements RES-05, TIM-14, TIM-16 and TIM-18, see A3.2, A3.3, A4.1 and A4.2: the same bits on every target, game
//! time and the 60-day year, and keyed chance; each module names its own.

#![deny(unsafe_code)]

pub mod bits;
pub mod chance;
pub mod m;
pub mod num;
pub mod time;
