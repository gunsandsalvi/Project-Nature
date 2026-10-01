//! B04-3: the history log (`PRN-15`): what happened, to whom, where and when.
//! Custom log: the open year goes to an append-only journal (one block per simulated day, with
//! a hash; flushed each day). At each year's end the year is sorted by region, person and time
//! and written as one segment file (temp, flush, rename), with a small index:
//! region -> run of events, person -> runs. Blocks of 4,096 events, stored raw or as
//! delta-coded columns compressed with lz4 or zstd.
//! SQLite log: one table, indexes on (actor, day) and (region, day), one transaction per day.

use crate::data::*;
use crate::save::{compress, decompress, write_atomic, Codec};
use std::fs::{self, File, OpenOptions};
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::{Path, PathBuf};

#[repr(C)]
#[derive(Clone, Copy, Default, PartialEq, Eq, Debug)]
pub struct Event {
    pub day: u32,
    pub minute: u16,
    pub kind: u16,
    pub actor: u32,
    pub target: u32,
    pub x: u32,
    pub y: u32,
    pub payload: u32,
    pub extra: u32,
} // 32 bytes
unsafe impl Pod for Event {}

pub const PER_DAY: u32 = 10; // events per person per day (NOTES.md)
pub const DAYS_PER_YEAR: u32 = 365;
const BLOCK: usize = 4096;

pub fn region(e: &Event) -> u16 {
    region_of(e.x, e.y) as u16
}

/// Synthetic events of one day. Bands of 25 share a camp that moves every 90 days within
/// a home range of +-30 km.
pub fn day_events(seed: u64, n_people: u32, day: u32, out: &mut Vec<Event>) {
    out.clear();
    let xmax = 1u32 << 21;
    let ymax = 1u32 << 20;
    for p in 0..n_people {
        let band = p / 25;
        let home = key(seed, 0xB0D, band as u64);
        let mv = key(seed, 0xB0E + band as u64, (day / 90) as u64);
        let cx = (home as u32 % xmax) as i64 + (mv % 60_001) as i64 - 30_000;
        let cy = ((home >> 32) as u32 % ymax) as i64 + ((mv >> 32) % 60_001) as i64 - 30_000;
        for k in 0..PER_DAY {
            let r = key(seed ^ day as u64, p as u64, k as u64);
            let target = match r % 10 {
                0..=4 => band * 25 + ((r >> 8) % 25) as u32,
                5..=7 => 100_000 + ((r >> 8) % 100_000) as u32,
                _ => 1_000_000 + ((r >> 8) % 1_000_000) as u32,
            };
            out.push(Event {
                day,
                minute: (k * 144 + ((r >> 30) % 144) as u32) as u16,
                kind: ((r >> 40) % 60) as u16,
                actor: p,
                target,
                x: ((cx + ((r >> 20) % 4001) as i64 - 2000).rem_euclid(xmax as i64)) as u32,
                y: ((cy + ((r >> 44) % 4001) as i64 - 2000).rem_euclid(ymax as i64)) as u32,
                payload: ((r >> 50) % 1000) as u32,
                extra: if (r >> 60) == 0 { (r >> 8) as u32 & 0xFFFF } else { 0 },
            });
        }
    }
}

// ---------- block encoding: raw, or delta-coded columns then compressed ----------

fn enc_block(ev: &[Event], codec: Codec) -> Vec<u8> {
    if codec == Codec::None {
        return as_bytes(ev).to_vec();
    }
    let n = ev.len();
    let mut c = Vec::with_capacity(n * 32);
    let mut prev = Event::default();
    for e in ev {
        c.extend_from_slice(&e.day.wrapping_sub(prev.day).to_le_bytes());
        prev.day = e.day;
    }
    for e in ev {
        c.extend_from_slice(&e.minute.to_le_bytes());
    }
    for e in ev {
        c.extend_from_slice(&e.kind.to_le_bytes());
    }
    for e in ev {
        c.extend_from_slice(&e.actor.wrapping_sub(prev.actor).to_le_bytes());
        prev.actor = e.actor;
    }
    for e in ev {
        c.extend_from_slice(&e.target.to_le_bytes());
    }
    for e in ev {
        c.extend_from_slice(&e.x.wrapping_sub(prev.x).to_le_bytes());
        prev.x = e.x;
    }
    for e in ev {
        c.extend_from_slice(&e.y.wrapping_sub(prev.y).to_le_bytes());
        prev.y = e.y;
    }
    for e in ev {
        c.extend_from_slice(&e.payload.to_le_bytes());
    }
    for e in ev {
        c.extend_from_slice(&e.extra.to_le_bytes());
    }
    compress(codec, &c)
}

