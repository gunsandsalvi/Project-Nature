//! B01 kernels: heat, walk, sum, learn, rng. Each kernel runs in "reps": a rep starts
//! from the same initial state and does a fixed amount of work, so every rep must end
//! with the same checksum, whatever the thread count (X11). Threads get a fixed,
//! contiguous share of the work; steps are separated by a barrier.
use crate::cpp::CppApi;
use crate::rng::{mix64, Gen, KeyedGen, Stream};
use crate::with_gen;
use serde_json::{json, Value};
use std::cell::UnsafeCell;
use std::ffi::c_void;
use std::ops::Range;

// ---------------------------------------------------------------- languages and formats

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Lang {
    Rust,
    Cpp,
    CppNc,
}
impl Lang {
    pub fn name(self) -> &'static str {
        match self {
            Lang::Rust => "rust",
            Lang::Cpp => "cpp",
            Lang::CppNc => "cppnc",
        }
    }
    pub fn from_name(s: &str) -> Option<Lang> {
        [Lang::Rust, Lang::Cpp, Lang::CppNc].into_iter().find(|l| l.name() == s)
    }
    pub fn api(self) -> Option<&'static CppApi> {
        match self {
            Lang::Rust => None,
            Lang::Cpp => Some(&crate::cpp::CPP),
            Lang::CppNc => Some(&crate::cpp::CPPNC),
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(i32)]
pub enum Format {
    F32 = 0,
    F64 = 1,
    Fx32 = 2,
    Fx64 = 3,
}
impl Format {
    pub fn name(self) -> &'static str {
        match self {
            Format::F32 => "f32",
            Format::F64 => "f64",
            Format::Fx32 => "fx32",
            Format::Fx64 => "fx64",
        }
    }
    pub fn from_name(s: &str) -> Option<Format> {
        [Format::F32, Format::F64, Format::Fx32, Format::Fx64].into_iter().find(|f| f.name() == s)
    }
}

/// A number format. fx32 = Q16.16 in i32 (i64 products), fx64 = Q32.32 in i64 (i128 products).
pub trait Num: Copy + Send + Sync + Default + 'static {
    const FMT: Format;
    /// u in [0, 2^24) maps to the value u * 100 / 2^24.
    fn from_unit(u: u64) -> Self;
    /// Signed 24-bit m in [-2^23, 2^23) maps to m * 2^(e - 23).
    fn from_mant_exp(m: i64, e: i32) -> Self;
    fn add(self, o: Self) -> Self;
    fn bits(self) -> u64;
    fn to_f64(self) -> f64;
}

fn pow2_f64(e: i32) -> f64 {
    f64::from_bits(((e + 1023) as u64) << 52)
}

impl Num for f32 {
    const FMT: Format = Format::F32;
    #[inline(always)]
    fn from_unit(u: u64) -> Self {
        (u as f32) * (100.0f32 / 16_777_216.0f32)
    }
    fn from_mant_exp(m: i64, e: i32) -> Self {
        ((m as f64) * pow2_f64(e - 23)) as f32 // exact: 24 significant bits
    }
    #[inline(always)]
    fn add(self, o: Self) -> Self {
        self + o
    }
    #[inline(always)]
    fn bits(self) -> u64 {
        self.to_bits() as u64
    }
    fn to_f64(self) -> f64 {
        self as f64
    }
}
impl Num for f64 {
    const FMT: Format = Format::F64;
    #[inline(always)]
    fn from_unit(u: u64) -> Self {
        (u as f64) * (100.0f64 / 16_777_216.0f64)
    }
    fn from_mant_exp(m: i64, e: i32) -> Self {
        (m as f64) * pow2_f64(e - 23)
    }
    #[inline(always)]
    fn add(self, o: Self) -> Self {
        self + o
    }
    #[inline(always)]
    fn bits(self) -> u64 {
        self.to_bits()
    }
    fn to_f64(self) -> f64 {
        self
    }
}
impl Num for i32 {
    const FMT: Format = Format::Fx32;
    #[inline(always)]
    fn from_unit(u: u64) -> Self {
        ((u * 100) >> 8) as i32
    }
    fn from_mant_exp(m: i64, e: i32) -> Self {
        // value * 2^16 = m * 2^(e - 7); arithmetic shift (floor) when e < 7.
        (if e >= 7 { m << (e - 7) } else { m >> (7 - e) }) as i32
    }
    #[inline(always)]
    fn add(self, o: Self) -> Self {
        self.wrapping_add(o)
    }
    #[inline(always)]
    fn bits(self) -> u64 {
        self as u32 as u64
    }
    fn to_f64(self) -> f64 {
        self as f64 / 65_536.0
    }
}
impl Num for i64 {
    const FMT: Format = Format::Fx64;
    #[inline(always)]
    fn from_unit(u: u64) -> Self {
        ((u * 100) << 8) as i64
    }
    fn from_mant_exp(m: i64, e: i32) -> Self {
        // value * 2^32 = m * 2^(e + 9); arithmetic shift (floor) when e + 9 < 0.
        if e + 9 >= 0 {
            m << (e + 9)
        } else {
            m >> -(e + 9)
        }
    }
    #[inline(always)]
    fn add(self, o: Self) -> Self {
        self.wrapping_add(o)
    }
    #[inline(always)]
    fn bits(self) -> u64 {
        self as u64
    }
    fn to_f64(self) -> f64 {
        self as f64 / 4_294_967_296.0
    }
}

