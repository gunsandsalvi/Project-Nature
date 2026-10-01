//! B04/B11 pre-test, cloud bench. Throwaway code; see NOTES.md.
//! Subcommands: layouts | moment | log | crash <fmt> <n> | damage <fmt> <n> | gen | detail | phone

mod fb;
mod layouts;

use kstorage::data::*;
use kstorage::hist::{self, Event, Log};
use kstorage::save::{self, Codec};
use kstorage::{sq, terrain};
use std::alloc::{GlobalAlloc, Layout, System};
use std::io::{BufRead, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicUsize, Ordering};
use std::time::Instant;

struct Counting;
static ALLOC: AtomicUsize = AtomicUsize::new(0);
unsafe impl GlobalAlloc for Counting {
    unsafe fn alloc(&self, l: Layout) -> *mut u8 {
        ALLOC.fetch_add(l.size(), Ordering::Relaxed);
        System.alloc(l)
    }
    unsafe fn dealloc(&self, p: *mut u8, l: Layout) {
        ALLOC.fetch_sub(l.size(), Ordering::Relaxed);
        System.dealloc(p, l)
    }
    unsafe fn realloc(&self, p: *mut u8, l: Layout, n: usize) -> *mut u8 {
        ALLOC.fetch_add(n, Ordering::Relaxed);
        ALLOC.fetch_sub(l.size(), Ordering::Relaxed);
        System.realloc(p, l, n)
    }
}
#[global_allocator]
static GA: Counting = Counting;
pub fn allocated() -> usize {
    ALLOC.load(Ordering::Relaxed)
}

pub fn median_spread(mut v: Vec<f64>) -> (f64, f64, f64) {
    v.sort_by(|a, b| a.total_cmp(b));
    (v[v.len() / 2], v[0], v[v.len() - 1])
}
fn ms(t: Instant) -> f64 {
    t.elapsed().as_secs_f64() * 1e3
}
fn fmt3(v: Vec<f64>) -> String {
    let (m, lo, hi) = median_spread(v);
    format!("{m:8.1} ({lo:.1}-{hi:.1})")
}

fn cache() -> PathBuf {
    PathBuf::from(std::env::var("CACHE").unwrap_or_else(|_| "/tmp/b04b11-cache".into()))
}
fn fresh(name: &str) -> PathBuf {
    let d = cache().join("b04b11-run").join(name);
    let _ = std::fs::remove_dir_all(&d);
    std::fs::create_dir_all(&d).unwrap();
    d
}

fn main() {
    let a: Vec<String> = std::env::args().collect();
    let arg = |i: usize, d: &str| a.get(i).cloned().unwrap_or_else(|| d.to_string());
    match arg(1, "").as_str() {
        "layouts" => layouts::run(),
        "moment" => moment(arg(2, "1").parse().unwrap(), arg(3, "3").parse().unwrap(), &arg(4, "all")),
        "log" => log_bench(arg(2, "100").parse().unwrap(), arg(3, "10").parse().unwrap(), &arg(4, "all"), arg(5, "3").parse().unwrap()),
        "crash" => crash(&arg(2, "custom"), arg(3, "100").parse().unwrap()),
        "crash-writer" => crash_writer(&arg(2, "custom"), Path::new(&arg(3, ".")), arg(4, "1").parse().unwrap()),
        "damage" => damage(&arg(2, "custom"), arg(3, "100").parse().unwrap()),
        "gen" => gen(arg(2, "3").parse().unwrap(), &arg(3, "warp,erode,plates")),
        "detail" => detail(arg(2, "3").parse().unwrap()),
        "phone" => {
            let d = fresh("phone");
            let cfg = arg(2, "{}").replace("DIR", &d.to_string_lossy());
            println!("{}", kstorage::run(&cfg));
        }
        _ => eprintln!("usage: b04b11 layouts|moment [div reps fmt]|log [people years fmt reps]|crash fmt n|damage fmt n|gen [reps methods]|detail [reps]|phone [json]"),
    }
}

// ---------------- B04-2 saved moments ----------------

