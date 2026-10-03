//! The ground (A11.5): the CPU store of the loaded areas, their textures, the morphing patch and the ground's draw.
//! Each area keeps its heights, their gradients, its surfaces' coverage and its light fields on the CPU, so after a lost
//! context its textures are made again from them alone (A11.13 rule 5): the sky field once, as it arrives, and the
//! sun field for the light's azimuth, made again as the azimuth moves (`field`). One shared 16 × 16 patch of quads, made from vertex and instance
//! numbers with no vertex buffer, covers the area at the spacing the art pixel asks for, its odd vertices sliding
//! onto the next spacing's mesh as the spacing's range ends, so a change of spacing moves nothing (A11.1 rule 3).
//!
//! Implements PRE-02 and PRE-20, see A11.5: a real 3D ground drawn at low resolution, its surfaces in their looks'
//! steps.

pub mod cover;

use kd_core::geo::{AreaId, Pos};
use kd_view::{AREA_SIDE, AREA_SQUARES, AreaMeshes};

use crate::RenderError;
use crate::camera::View;
use crate::field::{self, SunField, SunJob};
use crate::frame::Lighting;
use crate::gl::{self, Format, Program, State, Texture, unit};
use crate::light::{LUM, dot};
use crate::looks::Layout;
use crate::pixel::{COVER_CHANNELS, cover_level, relief_octaves};
use crate::shaders::{self, Stage};
use cover::{CONTACT_LEVELS, CONTACT_M, CONTACT_SIDE, Cover, CoverPass, CoverTable, ITEMS_ROW};

/// Quads along a patch's side.
pub const PATCH_QUADS: i32 = 16;
/// How far the skirts hang below the loaded ground's outer edge (A11.5).
pub const SKIRT_M: f32 = 4.0;
/// Surfaces the ground's uniforms hold, by number.
pub const MAX_SURFACES: usize = 16;
/// Looks a surface may name (A11.5).
pub const SURFACE_LOOKS: usize = 3;
/// Levels of an area's coverage, from its 256 × 256 square metres to one texel (A11.5).
pub const COVER_LEVELS: usize = 9;

/// A patch of ground's surfaces as coverage (A11.5): each surface's share of each texel in 255ths at every mip level,
/// from the square metres, each wholly one surface, to one texel, the shares of a texel summing to 255 exactly; up
/// to `COVER_CHANNELS` surfaces, four to a texture.
#[derive(Clone, Debug, PartialEq)]
pub struct Coverage {
    /// Texels along a side at level 0, a power of two.
    pub side: usize,
    /// The surface each channel holds, those present in rising number, −1 for none.
    pub ids: [i32; COVER_CHANNELS],
    /// Each level's texels row by row, `side >> level` to a side, each its channels' shares.
    pub levels: Vec<Vec<[u8; COVER_CHANNELS]>>,
}

impl Coverage {
    /// The coverage of `side` × `side` squares' surface numbers, row by row; an error with more than
    /// `COVER_CHANNELS` surfaces.
    pub fn new(surfaces: &[u8], side: usize) -> Result<Coverage, String> {
        if !side.is_power_of_two() || surfaces.len() != side * side {
            return Err(format!("{} squares for a coverage {side} to a side", surfaces.len()));
        }
        let mut present = surfaces.to_vec();
        present.sort_unstable();
        present.dedup();
        if present.len() > COVER_CHANNELS {
            return Err(format!(
                "{} surfaces in one area, at most {COVER_CHANNELS}",
                present.len()
            ));
        }
        let mut ids = [-1; COVER_CHANNELS];
        let mut channel = [0usize; 256];
        for (k, &n) in present.iter().enumerate() {
            ids[k] = i32::from(n);
            channel[usize::from(n)] = k;
        }
        let mut levels = vec![
            surfaces
                .iter()
                .map(|&n| {
                    let mut t = [0u8; COVER_CHANNELS];
                    t[channel[usize::from(n)]] = 255;
                    t
                })
                .collect::<Vec<_>>(),
        ];
        let mut n = side;
        while n > 1 {
            let prev = &levels[levels.len() - 1];
            let half = n / 2;
            let next = (0..half * half)
                .map(|i| {
                    let (x, y) = (2 * (i % half), 2 * (i / half));
                    average([
                        prev[y * n + x],
                        prev[y * n + x + 1],
                        prev[(y + 1) * n + x],
                        prev[(y + 1) * n + x + 1],
                    ])
                })
                .collect();
            levels.push(next);
            n = half;
        }
        Ok(Coverage { side, ids, levels })
    }

    /// How many textures of four channels the surfaces present take.
    pub fn textures(&self) -> usize {
        if self.ids[4] < 0 { 1 } else { 2 }
    }

