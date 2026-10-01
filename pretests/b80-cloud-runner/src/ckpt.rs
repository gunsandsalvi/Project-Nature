//! B80 / B07 stand-in: checkpoints ("saved moments", X4) written so a kill can never leave a
//! half-written one to load: write a temp file, fsync it, rename it, fsync the folder.
//! A checkpoint is loaded only if its length, header, parameters and trailing hash all check out;
//! anything else is renamed aside (".rejected") and the next older one is tried.

use crate::rng::splitmix64;
use crate::world::{Agent, Params, World};
use std::fs::{self, File};
use std::io::Write;
use std::path::{Path, PathBuf};
use std::time::Instant;

const MAGIC: &[u8; 8] = b"B80CKPT1";
const VERSION: u32 = 1;
const HEADER: usize = 64;
const AGENT_BYTES: usize = 24;

/// Hash of a byte string (also the run's checksum). Word-wise splitmix64.
pub fn hash_bytes(b: &[u8]) -> u64 {
    let mut h: u64 = 0x243F_6A88_85A3_08D3 ^ (b.len() as u64);
    let mut chunks = b.chunks_exact(8);
    for c in &mut chunks {
        h = splitmix64(h ^ u64::from_le_bytes(c.try_into().unwrap()));
    }
    let rem = chunks.remainder();
    if !rem.is_empty() {
        let mut buf = [0u8; 8];
        buf[..rem.len()].copy_from_slice(rem);
        h = splitmix64(h ^ u64::from_le_bytes(buf) ^ 0xFF);
    }
    h
}

/// The whole saved state (header and body) in a fixed little-endian layout.
pub fn encode(w: &World) -> Vec<u8> {
    let mut b = Vec::with_capacity(HEADER + w.field.len() * 4 + w.agents.len() * AGENT_BYTES + 8);
    b.extend_from_slice(MAGIC);
    b.extend_from_slice(&VERSION.to_le_bytes());
    b.extend_from_slice(&w.p.w.to_le_bytes());
    b.extend_from_slice(&w.p.h.to_le_bytes());
    b.extend_from_slice(&w.p.n0.to_le_bytes());
    b.extend_from_slice(&w.p.cap.to_le_bytes());
    b.extend_from_slice(&w.p.mind_iters.to_le_bytes());
    b.extend_from_slice(&w.p.seed.to_le_bytes());
    b.extend_from_slice(&w.day.to_le_bytes());
    b.extend_from_slice(&w.next_id.to_le_bytes());
    b.extend_from_slice(&(w.agents.len() as u64).to_le_bytes());
    debug_assert_eq!(b.len(), HEADER);
    for v in &w.field {
        b.extend_from_slice(&v.to_bits().to_le_bytes());
    }
    for a in &w.agents {
        b.extend_from_slice(&a.id.to_le_bytes());
        b.extend_from_slice(&a.energy.to_bits().to_le_bytes());
        b.extend_from_slice(&a.mind.to_bits().to_le_bytes());
        b.extend_from_slice(&a.age.to_le_bytes());
        b.extend_from_slice(&a.x.to_le_bytes());
        b.extend_from_slice(&a.y.to_le_bytes());
    }
    b
}

/// Checksum of the full state: the hash of its encoding.
pub fn checksum(w: &World) -> u64 {
    hash_bytes(&encode(w))
}

pub fn decode(b: &[u8], expect: &Params) -> Result<World, String> {
    if b.len() < HEADER + 8 {
        return Err(format!("too short ({} bytes)", b.len()));
    }
    if &b[0..8] != MAGIC {
        return Err("bad magic".into());
    }
    let u32at = |o: usize| u32::from_le_bytes(b[o..o + 4].try_into().unwrap());
    let u64at = |o: usize| u64::from_le_bytes(b[o..o + 8].try_into().unwrap());
    if u32at(8) != VERSION {
        return Err("bad version".into());
    }
    let p = Params {
        w: u32at(12),
        h: u32at(16),
        n0: u32at(20),
        cap: u32at(24),
        mind_iters: u32at(28),
        seed: u64at(32),
    };
    if p != *expect {
        return Err(format!("parameters differ: {:?}", p));
    }
    let (day, next_id, nag) = (u64at(40), u64at(48), u64at(56) as usize);
    if nag > p.cap as usize {
        return Err("too many agents".into());
    }
    let ncell = (p.w * p.h) as usize;
    let want = HEADER + ncell * 4 + nag * AGENT_BYTES + 8;
    if b.len() != want {
        return Err(format!("length {} but expected {}", b.len(), want));
    }
    let (h, trailer) = (hash_bytes(&b[..want - 8]), u64at(want - 8));
    if h != trailer {
        return Err(format!("hash {:016x} but trailer says {:016x}", h, trailer));
    }
    let mut w = World::alloc(p);
    w.day = day;
    w.next_id = next_id;
    let mut o = HEADER;
    for c in 0..ncell {
        w.field[c] = f32::from_bits(u32at(o));
        o += 4;
    }
    for _ in 0..nag {
        let x = u16::from_le_bytes(b[o + 20..o + 22].try_into().unwrap());
        let y = u16::from_le_bytes(b[o + 22..o + 24].try_into().unwrap());
        if x as u32 >= p.w || y as u32 >= p.h {
            return Err("agent off the map".into());
        }
        w.agents.push(Agent {
            id: u64at(o),
            energy: f32::from_bits(u32at(o + 8)),
            mind: f32::from_bits(u32at(o + 12)),
            age: u32at(o + 16),
            x,
            y,
        });
        o += AGENT_BYTES;
    }
    Ok(w)
}

