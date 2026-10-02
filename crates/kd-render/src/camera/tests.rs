use super::*;

/// The phone's art target in portrait (1080 × 2404 at 4) and turned.
const PORTRAIT: ArtSize = ArtSize {
    wf: 272,
    hf: 603,
    wd: 1080,
    hd: 2404,
    s: 4,
};
const LANDSCAPE: ArtSize = ArtSize {
    wf: 603,
    hf: 272,
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

/// The view's fraction of an art pixel along right and up, from the upscale's shift.
fn frac(c: &Camera) -> (f32, f32) {
    let a = PORTRAIT;
    (
        c.off[0] - (a.wf / 2) as f32 + a.wd as f32 / (2.0 * a.s as f32),
        c.off[1] - (a.hf / 2) as f32 + a.hd as f32 / (2.0 * a.s as f32),
    )
}

/// `p` moved `d` metres along the ground direction (`gx`, `gz`) in GPU axes (x east, z south).
fn moved(p: CameraPose, gx: f32, gz: f32, d: f32) -> CameraPose {
    CameraPose {
        target: geo::offset(p.target, geo::Vec2 { x: gx * d, y: gz * d }),
        ..p
    }
}

// checks: PRE-22
#[test]
fn snap_moves_whole_pixels() {
    for yaw in [0.0, 0.7, -2.1] {
        let mut p = pose(0.3, yaw);
        // centre the target in its art pixel first, so rounding to whole ticks cannot cross a pixel's edge
        let c = compute(&p, PORTRAIT, 300.0, 380.0);
        let (fx, fy) = frac(&c);
        p = moved(p, c.r[0], c.r[2], (0.5 - fx) * c.texel);
        let hl = (c.f[0] * c.f[0] + c.f[2] * c.f[2]).sqrt();
        let (hx, hz) = (c.f[0] / hl, c.f[2] / hl);
        let up_per_m = c.u[0] * hx + c.u[2] * hz;
        p = moved(p, hx, hz, (0.5 - fy) * c.texel / up_per_m);
        let a = compute(&p, PORTRAIT, 300.0, 380.0);
        // one art pixel along the view's right moves the snapped view by one and leaves the shift alone
        let b = compute(&moved(p, a.r[0], a.r[2], a.texel), PORTRAIT, 300.0, 380.0);
        assert_eq!(b.snapped[0], a.snapped[0] + 1, "yaw {yaw}");
        assert_eq!(b.snapped[1], a.snapped[1], "yaw {yaw}");
        assert!(
            (b.off[0] - a.off[0]).abs() < 0.01 && (b.off[1] - a.off[1]).abs() < 0.01,
            "{:?} {:?}",
            a.off,
            b.off
        );
        assert_eq!(b.dith[0], (a.dith[0] + 1) % 4);
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
