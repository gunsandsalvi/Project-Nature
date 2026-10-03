//! The light fields (pass 0, A11.5): each area's sun field and sky field, worked out on the CPU from its 1 m heights,
//! with no GL, so they are tested against brute-force marches (A11.13 rule 1).
//!
//! Both find each point's horizon in a direction, the steepest slope from it to the ground that way, in two parts.
//! Within `NEAR_M` it follows the point's own ray exactly: the ground along a line is known exactly from where it
//! crosses the grid's lines, between which it is a quadratic, and the top of any hump between. Beyond, it reads
//! rows turned to the direction, 1 m apart, whose ground is known the same way: from its own place and height, the
//! point takes the steepest slope to the upper convex hull of each of the two rows either side of it, weighted by
//! how near each runs. Reading from the point itself, rather than resampling horizons worked out along the rows,
//! keeps a cliff's rim from taking its face's horizons; following its own ray near it keeps a row a little uphill
//! from shading it.
//!
//! The sun field keeps each point's horizon toward the light's azimuth, as a slope `m`: the light's centre shows
//! where the light's own slope, `tan e`, is above it, and the share of the sun's disc showing ramps over the disc's
//! 0.53°, so a shadow's edge is sharp near its caster and widens with the caster's distance on its own (A11.4). It
//! depends on the light's azimuth alone, so the light's height is a uniform and the field follows the azimuth. The
//! sky field keeps the share of the sky each point's horizon leaves open: the mean over 16 directions of cos² of
//! the horizon's height.
//!
//! Until neighbours come (α02b), the ground runs on `GROUND_MARGIN_M` beyond the area's edge, each edge point's
//! height carried straight out, and beyond that lies open air, so the edges are drawn as their own inner points.
//!
//! Implements PRE-30, see A11.5 and A11.4: real shadows, sharp near their casters and softer far, and hollows darker
//! and cooler than open ground.

use kd_view::AREA_SIDE;

/// How far the light's azimuth may move before an area's sun field is worked out again: 0.1° (A11.5).
pub const SUN_FIELD_STEP_DEG: f32 = 0.1;
/// The directions the sky field looks along, evenly round (A11.5).
pub const SKY_DIRECTIONS: usize = 16;
/// The nearest ground that counts toward a horizon, in metres: nearer, the surface's own slope shades it (`n·l`),
/// and slopes over differences of heights so close would be lost to rounding.
pub const NEAREST_CASTER_M: f32 = 0.25;
/// How far the ground runs on beyond the area's edge, its edge's heights carried straight out (until α02b).
pub const GROUND_MARGIN_M: f32 = 2.0;
/// How far each point's own ray is followed exactly before the rows take over: within it a row a fraction of a
/// metre aside can lie metres higher, as across a cliff's face or a steep slope, so only the point's own ray will
/// do; beyond it such an offset tilts the horizon little.
pub const NEAR_M: f32 = 4.0;

/// Half the 16 directions, the other half their opposites: cos and sin of k × 22.5°, written out so every target
/// uses the same bits.
const HALF_ROUND: [[f32; 2]; SKY_DIRECTIONS / 2] = [
    [1.0, 0.0],
    [0.923_879_5, 0.382_683_43],
    [std::f32::consts::FRAC_1_SQRT_2, std::f32::consts::FRAC_1_SQRT_2],
    [0.382_683_43, 0.923_879_5],
    [0.0, 1.0],
    [-0.382_683_43, 0.923_879_5],
    [-std::f32::consts::FRAC_1_SQRT_2, std::f32::consts::FRAC_1_SQRT_2],
    [-0.923_879_5, 0.382_683_43],
];

const SIDE: usize = AREA_SIDE;
const EDGE: f32 = (AREA_SIDE - 1) as f32;
const MID: f32 = EDGE / 2.0;

/// An area's sun field: at each 1 m point the slope of its horizon toward the light's azimuth `toward` (east,
/// north, unit), the tangent of the highest angle at which the ground rises between it and the light.
#[derive(Clone, Debug, PartialEq)]
pub struct SunField {
    pub toward: [f32; 2],
    pub horizon: Vec<f32>,
}

