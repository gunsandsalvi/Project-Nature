//! JSON written by hand, for the requests the shells take (A2.5's `takeRequests`, A2.6's `take_requests`) and the
//! self-check's report: only `kd-android` may use `serde_json` (A2.2), and the web shell needs the same text.

use kd_render::probe::CrawlCount;

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

/// The crawl counter's counts (A11.10), one a pair of frames, as `{"crawl":[…],"changed":[…],"pixels":n}`:
/// `pixels`, the art pixels each pair counts, is the first pair's, 0 with none.
pub fn counts_json(counts: &[CrawlCount]) -> String {
    let list = |f: fn(&CrawlCount) -> u32| counts.iter().map(|c| f(c).to_string()).collect::<Vec<_>>().join(",");
    format!(
        "{{\"crawl\":[{}],\"changed\":[{}],\"pixels\":{}}}",
        list(|c| c.crawl),
        list(|c| c.changed),
        counts.first().map_or(0, |c| c.pixels)
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-22
    #[test]
    fn counts_as_json() {
        let c = |crawl, changed| CrawlCount {
            crawl,
            changed,
            pixels: 1_000,
        };
        assert_eq!(
            counts_json(&[c(3, 40), c(0, 7)]),
            r#"{"crawl":[3,0],"changed":[40,7],"pixels":1000}"#
        );
        assert_eq!(counts_json(&[]), r#"{"crawl":[],"changed":[],"pixels":0}"#);
    }
}
