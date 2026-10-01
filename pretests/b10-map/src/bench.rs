//! B10 timings: neighbour visits and rollups at the 1 km level (WLD-12) on the whole torus.
//! Run under the shared CPU lock; each figure is the median of 7 runs, with min-max spread.

use crate::checks::{HEX_M, HEX_N, SQ_WORLD};
use crate::hex::{HexTorus, Kind, E};
use crate::rng::Rng;
use std::hint::black_box;
use std::time::Instant;

const REPS: usize = 7;

/// Median, min, max of seconds per run.
fn time<F: FnMut()>(mut f: F) -> (f64, f64, f64) {
    f();
    let mut v: Vec<f64> = (0..REPS)
        .map(|_| {
            let t = Instant::now();
            f();
            t.elapsed().as_secs_f64()
        })
        .collect();
    v.sort_by(|a, b| a.partial_cmp(b).unwrap());
    (v[REPS / 2], v[0], v[REPS - 1])
}

fn show(label: &str, units: usize, per: usize, t: (f64, f64, f64)) {
    let ns = |s: f64| s * 1e9 / units as f64;
    println!(
        "  {label:<46} {:>6.2} ns/cell (spread {:.2}-{:.2}){}",
        ns(t.0),
        ns(t.1),
        ns(t.2),
        if per > 0 {
            format!(", {:>5.0} M neighbour reads/s", (units * per) as f64 / t.0 / 1e6)
        } else {
            String::new()
        }
    );
}

#[inline(always)]
fn wrapadd(x: usize, s: usize, a: usize) -> usize {
    let y = x + s;
    if y >= a {
        y - a
    } else {
        y
    }
}

/// Every cell: sum of its 8 neighbours. Wrap tested per cell.
fn sweep_sq(w: usize, h: usize, f: &[f32], out: &mut [f32]) {
    for y in 0..h {
        let ym = if y == 0 { h - 1 } else { y - 1 };
        let yp = if y + 1 == h { 0 } else { y + 1 };
        let (r0, r1, r2) = (ym * w, y * w, yp * w);
        for x in 0..w {
            let xm = if x == 0 { w - 1 } else { x - 1 };
            let xp = if x + 1 == w { 0 } else { x + 1 };
            out[r1 + x] = f[r0 + xm] + f[r0 + x] + f[r0 + xp] + f[r1 + xm] + f[r1 + xp] + f[r2 + xm] + f[r2 + x] + f[r2 + xp];
        }
    }
}

/// Same, with the edge columns peeled off so the inner loop has no wrap tests.
fn sweep_sq_peeled(w: usize, h: usize, f: &[f32], out: &mut [f32]) {
    for y in 0..h {
        let ym = if y == 0 { h - 1 } else { y - 1 };
        let yp = if y + 1 == h { 0 } else { y + 1 };
        let (a, b, c) = (&f[ym * w..ym * w + w], &f[y * w..y * w + w], &f[yp * w..yp * w + w]);
        let o = &mut out[y * w..y * w + w];
        for x in 1..w - 1 {
            o[x] = a[x - 1] + a[x] + a[x + 1] + b[x - 1] + b[x + 1] + c[x - 1] + c[x] + c[x + 1];
        }
        for x in [0, w - 1] {
            let xm = if x == 0 { w - 1 } else { x - 1 };
            let xp = if x + 1 == w { 0 } else { x + 1 };
            o[x] = a[xm] + a[x] + a[xp] + b[xm] + b[xp] + c[xm] + c[x] + c[xp];
        }
    }
}

/// Every cell: sum of its 4 side neighbours (reference).
fn sweep_sq4(w: usize, h: usize, f: &[f32], out: &mut [f32]) {
    for y in 0..h {
        let ym = if y == 0 { h - 1 } else { y - 1 };
        let yp = if y + 1 == h { 0 } else { y + 1 };
        let (r0, r1, r2) = (ym * w, y * w, yp * w);
        for x in 0..w {
            let xm = if x == 0 { w - 1 } else { x - 1 };
            let xp = if x + 1 == w { 0 } else { x + 1 };
            out[r1 + x] = f[r0 + x] + f[r1 + xm] + f[r1 + xp] + f[r2 + x];
        }
    }
}

