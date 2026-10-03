//! Keyed chance (A3.3): every draw is a pure function of its key (the world, the system, the purpose, the subject
//! and the moment) with no generator position, so threads, work order and skipped draws never change another draw.
//!
//! Implements TIM-16, see A3.3: each chance event belongs to one being and one moment.

use crate::num::{GOLDEN, mix64};
use crate::time::GameTime;

pub mod registry;

const P0: u64 = 0xa076_1d64_78bd_642f;
const P1: u64 = 0xe703_7ed1_a0b4_28db;
/// Bit 31 of the purpose marks fortune's retry of a draw (A3.3).
pub const RETRY_BIT: u32 = 1 << 31;

#[inline]
fn wymum(a: u64, b: u64) -> (u64, u64) {
    let r = u128::from(a) * u128::from(b);
    (r as u64, (r >> 64) as u64)
}

/// The guarded product: each half folded back into its factor, removing plain wyhash's zero flaw (B02).
#[inline]
fn wymum_safe(a: u64, b: u64) -> (u64, u64) {
    let (lo, hi) = wymum(a, b);
    (a ^ lo, b ^ hi)
}

/// A purpose's seed in one world, worked out once and cached in its `Stream` (A3.3).
pub fn stream_seed(world: u64, system: u32, purpose: u32) -> u64 {
    let mut h = mix64(world.wrapping_add(GOLDEN));
    h = mix64(h ^ u64::from(system).wrapping_add(GOLDEN.wrapping_mul(2)));
    h = mix64(h ^ u64::from(purpose).wrapping_add(GOLDEN.wrapping_mul(3)));
    let (lo, hi) = wymum(h ^ P0, P1);
    h ^ (lo ^ hi)
}

/// One draw: 64 bits from a stream's seed, the subject's uid and the moment (A3.3).
#[inline]
pub fn draw(seed: u64, subject: u64, moment: u64) -> u64 {
    let (a, b) = wymum_safe(subject ^ P1, moment ^ seed);
    let (c, d) = wymum_safe(a ^ P0 ^ 16, b ^ P1);
    c ^ d
}

/// A draw's moment: the game second, and a slot (0–65,535) that separates several draws by one subject for one
/// purpose in one second.
#[inline]
pub fn moment(t: GameTime, slot: u16) -> u64 {
    (t.0 << 16) + u64::from(slot)
}

/// The top 24 bits of a draw as a float in [0, 1), exactly (A3.2).
#[inline]
pub fn unit_of(draw: u64) -> f32 {
    (draw >> 40) as f32 * (1.0 / 16_777_216.0)
}

/// A whole number below `n` from a draw's top 32 bits, by multiplication rather than remainder (A3.3).
#[inline]
pub fn below_of(draw: u64, n: u32) -> u32 {
    (((draw >> 32) * u64::from(n)) >> 32) as u32
}

/// The four 16-bit quarters of one draw summed, less 2, times 1.732: mean 0 and spread 1, within ±3.46 (A3.3).
#[inline]
pub fn normal_of(draw: u64) -> f32 {
    let quarters = (draw & 0xffff) + ((draw >> 16) & 0xffff) + ((draw >> 32) & 0xffff) + (draw >> 48);
    (quarters as f32 * (1.0 / 65_536.0) - 2.0) * 1.732
}

/// One purpose's draws in one world: the cached seed of `stream_seed`, and A3.3's helpers on it, each taking the
/// subject's uid and the moment.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Stream {
    pub seed: u64,
}

impl Stream {
    pub fn new(world: u64, purpose: &Purpose) -> Stream {
        Stream {
            seed: stream_seed(world, purpose.system.number, purpose.number),
        }
    }

    /// Fortune's retry of the same purpose: bit 31 of the purpose flipped (A3.3).
    pub fn retry(world: u64, purpose: &Purpose) -> Stream {
        Stream {
            seed: stream_seed(world, purpose.system.number, purpose.number ^ RETRY_BIT),
        }
    }

