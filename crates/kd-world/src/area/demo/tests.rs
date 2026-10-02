use super::*;

// checks: WLD-12 PRE-23
#[test]
fn window_holds_the_cliff() {
    let a = demo_area(99);
    let (lo, hi) = (a.heights.iter().min().unwrap(), a.heights.iter().max().unwrap());
    assert_eq!(*lo, 0);
    assert!(hi - lo > 250, "the window spans {} dm", hi - lo);
    // the cliff shows rock, scree lies at its foot, and soil covers the rest
    for m in [SOIL, SCREE] {
        assert!(a.material.contains(&m), "no material {m}");
    }
    assert!(a.material.iter().any(|m| (1..=6).contains(m)), "no rock shows");
    assert_eq!(a.heights.len(), SIDE * SIDE);
    assert_eq!(a.material.len(), SQUARES * SQUARES);
}