fn fmt_codec(f: &str) -> (String, Codec) {
    let (base, c) = f.split_once('-').unwrap_or((f, "none"));
    let codec = match c {
        "lz4" => Codec::Lz4,
        "zstd" | "zstd1" => Codec::Zstd,
        _ => Codec::None,
    };
    (base.to_string(), codec)
}

fn moment(div: usize, reps: usize, which: &str) {
    let t = Instant::now();
    let s = build(42, 1000, div);
    let want = content_hash(&s);
    println!("# B04-2 saved moment: div={div} raw={:.1} MB, built in {:.0} ms", raw_bytes(&s) as f64 / 1e6, ms(t));
    println!("# per format: size MB | full write ms (encode / write / flush / rename) | cold full read ms | cold one-region ms  [median (min-max) of {reps}]");
    let busy = save::region_in(s.w, s.h, s.people[0].core.x, s.people[0].core.y);
    let fmts: Vec<String> = if which == "all" {
        ["custom", "fb", "sqlite"].iter().flat_map(|b| ["none", "lz4", "zstd"].iter().map(move |c| format!("{b}-{c}"))).collect()
    } else {
        which.split(',').map(String::from).collect()
    };
    for f in fmts {
        let (base, codec) = fmt_codec(&f);
        let dir = fresh("moment");
        let (mut tot, mut enc, mut wr, mut fl, mut rn, mut rd, mut rg) = (vec![], vec![], vec![], vec![], vec![], vec![], vec![]);
        let mut size = 0u64;
        for _ in 0..reps {
            let path;
            if base == "sqlite" {
                path = dir.join("m.sqlite");
                for e in ["", "-wal", "-shm"] {
                    let _ = std::fs::remove_file(format!("{}{e}", path.display()));
                }
                let t = Instant::now();
                let mut c = sq::open(&path).unwrap();
                let (ins, ck) = sq::save(&mut c, 1, &s, codec, 0, 0).unwrap();
                drop(c);
                tot.push(ms(t));
                wr.push(ins);
                fl.push(ck);
                size = std::fs::metadata(&path).unwrap().len();
                save::drop_cache(&path);
                let t = Instant::now();
                let c = sq::open(&path).unwrap();
                let back = sq::load_all(&c, 1).unwrap();
                rd.push(ms(t));
                assert_eq!(content_hash(&back), want);
                drop((back, c));
                save::drop_cache(&path);
                let t = Instant::now();
                let c = sq::open(&path).unwrap();
                let r = sq::load_region(&c, 1, busy as i64).unwrap();
                rg.push(ms(t));
                assert!(!r.people.is_empty());
            } else {
                path = dir.join(if base == "fb" { "m.kfb" } else { "m.ksave" });
                let t = Instant::now();
                let b = if base == "fb" { fb::encode(&s, codec) } else { save::encode(&s, codec) };
                let e = ms(t);
                let (w, f_, r) = save::write_atomic(&dir, path.file_name().unwrap().to_str().unwrap(), &b, None).unwrap();
                tot.push(ms(t));
                enc.push(e);
                wr.push(w);
                fl.push(f_);
                rn.push(r);
                size = b.len() as u64;
                drop(b);
                save::drop_cache(&path);
                let t = Instant::now();
                let back = if base == "fb" { fb::load_all(&path).unwrap() } else { save::load_all(&path).unwrap() };
                rd.push(ms(t));
                assert_eq!(content_hash(&back), want);
                drop(back);
                save::drop_cache(&path);
                let t = Instant::now();
                let r = if base == "fb" { fb::load_region(&path, busy).unwrap() } else { save::load_region(&path, busy).unwrap() };
                rg.push(ms(t));
                assert!(!r.people.is_empty());
            }
        }
        let parts = if base == "sqlite" {
            format!("insert+commit {} checkpoint {}", fmt3(wr), fmt3(fl))
        } else {
            format!("enc {} wr {} flush {} ren {}", fmt3(enc), fmt3(wr), fmt3(fl), fmt3(rn))
        };
        println!("{f:12} {:8.1} MB | write {} [{parts}] | read {} | region {}", size as f64 / 1e6, fmt3(tot), fmt3(rd), fmt3(rg));
        let _ = std::fs::remove_dir_all(&dir);
    }
}

