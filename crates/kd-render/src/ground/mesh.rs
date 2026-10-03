//! Height-field chunks (A11.5), ported from the mockup's `GeoTerrain.buildGrid`, `addSkirts` and `MeshBuilder`:
//! 64 m chunks with one vertex spacing a frame, chosen by the art pixel's size; a 20-byte vertex; normals from the
//! heights blurred three times; 4 m skirts round the area's outer edge; `u16` indices.
//! Implements `PRE-02` in part.

use kd_view::GroundGrid;

/// Metres along a chunk's side (A11.5).
pub const CHUNK_M: u32 = 64;
/// How far a skirt hangs below the edge, metres (`addSkirts`).
pub const SKIRT_M: f32 = 4.0;
/// Bytes a vertex.
pub const VERTEX_BYTES: usize = 20;

/// One ground vertex (A11.5): its position in metres from its area's corner (x east, y up, z south), the normal in
/// steps of 1/127, then a byte each for the surface (its row of the surfaces texture), the distance to water, wear,
/// cover and flags, in place of the mockup's `aA` and `aB`.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct GroundVertex {
    pub pos: [f32; 3],
    pub nrm: [i8; 3],
    pub surface: u8,
    pub water: u8,
    pub wear: u8,
    pub cover: u8,
    pub flags: u8,
}

impl GroundVertex {
    /// The vertex as the GPU reads it: 20 bytes, little-endian.
    pub fn bytes(&self) -> [u8; VERTEX_BYTES] {
        let mut b = [0u8; VERTEX_BYTES];
        for (k, v) in self.pos.iter().enumerate() {
            b[k * 4..k * 4 + 4].copy_from_slice(&v.to_le_bytes());
        }
        for (k, n) in self.nrm.iter().enumerate() {
            b[12 + k] = n.to_le_bytes()[0];
        }
        b[15..20].copy_from_slice(&[self.surface, self.water, self.wear, self.cover, self.flags]);
        b
    }
}

/// One chunk's mesh: its place in the area in chunks, its vertices and its triangles.
#[derive(Clone, Debug, Default, PartialEq)]
pub struct Chunk {
    pub cx: u32,
    pub cy: u32,
    pub verts: Vec<GroundVertex>,
    pub idx: Vec<u16>,
}

/// The vertex spacing in metres for art pixels of `texel` metres (A11.5): 1 m below 0.6 m, 2 m below 1.3, 4 m below
/// 2.6, else 8 m.
pub fn spacing_for(texel: f32) -> u32 {
    if texel < 0.6 {
        1
    } else if texel < 1.3 {
        2
    } else if texel < 2.6 {
        4
    } else {
        8
    }
}

/// The area's grid at `step` metres: heights, then heights blurred three times for the normals (`buildGrid`).
struct Grid {
    n: usize,
    step: usize,
    z: Vec<f32>,
    nrm: Vec<[i8; 3]>,
    surface: Vec<u8>,
}