// ---------------------------------------------------------------- checksums and shared buffers

pub const FNV_OFF: u64 = 0xcbf2_9ce4_8422_2325;
pub const FNV_PRIME: u64 = 0x0000_0100_0000_01b3;
#[inline(always)]
pub fn fnv(h: u64, v: u64) -> u64 {
    (h ^ v).wrapping_mul(FNV_PRIME)
}
/// Checksum of a state: FNV-1a over each value's bits (as a 64-bit word), then mix64.
pub fn checksum_of<T: Num>(xs: &[T]) -> u64 {
    mix64(xs.iter().fold(FNV_OFF, |h, x| fnv(h, x.bits())))
}

/// A buffer that threads write in disjoint parts (the fixed partition), between barriers.
pub struct Shared<T> {
    v: UnsafeCell<Vec<T>>,
}
unsafe impl<T: Send> Sync for Shared<T> {}
impl<T> Shared<T> {
    pub fn new(v: Vec<T>) -> Self {
        Shared { v: UnsafeCell::new(v) }
    }
    pub fn ptr(&self) -> *mut T {
        unsafe { (*self.v.get()).as_mut_ptr() }
    }
    pub fn len(&self) -> usize {
        unsafe { (*self.v.get()).len() }
    }
    /// Safety: no thread may be writing any part of the buffer.
    pub unsafe fn all(&self) -> &[T] {
        std::slice::from_raw_parts(self.ptr(), self.len())
    }
    /// Safety: the range must belong to the calling thread only.
    #[allow(clippy::mut_from_ref)]
    pub unsafe fn part_mut(&self, r: Range<usize>) -> &mut [T] {
        assert!(r.end <= self.len());
        std::slice::from_raw_parts_mut(self.ptr().add(r.start), r.len())
    }
}

#[derive(Clone, Copy, Debug)]
pub struct Part {
    pub t: usize,
    pub n: usize,
}
impl Part {
    /// Thread t's fixed share of `len` items: [len*t/n, len*(t+1)/n).
    pub fn range(&self, len: usize) -> Range<usize> {
        (len * self.t / self.n)..(len * (self.t + 1) / self.n)
    }
}

pub trait Kernel: Sync {
    fn ops_per_rep(&self) -> u64;
    fn steps(&self) -> usize;
    fn reset(&self, p: Part);
    fn step(&self, s: usize, p: Part);
    fn checksum(&self) -> u64;
    fn extra(&self) -> Value {
        Value::Null
    }
}

// ---------------------------------------------------------------- heat (B01: fields)

pub const HN: usize = 512;
const HEAT_SALT: u64 = 0x4845_4154_0000_0001;

