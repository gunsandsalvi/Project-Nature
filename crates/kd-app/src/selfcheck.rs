//! The self-check (A15.4, `PRC-11`): what a new build checks on the phone at start, and the report it sends back
//! as a `KDS1:` code when something fails: every shader compiled, the GL version OpenGL ES 3 or WebGL2, and the
//! core's maths and draws equal to the cloud's bits (α00b); the catalogue and the probe scene's steps equal to the
//! twins' (α01a); the rest join with their alphas.

use crate::json::json_str;

/// The code's prefix: version 1 of the self-check's report.
pub const PREFIX: &str = "KDS1:";

/// What failed, and where: the build, the device and the GL driver.
///
/// Implements PRC-11, see A15.4: the first time a build opens, a check of a few seconds, and a short code when
/// something fails.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct Report {
    pub build: String,
    pub device: String,
    pub gl: String,
    pub fail: Vec<String>,
}

impl Report {
    /// Compact JSON: `{"v":…,"dev":…,"gl":…,"fail":[…]}`, which `tools/decode-bench.py` reads back.
    pub fn to_json(&self) -> String {
        let fails: Vec<String> = self.fail.iter().map(|f| json_str(f)).collect();
        format!(
            "{{\"v\":{},\"dev\":{},\"gl\":{},\"fail\":[{}]}}",
            json_str(&self.build),
            json_str(&self.device),
            json_str(&self.gl),
            fails.join(",")
        )
    }
}

/// The report's line for core probes whose hashes differ from the cloud's, naming each with the hash made here,
/// or nothing when all are equal (A15.9 item 5).
pub fn core_line(differences: &[(&str, u64)]) -> Option<String> {
    if differences.is_empty() {
        return None;
    }
    let each: Vec<String> = differences.iter().map(|(name, h)| format!("{name} {h:016x}")).collect();
    Some(format!("core bits differ from the cloud's: {}", each.join(", ")))
}

/// The report's line for a probe scene whose steps differ from the twins', or nothing when they are equal (A11.13
/// rule 2): how many differ, and the first one.
pub fn probe_line(gpu: &[u8], twins: &[u8]) -> Option<String> {
    let differ: Vec<usize> = (0..gpu.len().max(twins.len()))
        .filter(|&i| gpu.get(i) != twins.get(i))
        .collect();
    let &first = differ.first()?;
    let show = |v: Option<&u8>| v.map_or("none".to_string(), u8::to_string);
    Some(format!(
        "probe: {} of {} steps differ from the twins, the first at {first}: GPU {}, twin {}",
        differ.len(),
        twins.len(),
        show(gpu.get(first)),
        show(twins.get(first))
    ))
}

