//! kd-app: the app object: frame loop, input, the self-check (A2.2, A2.4, A3.8, A15.4); implements PRC-11, PRE-22,
//! PRE-32 and PLT-02 in part.
//! In α01b it shows the valley, the demo area as pixel-art ground under the camera, with the version strip over it;
//! the cube stays as the golden scene `cube`. The simulation and I/O threads join from α03a and α07a.

pub mod camera;
pub mod json;
pub mod selfcheck;
pub mod valley;

pub use json::{json_str, requests_json};
pub use kd_view::{InputEvent, InputKind, Insets};

use camera::CameraCtl;
use kd_data::Catalogue;
use kd_render::camera::ArtSize;
use kd_render::crawl::{self, PairCount};
use kd_render::{Assets, DrawSettings, Frame, Renderer};
use kd_ui::gestures::Gestures;
use kd_ui::{Font, Ui};
use kd_view::{CameraPose, CubeView, GroundGrid, Snapshot};
use std::sync::Arc;

/// What the app needs from its shell (A2.2); `storage` arrives in α07a, `cores` in α02b.
pub trait Platform: Send + Sync {
    /// Monotonic nanoseconds: `CLOCK_MONOTONIC` on the phone, `performance.now()` on the web (A2.4).
    fn now_ns(&self) -> u64;
    /// Into the shell's outbox, which it drains each frame (A2.5).
    fn post(&self, r: Request);
}

/// What the app asks of its shell (A2.2).
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Request {
    /// Show the self-check's `KDS1:` code made from this JSON (A15.4).
    SelfCheck { json: String },
}

/// A message from the shell's UI thread (A2.2).
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum AppMsg {
    Input(InputEvent),
    /// The system's insets, screen pixels (A2.2).
    Insets(Insets),
    Pause,
    Resume,
    Back,
}

/// What the shell tells the app at start.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct AppConfig {
    pub device: String,
}

/// The build's version line: the alpha and commit set by the build scripts in `KD_BUILD`, else `dev` (A15.3).
pub fn build_line() -> &'static str {
    option_env!("KD_BUILD").unwrap_or("dev")
}

/// The catalogue blob compiled from `data/` by the build script (A3.6).
pub static CATALOGUE: &[u8] = include_bytes!(concat!(env!("OUT_DIR"), "/catalogue.bin"));

/// Screen pixels an art pixel spans, in both orientations, on the phone and on the web (`PRE-22`, A11.2).
pub const SCALE: u32 = 4;
/// Frames kept for the frame-time figures (A11.11).
const RING: usize = 1024;
/// Frames the strip's averages cover.
const AVERAGE: usize = 120;

/// The last 1,024 frames' intervals and GL thread times, milliseconds (A11.11).
struct FrameRing {
    interval_ms: Vec<f32>,
    gl_ms: Vec<f32>,
    n: usize,
}

impl FrameRing {
    fn push(&mut self, interval_ms: f32, gl_ms: f32) {
        let i = self.n % RING;
        self.interval_ms[i] = interval_ms;
        self.gl_ms[i] = gl_ms;
        self.n += 1;
    }

    /// The mean of the last frames, up to `AVERAGE`, of one column.
    fn mean(&self, col: &[f32]) -> f32 {
        let k = self.n.min(AVERAGE);
        if k == 0 {
            return 0.0;
        }
        (1..=k).map(|j| col[(self.n - j) % RING]).sum::<f32>() / k as f32
    }

    /// Frames a second and GL thread milliseconds, averaged.
    fn figures(&self) -> (f32, f32) {
        let iv = self.mean(&self.interval_ms);
        (if iv > 0.0 { 1000.0 / iv } else { 0.0 }, self.mean(&self.gl_ms))
    }
}

/// A fixed golden scene (A11.12): the cube, or the valley from a fixed pose.
#[derive(Clone, Copy, Debug, PartialEq)]
enum Golden {
    Cube(CubeView),
    Valley(CameraPose),
}

/// The art pixels of the phone's screen in portrait, where the crawl counter measures (A11.10).
const PHONE_W: u32 = 1080;
const PHONE_H: u32 = 2404;

