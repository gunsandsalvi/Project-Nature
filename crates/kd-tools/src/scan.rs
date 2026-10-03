//! Rust sources read for the checks (A2.3 rule 6): the code with every comment and every literal's inside blanked,
//! so a word in a comment or a string never counts, the string literals with their lines, and the parts that are
//! tests (`#[cfg(test)] mod`), which the f64 and name rules leave out.

/// A source file read for the checks.
pub struct Scanned {
    /// The source with comments and the insides of literals turned to spaces, one space a character; newlines
    /// stay, so line numbers are the source's.
    pub code: String,
    /// Each string literal's text as written between its quotes, and the line it starts on.
    pub strings: Vec<(usize, String)>,
}

fn is_ident(c: char) -> bool {
    c.is_alphanumeric() || c == '_'
}

/// Reads a source: comments (nested block comments too), strings, raw strings with any number of `#`, byte and C
/// strings, and character literals, told apart from lifetimes.
pub fn scan(src: &str) -> Scanned {
    let s: Vec<char> = src.chars().collect();
    let mut code: Vec<char> = s.clone();
    let mut strings = Vec::new();
    let blank = |code: &mut Vec<char>, from: usize, to: usize| {
        for c in &mut code[from..to] {
            if *c != '\n' {
                *c = ' ';
            }
        }
    };
    let line_at = |i: usize| s[..i].iter().filter(|&&c| c == '\n').count() + 1;
    let mut i = 0;
    while i < s.len() {
        let c = s[i];
        let next = s.get(i + 1).copied();
        let prev_ident = i > 0 && is_ident(s[i - 1]);
        if c == '/' && next == Some('/') {
            let end = s[i..].iter().position(|&c| c == '\n').map_or(s.len(), |p| i + p);
            blank(&mut code, i, end);
            i = end;
        } else if c == '/' && next == Some('*') {
            let (mut depth, mut j) = (1, i + 2);
            while j < s.len() && depth > 0 {
                if s[j] == '/' && s.get(j + 1) == Some(&'*') {
                    depth += 1;
                    j += 2;
                } else if s[j] == '*' && s.get(j + 1) == Some(&'/') {
                    depth -= 1;
                    j += 2;
                } else {
                    j += 1;
                }
            }
            blank(&mut code, i, j);
            i = j;
        } else if !prev_ident && raw_string_start(&s, i).is_some() {
            let (open, hashes) = raw_string_start(&s, i).unwrap_or_default();
            let body = open + 1;
            let mut j = body;
            while j < s.len() && !(s[j] == '"' && (1..=hashes).all(|k| s.get(j + k) == Some(&'#'))) {
                j += 1;
            }
            strings.push((line_at(i), s[body..j.min(s.len())].iter().collect()));
            blank(&mut code, body, j.min(s.len()));
            i = (j + 1 + hashes).min(s.len());
        } else if c == '"' || (!prev_ident && (c == 'b' || c == 'c') && next == Some('"')) {
            let body = if c == '"' { i + 1 } else { i + 2 };
            let mut j = body;
            while j < s.len() && s[j] != '"' {
                j += if s[j] == '\\' { 2 } else { 1 };
            }
            let j = j.min(s.len());
            strings.push((line_at(i), s[body..j].iter().collect()));
            blank(&mut code, body, j);
            i = j + 1;
        } else if c == '\'' || (!prev_ident && c == 'b' && next == Some('\'')) {
            let q = if c == '\'' { i } else { i + 1 };
            // A character literal: an escape, or one character then a quote; otherwise a lifetime or label.
            let end = if s.get(q + 1) == Some(&'\\') {
                let mut j = q + 3;
                while j < s.len() && s[j] != '\'' {
                    j += 1;
                }
                Some(j)
            } else if s.get(q + 2) == Some(&'\'') {
                Some(q + 2)
            } else {
                None
            };
            match end {
                Some(e) => {
                    blank(&mut code, q + 1, e.min(s.len()));
                    i = e + 1;
                }
                None => i = q + 1,
            }
        } else {
            i += 1;
        }
    }
    Scanned {
        code: code.into_iter().collect(),
        strings,
    }
}

/// Where a raw string starts at `i` (`r"`, `r#"`, `br"`, `cr##"` …): the opening quote's index and the number of
/// `#`, or nothing.
fn raw_string_start(s: &[char], i: usize) -> Option<(usize, usize)> {
    let mut j = i;
    if s[j] == 'b' || s[j] == 'c' {
        j += 1;
    }
    if s.get(j) != Some(&'r') {
        return None;
    }
    j += 1;
    let mut hashes = 0;
    while s.get(j) == Some(&'#') {
        hashes += 1;
        j += 1;
    }
    (s.get(j) == Some(&'"')).then_some((j, hashes))
}

