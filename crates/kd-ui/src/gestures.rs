//! The camera's gestures (A12.2): A12.2's recognisers turning raw touches into what the fingers ask of the land, in
//! screen pixels from the screen's top-left. Each gesture is measured from the fingers' places when it was taken
//! up, so the land under them follows them exactly, with nothing gathered from move to move; `kd-app` turns it into
//! the camera's pose (A11.2).
//!
//! Implements PRE-33 and PRE-22, see A12.2: drag, pinch, twist and double-tap drag follow the fingers, and a fling
//! or a twist eases to rest.

use kd_view::{InputEvent, InputKind};

use crate::{TAP_NS, TAP_SLOP};

/// A drag is taken up when one finger moves over this many UI pixels (A12.2).
pub const DRAG_SLOP: f32 = 6.0;
/// A second touch within this long and this many UI pixels of a tap starts a double-tap drag.
pub const DOUBLE_TAP_NS: u64 = 300_000_000;
pub const DOUBLE_TAP_SLOP: f32 = 12.0;
/// A second finger within this long of the first, before any claim, starts the two-finger gestures.
pub const SECOND_FINGER_NS: u64 = 150_000_000;
/// The twist is taken up past this turn, the pinch past this change of the fingers' distance.
pub const TWIST_DEG: f32 = 6.0;
pub const PINCH_SHARE: f32 = 0.06;
/// A double-tap drag of the screen's height changes the zoom by this much, dragging down zooming in.
pub const ZOOM_PER_SCREEN: f32 = 0.8;
/// A fling and a twist ease to rest with this time constant, in seconds.
pub const EASE_S: f32 = 0.3;
/// Velocities are measured over the last this long of a finger's moves.
const VELOCITY_NS: u64 = 80_000_000;

/// What the fingers ask of the land.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum Gesture {
    /// One finger moves the land under it: from where the drag was taken up to where the finger is now.
    Drag { from: [f32; 2], to: [f32; 2] },
    /// Two fingers, from their places when the gesture last took up a part (`epoch` counts those times, so the app
    /// takes the pose then as its base) to their places now; `scale` and `turn` say which parts have been taken up.
    Two {
        epoch: u32,
        from: [[f32; 2]; 2],
        to: [[f32; 2]; 2],
        scale: bool,
        turn: bool,
    },
    /// A double-tap drag: the change of zoom since it was taken up.
    Zoom { delta: f32 },
    /// The fingers have left: the land glides on at `velocity` (screen pixels a second) and turns on at
    /// `turn_rate` (radians a second, counter-clockwise on the screen) about `about`, easing to rest.
    Release {
        velocity: [f32; 2],
        turn_rate: f32,
        about: [f32; 2],
    },
    /// A quick touch that did not move.
    Tap { at: [f32; 2] },
}

/// The screen's similarity that two-finger gestures ask for: it carries the fingers' first places to their places
/// now, scaled about their midpoint by the change of their distance if `scale`, turned by the change of their
/// angle if `turn`, and moved with their midpoint, so with both the land stays under both fingers.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Similarity {
    pub from_mid: [f32; 2],
    pub to_mid: [f32; 2],
    pub ratio: f32,
    /// Radians, as the screen's angles count with y down.
    pub angle: f32,
}

impl Similarity {
    pub fn between(from: [[f32; 2]; 2], to: [[f32; 2]; 2], scale: bool, turn: bool) -> Similarity {
        let (a, b) = (sub(from[1], from[0]), sub(to[1], to[0]));
        let ratio = if scale && len(a) > 0.0 { len(b) / len(a) } else { 1.0 };
        let angle = if turn { b[1].atan2(b[0]) - a[1].atan2(a[0]) } else { 0.0 };
        Similarity {
            from_mid: mid(from),
            to_mid: mid(to),
            ratio,
            angle,
        }
    }

    /// Where a point of the screen goes.
    pub fn apply(&self, p: [f32; 2]) -> [f32; 2] {
        let d = sub(p, self.from_mid);
        let (s, c) = self.angle.sin_cos();
        [
            self.to_mid[0] + self.ratio * (c * d[0] - s * d[1]),
            self.to_mid[1] + self.ratio * (s * d[0] + c * d[1]),
        ]
    }
}

