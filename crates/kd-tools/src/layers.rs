//! `kd check layers` (A2.3, A3.2, A3.9, A15.1): every crate depends only on the workspace and outside crates its row
//! in `tools/layers.toml` allows, `unsafe` stays in the files its row allows, simulation crates (and `kd-view`) use
//! no `f64` outside their tests, and no feature but `kd-tools`' turns on `test-switches`.
//!
//! Implements PRN-14, see A2.3: modules stay separate because the layering is checked before every merge.

use std::collections::BTreeMap;
use std::path::{Path, PathBuf};

use crate::scan;

/// Outside crates every crate may use (A2.2).
pub const ALL_ALLOWED: [&str; 3] = ["libm", "serde", "log"];

/// A crate's row in `tools/layers.toml`.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Row {
    pub sim: bool,
    pub deps: Vec<String>,
    pub outside: Vec<String>,
    pub unsafe_code: Unsafe,
}

/// Where a crate may write `unsafe`.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Unsafe {
    None,
    Anywhere,
    Files(Vec<String>),
}

/// A workspace crate as `cargo metadata` declares it, with its Rust sources.
#[derive(Clone, Debug, Default)]
pub struct Crate {
    pub name: String,
    pub deps: Vec<Dep>,
    pub features: BTreeMap<String, Vec<String>>,
    /// Each source's path from the crate's folder, with `/` between parts, and its text.
    pub files: Vec<(String, String)>,
}

#[derive(Clone, Debug, Default)]
pub struct Dep {
    pub name: String,
    pub features: Vec<String>,
}

/// The rows of `tools/layers.toml`.
pub fn parse_rows(text: &str) -> Result<BTreeMap<String, Row>, String> {
    let table: toml::Table = toml::from_str(text).map_err(|e| format!("tools/layers.toml: {e}"))?;
    let mut rows = BTreeMap::new();
    for (name, v) in table {
        let t = v
            .as_table()
            .ok_or_else(|| format!("tools/layers.toml: {name} is not a table"))?;
        let list = |key: &str| -> Result<Vec<String>, String> {
            t.get(key)
                .and_then(|v| v.as_array())
                .ok_or_else(|| format!("tools/layers.toml: {name} has no list {key}"))?
                .iter()
                .map(|x| {
                    x.as_str()
                        .map(String::from)
                        .ok_or_else(|| format!("tools/layers.toml: {name}.{key}"))
                })
                .collect()
        };
        let unsafe_code = match t.get("unsafe") {
            Some(toml::Value::String(s)) if s == "none" => Unsafe::None,
            Some(toml::Value::String(s)) if s == "anywhere" => Unsafe::Anywhere,
            Some(toml::Value::Array(_)) => Unsafe::Files(list("unsafe")?),
            _ => {
                return Err(format!(
                    "tools/layers.toml: {name}.unsafe is \"none\", \"anywhere\" or a list of files"
                ));
            }
        };
        let sim = t
            .get("sim")
            .and_then(|v| v.as_bool())
            .ok_or_else(|| format!("tools/layers.toml: {name}.sim"))?;
        rows.insert(
            name.clone(),
            Row {
                sim,
                deps: list("deps")?,
                outside: list("outside")?,
                unsafe_code,
            },
        );
    }
    Ok(rows)
}

/// The workspace's crates from `cargo metadata --no-deps`, each with its folder.
pub fn parse_metadata(json: &str) -> Result<Vec<(Crate, PathBuf)>, String> {
    let v: serde_json::Value = serde_json::from_str(json).map_err(|e| format!("cargo metadata: {e}"))?;
    let packages = v["packages"].as_array().ok_or("cargo metadata: no packages")?;
    let strings = |v: &serde_json::Value| -> Vec<String> {
        v.as_array()
            .map(|a| a.iter().filter_map(|x| x.as_str().map(String::from)).collect())
            .unwrap_or_default()
    };
    let mut out = Vec::new();
    for p in packages {
        let name = p["name"].as_str().unwrap_or_default().to_string();
        let manifest = PathBuf::from(p["manifest_path"].as_str().unwrap_or_default());
        let deps = p["dependencies"]
            .as_array()
            .map(|a| {
                a.iter()
                    .map(|d| Dep {
                        name: d["name"].as_str().unwrap_or_default().to_string(),
                        features: strings(&d["features"]),
                    })
                    .collect()
            })
            .unwrap_or_default();
        let features = p["features"]
            .as_object()
            .map(|o| o.iter().map(|(k, v)| (k.clone(), strings(v))).collect())
            .unwrap_or_default();
        let dir = manifest.parent().map(Path::to_path_buf).unwrap_or_default();
        out.push((
            Crate {
                name,
                deps,
                features,
                files: Vec::new(),
            },
            dir,
        ));
    }
    Ok(out)
}