// ---------------- B04-3 history log ----------------

fn log_bench(people: u32, years: u32, which: &str, reps: usize) {
    let days = years * hist::DAYS_PER_YEAR;
    let n_ev = days as u64 * people as u64 * hist::PER_DAY as u64;
    println!("# B04-3 history log: {people} people x {years} years = {n_ev} events ({} per person per day)", hist::PER_DAY);
    let fmts: Vec<String> = if which == "all" { vec!["custom-none".into(), "custom-lz4".into(), "custom-zstd".into(), "sqlite".into()] } else { which.split(',').map(String::from).collect() };
    let mut buf = Vec::new();
    // query targets: person 7; the region of person 7's band at day 0; years 1/3 to 2/3 of the span
    hist::day_events(5, people, 0, &mut buf);
    let reg = hist::region(&buf[70]);
    let (d0, d1) = (days / 3, 2 * days / 3);
    for f in fmts {
        let (base, codec) = fmt_codec(&f);
        let (mut app, mut sizes, mut qp, mut qr) = (vec![], vec![], vec![], vec![]);
        let (mut np, mut nr) = (0usize, 0usize);
        for rep in 0..reps {
            let dir = fresh("log");
            let t = Instant::now();
            if base == "sqlite" {
                let p = dir.join("log.sqlite");
                let mut c = hist::sq_open(&p).unwrap();
                for d in 0..days {
                    hist::day_events(5, people, d, &mut buf);
                    hist::sq_append_day(&mut c, &buf, 0).unwrap();
                }
                sq::checkpoint(&c).unwrap();
                app.push(ms(t));
                sizes.push(hist::dir_bytes(&dir) as f64);
                for _ in 0..5 {
                    let t = Instant::now();
                    np = hist::sq_person(&c, 7).unwrap().len();
                    qp.push(ms(t));
                    let t = Instant::now();
                    nr = hist::sq_region(&c, reg, d0, d1).unwrap().len();
                    qr.push(ms(t));
                }
            } else {
                let (mut lg, _) = Log::open(&dir, codec).unwrap();
                for d in 0..days {
                    hist::day_events(5, people, d, &mut buf);
                    lg.append_day(d, &buf, true).unwrap();
                }
                lg.roll().unwrap();
                app.push(ms(t));
                sizes.push(lg.bytes() as f64);
                let (lg, _) = Log::open(&dir, codec).unwrap();
                for _ in 0..5 {
                    let t = Instant::now();
                    np = lg.person(7).unwrap().len();
                    qp.push(ms(t));
                    let t = Instant::now();
                    nr = lg.region(reg, d0, d1).unwrap().len();
                    qr.push(ms(t));
                }
            }
            let _ = rep;
        }
        let size = sizes[0];
        let per_ev = size / n_ev as f64;
        let per_1000y = per_ev * people as f64 * hist::PER_DAY as f64 * 365.0 * 1000.0 / 1e9;
        println!(
            "{f:12} {:7.1} MB  {per_ev:5.2} B/event  => {per_1000y:7.1} GB per 1,000 years | append all (flushed daily) {} ms | person query {} ms ({np} events) | region query {} ms ({nr} events)",
            size / 1e6,
            fmt3(app),
            fmt3(qp),
            fmt3(qr)
        );
    }
}

// ---------------- B04-4 crash safety ----------------

/// The state saved as moment k: a fixed base world, changed everywhere by k.
fn crash_state(base: &State, k: u64) -> State {
    let mut s = base.clone();
    s.day = k;
    for (i, t) in s.things.iter_mut().enumerate() {
        t.condition = (k as usize * 31 + i) as u16;
    }
    for (i, c) in s.cells.iter_mut().enumerate() {
        c.water = ((k as usize + i) % 97) as f32;
    }
    s
}
const CRASH_DIV: usize = 8;

