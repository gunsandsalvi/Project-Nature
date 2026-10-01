//! B04-2: saved moments in SQLite (WAL mode). One database holds the moments; each moment is
//! written in one transaction, so a kill mid-write leaves the previous moments as they were.
//! Map cells and patches go in as one blob per region (compressed by the codec); people as one
//! row each (core blob, memories and relations in a compressed blob); animals and things as plain rows.
//! The moment row keeps a content hash, checked on load (SQLite itself has no page checksums).

use crate::data::*;
use crate::save::{chunk_state, compress, decompress, reg_grid, Codec};
use rusqlite::{params, Connection};
use std::path::Path;
use std::time::Instant;

pub fn open(path: &Path) -> rusqlite::Result<Connection> {
    let c = Connection::open(path)?;
    let _: String = c.query_row("PRAGMA journal_mode=WAL", [], |r| r.get(0))?;
    c.execute_batch(
        "PRAGMA synchronous=FULL;
         CREATE TABLE IF NOT EXISTS moment(id INTEGER PRIMARY KEY, day INTEGER, seed INTEGER, w INTEGER, h INTEGER,
            np INTEGER, na INTEGER, nt INTEGER, codec INTEGER, hash INTEGER);
         CREATE TABLE IF NOT EXISTS chunk(moment INTEGER, region INTEGER, layer INTEGER, raw_len INTEGER, data BLOB);
         CREATE INDEX IF NOT EXISTS chunk_i ON chunk(moment, region);
         CREATE TABLE IF NOT EXISTS person(moment INTEGER, id INTEGER, region INTEGER, core BLOB, var_len INTEGER, var BLOB);
         CREATE INDEX IF NOT EXISTS person_i ON person(moment, region);
         CREATE TABLE IF NOT EXISTS animal(moment INTEGER, id INTEGER, region INTEGER, species INTEGER, flags INTEGER,
            x INTEGER, y INTEGER, energy REAL, health REAL, age INTEGER, herd INTEGER);
         CREATE INDEX IF NOT EXISTS animal_i ON animal(moment, region);
         CREATE TABLE IF NOT EXISTS thing(moment INTEGER, id INTEGER, region INTEGER, kind INTEGER, material INTEGER,
            x INTEGER, y INTEGER, z INTEGER, condition INTEGER, mass INTEGER, owner INTEGER, made INTEGER);
         CREATE INDEX IF NOT EXISTS thing_i ON thing(moment, region);",
    )?;
    Ok(c)
}

fn region_fn(w: usize, h: usize) -> impl Fn(u32, u32) -> i64 {
    let (rk, nx, ny) = reg_grid(w, h);
    move |x: u32, y: u32| (((y as usize >> 10) / rk % ny) * nx + ((x as usize >> 10) / rk % nx)) as i64
}

fn var_bytes(p: &Person) -> Vec<u8> {
    let mut v = Vec::with_capacity(4 + p.mem.len() * 24 + p.rel.len() * 16);
    v.extend_from_slice(&(p.mem.len() as u16).to_le_bytes());
    v.extend_from_slice(&(p.rel.len() as u16).to_le_bytes());
    v.extend_from_slice(as_bytes(&p.mem));
    v.extend_from_slice(as_bytes(&p.rel));
    v
}

