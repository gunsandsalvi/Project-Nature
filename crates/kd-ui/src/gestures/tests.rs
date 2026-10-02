use super::*;

const MS: u64 = 1_000_000;

fn ev(kind: InputKind, pointer: i32, x: f32, y: f32, t_ms: u64) -> InputEvent {
    InputEvent {
        kind,
        pointer,
        x,
        y,
        t_ns: t_ms * MS,
    }
}

/// Runs a trace through a fresh recogniser on a 1080 × 2404 screen at 4 screen pixels a UI pixel.
fn run(trace: &[InputEvent]) -> Vec<CameraCmd> {
    let mut g = Gestures::new(4, 2404);
    let mut out = Vec::new();
    for e in trace {
        g.input(e, &mut out);
    }
    out
}

fn kinds(cmds: &[CameraCmd]) -> String {
    let mut k: Vec<&str> = cmds
        .iter()
        .map(|c| match c {
            CameraCmd::Drag { .. } => "drag",
            CameraCmd::Fling { .. } => "fling",
            CameraCmd::Turn { .. } => "turn",
            CameraCmd::TurnFling { .. } => "turnfling",
            CameraCmd::Zoom { .. } => "zoom",
            CameraCmd::Tap { .. } => "tap",
        })
        .collect();
    k.dedup();
    k.join(" ")
}

fn zoom_sum(cmds: &[CameraCmd]) -> f32 {
    cmds.iter()
        .map(|c| if let CameraCmd::Zoom { dz } = c { *dz } else { 0.0 })
        .sum()
}

fn turn_sum(cmds: &[CameraCmd]) -> f32 {
    cmds.iter()
        .map(|c| if let CameraCmd::Turn { rad } = c { *rad } else { 0.0 })
        .sum()
}

/// Two fingers at `r` pixels either side of (540, 1200), at angle `a`.
fn pair(t: u64, kind: InputKind, a: f32, r: f32) -> [InputEvent; 2] {
    let (c, s) = (a.cos() * r, a.sin() * r);
    [
        ev(kind, 1, 540.0 + c, 1200.0 + s, t),
        ev(kind, 2, 540.0 - c, 1200.0 - s, t),
    ]
}

// checks: PRE-33
#[test]
fn scripted_traces() {
    use InputKind::{Down, Move, Up};
    // a tap: lifted within 300 ms and 6 UI pixels
    let tap = run(&[
        ev(Down, 1, 500.0, 900.0, 0),
        ev(Move, 1, 503.0, 901.0, 40),
        ev(Up, 1, 503.0, 901.0, 90),
    ]);
    assert_eq!(kinds(&tap), "tap");

    // a drag: the ground follows the finger exactly, then glides on
    let mut tr = vec![ev(Down, 1, 500.0, 900.0, 0)];
    for k in 1..=10 {
        tr.push(ev(Move, 1, 500.0 + 12.0 * k as f32, 900.0 + 6.0 * k as f32, 16 * k));
    }
    tr.push(ev(Up, 1, 620.0, 960.0, 168));
    let drag = run(&tr);
    assert_eq!(kinds(&drag), "drag fling");
    let (dx, dy) = drag.iter().fold((0.0, 0.0), |(x, y), c| match c {
        CameraCmd::Drag { dx, dy } => (x + dx, y + dy),
        _ => (x, y),
    });
    assert!((dx - 120.0).abs() < 1e-3 && (dy - 60.0).abs() < 1e-3, "{dx} {dy}");

    // a still finger held past a tap's time is nothing
    assert_eq!(
        kinds(&run(&[
            ev(Down, 1, 500.0, 900.0, 0),
            ev(Move, 1, 504.0, 900.0, 200),
            ev(Up, 1, 504.0, 900.0, 500)
        ])),
        ""
    );

    // a twist of 40 degrees at a steady distance turns, and only turns
    let mut tr = pair(0, Down, 0.0, 300.0).to_vec();
    for k in 1..=10 {
        tr.extend(pair(16 * k, Move, (40.0f32).to_radians() * k as f32 / 10.0, 300.0));
    }
    tr.extend(pair(176, Up, (40.0f32).to_radians(), 300.0));
    let twist = run(&tr);
    assert_eq!(kinds(&twist), "turn turnfling");
    let t = turn_sum(&twist);
    assert!(t > (30.0f32).to_radians() && t < (40.1f32).to_radians(), "turned {t}");

    // a pinch to twice the distance, without turning, zooms in by ln 2 × 0.16 past its 6% start, and only zooms
    let mut tr = pair(0, Down, 1.2, 150.0).to_vec();
    for k in 1..=10 {
        tr.extend(pair(16 * k, Move, 1.2, 150.0 + 15.0 * k as f32));
    }
    tr.extend(pair(176, Up, 1.2, 300.0));
    let pinch = run(&tr);
    assert_eq!(kinds(&pinch), "zoom");
    let z = zoom_sum(&pinch);
    assert!(z < -0.08 && z > -(2.0f32).ln() * ZOOM_PER_LN - 1e-4, "zoomed {z}");

    // a double tap, then drag down a quarter of the screen: zooms in by 0.2, with no drag
    let mut tr = vec![
        ev(Down, 1, 540.0, 1200.0, 0),
        ev(Up, 1, 540.0, 1200.0, 60),
        ev(Down, 1, 545.0, 1205.0, 180),
    ];
    for k in 1..=10 {
        tr.push(ev(Move, 1, 545.0, 1205.0 + 60.1 * k as f32, 180 + 16 * k));
    }
    tr.push(ev(Up, 1, 545.0, 1806.0, 360));
    let dd = run(&tr);
    assert_eq!(kinds(&dd), "tap zoom");
    let z = zoom_sum(&dd);
    assert!((z + 0.2).abs() < 0.01, "zoomed {z}");

    // the same second touch too late is a drag that glides on, not a zoom
    let mut late = tr.clone();
    for e in late.iter_mut().skip(2) {
        e.t_ns += 400 * MS;
    }
    assert_eq!(kinds(&run(&late)), "tap drag fling");
}