    /// Texture `t`'s levels as RGBA8 texels, for the GL layer.
    pub fn texture_levels(&self, t: usize) -> Vec<Vec<u8>> {
        self.levels
            .iter()
            .map(|level| level.iter().flat_map(|c| c[4 * t..4 * t + 4].iter().copied()).collect())
            .collect()
    }
}

/// Four texels' shares averaged, each channel's sum divided by four and rounded down, then a 255th more for the
/// channels with the largest remainders, the earlier first, so the shares still sum to 255.
fn average(kids: [[u8; COVER_CHANNELS]; 4]) -> [u8; COVER_CHANNELS] {
    let mut sum = [0u32; COVER_CHANNELS];
    for kid in kids {
        for (s, &v) in sum.iter_mut().zip(&kid) {
            *s += u32::from(v);
        }
    }
    let mut out = sum.map(|s| (s / 4) as u8);
    let mut left = 255 - out.iter().map(|&v| u32::from(v)).sum::<u32>();
    for r in [3, 2, 1] {
        for (o, &s) in out.iter_mut().zip(&sum) {
            if left > 0 && s % 4 == r {
                *o += 1;
                left -= 1;
            }
        }
    }
    out
}

/// The mesh's spacing for art pixels of `texel` metres, and how far its odd vertices have slid onto the next
/// spacing's mesh: the smallest power of two `s` with `s ≥ 1.25 × texel` and `s ≥ 1`, so triangles stay one to two
/// art pixels across, sliding over the upper half of the spacing's range and arriving as the next spacing takes
/// over (A11.5).
pub fn spacing(texel: f32) -> (i32, f32) {
    let want = 1.25 * texel;
    let mut s = 1;
    while (s as f32) < want && s < 1 << 20 {
        s *= 2;
    }
    let morph = ((want / s as f32 - 0.75) / 0.25).clamp(0.0, 1.0);
    (s, morph)
}

/// The height of grid point `g` (metres east and south of the area's corner) in the mesh of spacing `s`, its odd
/// vertices `morph` of the way onto the next spacing's mesh: onto the middle of their even neighbours along a row,
/// a column, or the quads' diagonal from the north-west, which the next spacing's quads share (A11.5). The ground
/// shader's `vertex_height` does the same.
pub fn vertex_height(heights: &[f32], g: [i32; 2], s: i32, morph: f32) -> f32 {
    let at = |x: i32, y: i32| heights[(y.clamp(0, 256) * AREA_SIDE as i32 + x.clamp(0, 256)) as usize];
    let h = at(g[0], g[1]);
    let odd = [(g[0] / s) & 1, (g[1] / s) & 1];
    let coarse = match odd {
        [0, 0] => return h,
        [1, 1] => 0.5 * (at(g[0] - s, g[1] - s) + at(g[0] + s, g[1] + s)),
        [1, _] => 0.5 * (at(g[0] - s, g[1]) + at(g[0] + s, g[1])),
        _ => 0.5 * (at(g[0], g[1] - s) + at(g[0], g[1] + s)),
    };
    h + (coarse - h) * morph
}

/// The mesh's height at any point (`x`, `y`) of the area: the triangle of the spacing-`s` quad holding it,
/// split along its diagonal from the north-west, between its morphed corners (for the tests).
pub fn mesh_height(heights: &[f32], x: f32, y: f32, s: i32, morph: f32) -> f32 {
    let sf = s as f32;
    let qx = ((x / sf).floor() as i32).clamp(0, 256 / s - 1);
    let qy = ((y / sf).floor() as i32).clamp(0, 256 / s - 1);
    let (u, v) = (x / sf - qx as f32, y / sf - qy as f32);
    let corner = |dx: i32, dy: i32| vertex_height(heights, [(qx + dx) * s, (qy + dy) * s], s, morph);
    let (h00, h11) = (corner(0, 0), corner(1, 1));
    if u >= v {
        let h10 = corner(1, 0);
        h00 + (h10 - h00) * u + (h11 - h10) * v
    } else {
        let h01 = corner(0, 1);
        h00 + (h01 - h00) * v + (h11 - h01) * u
    }
}

/// Each point's gradient, east and south, by central differences of the heights (one-sided at the area's edge,
/// until neighbours' heights are read across it): the ground shader blends the four round a pixel, which is the
/// central difference of the blended heights (A11.5).
pub fn gradients(heights: &[f32]) -> Vec<[f32; 2]> {
    let n = AREA_SIDE as i32;
    let at = |x: i32, y: i32| heights[(y * n + x) as usize];
    let diff = |lo: f32, hi: f32, span: i32| (hi - lo) / span as f32;
    let mut out = Vec::with_capacity(heights.len());
    for y in 0..n {
        for x in 0..n {
            let (x0, x1) = ((x - 1).max(0), (x + 1).min(n - 1));
            let (y0, y1) = ((y - 1).max(0), (y + 1).min(n - 1));
            out.push([diff(at(x0, y), at(x1, y), x1 - x0), diff(at(x, y0), at(x, y1), y1 - y0)]);
        }
    }
    out
}

