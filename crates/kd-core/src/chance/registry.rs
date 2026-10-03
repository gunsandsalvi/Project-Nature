//! The purpose registry (A3.3): every crate's `purposes!` lists joined, so no two purposes share a number or a
//! name and no retired number comes back. The joining test runs in `kd-sim` once systems exist; here are the rules.

use super::{PurposeList, System};
use std::collections::BTreeMap;

/// The largest number a system or a purpose may take: bit 31 of a purpose is fortune's retry (A3.3).
pub const MAX_NUMBER: u32 = 65_535;

/// Every problem with a set of purpose lists, or none: numbers outside 1–65,535, a purpose in another system's
/// list, a number used twice in one system, a name used twice, a retired number used again, and two systems
/// sharing a number or a name.
pub fn check(lists: &[PurposeList]) -> Result<(), Vec<String>> {
    let mut problems = Vec::new();
    let mut systems: BTreeMap<u32, System> = BTreeMap::new();
    let mut numbers: BTreeMap<(u32, u32), String> = BTreeMap::new();
    let mut names: BTreeMap<String, (u32, u32)> = BTreeMap::new();
    let mut retired: BTreeMap<(u32, u32), ()> = BTreeMap::new();
    for list in lists {
        let s = list.system;
        if !(1..=MAX_NUMBER).contains(&s.number) {
            problems.push(format!(
                "system {} has number {}, outside 1–{MAX_NUMBER}",
                s.name, s.number
            ));
        }
        match systems.get(&s.number) {
            Some(other) if other.name != s.name => problems.push(format!(
                "systems {} and {} share number {}",
                other.name, s.name, s.number
            )),
            _ => {
                systems.insert(s.number, s);
            }
        }
        for &r in list.retired {
            retired.insert((s.number, r), ());
        }
    }
    let mut by_name: BTreeMap<&str, u32> = BTreeMap::new();
    for s in systems.values() {
        if let Some(n) = by_name.insert(s.name, s.number) {
            problems.push(format!("systems {n} and {} share the name {}", s.number, s.name));
        }
    }
    for list in lists {
        for p in list.purposes {
            let name = p.name();
            if p.system != list.system {
                problems.push(format!("{name} is in the list of system {}", list.system.name));
            }
            if !(1..=MAX_NUMBER).contains(&p.number) {
                problems.push(format!("{name} has number {}, outside 1–{MAX_NUMBER}", p.number));
            }
            let key = (p.system.number, p.number);
            if retired.contains_key(&key) {
                problems.push(format!(
                    "{name} uses number {} of {}, which is retired",
                    p.number, p.system.name
                ));
            }
            if let Some(other) = numbers.insert(key, name.clone()) {
                problems.push(format!(
                    "{other} and {name} share number {} of {}",
                    p.number, p.system.name
                ));
            }
            if let Some(other) = names.insert(name.clone(), key) {
                problems.push(format!(
                    "{name} is declared twice, as {}:{} and {}:{}",
                    other.0, other.1, key.0, key.1
                ));
            }
        }
    }
    if problems.is_empty() { Ok(()) } else { Err(problems) }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::chance::{Fortune, Purpose, Subject, systems};

    const fn purpose(system: System, number: u32, ident: &'static str) -> Purpose {
        Purpose {
            system,
            number,
            ident,
            about: "a test",
            subject: Subject::Person,
            fortune: Fortune::None,
        }
    }

    const A: [Purpose; 2] = [purpose(systems::THINGS, 1, "TRY"), purpose(systems::THINGS, 2, "SPLIT")];
    const A1: [Purpose; 1] = [purpose(systems::THINGS, 1, "TRY")];
    const B: [Purpose; 1] = [purpose(systems::MINDS, 1, "TRY")];

    // checks: TIM-16
    #[test]
    fn repeated_and_retired_numbers_fail() {
        let things = PurposeList {
            system: systems::THINGS,
            purposes: &A,
            retired: &[3],
        };
        let minds = PurposeList {
            system: systems::MINDS,
            purposes: &B,
            retired: &[],
        };
        // One number in two systems, and one constant name in two systems, are both fine.
        assert_eq!(check(&[things, minds]), Ok(()));
        // The same number twice in a system, a reused retired number, a name twice, a number out of range.
        const C: [Purpose; 1] = [purpose(systems::THINGS, 2, "CARRY")];
        const D: [Purpose; 1] = [purpose(systems::THINGS, 3, "DROP")];
        const E: [Purpose; 1] = [purpose(systems::THINGS, 70_000, "FAR")];
        for (bad, says) in [
            (
                PurposeList {
                    system: systems::THINGS,
                    purposes: &C,
                    retired: &[],
                },
                "share number 2",
            ),
            (
                PurposeList {
                    system: systems::THINGS,
                    purposes: &D,
                    retired: &[],
                },
                "which is retired",
            ),
            (
                PurposeList {
                    system: systems::THINGS,
                    purposes: &A1,
                    retired: &[],
                },
                "declared twice",
            ),
            (
                PurposeList {
                    system: systems::THINGS,
                    purposes: &E,
                    retired: &[],
                },
                "outside 1–65535",
            ),
            (
                PurposeList {
                    system: systems::MINDS,
                    purposes: &C,
                    retired: &[],
                },
                "in the list of system minds",
            ),
        ] {
            let problems = check(&[things, minds, bad]).expect_err(says);
            assert!(problems.iter().any(|p| p.contains(says)), "{says}: {problems:?}");
        }
        // Two systems sharing a number.
        let clash = PurposeList {
            system: System {
                number: 8,
                name: "other",
            },
            purposes: &[],
            retired: &[],
        };
        assert!(check(&[things, clash]).is_err());
        // The thirteen systems of A3.3 are numbered 1 to 13 in order.
        let numbers: Vec<u32> = systems::ALL.iter().map(|s| s.number).collect();
        assert_eq!(numbers, (1..=13).collect::<Vec<_>>());
    }
}
