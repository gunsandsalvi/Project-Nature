//! `kd fixtures write` (A15.9 item 5): stores the core's probes, made on this machine, as the values and hashes
//! every other target must reproduce: `crates/kd-core/tests/fixtures/<probe>.bin` and `hashes.txt`.
//! Run it only when a probe is added or changed on purpose; the tests then compare every target against it.

use kd_core::bits;
use std::path::PathBuf;

fn dir() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../kd-core/tests/fixtures")
}

pub fn write() -> Result<String, String> {
    let dir = dir();
    std::fs::create_dir_all(&dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    let probes = bits::probes();
    for p in &probes {
        let path = dir.join(format!("{}.bin", p.name));
        std::fs::write(&path, &p.bytes).map_err(|e| format!("{}: {e}", path.display()))?;
    }
    let path = dir.join("hashes.txt");
    std::fs::write(&path, bits::hashes_text(&probes)).map_err(|e| format!("{}: {e}", path.display()))?;
    Ok(format!(
        "Fixtures: {} probes written to crates/kd-core/tests/fixtures",
        probes.len()
    ))
}