/// A loaded area on the CPU (A11.13 rule 5), and its textures while the context lives.
pub struct Area {
    pub id: AreaId,
    pub base_m: f32,
    pub heights: Vec<f32>,
    pub gradients: Vec<[f32; 2]>,
    /// Each square metre's surface number, and the surfaces as coverage (A11.5).
    pub surfaces: Vec<u8>,
    pub coverage: Coverage,
    /// Its stones and tufts and their contact shade (A11.5).
    pub cover: Cover,
    /// Its lowest and highest points, metres above `base_m`.
    pub span_m: [f32; 2],
    /// The share of the sky each point's horizon leaves open, in 255ths (A11.5).
    pub sky: Vec<u8>,
    /// The sun field for the light's azimuth, none until the first frame's light (A11.5).
    pub sun: Option<SunField>,
    /// The next sun field, worked out over frames while the light moves a little (A11.11).
    sun_job: Option<SunJob>,
    /// Whether the sun field is newer than its texture.
    sun_fresh: bool,
    gpu: Option<AreaTextures>,
}

/// An area's textures while the context lives (A11.13 rule 5).
struct AreaTextures {
    heights: Texture,
    grads: Texture,
    /// One or two RGBA8 textures with their mip levels.
    cover: Vec<Texture>,
    sun: Texture,
    sky: Texture,
    /// The contact shade, `R8` with its mip levels, and the items, `RGBA32F`, none for an area with none.
    contact: Texture,
    items: Option<Texture>,
}

impl Area {
    /// The area's north-west corner at its base height.
    pub fn corner(&self) -> Pos {
        let o = self.id.origin();
        Pos {
            z: (self.base_m * 256.0).round() as i32,
            ..o
        }
    }
}

/// Little-endian bytes of floats, as the GL layer uploads them.
fn f32_bytes(values: impl Iterator<Item = f32>) -> Vec<u8> {
    values.flat_map(f32::to_ne_bytes).collect()
}

/// The loaded areas (A11.5); in α01b, the demo area alone.
#[derive(Default)]
pub struct Store {
    pub areas: Vec<Area>,
    /// What each surface's ground holds, from the catalogue; with none, areas have no stones or tufts.
    pub cover: CoverTable,
}

impl Store {
    /// Takes an area's ground, replacing an earlier one of the same id; its textures are made on the next draw.
    pub fn insert(&mut self, m: AreaMeshes) -> Result<(), RenderError> {
        if m.heights.len() != AREA_SIDE * AREA_SIDE || m.surfaces.len() != AREA_SQUARES * AREA_SQUARES {
            return Err(RenderError::Gl(format!(
                "area {}: {} heights and {} surfaces",
                m.id.0,
                m.heights.len(),
                m.surfaces.len()
            )));
        }
        if let Some(&n) = m.surfaces.iter().find(|&&n| usize::from(n) >= MAX_SURFACES) {
            return Err(RenderError::Gl(format!(
                "area {}: surface {n} beyond {MAX_SURFACES}",
                m.id.0
            )));
        }
        // The base on the grid of ticks positions count in, the heights moved by what that changes (under 2 mm).
        let base_m = (m.base_m * 256.0).round() / 256.0;
        let heights: Vec<f32> = m.heights.iter().map(|h| h + (m.base_m - base_m)).collect();
        let lo = heights.iter().copied().fold(f32::INFINITY, f32::min);
        let hi = heights.iter().copied().fold(f32::NEG_INFINITY, f32::max);
        let coverage =
            Coverage::new(&m.surfaces, AREA_SQUARES).map_err(|e| RenderError::Gl(format!("area {}: {e}", m.id.0)))?;
        let corner = Pos {
            z: (base_m * 256.0).round() as i32,
            ..m.id.origin()
        };
        let cover = Cover::new(m.id, corner, &heights, &m.surfaces, &self.cover);
        let sky = field::sky_field(&heights)
            .iter()
            .map(|v| (v.clamp(0.0, 1.0) * 255.0).round() as u8)
            .collect();
        let area = Area {
            id: m.id,
            base_m,
            gradients: gradients(&heights),
            sky,
            heights,
            surfaces: m.surfaces,
            coverage,
            cover,
            span_m: [lo, hi],
            sun: None,
            sun_job: None,
            sun_fresh: false,
            gpu: None,
        };
        self.areas.retain(|a| a.id != area.id);
        self.areas.push(area);
        Ok(())
    }

    /// How many areas have their textures.
    pub fn uploaded(&self) -> usize {
        self.areas.iter().filter(|a| a.gpu.is_some()).count()
    }

