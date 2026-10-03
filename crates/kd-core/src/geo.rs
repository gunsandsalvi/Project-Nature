//! Positions on the wrap-around world (A3.7, `WLD-01`): ticks of 1/256 m on a torus 2,048 km around and 1,024 km
//! from pole to pole, the short way between two positions across either seam, and the cell and area indices a
//! position shifts down to. α02a completes A3.7's index table (buckets, metres, weather cells, regions,
//! neighbours).
//!
//! Implements WLD-01, see A3.7: the world wraps both ways, the poles along the north–south seam.

/// Ticks in a metre (A3.7).
pub const TICKS_PER_M: i32 = 256;
/// The world's width in ticks: 2,000 cells of 1,024 m (A3.7).
pub const W: i32 = 2_000 * 1_024 * 256;
/// The world's height in ticks, pole to pole: 1,000 cells of 1,024 m.
pub const H: i32 = 1_000 * 1_024 * 256;
/// The world-cell grid: 2,000 × 1,000 cells of 1,024 m.
pub const CELLS_X: u32 = 2_000;
pub const CELLS_Y: u32 = 1_000;
/// The area grid: 8,000 × 4,000 areas of 256 m, 4 × 4 to a cell.
pub const AREAS_X: u32 = 8_000;
pub const AREAS_Y: u32 = 4_000;
/// An area's side in metres, and in ticks.
pub const AREA_M: i32 = 256;
pub const AREA_TICKS: i32 = AREA_M * TICKS_PER_M;
/// From ticks to a cell's column or row, and to an area's (A3.7: level 10 and level 8).
const CELL_SHIFT: u32 = 18;
const AREA_SHIFT: u32 = 16;

/// A place in the world, in ticks of 1/256 m (A3.7): `x` east in [0, `W`), `y` from the north pole (0) to the
/// south pole (`H`), `z` the height above sea level.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub struct Pos {
    pub x: i32,
    pub y: i32,
    pub z: i32,
}

/// An offset, speed or local position in metres: x east, y south, as `Pos` counts.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Vec2 {
    pub x: f32,
    pub y: f32,
}

impl Vec2 {
    pub const fn new(x: f32, y: f32) -> Vec2 {
        Vec2 { x, y }
    }

    /// The length, as `dist` measures it.
    pub fn len(self) -> f32 {
        (self.x * self.x + self.y * self.y).sqrt()
    }
}

/// `d` wrapped into [−p/2, p/2): the short way round a loop of `p` (A3.7).
pub fn wrap(d: i32, p: i32) -> i32 {
    (d + p / 2).rem_euclid(p) - p / 2
}

/// The short way from `a` to `b` in metres, across either seam (A3.7).
pub fn delta(a: Pos, b: Pos) -> Vec2 {
    Vec2 {
        x: wrap(b.x - a.x, W) as f32 / 256.0,
        y: wrap(b.y - a.y, H) as f32 / 256.0,
    }
}

/// The horizontal distance from `a` to `b` in metres, the short way.
pub fn dist(a: Pos, b: Pos) -> f32 {
    delta(a, b).len()
}

/// `p` moved by `v` metres: `v` rounded to ticks, added, and wrapped onto the world; the height is kept.
pub fn offset(p: Pos, v: Vec2) -> Pos {
    // Any finite offset: the rounded ticks saturate at i32's ends, and the sum is taken in i64 before it wraps.
    let dx = (v.x * 256.0).round() as i32;
    let dy = (v.y * 256.0).round() as i32;
    Pos {
        x: (i64::from(p.x) + i64::from(dx)).rem_euclid(i64::from(W)) as i32,
        y: (i64::from(p.y) + i64::from(dy)).rem_euclid(i64::from(H)) as i32,
        z: p.z,
    }
}

/// The latitude of row `y` in degrees: 90 at the north pole (`y` = 0), 0 at the equator, −90 at the south pole.
pub fn lat_deg(y: i32) -> f32 {
    90.0 - 180.0 * y as f32 / H as f32
}

/// The longitude of column `x` in degrees, 0 to 360 east.
pub fn lon_deg(x: i32) -> f32 {
    360.0 * x as f32 / W as f32
}

/// A world cell, 1,024 m square: `cy × 2,000 + cx` (A3.7).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CellIx(pub u32);

/// An area, 256 m square: `ay × 8,000 + ax` (A3.7).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct AreaId(pub u32);

/// Whether a position lies on the world, as every stored position does.
fn on_world(p: Pos) -> bool {
    (0..W).contains(&p.x) && (0..H).contains(&p.y)
}

impl CellIx {
    /// The cell at column `cx` and row `cy`.
    pub fn at(cx: u32, cy: u32) -> CellIx {
        debug_assert!(cx < CELLS_X && cy < CELLS_Y, "cell ({cx}, {cy}) is off the world");
        CellIx(cy * CELLS_X + cx)
    }

    /// The cell a position lies in.
    pub fn of(p: Pos) -> CellIx {
        debug_assert!(on_world(p), "{p:?} is off the world");
        CellIx::at((p.x >> CELL_SHIFT) as u32, (p.y >> CELL_SHIFT) as u32)
    }