/// The app object (A2.2): owned by the GL thread on the phone, by the page on the web.
pub struct App {
    platform: Arc<dyn Platform>,
    cfg: AppConfig,
    renderer: Option<Renderer>,
    catalogue: Catalogue,
    assets: Assets,
    ui: Ui,
    ring: FrameRing,
    last_frame_ns: Option<u64>,
    golden: Option<Golden>,
    /// Frames drawn since the renderer took the ground (`ready`).
    frames_since_ground: u32,
    ground: GroundGrid,
    ctl: CameraCtl,
    gestures: Gestures,
    gl_info: String,
    fail: Vec<String>,
    reported: bool,
    frames: u64,
    w: u32,
    h: u32,
    paused: bool,
    /// Unpaused real seconds (`Frame::real_s`).
    run_s: f64,
    last_ns: Option<u64>,
}

impl App {
    /// Builds the app and runs kd-core's part of the self-check (A15.4; in α00 at every start).
    pub fn new(p: Arc<dyn Platform>, cfg: AppConfig) -> App {
        let mut fail: Vec<String> = kd_core::selfcheck::core_check().into_iter().map(String::from).collect();
        let catalogue = Catalogue::load(CATALOGUE).unwrap_or_else(|e| {
            fail.push(format!("catalogue: {e}"));
            Catalogue::default()
        });
        let font = Font::parse(kd_ui::font::GLYPHS_7).unwrap_or_else(|e| {
            fail.push(format!("font: {e}"));
            Font::default()
        });
        let assets = Assets { font: font.atlas() };
        let ui = Ui::new(font, &catalogue);
        let ground = valley::ground(&catalogue);
        let ctl = CameraCtl::new(valley::start(&ground));
        App {
            platform: p,
            cfg,
            renderer: None,
            catalogue,
            assets,
            ui,
            ring: FrameRing {
                interval_ms: vec![0.0; RING],
                gl_ms: vec![0.0; RING],
                n: 0,
            },
            last_frame_ns: None,
            golden: None,
            frames_since_ground: 0,
            ground,
            ctl,
            gestures: Gestures::new(SCALE, 1),
            gl_info: String::new(),
            fail,
            reported: false,
            frames: 0,
            w: 1,
            h: 1,
            paused: false,
            run_s: 0.0,
            last_ns: None,
        }
    }

    /// A GL context is ready: (re)builds the renderer; a failure is remembered as `shader: <log>`.
    pub fn gl_ready(&mut self, gl: glow::Context) {
        match Renderer::new(gl, &self.catalogue, &self.assets) {
            Ok(mut r) => {
                self.gl_info = r.gl_info();
                r.resize(self.w, self.h, SCALE);
                if let Err(e) = r.set_ground(&self.ground) {
                    self.fail.push(format!("ground: {e}"));
                }
                self.frames_since_ground = 0;
                self.renderer = Some(r);
            }
            Err(e) => {
                log::error!(target: "kd::app", "{e}");
                self.fail.push(format!("shader: {e}"));
                self.renderer = None;
            }
        }
    }

    /// The drawing surface's size in pixels.
    pub fn resize(&mut self, w: u32, h: u32) {
        self.w = w.max(1);
        self.h = h.max(1);
        if let Some(r) = &mut self.renderer {
            r.resize(self.w, self.h, SCALE);
        }
        self.ui.resize(self.w, SCALE);
        self.gestures.resize(SCALE, self.h);
    }

    /// The art target and the window, for the camera's sums.
    fn art(&self) -> ArtSize {
        let (wf, hf) = kd_render::target::art_size(self.w, self.h, SCALE);
        ArtSize {
            wf,
            hf,
            wd: self.w,
            hd: self.h,
            s: SCALE,
        }
    }

