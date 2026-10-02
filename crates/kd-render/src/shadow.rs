//! Pass 1, the sun's shadows (A11.2, A11.4): the shadow camera, ported from the mockup's `lightFor`, fits the sun's
//! orthographic frustum round the camera's footprint and snaps it to its own texels, so shadows never swim; the
//! scene shaders draw under `#define SHADOW` into a 2,048² 24-bit depth texture sampled directly (the mockup's
//! `packShadow` was only for WebGL1).
//! Implements `PRE-30` in part, see A11.2.

use crate::RenderError;
use crate::mat::Mat4;
use glow::HasContext;
use kd_core::m;

/// The shadow map's side in texels (A11.2).
pub const SHADOW_SIZE: u32 = 2048;
/// Shadows are drawn while art pixels are under this many metres (A11.2) and the sun this far up (its direction's
/// upward part).
pub const SHADOW_MAX_TEXEL: f32 = 3.2;
pub const SHADOW_MIN_SUN_UP: f32 = 0.02;

/// The sun's camera for one frame: its view-projection, metres per shadow texel and its depth range.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct LightCam {
    pub vp: Mat4,
    pub texel: f32,
    pub depth_r: f32,
}

impl LightCam {
    /// The shader's `uShadowBias` (the mockup's): along the normal, and in depth.
    pub fn bias(&self) -> [f32; 2] {
        [(self.texel * 1.5).max(0.015), 2.5 * self.texel / self.depth_r]
    }
}

fn dot(a: [f32; 3], b: [f32; 3]) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}

fn cross(a: [f32; 3], b: [f32; 3]) -> [f32; 3] {
    [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ]
}

fn norm(a: [f32; 3]) -> [f32; 3] {
    let l = dot(a, a).sqrt();
    [a[0] / l, a[1] / l, a[2] / l]
}

/// The sun's camera fitted round `foot`, the view's corners on the lowest and highest ground, for light from
/// direction `l` (toward the sun, GPU axes); the mockup's `lightFor`.
pub fn light_for(foot: &[[f32; 3]; 8], l: [f32; 3]) -> LightCam {
    let fl = [-l[0], -l[1], -l[2]];
    let mut rl = cross(fl, [0.0, 1.0, 0.0]);
    if dot(rl, rl).sqrt() < 1e-3 {
        rl = [1.0, 0.0, 0.0];
    }
    let rl = norm(rl);
    let ul = cross(rl, fl);
    let (mut x0, mut x1, mut y0, mut y1, mut d0, mut d1) = (f32::MAX, f32::MIN, f32::MAX, f32::MIN, f32::MAX, f32::MIN);
    for &p in foot {
        let (a, b, d) = (dot(rl, p), dot(ul, p), dot(fl, p));
        x0 = x0.min(a);
        x1 = x1.max(a);
        y0 = y0.min(b);
        y1 = y1.max(b);
        d0 = d0.min(d);
        d1 = d1.max(d);
    }
    // the side in steps of an eighth of a power of two, so small moves keep it
    let ext = (x1 - x0).max(y1 - y0) * 1.04;
    let ext = m::exp2((m::log2(ext) * 8.0).ceil() / 8.0);
    let tl = ext / SHADOW_SIZE as f32;
    let cxl = ((x0 + x1) / 2.0 / tl).floor() * tl;
    let cyl = ((y0 + y1) / 2.0 / tl).floor() * tl;
    let (left, bottom) = (cxl - ext / 2.0, cyl - ext / 2.0);
    let (near, far) = (d0 - 260.0, d1 + 20.0);
    let (rw, rd) = (2.0 / ext, 2.0 / (far - near));
    let rows = [
        [rl[0] * rw, rl[1] * rw, rl[2] * rw, -left * rw - 1.0],
        [ul[0] * rw, ul[1] * rw, ul[2] * rw, -bottom * rw - 1.0],
        [fl[0] * rd, fl[1] * rd, fl[2] * rd, -near * rd - 1.0],
        [0.0, 0.0, 0.0, 1.0],
    ];
    let mut vp = [0.0; 16];
    for (row, v) in rows.iter().enumerate() {
        for c in 0..4 {
            vp[c * 4 + row] = v[c];
        }
    }
    LightCam {
        vp: Mat4(vp),
        texel: tl,
        depth_r: far - near,
    }
}

/// The shadow map: a depth texture and the framebuffer that draws into it.
pub struct ShadowMap {
    pub fb: glow::Framebuffer,
    pub tex: glow::Texture,
}

impl ShadowMap {
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn new(gl: &glow::Context) -> Result<ShadowMap, RenderError> {
        unsafe {
            let tex = gl.create_texture().map_err(RenderError::Resource)?;
            gl.bind_texture(glow::TEXTURE_2D, Some(tex));
            gl.tex_image_2d(
                glow::TEXTURE_2D,
                0,
                glow::DEPTH_COMPONENT24 as i32,
                SHADOW_SIZE as i32,
                SHADOW_SIZE as i32,
                0,
                glow::DEPTH_COMPONENT,
                glow::UNSIGNED_INT,
                glow::PixelUnpackData::Slice(None),
            );
            for (p, v) in [
                (glow::TEXTURE_MIN_FILTER, glow::NEAREST),
                (glow::TEXTURE_MAG_FILTER, glow::NEAREST),
                (glow::TEXTURE_WRAP_S, glow::CLAMP_TO_EDGE),
                (glow::TEXTURE_WRAP_T, glow::CLAMP_TO_EDGE),
            ] {
                gl.tex_parameter_i32(glow::TEXTURE_2D, p, v as i32);
            }
            let fb = gl.create_framebuffer().map_err(RenderError::Resource)?;
            gl.bind_framebuffer(glow::FRAMEBUFFER, Some(fb));
            gl.framebuffer_texture_2d(
                glow::FRAMEBUFFER,
                glow::DEPTH_ATTACHMENT,
                glow::TEXTURE_2D,
                Some(tex),
                0,
            );
            gl.draw_buffers(&[glow::NONE]);
            gl.read_buffer(glow::NONE);
            let st = gl.check_framebuffer_status(glow::FRAMEBUFFER);
            gl.bind_framebuffer(glow::FRAMEBUFFER, None);
            if st != glow::FRAMEBUFFER_COMPLETE {
                return Err(RenderError::Resource(format!("shadow framebuffer incomplete {st:#x}")));
            }
            Ok(ShadowMap { fb, tex })
        }
    }

    /// Binds the shadow map, clears its depth and readies depth testing.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn begin(&self, gl: &glow::Context) {
        unsafe {
            gl.bind_framebuffer(glow::FRAMEBUFFER, Some(self.fb));
            gl.viewport(0, 0, SHADOW_SIZE as i32, SHADOW_SIZE as i32);
            gl.clear_depth_f32(1.0);
            gl.enable(glow::DEPTH_TEST);
            gl.depth_func(glow::LEQUAL);
            gl.depth_mask(true);
            gl.clear(glow::DEPTH_BUFFER_BIT);
        }
    }
}

#[cfg(test)]
mod tests;
