//! The camera's pose and how gestures move it (T01b.6, T01b.7, A12.2): a drag moves the ground under the finger
//! and a release glides on, easing to rest (τ 0.3 s); a twist turns the ground with the fingers and eases to rest;
//! a pinch or a double-tap drag zooms. The zoom keeps to 0.00–0.40 in α01b, and the target to the demo area.
//! Implements `PRE-22`, `PRE-33` and `PRE-34` in part.

use kd_core::geo::{self, Vec2};
use kd_render::camera::{self as rcam, ArtSize};
use kd_ui::gestures::{CameraCmd, FLING_TAU_S};
use kd_view::{CameraPose, GroundGrid};

/// The zoom's range in α01b: the demo area is 256 m, and the valley stop needs α02a's coarse ground.
pub const ZOOM_MIN: f32 = 0.0;
pub const ZOOM_MAX: f32 = 0.40;
/// Glides and turns under these speeds stop, metres and radians a second.
const REST_M_S: f32 = 0.01;
const REST_RAD_S: f32 = 0.001;

/// The camera's pose and its glide and turn speeds.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct CameraCtl {
    pub pose: CameraPose,
    /// Ground metres a second, east and south.
    vel: Vec2,
    /// Radians a second.
    turn_v: f32,
}

impl CameraCtl {
    pub fn new(pose: CameraPose) -> CameraCtl {
        CameraCtl {
            pose,
            vel: Vec2::default(),
            turn_v: 0.0,
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
                self.turn_v = 0.0;
                self.pose.yaw += rad;
            }
            CameraCmd::TurnFling { rad_s } => self.turn_v = rad_s,
            CameraCmd::Zoom { dz } => self.pose.zoom = (self.pose.zoom + dz).clamp(ZOOM_MIN, ZOOM_MAX),
            CameraCmd::Tap { .. } => {}
        }
    }

    /// Moves the target by `m` metres, kept over the ground, at the ground's height there.
    fn move_by(&mut self, m: Vec2, g: &GroundGrid) {
        let d = geo::delta(g.origin, self.pose.target);
        let side = (g.side - 1) as f32;
        let (x, y) = ((d.x + m.x).clamp(0.0, side), (d.y + m.y).clamp(0.0, side));
        self.pose = crate::valley::pose_at(g, x, y, self.pose.yaw, self.pose.zoom);
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
