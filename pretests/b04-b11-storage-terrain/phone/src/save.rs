//! B04-2: the custom fixed-layout save file ("saved moment", `PRN-15`, `PLT-07`), as in `B80`
//! but cut into region chunks so one region loads alone. Written to a temp file, flushed,
//! renamed, folder flushed. Loaded only if the table of contents and every chunk pass their hash.
//!
//! File: header (64) | chunks | table of contents (32 per chunk) | trailer (16).
//! Chunk layers per region: 0 cells, 1 patches, 2 people cores, 3 people memories+relations,
//! 4 animals, 5 things. Chunks are compressed one by one (none, lz4 or zstd level 1).

use crate::data::*;
use std::fs::{self, File};
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::Path;
use std::time::Instant;

pub const MAGIC: &[u8; 8] = b"KSAVE001";
pub const END: &[u8; 8] = b"KEND0001";
pub const LAYERS: usize = 6;

#[derive(Clone, Copy, PartialEq, Debug)]
pub enum Codec {
    None = 0,
    Lz4 = 1,
    Zstd = 2,
}
impl Codec {
    pub fn from_u8(v: u8) -> Option<Codec> {
        match v {
            0 => Some(Codec::None),
            1 => Some(Codec::Lz4),
            2 => Some(Codec::Zstd),
            _ => None,
        }
    }
    pub fn name(self) -> &'static str {
        match self {
            Codec::None => "none",
            Codec::Lz4 => "lz4",
            Codec::Zstd => "zstd1",
        }
    }
    pub fn all() -> [Codec; 3] {
        [Codec::None, Codec::Lz4, Codec::Zstd]
    }
}

pub fn compress(c: Codec, b: &[u8]) -> Vec<u8> {
    match c {
        Codec::None => b.to_vec(),
        Codec::Lz4 => lz4_flex::block::compress(b),
        Codec::Zstd => zstd::bulk::compress(b, 1).expect("zstd"),
    }
}
pub fn decompress(c: Codec, b: &[u8], raw_len: usize) -> Result<Vec<u8>, String> {
    let out = match c {
        Codec::None => b.to_vec(),
        Codec::Lz4 => lz4_flex::block::decompress(b, raw_len).map_err(|e| e.to_string())?,
        Codec::Zstd => zstd::bulk::decompress(b, raw_len).map_err(|e| e.to_string())?,
    };
    if out.len() != raw_len {
        return Err("bad length after decompress".into());
    }
    Ok(out)
}

/// Region side in cells (128, smaller on small test worlds) and the number of regions across and down.
pub fn reg_grid(w: usize, h: usize) -> (usize, usize, usize) {
    let rk = REG_KM.min(w).min(h).max(1);
    (rk, w / rk, h / rk)
}

/// Region-bucketed raw chunks of a state: chunks[region][layer].
pub fn chunk_state(s: &State) -> Vec<[Vec<u8>; LAYERS]> {
    let (rk, nreg_x, nreg_y) = reg_grid(s.w, s.h);
    let nreg = nreg_x * nreg_y;
    let mut out: Vec<[Vec<u8>; LAYERS]> = (0..nreg).map(|_| Default::default()).collect();
    let reg = |x: u32, y: u32| ((y as usize >> 10) / rk % nreg_y) * nreg_x + ((x as usize >> 10) / rk % nreg_x);
    for ry in 0..nreg_y {
        for rx in 0..nreg_x {
            let r = ry * nreg_x + rx;
            let o = &mut out[r];
            for y in ry * rk..(ry + 1) * rk {
                o[0].extend_from_slice(as_bytes(&s.cells[y * s.w + rx * rk..y * s.w + (rx + 1) * rk]));
            }
            let (pw, pk) = (s.w * PATCH_PER_KM, rk * PATCH_PER_KM);
            for y in ry * pk..(ry + 1) * pk {
                o[1].extend_from_slice(as_bytes(&s.patches[y * pw + rx * pk..y * pw + (rx + 1) * pk]));
            }
        }
    }
    for p in &s.people {
        let o = &mut out[reg(p.core.x, p.core.y)];
        o[2].extend_from_slice(as_bytes(std::slice::from_ref(&p.core)));
        o[3].extend_from_slice(&(p.mem.len() as u16).to_le_bytes());
        o[3].extend_from_slice(&(p.rel.len() as u16).to_le_bytes());
        o[3].extend_from_slice(as_bytes(&p.mem));
        o[3].extend_from_slice(as_bytes(&p.rel));
    }
    for a in &s.animals {
        out[reg(a.x, a.y)][4].extend_from_slice(as_bytes(std::slice::from_ref(a)));
    }
    for t in &s.things {
        out[reg(t.x, t.y)][5].extend_from_slice(as_bytes(std::slice::from_ref(t)));
    }
    out
}

