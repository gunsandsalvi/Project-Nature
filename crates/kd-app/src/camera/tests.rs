use super::*;

const SIZE: ArtSize = ArtSize {
    wf: 106,
    hf: 218,
    wd: 412,
    hd: 860,
    s: 4,
};

fn ground() -> GroundGrid {
    let side = 257;
    GroundGrid {
        side,
        heights_m: vec![300.0; side * side],
        surface: vec![0; 256 * 256],
        ..GroundGrid::default()
    }
}

fn at(c: &CameraCtl, g: &GroundGrid) -> (f32, f32) {
    let d = geo::delta(g.origin, c.pose.target);
    (d.x, d.y)
}

// checks: PRE-33 PRE-22
#[test]
fn drag_follows_the_finger_and_glides_to_rest() {
    let g = ground();
    // looking north (yaw 0): right on the screen is east, up the screen is north
    let mut c = CameraCtl::new(crate::valley::pose_at(&g, 128.0, 128.0, 0.0, 0.2));
    let texel = rcam::texel(0.2, SIZE);
    c.apply(CameraCmd::Drag { dx: 40.0, dy: 0.0 }, SIZE, &g);
    let (x, y) = at(&c, &g);
    // the ground moved 10 art pixels right under the finger, so the target moved 10 west
    assert!(
        (x - (128.0 - 10.0 * texel)).abs() < 0.01 && (y - 128.0).abs() < 0.01,
        "{x} {y}"
    );
    // a fling glides on and comes to rest
    c.apply(CameraCmd::Fling { vx: 400.0, vy: 0.0 }, SIZE, &g);
    assert!(c.step(1.0 / 60.0, &g));
    let (x1, _) = at(&c, &g);
    assert!(x1 < x, "glides on");
    let mut n = 0;
    while c.step(1.0 / 60.0, &g) {
        n += 1;
        assert!(n < 1_000, "never comes to rest");
    }
    // the glide covers about speed × τ
    let (x2, _) = at(&c, &g);
    let want = 400.0 / 4.0 * texel * FLING_TAU_S;
    assert!(
        ((x - x2) - want).abs() < want * 0.1,
        "glided {}, want about {want}",
        x - x2
    );
    // the target stays over the ground
    c.apply(CameraCmd::Drag { dx: -1.0e6, dy: 1.0e6 }, SIZE, &g);
    let (x3, y3) = at(&c, &g);
    assert!((0.0..=256.0).contains(&x3) && (0.0..=256.0).contains(&y3), "{x3} {y3}");
}

// checks: PRE-22 PRE-33
#[test]
fn turns_ease_and_zoom_keeps_its_range() {
    let g = ground();
    let mut c = CameraCtl::new(crate::valley::pose_at(&g, 128.0, 128.0, 0.0, 0.2));
    c.apply(CameraCmd::Turn { rad: 0.3 }, SIZE, &g);
    assert!((c.pose.yaw - 0.3).abs() < 1e-6);
    c.apply(CameraCmd::TurnFling { rad_s: 2.0 }, SIZE, &g);
    let mut n = 0;
    while c.step(1.0 / 60.0, &g) {
        n += 1;
        assert!(n < 1_000);
    }
    assert!(
        (c.pose.yaw - (0.3 + 2.0 * FLING_TAU_S)).abs() < 0.05,
        "eased to {}",
        c.pose.yaw
    );
    c.apply(CameraCmd::Zoom { dz: 5.0 }, SIZE, &g);
    assert_eq!(c.pose.zoom, ZOOM_MAX);
    c.apply(CameraCmd::Zoom { dz: -5.0 }, SIZE, &g);
    assert_eq!(c.pose.zoom, ZOOM_MIN);
}

/// Ground rising 0.2 m a metre to the east.
fn slope() -> GroundGrid {
    let side = 257;
    GroundGrid {
        side,
        heights_m: (0..side * side).map(|k| 300.0 + 0.2 * (k % side) as f32).collect(),
        surface: vec![0; 256 * 256],
        ..GroundGrid::default()
    }
}

// checks: PRE-22 PRE-33
#[test]
fn drags_keep_the_height_and_turns_pivot_on_the_ground() {
    let g = slope();
    let mut c = CameraCtl::new(crate::valley::pose_at(&g, 128.0, 128.0, 0.5, 0.2));
    let z0 = c.pose.target.z;
    // a drag across the slope keeps the target's height, so the picture only slides (A11.2's test)
    c.apply(CameraCmd::Drag { dx: -300.0, dy: 40.0 }, SIZE, &g);
    let (x, y) = at(&c, &g);
    assert_eq!(c.pose.target.z, z0);
    let off_ground = crate::valley::height_at(&g, x, y) - z0 as f32 / 256.0;
    assert!(off_ground.abs() > 2.0, "the drag crossed the slope: {off_ground}");
    // the first turn slides the target along the view's centre line onto the ground, and the picture stays put
    let before = rcam::compute(&c.pose, SIZE, 290.0, 360.0);
    c.apply(CameraCmd::Turn { rad: 0.0 }, SIZE, &g);
    let after = rcam::compute(&c.pose, SIZE, 290.0, 360.0);
    let (x, y) = at(&c, &g);
    let z = c.pose.target.z as f32 / 256.0;
    assert!(
        (z - crate::valley::height_at(&g, x, y)).abs() < 0.01,
        "{z} at ({x}, {y})"
    );
    let place = |k: &rcam::Camera, i: usize| k.snapped[i] as f32 + k.off[i];
    for i in 0..2 {
        let moved = place(&after, i) - place(&before, i);
        assert!(moved.abs() < 0.02, "the picture moved {moved} art pixels");
    }
    // later turns spin round that ground point, which stays in the middle of the screen
    let pivot = c.pose.target;
    c.apply(CameraCmd::Turn { rad: 0.8 }, SIZE, &g);
    c.apply(CameraCmd::Zoom { dz: -0.05 }, SIZE, &g);
    assert_eq!(c.pose.target, pivot);
}
