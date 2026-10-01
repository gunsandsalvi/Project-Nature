//! B11: terrain. World generation at about 1 km cells on the `WLD-03` torus (`WLD-11`), and
//! metre-level detail around a camp (`WLD-12`, `PRE-23`, `PRE-24`) stored two ways.
//! Generation uses only +, -, *, /, sqrt and floor on f32, with no fused multiply-add, so the
//! same seed gives the same bits on any IEEE machine (the phone reports its hash to compare).

use crate::data::{key, mix};

pub const CELL_M: f32 = 977.0; // 2,000 km / 2048 cells (B10)

#[inline]
fn grad(h: u64, dx: f32, dy: f32) -> f32 {
    match h & 7 {
        0 => dx + dy,
        1 => dx - dy,
        2 => -dx + dy,
        3 => -dx - dy,
        4 => dx,
        5 => -dx,
        6 => dy,
        _ => -dy,
    }
}
#[inline]
fn fade(t: f32) -> f32 {
    t * t * t * (t * (t * 6.0 - 15.0) + 10.0)
}

/// Periodic gradient noise, lattice period (px, py) in lattice units; roughly in [-1, 1].
#[inline]
pub fn noise(seed: u64, x: f32, y: f32, px: i64, py: i64) -> f32 {
    let (xf, yf) = (x.floor(), y.floor());
    let (fx, fy) = (x - xf, y - yf);
    let (ix, iy) = (xf as i64, yf as i64);
    let h = |a: i64, b: i64| key(seed, a.rem_euclid(px) as u64, b.rem_euclid(py) as u64);
    let n00 = grad(h(ix, iy), fx, fy);
    let n10 = grad(h(ix + 1, iy), fx - 1.0, fy);
    let n01 = grad(h(ix, iy + 1), fx, fy - 1.0);
    let n11 = grad(h(ix + 1, iy + 1), fx - 1.0, fy - 1.0);
    let (u, v) = (fade(fx), fade(fy));
    let a = n00 + (n10 - n00) * u;
    let b = n01 + (n11 - n01) * u;
    a + (b - a) * v
}

/// Fractal noise on a w x h torus of samples. `period` = largest feature in samples (power of two).
pub fn fbm(seed: u64, x: f32, y: f32, w: usize, h: usize, period: usize, octaves: u32, ridged: bool) -> f32 {
    let (mut acc, mut amp, mut norm) = (0.0f32, 1.0f32, 0.0f32);
    let mut per = period;
    for o in 0..octaves {
        if per < 2 {
            break;
        }
        let n = noise(seed ^ (o as u64) << 56, x / per as f32, y / per as f32, (w / per).max(1) as i64, (h / per).max(1) as i64);
        let v = if ridged {
            let r = 1.0 - n.abs();
            r * r
        } else {
            n
        };
        acc += amp * v;
        norm += amp;
        amp *= 0.5;
        per /= 2;
    }
    acc / norm
}

pub struct Map {
    pub w: usize,
    pub h: usize,
    pub z: Vec<f32>, // metres; sea level 0
}

fn par_rows(w: usize, h: usize, threads: usize, f: &(dyn Fn(usize, usize) -> f32 + Sync)) -> Vec<f32> {
    let mut z = vec![0f32; w * h];
    let t = threads.max(1);
    let rows_per = (h + t - 1) / t;
    std::thread::scope(|s| {
        for (k, part) in z.chunks_mut(rows_per * w).enumerate() {
            s.spawn(move || {
                for (i, v) in part.iter_mut().enumerate() {
                    let idx = k * rows_per * w + i;
                    *v = f(idx % w, idx / w);
                }
            });
        }
    });
    z
}

