//! The camera's pose and its easing (A11.2, A12.2): `kd-ui`'s gestures turned into poses that keep the ground
//! under the fingers, worked out in `f64` metres from the gesture's base target. A drag keeps the target's height;
//! the first turn or zoom after it slides the target along the view's centre line onto the ground, which moves
//! nothing on the screen, so turns and zooms pivot on the ground; a fling glides and a twist turns on, easing to
//! rest (τ 0.3 s).
//!
//! Implements PRE-33, PRE-22 and PRE-03, see A11.2 and A12.2: the land follows the fingers, motion eases to rest, and
//! the zoom runs from the person stop to the camp stop.

use kd_core::geo::{H, Pos, W, wrap};
use kd_render::ART_SCALE;
use kd_render::camera::{ZOOM_IN_REACH, basis, pitch_deg, texel, zoom_of};
use kd_ui::gestures::{EASE_S, Gesture, Similarity};
use kd_view::CameraPose;
use kd_world::area::{Ground, SIDE};

type V3 = [f64; 3];

/// A pose's view: its art pixel, pitch and directions (east, north, up).
#[derive(Clone, Copy, Debug)]
struct Optics {
    texel: f64,
    pitch: f64,
    right: V3,
    up: V3,
    fwd: V3,
}

impl Optics {
    fn of(texel_m: f64, yaw: f64) -> Optics {
        let pitch = pitch_deg(texel_m).to_radians();
        let [right, up, fwd] = basis(yaw, pitch);
        Optics {
            texel: texel_m,
            pitch,
            right,
            up,
            fwd,
        }
    }

    fn pose(p: &CameraPose) -> Optics {
        Optics::of(f64::from(texel(p.zoom)), f64::from(p.yaw))
    }
}

/// The base of a gesture: its pose and the ground points under its fingers, in metres from the pose's target
/// (east, north) and above sea level.
#[derive(Clone, Copy, Debug)]
enum Base {
    Drag {
        pose: CameraPose,
        ground: V3,
    },
    Two {
        epoch: u32,
        pose: CameraPose,
        ground: [V3; 2],
    },
    Zoom {
        pose: CameraPose,
    },
}

/// Motion easing to rest after the fingers lift: a glide of the target and a turn about a ground point.
#[derive(Clone, Copy, Debug)]
struct Ease {
    start_ns: u64,
    pose: CameraPose,
    /// Metres a second, east and north.
    velocity: [f64; 2],
    /// Radians a second, and the window point the turn keeps its ground under, and that ground.
    turn_rate: f64,
    about: [f64; 2],
    ground: V3,
}

/// The camera's pose, the gesture under way and any easing (A11.2).
#[derive(Clone, Debug)]
pub struct Control {
    pub pose: CameraPose,
    base: Option<Base>,
    ease: Option<Ease>,
}

/// A window point in screen pixels from its top-left, in art pixels from its middle, up positive.
fn from_middle(p: [f32; 2], window: [u32; 2]) -> [f64; 2] {
    let s = f64::from(ART_SCALE);
    [
        (f64::from(p[0]) - f64::from(window[0]) / 2.0) / s,
        (f64::from(window[1]) / 2.0 - f64::from(p[1])) / s,
    ]
}

/// The ground's height in metres above sea level at `e`, `n` metres from `target`, between its 1 m points, or none
/// beyond the area.
pub fn ground_height(g: &Ground, target: Pos, e: f64, n: f64) -> Option<f64> {
    let o = g.id.origin();
    let x = f64::from(wrap(target.x - o.x, W)) / 256.0 + e;
    let y = f64::from(wrap(target.y - o.y, H)) / 256.0 - n;
    let top = (SIDE - 1) as f64;
    if !(0.0..=top).contains(&x) || !(0.0..=top).contains(&y) {
        return None;
    }
    let (i, j) = ((x.floor() as usize).min(SIDE - 2), (y.floor() as usize).min(SIDE - 2));
    let (u, v) = (x - i as f64, y - j as f64);
    let h = |a: usize, b: usize| f64::from(g.height_m(a, b));
    let north = h(i, j) + (h(i + 1, j) - h(i, j)) * u;
    let south = h(i, j + 1) + (h(i + 1, j + 1) - h(i, j + 1)) * u;
    Some(north + (south - north) * v)
}