    /// The next message from the shell.
    pub fn handle(&mut self, m: AppMsg) {
        match m {
            AppMsg::Input(e) => {
                // A touch on the UI belongs to it; the camera never sees it (A12.2).
                if !self.ui.input(&e) {
                    self.input(e);
                }
            }
            AppMsg::Insets(i) => self.ui.set_insets(i),
            AppMsg::Pause => {
                self.paused = true;
                self.last_ns = None;
            }
            AppMsg::Resume => self.paused = false,
            AppMsg::Back => {}
        }
    }

    /// A touch the UI did not take: the gestures move the camera (A12.2).
    fn input(&mut self, e: InputEvent) {
        let mut cmds = Vec::new();
        self.gestures.input(&e, &mut cmds);
        let size = self.art();
        for c in cmds {
            self.ctl.apply(c, size, &self.ground);
        }
    }

    /// The turn shown, radians (the web test hook): the golden cube's, else the camera's.
    pub fn yaw(&self) -> f32 {
        match self.golden {
            Some(Golden::Cube(c)) => c.yaw,
            Some(Golden::Valley(p)) => p.yaw,
            None => self.ctl.pose.yaw,
        }
    }

    /// Unpaused real seconds so far.
    pub fn real_s(&self) -> f64 {
        self.run_s
    }

    /// Freezes time and the camera and shows a fixed golden scene (A11.12, A12.4) on dusk's row, with no strip:
    /// `cube`, at yaw 0.6 and pitch 0.5; `valley-camp`, `valley-close` and `valley-near`, the start's view of the
    /// cliff at the camp stop (zoom 0.30), the close camp stop (0.14) and the closest zoom (0.00), where tufts and
    /// stones show and the floating origin is not the area's corner. Returns whether the scene exists.
    pub fn golden(&mut self, name: &str) -> bool {
        let (x, y) = valley::START_AT;
        let valley = |zoom| Golden::Valley(valley::pose_at(&self.ground, x, y, valley::START_YAW, zoom));
        self.golden = match name {
            "cube" => Some(Golden::Cube(CubeView { yaw: 0.6, pitch: 0.5 })),
            "valley-camp" => Some(valley(0.30)),
            "valley-close" => Some(valley(0.14)),
            "valley-near" => Some(valley(camera::ZOOM_MIN)),
            _ => return false,
        };
        true
    }

    /// The camera (the web test hook): its target as (x, y) metres from the ground's corner and its height, its
    /// turn, its zoom, metres per art pixel, and where the target lies within its art pixel along right and up.
    pub fn camera(&self) -> [f32; 8] {
        let p = self.ctl.pose;
        let d = kd_core::geo::delta(self.ground.origin, p.target);
        let c = kd_render::camera::compute(&p, self.art(), 0.0, 1.0);
        [
            d.x,
            d.y,
            p.target.z as f32 / 256.0,
            p.yaw,
            p.zoom,
            c.texel,
            c.frac[0],
            c.frac[1],
        ]
    }

    /// Points the camera at (`x`, `y`) metres from the ground's corner, at height `z` or else the ground's, with a
    /// turn and a zoom (the web test hook).
    pub fn set_camera(&mut self, x: f32, y: f32, z: Option<f32>, yaw: f32, zoom: f32) {
        let zoom = zoom.clamp(camera::ZOOM_MIN, camera::ZOOM_MAX);
        let mut pose = valley::pose_at(&self.ground, x, y, yaw, zoom);
        if let Some(z) = z {
            pose.target.z = (z * 256.0).round() as i32;
        }
        self.ctl = CameraCtl::new(pose);
    }

    /// Whether the frame after the ground's upload has drawn (the test hook's `ready()`).
    pub fn ready(&self) -> bool {
        self.renderer.is_some() && self.frames_since_ground > 0
    }

