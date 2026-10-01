//! B10 checks: wrapping across both seams (WLD-01) and how cells nest across scales (WLD-12).

use crate::hex::{self, HexTorus, Kind, E};
use crate::path::Search;
use crate::rng::Rng;
use crate::sq::{self, SqTorus, SQRT2};

fn report(what: &str, failures: u64) {
    let tag = if failures == 0 { "PASS" } else { "FAIL" };
    println!("  [{tag}] {what} (failures: {failures})");
}

/// World tori at the 1 km level (WLD-03): squares 2048 x 1024 (cell 0.977 km), hexes 1862 x 1078
/// rows (spacing 1.074 km). 1862 and 539 (= half the rows) are multiples of 49, so four aperture-7
/// levels wrap exactly.
pub const SQ_WORLD: SqTorus = SqTorus { w: 2048, h: 1024 };
pub const HEX_N: i64 = 1862;
pub const HEX_M: i64 = 539;

pub fn run() {
    println!("== B10 wrap checks (WLD-01) ==");

    // 1. Neighbours on the full 1 km world: distinct, never the cell itself, always mutual.
    let s = SQ_WORLD;
    let mut bad = 0u64;
    for i in 0..s.n() {
        let nb = s.neighbours(i);
        for a in 0..8 {
            if nb[a] == i {
                bad += 1;
            }
            for b in a + 1..8 {
                if nb[a] == nb[b] {
                    bad += 1;
                }
            }
            if !s.neighbours(nb[a]).contains(&i) {
                bad += 1;
            }
        }
    }
    report("squares 2048x1024: 8 distinct, mutual neighbours for every cell", bad);
    let h = HexTorus::fine(HEX_N, HEX_M);
    let mut bad = 0u64;
    for i in 0..h.n() {
        let nb = h.neighbours(i);
        for a in 0..6 {
            if nb[a] == i {
                bad += 1;
            }
            for b in a + 1..6 {
                if nb[a] == nb[b] {
                    bad += 1;
                }
            }
            if !h.neighbours(nb[a]).contains(&i) {
                bad += 1;
            }
        }
    }
    report("hexes 1862x1078: 6 distinct, mutual neighbours for every cell", bad);

    // Seam spot checks: the cell east of the east edge is the west edge cell, and so on.
    let mut bad = 0u64;
    bad += (s.neighbours(s.index(2047, 500))[0] != s.index(0, 500)) as u64;
    bad += (s.neighbours(s.index(700, 1023))[2] != s.index(700, 0)) as u64;
    bad += (s.neighbours(s.index(700, 0))[6] != s.index(700, 1023)) as u64;
    bad += (s.neighbours(s.index(0, 0))[5] != s.index(2047, 1023)) as u64;
    let east_edge = h.index(E::new(HEX_N - 1, 300));
    bad += (h.neighbours(east_edge)[0] != h.index(E::new(0, 300))) as u64;
    report("seam spot checks (east edge to west edge, pole row to pole row, corner)", bad);

    // 2. Path search over the wrapped grid equals the wrap-aware distance formula, from sources
    //    on both seams and corners, to every cell (small tori so it runs in seconds).
    let st = SqTorus { w: 64, h: 32 };
    let mut search = Search::new(st.n());
    let neigh_sq = |i: usize, out: &mut Vec<(usize, f64)>| {
        for (k, j) in st.neighbours(i).iter().enumerate() {
            out.push((*j, if k % 2 == 1 { SQRT2 } else { 1.0 }));
        }
    };
    let sources = [(0, 0), (63, 0), (0, 31), (63, 31), (32, 0), (0, 16), (63, 16), (32, 31), (17, 9)];
    let mut bad = 0u64;
    let mut maxdiff: f64 = 0.0;
    for &(x, y) in &sources {
        let src = st.index(x, y);
        let d = search.all(src, &neigh_sq);
        for j in 0..st.n() {
            let diff = (d[j] - st.octile(src, j)).abs();
            maxdiff = maxdiff.max(diff);
            bad += (diff > 1e-9) as u64;
        }
    }
    report(&format!("squares 64x32: search = formula across both seams, max diff {maxdiff:.1e}"), bad);

    let ht = HexTorus::fine(60, 17);
    let mut hsearch = Search::new(ht.n());
    let neigh_hex = |i: usize, out: &mut Vec<(usize, f64)>| {
        for j in ht.neighbours(i) {
            out.push((j, 1.0));
        }
    };
    let mut bad = 0u64;
    let hsources = [
        E::new(0, 0),
        E::new(59, 0),
        E::new(0, 33),
        E::new(59, 33),
        E::new(30, 0),
        E::new(0, 17),
        E::new(30, 33),
        E::new(11, 7),
    ];
    for z in hsources {
        let src = ht.index(z);
        let d = hsearch.all(src, &neigh_hex);
        for j in 0..ht.n() {
            bad += (d[j] != ht.steps(src, j) as f64) as u64;
        }
    }
    report("hexes 60x34: search = formula across both seams", bad);

    // 3. The polar ice cap: block 3 rows each side of the north-south seam. Travel must never
    //    cross it, so search distances must equal the formula with east-west wrap only.
    let cap = 3i64;
    let in_cap_sq = |i: usize| {
        let (_, y) = st.cell(i);
        y < cap || y >= st.h - cap
    };
    let neigh_sq_cap = |i: usize, out: &mut Vec<(usize, f64)>| {
        for (k, j) in st.neighbours(i).iter().enumerate() {
            if !in_cap_sq(*j) {
                out.push((*j, if k % 2 == 1 { SQRT2 } else { 1.0 }));
            }
        }
    };
    let mut bad = 0u64;
    for &(x, y) in &[(0i64, 3i64), (63, 3), (10, 28), (63, 28), (40, 16)] {
        let src = st.index(x, y);
        let d = search.all(src, &neigh_sq_cap);
        for j in 0..st.n() {
            if in_cap_sq(j) {
                bad += d[j].is_finite() as u64;
                continue;
            }
            let (jx, jy) = st.cell(j);
            let f = sq::octile(sq::wrap_abs(jx - x, st.w) as f64, (jy - y).abs() as f64);
            bad += ((d[j] - f).abs() > 1e-9) as u64;
        }
    }
    report("squares: with the ice cap blocked, no path crosses the north-south seam", bad);
    let a = st.index(5, 3);
    let b = st.index(5, 28);
    let travel = search.all(a, &neigh_sq_cap)[b];
    println!(
        "         example: just south of the north cap to just north of the south cap: maths distance {} cells (across the seam), travel {} cells (round by the equator)",
        st.octile(a, b),
        travel
    );

    let in_cap_hex = |i: usize| {
        let z = ht.cell(i);
        z.r < cap || z.r >= ht.d - cap
    };
    let neigh_hex_cap = |i: usize, out: &mut Vec<(usize, f64)>| {
        for j in ht.neighbours(i) {
            if !in_cap_hex(j) {
                out.push((j, 1.0));
            }
        }
    };
    let mut bad = 0u64;
    for z in [E::new(0, 3), E::new(59, 3), E::new(10, 30), E::new(59, 30), E::new(40, 17)] {
        let src = ht.index(z);
        let d = hsearch.all(src, &neigh_hex_cap);
        for j in 0..ht.n() {
            if in_cap_hex(j) {
                bad += d[j].is_finite() as u64;
                continue;
            }
            let dz = ht.cell(j).sub(ht.cell(src));
            let f = (-2..=2).map(|k| E::new(dz.q + k * ht.a, dz.r).steps()).min().unwrap();
            bad += (d[j] != f as f64) as u64;
        }
    }
    report("hexes: with the ice cap blocked, no path crosses the north-south seam", bad);

    // 4. Continuous distance on the 2000 x 1000 km torus: symmetric, triangle inequality, seams.
    let (w, hh) = (2000.0, 1000.0);
    let dist = |p: (f64, f64), q: (f64, f64)| {
        let dx = sq::wrap_absf(q.0 - p.0, w);
        let dy = sq::wrap_absf(q.1 - p.1, hh);
        (dx * dx + dy * dy).sqrt()
    };
    let mut rng = Rng::new(4);
    let mut bad = 0u64;
    for _ in 0..1_000_000 {
        let p = (rng.range(0.0, w), rng.range(0.0, hh));
        let q = (rng.range(0.0, w), rng.range(0.0, hh));
        let r = (rng.range(0.0, w), rng.range(0.0, hh));
        bad += ((dist(p, q) - dist(q, p)).abs() > 1e-9) as u64;
        bad += (dist(p, r) > dist(p, q) + dist(q, r) + 1e-9) as u64;
        bad += (dist(p, (p.0 + w, p.1 - hh)) > 1e-9) as u64;
    }
    bad += ((dist((1.0, 500.0), (1999.0, 500.0)) - 2.0).abs() > 1e-9) as u64;
    bad += ((dist((500.0, 1.0), (500.0, 999.0)) - 2.0).abs() > 1e-9) as u64;
    report("km distance: symmetric, triangle inequality, 2 km across each seam (1M triples)", bad);

    // 5. Nesting on the torus: every coarse cell has exactly its share of children, also across
    //    both seams, and rolled-up totals equal fine totals.
    let mut bad = 0u64;
    let mut field: Vec<f64> = (0..s.n()).map(|_| rng.f64()).collect();
    let total: f64 = field.iter().sum();
    let (mut w0, mut h0) = (s.w as usize, s.h as usize);
    for _level in 1..=6 {
        let (w1, h1) = (w0 / 2, h0 / 2);
        let mut count = vec![0u32; w1 * h1];
        let mut next = vec![0f64; w1 * h1];
        for y in 0..h0 {
            for x in 0..w0 {
                let p = (y / 2) * w1 + x / 2;
                count[p] += 1;
                next[p] += field[y * w0 + x];
            }
        }
        bad += count.iter().filter(|&&c| c != 4).count() as u64;
        field = next;
        w0 = w1;
        h0 = h1;
    }
    let rolled: f64 = field.iter().sum();
    bad += ((rolled - total).abs() > 1e-9 * total) as u64;
    report("squares: 6 quadtree levels (1 km to 62.5 km), 4 children each, totals kept", bad);

    for kind in [Kind::A7Alt, Kind::A7Fixed, Kind::A4, Kind::A3] {
        let mut fits = 0;
        while HexTorus::level(HEX_N, HEX_M, kind, fits + 1).is_some() && fits < 8 {
            fits += 1;
        }
        println!("         {} on the 1862x1078 world: {} levels wrap exactly", kind.name(), fits);
    }
    let kind = Kind::A7Alt;
    let mut bad = 0u64;
    let mut field: Vec<f64> = (0..h.n()).map(|_| rng.f64()).collect();
    let total: f64 = field.iter().sum();
    for l in 0..4 {
        let fine = HexTorus::level(HEX_N, HEX_M, kind, l).unwrap();
        let coarse = HexTorus::level(HEX_N, HEX_M, kind, l + 1).unwrap();
        let mut count = vec![0u32; coarse.n()];
        let mut next = vec![0f64; coarse.n()];
        for i in 0..fine.n() {
            let p = coarse.index(kind.parent(l, fine.cell(i)));
            count[p] += 1;
            next[p] += field[i];
        }
        bad += count.iter().filter(|&&c| c != 7).count() as u64;
        field = next;
    }
    let rolled: f64 = field.iter().sum();
    bad += ((rolled - total).abs() > 1e-9 * total) as u64;
    report("hexes aperture 7 (H3-like): 4 levels (1.07 km to 52.6 km), 7 children each, totals kept", bad);
}

