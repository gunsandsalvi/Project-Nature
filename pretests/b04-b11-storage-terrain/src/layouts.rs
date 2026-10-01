//! B04-1: layouts in memory. Array of structs, struct of arrays, and the hecs entity-component
//! crate, on 100,000 animals, 1,000,000 things and 10,000 people cores. Each pass touches a few
//! fields of every record. Memory per record comes from a counting allocator (see main.rs).

use crate::{allocated, median_spread};
use kstorage::data::*;
use std::hint::black_box;
use std::time::Instant;

struct AnimalsSoA {
    id: Vec<u32>,
    species: Vec<u16>,
    flags: Vec<u16>,
    x: Vec<u32>,
    y: Vec<u32>,
    energy: Vec<f32>,
    health: Vec<f32>,
    age: Vec<u32>,
    herd: Vec<u32>,
}
struct ThingsSoA {
    id: Vec<u32>,
    kind: Vec<u16>,
    material: Vec<u16>,
    x: Vec<u32>,
    y: Vec<u32>,
    z: Vec<i16>,
    condition: Vec<u16>,
    mass: Vec<u32>,
    owner: Vec<u32>,
    made: Vec<i32>,
}
struct PeopleSoA {
    ident: Vec<[u32; 6]>,
    body: Vec<[f32; 16]>,
    needs: Vec<[f32; 8]>,
    feel: Vec<[f32; 8]>,
    skills: Vec<[u16; 32]>,
    traits: Vec<[u8; 40]>,
}

// hecs components (one per field, as an entity-component design would split them)
#[derive(Clone, Copy)]
struct Id(u32);
#[derive(Clone, Copy)]
struct Species(u16);
#[derive(Clone, Copy)]
struct Flags(u16);
#[derive(Clone, Copy)]
struct Pos(u32, u32);
#[derive(Clone, Copy)]
struct Energy(f32);
#[derive(Clone, Copy)]
struct Health(f32);
#[derive(Clone, Copy)]
struct Age(u32);
#[derive(Clone, Copy)]
struct Herd(u32);
#[derive(Clone, Copy)]
struct Kind(u16);
#[derive(Clone, Copy)]
struct Material(u16);
#[derive(Clone, Copy)]
struct Z(i16);
#[derive(Clone, Copy)]
struct Condition(u16);
#[derive(Clone, Copy)]
struct Mass(u32);
#[derive(Clone, Copy)]
struct Owner(u32);
#[derive(Clone, Copy)]
struct Made(i32);
#[derive(Clone, Copy)]
struct Ident([u32; 6]);
#[derive(Clone, Copy)]
struct Body([f32; 16]);
#[derive(Clone, Copy)]
struct Needs([f32; 8]);
#[derive(Clone, Copy)]
struct Feel([f32; 8]);
#[derive(Clone, Copy)]
struct Skills([u16; 32]);
#[derive(Clone, Copy)]
struct Traits([u8; 40]);

const DECAY: [u16; 64] = {
    let mut d = [0u16; 64];
    let mut i = 0;
    while i < 64 {
        d[i] = (i % 5) as u16;
        i += 1;
    }
    d
};

#[inline(never)]
fn pass_animals_aos(a: &mut [Animal]) {
    for r in a.iter_mut() {
        r.energy = r.energy * 0.999 - 0.0005 * (r.species % 3) as f32;
        r.age += 1;
        r.health = (r.health + 0.001).min(1.0);
    }
}
#[inline(never)]
fn pass_animals_soa(a: &mut AnimalsSoA) {
    for i in 0..a.energy.len() {
        a.energy[i] = a.energy[i] * 0.999 - 0.0005 * (a.species[i] % 3) as f32;
        a.age[i] += 1;
        a.health[i] = (a.health[i] + 0.001).min(1.0);
    }
}
#[inline(never)]
fn pass_animals_ecs(w: &mut hecs::World) {
    for (_, (e, ag, h, s)) in w.query_mut::<(&mut Energy, &mut Age, &mut Health, &Species)>() {
        e.0 = e.0 * 0.999 - 0.0005 * (s.0 % 3) as f32;
        ag.0 += 1;
        h.0 = (h.0 + 0.001).min(1.0);
    }
}
#[inline(never)]
fn pass_things_aos(t: &mut [Thing]) {
    for r in t.iter_mut() {
        r.condition = r.condition.saturating_sub(DECAY[(r.material & 63) as usize]);
    }
}
#[inline(never)]
fn pass_things_soa(t: &mut ThingsSoA) {
    for i in 0..t.condition.len() {
        t.condition[i] = t.condition[i].saturating_sub(DECAY[(t.material[i] & 63) as usize]);
    }
}
#[inline(never)]
fn pass_things_ecs(w: &mut hecs::World) {
    for (_, (c, m)) in w.query_mut::<(&mut Condition, &Material)>() {
        c.0 = c.0.saturating_sub(DECAY[(m.0 & 63) as usize]);
    }
}
#[inline(never)]
fn pass_people_aos(p: &mut [PersonCore]) {
    for r in p.iter_mut() {
        for k in 0..4 {
            r.needs[k] = (r.needs[k] + 0.01).min(1.0);
        }
        r.feel[0] *= 0.99;
    }
}
#[inline(never)]
fn pass_people_soa(p: &mut PeopleSoA) {
    for i in 0..p.needs.len() {
        for k in 0..4 {
            p.needs[i][k] = (p.needs[i][k] + 0.01).min(1.0);
        }
        p.feel[i][0] *= 0.99;
    }
}
#[inline(never)]
fn pass_people_ecs(w: &mut hecs::World) {
    for (_, (n, f)) in w.query_mut::<(&mut Needs, &mut Feel)>() {
        for k in 0..4 {
            n.0[k] = (n.0[k] + 0.01).min(1.0);
        }
        f.0[0] *= 0.99;
    }
}

