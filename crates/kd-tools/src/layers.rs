//! `kd check layers`: the crate graph and the simulation sources against `tools/layers.toml` (A2.3, A2.7, A3.2, A3.9).

use serde_json::Value;
use std::collections::{BTreeMap, BTreeSet};
use std::path::{Path, PathBuf};

/// The feature that switches mechanisms off for tests (`RES-10`, A3.9); play builds never hold it.
const SWITCHES: &str = "test-switches";
/// The one crate whose normal dependencies may turn the switches on (A3.9).
const SWITCH_HOLDER: &str = "kd-tools";
/// Where the rules live, relative to the workspace root.
const RULES_FILE: &str = "tools/layers.toml";

/// One crate's table in `tools/layers.toml`.
#[derive(Debug, Default)]
pub struct CrateRule {
    pub sim: bool,
    pub deps: BTreeSet<String>,
    pub outside: BTreeSet<String>,
    pub unsafe_files: BTreeSet<String>,
}

/// Every crate's rule, by package name.
pub type Rules = BTreeMap<String, CrateRule>;

/// Reads `tools/layers.toml`.
pub fn parse_rules(text: &str) -> Result<Rules, String> {
    let table: toml::Table = text.parse().map_err(|e| format!("{RULES_FILE}: {e}"))?;
    let mut rules = Rules::new();
    for (name, v) in &table {
        let t = v
            .as_table()
            .ok_or_else(|| format!("{RULES_FILE}: [{name}] is not a table"))?;
        let list = |key: &str| -> Result<BTreeSet<String>, String> {
            match t.get(key) {
                None => Ok(BTreeSet::new()),
                Some(toml::Value::Array(a)) => a
                    .iter()
                    .map(|x| {
                        x.as_str()
                            .map(str::to_owned)
                            .ok_or_else(|| format!("{RULES_FILE}: [{name}] {key}"))
                    })
                    .collect(),
                Some(_) => Err(format!("{RULES_FILE}: [{name}] {key} is not a list")),
            }
        };
        let sim = t
            .get("sim")
            .and_then(toml::Value::as_bool)
            .ok_or_else(|| format!("{RULES_FILE}: [{name}] needs sim = true or false"))?;
        let rule = CrateRule {
            sim,
            deps: list("deps")?,
            outside: list("outside")?,
            unsafe_files: list("unsafe_files")?,
        };
        rules.insert(name.clone(), rule);
    }
    Ok(rules)
}

/// Every breach of the crate graph in `cargo metadata --no-deps` output: workspace edges not in `deps`,
/// outside crates not in `outside`, and `test-switches` turned on by a normal or build dependency outside `kd-tools`.
/// Dev-dependencies are free, so a crate's own tests may switch a mechanism off.
/// Implements `PRN-14`, see A2.3 and A3.9.
pub fn check_graph(meta: &Value, rules: &Rules) -> Vec<String> {
    let mut breaches = Vec::new();
    let packages = meta["packages"].as_array().map(Vec::as_slice).unwrap_or(&[]);
    let workspace: BTreeSet<&str> = packages.iter().filter_map(|p| p["name"].as_str()).collect();
    for name in rules.keys() {
        if !workspace.contains(name.as_str()) {
            breaches.push(format!("{RULES_FILE} names {name}, which is not a workspace crate"));
        }
    }
    let mut sorted: Vec<&Value> = packages.iter().collect();
    sorted.sort_by_key(|p| p["name"].as_str().unwrap_or(""));
    for p in sorted {
        let name = p["name"].as_str().unwrap_or("?");
        let Some(rule) = rules.get(name) else {
            breaches.push(format!("{name}: no table in {RULES_FILE}"));
            continue;
        };
        for d in p["dependencies"].as_array().map(Vec::as_slice).unwrap_or(&[]) {
            if d["kind"].as_str() == Some("dev") {
                continue;
            }
            let dep = d["name"].as_str().unwrap_or("?");
            if workspace.contains(dep) {
                if !rule.deps.contains(dep) {
                    breaches.push(format!("{name} -> {dep}: not allowed ({RULES_FILE})"));
                }
            } else if !rule.outside.contains(dep) {
                breaches.push(format!("{name} -> {dep}: outside crate not allowed"));
            }
            let features = d["features"].as_array().map(Vec::as_slice).unwrap_or(&[]);
            if name != SWITCH_HOLDER && features.iter().any(|f| f.as_str() == Some(SWITCHES)) {
                breaches.push(format!(
                    "{name} -> {dep}: turns on {SWITCHES}; only {SWITCH_HOLDER} may (A3.9)"
                ));
            }
        }
        if name != SWITCH_HOLDER && default_turns_on_switches(&p["features"]) {
            breaches.push(format!("{name}: its default features turn on {SWITCHES} (A3.9)"));
        }
    }
    breaches
}

