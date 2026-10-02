//! kd-render: the renderer (A11) on OpenGL ES 3.0 and WebGL2 through `glow` (A1.3, A2.5, A2.6), the approved mockup
//! ported to Rust (A11.1): the palette as data (A11.3), the art target, post and upscale passes (A11.2), the golden
//! cube as pixel art (A11.12) and the UI pass (A12.1).
//! Implements PRE-01, PRE-20, PRE-21, PRE-22, PRE-30 and PRE-32 in part, and PRC-11 in part.

pub mod camera;
pub mod crawl;
pub mod cube;
pub mod ground;
pub mod mat;
pub mod palette;
pub mod pass;
pub mod shadow;
pub mod target;
pub mod ui;

use camera::ArtSize;
use crawl::{Base, Capture, CrawlSlot};
use glow::HasContext;
use ground::{GroundGpu, GroundUniforms};
use kd_core::geo;
use kd_data::Catalogue;
use kd_view::{CameraPose, FontAtlas, GroundGrid, Snapshot, UiDrawList};
use mat::Mat4;
use pass::{Passes, PostParams, Prog};
use shadow::{LightCam, ShadowMap};
use std::fmt;

/// The sun's direction in view space for the cube scene (T01a.6).
const SUN: [f32; 3] = [0.4, 0.8, 0.45];
/// The cube's field of view across the shorter side, degrees.
const FOV_DEG: f32 = 40.0;
/// The camera's distance from the cube's centre, cube sides.
const CAMERA_Z: f32 = 3.0;
/// The cube scene's depth range in cube sides: from in front of the cube to behind it.
const DEPTH_NEAR: f32 = -1.0;
const DEPTH_SPAN: f32 = 2.0;
/// Metres a cube side stands for in the post pass's depth thresholds, so the cube reads as a boulder (category rock)
/// whose silhouette is outlined and whose faces are not.
const CUBE_SIDE_M: f32 = 20.0;
/// Light settings of the mockup's dusk (`TOD.dusk`).
const SUN_I: f32 = 1.0;
const AMBIENT: f32 = 0.5;
/// The dither band's width (the mockup's `uBand`).
const BAND: f32 = 0.32;
/// The ground's sun until α03a brings the clock (T01b.5): the mockup's `TOD.dusk`, from its `SUN` direction across
/// the ground (x east, y south) at 21 degrees up, strength 1.0, sky 0.5.
const DUSK_SUN_G: [f32; 2] = [-0.93, 0.37];
const DUSK_SUN_EL_DEG: f32 = 21.0;
const DUSK_HAZE: f32 = 1.0;
/// The ground's patterns (patches, tufts, stones) take positions from the world's corner within blocks of this many
/// metres (`uWorldOff`), so they stay put when the floating origin moves; a block's edge is the only seam.
const WORLD_OFF_M: f64 = 8_192.0;

/// Why the renderer could not start (A3.8): the shader's or linker's info log, or a GPU resource.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum RenderError {
    Shader(String),
    Link(String),
    Resource(String),
}

impl fmt::Display for RenderError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            RenderError::Shader(log) => write!(f, "a shader did not compile: {log}"),
            RenderError::Link(log) => write!(f, "the shaders did not link: {log}"),
            RenderError::Resource(log) => write!(f, "a GPU resource could not be made: {log}"),
        }
    }
}

/// What an art pixel shows, for the post pass's outlines and rims (A11.2): the mockup's `packOut` categories, kept in
/// the low three bits of the art target's green channel, where 0 is the cleared `void` (T01a.5). The shaders name
/// them only through the `#define C_<NAME>` lines of [`Cat::defines`], so the numbers live here.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum Cat {
    Ground = 1,
    Rock = 2,
    Water = 3,
    Plant = 4,
    Figure = 5,
    Thing = 6,
    Effect = 7,
}

impl Cat {
    pub const ALL: [Cat; 7] = [
        Cat::Ground,
        Cat::Rock,
        Cat::Water,
        Cat::Plant,
        Cat::Figure,
        Cat::Thing,
        Cat::Effect,
    ];