/// Whether the driver is OpenGL ES 3 or WebGL2, which every shader is written for (A11.1).
pub fn gl_version_ok(info: &str) -> bool {
    info.contains("OpenGL ES 3") || info.contains("WebGL 2")
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::json::requests_json;
    use crate::{App, AppConfig, Platform, Request};
    use std::sync::{Arc, Mutex};

    struct Outbox(Mutex<Vec<Request>>);
    impl Platform for Outbox {
        fn now_ns(&self) -> u64 {
            0
        }
        fn post(&self, r: Request) {
            self.0.lock().unwrap().push(r);
        }
    }

    // checks: PRC-11
    #[test]
    fn report_as_requests() {
        // A failing report becomes compact JSON, and the request carrying it reaches the shells exactly so.
        let r = Report {
            build: "a00 · 1000 · abc1234".into(),
            device: "Google Pixel 11 Pro XL SDK 37".into(),
            gl: "Mali | OpenGL ES 3.2".into(),
            fail: vec!["shader \"upscale\" did not compile:\n0:1 error".into()],
        };
        let json = r.to_json();
        assert_eq!(
            json,
            "{\"v\":\"a00 · 1000 · abc1234\",\"dev\":\"Google Pixel 11 Pro XL SDK 37\",\"gl\":\"Mali | OpenGL ES 3.2\",\
             \"fail\":[\"shader \\\"upscale\\\" did not compile:\\n0:1 error\"]}"
        );
        let req = Request::ShowCode {
            title: "Kindling self-check".into(),
            prefix: PREFIX.into(),
            json,
        };
        // The whole request, exactly: the report's quotes and backslashes escaped once more inside its string.
        assert_eq!(
            requests_json(&[req]),
            r#"[{"ShowCode":{"title":"Kindling self-check","prefix":"KDS1:","json":"{\"v\":\"a00 · 1000 · abc1234\",\"dev\":\"Google Pixel 11 Pro XL SDK 37\",\"gl\":\"Mali | OpenGL ES 3.2\",\"fail\":[\"shader \\\"upscale\\\" did not compile:\\n0:1 error\"]}"}}]"#
        );
        assert_eq!(requests_json(&[]), "[]");
    }

    // checks: PRC-11
    #[test]
    fn one_code_a_run() {
        // Failures are gathered and sent once, never again in the same run; a run with none sends nothing.
        let outbox = Arc::new(Outbox(Mutex::new(Vec::new())));
        let mut app = App::new(outbox.clone(), AppConfig { device: "test".into() });
        app.finish_check();
        assert!(outbox.0.lock().unwrap().is_empty());
        app.check_failed("shader x did not compile".into());
        app.check_failed("GL version: OpenGL ES 2.0".into());
        app.finish_check();
        app.check_failed("later".into());
        app.finish_check();
        let posted = outbox.0.lock().unwrap();
        assert_eq!(posted.len(), 1);
        let Request::ShowCode { prefix, json, .. } = &posted[0];
        assert_eq!(prefix, PREFIX);
        assert!(json.contains("\"fail\":[\"shader x did not compile\",\"GL version: OpenGL ES 2.0\"]"));
    }

    // checks: RES-05 PRC-11
    #[test]
    fn core_bits_named() {
        // On the build machine the core's bits are the stored ones, so a new app reports nothing.
        assert!(kd_core::bits::differences().is_empty());
        let outbox = Arc::new(Outbox(Mutex::new(Vec::new())));
        let app = App::new(outbox.clone(), AppConfig { device: "test".into() });
        assert!(app.core_bits() && outbox.0.lock().unwrap().is_empty());
        // A difference names each probe with the hash made here.
        assert_eq!(core_line(&[]), None);
        assert_eq!(
            core_line(&[("m.sin", 0xab), ("chance.draws", 1)]).as_deref(),
            Some("core bits differ from the cloud's: m.sin 00000000000000ab, chance.draws 0000000000000001")
        );
    }

    // checks: PRE-20 PRC-11
    #[test]
    fn probe_named() {
        assert_eq!(probe_line(&[1, 2, 3], &[1, 2, 3]), None);
        assert_eq!(
            probe_line(&[1, 4, 3, 0], &[1, 2, 3, 5]).as_deref(),
            Some("probe: 2 of 4 steps differ from the twins, the first at 1: GPU 4, twin 2")
        );
        assert_eq!(
            probe_line(&[], &[7]).as_deref(),
            Some("probe: 1 of 1 steps differ from the twins, the first at 0: GPU none, twin 7")
        );
    }

    // checks: PRE-32 PLT-09
    #[test]
    fn version_and_hour_lines() {
        // The strip's version line: the alpha and version code, the catalogue's rules version and its hash.
        let cat = kd_data::Catalogue::load(crate::CATALOGUE).unwrap();
        let line = crate::version_line("a01a · 1011 · 1a2b3c4", &cat, 0xa11e_44f9_8124_a890);
        assert_eq!(line, "a01a · 1011 · catalogue 1.0 a11e44f9");
        assert_eq!(crate::version_line("dev", &cat, 0), "dev · catalogue 1.0 00000000");
        // The hours' words, every one drawable in the font.
        assert_eq!(crate::hour_line(4), "Late afternoon · 16:30");
        assert_eq!(crate::hour_line(5), "Dusk · 17:45");
        assert_eq!(crate::hour_line(8), "Dawn · 06:30");
        let font = kd_ui::font::font();
        for h in 0..crate::HOURS.len() {
            for c in crate::hour_line(h).chars() {
                assert!(font.glyphs.iter().any(|g| g.0 == c), "{c:?} has no glyph");
            }
        }
    }

    // checks: PRE-30 RES-05
    #[test]
    fn palette_rows_stored() {
        // The palette row at each of the app's hours, as this code makes it in the cloud; the smoke test checks the
        // browser makes the same bytes (tests/golden/palette.txt, rewritten with KD_WRITE_GOLDEN=1).
        let cat = kd_data::Catalogue::load(crate::CATALOGUE).unwrap();
        let layout = kd_render::looks::Layout::new(&cat).unwrap();
        let text: String = (0..crate::HOURS.len())
            .map(|h| {
                let row = kd_render::frame::Lighting::new(&cat, &layout, &crate::sky_at(h))
                    .palette
                    .row;
                let hex: Vec<String> = row
                    .iter()
                    .map(|c| format!("{:02x}{:02x}{:02x}", c[0], c[1], c[2]))
                    .collect();
                format!("{h} {}\n", hex.join(" "))
            })
            .collect();
        let path = concat!(env!("CARGO_MANIFEST_DIR"), "/../../tests/golden/palette.txt");
        if std::env::var_os("KD_WRITE_GOLDEN").is_some() {
            std::fs::write(path, &text).unwrap();
        }
        assert_eq!(
            std::fs::read_to_string(path).unwrap_or_default(),
            text,
            "tests/golden/palette.txt is not this code's palette: KD_WRITE_GOLDEN=1 cargo test -p kd-app palette_rows"
        );
    }

    // checks: PRE-30 MAT-13
    /// The cloud's timings for the bench file (A15.10): `cargo test --profile fast -p kd-app -- --ignored --nocapture`.
    #[test]
    #[ignore]
    fn light_timings() {
        let median = |f: &mut dyn FnMut()| {
            let mut t: Vec<u128> = (0..201)
                .map(|_| {
                    let t0 = std::time::Instant::now();
                    f();
                    t0.elapsed().as_nanos()
                })
                .collect();
            t.sort_unstable();
            t[100] as f64 / 1000.0
        };
        let cat = kd_data::Catalogue::load(crate::CATALOGUE).unwrap();
        let layout = kd_render::looks::Layout::new(&cat).unwrap();
        let light = kd_render::light::light(&cat.air, &crate::sky_at(4));
        let (row, radiance) = kd_render::looks::row(&cat, &layout, &light);
        let blob = median(&mut || {
            std::hint::black_box(kd_data::Catalogue::load(crate::CATALOGUE).unwrap());
        });
        let row_us = median(&mut || {
            std::hint::black_box(kd_render::looks::row(&cat, &layout, &light));
        });
        let view = kd_render::frame::VIEW;
        let tables_us = median(&mut || {
            std::hint::black_box(kd_render::looks::tables(
                &cat.air, &layout, &light, &row, &radiance, view,
            ));
        });
        println!("TIMINGS blob_load_us {blob:.1} palette_row_us {row_us:.1} tables_us {tables_us:.1}");
    }

    // checks: MAT-13 PLT-09
    #[test]
    fn catalogue_loads_at_start() {
        // The embedded blob loads, checked, well inside A3.6's 10 ms, and a new app reports nothing about it.
        let t0 = std::time::Instant::now();
        let cat = kd_data::Catalogue::load(crate::CATALOGUE).expect("the embedded catalogue loads");
        assert!(t0.elapsed().as_millis() < 10, "{:?}", t0.elapsed());
        assert_eq!((cat.colours[0].id.as_str(), cat.looks.len()), ("void", 5));
        let outbox = Arc::new(Outbox(Mutex::new(Vec::new())));
        let app = App::new(outbox.clone(), AppConfig { device: "test".into() });
        assert!(app.catalogue().is_some() && outbox.0.lock().unwrap().is_empty());
    }

    // checks: PRC-11
    #[test]
    fn gl_versions() {
        assert!(gl_version_ok("Adreno (TM) 830 | OpenGL ES 3.2 V@0800.0"));
        assert!(gl_version_ok("WebKit WebGL | WebGL 2.0 (OpenGL ES 3.0 Chromium)"));
        assert!(!gl_version_ok("Mali | OpenGL ES 2.0"));
    }
}
