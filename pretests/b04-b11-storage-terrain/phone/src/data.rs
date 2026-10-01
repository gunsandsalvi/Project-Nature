//! B04: synthetic world state at the sizes stated in NOTES.md ("Size assumptions").
//! Coordinates are `B10` map metres: x < 2^21, y < 2^20; a 1 km cell is 1024 of them,
//! a 256 m patch 256, a region 128 cells (2^17). Everything is drawn from keyed hashes
//! (`TIM-06` style), so the same seed always gives the same bytes on any machine.

pub const W_KM: usize = 2048;
pub const H_KM: usize = 1024;
pub const PATCH_PER_KM: usize = 4; // 256 m patches
pub const REG_KM: usize = 128;
pub const REG_X: usize = W_KM / REG_KM; // 16
pub const REG_Y: usize = H_KM / REG_KM; // 8
pub const N_REGIONS: usize = REG_X * REG_Y; // 128

#[repr(C)]
#[derive(Clone, Copy, Default, PartialEq, Debug)]
pub struct Animal {
    pub id: u32,
    pub species: u16,
    pub flags: u16,
    pub x: u32,
    pub y: u32,
    pub energy: f32,
    pub health: f32,
    pub age: u32,
    pub herd: u32,
} // 32 bytes

#[repr(C)]
#[derive(Clone, Copy, Default, PartialEq, Debug)]
pub struct Thing {
    pub id: u32,
    pub kind: u16,
    pub material: u16,
    pub x: u32,
    pub y: u32,
    pub z: i16,
    pub condition: u16,
    pub mass: u32,
    pub owner: u32,
    pub made: i32,
} // 32 bytes

#[repr(C)]
#[derive(Clone, Copy, Default, PartialEq, Debug)]
pub struct Cell {
    pub elev: f32,
    pub water: f32,
    pub soil: [u16; 4],
    pub veg: [u16; 4],
    pub snow: u16,
    pub temp: i16,
    pub flags: u32,
} // 32 bytes

#[repr(C)]
#[derive(Clone, Copy, Default, PartialEq, Debug)]
pub struct Patch {
    pub b: [u16; 4],
} // 8 bytes

#[repr(C)]
#[derive(Clone, Copy, Default, PartialEq, Debug)]
pub struct Memory {
    pub day: i32,
    pub kind: u16,
    pub valence: i16,
    pub subject: u32,
    pub place: u32,
    pub strength: f32,
    pub detail: u32,
} // 24 bytes

#[repr(C)]
#[derive(Clone, Copy, Default, PartialEq, Debug)]
pub struct Relation {
    pub other: u32,
    pub kind: u16,
    pub trust: i16,
    pub affection: i16,
    pub fear: i16,
    pub last_day: i32,
} // 16 bytes

#[repr(C)]
#[derive(Clone, Copy, PartialEq, Debug)]
pub struct PersonCore {
    pub id: u32,
    pub born: i32,
    pub x: u32,
    pub y: u32,
    pub band: u32,
    pub flags: u32,
    pub body: [f32; 16],
    pub needs: [f32; 8],
    pub feel: [f32; 8],
    pub skills: [u16; 32],
    pub traits: [u8; 40],
} // 256 bytes

impl Default for PersonCore {
    fn default() -> Self {
        PersonCore { id: 0, born: 0, x: 0, y: 0, band: 0, flags: 0, body: [0.0; 16], needs: [0.0; 8], feel: [0.0; 8], skills: [0; 32], traits: [0; 40] }
    }
}

#[derive(Clone, Default, PartialEq, Debug)]
pub struct Person {
    pub core: PersonCore,
    pub mem: Vec<Memory>,
    pub rel: Vec<Relation>,
}

/// Plain-old-data records: fixed little-endian layout, no padding (checked in tests).
pub unsafe trait Pod: Copy + Default {}
unsafe impl Pod for Animal {}
unsafe impl Pod for Thing {}
unsafe impl Pod for Cell {}
unsafe impl Pod for Patch {}
unsafe impl Pod for Memory {}
unsafe impl Pod for Relation {}
unsafe impl Pod for PersonCore {}

pub fn as_bytes<T: Pod>(v: &[T]) -> &[u8] {
    unsafe { std::slice::from_raw_parts(v.as_ptr() as *const u8, std::mem::size_of_val(v)) }
}

/// Copy records out of bytes (any alignment). Length must be a whole number of records.
pub fn from_bytes<T: Pod>(b: &[u8]) -> Option<Vec<T>> {
    let sz = std::mem::size_of::<T>();
    if b.len() % sz != 0 {
        return None;
    }
    let n = b.len() / sz;
    let mut v: Vec<T> = vec![T::default(); n];
    unsafe { std::ptr::copy_nonoverlapping(b.as_ptr(), v.as_mut_ptr() as *mut u8, b.len()) };
    Some(v)
}

