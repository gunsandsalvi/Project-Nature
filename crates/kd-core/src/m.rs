//! Maths only through `libm`, the same bits on every target (A3.2). Implements `RES-05` in part.
//! The standard library's versions call each platform's maths library and are banned in simulation crates.

macro_rules! wrap1 {
    ($($name:ident => $f:ident),* $(,)?) => {
        $(
            #[doc = concat!("`", stringify!($name), "` through `libm::", stringify!($f), "` (A3.2).")]
            #[inline]
            pub fn $name(x: f32) -> f32 {
                libm::$f(x)
            }
        )*
    };
}

wrap1! {
    sin => sinf, cos => cosf, tan => tanf, asin => asinf, acos => acosf, atan => atanf,
    exp => expf, exp2 => exp2f, ln => logf, log2 => log2f, log10 => log10f, cbrt => cbrtf, tanh => tanhf,
}

/// `atan2(y, x)` through `libm::atan2f` (A3.2).
#[inline]
pub fn atan2(y: f32, x: f32) -> f32 {
    libm::atan2f(y, x)
}

/// `x` to the power `y` through `libm::powf` (A3.2).
#[inline]
pub fn powf(x: f32, y: f32) -> f32 {
    libm::powf(x, y)
}

/// `sqrt(x² + y²)` through `libm::hypotf` (A3.2).
#[inline]
pub fn hypot(x: f32, y: f32) -> f32 {
    libm::hypotf(x, y)
}
