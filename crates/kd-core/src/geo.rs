//! Positions on the wrap-around world (A3.7): ticks of 1/256 m, the short way across either seam, and the area a
//! position lies in. The full set of indices and neighbours comes in α02a, with the cells.
//! Implements `WLD-01` in part, see A3.7.

/// Ticks around the world, east to west: 2,000 cells of 1,024 m, 256 ticks a metre (A3.7).
pub const W: i32 = 2_000 * 1_024 * 256;
/// Ticks from pole to pole: 1,000 cells of 1,024 m.
pub const H: i32 = 1_000 * 1_024 * 256;
/// Ticks in a metre.
pub const TICKS_PER_M: i32 = 256;
/// Areas across the world, east to west (256 m each).
pub const AREAS_X: u32 = 8_000;

/// A position in ticks of 1/256 m (A3.7): `x` runs east in [0, `W`), `y` from the north pole (0) toward the south
/// pole (`H`), `z` is height above sea level.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash, PartialOrd, Ord)]
pub struct Pos {
    pub x: i32,
    pub y: i32,
    pub z: i32,
}

/// An offset, speed or local position in metres (A3.7).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Vec2 {
    pub x: f32,
    pub y: f32,
}

/// `d` taken the short way round a circle of `p` ticks: in [−p/2, p/2).
fn wrap(d: i32, p: i32) -> i32 {
    (d + p / 2).rem_euclid(p) - p / 2
}

/// The short way from `a` to `b` in metres, across either seam (A3.7).
pub fn delta(a: Pos, b: Pos) -> Vec2 {
    Vec2 {
        x: wrap(b.x - a.x, W) as f32 / 256.0,
        y: wrap(b.y - a.y, H) as f32 / 256.0,
    }
}

/// Horizontal distance in metres, the short way.
pub fn dist(a: Pos, b: Pos) -> f32 {
    let v = delta(a, b);
    (v.x * v.x + v.y * v.y).sqrt()
}

/// `p` moved by `v` metres, rounded to whole ticks and wrapped onto the world; height unchanged.
pub fn offset(p: Pos, v: Vec2) -> Pos {
    let dx = (v.x * 256.0).round() as i64;
    let dy = (v.y * 256.0).round() as i64;
    Pos {
        x: (i64::from(p.x) + dx).rem_euclid(i64::from(W)) as i32,
        y: (i64::from(p.y) + dy).rem_euclid(i64::from(H)) as i32,
        z: p.z,
    }
}

/// Latitude in degrees: 90 at the north pole (`y` = 0), −90 at the south.
pub fn lat_deg(y: i32) -> f32 {
    90.0 - 180.0 * y as f32 / H as f32
}

/// Longitude in degrees east, from 0 at `x` = 0.
pub fn lon_deg(x: i32) -> f32 {
    360.0 * x as f32 / W as f32
}

/// An area of 256 m square (A3.7, A5.3): `ay × 8,000 + ax`, with `ax = x >> 16` and `ay = y >> 16`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash, PartialOrd, Ord)]
pub struct AreaId(pub u32);

impl AreaId {
    /// The area `p` lies in.
    pub fn of(p: Pos) -> AreaId {
        let ax = (p.x >> 16) as u32;
        let ay = (p.y >> 16) as u32;
        AreaId(ay * AREAS_X + ax)
    }

    /// The area's north-west corner, at sea level.
    pub fn origin(self) -> Pos {
        Pos {
            x: ((self.0 % AREAS_X) << 16) as i32,
            y: ((self.0 / AREAS_X) << 16) as i32,
            z: 0,
        }
    }
}

#[cfg(test)]
mod tests;
