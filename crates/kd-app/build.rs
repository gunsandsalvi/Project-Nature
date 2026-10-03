//! Compiles `data/` into the catalogue blob the app embeds (A3.6): on the build machine, for every target. A
//! catalogue that fails its checks, or whose generated tables are stale, fails the build (`MAT-17`).

use std::path::{Path, PathBuf};

use kd_data::compile::{self, Source};

fn collect(root: &Path, dir: &Path, out: &mut Vec<Source>) {
    let Ok(entries) = std::fs::read_dir(dir) else { return };
    for e in entries.flatten() {
        let path = e.path();
        if path.is_dir() {
            collect(root, &path, out);
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

fn main() {
    let manifest = PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").expect("cargo sets CARGO_MANIFEST_DIR"));
    let root = manifest.join("../..");
    let data = root.join("data");
    println!("cargo:rerun-if-changed={}", data.display());
    let mut sources = Vec::new();
    collect(&root, &data, &mut sources);
    let out = match compile::compile(&sources, false) {
        Ok(out) => out,
        Err(problems) => {
            for p in &problems {
                println!("cargo:warning={p}");
            }
            panic!(
                "the catalogue fails {} of its checks: kd catalog check says where",
                problems.len()
            );
        }
    };
    if !out.stale.is_empty() {
        panic!("the catalogue's generated tables are stale: run kd catalog tables");
    }
    let dest = PathBuf::from(std::env::var("OUT_DIR").expect("cargo sets OUT_DIR")).join("catalogue.kdcat");
    std::fs::write(&dest, &out.blob).expect("the blob is written to OUT_DIR");
}
