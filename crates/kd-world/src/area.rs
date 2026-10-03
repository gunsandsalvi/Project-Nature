//! Areas (A5.3): 256 m squares with detail to the metre, heights at 257 × 257 points and a material per square
//! metre. α01b builds the relief noise and the demo area that stands in for made areas until α02b.
//!
//! Implements WLD-12, see A5.3: an area's ground to the metre.

pub mod demo;
pub mod relief;

use kd_core::geo::AreaId;
use kd_core::num;

/// Height points along an area's side, 1 m apart.
pub const SIDE: usize = 257;
/// Squares along an area's side.
pub const SQUARES: usize = 256;

/// What a square metre's surface is made of (A5.3): soil where the ground is gentle, bare dirt where it steepens,
/// the rock bed at that height where steeper still, and scree under cliffs.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(u8)]
pub enum Material {
    Soil = 0,
    Dirt = 1,
    Scree = 2,
    HardRock = 3,
    SoftRock = 4,
}

impl Material {
    pub const ALL: [Material; 5] = [
        Material::Soil,
        Material::Dirt,
        Material::Scree,
        Material::HardRock,
        Material::SoftRock,
    ];

    /// The material a stored byte names, if any.
    pub fn from_u8(b: u8) -> Option<Material> {
        Material::ALL.get(usize::from(b)).copied()
    }
}

/// An area's ground (A5.3).
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Ground {
    pub id: AreaId,
    /// The lowest point's height, in decimetres above sea level.
    pub base_dm: i32,
    /// Heights at `SIDE` × `SIDE` points 1 m apart, row by row from the north-west corner, in decimetres above
    /// `base_dm`.
    pub heights: Vec<u16>,
    /// A `Material` per square metre, `SQUARES` × `SQUARES` row by row from the north-west corner.
    pub material: Vec<u8>,
}

impl Ground {
    /// The height of point (`i`, `j`), `i` east and `j` south of the north-west corner, in metres above sea level.
    pub fn height_m(&self, i: usize, j: usize) -> f32 {
        (self.base_dm + i32::from(self.heights[j * SIDE + i])) as f32 / 10.0
    }

    /// The material of square (`i`, `j`).
    pub fn material_at(&self, i: usize, j: usize) -> Material {
        Material::from_u8(self.material[j * SQUARES + i]).expect("a stored material")
    }

    /// A hash of everything it holds, little-endian, for golden tests and the targets' checks (A3.2).
    pub fn hash(&self) -> u64 {
        let mut bytes = Vec::with_capacity(8 + self.heights.len() * 2 + self.material.len());
        bytes.extend(self.id.0.to_le_bytes());
        bytes.extend(self.base_dm.to_le_bytes());
        bytes.extend(self.heights.iter().flat_map(|h| h.to_le_bytes()));
        bytes.extend(&self.material);
        num::hash64(&bytes)
    }
}
