//! The ground (A11.5): a view area's height-field chunks on the GPU at every vertex spacing, the surfaces texture
//! from `data/models/surfaces.md`, and the ground shader that draws them in passes 1 and 2.
//! Implements `PRE-02` and `PRE-20` in part.

pub mod mesh;

use crate::RenderError;
use crate::pass::Prog;
use glow::HasContext;
use kd_data::Catalogue;
use kd_view::{GroundGrid, Pos};
use mesh::{VERTEX_BYTES, build_chunks, spacing_for};

/// Rows of the surfaces texture: the most surfaces the catalogue may hold.
pub const SURFACE_ROWS: u32 = 64;
/// The vertex spacings built for each area, metres (A11.5).
const STEPS: [u32; 4] = [1, 2, 4, 8];

/// The surfaces texture, two RGBA8 texels a row (A11.5): the ladder, the share of stones and their size in
/// centimetres, the share of tufts; then the flags.
pub fn surface_texels(cat: &Catalogue) -> Vec<u8> {
    let mut t = vec![0u8; 2 * SURFACE_ROWS as usize * 4];
    let share = |k: u16| ((u32::from(k) * 255 + 500) / 1000).min(255) as u8;
    for (row, s) in cat.body.surfaces.iter().take(SURFACE_ROWS as usize).enumerate() {
        let o = row * 8;
        t[o..o + 4].copy_from_slice(&[
            s.ladder,
            share(s.stone_density),
            (s.stone_size_mm / 10).min(255) as u8,
            share(s.tuft_density),
        ]);
        t[o + 4] = s.flags;
    }
    t
}

struct GpuChunk {
    vao: glow::VertexArray,
    bufs: [glow::Buffer; 2],
    count: i32,
}

/// What the ground's draw needs each frame beyond the camera: set by the renderer.
pub struct GroundUniforms<'a> {
    pub vp: &'a [f32; 16],
    pub area_off: [f32; 3],
    pub step: u32,
}

/// One view area's ground on the GPU, at every spacing, and the programs that draw it.
pub struct GroundGpu {
    pub prog: Prog,
    pub shadow_prog: Prog,
    pub surf: glow::Texture,
    levels: Vec<(u32, Vec<GpuChunk>)>,
    /// The area's corner, and its lowest and highest ground in metres.
    pub origin: Pos,
    pub lo_m: f32,
    pub hi_m: f32,
}

impl GroundGpu {
    /// Compiles the ground's programs and makes the surfaces texture; no area yet.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn new(gl: &glow::Context, defines: &str, cat: &Catalogue) -> Result<GroundGpu, RenderError> {
        unsafe {
            let attribs = [(0, "aPos"), (1, "aNrm"), (2, "aB"), (3, "aFlags")];
            let (vs, fs) = (
                include_str!("../shaders/terrain.vert"),
                include_str!("../shaders/terrain.frag"),
            );
            let prog = Prog::new(gl, vs, fs, defines, true, &attribs)?;
            let shadow_prog = Prog::new(gl, vs, fs, &format!("{defines}#define SHADOW\n"), true, &attribs)?;
            let surf =
                crate::target::texture(gl, 2, SURFACE_ROWS, glow::RGBA8, glow::RGBA, Some(&surface_texels(cat)))?;
            Ok(GroundGpu {
                prog,
                shadow_prog,
                surf,
                levels: Vec::new(),
                origin: Pos::default(),
                lo_m: 0.0,
                hi_m: 0.0,
            })
        }
    }

    /// Uploads an area's ground at every spacing, replacing the last.
    ///
    /// # Safety
    /// Needs a current GL context on this thread.
    #[allow(unsafe_code)]
    pub unsafe fn upload(&mut self, gl: &glow::Context, g: &GroundGrid) -> Result<(), RenderError> {
        unsafe {
            for (_, chunks) in self.levels.drain(..) {
                for c in chunks {
                    gl.delete_vertex_array(c.vao);
                    gl.delete_buffer(c.bufs[0]);
                    gl.delete_buffer(c.bufs[1]);
                }
            }
            for step in STEPS {
                let mut gpu = Vec::new();
                for c in build_chunks(g, step) {
                    let vao = gl.create_vertex_array().map_err(RenderError::Resource)?;
                    gl.bind_vertex_array(Some(vao));
                    let vbo = gl.create_buffer().map_err(RenderError::Resource)?;
                    gl.bind_buffer(glow::ARRAY_BUFFER, Some(vbo));
                    let vb: Vec<u8> = c.verts.iter().flat_map(|v| v.bytes()).collect();
                    gl.buffer_data_u8_slice(glow::ARRAY_BUFFER, &vb, glow::STATIC_DRAW);
                    let ebo = gl.create_buffer().map_err(RenderError::Resource)?;
                    gl.bind_buffer(glow::ELEMENT_ARRAY_BUFFER, Some(ebo));
                    let ib: Vec<u8> = c.idx.iter().flat_map(|i| i.to_le_bytes()).collect();
                    gl.buffer_data_u8_slice(glow::ELEMENT_ARRAY_BUFFER, &ib, glow::STATIC_DRAW);
                    let stride = VERTEX_BYTES as i32;
                    gl.vertex_attrib_pointer_f32(0, 3, glow::FLOAT, false, stride, 0);
                    gl.vertex_attrib_pointer_f32(1, 3, glow::BYTE, true, stride, 12);
                    gl.vertex_attrib_pointer_f32(2, 4, glow::UNSIGNED_BYTE, false, stride, 15);
                    gl.vertex_attrib_pointer_f32(3, 1, glow::UNSIGNED_BYTE, false, stride, 19);
                    for a in 0..4 {
                        gl.enable_vertex_attrib_array(a);
                    }
                    gl.bind_vertex_array(None);
                    gpu.push(GpuChunk {
                        vao,
                        bufs: [vbo, ebo],
                        count: c.idx.len() as i32,
                    });
                }
                self.levels.push((step, gpu));
            }
            let (mut lo, mut hi) = (f32::MAX, f32::MIN);
            for &h in &g.heights_m {
                lo = lo.min(h);
                hi = hi.max(h);
            }
            self.origin = g.origin;
            self.lo_m = lo;
            self.hi_m = hi;
        }
        Ok(())
    }

    /// Whether an area is uploaded.
    pub fn ready(&self) -> bool {
        !self.levels.is_empty()
    }

    /// Draws every chunk at the spacing for art pixels of `texel` metres with `prog`, whose other uniforms are set.
    ///
    /// # Safety
    /// Needs a current GL context on this thread, with `prog` in use.
    #[allow(unsafe_code)]
    pub unsafe fn draw(&self, gl: &glow::Context, prog: &Prog, u: &GroundUniforms) -> u32 {
        let want = u.step;
        let Some((_, chunks)) = self.levels.iter().find(|(s, _)| *s == want) else {
            return 0;
        };
        unsafe {
            prog.f(gl, "uVP", u.vp);
            prog.f(gl, "uAreaOff", &u.area_off);
            prog.tex(gl, "uSurf", 3, Some(self.surf));
            for c in chunks {
                gl.bind_vertex_array(Some(c.vao));
                gl.draw_elements(glow::TRIANGLES, c.count, glow::UNSIGNED_SHORT, 0);
            }
            gl.bind_vertex_array(None);
        }
        chunks.len() as u32
    }
}

/// The spacing to draw at for art pixels of `texel` metres.
pub fn step_for(texel: f32) -> u32 {
    spacing_for(texel)
}