pub trait HeatNum: Num {
    fn alpha() -> Self;
    /// c + alpha * (((n + s) + (e + w)) - 4c): the same grouping in every language.
    fn update(c: Self, n: Self, s: Self, e: Self, w: Self, a: Self) -> Self;
    /// Safety: pointers to full 512x512 grids; rows r0..r1 of dst belong to the caller.
    unsafe fn cpp_step(api: &CppApi, src: *const Self, dst: *mut Self, r0: i32, r1: i32, a: Self);
}
impl HeatNum for f32 {
    fn alpha() -> Self {
        0.2
    }
    #[inline(always)]
    fn update(c: f32, n: f32, s: f32, e: f32, w: f32, a: f32) -> f32 {
        c + a * (((n + s) + (e + w)) - 4.0 * c)
    }
    unsafe fn cpp_step(api: &CppApi, src: *const f32, dst: *mut f32, r0: i32, r1: i32, a: f32) {
        (api.heat_f32)(src, dst, r0, r1, a)
    }
}
impl HeatNum for f64 {
    fn alpha() -> Self {
        0.2
    }
    #[inline(always)]
    fn update(c: f64, n: f64, s: f64, e: f64, w: f64, a: f64) -> f64 {
        c + a * (((n + s) + (e + w)) - 4.0 * c)
    }
    unsafe fn cpp_step(api: &CppApi, src: *const f64, dst: *mut f64, r0: i32, r1: i32, a: f64) {
        (api.heat_f64)(src, dst, r0, r1, a)
    }
}
impl HeatNum for i32 {
    fn alpha() -> Self {
        13_107 // round(0.2 * 2^16)
    }
    #[inline(always)]
    fn update(c: i32, n: i32, s: i32, e: i32, w: i32, a: i32) -> i32 {
        let lap = n.wrapping_add(s).wrapping_add(e.wrapping_add(w)).wrapping_sub(c.wrapping_mul(4));
        c.wrapping_add(((a as i64 * lap as i64) >> 16) as i32)
    }
    unsafe fn cpp_step(api: &CppApi, src: *const i32, dst: *mut i32, r0: i32, r1: i32, a: i32) {
        (api.heat_fx32)(src, dst, r0, r1, a)
    }
}
impl HeatNum for i64 {
    fn alpha() -> Self {
        858_993_459 // round(0.2 * 2^32)
    }
    #[inline(always)]
    fn update(c: i64, n: i64, s: i64, e: i64, w: i64, a: i64) -> i64 {
        let lap = n.wrapping_add(s).wrapping_add(e.wrapping_add(w)).wrapping_sub(c.wrapping_mul(4));
        c.wrapping_add(((a as i128 * lap as i128) >> 32) as i64)
    }
    unsafe fn cpp_step(api: &CppApi, src: *const i64, dst: *mut i64, r0: i32, r1: i32, a: i64) {
        (api.heat_fx64)(src, dst, r0, r1, a)
    }
}

/// Rows r0..r1 of one explicit step on the torus; `out` holds exactly those rows.
fn heat_rows<T: HeatNum>(src: &[T], out: &mut [T], r0: usize, r1: usize, a: T) {
    const N: usize = HN;
    for y in r0..r1 {
        let ym = (y + N - 1) % N;
        let yp = (y + 1) % N;
        let row = &src[y * N..y * N + N];
        let up = &src[ym * N..ym * N + N];
        let dn = &src[yp * N..yp * N + N];
        let o = &mut out[(y - r0) * N..(y - r0) * N + N];
        o[0] = T::update(row[0], up[0], dn[0], row[1], row[N - 1], a);
        o[N - 1] = T::update(row[N - 1], up[N - 1], dn[N - 1], row[0], row[N - 2], a);
        // Interior: zipped slices (west, centre, east, up, down) vectorize without shuffles.
        let it = o[1..N - 1]
            .iter_mut()
            .zip(&row[1..N - 1])
            .zip(&up[1..N - 1])
            .zip(&dn[1..N - 1])
            .zip(row[2..].iter().zip(&row[..N - 2]));
        for ((((o, &c), &u), &d), (&e, &w)) in it {
            *o = T::update(c, u, d, e, w, a);
        }
    }
}

pub fn heat_initial<T: Num>() -> Vec<T> {
    (0..HN * HN).map(|i| T::from_unit(mix64(i as u64 ^ HEAT_SALT) >> 40)).collect()
}