/// The ground under window point `f` (art pixels from the window's middle) for a pose, in metres from its target
/// and above sea level: where the view's ray meets the area's ground, or else the level of the target.
fn ground_under(pose: &CameraPose, f: [f64; 2], g: Option<&Ground>) -> V3 {
    let o = Optics::pose(pose);
    let tz = f64::from(pose.target.z) / 256.0;
    let start = [
        f[0] * o.texel * o.right[0] + f[1] * o.texel * o.up[0],
        f[0] * o.texel * o.right[1] + f[1] * o.texel * o.up[1],
        tz + f[0] * o.texel * o.right[2] + f[1] * o.texel * o.up[2],
    ];
    let at = |l: f64| {
        [
            start[0] + l * o.fwd[0],
            start[1] + l * o.fwd[1],
            start[2] + l * o.fwd[2],
        ]
    };
    if let Some(g) = g {
        // March down the ray through the ground's heights in quarter metres, then halve the step between.
        let lo = f64::from(g.base_dm) / 10.0 - 1.0;
        let hi = lo + f64::from(g.heights.iter().copied().max().unwrap_or(0)) / 10.0 + 2.0;
        let (l0, l1) = ((start[2] - hi) / -o.fwd[2], (start[2] - lo) / -o.fwd[2]);
        let above = |l: f64| {
            let p = at(l);
            ground_height(g, pose.target, p[0], p[1]).map(|h| p[2] - h)
        };
        let mut prev: Option<(f64, f64)> = None;
        let mut l = l0;
        while l <= l1 {
            if let Some(d) = above(l) {
                if let Some((pl, pd)) = prev
                    && pd > 0.0
                    && d <= 0.0
                {
                    let (mut a, mut b) = (pl, l);
                    for _ in 0..30 {
                        let m = (a + b) / 2.0;
                        if above(m).is_some_and(|x| x > 0.0) {
                            a = m
                        } else {
                            b = m
                        }
                    }
                    return at((a + b) / 2.0);
                }
                prev = Some((l, d));
            } else {
                prev = None;
            }
            l += 0.25;
        }
    }
    at((start[2] - tz) / -o.fwd[2])
}

/// The target, in metres from `from`'s target, that puts ground point `p` at window point `f` for a view, the
/// target keeping its height `tz`.
fn place(o: &Optics, p: V3, f: [f64; 2], tz: f64) -> [f64; 2] {
    let (sp, cp) = o.pitch.sin_cos();
    let rh = [o.right[0], o.right[1]];
    let gh = [-o.right[1], o.right[0]];
    let along = f[0] * o.texel;
    let ahead = (f[1] * o.texel - (p[2] - tz) * cp) / sp;
    [
        p[0] - along * rh[0] - ahead * gh[0],
        p[1] - along * rh[1] - ahead * gh[1],
    ]
}

/// A target moved by `d` metres east and north, its height kept.
fn moved(t: Pos, d: [f64; 2]) -> Pos {
    let x = i64::from(t.x) + (d[0] * 256.0).round() as i64;
    let y = i64::from(t.y) - (d[1] * 256.0).round() as i64;
    Pos {
        x: x.rem_euclid(i64::from(W)) as i32,
        y: y.rem_euclid(i64::from(H)) as i32,
        z: t.z,
    }
}

