//! B10 isotropy: how much longer grid paths are than the true distance (feeds B12).
//! Squares (8 neighbours, and 4 as a reference) against hexes (6 neighbours), with equal cell
//! areas, each with and without a simple any-angle smoothing.

use crate::checks::{HEX_M, HEX_N, SQ_WORLD};
use crate::hex::{self, HexTorus, DIRS, E, SQRT3};
use crate::path::{self, Search, P};
use crate::rng::Rng;
use crate::sq::{self, DIRS8, SQRT2};
use std::f64::consts::TAU;

/// Hex spacing whose cell area equals a unit square: (sqrt3/2) s^2 = 1.
pub fn hex_s() -> f64 {
    (2.0 / SQRT3).sqrt()
}

pub struct Stats {
    pub mean: f64,
    pub median: f64,
    pub p95: f64,
    pub max: f64,
    pub n: usize,
}

pub fn stats(v: &mut [f64]) -> Stats {
    v.sort_by(|a, b| a.partial_cmp(b).unwrap());
    let n = v.len();
    if n == 0 {
        return Stats { mean: f64::NAN, median: f64::NAN, p95: f64::NAN, max: f64::NAN, n };
    }
    Stats {
        mean: v.iter().sum::<f64>() / n as f64,
        median: v[n / 2],
        p95: v[((n as f64 * 0.95) as usize).min(n - 1)],
        max: v[n - 1],
        n,
    }
}

fn line(label: &str, v: &mut [f64]) {
    let s = stats(v);
    println!(
        "  {label:<34} mean {:>6.2}%  median {:>6.2}%  p95 {:>6.2}%  max {:>6.2}%  (n={})",
        100.0 * s.mean,
        100.0 * s.median,
        100.0 * s.p95,
        100.0 * s.max,
        s.n
    );
}

pub fn run() {
    open_ground();
    torus_pairs();
    obstacles(false, 300);
    obstacles(true, 60);
}

/// Open ground, uniform directions, 20 to 200 cells apart. Endpoints snap to their cells.
fn open_ground() {
    println!("== B10 isotropy, open ground: raw grid path / true distance - 1 ==");
    println!("  (smoothing makes every open-ground path exactly straight, so smoothed error is 0)");
    let s = hex_s();
    let mut rng = Rng::new(21);
    let (mut e8, mut e4, mut e6) = (vec![], vec![], vec![]);
    for _ in 0..200_000 {
        let p = (rng.range(0.0, 1000.0), rng.range(0.0, 1000.0));
        let (d, th) = (rng.range(20.0, 200.0), rng.range(0.0, TAU));
        let q = (p.0 + d * th.cos(), p.1 + d * th.sin());
        let dx = (q.0.floor() - p.0.floor()).abs();
        let dy = (q.1.floor() - p.1.floor()).abs();
        e8.push(sq::octile(dx, dy) / d - 1.0);
        e4.push((dx + dy) / d - 1.0);
        let zp = hex::cell_at(p.0 / s, p.1 / s);
        let zq = hex::cell_at(q.0 / s, q.1 / s);
        e6.push(zq.sub(zp).steps() as f64 * s / d - 1.0);
    }
    line("squares, 8 neighbours", &mut e8);
    line("squares, 4 neighbours (reference)", &mut e4);
    line("hexes, 6 neighbours", &mut e6);
    println!("  exact by direction alone: squares-8 mean 5.48% max 8.24%; hexes-6 mean 10.27% max 15.47%; squares-4 mean 27.32% max 41.42%");
}