fn dec_block(b: &[u8], n: usize, codec: Codec) -> Result<Vec<Event>, String> {
    if codec == Codec::None {
        return from_bytes(b).filter(|v: &Vec<Event>| v.len() == n).ok_or_else(|| "block size".to_string());
    }
    let c = decompress(codec, b, n * 32)?;
    let mut ev = vec![Event::default(); n];
    let u32s = |o: usize, i: usize| u32::from_le_bytes(c[o + i * 4..o + i * 4 + 4].try_into().unwrap());
    let u16s = |o: usize, i: usize| u16::from_le_bytes(c[o + i * 2..o + i * 2 + 2].try_into().unwrap());
    let (mut d, mut a, mut x, mut y) = (0u32, 0u32, 0u32, 0u32);
    for i in 0..n {
        d = d.wrapping_add(u32s(0, i));
        ev[i].day = d;
        ev[i].minute = u16s(n * 4, i);
        ev[i].kind = u16s(n * 6, i);
        a = a.wrapping_add(u32s(n * 8, i));
        ev[i].actor = a;
        ev[i].target = u32s(n * 12, i);
        x = x.wrapping_add(u32s(n * 16, i));
        ev[i].x = x;
        y = y.wrapping_add(u32s(n * 20, i));
        ev[i].y = y;
        ev[i].payload = u32s(n * 24, i);
        ev[i].extra = u32s(n * 28, i);
    }
    Ok(ev)
}

// ---------- segment files ----------

const SEG_MAGIC: &[u8; 8] = b"KSEG0001";

/// Sort a finished year and encode it as one segment file.
pub fn encode_segment(year: u32, mut ev: Vec<Event>, codec: Codec) -> Vec<u8> {
    ev.sort_unstable_by_key(|e| (region(e), e.actor, e.day, e.minute, e.kind, e.target));
    let mut out = vec![0u8; 64];
    let mut blocks: Vec<(u64, u32, u64)> = Vec::new(); // offset, stored length, hash
    for b in ev.chunks(BLOCK) {
        let st = enc_block(b, codec);
        blocks.push((out.len() as u64, st.len() as u32, hash(&st)));
        out.extend_from_slice(&st);
    }
    // index: per region (first event, count); per (actor, region) run (first event, count)
    let mut regions: Vec<(u16, u32, u32)> = Vec::new();
    let mut runs: Vec<(u32, u16, u32, u32)> = Vec::new();
    for (i, e) in ev.iter().enumerate() {
        let r = region(e);
        match regions.last_mut() {
            Some(l) if l.0 == r => l.2 += 1,
            _ => regions.push((r, i as u32, 1)),
        }
        match runs.last_mut() {
            Some(l) if l.0 == e.actor && l.1 == r => l.3 += 1,
            _ => runs.push((e.actor, r, i as u32, 1)),
        }
    }
    runs.sort_unstable_by_key(|r| (r.0, r.2));
    let idx_off = out.len() as u64;
    for b in &blocks {
        out.extend_from_slice(&b.0.to_le_bytes());
        out.extend_from_slice(&b.1.to_le_bytes());
        out.extend_from_slice(&b.2.to_le_bytes());
    }
    for r in &regions {
        out.extend_from_slice(&(r.0 as u32).to_le_bytes());
        out.extend_from_slice(&r.1.to_le_bytes());
        out.extend_from_slice(&r.2.to_le_bytes());
    }
    for r in &runs {
        out.extend_from_slice(&r.0.to_le_bytes());
        out.extend_from_slice(&(r.1 as u32).to_le_bytes());
        out.extend_from_slice(&r.2.to_le_bytes());
        out.extend_from_slice(&r.3.to_le_bytes());
    }
    out[0..8].copy_from_slice(SEG_MAGIC);
    out[8..12].copy_from_slice(&year.to_le_bytes());
    out[12] = codec as u8;
    out[16..24].copy_from_slice(&(ev.len() as u64).to_le_bytes());
    out[24..28].copy_from_slice(&(blocks.len() as u32).to_le_bytes());
    out[28..32].copy_from_slice(&(regions.len() as u32).to_le_bytes());
    out[32..36].copy_from_slice(&(runs.len() as u32).to_le_bytes());
    out[40..48].copy_from_slice(&idx_off.to_le_bytes());
    let mut ht = out[0..64].to_vec();
    ht.extend_from_slice(&out[idx_off as usize..]);
    let h = hash(&ht);
    out.extend_from_slice(&h.to_le_bytes());
    out
}

