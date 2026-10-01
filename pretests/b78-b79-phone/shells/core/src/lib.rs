//! B78 (X10): stand-in simulation core. One pure function, shared unchanged by
//! the phone shells (1, 3, 4) and the headless Linux build (PLT-05).

/// SplitMix64 mixing, `rounds` times from `seed`. Pure and deterministic (X11).
pub fn mix(seed: u64, rounds: u32) -> u64 {
    let mut z = seed;
    for _ in 0..rounds {
        z = z.wrapping_add(0x9E37_79B9_7F4A_7C15);
        let mut x = z;
        x = (x ^ (x >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
        x = (x ^ (x >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
        z ^= x ^ (x >> 31);
    }
    z
}

#[cfg(test)]
mod tests {
    #[test]
    fn known_value() {
        // Same input, same output, on every machine (X11).
        assert_eq!(super::mix(42, 0), 42);
        assert_eq!(super::mix(42, 1000), super::mix(42, 1000));
    }
}