/// Hex torus in sheared storage (d rows of a cells). Neighbours of (q, r): (q +- 1, r),
/// (q, r - 1), (q + 1, r - 1), (q, r + 1), (q - 1, r + 1). Crossing the pole seam shifts q by b.
fn sweep_hex(t: &HexTorus, f: &[f32], out: &mut [f32]) {
    let (a, b, d) = (t.a as usize, t.b as usize, t.d as usize);
    for r in 0..d {
        let (ru, su) = if r == 0 { (d - 1, b) } else { (r - 1, 0) };
        let (rd, sd) = if r + 1 == d { (0, a - b) } else { (r + 1, 0) };
        let (r0, r1, r2) = (ru * a, r * a, rd * a);
        for q in 0..a {
            let qm = if q == 0 { a - 1 } else { q - 1 };
            let qp = if q + 1 == a { 0 } else { q + 1 };
            out[r1 + q] = f[r1 + qm]
                + f[r1 + qp]
                + f[r0 + wrapadd(q, su, a)]
                + f[r0 + wrapadd(qp, su, a)]
                + f[r2 + wrapadd(q, sd, a)]
                + f[r2 + wrapadd(qm, sd, a)];
        }
    }
}

fn sweep_hex_peeled(t: &HexTorus, f: &[f32], out: &mut [f32]) {
    let (a, b, d) = (t.a as usize, t.b as usize, t.d as usize);
    for r in 0..d {
        let (ru, su) = if r == 0 { (d - 1, b) } else { (r - 1, 0) };
        let (rd, sd) = if r + 1 == d { (0, a - b) } else { (r + 1, 0) };
        let (up, mid, dn) = (&f[ru * a..ru * a + a], &f[r * a..r * a + a], &f[rd * a..rd * a + a]);
        let o = &mut out[r * a..r * a + a];
        if su == 0 && sd == 0 {
            for q in 1..a - 1 {
                o[q] = mid[q - 1] + mid[q + 1] + up[q] + up[q + 1] + dn[q] + dn[q - 1];
            }
            for q in [0, a - 1] {
                let qm = if q == 0 { a - 1 } else { q - 1 };
                let qp = if q + 1 == a { 0 } else { q + 1 };
                o[q] = mid[qm] + mid[qp] + up[q] + up[qp] + dn[q] + dn[qm];
            }
        } else {
            for q in 0..a {
                let qm = if q == 0 { a - 1 } else { q - 1 };
                let qp = if q + 1 == a { 0 } else { q + 1 };
                o[q] = mid[qm]
                    + mid[qp]
                    + up[wrapadd(q, su, a)]
                    + up[wrapadd(qp, su, a)]
                    + dn[wrapadd(q, sd, a)]
                    + dn[wrapadd(qm, sd, a)];
            }
        }
    }
}

/// Random cells: work out the neighbours from the cell index, read them.
fn rand_sq(w: usize, h: usize, idx: &[u32], f: &[f32]) -> f32 {
    let mut s = 0.0f32;
    for &i in idx {
        let i = i as usize;
        let (x, y) = (i % w, i / w);
        let xm = if x == 0 { w - 1 } else { x - 1 };
        let xp = if x + 1 == w { 0 } else { x + 1 };
        let ym = if y == 0 { h - 1 } else { y - 1 };
        let yp = if y + 1 == h { 0 } else { y + 1 };
        let (r0, r1, r2) = (ym * w, y * w, yp * w);
        s += f[r0 + xm] + f[r0 + x] + f[r0 + xp] + f[r1 + xm] + f[r1 + xp] + f[r2 + xm] + f[r2 + x] + f[r2 + xp];
    }
    s
}