/// The index part of a segment, read and checked.
pub struct SegIndex {
    pub path: PathBuf,
    pub year: u32,
    pub codec: Codec,
    pub n: usize,
    pub blocks: Vec<(u64, u32, u64)>,
    pub regions: Vec<(u16, u32, u32)>,
    pub runs: Vec<(u32, u16, u32, u32)>,
}

pub fn read_index(path: &Path) -> Result<SegIndex, String> {
    let mut f = File::open(path).map_err(|e| e.to_string())?;
    let len = f.metadata().map_err(|e| e.to_string())?.len();
    let mut head = [0u8; 64];
    f.read_exact(&mut head).map_err(|e| e.to_string())?;
    if &head[0..8] != SEG_MAGIC {
        return Err("bad magic".into());
    }
    let g32 = |o: usize| u32::from_le_bytes(head[o..o + 4].try_into().unwrap());
    let (nb, nr, nu) = (g32(24) as usize, g32(28) as usize, g32(32) as usize);
    let idx_off = u64::from_le_bytes(head[40..48].try_into().unwrap());
    let ilen = (nb * 20 + nr * 12 + nu * 16) as u64;
    if idx_off < 64 || idx_off + ilen + 8 != len {
        return Err("length does not match index".into());
    }
    let mut idx = vec![0u8; ilen as usize + 8];
    f.seek(SeekFrom::Start(idx_off)).map_err(|e| e.to_string())?;
    f.read_exact(&mut idx).map_err(|e| e.to_string())?;
    let mut ht = head.to_vec();
    ht.extend_from_slice(&idx[..ilen as usize]);
    if hash(&ht) != u64::from_le_bytes(idx[ilen as usize..].try_into().unwrap()) {
        return Err("index hash differs".into());
    }
    let u32i = |o: usize| u32::from_le_bytes(idx[o..o + 4].try_into().unwrap());
    let blocks = (0..nb).map(|i| (u64::from_le_bytes(idx[i * 20..i * 20 + 8].try_into().unwrap()), u32i(i * 20 + 8), u64::from_le_bytes(idx[i * 20 + 12..i * 20 + 20].try_into().unwrap()))).collect();
    let o = nb * 20;
    let regions = (0..nr).map(|i| (u32i(o + i * 12) as u16, u32i(o + i * 12 + 4), u32i(o + i * 12 + 8))).collect();
    let o = o + nr * 12;
    let runs = (0..nu).map(|i| (u32i(o + i * 16), u32i(o + i * 16 + 4) as u16, u32i(o + i * 16 + 8), u32i(o + i * 16 + 12))).collect();
    Ok(SegIndex { path: path.to_path_buf(), year: g32(8), codec: Codec::from_u8(head[12]).ok_or("codec")?, n: u64::from_le_bytes(head[16..24].try_into().unwrap()) as usize, blocks, regions, runs })
}

/// Events [first, first+count) of a segment.
pub fn read_range(si: &SegIndex, f: &mut File, first: usize, count: usize, out: &mut Vec<Event>) -> Result<(), String> {
    if count == 0 {
        return Ok(());
    }
    let (b0, b1) = (first / BLOCK, (first + count - 1) / BLOCK);
    for b in b0..=b1 {
        let (off, len, h) = *si.blocks.get(b).ok_or("block")?;
        let mut st = vec![0u8; len as usize];
        f.seek(SeekFrom::Start(off)).map_err(|e| e.to_string())?;
        f.read_exact(&mut st).map_err(|e| e.to_string())?;
        if hash(&st) != h {
            return Err("block hash differs".into());
        }
        let n = BLOCK.min(si.n - b * BLOCK);
        let ev = dec_block(&st, n, si.codec)?;
        let lo = first.max(b * BLOCK) - b * BLOCK;
        let hi = (first + count).min(b * BLOCK + n) - b * BLOCK;
        out.extend_from_slice(&ev[lo..hi]);
    }
    Ok(())
}

// ---------- the log: journal plus segments ----------

pub struct Log {
    pub dir: PathBuf,
    pub codec: Codec,
    journal: File,
    year_buf: Vec<Event>,
    pub segments: Vec<SegIndex>,
}

const J_MAGIC: u32 = 0x4A52_4E4C;