fn crash_writer(fmt: &str, dir: &Path, start: u64) {
    let base = build(9, 0, CRASH_DIV);
    let out = std::io::stdout();
    let say = |m: String| {
        let mut o = out.lock();
        writeln!(o, "{m}").unwrap();
        o.flush().unwrap();
    };
    if fmt.starts_with("log") {
        let people = 100;
        let mut buf = Vec::new();
        let mut d = start as u32;
        if fmt == "log-sqlite" {
            let mut c = hist::sq_open(&dir.join("log.sqlite")).unwrap();
            loop {
                hist::day_events(3, people, d, &mut buf);
                say(format!("BEGIN {d}"));
                hist::sq_append_day(&mut c, &buf, 0).unwrap();
                say(format!("SAVED {d}"));
                d += 1;
            }
        } else {
            let (mut lg, _) = Log::open(dir, Codec::Lz4).unwrap();
            loop {
                hist::day_events(3, people, d, &mut buf);
                say(format!("BEGIN {d}"));
                lg.append_day(d, &buf, true).unwrap();
                say(format!("SAVED {d}"));
                d += 1;
            }
        }
    }
    let (base_fmt, codec) = fmt_codec(fmt);
    let mut c = if base_fmt == "sqlite" { Some(sq::open(&dir.join("m.sqlite")).unwrap()) } else { None };
    let mut k = start;
    loop {
        let s = crash_state(&base, k);
        say(format!("BEGIN {k}"));
        match base_fmt.as_str() {
            "sqlite" => {
                sq::save(c.as_mut().unwrap(), k as i64, &s, codec, 2, 0).unwrap();
            }
            _ => {
                let (b, ext) = if base_fmt == "fb" { (fb::encode(&s, codec), "kfb") } else { (save::encode(&s, codec), "ksave") };
                say(format!("WRITING {k}"));
                save::write_atomic(dir, &format!("m-{k:08}.{ext}"), &b, None).unwrap();
                if k >= 2 {
                    let _ = std::fs::remove_file(dir.join(format!("m-{:08}.{ext}", k - 2)));
                }
            }
        }
        say(format!("SAVED {k}"));
        k += 1;
    }
}

/// Load the newest moment that passes every check; returns (k, content ok, rejected files).
fn load_newest(fmt: &str, dir: &Path, base: &State) -> Result<(u64, usize), String> {
    let (base_fmt, _) = fmt_codec(fmt);
    if base_fmt == "sqlite" {
        let c = sq::open(&dir.join("m.sqlite")).map_err(|e| e.to_string())?;
        let k = sq::newest(&c).map_err(|e| e.to_string())?.ok_or("no moment")?;
        let s = sq::load_all(&c, k)?;
        if content_hash(&s) != content_hash(&crash_state(base, k as u64)) {
            return Err(format!("moment {k} loaded with wrong content"));
        }
        return Ok((k as u64, 0));
    }
    let ext = if base_fmt == "fb" { ".kfb" } else { ".ksave" };
    let mut names: Vec<String> = std::fs::read_dir(dir).unwrap().flatten().map(|e| e.file_name().to_string_lossy().to_string()).collect();
    for n in names.iter().filter(|n| n.ends_with(".tmp")) {
        let _ = std::fs::remove_file(dir.join(n));
    }
    names.retain(|n| n.ends_with(ext));
    names.sort();
    let mut rejected = 0;
    for n in names.iter().rev() {
        let k: u64 = n[2..10].parse().unwrap();
        let p = dir.join(n);
        let r = if base_fmt == "fb" { fb::load_all(&p) } else { save::load_all(&p) };
        match r {
            Ok(s) => {
                if s.day != k || content_hash(&s) != content_hash(&crash_state(base, k)) {
                    return Err(format!("{n} loaded with wrong content"));
                }
                return Ok((k, rejected));
            }
            Err(_) => rejected += 1,
        }
    }
    Err("nothing loadable".into())
}

