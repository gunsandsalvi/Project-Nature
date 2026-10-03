//! Human tables (A3.6): `kd catalog tables` writes each entry's two-column table from its block, between its markers,
//! and `data/INDEX.md`, one short table per kind; never edited by hand, and a stale one fails the check.

use super::source::{TABLE_END, TABLE_START};
use super::{Entry, Parsed};
use crate::kinds::{KINDS, KindName};
use crate::schema::{LightMethod, LightRole, LightTable};
use std::path::Path;

fn row(out: &mut String, label: &str, value: &str) {
    out.push_str(&format!("| {label} | {value} |\n"));
}

fn list(v: &[String]) -> String {
    v.join(", ")
}

/// `x × m + a`, written plainly.
fn affine(x: &str, m: &str, a: &str) -> String {
    match a.strip_prefix('-') {
        Some(neg) => format!("{x} × {m} − {neg}"),
        None => format!("{x} × {m} + {a}"),
    }
}

fn light_rows(out: &mut String, t: &LightTable) {
    let role = match t.role {
        LightRole::Table => "table: each colour to the nearest allowed colour after the change",
        LightRole::Version => "palette version: each colour changed, except those kept",
    };
    row(out, "Use", role);
    let zero = String::from("0");
    let change = match t.method {
        LightMethod::Same => "none".to_string(),
        LightMethod::Warm => format!("firelight's warming, k {}", t.k.as_ref().unwrap_or(&zero)),
        LightMethod::Haze => {
            let [r, g, b] = t.toward.unwrap_or_default();
            format!("toward ({r}, {g}, {b}) by {}", t.amount.as_ref().unwrap_or(&zero))
        }
        LightMethod::Lab => {
            let (m, a) = (t.mul.clone().unwrap_or_default(), t.add.clone().unwrap_or_default());
            let mut s = format!("OKLab: L {}", affine("L", &m[0], &a[0]));
            if let Some(lo) = &t.l_min {
                s.push_str(&format!(", at least {lo}"));
            }
            if let Some(hi) = &t.l_max {
                s.push_str(&format!(", at most {hi}"));
            }
            s.push_str(&format!(
                "; a {}; b {}",
                affine("a", &m[1], &a[1]),
                affine("b", &m[2], &a[2])
            ));
            s
        }
    };
    row(out, "Change", &change);
    match t.role {
        LightRole::Table => {
            if !t.only_families.is_empty() {
                row(out, "Gives only", &list(&t.only_families));
            }
            if !t.exclude_families.is_empty() {
                row(out, "Never gives families", &list(&t.exclude_families));
            }
            if !t.exclude_colours.is_empty() {
                row(out, "Never gives colours", &list(&t.exclude_colours));
            }
        }
        LightRole::Version => {
            let mut kept = t.keep_families.clone();
            kept.extend(t.keep_colours.iter().cloned());
            if !kept.is_empty() {
                row(out, "Kept unchanged", &list(&kept));
            }
        }
    }
}

/// An entry's generated table, both markers included, without a final newline.
pub fn table(e: &Entry) -> String {
    let mut out = format!("{TABLE_START}\n| | |\n|---|---|\n");
    match &e.parsed {
        Parsed::ColourFamily(f) => {
            let cs: Vec<String> = f.colours.iter().map(|(n, h)| format!("{n} `{h}`")).collect();
            row(&mut out, "Colours", &cs.join(", "));
        }
        Parsed::Ladder(l) => {
            row(&mut out, "Steps, dark to light", &list(&l.steps));
            row(&mut out, "Length", &l.steps.len().to_string());
        }
        Parsed::LightTable(t) => light_rows(&mut out, t),
        Parsed::Surface(s) => {
            row(&mut out, "Ladder", &s.ladder);
            row(
                &mut out,
                "Stones",
                &format!("share {}, about {} m across", s.stone_density, s.stone_size),
            );
            row(&mut out, "Tufts", &format!("share {}", s.tuft_density));
            if !s.flags.is_empty() {
                let f: Vec<String> = s.flags.iter().map(|f| format!("{f:?}").to_lowercase()).collect();
                row(&mut out, "Marks", &list(&f));
            }
        }
    }
    out.push_str(TABLE_END);
    out
}

/// Every claimed file's text with its tables rewritten from the blocks; a missing table goes just before the block.
pub fn rewritten(root: &Path, entries: &[Entry]) -> Result<Vec<(String, String)>, String> {
    let mut files: Vec<&str> = entries.iter().map(|e| e.file.as_str()).collect();
    files.sort();
    files.dedup();
    let mut out = Vec::new();
    for file in files {
        let text = std::fs::read_to_string(root.join(file)).map_err(|e| format!("{file}: {e}"))?;
        let lines: Vec<&str> = text.split('\n').collect();
        let mut mine: Vec<&Entry> = entries.iter().filter(|e| e.file == file).collect();
        mine.sort_by_key(|e| e.raw.line);
        let mut new: Vec<String> = Vec::with_capacity(lines.len());
        let mut at = 0;
        for e in mine {
            let (from, to, blank) = match (e.raw.table, e.raw.block_line) {
                (Some((a, b)), _) => (a, b, false),
                (None, Some(b)) => (b, b, true),
                (None, None) => continue,
            };
            new.extend(lines[at..from].iter().map(|s| s.to_string()));
            new.push(table(e));
            if blank {
                new.push(String::new());
            }
            at = to;
        }
        new.extend(lines[at..].iter().map(|s| s.to_string()));
        out.push((file.to_string(), new.join("\n")));
    }
    Ok(out)
}

/// `data/INDEX.md`: one short table per kind, entries in number order.
pub fn index(entries: &[Entry]) -> String {
    let mut out = String::from(
        "# Catalogue index\n\nGenerated by `kd catalog tables` from the entries' blocks (A3.6); edit the entries, not this file.\n",
    );
    let mut kinds: Vec<(&str, KindName)> = KINDS.to_vec();
    kinds.dedup_by_key(|(_, k)| *k);
    for (path, kind) in kinds {
        out.push_str(&format!(
            "\n## {} (`data/{path}`)\n\n| Number | Id | Name | Stage |\n|---|---|---|---|\n",
            kind.title()
        ));
        for e in entries.iter().filter(|e| e.kind == kind) {
            let c = e.parsed.common();
            out.push_str(&format!("| {} | `{}` | {} | {} |\n", e.number, c.id, c.name, c.stage));
        }
    }
    out
}
