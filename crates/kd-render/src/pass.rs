//! Shader programs and the passes of A11.2: pass 2 (the scene into the art target, cleared to `void`), pass 3 (post:
//! outlines, rims, the palette row) and pass 5 (upscale to the window) here; pass 1 is in `shadow`, pass 4's slot in
//! `crawl` (its output target here), pass 6 (the UI) in `ui`.
//! Implements `PRE-01`, `PRE-21` and `PRE-22` in part, see A11.2.

use crate::RenderError;
use crate::target::{Target, bind_window};
use glow::HasContext;

/// The head of every shader: GLSL ES 3.00 at high precision, with derivatives (core in ES 3.00: the mockup's
/// `HAS_DERIV`).
const HEAD: &str = "#version 300 es\nprecision highp float;\n#define HAS_DERIV 1\n";
/// The mockup's shared fragment code, prepended to scene shaders.
pub const COMMON: &str = include_str!("../shaders/common.glsl");

/// A linked program; uniforms are looked up by name.
pub struct Prog {
    pub p: glow::Program,
}

#[allow(unsafe_code)]
unsafe fn compile(gl: &glow::Context, kind: u32, src: &str) -> Result<glow::Shader, RenderError> {
    unsafe {
        let s = gl.create_shader(kind).map_err(RenderError::Resource)?;
        gl.shader_source(s, src);
        gl.compile_shader(s);
        if !gl.get_shader_compile_status(s) {
            let log = gl.get_shader_info_log(s);
            gl.delete_shader(s);
            return Err(RenderError::Shader(log));
        }
        Ok(s)
    }
}

impl Prog {
    /// Compiles and links `vs` and `fs` after the head and `defines` (with the shared code before a scene shader's
    /// body when `scene`), binding the attributes to the given locations.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn new(
        gl: &glow::Context,
        vs: &str,
        fs: &str,
        defines: &str,
        scene: bool,
        attribs: &[(u32, &str)],
    ) -> Result<Prog, RenderError> {
        unsafe {
            let vs = compile(gl, glow::VERTEX_SHADER, &format!("{HEAD}{defines}{vs}"))?;
            let common = if scene { COMMON } else { "" };
            let fs = compile(gl, glow::FRAGMENT_SHADER, &format!("{HEAD}{defines}{common}{fs}"))?;
            let p = gl.create_program().map_err(RenderError::Resource)?;
            gl.attach_shader(p, vs);
            gl.attach_shader(p, fs);
            for (loc, name) in attribs {
                gl.bind_attrib_location(p, *loc, name);
            }
            gl.link_program(p);
            gl.delete_shader(vs);
            gl.delete_shader(fs);
            if !gl.get_program_link_status(p) {
                return Err(RenderError::Link(gl.get_program_info_log(p)));
            }
            Ok(Prog { p })
        }
    }

    /// Uses the program.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn bind(&self, gl: &glow::Context) {
        unsafe { gl.use_program(Some(self.p)) }
    }

    /// Sets a float uniform of 1 to 4 parts; an unknown or optimised-away name does nothing.
    ///
    /// # Safety
    /// Needs a current GL context on this thread, with this program in use.
    #[allow(unsafe_code)]
    pub unsafe fn f(&self, gl: &glow::Context, name: &str, v: &[f32]) {
        unsafe {
            let Some(l) = gl.get_uniform_location(self.p, name) else {
                return;
            };
            match v.len() {
                1 => gl.uniform_1_f32(Some(&l), v[0]),
                2 => gl.uniform_2_f32(Some(&l), v[0], v[1]),
                3 => gl.uniform_3_f32(Some(&l), v[0], v[1], v[2]),
                4 => gl.uniform_4_f32(Some(&l), v[0], v[1], v[2], v[3]),
                16 => gl.uniform_matrix_4_f32_slice(Some(&l), false, v),
                _ => {}
            }
        }
    }

    /// Binds a texture to a unit and points a sampler uniform at it.
    ///
    /// # Safety
    /// Needs a current GL context on this thread, with this program in use.
    #[allow(unsafe_code)]
    pub unsafe fn tex(&self, gl: &glow::Context, name: &str, unit: u32, t: Option<glow::Texture>) {
        unsafe {
            gl.active_texture(glow::TEXTURE0 + unit);
            gl.bind_texture(glow::TEXTURE_2D, t);
            if let Some(l) = gl.get_uniform_location(self.p, name) {
                gl.uniform_1_i32(Some(&l), unit as i32);
            }
        }
    }
}

/// A full-screen pair of triangles (`X.quad`), attribute 0.
pub struct Quad {
    vao: glow::VertexArray,
}

impl Quad {
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn new(gl: &glow::Context) -> Result<Quad, RenderError> {
        unsafe {
            let vao = gl.create_vertex_array().map_err(RenderError::Resource)?;
            gl.bind_vertex_array(Some(vao));
            let vbo = gl.create_buffer().map_err(RenderError::Resource)?;
            gl.bind_buffer(glow::ARRAY_BUFFER, Some(vbo));
            let v: [f32; 12] = [-1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0];
            let b: Vec<u8> = v.iter().flat_map(|f| f.to_le_bytes()).collect();
            gl.buffer_data_u8_slice(glow::ARRAY_BUFFER, &b, glow::STATIC_DRAW);
            gl.vertex_attrib_pointer_f32(0, 2, glow::FLOAT, false, 8, 0);
            gl.enable_vertex_attrib_array(0);
            gl.bind_vertex_array(None);
            Ok(Quad { vao })
        }
    }

    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn draw(&self, gl: &glow::Context) {
        unsafe {
            gl.bind_vertex_array(Some(self.vao));
            gl.draw_arrays(glow::TRIANGLES, 0, 6);
            gl.bind_vertex_array(None);
        }
    }
}

