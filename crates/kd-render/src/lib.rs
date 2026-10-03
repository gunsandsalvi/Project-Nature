//! kd-render: the renderer (A11), organised as A11.13 sets it: `gl` is the one layer that calls OpenGL, `passes`
//! holds each pass with its program and targets, and every shader shares `shaders/lib.glsl` and the constants
//! generated from Rust. It runs on the GL thread only, through `glow`: OpenGL ES 3.0 on the phone, WebGL2 on the web.
//! α01a draws the light card as palette indices, turns them into the palette's colours under the light of the hour,
//! and enlarges the result so an art pixel is 4 × 4 screen pixels (A11.2, A11.3); α01b draws the loaded areas'
//! ground through the camera instead, keeping the card for its golden scenes; α02a coarse ground's tiles beside
//! them, with the sea and the water lines (A11.5, A11.6).
//!
//! Implements PRE-22 and PLT-01, see A11.2 and A11.13: an art pixel exactly 4 × 4 screen pixels, drawn with OpenGL
//! ES 3.0 on the phone.

pub mod camera;
pub mod field;
pub mod frame;
pub mod gl;
pub mod ground;
pub mod light;
pub mod looks;
pub mod passes;
pub mod pixel;
pub mod probe;
pub mod shaders;

use std::fmt;

use kd_core::geo::CellIx;
use kd_data::Catalogue;
use kd_view::{AreaMeshes, CameraPose, CoarseTile};
use passes::crawl::{Base, CrawlSlot, Resolved};
use passes::post::{PostFrame, PostPass};
use passes::scene::{ArtView, ScenePass};
use passes::ui::UiPass;
use passes::upscale::UpscalePass;
use probe::ProbePass;

use camera::View;
use frame::Lighting;
use gl::{Format, Texture};
use ground::coarse::{CoarsePass, CoarseStore, TILE_M};
use ground::{GroundPass, Store};
use looks::{Layout, PALETTE_SIZE, Palette, TABLE_ROWS};

/// Screen pixels an art pixel on the phone, device pixels on the web (A11.2, `PRE-22`).
pub const ART_SCALE: u32 = 4;

#[derive(Debug)]
pub enum RenderError {
    Shader { name: String, log: String },
    Link { name: String, log: String },
    Target(String),
    Gl(String),
}

impl fmt::Display for RenderError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            RenderError::Shader { name, log } => write!(f, "shader {name} did not compile: {}", clean(log)),
            RenderError::Link { name, log } => write!(f, "program {name} did not link: {}", clean(log)),
            RenderError::Target(s) => write!(f, "render target: {s}"),
            RenderError::Gl(s) => write!(f, "GL: {s}"),
        }
    }
}

/// A driver's log without the NULs and blank ends some drivers add.
fn clean(log: &str) -> &str {
    log.trim_matches(|c: char| c == '\0' || c.is_whitespace())
}

/// One frame's inputs (A11.1); the display time joins with its alpha.
#[derive(Clone, Copy, Debug, Default)]
pub struct Frame {
    /// Frames drawn since the app started.
    pub count: u64,
    /// The sky the frame is lit by (A11.4).
    pub sky: kd_view::SkyView,
    /// Where the camera looks (A11.2).
    pub cam: CameraPose,
    /// The light card instead of the ground, for its golden scenes (A11.12).
    pub card: Option<passes::scene::card::CardFrame>,
    /// How long the frame may spend on sun fields worked out over frames, in nanoseconds by the renderer's clock
    /// (A11.11); none, or no clock, finishes each at once, as the golden scenes and test hooks need.
    pub field_ns: Option<u64>,
}

/// What a frame drew.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct FrameStats {
    pub art: [u32; 2],
    /// The ground's triangles, coarse ground's with them, and the stones and tufts handed to the GPU (A11.11).
    pub triangles: u64,
    pub items: u64,
    /// Coarse ground's tiles drawn, and its water lines' pieces.
    pub tiles: u64,
    pub pieces: u64,
}

/// The heights the view's footprint is taken between when choosing coarse tiles, metres: the deepest sea floor to
/// above the highest land.
pub const TILE_SPAN_M: [f32; 2] = [-250.0, 600.0];

