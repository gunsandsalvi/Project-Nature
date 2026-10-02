use super::*;
use std::path::PathBuf;

fn fixture(case: &str) -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("tests/fixtures")
        .join(case)
        .join("data")
}

/// The rules a fixture's check breaks, each once.
fn broken(case: &str) -> Vec<Rule> {
    match check(&fixture(case)) {
        Ok(_) => Vec::new(),
        Err(errs) => {
            let mut rules: Vec<Rule> = errs.iter().map(|e| e.rule).collect();
            rules.dedup();
            rules
        }
    }
}

// checks: MAT-17 MAT-13
#[test]
fn planted_errors() {
    assert_eq!(broken("clean"), vec![], "the clean fixture passes");
    let cases = [
        ("missing_block", Rule::MissingBlock),
        ("two_blocks", Rule::TwoBlocks),
        ("unknown_field", Rule::UnknownField),
        ("missing_field", Rule::MissingField),
        ("name_mismatch", Rule::NameMismatch),
        ("repeated_id", Rule::RepeatedId),
        ("not_snake_case", Rule::NotSnakeCase),
        ("not_locked", Rule::NotLocked),
        ("retired_reused", Rule::RetiredNumber),
        ("unresolved_reference", Rule::UnresolvedReference),
        ("empty_checks", Rule::EmptyChecks),
        ("dropped_check", Rule::CheckNotLive),
        ("stale_table", Rule::StaleTable),
        ("unclaimed_file", Rule::UnclaimedFile),
    ];
    for (case, rule) in cases {
        assert_eq!(broken(case), vec![rule], "{case} fails with its own rule and no other");
    }
}

// checks: MAT-17
#[test]
fn messages_name_the_place() {
    let errs = check(&fixture("dropped_check")).unwrap_err();
    assert_eq!(errs.len(), 1);
    assert_eq!(
        errs[0].to_string(),
        "data/palette/ladders.md:24: checks `OLD-01`, which PROJECT.md has dropped"
    );
}

// checks: PLT-09 MAT-13
#[test]
fn clean_fixture_compiles_in_number_order() {
    let c = compile(&fixture("clean"), false).unwrap();
    let b = &c.catalogue.body;
    assert_eq!(
        b.colours.iter().map(|c| c.name.as_str()).collect::<Vec<_>>(),
        ["void", "s0", "s1", "s2"]
    );
    assert_eq!(b.families[1].first, 1);
    assert_eq!(b.ladders[0].steps, vec![1, 2, 3]);
    assert_eq!(b.ladders[1].id, "lime_lit");
    assert_eq!((c.catalogue.major, c.catalogue.minor, c.catalogue.generator), (1, 0, 1));
}

// checks: MAT-17
#[test]
fn ids_and_numbers() {
    assert!(source::is_snake_case("grass_damp") && source::is_snake_case("warm_1"));
    for bad in ["", "Grass", "grass__damp", "_grass", "grass_", "1grass", "grass-damp"] {
        assert!(!source::is_snake_case(bad), "{bad}");
    }
    for good in ["0", "0.42", "-0.012", "2.2", "104"] {
        assert!(source::is_decimal(good), "{good}");
    }
    for bad in ["", ".5", "1.", "1e3", "--1", "0.4.2", "x"] {
        assert!(!source::is_decimal(bad), "{bad}");
    }
}
