//! kd-tools: the `kd` command line (A2.7, A15). Its commands arrive with their alphas.

#![deny(unsafe_code)]

mod fixtures;

const USAGE: &str = "usage: kd <command>
  kd fixtures write          store the core's probes as the bits every target must give (A15.9)";

fn main() {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let args: Vec<&str> = args.iter().map(String::as_str).collect();
    let result = match args.as_slice() {
        ["fixtures", "write"] => fixtures::write(),
        _ => {
            eprintln!("{USAGE}");
            std::process::exit(2);
        }
    };
    match result {
        Ok(line) => println!("{line}"),
        Err(e) => {
            eprintln!("kd: {e}");
            std::process::exit(1);
        }
    }
}
