//! The compiled catalogue (A3.6): its tables, and the blob that carries them to every target behind a header (magic
//! `KDCAT`, format version, rules version, generator version, `num::hash64` of the body).
//! Implements `MAT-13` and `PLT-09` in part, see A3.6.

use crate::schema::{LightMethod, LightRole};
use kd_core::kinds::{ColourId, FamilyId, LadderId, LightId};
use serde::{Deserialize, Serialize};
use std::fmt;

/// The blob's first five bytes.
pub const MAGIC: &[u8; 5] = b"KDCAT";
/// The blob layout's version; a change to the header or the body's encoding raises it.
pub const FORMAT: u16 = 1;
/// Header length: magic, format, major, minor, generator, hash.
pub const HEADER_LEN: usize = 5 + 2 + 2 + 2 + 2 + 8;

/// A colour family, in number order; its colours are `first .. first + count` of the palette.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct FamilyRec {
    pub id: String,
    pub name: String,
    pub first: u8,
    pub count: u8,
}

/// A palette colour, at its index.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct ColourRec {
    pub name: String,
    pub rgb: [u8; 3],
    pub family: u8,
}

/// A ladder, in number order, its steps as palette indices dark to light.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct LadderRec {
    pub id: String,
    pub name: String,
    pub steps: Vec<u8>,
}

/// A light table or palette version, in number order, its references resolved to numbers; numbers stay the decimal
/// strings the entry writes (see `schema`).
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct LightRec {
    pub id: String,
    pub name: String,
    pub role: LightRole,
    pub method: LightMethod,
    pub k: String,
    pub toward: [u8; 3],
    pub amount: String,
    pub mul: [String; 3],
    pub add: [String; 3],
    pub l_min: Option<String>,
    pub l_max: Option<String>,
    /// For a table, whether each palette index may be given; for a version, whether it is kept unchanged.
    pub mask: Vec<bool>,
}

/// The catalogue's body: every kind's table in id-number order.
#[derive(Clone, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct Body {
    pub families: Vec<FamilyRec>,
    pub colours: Vec<ColourRec>,
    pub ladders: Vec<LadderRec>,
    pub light: Vec<LightRec>,
}

/// The loaded catalogue: the header's versions and hash, and the body.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct Catalogue {
    pub major: u16,
    pub minor: u16,
    pub generator: u16,
    pub hash: u64,
    pub body: Body,
}

/// Why a blob did not load.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum LoadError {
    TooShort,
    Magic,
    Format(u16),
    Hash,
    Body(String),
}

impl fmt::Display for LoadError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            LoadError::TooShort => write!(f, "catalogue blob shorter than its header"),
            LoadError::Magic => write!(f, "catalogue blob does not start with KDCAT"),
            LoadError::Format(v) => write!(f, "catalogue blob format {v}, this build reads {FORMAT}"),
            LoadError::Hash => write!(f, "catalogue blob's hash does not match its body"),
            LoadError::Body(e) => write!(f, "catalogue blob's body does not decode: {e}"),
        }
    }
}

fn u16_at(b: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([b[at], b[at + 1]])
}

impl Catalogue {
    /// Loads a blob, checking magic, format and hash (A3.6: under 10 ms on the phone).
    pub fn load(b: &[u8]) -> Result<Catalogue, LoadError> {
        if b.len() < HEADER_LEN {
            return Err(LoadError::TooShort);
        }
        if &b[..5] != MAGIC {
            return Err(LoadError::Magic);
        }
        let format = u16_at(b, 5);
        if format != FORMAT {
            return Err(LoadError::Format(format));
        }
        let mut h = [0u8; 8];
        h.copy_from_slice(&b[13..21]);
        let hash = u64::from_le_bytes(h);
        let body = &b[HEADER_LEN..];
        if kd_core::num::hash64(body) != hash {
            return Err(LoadError::Hash);
        }
        let body: Body = postcard::from_bytes(body).map_err(|e| LoadError::Body(e.to_string()))?;
        Ok(Catalogue {
            major: u16_at(b, 7),
            minor: u16_at(b, 9),
            generator: u16_at(b, 11),
            hash,
            body,
        })
    }

    /// The blob for a body under these versions.
    pub fn encode(major: u16, minor: u16, generator: u16, body: &Body) -> Vec<u8> {
        let bytes = postcard::to_allocvec(body).unwrap_or_default();
        let mut out = Vec::with_capacity(HEADER_LEN + bytes.len());
        out.extend_from_slice(MAGIC);
        for v in [FORMAT, major, minor, generator] {
            out.extend_from_slice(&v.to_le_bytes());
        }
        out.extend_from_slice(&kd_core::num::hash64(&bytes).to_le_bytes());
        out.extend_from_slice(&bytes);
        out
    }

    /// `<major>.<minor> <first 8 hex digits of the hash>`, for the self-check (A15.4).
    pub fn version_line(&self) -> String {
        format!("{}.{} {:08x}", self.major, self.minor, self.hash >> 32)
    }

    /// The palette index of the colour with this name.
    pub fn colour(&self, name: &str) -> Option<ColourId> {
        let k = self.body.colours.iter().position(|c| c.name == name)?;
        u8::try_from(k).ok().map(ColourId)
    }

    /// The family with this id.
    pub fn family(&self, id: &str) -> Option<FamilyId> {
        let k = self.body.families.iter().position(|f| f.id == id)?;
        u8::try_from(k).ok().map(FamilyId)
    }

    /// The ladder with this id.
    pub fn ladder(&self, id: &str) -> Option<LadderId> {
        let k = self.body.ladders.iter().position(|l| l.id == id)?;
        u8::try_from(k).ok().map(LadderId)
    }

    /// The light entry with this id.
    pub fn light(&self, id: &str) -> Option<LightId> {
        let k = self.body.light.iter().position(|l| l.id == id)?;
        u8::try_from(k).ok().map(LightId)
    }

    /// The light entries of one role, in number order: the table texture's rows or the palette texture's rows.
    pub fn light_rows(&self, role: LightRole) -> impl Iterator<Item = &LightRec> {
        self.body.light.iter().filter(move |l| l.role == role)
    }
}

#[cfg(test)]
mod tests;
