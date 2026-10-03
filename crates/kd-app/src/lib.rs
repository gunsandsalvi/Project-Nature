//! kd-app: the app object: the frame loop, the simulation, I/O and view-builder threads, snapshot hand-off, input
//! and settings (A2.2, A2.4, A4). It lives on the GL thread; both shells drive it the same way.
//! It draws the renderer's test card each frame, keeps a panicking frame from taking the app down (A3.8), and
//! runs the self-check (A15.4): the shaders, the GL version, and from α00b the core's bits against the cloud's.
//!
//! Implements PRC-11 and RES-05, see A15.4 and A15.9: the self-check on a build's first start, the core's bits
//! among its checks.

pub mod camera;
pub mod ground;
pub mod json;
pub mod selfcheck;

use std::panic::{self, AssertUnwindSafe};
use std::sync::Arc;

use kd_data::Catalogue;
use kd_render::passes::scene::card;
use kd_render::{ART_SCALE, Frame, Renderer};
use kd_ui::strip::Colours;
use kd_ui::{Ui, UiAction};
use kd_view::{AreaMeshes, CameraPose, FontAtlas, InputEvent, Insets, UiDrawList};
use kd_world::area::{Ground, Material, demo};
use kd_world::cells::CellCtx;
use kd_world::lands::{self, PresetWorld};

pub use json::{counts_json, requests_json};

/// What the app needs from the shell it runs in (A2.2); storage and the CPU layout join with their alphas.
pub trait Platform: Send + Sync {
    /// A monotonic clock: `CLOCK_MONOTONIC` on the phone, `performance.now()` on the web.
    fn now_ns(&self) -> u64;
    /// Puts a request in the shell's outbox, which the shell drains each frame.
    fn post(&self, r: Request);
}

/// Requests to the shell (A2.2); α00 has the code dialog, and the others join with their alphas.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Request {
    /// The code dialog: the shell gzips `json`, writes it in base64 after `prefix` and shows it with Copy, which
    /// works before any UI exists and keeps a compression crate out of Rust (A15.4).
    ShowCode {
        title: String,
        prefix: String,
        json: String,
    },
}

/// Messages from the shell's UI thread, delivered on the GL thread in order (A2.2, A2.5).
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum AppMsg {
    Input(InputEvent),
    Insets(Insets),
    Pause,
    Resume,
    Back,
}

/// The eight hours the strip steps through until the clock runs them (α03a), on the spring equinox at the camera's
/// latitude (A11.4): their names and their hours.
pub const HOURS: [(&str, f32); 8] = [
    ("dawn", 6.5),
    ("morning", 9.0),
    ("noon", 12.0),
    ("afternoon", 15.0),
    ("late afternoon", 16.5),
    ("dusk", 17.75),
    ("twilight", 18.5),
    ("night", 23.0),
];
/// The demo area's latitude, which its golden scenes are lit at.
pub const HOURS_LAT: f32 = 21.0;
/// The land the app shows, its seed, and how its tiles of coarse ground are fed to the renderer: those within
/// `TILE_WANT_M` of the view made, nearest first, for up to `TILE_FRAME_NS` of a frame; those beyond `TILE_KEEP_M`
/// dropped (A11.5).
pub const FIRST_LAND: &str = "first_region";
pub const ISLAND_SEED: u64 = 1;
pub const TILE_WANT_M: f64 = 2_000.0;
pub const TILE_KEEP_M: f64 = 8_000.0;
pub const TILE_FRAME_NS: u64 = 4_000_000;
/// Late afternoon, the hour the app opens on.
pub const FIRST_HOUR: usize = 4;

/// A slow camera motion for the crawl counter (A11.10): a turn, its rate in degrees a frame; a zoom, as a share of
/// the art pixel a frame; or a pan along the screen's right, in art pixels a frame.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Motion {
    Turn,
    Zoom,
    Pan,
}

impl Motion {
    pub fn named(name: &str) -> Option<Motion> {
        match name {
            "turn" => Some(Motion::Turn),
            "zoom" => Some(Motion::Zoom),
            "pan" => Some(Motion::Pan),
            _ => None,
        }
    }
}

/// The sky at one of `HOURS` at latitude `lat`: spring day 1, the equinox, at that hour (A3.7).
pub fn sky_at(hour: usize, lat: f32) -> kd_view::SkyView {
    let (_, h) = HOURS[hour % HOURS.len()];
    let t = kd_core::time::GameTime((h * 3600.0) as u64);
    kd_render::light::sky_view(&kd_core::sky::sun_moon(t, lat, 0.0, &kd_core::sky::Sky::FIRST))
}