    #[inline]
    pub fn draw(self, subject: u64, moment: u64) -> u64 {
        draw(self.seed, subject, moment)
    }

    /// A float in [0, 1) with 24 bits.
    #[inline]
    pub fn unit(self, subject: u64, moment: u64) -> f32 {
        unit_of(self.draw(subject, moment))
    }

    /// True with probability `p`: never at 0, always at 1.
    #[inline]
    pub fn chance(self, subject: u64, moment: u64, p: f32) -> bool {
        self.unit(subject, moment) < p
    }

    /// A whole number in 0..n.
    #[inline]
    pub fn below(self, subject: u64, moment: u64, n: u32) -> u32 {
        below_of(self.draw(subject, moment), n)
    }

    /// A float from `lo` to `hi`: `lo + (hi − lo) × unit`.
    #[inline]
    pub fn range(self, subject: u64, moment: u64, lo: f32, hi: f32) -> f32 {
        lo + (hi - lo) * self.unit(subject, moment)
    }

    /// An index chosen in proportion to its weight, the weights added in index order; weights must not be
    /// negative and must not all be zero.
    pub fn pick_weighted(self, subject: u64, moment: u64, weights: &[f32]) -> usize {
        let total: f32 = weights.iter().fold(0.0, |a, &w| a + w);
        debug_assert!(
            total > 0.0 && weights.iter().all(|&w| w >= 0.0),
            "pick_weighted needs weights above 0"
        );
        let target = self.unit(subject, moment) * total;
        let mut acc = 0.0f32;
        let mut last = 0;
        for (i, &w) in weights.iter().enumerate() {
            if w > 0.0 {
                acc += w;
                last = i;
                if target < acc {
                    return i;
                }
            }
        }
        last
    }

    /// A normal-like float, mean 0 and spread 1, within ±3.46.
    #[inline]
    pub fn normal(self, subject: u64, moment: u64) -> f32 {
        normal_of(self.draw(subject, moment))
    }
}

/// A system that draws, numbered once and for ever in `systems` (A3.3).
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct System {
    pub number: u32,
    pub name: &'static str,
}

/// Every system that draws, by number; a new system takes the next number (A3.3).
pub mod systems {
    use super::System;

    pub const WORLD_GEN: System = System {
        number: 1,
        name: "world_gen",
    };
    pub const WEATHER: System = System {
        number: 2,
        name: "weather",
    };
    pub const WATER_LAND: System = System {
        number: 3,
        name: "water_land",
    };
    pub const PLANTS: System = System {
        number: 4,
        name: "plants",
    };
    pub const ANIMALS: System = System {
        number: 5,
        name: "animals",
    };
    pub const ILLNESS: System = System {
        number: 6,
        name: "illness",
    };
    pub const FIRE: System = System {
        number: 7,
        name: "fire",
    };
    pub const THINGS: System = System {
        number: 8,
        name: "things",
    };
    pub const BODIES: System = System {
        number: 9,
        name: "bodies",
    };
    pub const MINDS: System = System {
        number: 10,
        name: "minds",
    };
    pub const CULTURE: System = System {
        number: 11,
        name: "culture",
    };
    pub const POWERS: System = System {
        number: 12,
        name: "powers",
    };
    pub const SET_UP: System = System {
        number: 13,
        name: "set_up",
    };

    pub const ALL: [System; 13] = [
        WORLD_GEN, WEATHER, WATER_LAND, PLANTS, ANIMALS, ILLNESS, FIRE, THINGS, BODIES, MINDS, CULTURE, POWERS, SET_UP,
    ];
}

/// What a draw is about: the kind of uid its subject is (A3.3, A3.4); a pair purpose's subject is
/// `num::hash2(actor, other)`.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Subject {
    Person,
    Animal,
    Thing,
    Plant,
    Herd,
    Group,
    Place,
    Pair,
}

/// Which way fortune may turn a purpose's draws for a blessed or cursed subject (`GOD-04`, A3.3); A10 decides
/// which purposes it touches.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Fortune {
    Good,
    Bad,
    None,
}

