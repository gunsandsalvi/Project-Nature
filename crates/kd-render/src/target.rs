//! Render targets and GL helpers (A11.2), ported from the mockup's `GLX`: `X.texture`, `X.target`, `X.resizeTarget`,
//! `X.bind` and `X.quad`.
//! Implements `PRE-22` in part, see A11.2.

use crate::RenderError;
use glow::HasContext;

/// The art target's size for a window of `w` by `h` screen pixels at `s` screen pixels an art pixel (A11.2):
/// `ceil(W/s) + 2` by `ceil(H/s) + 2`, the border carrying the sub-pixel shift. The scale never changes with the
/// orientation, so a pixel keeps its size when the phone turns (`PRE-22`).
pub fn art_size(w: u32, h: u32, s: u32) -> (u32, u32) {
    let s = s.max(1);
    (w.div_ceil(s) + 2, h.div_ceil(s) + 2)
}

/// A texture, nearest-filtered and clamped (`X.texture`); `data` is row 0 first.
///
/// # Safety
/// Needs a current GL context on this thread.
#[allow(unsafe_code)]
pub unsafe fn texture(
    gl: &glow::Context,
    w: u32,
    h: u32,
    internal: u32,
    format: u32,
    data: Option<&[u8]>,
) -> Result<glow::Texture, RenderError> {
    unsafe {
        let t = gl.create_texture().map_err(RenderError::Resource)?;
        gl.bind_texture(glow::TEXTURE_2D, Some(t));
        gl.pixel_store_i32(glow::UNPACK_ALIGNMENT, 1);
        gl.tex_image_2d(
            glow::TEXTURE_2D,
            0,
            internal as i32,
            w as i32,
            h as i32,
            0,
            format,
            glow::UNSIGNED_BYTE,
            glow::PixelUnpackData::Slice(data),
        );
        for (p, v) in [
            (glow::TEXTURE_MIN_FILTER, glow::NEAREST),
            (glow::TEXTURE_MAG_FILTER, glow::NEAREST),
            (glow::TEXTURE_WRAP_S, glow::CLAMP_TO_EDGE),
            (glow::TEXTURE_WRAP_T, glow::CLAMP_TO_EDGE),
        ] {
            gl.tex_parameter_i32(glow::TEXTURE_2D, p, v as i32);
        }
        Ok(t)
    }
}

/// A framebuffer with an RGBA8 colour texture and, for the art target, a depth and stencil renderbuffer.
pub struct Target {
    pub fb: glow::Framebuffer,
    pub tex: Option<glow::Texture>,
    pub rb: Option<glow::Renderbuffer>,
    pub w: u32,
    pub h: u32,
}

impl Target {
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn new(gl: &glow::Context, w: u32, h: u32, depth: bool) -> Result<Target, RenderError> {
        unsafe {
            let fb = gl.create_framebuffer().map_err(RenderError::Resource)?;
            let rb = if depth {
                Some(gl.create_renderbuffer().map_err(RenderError::Resource)?)
            } else {
                None
            };
            let mut t = Target {
                fb,
                tex: None,
                rb,
                w: 0,
                h: 0,
            };
            t.resize(gl, w, h)?;
            Ok(t)
        }
    }

    /// Remakes the colour texture and depth storage at a new size (`X.resizeTarget`); the same size does nothing.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn resize(&mut self, gl: &glow::Context, w: u32, h: u32) -> Result<(), RenderError> {
        if self.w == w && self.h == h {
            return Ok(());
        }
        unsafe {
            self.w = w;
            self.h = h;
            if let Some(t) = self.tex.take() {
                gl.delete_texture(t);
            }
            let tex = texture(gl, w, h, glow::RGBA8, glow::RGBA, None)?;
            self.tex = Some(tex);
            gl.bind_framebuffer(glow::FRAMEBUFFER, Some(self.fb));
            gl.framebuffer_texture_2d(
                glow::FRAMEBUFFER,
                glow::COLOR_ATTACHMENT0,
                glow::TEXTURE_2D,
                Some(tex),
                0,
            );
            if let Some(rb) = self.rb {
                gl.bind_renderbuffer(glow::RENDERBUFFER, Some(rb));
                gl.renderbuffer_storage(glow::RENDERBUFFER, glow::DEPTH24_STENCIL8, w as i32, h as i32);
                gl.framebuffer_renderbuffer(
                    glow::FRAMEBUFFER,
                    glow::DEPTH_STENCIL_ATTACHMENT,
                    glow::RENDERBUFFER,
                    Some(rb),
                );
            }
            let st = gl.check_framebuffer_status(glow::FRAMEBUFFER);
            gl.bind_framebuffer(glow::FRAMEBUFFER, None);
            if st != glow::FRAMEBUFFER_COMPLETE {
                return Err(RenderError::Resource(format!("framebuffer incomplete {st:#x}")));
            }
        }
        Ok(())
    }

    /// Binds this target with a viewport over it (`X.bind`).
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn bind(&self, gl: &glow::Context) {
        unsafe {
            gl.bind_framebuffer(glow::FRAMEBUFFER, Some(self.fb));
            gl.viewport(0, 0, self.w as i32, self.h as i32);
        }
    }
}

/// Binds the window with a viewport over it (`X.bind(null)`).
///
/// # Safety
/// Needs a current GL context on this thread.
#[allow(unsafe_code)]
pub unsafe fn bind_window(gl: &glow::Context, w: u32, h: u32) {
    unsafe {
        gl.bind_framebuffer(glow::FRAMEBUFFER, None);
        gl.viewport(0, 0, w as i32, h as i32);
    }
}

#[cfg(test)]
mod tests {
    // checks: PRE-22 PLT-02
    #[test]
    fn art_size() {
        assert_eq!(super::art_size(1080, 2404, 4), (272, 603));
        assert_eq!(super::art_size(2404, 1080, 4), (603, 272));
        assert_eq!(super::art_size(412, 860, 4), (105, 217));
        assert_eq!(super::art_size(1081, 2405, 4), (273, 604));
    }
}