/// The latitude of a place, in whole degrees north, which its light is seen from until the clock comes (α03a).
pub fn latitude(p: kd_core::geo::Pos) -> f32 {
    let y = (f64::from(p.y) + 0.5) / f64::from(kd_core::geo::H);
    (90.0 - 180.0 * y).round() as f32
}

/// What the shell knows at start.
#[derive(Clone, Debug, Default)]
pub struct AppConfig {
    /// The device, for the self-check's report: maker, model and Android version, or the browser.
    pub device: String,
}

/// The catalogue blob, compiled from `data/` by the build script (A3.6).
pub static CATALOGUE: &[u8] = include_bytes!(concat!(env!("OUT_DIR"), "/catalogue.kdcat"));

/// The build's version line, set by the build scripts (`KD_BUILD`), such as `a00 · 1000 · 4f2c9e1`.
pub fn build_line() -> &'static str {
    option_env!("KD_BUILD").unwrap_or("dev")
}

/// The strip's version line (`PRE-32`): the alpha and its version code from the build line, then the catalogue's
/// rules version and the start of its hash, such as `a01a · 1011 · catalogue 1.0 a11e44f9`.
pub fn version_line(build: &str, cat: &Catalogue, hash: u64) -> String {
    let alpha: Vec<&str> = build.split(" · ").take(2).collect();
    format!(
        "{} · catalogue {}.{} {:08x}",
        alpha.join(" · "),
        cat.versions.major,
        cat.versions.minor,
        hash >> 32
    )
}

/// The strip's words for one of `HOURS`: its name and its time, such as `Late afternoon · 16:30`.
pub fn hour_line(hour: usize) -> String {
    let (name, h) = HOURS[hour % HOURS.len()];
    let mut c = name.chars();
    let first: String = c.next().map(|f| f.to_uppercase().collect()).unwrap_or_default();
    let minutes = (h * 60.0) as u32;
    format!("{first}{} · {:02}:{:02}", c.as_str(), minutes / 60, minutes % 60)
}

pub struct App {
    platform: Arc<dyn Platform>,
    cfg: AppConfig,
    renderer: Option<Renderer>,
    size: Option<[u32; 2]>,
    insets: Insets,
    paused: bool,
    frames: u64,
    panics_in_row: u32,
    report: selfcheck::Report,
    report_sent: bool,
    /// Whether the core's maths and draws give the cloud's bits here (A15.9 item 5).
    core_bits: bool,
    /// The catalogue, loaded at start; none if its blob failed its checks, which the self-check reports.
    catalogue: Option<Catalogue>,
    /// How long loading the catalogue took, in nanoseconds (A3.6: under 10 ms on the phone).
    catalogue_ns: u64,
    /// Which of `HOURS` lights the frame.
    hour: usize,
    /// The UI's state, its font and its fixed colours (A12.1).
    ui: Ui,
    font: FontAtlas,
    colours: Colours,
    /// The strip's version line, made once the catalogue is loaded.
    version: String,
    /// When the first frame was drawn, which the block's turn counts from.
    start_ns: Option<u64>,
    /// Test hook (A11.12): a golden scene with time frozen, which hides the strip.
    golden: Option<Golden>,
    /// The demo area (α01b): its ground, as the world made it and as the renderer takes it, kept so a new GL
    /// context gets it again (A11.13 rule 5).
    demo: Option<Ground>,
    demo_meshes: Option<AreaMeshes>,
    /// How long making the demo area took, in nanoseconds (the bench, A15.10).
    demo_ns: u64,
    /// The first region (A5.6), how long building it took, in nanoseconds, its highest ground, and the surface each
    /// material shows as.
    island: Option<PresetWorld>,
    island_ns: u64,
    island_top_m: f64,
    surfaces: Option<[u8; Material::ALL.len()]>,
    /// The longest a frame spent making tiles of coarse ground, in nanoseconds, and how many it has made (A15.10).
    tile_ns: u64,
    tiles_made: u64,
    /// Test hook (A12.4): the next frame makes every tile it needs and finishes every light field.
    settle_next: bool,
    /// Where the camera looks, the gesture moving it and its easing (A11.2, A12.2).
    control: camera::Control,
    /// The longest a frame's area uploads took, in nanoseconds, and the last frame's ground triangles (the bench,
    /// A15.10).
    upload_ns: u64,
    /// How long handing the demo area to the renderer took, its sky field with it, in nanoseconds (A15.10).
    insert_ns: u64,
    triangles: u64,
    /// The stones and tufts the last frame handed the GPU (A11.5).
    items: u64,
}

