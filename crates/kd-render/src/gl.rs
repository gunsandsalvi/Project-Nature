//! The one thin GL layer (A11.13 rule 3): only this module calls `glow`.
//! Programs look their uniform locations up once at link; textures come in the formats the renderer uses, float
//! ones sampled nearest since WebGL2 cannot filter them; targets carry their attachments; each pass sets the whole
//! pipeline state it needs rather than inheriting it. Nothing here deletes on drop: after a lost context the old
//! handles name nothing, and deleting them could delete the new context's objects, so the renderer is simply
//! rebuilt from CPU state (A11.13 rule 5).

use glow::HasContext;

use crate::RenderError;

/// Texture units, one table for every program (A11.13 rule 6: the shaders take them as generated defines).
pub mod unit {
    /// The art target, read by the upscale pass.
    pub const ART: u32 = 0;
}

/// The texture formats the renderer uses (A11.13 rule 3).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Format {
    Rgba8,
    R8,
    R32F,
    Rg32F,
}

impl Format {
    /// The sized internal format.
    pub const fn internal(self) -> u32 {
        match self {
            Format::Rgba8 => glow::RGBA8,
            Format::R8 => glow::R8,
            Format::R32F => glow::R32F,
            Format::Rg32F => glow::RG32F,
        }
    }

    /// The format of the texels handed over.
    pub const fn format(self) -> u32 {
        match self {
            Format::Rgba8 => glow::RGBA,
            Format::R8 | Format::R32F => glow::RED,
            Format::Rg32F => glow::RG,
        }
    }

    /// The type of each channel handed over.
    pub const fn channel_type(self) -> u32 {
        match self {
            Format::Rgba8 | Format::R8 => glow::UNSIGNED_BYTE,
            Format::R32F | Format::Rg32F => glow::FLOAT,
        }
    }

    /// Bytes a texel.
    pub const fn bytes(self) -> usize {
        match self {
            Format::Rgba8 => 4,
            Format::R8 => 1,
            Format::R32F => 4,
            Format::Rg32F => 8,
        }
    }
}

/// A 2D texture, nearest-sampled and clamped.
pub struct Texture {
    pub handle: glow::Texture,
    pub format: Format,
    pub w: u32,
    pub h: u32,
}

impl Texture {
    /// A texture of `w` × `h` texels, filled from `data` (exactly w × h × bytes) or left undefined.
    pub fn new(
        gl: &glow::Context,
        format: Format,
        w: u32,
        h: u32,
        data: Option<&[u8]>,
    ) -> Result<Texture, RenderError> {
        if let Some(d) = data
            && d.len() != w as usize * h as usize * format.bytes()
        {
            return Err(RenderError::Gl(format!(
                "texture data of {} bytes for {w} x {h} {format:?}",
                d.len()
            )));
        }
        // SAFETY: plain GL calls on the current context; `data` was checked to hold exactly the texels named.
        unsafe {
            let handle = gl.create_texture().map_err(RenderError::Gl)?;
            gl.bind_texture(glow::TEXTURE_2D, Some(handle));
            for (p, v) in [
                (glow::TEXTURE_MIN_FILTER, glow::NEAREST),
                (glow::TEXTURE_MAG_FILTER, glow::NEAREST),
                (glow::TEXTURE_WRAP_S, glow::CLAMP_TO_EDGE),
                (glow::TEXTURE_WRAP_T, glow::CLAMP_TO_EDGE),
            ] {
                gl.tex_parameter_i32(glow::TEXTURE_2D, p, v as i32);
            }
            gl.pixel_store_i32(glow::UNPACK_ALIGNMENT, 1);
            gl.tex_image_2d(
                glow::TEXTURE_2D,
                0,
                format.internal() as i32,
                w as i32,
                h as i32,
                0,
                format.format(),
                format.channel_type(),
                glow::PixelUnpackData::Slice(data),
            );
            gl.bind_texture(glow::TEXTURE_2D, None);
            Ok(Texture { handle, format, w, h })
        }
    }

    /// Binds the texture to a unit of the table above.
    pub fn bind(&self, gl: &glow::Context, unit: u32) {
        // SAFETY: plain GL calls on the current context.
        unsafe {
            gl.active_texture(glow::TEXTURE0 + unit);
            gl.bind_texture(glow::TEXTURE_2D, Some(self.handle));
        }
    }

    pub fn delete(self, gl: &glow::Context) {
        // SAFETY: the texture belongs to this context and is not used again.
        unsafe { gl.delete_texture(self.handle) }
    }
}

/// A framebuffer with its colour textures and an optional depth-and-stencil buffer.
pub struct Target {
    pub framebuffer: glow::Framebuffer,
    pub colours: Vec<Texture>,
    pub depth: Option<glow::Renderbuffer>,
    pub w: u32,
    pub h: u32,
}