fn sub(a: [f32; 2], b: [f32; 2]) -> [f32; 2] {
    [a[0] - b[0], a[1] - b[1]]
}

fn len(a: [f32; 2]) -> f32 {
    (a[0] * a[0] + a[1] * a[1]).sqrt()
}

fn mid(p: [[f32; 2]; 2]) -> [f32; 2] {
    [(p[0][0] + p[1][0]) / 2.0, (p[0][1] + p[1][1]) / 2.0]
}

/// The smallest turn between two angles, in radians.
fn turn_between(a: f32, b: f32) -> f32 {
    let mut d = b - a;
    while d > std::f32::consts::PI {
        d -= std::f32::consts::TAU;
    }
    while d < -std::f32::consts::PI {
        d += std::f32::consts::TAU;
    }
    d
}

/// The rate of turn over recent angles, in radians a second: the least-squares slope of the angles unwrapped from
/// the first, so the half-moved samples between two fingers' moves do not bias it.
fn turn_rate(angles: &[(f32, u64)]) -> f32 {
    let Some(&(first, t0)) = angles.first() else { return 0.0 };
    let mut unwrapped = Vec::with_capacity(angles.len());
    let mut last = first;
    let mut total = 0.0;
    for &(a, t) in angles {
        total += turn_between(last, a);
        last = a;
        unwrapped.push(((t - t0) as f32 / 1e9, total));
    }
    let n = unwrapped.len() as f32;
    let mt = unwrapped.iter().map(|u| u.0).sum::<f32>() / n;
    let ma = unwrapped.iter().map(|u| u.1).sum::<f32>() / n;
    let var: f32 = unwrapped.iter().map(|u| (u.0 - mt) * (u.0 - mt)).sum();
    if var <= 0.0 {
        return 0.0;
    }
    unwrapped.iter().map(|u| (u.0 - mt) * (u.1 - ma)).sum::<f32>() / var
}

/// One finger on the screen: its pointer, where and when it went down, and its recent places for its velocity.
#[derive(Clone, Debug, PartialEq)]
struct Finger {
    id: i32,
    down: [f32; 2],
    down_ns: u64,
    at: [f32; 2],
    recent: Vec<([f32; 2], u64)>,
}

impl Finger {
    fn new(e: &InputEvent) -> Finger {
        Finger {
            id: e.pointer,
            down: [e.x, e.y],
            down_ns: e.t_ns,
            at: [e.x, e.y],
            recent: vec![([e.x, e.y], e.t_ns)],
        }
    }

    fn moved(&mut self, e: &InputEvent) {
        self.at = [e.x, e.y];
        self.recent.push(([e.x, e.y], e.t_ns));
        self.recent.retain(|r| e.t_ns.saturating_sub(r.1) <= VELOCITY_NS);
    }