fn check_log(fmt: &str, dir: &Path) -> Result<u32, String> {
    let people = 100u32;
    let mut buf = Vec::new();
    if fmt == "log-sqlite" {
        let c = hist::sq_open(&dir.join("log.sqlite")).map_err(|e| e.to_string())?;
        let (last, n): (i64, i64) = c.query_row("SELECT max(day), count(*) FROM ev", [], |r| Ok((r.get(0)?, r.get(1)?))).map_err(|e| e.to_string())?;
        if n != (last + 1) * people as i64 * hist::PER_DAY as i64 {
            return Err(format!("count {n} for {} days", last + 1));
        }
        hist::day_events(3, people, last as u32, &mut buf);
        let mut got: Vec<Event> = (0..people).flat_map(|p| hist::sq_person(&c, p).unwrap().into_iter().filter(|e| e.day == last as u32)).collect();
        got.sort_by_key(|e| (e.actor, e.minute));
        if got != buf {
            return Err("last day differs".into());
        }
        return Ok(last as u32);
    }
    let (lg, last) = Log::open(dir, Codec::Lz4)?;
    let last = last.ok_or("empty log")?;
    let in_segs: usize = lg.segments.iter().map(|s| s.n).sum();
    let total = in_segs + lg.open_year_events().len();
    if total as u64 != (last as u64 + 1) * people as u64 * hist::PER_DAY as u64 {
        return Err(format!("{total} events for {} days", last + 1));
    }
    // the open year must equal what was written, day by day
    let mut want = Vec::new();
    let y0 = (last / hist::DAYS_PER_YEAR) * hist::DAYS_PER_YEAR;
    for d in y0..=last {
        hist::day_events(3, people, d, &mut buf);
        want.extend_from_slice(&buf);
    }
    if lg.open_year_events() != &want[..] {
        return Err("open year differs".into());
    }
    if let Some(s) = lg.segments.last() {
        let mut f = std::fs::File::open(&s.path).unwrap();
        let mut all = Vec::new();
        hist::read_range(s, &mut f, 0, s.n, &mut all)?;
        if all.len() != s.n {
            return Err("segment short".into());
        }
    }
    Ok(last)
}

fn crash(fmt: &str, n: usize) {
    let dir = fresh(&format!("crash-{fmt}"));
    let exe = std::env::current_exe().unwrap();
    let base = build(9, 0, CRASH_DIV);
    let is_log = fmt.starts_with("log");
    // typical cycle time, to spread kill moments over several saves
    let (mut start, mut fails, mut inside, mut after_write) = (0u64, 0usize, 0usize, 0usize);
    let mut ever_saved = false;
    let mut cycle_ms = 0f64;
    let mut rej_total = 0usize;
    for trial in 0..n {
        let mut ch = std::process::Command::new(&exe).args(["crash-writer", fmt, dir.to_str().unwrap(), &start.to_string()]).stdout(std::process::Stdio::piped()).spawn().unwrap();
        let stdout = ch.stdout.take().unwrap();
        let (tx, rx) = std::sync::mpsc::channel::<(String, Instant)>();
        std::thread::spawn(move || {
            for l in std::io::BufReader::new(stdout).lines().map_while(Result::ok) {
                let _ = tx.send((l, Instant::now()));
            }
        });
        // wait for the first BEGIN, then kill after a random delay of up to ~3 cycles
        let (first, t_first) = rx.recv().unwrap();
        assert!(first.starts_with("BEGIN"));
        let r = mix(trial as u64 ^ 0xC0FFEE);
        // first trial: run 1.5 s to learn the save cycle; then kill at a random moment within
        // about 3 save cycles (moments) or within 0.5 s of daily appends, which spans year ends (logs)
        let wait = if trial == 0 {
            1500.0
        } else if is_log {
            (r % 1000) as f64 / 1000.0 * 500.0
        } else {
            (r % 1000) as f64 / 1000.0 * cycle_ms * 3.0
        };
        std::thread::sleep(std::time::Duration::from_micros((wait * 1000.0) as u64));
        unsafe { libc::kill(ch.id() as i32, libc::SIGKILL) };
        let _ = ch.wait();
        let mut last_saved: Option<u64> = None;
        let mut last_line = first;
        let mut saves = 0usize;
        let mut t_last = t_first;
        while let Ok((l, t)) = rx.recv_timeout(std::time::Duration::from_millis(200)) {
            if let Some(k) = l.strip_prefix("SAVED ") {
                last_saved = Some(k.parse().unwrap());
                saves += 1;
                t_last = t;
            }
            last_line = l;
        }
        if saves > 0 {
            ever_saved = true;
            let c = (t_last - t_first).as_secs_f64() * 1e3 / saves as f64;
            cycle_ms = if cycle_ms == 0.0 { c } else { 0.8 * cycle_ms + 0.2 * c };
        }
        if !last_line.starts_with("SAVED") {
            inside += 1;
            if last_line.starts_with("WRITING") {
                after_write += 1;
            }
        }
        let res = if is_log { check_log(fmt, &dir).map(|d| (d as u64, 0)) } else { load_newest(fmt, &dir, &base) };
        match res {
            Ok((k, rej)) => {
                rej_total += rej;
                if let Some(ls) = last_saved {
                    if k < ls {
                        fails += 1;
                        eprintln!("trial {trial}: loaded {k} but {ls} was reported saved");
                    }
                }
                start = k + 1;
            }
            Err(e) => {
                // nothing loadable is right only if no save was ever reported
                if ever_saved {
                    fails += 1;
                    eprintln!("trial {trial}: {e}");
                }
            }
        }
    }
    println!(
        "{fmt:12} kills {n}: inside a save {inside} (of which during the file write {after_write}), failures {fails}, rejected files {rej_total}, last moment {}, cycle {:.0} ms",
        start.saturating_sub(1),
        cycle_ms
    );
    let _ = std::fs::remove_dir_all(&dir);
}