impl Log {
    /// Open (or create) a log; checks segments and drops a cut-off journal tail.
    /// Returns the log and the last whole day in it.
    pub fn open(dir: &Path, codec: Codec) -> Result<(Log, Option<u32>), String> {
        fs::create_dir_all(dir).map_err(|e| e.to_string())?;
        let mut segments = Vec::new();
        let mut names: Vec<_> = fs::read_dir(dir).map_err(|e| e.to_string())?.flatten().map(|e| e.file_name().to_string_lossy().to_string()).collect();
        names.sort();
        for n in &names {
            if n.ends_with(".tmp") {
                let _ = fs::remove_file(dir.join(n));
            } else if n.starts_with("seg-") {
                segments.push(read_index(&dir.join(n)).map_err(|e| format!("{n}: {e}"))?);
            }
        }
        let done_year = segments.last().map(|s| s.year + 1).unwrap_or(0);
        let jp = dir.join("journal.bin");
        let mut journal = OpenOptions::new().read(true).append(true).create(true).open(&jp).map_err(|e| e.to_string())?;
        let mut b = Vec::new();
        journal.seek(SeekFrom::Start(0)).map_err(|e| e.to_string())?;
        journal.read_to_end(&mut b).map_err(|e| e.to_string())?;
        let mut o = 0usize;
        let mut year_buf = Vec::new();
        let mut last = segments.last().map(|s| (s.year + 1) * DAYS_PER_YEAR - 1);
        while o + 24 <= b.len() {
            let m = u32::from_le_bytes(b[o..o + 4].try_into().unwrap());
            let n = u32::from_le_bytes(b[o + 4..o + 8].try_into().unwrap()) as usize;
            let day = u32::from_le_bytes(b[o + 8..o + 12].try_into().unwrap());
            let h = u64::from_le_bytes(b[o + 16..o + 24].try_into().unwrap());
            if m != J_MAGIC || o + 24 + n * 32 > b.len() || hash(&b[o + 24..o + 24 + n * 32]) != h {
                break;
            }
            if day / DAYS_PER_YEAR >= done_year {
                year_buf.extend(from_bytes::<Event>(&b[o + 24..o + 24 + n * 32]).unwrap());
                last = Some(day);
            }
            o += 24 + n * 32;
        }
        if o != b.len() {
            journal.set_len(o as u64).map_err(|e| e.to_string())?;
            journal.sync_all().map_err(|e| e.to_string())?;
        }
        Ok((Log { dir: dir.to_path_buf(), codec, journal, year_buf, segments }, last))
    }

    /// Append one day (flushed before returning). Rolls the year into a segment at year end.
    pub fn append_day(&mut self, day: u32, ev: &[Event], flush: bool) -> Result<(), String> {
        let year = day / DAYS_PER_YEAR;
        if let Some(e) = self.year_buf.first() {
            if e.day / DAYS_PER_YEAR != year {
                self.roll()?;
            }
        }
        let mut b = Vec::with_capacity(24 + ev.len() * 32);
        b.extend_from_slice(&J_MAGIC.to_le_bytes());
        b.extend_from_slice(&(ev.len() as u32).to_le_bytes());
        b.extend_from_slice(&day.to_le_bytes());
        b.extend_from_slice(&0u32.to_le_bytes());
        b.extend_from_slice(&hash(as_bytes(ev)).to_le_bytes());
        b.extend_from_slice(as_bytes(ev));
        self.journal.write_all(&b).map_err(|e| e.to_string())?;
        if flush {
            self.journal.sync_data().map_err(|e| e.to_string())?;
        }
        self.year_buf.extend_from_slice(ev);
        Ok(())
    }

    /// Write the open year as a segment, then empty the journal.
    pub fn roll(&mut self) -> Result<(), String> {
        if self.year_buf.is_empty() {
            return Ok(());
        }
        let year = self.year_buf[0].day / DAYS_PER_YEAR;
        let seg = encode_segment(year, std::mem::take(&mut self.year_buf), self.codec);
        let name = format!("seg-{year:06}.bin");
        write_atomic(&self.dir, &name, &seg, None).map_err(|e| e.to_string())?;
        self.segments.push(read_index(&self.dir.join(&name))?);
        self.journal.set_len(0).map_err(|e| e.to_string())?;
        self.journal.sync_all().map_err(|e| e.to_string())?;
        Ok(())
    }

    pub fn bytes(&self) -> u64 {
        fs::read_dir(&self.dir).map(|rd| rd.flatten().filter_map(|e| e.metadata().ok()).map(|m| m.len()).sum()).unwrap_or(0)
    }