/// The pose that puts ground points `g` (metres from `base`'s target) at window points `f`, by art pixel, heading and
/// target, the art pixel held within the zoom in reach (A12.2): the fingers' two points fix all four.
fn solve_two(base: &CameraPose, g: [V3; 2], f: [[f64; 2]; 2]) -> CameraPose {
    let d = [g[1][0] - g[0][0], g[1][1] - g[0][1], g[1][2] - g[0][2]];
    let e = [f[1][0] - f[0][0], f[1][1] - f[0][1]];
    let dh = (d[0] * d[0] + d[1] * d[1]).sqrt();
    if dh < 1e-6 || (e[0] * e[0] + e[1] * e[1]).sqrt() < 1e-6 {
        return *base;
    }
    // The art pixel at which the two points lie as far apart as the fingers: one root, found by halving.
    let split = |t: f64| {
        let p = pitch_deg(t).to_radians();
        (e[0] * t, (e[1] * t - d[2] * p.cos()) / p.sin())
    };
    let gap = |t: f64| {
        let (a, b) = split(t);
        a * a + b * b - dh * dh
    };
    let (mut lo, mut hi) = (f64::from(texel(ZOOM_IN_REACH[0])), f64::from(texel(ZOOM_IN_REACH[1])));
    let t = if gap(lo) >= 0.0 {
        lo
    } else if gap(hi) <= 0.0 {
        hi
    } else {
        for _ in 0..60 {
            let m = (lo * hi).sqrt();
            if gap(m) < 0.0 { lo = m } else { hi = m }
        }
        (lo * hi).sqrt()
    };
    let (a, b) = split(t);
    let yaw = d[1].atan2(d[0]) - b.atan2(a);
    let o = Optics::of(t, yaw);
    let tz = f64::from(base.target.z) / 256.0;
    let at = place(&o, g[0], f[0], tz);
    CameraPose {
        target: moved(base.target, at),
        yaw: yaw.rem_euclid(std::f64::consts::TAU) as f32,
        zoom: zoom_of(t as f32),
    }
}

impl Control {
    pub fn new(pose: CameraPose) -> Control {
        Control {
            pose,
            base: None,
            ease: None,
        }
    }

    /// The pose with its target slid along the view's centre line onto the ground: the picture does not move.
    fn on_ground(&self, g: Option<&Ground>) -> CameraPose {
        let p = ground_under(&self.pose, [0.0, 0.0], g);
        CameraPose {
            target: Pos {
                z: (p[2] * 256.0).round() as i32,
                ..moved(self.pose.target, [p[0], p[1]])
            },
            ..self.pose
        }
    }

    /// What the fingers ask, on a window of `window` screen pixels over the ground `g`.
    pub fn gesture(&mut self, gesture: Gesture, window: [u32; 2], g: Option<&Ground>, now_ns: u64) {
        self.ease = None;
        match gesture {
            Gesture::Drag { from, to } => {
                let (pose, ground) = match self.base {
                    Some(Base::Drag { pose, ground }) => (pose, ground),
                    _ => {
                        let ground = ground_under(&self.pose, from_middle(from, window), g);
                        self.base = Some(Base::Drag {
                            pose: self.pose,
                            ground,
                        });
                        (self.pose, ground)
                    }
                };
                let o = Optics::pose(&pose);
                let at = place(&o, ground, from_middle(to, window), f64::from(pose.target.z) / 256.0);
                self.pose = CameraPose {
                    target: moved(pose.target, at),
                    ..pose
                };
            }
            Gesture::Two {
                epoch,
                from,
                to,
                scale,
                turn,
            } => {
                let (pose, ground) = match self.base {
                    Some(Base::Two { epoch: e, pose, ground }) if e == epoch => (pose, ground),
                    _ => {
                        let pose = self.on_ground(g);
                        let ground = from.map(|p| ground_under(&pose, from_middle(p, window), g));
                        self.base = Some(Base::Two { epoch, pose, ground });
                        (pose, ground)
                    }
                };
                let s = Similarity::between(from, to, scale, turn);
                let want = from.map(|p| from_middle(s.apply(p), window));
                self.pose = solve_two(&pose, ground, want);
            }
            Gesture::Zoom { delta } => {
                let pose = match self.base {
                    Some(Base::Zoom { pose }) => pose,
                    _ => {
                        let pose = self.on_ground(g);
                        self.base = Some(Base::Zoom { pose });
                        pose
                    }
                };
                let [lo, hi] = ZOOM_IN_REACH;
                self.pose = CameraPose {
                    zoom: (pose.zoom + delta).clamp(lo, hi),
                    ..pose
                };
            }
            Gesture::Release {
                velocity,
                turn_rate,
                about,
            } => {
                self.base = None;
                let o = Optics::pose(&self.pose);
                // The land moved with the finger, so the target glides the other way.
                let va = [
                    f64::from(velocity[0]) / f64::from(ART_SCALE),
                    -f64::from(velocity[1]) / f64::from(ART_SCALE),
                ];
                let gh = [-o.right[1], o.right[0]];
                let ahead = va[1] * o.texel / o.pitch.sin();
                let v = [
                    -(va[0] * o.texel * o.right[0] + ahead * gh[0]),
                    -(va[0] * o.texel * o.right[1] + ahead * gh[1]),
                ];
                let about = from_middle(about, window);
                let pose = if turn_rate != 0.0 { self.on_ground(g) } else { self.pose };
                let ground = ground_under(&pose, about, g);
                if v[0].hypot(v[1]) > 0.01 || turn_rate.abs() > 0.01 {
                    self.ease = Some(Ease {
                        start_ns: now_ns,
                        pose,
                        velocity: v,
                        turn_rate: f64::from(turn_rate),
                        about,
                        ground,
                    });
                }
                self.pose = pose;
            }
            Gesture::Tap { .. } => self.base = None,
        }
    }

