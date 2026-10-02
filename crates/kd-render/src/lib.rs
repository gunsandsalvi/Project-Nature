//! kd-render: the renderer (A11); in α00 the lit cube on OpenGL ES 3.0 and WebGL2 through `glow` (A1.3, A2.5, A2.6).
//! Implements PRC-11 in part.

pub mod cube;
pub mod mat;
pub mod palette;

use glow::HasContext;
use mat::Mat4;
use std::fmt;

/// The palette's `void`, `#0d0b14` (the mockup's).
const VOID: [f32; 3] = [13.0 / 255.0, 11.0 / 255.0, 20.0 / 255.0];
/// The cube's colour, `#c8ad56` (the mockup's `g6`).
const GOLD: [f32; 3] = [200.0 / 255.0, 173.0 / 255.0, 86.0 / 255.0];
/// The light's direction before normalising.
const LIGHT: [f32; 3] = [0.4, 0.8, 0.45];
/// Vertical field of view, degrees.
const FOV_DEG: f32 = 40.0;

/// Why the renderer could not start (A3.8): the shader's or linker's info log.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum RenderError {
    Shader(String),
    Link(String),
    Resource(String),
}

impl fmt::Display for RenderError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            RenderError::Shader(log) => write!(f, "a shader did not compile: {log}"),
            RenderError::Link(log) => write!(f, "the shaders did not link: {log}"),
            RenderError::Resource(log) => write!(f, "a GPU resource could not be made: {log}"),
        }
    }
}

/// What one frame shows; grows into A11.1's `Frame` in α01a.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Frame {
    pub yaw: f32,
    pub pitch: f32,
}

/// The renderer: owns the GL context and the cube's GPU resources. Lives on the GL thread (A2.5).
pub struct Renderer {
    gl: glow::Context,
    program: glow::Program,
    vao: glow::VertexArray,
    u_mvp: Option<glow::UniformLocation>,
    u_nrm: Option<glow::UniformLocation>,
    u_base: Option<glow::UniformLocation>,
    u_light: Option<glow::UniformLocation>,
    count: i32,
    w: u32,
    h: u32,
}

fn bytes(v: impl Iterator<Item = [u8; 4]>) -> Vec<u8> {
    v.flatten().collect()
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

impl Renderer {
    /// Compiles and links the shaders and uploads the cube (A11.1).
    #[allow(unsafe_code)]
    pub fn new(gl: glow::Context) -> Result<Renderer, RenderError> {
        unsafe {
            let vs = compile(&gl, glow::VERTEX_SHADER, include_str!("../shaders/cube.vert"))?;
            let fs = compile(&gl, glow::FRAGMENT_SHADER, include_str!("../shaders/cube.frag"))?;
            let program = gl.create_program().map_err(RenderError::Resource)?;
            gl.attach_shader(program, vs);
            gl.attach_shader(program, fs);
            gl.link_program(program);
            gl.delete_shader(vs);
            gl.delete_shader(fs);
            if !gl.get_program_link_status(program) {
                return Err(RenderError::Link(gl.get_program_info_log(program)));
            }
            let (verts, idx) = cube::mesh();
            let vao = gl.create_vertex_array().map_err(RenderError::Resource)?;
            gl.bind_vertex_array(Some(vao));
            let vbo = gl.create_buffer().map_err(RenderError::Resource)?;
            gl.bind_buffer(glow::ARRAY_BUFFER, Some(vbo));
            let vb = bytes(verts.iter().flatten().map(|f| f.to_le_bytes()));
            gl.buffer_data_u8_slice(glow::ARRAY_BUFFER, &vb, glow::STATIC_DRAW);
            let ebo = gl.create_buffer().map_err(RenderError::Resource)?;
            gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, Some(ebo));
            let ib: Vec<u8> = idx.iter().flat_map(|i| i.to_le_bytes()).collect();
            gl.buffer_data_u8_slice(glow::ELEMENT_ARRAY_BUFFER, &ib, glow::STATIC_DRAW);
            gl.vertex_attrib_pointer_f32(0, 3, glow::FLOAT, false, 24, 0);
            gl.enable_vertex_attrib_array(0);
            gl.vertex_attrib_pointer_f32(1, 3, glow::FLOAT, false, 24, 12);
            gl.enable_vertex_attrib_array(1);
            gl.bind_vertex_array(None);
            let u = |n: &str| gl.get_uniform_location(program, n);
            let (u_mvp, u_nrm, u_base, u_light) = (u("uMVP"), u("uNrm"), u("uBase"), u("uLight"));
            Ok(Renderer {
                gl,
                program,
                vao,
                u_mvp,
                u_nrm,
                u_base,
                u_light,
                count: idx.len() as i32,
                w: 1,
                h: 1,
            })
        }
    }

    /// `GL_RENDERER | GL_VERSION`, for the self-check (A15.4).
    #[allow(unsafe_code)]
    pub fn gl_info(&self) -> String {
        unsafe {
            format!(
                "{} | {}",
                self.gl.get_parameter_string(glow::RENDERER),
                self.gl.get_parameter_string(glow::VERSION)
            )
        }
    }

    /// The drawing surface's new size in pixels.
    pub fn resize(&mut self, w: u32, h: u32) {
        self.w = w.max(1);
        self.h = h.max(1);
    }

    /// Draws one frame: the void, then the lit cube, back faces culled (no depth buffer on the window, A2.5).
    #[allow(unsafe_code)]
    pub fn draw(&mut self, f: &Frame) {
        let aspect = self.w as f32 / self.h as f32;
        let model = Mat4::rot_x(f.pitch).mul(&Mat4::rot_y(f.yaw));
        let mvp = Mat4::perspective(FOV_DEG.to_radians(), aspect, 0.1, 10.0)
            .mul(&Mat4::translate(0.0, 0.0, -3.0))
            .mul(&model);
        let len = (LIGHT[0] * LIGHT[0] + LIGHT[1] * LIGHT[1] + LIGHT[2] * LIGHT[2]).sqrt();
        let gl = &self.gl;
        unsafe {
            gl.viewport(0, 0, self.w as i32, self.h as i32);
            gl.clear_color(VOID[0], VOID[1], VOID[2], 1.0);
            gl.clear(glow::COLOR_BUFFER_BIT);
            gl.enable(glow::CULL_FACE);
            gl.cull_face(glow::BACK);
            gl.front_face(glow::CCW);
            gl.use_program(Some(self.program));
            gl.uniform_matrix_4_f32_slice(self.u_mvp.as_ref(), false, &mvp.0);
            gl.uniform_matrix_3_f32_slice(self.u_nrm.as_ref(), false, &model.normal3());
            gl.uniform_3_f32(self.u_base.as_ref(), GOLD[0], GOLD[1], GOLD[2]);
            gl.uniform_3_f32(self.u_light.as_ref(), LIGHT[0] / len, LIGHT[1] / len, LIGHT[2] / len);
            gl.bind_vertex_array(Some(self.vao));
            gl.draw_elements(glow::TRIANGLES, self.count, glow::UNSIGNED_SHORT, 0);
            gl.bind_vertex_array(None);
        }
    }

    /// The GL error flag (`glGetError`), 0 when none.
    #[allow(unsafe_code)]
    pub fn gl_error(&self) -> u32 {
        unsafe { self.gl.get_error() }
    }
}
