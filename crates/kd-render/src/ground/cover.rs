//! Stones and tufts (A11.5, `PRE-46`'s ground cover): each area's seeded list of small shapes by its surfaces'
//! densities in the catalogue, drawn instanced from a texture of their numbers with no vertex buffer: eight-faced
//! stones 0.05–0.6 m across, and tufts of three to five blades 0.1–0.3 m tall, each blade widened to at least an art
//! pixel. Each has a seeded importance `u` and shows while it spans `1.5 + 2u` art pixels, so as the camera rises
//! they thin out one by one (A11.1 rule 3); their contact shade, the sky their feet hide from the ground round
//! them, is mipmapped as the coverage is, so its average stays when they no longer show (rule 2). They are kept in
//! 16 m buckets, the ground's patches at its finest spacing, stones before tufts, so the buckets a view shows draw
//! as one run a row.
//!
//! Until areas' pictures carry their stones and ground cover (A5.3, α21), the renderer seeds them itself from the
//! area's number: they are the picture's alone, and nothing reads them (`WLD-13`).
//!
//! Implements PRE-46 and WLD-12 in part, see A11.5: stones and tufts on the areas' ground.

use kd_core::geo::{AreaId, Pos};
use kd_core::m;
use kd_core::num::hash2;
use kd_view::AREA_SQUARES;

use super::{Area, MAX_SURFACES, Store, mesh_height};
use crate::RenderError;
use crate::camera::{self, View};
use crate::frame::Lighting;
use crate::gl::{self, Program, State, unit};
use crate::light::{LUM, dot};
use crate::looks::Layout;
use crate::pixel::{SEED_SPLIT, faded_noise, split_look};
use crate::shaders::{self, Stage};

/// Metres along a bucket's side: the ground's patches at its finest spacing (A11.5).
pub const BUCKET_M: usize = 16;
/// Buckets along an area's side.
pub const BUCKETS: usize = AREA_SQUARES / BUCKET_M;
/// A stone's size across, and a tuft's height, in metres (A11.5).
pub const STONE_M: [f32; 2] = [0.05, 0.6];
pub const TUFT_M: [f32; 2] = [0.1, 0.3];
/// Beyond art pixels this large no item spans 1.5 of them, so none is drawn.
pub const COVER_MAX_TEXEL: f32 = STONE_M[1] / 1.5;
/// How far an item reaches beside its foot and above the ground, metres, for culling by the view.
pub const ITEM_REACH_M: f64 = 0.5;
pub const ITEM_RISE_M: f32 = 0.5;
/// Items a row of the items' texture, two `RGBA32F` texels each.
pub const ITEMS_ROW: usize = 1024;
/// Half a tuft's blade's width across the screen, in art pixels: a blade is never narrower than one (A11.5).
pub const BLADE_HALF_PX: f32 = 0.55;
/// The share of the sky a blade sees at its foot, rising to all of it at its tip, and a stone at its middle,
/// rising to all of it at its top: the ground and the tuft's other blades hide the sky from an item's foot, so a
/// tuft shows a darker foot under lighter tips and a stone a darker side under a lighter top, in shade and sun
/// alike (tuned at α01d; α02a moves them to `data/tuning/render.md`, A11.13 rule 6).
pub const BLADE_FOOT_SKY: f32 = 0.45;
pub const STONE_FOOT_SKY: f32 = 0.6;
/// The contact shade's texels, metres; texels along its side at level 0; and its levels down to one texel.
pub const CONTACT_M: f32 = 0.5;
pub const CONTACT_SIDE: usize = 512;
pub const CONTACT_LEVELS: usize = 10;
/// The share of the sky an item hides from the ground at its foot when it covers a whole texel, and the most any
/// texel loses (tuned at α01d; α02a moves them to `data/tuning/render.md`, A11.13 rule 6).
pub const CONTACT_STRENGTH: f32 = 0.6;
pub const CONTACT_MAX: f32 = 0.7;
/// The seeds of the items, mixed with the area's number, and of a tuft's blades.
pub const SEED_COVER: u64 = 0x636f_7665_7273;
pub const SEED_BLADE: u32 = 16;
/// Art pixels finer than any split noise's octave: the split read at its full detail, as it shows while items do.
const FULL_DETAIL: f32 = 1e6;