/// The golden scenes of A11.12, drawn with time frozen and no strip.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Golden {
    /// The whole light card, the block turned 30°.
    LightCard,
    /// The block alone, filling the screen, turned 30°.
    Block,
    /// The demo area at the camp stop, north up, its middle in the middle.
    ValleyCamp,
    /// The cliff's foot at the close camp stop, looking west-north-west at the face.
    ValleyClose,
    /// The scree and the grass below the face, near the person stop.
    ValleyNear,
}

impl Golden {
    pub fn named(name: &str) -> Option<Golden> {
        match name {
            "light-card" => Some(Golden::LightCard),
            "block" => Some(Golden::Block),
            "valley-camp" => Some(Golden::ValleyCamp),
            "valley-close" => Some(Golden::ValleyClose),
            "valley-near" => Some(Golden::ValleyNear),
            _ => None,
        }
    }

    /// Whether the scene is the light card's rather than the ground's.
    pub fn card(self) -> bool {
        matches!(self, Golden::LightCard | Golden::Block)
    }

    /// A ground scene's pose on the demo area: the point in metres east and south of its corner, on its ground,
    /// the heading and the zoom.
    pub fn pose(self, g: &Ground) -> Option<CameraPose> {
        let (e, s, yaw, zoom) = match self {
            Golden::ValleyCamp => (128, 128, 0.0, 0.30),
            Golden::ValleyClose => (178, 128, 1.2, 0.14),
            Golden::ValleyNear => (180, 136, 1.2, 0.05),
            _ => return None,
        };
        let o = g.id.origin();
        Some(CameraPose {
            target: kd_core::geo::Pos {
                x: o.x + e * 256,
                y: o.y + s * 256,
                z: (g.height_m(e as usize, s as usize) * 256.0).round() as i32,
            },
            yaw,
            zoom,
        })
    }
}

impl App {
    pub fn new(platform: Arc<dyn Platform>, cfg: AppConfig) -> App {
        let report = selfcheck::Report {
            build: build_line().to_string(),
            device: cfg.device.clone(),
            gl: String::new(),
            fail: Vec::new(),
        };
        let mut app = App {
            platform,
            cfg,
            renderer: None,
            size: None,
            insets: Insets::default(),
            paused: false,
            frames: 0,
            panics_in_row: 0,
            report,
            report_sent: false,
            core_bits: true,
            catalogue: None,
            catalogue_ns: 0,
            hour: FIRST_HOUR,
            ui: Ui::default(),
            font: kd_ui::font::font(),
            colours: Colours::default(),
            version: build_line().to_string(),
            start_ns: None,
            golden: None,
            demo: None,
            demo_meshes: None,
            demo_ns: 0,
            island: None,
            island_ns: 0,
            island_top_m: 0.0,
            surfaces: None,
            tile_ns: 0,
            tiles_made: 0,
            settle_next: false,
            control: camera::Control::new(CameraPose::default()),
            upload_ns: 0,
            insert_ns: 0,
            triangles: 0,
            items: 0,
        };
        let t0 = app.platform.now_ns();
        match Catalogue::load(CATALOGUE) {
            Ok(c) => {
                app.colours = Colours::from_catalogue(&c);
                app.version = version_line(build_line(), &c, Catalogue::hash_of(CATALOGUE).unwrap_or(0));
                app.catalogue = Some(c);
            }
            Err(e) => app.check_failed(format!("catalogue: {e}")),
        }
        app.catalogue_ns = app.platform.now_ns().saturating_sub(t0);
        log::info!(target: "kd::app", "catalogue loaded in {} µs", app.catalogue_ns / 1000);
        // The core's probes, made here and hashed, against the cloud's hashes: about a millisecond (A15.9 item 5).
        if let Some(line) = selfcheck::core_line(&kd_core::bits::differences()) {
            app.core_bits = false;
            app.check_failed(line);
        }
        app.make_demo();
        app.make_island();
        app
    }

