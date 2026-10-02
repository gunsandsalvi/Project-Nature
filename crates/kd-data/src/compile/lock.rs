//! `data/ids.lock` (A3.6 step 3): permanent numbers per kind, one line `<kind> <id> <number>` each, `retired` appended
//! for a removed entry, whose number is never used again (`PLT-09`).

use crate::kinds::KindName;
use std::collections::BTreeMap;

/// The lock's first lines when the compiler creates it.
pub const HEADER: &str = "# Permanent numbers of catalogue entries (A3.6, PLT-09): `<kind> <id> <number>`, `retired` appended for a\n\
# removed entry. Written by `kd catalog build --assign`; a number is never changed or used again.\n";

/// One line of the lock.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct LockLine {
    pub kind: KindName,
    pub id: String,
    pub number: u32,
    pub retired: bool,
}

/// The lock's lines in file order, and its text.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct Lock {
    pub lines: Vec<LockLine>,
    pub text: String,
}

impl Lock {
    /// Parses the lock; each problem is `(line from 1, message)`.
    pub fn parse(text: &str) -> (Lock, Vec<(usize, String)>) {
        let mut lock = Lock {
            lines: Vec::new(),
            text: text.to_string(),
        };
        let mut problems = Vec::new();
        for (n, line) in text.lines().enumerate() {
            let t = line.trim();
            if t.is_empty() || t.starts_with('#') {
                continue;
            }
            let w: Vec<&str> = t.split_whitespace().collect();
            let parsed =
                match w.as_slice() {
                    [k, id, num] | [k, id, num, "retired"] => KindName::from_lock_name(k)
                        .zip(num.parse::<u32>().ok())
                        .map(|(kind, number)| LockLine {
                            kind,
                            id: id.to_string(),
                            number,
                            retired: w.len() == 4,
                        }),
                    _ => None,
                };
            match parsed {
                Some(l) => lock.lines.push(l),
                None => problems.push((n + 1, format!("`{t}` is not `<kind> <id> <number> [retired]`"))),
            }
        }
        (lock, problems)
    }

    /// The line for an id of a kind.
    pub fn find(&self, kind: KindName, id: &str) -> Option<&LockLine> {
        self.lines.iter().find(|l| l.kind == kind && l.id == id)
    }

    /// Problems among the lines themselves: an id locked twice, or a number held twice in a kind (a retired number
    /// reused, or two live ids on one number).
    pub fn problems(&self) -> Vec<(bool, String)> {
        let mut out = Vec::new();
        let mut seen_id: BTreeMap<(KindName, &str), u32> = BTreeMap::new();
        let mut seen_num: BTreeMap<(KindName, u32), &LockLine> = BTreeMap::new();
        for l in &self.lines {
            if seen_id.insert((l.kind, &l.id), l.number).is_some() {
                out.push((false, format!("{} `{}` is locked twice", l.kind.lock_name(), l.id)));
            }
            if let Some(first) = seen_num.insert((l.kind, l.number), l) {
                let retired = first.retired || l.retired;
                let what = if retired {
                    "a retired number reused"
                } else {
                    "one number for two ids"
                };
                out.push((
                    retired,
                    format!(
                        "{} number {} is held by `{}` and `{}`: {what}",
                        l.kind.lock_name(),
                        l.number,
                        first.id,
                        l.id
                    ),
                ));
            }
        }
        out
    }

    /// Adds an id with the next number of its kind (one past every number it has used, retired ones too).
    pub fn assign(&mut self, kind: KindName, id: &str) -> u32 {
        let number = self
            .lines
            .iter()
            .filter(|l| l.kind == kind)
            .map(|l| l.number + 1)
            .max()
            .unwrap_or(0);
        self.lines.push(LockLine {
            kind,
            id: id.to_string(),
            number,
            retired: false,
        });
        if self.text.is_empty() {
            self.text.push_str(HEADER);
        } else if !self.text.ends_with('\n') {
            self.text.push('\n');
        }
        self.text.push_str(&format!("{} {id} {number}\n", kind.lock_name()));
        number
    }
}
