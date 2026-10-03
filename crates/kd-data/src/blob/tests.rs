use super::*;
use crate::schema::{LightMethod, LightRole};

fn sample() -> Body {
    Body {
        families: vec![FamilyRec {
            id: "a".into(),
            name: "A".into(),
            first: 0,
            count: 2,
        }],
        colours: vec![
            ColourRec {
                name: "x".into(),
                rgb: [1, 2, 3],
                family: 0,
            },
            ColourRec {
                name: "y".into(),
                rgb: [4, 5, 6],
                family: 0,
            },
        ],
        ladders: vec![LadderRec {
            id: "l".into(),
            name: "L".into(),
            steps: vec![0, 1],
        }],
        light: vec![LightRec {
            id: "t".into(),
            name: "T".into(),
            role: LightRole::Table,
            method: LightMethod::Lab,
            k: "0".into(),
            toward: [0, 0, 0],
            amount: "0".into(),
            mul: ["0.42".into(), "1".into(), "1".into()],
            add: ["0".into(), "0".into(), "-0.012".into()],
            l_min: None,
            l_max: Some("0.24".into()),
            mask: vec![false, true],
        }],
        surfaces: vec![SurfaceRec {
            id: "s".into(),
            name: "S".into(),
            ladder: 0,
            stone_density: 160,
            stone_size_mm: 80,
            tuft_density: 0,
            flags: 16,
        }],
    }
}

// checks: PLT-09 MAT-13
#[test]
fn round_trip() {
    let blob = Catalogue::encode(1, 2, 3, &sample());
    let c = Catalogue::load(&blob).unwrap();
    assert_eq!(c.body, sample());
    assert_eq!((c.major, c.minor, c.generator), (1, 2, 3));
    assert_eq!(c.colour("y").map(|k| k.0), Some(1));
    assert_eq!(c.ladder("l").map(|k| k.0), Some(0));
    assert_eq!(c.surface("s"), Some(0));
    assert_eq!(&blob[..5], b"KDCAT");
}

// checks: PLT-09 MAT-13
#[test]
fn round_trip_of_the_compiled_fixture() {
    // Compiles the clean fixture, then checks the loaded tables against the entries' own words, and the blob against
    // a fresh encoding of what was loaded, so a loss on either side shows.
    #[cfg(feature = "compile")]
    {
        use crate::compile::Parsed;
        let root = std::path::PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("tests/fixtures/clean/data");
        let c = crate::compile::compile(&root, false).unwrap();
        let loaded = Catalogue::load(&c.blob).unwrap();
        assert_eq!(
            Catalogue::encode(loaded.major, loaded.minor, loaded.generator, &loaded.body),
            c.blob
        );
        let ladders: Vec<_> = c
            .entries
            .iter()
            .filter_map(|e| match &e.parsed {
                Parsed::Ladder(l) => Some(l),
                _ => None,
            })
            .collect();
        assert!(!ladders.is_empty());
        assert_eq!(loaded.body.ladders.len(), ladders.len());
        for (rec, l) in loaded.body.ladders.iter().zip(&ladders) {
            assert_eq!(rec.id, l.common.id);
            let names: Vec<&str> = rec
                .steps
                .iter()
                .map(|&k| loaded.body.colours[k as usize].name.as_str())
                .collect();
            assert_eq!(names, l.steps, "ladder `{}`", rec.id);
        }
    }
}

// checks: PLT-09
#[test]
fn load_rejects_damage() {
    let blob = Catalogue::encode(1, 0, 1, &sample());
    let mut flipped = blob.clone();
    let last = flipped.len() - 1;
    flipped[last] ^= 1;
    assert_eq!(Catalogue::load(&flipped), Err(LoadError::Hash));
    let mut magic = blob.clone();
    magic[0] = b'X';
    assert_eq!(Catalogue::load(&magic), Err(LoadError::Magic));
    let mut format = blob.clone();
    format[5] = 9;
    assert_eq!(Catalogue::load(&format), Err(LoadError::Format(9)));
    assert_eq!(Catalogue::load(&blob[..10]), Err(LoadError::TooShort));
}
