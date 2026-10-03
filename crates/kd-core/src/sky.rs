//! The sky (A3.7, `WLD-07`): the one sun-and-moon function, which daylight, weather, calendars and the renderer's
//! light share. Pure, with every function through `kd_core::m`, so the sun stands in the same place on every
//! target.
//!
//! Implements WLD-07, see A3.7: the sun's path follows the season and the latitude, and the moon is full once a
//! season.

use crate::m;
use crate::time::{DAY, GameTime, SEASON, YEAR};

const TAU: f32 = 6.283_185_5;
const DEG: f32 = TAU / 360.0;

/// The world's sky: its axial tilt and the moon's cycles, drawn with the world (A5); until then `Sky::FIRST`.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Sky {
    pub tilt_deg: f32,
    /// Game seconds added to time before the moon's phase is read, so worlds differ.
    pub moon_start: u32,
    /// The eclipses' node cycle, for A5's rule (`MIL-04`).
    pub node_period: u32,
    pub node_start: u32,
}

impl Sky {
    /// The first region's sky until worlds draw their own (α03a): Earth's tilt, the moon from new at time 0.
    pub const FIRST: Sky = Sky {
        tilt_deg: 23.5,
        moon_start: 0,
        node_period: 0,
        node_start: 0,
    };
}

/// Where the sun and the moon stand, seen from one place at one time. Directions are unit vectors in (east, north,
/// up).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct SkyState {
    pub sun_dir: [f32; 3],
    pub sun_height_deg: f32,
    /// The day's length from sunrise to sunset, in hours.
    pub day_hours: f32,
    pub moon_dir: [f32; 3],
    /// 0 new, 0.5 full.
    pub moon_phase: f32,
    /// The share of the moon's face that is lit, 0 to 1.
    pub moon_lit: f32,
    /// The share of the sun or moon an eclipse hides: 0 until `MIL-04`.
    pub eclipse: f32,
}

/// A body's direction from its declination and hour angle at latitude `lat` (radians): (east, north, up).
fn direction(dec: f32, hour: f32, lat: f32) -> [f32; 3] {
    let (sd, cd) = (m::sin(dec), m::cos(dec));
    let (sh, ch) = (m::sin(hour), m::cos(hour));
    let (sl, cl) = (m::sin(lat), m::cos(lat));
    [-cd * sh, cl * sd - sl * cd * ch, sl * sd + cl * cd * ch]
}

/// The fractional part of a non-negative number.
fn frac(x: f32) -> f32 {
    x - x.floor()
}

