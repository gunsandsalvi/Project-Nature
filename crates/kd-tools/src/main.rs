//! kd-tools: the `kd` command line (A2.7, A15). Its commands arrive with their alphas.

#![deny(unsafe_code)]

mod fixtures;
mod layers;
mod names;
mod scan;

use std::path::PathBuf;

const USAGE: &str = "usage: kd <command>
  kd check layers            each crate names only what tools/layers.toml allows (A2.3)
  kd check names             no catalogue name written in a simulation crate's code (A2.3 rule 5)
  kd fixtures write          store the core's probes as the bits every target must give (A15.9)";

/// The repository's root: two folders above this crate's.
fn root() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../..")
}

fn main() {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let args: Vec<&str> = args.iter().map(String::as_str).collect();
    let result = match args.as_slice() {
        ["check", "layers"] => layers::run(&root()),
        ["check", "names"] => names::run(&root()),
        ["fixtures", "write"] => fixtures::write(),
        _ => {
            eprintln!("{USAGE}");
            std::process::exit(2);
        }
    };
    match result {
        Ok(line) => println!("{line}"),
        Err(e) => {
            eprintln!("{e}");
            std::process::exit(1);
        }
    }
}