/// The light's azimuth and slope from its direction (east, north, up): the unit way toward it across the ground,
/// east and north, and `tan e`; none when it stands straight overhead.
pub fn azimuth(dir: [f32; 3]) -> Option<([f32; 2], f32)> {
    let across = (dir[0] * dir[0] + dir[1] * dir[1]).sqrt();
    (across >= 1e-6).then(|| ([dir[0] / across, dir[1] / across], dir[2] / across))
}

impl SunField {
    /// Whether light from `dir` comes from an azimuth `SUN_FIELD_STEP_DEG` or more from the field's.
    pub fn stale(&self, dir: [f32; 3]) -> bool {
        let Some((t, _)) = azimuth(dir) else {
            // Overhead, no ground shades another: any field will do.
            return false;
        };
        let (cross, dot) = (
            self.toward[0] * t[1] - self.toward[1] * t[0],
            self.toward[0] * t[0] + self.toward[1] * t[1],
        );
        dot <= 0.0 || cross.abs() >= SUN_FIELD_STEP_DEG.to_radians().sin()
    }
}

/// The ground's height at `p`, metres east and south of the area's corner: bilinear between the 1 m points, and
/// beyond the area its edge's height carried straight out.
pub fn height_at(heights: &[f32], p: [f32; 2]) -> f32 {
    let (x, y) = (p[0].clamp(0.0, EDGE), p[1].clamp(0.0, EDGE));
    let (i, j) = ((x as usize).min(SIDE - 2), (y as usize).min(SIDE - 2));
    let (fx, fy) = (x - i as f32, y - j as f32);
    let at = |i: usize, j: usize| heights[j * SIDE + i];
    let north = at(i, j) + (at(i + 1, j) - at(i, j)) * fx;
    let south = at(i, j + 1) + (at(i + 1, j + 1) - at(i, j + 1)) * fx;
    north + (south - north) * fy
}

/// Whether `p` lies on the ground: the area and its margin.
pub fn on_ground(p: [f32; 2]) -> bool {
    p.iter()
        .all(|&v| (-GROUND_MARGIN_M..=EDGE + GROUND_MARGIN_M).contains(&v))
}

/// Rows across the area along the horizontal direction `a` (east, south), 1 m apart: row `k` is the line through
/// `MID + (k − half) b`, `b` a quarter turn from `a`, and `half` reaches past every corner of the ground, so each of
/// the area's points has a row either side.
struct Rows {
    a: [f32; 2],
    b: [f32; 2],
    half: i32,
}

impl Rows {
    fn new(a: [f32; 2]) -> Rows {
        let reach = (MID + GROUND_MARGIN_M) * (a[0].abs() + a[1].abs());
        Rows {
            a,
            b: [-a[1], a[0]],
            half: reach.ceil() as i32 + 1,
        }
    }

    /// How many rows.
    fn count(&self) -> usize {
        (2 * self.half + 1) as usize
    }

    /// Row `k`'s point `t` metres along it.
    fn point(&self, k: usize, t: f32) -> [f32; 2] {
        let r = k as f32 - self.half as f32;
        [MID + t * self.a[0] + r * self.b[0], MID + t * self.a[1] + r * self.b[1]]
    }

    /// The stretch of row `k` over the ground, from and to as `t`, or none where it misses.
    fn span(&self, k: usize) -> Option<(f32, f32)> {
        span_of(self.point(k, 0.0), self.a)
    }

    /// Row `k`'s ground over its whole stretch, as `line_profile` gives it.
    fn profile(&self, heights: &[f32], k: usize) -> Vec<(f32, f32)> {
        self.span(k)
            .map_or_else(Vec::new, |span| line_profile(heights, self.point(k, 0.0), self.a, span))
    }

    /// The rows each of the area's points reads: the two either side of it, each with the point's share, as row by
    /// row (t, point, share) in rising `t`.
    fn readers(&self) -> Vec<Vec<(f32, u32, f32)>> {
        let n = self.count();
        let mut out = vec![Vec::new(); n];
        for i in 0..SIDE * SIDE {
            let (dx, dy) = ((i % SIDE) as f32 - MID, (i / SIDE) as f32 - MID);
            let t = dx * self.a[0] + dy * self.a[1];
            let r = dx * self.b[0] + dy * self.b[1] + self.half as f32;
            let k = (r as usize).min(n - 2);
            let f = r - k as f32;
            out[k].push((t, i as u32, 1.0 - f));
            out[k + 1].push((t, i as u32, f));
        }
        for row in &mut out {
            row.sort_unstable_by(|p, q| p.0.total_cmp(&q.0));
        }
        out
    }

