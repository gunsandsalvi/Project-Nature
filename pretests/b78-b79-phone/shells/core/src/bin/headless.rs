// B78/B80: the same core, run headless on Linux (the cloud side of PLT-05).
fn main() {
    println!("mix(42, 1000000) = {:016x}", kcore::mix(42, 1_000_000));
}
