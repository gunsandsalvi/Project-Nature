//! B04/B11 pre-test, phone part (`kstorage`). `run(config_json) -> result_json` measures on the
//! phone's own storage: saved-moment write, flush and read speed (`B04`, `PLT-07`), small-append
//! flush cost (history journal), and one terrain generation method plus metre detail (`B11`,
//! `WLD-11`). Hashes in the result let the lead compare the phone's bits with the cloud's.
//! Config: {"dir": "<writable folder>", "div": 2, "reps": 3, "gen_w": 1024, "formats": ["custom-none","custom-lz4","sqlite-lz4"]}

pub mod data;
pub mod hist;
pub mod save;
pub mod sq;
pub mod terrain;

#[cfg(feature = "android")]
pub mod android;

use serde_json::{json, Value};
use std::path::{Path, PathBuf};
use std::time::Instant;

pub fn run(config_json: &str) -> String {
    match std::panic::catch_unwind(|| run_inner(config_json)) {
        Ok(Ok(v)) => v.to_string(),
        Ok(Err(e)) => json!({ "block": "B04-B11", "error": e }).to_string(),
        Err(_) => json!({ "block": "B04-B11", "error": "panic" }).to_string(),
    }
}

fn med(v: &mut Vec<f64>) -> f64 {
    v.sort_by(|a, b| a.total_cmp(b));
    if v.is_empty() {
        f64::NAN
    } else {
        v[v.len() / 2]
    }
}
fn stat(mut v: Vec<f64>) -> Value {
    let m = med(&mut v);
    json!({"median": (m * 100.0).round() / 100.0, "min": (v[0] * 100.0).round() / 100.0, "max": (v[v.len() - 1] * 100.0).round() / 100.0})
}
fn ms(t: Instant) -> f64 {
    t.elapsed().as_secs_f64() * 1e3
}

fn run_inner(cfg: &str) -> Result<Value, String> {
    let v: Value = serde_json::from_str(cfg).map_err(|e| format!("bad json: {e}"))?;
    let dir = PathBuf::from(v.get("dir").and_then(Value::as_str).ok_or("missing \"dir\"")?).join("b04b11");
    let div = v.get("div").and_then(Value::as_u64).unwrap_or(2) as usize;
    let reps = v.get("reps").and_then(Value::as_u64).unwrap_or(3).max(1) as usize;
    let gen_w = v.get("gen_w").and_then(Value::as_u64).unwrap_or(1024) as usize;
    let formats: Vec<String> = v
        .get("formats")
        .and_then(Value::as_array)
        .map(|a| a.iter().filter_map(|x| x.as_str().map(String::from)).collect())
        .unwrap_or_else(|| vec!["custom-none".into(), "custom-lz4".into(), "sqlite-lz4".into()]);
    let parts: Vec<String> = v
        .get("parts")
        .and_then(Value::as_array)
        .map(|a| a.iter().filter_map(|x| x.as_str().map(String::from)).collect())
        .unwrap_or_else(|| vec!["save".into(), "fsync".into(), "gen".into(), "detail".into()]);
    let t_all = Instant::now();
    let _ = std::fs::remove_dir_all(&dir);
    std::fs::create_dir_all(&dir).map_err(|e| format!("cannot create {}: {e}", dir.display()))?;
    let mut out = json!({"block": "B04-B11", "version": 1, "div": div, "reps": reps, "arch": std::env::consts::ARCH});
    if parts.iter().any(|p| p == "save") {
        out["save"] = save_bench(&dir, div, reps, &formats)?;
    }
    if parts.iter().any(|p| p == "fsync") {
        out["fsync"] = fsync_bench(&dir)?;
    }
    if parts.iter().any(|p| p == "gen") {
        out["gen"] = gen_bench(gen_w, reps)?;
    }
    if parts.iter().any(|p| p == "detail") {
        out["detail"] = detail_bench(reps)?;
    }
    let _ = std::fs::remove_dir_all(&dir);
    out["total_s"] = json!((t_all.elapsed().as_secs_f64() * 10.0).round() / 10.0);
    Ok(out)
}

fn codec_of(name: &str) -> save::Codec {
    if name.ends_with("lz4") {
        save::Codec::Lz4
    } else if name.ends_with("zstd") || name.ends_with("zstd1") {
        save::Codec::Zstd
    } else {
        save::Codec::None
    }
}

