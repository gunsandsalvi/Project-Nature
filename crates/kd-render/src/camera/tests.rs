use super::*;

/// The phone's art target in portrait (1080 × 2404 at 4) and turned.
const PORTRAIT: ArtSize = ArtSize {
    wf: 273,
    hf: 604,
    wd: 1080,
    hd: 2404,
    s: 4,
};
const LANDSCAPE: ArtSize = ArtSize {
    wf: 604,
    hf: 273,
    wd: 2404,
    hd: 1080,
    s: 4,
};

fn pose(zoom: f32, yaw: f32) -> CameraPose {
    CameraPose {
        target: Pos {
            x: 300_000_000,
            y: 100_000_000,
            z: 340 * 256,
        },
        yaw,
        zoom,
    }
}

// checks: PRE-03
#[test]
fn stops_hit_their_texels() {
    for (z, t) in STOPS {
        let got = texel(z, PORTRAIT);
        assert!((got / t - 1.0).abs() < 0.001, "zoom {z}: {got} m, want {t} m");
    }
    // the globe fits 0.84 of the shorter side: about 2.9 km an art pixel on the phone
    let g = texel(1.0, PORTRAIT);
    assert!(
        (g / texel_max(PORTRAIT) - 1.0).abs() < 0.001 && (2_700.0..3_100.0).contains(&g),
        "{g}"
    );
    // the pitch: 27 degrees at the person stop, straight down from 20 m art pixels
    assert!((pitch(0.03).to_degrees() - 27.0).abs() < 0.01);
    assert!((pitch(37.0).to_degrees() - 90.0).abs() < 0.01);
}

/// `p` moved `d` metres along the ground direction (`gx`, `gz`) in GPU axes (x east, z south).
fn moved(p: CameraPose, gx: f32, gz: f32, d: f32) -> CameraPose {
    CameraPose {
        target: geo::offset(p.target, geo::Vec2 { x: gx * d, y: gz * d }),
        ..p
    }
}

/// Where the picture lies along right and up: the snapped view plus the upscale's shift, in art pixels.
fn at(c: &Camera) -> [f64; 2] {
    [
        c.snapped[0] as f64 + f64::from(c.off[0]),
        c.snapped[1] as f64 + f64::from(c.off[1]),
    ]
}

// checks: PRE-22
#[test]
fn snap_moves_whole_pixels() {
    for yaw in [0.0, 0.7, -2.1] {
        let mut p = pose(0.3, yaw);
        // centre the target in its art pixel first, so rounding to whole ticks cannot cross a pixel's edge
        let c = compute(&p, PORTRAIT, 300.0, 380.0);
        let [fx, fy] = c.frac;
        p = moved(p, c.r[0], c.r[2], (0.5 - fx) * c.texel);
        let hl = (c.f[0] * c.f[0] + c.f[2] * c.f[2]).sqrt();
        let (hx, hz) = (c.f[0] / hl, c.f[2] / hl);
        let up_per_m = c.u[0] * hx + c.u[2] * hz;
        p = moved(p, hx, hz, (0.5 - fy) * c.texel / up_per_m);
        let a = compute(&p, PORTRAIT, 300.0, 380.0);
        assert!(a.off.iter().all(|o| (0.0..3.0).contains(o)), "{:?}", a.off);
        // one art pixel along the view's right moves the picture by one
        let b = compute(&moved(p, a.r[0], a.r[2], a.texel), PORTRAIT, 300.0, 380.0);
        let (pa, pb) = (at(&a), at(&b));
        assert!(
            (pb[0] - pa[0] - 1.0).abs() < 0.01 && (pb[1] - pa[1]).abs() < 0.01,
            "yaw {yaw}"
        );
        // two move the snapped view by two, as it snaps to even art pixels, and leave the shift alone
        let b = compute(&moved(p, a.r[0], a.r[2], 2.0 * a.texel), PORTRAIT, 300.0, 380.0);
        assert_eq!(b.snapped, [a.snapped[0] + 2, a.snapped[1]], "yaw {yaw}");
        assert!(
            (b.off[0] - a.off[0]).abs() < 0.01 && (b.off[1] - a.off[1]).abs() < 0.01,
            "{:?} {:?}",
            a.off,
            b.off
        );
        assert_eq!(b.dith[0], (a.dith[0] + 2) % 4);
    }
}

// checks: PRE-22
#[test]
fn pans_keep_the_projection_within_a_block() {
    for yaw in [0.0, 0.7, -2.1] {
        // the target mid-pixel, then walked right one art pixel at a time for more than a block (over 560 m, so
        // across area midlines too), each step from the start so the rounding to whole ticks never adds up
        let p0 = pose(0.3, yaw);
        let c = compute(&p0, PORTRAIT, 300.0, 380.0);
        let p = moved(p0, c.r[0], c.r[2], (0.5 - c.frac[0]) * c.texel);
        let corner = |c: &Camera| {
            let t = f64::from(c.texel);
            [
                (f64::from(c.left) + dot64(c.r, c.origin_m())) / t,
                (f64::from(c.bottom) + dot64(c.u, c.origin_m())) / t,
            ]
        };
        let mut prev = compute(&p, PORTRAIT, 300.0, 380.0);
        let mut blocks = 0;
        for k in 1..=(BLOCK + 20) {
            let b = compute(&moved(p, c.r[0], c.r[2], k as f32 * c.texel), PORTRAIT, 300.0, 380.0);
            let (pa, pb) = (at(&prev), at(&b));
            assert!(
                (pb[0] - pa[0] - 1.0).abs() < 0.02 && (pb[1] - pa[1]).abs() < 0.02,
                "yaw {yaw}, step {k}"
            );
            // the view snaps by two art pixels at a time, and the grid stays on the world whatever the origin does
            let step = b.snapped[0] - prev.snapped[0];
            assert!(step == 0 || step == 2, "yaw {yaw}, step {k}: {step}");
            let (ga, gb) = (corner(&prev), corner(&b));
            assert!(
                (gb[0] - ga[0] - step as f64).abs() < 1e-3 && (gb[1] - ga[1]).abs() < 1e-3,
                "yaw {yaw}, step {k}"
            );
            if b.view[0] == prev.view[0] - step as i32 {
                // within the block: the very same projection, the viewport over by the snap
                assert_eq!(
                    (b.vp, b.near, b.far, b.origin),
                    (prev.vp, prev.near, prev.far, prev.origin)
                );
                assert_eq!(&b.view[1..], &prev.view[1..]);
            } else {
                assert_eq!(b.view[0], 0, "a new block starts at its corner");
                blocks += 1;
            }
            prev = b;
        }
        assert_eq!(blocks, 1, "yaw {yaw}");
    }
}

// checks: PRE-22
#[test]
fn same_scale_both_orientations() {
    for k in 0..=100 {
        let z = k as f32 / 100.0;
        assert_eq!(texel(z, PORTRAIT), texel(z, LANDSCAPE), "zoom {z}");
    }
    let (a, b) = (
        compute(&pose(0.2, 0.4), PORTRAIT, 300.0, 380.0),
        compute(&pose(0.2, 0.4), LANDSCAPE, 300.0, 380.0),
    );
    assert_eq!(a.texel, b.texel);
    assert_eq!(a.pitch, b.pitch);
}
