//! Positions on the wrap-around world (A3.7, `WLD-01`): ticks of 1/256 m on a torus 2,048 km around and 1,024 km
//! from pole to pole, the short way between two positions across either seam, and A3.7's indices a position shifts
//! down to: the world cell, the area, the bucket and the metre in its area, and the weather cell and region a cell
//! lies in; a cell's eight neighbours, wrapped, and the seam's row.
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
/// The weather-cell grid: 200 × 100 cells of 10 × 10 world cells; the region grid: 20 × 10 regions of 100 × 100
/// world cells (A3.7).
pub const WEATHER_X: u32 = 200;
pub const WEATHER_Y: u32 = 100;
pub const REGIONS_X: u32 = 20;
pub const REGIONS_Y: u32 = 10;
/// World cells along a weather cell's side, and a region's.
pub const WEATHER_CELLS: u32 = 10;
pub const REGION_CELLS: u32 = 100;
/// From ticks to a cell's column or row, an area's, a bucket's and a metre's (A3.7: levels 10, 8, 4 and 0).
const CELL_SHIFT: u32 = 18;
const AREA_SHIFT: u32 = 16;
const BUCKET_SHIFT: u32 = 12;
const METRE_SHIFT: u32 = 8;

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

/// A bucket of 16 m in its area: `by × 16 + bx` (A3.7).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Bucket(pub u8);

/// A square metre in its area: `my × 256 + mx` (A3.7).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Metre(pub u16);

/// A weather cell, 10 × 10 world cells: `wy × 200 + wx` (A3.7).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct WeatherIx(pub u16);

/// A region, 100 × 100 world cells: `ry × 20 + rx` (A3.7).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct RegionIx(pub u8);