/// What an item is; the shaders take the numbers as `COVER_STONE` and `COVER_TUFT`.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Kind {
    Stone = 0,
    Tuft = 1,
}

impl Kind {
    pub const ALL: [Kind; 2] = [Kind::Stone, Kind::Tuft];

    /// Triangle corners an instance draws: a stone's eight faces, or five blades of two triangles.
    pub const fn corners(self) -> i32 {
        match self {
            Kind::Stone => 24,
            Kind::Tuft => 30,
        }
    }
}

/// One stone or tuft.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Item {
    pub kind: Kind,
    /// Metres east and south of the area's corner, and the height there of the ground's 1 m mesh, which is what
    /// draws while items show, metres above the corner.
    pub at: [f32; 3],
    /// A stone's size across, a tuft's height, in metres.
    pub size: f32,
    /// Its heading, radians.
    pub yaw: f32,
    /// Its importance, 0 to 1: it shows while it spans `1.5 + 2u` art pixels (A11.5).
    pub u: f32,
    /// The ladder it is drawn in: the first palette index and the steps.
    pub ladder: [u16; 2],
    /// Sixteen bits of its shape: a stone's proportions and lean, a tuft's blades.
    pub shape: u16,
}

impl Item {
    /// Its two texels for the items' texture: place and size; heading, importance, ladder (`base + 256 steps`)
    /// and shape, each a whole number a float holds exactly.
    pub fn texels(&self) -> [[f32; 4]; 2] {
        [
            [self.at[0], self.at[1], self.at[2], self.size],
            [
                self.yaw,
                self.u,
                f32::from(self.ladder[0] + 256 * self.ladder[1]),
                f32::from(self.shape),
            ],
        ]
    }
}

/// What a surface's ground holds (A11.5): its stones and tufts a square metre and the ladders they are drawn in,
/// a tuft taking its look by the surface's split where it stands when it has one for each look.
#[derive(Clone, Debug, Default, PartialEq)]
pub struct SurfaceCover {
    pub stones_per_m2: f32,
    pub stone: [u16; 2],
    pub tufts_per_m2: f32,
    pub tufts: Vec<[u16; 2]>,
    /// The surface's split: its octaves as (λ₀, 1/λ₀, λ₁, 1/λ₁), and where each next look takes over.
    pub split_oct: [f32; 4],
    pub split_at: [f32; 2],
}

/// Each surface's cover, by surface number, from the catalogue (A11.13 rule 1).
#[derive(Clone, Debug, Default, PartialEq)]
pub struct CoverTable {
    pub surfaces: Vec<SurfaceCover>,
}

impl CoverTable {
    pub fn new(cat: &kd_data::Catalogue, layout: &Layout) -> Result<CoverTable, String> {
        let mut t = CoverTable {
            surfaces: vec![SurfaceCover::default(); MAX_SURFACES],
        };
        let ladder = |look: u16| -> Result<[u16; 2], String> {
            let i = cat
                .looks
                .iter()
                .position(|l| l.number == look)
                .ok_or_else(|| format!("look {look} is missing"))?;
            let l = layout.ladders[i];
            Ok([u16::from(l.base), u16::from(l.steps)])
        };
        for s in &cat.surfaces {
            let n = usize::from(s.number);
            if n >= MAX_SURFACES || s.split_m.len() > 2 {
                return Err(format!("surface {} does not fit the ground's uniforms", s.id));
            }
            let c = &mut t.surfaces[n];
            c.stones_per_m2 = s.stones_per_m2;
            if let Some(look) = s.stone_look {
                c.stone = ladder(look).map_err(|e| format!("surface {}: {e}", s.id))?;
            }
            c.tufts_per_m2 = s.tufts_per_m2;
            c.tufts = s
                .tuft_looks
                .iter()
                .map(|&l| ladder(l))
                .collect::<Result<_, _>>()
                .map_err(|e| format!("surface {}: {e}", s.id))?;
            for (k, &m) in s.split_m.iter().enumerate() {
                c.split_oct[2 * k] = m;
                c.split_oct[2 * k + 1] = 1.0 / m;
            }
            for (k, &v) in s.split_at.iter().enumerate() {
                c.split_at[k] = v;
            }
        }
        Ok(t)
    }
}

