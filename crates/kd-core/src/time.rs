//! Game time (A4.1) and the 60-day year in code (A4.2): one clock of whole game seconds, the calendar's dates, and
//! the one rule that turns a length in life into a length in the game.

use core::fmt;

/// Between two barriers (A4.8): 288 a day; A3.4's uid window is `t / WINDOW`.
pub const WINDOW: u64 = 300;
/// Every third barrier: glances, talk, batch slots, the director's moments.
pub const QUARTER_HOUR: u64 = 900;
/// One weather step.
pub const HOUR: u64 = 3_600;
pub const DAY: u64 = 86_400;
/// 15 days.
pub const SEASON: u64 = 15 * DAY;
/// 60 days: four seasons of 15 (`TIM-18`).
pub const YEAR: u64 = 4 * SEASON;

/// Whole game seconds since the world began (A4.1): whole numbers order exactly and never drift.
///
/// Implements TIM-14, see A4.1: dates give the year, the season and the day.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct GameTime(pub u64);

impl GameTime {
    /// The uid window of A3.4, `t / WINDOW`: a u32 of them lasts about 248,000 game years.
    pub fn window(self) -> u32 {
        let w = self.0 / WINDOW;
        debug_assert!(w <= u64::from(u32::MAX), "game time beyond the uid windows (A3.4)");
        w as u32
    }

    /// The first barrier after `t`: the next multiple of `WINDOW`, never `t` itself.
    pub fn next_barrier(self) -> GameTime {
        GameTime((self.0 / WINDOW + 1) * WINDOW)
    }

    /// The date at `t`, history starting at `history_start` (a whole number of game years, A4.1): Year 1, spring,
    /// day 1, at second 0 of the day. Settling's years before it count down from Year 0.
    pub fn date(self, history_start: GameTime) -> Date {
        debug_assert_eq!(
            history_start.0 % YEAR,
            0,
            "history starts on a year's first second (A4.1)"
        );
        let since = self.0 as i64 - history_start.0 as i64;
        let in_year = since.rem_euclid(YEAR as i64) as u64;
        Date {
            year: (since.div_euclid(YEAR as i64) + 1) as i32,
            season: Season::ALL[(in_year / SEASON) as usize],
            day: ((in_year % SEASON) / DAY + 1) as u8,
            second: (in_year % DAY) as u32,
        }
    }
}

/// The four seasons of 15 days, named as in the half of the world where history begins (`TIM-14`).
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(u8)]
pub enum Season {
    Spring = 0,
    Summer = 1,
    Autumn = 2,
    Winter = 3,
}

impl Season {
    pub const ALL: [Season; 4] = [Season::Spring, Season::Summer, Season::Autumn, Season::Winter];

    /// The season's name as a date writes it.
    pub fn name(self) -> &'static str {
        match self {
            Season::Spring => "spring",
            Season::Summer => "summer",
            Season::Autumn => "autumn",
            Season::Winter => "winter",
        }
    }
}

/// A date (`TIM-14`): the year from the start of history, the season, the day of the season (1 to 15) and the
/// second of the day.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Date {
    pub year: i32,
    pub season: Season,
    pub day: u8,
    pub second: u32,
}

impl Date {
    /// The game time this date names, history starting at `history_start`: `date` read backwards.
    pub fn time(self, history_start: GameTime) -> GameTime {
        let since = i64::from(self.year - 1) * YEAR as i64
            + (self.season as i64) * SEASON as i64
            + i64::from(self.day - 1) * DAY as i64
            + i64::from(self.second);
        GameTime((history_start.0 as i64 + since) as u64)
    }
}

/// "Year 112, autumn, day 6" (`TIM-14`).
impl fmt::Display for Date {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Year {}, {}, day {}", self.year, self.season.name(), self.day)
    }
}

/// A length of game time, as the simulation sees every duration (A4.2).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Dur {
    pub game_s: u64,
}

/// A rate in game time, as the simulation sees every rate (A4.2).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Freq {
    pub per_game_s: f32,
}

impl Freq {
    /// A rate written `/d`: the real chance or amount per day, kept per game day (eating, tiring, work, weather,
    /// accidents).
    pub fn per_day(x: f32) -> Freq {
        Freq {
            per_game_s: x / DAY as f32,
        }
    }

    /// A rate written `/y`: what comes a few times a year in life comes as often per game year (births, crops,
    /// outbreaks, droughts, floods, wildfires, quakes).
    pub fn per_year(x: f32) -> Freq {
        Freq {
            per_game_s: x / YEAR as f32,
        }
    }
}