/// Whether a package's `default` feature reaches `test-switches`, its own or a dependency's.
fn default_turns_on_switches(features: &Value) -> bool {
    let Some(map) = features.as_object() else { return false };
    let mut seen = BTreeSet::new();
    let mut todo = vec!["default".to_owned()];
    while let Some(f) = todo.pop() {
        if !seen.insert(f.clone()) {
            continue;
        }
        if f == SWITCHES || f.ends_with(&format!("/{SWITCHES}")) {
            return true;
        }
        for x in map.get(&f).and_then(Value::as_array).map(Vec::as_slice).unwrap_or(&[]) {
            if let Some(s) = x.as_str() {
                todo.push(s.to_owned());
            }
        }
    }
    false
}

/// What a source scan flags.
#[derive(Debug, PartialEq, Eq)]
pub enum Finding {
    F64,
    Unsafe,
}

/// Flags, by line number from 1, the type `f64` (A3.2) and, unless `allow_unsafe`, the keyword `unsafe` (A2.3)
/// in one Rust source, with comments and the insides of string and character literals left out.
/// Implements `PRN-14`, see A2.3 and A3.2.
pub fn scan_source(text: &str, allow_unsafe: bool) -> Vec<(usize, Finding)> {
    let code = blank_comments_and_literals(text);
    let mut found = Vec::new();
    for (i, line) in code.lines().enumerate() {
        for token in line.split(|c: char| !(c.is_ascii_alphanumeric() || c == '_')) {
            let numeric = token.starts_with(|c: char| c.is_ascii_digit()) && !token.starts_with("0x");
            if token == "f64" || (numeric && token.ends_with("f64")) {
                found.push((i + 1, Finding::F64));
            } else if token == "unsafe" && !allow_unsafe {
                found.push((i + 1, Finding::Unsafe));
            }
        }
    }
    found
}

/// The source with every comment and the inside of every string and character literal turned to spaces;
/// line breaks stay, so line numbers hold.
fn blank_comments_and_literals(text: &str) -> String {
    let c: Vec<char> = text.chars().collect();
    let mut out = String::with_capacity(text.len());
    let ident = |ch: char| ch.is_alphanumeric() || ch == '_';
    let blank = |out: &mut String, ch: char| out.push(if ch == '\n' { '\n' } else { ' ' });
    let mut i = 0;
    while i < c.len() {
        let next = c.get(i + 1).copied();
        if c[i] == '/' && next == Some('/') {
            while i < c.len() && c[i] != '\n' {
                blank(&mut out, c[i]);
                i += 1;
            }
        } else if c[i] == '/' && next == Some('*') {
            let mut depth = 0;
            while i < c.len() {
                if c[i] == '/' && c.get(i + 1) == Some(&'*') {
                    depth += 1;
                    out.push_str("  ");
                    i += 2;
                } else if c[i] == '*' && c.get(i + 1) == Some(&'/') {
                    depth -= 1;
                    out.push_str("  ");
                    i += 2;
                    if depth == 0 {
                        break;
                    }
                } else {
                    blank(&mut out, c[i]);
                    i += 1;
                }
            }
        } else if c[i] == 'r' && (i == 0 || !ident(c[i - 1]) || (c[i - 1] == 'b' && (i < 2 || !ident(c[i - 2])))) && {
            let mut j = i + 1;
            while c.get(j) == Some(&'#') {
                j += 1;
            }
            c.get(j) == Some(&'"')
        } {
            // A raw string: r"..." or r#"..."#, ending at a quote followed by as many #.
            let mut j = i + 1;
            while c[j] == '#' {
                j += 1;
            }
            let hashes = j - i - 1;
            out.extend(std::iter::repeat_n(' ', j + 1 - i));
            i = j + 1;
            while i < c.len() {
                if c[i] == '"' && (0..hashes).all(|k| c.get(i + 1 + k) == Some(&'#')) {
                    out.extend(std::iter::repeat_n(' ', hashes + 1));
                    i += hashes + 1;
                    break;
                }
                blank(&mut out, c[i]);
                i += 1;
            }
        } else if c[i] == '"' {
            out.push(' ');
            i += 1;
            while i < c.len() && c[i] != '"' {
                if c[i] == '\\' && i + 1 < c.len() {
                    blank(&mut out, c[i]);
                    i += 1;
                }
                blank(&mut out, c[i]);
                i += 1;
            }
            out.push(' ');
            i += 1;
        } else if c[i] == '\'' && (next == Some('\\') || c.get(i + 2) == Some(&'\'')) {
            // A character literal ('x', '\n', '\u{..}'); a lifetime has no closing quote two along.
            out.push(' ');
            i += 1;
            while i < c.len() && c[i] != '\'' {
                if c[i] == '\\' {
                    out.push(' ');
                    i += 1;
                }
                if i < c.len() {
                    blank(&mut out, c[i]);
                    i += 1;
                }
            }
            out.push(' ');
            i += 1;
        } else {
            out.push(c[i]);
            i += 1;
        }
    }
    out
}

