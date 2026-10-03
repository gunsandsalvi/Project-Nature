//! The compiler's part for the land's kinds (A3.6, A5.6, A5.7, A5.11): rocks, soils, biomes, deposits and land
//! presets as written, their checks, and their human tables. Values with units go through `units`, names resolve
//! to the numbers of the entries compiled before them (rule 4), and every failure names its entry.

use serde::Deserialize;

use super::{Compiled, Entry, Head, Problem, and_list, at, head, parse};
use crate::schema::{Look, Surface};
use crate::units::{Dim, amount, value};
use crate::world::{
    Biome, COVER_GROUPS, Carried, Climate, Deposit, Escarpment, Herd, Land, Landform, Rock, Side, Soil, Valley, keeps,
    shares_255,
};

/// The landforms' words, as entries write them.
const LANDFORM_WORDS: [(&str, Landform); 6] = [
    ("floodplain", Landform::Floodplain),
    ("wetland", Landform::Wetland),
    ("coast", Landform::Coast),
    ("sea", Landform::Sea),
    ("scarp_top", Landform::ScarpTop),
    ("scarp_foot", Landform::ScarpFoot),
];

/// A landform's words in the tables.
fn landform_words(l: Landform) -> &'static str {
    match l {
        Landform::Floodplain => "a river's floodplain",
        Landform::Wetland => "a flat floodplain draining a wide land",
        Landform::Coast => "land by the sea",
        Landform::Sea => "the sea",
        Landform::ScarpTop => "an escarpment's high side",
        Landform::ScarpFoot => "an escarpment's face and foot",
    }
}

/// The cover groups' words in the tables, in their order (A5.2).
const COVER_WORDS: [&str; COVER_GROUPS] = ["trees", "bushes", "grass and herbs", "reeds", "bare ground"];
/// The words a soil's `keeps` writes, by flag.
const KEEPS_WORDS: [(&str, u8); 3] = [("bone", keeps::BONE), ("wood", keeps::WOOD), ("hide", keeps::HIDE)];

