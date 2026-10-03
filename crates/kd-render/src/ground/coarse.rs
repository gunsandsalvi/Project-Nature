//! Coarse ground (A11.5): the land beyond the view areas, and at the valley stop all of it, drawn from the cells'
//! coarse ground in tiles of 8 × 8 cells, 257 × 257 points 32 m apart, so a tile is laid out as an area is and the
//! ground's patch, shader rules and light fields serve it unchanged but for the points' spacing. Each tile keeps its
//! heights, their gradients, its squares' materials as coverage, where the sea lies, its cells' cover and the surface
//! each cover group shows as, its water lines, and its light fields, made over frames by jobs so no frame waits
//! (A11.11), nearest the view's target first.
//!
//! Three passes draw them: the ground, its cover as looks, crowns and the map look (`coarse.frag`); the sea, a flat
//! surface at its level over the points it covers (`sea.frag`); and the rivers' and big streams' lines, never under
//! an art pixel wide (`line.vert`, `PRE-26`).
//!
//! The plan called for 4 km tiles; 8 km tiles of 257 points are an area's own layout, so the light fields, whose
//! code is fixed to it, and the coverage serve them as they are, and an 8 km tile is the patterns' 8,192 m block.
//!
//! Implements PRE-03, PRE-29 and PRE-26, see A11.5 and A11.6: coarse ground beyond the areas, the map look from
//! above, and every river at least an art pixel wide.

use kd_core::geo::{CELLS_X, CELLS_Y, CellIx, Pos, TICKS_PER_M, W};
use kd_view::{CoarseTile, TILE_CELLS, TILE_POINT_M, TILE_RING, TILE_SIDE};

use super::{COVER_LEVELS, Coverage, PATCH_QUADS, SurfaceTable, gradients, patches_of, spacing};
use crate::RenderError;
use crate::camera::View;
use crate::field::{self, SkyJob, SunField, SunJob};
use crate::frame::Lighting;
use crate::gl::{self, Format, Program, State, Texture, unit};
use crate::light::{LUM, dot};
use crate::looks::Layout;
use crate::pixel::cover_level;
use crate::shaders::{self, Stage};

/// A world cell's side, metres.
pub const CELL_M: f32 = (W / CELLS_X as i32 / TICKS_PER_M) as f32;
/// A tile's side, metres.
pub const TILE_M: f32 = CELL_M * TILE_CELLS as f32;
/// Pieces of water line a row of the lines' texture, two `RGBA32F` texels each.
pub const LINES_ROW: usize = 256;
/// How far a water line is lifted above coarse ground, which carves no channel, metres (A11.6).
pub const LINE_LIFT_M: f32 = 1.2;
/// The least half-width of a water line across the screen, art pixels: every river at least one wide (A11.6).
pub const LINE_MIN_PX: f32 = 0.95;

/// Coarse ground's tuned numbers (`data/tuning/render.md`, A11.13 rule 6): the crowns' radius, least and most; the
/// flat cover's noise, as (λ₀, 1/λ₀, λ₁, 1/λ₁), and how widely its draws spread; and water's light, the floor deep
/// water keeps and the depth over which a bed's light fades by `e` down and back up.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct CoarseLook {
    pub crown_r: [f32; 2],
    pub cover_oct: [f32; 4],
    pub cover_spread: f32,
    pub water: [f32; 2],
}

impl CoarseLook {
    /// The numbers from the catalogue's `render` tuning; an error names what is missing or out of its range.
    pub fn new(cat: &kd_data::Catalogue) -> Result<CoarseLook, String> {
        let t = cat.tuning("render")?;
        let len = |k: &str| t.get(k, kd_data::Measure::Length);
        let (large, small) = (len("cover.noise_large")?, len("cover.noise_small")?);
        let crown_r = [len("crown.radius_min")?, len("crown.radius_max")?];
        let keeps = t.number("water.keeps")?;
        let scatter = t.number("water.scatter")?;
        if !(0.0 < keeps && keeps < 1.0) || !(0.0..1.0).contains(&scatter) {
            return Err(format!(
                "tuning render: water keeps {keeps} and scatters {scatter}, each under 1"
            ));
        }
        if !(0.0 < crown_r[0] && crown_r[0] <= crown_r[1]) || !(large > 0.0 && small > 0.0) {
            return Err("tuning render: crowns' radius and the cover's noise above 0, rising".into());
        }
        Ok(CoarseLook {
            crown_r,
            cover_oct: [large, 1.0 / large, small, 1.0 / small],
            cover_spread: t.number("cover.spread")?,
            water: [scatter, -1.0 / (2.0 * keeps.ln())],
        })
    }
}

/// A tile's id: its north-west cell, whose column and row are multiples of `TILE_CELLS`.
pub fn tile_of(c: CellIx) -> CellIx {
    let (x, y) = c.xy();
    CellIx::at(x - x % TILE_CELLS, y - y % TILE_CELLS)
}

