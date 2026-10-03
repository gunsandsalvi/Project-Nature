//! The camera (A11.2, A11.5), ported from the mockup's `Renderer.zoomToTexel`, `curve`, `PITCH`, `viewFor` and
//! `computeCamera`: the zoom gives metres per art pixel, log-linear between A11.5's stops and the same in both
//! orientations; the pitch follows the art pixel's size; the view is orthographic and snapped to whole art pixels,
//! the remainder going to the upscale's shift and the dither phase, so the art grid stays locked to the ground.
//! The art grid is counted from the world's corner, and the projection is kept while the view pans within a block
//! of art pixels, the art target's viewport moving by whole pixels instead, so a pan moves the picture by whole
//! pixels exactly (A11.2's test): rounding in a moved projection would flip a few edge pixels each frame. The view
//! snaps to even art pixels, the upscale's shift spanning two, so the GPU's 2 × 2 pixel groups, over which `fwidth`
//! sets the dither band (`setBand`), always hold the same ground.
//! Implements `PRE-03`, `PRE-20` and `PRE-22` in part.

use crate::mat::Mat4;
use kd_core::geo::{self, Pos};
use kd_core::{m, num};
use kd_view::CameraPose;

/// A11.5's stops below the globe: zoom, and metres per art pixel there (person, close camp, camp, valley, region,
/// world map).
pub const STOPS: [(f32, f32); 6] = [
    (0.00, 0.03),
    (0.14, 0.13),
    (0.30, 1.1),
    (0.50, 37.0),
    (0.68, 370.0),
    (0.84, 7_600.0),
];
/// The globe's radius in metres: the world's 2,048 km round its equator (A3.7).
pub const GLOBE_R_M: f32 = 2_048_000.0 / core::f32::consts::TAU;
/// The pitch in degrees by the log of the art pixel's size (the mockup's `PITCH`): 27° at 0.03 m to straight down
/// from 20 m.
const PITCH: [(f32, f32); 7] = [
    (0.03, 27.0),
    (0.064, 33.0),
    (0.2, 38.0),
    (0.62, 48.0),
    (2.4, 66.0),
    (8.0, 86.0),
    (20.0, 90.0),
];
/// Metres added to each end of the view's depth range (A11.2).
const DEPTH_MARGIN_M: f32 = 30.0;
/// Art pixels in a block: while the view pans within one, every frame draws with the same projection and the art
/// target's viewport moves by whole pixels; crossing into the next sets the projection up again. The viewport is the
/// art target plus a block, at most 603 + 512 pixels, within OpenGL ES 3.0's least viewport size on the phone (its
/// screen's 2,404 pixels).
pub const BLOCK: i64 = 512;

/// The art target and the window: art pixels across and down, window pixels across and down, window pixels an art
/// pixel (A11.2).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ArtSize {
    pub wf: u32,
    pub hf: u32,
    pub wd: u32,
    pub hd: u32,
    pub s: u32,
}

/// Metres per art pixel at the globe stop: the globe fills 0.84 of the shorter side.
pub fn texel_max(a: ArtSize) -> f32 {
    2.0 * GLOBE_R_M / (0.84 * a.wf.min(a.hf).max(8) as f32)
}

/// Metres per art pixel at `zoom` (0 the person stop, 1 the globe), log-linear between the stops.
pub fn texel(zoom: f32, a: ArtSize) -> f32 {
    let z = num::min(num::max(zoom, 0.0), 1.0);
    let mut k = [(0.0, 0.0); 7];
    k[..6].copy_from_slice(&STOPS);
    k[6] = (1.0, texel_max(a));
    for i in 0..6 {
        if z <= k[i + 1].0 {
            let u = (z - k[i].0) / (k[i + 1].0 - k[i].0);
            let (la, lb) = (m::ln(k[i].1), m::ln(k[i + 1].1));
            return m::exp(la + (lb - la) * u);
        }
    }
    k[6].1
}

fn smooth(a: f32, b: f32, x: f32) -> f32 {
    let t = num::min(num::max((x - a) / (b - a), 0.0), 1.0);
    t * t * (3.0 - 2.0 * t)
}

