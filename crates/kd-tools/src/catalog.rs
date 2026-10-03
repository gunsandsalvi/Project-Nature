//! `kd catalog build [--assign] | check | tables` (A3.6): the catalogue compiled from `data/`, checked before every
//! merge (A15.12 step 4), and its generated tables and index written.

use std::path::{Path, PathBuf};

use kd_data::compile::{self, Output, Source};

/// Every file under `data/`, with its path from the repository's root.
pub fn read_data(root: &Path) -> Result<Vec<Source>, String> {
    let mut out = Vec::new();
    let mut stack = vec![root.join("data")];
    while let Some(dir) = stack.pop() {
        let Ok(entries) = std::fs::read_dir(&dir) else { continue };
        for e in entries.flatten() {
            let path = e.path();
            if path.is_dir() {
                stack.push(path);
            } else if let Ok(text) = std::fs::read_to_string(&path) {
                let rel = path
                    .strip_prefix(root)
                    .unwrap_or(&path)
                    .to_string_lossy()
                    .replace('\\', "/");
                out.push(Source { path: rel, text });
            }
        }
    }
    out.sort_by(|a, b| a.path.cmp(&b.path));
    Ok(out)
}

fn compiled(root: &Path, assign: bool) -> Result<Output, String> {
    let sources = read_data(root)?;
    compile::compile(&sources, assign).map_err(|problems| {
        problems
            .iter()
            .map(|p| format!("Catalogue: {p}"))
            .collect::<Vec<_>>()
            .join("\n")
    })
}

fn summary(out: &Output) -> String {
    let v = out.catalogue.versions;
    let hash = kd_data::Catalogue::hash_of(&out.blob).unwrap_or(0);
    format!(
        "{} entries, rules {}.{}, generator {}, blob {} bytes, hash {hash:016x}",
        out.entries,
        v.major,
        v.minor,
        v.generator,
        out.blob.len()
    )
}

fn write(root: &Path, rel: &str, text: &[u8]) -> Result<(), String> {
    let path: PathBuf = root.join(rel);
    if let Some(dir) = path.parent() {
        std::fs::create_dir_all(dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    }
    std::fs::write(&path, text).map_err(|e| format!("{}: {e}", path.display()))
}

/// Compiles; with `assign`, gives new entries their numbers in `data/ids.lock`. The blob goes to
/// `target/catalogue/catalogue.kdcat`; stale tables are reported, and written by `kd catalog tables`.
pub fn build(root: &Path, assign: bool) -> Result<String, String> {
    let out = compiled(root, assign)?;
    let old = std::fs::read_to_string(root.join(compile::LOCK_FILE)).unwrap_or_default();
    if assign && out.lock != old {
        write(root, compile::LOCK_FILE, out.lock.as_bytes())?;
    }
    write(root, "target/catalogue/catalogue.kdcat", &out.blob)?;
    for (path, _) in &out.stale {
        eprintln!("Catalogue: {path}: its generated tables or index are stale: kd catalog tables");
    }
    Ok(format!("Catalogue: built, {}", summary(&out)))
}

/// The check before a merge (A15.12 step 4): the catalogue compiles with every entry numbered, `data/ids.lock`
/// reads as the compiler writes it, and no generated table or index is stale.
pub fn check(root: &Path) -> Result<String, String> {
    let out = compiled(root, false)?;
    let mut problems: Vec<String> = out
        .stale
        .iter()
        .map(|(path, _)| format!("Catalogue: {path}: its generated tables or index are stale: kd catalog tables"))
        .collect();
    if std::fs::read_to_string(root.join(compile::LOCK_FILE)).unwrap_or_default() != out.lock {
        problems.push(format!(
            "Catalogue: {} is not as kd catalog build writes it",
            compile::LOCK_FILE
        ));
    }
    if problems.is_empty() {
        Ok(format!("Catalogue: OK ({})", summary(&out)))
    } else {
        Err(problems.join("\n"))
    }
}

/// Writes every stale generated table and the index.
pub fn tables(root: &Path) -> Result<String, String> {
    let out = compiled(root, false)?;
    for (path, text) in &out.stale {
        write(root, path, text.as_bytes())?;
    }
    Ok(format!("Tables: {} files written", out.stale.len()))
}

/// Every catalogue id and English name, for `kd check names`: every kind's but the tuned numbers', which each system
/// reads by its own table's name (A11.13 rule 6, `PRN-17`).
pub fn names(root: &Path) -> Result<Vec<String>, String> {
    let cat = compiled(root, false)?.catalogue;
    let mut out: Vec<String> = Vec::new();
    for (id, name) in cat
        .colours
        .iter()
        .map(|c| (&c.id, &c.name))
        .chain(cat.looks.iter().map(|l| (&l.id, &l.name)))
        .chain([(&cat.air.id, &cat.air.name)])
        .chain(cat.surfaces.iter().map(|s| (&s.id, &s.name)))
        .chain(cat.rocks.iter().map(|r| (&r.id, &r.name)))
        .chain(cat.soils.iter().map(|s| (&s.id, &s.name)))
        .chain(cat.biomes.iter().map(|b| (&b.id, &b.name)))
        .chain(cat.deposits.iter().map(|d| (&d.id, &d.name)))
        .chain(cat.lands.iter().map(|l| (&l.id, &l.name)))
    {
        out.push(id.clone());
        out.push(name.clone());
    }
    out.sort();
    out.dedup();
    Ok(out)
}