/// Write moment `id`; keep only the newest `keep` moments (0 = keep all). Returns
/// (milliseconds to insert and commit, milliseconds for the checkpoint into the main file).
/// `pause_ms` (test only) sleeps halfway through the things, inside the transaction.
pub fn save(c: &mut Connection, id: i64, s: &State, codec: Codec, keep: i64, pause_ms: u64) -> rusqlite::Result<(f64, f64)> {
    let t0 = Instant::now();
    let reg = region_fn(s.w, s.h);
    let tx = c.transaction()?;
    {
        let chunks = chunk_state(s);
        let mut st = tx.prepare_cached("INSERT INTO chunk VALUES (?1, ?2, ?3, ?4, ?5)")?;
        for (r, layers) in chunks.iter().enumerate() {
            for l in 0..2 {
                st.execute(params![id, r as i64, l as i64, layers[l].len() as i64, compress(codec, &layers[l])])?;
            }
        }
        let mut st = tx.prepare_cached("INSERT INTO person VALUES (?1, ?2, ?3, ?4, ?5, ?6)")?;
        for p in &s.people {
            let v = var_bytes(p);
            st.execute(params![id, p.core.id as i64, reg(p.core.x, p.core.y), as_bytes(std::slice::from_ref(&p.core)), v.len() as i64, compress(codec, &v)])?;
        }
        let mut st = tx.prepare_cached("INSERT INTO animal VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)")?;
        for a in &s.animals {
            st.execute(params![id, a.id as i64, reg(a.x, a.y), a.species as i64, a.flags as i64, a.x as i64, a.y as i64, a.energy as f64, a.health as f64, a.age as i64, a.herd as i64])?;
        }
        let mut st = tx.prepare_cached("INSERT INTO thing VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12)")?;
        for (i, t) in s.things.iter().enumerate() {
            if pause_ms > 0 && i == s.things.len() / 2 {
                eprintln!("PAUSE moment={id}");
                std::thread::sleep(std::time::Duration::from_millis(pause_ms));
            }
            st.execute(params![id, t.id as i64, reg(t.x, t.y), t.kind as i64, t.material as i64, t.x as i64, t.y as i64, t.z as i64, t.condition as i64, t.mass as i64, t.owner as i64, t.made as i64])?;
        }
        tx.execute(
            "INSERT INTO moment VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)",
            params![id, s.day as i64, s.seed as i64, s.w as i64, s.h as i64, s.people.len() as i64, s.animals.len() as i64, s.things.len() as i64, codec as i64, content_hash(s) as i64],
        )?;
        if keep > 0 {
            for t in ["chunk", "person", "animal", "thing", "moment"] {
                let col = if t == "moment" { "id" } else { "moment" };
                tx.execute(&format!("DELETE FROM {t} WHERE {col} <= ?1"), params![id - keep])?;
            }
        }
    }
    tx.commit()?;
    let ins = t0.elapsed().as_secs_f64() * 1e3;
    let t1 = Instant::now();
    checkpoint(c)?;
    Ok((ins, t1.elapsed().as_secs_f64() * 1e3))
}

pub fn checkpoint(c: &Connection) -> rusqlite::Result<()> {
    let _: (i64, i64, i64) = c.query_row("PRAGMA wal_checkpoint(TRUNCATE)", [], |r| Ok((r.get(0)?, r.get(1)?, r.get(2)?)))?;
    Ok(())
}

pub fn newest(c: &Connection) -> rusqlite::Result<Option<i64>> {
    c.query_row("SELECT max(id) FROM moment", [], |r| r.get(0))
}

fn err(e: rusqlite::Error) -> String {
    e.to_string()
}

