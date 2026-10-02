//! Gestures (A12.2): a recogniser over raw touches giving camera commands, with A12.2's thresholds. One finger drags
//! after 6 UI pixels and flings on release; a second finger within 150 ms starts the two-finger gestures, where a
//! twist over 6° turns and a pinch over 6% zooms, together if both; a second touch within 300 ms and 12 pixels of a
//! tap, then moving, zooms by 0.8 a screen height; a finger lifted within 300 ms and 6 pixels is a tap.
//! Implements `PRE-33` and `PRE-34` in part.

use kd_core::m;
use kd_view::{InputEvent, InputKind};

/// One finger moves this far, UI pixels, before it drags.
pub const DRAG_UI_PX: f32 = 6.0;
/// A second finger down within this, ns, of the first starts two-finger gestures.
pub const SECOND_FINGER_NS: u64 = 150_000_000;
/// Two fingers turn this much, degrees, before they twist.
pub const TWIST_DEG: f32 = 6.0;
/// Their distance changes this share before they pinch.
pub const PINCH_SHARE: f32 = 0.06;
/// Zoom change per natural log of the pinch's ratio, as the mockup.
pub const ZOOM_PER_LN: f32 = 0.16;
/// A tap lifts within this, ns, and this many UI pixels.
pub const TAP_NS: u64 = 300_000_000;
pub const TAP_UI_PX: f32 = 6.0;
/// A double tap's second touch comes within this, ns, and this many UI pixels of the tap.
pub const DOUBLE_NS: u64 = 300_000_000;
pub const DOUBLE_UI_PX: f32 = 12.0;
/// The double-tap drag's zoom change for a screen height of movement; down zooms in.
pub const DOUBLE_ZOOM_PER_SCREEN: f32 = 0.8;
/// A fling eases to rest with this time constant, seconds (applied by the caller).
pub const FLING_TAU_S: f32 = 0.3;
/// Moves within this, ns, of the release set a fling's speed.
const FLING_WINDOW_NS: u64 = 80_000_000;

/// What the camera should do (A12.2), in screen pixels and radians; the caller turns them into the pose.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum CameraCmd {
    /// The finger moved the ground by (`dx`, `dy`) screen pixels.
    Drag { dx: f32, dy: f32 },
    /// The finger left while moving, at screen pixels a second; ease on to rest.
    Fling { vx: f32, vy: f32 },
    /// The two fingers turned by `rad`, positive clockwise on the screen.
    Turn { rad: f32 },
    /// The twist ended turning at `rad_s` a second; ease to rest.
    TurnFling { rad_s: f32 },
    /// Change the zoom by `dz` (negative zooms in).
    Zoom { dz: f32 },
    /// A tap at (`x`, `y`) screen pixels.
    Tap { x: f32, y: f32 },
}

#[derive(Clone, Copy, Debug, PartialEq)]
struct Pt {
    id: i32,
    x: f32,
    y: f32,
    t: u64,
}

#[derive(Clone, Copy, Debug, PartialEq)]
enum State {
    Idle,
    /// One finger down, not yet claimed; `double` when it may start a double-tap drag.
    One {
        start: Pt,
        last: Pt,
        double: bool,
    },
    Dragging {
        last: Pt,
        prev: Pt,
    },
    DoubleDrag {
        last: Pt,
    },
    /// Two fingers: where each started and now, the angle and distance at the start and last read, and what is on.
    Two {
        a: Pt,
        b: Pt,
        a0: f32,
        d0: f32,
        ang: f32,
        dist: f32,
        twist: bool,
        pinch: bool,
        turn_v: f32,
        t: u64,
    },
    /// A finger of a finished gesture is still down: ignored until all lift.
    Done {
        down: u32,
    },
}

/// The recogniser (A12.2).
pub struct Gestures {
    scale: f32,
    screen_h: f32,
    state: State,
    last_tap: Option<Pt>,
}

fn angle(a: Pt, b: Pt) -> f32 {
    m::atan2(b.y - a.y, b.x - a.x)
}

fn gap(a: Pt, b: Pt) -> f32 {
    ((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y)).sqrt()
}

/// `d` taken the short way round, in (−π, π].
fn wrap_pi(d: f32) -> f32 {
    let tau = core::f32::consts::TAU;
    let mut r = d % tau;
    if r > core::f32::consts::PI {
        r -= tau;
    } else if r <= -core::f32::consts::PI {
        r += tau;
    }
    r
}

impl Gestures {
    /// A recogniser for screens of `screen_h` pixels where a UI pixel spans `scale` screen pixels.
    pub fn new(scale: u32, screen_h: u32) -> Gestures {
        Gestures {
            scale: scale.max(1) as f32,
            screen_h: screen_h.max(1) as f32,
            state: State::Idle,
            last_tap: None,
        }
    }

    pub fn resize(&mut self, scale: u32, screen_h: u32) {
        self.scale = scale.max(1) as f32;
        self.screen_h = screen_h.max(1) as f32;
    }