/// Random pairs anywhere on the 2000 x 1000 km world, distance taken across both seams.
fn torus_pairs() {
    println!("== B10 isotropy, random pairs on the whole torus (1 km cells, both seams) ==");
    let mut rng = Rng::new(22);
    let (w, h) = (2000.0, 1000.0);
    let c = w / SQ_WORLD.w as f64;
    let ht = HexTorus::fine(HEX_N, HEX_M);
    let s = w / HEX_N as f64;
    let hh = ht.d as f64 * s * SQRT3 / 2.0;
    let (mut e8, mut e6) = (vec![], vec![]);
    for _ in 0..200_000 {
        let p = (rng.range(0.0, w), rng.range(0.0, h));
        let q = (rng.range(0.0, w), rng.range(0.0, h));
        let t = (sq::wrap_absf(q.0 - p.0, w).powi(2) + sq::wrap_absf(q.1 - p.1, h).powi(2)).sqrt();
        if t < 20.0 {
            continue;
        }
        let i = SQ_WORLD.index((p.0 / c) as i64, (p.1 / c) as i64);
        let j = SQ_WORLD.index((q.0 / c) as i64, (q.1 / c) as i64);
        e8.push(SQ_WORLD.octile(i, j) * c / t - 1.0);
        let p = (rng.range(0.0, w), rng.range(0.0, hh));
        let q = (rng.range(0.0, w), rng.range(0.0, hh));
        let t = (sq::wrap_absf(q.0 - p.0, w).powi(2) + sq::wrap_absf(q.1 - p.1, hh).powi(2)).sqrt();
        if t < 20.0 {
            continue;
        }
        let i = ht.index(hex::cell_at(p.0 / s, p.1 / s));
        let j = ht.index(hex::cell_at(q.0 / s, q.1 / s));
        e6.push(ht.steps(i, j) as f64 * s / t - 1.0);
    }
    line("squares, 8 neighbours", &mut e8);
    line("hexes, 6 neighbours (rows run east-west)", &mut e6);
}

const WIN: f64 = 256.0;

/// Blobby obstacles (lakes, cliffs, thickets): a sum of waves, blocked above a threshold.
struct Field {
    waves: Vec<(f64, f64, f64)>,
    t: f64,
}

impl Field {
    fn new(rng: &mut Rng, open: bool) -> Field {
        let waves = (0..24)
            .map(|_| {
                let (lam, th) = (rng.range(10.0, 40.0), rng.range(0.0, TAU));
                let k = TAU / lam;
                (k * th.cos(), k * th.sin(), rng.range(0.0, TAU))
            })
            .collect();
        let mut f = Field { waves, t: f64::INFINITY };
        if !open {
            let mut v: Vec<f64> =
                (0..20_000).map(|_| f.val(rng.range(0.0, WIN), rng.range(0.0, WIN))).collect();
            v.sort_by(|a, b| a.partial_cmp(b).unwrap());
            f.t = v[15_000]; // 25% of the ground blocked
        }
        f
    }
    fn val(&self, x: f64, y: f64) -> f64 {
        self.waves.iter().map(|w| (w.0 * x + w.1 * y + w.2).cos()).sum()
    }
    fn free(&self, x: f64, y: f64) -> bool {
        x >= 0.0 && y >= 0.0 && x < WIN && y < WIN && self.val(x, y) <= self.t
    }
}

struct SqWin {
    c: f64,
    n: usize,
    blocked: Vec<bool>,
}

impl SqWin {
    fn new(c: f64, f: &Field) -> SqWin {
        let n = (WIN / c).round() as usize;
        let blocked = (0..n * n)
            .map(|i| !f.free(((i % n) as f64 + 0.5) * c, ((i / n) as f64 + 0.5) * c))
            .collect();
        SqWin { c, n, blocked }
    }
    fn centre(&self, i: usize) -> P {
        (((i % self.n) as f64 + 0.5) * self.c, ((i / self.n) as f64 + 0.5) * self.c)
    }
    fn at(&self, x: f64, y: f64) -> Option<usize> {
        if x < 0.0 || y < 0.0 {
            return None;
        }
        let (i, j) = ((x / self.c) as usize, (y / self.c) as usize);
        if i >= self.n || j >= self.n {
            None
        } else {
            Some(j * self.n + i)
        }
    }
    fn free_pt(&self, x: f64, y: f64) -> bool {
        self.at(x, y).map_or(false, |i| !self.blocked[i])
    }
    fn neigh(&self, i: usize, out: &mut Vec<(usize, f64)>, eight: bool) {
        let n = self.n as i64;
        let (x, y) = ((i % self.n) as i64, (i / self.n) as i64);
        for (dx, dy) in DIRS8 {
            let diag = dx != 0 && dy != 0;
            if diag && !eight {
                continue;
            }
            let (nx, ny) = (x + dx, y + dy);
            if nx < 0 || ny < 0 || nx >= n || ny >= n {
                continue;
            }
            let j = (ny * n + nx) as usize;
            if self.blocked[j] {
                continue;
            }
            // no corner cutting between two blocked cells
            if diag && (self.blocked[(y * n + nx) as usize] || self.blocked[(ny * n + x) as usize]) {
                continue;
            }
            out.push((j, if diag { SQRT2 * self.c } else { self.c }));
        }
    }
    fn h(&self, i: usize, t: usize, eight: bool) -> f64 {
        let dx = (i % self.n) as f64 - (t % self.n) as f64;
        let dy = (i / self.n) as f64 - (t / self.n) as f64;
        self.c * if eight { sq::octile(dx, dy) } else { dx.abs() + dy.abs() }
    }
}

