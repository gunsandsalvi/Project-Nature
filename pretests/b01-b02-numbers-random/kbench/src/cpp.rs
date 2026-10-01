//! B01: the C++ kernels (cpp/kernels.cpp), compiled twice by build.rs:
//! `cpp_*` with the compiler's default float settings, `cppnc_*` with -ffp-contract=off.
use crate::rng::Stream;
use std::ffi::c_void;

type HeatF32 = unsafe extern "C" fn(*const f32, *mut f32, i32, i32, f32);
type HeatF64 = unsafe extern "C" fn(*const f64, *mut f64, i32, i32, f64);
type HeatFx32 = unsafe extern "C" fn(*const i32, *mut i32, i32, i32, i32);
type HeatFx64 = unsafe extern "C" fn(*const i64, *mut i64, i32, i32, i64);
type WalkStep =
    unsafe extern "C" fn(i32, i32, *const Stream, *const c_void, *mut u32, *mut u32, *mut c_void, u64, u64, u64);
type SumBlocks = unsafe extern "C" fn(i32, *const c_void, *mut c_void, u64, u64);
type SumTree = unsafe extern "C" fn(i32, *mut c_void, u64);
type SumNaive = unsafe extern "C" fn(i32, *const c_void, u64) -> u64;
type LearnStep = unsafe extern "C" fn(i32, *mut c_void, *const c_void, *const c_void, u64, u64, u64);
type RngSum = unsafe extern "C" fn(i32, *const Stream, u64, u64, u64) -> u64;
type RngOne = unsafe extern "C" fn(i32, *const Stream, u64, u64) -> u64;

pub struct CppApi {
    pub heat_f32: HeatF32,
    pub heat_f64: HeatF64,
    pub heat_fx32: HeatFx32,
    pub heat_fx64: HeatFx64,
    pub walk_step: WalkStep,
    pub sum_blocks: SumBlocks,
    pub sum_tree: SumTree,
    pub sum_naive: SumNaive,
    pub learn_step: LearnStep,
    pub rng_sum: RngSum,
    pub rng_one: RngOne,
}

extern "C" {
    fn cpp_heat_step_f32(s: *const f32, d: *mut f32, r0: i32, r1: i32, a: f32);
    fn cpp_heat_step_f64(s: *const f64, d: *mut f64, r0: i32, r1: i32, a: f64);
    fn cpp_heat_step_fx32(s: *const i32, d: *mut i32, r0: i32, r1: i32, a: i32);
    fn cpp_heat_step_fx64(s: *const i64, d: *mut i64, r0: i32, r1: i32, a: i64);
    fn cpp_walk_step(f: i32, g: i32, st: *const Stream, grid: *const c_void, px: *mut u32, py: *mut u32,
                     tally: *mut c_void, a0: u64, a1: u64, moment: u64);
    fn cpp_sum_blocks(f: i32, data: *const c_void, partials: *mut c_void, b0: u64, b1: u64);
    fn cpp_sum_tree(f: i32, partials: *mut c_void, n: u64);
    fn cpp_sum_naive(f: i32, data: *const c_void, n: u64) -> u64;
    fn cpp_learn_step(f: i32, w: *mut c_void, feats: *const c_void, targets: *const c_void, l0: u64, l1: u64, step: u64);
    fn cpp_rng_sum(g: i32, st: *const Stream, b0: u64, b1: u64, moment: u64) -> u64;
    fn cpp_rng_one(g: i32, st: *const Stream, being: u64, moment: u64) -> u64;

    fn cppnc_heat_step_f32(s: *const f32, d: *mut f32, r0: i32, r1: i32, a: f32);
    fn cppnc_heat_step_f64(s: *const f64, d: *mut f64, r0: i32, r1: i32, a: f64);
    fn cppnc_heat_step_fx32(s: *const i32, d: *mut i32, r0: i32, r1: i32, a: i32);
    fn cppnc_heat_step_fx64(s: *const i64, d: *mut i64, r0: i32, r1: i32, a: i64);
    fn cppnc_walk_step(f: i32, g: i32, st: *const Stream, grid: *const c_void, px: *mut u32, py: *mut u32,
                       tally: *mut c_void, a0: u64, a1: u64, moment: u64);
    fn cppnc_sum_blocks(f: i32, data: *const c_void, partials: *mut c_void, b0: u64, b1: u64);
    fn cppnc_sum_tree(f: i32, partials: *mut c_void, n: u64);
    fn cppnc_sum_naive(f: i32, data: *const c_void, n: u64) -> u64;
    fn cppnc_learn_step(f: i32, w: *mut c_void, feats: *const c_void, targets: *const c_void, l0: u64, l1: u64, step: u64);
    fn cppnc_rng_sum(g: i32, st: *const Stream, b0: u64, b1: u64, moment: u64) -> u64;
    fn cppnc_rng_one(g: i32, st: *const Stream, being: u64, moment: u64) -> u64;
}

pub static CPP: CppApi = CppApi {
    heat_f32: cpp_heat_step_f32,
    heat_f64: cpp_heat_step_f64,
    heat_fx32: cpp_heat_step_fx32,
    heat_fx64: cpp_heat_step_fx64,
    walk_step: cpp_walk_step,
    sum_blocks: cpp_sum_blocks,
    sum_tree: cpp_sum_tree,
    sum_naive: cpp_sum_naive,
    learn_step: cpp_learn_step,
    rng_sum: cpp_rng_sum,
    rng_one: cpp_rng_one,
};

pub static CPPNC: CppApi = CppApi {
    heat_f32: cppnc_heat_step_f32,
    heat_f64: cppnc_heat_step_f64,
    heat_fx32: cppnc_heat_step_fx32,
    heat_fx64: cppnc_heat_step_fx64,
    walk_step: cppnc_walk_step,
    sum_blocks: cppnc_sum_blocks,
    sum_tree: cppnc_sum_tree,
    sum_naive: cppnc_sum_naive,
    learn_step: cppnc_learn_step,
    rng_sum: cppnc_rng_sum,
    rng_one: cppnc_rng_one,
};
