//! kd-core's part of the self-check (A15.4, A3.2, A3.3): stored draws and the maths functions, which must give the
//! same bits on x86-64, arm64 and wasm32. Implements `RES-05` and `PRC-11` in part.

use crate::chance::hash::{draw, mix64, stream_seed};
use crate::m;
use crate::num::hash64;

/// The hash of the 10,000 stored draws (see `draws_hash`), recorded on x86-64 and matched on every target.
pub const DRAWS_HASH: u64 = 0x826d_134c_4b26_75dc;

/// The hash of each `m` function's outputs at its 1,000 fixed inputs (see `m_hashes`).
pub const M_HASHES: [(&str, u64); 16] = [
    ("sin", 0x2b6c_00a3_bcbd_e362),
    ("cos", 0x1a2e_92a3_3c81_03ea),
    ("tan", 0xeed6_83de_4498_72e9),
    ("asin", 0xd813_981c_21a9_4b4e),
    ("acos", 0x8633_e283_fdbe_5eb1),
    ("atan", 0x1d01_aa2b_24cb_0d81),
    ("atan2", 0xd9ec_f85c_77f4_0f05),
    ("exp", 0xe96b_055f_edbc_e278),
    ("exp2", 0x2fb4_4a7b_5642_57fa),
    ("ln", 0xf8a7_64f7_2706_f167),
    ("log2", 0xe307_7e60_3460_cc07),
    ("log10", 0xe298_21bb_4831_a2a8),
    ("powf", 0x0355_c231_c350_5e8e),
    ("hypot", 0xae62_c992_499d_da4f),
    ("cbrt", 0x9b9d_4f1a_c115_49f4),
    ("tanh", 0x6161_c8c1_92ef_3663),
];

/// The 10,000 stored draws hashed: key `i` is world `mix64(i)`, system `i % 13 + 1`, purpose `i % 97 + 1`,
/// subject `mix64(i + 7)`, moment `(i × 7,919) << 16 | i % 5`.
pub fn draws_hash() -> u64 {
    let mut bytes = Vec::with_capacity(80_000);
    for i in 0..10_000u64 {
        let seed = stream_seed(mix64(i), (i % 13 + 1) as u32, (i % 97 + 1) as u32);
        bytes.extend_from_slice(&draw(seed, mix64(i + 7), ((i * 7_919) << 16) | (i % 5)).to_le_bytes());
    }
    hash64(&bytes)
}

/// `x_i = lo + (hi − lo) × i / 999`, in `f32`.
fn lin(lo: f32, hi: f32, i: usize) -> f32 {
    lo + (hi - lo) * i as f32 / 999.0
}

/// `0.001 + 999.999 × (i / 999)²`: positive, spread toward small values.
fn pos(i: usize) -> f32 {
    let f = i as f32 / 999.0;
    0.001 + 999.999 * (f * f)
}

fn hash_outputs(f: impl Fn(usize) -> f32) -> u64 {
    let mut bytes = Vec::with_capacity(4_000);
    for i in 0..1_000 {
        bytes.extend_from_slice(&f(i).to_bits().to_le_bytes());
    }
    hash64(&bytes)
}

/// Each `m` function at 1,000 fixed inputs, its outputs' bits hashed.
pub fn m_hashes() -> [(&'static str, u64); 16] {
    let ang = |i| lin(-50.0, 50.0, i);
    let unit = |i| lin(-1.0, 1.0, i);
    [
        ("sin", hash_outputs(|i| m::sin(ang(i)))),
        ("cos", hash_outputs(|i| m::cos(ang(i)))),
        ("tan", hash_outputs(|i| m::tan(ang(i)))),
        ("asin", hash_outputs(|i| m::asin(unit(i)))),
        ("acos", hash_outputs(|i| m::acos(unit(i)))),
        ("atan", hash_outputs(|i| m::atan(ang(i)))),
        ("atan2", hash_outputs(|i| m::atan2(ang(i), ang(999 - i)))),
        ("exp", hash_outputs(|i| m::exp(lin(-20.0, 20.0, i)))),
        ("exp2", hash_outputs(|i| m::exp2(lin(-20.0, 20.0, i)))),
        ("ln", hash_outputs(|i| m::ln(pos(i)))),
        ("log2", hash_outputs(|i| m::log2(pos(i)))),
        ("log10", hash_outputs(|i| m::log10(pos(i)))),
        (
            "powf",
            hash_outputs(|i| m::powf(lin(0.01, 10.0, i), lin(-3.0, 3.0, 999 - i))),
        ),
        ("hypot", hash_outputs(|i| m::hypot(ang(i), ang(999 - i)))),
        ("cbrt", hash_outputs(|i| m::cbrt(pos(i)))),
        ("tanh", hash_outputs(|i| m::tanh(lin(-10.0, 10.0, i)))),
    ]
}

/// What fails among kd-core's checks: `"draws"` and `"m::<function>"`; empty when all pass.
pub fn core_check() -> Vec<&'static str> {
    let mut fail = Vec::new();
    if draws_hash() != DRAWS_HASH {
        fail.push("draws");
    }
    const NAMES: [&str; 16] = [
        "m::sin", "m::cos", "m::tan", "m::asin", "m::acos", "m::atan", "m::atan2", "m::exp", "m::exp2", "m::ln",
        "m::log2", "m::log10", "m::powf", "m::hypot", "m::cbrt", "m::tanh",
    ];
    for ((got, want), name) in m_hashes().iter().zip(M_HASHES.iter()).zip(NAMES) {
        if got.1 != want.1 {
            fail.push(name);
        }
    }
    fail
}