struct HexWin {
    s: f64,
    cols: usize,
    rows: usize,
    blocked: Vec<bool>,
}

impl HexWin {
    fn new(s: f64, f: &Field) -> HexWin {
        let cols = (WIN / s).ceil() as usize + 1;
        let rows = (WIN / (s * SQRT3 / 2.0)).ceil() as usize + 1;
        let mut w = HexWin { s, cols, rows, blocked: vec![] };
        w.blocked = (0..cols * rows)
            .map(|i| {
                let (x, y) = w.centre(i);
                !f.free(x, y)
            })
            .collect();
        w
    }
    fn axial(&self, i: usize) -> E {
        let (col, row) = ((i % self.cols) as i64, (i / self.cols) as i64);
        E::new(col - (row >> 1), row)
    }
    fn idx(&self, z: E) -> Option<usize> {
        if z.r < 0 || z.r >= self.rows as i64 {
            return None;
        }
        let col = z.q + (z.r >> 1);
        if col < 0 || col >= self.cols as i64 {
            None
        } else {
            Some(z.r as usize * self.cols + col as usize)
        }
    }
    fn centre(&self, i: usize) -> P {
        let (x, y) = self.axial(i).xy();
        (x * self.s, y * self.s)
    }
    fn at(&self, x: f64, y: f64) -> Option<usize> {
        self.idx(hex::cell_at(x / self.s, y / self.s))
    }
    fn free_pt(&self, x: f64, y: f64) -> bool {
        self.at(x, y).map_or(false, |i| !self.blocked[i])
    }
    fn neigh(&self, i: usize, out: &mut Vec<(usize, f64)>) {
        let z = self.axial(i);
        for d in DIRS {
            if let Some(j) = self.idx(z.add(d)) {
                if !self.blocked[j] {
                    out.push((j, self.s));
                }
            }
        }
    }
    fn h(&self, i: usize, t: usize) -> f64 {
        self.axial(t).sub(self.axial(i)).steps() as f64 * self.s
    }
}

/// Raw path through cell centres (exact endpoints), then the smoothed one.
fn raw_and_smoothed(
    cells: &[usize],
    p: P,
    q: P,
    centre: &dyn Fn(usize) -> P,
    step: f64,
    free: &dyn Fn(f64, f64) -> bool,
) -> (f64, f64) {
    let mut pts = vec![p];
    if cells.len() > 2 {
        for &c in &cells[1..cells.len() - 1] {
            pts.push(centre(c));
        }
    }
    pts.push(q);
    let sm = path::smooth(&pts, step, free);
    (path::length(&pts), path::length(&sm))
}