    /// All events of one person, in time order.
    pub fn person(&self, actor: u32) -> Result<Vec<Event>, String> {
        let mut out = Vec::new();
        for s in &self.segments {
            let lo = s.runs.partition_point(|r| r.0 < actor);
            let mut f = File::open(&s.path).map_err(|e| e.to_string())?;
            let mut part = Vec::new();
            for r in s.runs[lo..].iter().take_while(|r| r.0 == actor) {
                read_range(s, &mut f, r.2 as usize, r.3 as usize, &mut part)?;
            }
            part.sort_unstable_by_key(|e| (e.day, e.minute));
            out.extend(part);
        }
        out.extend(self.year_buf.iter().filter(|e| e.actor == actor));
        Ok(out)
    }

    /// Events in one region between two days (inclusive).
    pub fn region(&self, reg: u16, d0: u32, d1: u32) -> Result<Vec<Event>, String> {
        let mut out = Vec::new();
        for s in &self.segments {
            let (y0, y1) = (s.year * DAYS_PER_YEAR, (s.year + 1) * DAYS_PER_YEAR - 1);
            if y1 < d0 || y0 > d1 {
                continue;
            }
            if let Some(r) = s.regions.iter().find(|r| r.0 == reg) {
                let mut f = File::open(&s.path).map_err(|e| e.to_string())?;
                let mut part = Vec::new();
                read_range(s, &mut f, r.1 as usize, r.2 as usize, &mut part)?;
                out.extend(part.into_iter().filter(|e| e.day >= d0 && e.day <= d1));
            }
        }
        out.extend(self.year_buf.iter().filter(|e| region(e) == reg && e.day >= d0 && e.day <= d1));
        Ok(out)
    }

    pub fn open_year_events(&self) -> &[Event] {
        &self.year_buf
    }
}

// ---------- SQLite log ----------

pub fn sq_open(path: &Path) -> rusqlite::Result<rusqlite::Connection> {
    let c = rusqlite::Connection::open(path)?;
    let _: String = c.query_row("PRAGMA journal_mode=WAL", [], |r| r.get(0))?;
    c.execute_batch(
        "PRAGMA synchronous=FULL;
         CREATE TABLE IF NOT EXISTS ev(day INTEGER, minute INTEGER, kind INTEGER, actor INTEGER, target INTEGER,
            x INTEGER, y INTEGER, region INTEGER, payload INTEGER, extra INTEGER);
         CREATE INDEX IF NOT EXISTS ev_a ON ev(actor, day);
         CREATE INDEX IF NOT EXISTS ev_r ON ev(region, day);",
    )?;
    Ok(c)
}

pub fn sq_append_day(c: &mut rusqlite::Connection, ev: &[Event], pause_ms: u64) -> rusqlite::Result<()> {
    let tx = c.transaction()?;
    {
        let mut st = tx.prepare_cached("INSERT INTO ev VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)")?;
        for (i, e) in ev.iter().enumerate() {
            if pause_ms > 0 && i == ev.len() / 2 {
                std::thread::sleep(std::time::Duration::from_millis(pause_ms));
            }
            st.execute(rusqlite::params![e.day, e.minute, e.kind, e.actor, e.target, e.x, e.y, region(e), e.payload, e.extra])?;
        }
    }
    tx.commit()
}

fn sq_rows(c: &rusqlite::Connection, q: &str, args: &[i64]) -> rusqlite::Result<Vec<Event>> {
    let mut st = c.prepare_cached(q)?;
    let rows = st.query_map(rusqlite::params_from_iter(args.iter()), |r| {
        Ok(Event { day: r.get(0)?, minute: r.get(1)?, kind: r.get(2)?, actor: r.get(3)?, target: r.get(4)?, x: r.get(5)?, y: r.get(6)?, payload: r.get(7)?, extra: r.get(8)? })
    })?;
    rows.collect()
}

pub fn sq_person(c: &rusqlite::Connection, actor: u32) -> rusqlite::Result<Vec<Event>> {
    sq_rows(c, "SELECT day, minute, kind, actor, target, x, y, payload, extra FROM ev WHERE actor=?1 ORDER BY day, minute", &[actor as i64])
}

pub fn sq_region(c: &rusqlite::Connection, reg: u16, d0: u32, d1: u32) -> rusqlite::Result<Vec<Event>> {
    sq_rows(c, "SELECT day, minute, kind, actor, target, x, y, payload, extra FROM ev WHERE region=?1 AND day BETWEEN ?2 AND ?3", &[reg as i64, d0 as i64, d1 as i64])
}

pub fn dir_bytes(p: &Path) -> u64 {
    fs::read_dir(p).map(|rd| rd.flatten().filter_map(|e| e.metadata().ok()).map(|m| m.len()).sum()).unwrap_or(0)
}
