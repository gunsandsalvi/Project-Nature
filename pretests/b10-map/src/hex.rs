//! B10: hexagonal cells as Eisenstein integers, on the plane and on the torus (WLD-01, WLD-03).
//!
//! Axial coordinates (q, r) on the basis {1, w}, w = e^(i*pi/3).
//! Centre of (q, r), in units of the cell spacing: x = q + r/2, y = r*sqrt(3)/2.
//! A hierarchy level is the lattice G*Z[w], where G is the product of the generators so far.

pub const SQRT3: f64 = 1.732_050_807_568_877_2;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct E {
    pub q: i64,
    pub r: i64,
}

impl E {
    pub const fn new(q: i64, r: i64) -> E {
        E { q, r }
    }
    pub fn add(self, o: E) -> E {
        E::new(self.q + o.q, self.r + o.r)
    }
    pub fn sub(self, o: E) -> E {
        E::new(self.q - o.q, self.r - o.r)
    }
    /// (a + b w)(c + d w) with w^2 = w - 1.
    pub fn mul(self, o: E) -> E {
        E::new(self.q * o.q - self.r * o.r, self.q * o.r + self.r * o.q + self.r * o.r)
    }
    /// Complex conjugate: conj(w) = 1 - w.
    pub fn conj(self) -> E {
        E::new(self.q + self.r, -self.r)
    }
    pub fn norm(self) -> i64 {
        self.q * self.q + self.q * self.r + self.r * self.r
    }
    pub fn div_exact(self, g: E) -> Option<E> {
        let n = g.norm();
        let p = self.mul(g.conj());
        if p.q % n == 0 && p.r % n == 0 {
            Some(E::new(p.q / n, p.r / n))
        } else {
            None
        }
    }
    /// Steps between neighbours (6-neighbour hex distance).
    pub fn steps(self) -> i64 {
        (self.q.abs() + self.r.abs() + (self.q + self.r).abs()) / 2
    }
    pub fn xy(self) -> (f64, f64) {
        (self.q as f64 + 0.5 * self.r as f64, 0.5 * SQRT3 * self.r as f64)
    }
}

/// The six neighbour steps, counter-clockwise from east.
pub const DIRS: [E; 6] = [
    E::new(1, 0),
    E::new(0, 1),
    E::new(-1, 1),
    E::new(-1, 0),
    E::new(0, -1),
    E::new(1, -1),
];

/// Nearest lattice point to fractional axial coordinates (cube rounding).
#[inline]
pub fn round_axial(qf: f64, rf: f64) -> E {
    let (x, z) = (qf, rf);
    let y = -x - z;
    let (mut rx, ry, mut rz) = (x.round(), y.round(), z.round());
    let (dx, dy, dz) = ((rx - x).abs(), (ry - y).abs(), (rz - z).abs());
    if dx > dy && dx > dz {
        rx = -ry - rz;
    } else if dy > dz {
        // only the third cube coordinate changes; q and r stay
    } else {
        rz = -rx - ry;
    }
    E::new(rx as i64, rz as i64)
}

#[inline]
pub fn xy_to_axial(x: f64, y: f64) -> (f64, f64) {
    let r = 2.0 * y / SQRT3;
    (x - 0.5 * r, r)
}

/// The cell (spacing 1) that contains point (x, y).
#[inline]
pub fn cell_at(x: f64, y: f64) -> E {
    let (q, r) = xy_to_axial(x, y);
    round_axial(q, r)
}

/// Point (x, y) divided by the Eisenstein integer g, as fractional axial coordinates.
pub fn point_div(x: f64, y: f64, g: E) -> (f64, f64) {
    let (q, r) = xy_to_axial(x, y);
    let c = g.conj();
    let n = g.norm() as f64;
    let (cq, cr) = (c.q as f64, c.r as f64);
    ((q * cq - r * cr) / n, (q * cr + r * cq + r * cr) / n)
}

/// Standard hex hierarchies.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum Kind {
    /// Aperture 7, turning about 19.1 degrees and back on alternate levels (as in H3).
    A7Alt,
    /// Aperture 7, turning the same way every level (as in Generalised Balanced Ternary).
    A7Fixed,
    /// Aperture 4, same orientation every level.
    A4,
    /// Aperture 3, turning 30 degrees each level.
    A3,
}