/// Shift heights so that `land` of the cells lie above sea level (exact quantile by sorting).
fn set_sea_level(z: &mut [f32], land: f32) {
    let mut s: Vec<f32> = z.to_vec();
    let k = ((1.0 - land) * s.len() as f32) as usize;
    let (_, q, _) = s.select_nth_unstable_by(k.min(z.len() - 1), |a, b| a.total_cmp(b));
    let q = *q;
    for v in z.iter_mut() {
        *v -= q;
    }
}

/// (a) Shaped noise with domain warping.
pub fn gen_warp(seed: u64, w: usize, h: usize, threads: usize) -> Map {
    let sc = w / 2048usize.min(w).max(1); // keep features the same size in km on smaller maps
    let _ = sc;
    let p = |km: usize| (km * w / 2048).max(2);
    let f = |x: usize, y: usize| {
        let (xf, yf) = (x as f32, y as f32);
        let wx = fbm(seed ^ 11, xf, yf, w, h, p(256), 4, false) * p(96) as f32;
        let wy = fbm(seed ^ 12, xf, yf, w, h, p(256), 4, false) * p(96) as f32;
        let (qx, qy) = (xf + wx, yf + wy);
        let cont = fbm(seed ^ 13, qx, qy, w, h, p(512), 6, false);
        let ridge = fbm(seed ^ 14, qx, qy, w, h, p(128), 5, true);
        let detail = fbm(seed ^ 15, xf, yf, w, h, p(32), 4, false);
        let inland = (cont * 4.0).max(0.0).min(1.0);
        cont * 3000.0 + ridge * ridge * inland * 3500.0 + detail * 150.0
    };
    let mut z = par_rows(w, h, threads, &f);
    set_sea_level(&mut z, 0.35);
    Map { w, h, z }
}

// ---------- erosion: priority flood, steepest descent, implicit stream power ----------

#[inline]
fn nb(w: usize, h: usize, i: usize, k: usize) -> usize {
    const D: [(i64, i64); 8] = [(1, 0), (-1, 0), (0, 1), (0, -1), (1, 1), (-1, 1), (1, -1), (-1, -1)];
    let (x, y) = ((i % w) as i64, (i / w) as i64);
    let (dx, dy) = D[k];
    (((y + dy).rem_euclid(h as i64)) as usize) * w + ((x + dx).rem_euclid(w as i64)) as usize
}
const DIST: [f32; 8] = [1.0, 1.0, 1.0, 1.0, 1.414_213_6, 1.414_213_6, 1.414_213_6, 1.414_213_6];

#[inline]
fn okey(v: f32) -> u32 {
    let b = v.to_bits();
    if b >> 31 == 1 {
        !b
    } else {
        b | 0x8000_0000
    }
}

/// Fill pits so every land cell drains to the sea (priority flood with a tiny slope).
/// Returns the order cells were settled in (lowest first; sea cells are not listed).
pub fn fill(m: &mut Map) -> Vec<u32> {
    use std::cmp::Reverse;
    use std::collections::BinaryHeap;
    let (w, h) = (m.w, m.h);
    let n = w * h;
    let mut done = vec![false; n];
    let mut heap = BinaryHeap::new();
    for i in 0..n {
        if m.z[i] <= 0.0 {
            done[i] = true;
            if (0..8).any(|k| m.z[nb(w, h, i, k)] > 0.0) {
                heap.push(Reverse((okey(m.z[i]), i as u32)));
            }
        }
    }
    let mut order = Vec::with_capacity(n);
    while let Some(Reverse((_, i))) = heap.pop() {
        let i = i as usize;
        if m.z[i] > 0.0 {
            order.push(i as u32);
        }
        for k in 0..8 {
            let j = nb(w, h, i, k);
            if !done[j] {
                done[j] = true;
                let min = m.z[i] + 0.01 * DIST[k];
                if m.z[j] < min {
                    m.z[j] = min;
                }
                heap.push(Reverse((okey(m.z[j]), j as u32)));
            }
        }
    }
    order
}

