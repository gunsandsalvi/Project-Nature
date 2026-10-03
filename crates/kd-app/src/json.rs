//! JSON written by hand, for the requests the shells take (A2.5's `takeRequests`, A2.6's `take_requests`) and the
//! self-check's report: only `kd-android` may use `serde_json` (A2.2), and the web shell needs the same text.

use crate::Request;

/// `s` as a JSON string, quoted, with quotes, backslashes and control characters escaped (RFC 8259).
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

/// The requests as a JSON array, each an object named by its kind: `[{"ShowCode":{"title":…,"prefix":…,"json":…}}]`.
pub fn requests_json(requests: &[Request]) -> String {
    let items: Vec<String> = requests
        .iter()
        .map(|r| match r {
            Request::ShowCode { title, prefix, json } => format!(
                "{{\"ShowCode\":{{\"title\":{},\"prefix\":{},\"json\":{}}}}}",
                json_str(title),
                json_str(prefix),
                json_str(json)
            ),
        })
        .collect();
    format!("[{}]", items.join(","))
}
