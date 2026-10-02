//! The valley (α01b): the demo area made into the picture's ground, each square's surface chosen from its material
//! and slope (T01b.4: soil to grass under 30°, dirt from 30° to 45°, rock layers to rock above 45°, scree at the
//! cliff's foot), and where the camera starts.
//! Implements `PRE-02` and `PRE-20` in part.

use kd_core::geo::{AreaId, Pos};
use kd_data::Catalogue;
use kd_view::{CameraPose, GroundGrid};
use kd_world::area::demo::{self, SIDE, SQUARES};

/// Where the demo area lies in the world: an area at about 21° north (A3.7).
pub const DEMO_AREA: AreaId = AreaId(1_536 * 8_000 + 4_096);
/// B11's test site's seed.
pub const DEMO_SEED: u64 = 99;
/// The square of tan 30°: soil steeper than this shows as bare dirt.
const TAN2_30: f32 = 1.0 / 3.0;
/// The camera's start: just north of the cliff, looking south-south-east at its face, with art pixels of about a
/// quarter metre, so the 256 m of ground fills a portrait screen.
pub const START_AT: (f32, f32) = (128.0, 118.0);
pub const START_YAW: f32 = 3.74;
pub const START_ZOOM: f32 = 0.19;

/// The demo area as the picture's ground.
pub fn ground(cat: &Catalogue) -> GroundGrid {
    let a = demo::demo_area(DEMO_SEED);
    let heights_m: Vec<f32> = (0..SIDE * SIDE).map(|k| a.height_m(k % SIDE, k / SIDE)).collect();
    let s = |id: &str| cat.surface(id).unwrap_or(0);
    let (grass, dirt, rock, scree) = (s("grass"), s("dirt"), s("rock"), s("scree"));
    let surface = (0..SQUARES * SQUARES)
        .map(|k| {
            let (i, j) = (k % SQUARES, k / SQUARES);
            match a.material[k] {
                demo::SCREE => scree,
                1..=6 => rock,
                _ => {
                    let h = |x: usize, y: usize| heights_m[y * SIDE + x];
                    let gx = (h(i + 1, j) + h(i + 1, j + 1) - h(i, j) - h(i, j + 1)) * 0.5;
                    let gy = (h(i, j + 1) + h(i + 1, j + 1) - h(i, j) - h(i + 1, j)) * 0.5;
                    if gx * gx + gy * gy < TAN2_30 { grass } else { dirt }
                }
            }
        })
        .collect();
    GroundGrid {
        origin: DEMO_AREA.origin(),
        side: SIDE,
        heights_m,
        surface,
    }
}

/// The ground's height at (`x`, `y`) metres from its corner, between its points; the edge beyond.
pub fn height_at(g: &GroundGrid, x: f32, y: f32) -> f32 {
    let last = (g.side - 1) as f32;
    let (x, y) = (x.clamp(0.0, last), y.clamp(0.0, last));
    let (i, j) = ((x as usize).min(g.side - 2), (y as usize).min(g.side - 2));
    let (u, v) = (x - i as f32, y - j as f32);
    let h = |a: usize, b: usize| g.heights_m[b * g.side + a];
    let top = h(i, j) + (h(i + 1, j) - h(i, j)) * u;
    let bot = h(i, j + 1) + (h(i + 1, j + 1) - h(i, j + 1)) * u;
    top + (bot - top) * v
}

/// The pose looking at (`x`, `y`) metres from the ground's corner, at the ground's height there.
pub fn pose_at(g: &GroundGrid, x: f32, y: f32, yaw: f32, zoom: f32) -> CameraPose {
    let p = kd_core::geo::offset(g.origin, kd_core::geo::Vec2 { x, y });
    CameraPose {
        target: Pos {
            z: (height_at(g, x, y) * 256.0) as i32,
            ..p
        },
        yaw,
        zoom,
    }
}

/// The camera's start (`START_AT`, `START_YAW`, `START_ZOOM`).
pub fn start(g: &GroundGrid) -> CameraPose {
    pose_at(g, START_AT.0, START_AT.1, START_YAW, START_ZOOM)
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-20 PRE-02
    #[test]
    fn every_surface_shows() {
        let cat = Catalogue::load(crate::CATALOGUE).unwrap();
        let g = ground(&cat);
        assert_eq!(g.heights_m.len(), SIDE * SIDE);
        for id in ["grass", "dirt", "rock", "scree"] {
            let k = cat.surface(id).unwrap();
            assert!(g.surface.contains(&k), "no {id}");
        }
        // the start looks at the ground by the cliff, at the ground's height there
        let c = start(&g);
        let d = kd_core::geo::delta(g.origin, c.target);
        assert_eq!((d.x, d.y), START_AT);
        assert_eq!(c.target.z, (height_at(&g, START_AT.0, START_AT.1) * 256.0) as i32);
    }
}