/// Steepest-descent receiver of each cell (itself if no lower neighbour), and its distance in cells.
pub fn receivers(m: &Map) -> (Vec<u32>, Vec<f32>) {
    let (w, h) = (m.w, m.h);
    let mut r = vec![0u32; w * h];
    let mut d = vec![1f32; w * h];
    for i in 0..w * h {
        let (mut best, mut bs, mut bd) = (i, 0f32, 1f32);
        for k in 0..8 {
            let j = nb(w, h, i, k);
            let s = (m.z[i] - m.z[j]) / DIST[k];
            if s > bs {
                bs = s;
                best = j;
                bd = DIST[k];
            }
        }
        r[i] = best as u32;
        d[i] = bd;
    }
    (r, d)
}

/// Drainage area in cells, accumulated from high to low.
pub fn drainage(m: &Map, rec: &[u32]) -> Vec<f32> {
    let mut idx: Vec<u32> = (0..(m.w * m.h) as u32).collect();
    idx.sort_unstable_by(|a, b| m.z[*b as usize].total_cmp(&m.z[*a as usize]).then(a.cmp(b)));
    let mut a = vec![1f32; m.w * m.h];
    for &i in &idx {
        let r = rec[i as usize] as usize;
        if r != i as usize {
            a[r] += a[i as usize];
        }
    }
    a
}

/// Quick erosion: `iters` rounds of fill + steepest descent + implicit stream power
/// (Braun and Willett 2013), then a light hillslope smoothing.
pub fn erode(m: &mut Map, iters: usize, k: f32) {
    let (w, h) = (m.w, m.h);
    let cells_per_km = (w as f32 / 2048.0).max(1.0 / 64.0);
    for _ in 0..iters {
        let order = fill(m);
        let (rec, dist) = receivers(m);
        // drainage in settle order (lowest first) reversed = highest first
        let mut a = vec![1f32; w * h];
        for &i in order.iter().rev() {
            let r = rec[i as usize] as usize;
            if r != i as usize {
                a[r] += a[i as usize];
            }
        }
        for &i in &order {
            let i = i as usize;
            let r = rec[i] as usize;
            if r == i {
                continue;
            }
            let area_km2 = a[i] / (cells_per_km * cells_per_km);
            let f = k * area_km2.sqrt() / dist[i];
            m.z[i] = (m.z[i] + f * m.z[r]) / (1.0 + f);
        }
    }
    // hillslope: pull very steep land toward its neighbours a little
    for _ in 0..2 {
        let z0 = m.z.clone();
        for i in 0..w * h {
            if z0[i] <= 0.0 {
                continue;
            }
            let mut s = 0.0f32;
            for k in 0..4 {
                s += z0[nb(w, h, i, k)];
            }
            m.z[i] = z0[i] * 0.6 + s * 0.1;
        }
    }
    fill(m);
}

/// (b) Noise, then the quick erosion pass.
pub fn gen_erode(seed: u64, w: usize, h: usize, threads: usize) -> Map {
    let p = |km: usize| (km * w / 2048).max(2);
    let f = |x: usize, y: usize| {
        let (xf, yf) = (x as f32, y as f32);
        let cont = fbm(seed ^ 21, xf, yf, w, h, p(512), 7, false);
        let ridge = fbm(seed ^ 22, xf, yf, w, h, p(128), 5, true);
        let inland = (cont * 4.0).max(0.0).min(1.0);
        cont * 3000.0 + ridge * inland * 2500.0
    };
    let mut m = Map { w, h, z: par_rows(w, h, threads, &f) };
    set_sea_level(&mut m.z, 0.35);
    erode(&mut m, 4, 0.004);
    m
}

