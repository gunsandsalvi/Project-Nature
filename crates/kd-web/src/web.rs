use std::sync::{Arc, Mutex};

use kd_app::{App, AppConfig, AppMsg, Platform, Request};
use kd_view::{InputEvent, InputKind};
use wasm_bindgen::JsCast;
use wasm_bindgen::prelude::*;
use web_sys::{HtmlCanvasElement, WebGl2RenderingContext};

/// The app's platform in a browser: `performance.now()` and an outbox `take_requests` drains. It keeps no
/// JavaScript object, so it is `Send` and `Sync` as the app asks.
#[derive(Default)]
struct WebPlatform {
    outbox: Mutex<Vec<Request>>,
}

impl WebPlatform {
    fn take(&self) -> Vec<Request> {
        std::mem::take(&mut *self.outbox.lock().unwrap_or_else(|e| e.into_inner()))
    }
}

impl Platform for WebPlatform {
    fn now_ns(&self) -> u64 {
        web_sys::window()
            .and_then(|w| w.performance())
            .map(|p| (p.now() * 1e6) as u64)
            .unwrap_or(0)
    }

    fn post(&self, r: Request) {
        self.outbox.lock().unwrap_or_else(|e| e.into_inner()).push(r);
    }
}

/// A panic aborts the instance, since `wasm32-unknown-unknown` cannot unwind: the hook logs it to the console and
/// writes it in the page's status line, so the game never stops without a word (A3.8).
#[wasm_bindgen(start)]
pub fn start() {
    std::panic::set_hook(Box::new(|info| {
        console_error_panic_hook::hook(info);
        if let Some(line) = web_sys::window()
            .and_then(|w| w.document())
            .and_then(|d| d.get_element_by_id("status"))
        {
            line.set_text_content(Some(&format!("Kindling stopped: {info}")));
        }
    }));
}

/// The build's version line.
#[wasm_bindgen]
pub fn build_line() -> String {
    kd_app::build_line().to_string()
}

#[wasm_bindgen]
pub struct WebApp {
    app: App,
    platform: Arc<WebPlatform>,
    canvas: HtmlCanvasElement,
}

#[wasm_bindgen]
impl WebApp {
    /// The app on `canvas`'s WebGL2 context with A2.6's attributes; `files` holds saves from α07a.
    #[wasm_bindgen(constructor)]
    pub fn new(canvas: HtmlCanvasElement, files: js_sys::Map) -> Result<WebApp, JsValue> {
        let _ = files;
        let platform = Arc::new(WebPlatform::default());
        let device = web_sys::window()
            .and_then(|w| w.navigator().user_agent().ok())
            .unwrap_or_default();
        let mut app = App::new(platform.clone(), AppConfig { device });
        let attributes = js_sys::Object::new();
        for (key, value) in [
            ("alpha", JsValue::FALSE),
            ("antialias", JsValue::FALSE),
            ("depth", JsValue::FALSE),
            ("stencil", JsValue::FALSE),
            ("powerPreference", JsValue::from_str("high-performance")),
        ] {
            js_sys::Reflect::set(&attributes, &JsValue::from_str(key), &value)?;
        }
        let context = canvas
            .get_context_with_context_options("webgl2", &attributes)?
            .ok_or_else(|| JsValue::from_str("this browser gives no WebGL2 context"))?
            .dyn_into::<WebGl2RenderingContext>()?;
        app.gl_ready(glow::Context::from_webgl2_context(context));
        Ok(WebApp { app, platform, canvas })
    }

    /// One frame: an art pixel moves on a whole number of device pixels whatever the time (`PRE-22`).
    pub fn frame(&mut self, now_ms: f64) {
        self.app.frame(ms_to_ns(now_ms));
    }

