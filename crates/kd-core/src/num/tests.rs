use super::*;

// checks: RES-05
#[test]
fn hash64_known_answer() {
    assert_eq!(hash64(b""), 0x2d06_8005_38d3_94c2);
}

// checks: RES-05
#[test]
fn min_max_never_split_zero() {
    assert_eq!(min(-0.0, 0.0).to_bits(), 0.0f32.to_bits());
    assert_eq!(min(0.0, -0.0).to_bits(), (-0.0f32).to_bits());
    assert_eq!(max(-0.0, 0.0).to_bits(), 0.0f32.to_bits());
    assert_eq!(max(0.0, -0.0).to_bits(), (-0.0f32).to_bits());
    assert_eq!(min(1.0, 2.0), 1.0);
    assert_eq!(max(1.0, 2.0), 2.0);
}

// checks: RES-05
#[test]
fn clean_turns_negative_zero_positive() {
    assert_eq!(clean(-0.0).to_bits(), 0.0f32.to_bits());
    assert_eq!(clean(1.5), 1.5);
}