/// Every `.rs` file of a crate outside `tests/` folders and `tests.rs` files, as paths relative to the crate, sorted.
fn sources(crate_dir: &Path) -> Vec<String> {
    fn walk(dir: &Path, rel: &str, out: &mut Vec<String>) {
        let Ok(entries) = std::fs::read_dir(dir) else { return };
        let mut names: Vec<(String, PathBuf)> = entries
            .flatten()
            .map(|e| (e.file_name().to_string_lossy().into_owned(), e.path()))
            .collect();
        names.sort();
        for (name, path) in names {
            let r = if rel.is_empty() {
                name.clone()
            } else {
                format!("{rel}/{name}")
            };
            if path.is_dir() {
                if name != "tests" && name != "target" {
                    walk(&path, &r, out);
                }
            } else if name.ends_with(".rs") && name != "tests.rs" {
                out.push(r);
            }
        }
    }
    let mut out = Vec::new();
    walk(crate_dir, "", &mut out);
    out
}

/// Runs the whole check from the workspace root: `cargo metadata`, the graph, then the simulation sources.
/// Returns the lines to print and whether it passed.
pub fn run(root: &Path) -> (Vec<String>, bool) {
    let text = match std::fs::read_to_string(root.join(RULES_FILE)) {
        Ok(t) => t,
        Err(e) => return (vec![format!("{RULES_FILE}: {e}")], false),
    };
    let rules = match parse_rules(&text) {
        Ok(r) => r,
        Err(e) => return (vec![e], false),
    };
    let cargo = std::env::var("CARGO").unwrap_or_else(|_| "cargo".to_owned());
    let output = std::process::Command::new(cargo)
        .args(["metadata", "--format-version", "1", "--no-deps", "--locked"])
        .current_dir(root)
        .output();
    let meta: Value = match output {
        Ok(o) if o.status.success() => match serde_json::from_slice(&o.stdout) {
            Ok(v) => v,
            Err(e) => return (vec![format!("cargo metadata: {e}")], false),
        },
        Ok(o) => {
            return (
                vec![format!("cargo metadata: {}", String::from_utf8_lossy(&o.stderr).trim())],
                false,
            );
        }
        Err(e) => return (vec![format!("cargo metadata: {e}")], false),
    };
    let mut lines = check_graph(&meta, &rules);
    let packages = meta["packages"].as_array().map(Vec::as_slice).unwrap_or(&[]);
    for p in packages {
        let name = p["name"].as_str().unwrap_or("?");
        let (Some(rule), Some(manifest)) = (rules.get(name), p["manifest_path"].as_str()) else {
            continue;
        };
        if !rule.sim {
            continue;
        }
        let dir = Path::new(manifest).parent().unwrap_or(root);
        for rel in sources(dir) {
            let Ok(src) = std::fs::read_to_string(dir.join(&rel)) else {
                lines.push(format!("{name}/{rel}: unreadable"));
                continue;
            };
            for (line, what) in scan_source(&src, rule.unsafe_files.contains(&rel)) {
                lines.push(match what {
                    Finding::F64 => format!("{name}/{rel}:{line}: f64 in a simulation crate (A3.2)"),
                    Finding::Unsafe => format!("{name}/{rel}:{line}: unsafe outside unsafe_files ({RULES_FILE})"),
                });
            }
        }
    }
    let ok = lines.is_empty();
    lines.push(if ok {
        format!("Layers: OK ({} crates)", packages.len())
    } else {
        "Layers: FAIL".to_owned()
    });
    (lines, ok)
}

