//! Numeric ids of catalogue entries (A3.6): each kind's entries are numbered in `data/ids.lock`, and the numbers never
//! change, because saves and snapshots store them (`PLT-09`). Later kinds follow the same pattern (`ItemKind(u16)` in
//! α04a).
//! Implements `PLT-09` in part, see A3.6.

/// A palette colour: its index in the master palette, 0 being `void` (A11.3); at most 255 colours plus `void`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct ColourId(pub u8);

/// A colour family of the palette (`data/palette/colours.md`), by its number in `data/ids.lock`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct FamilyId(pub u8);

/// A ladder of shades, dark to light (`PRE-20`), by its number in `data/ids.lock`: its row in the ladder texture.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct LadderId(pub u8);

/// A light table or palette version (`data/palette/light.md`), by its number in `data/ids.lock`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct LightId(pub u8);