pub struct Renderer {
    gl: glow::Context,
    info: String,
    vao: glow::VertexArray,
    scene: ScenePass,
    post: PostPass,
    upscale: UpscalePass,
    ui: UiPass,
    probe: ProbePass,
    /// The crawl fix (A11.10): `Base` until the first visual review.
    crawl: Box<dyn CrawlSlot>,
    ground: GroundPass,
    store: Store,
    coarse: CoarsePass,
    tiles: CoarseStore,
    view: Option<ArtView>,
    /// The last frame's camera view, for the test hooks.
    last_view: Option<View>,
    cat: Catalogue,
    layout: Layout,
    /// The light and palette of the last frame (A11.13 rule 4), none before the first.
    lighting: Option<Lighting>,
    palette_tex: Texture,
    tables_tex: Texture,
    /// The app's monotonic clock in nanoseconds, timing the frame's share of field work (A11.11).
    clock: Option<Box<dyn Fn() -> u64>>,
    /// The longest a frame's field work has taken, in nanoseconds by that clock, for the bench (A15.10).
    longest_field_ns: u64,
}

impl Renderer {
    /// Builds every program, the shared vertex array, the palette's textures from the catalogue and the font's from
    /// the assets; a shader that fails names itself and the driver's log, which the self-check reports (A15.4).
    pub fn new(gl: glow::Context, cat: &Catalogue, assets: &kd_view::Assets) -> Result<Renderer, RenderError> {
        let info = gl::info(&gl);
        let layout = Layout::new(cat).map_err(RenderError::Gl)?;
        let vao = gl::empty_vertex_array(&gl)?;
        let scene = ScenePass::new(&gl)?;
        let post = PostPass::new(&gl)?;
        let upscale = UpscalePass::new(&gl)?;
        let ui = UiPass::new(&gl, &assets.font)?;
        let probe = ProbePass::new(&gl)?;
        let ground = GroundPass::new(&gl, cat, &layout)?;
        let store = Store {
            cover: ground::cover::CoverTable::new(cat, &layout).map_err(RenderError::Gl)?,
            ..Store::default()
        };
        let coarse = CoarsePass::new(&gl, cat, &layout)?;
        let tiles = CoarseStore::new(cat).map_err(RenderError::Gl)?;
        let palette_tex = Texture::new(&gl, Format::Rgba8, PALETTE_SIZE as u32, 1, None)?;
        let tables_tex = Texture::new(&gl, Format::R8, PALETTE_SIZE as u32, TABLE_ROWS as u32, None)?;
        Ok(Renderer {
            gl,
            info,
            vao,
            scene,
            post,
            upscale,
            ui,
            probe,
            crawl: Box::new(Base),
            ground,
            store,
            coarse,
            tiles,
            view: None,
            last_view: None,
            cat: cat.clone(),
            layout,
            lighting: None,
            palette_tex,
            tables_tex,
            clock: None,
            longest_field_ns: 0,
        })
    }

    /// The longest a frame's sun field work has taken, in nanoseconds, for the bench (A15.10): a field made at once
    /// for a new area or a jump of the light, or a frame's share of one worked over frames.
    pub fn longest_field_ns(&self) -> u64 {
        self.longest_field_ns
    }

    /// Gives the renderer the app's monotonic clock, in nanoseconds, so a frame can stop its field work when its
    /// share is spent (A11.11).
    pub fn set_clock(&mut self, clock: Box<dyn Fn() -> u64>) {
        self.clock = Some(clock);
    }

    /// The frame's light and palette, the haze seen along `view`, uploading the palette's textures when its row or
    /// tables changed (A11.3, A11.4).
    fn light_frame(&mut self, f: &Frame, view: [f32; 3]) {
        let upload = match &mut self.lighting {
            Some(l) => l.follow(&self.cat, &self.layout, &f.sky, view),
            None => {
                self.lighting = Some(Lighting::new(&self.cat, &self.layout, &f.sky, view));
                true
            }
        };
        if let (true, Some(l)) = (upload, &self.lighting) {
            for (tex, bytes) in [
                (&self.palette_tex, l.palette.texture_bytes()),
                (&self.tables_tex, l.palette.table_bytes()),
            ] {
                if let Err(e) = tex.update(&self.gl, &bytes) {
                    log::error!(target: "kd::render", "palette upload: {e}");
                }
            }
        }
    }

    /// The palette in use, for the test hooks (A11.13's `probe`).
    pub fn palette(&self) -> Option<&Palette> {
        self.lighting.as_ref().map(|l| &l.palette)
    }

    /// `GL_RENDERER | GL_VERSION`.
    pub fn gl_info(&self) -> &str {
        &self.info
    }