    /// Builds the first region (A5.6) and points the camera at its first camp's cave mouth from the valley stop,
    /// looking north-north-east up the valley (T02a.7).
    fn make_island(&mut self) {
        let Some(cat) = &self.catalogue else {
            return;
        };
        let Some(land) = cat.land(FIRST_LAND) else {
            self.check_failed(format!("the catalogue has no land {FIRST_LAND}"));
            return;
        };
        let t0 = self.platform.now_ns();
        let built = lands::build(land, ISLAND_SEED, cat);
        self.island_ns = self.platform.now_ns().saturating_sub(t0);
        match built {
            Ok(w) => {
                log::info!(target: "kd::app", "island built in {} ms", self.island_ns / 1_000_000);
                let f = &w.cells.fixed;
                self.island_top_m = w
                    .window
                    .cells()
                    .map(|c| f64::from(f.height[c.0 as usize]))
                    .fold(0.0, f64::max)
                    + 60.0;
                if let Some(camp) = f.cave_records.iter().find(|r| r.first_camp) {
                    let mouth = kd_core::geo::Pos {
                        x: camp.mouth[0],
                        y: camp.mouth[1],
                        z: 0,
                    };
                    let z = w.cells.coarse_z(mouth).max(0.0);
                    self.control = camera::Control::new(CameraPose {
                        target: kd_core::geo::Pos {
                            z: (z * 256.0).round() as i32,
                            ..mouth
                        },
                        yaw: START_YAW,
                        zoom: START_ZOOM,
                    });
                }
                self.island = Some(w);
            }
            Err(e) => self.check_failed(format!("island: {e}")),
        }
    }

    /// The land the gestures read: the demo area's ground where it has any, else the island's.
    fn lands<'a>(demo: &'a Option<Ground>, island: &'a Option<PresetWorld>, top_m: f64) -> camera::Lands<'a> {
        camera::Lands {
            demo: demo.as_ref(),
            island: island.as_ref().map(|w| camera::Island {
                cells: &w.cells,
                window: w.window,
                top_m,
            }),
        }
    }

    /// Feeds the renderer the tiles of coarse ground the view needs (A11.5): drops those beyond `TILE_KEEP_M`, and
    /// makes those within `TILE_WANT_M` it lacks, nearest first, until `budget_ns` of the frame is spent, all of
    /// them with none.
    fn feed_tiles(&mut self, budget_ns: Option<u64>) {
        let (Some(r), Some(w), Some(cat), Some(surfaces)) =
            (self.renderer.as_mut(), &self.island, &self.catalogue, &self.surfaces)
        else {
            return;
        };
        let pose = self.control.pose;
        let keep = r.tiles_wanted(&pose, TILE_KEEP_M);
        r.retain_tiles(&keep);
        let t0 = self.platform.now_ns();
        let cx = CellCtx {
            seed: ISLAND_SEED,
            cells: &w.cells,
            cat,
        };
        for tile in r.tiles_wanted(&pose, TILE_WANT_M) {
            if r.has_tile(tile) || !tile_in(w, tile) {
                continue;
            }
            r.upload_tile(ground::coarse_tile(&cx, tile, surfaces, kd_core::time::GameTime(0)));
            self.tiles_made += 1;
            if budget_ns.is_some_and(|b| self.platform.now_ns().saturating_sub(t0) >= b) {
                break;
            }
        }
        self.tile_ns = self.tile_ns.max(self.platform.now_ns().saturating_sub(t0));
    }

    /// Makes the demo area (α01b), checks its ground against the cloud's hash (A15.9 item 5), turns its materials
    /// into the catalogue's surfaces, and points the camera at its middle at the camp stop.
    fn make_demo(&mut self) {
        let t0 = self.platform.now_ns();
        let g = demo::make(demo::SEED);
        self.demo_ns = self.platform.now_ns().saturating_sub(t0);
        log::info!(target: "kd::app", "demo area made in {} ms", self.demo_ns / 1_000_000);
        if let Some(line) = selfcheck::demo_line(g.hash(), demo::HASH) {
            self.check_failed(line);
        }
        if let Some(cat) = &self.catalogue {
            match ground::surface_numbers(cat) {
                Ok(n) => {
                    self.demo_meshes = Some(ground::area_meshes(&g, &n));
                    self.surfaces = Some(n);
                }
                Err(e) => self.check_failed(format!("demo area: {e}")),
            }
        }
        let o = g.id.origin();
        let middle = kd_world::area::SQUARES / 2;
        self.control = camera::Control::new(CameraPose {
            target: kd_core::geo::Pos {
                x: o.x + middle as i32 * 256,
                y: o.y + middle as i32 * 256,
                z: (g.height_m(middle, middle) * 256.0).round() as i32,
            },
            yaw: 0.0,
            zoom: 0.30,
        });
        self.demo = Some(g);
    }