/// The sun and the moon at time `t`, latitude `lat` and longitude `lon` in degrees (A3.7): spring day 1 is the
/// spring equinox, local noon comes 4 minutes earlier for each degree east, and the moon is full once a season.
pub fn sun_moon(t: GameTime, lat: f32, lon: f32, sky: &Sky) -> SkyState {
    let tilt = sky.tilt_deg * DEG;
    let f = (t.0 % YEAR) as f32 / YEAR as f32;
    let dec = m::asin(m::sin(tilt) * m::sin(TAU * f));
    let hour = TAU * (t.0 % DAY) as f32 / DAY as f32 + lon * DEG - TAU / 2.0;
    let phi = lat.clamp(-89.9, 89.9) * DEG;
    let sun_dir = direction(dec, hour, phi);
    let day_hours = 24.0 * m::acos((-m::tan(phi) * m::tan(dec)).clamp(-1.0, 1.0)) / (TAU / 2.0);
    // The moon: a cycle a season, its declination swinging with the year shifted by its phase.
    let p = frac(((t.0 + u64::from(sky.moon_start)) % SEASON) as f32 / SEASON as f32);
    let moon_dec = m::asin(m::sin(tilt) * m::sin(TAU * f + TAU * p));
    let moon_dir = direction(moon_dec, hour - TAU * p, phi);
    SkyState {
        sun_dir,
        sun_height_deg: m::asin(sun_dir[2].clamp(-1.0, 1.0)) / DEG,
        day_hours,
        moon_dir,
        moon_phase: p,
        moon_lit: (1.0 - m::cos(TAU * p)) / 2.0,
        eclipse: 0.0,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::time::HOUR;

    fn at(day_of_year: u64, hour: u64) -> GameTime {
        GameTime(day_of_year * DAY + hour * HOUR)
    }

    // checks: WLD-07 TIM-18
    #[test]
    fn equinox_days_are_12_hours() {
        // Spring day 1 and autumn day 1, at the equinox itself (their first second), latitudes 0–60°, within 0.1 h.
        for day in [0, 30] {
            for lat in [0.0, 15.0, 30.0, 45.0, 60.0, -45.0] {
                let s = sun_moon(at(day, 0), lat, 0.0, &Sky::FIRST);
                assert!(
                    (s.day_hours - 12.0).abs() < 0.1,
                    "day {day}, latitude {lat}: {} h",
                    s.day_hours
                );
            }
        }
    }

    // checks: WLD-07
    #[test]
    fn day_length_formula() {
        // 46° N at the summer solstice (summer day 1) gives the standard formula's 15.6 h at a tilt of 23.5°.
        let s = sun_moon(at(15, 12), 46.0, 0.0, &Sky::FIRST);
        assert!((s.day_hours - 15.57).abs() < 0.1, "{} h", s.day_hours);
        // Noon's sun stands at 90° − (latitude − declination), due south; local noon is 4 minutes earlier a
        // degree east.
        assert!(
            (s.sun_height_deg - (90.0 - 46.0 + 23.5)).abs() < 0.05,
            "{}",
            s.sun_height_deg
        );
        assert!(s.sun_dir[1] < 0.0 && s.sun_dir[0].abs() < 1e-3);
        let east = sun_moon(GameTime(15 * DAY + 12 * HOUR - 4 * 60), 46.0, 1.0, &Sky::FIRST);
        assert!(east.sun_dir[0].abs() < 1e-3, "{:?}", east.sun_dir);
        // The morning sun is in the east.
        assert!(sun_moon(at(0, 9), 21.0, 0.0, &Sky::FIRST).sun_dir[0] > 0.5);
    }

    // checks: WLD-07
    #[test]
    fn midnight_sun_and_polar_night() {
        // Beyond 90° − tilt: the summer sun never sets and the winter sun never rises.
        let summer = sun_moon(at(15, 0), 70.0, 0.0, &Sky::FIRST);
        assert!((summer.day_hours - 24.0).abs() < 1e-4, "{}", summer.day_hours);
        assert!(
            summer.sun_height_deg > 0.0,
            "the midnight sun: {}",
            summer.sun_height_deg
        );
        let winter = sun_moon(at(45, 12), 70.0, 0.0, &Sky::FIRST);
        assert_eq!(winter.day_hours, 0.0);
        assert!(
            winter.sun_height_deg < 0.0,
            "polar night at noon: {}",
            winter.sun_height_deg
        );
        // At the pole itself, held at 89.9°, all the same.
        assert!((sun_moon(at(15, 6), 90.0, 0.0, &Sky::FIRST).day_hours - 24.0).abs() < 1e-4);
    }

    // checks: WLD-07
    #[test]
    fn full_moon_once_a_season() {
        let mut fulls = 0;
        let mut last = 0.0f32;
        let mut rising = true;
        for hour in 0..(4 * 15 * 24) {
            let s = sun_moon(GameTime(hour * HOUR), 21.0, 0.0, &Sky::FIRST);
            if rising && s.moon_lit < last {
                fulls += 1;
                assert!(last > 0.99, "full at {last}");
            }
            rising = s.moon_lit >= last;
            last = s.moon_lit;
        }
        assert_eq!(fulls, 4, "one full moon a season, four a year");
        // Full at the middle of each season's cycle, opposite the sun.
        let full = sun_moon(GameTime(SEASON / 2), 0.0, 0.0, &Sky::FIRST);
        assert!((full.moon_phase - 0.5).abs() < 1e-6 && full.moon_lit > 0.999);
        let dot: f32 = (0..3).map(|i| full.moon_dir[i] * full.sun_dir[i]).sum();
        assert!(dot < -0.99, "opposite the sun: {dot}");
    }
}
