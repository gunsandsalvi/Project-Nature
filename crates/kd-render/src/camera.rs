//! The camera (A11.2), all on the CPU (A11.13 rule 1): an orthographic view of the ground, worked out in `f64` from
//! the world's corner and handed to the shaders as small `f32` numbers per area, so the world's large coordinates
//! never reach the GPU.
//!
//! Each area is projected from its own corner: the corner's place on the screen, in art pixels counted from the
//! world's corner along the screen's axes, is split into a fraction, which the vertex shader adds before rounding
//! to 1/256 of an art pixel, and whole pixels from the viewport's corner, added after. The viewport is 2,048 pixels
//! square, a power of two, so from there to the window the GPU's arithmetic is exact, and a picture moved by whole
//! pixels draws every pixel the same (A11.2).
//!
//! Implements PRE-02 and PRE-22, see A11.2: an orthographic camera whose pans move the picture by whole pixels.

use kd_core::geo::{H, Pos, W, wrap};

use crate::passes::scene::ArtView;

/// The square viewport every ground frame draws with, in art pixels: a power of two (A11.2).
pub const VIEWPORT: i32 = 2048;
/// Depth kept above the highest and below the lowest ground (A11.2).
pub const DEPTH_MARGIN_M: f64 = 30.0;
/// Patterns read positions from the corner of the world's block of this many metres that holds the area (A11.2).
pub const PATTERN_BLOCK_M: i32 = 8_192;

/// The view's directions as the shaders take them.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Basis {
    pub right: [f32; 3],
    pub up: [f32; 3],
    pub fwd: [f32; 3],
}

/// An area's numbers for the ground shader (A11.2).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AreaProjection {
    /// The area corner's place on the screen: the fraction of an art pixel, and whole art pixels from the
    /// viewport's corner.
    pub frac: [f32; 2],
    pub px: [f32; 2],
    /// The corner's depth less the near plane's, and 1 / (far − near), in metres.
    pub depth: [f32; 2],
    /// The corner within its 8,192 m block of the world, metres east and south.
    pub pattern_off: [f32; 2],
}

/// One frame's view (A11.2).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct View {
    /// Metres an art pixel spans.
    pub texel: f64,
    /// The screen's right, its up and the view's direction, in (east, north, up).
    pub right: [f64; 3],
    pub up: [f64; 3],
    pub fwd: [f64; 3],
    /// What the middle of the window shows.
    pub target: Pos,
    /// The art target's bottom-left pixel, in art pixels along the screen's right and up from the world's corner.
    pub corner: [i64; 2],
    /// The corner the frame's projection counts from, in the same art pixels: the viewport's corner.
    pub block: [i64; 2],
    /// The upscale's shift: art pixels from the art target's corner to the window's bottom-left corner.
    pub off: [f32; 2],
    /// The art target's size.
    pub art: [u32; 2],
    /// The nearest and farthest ground drawn, metres along `fwd` from the world's corner.
    pub depth: [f64; 2],
}

/// The view's directions for a heading `yaw` (radians, counter-clockwise from north) and a `pitch` below the
/// horizon (radians): the screen's right, its up and the view's direction, in (east, north, up).
pub fn basis(yaw: f64, pitch: f64) -> [[f64; 3]; 3] {
    let (sy, cy) = yaw.sin_cos();
    let (sp, cp) = pitch.sin_cos();
    let right = [cy, sy, 0.0];
    let fwd = [-sy * cp, cy * cp, -sp];
    let up = [-sy * sp, cy * sp, cp];
    [right, up, fwd]
}

fn dot(a: [f64; 3], b: [f64; 3]) -> f64 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}

impl View {
    /// The view of `target` from heading `yaw` and `pitch` below the horizon (radians), with art pixels of `texel`
    /// metres, on the art target `art` describes; its depth range is fitted to the ground by `fit_depth`.
    pub fn new(target: Pos, yaw: f64, pitch: f64, texel: f64, art: &ArtView) -> View {
        let [right, up, fwd] = basis(yaw, pitch);
        let mut v = View {
            texel,
            right,
            up,
            fwd,
            target,
            corner: [0, 0],
            block: [0, 0],
            off: [0.0, 0.0],
            art: art.art,
            depth: [0.0, 1.0],
        };
        // The window's middle shows the target: the art target's corner is the whole pixel a little below and left
        // of the window's corner, and the upscale shifts by what remains.
        let t = v.screen(v.world_m(target));
        let half = [
            f64::from(art.window[0]) / (2.0 * f64::from(art.scale)),
            f64::from(art.window[1]) / (2.0 * f64::from(art.scale)),
        ];
        for k in 0..2 {
            let low = t[k] - half[k] - 1.0;
            v.corner[k] = low.floor() as i64;
            v.off[k] = (t[k] - half[k] - v.corner[k] as f64) as f32;
        }
        v.block = v.corner;
        v
    }