    /// B66's crawl count (A11.10), the test hook's `crawl()`: on the phone's art target in portrait, from the start's
    /// view at `zoom` (the camp stop, 0.30, unless given), the camera moves one 60 Hz step a frame for `frames`
    /// frames, turning by `rate` radians, zooming by `rate`, or panning `rate` metres east and 0.58 of that north at
    /// the start's height, as a drag does; each frame is drawn and captured, and each pair counted. Only the fix
    /// `base` exists until the owner's review. `None` without a renderer.
    pub fn crawl(&mut self, motion: &str, rate: f32, frames: u32, fix: &str, zoom: Option<f32>) -> Option<String> {
        if fix != "base" {
            return None;
        }
        let (w, h) = (self.w, self.h);
        self.renderer.as_mut()?.resize(PHONE_W, PHONE_H, SCALE);
        let (x, y) = valley::START_AT;
        let start = valley::pose_at(&self.ground, x, y, valley::START_YAW, zoom.unwrap_or(0.30));
        let (mut tot, mut changed_frames, mut worst) = (PairCount::default(), 0u32, (0u32, 0u64));
        let mut prev: Option<crawl::Shown> = None;
        for k in 0..=frames {
            let t = k as f32;
            let pose = match motion {
                "turn" => CameraPose {
                    yaw: start.yaw + rate * t,
                    ..start
                },
                "zoom" => CameraPose {
                    zoom: start.zoom + rate * t,
                    ..start
                },
                _ => {
                    let p = valley::pose_at(&self.ground, x + rate * t, y - 0.58 * rate * t, start.yaw, start.zoom);
                    CameraPose {
                        target: kd_core::geo::Pos {
                            z: start.target.z,
                            ..p.target
                        },
                        ..p
                    }
                }
            };
            let f = Frame {
                real_s: 10.0 + f64::from(t) / 60.0,
                palette_row: 0.0,
                set: DrawSettings::default(),
                camera: pose,
            };
            let r = self.renderer.as_mut()?;
            r.draw(&f, &Snapshot::default(), &kd_view::UiDrawList::default());
            let shown = crawl::compose(&r.capture()?);
            if let Some(p) = &prev {
                let n = crawl::pair(p, &shown);
                tot.valid += n.valid;
                tot.elig += n.elig;
                tot.crawl += n.crawl;
                tot.changed += n.changed;
                changed_frames += u32::from(n.changed > 0);
                if n.crawl > worst.1 {
                    worst = (k, n.crawl);
                }
            }
            prev = Some(shown);
        }
        self.renderer.as_mut()?.resize(w, h, SCALE);
        Some(format!(
            "{{\"frames\":{frames},\"valid\":{},\"elig\":{},\"crawl\":{},\"changed\":{},\"changedFrames\":{changed_frames},\"worstK\":{},\"worstCrawl\":{},\"art\":[{PHONE_W},{PHONE_H},{SCALE}]}}",
            tot.valid, tot.elig, tot.crawl, tot.changed, worst.0, worst.1
        ))
    }

    /// The palette row in use: dusk 0, dawn 1, day 2, night 3 (`PRE-30`).
    pub fn palette_row(&self) -> usize {
        if self.golden.is_some() {
            0
        } else {
            self.ui.palette_row()
        }
    }

    /// The current palette row's colours as RGB bytes, the catalogue's colours only (the web test hook's
    /// `palette only` check).
    pub fn palette_rgb(&self) -> Vec<u8> {
        let n = self.catalogue.body.colours.len();
        self.renderer.as_ref().map_or_else(Vec::new, |r| {
            r.palette_row_rgba(self.palette_row())
                .chunks(4)
                .take(n)
                .flat_map(|c| [c[0], c[1], c[2]])
                .collect()
        })
    }

