//! B10: square cells on the torus (WLD-01, WLD-03). Cell (x, y): x runs east, y runs from the
//! north pole (y = 0) to the south pole (y = h); both edges wrap. Row-major index y*w + x.
//! A quadtree level k cell (X, Y) owns the fine cells (2X..2X+1, 2Y..2Y+1).

pub const SQRT2: f64 = std::f64::consts::SQRT_2;

/// The eight neighbour steps, counter-clockwise from east.
pub const DIRS8: [(i64, i64); 8] = [
    (1, 0),
    (1, 1),
    (0, 1),
    (-1, 1),
    (-1, 0),
    (-1, -1),
    (0, -1),
    (1, -1),
];

#[derive(Clone, Copy, Debug)]
pub struct SqTorus {
    pub w: i64,
    pub h: i64,
}

impl SqTorus {
    pub fn n(&self) -> usize {
        (self.w * self.h) as usize
    }
    #[inline]
    pub fn index(&self, x: i64, y: i64) -> usize {
        (y.rem_euclid(self.h) * self.w + x.rem_euclid(self.w)) as usize
    }
    #[inline]
    pub fn cell(&self, i: usize) -> (i64, i64) {
        ((i as i64) % self.w, (i as i64) / self.w)
    }
    pub fn neighbours(&self, i: usize) -> [usize; 8] {
        let (x, y) = self.cell(i);
        let mut out = [0; 8];
        for (k, (dx, dy)) in DIRS8.iter().enumerate() {
            out[k] = self.index(x + dx, y + dy);
        }
        out
    }
    /// 8-neighbour path length between two cells, shortest image across both seams.
    pub fn octile(&self, i: usize, j: usize) -> f64 {
        let (ax, ay) = self.cell(i);
        let (bx, by) = self.cell(j);
        octile(wrap_abs(bx - ax, self.w) as f64, wrap_abs(by - ay, self.h) as f64)
    }
}

/// |d| on a circle of length p, the short way round.
pub fn wrap_abs(d: i64, p: i64) -> i64 {
    let m = d.rem_euclid(p);
    m.min(p - m)
}

pub fn wrap_absf(d: f64, p: f64) -> f64 {
    let m = d.rem_euclid(p);
    m.min(p - m)
}

pub fn octile(dx: f64, dy: f64) -> f64 {
    let (dx, dy) = (dx.abs(), dy.abs());
    let (lo, hi) = if dx < dy { (dx, dy) } else { (dy, dx) };
    hi + (SQRT2 - 1.0) * lo
}