/// A loaded tile on the CPU (A11.13 rule 5), and its textures while the context lives.
pub struct Tile {
    pub cell: CellIx,
    pub base_m: f32,
    /// Heights in metres above `base_m`, and the same in the points' spacing, which the fields read so their
    /// slopes come out true.
    pub heights: Vec<f32>,
    scaled: Vec<f32>,
    /// Each point's slope east and south.
    pub gradients: Vec<[f32; 2]>,
    /// The squares' materials as coverage, and the soil's surface, which shows the cells' cover.
    pub coverage: Coverage,
    pub soil: u8,
    /// 255 where the sea lies over a point.
    pub sea: Vec<u8>,
    /// The ring of cells' cover, RGBA8 (trees, bushes, reeds, bare), and the surface each group shows as, two to a
    /// byte (trees and bushes, grass and herbs and reeds, bare).
    cells: Vec<u8>,
    groups: Vec<u8>,
    /// The water lines to draw: a piece's start (east, south, up, half-width) and end (east, south, up, depth).
    pub lines: Vec<[f32; 8]>,
    /// Its lowest and highest points, metres above `base_m`.
    pub span_m: [f32; 2],
    /// Its fields; until they come, open sky and level horizons.
    pub sky: Option<Vec<u8>>,
    pub sun: Option<SunField>,
    fresh: bool,
    gpu: Option<TileTextures>,
}

impl Tile {
    /// The tile's north-west corner at its base height.
    pub fn corner(&self) -> Pos {
        Pos {
            z: (self.base_m * 256.0).round() as i32,
            ..self.cell.origin()
        }
    }
}

/// A tile's textures while the context lives (A11.13 rule 5).
struct TileTextures {
    heights: Texture,
    grads: Texture,
    cover: Vec<Texture>,
    sun: Texture,
    sky: Texture,
    cells: Texture,
    groups: Texture,
    sea: Texture,
    lines: Option<Texture>,
}

/// Field work under way on one tile.
enum Job {
    Sun(SunJob),
    Sky(SkyJob),
}

/// The loaded tiles (A11.5).
#[derive(Default)]
pub struct CoarseStore {
    pub tiles: Vec<Tile>,
    /// Streams draining less than this draw no line (`data/tuning/render.md`).
    pub stream_line_min_km2: f32,
    job: Option<(CellIx, Job)>,
}

fn f32_bytes(values: impl Iterator<Item = f32>) -> Vec<u8> {
    values.flat_map(f32::to_ne_bytes).collect()
}

impl CoarseStore {
    /// An empty store, with the render tuning's least drainage for a stream's line.
    pub fn new(cat: &kd_data::Catalogue) -> Result<CoarseStore, String> {
        Ok(CoarseStore {
            stream_line_min_km2: cat.tuning("render")?.number("stream_line_min_km2")?,
            ..CoarseStore::default()
        })
    }

    /// Whether a tile is loaded.
    pub fn has(&self, cell: CellIx) -> bool {
        self.tiles.iter().any(|t| t.cell == cell)
    }

    /// Takes a tile, replacing an earlier one of the same id; its textures are made at the next upload, its fields
    /// over the frames after.
    pub fn insert(&mut self, m: CoarseTile) -> Result<(), RenderError> {
        let side = TILE_SIDE;
        let squares = side - 1;
        let ring = TILE_RING * TILE_RING;
        if m.heights.len() != side * side
            || m.surfaces.len() != squares * squares
            || m.sea.len() != side * side
            || m.cover.len() != ring
            || m.cover_surfaces.len() != ring
        {
            return Err(RenderError::Gl(format!("tile {}: wrong sizes", m.cell.0)));
        }
        let all_surfaces = m.surfaces.iter().chain(m.cover_surfaces.iter().flatten());
        if let Some(&n) = all_surfaces.clone().find(|&&n| usize::from(n) >= super::MAX_SURFACES) {
            return Err(RenderError::Gl(format!("tile {}: surface {n}", m.cell.0)));
        }
        let base_m = (m.base_m * 256.0).round() / 256.0;
        let heights: Vec<f32> = m.heights.iter().map(|h| h + (m.base_m - base_m)).collect();
        let lo = heights.iter().copied().fold(f32::INFINITY, f32::min);
        let hi = heights.iter().copied().fold(f32::NEG_INFINITY, f32::max);
        let coverage =
            Coverage::new(&m.surfaces, squares).map_err(|e| RenderError::Gl(format!("tile {}: {e}", m.cell.0)))?;
        let unit = TILE_POINT_M;
        let gradients = gradients(&heights).iter().map(|g| [g[0] / unit, g[1] / unit]).collect();
        let cells = m.cover.iter().flat_map(|c| [c[0], c[1], c[3], c[4]]).collect();
        let groups = m
            .cover_surfaces
            .iter()
            .flat_map(|g| [g[0] | g[1] << 4, g[2] | g[3] << 4, g[4], 0])
            .collect();
        let min_km2 = self.stream_line_min_km2;
        let lines = m
            .water
            .iter()
            .filter(|p| p.river || p.drainage_km2 >= min_km2)
            .map(|p| {
                let (a, b) = (p.from, p.to);
                [
                    a[0],
                    a[1],
                    a[2] + (m.base_m - base_m),
                    p.half_width_m,
                    b[0],
                    b[1],
                    b[2] + (m.base_m - base_m),
                    p.depth_m,
                ]
            })
            .collect();
        let tile = Tile {
            cell: m.cell,
            base_m,
            scaled: heights.iter().map(|h| h / unit).collect(),
            heights,
            gradients,
            coverage,
            soil: m.soil_surface,
            sea: m.sea.iter().map(|&s| if s > 0 { 255 } else { 0 }).collect(),
            cells,
            groups,
            lines,
            span_m: [lo, hi],
            sky: None,
            sun: None,
            fresh: false,
            gpu: None,
        };
        self.tiles.retain(|t| t.cell != tile.cell);
        if self.job.as_ref().is_some_and(|(c, _)| *c == tile.cell) {
            self.job = None;
        }
        self.tiles.push(tile);
        Ok(())
    }