impl Kind {
    pub fn name(self) -> &'static str {
        match self {
            Kind::A7Alt => "hex aperture 7, alternating (H3-like)",
            Kind::A7Fixed => "hex aperture 7, fixed turn (GBT-like)",
            Kind::A4 => "hex aperture 4",
            Kind::A3 => "hex aperture 3",
        }
    }
    /// Generator from level `l` (finer) to level `l + 1` (coarser).
    pub fn gen(self, l: usize) -> E {
        match self {
            Kind::A7Alt => {
                if l % 2 == 0 {
                    E::new(2, 1)
                } else {
                    E::new(3, -1)
                }
            }
            Kind::A7Fixed => E::new(2, 1),
            Kind::A4 => E::new(2, 0),
            Kind::A3 => {
                if l % 2 == 0 {
                    E::new(1, 1)
                } else {
                    E::new(2, -1)
                }
            }
        }
    }
    /// Product of the generators of the first `k` levels.
    pub fn cum(self, k: usize) -> E {
        let mut g = E::new(1, 0);
        for l in 0..k {
            g = g.mul(self.gen(l));
        }
        g
    }
    pub fn aperture(self) -> i64 {
        self.gen(0).norm()
    }
    /// Logical parent: the level-(l+1) cell that owns level-l cell z. Every cell has exactly one.
    #[inline]
    pub fn parent(self, l: usize, z: E) -> E {
        match self {
            Kind::A4 => {
                // Centre child plus three edge children in alternating directions
                // (east, south-west, north-west), so the parent shape stays balanced.
                let (q, r) = (z.q, z.r);
                match (q.rem_euclid(2), r.rem_euclid(2)) {
                    (0, 0) => E::new(q / 2, r / 2),
                    (1, 0) => E::new((q - 1).div_euclid(2), r.div_euclid(2)),
                    (0, 1) => E::new(q.div_euclid(2), (r + 1).div_euclid(2)),
                    _ => E::new((q + 1).div_euclid(2), (r - 1).div_euclid(2)),
                }
            }
            _ => {
                let g = self.gen(l);
                let p = z.mul(g.conj());
                let n = g.norm() as f64;
                // A tiny fixed offset breaks the exact ties of aperture 3 the same way everywhere.
                round_axial(p.q as f64 / n + 1.3e-7, p.r as f64 / n + 3.1e-7)
            }
        }
    }
}

pub fn ext_gcd(a: i64, b: i64) -> (i64, i64, i64) {
    if b == 0 {
        if a < 0 {
            (-a, -1, 0)
        } else {
            (a, 1, 0)
        }
    } else {
        let (g, x, y) = ext_gcd(b, a.rem_euclid(b));
        // g = b*x + (a mod b)*y = b*x + (a - floor(a/b)*b)*y
        (g, y, x - a.div_euclid(b) * y)
    }
}

/// One hex level wrapped on a torus. The period lattice, in this level's coordinates, is kept
/// in Hermite normal form: basis (a, 0) and (b, d), 0 <= b < a. Storage is row-major, d rows
/// of a cells, which is a sheared ("axial") layout of the rectangular torus.
#[derive(Clone, Copy, Debug)]
pub struct HexTorus {
    pub a: i64,
    pub b: i64,
    pub d: i64,
}

impl HexTorus {
    pub fn from_periods(u1: E, u2: E) -> HexTorus {
        let (g, x, y) = ext_gcd(u1.r, u2.r);
        let mut w = E::new(x * u1.q + y * u2.q, x * u1.r + y * u2.r);
        if w.r < 0 {
            w = E::new(-w.q, -w.r);
        }
        assert_eq!(w.r, g);
        let det = (u1.q * u2.r - u1.r * u2.q).abs();
        let a = det / g;
        HexTorus { a, b: w.q.rem_euclid(a), d: g }
    }
    /// Fine level of a torus with `n` cells east-west and `2m` rows pole to pole.
    pub fn fine(n: i64, m: i64) -> HexTorus {
        HexTorus::from_periods(E::new(n, 0), E::new(-m, 2 * m))
    }
    /// Level k of hierarchy `kind` on the torus whose fine level is (n, 2m); None if the
    /// torus periods are not whole vectors of that level's lattice.
    pub fn level(n: i64, m: i64, kind: Kind, k: usize) -> Option<HexTorus> {
        let g = kind.cum(k);
        let u1 = E::new(n, 0).div_exact(g)?;
        let u2 = E::new(-m, 2 * m).div_exact(g)?;
        Some(HexTorus::from_periods(u1, u2))
    }
    pub fn n(&self) -> usize {
        (self.a * self.d) as usize
    }
    #[inline]
    pub fn canon(&self, z: E) -> E {
        let k = z.r.div_euclid(self.d);
        let r = z.r - k * self.d;
        let q = (z.q - k * self.b).rem_euclid(self.a);
        E::new(q, r)
    }
    #[inline]
    pub fn index(&self, z: E) -> usize {
        let c = self.canon(z);
        (c.r * self.a + c.q) as usize
    }
    #[inline]
    pub fn cell(&self, i: usize) -> E {
        E::new(i as i64 % self.a, i as i64 / self.a)
    }
    pub fn neighbours(&self, i: usize) -> [usize; 6] {
        let z = self.cell(i);
        let mut out = [0; 6];
        for (k, d) in DIRS.iter().enumerate() {
            out[k] = self.index(z.add(*d));
        }
        out
    }
    /// Steps between two cells on the torus: the shortest over the nearby images.
    pub fn steps(&self, i: usize, j: usize) -> i64 {
        let dz = self.cell(j).sub(self.cell(i));
        let p1 = E::new(self.a, 0);
        let p2 = E::new(self.b, self.d);
        let mut best = i64::MAX;
        for s in -2..=2i64 {
            for t in -2..=2i64 {
                let c = E::new(dz.q + s * p1.q + t * p2.q, dz.r + s * p1.r + t * p2.r);
                best = best.min(c.steps());
            }
        }
        best
    }
}