/// What the post pass needs beyond its textures.
pub struct PostParams {
    pub sun_scr: [f32; 2],
    pub fire_scr: [f32; 3],
    pub outlines: bool,
    pub palette_row: f32,
    pub depth_m: f32,
    pub texel: f32,
}

/// The art and post targets with the post and upscale programs.
pub struct Passes {
    pub scene: Target,
    pub post: Target,
    /// Pass 4's output, which the upscale reads (A11.10).
    pub crawl: Target,
    quad: Quad,
    post_prog: Prog,
    up_prog: Prog,
}

impl Passes {
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn new(gl: &glow::Context, defines: &str) -> Result<Passes, RenderError> {
        unsafe {
            let quad_vs = include_str!("../shaders/quad.vert");
            Ok(Passes {
                scene: Target::new(gl, 4, 4, true)?,
                post: Target::new(gl, 4, 4, false)?,
                crawl: Target::new(gl, 4, 4, false)?,
                quad: Quad::new(gl)?,
                post_prog: Prog::new(
                    gl,
                    quad_vs,
                    include_str!("../shaders/post.frag"),
                    defines,
                    false,
                    &[(0, "aPos")],
                )?,
                up_prog: Prog::new(
                    gl,
                    quad_vs,
                    include_str!("../shaders/upscale.frag"),
                    "",
                    false,
                    &[(0, "aPos")],
                )?,
            })
        }
    }

    /// Both targets at the art size.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn resize(&mut self, gl: &glow::Context, w: u32, h: u32) -> Result<(), RenderError> {
        unsafe {
            self.scene.resize(gl, w, h)?;
            self.post.resize(gl, w, h)?;
            self.crawl.resize(gl, w, h)
        }
    }

    /// Pass 2's start: the art target bound, colour 0 cleared to `void` (index 0) at the farthest depth.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn begin_scene(&self, gl: &glow::Context) {
        unsafe {
            self.scene.bind(gl);
            gl.disable(glow::BLEND);
            gl.clear_color(0.0, 0.0, 1.0, 1.0);
            gl.clear_depth_f32(1.0);
            gl.enable(glow::DEPTH_TEST);
            gl.depth_func(glow::LEQUAL);
            gl.depth_mask(true);
            gl.clear(glow::COLOR_BUFFER_BIT | glow::DEPTH_BUFFER_BIT);
        }
    }

    /// Pass 3: outlines, rims and the palette row, from the art target into the post target.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn post(&self, gl: &glow::Context, pal: glow::Texture, luts: glow::Texture, p: &PostParams) {
        unsafe {
            gl.disable(glow::DEPTH_TEST);
            gl.disable(glow::CULL_FACE);
            self.post.bind(gl);
            let g = &self.post_prog;
            g.bind(gl);
            g.tex(gl, "uImg", 0, self.scene.tex);
            g.tex(gl, "uPal", 1, Some(pal));
            g.tex(gl, "uLuts", 2, Some(luts));
            g.f(gl, "uRes", &[self.scene.w as f32, self.scene.h as f32]);
            g.f(gl, "uSunScr", &p.sun_scr);
            g.f(gl, "uFireScr", &p.fire_scr);
            g.f(gl, "uOutline", &[if p.outlines { 1.0 } else { 0.0 }]);
            g.f(gl, "uPalRow", &[p.palette_row]);
            g.f(gl, "uDepthM", &[p.depth_m]);
            g.f(gl, "uTexel", &[p.texel]);
            self.quad.draw(gl);
        }
    }

    /// Pass 5: pass 4's output onto the window, `scale` screen pixels an art pixel, shifted by `off` art pixels.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn upscale(&self, gl: &glow::Context, win: (u32, u32), off: [f32; 2], scale: u32) {
        unsafe {
            bind_window(gl, win.0, win.1);
            let g = &self.up_prog;
            g.bind(gl);
            g.tex(gl, "uImg", 0, self.crawl.tex);
            g.f(gl, "uImgSize", &[self.crawl.w as f32, self.crawl.h as f32]);
            g.f(gl, "uOff", &off);
            g.f(gl, "uScale", &[scale as f32]);
            self.quad.draw(gl);
        }
    }
}

/// The upscale's shift in art pixels for a window of `w` by `h` at `s` with no camera yet (the mockup's
/// `computeCamera` with the target at the origin): the art target's centre on the window's centre.
pub fn offset(art: (u32, u32), w: u32, h: u32, s: u32) -> [f32; 2] {
    let s = s.max(1) as f32;
    [
        (art.0 / 2) as f32 - w as f32 / (2.0 * s),
        (art.1 / 2) as f32 - h as f32 / (2.0 * s),
    ]
}
