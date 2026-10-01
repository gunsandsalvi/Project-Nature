//! B10 pre-test: the wrap-around map (WLD-01, WLD-02, WLD-03, WLD-12).
//! Square cells (quadtree) against hexes (apertures 7, 4, 3) on the 2:1 torus.
//! Throwaway code; method and results are in NOTES.md.

mod bench;
mod checks;
mod hex;
mod iso;
mod path;
mod rng;
mod sq;

fn main() {
    let what = std::env::args().nth(1).unwrap_or_else(|| "all".into());
    match what.as_str() {
        "checks" => checks::run(),
        "nesting" => checks::nesting(),
        "iso" => iso::run(),
        "iso-more" => iso::more_fields(),
        "bench" => bench::run(),
        "all" => {
            checks::run();
            checks::nesting();
            iso::run();
            bench::run();
        }
        _ => eprintln!("usage: b10-map [checks|nesting|iso|iso-more|bench|all]"),
    }
}