    /// Keeps each area's sun field on the light's azimuth `dir` (east, north, up; A11.5, A11.11): made at once for
    /// an area that has none or a jump of `SUN_JUMP_DEG` or more, as the strip's hours make; for a smaller move of
    /// `SUN_FIELD_STEP_DEG` or more, worked out over frames while `more()` allows, the old field drawn until the new
    /// one is whole. Its texture follows at the next upload. Returns how many fields it finished.
    pub fn follow_light(&mut self, dir: [f32; 3], more: &mut dyn FnMut() -> bool) -> usize {
        let mut made = 0;
        for a in &mut self.areas {
            let field = match &a.sun {
                Some(f) if !f.jumped(dir) => {
                    if a.sun_job.is_none() && f.stale(dir) {
                        a.sun_job = SunJob::new(dir);
                    }
                    let Some(job) = &mut a.sun_job else {
                        continue;
                    };
                    let Some(field) = job.step(&a.heights, more) else {
                        continue;
                    };
                    a.sun_job = None;
                    field
                }
                _ => {
                    a.sun_job = None;
                    field::sun_field(&a.heights, dir)
                }
            };
            a.sun = Some(field);
            a.sun_fresh = true;
            made += 1;
        }
        made
    }

    /// Makes the textures of areas that lack them (heights `R32F`, gradients `RG32F`, coverage `RGBA8` with its mip
    /// levels, the sun field `R32F`, the sky field `R8`, the contact shade `R8` with its levels and the stones and
    /// tufts `RGBA32F`), and brings a sun field's texture up to date when it was made again (A11.5).
    pub fn upload(&mut self, gl: &glow::Context) -> Result<(), RenderError> {
        let (side, squares) = (AREA_SIDE as u32, AREA_SQUARES as u32);
        for a in &mut self.areas {
            // Before the first frame's light, level horizons: the field arrives with it.
            let sun = || match &a.sun {
                Some(f) => f32_bytes(f.horizon.iter().copied()),
                None => f32_bytes(std::iter::repeat_n(0.0, AREA_SIDE * AREA_SIDE)),
            };
            match &a.gpu {
                Some(t) => {
                    if a.sun_fresh {
                        t.sun.update(gl, &sun())?;
                    }
                }
                None => {
                    let heights = Texture::new(
                        gl,
                        Format::R32F,
                        side,
                        side,
                        Some(&f32_bytes(a.heights.iter().copied())),
                    )?;
                    let grads = Texture::new(
                        gl,
                        Format::Rg32F,
                        side,
                        side,
                        Some(&f32_bytes(a.gradients.iter().flatten().copied())),
                    )?;
                    let cover = (0..a.coverage.textures())
                        .map(|t| {
                            Texture::with_levels(gl, Format::Rgba8, squares, squares, &a.coverage.texture_levels(t))
                        })
                        .collect::<Result<Vec<_>, _>>()?;
                    let sun_tex = Texture::new(gl, Format::R32F, side, side, Some(&sun()))?;
                    let sky = Texture::new(gl, Format::R8, side, side, Some(&a.sky))?;
                    let contact_levels: Vec<Vec<u8>> = a
                        .cover
                        .contact
                        .iter()
                        .map(|l| l.iter().map(|v| v[0]).collect())
                        .collect();
                    let contact = Texture::with_levels(
                        gl,
                        Format::R8,
                        CONTACT_SIDE as u32,
                        CONTACT_SIDE as u32,
                        &contact_levels,
                    )?;
                    let items = if a.cover.items.is_empty() {
                        None
                    } else {
                        let (rows, bytes) = a.cover.texture_bytes();
                        Some(Texture::new(
                            gl,
                            Format::Rgba32F,
                            2 * ITEMS_ROW as u32,
                            rows,
                            Some(&bytes),
                        )?)
                    };
                    a.gpu = Some(AreaTextures {
                        heights,
                        grads,
                        cover,
                        sun: sun_tex,
                        sky,
                        contact,
                        items,
                    });
                }
            }
            a.sun_fresh = false;
        }
        Ok(())
    }
}

/// What the ground's uniforms say about the surfaces: each surface's looks' ladders, its number of looks and
/// whether it draws as rock, and its split, worked out once from the catalogue (A11.13 rule 1).
#[derive(Clone, Debug, PartialEq)]
pub struct SurfaceTable {
    /// `(first palette index, steps)` of each surface's looks, `SURFACE_LOOKS` a surface.
    pub looks: Vec<[i32; 2]>,
    /// `(looks, rock)` a surface.
    pub info: Vec<[i32; 2]>,
    pub split_at: Vec<[f32; 2]>,
    /// The split's octaves as (λ₀, 1/λ₀, λ₁, 1/λ₁).
    pub split_oct: Vec<[f32; 4]>,
    /// The micro-relief as (λ₀, 1/λ₀, octaves, greatest tilt) (A11.5).
    pub relief: Vec<[f32; 4]>,
}

