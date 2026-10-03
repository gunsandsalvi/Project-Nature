//! Tuned numbers (`PRN-17`, A3.6, `data/tuning/<system>.md`): one entry a system, each a named table of numbers,
//! plain or with units, nested tables named by dotted keys (`river_rating.bankfull_depth`). The game reads them by
//! key and the kind of value it expects, so a number in the wrong unit, or a missing one, is refused where it is
//! read, with the entry and the key named.
//!
//! Implements PRN-17, see A3.6: every tunable number in a catalogue, never inline in code.

use serde::{Deserialize, Serialize};

/// What a tuned number measures, in its base unit: a plain number, metres, kilograms, litres, cubic metres a
/// second, degrees Celsius or degrees of angle.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub enum Measure {
    Number,
    Length,
    Mass,
    Volume,
    Flow,
    Temperature,
    Angle,
}

/// One tuned number: its dotted key, its value in its base unit, and what it measures.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct TuneValue {
    pub key: String,
    pub value: f32,
    pub measure: Measure,
}

/// A system's tuned numbers (`data/tuning/<system>.md`), sorted by key.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Tuning {
    pub id: String,
    pub name: String,
    pub number: u16,
    pub values: Vec<TuneValue>,
}

impl Tuning {
    /// The value at `key`, which must measure `measure`.
    pub fn get(&self, key: &str, measure: Measure) -> Result<f32, String> {
        let v = self
            .values
            .binary_search_by(|v| v.key.as_str().cmp(key))
            .map(|i| &self.values[i])
            .map_err(|_| format!("tuning {}: no number {key}", self.id))?;
        if v.measure == measure {
            Ok(v.value)
        } else {
            Err(format!(
                "tuning {}: {key} is a {:?}, not a {measure:?}",
                self.id, v.measure
            ))
        }
    }

    /// A plain number at `key`.
    pub fn number(&self, key: &str) -> Result<f32, String> {
        self.get(key, Measure::Number)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRN-17
    #[test]
    fn read_by_key_and_measure() {
        let t = Tuning {
            id: "world".into(),
            name: "World".into(),
            number: 0,
            values: vec![
                TuneValue {
                    key: "river_rating.bankfull_depth".into(),
                    value: 1.2,
                    measure: Measure::Length,
                },
                TuneValue {
                    key: "settle_years".into(),
                    value: 10.0,
                    measure: Measure::Number,
                },
            ],
        };
        assert_eq!(t.number("settle_years"), Ok(10.0));
        assert_eq!(t.get("river_rating.bankfull_depth", Measure::Length), Ok(1.2));
        let e = t.number("river_rating.bankfull_depth").unwrap_err();
        assert!(e.contains("tuning world") && e.contains("not a Number"), "{e}");
        assert!(
            t.number("stream_min_km2")
                .unwrap_err()
                .contains("no number stream_min_km2")
        );
    }
}
