//! Values with units in catalogue entries (A3.6, A4.2, A5.13): sizes and amounts, single, ranges or open above
//! (`"220 m"`, `"8-30 cm"`, `"6+ cm"`), flows (`"8 m3/s"`), temperatures (`"9 C"`), rain (`"170 mm"`) and angles
//! (`"23.5 deg"`); durations in life, which `kd_core::time::game_length` turns into game time (`"30 s"`, `"3 d"`,
//! `"9 mo"`, `"2 y"`, or `{ life = "6 w", game = "10 d" }` between two weeks and 85 days); rates per day or per year
//! (`"3 kg/d"`, `"2/y"`); and scaled values with the Earth value they were scaled from (`WLD-30`).
//! Each reader returns base units (metres, kilograms, litres, cubic metres a second, degrees Celsius, degrees of
//! angle, game seconds) or a message naming the field, to which the compiler adds the entry.
//!
//! Implements TIM-18 and WLD-30, see A3.6, A4.2 and A5.13: lengths in life become game time by one rule, rates
//! name their period, and scaled values carry their Earth value.

use kd_core::time::{DAY, Dur, Freq, HOUR, LIFE_MONTH, LIFE_YEAR, game_length};
use serde::Deserialize;

/// What a value measures: a field takes only its own kind's units.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Dim {
    /// Metres: mm, cm, m, km (rain is a length).
    Length,
    /// Kilograms: g, kg.
    Mass,
    /// Litres: l.
    Volume,
    /// Cubic metres a second: m3/s.
    Flow,
    /// Degrees Celsius: C.
    Temperature,
    /// Degrees of angle: deg.
    Angle,
}

impl Dim {
    pub const ALL: [Dim; 6] = [
        Dim::Length,
        Dim::Mass,
        Dim::Volume,
        Dim::Flow,
        Dim::Temperature,
        Dim::Angle,
    ];

    /// Its units and their size in its base unit, as a power of ten.
    fn units(self) -> &'static [(&'static str, i32)] {
        match self {
            Dim::Length => &[("mm", -3), ("cm", -2), ("m", 0), ("km", 3)],
            Dim::Mass => &[("g", -3), ("kg", 0)],
            Dim::Volume => &[("l", 0)],
            Dim::Flow => &[("m3/s", 0)],
            Dim::Temperature => &[("C", 0)],
            Dim::Angle => &[("deg", 0)],
        }
    }

    fn names(self) -> String {
        self.units().iter().map(|u| u.0).collect::<Vec<_>>().join(", ")
    }
}

/// An amount in base units: one value (`lo == hi`), a range, or open above (`hi` infinite, `"6+ cm"`).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Amount {
    pub lo: f32,
    pub hi: f32,
}

impl Amount {
    /// The value of an amount that must be one value.
    pub fn single(self, field: &str) -> Result<f32, String> {
        if self.lo == self.hi {
            Ok(self.lo)
        } else {
            Err(format!("field `{field}` takes one value, not a range"))
        }
    }
}

/// A plain decimal number exactly as written: its digits as one whole number, how many of them follow the point, and
/// its sign.
#[derive(Clone, Copy, Debug)]
struct Decimal {
    negative: bool,
    digits: u64,
    places: u32,
}

impl Decimal {
    /// The number times ten to the `shift`, rounded once to the nearest float, as Rust reads a decimal: so a value
    /// in its unit becomes exactly the float nearest its value in the base unit (`"30 cm"` the float nearest 0.3).
    fn times_ten_to(self, shift: i32) -> f32 {
        let exp = shift - self.places as i32;
        let sign = if self.negative { "-" } else { "" };
        format!("{sign}{}e{exp}", self.digits).parse().unwrap_or(f32::NAN)
    }
}

/// A plain decimal number, with a sign where `signed` allows one: no exponents, infinities or other spellings.
fn number(t: &str, signed: bool) -> Option<Decimal> {
    let (negative, body) = match t.strip_prefix('-') {
        Some(rest) if signed => (true, rest),
        _ => (false, t),
    };
    let ok = !body.is_empty()
        && body.chars().all(|c| c.is_ascii_digit() || c == '.')
        && body.chars().filter(|&c| c == '.').count() <= 1
        && !body.starts_with('.')
        && !body.ends_with('.');
    if !ok {
        return None;
    }
    let (whole, part) = body.split_once('.').unwrap_or((body, ""));
    Some(Decimal {
        negative,
        digits: format!("{whole}{part}").parse().ok()?,
        places: part.len() as u32,
    })
}