pub struct Heat<T: HeatNum> {
    lang: Lang,
    init: Vec<T>,
    bufs: [Shared<T>; 2],
    steps: usize,
}
impl<T: HeatNum> Heat<T> {
    pub fn new(lang: Lang, steps: usize) -> Self {
        let init = heat_initial::<T>();
        let bufs = [Shared::new(init.clone()), Shared::new(init.clone())];
        Heat { lang, init, bufs, steps }
    }
}
impl<T: HeatNum> Kernel for Heat<T> {
    fn ops_per_rep(&self) -> u64 {
        (HN * HN * self.steps) as u64
    }
    fn steps(&self) -> usize {
        self.steps
    }
    fn reset(&self, p: Part) {
        let r = p.range(HN);
        let cells = r.start * HN..r.end * HN;
        unsafe { self.bufs[0].part_mut(cells.clone()) }.copy_from_slice(&self.init[cells]);
    }
    fn step(&self, s: usize, p: Part) {
        let (src, dst) = (&self.bufs[s % 2], &self.bufs[(s + 1) % 2]);
        let r = p.range(HN);
        unsafe {
            match self.lang.api() {
                None => heat_rows(src.all(), dst.part_mut(r.start * HN..r.end * HN), r.start, r.end, T::alpha()),
                Some(api) => T::cpp_step(api, src.ptr(), dst.ptr(), r.start as i32, r.end as i32, T::alpha()),
            }
        }
    }
    fn checksum(&self) -> u64 {
        checksum_of(unsafe { self.bufs[self.steps % 2].all() })
    }
    fn extra(&self) -> Value {
        let total = |v: &[T]| v.iter().map(|x| x.to_f64()).sum::<f64>();
        let (t0, t1) = (total(&self.init), total(unsafe { self.bufs[self.steps % 2].all() }));
        json!({"total_initial": t0, "total_final": t1, "relative_drift_per_step": (t1 - t0) / t0 / self.steps as f64})
    }
}

// ---------------------------------------------------------------- walk (B01 agents, B02 draws)

pub const WN: usize = 1024;
pub const AGENTS: usize = 100_000;
const WALK_GRID_SALT: u64 = 0x5741_4c4b_0000_0001;
const WALK_POS_SALT: u64 = 0x5741_4c4b_0000_0002;
const DX: [u32; 4] = [1, WN as u32 - 1, 0, 0];
const DY: [u32; 4] = [0, 0, 1, WN as u32 - 1];

fn walk_agents<T: Num, G: KeyedGen>(
    g: &G, grid: &[T; WN * WN], px: &mut [u32], py: &mut [u32], tally: &mut [T], a0: u64, moment: u64,
) {
    for (i, ((x, y), t)) in px.iter_mut().zip(py.iter_mut()).zip(tally.iter_mut()).enumerate() {
        let d = g.draw(a0 + i as u64, moment);
        let dir = (d >> 62) as usize; // top bits: never rely on low bits
        let nx = x.wrapping_add(DX[dir]) & (WN as u32 - 1);
        let ny = y.wrapping_add(DY[dir]) & (WN as u32 - 1);
        *x = nx;
        *y = ny;
        *t = t.add(grid[((ny as usize) << 10) | nx as usize]);
    }
}

