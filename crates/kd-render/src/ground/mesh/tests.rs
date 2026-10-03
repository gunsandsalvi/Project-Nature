use super::*;

/// A 256 m area of rolling ground with a 30 m step across it, and four surfaces in bands.
fn area() -> GroundGrid {
    let side = 257;
    let heights_m = (0..side * side)
        .map(|k| {
            let (x, y) = ((k % side) as f32, (k / side) as f32);
            let wave = (x * 0.05).sin() * 3.0 + (y * 0.031).cos() * 4.0;
            let step = if y > 128.0 + 20.0 * (x * 0.02).sin() { 30.0 } else { 0.0 };
            320.0 + wave + step
        })
        .collect();
    let surface = (0..256 * 256).map(|k| ((k / 256) / 64) as u8).collect();
    GroundGrid {
        side,
        heights_m,
        surface,
        ..GroundGrid::default()
    }
}

// checks: PRE-02
#[test]
fn chunk_edges_match() {
    let g = area();
    for step in [1, 2, 4, 8] {
        let chunks = build_chunks(&g, step);
        assert_eq!(chunks.len(), 16, "step {step}");
        let per = (CHUNK_M / step) as usize + 1;
        let at = |c: &Chunk, i: usize, j: usize| c.verts[j * per + i];
        for a in &chunks {
            assert!(a.verts.len() < 65_536);
            for b in &chunks {
                if b.cx == a.cx + 1 && b.cy == a.cy {
                    for j in 0..per {
                        assert_eq!(at(a, per - 1, j).bytes(), at(b, 0, j).bytes(), "step {step}");
                    }
                }
                if b.cy == a.cy + 1 && b.cx == a.cx {
                    for i in 0..per {
                        assert_eq!(at(a, i, per - 1).bytes(), at(b, i, 0).bytes(), "step {step}");
                    }
                }
            }
        }
        // every triangle names a vertex of its chunk
        assert!(
            chunks
                .iter()
                .all(|c| c.idx.iter().all(|&k| (k as usize) < c.verts.len()))
        );
    }
    assert_eq!(spacing_for(0.13), 1);
    assert_eq!(spacing_for(1.1), 2);
    assert_eq!(spacing_for(2.0), 4);
    assert_eq!(spacing_for(6.0), 8);
}

// checks: PRE-02
#[test]
fn vertex_is_20_bytes() {
    let v = GroundVertex {
        pos: [1.0, 2.0, 3.0],
        nrm: [0, 127, -1],
        surface: 2,
        water: 255,
        wear: 0,
        cover: 0,
        flags: 0,
    };
    let b = v.bytes();
    assert_eq!(b.len(), 20);
    assert_eq!(&b[0..4], &1.0f32.to_le_bytes());
    assert_eq!(b[13], 127);
    assert_eq!(b[14], 255); // -1 as a byte
    assert_eq!(b[15], 2);
    // flat ground's normal points straight up
    let flat = GroundGrid {
        side: 257,
        heights_m: vec![300.0; 257 * 257],
        surface: vec![0; 256 * 256],
        ..GroundGrid::default()
    };
    assert!(
        build_chunks(&flat, 4)
            .iter()
            .all(|c| c.verts.iter().all(|v| v.nrm == [0, 127, 0]))
    );
}