    /// One raw touch that the UI did not take; its commands go to `out`.
    pub fn input(&mut self, e: &InputEvent, out: &mut Vec<CameraCmd>) {
        let p = Pt {
            id: e.pointer,
            x: e.x,
            y: e.y,
            t: e.t_ns,
        };
        let px = |ui: f32| ui * self.scale;
        self.state = match (self.state, e.kind) {
            (State::Idle, InputKind::Down) => {
                let double = self
                    .last_tap
                    .is_some_and(|t| p.t.saturating_sub(t.t) <= DOUBLE_NS && gap(t, p) <= px(DOUBLE_UI_PX));
                State::One {
                    start: p,
                    last: p,
                    double,
                }
            }
            (State::One { start, .. }, InputKind::Down) if p.id != start.id => {
                if p.t.saturating_sub(start.t) <= SECOND_FINGER_NS {
                    let (ang, dist) = (angle(start, p), gap(start, p));
                    State::Two {
                        a: start,
                        b: p,
                        a0: ang,
                        d0: dist,
                        ang,
                        dist,
                        twist: false,
                        pinch: false,
                        turn_v: 0.0,
                        t: p.t,
                    }
                } else {
                    State::Done { down: 2 }
                }
            }
            (State::One { start, double, .. }, InputKind::Move) if p.id == start.id => {
                if gap(start, p) > px(DRAG_UI_PX) {
                    if double {
                        out.push(CameraCmd::Zoom {
                            dz: -(p.y - start.y) / self.screen_h * DOUBLE_ZOOM_PER_SCREEN,
                        });
                        State::DoubleDrag { last: p }
                    } else {
                        out.push(CameraCmd::Drag {
                            dx: p.x - start.x,
                            dy: p.y - start.y,
                        });
                        State::Dragging { last: p, prev: start }
                    }
                } else {
                    State::One { start, last: p, double }
                }
            }
            (State::One { start, .. }, InputKind::Up) if p.id == start.id => {
                if p.t.saturating_sub(start.t) <= TAP_NS && gap(start, p) <= px(TAP_UI_PX) {
                    out.push(CameraCmd::Tap { x: start.x, y: start.y });
                    self.last_tap = Some(p);
                } else {
                    self.last_tap = None;
                }
                State::Idle
            }
            (State::Dragging { last, prev }, InputKind::Move) if p.id == last.id => {
                out.push(CameraCmd::Drag {
                    dx: p.x - last.x,
                    dy: p.y - last.y,
                });
                // keep the move that began the fling window, to measure the release speed
                let prev = if p.t.saturating_sub(prev.t) > FLING_WINDOW_NS {
                    last
                } else {
                    prev
                };
                State::Dragging { last: p, prev }
            }
            (State::Dragging { last, prev }, InputKind::Up) if p.id == last.id => {
                let dt = p.t.saturating_sub(prev.t) as f32 / 1e9;
                if dt > 0.0 && p.t.saturating_sub(last.t) < FLING_WINDOW_NS {
                    out.push(CameraCmd::Fling {
                        vx: (p.x - prev.x) / dt,
                        vy: (p.y - prev.y) / dt,
                    });
                }
                self.last_tap = None;
                State::Idle
            }
            (State::DoubleDrag { last }, InputKind::Move) if p.id == last.id => {
                out.push(CameraCmd::Zoom {
                    dz: -(p.y - last.y) / self.screen_h * DOUBLE_ZOOM_PER_SCREEN,
                });
                State::DoubleDrag { last: p }
            }
            (State::DoubleDrag { last }, InputKind::Up) if p.id == last.id => {
                self.last_tap = None;
                State::Idle
            }
            (
                State::Two {
                    a,
                    b,
                    a0,
                    d0,
                    ang,
                    dist,
                    twist,
                    pinch,
                    turn_v,
                    t,
                },
                InputKind::Move,
            ) if p.id == a.id || p.id == b.id => {
                let (a, b) = if p.id == a.id { (p, b) } else { (a, p) };
                let (na, nd) = (angle(a, b), gap(a, b));
                let twist = twist || wrap_pi(na - a0).abs() > TWIST_DEG.to_radians();
                let pinch = pinch || (nd / d0.max(1.0) - 1.0).abs() > PINCH_SHARE;
                let mut turn_v = turn_v;
                if twist {
                    let d = wrap_pi(na - ang);
                    out.push(CameraCmd::Turn { rad: d });
                    let dt = p.t.saturating_sub(t) as f32 / 1e9;
                    if dt > 0.0 {
                        turn_v = d / dt;
                    }
                }
                if pinch && dist > 0.0 && nd > 0.0 {
                    out.push(CameraCmd::Zoom {
                        dz: -m::ln(nd / dist) * ZOOM_PER_LN,
                    });
                }
                // only the parts past their thresholds move the camera; until then the start is the reference
                let ang = if twist { na } else { ang };
                let dist = if pinch { nd } else { dist };
                State::Two {
                    a,
                    b,
                    a0,
                    d0,
                    ang,
                    dist,
                    twist,
                    pinch,
                    turn_v,
                    t: p.t,
                }
            }
            (
                State::Two {
                    a, b, twist, turn_v, ..
                },
                InputKind::Up | InputKind::Cancel,
            ) if p.id == a.id || p.id == b.id => {
                if twist && turn_v != 0.0 {
                    out.push(CameraCmd::TurnFling { rad_s: turn_v });
                }
                self.last_tap = None;
                State::Done { down: 1 }
            }
            (State::Done { down }, InputKind::Down) => State::Done { down: down + 1 },
            (State::Done { down }, InputKind::Up | InputKind::Cancel) => {
                if down <= 1 {
                    State::Idle
                } else {
                    State::Done { down: down - 1 }
                }
            }
            (State::Dragging { .. } | State::DoubleDrag { .. }, InputKind::Down) => State::Done { down: 2 },
            (_, InputKind::Cancel) => State::Idle,
            (s, _) => s,
        };
    }
}

#[cfg(test)]
mod tests;
