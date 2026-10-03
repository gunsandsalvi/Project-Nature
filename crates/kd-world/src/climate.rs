//! The climate record (A5.8, `WLD-16`): each weather cell's seasons, made when the world is made and never changed
//! (`SCP-21`). Until `MIL-03` a land preset gives every weather cell over its island and sea its own numbers, with
//! each weather cell's reference height, from which each world cell's `clim` takes its warmth (A5.2).
//!
//! Implements WLD-16, see A5.8: each place's climate, which never changes.

use kd_core::geo::{WEATHER_X, WEATHER_Y, WeatherIx};
use kd_data::world::Side;

/// A weather cell's climate (A5.8, the part α02a keeps): each season's mean warmth and rain, the daily range, the
/// storm and thunder days each season, the wind's side, and the height its warmth is measured at.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ClimateRecord {
    pub mean_c: [f32; 4],
    pub range_c: f32,
    pub rain_m: [f32; 4],
    pub storm_days: [u8; 4],
    pub thunder_days: [u8; 4],
    pub wind: Side,
    pub ref_height_m: f32,
}

/// Every weather cell's climate, none where nothing lives (void).
#[derive(Clone, Debug, PartialEq)]
pub struct ClimateMap {
    pub records: Vec<Option<ClimateRecord>>,
}

impl Default for ClimateMap {
    fn default() -> ClimateMap {
        ClimateMap {
            records: vec![None; (WEATHER_X * WEATHER_Y) as usize],
        }
    }
}

impl ClimateMap {
    pub fn of(&self, w: WeatherIx) -> Option<&ClimateRecord> {
        self.records[usize::from(w.0)].as_ref()
    }
}