    /// One frame at the shell's frame time: spins, builds the UI, draws, measures the GL thread's time into the
    /// ring (A11.11), and once per run posts the self-check if anything failed; returns how many cards or views Back
    /// would close (none yet).
    pub fn frame(&mut self, now_ns: u64) -> u16 {
        let t0 = self.platform.now_ns();
        let interval_ms = self
            .last_frame_ns
            .map_or(0.0, |l| now_ns.saturating_sub(l) as f32 / 1e6);
        self.last_frame_ns = Some(now_ns);
        if !self.paused {
            if let Some(last) = self.last_ns {
                let dt = now_ns.saturating_sub(last) as f64 / 1e9;
                self.run_s += dt;
                if self.golden.is_none() {
                    self.ctl.step(dt as f32, &self.ground);
                }
            }
            self.last_ns = Some(now_ns);
        }
        let (hz, gl_ms) = self.ring.figures();
        let ui = if self.golden.is_some() {
            kd_view::UiDrawList::default()
        } else {
            self.ui.build(now_ns as f64 / 1e9, build_line(), hz, gl_ms)
        };
        let mut f = Frame {
            real_s: self.run_s,
            palette_row: self.palette_row() as f32,
            set: DrawSettings::default(),
            camera: self.ctl.pose,
        };
        // the cube shows only as the golden scene `cube`; otherwise the ground, from a golden pose when one is set
        let snap = Snapshot {
            cube: match self.golden {
                Some(Golden::Cube(c)) => Some(c),
                _ => None,
            },
        };
        if let Some(Golden::Valley(p)) = self.golden {
            f.camera = p;
        }
        if let Some(r) = &mut self.renderer {
            r.draw(&f, &snap, &ui);
            self.frames_since_ground = self.frames_since_ground.saturating_add(1);
        }
        let gl = self.platform.now_ns().saturating_sub(t0) as f32 / 1e6;
        if interval_ms > 0.0 {
            self.ring.push(interval_ms, gl);
        }
        self.frames += 1;
        if self.frames == 2
            && let Some(r) = &self.renderer
        {
            let e = r.gl_error();
            if e != 0 {
                self.fail.push(format!("gl: {e:#x}"));
            }
        }
        if self.frames >= 2 && !self.reported {
            self.reported = true;
            if !self.fail.is_empty() {
                let json = selfcheck::report_json(
                    build_line(),
                    &self.cfg.device,
                    &self.gl_info,
                    &self.catalogue.version_line(),
                    &self.fail,
                );
                self.platform.post(Request::SelfCheck { json });
            }
        }
        0
    }

    /// Frames a second and the GL thread's milliseconds a frame, over the last frames (A11.11).
    pub fn frame_figures(&self) -> (f32, f32) {
        self.ring.figures()
    }

    /// The loaded catalogue (A3.6).
    pub fn catalogue(&self) -> &Catalogue {
        &self.catalogue
    }

