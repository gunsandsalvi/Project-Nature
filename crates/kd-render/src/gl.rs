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
    /// The palette row, 256 × 1 RGBA8 (A11.3).
    pub const PALETTE: u32 = 1;
    /// The tables, 256 × 16 R8 (A11.3).
    pub const TABLES: u32 = 2;
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

    /// Replaces every texel with `data`, exactly w × h × bytes.
    pub fn update(&self, gl: &glow::Context, data: &[u8]) -> Result<(), RenderError> {
        if data.len() != self.w as usize * self.h as usize * self.format.bytes() {
            return Err(RenderError::Gl(format!(
                "texture update of {} bytes for {} x {} {:?}",
                data.len(),
                self.w,
                self.h,
                self.format
            )));
        }
        // SAFETY: plain GL calls on the current context; `data` was checked to hold exactly the texels named.
        unsafe {
            gl.bind_texture(glow::TEXTURE_2D, Some(self.handle));
            gl.pixel_store_i32(glow::UNPACK_ALIGNMENT, 1);
            gl.tex_sub_image_2d(
                glow::TEXTURE_2D,
                0,
                0,
                0,
                self.w as i32,
                self.h as i32,
                self.format.format(),
                self.format.channel_type(),
                glow::PixelUnpackData::Slice(Some(data)),
            );
            gl.bind_texture(glow::TEXTURE_2D, None);
        }
        Ok(())
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

/// A vertex attribute's component type, as the shader reads it.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Component {
    F32,
    /// Bytes read as floats 0 to 1.
    U8Norm,
    /// Signed bytes read as floats −1 to 1.
    I8Norm,
    /// Read as whole numbers (`in uint`, `in uvecN`).
    U16,
    U32,
}

impl Component {
    pub const fn bytes(self) -> usize {
        match self {
            Component::F32 | Component::U32 => 4,
            Component::U8Norm | Component::I8Norm => 1,
            Component::U16 => 2,
        }
    }

    pub const fn gl_type(self) -> u32 {
        match self {
            Component::F32 => glow::FLOAT,
            Component::U8Norm => glow::UNSIGNED_BYTE,
            Component::I8Norm => glow::BYTE,
            Component::U16 => glow::UNSIGNED_SHORT,
            Component::U32 => glow::UNSIGNED_INT,
        }
    }

    /// Whether the shader reads whole numbers (`glVertexAttribIPointer`) rather than floats.
    pub const fn integer(self) -> bool {
        matches!(self, Component::U16 | Component::U32)
    }
}

/// One attribute of a vertex: its `layout(location = n)` in the shader, its type and its 1 to 4 components.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Attr {
    pub location: u32,
    pub component: Component,
    pub count: u8,
}

/// A mesh's vertex layout, declared once beside its shader (A11.13 rule 3): the attributes packed in order, each
/// starting on a multiple of 4 bytes, which OpenGL ES 3.0 and WebGL2 both accept.
#[derive(Clone, Copy, Debug)]
pub struct Layout {
    pub attrs: &'static [Attr],
}

impl Layout {
    /// Each attribute's byte offset, and the stride.
    pub fn offsets(&self) -> (Vec<usize>, usize) {
        let mut at = 0;
        let mut offsets = Vec::with_capacity(self.attrs.len());
        for a in self.attrs {
            offsets.push(at);
            at += (a.component.bytes() * a.count as usize).next_multiple_of(4);
        }
        (offsets, at)
    }

    /// The layout's mistakes: no attributes, a count outside 1 to 4, or a location used twice or beyond the 16
    /// every device has.
    pub fn check(&self) -> Result<(), String> {
        if self.attrs.is_empty() {
            return Err("a layout with no attributes".into());
        }
        for (i, a) in self.attrs.iter().enumerate() {
            if !(1..=4).contains(&a.count) {
                return Err(format!("location {} has {} components", a.location, a.count));
            }
            if a.location >= 16 {
                return Err(format!("location {} is beyond the 16 every device has", a.location));
            }
            if self.attrs[..i].iter().any(|b| b.location == a.location) {
                return Err(format!("location {} is used twice", a.location));
            }
        }
        Ok(())
    }
}

/// The vertices or indices a mesh's draw takes: whole vertices of `stride` bytes, whole triangles, and every index
/// naming a vertex.
fn draw_count(stride: usize, vertex_bytes: usize, indices: Option<&[u32]>) -> Result<i32, String> {
    if !vertex_bytes.is_multiple_of(stride) {
        return Err(format!(
            "{vertex_bytes} vertex bytes are not whole vertices of {stride}"
        ));
    }
    let vertices = vertex_bytes / stride;
    let count = match indices {
        Some(ix) => {
            if let Some(i) = ix.iter().find(|&&i| i as usize >= vertices) {
                return Err(format!("index {i} beyond the {vertices} vertices"));
            }
            ix.len()
        }
        None => vertices,
    };
    if !count.is_multiple_of(3) {
        return Err(format!("{count} corners are not whole triangles"));
    }
    i32::try_from(count).map_err(|_| format!("{count} corners in one mesh"))
}

/// A mesh of triangles: vertices in a declared layout and, if given, 32-bit indices into them.
pub struct Mesh {
    pub vao: glow::VertexArray,
    pub vertices: glow::Buffer,
    pub indices: Option<glow::Buffer>,
    /// The vertices or indices one draw takes.
    pub count: i32,
}