/// A year in life, as durations write `y`: 365 days.
pub const LIFE_YEAR: u64 = 365 * DAY;
/// A month in life, as durations write `mo`: a twelfth of a life year.
pub const LIFE_MONTH: u64 = LIFE_YEAR / 12;
/// Lengths in life up to two weeks take their real time (A4.2).
pub const REAL_UP_TO: u64 = 14 * DAY;
/// Lengths in life from 85 days are squeezed by 60/365 (A4.2's decision).
pub const SQUEEZED_FROM: u64 = 85 * DAY;
/// An in-between length's game length is at least a week (A4.2).
pub const BETWEEN_AT_LEAST: u64 = 7 * DAY;

/// Which of A4.2's three rows a length in life falls in.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LengthClass {
    /// Up to 14 days: the same in the game.
    Real,
    /// Between: the entry gives its game length, from 7 days up to the life length.
    Between,
    /// 85 days or more: life × 60 / 365, so a year becomes a game year.
    Squeezed,
}

impl LengthClass {
    pub fn of(life_s: u64) -> LengthClass {
        if life_s <= REAL_UP_TO {
            LengthClass::Real
        } else if life_s >= SQUEEZED_FROM {
            LengthClass::Squeezed
        } else {
            LengthClass::Between
        }
    }
}

/// Why a duration breaks `TIM-18`'s rule, as the catalogue check reports it (A3.6 rule 5).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LengthError {
    /// An in-between length with no game length given.
    Missing { life_s: u64 },
    /// An in-between game length under a week or over the life length.
    OutOfRange { life_s: u64, given_s: u64 },
    /// A game length given for a real or squeezed length that differs from the rule's.
    NotTheRule { life_s: u64, given_s: u64, rule_s: u64 },
}

impl fmt::Display for LengthError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let days = |s: u64| s / DAY;
        match *self {
            LengthError::Missing { life_s } => write!(
                f,
                "{} days in life is between two weeks and 85 days, so it needs its game length (TIM-18)",
                days(life_s)
            ),
            LengthError::OutOfRange { life_s, given_s } => write!(
                f,
                "a game length of {} days for {} days in life is outside 7 days to the life length (TIM-18)",
                days(given_s),
                days(life_s)
            ),
            LengthError::NotTheRule {
                life_s,
                given_s,
                rule_s,
            } => write!(
                f,
                "{} days in life takes {} game seconds by the rule, not the {} given (TIM-18)",
                days(life_s),
                rule_s,
                given_s
            ),
        }
    }
}