/// (c) Simple plates with uplift at converging edges, noise detail, then the same erosion.
pub fn gen_plates(seed: u64, w: usize, h: usize, threads: usize) -> Map {
    let p = |km: usize| (km * w / 2048).max(2);
    let np = 14u64;
    let plates: Vec<(f32, f32, f32, f32, bool)> = (0..np)
        .map(|i| {
            let r = key(seed, 0x91A7E, i);
            let (x, y) = ((r % w as u64) as f32, ((r >> 20) % h as u64) as f32);
            let a = ((r >> 40) % 1000) as f32 / 1000.0;
            // direction from a rational approximation of a circle (no sin/cos)
            let t = a * 4.0 - 2.0;
            let (vx, vy) = ((1.0 - t * t) / (1.0 + t * t), 2.0 * t / (1.0 + t * t));
            (x, y, vx, vy, (r >> 60) % 5 < 2)
        })
        .collect();
    let (wf, hf) = (w as f32, h as f32);
    let f = |x: usize, y: usize| {
        let (xf, yf) = (x as f32, y as f32);
        let qx = xf + fbm(seed ^ 31, xf, yf, w, h, p(256), 4, false) * p(64) as f32;
        let qy = yf + fbm(seed ^ 32, xf, yf, w, h, p(256), 4, false) * p(64) as f32;
        let (mut d1, mut d2, mut i1, mut i2) = (f32::MAX, f32::MAX, 0usize, 0usize);
        let (mut dx1, mut dy1) = (0.0, 0.0);
        let mut dxy = [(0f32, 0f32); 14];
        for (i, pl) in plates.iter().enumerate() {
            let mut dx = (qx - pl.0).rem_euclid(wf);
            if dx > wf / 2.0 {
                dx -= wf;
            }
            let mut dy = (qy - pl.1).rem_euclid(hf);
            if dy > hf / 2.0 {
                dy -= hf;
            }
            dxy[i] = (dx, dy);
            let d = (dx * dx + dy * dy).sqrt();
            if d < d1 {
                d2 = d1;
                i2 = i1;
                d1 = d;
                i1 = i;
                dx1 = dx;
                dy1 = dy;
            } else if d < d2 {
                d2 = d;
                i2 = i;
            }
        }
        let _ = (dx1, dy1);
        let (a, b) = (plates[i1], plates[i2]);
        // direction from plate 1's centre toward plate 2's, through this cell
        let (ux, uy) = (dxy[i1].0 - dxy[i2].0, dxy[i1].1 - dxy[i2].1);
        let ul = (ux * ux + uy * uy).sqrt().max(1e-3);
        let conv = ((a.2 - b.2) * ux + (a.3 - b.3) * uy) / ul; // > 0: plates close in
        let edge = (d2 - d1) * 0.5 / p(48) as f32; // distance to the edge in units of 48 km
        let bump = 1.0 / (1.0 + edge * edge);
        let base = if a.4 { 300.0 } else { -3200.0 };
        let other = if b.4 { 300.0 } else { -3200.0 };
        let blend = 0.5 / (1.0 + edge * 2.0);
        let mut z = base * (1.0 - blend) + other * blend;
        if conv > 0.0 {
            let both = if a.4 && b.4 { 1.0 } else { 0.7 };
            z += conv * both * 4200.0 * bump;
        } else {
            z += conv * 900.0 * bump; // rifts
        }
        let ridge = fbm(seed ^ 33, xf, yf, w, h, p(64), 5, true);
        let detail = fbm(seed ^ 34, xf, yf, w, h, p(256), 6, false);
        z + detail * 900.0 + ridge * bump * 1500.0
    };
    let mut m = Map { w, h, z: par_rows(w, h, threads, &f) };
    set_sea_level(&mut m.z, 0.35);
    erode(&mut m, 3, 0.004);
    m
}

pub fn generate(method: &str, seed: u64, w: usize, h: usize, threads: usize) -> Map {
    match method {
        "warp" => gen_warp(seed, w, h, threads),
        "erode" => gen_erode(seed, w, h, threads),
        _ => gen_plates(seed, w, h, threads),
    }
}