/// Every `.rs` file under a crate's folder but `target`, as (path from the folder, text).
pub fn read_sources(dir: &Path) -> Result<Vec<(String, String)>, String> {
    let mut out = Vec::new();
    let mut stack = vec![dir.to_path_buf()];
    while let Some(d) = stack.pop() {
        let entries = std::fs::read_dir(&d).map_err(|e| format!("{}: {e}", d.display()))?;
        for e in entries.flatten() {
            let path = e.path();
            if path.is_dir() {
                if path.file_name().is_some_and(|n| n != "target") {
                    stack.push(path);
                }
            } else if path.extension().is_some_and(|x| x == "rs") {
                let text = std::fs::read_to_string(&path).map_err(|e| format!("{}: {e}", path.display()))?;
                let rel = path
                    .strip_prefix(dir)
                    .unwrap_or(&path)
                    .to_string_lossy()
                    .replace('\\', "/");
                out.push((rel, text));
            }
        }
    }
    out.sort();
    Ok(out)
}

/// The files of a crate that hold only tests: under `tests/` or `benches/`, or a `#[cfg(test)] mod x;` module.
pub fn test_files(files: &[(String, String)]) -> Vec<String> {
    let mut out: Vec<String> = files
        .iter()
        .filter(|(p, _)| p.starts_with("tests/") || p.starts_with("benches/"))
        .map(|(p, _)| p.clone())
        .collect();
    for (path, text) in files {
        let (_, modules) = scan::test_parts(&scan::scan(text).code);
        let (dir, stem) = match path.rsplit_once('/') {
            Some((d, f)) => (d.to_string(), f.trim_end_matches(".rs").to_string()),
            None => (String::new(), path.trim_end_matches(".rs").to_string()),
        };
        let base = if ["lib", "main", "mod"].contains(&stem.as_str()) {
            dir
        } else {
            format!("{dir}/{stem}")
        };
        for m in modules {
            out.push(format!("{base}/{m}.rs"));
            out.push(format!("{base}/{m}/mod.rs"));
        }
    }
    out
}

/// Every breach of the layering rules, one line each, naming the crate, and the file and line where there is one.
pub fn check(rows: &BTreeMap<String, Row>, crates: &[Crate]) -> Vec<String> {
    let mut problems = Vec::new();
    let names: Vec<&str> = crates.iter().map(|c| c.name.as_str()).collect();
    for name in rows.keys() {
        if !names.contains(&name.as_str()) {
            problems.push(format!("{name}: a row in tools/layers.toml, but no such crate"));
        }
    }
    for c in crates {
        let Some(row) = rows.get(&c.name) else {
            problems.push(format!("{}: no row in tools/layers.toml", c.name));
            continue;
        };
        // The crates it names.
        for d in &c.deps {
            let workspace = rows.contains_key(&d.name);
            let allowed = if workspace {
                row.deps.contains(&d.name)
            } else {
                row.outside.contains(&d.name) || ALL_ALLOWED.contains(&d.name.as_str())
            };
            if !allowed {
                let kind = if workspace { "workspace crate" } else { "outside crate" };
                problems.push(format!(
                    "{}: depends on the {kind} {}, which its row does not allow",
                    c.name, d.name
                ));
            }
            if c.name != "kd-tools" && d.features.iter().any(|f| f == "test-switches") {
                problems.push(format!(
                    "{}: turns on {}'s test-switches, which only kd-tools may (A3.9)",
                    c.name, d.name
                ));
            }
        }
        // Only a crate's own test-switches feature may pass test-switches on; only kd-tools may turn it on.
        if c.name != "kd-tools" {
            for (feature, enables) in &c.features {
                if feature != "test-switches"
                    && enables
                        .iter()
                        .any(|e| e == "test-switches" || e.ends_with("/test-switches"))
                {
                    problems.push(format!(
                        "{}: feature {feature} turns on test-switches, which only kd-tools may (A3.9)",
                        c.name
                    ));
                }
            }
        }
        let tests = test_files(&c.files);
        for (path, text) in &c.files {
            let scanned = scan::scan(text);
            // unsafe, and allowing unsafe_code, outside the files its row allows.
            let unsafe_allowed = match &row.unsafe_code {
                Unsafe::Anywhere => true,
                Unsafe::None => false,
                Unsafe::Files(files) => files.contains(path),
            };
            if !unsafe_allowed {
                let words = scan::words(&scanned.code);
                for (k, &(at, w)) in words.iter().enumerate() {
                    let allows = w == "unsafe_code" && k > 0 && ["allow", "expect", "warn"].contains(&words[k - 1].1);
                    if w == "unsafe" || allows {
                        let line = scan::line_of(&scanned.code, at);
                        problems.push(format!(
                            "{}: {path}:{line}: {w} outside the files its row allows",
                            c.name
                        ));
                    }
                }
            }
            // f64 in a simulation crate's code outside its tests (A3.2).
            if row.sim && !tests.contains(path) {
                let (spans, _) = scan::test_parts(&scanned.code);
                let code = scan::without_tests(&scanned.code, &spans);
                for (at, w) in scan::words(&code) {
                    let literal =
                        w.starts_with(|ch: char| ch.is_ascii_digit()) && !w.starts_with("0x") && w.ends_with("f64");
                    if w == "f64" || literal {
                        let line = scan::line_of(&code, at);
                        problems.push(format!(
                            "{}: {path}:{line}: f64 in a simulation crate outside its tests (A3.2)",
                            c.name
                        ));
                    }
                }
            }
        }
    }
    problems
}