    /// Its column and row.
    pub fn xy(self) -> (u32, u32) {
        (self.0 % CELLS_X, self.0 / CELLS_X)
    }

    /// Its 16 areas, row by row from its north-west corner.
    pub fn areas(self) -> [AreaId; 16] {
        let (cx, cy) = self.xy();
        std::array::from_fn(|k| AreaId::at(cx * 4 + k as u32 % 4, cy * 4 + k as u32 / 4))
    }
}

impl AreaId {
    /// The area at column `ax` and row `ay`.
    pub fn at(ax: u32, ay: u32) -> AreaId {
        debug_assert!(ax < AREAS_X && ay < AREAS_Y, "area ({ax}, {ay}) is off the world");
        AreaId(ay * AREAS_X + ax)
    }

    /// The area a position lies in.
    pub fn of(p: Pos) -> AreaId {
        debug_assert!(on_world(p), "{p:?} is off the world");
        AreaId::at((p.x >> AREA_SHIFT) as u32, (p.y >> AREA_SHIFT) as u32)
    }

    /// Its column and row.
    pub fn xy(self) -> (u32, u32) {
        (self.0 % AREAS_X, self.0 / AREAS_X)
    }

    /// The cell it lies in.
    pub fn cell(self) -> CellIx {
        let (ax, ay) = self.xy();
        CellIx::at(ax / 4, ay / 4)
    }

