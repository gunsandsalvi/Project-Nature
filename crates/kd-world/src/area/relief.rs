//! B11's metre detail, ported from `pretests/b04-b11-storage-terrain/phone/src/terrain.rs` (A2.9, A5.3): gradient
//! noise and its fractal sum, the camp site's layered escarpment with overhangs and caves, and the height map with
//! 3D pieces built from it. Only +, −, ×, ÷, floor, abs and comparisons on `f32`, with no fused multiply-add, so a
//! seed gives the same bits on every target. Single-threaded (A2.3: no threads in simulation crates); B11 gave the
//! same bits on 1 and 4 threads. B11's `f32::min` and `max` are `num::min` and `max`, equal for the finite values
//! here. The plates half waits for `MIL-04`.
//! Implements `WLD-12` and `TIM-16` in part, see A5.3.

use kd_core::num;

/// B11's mixer (`data::mix`).
#[inline]
pub fn mix(mut x: u64) -> u64 {
    x = (x ^ (x >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
    x = (x ^ (x >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
    x ^ (x >> 31)
}

/// B11's keyed draw of a seed and two numbers (`data::key`).
#[inline]
pub fn key(seed: u64, a: u64, b: u64) -> u64 {
    mix(seed ^ mix(a ^ mix(b ^ 0x9E37_79B9_7F4A_7C15)))
}

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

/// Periodic gradient noise with lattice period (`px`, `py`) in lattice units; roughly in [−1, 1].
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

/// Fractal noise on a `w` × `h` torus of samples; `period` is the largest feature in samples (a power of two).
#[allow(clippy::too_many_arguments)]
pub fn fbm(seed: u64, x: f32, y: f32, w: usize, h: usize, period: usize, octaves: u32, ridged: bool) -> f32 {
    let (mut acc, mut amp, mut norm) = (0.0f32, 1.0f32, 0.0f32);
    let mut per = period;
    for o in 0..octaves {
        if per < 2 {
            break;
        }
        let n = noise(
            seed ^ ((o as u64) << 56),
            x / per as f32,
            y / per as f32,
            (w / per).max(1) as i64,
            (h / per).max(1) as i64,
        );
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

/// The rock layer at height `z` (`PRE-23`): its index (1–6) and whether it is hard. Layers repeat every 64 m in a
/// keyed pattern of 4 m bands; a quarter are soft.
#[inline]
pub fn layer_at(seed: u64, z: f32) -> (u8, bool) {
    let zz = z.rem_euclid(64.0);
    let band = (zz / 4.0).floor() as u64;
    let r = key(seed, 0x1A7E, band);
    ((1 + (r % 6)) as u8, r & 3 != 0)
}

/// The material of a solid metre: 8 (soil) within a metre of the top, else the rock layer at its height.
pub fn material(seed: u64, z: f32, top: f32) -> u8 {
    if top - z < 1.0 { 8 } else { layer_at(seed, z).0 }
}

/// B11's camp site: a 1,024 m square with an escarpment of layered rock crossing it, caves behind the edge, and
/// overhangs where soft layers are cut back. `corners` are the coarse heights at its corners, in metres.
pub struct Site {
    pub seed: u64,
    pub corners: [f32; 4],
    /// Where the cliff edge runs: the edge's `y` for each metre column.
    line: Vec<f32>,
}

impl Site {
    pub fn new(seed: u64, corners: [f32; 4]) -> Site {
        let line = (0..1025)
            .map(|x| 512.0 + fbm(seed ^ 42, x as f32 + 0.5, 0.0, 1024, 1024, 256, 4, false) * 150.0)
            .collect();
        Site { seed, corners, line }
    }

    #[inline]
    fn line_at(&self, x: f32) -> f32 {
        self.line[(num::max(x, 0.0) as usize).min(1024)]
    }

    /// Ground height in metres ignoring overhangs and caves (the top surface).
    #[inline]
    pub fn surface(&self, x: f32, y: f32) -> f32 {
        let (u, v) = (x / 1024.0, y / 1024.0);
        let c = &self.corners;
        let base = (c[0] * (1.0 - u) + c[1] * u) * (1.0 - v) + (c[2] * (1.0 - u) + c[3] * u) * v;
        let detail = fbm(self.seed ^ 41, x, y, 1024, 1024, 256, 7, false) * 6.0;
        base + detail + self.cliff_step(x, y) * 30.0
    }

    /// 0 below the escarpment, 1 above; the edge wanders along a noisy line through the site.
    #[inline]
    pub fn cliff_step(&self, x: f32, y: f32) -> f32 {
        let d = (y - self.line_at(x)) / 2.0;
        if d <= -1.0 {
            0.0
        } else if d >= 1.0 {
            1.0
        } else {
            0.5 + d * 0.5
        }
    }

    /// Distance in metres from (`x`, `y`) to the cliff edge, positive on the high side.
    #[inline]
    pub fn edge_dist(&self, x: f32, y: f32) -> f32 {
        y - self.line_at(x)
    }

    /// Whether the point is rock: under the surface, less soft layers cut back under the cliff edge and caves.
    #[inline]
    pub fn solid(&self, x: f32, y: f32, z: f32, top: f32) -> bool {
        if z > top {
            return false;
        }
        let e = self.edge_dist(x, y);
        if e > 0.0 && e < 6.0 {
            // just behind the edge, soft layers are cut back up to 6 m: overhangs and rock shelters
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

    /// Whether the 16 m block (`bx`, `by`) could hold an overhang or a cave, from the generator's own zones.
    pub fn may_be_3d(&self, bx: usize, by: usize) -> bool {
        let mut lo = f32::MAX;
        let mut hi = f32::MIN;
        for (x, y) in [(0.0, 0.0), (16.0, 0.0), (0.0, 16.0), (16.0, 16.0), (8.0, 8.0)] {
            let e = self.edge_dist(bx as f32 * 16.0 + x, by as f32 * 16.0 + y);
            lo = num::min(lo, e);
            hi = num::max(hi, e);
        }
        hi > -2.0 - 24.0 && lo < 120.0 + 24.0
    }
}

/// A 16 m block of 1 m cubes (x, y and z in blocks) and each cube's material, 0 for air.
pub type Piece = ((u16, u16, i32), Vec<u8>);

/// The site as a height map with local 3D pieces (B11-1): heights in decimetres above the lowest point, a material
/// per column, and the 16 m blocks of cubes where the land is not a height field.
pub struct HeightPieces {
    pub base_dm: f32,
    pub height: Vec<u16>,
    pub layer: Vec<u8>,
    pub pieces: Vec<Piece>,
}

impl HeightPieces {
    /// Bytes held.
    pub fn bytes(&self) -> usize {
        self.height.len() * 2 + self.layer.len() + self.pieces.iter().map(|p| 8 + p.1.len()).sum::<usize>()
    }

    /// B11's hash of the whole, over a hash of bytes: `kd_core::num::hash64` in the game, a copy of B11's own in the
    /// porting check (A5.3).
    pub fn hash(&self, bytes: impl Fn(&[u8]) -> u64) -> u64 {
        let mut h = 0x4E1_u64 ^ u64::from(self.base_dm.to_bits());
        let hb: Vec<u8> = self.height.iter().flat_map(|v| v.to_le_bytes()).collect();
        h = mix(h ^ bytes(&hb));
        h = mix(h ^ bytes(&self.layer));
        for ((x, y, z), b) in &self.pieces {
            h = mix(h ^ ((u64::from(*x) << 48) | (u64::from(*y) << 32) | u64::from(*z as u32)));
            h = mix(h ^ bytes(b));
        }
        h
    }
}

/// One 16 m block of cubes and whether air lies under the top surface in it; `None` when all air or all rock.
fn block(site: &Site, tops: &[f32], bx: usize, by: usize, bz: i32) -> (Option<Vec<u8>>, bool) {
    let mut v = vec![0u8; 4096];
    let (mut any_air, mut any_solid, mut overhang) = (false, false, false);
    for ly in 0..16 {
        for lx in 0..16 {
            let (x, y) = (bx * 16 + lx, by * 16 + ly);
            let top = tops[y * 1024 + x];
            for lz in (0..16).rev() {
                let z = (bz * 16 + lz as i32) as f32 + 0.5;
                let s = site.solid(x as f32 + 0.5, y as f32 + 0.5, z, top);
                if s {
                    v[(lz * 16 + ly) * 16 + lx] = material(site.seed, z, top);
                    any_solid = true;
                } else {
                    any_air = true;
                }
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

/// The top surface at the middle of every metre column of the site, row by row.
pub fn tops(site: &Site) -> Vec<f32> {
    (0..1024 * 1024)
        .map(|idx| site.surface((idx % 1024) as f32 + 0.5, (idx / 1024) as f32 + 0.5))
        .collect()
}

/// The site's height map and its 3D pieces, only in blocks the generator marks as possibly 3D and only where they
/// really are.
pub fn build_height_pieces(site: &Site) -> HeightPieces {
    let t = tops(site);
    let lo = t.iter().copied().fold(f32::MAX, num::min);
    let base_dm = (lo * 10.0).floor();
    let height: Vec<u16> = t
        .iter()
        .map(|v| num::min(num::max((v * 10.0).floor() - base_dm, 0.0), 65535.0) as u16)
        .collect();
    let layer: Vec<u8> = t.iter().map(|v| material(site.seed, v - 0.5, *v)).collect();
    let mut pieces = Vec::new();
    for i in 0..64 * 64 {
        let (bx, by) = (i % 64, i / 64);
        if !site.may_be_3d(bx, by) {
            continue;
        }
        let mut lo = f32::MAX;
        let mut hi = f32::MIN;
        for y in by * 16..by * 16 + 16 {
            for x in bx * 16..bx * 16 + 16 {
                lo = num::min(lo, t[y * 1024 + x]);
                hi = num::max(hi, t[y * 1024 + x]);
            }
        }
        let (z0, z1) = (((lo - 41.0) / 16.0).floor() as i32, ((hi + 1.0) / 16.0).floor() as i32);
        for bz in z0..=z1 {
            if let (Some(v), true) = block(site, &t, bx, by, bz) {
                pieces.push(((bx as u16, by as u16, bz), v));
            }
        }
    }
    HeightPieces {
        base_dm,
        height,
        layer,
        pieces,
    }
}

#[cfg(test)]
mod tests;