    /// A pointer event, positions in device pixels from the canvas's top-left; `kind` 0 down, 1 move, 2 up,
    /// 3 cancel (A12.2).
    pub fn pointer(&mut self, kind: u8, id: i32, x: f32, y: f32, t_ms: f64) {
        let kind = match kind {
            0 => InputKind::Down,
            1 => InputKind::Move,
            2 => InputKind::Up,
            _ => InputKind::Cancel,
        };
        let t_ns = ms_to_ns(t_ms);
        self.app.handle(AppMsg::Input(InputEvent {
            kind,
            pointer: id,
            x,
            y,
            t_ns,
        }));
    }

    /// The canvas's size in device pixels, exactly as the browser reports it, so an art pixel is exactly 4 device
    /// pixels at any screen scale (A2.6).
    pub fn resize(&mut self, w_px: u32, h_px: u32) {
        self.canvas.set_width(w_px);
        self.canvas.set_height(h_px);
        self.app.resize(w_px, h_px);
    }

    pub fn pause(&mut self) {
        self.app.handle(AppMsg::Pause);
    }

    pub fn resume(&mut self) {
        self.app.handle(AppMsg::Resume);
    }

    /// The requests posted since the last call, as A2.5's JSON array, or nothing.
    pub fn take_requests(&mut self) -> Option<String> {
        let requests = self.platform.take();
        (!requests.is_empty()).then(|| kd_app::requests_json(&requests))
    }

    /// Test hook (A12.4): whether the next frame draws.
    pub fn ready(&self) -> bool {
        self.app.ready()
    }

    /// Test hook (A12.4): frames drawn so far.
    pub fn frames(&self) -> f64 {
        self.app.frames() as f64
    }

    /// Test hook (A12.4): the art target's width and height, or nothing before the first size.
    pub fn art_size(&self) -> Vec<u32> {
        self.app.art_size().map(|a| a.to_vec()).unwrap_or_default()
    }

    /// Test hook (A15.9 item 5): the core's probes made in this browser and hashed, in `hashes.txt`'s form, for
    /// the smoke test to compare with the cloud's.
    pub fn core_hashes(&self) -> String {
        kd_core::bits::hashes_text(&kd_core::bits::probes())
    }

    /// Test hook (A11.13 rule 2): the probe scene's steps, `{"gpu":[…],"twins":[…]}`, or `null` before the renderer.
    pub fn probe(&mut self) -> String {
        match self.app.probe() {
            Some((gpu, twins)) => {
                let list = |v: &[u8]| v.iter().map(u8::to_string).collect::<Vec<_>>().join(",");
                format!("{{\"gpu\":[{}],\"twins\":[{}]}}", list(&gpu), list(&twins))
            }
            None => "null".to_string(),
        }
    }

    /// Test hook (A11.12): lights the frame at one of the app's hours, 0 dawn to 7 night.
    pub fn set_hour(&mut self, hour: u32) {
        self.app.set_hour(hour as usize);
    }

    /// Test hook (A11.12): the palette row in use, as `rrggbb` words, or an empty string before the first frame.
    pub fn palette(&self) -> String {
        self.app.palette_hex().unwrap_or_default()
    }

    /// Test hook (A11.12): a golden scene with time frozen and no strip (`light-card`, `block`, `valley-camp`,
    /// `valley-close` or `valley-near`), or the live ground for any other name; whether the name was a golden's.
    pub fn golden(&mut self, name: &str) -> bool {
        let g = kd_app::Golden::named(name);
        self.app.set_golden(g);
        g.is_some()
    }

    /// Test hook (A12.4): where the camera looks and what the last frame's view was, as JSON: the target in
    /// ticks, the heading and the zoom; the art pixel in metres, the upscale's shift, and the art target's corner
    /// and the block in the world's grid of art pixels.
    pub fn camera(&self) -> String {
        let c = self.app.camera();
        let view = self.app.view().map_or("null".to_string(), |v| {
            format!(
                "{{\"texel\":{},\"off\":[{},{}],\"corner\":[{},{}],\"block\":[{},{}],\"origin\":[{},{}]}}",
                v.texel, v.off[0], v.off[1], v.corner[0], v.corner[1], v.block[0], v.block[1], v.origin.x, v.origin.y
            )
        });
        format!(
            "{{\"target\":[{},{},{}],\"yaw\":{},\"zoom\":{},\"view\":{view}}}",
            c.target.x, c.target.y, c.target.z, c.yaw, c.zoom
        )
    }