pub struct Walk<T: Num> {
    lang: Lang,
    gen: Gen,
    stream: Stream,
    grid: Vec<T>,
    init_x: Vec<u32>,
    init_y: Vec<u32>,
    px: Shared<u32>,
    py: Shared<u32>,
    tally: Shared<T>,
    steps: usize,
}
impl<T: Num> Walk<T> {
    pub fn new(lang: Lang, gen: Gen, steps: usize) -> Self {
        let grid: Vec<T> = (0..WN * WN).map(|i| T::from_unit(mix64(i as u64 ^ WALK_GRID_SALT) >> 40)).collect();
        let pos: Vec<u64> = (0..AGENTS).map(|a| mix64(a as u64 ^ WALK_POS_SALT)).collect();
        let init_x: Vec<u32> = pos.iter().map(|p| (*p as u32) & (WN as u32 - 1)).collect();
        let init_y: Vec<u32> = pos.iter().map(|p| ((p >> 32) as u32) & (WN as u32 - 1)).collect();
        Walk {
            lang,
            gen,
            stream: Stream::new(1, 7, 1), // world 1, system 7 ("movement"), purpose 1 ("step direction")
            grid,
            px: Shared::new(init_x.clone()),
            py: Shared::new(init_y.clone()),
            tally: Shared::new(vec![T::default(); AGENTS]),
            init_x,
            init_y,
            steps,
        }
    }
}
impl<T: Num> Kernel for Walk<T> {
    fn ops_per_rep(&self) -> u64 {
        (AGENTS * self.steps) as u64
    }
    fn steps(&self) -> usize {
        self.steps
    }
    fn reset(&self, p: Part) {
        let r = p.range(AGENTS);
        unsafe {
            self.px.part_mut(r.clone()).copy_from_slice(&self.init_x[r.clone()]);
            self.py.part_mut(r.clone()).copy_from_slice(&self.init_y[r.clone()]);
            self.tally.part_mut(r).fill(T::default());
        }
    }
    fn step(&self, s: usize, p: Part) {
        let r = p.range(AGENTS);
        let moment = s as u64;
        unsafe {
            match self.lang.api() {
                None => {
                    let grid: &[T; WN * WN] = self.grid.as_slice().try_into().unwrap();
                    let (px, py, t) =
                        (self.px.part_mut(r.clone()), self.py.part_mut(r.clone()), self.tally.part_mut(r.clone()));
                    with_gen!(self.gen, &self.stream, |g| walk_agents(&g, grid, px, py, t, r.start as u64, moment))
                }
                Some(api) => (api.walk_step)(
                    T::FMT as i32,
                    self.gen as i32,
                    &self.stream,
                    self.grid.as_ptr() as *const c_void,
                    self.px.ptr(),
                    self.py.ptr(),
                    self.tally.ptr() as *mut c_void,
                    r.start as u64,
                    r.end as u64,
                    moment,
                ),
            }
        }
    }
    fn checksum(&self) -> u64 {
        let (px, py, t) = unsafe { (self.px.all(), self.py.all(), self.tally.all()) };
        let mut h = FNV_OFF;
        for i in 0..AGENTS {
            h = fnv(h, px[i] as u64 | (py[i] as u64) << 32);
            h = fnv(h, t[i].bits());
        }
        mix64(h)
    }
}

// ---------------------------------------------------------------- sum (B01: big totals)

pub const SUM_N: usize = 1 << 24;
pub const SUM_B: usize = 4096; // block size of the fixed tree
pub const SUM_BLOCKS: usize = SUM_N / SUM_B;
const SUM_SALT: u64 = 0x5355_4d00_0000_0001;

pub trait SumNum: Num {
    /// The accumulator: the format itself for floats, i64 for fixed point (wrapping).
    type Acc: Num;
    fn to_acc(self) -> Self::Acc;
}
impl SumNum for f32 {
    type Acc = f32;
    #[inline(always)]
    fn to_acc(self) -> f32 {
        self
    }
}
impl SumNum for f64 {
    type Acc = f64;
    #[inline(always)]
    fn to_acc(self) -> f64 {
        self
    }
}
impl SumNum for i32 {
    type Acc = i64;
    #[inline(always)]
    fn to_acc(self) -> i64 {
        self as i64
    }
}
impl SumNum for i64 {
    type Acc = i64;
    #[inline(always)]
    fn to_acc(self) -> i64 {
        self
    }
}

/// Fixed balanced tree, in place: at each level item i takes item i + half.
/// The shape depends only on the length (a power of two), never on threads.
pub fn tree_in_place<A: Num>(t: &mut [A]) -> A {
    let mut h = t.len() / 2;
    while h >= 1 {
        let (a, b) = t.split_at_mut(h);
        for i in 0..h {
            a[i] = a[i].add(b[i]);
        }
        h /= 2;
    }
    t[0]
}

