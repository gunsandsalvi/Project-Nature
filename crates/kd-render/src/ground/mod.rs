//! The ground (A11.5): the CPU store of the loaded areas, their textures, the morphing patch and the ground's draw.
//! Each area keeps its heights, their gradients, its surfaces and its light fields on the CPU, so after a lost
//! context its textures are made again from them alone (A11.13 rule 5): the sky field once, as it arrives, and the
//! sun field for the light's azimuth, made again as the azimuth moves (`field`). One shared 16 × 16 patch of quads, made from vertex and instance
//! numbers with no vertex buffer, covers the area at the spacing the art pixel asks for, its odd vertices sliding
//! onto the next spacing's mesh as the spacing's range ends, so a change of spacing moves nothing (A11.1 rule 3).
//!
//! Implements PRE-02 and PRE-20, see A11.5: a real 3D ground drawn at low resolution, its surfaces in their looks'
//! steps.

use kd_core::geo::{AreaId, Pos};
use kd_view::{AREA_SIDE, AREA_SQUARES, AreaMeshes};

use crate::RenderError;
use crate::camera::View;
use crate::field::{self, SunField};
use crate::frame::Lighting;
use crate::gl::{self, Format, Program, State, Texture, unit};
use crate::light::{LUM, dot};
use crate::looks::Layout;
use crate::shaders::{self, Stage};

/// Quads along a patch's side.
pub const PATCH_QUADS: i32 = 16;
/// How far the skirts hang below the loaded ground's outer edge (A11.5).
pub const SKIRT_M: f32 = 4.0;
/// Surfaces the ground's uniforms hold, by number.
pub const MAX_SURFACES: usize = 16;
/// Looks a surface may name (A11.5).
pub const SURFACE_LOOKS: usize = 3;

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
    pub surfaces: Vec<u8>,
    /// Its lowest and highest points, metres above `base_m`.
    pub span_m: [f32; 2],
    /// The share of the sky each point's horizon leaves open, in 255ths (A11.5).
    pub sky: Vec<u8>,
    /// The sun field for the light's azimuth, none until the first frame's light (A11.5).
    pub sun: Option<SunField>,
    /// Whether the sun field is newer than its texture.
    sun_fresh: bool,
    gpu: Option<[Texture; 5]>,
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
            span_m: [lo, hi],
            sun: None,
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

    /// Works out the sun field of each area that has none, or one made for an azimuth `SUN_FIELD_STEP_DEG` or more
    /// from the light's `dir` (east, north, up; A11.5); its texture follows at the next upload. Returns how many
    /// it made.
    pub fn follow_light(&mut self, dir: [f32; 3]) -> usize {
        let mut made = 0;
        for a in &mut self.areas {
            if a.sun.as_ref().is_none_or(|f| f.stale(dir)) {
                a.sun = Some(field::sun_field(&a.heights, dir));
                a.sun_fresh = true;
                made += 1;
            }
        }
        made
    }

    /// Makes the textures of areas that lack them (heights `R32F`, gradients `RG32F`, surfaces `R8`, the sun field
    /// `R32F` and the sky field `R8`), and brings a sun field's texture up to date when it was made again (A11.5).
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
                        t[3].update(gl, &sun())?;
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
                    let surfaces = Texture::new(gl, Format::R8, squares, squares, Some(&a.surfaces))?;
                    let sun_tex = Texture::new(gl, Format::R32F, side, side, Some(&sun()))?;
                    let sky = Texture::new(gl, Format::R8, side, side, Some(&a.sky))?;
                    a.gpu = Some([heights, grads, surfaces, sun_tex, sky]);
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
}

impl SurfaceTable {
    pub fn new(cat: &kd_data::Catalogue, layout: &Layout) -> Result<SurfaceTable, String> {
        let mut t = SurfaceTable {
            looks: vec![[0, 1]; MAX_SURFACES * SURFACE_LOOKS],
            info: vec![[1, 0]; MAX_SURFACES],
            split_at: vec![[0.0; 2]; MAX_SURFACES],
            split_oct: vec![[0.0; 4]; MAX_SURFACES],
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
        }
        Ok(t)
    }
}

/// The ground's uniforms, in `GroundPass::u`'s order.
const UNIFORMS: [&str; 26] = [
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
];

/// The ground's program and its uniforms (A11.13 rule 3).
pub struct GroundPass {
    program: Program,
    u: [Option<glow::UniformLocation>; UNIFORMS.len()],
    table: SurfaceTable,
    /// Where the haze's levels 1 to 3 begin (A11.4).
    haze_levels: [f32; 3],
}

/// Triangles drawn by the last frame, for the bench (A11.11).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct GroundStats {
    pub triangles: u64,
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
        program.set_sampler(gl, "u_surfaces", unit::SURFACES);
        program.set_sampler(gl, "u_sun", unit::SUN);
        program.set_sampler(gl, "u_sky", unit::SKY);
        Ok(GroundPass {
            u: UNIFORMS.map(|name| program.uniform(gl, name)),
            program,
            table: SurfaceTable::new(cat, layout).map_err(RenderError::Gl)?,
            haze_levels: cat.air.haze_levels,
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
        let (beta, scale) = lighting.haze_air;
        gl::set_vec2(gl, u_haze_beta.as_ref(), beta);
        gl::set_vec2(gl, u_haze_scale.as_ref(), scale);
        gl::set_vec2(gl, u_eye.as_ref(), view.eye());
        gl::set_vec3(gl, u_haze_levels.as_ref(), self.haze_levels);
        for area in &store.areas {
            let Some([heights, grads, surfaces, sun, sky]) = &area.gpu else {
                continue;
            };
            let Some(patches) = patches_in_view(view, area, s) else {
                continue;
            };
            let p = view.area(area.corner());
            heights.bind(gl, unit::HEIGHTS);
            grads.bind(gl, unit::GRADS);
            surfaces.bind(gl, unit::SURFACES);
            sun.bind(gl, unit::SUN);
            sky.bind(gl, unit::SKY);
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