/// The numbers before the unit, in the base unit when the unit is ten to the `shift` of it: one, a range `lo-hi`
/// (rising), or open above `lo+`; only temperatures may be negative.
fn numbers(v: &str, signed: bool, shift: i32) -> Option<(f32, f32)> {
    if let Some(lo) = v.strip_suffix('+') {
        return number(lo, signed).map(|x| (x.times_ten_to(shift), f32::INFINITY));
    }
    // A range's dash comes after its first character, which may be the first number's sign.
    if let Some(i) = v.get(1..).and_then(|r| r.find('-')).map(|i| i + 1) {
        let (lo, hi) = (number(&v[..i], signed)?, number(&v[i + 1..], signed)?);
        return Some((lo.times_ten_to(shift), hi.times_ten_to(shift)));
    }
    number(v, signed).map(|x| (x.times_ten_to(shift), x.times_ten_to(shift)))
}

/// An amount of `dim` written with its unit, as `"8-30 cm"`.
pub fn amount(field: &str, text: &str, dim: Dim) -> Result<Amount, String> {
    let bad = |why: String| format!("field `{field}`: {text:?} {why}");
    let (v, unit) = text
        .trim()
        .rsplit_once(' ')
        .ok_or_else(|| bad(format!("needs a number and a unit ({})", dim.names())))?;
    let Some(&(_, shift)) = dim.units().iter().find(|u| u.0 == unit) else {
        return Err(bad(format!("has no unit this field takes ({})", dim.names())));
    };
    let (lo, hi) = numbers(v.trim(), dim == Dim::Temperature, shift).ok_or_else(|| bad("is not a number".into()))?;
    if hi < lo {
        return Err(bad("is a range from its higher value to its lower".into()));
    }
    Ok(Amount { lo, hi })
}

/// One value of `dim` with its unit, as `"220 m"`.
pub fn value(field: &str, text: &str, dim: Dim) -> Result<f32, String> {
    amount(field, text, dim)?.single(field)
}

/// One value with whatever unit it names, and what that unit measures, as a tuning table writes it (`"1.2 m"`):
/// no unit belongs to two kinds.
pub fn any_value(field: &str, text: &str) -> Result<(f32, Dim), String> {
    let unit = text.trim().rsplit_once(' ').map_or("", |(_, u)| u);
    let dim = Dim::ALL
        .into_iter()
        .find(|d| d.units().iter().any(|u| u.0 == unit))
        .ok_or_else(|| {
            let all: Vec<String> = Dim::ALL.iter().map(|d| d.names()).collect();
            format!("field `{field}`: {text:?} has no unit ({})", all.join(", "))
        })?;
    Ok((value(field, text, dim)?, dim))
}

/// A length in life in seconds, as `"6 w"`: s, min, h, d, w, mo (a twelfth of a year) or y (365 days).
pub fn life_seconds(field: &str, text: &str) -> Result<u64, String> {
    let bad = |why: &str| format!("field `{field}`: {text:?} {why}");
    let (v, unit) = text
        .trim()
        .rsplit_once(' ')
        .ok_or_else(|| bad("needs a number and a unit (s, min, h, d, w, mo, y)"))?;
    let size = match unit {
        "s" => 1,
        "min" => 60,
        "h" => HOUR,
        "d" => DAY,
        "w" => 7 * DAY,
        "mo" => LIFE_MONTH,
        "y" => LIFE_YEAR,
        _ => return Err(bad("has no unit of time (s, min, h, d, w, mo, y)")),
    };
    // Whole seconds, worked out exactly from the digits and rounded half up.
    let x = number(v, false).ok_or_else(|| bad("is not a number"))?;
    let scale = 10u128.checked_pow(x.places).ok_or_else(|| bad("is not a number"))?;
    let seconds = (u128::from(x.digits) * u128::from(size) + scale / 2) / scale;
    u64::try_from(seconds).map_err(|_| bad("is too long"))
}

/// A duration as an entry writes it: its length in life, or for one between two weeks and 85 days, its length in
/// life and in the game (A4.2).
#[derive(Clone, Debug, Deserialize, PartialEq)]
#[serde(untagged)]
pub enum DurSrc {
    Life(String),
    Given { life: String, game: String },
}

/// A duration in game time, by `TIM-18`'s one rule (A4.2).
pub fn duration(field: &str, d: &DurSrc) -> Result<Dur, String> {
    let (life, game) = match d {
        DurSrc::Life(l) => (life_seconds(field, l)?, None),
        DurSrc::Given { life, game } => (life_seconds(field, life)?, Some(life_seconds(field, game)?)),
    };
    game_length(life, game).map_err(|e| format!("field `{field}`: {e}"))
}

