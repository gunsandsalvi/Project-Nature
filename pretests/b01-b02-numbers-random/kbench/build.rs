// B01: compiles the C++ kernels into the same library, twice:
// - "cpp":   compiler-default float settings (clang fuses a*b+c into one FMA where the
//            chip has it, as an Android NDK build would);
// - "cppnc": the same source with -ffp-contract=off (no fused multiply-add), to show
//            what it takes for C++ floats to match across machines (X1, X11).
// KBENCH_CXX_MARCH (optional) sets -march for the C++ side, to match RUSTFLAGS target-cpu.
fn main() {
    println!("cargo:rerun-if-changed=cpp/kernels.cpp");
    println!("cargo:rerun-if-changed=cpp/kernels_nc.cpp");
    println!("cargo:rerun-if-env-changed=KBENCH_CXX_MARCH");
    println!("cargo:rerun-if-env-changed=CXX");

    let mut base = cc::Build::new();
    base.cpp(true)
        .std("c++17")
        .opt_level(3)
        .flag_if_supported("-fno-exceptions")
        .flag_if_supported("-fno-rtti")
        // The C++ code uses no C++ runtime (no new, no exceptions, no iostream), so the
        // .so needs no libc++_shared.so in the app.
        .cpp_link_stdlib(None)
        .warnings(true);
    if let Ok(m) = std::env::var("KBENCH_CXX_MARCH") {
        if !m.is_empty() {
            base.flag(format!("-march={m}"));
        }
    }

    let mut main = base.clone();
    main.file("cpp/kernels.cpp");
    main.compile("kbcpp");

    let mut nc = base.clone();
    nc.file("cpp/kernels_nc.cpp").flag("-ffp-contract=off");
    nc.compile("kbcppnc");
}