    /// Keeps only the tiles `keep` names.
    pub fn retain(&mut self, keep: &[CellIx]) {
        self.tiles.retain(|t| keep.contains(&t.cell));
        if self.job.as_ref().is_some_and(|(c, _)| !keep.contains(c)) {
            self.job = None;
        }
    }

    /// Works the tiles' fields while `more()` allows, nearest `near` (a tile's middle, metres east and south of the
    /// world's corner) first: each tile's sun field for the light's azimuth `dir`, made again when the azimuth has
    /// moved `SUN_FIELD_STEP_DEG`, the old one drawn meanwhile, then its sky field once. Returns how many it finished.
    pub fn follow_light(&mut self, dir: [f32; 3], near: [f64; 2], more: &mut dyn FnMut() -> bool) -> usize {
        let mut made = 0;
        loop {
            if self.job.is_none() {
                let dist = |t: &Tile| {
                    let o = t.cell.origin();
                    let half = f64::from(TILE_M) / 2.0;
                    let dx = f64::from(o.x) / 256.0 + half - near[0];
                    let dy = f64::from(o.y) / 256.0 + half - near[1];
                    dx * dx + dy * dy
                };
                let pick = |want: &dyn Fn(&Tile) -> bool| {
                    self.tiles
                        .iter()
                        .filter(|t| want(t))
                        .min_by(|a, b| dist(a).total_cmp(&dist(b)))
                        .map(|t| t.cell)
                };
                let stale = |t: &Tile| t.sun.as_ref().is_none_or(|f| f.stale(dir));
                if let Some(c) = pick(&stale) {
                    match SunJob::new(dir) {
                        Some(job) => self.job = Some((c, Job::Sun(job))),
                        None => {
                            // Light overhead: level horizons, at once.
                            if let Some(t) = self.tiles.iter_mut().find(|t| t.cell == c) {
                                t.sun = Some(field::sun_field(&t.scaled, dir));
                                t.fresh = true;
                                made += 1;
                            }
                            continue;
                        }
                    }
                } else if let Some(c) = pick(&|t: &Tile| t.sky.is_none()) {
                    self.job = Some((c, Job::Sky(SkyJob::new())));
                } else {
                    return made;
                }
            }
            let Some((c, job)) = &mut self.job else {
                return made;
            };
            let Some(t) = self.tiles.iter_mut().find(|t| t.cell == *c) else {
                self.job = None;
                continue;
            };
            let done = match job {
                Job::Sun(j) => j.step(&t.scaled, more).map(|f| t.sun = Some(f)),
                Job::Sky(j) => j.step(&t.scaled, more).map(|f| {
                    t.sky = Some(f.iter().map(|v| (v.clamp(0.0, 1.0) * 255.0).round() as u8).collect());
                }),
            };
            if done.is_none() {
                return made;
            }
            t.fresh = true;
            made += 1;
            self.job = None;
            if !more() {
                return made;
            }
        }
    }

    /// Makes the textures of tiles that lack them, and brings a field's texture up to date when it was made again.
    pub fn upload(&mut self, gl: &glow::Context) -> Result<(), RenderError> {
        let (side, squares, ring) = (TILE_SIDE as u32, (TILE_SIDE - 1) as u32, TILE_RING as u32);
        for t in &mut self.tiles {
            let sun = || match &t.sun {
                Some(f) => f32_bytes(f.horizon.iter().copied()),
                None => f32_bytes(std::iter::repeat_n(0.0, TILE_SIDE * TILE_SIDE)),
            };
            let sky = || t.sky.clone().unwrap_or_else(|| vec![255; TILE_SIDE * TILE_SIDE]);
            match &t.gpu {
                Some(g) => {
                    if t.fresh {
                        g.sun.update(gl, &sun())?;
                        g.sky.update(gl, &sky())?;
                    }
                }
                None => {
                    let cover = (0..t.coverage.textures())
                        .map(|k| {
                            Texture::with_levels(gl, Format::Rgba8, squares, squares, &t.coverage.texture_levels(k))
                        })
                        .collect::<Result<Vec<_>, _>>()?;
                    let lines = if t.lines.is_empty() {
                        None
                    } else {
                        let rows = t.lines.len().div_ceil(LINES_ROW);
                        let mut bytes = f32_bytes(t.lines.iter().flatten().copied());
                        bytes.resize(rows * LINES_ROW * 32, 0);
                        Some(Texture::new(
                            gl,
                            Format::Rgba32F,
                            2 * LINES_ROW as u32,
                            rows as u32,
                            Some(&bytes),
                        )?)
                    };
                    t.gpu = Some(TileTextures {
                        heights: Texture::new(
                            gl,
                            Format::R32F,
                            side,
                            side,
                            Some(&f32_bytes(t.heights.iter().copied())),
                        )?,
                        grads: Texture::new(
                            gl,
                            Format::Rg32F,
                            side,
                            side,
                            Some(&f32_bytes(t.gradients.iter().flatten().copied())),
                        )?,
                        cover,
                        sun: Texture::new(gl, Format::R32F, side, side, Some(&sun()))?,
                        sky: Texture::new(gl, Format::R8, side, side, Some(&sky()))?,
                        cells: Texture::new(gl, Format::Rgba8, ring, ring, Some(&t.cells))?,
                        groups: Texture::new(gl, Format::Rgba8, ring, ring, Some(&t.groups))?,
                        sea: Texture::new(gl, Format::R8, side, side, Some(&t.sea))?,
                        lines,
                    });
                }
            }
            t.fresh = false;
        }
        Ok(())
    }
}

