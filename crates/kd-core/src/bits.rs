//! The core's stored bits (A15.9 item 5, A3.2, A3.3): each `m` function at 1,000 fixed inputs, 10,000 draws over
//! fixed keys and a few fixed-order sums, made the same way everywhere and hashed. The cloud stores the values and
//! their hashes in `tests/fixtures/` (`kd fixtures write`); the x86-64 and arm64 tests compare every value, the
//! browser and the phone's self-check compare the hashes.
//!
//! Implements RES-05, see A15.9: the phone, the browser and the cloud get the same bits.

use crate::chance::{draw, stream_seed, unit_of};
use crate::m;
use crate::num::{dot_f32, hash64, mix64, sum_f32};

/// The cloud's hashes, one line a probe: its name and its bytes' `num::hash64` in hexadecimal.
pub const STORED: &str = include_str!("../tests/fixtures/hashes.txt");

/// How many inputs each `m` function is checked at.
pub const INPUTS: usize = 1000;
/// How many draws are checked.
pub const DRAWS: usize = 10_000;

/// One probe: a name and the bytes it makes (little-endian values in a fixed order).
pub struct Probe {
    pub name: &'static str,
    pub bytes: Vec<u8>,
}

/// Where a function's random inputs lie.
#[derive(Clone, Copy)]
enum Domain {
    /// Evenly from `lo` to `hi`.
    Span(f32, f32),
    /// Positive, evenly in the exponent from 2^-30 to 2^30, made from bits.
    Positive,
}

type Unary = fn(f32) -> f32;
type Binary = fn(f32, f32) -> f32;

const UNARY: [(&str, Unary, Domain); 13] = [
    ("m.sin", m::sin, Domain::Span(-1000.0, 1000.0)),
    ("m.cos", m::cos, Domain::Span(-1000.0, 1000.0)),
    ("m.tan", m::tan, Domain::Span(-1000.0, 1000.0)),
    ("m.asin", m::asin, Domain::Span(-1.0, 1.0)),
    ("m.acos", m::acos, Domain::Span(-1.0, 1.0)),
    ("m.atan", m::atan, Domain::Span(-1e4, 1e4)),
    ("m.exp", m::exp, Domain::Span(-87.0, 88.0)),
    ("m.exp2", m::exp2, Domain::Span(-126.0, 127.0)),
    ("m.ln", m::ln, Domain::Positive),
    ("m.log2", m::log2, Domain::Positive),
    ("m.log10", m::log10, Domain::Positive),
    ("m.cbrt", m::cbrt, Domain::Span(-1e6, 1e6)),
    ("m.tanh", m::tanh, Domain::Span(-20.0, 20.0)),
];

const BINARY: [(&str, Binary, Domain, Domain); 3] = [
    (
        "m.atan2",
        m::atan2,
        Domain::Span(-100.0, 100.0),
        Domain::Span(-100.0, 100.0),
    ),
    ("m.powf", m::powf, Domain::Span(0.001, 100.0), Domain::Span(-10.0, 10.0)),
    ("m.hypot", m::hypot, Domain::Span(-1e3, 1e3), Domain::Span(-1e3, 1e3)),
];

/// The probes' names, in the order `probes` makes them.
pub fn names() -> Vec<&'static str> {
    let mut v: Vec<&'static str> = UNARY.iter().map(|u| u.0).collect();
    v.extend(BINARY.iter().map(|b| b.0));
    v.extend(["chance.draws", "num.sums"]);
    v
}

/// The fixed seed the inputs are drawn from: "kindling" in ASCII.
const SEED: u64 = 0x6b69_6e64_6c69_6e67;

/// The `i`th input of a domain, from the draw for (`stream`, `i`): never a NaN, an infinity or a value outside the
/// function's domain, whose result could differ only in a NaN's bits.
fn input(domain: Domain, stream: u64, i: usize) -> f32 {
    let d = draw(stream_seed(SEED, 0, 0), i as u64, stream);
    match domain {
        Domain::Span(lo, hi) => lo + (hi - lo) * unit_of(d),
        Domain::Positive => {
            let exponent = ((d >> 32) % 60) as u32 + 127 - 30;
            f32::from_bits((exponent << 23) | (d as u32 & 0x7f_ffff))
        }
    }
}