    /// Each reader's horizon over row `k`'s ground `profile` beyond `NEAR_M`, as a slope, looking toward rising `t`
    /// (`ahead`) or falling `t`, from its own place and height: `use_it(point, share, slope)` takes each, the slope
    /// none where the row has no ground that far.
    fn read(
        &self,
        heights: &[f32],
        profile: &[(f32, f32)],
        readers: &[(f32, u32, f32)],
        ahead: bool,
        mut use_it: impl FnMut(usize, f32, Option<f32>),
    ) {
        // Looking toward falling `t` is looking toward rising `−t`.
        let flip = if ahead { 1.0 } else { -1.0 };
        let ground: Vec<(f32, f32)> = if ahead {
            profile.to_vec()
        } else {
            profile.iter().rev().map(|&(t, h)| (-t, h)).collect()
        };
        // The upper convex hull of the ground beyond `NEAR_M` from the current reader, its nearest point last;
        // readers come from the far end, so the hull only grows toward them.
        let mut hull: Vec<(f32, f32)> = Vec::new();
        let mut c = ground.len();
        let order: Box<dyn Iterator<Item = &(f32, u32, f32)>> = if ahead {
            Box::new(readers.iter().rev())
        } else {
            Box::new(readers.iter())
        };
        for &(t, i, share) in order {
            let t = flip * t;
            while c > 0 && ground[c - 1].0 > t + NEAR_M {
                c -= 1;
                let p = ground[c];
                // A point on or under the line from the new point to the one beyond it is never a horizon again.
                while hull.len() >= 2 {
                    let (a, b) = (hull[hull.len() - 2], hull[hull.len() - 1]);
                    if (a.0 - p.0) * (b.1 - p.1) - (a.1 - p.1) * (b.0 - p.0) > 0.0 {
                        break;
                    }
                    hull.pop();
                }
                hull.push(p);
            }
            let i = i as usize;
            if hull.is_empty() {
                use_it(i, share, None);
                continue;
            }
            // The steepest slope from the reader to the hull, which rises to one top along it.
            let h = heights[i];
            let slope = |k: usize| (hull[k].1 - h) / (hull[k].0 - t);
            let (mut lo, mut hi) = (0, hull.len() - 1);
            while lo < hi {
                let mid = (lo + hi) / 2;
                if slope(mid) < slope(mid + 1) {
                    lo = mid + 1;
                } else {
                    hi = mid;
                }
            }
            use_it(i, share, Some(slope(lo)));
        }
    }

    /// Every point's horizon looking along `a` (`ahead`) or against it: its own ray's within `NEAR_M`, and the
    /// rows' beyond, whichever is steeper; 0, level ground running on, where neither has ground.
    fn horizons(
        &self,
        heights: &[f32],
        profiles: &[Vec<(f32, f32)>],
        readers: &[Vec<(f32, u32, f32)>],
        ahead: bool,
    ) -> Vec<f32> {
        let (mut far, mut weight) = (vec![0.0f32; SIDE * SIDE], vec![0.0f32; SIDE * SIDE]);
        for (profile, readers) in profiles.iter().zip(readers) {
            if !readers.is_empty() {
                self.read(heights, profile, readers, ahead, |i, share, m| {
                    if let Some(m) = m {
                        far[i] += share * m;
                        weight[i] += share;
                    }
                });
            }
        }
        let way = if ahead { self.a } else { [-self.a[0], -self.a[1]] };
        let near = Near::new(way);
        (0..SIDE * SIDE)
            .map(|i| {
                let far = (weight[i] > 0.0).then(|| far[i] / weight[i]);
                match (near.horizon(heights, i), far) {
                    (Some(n), Some(f)) => n.max(f),
                    (Some(m), None) | (None, Some(m)) => m,
                    (None, None) => 0.0,
                }
            })
            .collect()
    }
}