pub fn map_hash(m: &Map) -> u64 {
    let mut b = Vec::with_capacity(m.z.len() * 4);
    for v in &m.z {
        b.extend_from_slice(&v.to_bits().to_le_bytes());
    }
    crate::data::hash(&b)
}

/// Quality check (B11-2): rivers reaching the sea by steepest descent, and the slope spread.
pub struct Quality {
    pub land: f64,
    pub river_cells: usize,
    pub river_to_sea: f64,
    pub land_to_sea: f64,
    pub pits_per_10k_km2: f64,
    pub slope_pct: [f64; 5], // p10, p50, p90, p99 in degrees, then share of land above 30 degrees
}

pub fn quality(m: &Map) -> Quality {
    let (w, h) = (m.w, m.h);
    let n = w * h;
    let km_per_cell = 2000.0 / w as f64;
    let (rec, _) = receivers(m);
    let a = drainage(m, &rec);
    // where does each cell end up? follow receivers with memo
    let mut end = vec![u32::MAX; n]; // 1 = sea, 0 = pit
    let mut stack = Vec::new();
    for i in 0..n {
        let mut j = i;
        while end[j] == u32::MAX {
            if m.z[j] <= 0.0 {
                end[j] = 1;
                break;
            }
            let r = rec[j] as usize;
            if r == j {
                end[j] = 0;
                break;
            }
            stack.push(j);
            j = r;
        }
        let e = end[j];
        for k in stack.drain(..) {
            end[k] = e;
        }
    }
    let land: Vec<usize> = (0..n).filter(|&i| m.z[i] > 0.0).collect();
    let river_min = 50.0 / (km_per_cell * km_per_cell);
    let rivers: Vec<usize> = land.iter().copied().filter(|&i| a[i] as f64 >= river_min).collect();
    let pits = land.iter().filter(|&&i| rec[i] as usize == i).count();
    let land_km2 = land.len() as f64 * km_per_cell * km_per_cell;
    let mut slopes: Vec<f64> = land
        .iter()
        .map(|&i| {
            let gx = (m.z[nb(w, h, i, 0)] - m.z[nb(w, h, i, 1)]) as f64 / (2.0 * km_per_cell * 1000.0);
            let gy = (m.z[nb(w, h, i, 2)] - m.z[nb(w, h, i, 3)]) as f64 / (2.0 * km_per_cell * 1000.0);
            (gx * gx + gy * gy).sqrt().atan().to_degrees()
        })
        .collect();
    slopes.sort_by(|a, b| a.total_cmp(b));
    let q = |p: f64| slopes[((slopes.len() - 1) as f64 * p) as usize];
    let steep = slopes.iter().filter(|&&s| s > 30.0).count() as f64 / slopes.len().max(1) as f64;
    Quality {
        land: land.len() as f64 / n as f64,
        river_cells: rivers.len(),
        river_to_sea: rivers.iter().filter(|&&i| end[i] == 1).count() as f64 / rivers.len().max(1) as f64,
        land_to_sea: land.iter().filter(|&&i| end[i] == 1).count() as f64 / land.len().max(1) as f64,
        pits_per_10k_km2: pits as f64 / land_km2 * 10_000.0,
        slope_pct: [q(0.10), q(0.50), q(0.90), q(0.99), steep * 100.0],
    }
}

// ---------------- B11-1: metre detail around a camp ----------------

/// Rock layers of different thicknesses (PRE-23): layer index and hardness at height z.
#[inline]
fn layer_at(seed: u64, z: f32) -> (u8, bool) {
    // layers 1.5 to 6 m thick, repeating every 64 m with a keyed pattern
    let zz = z.rem_euclid(64.0);
    let band = (zz / 4.0).floor() as u64; // 16 bands of 4 m; thickness varies by merging
    let r = key(seed, 0x1A7E, band);
    ((1 + (r % 6)) as u8, r & 3 != 0) // a quarter of the layers are soft
}

