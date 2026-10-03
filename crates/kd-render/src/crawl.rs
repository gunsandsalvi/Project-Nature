//! Pass 4, the crawling-pixel slot (A11.10), and B66's crawl counter. Pixels still crawl while the camera turns or
//! zooms; the fix is chosen with the owner at the first visual review, so until then the slot holds `Base`, which
//! passes the post target through. The counter, ported from `pretests/b66-drawing/drawing-test.html`'s `capture`,
//! `compose`, `worldAt` and `crawlPair`, counts art pixels that change colour although the surface under them moved
//! less than one art pixel since the last frame; `window.kd.crawl` runs it every alpha (A11.10).
//! Implements `PRE-22` in part, see A11.10.

use crate::camera::Camera;
use crate::target::Target;
use glow::HasContext;
use kd_view::CameraPose;

/// The fixes B66 measured (A11.10); the slot holds `Base` until the owner's review.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum CrawlFix {
    Base,
    Steps,
    Fade {
        turn_deg: f32,
        zoom_frac: f32,
        fade_ms: u16,
    },
    Majority,
    Sticky,
}

/// A crawl fix: may snap the camera's turn and zoom, sample more finely, and resolve the post target into `out`.
pub trait CrawlSlot {
    /// The pose to draw for the pose asked (Steps and Fade snap turn and zoom).
    fn quantise(&mut self, want: CameraPose) -> CameraPose;
    /// Samples per axis (2 for Majority and Sticky).
    fn samples(&self) -> u32;
    /// Pass 4: from the post target, and the last frame's, into `out`.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    unsafe fn resolve(&mut self, gl: &glow::Context, post: &Target, prev: &Target, out: &Target, now_ms: f32);
}

/// No fix: the pose as asked, one sample, the post target copied through.
pub struct Base;

impl CrawlSlot for Base {
    fn quantise(&mut self, want: CameraPose) -> CameraPose {
        want
    }

    fn samples(&self) -> u32 {
        1
    }

    #[allow(unsafe_code)]
    unsafe fn resolve(&mut self, gl: &glow::Context, post: &Target, _prev: &Target, out: &Target, _now_ms: f32) {
        unsafe {
            gl.bind_framebuffer(glow::READ_FRAMEBUFFER, Some(post.fb));
            gl.bind_framebuffer(glow::DRAW_FRAMEBUFFER, Some(out.fb));
            let (w, h) = (post.w as i32, post.h as i32);
            gl.blit_framebuffer(0, 0, w, h, 0, 0, w, h, glow::COLOR_BUFFER_BIT, glow::NEAREST);
            gl.bind_framebuffer(glow::READ_FRAMEBUFFER, None);
            gl.bind_framebuffer(glow::DRAW_FRAMEBUFFER, None);
        }
    }
}

/// One frame for the counter (B66's `capture`): the post target's colours and the scene's 16-bit view depth per art
/// pixel, rows from the bottom, and the frame's camera.
#[derive(Clone, Debug)]
pub struct Capture {
    pub w: u32,
    pub h: u32,
    pub col: Vec<u32>,
    pub dep16: Vec<u16>,
    pub cam: Camera,
}

/// What the screen showed on the frame's art grid (B66's `compose`, without a fade): colour and surface position
/// per art pixel, `None` where nothing was drawn.
pub struct Shown {
    w: u32,
    h: u32,
    col: Vec<u32>,
    wp: Vec<Option<[f32; 3]>>,
    cam: Camera,
}

/// The surface point at art pixel (`x`, `y`) with view depth `d` (0–1): B66's `worldAt`.
fn world_at(c: &Camera, x: u32, y: u32, d: f32) -> [f32; 3] {
    let u = c.left + (x as f32 + 0.5) * c.texel;
    let v = c.bottom + (y as f32 + 0.5) * c.texel;
    let w = c.near + d * (c.far - c.near);
    [0, 1, 2].map(|k| c.r[k] * u + c.u[k] * v + c.f[k] * w)
}

/// The frame as shown.
pub fn compose(cap: &Capture) -> Shown {
    let wp = (0..cap.w * cap.h)
        .map(|i| {
            let d = cap.dep16[i as usize];
            (d < u16::MAX).then(|| world_at(&cap.cam, i % cap.w, i / cap.w, f32::from(d) / 65535.0))
        })
        .collect();
    Shown {
        w: cap.w,
        h: cap.h,
        col: cap.col.clone(),
        wp,
        cam: cap.cam,
    }
}

/// B66's counts for one pair of frames: art pixels with a surface in both (`valid`), whose surface moved under one
/// art pixel (`elig`), and of those that changed colour, all (`changed`) and the small-move ones (`crawl`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct PairCount {
    pub valid: u64,
    pub elig: u64,
    pub crawl: u64,
    pub changed: u64,
}

/// B66's `crawlPair`: frame `a` then frame `b`.
pub fn pair(a: &Shown, b: &Shown) -> PairCount {
    let c = &b.cam;
    let (dox, doy) = (b.cam.off[0] - a.cam.off[0], b.cam.off[1] - a.cam.off[1]);
    // `a`'s surface points are from its floating origin; `b`'s camera works from its own
    let o = kd_core::geo::delta(b.cam.origin, a.cam.origin);
    let mut n = PairCount::default();
    for y in 0..a.h {
        for x in 0..a.w {
            let i = (y * a.w + x) as usize;
            let Some(w) = a.wp[i] else { continue };
            let w = [w[0] + o.x, w[1], w[2] + o.y];
            let x1 = (x as f32 + 0.5 + dox).floor();
            let y1 = (y as f32 + 0.5 + doy).floor();
            if x1 < 1.0 || y1 < 1.0 || x1 >= (b.w - 1) as f32 || y1 >= (b.h - 1) as f32 {
                continue;
            }
            n.valid += 1;
            let j = (y1 as u32 * b.w + x1 as u32) as usize;
            let changed = a.col[i] != b.col[j];
            let qx = (c.r[0] * w[0] + c.r[1] * w[1] + c.r[2] * w[2] - c.left) / c.texel;
            let qy = (c.u[0] * w[0] + c.u[1] * w[1] + c.u[2] * w[2] - c.bottom) / c.texel;
            let mx = (qx - b.cam.off[0]) - (x as f32 + 0.5 - a.cam.off[0]);
            let my = (qy - b.cam.off[1]) - (y as f32 + 0.5 - a.cam.off[1]);
            let small = mx * mx + my * my < 1.0;
            n.elig += u64::from(small);
            if changed {
                n.changed += 1;
                n.crawl += u64::from(small);
            }
        }
    }
    n
}

#[cfg(test)]
mod tests;