/// An area's stones and tufts and their contact shade (A11.5).
#[derive(Clone, Debug, PartialEq)]
pub struct Cover {
    /// Stones, then tufts, each bucket by bucket, row by row.
    pub items: Vec<Item>,
    /// Where each kind's bucket starts in `items`, `kind × BUCKETS² + row × BUCKETS + column`, and one more for the
    /// end.
    pub starts: Vec<u32>,
    /// The contact shade's levels, `CONTACT_SIDE >> level` to a side: the share of the sky hidden, in 255ths.
    pub contact: Vec<Vec<[u8; 1]>>,
}

/// The `k`th of a hash's draws, uniform in [0, 1): the top 24 bits of a hash of the two.
fn unit(h: u64, k: u64) -> f32 {
    (hash2(h, k) >> 40) as f32 * (1.0 / 16_777_216.0)
}

impl Cover {
    /// The area's items, seeded by its number: on each square metre, its surface's density of each kind, the
    /// fraction kept by a draw (A11.5); `heights` are the area's 1 m heights above its corner, `surfaces` its
    /// squares' surface numbers, and `corner` places the split's noise as the ground shader does.
    pub fn new(id: AreaId, corner: Pos, heights: &[f32], surfaces: &[u8], table: &CoverTable) -> Cover {
        let seed = hash2(SEED_COVER, u64::from(id.0));
        let w0 = camera::pattern_off(corner);
        let mut items = Vec::new();
        let mut starts = Vec::with_capacity(2 * BUCKETS * BUCKETS + 1);
        for kind in Kind::ALL {
            for b in 0..BUCKETS * BUCKETS {
                starts.push(items.len() as u32);
                let (bx, by) = (b % BUCKETS, b / BUCKETS);
                for y in by * BUCKET_M..(by + 1) * BUCKET_M {
                    for x in bx * BUCKET_M..(bx + 1) * BUCKET_M {
                        let square = y * AREA_SQUARES + x;
                        let Some(c) = table.surfaces.get(usize::from(surfaces[square])) else {
                            continue;
                        };
                        let density = match kind {
                            Kind::Stone => c.stones_per_m2,
                            Kind::Tuft if c.tufts.is_empty() => 0.0,
                            Kind::Tuft => c.tufts_per_m2,
                        };
                        if density <= 0.0 {
                            continue;
                        }
                        let key = hash2(seed, (square as u64) << 1 | kind as u64);
                        let n = density as u32 + u32::from(unit(key, 0) < density.fract());
                        for j in 0..n {
                            let h = hash2(key, u64::from(j) + 1);
                            let (ax, ay) = (x as f32 + unit(h, 1), y as f32 + unit(h, 2));
                            let r = unit(h, 3);
                            let (size, ladder) = match kind {
                                Kind::Stone => (STONE_M[0] * m::powf(STONE_M[1] / STONE_M[0], r * r), c.stone),
                                Kind::Tuft => {
                                    let look = if c.tufts.len() > 1 {
                                        let w = [w0[0] + ax, w0[1] + ay];
                                        let v = faded_noise(w, c.split_oct, FULL_DETAIL, SEED_SPLIT);
                                        split_look(v, c.split_at, c.tufts.len() as i32) as usize
                                    } else {
                                        0
                                    };
                                    (TUFT_M[0] + (TUFT_M[1] - TUFT_M[0]) * r, c.tufts[look])
                                }
                            };
                            items.push(Item {
                                kind,
                                at: [ax, ay, mesh_height(heights, ax, ay, 1, 0.0)],
                                size,
                                yaw: unit(h, 4) * std::f32::consts::TAU,
                                u: unit(h, 5),
                                ladder,
                                shape: (hash2(h, 6) >> 48) as u16,
                            });
                        }
                    }
                }
            }
        }
        starts.push(items.len() as u32);
        let contact = contact_levels(&items);
        Cover { items, starts, contact }
    }

