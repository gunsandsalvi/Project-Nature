//! The compiler's part for tuned numbers (`PRN-17`, A3.6): one entry a system, in `data/tuning/<system>.md` with
//! the system's name as its id. Besides the fields every entry has, each key holds a plain number, a value with its
//! unit (`"1.2 m"`), or a table of them whose keys join its own with a dot (`river_rating.bankfull_depth`).

use super::{Compiled, Entry, Head, Problem, at, head, parse};
use crate::tuning::{Measure, TuneValue, Tuning};
use crate::units::{Dim, any_value};

/// What an entry compiles to: its head, its human table and its entry.
type Out = Option<(Head, Vec<(String, String)>, Compiled)>;

fn measure(d: Dim) -> Measure {
    match d {
        Dim::Length => Measure::Length,
        Dim::Mass => Measure::Mass,
        Dim::Volume => Measure::Volume,
        Dim::Flow => Measure::Flow,
        Dim::Temperature => Measure::Temperature,
        Dim::Angle => Measure::Angle,
    }
}

/// A key as a tuning table may name it: snake_case, so a dot only ever joins a table's key to its own.
fn snake(k: &str) -> bool {
    k.chars().next().is_some_and(|c| c.is_ascii_lowercase())
        && k.chars()
            .all(|c| c.is_ascii_lowercase() || c.is_ascii_digit() || c == '_')
}

/// The tuned numbers under `t`, each with its dotted key and the text it was written as.
fn leaves(prefix: &str, t: &toml::Table, out: &mut Vec<(TuneValue, String)>, bad: &mut dyn FnMut(String)) {
    for (k, v) in t {
        let key = if prefix.is_empty() {
            k.clone()
        } else {
            format!("{prefix}.{k}")
        };
        if !snake(k) {
            bad(format!("key {key:?} is not snake_case"));
            continue;
        }
        let mut push = |value: f32, m: Measure, text: String| {
            out.push((
                TuneValue {
                    key: key.clone(),
                    value,
                    measure: m,
                },
                text,
            ));
        };
        match v {
            toml::Value::Integer(i) => push(*i as f32, Measure::Number, i.to_string()),
            toml::Value::Float(f) => push(*f as f32, Measure::Number, f.to_string()),
            toml::Value::String(s) => match any_value(&key, s) {
                Ok((value, dim)) => push(value, measure(dim), s.clone()),
                Err(e) => bad(e),
            },
            toml::Value::Table(t) => leaves(&key, t, out, bad),
            _ => bad(format!(
                "field `{key}`: a tuned number is a plain number, a value with its unit, or a table of them"
            )),
        }
    }
}

/// A head field taken out of the block: text, or a list of text for `checks`.
fn take(t: &mut toml::Table, field: &str) -> Result<toml::Value, String> {
    t.remove(field).ok_or_else(|| format!("missing field `{field}`"))
}

fn text(v: toml::Value, field: &str) -> Result<String, String> {
    match v {
        toml::Value::String(s) => Ok(s),
        _ => Err(format!("field `{field}` is text")),
    }
}

pub(super) fn tuning(e: &Entry, problems: &mut Vec<Problem>) -> Out {
    let mut t = parse::<toml::Table>(e, problems)?;
    let mut fail = |what: String| problems.push(at(&e.path, e.block_line, format!("{}: {what}", e.heading)));
    let head_of = |t: &mut toml::Table| -> Result<Head, String> {
        let id = text(take(t, "id")?, "id")?;
        let name = text(take(t, "name")?, "name")?;
        let stage = text(take(t, "stage")?, "stage")?;
        let checks = match take(t, "checks")? {
            toml::Value::Array(a) => a
                .into_iter()
                .map(|c| text(c, "checks"))
                .collect::<Result<Vec<_>, _>>()?,
            _ => return Err("field `checks` is a list of IDs".into()),
        };
        Ok(head(&id, &name, &stage, &checks))
    };
    let h = match head_of(&mut t) {
        Ok(h) => h,
        Err(why) => {
            fail(why);
            return None;
        }
    };
    // One entry a system: `data/tuning/world.md` holds `world`.
    let stem = e
        .path
        .rsplit('/')
        .next()
        .and_then(|f| f.strip_suffix(".md"))
        .unwrap_or("");
    if h.id != stem {
        fail(format!(
            "{} is in {}; a system's numbers are in data/tuning/{}.md",
            h.id, e.path, h.id
        ));
    }
    let mut out = Vec::new();
    leaves("", &t, &mut out, &mut fail);
    if out.is_empty() {
        fail(format!("{} holds no tuned number", h.id));
    }
    let table = out
        .iter()
        .map(|(v, text)| (format!("`{}`", v.key), text.clone()))
        .collect();
    let mut values: Vec<TuneValue> = out.into_iter().map(|(v, _)| v).collect();
    values.sort_by(|a, b| a.key.cmp(&b.key));
    let entry = Tuning {
        id: h.id.clone(),
        name: h.name.clone(),
        number: 0,
        values,
    };
    Some((h, table, Compiled::Tuning(entry)))
}
