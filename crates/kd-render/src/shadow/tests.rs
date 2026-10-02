use super::*;

/// The shadow map texel a point lands on, x and y, from the light's view-projection.
fn texel_at(c: &LightCam, p: [f32; 3]) -> [f32; 2] {
    let v = &c.vp.0;
    let clip = |row: usize| v[row] * p[0] + v[4 + row] * p[1] + v[8 + row] * p[2] + v[12 + row];
    [
        (clip(0) * 0.5 + 0.5) * SHADOW_SIZE as f32,
        (clip(1) * 0.5 + 0.5) * SHADOW_SIZE as f32,
    ]
}

/// A view's footprint: a 70 × 200 m box of ground from 300 to 380 m up, moved by `dx` metres east.
fn foot(dx: f32) -> [[f32; 3]; 8] {
    let mut f = [[0.0; 3]; 8];
    let mut k = 0;
    for x in [-35.0, 35.0] {
        for z in [-100.0, 100.0] {
            for y in [300.0, 380.0] {
                f[k] = [x + dx, y, z];
                k += 1;
            }
        }
    }
    f
}

// checks: PRE-30
#[test]
fn light_frustum_snaps() {
    let l = norm([-0.8, 0.36, 0.32]);
    let a = light_for(&foot(0.0), l);
    for dx in [0.37, 1.9, -5.3] {
        let b = light_for(&foot(dx * a.texel), l);
        assert_eq!(a.texel, b.texel, "small moves keep the shadow map's scale");
        // a fixed point of ground lands at the same fraction of a shadow texel, so its shadow never swims
        for p in [[1.0, 330.0, 3.0], [-20.5, 350.25, 61.0], [33.0, 301.0, -90.0]] {
            let (ta, tb) = (texel_at(&a, p), texel_at(&b, p));
            for i in 0..2 {
                let d = (ta[i] - tb[i]).rem_euclid(1.0);
                assert!(!(1e-2..=1.0 - 1e-2).contains(&d), "moved {dx} texels: {ta:?} vs {tb:?}");
            }
        }
    }
    // the footprint fits inside the map, and the bias grows with the texel
    for p in foot(0.0) {
        let t = texel_at(&a, p);
        assert!(t.iter().all(|&v| (0.0..SHADOW_SIZE as f32).contains(&v)), "{t:?}");
    }
    assert!(a.bias()[0] >= 0.015 && a.bias()[1] > 0.0);
}