    /// A finger touched down: any easing stops where it is.
    pub fn hold(&mut self) {
        self.ease = None;
    }

    /// Sets the pose outright, ending any gesture and easing (a test hook).
    pub fn set(&mut self, pose: CameraPose) {
        *self = Control::new(pose);
    }

    /// Moves the easing on to `now_ns`; whether anything is still easing.
    pub fn tick(&mut self, now_ns: u64) -> bool {
        let Some(e) = self.ease else { return false };
        let s = now_ns.saturating_sub(e.start_ns) as f64 / 1e9;
        let tau = f64::from(EASE_S);
        let gone = tau * (1.0 - (-s / tau).exp());
        let mut pose = CameraPose {
            target: moved(e.pose.target, [e.velocity[0] * gone, e.velocity[1] * gone]),
            ..e.pose
        };
        if e.turn_rate != 0.0 {
            // The land turns on about the ground the fingers left, which stays where they left it on the screen.
            let yaw = f64::from(e.pose.yaw) + e.turn_rate * gone;
            let o = Optics::of(f64::from(texel(e.pose.zoom)), yaw);
            let at = place(&o, e.ground, e.about, f64::from(e.pose.target.z) / 256.0);
            pose = CameraPose {
                target: moved(e.pose.target, at),
                yaw: yaw.rem_euclid(std::f64::consts::TAU) as f32,
                zoom: e.pose.zoom,
            };
        }
        self.pose = pose;
        if s > 5.0 * tau {
            self.ease = None;
        }
        true
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_world::area::demo;

    const WINDOW: [u32; 2] = [1080, 2404];

    /// Where a ground point (metres from `ref_t`'s target, and up) shows in the window for a pose, in screen
    /// pixels from the window's top-left.
    fn shows_at(pose: &CameraPose, reference: Pos, p: V3) -> [f64; 2] {
        let o = Optics::pose(pose);
        let dt = [
            f64::from(wrap(pose.target.x - reference.x, W)) / 256.0,
            -f64::from(wrap(pose.target.y - reference.y, H)) / 256.0,
        ];
        let q = [p[0] - dt[0], p[1] - dt[1], p[2] - f64::from(pose.target.z) / 256.0];
        let a = (q[0] * o.right[0] + q[1] * o.right[1]) / o.texel;
        let b = (q[0] * o.up[0] + q[1] * o.up[1] + q[2] * o.up[2]) / o.texel;
        let s = f64::from(ART_SCALE);
        [f64::from(WINDOW[0]) / 2.0 + a * s, f64::from(WINDOW[1]) / 2.0 - b * s]
    }

    fn demo_start() -> (Ground, CameraPose) {
        let g = demo::make(demo::SEED);
        let o = g.id.origin();
        let pose = CameraPose {
            target: Pos {
                x: o.x + 100 * 256,
                y: o.y + 140 * 256,
                z: (g.height_m(100, 140) * 256.0) as i32,
            },
            yaw: 0.3,
            zoom: 0.22,
        };
        (g, pose)
    }

    // checks: PRE-33 PRE-22
    #[test]
    fn land_stays_under_the_fingers() {
        let (g, start) = demo_start();
        let mut c = Control::new(start);
        // A drag across the cliff's slope: the ground under the finger stays under it, to a hundredth of a pixel.
        let from = [500.0, 1300.0];
        let ground = ground_under(&start, from_middle(from, WINDOW), Some(&g));
        for k in 1..=20 {
            let to = [500.0 + 13.0 * k as f32, 1300.0 - 21.0 * k as f32];
            c.gesture(Gesture::Drag { from, to }, WINDOW, Some(&g), 0);
            let at = shows_at(&c.pose, start.target, ground);
            assert!(
                (at[0] - f64::from(to[0])).abs() < 0.05 && (at[1] - f64::from(to[1])).abs() < 0.05,
                "{at:?} {to:?}"
            );
            assert_eq!(c.pose.target.z, start.target.z, "a drag keeps the target's height");
        }
        c.gesture(
            Gesture::Release {
                velocity: [0.0; 2],
                turn_rate: 0.0,
                about: [0.0; 2],
            },
            WINDOW,
            Some(&g),
            0,
        );
        // A pinch and twist together: both fingers' ground stays under them while the zoom and the pitch change.
        let pose = c.pose;
        let from = [[300.0, 1000.0], [800.0, 1500.0]];
        let base = c.on_ground(Some(&g));
        let ground = from.map(|p| ground_under(&base, from_middle(p, WINDOW), Some(&g)));
        for k in 1..=20 {
            let f = k as f32 / 20.0;
            let to = [
                [300.0 - 120.0 * f, 1000.0 - 40.0 * f],
                [800.0 + 60.0 * f, 1500.0 + 200.0 * f],
            ];
            c.gesture(
                Gesture::Two {
                    epoch: 1,
                    from,
                    to,
                    scale: true,
                    turn: true,
                },
                WINDOW,
                Some(&g),
                0,
            );
            for i in 0..2 {
                let at = shows_at(&c.pose, base.target, ground[i]);
                assert!(
                    (at[0] - f64::from(to[i][0])).abs() < 0.5 && (at[1] - f64::from(to[i][1])).abs() < 0.5,
                    "step {k}, finger {i}: {at:?} {:?}",
                    to[i]
                );
            }
        }
        // The heading turns the way the fingers did (y down on the screen), as the twist's easing assumes.
        let last = [[180.0, 960.0], [860.0, 1700.0]];
        let screen_turn = Similarity::between(from, last, true, true).angle;
        let mut turned = f64::from(c.pose.yaw) - f64::from(pose.yaw);
        turned = (turned + std::f64::consts::PI).rem_euclid(std::f64::consts::TAU) - std::f64::consts::PI;
        assert!(
            turned * f64::from(screen_turn) > 0.0 && (turned - f64::from(screen_turn)).abs() < 0.02,
            "{turned} {screen_turn}"
        );
        assert!(
            c.pose.zoom < pose.zoom,
            "spread fingers zoom in: {} {}",
            c.pose.zoom,
            pose.zoom
        );
        assert_ne!(c.pose.yaw, pose.yaw);
    }

    // checks: PRE-22 PRE-03
    #[test]
    fn turns_and_zooms_pivot_on_the_ground() {
        let (g, start) = demo_start();
        let mut c = Control::new(start);
        // The target lifted 5 m above the ground by drags keeping its height: the first zoom slides it onto the
        // ground along the view's centre line, which moves nothing on the screen.
        c.pose.target.z += 5 * 256;
        let lifted = c.pose;
        let middle = ground_under(&lifted, [0.0, 0.0], Some(&g));
        let before = shows_at(&lifted, lifted.target, middle);
        c.gesture(Gesture::Zoom { delta: 0.0 }, WINDOW, Some(&g), 0);
        let after = shows_at(&c.pose, lifted.target, middle);
        assert!((before[0] - after[0]).abs() < 0.05 && (before[1] - after[1]).abs() < 0.05);
        let h = ground_height(&g, c.pose.target, 0.0, 0.0).unwrap();
        assert!((f64::from(c.pose.target.z) / 256.0 - h).abs() < 0.02, "on the ground");
        // Zooming then keeps that ground in the middle of the screen.
        c.gesture(Gesture::Zoom { delta: -0.15 }, WINDOW, Some(&g), 0);
        assert!((c.pose.zoom - (start.zoom - 0.15)).abs() < 1e-6);
        let mid = shows_at(&c.pose, lifted.target, middle);
        // Within the target's ticks of 1/256 m, a tenth of a screen pixel at this zoom.
        assert!(
            (mid[0] - 540.0).abs() < 0.12 && (mid[1] - 1202.0).abs() < 0.12,
            "{mid:?}"
        );
        // The zoom stays within the stops in reach.
        c.gesture(Gesture::Zoom { delta: 2.0 }, WINDOW, Some(&g), 0);
        assert_eq!(c.pose.zoom, ZOOM_IN_REACH[1]);
    }

    // checks: PRE-22
    #[test]
    fn flings_and_twists_ease_to_rest() {
        let (g, start) = demo_start();
        let mut c = Control::new(start);
        c.gesture(
            Gesture::Release {
                velocity: [800.0, 0.0],
                turn_rate: 0.0,
                about: [540.0, 1202.0],
            },
            WINDOW,
            Some(&g),
            0,
        );
        // The glide goes on for about τ's worth of the speed, each frame less than the last, then stops.
        let mut last = start.target;
        let mut steps = Vec::new();
        for f in 1..=120 {
            assert!(c.tick(f * 16_666_667) || f > 90);
            steps
                .push(f64::from(wrap(c.pose.target.x - last.x, W)).hypot(f64::from(wrap(c.pose.target.y - last.y, H))));
            last = c.pose.target;
        }
        assert!(steps.windows(2).take(60).all(|w| w[1] <= w[0] + 1.0), "{steps:?}");
        assert!(!c.tick(10_000_000_000), "at rest after 5 τ");
        let o = Optics::pose(&start);
        let glide = f64::from(wrap(c.pose.target.x - start.target.x, W))
            .hypot(f64::from(wrap(c.pose.target.y - start.target.y, H)))
            / 256.0;
        let expect = 800.0 / 4.0 * o.texel * 0.3;
        assert!((glide - expect).abs() < expect * 0.02, "{glide} {expect}");
        // A twist turns on by its rate times τ, its ground point staying put.
        let mut c = Control::new(start);
        c.gesture(
            Gesture::Release {
                velocity: [0.0; 2],
                turn_rate: 1.0,
                about: [700.0, 900.0],
            },
            WINDOW,
            Some(&g),
            0,
        );
        let pivot = ground_under(&c.pose, from_middle([700.0, 900.0], WINDOW), Some(&g));
        let base = c.pose;
        c.tick(3_000_000_000);
        let turned = (f64::from(c.pose.yaw) - f64::from(base.yaw)).rem_euclid(std::f64::consts::TAU);
        assert!((turned - 0.3).abs() < 1e-3, "{turned}");
        let at = shows_at(&c.pose, base.target, pivot);
        assert!((at[0] - 700.0).abs() < 0.05 && (at[1] - 900.0).abs() < 0.05, "{at:?}");
    }
}