impl Target {
    pub fn new(gl: &glow::Context, w: u32, h: u32, colours: &[Format], depth: bool) -> Result<Target, RenderError> {
        let mut textures = Vec::with_capacity(colours.len());
        for &f in colours {
            textures.push(Texture::new(gl, f, w, h, None)?);
        }
        // SAFETY: plain GL calls on the current context, attaching textures and a buffer it just made.
        unsafe {
            let framebuffer = gl.create_framebuffer().map_err(RenderError::Gl)?;
            gl.bind_framebuffer(glow::FRAMEBUFFER, Some(framebuffer));
            let mut attachments = Vec::with_capacity(textures.len());
            for (i, t) in textures.iter().enumerate() {
                let a = glow::COLOR_ATTACHMENT0 + i as u32;
                gl.framebuffer_texture_2d(glow::FRAMEBUFFER, a, glow::TEXTURE_2D, Some(t.handle), 0);
                attachments.push(a);
            }
            gl.draw_buffers(&attachments);
            let depth = if depth {
                let rb = gl.create_renderbuffer().map_err(RenderError::Gl)?;
                gl.bind_renderbuffer(glow::RENDERBUFFER, Some(rb));
                gl.renderbuffer_storage(glow::RENDERBUFFER, glow::DEPTH24_STENCIL8, w as i32, h as i32);
                gl.framebuffer_renderbuffer(
                    glow::FRAMEBUFFER,
                    glow::DEPTH_STENCIL_ATTACHMENT,
                    glow::RENDERBUFFER,
                    Some(rb),
                );
                gl.bind_renderbuffer(glow::RENDERBUFFER, None);
                Some(rb)
            } else {
                None
            };
            let status = gl.check_framebuffer_status(glow::FRAMEBUFFER);
            gl.bind_framebuffer(glow::FRAMEBUFFER, None);
            if status != glow::FRAMEBUFFER_COMPLETE {
                return Err(RenderError::Target(format!(
                    "{w} x {h} framebuffer incomplete (status {status:#x})"
                )));
            }
            Ok(Target {
                framebuffer,
                colours: textures,
                depth,
                w,
                h,
            })
        }
    }

    pub fn bind(&self, gl: &glow::Context) {
        // SAFETY: plain GL call on the current context.
        unsafe { gl.bind_framebuffer(glow::FRAMEBUFFER, Some(self.framebuffer)) }
    }

    pub fn delete(self, gl: &glow::Context) {
        // SAFETY: the framebuffer and its attachments belong to this context and are not used again.
        unsafe {
            gl.delete_framebuffer(self.framebuffer);
            if let Some(rb) = self.depth {
                gl.delete_renderbuffer(rb);
            }
        }
        for t in self.colours {
            t.delete(gl);
        }
    }
}

/// Binds the window's own framebuffer.
pub fn bind_window(gl: &glow::Context) {
    // SAFETY: plain GL call on the current context.
    unsafe { gl.bind_framebuffer(glow::FRAMEBUFFER, None) }
}

/// A linked program; its passes look uniform locations up once, through `uniform`, and keep them.
pub struct Program {
    pub handle: glow::Program,
    pub name: &'static str,
}

impl Program {
    /// Compiles both stages from their full sources and links them; a failure carries the driver's log.
    pub fn new(gl: &glow::Context, name: &'static str, vertex: &str, fragment: &str) -> Result<Program, RenderError> {
        // SAFETY: plain GL calls on the current context; every object made here is deleted or kept.
        unsafe {
            let program = gl.create_program().map_err(RenderError::Gl)?;
            let mut shaders = Vec::with_capacity(2);
            for (stage, src) in [(glow::VERTEX_SHADER, vertex), (glow::FRAGMENT_SHADER, fragment)] {
                let s = gl.create_shader(stage).map_err(RenderError::Gl)?;
                gl.shader_source(s, src);
                gl.compile_shader(s);
                if !gl.get_shader_compile_status(s) {
                    let log = gl.get_shader_info_log(s);
                    gl.delete_shader(s);
                    for k in shaders {
                        gl.delete_shader(k);
                    }
                    gl.delete_program(program);
                    let which = if stage == glow::VERTEX_SHADER {
                        "vertex"
                    } else {
                        "fragment"
                    };
                    return Err(RenderError::Shader {
                        name: format!("{name} ({which})"),
                        log,
                    });
                }
                gl.attach_shader(program, s);
                shaders.push(s);
            }
            gl.link_program(program);
            let linked = gl.get_program_link_status(program);
            for s in shaders {
                gl.detach_shader(program, s);
                gl.delete_shader(s);
            }
            if !linked {
                let log = gl.get_program_info_log(program);
                gl.delete_program(program);
                return Err(RenderError::Link {
                    name: name.to_string(),
                    log,
                });
            }
            Ok(Program { handle: program, name })
        }
    }

    /// A uniform's location, looked up once when the pass is made.
    pub fn uniform(&self, gl: &glow::Context, name: &str) -> Option<glow::UniformLocation> {
        // SAFETY: plain GL call on the current context.
        unsafe { gl.get_uniform_location(self.handle, name) }
    }