    pub fn handle(&mut self, m: AppMsg) {
        match m {
            AppMsg::Input(e) => {
                // Any touch shows the strip and stops the camera's easing; a tap on the strip steps the hour, until
                // the clock runs time (α03a); the land's gestures move the camera (A12.2).
                if e.kind == kd_view::InputKind::Down {
                    self.control.hold();
                }
                match self.ui.input(&e, ART_SCALE, self.screen_ui(), self.inset_ui()) {
                    Some(UiAction::StepHour) => self.set_hour(self.hour + 1),
                    Some(UiAction::Camera(g)) => {
                        let window = self.size.unwrap_or([1, 1]);
                        let land = App::lands(&self.demo, &self.island, self.island_top_m);
                        self.control.gesture(g, window, Some(&land), e.t_ns);
                    }
                    None => {}
                }
            }
            AppMsg::Insets(i) => self.insets = i,
            AppMsg::Pause => self.paused = true,
            AppMsg::Resume => self.paused = false,
            AppMsg::Back => {} // nothing to close yet: frame() returns 0, so the shell sends the app to the back
        }
    }

    /// A new GL context, at start or after a lost one: the renderer is built again from scratch (A11.13 rule 5),
    /// and every shader that fails to compile goes into the self-check's report.
    pub fn gl_ready(&mut self, gl: glow::Context) {
        let info = kd_render::gl::info(&gl);
        self.report.gl.clone_from(&info);
        if !selfcheck::gl_version_ok(&info) {
            self.check_failed(format!("GL version: {info}"));
        }
        let Some(cat) = self.catalogue.as_ref() else {
            // The catalogue failed its checks, which the self-check already reports: nothing to draw with.
            self.finish_check();
            return;
        };
        let assets = kd_view::Assets {
            font: self.font.clone(),
        };
        self.renderer = match Renderer::new(gl, cat, &assets) {
            Ok(mut r) => {
                let platform = Arc::clone(&self.platform);
                r.set_clock(Box::new(move || platform.now_ns()));
                if let Some([w, h]) = self.size
                    && let Err(e) = r.resize(w, h, ART_SCALE)
                {
                    self.check_failed(e.to_string());
                }
                // The probe scene: the GPU must pick exactly the twins' steps (A11.13 rule 2).
                let (gpu, twins) = r.probe();
                if let Some(line) = selfcheck::probe_line(&gpu, &twins) {
                    self.check_failed(line);
                }
                if let Some(m) = &self.demo_meshes {
                    let t0 = self.platform.now_ns();
                    r.upload_area(m.clone());
                    self.insert_ns = self.platform.now_ns().saturating_sub(t0);
                }
                Some(r)
            }
            Err(e) => {
                self.check_failed(e.to_string());
                None
            }
        };
        self.finish_check();
    }

    /// No GL context could be made: the self-check says why.
    pub fn gl_failed(&mut self, why: String) {
        self.check_failed(format!("GL: {why}"));
        self.finish_check();
    }

    /// A new window size in screen pixels; an art pixel stays 4 × 4 of them (`PRE-22`).
    pub fn resize(&mut self, w: u32, h: u32) {
        self.size = Some([w, h]);
        if let Some(r) = self.renderer.as_mut()
            && let Err(e) = r.resize(w, h, ART_SCALE)
        {
            self.check_failed(e.to_string());
            self.finish_check();
        }
    }

