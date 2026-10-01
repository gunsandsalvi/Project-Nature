//! B04-2: saved moment as one FlatBuffers buffer (zero-copy schema format, schema/moment.fbs).
//! File: buffer (or the buffer compressed whole) | raw length (8) | hash of the stored bytes (8) | "KFBEND01".
//! Struct vectors are fixed little-endian layouts, the same bytes as the in-memory records.

#[allow(dead_code, unused_imports, clippy::all)]
#[path = "moment_generated.rs"]
mod moment_generated;
use moment_generated::kfb;

use kstorage::data::*;
use kstorage::save::{compress, decompress, region_in, reg_grid, Codec, RegionData};
use std::path::Path;

fn cast<T>(b: &[u8]) -> &[T] {
    let sz = std::mem::size_of::<T>();
    assert!(std::mem::align_of::<T>() == 1 && b.len() % sz == 0);
    unsafe { std::slice::from_raw_parts(b.as_ptr() as *const T, b.len() / sz) }
}

fn region_hash(cells: &[u8], patches: &[u8], people: &[(&[u8], &[u8], &[u8])], animals: &[u8], things: &[u8]) -> u64 {
    let mut h = mix(hash(cells) ^ mix(hash(patches)));
    for p in people {
        h = mix(h ^ hash(p.0) ^ mix(hash(p.1) ^ mix(hash(p.2))));
    }
    mix(h ^ hash(animals) ^ mix(hash(things)))
}

pub fn encode(s: &State, codec: Codec) -> Vec<u8> {
    let chunks = kstorage::save::chunk_state(s);
    let nreg = chunks.len();
    let mut by_reg: Vec<Vec<usize>> = vec![Vec::new(); nreg];
    for (i, p) in s.people.iter().enumerate() {
        by_reg[region_in(s.w, s.h, p.core.x, p.core.y)].push(i);
    }
    let mut b = flatbuffers::FlatBufferBuilder::with_capacity(raw_bytes(s) + (1 << 20));
    let mut regs = Vec::with_capacity(nreg);
    for r in 0..nreg {
        let l = &chunks[r];
        let cells = b.create_vector_direct(cast::<kfb::Cell>(&l[0]));
        let patches = b.create_vector_direct(cast::<kfb::Patch>(&l[1]));
        let mut pv = Vec::new();
        let mut ph = Vec::new();
        for &i in &by_reg[r] {
            let p = &s.people[i];
            let mem = b.create_vector_direct(cast::<kfb::Memory>(as_bytes(&p.mem)));
            let rel = b.create_vector_direct(cast::<kfb::Relation>(as_bytes(&p.rel)));
            let core = cast::<kfb::PersonCore>(as_bytes(std::slice::from_ref(&p.core)))[0];
            pv.push(kfb::Person::create(&mut b, &kfb::PersonArgs { core: Some(&core), memories: Some(mem), relations: Some(rel) }));
            ph.push((as_bytes(std::slice::from_ref(&p.core)), as_bytes(&p.mem), as_bytes(&p.rel)));
        }
        let people = b.create_vector(&pv);
        let animals = b.create_vector_direct(cast::<kfb::Animal>(&l[4]));
        let things = b.create_vector_direct(cast::<kfb::Thing>(&l[5]));
        let rh = region_hash(&l[0], &l[1], &ph, &l[4], &l[5]);
        regs.push(kfb::Region::create(
            &mut b,
            &kfb::RegionArgs { id: r as u16, hash: rh, cells: Some(cells), patches: Some(patches), people: Some(people), animals: Some(animals), things: Some(things) },
        ));
    }
    let regions = b.create_vector(&regs);
    let m = kfb::Moment::create(
        &mut b,
        &kfb::MomentArgs { day: s.day, seed: s.seed, w: s.w as u32, h: s.h as u32, np: s.people.len() as u32, na: s.animals.len() as u32, nt: s.things.len() as u32, regions: Some(regions) },
    );
    b.finish(m, None);
    let fbuf = b.finished_data();
    let mut out = compress(codec, fbuf);
    let h = hash(&out);
    out.extend_from_slice(&(fbuf.len() as u64).to_le_bytes());
    out.extend_from_slice(&h.to_le_bytes());
    out.push(codec as u8);
    out.extend_from_slice(b"KFBEND0");
    out
}

/// Check the trailer and hash, and return the buffer (decompressed if needed).
fn unwrap(b: &[u8]) -> Result<std::borrow::Cow<'_, [u8]>, String> {
    if b.len() < 24 || &b[b.len() - 7..] != b"KFBEND0" {
        return Err("missing end mark".into());
    }
    let n = b.len() - 24;
    let raw_len = u64::from_le_bytes(b[n..n + 8].try_into().unwrap()) as usize;
    let h = u64::from_le_bytes(b[n + 8..n + 16].try_into().unwrap());
    let codec = Codec::from_u8(b[n + 16]).ok_or("codec")?;
    if hash(&b[..n]) != h {
        return Err("hash differs".into());
    }
    if codec == Codec::None {
        if raw_len != n {
            return Err("length".into());
        }
        Ok(std::borrow::Cow::Borrowed(&b[..n]))
    } else {
        Ok(std::borrow::Cow::Owned(decompress(codec, &b[..n], raw_len)?))
    }
}

fn copy<T: Pod>(b: &[u8]) -> Vec<T> {
    from_bytes(b).unwrap()
}
fn sbytes<T>(v: &[T]) -> &[u8] {
    unsafe { std::slice::from_raw_parts(v.as_ptr() as *const u8, std::mem::size_of_val(v)) }
}

