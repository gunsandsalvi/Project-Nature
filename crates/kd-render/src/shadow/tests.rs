use super::*;

/// The shadow map texel a point lands on, x and y, from the light's view-projection and window, and its depth.
fn texel_at(c: &LightCam, p: [f32; 3]) -> [f32; 3] {
    let v = &c.vp.0;
    let clip = |row: usize| v[row] * p[0] + v[4 + row] * p[1] + v[8 + row] * p[2] + v[12 + row];
    let [span, kx, ky] = c.window();
    [
        (clip(0) * 0.5 + 0.5) * span - kx,
        (clip(1) * 0.5 + 0.5) * span - ky,
        clip(2),
    ]
}

/// A box of ground `w` × `d` m from 300 to 380 m up, moved by `dx` metres east: a view's footprint (70 × 200) or its
/// block's (300 × 400).
fn bx(w: f32, d: f32, dx: f32) -> [[f32; 3]; 8] {
    let mut f = [[0.0; 3]; 8];
    let mut k = 0;
    for x in [-w / 2.0, w / 2.0] {
        for z in [-d / 2.0, d / 2.0] {
            for y in [300.0, 380.0] {
                f[k] = [x + dx, y, z];
                k += 1;
            }
        }
    }
    f
}

const POINTS: [[f32; 3]; 3] = [[1.0, 330.0, 3.0], [-20.5, 350.25, 61.0], [33.0, 301.0, -90.0]];

// checks: PRE-30
#[test]
fn light_frustum_snaps() {
    // (a footprint off the axes, as a symmetric one centres on a texel's edge)
    let (l, x) = (norm([-0.8, 0.36, 0.32]), 13.37);
    let block = bx(300.0, 400.0, x);
    let a = light_for(&bx(70.0, 200.0, x), &block, l, [0.0; 3]);
    for dx in [0.37, 1.9, -5.3] {
        let b = light_for(&bx(70.0, 200.0, x + dx * a.texel), &block, l, [0.0; 3]);
        assert_eq!(a.texel, b.texel, "small moves keep the shadow map's scale");
        // a fixed point of ground lands at the same fraction of a shadow texel, so its shadow never swims
        for p in POINTS {
            let (ta, tb) = (texel_at(&a, p), texel_at(&b, p));
            for i in 0..2 {
                let d = (ta[i] - tb[i]).rem_euclid(1.0);
                assert!(!(1e-2..=1.0 - 1e-2).contains(&d), "moved {dx} texels: {ta:?} vs {tb:?}");
            }
        }
    }
    // moves along the light's right from the middle of a block keep the very projection; only the window moves, by
    // whole texels
    let rl = norm(cross([-l[0], -l[1], -l[2]], [0.0, 1.0, 0.0]));
    let mid = |n: i32| {
        let s = (128 + a.view[0] + n) as f32 * a.texel;
        let f = |p: [f32; 3]| [p[0] + rl[0] * s, p[1], p[2] + rl[2] * s];
        light_for(&bx(70.0, 200.0, x).map(f), &block, l, [0.0; 3])
    };
    let m0 = mid(0);
    assert_eq!(m0.view[0], -128);
    for n in 1..=5 {
        let m = mid(n);
        assert_eq!((m.vp, m.depth_r), (m0.vp, m0.depth_r));
        assert_eq!(m.view, [m0.view[0] - n, m0.view[1], m0.view[2], m0.view[3]]);
    }
    // the same ground from the next area's corner (the floating origin moved): the same texels and depths
    let shift = |p: [f32; 3]| [p[0] - 256.0, p[1], p[2]];
    let b = light_for(&bx(70.0, 200.0, x).map(shift), &block.map(shift), l, [256.0, 0.0, 0.0]);
    assert_eq!((a.texel, a.depth_r, a.view), (b.texel, b.depth_r, b.view));
    for p in POINTS {
        let (ta, tb) = (texel_at(&a, p), texel_at(&b, shift(p)));
        assert!(
            (ta[0] - tb[0]).abs() < 1e-2 && (ta[1] - tb[1]).abs() < 1e-2 && (ta[2] - tb[2]).abs() < 1e-5,
            "{ta:?} vs {tb:?}"
        );
    }
    // the footprint fits inside the map, and the bias grows with the texel
    for p in bx(70.0, 200.0, x) {
        let t = texel_at(&a, p);
        assert!(t[..2].iter().all(|&v| (0.0..SHADOW_SIZE as f32).contains(&v)), "{t:?}");
    }
    assert!(a.bias()[0] >= 0.015 && a.bias()[1] > 0.0);
    assert_eq!(a.view[2], SHADOW_SIZE as i32 + LIGHT_BLOCK as i32);
}