impl Mesh {
    /// A mesh from `vertices`, whole vertices in `layout`'s bytes, and `indices` into them; mistakes in either are
    /// refused before any GL call.
    pub fn new(
        gl: &glow::Context,
        layout: &Layout,
        vertices: &[u8],
        indices: Option<&[u32]>,
    ) -> Result<Mesh, RenderError> {
        layout.check().map_err(RenderError::Gl)?;
        let (offsets, stride) = layout.offsets();
        let count = draw_count(stride, vertices.len(), indices).map_err(RenderError::Gl)?;
        // SAFETY: plain GL calls on the current context, with the sizes checked above; the vertex array is unbound
        // before the index buffer could be, so it keeps its indices.
        unsafe {
            let vao = gl.create_vertex_array().map_err(RenderError::Gl)?;
            gl.bind_vertex_array(Some(vao));
            let buffer = gl.create_buffer().map_err(RenderError::Gl)?;
            gl.bind_buffer(glow::ARRAY_BUFFER, Some(buffer));
            gl.buffer_data_u8_slice(glow::ARRAY_BUFFER, vertices, glow::STATIC_DRAW);
            for (a, &offset) in layout.attrs.iter().zip(&offsets) {
                let (size, ty, stride, offset) = (a.count as i32, a.component.gl_type(), stride as i32, offset as i32);
                gl.enable_vertex_attrib_array(a.location);
                if a.component.integer() {
                    gl.vertex_attrib_pointer_i32(a.location, size, ty, stride, offset);
                } else {
                    gl.vertex_attrib_pointer_f32(a.location, size, ty, a.component != Component::F32, stride, offset);
                }
            }
            let index_buffer = match indices {
                Some(ix) => {
                    let b = gl.create_buffer().map_err(RenderError::Gl)?;
                    gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, Some(b));
                    let bytes: Vec<u8> = ix.iter().flat_map(|i| i.to_ne_bytes()).collect();
                    gl.buffer_data_u8_slice(glow::ELEMENT_ARRAY_BUFFER, &bytes, glow::STATIC_DRAW);
                    Some(b)
                }
                None => None,
            };
            gl.bind_vertex_array(None);
            gl.bind_buffer(glow::ARRAY_BUFFER, None);
            Ok(Mesh {
                vao,
                vertices: buffer,
                indices: index_buffer,
                count,
            })
        }
    }

    /// Draws the triangles with the program and the state the pass set.
    pub fn draw(&self, gl: &glow::Context) {
        // SAFETY: plain GL calls on the current context; the indices were checked against the vertices.
        unsafe {
            gl.bind_vertex_array(Some(self.vao));
            if self.indices.is_some() {
                gl.draw_elements(glow::TRIANGLES, self.count, glow::UNSIGNED_INT, 0);
            } else {
                gl.draw_arrays(glow::TRIANGLES, 0, self.count);
            }
            gl.bind_vertex_array(None);
        }
    }

    pub fn delete(self, gl: &glow::Context) {
        // SAFETY: the mesh's objects belong to this context and are not used again.
        unsafe {
            gl.delete_vertex_array(self.vao);
            gl.delete_buffer(self.vertices);
            if let Some(b) = self.indices {
                gl.delete_buffer(b);
            }
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

    const fn attr(location: u32, component: Component, count: u8) -> Attr {
        Attr {
            location,
            component,
            count,
        }
    }

    // checks: PLT-01
    #[test]
    fn mesh_layouts() {
        // A position, a normal in signed bytes and a whole-number index: each attribute starts on 4 bytes.
        const LAYOUT: Layout = Layout {
            attrs: &[
                attr(0, Component::F32, 2),
                attr(1, Component::I8Norm, 3),
                attr(2, Component::U16, 1),
            ],
        };
        assert_eq!(LAYOUT.check(), Ok(()));
        assert_eq!(LAYOUT.offsets(), (vec![0, 8, 12], 16));
        // The types are core in OpenGL ES 3.0 and WebGL2; whole numbers go through glVertexAttribIPointer.
        let types: Vec<(u32, usize, bool)> = [
            Component::F32,
            Component::U8Norm,
            Component::I8Norm,
            Component::U16,
            Component::U32,
        ]
        .map(|c| (c.gl_type(), c.bytes(), c.integer()))
        .into();
        assert_eq!(
            types,
            [
                (0x1406, 4, false),
                (0x1401, 1, false),
                (0x1400, 1, false),
                (0x1403, 2, true),
                (0x1405, 4, true)
            ]
        );
        // Mistakes are refused before any GL call.
        const TWICE: [Attr; 2] = [attr(0, Component::F32, 1), attr(0, Component::U8Norm, 4)];
        const FIVE: [Attr; 1] = [attr(0, Component::F32, 5)];
        const BEYOND: [Attr; 1] = [attr(16, Component::F32, 1)];
        let problem = |attrs: &'static [Attr]| Layout { attrs }.check().unwrap_err();
        assert!(problem(&TWICE).contains("used twice"), "{}", problem(&TWICE));
        assert!(problem(&[]).contains("no attributes"));
        assert!(problem(&FIVE).contains("5 components"));
        assert!(problem(&BEYOND).contains("beyond the 16"));
        // Whole vertices, whole triangles, and indices naming vertices.
        assert_eq!(draw_count(16, 48, None), Ok(3));
        assert_eq!(draw_count(16, 48, Some(&[0, 1, 2, 2, 1, 0])), Ok(6));
        assert!(draw_count(16, 40, None).is_err());
        assert!(draw_count(16, 64, None).is_err());
        assert!(draw_count(16, 48, Some(&[0, 1, 3])).is_err());
    }
}