fn damage(fmt: &str, n: usize) {
    let dir = fresh(&format!("damage-{fmt}"));
    let base = build(9, 0, CRASH_DIV);
    let s = crash_state(&base, 5);
    let (base_fmt, codec) = fmt_codec(fmt);
    let (mut caught, mut caught_by_engine, mut loaded_wrong, mut loaded_same) = (0, 0, 0, 0);
    if base_fmt == "sqlite" {
        let p = dir.join("m.sqlite");
        let mut c = sq::open(&p).unwrap();
        sq::save(&mut c, 5, &s, codec, 0, 0).unwrap();
        drop(c);
        let good = std::fs::read(&p).unwrap();
        for i in 0..n {
            let mut b = good.clone();
            let r = mix(i as u64 ^ 0xDA4A);
            let off = 4096 + (r as usize % (b.len() - 4096)); // leave the file header alone
            b[off] ^= 1 << (r >> 60 & 7);
            std::fs::write(&p, &b).unwrap();
            let res = sq::open(&p).map_err(|e| e.to_string()).and_then(|c| sq::load_all(&c, 5));
            match res {
                Err(e) => {
                    caught += 1;
                    if !e.contains("content hash") {
                        caught_by_engine += 1;
                    }
                }
                Ok(back) if content_hash(&back) == content_hash(&s) => loaded_same += 1,
                Ok(_) => loaded_wrong += 1,
            }
        }
    } else {
        let good = if base_fmt == "fb" { fb::encode(&s, codec) } else { save::encode(&s, codec) };
        for i in 0..n {
            let mut b = good.clone();
            let r = mix(i as u64 ^ 0xDA4A);
            match i % 3 {
                0 => b.truncate(r as usize % b.len()),
                1 => {
                    let off = r as usize % b.len();
                    b[off] ^= 1 << (r >> 60 & 7);
                }
                _ => {
                    let off = (r as usize % (b.len() / 4096).max(1)) * 4096;
                    for x in b.iter_mut().skip(off).take(4096) {
                        *x = 0;
                    }
                    if b == good {
                        b[off] ^= 1; // the block was zero already
                    }
                }
            }
            let res = if base_fmt == "fb" { fb::load_bytes(&b) } else { save::load_bytes(&b) };
            match res {
                Err(_) => caught += 1,
                Ok(back) if content_hash(&back) == content_hash(&s) => loaded_same += 1,
                Ok(_) => loaded_wrong += 1,
            }
        }
        caught_by_engine = caught;
    }
    println!("{fmt:12} damaged {n}: rejected {caught} (by the format/engine itself {caught_by_engine}), loaded with wrong content {loaded_wrong}, harmless {loaded_same}");
    let _ = std::fs::remove_dir_all(&dir);
}

