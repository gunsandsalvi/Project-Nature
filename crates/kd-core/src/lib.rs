//! kd-core: game time, uids, handles, stores, keyed chance, maths, coordinates, the sky, collections, errors and switches; the `Pool` trait (A3, A4).
//! Built so far: the numbers (`num`, `m`), game time (`time`), keyed chance (`chance`), the core's stored bits
//! (`bits`), positions on the world (`geo`) and the sky (`sky`); the rest join with their alphas
//! (IMPLEMENTATION.md).
//!
//! Implements RES-05, TIM-14, TIM-16, TIM-18, WLD-01 and WLD-07, see A3.2, A3.3, A3.7, A4.1 and A4.2: the same bits
//! on every target, game time and the 60-day year, keyed chance, the wrap-around world, and the sun and moon; each
//! module names its own.

#![deny(unsafe_code)]

pub mod bits;
pub mod chance;
pub mod geo;
pub mod m;
pub mod num;
pub mod sky;
pub mod time;