/// Load moment `id` in full and check its content hash.
pub fn load_all(c: &Connection, id: i64) -> Result<State, String> {
    let (day, seed, w, h, np, na, nt, codec, hsh): (i64, i64, i64, i64, i64, i64, i64, i64, i64) = c
        .query_row("SELECT day, seed, w, h, np, na, nt, codec, hash FROM moment WHERE id=?1", [id], |r| {
            Ok((r.get(0)?, r.get(1)?, r.get(2)?, r.get(3)?, r.get(4)?, r.get(5)?, r.get(6)?, r.get(7)?, r.get(8)?))
        })
        .map_err(err)?;
    let codec = Codec::from_u8(codec as u8).ok_or("codec")?;
    let (w, h) = (w as usize, h as usize);
    if w == 0 || h == 0 || w > W_KM || h > H_KM {
        return Err("bad map size".into());
    }
    let (rk, nx, _ny) = reg_grid(w, h);
    let mut s = State { day: day as u64, seed: seed as u64, w, h, ..Default::default() };
    s.cells = vec![Cell::default(); w * h];
    s.patches = vec![Patch::default(); w * h * 16];
    s.people = vec![Person::default(); np as usize];
    s.animals = vec![Animal::default(); na as usize];
    s.things = vec![Thing::default(); nt as usize];
    let mut st = c.prepare("SELECT region, layer, raw_len, data FROM chunk WHERE moment=?1").map_err(err)?;
    let mut rows = st.query([id]).map_err(err)?;
    while let Some(r) = rows.next().map_err(err)? {
        let (reg, layer, raw_len): (i64, i64, i64) = (r.get(0).map_err(err)?, r.get(1).map_err(err)?, r.get(2).map_err(err)?);
        let blob = r.get_ref(3).map_err(err)?.as_blob().map_err(|e| e.to_string())?;
        let raw = decompress(codec, blob, raw_len as usize)?;
        let (rx, ry) = (reg as usize % nx, reg as usize / nx);
        if layer == 0 {
            let cc: Vec<Cell> = from_bytes(&raw).ok_or("cells")?;
            if cc.len() != rk * rk {
                return Err("cells size".into());
            }
            for y in 0..rk {
                let d = (ry * rk + y) * w + rx * rk;
                s.cells[d..d + rk].copy_from_slice(&cc[y * rk..(y + 1) * rk]);
            }
        } else {
            let pc: Vec<Patch> = from_bytes(&raw).ok_or("patches")?;
            let (pw, pk) = (w * 4, rk * 4);
            if pc.len() != pk * pk {
                return Err("patches size".into());
            }
            for y in 0..pk {
                let d = (ry * pk + y) * pw + rx * pk;
                s.patches[d..d + pk].copy_from_slice(&pc[y * pk..(y + 1) * pk]);
            }
        }
    }
    read_people(c, codec, "SELECT core, var_len, var FROM person WHERE moment=?1", &[id], &mut |p| {
        let i = p.core.id as usize;
        if i >= s.people.len() {
            return Err("person id".into());
        }
        s.people[i] = p;
        Ok(())
    })?;
    read_animals(c, "SELECT id, species, flags, x, y, energy, health, age, herd FROM animal WHERE moment=?1", &[id], &mut |a| {
        let i = a.id as usize;
        if i >= s.animals.len() {
            return Err("animal id".into());
        }
        s.animals[i] = a;
        Ok(())
    })?;
    read_things(c, "SELECT id, kind, material, x, y, z, condition, mass, owner, made FROM thing WHERE moment=?1", &[id], &mut |t| {
        let i = t.id as usize;
        if i >= s.things.len() {
            return Err("thing id".into());
        }
        s.things[i] = t;
        Ok(())
    })?;
    if content_hash(&s) as i64 != hsh {
        return Err("content hash differs".into());
    }
    Ok(s)
}

fn read_people(c: &Connection, codec: Codec, q: &str, args: &[i64], f: &mut dyn FnMut(Person) -> Result<(), String>) -> Result<(), String> {
    let mut st = c.prepare(q).map_err(err)?;
    let mut rows = st.query(rusqlite::params_from_iter(args.iter())).map_err(err)?;
    while let Some(r) = rows.next().map_err(err)? {
        let core: Vec<PersonCore> = from_bytes(r.get_ref(0).map_err(err)?.as_blob().map_err(|e| e.to_string())?).ok_or("core")?;
        let vl: i64 = r.get(1).map_err(err)?;
        let v = decompress(codec, r.get_ref(2).map_err(err)?.as_blob().map_err(|e| e.to_string())?, vl as usize)?;
        if core.len() != 1 || v.len() < 4 {
            return Err("person row".into());
        }
        let nm = u16::from_le_bytes([v[0], v[1]]) as usize;
        let nr = u16::from_le_bytes([v[2], v[3]]) as usize;
        if v.len() != 4 + nm * 24 + nr * 16 {
            return Err("person var".into());
        }
        f(Person { core: core[0], mem: from_bytes(&v[4..4 + nm * 24]).unwrap(), rel: from_bytes(&v[4 + nm * 24..]).unwrap() })?;
    }
    Ok(())
}

