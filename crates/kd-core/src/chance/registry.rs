//! The registry of systems and purposes, so no two systems share draws (A3.3). Implements `TIM-16` in part.

/// Every system that draws, numbered once and for good (A3.3); new systems take the next number.
pub mod systems {
    pub const WORLD_GEN: u32 = 1;
    pub const WEATHER: u32 = 2;
    pub const WATER_LAND: u32 = 3;
    pub const PLANTS: u32 = 4;
    pub const ANIMALS: u32 = 5;
    pub const ILLNESS: u32 = 6;
    pub const FIRE: u32 = 7;
    pub const THINGS: u32 = 8;
    pub const BODIES: u32 = 9;
    pub const MINDS: u32 = 10;
    pub const CULTURE: u32 = 11;
    pub const POWERS: u32 = 12;
    pub const SETUP: u32 = 13;
}

/// What a purpose's subject is (A3.3); debug builds check it at each draw once the world exists.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum SubjectKind {
    Person,
    Animal,
    Being,
    Thing,
    Plant,
    Herd,
    Group,
    Place,
    Pair,
    Any,
}

/// Which way fortune (`GOD-04`, from α35b) may turn a draw.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Fortune {
    Good,
    Bad,
    None,
}

/// One registered reason to draw (A3.3).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Purpose {
    pub system: u32,
    pub number: u32,
    pub name: &'static str,
    pub about: &'static str,
    pub subject: SubjectKind,
    pub fortune: Fortune,
}

/// Declares one system's purposes: one `pub const` per line, `ALL` and `RETIRED` (A3.3).
///
/// ```
/// mod setup {
///     kd_core::purposes! {
///         system SETUP; retired [];
///         1 TEST_DRAW "setup.test_draw" "a draw for tests" subject Any fortune None;
///     }
/// }
/// assert_eq!(setup::ALL.len(), 1);
/// ```
#[macro_export]
macro_rules! purposes {
    (system $sys:ident; retired [$($r:expr),* $(,)?]; $($num:literal $id:ident $name:literal $about:literal subject $subj:ident fortune $fort:ident;)*) => {
        $(
            #[doc = $about]
            pub const $id: $crate::chance::Purpose = $crate::chance::Purpose {
                system: $crate::chance::systems::$sys,
                number: $num,
                name: $name,
                about: $about,
                subject: $crate::chance::SubjectKind::$subj,
                fortune: $crate::chance::Fortune::$fort,
            };
        )*
        /// Every purpose of this system, in number order.
        pub const ALL: &[$crate::chance::Purpose] = &[$($id),*];
        /// Numbers of removed purposes, never reused (A3.3).
        pub const RETIRED: &[u32] = &[$($r),*];
    };
}

/// Fails on a repeated system and number, a repeated name or a reused retired number (A3.3); `kd-sim` runs it over
/// every crate's list from α03c.
pub fn check_registry(lists: &[&[Purpose]], retired: &[(u32, &[u32])]) -> Result<(), String> {
    let mut seen: Vec<(u32, u32, &str)> = Vec::new();
    for p in lists.iter().flat_map(|l| l.iter()) {
        if p.number == 0 || p.number > 65_535 || p.system == 0 || p.system > 65_535 {
            return Err(format!("{}: system and purpose numbers run from 1 to 65,535", p.name));
        }
        if let Some(q) = seen.iter().find(|q| q.0 == p.system && q.1 == p.number) {
            return Err(format!(
                "{} and {} share system {} number {}",
                q.2, p.name, p.system, p.number
            ));
        }
        if seen.iter().any(|q| q.2 == p.name) {
            return Err(format!("the name {} is used twice", p.name));
        }
        for (system, numbers) in retired {
            if *system == p.system && numbers.contains(&p.number) {
                return Err(format!(
                    "{} reuses retired number {} of system {}",
                    p.name, p.number, p.system
                ));
            }
        }
        seen.push((p.system, p.number, p.name));
    }
    Ok(())
}
