//! The catalogue's source schemas (A3.6): one struct per kind of entry, parsed from an entry's fenced `toml` block with
//! no unknown and no missing field (`MAT-17`).
//! Implements `MAT-13` and `MAT-17` in part, see A3.6.
//!
//! **Decision:** the light tables' multipliers and offsets are written as decimal strings (`"0.42"`), because this
//! crate holds no `f64` (A3.2) and `kd-render`'s port of the mockup computes in doubles from exactly the numbers the
//! mockup writes (A11.3); the compiler checks that each string is a plain decimal number.

use serde::{Deserialize, Serialize};

/// The fields every entry has (A3.6): a permanent `snake_case` id, the English name (equal to the heading), the
/// milestone that first needs it (`MAT-16`) and the `PROJECT.md` IDs it supports (`MAT-17`).
#[derive(Clone, Debug, Default, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Common {
    pub id: String,
    pub name: String,
    pub stage: String,
    pub checks: Vec<String>,
}

/// A family of palette colours (`data/palette/colours.md`, A11.3): each colour a name and a `#rrggbb` value; a
/// colour's palette index is its position over the whole palette, families in number order.
#[derive(Clone, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct ColourFamily {
    #[serde(flatten)]
    pub common: Common,
    pub colours: Vec<(String, String)>,
}

/// A ladder of shades, dark to light (`data/palette/ladders.md`, `PRE-20`): 2 to 8 colour names.
#[derive(Clone, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Ladder {
    #[serde(flatten)]
    pub common: Common,
    pub steps: Vec<String>,
}

/// What a light entry is: an index table of the table texture, or a version of the palette (A11.3).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum LightRole {
    /// A row of the table texture: each colour to its nearest allowed colour after the change (the mockup's `table`).
    Table,
    /// A row of the palette texture: each colour changed, except those kept (the mockup's `variant`).
    Version,
}

/// How a light entry changes a colour, in the mockup's own expressions (A11.3).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum LightMethod {
    /// Unchanged.
    Same,
    /// Firelight's warming by `k` (the mockup's `warmFn`).
    Warm,
    /// Toward the haze colour `toward` by `amount` (the mockup's `lerp` per channel).
    Haze,
    /// In OKLab: each of L, a, b times `mul` plus `add`, L then kept within `l_min` and `l_max`.
    Lab,
}

/// A light table or palette version (`data/palette/light.md`, A11.3, `PRE-21`, `PRE-30`).
/// Tables name the colours they may give with `only_families`, `exclude_families` and `exclude_colours`; versions name
/// the colours they leave unchanged with `keep_families` and `keep_colours`.
#[derive(Clone, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct LightTable {
    #[serde(flatten)]
    pub common: Common,
    pub role: LightRole,
    pub method: LightMethod,
    #[serde(default)]
    pub k: Option<String>,
    #[serde(default)]
    pub toward: Option<[u8; 3]>,
    #[serde(default)]
    pub amount: Option<String>,
    #[serde(default)]
    pub mul: Option<[String; 3]>,
    #[serde(default)]
    pub add: Option<[String; 3]>,
    #[serde(default)]
    pub l_min: Option<String>,
    #[serde(default)]
    pub l_max: Option<String>,
    #[serde(default)]
    pub only_families: Vec<String>,
    #[serde(default)]
    pub exclude_families: Vec<String>,
    #[serde(default)]
    pub exclude_colours: Vec<String>,
    #[serde(default)]
    pub keep_families: Vec<String>,
    #[serde(default)]
    pub keep_colours: Vec<String>,
}

/// A mark of a surface's ground (A11.5): grikes in limestone, ash, gravel, mud or sand; the ground shader is to draw
/// each, and reads none yet.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum SurfaceFlag {
    Gravel,
    Mud,
    Sand,
    Ash,
    Grikes,
}

impl SurfaceFlag {
    /// The flag's bit in the blob and the surfaces texture.
    pub fn bit(self) -> u8 {
        match self {
            SurfaceFlag::Gravel => 1,
            SurfaceFlag::Mud => 2,
            SurfaceFlag::Sand => 4,
            SurfaceFlag::Ash => 8,
            SurfaceFlag::Grikes => 16,
        }
    }
}

/// A ground surface (`data/models/surfaces.md`, A11.5, `PRE-20`): the ladder its light picks from, the share of its
/// ground with stones and their size in metres, the share with tufts, and its flags. The numbers are decimal strings
/// of at most three decimals, as the light tables' are (see the decision above).
#[derive(Clone, Debug, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Surface {
    #[serde(flatten)]
    pub common: Common,
    pub ladder: String,
    pub stone_density: String,
    pub stone_size: String,
    pub tuft_density: String,
    #[serde(default)]
    pub flags: Vec<SurfaceFlag>,
}

/// `data/VERSION.toml` (A3.6, `PLT-09`): the rules version and the generator version.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Version {
    pub major: u16,
    pub minor: u16,
    pub generator: u16,
}