    /// The items' texture, `ITEMS_ROW` items a row, the last row filled out with nothing.
    pub fn texture_bytes(&self) -> (u32, Vec<u8>) {
        let rows = self.items.len().div_ceil(ITEMS_ROW).max(1);
        let mut bytes = Vec::with_capacity(rows * ITEMS_ROW * 32);
        for item in &self.items {
            bytes.extend(item.texels().iter().flatten().flat_map(|v| v.to_ne_bytes()));
        }
        bytes.resize(rows * ITEMS_ROW * 32, 0);
        (rows as u32, bytes)
    }

    /// The items of `kind` in a row of buckets, from column `x` for `n` columns, as a run of `items`.
    pub fn run(&self, kind: Kind, row: usize, x: usize, n: usize) -> std::ops::Range<usize> {
        let at = |b: usize| self.starts[kind as usize * BUCKETS * BUCKETS + row * BUCKETS + b] as usize;
        at(x)..at(x + n)
    }
}

/// The contact shade (A11.5): each item hides `CONTACT_STRENGTH` of the sky, times the share of a texel its foot
/// covers, spread over the four texels round its foot so the shade centres on it, each texel's sum held to
/// `CONTACT_MAX`; each level above averages four texels of the one below before either is rounded to 255ths, so
/// every level keeps the shade's average.
pub fn contact_levels(items: &[Item]) -> Vec<Vec<[u8; 1]>> {
    let n = CONTACT_SIDE;
    let mut level0 = vec![0.0f32; n * n];
    for item in items {
        let radius = match item.kind {
            Kind::Stone => 0.5 * item.size,
            Kind::Tuft => 0.35 * item.size,
        };
        let share = (std::f32::consts::PI * radius * radius / (CONTACT_M * CONTACT_M)).min(1.0);
        let p = [item.at[0] / CONTACT_M - 0.5, item.at[1] / CONTACT_M - 0.5];
        let i = p.map(f32::floor);
        let f = [p[0] - i[0], p[1] - i[1]];
        for (dx, dy, w) in [
            (0, 0, (1.0 - f[0]) * (1.0 - f[1])),
            (1, 0, f[0] * (1.0 - f[1])),
            (0, 1, (1.0 - f[0]) * f[1]),
            (1, 1, f[0] * f[1]),
        ] {
            let x = (i[0] as i32 + dx).clamp(0, n as i32 - 1) as usize;
            let y = (i[1] as i32 + dy).clamp(0, n as i32 - 1) as usize;
            level0[y * n + x] += CONTACT_STRENGTH * share * w;
        }
    }
    let mut field: Vec<f32> = level0.iter().map(|v| v.min(CONTACT_MAX)).collect();
    let mut levels = Vec::with_capacity(CONTACT_LEVELS);
    let mut side = n;
    loop {
        levels.push(field.iter().map(|v| [(v * 255.0).round() as u8]).collect());
        if side == 1 {
            return levels;
        }
        let half = side / 2;
        field = (0..half * half)
            .map(|k| {
                let (x, y) = (2 * (k % half), 2 * (k / half));
                let at = |a: usize, b: usize| field[b * side + a];
                0.25 * (at(x, y) + at(x + 1, y) + at(x, y + 1) + at(x + 1, y + 1))
            })
            .collect();
        side = half;
    }
}

/// The buckets of an area a view may show items in, as `patches_in_view` gives the ground's patches: from its
/// footprint between the area's lowest point and `ITEM_RISE_M` over its highest, widened by the items' reach.
pub fn buckets_in_view(view: &View, area: &Area) -> Option<[i32; 4]> {
    let span = [area.span_m[0], area.span_m[1] + ITEM_RISE_M];
    let bounds = view.local_footprint(area.corner(), span)?;
    let (n, size) = (BUCKETS as i32, BUCKET_M as f64);
    let lo = |v: f64| (((v - ITEM_REACH_M) / size).floor() as i32).clamp(0, n);
    let hi = |v: f64| (((v + ITEM_REACH_M) / size).ceil() as i32).clamp(0, n);
    let (x0, x1, y0, y1) = (lo(bounds[0]), hi(bounds[1]), lo(bounds[2]), hi(bounds[3]));
    (x1 > x0 && y1 > y0).then_some([x0, y0, x1 - x0, y1 - y0])
}