/// A rate naming its period, as `"3 kg/d"` (`dim` some) or `"2/y"` (a count, `dim` none): `/d` keeps the real
/// amount a day, `/y` comes as often a game year as a year in life (A4.2).
pub fn rate(field: &str, text: &str, dim: Option<Dim>) -> Result<Freq, String> {
    let bad = |why: String| format!("field `{field}`: {text:?} {why}");
    let (v, period) = text
        .trim()
        .rsplit_once('/')
        .ok_or_else(|| bad("names no period (/d or /y)".into()))?;
    let x = match dim {
        Some(dim) => value(field, v.trim(), dim)?,
        None => number(v.trim(), false)
            .ok_or_else(|| bad("is not a number".into()))?
            .times_ten_to(0),
    };
    match period.trim() {
        "d" => Ok(Freq::per_day(x)),
        "y" => Ok(Freq::per_year(x)),
        _ => Err(bad("names no period (/d or /y)".into())),
    }
}

/// A value set by the world's scale, with the Earth value it was scaled from (`WLD-30`, A5.13).
#[derive(Clone, Debug, Deserialize, PartialEq)]
#[serde(deny_unknown_fields)]
pub struct ScaledSrc {
    pub value: String,
    pub scaled_from: String,
}

/// A scaled amount and its Earth value, both of `dim`.
pub fn scaled(field: &str, s: &ScaledSrc, dim: Dim) -> Result<(Amount, Amount), String> {
    Ok((
        amount(field, &s.value, dim)?,
        amount(&format!("{field}.scaled_from"), &s.scaled_from, dim)?,
    ))
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_core::time::{SEASON, YEAR};

    // checks: TIM-18 WLD-30
    #[test]
    fn parses_every_form() {
        // Each unit of each kind, in base units.
        let one = |text: &str, dim: Dim| value("f", text, dim).unwrap();
        for (text, dim, want) in [
            ("170 mm", Dim::Length, 0.17),
            ("8 cm", Dim::Length, 0.08),
            ("220 m", Dim::Length, 220.0),
            ("3 km", Dim::Length, 3000.0),
            ("500 g", Dim::Mass, 0.5),
            ("2.5 kg", Dim::Mass, 2.5),
            ("4 l", Dim::Volume, 4.0),
            ("8 m3/s", Dim::Flow, 8.0),
            ("9 C", Dim::Temperature, 9.0),
            ("-3 C", Dim::Temperature, -3.0),
            ("23.5 deg", Dim::Angle, 23.5),
        ] {
            assert!((one(text, dim) - want).abs() < 1e-6, "{text}");
        }
        // Ranges, open ranges, and a range of temperatures below freezing.
        assert_eq!(
            amount("f", "8-30 cm", Dim::Length).unwrap(),
            Amount { lo: 0.08, hi: 0.3 }
        );
        assert_eq!(
            amount("f", "6+ cm", Dim::Length).unwrap(),
            Amount {
                lo: 0.06,
                hi: f32::INFINITY
            }
        );
        assert_eq!(
            amount("f", "-5--2 C", Dim::Temperature).unwrap(),
            Amount { lo: -5.0, hi: -2.0 }
        );
        assert_eq!(
            amount("f", "0.3-3 m", Dim::Length).unwrap(),
            Amount { lo: 0.3, hi: 3.0 }
        );
        // Durations: each unit; a real length stays, a squeezed one becomes a game year a year, and one between
        // takes the game length given.
        let life = |t: &str| life_seconds("f", t).unwrap();
        assert_eq!(
            [
                life("30 s"),
                life("5 min"),
                life("2 h"),
                life("3 d"),
                life("6 w"),
                life("9 mo"),
                life("2 y")
            ],
            [30, 300, 7_200, 3 * DAY, 42 * DAY, 9 * LIFE_MONTH, 2 * LIFE_YEAR]
        );
        let dur = |d: DurSrc| duration("f", &d).unwrap().game_s;
        assert_eq!(dur(DurSrc::Life("3 d".into())), 3 * DAY);
        assert_eq!(dur(DurSrc::Life("2 y".into())), 2 * YEAR);
        assert_eq!(dur(DurSrc::Life("1 y".into())), 4 * SEASON);
        assert_eq!(
            dur(DurSrc::Given {
                life: "6 w".into(),
                game: "10 d".into()
            }),
            10 * DAY
        );
        // The untagged form reads both ways as TOML writes them.
        #[derive(Deserialize)]
        struct T {
            a: DurSrc,
            b: DurSrc,
        }
        let t: T = toml::from_str("a = \"3 d\"\nb = { life = \"6 w\", game = \"10 d\" }").unwrap();
        assert_eq!((dur(t.a), dur(t.b)), (3 * DAY, 10 * DAY));
        // Rates: per day kept, per year as often a game year.
        let per = |t: &str, d: Option<Dim>| rate("f", t, d).unwrap().per_game_s;
        assert!((per("3 kg/d", Some(Dim::Mass)) - 3.0 / DAY as f32).abs() < 1e-12);
        assert!((per("2/y", None) - 2.0 / YEAR as f32).abs() < 1e-12);
        assert!((per("0.5 /d", None) - 0.5 / DAY as f32).abs() < 1e-12);
        // A scaled value and its Earth value.
        let s = ScaledSrc {
            value: "50 km".into(),
            scaled_from: "1000 km".into(),
        };
        let (v, earth) = scaled("storm_size", &s, Dim::Length).unwrap();
        assert_eq!((v.lo, earth.lo), (50_000.0, 1_000_000.0));
        // Read exactly: a value in its unit is the float nearest its value in the base unit, rounded once.
        assert_eq!(value("f", "30 cm", Dim::Length), Ok(0.3));
        assert_eq!(value("f", "0.1 mm", Dim::Length), Ok(0.0001));
        assert_eq!(value("f", "1.2 km", Dim::Length), Ok(1200.0));
        assert_eq!(life_seconds("f", "1.5 d"), Ok(DAY * 3 / 2));
        assert_eq!(life_seconds("f", "0.1 s"), Ok(0));
        // A tuning table's value says by its unit what it measures.
        assert_eq!(any_value("f", "1.2 m").unwrap(), (1.2, Dim::Length));
        assert_eq!(any_value("f", "100 mm").unwrap(), (0.1, Dim::Length));
        assert_eq!(any_value("f", "8 m3/s").unwrap(), (8.0, Dim::Flow));
        assert_eq!(any_value("f", "23.5 deg").unwrap(), (23.5, Dim::Angle));
    }

    // checks: TIM-18 WLD-30
    #[test]
    fn rejects_unknown_unit() {
        // Each failure names its field and says what is wrong.
        let fails = |r: Result<Amount, String>, want: &str| {
            let e = r.expect_err(want);
            assert!(e.starts_with("field `size`") && e.contains(want), "{e}");
        };
        fails(
            amount("size", "8 furlongs", Dim::Length),
            "no unit this field takes (mm, cm, m, km)",
        );
        fails(amount("size", "8 kg", Dim::Length), "no unit this field takes");
        fails(amount("size", "8 m", Dim::Mass), "no unit this field takes (g, kg)");
        fails(amount("size", "8", Dim::Length), "needs a number and a unit");
        fails(amount("size", "ten m", Dim::Length), "is not a number");
        fails(amount("size", "1e3 m", Dim::Length), "is not a number");
        fails(amount("size", "-3 m", Dim::Length), "is not a number");
        fails(
            amount("size", "30-8 cm", Dim::Length),
            "from its higher value to its lower",
        );
        fails(amount("size", "8 m3", Dim::Flow), "no unit this field takes (m3/s)");
        fails(amount("size", "9 F", Dim::Temperature), "no unit this field takes (C)");
        fails(amount("size", "23.5 rad", Dim::Angle), "no unit this field takes (deg)");
        let e = value("size", "8-30 cm", Dim::Length).unwrap_err();
        assert!(e.contains("one value, not a range"), "{e}");
        let e = any_value("size", "8 furlongs").unwrap_err();
        assert!(
            e.starts_with("field `size`") && e.contains("has no unit (mm, cm, m, km, g, kg"),
            "{e}"
        );
        assert!(any_value("size", "1-2 m").unwrap_err().contains("one value"));
        // Durations: an unknown unit, and one between two weeks and 85 days with no game length or a wrong one.
        let e = life_seconds("growth", "3 fortnights").unwrap_err();
        assert!(e.starts_with("field `growth`") && e.contains("no unit of time"), "{e}");
        let e = duration("growth", &DurSrc::Life("6 w".into())).unwrap_err();
        assert!(e.contains("needs its game length (TIM-18)"), "{e}");
        let e = duration(
            "growth",
            &DurSrc::Given {
                life: "6 w".into(),
                game: "3 d".into(),
            },
        )
        .unwrap_err();
        assert!(e.contains("outside 7 days to the life length"), "{e}");
        let e = duration(
            "growth",
            &DurSrc::Given {
                life: "2 y".into(),
                game: "200 d".into(),
            },
        )
        .unwrap_err();
        assert!(e.contains("by the rule"), "{e}");
        // Rates must name /d or /y, and their amount its unit.
        for bad in ["2/w", "2", "2 kg/d"] {
            let e = rate("births", bad, None).unwrap_err();
            assert!(e.starts_with("field `births`"), "{bad}: {e}");
        }
        assert!(rate("eats", "2/d", Some(Dim::Mass)).is_err());
        // A scaled value needs its Earth value, in the same kind.
        assert!(toml::from_str::<ScaledSrc>("value = \"50 km\"").is_err());
        let s = ScaledSrc {
            value: "50 km".into(),
            scaled_from: "1000 kg".into(),
        };
        let e = scaled("storm_size", &s, Dim::Length).unwrap_err();
        assert!(e.starts_with("field `storm_size.scaled_from`"), "{e}");
    }
}