    /// A new window size: the art target follows, an art pixel staying `scale` screen pixels (`PRE-22`).
    pub fn resize(&mut self, w_px: u32, h_px: u32, scale: u32) -> Result<(), RenderError> {
        if w_px == 0 || h_px == 0 {
            self.view = None;
            return Ok(());
        }
        let view = ArtView::new(w_px, h_px, scale);
        self.scene.resize(&self.gl, &view)?;
        self.post.resize(&self.gl, &view)?;
        self.view = Some(view);
        Ok(())
    }

    /// Takes an area's ground for drawing (A11.1, A11.5): kept on the CPU, its textures made at the next frame. A
    /// malformed area is logged and left out.
    pub fn upload_area(&mut self, m: AreaMeshes) {
        if let Err(e) = self.store.insert(m) {
            log::error!(target: "kd::render", "upload_area: {e}");
        }
    }

    /// Makes the textures of areas taken since the last frame (A11.5): the app times it for the bench; `draw` does
    /// it too, so a frame never draws an area without them. Returns how many areas were uploaded.
    pub fn upload_areas(&mut self) -> usize {
        let before = self.store.uploaded();
        if let Err(e) = self.store.upload(&self.gl) {
            log::error!(target: "kd::render", "area textures: {e}");
        }
        self.store.uploaded() - before
    }

    /// Takes a tile of coarse ground for drawing (A11.5), its textures made at the next frame and its light fields
    /// over the frames after; a malformed tile is logged and left out.
    pub fn upload_tile(&mut self, t: CoarseTile) {
        if let Err(e) = self.tiles.insert(t) {
            log::error!(target: "kd::render", "upload_tile: {e}");
        }
    }

    /// Whether a tile of coarse ground is loaded.
    pub fn has_tile(&self, cell: CellIx) -> bool {
        self.tiles.has(cell)
    }

    /// The tiles of coarse ground a camera pose's view needs, within `margin_m` of what it shows, nearest its target
    /// first (A11.5); none before the window's size is known.
    pub fn tiles_wanted(&self, cam: &CameraPose, margin_m: f64) -> Vec<CellIx> {
        let Some(art) = self.view else {
            return Vec::new();
        };
        ground::coarse::tiles_in_view(&View::from_pose(cam, &art), TILE_SPAN_M, margin_m)
    }

    /// Drops every tile of coarse ground but those `keep` names.
    pub fn retain_tiles(&mut self, keep: &[CellIx]) {
        self.tiles.retain(keep);
    }

    /// The view of a camera pose on the current art target (A11.2), its depth fitted to the loaded ground.
    pub fn view_of(&self, cam: &CameraPose) -> Option<View> {
        let art = self.view?;
        let mut v = View::from_pose(cam, &art);
        let areas = self.store.areas.iter().map(|a| (a.corner(), a.span_m, 256.0));
        let tiles = self
            .tiles
            .tiles
            .iter()
            .map(|t| (t.corner(), t.span_m, f64::from(TILE_M)));
        v.fit_depth(areas.chain(tiles));
        Some(v)
    }

