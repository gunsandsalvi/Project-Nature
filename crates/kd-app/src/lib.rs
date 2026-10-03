//! kd-app: the app object: the frame loop, the simulation, I/O and view-builder threads, snapshot hand-off, input
//! and settings (A2.2, A2.4, A4). It lives on the GL thread; both shells drive it the same way.
//! α00 draws the renderer's test card each frame, keeps a panicking frame from taking the app down (A3.8), and
//! runs the first part of the self-check (A15.4).

pub mod json;
pub mod selfcheck;

use std::panic::{self, AssertUnwindSafe};
use std::sync::Arc;

use kd_render::{ART_SCALE, Frame, Renderer};
use kd_view::{InputEvent, Insets};

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

/// What the shell knows at start.
#[derive(Clone, Debug, Default)]
pub struct AppConfig {
    /// The device, for the self-check's report: maker, model and Android version, or the browser.
    pub device: String,
}

/// The build's version line, set by the build scripts (`KD_BUILD`), such as `a00 · 1000 · 4f2c9e1`.
pub fn build_line() -> &'static str {
    option_env!("KD_BUILD").unwrap_or("dev")
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
}

impl App {
    pub fn new(platform: Arc<dyn Platform>, cfg: AppConfig) -> App {
        let report = selfcheck::Report {
            build: build_line().to_string(),
            device: cfg.device.clone(),
            gl: String::new(),
            fail: Vec::new(),
        };
        App {
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
        }
    }

    pub fn handle(&mut self, m: AppMsg) {
        match m {
            AppMsg::Input(_) => {} // gestures arrive in α01b
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
        self.renderer = match Renderer::new(gl) {
            Ok(mut r) => {
                if let Some([w, h]) = self.size
                    && let Err(e) = r.resize(w, h, ART_SCALE)
                {
                    self.check_failed(e.to_string());
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
        let _ = now_ns; // time starts to matter with the clock (α03a)
        if let Some(r) = self.renderer.as_mut() {
            let f = Frame { count: self.frames };
            match panic::catch_unwind(AssertUnwindSafe(|| r.draw(&f))) {
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