/// The stretch of the line through `o` along `a` over the ground, from and to as `t`, or none where it misses.
fn span_of(o: [f32; 2], a: [f32; 2]) -> Option<(f32, f32)> {
    let (lo_g, hi_g) = (-GROUND_MARGIN_M, EDGE + GROUND_MARGIN_M);
    let (mut lo, mut hi) = (f32::NEG_INFINITY, f32::INFINITY);
    for i in 0..2 {
        if a[i].abs() < 1e-6 {
            if !(lo_g..=hi_g).contains(&o[i]) {
                return None;
            }
        } else {
            let (t0, t1) = ((lo_g - o[i]) / a[i], (hi_g - o[i]) / a[i]);
            lo = lo.max(t0.min(t1));
            hi = hi.min(t0.max(t1));
        }
    }
    (lo <= hi).then_some((lo, hi))
}

/// The ground along the line through `o` along `a` from `t0` to `t1`, as (t, height) in rising `t`: its ends,
/// every point where it crosses one of the grid's lines, between which the ground along it is a quadratic, and the
/// top of any hump between two of them.
fn line_profile(heights: &[f32], o: [f32; 2], a: [f32; 2], (t0, t1): (f32, f32)) -> Vec<(f32, f32)> {
    let crossings = |i: usize| -> Vec<f32> {
        if a[i].abs() < 1e-6 {
            return Vec::new();
        }
        let (c0, c1) = (o[i] + t0 * a[i], o[i] + t1 * a[i]);
        let mut ts: Vec<f32> = ((c0.min(c1).ceil() as i32)..=(c0.max(c1).floor() as i32))
            .map(|g| (g as f32 - o[i]) / a[i])
            .filter(|&t| t > t0 && t < t1)
            .collect();
        if a[i] < 0.0 {
            ts.reverse();
        }
        ts
    };
    let (xs, ys) = (crossings(0), crossings(1));
    let mut ts = Vec::with_capacity(xs.len() + ys.len() + 2);
    ts.push(t0);
    let (mut i, mut j) = (0, 0);
    while i < xs.len() || j < ys.len() {
        let t = if j >= ys.len() || (i < xs.len() && xs[i] <= ys[j]) {
            i += 1;
            xs[i - 1]
        } else {
            j += 1;
            ys[j - 1]
        };
        if t > ts[ts.len() - 1] + 1e-4 {
            ts.push(t);
        }
    }
    if t1 > ts[ts.len() - 1] + 1e-4 {
        ts.push(t1);
    }
    let h = |t: f32| height_at(heights, [o[0] + t * a[0], o[1] + t * a[1]]);
    let mut out: Vec<(f32, f32)> = Vec::with_capacity(2 * ts.len());
    for (n, &t) in ts.iter().enumerate() {
        let here = h(t);
        if n > 0 {
            // Between two crossings the ground is one quadratic, so its ends and middle fix it.
            let (ta, ha) = out[out.len() - 1];
            let w = 0.5 * (t - ta);
            let tm = ta + w;
            let hm = h(tm);
            let curve = (ha - 2.0 * hm + here) / (2.0 * w * w);
            if curve < 0.0 {
                let slope = (here - ha) / (2.0 * w);
                let u = -slope / (2.0 * curve);
                if u.abs() < w {
                    out.push((tm + u, hm - slope * slope / (4.0 * curve)));
                }
            }
        }
        out.push((t, here));
    }
    out
}

/// The steepest slope from the area's point `i` to the ground along its own ray toward `a`, from
/// `NEAREST_CASTER_M` to `NEAR_M`, or none where the ground ends sooner.
fn near_horizon(heights: &[f32], i: usize, a: [f32; 2]) -> Option<f32> {
    let o = [(i % SIDE) as f32, (i / SIDE) as f32];
    let (_, end) = span_of(o, a)?;
    if end < NEAREST_CASTER_M {
        return None;
    }
    let h0 = heights[i];
    line_profile(heights, o, a, (NEAREST_CASTER_M, end.min(NEAR_M)))
        .iter()
        .map(|&(t, h)| (h - h0) / t)
        .reduce(f32::max)
}

