use super::*;

/// Positions near both seams and in between.
const SPOTS: [Pos; 6] = [
    Pos { x: 0, y: 0, z: 0 },
    Pos {
        x: W - 1,
        y: H - 1,
        z: 5,
    },
    Pos {
        x: W / 2,
        y: H / 2,
        z: -3,
    },
    Pos { x: 10, y: H - 10, z: 0 },
    Pos {
        x: W - 300,
        y: 77,
        z: 0,
    },
    Pos {
        x: 123_456_789,
        y: 98_765_432,
        z: 12,
    },
];

// checks: WLD-01
#[test]
fn wrap_round_trips() {
    // whole ticks, under half the world each way, so the round trip is exact; from the spots near the seams they cross
    let offsets = [
        (0.0, 0.0),
        (1.0, -1.0),
        (-0.5, 0.25),
        (300.0, -2_000.0),
        (-1_000_000.0, 500_000.0),
        (1_023_999.0, -511_999.5),
    ];
    for p in SPOTS {
        for (x, y) in offsets {
            let v = Vec2 { x, y };
            let q = offset(p, v);
            assert!((0..W).contains(&q.x) && (0..H).contains(&q.y), "{p:?} + {v:?} = {q:?}");
            assert_eq!(delta(p, q), v, "{p:?} + {v:?}");
            assert_eq!(q.z, p.z);
        }
    }
    // one metre east of half a metre before the east-west seam lands half a metre past it
    assert_eq!(offset(Pos { x: W - 128, y: 0, z: 0 }, Vec2 { x: 1.0, y: 0.0 }).x, 128);
    // one metre north of a quarter metre south of y = 0 crosses the seam at the poles
    assert_eq!(offset(Pos { x: 0, y: 64, z: 0 }, Vec2 { x: 0.0, y: -1.0 }).y, H - 192);
}

// checks: WLD-01
#[test]
fn delta_symmetric() {
    let mut s = 0x9E37_79B9_7F4A_7C15_u64;
    let mut next = |m: i32| {
        s = s
            .wrapping_mul(6_364_136_223_846_793_005)
            .wrapping_add(1_442_695_040_888_963_407);
        ((s >> 33) % m as u64) as i32
    };
    for _ in 0..10_000 {
        let a = Pos {
            x: next(W),
            y: next(H),
            z: 0,
        };
        let b = Pos {
            x: next(W),
            y: next(H),
            z: 0,
        };
        let (ab, ba) = (delta(a, b), delta(b, a));
        // the short way back is the same way reversed, except exactly half way round, where both ways are as short
        if (b.x - a.x).rem_euclid(W) != W / 2 {
            assert_eq!(ab.x, -ba.x, "{a:?} {b:?}");
        }
        if (b.y - a.y).rem_euclid(H) != H / 2 {
            assert_eq!(ab.y, -ba.y, "{a:?} {b:?}");
        }
        assert_eq!(dist(a, b), dist(b, a));
        assert!(ab.x.abs() <= 1_024_000.0 && ab.y.abs() <= 512_000.0, "{ab:?}");
    }
    for p in SPOTS {
        assert_eq!(delta(p, p), Vec2::default());
    }
}

// checks: WLD-01
#[test]
fn areas_hold_their_corners() {
    for a in [
        AreaId(0),
        AreaId(7_999),
        AreaId(8_000),
        AreaId(12_345_678),
        AreaId(31_999_999),
    ] {
        let o = a.origin();
        assert_eq!(AreaId::of(o), a);
        assert_eq!(
            AreaId::of(Pos {
                x: o.x + 65_535,
                y: o.y + 65_535,
                z: 0
            }),
            a
        );
    }
    assert_eq!(
        AreaId::of(Pos {
            x: W - 1,
            y: H - 1,
            z: 0
        }),
        AreaId(3_999 * 8_000 + 7_999)
    );
}