pub fn load_bytes(b: &[u8]) -> Result<State, String> {
    let buf = unwrap(b)?;
    let m = kfb::root_as_moment(&buf).map_err(|e| e.to_string())?;
    let (w, h) = (m.w() as usize, m.h() as usize);
    if w == 0 || h == 0 || w > W_KM || h > H_KM {
        return Err("bad map size".into());
    }
    let (rk, nx, _) = reg_grid(w, h);
    let mut s = State { day: m.day(), seed: m.seed(), w, h, ..Default::default() };
    s.cells = vec![Cell::default(); w * h];
    s.patches = vec![Patch::default(); w * h * 16];
    s.people = vec![Person::default(); m.np() as usize];
    s.animals = vec![Animal::default(); m.na() as usize];
    s.things = vec![Thing::default(); m.nt() as usize];
    for (r, reg) in m.regions().ok_or("regions")?.iter().enumerate() {
        let (rx, ry) = (r % nx, r / nx);
        let c: Vec<Cell> = copy(sbytes(reg.cells().ok_or("cells")?));
        if c.len() != rk * rk {
            return Err("cells size".into());
        }
        for y in 0..rk {
            let d = (ry * rk + y) * w + rx * rk;
            s.cells[d..d + rk].copy_from_slice(&c[y * rk..(y + 1) * rk]);
        }
        let pc: Vec<Patch> = copy(sbytes(reg.patches().ok_or("patches")?));
        let (pw, pk) = (w * 4, rk * 4);
        if pc.len() != pk * pk {
            return Err("patches size".into());
        }
        for y in 0..pk {
            let d = (ry * pk + y) * pw + rx * pk;
            s.patches[d..d + pk].copy_from_slice(&pc[y * pk..(y + 1) * pk]);
        }
        for p in reg.people().ok_or("people")?.iter() {
            let core: Vec<PersonCore> = copy(sbytes(std::slice::from_ref(p.core().ok_or("core")?)));
            let id = core[0].id as usize;
            if id >= s.people.len() {
                return Err("person id".into());
            }
            s.people[id] = Person { core: core[0], mem: copy(sbytes(p.memories().ok_or("mem")?)), rel: copy(sbytes(p.relations().ok_or("rel")?)) };
        }
        for a in copy::<Animal>(sbytes(reg.animals().ok_or("animals")?)) {
            let id = a.id as usize;
            if id >= s.animals.len() {
                return Err("animal id".into());
            }
            s.animals[id] = a;
        }
        for t in copy::<Thing>(sbytes(reg.things().ok_or("things")?)) {
            let id = t.id as usize;
            if id >= s.things.len() {
                return Err("thing id".into());
            }
            s.things[id] = t;
        }
    }
    Ok(s)
}

pub fn load_all(p: &Path) -> Result<State, String> {
    load_bytes(&std::fs::read(p).map_err(|e| e.to_string())?)
}

/// One region. Uncompressed: map the file and touch only that region (zero-copy), checked by
/// the buffer verifier and the region's own hash. Compressed: the whole file must be read.
pub fn load_region(p: &Path, region: usize) -> Result<RegionData, String> {
    let f = std::fs::File::open(p).map_err(|e| e.to_string())?;
    let len = f.metadata().map_err(|e| e.to_string())?.len() as usize;
    use std::os::unix::io::AsRawFd;
    let ptr = unsafe { libc::mmap(std::ptr::null_mut(), len, libc::PROT_READ, libc::MAP_PRIVATE, f.as_raw_fd(), 0) };
    if ptr == libc::MAP_FAILED {
        return Err("mmap failed".into());
    }
    let b: &[u8] = unsafe { std::slice::from_raw_parts(ptr as *const u8, len) };
    let res = (|| {
        if len < 24 || &b[len - 7..] != b"KFBEND0" {
            return Err("missing end mark".to_string());
        }
        let codec = Codec::from_u8(b[len - 8]).ok_or("codec")?;
        let owned;
        let buf: &[u8] = if codec == Codec::None {
            &b[..len - 24]
        } else {
            owned = unwrap(b)?;
            &owned
        };
        let m = kfb::root_as_moment(buf).map_err(|e| e.to_string())?;
        let reg = m.regions().ok_or("regions")?;
        if region >= reg.len() {
            return Err("no such region".into());
        }
        let reg = reg.get(region);
        let mut rd = RegionData::default();
        rd.cells = copy(sbytes(reg.cells().ok_or("cells")?));
        rd.patches = copy(sbytes(reg.patches().ok_or("patches")?));
        let mut ph = Vec::new();
        for p in reg.people().ok_or("people")?.iter() {
            let core: Vec<PersonCore> = copy(sbytes(std::slice::from_ref(p.core().ok_or("core")?)));
            rd.people.push(Person { core: core[0], mem: copy(sbytes(p.memories().ok_or("mem")?)), rel: copy(sbytes(p.relations().ok_or("rel")?)) });
        }
        for p in &rd.people {
            ph.push((as_bytes(std::slice::from_ref(&p.core)), as_bytes(&p.mem), as_bytes(&p.rel)));
        }
        rd.animals = copy(sbytes(reg.animals().ok_or("animals")?));
        rd.things = copy(sbytes(reg.things().ok_or("things")?));
        let rh = region_hash(as_bytes(&rd.cells), as_bytes(&rd.patches), &ph, as_bytes(&rd.animals), as_bytes(&rd.things));
        if rh != reg.hash() {
            return Err("region hash differs".into());
        }
        Ok(rd)
    })();
    unsafe { libc::munmap(ptr, len) };
    res
}