/// The words of some code: maximal runs of letters, digits and `_`, each with its byte offset.
pub fn words(code: &str) -> Vec<(usize, &str)> {
    let mut out = Vec::new();
    let mut start = None;
    for (i, c) in code.char_indices() {
        match (is_ident(c), start) {
            (true, None) => start = Some(i),
            (false, Some(s)) => {
                out.push((s, &code[s..i]));
                start = None;
            }
            _ => {}
        }
    }
    if let Some(s) = start {
        out.push((s, &code[s..]));
    }
    out
}

/// The line of a byte offset.
pub fn line_of(code: &str, offset: usize) -> usize {
    code[..offset].matches('\n').count() + 1
}

/// The test parts of some scanned code: the byte ranges of inline `#[cfg(test)] mod x { … }` blocks, and the
/// names of `#[cfg(test)] mod x;` modules kept in files of their own.
pub fn test_parts(code: &str) -> (Vec<(usize, usize)>, Vec<String>) {
    let (mut spans, mut files) = (Vec::new(), Vec::new());
    let mut from = 0;
    while let Some(p) = code[from..].find("#[cfg(test)]") {
        let at = from + p;
        let mut rest = at + "#[cfg(test)]".len();
        // Other attributes and `pub` may come between.
        loop {
            let t = code[rest..].trim_start();
            rest = code.len() - t.len();
            if t.starts_with("#[") {
                rest += t.find(']').map_or(t.len(), |e| e + 1);
            } else if let Some(after) = t.strip_prefix("pub(crate)").or_else(|| t.strip_prefix("pub")) {
                rest = code.len() - after.len();
            } else {
                break;
            }
        }
        let t = code[rest..].trim_start();
        if let Some(after) = t.strip_prefix("mod ") {
            let name: String = after.trim_start().chars().take_while(|&c| is_ident(c)).collect();
            let after_name = after.trim_start()[name.len()..].trim_start();
            if after_name.starts_with(';') {
                files.push(name);
            } else if after_name.starts_with('{') {
                let open = code.len() - after_name.len();
                let mut depth = 0;
                let mut end = code.len();
                for (k, ch) in code[open..].char_indices() {
                    match ch {
                        '{' => depth += 1,
                        '}' => {
                            depth -= 1;
                            if depth == 0 {
                                end = open + k + 1;
                                break;
                            }
                        }
                        _ => {}
                    }
                }
                spans.push((at, end));
            }
        }
        from = at + 1;
    }
    (spans, files)
}

/// The code with its test parts blanked, newlines kept.
pub fn without_tests(code: &str, spans: &[(usize, usize)]) -> String {
    let mut out: Vec<u8> = code.as_bytes().to_vec();
    for &(a, b) in spans {
        for byte in &mut out[a..b] {
            if *byte != b'\n' {
                *byte = b' ';
            }
        }
    }
    // Blanking turns whole characters to spaces byte by byte, so the text stays valid UTF-8 only if spans hold
    // whole characters, which they do: they start at `#` and end after `}`.
    String::from_utf8(out).unwrap_or_default()
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRN-14
    #[test]
    fn comments_strings_and_tests_set_aside() {
        let src = r####"
fn a<'x>(c: char) -> &'x str { // unsafe in a comment
    /* nested /* unsafe */ still a comment */
    let s = "unsafe \" here";
    let r = r#"raw "unsafe" text"#;
    let b = b'\''; let q = '"';
    if c == 'u' { "flint" } else { br##"x"## ; "" }
}
#[cfg(test)]
mod tests { fn t(x: f64) { let _ = "test"; } }
#[cfg(test)]
pub mod more;
"####;
        let sc = scan(src);
        assert!(!words(&sc.code).iter().any(|(_, w)| *w == "unsafe"), "{}", sc.code);
        assert!(sc.code.contains("fn a<'x>(c: char) -> &'x str {"));
        let texts: Vec<&str> = sc.strings.iter().map(|(_, t)| t.as_str()).collect();
        assert_eq!(
            texts,
            vec![r#"unsafe \" here"#, r#"raw "unsafe" text"#, "flint", "x", "", "test"]
        );
        assert_eq!(sc.strings[0].0, 4);
        let (spans, files) = test_parts(&sc.code);
        assert_eq!(files, vec!["more".to_string()]);
        let rest = without_tests(&sc.code, &spans);
        assert!(!words(&rest).iter().any(|(_, w)| *w == "f64"));
        assert!(words(&sc.code).iter().any(|(_, w)| *w == "f64"));
        assert_eq!(line_of(src, src.find("mod tests").unwrap_or(0)), 10);
    }
}