fn block_sum<T: SumNum>(x: &[T]) -> T::Acc {
    let mut tmp = [T::Acc::default(); SUM_B / 2];
    let (lo, hi) = x.split_at(SUM_B / 2);
    for i in 0..SUM_B / 2 {
        tmp[i] = lo[i].to_acc().add(hi[i].to_acc());
    }
    tree_in_place(&mut tmp)
}

pub fn sum_data<T: Num>() -> Vec<T> {
    (0..SUM_N)
        .map(|i| {
            let u = mix64(i as u64 ^ SUM_SALT);
            // 24-bit mantissa, exponent -16..15: magnitudes from 2^-16 to 2^15, so that
            // even f64 has to round (values fit Q16.16 and Q32.32).
            let m = ((u >> 40) as i64) - (1 << 23);
            let e = ((u >> 4) & 31) as i32 - 16;
            T::from_mant_exp(m, e)
        })
        .collect()
}

pub struct Sum<T: SumNum> {
    lang: Lang,
    data: Vec<T>,
    partials: Shared<T::Acc>,
}
impl<T: SumNum> Sum<T> {
    pub fn new(lang: Lang) -> Self {
        Sum { lang, data: sum_data::<T>(), partials: Shared::new(vec![T::Acc::default(); SUM_BLOCKS]) }
    }
    /// Plain left-to-right loop on one thread, for comparison.
    pub fn naive(&self) -> T::Acc {
        match self.lang.api() {
            None => self.data.iter().fold(T::Acc::default(), |s, x| s.add(x.to_acc())),
            Some(api) => {
                let bits = unsafe { (api.sum_naive)(T::FMT as i32, self.data.as_ptr() as *const c_void, SUM_N as u64) };
                acc_from_bits::<T::Acc>(bits)
            }
        }
    }
}
fn acc_from_bits<A: Num>(bits: u64) -> A {
    // Only used to compare bits: rebuild the value through its bit pattern.
    let mut out = A::default();
    unsafe {
        let sz = std::mem::size_of::<A>();
        std::ptr::copy_nonoverlapping(&bits as *const u64 as *const u8, &mut out as *mut A as *mut u8, sz);
    }
    out
}
impl<T: SumNum> Kernel for Sum<T> {
    fn ops_per_rep(&self) -> u64 {
        SUM_N as u64
    }
    fn steps(&self) -> usize {
        2
    }
    fn reset(&self, _p: Part) {}
    fn step(&self, s: usize, p: Part) {
        unsafe {
            if s == 0 {
                let r = p.range(SUM_BLOCKS);
                match self.lang.api() {
                    None => {
                        let out = self.partials.part_mut(r.clone());
                        for (k, b) in r.enumerate() {
                            out[k] = block_sum(&self.data[b * SUM_B..(b + 1) * SUM_B]);
                        }
                    }
                    Some(api) => (api.sum_blocks)(
                        T::FMT as i32,
                        self.data.as_ptr() as *const c_void,
                        self.partials.ptr() as *mut c_void,
                        r.start as u64,
                        r.end as u64,
                    ),
                }
            } else if p.t == 0 {
                match self.lang.api() {
                    None => {
                        tree_in_place(self.partials.part_mut(0..SUM_BLOCKS));
                    }
                    Some(api) => (api.sum_tree)(T::FMT as i32, self.partials.ptr() as *mut c_void, SUM_BLOCKS as u64),
                }
            }
        }
    }
    fn checksum(&self) -> u64 {
        mix64(fnv(FNV_OFF, unsafe { self.partials.all() }[0].bits()))
    }
    fn extra(&self) -> Value {
        let tree = unsafe { self.partials.all() }[0];
        let mut times = vec![];
        let mut naive = T::Acc::default();
        for _ in 0..3 {
            let t = std::time::Instant::now();
            naive = std::hint::black_box(self.naive());
            times.push(t.elapsed().as_secs_f64());
        }
        times.sort_by(|a, b| a.partial_cmp(b).unwrap());
        // fx32 sums are Q16.16 held in an i64, not Q32.32.
        let val = |a: T::Acc| if T::FMT == Format::Fx32 { a.bits() as i64 as f64 / 65_536.0 } else { a.to_f64() };
        json!({
            "tree_value": val(tree),
            "naive_value": val(naive),
            "naive_checksum": format!("{:016x}", mix64(fnv(FNV_OFF, naive.bits()))),
            "naive_differs": naive.bits() != tree.bits(),
            "naive_ops_per_sec_1t": SUM_N as f64 / times[1],
        })
    }
}