/// The camp site: a 1,024 x 1,024 m cell with an escarpment (cliff of layered rock), caves in it,
/// and overhangs where soft layers are cut back. `base` is the cell's interpolated coarse height
/// field (corners). Solid where `density > 0`.
pub struct Site {
    pub seed: u64,
    pub corners: [f32; 4],
}

impl Site {
    /// Ground height ignoring overhangs and caves (the top surface).
    #[inline]
    pub fn surface(&self, x: f32, y: f32) -> f32 {
        let (u, v) = (x / 1024.0, y / 1024.0);
        let c = &self.corners;
        let base = (c[0] * (1.0 - u) + c[1] * u) * (1.0 - v) + (c[2] * (1.0 - u) + c[3] * u) * v;
        let detail = fbm(self.seed ^ 41, x, y, 1024, 1024, 256, 7, false) * 6.0;
        base + detail + self.cliff_step(x, y) * 30.0
    }
    /// 0 below the escarpment, 1 above; the edge wanders along a noisy line through the cell.
    #[inline]
    pub fn cliff_step(&self, x: f32, y: f32) -> f32 {
        let line = 512.0 + fbm(self.seed ^ 42, x, 0.0, 1024, 1024, 256, 4, false) * 150.0;
        let d = (y - line) / 2.0;
        if d <= -1.0 {
            0.0
        } else if d >= 1.0 {
            1.0
        } else {
            0.5 + d * 0.5
        }
    }
    /// Distance (m) from (x, y) to the cliff edge, positive on the high side.
    #[inline]
    fn edge_dist(&self, x: f32, y: f32) -> f32 {
        y - (512.0 + fbm(self.seed ^ 42, x, 0.0, 1024, 1024, 256, 4, false) * 150.0)
    }
    /// Full 3D solid test: surface, minus soft layers cut back under the cliff edge, minus caves.
    #[inline]
    pub fn solid(&self, x: f32, y: f32, z: f32, top: f32) -> bool {
        if z > top {
            return false;
        }
        let e = self.edge_dist(x, y);
        if e > 0.0 && e < 6.0 {
            // just behind the edge: soft layers are cut back up to 6 m (overhangs, rock shelters)
            let (_, hard) = layer_at(self.seed, z);
            let bottom = top - 30.0;
            if !hard && z > bottom + 1.0 && z < top - 2.0 {
                let cut = 2.0 + 4.0 * (0.5 + 0.5 * noise(self.seed ^ 43, x / 16.0, z / 8.0, 64, 1 << 20));
                if e < cut {
                    return false;
                }
            }
        }
        if e > -2.0 && e < 120.0 {
            // caves: where two noise surfaces cross, inside the escarpment
            let depth = top - z;
            if depth > 2.0 && depth < 40.0 {
                let a = noise(self.seed ^ 44, x / 24.0, y / 24.0 + z / 12.0, 1 << 20, 1 << 20);
                let b = noise(self.seed ^ 45, x / 24.0 + z / 12.0, y / 24.0, 1 << 20, 1 << 20);
                if a.abs() < 0.08 && b.abs() < 0.08 {
                    return false;
                }
            }
        }
        true
    }
    /// Could this 16 m block hold an overhang or a cave? (known from the generator's own zones)
    pub fn may_be_3d(&self, bx: usize, by: usize) -> bool {
        let mut lo = f32::MAX;
        let mut hi = f32::MIN;
        for (x, y) in [(0.0, 0.0), (16.0, 0.0), (0.0, 16.0), (16.0, 16.0), (8.0, 8.0)] {
            let e = self.edge_dist(bx as f32 * 16.0 + x, by as f32 * 16.0 + y);
            lo = lo.min(e);
            hi = hi.max(e);
        }
        hi > -2.0 - 24.0 && lo < 120.0 + 24.0
    }
}

