//! `kd check names` (A2.3 rule 5): rules never name content, so no catalogue id or English name appears as a
//! string in a simulation crate's code; only `data/`, tests and scenes name entries (`PRN-07`, `MAT-13`).
//! The names come from the catalogue, compiled from `data/` (α01a).
//!
//! Implements PRN-07, see A2.3: rules never name a species, an item or a blueprint.

use std::path::Path;

use crate::layers::{self, Crate, Row};
use crate::scan;
use std::collections::BTreeMap;

/// Every catalogue id and English name the simulation crates must not write.
pub fn catalogue_names(root: &Path) -> Result<Vec<String>, String> {
    crate::catalog::names(root)
}

/// Every string literal in a simulation crate's code, outside its tests and the code its tools feature alone builds
/// (the catalogue's compiler, which spells the schema's words, such as the kind `rock`), that is a catalogue name.
pub fn check(names: &[String], rows: &BTreeMap<String, Row>, crates: &[Crate]) -> Vec<String> {
    let mut problems = Vec::new();
    for c in crates {
        let Some(row) = rows.get(&c.name).filter(|r| r.sim) else {
            continue;
        };
        let tests = layers::test_files(&c.files);
        let tools = row
            .tools_feature
            .as_ref()
            .map(|f| layers::tools_files(&c.files, f))
            .unwrap_or_default();
        for (path, text) in &c.files {
            if tests.contains(path) || tools.contains(path) {
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
    let names = catalogue_names(root)?;
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

    // checks: PRN-07
    #[test]
    fn tools_code_may_spell_the_schema() {
        // The code a crate's tools feature alone builds may spell a schema word that is also an entry's id; the
        // rest of the crate may not, nor a module the feature does not gate.
        let rows = layers::parse_rows(
            "[kd-data]\nsim = true\ndeps = []\noutside = []\nunsafe = \"none\"\ntools_feature = \"compile\"\n",
        )
        .expect("rows");
        let names = vec!["rock".to_string()];
        let file = |p: &str, t: &str| (p.to_string(), t.to_string());
        let data = Crate {
            name: "kd-data".into(),
            files: vec![
                file(
                    "src/lib.rs",
                    "#[cfg(feature = \"compile\")]\npub mod compile;\n#[cfg(feature = \"compile\")]\npub mod kinds;\npub mod world;",
                ),
                file("src/compile.rs", "mod land;\nconst K: &str = \"rock\";"),
                file("src/compile/land.rs", "const F: &str = \"rock\";"),
                file("src/kinds.rs", "const N: &str = \"rock\";"),
                file("src/world.rs", "pub const W: u8 = 1;"),
            ],
            ..Crate::default()
        };
        assert_eq!(check(&names, &rows, std::slice::from_ref(&data)), Vec::<String>::new());
        let mut bad = data;
        bad.files[4].1 = "const W: &str = \"rock\";".into();
        assert_eq!(
            check(&names, &rows, &[bad]),
            vec!["kd-data: src/world.rs:1: the catalogue name \"rock\" in code (A2.3 rule 5)"]
        );
    }
}