/// A domain's inputs: its edges first (zero of both signs, ±1, ½, its ends; for `Positive` the smallest normal and
/// a subnormal), then drawn values up to `INPUTS`.
fn inputs(domain: Domain, stream: u64) -> Vec<f32> {
    let mut v: Vec<f32> = match domain {
        Domain::Span(lo, hi) => [0.0, -0.0, 1.0, -1.0, 0.5, -0.5, lo, hi]
            .into_iter()
            .filter(|&x| x >= lo && x <= hi)
            .collect(),
        Domain::Positive => vec![1.0, 2.0, 0.5, 10.0, 0.1, f32::MIN_POSITIVE, 1e-40, 1e30],
    };
    let mut i = 0;
    while v.len() < INPUTS {
        v.push(input(domain, stream, i));
        i += 1;
    }
    v
}

/// The inputs of the `k`th unary function, and the two input lists of the `k`th binary one.
pub fn unary_inputs(k: usize) -> Vec<f32> {
    inputs(UNARY[k].2, 2 * k as u64)
}

pub fn binary_inputs(k: usize) -> (Vec<f32>, Vec<f32>) {
    let s = 2 * (UNARY.len() + k) as u64;
    (inputs(BINARY[k].2, s), inputs(BINARY[k].3, s + 1))
}

fn f32_bytes(values: impl Iterator<Item = f32>) -> Vec<u8> {
    values.flat_map(|x| x.to_bits().to_le_bytes()).collect()
}

/// Every probe, in the order of `names`.
pub fn probes() -> Vec<Probe> {
    let mut out = Vec::new();
    for (k, &(name, f, _)) in UNARY.iter().enumerate() {
        out.push(Probe {
            name,
            bytes: f32_bytes(unary_inputs(k).into_iter().map(f)),
        });
    }
    for (k, &(name, f, _, _)) in BINARY.iter().enumerate() {
        let (x, y) = binary_inputs(k);
        out.push(Probe {
            name,
            bytes: f32_bytes(x.into_iter().zip(y).map(|(x, y)| f(x, y))),
        });
    }
    out.push(Probe {
        name: "chance.draws",
        bytes: draws().flat_map(u64::to_le_bytes).collect(),
    });
    out.push(Probe {
        name: "num.sums",
        bytes: f32_bytes(sums().into_iter()),
    });
    out
}

/// 10,000 draws over keys that vary every field: 10 worlds, the 13 systems, 97 purposes, scattered subjects, and
/// moments with slots.
fn draws() -> impl Iterator<Item = u64> {
    (0..DRAWS as u64).map(|i| {
        let seed = stream_seed(mix64(i / 1000), 1 + (i % 13) as u32, 1 + ((i / 13) % 97) as u32);
        draw(seed, mix64(i ^ 0x55), ((i * 7919) << 16) | (i % 5))
    })
}

/// Fixed-order sums across the tree's block edges, and one dot product.
fn sums() -> Vec<f32> {
    let xs: Vec<f32> = (0..10_000)
        .map(|i| input(Domain::Span(-1000.0, 1000.0), 1_000, i))
        .collect();
    let mut v: Vec<f32> = [1, 7, 64, 4095, 4096, 4097, 10_000]
        .iter()
        .map(|&n| sum_f32(&xs[..n]))
        .collect();
    v.push(dot_f32(&xs[..1000], &xs[1000..2000]));
    v
}

/// The stored hashes, by name.
pub fn stored() -> Vec<(&'static str, u64)> {
    STORED
        .lines()
        .filter_map(|l| {
            let (name, hex) = l.split_once(' ')?;
            Some((name, u64::from_str_radix(hex.trim(), 16).ok()?))
        })
        .collect()
}

