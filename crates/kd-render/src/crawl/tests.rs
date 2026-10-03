use super::*;
use crate::camera::{ArtSize, compute};
use kd_core::geo::{self, Pos, Vec2};

const SIZE: ArtSize = ArtSize {
    wf: 41,
    hf: 61,
    wd: 152,
    hd: 232,
    s: 4,
};

fn pose(dx_m: f32) -> CameraPose {
    let t = Pos {
        x: 300_000_000,
        y: 100_000_000,
        z: 340 * 256,
    };
    CameraPose {
        target: geo::offset(t, Vec2 { x: dx_m, y: 0.0 }),
        yaw: 0.0,
        zoom: 0.25,
    }
}

/// A frame of the given camera whose colours are a pattern fixed to the art grid's columns, shifted by `shift`.
fn frame(cam: Camera, shift: i32) -> Capture {
    let (w, h) = (SIZE.wf, SIZE.hf);
    let col = (0..w * h)
        .map(|i| ((i % w) as i32 + shift).rem_euclid(5) as u32)
        .collect();
    Capture {
        w,
        h,
        col,
        dep16: vec![30_000; (w * h) as usize],
        cam,
    }
}

// checks: PRE-22
#[test]
fn counts_crawl_but_not_whole_moves() {
    let a = compute(&pose(0.0), SIZE, 300.0, 380.0);
    // the same picture twice: nothing changed
    let n = pair(&compose(&frame(a, 0)), &compose(&frame(a, 0)));
    assert!(n.valid > 0 && n.elig == n.valid);
    assert_eq!((n.changed, n.crawl), (0, 0));
    // a pixel changes while nothing moved: that is crawl
    let mut b = frame(a, 0);
    b.col[(30 * SIZE.wf + 20) as usize] ^= 0xff;
    let n = pair(&compose(&frame(a, 0)), &compose(&b));
    assert_eq!((n.changed, n.crawl), (1, 1));
    // a pan of two art pixels (rounded up to whole ticks of 1/256 m), one snap of the view, moves every pixel
    // whole: changes, but no crawl
    let two = compute(&pose((2.0 * a.texel * 256.0).ceil() / 256.0), SIZE, 300.0, 380.0);
    assert_eq!(two.snapped[0], a.snapped[0] + 2);
    let n = pair(&compose(&frame(a, 0)), &compose(&frame(two, 2)));
    assert!(n.changed > 0, "{n:?}");
    assert_eq!(n.crawl, 0, "{n:?}");
    // nothing drawn, nothing counted
    let mut empty = frame(a, 0);
    empty.dep16 = vec![u16::MAX; (SIZE.wf * SIZE.hf) as usize];
    assert_eq!(pair(&compose(&empty), &compose(&frame(a, 0))), PairCount::default());
}