fn read_animals(c: &Connection, q: &str, args: &[i64], f: &mut dyn FnMut(Animal) -> Result<(), String>) -> Result<(), String> {
    let mut st = c.prepare(q).map_err(err)?;
    let mut rows = st.query(rusqlite::params_from_iter(args.iter())).map_err(err)?;
    while let Some(r) = rows.next().map_err(err)? {
        let g = |i: usize| -> Result<i64, String> { r.get::<_, i64>(i).map_err(err) };
        let gf = |i: usize| -> Result<f64, String> { r.get::<_, f64>(i).map_err(err) };
        f(Animal { id: g(0)? as u32, species: g(1)? as u16, flags: g(2)? as u16, x: g(3)? as u32, y: g(4)? as u32, energy: gf(5)? as f32, health: gf(6)? as f32, age: g(7)? as u32, herd: g(8)? as u32 })?;
    }
    Ok(())
}

fn read_things(c: &Connection, q: &str, args: &[i64], f: &mut dyn FnMut(Thing) -> Result<(), String>) -> Result<(), String> {
    let mut st = c.prepare(q).map_err(err)?;
    let mut rows = st.query(rusqlite::params_from_iter(args.iter())).map_err(err)?;
    while let Some(r) = rows.next().map_err(err)? {
        let g = |i: usize| -> Result<i64, String> { r.get::<_, i64>(i).map_err(err) };
        f(Thing {
            id: g(0)? as u32,
            kind: g(1)? as u16,
            material: g(2)? as u16,
            x: g(3)? as u32,
            y: g(4)? as u32,
            z: g(5)? as i16,
            condition: g(6)? as u16,
            mass: g(7)? as u32,
            owner: g(8)? as u32,
            made: g(9)? as i32,
        })?;
    }
    Ok(())
}

/// Load one region of moment `id` (cells, patches, people, animals, things), using the region indexes.
pub fn load_region(c: &Connection, id: i64, region: i64) -> Result<crate::save::RegionData, String> {
    let codec: i64 = c.query_row("SELECT codec FROM moment WHERE id=?1", [id], |r| r.get(0)).map_err(err)?;
    let codec = Codec::from_u8(codec as u8).ok_or("codec")?;
    let mut rd = crate::save::RegionData::default();
    let mut st = c.prepare("SELECT layer, raw_len, data FROM chunk WHERE moment=?1 AND region=?2").map_err(err)?;
    let mut rows = st.query([id, region]).map_err(err)?;
    while let Some(r) = rows.next().map_err(err)? {
        let (layer, raw_len): (i64, i64) = (r.get(0).map_err(err)?, r.get(1).map_err(err)?);
        let raw = decompress(codec, r.get_ref(2).map_err(err)?.as_blob().map_err(|e| e.to_string())?, raw_len as usize)?;
        if layer == 0 {
            rd.cells = from_bytes(&raw).ok_or("cells")?;
        } else {
            rd.patches = from_bytes(&raw).ok_or("patches")?;
        }
    }
    read_people(c, codec, "SELECT core, var_len, var FROM person WHERE moment=?1 AND region=?2", &[id, region], &mut |p| {
        rd.people.push(p);
        Ok(())
    })?;
    read_animals(c, "SELECT id, species, flags, x, y, energy, health, age, herd FROM animal WHERE moment=?1 AND region=?2", &[id, region], &mut |a| {
        rd.animals.push(a);
        Ok(())
    })?;
    read_things(c, "SELECT id, kind, material, x, y, z, condition, mass, owner, made FROM thing WHERE moment=?1 AND region=?2", &[id, region], &mut |t| {
        rd.things.push(t);
        Ok(())
    })?;
    Ok(rd)
}
