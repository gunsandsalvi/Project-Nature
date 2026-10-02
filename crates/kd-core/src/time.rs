//! Game time, the calendar and the one rule of the 60-day year (A4.1, A4.2).
//! Implements `TIM-14` (dates) and `TIM-18` (`game_length`) in part.

use std::fmt;

/// Game seconds since the world began (A4.1). Whole numbers, so they order exactly and never drift.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct GameTime(pub u64);

/// Game seconds between two barriers (A4.8); 288 a day.
pub const WINDOW: u64 = 300;
/// A quarter hour: every third barrier.
pub const QUARTER: u64 = 900;
/// One hour: one weather step.
pub const HOUR: u64 = 3_600;
/// One day.
pub const DAY: u64 = 86_400;
/// One season: 15 days (`TIM-18`).
pub const SEASON: u64 = 1_296_000;
/// One year: 60 days (`TIM-18`).
pub const YEAR: u64 = 5_184_000;

/// The seasons, in calendar order (`TIM-14`, `TIM-18`).
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Season {
    Spring,
    Summer,
    Autumn,
    Winter,
}

impl Season {
    /// The season with this index, 0 to 3, in calendar order.
    pub fn from_index(i: u64) -> Season {
        match i % 4 {
            0 => Season::Spring,
            1 => Season::Summer,
            2 => Season::Autumn,
            _ => Season::Winter,
        }
    }

    /// The season's name in lower case, as dates show it.
    pub fn name(self) -> &'static str {
        match self {
            Season::Spring => "spring",
            Season::Summer => "summer",
            Season::Autumn => "autumn",
            Season::Winter => "winter",
        }
    }
}

/// A calendar date (A4.1): year from the start of history (zero or less while settling), season, day 1 to 15 and
/// the second of the day. Implements `TIM-14` in part.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Date {
    pub year: i32,
    pub season: Season,
    pub day: u8,
    pub second: u32,
}

impl fmt::Display for Date {
    /// "Year 112, autumn, day 6" (`TIM-14`).
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Year {}, {}, day {}", self.year, self.season.name(), self.day)
    }
}

impl GameTime {
    /// The uid window of A3.4: `t / WINDOW`.
    pub fn window(self) -> u32 {
        (self.0 / WINDOW) as u32
    }

    /// The next multiple of `WINDOW` after `t` (A4.1).
    pub fn next_barrier(self) -> GameTime {
        GameTime((self.0 / WINDOW + 1) * WINDOW)
    }

    /// The date at this time; `history_start` is a whole number of years (A4.1). Implements `TIM-14` in part.
    pub fn date(self, history_start: GameTime) -> Date {
        debug_assert!(
            history_start.0.is_multiple_of(YEAR),
            "history starts on a year boundary (A4.1)"
        );
        let since = self.0 as i64 - history_start.0 as i64;
        Date {
            year: (since.div_euclid(YEAR as i64) + 1) as i32,
            season: Season::from_index((self.0 % YEAR) / SEASON),
            day: ((self.0 % SEASON) / DAY + 1) as u8,
            second: (self.0 % DAY) as u32,
        }
    }
}

/// A length of game time, filled by the catalogue compiler from α01a (A4.2).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord)]
pub struct Dur {
    pub game_s: u64,
}

/// A rate per game second, filled by the catalogue compiler from α01a (A4.2).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Freq {
    pub per_game_s: f32,
}

/// Up to this length in life (14 days), a duration keeps its real length (A4.2).
pub const REAL_UP_TO_S: u64 = 14 * DAY;
/// From this length in life (85 days), a duration is squeezed by 60/365 (A4.2).
pub const SQUEEZED_FROM_S: u64 = 85 * DAY;
/// The shortest game length an in-between duration may be given (A4.2).
pub const BETWEEN_MIN_S: u64 = 7 * DAY;

/// Why a duration breaks the rule of the 60-day year (A4.2).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LengthError {
    /// Between 14 and 85 days in life, the entry must give its game length.
    MissingGameLength,
    /// A real or squeezed length was given a game length other than the rule's.
    GameLengthMismatch { expected_s: u64 },
    /// An in-between game length lies outside 7 days to the life length.
    GameLengthOutOfRange,
}

impl fmt::Display for LengthError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            LengthError::MissingGameLength => write!(f, "a length between 14 and 85 days needs its game length"),
            LengthError::GameLengthMismatch { expected_s } => {
                write!(f, "the game length must be {expected_s} game seconds by the rule")
            }
            LengthError::GameLengthOutOfRange => {
                write!(f, "the game length must lie between 7 days and the length in life")
            }
        }
    }
}

/// The one rule of the 60-day year (A4.2), from a length in life to a length in the game, in seconds.
/// Implements `TIM-18` in part.
pub fn game_length(life_s: u64, given_game_s: Option<u64>) -> Result<u64, LengthError> {
    let rule = if life_s <= REAL_UP_TO_S {
        Some(life_s)
    } else if life_s >= SQUEEZED_FROM_S {
        Some((life_s * 60 + 182) / 365)
    } else {
        None
    };
    match (rule, given_game_s) {
        (Some(r), None) => Ok(r),
        (Some(r), Some(g)) if g == r => Ok(r),
        (Some(r), Some(_)) => Err(LengthError::GameLengthMismatch { expected_s: r }),
        (None, None) => Err(LengthError::MissingGameLength),
        (None, Some(g)) if (BETWEEN_MIN_S..=life_s).contains(&g) => Ok(g),
        (None, Some(_)) => Err(LengthError::GameLengthOutOfRange),
    }
}

#[cfg(test)]
mod tests;
