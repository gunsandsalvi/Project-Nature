use super::*;

/// A copy of B11's `data::hash`, the byte hash its recorded hashes used (A5.3's porting check).
#[allow(clippy::needless_range_loop)]
fn b11_hash(b: &[u8]) -> u64 {
    const K: [u64; 4] = [
        0x9E37_79B9_7F4A_7C15,
        0xC2B2_AE3D_27D4_EB4F,
        0x1656_67B1_9E37_79F9,
        0x27D4_EB2F_1656_67C5,
    ];
    let mut s = [K[0] ^ b.len() as u64, K[1], K[2], K[3]];
    let mut ch = b.chunks_exact(32);
    for c in &mut ch {
        for l in 0..4 {
            let w = u64::from_le_bytes(c[l * 8..l * 8 + 8].try_into().unwrap());
            s[l] = (s[l] ^ w).wrapping_mul(K[l] | 1).rotate_left(29);
        }
    }
    let mut tail = [0u8; 32];
    let rem = ch.remainder();
    tail[..rem.len()].copy_from_slice(rem);
    for l in 0..4 {
        let w = u64::from_le_bytes(tail[l * 8..l * 8 + 8].try_into().unwrap());
        s[l] = (s[l] ^ w).wrapping_mul(K[l] | 1).rotate_left(29);
    }
    mix(mix(s[0] ^ mix(s[1])) ^ mix(s[2] ^ mix(s[3])))
}

// checks: WLD-12 TIM-16
#[test]
fn b11_detail_hash() {
    // B11's metre-detail test patch: the same height map and 1,860 pieces, bit for bit (results/detail.txt)
    let site = Site::new(99, [310.0, 342.0, 365.0, 330.0]);
    let hp = build_height_pieces(&site);
    assert_eq!(hp.pieces.len(), 1_860);
    assert_eq!(hp.hash(b11_hash), 0x5e3b_0c48_2d78_9a49);
}
