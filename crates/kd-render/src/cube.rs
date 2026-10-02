//! The cube's mesh: 24 vertices (a quad per face with its face normal) and 36 indices, counter-clockwise from outside.

/// Position then normal, per vertex.
pub type Vertex = [f32; 6];

/// The faces as (normal, first edge, second edge), with first × second = normal, so the corners run
/// counter-clockwise seen from outside.
const FACES: [([f32; 3], [f32; 3], [f32; 3]); 6] = [
    ([1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]),
    ([-1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, 1.0, 0.0]),
    ([0.0, 1.0, 0.0], [0.0, 0.0, 1.0], [1.0, 0.0, 0.0]),
    ([0.0, -1.0, 0.0], [1.0, 0.0, 0.0], [0.0, 0.0, 1.0]),
    ([0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0]),
    ([0.0, 0.0, -1.0], [0.0, 1.0, 0.0], [1.0, 0.0, 0.0]),
];

/// The cube of side 1, centred on the origin.
pub fn mesh() -> (Vec<Vertex>, Vec<u16>) {
    let mut v = Vec::with_capacity(24);
    let mut idx = Vec::with_capacity(36);
    for (n, a, b) in FACES {
        let base = v.len() as u16;
        for (sa, sb) in [(-0.5, -0.5), (0.5, -0.5), (0.5, 0.5), (-0.5, 0.5)] {
            let p = [0, 1, 2].map(|k| n[k] * 0.5 + a[k] * sa + b[k] * sb);
            v.push([p[0], p[1], p[2], n[0], n[1], n[2]]);
        }
        idx.extend_from_slice(&[base, base + 1, base + 2, base, base + 2, base + 3]);
    }
    (v, idx)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn sub(a: &Vertex, b: &Vertex) -> [f32; 3] {
        [a[0] - b[0], a[1] - b[1], a[2] - b[2]]
    }

    // checks: PRC-11
    #[test]
    fn cube_counts_and_normals() {
        let (v, idx) = mesh();
        assert_eq!(v.len(), 24);
        assert_eq!(idx.len(), 36);
        for p in &v {
            let len2 = p[3] * p[3] + p[4] * p[4] + p[5] * p[5];
            assert!((len2 - 1.0).abs() < 1e-6, "unit normal");
            assert!(p[0] * p[3] + p[1] * p[4] + p[2] * p[5] > 0.0, "normal points out");
        }
        for t in idx.chunks(3) {
            let (a, b, c) = (&v[t[0] as usize], &v[t[1] as usize], &v[t[2] as usize]);
            let (e1, e2) = (sub(b, a), sub(c, a));
            let cross = [
                e1[1] * e2[2] - e1[2] * e2[1],
                e1[2] * e2[0] - e1[0] * e2[2],
                e1[0] * e2[1] - e1[1] * e2[0],
            ];
            assert!(
                cross[0] * a[3] + cross[1] * a[4] + cross[2] * a[5] > 0.0,
                "counter-clockwise from outside"
            );
        }
    }
}
