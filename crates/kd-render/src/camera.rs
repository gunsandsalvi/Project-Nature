//! The camera (A11.2), all on the CPU (A11.13 rule 1): the art pixel's size by zoom, log-linear between A11.5's
//! stops; the pitch, one monotone cubic in the log of the art pixel through A11.2's four knots; and an orthographic
//! view of the ground, worked out in `f64` from the world's corner and handed to the shaders as small `f32` numbers
//! per area, so the world's large coordinates never reach the GPU.
//!
//! The view snaps to even art pixels counted from the world's corner, the remainder being the upscale's shift and
//! the dither's phase; within a block of 512 art pixels the projection stays put and only the viewport moves, by
//! whole pixels; the floating origin is the area corner nearest the ground in the middle of the view's block.
//!
//! Each area is projected from its own corner: the corner's place on the screen, in art pixels counted from the
//! world's corner along the screen's axes, is split into a fraction, which the vertex shader adds before rounding
//! to 1/256 of an art pixel, and whole pixels from the viewport's corner, added after. The viewport is 2,048 pixels
//! square, a power of two, so from there to the window the GPU's arithmetic is exact, and a picture moved by whole
//! pixels draws every pixel the same (A11.2).
//!
//! Implements PRE-02 and PRE-22, see A11.2: an orthographic camera whose pans move the picture by whole pixels.

use kd_core::geo::{AREA_TICKS, H, Pos, W, wrap};
use kd_view::CameraPose;

use crate::passes::scene::ArtView;

/// A11.5's zoom stops as (zoom, metres an art pixel): person, close camp, camp and valley.
pub const STOPS: [(f32, f32); 4] = [(0.00, 0.03), (0.14, 0.13), (0.30, 1.1), (0.50, 37.0)];
/// The zoom α01b reaches: the person, close camp and camp stops (the valley stop is α02a's).
pub const ZOOM_IN_REACH: [f32; 2] = [0.0, 0.34];
/// The pitch's knots as (metres an art pixel, degrees below the horizon) (A11.2).
pub const PITCH_KNOTS: [(f64, f64); 4] = [(0.03, 30.0), (0.13, 38.0), (1.1, 52.0), (37.0, 90.0)];
/// The art pixels in a block, within which the projection stays put (A11.2).
pub const BLOCK: i64 = 512;

/// Metres an art pixel spans at `zoom`, log-linear between the stops (A11.2, A11.5).
pub fn texel(zoom: f32) -> f32 {
    let z = zoom.clamp(STOPS[0].0, STOPS[3].0);
    let i = (0..3).find(|&i| z <= STOPS[i + 1].0).unwrap_or(2);
    let ((z0, a0), (z1, a1)) = (STOPS[i], STOPS[i + 1]);
    let t = f64::from((z - z0) / (z1 - z0));
    (f64::from(a0).ln() + t * (f64::from(a1).ln() - f64::from(a0).ln())).exp() as f32
}

/// The zoom whose art pixel is `texel` metres: `texel`'s inverse, held to the stops.
pub fn zoom_of(texel_m: f32) -> f32 {
    let lt = f64::from(texel_m).ln();
    let ln = |i: usize| f64::from(STOPS[i].1).ln();
    let i = (0..3).find(|&i| lt <= ln(i + 1)).unwrap_or(2);
    let t = ((lt - ln(i)) / (ln(i + 1) - ln(i))).clamp(0.0, 1.0);
    STOPS[i].0 + (t as f32) * (STOPS[i + 1].0 - STOPS[i].0)
}

