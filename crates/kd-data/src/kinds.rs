//! Which kind of entry each file under `data/` holds (A3.6): a file holds one kind, chosen by its place. A Markdown
//! file under `data/` that no kind claims fails the compiler, so nothing is silently left out.

/// The kinds of entry the catalogue has so far.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Kind {
    Colour,
    Look,
    Air,
}

impl Kind {
    pub const ALL: [Kind; 3] = [Kind::Colour, Kind::Look, Kind::Air];

    /// The kind's name in `data/ids.lock` and in messages.
    pub fn name(self) -> &'static str {
        match self {
            Kind::Colour => "colour",
            Kind::Look => "look",
            Kind::Air => "air",
        }
    }

    /// The file or folder that holds the kind, from the repository's root.
    pub fn place(self) -> &'static str {
        match self {
            Kind::Colour => "data/palette/colours.md",
            Kind::Look => "data/palette/looks.md",
            Kind::Air => "data/palette/light.md",
        }
    }
}

/// What a file under `data/` is.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FileRole {
    /// Entries of one kind.
    Entries(Kind),
    /// Not entries: the generated index, the tuning log, a folder's readme.
    Other,
    /// A Markdown file no kind claims, which fails.
    Unclaimed,
}

/// The role of a path from the repository's root (with `/` between parts).
pub fn role(path: &str) -> FileRole {
    if let Some(k) = Kind::ALL.into_iter().find(|k| {
        let place = k.place();
        path == place || (place.ends_with('/') && path.starts_with(place))
    }) {
        return FileRole::Entries(k);
    }
    match path {
        "data/INDEX.md" | "data/TUNING-LOG.md" => FileRole::Other,
        p if p.ends_with("/README.md") => FileRole::Other,
        p if p.ends_with(".md") => FileRole::Unclaimed,
        _ => FileRole::Other,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: MAT-13
    #[test]
    fn each_file_has_one_kind() {
        assert_eq!(role("data/palette/looks.md"), FileRole::Entries(Kind::Look));
        assert_eq!(role("data/palette/colours.md"), FileRole::Entries(Kind::Colour));
        assert_eq!(role("data/palette/light.md"), FileRole::Entries(Kind::Air));
        assert_eq!(role("data/INDEX.md"), FileRole::Other);
        assert_eq!(role("data/VERSION.toml"), FileRole::Other);
        assert_eq!(role("data/palette/stray.md"), FileRole::Unclaimed);
    }
}