fn rand_hex(t: &HexTorus, idx: &[u32], f: &[f32]) -> f32 {
    let (a, b, d) = (t.a as usize, t.b as usize, t.d as usize);
    let mut s = 0.0f32;
    for &i in idx {
        let i = i as usize;
        let (q, r) = (i % a, i / a);
        let qm = if q == 0 { a - 1 } else { q - 1 };
        let qp = if q + 1 == a { 0 } else { q + 1 };
        let (ru, su) = if r == 0 { (d - 1, b) } else { (r - 1, 0) };
        let (rd, sd) = if r + 1 == d { (0, a - b) } else { (r + 1, 0) };
        let (r0, r1, r2) = (ru * a, r * a, rd * a);
        s += f[r1 + qm]
            + f[r1 + qp]
            + f[r0 + wrapadd(q, su, a)]
            + f[r0 + wrapadd(qp, su, a)]
            + f[r2 + wrapadd(q, sd, a)]
            + f[r2 + wrapadd(qm, sd, a)];
    }
    s
}

/// Index arithmetic only: neighbour indices of random cells, no reads.
fn idx_sq(w: usize, h: usize, idx: &[u32]) -> usize {
    let mut s = 0usize;
    for &i in idx {
        let i = i as usize;
        let (x, y) = (i % w, i / w);
        let xm = if x == 0 { w - 1 } else { x - 1 };
        let xp = if x + 1 == w { 0 } else { x + 1 };
        let ym = if y == 0 { h - 1 } else { y - 1 };
        let yp = if y + 1 == h { 0 } else { y + 1 };
        let (r0, r1, r2) = (ym * w, y * w, yp * w);
        s ^= (r0 + xm) ^ (r0 + x).rotate_left(3) ^ (r0 + xp).rotate_left(6) ^ (r1 + xm).rotate_left(9)
            ^ (r1 + xp).rotate_left(12) ^ (r2 + xm).rotate_left(15) ^ (r2 + x).rotate_left(18) ^ (r2 + xp).rotate_left(21);
    }
    s
}

fn idx_hex(t: &HexTorus, idx: &[u32]) -> usize {
    let (a, b, d) = (t.a as usize, t.b as usize, t.d as usize);
    let mut s = 0usize;
    for &i in idx {
        let i = i as usize;
        let (q, r) = (i % a, i / a);
        let qm = if q == 0 { a - 1 } else { q - 1 };
        let qp = if q + 1 == a { 0 } else { q + 1 };
        let (ru, su) = if r == 0 { (d - 1, b) } else { (r - 1, 0) };
        let (rd, sd) = if r + 1 == d { (0, a - b) } else { (r + 1, 0) };
        let (r0, r1, r2) = (ru * a, r * a, rd * a);
        s ^= (r1 + qm) ^ (r1 + qp).rotate_left(3) ^ (r0 + wrapadd(q, su, a)).rotate_left(6)
            ^ (r0 + wrapadd(qp, su, a)).rotate_left(9) ^ (r2 + wrapadd(q, sd, a)).rotate_left(12)
            ^ (r2 + wrapadd(qm, sd, a)).rotate_left(15);
    }
    s
}

/// Quadtree rollup: each coarse cell is the mean of its 2 x 2 children.
fn rollup_sq(w: usize, h: usize, f: &[f32], out: &mut [f32]) {
    let (w2, h2) = (w / 2, h / 2);
    for y in 0..h2 {
        let (a, b) = (&f[2 * y * w..2 * y * w + w], &f[(2 * y + 1) * w..(2 * y + 1) * w + w]);
        let o = &mut out[y * w2..y * w2 + w2];
        for x in 0..w2 {
            o[x] = 0.25 * (a[2 * x] + a[2 * x + 1] + b[2 * x] + b[2 * x + 1]);
        }
    }
}

fn pyramid_sq(levels: usize, f: &[f32], bufs: &mut [Vec<f32>]) {
    let (mut w, mut h) = (SQ_WORLD.w as usize, SQ_WORLD.h as usize);
    rollup_sq(w, h, f, &mut bufs[0]);
    for l in 1..levels {
        w /= 2;
        h /= 2;
        let (lo, hi) = bufs.split_at_mut(l);
        rollup_sq(w, h, &lo[l - 1], &mut hi[0]);
    }
}