#[derive(Clone, Copy, Default, Debug)]
pub struct SaveTimes {
    pub bytes: usize,
    pub encode_us: f64,
    pub write_us: f64,
    pub fsync_us: f64,
    pub rename_us: f64,
    pub total_us: f64,
}

pub fn name_of(day: u64) -> String {
    format!("ckpt-{:010}.bin", day)
}

fn day_of(name: &str) -> Option<u64> {
    let d = name.strip_prefix("ckpt-")?.strip_suffix(".bin")?;
    if d.len() == 10 && d.bytes().all(|c| c.is_ascii_digit()) {
        d.parse().ok()
    } else {
        None
    }
}

/// Checkpoints in `dir`, newest first.
pub fn list(dir: &Path) -> Vec<(u64, PathBuf)> {
    let mut v: Vec<(u64, PathBuf)> = fs::read_dir(dir)
        .map(|rd| {
            rd.flatten()
                .filter_map(|e| day_of(&e.file_name().to_string_lossy()).map(|d| (d, e.path())))
                .collect()
        })
        .unwrap_or_default();
    v.sort_by(|a, b| b.0.cmp(&a.0));
    v
}

fn us(a: Instant, b: Instant) -> f64 {
    (b - a).as_secs_f64() * 1e6
}

/// Save atomically. `slow_ms` > 0 pauses halfway through the write (test of a kill mid-write).
pub fn save(dir: &Path, w: &World, slow_ms: u64, keep: usize) -> std::io::Result<SaveTimes> {
    let t0 = Instant::now();
    let mut b = encode(w);
    let h = hash_bytes(&b);
    b.extend_from_slice(&h.to_le_bytes());
    let t1 = Instant::now();
    let fin = dir.join(name_of(w.day));
    let tmp = dir.join(format!("{}.tmp", name_of(w.day)));
    let mut f = File::create(&tmp)?;
    if slow_ms > 0 {
        let half = b.len() / 2;
        f.write_all(&b[..half])?;
        eprintln!("CKPT-WRITING day={} written={} of={}", w.day, half, b.len());
        std::thread::sleep(std::time::Duration::from_millis(slow_ms));
        f.write_all(&b[half..])?;
    } else {
        f.write_all(&b)?;
    }
    let t2 = Instant::now();
    f.sync_all()?;
    drop(f);
    let t3 = Instant::now();
    fs::rename(&tmp, &fin)?;
    File::open(dir)?.sync_all()?;
    let t4 = Instant::now();
    if keep > 0 {
        for (_, old) in list(dir).into_iter().skip(keep) {
            let _ = fs::remove_file(old);
        }
    }
    Ok(SaveTimes {
        bytes: b.len(),
        encode_us: us(t0, t1),
        write_us: us(t1, t2),
        fsync_us: us(t2, t3),
        rename_us: us(t3, t4),
        total_us: us(t0, t4),
    })
}

/// Load the newest checkpoint that passes every check. Temp files are never loaded: they are
/// leftovers of a write that was cut off, and are removed.
pub fn load_newest(dir: &Path, expect: &Params) -> Option<World> {
    if let Ok(rd) = fs::read_dir(dir) {
        for e in rd.flatten() {
            let name = e.file_name().to_string_lossy().to_string();
            if name.ends_with(".tmp") {
                let len = e.metadata().map(|m| m.len()).unwrap_or(0);
                let _ = fs::remove_file(e.path());
                eprintln!("REMOVED-TEMP {} bytes={}", name, len);
            }
        }
    }
    for (day, path) in list(dir) {
        let name = path.file_name().unwrap().to_string_lossy().to_string();
        let res = fs::read(&path).map_err(|e| e.to_string()).and_then(|b| decode(&b, expect));
        match res {
            Ok(w) if w.day == day => {
                eprintln!("RESUME {} day={}", name, day);
                return Some(w);
            }
            Ok(w) => eprintln!("REJECT {}: day inside is {}", name, w.day),
            Err(e) => eprintln!("REJECT {}: {}", name, e),
        }
        let _ = fs::rename(&path, path.with_extension("bin.rejected"));
    }
    None
}
