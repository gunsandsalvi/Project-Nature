//! Shader sources: GLSL ES 3.00, the same on the phone and the web (A2.6). Each program is assembled from the
//! version line, a block of `#define`s generated from the Rust constants (A11.13 rule 6), `lib.glsl`, and its own
//! file, so a number shared by Rust and a shader is written once, in Rust.

use std::fmt::Write;

use crate::camera::VIEWPORT;
use crate::ground::{COVER_LEVELS, MAX_SURFACES, PATCH_QUADS, SKIRT_M, SURFACE_LOOKS};
use crate::looks::{PALETTE_SIZE, TABLE_ROWS, table};
use crate::passes::scene::card;
use crate::pixel::{
    Cat, EDGE_OCTAVES, EDGE_WOBBLE_M, OUTLINE_GAP_M, RELIEF_OCTAVES, SEED_EDGE_X, SEED_EDGE_Y, SEED_RELIEF_X,
    SEED_RELIEF_Y, SEED_SPLIT, SUN_TAN, flag,
};
use crate::probe;

const LIB: &str = include_str!("../shaders/lib.glsl");
pub const FULL_TARGET_VERT: &str = include_str!("../shaders/full_target.vert");
pub const LIGHT_CARD_FRAG: &str = include_str!("../shaders/light_card.frag");
pub const POST_FRAG: &str = include_str!("../shaders/post.frag");
pub const PROBE_FRAG: &str = include_str!("../shaders/probe.frag");
pub const UPSCALE_FRAG: &str = include_str!("../shaders/upscale.frag");
pub const UI_VERT: &str = include_str!("../shaders/ui.vert");
pub const UI_FRAG: &str = include_str!("../shaders/ui.frag");
pub const GROUND_VERT: &str = include_str!("../shaders/ground.vert");
pub const GROUND_FRAG: &str = include_str!("../shaders/ground.frag");

/// The generated defines.
pub fn defines() -> String {
    let mut s = String::new();
    for c in Cat::ALL {
        let _ = writeln!(s, "#define {} {}", c.define(), c as u8);
    }
    for (name, value) in [
        ("FLAG_SUNLIT", i32::from(flag::SUNLIT)),
        ("FLAG_HAZE_SHIFT", i32::from(flag::HAZE_SHIFT)),
        ("FLAG_FIRELIT", i32::from(flag::FIRELIT)),
        ("FLAG_GLOWING", i32::from(flag::GLOWING)),
        ("PALETTE_SIZE", PALETTE_SIZE as i32),
        ("TABLE_ROWS", TABLE_ROWS as i32),
        ("TABLE_OUTLINE", table::OUTLINE as i32),
        ("TABLE_EDGE", table::EDGE as i32),
        ("TABLE_HAZE", table::HAZE[0] as i32),
        ("CARD_MAX_LOOKS", card::MAX_LOOKS as i32),
        ("CARD_SWATCH", card::SWATCH),
        ("CARD_NAME_H", card::NAME_H),
        ("CARD_ROW", card::ROW),
        ("PROBE_W", probe::W as i32),
        ("PROBE_H", probe::H as i32),
        ("PROBE_LIGHT_ROWS", probe::LIGHT_ROWS as i32),
        ("PROBE_EDGE_ROW", (probe::LIGHT_ROWS + probe::SURFACE_ROWS) as i32),
        (
            "PROBE_RELIEF_ROW",
            (probe::LIGHT_ROWS + probe::SURFACE_ROWS + probe::EDGE_ROWS) as i32,
        ),
        ("VIEWPORT", VIEWPORT),
        ("PATCH_QUADS", PATCH_QUADS),
        ("MAX_SURFACES", MAX_SURFACES as i32),
        ("SURFACE_LOOKS", SURFACE_LOOKS as i32),
        ("COVER_SIDE", kd_view::AREA_SQUARES as i32),
        ("COVER_TOP", COVER_LEVELS as i32 - 1),
        ("PROBE_COVER_SIDE", probe::COVER_SIDE as i32),
        ("PROBE_COVER_TOP", probe::COVER_TOP as i32),
        ("SEED_EDGE_X", SEED_EDGE_X as i32),
        ("SEED_EDGE_Y", SEED_EDGE_Y as i32),
        ("SEED_SPLIT", SEED_SPLIT as i32),
        ("SEED_RELIEF_X", SEED_RELIEF_X as i32),
        ("SEED_RELIEF_Y", SEED_RELIEF_Y as i32),
        ("RELIEF_OCTAVES", RELIEF_OCTAVES as i32),
    ] {
        let _ = writeln!(s, "#define {name} {value}");
    }
    // Floats as Rust writes them, which GLSL reads back to the same f32.
    let _ = writeln!(s, "#define SKIRT_M {SKIRT_M:?}");
    let _ = writeln!(s, "#define EDGE_WOBBLE_M {EDGE_WOBBLE_M:?}");
    let _ = writeln!(s, "#define SUN_TAN {SUN_TAN:?}");
    let _ = writeln!(s, "#define CARD_DEPTH_M {:?}", card::DEPTH_M);
    let (b, sc, e, l) = (
        probe::PROBE_BETA,
        probe::PROBE_SCALE,
        probe::PROBE_EYE,
        probe::PROBE_LEVELS,
    );
    let _ = writeln!(s, "#define PROBE_BETA vec2({:?}, {:?})", b[0], b[1]);
    let _ = writeln!(s, "#define PROBE_SCALE vec2({:?}, {:?})", sc[0], sc[1]);
    let _ = writeln!(s, "#define PROBE_EYE vec2({:?}, {:?})", e[0], e[1]);
    let _ = writeln!(s, "#define PROBE_LEVELS vec3({:?}, {:?}, {:?})", l[0], l[1], l[2]);
    let gaps: Vec<String> = OUTLINE_GAP_M.iter().map(|g| format!("{g:?}")).collect();
    let _ = writeln!(s, "#define OUTLINE_GAP_M float[8]({})", gaps.join(", "));
    let o = EDGE_OCTAVES;
    let _ = writeln!(
        s,
        "#define EDGE_OCTAVES vec4({:?}, {:?}, {:?}, {:?})",
        o[0], o[1], o[2], o[3]
    );
    s
}

