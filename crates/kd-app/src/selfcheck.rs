//! The self-check report (A15.4, `PRC-11`): what failed, as JSON the shells turn into a `KDS1:` code.

use crate::json::json_str;

/// `{"v": build, "dev": device, "gl": gl_info, "cat": "<rules major.minor> <hash prefix>", "fail": [...]}`.
pub fn report_json(build: &str, device: &str, gl: &str, cat: &str, fail: &[String]) -> String {
    let fails: Vec<String> = fail.iter().map(|f| json_str(f)).collect();
    format!(
        "{{\"v\":{},\"dev\":{},\"gl\":{},\"cat\":{},\"fail\":[{}]}}",
        json_str(build),
        json_str(device),
        json_str(gl),
        json_str(cat),
        fails.join(",")
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRC-11
    #[test]
    fn report_shape() {
        let j = report_json(
            "a00 abc",
            "Pixel \"11\"",
            "ANGLE | OpenGL ES 3.2",
            "1.0 639ab0fc",
            &["m::sin".into(), "gl: 0x502".into()],
        );
        assert_eq!(
            j,
            r#"{"v":"a00 abc","dev":"Pixel \"11\"","gl":"ANGLE | OpenGL ES 3.2","cat":"1.0 639ab0fc","fail":["m::sin","gl: 0x502"]}"#
        );
        for key in ["\"v\"", "\"dev\"", "\"gl\"", "\"cat\"", "\"fail\""] {
            assert!(j.contains(key));
        }
    }
}