/// `TIM-18`'s rule in one place (A4.2), which the catalogue compiler applies to every duration: a length in life
/// of up to 14 days takes its real time; from 85 days, life × 60 / 365, rounded down to the second; in between, the
/// entry's own game length, from 7 days up to the life length. A game length given for a real or squeezed length
/// must equal the rule's.
///
/// Implements TIM-18, see A4.2.
pub fn game_length(life_s: u64, given_s: Option<u64>) -> Result<Dur, LengthError> {
    let rule_s = match LengthClass::of(life_s) {
        LengthClass::Real => life_s,
        LengthClass::Squeezed => life_s * 60 / 365,
        LengthClass::Between => {
            return match given_s {
                None => Err(LengthError::Missing { life_s }),
                Some(g) if g < BETWEEN_AT_LEAST || g > life_s => Err(LengthError::OutOfRange { life_s, given_s: g }),
                Some(g) => Ok(Dur { game_s: g }),
            };
        }
    };
    match given_s {
        Some(g) if g != rule_s => Err(LengthError::NotTheRule {
            life_s,
            given_s: g,
            rule_s,
        }),
        _ => Ok(Dur { game_s: rule_s }),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: TIM-14 TIM-18
    #[test]
    fn date_round_trips_at_every_edge() {
        let start = GameTime(10 * YEAR);
        assert_eq!(
            GameTime(10 * YEAR).date(start),
            Date {
                year: 1,
                season: Season::Spring,
                day: 1,
                second: 0
            }
        );
        // Every season and year edge of the first years and of Year 112, a second either side, round trips.
        for year in [1i64, 2, 3, 112] {
            for season in 0..4u64 {
                let edge = start.0 as i64 + (year - 1) * YEAR as i64 + (season * SEASON) as i64;
                for t in [edge - 1, edge, edge + 1, edge + DAY as i64 - 1, edge + 14 * DAY as i64] {
                    let t = GameTime(t as u64);
                    let d = t.date(start);
                    assert_eq!(d.time(start), t, "{d:?}");
                    assert!((1..=15).contains(&d.day) && d.second < DAY as u32, "{d:?}");
                }
                let d = GameTime(edge as u64).date(start);
                assert_eq!((d.year as i64, d.season as u64, d.day, d.second), (year, season, 1, 0));
                let before = GameTime(edge as u64 - 1).date(start);
                assert_eq!((before.day, before.second), (15, DAY as u32 - 1));
            }
        }
        // Settling's last second is Year 0's, in winter.
        let d = GameTime(start.0 - 1).date(start);
        assert_eq!((d.year, d.season, d.day), (0, Season::Winter, 15));
        assert_eq!(d.time(start), GameTime(start.0 - 1));
        // As TIM-14 writes a date.
        let d = Date {
            year: 112,
            season: Season::Autumn,
            day: 6,
            second: 0,
        };
        assert_eq!(d.to_string(), "Year 112, autumn, day 6");
        assert_eq!(d.time(start).date(start), d);
        assert_eq!((YEAR, SEASON, YEAR / WINDOW), (5_184_000, 1_296_000, 17_280));
    }

    // checks: TIM-14
    #[test]
    fn barriers_near_2_pow_40() {
        assert_eq!(GameTime(0).next_barrier(), GameTime(300));
        assert_eq!(GameTime(299).next_barrier(), GameTime(300));
        assert_eq!(GameTime(300).next_barrier(), GameTime(600));
        assert_eq!(
            (GameTime(0).window(), GameTime(299).window(), GameTime(300).window()),
            (0, 0, 1)
        );
        let big = 1u64 << 40;
        for t in [big - 301, big - 1, big, big + 1, big + 299] {
            let next = GameTime(t).next_barrier();
            assert!(
                next.0 > t && next.0 - t <= WINDOW && next.0.is_multiple_of(WINDOW),
                "{t}"
            );
            assert_eq!(next.window(), GameTime(t).window() + 1, "{t}");
        }
        assert_eq!(GameTime(big).window(), 3_665_038_759);
    }

    // checks: TIM-18
    #[test]
    fn game_length_rows() {
        let d = |n: u64| n * DAY;
        // 14 days in life is real.
        assert_eq!(game_length(d(14), None), Ok(Dur { game_s: d(14) }));
        assert_eq!(game_length(d(14), Some(d(14))), Ok(Dur { game_s: d(14) }));
        assert!(matches!(
            game_length(d(14), Some(d(10))),
            Err(LengthError::NotTheRule { .. })
        ));
        // 15 and 84 days are between: the entry's length, from a week to the life length, and never missing.
        for life in [d(15), d(84)] {
            assert_eq!(LengthClass::of(life), LengthClass::Between);
            assert_eq!(game_length(life, None), Err(LengthError::Missing { life_s: life }));
            assert_eq!(game_length(life, Some(d(10))), Ok(Dur { game_s: d(10) }));
            assert_eq!(game_length(life, Some(d(7))), Ok(Dur { game_s: d(7) }));
            assert_eq!(game_length(life, Some(life)), Ok(Dur { game_s: life }));
            assert!(matches!(
                game_length(life, Some(d(7) - 1)),
                Err(LengthError::OutOfRange { .. })
            ));
            assert!(matches!(
                game_length(life, Some(life + 1)),
                Err(LengthError::OutOfRange { .. })
            ));
        }
        // 85 days is squeezed: 60/365 of it, rounded down to the second, 13.97 days.
        assert_eq!(game_length(d(85), None), Ok(Dur { game_s: 1_207_232 }));
        // A life year is a game year, and nine months is 45 game days (BIO-15's pregnancy).
        assert_eq!(game_length(d(365), None), Ok(Dur { game_s: YEAR }));
        assert_eq!(game_length(9 * LIFE_MONTH, None), Ok(Dur { game_s: d(45) }));
        assert_eq!(
            game_length(40 * LIFE_YEAR, Some(40 * YEAR)),
            Ok(Dur { game_s: 40 * YEAR })
        );
        assert!(matches!(
            game_length(d(365), Some(d(61))),
            Err(LengthError::NotTheRule { .. })
        ));
        assert_eq!(Freq::per_year(1.0).per_game_s, 1.0 / 5_184_000.0);
        assert_eq!(Freq::per_day(2.0).per_game_s, 2.0 / 86_400.0);
    }
}
