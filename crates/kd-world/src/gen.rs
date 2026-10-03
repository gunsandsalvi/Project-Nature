//! World generation (A5.7): its steps, each a pure function of the seed and the steps before. Built so far: the
//! drainage that land presets share with it (`drain`); the rest joins at `MIL-04`.
//!
//! Implements WLD-08, see A5.7: worlds made by fast rules that imitate what deep time would make.

pub mod drain;
