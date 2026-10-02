//! The catalogue compiler (A3.6), behind feature `compile`: run by `kd catalog`, and by `kd-app`'s build script, which
//! embeds the blob. It follows A3.6's five steps and its validation rules 1, 2, 3, 4 and 6 as they apply to the kinds
//! built so far (`MAT-17`).
//! Implements `MAT-13`, `MAT-17` and `PLT-09` in part, see A3.6.
//!
//! A2.3 rule 4: this feature, used only by tools and build scripts, is where `kd-data` may touch files.

pub mod lock;
pub mod source;
pub mod tables;

use crate::blob::{Body, Catalogue, ColourRec, FamilyRec, LadderRec, LightRec, SurfaceRec};
use crate::kinds::{self, KindName};
use crate::schema::{ColourFamily, Common, Ladder, LightMethod, LightRole, LightTable, Surface, Version};
use lock::Lock;
use source::RawEntry;
use std::collections::{BTreeMap, BTreeSet};
use std::fmt;
use std::path::Path;

/// Which validation rule an error breaks (A3.6), so tests can tell the planted errors apart.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub enum Rule {
    /// A file, `VERSION.toml` or `PROJECT.md` could not be read.
    Read,
    /// A Markdown file under `data/` that no kind claims.
    UnclaimedFile,
    /// An entry with no `toml` block.
    MissingBlock,
    /// An entry with more than one `toml` block.
    TwoBlocks,
    UnknownField,
    MissingField,
    /// A block that is not valid TOML for its kind for another reason.
    Parse,
    /// `name` unlike the heading.
    NameMismatch,
    /// An id used twice in one kind, or a colour name used twice in the palette.
    RepeatedId,
    NotSnakeCase,
    /// An id with no line in `data/ids.lock`.
    NotLocked,
    /// An entry whose id is retired, or a retired number used again.
    RetiredNumber,
    /// A malformed lock line, an id locked twice, or one number for two ids.
    Lock,
    /// A reference (a colour, a family) that names nothing.
    UnresolvedReference,
    EmptyChecks,
    /// A `checks` ID that is not a live item of `PROJECT.md`.
    CheckNotLive,
    /// `stage` not a `MIL-0n` of `PROJECT.md`.
    BadStage,
    /// A value out of its range or of the wrong shape.
    BadValue,
    /// A generated table or `data/INDEX.md` that does not match the blocks.
    StaleTable,
}

/// One validation failure, with where it is.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct CatalogueError {
    pub rule: Rule,
    /// The file, relative to the repository (`data/palette/colours.md`).
    pub file: String,
    /// The line, from 1, when known.
    pub line: usize,
    pub message: String,
}

impl fmt::Display for CatalogueError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.line > 0 {
            write!(f, "{}:{}: {}", self.file, self.line, self.message)
        } else {
            write!(f, "{}: {}", self.file, self.message)
        }
    }
}

/// An entry's parsed block.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Parsed {
    ColourFamily(ColourFamily),
    Ladder(Ladder),
    LightTable(Box<LightTable>),
    Surface(Surface),
}

impl Parsed {
    pub fn common(&self) -> &Common {
        match self {
            Parsed::ColourFamily(e) => &e.common,
            Parsed::Ladder(e) => &e.common,
            Parsed::LightTable(e) => &e.common,
            Parsed::Surface(e) => &e.common,
        }
    }
}

/// A compiled entry: its kind, file, number and parsed block.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Entry {
    pub kind: KindName,
    /// Relative to `data/`, with `/`.
    pub file: String,
    pub raw: RawEntry,
    pub number: u32,
    pub parsed: Parsed,
}

/// What the compiler gives: the catalogue, its blob and the entries it came from.
#[derive(Clone, Debug)]
pub struct Compiled {
    pub catalogue: Catalogue,
    pub blob: Vec<u8>,
    pub entries: Vec<Entry>,
    /// Ids given new numbers by `assign`.
    pub assigned: Vec<String>,
}

struct Errors {
    list: Vec<CatalogueError>,
}

impl Errors {
    fn add(&mut self, rule: Rule, file: &str, line: usize, message: String) {
        self.list.push(CatalogueError {
            rule,
            file: format!("data/{file}"),
            line,
            message,
        });
    }
}

