//! B10: shortest paths and a simple any-angle smoothing, used by the wrap checks and the
//! isotropy test. Stand-in only; real route-finding belongs to B12.

use std::cmp::Ordering;
use std::collections::BinaryHeap;

#[derive(Clone, Copy)]
struct Item {
    f: f64,
    n: u32,
}
impl PartialEq for Item {
    fn eq(&self, o: &Self) -> bool {
        self.f == o.f
    }
}
impl Eq for Item {}
impl PartialOrd for Item {
    fn partial_cmp(&self, o: &Self) -> Option<Ordering> {
        Some(self.cmp(o))
    }
}
impl Ord for Item {
    // Reversed: BinaryHeap pops the smallest f first.
    fn cmp(&self, o: &Self) -> Ordering {
        o.f.partial_cmp(&self.f).unwrap_or(Ordering::Equal)
    }
}

pub struct Search {
    g: Vec<f64>,
    par: Vec<u32>,
    seen: Vec<u32>,
    done: Vec<u32>,
    cur: u32,
    heap: BinaryHeap<Item>,
    nb: Vec<(usize, f64)>,
}

impl Search {
    pub fn new(n: usize) -> Search {
        Search {
            g: vec![0.0; n],
            par: vec![0; n],
            seen: vec![0; n],
            done: vec![0; n],
            cur: 0,
            heap: BinaryHeap::new(),
            nb: Vec::with_capacity(16),
        }
    }

    /// A* from s to t (t = usize::MAX: Dijkstra to every node). `neigh` lists (node, cost);
    /// `h` must be a consistent lower bound.
    fn search(
        &mut self,
        s: usize,
        t: usize,
        neigh: &dyn Fn(usize, &mut Vec<(usize, f64)>),
        h: &dyn Fn(usize) -> f64,
    ) {
        self.cur += 1;
        let cur = self.cur;
        self.heap.clear();
        self.g[s] = 0.0;
        self.seen[s] = cur;
        self.par[s] = s as u32;
        self.heap.push(Item { f: h(s), n: s as u32 });
        let mut nb = std::mem::take(&mut self.nb);
        while let Some(Item { n, .. }) = self.heap.pop() {
            let u = n as usize;
            if self.done[u] == cur {
                continue;
            }
            self.done[u] = cur;
            if u == t {
                break;
            }
            nb.clear();
            neigh(u, &mut nb);
            let gu = self.g[u];
            for &(v, c) in nb.iter() {
                if self.done[v] == cur {
                    continue;
                }
                let gv = gu + c;
                if self.seen[v] != cur || gv < self.g[v] {
                    self.seen[v] = cur;
                    self.g[v] = gv;
                    self.par[v] = u as u32;
                    self.heap.push(Item { f: gv + h(v), n: v as u32 });
                }
            }
        }
        self.nb = nb;
    }

    pub fn path(
        &mut self,
        s: usize,
        t: usize,
        neigh: &dyn Fn(usize, &mut Vec<(usize, f64)>),
        h: &dyn Fn(usize) -> f64,
    ) -> Option<(f64, Vec<usize>)> {
        self.search(s, t, neigh, h);
        if self.done[t] != self.cur {
            return None;
        }
        let mut p = vec![t];
        let mut u = t;
        while u != s {
            u = self.par[u] as usize;
            p.push(u);
        }
        p.reverse();
        Some((self.g[t], p))
    }

    /// Distances from s to every node (infinity where unreachable).
    pub fn all(&mut self, s: usize, neigh: &dyn Fn(usize, &mut Vec<(usize, f64)>)) -> Vec<f64> {
        self.search(s, usize::MAX, neigh, &|_| 0.0);
        (0..self.g.len())
            .map(|i| if self.done[i] == self.cur { self.g[i] } else { f64::INFINITY })
            .collect()
    }
}

pub type P = (f64, f64);

pub fn length(p: &[P]) -> f64 {
    p.windows(2).map(|w| ((w[1].0 - w[0].0).powi(2) + (w[1].1 - w[0].1).powi(2)).sqrt()).sum()
}

/// Line of sight: every sample along a to b, `step` apart, is free.
pub fn los(a: P, b: P, step: f64, free: &dyn Fn(f64, f64) -> bool) -> bool {
    let (dx, dy) = (b.0 - a.0, b.1 - a.1);
    let n = (((dx * dx + dy * dy).sqrt() / step).ceil() as usize).max(1);
    (0..=n).all(|k| {
        let t = k as f64 / n as f64;
        free(a.0 + t * dx, a.1 + t * dy)
    })
}

/// Simple any-angle smoothing ("string pulling"): from each kept point, skip ahead along the
/// path for as long as the next point is still in sight.
pub fn smooth(p: &[P], step: f64, free: &dyn Fn(f64, f64) -> bool) -> Vec<P> {
    if p.len() < 3 {
        return p.to_vec();
    }
    let mut out = vec![p[0]];
    let mut anchor = p[0];
    for i in 1..p.len() - 1 {
        if !los(anchor, p[i + 1], step, free) {
            out.push(p[i]);
            anchor = p[i];
        }
    }
    out.push(p[p.len() - 1]);
    out
}