    /// Screen pixels a second over its last moves, none when it has stopped.
    fn velocity(&self, now_ns: u64) -> [f32; 2] {
        let recent: Vec<_> = self
            .recent
            .iter()
            .filter(|r| now_ns.saturating_sub(r.1) <= VELOCITY_NS)
            .collect();
        match (recent.first(), recent.last()) {
            (Some(a), Some(b)) if b.1 > a.1 => {
                let dt = (b.1 - a.1) as f32 / 1e9;
                [(b.0[0] - a.0[0]) / dt, (b.0[1] - a.0[1]) / dt]
            }
            _ => [0.0, 0.0],
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
enum State {
    Idle,
    /// One finger down and not yet claimed.
    Pending,
    Drag {
        from: [f32; 2],
    },
    /// Two fingers, neither part taken up yet: their places when the second went down.
    TwoPending {
        from: [[f32; 2]; 2],
    },
    Two {
        from: [[f32; 2]; 2],
        scale: bool,
        turn: bool,
        /// The angle between the fingers and its time, for the turn's rate at release.
        angles: Vec<(f32, u64)>,
    },
    /// The second touch of a double tap, not yet moving, then moving from `y`.
    ZoomPending,
    Zoom {
        y: f32,
    },
    /// A gesture has ended while fingers stay down: nothing more until they lift.
    Done,
}

/// The recognisers' state between touches.
#[derive(Clone, Debug, PartialEq)]
pub struct Gestures {
    fingers: Vec<Finger>,
    state: State,
    /// The last tap's place in screen pixels and when it lifted, for a double tap.
    last_tap: Option<([f32; 2], u64)>,
    epoch: u32,
}

impl Default for Gestures {
    fn default() -> Gestures {
        Gestures {
            fingers: Vec::new(),
            state: State::Idle,
            last_tap: None,
            epoch: 0,
        }
    }
}

impl Gestures {
    /// A raw touch, positions in screen pixels, `scale` of them a UI pixel, on a screen `screen_h` pixels high.
    pub fn input(&mut self, e: &InputEvent, scale: f32, screen_h: f32) -> Option<Gesture> {
        match e.kind {
            InputKind::Down => {
                self.down(e, scale);
                None
            }
            InputKind::Move => {
                let i = self.fingers.iter().position(|f| f.id == e.pointer)?;
                self.fingers[i].moved(e);
                let state = std::mem::replace(&mut self.state, State::Idle);
                let (state, out) = self.moved(state, self.fingers[i].at, e.t_ns, scale, screen_h);
                self.state = state;
                out
            }
            InputKind::Up => {
                let i = self.fingers.iter().position(|f| f.id == e.pointer)?;
                let finger = self.fingers.remove(i);
                let state = std::mem::replace(&mut self.state, State::Done);
                let out = self.lifted(state, &finger, e, scale);
                if self.fingers.is_empty() {
                    self.state = State::Idle;
                }
                out
            }
            InputKind::Cancel => {
                let active = !matches!(self.state, State::Idle | State::Pending | State::Done);
                *self = Gestures::default();
                active.then_some(Gesture::Release {
                    velocity: [0.0, 0.0],
                    turn_rate: 0.0,
                    about: [e.x, e.y],
                })
            }
        }
    }

    fn down(&mut self, e: &InputEvent, scale: f32) {
        match (&self.state, self.fingers.len()) {
            (State::Idle, _) => {
                let near_tap = self.last_tap.is_some_and(|(at, t)| {
                    e.t_ns.saturating_sub(t) <= DOUBLE_TAP_NS && len(sub([e.x, e.y], at)) / scale <= DOUBLE_TAP_SLOP
                });
                self.fingers = vec![Finger::new(e)];
                self.state = if near_tap { State::ZoomPending } else { State::Pending };
                self.last_tap = None;
            }
            (State::Pending, 1) if e.t_ns.saturating_sub(self.fingers[0].down_ns) <= SECOND_FINGER_NS => {
                self.fingers.push(Finger::new(e));
                self.state = State::TwoPending {
                    from: [self.fingers[0].at, self.fingers[1].at],
                };
            }
            // A late or a third finger is not followed (A12.2).
            _ => {}
        }
    }

    /// A finger moved to `at`: the next state, and what the fingers ask now.
    fn moved(&mut self, state: State, at: [f32; 2], t_ns: u64, scale: f32, screen_h: f32) -> (State, Option<Gesture>) {
        let slid = |f: &Finger| len(sub(at, f.down)) / scale > DRAG_SLOP;
        match state {
            State::Pending if slid(&self.fingers[0]) => {
                (State::Drag { from: at }, Some(Gesture::Drag { from: at, to: at }))
            }
            State::Drag { from } => (State::Drag { from }, Some(Gesture::Drag { from, to: at })),
            State::ZoomPending if slid(&self.fingers[0]) => {
                (State::Zoom { y: at[1] }, Some(Gesture::Zoom { delta: 0.0 }))
            }
            State::Zoom { y } => (
                State::Zoom { y },
                Some(Gesture::Zoom {
                    delta: -ZOOM_PER_SCREEN * (at[1] - y) / screen_h,
                }),
            ),
            State::TwoPending { from } if self.fingers.len() == 2 => self.two(from, false, false, Vec::new(), t_ns),
            State::Two {
                from,
                scale: s,
                turn,
                angles,
            } if self.fingers.len() == 2 => self.two(from, s, turn, angles, t_ns),
            other => (other, None),
        }
    }

    /// Two fingers moved: each part is taken up past its threshold, and the gesture starts again from where the
    /// fingers are then, so the land does not jump by the threshold.
    fn two(
        &mut self,
        from: [[f32; 2]; 2],
        was_scale: bool,
        was_turn: bool,
        mut angles: Vec<(f32, u64)>,
        t_ns: u64,
    ) -> (State, Option<Gesture>) {
        let now = [self.fingers[0].at, self.fingers[1].at];
        let (a, b) = (sub(from[1], from[0]), sub(now[1], now[0]));
        let ratio = if len(a) > 0.0 { len(b) / len(a) } else { 1.0 };
        let turned = turn_between(a[1].atan2(a[0]), b[1].atan2(b[0])).to_degrees();
        let scale = was_scale || (ratio - 1.0).abs() > PINCH_SHARE;
        let turn = was_turn || turned.abs() > TWIST_DEG;
        if !scale && !turn {
            return (State::TwoPending { from }, None);
        }
        let from = if (scale, turn) != (was_scale, was_turn) {
            self.epoch += 1;
            now
        } else {
            from
        };
        angles.push((b[1].atan2(b[0]), t_ns));
        angles.retain(|r| t_ns.saturating_sub(r.1) <= VELOCITY_NS);
        let out = Gesture::Two {
            epoch: self.epoch,
            from,
            to: now,
            scale,
            turn,
        };
        (
            State::Two {
                from,
                scale,
                turn,
                angles,
            },
            Some(out),
        )
    }

    /// A finger lifted: a tap, or the land's glide or turn easing to rest.
    fn lifted(&mut self, state: State, finger: &Finger, e: &InputEvent, scale: f32) -> Option<Gesture> {
        match state {
            State::Pending => {
                let quick = e.t_ns.saturating_sub(finger.down_ns) <= TAP_NS;
                let still = len(sub([e.x, e.y], finger.down)) / scale <= TAP_SLOP;
                (quick && still).then(|| {
                    self.last_tap = Some(([e.x, e.y], e.t_ns));
                    Gesture::Tap { at: [e.x, e.y] }
                })
            }
            State::Drag { .. } => Some(Gesture::Release {
                velocity: finger.velocity(e.t_ns),
                turn_rate: 0.0,
                about: [e.x, e.y],
            }),
            State::Two { angles, turn, .. } => {
                let rate = if turn { turn_rate(&angles) } else { 0.0 };
                let about = self.fingers.first().map_or(finger.at, |f| mid([f.at, finger.at]));
                Some(Gesture::Release {
                    velocity: [0.0, 0.0],
                    turn_rate: rate,
                    about,
                })
            }
            _ => None,
        }
    }

    /// Whether fingers are down, so the app holds its easing.
    pub fn busy(&self) -> bool {
        !self.fingers.is_empty()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const MS: u64 = 1_000_000;
    /// The phone's screen: 4 screen pixels a UI pixel, 2,404 pixels high.
    const SCALE: f32 = 4.0;
    const SCREEN_H: f32 = 2404.0;

    fn ev(kind: InputKind, pointer: i32, x: f32, y: f32, t_ms: u64) -> InputEvent {
        InputEvent {
            kind,
            pointer,
            x,
            y,
            t_ns: t_ms * MS,
        }
    }

    /// Feeds touches, keeping every gesture asked for.
    fn feed(g: &mut Gestures, events: &[InputEvent]) -> Vec<Gesture> {
        events.iter().filter_map(|e| g.input(e, SCALE, SCREEN_H)).collect()
    }

    /// Two fingers moving in `steps` from `a` to `b`, 10 ms apart, after going down 20 ms apart.
    fn two_fingers(a: [[f32; 2]; 2], b: [[f32; 2]; 2], steps: u64) -> Vec<InputEvent> {
        let mut out = vec![
            ev(InputKind::Down, 1, a[0][0], a[0][1], 0),
            ev(InputKind::Down, 2, a[1][0], a[1][1], 20),
        ];
        for k in 1..=steps {
            let f = k as f32 / steps as f32;
            for (id, i) in [(1, 0), (2, 1)] {
                let p = [a[i][0] + (b[i][0] - a[i][0]) * f, a[i][1] + (b[i][1] - a[i][1]) * f];
                out.push(ev(InputKind::Move, id, p[0], p[1], 20 + 10 * k));
            }
        }
        out
    }

    // checks: PRE-33
    #[test]
    fn drag_is_not_a_tap() {
        // A quick touch that stays within 6 UI pixels is a tap, and moves nothing.
        let mut g = Gestures::default();
        let tap = feed(
            &mut g,
            &[
                ev(InputKind::Down, 1, 500.0, 900.0, 0),
                ev(InputKind::Move, 1, 510.0, 905.0, 50),
                ev(InputKind::Up, 1, 510.0, 905.0, 120),
            ],
        );
        assert_eq!(tap, vec![Gesture::Tap { at: [510.0, 905.0] }]);
        // Over 6 UI pixels it is a drag, taken up where it passed the slop, so the land does not jump; it is no
        // tap however quick, and a quick lift flings.
        let mut g = Gestures::default();
        let drag = feed(
            &mut g,
            &[
                ev(InputKind::Down, 1, 500.0, 900.0, 1000),
                ev(InputKind::Move, 1, 520.0, 900.0, 1010),
                ev(InputKind::Move, 1, 530.0, 900.0, 1020),
                ev(InputKind::Move, 1, 600.0, 900.0, 1040),
                ev(InputKind::Up, 1, 600.0, 900.0, 1050),
            ],
        );
        assert_eq!(
            drag[0],
            Gesture::Drag {
                from: [530.0, 900.0],
                to: [530.0, 900.0]
            }
        );
        assert_eq!(
            drag[1],
            Gesture::Drag {
                from: [530.0, 900.0],
                to: [600.0, 900.0]
            }
        );
        let Gesture::Release {
            velocity, turn_rate, ..
        } = drag[2]
        else {
            panic!("{:?}", drag[2])
        };
        assert!(
            velocity[0] > 2_000.0 && velocity[1] == 0.0 && turn_rate == 0.0,
            "{velocity:?}"
        );
        assert!(!drag.iter().any(|d| matches!(d, Gesture::Tap { .. })));
        // A drag that stops before lifting does not fling; a slow touch is neither tap nor drag.
        let mut g = Gestures::default();
        let stopped = feed(
            &mut g,
            &[
                ev(InputKind::Down, 1, 500.0, 900.0, 0),
                ev(InputKind::Move, 1, 600.0, 900.0, 30),
                ev(InputKind::Up, 1, 600.0, 900.0, 400),
            ],
        );
        assert_eq!(
            stopped.last(),
            Some(&Gesture::Release {
                velocity: [0.0, 0.0],
                turn_rate: 0.0,
                about: [600.0, 900.0]
            })
        );
        let slow = feed(
            &mut g,
            &[
                ev(InputKind::Down, 1, 500.0, 900.0, 1000),
                ev(InputKind::Up, 1, 500.0, 900.0, 1500),
            ],
        );
        assert!(slow.is_empty());
    }

    // checks: PRE-33 PRE-22
    #[test]
    fn pinch_keeps_the_land_under_the_fingers() {
        // Fingers spreading from 300 to 600 pixels apart while their midpoint drifts: the pinch is taken up past
        // 6%, and from then the similarity it asks for carries the fingers' places then to their places now, so
        // the land under them stays under them.
        let a = [[400.0, 1000.0], [700.0, 1000.0]];
        let b = [[300.0, 1100.0], [900.0, 1100.0]];
        let mut g = Gestures::default();
        let asked = feed(&mut g, &two_fingers(a, b, 30));
        let first = asked
            .iter()
            .position(|x| matches!(x, Gesture::Two { .. }))
            .expect("a pinch");
        assert_eq!(first, 0);
        // Of the 60 moves, the first three come before the distance has changed by 6% and ask nothing.
        assert_eq!(asked.len(), 57);
        for (k, x) in asked.iter().enumerate() {
            let Gesture::Two {
                from, to, scale, turn, ..
            } = *x
            else {
                continue;
            };
            assert!(scale && !turn, "a straight spread is no twist");
            let s = Similarity::between(from, to, scale, turn);
            // The midpoint and the fingers' distance always follow; each finger exactly once both have moved
            // this step (every second move: between them the line joining them leans, which a pinch without
            // its twist leaves out).
            let m = s.apply(mid(from));
            assert!((m[0] - mid(to)[0]).abs() < 1e-3 && (m[1] - mid(to)[1]).abs() < 1e-3);
            let spread = len(sub(s.apply(from[1]), s.apply(from[0])));
            assert!((spread - len(sub(to[1], to[0]))).abs() < 1e-3);
            if k % 2 == 0 {
                for i in 0..2 {
                    let p = s.apply(from[i]);
                    assert!(
                        (p[0] - to[i][0]).abs() < 1e-3 && (p[1] - to[i][1]).abs() < 1e-3,
                        "{p:?} {:?}",
                        to[i]
                    );
                }
            }
        }
        let Some(Gesture::Two { from, .. }) = asked.last() else {
            panic!()
        };
        // The threshold's 6% is not jumped: the pinch starts from where the fingers were when it was taken up.
        let d0 = (from[1][0] - from[0][0]).abs();
        assert!(d0 > 300.0 * 1.06 && d0 < 300.0 * 1.12, "{d0}");
        // Small wobbles are no pinch.
        let mut g = Gestures::default();
        let wobble = feed(&mut g, &two_fingers(a, [[405.0, 1000.0], [710.0, 1003.0]], 5));
        assert!(wobble.is_empty(), "{wobble:?}");
    }

    // checks: PRE-33 PRE-22
    #[test]
    fn twist_keeps_the_land_under_the_fingers() {
        // Fingers turning 40 degrees about their midpoint while spreading a little: the twist is taken up past 6
        // degrees and the pinch joins past 6%, each time starting again where the fingers are (a new epoch), and
        // the similarity carries both fingers exactly.
        let centre = [540.0, 1200.0];
        let at = |deg: f32, r: f32| {
            let (s, c) = deg.to_radians().sin_cos();
            [
                [centre[0] - r * c, centre[1] - r * s],
                [centre[0] + r * c, centre[1] + r * s],
            ]
        };
        let mut events = vec![
            ev(InputKind::Down, 1, at(0.0, 200.0)[0][0], at(0.0, 200.0)[0][1], 0),
            ev(InputKind::Down, 2, at(0.0, 200.0)[1][0], at(0.0, 200.0)[1][1], 30),
        ];
        for k in 1..=40u64 {
            let p = at(k as f32, 200.0 + k as f32 * 1.5);
            events.push(ev(InputKind::Move, 1, p[0][0], p[0][1], 30 + 10 * k));
            events.push(ev(InputKind::Move, 2, p[1][0], p[1][1], 30 + 10 * k));
        }
        events.push(ev(InputKind::Up, 1, at(40.0, 260.0)[0][0], at(40.0, 260.0)[0][1], 440));
        let mut g = Gestures::default();
        let asked = feed(&mut g, &events);
        let twos: Vec<_> = asked
            .iter()
            .filter_map(|x| match *x {
                Gesture::Two {
                    epoch,
                    from,
                    to,
                    scale,
                    turn,
                } => Some((epoch, from, to, scale, turn)),
                _ => None,
            })
            .collect();
        assert!(twos[0].4 && !twos[0].3, "the twist first: {:?}", twos[0]);
        assert!(twos.last().is_some_and(|t| t.3 && t.4), "then the pinch joins");
        let epochs: Vec<u32> = twos.iter().map(|t| t.0).collect();
        assert_eq!(epochs.first().copied(), Some(1));
        assert_eq!(epochs.last().copied(), Some(2));
        for &(_, from, to, scale, turn) in &twos {
            let s = Similarity::between(from, to, scale, turn);
            if scale && turn {
                for i in 0..2 {
                    let p = s.apply(from[i]);
                    assert!(
                        (p[0] - to[i][0]).abs() < 1e-2 && (p[1] - to[i][1]).abs() < 1e-2,
                        "{p:?} {:?}",
                        to[i]
                    );
                }
            }
            // The midpoint, about which the land turns, always stays under the fingers' midpoint.
            let m = s.apply(mid(from));
            assert!((m[0] - mid(to)[0]).abs() < 1e-3 && (m[1] - mid(to)[1]).abs() < 1e-3);
        }
        // Lifting a finger eases the turn to rest at its rate, a degree every 10 ms, about the fingers' midpoint.
        let Some(Gesture::Release { turn_rate, about, .. }) = asked.last().copied() else {
            panic!("{asked:?}")
        };
        assert!((turn_rate - 100f32.to_radians()).abs() < 0.05, "{turn_rate}");
        assert!(
            (about[0] - centre[0]).abs() < 1.0 && (about[1] - centre[1]).abs() < 1.0,
            "{about:?}"
        );
        // A second finger too late is not followed: the first finger drags on.
        let mut g = Gestures::default();
        let late = feed(
            &mut g,
            &[
                ev(InputKind::Down, 1, 500.0, 900.0, 0),
                ev(InputKind::Down, 2, 800.0, 900.0, 200),
                ev(InputKind::Move, 2, 900.0, 900.0, 210),
                ev(InputKind::Move, 1, 560.0, 900.0, 220),
            ],
        );
        assert!(matches!(late.as_slice(), [Gesture::Drag { .. }]), "{late:?}");
    }

    // checks: PRE-33 PRE-03
    #[test]
    fn double_tap_drag_zooms() {
        // A tap, then within 300 ms and 12 UI pixels a second touch that moves: dragging down a whole screen
        // zooms in by 0.8, up zooms out, measured from where the drag was taken up.
        let mut g = Gestures::default();
        let asked = feed(
            &mut g,
            &[
                ev(InputKind::Down, 1, 500.0, 1000.0, 0),
                ev(InputKind::Up, 1, 500.0, 1000.0, 80),
                ev(InputKind::Down, 1, 520.0, 1010.0, 250),
                ev(InputKind::Move, 1, 520.0, 1050.0, 270),
                ev(InputKind::Move, 1, 520.0, 1050.0 + SCREEN_H / 2.0, 400),
                ev(InputKind::Move, 1, 520.0, 1050.0 - SCREEN_H / 4.0, 500),
                ev(InputKind::Up, 1, 520.0, 1050.0 - SCREEN_H / 4.0, 520),
            ],
        );
        assert!(matches!(asked[0], Gesture::Tap { .. }));
        assert_eq!(asked[1], Gesture::Zoom { delta: 0.0 });
        assert_eq!(asked[2], Gesture::Zoom { delta: -0.4 });
        assert_eq!(asked[3], Gesture::Zoom { delta: 0.2 });
        assert_eq!(asked.len(), 4, "no fling after a zoom");
        // Too late or too far from the tap, the second touch is a plain drag.
        for (dt, dx) in [(400, 0.0), (100, 60.0)] {
            let mut g = Gestures::default();
            let asked = feed(
                &mut g,
                &[
                    ev(InputKind::Down, 1, 500.0, 1000.0, 0),
                    ev(InputKind::Up, 1, 500.0, 1000.0, 80),
                    ev(InputKind::Down, 1, 500.0 + dx, 1000.0, 80 + dt),
                    ev(InputKind::Move, 1, 500.0 + dx, 1100.0, 100 + dt),
                ],
            );
            assert!(matches!(asked[1], Gesture::Drag { .. }), "{dt} ms, {dx} px: {asked:?}");
        }
    }
}