#[derive(Clone, Copy, Default, Debug)]
pub struct Entry {
    pub region: u16,
    pub layer: u16,
    pub codec: u8,
    pub raw_len: u32,
    pub stored_len: u32,
    pub off: u64,
    pub hash: u64,
}

#[derive(Clone, Copy, Default, Debug)]
pub struct Times {
    pub bytes: u64,
    pub encode_ms: f64,
    pub write_ms: f64,
    pub fsync_ms: f64,
    pub rename_ms: f64,
    pub total_ms: f64,
}

fn ms(a: Instant) -> f64 {
    a.elapsed().as_secs_f64() * 1e3
}

/// Header fields kept outside chunks: day, seed, map size, counts.
fn header(s: &State, codec: Codec, n_chunks: usize, toc_off: u64) -> [u8; 64] {
    let mut h = [0u8; 64];
    h[0..8].copy_from_slice(MAGIC);
    h[8..12].copy_from_slice(&1u32.to_le_bytes());
    h[12] = codec as u8;
    h[16..24].copy_from_slice(&s.day.to_le_bytes());
    h[24..32].copy_from_slice(&s.seed.to_le_bytes());
    h[32..36].copy_from_slice(&(s.w as u32).to_le_bytes());
    h[36..40].copy_from_slice(&(s.h as u32).to_le_bytes());
    h[40..44].copy_from_slice(&(s.people.len() as u32).to_le_bytes());
    h[44..48].copy_from_slice(&(s.animals.len() as u32).to_le_bytes());
    h[48..52].copy_from_slice(&(s.things.len() as u32).to_le_bytes());
    h[52..56].copy_from_slice(&(n_chunks as u32).to_le_bytes());
    h[56..64].copy_from_slice(&toc_off.to_le_bytes());
    h
}

/// Encode the whole file in memory.
pub fn encode(s: &State, codec: Codec) -> Vec<u8> {
    let chunks = chunk_state(s);
    let mut body: Vec<u8> = Vec::with_capacity(raw_bytes(s) / 2 + 4096);
    body.extend_from_slice(&[0u8; 64]);
    let mut toc: Vec<Entry> = Vec::new();
    for (r, layers) in chunks.iter().enumerate() {
        for (l, raw) in layers.iter().enumerate() {
            if raw.is_empty() {
                continue;
            }
            let st = compress(codec, raw);
            toc.push(Entry { region: r as u16, layer: l as u16, codec: codec as u8, raw_len: raw.len() as u32, stored_len: st.len() as u32, off: body.len() as u64, hash: hash(&st) });
            body.extend_from_slice(&st);
        }
    }
    let toc_off = body.len() as u64;
    body[0..64].copy_from_slice(&header(s, codec, toc.len(), toc_off));
    for e in &toc {
        body.extend_from_slice(&e.region.to_le_bytes());
        body.extend_from_slice(&e.layer.to_le_bytes());
        body.push(e.codec);
        body.extend_from_slice(&[0u8; 3]);
        body.extend_from_slice(&e.raw_len.to_le_bytes());
        body.extend_from_slice(&e.stored_len.to_le_bytes());
        body.extend_from_slice(&e.off.to_le_bytes());
        body.extend_from_slice(&e.hash.to_le_bytes());
    }
    // Trailer: hash of header and table of contents, then the end mark.
    let mut ht = body[0..64].to_vec();
    ht.extend_from_slice(&body[toc_off as usize..]);
    let th = hash(&ht);
    body.extend_from_slice(&th.to_le_bytes());
    body.extend_from_slice(END);
    body
}