/// One place on every near ray: how far along it lies, and the four grid points round it, as offsets from the
/// ray's start, with their bilinear weights.
#[derive(Clone, Copy, Debug)]
struct Tap {
    s: f32,
    at: [(i32, i32, f32); 4],
}

impl Tap {
    fn new(a: [f32; 2], s: f32) -> Tap {
        let (px, py) = (s * a[0], s * a[1]);
        let (ix, iy) = (px.floor(), py.floor());
        let (fx, fy) = (px - ix, py - iy);
        let (ix, iy) = (ix as i32, iy as i32);
        Tap {
            s,
            at: [
                (ix, iy, (1.0 - fx) * (1.0 - fy)),
                (ix + 1, iy, fx * (1.0 - fy)),
                (ix, iy + 1, (1.0 - fx) * fy),
                (ix + 1, iy + 1, fx * fy),
            ],
        }
    }

    /// The ground's height at this place on the ray from grid point `(x, y)`, which must lie clear of the edges.
    fn height(&self, heights: &[f32], x: i32, y: i32) -> f32 {
        self.at
            .iter()
            .map(|&(dx, dy, w)| w * heights[((y + dy) as usize) * SIDE + (x + dx) as usize])
            .sum()
    }
}

/// The near part of every ray toward `a` at once: from a grid point, a ray crosses the grid's lines at the same
/// distances wherever it starts, so its places (its ends, its crossings and the middle of each stretch between)
/// are worked out once (A11.5).
struct Near {
    a: [f32; 2],
    taps: Vec<Tap>,
    middles: Vec<Tap>,
    /// How far, in whole metres, the taps reach from the ray's start, east or west and north or south.
    reach: i32,
}

impl Near {
    fn new(a: [f32; 2]) -> Near {
        let mut ts = vec![NEAREST_CASTER_M, NEAR_M];
        for along in a.map(f32::abs).into_iter().filter(|&v| v >= 1e-6) {
            let mut g = 1;
            while g as f32 / along < NEAR_M {
                let t = g as f32 / along;
                if t > NEAREST_CASTER_M {
                    ts.push(t);
                }
                g += 1;
            }
        }
        ts.sort_by(f32::total_cmp);
        ts.dedup_by(|p, q| (*p - *q).abs() < 1e-4);
        let taps: Vec<Tap> = ts.iter().map(|&t| Tap::new(a, t)).collect();
        let middles = ts.windows(2).map(|w| Tap::new(a, 0.5 * (w[0] + w[1]))).collect();
        Near {
            a,
            taps,
            middles,
            reach: NEAR_M.ceil() as i32 + 1,
        }
    }

    /// The steepest slope from the area's point `i` to the ground along its own ray, from `NEAREST_CASTER_M` to
    /// `NEAR_M`, or none where the ground ends sooner: by the taps away from the edges, and near them by
    /// `near_horizon`, which reads the ground's margin.
    fn horizon(&self, heights: &[f32], i: usize) -> Option<f32> {
        let (x, y) = ((i % SIDE) as i32, (i / SIDE) as i32);
        let clear = |v: i32| v >= self.reach && v <= EDGE as i32 - self.reach;
        if !clear(x) || !clear(y) {
            return near_horizon(heights, i, self.a);
        }
        let h0 = heights[i];
        let mut best = f32::NEG_INFINITY;
        let mut last = (0.0, 0.0);
        for (k, tap) in self.taps.iter().enumerate() {
            let here = tap.height(heights, x, y);
            if k > 0 {
                // Between two crossings the ground is one quadratic, so its ends and middle fix it.
                let (ta, ha) = last;
                let w = 0.5 * (tap.s - ta);
                let hm = self.middles[k - 1].height(heights, x, y);
                let curve = (ha - 2.0 * hm + here) / (2.0 * w * w);
                if curve < 0.0 {
                    let slope = (here - ha) / (2.0 * w);
                    let u = -slope / (2.0 * curve);
                    if u.abs() < w {
                        best = best.max((hm - slope * slope / (4.0 * curve) - h0) / (ta + w + u));
                    }
                }
            }
            best = best.max((here - h0) / tap.s);
            last = (tap.s, here);
        }
        Some(best)
    }
}