// ---------------- B11-2 generation ----------------

fn gen(reps: usize, methods: &str) {
    let (w, h) = (2048usize, 1024usize);
    println!("# B11-2 generation, whole world {w} x {h} cells (~1 km), one thread; ms median (min-max) of {reps}");
    for m in methods.split(',') {
        let mut t = vec![];
        let mut map = None;
        for _ in 0..reps {
            let s = Instant::now();
            let mm = gen_variant(m, w, h, 1);
            t.push(ms(s));
            map = Some(mm);
        }
        let map = map.unwrap();
        let s = Instant::now();
        let m4 = gen_variant(m, w, h, 4);
        let t4 = ms(s);
        let same4 = terrain::map_hash(&m4) == terrain::map_hash(&map);
        let q = terrain::quality(&map);
        println!(
            "{m:7} {} ms | 4 threads {t4:.0} ms, same bits {same4} | hash {:016x} | land {:.1}% | river cells {} reach sea {:.1}% | land draining to sea {:.1}% | pits per 10,000 km2 {:.1} | slope p10 {:.2} p50 {:.2} p90 {:.2} p99 {:.2} deg, >30 deg {:.2}%",
            fmt3(t),
            terrain::map_hash(&map),
            q.land * 100.0,
            q.river_cells,
            q.river_to_sea * 100.0,
            q.land_to_sea * 100.0,
            q.pits_per_10k_km2,
            q.slope_pct[0],
            q.slope_pct[1],
            q.slope_pct[2],
            q.slope_pct[3],
            q.slope_pct[4]
        );
        // small version for the phone comparison
        let s = Instant::now();
        let small = gen_variant(m, 1024, 512, 1);
        println!("{m:7} 1024x512: {:.0} ms, hash {:016x}", ms(s), terrain::map_hash(&small));
        preview(&map, &format!("previews/{m}.png"));
    }
}

/// The three methods, plus an extra: warped noise with only the pit filling (not one of the rule's methods).
fn gen_variant(m: &str, w: usize, h: usize, threads: usize) -> terrain::Map {
    if m == "warpfill" {
        let mut mm = terrain::generate("warp", 7, w, h, threads);
        terrain::fill(&mut mm);
        mm
    } else {
        terrain::generate(m, 7, w, h, threads)
    }
}