/// Write bytes atomically: temp file, flush, rename, flush the folder (as `B80`).
/// `pause_at` (test only): stop after this many bytes and sleep, so a kill lands mid-write.
pub fn write_atomic(dir: &Path, name: &str, b: &[u8], pause_at: Option<(usize, u64)>) -> std::io::Result<(f64, f64, f64)> {
    let tmp = dir.join(format!("{name}.tmp"));
    let t = Instant::now();
    let mut f = File::create(&tmp)?;
    match pause_at {
        Some((at, sleep_ms)) if at < b.len() => {
            f.write_all(&b[..at])?;
            std::thread::sleep(std::time::Duration::from_millis(sleep_ms));
            f.write_all(&b[at..])?;
        }
        _ => {
            // Write in 4 MiB pieces, as a real saver streaming chunks would.
            for c in b.chunks(4 << 20) {
                f.write_all(c)?;
            }
        }
    }
    let w = ms(t);
    let t = Instant::now();
    f.sync_all()?;
    drop(f);
    let fs_ = ms(t);
    let t = Instant::now();
    fs::rename(&tmp, dir.join(name))?;
    File::open(dir)?.sync_all()?;
    Ok((w, fs_, ms(t)))
}

pub fn save(dir: &Path, name: &str, s: &State, codec: Codec) -> std::io::Result<Times> {
    let t0 = Instant::now();
    let b = encode(s, codec);
    let enc = ms(t0);
    let (w, f, r) = write_atomic(dir, name, &b, None)?;
    Ok(Times { bytes: b.len() as u64, encode_ms: enc, write_ms: w, fsync_ms: f, rename_ms: r, total_ms: ms(t0) })
}

/// Drop a file's pages from the page cache, so the next read comes from storage.
pub fn drop_cache(path: &Path) {
    #[cfg(any(target_os = "linux", target_os = "android"))]
    if let Ok(f) = File::open(path) {
        use std::os::unix::io::AsRawFd;
        unsafe {
            libc::posix_fadvise(f.as_raw_fd(), 0, 0, libc::POSIX_FADV_DONTNEED);
        }
    }
    #[cfg(not(any(target_os = "linux", target_os = "android")))]
    let _ = path;
}

pub struct Parsed {
    pub day: u64,
    pub seed: u64,
    pub w: usize,
    pub h: usize,
    pub n: [usize; 3],
    pub toc: Vec<Entry>,
}

fn u32at(b: &[u8], o: usize) -> u32 {
    u32::from_le_bytes(b[o..o + 4].try_into().unwrap())
}
fn u64at(b: &[u8], o: usize) -> u64 {
    u64::from_le_bytes(b[o..o + 8].try_into().unwrap())
}

/// Check header, table of contents and trailer. `file_len` is the whole file's length.
pub fn parse_meta(head: &[u8], toc_and_trailer: &[u8], file_len: u64) -> Result<Parsed, String> {
    if head.len() < 64 || &head[0..8] != MAGIC {
        return Err("bad magic".into());
    }
    if u32at(head, 8) != 1 {
        return Err("bad version".into());
    }
    let n_chunks = u32at(head, 52) as usize;
    let toc_off = u64at(head, 56);
    if toc_off < 64 || toc_off + (n_chunks as u64) * 32 + 16 != file_len {
        return Err(format!("length {file_len} does not match table of contents"));
    }
    let tt = toc_and_trailer;
    if tt.len() != n_chunks * 32 + 16 || &tt[tt.len() - 8..] != END {
        return Err("missing end mark".into());
    }
    let mut ht = head[0..64].to_vec();
    ht.extend_from_slice(&tt[..tt.len() - 16]);
    if hash(&ht) != u64at(tt, tt.len() - 16) {
        return Err("table of contents hash differs".into());
    }
    let mut toc = Vec::with_capacity(n_chunks);
    for i in 0..n_chunks {
        let e = &tt[i * 32..i * 32 + 32];
        let en = Entry {
            region: u16::from_le_bytes([e[0], e[1]]),
            layer: u16::from_le_bytes([e[2], e[3]]),
            codec: e[4],
            raw_len: u32at(e, 8),
            stored_len: u32at(e, 12),
            off: u64at(e, 16),
            hash: u64at(e, 24),
        };
        if en.off < 64 || en.off + en.stored_len as u64 > toc_off || en.layer as usize >= LAYERS || Codec::from_u8(en.codec).is_none() {
            return Err("bad chunk entry".into());
        }
        toc.push(en);
    }
    Ok(Parsed {
        day: u64at(head, 16),
        seed: u64at(head, 24),
        w: u32at(head, 32) as usize,
        h: u32at(head, 36) as usize,
        n: [u32at(head, 40) as usize, u32at(head, 44) as usize, u32at(head, 48) as usize],
        toc,
    })
}