/// An area's sun field for light from `dir` (east, north, up; A11.5): level horizons for light overhead.
pub fn sun_field(heights: &[f32], dir: [f32; 3]) -> SunField {
    assert_eq!(heights.len(), SIDE * SIDE, "an area's heights");
    let Some((toward, _)) = azimuth(dir) else {
        return SunField {
            toward: [0.0, 1.0],
            horizon: vec![0.0; SIDE * SIDE],
        };
    };
    // Rows run toward the light, east and south.
    let rows = Rows::new([toward[0], -toward[1]]);
    let profiles: Vec<_> = (0..rows.count()).map(|k| rows.profile(heights, k)).collect();
    let horizon = rows.horizons(heights, &profiles, &rows.readers(), true);
    SunField { toward, horizon }
}

/// An area's sky field (A11.5): at each 1 m point the share of the sky its horizon leaves open, from 0 to 1: the
/// mean over 16 directions of `cos²` of the horizon's height, `1 / (1 + m²)` for its slope `m` above level.
pub fn sky_field(heights: &[f32]) -> Vec<f32> {
    assert_eq!(heights.len(), SIDE * SIDE, "an area's heights");
    let mut sum = vec![0.0f32; SIDE * SIDE];
    for a in HALF_ROUND {
        let rows = Rows::new(a);
        let profiles: Vec<_> = (0..rows.count()).map(|k| rows.profile(heights, k)).collect();
        let readers = rows.readers();
        for ahead in [true, false] {
            for (s, m) in sum.iter_mut().zip(rows.horizons(heights, &profiles, &readers, ahead)) {
                *s += 1.0 / (1.0 + m.max(0.0) * m.max(0.0));
            }
        }
    }
    sum.into_iter().map(|s| s / SKY_DIRECTIONS as f32).collect()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::pixel::{SUN_TAN, sunlit};

    /// A light at elevation `e` degrees and azimuth `az` degrees east of north, as a direction (east, north, up).
    fn light(e: f32, az: f32) -> [f32; 3] {
        let (e, az) = (e.to_radians(), az.to_radians());
        [e.cos() * az.sin(), e.cos() * az.cos(), e.sin()]
    }

    /// Ground for the tests, like the demo area's: rolling hills down to a few metres across, a 30 m escarpment
    /// along a wandering line, its high side to the west, rising over 4 m, and a steep knoll east of it.
    fn test_ground() -> Vec<f32> {
        (0..SIDE * SIDE)
            .map(|i| {
                let (x, y) = ((i % SIDE) as f32, (i / SIDE) as f32);
                let hills = 4.0 * (x * 0.031).sin() * (y * 0.023).cos()
                    + 2.0 * (x * 0.11 + y * 0.07).sin()
                    + 0.6 * (x * 0.9).sin() * (y * 0.7).cos();
                let line = 120.0 + 25.0 * (y * 0.02).sin() + 8.0 * (y * 0.07).cos();
                let cliff = 30.0 * ((line - x) / 4.0 + 0.5).clamp(0.0, 1.0);
                let r = ((x - 200.0).powi(2) + (y - 90.0).powi(2)).sqrt();
                let knoll = 9.0 * (1.0 - r / 10.0).max(0.0);
                hills + cliff + knoll
            })
            .collect()
    }

    /// The steepest slope from the area's point `(x, y)` to the ground toward `a` (east, south), at least
    /// `NEAREST_CASTER_M` away, marched 1 cm a step to the ground's end, and how far off it lies; none with no
    /// ground that way.
    fn march(heights: &[f32], x: usize, y: usize, a: [f32; 2]) -> Option<(f32, f32)> {
        let h0 = heights[y * SIDE + x];
        let mut best: Option<(f32, f32)> = None;
        let mut s = NEAREST_CASTER_M + 0.005;
        loop {
            let p = [x as f32 + s * a[0], y as f32 + s * a[1]];
            if !on_ground(p) {
                return best;
            }
            let m = (height_at(heights, p) - h0) / s;
            if best.is_none_or(|(b, _)| m > b) {
                best = Some((m, s));
            }
            s += 0.01;
        }
    }

    // checks: PRE-30
    #[test]
    fn sun_field_matches_a_march() {
        // Against each point's own march toward the light, at six lights from 3° to 60° high: whether the light's
        // centre shows agrees at all but a thousandth of the points, and the horizon's angle lies within 0.05° of
        // the march at half of them and within 2° at 99 in 100. What differs lies where the light runs along the
        // cliff's face, so a row a fraction of a metre aside meets the face higher or lower than the point's own
        // ray would, far off.
        let ground = test_ground();
        for (e, az) in [
            (35.0, 250.0),
            (12.0, 290.0),
            (60.0, 135.0),
            (6.0, 90.0),
            (25.0, 3.0),
            (3.0, 275.0),
        ] {
            let dir = light(e, az);
            let f = sun_field(&ground, dir);
            let (toward, tan_e) = azimuth(dir).unwrap();
            let a = [toward[0], -toward[1]];
            let (mut angles, mut flips) = (Vec::new(), 0);
            for y in (2..SIDE - 2).step_by(3) {
                for x in (2..SIDE - 2).step_by(3) {
                    let (m, _) = march(&ground, x, y, a).unwrap_or((0.0, 1.0));
                    let got = f.horizon[y * SIDE + x];
                    angles.push((got.atan() - m.atan()).abs().to_degrees());
                    if (sunlit(got, tan_e) - sunlit(m, tan_e)).abs() > 0.5 {
                        flips += 1;
                    }
                }
            }
            angles.sort_by(f32::total_cmp);
            let at = |q: f32| angles[((angles.len() - 1) as f32 * q) as usize];
            assert!(
                flips * 1000 <= angles.len(),
                "{e}°, {az}°: {flips} of {} points lit otherwise",
                angles.len()
            );
            assert!(
                at(0.5) <= 0.05 && at(0.99) <= 2.0,
                "{e}°, {az}°: {}° and {}°",
                at(0.5),
                at(0.99)
            );
        }
    }

    // checks: PRE-30
    #[test]
    fn sky_field_matches_a_march() {
        let ground = test_ground();
        let v = sky_field(&ground);
        let mut worst = 0.0f32;
        for y in (2..SIDE - 2).step_by(6) {
            for x in (2..SIDE - 2).step_by(6) {
                let mut sum = 0.0;
                for d in 0..64 {
                    let phi = d as f32 * std::f32::consts::TAU / 64.0;
                    let m = march(&ground, x, y, [phi.cos(), phi.sin()]).map_or(0.0, |(m, _)| m.max(0.0));
                    sum += 1.0 / (1.0 + m * m);
                }
                let want = sum / 64.0;
                let err = (v[y * SIDE + x] - want).abs();
                if err > worst {
                    eprintln!("({x}, {y}): {} against {want}", v[y * SIDE + x]);
                }
                worst = worst.max(err);
            }
        }
        eprintln!("sky field: worst {worst}");
        assert!(worst <= 0.02);
    }

    // checks: PRE-30
    #[test]
    fn near_taps_match_the_ray() {
        // The near part worked out once for every point gives each point what following its own ray gives.
        let ground = test_ground();
        for a in [[1.0, 0.0], [0.6, 0.8], [-0.28, 0.96], [0.995, -0.0998]] {
            let near = Near::new(a);
            for i in (0..SIDE * SIDE).step_by(97) {
                let (fast, slow) = (near.horizon(&ground, i), near_horizon(&ground, i, a));
                assert_eq!(fast.is_some(), slow.is_some());
                if let (Some(f), Some(s)) = (fast, slow) {
                    assert!(
                        (f - s).abs() < 1e-4 * (1.0 + s.abs()),
                        "{i} along {a:?}: {f} against {s}"
                    );
                }
            }
        }
    }

    // checks: PRE-30
    #[test]
    fn open_flat_ground_sees_sun_and_sky() {
        let flat = vec![12.5; SIDE * SIDE];
        // The whole sky, everywhere, edges included.
        assert!(sky_field(&flat).iter().all(|&v| (v - 1.0).abs() < 1e-5));
        // Full sun from any light above the horizon by more than the disc's half, and none from below it.
        for (e, az) in [(0.5, 10.0), (20.0, 200.0), (89.0, 300.0), (90.0, 0.0)] {
            let dir = light(e, az);
            let f = sun_field(&flat, dir);
            let tan_e = azimuth(dir).map_or(1e6, |(_, t)| t);
            assert!(f.horizon.iter().all(|&m| sunlit(m, tan_e) == 1.0), "{e}°");
        }
        let f = sun_field(&flat, light(-2.0, 120.0));
        let tan_e = (-2.0f32).to_radians().tan();
        assert!(f.horizon.iter().all(|&m| sunlit(m, tan_e) == 0.0));
        // A pit is darker in the sky's light than open ground, and its floor darker than its rim.
        let pit: Vec<f32> = (0..SIDE * SIDE)
            .map(|i| {
                let (x, y) = ((i % SIDE) as f32 - 128.0, (i / SIDE) as f32 - 128.0);
                -6.0 * (1.0 - (x * x + y * y).sqrt() / 12.0).max(0.0)
            })
            .collect();
        let v = sky_field(&pit);
        let at = |x: usize, y: usize| v[y * SIDE + x];
        assert!(at(128, 128) < at(128, 136) && at(128, 136) < at(128, 200));
        assert!(at(128, 200) > 0.99);
        // The field follows the light's azimuth: moved under 0.1° it holds, moved more it is made again; the
        // light's height alone never stales it.
        let f = sun_field(&flat, light(30.0, 180.0));
        assert!(!f.stale(light(30.0, 180.05)) && !f.stale(light(50.0, 180.0)));
        assert!(f.stale(light(30.0, 180.2)) && f.stale(light(30.0, 0.0)));
    }

    // checks: PRE-30
    #[test]
    fn penumbra_widens_with_distance() {
        // A wall 20 m high along the area's west side, the light low in the west: the shadow on the flat ground
        // east of it ends where the wall's top hides the sun's centre, and its edge's soft band, where the sunlit
        // share goes from 0 to 1, widens with the distance from the wall as the sun's disc makes it.
        let wall: Vec<f32> = (0..SIDE * SIDE)
            .map(|i| if i % SIDE < 20 { 20.0 } else { 0.0 })
            .collect();
        let mut widths = Vec::new();
        for e in [15.0f32, 30.0] {
            let f = sun_field(&wall, light(e, 270.0));
            let tan_e = e.to_radians().tan();
            let row = 128 * SIDE;
            let share = |x: f32| {
                let (i, frac) = (x as usize, x.fract());
                sunlit(
                    f.horizon[row + i] + (f.horizon[row + i + 1] - f.horizon[row + i]) * frac,
                    tan_e,
                )
            };
            let edge = 19.0 + 20.0 / tan_e;
            assert!(share(edge - 3.0) == 0.0 && share(edge + 3.0) == 1.0, "{e}°");
            // The band's width on the ground: from the first point it begins to the first fully lit, 1 cm steps.
            let (mut x, mut start, mut width) = (edge - 3.0, None, 0.0);
            while x < 250.0 {
                let s = share(x);
                if s > 0.0 && start.is_none() {
                    start = Some(x);
                }
                if s >= 1.0 {
                    width = x - start.unwrap_or(x);
                    break;
                }
                x += 0.01;
            }
            // The disc's lower and upper limbs clear the wall's top at these distances from it.
            let half = (SUN_TAN / 2.0).atan();
            let want = 20.0 / (e.to_radians() - half).tan() - 20.0 / (e.to_radians() + half).tan();
            assert!(
                (width - want).abs() < 0.05 + 0.1 * want,
                "{e}°: {width} m against {want} m"
            );
            widths.push(width);
        }
        // The higher sun's shadow is shorter, and its edge sharper.
        assert!(widths[1] < widths[0] / 2.0, "{widths:?}");
        // Near its caster the edge is sharp: a 2 m step's shadow ramps within a few centimetres.
        let step: Vec<f32> = (0..SIDE * SIDE)
            .map(|i| if i % SIDE < 100 { 2.0 } else { 0.0 })
            .collect();
        let f = sun_field(&step, light(45.0, 270.0));
        let row = 128 * SIDE;
        assert!(sunlit(f.horizon[row + 100], 1.0) == 0.0 && sunlit(f.horizon[row + 102], 1.0) == 1.0);
    }
}