fn grid(g: &GroundGrid, step: usize) -> Grid {
    let n = (g.side - 1) / step + 1;
    let at = |i: usize, j: usize| g.heights_m[j * step * g.side + i * step];
    let z: Vec<f32> = (0..n * n).map(|k| at(k % n, k / n)).collect();
    // blur three times, keeping the edges
    let mut zs = z.clone();
    for _ in 0..3 {
        let zt = zs.clone();
        for j in 1..n - 1 {
            for i in 1..n - 1 {
                let k = j * n + i;
                zs[k] = (zt[k] * 4.0 + zt[k - 1] + zt[k + 1] + zt[k - n] + zt[k + n]) / 8.0;
            }
        }
    }
    let d = step as f32;
    let mut nrm = vec![[0i8; 3]; n * n];
    for j in 0..n {
        for i in 0..n {
            let (ia, ib) = (i.saturating_sub(1), (i + 1).min(n - 1));
            let (ja, jb) = (j.saturating_sub(1), (j + 1).min(n - 1));
            // tangents along x and along y (south), heights up: n = tx × ty, then into GPU axes (x, up, south)
            let tx = [(ib - ia) as f32 * d, 0.0, zs[j * n + ib] - zs[j * n + ia]];
            let ty = [0.0, (jb - ja) as f32 * d, zs[jb * n + i] - zs[ja * n + i]];
            let c = [
                tx[1] * ty[2] - tx[2] * ty[1],
                tx[2] * ty[0] - tx[0] * ty[2],
                tx[0] * ty[1] - tx[1] * ty[0],
            ];
            let l = (c[0] * c[0] + c[1] * c[1] + c[2] * c[2]).sqrt();
            let q = |v: f32| (v / l * 127.0).round() as i8;
            nrm[j * n + i] = [q(c[0]), q(c[2]), q(c[1])];
        }
    }
    // a vertex takes the surface of the square to its south-east, the last row and column their neighbour's
    let sq = g.side - 1;
    let surface = (0..n * n)
        .map(|k| {
            let (i, j) = ((k % n * step).min(sq - 1), (k / n * step).min(sq - 1));
            g.surface[j * sq + i]
        })
        .collect();
    Grid {
        n,
        step,
        z,
        nrm,
        surface,
    }
}

impl Grid {
    fn vertex(&self, i: usize, j: usize, drop: f32) -> GroundVertex {
        let k = j * self.n + i;
        GroundVertex {
            pos: [(i * self.step) as f32, self.z[k] - drop, (j * self.step) as f32],
            nrm: self.nrm[k],
            surface: self.surface[k],
            water: 255,
            ..GroundVertex::default()
        }
    }
}

/// The area's ground as chunks of 64 m at `step` metres between vertices, row by row from the north-west.
pub fn build_chunks(g: &GroundGrid, step: u32) -> Vec<Chunk> {
    let step = step as usize;
    let gr = grid(g, step);
    let per = CHUNK_M as usize / step; // quads along a chunk's side
    let chunks = (gr.n - 1) / per;
    let mut out = Vec::with_capacity(chunks * chunks);
    for cy in 0..chunks {
        for cx in 0..chunks {
            let mut c = Chunk {
                cx: cx as u32,
                cy: cy as u32,
                ..Chunk::default()
            };
            let (i0, j0) = (cx * per, cy * per);
            for j in j0..=j0 + per {
                for i in i0..=i0 + per {
                    c.verts.push(gr.vertex(i, j, 0.0));
                }
            }
            let row = per + 1;
            for j in 0..per {
                for i in 0..per {
                    let a = (j * row + i) as u16;
                    let (b, d) = (a + 1, a + row as u16);
                    c.idx.extend_from_slice(&[a, d, b, b, d, d + 1]);
                }
            }
            // skirts on the area's outer edge (`addSkirts`)
            let mut skirt = |pts: Vec<(usize, usize)>| {
                let base = c.verts.len() as u16;
                for &(i, j) in &pts {
                    c.verts.push(gr.vertex(i, j, 0.0));
                    c.verts.push(gr.vertex(i, j, SKIRT_M));
                }
                for k in 0..pts.len() as u16 - 1 {
                    let a = base + 2 * k;
                    c.idx.extend_from_slice(&[a, a + 1, a + 2, a + 1, a + 3, a + 2]);
                }
            };
            if cy == 0 {
                skirt((i0..=i0 + per).map(|i| (i, j0)).collect());
            }
            if cy == chunks - 1 {
                skirt((i0..=i0 + per).map(|i| (i, j0 + per)).collect());
            }
            if cx == 0 {
                skirt((j0..=j0 + per).map(|j| (i0, j)).collect());
            }
            if cx == chunks - 1 {
                skirt((j0..=j0 + per).map(|j| (i0 + per, j)).collect());
            }
            out.push(c);
        }
    }
    out
}

#[cfg(test)]
mod tests;