fn chunk_raw(e: &Entry, stored: &[u8]) -> Result<Vec<u8>, String> {
    if hash(stored) != e.hash {
        return Err(format!("chunk {}/{} hash differs", e.region, e.layer));
    }
    decompress(Codec::from_u8(e.codec).unwrap(), stored, e.raw_len as usize)
}

/// Contents of one region, decoded.
#[derive(Default)]
pub struct RegionData {
    pub cells: Vec<Cell>,
    pub patches: Vec<Patch>,
    pub people: Vec<Person>,
    pub animals: Vec<Animal>,
    pub things: Vec<Thing>,
}

pub fn decode_layer(layer: usize, raw: &[u8], rd: &mut RegionData) -> Result<(), String> {
    match layer {
        0 => rd.cells = from_bytes(raw).ok_or("cells")?,
        1 => rd.patches = from_bytes(raw).ok_or("patches")?,
        2 => {
            let cores: Vec<PersonCore> = from_bytes(raw).ok_or("cores")?;
            // people may already hold memories from layer 3 when layers come in another order
            if rd.people.len() < cores.len() {
                rd.people.resize(cores.len(), Person::default());
            }
            for (p, c) in rd.people.iter_mut().zip(cores) {
                p.core = c;
            }
        }
        3 => {
            let mut o = 0usize;
            let mut i = 0usize;
            while o < raw.len() {
                if o + 4 > raw.len() {
                    return Err("people records cut".into());
                }
                let nm = u16::from_le_bytes([raw[o], raw[o + 1]]) as usize;
                let nr = u16::from_le_bytes([raw[o + 2], raw[o + 3]]) as usize;
                o += 4;
                let (lm, lr) = (nm * 24, nr * 16);
                if o + lm + lr > raw.len() {
                    return Err("people records cut".into());
                }
                if rd.people.len() <= i {
                    rd.people.push(Person::default());
                }
                rd.people[i].mem = from_bytes(&raw[o..o + lm]).unwrap();
                rd.people[i].rel = from_bytes(&raw[o + lm..o + lm + lr]).unwrap();
                o += lm + lr;
                i += 1;
            }
        }
        4 => rd.animals = from_bytes(raw).ok_or("animals")?,
        5 => rd.things = from_bytes(raw).ok_or("things")?,
        _ => return Err("bad layer".into()),
    }
    Ok(())
}

/// Read and check a whole saved moment, and rebuild the state in id order.
pub fn load_all(path: &Path) -> Result<State, String> {
    let b = fs::read(path).map_err(|e| e.to_string())?;
    load_bytes(&b)
}

