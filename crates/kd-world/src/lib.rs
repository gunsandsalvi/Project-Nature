//! kd-world: world cells, areas, terrain, water, weather, generation and paths (A5).
//! Built so far: areas' relief noise and the demo area (`area`), and the world's cells (`cells`); the rest join with
//! their alphas (IMPLEMENTATION.md).
//!
//! Implements WLD-12, see A5.3: an area's ground to the metre; each module names its own.

#![deny(unsafe_code)]

pub mod area;
pub mod cells;