    /// Draws a frame: the scene, post, the upscale, then the UI's list over them (A11.2's order).
    pub fn draw(&mut self, f: &Frame, ui: &kd_view::UiDrawList) -> FrameStats {
        let Some(art) = self.view else {
            return FrameStats::default();
        };
        // The ground's view, through the crawl fix's snapping, or the light card's own: north, the card's pitch
        // below the horizon.
        let cam = self.crawl.quantise(f.cam);
        let view = if f.card.is_some() { None } else { self.view_of(&cam) };
        let (sp, cp) = (
            passes::scene::card::PITCH_DEG.to_radians().sin(),
            passes::scene::card::PITCH_DEG.to_radians().cos(),
        );
        let basis = match &view {
            Some(v) => v.basis_f32(),
            None => camera::Basis {
                right: [1.0, 0.0, 0.0],
                up: [0.0, sp, cp],
                fwd: [0.0, cp, -sp],
            },
        };
        self.light_frame(f, basis.fwd);
        let mut stats = FrameStats {
            art: art.art,
            ..FrameStats::default()
        };
        let mut off = art.off;
        let mut depth_m = passes::scene::card::DEPTH_M;
        if let Some(l) = &self.lighting {
            // The frame's share of field work, timed from here.
            let clock = self.clock.as_deref();
            let until = f.field_ns.zip(clock).map(|(ns, now)| now() + ns);
            let mut more = || until.zip(clock).is_none_or(|(end, now)| now() < end);
            let t0 = clock.map(|now| now());
            self.store.follow_light(l.light.dir, &mut more);
            let near = [f64::from(cam.target.x) / 256.0, f64::from(cam.target.y) / 256.0];
            self.tiles.follow_light(l.light.dir, near, &mut more);
            if let (Some(t0), Some(now)) = (t0, clock) {
                self.longest_field_ns = self.longest_field_ns.max(now().saturating_sub(t0));
            }
        }
        if let Err(e) = self.store.upload(&self.gl) {
            log::error!(target: "kd::render", "area textures: {e}");
        }
        if let Err(e) = self.tiles.upload(&self.gl) {
            log::error!(target: "kd::render", "tile textures: {e}");
        }
        if let Some(lighting) = &self.lighting {
            match (&f.card, view) {
                (Some(card), _) => self.scene.draw(&self.gl, &art, &self.layout, lighting, card, self.vao),
                (None, Some(view)) => {
                    if let Some(t) = &self.scene.target {
                        t.bind(&self.gl);
                        gl::clear(&self.gl, [0.0; 4], 1.0);
                        let g = self.ground.draw(&self.gl, &view, &self.store, lighting, self.vao);
                        let c = self.coarse.draw(
                            &self.gl,
                            &view,
                            &self.tiles,
                            lighting,
                            self.cat.air.haze_levels,
                            self.vao,
                        );
                        (stats.triangles, stats.items) = (g.triangles + c.triangles, g.items);
                        (stats.tiles, stats.pieces) = (c.tiles, c.pieces);
                        off = view.off;
                        depth_m = (view.depth[1] - view.depth[0]) as f32;
                        self.last_view = Some(view);
                    }
                }
                (None, None) => {}
            }
        }
        let sun = self.lighting.as_ref().map_or([0.0; 3], |l| l.light.dir);
        let dot3 = |a: [f32; 3], b: [f32; 3]| a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
        let frame = PostFrame {
            depth_m,
            sun_screen: [dot3(sun, basis.right), dot3(sun, basis.up)],
        };
        if let (Some(scene), Some(post)) = (&self.scene.target, &self.post.target) {
            self.post.draw(
                &self.gl,
                &scene.colours[0],
                &self.palette_tex,
                &self.tables_tex,
                frame,
                self.vao,
            );
            match self.crawl.resolve() {
                Resolved::Post => self.upscale.draw(&self.gl, &art, off, &post.colours[0], self.vao),
            }
        }
        self.ui.draw(&self.gl, &art, ui, &self.palette_tex);
        stats
    }

    /// The last frame's camera view, if it drew the ground.
    pub fn last_view(&self) -> Option<&View> {
        self.last_view.as_ref()
    }

    /// The last frame of the ground as the crawl counter sees it (A11.10): post's colours and colour 0's depths,
    /// read back; none before the ground has drawn.
    pub fn capture(&self) -> Option<probe::Capture> {
        let view = self.last_view?;
        let colours = self.post.target.as_ref()?.read_rgba8(&self.gl);
        let scene = self.scene.target.as_ref()?.read_rgba8(&self.gl);
        let span = view.depth[1] - view.depth[0];
        Some(probe::Capture {
            view,
            colours: colours.chunks(4).map(|c| [c[0], c[1], c[2]]).collect(),
            depths: scene
                .chunks(4)
                .map(|c| {
                    let d = f64::from(u16::from(c[2]) << 8 | u16::from(c[3])) / 65535.0;
                    (c[1] & 7 != pixel::Cat::Void as u8).then_some(view.depth[0] + d * span)
                })
                .collect(),
        })
    }

    /// Runs the probe scene (A11.13 rule 2): the steps the GPU picks for the fixed inputs, and the twins' steps.
    pub fn probe(&mut self) -> (Vec<u8>, Vec<u8>) {
        self.probe.run(&self.gl, self.vao)
    }

    /// The art target's size, once the window's is known.
    pub fn art_size(&self) -> Option<[u32; 2]> {
        self.view.map(|v| v.art)
    }

    /// The first GL error since the last call, or 0.
    pub fn gl_error(&self) -> u32 {
        gl::error(&self.gl)
    }
}

#[cfg(test)]
pub(crate) mod tests {
    use kd_data::Catalogue;
    use kd_data::compile::compile;

    /// The repository's catalogue, compiled as the app's build script compiles it.
    pub(crate) fn catalogue() -> Catalogue {
        compile(&kd_data::repo_sources!(), false)
            .expect("the catalogue compiles")
            .catalogue
    }
}
