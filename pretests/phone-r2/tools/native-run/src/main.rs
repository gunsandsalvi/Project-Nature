//! Round-2 check: `native-run storage '<config json>'` or `native-run sound '<config json>'` prints the
//! library's JSON result, exactly as the app receives it (the app adds nothing but the call).
fn main() {
    let args: Vec<String> = std::env::args().collect();
    let config = args.get(2).cloned().unwrap_or_else(|| "{}".into());
    let out = match args.get(1).map(String::as_str) {
        Some("storage") => kstorage::run(&config),
        Some("sound") => ksound::run(&config),
        _ => {
            eprintln!("usage: native-run storage|sound '<config json>'");
            std::process::exit(2);
        }
    };
    println!("{out}");
}
