//! The land's entries (A5.6, A5.7, A5.11): rocks, soils, biomes and deposits, and the land presets the first region
//! and the test lands are built from, as the game reads them: every value in base units (metres, cubic metres a
//! second, degrees Celsius, degrees of angle), every name resolved to its entry's number.
//!
//! Implements WLD-34, WLD-14 and WLD-27, see A5.6, A5.7 and A5.11: the island set from a few numbers, its rocks and
//! deposits, and its soils.

use serde::{Deserialize, Serialize};

/// A compass side: where a valley comes from, which way an escarpment faces, where the wind blows from.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub enum Side {
    North,
    East,
    South,
    West,
}

impl Side {
    pub const ALL: [Side; 4] = [Side::North, Side::East, Side::South, Side::West];

    /// Its word in the catalogue.
    pub fn word(self) -> &'static str {
        match self {
            Side::North => "north",
            Side::East => "east",
            Side::South => "south",
            Side::West => "west",
        }
    }
}

/// A rock (A5.7, `WLD-09`, `data/world/rocks.md`): how soft it wears (0.5 to 2, scaling the stream-power erosion),
/// how thick its beds lie, whether caves form in it, and the look it is drawn in.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Rock {
    pub id: String,
    pub name: String,
    /// Its permanent number in `data/ids.lock`.
    pub number: u16,
    pub softness: f32,
    /// Its beds' thinnest and thickest, metres.
    pub beds_m: [f32; 2],
    pub caves: bool,
    /// Its look's number.
    pub look: u16,
}

/// What a soil keeps of buried things, as flags (A5.11, `MAT-08`).
pub mod keeps {
    pub const BONE: u8 = 1;
    pub const WOOD: u8 = 2;
    pub const HIDE: u8 = 4;
}

/// A soil kind (A5.11, `WLD-27`, `data/world/soils.md`): the water it holds and takes in, how hard it digs, and what
/// it keeps of buried things.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Soil {
    pub id: String,
    pub name: String,
    pub number: u16,
    /// The water it can hold, metres.
    pub capacity_m: f32,
    /// The water it takes in a game hour, metres.
    pub intake_m: f32,
    /// How hard it digs, 1 to 10 (`MAT-06`).
    pub dig: u8,
    /// `keeps` flags.
    pub keeps: u8,
}

/// How many cover groups a cell's ground is split between (A5.2), in this order: trees, bushes, grass and herbs,
/// reeds, and bare ground.
pub const COVER_GROUPS: usize = 5;

/// A biome (A5.7 step 10, `WLD-31`, `data/world/biomes.md`): the look it takes on the map (A11.5) and its cover's
/// default shares, in 255ths summing to 255, by cover group.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Biome {
    pub id: String,
    pub name: String,
    pub number: u16,
    pub look: u16,
    pub cover: [u8; COVER_GROUPS],
}

/// How a deposit is carried downstream: the share kept every so many metres (A5.7 step 9).
#[derive(Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub struct Carried {
    pub keep: f32,
    pub every_m: f32,
}

/// A deposit (A5.7 step 9, `WLD-14`, `data/world/deposits.md`): in which rock it lies, or in rivers, how rich it
/// runs (1 to 5), and how a river carries it.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Deposit {
    pub id: String,
    pub name: String,
    pub number: u16,
    /// The rock it lies in, by number, or none.
    pub rock: Option<u16>,
    /// Whether it lies in river cells.
    pub rivers: bool,
    /// Its least and greatest richness.
    pub richness: [u8; 2],
    pub carried: Option<Carried>,
}

/// A land preset's river valley (A5.6): its river's flow until routing runs (A5.10), its width and its floodplain's,
/// and the side its source lies on.
#[derive(Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub struct Valley {
    pub flow_m3s: f32,
    pub width_m: f32,
    pub floodplain_m: f32,
    pub from: Side,
}

/// A land preset's escarpment (A5.6): the side of the island's middle it crosses, its height, the rocks of its
/// face from the top down, and its caves and rock shelters.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Escarpment {
    pub side: Side,
    pub height_m: f32,
    /// Rock numbers, top first.
    pub rocks: Vec<u16>,
    pub caves: u8,
    pub shelters: u8,
}

/// A land preset's climate (A5.6, A5.8): each season's mean temperature and rain, the daily range, the storm and
/// thunder days each season, and the wind's side.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Climate {
    pub mean_c: [f32; 4],
    pub range_c: f32,
    /// Rain each season, metres.
    pub rain_m: [f32; 4],
    pub storm_days: [u8; 4],
    pub thunder_days: [u8; 4],
    pub wind: Side,
}

/// A herd a land preset starts with (A5.6): its kind, by the id A7's animals will give it (α12b), and its count.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Herd {
    pub kind: String,
    pub count: u32,
}

/// A land preset (A5.6, `WLD-34`, `data/lands/`): an island of `island_m` across in a sea `sea_m` wide on every
/// side, around `centre_cell`, from a few numbers.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Land {
    pub id: String,
    pub name: String,
    pub number: u16,
    /// The cell at the island's middle, column and row.
    pub centre_cell: [u32; 2],
    pub island_m: f32,
    pub sea_m: f32,
    pub base_height_m: f32,
    pub relief_m: f32,
    pub valley: Valley,
    pub escarpment: Escarpment,
    /// Its soil's number and its base fertility, 0 to 5.
    pub soil: u16,
    pub fertility: u8,
    /// Its biome's number and its cover's shares, in 255ths summing to 255, by cover group.
    pub biome: u16,
    pub cover: [u8; COVER_GROUPS],
    pub climate: Climate,
    /// Read and kept for α12b's herds and α12c's small game: small game per km² of habitat (hares, birds, fish).
    pub herds: Vec<Herd>,
    pub small: [f32; 3],
    /// The world's tilt, degrees (A3.7's `Sky`).
    pub tilt_deg: f32,
}

/// Shares that sum to 1 as 255ths summing to 255 exactly: each rounded down, then a 255th more to those with the
/// largest remainders, the earlier first.
pub fn shares_255<const N: usize>(shares: [f32; N]) -> [u8; N] {
    let scaled = shares.map(|s| s.clamp(0.0, 1.0) * 255.0);
    let mut out = scaled.map(|s| s.floor() as u8);
    let mut left = 255u32.saturating_sub(out.iter().map(|&v| u32::from(v)).sum());
    let mut order: Vec<usize> = (0..N).collect();
    order.sort_by(|&a, &b| {
        (scaled[b] - scaled[b].floor())
            .total_cmp(&(scaled[a] - scaled[a].floor()))
            .then(a.cmp(&b))
    });
    for k in order {
        if left == 0 {
            break;
        }
        out[k] += 1;
        left -= 1;
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: WLD-34
    #[test]
    fn shares_sum_to_255() {
        assert_eq!(shares_255([0.55, 0.15, 0.25, 0.02, 0.03]), [140, 38, 64, 5, 8]);
        assert_eq!(shares_255([0.0, 0.0, 0.0, 0.0, 1.0]), [0, 0, 0, 0, 255]);
        for k in 0..1_000 {
            let a = (k % 97) as f32 / 97.0;
            let b = (1.0 - a) * ((k % 13) as f32 / 13.0);
            let s = shares_255([a, b, 1.0 - a - b]);
            assert_eq!(s.iter().map(|&v| u32::from(v)).sum::<u32>(), 255, "{a} {b}");
        }
    }
}