/// Hex rollup, parent worked out per cell (exact integer maths + rounding + torus wrap).
fn rollup_hex_fly(fine: &HexTorus, coarse: &HexTorus, kind: Kind, l: usize, f: &[f32], out: &mut [f32]) {
    out.iter_mut().for_each(|v| *v = 0.0);
    let a = fine.a as usize;
    for r in 0..fine.d {
        let row = &f[r as usize * a..r as usize * a + a];
        for q in 0..fine.a {
            let p = kind.parent(l, E::new(q, r));
            out[coarse.index(p)] += row[q as usize];
        }
    }
    let inv = 1.0 / kind.aperture() as f32;
    out.iter_mut().for_each(|v| *v *= inv);
}

fn parent_table(fine: &HexTorus, coarse: &HexTorus, kind: Kind, l: usize) -> Vec<u32> {
    (0..fine.n()).map(|i| coarse.index(kind.parent(l, fine.cell(i))) as u32).collect()
}

/// Hex rollup through a stored parent table (4 extra bytes per fine cell).
fn rollup_hex_table(par: &[u32], inv: f32, f: &[f32], out: &mut [f32]) {
    out.iter_mut().for_each(|v| *v = 0.0);
    for (i, &p) in par.iter().enumerate() {
        out[p as usize] += f[i];
    }
    out.iter_mut().for_each(|v| *v *= inv);
}

