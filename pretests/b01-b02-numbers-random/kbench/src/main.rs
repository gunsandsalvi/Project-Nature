//! kbench command-line tool for the cloud runs (B01, B02).
//!
//!   kbench '<config json>'          run one config, print the result JSON
//!   kbench --list                   kernels, formats, generators, suggested phone plan
//!   kbench --selftest               known answers + Rust/C++ draws agree
//!   kbench --batch FILE             one config JSON per line; prints one result per line
//!   kbench --stream GEN MODE        raw little-endian u64 draws to stdout, for PractRand
//!                                   MODE = moments (one key, moments 0,1,2...)
//!                                        | beings  (beings 0,1,2... at one moment)
//!                                        | retry   (alternating first draw / fortune retry)
use kbench::rng::{self, retry_purpose, Gen, KeyedGen, Stream};
use kbench::with_gen;
use std::io::{BufRead, Write};

fn stream_draws(gen: Gen, mode: &str) -> std::io::Result<()> {
    // Fixed test key: world 1, system 2, purpose 3; being 12345 for one-key streams.
    let st = Stream::new(1, 2, 3);
    let st_retry = Stream::new(1, 2, retry_purpose(3));
    let mut out = std::io::stdout().lock();
    let mut buf = vec![0u8; 1 << 20];
    let words = buf.len() / 8;
    let mut i: u64 = 0;
    loop {
        with_gen!(gen, &st, |g| {
            for w in 0..words {
                let v = match mode {
                    "moments" => g.draw(12345, i),
                    "beings" => g.draw(i, 777),
                    _ => {
                        // even words: first draw of chance event i; odd: its fortune retry
                        if i % 2 == 0 {
                            g.draw(i / 2, 777)
                        } else {
                            rng::draw(gen, &st_retry, i / 2, 777)
                        }
                    }
                };
                buf[w * 8..w * 8 + 8].copy_from_slice(&v.to_le_bytes());
                i += 1;
            }
        });
        if let Err(e) = out.write_all(&buf) {
            if e.kind() == std::io::ErrorKind::BrokenPipe {
                return Ok(());
            }
            return Err(e);
        }
    }
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    match args.get(1).map(String::as_str) {
        Some("--list") => println!("{}", kbench::run(r#"{"list":true}"#)),
        Some("--selftest") => println!("{}", kbench::run(r#"{"selftest":true}"#)),
        Some("--batch") => {
            let f = std::fs::File::open(&args[2]).expect("cannot open batch file");
            for line in std::io::BufReader::new(f).lines() {
                let line = line.unwrap();
                let line = line.trim();
                if line.is_empty() || line.starts_with('#') {
                    continue;
                }
                println!("{}", kbench::run(line));
                std::io::stdout().flush().unwrap();
            }
        }
        Some("--stream") => {
            let gen = Gen::from_name(&args[2]).expect("unknown generator");
            let mode = args.get(3).map(String::as_str).unwrap_or("moments");
            assert!(["moments", "beings", "retry"].contains(&mode), "unknown mode");
            stream_draws(gen, mode).unwrap();
        }
        Some(cfg) if cfg.starts_with('{') => println!("{}", kbench::run(cfg)),
        _ => {
            eprintln!("usage: kbench '<json>' | --list | --selftest | --batch FILE | --stream GEN moments|beings|retry");
            std::process::exit(2);
        }
    }
}