    /// One frame; returns how many cards or views Back would close. A panic skips the frame, and after three in a
    /// row the shell shows a code to send back (A3.8).
    pub fn frame(&mut self, now_ns: u64) -> u16 {
        // The strip shows for its 3 seconds when the app opens, as after a touch (PRE-32).
        if !self.ui.strip.touched() {
            self.ui.strip.touch(now_ns);
        }
        let start = *self.start_ns.get_or_insert(now_ns);
        let list = self.ui_list(now_ns);
        // The light card shows only as its golden scenes now the ground has come (α01b).
        let card = self
            .golden
            .filter(|g| g.card())
            .map(|_| self.card_frame(now_ns.saturating_sub(start)));
        if self.golden.is_none() {
            self.control.tick(now_ns);
        }
        // A golden scene's frame, or a settled one, has all its tiles, as its fields (A11.11).
        let whole = self.golden.is_some() || std::mem::take(&mut self.settle_next);
        self.feed_tiles((!whole).then_some(TILE_FRAME_NS));
        // The light card is lit at the demo's latitude, as it always was; the ground at its own.
        let lat = if self.golden.is_some_and(Golden::card) {
            HOURS_LAT
        } else {
            latitude(self.control.pose.target)
        };
        if let Some(r) = self.renderer.as_mut() {
            let f = Frame {
                count: self.frames,
                sky: sky_at(self.hour, lat),
                cam: self.control.pose,
                card,
                // A golden scene's frame, or a settled one, finishes its fields at once (A11.11).
                field_ns: (!whole).then_some(kd_render::field::SUN_FIELD_FRAME_NS),
            };
            let t0 = self.platform.now_ns();
            if r.upload_areas() > 0 {
                self.upload_ns = self.upload_ns.max(self.platform.now_ns().saturating_sub(t0));
            }
            match panic::catch_unwind(AssertUnwindSafe(|| r.draw(&f, &list))) {
                Ok(stats) => {
                    self.panics_in_row = 0;
                    (self.triangles, self.items) = (stats.triangles, stats.items);
                }
                Err(e) => {
                    self.panics_in_row += 1;
                    let msg = e
                        .downcast_ref::<&str>()
                        .map(|s| s.to_string())
                        .or_else(|| e.downcast_ref::<String>().cloned())
                        .unwrap_or_else(|| "a panic with no message".to_string());
                    log::error!(target: "kd::app", "frame {} skipped: {msg}", self.frames);
                    if self.panics_in_row == 3 {
                        let mut r = self.report.clone();
                        r.fail = vec![format!("three frames in a row panicked: {msg}")];
                        self.platform.post(Request::ShowCode {
                            title: "Kindling stopped drawing".to_string(),
                            prefix: selfcheck::PREFIX.to_string(),
                            json: r.to_json(),
                        });
                    }
                }
            }
        }
        self.frames += 1;
        0
    }

    /// The screen in UI pixels: whole ones across, and down to the last whole row, as the grid starts at the top-left
    /// corner (A12.1).
    fn screen_ui(&self) -> [i32; 2] {
        let [w, h] = self.size.unwrap_or([0, 0]);
        [w.div_ceil(ART_SCALE) as i32, (h / ART_SCALE) as i32]
    }

    /// The bottom inset (the gesture strip) in UI pixels.
    fn inset_ui(&self) -> i32 {
        self.insets.bottom.div_ceil(ART_SCALE) as i32
    }

    /// What the light card shows `age_ns` after the first frame: the block turning once in 24 seconds, or a golden
    /// scene's still (A11.12).
    fn card_frame(&self, age_ns: u64) -> card::CardFrame {
        let block_look = self
            .catalogue
            .as_ref()
            .and_then(|c| c.looks.iter().position(|l| l.id == "limestone"))
            .unwrap_or(0);
        let turn = match self.golden {
            Some(_) => 30f32.to_radians(),
            None => {
                let turns = (age_ns % (card::TURN_S as u64 * 1_000_000_000)) as f32 / (card::TURN_S * 1e9);
                turns * std::f32::consts::TAU
            }
        };
        card::CardFrame {
            block_look,
            turn,
            block_only: self.golden == Some(Golden::Block),
        }
    }

    /// Test hook (A12.4): the next frame makes every tile of coarse ground its view needs and finishes every light
    /// field, as a golden scene's does, so a picture of any pose is whole.
    pub fn settle(&mut self) {
        self.settle_next = true;
    }

    /// Test hook (A11.12): shows a golden scene with time frozen and no strip, a ground scene from its pose, or
    /// the live ground again.
    pub fn set_golden(&mut self, golden: Option<Golden>) {
        self.golden = golden;
        if let (Some(pose), Some(_)) = (golden.zip(self.demo.as_ref()).and_then(|(g, d)| g.pose(d)), golden) {
            self.control.set(pose);
        }
    }

    /// Test hook (A12.4): the ground under a window point in screen pixels, as a position.
    pub fn ground_at(&self, p: [f32; 2]) -> Option<kd_core::geo::Pos> {
        let land = App::lands(&self.demo, &self.island, self.island_top_m);
        Some(camera::ground_point(&self.control.pose, self.size?, Some(&land), p))
    }

    /// Test hook (A12.4): where a position shows in the window, in screen pixels.
    pub fn screen_of(&self, p: kd_core::geo::Pos) -> Option<[f64; 2]> {
        Some(camera::window_point(&self.control.pose, self.size?, p))
    }

