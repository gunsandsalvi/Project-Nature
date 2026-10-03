//! The demo area (α01b): the 256 m window at x 384–640, y 384–640 of B11's test site, which its escarpment crosses,
//! as heights every metre and a material per square metre. It stands in for areas until α02b, then stays as the
//! porting check's scene (A5.3).
//! Implements `WLD-12` in part, see A5.3.

use super::relief::{Site, layer_at};
use kd_core::num;

/// The window's north-west corner in the site, metres.
pub const WINDOW_X: f32 = 384.0;
pub const WINDOW_Y: f32 = 384.0;
/// Height points along a side: 256 squares of 1 m.
pub const SIDE: usize = 257;
/// Squares along a side.
pub const SQUARES: usize = 256;

/// Material of a square (A5.3): 1–6 the rock layer that shows where the ground is steeper than 45°, else soil, or
/// scree within 6 m of the cliff's foot. Soil's surface by slope is the picture's choice (A11.5).
pub const SOIL: u8 = 8;
pub const SCREE: u8 = 9;

/// B11's test site's corner heights, metres.
const CORNERS: [f32; 4] = [310.0, 342.0, 365.0, 330.0];

/// The demo area's ground.
pub struct DemoArea {
    /// The lowest point, decimetres above sea level.
    pub base_dm: f32,
    /// Heights at 257 × 257 points, rows from the north-west corner, decimetres above `base_dm`.
    pub heights: Vec<u16>,
    /// Per square metre, 256 × 256 rows from the north-west corner: `SOIL`, `SCREE` or a rock layer 1–6.
    pub material: Vec<u8>,
}

impl DemoArea {
    /// The height at point (`i`, `j`) in metres above sea level.
    pub fn height_m(&self, i: usize, j: usize) -> f32 {
        (self.base_dm + f32::from(self.heights[j * SIDE + i])) / 10.0
    }
}

/// The demo area of B11's site made with `seed` (99 is B11's).
pub fn demo_area(seed: u64) -> DemoArea {
    let site = Site::new(seed, CORNERS);
    let tops: Vec<f32> = (0..SIDE * SIDE)
        .map(|k| site.surface(WINDOW_X + (k % SIDE) as f32, WINDOW_Y + (k / SIDE) as f32))
        .collect();
    let lo = tops.iter().copied().fold(f32::MAX, num::min);
    let base_dm = (lo * 10.0).floor();
    let heights = tops
        .iter()
        .map(|v| num::min(num::max((v * 10.0).floor() - base_dm, 0.0), 65535.0) as u16)
        .collect();
    let mut material = vec![SOIL; SQUARES * SQUARES];
    for j in 0..SQUARES {
        for i in 0..SQUARES {
            let h = |a: usize, b: usize| tops[b * SIDE + a];
            // the square's rise per metre along x and y, from its corners; steeper than 45° when over 1
            let gx = (h(i + 1, j) + h(i + 1, j + 1) - h(i, j) - h(i, j + 1)) * 0.5;
            let gy = (h(i, j + 1) + h(i + 1, j + 1) - h(i, j) - h(i + 1, j)) * 0.5;
            let steep = gx * gx + gy * gy > 1.0;
            let (x, y) = (WINDOW_X + i as f32 + 0.5, WINDOW_Y + j as f32 + 0.5);
            let below_foot = -site.edge_dist(x, y) - 2.0; // the cliff spans 2 m either side of its edge line
            material[j * SQUARES + i] = if steep {
                layer_at(seed, site.surface(x, y)).0
            } else if (0.0..6.0).contains(&below_foot) {
                SCREE
            } else {
                SOIL
            };
        }
    }
    DemoArea {
        base_dm,
        heights,
        material,
    }
}

#[cfg(test)]
mod tests;
