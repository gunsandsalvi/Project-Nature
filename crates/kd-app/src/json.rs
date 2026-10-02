//! JSON written by hand, so neither shell needs a JSON crate for it (A2.2).

use crate::Request;

/// A JSON string literal, with `"`, `\` and characters below U+0020 escaped as RFC 8259 says.
pub fn json_str(s: &str) -> String {
    let mut out = String::with_capacity(s.len() + 2);
    out.push('"');
    for c in s.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            c if (c as u32) < 0x20 => out.push_str(&format!("\\u{:04x}", c as u32)),
            c => out.push(c),
        }
    }
    out.push('"');
    out
}

/// The shell's outbox as a JSON array, such as `[{"SelfCheck":{"json":"..."}}]` (A2.5, A2.6).
pub fn requests_json(r: &[Request]) -> String {
    let items: Vec<String> = r
        .iter()
        .map(|q| match q {
            Request::SelfCheck { json } => format!("{{\"SelfCheck\":{{\"json\":{}}}}}", json_str(json)),
        })
        .collect();
    format!("[{}]", items.join(","))
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRC-11
    #[test]
    fn escapes_and_requests() {
        assert_eq!(json_str("a\"b\\c"), r#""a\"b\\c""#);
        assert_eq!(json_str("x\ny\u{1}"), r#""x\ny\u0001""#);
        assert_eq!(json_str("é ok"), "\"é ok\"");
        let r = [Request::SelfCheck {
            json: "{\"v\":\"dev\"}".into(),
        }];
        assert_eq!(requests_json(&r), r#"[{"SelfCheck":{"json":"{\"v\":\"dev\"}"}}]"#);
        assert_eq!(requests_json(&[]), "[]");
    }
}
