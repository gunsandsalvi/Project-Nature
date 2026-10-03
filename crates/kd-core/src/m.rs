//! The only maths the simulation may call (A3.2): `libm`'s single-precision functions, written in plain Rust, so
//! they give the same bits on the phone, in the browser and in the cloud, where each platform's own maths library
//! differs in the last bits (B01). The standard versions are banned in the simulation crates (`clippy.toml`).
//!
//! Implements RES-05, see A3.2.

#[inline]
pub fn sin(x: f32) -> f32 {
    libm::sinf(x)
}

#[inline]
pub fn cos(x: f32) -> f32 {
    libm::cosf(x)
}

#[inline]
pub fn tan(x: f32) -> f32 {
    libm::tanf(x)
}

#[inline]
pub fn asin(x: f32) -> f32 {
    libm::asinf(x)
}

#[inline]
pub fn acos(x: f32) -> f32 {
    libm::acosf(x)
}

#[inline]
pub fn atan(x: f32) -> f32 {
    libm::atanf(x)
}

/// The angle of the point (x, y), in (−π, π].
#[inline]
pub fn atan2(y: f32, x: f32) -> f32 {
    libm::atan2f(y, x)
}

#[inline]
pub fn exp(x: f32) -> f32 {
    libm::expf(x)
}

#[inline]
pub fn exp2(x: f32) -> f32 {
    libm::exp2f(x)
}

/// The natural logarithm.
#[inline]
pub fn ln(x: f32) -> f32 {
    libm::logf(x)
}

#[inline]
pub fn log2(x: f32) -> f32 {
    libm::log2f(x)
}

#[inline]
pub fn log10(x: f32) -> f32 {
    libm::log10f(x)
}

/// `x` to the power `y`.
#[inline]
pub fn powf(x: f32, y: f32) -> f32 {
    libm::powf(x, y)
}

/// The length of (x, y), without overflow on the way.
#[inline]
pub fn hypot(x: f32, y: f32) -> f32 {
    libm::hypotf(x, y)
}

#[inline]
pub fn cbrt(x: f32) -> f32 {
    libm::cbrtf(x)
}

#[inline]
pub fn tanh(x: f32) -> f32 {
    libm::tanhf(x)
}

#[cfg(test)]
mod tests {
    // checks: RES-05
    #[test]
    fn stored_bits() {
        // Each function at its 1,000 inputs gives the bits stored on x86-64; this runs on x86-64, on arm64 under
        // qemu, and its hashes in the browser (A15.9 item 5).
        for name in crate::bits::names().into_iter().filter(|n| n.starts_with("m.")) {
            crate::bits::tests::compare(name);
        }
    }
}