    /// The category's name in the shaders.
    pub fn define_name(self) -> &'static str {
        match self {
            Cat::Ground => "C_GROUND",
            Cat::Rock => "C_ROCK",
            Cat::Water => "C_WATER",
            Cat::Plant => "C_PLANT",
            Cat::Figure => "C_FIGURE",
            Cat::Thing => "C_THING",
            Cat::Effect => "C_EFFECT",
        }
    }

    /// The shader lines naming every category: `#define C_GROUND 1.0` to `#define C_EFFECT 7.0`.
    pub fn defines() -> String {
        Cat::ALL
            .iter()
            .map(|&c| format!("#define {} {}.0\n", c.define_name(), c as u8))
            .collect()
    }
}

/// Switches of how a frame is drawn (A11.1).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct DrawSettings {
    pub outlines: bool,
}

impl Default for DrawSettings {
    fn default() -> DrawSettings {
        DrawSettings { outlines: true }
    }
}

/// What one frame shows beyond the snapshot (A11.1): unpaused real seconds, the palette row and the camera now;
/// display time and speed join in α03a.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Frame {
    pub real_s: f64,
    pub palette_row: f32,
    pub set: DrawSettings,
    pub camera: CameraPose,
}

fn dot3(a: [f32; 3], b: [f32; 3]) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}

/// The ground's sun direction in GPU axes (x east, y up, z south): the mockup's `sunDir` for dusk.
pub fn dusk_sun() -> [f32; 3] {
    let [gx, gy] = DUSK_SUN_G;
    let l = (gx * gx + gy * gy).sqrt();
    let el = DUSK_SUN_EL_DEG.to_radians();
    [gx / l * el.cos(), el.sin(), gy / l * el.cos()]
}

/// What a frame cost (A11.11): draw calls now; GPU times per pass join with timer queries.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct FrameStats {
    pub draws: u32,
}

/// What the renderer loads at start besides the catalogue: the font's atlas (A12.1).
#[derive(Clone, Debug, Default)]
pub struct Assets {
    pub font: FontAtlas,
}

struct CubeMesh {
    prog: Prog,
    vao: glow::VertexArray,
    count: i32,
}

/// The renderer: owns the GL context and every GPU resource. Lives on the GL thread (A2.5).
pub struct Renderer {
    gl: glow::Context,
    passes: Passes,
    ui: ui::UiPass,
    cube: CubeMesh,
    ground: GroundGpu,
    shadow: ShadowMap,
    /// Pass 4's fix: `Base` until the owner's review (A11.10).
    slot: Box<dyn CrawlSlot>,
    /// The last frame's ground camera, for the crawl counter.
    last_cam: Option<camera::Camera>,
    pal: glow::Texture,
    ramps: glow::Texture,
    luts: glow::Texture,
    palette_rgba: Vec<u8>,
    w: u32,
    h: u32,
    scale: u32,
}