pub fn material(seed: u64, z: f32, top: f32) -> u8 {
    if top - z < 1.0 {
        8 // soil
    } else {
        layer_at(seed, z).0
    }
}

/// Height map with local 3D pieces. Heights in decimetres (u16, above the tile's lowest point),
/// a rock layer per column, and 16 x 16 x 16 blocks of 1 m cubes where the land is not a height field.
pub struct HeightPieces {
    pub base_dm: f32,
    pub height: Vec<u16>,
    pub layer: Vec<u8>,
    pub pieces: Vec<((u16, u16, i32), Vec<u8>)>,
}

impl HeightPieces {
    pub fn bytes(&self) -> usize {
        self.height.len() * 2 + self.layer.len() + self.pieces.iter().map(|p| 8 + p.1.len()).sum::<usize>()
    }
    pub fn hash(&self) -> u64 {
        let mut h = crate::data::hash(crate::data::as_bytes(&[0u8; 0])) ^ self.base_dm.to_bits() as u64;
        let hb: Vec<u8> = self.height.iter().flat_map(|v| v.to_le_bytes()).collect();
        h = mix(h ^ crate::data::hash(&hb));
        h = mix(h ^ crate::data::hash(&self.layer));
        for ((x, y, z), b) in &self.pieces {
            h = mix(h ^ ((*x as u64) << 48 | (*y as u64) << 32 | *z as u32 as u64));
            h = mix(h ^ crate::data::hash(b));
        }
        h
    }
}

/// One 16 m block of cubes; None if it is all air or all rock.
fn block(site: &Site, tops: &[f32], bx: usize, by: usize, bz: i32) -> (Option<Vec<u8>>, bool) {
    let mut v = vec![0u8; 4096];
    let (mut any_air, mut any_solid, mut overhang) = (false, false, false);
    for ly in 0..16 {
        for lx in 0..16 {
            let (x, y) = (bx * 16 + lx, by * 16 + ly);
            let top = tops[y * 1024 + x];
            let mut prev_air = false;
            for lz in (0..16).rev() {
                let z = (bz * 16 + lz as i32) as f32 + 0.5;
                let s = site.solid(x as f32 + 0.5, y as f32 + 0.5, z, top);
                if s {
                    v[(lz * 16 + ly) * 16 + lx] = material(site.seed, z, top);
                    any_solid = true;
                } else {
                    any_air = true;
                    if z < top {
                        prev_air = true;
                    }
                }
                let _ = prev_air;
                if !s && z < top - 0.5 {
                    overhang = true; // air under the top surface: a cave or an overhang
                }
            }
        }
    }
    if any_air && any_solid {
        (Some(v), overhang)
    } else {
        (None, overhang)
    }
}

/// Top surface heights for the whole tile (1,024 x 1,024 columns), split over `threads`.
pub fn tops(site: &Site, threads: usize) -> Vec<f32> {
    let mut t = vec![0f32; 1024 * 1024];
    let rows = (1024 + threads.max(1) - 1) / threads.max(1);
    std::thread::scope(|s| {
        for (k, part) in t.chunks_mut(rows * 1024).enumerate() {
            s.spawn(move || {
                for (i, v) in part.iter_mut().enumerate() {
                    let idx = k * rows * 1024 + i;
                    *v = site.surface((idx % 1024) as f32 + 0.5, (idx / 1024) as f32 + 0.5);
                }
            });
        }
    });
    t
}

fn par_blocks<T: Send>(n: usize, threads: usize, f: &(dyn Fn(usize) -> T + Sync)) -> Vec<T> {
    let t = threads.max(1);
    let per = (n + t - 1) / t;
    let mut parts: Vec<Vec<T>> = Vec::new();
    std::thread::scope(|s| {
        let hs: Vec<_> = (0..t).map(|k| s.spawn(move || (k * per..((k + 1) * per).min(n)).map(f).collect::<Vec<T>>())).collect();
        for h in hs {
            parts.push(h.join().unwrap());
        }
    });
    parts.into_iter().flatten().collect()
}