/// The pitch in radians for metres per art pixel `texel` (the mockup's `curve` over `PITCH`, in the log of `texel`).
pub fn pitch(texel: f32) -> f32 {
    let lt = m::ln(texel);
    let deg = if lt <= m::ln(PITCH[0].0) {
        PITCH[0].1
    } else {
        let mut d = PITCH[PITCH.len() - 1].1;
        for i in 0..PITCH.len() - 1 {
            let (a, b) = (m::ln(PITCH[i].0), m::ln(PITCH[i + 1].0));
            if lt <= b {
                let u = smooth(a, b, lt);
                d = PITCH[i].1 + (PITCH[i + 1].1 - PITCH[i].1) * u;
                break;
            }
        }
        d
    };
    deg * core::f32::consts::PI / 180.0
}

fn cross(a: [f32; 3], b: [f32; 3]) -> [f32; 3] {
    [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ]
}

/// One frame's camera (the mockup's `computeCamera`), in GPU coordinates: metres from the floating origin, x east,
/// y up, z south.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Camera {
    pub texel: f32,
    pub pitch: f32,
    pub yaw: f32,
    /// Forward, right and up.
    pub f: [f32; 3],
    pub r: [f32; 3],
    pub u: [f32; 3],
    /// The floating origin (A11.2): the area corner nearest the ground in the middle of the view's block, so it
    /// stays put while the view pans within the block; and the target from it.
    pub origin: Pos,
    pub tg: [f32; 3],
    /// The projection, over the art target's span plus a block from the block's corner, and its depth range.
    pub vp: Mat4,
    pub near: f32,
    pub far: f32,
    /// The view's left and bottom edges, metres along right and up from the origin.
    pub left: f32,
    pub bottom: f32,
    /// The snapped view's corner in whole art pixels along right and up, counted from the world's corner; the view
    /// snaps to even art pixels.
    pub snapped: [i64; 2],
    /// The scene pass's viewport on the art target: x, y, width and height in art pixels.
    pub view: [i32; 4],
    /// Where the view's corners meet the lowest and highest ground, which the shadow camera fits (A11.2), and the
    /// block's corners, which set its depth range.
    pub foot: [[f32; 3]; 8],
    pub foot_block: [[f32; 3]; 8],
    /// The upscale's shift in art pixels (`uOff`), 0 to 3, and the dither phase (`dith`, `PRE-20`).
    pub off: [f32; 2],
    pub dith: [u32; 2],
    /// Where the target lies within its art pixel along right and up, 0 to 1.
    pub frac: [f32; 2],
}

impl Camera {
    /// The floating origin in GPU axes, metres from the world's corner: what grids fixed to the world (the art
    /// grid, the shadow map's texels) add to positions from the origin.
    pub fn origin_m(&self) -> [f64; 3] {
        [f64::from(self.origin.x) / 256.0, 0.0, f64::from(self.origin.y) / 256.0]
    }
}

/// `a` · `b` in `f64`, for positions from the world's corner.
pub fn dot64(a: [f32; 3], b: [f64; 3]) -> f64 {
    f64::from(a[0]) * b[0] + f64::from(a[1]) * b[1] + f64::from(a[2]) * b[2]
}