/// The tiles a view needs: every tile any part of whose ground, between `span_m` metres above the sea, falls within
/// `margin_m` of the art target, nearest the view's target first, never across the poles' rows.
pub fn tiles_in_view(view: &View, span_m: [f32; 2], margin_m: f64) -> Vec<CellIx> {
    let target = view.target;
    let corner = Pos { z: 0, ..target };
    let Some(b) = view.local_footprint(corner, span_m) else {
        return Vec::new();
    };
    let (tx, ty) = (f64::from(target.x) / 256.0, f64::from(target.y) / 256.0);
    let tile = f64::from(TILE_M);
    let rows = f64::from(CELLS_Y / TILE_CELLS);
    let lo = |v: f64| ((v - margin_m) / tile).floor();
    let hi = |v: f64| ((v + margin_m) / tile).floor();
    let mut out: Vec<(f64, CellIx)> = Vec::new();
    let (y0, y1) = (lo(ty + b[2]).max(0.0), hi(ty + b[3]).min(rows - 1.0));
    let (x0, x1) = (lo(tx + b[0]), hi(tx + b[1]));
    if x1 - x0 > 512.0 || y1 < y0 {
        return Vec::new();
    }
    let cols = i64::from(CELLS_X / TILE_CELLS);
    let mut y = y0;
    while y <= y1 {
        let mut x = x0;
        while x <= x1 {
            let col = (x as i64).rem_euclid(cols) as u32;
            let mid = [(x + 0.5) * tile - tx, (y + 0.5) * tile - ty];
            out.push((
                mid[0] * mid[0] + mid[1] * mid[1],
                CellIx::at(col * TILE_CELLS, y as u32 * TILE_CELLS),
            ));
            x += 1.0;
        }
        y += 1.0;
    }
    out.sort_by(|a, b| a.0.total_cmp(&b.0).then(a.1.cmp(&b.1)));
    // A view wider than the world meets a tile again round it: the nearest stays.
    let mut seen = std::collections::BTreeSet::new();
    out.into_iter().map(|p| p.1).filter(|c| seen.insert(*c)).collect()
}

/// A program and its uniforms by name (A11.13 rule 3).
struct Pass {
    program: Program,
    names: &'static [&'static str],
    locs: Vec<Option<glow::UniformLocation>>,
}

impl Pass {
    fn new(
        gl: &glow::Context,
        name: &'static str,
        vertex: String,
        fragment: String,
        names: &'static [&'static str],
        samplers: &[(&str, u32)],
    ) -> Result<Pass, RenderError> {
        let program = Program::new(gl, name, &vertex, &fragment)?;
        for &(s, u) in samplers {
            program.set_sampler(gl, s, u);
        }
        Ok(Pass {
            locs: names.iter().map(|n| program.uniform(gl, n)).collect(),
            program,
            names,
        })
    }

    fn at(&self, name: &str) -> Option<&glow::UniformLocation> {
        let k = self.names.iter().position(|n| *n == name)?;
        self.locs[k].as_ref()
    }

    /// Sets the view's and the light's uniforms the three passes share, those a pass lacks skipped.
    fn view_and_light(&self, gl: &glow::Context, view: &View, lighting: &Lighting, haze_levels: [f32; 3]) {
        let b = view.basis_f32();
        gl::set_vec2(gl, self.at("u_right"), [b.right[0], b.right[1]]);
        gl::set_vec3(gl, self.at("u_up"), b.up);
        gl::set_vec3(gl, self.at("u_fwd"), b.fwd);
        gl::set_f32(gl, self.at("u_inv_texel"), (1.0 / view.texel) as f32);
        gl::set_ivec2(gl, self.at("u_dither"), view.dither());
        let light = &lighting.light;
        gl::set_vec3(gl, self.at("u_light_dir"), light.dir);
        gl::set_f32(gl, self.at("u_light_tan"), super::light_tan(light.dir));
        gl::set_vec2(gl, self.at("u_y"), [dot(LUM, light.sky), dot(LUM, light.sun)]);
        gl::set_vec2(gl, self.at("u_range"), lighting.palette.range);
        let (beta, scale) = lighting.haze_air;
        gl::set_vec2(gl, self.at("u_haze_beta"), beta);
        gl::set_vec2(gl, self.at("u_haze_scale"), scale);
        gl::set_vec2(gl, self.at("u_eye"), view.eye());
        gl::set_vec3(gl, self.at("u_haze_levels"), haze_levels);
    }

    /// Sets a tile's projection (A11.2).
    fn place(&self, gl: &glow::Context, view: &View, t: &Tile) {
        let p = view.area(t.corner());
        gl::set_vec2(gl, self.at("u_area_frac"), p.frac);
        gl::set_vec2(gl, self.at("u_area_px"), p.px);
        gl::set_vec2(gl, self.at("u_depth"), p.depth);
        gl::set_vec2(gl, self.at("u_area_air"), p.air);
        gl::set_vec2(gl, self.at("u_pattern_off"), p.pattern_off);
    }
}