/// The hashes file's text for some probes: one line each, `name hash`.
pub fn hashes_text(probes: &[Probe]) -> String {
    probes
        .iter()
        .map(|p| format!("{} {:016x}\n", p.name, hash64(&p.bytes)))
        .collect()
}

/// The probes whose bytes, made here, hash differently from the cloud's stored hash, with the hash made here; a
/// probe missing from the stored file counts as different.
pub fn differences() -> Vec<(&'static str, u64)> {
    let stored = stored();
    probes()
        .into_iter()
        .map(|p| (p.name, hash64(&p.bytes)))
        .filter(|(name, h)| stored.iter().find(|(n, _)| n == name).map(|(_, s)| s) != Some(h))
        .collect()
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;

    macro_rules! files {
        ($($name:literal),* $(,)?) => {
            &[$(($name, include_bytes!(concat!("../tests/fixtures/", $name, ".bin")) as &[u8])),*]
        };
    }

    /// The stored values, as `kd fixtures write` wrote them on x86-64.
    const FILES: &[(&str, &[u8])] = files!(
        "m.sin",
        "m.cos",
        "m.tan",
        "m.asin",
        "m.acos",
        "m.atan",
        "m.exp",
        "m.exp2",
        "m.ln",
        "m.log2",
        "m.log10",
        "m.cbrt",
        "m.tanh",
        "m.atan2",
        "m.powf",
        "m.hypot",
        "chance.draws",
        "num.sums",
    );

    /// Checks one probe made here against its stored values, value by value, naming the first that differs.
    pub(crate) fn compare(name: &str) {
        let probe = probes()
            .into_iter()
            .find(|p| p.name == name)
            .expect("a probe of that name");
        let (_, stored) = FILES
            .iter()
            .find(|(n, _)| *n == name)
            .expect("a stored file of that name");
        let width = if name == "chance.draws" { 8 } else { 4 };
        assert_eq!(
            probe.bytes.len(),
            stored.len(),
            "{name}: {} bytes here, {} stored",
            probe.bytes.len(),
            stored.len()
        );
        if let Some(i) =
            (0..stored.len() / width).find(|&i| probe.bytes[i * width..][..width] != stored[i * width..][..width])
        {
            let at = |b: &[u8]| {
                b[i * width..][..width]
                    .iter()
                    .rev()
                    .map(|x| format!("{x:02x}"))
                    .collect::<String>()
            };
            panic!(
                "{name}: value {i} is 0x{} here, 0x{} stored",
                at(&probe.bytes),
                at(stored)
            );
        }
        let want = stored_hash(name);
        assert_eq!(
            hash64(stored),
            want,
            "{name}: the stored file does not match hashes.txt"
        );
    }

    fn stored_hash(name: &str) -> u64 {
        stored()
            .into_iter()
            .find(|(n, _)| *n == name)
            .map(|(_, h)| h)
            .expect("a stored hash")
    }

    // checks: RES-05
    #[test]
    fn inputs_in_their_domains_and_sums_stored() {
        // Every probe has its stored file and hash; m's and chance's are compared in their own modules' tests.
        assert_eq!(names(), FILES.iter().map(|f| f.0).collect::<Vec<_>>());
        compare("num.sums");
        assert_eq!(differences(), vec![]);
        // Every input is a number in its function's domain, so no result is a NaN, whose bits targets may choose.
        for (k, &(name, f, _)) in UNARY.iter().enumerate() {
            let x = unary_inputs(k);
            assert_eq!(x.len(), INPUTS);
            assert!(x.iter().all(|v| v.is_finite() && !f(*v).is_nan()), "{name}");
        }
        for (k, &(name, f, _, _)) in BINARY.iter().enumerate() {
            let (x, y) = binary_inputs(k);
            assert!(x.iter().zip(&y).all(|(a, b)| !f(*a, *b).is_nan()), "{name}");
        }
    }
}