/// The eight neighbours' steps in columns and rows, in B10's order: counter-clockwise from east, north being a row
/// up.
pub const NEIGHBOURS8: [(i32, i32); 8] = [(1, 0), (1, -1), (0, -1), (-1, -1), (-1, 0), (-1, 1), (0, 1), (1, 1)];

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

    /// Its north-west corner at sea level.
    pub fn origin(self) -> Pos {
        let (cx, cy) = self.xy();
        Pos {
            x: (cx << CELL_SHIFT) as i32,
            y: (cy << CELL_SHIFT) as i32,
            z: 0,
        }
    }

    /// Its middle at sea level.
    pub fn centre(self) -> Pos {
        let o = self.origin();
        let half = 1 << (CELL_SHIFT - 1);
        Pos {
            x: o.x + half,
            y: o.y + half,
            z: 0,
        }
    }

    /// Its eight neighbours in B10's order, counter-clockwise from east, wrapped across both seams (A3.7).
    pub fn neighbours8(self) -> [CellIx; 8] {
        let (cx, cy) = self.xy();
        NEIGHBOURS8.map(|(dx, dy)| {
            CellIx::at(
                (cx as i32 + dx).rem_euclid(CELLS_X as i32) as u32,
                (cy as i32 + dy).rem_euclid(CELLS_Y as i32) as u32,
            )
        })
    }

    /// Whether it lies in the seam's row, the middle of the polar ice, which paths treat as blocked (A3.7).
    pub fn on_seam(self) -> bool {
        self.xy().1 == 0
    }

    /// The weather cell it lies in.
    pub fn weather(self) -> WeatherIx {
        let (cx, cy) = self.xy();
        WeatherIx::at(cx / WEATHER_CELLS, cy / WEATHER_CELLS)
    }

    /// The region it lies in.
    pub fn region(self) -> RegionIx {
        let (cx, cy) = self.xy();
        RegionIx::at(cx / REGION_CELLS, cy / REGION_CELLS)
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

impl Bucket {
    /// The bucket at column `bx` and row `by` of its area.
    pub fn at(bx: u32, by: u32) -> Bucket {
        debug_assert!(bx < 16 && by < 16, "bucket ({bx}, {by}) is off its area");
        Bucket((by * 16 + bx) as u8)
    }

    /// The bucket of its area a position lies in.
    pub fn of(p: Pos) -> Bucket {
        debug_assert!(on_world(p), "{p:?} is off the world");
        Bucket::at((p.x >> BUCKET_SHIFT) as u32 & 15, (p.y >> BUCKET_SHIFT) as u32 & 15)
    }

    /// Its column and row in its area.
    pub fn xy(self) -> (u32, u32) {
        (u32::from(self.0) % 16, u32::from(self.0) / 16)
    }

    /// Its north-west corner in `area`, at sea level.
    pub fn origin_in(self, area: AreaId) -> Pos {
        let (bx, by) = self.xy();
        let o = area.origin();
        Pos {
            x: o.x + (bx << BUCKET_SHIFT) as i32,
            y: o.y + (by << BUCKET_SHIFT) as i32,
            z: 0,
        }
    }
}

impl Metre {
    /// The square metre at column `mx` and row `my` of its area.
    pub fn at(mx: u32, my: u32) -> Metre {
        debug_assert!(mx < 256 && my < 256, "metre ({mx}, {my}) is off its area");
        Metre((my * 256 + mx) as u16)
    }

    /// The square metre of its area a position lies in.
    pub fn of(p: Pos) -> Metre {
        debug_assert!(on_world(p), "{p:?} is off the world");
        Metre::at((p.x >> METRE_SHIFT) as u32 & 255, (p.y >> METRE_SHIFT) as u32 & 255)
    }

    /// Its column and row in its area.
    pub fn xy(self) -> (u32, u32) {
        (u32::from(self.0) % 256, u32::from(self.0) / 256)
    }

    /// Its north-west corner in `area`, at sea level.
    pub fn origin_in(self, area: AreaId) -> Pos {
        let (mx, my) = self.xy();
        let o = area.origin();
        Pos {
            x: o.x + (mx << METRE_SHIFT) as i32,
            y: o.y + (my << METRE_SHIFT) as i32,
            z: 0,
        }
    }

    /// The bucket it lies in.
    pub fn bucket(self) -> Bucket {
        let (mx, my) = self.xy();
        Bucket::at(mx / 16, my / 16)
    }
}

impl WeatherIx {
    /// The weather cell at column `wx` and row `wy`.
    pub fn at(wx: u32, wy: u32) -> WeatherIx {
        debug_assert!(
            wx < WEATHER_X && wy < WEATHER_Y,
            "weather cell ({wx}, {wy}) is off the world"
        );
        WeatherIx((wy * WEATHER_X + wx) as u16)
    }

    /// The weather cell a position lies in.
    pub fn of(p: Pos) -> WeatherIx {
        CellIx::of(p).weather()
    }

    /// Its column and row.
    pub fn xy(self) -> (u32, u32) {
        (u32::from(self.0) % WEATHER_X, u32::from(self.0) / WEATHER_X)
    }

    /// Its north-west world cell.
    pub fn first_cell(self) -> CellIx {
        let (wx, wy) = self.xy();
        CellIx::at(wx * WEATHER_CELLS, wy * WEATHER_CELLS)
    }
}

impl RegionIx {
    /// The region at column `rx` and row `ry`.
    pub fn at(rx: u32, ry: u32) -> RegionIx {
        debug_assert!(rx < REGIONS_X && ry < REGIONS_Y, "region ({rx}, {ry}) is off the world");
        RegionIx((ry * REGIONS_X + rx) as u8)
    }

    /// The region a position lies in.
    pub fn of(p: Pos) -> RegionIx {
        CellIx::of(p).region()
    }

    /// Its column and row.
    pub fn xy(self) -> (u32, u32) {
        (u32::from(self.0) % REGIONS_X, u32::from(self.0) / REGIONS_X)
    }

    /// Its north-west world cell.
    pub fn first_cell(self) -> CellIx {
        let (rx, ry) = self.xy();
        CellIx::at(rx * REGION_CELLS, ry * REGION_CELLS)
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

    // checks: WLD-01 WLD-12
    #[test]
    fn areas_nest_in_cells() {
        // Every cell's 16 areas lie in that cell, and between them the cells name each of the 32 million areas
        // once (A3.7).
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
        // And every weather cell holds 10 × 10 cells, every region 100 × 100, each cell in one of each.
        let mut weather = vec![0u32; (WEATHER_X * WEATHER_Y) as usize];
        let mut regions = vec![0u32; (REGIONS_X * REGIONS_Y) as usize];
        for c in 0..CELLS_X * CELLS_Y {
            weather[usize::from(CellIx(c).weather().0)] += 1;
            regions[usize::from(CellIx(c).region().0)] += 1;
        }
        assert!(weather.iter().all(|&n| n == 100) && regions.iter().all(|&n| n == 10_000));
    }

    // checks: WLD-01
    #[test]
    fn indices_round_trip_at_corners() {
        // At the world's corners, along its seams and across it, each index a position shifts down to holds it,
        // and its corner gives the index back: the cell, the area, the bucket and the metre in the area, the
        // weather cell and the region.
        let corners = [(0, 0), (W - 1, 0), (0, H - 1), (W - 1, H - 1)];
        let mut places: Vec<Pos> = corners.iter().map(|&(x, y)| Pos { x, y, z: 7 }).collect();
        places.extend(near_seams());
        places.extend((0..10_000).map(|i| pos(6, i)));
        let within =
            |p: Pos, o: Pos, ticks: i32| (0..ticks).contains(&(p.x - o.x)) && (0..ticks).contains(&(p.y - o.y));
        for p in places {
            let (c, a) = (CellIx::of(p), AreaId::of(p));
            assert!(
                within(p, c.origin(), 1 << CELL_SHIFT) && CellIx::of(c.origin()) == c,
                "{p:?}"
            );
            assert!(
                within(p, a.origin(), AREA_TICKS) && AreaId::of(a.origin()) == a,
                "{p:?}"
            );
            assert_eq!(a.cell(), c, "{p:?}");
            assert!(c.areas().contains(&a));
            let (b, m) = (Bucket::of(p), Metre::of(p));
            assert!(
                within(p, b.origin_in(a), 1 << BUCKET_SHIFT) && Bucket::of(b.origin_in(a)) == b,
                "{p:?}"
            );
            assert!(
                within(p, m.origin_in(a), 1 << METRE_SHIFT) && Metre::of(m.origin_in(a)) == m,
                "{p:?}"
            );
            assert_eq!(m.bucket(), b, "{p:?}");
            let (w, r) = (WeatherIx::of(p), RegionIx::of(p));
            let (cx, cy) = c.xy();
            let ((wx, wy), (rx, ry)) = (w.first_cell().xy(), r.first_cell().xy());
            assert!(
                (wx..wx + WEATHER_CELLS).contains(&cx) && (wy..wy + WEATHER_CELLS).contains(&cy),
                "{p:?}"
            );
            assert!(
                (rx..rx + REGION_CELLS).contains(&cx) && (ry..ry + REGION_CELLS).contains(&cy),
                "{p:?}"
            );
            assert_eq!((w.first_cell().weather(), r.first_cell().region()), (w, r));
            // The cell's middle lies in it, half a cell from its corner.
            let mid = c.centre();
            assert_eq!(CellIx::of(mid), c);
            assert_eq!(dist(c.origin(), mid), 512.0 * std::f32::consts::SQRT_2);
        }
        // The last of each, at the south-east corner.
        let last = Pos {
            x: W - 1,
            y: H - 1,
            z: 0,
        };
        assert_eq!(AreaId::of(last), AreaId(AREAS_X * AREAS_Y - 1));
        assert_eq!(CellIx::of(last), CellIx(CELLS_X * CELLS_Y - 1));
        assert_eq!((Bucket::of(last), Metre::of(last)), (Bucket(255), Metre(65_535)));
        assert_eq!(WeatherIx::of(last), WeatherIx((WEATHER_X * WEATHER_Y - 1) as u16));
        assert_eq!(RegionIx::of(last), RegionIx((REGIONS_X * REGIONS_Y - 1) as u8));
        assert_eq!(CellIx(2_001).areas()[5], AreaId::at(5, 5));
        // A3.7's table: a cell's column is x >> 18, a weather cell's cx / 10, a region's cx / 100.
        let p = Pos {
            x: 1234 << 18,
            y: 567 << 18,
            z: 0,
        };
        assert_eq!(CellIx::of(p), CellIx(567 * 2_000 + 1234));
        assert_eq!(WeatherIx::of(p), WeatherIx(56 * 200 + 123));
        assert_eq!(RegionIx::of(p), RegionIx(5 * 20 + 12));
        // Latitude and longitude at the poles, the equator and the date line.
        assert_eq!(lat_deg(0), 90.0);
        assert_eq!(lat_deg(H / 2), 0.0);
        assert!((lat_deg(H - 1) + 90.0).abs() < 1e-4);
        assert_eq!(lon_deg(W / 2), 180.0);
    }

    // checks: WLD-01
    #[test]
    fn neighbours_wrap_both_seams() {
        // B10's order, counter-clockwise from east, north a row up; at the corners the neighbours wrap across
        // both seams; every neighbour has the cell as its opposite neighbour, and its centre lies a cell or a
        // diagonal away, the short way round.
        let corner = CellIx::at(0, 0);
        assert_eq!(
            corner.neighbours8(),
            [
                CellIx::at(1, 0),
                CellIx::at(1, 999),
                CellIx::at(0, 999),
                CellIx::at(1999, 999),
                CellIx::at(1999, 0),
                CellIx::at(1999, 1),
                CellIx::at(0, 1),
                CellIx::at(1, 1),
            ]
        );
        let cells = [(0, 0), (1999, 0), (0, 999), (1999, 999), (1000, 500), (1, 998)];
        for (cx, cy) in cells {
            let c = CellIx::at(cx, cy);
            let n = c.neighbours8();
            for (k, &m) in n.iter().enumerate() {
                assert_eq!(m.neighbours8()[(k + 4) % 8], c, "{c:?} neighbour {k}");
                let want = if k % 2 == 0 {
                    1024.0
                } else {
                    1024.0 * std::f32::consts::SQRT_2
                };
                assert!(
                    (dist(c.centre(), m.centre()) - want).abs() < 1e-3,
                    "{c:?} neighbour {k}"
                );
            }
            let mut sorted = n.to_vec();
            sorted.sort();
            sorted.dedup();
            assert_eq!(sorted.len(), 8);
        }
        // Only row 0 is the seam's.
        assert!(corner.on_seam() && CellIx::at(1999, 0).on_seam());
        assert!(!CellIx::at(0, 1).on_seam() && !CellIx::at(0, 999).on_seam());
    }
}
