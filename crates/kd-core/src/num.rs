//! Numbers that repeat bit for bit on every target (A3.2).

/// The smaller of two floats, the same bits on every target (A3.2): never `f32::min`.
/// Implements `RES-05` in part, see A3.2.
#[inline]
pub fn min(a: f32, b: f32) -> f32 {
    if a < b { a } else { b }
}

/// The larger of two floats, the same bits on every target (A3.2): never `f32::max`.
/// Implements `RES-05` in part, see A3.2.
#[inline]
pub fn max(a: f32, b: f32) -> f32 {
    if a > b { a } else { b }
}

/// Turns −0.0 into +0.0 and changes nothing else; used by every store write and every float fed to a hash (A3.2).
#[inline]
pub fn clean(x: f32) -> f32 {
    debug_assert!(x.is_finite(), "non-finite float stored or hashed (A3.2)");
    x + 0.0
}

/// XXH3-64 of bytes: state hashes, chunk integrity and the catalogue (A3.2).
#[inline]
pub fn hash64(b: &[u8]) -> u64 {
    xxhash_rust::xxh3::xxh3_64(b)
}

#[cfg(test)]
mod tests;
