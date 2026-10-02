//! Pass 1, the sun's shadows (A11.2, A11.4): the shadow camera, ported from the mockup's `lightFor`, fits the sun's
//! orthographic frustum round the camera's footprint and snaps it to its own texels, so shadows never swim; the
//! scene shaders draw under `#define SHADOW` into a 2,048² 24-bit depth texture sampled directly (the mockup's
//! `packShadow` was only for WebGL1). As with the camera's blocks, the projection is kept while the map's window
//! moves within a block of texels, its viewport moving by whole texels, so a pan stores the very same depths.
//! Implements `PRE-30` in part, see A11.2.

use crate::RenderError;
use crate::camera::dot64;
use crate::mat::Mat4;
use glow::HasContext;
use kd_core::m;

/// The shadow map's side in texels (A11.2).
pub const SHADOW_SIZE: u32 = 2048;
/// Shadows are drawn while art pixels are under this many metres (A11.2) and the sun this far up (its direction's
/// upward part).
pub const SHADOW_MAX_TEXEL: f32 = 3.2;
pub const SHADOW_MIN_SUN_UP: f32 = 0.02;
/// The shadow camera's depth range moves in steps of this many metres from the world's corner.
const DEPTH_STEP_M: f64 = 16.0;
/// Texels in a block of the map's window, like the camera's `BLOCK`: the shadow pass's viewport is the map plus a
/// block, 2,304 texels, within OpenGL ES 3.0's least viewport size on the phone (its screen's 2,404 pixels).
pub const LIGHT_BLOCK: i64 = 256;

/// The sun's camera for one frame: its view-projection, over the map's span plus a block, metres per shadow texel,
/// its depth range, and the shadow pass's viewport (x, y, width and height in texels of the map).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct LightCam {
    pub vp: Mat4,
    pub texel: f32,
    pub depth_r: f32,
    pub view: [i32; 4],
}

impl LightCam {
    /// The shader's `uShadowBias` (the mockup's): along the normal, and in depth.
    pub fn bias(&self) -> [f32; 2] {
        [(self.texel * 1.5).max(0.015), 2.5 * self.texel / self.depth_r]
    }

    /// The shader's `uShadowWin`: texels across the projection, and where the map's corner lies in it.
    pub fn window(&self) -> [f32; 3] {
        [self.view[2] as f32, -self.view[0] as f32, -self.view[1] as f32]
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

/// The sun's camera fitted round `foot`, the view's corners on the lowest and highest ground in metres from the
/// floating origin, which lies at `om` from the world's corner, for light from direction `l` (toward the sun, GPU
/// axes); the mockup's `lightFor`, with its texels counted from the world's corner, and its depth range, on whole
/// steps from there, over `block`, the corners of the camera's block, so a pan within both blocks keeps it all.
pub fn light_for(foot: &[[f32; 3]; 8], block: &[[f32; 3]; 8], l: [f32; 3], om: [f64; 3]) -> LightCam {
    let fl = [-l[0], -l[1], -l[2]];
    let mut rl = cross(fl, [0.0, 1.0, 0.0]);
    if dot(rl, rl).sqrt() < 1e-3 {
        rl = [1.0, 0.0, 0.0];
    }
    let rl = norm(rl);
    let ul = cross(rl, fl);
    let (mut x0, mut x1, mut y0, mut y1) = (f32::MAX, f32::MIN, f32::MAX, f32::MIN);
    for &p in foot {
        let (a, b) = (dot(rl, p), dot(ul, p));
        x0 = x0.min(a);
        x1 = x1.max(a);
        y0 = y0.min(b);
        y1 = y1.max(b);
    }
    // the side in steps of an eighth of a power of two, so small moves keep it
    let ext = (x1 - x0).max(y1 - y0) * 1.04;
    let ext = m::exp2((m::log2(ext) * 8.0).ceil() / 8.0);
    let tl = ext / SHADOW_SIZE as f32;
    // the map's middle on whole texels from the world's corner, and the block it lies in
    let (ox, oy, od, tl64) = (dot64(rl, om), dot64(ul, om), dot64(fl, om), f64::from(tl));
    let cx = ((f64::from((x0 + x1) / 2.0) + ox) / tl64).floor() as i64;
    let cy = ((f64::from((y0 + y1) / 2.0) + oy) / tl64).floor() as i64;
    let (bx, by) = (
        cx.div_euclid(LIGHT_BLOCK) * LIGHT_BLOCK,
        cy.div_euclid(LIGHT_BLOCK) * LIGHT_BLOCK,
    );
    // the projection: the map's span plus a block, from the block's corner
    let half = i64::from(SHADOW_SIZE / 2);
    let span_px = i64::from(SHADOW_SIZE) + LIGHT_BLOCK;
    let left = ((bx - half) as f64 * tl64 - ox) as f32;
    let bottom = ((by - half) as f64 * tl64 - oy) as f32;
    let span = span_px as f32 * tl;
    let (mut d0, mut d1) = (f32::MAX, f32::MIN);
    for &p in block {
        d0 = d0.min(dot(fl, p));
        d1 = d1.max(dot(fl, p));
    }
    let step = |v: f32, up: bool| {
        let k = (f64::from(v) + od) / DEPTH_STEP_M;
        ((if up { k.ceil() } else { k.floor() }) * DEPTH_STEP_M - od) as f32
    };
    let (near, far) = (step(d0 - 260.0, false), step(d1 + 20.0, true));
    let (rw, rd) = (2.0 / span, 2.0 / (far - near));
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
        view: [-((cx - bx) as i32), -((cy - by) as i32), span_px as i32, span_px as i32],
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

    /// Binds the shadow map with the light's viewport, clears its depth and readies depth testing.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn begin(&self, gl: &glow::Context, light: &LightCam) {
        unsafe {
            gl.bind_framebuffer(glow::FRAMEBUFFER, Some(self.fb));
            let [x, y, w, h] = light.view;
            gl.viewport(x, y, w, h);
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
