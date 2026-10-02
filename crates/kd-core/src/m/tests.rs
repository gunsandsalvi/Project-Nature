use crate::selfcheck;

// checks: RES-05
#[test]
fn m_bits_stored() {
    let got = selfcheck::m_hashes();
    assert_eq!(got, selfcheck::M_HASHES, "update M_HASHES to {got:#x?}");
    assert!(selfcheck::core_check().iter().all(|f| !f.starts_with("m::")));
}