/// Runs the check on the repository at `root`.
pub fn run(root: &Path) -> Result<String, String> {
    let rows = parse_rows(&std::fs::read_to_string(root.join("tools/layers.toml")).map_err(|e| e.to_string())?)?;
    let out = std::process::Command::new("cargo")
        .args([
            "metadata",
            "--format-version",
            "1",
            "--no-deps",
            "--locked",
            "--offline",
        ])
        .current_dir(root)
        .output()
        .map_err(|e| format!("cargo metadata: {e}"))?;
    if !out.status.success() {
        return Err(format!("cargo metadata: {}", String::from_utf8_lossy(&out.stderr)));
    }
    let mut crates = Vec::new();
    let mut files = 0;
    for (mut c, dir) in parse_metadata(&String::from_utf8_lossy(&out.stdout))? {
        c.files = read_sources(&dir)?;
        files += c.files.len();
        crates.push(c);
    }
    let problems = check(&rows, &crates);
    if problems.is_empty() {
        Ok(format!("Layers: OK ({} crates, {files} files)", crates.len()))
    } else {
        Err(problems
            .iter()
            .map(|p| format!("Layers: {p}"))
            .collect::<Vec<_>>()
            .join("\n"))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const ROWS: &str = r#"
[kd-core]
sim = true
deps = []
outside = ["bytemuck"]
unsafe = "none"
[kd-data]
sim = true
deps = ["kd-core"]
outside = []
unsafe = "none"
[kd-sim]
sim = true
deps = ["kd-core", "kd-data"]
outside = []
unsafe = ["src/cluster/split.rs"]
[kd-app]
sim = false
deps = ["kd-core", "kd-sim"]
outside = []
unsafe = "anywhere"
[kd-tools]
sim = false
deps = ["kd-core", "kd-sim"]
outside = []
unsafe = "none"
"#;

    fn dep(name: &str) -> Dep {
        Dep {
            name: name.into(),
            features: Vec::new(),
        }
    }

    fn krate(name: &str, deps: &[&str], files: &[(&str, &str)]) -> Crate {
        Crate {
            name: name.into(),
            deps: deps.iter().map(|d| dep(d)).collect(),
            features: BTreeMap::new(),
            files: files.iter().map(|(p, t)| (p.to_string(), t.to_string())).collect(),
        }
    }

    fn clean() -> Vec<Crate> {
        vec![
            krate(
                "kd-core",
                &["bytemuck", "libm", "log"],
                &[("src/lib.rs", "#![deny(unsafe_code)]\npub fn f() {}")],
            ),
            krate("kd-data", &["kd-core", "serde"], &[]),
            krate(
                "kd-sim",
                &["kd-core", "kd-data"],
                &[("src/cluster/split.rs", "pub unsafe fn split() {}")],
            ),
            krate(
                "kd-app",
                &["kd-core", "kd-sim"],
                &[("src/lib.rs", "unsafe fn gl() {}\nfn x(a: f64) {}")],
            ),
            krate("kd-tools", &["kd-core", "kd-sim"], &[]),
        ]
    }

    // checks: PRN-14
    #[test]
    fn forbidden_edge_fails() {
        let rows = parse_rows(ROWS).expect("rows");
        assert_eq!(check(&rows, &clean()), Vec::<String>::new());
        let mut crates = clean();
        crates[0].deps.push(dep("kd-data"));
        crates[1].deps.push(dep("rayon"));
        let problems = check(&rows, &crates);
        assert_eq!(problems.len(), 2, "{problems:?}");
        assert!(problems[0].contains("kd-core: depends on the workspace crate kd-data"));
        assert!(problems[1].contains("kd-data: depends on the outside crate rayon"));
        // A crate with no row, and a row with no crate.
        let mut crates = clean();
        crates.push(krate("kd-new", &[], &[]));
        crates.remove(1);
        let problems = check(&rows, &crates);
        assert!(problems.iter().any(|p| p.contains("kd-new: no row")), "{problems:?}");
        assert!(
            problems
                .iter()
                .any(|p| p.contains("kd-data: a row in tools/layers.toml, but no such crate"))
        );
    }

    // checks: PRN-14
    #[test]
    fn unsafe_outside_allowed_fails() {
        let rows = parse_rows(ROWS).expect("rows");
        let mut crates = clean();
        // Words in comments and strings never count.
        crates[0].files.push((
            "src/a.rs".into(),
            "// unsafe\nfn a() -> &'static str { \"unsafe\" }".into(),
        ));
        assert_eq!(check(&rows, &crates), Vec::<String>::new());
        crates[0]
            .files
            .push(("src/b.rs".into(), "fn b() {\n    unsafe { }\n}".into()));
        crates[1]
            .files
            .push(("src/lib.rs".into(), "#![allow(unsafe_code)]".into()));
        crates[2]
            .files
            .push(("src/lib.rs".into(), "pub unsafe fn other() {}".into()));
        let problems = check(&rows, &crates);
        assert_eq!(
            problems,
            vec![
                "kd-core: src/b.rs:2: unsafe outside the files its row allows",
                "kd-data: src/lib.rs:1: unsafe_code outside the files its row allows",
                "kd-sim: src/lib.rs:1: unsafe outside the files its row allows",
            ]
        );
    }

    // checks: PRN-12 PRN-14
    #[test]
    fn test_switches_only_from_kd_tools() {
        let rows = parse_rows(ROWS).expect("rows");
        let mut crates = clean();
        // A crate's own test-switches may pass it on, and kd-tools may turn it on.
        crates[2]
            .features
            .insert("test-switches".into(), vec!["kd-core/test-switches".into()]);
        crates[4].deps[1].features = vec!["test-switches".into()];
        assert_eq!(check(&rows, &crates), Vec::<String>::new());
        // Anyone else turning it on fails: by a feature of its own, or in a dependency's features.
        crates[3]
            .features
            .insert("default".into(), vec!["kd-sim/test-switches".into()]);
        crates[3].deps[1].features = vec!["test-switches".into()];
        let problems = check(&rows, &crates);
        assert_eq!(problems.len(), 2, "{problems:?}");
        assert!(
            problems
                .iter()
                .all(|p| p.starts_with("kd-app:") && p.contains("test-switches"))
        );
    }

    // checks: RES-05 PRN-14
    #[test]
    fn f64_only_in_tests() {
        let rows = parse_rows(ROWS).expect("rows");
        let mut crates = clean();
        let tested =
            "pub fn a() -> f32 { 1.0 }\n#[cfg(test)]\nmod tests { fn t() -> f64 { 0.5f64 } }\n#[cfg(test)]\nmod more;";
        crates[1].files.push(("src/lib.rs".into(), tested.into()));
        crates[1]
            .files
            .push(("src/more.rs".into(), "fn m() -> f64 { 1.0 }".into()));
        crates[1]
            .files
            .push(("tests/stats.rs".into(), "fn s() -> f64 { 1.0 }".into()));
        crates[1]
            .files
            .push(("src/hex.rs".into(), "const H: u32 = 0x1f64;".into()));
        assert_eq!(check(&rows, &crates), Vec::<String>::new());
        crates[1].files.push((
            "src/bad.rs".into(),
            "fn b() {\n    let x = 2.0f64;\n    let y: f64 = 1.0;\n}".into(),
        ));
        let problems = check(&rows, &crates);
        assert_eq!(problems.len(), 2, "{problems:?}");
        assert!(problems[0].contains("kd-data: src/bad.rs:2: f64"));
        assert!(problems[1].contains("kd-data: src/bad.rs:3: f64"));
    }
}