    /// Its north-west corner at sea level.
    pub fn origin(self) -> Pos {
        let (ax, ay) = self.xy();
        Pos {
            x: (ax << AREA_SHIFT) as i32,
            y: (ay << AREA_SHIFT) as i32,
            z: 0,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::chance::{draw, stream_seed};

    /// The `i`th test position, spread over the whole world by keyed draws.
    fn pos(stream: u64, i: u64) -> Pos {
        let d = draw(stream_seed(0x0067_656f, 0, 0), i, stream);
        Pos {
            x: ((d & 0xffff_ffff) % W as u64) as i32,
            y: ((d >> 32) % H as u64) as i32,
            z: 0,
        }
    }

    /// Positions close to the seams and the world's corners, where wrapping matters most.
    fn near_seams() -> Vec<Pos> {
        let xs = [0, 1, 255, 256, W / 2 - 1, W / 2, W - 256, W - 1];
        let ys = [0, 1, 512, H / 2, H - 512, H - 1];
        xs.iter()
            .flat_map(|&x| ys.iter().map(move |&y| Pos { x, y, z: 0 }))
            .collect()
    }

    // checks: WLD-01
    #[test]
    fn delta_symmetric_across_seams() {
        // A metre either side of each seam is two metres apart the short way, both ways round.
        let west = Pos {
            x: W - 256,
            y: 1000,
            z: 0,
        };
        let east = Pos { x: 256, y: 1000, z: 0 };
        assert_eq!(delta(west, east), Vec2::new(2.0, 0.0));
        assert_eq!(delta(east, west), Vec2::new(-2.0, 0.0));
        let north = Pos { x: 5000, y: 512, z: 0 };
        let south = Pos {
            x: 5000,
            y: H - 512,
            z: 0,
        };
        assert_eq!(delta(south, north), Vec2::new(0.0, 4.0));
        assert_eq!(dist(north, south), 4.0);
        // From the corner to every other corner is one tick each way.
        let corner = Pos { x: 0, y: 0, z: 0 };
        assert_eq!(
            delta(
                corner,
                Pos {
                    x: W - 1,
                    y: H - 1,
                    z: 0
                }
            ),
            Vec2::new(-1.0 / 256.0, -1.0 / 256.0)
        );
        // Half the world away the short way is ambiguous, and wrap chooses west and north both ways.
        let half = Pos {
            x: W / 2,
            y: H / 2,
            z: 0,
        };
        assert_eq!(delta(corner, half), Vec2::new(-1_024_000.0, -512_000.0));
        assert_eq!(delta(half, corner), Vec2::new(-1_024_000.0, -512_000.0));
        // Anywhere else, the way back is exactly the way there reversed.
        let mut pairs: Vec<(Pos, Pos)> = Vec::new();
        for a in near_seams() {
            pairs.extend(near_seams().into_iter().map(|b| (a, b)));
        }
        pairs.extend((0..100_000).map(|i| (pos(0, i), pos(1, i))));
        for (a, b) in pairs {
            let (there, back) = (delta(a, b), delta(b, a));
            if wrap(b.x - a.x, W) != -W / 2 {
                assert_eq!(there.x, -back.x, "{a:?} {b:?}");
            }
            if wrap(b.y - a.y, H) != -H / 2 {
                assert_eq!(there.y, -back.y, "{a:?} {b:?}");
            }
            assert!(there.x.abs() <= 1_024_000.0 && there.y.abs() <= 512_000.0);
        }
    }

    // checks: WLD-01
    #[test]
    fn offset_then_delta_round_trips() {
        // Any offset short of half the world comes back as itself rounded to ticks: within half a tick, and
        // exactly once it is too large to hold finer than ticks.
        let starts: Vec<Pos> = near_seams().into_iter().chain((0..20_000).map(|i| pos(2, i))).collect();
        for (i, &p) in starts.iter().enumerate() {
            let d = draw(stream_seed(0x006f_6666, 0, 0), i as u64, 0);
            let unit = |bits: u64| (bits & 0xff_ffff) as f32 / 16_777_216.0 * 2.0 - 1.0;
            // Spans from a metre to most of the way round, so seams are crossed often.
            let scale = [1.0, 300.0, 70_000.0, 1_000_000.0][i % 4];
            let v = Vec2::new(unit(d) * scale, unit(d >> 24) * scale / 2.0);
            let q = offset(p, v);
            assert!((0..W).contains(&q.x) && (0..H).contains(&q.y), "{p:?} + {v:?} = {q:?}");
            let back = delta(p, q);
            for (got, want) in [(back.x, v.x), (back.y, v.y)] {
                assert!((got - want).abs() <= 1.0 / 512.0, "{p:?} + {v:?}: {back:?}");
                if want.abs() >= 65_536.0 {
                    assert_eq!(got, want, "{p:?} + {v:?}");
                }
            }
        }
        // Offsets of whole worlds land where they started; huge ones still land on the world.
        let p = Pos {
            x: 12_345,
            y: 678,
            z: 9,
        };
        assert_eq!(offset(p, Vec2::new(2_048_000.0, -1_024_000.0)), p);
        let far = offset(p, Vec2::new(1e30, -1e30));
        assert!((0..W).contains(&far.x) && (0..H).contains(&far.y));
    }

    // checks: WLD-01
    #[test]
    fn triangle_rule() {
        // A million keyed triples: no way round is shorter than the short way, within the float's rounding.
        for i in 0..1_000_000 {
            let (a, b, c) = (pos(3, i), pos(4, i), pos(5, i));
            let (ab, bc, ac) = (dist(a, b), dist(b, c), dist(a, c));
            assert!(
                ac <= (ab + bc) * (1.0 + 1e-6) + 1e-3,
                "{a:?} {b:?} {c:?}: {ac} > {ab} + {bc}"
            );
        }
        // The farthest two places can be is half the world each way.
        let far = dist(
            Pos::default(),
            Pos {
                x: W / 2,
                y: H / 2,
                z: 0,
            },
        );
        assert!((far - 1_144_866.3).abs() < 1.0, "{far}");
    }

    // checks: WLD-01
    #[test]
    fn area_cell_round_trips() {
        // Every cell's 16 areas lie in that cell, and between them the cells name each of the 32 million areas
        // once.
        let mut seen = vec![0u64; (AREAS_X * AREAS_Y).div_ceil(64) as usize];
        for c in 0..CELLS_X * CELLS_Y {
            for a in CellIx(c).areas() {
                assert_eq!(a.cell(), CellIx(c), "{a:?}");
                let (word, bit) = ((a.0 / 64) as usize, a.0 % 64);
                assert_eq!(seen[word] >> bit & 1, 0, "{a:?} named twice");
                seen[word] |= 1 << bit;
            }
        }
        assert_eq!(seen.iter().map(|w| w.count_ones()).sum::<u32>(), AREAS_X * AREAS_Y);
        // At the world's corners, and across it, a position's area and cell hold it, and an area's corner lies in
        // it.
        let corners = [(0, 0), (W - 1, 0), (0, H - 1), (W - 1, H - 1)];
        let mut places: Vec<Pos> = corners.iter().map(|&(x, y)| Pos { x, y, z: 7 }).collect();
        places.extend(near_seams());
        places.extend((0..10_000).map(|i| pos(6, i)));
        for p in places {
            let a = AreaId::of(p);
            let o = a.origin();
            assert!(
                (0..AREA_TICKS).contains(&(p.x - o.x)) && (0..AREA_TICKS).contains(&(p.y - o.y)),
                "{p:?}"
            );
            assert_eq!(AreaId::of(o), a);
            assert_eq!(a.cell(), CellIx::of(p), "{p:?}");
            assert!(CellIx::of(p).areas().contains(&a));
        }
        assert_eq!(
            AreaId::of(Pos {
                x: W - 1,
                y: H - 1,
                z: 0
            }),
            AreaId(AREAS_X * AREAS_Y - 1)
        );
        assert_eq!(
            CellIx::of(Pos {
                x: W - 1,
                y: H - 1,
                z: 0
            }),
            CellIx(CELLS_X * CELLS_Y - 1)
        );
        assert_eq!(CellIx(2_001).areas()[5], AreaId::at(5, 5));
        // Latitude and longitude at the poles, the equator and the date line.
        assert_eq!(lat_deg(0), 90.0);
        assert_eq!(lat_deg(H / 2), 0.0);
        assert!((lat_deg(H - 1) + 90.0).abs() < 1e-4);
        assert_eq!(lon_deg(W / 2), 180.0);
    }
}
