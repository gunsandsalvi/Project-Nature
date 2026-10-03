//! `kd check names` (A2.3 rule 5): rules never name content, so no catalogue id or English name appears as a
//! string in a simulation crate's code; only `data/`, tests and scenes name entries (`PRN-07`, `MAT-13`).
//! The catalogue arrives in α01a, which gives `catalogue_names` its list; until then the list is empty.
//!
//! Implements PRN-07, see A2.3: rules never name a species, an item or a blueprint.

use std::path::Path;

use crate::layers::{self, Crate, Row};
use crate::scan;
use std::collections::BTreeMap;

/// Every catalogue id and English name the simulation crates must not write: none until the catalogue (α01a).
pub fn catalogue_names(root: &Path) -> Vec<String> {
    let _ = root;
    Vec::new()
}

/// Every string literal in a simulation crate's code, outside its tests, that is a catalogue name.
pub fn check(names: &[String], rows: &BTreeMap<String, Row>, crates: &[Crate]) -> Vec<String> {
    let mut problems = Vec::new();
    for c in crates {
        if !rows.get(&c.name).is_some_and(|r| r.sim) {
            continue;
        }
        let tests = layers::test_files(&c.files);
        for (path, text) in &c.files {
            if tests.contains(path) {
                continue;
            }
            let scanned = scan::scan(text);
            let (spans, _) = scan::test_parts(&scanned.code);
            // A literal inside a test module starts on a line the module covers.
            let test_lines: Vec<(usize, usize)> = spans
                .iter()
                .map(|&(a, b)| (scan::line_of(&scanned.code, a), scan::line_of(&scanned.code, b)))
                .collect();
            for (line, s) in &scanned.strings {
                if test_lines.iter().any(|&(a, b)| (a..=b).contains(line)) {
                    continue;
                }
                if names.iter().any(|n| n == s) {
                    problems.push(format!(
                        "{}: {path}:{line}: the catalogue name \"{s}\" in code (A2.3 rule 5)",
                        c.name
                    ));
                }
            }
        }
    }
    problems
}

/// Runs the check on the repository at `root`.
pub fn run(root: &Path) -> Result<String, String> {
    let rows =
        layers::parse_rows(&std::fs::read_to_string(root.join("tools/layers.toml")).map_err(|e| e.to_string())?)?;
    let names = catalogue_names(root);
    let mut crates = Vec::new();
    for (name, row) in &rows {
        if row.sim {
            let dir = root.join("crates").join(name);
            crates.push(Crate {
                name: name.clone(),
                files: layers::read_sources(&dir)?,
                ..Crate::default()
            });
        }
    }
    let problems = check(&names, &rows, &crates);
    if problems.is_empty() {
        Ok(format!(
            "Names: OK ({} catalogue names, {} simulation crates)",
            names.len(),
            crates.len()
        ))
    } else {
        Err(problems
            .iter()
            .map(|p| format!("Names: {p}"))
            .collect::<Vec<_>>()
            .join("\n"))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRN-07
    #[test]
    fn content_names_in_code_fail() {
        let rows = layers::parse_rows(
            "[kd-life]\nsim = true\ndeps = []\noutside = []\nunsafe = \"none\"\n\
             [kd-ui]\nsim = false\ndeps = []\noutside = []\nunsafe = \"none\"\n",
        )
        .expect("rows");
        let names = vec!["red_deer".to_string(), "flint_nodule".to_string()];
        let file = |p: &str, t: &str| (p.to_string(), t.to_string());
        let life = Crate {
            name: "kd-life".into(),
            files: vec![
                file(
                    "src/lib.rs",
                    "// red_deer in a comment\nfn a() -> &'static str { \"deer\" }\n#[cfg(test)]\nmod tests { const D: &str = \"red_deer\"; }",
                ),
                file("tests/herds.rs", "const D: &str = \"red_deer\";"),
            ],
            ..Crate::default()
        };
        let ui = Crate {
            name: "kd-ui".into(),
            files: vec![file("src/lib.rs", "const D: &str = \"red_deer\";")],
            ..Crate::default()
        };
        assert_eq!(check(&names, &rows, &[life.clone(), ui.clone()]), Vec::<String>::new());
        let mut bad = life;
        bad.files
            .push(file("src/herd.rs", "fn h() {\n    let k = \"flint_nodule\";\n}"));
        assert_eq!(
            check(&names, &rows, &[bad, ui]),
            vec!["kd-life: src/herd.rs:2: the catalogue name \"flint_nodule\" in code (A2.3 rule 5)"]
        );
    }
}