impl SurfaceTable {
    pub fn new(cat: &kd_data::Catalogue, layout: &Layout) -> Result<SurfaceTable, String> {
        let mut t = SurfaceTable {
            looks: vec![[0, 1]; MAX_SURFACES * SURFACE_LOOKS],
            info: vec![[1, 0]; MAX_SURFACES],
            split_at: vec![[0.0; 2]; MAX_SURFACES],
            split_oct: vec![[0.0; 4]; MAX_SURFACES],
            relief: vec![[1.0, 1.0, 0.0, 0.0]; MAX_SURFACES],
        };
        for s in &cat.surfaces {
            let n = usize::from(s.number);
            if n >= MAX_SURFACES || s.looks.len() > SURFACE_LOOKS || s.split_m.len() > 2 {
                return Err(format!("surface {} does not fit the ground's uniforms", s.id));
            }
            for (k, &look) in s.looks.iter().enumerate() {
                let i = cat
                    .looks
                    .iter()
                    .position(|l| l.number == look)
                    .ok_or_else(|| format!("surface {}: look {look} is missing", s.id))?;
                let l = layout.ladders[i];
                t.looks[n * SURFACE_LOOKS + k] = [i32::from(l.base), i32::from(l.steps)];
            }
            t.info[n] = [s.looks.len() as i32, i32::from(s.rock)];
            for (k, &v) in s.split_at.iter().enumerate() {
                t.split_at[n][k] = v;
            }
            for (k, &m) in s.split_m.iter().enumerate() {
                t.split_oct[n][2 * k] = m;
                t.split_oct[n][2 * k + 1] = 1.0 / m;
            }
            let [smallest, largest] = s.relief_m;
            t.relief[n] = [
                largest,
                1.0 / largest,
                relief_octaves(smallest, largest) as f32,
                s.relief_tilt,
            ];
        }
        Ok(t)
    }
}

/// The ground's uniforms, in `GroundPass::u`'s order.
const UNIFORMS: [&str; 31] = [
    "u_spacing",
    "u_morph",
    "u_patches",
    "u_right",
    "u_up",
    "u_fwd",
    "u_inv_texel",
    "u_area_frac",
    "u_area_px",
    "u_depth",
    "u_pattern_off",
    "u_dither",
    "u_light_dir",
    "u_light_tan",
    "u_y",
    "u_range",
    "u_looks[0]",
    "u_surface_info[0]",
    "u_split_at[0]",
    "u_split_oct[0]",
    "u_skirt",
    "u_area_air",
    "u_haze_beta",
    "u_haze_scale",
    "u_eye",
    "u_haze_levels",
    "u_cover_ids[0]",
    "u_cover_textures",
    "u_cover_level",
    "u_relief[0]",
    "u_contact_level",
];

/// The ground's program and its uniforms (A11.13 rule 3), and the stones' and tufts' pass.
pub struct GroundPass {
    program: Program,
    u: [Option<glow::UniformLocation>; UNIFORMS.len()],
    table: SurfaceTable,
    /// Where the haze's levels 1 to 3 begin (A11.4).
    haze_levels: [f32; 3],
    cover: CoverPass,
}

/// The ground's triangles drawn by the last frame, and the stones and tufts handed to the GPU, for the bench
/// (A11.11).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct GroundStats {
    pub triangles: u64,
    pub items: u64,
}

/// The light's slope, `tan e`, from its direction (east, north, up): what the sun field's horizons are measured
/// against (A11.5), held below 10⁴ for light overhead.
pub fn light_tan(dir: [f32; 3]) -> f32 {
    field::azimuth(dir).map_or(1e4, |(_, t)| t.clamp(-1e4, 1e4))
}

/// The patches of an area a view needs: the first patch's column and row, and how many columns and rows, from the
/// view's footprint on the ground between the area's lowest and highest points (A11.5).
pub fn patches_in_view(view: &View, area: &Area, s: i32) -> Option<[i32; 4]> {
    let n = 256 / (PATCH_QUADS * s);
    let bounds = view.local_footprint(area.corner(), area.span_m)?;
    let size = (PATCH_QUADS * s) as f64;
    let lo = |v: f64| ((v / size).floor() as i32).clamp(0, n);
    let hi = |v: f64| ((v / size).ceil() as i32).clamp(0, n);
    let (x0, x1, y0, y1) = (lo(bounds[0]), hi(bounds[1]), lo(bounds[2]), hi(bounds[3]));
    (x1 > x0 && y1 > y0).then_some([x0, y0, x1 - x0, y1 - y0])
}