/// Saved moments: write (encode, write, flush, rename), cold read of the whole file, one region.
pub fn save_bench(dir: &Path, div: usize, reps: usize, formats: &[String]) -> Result<Value, String> {
    let t = Instant::now();
    let s = data::build(42, 1000, div);
    let build_ms = ms(t);
    let want = data::content_hash(&s);
    let raw = data::raw_bytes(&s);
    let busy = save::region_in(s.w, s.h, s.people[0].core.x, s.people[0].core.y);
    let mut res = json!({"raw_mb": raw as f64 / 1e6, "build_ms": build_ms.round(), "content_hash": format!("{want:016x}")});
    for f in formats {
        let codec = codec_of(f);
        let (mut enc, mut wr, mut fs_, mut ren, mut tot, mut rd, mut reg) = (vec![], vec![], vec![], vec![], vec![], vec![], vec![]);
        let mut bytes = 0u64;
        for _ in 0..reps {
            if f.starts_with("custom") {
                let name = "moment.ksave";
                let tm = save::save(dir, name, &s, codec).map_err(|e| e.to_string())?;
                bytes = tm.bytes;
                enc.push(tm.encode_ms);
                wr.push(tm.write_ms);
                fs_.push(tm.fsync_ms);
                ren.push(tm.rename_ms);
                tot.push(tm.total_ms);
                let p = dir.join(name);
                save::drop_cache(&p);
                let t = Instant::now();
                let back = save::load_all(&p)?;
                rd.push(ms(t));
                if data::content_hash(&back) != want {
                    return Err(format!("{f}: loaded content differs"));
                }
                drop(back);
                save::drop_cache(&p);
                let t = Instant::now();
                let r = save::load_region(&p, busy)?;
                reg.push(ms(t));
                if r.people.is_empty() {
                    return Err("region without people".into());
                }
                std::fs::remove_file(&p).map_err(|e| e.to_string())?;
            } else {
                let p = dir.join("moment.sqlite");
                for ext in ["", "-wal", "-shm"] {
                    let _ = std::fs::remove_file(format!("{}{ext}", p.display()));
                }
                let t = Instant::now();
                let mut c = sq::open(&p).map_err(|e| e.to_string())?;
                let (ins, ck) = sq::save(&mut c, 1, &s, codec, 0, 0).map_err(|e| e.to_string())?;
                tot.push(ms(t));
                wr.push(ins);
                fs_.push(ck);
                drop(c);
                bytes = std::fs::metadata(&p).map(|m| m.len()).unwrap_or(0);
                save::drop_cache(&p);
                let t = Instant::now();
                let c = sq::open(&p).map_err(|e| e.to_string())?;
                let back = sq::load_all(&c, 1)?;
                rd.push(ms(t));
                if data::content_hash(&back) != want {
                    return Err(format!("{f}: loaded content differs"));
                }
                drop(back);
                drop(c);
                save::drop_cache(&p);
                let t = Instant::now();
                let c = sq::open(&p).map_err(|e| e.to_string())?;
                let r = sq::load_region(&c, 1, busy as i64)?;
                reg.push(ms(t));
                if r.people.is_empty() {
                    return Err("region without people".into());
                }
            }
        }
        let mut e = json!({"mb": bytes as f64 / 1e6, "total_ms": stat(tot), "read_ms": stat(rd), "region_ms": stat(reg)});
        if f.starts_with("custom") {
            e["encode_ms"] = stat(enc);
            e["write_ms"] = stat(wr);
            e["fsync_ms"] = stat(fs_);
            e["rename_ms"] = stat(ren);
        } else {
            e["insert_commit_ms"] = stat(wr);
            e["checkpoint_ms"] = stat(fs_);
        }
        res[f] = e;
    }
    Ok(res)
}

/// Flush cost of small appends: 4 KiB records, and one day of 1,000 people's events (320 KB).
pub fn fsync_bench(dir: &Path) -> Result<Value, String> {
    use std::io::Write;
    let mut out = json!({});
    for (name, size, n) in [("append_4k", 4096usize, 60usize), ("append_day_1000p", 320_000, 30)] {
        let p = dir.join(format!("{name}.bin"));
        let mut f = std::fs::OpenOptions::new().create(true).append(true).open(&p).map_err(|e| e.to_string())?;
        let buf = vec![7u8; size];
        let (mut w, mut s) = (vec![], vec![]);
        for _ in 0..n {
            let t = Instant::now();
            f.write_all(&buf).map_err(|e| e.to_string())?;
            w.push(ms(t));
            let t = Instant::now();
            f.sync_data().map_err(|e| e.to_string())?;
            s.push(ms(t));
        }
        out[name] = json!({"write_ms": stat(w), "fsync_ms": stat(s)});
        let _ = std::fs::remove_file(&p);
    }
    Ok(out)
}

/// One generation method at reduced size (gen_w x gen_w/2 cells), one thread; hash to compare bits.
pub fn gen_bench(gen_w: usize, reps: usize) -> Result<Value, String> {
    let mut out = json!({});
    for method in ["plates"] {
        let mut t = vec![];
        let mut hsh = 0u64;
        for _ in 0..reps {
            let s = Instant::now();
            let m = terrain::generate(method, 7, gen_w, gen_w / 2, 1);
            t.push(ms(s));
            hsh = terrain::map_hash(&m);
        }
        out[method] = json!({"w": gen_w, "h": gen_w / 2, "ms": stat(t), "hash": format!("{hsh:016x}")});
    }
    Ok(out)
}

/// Metre detail for 1 km² around a camp (height map with 3D pieces), one thread and four.
pub fn detail_bench(reps: usize) -> Result<Value, String> {
    let site = terrain::Site::new(99, [310.0, 342.0, 365.0, 330.0]);
    let mut out = json!({});
    for th in [1usize, 4] {
        let mut t = vec![];
        let (mut hsh, mut bytes, mut pieces) = (0u64, 0usize, 0usize);
        for _ in 0..reps {
            let s = Instant::now();
            let hp = terrain::build_height_pieces(&site, th);
            t.push(ms(s));
            hsh = hp.hash();
            bytes = hp.bytes();
            pieces = hp.pieces.len();
        }
        out[format!("threads_{th}")] = json!({"ms": stat(t), "hash": format!("{hsh:016x}"), "bytes": bytes, "pieces": pieces});
    }
    Ok(out)
}