    /// What failed so far in the self-check.
    pub fn failures(&self) -> &[String] {
        &self.fail
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Mutex;

    #[derive(Default)]
    struct TestPlatform {
        out: Mutex<Vec<Request>>,
    }
    impl Platform for TestPlatform {
        fn now_ns(&self) -> u64 {
            0
        }
        fn post(&self, r: Request) {
            self.out.lock().unwrap().push(r);
        }
    }

    fn touch(kind: InputKind, x: f32, t_ms: u64) -> AppMsg {
        AppMsg::Input(InputEvent {
            kind,
            pointer: 0,
            x,
            y: 400.0,
            t_ns: t_ms * 1_000_000,
        })
    }

    // checks: PRC-11 PRE-33
    #[test]
    fn drag_moves_the_ground_and_glides_to_rest() {
        let p = Arc::new(TestPlatform::default());
        let mut app = App::new(p.clone(), AppConfig::default());
        app.resize(412, 860);
        // looking north: right on the screen is east
        app.set_camera(128.0, 128.0, None, 0.0, 0.2);
        let texel = kd_render::camera::texel(0.2, app.art());
        app.handle(touch(InputKind::Down, 300.0, 0));
        for i in 1..=10 {
            app.handle(touch(InputKind::Move, 300.0 - 20.0 * i as f32, 16 * i));
        }
        // the ground followed the finger 200 pixels left, 50 art pixels, so the target moved east by as much, within
        // the rounding of ten moves to whole ticks of 1/256 m
        let after_drag = app.camera()[0];
        assert!((after_drag - (128.0 + 50.0 * texel)).abs() < 0.03, "{after_drag}");
        app.handle(touch(InputKind::Up, 100.0, 170));
        app.frame(170_000_000);
        app.frame(186_000_000);
        assert!(app.camera()[0] > after_drag, "glides on after release");
        let mut t = 186_000_000;
        for _ in 0..300 {
            t += 16_000_000;
            app.frame(t);
        }
        let rest = app.camera()[0];
        app.frame(t + 16_000_000);
        assert_eq!(app.camera()[0], rest, "the glide eases to rest");
        assert!(
            p.out.lock().unwrap().is_empty(),
            "core checks pass, so no self-check box"
        );
    }

    // checks: PRC-11
    #[test]
    fn pause_stops_real_time() {
        let mut app = App::new(Arc::new(TestPlatform::default()), AppConfig::default());
        app.frame(0);
        app.frame(1_000_000_000);
        assert!((app.real_s() - 1.0).abs() < 1e-9);
        app.handle(AppMsg::Pause);
        app.frame(5_000_000_000);
        assert!((app.real_s() - 1.0).abs() < 1e-9);
        app.handle(AppMsg::Resume);
        app.frame(6_000_000_000);
        app.frame(7_000_000_000);
        assert!((app.real_s() - 2.0).abs() < 1e-9);
        assert_eq!(app.frame(8_000_000_000), 0);
    }

    // checks: PRC-11
    #[test]
    fn failure_posts_one_self_check() {
        let p = Arc::new(TestPlatform::default());
        let mut app = App::new(p.clone(), AppConfig { device: "test".into() });
        app.fail.push("shader: planted".into());
        for t in 0..5 {
            app.frame(t);
        }
        let out = p.out.lock().unwrap();
        assert_eq!(out.len(), 1);
        let Request::SelfCheck { json } = &out[0];
        assert!(json.contains("\"fail\":[\"shader: planted\"]") && json.contains("\"dev\":\"test\""));
        assert!(json.contains("\"cat\":\"1.0 "), "{json}");
    }

    // checks: PRE-22 PRE-03
    #[test]
    fn valley_goldens_freeze_the_camera() {
        let mut app = App::new(Arc::new(TestPlatform::default()), AppConfig::default());
        assert!(app.golden("valley-camp"));
        assert_eq!(app.yaw(), valley::START_YAW);
        assert_eq!(app.palette_row(), 0);
        assert!(app.golden("valley-close"));
        assert!(app.golden("valley-near"));
        // a drag moves the live camera, not the frozen one
        app.handle(touch(InputKind::Down, 300.0, 0));
        app.handle(touch(InputKind::Move, 100.0, 16));
        assert_eq!(app.yaw(), valley::START_YAW);
        assert!(!app.ready(), "no renderer, so not ready");
        assert_eq!(
            app.crawl("turn", 0.002, 4, "base", None),
            None,
            "no renderer, so no count"
        );
        assert_eq!(
            app.crawl("turn", 0.002, 4, "fade", None),
            None,
            "only base until the review"
        );
    }

    // checks: PRE-22 PRE-30
    #[test]
    fn golden_freezes_the_cube() {
        let mut app = App::new(Arc::new(TestPlatform::default()), AppConfig::default());
        assert!(!app.golden("nothing"));
        assert!(app.golden("cube"));
        app.frame(0);
        app.frame(1_000_000_000);
        app.handle(touch(InputKind::Down, 300.0, 0));
        app.handle(touch(InputKind::Move, 100.0, 16));
        assert_eq!(app.yaw(), 0.6, "time and the drag are frozen");
        assert_eq!(app.palette_row(), 0, "on dusk's row");
    }

    // checks: PRE-32
    #[test]
    fn frame_ring_averages() {
        let mut r = FrameRing {
            interval_ms: vec![0.0; RING],
            gl_ms: vec![0.0; RING],
            n: 0,
        };
        assert_eq!(r.figures(), (0.0, 0.0));
        for _ in 0..2000 {
            r.push(8.0, 0.5);
        }
        assert_eq!(r.figures(), (125.0, 0.5));
    }

    // checks: PLT-09 PRC-11
    #[test]
    fn the_embedded_catalogue_loads() {
        let app = App::new(Arc::new(TestPlatform::default()), AppConfig::default());
        assert!(app.failures().is_empty(), "{:?}", app.failures());
        assert_eq!((app.catalogue().major, app.catalogue().minor), (1, 0));
    }
}
