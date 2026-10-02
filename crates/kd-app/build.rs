//! Compiles `data/` into the catalogue blob the app embeds (A3.6): `$OUT_DIR/catalogue.bin`, rebuilt only when `data/`
//! changes. A catalogue that fails to compile fails the build, each problem printed as a warning.

use std::path::Path;

fn main() {
    println!("cargo:rerun-if-changed=../../data");
    let data = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../data");
    let out = Path::new(&std::env::var("OUT_DIR").expect("cargo sets OUT_DIR")).join("catalogue.bin");
    match kd_data::compile::compile(&data, false) {
        Ok(c) => std::fs::write(&out, &c.blob).expect("write catalogue.bin"),
        Err(errs) => {
            for e in &errs {
                println!("cargo:warning={e}");
            }
            panic!(
                "the catalogue in data/ does not compile: {} problem(s); run `kd catalog check`",
                errs.len()
            );
        }
    }
}