#[cfg(test)]
mod tests {
    use super::*;

    const RULES: &str = r#"
[kd-core]
sim = true
deps = []
outside = ["libm"]

[kd-sim]
sim = true
deps = ["kd-core"]
outside = []

[kd-app]
sim = false
deps = ["kd-core", "kd-sim"]
outside = []

[kd-tools]
sim = false
deps = ["kd-core", "kd-sim"]
outside = []
"#;

    fn dep(name: &str, kind: Option<&str>, features: &[&str]) -> Value {
        serde_json::json!({ "name": name, "kind": kind, "target": null, "features": features })
    }

    fn meta(packages: Vec<(&str, Vec<Value>)>) -> Value {
        let ps: Vec<Value> = packages
            .into_iter()
            .map(|(n, deps)| serde_json::json!({ "name": n, "dependencies": deps, "features": {} }))
            .collect();
        serde_json::json!({ "packages": ps })
    }

    fn clean() -> Vec<(&'static str, Vec<Value>)> {
        vec![
            ("kd-core", vec![dep("libm", None, &[])]),
            ("kd-sim", vec![dep("kd-core", None, &[])]),
            ("kd-app", vec![dep("kd-sim", None, &[]), dep("kd-core", None, &[])]),
            ("kd-tools", vec![dep("kd-sim", None, &[])]),
        ]
    }

    // checks: PRN-14
    #[test]
    fn forbidden_edge_fails() {
        let rules = parse_rules(RULES).unwrap();
        assert_eq!(check_graph(&meta(clean()), &rules), Vec::<String>::new());
        let mut ps = clean();
        ps[0].1.push(dep("kd-sim", None, &[]));
        assert_eq!(
            check_graph(&meta(ps), &rules),
            vec!["kd-core -> kd-sim: not allowed (tools/layers.toml)"]
        );
        let mut ps = clean();
        ps[1].1.push(dep("png", Some("build"), &[]));
        assert_eq!(
            check_graph(&meta(ps), &rules),
            vec!["kd-sim -> png: outside crate not allowed"]
        );
        // A dev-dependency is free.
        let mut ps = clean();
        ps[0].1.push(dep("kd-sim", Some("dev"), &[]));
        assert_eq!(check_graph(&meta(ps), &rules), Vec::<String>::new());
    }

    // checks: PRN-14
    #[test]
    fn f64_in_sim_source_fails() {
        assert_eq!(
            scan_source("pub fn x(a: f64) -> f32 { a as f32 }\n", false),
            vec![(1, Finding::F64)]
        );
        assert_eq!(scan_source("let y = 1.0f64;\n", false), vec![(1, Finding::F64)]);
        assert_eq!(
            scan_source("fn a() {}\nunsafe fn b() {}\n", false),
            vec![(2, Finding::Unsafe)]
        );
        assert_eq!(scan_source("unsafe fn b() {}\n", true), vec![]);
        // Comments, strings, names containing the letters, lifetimes and hex literals are not the type.
        let fine = "// an f64 here\n/* f64 /* nested unsafe */ f64 */\nlet s = \"f64 unsafe\";\nlet r = r#\"f64\"#;\n\
                    let c = '\"'; let f64_count = 0x1f64; fn g<'a>(x: &'a str) {}\n#![deny(unsafe_code)]\n";
        assert_eq!(scan_source(fine, false), vec![]);
    }

    // checks: PRN-14
    #[test]
    fn test_switches_only_from_kd_tools() {
        let rules = parse_rules(RULES).unwrap();
        let mut ps = clean();
        ps[2].1[1] = dep("kd-core", None, &["test-switches"]);
        assert_eq!(
            check_graph(&meta(ps), &rules),
            vec!["kd-app -> kd-core: turns on test-switches; only kd-tools may (A3.9)"]
        );
        let mut ps = clean();
        ps[3].1.push(dep("kd-core", None, &["test-switches"]));
        ps[1].1.push(dep("kd-core", Some("dev"), &["test-switches"]));
        assert_eq!(check_graph(&meta(ps), &rules), Vec::<String>::new());
        let mut m = meta(clean());
        m["packages"][1]["features"] = serde_json::json!({ "default": ["play"], "play": ["kd-core/test-switches"] });
        assert_eq!(
            check_graph(&m, &rules),
            vec!["kd-sim: its default features turn on test-switches (A3.9)"]
        );
    }
}
