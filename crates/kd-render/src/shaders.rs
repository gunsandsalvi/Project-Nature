//! Shader sources: GLSL ES 3.00, the same on the phone and the web (A2.6). Each program is assembled from the
//! version line, a block of `#define`s generated from the Rust constants (A11.13 rule 6), `lib.glsl`, and its own
//! file, so a number shared by Rust and a shader is written once, in Rust.

use std::fmt::Write;

use crate::passes::scene::card;

const LIB: &str = include_str!("../shaders/lib.glsl");
pub const FULL_TARGET_VERT: &str = include_str!("../shaders/full_target.vert");
pub const TEST_CARD_FRAG: &str = include_str!("../shaders/test_card.frag");
pub const UPSCALE_FRAG: &str = include_str!("../shaders/upscale.frag");

/// The generated defines.
pub fn defines() -> String {
    let mut s = String::new();
    for (name, value) in [
        ("CARD_MARGIN", card::MARGIN),
        ("CARD_CHECKER", card::CHECKER),
        ("CARD_GREY_STEPS", card::GREY_STEPS),
        ("CARD_GREY_W", card::GREY_W),
        ("CARD_GREY_Y", card::GREY_Y),
        ("CARD_GREY_H", card::GREY_H),
        ("CARD_BAR_Y", card::BAR_Y),
        ("CARD_BAR_H", card::BAR_H),
        ("CARD_CORE_X", card::CORE_X),
        ("CARD_CORE_Y", card::CORE_Y),
        ("CARD_CORE_W", card::CORE_W),
        ("CARD_CORE_H", card::CORE_H),
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

    // checks: PRE-22
    #[test]
    fn every_define_a_shader_uses_is_generated() {
        // A name a shader reads but Rust never defines would compile on no GPU; catch it without one.
        let defined = defines();
        for body in [FULL_TARGET_VERT, TEST_CARD_FRAG, UPSCALE_FRAG, LIB] {
            for word in body.split(|c: char| !(c.is_ascii_alphanumeric() || c == '_')) {
                if word.starts_with("CARD_") {
                    assert!(defined.contains(&format!("#define {word} ")), "{word} is not generated");
                }
            }
        }
        assert!(source(Stage::Vertex, "void main() {}").starts_with("#version 300 es\n"));
        // A helper reading a fragment-only variable never reaches a vertex shader.
        let vertex = source(Stage::Vertex, FULL_TARGET_VERT);
        assert!(vertex.contains("#define KD_VERTEX") && !vertex.contains("#define KD_FRAGMENT"));
        assert!(LIB.contains("#ifdef KD_FRAGMENT"));
    }
}
