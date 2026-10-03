//! kd-render: the renderer (A11), organised as A11.13 sets it: `gl` is the one layer that calls OpenGL, `passes`
//! holds each pass with its program and targets, and every shader shares `shaders/lib.glsl` and the constants
//! generated from Rust. It runs on the GL thread only, through `glow`: OpenGL ES 3.0 on the phone, WebGL2 on the web.
//! α00 draws a test card at art resolution and enlarges it so an art pixel is 4 × 4 screen pixels (A11.2).
//!
//! Implements PRE-22 and PLT-01, see A11.2 and A11.13: an art pixel exactly 4 × 4 screen pixels, drawn with OpenGL
//! ES 3.0 on the phone.

pub mod frame;
pub mod gl;
pub mod light;
pub mod looks;
pub mod passes;
pub mod shaders;

use std::fmt;

use kd_data::Catalogue;
use passes::scene::{ArtView, ScenePass};
use passes::upscale::UpscalePass;

use frame::Lighting;
use gl::{Format, Texture};
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

/// One frame's inputs (A11.1); the camera, the light and the display time join with their alphas.
#[derive(Clone, Copy, Debug, Default)]
pub struct Frame {
    /// Frames drawn since the app started: the test card's bar moves one art pixel a frame.
    pub count: u64,
    /// The self-check's core bits, shown on the test card: none before the check, then whether they equal the
    /// cloud's (A15.9 item 5).
    pub core_bits: Option<bool>,
    /// The sky the frame is lit by (A11.4).
    pub sky: kd_view::SkyView,
}

/// What a frame drew.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct FrameStats {
    pub art: [u32; 2],
}

pub struct Renderer {
    gl: glow::Context,
    info: String,
    vao: glow::VertexArray,
    scene: ScenePass,
    upscale: UpscalePass,
    view: Option<ArtView>,
    cat: Catalogue,
    layout: Layout,
    /// The light and palette of the last frame (A11.13 rule 4), none before the first.
    lighting: Option<Lighting>,
    palette_tex: Texture,
    tables_tex: Texture,
}

impl Renderer {
    /// Builds every program, the shared vertex array and the palette's textures from the catalogue; a shader that
    /// fails names itself and the driver's log, which the self-check reports (A15.4).
    pub fn new(gl: glow::Context, cat: &Catalogue) -> Result<Renderer, RenderError> {
        let info = gl::info(&gl);
        let layout = Layout::new(cat).map_err(RenderError::Gl)?;
        let vao = gl::empty_vertex_array(&gl)?;
        let scene = ScenePass::new(&gl)?;
        let upscale = UpscalePass::new(&gl)?;
        let palette_tex = Texture::new(&gl, Format::Rgba8, PALETTE_SIZE as u32, 1, None)?;
        let tables_tex = Texture::new(&gl, Format::R8, PALETTE_SIZE as u32, TABLE_ROWS as u32, None)?;
        Ok(Renderer {
            gl,
            info,
            vao,
            scene,
            upscale,
            view: None,
            cat: cat.clone(),
            layout,
            lighting: None,
            palette_tex,
            tables_tex,
        })
    }

    /// The frame's light and palette, uploading the palette's textures when its row or tables changed (A11.3).
    fn light_frame(&mut self, f: &Frame) {
        let upload = match &mut self.lighting {
            Some(l) => l.follow(&self.cat, &self.layout, &f.sky),
            None => {
                self.lighting = Some(Lighting::new(&self.cat, &self.layout, &f.sky));
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
        self.view = Some(view);
        Ok(())
    }

    pub fn draw(&mut self, f: &Frame) -> FrameStats {
        let Some(view) = self.view else {
            return FrameStats::default();
        };
        self.light_frame(f);
        let bar_x = (f.count % u64::from(view.visible[0].max(1))) as i32;
        let core = match f.core_bits {
            None => 0,
            Some(true) => 1,
            Some(false) => 2,
        };
        self.scene.draw(&self.gl, &view, bar_x, core, self.vao);
        if let Some(t) = &self.scene.target {
            self.upscale.draw(&self.gl, &view, &t.colours[0], self.vao);
        }
        FrameStats { art: view.art }
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
    use kd_data::compile::{Source, compile};

    /// The repository's catalogue, compiled as the app's build script compiles it.
    pub(crate) fn catalogue() -> Catalogue {
        let src = |p: &str, t: &str| Source {
            path: p.into(),
            text: t.into(),
        };
        let sources = [
            src("data/VERSION.toml", include_str!("../../../data/VERSION.toml")),
            src("data/ids.lock", include_str!("../../../data/ids.lock")),
            src("data/INDEX.md", include_str!("../../../data/INDEX.md")),
            src(
                "data/palette/colours.md",
                include_str!("../../../data/palette/colours.md"),
            ),
            src("data/palette/looks.md", include_str!("../../../data/palette/looks.md")),
            src("data/palette/light.md", include_str!("../../../data/palette/light.md")),
        ];
        compile(&sources, false).expect("the catalogue compiles").catalogue
    }
}