pub fn run() {
    println!("== B10 timings (median of {REPS} runs, spread min-max) ==");
    let (w, h) = (SQ_WORLD.w as usize, SQ_WORLD.h as usize);
    let ht = HexTorus::fine(HEX_N, HEX_M);
    let (ns, nh) = (w * h, ht.n());
    println!("  squares: {w} x {h} = {ns} cells; hexes: {} x {} = {nh} cells", ht.a, ht.d);
    let mut rng = Rng::new(7);
    let fs: Vec<f32> = (0..ns).map(|_| rng.f64() as f32).collect();
    let fh: Vec<f32> = (0..nh).map(|_| rng.f64() as f32).collect();
    let mut os = vec![0f32; ns];
    let mut oh = vec![0f32; nh];

    println!(" all neighbours of every cell, in storage order:");
    let t = time(|| {
        sweep_sq(w, h, &fs, &mut os);
        black_box(&os);
    });
    show("squares, 8 neighbours", ns, 8, t);
    let t = time(|| {
        sweep_hex(&ht, &fh, &mut oh);
        black_box(&oh);
    });
    show("hexes, 6 neighbours", nh, 6, t);
    let t = time(|| {
        sweep_sq_peeled(w, h, &fs, &mut os);
        black_box(&os);
    });
    show("squares, 8 neighbours, edge columns peeled", ns, 8, t);
    let t = time(|| {
        sweep_hex_peeled(&ht, &fh, &mut oh);
        black_box(&oh);
    });
    show("hexes, 6 neighbours, edge columns peeled", nh, 6, t);
    let t = time(|| {
        sweep_sq4(w, h, &fs, &mut os);
        black_box(&os);
    });
    show("squares, 4 neighbours (reference)", ns, 4, t);

    println!(" all neighbours of 1M random cells (index to coordinates, wrap, read):");
    let is: Vec<u32> = (0..1_000_000).map(|_| rng.below(ns as u64) as u32).collect();
    let ih: Vec<u32> = (0..1_000_000).map(|_| rng.below(nh as u64) as u32).collect();
    let t = time(|| {
        black_box(rand_sq(w, h, black_box(&is), &fs));
    });
    show("squares, 8 neighbours", is.len(), 8, t);
    let t = time(|| {
        black_box(rand_hex(&ht, black_box(&ih), &fh));
    });
    show("hexes, 6 neighbours", ih.len(), 6, t);
    let t = time(|| {
        black_box(idx_sq(w, h, black_box(&is)));
    });
    show("squares, 8 neighbour indices only (no reads)", is.len(), 8, t);
    let t = time(|| {
        black_box(idx_hex(&ht, black_box(&ih)));
    });
    show("hexes, 6 neighbour indices only (no reads)", ih.len(), 6, t);

    println!(" rolling 1 km data up (per fine cell):");
    let mut bufs: Vec<Vec<f32>> = (1..=6).map(|l| vec![0f32; ns >> (2 * l)]).collect();
    let t = time(|| {
        pyramid_sq(1, &fs, &mut bufs);
        black_box(&bufs);
    });
    show("squares, one level (x2)", ns, 0, t);
    let t = time(|| {
        pyramid_sq(6, &fs, &mut bufs);
        black_box(&bufs);
    });
    show("squares, six levels (to 62.5 km)", ns, 0, t);

    let kind = Kind::A7Alt;
    let lv: Vec<HexTorus> = (0..=4).map(|k| HexTorus::level(HEX_N, HEX_M, kind, k).unwrap()).collect();
    let mut hb: Vec<Vec<f32>> = lv.iter().map(|t| vec![0f32; t.n()]).collect();
    let t = time(|| {
        let (lo, hi) = hb.split_at_mut(1);
        rollup_hex_fly(&lv[0], &lv[1], kind, 0, &fh, &mut hi[0]);
        black_box(&lo);
    });
    show("hexes A7, one level (x2.65), parent computed", nh, 0, t);
    let t = time(|| {
        let (_, hi) = hb.split_at_mut(1);
        rollup_hex_fly(&lv[0], &lv[1], kind, 0, &fh, &mut hi[0]);
        for l in 1..4 {
            let (lo, hi) = hb.split_at_mut(l + 1);
            rollup_hex_fly(&lv[l], &lv[l + 1], kind, l, &lo[l], &mut hi[0]);
        }
        black_box(&hb);
    });
    show("hexes A7, four levels (to 52.6 km), computed", nh, 0, t);
    let tb = Instant::now();
    let tables: Vec<Vec<u32>> = (0..4).map(|l| parent_table(&lv[l], &lv[l + 1], kind, l)).collect();
    println!("  (building the four parent tables once took {:.0} ms, {:.1} MB)", tb.elapsed().as_secs_f64() * 1e3,
        tables.iter().map(|t| t.len() * 4).sum::<usize>() as f64 / 1e6);
    let t = time(|| {
        let (_, hi) = hb.split_at_mut(1);
        rollup_hex_table(&tables[0], 1.0 / 7.0, &fh, &mut hi[0]);
        black_box(&hb);
    });
    show("hexes A7, one level, parent table", nh, 0, t);
    let t = time(|| {
        let (_, hi) = hb.split_at_mut(1);
        rollup_hex_table(&tables[0], 1.0 / 7.0, &fh, &mut hi[0]);
        for l in 1..4 {
            let (lo, hi) = hb.split_at_mut(l + 1);
            rollup_hex_table(&tables[l], 1.0 / 7.0, &lo[l], &mut hi[0]);
        }
        black_box(&hb);
    });
    show("hexes A7, four levels, parent tables", nh, 0, t);

    // Aperture 4 needs a torus whose sides halve cleanly: 1856 x 1072.
    let k4 = Kind::A4;
    let f4 = HexTorus::level(1856, 536, k4, 0).unwrap();
    let c4 = HexTorus::level(1856, 536, k4, 1).unwrap();
    let ff: Vec<f32> = (0..f4.n()).map(|_| rng.f64() as f32).collect();
    let mut o4 = vec![0f32; c4.n()];
    let t = time(|| {
        rollup_hex_fly(&f4, &c4, k4, 0, &ff, &mut o4);
        black_box(&o4);
    });
    show("hexes A4 (1856x1072), one level (x2), computed", f4.n(), 0, t);
}