    /// Test hook (A11.12): the palette row in use, as `rrggbb` words, or nothing before the first frame.
    pub fn palette_hex(&self) -> Option<String> {
        let row = &self.renderer.as_ref()?.palette()?.row;
        Some(
            row.iter()
                .map(|c| format!("{:02x}{:02x}{:02x}", c[0], c[1], c[2]))
                .collect::<Vec<_>>()
                .join(" "),
        )
    }

    /// The frame's UI (A12.1): the strip while it shows, or in the light card's golden scene its look names over
    /// their swatches; a golden scene has no strip, so it never changes with the version or the clock.
    pub fn ui_list(&self, now_ns: u64) -> UiDrawList {
        let mut list = UiDrawList::default();
        if self.golden == Some(Golden::Block) {
            return list;
        }
        if let (Some(cat), Some(Golden::LightCard)) = (&self.catalogue, self.golden) {
            for (row, look) in cat.looks.iter().take(card::MAX_LOOKS).enumerate() {
                // The cell's cap height starts two rows down, on the row's name line.
                let y = card::MARGIN + row as i32 * card::ROW - 2;
                kd_ui::draw::text(
                    &mut list,
                    &self.font,
                    [card::MARGIN, y],
                    self.colours.text,
                    1.0,
                    &look.name,
                );
            }
        }
        if self.golden.is_some() {
            return list;
        }
        self.ui.strip.draw(
            &mut list,
            &self.font,
            now_ns,
            self.screen_ui(),
            self.inset_ui(),
            &hour_line(self.hour),
            &self.version,
            self.colours,
        );
        list
    }

    /// Test hook (A11.13 rule 2): the probe scene's steps from the GPU and from the twins, once the renderer exists.
    pub fn probe(&mut self) -> Option<(Vec<u8>, Vec<u8>)> {
        self.renderer.as_mut().map(Renderer::probe)
    }

    /// Frames drawn so far.
    pub fn frames(&self) -> u64 {
        self.frames
    }

    /// The art target's size, once the renderer and the window's size are known.
    pub fn art_size(&self) -> Option<[u32; 2]> {
        self.renderer.as_ref().and_then(Renderer::art_size)
    }

    /// Whether the next frame will draw: a renderer and a window size.
    pub fn ready(&self) -> bool {
        self.art_size().is_some()
    }

    pub fn paused(&self) -> bool {
        self.paused
    }

    pub fn insets(&self) -> Insets {
        self.insets
    }

    pub fn device(&self) -> &str {
        &self.cfg.device
    }

    /// Whether the core's maths and draws give the cloud's bits on this device.
    pub fn core_bits(&self) -> bool {
        self.core_bits
    }

    /// Which of `HOURS` lights the frame.
    pub fn hour(&self) -> usize {
        self.hour
    }

    /// Lights the frame at one of `HOURS` (the strip's tap steps it, α01a; the clock replaces it, α03a).
    pub fn set_hour(&mut self, hour: usize) {
        self.hour = hour % HOURS.len();
    }

    /// The catalogue, if its blob loaded.
    pub fn catalogue(&self) -> Option<&Catalogue> {
        self.catalogue.as_ref()
    }

    /// How long the catalogue took to load, in nanoseconds.
    pub fn catalogue_ns(&self) -> u64 {
        self.catalogue_ns
    }

    /// How long making the demo area took, in nanoseconds.
    pub fn demo_ns(&self) -> u64 {
        self.demo_ns
    }

    /// The first region, once built.
    pub fn island(&self) -> Option<&PresetWorld> {
        self.island.as_ref()
    }

    /// How long building the first region took, in nanoseconds; the longest a frame spent making tiles of coarse
    /// ground, and how many it has made.
    pub fn island_stats(&self) -> (u64, u64, u64) {
        (self.island_ns, self.tile_ns, self.tiles_made)
    }

    /// The longest a frame's area uploads took, in nanoseconds, the last frame's ground triangles, and the stones
    /// and tufts it handed the GPU.
    pub fn ground_stats(&self) -> (u64, u64, u64) {
        (self.upload_ns, self.triangles, self.items)
    }

    /// The light fields' times, in nanoseconds (A15.10): handing the demo area to the renderer, its sky field with
    /// it, and the longest a frame's sun field work has taken.
    pub fn field_stats(&self) -> (u64, u64) {
        let field = self.renderer.as_ref().map_or(0, Renderer::longest_field_ns);
        (self.insert_ns, field)
    }

