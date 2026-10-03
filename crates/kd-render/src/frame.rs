//! One frame's plan from its inputs, on the CPU (A11.13): the light at the frame's sky, kept while the sun moves
//! under 0.01°, and the palette row and tables, which change only when one of the row's colours would move by a
//! whole 1/255 (A11.3), so the textures upload a few times a minute at most.

use kd_data::Catalogue;
use kd_view::SkyView;

use crate::light::{Light, light};
use crate::looks::{Layout, Palette};

/// The view the haze's colour is seen along until the camera arrives (α01b): north and 52° down, the camp stop's
/// pitch (A11.2).
pub const VIEW: [f32; 3] = [0.0, 0.615_661_5, -0.788_010_8];

/// How far the sun may move before the light is worked out again: 0.01°, as a distance between unit vectors.
const SUN_STEP: f32 = 0.01 * std::f32::consts::PI / 180.0;

/// The frame's light and palette, kept from frame to frame (A11.13 rule 4: the light's last row).
#[derive(Clone, Debug)]
pub struct Lighting {
    pub sky: SkyView,
    pub light: Light,
    pub palette: Palette,
}

impl Lighting {
    pub fn new(cat: &Catalogue, layout: &Layout, sky: &SkyView) -> Lighting {
        let light = light(&cat.air, sky);
        let palette = Palette::new(cat, layout, &light, VIEW);
        Lighting {
            sky: *sky,
            light,
            palette,
        }
    }

    /// Follows the frame's sky: the light when the sun has moved 0.01° or the air changed, and the palette when a
    /// colour of its row or tables changed; returns whether the palette's textures must upload.
    pub fn follow(&mut self, cat: &Catalogue, layout: &Layout, sky: &SkyView) -> bool {
        let moved: f32 = (0..3)
            .map(|i| {
                let d = sky.sun_dir[i] - self.sky.sun_dir[i];
                d * d
            })
            .sum();
        if moved < SUN_STEP * SUN_STEP && sky.turbidity == self.sky.turbidity {
            return false;
        }
        self.sky = *sky;
        self.light = light(&cat.air, sky);
        let palette = Palette::new(cat, layout, &self.light, VIEW);
        let changed = palette.row != self.palette.row || palette.tables != self.palette.tables;
        // The range is a uniform, set every frame, so it follows the light even when the row holds.
        self.palette = palette;
        changed
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::light::tests::sky_at;

    // checks: PRE-01 PRE-30
    #[test]
    fn row_follows_the_light() {
        let cat = crate::tests::catalogue();
        let layout = Layout::new(&cat).unwrap();
        let mut l = Lighting::new(&cat, &layout, &sky_at(16.5));
        let first = l.palette.clone();
        // The same sky, or the sun moved by a few thousandths of a degree: nothing to upload.
        assert!(!l.follow(&cat, &layout, &sky_at(16.5)));
        assert!(!l.follow(&cat, &layout, &sky_at(16.5 + 0.0005)));
        assert_eq!(l.palette, first);
        // An hour later the row moves, and uploads.
        assert!(l.follow(&cat, &layout, &sky_at(17.5)));
        assert_ne!(l.palette.row, first.row);
        // Ten seconds on, the sun has moved 0.04°: the light is worked out again, and the row uploads only if one of
        // its colours moved by a whole 1/255.
        let held = l.palette.clone();
        let later = sky_at(17.5 + 10.0 / 3600.0);
        let changed = l.follow(&cat, &layout, &later);
        assert_eq!(l.sky, later);
        assert_ne!(l.palette.range, held.range);
        assert_eq!(changed, l.palette.row != held.row || l.palette.tables != held.tables);
    }
}