/// One registered reason to draw within a system: its permanent number (1–65,535), its name, what it draws
/// about and its fortune polarity (A3.3).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Purpose {
    pub system: System,
    pub number: u32,
    /// The constant's name, `BLUEPRINT_TRY`; `name()` gives `things.blueprint_try`.
    pub ident: &'static str,
    pub about: &'static str,
    pub subject: Subject,
    pub fortune: Fortune,
}

impl Purpose {
    /// The purpose's full name: its system's name, a dot, its constant's name in lower case.
    pub fn name(&self) -> String {
        format!("{}.{}", self.system.name, self.ident.to_ascii_lowercase())
    }

    pub fn stream(&self, world: u64) -> Stream {
        Stream::new(world, self)
    }
}

/// One crate's purposes in one system, with the numbers it has retired, which are never used again (A3.3).
#[derive(Clone, Copy, Debug)]
pub struct PurposeList {
    pub system: System,
    pub purposes: &'static [Purpose],
    pub retired: &'static [u32],
}

/// Declares a system's purposes in a crate's `purposes.rs` (A3.3):
///
/// ```
/// mod things {
///     kd_core::purposes! {
///         system THINGS;
///         1 BLUEPRINT_TRY "a try at a blueprint succeeds" subject Person fortune Good;
///         2 CHIP "a flake chips off as it is struck" subject Thing fortune None;
///         retired [3];
///     }
/// }
/// assert_eq!(things::BLUEPRINT_TRY.name(), "things.blueprint_try");
/// assert_eq!((things::LIST.purposes.len(), things::RETIRED), (2, &[3][..]));
/// ```
///
/// Each line gives a constant of that name, `RETIRED` the retired numbers, and `LIST` joins them for the registry
/// test.
#[macro_export]
macro_rules! purposes {
    (
        system $system:ident;
        $( $number:literal $ident:ident $about:literal subject $subject:ident fortune $fortune:ident; )*
        retired [ $( $retired:literal ),* $(,)? ];
    ) => {
        $(
            pub const $ident: $crate::chance::Purpose = $crate::chance::Purpose {
                system: $crate::chance::systems::$system,
                number: $number,
                ident: stringify!($ident),
                about: $about,
                subject: $crate::chance::Subject::$subject,
                fortune: $crate::chance::Fortune::$fortune,
            };
        )*
        /// The numbers this system's crate has retired, never to be used again.
        pub const RETIRED: &[u32] = &[ $( $retired ),* ];
        pub const LIST: $crate::chance::PurposeList = $crate::chance::PurposeList {
            system: $crate::chance::systems::$system,
            purposes: &[ $( $ident ),* ],
            retired: RETIRED,
        };
    };
}

#[cfg(test)]
mod tests {
    use super::*;

    mod demo {
        crate::purposes! {
            system THINGS;
            1 TRY "a try succeeds" subject Person fortune Good;
            2 SPLIT "a stone splits" subject Thing fortune None;
            retired [];
        }
    }

    // checks: TIM-16
    #[test]
    fn known_answers() {
        // Worked out apart from this code, from A3.3's definitions.
        assert_eq!(stream_seed(0, 0, 0), 0x2eb0_5158_dad9_abb4);
        assert_eq!(stream_seed(1, 8, 1), 0x08b1_91f2_235e_5c97);
        assert_eq!(draw(0, 0, 0), 0x31f3_90d6_2722_7fb9);
        assert_eq!(
            draw(0x1234_5678_9abc_def0, 42, moment(GameTime(86_400), 3)),
            0x56b2_3ebb_074d_0b2e
        );
        let s = demo::TRY.stream(1);
        assert_eq!(s.seed, stream_seed(1, 8, 1));
        assert_eq!(Stream::retry(1, &demo::TRY).seed, stream_seed(1, 8, 1 | RETRY_BIT));
        assert_eq!(moment(GameTime(1), 2), 65_538);
        assert_eq!(
            (demo::TRY.name(), demo::SPLIT.name()),
            ("things.try".to_string(), "things.split".to_string())
        );
        assert_eq!(registry::check(&[demo::LIST]), Ok(()));
    }