/// The items' uniforms, in `CoverPass::u`'s order.
const UNIFORMS: [&str; 19] = [
    "u_first",
    "u_kind",
    "u_right",
    "u_up",
    "u_fwd",
    "u_inv_texel",
    "u_area_frac",
    "u_area_px",
    "u_depth",
    "u_area_air",
    "u_dither",
    "u_light_dir",
    "u_light_tan",
    "u_y",
    "u_range",
    "u_haze_beta",
    "u_haze_scale",
    "u_eye",
    "u_haze_levels",
];

/// The stones' and tufts' program and its uniforms (A11.13 rule 3).
pub struct CoverPass {
    program: Program,
    u: [Option<glow::UniformLocation>; UNIFORMS.len()],
}

impl CoverPass {
    pub fn new(gl: &glow::Context) -> Result<CoverPass, RenderError> {
        let program = Program::new(
            gl,
            "cover",
            &shaders::source(Stage::Vertex, shaders::COVER_VERT),
            &shaders::source(Stage::Fragment, shaders::COVER_FRAG),
        )?;
        program.set_sampler(gl, "u_items", unit::ITEMS);
        program.set_sampler(gl, "u_sun", unit::SUN);
        program.set_sampler(gl, "u_sky", unit::SKY);
        Ok(CoverPass {
            u: UNIFORMS.map(|name| program.uniform(gl, name)),
            program,
        })
    }

