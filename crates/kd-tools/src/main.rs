//! kd-tools: the kd command line (A2.7, A15); implements PRN-14 in part (`kd check layers`), MAT-13, MAT-17 and PLT-09
//! in part (`kd catalog`, `kd check names`).
#![deny(unsafe_code)]

mod layers;
mod names;

use std::path::{Path, PathBuf};

const USAGE: &str = "Usage: kd <command>
  kd check layers                          the crate graph and the simulation sources against tools/layers.toml (A2.3)
  kd check names                           no simulation source names a catalogue entry (A2.3 rule 5)
  kd catalog build [--assign] [--out <f>]  compile data/ into the catalogue blob (A3.6); --assign numbers new ids
  kd catalog check                         validate data/ (A3.6, MAT-17)
  kd catalog tables                        rewrite each entry's generated table and data/INDEX.md (A3.6)
Later alphas add the other commands of A2.7.";

/// The workspace root: the nearest folder upward from here holding `tools/layers.toml`.
fn root() -> Option<PathBuf> {
    let mut dir = std::env::current_dir().ok()?;
    loop {
        if dir.join("tools/layers.toml").is_file() {
            return Some(dir);
        }
        if !dir.pop() {
            return None;
        }
    }
}

fn print(lines: Vec<String>, ok: bool) -> i32 {
    for l in lines {
        println!("{l}");
    }
    if ok { 0 } else { 1 }
}

fn errors(errs: Vec<kd_data::compile::CatalogueError>) -> i32 {
    for e in &errs {
        println!("{e}");
    }
    println!("Catalogue: {} problem(s)", errs.len());
    1
}

/// `kd catalog build`: compiles, optionally numbering new ids, and writes the blob (default `target/catalogue.bin`).
fn catalog_build(root: &Path, assign: bool, out: Option<&str>) -> i32 {
    match kd_data::compile::compile(&root.join("data"), assign) {
        Ok(c) => {
            let out = out
                .map(PathBuf::from)
                .unwrap_or_else(|| root.join("target/catalogue.bin"));
            if let Some(dir) = out.parent() {
                let _ = std::fs::create_dir_all(dir);
            }
            if let Err(e) = std::fs::write(&out, &c.blob) {
                println!("{}: {e}", out.display());
                return 1;
            }
            for id in &c.assigned {
                println!("assigned {id}");
            }
            println!(
                "Catalogue: {} entries, {} bytes, rules {} -> {}",
                c.entries.len(),
                c.blob.len(),
                c.catalogue.version_line(),
                out.display()
            );
            0
        }
        Err(errs) => errors(errs),
    }
}

fn catalog_check(root: &Path) -> i32 {
    match kd_data::compile::check(&root.join("data")) {
        Ok(c) => {
            println!(
                "Catalogue: OK ({} entries, {} bytes, rules {})",
                c.entries.len(),
                c.blob.len(),
                c.catalogue.version_line()
            );
            0
        }
        Err(errs) => errors(errs),
    }
}

fn catalog_tables(root: &Path) -> i32 {
    use kd_data::compile::{compile, tables, write_file};
    let data = root.join("data");
    let c = match compile(&data, false) {
        Ok(c) => c,
        Err(errs) => return errors(errs),
    };
    let files = match tables::rewritten(&data, &c.entries) {
        Ok(f) => f,
        Err(e) => {
            println!("{e}");
            return 1;
        }
    };
    let mut written = 0;
    for (file, text) in files
        .into_iter()
        .chain([("INDEX.md".to_string(), tables::index(&c.entries))])
    {
        let path = data.join(&file);
        if std::fs::read_to_string(&path).ok().as_deref() != Some(text.as_str()) {
            if let Err(e) = write_file(&path, &text) {
                println!("{e}");
                return 1;
            }
            println!("wrote data/{file}");
            written += 1;
        }
    }
    println!("Tables: {written} file(s) rewritten");
    0
}

fn main() {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let words: Vec<&str> = args.iter().map(String::as_str).collect();
    let Some(root) = root() else {
        eprintln!("kd: no tools/layers.toml here or above; run from the repository");
        std::process::exit(1);
    };
    let code = match words.as_slice() {
        ["check", "layers"] => {
            let (lines, ok) = layers::run(&root);
            print(lines, ok)
        }
        ["check", "names"] => {
            let (lines, ok) = names::run(&root);
            print(lines, ok)
        }
        ["catalog", "build", rest @ ..] => {
            let assign = rest.contains(&"--assign");
            let out = rest
                .iter()
                .position(|w| *w == "--out")
                .and_then(|i| rest.get(i + 1))
                .copied();
            let known = rest
                .iter()
                .enumerate()
                .all(|(i, w)| *w == "--assign" || *w == "--out" || (i > 0 && rest[i - 1] == "--out"));
            if known {
                catalog_build(&root, assign, out)
            } else {
                eprintln!("{USAGE}");
                2
            }
        }
        ["catalog", "check"] => catalog_check(&root),
        ["catalog", "tables"] => catalog_tables(&root),
        _ => {
            eprintln!("{USAGE}");
            2
        }
    };
    std::process::exit(code);
}