    /// Points a sampler uniform at its texture unit, once.
    pub fn set_sampler(&self, gl: &glow::Context, name: &str, unit: u32) {
        self.bind(gl);
        let loc = self.uniform(gl, name);
        // SAFETY: plain GL call on the current context, with this program in use.
        unsafe { gl.uniform_1_i32(loc.as_ref(), unit as i32) }
    }

    pub fn bind(&self, gl: &glow::Context) {
        // SAFETY: plain GL call on the current context.
        unsafe { gl.use_program(Some(self.handle)) }
    }

    pub fn delete(self, gl: &glow::Context) {
        // SAFETY: the program belongs to this context and is not used again.
        unsafe { gl.delete_program(self.handle) }
    }
}

/// Uniform setters, so passes never call `glow` themselves.
pub fn set_i32(gl: &glow::Context, loc: Option<&glow::UniformLocation>, v: i32) {
    // SAFETY: plain GL call on the current context, with the location's program in use.
    unsafe { gl.uniform_1_i32(loc, v) }
}

pub fn set_ivec2(gl: &glow::Context, loc: Option<&glow::UniformLocation>, v: [i32; 2]) {
    // SAFETY: as above.
    unsafe { gl.uniform_2_i32(loc, v[0], v[1]) }
}

pub fn set_f32(gl: &glow::Context, loc: Option<&glow::UniformLocation>, v: f32) {
    // SAFETY: as above.
    unsafe { gl.uniform_1_f32(loc, v) }
}

pub fn set_vec2(gl: &glow::Context, loc: Option<&glow::UniformLocation>, v: [f32; 2]) {
    // SAFETY: as above.
    unsafe { gl.uniform_2_f32(loc, v[0], v[1]) }
}

/// The whole pipeline state a pass draws with, set in full every time (A11.13 rule 3).
#[derive(Clone, Copy, Debug)]
pub struct State {
    /// x, y, width, height in the bound target's pixels.
    pub viewport: [i32; 4],
    pub depth_test: bool,
    pub depth_write: bool,
    pub blend: bool,
    pub cull_back: bool,
}

impl State {
    /// A full-target pass with no depth, blending or culling.
    pub const fn flat(w: u32, h: u32) -> State {
        State {
            viewport: [0, 0, w as i32, h as i32],
            depth_test: false,
            depth_write: false,
            blend: false,
            cull_back: false,
        }
    }
}

pub fn apply(gl: &glow::Context, s: &State) {
    // SAFETY: plain GL state calls on the current context.
    unsafe {
        let [x, y, w, h] = s.viewport;
        gl.viewport(x, y, w, h);
        let set = |cap: u32, on: bool| if on { gl.enable(cap) } else { gl.disable(cap) };
        set(glow::DEPTH_TEST, s.depth_test);
        set(glow::BLEND, s.blend);
        set(glow::CULL_FACE, s.cull_back);
        set(glow::SCISSOR_TEST, false);
        set(glow::STENCIL_TEST, false);
        gl.depth_mask(s.depth_write);
        gl.color_mask(true, true, true, true);
        if s.cull_back {
            gl.cull_face(glow::BACK);
        }
    }
}

/// An empty vertex array, for passes that make their vertices from `gl_VertexID`.
pub fn empty_vertex_array(gl: &glow::Context) -> Result<glow::VertexArray, RenderError> {
    // SAFETY: plain GL call on the current context.
    unsafe { gl.create_vertex_array().map_err(RenderError::Gl) }
}

/// Draws one triangle covering the bound target, from `gl_VertexID` alone.
pub fn draw_full_target(gl: &glow::Context, vao: glow::VertexArray) {
    // SAFETY: plain GL calls on the current context, with a program bound by the caller.
    unsafe {
        gl.bind_vertex_array(Some(vao));
        gl.draw_arrays(glow::TRIANGLES, 0, 3);
        gl.bind_vertex_array(None);
    }
}

/// `GL_RENDERER | GL_VERSION`, for the self-check (A15.4).
pub fn info(gl: &glow::Context) -> String {
    // SAFETY: plain GL queries on the current context.
    unsafe {
        format!(
            "{} | {}",
            gl.get_parameter_string(glow::RENDERER),
            gl.get_parameter_string(glow::VERSION)
        )
    }
}

/// The first GL error since the last call, or 0.
pub fn error(gl: &glow::Context) -> u32 {
    // SAFETY: plain GL query on the current context.
    unsafe { gl.get_error() }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PLT-01
    #[test]
    fn formats() {
        // Each format's GL constants are core in OpenGL ES 3.0 and WebGL2, and its texel size is what uploads assume.
        let expect = [
            (Format::Rgba8, 0x8058, 0x1908, 0x1401, 4),
            (Format::R8, 0x8229, 0x1903, 0x1401, 1),
            (Format::R32F, 0x822E, 0x1903, 0x1406, 4),
            (Format::Rg32F, 0x8230, 0x8227, 0x1406, 8),
        ];
        for (f, internal, format, ty, bytes) in expect {
            assert_eq!(
                (f.internal(), f.format(), f.channel_type(), f.bytes()),
                (internal, format, ty, bytes),
                "{f:?}"
            );
        }
    }
}
