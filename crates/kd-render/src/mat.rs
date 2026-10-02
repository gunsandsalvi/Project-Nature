//! 4 × 4 matrices, column-major `[f32; 16]` as GLSL reads them.

use kd_core::m;

/// A column-major 4 × 4 matrix: entry `[c * 4 + r]` is column `c`, row `r`.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Mat4(pub [f32; 16]);

impl Mat4 {
    pub const IDENTITY: Mat4 = Mat4([
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
    ]);

    /// A perspective projection: vertical field of view in radians, width over height, near and far planes.
    pub fn perspective(fovy: f32, aspect: f32, near: f32, far: f32) -> Mat4 {
        let f = 1.0 / m::tan(fovy * 0.5);
        let mut r = [0.0; 16];
        r[0] = f / aspect;
        r[5] = f;
        r[10] = (far + near) / (near - far);
        r[11] = -1.0;
        r[14] = 2.0 * far * near / (near - far);
        Mat4(r)
    }

    /// A move by (x, y, z).
    pub fn translate(x: f32, y: f32, z: f32) -> Mat4 {
        let mut r = Mat4::IDENTITY.0;
        r[12] = x;
        r[13] = y;
        r[14] = z;
        Mat4(r)
    }

    /// A turn about the x axis, in radians.
    pub fn rot_x(a: f32) -> Mat4 {
        let (s, c) = (m::sin(a), m::cos(a));
        Mat4([1.0, 0.0, 0.0, 0.0, 0.0, c, s, 0.0, 0.0, -s, c, 0.0, 0.0, 0.0, 0.0, 1.0])
    }

    /// A turn about the y axis, in radians.
    pub fn rot_y(a: f32) -> Mat4 {
        let (s, c) = (m::sin(a), m::cos(a));
        Mat4([c, 0.0, -s, 0.0, 0.0, 1.0, 0.0, 0.0, s, 0.0, c, 0.0, 0.0, 0.0, 0.0, 1.0])
    }

    /// `self × o`: `o` applies first.
    pub fn mul(&self, o: &Mat4) -> Mat4 {
        let (a, b) = (&self.0, &o.0);
        let mut r = [0.0; 16];
        for c in 0..4 {
            for row in 0..4 {
                let mut s = 0.0;
                for k in 0..4 {
                    s += a[k * 4 + row] * b[c * 4 + k];
                }
                r[c * 4 + row] = s;
            }
        }
        Mat4(r)
    }

    /// The normal matrix, column-major 3 × 3: the upper-left block, exact for turns and moves (no scaling).
    pub fn normal3(&self) -> [f32; 9] {
        let a = &self.0;
        [a[0], a[1], a[2], a[4], a[5], a[6], a[8], a[9], a[10]]
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRC-11
    #[test]
    fn perspective_known() {
        let aspect = 0.5;
        let p = Mat4::perspective(40f32.to_radians(), aspect, 0.1, 10.0);
        assert_eq!(p.0[11], -1.0);
        let want = 1.0 / (aspect as f64 * (20f64.to_radians()).tan());
        assert!(
            (p.0[0] as f64 - want).abs() < 1e-6 * want.max(1.0),
            "{} {}",
            p.0[0],
            want
        );
    }

    // checks: PRC-11
    #[test]
    fn mul_by_identity_and_turns() {
        let r = Mat4::rot_y(0.3).mul(&Mat4::rot_x(0.2));
        assert_eq!(r.mul(&Mat4::IDENTITY), r);
        let back = Mat4::rot_y(0.3).mul(&Mat4::rot_y(-0.3));
        for (i, v) in back.0.iter().enumerate() {
            assert!((v - Mat4::IDENTITY.0[i]).abs() < 1e-6);
        }
    }
}