impl GroundPass {
    pub fn new(gl: &glow::Context, cat: &kd_data::Catalogue, layout: &Layout) -> Result<GroundPass, RenderError> {
        let program = Program::new(
            gl,
            "ground",
            &shaders::source(Stage::Vertex, shaders::GROUND_VERT),
            &shaders::source(Stage::Fragment, shaders::GROUND_FRAG),
        )?;
        program.set_sampler(gl, "u_heights", unit::HEIGHTS);
        program.set_sampler(gl, "u_grads", unit::GRADS);
        program.set_sampler(gl, "u_cover0", unit::COVER0);
        program.set_sampler(gl, "u_cover1", unit::COVER1);
        program.set_sampler(gl, "u_sun", unit::SUN);
        program.set_sampler(gl, "u_sky", unit::SKY);
        program.set_sampler(gl, "u_contact", unit::CONTACT);
        Ok(GroundPass {
            u: UNIFORMS.map(|name| program.uniform(gl, name)),
            program,
            table: SurfaceTable::new(cat, layout).map_err(RenderError::Gl)?,
            haze_levels: cat.air.haze_levels,
            cover: CoverPass::new(gl, &CoverTable::new(cat, layout).map_err(RenderError::Gl)?)?,
        })
    }

    /// Draws the loaded areas into the bound art target with the view's projection and the frame's light; the
    /// caller has cleared colour 0 to void and the depth to far.
    pub fn draw(
        &self,
        gl: &glow::Context,
        view: &View,
        store: &Store,
        lighting: &Lighting,
        vao: glow::VertexArray,
    ) -> GroundStats {
        let mut stats = GroundStats::default();
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
            u_spacing,
            u_morph,
            u_patches,
            u_right,
            u_up,
            u_fwd,
            u_inv_texel,
            u_area_frac,
            u_area_px,
            u_depth,
            u_pattern_off,
            u_dither,
            u_light_dir,
            u_light_tan,
            u_y,
            u_range,
            u_looks,
            u_surface_info,
            u_split_at,
            u_split_oct,
            u_skirt,
            u_area_air,
            u_haze_beta,
            u_haze_scale,
            u_eye,
            u_haze_levels,
            u_cover_ids,
            u_cover_textures,
            u_cover_level,
            u_relief,
            u_contact_level,
        ] = &self.u;
        let (s, morph) = spacing(view.texel as f32);
        gl::set_f32(gl, u_spacing.as_ref(), s as f32);
        gl::set_f32(gl, u_morph.as_ref(), morph);
        let b = view.basis_f32();
        gl::set_vec2(gl, u_right.as_ref(), [b.right[0], b.right[1]]);
        gl::set_vec3(gl, u_up.as_ref(), b.up);
        gl::set_vec3(gl, u_fwd.as_ref(), b.fwd);
        gl::set_f32(gl, u_inv_texel.as_ref(), (1.0 / view.texel) as f32);
        gl::set_ivec2(gl, u_dither.as_ref(), view.dither());
        let light = &lighting.light;
        gl::set_vec3(gl, u_light_dir.as_ref(), light.dir);
        gl::set_f32(gl, u_light_tan.as_ref(), light_tan(light.dir));
        gl::set_vec2(gl, u_y.as_ref(), [dot(LUM, light.sky), dot(LUM, light.sun)]);
        gl::set_vec2(gl, u_range.as_ref(), lighting.palette.range);
        gl::set_ivec2_array(gl, u_looks.as_ref(), &self.table.looks);
        gl::set_ivec2_array(gl, u_surface_info.as_ref(), &self.table.info);
        gl::set_vec2_array(gl, u_split_at.as_ref(), &self.table.split_at);
        gl::set_vec4_array(gl, u_split_oct.as_ref(), &self.table.split_oct);
        gl::set_vec4_array(gl, u_relief.as_ref(), &self.table.relief);
        let (beta, scale) = lighting.haze_air;
        gl::set_vec2(gl, u_haze_beta.as_ref(), beta);
        gl::set_vec2(gl, u_haze_scale.as_ref(), scale);
        gl::set_vec2(gl, u_eye.as_ref(), view.eye());
        gl::set_vec3(gl, u_haze_levels.as_ref(), self.haze_levels);
        // The coverage's level the art pixel's footprint reads, the same everywhere in an orthographic view.
        gl::set_f32(
            gl,
            u_cover_level.as_ref(),
            cover_level(view.texel as f32, COVER_LEVELS - 1),
        );
        gl::set_f32(
            gl,
            u_contact_level.as_ref(),
            cover_level(view.texel as f32 / CONTACT_M, CONTACT_LEVELS - 1),
        );
        for area in &store.areas {
            let Some(t) = &area.gpu else {
                continue;
            };
            let Some(patches) = patches_in_view(view, area, s) else {
                continue;
            };
            let p = view.area(area.corner());
            t.heights.bind(gl, unit::HEIGHTS);
            t.grads.bind(gl, unit::GRADS);
            for (k, cover) in t.cover.iter().enumerate() {
                cover.bind(gl, [unit::COVER0, unit::COVER1][k]);
            }
            t.sun.bind(gl, unit::SUN);
            t.sky.bind(gl, unit::SKY);
            t.contact.bind(gl, unit::CONTACT);
            let ids = area.coverage.ids;
            gl::set_ivec4_array(
                gl,
                u_cover_ids.as_ref(),
                &[[ids[0], ids[1], ids[2], ids[3]], [ids[4], ids[5], ids[6], ids[7]]],
            );
            gl::set_i32(gl, u_cover_textures.as_ref(), t.cover.len() as i32);
            gl::set_vec2(gl, u_area_frac.as_ref(), p.frac);
            gl::set_vec2(gl, u_area_px.as_ref(), p.px);
            gl::set_vec2(gl, u_depth.as_ref(), p.depth);
            gl::set_vec2(gl, u_pattern_off.as_ref(), p.pattern_off);
            gl::set_vec2(gl, u_area_air.as_ref(), p.air);
            // The patches, then the skirts round the area's edge: four sides of 256 / s segments.
            gl::set_ivec4(gl, u_patches.as_ref(), patches);
            gl::set_i32(gl, u_skirt.as_ref(), 0);
            let per_patch = PATCH_QUADS * PATCH_QUADS * 6;
            let count = patches[2] * patches[3];
            gl::draw_instanced(gl, vao, per_patch, count);
            gl::set_i32(gl, u_skirt.as_ref(), 1);
            gl::draw_instanced(gl, vao, 6, 4 * 256 / s);
            stats.triangles += (count * per_patch / 3 + 2 * 4 * 256 / s) as u64;
        }
        stats.items = self.cover.draw(gl, view, store, lighting, self.haze_levels, vao);
        stats
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// A rough patch of ground: a slope, a ridge and a step, so every kind of vertex moves.
    fn test_heights() -> Vec<f32> {
        (0..AREA_SIDE * AREA_SIDE)
            .map(|k| {
                let (x, y) = ((k % AREA_SIDE) as f32, (k / AREA_SIDE) as f32);
                0.1 * x + 3.0 * (x * 0.31).sin() * (y * 0.17).cos() + if x > 120.0 { 8.0 } else { 0.0 }
            })
            .collect()
    }