/// How exactly cells nest (WLD-12): the share of the area whose fine data rolls up, through the
/// logical parents, into a coarse cell that does not contain it geometrically.
pub fn nesting() {
    println!("== B10 nesting: area rolled up into the wrong coarse cell (WLD-12) ==");
    let mut rng = Rng::new(10);
    let samples = 400_000;

    // Logical nesting is a true partition: each parent has exactly `aperture` children.
    for kind in [Kind::A7Alt, Kind::A7Fixed, Kind::A4, Kind::A3] {
        let mut bad = 0u64;
        for l in 0..3 {
            let g = kind.gen(l);
            for cq in -15..=15 {
                for cr in -15..=15 {
                    let c = E::new(cq, cr);
                    let centre = c.mul(g);
                    let mut n = 0;
                    for dq in -3..=3i64 {
                        for dr in -3..=3i64 {
                            let d = E::new(dq, dr);
                            if d.steps() <= 3 && kind.parent(l, centre.add(d)) == c {
                                n += 1;
                            }
                        }
                    }
                    bad += (n != kind.aperture()) as u64;
                }
            }
        }
        report(&format!("{}: every parent has exactly {} children (3 levels)", kind.name(), kind.aperture()), bad);
    }

    println!("  levels up | linear scale-up | share of area in the wrong coarse cell");
    // Squares: the quadtree parent of the cell holding a point is the coarse cell holding it.
    let mut bad = 0u64;
    for _ in 0..samples {
        let (x, y) = (rng.range(-5000.0, 5000.0), rng.range(-5000.0, 5000.0));
        let (mut cx, mut cy) = (x.floor() as i64, y.floor() as i64);
        for _ in 0..6 {
            cx = cx.div_euclid(2);
            cy = cy.div_euclid(2);
        }
        bad += (cx != (x / 64.0).floor() as i64 || cy != (y / 64.0).floor() as i64) as u64;
    }
    println!("  squares (quadtree), 6 levels, x64: {:.3}%", 100.0 * bad as f64 / samples as f64);

    for kind in [Kind::A7Alt, Kind::A7Fixed, Kind::A4, Kind::A3] {
        let kmax = match kind {
            Kind::A7Alt | Kind::A7Fixed => 4,
            Kind::A4 => 6,
            Kind::A3 => 8,
        };
        let mut line = format!("  {}:", kind.name());
        for k in 1..=kmax {
            let g = kind.cum(k);
            let mut bad = 0u64;
            for _ in 0..samples {
                let (x, y) = (rng.range(-5000.0, 5000.0), rng.range(-5000.0, 5000.0));
                let mut z = hex::cell_at(x, y);
                for l in 0..k {
                    z = kind.parent(l, z);
                }
                let (qf, rf) = hex::point_div(x, y, g);
                bad += (z != hex::round_axial(qf, rf)) as u64;
            }
            line += &format!(
                " | {k} (x{:.1}): {:.1}%",
                (g.norm() as f64).sqrt(),
                100.0 * bad as f64 / samples as f64
            );
        }
        println!("{line}");
    }
}