    /// Where the camera looks.
    pub fn camera(&self) -> CameraPose {
        self.control.pose
    }

    /// Points the camera (a test hook), the zoom held within the stops in reach.
    pub fn set_camera(&mut self, pose: CameraPose) {
        let [lo, hi] = kd_render::camera::ZOOM_IN_REACH;
        self.control.set(CameraPose {
            zoom: pose.zoom.clamp(lo, hi),
            ..pose
        });
    }

    /// The last frame's view of the ground, for the test hooks: the art pixel's size, the upscale's shift and the
    /// art target's corner in the world's grid.
    pub fn view(&self) -> Option<kd_render::camera::View> {
        self.renderer.as_ref()?.last_view().copied()
    }

    /// The demo area's ground, as the world made it.
    pub fn demo(&self) -> Option<&Ground> {
        self.demo.as_ref()
    }

    /// Test hook (A11.10): `frames` frames of a slow camera motion on from where the camera looks, the `k`th frame
    /// `k` times `rate` from it, all drawn at one moment so nothing else moves, and what changed between each frame
    /// and the next; the camera is left at the last frame.
    pub fn crawl(&mut self, motion: Motion, rate: f32, frames: u32, now_ns: u64) -> Vec<kd_render::probe::CrawlCount> {
        let start = self.control.pose;
        let texel = kd_render::camera::texel(start.zoom);
        let mut out = Vec::new();
        let mut last = None;
        for k in 0..=frames {
            let k = k as f32;
            self.set_camera(match motion {
                Motion::Turn => CameraPose {
                    yaw: start.yaw + (k * rate).to_radians(),
                    ..start
                },
                Motion::Zoom => CameraPose {
                    zoom: kd_render::camera::zoom_of(texel * (1.0 + rate).powf(k)),
                    ..start
                },
                Motion::Pan => {
                    // Along the screen's right: east turned by the heading, counter-clockwise from north.
                    let along = k * rate * texel * 256.0;
                    CameraPose {
                        target: kd_core::geo::Pos {
                            x: start.target.x + (along * start.yaw.cos()).round() as i32,
                            y: start.target.y - (along * start.yaw.sin()).round() as i32,
                            ..start.target
                        },
                        ..start
                    }
                }
            });
            self.frame(now_ns);
            let capture = self.renderer.as_ref().and_then(Renderer::capture);
            if let (Some(a), Some(b)) = (&last, &capture) {
                out.push(kd_render::probe::count(a, b));
            }
            last = capture;
        }
        out
    }

    /// Test hook (A11.12): what each step of the zoom changes, from zoom `from` to `to` by `step`, the camera's
    /// target and heading held.
    pub fn zoom_strip(&mut self, from: f32, to: f32, step: f32, now_ns: u64) -> Vec<kd_render::probe::CrawlCount> {
        let mut out = Vec::new();
        let mut last = None;
        let steps = ((to - from) / step).round() as u32;
        for k in 0..=steps {
            let zoom = from + step * k as f32;
            self.set_camera(CameraPose {
                zoom,
                ..self.control.pose
            });
            self.frame(now_ns);
            let capture = self.renderer.as_ref().and_then(Renderer::capture);
            if let (Some(a), Some(b)) = (&last, &capture) {
                out.push(kd_render::probe::count(a, b));
            }
            last = capture;
        }
        out
    }

    fn check_failed(&mut self, what: String) {
        log::error!(target: "kd::app", "self-check: {what}");
        self.report.fail.push(what);
    }

    /// Sends the report once a run, if anything failed.
    fn finish_check(&mut self) {
        if self.report_sent || self.report.fail.is_empty() {
            return;
        }
        self.report_sent = true;
        self.platform.post(Request::ShowCode {
            title: "Kindling self-check".to_string(),
            prefix: selfcheck::PREFIX.to_string(),
            json: self.report.to_json(),
        });
    }
}

/// Where the app opens (T02a.7): the valley stop, looking north-north-east up the valley.
pub const START_ZOOM: f32 = 0.50;
pub const START_YAW: f32 = -0.45;

/// Whether any of a tile's cells lies in the island's square.
fn tile_in(w: &PresetWorld, tile: kd_core::geo::CellIx) -> bool {
    let (x, y) = tile.xy();
    let (wx, wy, n) = (w.window.x0, w.window.y0, w.window.size);
    let t = kd_view::TILE_CELLS;
    x < wx + n && x + t > wx && y < wy + n && y + t > wy
}
