//! Shader sources: GLSL ES 3.00, the same on the phone and the web (A2.6). Each program is assembled from the
//! version line, a block of `#define`s generated from the Rust constants (A11.13 rule 6), `lib.glsl`, and its own
//! file, so a number shared by Rust and a shader is written once, in Rust.

use std::fmt::Write;

use crate::looks::{PALETTE_SIZE, TABLE_ROWS};
use crate::passes::scene::card;
use crate::pixel::{Cat, flag};
use crate::probe;

const LIB: &str = include_str!("../shaders/lib.glsl");
pub const FULL_TARGET_VERT: &str = include_str!("../shaders/full_target.vert");
pub const LIGHT_CARD_FRAG: &str = include_str!("../shaders/light_card.frag");
pub const POST_FRAG: &str = include_str!("../shaders/post.frag");
pub const PROBE_FRAG: &str = include_str!("../shaders/probe.frag");
pub const UPSCALE_FRAG: &str = include_str!("../shaders/upscale.frag");
pub const UI_VERT: &str = include_str!("../shaders/ui.vert");
pub const UI_FRAG: &str = include_str!("../shaders/ui.frag");

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
        ("CARD_MAX_LOOKS", card::MAX_LOOKS as i32),
        ("CARD_SWATCH", card::SWATCH),
        ("CARD_NAME_H", card::NAME_H),
        ("CARD_ROW", card::ROW),
        ("PROBE_W", probe::W as i32),
        ("PROBE_H", probe::H as i32),
    ] {
        let _ = writeln!(s, "#define {name} {value}");
    }
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
            LIB,
        ] {
            for word in body.split(|c: char| !(c.is_ascii_alphanumeric() || c == '_')) {
                let generated = ["CAT_", "FLAG_", "CARD_", "PROBE_", "PALETTE_", "TABLE_"];
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
        for name in ["bayer", "lightness", "ladder_pos", "light_step", "pack_out"] {
            assert!(LIB.contains(&format!(" {name}(")), "{name} missing from lib.glsl");
        }
        let twins = include_str!("pixel.rs");
        for name in [
            "fn bayer(",
            "fn lightness(",
            "fn ladder_pos(",
            "fn light_step(",
            "fn pack(",
        ] {
            assert!(twins.contains(name), "{name} missing from pixel.rs");
        }
    }
}