/// A program's stages.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Stage {
    Vertex,
    Fragment,
}

/// A stage's full source: `lib.glsl` sees `KD_VERTEX` or `KD_FRAGMENT`, so each stage gets only its own helpers.
pub fn source(stage: Stage, body: &str) -> String {
    let marker = match stage {
        Stage::Vertex => "KD_VERTEX",
        Stage::Fragment => "KD_FRAGMENT",
    };
    format!(
        "#version 300 es\nprecision highp float;\nprecision highp int;\nprecision highp sampler2D;\n#define {marker}\n{}{}\n{}",
        defines(),
        LIB,
        body
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-22 PRE-01
    #[test]
    fn every_define_a_shader_uses_is_generated() {
        // A name a shader reads but Rust never defines would compile on no GPU; catch it without one.
        let defined = defines();
        for body in [
            FULL_TARGET_VERT,
            LIGHT_CARD_FRAG,
            POST_FRAG,
            PROBE_FRAG,
            UPSCALE_FRAG,
            UI_VERT,
            UI_FRAG,
            GROUND_VERT,
            GROUND_FRAG,
            LIB,
        ] {
            for word in body.split(|c: char| !(c.is_ascii_alphanumeric() || c == '_')) {
                let generated = [
                    "CAT_", "FLAG_", "CARD_", "PROBE_", "PALETTE_", "TABLE_", "VIEWPORT", "PATCH_", "MAX_SURF",
                    "SURFACE_", "SEED_", "SKIRT_", "EDGE_",
                ];
                if generated.iter().any(|p| word.starts_with(p)) {
                    assert!(defined.contains(&format!("#define {word} ")), "{word} is not generated");
                }
            }
        }
        // The categories in the shaders are pixel::Cat's numbers.
        assert!(defined.contains("#define CAT_VOID 0\n") && defined.contains("#define CAT_EFFECT 7\n"));
        assert!(source(Stage::Vertex, "void main() {}").starts_with("#version 300 es\n"));
        // A helper reading a fragment-only variable never reaches a vertex shader.
        let vertex = source(Stage::Vertex, FULL_TARGET_VERT);
        assert!(vertex.contains("#define KD_VERTEX") && !vertex.contains("#define KD_FRAGMENT"));
        assert!(LIB.contains("#ifdef KD_FRAGMENT"));
    }

    // checks: PRE-20 PRE-01
    #[test]
    fn twins_share_their_names() {
        // Every per-pixel formula the shaders compute has a Rust twin of the same name in pixel.rs (A11.13 rule 2).
        let twins = include_str!("pixel.rs");
        for name in [
            "bayer",
            "sunlit",
            "sun_factor",
            "sky_factor",
            "outline_toward",
            "haze",
            "haze_level",
            "lightness",
            "ladder_pos",
            "light_step",
            "hash3",
            "fade5",
            "noise2",
            "octave_fade",
            "faded_noise",
            "edge_wobble",
            "cover_sample",
            "cover_pick",
            "relief_tilt",
            "ground_normal",
            "split_look",
        ] {
            assert!(LIB.contains(&format!(" {name}(")), "{name} missing from lib.glsl");
            assert!(twins.contains(&format!("fn {name}(")), "{name} missing from pixel.rs");
        }
        assert!(LIB.contains(" pack_out(") && twins.contains("fn pack("));
        // The ground's morph has its twin in ground.rs.
        assert!(
            GROUND_VERT.contains("float vertex_height(") && include_str!("ground/mod.rs").contains("fn vertex_height(")
        );
    }
}