/// The whole state of a world at a saved moment.
#[derive(Clone, Default, PartialEq, Debug)]
pub struct State {
    pub day: u64,
    pub seed: u64,
    /// km cells, row-major, W_KM x H_KM scaled by `div`.
    pub w: usize,
    pub h: usize,
    pub cells: Vec<Cell>,
    /// 256 m patches, row-major, (w*4) x (h*4).
    pub patches: Vec<Patch>,
    pub people: Vec<Person>,
    pub animals: Vec<Animal>,
    pub things: Vec<Thing>,
}

#[inline]
pub fn mix(mut x: u64) -> u64 {
    x = (x ^ (x >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
    x = (x ^ (x >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
    x ^ (x >> 31)
}
#[inline]
pub fn key(seed: u64, a: u64, b: u64) -> u64 {
    mix(seed ^ mix(a ^ mix(b ^ 0x9E37_79B9_7F4A_7C15)))
}
#[inline]
fn unit(h: u64) -> f32 {
    (h >> 40) as f32 / (1u64 << 24) as f32
}

/// Smooth periodic field in [0,1): bilinear lattice noise, two octaves. `p` = lattice period in samples.
fn smooth(seed: u64, x: usize, y: usize, w: usize, h: usize, p: usize) -> f32 {
    let mut acc = 0.0f32;
    let mut amp = 0.65f32;
    let mut per = p.max(2);
    for oct in 0..2u64 {
        let (gw, gh) = ((w / per).max(1), (h / per).max(1));
        let (fx, fy) = (x as f32 / per as f32, y as f32 / per as f32);
        let (ix, iy) = (fx as usize, fy as usize);
        let (tx, ty) = (fx - ix as f32, fy - iy as f32);
        let g = |gx: usize, gy: usize| unit(key(seed ^ oct, (gx % gw) as u64, (gy % gh) as u64));
        let a = g(ix, iy) + (g(ix + 1, iy) - g(ix, iy)) * tx;
        let b = g(ix, iy + 1) + (g(ix + 1, iy + 1) - g(ix, iy + 1)) * tx;
        acc += amp * (a + (b - a) * ty);
        amp = 0.35;
        per = (per / 4).max(2);
    }
    acc
}

pub fn region_of(x: u32, y: u32) -> usize {
    ((y as usize >> 17) % REG_Y) * REG_X + ((x as usize >> 17) % REG_X)
}

/// Build the synthetic state. `div` shrinks every count and the map side by that factor
/// (1 = full size; the crash test and the phone use smaller worlds).
pub fn build(seed: u64, day: u64, div: usize) -> State {
    let div = div.max(1);
    let (w, h) = (W_KM / div, H_KM / div);
    let (n_people, n_animals, n_things) = (10_000 / div, 100_000 / div, 1_000_000 / div);
    let xmax = (w as u32) << 10;
    let ymax = (h as u32) << 10;
    // km cells: elevation, water, soil and plants as smooth fields with a small wobble.
    let mut cells = Vec::with_capacity(w * h);
    for y in 0..h {
        for x in 0..w {
            let e = smooth(seed, x, y, w, h, 64) - 0.55;
            let r = key(seed ^ 0xCE11 ^ day, x as u64, y as u64);
            let wob = |k: u32| ((r >> (k * 4)) & 7) as u16;
            let q = |s: u64, sc: f32| (smooth(seed ^ s, x, y, w, h, 32) * sc) as u16;
            cells.push(Cell {
                elev: (e * 4000.0).round(),
                water: if e < 0.0 { 0.0 } else { (smooth(seed ^ 7, x, y, w, h, 16) * 50.0).round() / 10.0 },
                soil: [q(1, 6000.0) + wob(0), q(2, 6000.0) + wob(1), q(3, 3000.0), (q(4, 8.0)) as u16],
                veg: [q(5, 9000.0) + wob(2), q(6, 9000.0) + wob(3), q(8, 9000.0) + wob(4), q(9, 2000.0)],
                snow: if y < h / 8 || y > h - h / 8 { q(10, 500.0) } else { 0 },
                temp: (300.0 - (y as f32 - h as f32 / 2.0).abs() / h as f32 * 600.0) as i16 + wob(5) as i16,
                flags: if e < 0.0 { 1 } else { 0 },
            });
        }
    }
    let (pw, ph) = (w * PATCH_PER_KM, h * PATCH_PER_KM);
    let mut patches = Vec::with_capacity(pw * ph);
    for y in 0..ph {
        for x in 0..pw {
            let c = &cells[(y / PATCH_PER_KM) * w + x / PATCH_PER_KM];
            let r = key(seed ^ 0xBA7C ^ day, x as u64, y as u64);
            let b0 = c.veg[0] / 2 + ((r & 255) as u16);
            let b1 = c.veg[1] / 2 + (((r >> 8) & 255) as u16);
            let b2 = if c.flags & 1 == 1 { 0 } else { c.veg[2] / 2 + (((r >> 16) & 63) as u16) };
            let b3 = ((r >> 24) & 15) as u16;
            patches.push(Patch { b: [b0, b1, b2, b3] });
        }
    }
    let land = |x: u32, y: u32| cells[((y >> 10) as usize % h) * w + (x >> 10) as usize % w].flags & 1 == 0;
    let pick_land = |k: u64| -> (u32, u32) {
        for t in 0..64u64 {
            let r = key(seed, k, t);
            let (x, y) = ((r as u32) % xmax, ((r >> 32) as u32) % ymax);
            if land(x, y) {
                return (x, y);
            }
        }
        ((key(seed, k, 99) as u32) % xmax, 0)
    };
    // Bands of 25 people; camps on land, spread across about a fifth of the map's regions.
    let n_bands = (n_people + 24) / 25;
    let camps: Vec<(u32, u32)> = (0..n_bands as u64).map(|b| pick_land(0xCA4D_0000 + (b % 97) * 1_000 + b)).collect();
    let near = |c: (u32, u32), r: u64, spread: u32| -> (u32, u32) {
        let dx = (r as u32 % (2 * spread + 1)) as i64 - spread as i64;
        let dy = ((r >> 32) as u32 % (2 * spread + 1)) as i64 - spread as i64;
        (((c.0 as i64 + dx).rem_euclid(xmax as i64)) as u32, ((c.1 as i64 + dy).rem_euclid(ymax as i64)) as u32)
    };
    let mut people = Vec::with_capacity(n_people);
    for i in 0..n_people as u32 {
        let r = key(seed ^ day, 0x9E09, i as u64);
        let band = (i as usize / 25) as u32;
        let (x, y) = near(camps[band as usize], r, 2000);
        let mut core = PersonCore { id: i, born: -((r % 20000) as i32) + day as i32, x, y, band, flags: (r >> 60) as u32, ..Default::default() };
        for k in 0..16 {
            core.body[k] = (unit(key(seed, i as u64, 100 + k as u64)) * 100.0).round() / 100.0;
        }
        for k in 0..8 {
            core.needs[k] = (unit(key(seed ^ day, i as u64, 200 + k as u64)) * 1000.0).round() / 1000.0;
            core.feel[k] = (unit(key(seed ^ day, i as u64, 300 + k as u64)) * 100.0).round() / 100.0;
        }
        for k in 0..32 {
            core.skills[k] = (key(seed, i as u64, 400 + k as u64) % 1000) as u16;
        }
        for k in 0..40 {
            core.traits[k] = (key(seed, i as u64, 500 + k as u64) % 7) as u8;
        }
        let nm = 50 + (r % 101) as usize;
        let nr = 10 + ((r >> 8) % 51) as usize;
        let mem = (0..nm)
            .map(|k| {
                let m = key(seed ^ day, i as u64, 10_000 + k as u64);
                Memory {
                    day: day as i32 - (m % 7300) as i32,
                    kind: (m >> 20) as u16 % 40,
                    valence: ((m >> 28) % 200) as i16 - 100,
                    subject: (m >> 36) as u32 % (n_people as u32 + n_animals as u32),
                    place: ((y >> 10) << 11) | (x >> 10),
                    strength: ((m >> 50) % 100) as f32 / 100.0,
                    detail: (m >> 8) as u32 & 0xFFFF,
                }
            })
            .collect();
        let rel = (0..nr)
            .map(|k| {
                let m = key(seed ^ day, i as u64, 20_000 + k as u64);
                Relation {
                    other: if k < 20 { band * 25 + (k as u32 % 25) } else { (m as u32) % n_people as u32 },
                    kind: (m >> 32) as u16 % 12,
                    trust: ((m >> 40) % 200) as i16 - 100,
                    affection: ((m >> 48) % 200) as i16 - 100,
                    fear: ((m >> 56) % 100) as i16,
                    last_day: day as i32 - ((m >> 16) % 400) as i32,
                }
            })
            .collect();
        people.push(Person { core, mem, rel });
    }
    // Animals in herds of 20 on land.
    let mut animals = Vec::with_capacity(n_animals);
    let mut herd_pos = (0, 0);
    for i in 0..n_animals as u32 {
        if i % 20 == 0 {
            herd_pos = pick_land(0xA41_0000_0000 + i as u64);
        }
        let r = key(seed ^ day, 0xA41, i as u64);
        let (x, y) = near(herd_pos, r, 3000);
        animals.push(Animal {
            id: i,
            species: ((i / 20) % 50) as u16,
            flags: (r >> 60) as u16,
            x,
            y,
            energy: ((r >> 20) % 1000) as f32 / 1000.0,
            health: ((r >> 30) % 100) as f32 / 100.0,
            age: ((r >> 40) % 5000) as u32,
            herd: i / 20,
        });
    }
    // Things: 70% around camps (tools, food, firewood), 30% anywhere on land.
    let mut things = Vec::with_capacity(n_things);
    for i in 0..n_things as u32 {
        let r = key(seed ^ day, 0x7B1, i as u64);
        let (x, y, owner) = if r % 10 < 7 {
            let b = (r >> 8) as usize % n_bands;
            let (x, y) = near(camps[b], r >> 16, 1000);
            (x, y, (b * 25) as u32 + ((r >> 4) % 25) as u32)
        } else {
            let (x, y) = pick_land(0x7B1_0000_0000 + i as u64);
            (x, y, u32::MAX)
        };
        things.push(Thing {
            id: i,
            kind: ((r >> 20) % 300) as u16,
            material: ((r >> 30) % 60) as u16,
            x,
            y,
            z: ((r >> 40) % 40) as i16 - 20,
            condition: ((r >> 46) % 1000) as u16,
            mass: ((r >> 50) % 5000) as u32 + 10,
            owner,
            made: day as i32 - ((r >> 12) % 3650) as i32,
        });
    }
    State { day, seed, w, h, cells, patches, people, animals, things }
}

/// Fast 4-lane hash for integrity checks (any flipped bit changes it).
pub fn hash(b: &[u8]) -> u64 {
    const K: [u64; 4] = [0x9E37_79B9_7F4A_7C15, 0xC2B2_AE3D_27D4_EB4F, 0x1656_67B1_9E37_79F9, 0x27D4_EB2F_1656_67C5];
    let mut s = [K[0] ^ b.len() as u64, K[1], K[2], K[3]];
    let mut ch = b.chunks_exact(32);
    for c in &mut ch {
        for l in 0..4 {
            let w = u64::from_le_bytes(c[l * 8..l * 8 + 8].try_into().unwrap());
            s[l] = (s[l] ^ w).wrapping_mul(K[l] | 1).rotate_left(29);
        }
    }
    let mut tail = [0u8; 32];
    let rem = ch.remainder();
    tail[..rem.len()].copy_from_slice(rem);
    for l in 0..4 {
        let w = u64::from_le_bytes(tail[l * 8..l * 8 + 8].try_into().unwrap());
        s[l] = (s[l] ^ w).wrapping_mul(K[l] | 1).rotate_left(29);
    }
    mix(mix(s[0] ^ mix(s[1])) ^ mix(s[2] ^ mix(s[3])))
}

/// Hash of the full content in canonical (id) order.
pub fn content_hash(s: &State) -> u64 {
    let mut h = mix(s.day ^ mix(s.seed ^ mix((s.w as u64) << 32 | s.h as u64)));
    h = mix(h ^ hash(as_bytes(&s.cells)));
    h = mix(h ^ hash(as_bytes(&s.patches)));
    h = mix(h ^ hash(as_bytes(&s.animals)));
    h = mix(h ^ hash(as_bytes(&s.things)));
    for p in &s.people {
        h = mix(h ^ hash(as_bytes(std::slice::from_ref(&p.core))));
        h = mix(h ^ hash(as_bytes(&p.mem)) ^ (p.mem.len() as u64) << 1);
        h = mix(h ^ hash(as_bytes(&p.rel)) ^ (p.rel.len() as u64) << 2);
    }
    h
}

pub fn raw_bytes(s: &State) -> usize {
    std::mem::size_of_val(&s.cells[..])
        + std::mem::size_of_val(&s.patches[..])
        + std::mem::size_of_val(&s.animals[..])
        + std::mem::size_of_val(&s.things[..])
        + s.people.iter().map(|p| 256 + p.mem.len() * 24 + p.rel.len() * 16).sum::<usize>()
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn sizes() {
        assert_eq!(std::mem::size_of::<Animal>(), 32);
        assert_eq!(std::mem::size_of::<Thing>(), 32);
        assert_eq!(std::mem::size_of::<Cell>(), 32);
        assert_eq!(std::mem::size_of::<Patch>(), 8);
        assert_eq!(std::mem::size_of::<Memory>(), 24);
        assert_eq!(std::mem::size_of::<Relation>(), 16);
        assert_eq!(std::mem::size_of::<PersonCore>(), 256);
    }
}
