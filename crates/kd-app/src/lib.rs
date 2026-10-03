//! kd-app: the app object: the frame loop, the simulation, I/O and view-builder threads, snapshot hand-off, input
//! and settings (A2.2, A2.4, A4). It lives on the GL thread; both shells drive it the same way.
//! It draws the renderer's test card each frame, keeps a panicking frame from taking the app down (A3.8), and
//! runs the self-check (A15.4): the shaders, the GL version, and from α00b the core's bits against the cloud's.
//!
//! Implements PRC-11 and RES-05, see A15.4 and A15.9: the self-check on a build's first start, the core's bits
//! among its checks.

pub mod json;
pub mod selfcheck;

use std::panic::{self, AssertUnwindSafe};
use std::sync::Arc;

use kd_data::Catalogue;
use kd_render::passes::scene::card;
use kd_render::{ART_SCALE, Frame, Renderer};
use kd_ui::strip::Colours;
use kd_ui::{Ui, UiAction};
use kd_view::{FontAtlas, InputEvent, Insets, UiDrawList, UiItem};

pub use json::requests_json;

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

/// The eight hours the strip steps through until the clock runs them (α03a), at 21° N on the spring equinox
/// (A11.4): their names and their hours.
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
/// The latitude the hours are seen from.
pub const HOURS_LAT: f32 = 21.0;
/// Late afternoon, the hour the app opens on.
pub const FIRST_HOUR: usize = 4;

/// The sky at one of `HOURS`: spring day 1, the equinox, at that hour (A3.7).
pub fn sky_at(hour: usize) -> kd_view::SkyView {
    let (_, h) = HOURS[hour % HOURS.len()];
    let t = kd_core::time::GameTime((h * 3600.0) as u64);
    kd_render::light::sky_view(&kd_core::sky::sun_moon(t, HOURS_LAT, 0.0, &kd_core::sky::Sky::FIRST))
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
}

/// The golden scenes of A11.12, drawn with time frozen.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Golden {
    /// The whole light card, the block turned 30°.
    LightCard,
    /// The block alone, filling the screen, turned 30°.
    Block,
}

impl Golden {
    pub fn named(name: &str) -> Option<Golden> {
        match name {
            "light-card" => Some(Golden::LightCard),
            "block" => Some(Golden::Block),
            _ => None,
        }
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
        app
    }

    pub fn handle(&mut self, m: AppMsg) {
        match m {
            AppMsg::Input(e) => {
                // Any touch shows the strip; a tap on it steps the hour (A12.2), until the clock runs time (α03a).
                if let Some(UiAction::StepHour) = self.ui.input(&e, ART_SCALE, self.screen_ui(), self.inset_ui()) {
                    self.set_hour(self.hour + 1);
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
        let card = self.card_frame(now_ns.saturating_sub(start));
        if let Some(r) = self.renderer.as_mut() {
            let f = Frame {
                count: self.frames,
                sky: sky_at(self.hour),
                card,
            };
            match panic::catch_unwind(AssertUnwindSafe(|| r.draw(&f, &list))) {
                Ok(_) => self.panics_in_row = 0,
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

    /// Test hook (A11.12): shows a golden scene with time frozen, or the live card again.
    pub fn set_golden(&mut self, golden: Option<Golden>) {
        self.golden = golden;
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

    /// The frame's UI (A12.1): the light card's look names over their swatches, and the strip while it shows; a
    /// golden scene has neither, so it never changes with the version or the clock.
    pub fn ui_list(&self, now_ns: u64) -> UiDrawList {
        let mut list = UiDrawList::default();
        if self.golden == Some(Golden::Block) {
            return list;
        }
        if let Some(cat) = &self.catalogue {
            for (row, look) in cat.looks.iter().take(card::MAX_LOOKS).enumerate() {
                // The cell's cap height starts two rows down, on the row's name line.
                let y = card::MARGIN + row as i32 * card::ROW - 2;
                list.items.push(UiItem::Text {
                    at: [card::MARGIN, y],
                    colour: self.colours.text,
                    fade: 1.0,
                    glyphs: kd_ui::font::run(&self.font, &look.name),
                });
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
