//! Pass 4, the crawl fix (A11.10): pixels still crawl while the camera turns or zooms, and the fix that best lessens
//! it is chosen with the owner at the first visual review, on a real world (`PRE-22`, `PRE-31`). Until then the slot
//! holds `Base`, which takes the camera as asked, one sample an art pixel, and shows post's picture as it is; the
//! other fixes are built from A11.10's descriptions when the review needs them, and the crawl counter
//! (`probe::count`) measures each.
//!
//! Implements PRE-22 in part, see A11.10: the slot the chosen fix goes in.

use kd_view::CameraPose;

/// The fixes the review chooses between (A11.10); `Base` alone is built until then.
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

/// What the slot gives the upscale.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Resolved {
    /// Post's target, as post made it.
    Post,
}

/// A crawl fix (A11.10): how it snaps the camera's turn and zoom, how many samples an art pixel takes along each
/// axis, and what the upscale shows; a fix that blends frames over time gets the frame's time, the GL context and
/// the targets it needs when it is built.
pub trait CrawlSlot {
    fn fix(&self) -> CrawlFix;
    fn quantise(&mut self, want: CameraPose) -> CameraPose;
    fn samples(&self) -> u32;
    fn resolve(&mut self) -> Resolved;
}

/// No fix: the camera as asked, one sample an art pixel, post's picture as it is.
#[derive(Clone, Copy, Debug, Default)]
pub struct Base;

impl CrawlSlot for Base {
    fn fix(&self) -> CrawlFix {
        CrawlFix::Base
    }

    fn quantise(&mut self, want: CameraPose) -> CameraPose {
        want
    }

    fn samples(&self) -> u32 {
        1
    }

    fn resolve(&mut self) -> Resolved {
        Resolved::Post
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use kd_core::geo::Pos;

    // checks: PRE-22
    #[test]
    fn base_passes_through() {
        // The camera as asked, to the bit, one sample a pixel, and post's picture shown as it is, at any time.
        let mut slot: Box<dyn CrawlSlot> = Box::new(Base);
        assert_eq!(slot.fix(), CrawlFix::Base);
        for (i, yaw) in [0.0f32, 0.001_92, 1.2, -3.0].into_iter().enumerate() {
            let pose = CameraPose {
                target: Pos {
                    x: 12_345 + i as i32,
                    y: 678,
                    z: 9,
                },
                yaw,
                zoom: 0.14 + 0.003 * i as f32,
            };
            assert_eq!(slot.quantise(pose), pose);
            assert_eq!(slot.samples(), 1);
            assert_eq!(slot.resolve(), Resolved::Post);
        }
    }
}
