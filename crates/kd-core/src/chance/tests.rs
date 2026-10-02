use super::b02_reference as b02;
use super::hash::{GOLDEN, mix64};
use super::purposes::TEST_DRAW;
use super::stream::retry_number;
use super::*;
use crate::selfcheck;
use crate::time::GameTime;

/// The chi-square bound at 0.999 for 63 degrees of freedom (A3.3).
const CHI2_999_63: f64 = 103.4;

fn chi_square(bins: &[u64; 64], n: u64) -> f64 {
    let e = n as f64 / 64.0;
    bins.iter().map(|&b| (b as f64 - e) * (b as f64 - e) / e).sum()
}

// checks: TIM-16 RES-05
#[test]
fn splitmix_reference() {
    assert_eq!(mix64(GOLDEN), 0xe220_a839_7b1d_cdaf);
}

// checks: TIM-16 RES-05
#[test]
fn port_matches_b02_reference() {
    for i in 0..10_000u64 {
        let (world, system, purpose) = (mix64(i), (i % 13 + 1) as u32, (i % 97 + 1) as u32);
        let (subject, m) = (mix64(i + 7), ((i * 7_919) << 16) | (i % 5));
        let reference = b02::Stream::new(world, system, purpose);
        assert_eq!(stream_seed(world, system, purpose), reference.wy_seed);
        assert_eq!(
            draw(reference.wy_seed, subject, m),
            b02::WySafe::new(&reference).draw(subject, m)
        );
    }
}

// checks: TIM-16 RES-05
#[test]
fn stored_draws_match() {
    assert_eq!(
        selfcheck::draws_hash(),
        selfcheck::DRAWS_HASH,
        "update DRAWS_HASH to {:#x}",
        selfcheck::draws_hash()
    );
    assert!(!selfcheck::core_check().contains(&"draws"));
}

// checks: TIM-16
#[test]
fn guarded_against_zero_product() {
    let st = b02::Stream::new(1, 2, 3);
    let m = st.wy_seed;
    assert_eq!(b02::wy_draw(st.wy_seed, 1, m), b02::wy_draw(st.wy_seed, 2, m));
    assert_ne!(draw(st.wy_seed, 1, m), draw(st.wy_seed, 2, m));
}

// checks: TIM-16
#[test]
fn retry_key_independent() {
    for i in 0..10_000u64 {
        let p = Purpose {
            number: (i % 97 + 1) as u32,
            ..TEST_DRAW
        };
        let world = mix64(i);
        let (a, b) = (Stream::new(world, p), Stream::retry(world, p));
        assert_ne!(a.draw(i, i << 16), b.draw(i, i << 16));
    }
    assert_eq!(retry_number(3), 0x8000_0003);
}

// checks: TIM-16
#[test]
fn units_uniform_along_moments() {
    let s = Stream::new(42, TEST_DRAW);
    let mut bins = [0u64; 64];
    let n = 1_000_000u64;
    for t in 0..n {
        bins[(s.unit(7, moment(GameTime(t), 0)) * 64.0) as usize] += 1;
    }
    let chi = chi_square(&bins, n);
    assert!(chi < CHI2_999_63, "chi-square {chi}");
}

// checks: TIM-16
#[test]
fn units_uniform_along_subjects() {
    let s = Stream::new(42, TEST_DRAW);
    let mut bins = [0u64; 64];
    let n = 1_000_000u64;
    for subject in 0..n {
        bins[(s.unit(subject, moment(GameTime(1_000), 3)) * 64.0) as usize] += 1;
    }
    let chi = chi_square(&bins, n);
    assert!(chi < CHI2_999_63, "chi-square {chi}");
}

// checks: TIM-16
#[test]
fn below_stays_below() {
    let s = Stream::new(9, TEST_DRAW);
    for n in [1u32, 7, u32::MAX] {
        for i in 0..100_000u64 {
            assert!(s.below(i, i << 16, n) < n);
        }
    }
    assert_eq!(s.below(1, 2, 1), 0);
}

// checks: TIM-16
#[test]
fn normal_mean_spread_bounds() {
    let s = Stream::new(5, TEST_DRAW);
    let n = 1_000_000u64;
    let (mut sum, mut sq) = (0.0f64, 0.0f64);
    for i in 0..n {
        let x = s.normal(i, 0) as f64;
        assert!(x.abs() <= 3.4641, "{x}");
        sum += x;
        sq += x * x;
    }
    let mean = sum / n as f64;
    let spread = (sq / n as f64 - mean * mean).sqrt();
    assert!(mean.abs() < 0.005, "mean {mean}");
    assert!((spread - 1.0).abs() < 0.01, "spread {spread}");
}

// checks: TIM-16
#[test]
fn registry_rejects_repeats() {
    let ok: &[Purpose] = super::purposes::ALL;
    assert_eq!(check_registry(&[ok], &[]), Ok(()));
    let same_number = [
        TEST_DRAW,
        Purpose {
            name: "setup.other",
            ..TEST_DRAW
        },
    ];
    assert!(check_registry(&[&same_number], &[]).is_err());
    let same_name = [TEST_DRAW, Purpose { number: 2, ..TEST_DRAW }];
    assert!(check_registry(&[&same_name], &[]).is_err());
    let retired: &[u32] = &[1];
    assert!(check_registry(&[ok], &[(systems::SETUP, retired)]).is_err());
    let other_system = [
        TEST_DRAW,
        Purpose {
            system: systems::THINGS,
            name: "things.test",
            ..TEST_DRAW
        },
    ];
    assert_eq!(check_registry(&[&other_system], &[(systems::THINGS, &[2])]), Ok(()));
}

// checks: TIM-16
#[test]
fn pick_weighted_order_and_zero() {
    let s = Stream::new(3, TEST_DRAW);
    assert_eq!(s.pick_weighted(1, 2, &[0.0, 0.0]), 0);
    for i in 0..1_000u64 {
        assert_eq!(s.pick_weighted(i, 0, &[0.0, 1.0, 0.0]), 1);
        let r = s.range(i, 0, 2.0, 3.0);
        assert!((2.0..3.0).contains(&r));
    }
}
