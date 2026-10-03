use super::*;

// checks: WLD-12
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
    // scree lies at the cliff's foot: every scree square within 8 m of a rock square (6 m below the foot, which lies
    // 2 m out from the edge line)
    let rock = |i: usize, j: usize| (1..=6).contains(&a.material[j * SQUARES + i]);
    for k in (0..SQUARES * SQUARES).filter(|&k| a.material[k] == SCREE) {
        let (i, j) = (k % SQUARES, k / SQUARES);
        let near = (i.saturating_sub(8)..(i + 9).min(SQUARES))
            .any(|x| (j.saturating_sub(8)..(j + 9).min(SQUARES)).any(|y| rock(x, y)));
        assert!(near, "scree at ({i}, {j}) lies far from the cliff");
    }
    assert_eq!(a.heights.len(), SIDE * SIDE);
    assert_eq!(a.material.len(), SQUARES * SQUARES);
}
