//! Which kind of entry each catalogue file holds (A3.6: one kind per file, chosen by its place under `data/`).
//! Implements `MAT-13` in part, see A3.6.

/// The kinds of catalogue entry; each has its numbers in `data/ids.lock` under its lock name.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum KindName {
    ColourFamily,
    Ladder,
    LightTable,
}

impl KindName {
    /// The kind's name in `data/ids.lock` and in messages.
    pub fn lock_name(self) -> &'static str {
        match self {
            KindName::ColourFamily => "colour_family",
            KindName::Ladder => "ladder",
            KindName::LightTable => "light_table",
        }
    }

    /// The kind's heading in `data/INDEX.md`.
    pub fn title(self) -> &'static str {
        match self {
            KindName::ColourFamily => "Colour families",
            KindName::Ladder => "Ladders",
            KindName::LightTable => "Light tables and palette versions",
        }
    }

    /// The kind named in `data/ids.lock`, if any.
    pub fn from_lock_name(s: &str) -> Option<KindName> {
        KINDS.iter().map(|(_, k)| *k).find(|k| k.lock_name() == s)
    }
}

/// Each file (`palette/colours.md`) or folder (ending `/`) under `data/` and the kind it holds; a Markdown file under
/// `data/` that no line claims fails the check, so nothing hides (`UNCLAIMED_OK` aside).
pub const KINDS: &[(&str, KindName)] = &[
    ("palette/colours.md", KindName::ColourFamily),
    ("palette/ladders.md", KindName::Ladder),
    ("palette/light.md", KindName::LightTable),
];

/// Markdown files under `data/` that hold no entries: the generated index and the tuning log (A3.6).
pub const UNCLAIMED_OK: &[&str] = &["INDEX.md", "TUNING-LOG.md"];

/// The kind claiming a path relative to `data/`, written with `/`.
pub fn kind_of(rel: &str) -> Option<KindName> {
    KINDS
        .iter()
        .find(|(p, _)| {
            if p.ends_with('/') {
                rel.starts_with(p)
            } else {
                rel == *p
            }
        })
        .map(|(_, k)| *k)
}
