//! `kd check names` (A2.3 rule 5): no string literal in a simulation crate's non-test source equals a catalogue entry's
//! `id` or `name`, so rules never name content (`PRN-07`, `MAT-13`); only `data/`, tests and scenes name entries.

use crate::layers;
use std::collections::BTreeSet;
use std::path::Path;

/// Every string literal's contents in a Rust source, with its line from 1; comments and character literals skipped.
/// Plain strings keep their escapes as written, which no id or name holds.
pub fn literals(text: &str) -> Vec<(usize, String)> {
    let c: Vec<char> = text.chars().collect();
    let mut out = Vec::new();
    let mut line = 1;
    let mut i = 0;
    let ident = |ch: char| ch.is_alphanumeric() || ch == '_';
    while i < c.len() {
        let next = c.get(i + 1).copied();
        if c[i] == '\n' {
            line += 1;
            i += 1;
        } else if c[i] == '/' && next == Some('/') {
            while i < c.len() && c[i] != '\n' {
                i += 1;
            }
        } else if c[i] == '/' && next == Some('*') {
            let mut depth = 0;
            while i < c.len() {
                if c[i] == '/' && c.get(i + 1) == Some(&'*') {
                    depth += 1;
                    i += 2;
                } else if c[i] == '*' && c.get(i + 1) == Some(&'/') {
                    depth -= 1;
                    i += 2;
                    if depth == 0 {
                        break;
                    }
                } else {
                    line += usize::from(c[i] == '\n');
                    i += 1;
                }
            }
        } else if c[i] == 'r' && (next == Some('"') || next == Some('#')) && (i == 0 || !ident(c[i - 1])) {
            // A raw string: r"..." or r#"..."#.
            let mut j = i + 1;
            let mut hashes = 0;
            while c.get(j) == Some(&'#') {
                hashes += 1;
                j += 1;
            }
            if c.get(j) != Some(&'"') {
                i += 1;
                continue;
            }
            let start_line = line;
            let mut s = String::new();
            j += 1;
            while j < c.len() {
                if c[j] == '"' && (1..=hashes).all(|h| c.get(j + h) == Some(&'#')) {
                    j += 1 + hashes;
                    break;
                }
                line += usize::from(c[j] == '\n');
                s.push(c[j]);
                j += 1;
            }
            out.push((start_line, s));
            i = j;
        } else if c[i] == '"' {
            let start_line = line;
            let mut s = String::new();
            i += 1;
            while i < c.len() && c[i] != '"' {
                if c[i] == '\\' && i + 1 < c.len() {
                    s.push(c[i]);
                    i += 1;
                }
                line += usize::from(c[i] == '\n');
                s.push(c[i]);
                i += 1;
            }
            i += 1;
            out.push((start_line, s));
        } else if c[i] == '\'' {
            // A character literal ('a', '\n', '\'') or a lifetime ('a): skip the literal whole.
            if next == Some('\\') {
                i += 2;
                while i < c.len() && c[i] != '\'' {
                    i += 1;
                }
                i += 1;
            } else if c.get(i + 2) == Some(&'\'') {
                i += 3;
            } else {
                i += 1;
            }
        } else {
            i += 1;
        }
    }
    out
}

/// The findings for one source: `path:line: "literal" names a catalogue entry`.
pub fn scan(path: &str, text: &str, names: &BTreeSet<String>) -> Vec<String> {
    literals(text)
        .into_iter()
        .filter(|(_, s)| names.contains(s))
        .map(|(line, s)| format!("{path}:{line}: \"{s}\" names a catalogue entry (A2.3 rule 5)"))
        .collect()
}

/// Runs the check from the workspace root over every crate `tools/layers.toml` marks `sim`.
/// Implements `MAT-13` in part, see A2.3 rule 5.
pub fn run(root: &Path) -> (Vec<String>, bool) {
    let compiled = match kd_data::compile::compile(&root.join("data"), false) {
        Ok(c) => c,
        Err(errs) => return (errs.iter().map(|e| e.to_string()).collect(), false),
    };
    let mut names = BTreeSet::new();
    for e in &compiled.entries {
        let c = e.parsed.common();
        names.insert(c.id.clone());
        names.insert(c.name.clone());
    }
    let rules = match std::fs::read_to_string(root.join(layers::RULES_FILE)).map(|t| layers::parse_rules(&t)) {
        Ok(Ok(r)) => r,
        Ok(Err(e)) => return (vec![e], false),
        Err(e) => return (vec![format!("{}: {e}", layers::RULES_FILE)], false),
    };
    let mut lines = Vec::new();
    let mut crates = 0;
    for (name, rule) in &rules {
        if !rule.sim {
            continue;
        }
        crates += 1;
        lines.extend(scan_crate(&root.join("crates").join(name), name, &names));
    }
    let ok = lines.is_empty();
    if ok {
        lines.push(format!("Names: OK ({crates} crates, {} names)", names.len()));
    }
    (lines, ok)
}

/// The findings for one crate's non-test sources.
pub fn scan_crate(dir: &Path, name: &str, names: &BTreeSet<String>) -> Vec<String> {
    let mut out = Vec::new();
    for rel in layers::sources(dir) {
        if let Ok(text) = std::fs::read_to_string(dir.join(&rel)) {
            out.extend(scan(&format!("{name}/{rel}"), &text, names));
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: MAT-13 PRN-14
    #[test]
    fn finds_a_named_id() {
        let dir = Path::new(env!("CARGO_MANIFEST_DIR")).join("tests/fixtures/names");
        let names: BTreeSet<String> = ["grass_damp", "Grass damp"].iter().map(|s| s.to_string()).collect();
        let found = scan_crate(&dir, "fixture", &names);
        assert_eq!(
            found,
            vec!["fixture/src/lib.rs:5: \"grass_damp\" names a catalogue entry (A2.3 rule 5)"]
        );
    }

    // checks: MAT-13
    #[test]
    fn literals_skip_comments_and_chars() {
        let src = "// \"grass\"\nlet a = 'x'; let b = \"one\";\n/* \"two\" */ let c = r#\"thr\"ee\"#;\nfn f<'a>(x: &'a str) {}\n";
        let got: Vec<String> = literals(src).into_iter().map(|(_, s)| s).collect();
        assert_eq!(got, vec!["one", "thr\"ee"]);
    }
}