pub fn build_height_pieces(site: &Site, threads: usize) -> HeightPieces {
    let t = tops(site, threads);
    let lo = t.iter().cloned().fold(f32::MAX, f32::min);
    let base_dm = (lo * 10.0).floor();
    let height: Vec<u16> = t.iter().map(|v| ((v * 10.0).floor() - base_dm).max(0.0).min(65535.0) as u16).collect();
    let layer: Vec<u8> = t.iter().map(|v| material(site.seed, v - 0.5, *v)).collect();
    // 3D pieces only in blocks the generator marks as possibly 3D, and only if they really are.
    let cand: Vec<(usize, usize)> = (0..64 * 64).map(|i| (i % 64, i / 64)).filter(|&(bx, by)| site.may_be_3d(bx, by)).collect();
    let tt = &t;
    let found = par_blocks(cand.len(), threads, &|k| {
        let (bx, by) = cand[k];
        let mut lo = f32::MAX;
        let mut hi = f32::MIN;
        for y in by * 16..by * 16 + 16 {
            for x in bx * 16..bx * 16 + 16 {
                lo = lo.min(tt[y * 1024 + x]);
                hi = hi.max(tt[y * 1024 + x]);
            }
        }
        let mut out = Vec::new();
        let (z0, z1) = (((lo - 41.0) / 16.0).floor() as i32, ((hi + 1.0) / 16.0).floor() as i32);
        for bz in z0..=z1 {
            if let (Some(v), true) = block(site, tt, bx, by, bz) {
                out.push(((bx as u16, by as u16, bz), v));
            }
        }
        out
    });
    HeightPieces { base_dm, height, layer, pieces: found.into_iter().flatten().collect() }
}

/// Sparse grid of 1 m cubes in 16 m blocks: every block the surface or a cave passes through.
pub struct Cubes {
    pub blocks: Vec<((u16, u16, i32), Vec<u8>)>,
    pub uniform: usize,
}
impl Cubes {
    pub fn bytes(&self) -> usize {
        self.blocks.iter().map(|b| 8 + b.1.len()).sum::<usize>() + self.uniform * 8
    }
    pub fn hash(&self) -> u64 {
        let mut h = 0x5EED_u64 ^ self.uniform as u64;
        for ((x, y, z), b) in &self.blocks {
            h = mix(h ^ ((*x as u64) << 48 | (*y as u64) << 32 | *z as u32 as u64));
            h = mix(h ^ crate::data::hash(b));
        }
        h
    }
}

pub fn build_cubes(site: &Site, threads: usize) -> Cubes {
    let t = tops(site, threads);
    let tt = &t;
    let found = par_blocks(64 * 64, threads, &|i| {
        let (bx, by) = (i % 64, i / 64);
        let mut lo = f32::MAX;
        let mut hi = f32::MIN;
        for y in by * 16..by * 16 + 16 {
            for x in bx * 16..bx * 16 + 16 {
                lo = lo.min(tt[y * 1024 + x]);
                hi = hi.max(tt[y * 1024 + x]);
            }
        }
        // the cube grid does not know where caves are: it checks every block down to cave depth
        let (z0, z1) = (((lo - 41.0) / 16.0).floor() as i32, ((hi + 1.0) / 16.0).floor() as i32);
        let mut out = Vec::new();
        let mut uni = 0usize;
        for bz in z0..=z1 {
            match block(site, tt, bx, by, bz) {
                (Some(v), _) => out.push(((bx as u16, by as u16, bz), v)),
                (None, _) => uni += 1,
            }
        }
        (out, uni)
    });
    let mut c = Cubes { blocks: Vec::new(), uniform: 0 };
    for (o, u) in found {
        c.blocks.extend(o);
        c.uniform += u;
    }
    c
}
