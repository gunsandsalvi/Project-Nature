//! The catalogue blob (A3.6): a header, then the compiled catalogue in `postcard`. The header is the magic `KDCAT`,
//! the blob's format version, the rules and generator versions, and `num::hash64` of the body; `Catalogue::load`
//! refuses a blob whose magic, format or hash is wrong.
//!
//! Implements PLT-09, see A3.6: each catalogue carries its versions and its hash.

use core::fmt;

use crate::schema::{Catalogue, Versions};
use kd_core::num::hash64;

pub const MAGIC: &[u8; 5] = b"KDCAT";
/// The blob's own format: raised whenever the compiled entries' layout changes.
pub const FORMAT: u16 = 5;
/// Magic, format, three versions and the hash.
pub const HEADER_BYTES: usize = 5 + 2 + 3 * 2 + 8;

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum BlobError {
    Short,
    Magic,
    Format(u16),
    Hash { stored: u64, found: u64 },
    Body(String),
}

impl fmt::Display for BlobError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            BlobError::Short => write!(f, "the catalogue blob is shorter than its header"),
            BlobError::Magic => write!(f, "not a catalogue blob (no KDCAT)"),
            BlobError::Format(v) => write!(f, "catalogue blob format {v}, not {FORMAT}"),
            BlobError::Hash { stored, found } => {
                write!(
                    f,
                    "catalogue blob damaged: body hash {found:016x}, header says {stored:016x}"
                )
            }
            BlobError::Body(e) => write!(f, "catalogue blob body: {e}"),
        }
    }
}

/// The header's fields, read without decoding the body.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Header {
    pub format: u16,
    pub versions: Versions,
    pub hash: u64,
}

fn u16_at(b: &[u8], i: usize) -> u16 {
    u16::from_le_bytes([b[i], b[i + 1]])
}

/// Reads a blob's header, checking the magic and the length.
pub fn header(blob: &[u8]) -> Result<Header, BlobError> {
    if blob.len() < HEADER_BYTES {
        return Err(BlobError::Short);
    }
    if &blob[..5] != MAGIC {
        return Err(BlobError::Magic);
    }
    let mut hash = [0u8; 8];
    hash.copy_from_slice(&blob[13..21]);
    Ok(Header {
        format: u16_at(blob, 5),
        versions: Versions {
            major: u16_at(blob, 7),
            minor: u16_at(blob, 9),
            generator: u16_at(blob, 11),
        },
        hash: u64::from_le_bytes(hash),
    })
}

/// The blob of a catalogue: its header, then its body.
pub fn encode(cat: &Catalogue) -> Vec<u8> {
    let body = postcard::to_allocvec(cat).unwrap_or_default();
    let mut out = Vec::with_capacity(HEADER_BYTES + body.len());
    out.extend_from_slice(MAGIC);
    out.extend_from_slice(&FORMAT.to_le_bytes());
    for v in [cat.versions.major, cat.versions.minor, cat.versions.generator] {
        out.extend_from_slice(&v.to_le_bytes());
    }
    out.extend_from_slice(&hash64(&body).to_le_bytes());
    out.extend_from_slice(&body);
    out
}

impl Catalogue {
    /// The catalogue in a blob, after checking its magic, format and hash: under 10 ms on the phone (A3.6).
    pub fn load(blob: &[u8]) -> Result<Catalogue, BlobError> {
        let h = header(blob)?;
        if h.format != FORMAT {
            return Err(BlobError::Format(h.format));
        }
        let body = &blob[HEADER_BYTES..];
        let found = hash64(body);
        if found != h.hash {
            return Err(BlobError::Hash { stored: h.hash, found });
        }
        postcard::from_bytes(body).map_err(|e| BlobError::Body(e.to_string()))
    }

    /// The hash of the catalogue's body, as the version line and saves name it.
    pub fn hash_of(blob: &[u8]) -> Option<u64> {
        header(blob).ok().map(|h| h.hash)
    }
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;
    use crate::schema::{Air, Colour, Look, Surface};
    use crate::tuning::{Measure, TuneValue, Tuning};
    use crate::world::{Biome, Carried, Climate, Deposit, Escarpment, Herd, Land, Rock, Side, Soil, Valley};