impl Renderer {
    /// Builds the palette textures from the catalogue, compiles the programs and makes the targets (A11.1).
    #[allow(unsafe_code)]
    pub fn new(gl: glow::Context, cat: &Catalogue, assets: &Assets) -> Result<Renderer, RenderError> {
        let tx = palette::PaletteTextures::build(cat);
        let defines = format!("{}{}", palette::shader_defines(cat), Cat::defines());
        unsafe {
            let pal = target::texture(
                &gl,
                256,
                palette::VERSION_ROWS as u32,
                glow::RGBA8,
                glow::RGBA,
                Some(&tx.palette),
            )?;
            let ramps = target::texture(
                &gl,
                8,
                palette::RAMP_ROWS as u32,
                glow::RGBA8,
                glow::RGBA,
                Some(&tx.ladders),
            )?;
            let luts = target::texture(
                &gl,
                256,
                palette::TABLE_ROWS as u32,
                glow::RGBA8,
                glow::RGBA,
                Some(&tx.tables),
            )?;
            let passes = Passes::new(&gl, &defines)?;
            let ui = ui::UiPass::new(&gl, &assets.font)?;
            let ground = GroundGpu::new(&gl, &defines, cat)?;
            let shadow = ShadowMap::new(&gl)?;
            let prog = Prog::new(
                &gl,
                include_str!("../shaders/cube.vert"),
                include_str!("../shaders/cube.frag"),
                &defines,
                true,
                &[(0, "aPos"), (1, "aNrm")],
            )?;
            let (verts, idx) = cube::mesh();
            let vao = gl.create_vertex_array().map_err(RenderError::Resource)?;
            gl.bind_vertex_array(Some(vao));
            let vbo = gl.create_buffer().map_err(RenderError::Resource)?;
            gl.bind_buffer(glow::ARRAY_BUFFER, Some(vbo));
            let vb: Vec<u8> = verts.iter().flatten().flat_map(|f| f.to_le_bytes()).collect();
            gl.buffer_data_u8_slice(glow::ARRAY_BUFFER, &vb, glow::STATIC_DRAW);
            let ebo = gl.create_buffer().map_err(RenderError::Resource)?;
            gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, Some(ebo));
            let ib: Vec<u8> = idx.iter().flat_map(|i| i.to_le_bytes()).collect();
            gl.buffer_data_u8_slice(glow::ELEMENT_ARRAY_BUFFER, &ib, glow::STATIC_DRAW);
            gl.vertex_attrib_pointer_f32(0, 3, glow::FLOAT, false, 24, 0);
            gl.enable_vertex_attrib_array(0);
            gl.vertex_attrib_pointer_f32(1, 3, glow::FLOAT, false, 24, 12);
            gl.enable_vertex_attrib_array(1);
            gl.bind_vertex_array(None);
            Ok(Renderer {
                gl,
                passes,
                ui,
                cube: CubeMesh {
                    prog,
                    vao,
                    count: idx.len() as i32,
                },
                ground,
                shadow,
                slot: Box::new(Base),
                last_cam: None,
                pal,
                ramps,
                luts,
                palette_rgba: tx.palette,
                w: 1,
                h: 1,
                scale: 4,
            })
        }
    }

    /// `GL_RENDERER | GL_VERSION`, for the self-check (A15.4).
    #[allow(unsafe_code)]
    pub fn gl_info(&self) -> String {
        unsafe {
            format!(
                "{} | {}",
                self.gl.get_parameter_string(glow::RENDERER),
                self.gl.get_parameter_string(glow::VERSION)
            )
        }
    }

    /// The window's size in screen pixels and the screen pixels an art pixel spans (4: `PRE-22`); the art target
    /// follows, so turning the phone keeps the pixel's size.
    #[allow(unsafe_code)]
    pub fn resize(&mut self, w_px: u32, h_px: u32, scale: u32) {
        self.w = w_px.max(1);
        self.h = h_px.max(1);
        self.scale = scale.max(1);
        let (aw, ah) = self.art_size();
        if let Err(e) = unsafe { self.passes.resize(&self.gl, aw, ah) } {
            log::error!(target: "kd::render", "{e}");
        }
    }

    /// Uploads a view area's ground, drawn whenever no golden cube shows (A11.5).
    #[allow(unsafe_code)]
    pub fn set_ground(&mut self, g: &GroundGrid) -> Result<(), RenderError> {
        unsafe { self.ground.upload(&self.gl, g) }
    }

    /// The art target's size now.
    pub fn art_size(&self) -> (u32, u32) {
        target::art_size(self.w, self.h, self.scale)
    }

    /// A palette row's colours as RGBA bytes, 256 of them (the web test hook's `palette only` check).
    pub fn palette_row_rgba(&self, row: usize) -> &[u8] {
        let r = row.min(palette::VERSION_ROWS - 1);
        &self.palette_rgba[r * 256 * 4..(r + 1) * 256 * 4]
    }

    /// Draws one frame: the scene into the art target, post, upscale, then the UI (A11.2's passes 2, 3, 5 and 6).
    #[allow(unsafe_code)]
    pub fn draw(&mut self, f: &Frame, snap: &Snapshot, ui: &UiDrawList) -> FrameStats {
        let (aw, ah) = self.art_size();
        let gl = &self.gl;
        let mut draws = 0;
        // The cube's camera: perspective, the field of view across the shorter side, looking down −z at the cube.
        let aspect = aw as f32 / ah as f32;
        let tan_half = (FOV_DEG.to_radians() / 2.0).tan() / aspect.min(1.0);
        let fovy = 2.0 * tan_half.atan();
        let texel = 2.0 * CAMERA_Z * tan_half / ah as f32 * CUBE_SIDE_M;
        let len = (SUN[0] * SUN[0] + SUN[1] * SUN[1] + SUN[2] * SUN[2]).sqrt();
        let sun = SUN.map(|x| x / len);
        let size = ArtSize {
            wf: aw,
            hf: ah,
            wd: self.w,
            hd: self.h,
            s: self.scale,
        };
        // the ground's camera, and pass 1, its shadows, when no golden cube shows
        let pose = self.slot.quantise(f.camera);
        let ground_cam = (snap.cube.is_none() && self.ground.ready())
            .then(|| camera::compute(&pose, size, self.ground.lo_m, self.ground.hi_m));
        self.last_cam = ground_cam;
        let light = ground_cam.as_ref().and_then(|cam| self.draw_shadow(cam));
        draws += u32::from(light.is_some()) * 16;
        unsafe {
            self.passes.begin_scene(gl);
            if let Some(c) = snap.cube {
                let model = Mat4::rot_x(c.pitch).mul(&Mat4::rot_y(c.yaw));
                let mvp = Mat4::perspective(fovy, aspect, 0.1, 10.0)
                    .mul(&Mat4::translate(0.0, 0.0, -CAMERA_Z))
                    .mul(&model);
                gl.enable(glow::CULL_FACE);
                gl.cull_face(glow::BACK);
                gl.front_face(glow::CCW);
                let g = &self.cube.prog;
                g.bind(gl);
                g.f(gl, "uMVP", &mvp.0);
                g.f(gl, "uModel", &model.0);
                g.tex(gl, "uRamps", 0, Some(self.ramps));
                g.tex(gl, "uLuts", 1, Some(self.luts));
                g.f(gl, "uDith", &[0.0, 0.0]);
                g.f(gl, "uBand", &[BAND]);
                g.f(gl, "uScreenDither", &[0.0]);
                g.f(gl, "uTexel", &[texel]);
                g.f(gl, "uSunDir", &sun);
                g.f(gl, "uSunI", &[SUN_I]);
                g.f(gl, "uAmb", &[AMBIENT]);
                g.f(gl, "uCamF", &[0.0, 0.0, -1.0]);
                g.f(gl, "uDepthR", &[DEPTH_NEAR, 1.0 / DEPTH_SPAN]);
                g.f(gl, "uHaze", &[0.0, 0.0, 0.0]);
                g.f(gl, "uDbgA", &[0.0]);
                gl.bind_vertex_array(Some(self.cube.vao));
                gl.draw_elements(glow::TRIANGLES, self.cube.count, glow::UNSIGNED_SHORT, 0);
                gl.bind_vertex_array(None);
                draws += 1;
            }
            let sl = (sun[0] * sun[0] + sun[1] * sun[1]).sqrt().max(1e-6);
            let mut post = PostParams {
                sun_scr: [sun[0] / sl, sun[1] / sl],
                fire_scr: [-1.0e5, -1.0e5, 0.0],
                outlines: f.set.outlines,
                palette_row: f.palette_row,
                depth_m: DEPTH_SPAN * CUBE_SIDE_M,
                texel,
            };
            // the scene's grid moves with the camera's sub-pixel shift, and the UI stays on it (A12.1: a UI pixel is
            // an art pixel)
            let mut off = pass::offset((aw, ah), self.w, self.h, self.scale);
            if let Some(cam) = &ground_cam {
                draws += self.draw_ground(cam, light.as_ref(), aw, ah);
                let sun = dusk_sun();
                let ss = [dot3(sun, cam.r), dot3(sun, cam.u)];
                let sl = (ss[0] * ss[0] + ss[1] * ss[1]).sqrt().max(1e-6);
                post.sun_scr = [ss[0] / sl, ss[1] / sl];
                post.depth_m = cam.far - cam.near;
                post.texel = cam.texel;
                off = cam.off;
            }
            let gl = &self.gl;
            self.passes.post(gl, self.pal, self.luts, &post);
            let p = &self.passes;
            self.slot
                .resolve(gl, &p.post, &p.post, &p.crawl, (f.real_s * 1000.0) as f32);
            self.passes.upscale(gl, (self.w, self.h), off, self.scale);
            draws += 3;
            let grid = ui::Grid {
                win: (self.w, self.h),
                off,
                s: self.scale,
            };
            draws += self.ui.draw(gl, ui, self.pal, f.palette_row, grid);
        }
        FrameStats { draws }
    }

    /// Pass 1 for the ground: while the sun is up and art pixels are under 3.2 m, its chunks into the shadow map from
    /// the sun's camera fitted round the view (A11.2). Returns that camera when drawn.
    #[allow(unsafe_code)]
    fn draw_shadow(&self, cam: &camera::Camera) -> Option<LightCam> {
        let sun = dusk_sun();
        if cam.texel >= shadow::SHADOW_MAX_TEXEL || sun[1] <= shadow::SHADOW_MIN_SUN_UP {
            return None;
        }
        let light = shadow::light_for(&cam.foot, &cam.foot_block, sun, cam.origin_m());
        let gl = &self.gl;
        let d = geo::delta(cam.origin, self.ground.origin);
        unsafe {
            self.shadow.begin(gl, &light);
            gl.disable(glow::CULL_FACE);
            let g = &self.ground.shadow_prog;
            g.bind(gl);
            let u = GroundUniforms {
                vp: &light.vp.0,
                area_off: [d.x, 0.0, d.y],
                step: ground::step_for(cam.texel),
            };
            self.ground.draw(gl, g, &u);
        }
        Some(light)
    }

    /// Pass 2 for the ground: its chunks into the art target at the spacing for the camera's art pixels, with the
    /// mockup's light, shadow, dither and haze uniforms. Returns the draw calls.
    #[allow(unsafe_code)]
    fn draw_ground(&self, cam: &camera::Camera, light: Option<&LightCam>, aw: u32, ah: u32) -> u32 {
        let gl = &self.gl;
        let sun = dusk_sun();
        let d = geo::delta(cam.origin, self.ground.origin);
        // haze toward the far part of the view (the mockup's `uHaze`)
        let span = (ah as f32 * cam.texel / 2.0) / cam.pitch.tan().max(0.05);
        let haze = [
            dot3(cam.f, cam.tg) + 0.15 * span,
            1.0 / (1.6 * span).max(1.0),
            3.0 * DUSK_HAZE * 0.8,
        ];
        unsafe {
            gl.disable(glow::CULL_FACE);
            let g = &self.ground.prog;
            g.bind(gl);
            g.tex(gl, "uRamps", 0, Some(self.ramps));
            g.tex(gl, "uLuts", 1, Some(self.luts));
            g.f(gl, "uDith", &[cam.dith[0] as f32, cam.dith[1] as f32]);
            g.f(gl, "uBand", &[BAND]);
            g.f(gl, "uScreenDither", &[0.0]);
            g.f(gl, "uTexel", &[cam.texel]);
            g.f(gl, "uSunDir", &sun);
            g.f(gl, "uSunI", &[SUN_I]);
            g.f(gl, "uAmb", &[AMBIENT]);
            g.tex(gl, "uShadow", 2, Some(self.shadow.tex));
            match light {
                Some(l) => {
                    g.f(gl, "uShadowOn", &[1.0]);
                    g.f(gl, "uLightVP", &l.vp.0);
                    g.f(gl, "uShadowBias", &l.bias());
                    g.f(gl, "uShadowWin", &l.window());
                }
                None => g.f(gl, "uShadowOn", &[0.0]),
            }
            // the patterns are fixed to the world: the origin's place within 8,192 m blocks of it
            let o = cam.origin_m();
            g.f(
                gl,
                "uWorldOff",
                &[(o[0] % WORLD_OFF_M) as f32, (o[2] % WORLD_OFF_M) as f32],
            );
            g.f(gl, "uCamF", &cam.f);
            g.f(gl, "uDepthR", &[cam.near, 1.0 / (cam.far - cam.near)]);
            g.f(gl, "uHaze", &haze);
            g.f(gl, "uFire", &[0.0, 0.0, 0.0, 0.0]);
            g.f(gl, "uFire2", &[0.0, 0.0, 0.0, 0.0]);
            g.f(gl, "uView", &cam.view.map(|v| v as f32));
            g.f(gl, "uDbgA", &[0.0]);
            g.f(gl, "uDbgB", &[0.0]);
            let u = GroundUniforms {
                vp: &cam.vp.0,
                area_off: [d.x, 0.0, d.y],
                step: ground::step_for(cam.texel),
            };
            // the projection spans the view's block; the viewport puts the view on the art target
            let [x, y, w, h] = cam.view;
            gl.viewport(x, y, w, h);
            let n = self.ground.draw(gl, g, &u);
            gl.viewport(0, 0, aw as i32, ah as i32);
            n
        }
    }

    /// The last frame for the crawl counter (B66's `capture`): the post target's colours and the scene's view depth,
    /// with its ground camera; `None` when the last frame drew no ground.
    #[allow(unsafe_code)]
    pub fn capture(&self) -> Option<Capture> {
        let cam = self.last_cam?;
        let gl = &self.gl;
        let (w, h) = (self.passes.post.w, self.passes.post.h);
        let n = (w * h) as usize;
        let (mut col, mut scene) = (vec![0u8; n * 4], vec![0u8; n * 4]);
        unsafe {
            for (t, buf) in [(&self.passes.post, &mut col), (&self.passes.scene, &mut scene)] {
                t.bind(gl);
                gl.read_pixels(
                    0,
                    0,
                    w as i32,
                    h as i32,
                    glow::RGBA,
                    glow::UNSIGNED_BYTE,
                    glow::PixelPackData::Slice(Some(buf)),
                );
            }
            gl.bind_framebuffer(glow::FRAMEBUFFER, None);
        }
        Some(Capture {
            w,
            h,
            col: col
                .chunks(4)
                .map(|c| u32::from_le_bytes([c[0], c[1], c[2], c[3]]))
                .collect(),
            dep16: scene
                .chunks(4)
                .map(|c| u16::from(c[2]) << 8 | u16::from(c[3]))
                .collect(),
            cam,
        })
    }

    /// The GL error flag (`glGetError`), 0 when none.
    #[allow(unsafe_code)]
    pub fn gl_error(&self) -> u32 {
        unsafe { self.gl.get_error() }
    }
}

#[cfg(test)]
mod tests {
    use super::Cat;

    // checks: PRE-21
    #[test]
    fn categories_match_pack_out() {
        // The mockup's packOut numbers, 1 to 7 (0 is void), each with its define.
        let defs = Cat::defines();
        for (k, &c) in Cat::ALL.iter().enumerate() {
            assert_eq!(c as usize, k + 1, "{c:?}");
            let line = format!("#define {} {}.0\n", c.define_name(), k + 1);
            assert!(defs.contains(&line), "{defs}");
        }
        // The shaders pack and compare categories by name only, never by a bare number.
        let cube = include_str!("../shaders/cube.frag");
        assert!(cube.contains("packOut(idx, C_ROCK,"), "the cube packs as rock");
        for (file, src) in [("cube.frag", cube), ("post.frag", include_str!("../shaders/post.frag"))] {
            for op in ["cat == ", "cat != ", "cat > ", "cat < "] {
                for (i, _) in src.match_indices(op) {
                    let next = src[i + op.len()..].chars().next();
                    assert_eq!(next, Some('C'), "{file}: a bare category number after `{op}`");
                }
            }
        }
    }
}
