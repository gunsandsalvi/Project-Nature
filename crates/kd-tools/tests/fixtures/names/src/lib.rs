//! A fixture for `kd check names` (A2.3 rule 5): one literal names a catalogue entry.
// Mentioning "grass_damp" or "Grass damp" in a comment is fine.

pub fn ladder() -> &'static str {
    "grass_damp"
}

pub fn other() -> &'static str {
    "grass damp, in other words"
}
