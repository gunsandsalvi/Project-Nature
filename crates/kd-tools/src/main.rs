//! kd-tools: the kd command line (A2.7, A15); implements PRN-14 in part (`kd check layers`).
#![deny(unsafe_code)]

mod layers;

use std::path::PathBuf;

const USAGE: &str = "Usage: kd <command>
  kd check layers    the crate graph and the simulation sources against tools/layers.toml (A2.3)
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

fn main() {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let words: Vec<&str> = args.iter().map(String::as_str).collect();
    let code = match words.as_slice() {
        ["check", "layers"] => match root() {
            Some(root) => {
                let (lines, ok) = layers::run(&root);
                for l in lines {
                    println!("{l}");
                }
                if ok { 0 } else { 1 }
            }
            None => {
                eprintln!("kd: no tools/layers.toml here or above; run from the repository");
                1
            }
        },
        _ => {
            eprintln!("{USAGE}");
            2
        }
    };
    std::process::exit(code);
}
