//! The camera's pose and how gestures move it (A11.2, A12.2): a drag moves the ground under the finger
//! and a release glides on, easing to rest (τ 0.3 s); a twist turns the ground with the fingers and eases to rest;
//! a pinch or a double-tap drag zooms. The zoom keeps to 0.00–0.40 in α01b, and the target to the demo area.
//! Drags and glides keep the target's height, so a pan moves the picture by whole pixels and nothing bobs over
//! bumps (A11.2's test); the first turn or zoom after them slides the target along the view's centre line onto the
//! ground, which leaves the picture still, so turns and zooms pivot on the ground in the middle of the screen.
//! Implements `PRE-22`, `PRE-33` and `PRE-34` in part.

use kd_core::geo::{self, Pos, Vec2};
use kd_render::camera::{self as rcam, ArtSize};
use kd_ui::gestures::{CameraCmd, FLING_TAU_S};
use kd_view::{CameraPose, GroundGrid};

/// The zoom's range in α01b: the demo area is 256 m, and the valley stop needs α02a's coarse ground.
pub const ZOOM_MIN: f32 = 0.0;
pub const ZOOM_MAX: f32 = 0.40;
/// Glides and turns under these speeds stop, metres and radians a second.
const REST_M_S: f32 = 0.01;
const REST_RAD_S: f32 = 0.001;
/// The step, in metres along the view's centre line, at which `anchor` looks for the ground.
const ANCHOR_STEP_M: f32 = 0.25;

/// The camera's pose and its glide and turn speeds.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct CameraCtl {
    pub pose: CameraPose,
    /// Ground metres a second, east and south.
    vel: Vec2,
    /// Radians a second.
    turn_v: f32,
    /// Whether the target lies where the view's centre line meets the ground; drags and glides clear it.
    on_ground: bool,
}

impl CameraCtl {
    /// A camera at `pose`, whose target is taken to be on the ground.
    pub fn new(pose: CameraPose) -> CameraCtl {
        CameraCtl {
            pose,
            vel: Vec2::default(),
            turn_v: 0.0,
            on_ground: true,
        }
    }

    /// Ground metres for a move of (`dx`, `dy`) screen pixels: the ground under the finger follows it.
    fn ground_move(&self, dx: f32, dy: f32, size: ArtSize) -> Vec2 {
        let texel = rcam::texel(self.pose.zoom, size);
        let pitch = rcam::pitch(texel);
        let (s, c) = (self.pose.yaw.sin(), self.pose.yaw.cos());
        let s_px = size.s as f32;
        // right on the screen is (cos, −sin) on the ground; up the screen is forward, (−sin, −cos), stretched by
        // the tilt
        let right = -dx / s_px * texel;
        let fwd = dy / s_px * texel / pitch.sin().max(0.05);
        Vec2 {
            x: right * c - fwd * s,
            y: -right * s - fwd * c,
        }
    }

    /// Applies one command from the recogniser.
    pub fn apply(&mut self, cmd: CameraCmd, size: ArtSize, g: &GroundGrid) {
        match cmd {
            CameraCmd::Drag { dx, dy } => {
                self.vel = Vec2::default();
                let m = self.ground_move(dx, dy, size);
                self.move_by(m, g);
            }
            CameraCmd::Fling { vx, vy } => self.vel = self.ground_move(vx, vy, size),
            CameraCmd::Turn { rad } => {
                self.anchor(size, g);
                self.turn_v = 0.0;
                self.pose.yaw += rad;
            }
            CameraCmd::TurnFling { rad_s } => self.turn_v = rad_s,
            CameraCmd::Zoom { dz } => {
                self.anchor(size, g);
                self.pose.zoom = (self.pose.zoom + dz).clamp(ZOOM_MIN, ZOOM_MAX);
            }
            CameraCmd::Tap { .. } => {}
        }
    }

    /// Moves the target by `m` metres across the ground, kept over it, at the same height.
    fn move_by(&mut self, m: Vec2, g: &GroundGrid) {
        let d = geo::delta(g.origin, self.pose.target);
        let side = (g.side - 1) as f32;
        let (x, y) = ((d.x + m.x).clamp(0.0, side), (d.y + m.y).clamp(0.0, side));
        let p = geo::offset(g.origin, Vec2 { x, y });
        self.pose.target = Pos {
            z: self.pose.target.z,
            ..p
        };
        self.on_ground = false;
    }

    /// Slides the target along the view's centre line to the first ground it meets, unless it is there already:
    /// the view is orthographic, so the picture does not move, and the turn or zoom that follows pivots on the
    /// ground in the middle of the screen.
    fn anchor(&mut self, size: ArtSize, g: &GroundGrid) {
        if std::mem::replace(&mut self.on_ground, true) {
            return;
        }
        let pitch = rcam::pitch(rcam::texel(self.pose.zoom, size));
        let (cp, sp) = (pitch.cos(), pitch.sin().max(0.05));
        // per metre along the line, away from the eye: east, south and up
        let dir = [-self.pose.yaw.sin() * cp, -self.pose.yaw.cos() * cp, -sp];
        let d = geo::delta(g.origin, self.pose.target);
        let z = self.pose.target.z as f32 / 256.0;
        let above = |t: f32| z + dir[2] * t - crate::valley::height_at(g, d.x + dir[0] * t, d.y + dir[1] * t);
        // from where the line comes down to the highest ground, step along it to the first point at or under the
        // ground, then halve the step to the crossing
        let (lo, hi) = g
            .heights_m
            .iter()
            .fold((f32::MAX, f32::MIN), |(lo, hi), &h| (lo.min(h), hi.max(h)));
        let (mut t0, end) = ((z - hi) / sp, (z - lo) / sp + ANCHOR_STEP_M);
        let mut t1 = t0;
        while above(t1) > 0.0 {
            if t1 > end {
                return;
            }
            t0 = t1;
            t1 += ANCHOR_STEP_M;
        }
        for _ in 0..20 {
            let t = 0.5 * (t0 + t1);
            if above(t) > 0.0 {
                t0 = t;
            } else {
                t1 = t;
            }
        }
        let (x, y) = (d.x + dir[0] * t1, d.y + dir[1] * t1);
        let side = (g.side - 1) as f32;
        if !(0.0..=side).contains(&x) || !(0.0..=side).contains(&y) {
            return;
        }
        let p = geo::offset(g.origin, Vec2 { x, y });
        self.pose.target = Pos {
            z: ((z + dir[2] * t1) * 256.0).round() as i32,
            ..p
        };
    }

    /// Glides and turns on for `dt` seconds, easing to rest; true while moving.
    pub fn step(&mut self, dt: f32, g: &GroundGrid) -> bool {
        let keep = (-dt / FLING_TAU_S).exp();
        let mut moving = false;
        if self.vel.x.hypot(self.vel.y) > REST_M_S {
            let m = Vec2 {
                x: self.vel.x * dt,
                y: self.vel.y * dt,
            };
            self.move_by(m, g);
            self.vel = Vec2 {
                x: self.vel.x * keep,
                y: self.vel.y * keep,
            };
            moving = true;
        } else {
            self.vel = Vec2::default();
        }
        if self.turn_v.abs() > REST_RAD_S {
            self.pose.yaw += self.turn_v * dt;
            self.turn_v *= keep;
            moving = true;
        } else {
            self.turn_v = 0.0;
        }
        moving
    }
}

#[cfg(test)]
mod tests;