    /// Draws the stones and tufts of the buckets in view, over the ground already in the bound art target, with
    /// its depth; none while the art pixel is over `COVER_MAX_TEXEL`. Returns how many items it handed the GPU.
    pub fn draw(
        &self,
        gl: &glow::Context,
        view: &View,
        store: &Store,
        lighting: &Lighting,
        haze_levels: [f32; 3],
        vao: glow::VertexArray,
    ) -> u64 {
        if view.texel as f32 > COVER_MAX_TEXEL {
            return 0;
        }
        gl::apply(
            gl,
            &State {
                viewport: view.viewport(),
                depth_test: true,
                depth_write: true,
                blend: false,
                cull_back: false,
            },
        );
        self.program.bind(gl);
        let [
            u_first,
            u_kind,
            u_right,
            u_up,
            u_fwd,
            u_inv_texel,
            u_area_frac,
            u_area_px,
            u_depth,
            u_area_air,
            u_dither,
            u_light_dir,
            u_light_tan,
            u_y,
            u_range,
            u_haze_beta,
            u_haze_scale,
            u_eye,
            u_haze_levels,
        ] = &self.u;
        let b = view.basis_f32();
        gl::set_vec2(gl, u_right.as_ref(), [b.right[0], b.right[1]]);
        gl::set_vec3(gl, u_up.as_ref(), b.up);
        gl::set_vec3(gl, u_fwd.as_ref(), b.fwd);
        gl::set_f32(gl, u_inv_texel.as_ref(), (1.0 / view.texel) as f32);
        gl::set_ivec2(gl, u_dither.as_ref(), view.dither());
        let light = &lighting.light;
        gl::set_vec3(gl, u_light_dir.as_ref(), light.dir);
        gl::set_f32(gl, u_light_tan.as_ref(), super::light_tan(light.dir));
        gl::set_vec2(gl, u_y.as_ref(), [dot(LUM, light.sky), dot(LUM, light.sun)]);
        gl::set_vec2(gl, u_range.as_ref(), lighting.palette.range);
        let (beta, scale) = lighting.haze_air;
        gl::set_vec2(gl, u_haze_beta.as_ref(), beta);
        gl::set_vec2(gl, u_haze_scale.as_ref(), scale);
        gl::set_vec2(gl, u_eye.as_ref(), view.eye());
        gl::set_vec3(gl, u_haze_levels.as_ref(), haze_levels);
        let mut drawn = 0;
        for area in &store.areas {
            let Some(t) = &area.gpu else {
                continue;
            };
            let Some(items) = &t.items else {
                continue;
            };
            let Some([x, y, w, h]) = buckets_in_view(view, area) else {
                continue;
            };
            let p = view.area(area.corner());
            items.bind(gl, unit::ITEMS);
            t.sun.bind(gl, unit::SUN);
            t.sky.bind(gl, unit::SKY);
            gl::set_vec2(gl, u_area_frac.as_ref(), p.frac);
            gl::set_vec2(gl, u_area_px.as_ref(), p.px);
            gl::set_vec2(gl, u_depth.as_ref(), p.depth);
            gl::set_vec2(gl, u_area_air.as_ref(), p.air);
            for kind in Kind::ALL {
                gl::set_i32(gl, u_kind.as_ref(), kind as i32);
                for row in y..y + h {
                    let run = area.cover.run(kind, row as usize, x as usize, w as usize);
                    gl::set_i32(gl, u_first.as_ref(), run.start as i32);
                    gl::draw_instanced(gl, vao, kind.corners(), run.len() as i32);
                    drawn += run.len() as u64;
                }
            }
        }
        drawn
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::pixel::cover_shows;

    /// Bands of the catalogue's four surfaces across a rough area, each 64 m wide, and the cover table.
    fn sample() -> (Vec<f32>, Vec<u8>, CoverTable, kd_data::Catalogue) {
        let cat = crate::tests::catalogue();
        let layout = Layout::new(&cat).unwrap();
        let table = CoverTable::new(&cat, &layout).unwrap();
        let number = |id: &str| cat.surface(id).unwrap().number as u8;
        let bands = [number("grass"), number("dirt"), number("rock"), number("scree")];
        let surfaces = (0..AREA_SQUARES * AREA_SQUARES)
            .map(|i| bands[(i % AREA_SQUARES) / 64])
            .collect();
        let side = AREA_SQUARES + 1;
        let heights = (0..side * side)
            .map(|k| {
                let (x, y) = ((k % side) as f32, (k / side) as f32);
                0.2 * x + 2.0 * (x * 0.13).sin() * (y * 0.07).cos()
            })
            .collect();
        (heights, surfaces, table, cat)
    }

    const CORNER: Pos = Pos {
        x: 3 * 65_536,
        y: 5 * 65_536,
        z: 0,
    };

    // checks: PRE-46 WLD-12 PRE-22
    #[test]
    fn seeded_and_stable() {
        let (heights, surfaces, table, cat) = sample();
        let cover = Cover::new(AreaId(77), CORNER, &heights, &surfaces, &table);
        // The same area gives the same items, wherever and whenever it is made; another area other ones.
        assert_eq!(cover, Cover::new(AreaId(77), CORNER, &heights, &surfaces, &table));
        let other = Cover::new(AreaId(78), CORNER, &heights, &surfaces, &table);
        assert_ne!(cover.items, other.items);
        // Stones, then tufts, bucket by bucket: every item lies in its bucket, on the ground's 1 m mesh.
        assert_eq!(cover.starts.len(), 2 * BUCKETS * BUCKETS + 1);
        assert!(cover.starts.windows(2).all(|w| w[0] <= w[1]));
        for kind in Kind::ALL {
            for row in 0..BUCKETS {
                for col in 0..BUCKETS {
                    for item in &cover.items[cover.run(kind, row, col, 1)] {
                        assert_eq!(item.kind, kind);
                        assert_eq!(
                            (item.at[0] as usize / BUCKET_M, item.at[1] as usize / BUCKET_M),
                            (col, row)
                        );
                        assert_eq!(item.at[2], mesh_height(&heights, item.at[0], item.at[1], 1, 0.0));
                    }
                }
            }
        }
        // Each surface's ground holds its catalogue's density of each kind, in its looks: scree's stones, grass's
        // tufts in its two looks by the split under them, a few stones on grass and dirt, none on rock.
        let squares = (AREA_SQUARES * 64) as f32;
        for (band, s) in ["grass", "dirt", "rock", "scree"].iter().enumerate() {
            let s = cat.surface(s).unwrap();
            let c = &table.surfaces[usize::from(s.number)];
            let on = |kind: Kind| {
                cover
                    .items
                    .iter()
                    .filter(|i| i.kind == kind && i.at[0] as usize / 64 == band)
                    .collect::<Vec<_>>()
            };
            for (kind, density) in [(Kind::Stone, s.stones_per_m2), (Kind::Tuft, s.tufts_per_m2)] {
                let n = on(kind).len() as f32;
                let want = density * squares;
                assert!(
                    (n - want).abs() <= 4.0 * want.sqrt() + 1.0,
                    "{} {kind:?}: {n} for {want}",
                    s.id
                );
            }
            assert!(on(Kind::Stone).iter().all(|i| i.ladder == c.stone));
            assert!(on(Kind::Tuft).iter().all(|i| c.tufts.contains(&i.ladder)));
            if c.tufts.len() > 1 {
                for look in &c.tufts {
                    assert!(
                        on(Kind::Tuft).iter().any(|i| i.ladder == *look),
                        "{}: a look unused",
                        s.id
                    );
                }
                // A tuft takes the look of the ground's split where it stands, read as the ground shader reads it.
                let w0 = camera::pattern_off(CORNER);
                for i in on(Kind::Tuft) {
                    let v = faded_noise([w0[0] + i.at[0], w0[1] + i.at[1]], c.split_oct, 1.0 / 0.03, SEED_SPLIT);
                    assert_eq!(i.ladder, c.tufts[split_look(v, c.split_at, 2) as usize]);
                }
            }
        }
        // Sizes in their ranges, the stones mostly small.
        let sizes = |kind: Kind| cover.items.iter().filter(move |i| i.kind == kind).map(|i| i.size);
        assert!(sizes(Kind::Stone).all(|s| (STONE_M[0]..=STONE_M[1]).contains(&s)));
        assert!(sizes(Kind::Tuft).all(|s| (TUFT_M[0]..=TUFT_M[1]).contains(&s)));
        let small = sizes(Kind::Stone).filter(|&s| s < 0.15).count() as f32;
        assert!(small > 0.6 * sizes(Kind::Stone).count() as f32);
        // The texture holds each item's two texels, a row of ITEMS_ROW items at a time.
        let (rows, bytes) = cover.texture_bytes();
        assert_eq!(rows as usize, cover.items.len().div_ceil(ITEMS_ROW));
        assert_eq!(bytes.len(), rows as usize * ITEMS_ROW * 32);
        let i = cover.items.len() - 1;
        let f = |k: usize| f32::from_ne_bytes(bytes[32 * i + 4 * k..32 * i + 4 * k + 4].try_into().unwrap());
        assert_eq!(
            [f(0), f(3), f(5)],
            [cover.items[i].at[0], cover.items[i].size, cover.items[i].u]
        );
        assert_eq!(f(6) as u16, cover.items[i].ladder[0] + 256 * cover.items[i].ladder[1]);
    }

    // checks: PRE-46 PRE-22 PRE-20
    #[test]
    fn contact_shade_under_the_items() {
        // A stone half a metre across on bare ground: its shade, spread over the four texels round its foot, sums to
        // the share of the sky it hides, is darkest at its foot and gone a metre away.
        let stone = |x: f32, y: f32, size: f32| Item {
            kind: Kind::Stone,
            at: [x, y, 0.0],
            size,
            yaw: 0.0,
            u: 0.0,
            ladder: [1, 4],
            shape: 0,
        };
        let levels = contact_levels(&[stone(100.3, 40.7, 0.5)]);
        assert_eq!(levels.len(), CONTACT_LEVELS);
        assert_eq!(levels[CONTACT_LEVELS - 1].len(), 1);
        let hidden = CONTACT_STRENGTH * std::f32::consts::PI * 0.25 * 0.25 / (CONTACT_M * CONTACT_M);
        let sum: f32 = levels[0].iter().map(|v| f32::from(v[0])).sum();
        assert!((sum - hidden * 255.0).abs() <= 2.0, "{sum}");
        let at = |q: [f32; 2]| {
            crate::pixel::cover_sample(&levels, CONTACT_SIDE, [q[0] / CONTACT_M, q[1] / CONTACT_M], 0.0)[0]
        };
        let foot = at([100.3, 40.7]);
        assert!(
            foot > 0.1 && at([100.6, 40.7]) < foot && at([100.0, 40.4]) < foot,
            "{foot}"
        );
        assert_eq!(at([101.4, 40.7]), 0.0);
        // Many items on one texel are held at the most a texel loses; a tuft's foot shades less than a stone's.
        let heap = contact_levels(&[stone(10.25, 10.25, 0.6); 5]);
        let middle = 20 * CONTACT_SIDE + 20;
        assert_eq!(heap[0][middle][0], (CONTACT_MAX * 255.0).round() as u8);
        let tuft = Item {
            kind: Kind::Tuft,
            ..stone(10.25, 10.25, 0.3)
        };
        assert!(contact_levels(&[tuft])[0][middle][0] < contact_levels(&[stone(10.25, 10.25, 0.3)])[0][middle][0]);
        // Over the sample's scree, each level keeps the shade's average within rounding, so as the art pixel grows
        // the shade fades to its average rather than vanishing (A11.1 rule 2).
        let (heights, surfaces, table, _) = sample();
        let cover = Cover::new(AreaId(9), CORNER, &heights, &surfaces, &table);
        let mean = |k: usize| {
            let n = CONTACT_SIDE >> k;
            let band = &cover.contact[k];
            let (x0, x1) = (3 * n / 4, n);
            let total: f32 = (0..n)
                .flat_map(|y| (x0..x1).map(move |x| (x, y)))
                .map(|(x, y)| f32::from(band[y * n + x][0]))
                .sum();
            total / ((x1 - x0) * n) as f32
        };
        let base = mean(0);
        assert!(base > 2.0, "{base}");
        for k in 1..CONTACT_LEVELS - 2 {
            assert!(
                (mean(k) - base).abs() < 0.03 * base + 0.5,
                "level {k}: {} against {base}",
                mean(k)
            );
        }
    }

    // checks: PRE-46 PRE-22
    #[test]
    fn thin_out_by_importance() {
        // As the art pixel grows by 1% steps from the person stop, items only ever drop out, never come back, a few
        // at a time, the least important of a size first; all but a few tufts show at the person stop, and none from
        // the largest stone's 1.5 art pixels on.
        let (heights, surfaces, table, _) = sample();
        let cover = Cover::new(AreaId(5), CORNER, &heights, &surfaces, &table);
        let shown = |texel: f32| -> Vec<bool> {
            cover
                .items
                .iter()
                .map(|i| cover_shows(i.size, i.u, 1.0 / texel))
                .collect()
        };
        let tufts: Vec<bool> = cover.items.iter().map(|i| i.kind == Kind::Tuft).collect();
        let at_person = shown(0.03);
        let tufts_shown = at_person.iter().zip(&tufts).filter(|(s, t)| **s && **t).count();
        assert!(tufts_shown as f32 > 0.95 * tufts.iter().filter(|t| **t).count() as f32);
        let mut last = at_person;
        let mut texel = 0.03f32;
        let mut most = 0;
        while texel < COVER_MAX_TEXEL * 1.02 {
            texel *= 1.01;
            let now = shown(texel);
            assert!(
                now.iter().zip(&last).all(|(n, l)| !n || *l),
                "an item came back at {texel} m"
            );
            let dropped = now.iter().zip(&last).filter(|(n, l)| !**n && **l).count();
            most = most.max(dropped);
            last = now;
        }
        assert!(last.iter().all(|s| !s), "items still shown at {texel} m");
        assert!(
            most > 0 && (most as f32) < 0.01 * cover.items.len() as f32,
            "{most} dropped at once"
        );
        // Of two items the same size, the less important drops first.
        assert!(cover_shows(0.3, 0.2, 1.0 / 0.1) && !cover_shows(0.3, 0.8, 1.0 / 0.1));
        assert!(!cover_shows(STONE_M[1], 0.0, 1.0 / (COVER_MAX_TEXEL * 1.001)));
        assert!(cover_shows(STONE_M[1], 0.0, 1.0 / COVER_MAX_TEXEL));
    }
}