fn read(path: &Path) -> Result<String, String> {
    std::fs::read_to_string(path).map_err(|e| format!("{}: {e}", path.display()))
}

/// Every Markdown file under `dir`, relative to `root`, with `/`, sorted.
fn markdown_files(root: &Path, dir: &Path, out: &mut Vec<String>) -> Result<(), String> {
    let rd = std::fs::read_dir(dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    for item in rd {
        let p = item.map_err(|e| e.to_string())?.path();
        if p.is_dir() {
            markdown_files(root, &p, out)?;
        } else if p.extension().is_some_and(|x| x == "md") {
            let rel = p.strip_prefix(root).map_err(|e| e.to_string())?;
            let parts: Vec<String> = rel
                .components()
                .map(|c| c.as_os_str().to_string_lossy().into_owned())
                .collect();
            out.push(parts.join("/"));
        }
    }
    out.sort();
    Ok(())
}

fn parse_block(kind: KindName, block: &str) -> Result<Parsed, (Rule, String)> {
    let r = match kind {
        KindName::ColourFamily => toml::from_str(block).map(Parsed::ColourFamily),
        KindName::Ladder => toml::from_str(block).map(Parsed::Ladder),
        KindName::LightTable => toml::from_str(block).map(|t| Parsed::LightTable(Box::new(t))),
        KindName::Surface => toml::from_str(block).map(Parsed::Surface),
    };
    r.map_err(|e| {
        let m = e.message().to_string();
        let rule = if m.contains("unknown field") {
            Rule::UnknownField
        } else if m.contains("missing field") {
            Rule::MissingField
        } else {
            Rule::Parse
        };
        (rule, m)
    })
}

/// Compiles the catalogue under `root` (the `data/` folder; `PROJECT.md` is read from its parent): A3.6's steps 1 to 5
/// and validation rules 1 to 4. With `assign`, ids missing from `data/ids.lock` get the next number of their kind and
/// the lock is written; otherwise they fail.
pub fn compile(root: &Path, assign: bool) -> Result<Compiled, Vec<CatalogueError>> {
    let mut errs = Errors { list: Vec::new() };
    let version: Version = match read(&root.join("VERSION.toml")) {
        Ok(t) => match toml::from_str(&t) {
            Ok(v) => v,
            Err(e) => {
                errs.add(Rule::Parse, "VERSION.toml", 0, e.message().to_string());
                Version::default()
            }
        },
        Err(e) => {
            errs.add(Rule::Read, "VERSION.toml", 0, e);
            Version::default()
        }
    };
    let project = match root.parent().map(|p| read(&p.join("PROJECT.md"))) {
        Some(Ok(t)) => source::project_items(&t),
        _ => {
            errs.add(
                Rule::Read,
                "../PROJECT.md",
                0,
                "PROJECT.md not found beside data/".into(),
            );
            Vec::new()
        }
    };
    let status: BTreeMap<&str, &str> = project.iter().map(|(i, s)| (i.as_str(), s.as_str())).collect();
    let (mut lock, lock_problems) = match read(&root.join("ids.lock")) {
        Ok(t) => Lock::parse(&t),
        Err(_) => (Lock::default(), Vec::new()),
    };
    for (line, m) in lock_problems {
        errs.add(Rule::Lock, "ids.lock", line, m);
    }
    for (retired, m) in lock.problems() {
        errs.add(if retired { Rule::RetiredNumber } else { Rule::Lock }, "ids.lock", 0, m);
    }

    // Step 1: every Markdown file in sorted order, split into entries; step 2: each block parsed into its kind.
    let mut files = Vec::new();
    if let Err(e) = markdown_files(root, root, &mut files) {
        errs.add(Rule::Read, "", 0, e);
    }
    let mut parsed: Vec<(KindName, String, RawEntry, Parsed)> = Vec::new();
    for rel in &files {
        let Some(kind) = kinds::kind_of(rel) else {
            if !kinds::UNCLAIMED_OK.contains(&rel.as_str()) {
                errs.add(
                    Rule::UnclaimedFile,
                    rel,
                    0,
                    "no kind of entry claims this file (kd_data::kinds::KINDS)".into(),
                );
            }
            continue;
        };
        let text = match read(&root.join(rel)) {
            Ok(t) => t,
            Err(e) => {
                errs.add(Rule::Read, rel, 0, e);
                continue;
            }
        };
        for raw in source::split(&text) {
            let (h, line) = (raw.heading.clone(), raw.line);
            match raw.blocks.len() {
                0 => errs.add(Rule::MissingBlock, rel, line, format!("`{h}` has no toml block")),
                1 => match parse_block(kind, &raw.blocks[0]) {
                    Ok(p) => parsed.push((kind, rel.clone(), raw, p)),
                    Err((rule, m)) => errs.add(rule, rel, line, format!("`{h}`: {m}")),
                },
                n => errs.add(
                    Rule::TwoBlocks,
                    rel,
                    line,
                    format!("`{h}` has {n} toml blocks, not one"),
                ),
            }
        }
    }

    // Common fields, ids and the lock (rule 2), checks and stage (rule 6).
    let mut seen: BTreeSet<(KindName, String)> = BTreeSet::new();
    let mut entries = Vec::new();
    let mut assigned = Vec::new();
    for (kind, rel, raw, p) in parsed {
        let c = p.common();
        let line = raw.line;
        if c.name != raw.heading {
            errs.add(
                Rule::NameMismatch,
                &rel,
                line,
                format!("name `{}` is not the heading `{}`", c.name, raw.heading),
            );
        }
        if !source::is_snake_case(&c.id) {
            errs.add(
                Rule::NotSnakeCase,
                &rel,
                line,
                format!("id `{}` is not snake_case", c.id),
            );
        }
        if !seen.insert((kind, c.id.clone())) {
            errs.add(
                Rule::RepeatedId,
                &rel,
                line,
                format!("{} id `{}` used twice", kind.lock_name(), c.id),
            );
            continue;
        }
        let stage_ok = c.stage.len() == 6
            && c.stage.starts_with("MIL-0")
            && status.get(c.stage.as_str()).is_some_and(|s| *s != "Dropped");
        if !stage_ok {
            errs.add(
                Rule::BadStage,
                &rel,
                line,
                format!("stage `{}` is not a MIL-0n of PROJECT.md", c.stage),
            );
        }
        if c.checks.is_empty() {
            errs.add(Rule::EmptyChecks, &rel, line, format!("`{}` names no checks", c.id));
        }
        for x in &c.checks {
            match status.get(x.as_str()) {
                Some(&"Dropped") => errs.add(
                    Rule::CheckNotLive,
                    &rel,
                    line,
                    format!("checks `{x}`, which PROJECT.md has dropped"),
                ),
                Some(_) => {}
                None => errs.add(
                    Rule::CheckNotLive,
                    &rel,
                    line,
                    format!("checks `{x}`, which is not in PROJECT.md"),
                ),
            }
        }
        let number = match lock.find(kind, &c.id) {
            Some(l) if l.retired => {
                errs.add(
                    Rule::RetiredNumber,
                    &rel,
                    line,
                    format!("id `{}` is retired in ids.lock", c.id),
                );
                continue;
            }
            Some(l) => l.number,
            None if assign => {
                assigned.push(c.id.clone());
                lock.assign(kind, &c.id)
            }
            None => {
                let m = format!("id `{}` is not in data/ids.lock: run `kd catalog build --assign`", c.id);
                errs.add(Rule::NotLocked, &rel, line, m);
                continue;
            }
        };
        entries.push(Entry {
            kind,
            file: rel,
            raw,
            number,
            parsed: p,
        });
    }
    entries.sort_by_key(|a| (a.kind, a.number));

    // Step 4: per-kind tables in number order, references resolved (rules 3 and 4).
    let body = build_body(&entries, &mut errs);
    if !errs.list.is_empty() {
        return Err(errs.list);
    }
    if assign && !assigned.is_empty() {
        write_file(&root.join("ids.lock"), &lock.text).map_err(|e| {
            vec![CatalogueError {
                rule: Rule::Read,
                file: "data/ids.lock".into(),
                line: 0,
                message: e,
            }]
        })?;
    }
    // Step 5: the blob.
    let blob = Catalogue::encode(version.major, version.minor, version.generator, &body);
    let catalogue = Catalogue::load(&blob).map_err(|e| {
        vec![CatalogueError {
            rule: Rule::BadValue,
            file: "data/".into(),
            line: 0,
            message: e.to_string(),
        }]
    })?;
    Ok(Compiled {
        catalogue,
        blob,
        entries,
        assigned,
    })
}

/// Writes a file under `data/` (A2.3 rule 4: the compile feature's only writes: `ids.lock`, tables and the index).
#[allow(clippy::disallowed_methods)]
pub fn write_file(path: &Path, text: &str) -> Result<(), String> {
    std::fs::write(path, text).map_err(|e| format!("{}: {e}", path.display()))
}

fn build_body(entries: &[Entry], errs: &mut Errors) -> Body {
    let mut body = Body::default();
    let mut colour_at: BTreeMap<String, u8> = BTreeMap::new();
    let mut family_at: BTreeMap<String, u8> = BTreeMap::new();
    for e in entries {
        let Parsed::ColourFamily(f) = &e.parsed else { continue };
        let fam = body.families.len();
        let first = body.colours.len();
        for (name, hex) in &f.colours {
            let at = body.colours.len();
            let rgb = source::hex_rgb(hex);
            if rgb.is_none() {
                errs.add(
                    Rule::BadValue,
                    &e.file,
                    e.raw.line,
                    format!("colour `{name}`: `{hex}` is not #rrggbb"),
                );
            }
            if name.is_empty() || !name.bytes().all(|c| c.is_ascii_alphanumeric()) {
                errs.add(
                    Rule::BadValue,
                    &e.file,
                    e.raw.line,
                    format!("colour name `{name}` is not letters and digits"),
                );
            }
            if at > 255 {
                errs.add(
                    Rule::BadValue,
                    &e.file,
                    e.raw.line,
                    format!("colour `{name}`: over 256 colours"),
                );
                continue;
            }
            if colour_at.insert(name.clone(), at as u8).is_some() {
                errs.add(
                    Rule::RepeatedId,
                    &e.file,
                    e.raw.line,
                    format!("colour `{name}` named twice"),
                );
            }
            body.colours.push(ColourRec {
                name: name.clone(),
                rgb: rgb.unwrap_or_default(),
                family: fam as u8,
            });
        }
        if f.colours.is_empty() {
            errs.add(
                Rule::BadValue,
                &e.file,
                e.raw.line,
                format!("family `{}` has no colours", f.common.id),
            );
        }
        family_at.insert(f.common.id.clone(), fam as u8);
        body.families.push(FamilyRec {
            id: f.common.id.clone(),
            name: f.common.name.clone(),
            first: first.min(255) as u8,
            count: (body.colours.len() - first) as u8,
        });
    }
    let n_colours = body.colours.len();
    let colour = |errs: &mut Errors, e: &Entry, name: &str| -> Option<u8> {
        let c = colour_at.get(name).copied();
        if c.is_none() {
            errs.add(
                Rule::UnresolvedReference,
                &e.file,
                e.raw.line,
                format!("no colour named `{name}`"),
            );
        }
        c
    };
    for e in entries {
        let Parsed::Ladder(l) = &e.parsed else { continue };
        if !(2..=8).contains(&l.steps.len()) {
            let m = format!("ladder `{}` has {} steps, not 2 to 8", l.common.id, l.steps.len());
            errs.add(Rule::BadValue, &e.file, e.raw.line, m);
        }
        let steps = l.steps.iter().filter_map(|s| colour(errs, e, s)).collect();
        body.ladders.push(LadderRec {
            id: l.common.id.clone(),
            name: l.common.name.clone(),
            steps,
        });
    }
    if body.ladders.len() > 256 {
        errs.add(Rule::BadValue, "palette/ladders.md", 0, "over 256 ladders".into());
    }
    for e in entries {
        let Parsed::LightTable(t) = &e.parsed else { continue };
        body.light
            .push(light_rec(t, e, n_colours, &body, &family_at, &colour_at, errs));
    }
    if body.light.iter().filter(|l| l.role == LightRole::Table).count() > 16 {
        errs.add(
            Rule::BadValue,
            "palette/light.md",
            0,
            "over 16 tables (the table texture's rows)".into(),
        );
    }
    let ladder_at: BTreeMap<&str, u8> = body
        .ladders
        .iter()
        .enumerate()
        .map(|(k, l)| (l.id.as_str(), k.min(255) as u8))
        .collect();
    let mut surfaces = Vec::new();
    for e in entries {
        let Parsed::Surface(s) = &e.parsed else { continue };
        surfaces.push(surface_rec(s, e, &ladder_at, errs));
    }
    if surfaces.len() > 64 {
        errs.add(
            Rule::BadValue,
            "models/surfaces.md",
            0,
            "over 64 surfaces (the surfaces texture's rows)".into(),
        );
    }
    body.surfaces = surfaces;
    body
}

/// A decimal string of at most three decimals in thousandths: `"0.38"` is 380, `"2"` is 2,000.
pub fn thousandths(s: &str) -> Option<u32> {
    if !source::is_decimal(s) || s.starts_with('-') {
        return None;
    }
    let (whole, frac) = s.split_once('.').unwrap_or((s, ""));
    if frac.len() > 3 || whole.len() > 6 {
        return None;
    }
    let w: u32 = whole.parse().ok()?;
    let f: u32 = format!("{frac:0<3}").parse().ok()?;
    Some(w * 1000 + f)
}

fn surface_rec(s: &Surface, e: &Entry, ladder_at: &BTreeMap<&str, u8>, errs: &mut Errors) -> SurfaceRec {
    let id = &s.common.id;
    let ladder = ladder_at.get(s.ladder.as_str()).copied().unwrap_or_else(|| {
        errs.add(
            Rule::UnresolvedReference,
            &e.file,
            e.raw.line,
            format!("`{id}`: no ladder `{}`", s.ladder),
        );
        0
    });
    let mut num = |field: &str, v: &str, max: u32, unit: &str| -> u16 {
        match thousandths(v) {
            Some(k) if k <= max => k as u16,
            _ => {
                let m = format!("`{id}`: {field} `{v}` is not a decimal of at most three places from 0 to {}{unit}", max / 1000);
                errs.add(Rule::BadValue, &e.file, e.raw.line, m);
                0
            }
        }
    };
    let stone_density = num("stone_density", &s.stone_density, 1000, "");
    let stone_size_mm = num("stone_size", &s.stone_size, 10_000, " m");
    let tuft_density = num("tuft_density", &s.tuft_density, 1000, "");
    SurfaceRec {
        id: id.clone(),
        name: s.common.name.clone(),
        ladder,
        stone_density,
        stone_size_mm,
        tuft_density,
        flags: s.flags.iter().fold(0, |b, f| b | f.bit()),
    }
}

fn light_rec(
    t: &LightTable,
    e: &Entry,
    n_colours: usize,
    body: &Body,
    family_at: &BTreeMap<String, u8>,
    colour_at: &BTreeMap<String, u8>,
    errs: &mut Errors,
) -> LightRec {
    let id = &t.common.id;
    let mut bad = |rule: Rule, m: String| errs.add(rule, &e.file, e.raw.line, format!("`{id}`: {m}"));
    let mut num = |field: &str, v: &Option<String>, needed: bool| -> String {
        match v {
            Some(s) if source::is_decimal(s) => s.clone(),
            Some(s) => {
                bad(Rule::BadValue, format!("{field} `{s}` is not a decimal number"));
                String::from("0")
            }
            None if needed => {
                bad(Rule::MissingField, format!("method needs `{field}`"));
                String::from("0")
            }
            None => String::from("0"),
        }
    };
    let (warm, haze, lab) = (
        t.method == LightMethod::Warm,
        t.method == LightMethod::Haze,
        t.method == LightMethod::Lab,
    );
    let k = num("k", &t.k, warm);
    let amount = num("amount", &t.amount, haze);
    let triple = |num: &mut dyn FnMut(&str, &Option<String>, bool) -> String, f: &str, v: &Option<[String; 3]>| {
        let v = v.clone().map(|a| a.map(Some)).unwrap_or([None, None, None]);
        [0, 1, 2].map(|i| num(f, &v[i], lab))
    };
    let mul = triple(&mut num, "mul", &t.mul);
    let add = triple(&mut num, "add", &t.add);
    let l_min = t.l_min.as_ref().map(|s| num("l_min", &Some(s.clone()), false));
    let l_max = t.l_max.as_ref().map(|s| num("l_max", &Some(s.clone()), false));
    if haze && t.toward.is_none() {
        bad(Rule::MissingField, "method needs `toward`".into());
    }
    let empty = Vec::new();
    // (families listed, families excluded, colours named) and the fields the other role uses, which must be empty.
    type Lists<'a> = (
        (&'a Vec<String>, &'a Vec<String>, &'a Vec<String>),
        [&'a Vec<String>; 3],
    );
    let ((fam_in, fam_out, col_names), other): Lists = match t.role {
        LightRole::Table => (
            (&t.only_families, &t.exclude_families, &t.exclude_colours),
            [&t.keep_families, &t.keep_colours, &empty],
        ),
        LightRole::Version => (
            (&t.keep_families, &empty, &t.keep_colours),
            [&t.only_families, &t.exclude_families, &t.exclude_colours],
        ),
    };
    if other.iter().any(|v| !v.is_empty()) {
        let m = "a table names only_families, exclude_families and exclude_colours; a version keep_families and keep_colours";
        bad(Rule::BadValue, m.into());
    }
    let mut fams = |names: &Vec<String>| -> BTreeSet<u8> {
        names
            .iter()
            .filter_map(|n| {
                let f = family_at.get(n).copied();
                if f.is_none() {
                    bad(Rule::UnresolvedReference, format!("no colour family `{n}`"));
                }
                f
            })
            .collect()
    };
    let (f0, f1) = (fams(fam_in), fams(fam_out));
    let cols: BTreeSet<u8> = col_names
        .iter()
        .filter_map(|n| {
            let c = colour_at.get(n).copied();
            if c.is_none() {
                errs.add(
                    Rule::UnresolvedReference,
                    &e.file,
                    e.raw.line,
                    format!("`{id}`: no colour `{n}`"),
                );
            }
            c
        })
        .collect();
    let mask = (0..n_colours)
        .map(|k| {
            let fam = body.colours[k].family;
            match t.role {
                // A table may give a colour when its family is listed (or none are), its family is not excluded, and
                // the colour itself is not excluded.
                LightRole::Table => {
                    (f0.is_empty() || f0.contains(&fam)) && !f1.contains(&fam) && !cols.contains(&(k as u8))
                }
                // A version keeps a colour unchanged when its family or the colour itself is kept.
                LightRole::Version => f0.contains(&fam) || cols.contains(&(k as u8)),
            }
        })
        .collect();
    LightRec {
        id: id.clone(),
        name: t.common.name.clone(),
        role: t.role,
        method: t.method,
        k,
        toward: t.toward.unwrap_or_default(),
        amount,
        mul,
        add,
        l_min,
        l_max,
        mask,
    }
}

/// `kd catalog check` (A3.6, `PRC-10`): compiles without assigning, then fails every generated table, and
/// `data/INDEX.md`, that does not match the blocks (rule 6).
pub fn check(root: &Path) -> Result<Compiled, Vec<CatalogueError>> {
    let c = compile(root, false)?;
    let mut errs = Errors { list: Vec::new() };
    let mut texts: BTreeMap<&str, String> = BTreeMap::new();
    for e in &c.entries {
        let text = texts
            .entry(&e.file)
            .or_insert_with(|| read(&root.join(&e.file)).unwrap_or_default());
        let lines: Vec<&str> = text.split('\n').collect();
        let have = e.raw.table.map(|(a, b)| lines[a..b].join("\n"));
        if have.as_deref() != Some(tables::table(e).as_str()) {
            let m = format!(
                "`{}`: generated table out of date: run `kd catalog tables`",
                e.raw.heading
            );
            errs.add(Rule::StaleTable, &e.file, e.raw.line, m);
        }
    }
    let index = read(&root.join("INDEX.md")).unwrap_or_default();
    if index != tables::index(&c.entries) {
        errs.add(
            Rule::StaleTable,
            "INDEX.md",
            0,
            "out of date: run `kd catalog tables`".into(),
        );
    }
    if errs.list.is_empty() { Ok(c) } else { Err(errs.list) }
}

#[cfg(test)]
mod tests;