/// Ground with obstacles: 256 x 256 cells, 25% blocked in blobs 5 to 20 cells across, pairs 20 to
/// 80 cells apart. "True" length: the same search plus smoothing on squares 8 times finer.
fn obstacles(open: bool, pairs: usize) {
    let title = if open { "no obstacles (sanity check)" } else { "25% of the ground blocked" };
    println!("== B10 isotropy, {title}: path / reference - 1 ==");
    let mut rng = Rng::new(if open { 31 } else { 32 });
    let f = Field::new(&mut rng, open);
    let s = hex_s();
    let sqw = SqWin::new(1.0, &f);
    let hxw = HexWin::new(s, &f);
    let fine = SqWin::new(1.0 / 8.0, &f);
    let mut s_sq = Search::new(sqw.n * sqw.n);
    let mut s_hx = Search::new(hxw.cols * hxw.rows);
    let mut s_fine = Search::new(fine.n * fine.n);
    let (mut r8, mut m8, mut r4, mut m4, mut r6, mut m6) = (vec![], vec![], vec![], vec![], vec![], vec![]);
    let (mut lost8, mut lost4, mut lost6) = (0, 0, 0);
    let mut done = 0;
    let mut tries = 0;
    while done < pairs && tries < pairs * 50 {
        tries += 1;
        let p = (rng.range(8.0, WIN - 8.0), rng.range(8.0, WIN - 8.0));
        let (d, th) = (rng.range(20.0, 80.0), rng.range(0.0, TAU));
        let q = (p.0 + d * th.cos(), p.1 + d * th.sin());
        if q.0 < 8.0 || q.1 < 8.0 || q.0 > WIN - 8.0 || q.1 > WIN - 8.0 {
            continue;
        }
        let ok = |x: f64, y: f64| f.free(x, y) && sqw.free_pt(x, y) && hxw.free_pt(x, y) && fine.free_pt(x, y);
        if !ok(p.0, p.1) || !ok(q.0, q.1) {
            continue;
        }
        let (fs, ft) = (fine.at(p.0, p.1).unwrap(), fine.at(q.0, q.1).unwrap());
        let Some((_, cells)) = s_fine.path(fs, ft, &|i, o| fine.neigh(i, o, true), &|i| fine.h(i, ft, true))
        else {
            continue;
        };
        let (_, reference) =
            raw_and_smoothed(&cells, p, q, &|i| fine.centre(i), fine.c / 4.0, &|x, y| fine.free_pt(x, y));
        done += 1;
        let (a, b) = (sqw.at(p.0, p.1).unwrap(), sqw.at(q.0, q.1).unwrap());
        for eight in [true, false] {
            match s_sq.path(a, b, &|i, o| sqw.neigh(i, o, eight), &|i| sqw.h(i, b, eight)) {
                Some((_, cells)) => {
                    let (raw, sm) =
                        raw_and_smoothed(&cells, p, q, &|i| sqw.centre(i), 0.125, &|x, y| sqw.free_pt(x, y));
                    if eight {
                        r8.push(raw / reference - 1.0);
                        m8.push(sm / reference - 1.0);
                    } else {
                        r4.push(raw / reference - 1.0);
                        m4.push(sm / reference - 1.0);
                    }
                }
                None => {
                    if eight {
                        lost8 += 1
                    } else {
                        lost4 += 1
                    }
                }
            }
        }
        let (a, b) = (hxw.at(p.0, p.1).unwrap(), hxw.at(q.0, q.1).unwrap());
        match s_hx.path(a, b, &|i, o| hxw.neigh(i, o), &|i| hxw.h(i, b)) {
            Some((_, cells)) => {
                let (raw, sm) =
                    raw_and_smoothed(&cells, p, q, &|i| hxw.centre(i), 0.125 * s, &|x, y| hxw.free_pt(x, y));
                r6.push(raw / reference - 1.0);
                m6.push(sm / reference - 1.0);
            }
            None => lost6 += 1,
        }
    }
    line("squares-8, raw", &mut r8);
    line("squares-8, smoothed", &mut m8);
    line("hexes-6, raw", &mut r6);
    line("hexes-6, smoothed", &mut m6);
    line("squares-4, raw (reference)", &mut r4);
    line("squares-4, smoothed (reference)", &mut m4);
    println!("  pairs: {done}; no path found at test resolution: squares-8 {lost8}, hexes-6 {lost6}, squares-4 {lost4}");
}
