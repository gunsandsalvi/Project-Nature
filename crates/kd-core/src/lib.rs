//! kd-core: game time, keyed chance, maths and positions (A3, A3.7, A4.1, A4.2); implements TIM-16, TIM-14, TIM-18
//! and WLD-01 in part.
#![deny(unsafe_code)]

pub mod chance;
pub mod geo;
pub mod kinds;
pub mod m;
pub mod num;
pub mod selfcheck;
pub mod time;

pub use time::GameTime;