/// What an entry compiles to: its head, its human table and its entry.
type Out = Option<(Head, Vec<(String, String)>, Compiled)>;

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct RockSrc {
    id: String,
    name: String,
    stage: String,
    checks: Vec<String>,
    softness: f32,
    beds: String,
    caves: bool,
    look: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct SoilSrc {
    id: String,
    name: String,
    stage: String,
    checks: Vec<String>,
    capacity: String,
    intake: String,
    dig: u8,
    keeps: Vec<String>,
    fertility_shift: i8,
    /// Written for a soil that forms on a kind of ground, and only then.
    #[serde(default)]
    landform: Option<String>,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct SharesSrc {
    trees: f32,
    bushes: f32,
    grass: f32,
    reeds: f32,
    bare: f32,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct BiomeSrc {
    id: String,
    name: String,
    stage: String,
    checks: Vec<String>,
    look: String,
    cover: SharesSrc,
    surfaces: CoverSurfacesSrc,
    /// Written for a biome a kind of ground takes, and only then.
    #[serde(default)]
    landform: Option<String>,
}

/// The surface each cover group shows as, by id.
#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct CoverSurfacesSrc {
    trees: String,
    bushes: String,
    grass: String,
    reeds: String,
    bare: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct CarriedSrc {
    keep: f32,
    every: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct DepositSrc {
    id: String,
    name: String,
    stage: String,
    checks: Vec<String>,
    /// Written when it lies in a rock, and only then: TOML has no word for none.
    #[serde(default)]
    rock: Option<String>,
    rivers: bool,
    richness: [u8; 2],
    /// Written when a river carries it, and only then.
    #[serde(default)]
    carried: Option<CarriedSrc>,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct ValleySrc {
    flow: String,
    width: String,
    floodplain: String,
    from: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct EscarpmentSrc {
    side: String,
    height: String,
    rocks: Vec<String>,
    caves: u8,
    shelters: u8,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct LandSoilSrc {
    kind: String,
    fertility: u8,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct LandCoverSrc {
    biome: String,
    trees: f32,
    bushes: f32,
    grass: f32,
    reeds: f32,
    bare: f32,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct ClimateSrc {
    mean: [String; 4],
    range: String,
    rain: [String; 4],
    storm_days: [u8; 4],
    thunder_days: [u8; 4],
    wind: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct HerdSrc {
    kind: String,
    count: u32,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct SmallSrc {
    hare: f32,
    birds: f32,
    fish: f32,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct SkySrc {
    tilt: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct LandSrc {
    id: String,
    name: String,
    stage: String,
    checks: Vec<String>,
    centre_cell: [u32; 2],
    island_km: f32,
    sea_km: f32,
    base_height: String,
    relief: String,
    valley: ValleySrc,
    escarpment: EscarpmentSrc,
    soil: LandSoilSrc,
    cover: LandCoverSrc,
    climate: ClimateSrc,
    herds: Vec<HerdSrc>,
    small: SmallSrc,
    sky: SkySrc,
}

/// Where problems of one entry go: at its block, prefixed with its id.
struct Report<'a> {
    e: &'a Entry,
    id: String,
    problems: &'a mut Vec<Problem>,
}

impl Report<'_> {
    fn bad(&mut self, what: impl Into<String>) {
        let what = what.into();
        self.problems
            .push(at(&self.e.path, self.e.block_line, format!("{}: {what}", self.id)));
    }

    /// A value read by `units`, or 0 and its problem.
    fn unit(&mut self, r: Result<f32, String>) -> f32 {
        r.unwrap_or_else(|e| {
            self.bad(e);
            0.0
        })
    }

    /// The number of the entry named `id` among `known`, or its problem (rule 4).
    fn resolve<T>(&mut self, field: &str, id: &str, known: &[(u16, T)], id_of: fn(&T) -> &str, kind: &str) -> u16 {
        known.iter().find(|k| id_of(&k.1) == id).map_or_else(
            || {
                self.bad(format!("field `{field}`: {id:?} is not a {kind}"));
                0
            },
            |k| k.0,
        )
    }

    /// A compass side's word.
    fn side(&mut self, field: &str, word: &str) -> Side {
        Side::ALL.into_iter().find(|s| s.word() == word).unwrap_or_else(|| {
            self.bad(format!("field `{field}`: {word:?} is not north, east, south or west"));
            Side::North
        })
    }

    /// A landform's word.
    fn landform(&mut self, word: Option<&str>) -> Option<Landform> {
        let w = word?;
        let found = LANDFORM_WORDS.iter().find(|l| l.0 == w).map(|l| l.1);
        if found.is_none() {
            let all: Vec<String> = LANDFORM_WORDS.iter().map(|l| l.0.to_string()).collect();
            self.bad(format!("field `landform`: {w:?} is not {}", and_list(&all)));
        }
        found
    }

    /// Cover shares of the five groups, each 0 to 1 and summing to 1, as 255ths.
    fn shares(&mut self, field: &str, s: [f32; COVER_GROUPS]) -> [u8; COVER_GROUPS] {
        let sum: f32 = s.iter().sum();
        if s.iter().any(|v| !(0.0..=1.0).contains(v)) || (sum - 1.0).abs() > 1e-3 {
            self.bad(format!(
                "field `{field}`: the five shares are each 0 to 1 and sum to 1, not {sum:.3}"
            ));
        }
        shares_255(s)
    }
}

fn look_id(l: &Look) -> &str {
    &l.id
}

/// The shares' words, as a table shows them: "trees 55%, bushes 15%, …".
fn shares_words(cover: [u8; COVER_GROUPS]) -> String {
    and_list(
        &COVER_WORDS
            .iter()
            .zip(cover)
            .map(|(g, v)| format!("{g} {:.0}%", f32::from(v) / 2.55))
            .collect::<Vec<_>>(),
    )
}

/// A number as a table writes it: whole when it is whole.
fn num(v: f32) -> String {
    if v.fract() == 0.0 {
        format!("{v:.0}")
    } else {
        format!("{v}")
    }
}

pub(super) fn rock(e: &Entry, looks: &[(u16, Look)], problems: &mut Vec<Problem>) -> Out {
    let s = parse::<RockSrc>(e, problems)?;
    let mut r = Report {
        e,
        id: s.id.clone(),
        problems,
    };
    if !(0.5..=2.0).contains(&s.softness) {
        r.bad(format!("softness {} is outside A5.7's 0.5 to 2", s.softness));
    }
    let beds = amount("beds", &s.beds, Dim::Length).unwrap_or_else(|e| {
        r.bad(e);
        crate::units::Amount { lo: 0.0, hi: 0.0 }
    });
    if !(beds.lo > 0.0 && beds.hi.is_finite()) {
        r.bad("field `beds`: a range of thicknesses above 0");
    }
    let look = r.resolve("look", &s.look, looks, look_id, "look");
    let look_name = looks
        .iter()
        .find(|l| l.0 == look)
        .map_or(String::new(), |l| l.1.name.clone());
    let table = vec![
        ("Softness".to_string(), format!("{} (A5.7: 0.5 to 2)", num(s.softness))),
        (
            "Beds".to_string(),
            format!("{} to {} m thick", num(beds.lo), num(beds.hi)),
        ),
        (
            "Caves".to_string(),
            if s.caves { "form in it" } else { "none" }.to_string(),
        ),
        ("Look".to_string(), look_name),
    ];
    let entry = Rock {
        id: s.id.clone(),
        name: s.name.clone(),
        number: 0,
        softness: s.softness,
        beds_m: [beds.lo, beds.hi],
        caves: s.caves,
        look,
    };
    Some((head(&s.id, &s.name, &s.stage, &s.checks), table, Compiled::Rock(entry)))
}

pub(super) fn soil(e: &Entry, problems: &mut Vec<Problem>) -> Out {
    let s = parse::<SoilSrc>(e, problems)?;
    let mut r = Report {
        e,
        id: s.id.clone(),
        problems,
    };
    let capacity = r.unit(value("capacity", &s.capacity, Dim::Length));
    let intake = r.unit(value("intake", &s.intake, Dim::Length));
    if capacity <= 0.0 || intake <= 0.0 {
        r.bad("its capacity and intake are above 0");
    }
    if !(1..=10).contains(&s.dig) {
        r.bad(format!("dig {} is outside 1 to 10 (MAT-06)", s.dig));
    }
    if !(-2..=2).contains(&s.fertility_shift) {
        r.bad(format!("fertility_shift {} is outside −2 to 2", s.fertility_shift));
    }
    let landform = r.landform(s.landform.as_deref());
    let mut flags = 0;
    for word in &s.keeps {
        match KEEPS_WORDS.iter().find(|w| w.0 == word) {
            Some(&(_, f)) => flags |= f,
            None => r.bad(format!("field `keeps`: {word:?} is not bone, wood or hide")),
        }
    }
    let kept: Vec<String> = KEEPS_WORDS
        .iter()
        .filter(|w| flags & w.1 != 0)
        .map(|w| w.0.to_string())
        .collect();
    let table = vec![
        (
            "Water held".to_string(),
            format!(
                "holds {} mm, takes in {} mm a game hour",
                num(capacity * 1000.0),
                num(intake * 1000.0)
            ),
        ),
        ("Digging".to_string(), format!("{} of 10", s.dig)),
        (
            "Fertility".to_string(),
            match s.fertility_shift {
                0 => "its land's".to_string(),
                n => format!("{n:+} on its land's"),
            },
        ),
        (
            "Forms on".to_string(),
            landform.map_or("its land's choice".to_string(), |l| landform_words(l).to_string()),
        ),
        (
            "Keeps".to_string(),
            if kept.is_empty() {
                "nothing buried".into()
            } else {
                and_list(&kept)
            },
        ),
    ];
    let entry = Soil {
        id: s.id.clone(),
        name: s.name.clone(),
        number: 0,
        capacity_m: capacity,
        intake_m: intake,
        dig: s.dig,
        keeps: flags,
        fertility_shift: s.fertility_shift,
        landform,
    };
    Some((head(&s.id, &s.name, &s.stage, &s.checks), table, Compiled::Soil(entry)))
}

pub(super) fn biome(e: &Entry, looks: &[(u16, Look)], surfaces: &[(u16, Surface)], problems: &mut Vec<Problem>) -> Out {
    let s = parse::<BiomeSrc>(e, problems)?;
    let mut r = Report {
        e,
        id: s.id.clone(),
        problems,
    };
    let look = r.resolve("look", &s.look, looks, look_id, "look");
    let c = &s.cover;
    let cover = r.shares("cover", [c.trees, c.bushes, c.grass, c.reeds, c.bare]);
    let v = &s.surfaces;
    let shows = [&v.trees, &v.bushes, &v.grass, &v.reeds, &v.bare]
        .map(|id| r.resolve("surfaces", id, surfaces, |k: &Surface| &k.id, "surface"));
    let landform = r.landform(s.landform.as_deref());
    let look_name = looks
        .iter()
        .find(|l| l.0 == look)
        .map_or(String::new(), |l| l.1.name.clone());
    let surface_name = |n: u16| {
        surfaces
            .iter()
            .find(|k| k.0 == n)
            .map_or(String::new(), |k| k.1.name.clone())
    };
    let shown: Vec<String> = COVER_WORDS
        .iter()
        .zip(shows)
        .map(|(g, n)| format!("{g} as {}", surface_name(n)))
        .collect();
    let table = vec![
        ("Map look".to_string(), look_name),
        ("Cover".to_string(), shares_words(cover)),
        ("Shows".to_string(), and_list(&shown)),
        (
            "Takes".to_string(),
            landform.map_or("its land's choice".to_string(), |l| landform_words(l).to_string()),
        ),
    ];
    let entry = Biome {
        id: s.id.clone(),
        name: s.name.clone(),
        number: 0,
        look,
        cover,
        surfaces: shows,
        landform,
    };
    Some((head(&s.id, &s.name, &s.stage, &s.checks), table, Compiled::Biome(entry)))
}

pub(super) fn deposit(e: &Entry, rocks: &[(u16, Rock)], problems: &mut Vec<Problem>) -> Out {
    let s = parse::<DepositSrc>(e, problems)?;
    let mut r = Report {
        e,
        id: s.id.clone(),
        problems,
    };
    let rock = s
        .rock
        .as_deref()
        .map(|id| r.resolve("rock", id, rocks, |k: &Rock| &k.id, "rock"));
    if rock.is_none() && !s.rivers {
        r.bad("a deposit lies in a rock or in rivers");
    }
    let [lo, hi] = s.richness;
    if !(1..=5).contains(&lo) || !(lo..=5).contains(&hi) {
        r.bad(format!("richness {lo} to {hi}: from 1 to 5, rising"));
    }
    let carried = s.carried.as_ref().map(|c| {
        if !(c.keep > 0.0 && c.keep < 1.0) {
            r.bad(format!(
                "field `carried`: keeps {} of its share, between 0 and 1",
                c.keep
            ));
        }
        let every_m = r.unit(value("carried.every", &c.every, Dim::Length));
        if every_m <= 0.0 {
            r.bad("field `carried.every`: a distance above 0");
        }
        Carried { keep: c.keep, every_m }
    });
    let mut lies = Vec::new();
    if let Some(id) = &s.rock {
        lies.push(id.replace('_', " "));
    }
    if s.rivers {
        lies.push("river cells".to_string());
    }
    let table = vec![
        ("Lies in".to_string(), and_list(&lies)),
        ("Richness".to_string(), format!("{lo} to {hi} of 5")),
        (
            "Carried".to_string(),
            carried.map_or("no".into(), |c| {
                format!(
                    "downstream, keeping {} of its share every {} km",
                    num(c.keep),
                    num(c.every_m / 1000.0)
                )
            }),
        ),
    ];
    let entry = Deposit {
        id: s.id.clone(),
        name: s.name.clone(),
        number: 0,
        rock,
        rivers: s.rivers,
        richness: s.richness,
        carried,
    };
    Some((
        head(&s.id, &s.name, &s.stage, &s.checks),
        table,
        Compiled::Deposit(entry),
    ))
}

/// What a land preset's names resolve against: the rocks, soils and biomes compiled before it.
pub(super) struct Known<'a> {
    pub rocks: &'a [(u16, Rock)],
    pub soils: &'a [(u16, Soil)],
    pub biomes: &'a [(u16, Biome)],
}

pub(super) fn land(e: &Entry, known: &Known, problems: &mut Vec<Problem>) -> Out {
    let s = parse::<LandSrc>(e, problems)?;
    let mut r = Report {
        e,
        id: s.id.clone(),
        problems,
    };
    let [cx, cy] = s.centre_cell;
    if cx >= kd_core::geo::CELLS_X || cy >= kd_core::geo::CELLS_Y {
        r.bad(format!("centre_cell [{cx}, {cy}] is off the 2,000 × 1,000 cells"));
    }
    let span_km = s.island_km + 2.0 * s.sea_km;
    if s.island_km <= 0.0 || s.sea_km <= 0.0 || span_km >= 1024.0 {
        r.bad("an island and its sea are above 0 km and fit the world's height");
    }
    let base_height_m = r.unit(value("base_height", &s.base_height, Dim::Length));
    let relief_m = r.unit(value("relief", &s.relief, Dim::Length));
    let v = &s.valley;
    let valley = Valley {
        flow_m3s: r.unit(value("valley.flow", &v.flow, Dim::Flow)),
        width_m: r.unit(value("valley.width", &v.width, Dim::Length)),
        floodplain_m: r.unit(value("valley.floodplain", &v.floodplain, Dim::Length)),
        from: r.side("valley.from", &v.from),
    };
    if valley.flow_m3s < 0.0 || valley.width_m <= 0.0 || valley.floodplain_m < valley.width_m {
        r.bad("field `valley`: a flow of 0 or more, a width above 0 and a floodplain at least as wide");
    }
    let x = &s.escarpment;
    let rocks: Vec<u16> = x
        .rocks
        .iter()
        .map(|id| r.resolve("escarpment.rocks", id, known.rocks, |k: &Rock| &k.id, "rock"))
        .collect();
    if !(1..=3).contains(&rocks.len()) {
        r.bad("field `escarpment.rocks`: one to three rocks, top first (A5.2)");
    }
    let escarpment = Escarpment {
        side: r.side("escarpment.side", &x.side),
        height_m: r.unit(value("escarpment.height", &x.height, Dim::Length)),
        rocks,
        caves: x.caves,
        shelters: x.shelters,
    };
    if escarpment.height_m < 0.0 {
        r.bad("field `escarpment.height`: 0 m or more");
    }
    let soil = r.resolve("soil.kind", &s.soil.kind, known.soils, |k: &Soil| &k.id, "soil");
    if s.soil.fertility > 5 {
        r.bad(format!("field `soil.fertility`: {} is above 5", s.soil.fertility));
    }
    let c = &s.cover;
    let biome = r.resolve("cover.biome", &c.biome, known.biomes, |k: &Biome| &k.id, "biome");
    let cover = r.shares("cover", [c.trees, c.bushes, c.grass, c.reeds, c.bare]);
    let m = &s.climate;
    let mut four = |field: &str, v: &[String; 4], dim: Dim| -> [f32; 4] {
        std::array::from_fn(|k| r.unit(value(field, &v[k], dim)))
    };
    let mean_c = four("climate.mean", &m.mean, Dim::Temperature);
    let rain_m = four("climate.rain", &m.rain, Dim::Length);
    let range_c = r.unit(value("climate.range", &m.range, Dim::Temperature));
    if range_c < 0.0 || rain_m.iter().any(|&v| v < 0.0) {
        r.bad("field `climate`: a daily range and rain of 0 or more");
    }
    let days_in_season = (kd_core::time::SEASON / kd_core::time::DAY) as u8;
    if m.storm_days.iter().chain(&m.thunder_days).any(|&d| d > days_in_season) {
        r.bad(format!(
            "field `climate`: storm and thunder days are at most a season's {days_in_season}"
        ));
    }
    let climate = Climate {
        mean_c,
        range_c,
        rain_m,
        storm_days: m.storm_days,
        thunder_days: m.thunder_days,
        wind: r.side("climate.wind", &m.wind),
    };
    let small = [s.small.hare, s.small.birds, s.small.fish];
    if small.iter().any(|&v| v < 0.0) || s.herds.iter().any(|h| h.count == 0) {
        r.bad("fields `herds` and `small`: counts above 0 and densities of 0 or more");
    }
    let tilt_deg = r.unit(value("sky.tilt", &s.sky.tilt, Dim::Angle));
    if !(0.0..=90.0).contains(&tilt_deg) {
        r.bad(format!("field `sky.tilt`: {tilt_deg}° is outside 0 to 90"));
    }
    let name_of = |id: &str| id.replace('_', " ");
    let herds: Vec<String> = s
        .herds
        .iter()
        .map(|h| format!("{} {}", name_of(&h.kind), h.count))
        .collect();
    let table = vec![
        (
            "Island".to_string(),
            format!(
                "{} km across in a sea {} km wide, around cell {cx}, {cy}",
                num(s.island_km),
                num(s.sea_km)
            ),
        ),
        (
            "Height".to_string(),
            format!("{} m, varying by {} m", num(base_height_m), num(relief_m)),
        ),
        (
            "Valley".to_string(),
            format!(
                "from the {}: a river of {} m³/s, {} m wide, in a floodplain {} m wide",
                valley.from.word(),
                num(valley.flow_m3s),
                num(valley.width_m),
                num(valley.floodplain_m)
            ),
        ),
        (
            "Escarpment".to_string(),
            format!(
                "{} of the middle, {} m high, of {}; {} caves and {} shelters",
                escarpment.side.word(),
                num(escarpment.height_m),
                and_list(&x.rocks.iter().map(|id| name_of(id)).collect::<Vec<_>>()),
                x.caves,
                x.shelters
            ),
        ),
        (
            "Soil and cover".to_string(),
            format!(
                "{} at fertility {}; {}: {}",
                name_of(&s.soil.kind),
                s.soil.fertility,
                name_of(&c.biome),
                shares_words(cover)
            ),
        ),
        (
            "Climate".to_string(),
            format!(
                "seasons' means {} °C, daily range {} °C; rain {} mm; storm days {}, thunder days {}; wind from the {}",
                mean_c.map(num).join(", "),
                num(range_c),
                rain_m.map(|v| num(v * 1000.0)).join(", "),
                m.storm_days.map(|d| d.to_string()).join(", "),
                m.thunder_days.map(|d| d.to_string()).join(", "),
                climate.wind.word()
            ),
        ),
        (
            "Animals".to_string(),
            format!(
                "herds: {}; a km² of habitat holds {} hares, {} birds and {} fish",
                if herds.is_empty() {
                    "none".into()
                } else {
                    and_list(&herds)
                },
                num(small[0]),
                num(small[1]),
                num(small[2])
            ),
        ),
        ("Sky".to_string(), format!("the world tilted {}°", num(tilt_deg))),
    ];
    let entry = Land {
        id: s.id.clone(),
        name: s.name.clone(),
        number: 0,
        centre_cell: s.centre_cell,
        island_m: s.island_km * 1000.0,
        sea_m: s.sea_km * 1000.0,
        base_height_m,
        relief_m,
        valley,
        escarpment,
        soil,
        fertility: s.soil.fertility,
        biome,
        cover,
        climate,
        herds: s
            .herds
            .iter()
            .map(|h| Herd {
                kind: h.kind.clone(),
                count: h.count,
            })
            .collect(),
        small,
        tilt_deg,
    };
    Some((head(&s.id, &s.name, &s.stage, &s.checks), table, Compiled::Land(entry)))
}
