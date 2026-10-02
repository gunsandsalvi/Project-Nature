//! kd-app: the app object: frame loop, input, the self-check (A2.2, A2.4, A3.8, A15.4); implements PRC-11, PRE-22,
//! PRE-32 and PLT-02 in part.
//! In α01a it turns the golden cube as pixel art with the version strip over it; the simulation and I/O threads join
//! from α03a and α07a.

pub mod json;
pub mod selfcheck;

pub use json::{json_str, requests_json};
pub use kd_view::{InputEvent, InputKind, Insets};

use kd_data::Catalogue;
use kd_render::{Assets, DrawSettings, Frame, Renderer};
use kd_ui::{Font, Ui};
use kd_view::{CubeView, Snapshot};
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

/// Spin of the cube, radians a second of unpaused time.
const SPIN_PER_S: f32 = 0.6;
/// The cube's tilt, radians.
const PITCH: f32 = 0.5;
/// Radians of turn across a drag of the whole width (the mockup's `yawPerPx`).
const YAW_PER_WIDTH: f32 = 4.2;
/// The narrowest width the drag scale assumes, pixels (the mockup's).
const MIN_WIDTH: f32 = 240.0;
/// Momentum kept each frame after a release (the mockup's `step`).
const MOMENTUM_KEEP: f32 = 0.88;
/// Momentum under this, radians a frame, stops.
const MOMENTUM_STOP: f32 = 0.0004;
/// A release counts as moving when the last move was this recent, ns (the mockup's 90 ms).
const RELEASE_WINDOW_NS: u64 = 90_000_000;
/// Smallest release speed that keeps turning, radians a frame (the mockup's).
const RELEASE_MIN: f32 = 0.0008;
/// Largest momentum, radians a frame (the mockup's).
const MOMENTUM_MAX: f32 = 0.08;

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
    golden: Option<CubeView>,
    gl_info: String,
    fail: Vec<String>,
    reported: bool,
    frames: u64,
    w: u32,
    h: u32,
    paused: bool,
    spin_s: f32,
    last_ns: Option<u64>,
    drag_yaw: f32,
    pointer: Option<i32>,
    last_x: f32,
    last_move_ns: u64,
    vel: f32,
    momentum: f32,
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
            gl_info: String::new(),
            fail,
            reported: false,
            frames: 0,
            w: 1,
            h: 1,
            paused: false,
            spin_s: 0.0,
            last_ns: None,
            drag_yaw: 0.0,
            pointer: None,
            last_x: 0.0,
            last_move_ns: 0,
            vel: 0.0,
            momentum: 0.0,
        }
    }

    /// A GL context is ready: (re)builds the renderer; a failure is remembered as `shader: <log>`.
    pub fn gl_ready(&mut self, gl: glow::Context) {
        match Renderer::new(gl, &self.catalogue, &self.assets) {
            Ok(mut r) => {
                self.gl_info = r.gl_info();
                r.resize(self.w, self.h, SCALE);
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
    }

    /// The next message from the shell.
    pub fn handle(&mut self, m: AppMsg) {
        match m {
            AppMsg::Input(e) => {
                // A touch on the UI belongs to it; the cube never sees it (A12.2).
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

    fn input(&mut self, e: InputEvent) {
        let per_px = YAW_PER_WIDTH / kd_core::num::max(MIN_WIDTH, self.w as f32);
        match e.kind {
            InputKind::Down => {
                if self.pointer.is_none() {
                    self.pointer = Some(e.pointer);
                    self.last_x = e.x;
                    self.vel = 0.0;
                    self.momentum = 0.0;
                    self.last_move_ns = e.t_ns;
                } else {
                    // A second finger: no turn until all lift (pinch is α01b's).
                    self.pointer = Some(-1);
                }
            }
            InputKind::Move if self.pointer == Some(e.pointer) => {
                let d = -(e.x - self.last_x) * per_px;
                self.last_x = e.x;
                self.drag_yaw += d;
                let dt_ms = kd_core::num::max(8.0, e.t_ns.saturating_sub(self.last_move_ns) as f32 / 1e6);
                self.last_move_ns = e.t_ns;
                self.vel += (d * (16.0 / dt_ms) - self.vel) * 0.5;
            }
            InputKind::Move => {}
            InputKind::Up if self.pointer == Some(e.pointer) => {
                self.pointer = None;
                if e.t_ns.saturating_sub(self.last_move_ns) < RELEASE_WINDOW_NS && self.vel.abs() > RELEASE_MIN {
                    self.momentum = self.vel.clamp(-MOMENTUM_MAX, MOMENTUM_MAX);
                }
            }
            InputKind::Up | InputKind::Cancel => {
                self.pointer = None;
            }
        }
    }

    /// The cube's current turn, radians (the web test hook).
    pub fn yaw(&self) -> f32 {
        match self.golden {
            Some(c) => c.yaw,
            None => self.spin_s * SPIN_PER_S + self.drag_yaw,
        }
    }

    /// Freezes time and the drag and shows a fixed golden scene (A11.12, A12.4): `cube`, at yaw 0.6 and pitch 0.5 on
    /// dusk's row, with no strip. Returns whether the scene exists.
    pub fn golden(&mut self, name: &str) -> bool {
        match name {
            "cube" => {
                self.golden = Some(CubeView { yaw: 0.6, pitch: 0.5 });
                true
            }
            _ => false,
        }
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
                self.spin_s += now_ns.saturating_sub(last) as f32 / 1e9;
            }
            self.last_ns = Some(now_ns);
        }
        if self.momentum != 0.0 {
            self.drag_yaw += self.momentum;
            self.momentum *= MOMENTUM_KEEP;
            if self.momentum.abs() < MOMENTUM_STOP {
                self.momentum = 0.0;
            }
        }
        let (hz, gl_ms) = self.ring.figures();
        let ui = if self.golden.is_some() {
            kd_view::UiDrawList::default()
        } else {
            self.ui.build(now_ns as f64 / 1e9, build_line(), hz, gl_ms)
        };
        let f = Frame {
            real_s: f64::from(self.spin_s),
            palette_row: self.palette_row() as f32,
            set: DrawSettings::default(),
        };
        let pitch = self.golden.map_or(PITCH, |c| c.pitch);
        let snap = Snapshot {
            cube: Some(CubeView { yaw: self.yaw(), pitch }),
        };
        if let Some(r) = &mut self.renderer {
            r.draw(&f, &snap, &ui);
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
            y: 0.0,
            t_ns: t_ms * 1_000_000,
        })
    }

    // checks: PRC-11
    #[test]
    fn drag_turns_and_momentum_fades() {
        let p = Arc::new(TestPlatform::default());
        let mut app = App::new(p.clone(), AppConfig::default());
        app.resize(412, 860);
        app.handle(touch(InputKind::Down, 300.0, 0));
        for i in 1..=10 {
            app.handle(touch(InputKind::Move, 300.0 - 20.0 * i as f32, 16 * i));
        }
        let after_drag = app.yaw();
        assert!((after_drag - 200.0 * 4.2 / 412.0).abs() < 1e-4, "{after_drag}");
        app.handle(touch(InputKind::Up, 100.0, 170));
        app.frame(0);
        assert!(app.yaw() > after_drag, "keeps turning after release");
        for _ in 0..200 {
            app.frame(0);
        }
        let rest = app.yaw();
        app.frame(0);
        assert_eq!(app.yaw(), rest, "momentum fades to rest");
        assert!(
            p.out.lock().unwrap().is_empty(),
            "core checks pass, so no self-check box"
        );
    }

    // checks: PRC-11
    #[test]
    fn pause_stops_the_spin() {
        let mut app = App::new(Arc::new(TestPlatform::default()), AppConfig::default());
        app.frame(0);
        app.frame(1_000_000_000);
        assert!((app.yaw() - 0.6).abs() < 1e-6);
        app.handle(AppMsg::Pause);
        app.frame(5_000_000_000);
        assert!((app.yaw() - 0.6).abs() < 1e-6);
        app.handle(AppMsg::Resume);
        app.frame(6_000_000_000);
        app.frame(7_000_000_000);
        assert!((app.yaw() - 1.2).abs() < 1e-6);
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
