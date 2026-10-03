//! One frame's plan from its inputs, on the CPU (A11.13): the light at the frame's sky, kept while the sun moves
//! under 0.01°, and the palette row and tables, which change only when one of the row's colours would move by a
//! whole 1/255 (A11.3), so the textures upload a few times a minute at most; the haze's tables also follow the way
//! the camera looks, since the haze is warmer toward the sun, when its colour moves by a whole 1/255 (A11.4).

use kd_data::Catalogue;
use kd_view::SkyView;

use crate::light::{Light, haze_air, haze_colour, light, shade};
use crate::looks::{Layout, Palette};

/// The view the haze's colour is seen along before a camera looks (the light card): north and 52° down, the camp
/// stop's pitch (A11.2).
pub const VIEW: [f32; 3] = [0.0, 0.615_661_5, -0.788_010_8];

/// How far the sun may move before the light is worked out again: 0.01°, as a distance between unit vectors.
const SUN_STEP: f32 = 0.01 * std::f32::consts::PI / 180.0;

/// The frame's light and palette, kept from frame to frame (A11.13 rule 4: the light's last row).
#[derive(Clone, Debug)]
pub struct Lighting {
    pub sky: SkyView,
    pub light: Light,
    pub palette: Palette,
    /// The way the haze's colour was seen, and that colour as shown.
    pub view: [f32; 3],
    haze_shown: [u8; 3],
    /// The air the haze sees: extinction a metre at the sea's level and scale heights (A11.4).
    pub haze_air: ([f32; 2], [f32; 2]),
}

/// The air's turbidity for a sky: its own once weather sets it, else the catalogue's fine day.
fn turbidity(cat: &Catalogue, sky: &SkyView) -> f32 {
    if sky.turbidity > 0.0 {
        sky.turbidity
    } else {
        cat.air.turbidity
    }
}

impl Lighting {
    /// The light of a sky, with the haze seen along `view` (east, north, up, from the eye into the scene).
    pub fn new(cat: &Catalogue, layout: &Layout, sky: &SkyView, view: [f32; 3]) -> Lighting {
        let light = light(&cat.air, sky);
        let palette = Palette::new(cat, layout, &light, view);
        Lighting {
            sky: *sky,
            haze_shown: shade(&cat.air, &light, [1.0; 3], haze_colour(&cat.air, &light, view)),
            haze_air: haze_air(&cat.air, turbidity(cat, sky)),
            light,
            palette,
            view,
        }
    }

    /// Follows the frame's sky and view: the light when the sun has moved 0.01° or the air changed, the palette
    /// when a colour of its row or tables changed, and the haze's tables when its colour seen along the view moved
    /// by a whole 1/255; returns whether the palette's textures must upload.
    pub fn follow(&mut self, cat: &Catalogue, layout: &Layout, sky: &SkyView, view: [f32; 3]) -> bool {
        let moved: f32 = (0..3)
            .map(|i| {
                let d = sky.sun_dir[i] - self.sky.sun_dir[i];
                d * d
            })
            .sum();
        if moved < SUN_STEP * SUN_STEP && sky.turbidity == self.sky.turbidity {
            if view == self.view {
                return false;
            }
            let shown = shade(
                &cat.air,
                &self.light,
                [1.0; 3],
                haze_colour(&cat.air, &self.light, view),
            );
            self.view = view;
            if shown == self.haze_shown {
                return false;
            }
            self.haze_shown = shown;
            let palette = Palette::new(cat, layout, &self.light, view);
            let changed = palette.tables != self.palette.tables;
            self.palette = palette;
            return changed;
        }
        self.sky = *sky;
        self.view = view;
        self.light = light(&cat.air, sky);
        self.haze_shown = shade(
            &cat.air,
            &self.light,
            [1.0; 3],
            haze_colour(&cat.air, &self.light, view),
        );
        self.haze_air = haze_air(&cat.air, turbidity(cat, sky));
        let palette = Palette::new(cat, layout, &self.light, view);
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
        let mut l = Lighting::new(&cat, &layout, &sky_at(16.5), VIEW);
        let first = l.palette.clone();
        // The same sky, or the sun moved by a few thousandths of a degree: nothing to upload.
        assert!(!l.follow(&cat, &layout, &sky_at(16.5), VIEW));
        assert!(!l.follow(&cat, &layout, &sky_at(16.5 + 0.0005), VIEW));
        assert_eq!(l.palette, first);
        // An hour later the row moves, and uploads.
        assert!(l.follow(&cat, &layout, &sky_at(17.5), VIEW));
        assert_ne!(l.palette.row, first.row);
        // Ten seconds on, the sun has moved 0.04°: the light is worked out again, and the row uploads only if one of
        // its colours moved by a whole 1/255.
        let held = l.palette.clone();
        let later = sky_at(17.5 + 10.0 / 3600.0);
        let changed = l.follow(&cat, &layout, &later, VIEW);
        assert_eq!(l.sky, later);
        assert_ne!(l.palette.range, held.range);
        assert_eq!(changed, l.palette.row != held.row || l.palette.tables != held.tables);
    }
}
