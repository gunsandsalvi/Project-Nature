//! The banned fixture (A2.3 rule 6, A3.2, A15.1): each entry of the root clippy.toml used exactly once,
//! in its order; tools/check-banned.sh fails unless clippy flags every one. Never part of the game.
//! Implements PRN-14 in part (the bans can't silently ban nothing).

// disallowed-types
pub fn t1() -> usize { std::collections::HashMap::<u8, u8>::new().len() }
pub fn t2() -> usize { std::collections::HashSet::<u8>::new().len() }
pub fn t3() -> u8 { std::cell::Cell::new(1u8).get() }
pub fn t4() -> u8 { *std::cell::RefCell::new(1u8).borrow() }
pub fn t5() -> bool { std::cell::OnceCell::<u8>::new().get().is_none() }
pub fn t6() -> bool { std::sync::Mutex::new(1u8).lock().is_ok() }
pub fn t7() -> bool { std::sync::RwLock::new(1u8).read().is_ok() }
pub fn t8() -> bool { std::sync::atomic::AtomicBool::new(true).into_inner() }
pub fn t9() -> u32 { std::sync::atomic::AtomicU32::new(1).into_inner() }
pub fn t10() -> u64 { std::sync::atomic::AtomicU64::new(1).into_inner() }
pub fn t11() -> usize { std::sync::atomic::AtomicUsize::new(1).into_inner() }
pub fn t12() -> u128 { std::time::Instant::now().elapsed().as_nanos() }
pub fn t13() -> bool { std::time::SystemTime::now().elapsed().is_ok() }
pub fn t14() -> bool { std::fs::File::open("x").is_ok() }

// disallowed-methods
pub fn m15(x: f32) -> f32 { x.sin() }
pub fn m16(x: f32) -> f32 { x.cos() }
pub fn m17(x: f32) -> f32 { x.tan() }
pub fn m18(x: f32) -> f32 { x.asin() }
pub fn m19(x: f32) -> f32 { x.acos() }
pub fn m20(x: f32) -> f32 { x.atan() }
pub fn m21(x: f32) -> f32 { x.atan2(1.0) }
pub fn m22(x: f32) -> f32 { x.sin_cos().0 }
pub fn m23(x: f32) -> f32 { x.sinh() }
pub fn m24(x: f32) -> f32 { x.cosh() }
pub fn m25(x: f32) -> f32 { x.tanh() }
pub fn m26(x: f32) -> f32 { x.exp() }
pub fn m27(x: f32) -> f32 { x.exp2() }
pub fn m28(x: f32) -> f32 { x.exp_m1() }
pub fn m29(x: f32) -> f32 { x.ln() }
pub fn m30(x: f32) -> f32 { x.ln_1p() }
pub fn m31(x: f32) -> f32 { x.log(2.0) }
pub fn m32(x: f32) -> f32 { x.log2() }
pub fn m33(x: f32) -> f32 { x.log10() }
pub fn m34(x: f32) -> f32 { x.powf(2.0) }
pub fn m35(x: f32) -> f32 { x.powi(2) }
pub fn m36(x: f32) -> f32 { x.hypot(1.0) }
pub fn m37(x: f32) -> f32 { x.cbrt() }
pub fn m38(x: f32) -> f32 { x.mul_add(2.0, 1.0) }
pub fn m39(x: f32) -> f32 { x.min(1.0) }
pub fn m40(x: f32) -> f32 { x.max(1.0) }
pub fn m41() { let _ = std::thread::spawn(|| {}); }
pub fn m42() -> bool { std::env::var("X").is_ok() }
pub fn m43() -> bool { std::fs::read("x").is_ok() }
pub fn m44() -> bool { std::fs::write("x", b"").is_ok() }