// ---------------------------------------------------------------- learn (B01: learning updates)

pub const LEARNERS: usize = 4096;
pub const WEIGHTS: usize = 64;
pub const FEATS: usize = 256;
const LEARN_X_SALT: u64 = 0x4c45_4152_0000_0001;
const LEARN_T_SALT: u64 = 0x4c45_4152_0000_0002;
const LEARN_W_SALT: u64 = 0x4c45_4152_0000_0003;

pub trait LearnNum: Num {
    /// m in [-2^23, 2^23) scaled by 2^-shift.
    fn scaled(m: i64, shift: i32) -> Self;
    fn target(t: f64) -> Self;
    /// One delta-rule update: y = w.x, err = target - y, w += lr * err * x (lr = 1/64).
    fn update(w: &mut [Self], x: &[Self], target: Self);
}
macro_rules! float_learn {
    ($t:ty) => {
        impl LearnNum for $t {
            fn scaled(m: i64, shift: i32) -> Self {
                ((m as f64) * pow2_f64(-shift)) as $t
            }
            fn target(t: f64) -> Self {
                t as $t
            }
            #[inline(always)]
            fn update(w: &mut [Self], x: &[Self], target: Self) {
                let (w, x): (&mut [Self; WEIGHTS], &[Self; WEIGHTS]) = (w.try_into().unwrap(), x.try_into().unwrap());
                // Dot product in 8 fixed lanes, then a fixed tree: same order in every language.
                let mut acc = [0.0 as $t; 8];
                for c in 0..WEIGHTS / 8 {
                    for k in 0..8 {
                        acc[k] = acc[k] + w[c * 8 + k] * x[c * 8 + k];
                    }
                }
                let y = ((acc[0] + acc[1]) + (acc[2] + acc[3])) + ((acc[4] + acc[5]) + (acc[6] + acc[7]));
                let g = (1.0 / 64.0) * (target - y);
                for j in 0..WEIGHTS {
                    w[j] = w[j] + g * x[j];
                }
            }
        }
    };
}
float_learn!(f32);
float_learn!(f64);
impl LearnNum for i32 {
    fn scaled(m: i64, shift: i32) -> Self {
        // value * 2^16 = m * 2^(16 - shift)
        (if shift <= 16 { m << (16 - shift) } else { m >> (shift - 16) }) as i32
    }
    fn target(t: f64) -> Self {
        (t * 65_536.0).floor() as i32
    }
    #[inline(always)]
    fn update(w: &mut [i32], x: &[i32], target: i32) {
        let (w, x): (&mut [i32; WEIGHTS], &[i32; WEIGHTS]) = (w.try_into().unwrap(), x.try_into().unwrap());
        let mut acc: i64 = 0;
        for j in 0..WEIGHTS {
            acc = acc.wrapping_add(w[j] as i64 * x[j] as i64);
        }
        let y = (acc >> 16) as i32;
        let err = target.wrapping_sub(y);
        let g = ((1024i64 * err as i64) >> 16) as i32; // lr = 1/64 = 1024 in Q16.16
        for j in 0..WEIGHTS {
            w[j] = w[j].wrapping_add(((g as i64 * x[j] as i64) >> 16) as i32);
        }
    }
}