    // checks: PRE-20
    #[test]
    fn coverage_sums_to_one() {
        // Five surfaces in patches and specks over a whole area: each square is wholly its own surface, every texel
        // of every level, down to the one that spans the area, shares out exactly 255, and each surface's share of
        // the whole area is its count of squares, to within the rounding of the levels between.
        let surfaces: Vec<u8> = (0..AREA_SQUARES * AREA_SQUARES)
            .map(|i| {
                let (x, y) = ((i % AREA_SQUARES) as u64, (i / AREA_SQUARES) as u64);
                if kd_core::num::hash2(7, x * 977 + y).is_multiple_of(23) {
                    9
                } else {
                    [1, 4, 6, 12][((x / 37 + y / 23) % 4) as usize]
                }
            })
            .collect();
        let c = Coverage::new(&surfaces, AREA_SQUARES).unwrap();
        assert_eq!(c.ids, [1, 4, 6, 9, 12, -1, -1, -1]);
        assert_eq!((c.levels.len(), c.textures()), (COVER_LEVELS, 2));
        for (l, level) in c.levels.iter().enumerate() {
            assert_eq!(level.len(), (AREA_SQUARES >> l).pow(2));
            for t in level {
                assert_eq!(t.iter().map(|&v| u32::from(v)).sum::<u32>(), 255, "level {l}");
            }
        }
        assert!(c.levels[0].iter().all(|t| t.iter().filter(|&&v| v > 0).count() == 1));
        let whole = c.levels[COVER_LEVELS - 1][0];
        for (k, &id) in c.ids.iter().enumerate().take(5) {
            let want = surfaces.iter().filter(|&&n| i32::from(n) == id).count() as f32 / surfaces.len() as f32;
            assert!((f32::from(whole[k]) / 255.0 - want).abs() < 0.02, "surface {id}");
        }
        // Texture 1 holds the fifth surface's shares; four surfaces take one texture; nine are too many.
        assert_eq!(c.texture_levels(1)[0].len(), AREA_SQUARES * AREA_SQUARES * 4);
        assert_eq!(Coverage::new(&[3; 16], 4).unwrap().textures(), 1);
        assert!(Coverage::new(&(0..16).map(|i| (i % 9) as u8).collect::<Vec<_>>(), 4).is_err());
    }

