//! The web shell's exports (A2.6): one thread, so every call is direct.

use kd_app::{App, AppConfig, AppMsg, InputEvent, InputKind, Insets, Platform, Request};
use std::sync::{Arc, Mutex};
use wasm_bindgen::JsCast;
use wasm_bindgen::prelude::*;
use web_sys::{HtmlCanvasElement, WebGl2RenderingContext};

/// The web's `Platform` (A2.2): time from `performance.now()` (A2.4: no `Instant` on wasm32); it holds no JS object,
/// asking `web_sys::window()` each time, so it is `Send + Sync` without unsafe code.
#[derive(Default)]
struct WebPlatform {
    outbox: Mutex<Vec<Request>>,
}

impl Platform for WebPlatform {
    fn now_ns(&self) -> u64 {
        let ms = web_sys::window()
            .and_then(|w| w.performance())
            .map(|p| p.now())
            .unwrap_or(0.0);
        (ms * 1e6) as u64
    }
    fn post(&self, r: Request) {
        self.outbox.lock().unwrap_or_else(|e| e.into_inner()).push(r);
    }
}

#[wasm_bindgen(start)]
pub fn start() {
    console_error_panic_hook::set_once();
}

/// kd-core's stored draws and maths run in the browser: the wasm leg of the cross-target check (A3.1, `RES-05`).
#[wasm_bindgen]
pub fn core_check() -> bool {
    kd_core::selfcheck::core_check().is_empty()
}

/// The build's version line (A15.3).
#[wasm_bindgen]
pub fn build_line() -> String {
    kd_app::build_line().to_string()
}

/// The app on a canvas (A2.6).
#[wasm_bindgen]
pub struct WebApp {
    app: App,
    platform: Arc<WebPlatform>,
    canvas: HtmlCanvasElement,
}

#[wasm_bindgen]
impl WebApp {
    /// Gets a WebGL2 context (no alpha, antialiasing, depth or stencil) and starts the app; `files` is unused until
    /// α07a.
    #[wasm_bindgen(constructor)]
    pub fn new(canvas: HtmlCanvasElement, _files: js_sys::Map, dpr: f32) -> Result<WebApp, JsValue> {
        let opts = js_sys::Object::new();
        for (k, v) in [
            ("alpha", false),
            ("antialias", false),
            ("depth", false),
            ("stencil", false),
        ] {
            js_sys::Reflect::set(&opts, &k.into(), &v.into())?;
        }
        js_sys::Reflect::set(&opts, &"powerPreference".into(), &"high-performance".into())?;
        let ctx = canvas
            .get_context_with_context_options("webgl2", &opts)?
            .ok_or_else(|| JsValue::from_str("no WebGL2 context"))?
            .dyn_into::<WebGl2RenderingContext>()?;
        let platform = Arc::new(WebPlatform::default());
        let device = String::from("web");
        let mut app = App::new(platform.clone(), AppConfig { device });
        app.gl_ready(glow::Context::from_webgl2_context(ctx));
        let mut me = WebApp { app, platform, canvas };
        let (w, h) = (
            me.canvas.client_width().max(1) as u32,
            me.canvas.client_height().max(1) as u32,
        );
        me.resize(w, h, dpr);
        Ok(me)
    }

    /// One frame at `requestAnimationFrame`'s time.
    pub fn frame(&mut self, now_ms: f64) {
        self.app.frame((now_ms.max(0.0) * 1e6) as u64);
    }

    /// A pointer event in device pixels: kind 0 down, 1 move, 2 up, 3 cancel.
    pub fn pointer(&mut self, kind: u8, id: i32, x: f32, y: f32, t_ms: f64) {
        let kind = match kind {
            0 => InputKind::Down,
            1 => InputKind::Move,
            2 => InputKind::Up,
            _ => InputKind::Cancel,
        };
        let t_ns = (t_ms.max(0.0) * 1e6) as u64;
        self.app.handle(AppMsg::Input(InputEvent {
            kind,
            pointer: id,
            x,
            y,
            t_ns,
        }));
    }

    /// Sets the canvas's backing size to its CSS size × `dpr`.
    pub fn resize(&mut self, css_w: u32, css_h: u32, dpr: f32) {
        let (w, h) = (((css_w as f32) * dpr) as u32, ((css_h as f32) * dpr) as u32);
        self.canvas.set_width(w.max(1));
        self.canvas.set_height(h.max(1));
        self.app.resize(w.max(1), h.max(1));
    }

    pub fn pause(&mut self) {
        self.app.handle(AppMsg::Pause);
    }

    pub fn resume(&mut self) {
        self.app.handle(AppMsg::Resume);
    }

    /// The outbox as JSON, or nothing when empty.
    pub fn take_requests(&mut self) -> Option<String> {
        let mut out = self.platform.outbox.lock().unwrap_or_else(|e| e.into_inner());
        if out.is_empty() {
            return None;
        }
        let json = kd_app::requests_json(&out);
        out.clear();
        Some(json)
    }

    /// The page's safe-area insets in device pixels (A12.1: nothing under the insets).
    pub fn insets(&mut self, top: f32, right: f32, bottom: f32, left: f32) {
        self.app.handle(AppMsg::Insets(Insets {
            top,
            bottom,
            left,
            right,
        }));
    }

    /// The cube's turn, for the test hook.
    pub fn yaw(&self) -> f32 {
        self.app.yaw()
    }

    /// Shows a fixed golden scene, time and drag frozen (A11.12); false for an unknown name.
    pub fn golden(&mut self, name: &str) -> bool {
        self.app.golden(name)
    }

    /// The camera: x and y metres from the ground's corner, its height, turn and zoom, metres per art pixel, its
    /// place within an art pixel, and the floating origin's x and y from the ground's corner (the test hook's
    /// `camera()`).
    pub fn camera(&self) -> Vec<f32> {
        self.app.camera().to_vec()
    }

    /// Points the camera, at the ground's height when `z` is left out (the test hook's `camera(pose)`).
    pub fn set_camera(&mut self, x: f32, y: f32, z: Option<f32>, yaw: f32, zoom: f32) {
        self.app.set_camera(x, y, z, yaw, zoom);
    }

    /// Whether the frame after the ground's upload has drawn (the test hook's `ready()`).
    pub fn ready(&self) -> bool {
        self.app.ready()
    }

    /// B66's crawl count as JSON (the test hook's `crawl()`, A11.10); `null` for an unknown fix.
    pub fn crawl(&mut self, motion: &str, rate: f32, frames: u32, fix: &str, zoom: Option<f32>) -> Option<String> {
        self.app.crawl(motion, rate, frames, fix, zoom)
    }

    /// The GL thread's milliseconds a frame, averaged over the last frames (the bench file's `frame_ms_web`).
    pub fn gl_ms(&self) -> f32 {
        self.app.frame_figures().1
    }

    /// The current palette row's colours, RGB bytes, for the `palette only` check.
    pub fn palette_rgb(&self) -> Vec<u8> {
        self.app.palette_rgb()
    }
}