/// The camera for `pose` on an art target of `size`, the ground in view lying from `lo_m` to `hi_m` metres.
pub fn compute(pose: &CameraPose, size: ArtSize, lo_m: f32, hi_m: f32) -> Camera {
    let texel = texel(pose.zoom, size);
    let pitch = pitch(texel);
    let yaw = pose.yaw;
    let (cp, sp) = (m::cos(pitch), m::sin(pitch));
    let f = [-m::sin(yaw) * cp, -sp, -m::cos(yaw) * cp];
    let r = [m::cos(yaw), 0.0, -m::sin(yaw)];
    let u = cross(r, f);
    // the target on the screen's axes in art pixels from the world's corner, in f64: the art grid is fixed there
    let t64 = f64::from(texel);
    let tw = [
        f64::from(pose.target.x) / 256.0,
        f64::from(pose.target.z) / 256.0,
        f64::from(pose.target.y) / 256.0,
    ];
    let (cx, cy) = (dot64(r, tw) / t64, dot64(u, tw) / t64);
    let even = |c: f64| 2.0 * (c / 2.0).floor();
    let (sx, sy) = (even(cx) as i64, even(cy) as i64);
    let (fx, fy) = ((cx - even(cx)) as f32, (cy - even(cy)) as f32);
    // the view's block, and the origin: the area corner nearest the ground at the block's middle, at the target's
    // height
    let (bx, by) = (sx.div_euclid(BLOCK) * BLOCK, sy.div_euclid(BLOCK) * BLOCK);
    let (ma, mb) = ((bx + BLOCK / 2) as f64 * t64, (by + BLOCK / 2) as f64 * t64);
    let w = (tw[1] - mb * f64::from(u[1])) / f64::from(f[1]);
    let mid = |k: usize| ma * f64::from(r[k]) + mb * f64::from(u[k]) + w * f64::from(f[k]);
    let corner = |v: f64| (((v * 256.0).round() as i64 + 32_768) >> 16) << 16;
    let (ox, oy) = (corner(mid(0)), corner(mid(2)));
    let om = [ox as f64 / 256.0, 0.0, oy as f64 / 256.0];
    let origin = Pos {
        x: ox.rem_euclid(i64::from(geo::W)) as i32,
        y: oy.rem_euclid(i64::from(geo::H)) as i32,
        z: 0,
    };
    let tg = [(tw[0] - om[0]) as f32, tw[1] as f32, (tw[2] - om[2]) as f32];
    // the view, and the projection over the art target's span plus a block from the block's corner, in metres
    // along right and up from the origin
    // the target's column and row on the art target: the window's middle, rounded up, so the shift spans 0 to 3
    let half = |px: u32| f64::from(px) / (2.0 * f64::from(size.s));
    let (hw, hh) = (half(size.wd).ceil() as i64, half(size.hd).ceil() as i64);
    let (ro, uo) = (dot64(r, om), dot64(u, om));
    let edge = |n: i64, o: f64| (n as f64 * t64 - o) as f32;
    let (left, bottom) = (edge(sx - hw, ro), edge(sy - hh, uo));
    let (pl, pb) = (edge(bx - hw, ro), edge(by - hh, uo));
    let (pw, ph) = (size.wf + BLOCK as u32, size.hf + BLOCK as u32);
    let (pr, pt) = (pl + pw as f32 * texel, pb + ph as f32 * texel);
    // where a rectangle's corners meet the lowest and highest ground, and how far along forward
    let feet = |l: f32, rt: f32, b: f32, t: f32| {
        let (mut out, mut ts, mut k) = ([[0.0; 3]; 8], [0.0; 8], 0);
        for a in [l, rt] {
            for v in [b, t] {
                for h in [lo_m, hi_m] {
                    let o = [r[0] * a + u[0] * v, r[1] * a + u[1] * v, r[2] * a + u[2] * v];
                    let d = (h - o[1]) / f[1];
                    out[k] = [o[0] + f[0] * d, o[1] + f[1] * d, o[2] + f[2] * d];
                    ts[k] = d;
                    k += 1;
                }
            }
        }
        (out, ts)
    };
    let (foot, _) = feet(
        left,
        left + size.wf as f32 * texel,
        bottom,
        bottom + size.hf as f32 * texel,
    );
    let (foot_block, ts) = feet(pl, pr, pb, pt);
    let near = ts.iter().fold(f32::MAX, |a, &b| num::min(a, b)) - DEPTH_MARGIN_M;
    let far = ts.iter().fold(f32::MIN, |a, &b| num::max(a, b)) + DEPTH_MARGIN_M;
    let (rw, rh, rd) = (2.0 / (pr - pl), 2.0 / (pt - pb), 2.0 / (far - near));
    let rows = [
        [r[0] * rw, r[1] * rw, r[2] * rw, -pl * rw - 1.0],
        [u[0] * rh, u[1] * rh, u[2] * rh, -pb * rh - 1.0],
        [f[0] * rd, f[1] * rd, f[2] * rd, -near * rd - 1.0],
        [0.0, 0.0, 0.0, 1.0],
    ];
    let mut vp = [0.0; 16];
    for (row, v) in rows.iter().enumerate() {
        for c in 0..4 {
            vp[c * 4 + row] = v[c];
        }
    }
    let mod4 = |n: i64| n.rem_euclid(4) as u32;
    let shift = |f: f32, h: i64, px: u32| f + (h as f64 - half(px)) as f32;
    Camera {
        texel,
        pitch,
        yaw,
        f,
        r,
        u,
        origin,
        tg,
        vp: Mat4(vp),
        near,
        far,
        left,
        bottom,
        snapped: [sx - hw, sy - hh],
        view: [-((sx - bx) as i32), -((sy - by) as i32), pw as i32, ph as i32],
        foot,
        foot_block,
        off: [shift(fx, hw, size.wd), shift(fy, hh, size.hd)],
        dith: [mod4(sx - hw), mod4(sy - hh)],
        frac: [fx.fract(), fy.fract()],
    }
}

#[cfg(test)]
mod tests;
