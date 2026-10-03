//! Which kind of entry each file under `data/` holds (A3.6): a file holds one kind, chosen by its place. A Markdown
//! file under `data/` that no kind claims fails the compiler, so nothing is silently left out.

/// The kinds of entry the catalogue has so far; `Light` holds the one `Air` entry, `Land` the land presets (A5.6)
/// and `Tuning` the tuned numbers, one entry a system (`PRN-17`). A kind that names another's entries comes after
/// it, so the names resolve (A3.6 rule 4).
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Kind {
    Colour,
    Look,
    Light,
    Surface,
    Rock,
    Soil,
    Biome,
    Deposit,
    Land,
    Tuning,
}

impl Kind {
    pub const ALL: [Kind; 10] = [
        Kind::Colour,
        Kind::Look,
        Kind::Light,
        Kind::Surface,
        Kind::Rock,
        Kind::Soil,
        Kind::Biome,
        Kind::Deposit,
        Kind::Land,
        Kind::Tuning,
    ];

    /// The kind's name in `data/ids.lock` and in messages.
    pub fn name(self) -> &'static str {
        match self {
            Kind::Colour => "colour",
            Kind::Look => "look",
            Kind::Light => "light",
            Kind::Surface => "surface",
            Kind::Rock => "rock",
            Kind::Soil => "soil",
            Kind::Biome => "biome",
            Kind::Deposit => "deposit",
            Kind::Land => "land",
            Kind::Tuning => "tuning",
        }
    }

    /// The file or folder that holds the kind, from the repository's root.
    pub fn place(self) -> &'static str {
        match self {
            Kind::Colour => "data/palette/colours.md",
            Kind::Look => "data/palette/looks.md",
            Kind::Light => "data/palette/light.md",
            Kind::Surface => "data/models/surfaces.md",
            Kind::Rock => "data/world/rocks.md",
            Kind::Soil => "data/world/soils.md",
            Kind::Biome => "data/world/biomes.md",
            Kind::Deposit => "data/world/deposits.md",
            Kind::Land => "data/lands/",
            Kind::Tuning => "data/tuning/",
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
    if path.ends_with("/README.md") {
        return FileRole::Other;
    }
    if let Some(k) = Kind::ALL.into_iter().find(|k| {
        let place = k.place();
        path == place || (place.ends_with('/') && path.starts_with(place))
    }) {
        return FileRole::Entries(k);
    }
    match path {
        "data/INDEX.md" | "data/TUNING-LOG.md" => FileRole::Other,
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
        assert_eq!(role("data/palette/light.md"), FileRole::Entries(Kind::Light));
        assert_eq!(role("data/models/surfaces.md"), FileRole::Entries(Kind::Surface));
        assert_eq!(role("data/world/rocks.md"), FileRole::Entries(Kind::Rock));
        assert_eq!(role("data/world/deposits.md"), FileRole::Entries(Kind::Deposit));
        assert_eq!(role("data/lands/first-region.md"), FileRole::Entries(Kind::Land));
        assert_eq!(role("data/tuning/render.md"), FileRole::Entries(Kind::Tuning));
        assert_eq!(role("data/world/stray.md"), FileRole::Unclaimed);
        assert_eq!(role("data/tuning/README.md"), FileRole::Other);
        assert_eq!(role("data/INDEX.md"), FileRole::Other);
        assert_eq!(role("data/VERSION.toml"), FileRole::Other);
        assert_eq!(role("data/palette/stray.md"), FileRole::Unclaimed);
    }
}