    pub(crate) fn sample() -> Catalogue {
        Catalogue {
            versions: Versions {
                major: 1,
                minor: 0,
                generator: 1,
            },
            colours: vec![
                Colour {
                    id: "void".into(),
                    name: "Void".into(),
                    number: 0,
                    rgb: [0, 0, 0],
                },
                Colour {
                    id: "ink".into(),
                    name: "Ink".into(),
                    number: 1,
                    rgb: [20, 18, 24],
                },
            ],
            looks: vec![Look {
                id: "limestone".into(),
                name: "Limestone".into(),
                number: 0,
                rgb: [200, 196, 184],
                steps: 6,
                sheen: 0.0,
                glow: None,
            }],
            air: Air {
                id: "air".into(),
                name: "Air".into(),
                wavelengths_nm: [680.0, 550.0, 440.0],
                rayleigh: [0.041, 0.097, 0.243],
                turbidity: 0.08,
                aerosol_exponent: 1.3,
                ozone: [0.02, 0.027, 0.002],
                sky_rayleigh_share: 0.5,
                sky_aerosol_share: 0.7,
                horizon_sky: 0.04,
                horizon_fade_deg: 3.0,
                twilight_end_deg: -12.0,
                twilight_fall_deg: 1.2,
                twilight_ozone_per_deg: 10.0,
                moon_weakness: 400_000.0,
                moon_tint: [0.9, 0.95, 1.0],
                starlight: 2e-8,
                exposure_scale: 2.0,
                exposure_power: 0.85,
                rods_start: 1e-4,
                rods_full: 1e-7,
                grade_chroma: 1.2,
                haze_levels: [0.1, 0.25, 0.45],
                haze_mixes: [0.15, 0.33, 0.55],
                haze_g: 0.7,
                aerosol_height_m: 1200.0,
                rayleigh_height_m: 8000.0,
            },
            surfaces: vec![Surface {
                id: "rock".into(),
                name: "Rock".into(),
                number: 0,
                looks: vec![0],
                split_m: vec![],
                split_at: vec![],
                rock: true,
                relief_m: [0.2, 2.0],
                relief_tilt: 0.5,
                stones_per_m2: 0.0,
                stone_look: None,
                tufts_per_m2: 0.0,
                tuft_looks: vec![],
            }],
            rocks: vec![Rock {
                id: "chalk".into(),
                name: "Chalk".into(),
                number: 0,
                softness: 1.6,
                beds_m: [0.3, 3.0],
                caves: true,
                look: 0,
            }],
            soils: vec![Soil {
                id: "loam".into(),
                name: "Loam".into(),
                number: 0,
                capacity_m: 0.15,
                intake_m: 0.01,
                dig: 3,
                keeps: 3,
                fertility_shift: 0,
                landform: None,
            }],
            biomes: vec![Biome {
                id: "grassland".into(),
                name: "Grassland".into(),
                number: 0,
                look: 0,
                cover: [5, 26, 204, 5, 15],
                landform: Some(crate::world::Landform::Coast),
            }],
            deposits: vec![Deposit {
                id: "river_gravel".into(),
                name: "River gravel".into(),
                number: 0,
                rock: None,
                rivers: true,
                richness: [1, 3],
                carried: Some(Carried {
                    keep: 0.8,
                    every_m: 10_000.0,
                }),
            }],
            lands: vec![Land {
                id: "islet".into(),
                name: "Islet".into(),
                number: 0,
                centre_cell: [1000, 244],
                island_m: 4_000.0,
                sea_m: 50_000.0,
                base_height_m: 20.0,
                relief_m: 5.0,
                valley: Valley {
                    flow_m3s: 8.0,
                    width_m: 18.0,
                    floodplain_m: 800.0,
                    from: Side::North,
                },
                escarpment: Escarpment {
                    side: Side::North,
                    height_m: 30.0,
                    rocks: vec![0],
                    caves: 6,
                    shelters: 4,
                },
                soil: 0,
                fertility: 3,
                biome: 0,
                cover: [140, 38, 64, 5, 8],
                climate: Climate {
                    mean_c: [9.0, 19.0, 11.0, 3.0],
                    range_c: 9.0,
                    rain_m: [0.17, 0.14, 0.18, 0.16],
                    storm_days: [4, 3, 4, 5],
                    thunder_days: [1, 3, 1, 0],
                    wind: Side::West,
                },
                herds: vec![Herd {
                    kind: "red_deer".into(),
                    count: 300,
                }],
                small: [6.0, 20.0, 30.0],
                tilt_deg: 23.5,
            }],
            tunings: vec![Tuning {
                id: "world".into(),
                name: "World".into(),
                number: 0,
                values: vec![TuneValue {
                    key: "river_rating.bankfull_depth".into(),
                    value: 1.2,
                    measure: Measure::Length,
                }],
            }],
        }
    }

    // checks: PLT-09 MAT-13
    #[test]
    fn round_trip() {
        let cat = sample();
        let blob = encode(&cat);
        assert_eq!(&blob[..5], b"KDCAT");
        assert_eq!(Catalogue::load(&blob), Ok(cat.clone()));
        let h = header(&blob).expect("a header");
        assert_eq!((h.format, h.versions), (FORMAT, cat.versions));
        assert_eq!(Catalogue::hash_of(&blob), Some(hash64(&blob[HEADER_BYTES..])));
        // The same catalogue always gives the same bytes.
        assert_eq!(encode(&cat), blob);
    }

    // checks: PLT-09
    #[test]
    fn bad_hash_refused() {
        let mut blob = encode(&sample());
        let last = blob.len() - 1;
        blob[last] ^= 1;
        assert!(matches!(Catalogue::load(&blob), Err(BlobError::Hash { .. })));
    }

    // checks: PLT-09
    #[test]
    fn bad_magic_refused() {
        let mut blob = encode(&sample());
        blob[0] = b'X';
        assert_eq!(Catalogue::load(&blob), Err(BlobError::Magic));
        assert_eq!(Catalogue::load(&blob[..10]), Err(BlobError::Short));
        let mut other = encode(&sample());
        other[5] = 9;
        assert_eq!(Catalogue::load(&other), Err(BlobError::Format(9)));
    }
}