    // checks: TIM-16 RES-05
    #[test]
    fn stored_draws() {
        // 10,000 draws over fixed keys give the bits stored on x86-64, on every target (A15.9 item 5).
        crate::bits::tests::compare("chance.draws");
    }

    /// The chi-square of counts in equal bins, against an equal share each, times the share: exact in integers.
    fn chi_square_times_share(counts: &[u64], n: u64) -> u64 {
        let share = n / counts.len() as u64;
        counts.iter().map(|&c| (c as i64 - share as i64).pow(2) as u64).sum()
    }

    // checks: TIM-16
    #[test]
    fn uniform_in_64_bins() {
        // One million units in 64 bins along moments and along neighbouring subjects: chi-square with 63 degrees
        // of freedom below 103.4, its 0.1% tail.
        const N: u64 = 1 << 20;
        let s = demo::TRY.stream(7);
        let (mut by_moment, mut by_subject) = ([0u64; 64], [0u64; 64]);
        for i in 0..N {
            by_moment[(s.unit(99, i) * 64.0) as usize] += 1;
            by_subject[(s.unit(i, 1 << 16) * 64.0) as usize] += 1;
        }
        let limit = 103.4 * (N / 64) as f32;
        for (what, counts) in [("moments", by_moment), ("subjects", by_subject)] {
            let x = chi_square_times_share(&counts, N) as f32;
            assert!(x < limit, "along {what}: chi-square {}", x / (N / 64) as f32);
        }
    }

    // checks: TIM-16
    #[test]
    fn helpers_in_range() {
        let s = demo::SPLIT.stream(3);
        for i in 0..20_000u64 {
            let u = s.unit(5, i);
            assert!((0.0..1.0).contains(&u));
            for n in [1u32, 2, 3, 1000, u32::MAX] {
                assert!(s.below(5, i, n) < n);
            }
            let r = s.range(5, i, -2.5, 4.0);
            assert!((-2.5..=4.0).contains(&r));
            assert!(!s.chance(5, i, 0.0) && s.chance(5, i, 1.0));
            let k = s.pick_weighted(5, i, &[0.0, 1.0, 0.0, 3.0, 0.0]);
            assert!(k == 1 || k == 3, "{k}");
            let z = s.normal(5, i);
            assert!((-3.464..=3.464).contains(&z), "{z}");
        }
        // The extremes of each helper's input map inside their ranges.
        assert_eq!((unit_of(0), unit_of(u64::MAX)), (0.0, 16_777_215.0 / 16_777_216.0));
        assert_eq!((below_of(0, 10), below_of(u64::MAX, 10)), (0, 9));
        assert_eq!(
            (normal_of(0), normal_of(u64::MAX)),
            (-3.464, (4.0 * 65_535.0 / 65_536.0 - 2.0) * 1.732)
        );
        // Weights picked in proportion: 1 in 4 against 3 in 4.
        let ones = (0..40_000u64)
            .filter(|&i| s.pick_weighted(8, i, &[1.0, 3.0]) == 0)
            .count();
        assert!((9_500..10_500).contains(&ones), "{ones}");
    }

    // checks: TIM-16
    #[test]
    fn normal_mean_and_spread() {
        // Test statistics in f64 (A3.2 allows them).
        let s = demo::TRY.stream(11);
        let n = 1_000_000u64;
        let (mut sum, mut sq) = (0.0f64, 0.0f64);
        for i in 0..n {
            let z = f64::from(s.normal(i, 0));
            sum += z;
            sq += z * z;
        }
        let mean = sum / n as f64;
        let spread = (sq / n as f64 - mean * mean).sqrt();
        assert!(mean.abs() < 0.005, "mean {mean}");
        assert!((spread - 1.0).abs() < 0.005, "spread {spread}");
    }
}