/// Colour by height with hill shading and big rivers, at half size.
fn preview(m: &terrain::Map, path: &str) {
    let (rec, _) = terrain::receivers(m);
    let acc = terrain::drainage(m, &rec);
    let (w, h) = (m.w / 2, m.h / 2);
    let mut img = vec![0u8; w * h * 3];
    let z = |x: usize, y: usize| m.z[(y % m.h) * m.w + (x % m.w)];
    for y in 0..h {
        for x in 0..w {
            let (sx, sy) = (x * 2, y * 2);
            let e = (z(sx, sy) + z(sx + 1, sy) + z(sx, sy + 1) + z(sx + 1, sy + 1)) / 4.0;
            let shade = ((z(sx + 2, sy + 2) - z(sx, sy)) / 400.0).clamp(-0.5, 0.5);
            let river = (0..4).any(|k| acc[((sy + k / 2) % m.h) * m.w + (sx + k % 2)] > 400.0);
            let c: [f32; 3] = if e <= 0.0 {
                let d = (-e / 4000.0).min(1.0);
                [20.0 + 40.0 * (1.0 - d), 50.0 + 70.0 * (1.0 - d), 110.0 + 80.0 * (1.0 - d)]
            } else if river {
                [40.0, 90.0, 200.0]
            } else {
                let t = (e / 3500.0).min(1.0);
                let base = if t < 0.3 { [90.0 + 200.0 * t, 140.0 + 60.0 * t, 70.0] } else if t < 0.7 { [150.0 + 50.0 * (t - 0.3), 130.0 - 40.0 * (t - 0.3), 80.0] } else { [200.0 + 50.0 * (t - 0.7), 200.0 + 50.0 * (t - 0.7), 200.0 + 50.0 * (t - 0.7)] };
                let k = 1.0 - shade;
                [base[0] * k, base[1] * k, base[2] * k]
            };
            for i in 0..3 {
                img[(y * w + x) * 3 + i] = c[i].clamp(0.0, 255.0) as u8;
            }
        }
    }
    let f = std::fs::File::create(path).unwrap();
    let mut e = png::Encoder::new(std::io::BufWriter::new(f), w as u32, h as u32);
    e.set_color(png::ColorType::Rgb);
    e.set_depth(png::BitDepth::Eight);
    e.set_compression(png::Compression::Best);
    e.write_header().unwrap().write_image_data(&img).unwrap();
    println!("preview {path}: {} KB", std::fs::metadata(path).unwrap().len() / 1024);
}

// ---------------- B11-1 land storage ----------------

fn detail(reps: usize) {
    let site = terrain::Site::new(99, [310.0, 342.0, 365.0, 330.0]);
    println!("# B11-1 metre detail for one 1,024 x 1,024 m cell with a 30 m layered cliff, overhangs and caves");
    for th in [1usize, 4] {
        let (mut t1, mut t2) = (vec![], vec![]);
        let (mut h1, mut h2) = (vec![], vec![]);
        let (mut hp_b, mut cu_b, mut np, mut nb, mut nu) = (0, 0, 0, 0, 0);
        for _ in 0..reps {
            let s = Instant::now();
            let hp = terrain::build_height_pieces(&site, th);
            t1.push(ms(s));
            h1.push(hp.hash());
            hp_b = hp.bytes();
            np = hp.pieces.len();
            drop(hp);
            let s = Instant::now();
            let cu = terrain::build_cubes(&site, th);
            t2.push(ms(s));
            h2.push(cu.hash());
            cu_b = cu.bytes();
            nb = cu.blocks.len();
            nu = cu.uniform;
        }
        h1.dedup();
        h2.dedup();
        println!("threads {th}: height map + pieces {} ms, {:.2} MB ({np} pieces), hashes {:x?}", fmt3(t1), hp_b as f64 / 1e6, h1);
        println!("threads {th}: cube grid          {} ms, {:.2} MB ({nb} mixed blocks, {nu} uniform), hashes {:x?}", fmt3(t2), cu_b as f64 / 1e6, h2);
    }
    // the coarse levels: what a cube grid would need at 1 km and 256 m cubes, from a generated world
    let m = terrain::generate("plates", 7, 2048, 1024, 4);
    for (name, cube_m, sub) in [("1 km cubes", 977.0f32, 1usize), ("256 m cubes", 244.0, 4)] {
        // blocks of 16 x 16 columns; vertical blocks needed = span of heights / (16 cubes) + 1
        let cols = 16 / sub.min(16);
        let (bw, bh) = (m.w / cols.max(1), m.h / cols.max(1));
        let mut blocks = 0u64;
        for by in 0..bh {
            for bx in 0..bw {
                let mut lo = f32::MAX;
                let mut hi = f32::MIN;
                for y in by * cols..(by + 1) * cols {
                    for x in bx * cols..(bx + 1) * cols {
                        let v = m.z[y * m.w + x].max(0.0);
                        lo = lo.min(v);
                        hi = hi.max(v);
                    }
                }
                blocks += ((hi - lo) / (16.0 * cube_m)).floor() as u64 + 1;
            }
        }
        println!("whole world as {name}: {blocks} mixed blocks of 4 KB = {:.1} GB", blocks as f64 * 4104.0 / 1e9);
    }
}