pub fn load_bytes(b: &[u8]) -> Result<State, String> {
    if b.len() < 64 + 16 {
        return Err("too short".into());
    }
    let toc_off = u64at(b, 56) as usize;
    if toc_off < 64 || toc_off > b.len() {
        return Err("bad table offset".into());
    }
    let p = parse_meta(&b[..64], &b[toc_off..], b.len() as u64)?;
    let (w, h) = (p.w, p.h);
    if w == 0 || h == 0 || w > W_KM || h > H_KM {
        return Err("bad map size".into());
    }
    let (rk, nreg_x, nreg_y) = reg_grid(w, h);
    let mut s = State { day: p.day, seed: p.seed, w, h, ..Default::default() };
    s.cells = vec![Cell::default(); w * h];
    s.patches = vec![Patch::default(); w * h * PATCH_PER_KM * PATCH_PER_KM];
    s.people = vec![Person::default(); p.n[0]];
    s.animals = vec![Animal::default(); p.n[1]];
    s.things = vec![Thing::default(); p.n[2]];
    let mut seen = vec![[false; LAYERS]; nreg_x * nreg_y];
    let (mut np, mut na, mut nt) = (0usize, 0usize, 0usize);
    let mut cur_people: Vec<Person> = Vec::new();
    let mut cur_region = usize::MAX;
    for e in &p.toc {
        let r = e.region as usize;
        if r >= seen.len() || seen[r][e.layer as usize] {
            return Err("bad or repeated chunk".into());
        }
        seen[r][e.layer as usize] = true;
        let raw = chunk_raw(e, &b[e.off as usize..e.off as usize + e.stored_len as usize])?;
        let (rx, ry) = (r % nreg_x, r / nreg_x);
        if r != cur_region {
            cur_people.clear();
            cur_region = r;
        }
        match e.layer {
            0 => {
                let c: Vec<Cell> = from_bytes(&raw).ok_or("cells")?;
                if c.len() != rk * rk {
                    return Err("cells size".into());
                }
                for y in 0..rk {
                    let d = (ry * rk + y) * w + rx * rk;
                    s.cells[d..d + rk].copy_from_slice(&c[y * rk..(y + 1) * rk]);
                }
            }
            1 => {
                let pc: Vec<Patch> = from_bytes(&raw).ok_or("patches")?;
                let (pw, pk) = (w * PATCH_PER_KM, rk * PATCH_PER_KM);
                if pc.len() != pk * pk {
                    return Err("patches size".into());
                }
                for y in 0..pk {
                    let d = (ry * pk + y) * pw + rx * pk;
                    s.patches[d..d + pk].copy_from_slice(&pc[y * pk..(y + 1) * pk]);
                }
            }
            2 | 3 => {
                let mut rd = RegionData { people: std::mem::take(&mut cur_people), ..Default::default() };
                decode_layer(e.layer as usize, &raw, &mut rd)?;
                cur_people = rd.people;
                // a region's people are complete once both layers are in
                if seen[r][2] && seen[r][3] {
                    for pp in cur_people.drain(..) {
                        let id = pp.core.id as usize;
                        if id >= s.people.len() {
                            return Err("person id".into());
                        }
                        s.people[id] = pp;
                        np += 1;
                    }
                }
            }
            4 => {
                for a in from_bytes::<Animal>(&raw).ok_or("animals")? {
                    let id = a.id as usize;
                    if id >= s.animals.len() {
                        return Err("animal id".into());
                    }
                    s.animals[id] = a;
                    na += 1;
                }
            }
            _ => {
                for t in from_bytes::<Thing>(&raw).ok_or("things")? {
                    let id = t.id as usize;
                    if id >= s.things.len() {
                        return Err("thing id".into());
                    }
                    s.things[id] = t;
                    nt += 1;
                }
            }
        }
    }
    if np != p.n[0] || na != p.n[1] || nt != p.n[2] || seen.iter().any(|l| !l[0] || !l[1]) {
        return Err("missing records".into());
    }
    Ok(s)
}

/// Load one region: header, table of contents, then only that region's chunks.
pub fn load_region(path: &Path, region: usize) -> Result<RegionData, String> {
    let mut f = File::open(path).map_err(|e| e.to_string())?;
    let len = f.metadata().map_err(|e| e.to_string())?.len();
    let mut head = [0u8; 64];
    f.read_exact(&mut head).map_err(|e| e.to_string())?;
    let toc_off = u64at(&head, 56);
    if toc_off < 64 || toc_off > len {
        return Err("bad table offset".into());
    }
    let mut tt = vec![0u8; (len - toc_off) as usize];
    f.seek(SeekFrom::Start(toc_off)).map_err(|e| e.to_string())?;
    f.read_exact(&mut tt).map_err(|e| e.to_string())?;
    let p = parse_meta(&head, &tt, len)?;
    let mut rd = RegionData::default();
    let mut order: Vec<&Entry> = p.toc.iter().filter(|e| e.region as usize == region).collect();
    order.sort_by_key(|e| e.layer);
    for e in order {
        let mut st = vec![0u8; e.stored_len as usize];
        f.seek(SeekFrom::Start(e.off)).map_err(|e| e.to_string())?;
        f.read_exact(&mut st).map_err(|e| e.to_string())?;
        let raw = chunk_raw(e, &st)?;
        decode_layer(e.layer as usize, &raw, &mut rd)?;
    }
    Ok(rd)
}