fn time_pass(n: usize, passes: usize, runs: usize, f: &mut dyn FnMut()) -> (f64, f64, f64) {
    f(); // warm-up
    let mut v = Vec::new();
    for _ in 0..runs {
        let t = Instant::now();
        for _ in 0..passes {
            f();
        }
        v.push(t.elapsed().as_secs_f64() * 1e9 / (n * passes) as f64);
    }
    median_spread(v)
}

pub fn run() {
    let s = build(42, 1000, 1);
    let runs = 7;
    println!("# B04-1 layouts: ns per record per pass (median of {runs} runs, min-max), bytes per record");
    // animals
    let n = s.animals.len();
    let m0 = allocated();
    let mut aos: Vec<Animal> = s.animals.clone();
    let b_aos = allocated() - m0;
    let m0 = allocated();
    let a = &s.animals;
    let mut soa = AnimalsSoA {
        id: a.iter().map(|r| r.id).collect(),
        species: a.iter().map(|r| r.species).collect(),
        flags: a.iter().map(|r| r.flags).collect(),
        x: a.iter().map(|r| r.x).collect(),
        y: a.iter().map(|r| r.y).collect(),
        energy: a.iter().map(|r| r.energy).collect(),
        health: a.iter().map(|r| r.health).collect(),
        age: a.iter().map(|r| r.age).collect(),
        herd: a.iter().map(|r| r.herd).collect(),
    };
    let b_soa = allocated() - m0;
    let m0 = allocated();
    let mut w = hecs::World::new();
    w.spawn_batch(a.iter().map(|r| (Id(r.id), Species(r.species), Flags(r.flags), Pos(r.x, r.y), Energy(r.energy), Health(r.health), Age(r.age), Herd(r.herd))));
    let b_ecs = allocated() - m0;
    for (name, bytes, f) in [
        ("aos", b_aos, &mut (|| pass_animals_aos(black_box(&mut aos))) as &mut dyn FnMut()),
        ("soa", b_soa, &mut || pass_animals_soa(black_box(&mut soa))),
        ("hecs", b_ecs, &mut || pass_animals_ecs(black_box(&mut w))),
    ] {
        let (m, lo, hi) = time_pass(n, 200, runs, f);
        println!("animals {name:5} {m:7.3} ns ({lo:.3}-{hi:.3})  {:6.1} B/record", bytes as f64 / n as f64);
    }
    let ck: f64 = aos.iter().map(|r| r.energy as f64).sum::<f64>() - soa.energy.iter().map(|v| *v as f64).sum::<f64>();
    println!("check animals aos-soa energy difference {ck:e}");
    drop((aos, soa, w));
    // things
    let n = s.things.len();
    let m0 = allocated();
    let mut aos: Vec<Thing> = s.things.clone();
    let b_aos = allocated() - m0;
    let t = &s.things;
    let m0 = allocated();
    let mut soa = ThingsSoA {
        id: t.iter().map(|r| r.id).collect(),
        kind: t.iter().map(|r| r.kind).collect(),
        material: t.iter().map(|r| r.material).collect(),
        x: t.iter().map(|r| r.x).collect(),
        y: t.iter().map(|r| r.y).collect(),
        z: t.iter().map(|r| r.z).collect(),
        condition: t.iter().map(|r| r.condition).collect(),
        mass: t.iter().map(|r| r.mass).collect(),
        owner: t.iter().map(|r| r.owner).collect(),
        made: t.iter().map(|r| r.made).collect(),
    };
    let b_soa = allocated() - m0;
    let m0 = allocated();
    let mut w = hecs::World::new();
    w.spawn_batch(t.iter().map(|r| (Id(r.id), Kind(r.kind), Material(r.material), Pos(r.x, r.y), Z(r.z), Condition(r.condition), Mass(r.mass), Owner(r.owner), Made(r.made))));
    let b_ecs = allocated() - m0;
    for (name, bytes, f) in [
        ("aos", b_aos, &mut (|| pass_things_aos(black_box(&mut aos))) as &mut dyn FnMut()),
        ("soa", b_soa, &mut || pass_things_soa(black_box(&mut soa))),
        ("hecs", b_ecs, &mut || pass_things_ecs(black_box(&mut w))),
    ] {
        let (m, lo, hi) = time_pass(n, 40, runs, f);
        println!("things  {name:5} {m:7.3} ns ({lo:.3}-{hi:.3})  {:6.1} B/record", bytes as f64 / n as f64);
    }
    drop((aos, soa, w));
    // people cores (memories and relations stay in their own lists in every layout)
    let n = s.people.len();
    let m0 = allocated();
    let mut aos: Vec<PersonCore> = s.people.iter().map(|p| p.core).collect();
    let b_aos = allocated() - m0;
    let m0 = allocated();
    let c = &aos.clone();
    let mut soa = PeopleSoA {
        ident: c.iter().map(|r| [r.id, r.born as u32, r.x, r.y, r.band, r.flags]).collect(),
        body: c.iter().map(|r| r.body).collect(),
        needs: c.iter().map(|r| r.needs).collect(),
        feel: c.iter().map(|r| r.feel).collect(),
        skills: c.iter().map(|r| r.skills).collect(),
        traits: c.iter().map(|r| r.traits).collect(),
    };
    let b_soa = allocated() - m0 - std::mem::size_of_val(&c[..]);
    let m0 = allocated();
    let mut w = hecs::World::new();
    w.spawn_batch(c.iter().map(|r| (Ident([r.id, r.born as u32, r.x, r.y, r.band, r.flags]), Body(r.body), Needs(r.needs), Feel(r.feel), Skills(r.skills), Traits(r.traits))));
    let b_ecs = allocated() - m0;
    for (name, bytes, f) in [
        ("aos", b_aos, &mut (|| pass_people_aos(black_box(&mut aos))) as &mut dyn FnMut()),
        ("soa", b_soa, &mut || pass_people_soa(black_box(&mut soa))),
        ("hecs", b_ecs, &mut || pass_people_ecs(black_box(&mut w))),
    ] {
        let (m, lo, hi) = time_pass(n, 2000, runs, f);
        println!("people  {name:5} {m:7.3} ns ({lo:.3}-{hi:.3})  {:6.1} B/record (core only)", bytes as f64 / n as f64);
    }
    let var: usize = s.people.iter().map(|p| p.mem.capacity() * 24 + p.rel.capacity() * 16 + 48).sum();
    println!("people memories+relations: {:.0} B/person on top of the core", var as f64 / n as f64);
    // random access by id, as relations and targets need
    let idx: Vec<usize> = (0..1_000_000u64).map(|i| (mix(i) % s.things.len() as u64) as usize).collect();
    let mut aos: Vec<Thing> = s.things.clone();
    let mut w = hecs::World::new();
    let ents: Vec<hecs::Entity> = s.things.iter().map(|r| w.spawn((Id(r.id), Material(r.material), Condition(r.condition)))).collect();
    let (m, lo, hi) = time_pass(idx.len(), 5, runs, &mut || {
        for &i in &idx {
            aos[i].condition = aos[i].condition.wrapping_add(1);
        }
        black_box(&aos);
    });
    println!("random access things aos  {m:.2} ns ({lo:.2}-{hi:.2})");
    let (m, lo, hi) = time_pass(idx.len(), 5, runs, &mut || {
        for &i in &idx {
            if let Ok(mut c) = w.get::<&mut Condition>(ents[i]) {
                c.0 = c.0.wrapping_add(1);
            }
        }
    });
    println!("random access things hecs {m:.2} ns ({lo:.2}-{hi:.2})");
}