    /// Test hook (A15.10): the ground's times and size, as JSON: making the demo area, the longest frame's area
    /// uploads, handing the area to the renderer with its sky field, and the longest frame's sun field work, in
    /// milliseconds, and the last frame's ground triangles and the stones and tufts it handed the GPU.
    pub fn timings(&self) -> String {
        let (upload_ns, triangles, items) = self.app.ground_stats();
        let (insert_ns, field_ns) = self.app.field_stats();
        format!(
            "{{\"demo_ms\":{},\"upload_ms\":{},\"insert_ms\":{},\"field_ms\":{},\"triangles\":{triangles},\"items\":{items}}}",
            self.app.demo_ns() as f64 / 1e6,
            upload_ns as f64 / 1e6,
            insert_ns as f64 / 1e6,
            field_ns as f64 / 1e6
        )
    }

    /// Test hook (A12.4): the ground under a point of the canvas in device pixels, as a position in ticks.
    pub fn ground_at(&self, x: f32, y: f32) -> Vec<i32> {
        self.app
            .ground_at([x, y])
            .map(|p| vec![p.x, p.y, p.z])
            .unwrap_or_default()
    }

    /// Test hook (A12.4): where a position in ticks shows on the canvas, in device pixels.
    pub fn screen_of(&self, x: i32, y: i32, z: i32) -> Vec<f64> {
        self.app
            .screen_of(kd_core::geo::Pos { x, y, z })
            .map(|p| p.to_vec())
            .unwrap_or_default()
    }

    /// Test hook (A12.4): the next frame makes every tile of coarse ground its view needs and finishes every light
    /// field, so its picture is whole.
    pub fn settle(&mut self) {
        self.app.settle();
    }

    /// Test hook (A12.4): points the camera at a target in ticks, with a heading in radians and a zoom.
    pub fn aim(&mut self, x: i32, y: i32, z: i32, yaw: f32, zoom: f32) {
        self.app.set_camera(kd_view::CameraPose {
            target: kd_core::geo::Pos { x, y, z },
            yaw,
            zoom,
        });
    }

    /// Test hook (A11.10): a slow camera motion's frames, each against the last, as
    /// `{"crawl":[…],"changed":[…],"pixels":n}`: `motion` is `turn` (degrees a frame), `zoom` (a share of the art
    /// pixel a frame) or `pan` (art pixels a frame along the screen's right).
    pub fn crawl(&mut self, motion: &str, rate: f32, frames: u32, now_ms: f64) -> String {
        match kd_app::Motion::named(motion) {
            Some(m) => kd_app::counts_json(&self.app.crawl(m, rate, frames, ms_to_ns(now_ms))),
            None => "null".to_string(),
        }
    }

    /// Test hook (A11.12): what each step of the zoom changes, from `from` to `to` by `step`, as `crawl` gives it.
    pub fn zoom_strip(&mut self, from: f32, to: f32, step: f32, now_ms: f64) -> String {
        kd_app::counts_json(&self.app.zoom_strip(from, to, step, ms_to_ns(now_ms)))
    }

    /// Test hook (A3.8): a panic, for the smoke test to see its message in the status line.
    pub fn crash(&self) {
        panic!("a test panic, asked for by the smoke test");
    }
}

/// A time from `performance.now()`, in milliseconds, as the app's nanoseconds; never before 0.
fn ms_to_ns(ms: f64) -> u64 {
    (ms.max(0.0) * 1e6) as u64
}