pub struct Learn<T: LearnNum> {
    lang: Lang,
    feats: Vec<T>,
    targets: Vec<T>,
    init_w: Vec<T>,
    w: Shared<T>,
    steps: usize,
}
impl<T: LearnNum> Learn<T> {
    pub fn new(lang: Lang, steps: usize) -> Self {
        let m = |i: u64, salt: u64| ((mix64(i ^ salt) >> 40) as i64) - (1 << 23);
        let feats: Vec<T> = (0..FEATS * WEIGHTS).map(|i| T::scaled(m(i as u64, LEARN_X_SALT), 23)).collect();
        let w_true: Vec<f64> = (0..WEIGHTS).map(|j| m(j as u64, LEARN_T_SALT) as f64 * pow2_f64(-23)).collect();
        let targets: Vec<T> = (0..FEATS)
            .map(|f| {
                let t: f64 = (0..WEIGHTS)
                    .map(|j| w_true[j] * (m((f * WEIGHTS + j) as u64, LEARN_X_SALT) as f64 * pow2_f64(-23)))
                    .sum();
                T::target(t)
            })
            .collect();
        let init_w: Vec<T> = (0..LEARNERS * WEIGHTS).map(|i| T::scaled(m(i as u64, LEARN_W_SALT), 25)).collect();
        Learn { lang, feats, targets, w: Shared::new(init_w.clone()), init_w, steps }
    }
}
impl<T: LearnNum> Kernel for Learn<T> {
    fn ops_per_rep(&self) -> u64 {
        (LEARNERS * self.steps) as u64
    }
    fn steps(&self) -> usize {
        self.steps
    }
    fn reset(&self, p: Part) {
        let r = p.range(LEARNERS);
        let ws = r.start * WEIGHTS..r.end * WEIGHTS;
        unsafe { self.w.part_mut(ws.clone()) }.copy_from_slice(&self.init_w[ws]);
    }
    fn step(&self, s: usize, p: Part) {
        let r = p.range(LEARNERS);
        unsafe {
            match self.lang.api() {
                None => {
                    let w = self.w.part_mut(r.start * WEIGHTS..r.end * WEIGHTS);
                    for (k, l) in r.enumerate() {
                        let f = (l * 7 + s * 13) & (FEATS - 1);
                        T::update(
                            &mut w[k * WEIGHTS..(k + 1) * WEIGHTS],
                            &self.feats[f * WEIGHTS..(f + 1) * WEIGHTS],
                            self.targets[f],
                        );
                    }
                }
                Some(api) => (api.learn_step)(
                    T::FMT as i32,
                    self.w.ptr() as *mut c_void,
                    self.feats.as_ptr() as *const c_void,
                    self.targets.as_ptr() as *const c_void,
                    r.start as u64,
                    r.end as u64,
                    s as u64,
                ),
            }
        }
    }
    fn checksum(&self) -> u64 {
        checksum_of(unsafe { self.w.all() })
    }
}

// ---------------------------------------------------------------- rng (B02: one keyed draw)

pub const RNG_BEINGS: usize = 65_536;
const SLOT: usize = 8; // one cache line per thread's partial sum

pub struct Rng {
    lang: Lang,
    gen: Gen,
    stream: Stream,
    slots: Shared<u64>,
    steps: usize,
}
impl Rng {
    pub fn new(lang: Lang, gen: Gen, steps: usize) -> Self {
        Rng { lang, gen, stream: Stream::new(1, 3, 9), slots: Shared::new(vec![0; 64 * SLOT]), steps }
    }
}
impl Kernel for Rng {
    fn ops_per_rep(&self) -> u64 {
        (RNG_BEINGS * self.steps) as u64
    }
    fn steps(&self) -> usize {
        self.steps
    }
    fn reset(&self, p: Part) {
        unsafe { self.slots.part_mut(p.t * SLOT..p.t * SLOT + 1)[0] = 0 };
    }
    fn step(&self, s: usize, p: Part) {
        let r = p.range(RNG_BEINGS);
        let m = s as u64;
        let sum = match self.lang.api() {
            None => with_gen!(self.gen, &self.stream, |g| {
                let mut acc = 0u64;
                for b in r.start as u64..r.end as u64 {
                    acc = acc.wrapping_add(g.draw(b, m));
                }
                acc
            }),
            Some(api) => unsafe { (api.rng_sum)(self.gen as i32, &self.stream, r.start as u64, r.end as u64, m) },
        };
        let slot = unsafe { &mut self.slots.part_mut(p.t * SLOT..p.t * SLOT + 1)[0] };
        *slot = slot.wrapping_add(sum);
    }
    fn checksum(&self) -> u64 {
        // A wrapping sum does not depend on how the beings were split between threads.
        let total = unsafe { self.slots.all() }.iter().fold(0u64, |a, b| a.wrapping_add(*b));
        mix64(fnv(FNV_OFF, total))
    }
}