/// The view's angle below the horizon, in degrees, for art pixels of `texel` metres: the one cubic in the log of
/// the art pixel through the four knots, held at its ends, so a zoom never tilts back (A11.2).
pub fn pitch_deg(texel: f64) -> f64 {
    let x: Vec<f64> = PITCH_KNOTS.iter().map(|k| k.0.ln()).collect();
    let at = texel.ln().clamp(x[0], x[3]);
    (0..4)
        .map(|i| {
            let w: f64 = (0..4)
                .filter(|&j| j != i)
                .map(|j| (at - x[j]) / (x[i] - x[j]))
                .product();
            PITCH_KNOTS[i].1 * w
        })
        .sum()
}

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
    /// The corner less the floating origin, metres east and north (`uAreaOff`): whole areas, exact in `f32`.
    pub area_off: [f32; 2],
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
    /// The floating origin: the area corner nearest the ground in the middle of the view's block (A11.2).
    pub origin: Pos,
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
    /// The view a camera pose gives on the art target `art` describes (A11.2): the art pixel from its zoom, the
    /// pitch from the art pixel.
    pub fn from_pose(pose: &CameraPose, art: &ArtView) -> View {
        let t = f64::from(texel(pose.zoom));
        View::new(pose.target, f64::from(pose.yaw), pitch_deg(t).to_radians(), t, art)
    }

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
            origin: target,
        };
        // The window's middle shows the target: the art target's corner is the even pixel a little below and left
        // of the window's corner, so the GPU's 2 × 2 groups stay on the same ground, and the upscale shifts by what
        // remains, 1 to 3 art pixels; the block is the 512 pixels holding the corner.
        let t = v.screen(v.world_m(target));
        let half = [
            f64::from(art.window[0]) / (2.0 * f64::from(art.scale)),
            f64::from(art.window[1]) / (2.0 * f64::from(art.scale)),
        ];
        for k in 0..2 {
            let low = t[k] - half[k] - 1.0;
            v.corner[k] = 2 * (low / 2.0).floor() as i64;
            v.off[k] = (t[k] - half[k] - v.corner[k] as f64) as f32;
            v.block[k] = v.corner[k].div_euclid(BLOCK) * BLOCK;
        }
        v.origin = v.block_origin(art);
        v
    }

    /// The area corner nearest the ground in the middle of the block, the ground taken at the target's height:
    /// the same wherever the window lies within the block (A11.2).
    fn block_origin(&self, art: &ArtView) -> Pos {
        let mid = [
            self.block[0] as f64 + (BLOCK as f64 + f64::from(art.art[0])) / 2.0,
            self.block[1] as f64 + (BLOCK as f64 + f64::from(art.art[1])) / 2.0,
        ];
        let z = f64::from(self.target.z) / 256.0;
        let p = self.ground_at(mid, z);
        // To ticks, the nearest area corner, back onto the world.
        let area = f64::from(AREA_TICKS);
        let x = ((p[0] * 256.0 / area).round() * area) as i64;
        let y = ((-p[1] * 256.0 / area).round() * area) as i64;
        Pos {
            x: x.rem_euclid(i64::from(W)) as i32,
            y: y.rem_euclid(i64::from(H)) as i32,
            z: self.target.z,
        }
    }

    /// The point at height `z` metres whose place on the screen is `s` (art pixels from the world's corner), in
    /// metres east and north from the world's corner.
    pub fn ground_at(&self, s: [f64; 2], z: f64) -> [f64; 2] {
        let det = self.right[0] * self.up[1] - self.right[1] * self.up[0];
        let ra = s[0] * self.texel;
        let rb = s[1] * self.texel - z * self.up[2];
        [
            (ra * self.up[1] - rb * self.right[1]) / det,
            (rb * self.right[0] - ra * self.up[0]) / det,
        ]
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
        let o = self.world_m(self.origin);
        AreaProjection {
            frac,
            px,
            depth: [(dot(m, self.fwd) - self.depth[0]) as f32, (1.0 / span) as f32],
            pattern_off: [
                corner.x.rem_euclid(block) as f32 / 256.0,
                corner.y.rem_euclid(block) as f32 / 256.0,
            ],
            area_off: [(m[0] - o[0]) as f32, (m[1] - o[1]) as f32],
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
    /// with `DEPTH_MARGIN_M` either side, over the areas the block may show (A11.2): its depth stays put while the
    /// window moves within the block.
    pub fn fit_depth(&mut self, boxes: impl Iterator<Item = (Pos, [f32; 2])>) {
        let (mut near, mut far) = (f64::MAX, f64::MIN);
        let mut block_view = *self;
        block_view.corner = self.block;
        block_view.art = [self.art[0] + BLOCK as u32, self.art[1] + BLOCK as u32];
        for (corner, span) in boxes {
            let Some(f) = block_view.local_footprint(corner, span) else {
                continue;
            };
            if f[1] < 0.0 || f[0] > 256.0 || f[3] < 0.0 || f[2] > 256.0 {
                continue;
            }
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

#[cfg(test)]
mod tests {
    use super::*;

    /// The phone's portrait screen.
    fn phone() -> ArtView {
        ArtView::new(1080, 2404, 4)
    }

    /// A target near the seam, at 21° N, where the demo area lies.
    fn target(east_m: f64, south_m: f64) -> Pos {
        Pos {
            x: 2_044_928 * 256 + (east_m * 256.0).round() as i32,
            y: 392_448 * 256 + (south_m * 256.0).round() as i32,
            z: 340 * 256,
        }
    }

    // checks: PRE-22
    #[test]
    fn snap_moves_whole_pixels() {
        let pose = |t: Pos| CameraPose {
            target: t,
            yaw: 0.7,
            zoom: 0.2,
        };
        let base = View::from_pose(&pose(target(128.0, 128.0)), &phone());
        let corner = target(0.0, 0.0);
        for k in 0..2_000 {
            let t = target(128.0 + k as f64 * 0.0371, 128.0 - k as f64 * 0.0213);
            let v = View::from_pose(&pose(t), &phone());
            // The art target's corner is an even pixel of the world's grid, the upscale's shift 1 to 3 art pixels,
            // and the window's middle shows the target.
            assert!(v.corner.iter().all(|c| c.rem_euclid(2) == 0), "{:?}", v.corner);
            assert!(v.off.iter().all(|o| (1.0..3.0).contains(o)), "{:?}", v.off);
            let mid = v.in_window(t);
            assert!(
                (mid[0] - 135.0).abs() < 1e-6 && (mid[1] - 300.5).abs() < 1e-6,
                "{mid:?}"
            );
            assert_eq!(v.dither(), [(v.corner[0] & 3) as i32, (v.corner[1] & 3) as i32]);
            // The area's projection holds still but for whole blocks: its fraction never changes, its whole pixels
            // only by the block's moves, and its place in the art target is the world grid's, so a pan moves the
            // picture by whole pixels.
            let (a, b) = (base.area(corner), v.area(corner));
            assert_eq!(a.frac, b.frac);
            for i in 0..2 {
                let moved = (base.block[i] - v.block[i]) as f32;
                assert_eq!(b.px[i], a.px[i] + moved, "pan {k}");
                let in_target = f64::from(b.px[i]) + f64::from(b.frac[i]) + f64::from(v.viewport()[i]);
                let world = v.screen(v.world_m(corner))[i] - v.corner[i] as f64;
                assert!((in_target - world).abs() < 1e-3, "{in_target} {world}");
            }
            // The viewport always covers the art target.
            let [x0, y0, w, h] = v.viewport();
            assert!(x0 <= 0 && y0 <= 0 && x0 + w >= v.art[0] as i32 && y0 + h >= v.art[1] as i32);
        }
        // A turn or a zoom changes the grid itself: only pans keep it.
        let turned = View::from_pose(
            &CameraPose {
                yaw: 0.71,
                ..pose(target(128.0, 128.0))
            },
            &phone(),
        );
        assert_ne!(turned.area(corner).frac, base.area(corner).frac);
    }

    // checks: PRE-22
    #[test]
    fn origin_stays_put_within_a_block() {
        let pose = |east: f64| CameraPose {
            target: target(east, 100.0),
            yaw: 0.0,
            zoom: 0.25,
        };
        let mut origins = Vec::new();
        let mut last_block = [i64::MIN; 2];
        for k in 0..3_000 {
            let v = View::from_pose(&pose(k as f64 * 0.25), &phone());
            // An area's corner on the world's area grid, the corners' offsets from it whole areas.
            assert_eq!((v.origin.x % AREA_TICKS, v.origin.y % AREA_TICKS), (0, 0));
            let off = v.area(target(512.0, 0.0)).area_off;
            assert!(off.iter().all(|o| o % 256.0 == 0.0), "{off:?}");
            // Within a block the origin holds; it moves only with the block, to near the block's middle ground.
            if v.block == last_block {
                assert_eq!(Some(&v.origin), origins.last());
            } else {
                origins.push(v.origin);
                last_block = v.block;
            }
            let mid = [
                v.block[0] as f64 + (BLOCK as f64 + f64::from(v.art[0])) / 2.0,
                v.block[1] as f64 + (BLOCK as f64 + f64::from(v.art[1])) / 2.0,
            ];
            let ground = v.ground_at(mid, 340.0);
            let o = v.world_m(v.origin);
            assert!((ground[0] - o[0]).abs() <= 128.0 && (ground[1] - o[1]).abs() <= 128.0);
        }
        // 750 m of pan at 0.6 m art pixels crosses a few blocks, and the origin follows.
        assert!(
            origins.len() >= 3 && origins.windows(2).all(|w| w[0] != w[1]),
            "{origins:?}"
        );
        // The depth range holds the ground with 30 m to spare, and stays put within a block.
        let mut v = View::from_pose(&pose(10.0), &phone());
        let mut w = View::from_pose(&pose(10.5), &phone());
        let area = [(target(0.0, 0.0), [0.0f32, 60.0])];
        v.fit_depth(area.into_iter());
        w.fit_depth(area.into_iter());
        assert_eq!(v.block, w.block);
        assert_eq!(v.depth, w.depth);
        let c = v.world_m(target(0.0, 0.0));
        let corner_depth = dot([c[0], c[1], c[2] + 60.0], v.fwd);
        assert!(v.depth[0] <= corner_depth - DEPTH_MARGIN_M && v.depth[1] >= corner_depth + DEPTH_MARGIN_M);
    }

    // checks: PRE-03 PRE-22
    #[test]
    fn pitch_rises_through_its_knots() {
        // Through each knot, held beyond the ends, and rising all the way, so a zoom never tilts back.
        for (t, deg) in PITCH_KNOTS {
            assert!((pitch_deg(t) - deg).abs() < 1e-9, "{t} m: {}", pitch_deg(t));
        }
        assert_eq!(pitch_deg(0.01), 30.0);
        assert!((pitch_deg(500.0) - 90.0).abs() < 1e-9);
        let mut last = pitch_deg(0.03);
        for i in 1..=2_000 {
            let t = 0.03 * (37.0f64 / 0.03).powf(f64::from(i) / 2_000.0);
            let p = pitch_deg(t);
            assert!(p > last, "{t} m: {p} after {last}");
            last = p;
        }
        // The art pixel at each stop, and log-linear between them: halfway in zoom is the geometric mean.
        for (z, t) in STOPS {
            assert!((texel(z) - t).abs() < t * 1e-5, "zoom {z}: {}", texel(z));
        }
        assert!((texel(0.07) - (0.03f32 * 0.13).sqrt()).abs() < 1e-5);
        for i in 0..=50 {
            let z = i as f32 / 100.0;
            assert!((zoom_of(texel(z)) - z).abs() < 1e-5, "zoom {z}");
        }
        assert!(
            (texel(ZOOM_IN_REACH[1]) - 2.22).abs() < 0.01,
            "{}",
            texel(ZOOM_IN_REACH[1])
        );
        let mut last = 0.0;
        for i in 0..=500 {
            let t = texel(i as f32 / 1000.0);
            assert!(t > last);
            last = t;
        }
        // The camp stop is seen from 52° above, the person stop from 30°.
        let v = View::from_pose(
            &CameraPose {
                target: target(0.0, 0.0),
                yaw: 0.0,
                zoom: 0.30,
            },
            &phone(),
        );
        assert!((v.fwd[2] + 52f64.to_radians().sin()).abs() < 1e-4);
    }
}