/// The ground's patch uniforms, which the coarse and the sea pass share.
const PATCH: [&str; 13] = [
    "u_unit",
    "u_spacing",
    "u_morph",
    "u_patches",
    "u_skirt",
    "u_right",
    "u_up",
    "u_fwd",
    "u_inv_texel",
    "u_area_frac",
    "u_area_px",
    "u_depth",
    "u_area_air",
];

const COARSE_NAMES: [&str; 35] = [
    PATCH[0],
    PATCH[1],
    PATCH[2],
    PATCH[3],
    PATCH[4],
    PATCH[5],
    PATCH[6],
    PATCH[7],
    PATCH[8],
    PATCH[9],
    PATCH[10],
    PATCH[11],
    PATCH[12],
    "u_crown_r",
    "u_cover_oct",
    "u_cover_spread",
    "u_cover_ids[0]",
    "u_cover_textures",
    "u_cover_level",
    "u_soil",
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
    "u_relief[0]",
    "u_haze_beta",
    "u_haze_scale",
    "u_eye",
    "u_haze_levels",
];

const SEA_NAMES: [&str; 25] = [
    PATCH[0],
    PATCH[1],
    PATCH[2],
    PATCH[3],
    PATCH[4],
    PATCH[5],
    PATCH[6],
    PATCH[7],
    PATCH[8],
    PATCH[9],
    PATCH[10],
    PATCH[11],
    PATCH[12],
    "u_level",
    "u_water",
    "u_water_light",
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

const LINE_NAMES: [&str; 19] = [
    "u_right",
    "u_up",
    "u_fwd",
    "u_inv_texel",
    "u_area_frac",
    "u_area_px",
    "u_depth",
    "u_area_air",
    "u_water",
    "u_water_light",
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

/// The water's look, from the biome the sea takes (A11.6): its first palette index and steps.
fn water_ladder(cat: &kd_data::Catalogue, layout: &Layout) -> Result<[i32; 2], String> {
    let sea = cat
        .biomes
        .iter()
        .find(|b| b.landform == Some(kd_data::world::Landform::Sea))
        .ok_or("no biome takes the sea")?;
    let k = cat
        .looks
        .iter()
        .position(|l| l.number == sea.look)
        .ok_or("the sea's look is missing")?;
    let l = layout.ladders[k];
    Ok([i32::from(l.base), i32::from(l.steps)])
}

/// What the coarse passes drew last frame, for the bench (A11.11).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct CoarseStats {
    pub tiles: u64,
    pub triangles: u64,
    pub pieces: u64,
}

/// The coarse ground's, the sea's and the water lines' programs (A11.5, A11.6).
pub struct CoarsePass {
    ground: Pass,
    sea: Pass,
    lines: Pass,
    table: SurfaceTable,
    water: [i32; 2],
    look: CoarseLook,
}

impl CoarsePass {
    pub fn new(gl: &glow::Context, cat: &kd_data::Catalogue, layout: &Layout) -> Result<CoarsePass, RenderError> {
        let ground = Pass::new(
            gl,
            "coarse",
            shaders::source(Stage::Vertex, shaders::GROUND_VERT),
            shaders::source(Stage::Fragment, shaders::COARSE_FRAG),
            &COARSE_NAMES,
            &[
                ("u_heights", unit::HEIGHTS),
                ("u_grads", unit::GRADS),
                ("u_cover0", unit::COVER0),
                ("u_cover1", unit::COVER1),
                ("u_sun", unit::SUN),
                ("u_sky", unit::SKY),
                ("u_cells", unit::CELLS),
                ("u_groups", unit::GROUPS),
            ],
        )?;
        let sea = Pass::new(
            gl,
            "sea",
            shaders::source_with(Stage::Vertex, shaders::GROUND_VERT, &["SEA"]),
            shaders::source_with(Stage::Fragment, shaders::SEA_FRAG, &["SEA"]),
            &SEA_NAMES,
            &[("u_heights", unit::HEIGHTS), ("u_sea", unit::SEA)],
        )?;
        let lines = Pass::new(
            gl,
            "lines",
            shaders::source(Stage::Vertex, shaders::LINE_VERT),
            shaders::source(Stage::Fragment, shaders::LINE_FRAG),
            &LINE_NAMES,
            &[("u_lines", unit::LINES)],
        )?;
        Ok(CoarsePass {
            ground,
            sea,
            lines,
            table: SurfaceTable::new(cat, layout).map_err(RenderError::Gl)?,
            water: water_ladder(cat, layout).map_err(RenderError::Gl)?,
            look: CoarseLook::new(cat).map_err(RenderError::Gl)?,
        })
    }

    /// Draws the tiles in view into the bound art target over what it holds, with its depth: the ground, the sea's
    /// surface, then the water lines.
    pub fn draw(
        &self,
        gl: &glow::Context,
        view: &View,
        store: &CoarseStore,
        lighting: &Lighting,
        haze_levels: [f32; 3],
        vao: glow::VertexArray,
    ) -> CoarseStats {
        let mut stats = CoarseStats::default();
        if store.tiles.is_empty() {
            return stats;
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
        let unit_m = TILE_POINT_M;
        let (s, morph) = spacing(view.texel as f32 / unit_m);
        let in_view: Vec<(&Tile, &TileTextures, [i32; 4])> = store
            .tiles
            .iter()
            .filter_map(|t| {
                let g = t.gpu.as_ref()?;
                let p = patches_of(view, t.corner(), t.span_m, unit_m, s)?;
                Some((t, g, p))
            })
            .collect();
        let per_patch = PATCH_QUADS * PATCH_QUADS * 6;
        // The ground.
        let p = &self.ground;
        p.program.bind(gl);
        p.view_and_light(gl, view, lighting, haze_levels);
        gl::set_f32(gl, p.at("u_unit"), unit_m);
        gl::set_f32(gl, p.at("u_spacing"), s as f32);
        gl::set_f32(gl, p.at("u_morph"), morph);
        gl::set_f32(
            gl,
            p.at("u_cover_level"),
            cover_level(view.texel as f32 / unit_m, COVER_LEVELS - 1),
        );
        gl::set_ivec2_array(gl, p.at("u_looks[0]"), &self.table.looks);
        gl::set_ivec2_array(gl, p.at("u_surface_info[0]"), &self.table.info);
        gl::set_vec2_array(gl, p.at("u_split_at[0]"), &self.table.split_at);
        gl::set_vec4_array(gl, p.at("u_split_oct[0]"), &self.table.split_oct);
        gl::set_vec4_array(gl, p.at("u_relief[0]"), &self.table.relief);
        gl::set_vec2(gl, p.at("u_crown_r"), self.look.crown_r);
        gl::set_vec4(gl, p.at("u_cover_oct"), self.look.cover_oct);
        gl::set_f32(gl, p.at("u_cover_spread"), self.look.cover_spread);
        for &(t, g, patches) in &in_view {
            g.heights.bind(gl, unit::HEIGHTS);
            g.grads.bind(gl, unit::GRADS);
            for (k, c) in g.cover.iter().enumerate() {
                c.bind(gl, [unit::COVER0, unit::COVER1][k]);
            }
            g.sun.bind(gl, unit::SUN);
            g.sky.bind(gl, unit::SKY);
            g.cells.bind(gl, unit::CELLS);
            g.groups.bind(gl, unit::GROUPS);
            let ids = t.coverage.ids;
            gl::set_ivec4_array(
                gl,
                p.at("u_cover_ids[0]"),
                &[[ids[0], ids[1], ids[2], ids[3]], [ids[4], ids[5], ids[6], ids[7]]],
            );
            gl::set_i32(gl, p.at("u_cover_textures"), g.cover.len() as i32);
            gl::set_i32(gl, p.at("u_soil"), i32::from(t.soil));
            p.place(gl, view, t);
            gl::set_ivec4(gl, p.at("u_patches"), patches);
            gl::set_i32(gl, p.at("u_skirt"), 0);
            gl::draw_instanced(gl, vao, per_patch, patches[2] * patches[3]);
            gl::set_i32(gl, p.at("u_skirt"), 1);
            gl::draw_instanced(gl, vao, 6, 4 * 256 / s);
            stats.tiles += 1;
            stats.triangles += (patches[2] * patches[3] * per_patch / 3 + 2 * 4 * 256 / s) as u64;
        }
        // The sea's surface, over the tiles that hold any.
        let p = &self.sea;
        p.program.bind(gl);
        p.view_and_light(gl, view, lighting, haze_levels);
        gl::set_f32(gl, p.at("u_unit"), unit_m);
        gl::set_f32(gl, p.at("u_spacing"), s as f32);
        gl::set_f32(gl, p.at("u_morph"), 0.0);
        gl::set_ivec2(gl, p.at("u_water"), self.water);
        gl::set_vec2(gl, p.at("u_water_light"), self.look.water);
        gl::set_i32(gl, p.at("u_skirt"), 0);
        for &(t, g, patches) in &in_view {
            if !t.sea.iter().any(|&v| v > 0) {
                continue;
            }
            g.heights.bind(gl, unit::HEIGHTS);
            g.sea.bind(gl, unit::SEA);
            p.place(gl, view, t);
            gl::set_f32(gl, p.at("u_level"), -t.base_m);
            gl::set_ivec4(gl, p.at("u_patches"), patches);
            gl::draw_instanced(gl, vao, per_patch, patches[2] * patches[3]);
        }
        // The water lines.
        let p = &self.lines;
        p.program.bind(gl);
        p.view_and_light(gl, view, lighting, haze_levels);
        gl::set_ivec2(gl, p.at("u_water"), self.water);
        gl::set_vec2(gl, p.at("u_water_light"), self.look.water);
        for &(t, g, _) in &in_view {
            let Some(lines) = &g.lines else {
                continue;
            };
            lines.bind(gl, unit::LINES);
            p.place(gl, view, t);
            gl::draw_instanced(gl, vao, 6, t.lines.len() as i32);
            stats.pieces += t.lines.len() as u64;
        }
        stats
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::passes::scene::ArtView;
    use kd_view::{CameraPose, WaterPiece};

    /// A tile at `cell` of rolling ground, sloping 1 in 32 east, with sea over its south-west corner's points, woods
    /// in its ring of cells, and a river's piece and two streams' pieces, one under the least drainage drawn.
    fn tile(cell: CellIx) -> CoarseTile {
        let n = TILE_SIDE;
        let heights: Vec<f32> = (0..n * n)
            .map(|k| {
                let (x, y) = ((k % n) as f32, (k / n) as f32);
                50.0 + x + 3.0 * (y * 0.1).sin()
            })
            .collect();
        let piece = |river: bool, drainage_km2: f32| WaterPiece {
            from: [100.0, 200.0, 60.0],
            to: [130.0, 220.0, 59.0],
            half_width_m: 4.0,
            depth_m: 1.2,
            river,
            drainage_km2,
        };
        CoarseTile {
            cell,
            base_m: 10.0,
            heights,
            surfaces: vec![0; (n - 1) * (n - 1)],
            soil_surface: 0,
            sea: (0..n * n).map(|k| u8::from(k % n < 20 && k / n > 230)).collect(),
            cover: vec![[140, 38, 64, 5, 8]; TILE_RING * TILE_RING],
            cover_surfaces: vec![[4, 4, 0, 5, 1]; TILE_RING * TILE_RING],
            water: vec![piece(true, 20.0), piece(false, 500.0), piece(false, 50.0)],
        }
    }

    fn store() -> CoarseStore {
        CoarseStore::new(&crate::tests::catalogue()).expect("the render tuning")
    }

    // checks: PRN-17 PRE-03 PRE-26
    #[test]
    fn tuned_numbers_from_the_catalogue() {
        // The crowns, the cover's noise and the water's light come from `data/tuning/render.md` (A11.13 rule 6), the
        // probe's own numbers being the catalogue's at α02a; water keeping all its light is refused.
        let mut cat = crate::tests::catalogue();
        let l = CoarseLook::new(&cat).unwrap();
        assert_eq!((l.crown_r, l.cover_spread), ([2.0, 2.8], 1.2));
        assert_eq!(l.cover_oct, [24.0, 1.0 / 24.0, 6.0, 1.0 / 6.0]);
        assert!(l.water[0] == 0.25 && (l.water[1] - 4.746).abs() < 1e-3, "{:?}", l.water);
        let p = crate::probe::PROBE_LOOK;
        assert_eq!(
            (p.crown_r, p.cover_oct, p.cover_spread),
            (l.crown_r, l.cover_oct, l.cover_spread)
        );
        assert!((p.water[1] - l.water[1]).abs() < 0.01 && p.water[0] == l.water[0]);
        let render = cat.tunings.iter_mut().find(|t| t.id == "render").unwrap();
        render.values.iter_mut().find(|v| v.key == "water.keeps").unwrap().value = 1.0;
        let e = CoarseLook::new(&cat).unwrap_err();
        assert!(e.contains("water keeps 1"), "{e}");
    }

    // checks: PRE-03 PRE-26
    #[test]
    fn a_tile_is_laid_out_as_an_area() {
        let mut s = store();
        assert_eq!(s.stream_line_min_km2, 100.0);
        let c = CellIx::at(1000, 240);
        s.insert(tile(c)).expect("a whole tile");
        let t = &s.tiles[0];
        // Slopes per metre, the points being 32 m apart; the sea where the points say; the river's piece and the
        // big stream's drawn, the small stream's not.
        assert!((t.gradients[100 * TILE_SIDE + 100][0] - 1.0 / 32.0).abs() < 1e-6);
        assert_eq!(t.sea[240 * TILE_SIDE + 5], 255);
        assert_eq!(t.sea[100 * TILE_SIDE + 100], 0);
        assert_eq!(t.lines.len(), 2);
        assert_eq!(t.lines[0][3], 4.0);
        // Its cells' cover as trees, bushes, reeds and bare, and the groups' surfaces two to a byte.
        assert_eq!(&t.cells[..4], &[140, 38, 5, 8]);
        assert_eq!(&t.groups[..4], &[4 | 4 << 4, 5 << 4, 1, 0]);
        assert_eq!(t.corner(), Pos { z: 2560, ..c.origin() });
        assert!(s.has(c) && !s.has(CellIx::at(1008, 240)));
        // A tile of the wrong size, or naming a surface beyond the ground's, is refused; dropping keeps the rest.
        let mut bad = tile(CellIx::at(1008, 240));
        bad.heights.pop();
        assert!(s.insert(bad).is_err());
        let mut bad = tile(CellIx::at(1008, 240));
        bad.cover_surfaces[3][2] = 40;
        assert!(s.insert(bad).is_err());
        s.insert(tile(CellIx::at(1008, 240))).unwrap();
        s.retain(&[CellIx::at(1008, 240)]);
        assert!(!s.has(c) && s.has(CellIx::at(1008, 240)));
        assert_eq!(tile_of(CellIx::at(1013, 247)), CellIx::at(1008, 240));
    }

    // checks: PRE-30 PRE-03
    #[test]
    fn fields_come_over_frames_nearest_first() {
        // Two tiles, the light from the south-west: piece by piece with no time to spare, the tile nearest the view
        // gets its sun field first, then the other, then their sky fields, each what the field gives at once.
        let mut s = store();
        let (near, far) = (CellIx::at(1000, 240), CellIx::at(1008, 240));
        s.insert(tile(far)).unwrap();
        s.insert(tile(near)).unwrap();
        let dir = {
            let (e, az) = (20f32.to_radians(), 225f32.to_radians());
            [e.cos() * az.sin(), e.cos() * az.cos(), e.sin()]
        };
        let o = near.origin();
        let at = [f64::from(o.x) / 256.0 + 100.0, f64::from(o.y) / 256.0 + 100.0];
        let get = |s: &CoarseStore, c: CellIx| s.tiles.iter().position(|t| t.cell == c).unwrap();
        let mut order = Vec::new();
        for _ in 0..200_000 {
            if s.follow_light(dir, at, &mut || false) > 0 {
                let state: Vec<(bool, bool)> = [near, far]
                    .iter()
                    .map(|&c| {
                        let t = &s.tiles[get(&s, c)];
                        (t.sun.is_some(), t.sky.is_some())
                    })
                    .collect();
                order.push(state);
            }
            if order.len() == 4 {
                break;
            }
        }
        assert_eq!(
            order,
            vec![
                vec![(true, false), (false, false)],
                vec![(true, false), (true, false)],
                vec![(true, true), (true, false)],
                vec![(true, true), (true, true)],
            ]
        );
        let t = &s.tiles[get(&s, near)];
        assert_eq!(t.sun, Some(field::sun_field(&t.scaled, dir)));
        let at_once: Vec<u8> = field::sky_field(&t.scaled)
            .iter()
            .map(|v| (v.clamp(0.0, 1.0) * 255.0).round() as u8)
            .collect();
        assert_eq!(t.sky.as_ref(), Some(&at_once));
        // Nothing left to do; a small move of the light starts the sun fields again.
        assert_eq!(s.follow_light(dir, at, &mut || true), 0);
        let moved = {
            let (e, az) = (20f32.to_radians(), 225.5f32.to_radians());
            [e.cos() * az.sin(), e.cos() * az.cos(), e.sin()]
        };
        assert_eq!(s.follow_light(moved, at, &mut || true), 2);
    }

    // checks: PRE-03 WLD-01
    #[test]
    fn tiles_follow_the_view() {
        // At the valley stop over (1004, 236), looking straight down: the tile under the target first, each tile once,
        // the view's footprint covered, more with a wider margin; never past the poles' rows.
        let art = ArtView::new(1080, 2404, 4);
        let target = CellIx::at(1004, 236).centre();
        let pose = CameraPose {
            target,
            yaw: -0.45,
            zoom: 0.50,
        };
        let view = View::from_pose(&pose, &art);
        let tiles = tiles_in_view(&view, [-250.0, 600.0], 2_000.0);
        assert_eq!(tiles[0], tile_of(CellIx::of(target)));
        let mut sorted = tiles.clone();
        sorted.sort();
        sorted.dedup();
        assert_eq!(sorted.len(), tiles.len());
        assert!((4..=20).contains(&tiles.len()), "{} tiles", tiles.len());
        assert!(tiles.iter().all(|&t| tile_of(t) == t));
        let wide = tiles_in_view(&view, [-250.0, 600.0], 8_000.0);
        assert!(wide.len() > tiles.len() && tiles.iter().all(|t| wide.contains(t)));
        // Every corner of the art target's ground lies on a tile in the list.
        let b = view.local_footprint(Pos { z: 0, ..target }, [0.0, 0.0]).unwrap();
        for (e, s) in [(b[0], b[2]), (b[0], b[3]), (b[1], b[2]), (b[1], b[3])] {
            let p = Pos {
                x: target.x + (e * 256.0) as i32,
                y: target.y + (s * 256.0) as i32,
                z: 0,
            };
            assert!(tiles.contains(&tile_of(CellIx::of(p))), "{e} {s}");
        }
        let pole = View::from_pose(
            &CameraPose {
                target: CellIx::at(1004, 1).centre(),
                ..pose
            },
            &art,
        );
        assert!(
            tiles_in_view(&pole, [-250.0, 600.0], 2_000.0)
                .iter()
                .all(|t| t.xy().1 < 1_000)
        );
    }
}