    /// A position in metres (east, north, up) from the world's corner, taking the way round the world nearest the
    /// target, so the view never meets the seam.
    pub fn world_m(&self, p: Pos) -> [f64; 3] {
        let dx = f64::from(wrap(p.x - self.target.x, W)) / 256.0;
        let dy = f64::from(wrap(p.y - self.target.y, H)) / 256.0;
        [
            f64::from(self.target.x) / 256.0 + dx,
            -(f64::from(self.target.y) / 256.0 + dy),
            f64::from(p.z) / 256.0,
        ]
    }

    /// A point's place on the screen, in art pixels along the screen's right and up from the world's corner.
    pub fn screen(&self, m: [f64; 3]) -> [f64; 2] {
        [dot(m, self.right) / self.texel, dot(m, self.up) / self.texel]
    }

    /// A point's place in the window, in art pixels from its bottom-left corner.
    pub fn in_window(&self, p: Pos) -> [f64; 2] {
        let s = self.screen(self.world_m(p));
        [
            s[0] - self.corner[0] as f64 - f64::from(self.off[0]),
            s[1] - self.corner[1] as f64 - f64::from(self.off[1]),
        ]
    }

    /// The viewport, in the art target's pixels: its corner at the block's corner (A11.2).
    pub fn viewport(&self) -> [i32; 4] {
        [
            (self.block[0] - self.corner[0]) as i32,
            (self.block[1] - self.corner[1]) as i32,
            VIEWPORT,
            VIEWPORT,
        ]
    }

    /// The dither's phase: the art target's corner in the world's grid of art pixels, modulo the Bayer pattern's 4
    /// (A11.2, `PRE-20`).
    pub fn dither(&self) -> [i32; 2] {
        [(self.corner[0] & 3) as i32, (self.corner[1] & 3) as i32]
    }

    pub fn basis_f32(&self) -> Basis {
        Basis {
            right: self.right.map(|v| v as f32),
            up: self.up.map(|v| v as f32),
            fwd: self.fwd.map(|v| v as f32),
        }
    }

    /// The numbers the ground shader projects an area with, from its corner (`corner.z` the height its heights
    /// count from).
    pub fn area(&self, corner: Pos) -> AreaProjection {
        let m = self.world_m(corner);
        let s = self.screen(m);
        let mut frac = [0.0f32; 2];
        let mut px = [0.0f32; 2];
        for k in 0..2 {
            let from_block = s[k] - self.block[k] as f64;
            let whole = from_block.floor();
            frac[k] = (from_block - whole) as f32;
            px[k] = whole as f32;
        }
        let span = self.depth[1] - self.depth[0];
        let block = PATTERN_BLOCK_M * 256;
        AreaProjection {
            frac,
            px,
            depth: [(dot(m, self.fwd) - self.depth[0]) as f32, (1.0 / span) as f32],
            pattern_off: [
                corner.x.rem_euclid(block) as f32 / 256.0,
                corner.y.rem_euclid(block) as f32 / 256.0,
            ],
        }
    }

    /// The ground an area may show in the art target: its points between `span_m` metres above its corner whose
    /// place on the screen falls in the art target, as metres east (`[0]` to `[1]`) and south (`[2]` to `[3]`) of
    /// the corner; none when the view is edge-on.
    pub fn local_footprint(&self, corner: Pos, span_m: [f32; 2]) -> Option<[f64; 4]> {
        let c = self.world_m(corner);
        let det = self.right[0] * self.up[1] - self.right[1] * self.up[0];
        if det.abs() < 1e-9 {
            return None;
        }
        let (mut x0, mut x1, mut y0, mut y1) = (f64::MAX, f64::MIN, f64::MAX, f64::MIN);
        for h in span_m {
            for a in [self.corner[0], self.corner[0] + i64::from(self.art[0])] {
                for b in [self.corner[1], self.corner[1] + i64::from(self.art[1])] {
                    // The screen's (a, b) as metres east and north of the corner, at height h above it.
                    let ra = a as f64 * self.texel - dot(c, self.right);
                    let rb = b as f64 * self.texel - dot(c, self.up) - f64::from(h) * self.up[2];
                    let east = (ra * self.up[1] - rb * self.right[1]) / det;
                    let north = (rb * self.right[0] - ra * self.up[0]) / det;
                    x0 = x0.min(east);
                    x1 = x1.max(east);
                    y0 = y0.min(-north);
                    y1 = y1.max(-north);
                }
            }
        }
        Some([x0, x1, y0, y1])
    }

    /// Fits the depth range to boxes of ground, each an area's corner and its lowest and highest points above it,
    /// with `DEPTH_MARGIN_M` either side (A11.2).
    pub fn fit_depth(&mut self, boxes: impl Iterator<Item = (Pos, [f32; 2])>) {
        let (mut near, mut far) = (f64::MAX, f64::MIN);
        for (corner, span) in boxes {
            let c = self.world_m(corner);
            for x in [0.0, 256.0] {
                for y in [0.0, -256.0] {
                    for h in span {
                        let d = dot([c[0] + x, c[1] + y, c[2] + f64::from(h)], self.fwd);
                        near = near.min(d);
                        far = far.max(d);
                    }
                }
            }
        }
        if near <= far {
            self.depth = [near - DEPTH_MARGIN_M, far + DEPTH_MARGIN_M];
        }
    }
}