    // checks: PRE-30 PRE-02
    #[test]
    fn a_small_move_of_the_light_is_worked_over_frames_a_jump_at_once() {
        // A light 12° high from the north-west, and the same an eighth of a degree round, then 40° round.
        let light = |az: f32| {
            let (e, az) = (12f32.to_radians(), az.to_radians());
            [e.cos() * az.sin(), e.cos() * az.cos(), e.sin()]
        };
        let mut store = Store::default();
        store
            .insert(AreaMeshes {
                id: AreaId(7),
                base_m: 100.0,
                heights: test_heights(),
                surfaces: vec![0; AREA_SQUARES * AREA_SQUARES],
            })
            .expect("a whole area");
        let heights = store.areas[0].heights.clone();
        // A new area's field comes at once, whatever the frame's share.
        assert_eq!(store.follow_light(light(315.0), &mut || false), 1);
        assert_eq!(store.areas[0].sun, Some(field::sun_field(&heights, light(315.0))));
        // A move under the step changes nothing.
        assert_eq!(store.follow_light(light(315.05), &mut || false), 0);
        assert!(store.areas[0].sun_job.is_none());
        // A small move: a piece a frame with no time to spare, the old field drawn until the new one is whole.
        let mut frames = 1;
        while store.follow_light(light(315.125), &mut || false) == 0 {
            assert_eq!(store.areas[0].sun, Some(field::sun_field(&heights, light(315.0))));
            frames += 1;
        }
        assert!(frames > 100, "{frames} frames");
        assert_eq!(store.areas[0].sun, Some(field::sun_field(&heights, light(315.125))));
        // With time to spare, the next small move lands in one frame.
        assert_eq!(store.follow_light(light(315.25), &mut || true), 1);
        // A jump, as a tap of the strip makes, comes at once and drops any field under way.
        assert_eq!(store.follow_light(light(315.4), &mut || false), 0);
        assert!(store.areas[0].sun_job.is_some());
        assert_eq!(store.follow_light(light(355.0), &mut || false), 1);
        assert!(store.areas[0].sun_job.is_none());
        assert_eq!(store.areas[0].sun, Some(field::sun_field(&heights, light(355.0))));
    }

    // checks: PRE-02 PRE-22
    #[test]
    fn morph_is_continuous() {
        let h = test_heights();
        // At the top of each spacing's range its odd vertices have arrived on the next spacing's mesh: the two
        // meshes are one surface, so the switch moves no point of the ground.
        for s in [1, 2, 4, 8] {
            let top = s as f32 / 1.25;
            let (below, m_below) = spacing(top);
            let (above, m_above) = spacing(top * 1.0001);
            assert_eq!((below, above), (s, 2 * s), "texel {top}");
            assert_eq!(m_below, 1.0);
            assert!(m_above < 0.01, "{m_above}");
            for k in 0..2_000 {
                let (x, y) = ((k * 37 % 2560) as f32 / 10.0, (k * 91 % 2560) as f32 / 10.0);
                let a = mesh_height(&h, x, y, s, 1.0);
                let b = mesh_height(&h, x, y, 2 * s, 0.0);
                assert!((a - b).abs() < 1e-3, "spacing {s} at ({x}, {y}): {a} and {b}");
            }
        }
        // Within a spacing, a small change of zoom moves the ground a little: no vertex jumps.
        let mut last: Option<Vec<f32>> = None;
        for i in 0..400 {
            let texel = 0.4 + i as f32 * 0.01;
            let (s, m) = spacing(texel);
            let now: Vec<f32> = (0..500)
                .map(|k| mesh_height(&h, (k * 53 % 2560) as f32 / 10.0, (k * 29 % 2560) as f32 / 10.0, s, m))
                .collect();
            if let Some(prev) = &last {
                let jump = now.iter().zip(prev).map(|(a, b)| (a - b).abs()).fold(0.0, f32::max);
                assert!(jump < 0.6, "texel {texel}: a point moved {jump} m");
            }
            last = Some(now);
        }
    }

    // checks: PRE-02
    #[test]
    fn spacing_follows_the_pixel() {
        let mut last = (1, 0.0f32);
        for i in 0..3_000 {
            let texel = 0.01 + i as f32 * 0.001;
            let (s, morph) = spacing(texel);
            // Never finer than a metre, never finer than 1.25 art pixels, and never twice as coarse as needed.
            assert!(s >= 1 && s as f32 >= 1.25 * texel);
            assert!(s == 1 || (s as f32) < 2.5 * texel, "texel {texel}: spacing {s}");
            assert!(s.count_ones() == 1);
            // The slide rises through each spacing's range and starts again at the next.
            if s == last.0 {
                assert!(morph >= last.1, "texel {texel}");
            } else {
                assert_eq!(s, 2 * last.0);
            }
            last = (s, morph);
        }
        // The camp stop's 1.1 m art pixels draw 2 m quads, the person stop's 3 cm ones the 1 m heights.
        assert_eq!(spacing(1.1).0, 2);
        assert_eq!(spacing(0.03), (1, 0.0));
        // Gradients are central differences, one-sided at the edge.
        let h = test_heights();
        let g = gradients(&h);
        let at = |x: usize, y: usize| h[y * AREA_SIDE + x];
        assert_eq!(g[10 * AREA_SIDE + 5][0], (at(6, 10) - at(4, 10)) / 2.0);
        assert_eq!(g[10 * AREA_SIDE][0], at(1, 10) - at(0, 10));
        assert_eq!(g[256 * AREA_SIDE + 7][1], at(7, 256) - at(7, 255));
    }
}
