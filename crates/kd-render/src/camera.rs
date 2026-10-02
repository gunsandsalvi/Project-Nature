//! The camera (A11.2, A11.5), ported from the mockup's `Renderer.zoomToTexel`, `curve`, `PITCH`, `viewFor` and
//! `computeCamera`: the zoom gives metres per art pixel, log-linear between A11.5's stops and the same in both
//! orientations; the pitch follows the art pixel's size; the view is orthographic and snapped to whole art pixels,
//! the remainder going to the upscale's shift and the dither phase, so the art grid stays locked to the ground.
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

fn dot(a: [f32; 3], b: [f32; 3]) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
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
    /// The floating origin: the area corner nearest the target (A11.2).
    pub origin: Pos,
    pub vp: Mat4,
    pub near: f32,
    pub far: f32,
    pub left: f32,
    pub bottom: f32,
    /// The snapped view's corner in whole art pixels along right and up.
    pub snapped: [i64; 2],
    /// Where the view's corners meet the lowest and highest ground: the shadow camera fits them (A11.2).
    pub foot: [[f32; 3]; 8],
    /// The upscale's shift in art pixels (`uOff`) and the dither phase (`dith`, `PRE-20`).
    pub off: [f32; 2],
    pub dith: [u32; 2],
}

/// The area corner nearest `p`, at sea level (A11.2's floating origin).
pub fn floating_origin(p: Pos) -> Pos {
    let near = |v: i32, period: i32| ((i64::from(v) + 32_768) >> 16 << 16).rem_euclid(i64::from(period)) as i32;
    Pos {
        x: near(p.x, geo::W),
        y: near(p.y, geo::H),
        z: 0,
    }
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
    let origin = floating_origin(pose.target);
    let d = geo::delta(origin, pose.target);
    let tg = [d.x, pose.target.z as f32 / 256.0, d.y];
    let (cx, cy) = (dot(r, tg) / texel, dot(u, tg) / texel);
    let (sx, sy) = (cx.floor(), cy.floor());
    let (fx, fy) = (cx - sx, cy - sy);
    let (hw, hh) = ((size.wf / 2) as f32, (size.hf / 2) as f32);
    let left = (sx - hw) * texel;
    let right = left + size.wf as f32 * texel;
    let bottom = (sy - hh) * texel;
    let top = bottom + size.hf as f32 * texel;
    let (mut near, mut far) = (f32::MAX, f32::MIN);
    let mut foot = [[0.0; 3]; 8];
    let mut k = 0;
    for a in [left, right] {
        for b in [bottom, top] {
            for h in [lo_m, hi_m] {
                let o = [r[0] * a + u[0] * b, r[1] * a + u[1] * b, r[2] * a + u[2] * b];
                let t = (h - o[1]) / f[1];
                near = num::min(near, t);
                far = num::max(far, t);
                foot[k] = [o[0] + f[0] * t, o[1] + f[1] * t, o[2] + f[2] * t];
                k += 1;
            }
        }
    }
    near -= DEPTH_MARGIN_M;
    far += DEPTH_MARGIN_M;
    let (rw, rh, rd) = (2.0 / (right - left), 2.0 / (top - bottom), 2.0 / (far - near));
    let rows = [
        [r[0] * rw, r[1] * rw, r[2] * rw, -left * rw - 1.0],
        [u[0] * rh, u[1] * rh, u[2] * rh, -bottom * rh - 1.0],
        [f[0] * rd, f[1] * rd, f[2] * rd, -near * rd - 1.0],
        [0.0, 0.0, 0.0, 1.0],
    ];
    let mut vp = [0.0; 16];
    for (row, v) in rows.iter().enumerate() {
        for c in 0..4 {
            vp[c * 4 + row] = v[c];
        }
    }
    let (sxi, syi) = (sx as i64, sy as i64);
    let mod4 = |n: i64| n.rem_euclid(4) as u32;
    let half = |px: u32| px as f32 / (2.0 * size.s as f32);
    Camera {
        texel,
        pitch,
        yaw,
        f,
        r,
        u,
        origin,
        vp: Mat4(vp),
        near,
        far,
        left,
        bottom,
        snapped: [sxi - i64::from(size.wf / 2), syi - i64::from(size.hf / 2)],
        foot,
        off: [fx + hw - half(size.wd), fy + hh - half(size.hd)],
        dith: [mod4(sxi - i64::from(size.wf / 2)), mod4(syi - i64::from(size.hf / 2))],
    }
}

#[cfg(test)]
mod tests;
